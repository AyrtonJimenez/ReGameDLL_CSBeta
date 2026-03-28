#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "func_break.h"
#include "shake.h"
#include "player.h"

extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL int			g_iSkillLevel;

extern Vector VecBModelOrigin(entvars_t* pevBModel);
extern entvars_t* g_pevLastInflictor;

#define GERMAN_GIB_COUNT		4
#define	HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4

void CGib::LimitVelocity(void)
{
    float length = pev->velocity.Length();

    if (length > 1500.0)
        pev->velocity = pev->velocity.Normalize() * 1500;
}

void CGib::SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs)
{
    int i;

    if (g_Language == LANGUAGE_GERMAN)
    {
        return;
    }

    for (i = 0; i < cGibs; i++)
    {
        CGib* pGib = GetClassPtr((CGib*)NULL);

        pGib->Spawn("models/stickygib.mdl");
        pGib->pev->body = RANDOM_LONG(0, 2);

        if (pevVictim)
        {

            pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT(-3, 3);
            pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT(-3, 3);
            pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT(-3, 3);

            pGib->pev->velocity = g_vecAttackDir * -1;

            pGib->pev->velocity.x += RANDOM_FLOAT(-0.15, 0.15);
            pGib->pev->velocity.y += RANDOM_FLOAT(-0.15, 0.15);
            pGib->pev->velocity.z += RANDOM_FLOAT(-0.15, 0.15);

            pGib->pev->velocity = pGib->pev->velocity * 900;

            pGib->pev->avelocity.x = RANDOM_FLOAT(250, 400);
            pGib->pev->avelocity.y = RANDOM_FLOAT(250, 400);

            pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

            if (pevVictim->health > -50)
            {
                pGib->pev->velocity = pGib->pev->velocity * 0.7;
            }
            else if (pevVictim->health > -200)
            {
                pGib->pev->velocity = pGib->pev->velocity * 2;
            }
            else
            {
                pGib->pev->velocity = pGib->pev->velocity * 4;
            }

            pGib->pev->movetype = MOVETYPE_TOSS;
            pGib->pev->solid = SOLID_BBOX;
            UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
            pGib->SetTouch(&CGib::StickyGibTouch);
            pGib->SetThink(NULL);
        }
        pGib->LimitVelocity();
    }
}

void CGib::SpawnHeadGib(entvars_t* pevVictim)
{
    CGib* pGib = GetClassPtr((CGib*)NULL);

    if (g_Language == LANGUAGE_GERMAN)
    {
        pGib->Spawn("models/germangibs.mdl");
        pGib->pev->body = 0;
    }
    else
    {
        pGib->Spawn("models/hgibs.mdl");
        pGib->pev->body = 0;
    }

    if (pevVictim)
    {
        pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;

        edict_t* pentPlayer = FIND_CLIENT_IN_PVS(pGib->edict());

        if (RANDOM_LONG(0, 100) <= 5 && pentPlayer)
        {
            entvars_t* pevPlayer;

            pevPlayer = VARS(pentPlayer);
            pGib->pev->velocity = ((pevPlayer->origin + pevPlayer->view_ofs) - pGib->pev->origin).Normalize() * 300;
            pGib->pev->velocity.z += 100;
        }
        else
        {
            pGib->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
        }

        pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
        pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

        pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

        if (pevVictim->health > -50)
        {
            pGib->pev->velocity = pGib->pev->velocity * 0.7;
        }
        else if (pevVictim->health > -200)
        {
            pGib->pev->velocity = pGib->pev->velocity * 2;
        }
        else
        {
            pGib->pev->velocity = pGib->pev->velocity * 4;
        }
    }
    pGib->LimitVelocity();
}

void CGib::SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int human)
{
    int cSplat;

    for (cSplat = 0; cSplat < cGibs; cSplat++)
    {
        CGib* pGib = GetClassPtr((CGib*)NULL);

        if (g_Language == LANGUAGE_GERMAN)
        {
            pGib->Spawn("models/germangibs.mdl");
            pGib->pev->body = RANDOM_LONG(0, GERMAN_GIB_COUNT - 1);
        }
        else
        {
            if (human)
            {

                pGib->Spawn("models/hgibs.mdl");
                pGib->pev->body = RANDOM_LONG(1, HUMAN_GIB_COUNT - 1);
            }
            else
            {

                pGib->Spawn("models/agibs.mdl");
                pGib->pev->body = RANDOM_LONG(0, ALIEN_GIB_COUNT - 1);
            }
        }

        if (pevVictim)
        {

            pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT(0, 1));
            pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT(0, 1));
            pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT(0, 1)) + 1;

            pGib->pev->velocity = g_vecAttackDir * -1;

            pGib->pev->velocity.x += RANDOM_FLOAT(-0.25, 0.25);
            pGib->pev->velocity.y += RANDOM_FLOAT(-0.25, 0.25);
            pGib->pev->velocity.z += RANDOM_FLOAT(-0.25, 0.25);

            pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT(300, 400);

            pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
            pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

            pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

            if (pevVictim->health > -50)
            {
                pGib->pev->velocity = pGib->pev->velocity * 0.7;
            }
            else if (pevVictim->health > -200)
            {
                pGib->pev->velocity = pGib->pev->velocity * 2;
            }
            else
            {
                pGib->pev->velocity = pGib->pev->velocity * 4;
            }

            pGib->pev->solid = SOLID_BBOX;
            UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
        }
        pGib->LimitVelocity();
    }
}

