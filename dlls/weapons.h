#ifndef WEAPONS_H
#define WEAPONS_H

class CBasePlayer;
extern int gmsgWeapPickup;

void DeactivateSatchels( CBasePlayer *pOwner );

class CGrenade : public CBaseMonster
{
public:
	void Spawn( void );

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecAngles );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void EXPORT Smoke( void );

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );

	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual int ObjectCaps( void ) { return m_bIsC4 ? FCAP_CONTINUOUS_USE : 0; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	static CGrenade *ShootTimed2( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int team );
	void EXPORT C4Think( void );
	void EXPORT C4Touch( CBaseEntity *pOther );
	void Explode2( TraceResult *pTrace, int bitsDamageType );
	void Explode3( TraceResult *pTrace, int bitsDamageType );
	void EXPORT Smoke2( void );
	void EXPORT Smoke3_A( void );
	void EXPORT Smoke3_B( void );
	void EXPORT Smoke3_C( void );
	void EXPORT Detonate2( void );
	void EXPORT Detonate3( void );

	bool m_bStartDefuse;			
	bool m_bIsC4;					
	
	CBaseEntity *m_pBombDefuser;	
	float m_flDefuseCountDown;		
	float m_flC4Blow;				
	float m_flNextFreqInterval;		
	float m_flNextBeep;				
	float m_flNextFreq;				
	char *m_sBeepName;				
	float m_fAttenu;				
	float m_flNextBlink;			
	float m_fNextDefuse;			
	BOOL m_bJustBlew;				
	int m_iTeam;					
	int m_iCurWave;					
	edict_t *m_pentCurBombTarget;	
	BOOL m_fRegisteredSound;		
	int m_SGSmoke;					
	int m_bLightSmoke;				
	float m_flNextGlow;				
};

#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

#define WEAPON_NONE				0
#define WEAPON_CROWBAR			1
#define	WEAPON_GLOCK			2
#define WEAPON_PYTHON			3
#define WEAPON_MP5				4
#define WEAPON_CHAINGUN			5
#define WEAPON_CROSSBOW			6
#define WEAPON_SHOTGUN			7
#define WEAPON_RPG				8
#define WEAPON_GAUSS			9
#define WEAPON_EGON				10
#define WEAPON_HORNETGUN		11
#define WEAPON_HANDGRENADE		12
#define WEAPON_TRIPMINE			13
#define	WEAPON_SATCHEL			14
#define	WEAPON_SNARK			15

#define WEAPON_P228				1
#define WEAPON_SCOUT			3
#define WEAPON_HEGRENADE		4
#define WEAPON_XM1014			5
#define WEAPON_C4				6
#define WEAPON_MAC10			7
#define WEAPON_AUG				8
#define WEAPON_SMOKEGRENADE		9
#define WEAPON_ELITE			10
#define WEAPON_FIVESEVEN		11
#define WEAPON_UMP45			12
#define WEAPON_SG550			13
#define WEAPON_USP				16
#define WEAPON_GLOCK18			17
#define WEAPON_AWP				18
#define WEAPON_MP5N				19
#define WEAPON_M249				20
#define WEAPON_M3				21
#define WEAPON_M4A1				22
#define WEAPON_TMP				23
#define WEAPON_G3SG1			24
#define WEAPON_FLASHBANG		25
#define WEAPON_DEAGLE			26
#define WEAPON_SG552			27
#define WEAPON_AK47				28
#define WEAPON_KNIFE			29
#define WEAPON_P90				30
#define WEAPON_SHIELD			2

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	

#define MAX_WEAPONS			32

#define MAX_NORMAL_BATTERY	100

#define CROWBAR_WEIGHT		0
#define GLOCK_WEIGHT		10
#define PYTHON_WEIGHT		15
#define MP5_WEIGHT			15
#define SHOTGUN_WEIGHT		15
#define CROSSBOW_WEIGHT		10
#define RPG_WEIGHT			20
#define GAUSS_WEIGHT		20
#define EGON_WEIGHT			20
#define HORNETGUN_WEIGHT	10
#define HANDGRENADE_WEIGHT	5
#define SNARK_WEIGHT		5
#define SATCHEL_WEIGHT		-10
#define TRIPMINE_WEIGHT		-10

