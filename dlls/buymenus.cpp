#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "weapons.h"
#include "gamerules.h"

extern int gmsgShowMenu;
extern int gmsgMoney;
extern int gmsgBlinkAcct;
extern int gmsgTeamInfo;
extern int gmsgStatusIcon;

extern int SelectDefaultTeam(void);

void ShowMenu(CBasePlayer* pPlayer, int bitsValidSlots, int nDisplayTime, int fNeedMore, char* pszText)
{
    MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, ENT(pPlayer->pev));
    WRITE_SHORT(bitsValidSlots);
    WRITE_CHAR(nDisplayTime);
    WRITE_BYTE(fNeedMore);
    WRITE_STRING(pszText);
    MESSAGE_END();
}

BOOL HandleMenu_ChooseTeam(CBasePlayer* pPlayer, int slot)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    if (pPlayer->m_bIsVIP)
    {
        if (!pPlayer->pev->deadflag)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER,
                "You are the VIP! You can't change roles\n while you're alive");
            pPlayer->m_iTeam = TEAM_CT;
            CLIENT_COMMAND(ENT(pPlayer->pev), "slot10\n");
            return FALSE;
        }

        if (mp->IsVIPQueueEmpty())
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER,
                "You are the VIP!\n You can't change roles now\n because no one else wants to be\n the VIP");
            pPlayer->m_iTeam = TEAM_CT;
            CLIENT_COMMAND(ENT(pPlayer->pev), "slot10\n");
            return FALSE;
        }
    }

    int team;

    switch (slot)
    {
    case 1:
        team = TEAM_TERRORIST;
        break;

    case 2:
        team = TEAM_CT;
        break;

    case 3:

        pPlayer->m_iTeam = TEAM_CT;
        mp->AddToVIPQueue(pPlayer);
        CLIENT_COMMAND(ENT(pPlayer->pev), "slot10\n");
        return FALSE;

    case 5:
        team = SelectDefaultTeam();
        if (!team)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "All teams are full");
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    if (mp->TeamFull(team))
    {
        if (team == TEAM_TERRORIST)
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "The Terrorist team is full");
        else
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "The Counter-Terrorist team is full");
        return FALSE;
    }

    if (mp->TeamStacked(team, pPlayer->m_iTeam))
    {
        if (team == TEAM_TERRORIST)
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Too many Terrorist!");
        else
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Too many Counter-Terrorist!");
        return FALSE;
    }

    if (pPlayer->IsCommander())
        mp->PickNewCommander(pPlayer->m_iTeam);

    pPlayer->m_iTeam = team;

    if (team == TEAM_TERRORIST)
        ShowMenu(pPlayer, 23, -1, 0, "#Terrorist_Select");
    else if (team == TEAM_CT)
        ShowMenu(pPlayer, 31, -1, 0, "#CT_Select");

    pPlayer->m_iMenu = Menu_ChooseAppearance;

    if (pPlayer->pev->deadflag)
        pPlayer->m_bTeamChanged = true;
    else
        ClientKill(ENT(pPlayer->pev));

    int iTeam = pPlayer->m_iTeam;

    MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo, NULL);
    WRITE_BYTE(ENTINDEX(ENT(pPlayer->pev)));
    WRITE_BYTE(iTeam);
    MESSAGE_END();

    if (iTeam)
        pPlayer->SetScoreboardAttributes(NULL);

    const char* pszPlayerName = "unconnected";
    if (pPlayer->pev->netname && STRING(pPlayer->pev->netname)[0])
        pszPlayerName = STRING(pPlayer->pev->netname);

    char* pszJoinMsg;
    if (team == TEAM_TERRORIST)
        pszJoinMsg = UTIL_VarArgs("%s is joining the Terrorist force\n", pszPlayerName);
    else
        pszJoinMsg = UTIL_VarArgs("%s is joining the Counter-Terrorist force\n", pszPlayerName);

    UTIL_ClientPrintAll(HUD_PRINTNOTIFY, pszJoinMsg);

    return TRUE;
}

