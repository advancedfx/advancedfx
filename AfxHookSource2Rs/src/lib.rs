mod advancedfx;

use core::ffi::c_char;
use core::ffi::CStr;
use std::ffi::c_float;
use std::ffi::c_void;

use std::{cell::RefCell, collections::VecDeque, future::Future, pin::Pin};
use std::task::Poll::Ready;
use std::rc::Rc;
use std::rc::Weak;
use std::path::Path;
use std::path::PathBuf;

use futures::SinkExt;
use futures::StreamExt;
use futures::stream::SplitSink;
use futures::stream::SplitStream;

use boa_gc::GcRefCell;
use boa_engine::{
    class::{
        Class,
        ClassBuilder,
    },
    Context,
    builtins::promise::PromiseState,
    context::{
        ContextBuilder,
    },
    JsError,
    JsResult,
    JsNativeError,
    JsNativeErrorKind,
    JsObject,
    JsString,
    JsValue,
    JsBigInt,
    job::{
        NativeJob,
        FutureJob,
        JobQueue,
    },
    js_string,
    module::{
        ModuleLoader,
        Referrer,
        resolve_module_specifier
    },
    Module,
    native_function::NativeFunction,
    object::ObjectInitializer,
    object::builtins::JsArray,
    object::builtins::JsArrayBuffer,
    object::builtins::JsPromise,
    property::Attribute,
    Source
};
use boa_runtime::{
    Console,
    ConsoleState,
    Logger
};

use async_tungstenite::async_std::ConnectStream;
use async_tungstenite::tungstenite::protocol::Message;
use async_tungstenite::WebSocketStream;

type AfxEntityRef = c_void;

extern "C" {
    fn afx_hook_source2_message(s: *const c_char);
    fn afx_hook_source2_warning(s: *const c_char);
    fn afx_hook_source2_exec(s: *const c_char);

    fn afx_hook_source2_enable_on_record_start(value: bool);
    fn afx_hook_source2_enable_on_record_end(value: bool);
    fn afx_hook_source2_enable_on_game_event(value: bool);
    fn afx_hook_source2_enable_on_c_view_render_setup_view(value: bool);
    fn afx_hook_source2_enable_on_client_frame_stage_notify(value: bool);

    fn afx_hook_source2_make_handle(entry_index: i32, serial_number: i32) -> i32;
    fn afx_hook_source2_is_handle_valid(handle: i32) -> bool;
    fn afx_hook_source2_get_handle_entry_index(handle: i32) -> i32;
    fn afx_hook_source2_get_handle_serial_number(handle: i32) -> i32;

    fn afx_hook_source2_get_highest_entity_index() -> i32;

    fn afx_hook_source2_get_entity_ref_from_index(index: i32) -> * mut AfxEntityRef;
    fn afx_hook_source2_get_entity_ref_from_split_screen_player(index: i32) -> * mut AfxEntityRef;

    fn afx_hook_source2_add_ref_entity_ref(p_ref: * mut AfxEntityRef);
    fn afx_hook_source2_release_entity_ref(p_ref: * mut AfxEntityRef);

    fn afx_hook_source2_enable_on_add_entity(value: bool);
    fn afx_hook_source2_enable_on_remove_entity(value: bool);
    
    fn afx_hook_source2_get_entity_ref_is_valid(p_ref: * mut AfxEntityRef) -> bool;
    fn afx_hook_source2_get_entity_ref_name(p_ref: * mut AfxEntityRef) -> *const c_char;
    
    // can return nullptr to indicate no debug name.
    fn afx_hook_source2_get_entity_ref_debug_name(p_ref: * mut AfxEntityRef) -> *const c_char;

    fn afx_hook_source2_get_entity_ref_class_name(p_ref: * mut AfxEntityRef) -> *const c_char;

    fn afx_hook_source2_get_entity_ref_client_class_name(p_ref: * mut AfxEntityRef) -> *const c_char;

    fn afx_hook_source2_get_entity_ref_is_player_pawn(p_ref: * mut AfxEntityRef) -> bool;

    fn afx_hook_source2_get_entity_ref_player_pawn_handle(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_is_player_controller(p_ref: * mut AfxEntityRef) -> bool;

    fn afx_hook_source2_get_entity_ref_player_controller_handle(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_health(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_team(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_origin(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32);

    fn afx_hook_source2_get_entity_ref_render_eye_origin(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32);

    fn afx_hook_source2_get_entity_ref_render_eye_angles(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32);

    fn afx_hook_source2_get_entity_ref_view_entity_handle(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_active_weapon_handle(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_get_entity_ref_observer_mode(p_ref: * mut AfxEntityRef) -> u8;

    fn afx_hook_source2_get_entity_ref_observer_target_handle(p_ref: * mut AfxEntityRef) -> i32;

    fn afx_hook_source2_is_playing_demo() -> bool;

    fn afx_hook_source2_is_demo_paused() -> bool;

    fn afx_hook_source2_get_main_campath() -> * mut advancedfx::campath::CampathType;

    // can return nullptr to indicate no debug name.
    fn afx_hook_source2_get_entity_ref_player_name(p_ref: * mut AfxEntityRef) -> *const c_char;

    fn afx_hook_source2_get_entity_ref_steam_id(p_ref: * mut AfxEntityRef) -> u64;

    // can return nullptr to indicate no debug name.
    fn afx_hook_source2_get_entity_ref_sanitized_player_name(p_ref: * mut AfxEntityRef) -> *const c_char;

    fn afx_hook_source2_get_demo_tick(outTick: & mut i32) -> bool;

    fn afx_hook_source2_get_demo_time(outTime: & mut f64) -> bool;

    fn afx_hook_source2_get_cur_time(outCurTime: & mut f64);

    fn afx_hook_source2_get_entity_ref_attachment(p_ref: * mut AfxEntityRef, attachment_name: *const c_char, pos: * mut advancedfx::math::Vector3, angs: * mut advancedfx::math::Quaternion) -> bool;
}

////////////////////////////////////////////////////////////////////////////////

type CommandArgsRs = c_void;
type CommandCallbackRs = extern "C" fn(p_user_data: * mut c_void, p_command_args: * mut CommandArgsRs);
type ConCommandRs = c_void;

extern "C" {
    fn afx_hook_source2_new_command(psz_name: *const c_char, psz_help_string: *const c_char, flags: i64, additional_flags: i64, p_callback: CommandCallbackRs, p_user_data: * mut c_void) -> * mut ConCommandRs;

    fn afx_hook_source2_delete_command(p_con_command: * mut ConCommandRs);

    fn afx_hook_source2_command_args_argc(p_command_args: * mut CommandArgsRs) -> i32;
    
    fn afx_hook_source2_command_args_argv(p_command_args: * mut CommandArgsRs, index: i32) -> *const c_char;
}

#[derive(Trace, Finalize, JsData)]
struct ConCommandsArgs {
    #[unsafe_ignore_trace]
    p_command_args: Weak<* mut CommandArgsRs>
}

impl ConCommandsArgs {
    #[must_use]
    pub fn new(p_command_args: Weak<* mut CommandArgsRs>) -> Self {
        Self {
            p_command_args: p_command_args
        }   
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<ConCommandsArgs>()
            .expect("the AdvancedfxConCommandArgs builtin shouldn't exist");        
    }    

    fn error_typ(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(),"'this' is not a AdvancedfxConCommandArgs object",context).into())
    }

    fn arg_c(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(con_command_args) = object.downcast_mut::<ConCommandsArgs>() {
                if let Some(ptr) = con_command_args.p_command_args.upgrade() {
                    let result = unsafe {
                        afx_hook_source2_command_args_argc(*ptr)
                    };
                    return Ok(JsValue::Integer(result));
                }
            }
        }
        Self::error_typ(context)
    }

    fn arg_v(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(con_command_args) = object.downcast_mut::<ConCommandsArgs>() {
                if 1 == args.len() {
                    if let Ok(index) = args[0].to_i32(context) {
                        if let Some(ptr) = con_command_args.p_command_args.upgrade() {
                            let str_result = unsafe {CStr::from_ptr(
                                afx_hook_source2_command_args_argv(*ptr, index)
                            )}.to_str().unwrap();
                            return Ok( JsValue::String(js_string!(str_result)));
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into());
            }
        }
        Self::error_typ(context)
    }       
}


impl Class for ConCommandsArgs {
    const NAME: &'static str = "AdvancedfxConCommandArgs";
    const LENGTH: usize = 0;

    fn data_constructor(
        _this: &JsValue,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        return Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        class
            .method(
                js_string!("argC"),
                0,
                NativeFunction::from_fn_ptr(ConCommandsArgs::arg_c)
            )
            .method(
                js_string!("argV"),
                1,
                NativeFunction::from_fn_ptr(ConCommandsArgs::arg_v)
            );
        Ok(())
    }
}


#[derive(Trace, Finalize, JsData)]
struct ConCommand {
    #[unsafe_ignore_trace]
    callback: JsObject,

    context: advancedfx::js::ContextMutRef
}

impl ConCommand {
    #[must_use]
    pub fn new(callback: JsObject,  context: advancedfx::js::ContextMutRef) -> Self {
        Self {
            callback : callback,
            context: context
        }   
    }

    fn callback(&mut self, p_command_args: * mut CommandArgsRs) {
        if let Some(context) = self.context.get() {
            let rc = Rc::new(p_command_args);
            if let Ok(result_object) = ConCommandsArgs::from_data(ConCommandsArgs::new(Rc::downgrade(&rc.clone())), context) {
                if let Err(e) = self.callback.call(&JsValue::null(), &[JsValue::Object(result_object)], context) {
                    let _ = afx_on_error(&e, context);
                }
            }
        }
    }    
} 

extern "C" fn afx_hook_source2_callback_fn_impl(p_user_data: * mut c_void, p_command_args: * mut CommandArgsRs) {
    let con_command: &mut ConCommand = unsafe { &mut *(p_user_data as *mut ConCommand) };
    con_command.callback(p_command_args);
}

#[derive(Trace, JsData)]
struct ConCommandBox {    
    #[unsafe_ignore_trace]
    ptr: * mut ConCommand,

    #[unsafe_ignore_trace]
    handle: * mut ConCommandRs,
}

impl ConCommandBox {
    #[must_use]
    pub fn new(ptr: * mut ConCommand) -> Self {
        Self {
            ptr: ptr,
            handle: std::ptr::null_mut()
        }   
    }

    pub fn add_to_context(context: &mut Context) {
        context
            .register_global_class::<ConCommandBox>()
            .expect("the AdvancedfxConCommand builtin shouldn't exist");        
    }

    fn error_typ(context: &Context) -> JsResult<JsValue> {
        Err(advancedfx::js::errors::make_error!(JsNativeError::typ(),"'this' is not a AdvancedfxConCommand object",context).into())
    }

    fn unregister(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut con_command_box) = object.downcast_mut::<ConCommandBox>() {
                con_command_box.do_unregister();
                return Ok(JsValue::undefined());
            }
        }
        Self::error_typ(context)
    }
    
    fn register(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mut con_command_box) = object.downcast_mut::<ConCommandBox>() {
                if 2 == args.len() {
                    if let Some(name) = args[0].as_string() {
                        if let Ok(str_name) = name.to_std_string() {
                            if let Some(description) = args[1].as_string() {
                                if let Ok(str_description) = description.to_std_string() {
                                    con_command_box.do_register(str_name, str_description, 0, 0);
                                    return Ok(JsValue::Undefined);
                                }
                            }
                        }
                    }
                }
                return Err(advancedfx::js::errors::error_arguments(context).into())
            }
        }
        Self::error_typ(context)
    }

    fn do_register(&mut self, name: String, description: String, flags: i64, additional_flags: i64) {
        self.do_unregister();
        let c_name = std::ffi::CString::new(name).unwrap();
        let c_description = std::ffi::CString::new(description).unwrap();
        self.handle = unsafe { afx_hook_source2_new_command(c_name.as_ptr(), c_description.as_ptr(), flags, additional_flags, afx_hook_source2_callback_fn_impl, self.ptr as *mut c_void) };
    }

    fn do_unregister(&mut self) {
        if !self.handle.is_null() {
            unsafe {
                afx_hook_source2_delete_command(self.handle);
            }
            self.handle = std::ptr::null_mut();
        }
    }
}

