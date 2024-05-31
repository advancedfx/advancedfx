import {
	MirvHook,
	onCViewRenderSetupViewHookFn,
	onEntityHookFn,
	onGameEventHookFn
} from './utils.mjs';
import { IWsConnection, WsConnection } from './ws-connection.mjs';
import { events } from './ws-events.mjs';
// only one instance of MirvJS is allowed
export class MirvJS {
	private static instance: MirvJS;
	private static _tickCount: number = 0;
	private static _wsConnection: IWsConnection | null = null;
	private static _wsAddress: string = 'ws://localhost:31337/mirv';
	private static _wsEnable: boolean = false;
	private static _setView: mirv.OnCViewRenderSetupViewSet | null = null;
	private static _lastView: mirv.OnCViewRenderSetupViewArgs['lastView'] | null = null;
	static hooks = {
		onGameEvent: new MirvHook<onGameEventHookFn>(),
		onCViewRenderSetupView: new MirvHook<onCViewRenderSetupViewHookFn>(),
		onAddEntity: new MirvHook<onEntityHookFn>(),
		onRemoveEntity: new MirvHook<onEntityHookFn>()
	};
	private constructor() {}
	/**
	 * Initialize MirvJS.
	 *
	 * @param wsAddress - The address to connect to. Default: `ws://localhost:31337/mirv`
	 *
	 */
	static init(wsAddress?: { host: string; port: number; path: string }) {
		if (!MirvJS.instance) {
			MirvJS.instance = new MirvJS();
		}
		if (wsAddress)
			this._wsAddress = `ws://${wsAddress.host}:${wsAddress.port}/${wsAddress.path}?hlae=1`;
		return MirvJS.instance;
	}
	/**
	 * Websocket connection.
	 */
	static get ws() {
		return this._wsConnection;
	}
	/**
	 * Internal tick count.
	 */
	static get tick() {
		return this._tickCount;
	}
	static set tick(value: number) {
		this._tickCount = value;
	}
	/**
	 * Render view.
	 */
	static get setView() {
		return this._setView;
	}
	static set setView(value: mirv.OnCViewRenderSetupViewSet | null) {
		this._setView = value;
	}
	/**
	 * Last calculated render view.
	 */
	static get lastView() {
		return this._lastView;
	}

	static set lastView(value: mirv.OnCViewRenderSetupViewArgs['lastView'] | null) {
		this._lastView = value;
	}
	/**
	 * Enable/disable websocket connection.
	 */
	static get wsEnable() {
		return this._wsEnable;
	}

	static set wsEnable(value: boolean) {
		this._wsEnable = value;
	}
	/**
	 * Connect/reconnect to the websocket server.
	 *
	 * @remarks
	 * Use it in `mirv.onClientFrameStageNotify` to run on every tick.
	 *
	 */
	static connect() {
		// Restore websockett connection:
		if (this._wsConnection === null || this._wsConnection.hasException()) {
			if (this._wsConnection !== null) {
				mirv.warning(
					'onClientFrameStageNotify: wsConnection failed: ' +
						String(this._wsConnection.getException()) +
						'\n'
				);
				this._wsConnection.close();
				this._wsConnection = null;
				this.setView = null;
			}
			// Every 64 ticks we try to restore the connection:
			if (this._wsEnable && this._tickCount % 64 === 0) {
				mirv.message(
					'onClientFrameStageNotify: making new wsConnection: ' + this._wsAddress + '\n'
				);
				this._wsConnection = new WsConnection({
					address: this._wsAddress
				});
			}
		}
		// Close websocket connection if disabled
		if (this._wsConnection !== null && !this._wsEnable) {
			this._wsConnection.close();
			this._wsConnection = null;
		}
		// We use this to request an extra processing of jobs from HLAE (currently by default it only proccesses jobs upon after FRAME_RENDER_END == 6)
		mirv.run_jobs();
		mirv.run_jobs_async();
	}
	/**
	 * Send a warning message to the game console and to the websocket server.
	 */
	static sendWarning = (msg: string) => {
		mirv.warning(msg + '\n');
		if (this._wsConnection)
			this._wsConnection.send(
				JSON.stringify({
					type: events.warning,
					data: msg
				})
			);
	};
}
