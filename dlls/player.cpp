#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "../engine/shake.h"
#include "decals.h"
#include "gamerules.h"

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL	BOOL	g_fDrawLines;
int gEvilImpulse101;
extern DLL_GLOBAL int		g_iSkillLevel, gDisplayTitle;
BOOL gInitHUD = TRUE;

extern int gmsgNVGToggle;
extern int gmsgStatusIcon;
extern int gmsgTeamInfo;
extern int gmsgMoney;
extern int gmsgRadar;
extern int gmsgSendAudio;
extern int gmsgRoundTime;
extern int gmsgBlinkAcct;
extern int gmsgBarTime;
extern int gmsgScoreAttrib;

extern void CopyToBodyQue(entvars_t* pev);
extern void respawn(entvars_t* pev, BOOL fCopyCorpse);
extern Vector VecBModelOrigin(entvars_t* pevBModel);
extern edict_t* EntSelectSpawnPoint(CBaseEntity* pPlayer);
int MapTextureTypeStepType(char chTextureType);

extern CGraph	WorldGraph;

#define	PLAYER_WALLJUMP_SPEED 300 
#define PLAYER_LONGJUMP_SPEED 350 

#define TRAIN_ACTIVE	0x80 
#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

#define	FLASH_DRAIN_TIME	 1.2 
#define	FLASH_CHARGE_TIME	 0.2 

TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] =
{
    DEFINE_FIELD(CBasePlayer, m_flFlashLightTime, FIELD_TIME),
    DEFINE_FIELD(CBasePlayer, m_iFlashBattery, FIELD_INTEGER),

    DEFINE_FIELD(CBasePlayer, m_afButtonLast, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_afButtonPressed, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_afButtonReleased, FIELD_INTEGER),

    DEFINE_ARRAY(CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS - 1),
    DEFINE_FIELD(CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER),

    DEFINE_FIELD(CBasePlayer, m_flTimeStepSound, FIELD_TIME),
    DEFINE_FIELD(CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME),
    DEFINE_FIELD(CBasePlayer, m_flSwimTime, FIELD_TIME),
    DEFINE_FIELD(CBasePlayer, m_flDuckTime, FIELD_TIME),
    DEFINE_FIELD(CBasePlayer, m_flWallJumpTime, FIELD_TIME),

    DEFINE_FIELD(CBasePlayer, m_flSuitUpdate, FIELD_TIME),
    DEFINE_ARRAY(CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST),
    DEFINE_FIELD(CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER),
    DEFINE_ARRAY(CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT),
    DEFINE_ARRAY(CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT),
    DEFINE_FIELD(CBasePlayer, m_lastDamageAmount, FIELD_INTEGER),

    DEFINE_ARRAY(CBasePlayer, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
    DEFINE_FIELD(CBasePlayer, m_pActiveItem, FIELD_CLASSPTR),
    DEFINE_FIELD(CBasePlayer, m_pLastItem, FIELD_CLASSPTR),

    DEFINE_ARRAY(CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
    DEFINE_FIELD(CBasePlayer, m_idrowndmg, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_idrownrestored, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_tSneaking, FIELD_TIME),

    DEFINE_FIELD(CBasePlayer, m_iTrain, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_flFallVelocity, FIELD_FLOAT),
    DEFINE_FIELD(CBasePlayer, m_iTargetVolume, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_iWeaponVolume, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_iWeaponFlash, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_fLongJump, FIELD_BOOLEAN),
    DEFINE_FIELD(CBasePlayer, m_fInitHUD, FIELD_BOOLEAN),
    DEFINE_FIELD(CBasePlayer, m_tbdPrev, FIELD_TIME),

    DEFINE_FIELD(CBasePlayer, m_pTank, FIELD_EHANDLE),
    DEFINE_FIELD(CBasePlayer, m_iHideHUD, FIELD_INTEGER),
    DEFINE_FIELD(CBasePlayer, m_iFOV, FIELD_INTEGER),

    DEFINE_FIELD(CBasePlayer, m_flDisplayHistory, FIELD_INTEGER),
};

int giPrecacheGrunt = 0;
int gmsgShake = 0;
int gmsgFade = 0;
int gmsgSelAmmo = 0;
int gmsgFlashlight = 0;
int gmsgFlashBattery = 0;
extern int gmsgResetHUD;
int gmsgInitHUD = 0;
int gmsgShowGameTitle = 0;
int gmsgCurWeapon = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgWeaponList = 0;
int gmsgAmmoX = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
extern int gmsgTeamInfo;
extern int gmsgTeamScore;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgArmorType = 0;
int gmsgHideWeapon = 0;
int gmsgSetCurWeap = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
extern int gmsgShowMenu;
int gmsgStatusValue = 0;
int gmsgStatusText = 0;
int gmsgReloadSound = 0;
int gmsgCrosshair = 0;

LINK_ENTITY_TO_CLASS(player, CBasePlayer);

void CBasePlayer::Pain(void)
{
    float	flRndSound;

    flRndSound = RANDOM_FLOAT(0, 1);

    if (flRndSound <= 0.33)
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM);
    else if (flRndSound <= 0.66)
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM);
    else
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM);
}

Vector VecVelocityForDamage(float flDamage)
{
    Vector vec(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));

    if (flDamage > -50)
        vec = vec * 0.7;
    else if (flDamage > -200)
        vec = vec * 2;
    else
        vec = vec * 10;

    return vec;
}

int TrainSpeed(int iSpeed, int iMax)
{
    float fSpeed, fMax;
    int iRet = 0;

    fMax = (float)iMax;
    fSpeed = iSpeed;

    fSpeed = fSpeed / fMax;

    if (iSpeed < 0)
        iRet = TRAIN_BACK;
    else if (iSpeed == 0)
        iRet = TRAIN_NEUTRAL;
    else if (fSpeed < 0.33)
        iRet = TRAIN_SLOW;
    else if (fSpeed < 0.66)
        iRet = TRAIN_MEDIUM;
    else
        iRet = TRAIN_FAST;

    return iRet;
}

void CBasePlayer::DeathSound(void)
{
    switch (RANDOM_LONG(1, 4))
    {
    case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die1.wav", VOL_NORM, ATTN_NORM); break;
    case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die2.wav", VOL_NORM, ATTN_NORM); break;
    case 3: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die3.wav", VOL_NORM, ATTN_NORM); break;
    case 4: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/death6.wav", VOL_NORM, ATTN_NORM); break;
    }
}

int CBasePlayer::TakeHealth(float flHealth, int bitsDamageType)
{
    return CBaseMonster::TakeHealth(flHealth, bitsDamageType);
}

Vector CBasePlayer::GetGunPosition()
{
    return pev->origin + pev->view_ofs;
}

void CBasePlayer::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
    BOOL fShouldBleed = TRUE;

    if (pev->takedamage)
    {
        m_LastHitGroup = ptr->iHitgroup;

        switch (ptr->iHitgroup)
        {
        case HITGROUP_GENERIC:
            break;

        case HITGROUP_HEAD:
            if (m_iKevlar == ARMOR_VESTHELM)
                fShouldBleed = FALSE;

            flDamage *= 4;

            pev->punchangle.x = flDamage * -0.5;

            if (pev->punchangle.x < -12)
                pev->punchangle.x = -12;

            pev->punchangle.z = RANDOM_FLOAT(-1, 1) * flDamage;

            if (pev->punchangle.z < -9)
                pev->punchangle.z = -9;
            else if (pev->punchangle.z > 9)
                pev->punchangle.z = 9;

            break;

        case HITGROUP_CHEST:
            if (m_iKevlar != ARMOR_NONE)
                fShouldBleed = FALSE;
            goto apply_torso_punchangle;

        case HITGROUP_STOMACH:
            if (m_iKevlar != ARMOR_NONE)
                fShouldBleed = FALSE;
            flDamage *= 1.25;
        apply_torso_punchangle:
            pev->punchangle.x = flDamage * -0.1;

            if (pev->punchangle.x < -4)
                pev->punchangle.x = -4;
            break;

        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            if (m_iKevlar != ARMOR_NONE)
                fShouldBleed = FALSE;
            break;

        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            flDamage *= 0.75;
            break;

        default:
            break;
        }

        if (fShouldBleed)
        {
            BloodSplat(ptr->vecEndPos, vecDir, ptr->iHitgroup, (int)(flDamage * 5));
            SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);
            TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
        }
        else if (ptr->iHitgroup == HITGROUP_HEAD)
        {
            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, ptr->vecEndPos);
            WRITE_BYTE(TE_STREAK_SPLASH);
            WRITE_COORD(ptr->vecEndPos.x);
            WRITE_COORD(ptr->vecEndPos.y);
            WRITE_COORD(ptr->vecEndPos.z);
            WRITE_COORD(ptr->vecPlaneNormal.x);
            WRITE_COORD(ptr->vecPlaneNormal.y);
            WRITE_COORD(ptr->vecPlaneNormal.z);
            WRITE_BYTE(5);
            WRITE_SHORT(22);
            WRITE_SHORT(25);
            WRITE_SHORT(65);
            MESSAGE_END();
        }

        AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
    }
}

#define ARMOR_RATIO	 0.5	
#define ARMOR_BONUS  0.5	

static BOOL ArmorCoversHitGroup(int iKevlar, int hitGroup)
{
    switch (iKevlar)
    {
    case ARMOR_KEVLAR:

        switch (hitGroup)
        {
        case HITGROUP_GENERIC:
        case HITGROUP_CHEST:
        case HITGROUP_STOMACH:
        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            return TRUE;
        }
        return FALSE;

    case ARMOR_VESTHELM:

        switch (hitGroup)
        {
        case HITGROUP_GENERIC:
        case HITGROUP_HEAD:
        case HITGROUP_CHEST:
        case HITGROUP_STOMACH:
        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

int CBasePlayer::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    int bitsDamage = bitsDamageType;
    int ffound = TRUE;
    int fmajor;
    int fcritical;
    int fTookDamage;
    int ftrivial;
    float flRatio;
    float flBonus;
    float flHealthPrev = pev->health;

    flBonus = ARMOR_BONUS;
    flRatio = ARMOR_RATIO;

    if (m_bIsVIP)
        flRatio = 0.25;

    if (bitsDamageType & (DMG_BLAST | DMG_GRENADE))
    {
        if (!IsAlive())
            return 0;

        float ff = CVAR_GET_FLOAT("mp_friendlyfire");
        if (ff <= 0.0
            && (bitsDamageType & DMG_GRENADE)
            && FClassnameIs(pevInflictor, "grenade"))
        {
            CGrenade* pGrenade = (CGrenade*)CBaseEntity::Instance(pevInflictor);

            if (pGrenade && pGrenade->m_iTeam == m_iTeam
                && pev != pevAttacker)
            {
                return 0;
            }
        }

        int noInflictor = 0;
        if (!pevInflictor->pContainingEntity || !OFFSET(pevInflictor->pContainingEntity))
            noInflictor = 1;

        if (!noInflictor)
        {
            m_vBlastVector = pev->origin - pevInflictor->origin;
        }

        if (pev->armorvalue <= 0)
        {
            Pain(m_LastHitGroup, false);
        }
        else
        {
            flBonus = ARMOR_BONUS * 2;

            if (ArmorCoversHitGroup(m_iKevlar, m_LastHitGroup))
            {
                float flNew = flDamage * flRatio;
                float flArmor = (flDamage - flNew) * flBonus;

                if (flArmor > pev->armorvalue)
                {
                    flNew = flDamage - pev->armorvalue * (1.0 / flBonus);
                    pev->armorvalue = 0;
                }
                else
                {
                    pev->armorvalue -= flArmor;
                }
                flDamage = flNew;

                if (pev->armorvalue <= 0)
                    m_iKevlar = ARMOR_NONE;
            }
            Pain(m_LastHitGroup, true);
        }

        m_lastDamageAmount = (int)flDamage;

        if (pev->health > (float)m_lastDamageAmount)
        {
            SetAnimation(PLAYER_FLINCH);
            Pain(m_LastHitGroup, false);
        }
        else if (bitsDamageType & DMG_BLAST)
        {
            m_bKilledByBomb = TRUE;
        }
        else if (bitsDamageType & DMG_GRENADE)
        {
            m_bKilledByGrenade = TRUE;
        }

        fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType);

        {
            for (int i = 0; i < CDMG_TIMEBASED; i++)
            {
                if (bitsDamageType & (DMG_PARALYZE << i))
                {
                    m_rgbTimeBasedDamage[i] = 0;
                }
            }
        }

        ALERT(at_console, "EXPLOSION DAMAGE\n");
        return fTookDamage;
    }

    edict_t* pAttackerEdict = pevAttacker->pContainingEntity;
    if (!pAttackerEdict)
        pAttackerEdict = INDEXENT(0);
    CBaseEntity* pAttacker = (pAttackerEdict) ? CBaseEntity::Instance(pAttackerEdict) : NULL;

    if (!g_pGameRules->FPlayerCanTakeDamage(this, pAttacker)
        && strcmp("grenade", STRING(pevInflictor->classname)))
    {
        return 0;
    }

    m_flVelocityModifier = 0.5;

    if (m_LastHitGroup == HITGROUP_HEAD)
    {
        m_bHighDamage = (flDamage > 60.0);
    }
    else
    {
        m_bHighDamage = (flDamage > 20.0);
    }

    SetAnimation(PLAYER_FLINCH);

    if ((bitsDamageType & DMG_BLAST) && g_pGameRules->IsMultiplayer())
        flBonus *= 2;

    if (!IsAlive())
        return 0;

    CBaseEntity* pAttackerEnt = CBaseEntity::Instance(pevAttacker);
    if (pAttackerEnt && pAttackerEnt->IsPlayer())
    {
        CBasePlayer* pAttackerPlayer = (CBasePlayer*)pAttackerEnt;

        if (pAttackerPlayer->m_iTeam == m_iTeam)
        {
            ClientPrint(pAttackerPlayer->pev, HUD_PRINTCENTER, "You injured a teammate!\n");

            if (!(m_flDisplayHistory & DHF_FRIEND_INJURED))
            {
                m_flDisplayHistory |= DHF_FRIEND_INJURED;
                pAttackerPlayer->HintMessage("Try not to injure your teammates.\n", FALSE, FALSE);
            }
        }

        if (pAttackerPlayer->m_iTeam == m_iTeam)
            flDamage *= 0.5;

        if (pAttackerPlayer->m_pActiveItem)
        {
            int weaponId = ((CBasePlayerWeapon*)pAttackerPlayer->m_pActiveItem)->m_iId;
            float flMult = 1.0;
            switch (weaponId)
            {
            case WEAPON_P228:    flMult = 1.25;  break;
            case WEAPON_SCOUT:
            case WEAPON_KNIFE:   flMult = 1.7;   break;
            case WEAPON_MAC10:   flMult = 0.95;  break;
            case WEAPON_AUG:
            case WEAPON_SG552:   flMult = 1.45;  break;
            case WEAPON_GLOCK18: flMult = 1.05;  break;
            case WEAPON_AWP:     flMult = 1.95;  break;
            case WEAPON_M249:
            case WEAPON_DEAGLE:  flMult = 1.5;   break;
            case WEAPON_M4A1:    flMult = 1.375; break;
            case WEAPON_G3SG1:   flMult = 1.65;  break;
            case WEAPON_AK47:    flMult = 1.55;  break;
            case WEAPON_P90:     flMult = 1.35;  break;
            }
            flRatio *= flMult;
        }
    }

    m_lastDamageAmount = (int)flDamage;

    if (pev->armorvalue <= 0 || (bitsDamageType & (DMG_FALL | DMG_DROWN)))
    {
        Pain(m_LastHitGroup, false);
    }
    else
    {
        if (ArmorCoversHitGroup(m_iKevlar, m_LastHitGroup))
        {
            float flNew = flDamage * flRatio;
            float flArmor = (flDamage - flNew) * flBonus;

            if (flArmor > pev->armorvalue)
            {
                flNew = flDamage - pev->armorvalue * (1.0 / flBonus);
                pev->armorvalue = 0;
            }
            else
            {
                pev->armorvalue -= flArmor;
            }
            flDamage = flNew;

            if (pev->armorvalue <= 0)
                m_iKevlar = ARMOR_NONE;
        }
        Pain(m_LastHitGroup, true);
    }

    fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType);

    {
        for (int i = 0; i < CDMG_TIMEBASED; i++)
        {
            if (bitsDamageType & (DMG_PARALYZE << i))
            {
                m_rgbTimeBasedDamage[i] = 0;
            }
        }
    }

    ftrivial = (pev->health > 75 || m_lastDamageAmount < 5);
    fmajor = (m_lastDamageAmount > 25);
    fcritical = (pev->health < 30);

    m_bitsDamageType |= bitsDamage;
    m_bitsHUDDamage = -1;

    while (fTookDamage && (!ftrivial || (bitsDamage & DMG_TIMEBASED)) && ffound && bitsDamage)
    {
        ffound = FALSE;

        if (bitsDamage & DMG_CLUB)
        {
            if (fmajor)
                SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);
            bitsDamage &= ~DMG_CLUB;
            ffound = TRUE;
        }
        if (bitsDamage & (DMG_FALL | DMG_CRUSH))
        {
            if (fmajor)
                SetSuitUpdate("!HEV_DMG5", FALSE, SUIT_NEXT_IN_30SEC);
            else
                SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);
            bitsDamage &= ~(DMG_FALL | DMG_CRUSH);
            ffound = TRUE;
        }
        if (bitsDamage & DMG_BULLET)
        {
            if (m_lastDamageAmount > 5)
                SetSuitUpdate("!HEV_DMG6", FALSE, SUIT_NEXT_IN_30SEC);
            bitsDamage &= ~DMG_BULLET;
            ffound = TRUE;
        }
        if (bitsDamage & DMG_SLASH)
        {
            if (fmajor)
                SetSuitUpdate("!HEV_DMG1", FALSE, SUIT_NEXT_IN_30SEC);
            else
                SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC);
            bitsDamage &= ~DMG_SLASH;
            ffound = TRUE;
        }
        if (bitsDamage & DMG_SONIC)
        {
            if (fmajor)
                SetSuitUpdate("!HEV_DMG2", FALSE, SUIT_NEXT_IN_1MIN);
            bitsDamage &= ~DMG_SONIC;
            ffound = TRUE;
        }
        if (bitsDamage & (DMG_POISON | DMG_PARALYZE))
        {
            SetSuitUpdate("!HEV_DMG3", FALSE, SUIT_NEXT_IN_1MIN);
            bitsDamage &= ~(DMG_POISON | DMG_PARALYZE);
            ffound = TRUE;
        }
        if (bitsDamage & DMG_ACID)
        {
            SetSuitUpdate("!HEV_DET1", FALSE, SUIT_NEXT_IN_1MIN);
            bitsDamage &= ~DMG_ACID;
            ffound = TRUE;
        }
        if (bitsDamage & DMG_NERVEGAS)
        {
            SetSuitUpdate("!HEV_DET0", FALSE, SUIT_NEXT_IN_1MIN);
            bitsDamage &= ~DMG_NERVEGAS;
            ffound = TRUE;
        }
        if (bitsDamage & DMG_RADIATION)
        {
            SetSuitUpdate("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);
            bitsDamage &= ~DMG_RADIATION;
            ffound = TRUE;
        }
        if (bitsDamage & DMG_SHOCK)
        {
            bitsDamage &= ~DMG_SHOCK;
            ffound = TRUE;
        }
    }

    if (fTookDamage && !ftrivial && fmajor && flHealthPrev >= 75)
    {
        SetSuitUpdate("!HEV_MED1", FALSE, SUIT_NEXT_IN_30MIN);
        SetSuitUpdate("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30MIN);
    }

    if (fTookDamage && !ftrivial && fcritical && flHealthPrev < 75)
    {
        if (pev->health < 6)
            SetSuitUpdate("!HEV_HLTH3", FALSE, SUIT_NEXT_IN_10MIN);
        else if (pev->health < 20)
            SetSuitUpdate("!HEV_HLTH2", FALSE, SUIT_NEXT_IN_10MIN);

        if (!RANDOM_LONG(0, 3) && flHealthPrev < 50)
            SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN);
    }

    if (fTookDamage && (bitsDamageType & DMG_TIMEBASED) && flHealthPrev < 75)
    {
        if (flHealthPrev < 50)
        {
            if (!RANDOM_LONG(0, 3))
                SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN);
        }
        else
        {
            SetSuitUpdate("!HEV_HLTH1", FALSE, SUIT_NEXT_IN_10MIN);
        }
    }

    return fTookDamage;
}

const char* GetCSModelName(int item_id)
{
    const char* modelName = NULL;

    switch (item_id)
    {
    case WEAPON_P228:      modelName = "models/w_p228.mdl"; break;
    case WEAPON_SCOUT:     modelName = "models/w_scout.mdl"; break;
    case WEAPON_HEGRENADE: modelName = "models/w_hegrenade.mdl"; break;
    case WEAPON_XM1014:    modelName = "models/w_xm1014.mdl"; break;
    case WEAPON_C4:        modelName = "models/w_backpack.mdl"; break;
    case WEAPON_MAC10:     modelName = "models/w_mac10.mdl"; break;
    case WEAPON_AUG:       modelName = "models/w_aug.mdl"; break;
    case WEAPON_USP:       modelName = "models/w_usp.mdl"; break;
    case WEAPON_GLOCK18:   modelName = "models/w_glock18.mdl"; break;
    case WEAPON_AWP:       modelName = "models/w_awp.mdl"; break;
    case WEAPON_MP5N:      modelName = "models/w_mp5.mdl"; break;
    case WEAPON_M249:      modelName = "models/w_m249.mdl"; break;
    case WEAPON_M3:        modelName = "models/w_m3.mdl"; break;
    case WEAPON_M4A1:      modelName = "models/w_m4a1.mdl"; break;
    case WEAPON_TMP:       modelName = "models/w_tmp.mdl"; break;
    case WEAPON_G3SG1:     modelName = "models/w_g3sg1.mdl"; break;
    case WEAPON_FLASHBANG: modelName = "models/w_flashbang.mdl"; break;
    case WEAPON_DEAGLE:    modelName = "models/w_deagle.mdl"; break;
    case WEAPON_SG552:     modelName = "models/w_sg552.mdl"; break;
    case WEAPON_AK47:      modelName = "models/w_ak47.mdl"; break;
    case WEAPON_KNIFE:     modelName = "models/w_knife.mdl"; break;
    case WEAPON_P90:       modelName = "models/w_p90.mdl"; break;
    default:
        ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item- not creating weaponbox\n");
        break;
    }

    return modelName;
}

