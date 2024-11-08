import { onGameEventHookFn } from './utils.mjs';

export const firstOnGameEventHook: onGameEventHookFn = (lastResult, args) => {
	const e = args[0];
	if (e.name === 'player_hurt') {
		mirv.warning('firstOnGameEventHook: player_hurt\n');
	}
	e.id = 42069;
	return { result: e, abort: false };
};

export const secondOnGameEventHook: onGameEventHookFn = (lastResult, args) => {
	const e = args[0];
	if (lastResult) lastResult.name = '42069';
	return { result: lastResult ?? e, abort: true };
};

export const thirdOnGameEventHook: onGameEventHookFn = (lastResult, args) => {
	const e = args[0];
	if (lastResult) lastResult.id = 777;
	e.id = 777;
	return { result: lastResult ?? e, abort: false };
};
