use crate::advancedfx;

use std::ffi::c_char;
use std::ffi::c_void;
use std::cell::RefCell;
use std::rc::Rc;
use std::rc::Weak;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Value {
    pub p: advancedfx::math::Vector3,

    pub r: advancedfx::math::Quaternion,

    pub fov: f64,

    pub selected: bool,
}

impl Value {
    pub fn new(p: advancedfx::math::Vector3, r: advancedfx::math::Quaternion, fov: f64, selected: bool) -> Self {
        Self {
            p: p,
            r: r,
            fov: fov,
            selected: selected
        }
    }
}

type IteratorType = c_void;

extern "C" {
    fn advancedfx_campath_iterator_delete(ptr: * mut IteratorType);

    fn advancedfx_campath_iterator_get_time(ptr: * const IteratorType) -> f64;

    fn advancedfx_campath_iterator_get_value(ptr: * const IteratorType) -> Value;

    fn advancedfx_campath_iterator_next(ptr: * mut IteratorType);

    fn advancedfx_campath_iterator_equals(ptr: * const IteratorType, ptr_other: * const IteratorType) -> bool;
}

pub struct Iterator {
    ptr: * mut IteratorType,
    ptr_end: * mut IteratorType,
    valid: bool
}

impl Iterator {
    fn new(ptr: * mut IteratorType, ptr_end: * mut IteratorType) -> Self {
        Self {
            ptr: ptr,
            ptr_end: ptr_end,
            valid: true
        }
    }

    pub fn is_valid(&self) -> bool {
        self.valid && !unsafe{ advancedfx_campath_iterator_equals(self.ptr,self.ptr_end) }
    }

    pub fn get_time(&self) -> Option<f64> {
        if !self.is_valid() {
            return None;
        }
        Some(unsafe {
            advancedfx_campath_iterator_get_time(self.ptr)
        })
    }

    pub fn get_value(&self) -> Option<Value> {
        if !self.is_valid() {
            return None;
        }
        Some(unsafe {
            advancedfx_campath_iterator_get_value(self.ptr)
        })
    }

    pub fn next(&mut self) -> Option<()> {
        if !self.is_valid() {
            return None;
        }
        unsafe {
            advancedfx_campath_iterator_next(self.ptr);
        }
        Some(())
    }
}

impl Drop for Iterator {

    fn drop(&mut self) {
        unsafe {
            advancedfx_campath_iterator_delete(self.ptr);
        }
        self.ptr = std::ptr::null_mut();
    }
}

impl PartialEq for Iterator {
    fn eq(&self, other: &Self) -> bool {
        unsafe {
            advancedfx_campath_iterator_equals(self.ptr, other.ptr)
        }
    }
}

impl Eq for Iterator {}

impl CampathChangedObserver for Iterator {
    fn notify(&mut self) {
        self.valid = false;
    }
}

type CampathType = c_void;

#[repr(u8)]
pub enum DoubleInterp {
    Default = 0,
    Linear = 1,
    Cubic = 2,
}

impl std::convert::From<u8> for DoubleInterp {
    fn from(item: u8) -> Self {
       match item {
        1 => {
            DoubleInterp::Linear
        },
        2 => {
            DoubleInterp::Cubic
        }
        _ => {
            DoubleInterp::Default
        }
       }
    }
}

unsafe fn u8_to_double_interp(value: u8) -> DoubleInterp {
    unsafe { std::mem::transmute(value) }
}

#[repr(u8)]
pub enum QuaternionInterp {
    Default = 0,
    SLinear = 1,
    SCubic = 2
}

impl std::convert::From<u8> for QuaternionInterp {
    fn from(item: u8) -> Self {
       match item {
        1 => {
            QuaternionInterp::SLinear
        },
        2 => {
            QuaternionInterp::SCubic
        }
        _ => {
            QuaternionInterp::Default
        }
       }
    }
}

unsafe fn u8_to_quaternion_interp(value: u8) -> QuaternionInterp {
    unsafe { std::mem::transmute(value) }
}

type CampathChangedFn = extern "C" fn(p_user_data: * mut c_void);

