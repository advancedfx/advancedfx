use crate::advancedfx::js::errors;

use boa_engine::Finalize;
use boa_engine::JsData;
use boa_engine::JsError;
use boa_engine::JsObject;
use boa_engine::JsResult;
use boa_engine::JsValue;
use boa_engine::Trace;
use boa_engine::boa_class;
use boa_engine::class::Class;
use boa_engine::context::Context;
use boa_engine::error::JsNativeError;

#[derive(Trace, Finalize, JsData)]
struct DirEntry {

    #[unsafe_ignore_trace]
    dir_entry: std::fs::DirEntry
}

#[boa_class(rename = "AdvancedfxFsDirEntry")]
impl DirEntry {

    #[boa(constructor)]
    fn new() -> JsResult<Self> {
        Err(JsError::from_native(errors::error_not_implemented()))
    }

    fn path(&self) -> String {
        self.dir_entry.path().to_string_lossy().to_string()
    }
}

#[derive(Trace, Finalize, JsData)]
struct ReadDir {

    #[unsafe_ignore_trace]
    read_dir: std::fs::ReadDir
}

#[boa_class(rename = "AdvancedfxFsReadDir")]
impl ReadDir {

    #[boa(constructor)]
    fn new() -> JsResult<Self> {
        Err(JsError::from_native(errors::error_not_implemented()))
    }

    fn next(&mut self, context: &mut Context) -> JsResult<Option<JsObject>> {
        if let Some(dir_entry_result) = self.read_dir.next() {
            let dir_entry = dir_entry_result.map_err(|e| JsError::from_native(JsNativeError::error().with_message(e.to_string())))?;

            let data = DirEntry{dir_entry};

            let object = DirEntry::from_data(data, context)?;

            return Ok(Some(object));
        }

        Ok(None)
    }
}

#[derive(Trace, Finalize, JsData)]
pub struct Fs {
}

#[boa_class(rename = "AdvancedfxFs")]
impl Fs {

    #[boa(constructor)]
    fn new() -> JsResult<Self> {
        Ok(Self {})
    }

    fn read_dir(path: String, context: &mut Context) -> JsResult<JsObject> {
        let read_dir = std::fs::read_dir(path).map_err(|e| JsError::from_native(JsNativeError::error().with_message(e.to_string())))?;

        let data = self::ReadDir{read_dir};

        self::ReadDir::from_data(data, context)
    }

    fn main_path_separator() -> String {
       std::path::MAIN_SEPARATOR_STR.to_string()
    }
}

impl Fs{
    pub fn register(args: &[JsValue], context: &mut Context) -> JsResult<JsObject> {
        context.register_global_class::<DirEntry>()?;
        context.register_global_class::<ReadDir>()?;
        context.register_global_class::<Fs>()?;
    
        let class = context.get_global_class::<Fs>().ok_or_else(||JsNativeError::error())?;
        Fs::construct(&JsValue::from(class.constructor()),args,context)
    }
}
