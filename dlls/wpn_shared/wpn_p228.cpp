#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum p228_e
{
    P228_IDLE = 0,
    P228_SHOOT1,
    P228_SHOOT2,
    P228_SHOOT3,
    P228_SHOOT_EMPTY,
    P228_RELOAD,
    P228_DRAW,
};

class CP228 : public CBasePlayerWeapon
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

    void P228Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_p228, CP228);

void CP228::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_p228");

    Precache();
    m_iId = WEAPON_P228;
    SET_MODEL(ENT(pev), "models/w_p228.mdl");

    m_iDefaultAmmo = 13;
    FallInit();
}

void CP228::Precache(void)
{
    PRECACHE_MODEL("models/v_p228.mdl");
    PRECACHE_MODEL("models/v_p228_r.mdl");
    PRECACHE_MODEL("models/w_p228.mdl");
    PRECACHE_MODEL("models/p_p228.mdl");

    PRECACHE_SOUND("weapons/p228-1.wav");
    PRECACHE_SOUND("weapons/p228_clipout.wav");
    PRECACHE_SOUND("weapons/p228_clipin.wav");
    PRECACHE_SOUND("weapons/p228_sliderelease.wav");
    PRECACHE_SOUND("weapons/p228_slidepull.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CP228::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "357SIG";
    p->iMaxAmmo1 = 52;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 13;
    p->iSlot = 1;
    p->iPosition = 3;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_P228;
    p->iWeight = 5;
    return 1;
}

BOOL CP228::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.98f;

    if (m_pPlayer->m_bLeftHanded)
        return DefaultDeploy("models/v_p228.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded");
    else
        return DefaultDeploy("models/v_p228_r.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded");
}

void CP228::PrimaryAttack(void)
{
    float flSpread;

    if (m_pPlayer->pev->velocity.Length2D() > 0)
        flSpread = 0.255f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        flSpread = 0.35f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        flSpread = 0.075f * (1.0f - m_pPlayer->m_flAccuracy);
    else
        flSpread = 0.15f * (1.0f - m_pPlayer->m_flAccuracy);

    P228Fire(flSpread, 0.225f, FALSE);
}

void CP228::P228Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    if (fUseAutoAim)
    {
        m_pPlayer->m_iShotsFired++;
        if (m_pPlayer->m_iShotsFired > 1)
            return;
    }

    if (m_pPlayer->m_flLastFire != 0)
    {
        m_pPlayer->m_flAccuracy = (gpGlobals->time - m_pPlayer->m_flLastFire) * 0.3f + 0.7f;
        if (m_pPlayer->m_flAccuracy > 0.98f)
            m_pPlayer->m_flAccuracy = 0.98f;
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
        SendWeaponAnim(P228_SHOOT_EMPTY);
    else
    {
        int iAnim = RANDOM_LONG(0, 2);
        if (iAnim == 0)
            SendWeaponAnim(P228_SHOOT1);
        else if (iAnim == 1)
            SendWeaponAnim(P228_SHOOT2);
        else
            SendWeaponAnim(P228_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/p228-1.wav",
        RANDOM_FLOAT(0.5f, 0.6f), 0.8f, 0, 100);

    Vector vecAiming = gpGlobals->v_forward;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

    m_pPlayer->pev->punchangle.x -= 2.0f;

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded)
    {
        float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(100.0f, 260.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(-100.0f, 100.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
        vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -12.0f;
        vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 32.0f;
        vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
    }
    else
    {
        float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
        Vector vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

        float flUpScale = RANDOM_FLOAT(100.0f, 260.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

        float flForwardScale = RANDOM_FLOAT(-100.0f, 100.0f);
        vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
        vecShellOrigin = vecShellOrigin + gpGlobals->v_up * -12.0f;
        vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 32.0f;
        vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
    }

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 4096, 1,
        BULLET_PLAYER_357SIG, 31, 0.9f, NULL, true);
}

void CP228::Reload(void)
{
    if (DefaultReload(13, P228_RELOAD, 2.7f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.98f;
}

void CP228::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip)
    {
        m_flTimeWeaponIdle = gpGlobals->time + 3.0625f;
        SendWeaponAnim(P228_IDLE);
    }
}

