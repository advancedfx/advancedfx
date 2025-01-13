use crate::advancedfx;
use crate::advancedfx::campath::CampathChangedObservable;

use std::cell::RefCell;
use std::rc::Rc;

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
    JsObject,
    JsResult,
    JsValue,
    NativeFunction,
    Trace
};

#[derive(Trace, Finalize, JsData)]
pub struct Value {
    #[unsafe_ignore_trace]
    pub native: advancedfx::campath::Value
}

impl Value {
    #[must_use]
    pub fn new(native: advancedfx::campath::Value) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Value>()
            .expect("the AdvancedfxCampathValue builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxCampathValue object")
            .into())
    }

    fn get_pos(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                match advancedfx::js::math::Vector3::from_data(advancedfx::js::math::Vector3::new(campath_value.native.p.clone()), context) {
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

    fn set_pos(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(object_other) = args[0].as_object()  {
                        if JsObject::equals(&object, object_other) {
                            return Ok(JsValue::Undefined); 
                        }
                        if let Some(vector3_other) = object_other.downcast_ref::<advancedfx::js::math::Vector3>() {
                            campath_value.native.p = vector3_other.native.clone();
                            return Ok(JsValue::Undefined); 
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_rot(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                match advancedfx::js::math::Quaternion::from_data(advancedfx::js::math::Quaternion::new(campath_value.native.r.clone()),context) {
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

    fn set_rot(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(object_other) = args[0].as_object()  {
                        if JsObject::equals(&object, object_other) {
                            return Ok(JsValue::Undefined); 
                        }                        
                        if let Some(quaternion_other) = object_other.downcast_ref::<advancedfx::js::math::Quaternion>() {
                            campath_value.native.r = quaternion_other.native.clone();
                            return Ok(JsValue::Undefined); 
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_fov(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                return Ok(JsValue::Rational(campath_value.native.fov));
            }
        }
        Self::error_typ()
    }   

    fn set_fov(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        campath_value.native.fov = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_selected(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                return Ok(JsValue::Boolean(campath_value.native.selected));
            }
        }
        Self::error_typ()
    }   

    fn set_selected(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_boolean()  {
                        campath_value.native.selected = value;
                        return Ok(JsValue::Undefined);  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }
}

impl Class for Value {
    const NAME: &'static str = "AdvancedfxCampathValue";
    const LENGTH: usize = 4;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 4 == args.len() {
            if let Some(object_pos) = args[0].as_object()  {
                if let Some(value_pos) = object_pos.downcast_ref::<advancedfx::js::math::Vector3>() {
                    if let Some(object_rot) = args[1].as_object()  {
                        if let Some(value_rot) = object_rot.downcast_ref::<advancedfx::js::math::Quaternion>() {
                            if let Some(value_fov) = args[2].as_number()  {
                                if let Some(value_selected) = args[3].as_boolean()  {
                                    return Ok(advancedfx::js::campath::Value::new(advancedfx::campath::Value::new(value_pos.native.clone(),value_rot.native.clone(),value_fov,value_selected)));
                                }
                            }
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
            js_string!("pos"),
            Some(NativeFunction::from_fn_ptr(Value::get_pos).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(Value::set_pos).to_js_function(&realm)),
            Attribute::all()
        )
        .accessor(
            js_string!("rot"),
            Some(NativeFunction::from_fn_ptr(Value::get_rot).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(Value::set_rot).to_js_function(&realm)),
            Attribute::all()
        )
        .accessor(
            js_string!("fov"),
            Some(NativeFunction::from_fn_ptr(Value::get_fov).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(Value::set_fov).to_js_function(&realm)),
            Attribute::all()
        )
        .accessor(
            js_string!("selected"),
            Some(NativeFunction::from_fn_ptr(Value::get_selected).to_js_function(&realm)),
            Some(NativeFunction::from_fn_ptr(Value::set_selected).to_js_function(&realm)),
            Attribute::all()
        );

        Ok(())
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct Iterator {
    #[unsafe_ignore_trace]
    pub native: std::rc::Rc<std::cell::RefCell<advancedfx::campath::Iterator>>
}

impl Iterator {
    #[must_use]
    pub fn new(native: std::rc::Rc<std::cell::RefCell<advancedfx::campath::Iterator>>) -> Self {        
        Self {
            native: native
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Iterator>()
            .expect("the AdvancedfxCampathIterator builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxCampathIterator object")
            .into())
    }

    fn error_not_valid() -> JsResult<JsValue> {
        Err(JsNativeError::error().with_message("'this' AdvancedfxCampathIterator is not in valid state!").into())
    }    
    
    fn is_valid(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                return Ok(JsValue::Boolean((*campath_iterator.native).borrow().is_valid()));
            }
        }
        Self::error_typ()
    }

    fn get_time(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(value) = (*campath_iterator.native).borrow().get_time()  {
                    return Ok(JsValue::Rational(value));
                }
                return Self::error_not_valid();
            }
        }
        Self::error_typ()
    }     
    
    fn get_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(value) = (*campath_iterator.native).borrow().get_value()  {
                    match Value::from_data(Value::new(value), context) {
                        Ok(result_object) => {
                            return Ok(JsValue::Object(result_object));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
                return Self::error_not_valid();
            }
        }
        Self::error_typ()
    }
    
    fn next(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(()) = (*campath_iterator.native).borrow_mut().next()  {
                    return Ok(JsValue::undefined());
                }
                return Self::error_not_valid();
            }
        }
        Self::error_typ()
    }    
}

impl Class for Iterator {
    const NAME: &'static str = "AdvancedfxCampathIterator";
    const LENGTH: usize = 1;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        if 1 == args.len() {
            if let Some(object_campath) = args[0].as_object()  {
                if let Some(mut value_campath) = object_campath.downcast_mut::<Campath>() {       
                    return Ok(Iterator::new(value_campath.native.iterator()));
                }
            }     
        }
        Err(advancedfx::js::errors::error_arguments())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
        .accessor(
            js_string!("valid"),
            Some(NativeFunction::from_fn_ptr(Iterator::is_valid).to_js_function(&realm)),
            None,
            Attribute::all()
        )
        .accessor(
            js_string!("time"),
            Some(NativeFunction::from_fn_ptr(Iterator::get_time).to_js_function(&realm)),
            None,
            Attribute::all()
        )
        .accessor(
            js_string!("value"),
            Some(NativeFunction::from_fn_ptr(Iterator::get_value).to_js_function(&realm)),
            None,
            Attribute::all()
        )
        .method(
            js_string!("next"),
            0,
            NativeFunction::from_fn_ptr(Iterator::next)
        );                     

        Ok(())
    }
}

#[derive(Trace, Finalize, JsData)]
struct CampathChangedCallback {
    #[unsafe_ignore_trace]
    callback: JsObject,
    weak_context_wrapper: advancedfx::js::WeakContextWrapper,
    suppress: u32,
    suppressed_change: bool
}

impl CampathChangedCallback {
    #[must_use]
    fn new(callback: JsObject, weak_context_wrapper: advancedfx::js::WeakContextWrapper) -> Self {
        Self {
            callback: callback,
            weak_context_wrapper: weak_context_wrapper,
            suppress: 0,
            suppressed_change: false,
        }
    }

    fn suppress(&mut self) {
        self.suppress += 1;
    }

    fn unsuppress(&mut self) -> Option<JsObject> {
        self.suppress -= 1;
        if self.suppress == 0 && self.suppressed_change {
            self.suppressed_change = false;
            return Some(self.callback.clone());
        }
        return None;
    }

}

struct CampathChangedSuppressor {
    callback: Option<Rc<RefCell<CampathChangedCallback>>>
}

impl CampathChangedSuppressor {
    #[must_use]
    fn new() -> Self {
        Self {
            callback: None
        }
    }

    fn suppress(&mut self, campath: &Campath) {
        if let Some(some_on_changed) = &campath.on_changed {
            (*some_on_changed).borrow_mut().suppress();
            self.callback = Some(some_on_changed.clone());
        }
    }

    fn finish(&mut self, result: JsResult<JsValue>, context: &mut Context) -> JsResult<JsValue> {
        let mut object_option: Option<JsObject> = None;
        if let Some(some_on_changed) = &self.callback {
            object_option = (*some_on_changed).borrow_mut().unsuppress();
        }
        if !result.is_err() {
            if let Some(object) = object_option {
                if let Err(e) = object.call(&JsValue::null(), &[], context) {
                    return Err(e);
                }
            }
        }
        return result;
    }
}

impl Drop for CampathChangedSuppressor {
    fn drop(&mut self) {
    }
}

impl advancedfx::campath::CampathChangedObserver for CampathChangedCallback {
    fn notify(&mut self) {
        if 0 == self.suppress {
            if let Some(context_wrapper_rc) = self.weak_context_wrapper.context_wrapper.upgrade() {
                let _ = self.callback.call(&JsValue::null(), &[], &mut context_wrapper_rc.borrow_mut().context);
            }
        }
        else {
            self.suppressed_change = true;
        }
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct Campath {
    #[unsafe_ignore_trace]
    pub native: advancedfx::campath::Campath,

    #[unsafe_ignore_trace]
    on_changed: Option<Rc<RefCell<CampathChangedCallback>>>
}

impl Campath {
    #[must_use]
    pub fn new(native: advancedfx::campath::Campath) -> Self {        
        Self {
            native: native,
            on_changed : None
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Campath>()
            .expect("the AdvancedfxCampath builtin shouldn't exist");        
    }

    fn error_typ() -> JsResult<JsValue> {
        Err(JsNativeError::typ()
            .with_message("'this' is not a AdvancedfxCampath object")
            .into())
    }    

    fn get_enabled(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Boolean(campath.native.get_enabled()));
            }
        }
        Self::error_typ()
    }
    
    fn set_enabled(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_enabled_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn set_enabled_internal (suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_boolean() {
                        suppressor.suppress(&campath);
                        campath.native.set_enabled(value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }


    fn get_offset(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Rational(campath.native.get_offset()));
            }
        }
        Self::error_typ()
    }

    fn set_offset(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_offset_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn set_offset_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number() {
                        suppressor.suppress(&campath);
                        campath.native.set_offset(value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_hold(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Boolean(campath.native.get_hold()));
            }
        }
        Self::error_typ()
    }

    fn set_hold(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_hold_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn set_hold_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_boolean() {
                        suppressor.suppress(&campath);
                        campath.native.set_hold(value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }            
        }
        Self::error_typ()
    }

    fn get_position_interp(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Integer((campath.native.get_position_interp() as u8).into()));
            }
        }
        Self::error_typ()
    }

    fn set_position_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_position_interp_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn set_position_interp_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Ok(value) = args[0].to_uint8(context) {
                        let Ok(e_value) = advancedfx::campath::DoubleInterp::try_from(value);
                        suppressor.suppress(&campath);
                        campath.native.set_position_interp(e_value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_rotation_interp(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Integer((campath.native.get_rotation_interp() as u8).into()));
            }
        }
        Self::error_typ()
    }

    fn set_rotation_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_rotation_interp_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }
    
    fn set_rotation_interp_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Ok(value) = args[0].to_uint8(context) {
                        let Ok(e_value) = advancedfx::campath::QuaternionInterp::try_from(value);
                        suppressor.suppress(&campath);
                        campath.native.set_rotation_interp(e_value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn get_fov_interp(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::Integer((campath.native.get_fov_interp() as u8).into()));
            }
        }
        Self::error_typ()
    }

    fn set_fov_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_fov_interp_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn set_fov_interp_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Ok(value) = args[0].to_uint8(context) {
                        let Ok(e_value) = advancedfx::campath::DoubleInterp::try_from(value);
                        suppressor.suppress(&campath);
                        campath.native.set_fov_interp(e_value);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn add(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::add_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }


    fn add_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 2 == args.len() {
                    if let Some(time) = args[0].as_number() {
                        if let Some(value_object) = args[1].as_object() {
                            if let Some(value_object_inner) = value_object.downcast_ref::<Value>() {
                                suppressor.suppress(&campath);
                                campath.native.add(time,&value_object_inner.native);                          
                                return Ok(JsValue::undefined());
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn remove(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::remove_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }
    fn remove_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(time) = args[0].as_number() {
                        suppressor.suppress(&campath);
                        campath.native.remove(time);      
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }

    fn clear(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::clear_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }    
    
    fn clear_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                suppressor.suppress(&campath);
                campath.native.clear();
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ()
    }

    fn get_size(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::from(campath.native.get_size()));
            }
        }
        Self::error_typ()
    }

    fn get_duration(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::from(campath.native.get_duration()));
            }
        }
        Self::error_typ()
    }

    fn get_lower_bound(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                if 1 <= campath.native.get_size() {
                    return Ok(JsValue::from(campath.native.get_lower_bound()));
                }
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ()
    }

    fn get_upper_bound(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                if 1 <= campath.native.get_size() {
                    return Ok(JsValue::from(campath.native.get_upper_bound()));
                }
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ()
    }

    fn get_can_eval(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                return Ok(JsValue::from(campath.native.can_eval()));
            }
        }
        Self::error_typ()
    }

    fn eval(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                if 1 == args.len() {
                    if let Some(time) = args[0].as_number() {
                        if campath.native.can_eval() {
                            match Value::from_data(Value::new(campath.native.eval(time)), context) {
                                Ok(result_object) => {
                                    return Ok(JsValue::Object(result_object));
                                }
                                Err(e) => {
                                    return Err(e);
                                }
                            }
                        }
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn load(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::load_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn load_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(js_file_path) = args[0].as_string() {
                        if let Ok(str_file_path) = js_file_path.to_std_string() {
                            suppressor.suppress(&campath);
                            let result = campath.native.load(str_file_path);
                            return Ok(JsValue::from(result));
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn save(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(js_file_path) = args[0].as_string() {
                        if let Ok(str_file_path) = js_file_path.to_std_string() {
                            return Ok(JsValue::from(campath.native.save(str_file_path)));
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn set_start(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_start_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn set_start_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 <= args.len() {
                    if let Some(time) = args[0].as_number() {
                        if 1 == args.len() {
                            suppressor.suppress(&campath);
                            campath.native.set_start(time,false);
                            return Ok(JsValue::undefined());
                        }
                        else if 2 == args.len() {
                            if let Some(relative) = args[1].as_boolean() {
                                suppressor.suppress(&campath);
                                campath.native.set_start(time,relative);
                                return Ok(JsValue::undefined());
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }  

    fn set_duration(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_duration_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn set_duration_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(time) = args[0].as_number() {
                        suppressor.suppress(&campath);
                        campath.native.set_duration(time);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn set_position(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_position_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn set_position_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 3 == args.len() {
                    let result_x: Result<Option<f64>,()> = if args[0].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[0].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    let result_y: Result<Option<f64>,()> = if args[1].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[1].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    let result_z: Result<Option<f64>,()> = if args[2].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[2].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    if let Ok(x) = result_x {
                        if let Ok(y) = result_y {
                            if let Ok(z) = result_z {
                                suppressor.suppress(&campath);
                                campath.native.set_position(x,y,z);
                                return Ok(JsValue::undefined());                            
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn set_angles(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_angles_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn set_angles_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 3 == args.len() {
                    let result_x: Result<Option<f64>,()> = if args[0].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[0].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    let result_y: Result<Option<f64>,()> = if args[1].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[1].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    let result_z: Result<Option<f64>,()> = if args[2].is_undefined() {
                        Ok(None)
                    } else if let Some(value) = args[2].as_number() {
                        Ok(Some(value))
                    } else {
                        Err(())
                    };
                    if let Ok(x) = result_x {
                        if let Ok(y) = result_y {
                            if let Ok(z) = result_z {
                                suppressor.suppress(&campath);
                                campath.native.set_angles(x,y,z);
                                return Ok(JsValue::undefined());                            
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn set_fov(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::set_fov_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn set_fov_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    if let Some(fov) = args[0].as_number() {
                        suppressor.suppress(&campath);
                        campath.native.set_fov(fov);
                        return Ok(JsValue::undefined());
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn rotate(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::rotate_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn rotate_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 3 == args.len() {
                    if let Some(y_pitch) = args[0].as_number() {
                        if let Some(z_yaw) = args[1].as_number() {
                            if let Some(x_roll) = args[2].as_number() {
                                suppressor.suppress(&campath);
                                campath.native.rotate(y_pitch,z_yaw,x_roll);
                                return Ok(JsValue::undefined());
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn anchor_transform(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::anchor_transform_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn anchor_transform_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 12 == args.len() {
                    if let Some(anchor_x) = args[0].as_number() {
                        if let Some(anchor_y) = args[1].as_number() {
                            if let Some(anchor_z) = args[2].as_number() {
                                if let Some(anchor_y_pitch) = args[3].as_number() {
                                    if let Some(anchor_z_yaw) = args[4].as_number() {
                                        if let Some(anchor_x_roll) = args[5].as_number() {
                                            if let Some(dest_x) = args[6].as_number() {
                                                if let Some(dest_y) = args[7].as_number() {
                                                    if let Some(dest_z) = args[8].as_number() {
                                                        if let Some(dest_y_pitch) = args[9].as_number() {
                                                            if let Some(dest_z_yaw) = args[10].as_number() {
                                                                if let Some(dest_x_roll) = args[11].as_number() {
                                                                    suppressor.suppress(&campath);
                                                                    campath.native.anchor_transform(anchor_x, anchor_y, anchor_z, anchor_y_pitch, anchor_z_yaw, anchor_x_roll, dest_x, dest_y, dest_z, dest_y_pitch, dest_z_yaw, dest_x_roll);
                                                                    return Ok(JsValue::undefined());
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments());
            }
        }
        Self::error_typ()
    }

    fn select_all(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_all_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn select_all_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                suppressor.suppress(&campath);
                let result = campath.native.select_all();
                return Ok(JsValue::from(result));
            }
        }
        Self::error_typ()
    }

    fn select_none(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_none_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn select_none_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                suppressor.suppress(&campath);
                campath.native.select_none();
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ()
    }

    fn select_invert(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_invert_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn select_invert_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                suppressor.suppress(&campath);
                let result = campath.native.select_invert();
                return Ok(JsValue::from(result));
            }
        }
        Self::error_typ()
    }

    fn select_add_idx(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_add_idx_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)        
    }

    fn select_add_idx_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 2 == args.len() {
                    if let Some(min) = args[0].as_number() {
                        if let Some(max) = args[1].as_number() {
                            suppressor.suppress(&campath);
                            let result = campath.native.select_add_idx(min as usize,max as usize);
                            return Ok(JsValue::from(result));
                        }
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn select_add_min_count(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_add_min_count_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }
    fn select_add_min_count_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 2 == args.len() {
                    if let Some(min) = args[0].as_number() {
                        if let Some(count) = args[1].as_number() {
                            suppressor.suppress(&campath);
                            let result = campath.native.select_add_min_count(min,count as usize);
                            return Ok(JsValue::from(result));
                        }
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn select_add_min_max(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let mut suppressor = CampathChangedSuppressor::new();
        let result = Self::select_add_min_max_internal(&mut suppressor, this,args,context);
        suppressor.finish(result, context)
    }

    fn select_add_min_max_internal(suppressor: &mut CampathChangedSuppressor, this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 2 == args.len() {
                    if let Some(min) = args[0].as_number() {
                        if let Some(max) = args[1].as_number() {
                            suppressor.suppress(&campath);
                            let result = campath.native.select_add_min_max(min,max);
                            return Ok(JsValue::from(result));
                        }
                    }
                }
            }
        }
        Self::error_typ()
    }

    fn get_on_changed(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath) = object.downcast_ref::<Campath>() {
                if let Some(some_on_changed) = &campath.on_changed {
                    return Ok(JsValue::Object((*some_on_changed).borrow_mut().callback.clone()));
                }
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ()
    }

    fn set_on_changed(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath) = object.downcast_mut::<Campath>() {
                if 1 == args.len() {
                    match &args[0] {
                        JsValue::Undefined => {
                            if let Some(some_on_changed) = &campath.on_changed {
                                let observer =  some_on_changed.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                                campath.native.unregister(Rc::downgrade(&observer));
                            }
                            campath.on_changed = None;
                            return Ok(JsValue::Undefined); 
                        }
                        JsValue::Object(object) => {
                            if object.is_callable() {
                                if let Some(weak_context_wrapper) = context.get_data::<advancedfx::js::WeakContextWrapper>() {
                                    if let Some(some_on_changed) = &campath.on_changed {
                                        let observer =  some_on_changed.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                                        campath.native.unregister(Rc::downgrade(&observer));
                                    }
                                    let some_on_changed = Rc::<RefCell<CampathChangedCallback>>::new(RefCell::<CampathChangedCallback>::new(CampathChangedCallback::new(
                                        object.clone(),
                                        weak_context_wrapper.clone()
                                    )));
                                    let observer =  some_on_changed.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                                    campath.native.register(Rc::downgrade(&observer));                                    
                                    campath.on_changed = Some(some_on_changed);
                                    return Ok(JsValue::Undefined); 
                                }
                                return Err(advancedfx::js::errors::error_no_wrapper());
                            }
                        }
                        _ => {
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments())
            }
        }
        Self::error_typ()
    }
}

impl Class for Campath {
    const NAME: &'static str = "AdvancedfxCampath";
    const LENGTH: usize = 0;

    fn data_constructor(
        _this: &JsValue,
        _args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        return Ok(Campath::new(advancedfx::campath::Campath::new()));
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        let realm = class.context().realm().clone();
        class
            .accessor(
                js_string!("enabled"),
                Some(NativeFunction::from_fn_ptr(Campath::get_enabled).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_enabled).to_js_function(&realm)),
                Attribute::all()
            )        
            .accessor(
                js_string!("offset"),
                Some(NativeFunction::from_fn_ptr(Campath::get_offset).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_offset).to_js_function(&realm)),
                Attribute::all()
            )        
            .accessor(
                js_string!("hold"),
                Some(NativeFunction::from_fn_ptr(Campath::get_hold).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_hold).to_js_function(&realm)),
                Attribute::all()
            )        
            .accessor(
                js_string!("positionInterp"),
                Some(NativeFunction::from_fn_ptr(Campath::get_position_interp).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_position_interp).to_js_function(&realm)),
                Attribute::all()
            )
            .accessor(
                js_string!("rotationInterp"),
                Some(NativeFunction::from_fn_ptr(Campath::get_rotation_interp).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_rotation_interp).to_js_function(&realm)),
                Attribute::all()
            )        
            .accessor(
                js_string!("fovInterp"),
                Some(NativeFunction::from_fn_ptr(Campath::get_fov_interp).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_fov_interp).to_js_function(&realm)),
                Attribute::all()
            )
            .method(
                js_string!("add"),
                2,
                NativeFunction::from_fn_ptr(Campath::add)
            )
            .method(
                js_string!("remove"),
                1,
                NativeFunction::from_fn_ptr(Campath::remove)
            )
            .method(
                js_string!("clear"),
                0,
                NativeFunction::from_fn_ptr(Campath::clear)
            )
            .accessor(
                js_string!("size"),
                Some(NativeFunction::from_fn_ptr(Campath::get_size).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("duration"),
                Some(NativeFunction::from_fn_ptr(Campath::get_duration).to_js_function(&realm)),
                None,
                Attribute::all()
            )        
            .accessor(
                js_string!("lowerBound"),
                Some(NativeFunction::from_fn_ptr(Campath::get_lower_bound).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("upperBound"),
                Some(NativeFunction::from_fn_ptr(Campath::get_upper_bound).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .accessor(
                js_string!("canEval"),
                Some(NativeFunction::from_fn_ptr(Campath::get_can_eval).to_js_function(&realm)),
                None,
                Attribute::all()
            )
            .method(
                js_string!("eval"),
                1,
                NativeFunction::from_fn_ptr(Campath::eval)
            )
            .method(
                js_string!("load"),
                1,
                NativeFunction::from_fn_ptr(Campath::load)
            )         
            .method(
                js_string!("save"),
                1,
                NativeFunction::from_fn_ptr(Campath::save)
            )
            .method(
                js_string!("setStart"),
                2,
                NativeFunction::from_fn_ptr(Campath::set_start)
            )
            .method(
                js_string!("setDuration"),
                1,
                NativeFunction::from_fn_ptr(Campath::set_duration)
            )
            .method(
                js_string!("setPosition"),
                3,
                NativeFunction::from_fn_ptr(Campath::set_position)
            ) 
            .method(
                js_string!("setAngels"),
                3,
                NativeFunction::from_fn_ptr(Campath::set_angles)
            )
            .method(
                js_string!("setFov"),
                1,
                NativeFunction::from_fn_ptr(Campath::set_fov)
            )
            .method(
                js_string!("rotate"),
                3,
                NativeFunction::from_fn_ptr(Campath::rotate)
            )
            .method(
                js_string!("anchorTransform"),
                12,
                NativeFunction::from_fn_ptr(Campath::anchor_transform)
            )            
            .method(
                js_string!("selectAll"),
                0,
                NativeFunction::from_fn_ptr(Campath::select_all)
            )
            .method(
                js_string!("selectNone"),
                0,
                NativeFunction::from_fn_ptr(Campath::select_none)
            )            
            .method(
                js_string!("selectInvert"),
                0,
                NativeFunction::from_fn_ptr(Campath::select_invert)
            )       
            .method(
                js_string!("selectAddIdx"),
                2,
                NativeFunction::from_fn_ptr(Campath::select_add_idx)
            )       
            .method(
                js_string!("selectAddMinCount"),
                2,
                NativeFunction::from_fn_ptr(Campath::select_add_min_count)
            )
            .method(
                js_string!("selectAddMinMax"),
                2,
                NativeFunction::from_fn_ptr(Campath::select_add_min_max)
            )
            .accessor(
                js_string!("onChanged"),
                Some(NativeFunction::from_fn_ptr(Campath::get_on_changed).to_js_function(&realm)),
                Some(NativeFunction::from_fn_ptr(Campath::set_on_changed).to_js_function(&realm)),
                Attribute::all()
            );                  
            Ok(())
    }
}