void HandleMenu_ChooseAppearance(CBasePlayer* pPlayer, int slot)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    int iModelName;
    const char* pszModelCmd;
    const char* pszModelName;

    if (pPlayer->m_iTeam == TEAM_TERRORIST)
    {
        if (slot > 3)
            slot = RANDOM_LONG(1, 3);

        switch (slot)
        {
        case 1:
            iModelName = 2;
            pszModelCmd = "model terror\n";
            pszModelName = "terror";
            break;
        case 2:
            iModelName = 3;
            pszModelCmd = "model arab\n";
            pszModelName = "arab";
            break;
        case 3:
            iModelName = 4;
            pszModelCmd = "model arctic\n";
            pszModelName = "arctic";
            break;
        }
    }

    else if (pPlayer->m_iTeam == TEAM_CT)
    {
        if (slot > 4)
            slot = RANDOM_LONG(1, 4);

        switch (slot)
        {
        case 1:
            iModelName = 1;
            pszModelCmd = "model urban\n";
            pszModelName = "urban";
            break;
        case 2:
            iModelName = 5;
            pszModelCmd = "model gsg9\n";
            pszModelName = "gsg9";
            break;
        case 3:
            iModelName = 7;
            pszModelCmd = "model sas\n";
            pszModelName = "sas";
            break;
        case 4:
            iModelName = 6;
            pszModelCmd = "model gign\n";
            pszModelName = "gign";
            break;
        }
    }

    pPlayer->m_iMenu = Menu_OFF;

    if (pPlayer->m_iJoiningState == JOINED)
    {
        mp->CheckWinConditions();
    }
    else if (pPlayer->m_iJoiningState == PICKINGTEAM)
    {
        pPlayer->m_iJoiningState = GETINTOGAME;
    }

    pPlayer->pev->body = 0;
    pPlayer->m_iModelName = iModelName;

    CLIENT_COMMAND(ENT(pPlayer->pev), (char*)pszModelCmd);

    SET_CLIENT_KEY_VALUE(
        ENTINDEX(ENT(pPlayer->pev)),
        g_engfuncs.pfnGetInfoKeyBuffer(ENT(pPlayer->pev)),
        "model",
        (char*)pszModelName
    );

    if (!mp->m_bMapHasVIPSafetyZone)
    {
        if (UTIL_FindEntityByClassname(NULL, "func_vip_safetyzone"))
            mp->m_bMapHasVIPSafetyZone = 1;
        else
            mp->m_bMapHasVIPSafetyZone = 2;
    }

    if (mp->m_bMapHasVIPSafetyZone == 1 && !mp->m_pVIP && pPlayer->m_iTeam == TEAM_CT)
        pPlayer->MakeVIP();
}

BOOL BuyGunAmmo(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon, BOOL bBlinkMoney)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return FALSE;

    int iAmmoIndex = pWeapon->PrimaryAmmoIndex();
    if (iAmmoIndex == -1)
        return FALSE;

    int iWeaponId = pWeapon->m_iId;

    ItemInfo* pInfo = &CBasePlayerItem::ItemInfoArray[iWeaponId];
    if (pPlayer->m_rgAmmo[iAmmoIndex] >= pInfo->iMaxAmmo1)
        return FALSE;

    int iCost;
    const char* pszAmmoEntity;

    switch (iWeaponId)
    {
    case WEAPON_P228:
        iCost = 50;  pszAmmoEntity = "ammo_357sig"; break;
    case WEAPON_SCOUT:
    case WEAPON_G3SG1:
    case WEAPON_AK47:
        iCost = 80;  pszAmmoEntity = "ammo_762nato"; break;
    case WEAPON_XM1014:
    case WEAPON_M3:
        iCost = 65;  pszAmmoEntity = "ammo_buckshot"; break;
    case WEAPON_MAC10:
    case WEAPON_USP:
        iCost = 25;  pszAmmoEntity = "ammo_45acp"; break;
    case WEAPON_AUG:
    case WEAPON_M4A1:
    case WEAPON_SG552:
        iCost = 60;  pszAmmoEntity = "ammo_556nato"; break;
    case WEAPON_GLOCK18:
    case WEAPON_MP5N:
    case WEAPON_TMP:
        iCost = 20;  pszAmmoEntity = "ammo_9mm"; break;
    case WEAPON_AWP:
        iCost = 125; pszAmmoEntity = "ammo_338magnum"; break;
    case WEAPON_M249:
        iCost = 60;  pszAmmoEntity = "ammo_556natobox"; break;
    case WEAPON_DEAGLE:
        iCost = 40;  pszAmmoEntity = "ammo_50ae"; break;
    case WEAPON_P90:
        iCost = 50;  pszAmmoEntity = "ammo_57mm"; break;
    default:
        ALERT(at_console, "Tried to buy ammo for an unrecognized gun\n");
        return FALSE;
    }

    if (pPlayer->m_iAccount < iCost)
    {
        int iMaxAmmo = pInfo->iMaxAmmo1;
        if (pPlayer->m_rgAmmo[iAmmoIndex] > iMaxAmmo)
            pPlayer->m_rgAmmo[iAmmoIndex] = iMaxAmmo;

        if (bBlinkMoney)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
            MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
            WRITE_BYTE(2);
            MESSAGE_END();
        }
        return FALSE;
    }

    pPlayer->GiveNamedItem(pszAmmoEntity);
    pPlayer->AddAccount(-iCost, TRUE);
    return TRUE;
}