void CBasePlayer::PackDeadPlayerItems(void)
{
    BOOL bPackWeapons = (g_pGameRules->DeadPlayerWeapons(this) != GR_PLR_DROP_GUN_NO);
    BOOL bPackAmmo = (g_pGameRules->DeadPlayerAmmo(this) != GR_PLR_DROP_AMMO_NO);

    if (bPackWeapons)
    {
        int iBestWeight = 0;
        CBasePlayerItem* pBestItem = NULL;

        for (int i = 0; i < MAX_ITEM_TYPES; i++)
        {
            CBasePlayerItem* pPlayerItem = m_rgpPlayerItems[i];

            while (pPlayerItem)
            {
                ItemInfo II;
                if (pPlayerItem->iItemSlot() <= 2
                    && pPlayerItem->GetItemInfo(&II)
                    && II.iWeight > iBestWeight)
                {
                    iBestWeight = II.iWeight;
                    pBestItem = pPlayerItem;
                }

                pPlayerItem = pPlayerItem->m_pNext;
            }
        }

        const char* modelname = NULL;
        if (pBestItem)
            modelname = GetCSModelName(pBestItem->m_iId);

        if (modelname)
        {
            CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create(
                "weaponbox", pev->origin, pev->angles, edict());

            pWeaponBox->pev->angles.x = 0;
            pWeaponBox->pev->angles.z = 0;

            pWeaponBox->pev->velocity = pev->velocity * 0.75;

            pWeaponBox->SetThink(&CWeaponBox::Kill);
            pWeaponBox->pev->nextthink = gpGlobals->time + 300.0;

            pWeaponBox->PackWeapon(pBestItem);

            if (bPackAmmo)
            {
                int iAmmoIndex = pBestItem->PrimaryAmmoIndex();
                pWeaponBox->PackAmmo(
                    MAKE_STRING(CBasePlayerItem::ItemInfoArray[pBestItem->m_iId].pszAmmo1),
                    m_rgAmmo[iAmmoIndex]);
            }

            SET_MODEL(ENT(pWeaponBox->pev), modelname);
        }
    }

    RemoveAllItems(TRUE);
}

void CBasePlayer::RemoveAllItems(BOOL removeSuit)
{
    BOOL bKillProgBar = FALSE;

    if (m_bHasDefuser)
    {
        m_bHasDefuser = FALSE;
        pev->body = 0;

        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(STATUSICON_HIDE);
        WRITE_STRING("defuser");
        MESSAGE_END();

        bKillProgBar = TRUE;
    }

    if (m_bHasC4)
    {
        m_bHasC4 = FALSE;
        pev->body = 0;

        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(STATUSICON_HIDE);
        WRITE_STRING("c4");
        MESSAGE_END();

        bKillProgBar = TRUE;
    }

    if (bKillProgBar)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pev));
        WRITE_SHORT(0);
        MESSAGE_END();
    }

    if (m_pActiveItem)
    {
        ResetAutoaim();
        m_pActiveItem->Holster();
        m_pActiveItem = NULL;
    }

    m_pLastItem = NULL;

    int i;
    CBasePlayerItem* pPendingItem;
    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        m_pActiveItem = m_rgpPlayerItems[i];
        while (m_pActiveItem)
        {
            pPendingItem = m_pActiveItem->m_pNext;
            m_pActiveItem->Drop();
            m_pActiveItem = pPendingItem;
        }
        m_rgpPlayerItems[i] = NULL;
    }

    m_pActiveItem = NULL;

    pev->viewmodel = 0;
    pev->weaponmodel = 0;

    if (removeSuit)
        pev->weapons = 0;
    else
        pev->weapons &= ~WEAPON_ALLWEAPONS;

    for (i = 0; i < MAX_AMMO_SLOTS; i++)
        m_rgAmmo[i] = 0;

    UpdateClientData();

    MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_BYTE(0);
    WRITE_BYTE(0);
    MESSAGE_END();
}

entvars_t* g_pevLastInflictor;

void CBasePlayer::Killed(entvars_t* pevAttacker, int iGib)
{
    CSound* pSound;

    if (!m_bKilledByBomb)
        g_pGameRules->PlayerKilled(this, pevAttacker, g_pevLastInflictor);

    int chasecam = (int)CVAR_GET_FLOAT("mp_chasecam");
    switch (chasecam)
    {
    case 1:
        m_iChaseCamMode = 1;
        break;
    case 0:
    case 2:
        m_iChaseCamMode = 0;
        break;
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgNVGToggle, NULL, ENT(pev));
    WRITE_BYTE(0);
    MESSAGE_END();
    m_bNightVisionOn = false;

    CLIENT_COMMAND(edict(), "lambert 1.5\n");

    if (m_pTank != NULL)
    {
        m_pTank->Use(this, this, USE_OFF, 0);
        m_pTank = NULL;
    }

    pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));
    if (pSound)
        pSound->Reset();

    SetAnimation(PLAYER_DIE);

    pev->modelindex = g_ulModelIndexPlayer;
    pev->view_ofs = Vector(0, 0, -8);
    pev->deadflag = DEAD_DYING;
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_TOSS;
    ClearBits(pev->flags, FL_ONGROUND);

    ALERT(at_console, "Throw Direction : %i\n", m_iThrowDirection);

    if (m_iThrowDirection)
    {
        switch (m_iThrowDirection)
        {
        case THROW_FORWARD:
            UTIL_MakeVectors(pev->angles);
            pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(100, 200);
            pev->velocity.z = RANDOM_FLOAT(50, 100);
            break;
        case THROW_BACKWARD:
            UTIL_MakeVectors(pev->angles);
            pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(-100, -200);
            pev->velocity.z = RANDOM_FLOAT(50, 100);
            break;
        case THROW_HITVEL:
            if (FClassnameIs(pevAttacker, "player"))
            {
                UTIL_MakeVectors(pevAttacker->angles);
                pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(200, 300);
                pev->velocity.z = RANDOM_FLOAT(200, 300);
            }
            break;
        case THROW_BOMB:
        {
            float dist = m_vBlastVector.Length();
            pev->velocity = (m_vBlastVector / dist) * ((2300.0f - dist) / 4.0f);
            pev->velocity.z = (2300.0f - m_vBlastVector.Length()) / 2.75f;
            ALERT(at_console, "Fly with the Bomb's blast!!\n");
            break;
        }
        case THROW_GRENADE:
        {
            float dist = m_vBlastVector.Length();
            pev->velocity = (m_vBlastVector / dist) * (500.0f - dist);
            pev->velocity.z = (350.0f - m_vBlastVector.Length()) * 1.5f;
            ALERT(at_console, "Fly with the HE Grenade's blast!!\n");
            break;
        }
        }
        m_iThrowDirection = THROW_NONE;
    }

    SetSuitUpdate(NULL, FALSE, 0);

    m_iClientHealth = 0;
    MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, ENT(pev));
    WRITE_BYTE(m_iClientHealth);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_BYTE(0xFF);
    WRITE_BYTE(0xFF);
    MESSAGE_END();

    m_iClientFOV = 0;
    m_iFOV = 0;
    MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, ENT(pev));
    WRITE_BYTE(0);
    MESSAGE_END();

    ((CHalfLifeMultiplay*)g_pGameRules)->CheckWinConditions();

    m_bNotKilled = FALSE;

    if (m_bHasC4)
    {
        DropPlayerItem("weapon_c4");

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pev));
        WRITE_SHORT(0);
        MESSAGE_END();
    }
    else if (m_bHasDefuser)
    {
        m_bHasDefuser = FALSE;
        pev->body = 0;
        GiveNamedItem("item_thighpack");

        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(0);
        WRITE_STRING("defuser");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pev));
        WRITE_SHORT(0);
        MESSAGE_END();
    }

    m_bEscaped = FALSE;

    MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_STRING("buyzone");
    MESSAGE_END();

    if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
        CLIENT_COMMAND(edict(), "slot10\n");

    SetThink(&CBasePlayer::PlayerDeathThink);
    pev->nextthink = gpGlobals->time + 0.1;

    if ((pev->health < -9000 && iGib != GIB_NEVER) || iGib == GIB_ALWAYS)
    {
        GibMonster();
        pev->effects |= EF_NODRAW;
        ((CHalfLifeMultiplay*)g_pGameRules)->CheckWinConditions();
        return;
    }

    DeathSound();

    pev->angles.x = 0;
    pev->angles.z = 0;
    m_bTeamChanged = false;
}

#define ACT_FLINCH_CS	77	

void CBasePlayer::SetAnimation(PLAYER_ANIM playerAnim)
{
    int animDesired;
    float speed;
    char szAnim[64];

    if (!pev->modelindex)
        return;

    if (playerAnim != PLAYER_FLINCH && gpGlobals->time <= m_flFlinchTime && pev->health > 0)
        return;

    speed = pev->velocity.Length2D();

    if (pev->flags & FL_FROZEN)
    {
        speed = 0;
        playerAnim = PLAYER_IDLE;
    }

    switch (playerAnim)
    {
    case PLAYER_JUMP:
        m_IdealActivity = ACT_HOP;
        break;

    case PLAYER_SUPERJUMP:
        m_IdealActivity = ACT_LEAP;
        break;

    case PLAYER_DIE:
        m_IdealActivity = ACT_DIESIMPLE;
        DeathSound();
        break;

    case PLAYER_ATTACK1:
        switch (m_Activity)
        {
        case ACT_HOVER:
        case ACT_SWIM:
        case ACT_HOP:
        case ACT_LEAP:
        case ACT_DIESIMPLE:
            m_IdealActivity = m_Activity;
            break;
        default:
            m_IdealActivity = ACT_RANGE_ATTACK1;
            break;
        }
        break;

    case PLAYER_FLINCH:
        if (gpGlobals->time < m_flAnimTime)
            return;
        m_IdealActivity = (Activity)ACT_FLINCH_CS;
        break;

    case PLAYER_IDLE:
    case PLAYER_WALK:

        if (!FBitSet(pev->flags, FL_ONGROUND) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP))
        {
            m_IdealActivity = m_Activity;
        }
        else if (pev->waterlevel > 1)
        {
            if (speed == 0)
                m_IdealActivity = ACT_HOVER;
            else
                m_IdealActivity = ACT_SWIM;
        }
        else
        {
            m_IdealActivity = ACT_WALK;
        }
        break;

    default:
        break;
    }

    switch (m_IdealActivity)
    {
    case ACT_RANGE_ATTACK1:

        if (FBitSet(pev->flags, FL_DUCKING))
            strcpy(szAnim, "crouch_shoot_");
        else
            strcpy(szAnim, "ref_shoot_");
        strcat(szAnim, m_szAnimExtention);
        animDesired = LookupSequence(szAnim);
        if (animDesired == -1)
            animDesired = 0;

        if (pev->sequence != animDesired || !m_fSequenceLoops)
            pev->frame = 0;

        if (!m_fSequenceLoops)
            pev->effects |= EF_NOINTERP;

        m_Activity = m_IdealActivity;
        pev->sequence = animDesired;
        ResetSequenceInfo();
        break;

    case (Activity)ACT_FLINCH_CS:
    {
        m_Activity = (Activity)ACT_FLINCH_CS;
        const char* pszFlinchSeq;

        switch (m_LastHitGroup)
        {
        case HITGROUP_GENERIC:
        case HITGROUP_CHEST:
            pszFlinchSeq = m_bHighDamage ? "flinch_stomach1" : "sflinch_stomach1";
            break;
        case HITGROUP_HEAD:
            pszFlinchSeq = m_bHighDamage ? "flinch_head1" : "sflinch_head1";
            break;
        case HITGROUP_STOMACH:
            pszFlinchSeq = m_bHighDamage ? "flinch_stomach1" : "sflinch_stomach2";
            break;
        case HITGROUP_LEFTARM:
            pszFlinchSeq = m_bHighDamage ? "flinch_leftarm1" : "sflinch_leftarm1";
            break;
        case HITGROUP_RIGHTARM:
            pszFlinchSeq = m_bHighDamage ? "flinch_rightarm1" : "sflinch_rightarm1";
            break;
        case HITGROUP_LEFTLEG:
            pszFlinchSeq = m_bHighDamage ? "flinch_leftleg1" : "sflinch_leftleg1";
            break;
        case HITGROUP_RIGHTLEG:
            pszFlinchSeq = m_bHighDamage ? "flinch_rightleg1" : "sflinch_rightleg1";
            break;
        default:
            pszFlinchSeq = "sflinch_stomach1";
            break;
        }

        animDesired = LookupSequence(pszFlinchSeq);

        if (FBitSet(pev->flags, FL_DUCKING))
        {
            animDesired = LookupSequence(RANDOM_LONG(0, 1) ? "flinch_crouch1" : "flinch_crouch2");
        }

        m_flAnimTime = gpGlobals->time + 0.1;
        if (m_bHighDamage)
            m_flFlinchTime = gpGlobals->time + 1.2;
        else
            m_flFlinchTime = gpGlobals->time + 0.5;

        pev->gaitsequence = 0;
        pev->sequence = animDesired;
        pev->frame = 0;
        ResetSequenceInfo();
        return;
    }

    case ACT_DIESIMPLE:
    {
        if (m_Activity == ACT_DIESIMPLE)
            return;
        m_Activity = ACT_DIESIMPLE;

        m_flDeathThrowTime = 0;
        m_iThrowDirection = 0;

        switch (m_LastHitGroup)
        {
        case HITGROUP_GENERIC:
            switch (RANDOM_LONG(0, 4))
            {
            case 0: animDesired = LookupSequence("die_head1"); m_iThrowDirection = 2; break;
            case 1: animDesired = LookupSequence("die_rightarm1"); m_iThrowDirection = 3; break;
            case 2: animDesired = LookupSequence("die_legs1"); break;
            case 3: animDesired = LookupSequence("die_legs2"); break;
            default: animDesired = LookupSequence("die_backwards"); m_iThrowDirection = 3; break;
            }
            break;

        case HITGROUP_HEAD:
        {
            int r = RANDOM_LONG(0, 6);
            if (m_bHighDamage)
                r++;
            if (r == 0)
            {
                animDesired = LookupSequence("headshot");
            }
            else if (r <= 2)
            {
                animDesired = LookupSequence("die_head1");
                m_iThrowDirection = 2;
            }
            else if (r <= 4)
            {
                animDesired = LookupSequence("die_head2");
                m_iThrowDirection = 1;
            }
            else
            {
                animDesired = LookupSequence("die_head3");
                m_iThrowDirection = 3;
            }
            break;
        }

        case HITGROUP_CHEST:
            if (!RANDOM_LONG(0, 3))
                animDesired = LookupSequence("gutshot");
            else
                animDesired = LookupSequence("die_stomach1");
            break;

        case HITGROUP_STOMACH:
            if (!m_bHighDamage && RANDOM_LONG(0, 3))
                animDesired = LookupSequence("die_stomach1");
            else
            {
                animDesired = LookupSequence("die_backwards");
                m_iThrowDirection = 3;
            }
            break;

        case HITGROUP_LEFTARM:
        {
            int r = RANDOM_LONG(0, 1);
            if (m_bHighDamage || r == 0)
            {
                animDesired = LookupSequence("die_leftarm2");
                m_iThrowDirection = 3;
            }
            else
            {
                animDesired = LookupSequence("die_leftarm1");
            }
            break;
        }

        case HITGROUP_RIGHTARM:
            m_iThrowDirection = 3;
            animDesired = LookupSequence("die_rightarm1");
            break;

        case HITGROUP_LEFTLEG:
        {
            int r = RANDOM_LONG(0, 4);
            if (r <= 1) animDesired = LookupSequence("die_legs1");
            else if (r <= 3) animDesired = LookupSequence("die_legs2");
            else animDesired = LookupSequence("die_forwards");
            break;
        }

        case HITGROUP_RIGHTLEG:
        {
            int r = RANDOM_LONG(0, 4);
            if (r <= 1) animDesired = LookupSequence("die_legs1");
            else if (r <= 3) animDesired = LookupSequence("die_legs2");
            else animDesired = LookupSequence("die_forwards");
            break;
        }

        default:
            animDesired = LookupSequence("die_forwards");
            break;
        }

        if (FBitSet(pev->flags, FL_DUCKING))
        {
            animDesired = LookupSequence("die_crouch1");
            m_iThrowDirection = 2;
        }

        else if (m_bKilledByBomb || m_bKilledByGrenade)
        {
            UTIL_MakeVectors(pev->angles);
            float flDot = DotProduct(gpGlobals->v_forward, m_vBlastVector);
            if (flDot <= 0)
            {
                if (RANDOM_LONG(0, 1))
                    animDesired = LookupSequence("die_crouch1");
                else
                    animDesired = LookupSequence("die_head3");
            }
            else
            {
                animDesired = LookupSequence("die_leftarm2");
            }
            if (m_bKilledByBomb)
                m_iThrowDirection = 4;
            else if (m_bKilledByGrenade)
                m_iThrowDirection = 5;
        }

        if (pev->sequence == animDesired)
            return;
        pev->gaitsequence = 0;
        pev->sequence = animDesired;
        pev->frame = 0;
        ResetSequenceInfo();
        return;
    }

    case ACT_HOVER:
    case ACT_LEAP:
    case ACT_SWIM:
    case ACT_HOP:
    default:
        if (m_Activity == m_IdealActivity)
            return;
        m_Activity = m_IdealActivity;

        animDesired = LookupActivity(m_Activity);
        if (pev->sequence == animDesired)
            return;

        pev->gaitsequence = 0;
        pev->sequence = animDesired;
        pev->frame = 0;
        ResetSequenceInfo();
        return;

    case ACT_WALK:
        if (m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished)
        {
            if (FBitSet(pev->flags, FL_DUCKING))
                strcpy(szAnim, "crouch_aim_");
            else
                strcpy(szAnim, "ref_aim_");
            strcat(szAnim, m_szAnimExtention);
            animDesired = LookupSequence(szAnim);
            if (animDesired == -1)
                animDesired = 0;
            m_Activity = ACT_WALK;
        }
        else
        {
            animDesired = pev->sequence;
        }
        break;
    }

    if (FBitSet(pev->flags, FL_DUCKING))
    {
        if (speed == 0)
            pev->gaitsequence = LookupActivity(ACT_CROUCHIDLE);
        else
            pev->gaitsequence = LookupActivity(ACT_CROUCH);
    }
    else if (speed > 190)
    {
        pev->gaitsequence = LookupActivity(ACT_RUN);
    }
    else if (speed > 0)
    {
        pev->gaitsequence = LookupActivity(ACT_WALK);
    }
    else
    {
        pev->gaitsequence = LookupSequence("deep_idle");
    }

    if (pev->sequence == animDesired)
        return;

    pev->sequence = animDesired;
    pev->frame = 0;
    ResetSequenceInfo();
}

#define AIRTIME	12		

void CBasePlayer::WaterMove()
{
    int air;

    if (pev->movetype == MOVETYPE_NOCLIP)
        return;

    if (pev->health < 0)
        return;

    if (pev->waterlevel != 3)
    {

        if (pev->air_finished < gpGlobals->time)
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM);
        else if (pev->air_finished < gpGlobals->time + 9)
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM);

        pev->air_finished = gpGlobals->time + AIRTIME;
        pev->dmg = 2;

        if (m_idrowndmg > m_idrownrestored)
        {
            m_bitsDamageType |= DMG_DROWNRECOVER;
            m_bitsDamageType &= ~DMG_DROWN;
            m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
        }

    }
    else
    {
        m_bitsDamageType &= ~DMG_DROWNRECOVER;
        m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;

        if (pev->air_finished < gpGlobals->time)
        {
            if (pev->pain_finished < gpGlobals->time)
            {
                pev->dmg += 1;
                if (pev->dmg > 5)
                    pev->dmg = 5;
                TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN);
                pev->pain_finished = gpGlobals->time + 1;

                m_idrowndmg += pev->dmg;
            }
        }
        else
        {
            m_bitsDamageType &= ~DMG_DROWN;
        }
    }

    if (!pev->waterlevel)
    {
        if (FBitSet(pev->flags, FL_INWATER))
        {
            switch (RANDOM_LONG(0, 3))
            {
            case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM); break;
            case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM); break;
            case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM); break;
            case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM); break;
            }

            ClearBits(pev->flags, FL_INWATER);
        }
        return;
    }

    air = (int)(pev->air_finished - gpGlobals->time);
    if (!RANDOM_LONG(0, 0x1f) && RANDOM_LONG(0, AIRTIME - 1) >= air)
    {
        switch (RANDOM_LONG(0, 3))
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM); break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM); break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM); break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM); break;
        }
    }

    if (pev->watertype == CONTENT_LAVA)
    {
        if (pev->dmgtime < gpGlobals->time)
            TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
    }
    else if (pev->watertype == CONTENT_SLIME)
    {
        pev->dmgtime = gpGlobals->time + 1;
        TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 4 * pev->waterlevel, DMG_ACID);
    }

    if (!FBitSet(pev->flags, FL_INWATER))
    {
        if (pev->watertype == CONTENT_WATER)
        {
            switch (RANDOM_LONG(0, 3))
            {
            case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM); break;
            case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM); break;
            case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM); break;
            case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM); break;
            }
        }

        SetBits(pev->flags, FL_INWATER);
        pev->dmgtime = 0;
    }

    if (!FBitSet(pev->flags, FL_WATERJUMP))
        pev->velocity = pev->velocity - 0.8 * pev->waterlevel * gpGlobals->frametime * pev->velocity;
}

BOOL CBasePlayer::IsOnLadder(void)
{
    return (pev->movetype == MOVETYPE_FLY);
}

void CBasePlayer::CheckWaterJump()
{
    if (IsOnLadder())
        return;

    Vector vecStart = pev->origin;
    vecStart.z += 8;

    UTIL_MakeVectors(pev->angles);
    gpGlobals->v_forward.z = 0;
    gpGlobals->v_forward = gpGlobals->v_forward.Normalize();

    Vector vecEnd = vecStart + gpGlobals->v_forward * 24;

    TraceResult tr;
    UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &tr);
    if (tr.flFraction < 1)
    {
        vecStart.z += pev->maxs.z - 8;
        vecEnd = vecStart + gpGlobals->v_forward * 24;
        pev->movedir = tr.vecPlaneNormal * -50;
        UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, ENT(pev), &tr);
        if (tr.flFraction == 1)
        {
            SetBits(pev->flags, FL_WATERJUMP);
            pev->velocity.z = 225;
            pev->teleport_time = gpGlobals->time + 2;
        }
    }
}

