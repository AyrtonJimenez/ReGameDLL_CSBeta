#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"items.h"

extern DLL_GLOBAL CGameRules* g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;
extern int gmsgScoreInfo;
extern int gmsgMOTD;

CHalfLifeRules::CHalfLifeRules(void)
{
    RefreshSkillData();
}

void CHalfLifeRules::Think(void)
{
}

BOOL CHalfLifeRules::IsMultiplayer(void)
{
    return FALSE;
}

BOOL CHalfLifeRules::IsDeathmatch(void)
{
    return FALSE;
}

BOOL CHalfLifeRules::IsCoOp(void)
{
    return FALSE;
}

BOOL CHalfLifeRules::FShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
    if (!pPlayer->m_pActiveItem)
    {

        return TRUE;
    }

    if (!pPlayer->m_pActiveItem->CanHolster())
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CHalfLifeRules::GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{
    return FALSE;
}

BOOL CHalfLifeRules::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
    return TRUE;
}

void CHalfLifeRules::InitHUD(CBasePlayer* pl)
{
}

void CHalfLifeRules::ClientDisconnected(edict_t* pClient)
{
}

float CHalfLifeRules::FlPlayerFallDamage(CBasePlayer* pPlayer)
{
    pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
    return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

void CHalfLifeRules::PlayerSpawn(CBasePlayer* pPlayer)
{
}

BOOL CHalfLifeRules::AllowAutoTargetCrosshair(void)
{
    return (g_iSkillLevel == SKILL_EASY);
}

void CHalfLifeRules::PlayerThink(CBasePlayer* pPlayer)
{
}

BOOL CHalfLifeRules::FPlayerCanRespawn(CBasePlayer* pPlayer)
{
    return TRUE;
}

float CHalfLifeRules::FlPlayerSpawnTime(CBasePlayer* pPlayer)
{
    return gpGlobals->time;
}

int CHalfLifeRules::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
    return 1;
}

void CHalfLifeRules::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

void CHalfLifeRules::DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

void CHalfLifeRules::PlayerGotWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
}

float CHalfLifeRules::FlWeaponRespawnTime(CBasePlayerItem* pWeapon)
{
    return -1;
}

float CHalfLifeRules::FlWeaponTryRespawn(CBasePlayerItem* pWeapon)
{
    return 0;
}

Vector CHalfLifeRules::VecWeaponRespawnSpot(CBasePlayerItem* pWeapon)
{
    return pWeapon->pev->origin;
}

int CHalfLifeRules::WeaponShouldRespawn(CBasePlayerItem* pWeapon)
{
    return GR_WEAPON_RESPAWN_NO;
}

BOOL CHalfLifeRules::CanHaveItem(CBasePlayer* pPlayer, CItem* pItem)
{
    return TRUE;
}

void CHalfLifeRules::PlayerGotItem(CBasePlayer* pPlayer, CItem* pItem)
{
}

int CHalfLifeRules::ItemShouldRespawn(CItem* pItem)
{
    return GR_ITEM_RESPAWN_NO;
}

float CHalfLifeRules::FlItemRespawnTime(CItem* pItem)
{
    return -1;
}

Vector CHalfLifeRules::VecItemRespawnSpot(CItem* pItem)
{
    return pItem->pev->origin;
}

BOOL CHalfLifeRules::IsAllowedToSpawn(CBaseEntity* pEntity)
{
    return TRUE;
}

void CHalfLifeRules::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

int CHalfLifeRules::AmmoShouldRespawn(CBasePlayerAmmo* pAmmo)
{
    return GR_AMMO_RESPAWN_NO;
}

float CHalfLifeRules::FlAmmoRespawnTime(CBasePlayerAmmo* pAmmo)
{
    return -1;
}

Vector CHalfLifeRules::VecAmmoRespawnSpot(CBasePlayerAmmo* pAmmo)
{
    return pAmmo->pev->origin;
}

float CHalfLifeRules::FlHealthChargerRechargeTime(void)
{
    return 0;
}

int CHalfLifeRules::DeadPlayerWeapons(CBasePlayer* pPlayer)
{
    return GR_PLR_DROP_GUN_NO;
}

int CHalfLifeRules::DeadPlayerAmmo(CBasePlayer* pPlayer)
{
    return GR_PLR_DROP_AMMO_NO;
}

int CHalfLifeRules::PlayerRelationship(CBasePlayer* pPlayer, CBaseEntity* pTarget)
{

    return GR_NOTTEAMMATE;
}

BOOL CHalfLifeRules::FAllowMonsters(void)
{
    return TRUE;
}

