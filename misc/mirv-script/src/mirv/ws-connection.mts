interface IWsConnectionOptions {
	onException?: (e: unknown) => void;
	address: string;
}

export interface IWsConnection extends Omit<mirv.WsOut, 'drop'> {
	closed: boolean;
	exception: unknown;
	onException: IWsConnectionOptions['onException'];
	wsOut: mirv.WsOut | null;
	wsOutResolve: ((e?: unknown) => void) | null;
	wsOutReject: ((e: unknown) => void) | null;
	wsOutNext: Promise<unknown>;
	wsInBuffer: (string | ArrayBuffer)[];
	readNext: (wsIn: mirv.WsIn) => Promise<void>;
	setException: (e: unknown) => void;
	getException: () => unknown;
	hasException: () => boolean;
	isConnected: () => boolean;
	isClosed: () => boolean;
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

	async close() {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.close().catch((e) => this.setException(e));
		});
	}

	async flush() {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.flush().catch((e) => this.setException(e));
		});
	}

	async feed(data: string | ArrayBuffer) {
		this.wsOutNext.then(() => {
			if (this.wsOut) this.wsOut.feed(data).catch((e) => this.setException(e));
		});
	}

	async send(data: string | ArrayBuffer) {
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

	async readNext(wsIn: mirv.WsIn) {
		try {
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
