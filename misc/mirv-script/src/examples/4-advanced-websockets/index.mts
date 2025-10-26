// Purpose:
// Advanced example demonstrating communication between HLAE/server and server/clients.
// In this example we use simple-websockets library, which is tiny wrapper for convenience
// https://github.com/osztenkurden/simple-websockets
//
// See example in 1-basic-websockets folder to understand the basics.
// This example doesn't contain previous comments.
//
// Usage:
// 1) Run provided server implementation (server.js) in node.
//
// 2) Load this script in game with mirv_script_load
// e.g. mirv_script_load "C:\advancedfx\misc\mirv-script\dist\examples\4-advanced-websockets\index.mjs"
//
// 3) Expect message that HLAE was connected to the server.
//
// 4) Run provided client implementation (client.js) in node and follow instructions there.
//
// 5) Expect commands from client reach the server/HLAE to execute the events.

import { MirvWsConnection } from '../0-websockets-connection/index.js';
import { handleMessage } from './mirv-handler.js';

{
	mirv.onClientFrameStageNotify = undefined;
	const WS_ADDRESS = 'ws://localhost:31337/mirv?hlae=1';
	const wsConn = new MirvWsConnection((e) => console.error(e));

	let tickCounter = 0;
	let isFirstConnect = true;

	// Make sure this hook doesn't get overwritten elsewhere since currently HLAE doesn't handle such conflicts
	mirv.onClientFrameStageNotify = (e) => {
		if (e.curStage === SOURCESDK_CS2.ClientFrameStage_t.FRAME_START && e.isBefore) {
			if (!wsConn.isConnected() && tickCounter % 64 === 0) {
				isFirstConnect = true;

				if (!wsConn.isConnecting) {
					console.info(`Trying to connect to ${WS_ADDRESS}`);
					wsConn.connect(WS_ADDRESS);
				}
			} else if (wsConn.isConnected()) {
				if (isFirstConnect) {
					isFirstConnect = false;
					console.log(`Connected to ${WS_ADDRESS}`);
				}

				wsConn.flush();

				for (let message = wsConn.next(); message !== null; message = wsConn.next()) {
					try {
						if (message) handleMessage(wsConn, message);
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

			mirv.run_jobs();
			mirv.run_jobs_async();
		}

		if (e.curStage === SOURCESDK_CS2.ClientFrameStage_t.FRAME_RENDER_PASS && e.isBefore) {
			if (wsConn.isConnected()) wsConn.flush();
		}
	};
}
