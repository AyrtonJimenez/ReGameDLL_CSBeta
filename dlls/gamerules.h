class CBasePlayerItem;
class CBasePlayer;
class CItem;
class CBasePlayerAmmo;

#define MAX_MOTD_CHUNK		60
#define MAX_MOTD_LENGTH		240

#define ENTITY_INTOLERANCE	100

enum
{
	GR_NONE = 0,

	GR_WEAPON_RESPAWN_YES,
	GR_WEAPON_RESPAWN_NO,

	GR_AMMO_RESPAWN_YES,
	GR_AMMO_RESPAWN_NO,

	GR_ITEM_RESPAWN_YES,
	GR_ITEM_RESPAWN_NO,

	GR_PLR_DROP_GUN_ALL,
	GR_PLR_DROP_GUN_ACTIVE,
	GR_PLR_DROP_GUN_NO,

	GR_PLR_DROP_AMMO_ALL,
	GR_PLR_DROP_AMMO_ACTIVE,
	GR_PLR_DROP_AMMO_NO,
};

enum
{
	GR_NOTTEAMMATE = 0,
	GR_TEAMMATE,   
	GR_ENEMY,      
	GR_ALLY,       
	GR_NEUTRAL,    
};

class CGameRules
{
public:
	
	virtual void RefreshSkillData( void );
	
	virtual void Think( void ) = 0;
	
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;  

	virtual BOOL FAllowFlashlight( void ) = 0;
	
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;
	
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) = 0;

	virtual BOOL IsMultiplayer( void ) = 0;
	
	virtual BOOL IsDeathmatch( void ) = 0;
	
	virtual BOOL IsTeamplay( void ) { return FALSE; };
	
	virtual BOOL IsCoOp( void ) = 0;
	
	virtual const char *GetGameDescription( void ) { return "CounterStrike"; }  

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] ) = 0;
	
	virtual void InitHUD( CBasePlayer *pl ) = 0;		
	
	virtual void ClientDisconnected( edict_t *pClient ) = 0;
	
	virtual void UpdateGameMode( CBasePlayer *pPlayer ) {}  

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;
	
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) {return TRUE;};
	
	virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) { return TRUE; }

	virtual void PlayerSpawn( CBasePlayer *pPlayer ) = 0;
	
	virtual void PlayerThink( CBasePlayer *pPlayer ) = 0; 
	
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;
	
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;
	
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void ) { return TRUE; };
	
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd ) { return FALSE; };  
	
	virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer ) {}		

	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;
	
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor ) = 0;
	
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )=  0;
	
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;

	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon ) = 0;
	
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon ) = 0;
	
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon ) = 0; 
	
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon ) = 0;

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;
	
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;

	virtual int ItemShouldRespawn( CItem *pItem ) = 0;
	
	virtual float FlItemRespawnTime( CItem *pItem ) = 0;
	
	virtual Vector VecItemRespawnSpot( CItem *pItem ) = 0;

	virtual BOOL CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry );
	
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount ) = 0;

	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo ) = 0;
	
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo ) = 0;
	
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo ) = 0;
																			
	virtual float FlHealthChargerRechargeTime( void ) = 0;
	
	virtual float FlHEVChargerRechargeTime( void ) { return 0; }

	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;

	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;

	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;
	
	virtual int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget ) = 0;
	
	virtual int GetTeamIndex( const char *pTeamName ) { return -1; }
	
	virtual const char *GetIndexedTeamName( int teamIndex ) { return ""; }
	
	virtual BOOL IsValidTeam( const char *pTeamName ) { return TRUE; }
	
	virtual void ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib ) {}
	
	virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer ) { return ""; }

	virtual BOOL PlayTextureSounds( void ) { return TRUE; }
	
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol ) { return TRUE; }

	virtual BOOL FAllowMonsters( void ) = 0;

	virtual void EndMultiplayerGame( void ) {}
};

extern CGameRules *InstallGameRules( void );

class CHalfLifeRules : public CGameRules
{
public:
	CHalfLifeRules ( void );

	virtual void Think( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	
	virtual BOOL FAllowFlashlight( void ) { return TRUE; };

	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );		
	virtual void ClientDisconnected( edict_t *pClient );

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );

	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );

	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );

	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

	virtual float FlHealthChargerRechargeTime( void );

	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

	virtual BOOL FAllowMonsters( void );

	virtual const char *GetTeamID( CBaseEntity *pEntity ) {return "";};
	virtual int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget );
};

#define MAX_VIP_QUEUES 5

int CountTeamPlayers( int team );
void EndRoundMessage( const char *sentence );
void Broadcast( const char *sentence );

