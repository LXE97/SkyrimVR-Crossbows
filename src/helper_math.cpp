#include "helper_math.h"

namespace helper
{
    void RotateZ(RE::NiPoint3 &target, RE::NiMatrix3 &rotator)
    {
        float zangle = helper::GetAzimuth(rotator);
        float cosz = cos(zangle);
        float sinz = sin(zangle);
        target = {cosz * target[0] + sinz * target[1],
                  cosz * target[1] - sinz * target[0],
                  target[2]};
    }

    RE::NiPoint3 GetPalmVectorWS(RE::NiMatrix3 &handRotation, bool isLeft)
    {
        // RE::NiPoint3 palmVectorHandspace = {-0.018, -0.965, 0.261};
        RE::NiPoint3 palmVectorHandspace = {1, 0.0, 0};
        if (isLeft)
            palmVectorHandspace.x *= -1;
        return VectorNormalized(handRotation * palmVectorHandspace);
    }

    RE::NiPoint3 GetThumbVector(RE::NiMatrix3 &handRotation)
    {
        RE::NiPoint3 thumbVectorHandspace = {0.0, 0.0, 1.0};
        return VectorNormalized(handRotation * thumbVectorHandspace);
    }
}
