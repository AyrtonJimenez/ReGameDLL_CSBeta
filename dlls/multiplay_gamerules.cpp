#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"game.h"
#include	"items.h"
#include	"mapinfo.h"

extern DLL_GLOBAL CGameRules* g_pGameRules;
extern void ShowMenu(CBasePlayer* pPlayer, int bitsValidSlots, int nDisplayTime, int fNeedMore, char* pszText);
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;
extern int gmsgScoreInfo;
extern int gmsgMOTD;
extern int gmsgTeamScore;
extern int gmsgTeamInfo;
extern int gmsgShowTimer;
extern int gmsgAllowSpec;
extern int gmsgSendAudio;
extern int gmsgStatusIcon;

#include "hostage.h"

#define ITEM_RESPAWN_TIME	30
#define WEAPON_RESPAWN_TIME	20
#define AMMO_RESPAWN_TIME	20

#define ROUND_RESPAWN_TIME	20

int CountTeamPlayers(int team)
{
    int count = 0;
    CBaseEntity* pPlayer = NULL;

    while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
    {
        if (FNullEnt(pPlayer->edict()))
            break;

        if (pPlayer->pev->flags != FL_DORMANT)
        {
            CBasePlayer* pl = GetClassPtr((CBasePlayer*)pPlayer->pev);
            if (pl->m_iTeam == team)
                count++;
        }
    }
    return count;
}

void EndRoundMessage(const char* sentence)
{
    static hudtextparms_t textParms = {
        -1.0f, 0.4f,
        2,
        20, 255, 20, 200,
        0, 255, 0, 200,
        0.01f,
        0.7f,
        4.0f,
        0.07f,
        1
    };

    char szBuf[256];

    UTIL_HudMessageAll(textParms, sentence);

    strcpy(szBuf, "***");
    strcat(szBuf, sentence);
    strcat(szBuf, "***\n");
    UTIL_LogPrintf(szBuf);

    if (g_pGameRules)
    {
        CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
        sprintf(szBuf, "CT wins = %i\nTerrorist wins = %i\n", mp->m_iNumCTWins, mp->m_iNumTerroristWins);
        UTIL_LogPrintf(szBuf);
    }
}

void Broadcast(const char* sentence)
{
    if (!sentence)
        return;

    char buf[32];
    strcpy(buf, "%!MRAD_");
    strcat(buf, UTIL_VarArgs("%s", sentence));

    MESSAGE_BEGIN(MSG_BROADCAST, gmsgSendAudio, NULL);
    WRITE_BYTE(0);
    WRITE_STRING(buf);
    MESSAGE_END();
}

void ReadMultiplayCvars(CHalfLifeMultiplay* pRules)
{
    pRules->m_iRoundTime = (int)CVAR_GET_FLOAT("mp_roundtime");
    pRules->m_iC4Timer = (int)CVAR_GET_FLOAT("mp_c4timer");
    pRules->m_iIntroRoundTime = (int)CVAR_GET_FLOAT("mp_freezetime");
    pRules->m_iLimitTeams = (int)CVAR_GET_FLOAT("mp_limitteams");

    if (pRules->m_iRoundTime > 9)
    {
        CVAR_SET_FLOAT("mp_roundtime", 9);
        pRules->m_iRoundTime = 9;
    }
    else if (pRules->m_iRoundTime <= 2)
    {
        CVAR_SET_FLOAT("mp_roundtime", 1);
        pRules->m_iRoundTime = 1;
    }

    if (pRules->m_iIntroRoundTime > 15)
    {
        CVAR_SET_FLOAT("mp_roundtime", 9);
        pRules->m_iIntroRoundTime = 15;
    }
    else if (pRules->m_iIntroRoundTime < 0)
    {
        CVAR_SET_FLOAT("mp_freezetime", 0);
        pRules->m_iIntroRoundTime = 0;
    }

    if (pRules->m_iC4Timer > 90)
    {
        CVAR_SET_FLOAT("mp_c4timer", 90);
        pRules->m_iC4Timer = 90;
    }
    else if (pRules->m_iC4Timer <= 9)
    {
        CVAR_SET_FLOAT("mp_c4timer", 10);
        pRules->m_iC4Timer = 10;
    }

    if (pRules->m_iLimitTeams > 20)
    {
        CVAR_SET_FLOAT("mp_limitteams", 20);
        pRules->m_iLimitTeams = 20;
    }
    else if (pRules->m_iLimitTeams <= 0)
    {
        CVAR_SET_FLOAT("mp_limitteams", 0);
        pRules->m_iLimitTeams = 20;
    }
}

CHalfLifeMultiplay::CHalfLifeMultiplay()
{
    RefreshSkillData();

    m_iAccountCT = 0;
    m_iAccountTerrorist = 0;
    m_iHostagesRescued = 0;
    m_iRoundWinStatus = 0;

    m_iNumCTWins = 0;
    m_iNumTerroristWins = 0;

    m_pBomber = NULL;
    m_pC4Carrier = NULL;
    m_pVIP = NULL;

    m_iNumCT = 0;
    m_iNumTerrorist = 0;
    m_iNumSpawnableCT = 0;
    m_iNumSpawnableTerrorist = 0;

    m_iMapDefaultBuyStatus = 2;
    m_flIntermissionEndTime = 0;
    m_flRestartRoundTime = 0;

    g_fGameOver = FALSE;

    m_iLoserBonus = 1400;
    m_iNumConsecutiveCTLoses = 0;
    m_iNumConsecutiveTerroristLoses = 0;
    m_iC4Guy = 0;
    m_bBombDefused = false;
    m_bTargetBombed = false;
    m_bFreezePeriod = TRUE;
    m_bLevelInitialized = false;
    m_bGameStarted = false;
    m_bCompleteReset = true;
    m_iNumEscapers = 0;
    m_bCTCantBuy = false;
    m_bTCantBuy = false;
    m_tmNextPeriodicThink = 0.0;
    m_flRequiredEscapeRatio = 0.5f;
    m_flBombRadius = 500.0f;
    m_iTotalGunCount = 0;
    m_iTotalGrenadeCount = 0;
    m_iTotalArmourCount = 0;
    m_iConsecutiveVIP = 0;
    m_iUnBalancedRounds = 0;
    m_iNumEscapeRounds = 0;
    m_bMapHasEscapeZone = false;
    m_bMapHasVIPSafetyZone = false;
    m_bMapHasBombZone = false;
    m_bMapHasRescueZone = false;

    m_pVIPQueue[0] = NULL;
    m_pVIPQueue[1] = NULL;
    m_pVIPQueue[2] = NULL;
    m_pVIPQueue[3] = NULL;
    m_pVIPQueue[4] = NULL;

    CVAR_SET_FLOAT("cl_himodels", 0);

    ReadMultiplayCvars(this);

    m_iIntroRoundTime += 2;
    m_iRoundTimeSecs = m_iIntroRoundTime;
    m_fMaxRoundTime = (float)(120 * m_iRoundTime);

    if (IS_DEDICATED_SERVER())
    {
        const char* servercfgfile = CVAR_GET_STRING("servercfgfile");

        if (servercfgfile && servercfgfile[0])
        {
            char szCommand[256];
            sprintf(szCommand, "exec %s\n", servercfgfile);
            SERVER_COMMAND(szCommand);
        }
    }
    else
    {
        const char* lservercfgfile = CVAR_GET_STRING("lservercfgfile");

        if (lservercfgfile && lservercfgfile[0])
        {
            ALERT(at_console, "Executing listen server config file\n");
            char szCommand[256];
            sprintf(szCommand, "exec %s\n", lservercfgfile);
            SERVER_COMMAND(szCommand);
        }
    }

    m_fRoundStartTime = 0;
    m_fRoundStartTimeReal = 0;
}

void CHalfLifeMultiplay::RefreshSkillData(void)
{
    CGameRules::RefreshSkillData();

    gSkillData.suitchargerCapacity = 30;
    gSkillData.plrDmgCrowbar = 25;
    gSkillData.plrDmg9MM = 12;
    gSkillData.plrDmg357 = 40;
    gSkillData.plrDmgMP5 = 12;
    gSkillData.plrDmgM203Grenade = 100;
    gSkillData.plrDmgBuckshot = 20;
    gSkillData.plrDmgCrossbowClient = 20;
    gSkillData.plrDmgRPG = 120;
    gSkillData.plrDmgEgonWide = 20;
    gSkillData.plrDmgEgonNarrow = 10;
    gSkillData.plrDmgHandGrenade = 100;
    gSkillData.plrDmgSatchel = 120;
    gSkillData.plrDmgTripmine = 150;
    gSkillData.plrDmgHornet = 10;
}

