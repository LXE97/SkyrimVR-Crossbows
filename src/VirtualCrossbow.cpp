#include "VirtualCrossbow.h"

VirtualCrossbow::VirtualCrossbow(int base, bool hand)
{
    SKSE::log::info("Creating virtual crossbow for {:X} in {} hand", base, hand ? "left" : "right");
    _hand = hand;

    GotoState(State::Start);
}

VirtualCrossbow::~VirtualCrossbow()
{
    // ->DestroyLocalOverlapObject(OverlapSphereID_PlaceArrow);
    GotoState(State::End);
    WriteStateToExtraData();
}

void VirtualCrossbow::Update()
{
    using namespace RE;

    animator.Update();

    if (grabAnim)
    {
        auto crossbowRot = getLeverRotNode();
        if (crossbowRot)
        {

            if (reloadGrabState)
            {
                // reload lever is currently being grabbed
                auto crossbowGrab = getGrabNode();
                auto weaponNode = getThisWeaponNode();
                auto grabHand = getOtherHandNode();
                auto grabController = getOtherControllerNode();

                if (grabHand && crossbowGrab)
                {

                    // TODO: put in helper math function
                    // get angle between the rotation axis and the grabbing controller
                    NiPoint3 dvector = grabController->world.translate - crossbowRot->world.translate;
                    dvector = weaponNode->world.rotate.Transpose() * dvector;
                    AnimProgress_ReloadAngle = atan2(-1.0 * dvector.z, dvector.y);

                    AnimProgress_ReloadAngle -= AnimProgress_ReloadAngle_initial;

                    if (AnimProgress_ReloadAngle >= maxrot && state == State::Empty)
                    {
                        animator.RemoveAnimation(standard_reload);
                        GotoState(State::Cocked);
                    }

                    NiPoint3 rot;
                    auto ctx = NiUpdateData();
                    crossbowRot->local.rotate.ToEulerAnglesXYZ(rot);
                    rot.x = std::clamp(AnimProgress_ReloadAngle, minrot, maxrot);
                    crossbowRot->local.rotate.SetEulerAnglesXYZ(rot);
                    crossbowRot->Update(ctx);

                    // update grab hand transform
                    auto handTransform = grabHand->world;
                    auto palmPos = handTransform * higgs_palmPosHandspace;
                    auto desiredTransform = weaponNode->world;
                    desiredTransform.translate += palmPos - crossbowGrab->world.translate;
                    auto desiredTransformHandspace = handTransform.Invert() * desiredTransform;
                    g_higgsInterface->SetGrabTransform(true, desiredTransformHandspace);
                }
            }
            else
            {
                // lever is not being grabbed but we aren't done animating

                // TODO : accelerate based on framestep
                if (AnimProgress_ReloadAngle > minrot)
                {
                    AnimProgress_ReloadAngle -= (maxrot - minrot) / 60;
                    if (AnimProgress_ReloadAngle < minrot)
                    {
                        AnimProgress_ReloadAngle = minrot;
                    }

                    NiPoint3 rot;
                    auto ctx = NiUpdateData();
                    crossbowRot->local.rotate.ToEulerAnglesXYZ(rot);
                    rot.x = std::clamp(AnimProgress_ReloadAngle, minrot, maxrot);
                    crossbowRot->local.rotate.SetEulerAnglesXYZ(rot);
                    crossbowRot->Update(ctx);
                }
                else
                {
                    animator.RemoveAnimation(standard_reload);
                    grabAnim = false;
                }
            }
        }
    }
}

