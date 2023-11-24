#include "VRCR.h"
#include "RE/N/NiRTTI.h"

namespace VRCR
{
    using namespace vrinput;
    using namespace RE;

    // constants
    constexpr FormID playerID = 0x14;

    // engine constants
    BGSEquipSlot *equipRight;
    BGSEquipSlot *equipBoth;

    uint8_t thisPluginID;
    PlayerCharacter *g_player;
    TESAmmo *g_ammoToGrab;
    SKSE::detail::SKSETaskInterface *g_task;
    OpenVRHookManagerAPI *g_OVRHookManager;
    PapyrusVR::VRManagerAPI *g_VRManager;
    vr::TrackedDeviceIndex_t l_controller;
    vr::TrackedDeviceIndex_t r_controller;
    VirtualCrossbow *Crossbows[2] = {nullptr, nullptr};
    bool g_isVrikPresent;

    // TODO debug section
    vrinput::OverlapSphereManager coolguy;
    std::string debugHolsterNodeName = "DEBUGDRAWSPHERE";
    BGSProjectile *debugProjForm;
    float debugRadius = 1;
    NiPoint3 debugHolsterPos = {-4, 15, -40};
    int32_t debugHolsterSphereHndl;
    BSPointerHandle<Projectile> debugHolster3DHndl;
    auto TURNON = new NiColor(0xFF0000);
    auto TURNOFF = new NiColor(0x00FF00);
    FormID debugHolsterProjFormID = 0x00A83D;
    FormID debugHolsterSpell = 0x23D3E;
    Projectile *debugptr;
    NiTransform *debugTransform;
    SpellItem *debugSphereSpell;

    // TODO: configurize temp config section
    NiPoint3 config_SavedAimGrabPosition;
    NiTransform config_SavedAimGrabHandspace;
    float g_SavedAimFingers[5] = {};
    NiPoint3 higgs_palmPosHandspace;
    float g_initialtheta = 0;
    vr::EVRButtonId config_SecondaryBtn = vr::k_EButton_A;
    vr::EVRButtonId config_PrimaryBtn = vr::k_EButton_SteamVR_Trigger;
    std::unordered_map<std::string, double> higgs_ConfigOverride = {
        {"twoHandedHandToHandAlignmentFactor", 0.0},
        {"twoHandedHandToHandShiftFactor", 0.1},
        {"OffhandAffectsTwoHandedRotation", 0},
        {"twoHandedHandToHandRotationFactor", 0.0}};
    auto higgs_SaveConfig = higgs_ConfigOverride;
    bool g_print = false;

    void PreHiggsUpdate()
    {
        if (g_print)
        { 
            // DEBUG : update holster sphere position
            auto targetNode = g_player->Get3D(true)->GetObjectByName("SPHEREATTACH");
            auto destNode = g_player->GetVRNodeData()->LeftWandNode;
            auto srcNode = g_player->Get3D(true)->GetObjectByName("NPC Root [Root]");

            auto translate = destNode->world.translate - srcNode->world.translate;
            translate = srcNode->world.Invert().rotate * translate;
            targetNode->local.translate = translate;

        }
        // coolguy.Update();
    }

    // HIGGS two hand event handlers
    void onWeaponGrabStart()
    {
        for (auto wpn : Crossbows)
        {
            wpn ? wpn->OnGrabStart() : void();
        }
    }

    void onWeaponGrabFinish()
    {
        for (auto wpn : Crossbows)
        {
            wpn ? wpn->OnGrabStop() : void();
        }
    }

    // Button presses

    bool onDEBUGBtnPressA()
    {
        SKSE::log::info("A press ");
        if (!MenuChecker::isGameStopped())
        {

            // DEBUG: show holster sphere
            SKSE::log::info("casting spell");
            auto spell = debugSphereSpell; 
            if (spell)
            {
                auto caster = g_player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
                if (caster)
                {
                    caster->CastSpellImmediate(spell, false, g_player, 1.0, false, 1.0, g_player);
                }
            }
            return true;
        }
        return false;
    }

