#include "mod_animation.h"
#include "VRCR.h"

namespace Animation
{

    bool AnimationProcessor::AddAnimation(std::string &a)
    {
        SKSE::log::info("add animation: {}", a);
        AnimationDefinition *def = nullptr;
        if (AnimationDataManager::GetSingleton()->GetAnimationDefinition(a, def) && def)
        {

            ActiveAnimations.insert(std::make_pair(a, ActiveAnimation(animType::Timed, def, GetQPC())));
            return true;
        }
        // ActiveAnimations[*a] = std::make_unique<ActiveAnimation>(desiredAnim, animType::Timed, GetQPC());

        return false;
    }

    void AnimationProcessor::RemoveAnimation(std::string &a)
    {
        auto it = ActiveAnimations.find(a);
        if (it != ActiveAnimations.end())
        {
            ActiveAnimations.erase(it);
        }
    }

    bool AnimationDataManager::ReadAnimationsFromFile()
    {
        static bool runOnce = true;
        if (runOnce)
        {
            animKeyframes b = {
                {"CrossBowBone_R01", {{{-0.06304830, 0, 0, 0.99801}, 0.0}, {{-0.157982, 0, 0, 0.987442}, 1.0}}},
                {"CrossBowBone_R02", {{{0.9966588, 0, 0, 0.0816776}, 0.0}, {{0.984319, 0, 0, 0.1763976}, 1.0}}},
                {"StringR", {{{0.144275, -0.0376906, 0.0054983, 0.9888043}, 0.0}, {{0.43479, -0.0375817, 0.0181629, 0.899564}, 1.0}}},
                {"CrossBowBone_L01", {{{0, 0.998009, -0.0630725, 0}, 0.0}, {{0, 0.987445, -0.157964, 0}, 1.0}}},
                {"CrossBowBone_L02", {{{0.996659, 0, 0, 0.0816809}, 0.0}, {{0.984326, 0, 0, 0.176359}, 1.0}}},
                {"StringL", {{{-0.0376816, 0.144283, 0.988804, 0.00549792}, 0.0}, {{-0.0375721, 0.434792, 0.899563, 0.0181594}, 1.0}}}};

            // TODO: put in a file
            AnimationDefinitions["standard_reload"] = AnimationDefinition("CrossbowRoot", 1.0, b);
            runOnce = false;
            return true;
        }
        return false;
    }

    bool AnimationDataManager::GetAnimationDefinition(std::string &name, AnimationDefinition *&out)
    {
        auto it = AnimationDefinitions.find(name);
        if (it != AnimationDefinitions.end())
        {
            SKSE::log::info("anim found");
            out = &(it->second);
            return true;
        }
        SKSE::log::info("anim not found");
        return false;
    }

    void AnimationProcessor::Update()
    {

        double curTime = GetQPC();
        for (auto &[key, a] : ActiveAnimations)
        {
            if (!(a.def))
            {
                return;
            }

            auto RootNode = VRCR::g_player->GetNodeByName(a.def->rootNodeName);
            if (!RootNode)
                return;

            double targetTime = 0;
            SKSE::log::info("switching");
            switch (a.type)
            {
            case animType::Static:
                targetTime = a.StartTime * a.def->StopTime;
                break;
            case animType::Timed:
                targetTime = curTime - a.StartTime;
                break;
            case animType::Driven:
                //if (!a.driver)
                    return;
                //else
                //{
                //    targetTime = (a.max - *a.driver) / (a.max - a.min);
                //}
            }
            // flag for later use
            bool finished = true;
            // animate each bone
            SKSE::log::info("iterating bones");
            for (auto bone : a.def->data)
            {
                auto node = RootNode->GetObjectByName(bone.first);
                if (node)
                {
                    auto ninode = node->AsNode();
                    auto keys = bone.second;
                    auto prevFrame = keys.front();
                    // Find which 2 frames the targetTime falls between. We skip the first frame and stop as soon as we hit a frame
                    // with a time above the targetTime. If none are found, then we kill the animation
                    for (int i = 1; i < keys.size(); i++)
                    {
                        if (keys[i].second >= targetTime)
                        {
                            auto ctx = RE::NiUpdateData();
                            RE::NiMatrix3 rotate = helper::slerpQuat(targetTime, prevFrame.first, keys[i].first);
                            ninode->local.rotate = rotate;
                            ninode->Update(ctx);

                            finished = false;
                            break;
                        }
                        prevFrame = keys[i];
                    }
                }
            }

            if (finished)
            {
                ActiveAnimations.erase(key);
            }

            double doneTime = GetQPC();
            SKSE::log::info("time spent animating {}: {}", key, doneTime - curTime);
        }
    }

}