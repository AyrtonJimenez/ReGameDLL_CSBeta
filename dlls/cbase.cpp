#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"saverestore.h"
#include	"client.h"
#include	"decals.h"
#include	"gamerules.h"
#include	"game.h"

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd);

extern Vector VecBModelOrigin(entvars_t* pevBModel);
extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL int			g_iSkillLevel;

static DLL_FUNCTIONS gFunctionTable =
{
    GameDLLInit,
    DispatchSpawn,
    DispatchThink,
    DispatchUse,
    DispatchTouch,
    DispatchBlocked,
    DispatchKeyValue,
    DispatchSave,
    DispatchRestore,
    DispatchObjectCollsionBox,

    SaveWriteFields,
    SaveReadFields,

    SaveGlobalState,
    RestoreGlobalState,
    ResetGlobalState,

    ClientConnect,
    ClientDisconnect,
    ClientKill,
    ClientPutInServer,
    ClientCommand,
    ClientUserInfoChanged,
    ServerActivate,

    PlayerPreThink,
    PlayerPostThink,

    StartFrame,
    ParmsNewLevel,
    ParmsChangeLevel,

    GetGameDescription,
    PlayerCustomization,

    SpectatorConnect,
    SpectatorDisconnect,
    SpectatorThink,
};

static void SetObjectCollisionBox(entvars_t* pev);

int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
{
    if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
        return FALSE;

    memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
    return TRUE;
}

int DispatchSpawn(edict_t* pent)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

    if (pEntity)
    {
        pEntity->pev->absmin = pEntity->pev->origin - Vector(1, 1, 1);
        pEntity->pev->absmax = pEntity->pev->origin + Vector(1, 1, 1);

        pEntity->Spawn();

        pEntity = (CBaseEntity*)GET_PRIVATE(pent);

        if (pEntity)
        {

            if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
                return -1;
            if (pEntity->pev->flags & FL_KILLME)
                return -1;
        }

        if (pEntity && pEntity->pev->globalname)
        {
            const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
            if (pGlobal)
            {
                if (pGlobal->state == GLOBAL_DEAD)
                    return -1;
                else if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
                    pEntity->MakeDormant();
            }
            else
            {
                gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
            }
        }
    }

    return 0;
}

void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd)
{
    if (!pkvd || !pentKeyvalue)
        return;

    EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

    if (pkvd->fHandled || pkvd->szClassName == NULL)
        return;

    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentKeyvalue);

    if (!pEntity)
        return;

    pEntity->KeyValue(pkvd);
}

BOOL gTouchDisabled = FALSE;
void DispatchTouch(edict_t* pentTouched, edict_t* pentOther)
{
    if (gTouchDisabled)
        return;

    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentTouched);
    CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

    if (pEntity && pOther && !((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME))
        pEntity->Touch(pOther);
}

void DispatchUse(edict_t* pentUsed, edict_t* pentOther)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentUsed);
    CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

    if (pEntity && !(pEntity->pev->flags & FL_KILLME))
        pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

void DispatchThink(edict_t* pent)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
    if (pEntity)
    {
        if (FBitSet(pEntity->pev->flags, FL_DORMANT))
            ALERT(at_error, "Dormant entity %s is thinking!!\n", STRING(pEntity->pev->classname));

        pEntity->Think();
    }
}

void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentBlocked);
    CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

    if (pEntity)
        pEntity->Blocked(pOther);
}

void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

    if (pEntity && pSaveData)
    {
        ENTITYTABLE* pTable = &pSaveData->pTable[pSaveData->currentIndex];

        if (pTable->pent != pent)
            ALERT(at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n");

        if (pEntity->ObjectCaps() & FCAP_DONT_SAVE)
            return;

        if (pEntity->pev->movetype == MOVETYPE_PUSH)
        {
            float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
            pEntity->pev->ltime = gpGlobals->time;
            pEntity->pev->nextthink = pEntity->pev->ltime + delta;
        }

        pTable->location = pSaveData->size;
        pTable->classname = pEntity->pev->classname;

        CSave saveHelper(pSaveData);
        pEntity->Save(saveHelper);

        pTable->size = pSaveData->size - pTable->location;
    }
}

CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname)
{
    edict_t* pent = FIND_ENTITY_BY_STRING(NULL, "globalname", STRING(globalname));
    CBaseEntity* pReturn = CBaseEntity::Instance(pent);
    if (pReturn)
    {
        if (!FClassnameIs(pReturn->pev, STRING(classname)))
        {
            ALERT(at_console, "Global entity found %s, wrong class %s\n", STRING(globalname), STRING(pReturn->pev->classname));
            pReturn = NULL;
        }
    }

    return pReturn;
}

