#pragma once

#include "../shared/RecordingSettings.h"

#include <string>

class CStreamSettings {
public:
    CStreamSettings(advancedfx::CRecordingSettings * settings)
    : Settings(settings){
        settings->AddRef();
    }

    ~CStreamSettings() {
        Settings->Release();
    }

    CStreamSettings(const CStreamSettings & other)
        : Name(other.Name)
        , Record(other.Record)
        , Capture(other.Capture)
        , CaptureType(other.CaptureType)
        , Depth24(other.Depth24)
        , DepthZip(other.DepthZip)
        , DepthCompositeSmoke(other.DepthCompositeSmoke)
        , DepthVal(other.DepthVal)
        , DepthValMax(other.DepthValMax)
        , DepthChannels(other.DepthChannels)
        , DepthMode(other.DepthMode)
        , ClearBeforeUi(other.ClearBeforeUi)
        , ClearBeforeUiColor(other.ClearBeforeUiColor)
        , AutoForceFullResSmoke(other.AutoForceFullResSmoke)
        , Settings(other.Settings)
    {
        Settings->AddRef();
    }

    std::wstring Name;

    bool Record = true;

    enum class Capture_e {
        BeforePresent,
        BeforeUi
    } Capture = Capture_e::BeforePresent;

    enum class CaptureType_e {
        Rgb,
        Rgba,
        DepthRgb,
        DepthF
    } CaptureType = CaptureType_e::Rgb;

    bool Depth24 = false;
    bool DepthZip = false;

    bool DepthCompositeSmoke = true;

    float DepthVal = 7;
    float DepthValMax = 2100;

    enum class DepthChannels_e {
        Gray,
        SplitRgb,
        Dithered
    } DepthChannels = DepthChannels_e::Gray;

    enum class DepthMode_e {
        Inverse,
        Linear,
        LogE,
        PyramidalLinear,
        PyramidalLogE
    } DepthMode = DepthMode_e::Inverse;

    bool ClearBeforeUi = false;

    struct {
        float R = 0.0f;
        float G = 0.0f;
        float B = 0.0f;
        float A = 0.0f;
    } ClearBeforeUiColor;

    bool AutoForceFullResSmoke = true;

    advancedfx::CRecordingSettings* Settings;

    bool WantsSmokeComposite() {
        return (
            CaptureType == CaptureType_e::DepthF
            || CaptureType == CaptureType_e::DepthRgb
        ) && (
            DepthCompositeSmoke
        );
    }

    bool WantsFullResSmoke() {
        return WantsSmokeComposite()
            && AutoForceFullResSmoke
        ;
    }
};