void CBasePlayer::PlayerDeathThink(void)
{
    if (m_iJoiningState != JOINED)
        return;

    if (m_afPhysicsFlags & PFLAG_OBSERVER)
    {
        if (m_iChaseCamMode == 0)
            GhostMode();
        else if (m_iChaseCamMode == 1)
            ChaseCamMode();
    }

    if (FBitSet(pev->flags, FL_ONGROUND))
    {
        float flForward = pev->velocity.Length() - 20;
        if (flForward <= 0)
            pev->velocity = g_vecZero;
        else
            pev->velocity = flForward * pev->velocity.Normalize();
    }

    if (HasWeapons())
    {
        PackDeadPlayerItems();
    }

    if (pev->modelindex && !m_fSequenceFinished && pev->deadflag == DEAD_DYING)
    {
        StudioFrameAdvance();

        m_iRespawnFrames += 1.0;
        if (m_iRespawnFrames < 120.0)
            return;
    }

    if (pev->deadflag == DEAD_DYING)
    {
        m_fDeadTime = gpGlobals->time;
        pev->deadflag = DEAD_DEAD;
    }

    StopAnimation();
    pev->effects |= EF_NOINTERP;
    pev->framerate = 0.0;

    int fAnyButtonDown = pev->button;

    if (g_pGameRules->IsMultiplayer()
        && (m_fDeadTime + 3.0 < gpGlobals->time)
        && !(m_afPhysicsFlags & PFLAG_OBSERVER))
    {

        pev->flags |= FL_SPECTATOR;
        StartDeathCam();

        if (m_iChaseCamMode == 1)
        {
            HintMessage("You are in ChaseCam Mode. \n"
                "Press JUMP to switch back to FreeLook mode.\n"
                " Press ATTACK to toggle teammate views.", TRUE, FALSE);
        }
        else if (m_iChaseCamMode == 0)
        {
            HintMessage("You are in FreeLook Mode. \n"
                "Press JUMP to switch back to ChaseCam mode.\n"
                " Press ATTACK to toggle teammate views.", TRUE, FALSE);
        }
    }

    if (pev->deadflag == DEAD_DEAD && m_iTeam != TEAM_UNASSIGNED)
    {
        if (fAnyButtonDown)
            return;

        if (g_pGameRules->FPlayerCanRespawn(this))
        {
            pev->deadflag = DEAD_RESPAWNABLE;

            if (g_pGameRules->IsMultiplayer())
                ((CHalfLifeMultiplay*)g_pGameRules)->CheckWinConditions();
        }

        pev->nextthink = gpGlobals->time + 0.1;
    }
    else if (pev->deadflag == DEAD_RESPAWNABLE)
    {
        respawn(pev, FALSE);
        pev->button = 0;
        m_iRespawnFrames = 0;
        pev->nextthink = -1;
    }
}

#define	PLAYER_SEARCH_RADIUS	(float)64

void CBasePlayer::PlayerUse(void)
{
    if (!((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE))
        return;

    if (m_afButtonPressed & IN_USE)
    {
        if (m_pTank != NULL)
        {
            m_pTank->Use(this, this, USE_OFF, 0);
            m_pTank = NULL;
            return;
        }
        else
        {
            if (m_afPhysicsFlags & PFLAG_ONTRAIN)
            {
                m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
                m_iTrain = TRAIN_NEW | TRAIN_OFF;
                return;
            }
            else
            {
                CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);

                if (pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev))
                {
                    m_afPhysicsFlags |= PFLAG_ONTRAIN;
                    m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
                    m_iTrain |= TRAIN_NEW;
                    EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);
                    return;
                }
            }
        }
    }

    CBaseEntity* pObject = NULL;
    CBaseEntity* pClosest = NULL;
    Vector		vecLOS;
    float flMaxDot = VIEW_FIELD_NARROW;
    float flDot;

    UTIL_MakeVectors(pev->v_angle);

    while ((pObject = UTIL_FindEntityInSphere(pObject, pev->origin, PLAYER_SEARCH_RADIUS)) != NULL)
    {

        if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
        {
            vecLOS = (VecBModelOrigin(pObject->pev) - (pev->origin + pev->view_ofs));

            vecLOS = UTIL_ClampVectorToBox(vecLOS, pObject->pev->size * 0.5);

            flDot = DotProduct(vecLOS, gpGlobals->v_forward);
            if (flDot > flMaxDot)
            {
                pClosest = pObject;
                flMaxDot = flDot;
            }
        }
    }
    pObject = pClosest;

    if (pObject)
    {
        int caps = pObject->ObjectCaps();

        if (m_afButtonPressed & IN_USE)
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

        if (((pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE)) ||
            ((m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE | FCAP_ONOFF_USE))))
        {
            if (caps & FCAP_CONTINUOUS_USE)
                m_afPhysicsFlags |= PFLAG_USING;

            pObject->Use(this, this, USE_SET, 1);
        }

        else if ((m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE))
        {
            pObject->Use(this, this, USE_SET, 0);
        }
    }
    else
    {
        if (m_afButtonPressed & IN_USE)
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
    }
}

void CBasePlayer::Jump()
{
    float flScale;

    if (FBitSet(pev->flags, FL_WATERJUMP))
        return;

    if (pev->waterlevel >= 2)
    {
        switch ((int)pev->watertype)
        {
        case CONTENT_WATER:	flScale = 100;	break;
        case CONTENT_SLIME:	flScale = 80;	break;
        default:			flScale = 50;	break;
        }

        pev->velocity.z = flScale;

        if (m_flSwimTime < gpGlobals->time)
        {
            m_flSwimTime = gpGlobals->time + 1;
            switch (RANDOM_LONG(0, 3))
            {
            case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM); break;
            case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM); break;
            case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM); break;
            case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM); break;
            }
        }
        return;
    }

    if (!FBitSet(m_afButtonPressed, IN_JUMP))
        return;

    if (!(pev->flags & FL_ONGROUND) || !pev->groundentity)
        return;

    UTIL_MakeVectors(pev->angles);

    ClearBits(pev->flags, FL_ONGROUND);

    SetAnimation(PLAYER_JUMP);

    PlayStepSound(MapTextureTypeStepType(m_chTextureType), 1.0);

    if (FBitSet(pev->flags, FL_DUCKING) || FBitSet(m_afPhysicsFlags, PFLAG_DUCKING))
    {
        if (m_fLongJump && (pev->button & IN_DUCK) && gpGlobals->time - m_flDuckTime < 1 && pev->velocity.Length() > 50)
        {
            RANDOM_LONG(0, 1);

            pev->punchangle.x = -5;
            pev->velocity = gpGlobals->v_forward * (PLAYER_LONGJUMP_SPEED * 1.6);
            pev->velocity.z = sqrt(2 * 800 * 56.0);
            SetAnimation(PLAYER_SUPERJUMP);
        }
        else
        {
            pev->velocity.z = sqrt(2 * 800 * 45.0);
        }
    }
    else
    {
        pev->velocity.z = sqrt(2 * 800 * 45.0);
    }

    entvars_t* pevGround = VARS(pev->groundentity);
    if (pevGround)
    {
        if (pevGround->flags & FL_CONVEYOR)
        {
            pev->velocity = pev->velocity + pev->basevelocity;
        }

        if (FClassnameIs(pevGround, "func_tracktrain") || FClassnameIs(pevGround, "func_train"))
        {
            pev->velocity = pev->velocity + pevGround->velocity;
        }
    }
}

void FixPlayerCrouchStuck(edict_t* pPlayer)
{
    TraceResult trace;

    for (int i = 0; i < 18; i++)
    {
        UTIL_TraceHull(pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace);
        if (trace.fStartSolid)
            pPlayer->v.origin.z++;
        else
            break;
    }
}

#define TIME_TO_DUCK	0.4

void CBasePlayer::Duck()
{
    float time, duckFraction;

    if (pev->button & IN_DUCK)
    {
        if ((m_afButtonPressed & IN_DUCK) && !(pev->flags & FL_DUCKING))
        {
            m_flDuckTime = gpGlobals->time;
            SetBits(m_afPhysicsFlags, PFLAG_DUCKING);
        }
        time = (gpGlobals->time - m_flDuckTime);

        if (FBitSet(m_afPhysicsFlags, PFLAG_DUCKING))
        {
            if (time >= TIME_TO_DUCK || !(pev->flags & FL_ONGROUND))
            {
                if (pev->flags & FL_ONGROUND)
                {
                    pev->origin = pev->origin - (VEC_DUCK_HULL_MIN - VEC_HULL_MIN);
                    FixPlayerCrouchStuck(edict());
                }

                UTIL_SetOrigin(pev, pev->origin);

                UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
                pev->view_ofs = VEC_DUCK_VIEW;
                SetBits(pev->flags, FL_DUCKING);
                ClearBits(m_afPhysicsFlags, PFLAG_DUCKING);
            }
            else
            {

                duckFraction = UTIL_SplineFraction(time, (1.0 / TIME_TO_DUCK));
                pev->view_ofs = ((VEC_DUCK_VIEW - (VEC_DUCK_HULL_MIN - VEC_HULL_MIN)) * duckFraction) + (VEC_VIEW * (1 - duckFraction));
            }
        }

        SetAnimation(PLAYER_WALK);
    }
    else
    {
        TraceResult trace;
        Vector newOrigin = pev->origin;

        if ((pev->flags & FL_ONGROUND) && !FBitSet(m_afPhysicsFlags, PFLAG_DUCKING))
            newOrigin = newOrigin + (VEC_DUCK_HULL_MIN - VEC_HULL_MIN);

        UTIL_TraceHull(newOrigin, newOrigin, dont_ignore_monsters, human_hull, ENT(pev), &trace);

        if (!trace.fStartSolid)
        {
            ClearBits(pev->flags, FL_DUCKING);
            ClearBits(m_afPhysicsFlags, PFLAG_DUCKING);
            pev->view_ofs = VEC_VIEW;
            UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
            pev->origin = newOrigin;
        }
    }
}

int  CBasePlayer::Classify(void)
{
    return CLASS_PLAYER;
}

void CBasePlayer::AddPoints(int score, BOOL bAllowNegativeScore)
{
    if (score < 0 && !bAllowNegativeScore)
    {
        if (pev->frags < 0)
            return;

        if (-score > pev->frags)
        {
            score = -pev->frags;
        }
    }

    pev->frags += score;

    MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
    WRITE_BYTE(ENTINDEX(edict()));
    WRITE_SHORT(pev->frags);
    WRITE_SHORT(m_iDeaths);
    MESSAGE_END();
}

void CBasePlayer::AddPointsToTeam(int score, BOOL bAllowNegativeScore)
{
    int index = entindex();

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);

        if (pPlayer && i != index)
        {
            if (g_pGameRules->PlayerRelationship(this, pPlayer) == GR_TEAMMATE)
            {
                pPlayer->AddPoints(score, bAllowNegativeScore);
            }
        }
    }
}

#if 0
void CBasePlayer::CheckWeapon(void)
{

    if (gpGlobals->time > m_flTimeWeaponIdle)
    {
        WeaponIdle();
    }
}
#endif

#define STEP_CONCRETE	0		
#define STEP_METAL		1		
#define STEP_DIRT		2		
#define STEP_VENT		3		
#define STEP_GRATE		4		
#define STEP_TILE		5		
#define STEP_SLOSH		6		
#define STEP_WADE		7		
#define STEP_LADDER		8		
#define STEP_SNOW		9		

void CBasePlayer::PlayStepSound(int step, float fvol)
{
    static int iSkipStep = 0;

    if (!g_pGameRules->PlayFootstepSounds(this, fvol))
        return;

    int irand = RANDOM_LONG(0, 1) + (m_iStepLeft * 2);

    m_iStepLeft = !m_iStepLeft;

    switch (step)
    {
    default:
    case STEP_CONCRETE:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_METAL:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_metal1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_metal3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_metal2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_metal4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_DIRT:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_dirt1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_dirt3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_dirt2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_dirt4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_VENT:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_duct1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_duct3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_duct2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_duct4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_GRATE:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_grate1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_grate3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_grate2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_grate4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_TILE:
        if (!RANDOM_LONG(0, 4))
            irand = 4;
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_tile1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_tile3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_tile2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_tile4.wav", fvol, ATTN_NORM);	break;
        case 4: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_tile5.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_SLOSH:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_slosh1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_slosh3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_slosh2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_slosh4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_WADE:
        if (iSkipStep == 0)
        {
            iSkipStep++;
            break;
        }

        if (iSkipStep++ == 3)
        {
            iSkipStep = 0;
        }

        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade2.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade3.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_LADDER:
        switch (irand)
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_ladder1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_ladder3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_ladder2.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_ladder4.wav", fvol, ATTN_NORM);	break;
        }
        break;
    case STEP_SNOW:

        switch (m_iStepLeft * 2 + RANDOM_LONG(0, 2))
        {
        case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow1.wav", fvol, ATTN_NORM);	break;
        case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow3.wav", fvol, ATTN_NORM);	break;
        case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow5.wav", fvol, ATTN_NORM);	break;
        case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow2.wav", fvol, ATTN_NORM);	break;
        case 4:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow4.wav", fvol, ATTN_NORM);	break;
        case 5:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_snow6.wav", fvol, ATTN_NORM);	break;
        }
        break;
    }
}

int MapTextureTypeStepType(char chTextureType)
{
    switch (chTextureType)
    {
    default:
    case CHAR_TEX_CONCRETE:	return STEP_CONCRETE;
    case CHAR_TEX_METAL: return STEP_METAL;
    case CHAR_TEX_DIRT: return STEP_DIRT;
    case CHAR_TEX_VENT: return STEP_VENT;
    case CHAR_TEX_GRATE: return STEP_GRATE;
    case CHAR_TEX_TILE: return STEP_TILE;
    case CHAR_TEX_SLOSH: return STEP_SLOSH;
    case CHAR_TEX_SNOW: return STEP_SNOW;
    }
}

void CBasePlayer::UpdateStepSound(void)
{
    int	fWalking;
    float fvol;
    char szbuffer[64];
    const char* pTextureName;
    Vector start, end;
    float rgfl1[3];
    float rgfl2[3];
    Vector knee;
    Vector feet;
    Vector center;
    float height;
    float speed;
    float velrun;
    float velwalk;
    float flduck;
    int	fLadder;
    int step;

    if (gpGlobals->time <= m_flTimeStepSound)
        return;

    if (pev->flags & FL_FROZEN)
        return;

    speed = pev->velocity.Length();

    fLadder = IsOnLadder();

    if (FBitSet(pev->flags, FL_DUCKING) || fLadder)
    {
        velwalk = 60;
        velrun = 80;
        flduck = 0.1;
    }
    else
    {
        velwalk = 100;
        velrun = 140;
        flduck = 0.0;
    }

    if ((fLadder || FBitSet(pev->flags, FL_ONGROUND)) && pev->velocity != g_vecZero
        && (speed >= velwalk || !m_flTimeStepSound))
    {
        SetAnimation(PLAYER_WALK);

        fWalking = speed < velrun;

        center = knee = feet = (pev->absmin + pev->absmax) * 0.5;
        height = pev->absmax.z - pev->absmin.z;

        knee.z = pev->absmin.z + height * 0.2;
        feet.z = pev->absmin.z;

        if (fLadder)
        {
            step = STEP_LADDER;
            fvol = 0.35;
            m_flTimeStepSound = gpGlobals->time + 0.35;
        }
        else if (UTIL_PointContents(knee) == CONTENTS_WATER)
        {
            step = STEP_WADE;
            fvol = 0.65;
            m_flTimeStepSound = gpGlobals->time + 0.6;
        }
        else if (UTIL_PointContents(feet) == CONTENTS_WATER)
        {
            step = STEP_SLOSH;
            fvol = fWalking ? 0.2 : 0.5;
            m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
        }
        else
        {
            start = end = center;
            start.z = end.z = pev->absmin.z;
            start.z += 4.0;
            end.z -= 24.0;

            start.CopyToArray(rgfl1);
            end.CopyToArray(rgfl2);

            pTextureName = TRACE_TEXTURE(ENT(pev->groundentity), rgfl1, rgfl2);
            if (pTextureName)
            {
                if (*pTextureName == '-')
                    pTextureName += 2;
                if (*pTextureName == '{' || *pTextureName == '!')
                    pTextureName++;

                if (_strnicmp(pTextureName, m_szTextureName, CBTEXTURENAMEMAX - 1))
                {
                    strcpy(szbuffer, pTextureName);
                    szbuffer[CBTEXTURENAMEMAX - 1] = 0;
                    strcpy(m_szTextureName, szbuffer);

                    m_chTextureType = TEXTURETYPE_Find(m_szTextureName);
                }
            }

            step = MapTextureTypeStepType(m_chTextureType);

            switch (m_chTextureType)
            {
            default:
            case CHAR_TEX_CONCRETE:
                fvol = fWalking ? 0.2 : 0.5;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_METAL:
                fvol = fWalking ? 0.2 : 0.5;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_DIRT:
                fvol = fWalking ? 0.25 : 0.55;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_VENT:
                fvol = fWalking ? 0.4 : 0.7;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_GRATE:
                fvol = fWalking ? 0.2 : 0.5;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_TILE:
                fvol = fWalking ? 0.2 : 0.5;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;

            case CHAR_TEX_SLOSH:
                fvol = fWalking ? 0.2 : 0.5;
                m_flTimeStepSound = fWalking ? gpGlobals->time + 0.4 : gpGlobals->time + 0.3;
                break;
            }
        }

        m_flTimeStepSound += flduck;

        if (pev->flags & FL_DUCKING)
            fvol *= 0.35;

        PlayStepSound(step, fvol);
    }
}

#define CLIMB_SHAKE_FREQUENCY	22	
#define	MAX_CLIMB_SPEED			200	
#define	CLIMB_SPEED_DEC			15	
#define	CLIMB_PUNCH_X			-7  
#define CLIMB_PUNCH_Z			7	

void CBasePlayer::PreThink(void)
{

    int buttonsChanged = (m_afButtonLast ^ pev->button);
    m_afButtonPressed = buttonsChanged & pev->button;
    m_afButtonReleased = buttonsChanged & (~pev->button);

    m_hintMessageQueue.Update(this);

    g_pGameRules->PlayerThink(this);

    if (g_fGameOver)
        return;

    if (m_iJoiningState != JOINED)
        JoiningThink();

    if (m_bMissionBriefing)
    {
        if (m_afButtonPressed & (IN_ATTACK | IN_JUMP | IN_ATTACK2))
        {
            m_afButtonPressed &= ~(IN_ATTACK | IN_JUMP | IN_ATTACK2);
            RemoveLevelText();
            m_bMissionBriefing = false;
        }
    }

    UTIL_MakeVectors(pev->v_angle);

    ItemPreFrame();
    WaterMove();

    if (m_flVelocityModifier < 1.0f)
    {
        if (FBitSet(pev->flags, FL_ONGROUND))
        {
            m_flVelocityModifier += 0.01f;
            pev->velocity = pev->velocity * m_flVelocityModifier;
        }
    }
    if (m_flVelocityModifier > 1.0f)
        m_flVelocityModifier = 1.0f;

    if (gpGlobals->time >= m_flIdleCheckTime)
    {
        m_flIdleCheckTime = gpGlobals->time + 0.2f;

        if (m_bEscaped)
            UpdatePath();

        if (pev->button)
            m_fLastMovement = gpGlobals->time;

        if (gpGlobals->time - m_fLastMovement > ((CHalfLifeMultiplay*)g_pGameRules)->m_fMaxRoundTime)
        {
            if (CVAR_GET_FLOAT("mp_autokick") != 0)
            {
                UTIL_ClientPrintAll(HUD_PRINTCONSOLE,
                    "%s has been idle for too long and has been kicked.\n",
                    STRING(pev->netname));
                SERVER_COMMAND(UTIL_VarArgs("kick %s\n", STRING(pev->netname)));
                m_fLastMovement = gpGlobals->time;
            }
        }
    }

    if (g_pGameRules && g_pGameRules->FAllowFlashlight())
        m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
    else
        m_iHideHUD |= HIDEHUD_FLASHLIGHT;

    UpdateClientData();

    CheckTimeBasedDamage();
    CheckSuitUpdate();

    if (pev->waterlevel == 2)
        CheckWaterJump();
    else if (pev->waterlevel == 0)
    {
        pev->flags &= ~FL_WATERJUMP;
    }

    if (pev->deadflag > 0 && pev->deadflag != DEAD_RESPAWNABLE)
    {
        PlayerDeathThink();
        return;
    }

    if (m_afPhysicsFlags & PFLAG_ONTRAIN)
        pev->flags |= FL_ONTRAIN;
    else
        pev->flags &= ~FL_ONTRAIN;

    if (m_afPhysicsFlags & PFLAG_ONTRAIN)
    {
        CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);
        float vel;

        if (!pTrain)
        {
            TraceResult trainTrace;
            UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -38), ignore_monsters, ENT(pev), &trainTrace);

            if (trainTrace.flFraction != 1.0 && trainTrace.pHit)
                pTrain = CBaseEntity::Instance(trainTrace.pHit);

            if (!pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(pev))
            {
                m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
                m_iTrain = TRAIN_NEW | TRAIN_OFF;
                return;
            }
        }
        else if (!FBitSet(pev->flags, FL_ONGROUND) || FBitSet(pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL) || (pev->button & (IN_MOVELEFT | IN_MOVERIGHT)))
        {
            m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
            m_iTrain = TRAIN_NEW | TRAIN_OFF;
            return;
        }

        pev->velocity = g_vecZero;
        vel = 0;
        if (m_afButtonPressed & IN_FORWARD)
        {
            vel = 1;
            pTrain->Use(this, this, USE_SET, (float)vel);
        }
        else if (m_afButtonPressed & IN_BACK)
        {
            vel = -1;
            pTrain->Use(this, this, USE_SET, (float)vel);
        }

        if (vel)
        {
            m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
            m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
        }
    }
    else if (m_iTrain & TRAIN_ACTIVE)
        m_iTrain = TRAIN_NEW;

    if (pev->button & IN_JUMP)
        Jump();

    if ((pev->button & IN_DUCK) || FBitSet(pev->flags, FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING))
        Duck();

    UpdateStepSound();

    if (!FBitSet(pev->flags, FL_ONGROUND))
        m_flFallVelocity = -pev->velocity.z;

    m_hEnemy = NULL;

    if (m_afPhysicsFlags & PFLAG_ONBARNACLE)
        pev->velocity = g_vecZero;

    if (!(m_flDisplayHistory & DHF_BUY_ZONE) && CanPlayerBuy(FALSE))
    {
        HintMessage("Press the BUY key to purchase items.", FALSE, FALSE);
        m_flDisplayHistory |= DHF_BUY_ZONE;
    }

    if (m_bHasC4)
    {
        if (!(m_flDisplayHistory & 0x1000))
        {
            m_flDisplayHistory |= 0x1000;
            HintMessage("You have the bomb!\nFind the target zone or DROP\nthe bomb for another Terrorist.", FALSE, FALSE);
        }
    }
}

