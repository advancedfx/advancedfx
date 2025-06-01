// This is part of 4-advanced-websockets example.
// This file is meant to be run in node as server to use with
// index.mts script which is meant to be run in HLAE
//
// Purpose:
// Handle communication between server, HLAE and clients.
// In this example we do very basic auth in search params.
//
// We do handle hooks events for each client, based on their id.
// Also we disable hooks if given hook is not used by anyone.
// The other events are forwarded as is.
//
// We use simple-websockets library for convenience
// https://github.com/osztenkurden/simple-websockets
// It sends events in following format
// {
//    eventName: string,
//    values: unknown[]
// }
//
// See also client.ts

import http from 'http';
import { SimpleWebSocket } from 'simple-websockets';
import { SimpleWebSocketServer } from 'simple-websockets/server';
import { MIRV_EVENTS, MirvEventsMap } from './events.js';

const HOST = 'localhost';
const PORT = 31337;
const PATH = 'mirv';

function main() {
	const server = http.createServer();
	const wss = new SimpleWebSocketServer<MirvEventsMap>({
		server,
		path: '/' + PATH
	});

	let HLAE: SimpleWebSocket<MirvEventsMap> | null = null;

	const clients = new Map<
		string,
		{
			socket: SimpleWebSocket<MirvEventsMap>;
			hooks: {
				onViewSetup: boolean;
				onGameEvent: boolean;
				onAddEntity: boolean;
				onRemoveEntity: boolean;
			};
		}
	>();

	function registerHlae(socket: SimpleWebSocket<MirvEventsMap>) {
		console.log('HLAE has connected.');
		HLAE = socket;

		// socket._socket.addEventListener('error', (e) => {});
		socket.on('disconnect', () => {
			console.log('HLAE has disconnected.');
			HLAE = null;
		});

		for (const ev of MIRV_EVENTS) {
			// We skip these since they're intended for clients.
			if (
				ev === 'setAddEntity' ||
				ev === 'setRemoveEntity' ||
				ev === 'setGameEvent' ||
				ev === 'setOnCViewRenderSetupView'
			)
				continue;

			// Handle send data from hooks to subscribed clients.
			if (
				ev === 'onAddEntity' ||
				ev === 'onRemoveEntity' ||
				ev === 'onGameEvent' ||
				ev === 'onCViewRenderSetupView'
			) {
				socket.on(ev, (...data: unknown[]) => {
					clients.entries().forEach(([_id, { socket: client, hooks }]) => {
						if (ev === 'onAddEntity' && hooks.onAddEntity)
							client.send(ev, ...(data as MirvEventsMap['onAddEntity']));

						if (ev === 'onRemoveEntity' && hooks.onRemoveEntity)
							client.send(ev, ...(data as MirvEventsMap['onRemoveEntity']));

						if (ev === 'onGameEvent' && hooks.onGameEvent)
							client.send(ev, ...(data as MirvEventsMap['onGameEvent']));

						if (ev === 'onCViewRenderSetupView' && hooks.onViewSetup)
							client.send(ev, ...(data as MirvEventsMap['onCViewRenderSetupView']));
					});
				});

				continue;
			}

			// The rest of events
			socket.on(ev, (...data: unknown[]) => {
				clients.entries().forEach(([_id, { socket: s }]) => {
					s.send(ev, ...(data as MirvEventsMap[typeof ev]));
				});
			});
		}
	}

	function registerClient(socket: SimpleWebSocket<MirvEventsMap>, id: string) {
		if (clients.get(id)) {
			socket._socket.close(1011, `Client with id ${id} is already connected.`);
			return;
		}

		clients.set(id, {
			socket,
			hooks: {
				onViewSetup: false,
				onGameEvent: false,
				onAddEntity: false,
				onRemoveEntity: false
			}
		});

		console.log(`Client ${id} has connected.`);

		// socket._socket.addEventListener('error', (e) => {});
		socket.on('disconnect', () => {
			console.log(`Client ${id} has disconnected.`);
			clients.delete(id);

			if (!HLAE) return;

			// Disable hooks if the given one is not used by any client.
			for (const ev of [
				'setAddEntity',
				'setRemoveEntity',
				'setGameEvent',
				'setOnCViewRenderSetupView'
			] as const) {
				const someEnabled = clients.entries().some(([_id, { hooks }]) => {
					if (ev === 'setAddEntity') return hooks.onAddEntity;
					if (ev === 'setRemoveEntity') return hooks.onRemoveEntity;
					if (ev === 'setGameEvent') return hooks.onGameEvent;
					if (ev === 'setOnCViewRenderSetupView') return hooks.onViewSetup;

					return true;
				});
				if (!someEnabled) HLAE.send(ev, false);
			}
		});

		for (const ev of MIRV_EVENTS) {
			// We skip those as it's intended for HLAE, see above.
			if (
				ev === 'onAddEntity' ||
				ev === 'onRemoveEntity' ||
				ev === 'onGameEvent' ||
				ev === 'onCViewRenderSetupView'
			)
				continue;

			// Handle hooks on/off state.
			if (
				ev === 'setAddEntity' ||
				ev === 'setRemoveEntity' ||
				ev === 'setGameEvent' ||
				ev === 'setOnCViewRenderSetupView'
			) {
				socket.on(ev, (s) => {
					const thisClient = clients.get(id);
					if (thisClient) {
						if (ev === 'setAddEntity') thisClient.hooks.onAddEntity = s;
						if (ev === 'setRemoveEntity') thisClient.hooks.onRemoveEntity = s;
						if (ev === 'setGameEvent') thisClient.hooks.onGameEvent = s;
						if (ev === 'setOnCViewRenderSetupView') thisClient.hooks.onViewSetup = s;
						clients.set(id, thisClient);
					}

					if (!HLAE) return;

					if (s) {
						HLAE.send(ev, s);
					} else {
						const someEnabled = clients.entries().some(([_id, { hooks }]) => {
							if (ev === 'setAddEntity') return hooks.onAddEntity;
							if (ev === 'setRemoveEntity') return hooks.onRemoveEntity;
							if (ev === 'setGameEvent') return hooks.onGameEvent;
							if (ev === 'setOnCViewRenderSetupView') return hooks.onViewSetup;

							return true;
						});
						if (!someEnabled) HLAE.send(ev, s);
					}
				});

				continue;
			}

			// The rest of events
			socket.on(ev, (...data: unknown[]) => {
				if (HLAE) HLAE.send(ev, ...(data as MirvEventsMap[typeof ev]));
			});
		}
	}

	wss.onConnection((socket, req) => {
		const params = new URL(req.url || '', `ws://${req.headers.host}`).searchParams;

		if (params.has('hlae') && !HLAE) {
			registerHlae(socket);
			return;
		}

		if (params.has('clientId')) {
			const id = params.get('clientId');

			if (!id) {
				socket._socket.close(1011, 'Cannot get clientId from searchParams.');
				return;
			}

			registerClient(socket, id);
			return;
		}

		socket._socket.close(1000, 'No auth.');
	});

	server.listen(PORT, HOST, () => console.log(`Server listening on ${HOST}:${PORT}/${PATH}`));
}

main();
