use crate::advancedfx::js::errors;

use boa_engine::Finalize;
use boa_engine::JsData;
use boa_engine::JsError;
use boa_engine::JsObject;
use boa_engine::JsResult;
use boa_engine::JsValue;
use boa_engine::Trace;
use boa_engine::boa_class;
use boa_engine::js_string;
use boa_engine::class::Class;
use boa_engine::class::ClassBuilder;
use boa_engine::context::Context;
use boa_engine::error::JsNativeError;
use boa_engine::native_function::NativeFunction;
use boa_engine::property::Attribute;




#[derive(Trace, Finalize, JsData)]
struct Path {

    #[unsafe_ignore_trace]
    path: std::path::PathBuf
}
impl Path {

    fn to_string(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(js_string!(self_.path.as_path().to_string_lossy().to_string())))
            }
        }
        Err(errors::error_type(context).into())
    }

    fn is_absolute(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(self_.path.as_path().is_absolute()))
            }
        }
        Err(errors::error_type(context).into())
    }

    fn is_relative(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(self_.path.as_path().is_relative()))
            }
        }
        Err(errors::error_type(context).into())
    }

    fn is_dir(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(self_.path.as_path().is_dir()))
            }
        }
        Err(errors::error_type(context).into())
    }

    fn is_file(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(self_.path.as_path().is_file()))
            }
        }
        Err(errors::error_type(context).into())
    }

    fn is_symlink(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(self_) = &object.downcast_ref::<Path>(){
                return Ok(JsValue::from(self_.path.as_path().is_symlink()))
            }
        }
        Err(errors::error_type(context).into())
    }
}

impl Class for Path {
    const NAME: &'static str = "AdvancedfxPath";
    const LENGTH: usize = 1;

    fn data_constructor(
        this: &JsValue,
        args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        let (path_str, _): (String, &[boa_engine::JsValue]) =boa_engine::interop::TryFromJsArgument::try_from_js_argument( this, args, context )?;

        let path = std::path::Path::new(&path_str).to_path_buf();

        Ok(Self{path})
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        class
            .method(
                js_string!("toString"),
                0,
                NativeFunction::from_fn_ptr(Path::to_string)
            )
            .method(
                js_string!("isAbsolute"),
                0,
                NativeFunction::from_fn_ptr(Path::is_absolute)
            )
            .method(
                js_string!("isRelative"),
                0,
                NativeFunction::from_fn_ptr(Path::is_relative)
            )
            .method(
                js_string!("isDir"),
                0,
                NativeFunction::from_fn_ptr(Path::is_dir)
            )
            .method(
                js_string!("isFile"),
                0,
                NativeFunction::from_fn_ptr(Path::is_file)
            )
            .method(
                js_string!("isSymlink"),
                0,
                NativeFunction::from_fn_ptr(Path::is_symlink)
            )
            .static_property(
                js_string!("MAIN_SEPARATOR"),
                js_string!(std::path::MAIN_SEPARATOR_STR.to_string()),
                Attribute::all()
            );
        
        Ok(())
    }
}

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

    fn path(&self, context: &mut Context) -> JsResult<JsObject> {

        let path = self.dir_entry.path().to_path_buf();

        let data = Path{path};

        Path::from_data(data, context)
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

impl Fs {
    fn js_read_dir(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let (path, _): (String, &[boa_engine::JsValue]) =boa_engine::interop::TryFromJsArgument::try_from_js_argument( this, args, context )?;

        let read_dir = std::fs::read_dir(path).map_err(|e| JsError::from_native(JsNativeError::error().with_message(e.to_string())))?;

        let data = self::ReadDir{read_dir};

        let js_object = self::ReadDir::from_data(data, context)?;

        Ok(JsValue::from(js_object))
    }
}

impl Class for Fs {
    const NAME: &'static str = "AdvancedfxFs";
    const LENGTH: usize = 0;

    fn data_constructor(
        _this: &JsValue,
        _args: &[JsValue],
        _context: &mut Context,
    ) -> JsResult<Self> {
        return Ok(Self{});
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        class
            .static_method(
                js_string!("readDir"),
                1,
                NativeFunction::from_fn_ptr(Fs::js_read_dir)
            )
            .static_property(
                js_string!("MAIN_SEPARATOR"),
                js_string!(std::path::MAIN_SEPARATOR_STR.to_string()),
                Attribute::all()
            );
        
        Ok(())
    }
}


pub fn register(context: &mut Context) -> JsResult<()> {
    context.register_global_class::<Path>()?;
    context.register_global_class::<DirEntry>()?;
    context.register_global_class::<ReadDir>()?;
    context.register_global_class::<Fs>()?;
    Ok(())
}
