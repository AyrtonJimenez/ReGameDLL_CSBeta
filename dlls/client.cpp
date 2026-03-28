#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "weapons.h"
#ifdef REGAMEDLL_ADD
#include "hostage.h"
#include "hostage_localnav.h"
#endif

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL int		g_iSkillLevel;
extern DLL_GLOBAL ULONG		g_ulFrameCount;

extern void CopyToBodyQue(entvars_t* pev);
extern int giPrecacheGrunt;
extern int gmsgSayText;

int gmsgBuyClose = 0;
int gmsgMoney = 0;
int gmsgBlinkAcct = 0;
int gmsgTeamScore = 0;
int gmsgTeamInfo = 0;
int gmsgScoreAttrib = 0;
int gmsgNVGToggle = 0;
int gmsgRoundTime = 0;
int gmsgShowTimer = 0;
int gmsgBarTime = 0;
int gmsgResetHUD = 0;
int gmsgItemStatus = 0;
int gmsgAllowSpec = 0;
int gmsgBombDrop = 0;
int gmsgBombPickup = 0;
int gmsgHostagePos = 0;
int gmsgHostageK = 0;
int gmsgStatusIcon = 0;
int gmsgLocation = 0;
int gmsgVGUIMenu = 0;
int gmsgShowMenu = 0;
int gmsgSendAudio = 0;
int gmsgRadar = 0;

char g_szMapBriefingText[512];

void set_suicide_frame(entvars_t* pev)
{
    if (!FStrEq(STRING(pev->model), "models/player.mdl"))
        return;

    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_TOSS;
    pev->deadflag = DEAD_DEAD;

    pev->nextthink = -1.0f;
}

BOOL ClientConnect(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
    return g_pGameRules->ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void ClientDisconnect(edict_t* pEntity)
{
    if (g_fGameOver)
        return;

    char text[256];
    sprintf(text, "- %s has left the game\n", STRING(pEntity->v.netname));
    MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
    WRITE_BYTE(ENTINDEX(pEntity));
    WRITE_STRING(text);
    MESSAGE_END();

    CSound* pSound;
    pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(pEntity));
    {
        if (pSound)
        {
            pSound->Reset();
        }
    }

    pEntity->v.takedamage = DAMAGE_NO;
    pEntity->v.solid = SOLID_NOT;
    pEntity->v.flags = FL_DORMANT;
    UTIL_SetOrigin(&pEntity->v, pEntity->v.origin);

    g_pGameRules->ClientDisconnected(pEntity);
}

void respawn(entvars_t* pev, BOOL fCopyCorpse)
{
    if (gpGlobals->coop || gpGlobals->deathmatch)
    {
        if (fCopyCorpse)
        {
            CopyToBodyQue(pev);
        }

        GetClassPtr((CBasePlayer*)pev)->Spawn();
    }
    else
    {
        SERVER_COMMAND("reload\n");
    }
}

void ClientKill(edict_t* pEntity)
{
    entvars_t* pev = &pEntity->v;

    CBasePlayer* pPlayer = (CBasePlayer*)CBasePlayer::Instance(pev);

    if (pPlayer->m_iJoiningState)
        return;

    if (pPlayer->m_fNextSuicideTime > gpGlobals->time)
        return;

    pPlayer->m_fNextSuicideTime = gpGlobals->time + 1;

    pev->health = 0;
    pPlayer->Killed(pev, GIB_NEVER);

    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    if (mp->m_pVIP == pPlayer)
        mp->m_iConsecutiveVIP = 10;
}