BOOL CBaseMonster::HasHumanGibs(void)
{
    int myClass = Classify();

    if (myClass == CLASS_HUMAN_MILITARY ||
        myClass == CLASS_PLAYER_ALLY ||
        myClass == CLASS_HUMAN_PASSIVE ||
        myClass == CLASS_PLAYER)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CBaseMonster::HasAlienGibs(void)
{
    int myClass = Classify();

    if (myClass == CLASS_ALIEN_MILITARY ||
        myClass == CLASS_ALIEN_MONSTER ||
        myClass == CLASS_ALIEN_PASSIVE ||
        myClass == CLASS_INSECT ||
        myClass == CLASS_ALIEN_PREDATOR ||
        myClass == CLASS_ALIEN_PREY)
    {
        return TRUE;
    }

    return FALSE;
}

void CBaseMonster::FadeMonster(void)
{
    StopAnimation();
    pev->velocity = g_vecZero;
    pev->movetype = MOVETYPE_NONE;
    pev->avelocity = g_vecZero;
    pev->animtime = gpGlobals->time;
    pev->effects |= EF_NOINTERP;
    SUB_StartFadeOut();
}

void CBaseMonster::GibMonster(void)
{
    TraceResult	tr;
    BOOL		gibbed = FALSE;

    EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);

    if (HasHumanGibs())
    {
        if (CVAR_GET_FLOAT("violence_hgibs") != 0)
        {
            CGib::SpawnHeadGib(pev);
            CGib::SpawnRandomGibs(pev, 4, 1);
        }
        gibbed = TRUE;
    }
    else if (HasAlienGibs())
    {
        if (CVAR_GET_FLOAT("violence_agibs") != 0)
        {
            CGib::SpawnRandomGibs(pev, 4, 0);
        }
        gibbed = TRUE;
    }

    if (!IsPlayer())
    {
        if (gibbed)
        {
            SetThink(&CBaseEntity::SUB_Remove);
            pev->nextthink = gpGlobals->time;
        }
        else
        {
            FadeMonster();
        }
    }
}

Activity CBaseMonster::GetDeathActivity(void)
{
    Activity	deathActivity;
    BOOL		fTriedDirection;
    float		flDot;
    TraceResult	tr;
    Vector		vecSrc;

    if (pev->deadflag != DEAD_NO)
    {
        return m_IdealActivity;
    }

    vecSrc = Center();

    fTriedDirection = FALSE;
    deathActivity = ACT_DIESIMPLE;

    UTIL_MakeVectors(pev->angles);
    flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

    switch (m_LastHitGroup)
    {

    case HITGROUP_HEAD:
        deathActivity = ACT_DIE_HEADSHOT;
        break;

    case HITGROUP_STOMACH:
        deathActivity = ACT_DIE_GUTSHOT;
        break;

    case HITGROUP_GENERIC:

        fTriedDirection = TRUE;

        if (flDot > 0.3)
        {
            deathActivity = ACT_DIEFORWARD;
        }
        else if (flDot <= -0.3)
        {
            deathActivity = ACT_DIEBACKWARD;
        }
        break;

    default:

        fTriedDirection = TRUE;

        if (flDot > 0.3)
        {
            deathActivity = ACT_DIEFORWARD;
        }
        else if (flDot <= -0.3)
        {
            deathActivity = ACT_DIEBACKWARD;
        }
        break;
    }

    if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
    {
        if (fTriedDirection)
        {
            deathActivity = ACT_DIESIMPLE;
        }
        else
        {
            if (flDot > 0.3)
            {
                deathActivity = ACT_DIEFORWARD;
            }
            else if (flDot <= -0.3)
            {
                deathActivity = ACT_DIEBACKWARD;
            }
        }
    }

    if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
    {
        deathActivity = ACT_DIESIMPLE;
    }

    if (deathActivity == ACT_DIEFORWARD)
    {
        UTIL_TraceHull(vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

        if (tr.flFraction != 1.0)
        {
            deathActivity = ACT_DIESIMPLE;
        }
    }

    if (deathActivity == ACT_DIEBACKWARD)
    {
        UTIL_TraceHull(vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

        if (tr.flFraction != 1.0)
        {
            deathActivity = ACT_DIESIMPLE;
        }
    }

    return deathActivity;
}

Activity CBaseMonster::GetSmallFlinchActivity(void)
{
    Activity	flinchActivity;

    UTIL_MakeVectors(pev->angles);

    switch (m_LastHitGroup)
    {

    case HITGROUP_HEAD:
        flinchActivity = ACT_FLINCH_HEAD;
        break;
    case HITGROUP_STOMACH:
        flinchActivity = ACT_FLINCH_STOMACH;
        break;
    case HITGROUP_LEFTARM:
        flinchActivity = ACT_FLINCH_LEFTARM;
        break;
    case HITGROUP_RIGHTARM:
        flinchActivity = ACT_FLINCH_RIGHTARM;
        break;
    case HITGROUP_LEFTLEG:
        flinchActivity = ACT_FLINCH_LEFTLEG;
        break;
    case HITGROUP_RIGHTLEG:
        flinchActivity = ACT_FLINCH_RIGHTLEG;
        break;
    default:
        flinchActivity = ACT_SMALL_FLINCH;
        break;
    }

    if (LookupActivity(flinchActivity) == ACTIVITY_NOT_AVAILABLE)
    {
        flinchActivity = ACT_SMALL_FLINCH;
    }

    return flinchActivity;
}

void CBaseMonster::BecomeDead(void)
{
    pev->takedamage = DAMAGE_YES;

    pev->health = pev->max_health / 2;
    pev->max_health = 5;

    pev->movetype = MOVETYPE_TOSS;
}

BOOL CBaseMonster::ShouldGibMonster(int iGib)
{
    if ((iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE) || (iGib == GIB_ALWAYS))
        return TRUE;

    return FALSE;
}

void CBaseMonster::CallGibMonster(void)
{
    BOOL fade = FALSE;

    if (HasHumanGibs())
    {
        if (CVAR_GET_FLOAT("violence_hgibs") == 0)
            fade = TRUE;
    }
    else if (HasAlienGibs())
    {
        if (CVAR_GET_FLOAT("violence_agibs") == 0)
            fade = TRUE;
    }

    pev->takedamage = DAMAGE_NO;
    pev->solid = SOLID_NOT;

    if (fade)
    {
        FadeMonster();
    }
    else
    {
        pev->effects = EF_NODRAW;
        GibMonster();
    }

    pev->deadflag = DEAD_DEAD;
    FCheckAITrigger();

    if (pev->health < -99)
    {
        pev->health = 0;
    }

    if (ShouldFadeOnDeath() && !fade)
        UTIL_Remove(this);
}

void CBaseMonster::Killed(entvars_t* pevAttacker, int iGib)
{
    unsigned int	cCount = 0;
    BOOL			fDone = FALSE;

    if (HasMemory(bits_MEMORY_KILLED))
    {
        if (ShouldGibMonster(iGib))
            CallGibMonster();
        return;
    }

    Remember(bits_MEMORY_KILLED);

    EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
    m_IdealMonsterState = MONSTERSTATE_DEAD;

    SetConditions(bits_COND_LIGHT_DAMAGE);

    CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
    if (pOwner)
    {
        pOwner->DeathNotice(pev);
    }

    if (ShouldGibMonster(iGib))
    {
        CallGibMonster();
        return;
    }
    else if (pev->flags & FL_MONSTER)
    {
        SetTouch(NULL);
        BecomeDead();
    }

    if (pev->health < -99)
    {
        pev->health = 0;
    }

    m_IdealMonsterState = MONSTERSTATE_DEAD;
}

void CBaseEntity::SUB_StartFadeOut(void)
{
    if (pev->rendermode == kRenderNormal)
    {
        pev->renderamt = 255;
        pev->rendermode = kRenderTransTexture;
    }

    pev->solid = SOLID_NOT;
    pev->avelocity = g_vecZero;

    pev->nextthink = gpGlobals->time + 0.1;
    SetThink(&CBaseEntity::SUB_FadeOut);
}

void CBaseEntity::SUB_FadeOut(void)
{
    if (pev->renderamt > 7)
    {
        pev->renderamt -= 7;
        pev->nextthink = gpGlobals->time + 0.1;
    }
    else
    {
        pev->renderamt = 0;
        pev->nextthink = gpGlobals->time + 0.2;
        SetThink(&CBaseEntity::SUB_Remove);
    }
}

void CGib::WaitTillLand(void)
{
    if (!IsInWorld())
    {
        UTIL_Remove(this);
        return;
    }

    if (pev->velocity == g_vecZero)
    {
        SetThink(&CBaseEntity::SUB_StartFadeOut);
        pev->nextthink = gpGlobals->time + m_lifeTime;

        if (m_bloodColor != DONT_BLEED)
        {
            CSoundEnt::InsertSound(bits_SOUND_MEAT, pev->origin, 384, 25);
        }
    }
    else
    {
        pev->nextthink = gpGlobals->time + 0.5;
    }
}

void CGib::BounceGibTouch(CBaseEntity* pOther)
{
    Vector	vecSpot;
    TraceResult	tr;

    if (pev->flags & FL_ONGROUND)
    {

        pev->velocity = pev->velocity * 0.9;
        pev->angles.x = 0;
        pev->angles.z = 0;
        pev->avelocity.x = 0;
        pev->avelocity.z = 0;
    }
    else
    {
        if (g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED)
        {
            vecSpot = pev->origin + Vector(0, 0, 8);
            UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), ignore_monsters, ENT(pev), &tr);

            UTIL_BloodDecalTrace(&tr, m_bloodColor);

            m_cBloodDecals--;
        }

        if (m_material != matNone && RANDOM_LONG(0, 2) == 0)
        {
            float volume;
            float zvel = fabs(pev->velocity.z);

            volume = 0.8 * min(1.0, ((float)zvel) / 450.0);

            CBreakable::MaterialSoundRandom(edict(), (Materials)m_material, volume);
        }
    }
}

