

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Value {
    p: advancedfx::math::Vector3,

    r: advancedfx::math::Quaternion,

    fov: f64,

    selected: bool,
}

type IteratorType = c_void;

extern "C" {
    fn advancedfx_campath_iterator_delete(ptr: * mut IteratorType);

    fn advancedfx_campath_iterator_get_time(ptr: * IteratorType) -> f64;

    fn advancedfx_campath_iterator_get_value(ptr: * IteratorType) -> Value;

    fn advancedfx_campath_iterator_next(ptr: * mut IteratorType);

    fn advancedfx_campath_iterator_equals(ptr: * IteratorType, ptr_other * IteratorType) -> bool;
}

struct Iterator {
    ptr: * mut IteratorType;
}

impl Iterator {
    fn new(ptr: * mut IteratorType) -> Self {
        Self {
            ptr: ptr;
        }
    }

    fn get_time(&self) {
        unsafe {
            advancedfx_campath_iterator_get_time(self.ptr)
        }   
    }

    fn get_value(&self) {
        unsafe {
            advancedfx_campath_iterator_get_value(self.ptr);
        }   
    }

    fn next(&mut self) {
        advancedfx_campath_iterator_next(self.ptr);
    }
}

impl<T> Drop for Iterator {

    fn drop(&mut self) {
        unsafe {
            advancedfx_campath_iterator_delete(self.p_ref);
        }
        self.p_ref = null;
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

type CampathType = c_void;

enum DoubleInterp {
    Default = 0,
    Linera = 1,
    Cubic = 2,
};

enum QuaternionInterp {
    Default = 0,
    SLinear = 1,
    SCubic = 2
};

extern "C" {
    fn advancedfx_campath_new() -> * mut CampathType;

    fn advancedfx_campath_delete(ptr: * mut CampathType);

    fn advancedfx_campath_get_enabled(ptr: * CampathType) -> bool;

    fn advancedfx_campath_set_enabled(ptr: * mut CampathType, value: bool);

	fn advancedfx_campath_get_offset(ptr: * CampathType) -> f64;

	fn advancedfx_campath_set_offset(ptr: * mut CampathType, value: f64);

    fn advancedfx_campath_get_hold(ptr: * CampathType) -> bool;

    fn advancedfx_campath_set_hold(ptr: * mut CampathType, value: bool);

    fn advancedfx_campath_position_interp_get(ptr: * CampathType) -> i32;

    fn advancedfx_campath_position_interp_set(ptr: * mut CampathType, value: i32);

    fn advancedfx_campath_rotation_interp_get(ptr: * CampathType) -> i32;

    fn advancedfx_campath_rotation_interp_set(ptr: * mut CampathType, value: i32);

    fn advancedfx_campath_fov_interp_get(ptr: * CampathType) -> i32;

    fn advancedfx_campath_fov_interp_set(ptr: * mut CampathType, value: i32);

    fn advancedfx_campath_add(ptr: * mut CampathType, time: f64, &value: Value);

    fn advancedfx_campath_remove(ptr: * mut CampathType, time: f64);

    fn advancedfx_campath_clear(ptr: * mut CampathType);

	fn advancedfx_campath_get_size(ptr * CampathType) -> usize;

	fn advancedfx_campath_get_begin(ptr: * mut CampathType) -> * mut IteratorType;

    fn advancedfx_campath_get_end(ptr: * mut CampathType) -> * mut IteratorType;

    fn advancedfx_campath_get_duration(ptr: * CampathType) -> f64;

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
	fn advancedfx_campath_get_lower_bound(ptr: * CampathType) -> f64;

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
    fn advancedfx_campath_get_upper_bound(ptr: * CampathType) -> f64;

	fn advancedfx_campath_can_eval(ptr: * CampathType) -> bool;

    /***
      * @remarks Must not be called if CanEval() returns false!
      */    
    fn advancedfx_campath_eval(ptr: * CampathType, time: f64) -> CampathValue;

    fn advancedfx_campath_load(ptr: * CampathType, file_name: *const c_char) -> bool;

    fn advancedfx_campath_save(ptr: * CampathType, file_name: *const c_char) -> bool;

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

    fn advancedfx_campath_get_changed(ptr: * CampathType) -> bool;

    fn advancedfx_campath_reset_changed(ptr: * mut CampathType);
}

struct Campath {
    ptr: * mut CampathType,
    owned: bool,
}

impl Iterator {
    fn new() -> Self {
        let ptr = unsafe {
            advancedfx_campath_new()
        };
        Self {
            ptr: ptr,
            owned: true
        }
    }

    fn new_shared(ptr: * mut CampathType) -> Self {
        Self {
            ptr: ptr,
            owned: false
        }
    }

    fn get_enabled(&self) -> bool {
        unsafe {
            advancedfx_campath_get_enabled(self.ptr)
        }   
    }

    fn set_enabled(&mut self, value: bool) {
        unsafe {
            advancedfx_campath_set_enabled(self.ptr, value);
        }   
    }
    
    fn get_offset(&self) -> f64 {
        unsafe {
            advancedfx_campath_get_offset(self.ptr)
        }   
    }

    fn set_offset(&mut self, value: f64) {
        unsafe {
            advancedfx_campath_set_offset(self.ptr, value);
        }   
    }

    fn get_hold(&self) -> bool {
        unsafe {
            advancedfx_campath_get_hold(self.ptr)
        }   
    }

    fn set_hold(&mut self, value: bool) {
        unsafe {
            advancedfx_campath_set_hold(self.ptr, value);
        }   
    }

    fn get_position_interp(&self) -> DoubleInterp {
        unsafe {
            advancedfx_campath_position_interp_get(self.ptr)
        } as DoubleInterp
    }

    fn set_position_interp(&mut self, value: DoubleInterp) {
        let i32_value = value as i32;
        unsafe {
            advancedfx_campath_position_interp_set(self.ptr, i32_value);
        }   
    }

    fn get_rotation_interp(&self) -> QuaternionInterp {
        unsafe {
            advancedfx_campath_rotation_interp_get(self.ptr)
        } as QuaternionInterp
    }

    fn set_rotation_interp(&mut self, value: QuaternionInterp) {
        let i32_value = value as i32;
        unsafe {
            advancedfx_campath_rotation_interp_set(self.ptr, i32_value);
        }   
    }

    fn get_fov_interp(&self) -> DoubleInterp {
        unsafe {
            advancedfx_campath_fov_interp_get(self.ptr)
        } as DoubleInterp
    }

    fn set_fov_interp(&mut self, value: DoubleInterp) {
        let i32_value = value as i32;
        unsafe {
            advancedfx_campath_fov_interp_set(self.ptr, i32_value);
        }   
    }

    fn add(&mut self, time: f64, &value: Value) {
        unsafe {
            advancedfx_campath_add(self.ptr, time, value);
        }   
    }

    fn remove(&mut self, time: f64) {
        unsafe {
            advancedfx_campath_remove(self.ptr, time);
        }   
    }

    fn clear(&mut self) {
        unsafe {
            advancedfx_campath_clear(self.ptr);
        }   
    }

    fn get_size(&self) -> usize {
        unsafe {
            advancedfx_campath_get_size(self.ptr);
        }   
    }

    fn get_begin(&mut self) -> Iterator {
        Iterator::new(unsafe {
            advancedfx_campath_get_begin(self.ptr)
        })
    }

    fn get_end(&mut self) -> Iterator {
        Iterator::new(unsafe {
            advancedfx_campath_get_end(self.ptr)
        })
    }

    fn get_size(&self) -> usize {
        unsafe {
            advancedfx_campath_get_size(self.ptr);
        }   
    }

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */
    fn get_lower_bound(&self) -> f64 {
        assert(1<=self.get_size());
        unsafe {
            advancedfx_campath_get_lower_bound(self.ptr);
        }
    }

    /***
      * @remarks Must not be called if GetSize is less than 1!
      */    
    fn get_upper_bound(&self) -> f64 {
        assert(1<=self.get_size());
        unsafe {
            advancedfx_campath_get_upper_bound(self.ptr);
        }
    }  

    fn can_eval(&self) -> bool {
        unsafe {
            advancedfx_campath_can_eval(self.ptr);
        }   
    }

    /***
      * @remarks Must not be called if CanEval() returns false!
      */
    fn eval(&self, time: f64) -> Value {
        assert(self.can_eval());
        unsafe {
            advancedfx_campath_eval(self.ptr, time);
        }   
    }

    fn load(&mut self, file_name: String ) -> bool {
        let c_string_file_name = std::ffi::CString::new(file_name).unwrap();
        unsafe {
            advancedfx_campath_eval(c_string_file_name.as_ptr())
        }
    }

    fn save(&mut self, file_name: String ) -> bool {
        let c_string_file_name = std::ffi::CString::new(file_name).unwrap();
        unsafe {
            advancedfx_campath_save(c_string_file_name.as_ptr())
        }
    }

    /***
      * @remarks In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * @param relative If t is an relative offset (true), or absolute value (false).
      */
	fn set_start(& mut self, time: f64, relative: bool) {
        unsafe {
            advancedfx_campath_set_start(self.ptr, time, relative)
        }
    }

    /***
      * @remarks
      * In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
      * Setting duration for a path with less than 2 points will do nothing.
      */    
    fn set_duration(&mut self, time: f64) {
        unsafe {
            advancedfx_campath_set_duration(self.ptr, time)
        }
    }

    fn set_position(&mut self, x: Option<f64>, y: Option<f64>, z: Option<f64>) {
        let (v_x,set_x) = match x {
            Some(v) {
                (v, true)
            }
            None {
                (v, fasle)
            }
        }
        let (v_y,set_y) = match y {
            Some(v) {
                (v, true)
            }
            None {
                (v, fasle)
            }
        }
        let (v_z,set_z) = match z {
            Some(v) {
                (v, true)
            }
            None {
                (v, fasle)
            }
        }        
        unsafe {
            advancedfx_campath_set_position(self.ptr, v_x,v_y,v_z, set_x,set_y,set_z)
        }
    }

    fn set_angles(&mut self, y_pitch: Option<f64>, z_yaw Option<f64>, x_roll: Option<f64>) {
        let (v_y_pitch,set_y_pitch) = match y_pitch {
            Some(v) {
                (v, true)
            }
            None {
                (v, false)
            }
        }
        let (v_z_yaw,set_z_yaw) = match z_yaw {
            Some(v) {
                (v, true)
            }
            None {
                (v, false)
            }
        }
        let (v_x_roll,set_x_roll) = match x_roll {
            Some(v) {
                (v, true)
            }
            None {
                (v, false)
            }
        }        
        unsafe {
            advancedfx_campath_set_position(self.ptr, v_y_pitch,v_z_yaw,v_x_roll, set_y_pitch,set_z_yaw,set_x_roll)
        }
    }

    fn set_fov(&mut self, value: f64) {
        unsafe {
            advancedfx_campath_set_fov(self.ptr, value)
        }   
    }

    fn rotate(&mut self, y_pitch: f64, z_yaw: f64, x_roll: f64) {
        unsafe {
            advancedfx_campath_rotate(self.ptr, y_pitch,  z_yaw, x_roll)
        }
    }

    fn anchor_transform(&mut self, anchor_x: f64, anchor_y: f64, anchor_z: f64, anchor_y_pitch: f64, anchor_z_yaw: f64, anchor_x_roll: f64, dest_x: f64, dest_y: f64, dest_z: f64, dest_y_pitch: f64, dest_z_yaw: f64, dest_x_roll: f64) {
        unsafe {
            advancedfx_campath_anchor_transform(self.ptr, anchor_x, anchor_y, anchor_z, anchor_y_pitch, anchor_z_yaw, anchor_x_roll, dest_x, dest_y, dest_z, dest_y_pitch, dest_z_yaw, dest_x_roll)
        }
    }

    select_all(&self mut) -> usize {
        unsafe {
            advancedfx_campath_select_all(self.ptr)
        }
    }

    select_none(&self mut) {
        unsafe {
            advancedfx_campath_select_none(self.ptr)
        }
    }

    select_invert(&self mut) -> usize {
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
    fn select_add_idx(&mut self, min: usize, max: usize) -> usize {
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
    select_add_min_count(&mut self, min: f64, count: usize) -> usize {
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
    select_add_min_max(&mut self, min: f64, max: f64) -> usize {
        unsafe {
            advancedfx_campath_select_add_min_max(self.ptr, min, max)
        }
    }

    fn get_changed(&self) -> bool {
        unsafe {
            advancedfx_campath_get_changed(self.ptr)
        }   
    }

    fn reset_changed(&mut self) {
        unsafe {
            advancedfx_campath_reset_changed(self.ptr);
        }   
    }    
}

impl<T> Drop for Iterator {

    fn drop(&mut self) {
        if self.owned {
            unsafe {
                advancedfx_campath_iterator_delete(self.ptr);
            }
        }
        self.ptr = null;
    }
}
