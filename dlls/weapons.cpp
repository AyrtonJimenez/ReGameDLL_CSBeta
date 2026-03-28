#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"

extern CGraph	WorldGraph;
extern int gEvilImpulse101;

#define NOT_USED 255

DLL_GLOBAL	short	g_sModelIndexLaser;
DLL_GLOBAL  const char* g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short	g_sModelIndexLaserDot;
DLL_GLOBAL	short	g_sModelIndexFireball;
DLL_GLOBAL	short	g_sModelIndexSmoke;
DLL_GLOBAL	short	g_sModelIndexWExplosion;
DLL_GLOBAL	short	g_sModelIndexBubbles;
DLL_GLOBAL	short	g_sModelIndexBloodDrop;
DLL_GLOBAL	short	g_sModelIndexBloodSpray;

DLL_GLOBAL	short	g_sModelIndexSmokePuff;
DLL_GLOBAL	short	g_sModelIndexFireball2;
DLL_GLOBAL	short	g_sModelIndexFireball3;
DLL_GLOBAL	short	g_sModelIndexFireball4;
DLL_GLOBAL	short	g_sModelIndexRadio;
DLL_GLOBAL	short	g_sModelIndexCTGhost;
DLL_GLOBAL	short	g_sModelIndexTGhost;
DLL_GLOBAL	short	g_sModelIndexC4Glow;

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

extern int gmsgCurWeapon;
extern int gmsgReloadSound;

#define MAX_DIST_RELOAD_SOUND 512.0f

MULTIDAMAGE gMultiDamage;

#define TRACER_FREQ		4			

int MaxAmmoCarry(int iszName)
{
    for (int i = 0; i < MAX_WEAPONS; i++)
    {
        if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
            return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
        if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
            return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
    }

    ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
    return -1;
}

void ClearMultiDamage(void)
{
    gMultiDamage.pEntity = NULL;
    gMultiDamage.amount = 0;
    gMultiDamage.type = 0;
}

void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker)
{
    Vector		vecSpot1;
    Vector		vecDir;
    TraceResult	tr;

    if (!gMultiDamage.pEntity)
        return;

    gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type);
}

void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType)
{
    if (!pEntity)
        return;

    gMultiDamage.type |= bitsDamageType;

    if (pEntity != gMultiDamage.pEntity)
    {
        ApplyMultiDamage(pevInflictor, pevInflictor);
        gMultiDamage.pEntity = pEntity;
        gMultiDamage.amount = 0;
    }

    gMultiDamage.amount += flDamage;
}

void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
    UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

int DamageDecal(CBaseEntity* pEntity, int bitsDamageType)
{
    if (pEntity)
        return pEntity->DamageDecal(bitsDamageType);

    return RANDOM_LONG(DECAL_GUNSHOT4, DECAL_GUNSHOT5);
}

void DecalGunshot(TraceResult* pTrace, int iBulletType, bool bClientOnly, entvars_t* pShooter, bool bHitMetal)
{
    if (!UTIL_IsValidEntity(pTrace->pHit))
        return;

    if (VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP)
    {
        CBaseEntity* pEntity = NULL;
        if (!FNullEnt(pTrace->pHit))
            pEntity = CBaseEntity::Instance(pTrace->pHit);

        int decalIndex;
        if (bHitMetal)
        {
            ALERT(at_console, "Hit Metal!!\n");
            decalIndex = RANDOM_LONG(0, 2);
        }
        else
        {
            decalIndex = DamageDecal(pEntity, DMG_BULLET);
        }

        switch (iBulletType)
        {
        case BULLET_PLAYER_CROWBAR:
            UTIL_DecalTrace(pTrace, DamageDecal(pEntity, DMG_CLUB));
            break;
        default:
            UTIL_GunshotDecalTrace(pTrace, decalIndex, bClientOnly, pShooter);
            break;
        }
    }
}

#if 0
void ExplodeModel(const Vector& vecOrigin, float speed, int model, int count)
{
    MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
    WRITE_BYTE(TE_EXPLODEMODEL);
    WRITE_COORD(vecOrigin.x);
    WRITE_COORD(vecOrigin.y);
    WRITE_COORD(vecOrigin.z);
    WRITE_COORD(speed);
    WRITE_SHORT(model);
    WRITE_SHORT(count);
    WRITE_BYTE(15);
    MESSAGE_END();
}
#endif

int giAmmoIndex = 0;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname)
{
    for (int i = 0; i < MAX_AMMO_SLOTS; i++)
    {
        if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
            continue;

        if (stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0)
            return;
    }

    giAmmoIndex++;
    ASSERT(giAmmoIndex < MAX_AMMO_SLOTS);
    if (giAmmoIndex >= MAX_AMMO_SLOTS)
        giAmmoIndex = 0;

    CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
    CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;
}

