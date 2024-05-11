import { SimpleWebSocket } from 'simple-websockets';
import { MirvMessage, events } from './server';

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
	/** Disable transimission of game events */
	disableGameEvents() {
		this.send({ type: 'gameEvents', data: false });
		this.ws.removeAllListeners(events[5]);
	}
	/** Enable transimission of game events */
	enableGameEvents(callback: (gameEvents: mirv.GameEvent) => void) {
		this.send({ type: 'gameEvents', data: true });
		this.ws.on(events[5], callback);
	}
	/** Disable transimission of cViewRenderSetupView */
	disableCViewRenderSetupView() {
		this.send({ type: 'cViewRenderSetupView', data: false });
		this.ws.removeAllListeners(events[6]);
	}
	/** Enable transimission of cViewRenderSetupView */
	enableCViewRenderSetupView(callback: (view: mirv.OnCViewRenderSetupViewArgs) => void) {
		this.ws.on(events[6], callback);
		this.send({ type: 'cViewRenderSetupView', data: true });
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
}, 1000);
