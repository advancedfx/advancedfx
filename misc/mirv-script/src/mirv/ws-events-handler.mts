import { MirvJS } from './mirv.mjs';
import { EntityObject, makeEntityObject } from './utils.mjs';
import { MirvEvents, events } from './ws-events.mjs';

export const handleMessages = (
	message: unknown,
	handlers: {
		onGameEvent?: mirv.OnGameEvent;
		onCViewRenderSetupView?: mirv.OnCViewRenderSetupView;
		onAddEntity?: mirv.OnEntityEvent;
		onRemoveEntity?: mirv.OnEntityEvent;
	}
) => {
	if (MirvJS.ws === null) return;
	if (typeof message !== 'string') {
		MirvJS.sendWarning(
			'TypeError in onClientFrameStageNotify: Warning: Unhandled incoming message of type: ' +
				typeof message
		);
		return;
	}

	const messageObj = JSON.parse(message) as {
		type: MirvEvents;
		data: string | boolean | mirv.OnCViewRenderSetupViewSet;
	};

	switch (messageObj.type) {
		case 'listTypes':
			mirv.message('Available types:\n' + Object.values(events).join('\n') + '\n');
			MirvJS.ws.send(JSON.stringify({ type: events.listTypes, data: Object.values(events) }));
			break;
		case 'exec':
			if (typeof messageObj.data === 'string') {
				mirv.exec(messageObj.data);
			} else {
				MirvJS.sendWarning('TypeError in exec: expected string');
			}
			break;
		case 'getLastView':
			MirvJS.ws.send(
				JSON.stringify({
					type: events.getLastView,
					data: MirvJS.lastView
				})
			);
			break;
		case 'setView':
			if (messageObj.data === null) {
				MirvJS.setView = null;
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
				MirvJS.setView = messageObj.data;
			} else {
				MirvJS.sendWarning(
					'TypeError in setView: expected object with x,y,z,rX,rY,rZ,fov or null'
				);
			}
			break;
		case 'quit':
			MirvJS.wsEnable = false;
			MirvJS.ws.close();
			mirv.exec('quit');
			break;
		case 'setGameEvents':
			if (typeof messageObj.data === 'boolean') {
				messageObj.data
					? (mirv.onGameEvent = handlers.onGameEvent)
					: (mirv.onGameEvent = undefined);
			} else {
				MirvJS.sendWarning('TypeError in gameEvents: expected boolean');
			}
			break;
		case 'setCViewRenderSetupView':
			if (typeof messageObj.data === 'boolean') {
				messageObj.data
					? (mirv.onCViewRenderSetupView = handlers.onCViewRenderSetupView)
					: (mirv.onCViewRenderSetupView = undefined);
			} else {
				MirvJS.sendWarning('TypeError in cViewRenderSetupView: expected boolean');
			}
			break;

		case 'setEntityEvents':
			if (typeof messageObj.data === 'boolean') {
				messageObj.data
					? (mirv.onAddEntity = handlers.onAddEntity)
					: (mirv.onAddEntity = undefined);
				messageObj.data
					? (mirv.onRemoveEntity = handlers.onRemoveEntity)
					: (mirv.onRemoveEntity = undefined);
			} else {
				MirvJS.sendWarning('TypeError in entityEvents: expected boolean');
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
				MirvJS.ws.send(
					JSON.stringify({
						type: events.listEntities,
						data: entities
					})
				);
			}
			break;
		case 'listPlayerEntities':
			{
				const entities: EntityObject[] = [];
				const highest = mirv.getHighestEntityIndex();
				for (let i = 0; i < highest + 1; i++) {
					const entity = mirv.getEntityFromIndex(i);
					if (null === entity || (!entity.isPlayerPawn() && !entity.isPlayerController()))
						continue;
					entities.push(makeEntityObject(entity));
				}
				const msg = [
					[
						'name',
						'debugName',
						'pawnHandle',
						'pawnIndex',
						'pawnSerial',
						'controllerHandle',
						'controllerIndex',
						'controllerSerial',
						'health'
					].join(' , '),
					entities
						.map((ent) =>
							[
								ent.name,
								ent.debugName,
								ent.playerPawnHandle,
								ent.playerPawnIndex,
								ent.playerPawnSerial,
								ent.playerControllerHandle,
								ent.playerControllerIndex,
								ent.playerControllerSerial,
								ent.health
							].join(' , ')
						)
						.join('\n')
				].join('\n');
				mirv.message(msg + '\n');
				MirvJS.ws.send(
					JSON.stringify({
						type: events.listPlayerEntities,
						data: entities
					})
				);
			}
			break;
		case 'loadModule':
			if (typeof messageObj.data !== 'string') {
				MirvJS.sendWarning('TypeError in loadModule: expected string');
				console.warn('TypeError in loadModule: expected string');
				break;
			}
			mirv.load(messageObj.data);
			mirv.run_jobs();
			break;
		default:
			MirvJS.sendWarning(
				'TypeError in onClientFrameStageNotify: Unknown incoming message.type:' +
					typeof messageObj.type
			);
			break;
	}
};
