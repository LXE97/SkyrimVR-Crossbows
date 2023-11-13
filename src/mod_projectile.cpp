#include "mod_projectile.h"
namespace Fire
{
    void GetElevationAzimuth(const RE::NiMatrix3 &rot, float &x, float &z)
    {
        x = -1.0f * std::asin(std::clamp(rot.entry[2][1], -1.0f, 1.0f));

        if (std::abs(rot.entry[2][1]) < 0.9999999f)
        {
            z = -1.0f * std::atan2(-1.0f * rot.entry[0][1], rot.entry[1][1]);
        }
        else
        {
            z = -1.0f * std::atan2(rot.entry[1][0], rot.entry[0][0]);
        }
    }

    RE::BSPointerHandle<RE::Projectile> ArrowFromPoint(RE::Actor *akSource, RE::NiTransform &origin, RE::TESObjectWEAP *weapon, RE::TESAmmo *ammo)
    {
        if (ammo)
        {
            if (auto proj = ammo->GetRuntimeData().data.projectile)
            {
                RE::Projectile::LaunchData ldata;

                float elevation, azimuth;
                GetElevationAzimuth(origin.rotate, elevation, azimuth);

                ldata.origin = origin.translate;
                ldata.contactNormal = {0.0f, 0.0f, 0.0f};
                ldata.projectileBase = proj;
                ldata.shooter = akSource;
                ldata.combatController = akSource->GetActorRuntimeData().combatController;
                ldata.weaponSource = weapon;
                ldata.ammoSource = ammo;
                ldata.angleZ = azimuth;
                ldata.angleX = elevation;
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

                RE::BSPointerHandle<RE::Projectile> handle; // I guess this is for referencing the projectile after it's fired/created
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
            SKSE::log::info("Projectile launch error: invalid projectile");
            return RE::BSPointerHandle<RE::Projectile>();
        }
    }
}