void VirtualCrossbow::OnGrabStart()
{
    using namespace RE;

    // determine if grabbing hand is in position for aiming, reload, or other
    auto weaponNode = getThisWeaponNode();
    auto grabHand = getOtherHandNode();

    NiPoint3 weaponToHand = weaponNode->world.translate - grabHand->world.translate;
    NiPoint3 weaponToHandHandspace = weaponNode->world.rotate.Transpose() * weaponToHand;
    SKSE::log::info("crossbow grab");

    // TODO: check hand orientation in addition to relative position

    // hand above crossbow, reload
    if ((state == State::Empty || config_AllowAllActionsAlways) && weaponToHandHandspace.z > 1.f)
    {
        auto crossbowGrab = getGrabNode();
        auto crossbowRot = getLeverRotNode();
        auto grabController = getOtherControllerNode();
        if (crossbowGrab && crossbowRot && grabController)
        {
            if (crossbowGrab->world.translate.GetDistance(grabController->world.translate) < config_InteractReloadDistance)
            {
                // TODO finger curl - need VRIK possibly later callback

                // save/modify higgs config values
                VRCR::OverrideHiggsConfig();

                // get angle between the rotation axis and the grabbing controller
                NiPoint3 dvector = grabController->world.translate - crossbowRot->world.translate;
                dvector = weaponNode->world.rotate.Transpose() * dvector;
                AnimProgress_ReloadAngle_initial = atan2(-1.0 * dvector.z, dvector.y);

                auto handTransform = grabHand->world;
                auto palmPos = handTransform * higgs_palmPosHandspace;
                auto desiredTransform = weaponNode->world;
                desiredTransform.translate += palmPos - crossbowGrab->world.translate;
                auto desiredTransformHandspace = handTransform.Invert() * desiredTransform;
                g_higgsInterface->SetGrabTransform(true, desiredTransformHandspace);

                grabAnim = true;
                reloadGrabState = true;
                animator.AddAnimation(standard_reload, &AnimProgress_ReloadAngle, minrot, maxrot);
            }
            else
            {
                SKSE::log::info("too far away for reload grab");
            }
        }
    }

    // hand below crossbow, aim
    else if (state == State::Loaded || config_AllowAllActionsAlways)
    {
        if (VRCR::config_SavedAimGrabHandspace.scale > 0 &&
            grabHand->world.translate.GetDistance(weaponNode->world.translate + VRCR::config_SavedAimGrabPosition) < config_InteractAimDistance)
        {
            g_higgsInterface->SetGrabTransform(true, VRCR::config_SavedAimGrabHandspace);
        }
        // TODO: set fingers
    }
}

void VirtualCrossbow::OnGrabStop()
{
    if (grabAnim && reloadGrabState && state != State::Empty)
    {
        animator.RemoveAnimation(standard_reload);
        VRCR::RestoreHiggsConfig();
    }
    reloadGrabState = false;
}

void VirtualCrossbow::OnOverlap(const vrinput::OverlapEvent &e)
{
    using namespace RE;
    switch (state)
    {
    case State::Cocked:
        if (e.ID == OverlapSphereID_PlaceArrow)
        {
            auto heldRef = g_higgsInterface->GetGrabbedObject(!_hand);
            if (heldRef && heldRef->IsAmmo() && heldRef->As<TESAmmo>()->IsBolt())
            {
                ammo = heldRef->As<TESAmmo>();
                heldRef->DeleteThis();
            }
        }
    }
}

void VirtualCrossbow::OnAnimEvent()
{
    switch (state)
    {
    case State::Sheathed:
        // if event = un sheathe
        GotoState(prev_state);
    default:
        GotoState(State::Sheathed);
    }
}

bool VirtualCrossbow::OnPrimaryButtonPress()
{
    switch (state)
    {
    case State::Cocked:
        // TODO:
        if (config_AllowAllActionsAlways)
        {
            Fire();
            
            // block fire button from reaching the game
            return true;
        }
        break;
    case State::Loaded:
        Fire();
        return true;
    }
    return false;
}

void VirtualCrossbow::Fire()
{
    // play sound

    Fire::ArrowFromPoint(RE::PlayerCharacter::GetSingleton(), getThisWeaponNode()->world,
                         RE::PlayerCharacter::GetSingleton()->GetEquippedObject(_hand)->As<RE::TESObjectWEAP>(),
                         RE::PlayerCharacter::GetSingleton()->GetCurrentAmmo());
    // queue up animation
    animator.AddAnimation(standard_reload, 0.0);
}

void VirtualCrossbow::FireDry()
{
    // play sound

    // queue up animation
    animator.AddAnimation(standard_reload, 0.0);
    GotoState(State::Empty);
}

