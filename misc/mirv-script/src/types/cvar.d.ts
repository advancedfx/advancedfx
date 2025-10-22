declare namespace AdvancedfxCVar {
	enum TypeEnum {
		Invalid = -1,
		Bool = 0,
		Int16 = 1,
		UInt16 = 2,
		Int32 = 3,
		UInt32 = 4,
		Int64 = 5,
		UInt64 = 6,
		Float32 = 7,
		Float64 = 8,
		String = 9,
		Color = 10,
		Vector2 = 11,
		Vector3 = 12,
		Vector4 = 13,
		Qangle = 14
	}

	type Vector2 = number[2];

	type Vector3 = number[3];

	type Vector4 = number[4];

	/**
	 * Type mappings (JS - AdvancedfxCVar.TypeEnum):
	 * boolean - Bool,
	 * number - Int16, UInt16, Int32, UInt32, Float32, Float64
	 * bigint - Int64, UInt64
	 * string - String
	 * Vector2 - Vector2(x,y)
	 * Vector3 - Vector3(x,y,z), Qangle(x,y,z)
	 * Vector4 - Vector4(x,y,z,w), Color(r,g,b,a)
	 */
	type Type = boolean | number | bigint | string | Vector2 | Vector3 | Vector4;
}

declare class AdvancedfxCVar {
	/**
	 * Find a non-hidden cvar by name.
	 * @remarks Hidden cvars can NOT be accessed this way.
	 */
	static getIndexFromName(name: string): number | undefined;

	/**
	 * Find a cvar by index
	 * @remarks Hidden cvars CAN be accessed this way.
	 * @param index Hint: currently sane possible values are in [0,8191].
	 * @throws Error
	 * If cvar can not be found for this index (yet).
	 */
	constructor(index: number);

	getType(): AdvancedfxCVar.TypeEnum;

	readonly name: string;

	readonly helpString: string | undefined;

	/**
	 * get / set CVar value
	 * @remarks for type mappings see AdvancedfxCVar.Type.
	 */
	value: AdvancedfxCVar.Type;

	/**
	 * get CVar default value
	 * @remarks for type mappings see AdvancedfxCVar.Type.
	 */
	readonly defaultValue: AdvancedfxCVar.Type;

	/**
	 * get CVar minimum value (if any is set).
	 * @remarks for type mappings see AdvancedfxCVar.Type.
	 */
	readonly minValue: AdvancedfxCVar.Type | undefined;

	/**
	 * get CVar maximum value (if any is set).
	 * @remarks for type mappings see AdvancedfxCVar.Type.
	 */
	readonly maxValue: AdvancedfxCVar.Type | undefined;
}
