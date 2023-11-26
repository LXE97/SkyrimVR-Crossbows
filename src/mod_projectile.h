#pragma once

namespace Fire
{
    // Returns a smart pointer for the resulting projectile, not needed unless you want to do something with it later
    RE::BSPointerHandle<RE::Projectile> ArrowFromPoint(RE::Actor *akSource, RE::NiTransform &origin, RE::TESObjectWEAP *weapon, RE::TESAmmo *ammo);
    RE::BSPointerHandle<RE::Projectile> SpawnProjectile(RE::Actor *akSource, RE::NiPoint3 originpos, RE::BGSProjectile *proj);
}