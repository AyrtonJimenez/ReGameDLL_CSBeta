#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum hegrenade_e
{
    HEGRENADE_IDLE = 0,
    HEGRENADE_PULLPIN,
    HEGRENADE_THROW,
    HEGRENADE_DRAW,
};

class CHEGrenade : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 4; }

    BOOL Deploy(void);
    BOOL CanHolster(void);
    void Holster(void);
    void PrimaryAttack(void);
    void WeaponIdle(void);
    BOOL CanDeploy(void);
    BOOL CanDrop(void) { return FALSE; }
    float GetMaxSpeed(void) { return 260.0f; }

    float m_flStartThrow;

    float m_flReleaseThrow;
};

LINK_ENTITY_TO_CLASS(weapon_hegrenade, CHEGrenade);

void CHEGrenade::Spawn(void)
{
    Precache();
    m_iId = WEAPON_HEGRENADE;
    SET_MODEL(ENT(pev), "models/w_hegrenade.mdl");
    pev->dmg = 4;
    m_iDefaultAmmo = 1;
    FallInit();
}

void CHEGrenade::Precache(void)
{
    PRECACHE_MODEL("models/w_hegrenade.mdl");
    PRECACHE_MODEL("models/v_hegrenade_r.mdl");
    PRECACHE_MODEL("models/v_hegrenade.mdl");
    PRECACHE_MODEL("models/p_hegrenade.mdl");

    PRECACHE_SOUND("weapons/hegrenade-1.wav");
    PRECACHE_SOUND("weapons/hegrenade-2.wav");
    PRECACHE_SOUND("weapons/he_bounce-1.wav");
    PRECACHE_SOUND("weapons/pinpull.wav");
}

int CHEGrenade::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "HEGrenade";
    p->iMaxAmmo1 = 1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;

    p->iSlot = 3;
    p->iPosition = 1;

    p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
    p->iId = m_iId = WEAPON_HEGRENADE;
    p->iWeight = 2;
    return 1;
}

BOOL CHEGrenade::Deploy(void)
{
    m_flReleaseThrow = -1.0f;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_hegrenade.mdl", "models/p_hegrenade.mdl", HEGRENADE_DRAW, "crowbar");
    else
        return DefaultDeploy("models/v_hegrenade_r.mdl", "models/p_hegrenade.mdl", HEGRENADE_DRAW, "crowbar");
}

BOOL CHEGrenade::CanHolster(void)
{
    return (m_flStartThrow == 0);
}

void CHEGrenade::Holster(void)
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5f;

    if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
    {
        m_pPlayer->pev->weapons &= ~(1 << WEAPON_HEGRENADE);
        DestroyItem();
    }

    m_flReleaseThrow = 0;
    m_flStartThrow = 0;

    EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0f, ATTN_NORM);
}

void CHEGrenade::PrimaryAttack(void)
{
    if (m_flStartThrow != 0)
        return;

    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        m_flStartThrow = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim(HEGRENADE_PULLPIN);

        m_flTimeWeaponIdle = gpGlobals->time + 0.5f;
    }
}

BOOL CHEGrenade::CanDeploy(void)
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
}

void CHEGrenade::WeaponIdle(void)
{
    if (m_flReleaseThrow == 0)
        m_flReleaseThrow = gpGlobals->time;

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_flStartThrow != 0)
    {
        m_pPlayer->Radio("%!MRAD_FIREINHOLE", "Fire in the hole!\n");

        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if (angThrow.x >= 0)
            angThrow.x = angThrow.x * (10.0f / 9.0f) - 10.0f;
        else
            angThrow.x = angThrow.x * (8.0f / 9.0f) - 10.0f;

        float flVel = (90.0f - angThrow.x) * 6.0f;
        if (flVel > 750.0f)
            flVel = 750.0f;

        UTIL_MakeVectors(angThrow);

        Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16.0f;

        if (m_pPlayer->m_bLeftHanded == 1)
            vecSrc = vecSrc - gpGlobals->v_right * 12.0f;
        else
            vecSrc = vecSrc + gpGlobals->v_right * 12.0f;

        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        float flTime = m_flStartThrow - gpGlobals->time + 3.0f;

        CGrenade::ShootTimed2(m_pPlayer->pev, vecSrc, vecThrow, 1.5f, m_pPlayer->m_iTeam);

        SendWeaponAnim(HEGRENADE_THROW);
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        m_flStartThrow = 0;
        m_flNextPrimaryAttack = gpGlobals->time + 0.5f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.75f;

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

        if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 0.5f;
        }
    }
    else if (m_flReleaseThrow > 0)
    {
        m_flStartThrow = 0;

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            SendWeaponAnim(HEGRENADE_DRAW);
            m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
            m_flReleaseThrow = -1.0f;
        }
        else
        {
            RetireWeapon();
        }
    }
    else
    {

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            SendWeaponAnim(HEGRENADE_IDLE);
            m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
        }
    }
}

