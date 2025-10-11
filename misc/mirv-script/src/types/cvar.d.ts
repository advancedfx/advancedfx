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
     * BigInt - Int64, Int64
     * string - String
     * Color - Color
     * Vector2 - Vector2
     * Vector3 - Vector3, Qangle
     * Vector4 - Vector4
     */
    type Type = boolean | number | bigint | string | Color | Vector2 | Vector3 | Vector4;
}

declare class AdvancedfxCVar {

    /**
     * Determine if there's is a cvar yet with this index.
     * @param index the index of the cvar, typical values are in [0,65535].
     */
    static isValidIndex(index: number): boolean;

    /**
     * Find a cvar by name
     * @throws Error
     * If cvar can not be found (yet).
     */
    constructor(name: string);

    /**
     * Find a cvar by index
     * @remarks Use AdvancedfxCVar.isValidIndex to determine if an index is valid in advance.
     * @throws Error
     * If cvar can not be found (yet).
     */
    constructor(index: number);

    getType(): AdvancedfxCVar.TypeEnum;

    /**
     * get CVar value
     * @remarks for type mappings see AdvancedfxCVar.Type.
     */
    readonly value: AdvancedfxCVar.Type;

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
