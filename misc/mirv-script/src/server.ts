import http from 'http';
import { WebSocket, WebSocketServer } from 'ws';

const serverOptions = {
	host: 'localhost',
	port: 31337,
	path: '/mirv'
};

export class MirvServer {
	host: string;
	port: number;
	path: string;
	private users: Map<string, WebSocket>;
	private hlae: WebSocket | null;
	private server: http.Server;
	private wss: WebSocketServer;

	constructor(options: { host: string; port: number; path: string }) {
		this.host = options.host;
		this.port = options.port;
		this.path = options.path;
		this.users = new Map<string, WebSocket>();
		this.hlae = null;
		this.server = http.createServer();
		this.wss = new WebSocketServer({ server: this.server, path: this.path });
		this.wss.on('connection', (socket, request) => {
			console.log('/mirv	 connected');

			const params = new URL(request.url || '', `wss://${request.headers.host}`).searchParams;
			if (params.has('user')) {
				const id = params.get('user');
				console.log(`User ${id} connected`);
				if (id) {
					const localUser = this.users.get(id);
					if (localUser) {
						localUser.close();
						this.users.delete(id);
					}

					socket.on('message', (data) => {
						if (this.hlae) this.hlae.send(data.toString());
					});
					socket.on('close', () => {
						console.log(`User ${id} disconnected`);
						this.users.delete(id);
					});
					socket.on('error', (e) => {
						socket.close();
						console.error('Error: ' + e);
					});

					this.users.set(id, socket);
				}
				return;
			}

			if (this.hlae) this.hlae.close();
			this.hlae = socket;

			this.hlae.on('message', function (data) {
				if (typeof data === 'string') {
					console.log(data);
				}
				if (data instanceof Buffer) {
					console.log(data.toString());
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
		this.server.close((err) => {
			if (err) console.error(err);
		});
		this.wss.close((err) => {
			if (err) console.error(err);
		});
	}
}

const server = new MirvServer(serverOptions);
server.start();
