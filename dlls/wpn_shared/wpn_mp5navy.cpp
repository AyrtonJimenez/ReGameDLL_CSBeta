#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum mp5n_e
{
    MP5N_IDLE1 = 0,
    MP5N_RELOAD,
    MP5N_DRAW,
    MP5N_SHOOT1,
    MP5N_SHOOT2,
    MP5N_SHOOT3,
};

class CMP5N : public CBasePlayerWeapon
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
    float GetMaxSpeed(void) { return 250.0f; }

    void MP5NFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_mp5navy, CMP5N);

void CMP5N::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_mp5navy");

    Precache();
    m_iId = WEAPON_MP5N;
    SET_MODEL(ENT(pev), "models/w_mp5.mdl");

    m_iDefaultAmmo = 30;
    FallInit();
}

void CMP5N::Precache(void)
{
    PRECACHE_MODEL("models/v_mp5.mdl");
    PRECACHE_MODEL("models/v_mp5_r.mdl");
    PRECACHE_MODEL("models/w_mp5.mdl");
    PRECACHE_MODEL("models/p_mp5.mdl");

    PRECACHE_SOUND("weapons/mp5-1.wav");
    PRECACHE_SOUND("weapons/mp5-2.wav");
    PRECACHE_SOUND("weapons/mp5_clipout.wav");
    PRECACHE_SOUND("weapons/mp5_clipin.wav");
    PRECACHE_SOUND("weapons/mp5_slideback.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CMP5N::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "9mm";
    p->iMaxAmmo1 = 120;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 7;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_MP5N;
    p->iWeight = 25;
    return 1;
}

BOOL CMP5N::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;
    iShellOn = 1;
    m_bDelayFire = FALSE;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_mp5.mdl", "models/p_mp5.mdl", MP5N_DRAW, "mp5");
    else
        return DefaultDeploy("models/v_mp5_r.mdl", "models/p_mp5.mdl", MP5N_DRAW, "mp5");
}

void CMP5N::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        MP5NFire(0.085f * m_pPlayer->m_flAccuracy, 0.075f, FALSE);
    else
        MP5NFire(0.04f * m_pPlayer->m_flAccuracy, 0.075f, FALSE);
}

void CMP5N::MP5NFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = (float)(m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4f;
    if (m_pPlayer->m_flAccuracy > 0.8f)
        m_pPlayer->m_flAccuracy = 0.8f;

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
        if (anim == 0) SendWeaponAnim(MP5N_SHOOT1);
        else if (anim == 1) SendWeaponAnim(MP5N_SHOOT2);
        else SendWeaponAnim(MP5N_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mp5-1.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 0.8f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mp5-2.wav",
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
        KickBack(0.65f, 0.3f, 0.2f, 0.03f, 3.5f, 2.0f, 10);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(0.7f, 0.375f, 0.25f, 0.0325f, 4.0f, 2.0f, 10);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.35f, 0.2f, 0.1f, 0.015f, 3.0f, 1.0f, 10);
    else
        KickBack(0.375f, 0.225f, 0.125f, 0.02f, 3.25f, 1.25f, 10);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(-100.0f, 150.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(-100.0f, 100.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -11.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 32.0f;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 12.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {
        float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(-100.0f, 150.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(-100.0f, 100.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        if (iShellOn)
        {
            Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -11.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 32.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 12.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, 1,
        BULLET_PLAYER_9MM, 26, 0.8f, 0, false);
}

void CMP5N::Reload(void)
{
    if (DefaultReload(30, MP5N_RELOAD, 2.3f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
}

void CMP5N::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(MP5N_IDLE1);
}

