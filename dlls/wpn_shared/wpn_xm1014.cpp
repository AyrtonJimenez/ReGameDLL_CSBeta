#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define XM1014_DRAW			6
#define XM1014_SHOOT1		1
#define XM1014_SHOOT2		2
#define XM1014_INSERT		3
#define XM1014_AFTER_RELOAD	4
#define XM1014_START_RELOAD	5
#define XM1014_IDLE			0

class CXM1014 : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int Save(CSave& save);
    int Restore(CRestore& restore);
    static TYPEDESCRIPTION m_SaveData[];
    BOOL Deploy(void);
    void PrimaryAttack(void);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 240.0f; }
    int iItemSlot(void) { return 1; }

    int m_fInSpecialReload;
    float m_flNextReload;
    int m_iShell;
    float m_flPumpTime;
};

LINK_ENTITY_TO_CLASS(weapon_xm1014, CXM1014);

TYPEDESCRIPTION CXM1014::m_SaveData[] =
{
    DEFINE_FIELD(CXM1014, m_flNextReload, FIELD_TIME),
    DEFINE_FIELD(CXM1014, m_fInSpecialReload, FIELD_INTEGER),
    DEFINE_FIELD(CXM1014, m_flNextReload, FIELD_TIME),
    DEFINE_FIELD(CXM1014, m_flPumpTime, FIELD_TIME),
};
IMPLEMENT_SAVERESTORE(CXM1014, CBasePlayerWeapon);

void CXM1014::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_xm1014");
    Precache();
    m_iId = WEAPON_XM1014;
    SET_MODEL(ENT(pev), "models/w_xm1014.mdl");
    m_iDefaultAmmo = 7;

    FallInit();
}

void CXM1014::Precache(void)
{
    PRECACHE_MODEL("models/v_xm1014.mdl");
    PRECACHE_MODEL("models/v_xm1014_r.mdl");
    PRECACHE_MODEL("models/w_xm1014.mdl");
    PRECACHE_MODEL("models/p_xm1014.mdl");

    m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

    PRECACHE_SOUND("weapons/xm1014-1.wav");
    PRECACHE_SOUND("weapons/reload1.wav");
    PRECACHE_SOUND("weapons/reload3.wav");
}

int CXM1014::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "buckshot";
    p->iMaxAmmo1 = 32;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 7;
    p->iSlot = 0;
    p->iPosition = 12;
    p->iFlags = 0;
    m_iId = WEAPON_XM1014;
    p->iId = WEAPON_XM1014;
    p->iWeight = 20;

    return 1;
}

BOOL CXM1014::Deploy(void)
{
    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_xm1014.mdl", "models/p_xm1014.mdl", XM1014_DRAW, "mp5");
    else
        return DefaultDeploy("models/v_xm1014_r.mdl", "models/p_xm1014.mdl", XM1014_DRAW, "mp5");
}

void CXM1014::PrimaryAttack(void)
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = gpGlobals->time + 0.15f;
        return;
    }

    if (m_iClip <= 0)
    {
        Reload();
        if (m_iClip == 0)
            PlayEmptySound();
        return;
    }

    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

    m_iClip--;

    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    if (RANDOM_LONG(0, 1))
        SendWeaponAnim(XM1014_SHOOT1);
    else
        SendWeaponAnim(XM1014_SHOOT2);

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.1f;

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/xm1014-1.wav",
        RANDOM_FLOAT(0.95f, 1.0f), 0.4f, 0, 100);

    Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    if (m_iClip != 0)
        *(float*)&iShellOn = gpGlobals->time + 0.125f;

    m_flNextPrimaryAttack = gpGlobals->time + 0.25f;
    m_flNextSecondaryAttack = gpGlobals->time + 0.25f;

    if (m_iClip != 0)
        m_flTimeWeaponIdle = gpGlobals->time + 5.0f;
    else
        m_flTimeWeaponIdle = 0.75f;

    m_bDelayFire = FALSE;

    m_pPlayer->pev->punchangle.x -= (float)RANDOM_LONG(3, 5);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    m_pPlayer->FireBullets(6, vecSrc, vecAiming,
        Vector(0.0725f, 0.0725f, 0.0f), 3048, BULLET_PLAYER_BUCKSHOT, 0, 0, 0);
}

void CXM1014::Reload(void)
{
    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        return;

    if (m_iClip == 7)
        return;

    if (gpGlobals->time < m_flNextReload)
        return;

    if (gpGlobals->time < m_flNextPrimaryAttack)
        return;

    if (m_fInSpecialReload == 0)
    {
        SendWeaponAnim(XM1014_START_RELOAD);
        m_fInSpecialReload = 1;

        m_pPlayer->m_flNextAttack = gpGlobals->time + 0.4f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.4f;
        m_flNextPrimaryAttack = gpGlobals->time + 1.0f;
        m_flNextSecondaryAttack = gpGlobals->time + 1.0f;
    }
    else if (m_fInSpecialReload == 1)
    {
        if (gpGlobals->time < m_flTimeWeaponIdle)
            return;

        m_fInSpecialReload = 2;

        if (RANDOM_LONG(0, 1))
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM,
                "weapons/reload1.wav", 1.0f, 0.8f, 0, RANDOM_LONG(0, 31) + 85);
        else
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM,
                "weapons/reload3.wav", 1.0f, 0.8f, 0, RANDOM_LONG(0, 31) + 85);

        SendWeaponAnim(XM1014_INSERT);

        m_flNextReload = gpGlobals->time + 0.3f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.3f;
    }
    else
    {
        m_iClip++;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_fInSpecialReload = 1;
    }
}

void CXM1014::WeaponIdle(void)
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

    if (m_flPumpTime > 0 && m_flPumpTime < gpGlobals->time)
        m_flPumpTime = 0;

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip == 0 && m_fInSpecialReload == 0 &&
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        Reload();
        return;
    }

    if (m_fInSpecialReload == 0)
    {
        SendWeaponAnim(XM1014_IDLE);
        return;
    }

    if (m_iClip != 7 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        Reload();
        return;
    }

    SendWeaponAnim(XM1014_AFTER_RELOAD);
    m_fInSpecialReload = 0;
    m_flTimeWeaponIdle = gpGlobals->time + 1.5f;
}

