declare namespace SOURCESDK_CS2 {
	const enum ClientFrameStage_t {
		// There are more values in-between, but their meanings have changed and we did not confirm them yet.
		// These can also change between updates
		FRAME_UNDEFINED = -1,
		FRAME_START = 0,
		FRAME_RENDER_PASS = 10
	}
}