void CHalfLifeMultiplay::RemoveGuns(void)
{
    CBaseEntity* toremove = NULL;

    while ((toremove = UTIL_FindEntityByClassname(toremove, "weaponbox")) != NULL)
    {
        ((CWeaponBox*)toremove)->Kill();
    }
}

BOOL CHalfLifeMultiplay::IsMultiplayer(void)
{
    return TRUE;
}

BOOL CHalfLifeMultiplay::IsDeathmatch(void)
{
    return TRUE;
}

BOOL CHalfLifeMultiplay::IsCoOp(void)
{
    return gpGlobals->coop;
}

BOOL CHalfLifeMultiplay::FShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
    if (!pWeapon->CanDeploy())
        return FALSE;

    if (!pPlayer->m_pActiveItem)
        return TRUE;

    if (pPlayer->m_pActiveItem->CanHolster()
        && CBasePlayerItem::ItemInfoArray[pWeapon->m_iId].iWeight > CBasePlayerItem::ItemInfoArray[pPlayer->m_pActiveItem->m_iId].iWeight)
        return TRUE;

    return FALSE;
}

float CHalfLifeMultiplay::FlPlayerFallDamage(CBasePlayer* pPlayer)
{
    pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
    return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED * 1.25;
}

BOOL CHalfLifeMultiplay::FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
    if (pAttacker)
    {
        if (PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE)
        {
            if (CVAR_GET_FLOAT("mp_friendlyfire") == 0 && pAttacker != pPlayer)
                return FALSE;
        }
    }

    return TRUE;
}

BOOL CHalfLifeMultiplay::TeamFull(int team_id)
{
    if (team_id == 1)
        return m_iNumTerrorist >= m_iSpawnPointCount_Terrorist;
    if (team_id == 2)
        return m_iNumCT >= m_iSpawnPointCount_CT;
    return FALSE;
}

BOOL CHalfLifeMultiplay::TeamStacked(int newTeam_id, int curTeam_id)
{
    if (newTeam_id == curTeam_id)
        return FALSE;

    if (newTeam_id == TEAM_TERRORIST)
    {
        if (curTeam_id)
            return (m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeams - 1);
        else
            return (m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeams);
    }
    else if (newTeam_id == TEAM_CT)
    {
        if (curTeam_id)
            return (m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeams - 1);
        else
            return (m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeams);
    }

    return FALSE;
}

float CHalfLifeMultiplay::TimeRemaining(void)
{
    return (float)m_iRoundTimeSecs - gpGlobals->time + m_fRoundStartTime;
}

BOOL CHalfLifeMultiplay::IsFreezePeriod(void)
{
    return m_bFreezePeriod;
}

void CHalfLifeMultiplay::PlayerThink(CBasePlayer* pPlayer)
{
    if (g_fGameOver)
    {
        if (pPlayer->m_afButtonPressed & (IN_ATTACK | IN_JUMP | IN_DUCK | IN_USE | IN_ATTACK2))
            m_iEndIntermissionButtonHit = TRUE;

        pPlayer->m_afButtonPressed = 0;
        pPlayer->pev->button = 0;
        pPlayer->m_afButtonReleased = 0;
    }

    if (pPlayer->m_iMenu != Menu_ChooseTeam && pPlayer->m_iJoiningState == SHOWTEAMSELECT)
    {
        ShowMenu(pPlayer, 19, -1, 0, "#Team_Select");
        pPlayer->m_iMenu = Menu_ChooseTeam;
        pPlayer->m_iJoiningState = PICKINGTEAM;
    }
}

void CHalfLifeMultiplay::PlayerSpawn(CBasePlayer* pPlayer)
{
    if (pPlayer->m_bJustConnected)
        return;

    pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

    BOOL addDefault = TRUE;

    CBaseEntity* pWeaponEntity = NULL;
    while ((pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip")) != NULL)
    {
        pWeaponEntity->Touch(pPlayer);
        addDefault = FALSE;
    }

    if (pPlayer->m_bNotKilled)
        addDefault = FALSE;

    if (addDefault)
        pPlayer->GiveDefaultItems();

    pPlayer->SetPlayerModel(FALSE);
}

BOOL CHalfLifeMultiplay::FPlayerCanRespawn(CBasePlayer* pPlayer)
{
    if (pPlayer->m_iNumSpawns > 0)
        return FALSE;

    if (m_iNumTerrorist > 0 && m_iNumCT > 0
        && m_fRoundStartTime + 20.0 < gpGlobals->time)
        return FALSE;

    if (pPlayer->m_iMenu == Menu_ChooseAppearance)
        return FALSE;

    return TRUE;
}

float CHalfLifeMultiplay::FlPlayerSpawnTime(CBasePlayer* pPlayer)
{
    return gpGlobals->time;
}

BOOL CHalfLifeMultiplay::AllowAutoTargetCrosshair(void)
{
    return CVAR_GET_FLOAT("mp_autocrosshair") != 0;
}

int CHalfLifeMultiplay::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
    return 1;
}

void CHalfLifeMultiplay::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
    DeathNotice(pVictim, pKiller, pInflictor);

    pVictim->m_iDeaths++;
    pVictim->m_bNotKilled = FALSE;

    if (pVictim->m_iTeam == TEAM_CT && m_pBomber == pVictim
        || pVictim->m_iTeam == TEAM_TERRORIST && m_pC4Carrier == pVictim)
    {
        PickNewCommander(pVictim->m_iTeam);
    }

    CBaseEntity* ktmp = CBaseEntity::Instance(pKiller);
    CBasePlayer* pKillerPlayer = NULL;

    if (ktmp && ktmp->Classify() == CLASS_PLAYER)
        pKillerPlayer = (CBasePlayer*)ktmp;

    FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);

    if (pVictim->pev == pKiller)
    {
        pKiller->frags -= 1;
    }
    else if (ktmp && ktmp->IsPlayer())
    {
        CBasePlayer* killer = GetClassPtr((CBasePlayer*)pKiller);

        pKiller->frags += IPointsForKill(pKillerPlayer, pVictim);

        if (killer->m_iTeam == pVictim->m_iTeam)
        {
            killer->AddAccount(-3300, TRUE);
            killer->m_iTeamKills++;
            killer->m_bTKPunished = TRUE;
            ClientPrint(killer->pev, HUD_PRINTCENTER, "You killed a teammate!");
            ClientPrint(killer->pev, HUD_PRINTCONSOLE, UTIL_VarArgs("Teammate kills: %i of 3\n", killer->m_iTeamKills));

            if (killer->m_iTeamKills == 3 && CVAR_GET_FLOAT("mp_autokick") != 0)
            {
                ClientPrint(killer->pev, HUD_PRINTCONSOLE, "You are being banned from the server for killing teammates\n");
                int userid = g_engfuncs.pfnGetPlayerUserId(killer->edict());
                if (userid != -1)
                    SERVER_COMMAND(UTIL_VarArgs("kick # %d\n", userid));
            }

            if (!(killer->m_flDisplayHistory & DHF_FRIEND_KILLED))
            {
                killer->m_flDisplayHistory |= DHF_FRIEND_KILLED;
                killer->HintMessage("Careful!\nKilling teammates will not be tolerated!", FALSE, FALSE);
            }
        }
        else
        {
            killer->AddAccount(300, TRUE);

            if (!(killer->m_flDisplayHistory & DHF_ENEMY_KILLED))
            {
                killer->m_flDisplayHistory |= DHF_ENEMY_KILLED;
                killer->HintMessage("You killed an Enemy!\nWin the round by eliminating\nthe opposing force.", FALSE, FALSE);
            }
        }

        FireTargets("game_playerkill", ktmp, ktmp, USE_TOGGLE, 0);
    }
    else
    {

        pKiller->frags -= 1;
    }

    MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
    WRITE_BYTE(ENTINDEX(pVictim->edict()));
    WRITE_SHORT((int)pVictim->pev->frags);
    WRITE_SHORT(pVictim->m_iDeaths);
    MESSAGE_END();

    CBaseEntity* pKillerEnt = CBaseEntity::Instance(pKiller);
    if (pKillerEnt && pKillerEnt->Classify() == CLASS_PLAYER)
    {
        CBasePlayer* pk = (CBasePlayer*)pKillerEnt;
        MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
        WRITE_BYTE(ENTINDEX(pk->edict()));
        WRITE_SHORT((int)pk->pev->frags);
        WRITE_SHORT(pk->m_iDeaths);
        MESSAGE_END();

        pk->m_flNextDecalTime = gpGlobals->time;
    }

    if (pVictim->HasNamedPlayerItem("weapon_satchel"))
        DeactivateSatchels(pVictim);
}

