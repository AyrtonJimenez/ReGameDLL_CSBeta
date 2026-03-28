#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum sg552_e
{
    SG552_IDLE1 = 0,
    SG552_RELOAD,
    SG552_DRAW,
    SG552_SHOOT1,
    SG552_SHOOT2,
    SG552_SHOOT3,
};

class CSG552 : public CBasePlayerWeapon
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
    float GetMaxSpeed(void) { return 235.0f; }

    void SG552Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_sg552, CSG552);

void CSG552::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_sg552");
    Precache();
    m_iId = WEAPON_SG552;
    SET_MODEL(ENT(pev), "models/w_sg552.mdl");
    m_iDefaultAmmo = 30;
    FallInit();
}

void CSG552::Precache(void)
{
    PRECACHE_MODEL("models/v_sg552.mdl");
    PRECACHE_MODEL("models/v_sg552_r.mdl");
    PRECACHE_MODEL("models/w_sg552.mdl");
    PRECACHE_MODEL("models/p_sg552.mdl");

    PRECACHE_SOUND("weapons/sg552-1.wav");
    PRECACHE_SOUND("weapons/sg552-2.wav");
    PRECACHE_SOUND("weapons/sg552_clipout.wav");
    PRECACHE_SOUND("weapons/sg552_clipin.wav");
    PRECACHE_SOUND("weapons/sg552_boltpull.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CSG552::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "556Nato";
    p->iMaxAmmo1 = 90;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 10;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_SG552;
    p->iWeight = 25;

    return 1;
}

BOOL CSG552::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;
    iShellOn = 1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_sg552.mdl", "models/p_sg552.mdl", SG552_DRAW, "mp5");
    else
        return DefaultDeploy("models/v_sg552_r.mdl", "models/p_sg552.mdl", SG552_DRAW, "mp5");
}

void CSG552::SecondaryAttack(void)
{
    if (m_pPlayer->m_iFOV == 90)
        m_pPlayer->m_iFOV = 45;
    else
        m_pPlayer->m_iFOV = 90;

    m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CSG552::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        SG552Fire(0.25f * m_pPlayer->m_flAccuracy, 0.0825f, FALSE);
    else if (m_pPlayer->m_iFOV == 90)
        SG552Fire(0.02f * m_pPlayer->m_flAccuracy, 0.0825f, FALSE);
    else
        SG552Fire(0.02f * m_pPlayer->m_flAccuracy, 0.135f, FALSE);
}

void CSG552::SG552Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = TRUE;

    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = (float)(m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 225) + 0.3f;
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
        if (anim == 0) SendWeaponAnim(SG552_SHOOT1);
        else if (anim == 1) SendWeaponAnim(SG552_SHOOT2);
        else SendWeaponAnim(SG552_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = 600;
    m_pPlayer->m_iWeaponFlash = 512;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sg552-1.wav",
            RANDOM_FLOAT(0.92f, 1.0f), 0.56f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sg552-2.wav",
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
        KickBack(0.6f, 0.35f, 0.2f, 0.0125f, 4.75f, 2.0f, 10);
    else
        KickBack(0.625f, 0.375f, 0.25f, 0.0125f, 5.0f, 2.25f, 9);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
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
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 20.0f;
            vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 10.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
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
            vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 20.0f;
            vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 10.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192.0f, 2,
        BULLET_PLAYER_556MM, 33, 0.955f, NULL, FALSE);
}

void CSG552::Reload(void)
{
    if (DefaultReload(30, SG552_RELOAD, 2.5f))
    {
        if (m_pPlayer->m_iFOV != 90)
            SecondaryAttack();

        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
    m_pPlayer->m_iShotsFired = 0;
    m_bDelayFire = FALSE;
}

void CSG552::WeaponIdle(void)
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(SG552_IDLE1);
}

