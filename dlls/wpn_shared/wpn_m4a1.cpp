#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum m4a1_e
{
    M4A1_IDLE1 = 0,
    M4A1_RELOAD,
    M4A1_DRAW,
    M4A1_SHOOT1,
    M4A1_SHOOT2,
    M4A1_SHOOT3,
};

class CM4A1 : public CBasePlayerWeapon
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

    void M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_m4a1, CM4A1);

void CM4A1::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_m4a1");
    Precache();
    m_iId = WEAPON_M4A1;
    SET_MODEL(ENT(pev), "models/w_m4a1.mdl");
    m_iDefaultAmmo = 30;
    FallInit();
}

void CM4A1::Precache(void)
{
    PRECACHE_MODEL("models/v_m4a1.mdl");
    PRECACHE_MODEL("models/v_m4a1_r.mdl");
    PRECACHE_MODEL("models/w_m4a1.mdl");
    PRECACHE_MODEL("models/p_m4a1.mdl");

    PRECACHE_SOUND("weapons/m4a1-1.wav");
    PRECACHE_SOUND("weapons/m4a1-2.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CM4A1::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "556Nato";
    p->iMaxAmmo1 = 90;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 6;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_M4A1;
    p->iWeight = 25;
    return 1;
}

BOOL CM4A1::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;
    m_bDelayFire = TRUE;
    iShellOn = 1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_DRAW, "silenced_rifle");
    else
        return DefaultDeploy("models/v_m4a1_r.mdl", "models/p_m4a1.mdl", M4A1_DRAW, "silenced_rifle");
}

void CM4A1::SecondaryAttack(void)
{
    if (m_pPlayer->m_iFOV == 90)
        m_pPlayer->m_iFOV = 45;
    else
        m_pPlayer->m_iFOV = 90;

    m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CM4A1::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        M4A1Fire(0.25f * m_pPlayer->m_flAccuracy, 0.0875f, FALSE);

    float flCycleTime;
    if (m_pPlayer->m_iFOV == 90)
        flCycleTime = 0.0875f;
    else
        flCycleTime = 0.165f;

    M4A1Fire(0.02f * m_pPlayer->m_flAccuracy, flCycleTime, FALSE);
}

void CM4A1::M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = TRUE;
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.3f;
    if (m_pPlayer->m_flAccuracy > 0.5f)
        m_pPlayer->m_flAccuracy = 0.5f;

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

    if (m_iClip != 0)
    {
        int anim = RANDOM_LONG(0, 2);
        if (anim == 0) SendWeaponAnim(M4A1_SHOOT1);
        else if (anim == 1) SendWeaponAnim(M4A1_SHOOT2);
        else if (anim == 2) SendWeaponAnim(M4A1_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1-1.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 1.28f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m4a1-2.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 1.28f, 0, 100);

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
        KickBack(1.0f, 0.4f, 0.23f, 0.15f, 5.0f, 3.0f, 7);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(1.2f, 0.45f, 0.23f, 0.15f, 5.5f, 3.5f, 6);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.55f, 0.3f, 0.2f, 0.0125f, 4.5f, 1.5f, 10);
    else
        KickBack(0.6f, 0.35f, 0.25f, 0.015f, 4.5f, 1.5f, 10);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
    float flUpScale = RANDOM_FLOAT(35.0f, 55.0f);
    float flForwardScale = RANDOM_FLOAT(40.0f, 55.0f);

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * flRightScale
            + gpGlobals->v_up * flUpScale
            + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                - gpGlobals->v_up * 8.0f
                + gpGlobals->v_forward * 20.0f
                - gpGlobals->v_right * 10.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * flRightScale
            + gpGlobals->v_up * flUpScale
            + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                - gpGlobals->v_up * 8.0f
                + gpGlobals->v_forward * 20.0f
                + gpGlobals->v_right * 10.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, 2,
        BULLET_PLAYER_556MM, 32, 0.97f, NULL, false);
}

void CM4A1::Reload(void)
{
    if (DefaultReload(30, M4A1_RELOAD, 2.2f))
    {
        if (m_pPlayer->m_iFOV != 90)
            SecondaryAttack();

        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
    m_pPlayer->m_iShotsFired = 0;
    m_bDelayFire = FALSE;
}

void CM4A1::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(M4A1_IDLE1);
}

float CM4A1::GetMaxSpeed(void)
{
    if (m_pPlayer->m_iFOV == 90)
        return 230.0f;

    return 200.0f;
}