void CBasePlayer::CheckTimeBasedDamage()
{
    int i;
    BYTE bDuration = 0;

    if (!(m_bitsDamageType & DMG_TIMEBASED))
        return;

    if (abs((int)(gpGlobals->time - m_tbdPrev)) < 2.0)
        return;

    m_tbdPrev = gpGlobals->time;

    for (i = 0; i < CDMG_TIMEBASED; i++)
    {
        if (m_bitsDamageType & (DMG_PARALYZE << i))
        {
            switch (i)
            {
            case itbd_Paralyze:

                bDuration = PARALYZE_DURATION;
                break;
            case itbd_NerveGas:
                bDuration = NERVEGAS_DURATION;
                break;
            case itbd_Poison:
                TakeDamage(pev, pev, POISON_DAMAGE, DMG_GENERIC);
                bDuration = POISON_DURATION;
                break;
            case itbd_Radiation:
                bDuration = RADIATION_DURATION;
                break;
            case itbd_DrownRecover:

                if (m_idrowndmg > m_idrownrestored)
                {
                    int idif = min(m_idrowndmg - m_idrownrestored, 10);

                    TakeHealth(idif, DMG_GENERIC);
                    m_idrownrestored += idif;
                }
                bDuration = 4;
                break;
            case itbd_Acid:
                bDuration = ACID_DURATION;
                break;
            case itbd_SlowBurn:
                bDuration = SLOWBURN_DURATION;
                break;
            case itbd_SlowFreeze:
                bDuration = SLOWFREEZE_DURATION;
                break;
            default:
                bDuration = 0;
            }

            if (m_rgbTimeBasedDamage[i])
            {
                if (((i == itbd_NerveGas) && (m_rgbTimeBasedDamage[i] < NERVEGAS_DURATION)) ||
                    ((i == itbd_Poison) && (m_rgbTimeBasedDamage[i] < POISON_DURATION)))
                {
                    if (m_rgItems[ITEM_ANTIDOTE])
                    {
                        m_rgbTimeBasedDamage[i] = 0;
                        m_rgItems[ITEM_ANTIDOTE]--;
                        SetSuitUpdate("!HEV_HEAL4", FALSE, SUIT_REPEAT_OK);
                    }
                }

                if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
                {
                    m_rgbTimeBasedDamage[i] = 0;
                    m_bitsDamageType &= ~(DMG_PARALYZE << i);
                }
            }
            else
            {
                m_rgbTimeBasedDamage[i] = bDuration;
            }
        }
    }
}

int gmsgGeigerRange;

#define GEIGERDELAY 0.25

void CBasePlayer::UpdateGeigerCounter(void)
{
    BYTE range;

    if (gpGlobals->time < m_flgeigerDelay)
        return;

    m_flgeigerDelay = gpGlobals->time + GEIGERDELAY;

    range = (BYTE)(m_flgeigerRange / 4);

    if (range != m_igeigerRangePrev)
    {
        m_igeigerRangePrev = range;

        MESSAGE_BEGIN(MSG_ONE, gmsgGeigerRange, NULL, ENT(pev));
        WRITE_BYTE(range);
        MESSAGE_END();
    }

    if (!RANDOM_LONG(0, 3))
        m_flgeigerRange = 1000;

}

#define SUITUPDATETIME	3.5
#define SUITFIRSTUPDATETIME 0.1

void CBasePlayer::CheckSuitUpdate()
{
    int i;
    int isentence = 0;
    int isearch = m_iSuitPlayNext;

    if (!(pev->weapons & (1 << WEAPON_SUIT)))
        return;

    UpdateGeigerCounter();

    if (g_pGameRules->IsMultiplayer())
    {
        return;
    }

    if (gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
    {
        for (i = 0; i < CSUITPLAYLIST; i++)
        {
            if (isentence = m_rgSuitPlayList[isearch])
                break;

            if (++isearch == CSUITPLAYLIST)
                isearch = 0;
        }

        if (isentence)
        {
            m_rgSuitPlayList[isearch] = 0;
            if (isentence > 0)
            {

                char sentence[CBSENTENCENAME_MAX + 1];
                strcpy(sentence, "!");
                strcat(sentence, gszallsentencenames[isentence]);
                EMIT_SOUND_SUIT(ENT(pev), sentence);
            }
            else
            {

                EMIT_GROUPID_SUIT(ENT(pev), -isentence);
            }
            m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
        }
        else
        {
            m_flSuitUpdate = 0;
        }
    }
}

void CBasePlayer::SetSuitUpdate(char* name, int fgroup, int iNoRepeatTime)
{
    int i;
    int isentence;
    int iempty = -1;

    if (!(pev->weapons & (1 << WEAPON_SUIT)))
        return;

    if (g_pGameRules->IsMultiplayer())
    {
        return;
    }

    if (!name)
    {
        for (i = 0; i < CSUITPLAYLIST; i++)
            m_rgSuitPlayList[i] = 0;
        return;
    }

    if (!fgroup)
    {
        isentence = SENTENCEG_Lookup(name, NULL);
        if (isentence < 0)
            return;
    }
    else
    {
        isentence = -SENTENCEG_GetIndex(name);
    }

    for (i = 0; i < CSUITNOREPEAT; i++)
    {
        if (isentence == m_rgiSuitNoRepeat[i])
        {
            if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time)
            {

                m_rgiSuitNoRepeat[i] = 0;
                m_rgflSuitNoRepeatTime[i] = 0.0;
                iempty = i;
                break;
            }
            else
            {
                return;
            }
        }

        if (!m_rgiSuitNoRepeat[i])
            iempty = i;
    }

    if (iNoRepeatTime)
    {
        if (iempty < 0)
            iempty = RANDOM_LONG(0, CSUITNOREPEAT - 1);
        m_rgiSuitNoRepeat[iempty] = isentence;
        m_rgflSuitNoRepeatTime[iempty] = iNoRepeatTime + gpGlobals->time;
    }

    m_rgSuitPlayList[m_iSuitPlayNext++] = isentence;
    if (m_iSuitPlayNext == CSUITPLAYLIST)
        m_iSuitPlayNext = 0;

    if (m_flSuitUpdate <= gpGlobals->time)
    {
        if (m_flSuitUpdate == 0)
            m_flSuitUpdate = gpGlobals->time + SUITFIRSTUPDATETIME;
        else
            m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
    }

}

static void
CheckPowerups(entvars_t* pev)
{
    if (pev->health <= 0)
        return;

    pev->modelindex = g_ulModelIndexPlayer;
}

void CBasePlayer::UpdatePlayerSound(void)
{
    int iBodyVolume;
    int iVolume;
    CSound* pSound;

    pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));

    if (!pSound)
    {
        ALERT(at_console, "Client lost reserved sound!\n");
        return;
    }

    pSound->m_iType = bits_SOUND_NONE;

    if (FBitSet(pev->flags, FL_ONGROUND))
    {
        iBodyVolume = pev->velocity.Length();

        if (iBodyVolume > 512)
        {
            iBodyVolume = 512;
        }
    }
    else
    {
        iBodyVolume = 0;
    }

    if (pev->button & IN_JUMP)
    {
        iBodyVolume += 100;
    }

    if (m_iWeaponVolume > iBodyVolume)
    {
        m_iTargetVolume = m_iWeaponVolume;

        pSound->m_iType |= bits_SOUND_COMBAT;
    }
    else
    {
        m_iTargetVolume = iBodyVolume;
    }

    m_iWeaponVolume -= 250 * gpGlobals->frametime;

    iVolume = pSound->m_iVolume;

    if (m_iTargetVolume > iVolume)
    {
        iVolume = m_iTargetVolume;
    }
    else if (iVolume > m_iTargetVolume)
    {
        iVolume -= 250 * gpGlobals->frametime;

        if (iVolume < m_iTargetVolume)
        {
            iVolume = 0;
        }
    }

    if (m_fNoPlayerSound)
    {
        iVolume = 0;
    }

    if (gpGlobals->time > m_flStopExtraSoundTime)
    {
        m_iExtraSoundTypes = 0;
    }

    pSound->m_vecOrigin = pev->origin;
    pSound->m_iType |= (bits_SOUND_PLAYER | m_iExtraSoundTypes);
    pSound->m_iVolume = iVolume;

    m_iWeaponFlash -= 256 * gpGlobals->frametime;
    if (m_iWeaponFlash < 0)
        m_iWeaponFlash = 0;

    UTIL_MakeVectors(pev->angles);
    gpGlobals->v_forward.z = 0;
}

void CBasePlayer::PostThink()
{
    if (g_fGameOver)
        return;

    if (!IsAlive())
        return;

    if (m_pTank != NULL)
    {
        if (m_pTank->OnControls(pev) && !pev->weaponmodel)
        {
            m_pTank->Use(this, this, USE_SET, 2);
        }
        else
        {
            m_pTank->Use(this, this, USE_OFF, 0);
            m_pTank = NULL;
        }
    }

    ItemPostFrame();

    if ((FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
    {
        float fvol = 0.5;

        if (pev->watertype == CONTENT_WATER)
        {

        }
        else if (m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
        {
            switch (RANDOM_LONG(0, 1))
            {
            case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_fallpain2.wav", 1, ATTN_NORM);
            case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM);
            }

            float flFallDamage = g_pGameRules->FlPlayerFallDamage(this);

            if (flFallDamage > pev->health)
            {
                EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM);
            }

            if (flFallDamage > 0)
            {
                TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL);
            }

            fvol = 1.0;
        }
        else if (m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2)
        {
            fvol = 0.85;
        }
        else if (m_flFallVelocity < PLAYER_MIN_BOUNCE_SPEED)
        {
            fvol = 0;
        }

        if (fvol > 0.0)
        {
            m_flTimeStepSound = 0;
            UpdateStepSound();

            PlayStepSound(MapTextureTypeStepType(m_chTextureType), fvol);

            pev->punchangle.x = m_flFallVelocity * 0.018;

            if (pev->punchangle.x > 8)
            {
                pev->punchangle.x = 8;
            }

            if (RANDOM_LONG(0, 1))
            {
                pev->punchangle.z = m_flFallVelocity * 0.009;
            }

        }

        if (IsAlive())
        {
            SetAnimation(PLAYER_WALK);
        }
    }

    if (FBitSet(pev->flags, FL_ONGROUND))
    {
        if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
        {
            CSoundEnt::InsertSound(bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2);

        }
        m_flFallVelocity = 0;
    }

    if (IsAlive())
    {
        if (!pev->velocity.x && !pev->velocity.y)
            SetAnimation(PLAYER_IDLE);
        else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
            SetAnimation(PLAYER_WALK);
        else if (pev->waterlevel > 1)
            SetAnimation(PLAYER_WALK);
    }

    StudioFrameAdvance();
    CheckPowerups(pev);

    UpdatePlayerSound();

    m_afButtonLast = pev->button;
}

BOOL IsSpawnPointValid(CBaseEntity* pPlayer, CBaseEntity* pSpot)
{
    CBaseEntity* pEntity = NULL;

    if (!pSpot->IsTriggered(pPlayer))
    {
        return FALSE;
    }

    while ((pEntity = UTIL_FindEntityInSphere(pEntity, pSpot->pev->origin, 64)) != NULL)
    {
        if (pEntity->IsPlayer() && pEntity != pPlayer)
            return FALSE;
    }

    return TRUE;
}

DLL_GLOBAL CBaseEntity* g_pLastSpawn;
CBaseEntity* g_pLastCTSpawn;
CBaseEntity* g_pLastTerroristSpawn;
inline int FNullEnt(CBaseEntity* ent) { return (ent == NULL) || FNullEnt(ent->edict()); }

edict_t* EntSelectSpawnPoint(CBaseEntity* pPlayer)
{
    CBaseEntity* pSpot;
    edict_t* player;

    player = pPlayer->edict();

    if (g_pGameRules->IsCoOp())
    {
        pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_coop");
        if (!FNullEnt(pSpot))
            goto ReturnSpot;

        pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_start");
        if (!FNullEnt(pSpot))
            goto ReturnSpot;
    }
    else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer*)pPlayer)->m_bIsVIP)
    {
        ALERT(at_console, "Looking for a VIP spawn point\n");

        pSpot = UTIL_FindEntityByClassname(NULL, "info_vip_start");

        if (!FNullEnt(pSpot))
            goto ReturnSpot;

        goto CTSpawn;
    }
    else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer*)pPlayer)->m_iTeam == TEAM_CT)
    {
    CTSpawn:
        ALERT(at_console, "Looking for a CT spawn point\n");

        pSpot = UTIL_FindEntityByClassname(g_pLastCTSpawn, "info_player_start");

        if (FNullEnt(pSpot))
            pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_start");

        CBaseEntity* pFirstSpot = pSpot;

        do
        {
            if (pSpot)
            {
                if (IsSpawnPointValid(pPlayer, pSpot))
                {
                    if (pSpot->pev->origin == Vector(0, 0, 0))
                    {
                        pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_start");
                        continue;
                    }

                    goto ReturnSpot;
                }
            }

            pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_start");
        } while (pSpot != pFirstSpot);

        if (!FNullEnt(pSpot))
        {
            CBaseEntity* pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pSpot->pev->origin, 64)) != NULL)
            {
                if (pEntity->IsPlayer() && pEntity->edict() != player)
                    pEntity->TakeDamage(VARS(INDEXENT(0)), VARS(INDEXENT(0)), 200, DMG_GENERIC);
            }
            goto ReturnSpot;
        }
    }
    else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer*)pPlayer)->m_iTeam == TEAM_TERRORIST)
    {
        ALERT(at_console, "Looking for a TERRORIST spawn point\n");

        pSpot = UTIL_FindEntityByClassname(g_pLastTerroristSpawn, "info_player_deathmatch");

        if (FNullEnt(pSpot))
            pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");

        CBaseEntity* pFirstSpot = pSpot;

        do
        {
            if (pSpot)
            {
                if (IsSpawnPointValid(pPlayer, pSpot))
                {
                    if (pSpot->pev->origin == Vector(0, 0, 0))
                    {
                        pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");
                        continue;
                    }

                    goto ReturnSpot;
                }
            }

            pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");
        } while (pSpot != pFirstSpot);

        if (!FNullEnt(pSpot))
        {
            CBaseEntity* pEntity = NULL;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pSpot->pev->origin, 64)) != NULL)
            {
                if (pEntity->IsPlayer() && pEntity->edict() != player)
                    pEntity->TakeDamage(VARS(INDEXENT(0)), VARS(INDEXENT(0)), 200, DMG_GENERIC);
            }
            goto ReturnSpot;
        }
    }

    if (FStringNull(gpGlobals->startspot) || !strlen(STRING(gpGlobals->startspot)))
    {
        pSpot = UTIL_FindEntityByClassname(NULL, "info_player_deathmatch");
        if (!FNullEnt(pSpot))
            goto ReturnSpot;
    }
    else
    {
        pSpot = UTIL_FindEntityByTargetname(NULL, STRING(gpGlobals->startspot));
        if (!FNullEnt(pSpot))
            goto ReturnSpot;
    }

ReturnSpot:
    if (FNullEnt(pSpot))
    {
        ALERT(at_error, "PutClientInServer: no info_player_start on level");
        return INDEXENT(0);
    }

    if (((CBasePlayer*)pPlayer)->m_iTeam == TEAM_TERRORIST)
        g_pLastTerroristSpawn = pSpot;
    else
        g_pLastCTSpawn = pSpot;

    return pSpot->edict();
}

CBasePlayer::CBasePlayer()
{
    m_iSignalsPending = 0;
    m_iSignals = 0;
}

void CBasePlayer::Spawn(void)
{
    pev->classname = MAKE_STRING("player");
    pev->health = 100;

    if (!m_bNotKilled)
    {
        pev->armorvalue = 0;
        m_iKevlar = ARMOR_NONE;
    }

    pev->takedamage = DAMAGE_AIM;
    pev->solid = SOLID_SLIDEBOX;
    pev->movetype = MOVETYPE_WALK;
    pev->max_health = pev->health;
    pev->flags = FL_CLIENT;
    pev->air_finished = gpGlobals->time + 12;
    pev->dmg = 2;
    pev->effects = 0;
    pev->deadflag = DEAD_NO;
    pev->dmg_take = 0;
    pev->dmg_save = 0;

    m_bitsHUDDamage = -1;
    m_bitsDamageType = 0;
    m_afPhysicsFlags = 0;
    m_fLongJump = FALSE;
    m_iClientFOV = -1;
    m_bHasAssaultSuit = 0;
    m_pentCurBombTarget = NULL;

    m_hintMessageQueue.Reset();

    m_iLastZoom = DEFAULT_FOV;
    m_iRadioMessages = 60;
    m_bHasC4 = false;
    m_bKilledByBomb = false;
    m_bKilledByGrenade = false;
    m_flDisplayHistory &= 0xFFFFEBBD;
    m_iChaseTarget = 1;
    m_bEscaped = false;
    m_bHasEscaped = false;

    m_flVelocityModifier = 1.0;
    m_flLastFire = 0;
    m_flAccuracy = 0.8f;
    m_flBurstShootTime = 0;
    m_flLastTalk = 0;
    m_flIdleCheckTime = 0;
    m_flRadioTime = 0;
    m_tmHandleSignals = 0;
    m_fCamSwitch = 0;

    m_tmNextRadarUpdate = gpGlobals->time;
    m_vLastOrigin = Vector(0, 0, 0);
    m_iCurrentKickVote = 0;
    m_flNextVoteTime = 0;
    m_bTKPunished = false;

    InitStatusBar();

    m_iNumSpawns++;

    for (int i = 0; i < MAX_RECENT_PATH; i++)
    {
        m_vRecentPath[i] = Vector(0, 0, 0);
    }
    m_vLastPathPos = Vector(0, 0, 0);

    if (m_pActiveItem && !pev->viewmodel)
    {
        if (m_bLeftHanded == 1)
        {
            switch (((CBasePlayerWeapon*)m_pActiveItem)->m_iId)
            {
            case WEAPON_AWP:
                pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
                break;
            case WEAPON_G3SG1:
                pev->viewmodel = MAKE_STRING("models/v_g3sg1.mdl");
                break;
            case WEAPON_SCOUT:
                pev->viewmodel = MAKE_STRING("models/v_scout.mdl");
                break;
            }
        }
        else
        {
            switch (((CBasePlayerWeapon*)m_pActiveItem)->m_iId)
            {
            case WEAPON_AWP:
                pev->viewmodel = MAKE_STRING("models/v_awp_r.mdl");
                break;
            case WEAPON_G3SG1:
                pev->viewmodel = MAKE_STRING("models/v_g3sg1_r.mdl");
                break;
            case WEAPON_SCOUT:
                pev->viewmodel = MAKE_STRING("models/v_scout_r.mdl");
                break;
            }
        }
    }

    m_iFOV = DEFAULT_FOV;
    m_flNextDecalTime = 0;
    m_iStepLeft = 0;
    m_bloodColor = BLOOD_COLOR_RED;
    m_flgeigerDelay = gpGlobals->time + 2.0;
    m_flTimeStepSound = 0;
    m_flFieldOfView = 0.5;
    m_flNextAttack = gpGlobals->time;

    StartSneaking();

    m_iFlashBattery = 99;
    m_flFlashLightTime = 1;

    pev->body = (m_bHasDefuser) ? 1 : 0;

    if (m_bMissionBriefing)
    {
        RemoveLevelText();
        m_bMissionBriefing = false;
    }

    m_flFallVelocity = 0;

    g_pGameRules->GetPlayerSpawnSpot(this);

    SET_MODEL(ENT(pev), "models/player.mdl");
    g_ulModelIndexPlayer = pev->modelindex;
    pev->sequence = LookupActivity(ACT_IDLE);

    if (FBitSet(pev->flags, FL_DUCKING))
        UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
    else
        UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

    pev->view_ofs = VEC_VIEW;
    Precache();
    m_HackedGunPos = Vector(0, 32, 0);

    if (m_iPlayerSound == SOUNDLIST_EMPTY)
    {
        ALERT(at_console, "Couldn't alloc player sound slot!\n");
    }

    m_fNoPlayerSound = FALSE;
    m_pLastItem = NULL;
    m_fWeapon = FALSE;
    m_pClientActiveItem = NULL;
    m_iClientBattery = -1;
    m_fInitHUD = TRUE;
    m_iHideHUD &= ~(HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);

    if (!m_bNotKilled)
    {
        m_iClientHideHUD = -1;

        for (int i = 0; i < MAX_AMMO_SLOTS; i++)
        {
            m_rgAmmo[i] = 0;
            m_rgAmmoLast[i] = 0;
        }

        m_bHasPrimary = false;
        m_bHasNightVision = false;
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgNVGToggle, NULL, ENT(pev));
    WRITE_BYTE(0);
    MESSAGE_END();
    m_bNightVisionOn = false;

    CLIENT_COMMAND(edict(), "lambert 1.5\n");

    m_lasty = 0;
    m_lastx = 0;

    g_pGameRules->PlayerSpawn(this);

    m_bNotKilled = true;

    if (m_bBeingKicked == 1)
    {
        pev->health = 0;
        Killed(pev, GIB_NORMAL);
    }

    ResetMaxSpeed();

    if (m_bIsVIP)
    {
        m_iKevlar = ARMOR_VESTHELM;
        pev->armorvalue = 200;
        HintMessage("You are the VIP\n Make your way to the safety zones!\n", TRUE, FALSE);
        RemoveAllItems(FALSE);
        GiveNamedItem("weapon_knife");
    }

    SetScoreboardAttributes(NULL);

    MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo, NULL);
    WRITE_BYTE(ENTINDEX(edict()));
    WRITE_BYTE(m_iTeam);
    MESSAGE_END();
}

