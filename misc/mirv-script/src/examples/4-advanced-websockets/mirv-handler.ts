// This is part of 4-advanced-websockets example.
// See entry point in index.mts
//
// Purpose:
// Define function to handle incoming messages from server.

import { ID } from './id.js';
import { EntityObject, MIRV_EVENTS, MirvEventsMap } from './events.js';
import { MirvWsConnection } from '../0-websockets-connection/index.js';

// From simple-websockets
// https://github.com/osztenkurden/simple-websockets/blob/master/src/util.ts
const convertMessageToEvent = (data: unknown): { eventName: string; values: unknown[] } | null => {
	if (!data) return null;
	if (typeof data !== 'string') return null;
	try {
		const dataObject = JSON.parse(data);
		if (!dataObject.eventName && typeof dataObject.eventName !== 'string') return null;
		if (dataObject.values && !Array.isArray(dataObject.values)) return null;
		return {
			eventName: dataObject.eventName,
			values: dataObject.values || []
		};
	} catch {
		return null;
	}
};

const makeEntityObj = (e: mirv.Entity, i?: number, h?: number): EntityObject => {
	return {
		idx: i !== undefined ? i : h ? mirv.getHandleEntryIndex(h) : -1,
		handle: h ? h : -1,
		debugName: e.getDebugName(),
		className: e.getClassName(),
		clientClassName: e.getClientClassName(),
		isPlayerPawn: e.isPlayerPawn(),
		isPlayerController: e.isPlayerController(),
		health: e.getHealth(),
		origin: e.getOrigin(),
		renderEyeOrigin: e.getRenderEyeOrigin(),
		renderEyeAngles: e.getRenderEyeAngles()
	};
};

const getEntities = (): EntityObject[] => {
	const res: EntityObject[] = [];
	for (let i = 0; i < mirv.getHighestEntityIndex() + 1; i++) {
		const entity = mirv.getEntityFromIndex(i);
		if (entity) res.push(makeEntityObj(entity, i));
	}
	return res;
};

export const handleMessage = (wsConn: MirvWsConnection, msg: unknown) => {
	const id = ID + '/mirv-handle-message';

	const ev = convertMessageToEvent(msg);
	if (!ev) return;
	if (!MIRV_EVENTS.includes(ev.eventName as (typeof MIRV_EVENTS)[number])) return;

	// Since we use simple-websockets we have to send it in specific format
	function sendMessage<T extends keyof MirvEventsMap>(eventName: T, values: MirvEventsMap[T]) {
		wsConn.send(
			JSON.stringify({
				eventName,
				values
			})
		);
	}

	// Just to correctly resolve types in TypeScript
	function getValue<T extends keyof MirvEventsMap>(eventName: T, values: unknown[]) {
		return values as MirvEventsMap[typeof eventName];
	}

	// Ideally we should do runtime checks, but we just assume data comes correctly
	const eventName = ev.eventName as (typeof MIRV_EVENTS)[number];
	switch (eventName) {
		case 'exec': {
			const [cmd] = getValue(eventName, ev.values);
			mirv.exec(cmd);
			break;
		}
		case 'gameConsoleLog': {
			const [message, isWarning] = getValue(eventName, ev.values);
			isWarning ? console.warn(message) : console.log(message);
			break;
		}
		case 'listEntities': {
			sendMessage(eventName, [getEntities()]);
			break;
		}
		case 'setOnCViewRenderSetupView': {
			const [state] = getValue(eventName, ev.values);
			if (state) {
				mirv.events.cViewRenderSetupView.on(
					id,
					(e: AdvancedfxMirv.Events.CViewRenderSetupViewEvent) => {
						sendMessage('onCViewRenderSetupView', [e]);
						// If we write additional event and manage the state,
						// then we can overwrite current view by returning it here
						return undefined;
					}
				);
			} else {
				mirv.events.cViewRenderSetupView.off(id);
			}
			break;
		}
		case 'setGameEvent': {
			const [state] = getValue(eventName, ev.values);
			if (state) {
				mirv.events.gameEvent.on(id, (e: AdvancedfxMirv.Events.GameEventEvent) => {
					sendMessage('onGameEvent', [e.id, e.name, e.data]);
					return undefined;
				});
			} else {
				mirv.events.gameEvent.off(id);
			}
			break;
		}
		case 'setAddEntity': {
			const [state] = getValue(eventName, ev.values);
			if (state) {
				mirv.events.addEntity.on(id, (e: AdvancedfxMirv.Events.EntityEvent) => {
					sendMessage('onAddEntity', [makeEntityObj(e.entity, undefined, e.handle)]);
					return undefined;
				});
			} else {
				mirv.events.addEntity.off(id);
			}
			break;
		}
		case 'setRemoveEntity': {
			const [state] = getValue(eventName, ev.values);
			if (state) {
				mirv.events.removeEntity.on(id, (e: AdvancedfxMirv.Events.EntityEvent) => {
					sendMessage('onRemoveEntity', [makeEntityObj(e.entity, undefined, e.handle)]);
					return undefined;
				});
			} else {
				mirv.events.removeEntity.off(id);
			}
			break;
		}

		default:
			console.warn('Got unknown event');
	}
};
