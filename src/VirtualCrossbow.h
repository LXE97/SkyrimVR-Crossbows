#pragma once
#include "mod_projectile.h"
#include "VRCR.h"
#include "VRHolster.h"

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
    void GotoState(State newstate);

    inline State GetState() { return state; };
    inline bool GetHand() { return _hand; };

    // Public event handlers
    // Since I can't pass pointers to member functions, the main plugin registers its own listeners that act as a proxy for these
    void Update();
    void OnGrabStart();
    void OnGrabStop();
    void OnOverlap(const vrinput::OverlapEvent &e);
    void OnPrimaryButtonPress(const vr::VRControllerState_t *out);
    void OnAnimEvent();
    void onUnsheathe();
    void onSheathe();

private:
    // state variables
    RE::TESObjectWEAP *_base;
    bool _hand;
    State prev_state = State::Start;
    State state;

    RE::TESAmmo *ammo;
    int32_t OverlapSphereID_PlaceArrow;
    // animation related state variables and constants
    std::string standard_reload = "standard_reload";
    const float maxrot = 0.6283185;
    const float minrot = 0.01745329;
    bool reloadGrabState = false;
    bool grabAnim = false;
    float AnimProgress_ReloadAngle;
    float AnimProgress_ReloadAngle_initial;

    // TODO: read from config file
    float config_InteractReloadDistance = 8;
    float config_InteractAimDistance = 8;
    bool config_AllowAllActionsAlways = true;
    RE::NiPoint3 higgs_palmPosHandspace = {0, -2.4, 6};

    // Node getters / ambidexterity support
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

    // misc
    void Fire();
    void FireDry();

    // misc helpers
    void CreateOverlapSpheres(uint32_t &PlaceArrow);
};