void CBasePlayer::Precache(void)
{
    if (WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet)
    {
        if (!WorldGraph.FSetGraphPointers())
        {
            ALERT(at_console, "**Graph pointers were not set!\n");
        }
        else
        {
            ALERT(at_console, "**Graph Pointers Set!\n");
        }
    }

    m_flgeigerRange = 1000;
    m_igeigerRangePrev = 1000;

    m_bitsDamageType = 0;
    m_bitsHUDDamage = -1;

    m_iClientBattery = -1;

    m_iTrain = TRAIN_NEW;

    gmsgSelAmmo = REG_USER_MSG("SelAmmo", sizeof(SelAmmo));
    gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
    gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
    gmsgFlashlight = REG_USER_MSG("Flashlight", 2);
    gmsgFlashBattery = REG_USER_MSG("FlashBat", 1);
    gmsgHealth = REG_USER_MSG("Health", 1);
    gmsgDamage = REG_USER_MSG("Damage", 12);
    gmsgBattery = REG_USER_MSG("Battery", 2);
    gmsgTrain = REG_USER_MSG("Train", 1);
    gmsgHudText = REG_USER_MSG("HudText", -1);
    gmsgSayText = REG_USER_MSG("SayText", -1);
    gmsgTextMsg = REG_USER_MSG("TextMsg", -1);
    gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
    gmsgResetHUD = REG_USER_MSG("ResetHUD", 0);
    gmsgInitHUD = REG_USER_MSG("InitHUD", 0);
    gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
    gmsgDeathMsg = REG_USER_MSG("DeathMsg", -1);
    gmsgScoreAttrib = REG_USER_MSG("ScoreAttrib", 2);
    gmsgScoreInfo = REG_USER_MSG("ScoreInfo", 5);
    gmsgTeamInfo = REG_USER_MSG("TeamInfo", -1);
    gmsgTeamScore = REG_USER_MSG("TeamScore", -1);
    gmsgGameMode = REG_USER_MSG("GameMode", 1);
    gmsgMOTD = REG_USER_MSG("MOTD", -1);
    gmsgAmmoPickup = REG_USER_MSG("AmmoPickup", 2);
    gmsgWeapPickup = REG_USER_MSG("WeapPickup", 1);
    gmsgItemPickup = REG_USER_MSG("ItemPickup", -1);
    gmsgHideWeapon = REG_USER_MSG("HideWeapon", 1);
    gmsgSetFOV = REG_USER_MSG("SetFOV", 1);
    gmsgShowMenu = REG_USER_MSG("ShowMenu", -1);
    gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
    gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
    gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
    gmsgSendAudio = REG_USER_MSG("SendAudio", -1);
    gmsgRoundTime = REG_USER_MSG("RoundTime", 2);
    gmsgMoney = REG_USER_MSG("Money", 5);
    gmsgArmorType = REG_USER_MSG("ArmorType", 1);
    gmsgBlinkAcct = REG_USER_MSG("BlinkAcct", 1);
    gmsgStatusValue = REG_USER_MSG("StatusValue", -1);
    gmsgStatusText = REG_USER_MSG("StatusText", -1);
    gmsgStatusIcon = REG_USER_MSG("StatusIcon", -1);
    gmsgBarTime = REG_USER_MSG("BarTime", -1);
    gmsgReloadSound = REG_USER_MSG("ReloadSound", 2);
    gmsgCrosshair = REG_USER_MSG("Crosshair", 1);
    gmsgNVGToggle = REG_USER_MSG("NVGToggle", 1);
    gmsgRadar = REG_USER_MSG("Radar", 7);

    m_iUpdateTime = 5;

    if (gInitHUD)
        m_fInitHUD = TRUE;
}

int CBasePlayer::Save(CSave& save)
{
    if (!CBaseMonster::Save(save))
        return 0;

    return save.WriteFields("PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData));
}

void CBasePlayer::RenewItems(void)
{

}

int CBasePlayer::Restore(CRestore& restore)
{
    if (!CBaseMonster::Restore(restore))
        return 0;

    int status = restore.ReadFields("PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData));

    SAVERESTOREDATA* pSaveData = (SAVERESTOREDATA*)gpGlobals->pSaveData;

    if (!pSaveData->fUseLandmark)
    {
        ALERT(at_console, "No Landmark:%s\n", pSaveData->szLandmarkName);

        edict_t* pentSpawnSpot = EntSelectSpawnPoint(this);
        pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
        pev->angles = VARS(pentSpawnSpot)->angles;
    }
    pev->v_angle.z = 0;
    pev->angles = pev->v_angle;

    pev->fixangle = TRUE;

    m_bloodColor = BLOOD_COLOR_RED;

    g_ulModelIndexPlayer = pev->modelindex;

    if (FBitSet(pev->flags, FL_DUCKING))
    {
        FixPlayerCrouchStuck(edict());
        UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
    }
    else
    {
        UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
    }

    RenewItems();

    m_flDisplayHistory &= ~DHM_CONNECT_CLEAR;
    SetScoreboardAttributes(NULL);

    return status;
}

void CBasePlayer::SelectNextItem(int iItem)
{
    CBasePlayerItem* pItem;

    pItem = m_rgpPlayerItems[iItem];

    if (!pItem)
        return;

    if (pItem == m_pActiveItem)
    {
        pItem = m_pActiveItem->m_pNext;
        if (!pItem)
        {
            return;
        }

        CBasePlayerItem* pLast;
        pLast = pItem;
        while (pLast->m_pNext)
            pLast = pLast->m_pNext;

        pLast->m_pNext = m_pActiveItem;
        m_pActiveItem->m_pNext = NULL;
        m_rgpPlayerItems[iItem] = pItem;
    }

    ResetAutoaim();

    if (m_pActiveItem)
    {
        m_pActiveItem->Holster();
    }

    m_pActiveItem = pItem;

    m_pActiveItem->Deploy();
    m_pActiveItem->UpdateItemInfo();

    ResetMaxSpeed();
}

void CBasePlayer::SelectItem(const char* pstr)
{
    if (!pstr)
        return;

    CBasePlayerItem* pItem = NULL;

    for (int i = 0; i < MAX_ITEM_TYPES; i++)
    {
        if (m_rgpPlayerItems[i])
        {
            pItem = m_rgpPlayerItems[i];

            while (pItem)
            {
                if (FClassnameIs(pItem->pev, pstr))
                    break;
                pItem = pItem->m_pNext;
            }
        }

        if (pItem)
            break;
    }

    if (!pItem)
        return;

    if (pItem == m_pActiveItem)
        return;

    ResetAutoaim();

    if (m_pActiveItem)
        m_pActiveItem->Holster();

    m_pLastItem = m_pActiveItem;
    m_pActiveItem = pItem;

    if (m_pActiveItem)
    {
        m_pActiveItem->Deploy();
        m_pActiveItem->UpdateItemInfo();
        ResetMaxSpeed();
    }
}

void CBasePlayer::SelectLastItem(void)
{
    if (!m_pLastItem)
    {
        return;
    }

    if (m_pActiveItem && !m_pActiveItem->CanHolster())
    {
        return;
    }

    ResetAutoaim();

    if (m_pActiveItem)
        m_pActiveItem->Holster();

    CBasePlayerItem* pTemp = m_pActiveItem;
    m_pActiveItem = m_pLastItem;
    m_pLastItem = pTemp;
    m_pActiveItem->Deploy();
    m_pActiveItem->UpdateItemInfo();

    ResetMaxSpeed();
}

BOOL CBasePlayer::HasWeapons(void)
{
    int i;

    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        if (m_rgpPlayerItems[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CBasePlayer::SelectPrevItem(int iItem)
{
}

const char* CBasePlayer::TeamID(void)
{
    if (pev == NULL)
        return "";

    return m_szTeamName;
}

class CSprayCan : public CBaseEntity
{
public:
    void	Spawn(entvars_t* pevOwner);
    void	Think(void);

    virtual int	ObjectCaps(void) { return FCAP_DONT_SAVE; }
};

void CSprayCan::Spawn(entvars_t* pevOwner)
{

    pev->origin = pevOwner->origin + Vector(0, 0, 32);
    pev->angles = pevOwner->v_angle;
    pev->owner = ENT(pevOwner);
    pev->frame = 0;

    pev->nextthink = gpGlobals->time + 0.1;
    EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/sprayer.wav", 1, ATTN_NORM);
}

void CSprayCan::Think(void)
{
    TraceResult	tr;
    int playernum;
    int nFrames;
    CBasePlayer* pPlayer;

    pPlayer = (CBasePlayer*)GET_PRIVATE(pev->owner);

    if (pPlayer)
        nFrames = pPlayer->GetCustomDecalFrames();
    else
        nFrames = -1;

    playernum = ENTINDEX(pev->owner);

    UTIL_MakeVectors(pev->angles);
    UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr);

    if (nFrames == -1)
    {
        UTIL_DecalTrace(&tr, DECAL_LAMBDA6);
        UTIL_Remove(this);
    }
    else
    {
        UTIL_PlayerDecalTrace(&tr, playernum, pev->frame, TRUE);

        if (pev->frame++ >= (nFrames - 1))
            UTIL_Remove(this);
    }

    pev->nextthink = gpGlobals->time + 0.1;
}

class	CBloodSplat : public CBaseEntity
{
public:
    void	Spawn(entvars_t* pevOwner);
    void	EXPORT Spray(void);
};

void CBloodSplat::Spawn(entvars_t* pevOwner)
{
    pev->origin = pevOwner->origin + Vector(0, 0, 32);
    pev->angles = pevOwner->v_angle;
    pev->owner = ENT(pevOwner);

    SetThink(&CBloodSplat::Spray);
    pev->nextthink = gpGlobals->time + 0.1;
}

void CBloodSplat::Spray(void)
{
    TraceResult	tr;

    if (g_Language != LANGUAGE_GERMAN)
    {
        UTIL_MakeVectors(pev->angles);
        UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr);

        UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
    }
    SetThink(&CBaseEntity::SUB_Remove);
    pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayer::GiveNamedItem(const char* pszName)
{
    string_t istr = MAKE_STRING(pszName);

    edict_t* pent = CREATE_NAMED_ENTITY(istr);
    if (FNullEnt(pent))
    {
        ALERT(at_console, "NULL Ent in GiveNamedItem!\n");
        return;
    }
    VARS(pent)->origin = pev->origin;
    pent->v.spawnflags |= SF_NORESPAWN;

    DispatchSpawn(pent);
    DispatchTouch(pent, ENT(pev));

    if (!strcmp(pszName, "item_assaultsuit"))
    {
        m_bHasAssaultSuit = 1;
    }
    else if (!strcmp(pszName, "item_kevlar"))
    {
        m_bHasAssaultSuit = 0;
    }
}

CBaseEntity* FindEntityForward(CBaseEntity* pMe)
{
    TraceResult tr;

    UTIL_MakeVectors(pMe->pev->v_angle);
    UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs, pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192, dont_ignore_monsters, pMe->edict(), &tr);
    if (tr.flFraction != 1.0 && !FNullEnt(tr.pHit))
    {
        CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
        return pHit;
    }
    return NULL;
}

BOOL CBasePlayer::FlashlightIsOn(void)
{
    return FBitSet(pev->effects, EF_DIMLIGHT);
}

void CBasePlayer::FlashlightTurnOn(void)
{
    if (!g_pGameRules->FAllowFlashlight())
    {
        return;
    }

    if ((pev->weapons & (1 << WEAPON_SUIT)))
    {
        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_ON, 1.0, ATTN_NORM, 0, PITCH_NORM);
        SetBits(pev->effects, EF_DIMLIGHT);
        MESSAGE_BEGIN(MSG_ONE, gmsgFlashlight, NULL, ENT(pev));
        WRITE_BYTE(1);
        WRITE_BYTE(m_iFlashBattery);
        MESSAGE_END();

        m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
    }
}

void CBasePlayer::FlashlightTurnOff(void)
{
    EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM);
    ClearBits(pev->effects, EF_DIMLIGHT);
    MESSAGE_BEGIN(MSG_ONE, gmsgFlashlight, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_BYTE(m_iFlashBattery);
    MESSAGE_END();

    m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
}

void CBasePlayer::ForceClientDllUpdate(void)
{
    m_iClientHealth = -1;
    m_iClientBattery = -1;
    m_iTrain |= TRAIN_NEW;
    m_fWeapon = FALSE;
    m_fKnownItem = FALSE;

    UpdateClientData();
}

extern float g_flWeaponCheat;

void CBasePlayer::ImpulseCommands()
{
    TraceResult	tr;

    PlayerUse();

    int iImpulse = (int)pev->impulse;
    switch (iImpulse)
    {
    case 99:
    {
        int iOn;

        if (!gmsgLogo)
        {
            iOn = 1;
            gmsgLogo = REG_USER_MSG("Logo", 1);
        }
        else
        {
            iOn = 0;
        }

        ASSERT(gmsgLogo > 0);

        MESSAGE_BEGIN(MSG_ONE, gmsgLogo, NULL, ENT(pev));
        WRITE_BYTE(iOn);
        MESSAGE_END();

        if (!iOn)
            gmsgLogo = 0;
        break;
    }
    case 100:
        if (FlashlightIsOn())
        {
            FlashlightTurnOff();
        }
        else
        {
            FlashlightTurnOn();
        }
        break;
    case 201:

        if (gpGlobals->time < m_flNextDecalTime)
        {
            break;
        }

        UTIL_MakeVectors(pev->v_angle);
        UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);

        if (tr.flFraction != 1.0)
        {
            m_flNextDecalTime = gpGlobals->time + CVAR_GET_FLOAT("decalfrequency");
            CSprayCan* pCan = GetClassPtr((CSprayCan*)NULL);
            pCan->Spawn(pev);
        }

        break;
    case 204:
        ForceClientDllUpdate();
        break;
    default:
        CheatImpulseCommands(iImpulse);
        break;
    }

    pev->impulse = 0;
}

void CBasePlayer::CheatImpulseCommands(int iImpulse)
{
#if !defined( HLDEMO_BUILD )
    if (g_flWeaponCheat == 0.0)
    {
        return;
    }

    CBaseEntity* pEntity;
    TraceResult tr;

    switch (iImpulse)
    {
    case 76:
    {
        if (!giPrecacheGrunt)
        {
            giPrecacheGrunt = 1;
            ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
        }
        else
        {
            UTIL_MakeVectors(Vector(0, pev->v_angle.y, 0));
            Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
        }
        break;
    }

    case 101:
        gEvilImpulse101 = TRUE;
        AddAccount(16000, true);
        ALERT(at_console, "Crediting %s with $16000\n", STRING(pev->netname));
        break;

    case 102:
        CGib::SpawnRandomGibs(pev, 1, 1);
        break;

    case 103:
        pEntity = FindEntityForward(this);
        if (pEntity)
        {
            CBaseMonster* pMonster = pEntity->MyMonsterPointer();
            if (pMonster)
                pMonster->ReportAIState();
        }
        break;

    case 104:
        gGlobalState.DumpGlobals();
        break;

    case 105:
    {
        if (m_fNoPlayerSound)
        {
            ALERT(at_console, "Player is audible\n");
            m_fNoPlayerSound = FALSE;
        }
        else
        {
            ALERT(at_console, "Player is silent\n");
            m_fNoPlayerSound = TRUE;
        }
        break;
    }

    case 106:
        pEntity = FindEntityForward(this);
        if (pEntity)
        {
            ALERT(at_console, "Classname: %s", STRING(pEntity->pev->classname));

            if (!FStringNull(pEntity->pev->targetname))
            {
                ALERT(at_console, " - Targetname: %s\n", STRING(pEntity->pev->targetname));
            }
            else
            {
                ALERT(at_console, " - TargetName: No Targetname\n");
            }

            ALERT(at_console, "Model: %s\n", STRING(pEntity->pev->model));
            if (pEntity->pev->globalname)
                ALERT(at_console, "Globalname: %s\n", STRING(pEntity->pev->globalname));
        }
        break;

    case 107:
    {
        TraceResult tr;

        edict_t* pWorld = g_engfuncs.pfnPEntityOfEntIndex(0);

        Vector start = pev->origin + pev->view_ofs;
        Vector end = start + gpGlobals->v_forward * 1024;
        UTIL_TraceLine(start, end, ignore_monsters, edict(), &tr);
        if (tr.pHit)
            pWorld = tr.pHit;
        const char* pTextureName = TRACE_TEXTURE(pWorld, start, end);
        if (pTextureName)
            ALERT(at_console, "Texture: %s\n", pTextureName);
    }
    break;
    case 195:
    {
        Create("node_viewer_fly", pev->origin, pev->angles);
    }
    break;
    case 196:
    {
        Create("node_viewer_large", pev->origin, pev->angles);
    }
    break;
    case 197:
    {
        Create("node_viewer_human", pev->origin, pev->angles);
    }
    break;
    case 199:
    {
        ALERT(at_console, "%d\n", WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
        WorldGraph.ShowNodeConnections(WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
    }
    break;
    case 202:
        UTIL_MakeVectors(pev->v_angle);
        UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);

        if (tr.flFraction != 1.0)
        {
            CBloodSplat* pBlood = GetClassPtr((CBloodSplat*)NULL);
            pBlood->Spawn(pev);
        }
        break;
    case 203:
        pEntity = FindEntityForward(this);
        if (pEntity)
        {
            if (pEntity->pev->takedamage)
                pEntity->SetThink(&CBaseEntity::SUB_Remove);
        }
        break;
    }
#endif	
}

int CBasePlayer::AddPlayerItem(CBasePlayerItem* pItem)
{
    CBasePlayerItem* pInsert;

    pInsert = m_rgpPlayerItems[pItem->iItemSlot()];

    while (pInsert)
    {
        if (FClassnameIs(pInsert->pev, STRING(pItem->pev->classname)))
        {
            if (pItem->AddDuplicate(pInsert))
            {
                g_pGameRules->PlayerGotWeapon(this, pItem);
                pItem->CheckRespawn();

                pInsert->UpdateItemInfo();
                if (m_pActiveItem)
                    m_pActiveItem->UpdateItemInfo();

                pItem->Kill();
            }
            else if (gEvilImpulse101)
            {
                pItem->Kill();
            }
            return FALSE;
        }
        pInsert = pInsert->m_pNext;
    }

    if (pItem->AddToPlayer(this))
    {
        g_pGameRules->PlayerGotWeapon(this, pItem);

        if (pItem->iItemSlot() == 1)
            m_bHasPrimary = true;

        pItem->CheckRespawn();

        pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()];
        m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

        if (g_pGameRules->FShouldSwitchWeapon(this, pItem))
        {
            SwitchWeapon(pItem);
        }

        return TRUE;
    }
    else if (gEvilImpulse101)
    {
        pItem->Kill();
    }
    return FALSE;
}

int CBasePlayer::RemovePlayerItem(CBasePlayerItem* pItem)
{
    if (m_pActiveItem == pItem)
    {
        ResetAutoaim();

        pItem->pev->nextthink = 0;
        pItem->SetThink(NULL);
        m_pActiveItem = NULL;

        pev->viewmodel = 0;
        pev->weaponmodel = 0;
    }
    else if (m_pLastItem == pItem)
        m_pLastItem = NULL;

    CBasePlayerItem* pPrev = m_rgpPlayerItems[pItem->iItemSlot()];

    if (pPrev == pItem)
    {
        m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
        return TRUE;
    }
    else
    {
        while (pPrev && pPrev->m_pNext != pItem)
        {
            pPrev = pPrev->m_pNext;
        }
        if (pPrev)
        {
            pPrev->m_pNext = pItem->m_pNext;
            return TRUE;
        }
    }
    return FALSE;
}

int CBasePlayer::GiveAmmo(int iCount, char* szName, int iMax)
{
    if (pev->flags == FL_SPECTATOR)
        return -1;

    if (!szName)
    {
        return -1;
    }

    if (!g_pGameRules->CanHaveAmmo(this, szName, iMax))
    {
        return -1;
    }

    int i = GetAmmoIndex(szName);

    if (i < 0 || i >= MAX_AMMO_SLOTS)
        return -1;

    int iAdd = iMax - m_rgAmmo[i];
    if (iAdd > iCount)
        iAdd = iCount;

    if (iAdd > 0)
    {
        m_rgAmmo[i] += iAdd;

        if (gmsgAmmoPickup)
        {

            MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, ENT(pev));
            WRITE_BYTE(GetAmmoIndex(szName));
            WRITE_BYTE(iAdd);
            MESSAGE_END();
        }
    }

    return i;
}

void CBasePlayer::ItemPreFrame()
{
    if (gpGlobals->time < m_flNextAttack)
        return;

    if (!m_pActiveItem)
        return;

    m_pActiveItem->ItemPreFrame();
}

void CBasePlayer::ItemPostFrame()
{
    if (m_pTank != NULL)
        return;

    if (gpGlobals->time < m_flNextAttack)
        return;

    ImpulseCommands();

    if (m_pActiveItem)
        m_pActiveItem->ItemPostFrame();
}

int CBasePlayer::AmmoInventory(int iAmmoIndex)
{
    if (iAmmoIndex == -1)
    {
        return -1;
    }

    return m_rgAmmo[iAmmoIndex];
}

int CBasePlayer::GetAmmoIndex(const char* psz)
{
    int i;

    if (!psz)
        return -1;

    for (i = 1; i < MAX_AMMO_SLOTS; i++)
    {
        if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
            continue;

        if (stricmp(psz, CBasePlayerItem::AmmoInfoArray[i].pszName) == 0)
            return i;
    }

    return -1;
}

void CBasePlayer::SendAmmoUpdate(void)
{
    for (int i = 0; i < MAX_AMMO_SLOTS; i++)
    {
        if (m_rgAmmo[i] != m_rgAmmoLast[i])
        {
            m_rgAmmoLast[i] = m_rgAmmo[i];

            ASSERT(m_rgAmmo[i] >= 0);
            ASSERT(m_rgAmmo[i] < 255);

            MESSAGE_BEGIN(MSG_ONE, gmsgAmmoX, NULL, ENT(pev));
            WRITE_BYTE(i);
            WRITE_BYTE(max(min(m_rgAmmo[i], 254), 0));
            MESSAGE_END();
        }
    }
}

