#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "player.h"
#include "gamerules.h"

extern int gmsgBarTime;

LINK_ENTITY_TO_CLASS(grenade, CGrenade);

#define SF_DETONATE		0x0001

void CGrenade::Explode(Vector vecSrc, Vector vecAim)
{
    TraceResult tr;

    UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -32), ignore_monsters, ENT(pev), &tr);

    Explode(&tr, DMG_BLAST);
}

void CGrenade::Explode(TraceResult* pTrace, int bitsDamageType)
{
    float flRndSound;

    pev->model = iStringNull;
    pev->solid = SOLID_NOT;
    pev->takedamage = DAMAGE_NO;

    if (pTrace->flFraction != 1.0)
    {
        pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
    }

    int iContents = UTIL_PointContents(pev->origin);

    CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0);

    entvars_t* pevOwner;
    if (pev->owner)
        pevOwner = VARS(pev->owner);
    else
        pevOwner = NULL;

    pev->owner = NULL;

    RadiusFlash(pev->origin, pev, pevOwner, 4, CLASS_NONE, bitsDamageType);

    if (RANDOM_FLOAT(0, 1) < 0.5)
    {
        UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
    }
    else
    {
        UTIL_DecalTrace(pTrace, DECAL_SCORCH2);
    }

    flRndSound = RANDOM_FLOAT(0, 1);

    switch (RANDOM_LONG(0, 1))
    {
    case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-2.wav", 0.55, ATTN_NORM);	break;
    case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-1.wav", 0.55, ATTN_NORM);	break;
    }

    pev->effects |= EF_NODRAW;
    SetThink(&CGrenade::Smoke);
    pev->velocity = g_vecZero;
    pev->nextthink = gpGlobals->time + 0.3;

    if (iContents != CONTENTS_WATER)
    {
        int sparkCount = RANDOM_LONG(0, 3);
        for (int i = 0; i < sparkCount; i++)
            Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
    }
}

void CGrenade::Smoke(void)
{
    if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
    {
        UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
    }
    else
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_SMOKE);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z);
        WRITE_SHORT(g_sModelIndexSmoke);
        WRITE_BYTE(25);
        WRITE_BYTE(6);
        MESSAGE_END();
    }
    UTIL_Remove(this);
}

void CGrenade::Killed(entvars_t* pevAttacker, int iGib)
{
    Detonate();
}

void CGrenade::DetonateUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    SetThink(&CGrenade::Detonate);
    pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate(void)
{
    CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, 400, 0.3);

    SetThink(&CGrenade::Detonate);
    pev->nextthink = gpGlobals->time + 1;
}

void CGrenade::Detonate(void)
{
    TraceResult tr;
    Vector		vecSpot;

    vecSpot = pev->origin + Vector(0, 0, 8);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);

    Explode(&tr, DMG_BLAST);
}

void CGrenade::ExplodeTouch(CBaseEntity* pOther)
{
    TraceResult tr;
    Vector		vecSpot;

    pev->enemy = pOther->edict();

    vecSpot = pev->origin - pev->velocity.Normalize() * 32;
    UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);

    Explode(&tr, DMG_BLAST);
}

void CGrenade::DangerSoundThink(void)
{
    if (!IsInWorld())
    {
        UTIL_Remove(this);
        return;
    }

    CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length(), 0.2);
    pev->nextthink = gpGlobals->time + 0.2;

    if (pev->waterlevel != 0)
    {
        pev->velocity = pev->velocity * 0.5;
    }
}

void CGrenade::BounceTouch(CBaseEntity* pOther)
{
    if (pOther->edict() == pev->owner)
        return;

    if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
    {
        pev->velocity = pev->velocity * -2.0f;
        return;
    }

    Vector vecTestVelocity;

    vecTestVelocity = pev->velocity;
    vecTestVelocity.z *= 0.7f;

    if (!m_fRegisteredSound && vecTestVelocity.Length() <= 60.0f)
    {
        CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3);
        m_fRegisteredSound = TRUE;
    }

    if (pev->flags & FL_ONGROUND)
    {

        pev->velocity = pev->velocity * 0.8f;

        pev->sequence = RANDOM_LONG(1, 1);
    }
    else
    {
        BounceSound();
    }

    pev->framerate = pev->velocity.Length() / 200.0f;
    if (pev->framerate > 1.0)
        pev->framerate = 1;
    else if (pev->framerate < 0.5)
        pev->framerate = 0;
}

