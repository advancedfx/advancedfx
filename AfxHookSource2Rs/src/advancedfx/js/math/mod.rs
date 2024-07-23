use crate::advancedfx;

use boa_engine::{
    class::{
        Class,
        ClassBuilder,
    },
    object::builtins::JsArray,
    property::Attribute,
    js_string,
    Context,
    Finalize,
    JsData,
    JsNativeError,
    JsObject,
    JsResult,
    JsValue,
    NativeFunction,
    Trace
};

#[derive(Trace, Finalize, JsData)]
pub struct Vector3 {
    #[unsafe_ignore_trace]
    pub native: advancedfx::math::Vector3
}

impl Vector3 {
    #[must_use]
    pub fn new(native: advancedfx::math::Vector3) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Vector3>()
            .expect("the AdvancedfxMathVector3 builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxMathVector3 object")
            .into())
    }    

    fn get_x(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                return Ok(JsValue::Rational(object_inner.native.x));
            }
        }
        Self::error_typ()
    }   

    fn set_x(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.x = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_y(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                return Ok(JsValue::Rational(object_inner.native.y));
            }
        }
        Self::error_typ()
    }   

    fn set_y(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                    object_inner.native.y = value;
                    return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_z(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                return Ok(JsValue::Rational(object_inner.native.z));
            }
        }
        Self::error_typ()
    }   

    fn set_z(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.z = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn length(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                return Ok(JsValue::Rational(object_inner.native.length()));
            }
        }
        Self::error_typ()
    }

    fn normalized(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                match object_inner.native.normalized() {
                    Ok(value) => {
                        match Vector3::from_data(Vector3::new(value), context) {
                            Ok(result_object) => {
                                return Ok(JsValue::Object(result_object));
                            }
                            Err(e) => {
                                return Err(e);
                            }
                        }
                    }
                    Err(e) => {
                        return Err(JsNativeError::error().with_message(e.to_string()).into());
                    }
                }

            }
        }
        Self::error_typ()
    }

    fn add(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Vector3>() {
                            match Vector3::from_data(Vector3::new(object_inner.native + value_object_inner.native), context) {
                                Ok(result_object) => {
                                    return Ok(JsValue::Object(result_object));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn add_assign(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if JsObject::equals(&object,value_object) {
                            let mut native: advancedfx::math::Vector3 = object_inner.native;
                            native += native;
                        } else {
                            if let Some(value_object_inner) = value_object.downcast_ref::<Vector3>() {
                                object_inner.native += value_object_inner.native;
                            } else {
                                return Err(advancedfx::js::errors::error_arguments())
                            }
                        }
                    } else {
                        return Err(advancedfx::js::errors::error_arguments())
                    }
                } else {
                    return Err(advancedfx::js::errors::error_arguments())
                }
            } else {
                return Self::error_typ();
            }
        } else {
            return Self::error_typ();
        }

        Ok(this.clone())
    }

    fn sub(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Vector3>() {
                            match Vector3::from_data(Vector3::new(object_inner.native - value_object_inner.native), context) {
                                Ok(result_object) => {
                                    return Ok(JsValue::Object(result_object));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn sub_assign(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if JsObject::equals(&object,value_object) {
                            let mut native: advancedfx::math::Vector3 = object_inner.native;
                            native -= native;
                        } else {
                            if let Some(value_object_inner) = value_object.downcast_ref::<Vector3>() {
                                object_inner.native -= value_object_inner.native;
                            } else {
                                return Err(advancedfx::js::errors::error_arguments())
                            }
                        }
                    } else {
                        return Err(advancedfx::js::errors::error_arguments())
                    }
                } else {
                    return Err(advancedfx::js::errors::error_arguments())
                }                    
            } else {
                return Self::error_typ();
            }
        } else {
            return Self::error_typ();
        }
        Ok(this.clone())
    }

    fn left_mul(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number() {
                        match Vector3::from_data(Vector3::new(value * object_inner.native), context) {
                            Ok(result_object) => {
                                return Ok(JsValue::Object(result_object));
                            }
                            Err(e) => {
                                return Err(e);
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn left_mul_assign(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Vector3>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number() {
                        object_inner.native.left_mul_assign(value);
                    } else {
                        return Err(advancedfx::js::errors::error_arguments())
                    }
                } else {
                    return Err(advancedfx::js::errors::error_arguments())
                }
        } else {
                return Self::error_typ();
            }
        } else {
            return Self::error_typ();
        }

        Ok(this.clone())
    }
}

