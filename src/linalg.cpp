#include "linalg.h"

namespace helper
{
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
