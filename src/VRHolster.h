#pragma once

namespace vrinput
{
    struct OverlapEvent
    {
        OverlapEvent(int32_t id, bool entered, bool isLeft) : ID(id), entered(entered), isLeft(isLeft) {}
        int32_t ID;
        bool entered;
        bool isLeft;
    };

    using OverlapCallback = void (*)(const OverlapEvent &e);

    class OverlapSphereManager
    {
    public:
        void Update();
        static OverlapSphereManager *GetSingleton()
        {
            static OverlapSphereManager singleton;
            return &singleton;
        }

        // If followRotation is true it will move as if a child of the attachNode, if false it will only
        // track the compass rotation (Heading / world Z-axis rotation)
        int32_t Create(RE::NiNode *attachNode, RE::NiPoint3 localPosition, float radius, bool followRotation = false);
        void Destroy(int32_t targetID);

        void ShowHolsterSpheres();
        void HideHolsterSpheres();

    private:
        OverlapSphereManager();
        OverlapSphereManager(const OverlapSphereManager &) = delete;
        OverlapSphereManager &operator=(const OverlapSphereManager &) = delete;

        struct OverlapSphere
        {
        public:
            OverlapSphere(RE::NiNode *attachNode, RE::NiPoint3 localPosition, float radius, int32_t ID, bool followRotation = false)
                : attachNode(attachNode), localPosition(localPosition), squaredRadius(radius * radius), ID(ID), followRotation(followRotation) {}

            RE::NiNode *attachNode;
            RE::NiPoint3 localPosition;
            float squaredRadius;
            bool followRotation;
            int32_t ID;
            bool overlapState[2];
        };

        void AddVisibleHolster(int32_t id, float scale);
        void DestroyVisibleHolster(int32_t ID);

        RE::NiPointer<RE::NiNode> controllers[2];
        std::vector<OverlapSphere> spheres;
        OverlapCallback _cb;
        bool DrawHolsters = false;
        int32_t next_ID = 0;

        std::mutex CastSpellMutex;

        RE::SpellItem *DrawSphereSpell;
        RE::EffectSetting *DrawSphereMGEF;
        std::string DrawSphereSpellEditorName = "Z4K_HolsterDebugSphere";
        std::string DrawSphereMGEFEditorName = "Z4K_HolsterDebugSphereMGEF";
        std::string DrawNodeName = "SPHEREATTACH";
        std::string DrawInitialAttachNode = "WEAPON";
        std::string DrawNewParentNode = "NPC Root [Root]";

        inline void Dispel()
        { // dispel the magic effect that adds holster model
            auto pp = RE::PlayerCharacter::GetSingleton()->GetHandle();
            if (RE::PlayerCharacter::GetSingleton()->GetMagicTarget() &&
                RE::PlayerCharacter::GetSingleton()->GetMagicTarget()->HasMagicEffect(DrawSphereMGEF))
            {
                RE::PlayerCharacter::GetSingleton()->GetMagicTarget()->DispelEffect(DrawSphereSpell, pp);
            }
        }
    };
}