void CGrenade::SlideTouch(CBaseEntity* pOther)
{
    if (pOther->edict() == pev->owner)
        return;

    if (pev->flags & FL_ONGROUND)
    {
        pev->velocity = pev->velocity * 0.95;
    }
    else
    {
        BounceSound();
    }
}

void CGrenade::BounceSound(void)
{
    if (pev->dmg > 50.0)
    {
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/he_bounce-1.wav", 0.25, ATTN_NORM);
        return;
    }

    switch (RANDOM_LONG(0, 2))
    {
    case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM);	break;
    case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM);	break;
    case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM);	break;
    }
}

void CGrenade::TumbleThink(void)
{
    if (!IsInWorld())
    {
        UTIL_Remove(this);
        return;
    }

    StudioFrameAdvance();
    pev->nextthink = gpGlobals->time + 0.1;

    if (pev->dmgtime - 1 < gpGlobals->time)
    {
        CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1);
    }

    if (pev->dmgtime <= gpGlobals->time)
    {
        if (pev->dmg <= 40.0)
            SetThink(&CGrenade::Detonate);
        else
            SetThink(&CGrenade::Detonate3);
    }

    if (pev->waterlevel != 0)
    {
        pev->velocity = pev->velocity * 0.5;
        pev->framerate = 0.2;
    }
}

void CGrenade::Spawn(void)
{
    pev->movetype = MOVETYPE_BOUNCE;
    pev->classname = MAKE_STRING("grenade");

    m_bIsC4 = false;
    pev->solid = SOLID_BBOX;

    SET_MODEL(ENT(pev), "models/grenade.mdl");
    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    pev->dmg = 30;
    m_iCurWave = 0;
}

CGrenade* CGrenade::ShootContact(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
    CGrenade* pGrenade = GetClassPtr((CGrenade*)NULL);
    pGrenade->Spawn();

    pGrenade->pev->gravity = 0.5;
    UTIL_SetOrigin(pGrenade->pev, vecStart);
    pGrenade->pev->velocity = vecVelocity;
    pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
    pGrenade->pev->owner = ENT(pevOwner);

    pGrenade->SetThink(&CGrenade::DangerSoundThink);
    pGrenade->pev->nextthink = gpGlobals->time;

    pGrenade->pev->avelocity.x = RANDOM_FLOAT(-100, -500);

    pGrenade->SetTouch(&CGrenade::ExplodeTouch);

    pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

    pGrenade->m_bJustBlew = TRUE;

    return pGrenade;
}

CGrenade* CGrenade::ShootTimed(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time)
{
    CGrenade* pGrenade = GetClassPtr((CGrenade*)NULL);
    pGrenade->Spawn();

    UTIL_SetOrigin(pGrenade->pev, vecStart);
    pGrenade->pev->velocity = vecVelocity;
    pGrenade->pev->angles = pevOwner->angles;
    pGrenade->pev->owner = ENT(pevOwner);

    pGrenade->SetTouch(&CGrenade::BounceTouch);

    pGrenade->pev->dmgtime = gpGlobals->time + time;
    pGrenade->SetThink(&CGrenade::TumbleThink);
    pGrenade->pev->nextthink = gpGlobals->time + 0.1;

    if (time < 0.1)
    {
        pGrenade->pev->nextthink = gpGlobals->time;
        pGrenade->pev->velocity = Vector(0, 0, 0);
    }

    pGrenade->pev->sequence = RANDOM_LONG(3, 6);
    pGrenade->pev->framerate = 1.0;

    pGrenade->m_bJustBlew = TRUE;

    pGrenade->pev->gravity = 0.5;
    pGrenade->pev->friction = 0.8;

    SET_MODEL(ENT(pGrenade->pev), "models/w_flashbang.mdl");
    pGrenade->pev->dmg = 35.0;

    return pGrenade;
}

