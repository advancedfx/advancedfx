// Purpose:
// Demonstrate how to register custom command in game and use it with mirv hooks.
//
// This script uses SubCommand wrapper. See subcommand.ts
// Wrapper might not be ideal in some cases.
// See snippets folder for examples without wrapper.
//
// Usage:
// 1) Load this script in game with mirv_script_load
// e.g. mirv_script_load "C:\advancedfx\misc\mirv-script\dist\examples\3-command-snippet\index.js"
//
// 2) Enter mirv_script_example in console

(async () => {
	// We have to import class this way, so we can avoid using modules,
	// it allows us to run script again and redefine command
	const { SubCommand, subCommandsCallback } = await import('./subcommand.js');

	// Alternatively, you can use just one subcommand with actual value, and then check it for 0 in hook.
	const fovToggle = new SubCommand({
		type: 'boolean',
		path: ['fov', 'enabled'],
		placeholder: '1|0',
		description: 'Enable / disable fov overwrite.',
		initialValue: false
	});

	// It's not best practice to do path like that, it works this way too.
	// The better way would be ['fov', 'value']
	const fovState = new SubCommand({
		type: 'float',
		path: ['fov'],
		placeholder: 'fValue',
		description: 'Set custom fov value.',
		initialValue: 90.0
	});

	// Set hook that can be controlled via console
	mirv.onCViewRenderSetupView = () => {
		if (!fovToggle.value) return undefined;
		return {
			fov: fovState.value
		};
	};

	const subCommands = [
		fovToggle,
		fovState,
		new SubCommand({
			type: 'int',
			path: ['int'],
			placeholder: 'iValue',
			description: 'Set integer value.',
			initialValue: 0
		}),
		new SubCommand({
			type: 'float',
			path: ['float'],
			placeholder: 'fValue',
			description: 'Set float value.',
			initialValue: 0.0
		}),
		new SubCommand({
			type: 'string',
			path: ['string'],
			placeholder: 'sValue',
			description: 'Set string value.',
			initialValue: ''
		}),
		new SubCommand({
			type: 'boolean',
			path: ['bool'],
			placeholder: '1|0',
			description: 'Set boolean value.',
			initialValue: false
		}),
		new SubCommand({
			type: 'string',
			path: ['some', 'nested', 'cmd'],
			placeholder: 'Value',
			description: 'blank desc.',
			initialValue: 'none',
			onSet: (v) => console.log('Some nested cmd was set to', v)
		}),
		new SubCommand({
			type: 'string',
			path: ['some', 'more', 'nested'],
			placeholder: 'Value',
			description: 'blank desc.',
			initialValue: 'none'
		}),
		new SubCommand({
			type: 'none',
			path: ['hello'],
			placeholder: undefined,
			description: 'Print hello.',
			initialValue: undefined,
			// For subCommands without args you can do action in "onSet"
			onSet: () => console.log('Hello.')
		}),
		new SubCommand({
			type: 'multi',
			path: ['multi'],
			placeholder: ['Value1', 'Value2'],
			description: 'Set multiple values.',
			argsCount: 2,
			initialValue: [],
			onSet: (values) => {
				// When having multi-line subCommand,
				// you might want to do some side effects
				console.log(`Did set values: [${values.join(', ')}]`);
			}
		})
	];

	// See subcommand.ts for implementation.
	// Or snippets folder to see ones without wrapper.
	const fn: AdvancedfxConCommand.OnCallback = (args) => subCommandsCallback(subCommands, args);

	// Note:
	// Currently there is no automatic hooks handling
	// you have to handle it yourself e.g. one script can overwrite another's script hook

	// Unregister if there's already one.
	// @ts-ignore
	if (mirv._mirv_script_example !== undefined) mirv._mirv_script_example.unregister();
	// Store command in object.
	// @ts-ignore
	mirv._mirv_script_example = new AdvancedfxConCommand(fn);
	// Register command
	// @ts-ignore
	mirv._mirv_script_example.register('mirv_script_example', 'Example command');
})();