impl Finalize for ConCommandBox {
    fn finalize(&self) {
        if !self.handle.is_null() {
            unsafe {
                afx_hook_source2_delete_command(self.handle);
            }
        }
        unsafe {
            drop(Box::from_raw(self.ptr));
        }
    }
}

impl Class for ConCommandBox {
    const NAME: &'static str = "AdvancedfxConCommand";
    const LENGTH: usize = 1;

    fn data_constructor(
        _this: &JsValue,
        args: &[JsValue],
        context: &mut Context,
    ) -> JsResult<Self> {
        if 1 == args.len() {
            if let Some(object) = args[0].as_object() {
                if object.is_callable() {
                    if let Some(context) = context.get_data::<advancedfx::js::ContextMutRef>() {
                        let command_ptr = Box::into_raw(Box::new(ConCommand::new(object.clone(), context.clone())));
                        return Ok(ConCommandBox::new(command_ptr));        
                    }
                    return Err(advancedfx::js::errors::error_no_wrapper(context).into());
                }
            }
        }
        return Err(advancedfx::js::errors::error_arguments(context).into())
    }

    fn init(class: &mut ClassBuilder<'_>) -> JsResult<()> {
        class
            .method(
                js_string!("register"),
                2,
                NativeFunction::from_fn_ptr(ConCommandBox::register)
            )
            .method(
                js_string!("unregister"),
                0,
                NativeFunction::from_fn_ptr(ConCommandBox::unregister)
            );
            Ok(())
    }
}



////////////////////////////////////////////////////////////////////////////////

#[derive(Debug)]
pub struct AfxSimpleModuleLoader {
    module_map: GcRefCell<std::collections::HashMap<PathBuf, Module>>,
}

impl AfxSimpleModuleLoader {
    /// Creates a new `AfxSimpleModuleLoader`
    pub fn new() -> AfxSimpleModuleLoader {
        Self {
            module_map: GcRefCell::default(),
        }
    }

    /// Inserts a new module onto the module map.
    #[inline]
    pub fn insert(&self, path: PathBuf, module: Module) {
        self.module_map.borrow_mut().insert(path, module);
    }

    /// Gets a module from its original path.
    #[inline]
    pub fn get(&self, path: &Path) -> Option<Module> {
        self.module_map.borrow().get(path).cloned()
    }
}

impl ModuleLoader for AfxSimpleModuleLoader {
    fn load_imported_module(
        &self,
        referrer: Referrer,
        specifier: JsString,
        finish_load: Box<dyn FnOnce(JsResult<Module>, &mut Context)>,
        context: &mut Context,
    ) {
        let result = (|| {
            let short_path = specifier.to_std_string_escaped();
            let path =
                resolve_module_specifier(None, &specifier, referrer.path(), context)?;
            if let Some(module) = self.get(&path) {
                return Ok(module);
            }

            let source = Source::from_filepath(&path).map_err(|err| {
                advancedfx::js::errors::make_error!(JsNativeError::error(),format!("could not open file `{short_path}`").to_string(),context)
                    .with_cause(JsError::from_opaque(js_string!(err.to_string()).into()))
            })?;
            let module = Module::parse(source, None, context).map_err(|err| {
                advancedfx::js::errors::make_error!(JsNativeError::syntax(),format!("could not parse module `{short_path}`").to_string(),context)
                    .with_cause(err)
            })?;
            self.insert(path, module.clone());
            Ok(module)
        })();

        finish_load(result, context);
    }

    fn register_module(&self, specifier: JsString, module: Module) {
        let path = PathBuf::from(specifier.to_std_string_escaped());

        self.insert(path, module);
    }

    fn get_module(&self, specifier: JsString) -> Option<Module> {
        let path = specifier.to_std_string_escaped();

        self.get(Path::new(&path))
    }
}


/**
 * @todo Implement context / waker.
 */
pub struct AsyncJobQueue {
    jobs: RefCell<VecDeque<NativeJob>>,
    futures: RefCell<Vec<FutureJob>>
}

impl AsyncJobQueue {
    #[must_use]
    pub fn new() -> Self {
        let jobs = RefCell::<VecDeque<NativeJob>>::default();
        let futures = RefCell::<Vec<FutureJob>>::default();
        Self { jobs, futures }
    }
}

impl JobQueue for AsyncJobQueue {
    fn enqueue_promise_job(&self, job: NativeJob, _context: &mut Context) {
        self.jobs.borrow_mut().push_back(job);
    }

    fn run_jobs(&self, context: &mut Context) {
        // Yeah, I have no idea why Rust extends the lifetime of a `RefCell` that should be immediately
        // dropped after calling `pop_front`.
        let mut next_job = self.jobs.borrow_mut().pop_front();
        while let Some(job) = next_job {
            if job.call(context).is_err() {
                self.jobs.borrow_mut().clear();
                return;
            };
            next_job = self.jobs.borrow_mut().pop_front();
        }
    }

    fn enqueue_future_job(&self, future: FutureJob, _context: &mut Context) {
        self.futures.borrow_mut().push(future);
    }

    fn run_jobs_async<'a, 'ctx, 'fut>(
        &'a self,
        context: &'ctx mut Context,
    ) -> Pin<Box<dyn Future<Output = ()> + 'fut>>
    where
        'a: 'fut,
        'ctx: 'fut,
    {
        Box::pin(async {
            loop {
                self.run_jobs(context);
                let mut items_removed = false;
                let mut futures = self.futures.borrow_mut();
                for i in (0..futures.len()).rev() {
                    let mut future_ready = false;
                    if let Ready(job) = futures::poll!(futures[i].as_mut()) {
                        self.jobs.borrow_mut().push_back(job);
                        future_ready = true;
                    }
                    if future_ready {
                        drop(futures.swap_remove(i));
                        items_removed = true;
                    }
                }
                if !items_removed {
                    break;
                }
            }
        })
    }
}