void CBasePlayer::UpdateClientData(void)
{
    if (m_fInitHUD)
    {
        m_fInitHUD = FALSE;
        gInitHUD = FALSE;

        m_iSignals = m_iSignalsPending;
        m_iSignalsPending = 0;

        ALERT(at_console, "1\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgResetHUD, NULL, ENT(pev));
        MESSAGE_END();

        if (!m_fGameHUDInitialized)
        {
            ALERT(at_console, "2\n");

            MESSAGE_BEGIN(MSG_ONE, gmsgInitHUD, NULL, ENT(pev));
            MESSAGE_END();

            g_pGameRules->InitHUD(this);
            m_fGameHUDInitialized = TRUE;

            if (g_pGameRules->IsMultiplayer())
            {
                FireTargets("game_playerjoin", this, this, USE_TOGGLE, 0);
            }
        }

        FireTargets("game_playerspawn", this, this, USE_TOGGLE, 0);

        ALERT(at_console, "3\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, ENT(pev));
        WRITE_LONG(m_iAccount);
        WRITE_BYTE(0);
        MESSAGE_END();

        if (m_bHasDefuser == 1)
        {
            ALERT(at_console, "4\n");

            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(1);
            WRITE_STRING("defuser");
            WRITE_BYTE(0);
            WRITE_BYTE(160);
            WRITE_BYTE(0);
            MESSAGE_END();
        }

        if (m_iTeam)
            SyncRoundTimer();
        SetBombIcon(0);
    }

    if (m_iHideHUD != m_iClientHideHUD)
    {
        ALERT(at_console, "5\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgHideWeapon, NULL, ENT(pev));
        WRITE_BYTE(m_iHideHUD);
        MESSAGE_END();

        m_iClientHideHUD = m_iHideHUD;
    }

    if (m_iFOV != m_iClientFOV)
    {
        ALERT(at_console, "6\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, ENT(pev));
        WRITE_BYTE(m_iFOV);
        MESSAGE_END();
    }

    if (gDisplayTitle)
    {
        ALERT(at_console, "7\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgShowGameTitle, NULL, ENT(pev));
        WRITE_BYTE(0);
        MESSAGE_END();
        gDisplayTitle = 0;
    }

    if ((int)pev->health != m_iClientHealth)
    {
        int iHealth = max((int)pev->health, 0);

        ALERT(at_console, "8\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, ENT(pev));
        WRITE_BYTE(iHealth);
        MESSAGE_END();

        m_iClientHealth = (int)pev->health;
    }

    if ((int)pev->armorvalue != m_iClientBattery)
    {
        m_iClientBattery = (int)pev->armorvalue;

        ALERT(at_console, "9\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgBattery, NULL, ENT(pev));
        WRITE_SHORT((int)pev->armorvalue);
        MESSAGE_END();
    }

    if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
    {
        Vector damageOrigin = pev->origin;

        edict_t* other = pev->dmg_inflictor;
        if (other)
        {
            CBaseEntity* pEntity = CBaseEntity::Instance(other);
            if (pEntity)
                damageOrigin = pEntity->Center();
        }

        int visibleDamageBits = m_bitsDamageType;

        ALERT(at_console, "10\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgDamage, NULL, ENT(pev));
        WRITE_BYTE((int)pev->dmg_save);
        WRITE_BYTE((int)pev->dmg_take);
        WRITE_LONG(visibleDamageBits & DMG_SHOWNHUD);
        WRITE_COORD(damageOrigin.x);
        WRITE_COORD(damageOrigin.y);
        WRITE_COORD(damageOrigin.z);
        MESSAGE_END();

        pev->dmg_take = 0;
        pev->dmg_save = 0;
        m_bitsHUDDamage = m_bitsDamageType;

        m_bitsDamageType &= DMG_TIMEBASED;
    }

    if (m_flFlashLightTime && m_flFlashLightTime <= gpGlobals->time)
    {
        if (FlashlightIsOn())
        {
            if (m_iFlashBattery)
            {
                m_flFlashLightTime = gpGlobals->time + FLASH_DRAIN_TIME;
                m_iFlashBattery--;

                if (!m_iFlashBattery)
                    FlashlightTurnOff();
            }
        }
        else
        {
            if (m_iFlashBattery < 100)
            {
                m_flFlashLightTime = gpGlobals->time + FLASH_CHARGE_TIME;
                m_iFlashBattery++;
            }
            else
                m_flFlashLightTime = 0;
        }

        ALERT(at_console, "11\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgFlashBattery, NULL, ENT(pev));
        WRITE_BYTE(m_iFlashBattery);
        MESSAGE_END();
    }

    if (m_iTrain & TRAIN_NEW)
    {
        ALERT(at_console, "12\n");

        MESSAGE_BEGIN(MSG_ONE, gmsgTrain, NULL, ENT(pev));
        WRITE_BYTE(m_iTrain & 0xF);
        MESSAGE_END();

        m_iTrain &= ~TRAIN_NEW;
    }

    if (!m_fKnownItem)
    {
        m_fKnownItem = TRUE;

        for (int i = 0; i < MAX_WEAPONS; i++)
        {
            ItemInfo& II = CBasePlayerItem::ItemInfoArray[i];

            if (!II.iId)
                continue;

            const char* pszName;
            if (!II.pszName)
                pszName = "Empty";
            else
                pszName = II.pszName;

            ALERT(at_console, "13\n");

            MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList, NULL, ENT(pev));
            WRITE_STRING(pszName);
            WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));
            WRITE_BYTE(II.iMaxAmmo1);
            WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));
            WRITE_BYTE(II.iMaxAmmo2);
            WRITE_BYTE(II.iSlot);
            WRITE_BYTE(II.iPosition);
            WRITE_BYTE(II.iId);
            WRITE_BYTE(II.iFlags);
            MESSAGE_END();
        }
    }

    SendAmmoUpdate();

    for (int i = 0; i < MAX_ITEM_TYPES; i++)
    {
        if (m_rgpPlayerItems[i])
            m_rgpPlayerItems[i]->UpdateClientData(this);
    }

    m_pClientActiveItem = m_pActiveItem;
    m_iClientFOV = m_iFOV;

    if (gpGlobals->time >= m_flNextSBarUpdateTime)
    {
        UpdateStatusBar();
        m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
    }

    if (!(m_flDisplayHistory & DHF_AMMO_EXHAUSTED))
    {
        if (m_pActiveItem)
        {
            if (m_pActiveItem->IsWeapon())
            {
                CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pActiveItem;

                if ((CBasePlayerItem::ItemInfoArray[pWeapon->m_iId].iSlot & ITEM_FLAG_EXHAUSTIBLE)
                    && !AmmoInventory(pWeapon->m_iPrimaryAmmoType)
                    && !pWeapon->m_iClip)
                {
                    m_flDisplayHistory |= DHF_AMMO_EXHAUSTED;
                    HintMessage("You are out of ammunition.\nReturn to a buy zone to purchase more.\n", FALSE, FALSE);
                }
            }
        }
    }

    if (gpGlobals->time >= m_tmHandleSignals)
    {
        m_tmHandleSignals = gpGlobals->time + 0.5;
        HandleSignals();
    }

    if (!pev->deadflag)
    {
        if (gpGlobals->time >= m_tmNextRadarUpdate)
        {
            m_tmNextRadarUpdate = gpGlobals->time + 1.0;

            Vector vecDelta = pev->origin - m_vLastOrigin;
            if (vecDelta.Length() >= 64.0)
            {
                for (int i = 1; i <= gpGlobals->maxClients; i++)
                {
                    CBaseEntity* pEntity = UTIL_PlayerByIndex(i);
                    if (!pEntity)
                        continue;

                    if (i == ENTINDEX(edict()))
                        continue;

                    CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

                    if (pPlayer->pev->flags != 0x80000000
                        && !pPlayer->pev->deadflag
                        && pPlayer->m_iTeam == m_iTeam)
                    {
                        MESSAGE_BEGIN(MSG_ONE, gmsgRadar, NULL, pPlayer->edict());
                        WRITE_BYTE(ENTINDEX(edict()));
                        WRITE_COORD(pev->origin.x);
                        WRITE_COORD(pev->origin.y);
                        WRITE_COORD(pev->origin.z);
                        MESSAGE_END();
                    }
                }
            }

            m_vLastOrigin = pev->origin;
        }
    }
}

BOOL CBasePlayer::FBecomeProne(void)
{
    m_afPhysicsFlags |= PFLAG_ONBARNACLE;
    return TRUE;
}

void CBasePlayer::BarnacleVictimBitten(entvars_t* pevBarnacle)
{
    TakeDamage(pevBarnacle, pevBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB);
}

void CBasePlayer::BarnacleVictimReleased(void)
{
    m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
}

int CBasePlayer::Illumination(void)
{
    int iIllum = CBaseEntity::Illumination();

    iIllum += m_iWeaponFlash;
    if (iIllum > 255)
        return 255;
    return iIllum;
}

void CBasePlayer::EnableControl(BOOL fControl)
{
    if (!fControl)
        pev->flags |= FL_FROZEN;
    else
        pev->flags &= ~FL_FROZEN;

}

#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367

Vector CBasePlayer::GetAutoaimVector(float flDelta)
{
    if (g_iSkillLevel == SKILL_HARD)
    {
        UTIL_MakeVectors(pev->v_angle + pev->punchangle);
        return gpGlobals->v_forward;
    }

    Vector vecSrc = GetGunPosition();

    m_vecAutoAim = Vector(0, 0, 0);

    BOOL m_fOldTargeting = m_fOnTarget;
    Vector angles = AutoaimDeflection(vecSrc, 8192, flDelta);

    if (g_pGameRules->AllowAutoTargetCrosshair())
    {
        if (m_fOldTargeting != m_fOnTarget)
            m_pActiveItem->UpdateItemInfo();
    }
    else
        m_fOnTarget = FALSE;

    if (angles.x > 180)
        angles.x -= 360;
    if (angles.x < -180)
        angles.x += 360;
    if (angles.y > 180)
        angles.y -= 360;
    if (angles.y < -180)
        angles.y += 360;

    if (angles.x > 25)
        angles.x = 25;
    if (angles.x < -25)
        angles.x = -25;
    if (angles.y > 12)
        angles.y = 12;
    if (angles.y < -12)
        angles.y = -12;

    if (g_iSkillLevel == SKILL_EASY)
    {
        m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
    }
    else
    {
        m_vecAutoAim = angles * 0.9;
    }

    if (CVAR_GET_FLOAT("sv_aim") != 0)
    {
        if (m_vecAutoAim.x != m_lastx ||
            m_vecAutoAim.y != m_lasty)
        {
            SET_CROSSHAIRANGLE(edict(), -m_vecAutoAim.x, m_vecAutoAim.y);

            m_lastx = m_vecAutoAim.x;
            m_lasty = m_vecAutoAim.y;
        }
    }

    UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);
    return gpGlobals->v_forward;
}

Vector CBasePlayer::AutoaimDeflection(Vector& vecSrc, float flDist, float flDelta)
{
    edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
    CBaseEntity* pEntity;
    float		bestdot;
    Vector		bestdir;
    edict_t* bestent;
    TraceResult tr;

    if (CVAR_GET_FLOAT("sv_aim") == 0)
    {
        m_fOnTarget = FALSE;
        return g_vecZero;
    }

    UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);

    bestdir = gpGlobals->v_forward;
    bestdot = flDelta;
    bestent = NULL;

    m_fOnTarget = FALSE;

    UTIL_TraceLine(vecSrc, vecSrc + bestdir * flDist, dont_ignore_monsters, edict(), &tr);

    if (tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
    {
        if (!((pev->waterlevel != 3 && tr.pHit->v.waterlevel == 3)
            || (pev->waterlevel == 3 && tr.pHit->v.waterlevel == 0)))
        {
            if (tr.pHit->v.takedamage == DAMAGE_AIM)
                m_fOnTarget = TRUE;

            return m_vecAutoAim;
        }
    }

    for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
    {
        Vector center;
        Vector dir;
        float dot;

        if (pEdict->free)
            continue;

        if (pEdict->v.takedamage != DAMAGE_AIM)
            continue;
        if (pEdict == edict())
            continue;
        if (!g_pGameRules->ShouldAutoAim(this, pEdict))
            continue;

        pEntity = Instance(pEdict);
        if (pEntity == NULL)
            continue;

        if (!pEntity->IsAlive())
            continue;

        if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3)
            || (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
            continue;

        center = pEntity->BodyTarget(vecSrc);

        dir = (center - vecSrc).Normalize();

        if (DotProduct(dir, gpGlobals->v_forward) < 0)
            continue;

        dot = fabs(DotProduct(dir, gpGlobals->v_right))
            + fabs(DotProduct(dir, gpGlobals->v_up)) * 0.5;

        dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

        if (dot > bestdot)
            continue;

        UTIL_TraceLine(vecSrc, center, dont_ignore_monsters, edict(), &tr);
        if (tr.flFraction != 1.0 && tr.pHit != pEdict)
            continue;

        if (IRelationship(pEntity) < 0)
        {
            if (!pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
                continue;
        }

        if (pEntity->IsPlayer() && ((CBasePlayer*)pEntity)->m_iTeam == m_iTeam)
            continue;

        bestdot = dot;
        bestent = pEdict;
        bestdir = dir;
    }

    if (bestent)
    {
        bestdir = UTIL_VecToAngles(bestdir);
        bestdir.x = -bestdir.x;
        bestdir = bestdir - pev->v_angle - pev->punchangle;

        if (bestent->v.takedamage == DAMAGE_AIM)
            m_fOnTarget = TRUE;

        return bestdir;
    }

    return Vector(0, 0, 0);
}

void CBasePlayer::ResetAutoaim()
{
    if (m_vecAutoAim.x != 0.0f || m_vecAutoAim.y != 0.0f)
    {
        m_vecAutoAim = Vector(0, 0, 0);
        SET_CROSSHAIRANGLE(ENT(pev), 0, 0);
    }
    m_fOnTarget = FALSE;
}

void CBasePlayer::SetCustomDecalFrames(int nFrames)
{
    if (nFrames > 0 &&
        nFrames < 8)
        m_nCustomSprayFrames = nFrames;
    else
        m_nCustomSprayFrames = -1;
}

int CBasePlayer::GetCustomDecalFrames(void)
{
    return m_nCustomSprayFrames;
}

void CBasePlayer::DropPlayerItem(char* pszItemName)
{
    if (!strlen(pszItemName))
    {
        pszItemName = NULL;
    }

    CBasePlayerItem* pWeapon;
    int i;

    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        pWeapon = m_rgpPlayerItems[i];

        while (pWeapon)
        {
            if (pszItemName)
            {
                if (!strcmp(pszItemName, STRING(pWeapon->pev->classname)))
                    break;
            }
            else
            {
                if (pWeapon == m_pActiveItem)
                    break;
            }

            pWeapon = pWeapon->m_pNext;
        }

        if (pWeapon)
            break;
    }

    if (!pWeapon)
        return;

    if (!pWeapon->CanDrop())
    {
        ClientPrint(pev, HUD_PRINTCENTER, "This weapon cannot be dropped");
        return;
    }

    g_pGameRules->GetNextBestWeapon(this, pWeapon);

    UTIL_MakeVectors(pev->angles);

    pev->weapons &= ~(1 << pWeapon->m_iId);

    if (pWeapon->iItemSlot() == 1)
        m_bHasPrimary = FALSE;

    if (FClassnameIs(pWeapon->pev, "weapon_c4"))
    {
        m_bHasC4 = FALSE;
        pev->body = 0;
        SetBombIcon(FALSE);

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pWeapon->m_pPlayer->pev));
        WRITE_SHORT(0);
        MESSAGE_END();

        if (!((CHalfLifeMultiplay*)g_pGameRules)->m_flRestartRoundTime)
        {
            UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s dropped the bomb\n", STRING(pev->netname));
        }
    }

    CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create(
        "weaponbox",
        pev->origin + gpGlobals->v_forward * 10,
        pev->angles,
        edict());

    pWeaponBox->pev->angles.x = 0;
    pWeaponBox->pev->angles.z = 0;

    pWeaponBox->SetThink(&CWeaponBox::Kill);
    pWeaponBox->pev->nextthink = gpGlobals->time + 300.0;

    pWeaponBox->PackWeapon(pWeapon);

    pWeaponBox->pev->velocity = gpGlobals->v_forward * 400;

    if (CBasePlayerItem::ItemInfoArray[pWeapon->m_iId].iFlags & ITEM_FLAG_EXHAUSTIBLE)
    {
        int iAmmoIndex = GetAmmoIndex(CBasePlayerItem::ItemInfoArray[pWeapon->m_iId].pszAmmo1);

        if (iAmmoIndex != -1)
        {
            pWeaponBox->PackAmmo(
                MAKE_STRING(CBasePlayerItem::ItemInfoArray[pWeapon->m_iId].pszAmmo1),
                m_rgAmmo[iAmmoIndex] > 0);
            m_rgAmmo[iAmmoIndex] = 0;
        }
    }

    const char* modelname = GetCSModelName(pWeapon->m_iId);
    if (modelname)
    {
        SET_MODEL(ENT(pWeaponBox->pev), modelname);
    }
}

BOOL CBasePlayer::HasPlayerItem(CBasePlayerItem* pCheckItem)
{
    CBasePlayerItem* pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

    while (pItem)
    {
        if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
        {
            return TRUE;
        }
        pItem = pItem->m_pNext;
    }

    return FALSE;
}

BOOL CBasePlayer::HasNamedPlayerItem(const char* pszItemName)
{
    CBasePlayerItem* pItem;
    int i;

    for (i = 0; i < MAX_ITEM_TYPES; i++)
    {
        pItem = m_rgpPlayerItems[i];

        while (pItem)
        {
            if (!strcmp(pszItemName, STRING(pItem->pev->classname)))
            {
                return TRUE;
            }
            pItem = pItem->m_pNext;
        }
    }

    return FALSE;
}

BOOL CBasePlayer::SwitchWeapon(CBasePlayerItem* pWeapon)
{
    if (!pWeapon->CanDeploy())
    {
        return FALSE;
    }

    ResetAutoaim();

    if (m_pActiveItem)
    {
        m_pActiveItem->Holster();
    }

    m_pActiveItem = pWeapon;
    pWeapon->Deploy();

    if (pWeapon->m_pPlayer)
    {
        pWeapon->m_pPlayer->ResetMaxSpeed();
    }

    return TRUE;
}

class CDeadHEV : public CBaseMonster
{
public:
    void Spawn(void);
    int	Classify(void) { return	CLASS_HUMAN_MILITARY; }

    void KeyValue(KeyValueData* pkvd);

    int	m_iPose;
    static char* m_szPoses[4];
};

char* CDeadHEV::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

void CDeadHEV::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "pose"))
    {
        m_iPose = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseMonster::KeyValue(pkvd);
    }
}

LINK_ENTITY_TO_CLASS(monster_hevsuit_dead, CDeadHEV);

void CDeadHEV::Spawn(void)
{
    PRECACHE_MODEL("models/player.mdl");
    SET_MODEL(ENT(pev), "models/player.mdl");

    pev->effects = 0;
    pev->yaw_speed = 8;
    pev->sequence = 0;
    pev->body = 1;
    m_bloodColor = BLOOD_COLOR_RED;

    pev->sequence = LookupSequence(m_szPoses[m_iPose]);

    if (pev->sequence == -1)
    {
        ALERT(at_console, "Dead hevsuit with bad pose\n");
        pev->sequence = 0;
        pev->effects = EF_BRIGHTFIELD;
    }

    pev->health = 8;

    MonsterInitDead();
}

class CStripWeapons : public CPointEntity
{
public:
    void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

private:
};

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

void CStripWeapons::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    CBasePlayer* pPlayer = NULL;

    if (pActivator && pActivator->IsPlayer())
    {
        pPlayer = (CBasePlayer*)pActivator;
    }
    else if (!g_pGameRules->IsDeathmatch())
    {
        pPlayer = (CBasePlayer*)CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
    }

    if (pPlayer)
        pPlayer->RemoveAllItems(FALSE);
}

class CRevertSaved : public CPointEntity
{
public:
    void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
    void	EXPORT MessageThink(void);
    void	EXPORT LoadThink(void);
    void	KeyValue(KeyValueData* pkvd);

    virtual int		Save(CSave& save);
    virtual int		Restore(CRestore& restore);
    static	TYPEDESCRIPTION m_SaveData[];

    inline	float	Duration(void) { return pev->dmg_take; }
    inline	float	HoldTime(void) { return pev->dmg_save; }
    inline	float	MessageTime(void) { return m_messageTime; }
    inline	float	LoadTime(void) { return m_loadTime; }

    inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
    inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
    inline	void	SetMessageTime(float time) { m_messageTime = time; }
    inline	void	SetLoadTime(float time) { m_loadTime = time; }

private:
    float	m_messageTime;
    float	m_loadTime;
};

LINK_ENTITY_TO_CLASS(player_loadsaved, CRevertSaved);

TYPEDESCRIPTION	CRevertSaved::m_SaveData[] =
{
    DEFINE_FIELD(CRevertSaved, m_messageTime, FIELD_FLOAT),
    DEFINE_FIELD(CRevertSaved, m_loadTime, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CRevertSaved, CPointEntity);

void CRevertSaved::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "duration"))
    {
        SetDuration(atof(pkvd->szValue));
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "holdtime"))
    {
        SetHoldTime(atof(pkvd->szValue));
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "messagetime"))
    {
        SetMessageTime(atof(pkvd->szValue));
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "loadtime"))
    {
        SetLoadTime(atof(pkvd->szValue));
        pkvd->fHandled = TRUE;
    }
    else
    {
        pkvd->fHandled = FALSE;
    }
}

void CRevertSaved::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), pev->renderamt, FFADE_OUT);
    pev->nextthink = gpGlobals->time + MessageTime();
    SetThink(&CRevertSaved::MessageThink);
}

void CRevertSaved::MessageThink(void)
{
    UTIL_ShowMessageAll(STRING(pev->message));
    float nextThink = LoadTime() - MessageTime();
    if (nextThink > 0)
    {
        pev->nextthink = gpGlobals->time + nextThink;
        SetThink(&CRevertSaved::LoadThink);
    }
    else
    {
        LoadThink();
    }
}

void CRevertSaved::LoadThink(void)
{
    if (!gpGlobals->deathmatch)
    {
        SERVER_COMMAND("reload\n");
    }
}

class CInfoIntermission :public CPointEntity
{
    void Spawn(void);
    void Think(void);
};

void CInfoIntermission::Spawn(void)
{
    UTIL_SetOrigin(pev, pev->origin);
    pev->solid = SOLID_NOT;
    pev->effects = EF_NODRAW;
    pev->v_angle = g_vecZero;

    pev->nextthink = gpGlobals->time + 2.0;
}

void CInfoIntermission::Think(void)
{
    edict_t* pTarget;

    pTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));

    if (!FNullEnt(pTarget))
    {
        pev->v_angle = UTIL_VecToAngles((pTarget->v.origin - pev->origin).Normalize());
        pev->v_angle.x = -pev->v_angle.x;
    }
}

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);

void CBasePlayer::Radio(const char* msg_id, const char* msg_verbose)
{
    if (!IsPlayer())
        return;

    char* szVarArgs = NULL;
    if (msg_verbose)
        szVarArgs = (char*)UTIL_VarArgs("%s (RADIO): %s\n", STRING(pev->netname), msg_verbose);

    CBaseEntity* pEntity = NULL;
    while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
    {
        if (FNullEnt(pEntity->edict()))
            break;

        if (!pEntity->IsPlayer())
            continue;

        if (pEntity->pev->flags == FL_DORMANT)
            continue;

        CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

        if (pPlayer->pev->deadflag == DEAD_NO
            && pPlayer->m_iTeam == m_iTeam
            && !pPlayer->m_bIgnoreRadio)
        {
            if (msg_verbose)
                ClientPrint(pEntity->pev, HUD_PRINTTALK, szVarArgs);

            MESSAGE_BEGIN(MSG_ONE, gmsgSendAudio, NULL, pEntity->edict());
            WRITE_BYTE(ENTINDEX(edict()));
            WRITE_STRING(msg_id);
            MESSAGE_END();

            if (msg_verbose)
            {
                MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity->edict());
                WRITE_BYTE(TE_PLAYERATTACHMENT);
                WRITE_BYTE(ENTINDEX(edict()));
                WRITE_COORD(35);
                WRITE_SHORT(g_sModelIndexRadio);
                WRITE_SHORT(15);
                MESSAGE_END();
            }
        }
    }
}

