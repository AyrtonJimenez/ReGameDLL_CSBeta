#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#define BOLT_AIR_VELOCITY	2000
#define BOLT_WATER_VELOCITY	1000

class CDebris : public CBaseEntity
{
    void Spawn(void);
    void Precache(void);
    void EXPORT BubbleThink(void);
    void EXPORT BoltTouch(CBaseEntity* pOther);

    int m_iTrail;
    int m_iBounce;
    float m_flDmg;

public:
    static CDebris* BoltCreate(void);
};
LINK_ENTITY_TO_CLASS(debris, CDebris);

CDebris* CDebris::BoltCreate(void)
{
    CDebris* pBolt = GetClassPtr((CDebris*)NULL);
    pBolt->pev->classname = MAKE_STRING("debris");
    pBolt->Spawn();

    return pBolt;
}

void CDebris::Spawn()
{
    Precache();

    pev->movetype = MOVETYPE_BOUNCE;
    pev->solid = SOLID_BBOX;

    pev->gravity = 0.5;

    m_iBounce = 0;
    m_flDmg = 30;

    SET_MODEL(ENT(pev), "models/shrapnel.mdl");

    UTIL_SetOrigin(pev, pev->origin);
    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    SetTouch(&CDebris::BoltTouch);
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 1.5;
}

void CDebris::Precache()
{
    PRECACHE_MODEL("models/shrapnel.mdl");
    PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
    PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
    PRECACHE_SOUND("weapons/xbow_fly1.wav");
    PRECACHE_SOUND("weapons/xbow_hit1.wav");
    PRECACHE_SOUND("weapons/ric1.wav");
    PRECACHE_SOUND("weapons/ric2.wav");
    PRECACHE_SOUND("weapons/ric3.wav");
    PRECACHE_SOUND("weapons/ric4.wav");
    PRECACHE_SOUND("weapons/ric5.wav");
    PRECACHE_SOUND("fvox/beep.wav");
    m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}

void CDebris::BoltTouch(CBaseEntity* pOther)
{
    if (pOther->pev->takedamage)
    {
        TraceResult tr = UTIL_GetGlobalTrace();
        entvars_t* pevOwner;

        if (pev->owner)
            pevOwner = VARS(pev->owner);
        else
            pevOwner = NULL;

        if (pevOwner)
        {
            ClearMultiDamage();

            if (pOther->IsPlayer())
            {
                pOther->TraceAttack(pevOwner, m_flDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
            }
            else
            {
                pOther->TraceAttack(pevOwner, m_flDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
            }

            ApplyMultiDamage(pev, pevOwner);

            pev->velocity = Vector(0, 0, 0);

            switch (RANDOM_LONG(0, 1))
            {
            case 0:
                EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM); break;
            case 1:
                EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM); break;
            }
        }

        Killed(pev, GIB_NEVER);
    }
}

void CDebris::BubbleThink(void)
{
    pev->nextthink = gpGlobals->time + 0.1;

    if (pev->waterlevel == 0)
        return;

    UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

enum crossbow_e {
    CROSSBOW_IDLE1 = 0,
    CROSSBOW_IDLE2,
    CROSSBOW_FIDGET1,
    CROSSBOW_FIDGET2,
    CROSSBOW_FIRE1,
    CROSSBOW_FIRE2,
    CROSSBOW_FIRE3,
    CROSSBOW_RELOAD,
    CROSSBOW_DRAW1,
    CROSSBOW_DRAW2,
    CROSSBOW_HOLSTER1,
    CROSSBOW_HOLSTER2,
};

class CCrossbow : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int iItemSlot() { return 3; }
    int GetItemInfo(ItemInfo* p);

    void FireBolt(void);
    void FireSniperBolt(void);
    void PrimaryAttack(void);
    void SecondaryAttack(void);
    int AddToPlayer(CBasePlayer* pPlayer);
    BOOL Deploy();
    void Holster();
    void Reload(void);
    void WeaponIdle(void);

    int m_fInZoom;
};
LINK_ENTITY_TO_CLASS(weapon_crossbow, CCrossbow);

