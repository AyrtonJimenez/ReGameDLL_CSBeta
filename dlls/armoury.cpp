#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

#define ARMOURY_MP5NAVY		0
#define ARMOURY_TMP			1
#define ARMOURY_P90			2
#define ARMOURY_MAC10		3
#define ARMOURY_AK47		4
#define ARMOURY_SG552		5
#define ARMOURY_M4A1		6
#define ARMOURY_AUG			7
#define ARMOURY_SCOUT		8
#define ARMOURY_G3SG1		9
#define ARMOURY_AWP			10
#define ARMOURY_M3			11
#define ARMOURY_XM1014		12
#define ARMOURY_M249		13
#define ARMOURY_FLASHBANG	14
#define ARMOURY_HEGRENADE	15
#define ARMOURY_KEVLAR		16
#define ARMOURY_ASSAULT		17
#define ARMOURY_SMOKEGRENADE 18

class CArmoury : public CBaseEntity
{
public:
    void Spawn(void);
    void Precache(void);
    void Restart(void);
    void KeyValue(KeyValueData* pkvd);
    void EXPORT ArmouryTouch(CBaseEntity* pOther);

    int m_iItem;
    int m_iCount;
    int m_iInitialCount;
    BOOL m_bAlreadyCounted;
};

LINK_ENTITY_TO_CLASS(armoury_entity, CArmoury);

void CArmoury::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "item"))
    {
        m_iItem = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "count"))
    {
        m_iCount = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseEntity::KeyValue(pkvd);
    }
}

void CArmoury::Spawn(void)
{
    Precache();

    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

    UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
    UTIL_SetOrigin(pev, pev->origin);

    SetTouch(&CArmoury::ArmouryTouch);

    switch (m_iItem)
    {
    case ARMOURY_MP5NAVY:	SET_MODEL(ENT(pev), "models/w_mp5.mdl"); break;
    case ARMOURY_TMP:		SET_MODEL(ENT(pev), "models/w_tmp.mdl"); break;
    case ARMOURY_P90:		SET_MODEL(ENT(pev), "models/w_p90.mdl"); break;
    case ARMOURY_MAC10:		SET_MODEL(ENT(pev), "models/w_mac10.mdl"); break;
    case ARMOURY_AK47:		SET_MODEL(ENT(pev), "models/w_ak47.mdl"); break;
    case ARMOURY_SG552:		SET_MODEL(ENT(pev), "models/w_sg552.mdl"); break;
    case ARMOURY_M4A1:		SET_MODEL(ENT(pev), "models/w_m4a1.mdl"); break;
    case ARMOURY_AUG:		SET_MODEL(ENT(pev), "models/w_aug.mdl"); break;
    case ARMOURY_SCOUT:		SET_MODEL(ENT(pev), "models/w_scout.mdl"); break;
    case ARMOURY_G3SG1:		SET_MODEL(ENT(pev), "models/w_g3sg1.mdl"); break;
    case ARMOURY_AWP:		SET_MODEL(ENT(pev), "models/w_awp.mdl"); break;
    case ARMOURY_M3:		SET_MODEL(ENT(pev), "models/w_m3.mdl"); break;
    case ARMOURY_XM1014:	SET_MODEL(ENT(pev), "models/w_xm1014.mdl"); break;
    case ARMOURY_M249:		SET_MODEL(ENT(pev), "models/w_m249.mdl"); break;
    case ARMOURY_FLASHBANG:	SET_MODEL(ENT(pev), "models/w_flashbang.mdl"); break;
    case ARMOURY_HEGRENADE:	SET_MODEL(ENT(pev), "models/w_hegrenade.mdl"); break;
    case ARMOURY_ASSAULT:	SET_MODEL(ENT(pev), "models/w_assault.mdl"); break;
    case ARMOURY_KEVLAR:
    default:				SET_MODEL(ENT(pev), "models/w_kevlar.mdl"); break;
    }

    if (m_iCount <= 0)
        m_iCount = 1;

    m_iInitialCount = m_iCount;
    m_bAlreadyCounted = FALSE;
}

