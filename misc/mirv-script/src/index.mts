import { MirvJS } from './mirv/mirv.mjs';
import { makeEntityObject } from './mirv/utils.mjs';
import { handleMessages } from './mirv/ws-events-handler.mjs';
import { events } from './mirv/ws-events.mjs';
{
	MirvJS.init({
		host: 'localhost',
		port: 31337,
		path: 'mirv'
	});
	MirvJS.wsEnable = true;
	const debug = false;
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
						mirv.warning(
							'onClientFrameStageNotify: Error while handling incoming message:' +
								String(err) +
								'\n'
						);
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
		if (debug) mirv.message('onCViewRenderSetupView\n');
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
				mirv.warning(
					'onCViewRenderSetupView: Error while sending message:' + String(err) + '\n'
				);
			}
		}
		if (MirvJS.setView !== null) {
			return MirvJS.setView;
		}
	};

	const onGameEvent: mirv.OnGameEvent = (e) => {
		if (debug) mirv.message('onGameEvent\n');
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onGameEvent,
						data: e
					})
				);
			} catch (err) {
				mirv.warning('onGameEvent: Error while sending message:' + String(err) + '\n');
			}
		}
	};

	const onAddEntity: mirv.OnEntityEvent = (e, h) => {
		if (debug) mirv.message('onAddEntity\n');
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onAddEntity,
						data: makeEntityObject(e, h)
					})
				);
			} catch (err) {
				mirv.warning('onAddEntity: Error while sending message:' + String(err) + '\n');
			}
		}
	};
	const onRemoveEntity: mirv.OnEntityEvent = (e, h) => {
		if (debug) mirv.message('onRemoveEntity\n');
		if (MirvJS.ws !== null) {
			try {
				MirvJS.ws.send(
					JSON.stringify({
						type: events.onRemoveEntity,
						data: makeEntityObject(e, h)
					})
				);
			} catch (err) {
				mirv.warning('onRemoveEntity: Error while sending message:' + String(err) + '\n');
			}
		}
	};
}