void ClientPutInServer(edict_t* pEntity)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    entvars_t* pev = &pEntity->v;
    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pev);
    pPlayer->SetCustomDecalFrames(-1);

    pPlayer->m_bNotKilled = true;
    pPlayer->m_bBeingKicked = 0;
    pPlayer->m_iIgnoreMessages = 0;
    pPlayer->m_iTeamKills = 0;
    pPlayer->m_bJustConnected = true;

    char* infoBuf = g_engfuncs.pfnGetInfoKeyBuffer(pEntity);
    char* pLeftHand = g_engfuncs.pfnInfoKeyValue(infoBuf, "lefthand");
    pPlayer->m_bLeftHanded = (pLeftHand && *pLeftHand != '0');

    pPlayer->Spawn();

    pPlayer->m_iAccount = 800;
    pPlayer->m_fGameHUDInitialized = FALSE;

    pPlayer->m_flDisplayHistory &= ~DHF_BUY_ZONE;
    pev->flags |= FL_SPECTATOR;
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NOCLIP;
    pev->effects = EF_NODRAW;
    pev->takedamage = DAMAGE_NO;
    pev->deadflag = DEAD_DEAD;

    pev->velocity = g_vecZero;
    pev->punchangle = g_vecZero;

    pPlayer->m_iJoiningState = READINGLTEXT;
    pPlayer->m_iTeam = TEAM_UNASSIGNED;
    pev->fixangle = 1;
    pPlayer->m_iModelName = 1;
    pPlayer->m_bContextHelp = true;
    pPlayer->m_bHasNightVision = false;

    g_engfuncs.pfnSetClientMaxspeed(ENT(pPlayer->pev), 0.001f);
    SET_MODEL(ENT(pPlayer->pev), "models/player.mdl");

    pPlayer->SetThink(NULL);

    int team = pPlayer->m_iTeam;
    MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo, NULL);
    WRITE_BYTE(ENTINDEX(ENT(pPlayer->pev)));
    WRITE_BYTE(team);
    MESSAGE_END();

    if (team)
        pPlayer->SetScoreboardAttributes(NULL);

    CBaseEntity* pIntroCamera = UTIL_FindEntityByClassname(NULL, "trigger_camera");
    pPlayer->m_pIntroCamera = pIntroCamera;

    if (mp && mp->m_iMapDefaultBuyStatus == 2)
    {
        if (pIntroCamera)
            mp->m_iMapDefaultBuyStatus = 1;
        else
            mp->m_iMapDefaultBuyStatus = 0;
    }

    CBaseEntity* pTarget = NULL;
    if (pPlayer->m_pIntroCamera)
        pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pPlayer->m_pIntroCamera->pev->target));

    if (pPlayer->m_pIntroCamera && pTarget)
    {
        Vector CamAngles = UTIL_VecToAngles((pTarget->pev->origin - pPlayer->m_pIntroCamera->pev->origin).Normalize());
        CamAngles.x = -CamAngles.x;

        UTIL_SetOrigin(pPlayer->pev, pPlayer->m_pIntroCamera->pev->origin);

        pPlayer->pev->angles = CamAngles;
        pPlayer->pev->v_angle = pPlayer->pev->angles;

        pPlayer->m_fIntroCamTime = gpGlobals->time + 6.0f;
        pEntity->v.view_ofs = g_vecZero;
    }
    else
    {
        pPlayer->m_iTeam = TEAM_CT;
        if (mp)
            mp->GetPlayerSpawnSpot(pPlayer);
        pPlayer->m_iTeam = TEAM_UNASSIGNED;

        pPlayer->pev->v_angle = g_vecZero;
        pPlayer->pev->angles = gpGlobals->v_forward;
    }

    pPlayer->m_iJoiningState = SHOWLTEXT;

    const char* name = "<unconnected>";
    if (pPlayer->pev->netname && STRING(pPlayer->pev->netname)[0])
        name = STRING(pPlayer->pev->netname);

    UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("%s connected\n", name));
}

void Host_Say(edict_t* pEntity, int teamonly)
{
    CBasePlayer* client;
    int		j;
    char* p;
    char	text[128];
    char    szTemp[256];
    const char* cpSay = "say";
    const char* cpSayTeam = "say_team";
    const char* pcmd = CMD_ARGV(0);

    BOOL isDead = FALSE;

    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)&pEntity->v);

    if (pPlayer->m_flLastTalk != 0.0f && gpGlobals->time - pPlayer->m_flLastTalk < 1.0f)
        return;

    pPlayer->m_flLastTalk = gpGlobals->time;

    if (pEntity->v.deadflag)
        isDead = TRUE;

    if (CMD_ARGC() == 0)
        return;

    if (!stricmp(pcmd, cpSay) || !stricmp(pcmd, cpSayTeam))
    {
        if (CMD_ARGC() >= 2)
        {
            p = (char*)CMD_ARGS();
        }
        else
        {
            return;
        }
    }
    else
    {
        if (CMD_ARGC() >= 2)
        {
            sprintf(szTemp, "%s %s", (char*)pcmd, (char*)CMD_ARGS());
        }
        else
        {
            sprintf(szTemp, "%s", (char*)pcmd);
        }
        p = szTemp;
    }

    if (*p == '"')
    {
        p++;
        p[strlen(p) - 1] = 0;
    }

    char* pc = p;
    for (; pc != NULL && *pc != 0; pc++)
    {
        if (isprint(*pc) && !isspace(*pc))
        {
            pc = NULL;
            break;
        }
    }
    if (pc != NULL)
        return;

    if (teamonly)
    {
        if (pPlayer->m_iTeam == TEAM_CT && !isDead)
            sprintf(text, "%c(Counter-Terrorist) %s :   ", 2, STRING(pEntity->v.netname));
        else if (pPlayer->m_iTeam == TEAM_TERRORIST && !isDead)
            sprintf(text, "%c(Terrorist) %s :   ", 2, STRING(pEntity->v.netname));
        else if (pPlayer->m_iTeam == TEAM_CT && isDead)
            sprintf(text, "%c*DEAD*(CT) %s :   ", 2, STRING(pEntity->v.netname));
        else if (pPlayer->m_iTeam == TEAM_TERRORIST && isDead)
            sprintf(text, "%c*DEAD*(Terrorist) %s :   ", 2, STRING(pEntity->v.netname));
    }
    else
    {
        if (!isDead)
            sprintf(text, "%c%s :    ", 2, STRING(pEntity->v.netname));
        else
            sprintf(text, "%c*DEAD*%s :    ", 2, STRING(pEntity->v.netname));
    }

    j = sizeof(text) - 2 - strlen(text);
    if ((int)strlen(p) > j)
        p[j] = 0;

    strcat(text, p);
    strcat(text, "\n");

    client = NULL;
    while ((client = (CBasePlayer*)UTIL_FindEntityByClassname(client, "player")) != NULL)
    {
        if (FNullEnt(client->edict()))
            break;

        if (!client->pev)
            continue;

        if (client->edict() == pEntity)
            continue;

        if (!(client->IsNetClient()))
            continue;

        if (teamonly && g_pGameRules->PlayerRelationship(client, CBaseEntity::Instance(pEntity)) != GR_TEAMMATE)
            continue;

        if (client->pev->deadflag != 0)
        {
            if (!isDead)
                continue;
        }
        else
        {
            if (isDead)
                continue;
        }

        if (client->m_iIgnoreMessages == 1)
        {
            if (client->m_iTeam != pPlayer->m_iTeam)
                continue;
        }
        else if (client->m_iIgnoreMessages != 0)
        {
            continue;
        }

        MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->edict());
        WRITE_BYTE(ENTINDEX(client->edict()));
        WRITE_STRING(text);
        MESSAGE_END();
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity);
    WRITE_BYTE(ENTINDEX(pEntity));
    WRITE_STRING(text);
    MESSAGE_END();

    g_engfuncs.pfnServerPrint(text);
}

