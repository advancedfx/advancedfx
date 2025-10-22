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
    JsError,
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

    fn get_index_from_name(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 != args.len() { return Err(advancedfx::js::errors::error_arguments(context).into()) };

        let value = args[0].to_string(context).or_else(|_| Err(advancedfx::js::errors::error_arguments(context)))?;

        let c_string = std::ffi::CString::new(value.to_std_string_escaped()).unwrap();
        let index = unsafe {advancedfx::cvar::afx_hook_source2_find_convar_index(c_string.as_ptr())};
        if usize::MAX == index {
            return Ok(JsValue::Undefined);
        }

        Ok(JsValue::Integer(index as i32))
    }

    fn get_value_ex(this: &JsValue, context: &mut Context, get_mode: advancedfx::cvar::CVarGetMode) -> JsResult<JsValue> {
        let object = this.as_object().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        let object_inner = object.downcast_ref::<CVar>().ok_or_else(|| Self::error_typ(context).unwrap_err())?;

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

        Ok(JsValue::Undefined)
    }

    fn set_value_error(context: &mut Context) -> JsError {
        return advancedfx::js::errors::make_error!(JsNativeError::error(),"could not set cvar value", context).into();
    }

    fn set_value_ex(this: &JsValue, args: &[JsValue], context: &mut Context, get_mode: advancedfx::cvar::CVarGetMode) -> JsResult<JsValue> {
        let object = this.as_object().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        let object_inner = object.downcast_ref::<CVar>().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        if 1 != args.len() { return Err(advancedfx::js::errors::error_arguments(context).into()) };

        let p_cvar = object_inner.native;
        let get_mode_i8 = get_mode as i8;
        let cvar_type = advancedfx::cvar::afx_get_convar_type(p_cvar);

        let arg0 = &args[0];
        match cvar_type {
            advancedfx::cvar::CVarType::Invalid => {
                return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"invalid cvar type", context).into());
            }
            advancedfx::cvar::CVarType::Bool => {
               let value = arg0.to_boolean();
               if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_bool(p_cvar, get_mode_i8, value)} {
                   return Err(Self::set_value_error(context));
               }
               return Ok(JsValue::Undefined);                                
            }
            advancedfx::cvar::CVarType::Int16 => {
                if let Ok(value) = arg0.to_int16(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_int(p_cvar, get_mode_i8, value as i32)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }
            }
            advancedfx::cvar::CVarType::UInt16 => {
                if let Ok(value) = arg0.to_uint16(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_uint(p_cvar, get_mode_i8, value as u32)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }
            }
            advancedfx::cvar::CVarType::Int32 => {
                if let Ok(value) = arg0.to_i32(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_int(p_cvar, get_mode_i8, value)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }
            }
            advancedfx::cvar::CVarType::UInt32 => {
                if let Ok(value) = arg0.to_u32(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_uint(p_cvar, get_mode_i8, value)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }
            }
            advancedfx::cvar::CVarType::Int64 => {
                if let Ok(value) = arg0.to_big_int64(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_int64(p_cvar, get_mode_i8, value)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }
            }
            advancedfx::cvar::CVarType::UInt64 => {
                if let Ok(value) = arg0.to_big_uint64(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_uint64(p_cvar, get_mode_i8, value)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }                       
            }
            advancedfx::cvar::CVarType::Float32 => {
                if let Ok(value) = arg0.to_f32(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_double(p_cvar, get_mode_i8, value as f64)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }                        
            }
            advancedfx::cvar::CVarType::Float64 => {
                if let Ok(value) = arg0.to_number(context) {
                    if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_double(p_cvar, get_mode_i8, value)} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);                                
                }                         
            }
            advancedfx::cvar::CVarType::String => {
                if let Ok(value) = arg0.to_string(context) {
                    let c_string = std::ffi::CString::new(value.to_std_string_escaped()).unwrap();
                    if !unsafe {advancedfx::cvar::afx_hook_source2_set_convar_string(p_cvar,get_mode_i8, c_string.as_ptr())} {
                        return Err(Self::set_value_error(context));
                    }
                    return Ok(JsValue::Undefined);
                }                                   
            }
            advancedfx::cvar::CVarType::Color => {
                if let JsValue::Object(value) = arg0 {
                    if let Ok(array) = JsUint8Array::from_object(value.clone()) {
                        let vec: Vec<u8> = array.iter(context).collect();
                        if vec.len() == 4 {
                            let value = advancedfx::cvar::Color{r:vec[0],g:vec[1],b:vec[2],a:vec[3]};
                            if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_color(p_cvar, get_mode_i8, &value)} {
                                return Err(Self::set_value_error(context));
                            }
                            return Ok(JsValue::Undefined);
                        }
                    }
                }
            }
            advancedfx::cvar::CVarType::Vector2 => {
                if let JsValue::Object(value) = arg0 {
                    if let Ok(array) = JsFloat32Array::from_object(value.clone()) {
                        let vec: Vec<f32> = array.iter(context).collect();
                        if vec.len() == 2 {
                            let value = advancedfx::cvar::Vector2{x:vec[0],y:vec[1]};
                            if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_vec2(p_cvar, get_mode_i8, &value)} {
                                return Err(Self::set_value_error(context));
                            }
                            return Ok(JsValue::Undefined);
                        }
                    }                                
                }
            }
            advancedfx::cvar::CVarType::Vector3 => {
                if let JsValue::Object(value) = arg0 {
                    if let Ok(array) = JsFloat32Array::from_object(value.clone()) {
                        let vec: Vec<f32> = array.iter(context).collect();
                        if vec.len() == 3 {
                            let value = advancedfx::cvar::Vector3{x:vec[0],y:vec[1],z:vec[2]};
                            if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_vec3(p_cvar, get_mode_i8, &value)} {
                                return Err(Self::set_value_error(context));
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } 
                }
            }
            advancedfx::cvar::CVarType::Vector4 => {
                if let JsValue::Object(value) = arg0 {
                    if let Ok(array) = JsFloat32Array::from_object(value.clone()) {
                        let vec: Vec<f32> = array.iter(context).collect();
                        if vec.len() == 4 {
                            let value = advancedfx::cvar::Vector4{x:vec[0],y:vec[1],z:vec[2],w:vec[3]};
                            if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_vec4(p_cvar, get_mode_i8, &value)} {
                                return Err(Self::set_value_error(context));
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } 
                }
            }
            advancedfx::cvar::CVarType::Qangle => {
                if let JsValue::Object(value) = arg0 {
                    if let Ok(array) = JsFloat32Array::from_object(value.clone()) {
                        let vec: Vec<f32> = array.iter(context).collect();
                        if vec.len() == 3 {
                            let value = advancedfx::cvar::QAngle{x:vec[0],y:vec[1],z:vec[2]};
                            if !unsafe{advancedfx::cvar::afx_hook_source2_set_convar_qangle(p_cvar, get_mode_i8, &value)} {
                                return Err(Self::set_value_error(context));
                            }
                            return Ok(JsValue::Undefined);
                        }
                    }    
                }
            }
        }

        Ok(JsValue::Undefined)
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

    fn set_value(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        return Self::set_value_ex(this, args, context, advancedfx::cvar::CVarGetMode::Value);
    }   

    fn get_type(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let object = this.as_object().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        let object_inner = object.downcast_ref::<CVar>().ok_or_else(|| Self::error_typ(context).unwrap_err())?;

        let p_cvar = object_inner.native;
        let cvar_type = advancedfx::cvar::afx_get_convar_type(p_cvar);

        Ok(JsValue::Integer(cvar_type as i32))
    }

    fn get_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let object = this.as_object().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        let object_inner = object.downcast_ref::<CVar>().ok_or_else(|| Self::error_typ(context).unwrap_err())?;

        let p_cvar = object_inner.native;
        let mut result: *const c_char = std::ptr::null();
        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_name(p_cvar,&mut result)} {
            if !result.is_null() {
                return Ok(JsValue::String(js_string!(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string())));
            }
        }

        Ok(JsValue::Undefined) // This probably will never happen, just here in case.
    }
    fn get_help_string(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let object = this.as_object().ok_or_else(|| Self::error_typ(context).unwrap_err())?;
        let object_inner = object.downcast_ref::<CVar>().ok_or_else(|| Self::error_typ(context).unwrap_err())?;

        let p_cvar = object_inner.native;
        let mut result: *const c_char = std::ptr::null();
        if unsafe {advancedfx::cvar::afx_hook_source2_get_convar_help_string(p_cvar,&mut result)} {
            if !result.is_null() {
                return Ok(JsValue::String(js_string!(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string())));
            }
        }

        Ok(JsValue::Undefined)
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
        if 1 != args.len() { return Err(advancedfx::js::errors::error_arguments(context).into()) };

        let index = args[0].to_i32(context).or_else(|_| Err(advancedfx::js::errors::error_arguments(context)))?;

        let cvar = unsafe { advancedfx::cvar::afx_hook_source2_get_convar(index as usize) };
        if cvar.is_null() {
            return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"not a valid cvar", context).into());
        }

        Ok(CVar::new(cvar))
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .static_method(
                js_string!("getIndexFromName"),
                1,
                NativeFunction::from_fn_ptr(CVar::get_index_from_name),
            )        
            .accessor(
                js_string!("name"),
                Some(NativeFunction::from_fn_ptr(CVar::get_name).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("helpString"),
                Some(NativeFunction::from_fn_ptr(CVar::get_help_string).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("value"),
                Some(NativeFunction::from_fn_ptr(CVar::get_value).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(CVar::set_value).to_js_function(&realm)),
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