CGrenade* CGrenade::ShootSatchelCharge(entvars_t* pevOwner, Vector vecStart, Vector vecAngles)
{
    CGrenade* pGrenade = GetClassPtr((CGrenade*)NULL);
    pGrenade->pev->movetype = MOVETYPE_TOSS;
    pGrenade->pev->classname = MAKE_STRING("grenade");

    pGrenade->pev->solid = SOLID_BBOX;

    SET_MODEL(ENT(pGrenade->pev), "models/w_c4.mdl");

    UTIL_SetSize(pGrenade->pev, Vector(0, 0, 0), Vector(8, 8, 8));

    pGrenade->pev->dmg = 100;
    UTIL_SetOrigin(pGrenade->pev, vecStart);
    pGrenade->pev->velocity = g_vecZero;
    pGrenade->pev->angles = vecAngles;
    pGrenade->pev->owner = ENT(pevOwner);

    pGrenade->SetThink(&CGrenade::C4Think);
    pGrenade->SetTouch(&CGrenade::C4Touch);
    pGrenade->pev->spawnflags = SF_DETONATE;

    pGrenade->pev->nextthink = gpGlobals->time + 0.1f;

    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    pGrenade->m_flC4Blow = gpGlobals->time + mp->m_iC4Timer;
    pGrenade->m_flNextFreqInterval = (float)(mp->m_iC4Timer / 4);
    pGrenade->m_flNextFreq = gpGlobals->time;

    pGrenade->m_iCurWave = 0;
    pGrenade->m_sBeepName = NULL;
    pGrenade->m_flNextBeep = gpGlobals->time + 0.5f;
    pGrenade->m_fAttenu = 0;

    pGrenade->m_bIsC4 = true;
    pGrenade->m_bStartDefuse = false;
    pGrenade->m_flNextBlink = gpGlobals->time + 2.0f;
    pGrenade->m_fNextDefuse = 0;

    pGrenade->pev->friction = 0.9f;
    pGrenade->m_bJustBlew = FALSE;

    edict_t* pentOwner = ENT(pevOwner);
    if (!pentOwner)
        pentOwner = INDEXENT(0);

    CBaseEntity* pOwner = CBaseEntity::Instance(pentOwner);

    if (pOwner && pOwner->IsPlayer())
        pGrenade->m_pentCurBombTarget = ((CBasePlayer*)pOwner)->m_pentCurBombTarget;
    else
        pGrenade->m_pentCurBombTarget = NULL;

    return pGrenade;
}

void CGrenade::UseSatchelCharges(entvars_t* pevOwner, SATCHELCODE code)
{
    edict_t* pentFind;
    edict_t* pentOwner;

    if (!pevOwner)
        return;

    CBaseEntity* pOwner = CBaseEntity::Instance(pevOwner);

    pentOwner = pOwner->edict();

    pentFind = FIND_ENTITY_BY_CLASSNAME(NULL, "grenade");
    while (!FNullEnt(pentFind))
    {
        CBaseEntity* pEnt = Instance(pentFind);
        if (pEnt)
        {
            if (FBitSet(pEnt->pev->spawnflags, SF_DETONATE) && pEnt->pev->owner == pentOwner)
            {
                if (code == SATCHEL_DETONATE)
                    pEnt->Use(pOwner, pOwner, USE_ON, 0);
                else
                    pEnt->pev->owner = NULL;
            }
        }
        pentFind = FIND_ENTITY_BY_CLASSNAME(pentFind, "grenade");
    }
}

void CGrenade::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    if (!m_bIsC4)
        return;

    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pActivator->pev);

    if (pPlayer->m_iTeam != TEAM_CT)
        return;

    if (m_bStartDefuse)
    {
        m_fNextDefuse = gpGlobals->time + 0.5f;
        return;
    }

    if (pPlayer->m_bHasDefuser)
    {
        ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Defusing bomb WITH Defuse kit...");
        EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, 0.8f);

        m_pBombDefuser = pActivator;
        m_bStartDefuse = true;
        m_flDefuseCountDown = gpGlobals->time + 5.0f;
        m_fNextDefuse = gpGlobals->time + 0.5f;

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pPlayer->pev));
        WRITE_SHORT(5);
    }
    else
    {
        ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Defusing bomb WITHOUT Defuse kit...");
        EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, 0.8f);

        m_pBombDefuser = pActivator;
        m_bStartDefuse = true;
        m_flDefuseCountDown = gpGlobals->time + 10.0f;
        m_fNextDefuse = gpGlobals->time + 0.5f;

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pPlayer->pev));
        WRITE_SHORT(10);
    }
    MESSAGE_END();
}