extern "C" {
    fn advancedfx_campath_new() -> * mut CampathType;

    fn advancedfx_campath_delete(ptr: * mut CampathType);

    fn advancedfx_campath_get_enabled(ptr: * const CampathType) -> bool;

    fn advancedfx_campath_set_enabled(ptr: * mut CampathType, value: bool);

	fn advancedfx_campath_get_offset(ptr: * const CampathType) -> f64;

	fn advancedfx_campath_set_offset(ptr: * mut CampathType, value: f64);

    fn advancedfx_campath_get_hold(ptr: * const CampathType) -> bool;

    fn advancedfx_campath_set_hold(ptr: * mut CampathType, value: bool);

    fn advancedfx_campath_position_interp_get(ptr: * const CampathType) -> u8;

    fn advancedfx_campath_position_interp_set(ptr: * mut CampathType, value: u8);

    fn advancedfx_campath_rotation_interp_get(ptr: * const CampathType) -> u8;

    fn advancedfx_campath_rotation_interp_set(ptr: * mut CampathType, value: u8);

    fn advancedfx_campath_fov_interp_get(ptr: * const CampathType) -> u8;

    fn advancedfx_campath_fov_interp_set(ptr: * mut CampathType, value: u8);

    fn advancedfx_campath_add(ptr: * mut CampathType, time: f64, value: * const Value);

    fn advancedfx_campath_remove(ptr: * mut CampathType, time: f64);

    fn advancedfx_campath_clear(ptr: * mut CampathType);

	fn advancedfx_campath_get_size(ptr: * const CampathType) -> usize;

	fn advancedfx_campath_get_begin(ptr: * mut CampathType) -> * mut IteratorType;

    fn advancedfx_campath_get_end(ptr: * mut CampathType) -> * mut IteratorType;

    fn advancedfx_campath_get_duration(ptr: * const CampathType) -> f64;

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
	fn advancedfx_campath_get_lower_bound(ptr: * const CampathType) -> f64;

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
    fn advancedfx_campath_get_upper_bound(ptr: * const CampathType) -> f64;

	fn advancedfx_campath_can_eval(ptr: * const CampathType) -> bool;

    /***
      * @remarks Must not be called if CanEval() returns false!
      */    
    fn advancedfx_campath_eval(ptr: * const CampathType, time: f64) -> Value;

    fn advancedfx_campath_load(ptr: * const CampathType, file_name: *const c_char) -> bool;

    fn advancedfx_campath_save(ptr: * const CampathType, file_name: *const c_char) -> bool;

    /***
      * @remarks In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * @param relative If t is an relative offset (true), or absolute value (false).
      */
	fn advancedfx_campath_set_start(ptr: * mut CampathType, time: f64, relative: bool);

    /***
      * @remarks
      * In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * Setting duration for a path with less than 2 points will do nothing.
      */
	fn advancedfx_campath_set_duration(ptr: * mut CampathType, time: f64);

	fn advancedfx_campath_set_position(ptr: * mut CampathType, x: f64, y: f64, z: f64, set_x: bool, set_y: bool, set_z: bool);

	fn advancedfx_campath_set_angles(ptr: * mut CampathType, y_pitch: f64, z_yaw: f64, x_roll: f64, set_y: bool, set_z: bool, set_x: bool);

	fn advancedfx_campath_set_fov(ptr: * mut CampathType, fov: f64);

    fn advancedfx_campath_rotate(ptr: * mut CampathType, y_pitch: f64, z_yaw: f64, x_roll: f64);

    fn advancedfx_campath_anchor_transform(ptr: * mut CampathType, anchor_x: f64, anchor_y: f64, anchor_z: f64, anchor_y_pitch: f64, anchor_z_yaw: f64, anchor_x_roll: f64, dest_x: f64, dest_y: f64, dest_z: f64, dest_y_pitch: f64, dest_z_yaw: f64, dest_x_roll: f64);

	fn advancedfx_campath_select_all(ptr: * mut CampathType) -> usize;

	fn advancedfx_campath_select_none(ptr: * mut CampathType);

	fn advancedfx_campath_select_invert(ptr: * mut CampathType) -> usize;

    /***
      * Adds a range of key frames to the selection.
	  * @param min Index of first keyframe to add to selection.
	  * @param max Index of last keyframe to add to selection.
      * @returns Number of selected keyframes.
      */
    fn advancedfx_campath_select_add_idx(ptr: * mut CampathType, min: usize, max: usize) -> usize;

    /***
      * Adds a range of key frames to the selection.
      * @param min Lower time bound to start adding selection at.
      * @param count Number of keyframes to select.
      * @returns Number of selected keyframes.
      */
	fn advancedfx_campath_select_add_min_count(ptr: * mut CampathType, min: f64, count: usize) -> usize;

    /***
      * Adds a range of key frames to the selection.
      * @param min Lower time bound to start adding selection at.
      * @param max Upper bound to end adding selection at.
      * @returns Number of selected keyframes.
      */
    fn advancedfx_campath_select_add_min_max(ptr: * mut CampathType, min: f64, max: f64) -> usize;

    fn advancedfx_campath_on_changed_add(ptr: * mut CampathType, p_campath_changed: CampathChangedFn , p_user_data: * mut c_void);
    
    fn advancedfx_campath_on_changed_remove(ptr: * mut CampathType, p_campath_changed: CampathChangedFn , p_user_data: * mut c_void);
}