int SelectDefaultTeam(void)
{
    CHalfLifeMultiplay* pGameRules = (CHalfLifeMultiplay*)g_pGameRules;

    int team = TEAM_CT;

    if (pGameRules->m_iNumTerrorist < pGameRules->m_iNumCT)
        team = TEAM_TERRORIST;

    if (!pGameRules->TeamFull(team))
        return team;

    team = (team == TEAM_TERRORIST) ? TEAM_CT : TEAM_TERRORIST;

    if (pGameRules->TeamFull(team))
        return TEAM_UNASSIGNED;

    return team;
}

extern float g_flWeaponCheat;
extern char g_szMapBriefingText[];

extern void ShowMenu(CBasePlayer* pPlayer, int bitsValidSlots, int nDisplayTime, int fNeedMore, char* pszText);
extern BOOL HandleMenu_ChooseTeam(CBasePlayer* pPlayer, int slot);
extern void HandleMenu_ChooseAppearance(CBasePlayer* pPlayer, int slot);
extern void BuyPistol(CBasePlayer* pPlayer, int slot);
extern void BuyRifle(CBasePlayer* pPlayer, int slot);
extern void BuyShotgun(CBasePlayer* pPlayer, int slot);
extern void BuySubMachineGun(CBasePlayer* pPlayer, int slot);
extern void BuyMachineGun(CBasePlayer* pPlayer, int slot);
extern void BuyItem(CBasePlayer* pPlayer, int slot);
extern int BuyAmmo(CBasePlayer* pPlayer, int nSlot, BOOL bBlinkMoney);
extern void Radio1(CBasePlayer* pPlayer, int slot);
extern void Radio2(CBasePlayer* pPlayer, int slot);
extern void Radio3(CBasePlayer* pPlayer, int slot);
extern void ListPlayers(CBasePlayer* pPlayer);
extern void KickPlayer(int playerIndex, int voteCount, int team);

