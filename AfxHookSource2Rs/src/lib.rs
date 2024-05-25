use core::ffi::c_char;
use core::ffi::CStr;
use std::ffi::c_float;

use std::error::Error;
use std::{cell::RefCell, collections::VecDeque, future::Future, pin::Pin};
use std::task::Poll::Ready;
use std::rc::Rc;

use futures::SinkExt;
use futures::StreamExt;
use futures::stream::SplitSink;
use futures::stream::SplitStream;

use boa_engine::{
    Context,
    context::{
        ContextBuilder,
    },
    JsResult,
    JsNativeError,
    JsObject,
    JsValue,
    job::{
        NativeJob,
        FutureJob,
        JobQueue,
    },
    js_string,
    native_function::NativeFunction,
    object::ObjectInitializer,
    object::builtins::JsArray,
    object::builtins::JsArrayBuffer,
    property::Attribute,
};

use async_tungstenite::async_std::ConnectStream;
use async_tungstenite::tungstenite::protocol::Message;
use async_tungstenite::WebSocketStream;

pub struct AfxEntityRef {

}

pub struct AfxHookSource2 {
    message: unsafe extern "C" fn(s: *const c_char),
    warning: unsafe extern "C" fn(s: *const c_char),
    exec: unsafe extern "C" fn(s: *const c_char),
    enable_on_game_event: unsafe extern "C" fn(value: bool),
    enable_on_c_view_render_setup_view: unsafe extern "C" fn(value: bool),
    enable_on_client_frame_stage_notify: unsafe extern "C" fn(value: bool),

    make_handle: unsafe extern "C" fn(entry_index: i32, serial_number: i32) -> i32,
    is_handle_valid: unsafe extern "C" fn(handle: i32) -> bool,
    get_handle_entry_index: unsafe extern "C" fn(handle: i32) -> i32,
    get_handle_serial_number: unsafe extern "C" fn(handle: i32) -> i32,
    get_highest_entity_index: unsafe extern "C" fn() -> i32,
    get_entity_ref_from_index: unsafe extern "C" fn(index: i32) -> * mut AfxEntityRef,
    add_ref_entity_ref: unsafe extern "C" fn(p_ref: * mut AfxEntityRef),
    release_entity_ref: unsafe extern "C" fn(p_ref: * mut AfxEntityRef),
    enable_on_add_entity: unsafe extern "C" fn(value: bool),
    enable_on_remove_entity: unsafe extern "C" fn(value: bool),
    
    get_entity_ref_is_valid: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> bool,
    get_entity_ref_name: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> *const c_char,
    
    // can return nullptr to indicate no debug name.
    get_entity_ref_debug_name: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> *const c_char,

    get_entity_ref_class_name: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> *const c_char,

    get_entity_ref_is_player_pawn: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> bool,

    get_entity_ref_player_pawn_handle: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> i32,

    get_entity_ref_is_player_controller: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> bool,

    get_entity_ref_player_controller_handle: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> i32,

    get_entity_ref_health: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> i32,

    get_entity_ref_origin: unsafe extern "C" fn(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32),

    get_entity_ref_render_eye_origin: unsafe extern "C" fn(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32),

    get_entity_ref_render_eye_angles: unsafe extern "C" fn(p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32),

    get_entity_ref_view_entity_handle: unsafe extern "C" fn(p_ref: * mut AfxEntityRef) -> i32,
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
    on_game_event: RefCell<Option<JsObject>>,
    on_c_view_render_setup_view: RefCell<Option<JsObject>>,
    on_client_frame_stage_notify: RefCell<Option<JsObject>>,
    on_add_entity: RefCell<Option<JsObject>>,
    on_remove_entity: RefCell<Option<JsObject>>,
}

impl MirvEvents {
    fn new() -> Self {
        Self {
            on_game_event: RefCell::<Option<JsObject>>::new(None),
            on_c_view_render_setup_view: RefCell::<Option<JsObject>>::new(None),
            on_client_frame_stage_notify: RefCell::<Option<JsObject>>::new(None),
            on_add_entity: RefCell::<Option<JsObject>>::new(None),
            on_remove_entity: RefCell::<Option<JsObject>>::new(None),
        }
    }    
}

pub struct AfxHookSource2Rs {
    iface: * mut AfxHookSource2,
    context: boa_engine::Context,
    events: Rc<MirvEvents>
}

