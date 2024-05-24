import { SimpleWebSocket } from 'simple-websockets';
import { MirvMessage } from './server';

const events = [
	'listTypes',
	'quit',
	'exec',
	'getLastView',
	'setView',
	'setGameEvents',
	'setCViewRenderSetupView',
	'setEntityEvents',
	'onCViewRenderSetupView',
	'onGameEvent',
	'onAddEntity',
	'onRemoveEntity',
	'warning'
] as const;

type ConnectionOptions = {
	host: string;
	port: number;
	path?: string;
	user: number | string;
};

export class MirvClient {
	private ws: SimpleWebSocket;

	/**
	 * @param path - path is optional, if not provided it will default to mirv.
	 */
	constructor({ host, port, path, user }: ConnectionOptions) {
		this.ws = new SimpleWebSocket(`ws://${host}:${port}/${path || 'mirv'}?user=${user}`);
		this.ws.on('warning', (message) => {
			console.log('warning:', message);
		});
	}

	send(message: MirvMessage) {
		this.ws.send(message.type, message.data);
	}
	/** List available types */
	getTypes(callback: (types: string[]) => void) {
		this.ws.once('listTypes', callback);
		this.send({ type: 'listTypes' });
	}
	/** Shorthand for quit */
	sendQuit() {
		this.send({ type: 'quit' });
	}
	/** Execute ingame command */
	sendExec(command: string) {
		this.send({ type: 'exec', data: command });
	}
	/** Enable transimission of game events */
	enableGameEvents(callback: (gameEvents: mirv.GameEvent) => void) {
		this.ws.on(events[9], callback);
		this.send({ type: 'setGameEvents', data: true });
	}
	/** Disable transimission of game events */
	disableGameEvents() {
		this.ws.removeAllListeners(events[9]);
		this.send({ type: 'setGameEvents', data: false });
	}
	/** Enable transimission of cViewRenderSetupView events */
	enableCViewRenderSetupView(callback: (view: mirv.OnCViewRenderSetupViewArgs) => void) {
		this.ws.on(events[8], callback);
		this.send({ type: 'setCViewRenderSetupView', data: true });
	}
	/** Disable transimission of cViewRenderSetupView events */
	disableCViewRenderSetupView() {
		this.ws.removeAllListeners(events[8]);
		this.send({ type: 'setCViewRenderSetupView', data: false });
	}
	/** Get last cached render view  */
	getLastView(callback: (view: mirv.OnCViewRenderSetupViewArgs['lastView']) => void) {
		this.ws.once(events[3], callback);
		this.send({ type: 'getLastView' });
	}
	/** Set render view */
	setView(view: mirv.OnCViewRenderSetupViewSet) {
		this.send({ type: 'setView', data: view });
	}
	/** Reset render view */
	resetView() {
		this.send({ type: 'setView' });
	}
	/** Enable transimission of entity events */
	enableEntityEvents(
		onAddEntity: (entity: mirv.Entity) => void,
		onRemoveEntity: (entity: mirv.Entity) => void
	) {
		this.ws.on(events[10], onAddEntity);
		this.ws.on(events[11], onRemoveEntity);
		this.send({ type: 'setEntityEvents', data: true });
	}
	/** Disable transimission of entity events */
	disableEntityEvents() {
		this.ws.removeAllListeners(events[10]);
		this.ws.removeAllListeners(events[11]);
		this.send({ type: 'setEntityEvents', data: false });
	}
}

const client = new MirvClient({ host: 'localhost', port: 31337, user: 1 });

// test that evertything works
setTimeout(() => {
	client.sendExec('echo test');
	client.getTypes((types) => {
		console.log(types);
	});
	setTimeout(() => {
		client.enableCViewRenderSetupView((view) => {
			console.log(view);
		});
	}, 5000);
	setTimeout(() => {
		client.disableCViewRenderSetupView();
	}, 5500);
	setTimeout(() => {
		client.getLastView((view) => {
			console.log(view);
		});
	}, 7000);

	// intentionally send wrong data
	setTimeout(() => {
		client.send({
			type: 'setView',
			data: 'test'
		});
	}, 8000);

	setTimeout(() => {
		client.enableEntityEvents(
			(entity) => {
				console.log(entity);
			},
			(entity) => {
				console.log(entity);
			}
		);
	}, 10000);
	setTimeout(() => {
		client.disableEntityEvents();
	}, 11000);
}, 1000);
