/**
 * Quake vector.
 */
declare class AdvancedfxMathVector3 {
	constructor(x: number, y: number, z: number);

	x: number;

	y: number;

	z: number;

	readonly length: number;

	readonly normalized: number;

	add(other: AdvancedfxMathVector3): AdvancedfxMathVector3;

	addAssign(other: AdvancedfxMathVector3): void;

	sub(other: AdvancedfxMathVector3): AdvancedfxMathVector3;

	subAssign(other: AdvancedfxMathVector3): void;

	leftMul(value: number): AdvancedfxMathVector3;

	leftMulAssign(value: number): void;
}

/**
 * Quake euler angles in degrees.
 */
declare class AdvancedfxMathQEulerAngles {
	constructor(pitch: number, yaw: number, roll: number);

	pitch: number;

	yaw: number;

	roll: number;
}

/**
 * Quake euler angles in radians.
 */
declare class AdvancedfxMathQREulerAngles {
	constructor(pitch: number, yaw: number, roll: number);

	pitch: number;

	yaw: number;

	roll: number;

	toQEulerAngles(): AdvancedfxMathQEulerAngles;

	static fromQEulerAngles(value: AdvancedfxMathQEulerAngles): AdvancedfxMathQREulerAngles;
}

declare class AdvancedfxMathQuaternion {
	constructor(w: number, x: number, y: number, z: number);

	w: number;

	x: number;

	y: number;

	z: number;

	dot(other: AdvancedfxMathQuaternion): AdvancedfxMathQuaternion;

	readonly norm: number;

	readonly normalized: AdvancedfxMathQuaternion;

	readonly conjugate: AdvancedfxMathQuaternion;

	getAng(other: AdvancedfxMathQuaternion): AdvancedfxMathQuaternion;

	slerp(other: AdvancedfxMathQuaternion, t: number): AdvancedfxMathQuaternion;

	add(other: AdvancedfxMathQuaternion): AdvancedfxMathQuaternion;

	sub(other: AdvancedfxMathQuaternion): AdvancedfxMathQuaternion;

	mul(other: AdvancedfxMathQuaternion): AdvancedfxMathQuaternion;

	leftMul(value: number): AdvancedfxMathQuaternion;

	toQREulerAngles(): AdvancedfxMathQREulerAngles;

	static fromQREulerAngles(value: AdvancedfxMathQREulerAngles): AdvancedfxMathQuaternion;
}
