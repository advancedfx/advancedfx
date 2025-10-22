#include "stdafx.h"

#include "../shared/FFITools.h"

#include "../shared/AfxConsole.h"

#include "../deps/release/prop/cs2/sdk_src/public/tier1/convar.h"
#include "../deps/release/prop/cs2/sdk_src/public/icvar.h"

typedef SOURCESDK::CS2::Cvar_s CVarRs_t;

extern "C" size_t afx_hook_source2_find_convar_index(const char * psz_name) {
    if(SOURCESDK::CS2::g_pCVar) {
        SOURCESDK::CS2::ConVarHandle handle = SOURCESDK::CS2::g_pCVar->FindConVar(psz_name, false);
        return handle.IsValid() ? handle.Get() : -1;
    }
    return -1;
}

extern "C" CVarRs_t * afx_hook_source2_get_convar(size_t index) {
    if(SOURCESDK::CS2::g_pCVar
        && index < SOURCESDK_CS2_MAX_VALID_CVARS // otherwise crash / invalid pointer ... :)
    ) {
        return SOURCESDK::CS2::g_pCVar->GetCvar(index);
    }

    return nullptr;
}

enum class CVarTypeRs_e : int16_t {
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
};

extern "C" int16_t afx_hook_source2_get_convar_type(CVarRs_t * p_cvar) {
    if(p_cvar) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            return static_cast<int16_t>(CVarTypeRs_e::Bool);
        case SOURCESDK::CS2::EConVarType_Int16:
            return static_cast<int16_t>(CVarTypeRs_e::Int16);
        case SOURCESDK::CS2::EConVarType_UInt16:
            return static_cast<int16_t>(CVarTypeRs_e::UInt16);
        case SOURCESDK::CS2::EConVarType_Int32:
            return static_cast<int16_t>(CVarTypeRs_e::Int32);
        case SOURCESDK::CS2::EConVarType_UInt32:
            return static_cast<int16_t>(CVarTypeRs_e::UInt32);
        case SOURCESDK::CS2::EConVarType_Int64:
            return static_cast<int16_t>(CVarTypeRs_e::Int64);
        case SOURCESDK::CS2::EConVarType_UInt64:
            return static_cast<int16_t>(CVarTypeRs_e::UInt64);
        case SOURCESDK::CS2::EConVarType_Float32:
            return static_cast<int16_t>(CVarTypeRs_e::Float32);
        case SOURCESDK::CS2::EConVarType_Float64:
            return static_cast<int16_t>(CVarTypeRs_e::Float64);
        case SOURCESDK::CS2::EConVarType_String:
            return static_cast<int16_t>(CVarTypeRs_e::String);
        case SOURCESDK::CS2::EConVarType_Color:
            return static_cast<int16_t>(CVarTypeRs_e::Color);
        case SOURCESDK::CS2::EConVarType_Vector2:
            return static_cast<int16_t>(CVarTypeRs_e::Vector2);
        case SOURCESDK::CS2::EConVarType_Vector3:
            return static_cast<int16_t>(CVarTypeRs_e::Vector3);
        case SOURCESDK::CS2::EConVarType_Vector4:
            return static_cast<int16_t>(CVarTypeRs_e::Vector4);
        case SOURCESDK::CS2::EConVarType_Qangle:
            return static_cast<int16_t>(CVarTypeRs_e::Qangle);
        }
    }
    return static_cast<int16_t>(CVarTypeRs_e::Invalid);
}

