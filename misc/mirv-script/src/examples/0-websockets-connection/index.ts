// Purpose:
// Abstract websockets connection implementaton,
// so it can be copy pasted or imported and used more conveniently.
//
// See mirv.d.ts for remarks regarding ws objects.

export class MirvWsConnection {
	private wsIn: mirv.WsIn | null = null;
	private wsOut: mirv.WsOut | null = null;
	private wsInBuffer: (string | ArrayBuffer)[] = [];
	private exception: unknown = null;
	private onException: ((e: unknown) => void) | null = null;
	private isClosed = false;
	private _isConnecting = false;

	/**
	 * @param onException Optional, onException listener.
	 */
	constructor(onException?: (e: unknown) => void) {
		if (onException) this.onException = onException;
		this.readNext = this.readNext.bind(this);
	}

	/**
	 * @param address Websockets address to connect to e.g. `ws://localhost:31337`
	 */
	async connect(address: string) {
		if (this.isConnecting || this.isConnected()) return;

		this._isConnecting = true;
		this.wsIn = null;
		this.wsOut = null;
		this.exception = null;
		this.wsInBuffer = [];

		try {
			const ws = await mirv.connect_async(address);
			this.wsIn = ws.in;
			this.wsOut = ws.out;
			this._isConnecting = false;
			this.isClosed = false;
			this.readNext();
		} catch (e) {
			this._isConnecting = false;
			this.setException(e);
		}
	}

	get isConnecting() {
		return this._isConnecting;
	}

	isConnected() {
		return null === this.exception && null !== this.wsOut && !this.isClosed;
	}

	private setException(e: unknown) {
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

	next() {
		if (0 < this.wsInBuffer.length) return this.wsInBuffer.shift();
		return null;
	}

	private async readNext() {
		if (!this.wsIn) return;
		try {
			while (true) {
				const message = await this.wsIn.next();
				if (message === null) {
					this.wsIn.drop();
					this.wsIn = null;
					if (this.wsOut) {
						this.wsOut.drop();
						this.wsOut = null;
					}
					this.isClosed = true;
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
						throw 'WsConnection: Unknown incoming message type.';
				}
			}
		} catch (e) {
			this.setException(e);
			try {
				if (this.wsIn) this.wsIn.drop();
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

	private async withOut(fn: (out: mirv.WsOut) => Promise<void>) {
		try {
			if (this.wsOut) await fn(this.wsOut);
		} catch (e) {
			this.setException(e);
		}
	}

	async send(msg: string | ArrayBuffer) {
		return await this.withOut((out) => out.send(msg));
	}

	async feed(msg: string | ArrayBuffer) {
		return await this.withOut((out) => out.feed(msg));
	}

	async flush() {
		return await this.withOut((out) => out.flush());
	}

	async close() {
		return await this.withOut((out) => out.close());
	}
}
