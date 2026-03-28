#ifndef HOSTAGE_H
#define HOSTAGE_H

#ifdef REGAMEDLL_ADD
class CLocalNav;
#endif

class CHostage : public CBaseMonster
{
public:
	
	CHostage();

	void Spawn( void );
	
	void Precache( void );
	int Classify( void ) { return 3; }  
	
	void SetActivity( int activity );
	
	void SetCollisionBox( void );

	void EXPORT IdleThink( void );
	
	void EXPORT Remove( void );
	
	void RePosition( void );

	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	int ObjectCaps( void ) { return (CBaseMonster::ObjectCaps() | FCAP_ONOFF_USE); }  
	
	void Touch( CBaseEntity *pOther );
	
	int BloodColor( void ) { return BLOOD_COLOR_RED; }  

	int GetActivity( void ) { return m_Activity; }

#ifdef REGAMEDLL_ADD
	
	void DoFollow( void );
	
	void PointAt( const Vector &vecLoc );
	
	void MoveToward( const Vector &vecLoc );
	
	void NavReady( void );
	
	void Wiggle( void );
	
	void PreThink( void );
	
	BOOL IsOnLadder( void );
#endif

public:
	
	int m_Activity;				
	BOOL m_bTouched;			
	BOOL m_bRescueMe;			
	float m_flNextChange;		
	float m_flGoalSpeed;		
	float m_flNextUseTime;		
	float m_flFlinchTime;		
	float m_flPathCheckInterval;	
	int m_nPathNodes;			
	Vector m_vStart;			
	Vector m_vStartAngles;		
	Vector m_vPathToFollow[20];	
	int m_iCurrentNode;			
	BOOL m_bStuck;				
	float m_flStuckTime;		
	int m_iTargetNode;			

#ifdef REGAMEDLL_ADD
	
	CLocalNav *m_LocalNav;		
	int m_nTargetNode;			
#ifdef REGAMEDLL_FIXES
	Vector vecNodes[200];		
#else
	Vector vecNodes[100];		
#endif
	int m_nNavPathNodes;		
	BOOL m_fHasPath;			
	float m_flLastPathCheck;		
	float m_flPathAcquired;		
	float m_flNextFullThink;		
#endif
};

#endif 

