#include "stdafx.h"

#include "CamPath.h"

#include "FFITools.h"
#include "StringTools.h"

#include <string>

struct Vector3Rs {
    double x;
    double y;
    double z;
};

struct QuaternionRs {
    double w;
    double x;
    double y;
    double z;
};

struct CamPathValueRs {
    Vector3Rs p;
    QuaternionRs r;
    double fov;
    FFIBool selected;
};


extern "C" void advancedfx_campath_iterator_delete(CamPathIterator * ptr) {
    delete ptr;
}

extern "C" double advancedfx_campath_iterator_get_time(const CamPathIterator * ptr) {
    return ptr->GetTime();
}

extern "C" CamPathValueRs advancedfx_campath_iterator_get_value(const CamPathIterator * ptr) {
    CamPathValue v(ptr->GetValue());
    return {
        {v.X,v.Y,v.Z},
        {v.R.W,v.R.X,v.R.Y,v.R.Z},
        v.Fov,
        BOOL_TO_FFIBOOL(v.Selected)
    };
}

extern "C" void advancedfx_campath_iterator_next(CamPathIterator * ptr) {
    ptr->operator++();
}

extern "C" FFIBool advancedfx_campath_iterator_equals(const CamPathIterator * ptr, const CamPathIterator * other_ptr) {
    return BOOL_TO_FFIBOOL(ptr->operator==(*other_ptr));
}

extern "C" CamPath * advancedfx_campath_new(void) {
    return new CamPath();
}

extern "C" void advancedfx_campath_delete(CamPath * ptr) {
    delete ptr;
}

extern "C" FFIBool advancedfx_campath_get_enabled(const CamPath * ptr) {
    return BOOL_TO_FFIBOOL(ptr->Enabled_get());
}

extern "C" void advancedfx_campath_set_enabled(CamPath * ptr, FFIBool value) {
    ptr->Enabled_set(FFIBOOL_TO_BOOL(value));
}

extern "C" double advancedfx_campath_get_offset(const CamPath * ptr) {
    return ptr->GetOffset();
}

extern "C" void advancedfx_campath_set_offset(CamPath * ptr, double value) {
    ptr->SetOffset(value);
}

extern "C" FFIBool advancedfx_campath_get_hold(const CamPath * ptr) {
    return BOOL_TO_FFIBOOL(ptr->GetHold());
}

extern "C" void advancedfx_campath_set_hold(CamPath * ptr, FFIBool value) {
    ptr->SetHold(FFIBOOL_TO_BOOL(value));
}

extern "C" uint8_t advancedfx_campath_position_interp_get(const CamPath * ptr) {
    return (uint8_t)ptr->PositionInterpMethod_get();
}

extern "C" void advancedfx_campath_position_interp_set(CamPath * ptr, uint8_t value) {
    ptr->PositionInterpMethod_set((CamPath::DoubleInterp)value);
}

extern "C" uint8_t advancedfx_campath_rotation_interp_get(const CamPath * ptr) {
    return (uint8_t)ptr->RotationInterpMethod_get();
}

extern "C" void advancedfx_campath_rotation_interp_set(CamPath * ptr, uint8_t value) {
    ptr->RotationInterpMethod_set((CamPath::QuaternionInterp)value);
}

extern "C" uint8_t advancedfx_campath_fov_interp_get(const CamPath * ptr) {
    return (uint8_t)ptr->FovInterpMethod_get();
}

extern "C" void advancedfx_campath_fov_interp_set(CamPath * ptr, uint8_t value) {
    ptr->FovInterpMethod_set((CamPath::DoubleInterp)value);
}

extern "C" void advancedfx_campath_add(CamPath * ptr, double time, const CamPathValueRs * value) {
    ptr->Add(time,CamPathValue(
        value->p.x, value->p.y, value->p.z, value->r.w, value->r.x, value->r.y, value->r.z, value->fov, FFIBOOL_TO_BOOL(value->selected)
    ));
}

extern "C" void advancedfx_campath_remove(CamPath * ptr, double time) {
    ptr->Remove(time);
}

extern "C" void advancedfx_campath_clear(CamPath * ptr) {
    ptr->Clear();
}

extern "C" size_t advancedfx_campath_get_size(const CamPath * ptr) {
    return ptr->GetSize();
}

extern "C" CamPathIterator * advancedfx_campath_get_begin(CamPath * ptr) {
    return new CamPathIterator(ptr->GetBegin());
}

extern "C" CamPathIterator * advancedfx_campath_get_end(CamPath * ptr) {
    return new CamPathIterator(ptr->GetEnd());
}

extern "C" double advancedfx_campath_get_duration(const CamPath * ptr) {
    return ptr->GetDuration();
}

