use std::ffi::c_char;
use std::ffi::c_void;

pub type CVar = c_void;

#[repr(i16)]
pub enum CVarType {
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

impl std::convert::From<i16> for CVarType {
    fn from(item: i16) -> Self {
        match item {
            0 => {
                CVarType::Bool
            },
            1 => {
                CVarType::Int16
            },
            2 => {
               CVarType::UInt16
            },
            3 => {
                CVarType::Int32
            },
            4 => {
                CVarType::UInt32
            },
            5 => {
                CVarType::Int64
            },
            6 => {
                CVarType::UInt64
            },
            7 => {
                CVarType::Float32
            },
            8 => {
                CVarType::Float64
            },
            9 => {
                CVarType::String
            },
            10 => {
                CVarType::Color
            },
            11 => {
                CVarType::Vector2
            },
            12 => {
                CVarType::Vector3
            },
            13 => {
                CVarType::Vector4
            },
            14 => {
                CVarType::Qangle
            },
            _ => {
                CVarType::Invalid
            }
        }
    }
}

unsafe fn i16_to_cvar_type(value: i16) -> CVarType {
    unsafe { std::mem::transmute(value) }
}


#[repr(i8)]
pub enum CVarGetMode {
    Value = 0,
    DefaultValue = 1,
    MinValue = 2,
    MaxValue = 3
}

impl std::convert::From<i8> for CVarGetMode {
    fn from(item: i8) -> Self {
        match item {
            1 => {
                CVarGetMode::DefaultValue
            },
            2 => {
                CVarGetMode::MinValue
            },
            3 => {
                CVarGetMode::MaxValue
            },
            _ => {
                CVarGetMode::Value
            }
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Color {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector2 {
    pub x: f32,
    pub y: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct QAngle {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

extern "C" {
    pub(crate) fn afx_hook_source2_find_convar_index(psz_name: *const c_char) -> usize;
    
    pub(crate) fn afx_hook_source2_get_convar(index: usize) -> * mut CVar;

    pub(crate) fn afx_hook_source2_get_convar_type(p_cvar:  * mut CVar) -> i16;

    pub(crate) fn afx_hook_source2_get_convar_name(p_cvar:  * mut CVar, p_out_value: &mut *const c_char) -> bool;

    pub(crate) fn  afx_hook_source2_get_convar_help_string(p_cvar:  * mut CVar, p_out_value: &mut *const c_char) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_bool(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut bool) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_bool(p_cvar:  * mut CVar, mode: i8, value: bool) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_int(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut i32) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_int(p_cvar:  * mut CVar, mode: i8, value: i32) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_int64(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut i64) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_int64(p_cvar:  * mut CVar, mode: i8, value: i64) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_uint(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut u32) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_uint(p_cvar:  * mut CVar, mode: i8, value : u32) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_uint64(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut u64) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_uint64(p_cvar:  * mut CVar, mode: i8, value : u64) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_double(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut f64) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_double(p_cvar:  * mut CVar, mode: i8, value : f64) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_string(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut *const c_char) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_string(p_cvar:  * mut CVar, mode: i8, value : * const c_char) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_color(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut Color) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_color(p_cvar:  * mut CVar, mode: i8, value : * const Color) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_vec2(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut Vector2) -> bool;
    
    pub(crate) fn afx_hook_source2_set_convar_vec2(p_cvar:  * mut CVar, mode: i8, value : * const Vector2) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_vec3(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut Vector3) -> bool;
    
    pub(crate) fn afx_hook_source2_set_convar_vec3(p_cvar:  * mut CVar, mode: i8, value : * const Vector3) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_vec4(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut Vector4) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_vec4(p_cvar:  * mut CVar, mode: i8, value : * const Vector4) -> bool;

    pub(crate) fn afx_hook_source2_get_convar_qangle(p_cvar:  * mut CVar, mode: i8, p_out_value: &mut QAngle) -> bool;

    pub(crate) fn afx_hook_source2_set_convar_qangle(p_cvar:  * mut CVar, mode: i8, value : * const QAngle) -> bool;
}

pub(crate) fn afx_get_convar_type(p_cvar:  * mut CVar) -> CVarType {
    return unsafe { i16_to_cvar_type( afx_hook_source2_get_convar_type(p_cvar) ) };
}