    bool onDEBUGBtnReleaseB()
    {
        SKSE::log::info("B press ");

        g_print = false;
        vrinput::RemoveCallback(vr::k_EButton_ApplicationMenu, onDEBUGBtnReleaseB, Right, Press, ButtonUp);
        return false;
    }
    bool onDEBUGBtnPressB()
    {
        SKSE::log::info("B press ");
        if (!MenuChecker::isGameStopped())
        {
            g_print = true;
            vrinput::AddCallback(vr::k_EButton_ApplicationMenu, onDEBUGBtnReleaseB, Right, Press, ButtonUp);

            return true;
        }
        return false;
    }

    bool onPrimaryCrossbowButtonPress()
    {
        if (!MenuChecker::isGameStopped())
        {
            SKSE::log::info("fire button pressed");

            // DEBUG section: move holster spheres

            // find sphere node
            auto node = g_player->Get3D(true)->GetObjectByName("SPHEREATTACH");
            if (node)
            {
                // move to target node
                auto root = g_player->Get3D(true)->GetObjectByName("NPC Root [Root]");
                // auto Lhand = g_player->GetVRNodeData()->UprightHmdNode;
                if (root)
                {
                    root->AsNode()->AttachChild(node);
                }
                else
                {
                    SKSE::log::info("attach node not found");
                }
            }

            auto pp = g_player->GetHandle();
            auto eff = TESForm::LookupByID<EffectSetting>(getFullFormID(0x23D3F));
            if (g_player->GetMagicTarget()->HasMagicEffect(eff))
            {
                g_player->GetMagicTarget()->DispelEffect(debugSphereSpell, pp);
                SKSE::log::info("yes its true");
            }

            SKSE::log::info("debug draw sphere over ");

            bool blocking = false;
            for (auto wpn : Crossbows)
            {
                blocking = wpn ? wpn->OnPrimaryButtonPress() : false;
            }
            return blocking;
        }
        return false;
    }

    bool onGrabButtonPress()
    {
        if (!MenuChecker::isGameStopped())
        {
            SKSE::log::info("holster grabbed");
            // we already know we are overlapping
            return true;
        }
        return false;
    }

    bool onGrabButtonRelease()
    {
        if (!MenuChecker::isGameStopped())
        {
            SKSE::log::info("holster released");

            // get the difference from initial position to current position

            // copy new transform
            return true;
        }
        return false;
    }

    bool onSecondaryBtnPress()
    {
        if (!MenuChecker::isGameStopped())
        {
            return true;
        }
        return false;
    }

    bool onHolsterBtnPress()
    {
        if (!MenuChecker::isGameStopped())
        {
            if (g_higgsInterface->CanGrabObject(Left))
            {
                SKSE::log::info("LA grab ok");
                auto ammoToGrab = g_player->GetCurrentAmmo();
                if (ammoToGrab)
                {
                    RE::NiPoint3 droploc = g_player->GetVRNodeData()->LeftWandNode->world.translate;
                    // auto droppedAmmoHandle = g_player->DropObject(g_ammoToGrab, nullptr, 1, &droploc); //, g_player->GetVRNodeData()->LeftWandNode->world.translate);
                    auto droppedHandle = g_player->RemoveItem(ammoToGrab, 1, ITEM_REMOVE_REASON::kDropping, nullptr, nullptr, &droploc);
                    if (droppedHandle)
                    {
                        SKSE::log::info("dropped ammo");
                        g_higgsInterface->GrabObject(droppedHandle.get().get(), Left);
                    }
                }
            }
            else
            {
                SKSE::log::info("no LA grab bad");
            }
            return true;
        }
        return false;
    }

    void onWeaponDraw(const SKSE::ActionEvent *event)
    {
        if (g_player && g_player->As<TESObjectREFR>() == event->actor)
        {
            if (event->type == SKSE::ActionEvent::Type::kBeginDraw || event->type == SKSE::ActionEvent::Type::kEndDraw)
            {
                for (auto wpn : Crossbows)
                {
                    wpn ? wpn->onUnsheathe() : void();
                }
            }

            else if (event->type == SKSE::ActionEvent::Type::kBeginSheathe || event->type == SKSE::ActionEvent::Type::kEndSheathe)
            {
                for (auto wpn : Crossbows)
                {
                    wpn ? wpn->onSheathe() : void();
                }
            }
        }
    }

