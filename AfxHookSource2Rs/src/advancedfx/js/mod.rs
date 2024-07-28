pub mod campath;
pub mod errors;
pub mod math;

use std::cell::RefCell;
use std::rc::Weak;

use boa_engine::{
    Context,
    Finalize,
    JsData,
    Trace
};

pub struct ContextWrapper {
    pub context: Context,
}

impl ContextWrapper {
    pub fn new(context: Context) -> Self {
        Self {
            context: context
        }
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct WeakContextWrapper {
    #[unsafe_ignore_trace]
    pub context_wrapper: Weak<RefCell<ContextWrapper>>
}

impl WeakContextWrapper {
    pub fn new(context_wrapper: Weak<RefCell<ContextWrapper>>) -> Self {
        Self {
            context_wrapper: context_wrapper
        }
    }
}

impl Clone for WeakContextWrapper {
    fn clone(&self) -> Self {
        Self {
            context_wrapper: self.context_wrapper.clone()
        }
    }
}