struct CampathChangedEvent {
    observers: Vec<Weak<RefCell<dyn CampathChangedObserver>>>
}

impl CampathChangedEvent {

    #[must_use]
    fn new() -> Self {
        Self {
            observers: Vec::<Weak<RefCell<dyn CampathChangedObserver>>>::new()        
        }
    }

    fn trigger(&mut self) {
        let mut cleanup = false;
        for x in self.observers.iter() {
            if let Some(x_rc) = x.upgrade() {
                let mut observer = x_rc.borrow_mut();
                observer.notify();
            } else {
                cleanup = true;
            }
        }
        if cleanup {
            self.observers.retain(|ref x| {
                0 < x.strong_count()
            });            
        }
    }
}

trait CampathChangedObservable {    
    fn register(&mut self, observer: Weak<RefCell<dyn CampathChangedObserver>>);

    /*fn unregister(&mut self, observer: Weak<RefCell<dyn CampathChangedObserver>>);*/
}

trait CampathChangedObserver {
    fn notify(&mut self);
}

pub struct Campath {
    ptr: * mut CampathType,
    owned: bool,
    changed_event: Option<* mut CampathChangedEvent>,
}

impl Campath {
    pub fn new() -> Self {
        let ptr = unsafe {
            advancedfx_campath_new()
        };
        Self {
            ptr: ptr,
            owned: true,
            changed_event: None,
        }
    }

    /*
    pub fn new_shared(ptr: * mut CampathType) -> Self {
        Self {
            ptr: ptr,
            owned: false
        }
    }*/

    pub fn get_enabled(&self) -> bool {
        unsafe {
            advancedfx_campath_get_enabled(self.ptr)
        }   
    }

    pub fn set_enabled(&mut self, value: bool) {
        unsafe {
            advancedfx_campath_set_enabled(self.ptr, value)
        }   
    }
    
    pub fn get_offset(&self) -> f64 {
        unsafe {
            advancedfx_campath_get_offset(self.ptr)
        }   
    }

    pub fn set_offset(&mut self, value: f64) {
        unsafe {
            advancedfx_campath_set_offset(self.ptr, value)
        }   
    }

    pub fn get_hold(&self) -> bool {
        unsafe {
            advancedfx_campath_get_hold(self.ptr)
        }   
    }

    pub fn set_hold(&mut self, value: bool) {
        unsafe {
            advancedfx_campath_set_hold(self.ptr, value)
        }   
    }

    pub fn get_position_interp(&self) -> DoubleInterp {
        unsafe { u8_to_double_interp( advancedfx_campath_position_interp_get(self.ptr) ) }
    }

    pub fn set_position_interp(&mut self, value: DoubleInterp) {
        let u8_value = value as u8;
        unsafe {
            advancedfx_campath_position_interp_set(self.ptr, u8_value)
        }   
    }

    pub fn get_rotation_interp(&self) -> QuaternionInterp {
        unsafe { u8_to_quaternion_interp( advancedfx_campath_rotation_interp_get(self.ptr) ) }
    }

    pub fn set_rotation_interp(&mut self, value: QuaternionInterp) {
        let u8_value = value as u8;
        unsafe {
            advancedfx_campath_rotation_interp_set(self.ptr, u8_value)
        }   
    }

    pub fn get_fov_interp(&self) -> DoubleInterp {
        unsafe { u8_to_double_interp( advancedfx_campath_fov_interp_get(self.ptr) ) }
    }

    pub fn set_fov_interp(&mut self, value: DoubleInterp) {
        let u8_value = value as u8;
        unsafe {
            advancedfx_campath_fov_interp_set(self.ptr, u8_value)
        }   
    }

