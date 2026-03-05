use boa_engine::{
    JsNativeError,
    Context
};

#[macro_export]
macro_rules! make_error {
    ($js_native_error: expr, $message: expr, $context: expr) => {
        {
            let _ = $context;
            $js_native_error.with_message($message)
        }
    };    
}

pub use make_error; 

pub fn error_type(context: &Context) -> JsNativeError {
    make_error!(JsNativeError::typ(),"invalid type!",context)
}

pub fn error_arguments(context: &Context) -> JsNativeError {
    make_error!(JsNativeError::error(),"invalid arguments!",context)
}

pub fn error_async_conflict(context: &Context) -> JsNativeError {
    make_error!(JsNativeError::error(),"async conflict!",context)
}

pub fn get_stack_dump(context: &Context) -> String {
    return context
    .stack_trace()
    .map(|frame| frame.code_block().name())
    .collect::<Vec<_>>()
    .into_iter()
    .map(boa_engine::JsString::to_std_string_escaped)
    .collect::<Vec<_>>()
    .join("\n");
}
