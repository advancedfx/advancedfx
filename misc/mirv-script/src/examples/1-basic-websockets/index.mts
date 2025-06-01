// Purpose:
// Minimal example, showing how to handle messages over websockets.
//
// Usage:
// 1) Run server at specified below address.
// You can use the provided example server implementation in this folder
// or use your own, in such case adjust code accordingly if needed.
//
// 2) Load this script in game with mirv_script_load
// e.g. mirv_script_load "C:\advancedfx\misc\mirv-script\dist\examples\1-basic-websockets\index.mjs"
//
// 3) Expect message in game's console that HLAE was connected to the server.
//
// 4) Send "ping" string (without quotes) from to HLAE, expect "pong" in response.
//
// Note:
// Because we import class from another file we have to use .mjs extension for this one
// to treat it as module. Caveat is that modules are only evaluated once and then cached.
// If you want "hot reload" the script, then you have to use .js extension and do imports inline.
// See 3-command-snippet example.

import { MirvWsConnection } from '../0-websockets-connection/index.js';

{
	mirv.onClientFrameStageNotify = undefined;
	// We use search params to determine that it is HLAE that is trying to connect.
	// See also server.ts
	// This is very basic auth for demonstrating purpose.
	const WS_ADDRESS = 'ws://localhost:31337/mirv?hlae=1';
	const wsConn = new MirvWsConnection((e) => {
		console.trace();
		console.error(e);
	});

	let tickCounter = 0;
	let isFirstConnect = true;
	let isTimeout = false;
	let secsSinceConnectAttempt = 0;

	// Make sure this hook doesn't get overwritten elsewhere since currently HLAE doesn't handle such conflicts
	mirv.onClientFrameStageNotify = (e) => {
		// FRAME_START - called on host_frame (1 per tick).
		if (e.curStage == 0 && e.isBefore) {
			// Every 64 ticks (1 second) we try to restore the connection:
			if (!wsConn.isConnected() && tickCounter % 64 === 0) {
				isFirstConnect = true;
				secsSinceConnectAttempt++;

				// Give up after 10 seconds.
				// Note:
				// Since this file is a module, it's evaluated only once,
				// so it's up to you to implement restart logic or just remove timeout logic completely.
				// It's here only for demonstrating purpose.
				if (10 < secsSinceConnectAttempt && !isTimeout && !wsConn.isConnecting) {
					console.error(`Timeout: failed to connect to ${WS_ADDRESS}`);
					isTimeout = true;
				}

				if (!isTimeout && !wsConn.isConnecting) {
					console.info(`Trying to connect to ${WS_ADDRESS}`);
					wsConn.connect(WS_ADDRESS);
				}
			} else if (wsConn.isConnected()) {
				// Send message to game's console once connected
				if (isFirstConnect) {
					isFirstConnect = false;
					isTimeout = false;
					secsSinceConnectAttempt = 0;
					console.log(`Connected to ${WS_ADDRESS}`);
				}
				// Flush any messages that are lingering
				wsConn.flush();
				// Handle messages that came in meanwhile
				for (let message = wsConn.next(); message !== null; message = wsConn.next()) {
					try {
						// You can handle messages in separate function like we do in advanced example
						if (message === 'ping') wsConn.send('pong');
					} catch (err) {
						console.error(
							'onClientFrameStageNotify: Error while handling incoming message:',
							err
						);
						console.trace();
					}
				}
			}

			tickCounter++;

			// We use this to request an extra processing of jobs from HLAE.
			// Currently by default it only proccesses jobs upon after FRAME_RENDER_END == 6
			mirv.run_jobs();
			mirv.run_jobs_async();
		}

		// FRAME_RENDER_END - this is not called when demo is paused (can be multiple per tick).
		if (e.curStage === 6 && e.isBefore) {
			if (wsConn.isConnected()) wsConn.flush();
		}
	};
}