void CCrossbow::Spawn()
{
    Precache();
    m_iId = WEAPON_CROSSBOW;
    SET_MODEL(ENT(pev), "models/w_crossbow.mdl");

    m_iDefaultAmmo = CROSSBOW_DEFAULT_GIVE;

    FallInit();
}

int CCrossbow::AddToPlayer(CBasePlayer* pPlayer)
{
    if (CBasePlayerWeapon::AddToPlayer(pPlayer))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->edict());
        WRITE_BYTE(m_iId);
        MESSAGE_END();
        return TRUE;
    }
    return FALSE;
}

void CCrossbow::Precache(void)
{
    PRECACHE_MODEL("models/w_crossbow.mdl");
    PRECACHE_MODEL("models/v_crossbow.mdl");
    PRECACHE_MODEL("models/p_crossbow.mdl");

    PRECACHE_SOUND("weapons/xbow_fire1.wav");
    PRECACHE_SOUND("weapons/xbow_reload1.wav");

    UTIL_PrecacheOther("crossbow_bolt");
}

int CCrossbow::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "bolts";
    p->iMaxAmmo1 = BOLT_MAX_CARRY;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = CROSSBOW_MAX_CLIP;
    p->iSlot = 2;
    p->iPosition = 2;
    p->iId = WEAPON_CROSSBOW;
    p->iFlags = 0;
    p->iWeight = CROSSBOW_WEIGHT;
    return 1;
}

BOOL CCrossbow::Deploy()
{
    if (m_iClip)
        return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW1, "bow");
    return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW2, "bow");
}

void CCrossbow::Holster()
{
    m_fInReload = FALSE;

    if (m_fInZoom)
    {
        SecondaryAttack();
    }

    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
    if (m_iClip)
        SendWeaponAnim(CROSSBOW_HOLSTER1);
    else
        SendWeaponAnim(CROSSBOW_HOLSTER2);
}

void CCrossbow::PrimaryAttack(void)
{
    if (m_fInZoom && g_pGameRules->IsMultiplayer())
    {
        FireSniperBolt();
        return;
    }

    FireBolt();
}

void CCrossbow::FireSniperBolt()
{
    m_flNextPrimaryAttack = gpGlobals->time + 0.75;

    if (m_iClip == 0)
    {
        PlayEmptySound();
        return;
    }

    TraceResult tr;

    m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
    m_iClip--;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/xbow_fire1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));

    if (m_iClip)
    {
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
        SendWeaponAnim(CROSSBOW_FIRE1);
    }
    else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
    {
        SendWeaponAnim(CROSSBOW_FIRE3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
    UTIL_MakeVectors(anglesAim);
    Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
    Vector vecDir = gpGlobals->v_forward;

    UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr);

    if (tr.pHit->v.takedamage)
    {
        switch (RANDOM_LONG(0, 1))
        {
        case 0:
            EMIT_SOUND(tr.pHit, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM); break;
        case 1:
            EMIT_SOUND(tr.pHit, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM); break;
        }

        ClearMultiDamage();
        CBaseEntity::Instance(tr.pHit)->TraceAttack(m_pPlayer->pev, 120, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB);
        ApplyMultiDamage(pev, m_pPlayer->pev);
    }
    else
    {
        CDebris* pBolt = CDebris::BoltCreate();
        pBolt->pev->origin = tr.vecEndPos - vecDir * 10;
        pBolt->pev->angles = UTIL_VecToAngles(vecDir);
        pBolt->pev->solid = SOLID_NOT;
        pBolt->SetTouch(NULL);
        pBolt->SetThink(&CBaseEntity::SUB_Remove);

        EMIT_SOUND(pBolt->edict(), CHAN_WEAPON, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM);

        if (UTIL_PointContents(tr.vecEndPos) != CONTENTS_WATER)
        {
            UTIL_Sparks(tr.vecEndPos);
        }

        if (FClassnameIs(tr.pHit, "worldspawn"))
        {
            pBolt->pev->nextthink = gpGlobals->time + 5.0;
        }
        else
        {
            pBolt->pev->nextthink = gpGlobals->time;
        }
    }
}

