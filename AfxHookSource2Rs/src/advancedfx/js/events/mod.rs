use boa_engine::Finalize;
use boa_engine::Context;
use boa_engine::JsData;
use boa_engine::JsString;
use boa_engine::JsObject;
use boa_engine::JsResult;
use boa_engine::JsValue;
use boa_engine::JsVariant;
use boa_engine::Trace;
use boa_engine::boa_class;
use boa_engine::class::Class;
use boa_engine::js_value;
use boa_engine::object::Ref;
use boa_engine::object::RefMut;
use boa_engine::property::Attribute;
use boa_engine::property::PropertyDescriptor;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::collections::HashMap;
use std::rc::Weak;
use std::vec::Vec;

#[derive(Clone, Trace, Finalize, JsData)]
pub struct Event {
    abort: bool,
    result: JsValue
}

impl Event {
    pub fn get_abort(&self) -> bool {
        self.abort
    }
}

#[boa_class(rename="AdvancedfxEvent")]
impl Event {
    #[boa(constructor)]
    pub fn new(default_result: JsValue) -> Self {
        Self {
            abort: false,
            result: default_result
        }
    }

    pub fn abort(&mut self) {
        self.abort = true;
    }

    #[boa(getter)]
    #[boa(rename="result")]
    pub fn get_result(&self) -> JsValue {
        self.result.clone()
    }

    #[boa(setter)]
    #[boa(rename="result")]
    pub fn set_result(&mut self, value: JsValue) {
        self.result = value;
    }
}

pub struct EventListener {
    name: String,
    callback: JsObject
}

pub trait EventSourceEmptyChanged {
    fn notify(&mut self, source: &mut EventSource, value: bool);
}

struct EventPriority(f64);
impl PartialEq for EventPriority {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == std::cmp::Ordering::Equal
    }
}
impl Eq for EventPriority {}
impl PartialOrd for EventPriority {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        self.0.partial_cmp(&other.0)
    }
}
impl Ord for EventPriority{
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.0.partial_cmp(&other.0).unwrap_or(std::cmp::Ordering::Equal)
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct EventSource {

    #[unsafe_ignore_trace]
    listeners: BTreeMap<EventPriority,Vec<EventListener>>,

    #[unsafe_ignore_trace]
    on_empty_changed: Option<Weak<RefCell<dyn EventSourceEmptyChanged>>>
}

impl EventSource {
    pub fn set_on_empty_changed(&mut self, value: Option<Weak<RefCell<dyn EventSourceEmptyChanged>>>) {
        self.on_empty_changed = value;
    }


    pub fn get(&self, name: String, f_priority: f64) -> Option<JsObject> {
        let priority = EventPriority(f_priority);
        if let Some(listener_vec) = self.listeners.get(&priority) {
            if let Some(listener) = listener_vec.iter().find(|&listener| listener.name == name) {
                    return Some(listener.callback.clone());
            }
        }        
        None 
    }

    pub fn get_callbacks(&self) -> Vec<JsObject> {
        let mut result = Vec::<JsObject>::new();
        for listener_vec in self.listeners.values() {
            for listener in listener_vec {
                result.push(listener.callback.clone());
            }
        }
        return result;
    }

    pub fn dispatch(source: &EventSource, context: &mut boa_engine::Context, default_result: JsValue, properties: HashMap<JsString,JsValue>) -> JsResult<JsValue> {
        match Event::from_data(Event::new(default_result), context) {
            Ok(event) => {
                let attribute = Attribute::all();
                for (k,v) in properties {
                    let property = PropertyDescriptor::builder()
                        .value(v)
                        .writable(attribute.writable())
                        .enumerable(attribute.enumerable())
                        .configurable(attribute.configurable());
                    let _ = event.insert_property(k,property);
                }
                let callbacks = source.get_callbacks();
                for callback in callbacks {
                    let result = callback.call(&js_value!(callback.clone()), &[js_value!(event.clone())], context);
                    match result {
                        Ok(val) =>{
                            match val.variant() {
                                JsVariant::Undefined => {
                                    // continue unaltered
                                },
                                _ => {
                                    event.downcast_mut::<Event>().unwrap().set_result(val);
                                }
                            }

                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }                
                    if event.downcast_ref::<Event>().unwrap().get_abort() {
                        break;
                    }
                }
                return Ok(event.downcast_ref::<Event>().unwrap().get_result().clone());
            }
            Err(e) => {
                return Err(e);
            }
        }  
    }     
}

#[boa_class(rename="AdvancedfxEventSource")]
impl EventSource {
    
    #[boa(constructor)]
    pub fn new() -> Self {
        Self {
            listeners: BTreeMap::<EventPriority,Vec<EventListener>>::new(),
            on_empty_changed: None
        }
    }

    pub fn on(&mut self, name: String, callback: JsObject, priority_opt: Option<f64>) -> Option<JsObject> {
        let priority = EventPriority(priority_opt.unwrap_or(0.0));
        if let Some(listener_vec) = self.listeners.get_mut(&priority) {
            let mut last: Option<JsObject> = None;
            listener_vec.retain(|listener| {
                if listener.name == name {
                    last = Some(listener.callback.clone());
                    return false;
                }
                true
            });
            listener_vec.push(EventListener{
                name: name,
                callback: callback
            });
            return last;
        }

        let was_empty = self.listeners.is_empty();

        self.listeners.insert(priority, Vec::<EventListener>::new());

        if was_empty {
            if let Some(on_empty_changed) = &self.on_empty_changed {
                if let Some(rc) = on_empty_changed.upgrade() {
                    rc.borrow_mut().notify(self, false);
                }
            }
        }

        None
    }

    pub fn off(&mut self, name: String, priority_opt: Option<f64>) -> Option<JsObject> {
        let priority = EventPriority(priority_opt.unwrap_or(0.0));
        let mut last: Option<JsObject> = None;
        let mut listener_vec_empty = false;
        if let Some(listener_vec) = self.listeners.get_mut(&priority) {
            listener_vec.retain(|listener| {
                if listener.name == name {
                    last = Some(listener.callback.clone());
                    return false;
                }
                true
            });
            listener_vec_empty = listener_vec.is_empty();
        }
        if listener_vec_empty {
            self.listeners.remove(&priority);

            if self.listeners.is_empty() {            
                if let Some(on_empty_changed) = &self.on_empty_changed {
                    if let Some(rc) = on_empty_changed.upgrade() {
                        rc.borrow_mut().notify(self, true);
                    }
                }
            }
        }
        last     
    }
}

pub struct EventSourceContainer {
    js_object: JsObject
}

impl EventSourceContainer {
    pub fn new(event_source: EventSource, context: &mut Context) -> Self {
        Self {
            js_object: EventSource::from_data(event_source, context).unwrap()
        }
    }

    pub fn outer_clone(&self) -> JsObject {
        self.js_object.clone()
    }

    pub fn inner_ref(&self) -> Ref<'_, EventSource> {
        self.js_object.downcast_ref::<EventSource>().unwrap()
    }

    pub fn inner_mut(&self) -> RefMut<'_, EventSource> {
        self.js_object.downcast_mut::<EventSource>().unwrap()
    }
}