void CHalfLifeMultiplay::DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
    CBaseEntity::Instance(pKiller);

    const char* killer_weapon_name = "world";
    int killer_index = 0;

    if (pKiller->flags & FL_CLIENT)
    {
        killer_index = ENTINDEX(ENT(pKiller));

        if (pInflictor)
        {
            if (pInflictor == pKiller)
            {

                CBasePlayer* pPK = (CBasePlayer*)CBaseEntity::Instance(pKiller);
                if (pPK && pPK->m_pActiveItem)
                    killer_weapon_name = pPK->m_pActiveItem->pszName();
            }
            else
            {
                killer_weapon_name = STRING(pInflictor->classname);
            }
        }
    }
    else
    {
        killer_weapon_name = STRING(pInflictor->classname);
    }

    if (!strncmp(killer_weapon_name, "weapon_", 7))
        killer_weapon_name += 7;
    else if (!strncmp(killer_weapon_name, "monster_", 8))
        killer_weapon_name += 8;
    else if (!strncmp(killer_weapon_name, "func_", 5))
        killer_weapon_name += 5;

    MESSAGE_BEGIN(MSG_ALL, gmsgDeathMsg);
    WRITE_BYTE(killer_index);
    WRITE_BYTE(ENTINDEX(pVictim->edict()));
    WRITE_STRING(killer_weapon_name);
    MESSAGE_END();

    if (!strcmp(killer_weapon_name, "egon"))
        killer_weapon_name = "gluon gun";
    else if (!strcmp(killer_weapon_name, "gauss"))
        killer_weapon_name = "tau_cannon";

    if (pVictim->pev == pKiller)
    {

        UTIL_LogPrintf("\"%s<%i>\" killed self with %s\n",
            STRING(pVictim->pev->netname),
            g_engfuncs.pfnGetPlayerUserId(pVictim->edict()),
            killer_weapon_name);
    }
    else if (pKiller->flags & FL_CLIENT)
    {

        UTIL_LogPrintf("\"%s<%i>\" killed \"%s<%i>\" with %s\n",
            STRING(pKiller->netname),
            g_engfuncs.pfnGetPlayerUserId(ENT(pKiller)),
            STRING(pVictim->pev->netname),
            g_engfuncs.pfnGetPlayerUserId(pVictim->edict()),
            killer_weapon_name);
    }
    else
    {

        UTIL_LogPrintf("\"%s<%i>\" killed by world with %s\n",
            STRING(pVictim->pev->netname),
            g_engfuncs.pfnGetPlayerUserId(pVictim->edict()),
            killer_weapon_name);
    }

    CheckWinConditions();
}

void CHalfLifeMultiplay::PlayerGotWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
}

BOOL CHalfLifeMultiplay::CanHavePlayerItem(CBasePlayer* pPlayer, CBasePlayerItem* pItem)
{
    return CGameRules::CanHavePlayerItem(pPlayer, pItem);
}

int CHalfLifeMultiplay::WeaponShouldRespawn(CBasePlayerItem* pWeapon)
{
    if (pWeapon->pev->spawnflags & SF_NORESPAWN)
    {
        return GR_WEAPON_RESPAWN_NO;
    }

    return GR_WEAPON_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlWeaponRespawnTime(CBasePlayerItem* pWeapon)
{
    return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

float CHalfLifeMultiplay::FlWeaponTryRespawn(CBasePlayerItem* pWeapon)
{
    if (pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD))
    {
        if (NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE))
            return 0;

        return FlWeaponRespawnTime(pWeapon);
    }

    return 0;
}

Vector CHalfLifeMultiplay::VecWeaponRespawnSpot(CBasePlayerItem* pWeapon)
{
    return pWeapon->pev->origin;
}

BOOL CHalfLifeMultiplay::CanHaveItem(CBasePlayer* pPlayer, CItem* pItem)
{
    return TRUE;
}

void CHalfLifeMultiplay::PlayerGotItem(CBasePlayer* pPlayer, CItem* pItem)
{
}

