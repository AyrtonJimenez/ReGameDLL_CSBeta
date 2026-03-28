#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum mac10_e
{
    MAC10_IDLE1 = 0,
    MAC10_RELOAD,
    MAC10_DRAW,
    MAC10_SHOOT1,
    MAC10_SHOOT2,
    MAC10_SHOOT3,
};

class CMAC10 : public CBasePlayerWeapon
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

    void MAC10Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_mac10, CMAC10);

void CMAC10::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_mac10");

    Precache();
    m_iId = WEAPON_MAC10;
    SET_MODEL(ENT(pev), "models/w_mac10.mdl");

    m_iDefaultAmmo = 30;
    FallInit();
}

void CMAC10::Precache(void)
{
    PRECACHE_MODEL("models/v_mac10.mdl");
    PRECACHE_MODEL("models/v_mac10_r.mdl");
    PRECACHE_MODEL("models/w_mac10.mdl");
    PRECACHE_MODEL("models/p_mac10.mdl");

    PRECACHE_SOUND("weapons/mac10-1.wav");
    PRECACHE_SOUND("weapons/mac10_clipout.wav");
    PRECACHE_SOUND("weapons/mac10_clipin.wav");
    PRECACHE_SOUND("weapons/mac10_boltpull.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CMAC10::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "45acp";
    p->iMaxAmmo1 = 90;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 13;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_MAC10;
    p->iWeight = 25;
    return 1;
}

BOOL CMAC10::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.15f;
    iShellOn = 1;
    m_bDelayFire = FALSE;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_mac10.mdl", "models/p_mac10.mdl", MAC10_DRAW, "onehanded");
    else
        return DefaultDeploy("models/v_mac10_r.mdl", "models/p_mac10.mdl", MAC10_DRAW, "onehanded");
}

void CMAC10::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        MAC10Fire(0.15f * m_pPlayer->m_flAccuracy, 0.07f, FALSE);
    else
        MAC10Fire(0.03f * m_pPlayer->m_flAccuracy, 0.07f, FALSE);
}

void CMAC10::MAC10Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = ((m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired) / 200) + 0.6f;
    if (m_pPlayer->m_flAccuracy > 1.65f)
        m_pPlayer->m_flAccuracy = 1.65f;

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
        if (anim == 0) SendWeaponAnim(MAC10_SHOOT1);
        else if (anim == 1) SendWeaponAnim(MAC10_SHOOT2);
        else SendWeaponAnim(MAC10_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mac10-1.wav", 0.95f, 0.8f, 0, 100);

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
        KickBack(0.9f, 0.45f, 0.25f, 0.035f, 3.5f, 2.75f, 7);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(1.2f, 0.45f, 0.3f, 0.04f, 3.75f, 2.75f, 7);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.75f, 0.4f, 0.175f, 0.03f, 2.75f, 2.5f, 10);
    else
        KickBack(0.775f, 0.425f, 0.2f, 0.03f, 3.0f, 2.75f, 9);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    Vector vecShellVelocity, vecShellOrigin;
    if (m_pPlayer->m_bLeftHanded == 1)
    {
        float flRightRand = RANDOM_FLOAT(100.0f, 120.0f);
        float flUpRand = RANDOM_FLOAT(25.0f, 75.0f);

        vecShellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * flRightRand
            + gpGlobals->v_up * flUpRand
            + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                + gpGlobals->v_up * -9.0f
                + gpGlobals->v_forward * 32.0f
                - gpGlobals->v_right * 11.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {
        float flRightRand = RANDOM_FLOAT(100.0f, 120.0f);
        float flUpRand = RANDOM_FLOAT(25.0f, 75.0f);

        vecShellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * flRightRand
            + gpGlobals->v_up * flUpRand
            + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                + gpGlobals->v_up * -9.0f
                + gpGlobals->v_forward * 32.0f
                + gpGlobals->v_right * 11.0f;

            EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, 1,
        BULLET_PLAYER_45ACP, 29, 0.82f, NULL, false);
}

void CMAC10::Reload(void)
{
    if (DefaultReload(30, MAC10_RELOAD, 3.3f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
}

void CMAC10::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(MAC10_IDLE1);
}