void VirtualCrossbow::CreateOverlapSpheres(uint32_t &PlaceArrow)
{
    using namespace RE;

    // PlaceArrow = VRCR::g_VRManager->CreateLocalOverlapSphere(
    //     0.2f, &transform, _hand ? PapyrusVR::VRDevice_LeftController : PapyrusVR::VRDevice_RightController);
}

void VirtualCrossbow::GotoState(State newstate)
{
    OnExitState();
    prev_state = state;
    state = newstate;
    OnEnterState();
}

void VirtualCrossbow::onUnsheathe()
{
    if (state == State::Sheathed)
    {
        SKSE::log::info("{} crossbow draw", _hand ? "left" : "right");
        GotoState(prev_state);
    }
}

void VirtualCrossbow::onSheathe()
{
    if (state != State::Sheathed)
    {
        GotoState(State::Sheathed);
        SKSE::log::info("{} crossbow sheathe", _hand ? "left" : "right");
    }
}

void VirtualCrossbow::WriteStateToExtraData()
{
}

void VirtualCrossbow::ReadStateFromExtraData()
{
    GotoState(State::Empty);
}

void VirtualCrossbow::OnEnterState()
{
    SKSE::log::info("OnEnterState {:d}", (int)state);
    switch (state)
    {
    case State::Start:
        ReadStateFromExtraData();
        break;
    case State::Sheathed:
        break;
    case State::Empty:
        if (prev_state == State::Start)
        {
            animator.AddAnimation(standard_reload, 0.0);
        }
        break;
    case State::Cocked:
        // can only get here from Empty or Holstered, either way we want to set animation
        animator.AddAnimation(standard_reload, 1.0);
        break;
    case State::Loaded:
        if (prev_state == State::Start)
        {
            // set animation
            animator.AddAnimation(standard_reload, 1.0);

            // set ammo visibility
        }
        break;
    case State::End:
        break;
    }
}

void VirtualCrossbow::OnExitState()
{
    switch (state)
    {
    case State::Start:
        break;
    case State::Sheathed:
        break;
    case State::Empty:
        break;
    case State::Cocked:
        break;
    case State::Loaded:
        break;
    case State::End:
        break;
    }
}

RE::NiPointer<RE::NiNode> VirtualCrossbow::getThisHandNode()
{
    if (_hand)
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCLHnd;
    }
    else
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCRHnd;
    }
}
RE::NiPointer<RE::NiNode> VirtualCrossbow::getOtherHandNode()
{
    if (!_hand)
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCLHnd;
    }
    else
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCRHnd;
    }
}
RE::NiPointer<RE::NiNode> VirtualCrossbow::getThisControllerNode()
{
    if (_hand)
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->LeftValveIndexControllerNode;
    }
    else
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->RightValveIndexControllerNode;
    }
}
RE::NiPointer<RE::NiNode> VirtualCrossbow::getOtherControllerNode()
{
    if (!_hand)
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->LeftValveIndexControllerNode;
    }
    else
    {
        return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->RightValveIndexControllerNode;
    }
}
RE::NiNode *VirtualCrossbow::getThisWeaponNode()
{
    if (_hand)
    {
        auto node = RE::PlayerCharacter::GetSingleton()->GetNodeByName("SHIELD");
        if (node)
        {
            return node->AsNode();
        }
        else
        {
            SKSE::log::info("shield not found");
        }
    }
    else
    {
        auto node = RE::PlayerCharacter::GetSingleton()->GetNodeByName("WEAPON");
        if (node)
        {
            return node->AsNode();
        }
        else
        {
            SKSE::log::info("weapon not found");
        }
    }
    return nullptr;
}
RE::NiNode *VirtualCrossbow::getGrabNode()
{
    auto node = getThisWeaponNode();
    if (node)
    {
        auto AVnode = node->GetObjectByName("GrabNode");
        if (AVnode)
        {
            return AVnode->AsNode();
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}
RE::NiNode *VirtualCrossbow::getLeverRotNode()
{
    auto node = getThisWeaponNode();
    if (node)
    {
        auto AVnode = node->GetObjectByName("CockingMechanismCtrl");
        if (AVnode)
        {
            return AVnode->AsNode();
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}