int CHalfLifeMultiplay::ItemShouldRespawn(CItem* pItem)
{
    if (pItem->pev->spawnflags & SF_NORESPAWN)
    {
        return GR_ITEM_RESPAWN_NO;
    }

    return GR_ITEM_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlItemRespawnTime(CItem* pItem)
{
    return gpGlobals->time + ITEM_RESPAWN_TIME;
}

Vector CHalfLifeMultiplay::VecItemRespawnSpot(CItem* pItem)
{
    return pItem->pev->origin;
}

void CHalfLifeMultiplay::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

int CHalfLifeMultiplay::AmmoShouldRespawn(CBasePlayerAmmo* pAmmo)
{
    if (pAmmo->pev->spawnflags & SF_NORESPAWN)
    {
        return GR_AMMO_RESPAWN_NO;
    }

    return GR_AMMO_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlAmmoRespawnTime(CBasePlayerAmmo* pAmmo)
{
    return gpGlobals->time + AMMO_RESPAWN_TIME;
}

Vector CHalfLifeMultiplay::VecAmmoRespawnSpot(CBasePlayerAmmo* pAmmo)
{
    return pAmmo->pev->origin;
}

float CHalfLifeMultiplay::FlHealthChargerRechargeTime(void)
{
    return 60;
}

float CHalfLifeMultiplay::FlHEVChargerRechargeTime(void)
{
    return 30;
}

int CHalfLifeMultiplay::DeadPlayerWeapons(CBasePlayer* pPlayer)
{
    return GR_PLR_DROP_GUN_ACTIVE;
}

int CHalfLifeMultiplay::DeadPlayerAmmo(CBasePlayer* pPlayer)
{
    return GR_PLR_DROP_AMMO_ACTIVE;
}

const char* CHalfLifeMultiplay::GetTeamID(CBaseEntity* pEntity)
{
    return "";
}

int CHalfLifeMultiplay::PlayerRelationship(CBasePlayer* pPlayer, CBaseEntity* pTarget)
{
    if (!pPlayer || !pTarget)
        return GR_NOTTEAMMATE;

    if (!pTarget->IsPlayer())
        return GR_NOTTEAMMATE;

    CBasePlayer* pl1 = GetClassPtr((CBasePlayer*)pPlayer->pev);
    CBasePlayer* pl2 = GetClassPtr((CBasePlayer*)pTarget->pev);

    if (pl1->m_iTeam == pl2->m_iTeam)
        return GR_TEAMMATE;

    return GR_NOTTEAMMATE;
}

BOOL CHalfLifeMultiplay::PlayFootstepSounds(CBasePlayer* pl, float fvol)
{
    if (CVAR_GET_FLOAT("mp_footsteps") == 0.0)
        return FALSE;

    if (pl->IsOnLadder())
        return TRUE;

    float speed = sqrt(pl->pev->velocity.x * pl->pev->velocity.x + pl->pev->velocity.y * pl->pev->velocity.y);

    if (speed > 190.0)
        return TRUE;

    return FALSE;
}

BOOL CHalfLifeMultiplay::FAllowFlashlight(void)
{
    return CVAR_GET_FLOAT("mp_flashlight") != 0;
}

BOOL CHalfLifeMultiplay::FAllowMonsters(void)
{
    return CVAR_GET_FLOAT("mp_allowmonsters") != 0;
}

void CHalfLifeMultiplay::Think(void)
{
    if (m_fRoundStartTime == 0.0f)
    {
        m_fRoundStartTime = gpGlobals->time;
        m_fRoundStartTimeReal = gpGlobals->time;
    }

    if (g_fGameOver)
    {
        if (gpGlobals->time >= m_flIntermissionEndTime)
        {
            if (m_iEndIntermissionButtonHit)
            {
                ChangeLevel();
                return;
            }

            if (gpGlobals->time >= m_flIntermissionEndTime + 120.0f)
            {
                ChangeLevel();
                return;
            }
        }
        return;
    }

    float flTimeLimit = timelimit.value * 60.0f;
    if (flTimeLimit != 0.0f && gpGlobals->time >= flTimeLimit)
    {
        GoToIntermission();
        return;
    }

    float flRoundStartTime = m_fRoundStartTime;

    if (!m_bFreezePeriod)
    {
        float flTimeLeft = flRoundStartTime + (float)m_iRoundTimeSecs - gpGlobals->time;
        if (flTimeLeft > 0)
            goto label_post_round;

        UTIL_FindEntityByClassname(NULL, "grenade");

        if (m_bMapHasBombTarget == 1)
        {
            EndRoundMessage("CTs win!!! Target has been saved!");
            Broadcast("ctwin");
            m_iAccountCT += 2000;

            ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 1;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
            m_iNumCTWins++;
            MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
            WRITE_BYTE(2);
            WRITE_SHORT(m_iNumCTWins);
            MESSAGE_END();
        }
        else if (UTIL_FindEntityByClassname(NULL, "hostage_entity"))
        {
            EndRoundMessage("Terrorists win! Hostages have not been rescued!");
            Broadcast("terwin");
            m_iAccountTerrorist += 2000;

            ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 2;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
            m_iNumTerroristWins++;
            MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
            WRITE_BYTE(1);
            WRITE_SHORT(m_iNumTerroristWins);
            MESSAGE_END();
        }
        else if (m_bMapHasEscapeZone == 1)
        {
            EndRoundMessage("CTs win! Terrorists have not escaped!");
            Broadcast("ctwin");

            ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 1;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
            m_iNumCTWins++;
            MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
            WRITE_BYTE(2);
            WRITE_SHORT(m_iNumCTWins);
            MESSAGE_END();
        }
        else if (m_bMapHasVIPSafetyZone == 1)
        {
            EndRoundMessage("Terrorists win! VIP has not escaped!");
            Broadcast("terwin");
            m_iAccountTerrorist += 2000;

            ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 2;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
            m_iNumTerroristWins++;
            MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
            WRITE_BYTE(1);
            WRITE_SHORT(m_iNumTerroristWins);
            MESSAGE_END();
        }

        m_fRoundStartTime = gpGlobals->time + 60.0f;
    }
    else
    {
        float flTimeLeft = flRoundStartTime + (float)m_iRoundTimeSecs - gpGlobals->time;
        if (flTimeLeft > 0)
            goto label_post_round;

        m_bFreezePeriod = FALSE;

        int pick = RANDOM_LONG(0, 3);
        char szCTRadio[40], szTRadio[40];

        switch (pick)
        {
        case 0:
            strcpy(szCTRadio, "%!MRAD_MOVEOUT");
            strcpy(szTRadio, "%!MRAD_MOVEOUT");
            break;
        case 1:
            strcpy(szCTRadio, "%!MRAD_LETSGO");
            strcpy(szTRadio, "%!MRAD_LETSGO");
            break;
        case 2:
            strcpy(szCTRadio, "%!MRAD_LOCKNLOAD");
            strcpy(szTRadio, "%!MRAD_LOCKNLOAD");
            break;
        default:
            strcpy(szCTRadio, "%!MRAD_GO");
            strcpy(szTRadio, "%!MRAD_GO");
            break;
        }

        if (m_bMapHasEscapeZone == 1)
        {
            strcpy(szCTRadio, "%!MRAD_ELIM");
            strcpy(szTRadio, "%!MRAD_GETOUT");
        }
        else if (m_bMapHasVIPSafetyZone == 1)
        {
            strcpy(szCTRadio, "%!MRAD_VIP");
            strcpy(szTRadio, "%!MRAD_LOCKNLOAD");
        }

        m_iRoundTimeSecs = 60 * m_iRoundTime;
        m_fRoundStartTime = gpGlobals->time;

        BOOL bCTRadioPlayed = FALSE;
        BOOL bTRadioPlayed = FALSE;

        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);
            if (!pPlayer || FNullEnt(pPlayer->pev) || pPlayer->m_iJoiningState)
                continue;

            if (pPlayer->m_iTeam == TEAM_CT && !bCTRadioPlayed)
            {
                pPlayer->Radio(szCTRadio, NULL);
                bCTRadioPlayed = TRUE;
            }
            else if (pPlayer->m_iTeam == TEAM_TERRORIST && !bTRadioPlayed)
            {
                pPlayer->Radio(szTRadio, NULL);
                bTRadioPlayed = TRUE;
            }

            pPlayer->SyncRoundTimer();
            pPlayer->ResetMaxSpeed();
        }
    }

label_post_round:

    if (m_flRestartRoundTime != 0.0f && m_flRestartRoundTime <= gpGlobals->time)
    {
        RestartRound();
    }

    if (!m_bLevelInitialized)
    {
        m_iSpawnPointCount_Terrorist = 0;
        m_iSpawnPointCount_CT = 0;

        CBaseEntity* pSpot = NULL;
        while ((pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch")) != NULL)
            m_iSpawnPointCount_Terrorist++;

        while ((pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_start")) != NULL)
            m_iSpawnPointCount_CT++;

        m_bLevelInitialized = true;
    }

    if (m_tmNextPeriodicThink < gpGlobals->time)
    {
        int iRestartRound = (int)CVAR_GET_FLOAT("sv_restartround");
        if (iRestartRound > 0)
        {
            if (iRestartRound > 10)
                iRestartRound = 10;

            const char* pszUnit = (iRestartRound == 1) ? "SECOND" : "SECONDS";
            const char* pszMsg = UTIL_VarArgs("The game will restart in %i %s\n", iRestartRound, pszUnit);

            EndRoundMessage(pszMsg);
            UTIL_ClientPrintAll(HUD_PRINTCONSOLE, pszMsg);

            m_bCompleteReset = true;
            m_flRestartRoundTime = gpGlobals->time + (float)iRestartRound;
            CVAR_SET_FLOAT("sv_restartround", 0);
        }

        m_tmNextPeriodicThink = gpGlobals->time + 1.0f;

        if (CVAR_GET_FLOAT("sv_accelerate") != 5.0f)
            CVAR_SET_FLOAT("sv_accelerate", 5.0f);
        if (CVAR_GET_FLOAT("sv_friction") != 4.0f)
            CVAR_SET_FLOAT("sv_friction", 4.0f);
        if (CVAR_GET_FLOAT("sv_stopspeed") != 75.0f)
            CVAR_SET_FLOAT("sv_stopspeed", 75.0f);
    }
}

void CHalfLifeMultiplay::CheckWinConditions(void)
{
    if (m_iRoundWinStatus != 0)
        return;

    int NumAliveTerrorist = 0, NumAliveCT = 0;
    int NumDeadTerrorist = 0, NumDeadCT = 0;
    BOOL bNeededPlayers = FALSE;

    m_iNumSpawnableCT = 0;
    m_iNumSpawnableTerrorist = 0;
    m_iNumCT = 0;
    m_iNumTerrorist = 0;
    m_iHaveEscaped = 0;

    CBaseEntity* pPlayer = NULL;
    while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
    {
        if (FNullEnt(pPlayer->edict()))
            break;

        CBasePlayer* pl = GetClassPtr((CBasePlayer*)pPlayer->pev);

        if (pPlayer->pev->flags == FL_DORMANT)
            continue;

        if (pl->m_iTeam == TEAM_TERRORIST)
        {
            m_iNumTerrorist++;
            if (pl->m_iMenu != Menu_ChooseAppearance)
                m_iNumSpawnableTerrorist++;
            if (pPlayer->pev->deadflag)
                NumDeadTerrorist++;
            else
                NumAliveTerrorist++;
            if (pl->m_bHasEscaped == TRUE)
                m_iHaveEscaped++;
        }
        else if (pl->m_iTeam == TEAM_CT)
        {
            m_iNumCT++;
            if (pl->m_iMenu != Menu_ChooseAppearance)
                m_iNumSpawnableCT++;
            if (pPlayer->pev->deadflag)
                NumDeadCT++;
            else
                NumAliveCT++;
        }
    }

    if (!m_iNumSpawnableTerrorist || !m_iNumSpawnableCT)
    {
        UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Scoring will not start until both teams have players.\n");

        if (!m_bGameStarted)
        {
            UTIL_LogPrintf("*** First player has joined the server. Game Commencing! ***\n");
            ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 3;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
            ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time;
            m_bGameStarted = true;
            return;
        }
        bNeededPlayers = TRUE;
    }

    if (m_bMapHasVIPSafetyZone == TRUE)
    {
        CBasePlayer* pVIP = m_pVIP;
        if (pVIP)
        {
            if (pVIP->m_bHasEscaped)
            {
                EndRoundMessage("VIP has escaped!");
                Broadcast("ctwin");
                m_iAccountCT += 2750;
                goto CT_WIN;
            }
            if (pVIP->pev->deadflag != DEAD_NO)
            {
                EndRoundMessage("VIP has been assassinated!");
                Broadcast("terwin");
                m_iAccountTerrorist += 2750;
                goto T_WIN;
            }
        }
    }

    if (m_bMapHasEscapeZone == TRUE)
    {
        float escapeRatio = (float)m_iHaveEscaped / (float)m_iNumEscapers;

        if (m_flRequiredEscapeRatio <= escapeRatio)
        {
            EndRoundMessage("Terrorists have escaped!");
            Broadcast("terwin");
            goto T_WIN;
        }

        if (!NumAliveTerrorist)
        {
            if (m_flRequiredEscapeRatio > escapeRatio)
            {
                EndRoundMessage("CTs have stopped most of the Ts from escaping!");
                Broadcast("ctwin");
                m_iAccountCT += (int)((1.0f - escapeRatio) * 3500.0f);
                goto CT_WIN;
            }

            if (NumDeadTerrorist && m_iNumSpawnableCT > 0)
            {
                EndRoundMessage("Escaping terrorists have been neutralized!");
                Broadcast("ctwin");
                m_iAccountCT += (int)((1.0f - escapeRatio) * 3500.0f);
                goto CT_WIN;
            }
        }
    }

    if (m_bTargetBombed && m_bMapHasBombTarget)
    {
        EndRoundMessage("Target successfully bombed!");
        Broadcast("terwin");
        m_iAccountTerrorist += 2750;
        goto T_WIN;
    }

    if (m_bBombDefused && m_bMapHasBombTarget)
    {
        EndRoundMessage("Bomb successfully defused!");
        Broadcast("ctwin");
        m_iAccountCT += 2750;
        goto CT_WIN;
    }

    if (NumAliveTerrorist == 0 && NumDeadTerrorist > 0 && m_iNumSpawnableCT > 0)
    {

        ALERT(at_console, "The CTs win by elimination!!...\n");
        int bBombPresent = 0;

        CBaseEntity* pGrenade = UTIL_FindEntityByClassname(NULL, "grenade");
        if (pGrenade && !((CGrenade*)pGrenade)->m_bJustBlew)
        {
            ALERT(at_console, "... But there's a C4 bomb present.\n");
            bBombPresent = 1;
        }

        if (!bBombPresent)
        {
            EndRoundMessage("Counter-Terrorists WIN");
            Broadcast("ctwin");
            if (m_bMapHasBombTarget)
                m_iAccountCT += 2500;
            else
                m_iAccountCT += 2000;
            goto CT_WIN;
        }
    }
    else
    {
        if (!NumAliveCT)
        {
            if (NumDeadCT && m_iNumSpawnableTerrorist > 0)
            {
                ALERT(at_console, "Terrorists win by virtue of elimination!\n");
                EndRoundMessage("Terrorists WIN");
                Broadcast("terwin");
                if (m_bMapHasBombTarget)
                    m_iAccountTerrorist += 2500;
                else
                    m_iAccountTerrorist += 2000;
                goto T_WIN;
            }

            if (!NumAliveTerrorist)
            {
                EndRoundMessage("Round DRAW");
                Broadcast("rounddraw");
                ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 3;
                ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
                ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
                return;
            }
        }

        {
            BOOL anyAlive = FALSE;
            int numHostages = 0;
            CBaseEntity* pHostage = NULL;

            while ((pHostage = UTIL_FindEntityByClassname(pHostage, "hostage_entity")) != NULL)
            {
                numHostages++;
                if (pHostage->pev->takedamage >= 1.0f)
                    anyAlive = TRUE;
            }

            if (!anyAlive && numHostages > 0 && 0.5f * (float)numHostages <= (float)m_iHostagesRescued)
            {
                EndRoundMessage("All hostages rescued!\nCounter-Terrorists WIN");
                Broadcast("ctwin");
                m_iAccountCT += 2000;
                goto CT_WIN;
            }
        }
    }

    return;

T_WIN:
    if (!bNeededPlayers)
    {
        m_iNumTerroristWins++;
        MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
        WRITE_BYTE(1);
        WRITE_SHORT(m_iNumTerroristWins);
        MESSAGE_END();
    }
    ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 2;
    ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
    ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
    return;

CT_WIN:
    if (!bNeededPlayers)
    {
        m_iNumCTWins++;
        MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
        WRITE_BYTE(2);
        WRITE_SHORT(m_iNumCTWins);
        MESSAGE_END();
    }
    ((CHalfLifeMultiplay*)g_pGameRules)->m_iRoundWinStatus = 1;
    ((CHalfLifeMultiplay*)g_pGameRules)->m_bRoundTerminating = true;
    ((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime = gpGlobals->time + 5.0f;
    return;
}

void CHalfLifeMultiplay::RestartRound(void)
{
    CVAR_SET_FLOAT("sv_accelerate", 5.0f);
    CVAR_SET_FLOAT("sv_friction", 4.0f);
    CVAR_SET_FLOAT("sv_stopspeed", 75.0f);

    m_iNumCT = CountTeamPlayers(TEAM_CT);
    m_iNumTerrorist = CountTeamPlayers(TEAM_TERRORIST);

    if (CVAR_GET_FLOAT("mp_autoteambalance") != 0 && m_iUnBalancedRounds > 0)
        BalanceTeams();

    if (m_iNumCT - m_iNumTerrorist <= 1 && m_iNumTerrorist - m_iNumCT <= 1)
        m_iUnBalancedRounds = 0;
    else
        m_iUnBalancedRounds++;

    if (CVAR_GET_FLOAT("mp_autoteambalance") != 0 && m_iUnBalancedRounds == 1)
        UTIL_ClientPrintAll(HUD_PRINTCENTER, "Teams will be automatically balanced next round...\n");

    if (m_bCompleteReset)
    {
        m_iNumTerroristWins = 0;
        m_iNumCTWins = 0;
        m_iNumConsecutiveTerroristLoses = 0;
        m_iNumConsecutiveCTLoses = 0;

        MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
        WRITE_BYTE(1);
        WRITE_SHORT(0);
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
        WRITE_BYTE(2);
        WRITE_SHORT(0);
        MESSAGE_END();

        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);
            if (pPlayer)
            {
                if (FNullEnt(pPlayer->edict()))
                    continue;
                pPlayer->Reset();
            }
        }
    }

    m_bFreezePeriod = TRUE;
    m_bRoundTerminating = false;

    ReadMultiplayCvars(this);

    m_fMaxRoundTime = (float)(120 * m_iRoundTime);
    m_iRoundTimeSecs = m_iIntroRoundTime;

    CMapInfo* pMapInfo = (CMapInfo*)UTIL_FindEntityByClassname(NULL, "info_map_parameters");
    if (pMapInfo)
    {
        ALERT(at_console, "*****Checking Mi*****\n");

        switch (pMapInfo->m_iBuyingStatus)
        {
        case 0:
            m_bCTCantBuy = false;
            m_bTCantBuy = false;
            ALERT(at_console, "EVERYONE CAN BUY!\n");
            break;
        case 1:
            m_bCTCantBuy = false;
            m_bTCantBuy = true;
            ALERT(at_console, "Only CT's can buy!!\n");
            break;
        case 2:
            m_bCTCantBuy = true;
            m_bTCantBuy = false;
            ALERT(at_console, "Only T's can buy!!\n");
            break;
        case 3:
            m_bCTCantBuy = true;
            m_bTCantBuy = true;
            ALERT(at_console, "No one can buy!!\n");
            break;
        default:
            m_bCTCantBuy = false;
            m_bTCantBuy = false;
            break;
        }

        m_flBombRadius = pMapInfo->m_flBombRadius;
        ALERT(at_console, "Bomb Radius is : %f\n", m_flBombRadius);
    }

    if (UTIL_FindEntityByClassname(NULL, "func_bomb_target"))
    {
        m_bMapHasBombTarget = true;
        m_bMapHasBombZone = true;
    }
    else
    {
        if (UTIL_FindEntityByClassname(NULL, "info_bomb_target"))
            m_bMapHasBombTarget = true;
        else
            m_bMapHasBombTarget = false;
        m_bMapHasBombZone = false;
    }

    if (UTIL_FindEntityByClassname(NULL, "func_hostage_rescue"))
        m_bMapHasRescueZone = true;
    else
        m_bMapHasRescueZone = false;

    if (UTIL_FindEntityByClassname(NULL, "func_buyzone"))
        m_bMapHasBuyZone = true;
    else
        m_bMapHasBuyZone = false;

    if (UTIL_FindEntityByClassname(NULL, "func_escapezone"))
    {
        m_bMapHasEscapeZone = true;
        m_iHaveEscaped = 0;
        m_iNumEscapers = 0;

        if (m_iNumEscapeRounds > 7)
        {
            SwapAllPlayers();
            m_iNumEscapeRounds = 0;
        }
        m_iNumEscapeRounds++;
    }
    else
    {
        m_bMapHasEscapeZone = false;
    }

    if (UTIL_FindEntityByClassname(NULL, "func_vip_safetyzone"))
    {
        PickNextVIP();
        m_iConsecutiveVIP++;
        m_bMapHasVIPSafetyZone = 1;
    }
    else
    {
        m_bMapHasVIPSafetyZone = 2;
    }

    int hostageBonus = 0;
    CBaseEntity* pHostage = NULL;
    while ((pHostage = UTIL_FindEntityByClassname(pHostage, "hostage_entity")) != NULL)
    {
        if (hostageBonus > 1999)
            break;

        if (pHostage->pev->solid != SOLID_NOT)
        {
            hostageBonus += 150;
            if (pHostage->pev->deadflag == DEAD_DEAD)
                pHostage->pev->deadflag = DEAD_RESPAWNABLE;
        }
        ((CHostage*)pHostage)->RePosition();
    }

    if (m_iRoundWinStatus == 2)
    {
        if (m_iNumConsecutiveTerroristLoses > 2)
            m_iLoserBonus = 1400;
        m_iNumConsecutiveTerroristLoses = 0;
        m_iNumConsecutiveCTLoses++;
    }
    else if (m_iRoundWinStatus == 1)
    {
        if (m_iNumConsecutiveCTLoses > 2)
            m_iLoserBonus = 1400;
        m_iNumConsecutiveCTLoses = 0;
        m_iNumConsecutiveTerroristLoses++;
    }

    if (m_iNumConsecutiveTerroristLoses > 2 && m_iLoserBonus <= 2899)
        m_iLoserBonus += 500;
    else if (m_iNumConsecutiveCTLoses > 2 && m_iLoserBonus <= 2899)
        m_iLoserBonus += 500;

    if (m_iRoundWinStatus == 2)
    {
        m_iAccountTerrorist += hostageBonus;
        m_iAccountCT += m_iLoserBonus;
    }
    else if (m_iRoundWinStatus == 1)
    {
        m_iAccountCT += hostageBonus;
        if (!m_bMapHasEscapeZone)
            m_iAccountTerrorist += m_iLoserBonus;
    }

    m_iAccountCT += 250 * m_iHostagesRescued;

    m_fRoundStartTime = gpGlobals->time;
    m_fRoundStartTimeReal = gpGlobals->time;

    PickNewCommander(TEAM_TERRORIST);
    PickNewCommander(TEAM_CT);

    m_iRoundState = 2;

    CBaseEntity* pEnt = NULL;
    while ((pEnt = UTIL_FindEntityByClassname(pEnt, "player")) != NULL)
    {
        if (FNullEnt(pEnt->edict()))
            break;

        if (pEnt->pev->flags == FL_DORMANT)
            continue;

        CBasePlayer* pl = GetClassPtr((CBasePlayer*)pEnt->pev);

        if (pl->m_iTeam)
        {
            if (pl->m_bHasC4)
                pl->DropPlayerItem("weapon_c4");
            pl->RoundRespawn();

            if (m_bCompleteReset)
                pl->GiveDefaultItems();
        }

        if (pl->m_iTeam == TEAM_CT)
        {
            pl->AddAccount(m_iAccountCT, 1);
        }
        else if (pl->m_iTeam == TEAM_TERRORIST)
        {
            m_iNumEscapers++;
            pl->AddAccount(m_iAccountTerrorist, 1);

            if (m_bMapHasEscapeZone)
                pl->GiveDefaultItems();
        }
    }

    CleanUpMap();

    if (m_bMapHasBombTarget)
        GiveC4();

    m_iAccountCT = 0;
    m_iAccountTerrorist = 0;
    m_iHostagesRescued = 0;
    m_iHostagesTouched = 0;
    m_iRoundWinStatus = 0;
    m_bBombDefused = false;
    m_bTargetBombed = false;
    m_bLevelInitialized = false;
    m_bCompleteReset = false;
    m_flIntermissionEndTime = 0;
    m_flRestartRoundTime = 0;
}

void CHalfLifeMultiplay::CleanUpMap(void)
{
    CBaseEntity* pEntity;

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "light")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_breakable")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_door")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_water")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_door_rotating")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_tracktrain")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "func_train")) != NULL)
        pEntity->Restart();

    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "armoury_entity")) != NULL)
        pEntity->Restart();

    CBaseEntity* pGrenade = UTIL_FindEntityByClassname(NULL, "grenade");
    if (pGrenade)
        UTIL_Remove(pGrenade);

    int kitCount = 4;
    pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "item_thighpack")) != NULL)
    {
        if (kitCount <= 0)
            break;
        UTIL_Remove(pEntity);
        kitCount--;
    }

    RemoveGuns();
}

