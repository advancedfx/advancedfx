{
	var fov = 90.0;
	var handleZoom = 90.0;

	mirv._mirv_script_fov = new AdvancedfxConCommand(function (args) {
		let argC = args.argC();
		let arg0 = args.argV(0);
		if (2 <= argC) {
			let arg1 = args.argV(1).toLowerCase();
			if ('default' === arg1) {
				mirv.onCViewRenderSetupView = undefined;
				return;
			} else if ('handlezoom' === arg1) {
				if (3 <= argC) {
					let arg2 = args.argV(2).toLowerCase();
					if ('false' === arg2) {
						handleZoom = false;
					} else {
						handleZoom = parseFloat(arg2);
					}
					return;
				}
				mirv.message(
					'Usage:\n' +
						arg0 +
						" handleZoom <f>|false - Whether to enable zoom handling (if enabled mirv_fov is only active if it's not below minUnzoomedFov (not zoomed)).\n" +
						'Current value: ' +
						handleZoom +
						'\n'
				);
				return;
			} else {
				fov = parseFloat(arg1);
				mirv.onCViewRenderSetupView = function (e) {
					if (handleZoom === false || e.currentView.fov <= handleZoom)
						return { fov: fov };
				};
				return;
			}
		}

		mirv.message(
			'Usage:\n' +
				arg0 +
				' <f> - Override fov with given floating point value <f>.\n' +
				arg0 +
				" default - Revert to the game's default behaviour.\n" +
				arg0 +
				' handleZoom [...] - Handle zooming (e.g. AWP in CS:GO).\n' +
				'Current value: ' +
				fov +
				'\n'
		);
	});
	mirv._mirv_script_fov.register('mirv_script_fov', 'Control fov override');
}
