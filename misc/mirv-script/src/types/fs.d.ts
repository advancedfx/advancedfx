/**
 * Since HLAE 2.191.0.
 */
declare class AdvancedfxPath {
	constructor(path: string);

	toString(): string;

	isAbsolute(): boolean;

	isRelative(): boolean;

	isDir(): boolean;

	isFile(): boolean;

	isSymlink(): boolean;

	static MAIN_SEPARATOR: string;
}

/**
 * Since HLAE 2.191.0.
 */
declare class AdvancedfxFsDirEntry {
	path(): AdvancedfxPath;
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
	static readDir(path: string): AdvancedfxFsReadDir;
}
