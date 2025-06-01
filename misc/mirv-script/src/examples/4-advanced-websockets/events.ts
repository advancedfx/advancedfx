// This is part of 4-advanced-websockets example.
// See entry point in index.mts
// Also server.ts and client.ts
//
// Purpose:
// Declare events that are used in server and clients.

export type EntityObject = {
	/**
	 * Index in entity list.
	 * -1 means couldn't get one.
	 */
	idx: number;
	/**
	 * -1 means invalid handle or couldn't get one.
	 */
	handle: number;
	debugName: string | null;
	className: string;
	clientClassName: string | null;
	isPlayerPawn: boolean;
	isPlayerController: boolean;
	health: number;
	origin: number[];
	renderEyeOrigin: number[];
	renderEyeAngles: number[];
};

export type MirvEventsMap = {
	exec: [cmd: string];
	gameConsoleLog: [msg: string, isWarning: boolean];
	listEntities: [entities: EntityObject[]];
	onCViewRenderSetupView: [data: mirv.OnCViewRenderSetupViewArgs];
	onGameEvent: [id: number, name: string, data: string];
	onAddEntity: [entity: EntityObject];
	onRemoveEntity: [entity: EntityObject];
	setOnCViewRenderSetupView: [state: boolean];
	setGameEvent: [state: boolean];
	setAddEntity: [state: boolean];
	setRemoveEntity: [state: boolean];
};

function createAllEventKeys<T extends readonly (keyof MirvEventsMap)[]>(
	keys: T & (keyof MirvEventsMap extends T[number] ? T : never)
): T {
	return keys;
}

export const MIRV_EVENTS = createAllEventKeys([
	'exec',
	'gameConsoleLog',
	'listEntities',
	'onCViewRenderSetupView',
	'onGameEvent',
	'onAddEntity',
	'onRemoveEntity',
	'setOnCViewRenderSetupView',
	'setGameEvent',
	'setAddEntity',
	'setRemoveEntity'
] as const);
