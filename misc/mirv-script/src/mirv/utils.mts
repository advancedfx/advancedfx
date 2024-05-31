export type EntityObject = {
	name: string;
	handle: number;
	debugName: string | null;
	className: string;
	isPlayerPawn: boolean;
	playerPawnHandle: number;
	playerPawnIndex: number;
	playerPawnSerial: number;
	isPlayerController: boolean;
	playerControllerHandle: number;
	playerControllerIndex: number;
	playerControllerSerial: number;
	health: number;
	origin: number[];
	renderEyeOrigin: number[];
	renderEyeAngles: number[];
};

export const makeEntityObject = (e: mirv.Entity, h?: number): EntityObject => {
	const playerPawnHandle = e.getPlayerPawnHandle();
	const playerControllerHandle = e.getPlayerControllerHandle();
	return {
		name: e.getName(),
		handle: h ? h : -1,
		debugName: e.getDebugName(),
		className: e.getClassName(),
		isPlayerPawn: e.isPlayerPawn(),
		playerPawnHandle: playerPawnHandle,
		playerPawnIndex: playerPawnHandle !== -1 ? mirv.getHandleEntryIndex(playerPawnHandle) : -1,
		playerPawnSerial:
			playerPawnHandle !== -1 ? mirv.getHandleSerialNumber(playerPawnHandle) : -1,
		isPlayerController: e.isPlayerController(),
		playerControllerHandle: playerControllerHandle,
		playerControllerIndex:
			playerControllerHandle !== -1 ? mirv.getHandleEntryIndex(playerControllerHandle) : -1,
		playerControllerSerial:
			playerControllerHandle !== -1 ? mirv.getHandleSerialNumber(playerControllerHandle) : -1,
		health: e.getHealth(),
		origin: e.getOrigin(),
		renderEyeOrigin: e.getRenderEyeOrigin(),
		renderEyeAngles: e.getRenderEyeAngles()
	};
};

type MirvHookFn<R, A> = (
	lastResult: R | undefined,
	...args: Array<A>
) => { result: R | undefined; abort: boolean };

export type onGameEventHookFn = MirvHookFn<mirv.GameEvent, Parameters<mirv.OnGameEvent>>;

export type onCViewRenderSetupViewHookFn = MirvHookFn<
	mirv.OnCViewRenderSetupViewSet | undefined,
	Parameters<mirv.OnCViewRenderSetupView>
>;

export type onEntityHookFn = MirvHookFn<
	[entity: mirv.Entity, handle: number],
	Parameters<mirv.OnEntityEvent>
>;

type MirvHooks = onGameEventHookFn | onCViewRenderSetupViewHookFn | onEntityHookFn;

export class MirvHook<T extends MirvHooks> {
	private _hooks: Array<MirvHookFn<ReturnType<T>['result'], Parameters<T>[1]>> = [];
	constructor() {}

	/**
	 * Add a hook to the end of the array.
	 *
	 * @param hook - The hook to add.
	 *
	 * @returns `true` if the hook was added, `false` if it was already exists.
	 *
	 */
	push(hook: MirvHookFn<ReturnType<T>['result'], Parameters<T>[1]>) {
		for (const h of this._hooks) {
			if (h === hook) return false;
		}
		this._hooks.push(hook);
		return true;
	}
	/**
	 * Add a hook to the beginning of the array.
	 *
	 * @param hook - The hook to add.
	 *
	 * @returns `true` if the hook was added, `false` if it was already exists.
	 *
	 */
	unshift(hook: MirvHookFn<ReturnType<T>['result'], Parameters<T>[1]>) {
		for (const h of this._hooks) {
			if (h === hook) return false;
		}
		this._hooks.unshift(hook);
		return true;
	}
	/**
	 * Remove a hook from the array.
	 *
	 * @param hook - The hook to remove.
	 *
	 * @returns `true` if the hook was removed, `false` if it was not found.
	 *
	 */
	remove = (hook: MirvHookFn<ReturnType<T>['result'], Parameters<T>[1]>) => {
		const index = this._hooks.indexOf(hook);
		if (index !== -1) {
			this._hooks.splice(index, 1);
			return true;
		}
		return false;
	};
	/**
	 * Run all hooks.
	 *
	 * @param data - The data to pass to the hooks.
	 *
	 * @returns The result of the last hook.
	 */
	runHooks(...data: Parameters<T>[1]): ReturnType<T>['result'] | undefined {
		let lastResult: ReturnType<T>['result'] | undefined;
		for (const hook of this._hooks) {
			const { result, abort } = hook(lastResult, data);
			if (abort) return result;
			lastResult = result;
		}
		return lastResult;
	}
}