void ClientCommand(edict_t* pEntity)
{
    const char* pcmd = CMD_ARGV(0);
    const char* pstr;

    if (!pEntity->pvPrivateData)
        return;

    entvars_t* pev = &pEntity->v;
    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pev);

    if (pPlayer->m_bBeingKicked)
        return;

    if (FStrEq(pcmd, "say"))
    {
        Host_Say(pEntity, 0);
    }
    else if (FStrEq(pcmd, "say_team"))
    {
        Host_Say(pEntity, 1);
    }
    else if (FStrEq(pcmd, "vote"))
    {
        if (gpGlobals->time < pPlayer->m_flNextVoteTime)
            return;

        pPlayer->m_flNextVoteTime = gpGlobals->time + 3.0f;

        char* pszArg = (char*)CMD_ARGV(1);

        if (strlen(pszArg) <= 3)
        {
            int targetIndex = atoi(pszArg);
            char s[4];
            char text[100];
            strcpy(text, "Vote casted against player #");
            sprintf(s, "%d", targetIndex);
            strcat(text, s);

            UTIL_ClientPrintAll(HUD_PRINTCONSOLE, text);

            pPlayer->m_iCurrentKickVote = targetIndex;

            int voteCount = 0;
            CBaseEntity* pEnt = NULL;
            while ((pEnt = UTIL_FindEntityByClassname(pEnt, "player")) != NULL)
            {
                if (FNullEnt(pEnt->edict()) || !g_engfuncs.pfnEntOffsetOfPEntity(pEnt->pev->pContainingEntity))
                    break;

                if (pEnt->IsPlayer())
                {
                    CBasePlayer* pl = GetClassPtr((CBasePlayer*)pEnt->pev);
                    if (pl->m_iTeam == pPlayer->m_iTeam && pl->m_iCurrentKickVote == pPlayer->m_iCurrentKickVote)
                        voteCount++;
                }
            }

            KickPlayer(targetIndex, voteCount, pPlayer->m_iTeam);
        }
    }
    else if (FStrEq(pcmd, "nightvision"))
    {
        if (pPlayer->m_bHasNightVision)
        {
            edict_t* pEntityEdict = pPlayer->pev->pContainingEntity;

            if (pPlayer->m_bNightVisionOn)
            {
                EMIT_SOUND(pEntityEdict, CHAN_ITEM, "items/nvg_off.wav", RANDOM_FLOAT(0.92f, 1.0f), ATTN_NORM);
                MESSAGE_BEGIN(MSG_ONE, gmsgNVGToggle, NULL, pPlayer->pev->pContainingEntity);
                WRITE_BYTE(0);
                MESSAGE_END();
                pPlayer->m_bNightVisionOn = false;
                CLIENT_COMMAND(pEntity, "lambert 1.5\n");
            }
            else
            {

                EMIT_SOUND(pEntityEdict, CHAN_ITEM, "items/nvg_on.wav", RANDOM_FLOAT(0.92f, 1.0f), ATTN_NORM);
                MESSAGE_BEGIN(MSG_ONE, gmsgNVGToggle, NULL, pPlayer->pev->pContainingEntity);
                WRITE_BYTE(1);
                MESSAGE_END();
                pPlayer->m_bNightVisionOn = true;
                CLIENT_COMMAND(pEntity, "lambert -1.1\n");
            }
        }
    }
    else if (FStrEq(pcmd, "menuselect"))
    {
        int slot = atoi(CMD_ARGV(1));

        if (slot == 10)
            pPlayer->m_iMenu = Menu_OFF;

        switch (pPlayer->m_iMenu)
        {
        case Menu_OFF:
            break;
        case Menu_ChooseTeam:
            if (!HandleMenu_ChooseTeam(pPlayer, slot))
            {
                if (pPlayer->m_iJoiningState)
                    ShowMenu(pPlayer, 19, -1, 0, "#Team_Select");
                else
                    ShowMenu(pPlayer, 531, -1, 0, "#IG_Team_Select");
                pPlayer->m_iMenu = Menu_ChooseTeam;
            }
            break;
        case Menu_IGChooseTeam:
            HandleMenu_ChooseTeam(pPlayer, slot);
            break;
        case Menu_ChooseAppearance:
            HandleMenu_ChooseAppearance(pPlayer, slot);
            break;
        case Menu_Buy:
        {
            CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
            switch (slot)
            {
            case 1:
                ShowMenu(pPlayer, 527, -1, 0, "#BuyPistol");
                pPlayer->m_iMenu = Menu_BuyPistol;
                break;
            case 2:
                if (mp->m_bMapHasVIPSafetyZone == 1 && pPlayer->m_iTeam == TEAM_TERRORIST)
                    ShowMenu(pPlayer, 512, -1, 0, "#AS_BuyShotgun");
                else
                    ShowMenu(pPlayer, 515, -1, 0, "#BuyShotgun");
                pPlayer->m_iMenu = Menu_BuyShotgun;
                break;
            case 3:
                if (mp->m_bMapHasVIPSafetyZone == 1)
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 519, -1, 0, "#AS_CT_BuySubMachineGun");
                    else
                        ShowMenu(pPlayer, 520, -1, 0, "#AS_T_BuySubMachineGun");
                }
                else
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 519, -1, 0, "#CT_BuySubMachineGun");
                    else
                        ShowMenu(pPlayer, 525, -1, 0, "#T_BuySubMachineGun");
                }
                pPlayer->m_iMenu = Menu_BuySubMachineGun;
                break;
            case 4:
                if (mp->m_bMapHasVIPSafetyZone == 1)
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 588, -1, 0, "#AS_CT_BuyRifle");
                    else
                        ShowMenu(pPlayer, 625, -1, 0, "#AS_T_BuyRifle");
                }
                else
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 636, -1, 0, "#CT_BuyRifle");
                    else
                        ShowMenu(pPlayer, 627, -1, 0, "#T_BuyRifle");
                }
                pPlayer->m_iMenu = Menu_BuyRifle;
                break;
            case 5:
                if (mp->m_bMapHasVIPSafetyZone == 1 && pPlayer->m_iTeam == TEAM_TERRORIST)
                    ShowMenu(pPlayer, 512, -1, 0, "#AS_T_BuyMachineGun");
                else
                    ShowMenu(pPlayer, 513, -1, 0, "#BuyMachineGun");
                pPlayer->m_iMenu = Menu_BuyMachineGun;
                break;
            case 6:
                if ((pPlayer->m_iSignals & SIGNAL_BUY) && BuyAmmo(pPlayer, 1, TRUE))
                {
                    while (BuyAmmo(pPlayer, 1, FALSE))
                        ;
                }
                break;
            case 7:
                if ((pPlayer->m_iSignals & SIGNAL_BUY) && BuyAmmo(pPlayer, 2, TRUE))
                {
                    while (BuyAmmo(pPlayer, 2, FALSE))
                        ;
                }
                break;
            case 8:
                if (!(pPlayer->m_iSignals & SIGNAL_BUY))
                    break;
                if (mp->m_bMapHasBombTarget == 1)
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 575, -1, 0, "#DCT_BuyItem");
                    else
                        ShowMenu(pPlayer, 575, -1, 0, "#DT_BuyItem");
                }
                else
                {
                    ShowMenu(pPlayer, 559, -1, 0, "#BuyItem");
                }
                pPlayer->m_iMenu = Menu_BuyItem;
                break;
            }
            break;
        }
        case Menu_BuyPistol:
            BuyPistol(pPlayer, slot);
            break;
        case Menu_BuyRifle:
            BuyRifle(pPlayer, slot);
            break;
        case Menu_BuyMachineGun:
            BuyMachineGun(pPlayer, slot);
            break;
        case Menu_BuyShotgun:
            BuyShotgun(pPlayer, slot);
            break;
        case Menu_BuySubMachineGun:
            BuySubMachineGun(pPlayer, slot);
            break;
        case Menu_BuyItem:
            BuyItem(pPlayer, slot);
            break;
        case Menu_Radio1:
            Radio1(pPlayer, slot);
            break;
        case Menu_Radio2:
            Radio2(pPlayer, slot);
            break;
        case Menu_Radio3:
            Radio3(pPlayer, slot);
            break;
        default:
            ALERT(at_console, "ClientCommand(): Invalid menu selected\n");
            break;
        }
    }
    else if (FStrEq(pcmd, "chooseteam"))
    {
        if (!pPlayer->m_iJoiningState && pPlayer->m_iMenu != Menu_ChooseAppearance)
        {
            if (pPlayer->m_bTeamChanged && pev->deadflag)
            {
                ClientPrint(pev, HUD_PRINTCONSOLE, "Only 1 team change per death for Observers");
            }
            else
            {
                CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
                if (mp->m_bMapHasVIPSafetyZone == 1 && pPlayer->m_iTeam == TEAM_CT)
                    ShowMenu(pPlayer, 535, -1, 0, "#IG_VIP_Team_Select");
                else
                    ShowMenu(pPlayer, 531, -1, 0, "#IG_Team_Select");
                pPlayer->m_iMenu = Menu_ChooseTeam;
            }
        }
    }
    else if (FStrEq(pcmd, "showbriefing"))
    {
        if (g_szMapBriefingText[0] && pPlayer->m_iTeam)
        {
            pPlayer->MenuPrint(pPlayer, g_szMapBriefingText);
            pPlayer->m_bMissionBriefing = TRUE;
        }
    }
    else if (FStrEq(pcmd, "listplayers"))
    {
        ListPlayers(pPlayer);
    }
    else if (FStrEq(pcmd, "ignoremsg"))
    {
        if (pPlayer->m_iIgnoreMessages == 0)
        {
            pPlayer->m_iIgnoreMessages = 1;
            ClientPrint(pev, HUD_PRINTCONSOLE, "Now ignoring BROADCAST messages\n");
        }
        else if (pPlayer->m_iIgnoreMessages == 1)
        {
            pPlayer->m_iIgnoreMessages = 2;
            ClientPrint(pev, HUD_PRINTCONSOLE, "Now ignoring TEAM/BROADCAST messages\n");
        }
        else
        {
            pPlayer->m_iIgnoreMessages = 0;
            ClientPrint(pev, HUD_PRINTCONSOLE, "Now accepting ALL text messages\n");
        }
    }
    else if (FStrEq(pcmd, "ignorerad"))
    {
        if (!pPlayer->m_bIgnoreRadio)
        {
            pPlayer->m_bIgnoreRadio = TRUE;
            ClientPrint(pev, HUD_PRINTCONSOLE, "Now IGNORING radio messages\n");
        }
        else
        {
            pPlayer->m_bIgnoreRadio = FALSE;
            ClientPrint(pev, HUD_PRINTCONSOLE, "Now ACCEPTING radio messages\n");
        }
    }
    else if (FStrEq(pcmd, "become_vip"))
    {
        CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
        if (!pPlayer->m_iJoiningState && pPlayer->m_iTeam == TEAM_CT)
            mp->AddToVIPQueue(pPlayer);
        else
            ClientPrint(pev, HUD_PRINTCONSOLE, "This command not available\n");
    }
    else if (!pev->deadflag)
    {
        if (FStrEq(pcmd, "radio1"))
        {
            ShowMenu(pPlayer, 575, -1, 0, "#RadioA");
            pPlayer->m_iMenu = Menu_Radio1;
        }
        else if (FStrEq(pcmd, "radio2"))
        {
            ShowMenu(pPlayer, 575, -1, 0, "#RadioB");
            pPlayer->m_iMenu = Menu_Radio2;
        }
        else if (FStrEq(pcmd, "radio3"))
        {
            ShowMenu(pPlayer, 1023, -1, 0, "#RadioC");
            pPlayer->m_iMenu = Menu_Radio3;
        }
        else if (FStrEq(pcmd, "drop"))
        {

            GetClassPtr((CBasePlayer*)pev)->DropPlayerItem((char*)CMD_ARGV(1));
        }
        else if (FStrEq(pcmd, "fov"))
        {
            if (g_flWeaponCheat && CMD_ARGC() > 1)
                GetClassPtr((CBasePlayer*)pev)->m_iFOV = atoi(CMD_ARGV(1));
            else
                CLIENT_PRINTF(pEntity, print_console, UTIL_VarArgs("\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer*)pev)->m_iFOV));
        }
        else if (FStrEq(pcmd, "use"))
        {
            GetClassPtr((CBasePlayer*)pev)->SelectItem((char*)CMD_ARGV(1));
        }
        else if (((pstr = strstr(pcmd, "weapon_")) != NULL) && (pstr == pcmd))
        {
            GetClassPtr((CBasePlayer*)pev)->SelectItem(pcmd);
        }
        else if (FStrEq(pcmd, "lastinv"))
        {
            GetClassPtr((CBasePlayer*)pev)->SelectLastItem();
        }
        else if (FStrEq(pcmd, "buyammo1"))
        {
            if (pPlayer->m_iSignals & SIGNAL_BUY)
                BuyAmmo(pPlayer, 1, TRUE);
        }
        else if (FStrEq(pcmd, "buyammo2"))
        {
            if (pPlayer->m_iSignals & SIGNAL_BUY)
                BuyAmmo(pPlayer, 2, TRUE);
        }
        else if (FStrEq(pcmd, "buyequip"))
        {
            if (pPlayer->m_iSignals & SIGNAL_BUY)
            {
                CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
                if (mp->m_bMapHasBombTarget == 1)
                {
                    if (pPlayer->m_iTeam == TEAM_CT)
                        ShowMenu(pPlayer, 575, -1, 0, "#DCT_BuyItem");
                    else
                        ShowMenu(pPlayer, 575, -1, 0, "#DT_BuyItem");
                }
                else
                {
                    ShowMenu(pPlayer, 559, -1, 0, "#BuyItem");
                }
                pPlayer->m_iMenu = Menu_BuyItem;
            }
        }
        else if (FStrEq(pcmd, "buy"))
        {
            if (pPlayer->m_iSignals & SIGNAL_BUY)
            {
                ShowMenu(pPlayer, 767, -1, 0, "#Buy");
                pPlayer->m_iMenu = Menu_Buy;
            }
        }
        else if (g_pGameRules->ClientCommand(GetClassPtr((CBasePlayer*)pev), pcmd))
        {

        }
        else
        {
            ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs("Unknown command: %s\n", pcmd));
        }
    }
}

