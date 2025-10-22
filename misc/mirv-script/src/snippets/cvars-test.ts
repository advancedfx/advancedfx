{
	const typesMap = {
		[-1]: 'Invalid',
		0: 'Bool',
		1: 'Int16',
		2: 'UInt16',
		3: 'Int32',
		4: 'UInt32',
		5: 'Int64',
		6: 'UInt64',
		7: 'Float32',
		8: 'Float64',
		9: 'String',
		10: 'Color',
		11: 'Vector2',
		12: 'Vector3',
		13: 'Vector4',
		14: 'Qangle'
	} as const;

	function getCvarByName(name: string) {
		const idx = AdvancedfxCVar.getIndexFromName(name);
		if (!idx) return;

		let cvar: AdvancedfxCVar | null = null;
		try {
			cvar = new AdvancedfxCVar(idx);
		} catch {
			return;
		}

		return cvar;
	}

	function printCvar(name: string) {
		const cvar = getCvarByName(name);
		if (!cvar) return;

		const msg = [name];
		msg.push(`\ttype: ${cvar.getType()} / ${typesMap[cvar.getType()]}`);
		msg.push(`\tvalue: ${cvar.value}`);
		msg.push(`\thelp: ${cvar.helpString}`);
		msg.push(`\tdefault value: ${cvar.defaultValue}`);
		msg.push(`\tmin value: ${cvar.minValue}`);
		msg.push(`\tmax value: ${cvar.maxValue}`);

		console.log(msg.join('\n'));
	}

	const typesFound: AdvancedfxCVar.TypeEnum[] = [];

	for (let i = 0; i < 8192; i++) {
		let cvar = null;

		try {
			cvar = new AdvancedfxCVar(i);
		} catch {
			console.log(`Last valid cvar index ${i - 1}`);
		}

		if (cvar === null) break;

		const type = cvar.getType();
		if (undefined === typesFound.find((v) => v === type)) typesFound.push(type);
	}

	console.log('Types found:');
	typesFound.sort((a, b) => a - b);
	for (const t of typesFound) {
		console.log(`${t}: ${typesMap[t]}`);
	}

	// We cannot access hidden ones by name
	mirv.exec('mirv_cvar_unhide_all');

	// Game doesn't use all types for cvars
	printCvar('bot_allow_grenades'); // Bool
	printCvar('bot_debug'); // Int32
	printCvar('snd_steamaudio_active_hrtf'); // UInt32
	printCvar('tv_allow_camera_man_steamid'); // UInt64
	printCvar('cl_crosshairgap'); // Float32
	printCvar('cl_language'); // String
	printCvar('cl_teammate_color_1'); // Color
	printCvar('lb_override_barn_light_fade_sizes'); // Vector2
	printCvar('cl_eye_target_override'); // Vector3

	const fps_max = getCvarByName('fps_max');
	if (fps_max) {
		const oldValue = fps_max.value;
		console.log('Current fps_max:', fps_max.value);
		console.log('Setting it to', 400);
		fps_max.value = 400;
		console.log('Current fps_max:', fps_max.value);
		console.log('Restoring previous fps_max value');
		fps_max.value = oldValue;
		console.log('Current fps_max:', fps_max.value);
	}
}
