#pragma once
#include <cmath>

namespace Fire
{
    // Determines firing direction from transform with no regard to the object's "roll" or rotation about the barrel axis
    void GetElevationAzimuth(const RE::NiMatrix3 &rot, float &x, float &z);

    // Returns a smart pointer for the resulting projectile, not needed unless you want to do something with it later
    RE::BSPointerHandle<RE::Projectile> ArrowFromPoint(RE::Actor *akSource, RE::NiTransform &origin, RE::TESObjectWEAP *weapon, RE::TESAmmo *ammo);
}