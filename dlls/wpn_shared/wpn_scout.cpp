#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum scout_e
{
	SCOUT_IDLE = 0,
	SCOUT_SHOOT1,
	SCOUT_SHOOT2,
	SCOUT_RELOAD,
	SCOUT_DRAW,
};

class CSCOUT : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int iItemSlot( void ) { return 1; }

	BOOL Deploy( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Reload( void );
	void WeaponIdle( void );
	float GetMaxSpeed( void );

	void SCOUTFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
};

LINK_ENTITY_TO_CLASS( weapon_scout, CSCOUT );

void CSCOUT::Spawn( void )
{
	
	pev->classname = MAKE_STRING("weapon_scout");
	
	Precache();
	m_iId = WEAPON_SCOUT;
	SET_MODEL(ENT(pev), "models/w_scout.mdl");
	
	m_iDefaultAmmo = 10;
	FallInit();
}

void CSCOUT::Precache( void )
{
	
	PRECACHE_MODEL("models/v_scout.mdl");
	PRECACHE_MODEL("models/v_scout_r.mdl");
	PRECACHE_MODEL("models/w_scout.mdl");
	PRECACHE_MODEL("models/p_scout.mdl");

	PRECACHE_SOUND("weapons/scout_fire-1.wav");
	PRECACHE_SOUND("weapons/scout_bolt.wav");
	PRECACHE_SOUND("weapons/scout_clipin.wav");
	PRECACHE_SOUND("weapons/scout_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CSCOUT::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = 60;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 10;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SCOUT;
	p->iWeight = 30;
	return 1;
}

BOOL CSCOUT::Deploy( void )
{
	
	if (m_pPlayer->m_bLeftHanded == 1)
		return DefaultDeploy("models/v_scout.mdl", "models/p_scout.mdl", SCOUT_DRAW, "silentsniper");
	else
		return DefaultDeploy("models/v_scout_r.mdl", "models/p_scout.mdl", SCOUT_DRAW, "silentsniper");
}

void CSCOUT::PrimaryAttack( void )
{
	
	if (m_pPlayer->pev->velocity.Length2D() > 170)
		SCOUTFire(0.075f, 1.35f, FALSE);
	else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		SCOUTFire(0.1f, 1.35f, FALSE);
	else if (m_pPlayer->pev->flags & FL_DUCKING)
		SCOUTFire(0.0f, 1.35f, FALSE);
	else
		SCOUTFire(0.007f, 1.35f, FALSE);
}

void CSCOUT::SCOUTFire( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
	
	if (m_pPlayer->m_iFOV == 90)
	{
		flSpread += 0.025f;
	}
	else
	{
		
		if (m_pPlayer->m_bLeftHanded == 1)
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout.mdl");
		else
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout_r.mdl");

		m_pPlayer->m_bResumeZoom = TRUE;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = 90;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->time + 0.2f;
		}
		return;
	}

	m_iClip--;

	if (RANDOM_LONG(0, 1))
		SendWeaponAnim(SCOUT_SHOOT1);
	else
		SendWeaponAnim(SCOUT_SHOOT2);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.56f;

	m_pPlayer->m_iWeaponVolume = 2048;
	m_pPlayer->m_iWeaponFlash = 256;

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/scout_fire-1.wav",
		RANDOM_FLOAT(0.92f, 1.0f), 1.28f, 0, RANDOM_LONG(0, 3) + 98);

	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

	m_pPlayer->pev->punchangle.x -= 2.0f;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(
		vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, 3,
		BULLET_PLAYER_762MM, 75, 0.98f, NULL, true);
}

void CSCOUT::SecondaryAttack( void )
{
	m_pPlayer->pev->viewmodel = 0;

	switch (m_pPlayer->m_iFOV)
	{
	case 90:
		m_pPlayer->m_iFOV = 40;
		break;
	case 40:
		m_pPlayer->m_iFOV = 15;
		break;
	case 15:
		if (m_pPlayer->m_bLeftHanded == 1)
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout.mdl");
		else
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout_r.mdl");
		m_pPlayer->m_iFOV = 90;
		break;
	}

	m_pPlayer->ResetMaxSpeed();

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1.0f, 0.8f, 0, 100);
	m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CSCOUT::Reload( void )
{
	if (DefaultReload(10, SCOUT_RELOAD, 2.0f))
	{
		
		if (m_pPlayer->m_iFOV != 90)
		{
			m_pPlayer->m_iFOV = 15;
			SecondaryAttack();
		}

		m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
	}
}

void CSCOUT::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = gpGlobals->time + 60.0f;
		SendWeaponAnim(SCOUT_IDLE);
	}
}

float CSCOUT::GetMaxSpeed( void )
{
	if (m_pPlayer->m_iFOV == 90)
		return 260.0f;
	return 220.0f;
}

