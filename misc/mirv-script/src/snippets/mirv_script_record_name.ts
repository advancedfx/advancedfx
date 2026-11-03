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

	let enabled = false;
	let ext: string = 'mov';
	let ffmpegPresets: string[] = [];

	function getSpectatedPlayer() {
		const localControllerEntity = mirv.getEntityFromSplitScreenPlayer(0);
		if (null === localControllerEntity) return null;
		const localPawnEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localControllerEntity.getPlayerPawnHandle())
		);
		if (null === localPawnEntity) return null;
		const observerTargetEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localPawnEntity.getObserverTargetHandle())
		);
		if (null === observerTargetEntity) return null;
		const obserTargetControllerEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(observerTargetEntity.getPlayerControllerHandle())
		);
		if (null === obserTargetControllerEntity) return null;
		return obserTargetControllerEntity;
	}

	function getBaseFolder() {
		const demoNamePath = mirv.getDemoFilePath();

		if (demoNamePath) {
			const demoName = (demoNamePath.split(/[\\/]/).pop() ?? '').replace('.dem', '');
			return `{RECORD_PATH}\\${demoName}`;
		}

		return '{RECORD_PATH}';
	}

	function getScreenShotsPreset(base: string) {
		return `${getFfmpegPreset(base, false).replace(`.${ext}`, '')}\\{STREAM_NAME}\\{SEQUENCE_NR}.{EXT}`;
	}

	function getFfmpegPreset(base: string, addStreamName = true) {
		let fileName = '';

		const player = getSpectatedPlayer();
		if (player) {
			fileName += `pov_${player.getSanitizedPlayerName()}_`;
		} else {
			fileName += 'cam_';
		}

		fileName += `{TICK}_`;

		if (addStreamName) fileName += '{STREAM_NAME}_';
		fileName += `{TAKE}{TAKE_NUMBER}`;

		return `${base}\\${fileName}.${ext}`;
	}

	function applyTemplates() {
		const baseFolder = getBaseFolder();
		mirv.exec(
			`mirv_streams settings edit afxClassic path "${getScreenShotsPreset(baseFolder)}"`
		);
		ffmpegPresets.forEach((p) =>
			mirv.exec(`mirv_streams settings edit ${p} path "${getFfmpegPreset(baseFolder)}"`)
		);
		if (ffmpegPresets.length === 0) {
			mirv.exec(
				`mirv_streams record startMovieWavPath "${baseFolder}\\{TAKE}{TAKE_NUMBER}.wav"`
			);
		}
		mirv.exec(
			`mirv_streams record startMovieWavPath "${getFfmpegPreset(baseFolder, false)}"`.replace(
				`.${ext}`,
				'.wav'
			)
		);
		mirv.exec(
			`mirv_streams record campath path "${getFfmpegPreset(baseFolder, false)}"`.replace(
				`.${ext}`,
				'.xml'
			)
		);
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
			} else if (arg1 === 'ext') {
				if (3 <= argC) {
					ext = args.argV(2).replace('.', '');
					return;
				}

				mirv.message(
					`${arg0} ${arg1} <str> - Set extension for ffmpeg preset.
					Current value: ${ext}
					`.dedent()
				);
				return;
			} else if (arg1 === 'ffmpegPresets'.toLowerCase()) {
				if (3 <= argC) {
					const arg2 = args.argV(2).toLowerCase();
					if (arg2 === 'add' && 4 <= argC) {
						ffmpegPresets.push(args.argV(3));
						return;
					} else if (arg2 === 'print') {
						let msg = '';
						ffmpegPresets.forEach((p) => (msg += `${p}\n`));
						console.log(msg);
						return;
					} else if (arg2 === 'clear') {
						ffmpegPresets = [];
						return;
					}
				}

				mirv.message(
					`${arg0} ${arg1} add - Add ffmpeg settings name to apply to.
					${arg0} ${arg1} print - Print current presets.
					${arg0} ${arg1} clear - Clear current presets.
					`.dedent()
				);
				return;
			} else if (arg1 === 'apply') {
				applyTemplates();
				enabled = true;
				return;
			}
		}

		mirv.message(
			`This command is used to update recording settings paths to more convinient format.
			Usage:
			${arg0} ext <str> - Set extension for ffmpeg presets. Default: "mov"
			${arg0} ffmpegPresets [...] - Manage presets that will be updated.
			${arg0} apply - Apply settings to added presets and enabled their update.
			${arg0} enabled 1|0 - Enable or disable presets update.
			Example usage:
			${arg0} ffmpegPresets add afxFfmpegProres;
			${arg0} apply;
			`.dedent()
		);
	});

	// @ts-ignore
	globalThis[id] = command;
	command.register('mirv_script_record_name', '');
}
