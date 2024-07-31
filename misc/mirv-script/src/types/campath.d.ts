declare class AdvancedfxCampathValue {
	constructor(
		pos: AdvancedfxMathVector3,
		rot: AdvancedfxMathQuaternion,
		fov: number,
		selected: boolean
	);

	pos: AdvancedfxMathVector3;

	rot: AdvancedfxMathQuaternion;

	fov: number;

	selected: boolean;
}

declare class AdvancedfxCampathIterator {
	constructor(campath: AdvancedfxCampath);

	readonly valid: boolean;

	readonly time: number;

	readonly value: AdvancedfxCampathValue;

	next(): void;
}

declare namespace AdvancedfxCampath {
	enum DoubleInterp {
		Default = 0,
		Linear = 1,
		Cubic = 2
	}

	enum QuaternionInterp {
		Default = 0,
		SLinear = 1,
		SCubic = 2
	}

	type OnEvent = () => void;
}

declare class AdvancedfxCampath {
	constructor();

	enabled: boolean;

	offset: number;

	hold: boolean;

	positionInterp: AdvancedfxCampath.DoubleInterp;

	rotationInterp: AdvancedfxCampath.QuaternionInterp;

	fovInterp: AdvancedfxCampath.DoubleInterp;

	/**
	 * Add keyframe.
	 * @param time Time in seconds.
	 * @remarks If keyframes happen to fall on same time, the last one wins.
	 */
	add(time: number, value: AdvancedfxCampathValue): void;

	/**
	 * Remove keyframe.
	 */
	remove(time: number): void;

	/**
	 * @remarks
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	clear(): void;

	/**
	 * Number of keyframes.
	 */
	readonly size: number;

	/**
	 * Length of path in seconds.
	 */
	readonly duration: number;

	/**
	 * Start time if any, otherwise undefined
	 */
	readonly lowerBound: number | undefined;

	/**
	 * End time if any, otherwise undefined.
	 */
	readonly upperBound: number | undefined;

	/**
	 * If the path is defined / can be evaluated.
	 */
	readonly canEval: boolean;

	eval(time: number): AdvancedfxCampathValue | undefined;

	load(filePath: string): boolean;

	save(filePath: string): boolean;

	/**
	 * @remarks
	 * If keyframes happen to fall on same time, the last one wins.
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setStart(time: number): void;

	/**
	 * @remarks
	 * If keyframes happen to fall on same time, the last one wins.
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setStart(time: number, relative: boolean): void;

	/**
	 * @remarks
	 * If keyframes happen to fall on same time, the last one wins.
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setDuration(time: number): void;

	/**
	 * @remarks
	 * Undefined arguments remain unchanged.
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setPosition(x: number | undefined, y: number | undefined, z: number | undefined): void;

	/**
	 * @remarks
	 * Undefined arguments remain unchanged.
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setAngles(
		yPitch: number | undefined,
		zYaw: number | undefined,
		xRoll: number | undefined
	): void;

	/**
	 * @remarks
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	setFov(fov: number): void;

	/**
	 * @remarks
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	rotate(yPitch: number, zYaw: number, xRoll: number): void;

	/**
	 * @remarks
	 * Applies to currently selected keyframes, otherwise all keyframes.
	 */
	anchorTransform(
		anchorX: number,
		anchorY: number,
		anchorZ: number,
		anchorYPitch: number,
		anchorZYaw: number,
		anchorXRoll: number,
		destX: number,
		destY: number,
		destZ: number,
		destYPitch: number,
		destZYaw: number,
		destXRoll: number
	): void;

	/**
	 * @returns Total number of selected keyframes.
	 */
	selectAll(): number;

	selectNone(): void;

	/**
	 * @returns Total number of selected keyframes.
	 */
	selectInvert(): number;

	/**
	 * @param min First index
	 * @param max Last index
	 * @returns Total number of selected keyframes.
	 */
	selectAddIdx(min: number, max: number): number;

	/**
	 * Adds a range of key frames to the selection.
	 * @param min Lower time bound to start adding selection at.
	 * @param count Number of keyframes to select.
	 * @returns Number of selected keyframes.
	 */
	selectAddMinCount(min: number, count: number): number;

	/**
	 * Adds a range of key frames to the selection.
	 * @param min Lower time bound to start adding selection at.
	 * @param max Upper time bound to end adding selection at.
	 * @returns Number of selected keyframes.
	 */
	selectAddMinMax(min: number, max: number): number;

	/**
	 * @remarks Be careful to avoid infinite recursion e.g. due to changing the campath in onChanged event.
	 */
	onChanged: AdvancedfxCampath.OnEvent | undefined;
}
