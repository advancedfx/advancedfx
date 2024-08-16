{
	var value = 'default';
	var re_int = /^[0-9]+$/;

	if (mirv._mirv_script_voice !== undefined) mirv._mirv_script_voice.unregister();
	mirv._mirv_script_voice = new AdvancedfxConCommand(function (args) {
		let argC = args.argC();
		let arg0 = args.argV(0);
		if (2 <= argC) {
			var userIds = [];
			var bOk = true;
			for (var i = 1; i < argC; i++) {
				let arg = args.argV(i).toLowerCase();
				if ('default' === arg || 'none' === arg) {
				} else if ('all' === arg) {
					for (var i = 0; i < 64; i++) {
						userIds.push(i);
					}
				} else if ('t' === arg || 'ct' === arg) {
					for (var i = 0; i < 64; i++) {
						var entity = mirv.getEntityFromIndex(i + 1);
						if (null !== entity && entity.isPlayerController()) {
							var team = entity.getTeam();
							if ((team == 2 && 't' == arg) || (team == 3 && 'ct' == arg))
								userIds.push(i);
						}
					}
				} else if (re_int.test(arg)) {
					var userId = parseInt(arg);
					if (0 <= userId && userId <= 63) userIds.push(userId);
				} else {
					bOk = false;
				}
			}
			if (bOk) {
				value = '';
				for (var i = 1; i < argC; i++)
					value =
						value + (1 < i ? ' ' : '') + '"' + args.argV(i).replace('\\', '\\\\') + '"';
				var value_lo = 0;
				var value_hi = 0;
				for (var i = 0; i < userIds.length; i++) {
					var userId = userIds[i];
					if (0 <= userId && userId < 32) {
						value_lo = value_lo | (1 << userId);
					} else if (32 <= userId && userId < 64) {
						value_hi = value_hi | (1 << (userId - 32));
					}
				}
				mirv.exec('tv_listen_voice_indices ' + value_lo.toString());
				mirv.exec('tv_listen_voice_indices_h ' + value_hi.toString());
				return;
			}
		}

		mirv.message(
			'Usage:\n' +
				arg0 +
				' [all|ct|t|none|default|<userId>]+ - Override fov with given floating point value <f>.\n' +
				'Current value: ' +
				value +
				'\n'
		);
	});
	mirv._mirv_script_voice.register(
		'mirv_script_voice',
		'Easier frontend to tv_listen_voice_indices / tv_listen_voice_indices_h'
	);
}