void ClientUserInfoChanged(edict_t* pEntity, char* infobuffer)
{
    if (!pEntity->pvPrivateData)
        return;

    CBasePlayer* pPlayer = NULL;
    if (pEntity)
        pPlayer = (CBasePlayer*)pEntity->pvPrivateData;

    char* pModelName;
    switch (pPlayer->m_iModelName)
    {
    case 2: CLIENT_COMMAND(pEntity, "model terror\n"); pModelName = "terror"; break;
    case 3: CLIENT_COMMAND(pEntity, "model arab\n"); pModelName = "arab"; break;
    case 4: CLIENT_COMMAND(pEntity, "model arctic\n"); pModelName = "arctic"; break;
    case 5: CLIENT_COMMAND(pEntity, "model gsg9\n"); pModelName = "gsg9"; break;
    case 6: CLIENT_COMMAND(pEntity, "model gign\n"); pModelName = "gign"; break;
    case 7: CLIENT_COMMAND(pEntity, "model sas\n"); pModelName = "sas"; break;
    case 8: CLIENT_COMMAND(pEntity, "model vip\n"); pModelName = "vip"; break;
    default: CLIENT_COMMAND(pEntity, "model urban\n"); pModelName = "urban"; break;
    }

    SET_CLIENT_KEY_VALUE(ENTINDEX(pEntity), g_engfuncs.pfnGetInfoKeyBuffer(pEntity), "model", pModelName);

    if (pEntity->v.netname && STRING(pEntity->v.netname)[0] != 0 && !FStrEq(STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name")))
    {
        if (pPlayer->pev->deadflag)
        {
            char text[256];
            strcpy(text, STRING(pEntity->v.netname));
            SET_CLIENT_KEY_VALUE(ENTINDEX(pEntity), g_engfuncs.pfnGetInfoKeyBuffer(pEntity), "name", text);
            return;
        }

        char text[256];
        sprintf(text, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
        MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
        WRITE_BYTE(ENTINDEX(pEntity));
        WRITE_STRING(text);
        MESSAGE_END();

        UTIL_LogPrintf("\"%s<%i>\" changed name to \"%s<%i>\"\n", STRING(pEntity->v.netname), GETPLAYERUSERID(pEntity), g_engfuncs.pfnInfoKeyValue(infobuffer, "name"), GETPLAYERUSERID(pEntity));
    }

    char* pLeftHand = g_engfuncs.pfnInfoKeyValue(infobuffer, "lefthand");
    pPlayer->m_bLeftHanded = (pLeftHand && *pLeftHand != '0') ? TRUE : FALSE;

    g_pGameRules->ClientUserInfoChanged(GetClassPtr((CBasePlayer*)&pEntity->v), infobuffer);
}

void ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
    int				i;
    CBaseEntity* pClass;

    for (i = 0; i < edictCount; i++)
    {
        if (pEdictList[i].free)
            continue;

        if (i < clientMax || !pEdictList[i].pvPrivateData)
            continue;

        pClass = CBaseEntity::Instance(&pEdictList[i]);

        if (pClass && pClass->pev->flags >= 0)
        {

            pClass->Activate();
        }
        else
        {
            ALERT(at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname));
        }
    }
}

void PlayerPreThink(edict_t* pEntity)
{
    CBasePlayer* pPlayer = NULL;

    if (pEntity)
        pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

    if (pPlayer)
        pPlayer->PreThink();
}

void PlayerPostThink(edict_t* pEntity)
{
    entvars_t* pev = &pEntity->v;
    CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

    if (pPlayer)
        pPlayer->PostThink();
}

void ParmsNewLevel(void)
{

}

void ParmsChangeLevel(void)
{
    SAVERESTOREDATA* pSaveData = (SAVERESTOREDATA*)gpGlobals->pSaveData;

    if (pSaveData)
        pSaveData->connectionCount = BuildChangeList(pSaveData->levelList, MAX_LEVEL_CONNECTIONS);
}

void StartFrame(void)
{
    if (g_pGameRules)
        g_pGameRules->Think();

    if (g_fGameOver)
        return;

#ifdef REGAMEDLL_ADD
    CLocalNav::Think();
#endif

    gpGlobals->teamplay = 1.0;
    g_iSkillLevel = (int)CVAR_GET_FLOAT("skill");
    g_ulFrameCount++;
}

void ClientPrecache(void)
{
    PRECACHE_SOUND("player/pl_shot1.wav");
    PRECACHE_SOUND("player/pl_die1.wav");
    PRECACHE_SOUND("player/headshot1.wav");
    PRECACHE_SOUND("player/headshot2.wav");
    PRECACHE_SOUND("player/headshot3.wav");
    PRECACHE_SOUND("player/bhit_flesh-1.wav");
    PRECACHE_SOUND("player/bhit_flesh-2.wav");
    PRECACHE_SOUND("player/bhit_flesh-3.wav");
    PRECACHE_SOUND("player/bhit_kevlar-1.wav");
    PRECACHE_SOUND("player/bhit_helmet-1.wav");
    PRECACHE_SOUND("player/die1.wav");
    PRECACHE_SOUND("player/die2.wav");
    PRECACHE_SOUND("player/die3.wav");
    PRECACHE_SOUND("player/death6.wav");

    PRECACHE_SOUND("radio/locknload.wav");
    PRECACHE_SOUND("radio/letsgo.wav");
    PRECACHE_SOUND("radio/moveout.wav");
    PRECACHE_SOUND("radio/com_go.wav");
    PRECACHE_SOUND("radio/rescued.wav");
    PRECACHE_SOUND("radio/rounddraw.wav");

    PRECACHE_SOUND("items/kevlar.wav");
    PRECACHE_SOUND("items/nvg_on.wav");
    PRECACHE_SOUND("items/nvg_off.wav");
    PRECACHE_SOUND("items/equip_nvg.wav");

    PRECACHE_SOUND("weapons/c4_beep1.wav");
    PRECACHE_SOUND("weapons/c4_beep2.wav");
    PRECACHE_SOUND("weapons/c4_beep3.wav");
    PRECACHE_SOUND("weapons/c4_beep4.wav");
    PRECACHE_SOUND("weapons/c4_beep5.wav");
    PRECACHE_SOUND("weapons/c4_explode1.wav");
    PRECACHE_SOUND("weapons/c4_plant.wav");
    PRECACHE_SOUND("weapons/c4_disarm.wav");
    PRECACHE_SOUND("weapons/c4_disarmed.wav");
    PRECACHE_SOUND("weapons/explode3.wav");
    PRECACHE_SOUND("weapons/explode4.wav");
    PRECACHE_SOUND("weapons/explode5.wav");

    PRECACHE_SOUND("player/sprayer.wav");

    PRECACHE_SOUND("player/pl_fallpain2.wav");
    PRECACHE_SOUND("player/pl_fallpain3.wav");

    PRECACHE_SOUND("player/pl_snow1.wav");
    PRECACHE_SOUND("player/pl_snow2.wav");
    PRECACHE_SOUND("player/pl_snow3.wav");
    PRECACHE_SOUND("player/pl_snow4.wav");
    PRECACHE_SOUND("player/pl_snow5.wav");
    PRECACHE_SOUND("player/pl_snow6.wav");

    PRECACHE_SOUND("player/pl_step1.wav");
    PRECACHE_SOUND("player/pl_step2.wav");
    PRECACHE_SOUND("player/pl_step3.wav");
    PRECACHE_SOUND("player/pl_step4.wav");

    PRECACHE_SOUND("common/npc_step1.wav");
    PRECACHE_SOUND("common/npc_step2.wav");
    PRECACHE_SOUND("common/npc_step3.wav");
    PRECACHE_SOUND("common/npc_step4.wav");

    PRECACHE_SOUND("player/pl_metal1.wav");
    PRECACHE_SOUND("player/pl_metal2.wav");
    PRECACHE_SOUND("player/pl_metal3.wav");
    PRECACHE_SOUND("player/pl_metal4.wav");

    PRECACHE_SOUND("player/pl_dirt1.wav");
    PRECACHE_SOUND("player/pl_dirt2.wav");
    PRECACHE_SOUND("player/pl_dirt3.wav");
    PRECACHE_SOUND("player/pl_dirt4.wav");

    PRECACHE_SOUND("player/pl_duct1.wav");
    PRECACHE_SOUND("player/pl_duct2.wav");
    PRECACHE_SOUND("player/pl_duct3.wav");
    PRECACHE_SOUND("player/pl_duct4.wav");

    PRECACHE_SOUND("player/pl_grate1.wav");
    PRECACHE_SOUND("player/pl_grate2.wav");
    PRECACHE_SOUND("player/pl_grate3.wav");
    PRECACHE_SOUND("player/pl_grate4.wav");

    PRECACHE_SOUND("player/pl_slosh1.wav");
    PRECACHE_SOUND("player/pl_slosh2.wav");
    PRECACHE_SOUND("player/pl_slosh3.wav");
    PRECACHE_SOUND("player/pl_slosh4.wav");

    PRECACHE_SOUND("player/pl_tile1.wav");
    PRECACHE_SOUND("player/pl_tile2.wav");
    PRECACHE_SOUND("player/pl_tile3.wav");
    PRECACHE_SOUND("player/pl_tile4.wav");
    PRECACHE_SOUND("player/pl_tile5.wav");

    PRECACHE_SOUND("player/pl_swim1.wav");
    PRECACHE_SOUND("player/pl_swim2.wav");
    PRECACHE_SOUND("player/pl_swim3.wav");
    PRECACHE_SOUND("player/pl_swim4.wav");

    PRECACHE_SOUND("player/pl_ladder1.wav");
    PRECACHE_SOUND("player/pl_ladder2.wav");
    PRECACHE_SOUND("player/pl_ladder3.wav");
    PRECACHE_SOUND("player/pl_ladder4.wav");

    PRECACHE_SOUND("player/pl_wade1.wav");
    PRECACHE_SOUND("player/pl_wade2.wav");
    PRECACHE_SOUND("player/pl_wade3.wav");
    PRECACHE_SOUND("player/pl_wade4.wav");

    PRECACHE_SOUND("debris/wood1.wav");
    PRECACHE_SOUND("debris/wood2.wav");
    PRECACHE_SOUND("debris/wood3.wav");

    PRECACHE_SOUND("plats/train_use1.wav");

    PRECACHE_SOUND("buttons/spark5.wav");
    PRECACHE_SOUND("buttons/spark6.wav");
    PRECACHE_SOUND("debris/glass1.wav");
    PRECACHE_SOUND("debris/glass2.wav");
    PRECACHE_SOUND("debris/glass3.wav");

    PRECACHE_SOUND(SOUND_FLASHLIGHT_ON);
    PRECACHE_SOUND(SOUND_FLASHLIGHT_OFF);

    PRECACHE_SOUND("common/bodysplat.wav");

    PRECACHE_SOUND("player/pl_pain2.wav");
    PRECACHE_SOUND("player/pl_pain4.wav");
    PRECACHE_SOUND("player/pl_pain5.wav");
    PRECACHE_SOUND("player/pl_pain6.wav");
    PRECACHE_SOUND("player/pl_pain7.wav");

    PRECACHE_MODEL("models/player.mdl");

    PRECACHE_SOUND("common/wpn_hudoff.wav");
    PRECACHE_SOUND("common/wpn_hudon.wav");
    PRECACHE_SOUND("common/wpn_moveselect.wav");
    PRECACHE_SOUND("common/wpn_select.wav");
    PRECACHE_SOUND("common/wpn_denyselect.wav");

    PRECACHE_SOUND("player/geiger6.wav");
    PRECACHE_SOUND("player/geiger5.wav");
    PRECACHE_SOUND("player/geiger4.wav");
    PRECACHE_SOUND("player/geiger3.wav");
    PRECACHE_SOUND("player/geiger2.wav");
    PRECACHE_SOUND("player/geiger1.wav");

    if (giPrecacheGrunt)
        UTIL_PrecacheOther("monster_human_grunt");
}

const char* GetGameDescription()
{
    if (g_pGameRules)
        return g_pGameRules->GetGameDescription();
    else
        return "Half-Life";
}

void PlayerCustomization(edict_t* pEntity, customization_t* pCust)
{
    CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

    if (!pPlayer)
    {
        ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
        return;
    }

    if (!pCust)
    {
        ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
        return;
    }

    switch (pCust->resource.type)
    {
    case t_decal:
        pPlayer->SetCustomDecalFrames(pCust->nUserData2);
        break;
    case t_sound:
    case t_skin:
    case t_model:
        break;
    default:
        ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
        break;
    }
}

void SpectatorConnect(edict_t* pEntity)
{
    CBaseSpectator* pPlayer = NULL;

    if (pEntity)
        pPlayer = (CBaseSpectator*)GET_PRIVATE(pEntity);

    if (pPlayer)
        pPlayer->SpectatorConnect();
}

void SpectatorDisconnect(edict_t* pEntity)
{
    CBaseSpectator* pPlayer = NULL;

    if (pEntity)
        pPlayer = (CBaseSpectator*)GET_PRIVATE(pEntity);

    if (pPlayer)
        pPlayer->SpectatorDisconnect();
}

void SpectatorThink(edict_t* pEntity)
{
    CBaseSpectator* pSpectator = NULL;

    if (pEntity)
        pSpectator = (CBaseSpectator*)GET_PRIVATE(pEntity);

    if (pSpectator)
        pSpectator->SpectatorThink();
}

