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
        , Preview(other.Preview)
        , Capture(other.Capture)
        , CaptureType(other.CaptureType)
        , Depth24(other.Depth24)
        , DepthZip(other.DepthZip)
        , DepthCompositeSmoke(other.DepthCompositeSmoke)
        , DepthVal(other.DepthVal)
        , DepthValMax(other.DepthValMax)
        , DepthChannels(other.DepthChannels)
        , DepthMode(other.DepthMode)
        , ClearOverride(other.ClearOverride)
        , ClearOverrideColor(other.ClearOverrideColor)
        , ClearBeforeUi(other.ClearBeforeUi)
        , ClearBeforeUiColor(other.ClearBeforeUiColor)
        , AutoForceFullResSmoke(other.AutoForceFullResSmoke)
        , Settings(other.Settings)
        , BeforeCommands(other.BeforeCommands)
        , AfterCommands(other.AfterCommands)
        , ViewModelAction(other.ViewModelAction)
        , ParticlesAction(other.ParticlesAction)
        , FirstPersonLegsAction(other.FirstPersonLegsAction)
        , ShellsAction(other.ShellsAction)
        , WeaponsAction(other.WeaponsAction)
        , PlayersAction(other.PlayersAction)
        , WorldAction(other.WorldAction)
        , SkyAction(other.SkyAction)
        , SmokeAction(other.SmokeAction)
        , OverlaysAction(other.OverlaysAction)
        , ScopeAction(other.ScopeAction)
    {
        Settings->AddRef();
    }

    bool CanCaptureInMainPass() const {
        return true
            && false == ClearOverride
            && false == ClearBeforeUi
            && BeforeCommands.empty()
            && AfterCommands.empty()
            && Action::Draw == ViewModelAction
            && Action::Draw == ParticlesAction
            && Action::Draw == FirstPersonLegsAction
            && Action::Draw == ShellsAction
            && Action::Draw == WeaponsAction
            && Action::Draw == PlayersAction
            && Action::Draw == WorldAction
            && Action::Draw == SkyAction
            && Action::Draw == SmokeAction
            && Action::Draw == OverlaysAction
            && Action::Draw == ScopeAction
        ;
    }

    bool CompareRenderPassLess(const CStreamSettings &  o) const {
        int cmp;
        if(cmp = CompareBool(ClearOverride, o.ClearOverride)) return cmp < 0;
        if(ClearOverride) {
           if(cmp = CompareFloat(ClearOverrideColor.R, o.ClearOverrideColor.R)) return cmp < 0;
           if(cmp = CompareFloat(ClearOverrideColor.G, o.ClearOverrideColor.G)) return cmp < 0;
           if(cmp = CompareFloat(ClearOverrideColor.B, o.ClearOverrideColor.B)) return cmp < 0;
           if(cmp = CompareFloat(ClearOverrideColor.A, o.ClearOverrideColor.A)) return cmp < 0;
        }
        if(cmp = CompareBool(ClearBeforeUi, o.ClearBeforeUi)) return cmp < 0;
        if(ClearBeforeUi) {
           if(cmp = CompareFloat(ClearBeforeUiColor.R, o.ClearBeforeUiColor.R)) return cmp < 0;
           if(cmp = CompareFloat(ClearBeforeUiColor.G, o.ClearBeforeUiColor.G)) return cmp < 0;
           if(cmp = CompareFloat(ClearBeforeUiColor.B, o.ClearBeforeUiColor.B)) return cmp < 0;
           if(cmp = CompareFloat(ClearBeforeUiColor.A, o.ClearBeforeUiColor.A)) return cmp < 0;
        }
        if(cmp = CompareCommands(BeforeCommands, o.BeforeCommands)) return cmp < 0;
        if(cmp = CompareCommands(AfterCommands, o.AfterCommands)) return cmp < 0;
        if(cmp = CompareAction(ViewModelAction, o.ViewModelAction)) return cmp < 0;
        if(cmp = CompareAction(ParticlesAction, o.ParticlesAction)) return cmp < 0;
        if(cmp = CompareAction(FirstPersonLegsAction, o.FirstPersonLegsAction)) return cmp < 0;
        if(cmp = CompareAction(ShellsAction, o.ShellsAction)) return cmp < 0;
        if(cmp = CompareAction(WeaponsAction, o.WeaponsAction)) return cmp < 0;
        if(cmp = CompareAction(PlayersAction, o.PlayersAction)) return cmp < 0;
        if(cmp = CompareAction(WorldAction, o.WorldAction)) return cmp < 0;
        if(cmp = CompareAction(SkyAction, o.SkyAction)) return cmp < 0;
        if(cmp = CompareAction(SmokeAction, o.SmokeAction)) return cmp < 0;
        if(cmp = CompareAction(OverlaysAction, o.OverlaysAction)) return cmp < 0;
        if(cmp = CompareAction(ScopeAction, o.ScopeAction)) return cmp;
        
        // A stream can not previewed in before-capture state if there is anything else
        // relying on a later state:, so we need to differ those:
        if(
            Preview && AffectedByPreview(o.CaptureType)
        )
        {
            if((int)Capture < (int)o.Capture) return true;
        }

        return false;
    }

    std::wstring Name;

    bool Record = true;
    bool Preview = false;

    // The number ordering is intentional, so that the most "complete" pass is guaranteed to be rendered first
    // (e.g. in main pass if possible), so we don't taint later passes.
    // Basically latest capture should have lowest number and increase from there (the earlier the higher the number).
    enum class Capture_e : int {
        BeforePresent = 0,
        BeforeUi = 1
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

    bool ClearOverride = false;

    struct {
        float R = 0.0f;
        float G = 0.0f;
        float B = 0.0f;
        float A = 0.0f;
    } ClearOverrideColor;

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

    enum class Action : int {
        Draw = 0,
        NoDraw,
        ZOnly
    };

    Action ViewModelAction = Action::Draw;
    Action ParticlesAction = Action::Draw;
    Action FirstPersonLegsAction = Action::Draw;
    Action ShellsAction = Action::Draw;
    Action WeaponsAction = Action::Draw;
    Action PlayersAction = Action::Draw;
    Action WorldAction = Action::Draw;
    Action SkyAction = Action::Draw;
    Action SmokeAction = Action::Draw;
    Action OverlaysAction = Action::Draw;
    Action ScopeAction = Action::Draw;

private:
    static bool AffectedByPreview(CaptureType_e captureType) {
        switch(captureType) {
        case CStreamSettings::CaptureType_e::DepthRgb:
        case CStreamSettings::CaptureType_e::DepthF:
            return false;
        }
        return true;
    }


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
            itLhs++;
            itRhs++;
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
            itLhs++;
            itRhs++;
        }
        if(itLhs != lhs.end()) return 1;
        if(itRhs != rhs.end()) return -1;
        return 0;
    }

    static int CompareAction(Action lhs, Action rhs) {
        int cmp = (int)lhs - (int)rhs;
        if(cmp) return 0 < cmp ? 1 : -1;
        return 0;
    }
};
