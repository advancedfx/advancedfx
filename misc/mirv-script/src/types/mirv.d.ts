// Type definitions for AfxHookSource2 mirv-script
// Project: advancedfx

declare namespace mirv {
	/**
	 * Allows to handle client DLL FrameStageNotify events.
	 */
	let onClientFrameStageNotify: undefined | OnClientFrameStageNotify;

	let onRecordStart: undefined | OnRecordEnd;

	let onRecordEnd: undefined | OnRecordStart;

	/**
	 * Allows to handle game event system events.
	 */
	let onGameEvent: undefined | OnGameEvent;

	/**
	 * Allows to receive the currently calculated game view in client DLL and manipulate it as well.
	 */
	let onCViewRenderSetupView: undefined | OnCViewRenderSetupView;

	/**
	 * Called after an entity has been added.
	 */
	let onAddEntity: undefined | OnEntityEvent;

	/**
	 * Called before an entity is removed.
	 */
	let onRemoveEntity: undefined | OnEntityEvent;

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
	 * @returns Promise that resolves with .in and an .out dictionary to acccess the in and outgoing streams respectively, or rejects with Error if connection fails.
	 */
	function connect_async(address: string): Promise<{ in: WsIn; out: WsOut }>;

	function trace(msg: string): Array<string>;

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
	 * Load a JavaScript module (.mjs) or execute script (.js).
	 * @param filePath - Full path to file to load.
	 *
	 * @remarks
	 * A promise is returned and the module is usually not resolved instantly,
	 * if you need instant resolve, you need to call run_jobs to resolve the promise faster.
	 */
	function load(filePath: string): Promise<void>;

	/**
	 * This allows to manually trigger processing of synchronous jobs (Promises).
	 *
	 * @remarks
	 * Currently HLAE triggers this automatically upon after FRAME_RENDER_PASS after onClientFrameStageNotify.
	 */
	function run_jobs(): void;

	/**
	 * This allows to manually trigger processing of asynchronous jobs (Futures) that turn into Promises eventually.
	 *
	 * @remarks
	 * Currently HLAE triggers this automatically upon after FRAME_RENDER_PASS after onClientFrameStageNotify.
	 */
	function run_jobs_async(): void;

	function makeHandle(entryIndex: number, serialNumber: number): number;

	function isHandleValid(handle: number): boolean;

	function getHandleEntryIndex(handle: number): number;

	function getHandleSerialNumber(handle: number): number;

	function getHighestEntityIndex(): number;

	function getEntityFromIndex(index: number): null | Entity;

	function getEntityFromSplitScreenPlayer(index: number): null | Entity;

	function getMainCampath(): AdvancedfxCampath;

	/**
	 * Returns current game time.
	 */
	function getCurTime(): number;

	/**
	 * Returns current demo time in seconds.
	 *
	 * @remarks
	 * Should not be used, when no demo is playing.
	 * Can return undefined or negative values.
	 */
	function getDemoTime(): number | undefined;

	/**
	 * Returns current demo tick.
	 *
	 * @remarks
	 * Should not be used, when no demo is playing.
	 * Can return undefined or negative values.
	 */
	function getDemoTick(): number | undefined;

	function isPlayingDemo(): boolean;

	function isDemoPaused(): boolean;

	/**
	 * Represents a binary data message.
	 */
	class WsBinary {
		/**
		 * Returns the data.
		 *
		 * @remarks After this object must not be used any further.
		 *
		 * @throws Error
		 * if buffer has been already consumed.
		 */
		consume(): ArrayBuffer;

		/**
		 * Returns a copy of the data.
		 *
		 * @throws Error
		 * if buffer has been already consumed.
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
		 *
		 * @throws Error
		 * If asychronous ordering is void (called while an other async operation is still pending) or if already dropped.
		 */
		drop(): void;

