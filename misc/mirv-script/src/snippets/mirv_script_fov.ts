{
	String.prototype.dedent = function () {
		return this.split('\n')
			.map((l) => l.trim())
			.join('\n');
	};

	const re_float = /^[0-9]+(\.[0-9]+)?$/;
	let fov = 90.0;
	let handleZoom: boolean | number = 90.0;

	// @ts-ignore
	if (mirv._mirv_script_fov !== undefined) mirv._mirv_script_fov.unregister();
	// @ts-ignore
	mirv._mirv_script_fov = new AdvancedfxConCommand((args) => {
		const argC = args.argC();
		const arg0 = args.argV(0);
		if (2 <= argC) {
			const arg1 = args.argV(1).toLowerCase();
			if ('default' === arg1) {
				mirv.onCViewRenderSetupView = undefined;
				return;
			} else if ('handlezoom' === arg1) {
				if (3 <= argC) {
					const arg2 = args.argV(2).toLowerCase();
					if ('false' === arg2) {
						handleZoom = false;
					} else {
						handleZoom = parseFloat(arg2);
					}
					return;
				}
				mirv.message(
					`Usage: 
					${arg0} handleZoom <f>|false - Whether to enable zoom handling (if enabled mirv_script_fov is only active if its not below minUnzoomedFov (not zoomed)). 
					Current value: ${handleZoom}
					`.dedent()
				);
				return;
			} else if (arg1.match(re_float)) {
				fov = parseFloat(arg1);
				mirv.onCViewRenderSetupView = (e) => {
					if (
						handleZoom === false ||
						(typeof handleZoom === 'number' && e.currentView.fov >= handleZoom)
					)
						return { fov: fov };
				};
				return;
			}
		}

		mirv.message(
			`Usage:
			${arg0} <f> - Override fov with given floating point value <f> e.g. 90.0.
			${arg0} default - Revert to the games default behaviour. 
			${arg0} handleZoom [...] - Handle zooming (e.g. when scoping with AWP).
			Current value: ${fov}
			`.dedent()
		);
	});
	// @ts-ignore
	mirv._mirv_script_fov.register('mirv_script_fov', 'Control fov override');
}