#[derive(Trace, Finalize, JsData)]
struct MirvStruct {
    #[unsafe_ignore_trace]
    iface: * mut AfxHookSource2,
    #[unsafe_ignore_trace]
    events: Rc<MirvEvents>
}

fn afx_message(iface: * mut AfxHookSource2, s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        ((*iface).message)(c_string.as_ptr());
    }
}

fn afx_warning(iface: * mut AfxHookSource2, s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        ((*iface).warning)(c_string.as_ptr());
    }
}

fn afx_exec(iface: * mut AfxHookSource2, s: String) {
    let c_string = std::ffi::CString::new(s).unwrap();
    unsafe {
        ((*iface).exec)(c_string.as_ptr());
    }
}

fn afx_enable_on_game_event(iface: * mut AfxHookSource2, value: bool) {
    unsafe {
        ((*iface).enable_on_game_event)(value);        
    }
}

fn afx_enable_on_c_view_render_setup_view(iface: * mut AfxHookSource2, value: bool) {
    unsafe {
        ((*iface).enable_on_c_view_render_setup_view)(value);        
    }
}

fn afx_enable_on_client_frame_stage_notify(iface: * mut AfxHookSource2, value: bool) {
    unsafe {
        ((*iface).enable_on_client_frame_stage_notify)(value);        
    }
}

fn afx_make_handle(iface: * mut AfxHookSource2, entry_index: i32, serial_number: i32) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).make_handle)(entry_index, serial_number);
    }
    return result;
}

fn afx_is_handle_valid(iface: * mut AfxHookSource2, handle: i32) -> bool {
    let result: bool;
    unsafe {
        result = ((*iface).is_handle_valid)(handle);
    }
    return result;
}

fn afx_get_handle_entry_index(iface: * mut AfxHookSource2, handle: i32) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_handle_entry_index)(handle);
    }
    return result;
}

fn afx_get_handle_serial_number(iface: * mut AfxHookSource2, handle: i32) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_handle_serial_number)(handle);
    }
    return result;
}

fn afx_get_highest_entity_index(iface: * mut AfxHookSource2) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_highest_entity_index)();
    }
    return result;
}

fn afx_get_entity_ref_from_index(iface: * mut AfxHookSource2, index: i32) -> * mut AfxEntityRef {
    let result: * mut AfxEntityRef;
    unsafe {
        result = ((*iface).get_entity_ref_from_index)(index);
    }
    return result;
}



fn afx_add_ref_entity_ref(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) {
    unsafe {
        ((*iface).add_ref_entity_ref)(p_ref);
    }
}

fn afx_release_entity_ref(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) {
    unsafe {
        ((*iface).release_entity_ref)(p_ref);
    }
}

fn afx_enable_on_add_entity(iface: * mut AfxHookSource2, value: bool) {
    unsafe {
        ((*iface).enable_on_add_entity)(value);        
    }
}

fn afx_enable_on_remove_entity(iface: * mut AfxHookSource2, value: bool) {
    unsafe {
        ((*iface).enable_on_remove_entity)(value);        
    }
}

