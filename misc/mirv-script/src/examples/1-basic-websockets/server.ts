// This is part of 1-basic-websockets example.
// This file is meant to be run in node.
//
// Purpose:
// Basic websockets server to use with script example in this folder
// See index.mts file in this folder for instructions

import http from 'http';
import { WebSocket, WebSocketServer } from 'ws';

const HOST = 'localhost';
const PORT = 31337;
const PATH = 'mirv';

function main() {
	let HLAE: WebSocket | null = null;
	const server = http.createServer();
	const wss = new WebSocketServer({ server, path: '/' + PATH });

	wss.on('connection', (ws, req) => {
		const params = new URL(req.url || '', `ws://${req.headers.host}`).searchParams;
		// Drop any other connection that is not HLAE
		// This is very basic auth that should not be used in production.
		if (!params.has('hlae')) {
			ws.close();
			return;
		}
		// Allow only one connection from HLAE.
		if (!HLAE) {
			HLAE = ws;
		} else {
			ws.close();
			return;
		}

		console.log('HLAE is connected.');

		setTimeout(() => {
			console.log('sent ping');
			ws.send('ping');
		}, 1000);

		ws.on('message', (data) => {
			const msg = typeof data === 'string' ? data : data.toString();
			console.log('Got', msg);
		});
		ws.on('close', (code, reason) =>
			console.log(`HLAE Connection closed: ${code} / ${reason}`)
		);
		ws.on('error', (e) => console.error(`HLAE Connection Error: ${e}`));
	});

	server.listen(PORT, HOST, () => console.log(`Server listening on ${HOST}:${PORT}/${PATH}`));
}

main();
