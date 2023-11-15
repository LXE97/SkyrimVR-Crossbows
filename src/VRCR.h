#pragma once
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#include <cmath>
#include <algorithm>
#include "SKSE/Impl/Stubs.h"

#include "VR/PapyrusVRAPI.h"
#include "VR/VRManagerAPI.h"
#include "VR/OpenVRUtils.h"
#include "higgsinterface001.h"
#include "vrikinterface001.h"

#include "mod_input.h"
#include "mod_eventSink.hpp"
#include "menuChecker.h"
#include "mod_projectile.h"
#include "mod_animation.h"
#include "helper_math.h"
#include "VirtualCrossbow.h"
#include "VRHolster.h"

class AnimationDataManager;

namespace VRCR
{
    extern RE::PlayerCharacter *g_player;
    extern SKSE::detail::SKSETaskInterface *g_task;
    extern OpenVRHookManagerAPI *g_OVRHookManager;
    extern PapyrusVR::VRManagerAPI *g_VRManager;
    extern vr::TrackedDeviceIndex_t l_controller;
    extern vr::TrackedDeviceIndex_t r_controller;

    const std::string thisPluginName = "VR_Crossbow_Overhaul.esp";
    extern uint8_t thisPluginID;

    // temp config section
    extern RE::NiTransform config_SavedAimGrabHandspace;
    extern RE::NiPoint3 config_SavedAimGrabPosition;

    /// Main plugin entry point/ initialization function
    void StartMod();
    /// kPostLoadGame do stuff once player has loaded
    void GameLoad();
    void PreGameLoad();

    // HIGGS
    void onWeaponGrabStart();
    void onWeaponGrabFinish();
    void Update_PreHIGGS();
    void OverrideHiggsConfig();
    void RestoreHiggsConfig();
    void OnOverlap(const vrinput::OverlapEvent &e);

    // Event handlers
    void onMenuOpenClose(const RE::MenuOpenCloseEvent *event);
    void onAnimEvent(const RE::BSAnimationGraphEvent *event);
    void onEquipEvent(const RE::TESEquipEvent *event);
    void onContainerChange(const RE::TESContainerChangedEvent *event);
    void onGrabButtonPress(vr::VRControllerState_t *const out);
    void onGrabButtonRelease(vr::VRControllerState_t *const out);

    void RegisterVRInputCallback();

    // Utilities
    inline RE::FormID getFullFormID(RE::FormID partial) { return thisPluginID << 24 | partial; }
}
