// Type definitions for AfxHookSource2 mirv-script
// Project: advancedfx

declare namespace mirv {
	/**
	 * Allows to handle client DLL FrameStageNotify events.
	 * Since HLAE 2.162.0
	 */
	let onClientFrameStageNotify: undefined | OnClientFrameStageNotify;

	/*
	 * Since HLAE 2.171.0
	 */
	let onRecordStart: undefined | OnRecordEnd;

	/*
	 * Since HLAE 2.171.0
	 */
	let onRecordEnd: undefined | OnRecordStart;

	/**
	 * Allows to handle game event system events.
	 * Since HLAE 2.162.0
	 */
	let onGameEvent: undefined | OnGameEvent;

	/**
	 * Allows to receive the currently calculated game view in client DLL and manipulate it as well.
	 * Since HLAE 2.162.0
	 */
	let onCViewRenderSetupView: undefined | OnCViewRenderSetupView;

	/**
	 * Called after an entity has been added.
	 * Since HLAE 2.163.0
	 */
	let onAddEntity: undefined | OnEntityEvent;

	/**
	 * Called before an entity is removed.
	 * Since HLAE 2.163.0
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
	 *
	 * Since HLAE 2.162.0
	 */
	function connect_async(address: string): Promise<{ in: WsIn; out: WsOut }>;

	/**
	 * Since HLAE 2.179.0
	 */
	function trace(msg: string): Array<string>;

	/**
	 * Write a warning to the game console.
	 * @param msg
	 *
	 * Since HLAE 2.162.0
	 */
	function warning(msg: string): void;

	/**
	 * Write a message to the game console.
	 * @param msg
	 *
	 * Since HLAE 2.162.0
	 */
	function message(msg: string): void;

	/**
	 * Execute command(s) in the game console.
	 * @param command
	 *
	 * Since HLAE 2.162.0
	 */
	function exec(command: string): void;

	/**
	 * Load a JavaScript module (.mjs) or execute script (.js).
	 * @param filePath - Full path to file to load.
	 *
	 * @remarks
	 * A promise is returned and the module is usually not resolved instantly,
	 * if you need instant resolve, you need to call run_jobs to resolve the promise faster.
	 *
	 * Since HLAE 2.164.0
	 */
	function load(filePath: string): Promise<void>;

	/**
	 * This allows to manually trigger processing of synchronous jobs (Promises).
	 *
	 * @remarks
	 * Currently HLAE triggers this automatically upon after FRAME_RENDER_PASS after onClientFrameStageNotify.
	 *
	 * Since HLAE 2.162.0
	 */
	function run_jobs(): void;

	/**
	 * This allows to manually trigger processing of asynchronous jobs (Futures) that turn into Promises eventually.
	 *
	 * @remarks
	 * Currently HLAE triggers this automatically upon after FRAME_RENDER_PASS after onClientFrameStageNotify.
	 *
	 * Since HLAE 2.162.0
	 */
	function run_jobs_async(): void;

	/**
	 * Since HLAE 2.163.0
	 */
	function makeHandle(entryIndex: number, serialNumber: number): number;

	/**
	 * Since HLAE 2.163.0
	 */
	function isHandleValid(handle: number): boolean;

	/**
	 * Since HLAE 2.163.0
	 */
	function getHandleEntryIndex(handle: number): number;

	/**
	 * Since HLAE 2.163.0
	 */
	function getHandleSerialNumber(handle: number): number;

	/**
	 * Since HLAE 2.163.0
	 */
	function getHighestEntityIndex(): number;

	/**
	 * Since HLAE 2.163.0
	 */
	function getEntityFromIndex(index: number): null | Entity;

	function getEntityFromSplitScreenPlayer(index: number): null | Entity;

	/**
	 * Since HLAE 2.169.0
	 */
	function getMainCampath(): AdvancedfxCampath;

	/**
	 * Returns current game time.
	 * Since HLAE 2.183.0
	 */
	function getCurTime(): number;

	/**
	 * Returns current demo time in seconds.
	 *
	 * @remarks
	 * Should not be used, when no demo is playing.
	 * Can return undefined or negative values.
	 * Since HLAE 2.183.0
	 */
	function getDemoTime(): number | undefined;

	/**
	 * Returns current demo tick.
	 *
	 * @remarks
	 * Should not be used, when no demo is playing.
	 * Can return undefined or negative values.
	 * Since HLAE 2.183.0
	 */
	function getDemoTick(): number | undefined;

	/**
	 * Since HLAE 2.171.1
	 */
	function isPlayingDemo(): boolean;