int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

    if (pEntity && pSaveData)
    {
        entvars_t tmpVars;
        Vector oldOffset;

        CRestore restoreHelper(pSaveData);

        if (globalEntity)
        {
            CRestore tmpRestore(pSaveData);
            tmpRestore.PrecacheMode(0);
            tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

            pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
            pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;

            const globalentity_t* pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

            if (!FStrEq(pSaveData->szCurrentMapName, pGlobal->levelName))
                return 0;

            oldOffset = pSaveData->vecLandmarkOffset;
            CBaseEntity* pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);
            if (pNewEntity)
            {

                restoreHelper.SetGlobalMode(1);
                pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
                pEntity = pNewEntity;
                pent = ENT(pEntity->pev);

                gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
            }
            else
            {
                return 0;
            }

        }

        if (pEntity->ObjectCaps() & FCAP_MUST_SPAWN)
        {
            pEntity->Restore(restoreHelper);
            pEntity->Spawn();
        }
        else
        {
            pEntity->Restore(restoreHelper);
            pEntity->Precache();
        }

        pEntity = (CBaseEntity*)GET_PRIVATE(pent);

#if 0
        if (pEntity && pEntity->pev->globalname && globalEntity)
        {
            ALERT(at_console, "Global %s is %s\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model));
        }
#endif

        if (globalEntity)
        {
            pSaveData->vecLandmarkOffset = oldOffset;
            if (pEntity)
            {
                UTIL_SetOrigin(pEntity->pev, pEntity->pev->origin);
                pEntity->OverrideReset();
            }
        }
        else if (pEntity && pEntity->pev->globalname)
        {
            const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
            if (pGlobal)
            {
                if (pGlobal->state == GLOBAL_DEAD)
                    return -1;
                else if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
                {
                    pEntity->MakeDormant();
                }

            }
            else
            {
                ALERT(at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname));

                gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
            }
        }
    }
    return 0;
}

void DispatchObjectCollsionBox(edict_t* pent)
{
    CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
    if (pEntity)
    {
        pEntity->SetObjectCollisionBox();
    }
    else
    {
        SetObjectCollisionBox(&pent->v);
    }
}

void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
    CSave saveHelper(pSaveData);
    saveHelper.WriteFields(pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
    CRestore restoreHelper(pSaveData);
    restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

edict_t* EHANDLE::Get(void)
{
    if (m_pent)
    {
        if (m_pent->serialnumber == m_serialnumber)
            return m_pent;
        else
            return NULL;
    }
    return NULL;
}

edict_t* EHANDLE::Set(edict_t* pent)
{
    m_pent = pent;
    if (pent)
        m_serialnumber = m_pent->serialnumber;
    return pent;
}

EHANDLE::operator CBaseEntity*()
{
    return (CBaseEntity*)GET_PRIVATE(Get());
}

CBaseEntity* EHANDLE::operator=(CBaseEntity* pEntity)
{
    if (pEntity)
    {
        m_pent = ENT(pEntity->pev);
        if (m_pent)
            m_serialnumber = m_pent->serialnumber;
    }
    else
    {
        m_pent = NULL;
        m_serialnumber = 0;
    }
    return pEntity;
}

EHANDLE::operator int()
{
    return Get() != NULL;
}

CBaseEntity* EHANDLE::operator->()
{
    return (CBaseEntity*)GET_PRIVATE(Get());
}

int CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
    if (!pev->takedamage)
        return 0;

    if (pev->health >= pev->max_health)
        return 0;

    pev->health += flHealth;

    if (pev->health > pev->max_health)
        pev->health = pev->max_health;

    return 1;
}

int CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    Vector			vecTemp;

    if (!pev->takedamage)
        return 0;

    if (pevAttacker == pevInflictor)
    {
        vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
    }
    else
    {
        vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
    }

    g_vecAttackDir = vecTemp.Normalize();

    if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER))
    {
        Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
        vecDir = vecDir.Normalize();

        float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

        if (flForce > 1000.0)
            flForce = 1000.0;
        pev->velocity = pev->velocity + vecDir * flForce;
    }

    pev->health -= flDamage;
    if (pev->health <= 0)
    {
        Killed(pevAttacker, GIB_NORMAL);
        return 0;
    }

    return 1;
}

void CBaseEntity::Killed(entvars_t* pevAttacker, int iGib)
{
    pev->takedamage = DAMAGE_NO;
    pev->deadflag = DEAD_DEAD;
    UTIL_Remove(this);
}

CBaseEntity* CBaseEntity::GetNextTarget(void)
{
    if (FStringNull(pev->target))
        return NULL;
    edict_t* pTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));
    if (FNullEnt(pTarget))
        return NULL;

    return Instance(pTarget);
}