int BuyAmmo(CBasePlayer* pPlayer, int nSlot, BOOL bBlinkMoney)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return 0;

    if (!(pPlayer->m_iSignals & 1))
        return 0;

    if (nSlot < 1 || nSlot > 2)
        return 0;

    CBasePlayerItem* pItem = pPlayer->m_rgpPlayerItems[nSlot];
    if (!pItem)
        return 0;

    do
    {
        if (!BuyGunAmmo(pPlayer, pItem, bBlinkMoney))
            return 0;
        pItem = pItem->m_pNext;
    } while (pItem);

    return 1;
}

static BOOL BuyWeapon(CBasePlayer* pPlayer, int iCost, const char* pszWeapon)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return FALSE;

    if (pPlayer->m_iAccount < iCost)
    {
        ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
        MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
        WRITE_BYTE(2);
        MESSAGE_END();
        return FALSE;
    }

    pPlayer->AddAccount(-iCost, TRUE);
    pPlayer->GiveNamedItem(pszWeapon);
    return TRUE;
}

BOOL CanBuyThis(CBasePlayer* pPlayer, int iWeaponID)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    if (mp->m_bMapHasVIPSafetyZone == 1)
    {
        if (pPlayer->m_iTeam == TEAM_CT)
        {
            switch (iWeaponID)
            {
            case WEAPON_XM1014:
            case WEAPON_AUG:
            case WEAPON_MP5N:
            case WEAPON_M249:
            case WEAPON_M3:
            case WEAPON_M4A1:
            case WEAPON_G3SG1:
            case WEAPON_P90:
                return TRUE;
            default:
                ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You cannot buy this!!\n");
                return FALSE;
            }
        }

        if (pPlayer->m_iTeam)
        {
            switch (iWeaponID)
            {
            case WEAPON_SCOUT:
            case WEAPON_MAC10:
            case WEAPON_AWP:
            case WEAPON_M4A1:
            case WEAPON_G3SG1:
            case WEAPON_AK47:
                return TRUE;
            default:
                ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You cannot buy this!!\n");
                return FALSE;
            }
        }

        return TRUE;
    }

    if (pPlayer->m_iTeam == TEAM_CT)
    {
        switch (iWeaponID)
        {
        case WEAPON_SCOUT:
        case WEAPON_XM1014:
        case WEAPON_AUG:
        case WEAPON_AWP:
        case WEAPON_MP5N:
        case WEAPON_M249:
        case WEAPON_M3:
        case WEAPON_M4A1:
        case WEAPON_TMP:
        case WEAPON_G3SG1:
        case WEAPON_P90:
            return TRUE;
        default:
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You cannot buy this!!\n");
            return FALSE;
        }
    }

    if (!pPlayer->m_iTeam)
        return TRUE;

    switch (iWeaponID)
    {
    case WEAPON_SCOUT:
    case WEAPON_XM1014:
    case WEAPON_MAC10:
    case WEAPON_AWP:
    case WEAPON_MP5N:
    case WEAPON_M249:
    case WEAPON_M3:
    case WEAPON_G3SG1:
    case WEAPON_SG552:
    case WEAPON_AK47:
    case WEAPON_P90:
        return TRUE;
    default:
        ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You cannot buy this!!\n");
        return FALSE;
    }
}