void UTIL_PrecacheOtherWeapon(const char* szClassname)
{
    edict_t* pent;

    pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));
    if (FNullEnt(pent))
    {
        ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
        return;
    }

    CBaseEntity* pEntity = CBaseEntity::Instance(VARS(pent));

    if (pEntity)
    {
        ItemInfo II;
        pEntity->Precache();
        memset(&II, 0, sizeof II);
        if (((CBasePlayerItem*)pEntity)->GetItemInfo(&II))
        {
            CBasePlayerItem::ItemInfoArray[II.iId] = II;

            if (II.pszAmmo1 && *II.pszAmmo1)
            {
                AddAmmoNameToAmmoRegistry(II.pszAmmo1);
            }

            if (II.pszAmmo2 && *II.pszAmmo2)
            {
                AddAmmoNameToAmmoRegistry(II.pszAmmo2);
            }

            memset(&II, 0, sizeof II);
        }
    }

    REMOVE_ENTITY(pent);
}

void UTIL_PrecacheOtherWeapon2(const char* szClassname)
{
    edict_t* pent;

    pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));
    if (FNullEnt(pent))
    {
        ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
        return;
    }

    CBaseEntity* pEntity = CBaseEntity::Instance(VARS(pent));

    if (pEntity)
    {
        ItemInfo II;
        pEntity->Precache();
        memset(&II, 0, sizeof II);
        if (((CBasePlayerItem*)pEntity)->GetItemInfo(&II))
        {
            CBasePlayerItem::ItemInfoArray[II.iId] = II;

            if (II.pszAmmo1 && *II.pszAmmo1)
            {
                AddAmmoNameToAmmoRegistry(II.pszAmmo1);
            }

            if (II.pszAmmo2 && *II.pszAmmo2)
            {
                AddAmmoNameToAmmoRegistry(II.pszAmmo2);
            }

            memset(&II, 0, sizeof II);
        }
    }

    REMOVE_ENTITY(pent);
}

void W_Precache(void)
{
    memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
    memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
    giAmmoIndex = 0;

    UTIL_PrecacheOther("item_suit");
    UTIL_PrecacheOther("item_battery");
    UTIL_PrecacheOther("item_antidote");
    UTIL_PrecacheOther("item_security");
    UTIL_PrecacheOther("item_longjump");
    UTIL_PrecacheOther("item_kevlar");
    UTIL_PrecacheOther("item_assaultsuit");
    UTIL_PrecacheOther("item_thighpack");

    UTIL_PrecacheOtherWeapon("weapon_awp");
    UTIL_PrecacheOther("ammo_338magnum");
    UTIL_PrecacheOtherWeapon("weapon_g3sg1");
    UTIL_PrecacheOtherWeapon("weapon_ak47");
    UTIL_PrecacheOtherWeapon("weapon_scout");
    UTIL_PrecacheOther("ammo_762nato");

    UTIL_PrecacheOtherWeapon("weapon_m249");
    UTIL_PrecacheOther("ammo_556natobox");

    UTIL_PrecacheOtherWeapon("weapon_m4a1");
    UTIL_PrecacheOtherWeapon("weapon_sg552");
    UTIL_PrecacheOtherWeapon("weapon_aug");
    UTIL_PrecacheOther("ammo_556nato");

    UTIL_PrecacheOtherWeapon("weapon_m3");
    UTIL_PrecacheOtherWeapon("weapon_xm1014");
    UTIL_PrecacheOther("ammo_buckshot");

    UTIL_PrecacheOtherWeapon("weapon_usp");
    UTIL_PrecacheOtherWeapon("weapon_mac10");
    UTIL_PrecacheOther("ammo_45acp");
    UTIL_PrecacheOtherWeapon("weapon_p90");
    UTIL_PrecacheOther("ammo_57mm");
    UTIL_PrecacheOtherWeapon("weapon_deagle");
    UTIL_PrecacheOther("ammo_50ae");
    UTIL_PrecacheOtherWeapon("weapon_p228");
    UTIL_PrecacheOther("ammo_357sig");

    UTIL_PrecacheOtherWeapon("weapon_knife");
    UTIL_PrecacheOtherWeapon("weapon_glock18");
    UTIL_PrecacheOtherWeapon("weapon_mp5navy");
    UTIL_PrecacheOtherWeapon("weapon_tmp");
    UTIL_PrecacheOther("ammo_9mm");

    UTIL_PrecacheOtherWeapon("weapon_flashbang");
    UTIL_PrecacheOtherWeapon("weapon_hegrenade");

    UTIL_PrecacheOtherWeapon("weapon_c4");

    UTIL_PrecacheOther("debris");
    UTIL_PrecacheOtherWeapon("weapon_satchel");

    if (g_pGameRules->IsDeathmatch())
    {
        UTIL_PrecacheOther("weaponbox");
    }

    g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");
    g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");
    g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");
    g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");
    g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");
    g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");
    g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
    g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
    g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
    g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
    g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");
    g_sModelIndexCTGhost = PRECACHE_MODEL("sprites/b-tele1.spr");
    g_sModelIndexTGhost = PRECACHE_MODEL("sprites/c-tele1.spr");
    g_sModelIndexC4Glow = PRECACHE_MODEL("sprites/ledglow.spr");

    g_sModelIndexLaser = PRECACHE_MODEL((char*)g_pModelNameLaser);
    g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

    PRECACHE_MODEL("models/grenade.mdl");
    PRECACHE_MODEL("models/w_c4.mdl");
    PRECACHE_MODEL("sprites/explode1.spr");

    PRECACHE_SOUND("weapons/debris1.wav");
    PRECACHE_SOUND("weapons/debris2.wav");
    PRECACHE_SOUND("weapons/debris3.wav");
    PRECACHE_SOUND("weapons/grenade_hit1.wav");
    PRECACHE_SOUND("weapons/grenade_hit2.wav");
    PRECACHE_SOUND("weapons/grenade_hit3.wav");
    PRECACHE_SOUND("weapons/bullet_hit1.wav");
    PRECACHE_SOUND("weapons/bullet_hit2.wav");
    PRECACHE_SOUND("items/weapondrop1.wav");
    PRECACHE_SOUND("weapons/generic_reload.wav");
}

TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] =
{
    DEFINE_FIELD(CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR),
    DEFINE_FIELD(CBasePlayerItem, m_pNext, FIELD_CLASSPTR),
    DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating);

TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] =
{
    DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
    DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
    DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
    DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem);

void CBasePlayerItem::SetObjectCollisionBox(void)
{
    pev->absmin = pev->origin + Vector(-24, -24, 0);
    pev->absmax = pev->origin + Vector(24, 24, 16);
}

void CBasePlayerItem::FallInit(void)
{
    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_BBOX;

    UTIL_SetOrigin(pev, pev->origin);
    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    SetTouch(&CBasePlayerItem::DefaultTouch);
    SetThink(&CBasePlayerItem::FallThink);

    pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayerItem::FallThink(void)
{
    pev->nextthink = gpGlobals->time + 0.1;

    if (pev->flags & FL_ONGROUND)
    {

        if (!FNullEnt(pev->owner))
        {
            int pitch = 95 + RANDOM_LONG(0, 29);
            EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
        }

        pev->angles.x = 0;
        pev->angles.z = 0;

        Materialize();
    }
}

void CBasePlayerItem::Materialize(void)
{
    if (pev->effects & EF_NODRAW)
    {

        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
        pev->effects &= ~EF_NODRAW;
        pev->effects |= EF_MUZZLEFLASH;
    }

    pev->solid = SOLID_TRIGGER;

    UTIL_SetOrigin(pev, pev->origin);
    SetTouch(&CBasePlayerItem::DefaultTouch);
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 1.0;
}

void CBasePlayerItem::AttemptToMaterialize(void)
{
    float time = g_pGameRules->FlWeaponTryRespawn(this);

    if (time == 0)
    {
        Materialize();
        return;
    }

    pev->nextthink = gpGlobals->time + time;
}

void CBasePlayerItem::CheckRespawn(void)
{
    g_pGameRules->WeaponShouldRespawn(this);
}

CBaseEntity* CBasePlayerItem::Respawn(void)
{

    CBaseEntity* pNewWeapon = CBaseEntity::Create((char*)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

    if (pNewWeapon)
    {
        pNewWeapon->pev->effects |= EF_NODRAW;
        pNewWeapon->SetTouch(NULL);
        pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

        DROP_TO_FLOOR(ENT(pev));

        pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
    }
    else
    {
        ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
    }

    return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch(CBaseEntity* pOther)
{
    if (!pOther->IsPlayer())
        return;

    CBasePlayer* pPlayer = (CBasePlayer*)pOther;

    if (m_iId != WEAPON_KNIFE && pPlayer->m_bIsVIP)
        return;

    if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
    {
        if (gEvilImpulse101)
        {
            UTIL_Remove(this);
        }
        return;
    }

    if (pOther->AddPlayerItem(this))
    {
        AttachToPlayer(pPlayer);
        EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
    }

    SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

void CBasePlayerWeapon::FireRemaining(void)
{
    float nexttime;

    if (--m_iClip < 0)
    {
        m_iClip = 0;
        m_pPlayer->m_iBurstShotsFired = 3;
        m_pPlayer->m_flBurstShootTime = 0;
        return;
    }

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecDir;

    Vector shellVelocity;
    if (m_pPlayer->m_bLeftHanded)
    {
        shellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * RANDOM_FLOAT(35, 55)
            + gpGlobals->v_up * RANDOM_FLOAT(100, 200)
            + gpGlobals->v_forward * RANDOM_FLOAT(-100, 100);
    }
    else
    {
        shellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * RANDOM_FLOAT(35, 55)
            + gpGlobals->v_up * RANDOM_FLOAT(100, 200)
            + gpGlobals->v_forward * RANDOM_FLOAT(-100, 100);
    }

    vecDir = gpGlobals->v_forward;

    if (m_iId == WEAPON_GLOCK18)
    {
        Vector shellOrigin = pev->origin + m_pPlayer->pev->view_ofs;

        if (m_pPlayer->m_bLeftHanded)
        {
            shellOrigin = shellOrigin
                - gpGlobals->v_up * 12.0
                + gpGlobals->v_forward * 32.0
                - gpGlobals->v_right * 11.0;
        }
        else
        {
            shellOrigin = shellOrigin
                - gpGlobals->v_up * 12.0
                + gpGlobals->v_forward * 32.0
                + gpGlobals->v_right * 11.0;
        }

        EjectBrass2(shellOrigin, shellVelocity, pev->angles[1], m_iShellId, 1, m_pPlayer->pev);

        m_pPlayer->FireBullets3(
            vecSrc, vecDir, Vector(0.025, 0.025, 0.025),
            8192.0, 1, BULLET_PLAYER_9MM, 18, 0.9, NULL, true);

        nexttime = 0.1;
    }
    else if (m_iId == WEAPON_M4A1)
    {
        ALERT(at_console, " I'm here!\n");

        Vector shellOrigin = pev->origin + m_pPlayer->pev->view_ofs;

        shellOrigin = shellOrigin
            - gpGlobals->v_up * 8.0
            + gpGlobals->v_forward * 20.0
            - gpGlobals->v_right * 10.0;

        EjectBrass2(shellOrigin, shellVelocity, pev->angles[1], m_iShellId, 1, m_pPlayer->pev);

        m_pPlayer->FireBullets3(
            vecSrc, vecDir, Vector(0.015, 0.015, 0.015),
            8192.0, 1, BULLET_PLAYER_556MM, 26, 0.94, NULL, false);

        nexttime = 0.075;
    }

    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    if (++m_pPlayer->m_iBurstShotsFired != 3)
    {
        m_pPlayer->m_flBurstShootTime = gpGlobals->time + nexttime;
    }
    else
    {
        m_pPlayer->m_flBurstShootTime = 0;
    }
}

void CBasePlayerWeapon::ItemPostFrame(void)
{
    if (m_pPlayer->m_flBurstShootTime != 0)
    {
        FireRemaining();
    }

    if (m_flNextPrimaryAttack <= gpGlobals->time)
    {
        if (m_pPlayer->m_bResumeZoom)
        {
            m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;

            if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
            {
                m_pPlayer->pev->viewmodel = 0;
                m_pPlayer->m_bResumeZoom = false;
            }
        }
    }

    if (m_pPlayer->m_flEjectBrass != 0 && m_pPlayer->m_flEjectBrass <= gpGlobals->time)
    {
        m_pPlayer->m_flEjectBrass = 0;
        EjectBrassLate();
    }

    if (m_fInReload && m_pPlayer->m_flNextAttack <= gpGlobals->time)
    {
        int j = min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

        m_iClip += j;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

        m_fInReload = FALSE;
    }

    if ((m_pPlayer->pev->button & IN_ATTACK2) && m_flNextSecondaryAttack <= gpGlobals->time)
    {
        if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
        {
            m_fFireOnEmpty = TRUE;
        }

        SecondaryAttack();
        m_pPlayer->pev->button &= ~IN_ATTACK2;
    }
    else if ((m_pPlayer->pev->button & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->time)
    {
        if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
        {
            m_fFireOnEmpty = TRUE;
        }

        if (g_pGameRules->IsMultiplayer() && !((CHalfLifeMultiplay*)g_pGameRules)->m_bFreezePeriod)
        {
            PrimaryAttack();
        }
    }
    else if ((m_pPlayer->pev->button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload)
    {
        Reload();
    }
    else if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
    {
        if (m_bDelayFire)
        {
            m_bDelayFire = FALSE;
            m_pPlayer->m_iShotsFired = 20;
        }

        m_fFireOnEmpty = FALSE;

        if (m_pPlayer->m_iShotsFired > 0)
            m_pPlayer->m_iShotsFired--;

        if (!IsUseable() && m_flNextPrimaryAttack < gpGlobals->time)
        {
            WeaponIdle();
            return;
        }

        if (!m_iClip && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->time)
        {
            Reload();
            return;
        }

        WeaponIdle();
        return;
    }

    if (ShouldWeaponIdle())
    {
        WeaponIdle();
    }
}

void CBasePlayerItem::DestroyItem(void)
{
    if (m_pPlayer)
    {

        m_pPlayer->RemovePlayerItem(this);
    }

    Kill();
}

int CBasePlayerItem::AddToPlayer(CBasePlayer* pPlayer)
{
    m_pPlayer = pPlayer;

    MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->edict());
    WRITE_BYTE(m_iId);
    MESSAGE_END();

    return TRUE;
}

void CBasePlayerItem::Drop(void)
{
    SetTouch(NULL);
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Kill(void)
{
    SetTouch(NULL);
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Holster(void)
{
    m_pPlayer->pev->viewmodel = 0;
    m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer* pPlayer)
{
    pev->movetype = MOVETYPE_FOLLOW;
    pev->solid = SOLID_NOT;
    pev->aiment = pPlayer->edict();
    pev->effects = EF_NODRAW;
    pev->modelindex = 0;
    pev->model = iStringNull;
    pev->owner = pPlayer->edict();
    pev->nextthink = gpGlobals->time + .1;
    SetThink(NULL);
    SetTouch(NULL);
}

int CBasePlayerWeapon::AddDuplicate(CBasePlayerItem* pOriginal)
{
    if (m_iDefaultAmmo)
    {
        return ExtractAmmo((CBasePlayerWeapon*)pOriginal);
    }
    else
    {
        return ExtractClipAmmo((CBasePlayerWeapon*)pOriginal);
    }
}

int CBasePlayerWeapon::AddToPlayer(CBasePlayer* pPlayer)
{
    m_pPlayer = pPlayer;
    pPlayer->pev->weapons |= (1 << m_iId);

    if (!m_iPrimaryAmmoType)
    {
        m_iPrimaryAmmoType = CBasePlayer::GetAmmoIndex(ItemInfoArray[m_iId].pszAmmo1);
        m_iSecondaryAmmoType = CBasePlayer::GetAmmoIndex(ItemInfoArray[m_iId].pszAmmo2);
    }

    if (!AddWeapon())
        return FALSE;

    MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->edict());
    WRITE_BYTE(m_iId);
    MESSAGE_END();

    return TRUE;
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer* pPlayer)
{
    int state = 0;
    CBasePlayerItem* pActive = pPlayer->m_pActiveItem;

    if (pActive == this)
    {
        state = 1;
        if (pPlayer->m_fOnTarget)
            state = WEAPON_IS_ONTARGET;
    }

    if (!pPlayer->m_fWeapon || pActive != pPlayer->m_pClientActiveItem || m_iClip != m_iClientClip || state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pPlayer->edict());
        WRITE_BYTE(state);
        WRITE_BYTE(m_iId);
        WRITE_BYTE(m_iClip);
        MESSAGE_END();

        m_iClientClip = m_iClip;
        m_iClientWeaponState = state;
        pPlayer->m_fWeapon = TRUE;
    }

    if (m_pNext)
        m_pNext->UpdateClientData(pPlayer);

    return 1;
}

void CBasePlayerWeapon::SendWeaponAnim(int iAnim)
{
    MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->edict());
    WRITE_BYTE(iAnim);
    WRITE_BYTE(pev->body);
    MESSAGE_END();
}

BOOL CBasePlayerWeapon::AddPrimaryAmmo(int iCount, char* szName, int iMaxClip, int iMaxCarry)
{
    int iIdAmmo;

    if (iMaxClip < 1)
    {
        m_iClip = -1;
        iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
    }
    else if (m_iClip == 0)
    {
        int i;
        i = min(m_iClip + iCount, iMaxClip) - m_iClip;
        m_iClip += i;
        iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, szName, iMaxCarry);
    }
    else
    {
        iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
    }

    if (iIdAmmo > 0)
    {
        m_iPrimaryAmmoType = iIdAmmo;
        if (m_pPlayer->HasPlayerItem(this))
        {
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
        }
    }

    return iIdAmmo > 0 ? TRUE : FALSE;
}

BOOL CBasePlayerWeapon::AddSecondaryAmmo(int iCount, char* szName, int iMax)
{
    int iIdAmmo;

    iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMax);

    if (iIdAmmo > 0)
    {
        m_iSecondaryAmmoType = iIdAmmo;
        EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
    }
    return iIdAmmo > 0 ? TRUE : FALSE;
}

BOOL CBasePlayerWeapon::IsUseable(void)
{
    if (m_iClip <= 0)
    {
        if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1)
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CBasePlayerWeapon::CanDeploy(void)
{
    return TRUE;
}

BOOL CBasePlayerWeapon::DefaultDeploy(char* szViewModel, char* szWeaponModel, int iAnim, char* szAnimExt)
{
    if (!CanDeploy())
        return FALSE;

    m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
    m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
    strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
    SendWeaponAnim(iAnim);

    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
    m_flTimeWeaponIdle = gpGlobals->time + 1.0;

    m_pPlayer->m_iFOV = 90;
    m_pPlayer->m_bResumeZoom = 0;
    m_pPlayer->m_iLastZoom = 90;

    return TRUE;
}

void CBasePlayerWeapon::ReloadSound(void)
{
    CBaseEntity* pPlayer = NULL;
    while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
    {
        if (pPlayer->pev->flags == FL_DORMANT)
            break;

        if (pPlayer != (CBaseEntity*)m_pPlayer)
        {
            float distance = (m_pPlayer->pev->origin - pPlayer->pev->origin).Length();
            if (distance <= MAX_DIST_RELOAD_SOUND)
            {
                MESSAGE_BEGIN(MSG_ONE, gmsgReloadSound, NULL, ENT(pPlayer->pev));
                WRITE_BYTE((int)((1.0 - distance / MAX_DIST_RELOAD_SOUND) * 255.0));
                if (!strcmp(STRING(pev->classname), "weapon_m3")
                    || !strcmp(STRING(pev->classname), "weapon_xm1014"))
                    WRITE_BYTE(0);
                else
                    WRITE_BYTE(1);
                MESSAGE_END();
            }
        }
    }
}

BOOL CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay)
{
    int j = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

    if (j <= 0)
        return FALSE;

    if (j > iClipSize - m_iClip)
        j = iClipSize - m_iClip;

    if (!j)
        return FALSE;

    m_pPlayer->m_flNextAttack = fDelay + gpGlobals->time;

    ReloadSound();
    SendWeaponAnim(iAnim);

    m_fInReload = TRUE;

    m_flTimeWeaponIdle = gpGlobals->time + fDelay + 0.5;
    return TRUE;
}

BOOL CBasePlayerWeapon::PlayEmptySound(void)
{
    if (m_iPlayEmptySound)
    {
        EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
        m_iPlayEmptySound = 0;
    }

    return 0;
}

void CBasePlayerWeapon::ResetEmptySound(void)
{
    m_iPlayEmptySound = 1;
}

int CBasePlayerWeapon::PrimaryAmmoIndex(void)
{
    return m_iPrimaryAmmoType;
}

int CBasePlayerWeapon::SecondaryAmmoIndex(void)
{
    return -1;
}

void CBasePlayerWeapon::Holster(void)
{
    if (m_iId == WEAPON_G3SG1)
    {
        Vector color(20.0, 20.0, 20.0);
        UTIL_ScreenFade(m_pPlayer, color, 0.5, 0.1, 0, 0);
    }

    m_fInReload = FALSE;
    m_pPlayer->pev->viewmodel = 0;
    m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerAmmo::Spawn(void)
{
    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;
    UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
    UTIL_SetOrigin(pev, pev->origin);

    SetTouch(&CBasePlayerAmmo::DefaultTouch);

    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 2.0;
}

CBaseEntity* CBasePlayerAmmo::Respawn(void)
{
    pev->effects |= EF_NODRAW;
    SetTouch(NULL);

    UTIL_SetOrigin(pev, g_pGameRules->VecAmmoRespawnSpot(this));

    SetThink(&CBasePlayerAmmo::Materialize);
    pev->nextthink = g_pGameRules->FlAmmoRespawnTime(this);

    return this;
}

void CBasePlayerAmmo::Materialize(void)
{
    if (pev->effects & EF_NODRAW)
    {

        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
        pev->effects &= ~EF_NODRAW;
        pev->effects |= EF_MUZZLEFLASH;
    }

    SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

void CBasePlayerAmmo::DefaultTouch(CBaseEntity* pOther)
{
    if (!pOther->IsPlayer())
    {
        return;
    }

    if (AddAmmo(pOther))
    {
        if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
        {
            Respawn();
        }
        else
        {
            SetTouch(NULL);
            SetThink(&CBaseEntity::SUB_Remove);
            pev->nextthink = gpGlobals->time + .1;
        }
    }
    else if (gEvilImpulse101)
    {

        SetTouch(NULL);
        SetThink(&CBaseEntity::SUB_Remove);
        pev->nextthink = gpGlobals->time + .1;
    }
}

int CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
    int			iReturn;

    if (pszAmmo1() != NULL)
    {

        iReturn = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char*)pszAmmo1(), iMaxClip(), iMaxAmmo1());
        m_iDefaultAmmo = 0;
    }

    if (pszAmmo2() != NULL)
    {
        iReturn = pWeapon->AddSecondaryAmmo(0, (char*)pszAmmo2(), iMaxAmmo2());
    }

    return iReturn;
}

int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon* pWeapon)
{
    int			iAmmo;

    if (m_iClip == WEAPON_NOCLIP)
    {
        iAmmo = 0;
    }
    else
    {
        iAmmo = m_iClip;
    }

    return pWeapon->m_pPlayer->GiveAmmo(iAmmo, (char*)pszAmmo1(), iMaxAmmo1());
}