impl Class for Vector3 {
    const NAME: &'static str = "AdvancedfxMathVector3";
    const LENGTH: usize = 3;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 3 == args.len() {
            if let Some(value_x) = args[0].as_number()  {
                if let Some(value_y) = args[1].as_number()  {
                    if let Some(value_z) = args[2].as_number()  {
                        return Ok(Vector3::new(advancedfx::math::Vector3::new(value_x,value_y,value_z)));
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .accessor(
                js_string!("x"),
                Some(NativeFunction::from_fn_ptr(Vector3::get_x).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Vector3::set_x).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("y"),
                Some(NativeFunction::from_fn_ptr(Vector3::get_y).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Vector3::set_y).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("z"),
                Some(NativeFunction::from_fn_ptr(Vector3::get_z).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Vector3::set_z).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("length"),
                Some(NativeFunction::from_fn_ptr(Vector3::length).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("normalized"),
                Some(NativeFunction::from_fn_ptr(Vector3::normalized).to_js_function(&realm)),
                None,
                Attribute::all()
            )    
            .method(
                js_string!("add"),
                1,
                NativeFunction::from_fn_ptr(Vector3::add),
            )
            .method(
                js_string!("addAssign"),
                1,
                NativeFunction::from_fn_ptr(Vector3::add_assign),
            )
            .method(
                js_string!("sub"),
                1,
                NativeFunction::from_fn_ptr(Vector3::sub),
            )
            .method(
                js_string!("subAssign"),
                1,
                NativeFunction::from_fn_ptr(Vector3::sub_assign),
            )
            .method(
                js_string!("leftMul"),
                1,
                NativeFunction::from_fn_ptr(Vector3::left_mul),
            )
            .method(
                js_string!("leftMulAssign"),
                1,
                NativeFunction::from_fn_ptr(Vector3::left_mul_assign),
            )
        ;
        Ok(())
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct QEulerAngles {
    #[unsafe_ignore_trace]
    pub native: advancedfx::math::QEulerAngles
}

impl QEulerAngles {
    #[must_use]
    pub fn new(native: advancedfx::math::QEulerAngles) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<QEulerAngles>()
            .expect("the AdvancedfxMathQEulerAngles builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxMathQEulerAngles object")
            .into())
    }    

    fn get_pitch(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QEulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.pitch));
            }
        }
        Self::error_typ()
    }   

    fn set_pitch(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QEulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.pitch = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_yaw(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QEulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.yaw));
            }
        }
        Self::error_typ()
    }   

    fn set_yaw(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QEulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.yaw = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_roll(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QEulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.roll));
            }
        }
        Self::error_typ()
    }   

    fn set_roll(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QEulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.roll = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }
}

impl Class for QEulerAngles {
    const NAME: &'static str = "AdvancedfxMathQEulerAngles";
    const LENGTH: usize = 3;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 3 == args.len() {
            if let Some(value_pitch) = args[0].as_number()  {
                if let Some(value_yaw) = args[1].as_number()  {
                    if let Some(value_roll) = args[2].as_number()  {
                        return Ok(QEulerAngles::new(advancedfx::math::QEulerAngles::new(value_pitch,value_yaw,value_roll)));
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
        .accessor(
            js_string!("pitch"),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::get_pitch).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::set_pitch).to_js_function(&realm)),
            Attribute::all()
        )
        .accessor(
            js_string!("yaw"),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::get_yaw).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::set_yaw).to_js_function(&realm)),
            Attribute::all()
        )
        .accessor(
            js_string!("roll"),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::get_roll).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(QEulerAngles::set_roll).to_js_function(&realm)),
            Attribute::all()
        );

        Ok(())
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct QREulerAngles {
    #[unsafe_ignore_trace]
    pub native: advancedfx::math::QREulerAngles
}

