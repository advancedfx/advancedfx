export type MirvEvents = (typeof events)[keyof typeof events];
export const events = {
	warning: 'warning',
	listTypes: 'listTypes',
	quit: 'quit',
	exec: 'exec',
	getLastView: 'getLastView',
	setView: 'setView',
	setGameEvents: 'setGameEvents',
	setCViewRenderSetupView: 'setCViewRenderSetupView',
	setEntityEvents: 'setEntityEvents',
	onCViewRenderSetupView: 'onCViewRenderSetupView',
	onGameEvent: 'onGameEvent',
	onAddEntity: 'onAddEntity',
	onRemoveEntity: 'onRemoveEntity',
	listEntities: 'listEntities',
	listPlayerEntities: 'listPlayerEntities',
	loadModule: 'loadModule'
} as const;
