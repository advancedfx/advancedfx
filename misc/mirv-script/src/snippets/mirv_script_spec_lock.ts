{
	const id = 'mirv_script_spec_lock/c4104f7d-cd98-4bf1-9afc-badd777b7a4a';

	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};

	let idx: number | null = null;

	// @ts-ignore
	if (globalThis[id] !== undefined) {
		// @ts-ignore
		globalThis[id].unregister();
		// @ts-ignore
		delete globalThis[id];
	}

	const command = new AdvancedfxConCommand((args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);

		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();

			if (arg1 === 'list') {
				for (let i = 0; i < 64; i++) {
					const entity = mirv.getEntityFromIndex(i + 1);
					if (null !== entity && entity.isPlayerController()) {
						mirv.message(`${i + 1} : ${entity.getSanitizedPlayerName()}\n`);
					}
				}
				return;
			}

			idx = parseInt(arg1);
			if (isNaN(idx) || idx === 0) idx = null;

			mirv.events.clientFrameStageNotify.on(
				id,
				(e: AdvancedfxMirv.Events.ClientFrameStageNotifyEvent) => {
					if (!e.isBefore && idx) {
						mirv.exec(`spec_player ${idx}`);
					}
				}
			);

			return;
		}

		mirv.message(
			`Usage:
			${arg0} <i> - Lock spectating to player with index <i>.
			${arg0} 0 - To disable.
			${arg0} list - List players.
			Current value: ${idx ?? 'none'}
			`.dedent()
		);
	});

	// @ts-ignore
	globalThis[id] = command;
	command.register('mirv_script_spec_lock', '');
}