void CGib::StickyGibTouch(CBaseEntity* pOther)
{
    Vector	vecSpot;
    TraceResult	tr;

    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 10;

    if (!FClassnameIs(pOther->pev, "worldspawn"))
    {
        pev->nextthink = gpGlobals->time;
        return;
    }

    UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT(pev), &tr);

    UTIL_BloodDecalTrace(&tr, m_bloodColor);

    pev->velocity = tr.vecPlaneNormal * -1;
    pev->angles = UTIL_VecToAngles(pev->velocity);
    pev->velocity = g_vecZero;
    pev->avelocity = g_vecZero;
    pev->movetype = MOVETYPE_NONE;
}

void CGib::Spawn(const char* szGibModel)
{
    pev->movetype = MOVETYPE_BOUNCE;
    pev->friction = 0.55;

    pev->renderamt = 255;
    pev->rendermode = kRenderNormal;
    pev->renderfx = kRenderFxNone;
    pev->solid = SOLID_SLIDEBOX;
    pev->classname = MAKE_STRING("gib");

    SET_MODEL(ENT(pev), szGibModel);
    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    pev->nextthink = gpGlobals->time + 4;
    m_lifeTime = 25;
    SetThink(&CGib::WaitTillLand);
    SetTouch(&CGib::BounceGibTouch);

    m_material = matNone;
    m_cBloodDecals = 5;
}

int CBaseMonster::TakeHealth(float flHealth, int bitsDamageType)
{
    if (!pev->takedamage)
        return 0;

    m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);

    return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