void CGrenade::C4Think(void)
{
    if (!IsInWorld())
    {
        UTIL_Remove(this);
        return;
    }

    pev->nextthink = gpGlobals->time + 0.12f;

    if (gpGlobals->time >= m_flNextFreq)
    {
        m_flNextFreq = gpGlobals->time + m_flNextFreqInterval;
        m_flNextFreqInterval *= 0.9f;

        switch (m_iCurWave)
        {
        case 0:
            m_sBeepName = "weapons/c4_beep1.wav";
            m_fAttenu = 1.5f;
            break;
        case 1:
            m_sBeepName = "weapons/c4_beep2.wav";
            m_fAttenu = 1.0f;
            break;
        case 2:
            m_sBeepName = "weapons/c4_beep3.wav";
            m_fAttenu = 0.8f;
            break;
        case 3:
            m_sBeepName = "weapons/c4_beep4.wav";
            m_fAttenu = 0.5f;
            break;
        case 4:
            m_sBeepName = "weapons/c4_beep5.wav";
            m_fAttenu = 0.2f;
            break;
        }
        m_iCurWave++;
    }

    if (gpGlobals->time >= m_flNextBeep)
    {
        m_flNextBeep = gpGlobals->time + 1.4f;
        EMIT_SOUND(ENT(pev), CHAN_VOICE, m_sBeepName, VOL_NORM, m_fAttenu);
    }

    if (gpGlobals->time >= m_flNextBlink)
    {
        m_flNextBlink = gpGlobals->time + 2.0f;

        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_GLOWSPRITE);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z + 5.0f);
        WRITE_SHORT(g_sModelIndexC4Glow);
        WRITE_BYTE(1);
        WRITE_BYTE(3);
        WRITE_BYTE(255);
        MESSAGE_END();
    }

    if (gpGlobals->time >= m_flC4Blow)
    {
        if (m_pentCurBombTarget)
        {
            CBaseEntity* pBombTarget = CBaseEntity::Instance(m_pentCurBombTarget);
            if (pBombTarget)
            {
                CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
                if (!pOwner)
                    pOwner = CBaseEntity::Instance(INDEXENT(0));

                pBombTarget->Use(pOwner, this, USE_TOGGLE, 0);
            }
        }

        if (pev->waterlevel != 0)
        {
            UTIL_Remove(this);
        }
        else
        {
            SetThink(&CGrenade::Detonate2);
        }
    }

    if (m_bStartDefuse)
    {
        CBasePlayer* pDefuser = (CBasePlayer*)m_pBombDefuser;
        if (pDefuser)
        {
            if (gpGlobals->time >= m_flDefuseCountDown)
            {
                if (pDefuser->pev->deadflag)
                {
                    m_bStartDefuse = false;
                    m_pBombDefuser = NULL;
                }
                else
                {
                    Broadcast("BOMBDEF");
                    UTIL_EmitAmbientSound(ENT(pev), pev->origin, "weapons/c4_beep5.wav", 0, 0, SND_STOP, 0);
                    EMIT_SOUND(ENT(pDefuser->pev), CHAN_WEAPON, "weapons/c4_disarmed.wav", 1.0f, 0.8f);
                    UTIL_Remove(this);
                    m_bJustBlew = TRUE;

                    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
                    mp->m_bBombDefused = TRUE;
                    mp->CheckWinConditions();

                    m_pBombDefuser = NULL;
                    m_bStartDefuse = false;
                }
            }
            else if (gpGlobals->time > m_fNextDefuse)
            {

                MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pDefuser->pev));
                WRITE_BYTE(0);
                MESSAGE_END();
                m_pBombDefuser = NULL;
                m_bStartDefuse = false;
                m_flDefuseCountDown = 0;
            }
        }
    }
}

