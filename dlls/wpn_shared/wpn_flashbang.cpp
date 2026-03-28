#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum flashbang_e
{
    FLASHBANG_IDLE = 0,
    FLASHBANG_PULLPIN,
    FLASHBANG_THROW,
    FLASHBANG_DRAW,
};

class CFlashbang : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 4; }

    BOOL Deploy(void);
    BOOL CanHolster(void) { return TRUE; }
    void Holster(int skiplocal = 0);
    void PrimaryAttack(void);
    void WeaponIdle(void);
    BOOL CanDrop(void) { return FALSE; }
    float GetMaxSpeed(void) { return 250.0f; }

    float m_flStartThrow;
    float m_flReleaseThrow;
};

LINK_ENTITY_TO_CLASS(weapon_flashbang, CFlashbang);

void CFlashbang::Spawn(void)
{
    pev->classname = MAKE_STRING("weapon_flashbang");
    Precache();
    m_iId = WEAPON_FLASHBANG;
    SET_MODEL(ENT(pev), "models/w_flashbang.mdl");
    pev->dmg = 4.0f;
    m_iDefaultAmmo = 1;

    FallInit();
}

void CFlashbang::Precache(void)
{
    PRECACHE_MODEL("models/w_flashbang.mdl");
    PRECACHE_MODEL("models/v_flashbang_r.mdl");
    PRECACHE_MODEL("models/v_flashbang.mdl");
    PRECACHE_MODEL("models/p_flashbang.mdl");

    PRECACHE_SOUND("weapons/flashbang-1.wav");
    PRECACHE_SOUND("weapons/flashbang-2.wav");
    PRECACHE_SOUND("weapons/pinpull.wav");
}

int CFlashbang::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "Flashbang";
    p->iMaxAmmo1 = 2;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 3;
    p->iPosition = 2;
    p->iId = m_iId = WEAPON_FLASHBANG;
    p->iWeight = 1;

    p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
    return 1;
}

BOOL CFlashbang::Deploy(void)
{
    m_flReleaseThrow = -1.0f;

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_flashbang.mdl", "models/p_flashbang.mdl", FLASHBANG_DRAW, "crowbar");
    else
        return DefaultDeploy("models/v_flashbang_r.mdl", "models/p_flashbang.mdl", FLASHBANG_DRAW, "crowbar");
}

void CFlashbang::Holster(int skiplocal)
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5f;

    if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
    {
        m_pPlayer->pev->weapons &= ~(1 << WEAPON_FLASHBANG);
        DestroyItem();
    }

    m_flReleaseThrow = 0;
    m_flStartThrow = 0;

    EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0f, ATTN_NORM);
}

void CFlashbang::PrimaryAttack(void)
{
    if (m_flStartThrow != 0)
        return;

    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        m_flStartThrow = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim(FLASHBANG_PULLPIN);

        m_flTimeWeaponIdle = gpGlobals->time + 0.5f;
    }
}

void CFlashbang::WeaponIdle(void)
{
    if (m_flReleaseThrow == 0)
        m_flReleaseThrow = gpGlobals->time;

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_flStartThrow != 0)
    {
        m_pPlayer->Radio("%!MRAD_FIREINHOLE", "Fire in the hole!\n");

        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if (angThrow.x < 0)
            angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
        else
            angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

        float flVel = (90.0f - angThrow.x) * 6.0f;
        if (flVel > 750.0f)
            flVel = 750.0f;

        UTIL_MakeVectors(angThrow);

        Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16.0f;

        if (m_pPlayer->m_bLeftHanded == 1)
            vecSrc = vecSrc - gpGlobals->v_right * 12.0f;
        else
            vecSrc = vecSrc + gpGlobals->v_right * 12.0f;

        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        float flTimeDelta = m_flStartThrow - gpGlobals->time + 3.0;

        CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, 1.5f);

        SendWeaponAnim(FLASHBANG_THROW);
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        m_flStartThrow = 0;
        m_flNextPrimaryAttack = gpGlobals->time + 0.5f;
        m_flTimeWeaponIdle = gpGlobals->time + 0.75f;

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

        if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 0.5f;
        }
    }

    else if (m_flReleaseThrow > 0)
    {
        m_flStartThrow = 0;

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            SendWeaponAnim(FLASHBANG_DRAW);
            m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
            m_flReleaseThrow = -1.0f;
        }
        else
        {
            RetireWeapon();
        }
    }

    else
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            if (RANDOM_FLOAT(0, 1) > 0.75f)
                m_flTimeWeaponIdle = gpGlobals->time + 2.5f;
            else
                m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

            SendWeaponAnim(FLASHBANG_IDLE);
        }
    }
}