#define URANIUM_MAX_CARRY		100
#define	_9MM_MAX_CARRY			250
#define _357_MAX_CARRY			36
#define BUCKSHOT_MAX_CARRY		125
#define BOLT_MAX_CARRY			50
#define ROCKET_MAX_CARRY		5
#define HANDGRENADE_MAX_CARRY	10
#define SATCHEL_MAX_CARRY		5
#define TRIPMINE_MAX_CARRY		5
#define SNARK_MAX_CARRY			15
#define HORNET_MAX_CARRY		8
#define M203_GRENADE_MAX_CARRY	10

#define WEAPON_NOCLIP			-1

#define GLOCK_MAX_CLIP			17
#define PYTHON_MAX_CLIP			6
#define MP5_MAX_CLIP			50
#define MP5_DEFAULT_AMMO		25
#define SHOTGUN_MAX_CLIP		8
#define CROSSBOW_MAX_CLIP		5
#define RPG_MAX_CLIP			1
#define GAUSS_MAX_CLIP			WEAPON_NOCLIP
#define EGON_MAX_CLIP			WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP		WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP	WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP		WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP		WEAPON_NOCLIP
#define SNARK_MAX_CLIP			WEAPON_NOCLIP

#define GLOCK_DEFAULT_GIVE			17
#define PYTHON_DEFAULT_GIVE			6
#define MP5_DEFAULT_GIVE			25
#define MP5_DEFAULT_AMMO			25
#define MP5_M203_DEFAULT_GIVE		0
#define SHOTGUN_DEFAULT_GIVE		12
#define CROSSBOW_DEFAULT_GIVE		5
#define RPG_DEFAULT_GIVE			1
#define GAUSS_DEFAULT_GIVE			20
#define EGON_DEFAULT_GIVE			20
#define HANDGRENADE_DEFAULT_GIVE	5
#define SATCHEL_DEFAULT_GIVE		1
#define TRIPMINE_DEFAULT_GIVE		1
#define SNARK_DEFAULT_GIVE			5
#define HIVEHAND_DEFAULT_GIVE		8

#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_GLOCKCLIP_GIVE		GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE		PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE		MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE		200
#define AMMO_M203BOX_GIVE		2
#define AMMO_BUCKSHOTBOX_GIVE	12
#define AMMO_CROSSBOWCLIP_GIVE	CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE		RPG_MAX_CLIP
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_SNARKBOX_GIVE		5

typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, 
	BULLET_PLAYER_MP5, 
	BULLET_PLAYER_357, 
	BULLET_PLAYER_BUCKSHOT, 
	BULLET_PLAYER_CROWBAR, 

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	BULLET_PLAYER_45ACP,	
	BULLET_PLAYER_338MAG,	
	BULLET_PLAYER_762MM,	
	BULLET_PLAYER_556MM,	
	BULLET_PLAYER_50AE,		
	BULLET_PLAYER_57MM,		
	BULLET_PLAYER_357SIG,	
} Bullet;

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;				
	int		iPosition;			
	const char	*pszAmmo1;	
	int		iMaxAmmo1;		
	const char	*pszAmmo2;	
	int		iMaxAmmo2;		
	const char	*pszName;	
	int		iMaxClip;		
	int		iId;			
	int		iFlags;			
	int		iWeight;		
} ItemInfo;

typedef struct
{
	const char *pszName;	
	int iId;			
} AmmoInfo;

class CBasePlayerItem : public CBaseAnimating
{
public:
	
	virtual void SetObjectCollisionBox( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	
	
	void EXPORT DestroyItem( void );
	
	void EXPORT DefaultTouch( CBaseEntity *pOther );	
	
	void EXPORT FallThink ( void );
	
	void EXPORT Materialize( void );
	
	void EXPORT AttemptToMaterialize( void );  
	
	CBaseEntity* Respawn ( void );
	
	void FallInit( void );
	
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL CanDrop( void ) { return TRUE; }		
	virtual BOOL Deploy( )								
		 { return TRUE; };

	virtual BOOL IsWeapon( void ) { return FALSE; }	
	virtual BOOL CanHolster( void ) { return TRUE; };
	virtual void Holster( );
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void )	{ return; }		
	virtual void ItemPostFrame( void ) { return; }		

