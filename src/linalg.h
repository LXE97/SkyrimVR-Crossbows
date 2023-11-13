#pragma once
#include <algorithm>
namespace helper
{
    inline float VectorLengthSquared(const RE::NiPoint3 &vec) { return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z; }
    inline float VectorLength(const RE::NiPoint3 &vec) { return sqrtf(VectorLengthSquared(vec)); }
    inline float dotProduct(const RE::NiPoint3 &x, const RE::NiPoint3 &y) { return x.x * y.x + x.y * y.y + x.z * y.z; }
    inline float DotProductSafe(const RE::NiPoint3 &vec1, const RE::NiPoint3 &vec2) { return std::clamp(dotProduct(vec1, vec2), -1.f, 1.f); }

    inline RE::NiPoint3 VectorNormalized(const RE::NiPoint3 &vec)
    {
        float length = VectorLength(vec);
        return length > 0.0f ? vec / length : RE::NiPoint3();
    }

    RE::NiPoint3 GetPalmVectorWS(RE::NiMatrix3 &handRotation, bool isLeft);

    RE::NiPoint3 GetThumbVector(RE::NiMatrix3 &handRotation);

    inline void Quat2Mat(RE::NiMatrix3 &matrix, RE::NiQuaternion &quaternion)
    {
        float xx = quaternion.x * quaternion.x;
        float xy = quaternion.x * quaternion.y;
        float xz = quaternion.x * quaternion.z;
        float xw = quaternion.x * quaternion.w;

        float yy = quaternion.y * quaternion.y;
        float yz = quaternion.y * quaternion.z;
        float yw = quaternion.y * quaternion.w;

        float zz = quaternion.z * quaternion.z;
        float zw = quaternion.z * quaternion.w;

        matrix.entry[0][0] = 1 - 2 * (yy + zz);
        matrix.entry[0][1] = 2 * (xy - zw);
        matrix.entry[0][2] = 2 * (xz + yw);

        matrix.entry[1][0] = 2 * (xy + zw);
        matrix.entry[1][1] = 1 - 2 * (xx + zz);
        matrix.entry[1][2] = 2 * (yz - xw);

        matrix.entry[2][0] = 2 * (xz - yw);
        matrix.entry[2][1] = 2 * (yz + xw);
        matrix.entry[2][2] = 1 - 2 * (xx + yy);
    }

    // modified from above
    static inline RE::NiMatrix3 slerpQuat(float interp, RE::NiQuaternion q1, RE::NiQuaternion q2)
    {
        // Convert mat1 to a quaternion
        float q1w = q1.w;
        float q1x = q1.x;
        float q1y = q1.y;
        float q1z = q1.z;

        // Convert mat2 to a quaternion
        float q2w = q2.w;
        float q2x = q2.x;
        float q2y = q2.y;
        float q2z = q2.z;

        // Take the dot product, inverting q2 if it is negative
        double dot = q1w * q2w + q1x * q2x + q1y * q2y + q1z * q2z;
        if (dot < 0.0f)
        {
            q2w = -q2w;
            q2x = -q2x;
            q2y = -q2y;
            q2z = -q2z;
            dot = -dot;
        }

        // Linearly interpolate and normalize if the dot product is too close to 1
        float q3w, q3x, q3y, q3z;
        if (dot > 0.9995)
        {
            q3w = q1w + interp * (q2w - q1w);
            q3x = q1x + interp * (q2x - q1x);
            q3y = q1y + interp * (q2y - q1y);
            q3z = q1z + interp * (q2z - q1z);
            float length = sqrtf(q3w * q3w + q3x + q3x + q3y * q3y + q3z * q3z);
            q3w /= length;
            q3x /= length;
            q3y /= length;
            q3z /= length;

            // Otherwise do a spherical linear interpolation normally
        }
        else
        {
            float theta_0 = acosf(dot);                             // theta_0 = angle between input vectors
            float theta = theta_0 * interp;                         // theta = angle between q1 and result
            float sin_theta = sinf(theta);                          // compute this value only once
            float sin_theta_0 = sinf(theta_0);                      // compute this value only once
            float s0 = cosf(theta) - dot * sin_theta / sin_theta_0; // == sin(theta_0 - theta) / sin(theta_0)
            float s1 = sin_theta / sin_theta_0;
            q3w = (s0 * q1w) + (s1 * q2w);
            q3x = (s0 * q1x) + (s1 * q2x);
            q3y = (s0 * q1y) + (s1 * q2y);
            q3z = (s0 * q1z) + (s1 * q2z);
        }

        // Convert the new quaternion back to a matrix
        RE::NiMatrix3 result;
        result.entry[0][0] = 1 - (2 * q3y * q3y) - (2 * q3z * q3z);
        result.entry[0][1] = (2 * q3x * q3y) - (2 * q3z * q3w);
        result.entry[0][2] = (2 * q3x * q3z) + (2 * q3y * q3w);
        result.entry[1][0] = (2 * q3x * q3y) + (2 * q3z * q3w);
        result.entry[1][1] = 1 - (2 * q3x * q3x) - (2 * q3z * q3z);
        result.entry[1][2] = (2 * q3y * q3z) - (2 * q3x * q3w);
        result.entry[2][0] = (2 * q3x * q3z) - (2 * q3y * q3w);
        result.entry[2][1] = (2 * q3y * q3z) + (2 * q3x * q3w);
        result.entry[2][2] = 1 - (2 * q3x * q3x) - (2 * q3y * q3y);
        return result;
    }
}
