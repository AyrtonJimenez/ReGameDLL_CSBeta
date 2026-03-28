#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum p90_e
{
    P90_IDLE1 = 0,
    P90_RELOAD,
    P90_DRAW,
    P90_SHOOT1,
    P90_SHOOT2,
    P90_SHOOT3,
};

class CP90 : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 1; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 245.0f; }

    void P90Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_p90, CP90);

void CP90::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_p90");

    Precache();
    m_iId = WEAPON_P90;
    SET_MODEL(ENT(pev), "models/w_p90.mdl");

    m_iDefaultAmmo = 50;
    FallInit();
}

void CP90::Precache(void)
{
    PRECACHE_MODEL("models/v_p90.mdl");
    PRECACHE_MODEL("models/v_p90_r.mdl");
    PRECACHE_MODEL("models/w_p90.mdl");
    PRECACHE_MODEL("models/p_p90.mdl");

    PRECACHE_SOUND("weapons/p90-1.wav");
    PRECACHE_SOUND("weapons/p90_clipout.wav");
    PRECACHE_SOUND("weapons/p90_clipin.wav");
    PRECACHE_SOUND("weapons/p90_boltpull.wav");
    PRECACHE_SOUND("weapons/p90_cliprelease.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CP90::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "57mm";
    p->iMaxAmmo1 = 100;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 50;
    p->iSlot = 0;
    p->iPosition = 8;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_P90;
    p->iWeight = 26;
    return 1;
}

BOOL CP90::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;
    iShellOn = 1;
    m_bDelayFire = FALSE;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_p90.mdl", "models/p_p90.mdl", P90_DRAW, "p90");
    else
        return DefaultDeploy("models/v_p90_r.mdl", "models/p_p90.mdl", P90_DRAW, "p90");
}

void CP90::PrimaryAttack(void)
{
    float flSpread;
    if (m_pPlayer->pev->flags & FL_ONGROUND)
    {
        if (m_pPlayer->pev->velocity.Length2D() > 170)
            flSpread = 0.15f * m_pPlayer->m_flAccuracy;
        else
            flSpread = 0.045f * m_pPlayer->m_flAccuracy;
    }
    else
    {
        flSpread = 0.4f * m_pPlayer->m_flAccuracy;
    }

    P90Fire(flSpread, 0.066f, FALSE);
}

void CP90::P90Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = ((float)(m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 175)) + 0.45f;
    if (m_pPlayer->m_flAccuracy > 1.0f)
        m_pPlayer->m_flAccuracy = 1.0f;

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

    if (m_iClip != 0)
    {
        int anim = RANDOM_LONG(0, 1);
        if (anim == 0)
            SendWeaponAnim(P90_SHOOT1);
        else
            SendWeaponAnim(P90_SHOOT2);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = 600;
    m_pPlayer->m_iWeaponFlash = 128;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/p90-1.wav",
        RANDOM_FLOAT(0.92f, 1.0f), 0.8f, 0, 100);

    Vector vecAiming;
    if (fUseAutoAim)
        vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
    else
        vecAiming = gpGlobals->v_forward;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

    if (m_pPlayer->pev->velocity.Length2D() > 0)
        KickBack(0.65f, 0.3f, 0.2f, 0.0275f, 4.0f, 2.25f, 4);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(0.65f, 0.35f, 0.25f, 0.03f, 4.25f, 2.5f, 3);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.575f, 0.25f, 0.125f, 0.02f, 3.5f, 1.0f, 8);
    else
        KickBack(0.6f, 0.275f, 0.125f, 0.02f, 3.75f, 1.25f, 6);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        Vector vecShellVelocity;
        float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
        vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(50.0f, 90.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -14.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 35.0f;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 22.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {
        Vector vecShellVelocity;
        float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
        vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(50.0f, 90.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -14.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 35.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 22.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192.0f, 1,
        BULLET_PLAYER_57MM, 22, 0.9f, NULL, FALSE);
}

void CP90::Reload(void)
{
    if (DefaultReload(50, P90_RELOAD, 3.5f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
}

void CP90::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(P90_IDLE1);
}

