import { Mirv } from '../mirv/mirv';

declare const mirv: Mirv;

interface IWsConnectionOptions {
	onException?: (e: unknown) => void;
	address: string;
}

export interface WsInOut {
	next: () => Promise<string | Message>;
	drop: () => void;
	close: () => Promise<void>;
	flush: () => Promise<void>;
	feed: (message: Message | unknown) => Promise<void>;
	send: (message: unknown) => Promise<void>;
	hasNext: () => boolean;
}

interface Message {
	consume: () => string;
}

export interface IWsConnection {
	closed: boolean;
	exception: unknown;
	onException: IWsConnectionOptions['onException'];
	wsOut: WsInOut | null;
	wsOutResolve: ((e?: unknown) => void) | null;
	wsOutReject: ((e: unknown) => void) | null;
	wsOutNext: Promise<unknown>;
	wsInBuffer: (unknown | string | object)[];
	readNext: (wsIn: WsInOut) => Promise<void>;
	setException: (e: unknown) => void;
	getException: () => unknown;
	hasException: () => boolean;
	isConnected: () => boolean;
	isClosed: () => boolean;
	close: () => void;
	flush: () => void;
	feed: (message: Message | unknown) => void;
	send: (message: unknown) => void;
	next: () => string | object | unknown | null;
	hasNext: () => boolean;
}

export class WsConnection implements IWsConnection {
	closed: IWsConnection['closed'];
	exception: IWsConnection['exception'];
	onException: IWsConnectionOptions['onException'];
	wsOut: IWsConnection['wsOut'];
	wsOutResolve: IWsConnection['wsOutResolve'];
	wsOutReject: IWsConnection['wsOutReject'];
	wsOutNext: IWsConnection['wsOutNext'];
	wsInBuffer: IWsConnection['wsInBuffer'];

	constructor(options: IWsConnectionOptions) {
		this.closed = false;
		this.exception = null;
		this.onException = options.onException;
		this.wsOut = null;
		this.wsOutResolve = null;
		this.wsOutReject = null;
		this.wsOutNext = new Promise((resolve, reject) => {
			this.wsOutResolve = resolve;
			this.wsOutReject = reject;
		});
		this.wsInBuffer = [];
		this.readNext = this.readNext.bind(this);

		mirv.connect_async(options.address)
			.then((ws) => {
				this.wsOut = ws.out;
				this.readNext(ws.in);
				if (this.wsOutResolve) this.wsOutResolve();
			})
			.catch((e) => {
				this.setException(e);
				if (this.wsOutReject) this.wsOutReject(e);
			});
	}
	setException(e: unknown) {
		if (this.exception === null) {
			this.exception = e;
			if (this.onException) this.onException(e);
		}
	}

	getException() {
		return this.exception;
	}

	hasException() {
		return this.exception !== null;
	}

	isClosed() {
		return this.closed;
	}

	isConnected() {
		return null === this.exception && null !== this.wsOut && !this.isClosed;
	}

	close() {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.close().catch((e) => this.setException(e));
		});
	}

	flush() {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.flush().catch((e) => this.setException(e));
		});
	}

	feed(data: string | Message | unknown) {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.feed(data).catch((e) => this.setException(e));
		});
	}

	send(data: string | Message | unknown) {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.send(data).catch((e) => this.setException(e));
		});
	}

	next() {
		if (0 < this.wsInBuffer.length) return this.wsInBuffer.pop();
		return null;
	}

	hasNext() {
		return 0 < this.wsInBuffer.length;
	}

	async readNext(wsIn: WsInOut) {
		try {
			// eslint-disable-next-line no-constant-condition
			while (true) {
				const message = await wsIn.next();
				if (message === null) {
					wsIn.drop();
					if (this.wsOut) this.wsOut.drop();
					this.closed = true;
					break;
				}
				switch (typeof message) {
					case 'string':
						this.wsInBuffer.push(message);
						break;
					case 'object':
						this.wsInBuffer.push(message.consume());
						break;
					default:
						throw 'WsConnection: Unknown incomming message type.';
				}
			}
		} catch (e) {
			this.setException(e);
			try {
				wsIn.drop();
			} catch {
				//
			}
			try {
				if (this.wsOut) this.wsOut.drop();
			} catch {
				//
			}
		}
	}
}