    pub fn add(&mut self, time: f64, value: &Value) {
        unsafe {
            advancedfx_campath_add(self.ptr, time, value)
        }   
    }

    pub fn remove(&mut self, time: f64) {
        unsafe {
            advancedfx_campath_remove(self.ptr, time)
        }   
    }

    pub fn clear(&mut self) {
        unsafe {
            advancedfx_campath_clear(self.ptr)
        }   
    }

    pub fn get_size(&self) -> usize {
        unsafe {
            advancedfx_campath_get_size(self.ptr)
        }   
    }

    pub fn iterator(&mut self) -> Rc<RefCell<Iterator>> {
        let it = Rc::<RefCell<Iterator>>::new(RefCell::<Iterator>::new(Iterator::new(unsafe {
            advancedfx_campath_get_begin(self.ptr)
        }, unsafe {
            advancedfx_campath_get_end(self.ptr)
        })));
        let it_observer =  it.clone() as Rc<RefCell<dyn CampathChangedObserver>>;
        self.register(Rc::downgrade(&it_observer));
        it
    }

    pub fn get_duration(&self) -> f64 {
        unsafe {
            advancedfx_campath_get_duration(self.ptr)
        }   
    }

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
    pub fn get_lower_bound(&self) -> f64 {
        assert!(1<=self.get_size());
        unsafe {
            advancedfx_campath_get_lower_bound(self.ptr)
        }
    }

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */    
    pub fn get_upper_bound(&self) -> f64 {
        assert!(1<=self.get_size());
        unsafe {
            advancedfx_campath_get_upper_bound(self.ptr)
        }
    }  

    pub fn can_eval(&self) -> bool {
        unsafe {
            advancedfx_campath_can_eval(self.ptr)
        }   
    }

    /***
      * @remarks Must not be called if CanEval() returns false!
      */
    pub fn eval(&self, time: f64) -> Value {
        assert!(self.can_eval());
        unsafe {
            advancedfx_campath_eval(self.ptr, time)
        }   
    }

    pub fn load(&mut self, file_name: String ) -> bool {
        let c_string_file_name = std::ffi::CString::new(file_name).unwrap();
        unsafe {
            advancedfx_campath_load(self.ptr, c_string_file_name.as_ptr())
        }
    }

    pub fn save(&mut self, file_name: String ) -> bool {
        let c_string_file_name = std::ffi::CString::new(file_name).unwrap();
        unsafe {
            advancedfx_campath_save(self.ptr,c_string_file_name.as_ptr())
        }
    }

    /***
      * @remarks In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * @param relative If t is an relative offset (true), or absolute value (false).
      */
	pub fn set_start(& mut self, time: f64, relative: bool) {
        unsafe {
            advancedfx_campath_set_start(self.ptr, time, relative)
        }
    }

    /***
      * @remarks
      * In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * Setting duration for a path with less than 2 points will do nothing.
      */    
    pub fn set_duration(&mut self, time: f64) {
        unsafe {
            advancedfx_campath_set_duration(self.ptr, time)
        }
    }

    pub fn set_position(&mut self, x: Option<f64>, y: Option<f64>, z: Option<f64>) {
        let (v_x,set_x) = match x {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };
        let (v_y,set_y) = match y {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };
        let (v_z,set_z) = match z {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };      
        unsafe {
            advancedfx_campath_set_position(self.ptr, v_x,v_y,v_z, set_x,set_y,set_z)
        }
    }

    pub fn set_angles(&mut self, y_pitch: Option<f64>, z_yaw: Option<f64>, x_roll: Option<f64>) {
        let (v_y_pitch,set_y_pitch) = match y_pitch {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };
        let (v_z_yaw,set_z_yaw) = match z_yaw {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };
        let (v_x_roll,set_x_roll) = match x_roll {
            Some(v) => {
                (v, true)
            }
            None => {
                (0.0, false)
            }
        };
        unsafe {
            advancedfx_campath_set_angles(self.ptr, v_y_pitch,v_z_yaw,v_x_roll, set_y_pitch,set_z_yaw,set_x_roll)
        }
    }

    pub fn set_fov(&mut self, value: f64) {
        unsafe {
            advancedfx_campath_set_fov(self.ptr, value)
        }   
    }

    pub fn rotate(&mut self, y_pitch: f64, z_yaw: f64, x_roll: f64) {
        unsafe {
            advancedfx_campath_rotate(self.ptr, y_pitch,  z_yaw, x_roll)
        }
    }