void CBasePlayerWeapon::KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
{
    float flKickUp = up_base;
    float flKickLateral = lateral_base;

    if (m_pPlayer->m_iShotsFired != 1)
    {
        flKickUp += (float)m_pPlayer->m_iShotsFired * up_modifier;
        flKickLateral += (float)m_pPlayer->m_iShotsFired * lateral_modifier;
    }

    ALERT(at_console, "Kick: UP = %.2f/%.2f  LAT = %.2f/%.2f\n", flKickUp, up_max, flKickLateral, lateral_max);

    m_pPlayer->pev->punchangle.x -= flKickUp;

    if (m_pPlayer->pev->punchangle.x < -up_max)
        m_pPlayer->pev->punchangle.x = -up_max;

    if (m_iDirection == 1)
    {
        m_pPlayer->pev->punchangle.y += flKickLateral;
        if (m_pPlayer->pev->punchangle.y > lateral_max)
            m_pPlayer->pev->punchangle.y = lateral_max;
    }
    else
    {
        m_pPlayer->pev->punchangle.y -= flKickLateral;
        if (m_pPlayer->pev->punchangle.y < -lateral_max)
            m_pPlayer->pev->punchangle.y = -lateral_max;
    }

    if (RANDOM_LONG(0, direction_change) == 0)
        m_iDirection = 1 - m_iDirection;
}