void CBasePlayer::Pain(int iLastHitGroup, bool bHasArmour)
{
    int rand = RANDOM_LONG(0, 2);

    switch (iLastHitGroup)
    {
    case HITGROUP_HEAD:
        if (m_iKevlar == ARMOR_VESTHELM)
        {
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_helmet-1.wav", 1, ATTN_NORM);
        }
        else
        {
            switch (rand)
            {
            case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot1.wav", 1, ATTN_NORM); break;
            case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot2.wav", 1, ATTN_NORM); break;
            default: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot3.wav", 1, ATTN_NORM); break;
            }
        }
        break;

    case HITGROUP_LEFTLEG:
    case HITGROUP_RIGHTLEG:
        switch (rand)
        {
        case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-1.wav", 1, ATTN_NORM); break;
        case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-2.wav", 1, ATTN_NORM); break;
        default: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-3.wav", 1, ATTN_NORM); break;
        }
        break;

    default:

        if (bHasArmour)
        {
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_kevlar-1.wav", 1, ATTN_NORM);
        }
        else
        {
            switch (rand)
            {
            case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-1.wav", 1, ATTN_NORM); break;
            case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-2.wav", 1, ATTN_NORM); break;
            default: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-3.wav", 1, ATTN_NORM); break;
            }
        }
        break;
    }
}

extern void OLD_CheckBuyZone(CBasePlayer* pPlayer);
extern void OLD_CheckBombTarget(CBasePlayer* pPlayer);
extern void OLD_CheckRescueZone(CBasePlayer* pPlayer);

void CBasePlayer::HandleSignals(void)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    if (g_pGameRules->IsMultiplayer())
    {
        if (!mp->m_bMapHasBuyZone)
            OLD_CheckBuyZone(this);
        if (!mp->m_bMapHasBombZone)
            OLD_CheckBombTarget(this);
        if (!mp->m_bMapHasRescueZone)
            OLD_CheckRescueZone(this);
    }

    int oldSignals = m_iSignals;
    m_iSignals = m_iSignalsPending;
    m_iSignalsPending = 0;

    int changed = oldSignals ^ m_iSignals;

    if (changed & SIGNAL_BUY)
    {
        if (m_iSignals & SIGNAL_BUY)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(1);
            WRITE_STRING("buyzone");
            WRITE_BYTE(0);
            WRITE_BYTE(160);
            WRITE_BYTE(0);
            MESSAGE_END();
        }
        else
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(0);
            WRITE_STRING("buyzone");
            MESSAGE_END();

            if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
                CLIENT_COMMAND(edict(), "slot10\n");
        }
    }

    if (changed & SIGNAL_BOMB)
    {
        if (m_iSignals & SIGNAL_BOMB)
        {
            if (m_bHasC4)
            {
                if (!(m_flDisplayHistory & DHF_IN_TARGET_ZONE))
                {
                    m_flDisplayHistory |= DHF_IN_TARGET_ZONE;
                    HintMessage("You are in the target zone.\nSelect the bomb in your inventory\nand plant it by holding FIRE!", FALSE, FALSE);
                }
            }
            SetBombIcon(1);
        }
        else
        {
            SetBombIcon(0);
        }
    }

    if (changed & SIGNAL_RESCUE)
    {
        if (m_iSignals & SIGNAL_RESCUE)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(1);
            WRITE_STRING("rescue");
            WRITE_BYTE(0);
            WRITE_BYTE(160);
            WRITE_BYTE(0);
            MESSAGE_END();

            if (m_iTeam == TEAM_CT)
            {
                if (!(m_flDisplayHistory & DHF_IN_RESCUE_ZONE))
                {
                    m_flDisplayHistory |= DHF_IN_RESCUE_ZONE;
                    HintMessage("You are in a hostage rescue zone.\nFind the hostages and bring them here!", FALSE, FALSE);
                }
            }
        }
        else
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(0);
            WRITE_STRING("rescue");
            MESSAGE_END();

            if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
                CLIENT_COMMAND(edict(), "slot10\n");
        }
    }

    if (changed & SIGNAL_ESCAPE)
    {
        if (m_iSignals & SIGNAL_ESCAPE)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(1);
            WRITE_STRING("escape");
            WRITE_BYTE(0);
            WRITE_BYTE(160);
            WRITE_BYTE(0);
            MESSAGE_END();

            if (m_iTeam == TEAM_CT)
            {
                if (!(m_flDisplayHistory & DHF_IN_ESCAPE_ZONE))
                {
                    m_flDisplayHistory |= DHF_IN_ESCAPE_ZONE;
                    HintMessage("You are in a terrorist escape zone.\nPrevent the terrorists from getting here!", FALSE, FALSE);
                }
            }
        }
        else
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(0);
            WRITE_STRING("escape");
            MESSAGE_END();

            if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
                CLIENT_COMMAND(edict(), "slot10\n");
        }
    }

    if (changed & SIGNAL_VIPSAFETY)
    {
        if (m_iSignals & SIGNAL_VIPSAFETY)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(1);
            WRITE_STRING("vipsafety");
            WRITE_BYTE(0);
            WRITE_BYTE(160);
            WRITE_BYTE(0);
            MESSAGE_END();

            if (m_iTeam == TEAM_CT)
            {
                if (!(m_flDisplayHistory & DHF_IN_VIPSAFETY_ZONE))
                {
                    m_flDisplayHistory |= DHF_IN_VIPSAFETY_ZONE;
                    HintMessage("You are in a VIP escape zone.\nEscort the VIP to any one of these zones!", TRUE, FALSE);
                }
            }
            else if (m_iTeam == TEAM_TERRORIST)
            {
                if (!(m_flDisplayHistory & DHF_IN_VIPSAFETY_ZONE))
                {
                    m_flDisplayHistory |= DHF_IN_VIPSAFETY_ZONE;
                    HintMessage("You are in a VIP escape zone.\nPrevent the VIP from reaching any one of these zones", TRUE, FALSE);
                }
            }
        }
        else
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
            WRITE_BYTE(0);
            WRITE_STRING("vipsafety");
            MESSAGE_END();

            if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
                CLIENT_COMMAND(edict(), "slot10\n");
        }
    }
}

void CBasePlayer::SetPlayerModel(int HasC4)
{
    int playerIndex;
    char* infobuffer;
    char* model;

    playerIndex = ENTINDEX(edict());

    switch (m_iModelName)
    {
    case 2:
        CLIENT_COMMAND(edict(), "model terror\n");
        model = "terror";
        break;
    case 3:
        CLIENT_COMMAND(edict(), "model arab\n");
        model = "arab";
        break;
    case 4:
        CLIENT_COMMAND(edict(), "model arctic\n");
        model = "arctic";
        break;
    case 5:
        CLIENT_COMMAND(edict(), "model gsg9\n");
        model = "gsg9";
        break;
    case 6:
        CLIENT_COMMAND(edict(), "model gign\n");
        model = "gign";
        break;
    case 7:
        CLIENT_COMMAND(edict(), "model sas\n");
        model = "sas";
        break;
    case 8:
        CLIENT_COMMAND(edict(), "model vip\n");
        model = "vip";
        break;
    default:
        CLIENT_COMMAND(edict(), "model urban\n");
        model = "urban";
        break;
    }

    infobuffer = g_engfuncs.pfnGetInfoKeyBuffer(edict());
    SET_CLIENT_KEY_VALUE(playerIndex, infobuffer, "model", model);
}

void CBasePlayer::SyncRoundTimer(void)
{
    float flTimeLeft;

    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

    if (g_pGameRules->IsMultiplayer())
        flTimeLeft = (float)mp->m_iRoundTimeSecs - gpGlobals->time + mp->m_fRoundStartTime;
    else
        flTimeLeft = 0;

    MESSAGE_BEGIN(MSG_ONE, gmsgRoundTime, NULL, ENT(pev));
    WRITE_SHORT((int)flTimeLeft > 0 ? (int)flTimeLeft : 0);
    MESSAGE_END();
}

void CBasePlayer::RemoveLevelText(void)
{
    ResetMenu();

    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
    if (mp->m_bFreezePeriod == 1)
    {
        Vector color(0, 0, 0);
        UTIL_ScreenFade(this, color, 0.5f, 0.5f, 0, 0);
    }
}

#define MAX_BUFFER_MENU_BRIEFING 50

void CBasePlayer::MenuPrint(CBasePlayer* pPlayer, const char* pszText)
{
    const char* msg_portion = pszText;
    char sbuf[MAX_BUFFER_MENU_BRIEFING + 1];

    while (strlen(msg_portion) >= MAX_BUFFER_MENU_BRIEFING)
    {
        strncpy(sbuf, msg_portion, MAX_BUFFER_MENU_BRIEFING);
        sbuf[MAX_BUFFER_MENU_BRIEFING] = '\0';

        MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pPlayer->edict());
        WRITE_SHORT(0xFFFF);
        WRITE_CHAR(-1);
        WRITE_BYTE(1);
        WRITE_STRING(sbuf);
        MESSAGE_END();

        msg_portion += MAX_BUFFER_MENU_BRIEFING;
    }

    MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pPlayer->edict());
    WRITE_SHORT(0xFFFF);
    WRITE_CHAR(-1);
    WRITE_BYTE(0);
    WRITE_STRING(msg_portion);
    MESSAGE_END();
}

void CBasePlayer::JoiningThink(void)
{
    CHalfLifeMultiplay* pGameRules = (CHalfLifeMultiplay*)g_pGameRules;

    switch (m_iJoiningState)
    {
    case JOINED:
        return;

    case SHOWLTEXT:
    {
        ResetMenu();

        int bShowBriefing = TRUE;

        char* pInfoBuffer = g_engfuncs.pfnGetInfoKeyBuffer(edict());
        char* pszValue = g_engfuncs.pfnInfoKeyValue(pInfoBuffer, "dm");
        if (pszValue && *pszValue && (!strcmp(pszValue, "0") || !strcmp(pszValue, "off")))
            bShowBriefing = FALSE;

        extern char g_szMapBriefingText[];
        if (!g_szMapBriefingText[0])
            bShowBriefing = FALSE;

        if (bShowBriefing)
        {
            if (pGameRules->m_iMapDefaultBuyStatus)
            {
                Vector color(0, 0, 0);
                UTIL_ScreenFade(this, color, 0.5f, 15.9f, 255, FFADE_OUT | FFADE_MODULATE | FFADE_STAYOUT);
            }
            else
            {
                Vector color(10, 10, 10);
                UTIL_ScreenFade(this, color, 0.0f, 0.0f, 255, FFADE_OUT | FFADE_STAYOUT);
            }

            m_iHideHUD |= (HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);
            MenuPrint(this, g_szMapBriefingText);
            m_iJoiningState = READINGLTEXT;
        }
        else
        {
            m_iJoiningState = SHOWTEAMSELECT;
        }

        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(0);
        WRITE_STRING("defuser");
        MESSAGE_END();

        m_bHasDefuser = false;
        m_bMissionBriefing = false;
        m_iNoGhosts = 0;
        m_fLastMovement = gpGlobals->time;

        char* pszGhosts = g_engfuncs.pfnInfoKeyValue(pInfoBuffer, "ghosts");
        if (pszGhosts && *pszGhosts && (!strcmp(pszGhosts, "0") || !strcmp(pszGhosts, "off")))
            m_iNoGhosts = 1;

        break;
    }

    case READINGLTEXT:
    {
        if (m_afButtonPressed & (IN_ATTACK | IN_ATTACK2 | IN_JUMP))
        {
            m_afButtonPressed &= ~(IN_ATTACK | IN_ATTACK2 | IN_JUMP);
            RemoveLevelText();
            m_iJoiningState = SHOWTEAMSELECT;
        }
        break;
    }

    case GETINTOGAME:
    {
        if (!pGameRules->m_iMapDefaultBuyStatus)
        {
            Vector color(0, 0, 0);
            UTIL_ScreenFade(this, color, 0.0f, 0.0f, 255, 0);
        }

        m_bNotKilled = false;
        m_bBeingKicked = 0;
        m_iIgnoreMessages = 0;
        m_iTeamKills = 0;
        m_iFOV = DEFAULT_FOV;
        m_bJustConnected = FALSE;
        m_iNumSpawns = 0;
        ResetMaxSpeed();
        m_iJoiningState = JOINED;

        CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

        if (mp->m_bMapHasEscapeZone == 1 && m_iTeam == TEAM_CT)
        {
            m_iAccount = 0;
            AddAccount(3000, true);
        }

        if (g_pGameRules->FPlayerCanRespawn(this))
        {
            Spawn();
            mp->CheckWinConditions();

            if (mp->m_flRestartRoundTime == 0.0f)
            {
                if (m_iTeam == TEAM_CT)
                {
                    if (!mp->m_pBomber)
                        mp->m_pBomber = this;
                }
                else
                {
                    if (!mp->m_pC4Carrier)
                    {
                        if (mp->m_bMapHasBombTarget)
                            mp->GiveC4();
                        else
                            mp->m_pC4Carrier = this;
                    }
                }
            }

            if (m_iTeam == TEAM_TERRORIST)
                mp->m_iNumEscapers++;
        }
        else
        {

            pev->deadflag = DEAD_RESPAWNABLE;
            pev->classname = MAKE_STRING("player");
            pev->flags = FL_CLIENT | FL_SPECTATOR;
            m_bTeamChanged = false;

            edict_t* pSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(this);
            StartObserver(pev->origin, pSpawnSpot->v.angles);
            mp->CheckWinConditions();
        }
        return;
    }

    default:
        break;
    }

    if (m_pIntroCamera && m_fIntroCamTime <= gpGlobals->time)
    {
        m_pIntroCamera = UTIL_FindEntityByClassname(m_pIntroCamera, "trigger_camera");

        if (!m_pIntroCamera)
            m_pIntroCamera = UTIL_FindEntityByClassname(NULL, "trigger_camera");

        CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_pIntroCamera->pev->target));
        if (pTarget)
        {

            Vector vecDir = (pTarget->pev->origin - m_pIntroCamera->pev->origin).Normalize();

            Vector vecAngles = UTIL_VecToAngles(vecDir);
            vecAngles.x = -vecAngles.x;

            UTIL_SetOrigin(pev, m_pIntroCamera->pev->origin);
            pev->angles = vecAngles;
            pev->v_angle = pev->angles;
            pev->velocity = g_vecZero;
            pev->punchangle = g_vecZero;
            pev->fixangle = 1;
            pev->view_ofs = g_vecZero;
            m_fIntroCamTime = gpGlobals->time + 6.0f;
        }
        else
        {
            m_pIntroCamera = NULL;
        }
    }
}

void CBasePlayer::Disappear(void)
{
    if (m_pTank != NULL)
    {
        m_pTank->Use(this, this, USE_OFF, 0);
        m_pTank = NULL;
    }

    CSound* pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));
    if (pSound)
        pSound->Reset();

    m_fSequenceFinished = TRUE;
    pev->modelindex = g_ulModelIndexPlayer;
    pev->view_ofs = Vector(0, 0, -8);
    pev->deadflag = DEAD_DYING;
    pev->solid = SOLID_NOT;
    pev->flags &= ~FL_ONGROUND;

    SetSuitUpdate(NULL, FALSE, 0);

    m_iClientHealth = 0;
    MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, ENT(pev));
    WRITE_BYTE(m_iClientHealth);
    MESSAGE_END();

    MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_BYTE(0xFF);
    WRITE_BYTE(0xFF);
    MESSAGE_END();

    m_iClientFOV = 0;
    m_iFOV = 0;
    MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, ENT(pev));
    WRITE_BYTE(0);
    MESSAGE_END();

    ((CHalfLifeMultiplay*)g_pGameRules)->CheckWinConditions();
    m_bNotKilled = false;

    if (m_bHasC4)
    {
        DropPlayerItem("weapon_c4");

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pev));
        WRITE_SHORT(0);
        MESSAGE_END();
    }
    else if (m_bHasDefuser)
    {
        m_bHasDefuser = false;
        pev->body = 0;
        GiveNamedItem("item_thighpack");

        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(0);
        WRITE_STRING("defuser");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, ENT(pev));
        WRITE_SHORT(0);
        MESSAGE_END();
    }

    m_bEscaped = false;

    MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
    WRITE_BYTE(0);
    WRITE_STRING("buyzone");
    MESSAGE_END();

    if (m_iMenu >= Menu_Buy && m_iMenu <= Menu_BuyItem)
        CLIENT_COMMAND(edict(), "slot10\n");

    SetThink(&CBasePlayer::PlayerDeathThink);
    pev->nextthink = gpGlobals->time + 0.1;

    pev->angles.x = 0;
    pev->angles.z = 0;

    m_bTeamChanged = false;
}

void CBasePlayer::StartDeathCam(void)
{
    if (pev->view_ofs == g_vecZero)
        return;

    edict_t* pSpot = FIND_ENTITY_BY_STRING(NULL, "classname", "info_intermission");

    if (FNullEnt(pSpot))
    {
        Vector vecEnd = pev->origin + Vector(0, 0, 128);

        TraceResult tr;
        UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->pContainingEntity, &tr);

        vecEnd = tr.vecEndPos;

        Vector vecDir = vecEnd - pev->origin;
        Vector vecAngles = UTIL_VecToAngles(vecDir);

        StartObserver(vecEnd, vecAngles);
    }
    else
    {
        int index = RANDOM_LONG(0, 3);

        for (int i = 0; i < index; i++)
        {
            edict_t* pNewSpot = FIND_ENTITY_BY_STRING(pSpot, "classname", "info_intermission");
            if (pNewSpot)
                pSpot = pNewSpot;
        }

        Vector vecOrigin = pSpot->v.origin;
        Vector vecAngle = pSpot->v.v_angle;

        StartObserver(vecOrigin, vecAngle);
    }
}

void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle)
{
    m_iHideHUD |= (HIDEHUD_WEAPONS | HIDEHUD_HEALTH);
    m_afPhysicsFlags |= PFLAG_OBSERVER;

    pev->view_ofs = g_vecZero;
    pev->v_angle = vecViewAngle;
    pev->angles = pev->v_angle;
    pev->fixangle = 1;
    pev->solid = SOLID_NOT;
    pev->movetype = MOVETYPE_NOCLIP;
    pev->takedamage = DAMAGE_NO;
    pev->effects = EF_NODRAW;
    pev->modelindex = 0;

    UTIL_SetOrigin(pev, vecPosition);

    UTIL_SetSize(pev, Vector(-1, -1, -1), Vector(1, 1, 1));

    ClearBits(m_afPhysicsFlags, PFLAG_DUCKING);
    ClearBits(pev->flags, FL_DUCKING);

    ResetMaxSpeed();

    if (!m_iChaseCamMode)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgCrosshair, NULL, ENT(pev));
        WRITE_BYTE(1);
        MESSAGE_END();
    }
}

void CBasePlayer::GhostMode(void)
{
    int button = pev->button;
    int chasecam = (int)CVAR_GET_FLOAT("mp_chasecam");

    if ((button & IN_JUMP) && gpGlobals->time > m_fCamSwitch && (chasecam >= 1 && chasecam <= 2))
    {
        m_fCamSwitch = gpGlobals->time + 0.5;
        m_iChaseCamMode = 1;

        CBasePlayer* pTarget = GetPlayerToChase();
        if (!pTarget)
        {
            m_iChaseCamMode = 0;
        }
        else
        {
            HintMessage("You are in ChaseCam Mode. \nPress JUMP to switch back to FreeLook mode.\n Press ATTACK to toggle teammate views.", TRUE, FALSE);

            MESSAGE_BEGIN(MSG_ONE, gmsgCrosshair, NULL, ENT(pev));
            WRITE_BYTE(0);
            MESSAGE_END();

            Vector viewOffset(0, 0, 48);
            Vector origin = pTarget->pev->origin;
            origin.z += 48.0;

            UTIL_SetOrigin(pev, origin);
        }
    }
    else if (button & IN_ATTACK)
    {
        if (gpGlobals->time > m_fCamSwitch)
        {
            m_fCamSwitch = gpGlobals->time + 0.5;

            CBasePlayer* pTarget = GetPlayerToChase();
            if (pTarget)
            {
                Vector viewOffset(0, 0, 48);
                Vector origin = pTarget->pev->origin;
                origin.z += 48.0;

                UTIL_SetOrigin(pev, origin);
            }
        }
    }

    if (gpGlobals->time > m_flNextSBarUpdateTime)
    {
        UpdateStatusBar();
        m_flNextSBarUpdateTime = gpGlobals->time + 0.25;
    }
}

void CBasePlayer::ChaseCamMode(void)
{
    int button = pev->button;
    int chasecam = (int)CVAR_GET_FLOAT("mp_chasecam");

    if ((button & IN_JUMP) && gpGlobals->time > m_fCamSwitch && (!chasecam || chasecam == 2))
    {
        HintMessage("You are in FreeLook Mode. \nPress JUMP to switch back to ChaseCam mode.\n Press ATTACK to toggle teammate views.", TRUE, FALSE);

        MESSAGE_BEGIN(MSG_ONE, gmsgCrosshair, NULL, ENT(pev));
        WRITE_BYTE(1);
        MESSAGE_END();

        m_iChaseCamMode = 0;
        m_fCamSwitch = gpGlobals->time + 0.5;
    }
    else
    {
        if ((button & IN_ATTACK) && gpGlobals->time > m_fCamSwitch)
        {
            m_fCamSwitch = gpGlobals->time + 0.5;

            if (GetPlayerToChase())
            {

                UTIL_MakeVectors(m_pChaseTarget->pev->angles);

                pev->origin.x = m_pChaseTarget->pev->origin.x - gpGlobals->v_forward.x * 20.0;
                pev->origin.y = m_pChaseTarget->pev->origin.y - gpGlobals->v_forward.y * 20.0;
                pev->origin.z = m_pChaseTarget->pev->origin.z + 48.0 - gpGlobals->v_forward.z * 20.0;
            }
        }

        if (!m_pChaseTarget)
        {
            m_iChaseCamMode = 0;
            HintMessage("You are in FreeLook Mode. \nPress JUMP to switch back to ChaseCam mode.\n Press ATTACK to toggle teammate views.", TRUE, FALSE);
        }
        else
        {
            if (m_pChaseTarget->pev->deadflag)
            {
                GetPlayerToChase();
            }
            else
            {
                UTIL_MakeVectors(m_pChaseTarget->pev->angles);

                pev->origin.x = m_pChaseTarget->pev->origin.x - gpGlobals->v_forward.x * 20.0;
                pev->origin.y = m_pChaseTarget->pev->origin.y - gpGlobals->v_forward.y * 20.0;
                pev->origin.z = m_pChaseTarget->pev->origin.z + 48.0 - gpGlobals->v_forward.z * 20.0;
            }
        }
    }
}