void CHalfLifeMultiplay::GiveC4(void)
{
    int iTeamCount;
    int iTemp = 0;

    iTeamCount = m_iNumTerrorist;
    m_iC4Guy++;

    if (m_iC4Guy > iTeamCount)
        m_iC4Guy = 1;

    CBaseEntity* pEntity = UTIL_FindEntityByClassname(NULL, "player");

    while (pEntity)
    {
        if (FNullEnt(pEntity->edict()))
            break;

        if (pEntity->IsPlayer())
        {
            if (pEntity->pev->flags != FL_DORMANT)
            {
                CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

                if (!pPlayer->pev->deadflag
                    && pPlayer->m_iTeam == TEAM_TERRORIST
                    && ++iTemp == m_iC4Guy)
                {
                    pPlayer->m_bHasC4 = TRUE;
                    pPlayer->GiveNamedItem("weapon_c4");
                    pPlayer->SetBombIcon(FALSE);
                    pPlayer->pev->body = 1;
                    m_pC4Carrier = pPlayer;
                }
            }
        }

        pEntity = UTIL_FindEntityByClassname(pEntity, "player");
    }
}

BOOL CHalfLifeMultiplay::GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{
    int iBestWeight = -1;
    CBasePlayerItem* pBest = NULL;

    if (!pCurrentWeapon->CanHolster())
        return FALSE;

    for (int i = 0; i <= 5; i++)
    {
        CBasePlayerItem* pItem = pPlayer->m_rgpPlayerItems[i];

        if (pItem)
        {
            do
            {
                int iItemWeight = pItem->iWeight();

                if (iItemWeight <= -1 || iItemWeight != pCurrentWeapon->iWeight() || pItem == pCurrentWeapon)
                {

                    if (pItem->iWeight() > iBestWeight
                        && pItem != pCurrentWeapon
                        && pItem->CanDeploy())
                    {
                        iBestWeight = pItem->iWeight();
                        pBest = pItem;
                    }
                }
                else
                {

                    if (pItem->CanDeploy() && pPlayer->SwitchWeapon(pItem))
                        return TRUE;
                }

                pItem = pItem->m_pNext;
            } while (pItem);
        }
    }

    if (!pBest)
        return FALSE;

    pPlayer->SwitchWeapon(pBest);
    return TRUE;
}