void CGrenade::C4Touch(CBaseEntity* pOther)
{

}

void CGrenade::Explode2(TraceResult* pTrace, int bitsDamageType)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    pev->model = iStringNull;
    pev->solid = SOLID_NOT;
    pev->takedamage = DAMAGE_NO;

    UTIL_ScreenShake(pTrace->vecEndPos, 25.0f, 150.0f, 1.0f, 3000.0f);

    mp->m_bTargetBombed = true;
    m_bJustBlew = TRUE;

    mp->CheckWinConditions();

    if (pTrace->flFraction != 1.0)
    {
        pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
    }

    int iContents = UTIL_PointContents(pev->origin);

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_SPRITE);
    WRITE_COORD(pev->origin.x);
    WRITE_COORD(pev->origin.y);
    WRITE_COORD(pev->origin.z - 10.0f);
    WRITE_SHORT(g_sModelIndexFireball3);
    WRITE_BYTE((int)((pev->dmg - 275) * 0.6));
    WRITE_BYTE(150);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_SPRITE);
    WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
    WRITE_SHORT(g_sModelIndexFireball2);
    WRITE_BYTE((int)((pev->dmg - 275) * 0.6));
    WRITE_BYTE(150);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_SPRITE);
    WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
    WRITE_SHORT(g_sModelIndexFireball3);
    WRITE_BYTE((int)((pev->dmg - 275) * 0.6));
    WRITE_BYTE(150);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_SPRITE);
    WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
    WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
    WRITE_SHORT(g_sModelIndexFireball);
    WRITE_BYTE((int)((pev->dmg - 275) * 0.6));
    WRITE_BYTE(17);
    MESSAGE_END();

    EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/c4_explode1.wav", 1.0f, 0.25f);

    CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 1024, 3.0);

    entvars_t* pevOwner;
    if (pev->owner)
        pevOwner = VARS(pev->owner);
    else
        pevOwner = NULL;

    pev->owner = NULL;

    RadiusDamage(pev, pevOwner, mp->m_flBombRadius, CLASS_NONE, bitsDamageType);

    if (RANDOM_FLOAT(0, 1) < 0.5f)
        UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
    else
        UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

    RANDOM_FLOAT(0, 1);

    switch (RANDOM_LONG(0, 2))
    {
    case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55f, ATTN_NORM); break;
    case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55f, ATTN_NORM); break;
    case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55f, ATTN_NORM); break;
    }

    pev->effects |= EF_NODRAW;
    SetThink(&CGrenade::Smoke2);
    pev->velocity = g_vecZero;
    pev->nextthink = gpGlobals->time + 0.85f;

    if (iContents != CONTENTS_WATER)
    {
        int sparkCount = RANDOM_LONG(0, 3);
        for (int i = 0; i < sparkCount; i++)
        {
            Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
        }
    }
}

void CGrenade::Explode3(TraceResult* pTrace, int bitsDamageType)
{
    pev->model = iStringNull;
    pev->solid = SOLID_NOT;
    pev->takedamage = DAMAGE_NO;

    if (pTrace->flFraction != 1.0)
    {
        pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
    }

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_EXPLOSION);
    WRITE_COORD(pev->origin.x);
    WRITE_COORD(pev->origin.y);
    WRITE_COORD(pev->origin.z + 20.0f);
    WRITE_SHORT(g_sModelIndexFireball3);
    WRITE_BYTE(25);
    WRITE_BYTE(30);
    WRITE_BYTE(TE_EXPLFLAG_NONE);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
    WRITE_BYTE(TE_EXPLOSION);
    WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64, 64));
    WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64, 64));
    WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
    WRITE_SHORT(g_sModelIndexFireball2);
    WRITE_BYTE(30);
    WRITE_BYTE(30);
    WRITE_BYTE(TE_EXPLFLAG_NONE);
    MESSAGE_END();

    CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 1024, 3.0);

    entvars_t* pevOwner;
    if (pev->owner)
        pevOwner = VARS(pev->owner);
    else
        pevOwner = NULL;

    pev->owner = NULL;

    RadiusDamage(pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType);

    if (RANDOM_FLOAT(0, 1) < 0.5f)
        UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
    else
        UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

    RANDOM_FLOAT(0, 1);

    switch (RANDOM_LONG(0, 2))
    {
    case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55f, ATTN_NORM); break;
    case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55f, ATTN_NORM); break;
    case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55f, ATTN_NORM); break;
    }

    pev->effects |= EF_NODRAW;
    SetThink(&CGrenade::Smoke3_C);
    pev->velocity = g_vecZero;
    pev->nextthink = gpGlobals->time + 0.55f;

    int sparkCount = RANDOM_LONG(0, 3);
    for (int i = 0; i < sparkCount; i++)
    {
        Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
    }
}