	virtual void Drop( void );
	virtual void Kill( void );
	
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual float GetMaxSpeed( void ) { return 260.0f; }	

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;												

	virtual int iItemSlot( void ) { return 0; }			

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

};

class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon );
	
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	

	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {};	

	virtual BOOL IsWeapon( void ) { return TRUE; }

	int m_iPlayEmptySound;	
	int m_fFireOnEmpty;		
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim );	

	virtual BOOL CanDeploy( void );	
	virtual BOOL IsUseable( void );	
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt );
	int DefaultReload( int iClipSize, int iAnim, float fDelay );

	virtual void ItemPostFrame( void );	
	virtual void PrimaryAttack( void ) { return; }				
	virtual void SecondaryAttack( void ) { return; }			
	virtual void Reload( void ) { return; }						
	virtual void WeaponIdle( void ) { return; }					
	virtual int UpdateClientData( CBasePlayer *pPlayer );		
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( void );

	void ReloadSound( void );

	void EjectBrassLate( void );

	void FireRemaining( void );

	void KickBack( float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change );
	
	int	PrimaryAmmoIndex();	
	int	SecondaryAmmoIndex();	

	float	m_flNextPrimaryAttack;	
	float	m_flNextSecondaryAttack;	
	float	m_flTimeWeaponIdle;		
	int		m_iPrimaryAmmoType;		
	int		m_iSecondaryAmmoType;	
	int		m_iClip;					
	int		m_iClientClip;			
	int		m_iClientWeaponState;	
	int		m_fInReload;			

	int		m_iDefaultAmmo;	

	int		m_iShellId;			
	BOOL	m_bDelayFire;		
	int		m_iDirection;		
	int		m_iShell;			
	int		iShellOn;			
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );	
	void EXPORT DefaultTouch( CBaseEntity *pOther );	
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };	

	CBaseEntity* Respawn( void );	
	void EXPORT Materialize( void );	
};

extern DLL_GLOBAL	short	g_sModelIndexLaser;
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;
extern DLL_GLOBAL	short	g_sModelIndexFireball;
extern DLL_GLOBAL	short	g_sModelIndexSmoke;
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;
extern DLL_GLOBAL	short	g_sModelIndexBubbles;
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;

void EjectBrass( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype );
void EjectBrass2( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype, entvars_t *pevShooter );

extern DLL_GLOBAL	short	g_sModelIndexSmokePuff;
extern DLL_GLOBAL	short	g_sModelIndexFireball2;
extern DLL_GLOBAL	short	g_sModelIndexFireball3;
extern DLL_GLOBAL	short	g_sModelIndexFireball4;
extern DLL_GLOBAL	short	g_sModelIndexRadio;
extern DLL_GLOBAL	short	g_sModelIndexCTGhost;
extern DLL_GLOBAL	short	g_sModelIndexTGhost;
extern DLL_GLOBAL	short	g_sModelIndexC4Glow;

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType, bool bClientOnly, entvars_t *pShooter, bool bHitMetal );
inline void DecalGunshot( TraceResult *pTrace, int iBulletType ) { DecalGunshot( pTrace, iBulletType, false, NULL, false ); }
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );
extern void RadiusDamage2( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );
extern void RadiusFlash( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

class CWeaponBox : public CBaseEntity
{
	void Precache( void );	
	void Spawn( void );		
	void Touch( CBaseEntity *pOther );	
	void KeyValue( KeyValueData *pkvd );	
	BOOL IsEmpty( void );	
	int  GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );	
	void SetObjectCollisionBox( void );	

public:
	void EXPORT Kill ( void );	
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int HasWeapon( CBasePlayerItem *pCheckItem );	
	BOOL PackWeapon( CBasePlayerItem *pWeapon );	
	BOOL PackAmmo( int iszName, int iCount );	

	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];	

	int m_rgiszAmmo[MAX_AMMO_SLOTS];	
	int	m_rgAmmo[MAX_AMMO_SLOTS];		

	int m_cAmmoTypes;	
};

#endif 

