export const events = [
	'listTypes',
	'quit',
	'exec',
	'getLastView',
	'setView',
	'gameEvents',
	'cViewRenderSetupView'
] as const;
export type MirvEvents = (typeof events)[number];

export type MirvMessage = {
	type: MirvEvents;
	data?: string | number | object | boolean;
};

export type GameEvent = {
	id: string;
	name: string;
	data: string;
};

export type onCViewRenderSetupView = {
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

export type AfxHookSourceView = {
	x: number;
	y: number;
	z: number;
	rX: number;
	rY: number;
	rZ: number;
	fov: number;
};

export type AfxHookSourceViewSet = {
	x?: number;
	y?: number;
	z?: number;
	rX?: number;
	rY?: number;
	rZ?: number;
	fov?: number;
};