int CBaseMonster::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    float	flTake;
    Vector	vecDir;

    if (!pev->takedamage)
        return 0;

    if (!IsAlive())
    {
        return DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
    }

    if (pev->deadflag == DEAD_NO)
    {
        PainSound();
    }

    flTake = flDamage;

    m_bitsDamageType |= bitsDamageType;

    vecDir = Vector(0, 0, 0);
    if (!FNullEnt(pevInflictor))
    {
        CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor);
        if (pInflictor)
        {
            vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
            vecDir = g_vecAttackDir = vecDir.Normalize();
        }
    }

    if (IsPlayer())
    {
        if (pevInflictor)
            pev->dmg_inflictor = ENT(pevInflictor);

        pev->dmg_take += flTake;
    }

    pev->health -= flTake;

    if (m_MonsterState == MONSTERSTATE_SCRIPT)
    {
        SetConditions(bits_COND_LIGHT_DAMAGE);
        return 0;
    }

    if (pev->health <= 0)
    {
        g_pevLastInflictor = pevInflictor;

        if (bitsDamageType & DMG_ALWAYSGIB)
        {
            Killed(pevAttacker, GIB_ALWAYS);
        }
        else if (bitsDamageType & DMG_NEVERGIB)
        {
            Killed(pevAttacker, GIB_NEVER);
        }
        else
        {
            Killed(pevAttacker, GIB_NORMAL);
        }

        g_pevLastInflictor = NULL;

        return 0;
    }

    if ((pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker))
    {
        if (pevAttacker->flags & (FL_MONSTER | FL_CLIENT))
        {
            if (pevInflictor)
            {
                if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
                {
                    m_vecEnemyLKP = pevInflictor->origin;
                }
            }
            else
            {
                m_vecEnemyLKP = pev->origin + (g_vecAttackDir * 64);
            }

            MakeIdealYaw(m_vecEnemyLKP);

            if (flDamage > 0)
            {
                SetConditions(bits_COND_LIGHT_DAMAGE);
            }

            if (flDamage >= 20)
            {
                SetConditions(bits_COND_HEAVY_DAMAGE);
            }
        }
    }

    return 1;
}

int CBaseMonster::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    Vector			vecDir;

    vecDir = Vector(0, 0, 0);
    if (!FNullEnt(pevInflictor))
    {
        CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor);
        if (pInflictor)
        {
            vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
            vecDir = g_vecAttackDir = vecDir.Normalize();
        }
    }

#if 0

    pev->flags &= ~FL_ONGROUND;
    pev->origin.z += 1;

    if (!FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER))
    {
        pev->velocity = pev->velocity + vecDir * -DamageForce(flDamage);
    }

#endif

    if (bitsDamageType & DMG_GIB_CORPSE)
    {
        if (pev->health <= flDamage)
        {
            pev->health = -50;
            Killed(pevAttacker, GIB_ALWAYS);
            return 0;
        }

        pev->health -= flDamage * 0.1;
    }

    return 1;
}

float CBaseMonster::DamageForce(float damage)
{
    float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

    if (force > 1000.0)
    {
        force = 1000.0;
    }

    return force;
}

void RadiusFlash(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
    CBaseEntity* pEntity = NULL;
    TraceResult tr;
    float flAdjustedDamage, falloff;
    Vector vecSpot;

    falloff = flDamage / 1000.0;

    int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

    vecSrc.z += 1;

    while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 1000.0)) != NULL)
    {
        Vector vecLOS;
        float flDot;
        float fadeTime;
        float fadeHold;
        int alpha;
        CBasePlayer* pPlayer;

        if (!pEntity->IsPlayer())
            break;

        pPlayer = (CBasePlayer*)pEntity;

        if (pPlayer->pev->takedamage == DAMAGE_NO || pPlayer->pev->deadflag != DEAD_NO)
            continue;

        if (bInWater && pPlayer->pev->waterlevel == 0)
            continue;

        if (!bInWater && pPlayer->pev->waterlevel == 3)
            continue;

        vecSpot = pPlayer->BodyTarget(vecSrc);

        UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

        if (tr.flFraction != 1.0f && tr.pHit != pPlayer->pev->pContainingEntity)
            continue;

        if (tr.fStartSolid)
        {
            tr.vecEndPos = vecSrc;
            tr.flFraction = 0;
        }

        flAdjustedDamage = flDamage - (vecSrc - tr.vecEndPos).Length() * falloff;
        if (flAdjustedDamage < 0)
            flAdjustedDamage = 0;

        UTIL_MakeVectors(pPlayer->pev->v_angle);
        vecLOS = vecSrc - pPlayer->EarPosition();
        flDot = DotProduct(vecLOS, gpGlobals->v_forward);

        if (flDot < 0)
        {
            alpha = 200;
            fadeTime = flAdjustedDamage;
            fadeHold = flAdjustedDamage / 6.0;
        }
        else
        {
            alpha = 255;
            fadeTime = flAdjustedDamage * 3;
            fadeHold = flAdjustedDamage / 1.5;
        }

        Vector color(255, 255, 255);
        UTIL_ScreenFade(pPlayer, color, fadeTime, fadeHold, alpha, FFADE_IN);
    }
}

void RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
{
    CBaseEntity* pEntity = NULL;
    float		flAdjustedDamage, falloff;

    if (flRadius)
        falloff = flDamage / flRadius;
    else
        falloff = 1.0;

    int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

    vecSrc.z += 1;

    if (!pevAttacker)
        pevAttacker = pevInflictor;

    while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
    {
        if (pEntity->pev->takedamage != DAMAGE_NO)
        {
            if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
                continue;

            if (bInWater && pEntity->pev->waterlevel == 0)
                continue;
            if (!bInWater && pEntity->pev->waterlevel == 3)
                continue;

            float length = (vecSrc - pEntity->pev->origin).Length();

            flAdjustedDamage = flDamage - length * falloff;

            if (flAdjustedDamage < 0)
                flAdjustedDamage = 0;

            pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
        }
    }
}