void CHalfLifeMultiplay::InitHUD(CBasePlayer* pl)
{
    UTIL_LogPrintf("\"%s<%i>\" has entered the game\n",
        STRING(pl->pev->netname), GETPLAYERUSERID(pl->edict()));

    UpdateGameMode(pl);

    MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
    WRITE_BYTE(ENTINDEX(pl->edict()));
    WRITE_SHORT(0);
    WRITE_SHORT(0);
    MESSAGE_END();

    SendMOTDToClient(pl->edict());

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);
        if (plr)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
            WRITE_BYTE(i);
            WRITE_SHORT((int)plr->pev->frags);
            WRITE_SHORT(plr->m_iDeaths);
            MESSAGE_END();
        }
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
    WRITE_BYTE(1);
    WRITE_SHORT(m_iNumTerroristWins);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
    WRITE_BYTE(2);
    WRITE_SHORT(m_iNumCTWins);
    MESSAGE_END();

    if (g_fGameOver)
    {
        MESSAGE_BEGIN(MSG_ONE, SVC_INTERMISSION, NULL, pl->edict());
        MESSAGE_END();
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pl->edict());
    WRITE_BYTE(0);
    WRITE_STRING("defuser");
    MESSAGE_END();

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);
        if (plr)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pl->edict());
            WRITE_BYTE(ENTINDEX(plr->edict()));
            WRITE_BYTE(plr->m_iTeam);
            MESSAGE_END();

            plr->SetScoreboardAttributes(pl);
        }
    }
}

