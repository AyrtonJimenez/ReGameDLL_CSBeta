#ifndef PLAYER_H
#define PLAYER_H

#include "hintmessage.h"

#define DEFAULT_FOV				90

#define DHF_ROUND_STARTED		0x01
#define DHF_BUY_ZONE			0x02	
#define DHF_HOSTAGE_SEEN_FAR	0x04	
#define DHF_HOSTAGE_SEEN_NEAR	0x08	
#define DHF_HOSTAGE_USED		0x10	
#define DHF_HOSTAGE_INJURED		0x20
#define DHF_HOSTAGE_KILLED		0x40
#define DHF_FRIEND_SEEN			0x80	
#define DHF_ENEMY_SEEN			0x100	
#define DHF_FRIEND_INJURED		0x200
#define DHF_FRIEND_KILLED		0x400
#define DHF_ENEMY_KILLED		0x800
#define DHF_BOMB_RETRIEVED		0x1000
#define DHF_AMMO_EXHAUSTED		0x8000
#define DHF_IN_TARGET_ZONE		0x10000
#define DHF_IN_RESCUE_ZONE		0x20000
#define DHF_IN_ESCAPE_ZONE		0x40000
#define DHF_IN_VIPSAFETY_ZONE	0x80000
#define DHF_NIGHTVISION			0x100000	

#define DHM_ROUND_CLEAR			(DHF_ROUND_STARTED | DHF_IN_TARGET_ZONE | DHF_IN_RESCUE_ZONE | DHF_IN_ESCAPE_ZONE)

#define MAX_ID_RANGE			1024
#define MAX_SPEC_ID_RANGE		8192
#define MAX_SBAR_STRING			128

#define SBAR_TARGETTYPE_TEAMMATE	1
#define SBAR_TARGETTYPE_ENEMY		2
#define SBAR_TARGETTYPE_HOSTAGE		3

enum sbar_data
{
    SBAR_ID_TARGETTYPE = 1,
    SBAR_ID_TARGETNAME,
    SBAR_ID_TARGETHEALTH,
    SBAR_END
};

#define DHM_CONNECT_CLEAR		0xF8BBC

#define PLAYER_FATAL_FALL_SPEED		1100
#define PLAYER_MAX_SAFE_FALL_SPEED	500
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)250 

#define		PFLAG_ONLADDER		( 1<<0 )
#define		PFLAG_ONSWING		( 1<<0 )
#define		PFLAG_ONTRAIN		( 1<<1 )
#define		PFLAG_ONBARNACLE	( 1<<2 )
#define		PFLAG_DUCKING		( 1<<3 )		
#define		PFLAG_USING			( 1<<4 )		
#define		PFLAG_OBSERVER		( 1<<5 )		

#define CSUITPLAYLIST	4		

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

#define MAX_RECENT_PATH		20

#define SIGNAL_BUY			(1<<0)
#define SIGNAL_BOMB			(1<<1)
#define SIGNAL_RESCUE		(1<<2)
#define SIGNAL_ESCAPE		(1<<3)
#define SIGNAL_VIPSAFETY	(1<<4)

#define TEAM_UNASSIGNED		0
#define TEAM_TERRORIST		1
#define TEAM_CT				2
#define TEAM_SPECTATOR		3

#define JOINED				0
#define SHOWLTEXT			1
#define READINGLTEXT		2
#define SHOWTEAMSELECT		3
#define PICKINGTEAM			4
#define GETINTOGAME			5

#define Menu_OFF				0
#define Menu_ChooseTeam			1
#define Menu_IGChooseTeam		2
#define Menu_ChooseAppearance	3
#define Menu_Buy				4
#define Menu_BuyPistol			5
#define Menu_BuyRifle			6
#define Menu_BuyMachineGun		7
#define Menu_BuyShotgun			8
#define Menu_BuySubMachineGun	9
#define Menu_BuyItem			10
#define Menu_Radio1				11
#define Menu_Radio2				12
#define Menu_Radio3				13
#define Menu_ClientBuy			14

