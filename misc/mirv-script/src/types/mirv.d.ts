// Type definitions for AfxHookSource2 mirv-script
// Project: advancedfx

declare namespace mirv {
	/**
	 * Allows to handle client DLL FrameStageNotify events.
	 */
	let onClientFrameStageNotify: undefined | OnClientFrameStageNotify;

	/**
	 * Allows to handle game event system events.
	 */
	let onGameEvent: undefined | OnGameEvent;

	/**
	 * Allows to receive the currently calculated game view in client DLL and manipulate it as well.
	 */
	let onCViewRenderSetupView: undefined | OnCViewRenderSetupView;

	/**
	 * Opens a websocket connection
	 *
	 * @remarks
	 * In order to close the connection timely, the in and out stream need to
	 * be dropped by calling their drop function. This should usually happen
	 * only after first calling close on the out stream, and then waiting
	 * for a null message on the in stream, after that it's save to drop both.
	 *
	 * @param address - The address to connect to. Example: ws://localhost:31337/mirv
	 * @returns An in and an out object to acccess the in and outgoing streams respectively.
	 */
	function connect_async(address: string): Promise<{ in: WsIn; out: WsOut }>;

	/**
	 * Write a warning to the game console.
	 * @param msg
	 */
	function warning(msg: string): void;

	/**
	 * Write a message to the game console.
	 * @param msg
	 */
	function message(msg: string): void;

	/**
	 * Execute command(s) in the game console.
	 * @param command
	 */
	function exec(command: string): void;

	/**
	 * This allows to manually trigger processing of synchronous jobs (Promises).
	 *
	 * @remarks
	 * Currrently HLAE triggers this automatically upon after FRAME_RENDER_END in onClientFrameStageNotify,
	 * which is not called by CS2 when the demo is paused. This subject to change.
	 */
	function run_jobs(): void;

	/**
	 * This allows to manually trigger processing of asynchronous jobs (Futures) that turn into Promises eventually.
	 *
	 * @remarks
	 * Currrently HLAE triggers this automatically upon after FRAME_RENDER_END in onClientFrameStageNotify,
	 * which is not called by CS2 when the demo is paused. This subject to change.
	 */
	function run_jobs_async(): void;

	/**
	 * Represents a binary data message.
	 */
	class WsBinary {
		/**
		 * Returns the data.
		 *
		 * @remarks After this object must not be used any further.
		 */
		consume(): ArrayBuffer;

		/**
		 * Returns a copy of the data.
		 */
		clone(): ArrayBuffer;
	}

	/**
	 * Incoming websocket stream
	 */
	class WsIn {
		/**
		 * Drops the underlying object.
		 *
		 * @remarks After this object must not be used any further.
		 */
		drop(): void;

		/**
		 * Waits for the next incoming message.
		 *
		 * @returns String for text message, WsBinary for binary messages and null if there's no more messages (connection has been closed).
		 */
		next(): Promise<string | WsBinary | null>;
	}

	/**
	 * Outgoing websocket stream
	 */
	class WsOut {
		/**
		 * Send a close request message to the server.
		 */
		close(): Promise<void>;

		/**
		 * Drops the underlying object.
		 *
		 * @remarks After this object must not be used any further.
		 */
		drop(): void;

		/**
		 * Queues a message for sending but does not flush it.
		 * @param msg
		 */
		feed(msg: string | ArrayBuffer): Promise<void>;

		/**
		 * Flushes messages in the queue.
		 */
		flush(): Promise<void>;

		/**
		 * Queues a message for sending and flushes.
		 * @param msg
		 */
		send(msg: string | ArrayBuffer): Promise<void>;
	}

	/**
	 * Game event data.
	 */
	type GameEvent = {
		/**
		 * Event ID.
		 */
		id: number;

		/**
		 * Event name.
		 */
		name: string;

		/**
		 * Event data as JSON string.
		 */
		data: string;
	};

	/**
	 * Called before and after the client DLL is notified about the current stage.
	 *
	 * @remarks
	 * curStage usually has one of the following values in CS2:
	 * FRAME_UNDEFINED = -1, // usually doesn't happen.
	 * FRAME_START = 0, // A new host frame (tick) is begun.
	 * FRAME_NET_UPDATE_START = 1 // Processing of network data is begun
	 * FRAME_NET_UPDATE_POSTDATAUPDATE_START = 2 // Data has been received and we're going to start calling PostDataUpdate
	 * FRAME_NET_UPDATE_POSTDATAUPDATE_END = 3, // Data has been received and we've called PostDataUpdate on all data recipients
	 * FRAME_NET_UPDATE_END = 4, // We've received all packets, we can now do interpolation, prediction, etc..
	 * FRAME_RENDER_START = 5, // We're about to start rendering the scene. Currently not called in CS2 when demo is paused!
	 * FRAME_RENDER_END = 6, // We've finished rendering the scene. Currently not called in CS2 when demo is paused!
	 *
	 * @param e - curStage - current stage, isBefore - if called before (true) or after (false) client DLL for this stage.
	 */
	type OnClientFrameStageNotify = (e: { curStage: number; isBefore: boolean }) => void;

	type OnGameEvent = (e: GameEvent) => void;

	type OnCViewRenderSetupViewArgs = {
		curTime: number;
		absTime: number;
		lastAbsTime: number;
		currentView: {
			x: number;
			y: number;
			z: number;
			rX: number;
			rY: number;
			rZ: number;
			fov: number;
		};
		gameView: {
			x: number;
			y: number;
			z: number;
			rX: number;
			rY: number;
			rZ: number;
			fov: number;
		};
		lastView: {
			x: number;
			y: number;
			z: number;
			rX: number;
			rY: number;
			rZ: number;
			fov: number;
		};
		width: number;
		height: number;
	};

	/**
	 * @remarks Unprovided members well remain at their original value.
	 */
	type OnCViewRenderSetupViewSet = {
		x?: number;
		y?: number;
		z?: number;
		rX?: number;
		rY?: number;
		rZ?: number;
		fov?: number;
	};

	/**
	 * @remarks Return nothing (undefined) to not manipulate the current view (return;).
	 */
	type OnCViewRenderSetupView = (
		e: OnCViewRenderSetupViewArgs
	) => undefined | OnCViewRenderSetupViewSet;
}