struct MirvEvents {
    on_record_start: RefCell<Option<JsObject>>,
    on_record_end: RefCell<Option<JsObject>>,
    on_game_event: RefCell<Option<JsObject>>,
    on_c_view_render_setup_view: RefCell<Option<JsObject>>,
    on_client_frame_stage_notify: RefCell<Option<JsObject>>,
    on_add_entity: RefCell<Option<JsObject>>,
    on_remove_entity: RefCell<Option<JsObject>>,
}

impl MirvEvents {
    fn new() -> Self {
        Self {
            on_record_start: RefCell::<Option<JsObject>>::new(None),
            on_record_end: RefCell::<Option<JsObject>>::new(None),
            on_game_event: RefCell::<Option<JsObject>>::new(None),
            on_c_view_render_setup_view: RefCell::<Option<JsObject>>::new(None),
            on_client_frame_stage_notify: RefCell::<Option<JsObject>>::new(None),
            on_add_entity: RefCell::<Option<JsObject>>::new(None),
            on_remove_entity: RefCell::<Option<JsObject>>::new(None),
        }
    }    
}

pub struct AfxHookSource2Rs {
    loader: Rc<AfxSimpleModuleLoader>,
    context: Rc<advancedfx::js::ContextHolder>,
    events: Rc<MirvEvents>
}

pub fn afx_hooks_source_2_rs_ptr_to_ref<'a>(ptr: * mut AfxHookSource2Rs) -> &'a mut AfxHookSource2Rs {
    unsafe{&mut *ptr}
}

#[derive(Trace, Finalize, JsData)]
struct MirvStruct {
    #[unsafe_ignore_trace]
    loader: Rc<AfxSimpleModuleLoader>,
    
    #[unsafe_ignore_trace]
    events: Rc<MirvEvents>,

    #[unsafe_ignore_trace]
    main_campath: Option<JsValue>,
}

fn afx_message(s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        afx_hook_source2_message(c_string.as_ptr());
    }
}

fn afx_warning(s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        afx_hook_source2_warning(c_string.as_ptr());
    }
}

fn afx_exec(s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        afx_hook_source2_exec(c_string.as_ptr());
    }
}

fn afx_enable_on_record_start(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_record_start(value);        
    }
}

fn afx_enable_on_record_end(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_record_end(value);        
    }
}

fn afx_enable_on_game_event(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_game_event(value);        
    }
}

fn afx_enable_on_c_view_render_setup_view(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_c_view_render_setup_view(value);        
    }
}

fn afx_enable_on_client_frame_stage_notify(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_client_frame_stage_notify(value);        
    }
}

fn afx_make_handle(entry_index: i32, serial_number: i32) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_make_handle(entry_index, serial_number);
    }
    return result;
}

fn afx_is_handle_valid(handle: i32) -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_is_handle_valid(handle);
    }
    return result;
}

fn afx_get_handle_entry_index(handle: i32) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_handle_entry_index(handle);
    }
    return result;
}

fn afx_get_handle_serial_number(handle: i32) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_handle_serial_number(handle);
    }
    return result;
}

fn afx_get_highest_entity_index() -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_highest_entity_index();
    }
    return result;
}

fn afx_get_entity_ref_from_index(index: i32) -> * mut AfxEntityRef {
    let result: * mut AfxEntityRef;
    unsafe {
        result = afx_hook_source2_get_entity_ref_from_index(index);
    }
    return result;
}


fn afx_get_entity_ref_from_split_screen_player(index: i32) -> * mut AfxEntityRef {
    let result: * mut AfxEntityRef;
    unsafe {
        result = afx_hook_source2_get_entity_ref_from_split_screen_player(index);
    }
    return result;
}



fn afx_add_ref_entity_ref(p_ref: * mut AfxEntityRef) {
    unsafe {
        afx_hook_source2_add_ref_entity_ref(p_ref);
    }
}

fn afx_release_entity_ref( p_ref: * mut AfxEntityRef) {
    unsafe {
        afx_hook_source2_release_entity_ref(p_ref);
    }
}

fn afx_enable_on_add_entity(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_add_entity(value);        
    }
}

fn afx_enable_on_remove_entity(value: bool) {
    unsafe {
        afx_hook_source2_enable_on_remove_entity(value);        
    }
}

fn afx_get_entity_ref_is_valid(p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_get_entity_ref_is_valid(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_name( p_ref: * mut AfxEntityRef) -> String {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_name(p_ref);
    }
    return unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string() ;
}

fn afx_get_entity_ref_debug_name(p_ref: * mut AfxEntityRef) -> Option<String> {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_debug_name(p_ref);
    }
    if result.is_null() {
         return None;
    }
    return Some(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string());
}

fn afx_get_entity_ref_player_name(p_ref: * mut AfxEntityRef) -> Option<String> {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_player_name(p_ref);
    }
    if result.is_null() {
         return None;
    }
    return Some(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string());
}


fn afx_get_entity_ref_sanitized_player_name(p_ref: * mut AfxEntityRef) -> Option<String> {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_sanitized_player_name(p_ref);
    }
    if result.is_null() {
         return None;
    }
    return Some(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string());
}

fn afx_get_entity_ref_class_name( p_ref: * mut AfxEntityRef) -> String {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_class_name(p_ref);
    }
    return unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string();
}

fn afx_get_entity_ref_client_class_name( p_ref: * mut AfxEntityRef) -> Option<String> {
    let result: *const c_char;
    unsafe {
        result = afx_hook_source2_get_entity_ref_client_class_name(p_ref);
    }
    if result.is_null() {
        return None;
    }    
    return Some(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string());
}


