#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum usp_e
{
    USP_IDLE1 = 0,
    USP_IDLE2,
    USP_IDLE3,
    USP_SHOOT1,
    USP_SHOOT_EMPTY,
    USP_RELOAD,
    USP_DRAW,
    USP_ATTACH_SILENCER,
    USP_UNSIL_IDLE,
    USP_UNSIL_SHOOT1,
    USP_UNSIL_SHOOT2,
    USP_UNSIL_RELOAD,
};

class CUSP : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 2; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void USPFire(float flSpread, float flCycleTime, BOOL fUseSemi);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 250.0f; }
};

LINK_ENTITY_TO_CLASS(weapon_usp, CUSP);

void CUSP::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_usp");
    Precache();
    m_iId = WEAPON_USP;
    SET_MODEL(ENT(pev), "models/w_usp.mdl");
    m_iDefaultAmmo = 12;

    FallInit();
}

void CUSP::Precache(void)
{
    PRECACHE_MODEL("models/v_usp.mdl");
    PRECACHE_MODEL("models/v_usp_r.mdl");
    PRECACHE_MODEL("models/w_usp.mdl");
    PRECACHE_MODEL("models/p_usp.mdl");

    PRECACHE_SOUND("weapons/usp1.wav");
    PRECACHE_SOUND("weapons/usp2.wav");
    PRECACHE_SOUND("weapons/clipout1.wav");
    PRECACHE_SOUND("weapons/clipin1.wav");
    PRECACHE_SOUND("weapons/sliderelease1.wav");
    PRECACHE_SOUND("weapons/slideback1.wav");

    m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CUSP::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "45ACP";
    p->iMaxAmmo1 = 48;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 12;
    p->iSlot = 1;
    p->iPosition = 4;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_USP;
    p->iWeight = 5;

    return 1;
}

BOOL CUSP::Deploy(void)
{
    m_pPlayer->m_flAccuracy = 0.98f;

    int iAnim;
    if (RANDOM_LONG(0, 1))
        iAnim = USP_DRAW;
    else
        iAnim = USP_UNSIL_SHOOT1;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_usp.mdl", "models/p_usp.mdl", iAnim, "tmp");
    else
        return DefaultDeploy("models/v_usp_r.mdl", "models/p_usp.mdl", iAnim, "tmp");
}

void CUSP::PrimaryAttack(void)
{
    float flSpread;
    if (m_pPlayer->pev->velocity.Length2D() > 0)
        flSpread = 0.225f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
        flSpread = 0.3f * (1.0f - m_pPlayer->m_flAccuracy);
    else if (m_pPlayer->pev->flags & FL_DUCKING)
        flSpread = 0.08f * (1.0f - m_pPlayer->m_flAccuracy);
    else
        flSpread = 0.1f * (1.0f - m_pPlayer->m_flAccuracy);

    USPFire(flSpread, 0.25f, FALSE);
}

void CUSP::USPFire(float flSpread, float flCycleTime, BOOL fUseSemi)
{
    if (fUseSemi)
    {
        ++m_pPlayer->m_iShotsFired;
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

    if (m_iClip == 0)
        SendWeaponAnim(USP_SHOOT_EMPTY);
    else if (RANDOM_LONG(0, 1))
        SendWeaponAnim(USP_SHOOT1);
    else
        SendWeaponAnim(USP_UNSIL_SHOOT2);

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    if (pev->body == 1)
    {
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
        if (RANDOM_LONG(0, 1))
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/usp1.wav",
                RANDOM_FLOAT(0.5f, 0.6f), 0.8f, 0, 100);
        else
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/usp2.wav",
                RANDOM_FLOAT(0.5f, 0.6f), 0.8f, 0, 100);
    }
    else
    {
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
        if (RANDOM_LONG(0, 1))
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/usp1.wav",
                RANDOM_FLOAT(0.92f, 1.0f), 1.6f, 0, 100);
        else
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/usp2.wav",
                RANDOM_FLOAT(0.92f, 1.0f), 1.6f, 0, 100);
    }

    Vector vecAiming = gpGlobals->v_forward;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

    m_pPlayer->pev->punchangle.x -= 2.0f;

    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (m_pPlayer->m_bLeftHanded == 1)
    {
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            + gpGlobals->v_right * RANDOM_FLOAT(40.0f, 150.0f)
            + gpGlobals->v_up * RANDOM_FLOAT(70.0f, 170.0f)
            + gpGlobals->v_forward * RANDOM_FLOAT(0.0f, 100.0f);

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12.0f
            + gpGlobals->v_forward * 32.0f
            - gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
    }
    else
    {
        Vector vecShellVelocity = m_pPlayer->pev->velocity
            - gpGlobals->v_right * RANDOM_FLOAT(40.0f, 150.0f)
            + gpGlobals->v_up * RANDOM_FLOAT(70.0f, 170.0f)
            + gpGlobals->v_forward * RANDOM_FLOAT(0.0f, 100.0f);

        Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs
            + gpGlobals->v_up * -12.0f
            + gpGlobals->v_forward * 32.0f
            + gpGlobals->v_right * 12.0f;

        EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
    }

    Vector vecDir = m_pPlayer->FireBullets3(
        vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 4096, 1,
        BULLET_PLAYER_45ACP, 34, 0.83f, NULL, true);
}

void CUSP::Reload(void)
{
    int iAnim;
    if (RANDOM_LONG(0, 1))
        iAnim = USP_RELOAD;
    else
        iAnim = USP_UNSIL_RELOAD;

    if (DefaultReload(12, iAnim, 2.2f))
    {
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
    }

    m_pPlayer->m_flAccuracy = 0.98f;
}

void CUSP::WeaponIdle(void)
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip)
    {
        float flRand = RANDOM_FLOAT(0.0f, 1.0f);

        if (flRand <= 0.3f)
        {
            m_flTimeWeaponIdle = gpGlobals->time + 3.0625f;
            SendWeaponAnim(USP_IDLE3);
        }
        else if (flRand <= 0.6f)
        {
            m_flTimeWeaponIdle = gpGlobals->time + 3.75f;
            SendWeaponAnim(USP_IDLE1);
        }
        else
        {
            m_flTimeWeaponIdle = gpGlobals->time + 2.5f;
            SendWeaponAnim(USP_IDLE2);
        }
    }
}