void BuyPistol(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    switch (slot)
    {
    case 1:
        if (pPlayer->m_iAccount <= 499)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[2])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[2]->pev->classname));
        pPlayer->GiveNamedItem("weapon_usp");
        pPlayer->AddAccount(-500, true);
        break;

    case 2:
        if (pPlayer->m_iAccount <= 399)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[2])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[2]->pev->classname));
        pPlayer->GiveNamedItem("weapon_glock18");
        pPlayer->AddAccount(-400, true);
        break;

    case 3:
        if (pPlayer->m_iAccount <= 649)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[2])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[2]->pev->classname));
        pPlayer->GiveNamedItem("weapon_deagle");
        pPlayer->AddAccount(-650, true);
        break;

    case 4:
        if (pPlayer->m_iAccount <= 599)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[2])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[2]->pev->classname));
        pPlayer->GiveNamedItem("weapon_p228");
        pPlayer->AddAccount(-600, true);
        break;

    default:
        return;
    }
    return;

NOT_ENOUGH_MONEY:
    ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
    MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
    WRITE_BYTE(2);
    MESSAGE_END();
}

void BuyShotgun(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    if (slot == 1)
    {
        if (!CanBuyThis(pPlayer, WEAPON_M3))
            return;

        if (pPlayer->m_iAccount <= 1699)
            goto NOT_ENOUGH_MONEY;

        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));

        pPlayer->GiveNamedItem("weapon_m3");
        pPlayer->AddAccount(-1700, TRUE);
    }
    else if (slot == 2)
    {
        if (!CanBuyThis(pPlayer, WEAPON_XM1014))
            return;

        if (pPlayer->m_iAccount <= 2999)
            goto NOT_ENOUGH_MONEY;

        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));

        pPlayer->GiveNamedItem("weapon_xm1014");
        pPlayer->AddAccount(-3000, TRUE);
    }

    return;

NOT_ENOUGH_MONEY:
    ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
    MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
    WRITE_BYTE(2);
    MESSAGE_END();
}

void BuySubMachineGun(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    switch (slot)
    {
    case 1:
        if (!CanBuyThis(pPlayer, WEAPON_MP5N))
            return;
        if (pPlayer->m_iAccount <= 1499)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_mp5navy");
        pPlayer->AddAccount(-1500, TRUE);
        break;

    case 2:
        if (!CanBuyThis(pPlayer, WEAPON_TMP))
            return;
        if (pPlayer->m_iAccount <= 1249)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_tmp");
        pPlayer->AddAccount(-1250, TRUE);
        break;

    case 3:
        if (!CanBuyThis(pPlayer, WEAPON_P90))
            return;
        if (pPlayer->m_iAccount <= 2749)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_p90");
        pPlayer->AddAccount(-2750, TRUE);
        break;

    case 4:
        if (!CanBuyThis(pPlayer, WEAPON_MAC10))
            return;
        if (pPlayer->m_iAccount <= 1399)
        {
        NOT_ENOUGH_MONEY:
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
            MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
            WRITE_BYTE(2);
            MESSAGE_END();
            return;
        }
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_mac10");
        pPlayer->AddAccount(-1400, TRUE);
        break;

    default:
        return;
    }
}

