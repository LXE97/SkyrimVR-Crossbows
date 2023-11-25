#include "VRHolster.h"
#include "helper_math.h"
#include "helper_game.h"

namespace vrinput
{
    OverlapSphereManager::OverlapSphereManager()
    {
        controllers[1] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->LeftWandNode;
        controllers[0] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->RightWandNode;

        DrawSphereSpell = helper::LookupByName(RE::FormType::Spell, DrawSphereSpellEditorName.c_str())->As<RE::SpellItem>();
        DrawSphereMGEF = helper::LookupByName(RE::FormType::MagicEffect, DrawSphereMGEFEditorName.c_str())->As<RE::EffectSetting>();
    }

    void OverlapSphereManager::Update()
    {
        if (RE::PlayerCharacter::GetSingleton()->GetVRNodeData())
        {
            static RE::NiPoint3 sphereWorld;
            for (auto &s : spheres)
            {
                if (s.attachNode)
                {
                    // Update sphere position
                    if (s.followRotation)
                    {
                        sphereWorld = s.attachNode->world.translate + s.attachNode->world.rotate * s.localPosition;
                    }
                    else
                    {
                        RE::NiPoint3 rotated = s.localPosition;
                        helper::RotateZ(rotated, s.attachNode->world.rotate);
                        sphereWorld = s.attachNode->world.translate + rotated;
                    }

                    RE::NiAVObject *sphereVisNode = nullptr;
                    // Update visible sphere position
                    if (DrawHolsters)
                    {
                        if (sphereVisNode = RE::PlayerCharacter::GetSingleton()->GetNodeByName(DrawNodeName + std::to_string(s.ID)))
                        {
                            if (auto parentNode = RE::PlayerCharacter::GetSingleton()->GetNodeByName(DrawNewParentNode))
                            {
                                sphereVisNode->local.translate = parentNode->world.rotate.Transpose() * (sphereWorld - parentNode->world.translate);
                                SKSE::log::info("local: {} {} {} ", sphereVisNode->local.translate.x, sphereVisNode->local.translate.y, sphereVisNode->local.translate.z);
                            }
                        }
                        else
                        {
                            SKSE::log::info("vis node not found: {}", DrawNodeName + std::to_string(s.ID));
                        }
                    }

                    // Test sphere collision
                    for (auto isLeft : {false, true})
                    {
                        if (sphereWorld.GetSquaredDistance(controllers[isLeft]->world.translate) < s.squaredRadius && !s.overlapState[isLeft])
                        {
                            s.overlapState[isLeft] = true;
                            OverlapEvent e = OverlapEvent(s.ID, true, isLeft);
                            //_cb(e);
                            if (sphereVisNode)
                            {
                                // set shader color to red
                            }
                        }
                        else if (s.overlapState[isLeft])
                        {
                            s.overlapState[isLeft] = false;
                            OverlapEvent e = OverlapEvent(s.ID, false, isLeft);
                            //_cb(e);
                            if (sphereVisNode)
                            {
                                // set shader color to green
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t OverlapSphereManager::Create(RE::NiNode *attachNode, RE::NiPoint3 localPosition, float radius, bool followRotation)
    {

        if (attachNode)
        {
            spheres.push_back(OverlapSphere(attachNode, localPosition, radius, next_ID, followRotation));
            if (DrawHolsters)
            {
                AddVisibleHolster(next_ID, radius);
                Dispel();
            }
            SKSE::log::info("holster created");
            return next_ID++;
        }
        else
        {
            SKSE::log::info("holster failed");
            return -1;
        }
    }

    void OverlapSphereManager::Destroy(int32_t targetID)
    {
        if (DrawHolsters)
        {
            DestroyVisibleHolster(targetID);
        }

        // delete virtual holster
        std::erase_if(spheres, [targetID](const OverlapSphere &x)
                      { return (x.ID == targetID); });
    }

    void OverlapSphereManager::ShowHolsterSpheres()
    {
        // TODO add dummy spheres to visualize the hand collision

        for (auto &s : spheres)
        {
            SKSE::log::info("drawing holster sphere");
            AddVisibleHolster(s.ID, sqrt(s.squaredRadius));
        }

        Dispel();
        DrawHolsters = true;
    }

    void OverlapSphereManager::HideHolsterSpheres()
    {
        for (auto &s : spheres)
        {
            DestroyVisibleHolster(s.ID);
        }
        DrawHolsters = false;
    }

    void OverlapSphereManager::AddVisibleHolster(int32_t id, float scale)
    {
        // each sphere must be added sequentially, and it takes some ms
        std::thread CastSpellAndEditHitArt([this, id, scale]()
                                           {
        std::lock_guard<std::mutex> lock(CastSpellMutex);
        if (DrawSphereSpell)
        {
            auto player = RE::PlayerCharacter::GetSingleton();
            if (player){
                auto caster = player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
                if (caster)
                {
                    caster->CastSpellImmediate(DrawSphereSpell, false, player, 1.0, false, 1.0, player);

                    RE::NiAVObject *holsterNode;
                    int sleeps = 0;
                    do
                    {
                        std::this_thread::sleep_for(20ms);
                        holsterNode = player->GetNodeByName(DrawNodeName);
                        SKSE::log::info("waiting for node {}", id);
                    } while (!holsterNode && sleeps++ < 100);
                    if (holsterNode){
                        std::this_thread::sleep_for(20ms);
                        // by attaching the node to a different parent, we break the link to the magic effect and it becomes persistent
                        auto newparent = player->Get3D(true)->GetObjectByName(DrawNewParentNode);
                        if (newparent)
                        {
                            newparent->AsNode()->AttachChild(holsterNode);
                            holsterNode->local.scale = 10.f;
                            holsterNode->name = DrawNodeName + std::to_string(id);
                        }
                    }
                }
            }
        } else {
            SKSE::log::info("holster visible failed");
        } });

        // technically this is not good because the mutex can be destroyed while in use if the OverlapSphereManager goes out of scope, but it's a singleton that lives as long as Skyrim.exe
        CastSpellAndEditHitArt.detach();
    }

    void OverlapSphereManager::DestroyVisibleHolster(int32_t id)
    {
        if (auto sphereVisNode = RE::PlayerCharacter::GetSingleton()->GetNodeByName(DrawNodeName + std::to_string(id)))
        {
            sphereVisNode->parent->DetachChild2(sphereVisNode);
        }
    }
}