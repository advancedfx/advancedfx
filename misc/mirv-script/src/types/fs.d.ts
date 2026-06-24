/**
 * Since HLAE 2.191.0.
 */
declare class AdvancedfxFsDirEntry {
	path(): string;
}

/**
 * Since HLAE 2.191.0.
 */
declare class AdvancedfxFsReadDir {
	/**
	 * @throws {Error}
	 */
	next(): AdvancedfxFsDirEntry | undefined | never;
}

/**
 * Since HLAE 2.191.0.
 */
declare class AdvancedfxFs {
	readDir(path: string): AdvancedfxFsReadDir;
}
