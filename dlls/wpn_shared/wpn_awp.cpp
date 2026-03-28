#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum awp_e
{
    AWP_IDLE = 0,
    AWP_SHOOT1,
    AWP_SHOOT2,
    AWP_RELOAD,
    AWP_DRAW,
};

class CAWP : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 1; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void SecondaryAttack(void);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void);

    void AWPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_awp, CAWP);

void CAWP::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_awp");
    Precache();
    m_iId = WEAPON_AWP;
    SET_MODEL(ENT(pev), "models/w_awp.mdl");
    m_iDefaultAmmo = 10;
    FallInit();
}

void CAWP::Precache(void)
{
    PRECACHE_MODEL("models/v_awp.mdl");
    PRECACHE_MODEL("models/v_awp_r.mdl");
    PRECACHE_MODEL("models/w_awp.mdl");
    PRECACHE_MODEL("models/p_awp.mdl");

    PRECACHE_SOUND("weapons/awp1.wav");
    PRECACHE_SOUND("weapons/boltpull1.wav");
    PRECACHE_SOUND("weapons/boltup.wav");
    PRECACHE_SOUND("weapons/boltdown.wav");
    PRECACHE_SOUND("weapons/zoom.wav");
    PRECACHE_SOUND("weapons/awp_deploy.wav");
    PRECACHE_SOUND("weapons/awp_clipin.wav");
    PRECACHE_SOUND("weapons/awp_clipout.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
    m_iShellId = m_iShell;
}

int CAWP::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "338Magnum";
    p->iMaxAmmo1 = 30;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 10;
    p->iSlot = 0;
    p->iPosition = 2;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_AWP;
    p->iWeight = 30;
    return 1;
}

BOOL CAWP::Deploy(void)
{
    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_awp.mdl", "models/p_awp.mdl", AWP_DRAW, "sniper");
    else
        return DefaultDeploy("models/v_awp_r.mdl", "models/p_awp.mdl", AWP_DRAW, "sniper");
}

void CAWP::SecondaryAttack(void)
{
    m_pPlayer->pev->viewmodel = 0;

    switch (m_pPlayer->m_iFOV)
    {
    case 90:
        m_pPlayer->m_iFOV = 40;
        break;
    case 40:
        m_pPlayer->m_iFOV = 10;
        break;
    case 10:

        if (m_pPlayer->m_bLeftHanded == 1)
            m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
        else
            m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp_r.mdl");
        m_pPlayer->m_iFOV = 90;
        break;
    }

    m_pPlayer->ResetMaxSpeed();

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1.0f, 0.8f, 0, 100);

    m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CAWP::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        AWPFire(0.85f, 1.9f, FALSE);
    else if (m_pPlayer->pev->velocity.Length2D() > 170.0f)
        AWPFire(0.65f, 1.9f, FALSE);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        AWPFire(0.0f, 1.9f, FALSE);
    else
        AWPFire(0.007f, 1.9f, FALSE);
}

void CAWP::AWPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    if (m_pPlayer->m_iFOV == 90)
    {
        flSpread += 0.025f;
    }
    else
    {
        if (m_pPlayer->m_bLeftHanded == 1)
            m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
        else
            m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp_r.mdl");

        m_pPlayer->m_bResumeZoom = TRUE;
        m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
        m_pPlayer->m_iFOV = 90;
    }

    if (m_iClip <= 0)
    {
        if (m_fFireOnEmpty)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = gpGlobals->time + 0.2f;
        }
        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    if (RANDOM_LONG(0, 1))
        SendWeaponAnim(AWP_SHOOT1);
    else
        SendWeaponAnim(AWP_SHOOT2);

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55f;
    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

    int pitch = RANDOM_LONG(0, 3) + 98;
    float volume = RANDOM_FLOAT(0.92f, 1.0f);
    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/awp1.wav",
        volume, 0.8f, 0, pitch);

    Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

    m_pPlayer->pev->punchangle.x -= 2.0f;

    Vector vecSrc = m_pPlayer->GetGunPosition();

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192.0f, 3,
        BULLET_PLAYER_338MAG, 200, 0.99f, NULL, TRUE);
}

void CAWP::Reload(void)
{
    if (DefaultReload(10, AWP_RELOAD, 2.5f))
    {
        if (m_pPlayer->m_iFOV != 90)
        {
            m_pPlayer->m_iFOV = 10;
            SecondaryAttack();
        }

        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }
}

void CAWP::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip)
    {
        m_flTimeWeaponIdle = gpGlobals->time + 60.0f;
        SendWeaponAnim(AWP_IDLE);
    }
}

float CAWP::GetMaxSpeed(void)
{
    if (m_pPlayer->m_iFOV == 90)
        return 210.0f;
    return 150.0f;
}

