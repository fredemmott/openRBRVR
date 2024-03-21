#pragma once

#include "Config.hpp"
#include "IPlugin.h"
#include "OpenXR.hpp"
#include "VR.hpp"
#include "openRBRVR.hpp"

extern Config gCfg;
extern std::unique_ptr<VRInterface> gVR;
extern bool gDrawOverlayBorder;

static openRBRVR* gPlugin;
int64_t gSeatMovement;

extern "C" __declspec(dllexport) IPlugin* RBR_CreatePlugin(IRBRGame* game)
{
    if (!gPlugin) {
        gPlugin = new openRBRVR(game);
    }
    return gPlugin;
}

enum ApiOperations : uint64_t {
    API_VERSION = 0x0,
    RECENTER_VR_VIEW = 0x1,
    TOGGLE_DEBUG_INFO = 0x2,
    OPENXR_REQUEST_INSTANCE_EXTENSIONS = 0x4,
    OPENXR_REQUEST_DEVICE_EXTENSIONS = 0x8,
    GET_VR_RUNTIME = 0x10,
    MOVE_SEAT = 0x20,
};

extern "C" __declspec(dllexport) int64_t openRBRVR_Exec(ApiOperations ops, uint64_t value)
{
    Dbg(std::format("Exec: {} {}", (uint64_t)ops, value));

    if (ops == MOVE_SEAT) {
        gSeatMovement = value;
        return 0;
    }
    if (ops == API_VERSION) {
        return 2;
    }
    if (ops & RECENTER_VR_VIEW) {
        gVR->ResetView();
    }
    if (ops & TOGGLE_DEBUG_INFO) {
        gCfg.debug = !gCfg.debug;
        gDrawOverlayBorder = (gCfg.debug && gCfg.debugMode == 0);
    }
    if ((ops & OPENXR_REQUEST_INSTANCE_EXTENSIONS) && gVR && gVR->GetRuntimeType() == OPENXR) {
        try {
            OpenXR* vr = reinterpret_cast<OpenXR*>(gVR.get());
            return reinterpret_cast<int64_t>(vr->GetInstanceExtensions());
        } catch (const std::runtime_error& e) {
            MessageBoxA(nullptr, std::format("Could not get OpenXR extensions: {}", e.what()).c_str(), "OpenXR init error", MB_OK);
        }
    }
    if ((ops & OPENXR_REQUEST_DEVICE_EXTENSIONS) && gVR && gVR->GetRuntimeType() == OPENXR) {
        try {
            OpenXR* vr = reinterpret_cast<OpenXR*>(gVR.get());
            return reinterpret_cast<int64_t>(vr->GetDeviceExtensions());
        } catch (const std::runtime_error& e) {
            MessageBoxA(nullptr, std::format("Could not get OpenXR extensions: {}", e.what()).c_str(), "OpenXR init error", MB_OK);
        }
    }
    if (ops & GET_VR_RUNTIME) {
        if (!gVR) {
            return 0;
        }
        return static_cast<int64_t>(gVR->GetRuntimeType());
    }

    return 0;
}