void RadiusDamage2(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
{
    CBaseEntity* pEntity = NULL;
    TraceResult	tr;
    float		flAdjustedDamage, falloff;
    Vector		vecSpot;

    if (flRadius)
        falloff = flDamage / flRadius;
    else
        falloff = 1.0;

    int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

    vecSrc.z += 1;

    if (!pevAttacker)
        pevAttacker = pevInflictor;

    while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
    {
        if (pEntity->pev->takedamage != DAMAGE_NO)
        {
            if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
                continue;

            if (bInWater && pEntity->pev->waterlevel == 0)
                continue;
            if (!bInWater && pEntity->pev->waterlevel == 3)
                continue;

            vecSpot = pEntity->BodyTarget(vecSrc);

            UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

            if (tr.flFraction == 1.0 || tr.pHit == pEntity->edict())
            {
                if (tr.fStartSolid)
                {
                    tr.vecEndPos = vecSrc;
                    tr.flFraction = 0.0;
                }

                flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;

                if (flAdjustedDamage < 0)
                    flAdjustedDamage = 0;
                else if (flAdjustedDamage > 75)
                    flAdjustedDamage = 75;

                if (tr.flFraction == 1.0)
                {
                    pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
                }
                else
                {
                    ClearMultiDamage();
                    pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
                    ApplyMultiDamage(pevInflictor, pevAttacker);
                }
            }
        }
    }
}

void CBaseMonster::RadiusDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
    if (flDamage > 80.0)
    {
        ::RadiusDamage(pev->origin, pevInflictor, pevAttacker, flDamage, 3.5 * flDamage, iClassIgnore, bitsDamageType);
    }
    else
    {
        ::RadiusDamage2(pev->origin, pevInflictor, pevAttacker, flDamage, (RANDOM_FLOAT(0.5, 1.5) + 3.0) * flDamage, iClassIgnore, bitsDamageType);
    }
}

void CBaseMonster::RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
    if (flDamage > 80.0)
    {
        ::RadiusDamage(pev->origin, pevInflictor, pevAttacker, flDamage, 3.5 * flDamage, iClassIgnore, bitsDamageType);
    }
    else
    {
        ::RadiusDamage2(pev->origin, pevInflictor, pevAttacker, flDamage, (RANDOM_FLOAT(0.5, 1.5) + 3.0) * flDamage, iClassIgnore, bitsDamageType);
    }
}

CBaseEntity* CBaseMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
    TraceResult tr;

    if (IsPlayer())
        UTIL_MakeVectors(pev->angles);
    else
        UTIL_MakeAimVectors(pev->angles);

    Vector vecStart = pev->origin;
    vecStart.z += pev->size.z * 0.5;
    Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

    UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

    if (tr.pHit)
    {
        CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

        if (iDamage > 0)
        {
            pEntity->TakeDamage(pev, pev, iDamage, iDmgType);
        }

        return pEntity;
    }

    return NULL;
}

BOOL CBaseMonster::FInViewCone(CBaseEntity* pEntity)
{
    Vector2D	vec2LOS;
    float	flDot;

    UTIL_MakeVectors(pev->angles);

    vec2LOS = (pEntity->pev->origin - pev->origin).Make2D();
    vec2LOS = vec2LOS.Normalize();

    flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

    if (flDot > m_flFieldOfView)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CBaseMonster::FInViewCone(Vector* pOrigin)
{
    Vector2D	vec2LOS;
    float		flDot;

    UTIL_MakeVectors(pev->angles);

    vec2LOS = (*pOrigin - pev->origin).Make2D();
    vec2LOS = vec2LOS.Normalize();

    flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

    if (flDot > m_flFieldOfView)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CBaseMonster::KeyValue(KeyValueData* pkvd)
{
    CBaseToggle::KeyValue(pkvd);
}

BOOL CBaseEntity::FVisible(CBaseEntity* pEntity)
{
    TraceResult tr;
    Vector		vecLookerOrigin;
    Vector		vecTargetOrigin;

    if (FBitSet(pEntity->pev->flags, FL_NOTARGET))
        return FALSE;

    if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3)
        || (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
        return FALSE;

    vecLookerOrigin = pev->origin + pev->view_ofs;
    vecTargetOrigin = pEntity->EyePosition();

    UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev), &tr);

    if (tr.flFraction != 1.0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL CBaseEntity::FVisible(const Vector& vecOrigin)
{
    TraceResult tr;
    Vector		vecLookerOrigin;

    vecLookerOrigin = EyePosition();

    UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT(pev), &tr);

    if (tr.flFraction != 1.0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void CBaseEntity::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
    Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

    if (pev->takedamage)
    {
        AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);

        int blood = BloodColor();

        if (blood != DONT_BLEED)
        {
            SpawnBlood(vecOrigin, blood, flDamage);
            TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
        }
    }
}

void CBaseMonster::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
    if (pev->takedamage)
    {
        m_LastHitGroup = ptr->iHitgroup;

        switch (ptr->iHitgroup)
        {
        case HITGROUP_HEAD:
            flDamage *= 3.0;
            break;
        case HITGROUP_CHEST:
        case HITGROUP_STOMACH:
            flDamage *= 1.5;
            break;
        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            flDamage *= 0.75;
            break;
        default:
            break;
        }

        Vector vecSpot = ptr->vecEndPos - vecDir * 4;

        AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);

        int bloodColor = BloodColor();
        if (bloodColor != DONT_BLEED)
        {
            SpawnBlood(vecSpot, bloodColor, flDamage);
        }
    }
}

