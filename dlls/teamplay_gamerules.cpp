#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"game.h"

static char team_names[MAX_TEAMS][MAX_TEAMNAME_LENGTH];
static int team_scores[MAX_TEAMS];
static int num_teams = 0;

extern DLL_GLOBAL BOOL		g_fGameOver;

CHalfLifeTeamplay::CHalfLifeTeamplay()
{
    m_DisableDeathMessages = FALSE;
    m_DisableDeathPenalty = FALSE;

    memset(team_names, 0, sizeof(team_names));
    memset(team_scores, 0, sizeof(team_scores));
    num_teams = 0;

    m_szTeamList[0] = 0;

    strncpy(m_szTeamList, teamlist.string, TEAMPLAY_TEAMLISTLENGTH);

    edict_t* pWorld = INDEXENT(0);
    if (pWorld && pWorld->v.team)
    {
        if (teamoverride.value)
        {

            const char* pTeamList = STRING(pWorld->v.team);
            if (pTeamList && strlen(pTeamList))
            {
                strncpy(m_szTeamList, pTeamList, TEAMPLAY_TEAMLISTLENGTH);
            }
        }
    }

    if (strlen(m_szTeamList))
        m_teamLimit = TRUE;
    else
        m_teamLimit = FALSE;

    RecountTeams();
}

void CHalfLifeTeamplay::Think(void)
{
    if (g_fGameOver)
    {
        CHalfLifeMultiplay::Think();
        return;
    }

    float flTimeLimit = CVAR_GET_FLOAT("mp_timelimit") * 60;

    if (flTimeLimit != 0 && gpGlobals->time >= flTimeLimit)
    {
        GoToIntermission();
        return;
    }

    float flFragLimit = fraglimit.value;
    if (flFragLimit)
    {
        for (int i = 0; i < num_teams; i++)
        {
            if (team_scores[i] >= flFragLimit)
            {
                GoToIntermission();
                return;
            }
        }
    }
}

BOOL CHalfLifeTeamplay::ClientCommand(CBasePlayer* pPlayer, const char* pcmd)
{
    if (FStrEq(pcmd, "menuselect"))
    {
        if (CMD_ARGC() < 2)
            return TRUE;

        int slot = atoi(CMD_ARGV(1));

        return TRUE;
    }

    return FALSE;
}

extern int gmsgGameMode;
extern int gmsgSayText;
extern int gmsgTeamInfo;

void CHalfLifeTeamplay::UpdateGameMode(CBasePlayer* pPlayer)
{
    MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
    WRITE_BYTE(1);
    MESSAGE_END();
}

const char* CHalfLifeTeamplay::SetDefaultPlayerTeam(CBasePlayer* pPlayer)
{
    char* mdls = g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model");
    strncpy(pPlayer->m_szTeamName, mdls, TEAM_NAME_LENGTH);

    RecountTeams();

    if (pPlayer->m_szTeamName[0] == '\0' || !IsValidTeam(pPlayer->m_szTeamName) || defaultteam.value)
    {
        const char* pTeamName = NULL;

        if (defaultteam.value)
        {
            pTeamName = team_names[0];
        }
        else
        {
            pTeamName = TeamWithFewestPlayers();
        }
        strncpy(pPlayer->m_szTeamName, pTeamName, TEAM_NAME_LENGTH);
    }

    return pPlayer->m_szTeamName;
}

void CHalfLifeTeamplay::InitHUD(CBasePlayer* pPlayer)
{
    SetDefaultPlayerTeam(pPlayer);
    CHalfLifeMultiplay::InitHUD(pPlayer);

    RecountTeams();

    char* mdls = g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model");

    char text[1024];
    if (!strcmp(mdls, pPlayer->m_szTeamName))
    {
        sprintf(text, "* you are on team \'%s\'\n", pPlayer->m_szTeamName);
    }
    else
    {
        sprintf(text, "* assigned to team %s\n", pPlayer->m_szTeamName);
    }

    ChangePlayerTeam(pPlayer, pPlayer->m_szTeamName, FALSE, FALSE);
    UTIL_SayText(text, pPlayer);
    int clientIndex = pPlayer->entindex();
    RecountTeams();

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBaseEntity* plr = UTIL_PlayerByIndex(i);
        if (plr && IsValidTeam(plr->TeamID()))
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pPlayer->edict());
            WRITE_BYTE(plr->entindex());
            WRITE_STRING(plr->TeamID());
            MESSAGE_END();
        }
    }
}