    pub fn anchor_transform(&mut self, anchor_x: f64, anchor_y: f64, anchor_z: f64, anchor_y_pitch: f64, anchor_z_yaw: f64, anchor_x_roll: f64, dest_x: f64, dest_y: f64, dest_z: f64, dest_y_pitch: f64, dest_z_yaw: f64, dest_x_roll: f64) {
        unsafe {
            advancedfx_campath_anchor_transform(self.ptr, anchor_x, anchor_y, anchor_z, anchor_y_pitch, anchor_z_yaw, anchor_x_roll, dest_x, dest_y, dest_z, dest_y_pitch, dest_z_yaw, dest_x_roll)
        }
    }

    pub fn select_all(&mut self) -> usize {
        unsafe {
            advancedfx_campath_select_all(self.ptr)
        }
    }

    pub fn select_none(&mut self) {
        unsafe {
            advancedfx_campath_select_none(self.ptr)
        }
    }

    pub fn select_invert(&mut self) -> usize {
        unsafe {
            advancedfx_campath_select_invert(self.ptr)
        }
    }

    /***
      * Adds a range of key frames to the selection.
	  * @param min Index of first keyframe to add to selection.
	  * @param max Index of last keyframe to add to selection.
      * @returns Number of selected keyframes.
      */
    pub fn select_add_idx(&mut self, min: usize, max: usize) -> usize {
        unsafe {
            advancedfx_campath_select_add_idx(self.ptr, min, max)
        }
    }

    /***
      * Adds a range of key frames to the selection.
      * @param min Lower time bound to start adding selection at.
      * @param count Number of keyframes to select.
      * @returns Number of selected keyframes.
      */
    pub fn select_add_min_count(&mut self, min: f64, count: usize) -> usize {
        unsafe {
            advancedfx_campath_select_add_min_count(self.ptr, min, count)
        }
    }
    
    /***
      * Adds a range of key frames to the selection.
      * @param min Lower time bound to start adding selection at.
      * @param max Upper bound to end adding selection at.
      * @returns Number of selected keyframes.
      */
    pub fn select_add_min_max(&mut self, min: f64, max: f64) -> usize {
        unsafe {
            advancedfx_campath_select_add_min_max(self.ptr, min, max)
        }
    }
}

extern "C" fn advancedfx_campath_changed_fn_impl(p_user_data: * mut c_void) {
    let changed_event: &mut CampathChangedEvent = unsafe { &mut *(p_user_data as *mut CampathChangedEvent) };
    changed_event.trigger();
}

impl CampathChangedObservable for Campath {
    fn register(&mut self, observer: Weak<RefCell<dyn CampathChangedObserver>>) {
        if let Some(ptr_changed) = self.changed_event {
            unsafe{(*ptr_changed).observers.push(observer)};
        } else {
            let ptr_changed = Box::into_raw(Box::new(CampathChangedEvent::new()));
            unsafe{(*ptr_changed).observers.push(observer)};
            self.changed_event = Some(ptr_changed);
            unsafe {
                advancedfx_campath_on_changed_add(self.ptr, advancedfx_campath_changed_fn_impl, ptr_changed as *mut c_void);
            }
        }
    }

    /*fn unregister(&mut self, observer: Weak<RefCell<dyn CampathChangedObserver>>) {
        if let Some(ptr_changed) = self.changed_event {
            unsafe{(*ptr_changed).observers.retain(|x| 0 < x.strong_count() && !x.ptr_eq(&observer))};
            if 0 == unsafe{(*ptr_changed).observers.len()} {
                unsafe {
                    advancedfx_campath_on_changed_remove(self.ptr, advancedfx_campath_changed_fn_impl, ptr_changed as *mut c_void);
                }
                self.changed_event = None;
                unsafe {
                    drop(Box::from_raw(ptr_changed));
                }
            }
        }
    }*/
}

impl Drop for Campath {

    fn drop(&mut self) {
        if let Some(ptr_changed) = self.changed_event {
            unsafe {
                advancedfx_campath_on_changed_remove(self.ptr, advancedfx_campath_changed_fn_impl, ptr_changed as *mut c_void);
            }
            self.changed_event = None;
            unsafe {
                drop(Box::from_raw(ptr_changed));
            }
        }
        if self.owned {
            unsafe {
                advancedfx_campath_delete(self.ptr);
            }
        }
        self.ptr = std::ptr::null_mut();
    }
}
