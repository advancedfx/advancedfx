mirv.onClientFrameStageNotify = function (e) {
	if (mirv.isPlayingDemo() && mirv.isDemoPaused()) {
		mirv.exec('quit');
	}
};
