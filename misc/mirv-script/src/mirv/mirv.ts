import { IWsConnection, WsConnection } from '../ws-connection/ws-connection';

let wsAddress = 'ws://localhost:31337/mirv';
let wsEnable = true;
let tickCount = 0;
let setView: mirv.OnCViewRenderSetupViewSet | null = null;

interface wsAddress {
	host: string;
	port: number;
	path: string;
}

export type EntityObject = {
	name: string;
	debugName: string | null;
	className: string;
	isPlayerPawn: boolean;
	isPlayerController: boolean;
	playerPawnHandle: number;
	playerControllerHandle: number;
	health: number;
	origin: number[];
	renderEyeOrigin: number[];
	renderEyeAngles: number[];
};

export const main = (address?: wsAddress) => {
	let wsConnection: IWsConnection | null = null;
	if (address) wsAddress = `ws://${address.host}:${address.port}/${address.path}?hlae=1`;
	let lastView: mirv.OnCViewRenderSetupViewArgs['lastView'];
	// could've been imported from other file, but not possible for now
	const events = [
		'listTypes',
		'quit',
		'exec',
		'getLastView',
		'setView',
		'setGameEvents',
		'setCViewRenderSetupView',
		'setEntityEvents',
		'onCViewRenderSetupView',
		'onGameEvent',
		'onAddEntity',
		'onRemoveEntity',
		'listEntities'
	] as const;
	// just for ease of change
	const warningEvent = 'warning';

	const sendWarning = (msg: string) => {
		mirv.warning(msg);
		if (wsConnection)
			wsConnection.send(
				JSON.stringify({
					type: warningEvent,
					data: msg
				})
			);
	};

	const makeEntityObject = (e: mirv.Entity): EntityObject => {
		return {
			name: e.getName(),
			debugName: e.getDebugName(),
			className: e.getClassName(),
			isPlayerPawn: e.isPlayerPawn(),
			isPlayerController: e.isPlayerController(),
			playerPawnHandle: e.getPlayerPawnHandle(),
			playerControllerHandle: e.getPlayerControllerHandle(),
			health: e.getHealth(),
			origin: e.getOrigin(),
			renderEyeOrigin: e.getRenderEyeOrigin(),
			renderEyeAngles: e.getRenderEyeAngles()
		};
	};

	// here we define behaviour of websocket events and websocket connection as well
	mirv.onClientFrameStageNotify = function (e) {
		//mirv.message("onClientFrameStageNotify: "+e.curStage+" / "+e.isBefore+"\n");

		if (
			e.curStage == 0 && // FRAME_START - called on host_frame (1 per tick).
			e.isBefore
		) {
			// Restore websockett connection:

			if (null === wsConnection || wsConnection.hasException()) {
				if (null !== wsConnection) {
					mirv.warning(
						'onClientFrameStageNotify: wsConnection failed: ' +
							String(wsConnection.getException()) +
							'\n'
					);
					wsConnection.close();
					wsConnection = null;
					// setView = null; // our connection crashed ... consider this.
				}

				// Every 64 ticks we try to restore the connection:
				if (wsEnable && 0 == tickCount % 64) {
					mirv.message(
						'onClientFrameStageNotify: making new wsConnection: ' + wsAddress + '\n'
					);
					wsConnection = new WsConnection({
						address: wsAddress
					});
				}
			}

			if (null !== wsConnection && !wsEnable) {
				wsConnection.close();
				wsConnection = null;
			}

			// We use this to request an extra processing of jobs from HLAE (currently by default it only proccesses jobs upon after FRAME_RENDER_END == 6)
			mirv.run_jobs();
			mirv.run_jobs_async();

			if (null !== wsConnection) {
				// Flush any messages that are lingering:
				wsConnection.flush();

				// Handle messages that came in meanwhile:
				for (
					let message = wsConnection.next();
					message !== null;
					message = wsConnection.next()
				) {
					try {
						switch (typeof message) {
							case 'string':
								{
									// mirv.message(message.toString() + '\n');
									const messageObj = JSON.parse(message) as {
										type: (typeof events)[number];
										data: string | boolean | mirv.OnCViewRenderSetupViewSet;
									};

									switch (messageObj.type) {
										case 'listTypes':
											mirv.message(
												'Available types:\n' + events.join('\n') + '\n'
											);
											wsConnection.send(
												JSON.stringify({ type: events[0], data: events })
											);
											break;
										case 'exec':
											if (typeof messageObj.data === 'string') {
												mirv.exec(messageObj.data);
											} else {
												sendWarning('TypeError in exec: expected string');
											}
											break;
										case 'getLastView':
											wsConnection.send(
												JSON.stringify({
													type: events[3],
													data: lastView
												})
											);
											break;
										case 'setView':
											if (messageObj.data === null) {
												setView = null;
												break;
											}
											if (
												typeof messageObj.data !== 'string' &&
												typeof messageObj.data !== 'boolean' &&
												('x' in messageObj.data ||
													'y' in messageObj.data ||
													'z' in messageObj.data ||
													'rX' in messageObj.data ||
													'rY' in messageObj.data ||
													'rZ' in messageObj.data ||
													'fov' in messageObj.data)
											) {
												setView = messageObj.data;
											} else {
												sendWarning(
													'TypeError in setView: expected object with x,y,z,rX,rY,rZ,fov or null'
												);
											}
											break;
										case 'quit':
											wsEnable = false;
											wsConnection.close();
											mirv.exec('quit');
											break;
										case 'setGameEvents':
											if (typeof messageObj.data === 'boolean') {
												messageObj.data
													? (mirv.onGameEvent = onGameEvent)
													: (mirv.onGameEvent = undefined);
											} else {
												sendWarning(
													'TypeError in gameEvents: expected boolean'
												);
											}
											break;
										case 'setCViewRenderSetupView':
											if (typeof messageObj.data === 'boolean') {
												messageObj.data
													? (mirv.onCViewRenderSetupView =
															onCViewRenderSetupView)
													: (mirv.onCViewRenderSetupView = undefined);
											} else {
												sendWarning(
													'TypeError in cViewRenderSetupView: expected boolean'
												);
											}
											break;

										case 'setEntityEvents':
											if (typeof messageObj.data === 'boolean') {
												messageObj.data
													? (mirv.onAddEntity = onAddEntity)
													: (mirv.onAddEntity = undefined);
												messageObj.data
													? (mirv.onRemoveEntity = onRemoveEntity)
													: (mirv.onRemoveEntity = undefined);
											} else {
												sendWarning(
													'TypeError in entityEvents: expected boolean'
												);
											}
											break;
										case 'listEntities':
											{
												const entities: EntityObject[] = [];
												const highest = mirv.getHighestEntityIndex();
												for (let i = 0; i < highest + 1; i++) {
													const entity = mirv.getEntityFromIndex(i);
													if (entity === null) continue;
													entities.push(makeEntityObject(entity));
												}
												wsConnection.send(
													JSON.stringify({
														type: events[12],
														data: entities
													})
												);
											}
											break;
										default:
											sendWarning(
												'TypeError in onClientFrameStageNotify: Unknown incoming message.type:' +
													typeof messageObj.type
											);
											break;
									}
								}
								break;
							default:
								sendWarning(
									'TypeError in onClientFrameStageNotify: Warning: Unhandled incoming message of type: ' +
										typeof message
								);

								break;
						}
					} catch (err) {
						mirv.warning(
							'onClientFrameStageNotify: Error while handling incoming message:' +
								String(err) +
								'\n'
						);
					}
				}
			}

			tickCount += 1;
		}

		if (
			e.curStage == 5 && // FRAME_RENDER_START - this is not called when demo is paused (can be multiple per tick).
			e.isBefore
		) {
			//
		}

		if (
			e.curStage == 5 && // FRAME_RENDER_END - this is not called when demo is paused (can be multiple per tick).
			e.isBefore
		) {
			if (null !== wsConnection) wsConnection.flush();
		}
	};

	// we define it here to assign later on demand, same for others
	const onCViewRenderSetupView: mirv.OnCViewRenderSetupView = (e) => {
		if (null !== wsConnection) {
			lastView = e.lastView;
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[8],
						data: e
					})
				);

				// we could flush and then wait for a reply here to set a view instantly, but don't understimate network round-trip time!
			} catch (err) {
				mirv.warning(
					'onCViewRenderSetupView: Error while sending message:' + String(err) + '\n'
				);
			}
		}
		if (setView !== null) {
			return setView;
		}
	};

	const onGameEvent: mirv.OnGameEvent = (e) => {
		if (null !== wsConnection) {
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[9],
						data: e
					})
				);
			} catch (err) {
				mirv.warning('onGameEvent: Error while sending message:' + String(err) + '\n');
			}
		}
	};

	const onAddEntity: mirv.OnEntityEvent = (e) => {
		if (null !== wsConnection) {
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[10],
						data: makeEntityObject(e)
					})
				);
			} catch (err) {
				mirv.warning('onAddEntity: Error while sending message:' + String(err) + '\n');
			}
		}
	};
	const onRemoveEntity: mirv.OnEntityEvent = (e) => {
		if (null !== wsConnection) {
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[11],
						data: makeEntityObject(e)
					})
				);
			} catch (err) {
				mirv.warning('onRemoveEntity: Error while sending message:' + String(err) + '\n');
			}
		}
	};

	// if set to undefined, then it's completely disabled (but can be turned on later, when defined)
	mirv.onCViewRenderSetupView = undefined;
	mirv.onGameEvent = undefined;
	mirv.onAddEntity = undefined;
	mirv.onRemoveEntity = undefined;
};
