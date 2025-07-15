pub mod campath;
pub mod errors;
pub mod math;

use std::rc::Rc;
use std::rc::Weak;

use boa_engine::{
    Context,
    Finalize,
    JsData,
    Trace
};

#[derive(Trace, Finalize, JsData)]
pub struct ContextMutRef {

    #[unsafe_ignore_trace]
    context_holder: Weak::<ContextHolder>
}

impl ContextMutRef {
    pub fn new(context_holder: Weak::<ContextHolder>) -> Self {
        Self {
            context_holder: context_holder
        }
    }  
    
    pub fn get<'a>(&self) -> Option<&'a mut Context> {
        self.context_holder.upgrade().map(|x| unsafe{&mut *x.ptr})        
    }
}

impl Clone for ContextMutRef {
    fn clone(&self) -> Self {
        Self {
            context_holder: self.context_holder.clone()
        }
    }
}

pub struct ContextHolder {
    ptr: * mut Context
}

impl ContextHolder {
    pub fn new(context: Context) -> Rc<Self> {
        let ptr = Box::into_raw(Box::new(context));

        let result = Rc::<ContextHolder>::new(Self {
            ptr: ptr
        });

        result.get().insert_data(ContextMutRef::new(Rc::downgrade(&result)));

        result
    }

    pub fn get<'a>(&self) -> &'a mut Context {
        unsafe{&mut *self.ptr}
    }
}

impl Drop for ContextHolder {
    fn drop(&mut self) {
        if self.ptr.is_null() {
            return;
        }
        self.get().remove_data::<ContextMutRef>();
        drop(unsafe{Box::from_raw(self.ptr)});
        self.ptr = std::ptr::null_mut();
    }
}