BOOL CHalfLifeMultiplay::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
    return TRUE;
}

void CHalfLifeMultiplay::ClientDisconnected(edict_t* pClient)
{
    CLIENT_COMMAND(pClient, "lambert 1.5\n");

    if (pClient)
    {
        CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pClient);

        if (pPlayer)
        {
            if (pPlayer->m_bHasC4)
                pPlayer->DropPlayerItem("weapon_c4");

            if (pPlayer->m_bHasDefuser)
                pPlayer->GiveNamedItem("item_thighpack");

            if (pPlayer->m_bIsVIP)
                m_pVIP = NULL;

            FireTargets("game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0);

            UTIL_LogPrintf("\"%s<%i>\" disconnected\n",
                STRING(pPlayer->pev->netname),
                GETPLAYERUSERID(pPlayer->edict()));

            pPlayer->RemoveAllItems(TRUE);

            if (pPlayer->m_iTeam == TEAM_CT)
                PickNewCommander(TEAM_CT);

            if (pPlayer->m_pObserver)
                pPlayer->m_pObserver->SUB_Remove();
        }
    }

    CheckWinConditions();
}

void CHalfLifeMultiplay::UpdateGameMode(CBasePlayer* pPlayer)
{
    extern int gmsgGameMode;
    MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
    WRITE_BYTE(0);
    MESSAGE_END();
}

edict_t* CHalfLifeMultiplay::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
    edict_t* pentSpawnSpot = CGameRules::GetPlayerSpawnSpot(pPlayer);

    if (IsMultiplayer())
    {
        if (pentSpawnSpot->v.target)
        {
            FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);
        }
    }

    return pentSpawnSpot;
}

void CHalfLifeMultiplay::PickNextVIP(void)
{
    if (!IsVIPQueueEmpty())
    {
        if (m_pVIP)
            ResetCurrentVIP();

        for (int i = 0; i < MAX_VIP_QUEUES; i++)
        {
            if (m_pVIPQueue[i])
            {
                m_pVIP = m_pVIPQueue[i];
                m_pVIP->MakeVIP();
                m_pVIPQueue[i] = NULL;
                StackVIPQueue();
                m_iConsecutiveVIP = 0;
                return;
            }
        }
    }
    else
    {
        if (m_iConsecutiveVIP <= 2)
        {
            if (!m_pVIP)
            {
                CBaseEntity* pPlayer = NULL;
                while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
                {
                    if (FNullEnt(pPlayer->edict()))
                        break;
                    if (pPlayer->pev->flags == FL_DORMANT)
                        continue;

                    CBasePlayer* pl = GetClassPtr((CBasePlayer*)pPlayer->pev);
                    if (pl->m_iTeam == TEAM_CT)
                    {
                        pl->MakeVIP();
                        m_iConsecutiveVIP = 0;
                        return;
                    }
                }
            }
        }
        else
        {
            int pick = RANDOM_LONG(1, m_iNumCT);

            if (m_pVIP && pick == ENTINDEX(m_pVIP->edict()))
                pick++;
            if (pick > m_iNumCT)
                pick = 1;

            int count = 1;
            CBaseEntity* pPlayer = NULL;
            while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
            {
                if (FNullEnt(pPlayer->edict()))
                    break;
                if (pPlayer->pev->flags == FL_DORMANT)
                    continue;

                CBasePlayer* pl = GetClassPtr((CBasePlayer*)pPlayer->pev);
                if (pl->m_iTeam == TEAM_CT)
                {
                    if (count == pick)
                    {
                        if (m_pVIP)
                            ResetCurrentVIP();
                        pl->MakeVIP();
                        m_iConsecutiveVIP = 0;
                        return;
                    }
                    count++;
                }
            }
        }
    }
}

bool CHalfLifeMultiplay::AddToVIPQueue(CBasePlayer* toAdd)
{
    IsVIPQueueEmpty();

    if (toAdd->m_iTeam != TEAM_CT)
        return false;

    for (int i = 0; i < MAX_VIP_QUEUES; i++)
    {
        if (m_pVIPQueue[i] == toAdd)
        {
            char buf[256];
            sprintf(buf, "You are already in position %i of 5\n", i + 1);
            ClientPrint(toAdd->pev, HUD_PRINTCENTER, buf);
            return false;
        }
    }

    for (int i = 0; i < MAX_VIP_QUEUES; i++)
    {
        if (!m_pVIPQueue[i])
        {
            m_pVIPQueue[i] = toAdd;
            StackVIPQueue();
            char buf[256];
            sprintf(buf, "You have been added to position %i of 5\n", i + 1);
            ClientPrint(toAdd->pev, HUD_PRINTCENTER, buf);
            return true;
        }
    }

    ClientPrint(toAdd->pev, HUD_PRINTCENTER, "All 5 slots of the queue have been filled up!\n");
    return false;
}

void CHalfLifeMultiplay::ResetCurrentVIP(void)
{
    switch (RANDOM_LONG(0, 3))
    {
    case 0:
    case 2:
        m_pVIP->m_iModelName = 1;
        CLIENT_COMMAND(m_pVIP->edict(), "model urban\n");
        SET_CLIENT_KEY_VALUE(ENTINDEX(m_pVIP->edict()), g_engfuncs.pfnGetInfoKeyBuffer(m_pVIP->edict()), "model", "urban");
        break;
    case 1:
        m_pVIP->m_iModelName = 5;
        CLIENT_COMMAND(m_pVIP->edict(), "model gsg9\n");
        SET_CLIENT_KEY_VALUE(ENTINDEX(m_pVIP->edict()), g_engfuncs.pfnGetInfoKeyBuffer(m_pVIP->edict()), "model", "gsg9");
        break;
    case 3:
        m_pVIP->m_iModelName = 6;
        CLIENT_COMMAND(m_pVIP->edict(), "model gign\n");
        SET_CLIENT_KEY_VALUE(ENTINDEX(m_pVIP->edict()), g_engfuncs.pfnGetInfoKeyBuffer(m_pVIP->edict()), "model", "gign");
        break;
    }

    m_pVIP->m_bIsVIP = false;
    m_pVIP->m_bNotKilled = false;
}

