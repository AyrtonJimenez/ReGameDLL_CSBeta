#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

extern int gmsgBarTime;

enum c4_e
{
    C4_IDLE1 = 0,
    C4_DRAW,
    C4_DROP,
    C4_ARM,
};

class CC4 : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    int iItemSlot(void) { return 5; }

    BOOL Deploy(void);
    void Holster(void);
    void PrimaryAttack(void);
    void WeaponIdle(void);
    float GetMaxSpeed(void) { return 250.0f; }

    bool m_bStartedArming;
    bool m_bBombPlacedAnimation;
    float m_fArmedTime;
};

LINK_ENTITY_TO_CLASS(weapon_c4, CC4);

void CC4::Spawn(void)
{
    SET_MODEL(ENT(pev), "models/w_backpack.mdl");

    pev->frame = 0;
    pev->body = 3;
    pev->sequence = 0;
    pev->framerate = 0;

    m_iId = WEAPON_C4;
    m_iDefaultAmmo = 1;

    FallInit();
    SetThink(&CBasePlayerItem::FallThink);
    pev->nextthink = gpGlobals->time + 0.1f;
}

void CC4::Precache(void)
{
    PRECACHE_MODEL("models/v_c4.mdl");
    PRECACHE_MODEL("models/p_c4.mdl");
    PRECACHE_MODEL("models/w_backpack.mdl");
    PRECACHE_SOUND("weapons/c4_click.wav");
}

int CC4::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "C4";
    p->iMaxAmmo1 = 1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 4;
    p->iPosition = 3;
    p->iId = m_iId = WEAPON_C4;
    p->iWeight = 3;
    p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
    return 1;
}

BOOL CC4::Deploy(void)
{
    pev->body = 0;
    m_bStartedArming = false;
    m_fArmedTime = 0;

    return DefaultDeploy("models/v_c4.mdl", "models/p_c4.mdl", C4_DRAW, "trip");
}

void CC4::Holster(void)
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5f;
    m_bStartedArming = false;

    if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
    {
        m_pPlayer->pev->weapons &= ~(1 << WEAPON_C4);
        DestroyItem();
    }

    SendWeaponAnim(C4_DRAW);
    EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0f, ATTN_NORM);
}

void CC4::PrimaryAttack(void)
{
    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        return;

    if (!m_bStartedArming)
    {
        if (!(m_pPlayer->m_iSignals & 2))
        {
            ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "C4 must be activated at a Bomb Target.");
            m_flNextPrimaryAttack = gpGlobals->time + 1.0f;
            return;
        }

        m_bStartedArming = true;
        m_bBombPlacedAnimation = false;
        m_fArmedTime = gpGlobals->time + 3.0f;

        SendWeaponAnim(C4_ARM);

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(m_pPlayer->pev));
        WRITE_SHORT(3);
        MESSAGE_END();

        m_flNextPrimaryAttack = gpGlobals->time + 0.3f;
        m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
        return;
    }

    if (gpGlobals->time >= m_fArmedTime)
    {
        m_bStartedArming = false;
        m_fArmedTime = 0.0f;

        if (m_pPlayer->m_iSignals & 2)
        {
            Broadcast("BOMBPL");

            m_pPlayer->m_bHasC4 = false;

            Vector vecAiming;
            GET_AIM_VECTOR(ENT(m_pPlayer->pev), 0, vecAiming);

            float flLength = vecAiming.Length();
            if (flLength > 0)
                vecAiming = vecAiming * (1.0f / flLength);
            else
                vecAiming = Vector(0, 0, 1);

            Vector vecPlant = vecAiming * 12.0f;
            vecPlant.z = -10.0f;

            Vector vecDst = vecPlant + m_pPlayer->pev->origin;

            TraceResult tr;
            UTIL_TraceLine(m_pPlayer->pev->origin, vecDst, ignore_monsters, ENT(pev), &tr);

            if (tr.flFraction < 1.0f)
            {
                Vector vecOrigin = tr.vecEndPos + tr.vecPlaneNormal * 5.0f;
                CGrenade::ShootSatchelCharge(m_pPlayer->pev, vecOrigin, Vector(0, 0, 0));
            }
            else
            {
                CGrenade::ShootSatchelCharge(m_pPlayer->pev, vecDst, Vector(0, 0, 0));
            }

            UTIL_ClientPrintAll(HUD_PRINTCENTER, "The bomb has been planted!", NULL, NULL, NULL, NULL);
            EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/c4_plant.wav", 1.0f, ATTN_NORM);

            m_pPlayer->pev->body = 0;
            m_pPlayer->SetBombIcon(0);

            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
            if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
            {
                RetireWeapon();
                return;
            }
        }
        else
        {
            ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "C4 must be activated at a Bomb Target.");
            m_flNextPrimaryAttack = gpGlobals->time + 1.0f;
            return;
        }
    }
    else
    {
        if (gpGlobals->time >= m_fArmedTime - 0.75f && !m_bBombPlacedAnimation)
        {
            m_bBombPlacedAnimation = true;
            SendWeaponAnim(C4_DROP);
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        }

        if (!(m_pPlayer->m_iSignals & 2))
        {
            ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER,
                "Arming Sequence Canceled.\nC4 can only be placed at a Bomb Target.");
            m_bStartedArming = false;
            m_flNextPrimaryAttack = gpGlobals->time + 1.5f;

            MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(m_pPlayer->pev));
            WRITE_SHORT(0);
            MESSAGE_END();

            if (m_bBombPlacedAnimation)
                SendWeaponAnim(C4_DRAW);
            else
                SendWeaponAnim(C4_IDLE1);
            return;
        }
    }

    m_flNextPrimaryAttack = gpGlobals->time + 0.3f;
    m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
}

void CC4::WeaponIdle(void)
{
    if (m_bStartedArming)
    {
        m_bStartedArming = false;
        m_flNextPrimaryAttack = gpGlobals->time + 1.0f;

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(m_pPlayer->pev));
        WRITE_SHORT(0);
        MESSAGE_END();

        if (m_bBombPlacedAnimation)
            SendWeaponAnim(C4_DRAW);
        else
            SendWeaponAnim(C4_IDLE1);
    }

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
    {
        SendWeaponAnim(C4_DRAW);
        SendWeaponAnim(C4_IDLE1);
    }
    else
    {
        RetireWeapon();
    }
}