	/**
	 * Since HLAE 2.171.1
	 */
	function isDemoPaused(): boolean;

	/**
	 * Represents a binary data message.
	 * Since HLAE 2.162.0
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
	 * Since HLAE 2.162.0
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
	 * Since HLAE 2.162.0
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
	 * Since HLAE 2.162.0
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
	 * FRAME_UNDEFINED		// (haven't run any frames yet)
	 * FRAME_RENDER_PASS	// Render a frame for display
	 * There are more values in-between, but their meanings have changed and we did not confirm them yet.
	 * See values in prop.d.ts
	 *
	 * @param e - curStage - current stage, isBefore - if called before (true) or after (false) client DLL for this stage.
	 *
	 * Since HLAE 2.162.0
	 */
	type OnClientFrameStageNotify = (e: { curStage: number; isBefore: boolean }) => void;

	/*
	 * Since HLAE 2.171.0
	 */
	type OnRecordStart = (e: RecordStart) => void;

	/*
	 * Since HLAE 2.171.0
	 */
	type OnRecordEnd = () => void;

	/*
	 * Since HLAE 2.162.0
	 */
	type OnGameEvent = (e: GameEvent) => void;

	/*
	 * Since HLAE 2.162.0
	 */
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
	 * Since HLAE 2.162.0
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
	 * Since HLAE 2.162.0
	 */
	type OnCViewRenderSetupView = (
		e: OnCViewRenderSetupViewArgs
	) => undefined | OnCViewRenderSetupViewSet;

	/**
	 * An entity reference.
	 * Since HLAE 2.163.0
	 */
	class Entity {
		/**
		 * If the refernce is still valid.
		 * Since HLAE 2.163.0
		 */
		isValid(): boolean;
		/**
		 * Since HLAE 2.163.0
		 */
		getName(): string;
		/**
		 * Since HLAE 2.163.0
		 */
		getDebugName(): null | string;
		/**
		 * Since HLAE 2.163.0
		 */
		getClassName(): string;
		/*
		 * Since HLAE 2.171.0
		 */
		getClientClassName(): null | string;
		/**
		 * Since HLAE 2.163.0
		 */
		isPlayerPawn(): boolean;
		/**
		 * Since HLAE 2.163.0
		 */
		getPlayerPawnHandle(): number;
		/**
		 * Since HLAE 2.163.0
		 */
		isPlayerController(): boolean;
		/**
		 * Since HLAE 2.163.0
		 */
		getPlayerControllerHandle(): number;
		/**
		 * Since HLAE 2.163.0
		 */
		getHealth(): number;

		/*
		 * Since HLAE 2.170.0
		 */
		getTeam(): number;

		/**
		 * @returns Array with x,y,z.
		 * Since HLAE 2.163.0
		 */
		getOrigin(): number[];

		/**
		 * @returns Array with x,y,z.
		 * Since HLAE 2.163.0
		 */
		getRenderEyeOrigin(): number[];

		/**
		 * @returns Array with x,y,z.
		 * Since HLAE 2.163.0
		 */
		getRenderEyeAngles(): number[];

		/**
		 * @remarks makes sense only on PlayerPawn.
		 * Since HLAE 2.162
		 */
		getViewEntityHandle(): number;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 * Since HLAE 2.171.0
		 */
		getActiveWeaponHandle(): number;

		/**
		 * @remarks makes sense only on PlayerController.
		 * Since HLAE 2.171.0
		 */
		getPlayerName(): null | string;

		/**
		 * @remarks makes sense only on PlayerController.
		 * @returns 64 bit steam id.
		 * Since HLAE 2.171.0
		 */
		getSteamId(): bigint;

		/**
		 * @remarks makes sense only on PlayerController.
		 * Since HLAE 2.171.0
		 */
		getSanitizedPlayerName(): null | string;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 * Since HLAE 2.171.0
		 */
		getObserverMode(): number;

		/**
		 * @remarks makes sense only on PlayerPawn.
		 * Since HLAE 2.171.0
		 */
		getObserverTargetHandle(): number;

		/**
		 * @remarks makes sense only on entities that have attachment points e.g player models (Pawns), weapons.
		 * Attachments names could be found when inspecting models files via Source 2 Viewer.
		 * Since HLAE 2.187.2
		 */
		getAttachment(name: string): {
			position: AdvancedfxMathVector3;
			angles: AdvancedfxMathQuaternion;
		} | null;
	}

	/**
	 * Since HLAE 2.163.0
	 */
	type OnEntityEvent = (entity: Entity, handle: number) => void;
}
