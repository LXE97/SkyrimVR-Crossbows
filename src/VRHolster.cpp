#include "VRHolster.h"
#include "helper_math.h"

namespace vrinput
{

    OverlapSphereManager::OverlapSphereManager(OverlapCallback cb)
    {
        controllers[1] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->LeftCrossbowOffsetNode;
        controllers[0] = RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->RightCrossbowOffsetNode;

        _cb = cb;
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
                    if (s.followRotation)
                    {
                        sphereWorld = s.attachNode->world.translate + s.attachNode->world.rotate * *s.localPosition;
                    }
                    else
                    {
                        RE::NiPoint3 rotated = *s.localPosition;
                        helper::RotateZ(rotated, s.attachNode->world.rotate);
                        sphereWorld = s.attachNode->world.translate + rotated;
                    }

                    for (auto isLeft : {false, true})
                    {
                        if (sphereWorld.GetSquaredDistance(controllers[isLeft]->world.translate) < s.squaredRadius && !s.overlapState[isLeft])
                        {
                            s.overlapState[isLeft] = true;
                            OverlapEvent e = OverlapEvent(s.ID, true, isLeft);
                            _cb(e);
                        }
                        else if (s.overlapState[isLeft])
                        {
                            s.overlapState[isLeft] = false;
                            OverlapEvent e = OverlapEvent(s.ID, false, isLeft);
                            _cb(e);
                        }
                    }
                }
            }
        }
    }

    int32_t OverlapSphereManager::Create(RE::NiNode *attachNode, RE::NiPoint3 *localPosition, float radius, bool followRotation)
    {
        if (attachNode && localPosition)
        {
            spheres.push_back(OverlapSphere(attachNode, localPosition, radius, next_ID, followRotation));
            return next_ID++;
        }
        else
        {
            return -1;
        }
    }

    void OverlapSphereManager::Destroy(int32_t targetID)
    {
        std::erase_if(spheres, [targetID](const OverlapSphere &x)
                      { return (x.ID == targetID); });
    }

}