extern "C" FFIBool afx_hook_source2_get_convar_name(CVarRs_t * p_cvar, const char * &outValue) {
    if(p_cvar) {
        outValue = p_cvar->m_pszName;
        return FFIBOOL_TRUE;
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_help_string(CVarRs_t * p_cvar, const char * &outValue) {
    if(p_cvar) {
        outValue = p_cvar->m_pszHelpString;
        return FFIBOOL_TRUE;
    }
    return FFIBOOL_FALSE;
}

enum class CVarGetMode_e : int8_t {
    Value = 0,
    DefaultValue = 1,
    MinValue = 2,
    MaxValue = 3
};

SOURCESDK::CS2::CVValue_t * GetCvarValue(CVarRs_t * p_cvar, CVarGetMode_e mode) {
    if(p_cvar) {
        switch(mode) {
        case CVarGetMode_e::Value:
            return &(p_cvar->m_Value);
        case CVarGetMode_e::DefaultValue:
            return p_cvar->m_defaultValue;
        case CVarGetMode_e::MinValue:
            return p_cvar->m_minValue;
        case CVarGetMode_e::MaxValue:
            return p_cvar->m_maxValue;
        }
    }
    return nullptr;
}

extern "C" FFIBool afx_hook_source2_get_convar_bool(CVarRs_t * p_cvar, CVarGetMode_e mode, FFIBool & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = BOOL_TO_FFIBOOL(p_value->m_bValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_i16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_u16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_i32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_u32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_i64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_u64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_flValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = BOOL_TO_FFIBOOL(0 != p_value->m_dbValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_bool(CVarRs_t * p_cvar, CVarGetMode_e mode, FFIBool value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = FFIBOOL_TO_BOOL(value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = FFIBOOL_TO_BOOL(value) ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = FFIBOOL_TO_BOOL(value) ? 1.0f : 0.0f;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = FFIBOOL_TO_BOOL(value) ? 1.0 : 0.0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_int(CVarRs_t * p_cvar, CVarGetMode_e mode, signed int & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = p_value->m_bValue ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = p_value->m_i16Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = (signed int)(p_value->m_u16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = p_value->m_i32Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = (signed int)(p_value->m_u32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = (signed int)(p_value->m_i64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = (signed int)(p_value->m_u64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = (signed int)(p_value->m_flValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = (signed int)(p_value->m_dbValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_int(CVarRs_t * p_cvar, CVarGetMode_e mode, signed int value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = 0 != value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = (signed short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = (unsigned short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = (signed int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = (unsigned int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = (uint64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = (float)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = (double)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_int64(CVarRs_t * p_cvar, CVarGetMode_e mode, int64_t & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = p_value->m_bValue ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = p_value->m_i16Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = (int64_t)(p_value->m_u16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = p_value->m_i32Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = (int64_t)(p_value->m_u32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = p_value->m_i64Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = (int64_t)(p_value->m_u64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = (int64_t)(p_value->m_flValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = (int64_t)(p_value->m_dbValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_int64(CVarRs_t * p_cvar, CVarGetMode_e mode, int64_t value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = 0 != value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = (signed short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = (unsigned short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = (signed int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = (unsigned int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = (uint64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = (float)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = (double)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_uint(CVarRs_t * p_cvar, CVarGetMode_e mode, unsigned int & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = p_value->m_bValue ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = (unsigned int)(p_value->m_i16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = p_value->m_u16Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = (unsigned int)(p_value->m_i32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = p_value->m_u32Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = (unsigned int)p_value->m_i64Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = (unsigned int)p_value->m_u64Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = (unsigned int)p_value->m_flValue;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = (unsigned int)p_value->m_dbValue;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_uint(CVarRs_t * p_cvar, CVarGetMode_e mode, unsigned int value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = 0 != value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = (signed short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = (unsigned short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = (signed int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = (unsigned short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = (int64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = (uint64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = (float)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = (double)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_uint64(CVarRs_t * p_cvar, CVarGetMode_e mode, uint64_t & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = p_value->m_bValue ? 1 : 0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = (uint64_t)(p_value->m_i16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = p_value->m_u16Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = (uint64_t)(p_value->m_i32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = p_value->m_u32Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = (uint64_t)(p_value->m_i64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = p_value->m_u64Value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = (uint64_t)(p_value->m_flValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = (uint64_t)(p_value->m_dbValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_uint64(CVarRs_t * p_cvar, CVarGetMode_e mode, uint64_t value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = 0 != value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = (signed short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = (unsigned short int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = (signed int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = (unsigned int)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = (int64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = (float)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = (double)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_double(CVarRs_t * p_cvar, CVarGetMode_e mode, double & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            outValue = p_value->m_bValue ? 1.0 : 0.0;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            outValue = (double)(p_value->m_i16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            outValue = (double)(p_value->m_u16Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            outValue = (double)(p_value->m_i32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            outValue = (double)(p_value->m_u32Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            outValue = (double)(p_value->m_i64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            outValue = (double)(p_value->m_u64Value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            outValue = (double)(p_value->m_flValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            outValue = (double)(p_value->m_dbValue);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_double(CVarRs_t * p_cvar, CVarGetMode_e mode, double value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            p_value->m_bValue = 0 != value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int16:
            p_value->m_i16Value = (int16_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt16:
            p_value->m_u16Value = (uint16_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int32:
            p_value->m_i32Value = (int32_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt32:
            p_value->m_u32Value = (uint32_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Int64:
            p_value->m_i64Value = (int64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_UInt64:
            p_value->m_u64Value = (uint64_t)value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float32:
            p_value->m_flValue = (float)value;;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Float64:
            p_value->m_dbValue = value;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_get_convar_string(CVarRs_t * p_cvar, CVarGetMode_e mode, const char * & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            outValue = p_value->m_szValue.Get();
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_string(CVarRs_t * p_cvar, CVarGetMode_e mode, const char * value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            p_value->m_szValue.Set(value);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}


struct AfxHookSource2RsColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

extern "C" FFIBool afx_hook_source2_get_convar_color(CVarRs_t * p_cvar, CVarGetMode_e mode, AfxHookSource2RsColor & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            outValue.r = p_value->m_clrValue.r();
            outValue.g = p_value->m_clrValue.g();
            outValue.b = p_value->m_clrValue.b();
            outValue.a = p_value->m_clrValue.a();
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_color(CVarRs_t * p_cvar, CVarGetMode_e mode, const AfxHookSource2RsColor & value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            p_value->m_clrValue.SetColor(value.r,value.g,value.b,value.a);
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

struct AfxHookSource2RsVec2{
    float x;
    float y;
};

extern "C" FFIBool afx_hook_source2_get_convar_vec2(CVarRs_t * p_cvar, CVarGetMode_e mode, AfxHookSource2RsVec2 & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            outValue.x = p_value->m_vec2Value.x;
            outValue.y = p_value->m_vec2Value.y;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_vec2(CVarRs_t * p_cvar, CVarGetMode_e mode, const AfxHookSource2RsVec2 & value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            p_value->m_vec2Value.x = value.x;
            p_value->m_vec2Value.y = value.y;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

struct AfxHookSource2RsVec3 {
    float x;
    float y;
    float z;
};

extern "C" FFIBool afx_hook_source2_get_convar_vec3(CVarRs_t * p_cvar, CVarGetMode_e mode, AfxHookSource2RsVec3 & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            outValue.x = p_value->m_vec3Value.x;
            outValue.y = p_value->m_vec3Value.y;
            outValue.z = p_value->m_vec3Value.z;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_vec3(CVarRs_t * p_cvar, CVarGetMode_e mode, const AfxHookSource2RsVec3 & value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            p_value->m_vec3Value.x = value.x;
            p_value->m_vec3Value.y = value.y;
            p_value->m_vec3Value.z = value.z;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

struct AfxHookSource2RsVec4 {
    float x;
    float y;
    float z;
    float w;
};

extern "C" FFIBool afx_hook_source2_get_convar_vec4(CVarRs_t * p_cvar, CVarGetMode_e mode, AfxHookSource2RsVec4 & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            outValue.x = p_value->m_vec4Value.x;
            outValue.y = p_value->m_vec4Value.y;
            outValue.z = p_value->m_vec4Value.z;
            outValue.w = p_value->m_vec4Value.w;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_vec4(CVarRs_t * p_cvar, CVarGetMode_e mode, const AfxHookSource2RsVec4 & value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            p_value->m_vec4Value.x = value.x;
            p_value->m_vec4Value.y = value.y;
            p_value->m_vec4Value.z = value.z;
            p_value->m_vec4Value.w = value.w;
            return FFIBOOL_TRUE;
        case SOURCESDK::CS2::EConVarType_Qangle:
            break; // undefined
        }
    }
    return FFIBOOL_FALSE;
}

struct AfxHookSource2RsQangle {
    float x;
    float y;
    float z;
};

extern "C" FFIBool afx_hook_source2_get_convar_qangle(CVarRs_t * p_cvar, CVarGetMode_e mode, AfxHookSource2RsQangle & outValue) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            outValue.x = p_value->m_angValue.x;
            outValue.y = p_value->m_angValue.y;
            outValue.z = p_value->m_angValue.z;
            return FFIBOOL_TRUE;
        }
    }
    return FFIBOOL_FALSE;
}

extern "C" FFIBool afx_hook_source2_set_convar_qangle(CVarRs_t * p_cvar, CVarGetMode_e mode, const AfxHookSource2RsQangle & value) {
    if(SOURCESDK::CS2::CVValue_t *p_value = GetCvarValue(p_cvar, mode)) {
        switch(p_cvar->m_eVarType) {
        case SOURCESDK::CS2::EConVarType_Bool:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt16:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Int64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_UInt64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float32:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Float64:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_String:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Color:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector2:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector3:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Vector4:
            break; // undefined
        case SOURCESDK::CS2::EConVarType_Qangle:
            p_value->m_angValue.x = value.x;
            p_value->m_angValue.y = value.y;
            p_value->m_angValue.z = value.z;
            return FFIBOOL_TRUE;
        }
    }
    return FFIBOOL_FALSE;
}
