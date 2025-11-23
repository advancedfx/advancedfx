#pragma once

#include "../shared/RecordingSettings.h"

#include <string>
#include <list>

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
        , BeforeCommands(other.BeforeCommands)
        , AfterCommands(other.AfterCommands)
    {
        Settings->AddRef();
    }

    bool CanCaptureInMainPass() const {
        return true
            && false == ClearBeforeUi
            && BeforeCommands.empty()
            && AfterCommands.empty()
        ;
    }

    int CompareRenderPass(const CStreamSettings &  o) const {
        int cmp;
        if(cmp = CompareBool(ClearBeforeUi, o.ClearBeforeUi)) return cmp;
        if(ClearBeforeUi) {
           if(cmp = CompareFloat(ClearBeforeUiColor.R, o.ClearBeforeUiColor.R)) return cmp;
           if(cmp = CompareFloat(ClearBeforeUiColor.G, o.ClearBeforeUiColor.G)) return cmp;
           if(cmp = CompareFloat(ClearBeforeUiColor.B, o.ClearBeforeUiColor.B)) return cmp;
           if(cmp = CompareFloat(ClearBeforeUiColor.A, o.ClearBeforeUiColor.A)) return cmp;
        }
        if(cmp = CompareCommands(BeforeCommands, o.BeforeCommands)) return cmp;
        if(cmp = CompareCommands(AfterCommands, o.AfterCommands)) return cmp;

        return 0;
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

    std::list<std::list<std::string>> BeforeCommands;
    std::list<std::list<std::string>> AfterCommands;

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

private:
    static int CompareBool(bool lhs, bool rhs) {
        if(lhs != rhs) return lhs ? 1 : -1;
        return 0;
    }

    static int CompareFloat(float lhs, float rhs) {
        if(lhs != rhs) return lhs > rhs ? 1 : -1;
        return 0;
    }

    static int CompareCommand(const std::list<std::string> & lhs, const std::list<std::string> & rhs) {
        auto itLhs = lhs.begin();
        auto itRhs = rhs.begin();
        while(itLhs != lhs.end() && itRhs != rhs.end()) {
            if(int cmp = itLhs->compare(*itRhs)) return cmp;
        }
        if(itLhs != lhs.end()) return 1;
        if(itRhs != rhs.end()) return -1;
        return 0;        
    }

    static int CompareCommands(const std::list<std::list<std::string>> & lhs, const std::list<std::list<std::string>> & rhs) {        
        auto itLhs = lhs.begin();
        auto itRhs = rhs.begin();
        while(itLhs != lhs.end() && itRhs != rhs.end()) {
            if(int cmp = CompareCommand(*itLhs, *itRhs)) return cmp;
        }
        if(itLhs != lhs.end()) return 1;
        if(itRhs != rhs.end()) return -1;
        return 0;
    }

};
