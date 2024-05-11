import http from 'http';
import { SimpleWebSocket, SimpleWebSocketServer } from 'simple-websockets-server';
import { WebSocket } from 'ws';

export const events = [
	'listTypes',
	'quit',
	'exec',
	'getLastView',
	'setView',
	'gameEvents',
	'cViewRenderSetupView'
] as const;
export type MirvEvents = (typeof events)[number];

export type MirvMessage = {
	type: MirvEvents;
	data?: string | number | object | boolean;
};

const serverOptions = {
	host: 'localhost',
	port: 31337,
	path: '/mirv'
};

export class MirvServer {
	host: string;
	port: number;
	path: string;
	private users: Map<string, SimpleWebSocket>;
	private hlae: WebSocket | null;
	private server: http.Server;
	private wss: SimpleWebSocketServer;

	constructor(options: { host: string; port: number; path: string }) {
		this.host = options.host;
		this.port = options.port;
		this.path = options.path;
		this.users = new Map<string, SimpleWebSocket>();
		this.hlae = null;
		this.server = http.createServer();
		this.wss = new SimpleWebSocketServer({ server: this.server, path: this.path });
		// Accept here all users. Note it returns SimpleWebSocket.
		this.wss.onConnection((socket, request) => {
			const params = new URL(request.url || '', `wss://${request.headers.host}`).searchParams;
			if (params.has('user')) {
				const id = params.get('user');
				console.log(`User ${id} connected`);
				if (id) {
					const localUser = this.users.get(id);
					if (localUser) {
						localUser._socket.close();
						this.users.delete(id);
					}

					socket.on('disconnect', () => {
						console.log(`User ${id} disconnected`);
						this.users.delete(id);
					});

					socket.addListener('error', (e) => {
						socket._socket.close();
						this.users.delete(id);
						console.error('Error: ' + e);
					});

					for (const event of events) {
						socket.on(event, (data) => {
							if (this.hlae) this.hlae.send(JSON.stringify({ type: event, data }));
						});
					}

					this.users.set(id, socket);
				}
				return;
			}
		});
		// Accept here only hlae. Note it returns regular WebSocket.
		this.wss.on('connection', (socket, request) => {
			const params = new URL(request.url || '', `wss://${request.headers.host}`).searchParams;

			if (!params.has('user') && !params.has('hlae')) socket.close();
			if (!params.has('hlae')) return;

			if (this.hlae) this.hlae.close();
			this.hlae = socket;
			console.log('HLAE connected');

			this.hlae.on('message', (data) => {
				const msg = typeof data === 'string' ? data : data.toString();
				const msgObject = JSON.parse(msg) as MirvMessage;
				console.log(msg);
				if (events.includes(msgObject.type) || (msgObject.type as string) === 'warning') {
					if (msgObject.data) {
						this.users.forEach((user) => {
							user.send(msgObject.type, msgObject.data);
						});
					}
				}
			});

			this.hlae.on('close', function (code, reason) {
				console.log('HLAE Connection closed: ' + code.toString() + ' / ' + reason);
			});
			this.hlae.on('error', function (e) {
				console.error('Error: ' + e);
			});
		});
	}

	start() {
		this.server.listen(serverOptions.port, serverOptions.host);
		console.log(
			`${serverOptions.host} listening on port ${serverOptions.port}, path ${serverOptions.path} ...`
		);
	}

	stop() {
		this.wss.close((err) => {
			if (err) console.error(err);
		});
		this.server.close((err) => {
			if (err) console.error(err);
		});
	}
}

const server = new MirvServer(serverOptions);
server.start();