class CHalfLifeMultiplay : public CGameRules
{
public:
	CHalfLifeMultiplay();

	virtual void RefreshSkillData( void );
	virtual void Think( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	virtual BOOL FAllowFlashlight( void );
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );
	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );
	virtual void ClientDisconnected( edict_t *pClient );
	virtual void UpdateGameMode( CBasePlayer *pPlayer );

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );

	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );

	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

	virtual float FlHealthChargerRechargeTime( void );
	virtual float FlHEVChargerRechargeTime( void );

	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

	virtual const char *GetTeamID( CBaseEntity *pEntity );
	virtual int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget );

	virtual BOOL PlayTextureSounds( void ) { return FALSE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol );

	virtual BOOL FAllowMonsters( void );

	virtual void EndMultiplayerGame( void );

	virtual void CleanUpMap( void );
	
	virtual void RestartRound( void );
	
	virtual void CheckWinConditions( void );
	
	virtual void PickNewCommander( int team );
	
	virtual void RemoveGuns( void );
	
	virtual void GiveC4( void );
	
	virtual void ChangeLevel( void );
	
	virtual void GoToIntermission( void );

public:
	
	void SendMOTDToClient( edict_t *client );

	BOOL TeamFull( int team_id );
	
	BOOL TeamStacked( int newTeam_id, int curTeam_id );
	
	void StackVIPQueue( void );
	
	bool IsVIPQueueEmpty( void );
	
	bool AddToVIPQueue( CBasePlayer *toAdd );
	
	void PickNextVIP( void );
	
	void ResetCurrentVIP( void );
	
	void BalanceTeams( void );
	
	void SwapAllPlayers( void );

	float TimeRemaining( void );
	
	BOOL IsFreezePeriod( void );

public:
	
	float m_flRestartRoundTime;				
	
	float m_flCheckWinConditions;			
	
	float m_fRoundStartTime;				
	
	int m_iRoundTime;						
	
	int m_iRoundTimeSecs;					
	
	int m_iIntroRoundTime;					
	
	float m_fRoundStartTimeReal;			
	
	int m_iAccountTerrorist;				
	
	int m_iAccountCT;						
	
	int m_iNumTerrorist;					
	
	int m_iNumCT;							
	
	int m_iNumSpawnableTerrorist;			
	
	int m_iNumSpawnableCT;					
	
	int m_iSpawnPointCount_Terrorist;		
	
	int m_iSpawnPointCount_CT;				
	
	int m_iHostagesRescued;					
	
	int m_iHostagesTouched;					
	
	int m_iRoundWinStatus;					

	short m_iNumCTWins;						
	
	short m_iNumTerroristWins;				

	int m_iRoundState;						

	bool m_bTargetBombed;					
	
	bool m_bBombDefused;					
	
	bool m_bMapHasBombTarget;				
	
	bool m_bMapHasBombZone;					
	
	bool m_bMapHasBuyZone;					
	
	bool m_bMapHasRescueZone;				
	
	bool m_bMapHasEscapeZone;				
	
	BOOL m_bMapHasVIPSafetyZone;			
	
	int m_iMapDefaultBuyStatus;				
	
	int m_iC4Timer;							
	
	int m_iC4Guy;							
	
	int m_iLoserBonus;						
	
	int m_iNumConsecutiveCTLoses;			
	
	int m_iNumConsecutiveTerroristLoses;	
	
	float m_fMaxRoundTime;					
	
	int m_iLimitTeams;						

	bool m_bLevelInitialized;				
	
	bool m_bRoundTerminating;				
	
	bool m_bCompleteReset;					
	
	float m_flRequiredEscapeRatio;			
	
	int m_iNumEscapers;						
	
	int m_iHaveEscaped;						

	bool m_bCTCantBuy;						
	
	bool m_bTCantBuy;						
	
	float m_flBombRadius;					
	
	int m_iConsecutiveVIP;					
	
	int m_iTotalGunCount;					
	
	int m_iTotalGrenadeCount;				
	
	int m_iTotalArmourCount;				
	
	int m_iUnBalancedRounds;				
	
	int m_iNumEscapeRounds;					

	CBasePlayer *m_pBomber;					
	
	CBasePlayer *m_pC4Carrier;				
	
	CBasePlayer *m_pVIP;					
	
	CBasePlayer *m_pVIPQueue[MAX_VIP_QUEUES];	

	float m_flIntermissionEndTime;			
	
	BOOL m_iEndIntermissionButtonHit;		
	
	BOOL m_bFreezePeriod;					
	
	float m_tmNextPeriodicThink;			
	
	bool m_bGameStarted;					
};

extern DLL_GLOBAL CGameRules*	g_pGameRules;

