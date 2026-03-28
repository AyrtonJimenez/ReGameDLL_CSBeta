#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum m249_e
{
    M249_IDLE1 = 0,
    M249_SHOOT1,
    M249_SHOOT2,
    M249_RELOAD,
    M249_DRAW,
};

class CM249 : public CBasePlayerWeapon
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
    float GetMaxSpeed(void) { return 220.0f; }

    void M249Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_m249, CM249);

void CM249::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_m249");
    Precache();
    m_iId = WEAPON_M249;
    SET_MODEL(ENT(pev), "models/w_m249.mdl");

    m_iDefaultAmmo = 100;
    FallInit();
}

void CM249::Precache(void)
{
    PRECACHE_MODEL("models/v_m249.mdl");
    PRECACHE_MODEL("models/v_m249_r.mdl");
    PRECACHE_MODEL("models/w_m249.mdl");
    PRECACHE_MODEL("models/p_m249.mdl");

    PRECACHE_SOUND("weapons/m249-1.wav");
    PRECACHE_SOUND("weapons/m249-2.wav");
    PRECACHE_SOUND("weapons/m249_boxout.wav");
    PRECACHE_SOUND("weapons/m249_boxin.wav");
    PRECACHE_SOUND("weapons/m249_chain.wav");
    PRECACHE_SOUND("weapons/m249_coverup.wav");
    PRECACHE_SOUND("weapons/m249_coverdown.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CM249::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "556NatoBox";
    p->iMaxAmmo1 = 200;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 100;
    p->iSlot = 0;
    p->iPosition = 4;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_M249;
    p->iWeight = 25;
    return 1;
}

BOOL CM249::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.95f;
    iShellOn = 1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_m249.mdl", "models/p_m249.mdl", M249_DRAW, "m249");
    else
        return DefaultDeploy("models/v_m249_r.mdl", "models/p_m249.mdl", M249_DRAW, "m249");
}

void CM249::PrimaryAttack(void)
{
    float flSpread;
    if (m_pPlayer->pev->flags & FL_ONGROUND)
        flSpread = 0.03f * m_pPlayer->m_flAccuracy;
    else
        flSpread = 0.5f * m_pPlayer->m_flAccuracy;

    M249Fire(flSpread, 0.1f, FALSE);
}

void CM249::M249Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = TRUE;
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 150) + 0.45f;
    if (m_pPlayer->m_flAccuracy > 1.4f)
        m_pPlayer->m_flAccuracy = 1.4f;

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
        int iAnimSelect = RANDOM_LONG(0, 1);
        if (iAnimSelect == 0)
            SendWeaponAnim(M249_SHOOT1);
        else if (iAnimSelect == 1)
            SendWeaponAnim(M249_SHOOT2);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m249-1.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 0.52f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m249-2.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 0.52f, 0, 100);

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
        KickBack(1.8f, 0.55f, 0.35f, 0.1f, 5.0f, 3.0f, 8);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(1.8f, 0.65f, 0.45f, 0.125f, 5.0f, 3.5f, 8);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.75f, 0.325f, 0.25f, 0.0375f, 4.0f, 2.5f, 9);
    else
        KickBack(0.8f, 0.35f, 0.3f, 0.04f, 4.5f, 3.0f, 9);

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
            vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 14.0f;
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
            vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 14.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 35.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 22.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, 2,
        BULLET_PLAYER_556MM, 35, 0.97f, NULL, FALSE);
}

void CM249::Reload(void)
{
    if (DefaultReload(100, M249_RELOAD, 4.6f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
    m_bDelayFire = FALSE;
    m_pPlayer->m_iShotsFired = 0;
}

void CM249::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(M249_IDLE1);
}

