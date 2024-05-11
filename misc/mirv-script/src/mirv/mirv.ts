import { MirvEvents } from '../server';
import { IWsConnection, WsConnection } from '../ws-connection/ws-connection';

export interface MirvFlags {
	gameEvents: boolean;
	cViewRenderSetupView: boolean;
}

let wsAddress = 'ws://localhost:31337/mirv';
let wsEnable = true;
let tickCount = 0;
let setView: mirv.OnCViewRenderSetupViewSet | null = null;
let mirvFlags = {
	gameEvents: false,
	cViewRenderSetupView: false
};

interface wsAddress {
	host: string;
	port: number;
	path: string;
}

export const main = (flags?: MirvFlags, address?: wsAddress) => {
	let wsConnection: IWsConnection | null = null;
	if (flags) mirvFlags = flags;
	if (address) wsAddress = `ws://${address.host}:${address.port}/${address.path}?hlae=1`;
	let lastView: mirv.OnCViewRenderSetupViewArgs['lastView'];
	const events = [
		'listTypes',
		'quit',
		'exec',
		'getLastView',
		'setView',
		'gameEvents',
		'cViewRenderSetupView'
	] as const;

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
										type: MirvEvents;
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
												mirv.warning('exec error: expected string');
												wsConnection.send(
													JSON.stringify({
														type: 'warning',
														data: 'exec error: expected string'
													})
												);
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
												mirv.warning(
													'setView error: expected x,y,z,rX,rY,rZ,fov'
												);
												wsConnection.send(
													JSON.stringify({
														type: 'warning',
														data: 'setView error: expected x,y,z,rX,rY,rZ,fov'
													})
												);
											}
											break;
										case 'quit':
											wsEnable = false;
											wsConnection.close();
											mirv.exec('quit');
											break;
										case 'gameEvents':
											if (typeof messageObj.data === 'boolean') {
												mirvFlags.gameEvents = messageObj.data;
											} else {
												mirv.warning(
													'onClientFrameStageNotify: Unknown gameEvents value: ' +
														messageObj.data +
														'\n' +
														'Possible values are: true, false' +
														'\n'
												);
												wsConnection.send(
													JSON.stringify({
														type: 'warning',
														data:
															'onClientFrameStageNotify: Unknown gameEvents value: ' +
															messageObj.data +
															'\n' +
															'Possible values are: true, false' +
															'\n'
													})
												);
											}
											break;
										case 'cViewRenderSetupView':
											if (typeof messageObj.data === 'boolean') {
												mirvFlags.cViewRenderSetupView = messageObj.data;
											} else {
												mirv.warning(
													'onClientFrameStageNotify: Unknown cViewRenderSetupView value: ' +
														messageObj.data +
														'\n' +
														'Possible values are: true, false' +
														'\n'
												);
												wsConnection.send(
													JSON.stringify({
														type: 'warning',
														data:
															'onClientFrameStageNotify: Unknown cViewRenderSetupView value: ' +
															messageObj.data +
															'\n' +
															'Possible values are: true, false' +
															'\n'
													})
												);
											}
											break;
										default:
											mirv.warning(
												'onClientFrameStageNotify: Unknown incoming message.type:' +
													messageObj.type +
													'\n'
											);
											wsConnection.send(
												JSON.stringify({
													type: 'warning',
													data:
														'onClientFrameStageNotify: Unknown incoming message.type:' +
														messageObj.type +
														'\n'
												})
											);
											break;
									}
								}
								break;
							default:
								mirv.warning(
									'onClientFrameStageNotify: Warning: Unhandled incoming message of type: ' +
										typeof message
								);
								wsConnection.send(
									JSON.stringify({
										type: 'warning',
										data:
											'onClientFrameStageNotify: Warning: Unhandled incoming message of type: ' +
											typeof message
									})
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

	mirv.onGameEvent = function (e) {
		// mirv.message("onGameEvent: "+e.name+"("+e.id+") \""+e.data+"\"\n");
		if (null !== wsConnection && mirvFlags.gameEvents) {
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[5],
						data: e
					})
				);
			} catch (err) {
				mirv.warning('onGameEvent: Error while sending message:' + String(err) + '\n');
			}
		}
	};

	mirv.onCViewRenderSetupView = function (e) {
		if (null !== wsConnection && mirvFlags.cViewRenderSetupView) {
			lastView = e.lastView;
			try {
				wsConnection.send(
					JSON.stringify({
						type: events[6],
						data: e
					})
				);

				// we could flush and then wait for a reply here to set a view instantly, but don't understimate network round-trip time!
			} catch (err) {
				mirv.warning('onGameEvent: Error while sending message:' + String(err) + '\n');
			}
		}
		if (setView !== null) {
			return setView;
		}
	};
};