void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype)
{
    MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
    WRITE_BYTE(TE_MODEL);
    WRITE_COORD(vecOrigin.x);
    WRITE_COORD(vecOrigin.y);
    WRITE_COORD(vecOrigin.z);
    WRITE_COORD(vecVelocity.x);
    WRITE_COORD(vecVelocity.y);
    WRITE_COORD(vecVelocity.z);
    WRITE_ANGLE(rotation);
    WRITE_SHORT(model);
    WRITE_BYTE(soundtype);
    WRITE_BYTE(25);
    MESSAGE_END();
}

void EjectBrass2(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype, entvars_t* pevShooter)
{
    MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, ENT(pevShooter));
    WRITE_BYTE(TE_MODEL);
    WRITE_COORD(vecOrigin.x);
    WRITE_COORD(vecOrigin.y);
    WRITE_COORD(vecOrigin.z);
    WRITE_COORD(vecVelocity.x);
    WRITE_COORD(vecVelocity.y);
    WRITE_COORD(vecVelocity.z);
    WRITE_ANGLE(rotation);
    WRITE_SHORT(model);
    WRITE_BYTE(0);
    WRITE_BYTE(5);
    MESSAGE_END();
}

void CBasePlayerWeapon::EjectBrassLate(void)
{
    Vector vecShellVelocity;
    Vector vecShellOrigin;
    Vector vecAngles;

    if (m_pPlayer->m_bLeftHanded)
    {
        vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
        UTIL_MakeVectors(vecAngles);

        vecShellVelocity = m_pPlayer->pev->velocity
            + RANDOM_FLOAT(50, 70) * gpGlobals->v_right
            + RANDOM_FLOAT(100, 150) * gpGlobals->v_up
            + gpGlobals->v_forward * 25;

        vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12
            + gpGlobals->v_forward * 16
            - gpGlobals->v_right * 6;
    }
    else
    {
        vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
        UTIL_MakeVectors(vecAngles);

        vecShellVelocity = m_pPlayer->pev->velocity
            - RANDOM_FLOAT(50, 70) * gpGlobals->v_right
            + RANDOM_FLOAT(100, 150) * gpGlobals->v_up
            + gpGlobals->v_forward * 25;

        vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12
            + gpGlobals->v_forward * 16
            + gpGlobals->v_right * 6;
    }

    EjectBrass(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShellId, 1);
}

