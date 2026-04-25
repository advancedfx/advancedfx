use crate::advancedfx;
use crate::advancedfx::campath::CampathChangedObservable;
use crate::advancedfx::js::events::EventSource;

use boa_engine::Context;
use boa_engine::Finalize;
use boa_engine::NativeFunction;
use boa_engine::JsData;
use boa_engine::JsNativeError;
use boa_engine::JsObject;
use boa_engine::JsResult;
use boa_engine::JsValue;
use boa_engine::JsVariant;
use boa_engine::Trace;
use boa_engine::class::Class;
use boa_engine::class::ClassBuilder;
use boa_engine::js_string;
use boa_engine::js_value;
use boa_engine::object::ObjectInitializer;
use boa_engine::object::builtins::JsFunction;
use boa_engine::property::Attribute;
use boa_engine::property::PropertyDescriptor;
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;
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

    fn error_typ(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(),"'this' is not a AdvancedfxCampathValue object",context).into())
    }

    fn get_pos(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                match advancedfx::js::math::Vector3::from_data(advancedfx::js::math::Vector3::new(campath_value.native.p.clone()), context) {
                    Ok(result_object) => {
                        return Ok(js_value!(result_object));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
        Self::error_typ(context)
    }   

    fn set_pos(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(object_other) = args[0].as_object()  {
                        if JsObject::equals(&object, &object_other) {
                            return Ok(JsValue::undefined()); 
                        }
                        if let Some(vector3_other) = object_other.downcast_ref::<advancedfx::js::math::Vector3>() {
                            campath_value.native.p = vector3_other.native.clone();
                            return Ok(JsValue::undefined()); 
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into())
            }
        }
        Self::error_typ(context)
    }

    fn get_rot(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                match advancedfx::js::math::Quaternion::from_data(advancedfx::js::math::Quaternion::new(campath_value.native.r.clone()),context) {
                    Ok(result_object) => {
                        return Ok(js_value!(result_object));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
        Self::error_typ(context)
    }   

    fn set_rot(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(object_other) = args[0].as_object()  {
                        if JsObject::equals(&object, &object_other) {
                            return Ok(JsValue::undefined()); 
                        }                        
                        if let Some(quaternion_other) = object_other.downcast_ref::<advancedfx::js::math::Quaternion>() {
                            campath_value.native.r = quaternion_other.native.clone();
                            return Ok(JsValue::undefined()); 
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into())
            }
        }
        Self::error_typ(context)
    }

    fn get_fov(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                return Ok(js_value!(campath_value.native.fov));
            }
        }
        Self::error_typ(context)
    }   

    fn set_fov(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_number()  {
                        campath_value.native.fov = value;
                        return Ok(JsValue::undefined());  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into())
            }
        }
        Self::error_typ(context)
    }

    fn get_selected(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_value) = object.downcast_ref::<Value>() {
                return Ok(js_value!(campath_value.native.selected));
            }
        }
        Self::error_typ(context)
    }   

    fn set_selected(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut campath_value) = object.downcast_mut::<Value>() {
                if 1 == args.len() {
                    if let Some(value) = args[0].as_boolean()  {
                        campath_value.native.selected = value;
                        return Ok(JsValue::undefined());  
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into())
            }
        }
        Self::error_typ(context)
    }
}

impl Class for Value {
    const NAME: &'static str = "AdvancedfxCampathValue";
    const LENGTH: usize = 4;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        context: &mut Context,
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
        Err(advancedfx::js::errors::error_arguments(context).into())
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

    fn error_typ(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(),"'this' is not a AdvancedfxCampathIterator object",context).into())
    }

    fn error_not_valid(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::error(),"'this' AdvancedfxCampathIterator is not in valid state!",context).into())
    }    
    
    fn is_valid(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                return Ok(js_value!((*campath_iterator.native).borrow().is_valid()));
            }
        }
        Self::error_typ(context)
    }

    fn get_time(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(value) = (*campath_iterator.native).borrow().get_time()  {
                    return Ok(js_value!(value));
                }
                return Self::error_not_valid(context);
            }
        }
        Self::error_typ(context)
    }     
    
    fn get_value(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(value) = (*campath_iterator.native).borrow().get_value()  {
                    match Value::from_data(Value::new(value), context) {
                        Ok(result_object) => {
                            return Ok(js_value!(result_object));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
                return Self::error_not_valid(context);
            }
        }
        Self::error_typ(context)
    }
    
    fn next(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(campath_iterator) = object.downcast_ref::<Iterator>() {
                if let Some(()) = (*campath_iterator.native).borrow_mut().next()  {
                    return Ok(JsValue::undefined());
                }
                return Self::error_not_valid(context);
            }
        }
        Self::error_typ(context)
    }    
}

