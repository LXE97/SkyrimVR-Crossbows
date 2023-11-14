#pragma once
#include "mod_projectile.h"
#include "VRCR.h"

class AnimationProcessor;

class VirtualCrossbow
{
public:
    VirtualCrossbow(int base, bool hand);
    ~VirtualCrossbow();

    Animation::AnimationProcessor animator;

    enum class State : uint8_t
    {
        Start,    // default state when first equipped
        Empty,    // after firing
        Cocked,   // after reload lever pulled
        Loaded,   // ready to fire
        Sheathed, // everything disabled
        End       // used to unregister global handlers before destroying
    };
    void Global_OnGrabFinish();
    void GotoState(State newstate);

    inline State GetState() { return state; };
    inline bool GetHand() { return _hand; };
    void Unsheathe();
    void Sheathe();

    // Generic event handlers
    // Since I can't use pointers to member functions, the main plugin registers its own listeners that act as a proxy for these
    void Update();
    void OnGrabStart();
    void OnGrabStop();
    void OnOverlap(PapyrusVR::VROverlapEvent e, uint32_t id, PapyrusVR::VRDevice device);
    void OnPrimaryButtonPress(const vr::VRControllerState_t *out);
    void OnAnimEvent();

private:
    RE::TESObjectWEAP *_base;
    bool _hand;
    State prev_state;
    State state;
    float grab_initialtheta;
    bool grabAnim;
    RE::TESAmmo *ammo;

    float config_InteractReloadDistance;
    float config_InteractAimDistance;
    bool config_AllowAllActionsAlways;

    RE::NiPoint3 higgs_palmPosHandspace;
    uint32_t OverlapSphereID_PlaceArrow;

    // ambidexterity support
    RE::NiPointer<RE::NiNode> getThisHandNode();
    RE::NiPointer<RE::NiNode> getOtherHandNode();
    RE::NiPointer<RE::NiNode> getThisControllerNode();
    RE::NiPointer<RE::NiNode> getOtherControllerNode();
    RE::NiNode *getThisWeaponNode();
    RE::NiNode *getGrabNode();
    RE::NiNode *getLeverRotNode();

    // State machine
    void WriteStateToExtraData();
    void ReadStateFromExtraData();
    void OnEnterState();
    void OnExitState();

    // Specific Event handlers
    void Fire();
    void FireDry();

    // misc helpers
    void CreateOverlapSpheres(uint32_t &PlaceArrow);

    // Animation
    std::string standard_reload = "standard_reload";

    float Reload_Progress = 0;
};