void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker)
{
    static int tracerCount;
    int tracer;
    TraceResult tr;
    Vector vecRight = gpGlobals->v_right;
    Vector vecUp = gpGlobals->v_up;

    if (pevAttacker == NULL)
        pevAttacker = pev;

    ClearMultiDamage();
    gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

    for (ULONG iShot = 1; iShot <= cShots; iShot++)
    {

        float x, y, z;
        do {
            x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
            y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
            z = x * x + y * y;
        } while (z > 1);

        Vector vecDir = vecDirShooting +
            x * vecSpread.x * vecRight +
            y * vecSpread.y * vecUp;
        Vector vecEnd;

        vecEnd = vecSrc + vecDir * flDistance;
        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

        tracer = 0;
        if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
        {
            Vector vecTracerSrc;

            if (IsPlayer())
            {
                vecTracerSrc = vecSrc + Vector(0, 0, -4) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
            }
            else
            {
                vecTracerSrc = vecSrc;
            }

            if (iTracerFreq != 1)
                tracer = 1;

            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecTracerSrc);
            WRITE_BYTE(TE_TRACER);
            WRITE_COORD(vecTracerSrc.x);
            WRITE_COORD(vecTracerSrc.y);
            WRITE_COORD(vecTracerSrc.z);
            WRITE_COORD(tr.vecEndPos.x);
            WRITE_COORD(tr.vecEndPos.y);
            WRITE_COORD(tr.vecEndPos.z);
            MESSAGE_END();
        }

        if (tr.flFraction != 1.0)
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            if (iDamage)
            {
                pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));

                TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                DecalGunshot(&tr, iBulletType, false, pev, false);
            }
            else switch (iBulletType)
            {
            default:
            case BULLET_PLAYER_9MM:
                pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET);
                if (!tracer)
                {
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                break;

            case BULLET_PLAYER_MP5:
                pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET);
                if (!tracer)
                {
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                break;

            case BULLET_PLAYER_BUCKSHOT:
            {

                int iShotgunDamage = (int)((1.0 - tr.flFraction) * 20.0);
                ALERT(at_console, "Shotgun Damage = %i\n", iShotgunDamage);
                pEntity->TraceAttack(pevAttacker, iShotgunDamage, vecDir, &tr, DMG_BULLET);
                if (!tracer)
                {
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                break;
            }

            case BULLET_PLAYER_357:
                pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET);
                if (!tracer)
                {
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                break;

            case BULLET_MONSTER_9MM:
                pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);

                TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                DecalGunshot(&tr, iBulletType, false, pev, false);

                break;

            case BULLET_MONSTER_MP5:
                pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);

                TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                DecalGunshot(&tr, iBulletType, false, pev, false);

                break;

            case BULLET_MONSTER_12MM:
                pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET);
                if (!tracer)
                {
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                break;

            case BULLET_NONE:
                pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
                TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);

                if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
                {
                    UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
                }

                break;
            }
        }

        UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
    }
    ApplyMultiDamage(pev, pevAttacker);
}

void CBaseEntity::FireBullets2(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iPenetration, int iTracerFreq, int iDamage, entvars_t* pevAttacker)
{
    TraceResult tr;
    Vector vecRight = gpGlobals->v_right;
    Vector vecUp = gpGlobals->v_up;
    BOOL bHitPlayer = FALSE;

    if (pevAttacker == NULL)
        pevAttacker = pev;

    ClearMultiDamage();
    gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

    for (ULONG iShot = 1; iShot <= cShots; iShot++)
    {
        float x, y, z;
        do {
            x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
            y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
            z = x * x + y * y;
        } while (z > 1);

        Vector vecDir = vecDirShooting +
            x * vecSpread.x * vecRight +
            y * vecSpread.y * vecUp;

        Vector vecEnd = vecSrc + vecDir * flDistance;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

        if (tr.flFraction != 1.0)
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

            if (VARS(tr.pHit)->solid == SOLID_BSP)
            {
                if (iPenetration)
                {
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    --iPenetration;

                    vecSrc = tr.vecEndPos + vecDir * 32.0;
                    vecEnd = vecSrc + vecDir * flDistance;

                    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET);

                    UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

                    if (tr.flFraction != 1.0)
                    {
                        pEntity = CBaseEntity::Instance(tr.pHit);
                    }
                }
            }

            if (tr.flFraction != 1.0)
            {
                if (iDamage)
                {
                    pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr,
                        DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                }
                else switch (iBulletType)
                {
                default:
                case BULLET_PLAYER_9MM:
                    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_PLAYER_MP5:
                    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_PLAYER_357:
                    pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_PLAYER_BUCKSHOT:
                    pEntity->TraceAttack(pevAttacker, 20, vecDir, &tr, DMG_BULLET);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_MONSTER_9MM:
                    pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_MONSTER_MP5:
                    pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_MONSTER_12MM:
                    pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    DecalGunshot(&tr, iBulletType, false, pev, false);
                    break;

                case BULLET_NONE:
                    pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
                    TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
                    if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
                    {
                        UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
                    }
                    break;
                }

                if (pEntity->pev->classname == MAKE_STRING("player") && iPenetration > 0 && cShots == 1)
                    bHitPlayer = TRUE;
            }
        }

        UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
    }

    ApplyMultiDamage(pev, pevAttacker);

    if (bHitPlayer)
    {
        vecSrc = tr.vecEndPos + vecDirShooting * 32.0;
        Vector vecEnd = vecSrc + vecDirShooting * flDistance;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

        if (tr.flFraction != 1.0)
        {
            CBaseEntity* pEntity2 = CBaseEntity::Instance(tr.pHit);
            pEntity2->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDirShooting, &tr, DMG_BULLET);
            ApplyMultiDamage(pEntity2->pev, pevAttacker);
        }
    }
}