CBasePlayer* CBasePlayer::GetPlayerToChase(void)
{
    int selfIndex = ENTINDEX(ENT(pev));

    if (m_iChaseTarget > gpGlobals->maxClients)
        m_iChaseTarget = 1;

    int nextTarget = m_iChaseTarget + 1;
    for (int i = m_iChaseTarget; i <= gpGlobals->maxClients; i++, nextTarget++)
    {
        CBaseEntity* pEntity = UTIL_PlayerByIndex(i);
        if (!pEntity || i == selfIndex || !pEntity->IsPlayer())
            continue;

        CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

        if (m_iChaseCamMode == 1)
        {
            if (pPlayer->pev->flags != FL_DORMANT && !pPlayer->pev->deadflag)
            {
                if (pPlayer->m_iTeam == m_iTeam)
                {
                    m_iChaseTarget = nextTarget;
                    m_pChaseTarget = pEntity;
                    return (CBasePlayer*)pEntity;
                }
            }
        }
        else if (m_iChaseCamMode == 0)
        {
            if (pPlayer->pev->flags != FL_DORMANT && !pPlayer->pev->deadflag)
            {
                m_iChaseTarget = nextTarget;
                m_pChaseTarget = pEntity;
                return (CBasePlayer*)pEntity;
            }
        }
    }

    m_iChaseTarget = 1;
    nextTarget = 2;
    for (int j = 1; j <= gpGlobals->maxClients; j++, nextTarget++)
    {
        CBaseEntity* pEntity = UTIL_PlayerByIndex(j);
        if (!pEntity || j == selfIndex || !pEntity->IsPlayer())
            continue;

        CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

        if (m_iChaseCamMode == 1)
        {
            if (pPlayer->pev->flags != FL_DORMANT && !pPlayer->pev->deadflag)
            {
                if (pPlayer->m_iTeam == m_iTeam)
                {
                    m_iChaseTarget = nextTarget;
                    m_pChaseTarget = pEntity;
                    return (CBasePlayer*)pEntity;
                }
            }
        }
        else if (m_iChaseCamMode == 0)
        {
            if (pPlayer->pev->flags != FL_DORMANT && !pPlayer->pev->deadflag)
            {
                m_iChaseTarget = nextTarget;
                m_pChaseTarget = pEntity;
                return (CBasePlayer*)pEntity;
            }
        }
    }

    m_iChaseTarget = 1;
    for (int k = 1; k <= gpGlobals->maxClients; k++)
    {
        CBaseEntity* pEntity = UTIL_PlayerByIndex(k);
        if (!pEntity || k == selfIndex || !pEntity->IsPlayer())
            continue;

        CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)pEntity->pev);

        if (pPlayer->pev->flags != FL_DORMANT && !pPlayer->pev->deadflag)
        {
            m_iChaseTarget = k + 1;
            m_pChaseTarget = pEntity;
            return (CBasePlayer*)pEntity;
        }
    }

    m_pChaseTarget = NULL;
    m_iChaseTarget = 1;
    return NULL;
}

void CBasePlayer::UpdatePath(void)
{
    Vector dir = m_vRecentPath[0] - pev->origin;

    float dist = dir.Length();

    Vector unitDir;
    if (dist == 0.0f)
    {
        unitDir = Vector(0, 0, 1);
    }
    else
    {
        float invDist = 1.0f / dist;
        unitDir.x = dir.x * invDist;
        unitDir.y = dir.y * invDist;
        unitDir.z = dir.z * invDist;
    }

    dist = dir.Length();

    Vector traceStart = pev->origin + Vector(0, 0, 2);
    Vector traceEnd = pev->origin + unitDir * dist + Vector(0, 0, 2);

    TraceResult tr;
    UTIL_TraceHull(traceStart, traceEnd, ignore_monsters, human_hull, ENT(pev), &tr);

    if (tr.flFraction < 1.0f)
    {
        ALERT(at_console, "Inserting a waypoint : %f , %f , %f\n",
            m_vLastPathPos.x, m_vLastPathPos.y, m_vLastPathPos.z);

        for (int i = MAX_RECENT_PATH - 1; i > 0; i--)
            m_vRecentPath[i] = m_vRecentPath[i - 1];

        m_vRecentPath[0] = m_vLastPathPos;
    }

    m_vLastPathPos = pev->origin;
}

void CBasePlayer::InitStatusBar(void)
{
    m_flStatusBarDisappearDelay = 0;
    m_SbarString0[0] = '\0';
    m_SbarString1[0] = '\0';
}

void CBasePlayer::UpdateStatusBar(void)
{
    int newSBarState[SBAR_END];
    char sbuf0[MAX_SBAR_STRING];
    char sbuf1[MAX_SBAR_STRING];

    memset(newSBarState, 0, sizeof(newSBarState));
    strcpy(sbuf0, m_SbarString0);
    strcpy(sbuf1, m_SbarString1);

    if ((m_afPhysicsFlags & PFLAG_OBSERVER) && m_iChaseCamMode == 1)
    {
        CBasePlayer* pTarget = NULL;

        if (m_pChaseTarget)
            pTarget = GetClassPtr((CBasePlayer*)m_pChaseTarget->pev);

        if (!pTarget || FNullEnt(pTarget->edict()))
        {
            if (gpGlobals->time < m_flStatusBarDisappearDelay)
            {
                newSBarState[SBAR_ID_TARGETTYPE] = m_izSBarState[SBAR_ID_TARGETTYPE];
                newSBarState[SBAR_ID_TARGETNAME] = m_izSBarState[SBAR_ID_TARGETNAME];
                newSBarState[SBAR_ID_TARGETHEALTH] = m_izSBarState[SBAR_ID_TARGETHEALTH];
            }
        }
        else
        {
            strcpy(sbuf1, "1 (Chase mode) %c1: %p2\n2  Health: %i3%%");

            newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pTarget->edict());

            if (pTarget->m_iTeam == m_iTeam)
                newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_TEAMMATE;
            else
                newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_ENEMY;

            newSBarState[SBAR_ID_TARGETHEALTH] = (int)(pTarget->pev->health / pTarget->pev->max_health * 100.0f);
        }

        m_flStatusBarDisappearDelay = gpGlobals->time + 10.0f;
    }
    else
    {
        UTIL_MakeVectors(pev->v_angle + pev->punchangle);

        Vector vecSrc = EyePosition();
        Vector vecEnd = vecSrc + (gpGlobals->v_forward * ((pev->flags & FL_SPECTATOR) ? MAX_SPEC_ID_RANGE : MAX_ID_RANGE));

        TraceResult tr;
        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

        if (tr.flFraction != 1.0f)
        {
            if (!FNullEnt(tr.pHit))
            {
                CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

                if (pEntity->Classify() == CLASS_PLAYER)
                {
                    CBasePlayer* pTarget = (CBasePlayer*)pEntity;

                    newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pTarget->edict());

                    if (pTarget->m_iTeam == m_iTeam)
                        newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_TEAMMATE;
                    else
                        newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_ENEMY;

                    if (m_iTeam == pTarget->m_iTeam || (pev->flags & FL_SPECTATOR))
                    {

                        strcpy(sbuf1, "1 %c1: %p2\n2  Health: %i3%%");
                        newSBarState[SBAR_ID_TARGETHEALTH] = (int)(pTarget->pev->health / pTarget->pev->max_health * 100.0f);

                        if (!(m_flDisplayHistory & DHF_FRIEND_SEEN) && !(pev->flags & FL_SPECTATOR))
                        {
                            m_flDisplayHistory |= DHF_FRIEND_SEEN;
                            HintMessage("You have spotted a friend.", FALSE, FALSE);
                        }
                    }
                    else
                    {

                        strcpy(sbuf1, "1 %c1: %p2");

                        if (!(m_flDisplayHistory & DHF_ENEMY_SEEN))
                        {
                            m_flDisplayHistory |= DHF_ENEMY_SEEN;
                            HintMessage("You have spotted an enemy.", FALSE, FALSE);
                        }
                    }

                    m_flStatusBarDisappearDelay = gpGlobals->time + 10.0f;
                }
                else if (pEntity->Classify() == CLASS_HUMAN_PASSIVE)
                {

                    strcpy(sbuf1, "1 %c1  Health: %i3%%");
                    newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_HOSTAGE;
                    newSBarState[SBAR_ID_TARGETHEALTH] = (int)(pEntity->pev->health / pEntity->pev->max_health * 100.0f);

                    if (!(m_flDisplayHistory & DHF_HOSTAGE_SEEN_FAR) && tr.flFraction > 0.1f)
                    {
                        m_flDisplayHistory |= DHF_HOSTAGE_SEEN_FAR;

                        if (m_iTeam == TEAM_TERRORIST)
                            HintMessage("Prevent the Counter-terrorists from\nrescuing the hostages!", TRUE, FALSE);
                        else if (m_iTeam == TEAM_CT)
                            HintMessage("Rescue the hostages for money!", TRUE, FALSE);
                    }
                    else if (!(m_flDisplayHistory & DHF_HOSTAGE_SEEN_NEAR) && tr.flFraction <= 0.1f)
                    {
                        m_flDisplayHistory |= (DHF_HOSTAGE_SEEN_FAR | DHF_HOSTAGE_SEEN_NEAR);
                        HintMessage("Press USE to get the hostage to follow you.", FALSE, FALSE);
                    }

                    m_flStatusBarDisappearDelay = gpGlobals->time + 10.0f;
                }
            }
        }
        else if (gpGlobals->time < m_flStatusBarDisappearDelay)
        {
            newSBarState[SBAR_ID_TARGETTYPE] = m_izSBarState[SBAR_ID_TARGETTYPE];
            newSBarState[SBAR_ID_TARGETNAME] = m_izSBarState[SBAR_ID_TARGETNAME];
            newSBarState[SBAR_ID_TARGETHEALTH] = m_izSBarState[SBAR_ID_TARGETHEALTH];
        }
    }

    BOOL bForceResend = FALSE;

    if (strcmp(sbuf0, m_SbarString0))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, ENT(pev));
        WRITE_BYTE(0);
        WRITE_STRING(sbuf0);
        MESSAGE_END();

        strcpy(m_SbarString0, sbuf0);
        bForceResend = TRUE;
    }

    if (strcmp(sbuf1, m_SbarString1))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, ENT(pev));
        WRITE_BYTE(1);
        WRITE_STRING(sbuf1);
        MESSAGE_END();

        strcpy(m_SbarString1, sbuf1);
        bForceResend = TRUE;
    }

    for (int i = 1; i < SBAR_END; i++)
    {
        if (newSBarState[i] != m_izSBarState[i] || bForceResend)
        {
            MESSAGE_BEGIN(MSG_ONE, gmsgStatusValue, NULL, ENT(pev));
            WRITE_BYTE(i);
            WRITE_SHORT(newSBarState[i]);
            MESSAGE_END();

            m_izSBarState[i] = newSBarState[i];
        }
    }
}

void CBasePlayer::ResetMaxSpeed(void)
{
    float speed;

    if (m_afPhysicsFlags & PFLAG_OBSERVER)
    {
        speed = 350.0f;
    }
    else if (g_pGameRules->IsMultiplayer() && ((CHalfLifeMultiplay*)g_pGameRules)->m_bFreezePeriod)
    {
        speed = 0.001f;
    }
    else if (m_bIsVIP)
    {
        speed = 220.0f;
    }
    else if (m_pActiveItem)
    {
        speed = m_pActiveItem->GetMaxSpeed();
    }
    else
    {
        speed = 240.0f;
    }

    g_engfuncs.pfnSetClientMaxspeed(edict(), speed);
}

void OLD_CheckBuyZone(CBasePlayer* pPlayer)
{
    const char* pszSpawnClass = NULL;

    if (pPlayer->m_iTeam == 1)
        pszSpawnClass = "info_player_deathmatch";
    else if (pPlayer->m_iTeam == 2)
        pszSpawnClass = "info_player_start";

    if (pszSpawnClass)
    {
        CBaseEntity* pSpot = NULL;
        while ((pSpot = UTIL_FindEntityByClassname(pSpot, pszSpawnClass)) != NULL)
        {
            if ((pSpot->pev->origin - pPlayer->pev->origin).Length() < 200.0f)
            {
                pPlayer->m_iSignalsPending |= SIGNAL_BUY;
                return;
            }
        }
    }
}

void OLD_CheckBombTarget(CBasePlayer* pPlayer)
{
    CBaseEntity* pSpot = NULL;
    while ((pSpot = UTIL_FindEntityByClassname(pSpot, "info_bomb_target")) != NULL)
    {
        if ((pSpot->pev->origin - pPlayer->pev->origin).Length() <= 256.0f)
        {
            pPlayer->m_iSignalsPending |= SIGNAL_BOMB;
            return;
        }
    }
}

void OLD_CheckRescueZone(CBasePlayer* pPlayer)
{
    CBaseEntity* pSpot = NULL;
    while ((pSpot = UTIL_FindEntityByClassname(pSpot, "info_hostage_rescue")) != NULL)
    {
        if ((pSpot->pev->origin - pPlayer->pev->origin).Length() <= 256.0f)
        {
            pPlayer->m_iSignalsPending |= SIGNAL_RESCUE;
            return;
        }
    }
}

void CBasePlayer::RoundRespawn(void)
{
    if (m_bTKPunished)
    {
        m_bTKPunished = FALSE;
        CLIENT_COMMAND(edict(), "kill\n");
        HintMessage("You're not allowed to play this round because/n you TK'ed last round", TRUE, TRUE);
    }

    if (m_iMenu != Menu_ChooseAppearance)
    {
        respawn(pev, FALSE);
        pev->button = 0;
        m_iRespawnFrames = 0;
        pev->nextthink = -1;
    }
}

void CBasePlayer::Reset(void)
{
    pev->frags = 0;
    m_iDeaths = 0;
    m_iAccount = 0;

    AddAccount(-16000, FALSE);

    if (UTIL_FindEntityByClassname(NULL, "func_escapezone") && m_iTeam == TEAM_CT)
    {
        ALERT(at_console, "This map has an escape zone\n");
        AddAccount(3000, TRUE);
    }
    else
    {
        ALERT(at_console, "This map DOES NOT have an escape zone\n");
        AddAccount(800, TRUE);
    }

    MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo, NULL);
    WRITE_BYTE(ENTINDEX(edict()));
    WRITE_SHORT(0);
    WRITE_SHORT(0);
    MESSAGE_END();
}

void CBasePlayer::ThrowWeapon(char* pszItemName)
{
    for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
    {
        CBasePlayerItem* pWeapon = m_rgpPlayerItems[i];

        while (pWeapon)
        {
            if (!strcmp(pszItemName, STRING(pWeapon->pev->classname)))
            {
                ALERT(at_console, "%i \n", pWeapon->m_iId);
                DropPlayerItem(pszItemName);
                return;
            }

            pWeapon = pWeapon->m_pNext;
        }
    }
}

void CBasePlayer::ThrowPrimary(void)
{
    ThrowWeapon("weapon_m249");
    ThrowWeapon("weapon_g3sg1");
    ThrowWeapon("weapon_awp");
    ThrowWeapon("weapon_mp5navy");
    ThrowWeapon("weapon_tmp");
    ThrowWeapon("weapon_p90");
    ThrowWeapon("weapon_m4a1");
    ThrowWeapon("weapon_m3");
    ThrowWeapon("weapon_sg552");
    ThrowWeapon("weapon_scout");
}

void CBasePlayer::AddAccount(int amount, bool bTrackChange)
{
    m_iAccount += amount;

    if (m_iAccount < 0)
        m_iAccount = 0;
    else if (m_iAccount > 16000)
        m_iAccount = 16000;

    MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, ENT(pev));
    WRITE_LONG(m_iAccount);
    WRITE_BYTE(bTrackChange);
    MESSAGE_END();
}

void CBasePlayer::ResetMenu(void)
{
    extern int gmsgShowMenu;
    MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, ENT(pev));
    WRITE_SHORT(0);
    WRITE_CHAR(0);
    WRITE_BYTE(0);
    WRITE_STRING("");
    MESSAGE_END();
}

void CBasePlayer::SetBombIcon(int bFlash)
{
    if (m_bHasC4)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(bFlash ? 2 : 1);
        WRITE_STRING("c4");
        WRITE_BYTE(0);
        WRITE_BYTE(160);
        WRITE_BYTE(0);
        MESSAGE_END();
    }
    else
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, ENT(pev));
        WRITE_BYTE(0);
        WRITE_STRING("c4");
        MESSAGE_END();
    }

    SetScoreboardAttributes(NULL);
}

bool CBasePlayer::HintMessage(const char* pMessage, BOOL bDisplayIfPlayerDead, BOOL bOverride)
{
    if (!bDisplayIfPlayerDead && !IsAlive())
        return false;

    if (bOverride)
        return m_hintMessageQueue.AddMessage(pMessage);

    char* pAutoHelp = g_engfuncs.pfnInfoKeyValue(
        g_engfuncs.pfnGetInfoKeyBuffer(pev->pContainingEntity),
        "ah"
    );

    if (pAutoHelp && *pAutoHelp != '0')
        return m_hintMessageQueue.AddMessage(pMessage);

    return true;
}

BOOL CBasePlayer::CanPlayerBuy(bool bDisplayMessage)
{
    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
    char szMsg[60];
    BOOL bCanBuy = TRUE;

    if (gpGlobals->time - mp->m_fRoundStartTime > 60.0f)
    {
        bCanBuy = FALSE;
        strcpy(szMsg, "60 seconds have passed... You can't buy anything now!\n");
    }

    if (m_bIsVIP)
    {
        bCanBuy = FALSE;
        strcpy(szMsg, "You are the VIP... You can't buy anything!\n");
    }

    if (mp->m_bCTCantBuy && m_iTeam == TEAM_CT)
    {
        bCanBuy = FALSE;
        strcpy(szMsg, "The CTs aren't allowed to buy anything on this map!\n");
    }

    if (mp->m_bTCantBuy && m_iTeam == TEAM_TERRORIST)
    {
        bCanBuy = FALSE;
        strcpy(szMsg, "The Terrorists aren't allowed to buy anything on this map!\n");
    }

    if (bCanBuy)
        return TRUE;

    if (bDisplayMessage)
        ClientPrint(pev, HUD_PRINTCENTER, szMsg);

    return FALSE;
}

void CBasePlayer::SwitchTeam(void)
{
    char modelName[32];
    char modelCmd[32];

    if (m_iTeam == TEAM_CT)
    {
        m_iTeam = TEAM_TERRORIST;

        switch (m_iModelName)
        {
        case 1:
            m_iModelName = 3;
            strcpy(modelCmd, "model arab\n");
            strcpy(modelName, "arab");
            break;
        case 7:
            m_iModelName = 4;
            strcpy(modelCmd, "model arctic\n");
            strcpy(modelName, "arctic");
            break;
        default:
            m_iModelName = 2;
            strcpy(modelCmd, "model terror\n");
            strcpy(modelName, "terror");
            break;
        }
    }
    else if (m_iTeam == TEAM_TERRORIST)
    {
        m_iTeam = TEAM_CT;

        switch (m_iModelName)
        {
        case 2:
            m_iModelName = 5;
            strcpy(modelCmd, "model gsg9\n");
            strcpy(modelName, "gsg9");
            break;
        case 4:
            m_iModelName = 7;
            strcpy(modelCmd, "model sas\n");
            strcpy(modelName, "sas");
            break;
        default:
            m_iModelName = 1;
            strcpy(modelCmd, "model urban\n");
            strcpy(modelName, "urban");
            break;
        }
    }
    else
    {
        return;
    }

    CLIENT_COMMAND(edict(), modelCmd);
    SET_CLIENT_KEY_VALUE(ENTINDEX(edict()), g_engfuncs.pfnGetInfoKeyBuffer(edict()), "model", modelName);
}

void CBasePlayer::MakeVIP(void)
{
    pev->body = 0;
    m_iModelName = 8;

    CLIENT_COMMAND(edict(), "model vip\n");
    SET_CLIENT_KEY_VALUE(ENTINDEX(edict()), g_engfuncs.pfnGetInfoKeyBuffer(edict()), "model", "vip");

    m_iTeam = TEAM_CT;
    m_bIsVIP = true;

    CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;
    mp->m_pVIP = this;
    mp->m_iConsecutiveVIP = 1;
}

BOOL CBasePlayer::HasAssaultSuit(void)
{
    if (m_bHasAssaultSuit && pev->armorvalue != 0)
        return TRUE;

    return FALSE;
}

BOOL CBasePlayer::IsCommander(void)
{
    if (g_pGameRules->IsMultiplayer())
    {
        CHalfLifeMultiplay* mp = (CHalfLifeMultiplay*)g_pGameRules;

        if (m_iTeam == TEAM_TERRORIST)
        {
            if (this == mp->m_pC4Carrier)
                return TRUE;
        }
        else if (m_iTeam == TEAM_CT)
        {
            if (this == mp->m_pBomber)
                return TRUE;
        }
    }

    return FALSE;
}

void CBasePlayer::HostageUsed(void)
{
    if (m_flDisplayHistory & DHF_HOSTAGE_USED)
        return;

    if (m_iTeam == TEAM_TERRORIST)
        HintMessage("You may USE the hostage again to\nstop him from following.", FALSE, FALSE);
    else if (m_iTeam == TEAM_CT)
        HintMessage("Lead the hostage to the rescue point!\nYou may USE the hostage again to\nstop him from following.", FALSE, FALSE);

    m_flDisplayHistory |= DHF_HOSTAGE_USED;
}

void CBasePlayer::SetScoreboardAttributes(CBasePlayer* pPlayer)
{
    extern int gmsgScoreAttrib;

    if (pPlayer)
    {

        int state = (pev->deadflag != DEAD_NO);

        if (m_bHasC4 && pPlayer->m_iTeam == m_iTeam)
            state |= 2;

        if (m_bIsVIP == 1 && pPlayer->m_iTeam == m_iTeam)
            state |= 4;

        MESSAGE_BEGIN(MSG_ONE, gmsgScoreAttrib, NULL, pPlayer->edict());
        WRITE_BYTE(ENTINDEX(edict()));
        WRITE_BYTE(state);
        MESSAGE_END();
    }
    else
    {

        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pEntity = (CBasePlayer*)UTIL_PlayerByIndex(i);

            if (!pEntity)
                continue;

            if (FNullEnt(pEntity->edict()))
                continue;

            int state = (pev->deadflag != DEAD_NO);

            if (m_bHasC4 && pEntity->m_iTeam == m_iTeam)
                state |= 2;

            if (m_bIsVIP == 1 && pEntity->m_iTeam == m_iTeam)
                state |= 4;

            MESSAGE_BEGIN(MSG_ONE, gmsgScoreAttrib, NULL, pEntity->edict());
            WRITE_BYTE(ENTINDEX(edict()));
            WRITE_BYTE(state);
            MESSAGE_END();
        }
    }
}

void CBasePlayer::GiveDefaultItems(void)
{
    RemoveAllItems(FALSE);
    m_bHasPrimary = false;

    if (m_iTeam == TEAM_CT)
    {
        GiveNamedItem("weapon_knife");
        GiveNamedItem("weapon_usp");
        GiveAmmo(24, "45acp", 48);
    }
    else if (m_iTeam == TEAM_TERRORIST)
    {
        GiveNamedItem("weapon_knife");
        GiveNamedItem("weapon_glock18");
        GiveAmmo(40, "9mm", 120);
    }
}

