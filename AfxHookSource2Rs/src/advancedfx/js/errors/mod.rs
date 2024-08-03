use boa_engine::{
    JsNativeError,
    JsError
};

pub fn error_type() -> JsError {
    JsNativeError::typ().with_message("invalid type!").into()
}

pub fn error_arguments() -> JsError {
    JsNativeError::error().with_message("invalid arguments!").into()
}

pub fn error_async_conflict() -> JsError {
    JsNativeError::error().with_message("async conflict!").into()
}