fn afx_get_entity_ref_is_player_pawn(p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_get_entity_ref_is_player_pawn(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_player_pawn_handle(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_player_pawn_handle(p_ref);
    }
    return result;
}


fn afx_get_entity_ref_is_player_controller(p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_get_entity_ref_is_player_controller(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_player_controller_handle(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_player_controller_handle(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_health(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_health(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_team(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_team(p_ref);
    }
    return result;
}


fn afx_get_entity_ref_origin(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        afx_hook_source2_get_entity_ref_origin(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_render_eye_origin(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        afx_hook_source2_get_entity_ref_render_eye_origin(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_render_eye_angles(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        afx_hook_source2_get_entity_ref_render_eye_angles(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_view_entity_handle(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_view_entity_handle(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_active_weapon_handle(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_active_weapon_handle(p_ref);
    }
    return result;
}


fn afx_get_entity_ref_observer_mode(p_ref: * mut AfxEntityRef) -> u8 {
    let result: u8;
    unsafe {
        result = afx_hook_source2_get_entity_ref_observer_mode(p_ref);
    }
    return result;
}


fn afx_get_entity_ref_observer_target_handle(p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = afx_hook_source2_get_entity_ref_observer_target_handle(p_ref);
    }
    return result;
}




fn afx_get_entity_ref_steam_id(p_ref: * mut AfxEntityRef) -> u64 {
    let result: u64;
    unsafe {
        result = afx_hook_source2_get_entity_ref_steam_id(p_ref);
    }
    return result;
}


fn afx_is_playing_demo() -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_is_playing_demo();
    }
    return result;
}

fn afx_is_demo_paused() -> bool {
    let result: bool;
    unsafe {
        result = afx_hook_source2_is_demo_paused();
    }
    return result;
}


#[derive(Debug, Trace, Finalize)]
pub struct AfxLogger;

impl Logger for AfxLogger {
    #[inline]
    fn log(&self, msg: String, state: &ConsoleState, _context: &mut Context) -> JsResult<()> {
        let mut s = String::new();
        let indent = state.indent();
        use std::fmt::Write as _;
        writeln!(&mut s, "{msg:>indent$}").map_err(JsError::from_rust)?;
        afx_message(s);
        Ok(())
    }

    #[inline]
    fn info(&self, msg: String, state: &ConsoleState, context: &mut Context) -> JsResult<()> {
        self.log(msg, state, context)
    }

    #[inline]
    fn warn(&self, msg: String, state: &ConsoleState, _context: &mut Context) -> JsResult<()> {
        let mut s = String::new();
        let indent = state.indent();
        use std::fmt::Write as _;
        writeln!(&mut s, "{msg:>indent$}").map_err(JsError::from_rust)?;
        afx_warning(s);
        Ok(())
    }

    #[inline]
    fn error(&self, msg: String, state: &ConsoleState, context: &mut Context) -> JsResult<()> {
        self.warn(msg, state, context)
    }
}

fn afx_print_error(error: &JsError, indent: usize, context: &mut Context) -> JsResult<()> {
    use std::fmt::Write;
    if let Ok(native_error) = error.try_native(context) {
        let mut s = String::new();
        writeln!(&mut s, "{:>indent$}Uncaught error `{}`","",error).map_err(JsError::from_rust)?;
        afx_warning(s);
        match &native_error.kind {
            JsNativeErrorKind::Aggregate(errors) => {
                for sub_error in errors {
                    afx_print_error(&sub_error, indent+1, context)?;
                }
            }
            _ => {
                if let Some(cause) = native_error.cause() {
                    afx_print_error(&cause, indent+1, context)?;
                }
            }
        }        
    } else {
        let mut s = String::new();
        writeln!(&mut s, "{:>indent$}Uncaught error `{}`","",error).map_err(JsError::from_rust)?;
        afx_warning(s);
    }
    
    Ok(())
}

fn afx_on_error(error: &JsError, context: &mut Context) -> JsResult<()> {

    afx_print_error(error, 0, context)?;

    let dump = advancedfx::js::errors::get_stack_dump(context);

    if 0 < dump.len() {
        afx_message("Stack trance:\n".to_string());
        afx_message(dump);
    }

    Ok(())
}

fn mirv_trace(_this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    let trace = context.stack_trace().map(|frame| JsValue::String(frame.code_block().name().clone()))
    .collect::<Vec<_>>()
    .into_iter();

    Ok(JsValue::Object(JsObject::from(JsArray::from_iter(trace,context))))
}

fn mirv_message(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    for x in args {
        match x.to_string(context) {
            Ok(s) => {
                afx_message(s.to_std_string_escaped());
            }
            Err(e) => {
                return Err(e);
            }
        }
    }
    return Ok(JsValue::Undefined)
}

fn mirv_warning(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    for x in args {
        match x.to_string(context) {
            Ok(s) => {
                afx_warning(s.to_std_string_escaped());
            }
            Err(e) => {
                return Err(e);
            }
        }
    }
    return Ok(JsValue::Undefined)
}

fn mirv_exec(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    for x in args {
        match x.to_string(context) {
            Ok(s) => {
                afx_exec(s.to_std_string_escaped());
            }
            Err(e) => {
                return Err(e);
            }
        }
    }
    return Ok(JsValue::Undefined)
}

fn mirv_set_on_record_start(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_record_start.replace(None);
                        afx_enable_on_record_start(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_record_start.replace(Some(object.clone()));
                            afx_enable_on_record_start(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_record_start(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_record_start.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_set_on_record_end(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_record_end.replace(None);
                        afx_enable_on_record_end(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_record_end.replace(Some(object.clone()));
                            afx_enable_on_record_end(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_record_end(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_record_end.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_set_on_game_event(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_game_event.replace(None);
                        afx_enable_on_game_event(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_game_event.replace(Some(object.clone()));
                            afx_enable_on_game_event(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_game_event(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_game_event.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_set_on_c_view_render_setup_view(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_c_view_render_setup_view.replace(None);
                        afx_enable_on_c_view_render_setup_view(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_c_view_render_setup_view.replace(Some(object.clone()));
                            afx_enable_on_c_view_render_setup_view(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_c_view_render_setup_view(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_c_view_render_setup_view.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_set_on_client_frame_stage_notify(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_client_frame_stage_notify.replace(None);
                        afx_enable_on_client_frame_stage_notify(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_client_frame_stage_notify.replace(Some(object.clone()));
                            afx_enable_on_client_frame_stage_notify(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_client_frame_stage_notify(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_client_frame_stage_notify.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_run_jobs(_this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    context.run_jobs();
    Ok(JsValue::Undefined)
}

fn mirv_run_jobs_async(_this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    let task = context.run_jobs_async();
    futures::executor::block_on(task);    
    Ok(JsValue::Undefined)
}

#[derive(Trace, Finalize, JsData)]
struct MirvWsResult {
    #[unsafe_ignore_trace]
    state: RefCell<Option<Vec<u8>>>,
}

impl MirvWsResult {
    #[must_use]
    fn new() -> Self {
        let state = RefCell::<Option<Vec<u8>>>::new(None);
        Self { state }
    }

    fn create(
        _this: &JsValue,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsObject {

       return ObjectInitializer::with_native_data::<MirvWsResult>(MirvWsResult::new(), context)
            .function(
                NativeFunction::from_fn_ptr(MirvWsResult::consume),
                js_string!("consume"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvWsResult::clone),
                js_string!("clone"),
                0,
            )
            .build();
    }

    fn consume(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvWsResult>() {
                if let Some(bin) = mirv.state.replace(None)  {
                        match JsArrayBuffer::from_byte_block(bin,context) {
                            Ok(buffer) => {
                                return Ok(JsValue::from(buffer));
                            }
                            Err(e) => {
                                return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),e.to_string(),context).into());
                            }
                        }
                }
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn clone(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvWsResult>() {
                if let Some(bin) = &mut *mirv.state.borrow_mut()  {
                        match JsArrayBuffer::from_byte_block(bin.clone(),context) {
                            Ok(buffer) => {
                                return Ok(JsValue::from(buffer));
                            }
                            Err(e) => {
                                return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),e.to_string(),context).into());
                            }
                        }
                }
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }
}

macro_rules! afx_make_async_error {
    (StandardError: $macro: expr, $context_ptr_option: ident) => {
        if let Some(context_ptr) = $context_ptr_option {
            if let Some(context) = context_ptr.get() {
                $macro(context)
            } else {
                advancedfx::js::errors::error_resolve_wrapper_failed()
            }
        } else {
            advancedfx::js::errors::error_resolve_wrapper_failed()
        }        
    };
    (CustomError: $js_native_error: expr, $message: expr, $context_ptr_option: ident) => {
        if let Some(context_ptr) = $context_ptr_option {
            if let Some(context) = context_ptr.get() {
                advancedfx::js::errors::make_error!($js_native_error, $message, context)
            } else {
                advancedfx::js::errors::error_resolve_wrapper_failed()
            }            
        } else {
            advancedfx::js::errors::error_resolve_wrapper_failed()
        }
    };
}

#[derive(Trace, Finalize, JsData)]
struct MirvWsWrite {
    #[unsafe_ignore_trace]
    state: RefCell<Option<SplitSink<WebSocketStream<ConnectStream>, Message>>>,
}

impl MirvWsWrite {
    #[must_use]
    fn new() -> Self {
        let state = RefCell::<Option<SplitSink<WebSocketStream<ConnectStream>, Message>>>::new(None);
        Self { state }
    }

    fn create(
        _this: &JsValue,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsObject {

        return ObjectInitializer::with_native_data::<MirvWsWrite>(MirvWsWrite::new(), context)
            .function(
                NativeFunction::from_async_fn(MirvWsWrite::close),
                js_string!("close"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvWsWrite::drop),
                js_string!("drop"),
                0,
            )            
            .function(
                NativeFunction::from_async_fn(MirvWsWrite::feed),
                js_string!("feed"),
                0,
            )
            .function(
                NativeFunction::from_async_fn(MirvWsWrite::flush),
                js_string!("flush"),
                0,
            )
            .function(
                NativeFunction::from_async_fn(MirvWsWrite::send),
                js_string!("send"),
                0,
            )
            .build();
    }


    fn close(this: &JsValue, _args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            if let Err(e) = ws_write.close().await {
                                return Err(afx_make_async_error!(CustomError: JsNativeError::error(), e.to_string(), context_ptr_option).into());
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } else {
                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_async_conflict, context_ptr_option).into());
                    }
                }
            }
            Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_type, context_ptr_option).into())
        }
    }

    fn drop(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                if let Err(_) = mirv_ws_write.state.try_borrow_mut() {
                    return Err(advancedfx::js::errors::error_async_conflict(context).into());
                }
                drop(mirv_ws_write.state.replace(None));
                return Ok(JsValue::Undefined);         
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn feed(this: &JsValue, args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let args_clone: Vec<_> = args.iter().cloned().collect();
        let this_clone = (*this).clone();
        let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            for x in args_clone {
                                match x {
                                    JsValue::String(js_string) => {
                                        if let Err(e) = ws_write.feed(Message::text(js_string.to_std_string_escaped())).await {
                                            return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(),context_ptr_option).into());
                                        }
                                    }
                                    JsValue::Object(js_object) => {
                                        if let Ok(array_buffer) = JsArrayBuffer::from_object(js_object.clone()) {
                                            if let Ok(data) = array_buffer.detach(&JsValue::undefined()) {
                                                if let Err(e) = ws_write.feed(Message::binary(data)).await {
                                                    return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(),context_ptr_option).into());
                                                }
                                            }
                                        }
                                    }
                                    _ => {
                                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_arguments, context_ptr_option).into());
                                    }
                                }
                            }
                            return Ok(JsValue::Undefined)
                        }
                    } else {
                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_async_conflict, context_ptr_option).into());
                    }
                }
            }
            Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_type, context_ptr_option).into())
        }
    }

    fn flush(this: &JsValue, _args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            if let Err(e) = ws_write.flush().await {
                                return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(),context_ptr_option).into());
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } else {
                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_async_conflict, context_ptr_option).into());
                    }                        
                }
            }
            Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_type, context_ptr_option).into())
        }
    }

    fn send(this: &JsValue, args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let args_clone: Vec<_> = args.iter().cloned().collect();
        let this_clone = (*this).clone();
        let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            for x in args_clone {
                                match x {
                                    JsValue::String(js_string) => {
                                        if let Err(e) = ws_write.send(Message::text(js_string.to_std_string_escaped())).await {
                                            return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(), context_ptr_option).into());
                                        }
                                    }
                                    JsValue::Object(js_object) => {
                                        if let Ok(array_buffer) = JsArrayBuffer::from_object(js_object.clone()) {
                                            if let Ok(data) = array_buffer.detach(&JsValue::undefined()) {
                                                if let Err(e) = ws_write.send(Message::binary(data)).await {
                                                    return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(), context_ptr_option).into());
                                                }
                                            }
                                        }
                                    }
                                    _ => {
                                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_arguments, context_ptr_option).into());
                                    }
                                }
                            }
                            return Ok(JsValue::Undefined)
                        }
                    } else {
                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_async_conflict, context_ptr_option).into());
                    }
                }
            }
            Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_type, context_ptr_option).into())
        }
    }
}

