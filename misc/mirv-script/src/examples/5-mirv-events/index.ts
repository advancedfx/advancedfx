// Purpose:
// Demonstrate how to register multiple mirv.events listeners for the same event.
// Unlike mirv.onGameEvent, these listeners do not overwrite each other.
//
// Usage:
// 1) Load this script in game with mirv_script_load
// e.g. mirv_script_load "C:\advancedfx\misc\mirv-script\dist\examples\5-mirv-events\index.js"
//
// 2) Enter "mirv_event_example on" in the game console.
// 3) Play a demo until a player_death event happens.
// 4) Check the game console output and note that both listeners fire.
// 5) Enter "mirv_event_example off" to remove both listeners.

{
	const listenerNameA = 'examplePlayerDeathName';
	const listenerNameB = 'examplePlayerDeathData';

	const install = () => {
		mirv.events.GameEvent.on(listenerNameA, (e) => {
			if (e.name === 'player_death') {
				console.log('[listener A] event name:', e.name);
			}
		});

		mirv.events.GameEvent.on(listenerNameB, (e) => {
			if (e.name === 'player_death') {
				console.log('[listener B] event data:', e.data);
			}
		});
	};

	const uninstall = () => {
		mirv.events.GameEvent.remove(listenerNameA);
		mirv.events.GameEvent.remove(listenerNameB);
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Register command
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// @ts-ignore
	if (mirv._mirv_event_example !== undefined) mirv._mirv_event_example.unregister();
	// @ts-ignore
	mirv._mirv_event_example = new AdvancedfxConCommand((args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);

		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();

			if ('on' === arg1) {
				uninstall();
				install();
				mirv.message(`${arg0}: registered 2 GameEvent listeners.\n`);
				return;
			}

			if ('off' === arg1) {
				uninstall();
				mirv.message(`${arg0}: removed GameEvent listeners.\n`);
				return;
			}
		}

		mirv.message(
			`Usage:
			${arg0} on - Register two mirv.events.GameEvent listeners.
			${arg0} off - Remove both listeners.
			`
		);
	});
	// @ts-ignore
	mirv._mirv_event_example.register(
		'mirv_event_example',
		'Register or remove example GameEvent listeners.'
	);
}