void CHalfLifeTeamplay::ChangePlayerTeam(CBasePlayer* pPlayer, const char* pTeamName, BOOL bKill, BOOL bGib)
{
    int damageFlags = DMG_GENERIC;
    int clientIndex = pPlayer->entindex();

    if (!bGib)
    {
        damageFlags |= DMG_NEVERGIB;
    }
    else
    {
        damageFlags |= DMG_ALWAYSGIB;
    }

    if (bKill)
    {

        m_DisableDeathMessages = TRUE;
        m_DisableDeathPenalty = TRUE;

        entvars_t* pevWorld = VARS(INDEXENT(0));
        pPlayer->TakeDamage(pevWorld, pevWorld, 900, damageFlags);

        m_DisableDeathMessages = FALSE;
        m_DisableDeathPenalty = FALSE;
    }

    strncpy(pPlayer->m_szTeamName, pTeamName, TEAM_NAME_LENGTH);

    g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
    g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "team", pPlayer->m_szTeamName);

    MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
    WRITE_BYTE(clientIndex);
    WRITE_STRING(pPlayer->m_szTeamName);
    MESSAGE_END();
}

void CHalfLifeTeamplay::ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer)
{
    char text[1024];
    char* mdls = g_engfuncs.pfnInfoKeyValue(infobuffer, "model");

    if (!stricmp(mdls, pPlayer->m_szTeamName))
        return;

    if (defaultteam.value)
    {
        int clientIndex = pPlayer->entindex();

        g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
        g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "team", pPlayer->m_szTeamName);
        sprintf(text, "* Not allowed to change teams in this game!\n");
        UTIL_SayText(text, pPlayer);
        return;
    }

    if (defaultteam.value || !IsValidTeam(mdls))
    {
        int clientIndex = pPlayer->entindex();

        g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
        sprintf(text, "* Can't change team to \'%s\'\n", mdls);
        UTIL_SayText(text, pPlayer);
        sprintf(text, "* Server limits teams to \'%s\'\n", m_szTeamList);
        UTIL_SayText(text, pPlayer);
        return;
    }

    sprintf(text, "* %s has changed to team \'%s\'\n", STRING(pPlayer->pev->netname), mdls);
    UTIL_SayTextAll(text, pPlayer);

    UTIL_LogPrintf("\"%s<%i>\" changed to team %s\n", STRING(pPlayer->pev->netname), GETPLAYERUSERID(pPlayer->edict()), mdls);

    ChangePlayerTeam(pPlayer, mdls, TRUE, TRUE);

    RecountTeams();
}

extern int gmsgDeathMsg;

void CHalfLifeTeamplay::DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pevInflictor)
{
    if (m_DisableDeathMessages)
        return;

    if (pVictim && pKiller && pKiller->flags & FL_CLIENT)
    {
        CBasePlayer* pk = (CBasePlayer*)CBaseEntity::Instance(pKiller);

        if (pk)
        {

            if ((pk != pVictim) && (PlayerRelationship(pVictim, pk) == GR_TEAMMATE))
            {
                MESSAGE_BEGIN(MSG_ALL, gmsgDeathMsg);
                WRITE_BYTE(ENTINDEX(ENT(pKiller)));
                WRITE_BYTE(ENTINDEX(pVictim->edict()));
                WRITE_STRING("teammate");
                MESSAGE_END();
                return;
            }
        }
    }

    CHalfLifeMultiplay::DeathNotice(pVictim, pKiller, pevInflictor);
}

void CHalfLifeTeamplay::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
    if (!m_DisableDeathPenalty)
    {
        CHalfLifeMultiplay::PlayerKilled(pVictim, pKiller, pInflictor);
        RecountTeams();
    }
}

BOOL CHalfLifeTeamplay::IsTeamplay(void)
{
    return TRUE;
}

BOOL CHalfLifeTeamplay::FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
    if (pAttacker && PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE)
    {
        if ((CVAR_GET_FLOAT("mp_friendlyfire") == 0) && (pAttacker != pPlayer))
        {
            return FALSE;
        }
    }

    return CHalfLifeMultiplay::FPlayerCanTakeDamage(pPlayer, pAttacker);
}