void BuyRifle(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    switch (slot)
    {
    case 1:
        if (!CanBuyThis(pPlayer, WEAPON_AK47))
            break;
        if (pPlayer->m_iTeam == TEAM_CT)
            goto WEAPON_NOT_AVAILABLE;
        if (pPlayer->m_iAccount <= 2499)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_ak47");
        pPlayer->AddAccount(-2500, TRUE);
        break;

    case 2:
        if (!CanBuyThis(pPlayer, WEAPON_SG552))
            break;
        if (pPlayer->m_iAccount <= 3499)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_sg552");
        pPlayer->AddAccount(-3500, TRUE);
        break;

    case 3:
        if (!CanBuyThis(pPlayer, WEAPON_M4A1))
            break;
        if (pPlayer->m_iTeam == TEAM_TERRORIST)
            goto WEAPON_NOT_AVAILABLE;
        if (pPlayer->m_iAccount <= 3099)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_m4a1");
        pPlayer->AddAccount(-3100, TRUE);
        break;

    case 4:
        if (!CanBuyThis(pPlayer, WEAPON_AUG))
            break;
        if (pPlayer->m_iTeam == TEAM_TERRORIST)
        {
        WEAPON_NOT_AVAILABLE:
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Weapon not available");
        }
        else
        {
            if (pPlayer->m_iAccount <= 3499)
                goto NOT_ENOUGH_MONEY;
            while (pPlayer->m_rgpPlayerItems[1])
                pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
            pPlayer->GiveNamedItem("weapon_aug");
            pPlayer->AddAccount(-3500, TRUE);
        }
        break;

    case 5:
        if (!CanBuyThis(pPlayer, WEAPON_SCOUT))
            break;
        if (pPlayer->m_iAccount <= 2749)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_scout");
        pPlayer->AddAccount(-2750, TRUE);
        break;

    case 6:
        if (!CanBuyThis(pPlayer, WEAPON_AWP))
            break;
        if (pPlayer->m_iAccount <= 4749)
            goto NOT_ENOUGH_MONEY;
        while (pPlayer->m_rgpPlayerItems[1])
            pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
        pPlayer->GiveNamedItem("weapon_awp");
        pPlayer->AddAccount(-4750, TRUE);
        break;

    case 7:
        if (!CanBuyThis(pPlayer, WEAPON_G3SG1))
            break;
        if (pPlayer->m_iAccount <= 4999)
        {
        NOT_ENOUGH_MONEY:
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
            MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
            WRITE_BYTE(2);
            MESSAGE_END();
        }
        else
        {
            while (pPlayer->m_rgpPlayerItems[1])
                pPlayer->DropPlayerItem((char*)STRING(pPlayer->m_rgpPlayerItems[1]->pev->classname));
            pPlayer->GiveNamedItem("weapon_g3sg1");
            pPlayer->AddAccount(-5000, TRUE);
        }
        break;

    default:
        return;
    }
}

void BuyMachineGun(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    if (slot != 1)
        return;

    if (!CanBuyThis(pPlayer, WEAPON_M249))
        return;

    if (pPlayer->m_iAccount < 5750)
    {
        ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
        MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
        WRITE_BYTE(2);
        MESSAGE_END();
    }
    else
    {

        CBasePlayerItem* pItem;
        while ((pItem = pPlayer->m_rgpPlayerItems[1]) != NULL)
        {
            pPlayer->DropPlayerItem((char*)STRING(pItem->pev->classname));
        }

        pPlayer->GiveNamedItem("weapon_m249");
        pPlayer->AddAccount(-5750, TRUE);
    }
}

void BuyItem(CBasePlayer* pPlayer, int slot)
{
    if (!pPlayer->CanPlayerBuy(TRUE))
        return;

    if (!(pPlayer->m_iSignals & SIGNAL_BUY))
        return;

    switch (slot)
    {
    case 1:
        if (pPlayer->pev->armorvalue >= 100)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You already have armor");
        }
        else if (pPlayer->m_iAccount <= 649)
        {
            goto NOT_ENOUGH_MONEY;
        }
        else
        {
            EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/kevlar.wav", 1, ATTN_NORM);
            pPlayer->GiveNamedItem("item_kevlar");
            pPlayer->AddAccount(-650, TRUE);
        }
        break;

    case 2:
        if (pPlayer->pev->armorvalue >= 100 && pPlayer->HasAssaultSuit())
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You already have armor");
        }
        else if (pPlayer->m_iAccount <= 999)
        {
            goto NOT_ENOUGH_MONEY;
        }
        else
        {
            EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/kevlar.wav", 1, ATTN_NORM);
            pPlayer->GiveNamedItem("item_assaultsuit");
            pPlayer->AddAccount(-1000, TRUE);
        }
        break;

    case 3:
    {
        int iAmmoIndex = CBasePlayer::GetAmmoIndex("Flashbang");
        if (pPlayer->AmmoInventory(iAmmoIndex) > 1)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You can't carry any more");
        }
        else if (pPlayer->m_iAccount <= 199)
        {
            goto NOT_ENOUGH_MONEY;
        }
        else
        {
            pPlayer->GiveNamedItem("weapon_flashbang");
            pPlayer->AddAccount(-200, TRUE);
        }
        break;
    }

    case 4:
    {
        int iAmmoIndex = CBasePlayer::GetAmmoIndex("HEGrenade");
        if (pPlayer->AmmoInventory(iAmmoIndex) > 0)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You can't carry any more");
        }
        else if (pPlayer->m_iAccount <= 299)
        {
            goto NOT_ENOUGH_MONEY;
        }
        else
        {
            pPlayer->GiveNamedItem("weapon_hegrenade");
            pPlayer->AddAccount(-300, TRUE);
        }
        break;
    }

    case 5:
        if (pPlayer->m_iTeam == TEAM_CT)
        {
            CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
            if (mp->m_bMapHasBombTarget == 1)
            {
                if (pPlayer->m_bHasDefuser)
                {
                    ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You already have one");
                }
                else if (pPlayer->m_iAccount <= 199)
                {
                    goto NOT_ENOUGH_MONEY;
                }
                else
                {
                    pPlayer->m_bHasDefuser = TRUE;

                    MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pPlayer->pev));
                    WRITE_BYTE(1);
                    WRITE_STRING("defuser");
                    WRITE_BYTE(0);
                    WRITE_BYTE(160);
                    WRITE_BYTE(0);
                    MESSAGE_END();

                    pPlayer->pev->body = 1;
                    pPlayer->AddAccount(-200, TRUE);
                    EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/kevlar.wav", 1, ATTN_NORM);
                }
            }
        }
        break;

    case 6:
        if (pPlayer->m_bHasNightVision)
        {
            ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "You already have NVG");
        }
        else if (pPlayer->m_iAccount <= 1249)
        {
            goto NOT_ENOUGH_MONEY;
        }
        else
        {

            if (!(pPlayer->m_flDisplayHistory & DHF_NIGHTVISION))
            {
                pPlayer->HintMessage(
                    "Press the NIGHTVISION key to turn on/off nightvision goggles.\n"
                    " Nightvision can be adjusted by typing:\n"
                    " +nvgadjust\n"
                    "-nvgadjust\n"
                    "at the console.", FALSE, FALSE);

                pPlayer->m_flDisplayHistory |= DHF_NIGHTVISION;
            }

            EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/equip_nvg.wav", 1, ATTN_NORM);
            pPlayer->m_bHasNightVision = TRUE;
            pPlayer->AddAccount(-1250, TRUE);
        }
        break;
    }

    return;