#define ARMOR_NONE		0
#define ARMOR_KEVLAR	1
#define ARMOR_VESTHELM	2

typedef enum
{
    PLAYER_IDLE,
    PLAYER_WALK,
    PLAYER_JUMP,
    PLAYER_SUPERJUMP,
    PLAYER_DIE,
    PLAYER_ATTACK1,
    PLAYER_FLINCH,
} PLAYER_ANIM;

enum ThrowDirection
{
    THROW_NONE,
    THROW_FORWARD,
    THROW_BACKWARD,
    THROW_HITVEL,
    THROW_BOMB,
    THROW_GRENADE,
    THROW_HITVEL_MINUS_AIRVEL
};

class CBasePlayer : public CBaseMonster
{
public:

    float m_flFlinchTime;
    float m_flAnimTime;
    int m_bHighDamage;
    float m_flVelocityModifier;
    float m_flLastFire;
    float m_flAccuracy;
    float m_flBurstShootTime;
    int m_iBurstShotsFired;
    int m_iLastZoom;
    BOOL m_bResumeZoom;
    int m_iShotsFired;
    float m_flEjectBrass;
    int m_iKevlar;
    bool m_bNotKilled;

    int m_iTeam;
    int m_iAccount;
    bool m_bHasPrimary;
    float m_flDeathThrowTime;
    int m_iThrowDirection;
    float m_flLastTalk;
    bool m_bJustConnected;
    bool m_bContextHelp;

    int m_iJoiningState;
    CBaseEntity* m_pIntroCamera;
    float m_fIntroCamTime;
    int m_iNoGhosts;
    float m_fLastMovement;
    bool m_bMissionBriefing;
    bool m_bTeamChanged;

    int m_iModelName;
    int m_bBeingKicked;
    int m_iTeamKills;
    int m_iIgnoreMessages;
    bool m_bHasNightVision;
    bool m_bNightVisionOn;

    Vector m_vRecentPath[MAX_RECENT_PATH];
    float m_flIdleCheckTime;
    float m_flRadioTime;
    int m_iRadioMessages;
    bool m_bIgnoreRadio;
    bool m_bHasC4;
    bool m_bHasDefuser;
    bool m_bKilledByBomb;
    Vector m_vBlastVector;
    bool m_bKilledByGrenade;

    CHintMessageQueue m_hintMessageQueue;
    int m_flDisplayHistory;
    int m_iMenu;
    int m_iChaseCamMode;
    int m_iChaseTarget;
    CBaseEntity* m_pChaseTarget;
    float m_fCamSwitch;
    bool m_bEscaped;

    Vector m_vLastPathPos;
    bool m_bHasEscaped;
    bool m_bIsVIP;

    float m_tmNextRadarUpdate;
    Vector m_vLastOrigin;
    bool m_bLeftHanded;

    int m_iCurrentKickVote;
    float m_flNextVoteTime;
    bool m_bTKPunished;

    float m_tmHandleSignals;
    int m_iSignalsPending;
    int m_iSignals;
    edict_t* m_pentCurBombTarget;

    int					m_iPlayerSound;
    int					m_iTargetVolume;
    int					m_iWeaponVolume;
    int					m_iExtraSoundTypes;
    int					m_iWeaponFlash;
    float				m_flStopExtraSoundTime;

    float				m_flFlashLightTime;
    int					m_iFlashBattery;

    int					m_afButtonLast;
    int					m_afButtonPressed;
    int					m_afButtonReleased;

    edict_t* m_pentSndLast;
    float				m_flSndRoomtype;
    float				m_flSndRange;

    float				m_flFallVelocity;

    int					m_rgItems[MAX_ITEMS - 1];
    BOOL				m_fKnownItem;
    int					m_fNewAmmo;

    unsigned int		m_afPhysicsFlags;
    float				m_fNextSuicideTime;

    float				m_flTimeStepSound;
    float				m_flTimeWeaponIdle;
    float				m_flSwimTime;
    float				m_flDuckTime;
    float				m_flWallJumpTime;

