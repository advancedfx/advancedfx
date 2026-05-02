{
	const id = 'quit-when-demo-is-paused/d51cfe1b-278e-47c7-8cea-f4e9b350b27a';
	mirv.events.clientFrameStageNotify.on(id, () => {
		if (mirv.isPlayingDemo() && mirv.isDemoPaused()) {
			mirv.exec('quit');
		}
	});
}
