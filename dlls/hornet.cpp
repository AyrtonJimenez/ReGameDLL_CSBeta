#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"hornet.h"
#include	"gamerules.h"

int iHornetTrail;
int iHornetPuff;

LINK_ENTITY_TO_CLASS(hornet, CHornet);

TYPEDESCRIPTION	CHornet::m_SaveData[] =
{
    DEFINE_FIELD(CHornet, m_flStopAttack, FIELD_TIME),
    DEFINE_FIELD(CHornet, m_iHornetType, FIELD_INTEGER),
    DEFINE_FIELD(CHornet, m_flFlySpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CHornet, CBaseMonster);

int CHornet::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    bitsDamageType &= ~(DMG_ALWAYSGIB);
    bitsDamageType |= DMG_NEVERGIB;

    return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CHornet::Spawn(void)
{
    Precache();

    pev->movetype = MOVETYPE_FLY;
    pev->solid = SOLID_BBOX;
    pev->takedamage = DAMAGE_YES;
    pev->flags |= FL_MONSTER;
    pev->health = 1;

    if (g_pGameRules->IsMultiplayer())
    {
        m_flStopAttack = gpGlobals->time + 3.5;
    }
    else
    {
        m_flStopAttack = gpGlobals->time + 5.0;
    }

    m_flFieldOfView = 0.9;

    if (RANDOM_LONG(1, 5) <= 2)
    {
        m_iHornetType = HORNET_TYPE_RED;
        m_flFlySpeed = HORNET_RED_SPEED;
    }
    else
    {
        m_iHornetType = HORNET_TYPE_ORANGE;
        m_flFlySpeed = HORNET_ORANGE_SPEED;
    }

    SET_MODEL(ENT(pev), "models/hornet.mdl");
    UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));

    SetTouch(&CHornet::DieTouch);
    SetThink(&CHornet::StartTrack);

    edict_t* pSoundEnt = pev->owner;
    if (!pSoundEnt)
        pSoundEnt = edict();

    switch (RANDOM_LONG(0, 2))
    {
    case 0:	EMIT_SOUND(pSoundEnt, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM);	break;
    case 1:	EMIT_SOUND(pSoundEnt, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM);	break;
    case 2:	EMIT_SOUND(pSoundEnt, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM);	break;
    }

    if (!FNullEnt(pev->owner) && (pev->owner->v.flags & FL_CLIENT))
    {
        pev->dmg = gSkillData.plrDmgHornet;
    }
    else
    {
        pev->dmg = gSkillData.monDmgHornet;
    }

    pev->nextthink = gpGlobals->time + 0.1;
    ResetSequenceInfo();
}

void CHornet::Precache()
{
    PRECACHE_MODEL("models/hornet.mdl");

    PRECACHE_SOUND("agrunt/ag_fire1.wav");
    PRECACHE_SOUND("agrunt/ag_fire2.wav");
    PRECACHE_SOUND("agrunt/ag_fire3.wav");

    PRECACHE_SOUND("hornet/ag_buzz1.wav");
    PRECACHE_SOUND("hornet/ag_buzz2.wav");
    PRECACHE_SOUND("hornet/ag_buzz3.wav");

    PRECACHE_SOUND("hornet/ag_hornethit1.wav");
    PRECACHE_SOUND("hornet/ag_hornethit2.wav");
    PRECACHE_SOUND("hornet/ag_hornethit3.wav");

    iHornetPuff = PRECACHE_MODEL("sprites/muz1.spr");
    iHornetTrail = PRECACHE_MODEL("sprites/laserbeam.spr");
}

int CHornet::IRelationship(CBaseEntity* pTarget)
{
    if (pTarget->pev->modelindex == pev->modelindex)
    {
        return R_NO;
    }

    return CBaseMonster::IRelationship(pTarget);
}

int CHornet::Classify(void)
{
    if (pev->owner && pev->owner->v.flags & FL_CLIENT)
    {
        return CLASS_PLAYER_BIOWEAPON;
    }

    return	CLASS_ALIEN_BIOWEAPON;
}

void CHornet::StartTrack(void)
{
    IgniteTrail();

    SetTouch(&CHornet::TrackTouch);
    SetThink(&CHornet::TrackTarget);

    pev->nextthink = gpGlobals->time + 0.1;
}

void CHornet::StartDart(void)
{
    IgniteTrail();

    SetTouch(&CHornet::DartTouch);

    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 4;
}