Vector CBaseEntity::FireBullets3(Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, bool bPistol)
{
    int iPenetrationPower;
    float flPenetrationDist;
    int iSparksAmount;
    int iCurrentDamage;
    float flCurrentDistance;

    Vector vecRight = gpGlobals->v_right;
    Vector vecUp = gpGlobals->v_up;

    bool bHitMetal = false;

    int iStartPenetration = iPenetration;

    switch (iBulletType)
    {
    case BULLET_PLAYER_9MM:
        iPenetrationPower = 14;
        flPenetrationDist = 800.0f;
        iSparksAmount = 15;
        break;
    case BULLET_PLAYER_45ACP:
        iPenetrationPower = 10;
        flPenetrationDist = 500.0f;
        iSparksAmount = 20;
        break;
    case BULLET_PLAYER_50AE:
        iPenetrationPower = 30;
        flPenetrationDist = 8000.0f;
        iSparksAmount = 30;
        break;
    case BULLET_PLAYER_762MM:
        iPenetrationPower = 24;
        flPenetrationDist = 5000.0f;
        iSparksAmount = 30;
        break;
    case BULLET_PLAYER_556MM:
        iPenetrationPower = 20;
        flPenetrationDist = 4000.0f;
        iSparksAmount = 30;
        break;
    case BULLET_PLAYER_338MAG:
        iPenetrationPower = 20;
        flPenetrationDist = 1000.0f;
        iSparksAmount = 20;
        break;
    case BULLET_PLAYER_57MM:
        iPenetrationPower = 20;
        flPenetrationDist = 2500.0f;
        iSparksAmount = 20;
        break;
    case BULLET_PLAYER_357SIG:
        iPenetrationPower = 20;
        flPenetrationDist = 800.0f;
        iSparksAmount = 20;
        break;
    default:
        iPenetrationPower = 0;
        flPenetrationDist = 0.0f;
        break;
    }

    if (!pevAttacker)
        pevAttacker = pev;

    gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

    float x, y, z;
    do {
        x = RANDOM_FLOAT(-0.5f, 0.5f) + RANDOM_FLOAT(-0.5f, 0.5f);
        y = RANDOM_FLOAT(-0.5f, 0.5f) + RANDOM_FLOAT(-0.5f, 0.5f);
        z = x * x + y * y;
    } while (z > 1.0f);

    Vector vecDir = vecDirShooting +
        x * vecSpread.x * vecRight +
        y * vecSpread.y * vecUp;

    Vector vecEnd = vecSrc + vecDir * flDistance;

    while (iPenetration != 0)
    {
        ClearMultiDamage();

        TraceResult tr;
        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

        char cTextureType = UTIL_TextureHit(&tr, vecSrc, vecEnd);
        bool bSparks = false;

        switch (cTextureType)
        {
        case CHAR_TEX_METAL:
            bSparks = true;
            bHitMetal = true;
            iPenetrationPower *= 0.25;
            break;
        case CHAR_TEX_CONCRETE:
            iPenetrationPower *= 0.35;
            break;
        case CHAR_TEX_GRATE:
        case CHAR_TEX_VENT:
            bSparks = true;
            bHitMetal = true;
            iPenetrationPower *= 0.65;
            break;
        case CHAR_TEX_TILE:
            iPenetrationPower *= 0.75;
            break;
        case CHAR_TEX_COMPUTER:
            bSparks = true;
            bHitMetal = true;
            iPenetrationPower *= 0.4;
            break;
        case CHAR_TEX_WOOD:
            break;
        default:
            bSparks = false;
            break;
        }

        ALERT(at_console, "Penetration Power : %i\n", iPenetrationPower);

        if (tr.flFraction >= 1.0f)
        {
            iPenetration = 0;
        }
        else
        {

            CBaseEntity* pEntity = NULL;
            if (tr.pHit)
                pEntity = CBaseEntity::Instance(tr.pHit);

            iPenetration--;

            flCurrentDistance = tr.flFraction * flDistance;
            iCurrentDamage = (int)((float)iDamage * pow(flRangeModifier, flCurrentDistance * 0.002));

            if (iPenetration == iStartPenetration - 1 && bSparks)
            {
                if (!RANDOM_LONG(0, 2))
                {
                    MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, tr.vecEndPos, ENT(pev));
                    WRITE_BYTE(TE_STREAK_SPLASH);
                    WRITE_COORD(tr.vecEndPos.x);
                    WRITE_COORD(tr.vecEndPos.y);
                    WRITE_COORD(tr.vecEndPos.z);
                    WRITE_COORD(tr.vecPlaneNormal.x);
                    WRITE_COORD(tr.vecPlaneNormal.y);
                    WRITE_COORD(tr.vecPlaneNormal.z);
                    WRITE_BYTE(5);
                    WRITE_SHORT(iSparksAmount);
                    WRITE_SHORT(50);
                    WRITE_SHORT(100);
                    MESSAGE_END();
                }
            }

            if (flCurrentDistance > flPenetrationDist)
                iPenetration = 0;

            if (VARS(tr.pHit)->solid == SOLID_BSP && iPenetration > 0)
            {
                DecalGunshot(&tr, iBulletType, (!bPistol && RANDOM_LONG(0, 3)), pev, bHitMetal);

                vecSrc = tr.vecEndPos + vecDir * (float)iPenetrationPower;

                flDistance *= 0.5f;

                vecEnd = vecSrc + vecDir * flDistance;

                if (pEntity)
                    pEntity->TraceAttack(pevAttacker, iCurrentDamage, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB);

                iDamage = (int)(0.6f * (float)iCurrentDamage);
            }
            else
            {
                DecalGunshot(&tr, iBulletType, (!bPistol && RANDOM_LONG(0, 3)), pev, bHitMetal);

                vecSrc = tr.vecEndPos + vecDir * 32.0f;

                flDistance *= 0.7f;

                vecEnd = vecSrc + vecDir * flDistance;

                if (pEntity)
                    pEntity->TraceAttack(pevAttacker, iCurrentDamage, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB);

                iDamage = (int)(0.75f * (float)iCurrentDamage);
            }
        }

        ApplyMultiDamage(pev, pevAttacker);
    }

    return Vector(x * vecSpread.x, y * vecSpread.y, 0);
}

void CBaseEntity::TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
    if (BloodColor() == DONT_BLEED)
        return;

    if (flDamage == 0)
        return;

    if (!(bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)))
        return;

    TraceResult Bloodtr;
    Vector vecTraceDir;
    float flNoise;
    int cCount;
    int i;

    if (flDamage < 10)
    {
        flNoise = 0.1;
        cCount = 1;
    }
    else if (flDamage < 25)
    {
        flNoise = 0.2;
        cCount = 2;
    }
    else
    {
        flNoise = 0.3;
        cCount = 4;
    }

    for (i = 0; i < cCount; i++)
    {
        vecTraceDir = vecDir * -1;

        vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
        vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
        vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

        UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

        if (Bloodtr.flFraction != 1.0)
        {
            if (!RANDOM_LONG(0, 2))
            {
                UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
            }
        }
    }
}

