export type EntityObject = {
	name: string;
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

export const makeEntityObject = (e: mirv.Entity): EntityObject => {
	const playerPawnHandle = e.getPlayerPawnHandle();
	const playerControllerHandle = e.getPlayerControllerHandle();
	return {
		name: e.getName(),
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