impl Class for Iterator {
    const NAME: &'static str = "AdvancedfxCampathIterator";
    const LENGTH: usize = 1;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        if 1 == args.len() {
            return Ok(Self::new(Campath::inner_from_this(&args[0],context)?.borrow_mut().data_mut().native.iterator()));
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
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
    event_source: JsObject<EventSource>,

    #[unsafe_ignore_trace]
    context: Rc<advancedfx::js::BoxedContext>
}

impl CampathChangedCallback {
    #[must_use]
    fn new(event_source: JsObject<EventSource>, context: Rc<advancedfx::js::BoxedContext>) -> Self {
        Self {
            event_source: event_source,
            context: context
        }
    }
}

impl advancedfx::campath::CampathChangedObserver for CampathChangedCallback {
    fn notify(&mut self) {
        let context = (*self.context).get();

        let _ = EventSource::obj_dispatch(
            &self.event_source,
            context,
            JsValue::undefined(),
            HashMap::from([])
        );
    }
}

#[derive(Trace, Finalize, JsData)]
struct CampathInner {
    #[unsafe_ignore_trace]
    native: advancedfx::campath::Campath,  

    #[unsafe_ignore_trace]
    callback: Option<Rc<RefCell<CampathChangedCallback>>>,

    #[unsafe_ignore_trace]
    event_source: JsObject<EventSource>,   
}

const CAMPATH_ON_CHANGED_STR: &'static str = "AdvancedfxCampath.onChanged";

#[derive(Trace, Finalize, JsData)]
pub struct Campath {
    inner: JsObject<CampathInner>,

    #[unsafe_ignore_trace]
    on_changed: RefCell<Option<JsFunction>>,
}

impl Campath {
    #[must_use]
    pub fn new(native: advancedfx::campath::Campath, context: &mut Context) -> Self {
        let inner = JsObject::from_proto_and_data(None, CampathInner{
            native: native,
            callback: None,
            event_source: EventSource::obj_new(EventSource::new(), context)
        });

        Self {
            inner: inner.downcast::<CampathInner>().unwrap(),
            on_changed: RefCell::<Option<JsFunction>>::new(None)
        }
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<Campath>()
            .expect("the AdvancedfxCampath builtin shouldn't exist");        
    }