void CBaseMonster::BloodSplat(Vector& vecSrc, Vector& vecDir, int hitgroup, int iDamage)
{
    if (hitgroup == 1)
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, (float*)&vecSrc);
        WRITE_BYTE(TE_BLOODSTREAM);
        WRITE_COORD(vecSrc.x);
        WRITE_COORD(vecSrc.y);
        WRITE_COORD(vecSrc.z);
        WRITE_COORD(vecDir.x);
        WRITE_COORD(vecDir.y);
        WRITE_COORD(vecDir.z);
        WRITE_BYTE(223);
        WRITE_BYTE(iDamage + RANDOM_LONG(0, 100));
        MESSAGE_END();
    }
}

void CBaseMonster::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult* ptr, const Vector& vecDir)
{

    TraceResult Bloodtr;
    Vector vecTraceDir;
    int i;

    if (!IsAlive())
    {
        if (pev->max_health <= 0)
        {

            return;
        }
        else
        {
            pev->max_health--;
        }
    }

    for (i = 0; i < cCount; i++)
    {
        vecTraceDir = vecDir;

        vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
        vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
        vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

        UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

        if (Bloodtr.flFraction != 1.0)
        {
            UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
        }
    }
}

void CBaseMonster::CorpseFallThink(void)
{
    if (pev->flags & FL_ONGROUND)
    {
        SetThink(NULL);

        SetSequenceBox();
        UTIL_SetOrigin(pev, pev->origin);
    }
    else
        pev->nextthink = gpGlobals->time + 0.1;
}

void CBaseMonster::MonsterInitDead(void)
{
    InitBoneControllers();

    pev->solid = SOLID_BBOX;
    pev->movetype = MOVETYPE_TOSS;

    pev->frame = 0;
    ResetSequenceInfo();
    pev->framerate = 0;

    pev->max_health = pev->health;
    pev->deadflag = DEAD_DEAD;

    UTIL_SetSize(pev, g_vecZero, g_vecZero);
    UTIL_SetOrigin(pev, pev->origin);

    BecomeDead();
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 0.5;
}

int CBaseMonster::IRelationship(CBaseEntity* pTarget)
{
    static int iEnemy[14][14] =
    {
    { R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_DL	,R_DL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_DL,	R_DL	},
    { R_NO	,R_DL	,R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_DL,	R_DL	},
    { R_NO	,R_NO	,R_AL	,R_AL	,R_HT	,R_FR	,R_NO	,R_HT	,R_DL	,R_FR	,R_NO	,R_AL,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_HT	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_HT,	R_NO,	R_NO	},
    { R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
    { R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_FR	,R_NO	,R_DL,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO	},
    { R_FR	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR,	R_NO,	R_NO	},
    { R_NO	,R_DL	,R_AL	,R_AL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO	},
    { R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_NO,	R_DL	},
    { R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_AL	,R_NO	,R_DL	,R_DL	,R_NO	,R_NO	,R_DL,	R_DL,	R_NO	}
    };

    return iEnemy[Classify()][pTarget->Classify()];
}

void CBaseMonster::Look(int iDistance)
{
    int	iSighted = 0;

    ClearConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT);

    m_pLink = NULL;

    CBaseEntity* pSightEnt = NULL;

    CBaseEntity* pList[100];

    Vector delta = Vector(iDistance, iDistance, iDistance);

    int count = UTIL_EntitiesInBox(pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER);
    for (int i = 0; i < count; i++)
    {
        pSightEnt = pList[i];
        if (pSightEnt != this && pSightEnt->pev->health > 0)
        {
            if (IRelationship(pSightEnt) != R_NO && FInViewCone(pSightEnt) && !FBitSet(pSightEnt->pev->flags, FL_NOTARGET) && FVisible(pSightEnt))
            {
                if (pSightEnt->IsPlayer())
                {
                    iSighted |= bits_COND_SEE_CLIENT;
                }

                pSightEnt->m_pLink = m_pLink;
                m_pLink = pSightEnt;

                if (pSightEnt == m_hEnemy)
                {
                    iSighted |= bits_COND_SEE_ENEMY;
                }

                switch (IRelationship(pSightEnt))
                {
                case	R_NM:
                    iSighted |= bits_COND_SEE_NEMESIS;
                    break;
                case	R_HT:
                    iSighted |= bits_COND_SEE_HATE;
                    break;
                case	R_DL:
                    iSighted |= bits_COND_SEE_DISLIKE;
                    break;
                case	R_FR:
                    iSighted |= bits_COND_SEE_FEAR;
                    break;
                default:
                    break;
                }
            }
        }
    }

    SetConditions(iSighted);
}

CBaseEntity* CBaseMonster::BestVisibleEnemy(void)
{
    CBaseEntity* pReturn;
    CBaseEntity* pNextEnt;
    int			iNearest;
    int			iDist;
    int			iBestRelationship;

    iNearest = 8192;
    pNextEnt = m_pLink;
    pReturn = NULL;
    iBestRelationship = R_NO;

    while (pNextEnt != NULL)
    {
        if (pNextEnt->IsAlive())
        {
            if (IRelationship(pNextEnt) > iBestRelationship)
            {
                iBestRelationship = IRelationship(pNextEnt);
                iNearest = (pNextEnt->pev->origin - pev->origin).Length();
                pReturn = pNextEnt;
            }
            else if (IRelationship(pNextEnt) == iBestRelationship)
            {
                iDist = (pNextEnt->pev->origin - pev->origin).Length();

                if (iDist <= iNearest)
                {
                    iNearest = iDist;
                    iBestRelationship = IRelationship(pNextEnt);
                    pReturn = pNextEnt;
                }
            }
        }

        pNextEnt = pNextEnt->m_pLink;
    }

    return pReturn;
}

