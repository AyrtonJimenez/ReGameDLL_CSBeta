#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum aug_e
{
    AUG_IDLE1 = 0,
    AUG_RELOAD,
    AUG_DRAW,
    AUG_SHOOT1,
    AUG_SHOOT2,
    AUG_SHOOT3,
};

class CAUG : public CBasePlayerWeapon
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
    float GetMaxSpeed(void) { return 240.0f; }

    void AUGFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_aug, CAUG);

void CAUG::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_aug");
    Precache();
    m_iId = WEAPON_AUG;
    SET_MODEL(ENT(pev), "models/w_aug.mdl");
    m_iDefaultAmmo = 30;

    FallInit();
}

void CAUG::Precache(void)
{
    PRECACHE_MODEL("models/v_aug.mdl");
    PRECACHE_MODEL("models/v_aug_r.mdl");
    PRECACHE_MODEL("models/w_aug.mdl");
    PRECACHE_MODEL("models/p_aug.mdl");

    PRECACHE_SOUND("weapons/aug-1.wav");
    PRECACHE_SOUND("weapons/aug_clipout.wav");
    PRECACHE_SOUND("weapons/aug_clipin.wav");
    PRECACHE_SOUND("weapons/aug_boltpull.wav");
    PRECACHE_SOUND("weapons/aug_boltslap.wav");
    PRECACHE_SOUND("weapons/aug_forearm.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CAUG::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "556Nato";
    p->iMaxAmmo1 = 90;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 14;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_AUG;
    p->iWeight = 25;
    return 1;
}

BOOL CAUG::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;

    iShellOn = 1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_aug.mdl", "models/p_aug.mdl", AUG_DRAW, "aug");
    else
        return DefaultDeploy("models/v_aug_r.mdl", "models/p_aug.mdl", AUG_DRAW, "aug");
}

void CAUG::SecondaryAttack(void)
{
    if (m_pPlayer->m_iFOV == 90)
        m_pPlayer->m_iFOV = 65;
    else
        m_pPlayer->m_iFOV = 90;

    m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CAUG::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        AUGFire(0.25f * m_pPlayer->m_flAccuracy, 0.0825f, FALSE);
    else if (m_pPlayer->m_iFOV == 90)
        AUGFire(0.02f * m_pPlayer->m_flAccuracy, 0.0825f, FALSE);
    else
        AUGFire(0.02f * m_pPlayer->m_flAccuracy, 0.135f, FALSE);
}

void CAUG::AUGFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = TRUE;
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = ((float)(m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 275)) + 0.3f;
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
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    if (m_iClip != 0)
    {
        int anim = RANDOM_LONG(0, 2);
        if (anim == 0)
            SendWeaponAnim(AUG_SHOOT1);
        else if (anim == 1)
            SendWeaponAnim(AUG_SHOOT2);
        else
            SendWeaponAnim(AUG_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/aug-1.wav",
        RANDOM_FLOAT(0.92f, 1.0f), 0.56f, 0, 100);

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
        KickBack(1.15f, 0.45f, 0.22f, 0.18f, 5.0f, 3.5f, 6);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(1.25f, 0.45f, 0.22f, 0.18f, 5.5f, 4.0f, 5);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.525f, 0.275f, 0.2f, 0.011f, 4.25f, 1.25f, 12);
    else
        KickBack(0.575f, 0.325f, 0.25f, 0.0125f, 4.25f, 1.25f, 12);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        Vector vecShellVelocity;
        float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
        vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(35.0f, 55.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(40.0f, 55.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 8.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 15.0f;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 15.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {
        Vector vecShellVelocity;
        float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
        vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(35.0f, 55.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(40.0f, 55.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 8.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 15.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 15.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192.0f, 2,
        BULLET_PLAYER_556MM, 33, 0.96f, NULL, FALSE);
}

void CAUG::Reload(void)
{
    if (DefaultReload(30, AUG_RELOAD, 3.9f))
    {
        if (m_pPlayer->m_iFOV != 90)
            SecondaryAttack();

        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
    m_pPlayer->m_iShotsFired = 0;
    m_bDelayFire = FALSE;
}

void CAUG::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(AUG_IDLE1);
}

