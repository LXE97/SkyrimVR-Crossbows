#include "mod_projectile.h"
#include "helper_math.h"
#include <cmath>

namespace Fire
{

    RE::BSPointerHandle<RE::Projectile> ArrowFromPoint(RE::Actor *akSource, RE::NiTransform &origin, RE::TESObjectWEAP *weapon, RE::TESAmmo *ammo)
    {
        if (ammo)
        {
            if (auto proj = ammo->GetRuntimeData().data.projectile)
            {
                RE::Projectile::LaunchData ldata;

                ldata.origin = origin.translate;
                ldata.contactNormal = {0.0f, 0.0f, 0.0f};
                ldata.projectileBase = proj;
                ldata.shooter = akSource;
                ldata.combatController = akSource->GetActorRuntimeData().combatController;
                ldata.weaponSource = weapon;
                ldata.ammoSource = ammo;
                ldata.angleZ = helper::GetAzimuth(origin.rotate);
                ldata.angleX = helper::GetElevation(origin.rotate);
                ldata.unk50 = nullptr;
                ldata.desiredTarget = nullptr;
                ldata.unk60 = 0.0f;
                ldata.unk64 = 0.0f;
                ldata.parentCell = akSource->GetParentCell();
                ldata.spell = nullptr;
                ldata.castingSource = RE::MagicSystem::CastingSource::kOther;
                ldata.enchantItem = nullptr;
                ldata.poison = nullptr;
                ldata.area = 0;
                ldata.power = 1.0f;
                ldata.scale = 1.0f;
                ldata.alwaysHit = false;
                ldata.noDamageOutsideCombat = false;
                ldata.autoAim = false;
                ldata.useOrigin = true;
                ldata.deferInitialization = false;
                ldata.forceConeOfFire = false;

                RE::BSPointerHandle<RE::Projectile> handle;
                RE::Projectile::Launch(&handle, ldata);
                return handle;
            }
            else
            {
                SKSE::log::info("Projectile launch error: invalid projectile");
                return RE::BSPointerHandle<RE::Projectile>();
            }
        }
        else
        {
            SKSE::log::info("Projectile launch error: invalid ammo");
            return RE::BSPointerHandle<RE::Projectile>();
        }
    }

    RE::BSPointerHandle<RE::Projectile> SpawnProjectile(RE::Actor *akSource, RE::NiPoint3 originpos, RE::BGSProjectile *proj)
    {
        if (proj)
        {RE::Projectile::LaunchData ldata;

                ldata.origin = originpos;
                ldata.contactNormal = {0.0f, 0.0f, 0.0f};
                ldata.projectileBase = proj;
                ldata.shooter = akSource;
                ldata.combatController = akSource->GetActorRuntimeData().combatController;
                ldata.weaponSource = nullptr;
                ldata.ammoSource = nullptr;
                ldata.angleZ = 0;
                ldata.angleX = 0;
                ldata.unk50 = nullptr;
                ldata.desiredTarget = nullptr;
                ldata.unk60 = 0.0f;
                ldata.unk64 = 0.0f;
                ldata.parentCell = akSource->GetParentCell();
                ldata.spell = nullptr;
                ldata.castingSource = RE::MagicSystem::CastingSource::kOther;
                ldata.enchantItem = nullptr;
                ldata.poison = nullptr;
                ldata.area = 0;
                ldata.power = 1.0f;
                ldata.scale = 1.0f;
                ldata.alwaysHit = false;
                ldata.noDamageOutsideCombat = false;
                ldata.autoAim = false;
                ldata.useOrigin = true;
                ldata.deferInitialization = false;
                ldata.forceConeOfFire = false;

                RE::BSPointerHandle<RE::Projectile> handle;
                RE::Projectile::Launch(&handle, ldata);
                return handle;
        }
        else
        {
            SKSE::log::info("Projectile launch error: invalid projectile");
            return RE::BSPointerHandle<RE::Projectile>();
        }
    }
}