void CBasePlayerWeapon::RetireWeapon(void)
{
    m_pPlayer->pev->viewmodel = iStringNull;
    m_pPlayer->pev->weaponmodel = iStringNull;

    g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
}

class CShield : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    BOOL Deploy(void);
    void Holster(int skiplocal = 0);
    void PrimaryAttack(void);
    float GetMaxSpeed(void);
    int iItemSlot(void) { return 1; }
};

LINK_ENTITY_TO_CLASS(weapon_shield, CShield);

void CShield::Spawn(void)
{
    Precache();
    m_iId = WEAPON_SHIELD;
    SET_MODEL(ENT(pev), "models/w_knife.mdl");
    m_iClip = -1;
    FallInit();
}

void CShield::Precache(void)
{
    PRECACHE_MODEL("models/v_knife.mdl");
    PRECACHE_MODEL("models/w_knife.mdl");
    PRECACHE_MODEL("models/p_shield.mdl");
}

int CShield::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 0;
    p->iPosition = 2;
    p->iId = WEAPON_SHIELD;
    p->iWeight = 2;
    return 1;
}

BOOL CShield::Deploy(void)
{
    return DefaultDeploy("models/v_knife.mdl", "models/p_shield.mdl", 3, "shield");
}

void CShield::Holster(int skiplocal)
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
}

