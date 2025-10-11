use crate::advancedfx;

use std::ffi::c_char;
use core::ffi::CStr;

use boa_engine::{
    class::{
        Class,
        ClassBuilder,
    },
    property::Attribute,
    js_string,
    Context,
    Finalize,
    JsData,
    JsNativeError,
    JsResult,
    JsValue,
    NativeFunction,
    object::builtins::{
        JsFloat32Array,
        JsUint8Array
    },

    Trace
};

#[derive(Trace, Finalize, JsData)]
pub struct CVar {
    #[unsafe_ignore_trace]
    pub native: * mut advancedfx::cvar::CVar
}

impl CVar {
    #[must_use]
    pub fn new(native: * mut advancedfx::cvar::CVar) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<CVar>()
            .expect("the AdvancedfxCVar builtin shouldn't exist");        
    }

    fn error_typ(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(), "'this' is not a AdvancedfxCVar object", context).into())
    }


    fn is_valid_index(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let JsValue::Integer(index) = args[0] {
                let result = unsafe {advancedfx::cvar::afx_hook_source2_is_convar_index_valid(index as usize)};
                return Ok(JsValue::Boolean(result));
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_value_ex(this: &JsValue, context: &mut Context, get_mode: advancedfx::cvar::CVarGetMode) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<CVar>() {
                let p_cvar = object_inner.native;
                let get_mode_i8 = get_mode as i8;
                let cvar_type = advancedfx::cvar::afx_get_convar_type(p_cvar);
                match cvar_type {
                    advancedfx::cvar::CVarType::Invalid => {
                        return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"invalid cvar type", context).into());
                    }
                    advancedfx::cvar::CVarType::Bool => {
                        let mut result : bool = false;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_bool(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Boolean(result));
                        }
                    }
                    advancedfx::cvar::CVarType::Int16 => {
                        let mut result : i32 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_int(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Integer(result));
                        }                        
                    }
                    advancedfx::cvar::CVarType::UInt16 => {
                        let mut result : u32 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_uint(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Integer(result as i32));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Int32 => {
                        let mut result : i32 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_int(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Integer(result));
                        }                        
                    }
                    advancedfx::cvar::CVarType::UInt32 => {
                        let mut result : u32 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_uint(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Integer(result as i32));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Int64 => {
                        let mut result : i64 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_int64(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::BigInt(result.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::UInt64 => {
                        let mut result : u64 = 0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_uint64(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::BigInt(result.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Float32 => {
                        let mut result : f64 = 0.0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_double(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Rational(result));
                        }                      
                    }
                    advancedfx::cvar::CVarType::Float64 => {
                        let mut result : f64 = 0.0;
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_double(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::Rational(result));
                        }                        
                    }
                    advancedfx::cvar::CVarType::String => {
                        let mut result: *const c_char = std::ptr::null();
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_string(p_cvar,get_mode_i8,&mut result)} {
                            return Ok(JsValue::String(js_string!(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string())));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Color => {
                        let mut result = advancedfx::cvar::Color{r:0,g:0,b:0,a:0};
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_color(p_cvar,get_mode_i8,&mut result)} {
                            let array = JsUint8Array::from_iter(vec![result.r, result.g, result.b, result.a], context)?;
                            return Ok(JsValue::Object(array.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Vector2 => {
                        let mut result = advancedfx::cvar::Vector2{x:0.0,y:0.0};
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_vec2(p_cvar,get_mode_i8,&mut result)} {
                            let array = JsFloat32Array::from_iter(vec![result.x, result.y], context)?;
                            return Ok(JsValue::Object(array.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Vector3 => {
                        let mut result = advancedfx::cvar::Vector3{x:0.0,y:0.0,z:0.0};
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_vec3(p_cvar,get_mode_i8,&mut result)} {
                            let array = JsFloat32Array::from_iter(vec![result.x, result.y, result.z], context)?;
                            return Ok(JsValue::Object(array.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Vector4 => {
                        let mut result = advancedfx::cvar::Vector4{x:0.0,y:0.0,z:0.0,w:0.0};
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_vec4(p_cvar,get_mode_i8,&mut result)} {
                            let array = JsFloat32Array::from_iter(vec![result.x, result.y, result.z,result.w], context)?;
                            return Ok(JsValue::Object(array.into()));
                        }                        
                    }
                    advancedfx::cvar::CVarType::Qangle => {
                        let mut result = advancedfx::cvar::QAngle{x:0.0,y:0.0,z:0.0};
                        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_qangle(p_cvar,get_mode_i8,&mut result)} {
                            let array = JsFloat32Array::from_iter(vec![result.x, result.y, result.z], context)?;
                            return Ok(JsValue::Object(array.into()));
                        }                        
                    }
                }
                return Ok(JsValue::Undefined);
            }
        }
        Self::error_typ(context)        
    }

    fn get_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        return Self::get_value_ex(this, context, advancedfx::cvar::CVarGetMode::Value);
    }   
    fn get_default_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        return Self::get_value_ex(this, context, advancedfx::cvar::CVarGetMode::DefaultValue);
    }   
    fn get_min_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        return Self::get_value_ex(this, context, advancedfx::cvar::CVarGetMode::MinValue);
    }   
    fn get_max_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        return Self::get_value_ex(this, context, advancedfx::cvar::CVarGetMode::MaxValue);
    } 

    fn get_type(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<CVar>() {
                let p_cvar = object_inner.native;
                let cvar_type = advancedfx::cvar::afx_get_convar_type(p_cvar);
                return Ok(JsValue::Integer(cvar_type as i32))
            }
        }
        Self::error_typ(context)
    }  
}

impl Class for CVar {
    const NAME: &'static str = "AdvancedfxCVar";
    const LENGTH: usize = 3;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        if 1 == args.len() {
            match &args[0] {
                JsValue::String(js_string) => {
                    let c_string = std::ffi::CString::new(js_string.to_std_string_escaped()).unwrap();
                    let index = unsafe { advancedfx::cvar::afx_hook_source2_find_convar_index(c_string.as_ptr()) };
                    let cvar = unsafe { advancedfx::cvar::afx_hook_source2_get_convar(index) };
                    if cvar.is_null() {
                        return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"not a valid cvar", context).into());
                    }
                    return Ok(CVar::new(cvar));
                }
                JsValue::Integer(index) => {
                    let cvar = unsafe { advancedfx::cvar::afx_hook_source2_get_convar(*index as usize) };
                    if cvar.is_null() {
                        return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"not a valid cvar", context).into());
                    }
                    return Ok(CVar::new(cvar));                
                }
                _=> {

                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .static_method(
                js_string!("isValidIndex"),
                1,
                NativeFunction::from_fn_ptr(CVar::is_valid_index),
            )
            .accessor(
                js_string!("value"),
                Some(NativeFunction::from_fn_ptr(CVar::get_value).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("defaultValue"),
                Some(NativeFunction::from_fn_ptr(CVar::get_default_value).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("minValue"),
                Some(NativeFunction::from_fn_ptr(CVar::get_min_value).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("maxValue"),
                Some(NativeFunction::from_fn_ptr(CVar::get_max_value).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .method(
                js_string!("getType"),
                1,
                NativeFunction::from_fn_ptr(CVar::get_type),
            )
        ;
        Ok(())
    }
}


