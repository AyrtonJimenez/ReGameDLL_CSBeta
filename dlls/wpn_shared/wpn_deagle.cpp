#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum deagle_e
{
    DEAGLE_IDLE1 = 0,
    DEAGLE_SHOOT1,
    DEAGLE_SHOOT2,
    DEAGLE_SHOOT_EMPTY,
    DEAGLE_RELOAD,
    DEAGLE_DRAW,
};

class CDEAGLE : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 2; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 250.0f; }

    void DEAGLEFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_deagle, CDEAGLE);

void CDEAGLE::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_deagle");
    Precache();
    m_iId = WEAPON_DEAGLE;
    SET_MODEL(ENT(pev), "models/w_deagle.mdl");
    m_iDefaultAmmo = 7;

    FallInit();
}

void CDEAGLE::Precache(void)
{
    PRECACHE_MODEL("models/v_deagle.mdl");
    PRECACHE_MODEL("models/v_deagle_r.mdl");
    PRECACHE_MODEL("models/w_deagle.mdl");
    PRECACHE_MODEL("models/p_deagle.mdl");

    PRECACHE_SOUND("weapons/deagle-1.wav");
    PRECACHE_SOUND("weapons/deagle-2.wav");
    PRECACHE_SOUND("weapons/de_clipout.wav");
    PRECACHE_SOUND("weapons/de_clipin.wav");
    PRECACHE_SOUND("weapons/de_deploy.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CDEAGLE::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "50AE";
    p->iMaxAmmo1 = 35;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 7;
    p->iSlot = 1;
    p->iPosition = 1;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_DEAGLE;
    p->iWeight = 7;

    return 1;
}

BOOL CDEAGLE::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.875f;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_deagle.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded");
    else
        return DefaultDeploy("models/v_deagle_r.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded");
}

void CDEAGLE::PrimaryAttack(void)
{
    float flSpread;
    if (m_pPlayer->pev->velocity.Length2D() > 0)
        flSpread = 0.25f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        flSpread = 0.35f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        flSpread = 0.115f * (1.0f - m_pPlayer->m_flAccuracy);
    else
        flSpread = 0.13f * (1.0f - m_pPlayer->m_flAccuracy);

    DEAGLEFire(flSpread, 0.335f, FALSE);
}

void CDEAGLE::DEAGLEFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    if (fUseAutoAim == 1)
    {
        m_pPlayer->m_iShotsFired++;
        if (m_pPlayer->m_iShotsFired > 1)
            return;
    }

    if (m_pPlayer->m_flLastFire != 0)
    {
        m_pPlayer->m_flAccuracy = (gpGlobals->time - m_pPlayer->m_flLastFire) * 0.2f + 0.6f;
        if (m_pPlayer->m_flAccuracy > 0.875f)
            m_pPlayer->m_flAccuracy = 0.875f;
    }
    m_pPlayer->m_flLastFire = gpGlobals->time;

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

    if (m_iClip == 0)
        SendWeaponAnim(DEAGLE_SHOOT_EMPTY);
    else if (RANDOM_LONG(0, 1))
        SendWeaponAnim(DEAGLE_SHOOT1);
    else
        SendWeaponAnim(DEAGLE_SHOOT2);

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = 2048;
    m_pPlayer->m_iWeaponFlash = 512;

    int iResult = RANDOM_LONG(0, 1);
    if (iResult == 0)
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/deagle-1.wav",
            RANDOM_FLOAT(0.9f, 1.0f), 0.6f, 0, 100);
    else if (iResult == 1)
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/deagle-2.wav",
            RANDOM_FLOAT(0.9f, 1.0f), 0.6f, 0, 100);

    Vector vecAiming = gpGlobals->v_forward;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

    m_pPlayer->pev->punchangle.x -= 2.0f;

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        float fR = RANDOM_FLOAT(50.0f, 70.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * fR
            + gpGlobals->v_up * RANDOM_FLOAT(100.0f, 150.0f)
            + gpGlobals->v_forward * 25.0f;

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12.0f
            + gpGlobals->v_forward * 32.0f
            - gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
    }
    else
    {
        float fR = RANDOM_FLOAT(50.0f, 70.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * fR
            + gpGlobals->v_up * RANDOM_FLOAT(100.0f, 150.0f)
            + gpGlobals->v_forward * 25.0f;

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12.0f
            + gpGlobals->v_forward * 32.0f
            + gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
    }

    Vector vecDir = m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 4096, 2,
        BULLET_PLAYER_50AE, 54, 0.8f, NULL, true);
}

void CDEAGLE::Reload(void)
{
    if (DefaultReload(7, DEAGLE_RELOAD, 2.1f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.98f;
}

void CDEAGLE::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip)
    {
        m_flTimeWeaponIdle = gpGlobals->time + 3.75f;
        SendWeaponAnim(DEAGLE_IDLE1);
    }
}