void CArmoury::Precache(void)
{
    char* pszModel;

    switch (m_iItem)
    {
    case ARMOURY_MP5NAVY:	pszModel = "models/w_mp5.mdl"; break;
    case ARMOURY_TMP:		pszModel = "models/w_tmp.mdl"; break;
    case ARMOURY_P90:		pszModel = "models/w_p90.mdl"; break;
    case ARMOURY_MAC10:		pszModel = "models/w_mac10.mdl"; break;
    case ARMOURY_AK47:		pszModel = "models/w_ak47.mdl"; break;
    case ARMOURY_SG552:		pszModel = "models/w_sg552.mdl"; break;
    case ARMOURY_M4A1:		pszModel = "models/w_m4a1.mdl"; break;
    case ARMOURY_AUG:		pszModel = "models/w_aug.mdl"; break;
    case ARMOURY_SCOUT:		pszModel = "models/w_scout.mdl"; break;
    case ARMOURY_G3SG1:		pszModel = "models/w_g3sg1.mdl"; break;
    case ARMOURY_AWP:		pszModel = "models/w_awp.mdl"; break;
    case ARMOURY_M3:		pszModel = "models/w_m3.mdl"; break;
    case ARMOURY_XM1014:	pszModel = "models/w_xm1014.mdl"; break;
    case ARMOURY_M249:		pszModel = "models/w_m249.mdl"; break;
    case ARMOURY_FLASHBANG:	pszModel = "models/w_flashbang.mdl"; break;
    case ARMOURY_HEGRENADE:	pszModel = "models/w_hegrenade.mdl"; break;
    case ARMOURY_ASSAULT:	pszModel = "models/w_assault.mdl"; break;
    default:				pszModel = "models/w_kevlar.mdl"; break;
    }

    PRECACHE_MODEL(pszModel);
}

void CArmoury::Restart(void)
{
    CHalfLifeMultiplay* pGameRules = (CHalfLifeMultiplay*)g_pGameRules;

    if (m_iItem == ARMOURY_FLASHBANG || m_iItem == ARMOURY_HEGRENADE)
    {
        if (!m_bAlreadyCounted)
        {
            m_bAlreadyCounted = TRUE;
            pGameRules->m_iTotalGrenadeCount += m_iInitialCount;
            m_iCount = m_iInitialCount;
        }
        else
        {

            m_iCount = (int)(1.75 * pGameRules->m_iNumTerrorist * (m_iInitialCount / pGameRules->m_iTotalGrenadeCount));
            if (m_iCount <= 0)
                m_iCount = 1;
        }
    }

    else if (m_iItem == ARMOURY_KEVLAR || m_iItem == ARMOURY_ASSAULT)
    {
        if (!m_bAlreadyCounted)
        {
            m_bAlreadyCounted = TRUE;
            pGameRules->m_iTotalArmourCount += m_iInitialCount;
            m_iCount = m_iInitialCount;
        }
        else
        {
            m_iCount = (int)(0.7 * pGameRules->m_iNumTerrorist * (m_iInitialCount / pGameRules->m_iTotalArmourCount));
            if (m_iCount <= 0)
                m_iCount = 1;
        }
    }

    else
    {
        if (!m_bAlreadyCounted)
        {
            m_bAlreadyCounted = TRUE;
            pGameRules->m_iTotalGunCount += m_iInitialCount;
            m_iCount = m_iInitialCount;
        }
        else
        {
            m_iCount = (int)(0.7 * pGameRules->m_iNumTerrorist * (m_iInitialCount / pGameRules->m_iTotalGunCount));
            if (m_iCount <= 0)
                m_iCount = 1;
        }
    }

    pev->effects &= ~EF_NODRAW;
}