NOT_ENOUGH_MONEY:
    ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Not enough money");
    MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, ENT(pPlayer->pev));
    WRITE_BYTE(2);
    MESSAGE_END();
}

void Radio1(CBasePlayer* pPlayer, int slot)
{
    if (gpGlobals->time > pPlayer->m_flRadioTime)
    {
        if (pPlayer->m_iRadioMessages > 0)
        {
            pPlayer->m_iRadioMessages--;
            pPlayer->m_flRadioTime = gpGlobals->time + 1.5;

            switch (slot)
            {
            case 1: pPlayer->Radio("%!MRAD_COVERME", "Cover me!\n"); break;
            case 2: pPlayer->Radio("%!MRAD_TAKEPOINT", "You Take the Point.\n"); break;
            case 3: pPlayer->Radio("%!MRAD_POSITION", "Hold This Position.\n"); break;
            case 4: pPlayer->Radio("%!MRAD_REGROUP", "Regroup Team.\n"); break;
            case 5: pPlayer->Radio("%!MRAD_FOLLOWME", "Follow Me.\n"); break;
            case 6: pPlayer->Radio("%!MRAD_HITASSIST", "Taking Fire.. Need Assistance!.\n"); break;
            }
        }
    }
}

void Radio2(CBasePlayer* pPlayer, int slot)
{
    if (gpGlobals->time > pPlayer->m_flRadioTime)
    {
        if (pPlayer->m_iRadioMessages > 0)
        {
            pPlayer->m_iRadioMessages--;
            pPlayer->m_flRadioTime = gpGlobals->time + 1.5;

            switch (slot)
            {
            case 1: pPlayer->Radio("%!MRAD_GO", "Go go go!\n"); break;
            case 2: pPlayer->Radio("%!MRAD_FALLBACK", "Team, fall back!\n"); break;
            case 3: pPlayer->Radio("%!MRAD_STICKTOG", "Stick together, team.\n"); break;
            case 4: pPlayer->Radio("%!MRAD_GETINPOS", "Get in position and wait for my go.\n"); break;
            case 5: pPlayer->Radio("%!MRAD_STORMFRONT", "Storm the Front!\n"); break;
            case 6: pPlayer->Radio("%!MRAD_REPORTIN", "Report in, team.\n"); break;
            }
        }
    }
}

