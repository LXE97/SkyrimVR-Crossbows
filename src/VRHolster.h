#pragma once

namespace vrinput
{
    struct OverlapEvent
    {
        OverlapEvent(int32_t i, bool b, bool bb) : ID(i), entered(b), isLeft(bb) {}
        int32_t ID;
        bool entered;
        bool isLeft;
    };

    using OverlapCallback = void (*)(const OverlapEvent &e);

    class OverlapSphereManager
    {
    public:
        OverlapSphereManager() = default;
        OverlapSphereManager(OverlapCallback cb);

        void Update();

        // If followRotation is true it will move as if a child of the attachNode, if false it will only
        // track the compass rotation (Heading / world Z-axis rotation)
        int32_t Create(RE::NiNode *attachNode, RE::NiPoint3 *localPosition, float radius, bool followRotation = false);
        void Destroy(int32_t targetID);

    private:
        RE::NiPointer<RE::NiNode> controllers[2];

        OverlapCallback _cb;

        struct OverlapSphere
        {
        public:
            OverlapSphere(RE::NiNode *attachNode, RE::NiPoint3 *localPosition, float radius, int32_t ID, bool followRotation = false)
                : attachNode(attachNode), localPosition(localPosition), squaredRadius(radius * radius), ID(ID), followRotation(followRotation) {}

            RE::NiNode *attachNode;
            RE::NiPoint3 *localPosition;
            float squaredRadius;
            bool followRotation;
            int32_t ID;
            bool overlapState[2];
        };

        std::vector<OverlapSphere> spheres;
        int32_t next_ID = 0;
    };
}
