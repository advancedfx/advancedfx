{
	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};
	// @ts-ignore
	if (mirv._mirv_script_point !== undefined) mirv._mirv_script_point.unregister();

	const state = {
		prev: -1,
		current: 0,
		next: 1,
		total: 0,
		debug: false
	};

	mirv.getMainCampath().onChanged = () => {
		if (state.total !== mirv.getMainCampath().size) {
			state.prev = -1;
			state.current = 0;
			state.next = 1;
			state.total = mirv.getMainCampath().size;
		}
	};

	type Point = { time: number; value: AdvancedfxCampathValue; index: number };

	const getPointByDirection = (dir: 'prev' | 'next') => {
		const totalKeyframes = mirv.getMainCampath().size;
		const iterator = new AdvancedfxCampathIterator(mirv.getMainCampath());
		if (!iterator.valid) {
			mirv.warning('No campath points.\n');
			return;
		}
		if (dir === 'prev') {
			if (state.current > 0) {
				state.next = state.current;
				state.current--;
				state.prev = state.current - 1;
			}
		} else if (dir === 'next') {
			if (state.current < totalKeyframes - 1) {
				state.prev = state.current;
				state.current++;
				state.next = state.current + 1;
			}
		}
		let i = 0;
		while (iterator.valid) {
			if (i === state.current) {
				return { time: iterator.time, value: iterator.value, index: i };
			}
			i++;
			iterator.next();
		}
	};

	const getSelectedPoint = () => {
		const iterator = new AdvancedfxCampathIterator(mirv.getMainCampath());
		if (!iterator.valid) {
			mirv.warning('No campath points.\n');
			return;
		}
		let selectedPoint: AdvancedfxCampathValue | undefined;
		let time;
		let i = 0;
		while (iterator.valid) {
			const campathPoint = iterator.value;
			if (campathPoint.selected) {
				selectedPoint = campathPoint;
				time = iterator.time;
				break;
			}
			i++;
			iterator.next();
		}
		if (!selectedPoint || !time) return;
		return { time, value: selectedPoint, index: i };
	};

	const getPointByIndex = (index: number) => {
		const iterator = new AdvancedfxCampathIterator(mirv.getMainCampath());
		if (!iterator.valid) {
			mirv.warning('No campath points.\n');
			return;
		}
		let i = 0;
		while (iterator.valid) {
			if (index === i) {
				return { time: iterator.time, value: iterator.value, index: i };
			}
			i++;
			iterator.next();
		}
	};

	const resolveData = (
		type: 'position' | 'angles' | 'fov' | 'time' | 'all',
		index?: string | number,
		shouldSelect?: string
	) => {
		let camPoint: Point | undefined;
		switch (typeof index) {
			case 'number':
				camPoint = getPointByIndex(index);
				break;
			case 'string':
				{
					if (index === 'prev' || index === 'next') {
						camPoint = getPointByDirection(index);
					} else {
						mirv.warning(`Unknown argument: ${index}\n`);
						return;
					}
				}
				break;
			default:
				camPoint = getSelectedPoint();
				break;
		}
		if (!camPoint) {
			index
				? mirv.warning(`No point at index ${index}.\n`)
				: mirv.warning('No selected point.\n');
			return;
		}
		const position = `mirv_input position ${camPoint.value.pos.x} ${camPoint.value.pos.y} ${camPoint.value.pos.z}`;
		const anglesRaw = camPoint.value.rot.toQREulerAngles().toQEulerAngles();
		const angles = `mirv_input angles ${anglesRaw.pitch} ${anglesRaw.yaw} ${anglesRaw.roll}`;
		const fov = `mirv_input fov ${camPoint.value.fov}`;
		const time = `mirv_skip time toGame ${camPoint.time}`;

		mirv.exec('mirv_input camera');
		type === 'position' && mirv.exec(position);
		type === 'angles' && mirv.exec(angles);
		type === 'fov' && mirv.exec(fov);
		type === 'time' && mirv.exec(time);
		if (type === 'all') mirv.exec([position, angles, fov, time].join('; '));
		if (shouldSelect) mirv.exec(`mirv_campath select #${camPoint.index} #${camPoint.index}`);
		if (state.debug) {
			mirv.message(
				`Resolved campath point:
				id : gameTime -> ( x y z ) fov ( pitch yaw roll )
				${camPoint.index} : ${camPoint.time.toFixed(6)} -> ( ${camPoint.value.pos.x.toFixed(6)} ${camPoint.value.pos.y.toFixed(6)} ${camPoint.value.pos.z.toFixed(6)} ) ${camPoint.value.fov} ( ${anglesRaw.pitch.toFixed(6)} ${anglesRaw.yaw.toFixed(6)} ${anglesRaw.roll.toFixed(6)} )
				`.dedent()
			);
		}

		return true;
	};

	// @ts-ignore
	mirv._mirv_script_point = new AdvancedfxConCommand((args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);
		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();
			let arg2: string | number | undefined = argC >= 3 ? args.argV(2) : undefined;
			if (arg2) {
				arg2 = isNaN(Number(arg2)) ? arg2.toLowerCase() : Number(arg2);
			}
			const arg3 = argC >= 4 ? args.argV(3) : undefined;

			let ok;
			switch (arg1) {
				case 'position':
					ok = resolveData('position', arg2, arg3);
					break;
				case 'angles':
					ok = resolveData('angles', arg2, arg3);
					break;
				case 'fov':
					ok = resolveData('fov', arg2, arg3);
					break;
				case 'time':
					ok = resolveData('time', arg2, arg3);
					break;
				case 'all':
					ok = resolveData('all', arg2, arg3);
					break;
				case 'debug':
					state.debug = 1 === arg2;
					ok = true;
					break;
			}
			if (ok) return;
		}
		mirv.message(
			`Usage:
			${arg0} position <index|prev|next> <shouldSelect> - Apply point position to mirv_input.
			${arg0} angles <index|prev|next> <shouldSelect> - Apply point angles to mirv_input.
			${arg0} fov <index|prev|next> <shouldSelect> - Apply point fov to mirv_input.
			${arg0} time <index|prev|next> <shouldSelect> - Apply point time to mirv_input.
			${arg0} all <index|prev|next> <shouldSelect> - Apply point position, angles, fov to mirv_input and skips to point's time.
			${arg0} debug <1|0> - Enable/disable resolved campath point info printing.
			If no value is provided the first selected point will be used (if exists).
			<shouldSelect> can be any value. If provided it will select the given point.
			`.dedent()
		);
	});
	// @ts-ignore
	mirv._mirv_script_point.register(
		'mirv_script_point',
		'Apply campath point properties to mirv_input'
	);
}
