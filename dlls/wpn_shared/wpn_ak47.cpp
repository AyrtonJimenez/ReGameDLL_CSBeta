#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum ak47_e
{
    AK47_IDLE1 = 0,
    AK47_RELOAD,
    AK47_DRAW,
    AK47_SHOOT1,
    AK47_SHOOT2,
    AK47_SHOOT3,
};

class CAK47 : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 1; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void SecondaryAttack(void) {}
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 221.0f; }

    void AK47Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_ak47, CAK47);

void CAK47::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_ak47");
    Precache();
    m_iId = WEAPON_AK47;
    SET_MODEL(ENT(pev), "models/w_ak47.mdl");
    m_iDefaultAmmo = 30;

    FallInit();
}

void CAK47::Precache(void)
{

    PRECACHE_MODEL("models/v_ak47.mdl");
    PRECACHE_MODEL("models/v_ak47_r.mdl");
    PRECACHE_MODEL("models/w_ak47.mdl");
    PRECACHE_MODEL("models/p_ak47.mdl");

    PRECACHE_SOUND("weapons/ak47-1.wav");
    PRECACHE_SOUND("weapons/ak47-2.wav");
    PRECACHE_SOUND("weapons/ak47_clipout.wav");
    PRECACHE_SOUND("weapons/ak47_clipin.wav");
    PRECACHE_SOUND("weapons/ak47_boltpull.wav");

    m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CAK47::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "762Nato";
    p->iMaxAmmo1 = 90;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 1;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_AK47;
    p->iWeight = 25;

    return 1;
}

BOOL CAK47::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.0f;

    iShellOn = 1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_ak47.mdl", "models/p_ak47.mdl", AK47_DRAW, "mp5");
    else
        return DefaultDeploy("models/v_ak47_r.mdl", "models/p_ak47.mdl", AK47_DRAW, "mp5");
}

void CAK47::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        AK47Fire(0.35f * m_pPlayer->m_flAccuracy, 0.0955f, FALSE);
    else
        AK47Fire(0.0275f * m_pPlayer->m_flAccuracy, 0.0955f, FALSE);
}

void CAK47::AK47Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = TRUE;
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = ((float)(m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 200)) + 0.35f;
    if (m_pPlayer->m_flAccuracy > 0.65f)
        m_pPlayer->m_flAccuracy = 0.65f;

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
            SendWeaponAnim(AK47_SHOOT1);
        else if (anim == 1)
            SendWeaponAnim(AK47_SHOOT2);
        else
            SendWeaponAnim(AK47_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = 600;
    m_pPlayer->m_iWeaponFlash = 512;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak47-1.wav", 1.0f, 0.4f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak47-2.wav", 1.0f, 0.4f, 0, 100);

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
        KickBack(1.5f, 0.45f, 0.25f, 0.185f, 7.0f, 4.0f, 6);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(2.0f, 1.0f, 0.5f, 0.35f, 9.0f, 6.0f, 5);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.9f, 0.35f, 0.15f, 0.025f, 5.5f, 1.5f, 9);
    else
        KickBack(1.0f, 0.375f, 0.175f, 0.0375f, 5.75f, 1.75f, 8);

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
        BULLET_PLAYER_762MM, 37, 0.98f, NULL, FALSE);
}

void CAK47::Reload(void)
{
    if (DefaultReload(30, AK47_RELOAD, 2.75f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
    m_pPlayer->m_iShotsFired = 0;
    m_bDelayFire = FALSE;
}

void CAK47::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(AK47_IDLE1);
}

