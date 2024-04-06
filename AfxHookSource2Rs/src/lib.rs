use core::ffi::c_char;

use std::{cell::RefCell, collections::VecDeque, future::Future, pin::Pin};
use std::task::Poll::Ready;

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
    object::builtins::JsArrayBuffer,
    property::Attribute,
};

use async_tungstenite::async_std::ConnectStream;
use async_tungstenite::tungstenite::protocol::Message;
use async_tungstenite::WebSocketStream;

pub struct AfxHookSource2 {
    message: unsafe extern "C" fn(s: *const c_char),
    warning: unsafe extern "C" fn(s: *const c_char),
    exec: unsafe extern "C" fn(s: *const c_char),
//    list_entities: unsafe extern "C" fn(unsafe extern "C" fn(i32, *mut c_void) -> bool),
}

/*
        let runtime = tokio::runtime::Builder::new_multi_thread()
            .thread_name("afx-hook-source2-rs")
            .build()
            .unwrap();
*/

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

pub struct AfxHookSource2Rs {
    iface: * mut AfxHookSource2,
    context: boa_engine::Context,
}

#[derive(Trace, Finalize, JsData)]
struct MirvStruct {
    #[unsafe_ignore_trace]
    iface: * mut AfxHookSource2,
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

impl AfxHookSource2Rs {
    pub fn new(iface: * mut AfxHookSource2) -> Self {

        let mut context = ContextBuilder::default()
            .job_queue(AsyncJobQueue::new().into()  )
            .build().unwrap();

        let mirv = MirvStruct {
            iface: iface
        };
        
        let object = ObjectInitializer::with_native_data::<MirvStruct>(mirv, &mut context)
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
            NativeFunction::from_async_fn(mirv_connect_async),
            js_string!("connect_async"),
            0,
        )
        .build();

        context
        .register_global_property(js_string!("mirv"), object, Attribute::all())
        .expect("property mirv shouldn't exist"); 

        Self {
            iface, context
        }
    }    
}

use boa_engine::{Finalize, JsData, Trace};


//fn(_: &JsValue, _: &[JsValue], _: &mut Context) -> JsResult<JsValue>;

#[no_mangle]
pub unsafe extern "C" fn afx_hook_source2_rs_new(
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
            write!(&mut s, "Uncaught {e}\n").unwrap();
            afx_warning((*this_ptr).iface, s);
        }
    };
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