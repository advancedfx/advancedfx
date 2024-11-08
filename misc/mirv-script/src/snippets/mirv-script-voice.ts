{
	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};

	const re_int = /^[0-9]+$/;
	let value = 'default';

	// @ts-ignore
	if (mirv._mirv_script_voice !== undefined) mirv._mirv_script_voice.unregister();
	// @ts-ignore
	mirv._mirv_script_voice = new AdvancedfxConCommand(function (args) {
		const argC = args.argC();
		const arg0 = args.argV(0);
		if (2 <= argC) {
			const userIds = [];
			let bOk = true;
			for (let i = 1; i < argC; i++) {
				const arg = args.argV(i).toLowerCase();
				if ('default' === arg || 'none' === arg) {
				} else if ('all' === arg) {
					for (let j = 0; j < 64; j++) {
						userIds.push(j);
					}
				} else if ('t' === arg || 'ct' === arg) {
					for (let j = 0; j < 64; j++) {
						const entity = mirv.getEntityFromIndex(j + 1);
						if (null !== entity && entity.isPlayerController()) {
							const team = entity.getTeam();
							if ((team === 2 && 't' === arg) || (team === 3 && 'ct' === arg))
								userIds.push(j);
						}
					}
				} else if (arg.match(re_int)) {
					const userId = parseInt(arg);
					if (0 <= userId && userId <= 63) userIds.push(userId);
				} else {
					bOk = false;
				}
			}
			if (bOk) {
				value = '';
				for (let i = 1; i < argC; i++)
					value =
						value + (1 < i ? ' ' : '') + '"' + args.argV(i).replace('\\', '\\\\') + '"';
				let valueLo = 0;
				let valueHi = 0;
				for (let i = 0; i < userIds.length; i++) {
					const userId = userIds[i];
					if (0 <= userId && userId < 32) {
						valueLo = valueLo | (1 << userId);
					} else if (32 <= userId && userId < 64) {
						valueHi = valueHi | (1 << (userId - 32));
					}
				}
				mirv.exec(`tv_listen_voice_indices ${valueLo.toString()}`);
				mirv.exec(`tv_listen_voice_indices_h ${valueHi.toString()}`);
				return;
			}
		}

		mirv.message(
			`Usage:
			${arg0} [all|ct|t|none|default|<userId>] - Isolate voices for given player(s).
			Current value: ${value}
			`.dedent()
		);
	});
	// @ts-ignore
	mirv._mirv_script_voice.register(
		'mirv_script_voice',
		'Easier frontend to tv_listen_voice_indices / tv_listen_voice_indices_h'
	);
}
