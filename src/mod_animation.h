#pragma once
#pragma warning(disable : 4305) // double to float

#include "Windows.h"

namespace Animation
{
    typedef std::unordered_map<std::string, std::vector<std::pair<RE::NiQuaternion, double>>> animKeyframes;

    enum animType
    {
        Static, // runs once, applies the given keyframe and exits
        Timed,  // runs until StopTime, updates based on current time
        Driven  // driven by an external parameter, must be terminated manually via RemoveAnimation
    };

    // Read from the animations file at startup and stored in the data manager
    struct AnimationDefinition
    {
        AnimationDefinition() = default;
        AnimationDefinition(std::string s, double d, animKeyframes a) : rootNodeName(s), StopTime(d), data(a) {}

        std::string rootNodeName;
        double StopTime;

        animKeyframes data;
    };

    // Created at runtime with parameters of the desired animation type
    struct ActiveAnimation
    {
        ActiveAnimation() = default;
        ActiveAnimation(animType t, AnimationDefinition *a, double d) : type(t), def(a), StartTime(d) {}

        animType type;
        AnimationDefinition *def;
        double StartTime;
    };

    class AnimationProcessor
    {
    public:
        void Update();

        // Timed
        bool AddAnimation(std::string &a);
        void RemoveAnimation(std::string &a);

    private:
        std::unordered_map<std::string, ActiveAnimation> ActiveAnimations;
    };

    class AnimationDataManager
    {
    public:
        bool ReadAnimationsFromFile();
        bool GetAnimationDefinition(std::string &name, AnimationDefinition *&out);

        static AnimationDataManager *GetSingleton()
        {
            static AnimationDataManager singleton;
            return &singleton;
        }

    private:
        std::unordered_map<std::string, AnimationDefinition> AnimationDefinitions;
    };

    inline double GetQPC() noexcept
    {
        LARGE_INTEGER f, i;
        if (QueryPerformanceCounter(&i) && QueryPerformanceFrequency(&f))
        {
            auto frequency = 1.0 / static_cast<double>(f.QuadPart);
            return static_cast<double>(i.QuadPart) * frequency;
        }
        return 0.0;
    }
}