    void onEquipEvent(const TESEquipEvent *event)
    {
        SKSE::log::info("equip event: getting actor");

        if (g_player && g_player == event->actor.get())
        {
            SKSE::log::info("equip event: looking up formid");
            auto item = TESForm::LookupByID(event->baseObject);
            if (item && item->IsWeapon() && item->As<TESObjectWEAP>()->IsCrossbow())
            {
                SKSE::log::info("equip event: item is crossbow");
                if (event->equipped)
                {

                    // find out which hand just equipped it
                    bool hand;
                    if (g_player->GetEquippedObject(Right))
                    {
                        // it could have been left hand, unless virtual crossbow already exists
                        if (Crossbows[Right])
                        {
                            hand = Left;
                        }
                        else
                        {
                            hand = Right;
                        }
                    }
                    else
                    {
                        hand = Left;
                    }

                    // TODO: get extra data
                    SKSE::log::info("equip event: crossbow equipped in {} hand", hand);
                    Crossbows[hand] = new VirtualCrossbow(event->baseObject, hand);
                }
                else
                {

                    // find out which hand unequipped
                    if (Crossbows[Right] && g_player->GetEquippedObject(Right) == nullptr)
                    {
                        SKSE::log::info("equip event: crossbow unequippped right");
                        delete Crossbows[Right];
                        Crossbows[Right] = nullptr;
                    }
                    else if (Crossbows[Left] && g_player->GetEquippedObject(Left) == nullptr)
                    {
                        SKSE::log::info("equip event: crossbow unequippped left");
                        delete Crossbows[Left];
                        Crossbows[Left] = nullptr;
                    }
                }
            }
        }
    }

    void SetFavoriteGrabPosition()
    { /*
         if (g_higgsInterface->IsTwoHanding())
         {
             SKSE::log::info("saving grab transform");
             SavedAimGrabHandspace = g_higgsInterface->GetGrabTransform(true);
             auto weaponNode = g_player->GetVRNodeData()->NPCRHnd->GetObjectByName(rightWeaponNode);
             auto LHand = g_player->GetVRNodeData()->NPCLHnd;
             NiPoint3 weaponToHand = LHand->world.translate - weaponNode->world.translate;
             SavedAimGrabPosition = weaponToHand;
             g_higgsInterface->GetFingerValues(true, g_SavedAimFingers);
         }
         else
         {
             SKSE::log::info("resetting grab transform");
             SavedAimGrabHandspace.scale = 0;
         }*/
    }

    // TODO: Watch the player inventory and swaps any crossbows with a duplicate that's set to one handed, for player use only.
    void onContainerChange(const TESContainerChangedEvent *event)
    {
        SKSE::log::info("container event: {}", event->newContainer);
        if (!(g_player) || !(g_player)->loadedData)
        {
            return;
        }
        if (event->newContainer == playerID)
        { // item added to player
            SKSE::log::info("player received item: {:x}", event->baseObj);
            auto eventitem = TESForm::LookupByID(event->baseObj);
            if (eventitem && eventitem->IsWeapon() && eventitem->As<TESObjectWEAP>()->IsCrossbow())
            {
                // Offset::TESDescription::GetDescription;
                //  g_player->GetContainer()->RemoveObjectFromContainer(eventitem->As<TESBoundObject>(), event->itemCount);
            }
        }
        else if (event->oldContainer == playerID)
        { // item removed from player
            SKSE::log::info("player lost item: {:X} new container: {}", event->baseObj, event->newContainer);
            if (event->reference && event->reference.get())
            {
                SKSE::log::info("item: {:X}", event->reference.get()->formID);
            }

            // TODO : get the extra data from the dupe

            /*
            auto f = event->reference.get()->extraList;
            if (f)
            {
                auto enchant = f.GetByType(ExtraDataType::kEnchantment);
                if (enchant)
                {

                }

                // lookup what the dupe was referencing
                // extradata = event->baseObj
                // TESForm::LookupByID(extradata);
                // remove the fake crossbow from the target's inventory and add a real one
                // g_player->GetContainer()->AddObjectToContainer(eventitem->As<TESBoundObject>(), event->itemCount);

            }
            */
            if (event->newContainer)
            {
                // add new item
                // TESForm::LookupByID<TESContainer>(event->newContainer)->AddObjectToContainer(TESForm::LookupByID<TESBoundObject>(swappa), event->itemCount, g_player);
            }
        }
    }

