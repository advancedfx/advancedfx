{
	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};

	const AFX_MATH_EPS = 1.0e-6;
	const M_PI = 3.141592653589793;
	const percentPerSecond = 500;
	let index = -1;
	let snapTo = false;
	let active = false;

	const lookAnglesFromTo = (
		from: AdvancedfxMathVector3,
		to: AdvancedfxMathVector3,
		fallBackPitch: number,
		fallBackYaw: number
	) => {
		const dir = to.sub(from);

		// Store then zero height
		const dz = dir.z;

		dir.z = 0;

		// Need this for later
		//const length = dir.length; // this is currrently bugged in HLAE due to typo
		const length = Math.sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

		if (length <= AFX_MATH_EPS) {
			return {
				pitch: fallBackPitch,
				yaw: fallBackYaw
			};
		}

		//dir = dir.normalized; // this is currrently bugged in HLAE due to typo
		dir.x = dir.x / length;
		dir.y = dir.y / length;
		dir.z = dir.z / length;

		// This is our forward angle
		const vForward = new AdvancedfxMathVector3(1.0, 0.0, 0.0);

		const dotProduct = dir.x * vForward.x + dir.y * vForward.y + dir.z * vForward.z;

		let angle = (Math.acos(dotProduct) * 180.0) / M_PI;

		if (dir.y < 0) angle = 360.0 - angle;

		const pitch = (Math.atan(dz / length) * 180.0) / M_PI;

		return {
			yaw: angle,
			pitch: -pitch
		};
	};

	const aim: mirv.OnCViewRenderSetupView = (e) => {
		const entity = mirv.getEntityFromIndex(index);
		if (null !== entity) {
			const eyeOrigin = entity.getRenderEyeOrigin();
			const lookAngles = lookAnglesFromTo(
				new AdvancedfxMathVector3(e.currentView.x, e.currentView.y, e.currentView.z),
				new AdvancedfxMathVector3(eyeOrigin[0], eyeOrigin[1], eyeOrigin[2]),
				e.lastView.rX,
				e.lastView.rY
			);
			if (snapTo) {
				return {
					rX: lookAngles.pitch,
					rY: lookAngles.yaw,
					rZ: 0.0
				};
			}
			const sourceQuat = AdvancedfxMathQuaternion.fromQREulerAngles(
				AdvancedfxMathQREulerAngles.fromQEulerAngles(
					new AdvancedfxMathQEulerAngles(e.lastView.rX, e.lastView.rY, e.lastView.rZ)
				)
			);
			let targetQuat = AdvancedfxMathQuaternion.fromQREulerAngles(
				AdvancedfxMathQREulerAngles.fromQEulerAngles(
					new AdvancedfxMathQEulerAngles(lookAngles.pitch, lookAngles.yaw, 0.0)
				)
			);

			const dot = sourceQuat.dot(targetQuat);
			if (dot < 0) targetQuat = targetQuat.leftMul(-1.0);

			const interQuat = sourceQuat.slerp(
				targetQuat,
				e.absTime ? Math.max(0, Math.min((percentPerSecond / 100.0) * e.absTime, 1)) : 0
			);
			const angles = interQuat.toQREulerAngles().toQEulerAngles();
			return {
				rX: angles.pitch,
				rY: angles.yaw,
				rZ: angles.roll
			};
		}
	};

	// we use fn, otherwise we run into https://github.com/boa-dev/boa/issues/162
	const fn: AdvancedfxConCommand.OnCallback = (args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);
		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();
			if (arg1 === 'active') {
				if (3 === argC) {
					active = 1 === parseInt(args.argV(2));
					if (active) {
						mirv.onCViewRenderSetupView = aim;
					} else {
						mirv.onCViewRenderSetupView = undefined;
						// these dont do anything?
						// lastYPitch = 0.0;
						// lastZYaw = 0.0;
					}
					return;
				}
				mirv.message(
					`${arg0} active 0|1 - Whether aiming is active (1) or not (0).
					Current value: ${active ? 1 : 0}
					`.dedent()
				);
				return;
			} else if (arg1 === 'entityindex') {
				if (3 === argC) {
					index = parseInt(args.argV(2));
					return;
				}
				mirv.message(
					`${arg0} entityIndex <n> - Entity index to aim after (use ${arg0} list to get one). Use invalid index (i.e. -1) to deactivate re-targeting.
					Current value: ${index}
					`.dedent()
				);
				return;
			} else if (arg1 === 'snapto') {
				if (3 === argC) {
					snapTo = 1 === parseInt(args.argV(2));
					return;
				}
				mirv.message(
					`${arg0} snapTo 0|1 - Whether to aim non-soft (1) or not (0).
					Current value: ${snapTo ? 1 : 0}
					`.dedent()
				);
				return;
			} else if (arg1 === 'list') {
				if (3 === argC) {
					const arg2 = args.argV(2).toLowerCase();
					if (arg2 === 'entities') {
						const highest = mirv.getHighestEntityIndex();
						for (let i = 0; i < highest + 1; i++) {
							const entity = mirv.getEntityFromIndex(i);
							if (null !== entity) {
								mirv.message(
									`${i} : ${entity.getClientClassName()} / ${entity.getDebugName()} / ${entity.getClassName()} | ${entity.getRenderEyeOrigin()} / ${entity.getRenderEyeAngles()}\n`
								);
							}
						}
						return;
					} else if (arg2 === 'playermodels') {
						for (let i = 0; i < 64; i++) {
							const entity = mirv.getEntityFromIndex(i + 1);
							if (null !== entity && entity.isPlayerController()) {
								const handle = entity.getPlayerPawnHandle();
								if (mirv.isHandleValid(handle)) {
									const idx = mirv.getHandleEntryIndex(handle);
									mirv.message(`${idx} : ${entity.getSanitizedPlayerName()}\n`);
								}
							}
						}
						return;
					}
				}
				mirv.message(`${arg0} list entities|playerModels\n`);
				return;
			}
		}

		mirv.message(
			`Usage:
			${arg0} active [...] - Whether aiming is active.
			${arg0} snapTo [...] - Whether to aim non-soft or not.
			${arg0} entityIndex [...] - Entity index to aim after.
			${arg0} list [...] - List entites / player models
			`.dedent()
		);
	};

	// @ts-ignore
	if (mirv._mirv_script_aim !== undefined) mirv._mirv_script_aim.unregister();
	// @ts-ignore
	mirv._mirv_script_aim = new AdvancedfxConCommand(fn);
	// @ts-ignore
	mirv._mirv_script_aim.register('mirv_script_aim', 'Aiming system control.');
}