impl QREulerAngles {
    #[must_use]
    pub fn new(native: advancedfx::math::QREulerAngles) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<QREulerAngles>()
            .expect("the AdvancedfxMathQREulerAngles builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxMathQREulerAngles object")
            .into())
    }    

    fn get_pitch(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QREulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.pitch));
            }
        }
        Self::error_typ()
    }   

    fn set_pitch(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QREulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.pitch = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_yaw(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QREulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.yaw));
            }
        }
        Self::error_typ()
    }   

    fn set_yaw(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QREulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.yaw = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_roll(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QREulerAngles>() {
                return Ok(JsValue::Rational(object_inner.native.roll));
            }
        }
        Self::error_typ()
    }   

    fn set_roll(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<QREulerAngles>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.roll = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn to_q_euler_angles(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<QREulerAngles>() {
                match QEulerAngles::from_data(QEulerAngles::new(advancedfx::math::QEulerAngles::from(object_inner.native)), context) {
                    Ok(result_object) => {
                        return Ok(JsValue::Object(result_object));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn from_q_euler_angles(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(object) = args[0].as_object() {
                if let Some(object_inner) = object.downcast_ref::<QEulerAngles>() {
                    match QREulerAngles::from_data(QREulerAngles::new(advancedfx::math::QREulerAngles::from(object_inner.native)), context) {
                        Ok(result_object) => {
                            return Ok(JsValue::Object(result_object));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }

}

impl Class for QREulerAngles {
    const NAME: &'static str = "AdvancedfxMathQREulerAngles";
    const LENGTH: usize = 3;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 3 == args.len() {
            if let Some(value_pitch) = args[0].as_number()  {
                if let Some(value_yaw) = args[1].as_number()  {
                    if let Some(value_roll) = args[2].as_number()  {
                        return Ok(QREulerAngles::new(advancedfx::math::QREulerAngles::new(value_pitch,value_yaw,value_roll)));
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .accessor(
                js_string!("pitch"),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::get_pitch).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::set_pitch).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("yaw"),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::get_yaw).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::set_yaw).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("roll"),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::get_roll).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(QREulerAngles::set_roll).to_js_function(&realm)),
                Attribute::all()
            )
            .method(
                js_string!("toQEulerAngles"),
                1,
                NativeFunction::from_fn_ptr(QREulerAngles::to_q_euler_angles),
            )  
            .static_method(
                js_string!("fromQEulerAngles"),
                1,
                NativeFunction::from_fn_ptr(QREulerAngles::from_q_euler_angles),
            )
        ;

        Ok(())
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct Quaternion {
    #[unsafe_ignore_trace]
    pub native: advancedfx::math::Quaternion,
}

impl Quaternion {
    #[must_use]
    pub fn new(native: advancedfx::math::Quaternion) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Quaternion>()
            .expect("the AdvancedfxMathQuaternion builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxMathQuaternion object")
            .into())
    }
    
    fn get_w(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                return Ok(JsValue::Rational(object_inner.native.w));
            }
        }
        Self::error_typ()
    }   

    fn set_w(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.w = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }    

    fn get_x(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                return Ok(JsValue::Rational(object_inner.native.x));
            }
        }
        Self::error_typ()
    }   

    fn set_x(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.x = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_y(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                return Ok(JsValue::Rational(object_inner.native.y));
            }
        }
        Self::error_typ()
    }   

    fn set_y(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.y = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_z(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                return Ok(JsValue::Rational(object_inner.native.z));
            }
        }
        Self::error_typ()
    }   

    fn set_z(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut object_inner) = object.downcast_mut::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        object_inner.native.z = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn norm(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                return Ok(JsValue::Rational(object_inner.native.norm()));
            }
        }
        Self::error_typ()
    }

    fn normalized(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                match object_inner.native.normalized() {
                    Ok(value) => {
                        match Quaternion::from_data(Quaternion::new(value), context) {
                            Ok(result_object) => {
                                return Ok(JsValue::Object(result_object));
                            }
                            Err(e) => {
                                return Err(e);
                            }
                        }
                    }
                    Err(e) => {
                        return Err(JsNativeError::error().with_message(e.to_string()).into());
                    }
                }

            }
        }
        Self::error_typ()
    }

    fn dot(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                            return Ok(JsValue::Rational(object_inner.native.dot(value_object_inner.native)));
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn conjugate(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                match Quaternion::from_data(Quaternion::new(object_inner.native.conjugate()), context) {
                    Ok(result_object) => {
                        return Ok(JsValue::Object(result_object));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn get_ang(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                            let (axis,ang) = object_inner.native.get_ang(value_object_inner.native);
                            match Vector3::from_data(Vector3::new(axis), context) {
                                Ok(result_object) => {
                                    let array = JsArray::new(context);
                                    array.push(JsValue::Object(result_object), context)?;
                                    array.push(JsValue::Rational(ang), context)?;
                                    return Ok(JsValue::Object(array.into()));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn slerp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 2 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                            if let Some(t) = args[1].as_number() {
                                match Quaternion::from_data(Quaternion::new(object_inner.native.slerp(value_object_inner.native,t)), context) {
                                    Ok(result_object) => {
                                        return Ok(JsValue::Object(result_object));
                                    }
                                    Err(e) => {
                                        return Err(e);
                                    }
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn add(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                            match Quaternion::from_data(Quaternion::new(object_inner.native + value_object_inner.native), context) {
                                Ok(result_object) => {
                                    return Ok(JsValue::Object(result_object));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn sub(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value_object) = args[0].as_object() {
                        if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                            match Quaternion::from_data(Quaternion::new(object_inner.native - value_object_inner.native), context) {
                                Ok(result_object) => {
                                    return Ok(JsValue::Object(result_object));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }
    
    fn mul(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if args[0].is_object() {
                        if let Some(value_object) = args[0].as_object() {
                            if let Some(value_object_inner) = value_object.downcast_ref::<Quaternion>() {
                                match Quaternion::from_data(Quaternion::new(object_inner.native * value_object_inner.native), context) {
                                    Ok(result_object) => {
                                        return Ok(JsValue::Object(result_object));
                                    }
                                    Err(e) => {
                                        return Err(e);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn left_mul(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number() {
                        match Quaternion::from_data(Quaternion::new(value * object_inner.native), context) {
                            Ok(result_object) => {
                                return Ok(JsValue::Object(result_object));
                            }
                            Err(e) => {
                                return Err(e);
                            }
                        }                    
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }    
    
    fn to_q_r_euler_angles(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(object_inner) = object.downcast_ref::<Quaternion>() {
                match QREulerAngles::from_data(QREulerAngles::new(advancedfx::math::QREulerAngles::from(object_inner.native)), context) {
                    Ok(result_object) => {
                        return Ok(JsValue::Object(result_object));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn from_q_r_euler_angles(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(object) = args[0].as_object() {
                if let Some(object_inner) = object.downcast_ref::<QREulerAngles>() {
                    match Quaternion::from_data(Quaternion::new(advancedfx::math::Quaternion::from(object_inner.native)), context) {
                        Ok(result_object) => {
                            return Ok(JsValue::Object(result_object));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }
    
}

impl Class for Quaternion {
    const NAME: &'static str = "AdvancedfxMathQuaternion";
    const LENGTH: usize = 3;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 4 == args.len() {
            if let Some(value_w) = args[0].as_number()  {
                if let Some(value_x) = args[1].as_number()  {
                    if let Some(value_y) = args[2].as_number()  {
                        if let Some(value_z) = args[3].as_number()  {
                            return Ok(Quaternion::new(advancedfx::math::Quaternion::new(value_w,value_x,value_y,value_z)));
                        }
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .accessor(
                js_string!("w"),
                Some(NativeFunction::from_fn_ptr(Quaternion::get_w).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Quaternion::set_w).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("x"),
                Some(NativeFunction::from_fn_ptr(Quaternion::get_x).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Quaternion::set_x).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("y"),
                Some(NativeFunction::from_fn_ptr(Quaternion::get_y).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Quaternion::set_y).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("z"),
                Some(NativeFunction::from_fn_ptr(Quaternion::get_z).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Quaternion::set_z).to_js_function(&realm)),
                Attribute::all()
            )
            .method(
                js_string!("dot"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::dot),
            )
            .accessor(
                js_string!("norm"),
                Some(NativeFunction::from_fn_ptr(Quaternion::norm).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("normalized"),
                Some(NativeFunction::from_fn_ptr(Quaternion::normalized).to_js_function(&realm)),
                None,
                Attribute::all()
            )    
            .accessor(
                js_string!("conjugate"),
                Some(NativeFunction::from_fn_ptr(Quaternion::conjugate).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .method(
                js_string!("getAng"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::get_ang),
            )
            .method(
                js_string!("slerp"),
                2,
                NativeFunction::from_fn_ptr(Quaternion::slerp),
            )
            .method(
                js_string!("add"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::add),
            )                          
            .method(
                js_string!("sub"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::sub),
            )                          
            .method(
                js_string!("mul"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::mul),
            )
            .method(
                js_string!("leftMul"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::left_mul),
            )
            .method(
                js_string!("toQREulerAngles"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::to_q_r_euler_angles),
            )  
            .static_method(
                js_string!("fromQREulerAngles"),
                1,
                NativeFunction::from_fn_ptr(Quaternion::from_q_r_euler_angles),
            )             
        ;
        Ok(())
    }
}