#[derive(Trace, Finalize, JsData)]
struct MirvWsRead {
    #[unsafe_ignore_trace]
    state: RefCell<Option<SplitStream<WebSocketStream<ConnectStream>>>>,
}

impl MirvWsRead {
    #[must_use]
    fn new() -> Self {
        let state = RefCell::<Option<SplitStream<WebSocketStream<ConnectStream>>>>::new(None);
        Self { state }
    }

    fn create(
        _this: &JsValue,
        _args: &[JsValue],
        context: &mut Context,
    ) -> JsObject {

        return ObjectInitializer::with_native_data::<MirvWsRead>(MirvWsRead::new(), context)
            .function(
                NativeFunction::from_fn_ptr(MirvWsRead::drop),
                js_string!("drop"),
                0,
            )     
            .function(
                NativeFunction::from_async_fn(MirvWsRead::next),
                js_string!("next"),
                0,
            )
            .build();
    }

    fn drop(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv_ws_read) = object.downcast_ref::<MirvWsRead>() {
                if let Err(_) = mirv_ws_read.state.try_borrow_mut() {
                    return Err(advancedfx::js::errors::error_async_conflict(context).into());
                }
                drop(mirv_ws_read.state.replace(None));
                return Ok(JsValue::Undefined);         
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }    
    
    fn next(this: &JsValue, args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        let mirv_result = MirvWsResult::create(this,args,context);
        let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_read) = object.downcast_ref::<MirvWsRead>() {
                    if let Ok(mut borrow_mut) = mirv_ws_read.state.try_borrow_mut() {
                        if let Some(ws_read) = &mut *borrow_mut  {
                            match ws_read.next().await {
                                Some(result) => {
                                    match result {
                                        Ok(message) => {
                                            match message {
                                                Message::Text(text) => {
                                                    return Ok(JsValue::String(js_string!(text)));
                                                }
                                                Message::Binary(bin) => {
                                                    drop(mirv_result.downcast_ref::<MirvWsResult>().unwrap().state.replace(Some(bin)));
                                                    return Ok(JsValue::Object(mirv_result));
                                                }
                                                Message::Close(_) => {
                                                    return Ok(JsValue::null());
                                                }
                                                _ => {
                                                    return Err(afx_make_async_error!(CustomError: JsNativeError::error(),"Unexpected websocket message", context_ptr_option).into());
                                                }
                                            }
                                        }
                                        Err(e) => {
                                            return Err(afx_make_async_error!(CustomError: JsNativeError::error(),e.to_string(), context_ptr_option).into());
                                        }
                                    }
                                }
                                None => {
                                    return Ok(JsValue::null());
                                }
                            }
                        }
                    } else {
                        return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_async_conflict, context_ptr_option).into());
                    }
                }
            }
            Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_type, context_ptr_option).into())
        }
    }    
}

fn mirv_connect_async(this: &JsValue, args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {

    let args_clone: Vec<_> = args.iter().cloned().collect();

    let mirv_ws_read_object = MirvWsRead::create(this,args,context);
    let mirv_ws_write_object = MirvWsWrite::create(this,args,context);

    let in_out_object = ObjectInitializer::new(context)
        .property(js_string!("in"), mirv_ws_read_object.clone(), Attribute::all())
        .property(js_string!("out"), mirv_ws_write_object.clone(), Attribute::all())
        .build();

    let context_ptr_option: Option<advancedfx::js::ContextMutRef> = context.get_data::<advancedfx::js::ContextMutRef>().map(|x| x.clone());

    async move {
        if 0 < args_clone.len() {
            if let JsValue::String(js_string) = &args_clone[0] {
                let connect_addr = js_string.to_std_string_escaped();
                let result = async_tungstenite::async_std::connect_async(connect_addr).await;
                match result {
                    Ok((ws,_)) => {
                        let (ws_write,ws_read) = ws.split();
                        drop(mirv_ws_read_object.downcast_ref::<MirvWsRead>().unwrap().state.replace(Some(ws_read)));
                        drop(mirv_ws_write_object.downcast_ref::<MirvWsWrite>().unwrap().state.replace(Some(ws_write)));
                        return Ok(JsValue::Object(in_out_object));
                    }
                    Err(e) => {
                        use std::fmt::Write as _;
                        let mut s = String::new();
                        write!(&mut s, "{e} in ").unwrap();
                        return Err(afx_make_async_error!(CustomError: JsNativeError::error(),s, context_ptr_option).into());
                    }
                }
            } else { return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_arguments, context_ptr_option).into()); }
        } else { return Err(afx_make_async_error!(StandardError: advancedfx::js::errors::error_arguments, context_ptr_option).into()); }
    }
}

