#ifndef BASEMONSTER_H
#define BASEMONSTER_H

class CBaseMonster : public CBaseToggle
{
public:
	
	Activity			m_Activity;
	
	Activity			m_IdealActivity;
	
	int					m_LastHitGroup; 
	
	int					m_bitsDamageType;	
	
	BYTE				m_rgbTimeBasedDamage[CDMG_TIMEBASED];
	
	MONSTERSTATE		m_MonsterState;
	
	MONSTERSTATE		m_IdealMonsterState;
	
	int					m_afConditions;
	
	int					m_afMemory;
	
	float				m_flNextAttack;		
	
	EHANDLE				m_hEnemy;		 
	
	EHANDLE				m_hTargetEnt;	 
	
	float				m_flFieldOfView;
	
	int					m_bloodColor;		
	
	Vector				m_HackedGunPos;	
	
	Vector				m_vecEnemyLKP;

	void KeyValue( KeyValueData *pkvd );

	void MakeIdealYaw( Vector vecTarget );
	
	virtual float ChangeYaw ( int speed );
	
	virtual BOOL HasHumanGibs( void );
	
	virtual BOOL HasAlienGibs( void );
	
	virtual void FadeMonster( void );	
	
	virtual void GibMonster( void );
	
	virtual Activity GetDeathActivity ( void );
	
	Activity GetSmallFlinchActivity( void );
	
	virtual void BecomeDead( void );
	
	BOOL		 ShouldGibMonster( int iGib );
	
	void		 CallGibMonster( void );
	
	virtual BOOL	ShouldFadeOnDeath( void );
	
	BOOL FCheckAITrigger( void );
	
	virtual int IRelationship ( CBaseEntity *pTarget );
	
	virtual int TakeHealth( float flHealth, int bitsDamageType );
	
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	
	int			DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	
	float DamageForce( float damage );
	
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	
	virtual void PainSound ( void ) { return; };
	
	virtual void ResetMaxSpeed( void ) {}

	void RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );
	
	void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType );

	inline void	SetConditions( int iConditions ) { m_afConditions |= iConditions; }
	
	inline void	ClearConditions( int iConditions ) { m_afConditions &= ~iConditions; }
	
	inline BOOL HasConditions( int iConditions ) { if ( m_afConditions & iConditions ) return TRUE; return FALSE; }
	
	inline BOOL HasAllConditions( int iConditions ) { if ( (m_afConditions & iConditions) == iConditions ) return TRUE; return FALSE; }

	inline void	Remember( int iMemory ) { m_afMemory |= iMemory; }
	
	inline void	Forget( int iMemory ) { m_afMemory &= ~iMemory; }
	
	inline BOOL HasMemory( int iMemory ) { if ( m_afMemory & iMemory ) return TRUE; return FALSE; }
	
	inline BOOL HasAllMemories( int iMemory ) { if ( (m_afMemory & iMemory) == iMemory ) return TRUE; return FALSE; }

	inline void StopAnimation( void ) { pev->framerate = 0; }

	virtual void ReportAIState( void );
	
	virtual void MonsterInitDead( void );	
	
	void EXPORT CorpseFallThink( void );

	virtual void Look ( int iDistance );
	
	virtual CBaseEntity* BestVisibleEnemy ( void );
	
	CBaseEntity *CheckTraceHullAttack( float flDist, int iDamage, int iDmgType );
	
	virtual BOOL FInViewCone ( CBaseEntity *pEntity );
	
	virtual BOOL FInViewCone ( Vector *pOrigin );
	
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	
	void BloodSplat( Vector &vecSrc, Vector &vecDir, int hitgroup, int iDamage );
	
	void MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir );
	
	virtual int		BloodColor( void ) { return m_bloodColor; }
	
	virtual BOOL	IsAlive( void ) { return (pev->deadflag != DEAD_DEAD); }

};

#endif

