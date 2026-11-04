{
	const id = 'mirv_script_record_name/51c6b513-5ca7-44a1-aa44-37eb4609ea17';

	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};

	// @ts-ignore
	if (globalThis[id] !== undefined) {
		// @ts-ignore
		globalThis[id].unregister();
		// @ts-ignore
		delete globalThis[id];
	}

	mirv.events.clientFrameStageNotify.off(id);

	let enabled = false;
	const ffmpegPresets = new Map<string, string>();

	const OBSERVER_MODE_POV = 2;
	const TAKE_SUFFIX = '_{TAKE_NUMBER:2}';

	// eslint-disable-next-line no-control-regex
	const INVALID_FILE_NAME_CHARS = /[<>:"/\\|?*\x00-\x1f]/g;

	function sanitize(value: string) {
		return value
			.replace(INVALID_FILE_NAME_CHARS, '_')
			.replace(/[. ]+$/, '')
			.replace(/[{}]/g, (c) => c + c);
	}

	function getSpectatedPlayer() {
		const localControllerEntity = mirv.getEntityFromSplitScreenPlayer(0);
		if (null === localControllerEntity) return null;
		const localPawnEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localControllerEntity.getPlayerPawnHandle())
		);
		if (null === localPawnEntity) return null;
		if (OBSERVER_MODE_POV !== localPawnEntity.getObserverMode()) return null;
		const observerTargetEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localPawnEntity.getObserverTargetHandle())
		);
		if (null === observerTargetEntity) return null;
		const observerTargetControllerEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(observerTargetEntity.getPlayerControllerHandle())
		);
		if (null === observerTargetControllerEntity) return null;
		return observerTargetControllerEntity;
	}

	function getBaseFolder() {
		const demoNamePath = mirv.getDemoFilePath();

		if (demoNamePath) {
			const demoName = sanitize(
				(demoNamePath.split(/[\\/]/).pop() ?? '').replace(/\.dem$/i, '')
			);
			if (demoName) return `{RECORD_PATH}\\${demoName}`;
		}

		return '{RECORD_PATH}';
	}

	function getFilePath(base: string, label?: string) {
		let fileName = '';

		const player = getSpectatedPlayer();
		if (player) {
			const playerName = sanitize(player.getSanitizedPlayerName() ?? '');
			fileName += playerName ? `pov_${playerName}_` : 'pov_';
		} else {
			fileName += 'cam_';
		}

		fileName += '{TICK:6}';

		if (label) fileName += `_${label}`;
		fileName += TAKE_SUFFIX;

		return `${base}\\${fileName}`;
	}

	function getScreenShotsPreset(base: string) {
		return `${getFilePath(base)}\\{STREAM_NAME}\\{SEQUENCE_NR}.{EXT}`;
	}

	function getFfmpegPreset(base: string, ext: string) {
		return `${getFilePath(base, '{STREAM_NAME}')}.${sanitize(ext)}`;
	}

	function getRecordFile(base: string, label: string, ext: string) {
		return ffmpegPresets.size === 0
			? `${getFilePath(base)}\\${label}.${ext}`
			: `${getFilePath(base, label)}.${ext}`;
	}

	function applyTemplates() {
		const baseFolder = getBaseFolder();
		mirv.exec(
			`mirv_streams settings edit afxClassic path "${getScreenShotsPreset(baseFolder)}"`
		);
		ffmpegPresets.forEach((ext, p) =>
			mirv.exec(`mirv_streams settings edit ${p} path "${getFfmpegPreset(baseFolder, ext)}"`)
		);
		mirv.exec(
			`mirv_streams record startMovieWavPath "${getRecordFile(baseFolder, 'audio', 'wav')}"`
		);
		mirv.exec(
			`mirv_streams record campath path "${getRecordFile(baseFolder, 'campath', 'xml')}"`
		);
		mirv.exec(`mirv_streams record cam path "${getRecordFile(baseFolder, 'camera', 'cam')}"`);
	}

	function resetTemplates() {
		enabled = false;
		mirv.exec('mirv_streams settings edit afxClassic path default');
		ffmpegPresets.forEach((_, p) => mirv.exec(`mirv_streams settings edit ${p} path default`));
		mirv.exec('mirv_streams record startMovieWavPath default');
		mirv.exec('mirv_streams record campath path default');
		mirv.exec('mirv_streams record cam path default');
	}

	mirv.events.clientFrameStageNotify.on(id, (e) => {
		if (e.curStage === SOURCESDK_CS2.ClientFrameStage_t.FRAME_START && e.isBefore) {
			if (enabled) applyTemplates();
		}
	});

	const command = new AdvancedfxConCommand((args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);

		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();

			if (arg1 === 'enabled') {
				if (3 <= argC) {
					enabled = args.argV(2) === '1';
					return;
				}
				mirv.message(
					`${arg0} ${arg1} 1|0 - Enable or disable presets update.
					Current value: ${enabled ? 1 : 0}
					`.dedent()
				);
				return;
			} else if (arg1 === 'ffmpegPresets'.toLowerCase()) {
				if (3 <= argC) {
					const arg2 = args.argV(2).toLowerCase();
					if (arg2 === 'add' && 5 <= argC) {
						ffmpegPresets.set(args.argV(3), args.argV(4).replace(/^\./, ''));
						return;
					} else if (arg2 === 'print') {
						let msg = 'name extension\n---------------\n';
						ffmpegPresets.forEach((ext, p) => (msg += `${p} ${ext}\n`));
						console.log(msg);
						return;
					}
				}

				mirv.message(
					`${arg0} ${arg1} add <name> <ext> - Add ffmpeg settings name to apply to, <ext> must be supported by its codec.
					${arg0} ${arg1} print - Print current presets.
					`.dedent()
				);
				return;
			} else if (arg1 === 'apply') {
				applyTemplates();
				enabled = true;
				return;
			} else if (arg1 === 'reset') {
				resetTemplates();
				ffmpegPresets.clear();
				return;
			}
		}

		mirv.message(
			`This command is used to update recording settings paths to a more convenient format.
			Also works if no ffmpeg is used (assuming recording is set with screenshots).

			Usage:
			${arg0} ffmpegPresets [...] - Manage presets that will be updated.
			${arg0} apply - Apply settings to added presets and enable their update.
			${arg0} reset - Disable update and restore default paths, clear all presets.
			${arg0} enabled 1|0 - Enable or disable presets update.

			Example usage:
			mirv_streams add normal normal;
			mirv_streams edit normal settings afxFfmpegProres;
			${arg0} ffmpegPresets add afxFfmpegProres mov;
			${arg0} apply;
			`.dedent()
		);
	});

	// @ts-ignore
	globalThis[id] = command;
	command.register('mirv_script_record_name', '');
}
