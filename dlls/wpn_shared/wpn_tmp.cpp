#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum tmp_e
{
    TMP_IDLE1 = 0,
    TMP_RELOAD,
    TMP_DRAW,
    TMP_SHOOT1,
    TMP_SHOOT2,
    TMP_SHOOT3,
};

class CTMP : public CBasePlayerWeapon
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

    void TMPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
};

LINK_ENTITY_TO_CLASS(weapon_tmp, CTMP);

void CTMP::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_tmp");
    Precache();
    m_iId = WEAPON_TMP;
    SET_MODEL(ENT(pev), "models/w_tmp.mdl");
    m_iDefaultAmmo = 30;

    FallInit();
}

void CTMP::Precache(void)
{
    PRECACHE_MODEL("models/v_tmp.mdl");
    PRECACHE_MODEL("models/v_tmp_r.mdl");
    PRECACHE_MODEL("models/w_tmp.mdl");
    PRECACHE_MODEL("models/p_tmp.mdl");

    PRECACHE_SOUND("weapons/tmp-1.wav");
    PRECACHE_SOUND("weapons/tmp-2.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CTMP::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "9mm";
    p->iMaxAmmo1 = 120;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 30;
    p->iSlot = 0;
    p->iPosition = 11;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_TMP;
    p->iWeight = 25;

    return 1;
}

BOOL CTMP::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.95f;
    iShellOn = 1;
    m_bDelayFire = FALSE;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_tmp.mdl", "models/p_tmp.mdl", TMP_DRAW, "tmp");
    else
        return DefaultDeploy("models/v_tmp_r.mdl", "models/p_tmp.mdl", TMP_DRAW, "tmp");
}

void CTMP::PrimaryAttack(void)
{
    if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        TMPFire(0.3f * m_pPlayer->m_flAccuracy, 0.07f, FALSE);
    else
        TMPFire(0.03f * m_pPlayer->m_flAccuracy, 0.07f, FALSE);
}

void CTMP::TMPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_pPlayer->m_iShotsFired++;

    m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 200) + 0.55f;
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

    if (m_iClip != 0)
    {
        int anim = RANDOM_LONG(0, 2);
        if (anim == 0) SendWeaponAnim(TMP_SHOOT1);
        else if (anim == 1) SendWeaponAnim(TMP_SHOOT2);
        else SendWeaponAnim(TMP_SHOOT3);
    }

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_iWeaponVolume = 600;

    if (RANDOM_LONG(0, 1))
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tmp-1.wav",
            0.6f, 1.6f, 0, 100);
    else
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tmp-2.wav",
            0.6f, 1.6f, 0, 100);

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
        KickBack(0.8f, 0.4f, 0.2f, 0.03f, 3.0f, 2.5f, 7);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        KickBack(1.0f, 0.4f, 0.25f, 0.035f, 3.5f, 2.5f, 7);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        KickBack(0.7f, 0.35f, 0.125f, 0.025f, 2.5f, 2.0f, 10);
    else
        KickBack(0.725f, 0.375f, 0.15f, 0.025f, 2.75f, 2.25f, 9);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    Vector vecShellVelocity, ShellOrigin;
    float fR = RANDOM_FLOAT(100.0f, 120.0f);
    float fU = RANDOM_FLOAT(5.0f, 55.0f);

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        vecShellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * fR
            - gpGlobals->v_up * fU
            + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            ShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                + gpGlobals->v_up * -6.0f
                + gpGlobals->v_forward * 32.0f
                - gpGlobals->v_right * 11.0f;

            EjectBrass2(ShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }
    else
    {

        vecShellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * fR
            - gpGlobals->v_up * fU
            + gpGlobals->v_forward * 25.0f;

        if (iShellOn)
        {
            ShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
                + gpGlobals->v_up * -6.0f
                + gpGlobals->v_forward * 32.0f
                + gpGlobals->v_right * 11.0f;

            EjectBrass2(ShellOrigin, vecShellVelocity, pev->angles[1], m_iShell, 1, m_pPlayer->pev);
        }
    }

    iShellOn = 1 - iShellOn;

    m_pPlayer->FireBullets3(vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread),
        8192, 1, BULLET_PLAYER_9MM, 20, 0.85f, 0, false);
}

void CTMP::Reload(void)
{
    if (DefaultReload(30, TMP_RELOAD, 2.0f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.0f;
}

void CTMP::WeaponIdle(void)
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    m_flTimeWeaponIdle = gpGlobals->time + 20.0f;
    SendWeaponAnim(TMP_IDLE1);
}

