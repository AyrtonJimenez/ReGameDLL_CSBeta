#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum m3_e
{
    M3_IDLE = 0,
    M3_FIRE1,
    M3_FIRE2,
    M3_RELOAD,
    M3_PUMP,
    M3_START_RELOAD,
    M3_DRAW,
    M3_HOLSTER,
};

class CM3 : public CBasePlayerWeapon
{
public:
    int Save(CSave& save);
    int Restore(CRestore& restore);
    static TYPEDESCRIPTION m_SaveData[];

    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 1; }

    BOOL Deploy(void);
    void PrimaryAttack(void);
    void Reload(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 230.0f; }

    int m_fInSpecialReload;

    float m_flNextReload;
    int m_iShell;

    float m_flPumpTime;
};

LINK_ENTITY_TO_CLASS(weapon_m3, CM3);

TYPEDESCRIPTION CM3::m_SaveData[] =
{
    DEFINE_FIELD(CM3, m_flNextReload, FIELD_TIME),
    DEFINE_FIELD(CM3, m_fInSpecialReload, FIELD_INTEGER),
    DEFINE_FIELD(CM3, m_flNextReload, FIELD_TIME),
    DEFINE_FIELD(CM3, m_flPumpTime, FIELD_TIME),
};
IMPLEMENT_SAVERESTORE(CM3, CBasePlayerWeapon);

void CM3::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_m3");
    Precache();
    m_iId = WEAPON_M3;
    SET_MODEL(ENT(pev), "models/w_m3.mdl");
    m_iDefaultAmmo = 8;
    FallInit();
}

void CM3::Precache(void)
{
    PRECACHE_MODEL("models/v_m3.mdl");
    PRECACHE_MODEL("models/v_m3_r.mdl");
    PRECACHE_MODEL("models/w_m3.mdl");
    PRECACHE_MODEL("models/p_m3.mdl");

    m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

    PRECACHE_SOUND("weapons/m3-1.wav");
    PRECACHE_SOUND("weapons/reload1.wav");
    PRECACHE_SOUND("weapons/reload3.wav");
}

int CM3::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "buckshot";
    p->iMaxAmmo1 = 32;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 8;
    p->iSlot = 0;
    p->iPosition = 5;
    p->iFlags = 0;
    m_iId = WEAPON_M3;
    p->iId = WEAPON_M3;
    p->iWeight = 20;
    return 1;
}

BOOL CM3::Deploy(void)
{
    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_m3.mdl", "models/p_m3.mdl", M3_DRAW, "mp5");
    else
        return DefaultDeploy("models/v_m3_r.mdl", "models/p_m3.mdl", M3_DRAW, "mp5");
}

void CM3::PrimaryAttack(void)
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
        SendWeaponAnim(M3_FIRE1);
    else
        SendWeaponAnim(M3_FIRE2);

    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m3-1.wav",
        RANDOM_FLOAT(0.95f, 1.0f), 0.4f, 0, 100);

    Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

    if (m_iClip != 0)
        m_flPumpTime = gpGlobals->time + 0.5f;

    m_flNextPrimaryAttack = gpGlobals->time + 0.675f;
    m_flNextSecondaryAttack = gpGlobals->time + 0.675f;

    if (m_iClip != 0)
        m_flTimeWeaponIdle = gpGlobals->time + 5.0f;
    else
        m_flTimeWeaponIdle = 0.75f;

    m_fInSpecialReload = 0;

    m_pPlayer->pev->punchangle.x -= (float)RANDOM_LONG(4, 6);

    Vector vecSrc = m_pPlayer->GetGunPosition();

    m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45f;

    m_pPlayer->FireBullets(8, vecSrc, vecAiming,
        Vector(0.0675f, 0.0675f, 0.0f), 3000, BULLET_PLAYER_BUCKSHOT, 0, 0, 0);
}

void CM3::Reload(void)
{
    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        return;

    if (m_iClip == 8)
        return;

    if (gpGlobals->time < m_flNextReload)
        return;

    if (gpGlobals->time < m_flNextPrimaryAttack)
        return;

    if (m_fInSpecialReload == 0)
    {

        SendWeaponAnim(M3_START_RELOAD);
        m_fInSpecialReload = 1;

        m_pPlayer->m_flNextAttack = gpGlobals->time + 0.45f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.45f;
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

        SendWeaponAnim(M3_RELOAD);

        m_flNextReload = gpGlobals->time + 0.35f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.35f;
    }
    else
    {
        m_iClip++;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_fInSpecialReload = 1;
    }
}

void CM3::WeaponIdle(void)
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

    if (m_flPumpTime > 0 && m_flPumpTime < gpGlobals->time)
        m_flPumpTime = 0;

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_iClip == 0 && m_fInSpecialReload == 0
        && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        Reload();
        return;
    }

    if (m_fInSpecialReload != 0)
    {
        if (m_iClip != 8 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
        {
            Reload();
            return;
        }

        SendWeaponAnim(M3_PUMP);
        m_fInSpecialReload = 0;
        m_flTimeWeaponIdle = gpGlobals->time + 1.5f;
        return;
    }

    SendWeaponAnim(M3_IDLE);
}