		/**
		 * Waits for the next incoming message.
		 *
		 * @returns Promise that resolves with string for text message, WsBinary for binary messages and null if there's no more messages (connection has been closed) or rejects with Error if recieving message failed or if asychronous ordering is void (called while an other async operation is still pending) or if already dropped.
		 */
		next(): Promise<string | WsBinary | null>;
	}

	/**
	 * Outgoing websocket stream
	 */
	class WsOut {
		/**
		 * Send a close request message to the server.
		 *
		 * @returns Promise that resolves on success or rejects with Error if asychronous ordering is voided (called while an other async operation is still pending) or if already dropped.
		 */
		close(): Promise<void>;

		/**
		 * Drops the underlying object.
		 *
		 * @remarks After this object must not be used any further.
		 *
		 * @throws Error
		 * If asychronous ordering is voided (called while an other async operation is still pending) or if already dropped.
		 */
		drop(): void;

		/**
		 * Queues a message for sending but does not flush it.
		 * @param msg
		 * @returns Promise that resolves on success or rejects with Error if asychronous ordering is voided (called while an other async operation is still pending) or if already dropped.
		 */
		feed(msg: string | ArrayBuffer): Promise<void>;

		/**
		 * Flushes messages in the queue.
		 * @returns Promise that resolves on success or rejects with Error if asychronous ordering is voided (called while an other async operation is still pending) or if already dropped.
		 */
		flush(): Promise<void>;

		/**
		 * Queues a message for sending and flushes.
		 * @param msg
		 * @returns Promise that resolves on success or rejects with Error if asychronous ordering is voided (called while an other async operation is still pending) or if already dropped.
		 */
		send(msg: string | ArrayBuffer): Promise<void>;
	}

	type RecordStart = {
		/**
		 * @param takeFolder this will almost always be not null (only if take folder can't be converted to UTF-8, which shouldn't happen).
		 */
		takeFolder: null | string;
	};

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
	 * FRAME_UNDEFINED=-1,		// (haven't run any frames yet)
	 * FRAME_RENDER_PASS = 8	// Render a frame for display
	 * There are more values in-between, but their meanings have changed and we did not confirm them yet.
	 *
	 * @param e - curStage - current stage, isBefore - if called before (true) or after (false) client DLL for this stage.
	 */
	type OnClientFrameStageNotify = (e: { curStage: number; isBefore: boolean }) => void;

	type OnRecordStart = (e: RecordStart) => void;

	type OnRecordEnd = () => void;

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

	/**
	 * An entity reference.
	 */
	class Entity {
		/**
		 * If the refernce is still valid.
		 */
		isValid(): boolean;

		getName(): string;

		getDebugName(): null | string;

		getClassName(): string;

		getClientClassName(): null | string;

		isPlayerPawn(): boolean;

		getPlayerPawnHandle(): number;

		isPlayerController(): boolean;

		getPlayerControllerHandle(): number;

		getHealth(): number;

		getTeam(): number;

		/**
		 * @returns Array with x,y,z.
		 */
		getOrigin(): number[];

		/**
		 * @returns Array with x,y,z.
		 */
		getRenderEyeOrigin(): number[];

		/**
		 * @returns Array with x,y,z.
		 */
		getRenderEyeAngles(): number[];

		/**
		 * @remarks makes sense only on PlayerPawn.
		 */
		getViewEntityHandle(): number;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 */
		getActiveWeaponHandle(): number;

		/**
		 * @remarks makes sense only on PlayerController.
		 */
		getPlayerName(): null | string;

		/**
		 * @remarks makes sense only on PlayerController.
		 * @returns 64 bit steam id.
		 */
		getSteamId(): bigint;

		/**
		 * @remarks makes sense only on PlayerController.
		 */
		getSanitizedPlayerName(): null | string;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 */
		getObserverMode(): number;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 */
		getObserverTargetHandle(): number;
	}

	type OnEntityEvent = (entity: Entity, handle: number) => void;
}
