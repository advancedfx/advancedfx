{
	var AFX_MATH_EPS = 1.0e-6;
	var M_PI = 3.14159265358979323846;
	var index = -1;
	var active = false;
	var lastYPitch = 0.0;
	var lastZYaw = 0.0;
	var percentPerSecond = 500;
	var snapTo = false;

	function lookAnglesFromTo(from, to, fallBackPitch, fallBackYaw) {
		var dir = to.sub(from);

		// Store then zero height
		var dz = dir.z;

		dir.z = 0;

		// Need this for later
		//var length = dir.length; // this is currrently bugged in HLAE due to typo
		var length = Math.sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

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
		vForward = new AdvancedfxMathVector3(1.0, 0.0, 0.0);

		var dot_product = dir.x * vForward.x + dir.y * vForward.y + dir.z * vForward.z;

		var angle = (Math.acos(dot_product) * 180.0) / M_PI;

		if (dir.y < 0) angle = 360.0 - angle;

		var pitch = (Math.atan(dz / length) * 180.0) / M_PI;

		return {
			yaw: angle,
			pitch: -pitch
		};
	}

	function aim(e) {
		var entity = mirv.getEntityFromIndex(index);
		if (null !== entity) {
			var eyeOrigin = entity.getRenderEyeOrigin();
			var lookAngles = lookAnglesFromTo(
				new AdvancedfxMathVector3(e.currentView.x, e.currentView.y, e.currentView.z),
				new AdvancedfxMathVector3(eyeOrigin[0], eyeOrigin[1], eyeOrigin[2]),
				lastYPitch,
				lastZYaw
			);
			lastYPitch = lookAngles.pitch;
			lastZYaw = lookAngles.yaw;
			if (snapTo) {
				return {
					rX: lastYPitch,
					rY: lastZYaw,
					rZ: 0.0
				};
			}
			var sourceQuat = AdvancedfxMathQuaternion.fromQREulerAngles(
				AdvancedfxMathQREulerAngles.fromQEulerAngles(
					new AdvancedfxMathQEulerAngles(
						e.currentView.rX,
						e.currentView.rY,
						e.currentView.rZ
					)
				)
			);
			var targetQuat = AdvancedfxMathQuaternion.fromQREulerAngles(
				AdvancedfxMathQREulerAngles.fromQEulerAngles(
					new AdvancedfxMathQEulerAngles(lastYPitch, lastZYaw, 0.0)
				)
			);

			var dot = sourceQuat.dot(targetQuat);
			if (dot < 0) targetQuat = targetQuat.leftMul(-1.0);

			var interQuat = sourceQuat.slerp(
				targetQuat,
				e.absTime ? Math.max(0, Math.min((percentPerSecond / 100.0) * e.absTime, 1)) : 0
			);
			angles = interQuat.toQREulerAngles().toQEulerAngles();
			return {
				rX: angles.pitch,
				rY: angles.yaw,
				rZ: angles.roll
			};
		}
		lastYPitch = e.currentView.rX;
		lastZYaw = e.currentView.rY;
	}

	// we use fn, otherwise we run into https://github.com/boa-dev/boa/issues/162
	function fn(args) {
		let argC = args.argC();
		let arg0 = args.argV(0);
		if (2 <= argC) {
			var arg1 = args.argV(1).toLowerCase();
			if (arg1 == 'active') {
				if (3 == argC) {
					active = 1 == parseInt(args.argV(2));
					if (active) {
						mirv.onCViewRenderSetupView = aim;
					} else {
						mirv.onCViewRenderSetupView = undefined;
						lastYPitch = 0.0;
						lastZYaw = 0.0;
					}
					return;
				}
				mirv.message(
					arg0 +
						' active 0|1 - Whether aiming is active (1) or not (0).\n' +
						'Current value: ' +
						(active ? '1' : '0') +
						'\n'
				);
				return;
			} else if (arg1 == 'entityindex') {
				if (3 == argC) {
					index = parseInt(args.argV(2));
					return;
				}
				mirv.message(
					arg0 +
						' entityIndex <n> - Entity index to aim after (use ' +
						arg0 +
						' list to get one). Use invalid index (i.e. -1) to deactivate re-targeting.\n' +
						'Current value: ' +
						index +
						'\n'
				);
				return;
			} else if (arg1 == 'snapto') {
				if (3 == argC) {
					snapTo = 1 == parseInt(args.argV(2));
					return;
				}
				mirv.message(
					arg0 +
						' snapTo 0|1 - Whether to aim non-soft (1) or not (0).\n' +
						'Current value: ' +
						(snapto ? '1' : '0') +
						'\n'
				);
				return;
			} else if (arg1 == 'list') {
				if (3 == argC) {
					var arg2 = args.argV(2).toLowerCase();
					if (arg2 == 'entities') {
						var highest = mirv.getHighestEntityIndex();
						for (var i = 0; i < highest + 1; i++) {
							var entity = mirv.getEntityFromIndex(i);
							if (null !== entity) {
								mirv.message(
									i +
										': ' +
										entity.getClientClassName() +
										' / ' +
										entity.getDebugName() +
										' / ' +
										entity.getClassName() +
										' | ' +
										entity.getRenderEyeOrigin() +
										' / ' +
										entity.getRenderEyeAngles() +
										'\n'
								);
							}
						}
						return;
					} else if (arg2 == 'playermodels') {
						for (var i = 0; i < 64; i++) {
							var entity = mirv.getEntityFromIndex(i + 1);
							if (null !== entity && entity.isPlayerController()) {
								var handle = entity.getPlayerPawnHandle();
								if (mirv.isHandleValid(handle)) {
									var idx = mirv.getHandleEntryIndex(handle);
									mirv.message(
										idx + ': ' + entity.getSanitizedPlayerName() + '\n'
									);
								}
							}
						}
					}
				}

				mirv.message(arg0 + ' list entites|playerModels\n');
				return;
			}
		}

		mirv.message(
			'Usage:\n' +
				arg0 +
				' active [...] - Whether aiming is active.\n' +
				arg0 +
				' entityIndex [...] - Entity index to aim after.\n' +
				arg0 +
				' list [...] - List entites / player models.\n'
		);
	}

	if (mirv._mirv_script_aim !== undefined) mirv._mirv_script_aim.unregister();
	mirv._mirv_script_aim = new AdvancedfxConCommand(fn);
	mirv._mirv_script_aim.register('mirv_script_aim', 'Aiming system control.');
}