void CHalfLifeMultiplay::StackVIPQueue(void)
{
    for (int i = MAX_VIP_QUEUES - 2; i > 0; i--)
    {
        if (m_pVIPQueue[i - 1])
        {
            if (!m_pVIPQueue[i])
            {
                m_pVIPQueue[i] = m_pVIPQueue[i + 1];
                m_pVIPQueue[i + 1] = NULL;
            }
        }
        else
        {
            m_pVIPQueue[i - 1] = m_pVIPQueue[i];
            m_pVIPQueue[i] = m_pVIPQueue[i + 1];
            m_pVIPQueue[i + 1] = NULL;
        }
    }
}

bool CHalfLifeMultiplay::IsVIPQueueEmpty(void)
{
    for (int i = 0; i < MAX_VIP_QUEUES; i++)
    {
        if (m_pVIPQueue[i] && m_pVIPQueue[i]->m_iTeam == TEAM_TERRORIST)
            m_pVIPQueue[i] = NULL;
    }

    StackVIPQueue();

    for (int i = 0; i < MAX_VIP_QUEUES; i++)
    {
        if (m_pVIPQueue[i])
            return false;
    }
    return true;
}

void CHalfLifeMultiplay::BalanceTeams(void)
{
    int iTeamToSwap;
    int iDifference;

    if (m_iNumCT > m_iNumTerrorist)
    {
        iTeamToSwap = TEAM_CT;
        iDifference = m_iNumCT - m_iNumTerrorist;
    }
    else if (m_iNumTerrorist > m_iNumCT)
    {
        iTeamToSwap = TEAM_TERRORIST;
        iDifference = m_iNumTerrorist - m_iNumCT;
    }
    else
    {
        return;
    }

    int iNumToSwap = iDifference / 2;
    if (iNumToSwap > 4)
        iNumToSwap = 4;

    for (int i = 1; i <= iNumToSwap; i++)
    {
        int iHighestFrags = -100;
        CBasePlayer* toSwap = NULL;

        CBaseEntity* pEntity = NULL;
        while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
        {
            if (FNullEnt(pEntity->edict()))
                break;
            if (pEntity->pev->flags == FL_DORMANT)
                continue;

            CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);
            if (pPlayer->m_iTeam == iTeamToSwap)
            {
                if (pPlayer->pev->frags >= iHighestFrags && m_pVIP != pPlayer)
                {
                    iHighestFrags = (int)pPlayer->pev->frags;
                    toSwap = pPlayer;
                }
            }
        }

        if (toSwap)
            toSwap->SwitchTeam();
    }
}

void CHalfLifeMultiplay::SwapAllPlayers(void)
{
    CBaseEntity* pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
    {
        if (FNullEnt(pEntity->edict()))
            break;
        if (pEntity->pev->flags == FL_DORMANT)
            continue;

        CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);
        pPlayer->SwitchTeam();
    }

    short iSavedCTWins = m_iNumCTWins;
    m_iNumCTWins = m_iNumTerroristWins;
    m_iNumTerroristWins = iSavedCTWins;

    MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
    WRITE_BYTE(TEAM_CT);
    WRITE_SHORT(m_iNumCTWins);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore, NULL);
    WRITE_BYTE(TEAM_TERRORIST);
    WRITE_SHORT(m_iNumTerroristWins);
    MESSAGE_END();
}

void CHalfLifeMultiplay::PickNewCommander(int iTeam)
{
    if (iTeam == TEAM_CT)
        m_pBomber = NULL;
    else
        m_pC4Carrier = NULL;

    int iCount = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
    {
        CBasePlayer* pTeamPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);
        if (pTeamPlayer->m_iTeam == iTeam && pTeamPlayer->pev->flags != FL_DORMANT)
            iCount++;
    }

    int iRandomPick = RANDOM_LONG(1, iCount);

    int iNum = 0;
    CBaseEntity* pPlayer = NULL;
    while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
    {
        if (FNullEnt(pPlayer->edict()))
            return;

        if (pPlayer->IsPlayer())
        {
            if (pPlayer->pev->flags != FL_DORMANT)
            {
                CBasePlayer* pClassPlayer = GetClassPtr((CBasePlayer*)pPlayer->pev);

                if (!pPlayer->pev->deadflag)
                {
                    if (iTeam == TEAM_TERRORIST)
                    {
                        if (pClassPlayer->m_iTeam == TEAM_TERRORIST)
                        {
                            iNum++;
                            m_pC4Carrier = pClassPlayer;
                            if (iNum >= iRandomPick)
                                return;
                        }
                    }
                    else if (iTeam == TEAM_CT && pClassPlayer->m_iTeam == TEAM_CT)
                    {
                        iNum++;
                        m_pBomber = pClassPlayer;
                        if (iNum >= iRandomPick)
                            return;
                    }
                }
            }
        }
    }
}

void CHalfLifeMultiplay::EndMultiplayerGame(void)
{
    GoToIntermission();
}

void CHalfLifeMultiplay::GoToIntermission(void)
{
    if (g_fGameOver)
        return;

    MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
    MESSAGE_END();

    m_flIntermissionEndTime = gpGlobals->time + 6.0f;
    g_fGameOver = TRUE;
    m_iEndIntermissionButtonHit = FALSE;
    m_iSpawnPointCount_Terrorist = 0;
    m_iSpawnPointCount_CT = 0;
    m_bLevelInitialized = false;
}

void CHalfLifeMultiplay::ChangeLevel(void)
{
    char szNextMap[32];
    char szFirstMapInList[32];
    char szBuffer[32];
    char* aFileList;
    const char* pFileList;
    int length;
    int bFoundCurrentMap;

    strcpy(szFirstMapInList, "hldm1");

    char* mapcfile = (char*)CVAR_GET_STRING("mapcyclefile");

    strcpy(szNextMap, STRING(gpGlobals->mapname));
    strcpy(szFirstMapInList, STRING(gpGlobals->mapname));

    aFileList = (char*)LOAD_FILE_FOR_ME(mapcfile, &length);
    pFileList = aFileList;

    if (aFileList && length)
    {

        sscanf(aFileList, " %32s", szNextMap);

        if (IS_MAP_VALID(szNextMap))
            strcpy(szFirstMapInList, szNextMap);

        bFoundCurrentMap = FALSE;

        while (*pFileList)
        {

            while (*pFileList && isspace(*pFileList))
                pFileList++;

            if (!*pFileList)
                break;

            if (sscanf(pFileList, " %32s", szBuffer) != 1 || szBuffer[0] <= 12)
                break;

            if (bFoundCurrentMap && IS_MAP_VALID(szBuffer))
            {
                strcpy(szNextMap, szBuffer);
                break;
            }

            if (!strcmp(szBuffer, STRING(gpGlobals->mapname)))
                bFoundCurrentMap = TRUE;

            pFileList += strlen(szBuffer);
        }

        FREE_FILE(aFileList);
    }

    if (!IS_MAP_VALID(szNextMap))
        strcpy(szNextMap, szFirstMapInList);

    g_fGameOver = TRUE;

    ALERT(at_console, "CHANGE LEVEL: %s\n", szNextMap);
    CHANGE_LEVEL(szNextMap, NULL);
}

void CHalfLifeMultiplay::SendMOTDToClient(edict_t* client)
{
    int char_count = 0;
    int length;
    char* aFileList = (char*)LOAD_FILE_FOR_ME("motd.txt", &length);
    char* pFileList = aFileList;

    while (pFileList && *pFileList)
    {
        if (char_count > MAX_MOTD_LENGTH - 1)
            break;

        char chunk[64];

        if (strlen(pFileList) > MAX_MOTD_CHUNK - 1)
        {
            strncpy(chunk, pFileList, MAX_MOTD_CHUNK);
            chunk[MAX_MOTD_CHUNK] = '\0';
        }
        else
        {
            strcpy(chunk, pFileList);
        }

        char_count += strlen(chunk);

        if (char_count > MAX_MOTD_LENGTH - 1)
            *pFileList = '\0';
        else
            pFileList = aFileList + char_count;

        MESSAGE_BEGIN(MSG_ONE, gmsgMOTD, NULL, client);
        WRITE_BYTE(*pFileList == '\0');
        WRITE_STRING(chunk);
        MESSAGE_END();
    }

    FREE_FILE(aFileList);
}

BOOL CHalfLifeMultiplay::IsAllowedToSpawn(CBaseEntity* pEntity)
{
    return TRUE;
}