void CCrossbow::FireBolt()
{
    TraceResult tr;

    if (m_iClip == 0)
    {
        PlayEmptySound();
        return;
    }

    m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

    m_iClip--;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/xbow_fire1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));

    if (m_iClip)
    {

        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
        SendWeaponAnim(CROSSBOW_FIRE1);
    }
    else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
    {
        SendWeaponAnim(CROSSBOW_FIRE3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
    UTIL_MakeVectors(anglesAim);

    anglesAim.x = -anglesAim.x;
    Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
    Vector vecDir = gpGlobals->v_forward;

    CDebris* pBolt = CDebris::BoltCreate();
    pBolt->pev->origin = vecSrc;
    pBolt->pev->angles = anglesAim;
    pBolt->pev->owner = m_pPlayer->edict();

    if (m_pPlayer->pev->waterlevel == 3)
    {
        pBolt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
        pBolt->pev->speed = BOLT_WATER_VELOCITY;
    }
    else
    {
        pBolt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
        pBolt->pev->speed = BOLT_AIR_VELOCITY;
    }
    pBolt->pev->avelocity.z = 10;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)

        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flNextPrimaryAttack = gpGlobals->time + 0.75;

    m_flNextSecondaryAttack = gpGlobals->time + 0.75;
    if (m_iClip != 0)
        m_flTimeWeaponIdle = gpGlobals->time + 5.0;
    else
        m_flTimeWeaponIdle = 0.75;

    m_pPlayer->pev->punchangle.x -= 2;
}

void CCrossbow::SecondaryAttack()
{
    if (m_fInZoom)
    {
        m_pPlayer->m_iFOV = 0;
        m_fInZoom = 0;
    }
    else
    {
        m_pPlayer->m_iFOV = 20;
        m_fInZoom = 1;
    }

    pev->nextthink = gpGlobals->time + 0.1;
    m_flNextSecondaryAttack = gpGlobals->time + 1.0;
}

void CCrossbow::Reload(void)
{
    if (m_fInZoom)
    {
        SecondaryAttack();
    }

    if (DefaultReload(5, CROSSBOW_RELOAD, 4.5))
    {
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
    }
}

void CCrossbow::WeaponIdle(void)
{
    m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

    ResetEmptySound();

    if (m_flTimeWeaponIdle < gpGlobals->time)
    {
        float flRand = RANDOM_FLOAT(0, 1);
        if (flRand <= 0.75)
        {
            if (m_iClip)
            {
                SendWeaponAnim(CROSSBOW_IDLE1);
            }
            else
            {
                SendWeaponAnim(CROSSBOW_IDLE2);
            }
            m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
        }
        else
        {

            if (m_iClip)
            {
                SendWeaponAnim(CROSSBOW_FIDGET1);
                m_flTimeWeaponIdle = gpGlobals->time + 90.0 / 30.0;
            }
            else
            {
                SendWeaponAnim(CROSSBOW_FIDGET2);
                m_flTimeWeaponIdle = gpGlobals->time + 80.0 / 30.0;
            }
        }
    }
}

class CCrossbowAmmo : public CBasePlayerAmmo
{
    void Spawn(void)
    {
        Precache();
        SET_MODEL(ENT(pev), "models/w_crossbow_clip.mdl");
        CBasePlayerAmmo::Spawn();
    }
    void Precache(void)
    {
        PRECACHE_MODEL("models/w_crossbow_clip.mdl");
        PRECACHE_SOUND("items/9mmclip1.wav");
    }
    BOOL AddAmmo(CBaseEntity* pOther)
    {
        if (pOther->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE, "bolts", BOLT_MAX_CARRY) != -1)
        {
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
            return TRUE;
        }
        return FALSE;
    }
};
LINK_ENTITY_TO_CLASS(ammo_crossbow, CCrossbowAmmo);

#endif