void CHornet::IgniteTrail(void)
{
    MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
    WRITE_BYTE(TE_BEAMFOLLOW);
    WRITE_SHORT(entindex());
    WRITE_SHORT(iHornetTrail);
    WRITE_BYTE(10);
    WRITE_BYTE(2);

    switch (m_iHornetType)
    {
    case HORNET_TYPE_RED:
        WRITE_BYTE(179);
        WRITE_BYTE(39);
        WRITE_BYTE(14);
        break;
    case HORNET_TYPE_ORANGE:
        WRITE_BYTE(255);
        WRITE_BYTE(128);
        WRITE_BYTE(0);
        break;
    }

    WRITE_BYTE(128);

    MESSAGE_END();
}

void CHornet::TrackTarget(void)
{
    Vector	vecFlightDir;
    Vector	vecDirToEnemy;
    float	flDelta;

    StudioFrameAdvance();

    if (gpGlobals->time > m_flStopAttack)
    {
        SetTouch(NULL);
        SetThink(&CBaseEntity::SUB_Remove);
        pev->nextthink = gpGlobals->time + 0.1;
        return;
    }

    if (m_hEnemy == NULL)
    {
        Look(512);
        m_hEnemy = BestVisibleEnemy();
    }

    if (m_hEnemy != NULL && FVisible(m_hEnemy))
    {
        m_vecEnemyLKP = m_hEnemy->BodyTarget(pev->origin);
    }
    else
    {
        m_vecEnemyLKP = m_vecEnemyLKP + pev->velocity * m_flFlySpeed * 0.1;
    }

    vecDirToEnemy = (m_vecEnemyLKP - pev->origin).Normalize();

    if (pev->velocity.Length() < 0.1)
        vecFlightDir = vecDirToEnemy;
    else
        vecFlightDir = pev->velocity.Normalize();

    flDelta = DotProduct(vecFlightDir, vecDirToEnemy);

    if (flDelta < 0.5)
    {
        switch (RANDOM_LONG(0, 2))
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
        }
    }

    if (flDelta <= 0 && m_iHornetType == HORNET_TYPE_RED)
    {
        flDelta = 0.25;
    }

    pev->velocity = (vecFlightDir + vecDirToEnemy).Normalize();

    if (pev->owner && (pev->owner->v.flags & FL_MONSTER))
    {
        pev->velocity.x += RANDOM_FLOAT(-0.10, 0.10);
        pev->velocity.y += RANDOM_FLOAT(-0.10, 0.10);
        pev->velocity.z += RANDOM_FLOAT(-0.10, 0.10);
    }

    switch (m_iHornetType)
    {
    case HORNET_TYPE_RED:
        pev->velocity = pev->velocity * (m_flFlySpeed * flDelta);
        pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.3);
        break;
    case HORNET_TYPE_ORANGE:
        pev->velocity = pev->velocity * m_flFlySpeed;
        pev->nextthink = gpGlobals->time + 0.1;
        break;
    }

    pev->angles = UTIL_VecToAngles(pev->velocity);

    pev->solid = SOLID_BBOX;

    if (m_hEnemy != NULL && !g_pGameRules->IsMultiplayer())
    {
        if (flDelta >= 0.4 && (pev->origin - m_vecEnemyLKP).Length() <= 300)
        {
            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_SPRITE);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(iHornetPuff);

            WRITE_BYTE(2);
            WRITE_BYTE(128);
            MESSAGE_END();

            switch (RANDOM_LONG(0, 2))
            {
            case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
            case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
            case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
            }
            pev->velocity = pev->velocity * 2;
            pev->nextthink = gpGlobals->time + 1.0;

            m_flStopAttack = gpGlobals->time;
        }
    }
}

void CHornet::TrackTouch(CBaseEntity* pOther)
{
    if (pOther->edict() == pev->owner || pOther->pev->modelindex == pev->modelindex)
    {
        pev->solid = SOLID_NOT;
        return;
    }

    if (IRelationship(pOther) <= R_NO)
    {

        pev->velocity = pev->velocity.Normalize();

        pev->velocity.x *= -1;
        pev->velocity.y *= -1;

        pev->origin = pev->origin + pev->velocity * 4;
        pev->velocity = pev->velocity * m_flFlySpeed;

        return;
    }

    DieTouch(pOther);
}

void CHornet::DartTouch(CBaseEntity* pOther)
{
    DieTouch(pOther);
}

void CHornet::DieTouch(CBaseEntity* pOther)
{
    if (pOther && pOther->pev->takedamage)
    {

        switch (RANDOM_LONG(0, 2))
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_hornethit1.wav", 1, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_hornethit2.wav", 1, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hornet/ag_hornethit3.wav", 1, ATTN_NORM);	break;
        }

        pOther->TakeDamage(pev, VARS(pev->owner), pev->dmg, DMG_BULLET);
    }

    pev->modelindex = 0;
    pev->solid = SOLID_NOT;

    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 1;
}

