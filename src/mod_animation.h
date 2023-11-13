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
        AnimationDefinition(std::string s, double d) : rootNodeName(s), StopTime(d) {}
        std::string rootNodeName;
        double StopTime;
        // animKeyframes keyFrames;
        //  TODO: make keyframes into arrays
        //  std::vector<std::string> bones;
        //  std::vector<std::pair<RE::NiQuaternion, float>> rotate;
        //  std::vector<std::pair<RE::NiPoint3, float>> translate;
    };

    // Created at runtime with parameters of the desired animation type
    struct ActiveAnimation
    {
        ActiveAnimation(std::shared_ptr<AnimationDefinition> p,
                        animType t, double d) : def(p), type(t), StartTime(d) {}
        ActiveAnimation(std::shared_ptr<AnimationDefinition> p,
                        animType t, double d, float *dd, float m, float mm) : def(p), type(t), StartTime(d), driver(dd), min(m), max(mm) {}

        std::shared_ptr<AnimationDefinition> def;

        animType type;
        double StartTime; // represents target time for Static anims
        float *driver;
        float min;
        float max;
    };

    class AnimationProcessor
    {
    public:
        void Update();

        // Timed
        bool AddAnimation(std::string &a);
        // Static- float should be between 0-1
        bool AddAnimation(std::string &a, double scaledTime);
        // Driven- tuple is &driving_value, min_value, max_value
        bool AddAnimation(std::string *a, float *driver, float min, float max);

        void RemoveAnimation(std::string &a);

    private:
        std::unordered_map<std::string, ActiveAnimation> ActiveAnimations;
    };

    class AnimationDataManager
    {
    public:
        bool ReadAnimationsFromFile();
        bool GetAnimationDefinition(std::string &name, std::shared_ptr<AnimationDefinition> out);

        static AnimationDataManager *GetSingleton()
        {
            static AnimationDataManager singleton;
            return &singleton;
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<AnimationDefinition>> AnimationDefinitions;
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