void CGrenade::Smoke2(void)
{
    if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
    {
        UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
    }
    else
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_SMOKE);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z);
        WRITE_SHORT(g_sModelIndexSmoke);
        WRITE_BYTE(150);
        WRITE_BYTE(8);
        MESSAGE_END();
    }

    UTIL_Remove(this);
}

void CGrenade::Smoke3_A(void)
{
    if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
    {
        UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
    }
    else
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_SMOKE);
        WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-128, 128));
        WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-128, 128));
        WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
        WRITE_SHORT(g_sModelIndexSmoke);
        WRITE_BYTE((int)(RANDOM_FLOAT(0, 10) + 15.0f));
        WRITE_BYTE(12);
        MESSAGE_END();
    }

    UTIL_Remove(this);
}

void CGrenade::Smoke3_B(void)
{
    if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
    {
        UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
    }
    else
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_SMOKE);
        WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-128, 128));
        WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-128, 128));
        WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
        WRITE_SHORT(g_sModelIndexSmoke);
        WRITE_BYTE((int)(RANDOM_FLOAT(0, 10) + 15.0f));
        WRITE_BYTE(10);
        MESSAGE_END();
    }

    pev->nextthink = gpGlobals->time + 0.15f;
    SetThink(&CGrenade::Smoke3_A);
}

void CGrenade::Smoke3_C(void)
{
    if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
    {
        UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
    }
    else
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
        WRITE_BYTE(TE_SMOKE);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z - 5.0f);
        WRITE_SHORT(g_sModelIndexSmoke);
        WRITE_BYTE((int)(RANDOM_FLOAT(0, 10) + 35.0f));
        WRITE_BYTE(5);
        MESSAGE_END();
    }

    UTIL_Remove(this);
}

void CGrenade::Detonate2(void)
{
    ALERT(at_console, " ******* STAGE 0 ******* \n");

    TraceResult tr;
    Vector vecSpot;

    vecSpot = pev->origin + Vector(0, 0, 8);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);

    Explode2(&tr, DMG_BLAST);
}

void CGrenade::Detonate3(void)
{
    TraceResult tr;
    Vector vecSpot;

    vecSpot = pev->origin + Vector(0, 0, 8);
    UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);

    Explode3(&tr, 0x1000000);
}

CGrenade* CGrenade::ShootTimed2(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time, int team)
{
    CGrenade* pGrenade = GetClassPtr((CGrenade*)NULL);
    pGrenade->Spawn();

    UTIL_SetOrigin(pGrenade->pev, vecStart);
    pGrenade->pev->velocity = vecVelocity;
    pGrenade->pev->angles = pevOwner->angles;
    pGrenade->pev->owner = ENT(pevOwner);

    pGrenade->SetTouch(&CGrenade::BounceTouch);

    pGrenade->pev->dmgtime = gpGlobals->time + time;
    pGrenade->SetThink(&CGrenade::TumbleThink);
    pGrenade->pev->nextthink = gpGlobals->time + 0.1;

    pGrenade->pev->sequence = RANDOM_LONG(3, 6);
    pGrenade->pev->framerate = 1.0;

    pGrenade->m_bJustBlew = TRUE;

    pGrenade->pev->gravity = 0.55f;
    pGrenade->pev->friction = 0.7f;

    pGrenade->m_iTeam = team;

    SET_MODEL(ENT(pGrenade->pev), "models/w_hegrenade.mdl");
    pGrenade->pev->dmg = 100.0;

    return pGrenade;
}

