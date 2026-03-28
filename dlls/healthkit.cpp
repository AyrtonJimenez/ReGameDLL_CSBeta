#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;

class CHealthKit : public CItem
{
    void Spawn(void);
    void Precache(void);
    BOOL MyTouch(CBasePlayer* pPlayer);
};

LINK_ENTITY_TO_CLASS(item_healthkit, CHealthKit);

void CHealthKit::Spawn(void)
{
    Precache();
    SET_MODEL(ENT(pev), "models/w_medkit.mdl");

    CItem::Spawn();
}

void CHealthKit::Precache(void)
{
    PRECACHE_MODEL("models/w_medkit.mdl");
    PRECACHE_SOUND("items/smallmedkit1.wav");
}

BOOL CHealthKit::MyTouch(CBasePlayer* pPlayer)
{
    if (pPlayer->TakeHealth(gSkillData.healthkitCapacity, DMG_GENERIC))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->edict());
        WRITE_STRING(STRING(pev->classname));
        MESSAGE_END();

        EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM);

        if (g_pGameRules->ItemShouldRespawn(this))
        {
            Respawn();
        }
        else
        {
            UTIL_Remove(this);
        }

        return TRUE;
    }

    return FALSE;
}

class CWallHealth : public CBaseToggle
{
public:
    void Spawn();
    void Precache(void);
    void EXPORT Off(void);
    void EXPORT Recharge(void);
    void KeyValue(KeyValueData* pkvd);
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
    virtual int		Save(CSave& save);
    virtual int		Restore(CRestore& restore);

    static	TYPEDESCRIPTION m_SaveData[];

    float m_flNextCharge;
    int		m_iReactivate;
    int		m_iJuice;
    int		m_iOn;
    float   m_flSoundTime;
};

TYPEDESCRIPTION CWallHealth::m_SaveData[] =
{
    DEFINE_FIELD(CWallHealth, m_flNextCharge, FIELD_TIME),
    DEFINE_FIELD(CWallHealth, m_iReactivate, FIELD_INTEGER),
    DEFINE_FIELD(CWallHealth, m_iJuice, FIELD_INTEGER),
    DEFINE_FIELD(CWallHealth, m_iOn, FIELD_INTEGER),
    DEFINE_FIELD(CWallHealth, m_flSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CWallHealth, CBaseEntity);

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);

void CWallHealth::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "style") ||
        FStrEq(pkvd->szKeyName, "height") ||
        FStrEq(pkvd->szKeyName, "value1") ||
        FStrEq(pkvd->szKeyName, "value2") ||
        FStrEq(pkvd->szKeyName, "value3"))
    {
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "dmdelay"))
    {
        m_iReactivate = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseToggle::KeyValue(pkvd);
    }
}

void CWallHealth::Spawn()
{
    Precache();

    pev->solid = SOLID_BSP;
    pev->movetype = MOVETYPE_PUSH;

    UTIL_SetOrigin(pev, pev->origin);
    UTIL_SetSize(pev, pev->mins, pev->maxs);
    SET_MODEL(ENT(pev), STRING(pev->model));
    m_iJuice = gSkillData.healthchargerCapacity;
    pev->frame = 0;

}

void CWallHealth::Precache()
{
    PRECACHE_SOUND("items/medshot4.wav");
    PRECACHE_SOUND("items/medshotno1.wav");
    PRECACHE_SOUND("items/medcharge4.wav");
}

void CWallHealth::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (!pActivator)
        return;

    if (!pActivator->IsPlayer())
        return;

    if (m_iJuice <= 0)
    {
        pev->frame = 1;
        Off();
    }

    if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1 << WEAPON_SUIT))))
    {
        if (m_flSoundTime <= gpGlobals->time)
        {
            m_flSoundTime = gpGlobals->time + 0.62;
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshotno1.wav", 1.0, ATTN_NORM);
        }
        return;
    }

    pev->nextthink = pev->ltime + 0.25;
    SetThink(&CWallHealth::Off);

    if (m_flNextCharge >= gpGlobals->time)
        return;

    if (!m_iOn)
    {
        m_iOn++;
        EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
        m_flSoundTime = 0.56 + gpGlobals->time;
    }
    if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
    {
        m_iOn++;
        EMIT_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM);
    }

    if (pActivator->TakeHealth(1, DMG_GENERIC))
    {
        m_iJuice--;
    }

    m_flNextCharge = gpGlobals->time + 0.1;
}

void CWallHealth::Recharge(void)
{
    EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
    m_iJuice = gSkillData.healthchargerCapacity;
    pev->frame = 0;
    SetThink(&CBaseEntity::SUB_DoNothing);
}

void CWallHealth::Off(void)
{
    if (m_iOn > 1)
        STOP_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav");

    m_iOn = 0;

    if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime()) > 0))
    {
        pev->nextthink = pev->ltime + m_iReactivate;
        SetThink(&CWallHealth::Recharge);
    }
    else
    {
        SetThink(&CBaseEntity::SUB_DoNothing);
    }
}