fn mirv_make_handle(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if args.len() == 2 {
        if let Some(entry_index) = args[0].as_number() {
            if let Some(serial_number) = args[1].as_number() {
                return Ok(JsValue::Integer(afx_make_handle(entry_index as i32,serial_number as i32)));
            }
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}

fn mirv_is_handle_valid(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if args.len() == 1 {
        if let Some(handle) = args[0].as_number() {
            return Ok(JsValue::Boolean(afx_is_handle_valid(handle as i32)));
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}

fn mirv_get_handle_entry_index(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if args.len() == 1 {
        if let Some(handle) = args[0].as_number() {
            return Ok(JsValue::Integer(afx_get_handle_entry_index(handle as i32)));
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}

fn mirv_get_handle_serial_number(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if args.len() == 1 {
        if let Some(handle) = args[0].as_number() {
            return Ok(JsValue::Integer(afx_get_handle_serial_number(handle as i32)));
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}

fn mirv_get_highest_entity_index(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    return Ok(JsValue::Integer(afx_get_highest_entity_index()));
}

#[derive(Trace, JsData)]
struct MirvEntityRef {
    #[unsafe_ignore_trace]
    entity_ref: * mut AfxEntityRef
}

impl Finalize for MirvEntityRef {
    // Provided method
    fn finalize(&self) {
        afx_release_entity_ref(self.entity_ref);
    }
}

impl MirvEntityRef {
    #[must_use]
    fn new(afx_entity_ref: * mut AfxEntityRef) -> Self {
        
        Self {
            entity_ref: afx_entity_ref
        }
    }

    fn create(afx_entity_ref: * mut AfxEntityRef,
        context: &mut Context
    ) -> JsObject {

       return ObjectInitializer::with_native_data::<MirvEntityRef>(MirvEntityRef::new(afx_entity_ref), context)
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::is_valid),
                js_string!("isValid"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_name),
                js_string!("getName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_debug_name),
                js_string!("getDebugName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_class_name),
                js_string!("getClassName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_client_class_name),
                js_string!("getClientClassName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::is_player_pawn),
                js_string!("isPlayerPawn"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_player_pawn_handle),
                js_string!("getPlayerPawnHandle"),
                0,
            )  
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::is_player_controller),
                js_string!("isPlayerController"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_player_controller_handle),
                js_string!("getPlayerControllerHandle"),
                0,
            )             
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_health),
                js_string!("getHealth"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_team),
                js_string!("getTeam"),
                0,
            )                                  
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_origin),
                js_string!("getOrigin"),
                0,
            )             
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_render_eye_origin),
                js_string!("getRenderEyeOrigin"),
                0,
            )             
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_render_eye_angles),
                js_string!("getRenderEyeAngles"),
                0,
            )     
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_view_entity_handle),
                js_string!("getViewEntityHandle"),
                0,
            )  
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_active_weapon_handle),
                js_string!("getActiveWeaponHandle"),
                0,
            )  
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_player_name),
                js_string!("getPlayerName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_steam_id),
                js_string!("getSteamId"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_sanitized_player_name),
                js_string!("getSanitizedPlayerName"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_observer_mode),
                js_string!("getObserverMode"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_observer_target_handle),
                js_string!("getObserverTargetHandle"),
                0,
            )
            .function(
                NativeFunction::from_fn_ptr(MirvEntityRef::get_attachment),
                js_string!("getAttachment"),
                0,
            )
            .build();
    }

    fn is_valid(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_valid(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::String(js_string!(afx_get_entity_ref_name(mirv.entity_ref))));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_debug_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                if let Some(str) = afx_get_entity_ref_debug_name(mirv.entity_ref) {
                    return Ok(JsValue::String(js_string!(str)));
                }
                return Ok(JsValue::null());
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_player_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                if let Some(str) = afx_get_entity_ref_player_name(mirv.entity_ref) {
                    return Ok(JsValue::String(js_string!(str)));
                }
                return Ok(JsValue::null());
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }   
    
    fn get_sanitized_player_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                if let Some(str) = afx_get_entity_ref_sanitized_player_name(mirv.entity_ref) {
                    return Ok(JsValue::String(js_string!(str)));
                }
                return Ok(JsValue::null());
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }  

    fn get_class_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::String(js_string!(afx_get_entity_ref_class_name(mirv.entity_ref))));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }
    
    fn get_client_class_name(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                if let Some(str) = afx_get_entity_ref_client_class_name(mirv.entity_ref) {
                    return Ok(JsValue::String(js_string!(str)));
                }
                return Ok(JsValue::null());
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn is_player_pawn(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_player_pawn(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_player_pawn_handle(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_player_pawn_handle(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn is_player_controller(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_player_controller(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_player_controller_handle(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_player_controller_handle(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }
    
    fn get_health(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_health(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_team(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_team(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_origin(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_origin(mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_render_eye_origin(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_render_eye_origin(mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    } 

    fn get_render_eye_angles(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_render_eye_angles(mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_view_entity_handle(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_view_entity_handle(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_active_weapon_handle(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_active_weapon_handle(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_steam_id(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::from(JsBigInt::from(afx_get_entity_ref_steam_id(mirv.entity_ref))));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_observer_mode(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::from(afx_get_entity_ref_observer_mode(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }    
    
    fn get_observer_target_handle(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_observer_target_handle(mirv.entity_ref)));
            }
        }
        Err(advancedfx::js::errors::error_type(context).into())
    }

    fn get_attachment(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        let object = this.as_object().ok_or(advancedfx::js::errors::error_type(context))?;
        let mirv = object.downcast_ref::<MirvEntityRef>().ok_or(advancedfx::js::errors::error_type(context))?;

        if 1 != args.len() { return Err(advancedfx::js::errors::error_arguments(context).into()) };
        let arg0 = args[0].as_string().ok_or(advancedfx::js::errors::error_arguments(context))?;

        let attachment_name = std::ffi::CString::new(arg0.to_std_string().unwrap()).unwrap();
        let mut position = advancedfx::math::Vector3::new(0.0, 0.0, 0.0);
        let mut angles = advancedfx::math::Quaternion::new(0.0, 0.0, 0.0, 0.0);

        let result = unsafe { afx_hook_source2_get_entity_ref_attachment(mirv.entity_ref, attachment_name.as_ptr(), &mut position, &mut angles) };

        match result {
            true => {
                let pos_js = advancedfx::js::math::Vector3::new(position).to_js_object(context).unwrap();
                let angs_js = advancedfx::js::math::Quaternion::new(angles).to_js_object(context).unwrap();
                let out = ObjectInitializer::new(context)
                    .property(js_string!("position"), pos_js, Attribute::all())
                    .property(js_string!("angles"), angs_js, Attribute::all())
                    .build();

                Ok(JsValue::from(out))
            },
            false => Ok(JsValue::Null)
        }
    }
}

fn mirv_get_entity_ref_from_index(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if 1 == args.len() {
        if let Some(index) = args[0].as_number() {
            let entity_ref: * mut AfxEntityRef;
            entity_ref = afx_get_entity_ref_from_index(index as i32);
            if entity_ref.is_null() {
                return Ok(JsValue::null());
            }
            return Ok(JsValue::Object(MirvEntityRef::create(entity_ref,context)));
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}


fn mirv_get_entity_ref_from_split_screen_player(_this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if 1 == args.len() {
        if let Some(index) = args[0].as_number() {
            let entity_ref: * mut AfxEntityRef;
            entity_ref = afx_get_entity_ref_from_split_screen_player(index as i32);
            if entity_ref.is_null() {
                return Ok(JsValue::null());
            }
            return Ok(JsValue::Object(MirvEntityRef::create(entity_ref,context)));
        }
    }
    return Err(advancedfx::js::errors::error_arguments(context).into());
}

fn mirv_set_on_add_entity(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_add_entity.replace(None);
                        afx_enable_on_add_entity(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_add_entity.replace(Some(object.clone()));
                            afx_enable_on_add_entity(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_add_entity(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_add_entity.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}


fn mirv_set_on_remove_entity(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_remove_entity.replace(None);
                        afx_enable_on_remove_entity(false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_remove_entity.replace(Some(object.clone()));
                            afx_enable_on_remove_entity(true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_on_remove_entity(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            match & *mirv.events.on_remove_entity.borrow() {
                None => {
                    return Ok(JsValue::Undefined);
                }
                Some(js_object) => {
                    return Ok(JsValue::Object(js_object.clone()));
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn afx_load_module(specifier: &JsString, afx_loader: Rc<AfxSimpleModuleLoader>, context: &mut Context) -> Result<JsPromise,JsError> {

    let referrer = Referrer::Realm(context.realm().clone());  

    if let Ok(path) = resolve_module_specifier(None, &specifier, referrer.path(), context) {
        if let Some(module) = afx_loader.get(&path) {
             return Ok(module.load_link_evaluate(context));
        }
        match boa_engine::Source::from_filepath(&path) {
            Ok(js_source) => {
                match Module::parse(js_source, None, context) {
                    Ok(module) => {
                        afx_loader.insert(path, module.clone());
                        return Ok(module.load_link_evaluate(context));
                    }
                    Err(err) => {
                        return Err(advancedfx::js::errors::make_error!(JsNativeError::syntax(),format!("Error parsing module `{}`",path.display()),context)
                        .with_cause(err).into());        
                    }
                }
            }
            Err(err) => {
                return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),format!("could not open file `{}`",path.display()),context)
                .with_cause(JsError::from_opaque(js_string!(err.to_string()).into())    ).into());
            }
        }
    }

    let err_path = specifier.to_std_string_lossy();
    return Err(advancedfx::js::errors::make_error!(JsNativeError::error(), format!("could not resolve path `{err_path}`"), context).into());
}

fn afx_load(file_path: &JsString, afx_loader: Rc<AfxSimpleModuleLoader>, context: &mut Context) -> Result<JsPromise,JsError> {

    if file_path.to_std_string_lossy().ends_with("mjs") {
        return afx_load_module(file_path,afx_loader,context);
    }

    let referrer = Referrer::Realm(context.realm().clone());  

    if let Ok(path) = resolve_module_specifier(None, &file_path, referrer.path(), context) {
        match boa_engine::Source::from_filepath(&path) {
            Ok(js_source) => {
                match context.eval(js_source) {
                    Ok(res) => {
                        return Ok(JsPromise::resolve(res,context))
                    }
                    Err(err) => {
                        return Err(advancedfx::js::errors::make_error!(JsNativeError::syntax(),format!("Error evaluating script `{}`",path.display()),context)
                        .with_cause(err).into());
                    }
                }                
            }
            Err(err) => {
                return Err(advancedfx::js::errors::make_error!(JsNativeError::error(),format!("could not open file `{}`",path.display()),context)
                .with_cause(JsError::from_opaque(js_string!(err.to_string()).into())    ).into());
            }
        }
    }
    let err_path = file_path.to_std_string_lossy();
    return Err(advancedfx::js::errors::make_error!(JsNativeError::error(), format!("could not resolve path `{err_path}`"), context).into());  
}

fn mirv_load(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 1 == args.len() {
                if let Some(js_file_path) = args[0].as_string() {
                    match afx_load(&js_file_path, mirv.loader.clone(), context) {
                        Ok(js_promise) => {
                            return Ok(JsValue::Object(JsObject::from(js_promise)));
                        }
                        Err(e) => {
                            return Err(e);
                        }
                    }
                }
            }
            return Err(advancedfx::js::errors::error_arguments(context).into());
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())    
}

fn mirv_is_playing_demo(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
   return Ok(JsValue::Boolean(afx_is_playing_demo()));
}

fn mirv_is_demo_paused(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
   return Ok(JsValue::Boolean(afx_is_demo_paused()));
}

fn mirv_get_main_campath(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mut mirv) = object.downcast_mut::<MirvStruct>() {
            if let Some(value) = &mirv.main_campath {
                return Ok(value.clone());
            } else {
                match advancedfx::js::campath::Campath::from_data(advancedfx::js::campath::Campath::new(advancedfx::campath::Campath::new_shared(unsafe{
                    afx_hook_source2_get_main_campath()
                })), context) {
                    Ok(result_object) => {
                        mirv.main_campath = Some(JsValue::Object(result_object.clone()));
                        return Ok(JsValue::Object(result_object.clone()));
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
        }
    }
    Err(advancedfx::js::errors::error_type(context).into())
}

fn mirv_get_demo_tick(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    let mut out_tick: i32 = 0;

    let result = unsafe { afx_hook_source2_get_demo_tick(&mut out_tick) };

    match result {
        true => Ok(JsValue::Integer(out_tick)),
        false => Ok(JsValue::Undefined)
    }
}

fn mirv_get_demo_time(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    let mut out_time: f64 = 0.0;

    let result = unsafe { afx_hook_source2_get_demo_time(&mut out_time) };

    match result {
        true => Ok(JsValue::Rational(out_time)),
        false => Ok(JsValue::Undefined)
    }
}

fn mirv_get_cur_time(_this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    let mut out_time: f64 = 0.0;

    unsafe { afx_hook_source2_get_cur_time(&mut out_time) };

    Ok(JsValue::Rational(out_time))
}

impl AfxHookSource2Rs {
    pub fn new() -> Self {

        let loader = Rc::new(AfxSimpleModuleLoader::new());

        let mut context = ContextBuilder::default()
            .job_queue(AsyncJobQueue::new().into())
            .module_loader(loader.clone())
            .build().unwrap();

        let console = Console::init_with_logger(&mut context, AfxLogger);
        context
            .register_global_property(Console::NAME, console, Attribute::all())
            .expect("the console builtin shouldn't exist"); 

        advancedfx::js::math::Vector3::add_to_context(&mut context);
        advancedfx::js::math::QEulerAngles::add_to_context(&mut context);
        advancedfx::js::math::QREulerAngles::add_to_context(&mut context);
        advancedfx::js::math::Quaternion::add_to_context(&mut context);

        advancedfx::js::campath::Value::add_to_context(&mut context);
        advancedfx::js::campath::Iterator::add_to_context(&mut context);
        advancedfx::js::campath::Campath::add_to_context(&mut context);

        ConCommandsArgs::add_to_context(&mut context);
        ConCommandBox::add_to_context(&mut context);

        let events = Rc::<MirvEvents>::new(MirvEvents::new());

        let mirv = MirvStruct {
            loader: loader.clone(),
            events: Rc::clone(&events),
            main_campath: None
        };

        let fn_mirv_set_on_record_start = NativeFunction::from_fn_ptr(mirv_set_on_record_start).to_js_function(context.realm());
        let fn_mirv_get_on_record_start = NativeFunction::from_fn_ptr(mirv_get_on_record_start).to_js_function(context.realm());
        let fn_mirv_set_on_record_end = NativeFunction::from_fn_ptr(mirv_set_on_record_end).to_js_function(context.realm());
        let fn_mirv_get_on_record_end = NativeFunction::from_fn_ptr(mirv_get_on_record_end).to_js_function(context.realm());
        let fn_mirv_set_on_game_event = NativeFunction::from_fn_ptr(mirv_set_on_game_event).to_js_function(context.realm());
        let fn_mirv_get_on_game_event = NativeFunction::from_fn_ptr(mirv_get_on_game_event).to_js_function(context.realm());
        let fn_mirv_set_on_c_view_render_setup_view = NativeFunction::from_fn_ptr(mirv_set_on_c_view_render_setup_view).to_js_function(context.realm());
        let fn_mirv_get_on_c_view_render_setup_view = NativeFunction::from_fn_ptr(mirv_get_on_c_view_render_setup_view).to_js_function(context.realm());
        let fn_mirv_set_on_client_frame_stage_notify = NativeFunction::from_fn_ptr(mirv_set_on_client_frame_stage_notify).to_js_function(context.realm());
        let fn_mirv_get_on_client_frame_stage_notify = NativeFunction::from_fn_ptr(mirv_get_on_client_frame_stage_notify).to_js_function(context.realm());
        let fn_mirv_set_on_add_entity = NativeFunction::from_fn_ptr(mirv_set_on_add_entity).to_js_function(context.realm());
        let fn_mirv_get_on_add_entity = NativeFunction::from_fn_ptr(mirv_get_on_add_entity).to_js_function(context.realm());
        let fn_mirv_set_on_remove_entity = NativeFunction::from_fn_ptr(mirv_set_on_remove_entity).to_js_function(context.realm());
        let fn_mirv_get_on_remove_entity = NativeFunction::from_fn_ptr(mirv_get_on_remove_entity).to_js_function(context.realm());
      
        let object = ObjectInitializer::with_native_data::<MirvStruct>(mirv, &mut context)
        .function(
            NativeFunction::from_fn_ptr(mirv_get_cur_time),
            js_string!("getCurTime"),
            0,
        )           
        .function(
            NativeFunction::from_fn_ptr(mirv_get_demo_time),
            js_string!("getDemoTime"),
            0,
        )           
        .function(
            NativeFunction::from_fn_ptr(mirv_get_demo_tick),
            js_string!("getDemoTick"),
            0,
        )           
        .function(
            NativeFunction::from_fn_ptr(mirv_get_entity_ref_from_index),
            js_string!("getEntityFromIndex"),
            0,
        )           
        .function(
            NativeFunction::from_fn_ptr(mirv_get_entity_ref_from_split_screen_player),
            js_string!("getEntityFromSplitScreenPlayer"),
            0,
        )           
        .function(
            NativeFunction::from_fn_ptr(mirv_get_handle_entry_index),
            js_string!("getHandleEntryIndex"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_get_handle_serial_number),
            js_string!("getHandleSerialNumber"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_get_highest_entity_index),
            js_string!("getHighestEntityIndex"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_get_main_campath),
            js_string!("getMainCampath"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_is_handle_valid),
            js_string!("isHandleValid"),
            0,
        )        
        .function(
            NativeFunction::from_fn_ptr(mirv_is_playing_demo),
            js_string!("isPlayingDemo"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_is_demo_paused),
            js_string!("isDemoPaused"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_load),
            js_string!("load"),
            0,
        )        
        .function(
            NativeFunction::from_fn_ptr(mirv_make_handle),
            js_string!("makeHandle"),
            0,
        )                 
        .function(
            NativeFunction::from_fn_ptr(mirv_message),
            js_string!("message"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_warning),
            js_string!("warning"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_exec),
            js_string!("exec"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_run_jobs),
            js_string!("run_jobs"),
            0,
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_run_jobs_async),
            js_string!("run_jobs_async"),
            0,
        )
        .function(
            NativeFunction::from_async_fn(mirv_connect_async),
            js_string!("connect_async"),
            0,
        )
        .accessor(
            js_string!("onAddEntity"),
            Some(fn_mirv_get_on_add_entity),
            Some(fn_mirv_set_on_add_entity),
            Attribute::all()
        )
        .accessor(
            js_string!("onRecordStart"),
            Some(fn_mirv_get_on_record_start),
            Some(fn_mirv_set_on_record_start),
            Attribute::all()
        )
        .accessor(
            js_string!("onRecordEnd"),
            Some(fn_mirv_get_on_record_end),
            Some(fn_mirv_set_on_record_end),
            Attribute::all()
        )
        .accessor(
            js_string!("onGameEvent"),
            Some(fn_mirv_get_on_game_event),
            Some(fn_mirv_set_on_game_event),
            Attribute::all()
        )
        .accessor(
            js_string!("onCViewRenderSetupView"),
            Some(fn_mirv_get_on_c_view_render_setup_view),
            Some(fn_mirv_set_on_c_view_render_setup_view),
            Attribute::all()
        )
        .accessor(
            js_string!("onClientFrameStageNotify"),
            Some(fn_mirv_get_on_client_frame_stage_notify),
            Some(fn_mirv_set_on_client_frame_stage_notify),
            Attribute::all()
        )
        .accessor(
            js_string!("onRemoveEntity"),
            Some(fn_mirv_get_on_remove_entity),
            Some(fn_mirv_set_on_remove_entity),
            Attribute::all()
        )
        .function(
            NativeFunction::from_fn_ptr(mirv_trace),
            js_string!("trace"),
            0,
        )        
        .build();

        context
        .register_global_property(js_string!("mirv"), object, Attribute::all())
        .expect("property mirv shouldn't exist");

        Self {
            loader: loader,
            context: advancedfx::js::ContextHolder::new(context),
            events: Rc::clone(&events)
        }
    }    
}

use boa_engine::{Finalize, JsData, Trace};


//fn(_: &JsValue, _: &[JsValue], _: &mut Context) -> JsResult<JsValue>;

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_new() -> * mut AfxHookSource2Rs { 
    Box::into_raw(Box::new(AfxHookSource2Rs::new()))
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_run_jobs(this_ptr: *mut AfxHookSource2Rs) {
    let task = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get().run_jobs_async();
    futures::executor::block_on(task);
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_destroy(this_ptr: *mut AfxHookSource2Rs) {
    if this_ptr.is_null() {
        return;
    }
     drop(unsafe{Box::from_raw(this_ptr)});
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_execute(this_ptr: *mut AfxHookSource2Rs, p_data: *mut u8, len_data: usize) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let js_code = unsafe { std::slice::from_raw_parts(p_data, len_data) };
    match context.eval(boa_engine::Source::from_bytes(js_code)) {
        Ok(res) => {
            if let Ok(js_str) = res.to_string(context) {
                let mut str = js_str.to_std_string_escaped();
                str.push_str("\n");
                afx_message(str);
            }
        }
        Err(e) => {
            let _ = afx_on_error(&e, context);
        }
    };
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_load(this_ptr: *mut AfxHookSource2Rs, file_path: *const c_char) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let str_file_path = unsafe{CStr::from_ptr(file_path)}.to_str().unwrap();
    let js_str_path = js_string!(str_file_path);
    match afx_load(&js_str_path, afx_hooks_source_2_rs_ptr_to_ref(this_ptr).loader.clone(), context) {
        Ok(promise_result) => {

            // push forward the promise:
            let task = context.run_jobs_async();
            futures::executor::block_on(task);

            match promise_result.state() {
                PromiseState::Pending => {
                    let native_error: JsError = advancedfx::js::errors::make_error!(JsNativeError::typ(),"module didn't execute!",context).into();                    
                    let _ = afx_on_error(&native_error, context);
                }
                PromiseState::Fulfilled(v) => {
                    if let Ok(js_str) = v.to_string(context) {
                        let mut str = js_str.to_std_string_escaped();
                        str.push_str("\n");
                        afx_message(str);
                    }
                }
                PromiseState::Rejected(err) => {              
                    let _ = afx_on_error(&JsError::from_opaque(err), context);
                }
            }
            return;
        }
        Err(e) => {
            let _ = afx_on_error(&e, context);
        }                
    }
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_record_start(this_ptr: *mut AfxHookSource2Rs, taker_folder_path: *const c_char) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_record_start.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {

        let mut js_value_take_folder_path: JsValue = JsValue::null();
        if !taker_folder_path.is_null() {
            let str_take_folder_path = unsafe{CStr::from_ptr(taker_folder_path)}.to_str().unwrap();
            js_value_take_folder_path = JsValue::String(js_string!(str_take_folder_path));
        }

        let js_object = ObjectInitializer::new(context)
        .property(js_string!("takeFolder"), js_value_take_folder_path, Attribute::all())
        .build();

        if let Err(e) = event_clone.call(&JsValue::undefined(), &[JsValue::Object(js_object)], context) {              
            let _ = afx_on_error(&e, context);
        }
    }
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_record_end(this_ptr: *mut AfxHookSource2Rs) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_record_end.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        if let Err(e) = event_clone.call(&JsValue::undefined(), &[], context) {
            let _ = afx_on_error(&e, context);
        }
    }
}


#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_game_event(this_ptr: *mut AfxHookSource2Rs, event_name: *const c_char, event_id: i32, json: *const c_char) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_game_event.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        let str_event_name = unsafe{CStr::from_ptr(event_name)}.to_str().unwrap();
        let str_json = unsafe{CStr::from_ptr(json)}.to_str().unwrap();

        let js_object = ObjectInitializer::new(context)
        .property(js_string!("name"), JsValue::String(js_string!(str_event_name)), Attribute::all())
        .property(js_string!("id"), JsValue::Integer(event_id), Attribute::all())
        .property(js_string!("data"), JsValue::String(js_string!(str_json)), Attribute::all())
        .build();

        if let Err(e) = event_clone.call(&JsValue::undefined(), &[JsValue::Object(js_object)], context) {
            let _ = afx_on_error(&e, context);
        }
    }
}

pub struct AfxHookSourceRsView {
    x: c_float,
    y: c_float,
    z: c_float,
    rx: c_float,
    ry: c_float,
    rz: c_float,
    fov: c_float
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_c_view_render_setup_view(this_ptr: *mut AfxHookSource2Rs, cur_time: c_float, abs_time: c_float, last_abs_time: c_float, current_view: &mut AfxHookSourceRsView , game_view: &AfxHookSourceRsView, last_view: &AfxHookSourceRsView, width: i32, height: i32) -> bool {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_c_view_render_setup_view.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);  
    if let Some(event_clone) = event_option_clone {

        let js_object_current_view = ObjectInitializer::new(context)
        .property(js_string!("x"), JsValue::Rational(current_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(current_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(current_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(current_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(current_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(current_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(current_view.fov.into()), Attribute::all())
        .build();
        let js_object_game_view = ObjectInitializer::new(context)
        .property(js_string!("x"), JsValue::Rational(game_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(game_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(game_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(game_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(game_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(game_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(game_view.fov.into()), Attribute::all())
        .build();
        let js_object_last_view = ObjectInitializer::new(context)
        .property(js_string!("x"), JsValue::Rational(last_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(last_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(last_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(last_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(last_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(last_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(last_view.fov.into()), Attribute::all())
        .build();        

        let js_object = ObjectInitializer::new(context)
        .property(js_string!("curTime"), JsValue::Rational(cur_time.into()), Attribute::all())
        .property(js_string!("absTime"), JsValue::Rational(abs_time.into()), Attribute::all())
        .property(js_string!("lastAbsTime"), JsValue::Rational(last_abs_time.into()), Attribute::all())
        .property(js_string!("currentView"), JsValue::Object(js_object_current_view), Attribute::all())
        .property(js_string!("gameView"), JsValue::Object(js_object_game_view), Attribute::all())
        .property(js_string!("lastView"), JsValue::Object(js_object_last_view), Attribute::all())
        .property(js_string!("width"), JsValue::Integer(width), Attribute::all())
        .property(js_string!("height"), JsValue::Integer(height), Attribute::all())
        .build();

        match event_clone.call(&JsValue::undefined(), &[JsValue::Object(js_object)], context) {
            Ok(js_value) => {
                if let JsValue::Object(js_object) = js_value {
                    if let Ok(js_val_x) = js_object.get(js_string!("x"), context) {
                        if let Some(x) = js_val_x.as_number() {
                            current_view.x = x as f32;
                        }
                    }
                    if let Ok(js_val_y) = js_object.get(js_string!("y"), context) {
                        if let Some(y) = js_val_y.as_number() {
                            current_view.y = y as f32;
                        }
                    }
                    if let Ok(js_val_z) = js_object.get(js_string!("z"), context) {
                        if let Some(z) = js_val_z.as_number() {
                            current_view.z = z as f32;
                        }
                    }
                    if let Ok(js_val_rx) = js_object.get(js_string!("rX"), context) {
                        if let Some(rx) = js_val_rx.as_number() {
                            current_view.rx = rx as f32;
                        }
                    }
                    if let Ok(js_val_ry) = js_object.get(js_string!("rY"), context) {
                        if let Some(ry) = js_val_ry.as_number() {
                            current_view.ry = ry as f32;
                        }
                    }
                    if let Ok(js_val_rz) = js_object.get(js_string!("rZ"), context) {
                        if let Some(rz) = js_val_rz.as_number() {
                            current_view.rz = rz as f32;
                        }
                    }
                    if let Ok(js_val_fov) = js_object.get(js_string!("fov"), context) {
                        if let Some(fov) = js_val_fov.as_number() {
                            current_view.fov = fov as f32;
                        }
                    }
                    return true;
                }
            }
            Err(e) =>  {
                let _ = afx_on_error(&e, context);
            }
        }
    }
    return false;
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_client_frame_stage_notify(this_ptr: *mut AfxHookSource2Rs, event_id: i32, is_before: bool) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_client_frame_stage_notify.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        let js_object = ObjectInitializer::new(context)
        .property(js_string!("curStage"), JsValue::Integer(event_id), Attribute::all())
        .property(js_string!("isBefore"), JsValue::Boolean(is_before), Attribute::all())
        .build();

        if let Err(e) = event_clone.call(&JsValue::undefined(), &[JsValue::Object(js_object)], context) {
            let _ = afx_on_error(&e, context);
        }
    }
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_add_entity(this_ptr: *mut AfxHookSource2Rs, p_ref: * mut AfxEntityRef, handle: i32) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_add_entity.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        afx_add_ref_entity_ref(p_ref);
        let entity_ref = MirvEntityRef::create(p_ref, context);
        if let Err(e) = event_clone.call(&JsValue::undefined(), &[JsValue::Object(entity_ref),JsValue::Integer(handle)], context) {
            let _ = afx_on_error(&e, context);
        }
    }
}

#[no_mangle]
pub extern "C" fn afx_hook_source2_rs_on_remove_entity(this_ptr: *mut AfxHookSource2Rs, p_ref: * mut AfxEntityRef, handle: i32) {
    let context = &mut afx_hooks_source_2_rs_ptr_to_ref(this_ptr).context.get();
    let borrowed = afx_hooks_source_2_rs_ptr_to_ref(this_ptr).events.on_remove_entity.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        afx_add_ref_entity_ref(p_ref);
        let entity_ref = MirvEntityRef::create(p_ref, context);
        if let Err(e) = event_clone.call(&JsValue::undefined(), &[JsValue::Object(entity_ref),JsValue::Integer(handle)], context) {
            let _ = afx_on_error(&e, context);
        }
    }
}