fn afx_get_entity_ref_is_valid(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = ((*iface).get_entity_ref_is_valid)(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_name(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> String {
    let result: *const c_char;
    unsafe {
        result = ((*iface).get_entity_ref_name)(p_ref);
    }
    return unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string() ;
}

fn afx_get_entity_ref_debug_name(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> Option<String> {
    let result: *const c_char;
    unsafe {
        result = ((*iface).get_entity_ref_debug_name)(p_ref);
    }
    if result.is_null() {
         return None;
    }
    return Some(unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string());
}

fn afx_get_entity_ref_class_name(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> String {
    let result: *const c_char;
    unsafe {
        result = ((*iface).get_entity_ref_class_name)(p_ref);
    }
    return unsafe { CStr::from_ptr(result) }.to_str().unwrap().to_string();
}

fn afx_get_entity_ref_is_player_pawn(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = ((*iface).get_entity_ref_is_player_pawn)(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_player_pawn_handle(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_entity_ref_player_pawn_handle)(p_ref);
    }
    return result;
}


fn afx_get_entity_ref_is_player_controller(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> bool {
    let result: bool;
    unsafe {
        result = ((*iface).get_entity_ref_is_player_controller)(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_player_controller_handle(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_entity_ref_player_controller_handle)(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_health(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_entity_ref_health)(p_ref);
    }
    return result;
}

fn afx_get_entity_ref_origin(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        ((*iface).get_entity_ref_origin)(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_render_eye_origin(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        ((*iface).get_entity_ref_render_eye_origin)(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_render_eye_angles(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef, x: & mut f32, y: & mut f32, z: & mut f32){
    unsafe {
        ((*iface).get_entity_ref_render_eye_angles)(p_ref, x, y, z);
    }
}

fn afx_get_entity_ref_view_entity_handle(iface: * mut AfxHookSource2, p_ref: * mut AfxEntityRef) -> i32 {
    let result: i32;
    unsafe {
        result = ((*iface).get_entity_ref_view_entity_handle)(p_ref);
    }
    return result;
}

fn mirv_error_type() -> JsResult<JsValue> {
    Err(JsNativeError::typ().with_message("invalid type!").into())
}

fn mirv_error_arguments() -> JsResult<JsValue> {
    Err(JsNativeError::error().with_message("invalid arguments!").into())
}

fn mirv_error_async_conflict()  -> JsResult<JsValue> {
    Err(JsNativeError::error().with_message("async conflict!").into())
}

fn mirv_message(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            for x in args {
                match x.to_string(context) {
                    Ok(s) => {
                        afx_message(mirv.iface, s.to_std_string_escaped());
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
            return Ok(JsValue::Undefined)
        }
    }
    mirv_error_type()
}

fn mirv_warning(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            for x in args {
                match x.to_string(context) {
                    Ok(s) => {
                        afx_warning(mirv.iface, s.to_std_string_escaped());
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
            return Ok(JsValue::Undefined)
        }
    }
    mirv_error_type()
}

fn mirv_exec(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            for x in args {
                match x.to_string(context) {
                    Ok(s) => {
                        afx_exec(mirv.iface, s.to_std_string_escaped());
                    }
                    Err(e) => {
                        return Err(e);
                    }
                }
            }
            return Ok(JsValue::Undefined)
        }
    }
    mirv_error_type()
}

fn mirv_set_on_game_event(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_game_event.replace(None);
                        afx_enable_on_game_event(mirv.iface, false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_game_event.replace(Some(object.clone()));
                            afx_enable_on_game_event(mirv.iface, true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    mirv_error_type()
}

fn mirv_get_on_game_event(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
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
    mirv_error_type()
}

fn mirv_set_on_c_view_render_setup_view(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_c_view_render_setup_view.replace(None);
                        afx_enable_on_c_view_render_setup_view(mirv.iface, false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_c_view_render_setup_view.replace(Some(object.clone()));
                            afx_enable_on_c_view_render_setup_view(mirv.iface, true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    mirv_error_type()
}

fn mirv_get_on_c_view_render_setup_view(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
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
    mirv_error_type()
}

fn mirv_set_on_client_frame_stage_notify(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_client_frame_stage_notify.replace(None);
                        afx_enable_on_client_frame_stage_notify(mirv.iface, false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_client_frame_stage_notify.replace(Some(object.clone()));
                            afx_enable_on_client_frame_stage_notify(mirv.iface, true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    mirv_error_type()
}

fn mirv_get_on_client_frame_stage_notify(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
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
    mirv_error_type()
}

fn mirv_run_jobs(_this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    context.run_jobs();
    Ok(JsValue::Undefined)
}

fn mirv_run_jobs_async(_this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    let task = context.run_jobs_async();
    pollster::block_on(task);    
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
                                return Err(JsNativeError::error().with_message(e.to_string()).into());
                            }
                        }
                }
            }
        }
        mirv_error_type()
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
                                return Err(JsNativeError::error().with_message(e.to_string()).into());
                            }
                        }
                }
            }
        }
        mirv_error_type()
    }
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


    fn close(this: &JsValue, _args: &[JsValue], _: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            if let Err(e) = ws_write.close().await {
                                return Err(JsNativeError::error().with_message(e.to_string()).into());
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } else {
                        return mirv_error_async_conflict();
                    }
                }
            }
            mirv_error_type()
        }
    }

    fn drop(this: &JsValue, _args: &[JsValue], _: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                if let Err(_) = mirv_ws_write.state.try_borrow_mut() {
                    return mirv_error_async_conflict();
                }
                drop(mirv_ws_write.state.replace(None));
                return Ok(JsValue::Undefined);         
            }
        }
        mirv_error_type()
    }

    fn feed(this: &JsValue, args: &[JsValue], _: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let args_clone: Vec<_> = args.iter().cloned().collect();
        let this_clone = (*this).clone();
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            for x in args_clone {
                                match x {
                                    JsValue::String(js_string) => {
                                        if let Err(e) = ws_write.feed(Message::text(js_string.to_std_string_escaped())).await {
                                            return Err(JsNativeError::error().with_message(e.to_string()).into());
                                        }
                                    }
                                    JsValue::Object(js_object) => {
                                        if let Ok(array_buffer) = JsArrayBuffer::from_object(js_object.clone()) {
                                            if let Ok(data) = array_buffer.detach(&JsValue::undefined()) {
                                                if let Err(e) = ws_write.feed(Message::binary(data)).await {
                                                    return Err(JsNativeError::error().with_message(e.to_string()).into());
                                                }
                                            }
                                        }
                                    }
                                    _ => {
                                        return mirv_error_arguments();
                                    }
                                }
                            }
                            return Ok(JsValue::Undefined)
                        }
                    } else {
                        return mirv_error_async_conflict();
                    }
                }
            }
            mirv_error_type()
        }
    }

    fn flush(this: &JsValue, _args: &[JsValue], _: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            if let Err(e) = ws_write.flush().await {
                                return Err(JsNativeError::error().with_message(e.to_string()).into());
                            }
                            return Ok(JsValue::Undefined);
                        }
                    } else {
                        return mirv_error_async_conflict();
                    }                        
                }
            }
            mirv_error_type()
        }
    }

    fn send(this: &JsValue, args: &[JsValue], _: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let args_clone: Vec<_> = args.iter().cloned().collect();
        let this_clone = (*this).clone();
        async move {
            if let Some(object) = this_clone.as_object() {
                if let Some(mirv_ws_write) = object.downcast_ref::<MirvWsWrite>() {
                    if let Ok(mut borrow_mut) = mirv_ws_write.state.try_borrow_mut() {
                        if let Some(ws_write) = &mut *borrow_mut  {
                            for x in args_clone {
                                match x {
                                    JsValue::String(js_string) => {
                                        if let Err(e) = ws_write.send(Message::text(js_string.to_std_string_escaped())).await {
                                            return Err(JsNativeError::error().with_message(e.to_string()).into());
                                        }
                                    }
                                    JsValue::Object(js_object) => {
                                        if let Ok(array_buffer) = JsArrayBuffer::from_object(js_object.clone()) {
                                            if let Ok(data) = array_buffer.detach(&JsValue::undefined()) {
                                                if let Err(e) = ws_write.send(Message::binary(data)).await {
                                                    return Err(JsNativeError::error().with_message(e.to_string()).into());
                                                }
                                            }
                                        }
                                    }
                                    _ => {
                                        return mirv_error_arguments();
                                    }
                                }
                            }
                            return Ok(JsValue::Undefined)
                        }
                    } else {
                        return mirv_error_async_conflict();
                    }
                }
            }
            mirv_error_type()
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

    fn drop(this: &JsValue, _args: &[JsValue], _: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv_ws_read) = object.downcast_ref::<MirvWsRead>() {
                if let Err(_) = mirv_ws_read.state.try_borrow_mut() {
                    return mirv_error_async_conflict();
                }
                drop(mirv_ws_read.state.replace(None));
                return Ok(JsValue::Undefined);         
            }
        }
        mirv_error_type()
    }    
    
    fn next(this: &JsValue, args: &[JsValue], context: &mut Context) -> impl Future<Output = JsResult<JsValue>> {
        let this_clone = (*this).clone();
        let mirv_result = MirvWsResult::create(this,args,context);
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
                                                    return Err(JsNativeError::error().with_message("Unexpected websocket message").into());
                                                }
                                            }
                                        }
                                        Err(e) => {
                                            return Err(JsNativeError::error().with_message(e.to_string()).into());
                                        }
                                    }
                                }
                                None => {
                                    return Ok(JsValue::null());
                                }
                            }
                        }
                    } else {
                        return mirv_error_async_conflict();
                    }
                }
            }
            mirv_error_type()
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
                        write!(&mut s, "Uncaught {e}\n").unwrap();
                        return Err(JsNativeError::error().with_message(s).into());
                    }
                }
            } else { return mirv_error_arguments(); }
        } else { return mirv_error_arguments(); }
    }
}