    void OnOverlap(const OverlapEvent &e)
    {
        if (e.ID == debugHolsterSphereHndl && e.isLeft)
        {
            if (e.entered)
            {
                SKSE::log::info("HOLSTER : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
                vrinput::AddCallback(config_SecondaryBtn, onHolsterBtnPress, Left, Press, ButtonDown);
                vrinput::AddCallback(vr::k_EButton_ApplicationMenu, onGrabButtonPress, Right, Press, ButtonDown);
            }
            else
            {
                SKSE::log::info("HOLSTER :                  000000000000");
                vrinput::RemoveCallback(config_SecondaryBtn, onHolsterBtnPress, Left, Press, ButtonDown);
                vrinput::RemoveCallback(vr::k_EButton_ApplicationMenu, onGrabButtonPress, Right, Press, ButtonDown);
            }
        }
        else
        {
            for (auto wpn : Crossbows)
            {
                wpn ? wpn->OnOverlap(e) : void();
            }
        }
    }

    void OverrideHiggsConfig()
    {
        for (auto &[key, value] : higgs_ConfigOverride)
        {
            g_higgsInterface->SetSettingDouble(key, value);
        }
    }

    void RestoreHiggsConfig()
    {
        for (auto &[key, value] : higgs_SaveConfig)
        {
            bool set = g_higgsInterface->SetSettingDouble(key, value);
            if (!set)
            {
                SKSE::log::info("setting failed!");
            }
        }
    }

    void GetExtraData()
    {
        /*

        // get player inventory extra data
        InventoryEntryData* curEntryData = pContainerChanges->data->FindItemEntry(form);

        if (curEntryData)
        {
        ExtendDataList* curExtendDataList = curEntryData->extendDataList;

        if (curExtendDataList)
        {
        extraStuffCount = curExtendDataList->Count();

        for (UInt32 m = 0; m < curExtendDataList->Count(); m++)
        {
        BaseExtraList* itemExtraData = curExtendDataList->GetNthItem(m);
        if (itemExtraData != nullptr)
        {
        if (form->formType == kFormType_Armor)
        {
        ExtraEnchantment* extraEnchant = static_cast<ExtraEnchantment*>(itemExtraData->GetByType(kExtraData_Enchantment));
        ExtraHealth* xHealth = static_cast<ExtraHealth*>(itemExtraData->GetByType(kExtraData_Health));
        ExtraTextDisplayData* xTextDisplayData = static_cast<ExtraTextDisplayData*>(itemExtraData->GetByType(kExtraData_TextDisplayData));

        */
    }

    void StartMod()
    {
        SKSE::log::info("StartMod entry");

        // constant runtime formID references
        equipRight = TESForm::LookupByID<BGSEquipSlot>(0x13f42);
        equipBoth = TESForm::LookupByID<BGSEquipSlot>(0x13f45);
        debugProjForm = TESForm::LookupByID<BGSProjectile>(getFullFormID(debugHolsterProjFormID));
        debugSphereSpell = TESForm::LookupByID<SpellItem>(getFullFormID(debugHolsterSpell));

        // VR init
        l_controller = g_OVRHookManager->GetVRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
        r_controller = g_OVRHookManager->GetVRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
        RegisterVRInputCallback();

        Animation::AnimationDataManager::GetSingleton()->ReadAnimationsFromFile();

        // Register MenuOpenCloseEvent handler
        MenuChecker::begin();

        // HIGGS setup
        if (g_higgsInterface)
        {
            SKSE::log::info("adding HIGGS callback");
            g_higgsInterface->AddStartTwoHandingCallback(onWeaponGrabStart);
            g_higgsInterface->AddStopTwoHandingCallback(onWeaponGrabFinish);
            g_higgsInterface->AddPostVrikPreHiggsCallback(PreHiggsUpdate);

            // TODO: read this from our config
            higgs_palmPosHandspace = {0, -2.4, 6};

            higgs_SaveConfig = higgs_ConfigOverride;
            SKSE::log::info("initial HIGGS settings:");
            for (auto &[key, value] : higgs_SaveConfig)
            {
                bool read = g_higgsInterface->GetSettingDouble(key, value);
                SKSE::log::info("{} : {}", key, higgs_SaveConfig[key]);
            }
        }

        // register event sinks and handlers
        auto SKSEActionEventSink = EventSink<SKSE::ActionEvent>::GetSingleton();
        SKSE::GetActionEventSource()->AddEventSink(SKSEActionEventSink);
        SKSEActionEventSink->AddCallback(onWeaponDraw);

        auto containerSink = EventSink<TESContainerChangedEvent>::GetSingleton();
        ScriptEventSourceHolder::GetSingleton()->AddEventSink(containerSink);
        containerSink->AddCallback(onContainerChange);

        auto equipSink = EventSink<TESEquipEvent>::GetSingleton();
        ScriptEventSourceHolder::GetSingleton()->AddEventSink(equipSink);
        equipSink->AddCallback(onEquipEvent);

        // coolguy = vrinput::OverlapSphereManager(&OnOverlap);
        // coolguy.Create(g_player->GetVRNodeData()->UprightHmdNode.get(), &debugHolsterPos, &debugRadius);

        vrinput::AddCallback(vr::k_EButton_A, onDEBUGBtnPressA, Right, Press, ButtonDown);
        vrinput::AddCallback(vr::k_EButton_ApplicationMenu, onDEBUGBtnPressB, Right, Press, ButtonDown);
        vrinput::AddCallback(vr::k_EButton_SteamVR_Trigger, onPrimaryCrossbowButtonPress, Right, Press, ButtonDown);
    }

    void GameLoad()
    {
        g_player = PlayerCharacter::GetSingleton();
    }

    void PreGameLoad()
    {
    }

    //  TODO: move this into vrinput files
    // handles low level button/trigger events
    bool ControllerInput_CB(vr::TrackedDeviceIndex_t unControllerDeviceIndex, const vr::VRControllerState_t *pControllerState, uint32_t unControllerStateSize, vr::VRControllerState_t *pOutputControllerState)
    {
        // save last controller input to only do processing on button changes
        static uint64_t prev_Pressed[2] = {};
        static uint64_t prev_Touched[2] = {};

        // need to remember the last output sent to the game in order to maintain input blocking without calling our game logic every packet
        static uint64_t prev_Pressed_out[2] = {};
        static uint64_t prev_Touched_out[2] = {};

        if (pControllerState && !MenuChecker::isGameStopped())
        {
            bool isLeft = unControllerDeviceIndex == l_controller;
            if (isLeft || unControllerDeviceIndex == r_controller)
            {
                uint64_t pressedChange = prev_Pressed[isLeft] ^ pControllerState->ulButtonPressed;
                uint64_t touchedChange = prev_Touched[isLeft] ^ pControllerState->ulButtonTouched;
                if (pressedChange)
                {
                    vrinput::processButtonChanges(pressedChange, pControllerState->ulButtonPressed, isLeft, false, pOutputControllerState);
                    prev_Pressed[isLeft] = pControllerState->ulButtonPressed;
                    prev_Pressed_out[isLeft] = pOutputControllerState->ulButtonPressed;
                }
                else
                {
                    pOutputControllerState->ulButtonPressed = prev_Pressed_out[isLeft];
                }
                if (touchedChange)
                {
                    vrinput::processButtonChanges(touchedChange, pControllerState->ulButtonTouched, isLeft, true, pOutputControllerState);
                    prev_Touched[isLeft] = pControllerState->ulButtonTouched;
                    prev_Touched_out[isLeft] = pOutputControllerState->ulButtonTouched;
                }
                else
                {
                    pOutputControllerState->ulButtonTouched = prev_Touched_out[isLeft];
                }
            }
        }
        return true;
    }

    // Register SkyrimVRTools callback
    void RegisterVRInputCallback()
    {
        if (g_OVRHookManager->IsInitialized())
        {
            g_OVRHookManager = RequestOpenVRHookManagerObject();
            if (g_OVRHookManager)
            {
                SKSE::log::info("Successfully requested OpenVRHookManagerAPI.");
                // InitSystem(g_OVRHookManager->GetVRSystem()); required for haptic triggers, set up later

                g_OVRHookManager->RegisterControllerStateCB(ControllerInput_CB);
            }
        }
    }
}
