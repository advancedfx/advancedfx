import {
	firstOnGameEventHook,
	secondOnGameEventHook,
	thirdOnGameEventHook
} from './mirv/hooks.mjs';
import { MirvJS } from './mirv/mirv.mjs';
import { makeEntityObject } from './mirv/utils.mjs';
import { handleMessages } from './mirv/ws-events-handler.mjs';
import { events } from './mirv/ws-events.mjs';
{
	BigInt.prototype.toJSON = function () {
		return this.toString();
	};
	MirvJS.init({
		host: 'localhost',
		port: 31337,
		path: 'mirv'
	});
	MirvJS.wsEnable = true;
	const debug = false;
	// register hooks
	MirvJS.hooks.onGameEvent.push(firstOnGameEventHook);
	MirvJS.hooks.onGameEvent.push(secondOnGameEventHook);
	MirvJS.hooks.onGameEvent.push(thirdOnGameEventHook);
	// main logic defined here, it runs on every tick
	mirv.onClientFrameStageNotify = (e) => {
		// FRAME_START - called on host_frame (1 per tick).
		if (e.curStage == 0 && e.isBefore) {
			MirvJS.connect();
			if (MirvJS.ws !== null) {
				// Flush any messages that are lingering:
				MirvJS.ws.flush();
				// Handle messages that came in meanwhile:
				for (let message = MirvJS.ws.next(); message !== null; message = MirvJS.ws.next()) {
					try {
						handleMessages(message, {
							onGameEvent,
							onCViewRenderSetupView,
							onAddEntity,
							onRemoveEntity
						});
					} catch (err) {
						console.error(
							'onClientFrameStageNotify: Error while handling incoming message:',
							err
						);
						console.trace();
					}
				}
			}
			MirvJS.tick++;
		}
		// FRAME_RENDER_START - this is not called when demo is paused (can be multiple per tick).
		if (e.curStage === 5 && e.isBefore) {
			//
		}
		// FRAME_RENDER_END - this is not called when demo is paused (can be multiple per tick).
		// note: double check if it's 6
		if (e.curStage === 5 && e.isBefore) {
			if (MirvJS.ws !== null) MirvJS.ws.flush();
		}
	};
	// if set to undefined, then it's completely disabled (but can be turned on later, when defined)
	mirv.onCViewRenderSetupView = undefined;
	mirv.onGameEvent = undefined;
	mirv.onAddEntity = undefined;
	mirv.onRemoveEntity = undefined;
	// we define it here separately to assign (enable) later on demand
	const onCViewRenderSetupView: mirv.OnCViewRenderSetupView = (e) => {
		if (debug) console.debug('onCViewRenderSetupView');
		// pass data to hooks
		const hooksResult = MirvJS.hooks.onCViewRenderSetupView.runHooks(e);
		if (hooksResult) {
			// do something with the result
		}
		if (MirvJS.ws !== null) {
			MirvJS.lastView = e.lastView;
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onCViewRenderSetupView,
						data: e
					})
				);

				// we could flush and then wait for a reply here to set a view instantly, but don't understimate network round-trip time!
			} catch (err) {
				console.error('onCViewRenderSetupView: Error while sending message:', err);
				console.trace();
			}
		}
		if (MirvJS.setView !== null) {
			return MirvJS.setView;
		}
	};

	const onGameEvent: mirv.OnGameEvent = (e) => {
		if (debug) console.debug('onGameEvent');
		// example remove hook, dont do it like this though since it will trigger every time
		if (e.name === 'player_death') {
			MirvJS.hooks.onGameEvent.remove(firstOnGameEventHook);
			mirv.message('firstOnGameEventHook removed\n');
			MirvJS.hooks.onGameEvent.remove(secondOnGameEventHook);
			mirv.message('secondOnGameEventHook removed\n');
		}
		const hooksResult = MirvJS.hooks.onGameEvent.runHooks(e);
		if (hooksResult) {
			if (hooksResult.name === '42069' || hooksResult.name === 'player_hurt')
				mirv.message(`onGameEventHook: name: ${hooksResult.name}, id: ${hooksResult.id}\n`);
		}
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onGameEvent,
						data: e
					})
				);
			} catch (err) {
				console.error('onGameEvent: Error while sending message:', err);
				console.trace();
			}
		}
	};

	const onAddEntity: mirv.OnEntityEvent = (e, h) => {
		if (debug) console.debug('onAddEntity');
		const hooksResult = MirvJS.hooks.onAddEntity.runHooks(e, h);
		if (hooksResult !== undefined) {
			const [entity, handle] = hooksResult;
		}
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onAddEntity,
						data: makeEntityObject(e, h)
					})
				);
			} catch (err) {
				console.error('onAddEntity: Error while sending message:', err);
				console.trace();
			}
		}
	};

	const onRemoveEntity: mirv.OnEntityEvent = (e, h) => {
		if (debug) console.debug('onRemoveEntity');
		const hooksResult = MirvJS.hooks.onAddEntity.runHooks(e, h);
		if (hooksResult !== undefined) {
			const [entity, handle] = hooksResult;
		}
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onRemoveEntity,
						data: makeEntityObject(e, h)
					})
				);
			} catch (err) {
				console.error('onRemoveEntity: Error while sending message:', err);
				console.trace();
			}
		}
	};
}