extern "C" double advancedfx_campath_get_lower_bound(const CamPath * ptr) {
    return ptr->GetLowerBound();
}

extern "C" double advancedfx_campath_get_upper_bound(const CamPath * ptr) {
    return ptr->GetUpperBound();
}

extern "C" FFIBool advancedfx_campath_can_eval(const CamPath * ptr) {
    return BOOL_TO_FFIBOOL(ptr->CanEval());
}

extern "C" CamPathValueRs advancedfx_campath_eval(const CamPath * ptr, double time) {
    CamPathValue v(const_cast<CamPath *>(ptr)->Eval(time));
    return {
        {v.X,v.Y,v.Z},
        {v.R.W,v.R.X,v.R.Y,v.R.Z},
        v.Fov,
        v.Selected
    };
}

extern "C" FFIBool advancedfx_campath_load(CamPath * ptr, const char * file_name) {
    std::wstring wideStr;
    if(!UTF8StringToWideString(file_name,wideStr)) return FFIBOOL_FALSE;
    return BOOL_TO_FFIBOOL(ptr->Load(wideStr.c_str()));
}

extern "C" FFIBool advancedfx_campath_save(CamPath * ptr, const char * file_name) {
    std::wstring wideStr;
    if(!UTF8StringToWideString(file_name,wideStr)) return FFIBOOL_FALSE;
    return BOOL_TO_FFIBOOL(ptr->Save(wideStr.c_str()));
}

extern "C" void advancedfx_campath_set_start(CamPath * ptr, double time, FFIBool relative) {
    ptr->SetStart(time, FFIBOOL_TO_BOOL(relative));
}

extern "C" void advancedfx_campath_set_duration(CamPath * ptr, double time) {
    ptr->SetDuration(time);
}

extern "C" void advancedfx_campath_set_position(CamPath * ptr, double x, double y, double z, FFIBool set_x, FFIBool set_y, FFIBool set_z) {
    ptr->SetPosition(x,y,z,FFIBOOL_TO_BOOL(set_x),FFIBOOL_TO_BOOL(set_y),FFIBOOL_TO_BOOL(set_z));
}

extern "C" void advancedfx_campath_set_angles(CamPath * ptr, double y_pitch, double z_yaw, double x_roll, FFIBool set_y, FFIBool set_z, FFIBool set_x)  {
    ptr->SetAngles(y_pitch,z_yaw,x_roll,FFIBOOL_TO_BOOL(set_y),FFIBOOL_TO_BOOL(set_z),FFIBOOL_TO_BOOL(set_x));
}

extern "C" void advancedfx_campath_set_fov(CamPath * ptr, double fov) {
    ptr->SetFov(fov);
}

extern "C" void advancedfx_campath_rotate(CamPath * ptr, double y_pitch, double z_yaw, double x_roll) {
    ptr->Rotate(y_pitch,z_yaw,x_roll);
}

extern "C" void advancedfx_campath_anchor_transform(CamPath * ptr, double anchor_x, double anchor_y, double anchor_z, double anchor_y_pitch, double anchor_z_yaw, double anchor_x_roll, double dest_x, double dest_y, double dest_z, double dest_y_pitch, double dest_z_yaw, double dest_x_roll) {
    ptr->AnchorTransform(anchor_x,anchor_y,anchor_z,anchor_y_pitch,anchor_z_yaw,anchor_x_roll,dest_x,dest_y,dest_z,dest_y_pitch,dest_z_yaw,dest_x_roll);
}

extern "C" size_t advancedfx_campath_select_all(CamPath * ptr) {
    return ptr->SelectAll();
}

extern "C" void advancedfx_campath_select_none(CamPath * ptr) {
    ptr->SelectNone();
}

extern "C" size_t advancedfx_campath_select_invert(CamPath * ptr) {
    return ptr->SelectInvert();
}


extern "C" size_t advancedfx_campath_select_add_idx(CamPath * ptr, size_t min, size_t max) {
    return ptr->SelectAdd(min,max);
}

extern "C" size_t advancedfx_campath_select_add_min_count(CamPath * ptr, double min, size_t count) {
    return ptr->SelectAdd(min,count);
}

extern "C" size_t advancedfx_campath_select_add_min_max(CamPath * ptr, double min, double max) {
    return ptr->SelectAdd(min,max);
}

extern "C" void advancedfx_campath_on_changed_add(CamPath * ptr, CamPathChanged p_campath_changed, void * p_user_data) {
    ptr->OnChangedAdd(p_campath_changed,p_user_data);
}

extern "C" void advancedfx_campath_on_changed_remove(CamPath * ptr, CamPathChanged p_campath_changed, void * p_user_data) {
    ptr->OnChangedRemove(p_campath_changed,p_user_data);
}