    fn downcast_from_this(this: &JsValue, context: &mut Context) -> JsResult<JsObject<Campath>> {
        if let Some(object) = this.as_object() {
            if let Ok(campath) = object.downcast::<Campath>() {
                return Ok(campath);
            }
        }
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(), "'this' is not a AdvancedfxCampath object", context).into())      
    }

    fn inner_from_this(this: &JsValue, context: &mut Context) -> JsResult<JsObject<CampathInner>> {
        let obj = Self::downcast_from_this(this, context)?;
        Ok(obj.borrow().data().inner.clone())
    }

    fn get_enabled(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_enabled()))
    }
    
    fn set_enabled(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(value) = args[0].as_boolean() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_enabled(value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }


    fn get_offset(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_offset()))
    }

    fn set_offset(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(value) = args[0].as_number() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_offset(value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_hold(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_hold()))
    }

    fn set_hold(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(value) = args[0].as_boolean() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_hold(value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_position_interp(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_position_interp() as u8))
    }

    fn set_position_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Ok(value) = args[0].to_uint8(context) {
                let Ok(e_value) = advancedfx::campath::DoubleInterp::try_from(value);
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_position_interp(e_value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_rotation_interp(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_rotation_interp() as u8))
    }

    fn set_rotation_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Ok(value) = args[0].to_uint8(context) {
                let Ok(e_value) = advancedfx::campath::QuaternionInterp::try_from(value);
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_rotation_interp(e_value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_fov_interp(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(js_value!(Self::inner_from_this(this,context)?.borrow().data().native.get_fov_interp() as u8))
    }

    fn set_fov_interp(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Ok(value) = args[0].to_uint8(context) {
                let Ok(e_value) = advancedfx::campath::DoubleInterp::try_from(value);
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_fov_interp(e_value);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn add(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 2 == args.len() {
            if let Some(time) = args[0].as_number() {
                if let Some(value_object) = args[1].as_object() {
                    if let Some(value_object_inner) = value_object.downcast_ref::<Value>() {
                        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.add(time,&value_object_inner.native);                          
                        return Ok(JsValue::undefined());
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn remove(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(time) = args[0].as_number() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.remove(time);      
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }
  
    fn clear(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.clear();
        Ok(JsValue::undefined())
    }

    fn get_size(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(JsValue::from(Self::inner_from_this(this,context)?.borrow().data().native.get_size()))
    }

    fn get_duration(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(JsValue::from(Self::inner_from_this(this,context)?.borrow().data().native.get_duration()))
    }

    fn get_lower_bound(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let inner = Self::inner_from_this(this,context)?;
        let borrow = inner.borrow();
        let native = &borrow.data().native;
        if 1 <= native.get_size() {
            return Ok(JsValue::from(native.get_lower_bound()));
        }
        Ok(JsValue::undefined())
    }

    fn get_upper_bound(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let inner = Self::inner_from_this(this,context)?;
        let borrow = inner.borrow();
        let native = &borrow.data().native;
        if 1 <= native.get_size() {
            return Ok(JsValue::from(native.get_upper_bound()));
        }
        Ok(JsValue::undefined())
    }

    fn get_can_eval(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Ok(JsValue::from(Self::inner_from_this(this,context)?.borrow().data().native.can_eval()))
    }

    fn eval(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let inner = Self::inner_from_this(this,context)?;
        let borrow = inner.borrow();
        let native = &borrow.data().native;
        if 1 == args.len() {
            if let Some(time) = args[0].as_number() {
                if native.can_eval() {
                    match Value::from_data(Value::new(native.eval(time)), context) {
                        Ok(result_object) => {
                            return Ok(js_value!(result_object));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn load(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(js_file_path) = args[0].as_string() {
                if let Ok(str_file_path) = js_file_path.to_std_string() {
                    let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.load(str_file_path);
                    return Ok(JsValue::from(result));
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn save(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(js_file_path) = args[0].as_string() {
                if let Ok(str_file_path) = js_file_path.to_std_string() {
                    return Ok(JsValue::from(Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.save(str_file_path)));
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn set_start(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 <= args.len() {
            if let Some(time) = args[0].as_number() {
                if 1 == args.len() {
                    Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_start(time,false);
                    return Ok(JsValue::undefined());
                }
                else if 2 == args.len() {
                    if let Some(relative) = args[1].as_boolean() {
                        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_start(time,relative);
                        return Ok(JsValue::undefined());
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }  

    fn set_duration(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(time) = args[0].as_number() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_duration(time);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn set_position(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
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
                        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_position(x,y,z);
                        return Ok(JsValue::undefined());                            
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn set_angles(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
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
                        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_angles(x,y,z);
                        return Ok(JsValue::undefined());                            
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn set_fov(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 1 == args.len() {
            if let Some(fov) = args[0].as_number() {
                Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.set_fov(fov);
                return Ok(JsValue::undefined());
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn rotate(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 3 == args.len() {
            if let Some(y_pitch) = args[0].as_number() {
                if let Some(z_yaw) = args[1].as_number() {
                    if let Some(x_roll) = args[2].as_number() {
                        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.rotate(y_pitch,z_yaw,x_roll);
                        return Ok(JsValue::undefined());
                    }
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn anchor_transform(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
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
                                                            Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.anchor_transform(anchor_x, anchor_y, anchor_z, anchor_y_pitch, anchor_z_yaw, anchor_x_roll, dest_x, dest_y, dest_z, dest_y_pitch, dest_z_yaw, dest_x_roll);
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
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn select_all(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_all();
        Ok(JsValue::from(result))
    }

    fn select_none(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_none();
        Ok(JsValue::undefined())
    }

    fn select_invert(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_invert();
        Ok(JsValue::from(result))
    }

    fn select_add_idx(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 2 == args.len() {
            if let Some(min) = args[0].as_number() {
                if let Some(max) = args[1].as_number() {
                    let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_add_idx(min as usize,max as usize);
                    return Ok(JsValue::from(result));
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn select_add_min_count(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 2 == args.len() {
            if let Some(min) = args[0].as_number() {
                if let Some(count) = args[1].as_number() {
                    let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_add_min_count(min,count as usize);
                    return Ok(JsValue::from(result));
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn select_add_min_max(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 2 == args.len() {
            if let Some(min) = args[0].as_number() {
                if let Some(max) = args[1].as_number() {
                    let result = Self::inner_from_this(this,context)?.borrow_mut().data_mut().native.select_add_min_max(min,max);
                    return Ok(JsValue::from(result));
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn get_on_changed(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        match &*Self::downcast_from_this(this,context)?.borrow().data().on_changed.borrow() {
            None => {
                Ok(JsValue::undefined())
            }
            Some(callback) => {
                Ok(js_value!(callback.clone()))
            }
        }
    }

    fn set_on_changed(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if 0 < args.len() {
            match &args[0].variant() {
                JsVariant::Undefined => {
                    let self_ = Self::downcast_from_this(this,context)?;
                    let borrow = self_.borrow();
                    let campath = &borrow.data();
                    campath.on_changed.replace(None);
                    let event_source = campath.inner.borrow().data().event_source.clone();
                    drop(borrow);
                    drop(self_);
                    EventSource::obj_off(&event_source, context, CAMPATH_ON_CHANGED_STR.to_string(), Some(-1.0));
                    return Ok(JsValue::undefined()); 
                }
                JsVariant::Object(object) => {
                    if let Some(callback) = JsFunction::from_object(object.clone()) {
                        let self_ = Self::downcast_from_this(this,context)?;
                        let borrow = self_.borrow();
                        let campath = &borrow.data();
                        campath.on_changed.replace(Some(callback.clone()));
                        let event_source = campath.inner.borrow().data().event_source.clone();
                        drop(borrow);
                        drop(self_);                        
                        let wrapper = NativeFunction::from_copy_closure_with_captures(
                            |_,_,callback,context| {
                                callback.call(&JsValue::undefined(), &[], context)
                            },
                            callback
                        );
                        EventSource::obj_on(&event_source,
                            context,
                            CAMPATH_ON_CHANGED_STR.to_string(),
                            wrapper.to_js_function(context.realm()),
                            Some(-1.0)
                        );
                        return Ok(JsValue::undefined()); 
                    }
                }
                _ => {
                }
            }
        }
        Err(advancedfx::js::errors::error_arguments(context).into())
    }
}

impl Class for Campath {
    const NAME: &'static str = "AdvancedfxCampath";
    const LENGTH: usize = 0;

    fn data_constructor(
        _this: &JsValue,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        return Ok(Campath::new(advancedfx::campath::Campath::new(), context));
    }

    fn object_constructor(
        instance: &JsObject<Campath>,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<()> {

        let borrow = instance.borrow();
        let obj_inner = &borrow.data().inner.clone();
        drop(borrow);

        let on_empty_changed = NativeFunction::from_copy_closure_with_captures(|_,args,obj_inner,context| {
            let mut inner_borrow_mut = obj_inner.borrow_mut();
            let inner = &mut inner_borrow_mut.data_mut();
            if args[0].as_boolean().unwrap() {
                // empty.
                if let Some(callback) = &inner.callback {
                    let observer =  callback.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                    inner.native.unregister(Rc::downgrade(&observer));
                }
                inner.callback = None;
            } else {
                if let Some(callback) = &inner.callback {
                    let observer =  callback.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                    inner.native.unregister(Rc::downgrade(&observer));
                }
                let callback = Rc::<RefCell<CampathChangedCallback>>::new(RefCell::<CampathChangedCallback>::new(CampathChangedCallback::new(
                    inner.event_source.clone(),
                    advancedfx::js::ContextRef::get_from_context(context)
                )));
                let observer =  callback.clone() as Rc<RefCell<dyn advancedfx::campath::CampathChangedObserver>>;
                inner.native.register(Rc::downgrade(&observer));                                    
                inner.callback = Some(callback);
            }
            Ok(JsValue::undefined())
        }, obj_inner.clone());

        let inner_borrow = obj_inner.borrow();
        let event_source = &inner_borrow.data().event_source;

        event_source.borrow_mut().data_mut().set_on_empty_changed(Some(on_empty_changed));

        let events_object = ObjectInitializer::new(context)
        .property(js_string!("changed"), event_source.clone(), Attribute::all())
        .build();

        let attribute = Attribute::all();

        let property = PropertyDescriptor::builder()
        .value(events_object)
        .writable(attribute.writable())
        .enumerable(attribute.enumerable())
        .configurable(attribute.configurable());

        let _ = instance.insert_property(js_string!("events"), property);

        Ok(())
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
                js_string!("setAngles"),
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
