// add hook on module load and remove afterwards

import { MirvJS } from './mirv.mjs';
import { onCViewRenderSetupViewHookFn } from './utils.mjs';

mirv.message('test-hook-module loaded\n');

let count = 0;

const sampleRenderViewHook: onCViewRenderSetupViewHookFn = (lastResult, args) => {
	const e = args[0];
	if (count < Math.floor(e.curTime))
		mirv.message(`sampleRenderViewHook: current time is ${e.curTime}\n`);

	count = Math.floor(e.curTime);

	if (e.curTime > 50) {
		// remove hook
		MirvJS.hooks.onCViewRenderSetupView.push(sampleRenderViewHook);
		mirv.onCViewRenderSetupView = undefined;
		mirv.message('test-hook-module unhooked\n');
	}

	return { result: lastResult, abort: false };
};

MirvJS.hooks.onCViewRenderSetupView.push(sampleRenderViewHook);
