pub mod campath;
pub mod cvar;
pub mod errors;
pub mod math;

use std::rc::Rc;
use std::rc::Weak;

use boa_engine::Context;
use boa_engine::Finalize;
use boa_engine::JsData;
use boa_engine::Trace;

pub struct BoxedContext {
    ptr: *mut Context
}

impl Finalize for BoxedContext {
    fn finalize(&self) {
        unsafe {
            drop(Box::from_raw(self.ptr));
        }
    }
}

impl BoxedContext {
    pub fn new(context: Context) -> Self {
        Self {
            ptr: Box::into_raw(Box::new(context))
        }
    }

    pub fn get(&self) -> &mut Context {
        unsafe {&mut *self.ptr}
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct ContextRef {
    #[unsafe_ignore_trace]
    context: Weak<BoxedContext>
}

impl ContextRef {
    pub fn add_to_context(context:  Rc<BoxedContext>) {
        let context_ref = ContextRef {
            context: Rc::downgrade(&context)
        };

        context.get().insert_data(context_ref);
    }

    pub fn get_from_context(context: &Context) -> Rc::<BoxedContext> {
        context.get_data::<ContextRef>().unwrap().context.upgrade().unwrap()
    }

    pub fn get_from_context_weak(context: &Context) -> Weak::<BoxedContext> {
        context.get_data::<ContextRef>().unwrap().context.clone()
    }
}