int CHalfLifeTeamplay::PlayerRelationship(CBasePlayer* pPlayer, CBaseEntity* pTarget)
{
    if (!pPlayer || !pTarget || !pTarget->IsPlayer())
        return GR_NOTTEAMMATE;

    if ((*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp(GetTeamID(pPlayer), GetTeamID(pTarget)))
    {
        return GR_TEAMMATE;
    }

    return GR_NOTTEAMMATE;
}

BOOL CHalfLifeTeamplay::ShouldAutoAim(CBasePlayer* pPlayer, edict_t* target)
{
    CBaseEntity* pTgt = CBaseEntity::Instance(target);
    if (pTgt && pTgt->IsPlayer())
    {
        if (PlayerRelationship(pPlayer, pTgt) == GR_TEAMMATE)
            return FALSE;
    }

    return CHalfLifeMultiplay::ShouldAutoAim(pPlayer, target);
}

int CHalfLifeTeamplay::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
    if (!pKilled)
        return 0;

    if (!pAttacker)
        return 1;

    if (pAttacker != pKilled && PlayerRelationship(pAttacker, pKilled) == GR_TEAMMATE)
        return -1;

    return 1;
}

const char* CHalfLifeTeamplay::GetTeamID(CBaseEntity* pEntity)
{
    if (pEntity == NULL || pEntity->pev == NULL)
        return "";

    return pEntity->TeamID();
}

int CHalfLifeTeamplay::GetTeamIndex(const char* pTeamName)
{
    if (pTeamName && *pTeamName != 0)
    {
        for (int tm = 0; tm < num_teams; tm++)
        {
            if (!stricmp(team_names[tm], pTeamName))
                return tm;
        }
    }

    return -1;
}

const char* CHalfLifeTeamplay::GetIndexedTeamName(int teamIndex)
{
    if (teamIndex < 0 || teamIndex >= num_teams)
        return "";

    return team_names[teamIndex];
}

BOOL CHalfLifeTeamplay::IsValidTeam(const char* pTeamName)
{
    if (!m_teamLimit)
        return TRUE;

    return (GetTeamIndex(pTeamName) != -1) ? TRUE : FALSE;
}

const char* CHalfLifeTeamplay::TeamWithFewestPlayers(void)
{
    int i;
    int minPlayers = MAX_TEAMS;
    int teamCount[MAX_TEAMS];
    char* pTeamName = NULL;

    memset(teamCount, 0, MAX_TEAMS * sizeof(int));

    for (i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBaseEntity* plr = UTIL_PlayerByIndex(i);

        if (plr)
        {
            int team = GetTeamIndex(plr->TeamID());
            if (team >= 0)
                teamCount[team] ++;
        }
    }

    for (i = 0; i < num_teams; i++)
    {
        if (teamCount[i] < minPlayers)
        {
            minPlayers = teamCount[i];
            pTeamName = team_names[i];
        }
    }

    return pTeamName;
}

void CHalfLifeTeamplay::RecountTeams(void)
{
    char* pName;
    char	teamlist[TEAMPLAY_TEAMLISTLENGTH];

    num_teams = 0;

    strcpy(teamlist, m_szTeamList);
    pName = teamlist;
    pName = strtok(pName, ";");
    while (pName != NULL && *pName)
    {
        if (GetTeamIndex(pName) < 0)
        {
            strcpy(team_names[num_teams], pName);
            num_teams++;
        }
        pName = strtok(NULL, ";");
    }

    if (num_teams < 2)
    {
        num_teams = 0;
        m_teamLimit = FALSE;
    }

    memset(team_scores, 0, sizeof(team_scores));

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBaseEntity* plr = UTIL_PlayerByIndex(i);

        if (plr)
        {
            const char* pTeamName = plr->TeamID();

            int tm = GetTeamIndex(pTeamName);

            if (tm < 0)
            {
                if (!m_teamLimit)
                {
                    tm = num_teams;
                    num_teams++;
                    team_scores[tm] = 0;
                    strncpy(team_names[tm], pTeamName, MAX_TEAMNAME_LENGTH);
                }
            }

            if (tm >= 0)
            {
                team_scores[tm] += plr->pev->frags;
            }
        }
    }
}