void CArmoury::ArmouryTouch(CBaseEntity* pOther)
{
    if (!pOther->IsPlayer())
        return;

    CBasePlayer* pPlayer = (CBasePlayer*)pOther;

    if (pPlayer->m_bIsVIP)
        return;

    if (m_iCount <= 0)
        return;

    if (m_iItem <= ARMOURY_M249)
    {
        if (pPlayer->m_bHasPrimary)
            return;

        m_iCount--;

        switch (m_iItem)
        {
        case ARMOURY_MP5NAVY:
            pPlayer->GiveNamedItem("weapon_mp5navy");
            pPlayer->GiveAmmo(60, "9mm", 120);
            break;
        case ARMOURY_TMP:
            pPlayer->GiveNamedItem("weapon_tmp");
            pPlayer->GiveAmmo(60, "9mm", 120);
            break;
        case ARMOURY_P90:
            pPlayer->GiveNamedItem("weapon_p90");
            pPlayer->GiveAmmo(50, "57mm", 150);
            break;
        case ARMOURY_MAC10:
            pPlayer->GiveNamedItem("weapon_mac10");
            pPlayer->GiveAmmo(60, "45acp", 120);
            break;
        case ARMOURY_AK47:
            pPlayer->GiveNamedItem("weapon_ak47");
            pPlayer->GiveAmmo(60, "762Nato", 120);
            break;
        case ARMOURY_SG552:
            pPlayer->GiveNamedItem("weapon_sg552");
            pPlayer->GiveAmmo(60, "556Nato", 200);
            break;
        case ARMOURY_M4A1:
            pPlayer->GiveNamedItem("weapon_m4a1");
            pPlayer->GiveAmmo(60, "556Nato", 200);
            break;
        case ARMOURY_AUG:
            pPlayer->GiveNamedItem("weapon_aug");
            pPlayer->GiveAmmo(60, "556Nato", 200);
            break;
        case ARMOURY_SCOUT:
            pPlayer->GiveNamedItem("weapon_scout");
            pPlayer->GiveAmmo(30, "762Nato", 120);
            break;
        case ARMOURY_G3SG1:
            pPlayer->GiveNamedItem("weapon_g3sg1");
            pPlayer->GiveAmmo(30, "762Nato", 120);
            break;
        case ARMOURY_AWP:
            pPlayer->GiveNamedItem("weapon_awp");
            pPlayer->GiveAmmo(20, "338Magnum", 30);
            break;
        case ARMOURY_M3:
            pPlayer->GiveNamedItem("weapon_m3");
            pPlayer->GiveAmmo(24, "buckshot", 35);
            break;
        case ARMOURY_XM1014:
            pPlayer->GiveNamedItem("weapon_xm1014");
            pPlayer->GiveAmmo(24, "buckshot", 35);
            break;
        case ARMOURY_M249:
            pPlayer->GiveNamedItem("weapon_m249");
            pPlayer->GiveAmmo(60, "556NatoBox", 200);
            break;
        }
    }

    else
    {
        switch (m_iItem)
        {
        case ARMOURY_FLASHBANG:
        {
            int iAmmoIndex = CBasePlayer::GetAmmoIndex("Flashbang");
            if (pPlayer->AmmoInventory(iAmmoIndex) >= 2)
                return;

            pPlayer->GiveNamedItem("weapon_flashbang");
            m_iCount--;
            break;
        }
        case ARMOURY_HEGRENADE:
        {
            int iAmmoIndex = CBasePlayer::GetAmmoIndex("HEGrenade");
            if (pPlayer->AmmoInventory(iAmmoIndex) >= 1)
                return;

            pPlayer->GiveNamedItem("weapon_hegrenade");
            m_iCount--;
            break;
        }
        case ARMOURY_KEVLAR:
        {
            if (pPlayer->m_iKevlar == ARMOR_KEVLAR)
                return;

            pPlayer->GiveNamedItem("item_kevlar");
            m_iCount--;
            break;
        }
        case ARMOURY_ASSAULT:
        {
            if (pPlayer->m_iKevlar == ARMOR_VESTHELM)
                return;

            pPlayer->GiveNamedItem("item_assaultsuit");
            m_iCount--;
            break;
        }
        }
    }

    if (!m_iCount)
    {
        pev->effects |= EF_NODRAW;
    }
}