fn mirv_make_handle(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if args.len() == 2 {
                if let Some(entry_index) = args[0].as_number() {
                    if let Some(serial_number) = args[1].as_number() {
                        return Ok(JsValue::Integer(afx_make_handle(mirv.iface, entry_index as i32,serial_number as i32)));
                    }
                }
            }
            return mirv_error_arguments();
        }
    }
    mirv_error_type()
}

fn mirv_is_handle_valid(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if args.len() == 1 {
                if let Some(handle) = args[0].as_number() {
                    return Ok(JsValue::Boolean(afx_is_handle_valid(mirv.iface,handle as i32)));
                }
            }
            return mirv_error_arguments();
        }
    }
    mirv_error_type()
}

fn mirv_get_handle_entry_index(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if args.len() == 1 {
                if let Some(handle) = args[0].as_number() {
                    return Ok(JsValue::Integer(afx_get_handle_entry_index(mirv.iface,handle as i32)));
                }
            }
            return mirv_error_arguments();
        }
    }
    mirv_error_type()
}

fn mirv_get_handle_serial_number(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if args.len() == 1 {
                if let Some(handle) = args[0].as_number() {
                    return Ok(JsValue::Integer(afx_get_handle_serial_number(mirv.iface, handle as i32)));
                }
            }
            return mirv_error_arguments();
        }
    }
    mirv_error_type()
}

