export type MirvEvents = (typeof events)[keyof typeof events];
export const events = {
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
	warning: 'warning'
} as const;