void Radio3(CBasePlayer* pPlayer, int slot)
{
    if (gpGlobals->time > pPlayer->m_flRadioTime)
    {
        if (pPlayer->m_iRadioMessages > 0)
        {
            pPlayer->m_iRadioMessages--;
            pPlayer->m_flRadioTime = gpGlobals->time + 1.5;

            switch (slot)
            {
            case 1:
                if (RANDOM_LONG(0, 1))
                    pPlayer->Radio("%!MRAD_AFFIRM", "Affirmative.\n");
                else
                    pPlayer->Radio("%!MRAD_ROGER", "Roger that.\n");
                break;
            case 2: pPlayer->Radio("%!MRAD_ENEMYSPOT", "Enemy spotted.\n"); break;
            case 3: pPlayer->Radio("%!MRAD_BACKUP", "Need backup.\n"); break;
            case 4: pPlayer->Radio("%!MRAD_CLEAR", "Sector clear.\n"); break;
            case 5: pPlayer->Radio("%!MRAD_INPOS", "I'm in position.\n"); break;
            case 6: pPlayer->Radio("%!MRAD_REPRTINGIN", "Reporting in.\n"); break;
            case 7: pPlayer->Radio("%!MRAD_BLOW", "Get out of there, it's gonna blow!\n"); break;
            case 8: pPlayer->Radio("%!MRAD_NEGATIVE", "Negative.\n"); break;
            case 9: pPlayer->Radio("%!MRAD_ENEMYDOWN", "Enemy down.\n"); break;
            }
        }
    }
}

int CountTeams(void)
{
    int iNumCT = 0;
    int iNumTerrorist = 0;

    CBaseEntity* pPlayer = NULL;

    while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
    {
        if (FNullEnt(pPlayer->edict()))
            break;

        CBasePlayer* pl = GetClassPtr((CBasePlayer*)pPlayer->pev);
        int iTeam = pl->m_iTeam;

        if (iTeam && pPlayer->pev->flags != FL_DORMANT)
        {
            if (iTeam == TEAM_CT)
                iNumCT++;
            else if (iTeam == TEAM_TERRORIST)
                iNumTerrorist++;
        }
    }

    return iNumCT - iNumTerrorist;
}

void ListPlayers(CBasePlayer* pPlayer)
{
    int iCount = 0;
    char szNum[4];
    char message[120];

    message[0] = '\0';

    CBaseEntity* pEntity = UTIL_FindEntityByClassname(NULL, "player");

    while (pEntity)
    {
        if (FNullEnt(pEntity->edict()))
            break;

        if (pEntity->pev->flags != FL_DORMANT)
        {
            CBasePlayer* pListPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

            sprintf(szNum, "%d", ++iCount);
            strcpy(message, "\n");
            strcat(message, szNum);
            strcat(message, " : ");
            strcat(message, STRING(pListPlayer->pev->netname));

            ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, message);
        }

        pEntity = UTIL_FindEntityByClassname(pEntity, "player");
    }
}

void KickPlayer(int playerIndex, int voteCount, int team)
{
    int playerCount = 0;
    bool bKicked = false;

    if (playerIndex)
    {
        CBaseEntity* pEntity = NULL;

        while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
        {
            if (FNullEnt(pEntity->edict()))
                break;

            if (pEntity->IsPlayer())
            {
                if (pEntity->pev->flags != FL_DORMANT && ++playerCount == playerIndex)
                {
                    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

                    if (pPlayer->m_iTeam == team
                        && 0.65 * (double)CountTeamPlayers(team) < (double)voteCount
                        && CountTeamPlayers(team) > 1
                        && voteCount <= 99)
                    {
                        bKicked = true;
                        edict_t* pEdict = pEntity->pev->pContainingEntity;
                        pPlayer->m_bBeingKicked = 1;

                        char szMsg[120];
                        strcpy(szMsg, "Kicked ");
                        strcat(szMsg, STRING(pEntity->pev->netname));
                        UTIL_ClientPrintAll(HUD_PRINTCENTER, szMsg);

                        CLIENT_COMMAND(pEdict, "disconnect\n");
                    }
                }
            }
        }
    }

    if (bKicked)
    {
        CBaseEntity* pEntity = NULL;

        while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
        {
            if (FNullEnt(pEntity->edict()))
                break;

            if (pEntity->IsPlayer())
            {
                if (pEntity->pev->flags != FL_DORMANT)
                {
                    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);
                    pPlayer->m_iCurrentKickVote = 0;
                }
            }
        }
    }
}