fn mirv_get_highest_entity_index(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            return Ok(JsValue::Integer(afx_get_highest_entity_index(mirv.iface)));
        }
    }
    mirv_error_type()
}

#[derive(Trace, JsData)]
struct MirvEntityRef {
    #[unsafe_ignore_trace]
    iface: * mut AfxHookSource2,
    #[unsafe_ignore_trace]
    entity_ref: * mut AfxEntityRef
}

impl Finalize for MirvEntityRef {
    // Provided method
    fn finalize(&self) {
        afx_release_entity_ref(self.iface,self.entity_ref);
    }
}

impl MirvEntityRef {
    #[must_use]
    fn new(afx_iface: * mut AfxHookSource2, afx_entity_ref: * mut AfxEntityRef) -> Self {
        
        Self {
            iface: afx_iface,
            entity_ref: afx_entity_ref
        }
    }

    fn create(
        afx_iface: * mut AfxHookSource2, afx_entity_ref: * mut AfxEntityRef,
        context: &mut Context
    ) -> JsObject {

       return ObjectInitializer::with_native_data::<MirvEntityRef>(MirvEntityRef::new(afx_iface,afx_entity_ref), context)
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
            .build();
    }

    fn is_valid(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_valid(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }

    fn get_name(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::String(js_string!(afx_get_entity_ref_name(mirv.iface,mirv.entity_ref))));
            }
        }
        mirv_error_type()
    }

    fn get_debug_name(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                if let Some(str) = afx_get_entity_ref_debug_name(mirv.iface,mirv.entity_ref) {
                    return Ok(JsValue::String(js_string!(str)));
                }
                return Ok(JsValue::null());
            }
        }
        mirv_error_type()
    }

    fn get_class_name(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::String(js_string!(afx_get_entity_ref_class_name(mirv.iface,mirv.entity_ref))));
            }
        }
        mirv_error_type()
    }

    fn is_player_pawn(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_player_pawn(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }

    fn get_player_pawn_handle(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_player_pawn_handle(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }

    fn is_player_controller(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Boolean(afx_get_entity_ref_is_player_controller(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }

    fn get_player_controller_handle(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_player_controller_handle(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }
    
    fn get_health(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_health(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }

    fn get_origin(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_origin(mirv.iface,mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        mirv_error_type()
    }

    fn get_render_eye_origin(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_render_eye_origin(mirv.iface,mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        mirv_error_type()
    } 

    fn get_render_eye_angles(this: &JsValue, _args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                let mut x: f32 = 0.0;
                let mut y: f32 = 0.0;
                let mut z: f32 = 0.0;

                afx_get_entity_ref_render_eye_angles(mirv.iface,mirv.entity_ref,&mut x, &mut y, &mut z);

                let js_array = JsArray::from_iter(
                    [JsValue::Rational(x.into()),JsValue::Rational(y.into()),JsValue::Rational(z.into())],
                    context
                );

                return Ok(JsValue::Object(JsObject::from(js_array)));
            }
        }
        mirv_error_type()
    }

    fn get_view_entity_handle(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
        if let Some(object) = this.as_object() {
            if let Some(mirv) = object.downcast_ref::<MirvEntityRef>() {
                return Ok(JsValue::Integer(afx_get_entity_ref_view_entity_handle(mirv.iface,mirv.entity_ref)));
            }
        }
        mirv_error_type()
    }     
}

fn mirv_get_entity_ref_from_index(this: &JsValue, args: &[JsValue], context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 1 == args.len() {
                if let Some(index) = args[0].as_number() {
                    let entity_ref: * mut AfxEntityRef;
                    entity_ref = afx_get_entity_ref_from_index(mirv.iface, index as i32);
                    if entity_ref.is_null() {
                        return Ok(JsValue::null());
                    }
                    return Ok(JsValue::Object(MirvEntityRef::create(mirv.iface,entity_ref,context)));
                }
            }
            return mirv_error_arguments();
        }
    }
    mirv_error_type()
}

fn mirv_set_on_add_entity(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_add_entity.replace(None);
                        afx_enable_on_add_entity(mirv.iface, false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_add_entity.replace(Some(object.clone()));
                            afx_enable_on_add_entity(mirv.iface, true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    mirv_error_type()
}

fn mirv_get_on_add_entity(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
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
    mirv_error_type()
}


fn mirv_set_on_remove_entity(this: &JsValue, args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
    if let Some(object) = this.as_object() {
        if let Some(mirv) = object.downcast_ref::<MirvStruct>() {
            if 0 < args.len() {
                match &args[0] {
                    JsValue::Undefined => {
                        mirv.events.on_remove_entity.replace(None);
                        afx_enable_on_remove_entity(mirv.iface, false);
                        return Ok(JsValue::Undefined); 
                    }
                    JsValue::Object(object) => {
                        if object.is_callable() {
                            mirv.events.on_remove_entity.replace(Some(object.clone()));
                            afx_enable_on_remove_entity(mirv.iface, true);
                            return Ok(JsValue::Undefined); 
                        }
                    }
                    _ => {
                    }
                }
            }
        }
    }
    mirv_error_type()
}

fn mirv_get_on_remove_entity(this: &JsValue, _args: &[JsValue], _context: &mut Context) -> JsResult<JsValue> {
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
    mirv_error_type()
}

impl AfxHookSource2Rs {
    pub fn new(iface: * mut AfxHookSource2) -> Self {

        let mut context = ContextBuilder::default()
            .job_queue(AsyncJobQueue::new().into())
            .build().unwrap();


        let events = Rc::<MirvEvents>::new(MirvEvents::new());

        let mirv = MirvStruct {
            iface: iface,
            events: Rc::clone(&events)
        };

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
            NativeFunction::from_fn_ptr(mirv_get_entity_ref_from_index),
            js_string!("getEntityFromIndex"),
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
            NativeFunction::from_fn_ptr(mirv_is_handle_valid),
            js_string!("isHandleValid"),
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
        .build();

        context
        .register_global_property(js_string!("mirv"), object, Attribute::all())
        .expect("property mirv shouldn't exist");

        Self {
            iface, context, events: Rc::clone(&events)
        }
    }    
}

use boa_engine::{Finalize, JsData, Trace};


//fn(_: &JsValue, _: &[JsValue], _: &mut Context) -> JsResult<JsValue>;

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_new<'a>(
    iface: * mut AfxHookSource2
) -> * mut AfxHookSource2Rs { 
    Box::into_raw(Box::new(AfxHookSource2Rs::new(iface)))
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_run_jobs(this_ptr: *mut AfxHookSource2Rs) {
    let task = (*this_ptr).context.run_jobs_async();
    pollster::block_on(task);
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_destroy(this_ptr: *mut AfxHookSource2Rs) {
    if this_ptr.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(this_ptr));
    }
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_execute(this_ptr: *mut AfxHookSource2Rs, p_data: *mut u8, len_data: usize) {
    let js_code = unsafe { std::slice::from_raw_parts(p_data, len_data) };
    match (*this_ptr).context.eval(boa_engine::Source::from_bytes(js_code)) {
        Ok(res) => {
            if let Ok(js_str) = res.to_string(&mut (*this_ptr).context) {
                let mut str = js_str.to_std_string_escaped();
                str.push_str("\n");
                afx_message((*this_ptr).iface, str);
            }
        }
        Err(e) => {
            use std::fmt::Write as _;
            let mut s = String::new();
            if let Some(source) = e.source() {
                write!(&mut s, "Uncaught {} in {}\n",e,source).unwrap();
            } else {
                write!(&mut s, "Uncaught {}\n",e).unwrap();
            }
            afx_warning((*this_ptr).iface, s);
        }
    };
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_load(this_ptr: *mut AfxHookSource2Rs, file_path: *const c_char) {
    let str_file_path = CStr::from_ptr(file_path).to_str().unwrap();
    let path = std::path::Path::new(str_file_path);
    match boa_engine::Source::from_filepath(path) {
        Ok(js_source) => {
            match (*this_ptr).context.eval(js_source) {
                Ok(res) => {
                    if let Ok(js_str) = res.to_string(&mut (*this_ptr).context) {
                        let mut str = js_str.to_std_string_escaped();
                        str.push_str("\n");
                        afx_message((*this_ptr).iface, str);
                    }
                }
                Err(e) => {
                    use std::fmt::Write as _;
                    let mut s = String::new();
                    if let Some(source) = e.source() {
                        write!(&mut s, "Uncaught {} in {}\n",e,source).unwrap();
                    } else {
                        write!(&mut s, "Uncaught {}\n",e).unwrap();
                    }
                    afx_warning((*this_ptr).iface, s);
                }
            };
        }
        Err(e) => {
            use std::fmt::Write as _;
            let mut s = String::new();
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_on_game_event(this_ptr: *mut AfxHookSource2Rs, event_name: *const c_char, event_id: i32, json: *const c_char) {
    let borrowed = (*this_ptr).events.on_game_event.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        let str_event_name = CStr::from_ptr(event_name).to_str().unwrap();
        let str_json = CStr::from_ptr(json).to_str().unwrap();

        let js_object = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("name"), JsValue::String(js_string!(str_event_name)), Attribute::all())
        .property(js_string!("id"), JsValue::Integer(event_id), Attribute::all())
        .property(js_string!("data"), JsValue::String(js_string!(str_json)), Attribute::all())
        .build();

        if let Err(e) = event_clone.call(&JsValue::null(), &[JsValue::Object(js_object)], &mut (*this_ptr).context) {
            use std::fmt::Write as _;
            let mut s = String::new();
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
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
pub unsafe extern "C" fn afx_hook_source2_rs_on_c_view_render_setup_view(this_ptr: *mut AfxHookSource2Rs, cur_time: c_float, abs_time: c_float, last_abs_time: c_float, current_view: &mut AfxHookSourceRsView , game_view: &AfxHookSourceRsView, last_view: &AfxHookSourceRsView, width: i32, height: i32) -> bool {
    let borrowed = (*this_ptr).events.on_c_view_render_setup_view.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);  
    if let Some(event_clone) = event_option_clone {

        let js_object_current_view = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("x"), JsValue::Rational(current_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(current_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(current_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(current_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(current_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(current_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(current_view.fov.into()), Attribute::all())
        .build();
        let js_object_game_view = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("x"), JsValue::Rational(game_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(game_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(game_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(game_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(game_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(game_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(game_view.fov.into()), Attribute::all())
        .build();
        let js_object_last_view = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("x"), JsValue::Rational(last_view.x.into()), Attribute::all())
        .property(js_string!("y"), JsValue::Rational(last_view.y.into()), Attribute::all())
        .property(js_string!("z"), JsValue::Rational(last_view.z.into()), Attribute::all())
        .property(js_string!("rX"), JsValue::Rational(last_view.rx.into()), Attribute::all())
        .property(js_string!("rY"), JsValue::Rational(last_view.ry.into()), Attribute::all())
        .property(js_string!("rZ"), JsValue::Rational(last_view.rz.into()), Attribute::all())
        .property(js_string!("fov"), JsValue::Rational(last_view.fov.into()), Attribute::all())
        .build();        

        let js_object = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("curTime"), JsValue::Rational(cur_time.into()), Attribute::all())
        .property(js_string!("absTime"), JsValue::Rational(abs_time.into()), Attribute::all())
        .property(js_string!("lastAbsTime"), JsValue::Rational(last_abs_time.into()), Attribute::all())
        .property(js_string!("currentView"), JsValue::Object(js_object_current_view), Attribute::all())
        .property(js_string!("gameView"), JsValue::Object(js_object_game_view), Attribute::all())
        .property(js_string!("lastView"), JsValue::Object(js_object_last_view), Attribute::all())
        .property(js_string!("width"), JsValue::Integer(width), Attribute::all())
        .property(js_string!("height"), JsValue::Integer(height), Attribute::all())
        .build();

        match event_clone.call(&JsValue::null(), &[JsValue::Object(js_object)], &mut (*this_ptr).context) {
            Ok(js_value) => {
                if let JsValue::Object(js_object) = js_value {
                    if let Ok(js_val_x) = js_object.get(js_string!("x"), &mut (*this_ptr).context) {
                        if let Some(x) = js_val_x.as_number() {
                            current_view.x = x as f32;
                        }
                    }
                    if let Ok(js_val_y) = js_object.get(js_string!("y"), &mut (*this_ptr).context) {
                        if let Some(y) = js_val_y.as_number() {
                            current_view.y = y as f32;
                        }
                    }
                    if let Ok(js_val_z) = js_object.get(js_string!("z"), &mut (*this_ptr).context) {
                        if let Some(z) = js_val_z.as_number() {
                            current_view.z = z as f32;
                        }
                    }
                    if let Ok(js_val_rx) = js_object.get(js_string!("rX"), &mut (*this_ptr).context) {
                        if let Some(rx) = js_val_rx.as_number() {
                            current_view.rx = rx as f32;
                        }
                    }
                    if let Ok(js_val_ry) = js_object.get(js_string!("rY"), &mut (*this_ptr).context) {
                        if let Some(ry) = js_val_ry.as_number() {
                            current_view.ry = ry as f32;
                        }
                    }
                    if let Ok(js_val_rz) = js_object.get(js_string!("rZ"), &mut (*this_ptr).context) {
                        if let Some(rz) = js_val_rz.as_number() {
                            current_view.rz = rz as f32;
                        }
                    }
                    if let Ok(js_val_fov) = js_object.get(js_string!("fov"), &mut (*this_ptr).context) {
                        if let Some(fov) = js_val_fov.as_number() {
                            current_view.fov = fov as f32;
                        }
                    }
                    return true;
                }
            }
            Err(e) =>  {
                use std::fmt::Write as _;
                let mut s = String::new();
                write!(&mut s, "Uncaught {e}\n").unwrap();
                afx_warning((*this_ptr).iface, s);
            }
        }
    }
    return false;
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_on_client_frame_stage_notify(this_ptr: *mut AfxHookSource2Rs, event_id: i32, is_before: bool) {
    let borrowed = (*this_ptr).events.on_client_frame_stage_notify.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        let js_object = ObjectInitializer::new(&mut (*this_ptr).context)
        .property(js_string!("curStage"), JsValue::Integer(event_id), Attribute::all())
        .property(js_string!("isBefore"), JsValue::Boolean(is_before), Attribute::all())
        .build();

        if let Err(e) = event_clone.call(&JsValue::null(), &[JsValue::Object(js_object)], &mut (*this_ptr).context) {
            use std::fmt::Write as _;
            let mut s = String::new();
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_on_add_entity(this_ptr: *mut AfxHookSource2Rs, p_ref: * mut AfxEntityRef, handle: i32) {
    let borrowed = (*this_ptr).events.on_add_entity.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        afx_add_ref_entity_ref((*this_ptr).iface,p_ref);
        let entity_ref = MirvEntityRef::create((*this_ptr).iface, p_ref, &mut (*this_ptr).context);
        if let Err(e) = event_clone.call(&JsValue::null(), &[JsValue::Object(entity_ref),JsValue::Integer(handle)], &mut (*this_ptr).context) {
            use std::fmt::Write as _;
            let mut s = String::new();
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_on_remove_entity(this_ptr: *mut AfxHookSource2Rs, p_ref: * mut AfxEntityRef, handle: i32) {
    let borrowed = (*this_ptr).events.on_remove_entity.borrow();
    let event_option_clone = borrowed.clone();
    std::mem::drop(borrowed);
    if let Some(event_clone) = event_option_clone {
        afx_add_ref_entity_ref((*this_ptr).iface,p_ref);
        let entity_ref = MirvEntityRef::create((*this_ptr).iface, p_ref, &mut (*this_ptr).context);
        if let Err(e) = event_clone.call(&JsValue::null(), &[JsValue::Object(entity_ref),JsValue::Integer(handle)], &mut (*this_ptr).context) {
            use std::fmt::Write as _;
            let mut s = String::new();
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
        }
    }
}

/*
fn main() {
    use boa_engine::{Context, Source};

    let js_code = r#"
        let two = 1 + 1;
        let definitely_not_four = two + "2";
    
        definitely_not_four
    "#;
    
    // Instantiate the execution context
    let mut context = Context::default();
    
    // Parse the source code
    match context.eval(Source::from_bytes(js_code)) {
        Ok(res) => {
            println!(
                "{}",
                res.to_string(&mut context).unwrap().to_std_string_escaped()
            );
        }
        Err(e) => {
            // Pretty print the error
            eprintln!("Uncaught {e}");
        }
    };
}
*/