#include "VRInteractionSphere.h"
#include "helper_math.h"
#include "helper_game.h"

namespace vrinput
{
    OverlapSphereManager::OverlapSphereManager()
    {
        DrawSphereSpell = helper::LookupByName(RE::FormType::Spell, DrawSphereSpellEditorName.c_str())->As<RE::SpellItem>();
        DrawSphereMGEF = helper::LookupByName(RE::FormType::MagicEffect, DrawSphereMGEFEditorName.c_str())->As<RE::EffectSetting>();

        TURNON = new RE::NiColor(0x16ff75);
        TURNOFF = new RE::NiColor(0x00b6ff);
    }

    void OverlapSphereManager::SetOverlapEventHandler(OverlapCallback cb)
    {
        _cb = cb;
    }

    void OverlapSphereManager::Update()
    {
        if (RE::PlayerCharacter::GetSingleton()->Get3D(true))
        {
            controllers[1] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCLHnd;
            controllers[0] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->NPCRHnd;

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

                    // Update visible sphere position
                    RE::NiAVObject *sphereVisNode = nullptr;
                    if (DrawHolsters)
                    {
                        if (sphereVisNode = RE::PlayerCharacter::GetSingleton()->GetNodeByName(DrawNodeName + std::to_string(s.ID)))
                        {
                            if (auto parentNode = RE::PlayerCharacter::GetSingleton()->GetNodeByName(DrawNewParentNode))
                            {
                                RE::NiUpdateData ctx;
                                sphereVisNode->local.translate = parentNode->world.rotate.Transpose() * (sphereWorld - parentNode->world.translate);
                                sphereVisNode->Update(ctx);
                            }
                        }
                    }

                    if (!s.debugNode)
                    {
                        // Test sphere collision
                        bool changed = false;
                        for (bool isLeft : {false, true})
                        {
                            auto dist = sphereWorld.GetSquaredDistance(controllers[isLeft]->world.translate + controllers[isLeft]->world.rotate * palmoffset);
                            if (dist <= s.squaredRadius - hysteresis && !s.overlapState[isLeft])
                            {
                                s.overlapState[isLeft] = true;
                                OverlapEvent e = OverlapEvent(s.ID, true, isLeft);
                                changed = true;
                                if (_cb)
                                {
                                    _cb(e);
                                }
                            }
                            else if (dist > s.squaredRadius + hysteresis && s.overlapState[isLeft])
                            {
                                s.overlapState[isLeft] = false;
                                OverlapEvent e = OverlapEvent(s.ID, false, isLeft);
                                changed = true;
                                if (_cb)
                                {
                                    _cb(e);
                                }
                            }
                        }
                        // reflect both collision states in sphere color
                        if (changed && DrawHolsters && sphereVisNode)
                        {
                            if (s.overlapState[0] || s.overlapState[1])
                            {
                                // set shader color to red
                                SetGlowColor(sphereVisNode, TURNON);
                            }
                            else if (!s.overlapState[0] && !s.overlapState[1])
                            { // set shader color back to green
                                SetGlowColor(sphereVisNode, TURNOFF);
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t OverlapSphereManager::Create(RE::NiNode *attachNode, RE::NiPoint3 localPosition, float radius, bool followRotation, bool debugNode)
    {

        if (attachNode)
        {
            spheres.push_back(OverlapSphere(attachNode, localPosition, radius, next_ID, followRotation, debugNode));
            if (DrawHolsters)
            {
                AddVisibleHolster(next_ID, radius, debugNode);
                Dispel();
            }
            SKSE::log::info("holster created with id {}", next_ID);
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
        if (!DrawHolsters)
        {
            for (auto &s : spheres)
            {
                AddVisibleHolster(s.ID, sqrt(s.squaredRadius), s.debugNode);
            }

            Dispel();
            DrawHolsters = true;
        }
    }

    void OverlapSphereManager::HideHolsterSpheres()
    {
        if (DrawHolsters)
        {
            for (auto &s : spheres)
            {
                DestroyVisibleHolster(s.ID);
            }
            DrawHolsters = false;
        }
    }

    void OverlapSphereManager::AddVisibleHolster(int32_t id, float scale, bool debugNode)
    {
        // each sphere must be added sequentially, and it takes some ms
        std::thread CastSpellAndEditHitArt([this, id, scale, debugNode]()
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

                        auto newparent = player->Get3D(true)->GetObjectByName(DrawNewParentNode);
                        if (newparent)
                        {
                            newparent->AsNode()->AttachChild(holsterNode);
                            holsterNode->local.scale = scale;
                            holsterNode->name = DrawNodeName + std::to_string(id);
                            if (debugNode){  // set to always draw
                                auto geometry = holsterNode->GetFirstGeometryOfShaderType(RE::BSShaderMaterial::Feature::kGlowMap);
                                if (geometry)
                                {
                                    auto shaderProp = geometry->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get();
                                    if (shaderProp)
                                    {
                                        auto shader = netimmerse_cast<RE::BSLightingShaderProperty *>(shaderProp);
                                        if (shader)
                                        {
                                            shader->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kZBufferTest, false);
                                        }
                                    }
                                }
                            }
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

    void OverlapSphereManager::Dispel()
    { // dispel the magic effect that adds holster model
        auto pp = RE::PlayerCharacter::GetSingleton()->GetHandle();
        if (RE::PlayerCharacter::GetSingleton()->GetMagicTarget() &&
            RE::PlayerCharacter::GetSingleton()->GetMagicTarget()->HasMagicEffect(DrawSphereMGEF))
        {
            RE::PlayerCharacter::GetSingleton()->GetMagicTarget()->DispelEffect(DrawSphereSpell, pp);
        }
    }

    void OverlapSphereManager::SetGlowColor(RE::NiAVObject *target, RE::NiColor *c)
    {
        auto geometry = target->GetFirstGeometryOfShaderType(RE::BSShaderMaterial::Feature::kGlowMap);
        if (geometry)
        {
            auto shaderProp = geometry->GetGeometryRuntimeData().properties[RE::BSGeometry::States::kEffect].get();
            if (shaderProp)
            {
                auto shader = netimmerse_cast<RE::BSLightingShaderProperty *>(shaderProp);
                if (shader)
                {
                    shader->emissiveColor = c;
                }
            }
        }
    }
}