TYPEDESCRIPTION	CBaseEntity::m_SaveData[] =
{
    DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),

    DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION),
    DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
    DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
    DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};

int CBaseEntity::Save(CSave& save)
{
    if (save.WriteEntVars("ENTVARS", pev))
        return save.WriteFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

    return 0;
}

int CBaseEntity::Restore(CRestore& restore)
{
    int status;

    status = restore.ReadEntVars("ENTVARS", pev);
    if (status)
        status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

    if (pev->modelindex != 0 && !FStringNull(pev->model))
    {
        Vector mins, maxs;
        mins = pev->mins;
        maxs = pev->maxs;

        PRECACHE_MODEL((char*)STRING(pev->model));
        SET_MODEL(ENT(pev), STRING(pev->model));
        UTIL_SetSize(pev, mins, maxs);
    }

    return status;
}

void SetObjectCollisionBox(entvars_t* pev)
{
    if ((pev->solid == SOLID_BSP) &&
        (pev->angles.x || pev->angles.y || pev->angles.z))
    {
        float		max, v;
        int			i;

        max = 0;
        for (i = 0; i < 3; i++)
        {
            v = fabs(pev->mins[i]);
            if (v > max)
                max = v;
            v = fabs(pev->maxs[i]);
            if (v > max)
                max = v;
        }
        for (i = 0; i < 3; i++)
        {
            pev->absmin[i] = pev->origin[i] - max;
            pev->absmax[i] = pev->origin[i] + max;
        }
    }
    else
    {

        pev->absmin = pev->origin + pev->mins;
        pev->absmax = pev->origin + pev->maxs;
    }

    pev->absmin.x -= 1;
    pev->absmin.y -= 1;
    pev->absmin.z -= 1;
    pev->absmax.x += 1;
    pev->absmax.y += 1;
    pev->absmax.z += 1;
}

void CBaseEntity::SetObjectCollisionBox(void)
{
    ::SetObjectCollisionBox(pev);
}

int	CBaseEntity::Intersects(CBaseEntity* pOther)
{
    if (pOther->pev->absmin.x > pev->absmax.x ||
        pOther->pev->absmin.y > pev->absmax.y ||
        pOther->pev->absmin.z > pev->absmax.z ||
        pOther->pev->absmax.x < pev->absmin.x ||
        pOther->pev->absmax.y < pev->absmin.y ||
        pOther->pev->absmax.z < pev->absmin.z)
    {
        return 0;
    }

    return 1;
}

void CBaseEntity::MakeDormant(void)
{
    SetBits(pev->flags, FL_DORMANT);

    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NONE;
    SetBits(pev->effects, EF_NODRAW);
    pev->nextthink = 0;
    UTIL_SetOrigin(pev, pev->origin);
}

int CBaseEntity::IsDormant(void)
{
    return FBitSet(pev->flags, FL_DORMANT);
}

BOOL CBaseEntity::IsInWorld(void)
{
    if (pev->origin.x >= 4096) return FALSE;
    if (pev->origin.y >= 4096) return FALSE;
    if (pev->origin.z >= 4096) return FALSE;
    if (pev->origin.x <= -4096) return FALSE;
    if (pev->origin.y <= -4096) return FALSE;
    if (pev->origin.z <= -4096) return FALSE;

    if (pev->velocity.x >= 2000) return FALSE;
    if (pev->velocity.y >= 2000) return FALSE;
    if (pev->velocity.z >= 2000) return FALSE;
    if (pev->velocity.x <= -2000) return FALSE;
    if (pev->velocity.y <= -2000) return FALSE;
    if (pev->velocity.z <= -2000) return FALSE;

    return TRUE;
}

int CBaseEntity::ShouldToggle(USE_TYPE useType, BOOL currentState)
{
    if (useType != USE_TOGGLE && useType != USE_SET)
    {
        if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
            return 0;
    }
    return 1;
}

int	CBaseEntity::DamageDecal(int bitsDamageType)
{
    if (pev->rendermode == kRenderTransAlpha)
        return -1;

    if (pev->rendermode != kRenderNormal)
        return DECAL_BPROOF1;

    return RANDOM_LONG(DECAL_GUNSHOT4, DECAL_GUNSHOT5);
}

CBaseEntity* CBaseEntity::Create(char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
{
    edict_t* pent;
    CBaseEntity* pEntity;

    pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));
    if (FNullEnt(pent))
    {
        ALERT(at_console, "NULL Ent in Create!\n");
        return NULL;
    }
    pEntity = Instance(pent);
    pEntity->pev->owner = pentOwner;
    pEntity->pev->origin = vecOrigin;
    pEntity->pev->angles = vecAngles;

    DispatchSpawn(pEntity->edict());
    return pEntity;
}

