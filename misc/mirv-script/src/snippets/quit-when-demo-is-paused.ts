mirv.onClientFrameStageNotify = () => {
	if (mirv.isPlayingDemo() && mirv.isDemoPaused()) {
		mirv.exec('quit');
	}
};
