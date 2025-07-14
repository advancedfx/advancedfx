pub mod campath;
pub mod errors;
pub mod math;

use std::rc::Rc;

use boa_engine::{
    Context,
    Finalize,
    JsData,
    Trace
};

#[derive(Trace, Finalize, JsData)]
pub struct ContextMutRef {

    #[unsafe_ignore_trace]
    ptr: * mut Context,

    #[unsafe_ignore_trace]
    rc: Rc::<ContextHolder>
}

impl ContextMutRef {
    pub fn new(context_holder: Rc::<ContextHolder>) -> Self {
        Self {
            ptr: context_holder.ptr,
            rc: context_holder.clone()
        }
    }  
    
    pub fn get<'a>(&self) -> &'a mut Context {
        unsafe{&mut *self.ptr}
    }
}

impl Clone for ContextMutRef {
    fn clone(&self) -> Self {
        Self {
            ptr: self.ptr,
            rc: self.rc.clone()
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

        result.get().insert_data(ContextMutRef::new(result.clone()));

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