    float				m_flSuitUpdate;
    int					m_rgSuitPlayList[CSUITPLAYLIST];
    int					m_iSuitPlayNext;
    int					m_rgiSuitNoRepeat[CSUITNOREPEAT];
    float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];
    int					m_lastDamageAmount;
    float				m_tbdPrev;

    float				m_flgeigerRange;
    float				m_flgeigerDelay;
    int					m_igeigerRangePrev;
    int					m_iStepLeft;
    char				m_szTextureName[CBTEXTURENAMEMAX];
    char				m_chTextureType;

    int					m_idrowndmg;
    int					m_idrownrestored;

    int					m_bitsHUDDamage;
    BOOL				m_fInitHUD;
    BOOL				m_fGameHUDInitialized;
    int					m_iTrain;
    BOOL				m_fWeapon;

    EHANDLE				m_pTank;
    float				m_fDeadTime;

    BOOL			m_fNoPlayerSound;
    BOOL			m_fLongJump;

    float       m_tSneaking;
    int			m_iUpdateTime;
    int			m_iClientHealth;
    int			m_iClientBattery;
    int			m_iHideHUD;
    int			m_iClientHideHUD;
    int			m_iFOV;
    int			m_iClientFOV;

    int			m_bHasAssaultSuit;
    int			m_iNumSpawns;
    CBaseEntity* m_pObserver;

    CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];
    CBasePlayerItem* m_pActiveItem;
    CBasePlayerItem* m_pClientActiveItem;
    CBasePlayerItem* m_pLastItem;

    int	m_rgAmmo[MAX_AMMO_SLOTS];
    int	m_rgAmmoLast[MAX_AMMO_SLOTS];

    Vector				m_vecAutoAim;
    BOOL				m_fOnTarget;
    int					m_iDeaths;
    float				m_iRespawnFrames;

    int m_izSBarState[4];
    float m_flNextSBarUpdateTime;
    float m_flStatusBarDisappearDelay;
    char m_SbarString0[128];
    char m_SbarString1[128];

    int m_lastx, m_lasty;

    int m_nCustomSprayFrames;
    float	m_flNextDecalTime;

    char m_szTeamName[TEAM_NAME_LENGTH];

    char m_szAnimExtention[32];

    CBasePlayer();

    virtual void Spawn(void);
    void Pain(void);

    virtual void Jump(void);
    virtual void Duck(void);
    virtual void PreThink(void);
    virtual void PostThink(void);
    virtual Vector GetGunPosition(void);
    virtual int TakeHealth(float flHealth, int bitsDamageType);
    virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
    virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
    virtual void	Killed(entvars_t* pevAttacker, int iGib);
    virtual Vector BodyTarget(const Vector& posSrc) { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); };
    virtual void StartSneaking(void) { m_tSneaking = gpGlobals->time - 1; }
    virtual void StopSneaking(void) { m_tSneaking = gpGlobals->time + 30; }
    virtual BOOL IsSneaking(void) { return m_tSneaking <= gpGlobals->time; }
    virtual BOOL IsAlive(void) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
    virtual BOOL ShouldFadeOnDeath(void) { return FALSE; }
    virtual	BOOL IsPlayer(void) { return (pev->flags & FL_SPECTATOR) != FL_SPECTATOR; }

    virtual BOOL IsNetClient(void) { return TRUE; }

    virtual const char* TeamID(void);

    virtual int		Save(CSave& save);
    virtual int		Restore(CRestore& restore);
    void RenewItems(void);
    void PackDeadPlayerItems(void);
    void RemoveAllItems(BOOL removeSuit);
    BOOL SwitchWeapon(CBasePlayerItem* pWeapon);

    virtual void UpdateClientData(void);

    static	TYPEDESCRIPTION m_playerSaveData[];

    virtual int		ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
    virtual void	Precache(void);
    BOOL			IsOnLadder(void);
    BOOL			FlashlightIsOn(void);
    void			FlashlightTurnOn(void);
    void			FlashlightTurnOff(void);

    void UpdatePlayerSound(void);
    void DeathSound(void);

    int Classify(void);
    void SetAnimation(PLAYER_ANIM playerAnim);
    void SetWeaponAnimType(const char* szExtention);

    virtual void ImpulseCommands(void);
    void CheatImpulseCommands(int iImpulse);

    void StartDeathCam(void);
    void StartObserver(Vector vecPosition, Vector vecViewAngle);

    void AddPoints(int score, BOOL bAllowNegativeScore);
    void AddPointsToTeam(int score, BOOL bAllowNegativeScore);
    BOOL AddPlayerItem(CBasePlayerItem* pItem);
    BOOL RemovePlayerItem(CBasePlayerItem* pItem);
    void DropPlayerItem(char* pszItemName);
    BOOL HasPlayerItem(CBasePlayerItem* pCheckItem);
    BOOL HasNamedPlayerItem(const char* pszItemName);
    BOOL HasWeapons(void);
    void SelectPrevItem(int iItem);
    void SelectNextItem(int iItem);
    void SelectLastItem(void);
    void SelectItem(const char* pstr);
    void ItemPreFrame(void);
    void ItemPostFrame(void);
    void GiveNamedItem(const char* szName);
    void EnableControl(BOOL fControl);

    int  GiveAmmo(int iAmount, char* szName, int iMax);
    void SendAmmoUpdate(void);

    void WaterMove(void);
    void CheckWaterJump(void);
    void EXPORT PlayerDeathThink(void);
    void PlayerUse(void);

    void CheckSuitUpdate();
    void SetSuitUpdate(char* name, int fgroup, int iNoRepeat);
    void UpdateGeigerCounter(void);
    void CheckTimeBasedDamage(void);
    void UpdateStepSound(void);
    void PlayStepSound(int step, float fvol);

    BOOL FBecomeProne(void);
    void BarnacleVictimBitten(entvars_t* pevBarnacle);
    void BarnacleVictimReleased(void);
    static int GetAmmoIndex(const char* psz);
    int AmmoInventory(int iAmmoIndex);
    int Illumination(void);

    void ResetAutoaim(void);
    Vector GetAutoaimVector(float flDelta);
    Vector AutoaimDeflection(Vector& vecSrc, float flDist, float flDelta);

    void ForceClientDllUpdate(void);

    void DeathMessage(entvars_t* pevKiller);

    void SetCustomDecalFrames(int nFrames);
    int GetCustomDecalFrames(void);

    void Radio(const char* msg_id, const char* msg_verbose);
    void Pain(int iLastHitGroup, bool bHasArmour);
    void HandleSignals(void);
    void SetPlayerModel(int HasC4);
    void SyncRoundTimer(void);
    void RemoveLevelText(void);
    void MenuPrint(CBasePlayer* pPlayer, const char* msg);
    void JoiningThink(void);
    void Disappear(void);
    void GhostMode(void);
    void ChaseCamMode(void);
    CBasePlayer* GetPlayerToChase(void);
    void UpdatePath(void);
    void RoundRespawn(void);
    void Reset(void);
    void ThrowWeapon(char* pszItemName);
    void ThrowPrimary(void);
    void AddAccount(int amount, bool bTrackChange);
    void ResetMenu(void);
    void SetBombIcon(int bFlash);
    bool HintMessage(const char* pMessage, BOOL bDisplayIfPlayerDead, BOOL bOverride);
    BOOL CanPlayerBuy(bool bDisplayMessage);
    void SwitchTeam(void);
    void MakeVIP(void);
    BOOL HasAssaultSuit(void);
    BOOL IsCommander(void);
    void HostageUsed(void);
    virtual void ResetMaxSpeed(void);
    void SetScoreboardAttributes(CBasePlayer* pPlayer);
    void GiveDefaultItems(void);

    void InitStatusBar(void);
    void UpdateStatusBar(void);
};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669

extern int	gmsgHudText;
extern BOOL gInitHUD;

const char* GetCSModelName(int item_id);

#endif 