void CShield::PrimaryAttack(void)
{
}

float CShield::GetMaxSpeed(void)
{
    return 200.0;
}

LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox);

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] =
{
    DEFINE_ARRAY(CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
    DEFINE_ARRAY(CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_SLOTS),
    DEFINE_ARRAY(CWeaponBox, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
    DEFINE_FIELD(CWeaponBox, m_cAmmoTypes, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CWeaponBox, CBaseEntity);

void CWeaponBox::Precache(void)
{
    PRECACHE_MODEL("models/w_weaponbox.mdl");
}

void CWeaponBox::KeyValue(KeyValueData* pkvd)
{
    if (m_cAmmoTypes < MAX_AMMO_SLOTS)
    {
        PackAmmo(ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue));
        m_cAmmoTypes++;

        pkvd->fHandled = TRUE;
    }
    else
    {
        ALERT(at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_SLOTS);
    }
}

void CWeaponBox::Spawn(void)
{
    Precache();

    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

    UTIL_SetSize(pev, g_vecZero, g_vecZero);

    SET_MODEL(ENT(pev), "models/w_weaponbox.mdl");
}

void CWeaponBox::Kill(void)
{
    CBasePlayerItem* pWeapon;
    int i;

    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        pWeapon = m_rgpPlayerItems[i];

        while (pWeapon)
        {
            pWeapon->SetThink(&CBaseEntity::SUB_Remove);
            pWeapon->pev->nextthink = gpGlobals->time + 0.1;
            pWeapon = pWeapon->m_pNext;
        }
    }

    UTIL_Remove(this);
}

void CWeaponBox::Touch(CBaseEntity* pOther)
{
    if (!(pev->flags & FL_ONGROUND))
    {
        return;
    }

    if (!pOther->IsPlayer())
    {

        return;
    }

    if (!pOther->IsAlive())
    {

        return;
    }

    CBasePlayer* pPlayer = (CBasePlayer*)pOther;

    if (pPlayer->m_bIsVIP)
        return;

    BOOL bEmptyBox = TRUE;
    BOOL bPickedUpWeapon = FALSE;

    for (int i = 0; i < MAX_ITEM_TYPES; i++)
    {
        CBasePlayerItem* pItem = (CBasePlayerItem*)m_rgpPlayerItems[i];

        while (pItem)
        {
            if (FClassnameIs(pItem->pev, "weapon_c4"))
            {

                if (pPlayer->m_iTeam != TEAM_TERRORIST || pPlayer->pev->deadflag || !pPlayer)
                    return;

                pPlayer->m_bHasC4 = TRUE;
                pPlayer->SetBombIcon(FALSE);
                pPlayer->pev->body = 1;
            }

            if ((i == 1 || i == 2) && pPlayer->m_rgpPlayerItems[i])
            {
                bEmptyBox = FALSE;
                pItem = (CBasePlayerItem*)m_rgpPlayerItems[i]->m_pNext;
            }
            else
            {
                bPickedUpWeapon = TRUE;

                if (pPlayer->AddPlayerItem(pItem))
                {
                    pItem->AttachToPlayer(pPlayer);
                }

                m_rgpPlayerItems[i] = m_rgpPlayerItems[i]->m_pNext;
                pItem = (CBasePlayerItem*)m_rgpPlayerItems[i];
            }
        }
    }

    if (bEmptyBox)
    {
        for (int i = 0; i < MAX_AMMO_SLOTS; i++)
        {
            if (!FStringNull(m_rgiszAmmo[i]))
            {

                pPlayer->GiveAmmo(m_rgAmmo[i], (char*)STRING(m_rgiszAmmo[i]), MaxAmmoCarry(m_rgiszAmmo[i]));

                m_rgiszAmmo[i] = iStringNull;
                m_rgAmmo[i] = 0;
            }
        }
    }

    if (bPickedUpWeapon)
    {
        EMIT_SOUND(pOther->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
    }

    if (bEmptyBox)
    {
        SetThink(NULL);
        SetTouch(NULL);
        UTIL_Remove(this);
    }
}

BOOL CWeaponBox::PackWeapon(CBasePlayerItem* pWeapon)
{
    if (HasWeapon(pWeapon))
    {
        return FALSE;
    }

    if (pWeapon->m_pPlayer)
    {
        if (pWeapon->m_pPlayer->m_pActiveItem == pWeapon)
        {
            pWeapon->Holster();
        }

        if (!pWeapon->m_pPlayer->RemovePlayerItem(pWeapon))
        {

            return FALSE;
        }
    }

    int iWeaponSlot = pWeapon->iItemSlot();

    if (m_rgpPlayerItems[iWeaponSlot])
    {
        pWeapon->m_pNext = m_rgpPlayerItems[iWeaponSlot];
        m_rgpPlayerItems[iWeaponSlot] = pWeapon;
    }
    else
    {
        m_rgpPlayerItems[iWeaponSlot] = pWeapon;
        pWeapon->m_pNext = NULL;
    }

    pWeapon->pev->spawnflags |= SF_NORESPAWN;
    pWeapon->pev->movetype = MOVETYPE_NONE;
    pWeapon->pev->solid = SOLID_NOT;
    pWeapon->pev->effects = EF_NODRAW;
    pWeapon->pev->modelindex = 0;
    pWeapon->pev->model = iStringNull;
    pWeapon->pev->owner = edict();
    pWeapon->SetThink(NULL);
    pWeapon->SetTouch(NULL);
    pWeapon->m_pPlayer = NULL;

    return TRUE;
}

BOOL CWeaponBox::PackAmmo(int iszName, int iCount)
{
    int iMaxCarry;

    if (FStringNull(iszName))
    {
        ALERT(at_console, "NULL String in PackAmmo!\n");
        return FALSE;
    }

    iMaxCarry = MaxAmmoCarry(iszName);

    if (iMaxCarry != -1 && iCount > 0)
    {
        GiveAmmo(iCount, (char*)STRING(iszName), iMaxCarry);
        return TRUE;
    }

    return FALSE;
}

int CWeaponBox::GiveAmmo(int iCount, char* szName, int iMax, int* pIndex)
{
    int i;

    for (i = 1; i < MAX_AMMO_SLOTS && !FStringNull(m_rgiszAmmo[i]); i++)
    {
        if (stricmp(szName, STRING(m_rgiszAmmo[i])) == 0)
        {
            if (pIndex)
                *pIndex = i;

            int iAdd = min(iCount, iMax - m_rgAmmo[i]);
            if (iCount == 0 || iAdd > 0)
            {
                m_rgAmmo[i] += iAdd;

                return i;
            }
            return -1;
        }
    }
    if (i < MAX_AMMO_SLOTS)
    {
        if (pIndex)
            *pIndex = i;

        m_rgiszAmmo[i] = MAKE_STRING(szName);
        m_rgAmmo[i] = iCount;

        return i;
    }
    ALERT(at_console, "out of named ammo slots\n");
    return i;
}

BOOL CWeaponBox::HasWeapon(CBasePlayerItem* pCheckItem)
{
    CBasePlayerItem* pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

    while (pItem)
    {
        if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
        {
            return TRUE;
        }
        pItem = pItem->m_pNext;
    }

    return FALSE;
}

BOOL CWeaponBox::IsEmpty(void)
{
    int i;

    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        if (m_rgpPlayerItems[i])
        {
            return FALSE;
        }
    }

    for (i = 0; i < MAX_AMMO_SLOTS; i++)
    {
        if (!FStringNull(m_rgiszAmmo[i]))
        {

            return FALSE;
        }
    }

    return TRUE;
}

void CWeaponBox::SetObjectCollisionBox(void)
{
    pev->absmin = pev->origin + Vector(-16, -16, 0);
    pev->absmax = pev->origin + Vector(16, 16, 16);
}

