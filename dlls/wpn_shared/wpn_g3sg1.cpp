#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum g3sg1_e
{
	G3SG1_IDLE = 0,
	G3SG1_SHOOT1,
	G3SG1_SHOOT2,
	G3SG1_RELOAD,
	G3SG1_DRAW,
};

class CG3SG1 : public CBasePlayerWeapon
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

	void G3SG1Fire( float flSpread, float flCycleTime );
};

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CG3SG1 );

void CG3SG1::Spawn( void )
{
	pev->classname = MAKE_STRING("weapon_g3sg1");
	Precache();
	m_iId = WEAPON_G3SG1;
	SET_MODEL(ENT(pev), "models/w_g3sg1.mdl");
	m_iDefaultAmmo = 20;
	FallInit();
}

void CG3SG1::Precache( void )
{
	PRECACHE_MODEL("models/v_g3sg1.mdl");
	PRECACHE_MODEL("models/v_g3sg1_r.mdl");
	PRECACHE_MODEL("models/w_g3sg1.mdl");
	PRECACHE_MODEL("models/p_g3sg1.mdl");

	PRECACHE_SOUND("weapons/g3sg1-1.wav");
	PRECACHE_SOUND("weapons/g3sg1_slide.wav");
	PRECACHE_SOUND("weapons/g3sg1_clipin.wav");
	PRECACHE_SOUND("weapons/g3sg1_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
}

int CG3SG1::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = 40;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 20;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_G3SG1;
	p->iWeight = 20;
	return 1;
}

BOOL CG3SG1::Deploy( void )
{
	char *pszViewModel;

	if (m_pPlayer->m_bLeftHanded == 1)
		pszViewModel = "models/v_g3sg1.mdl";
	else
		pszViewModel = "models/v_g3sg1_r.mdl";

	return DefaultDeploy(pszViewModel, "models/p_g3sg1.mdl", G3SG1_DRAW, "mp5");
}

void CG3SG1::PrimaryAttack( void )
{
	float flSpread;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
	{
		
		if (m_pPlayer->pev->velocity.Length2D() > 0)
		{
			G3SG1Fire(0.15f, 0.25f);
			return;
		}

		if (m_pPlayer->pev->flags & FL_DUCKING)
			flSpread = 0.035f * (1.0f - m_pPlayer->m_flAccuracy);
		else
			flSpread = 0.055f * (1.0f - m_pPlayer->m_flAccuracy);
	}
	else
	{
		flSpread = 0.45f * (1.0f - m_pPlayer->m_flAccuracy);
	}

	G3SG1Fire(flSpread, 0.25f);
}

void CG3SG1::SecondaryAttack( void )
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
	{
		char *pszViewModel;

		if (m_pPlayer->m_bLeftHanded == 1)
			pszViewModel = "models/v_g3sg1.mdl";
		else
			pszViewModel = "models/v_g3sg1_r.mdl";

		m_pPlayer->pev->viewmodel = MAKE_STRING(pszViewModel);
		m_pPlayer->m_iFOV = 90;
		break;
	}
	}

	m_pPlayer->ResetMaxSpeed();

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1.0f, 0.8f, 0, 100);
	m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CG3SG1::G3SG1Fire( float flSpread, float flCycleTime )
{
	if (m_pPlayer->m_iFOV == 90)
		flSpread += 0.025f;

	if (m_pPlayer->m_flLastFire != 0)
	{
		m_pPlayer->m_flAccuracy = (gpGlobals->time - m_pPlayer->m_flLastFire) * 0.225f + 0.65f;
		if (m_pPlayer->m_flAccuracy > 0.98f)
			m_pPlayer->m_flAccuracy = 0.98f;
	}
	m_pPlayer->m_flLastFire = gpGlobals->time;

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

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	if (RANDOM_LONG(0, 1))
		SendWeaponAnim(G3SG1_SHOOT1);
	else
		SendWeaponAnim(G3SG1_SHOOT2);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/g3sg1-1.wav",
		RANDOM_FLOAT(0.92f, 1.0f), 0.4f, 0, 100);

	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

	m_pPlayer->pev->punchangle.x -= 2.0f;

	Vector vecSrc = m_pPlayer->GetGunPosition();

	if (m_pPlayer->m_bLeftHanded == 1)
	{
		float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
		Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

		float flUpScale = RANDOM_FLOAT(35.0f, 55.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

		float flForwardScale = RANDOM_FLOAT(40.0f, 55.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

		Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 8.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 20.0f;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 10.0f;

		EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
	}
	else
	{
		float flRightScale = RANDOM_FLOAT(100.0f, 120.0f);
		Vector vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

		float flUpScale = RANDOM_FLOAT(35.0f, 55.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

		float flForwardScale = RANDOM_FLOAT(40.0f, 55.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

		Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 8.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 20.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 10.0f;

		EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
	}

	m_pPlayer->FireBullets3(
		vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192.0f, 3,
		BULLET_PLAYER_762MM, 60, 0.98f, NULL, true);
}

void CG3SG1::Reload( void )
{
	if (DefaultReload(20, G3SG1_RELOAD, 4.6f))
	{
		if (m_pPlayer->m_iFOV != 90)
		{
			m_pPlayer->m_iFOV = 15;
			SecondaryAttack();
		}

		m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
	}
}

void CG3SG1::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = gpGlobals->time + 60.0f;
		SendWeaponAnim(G3SG1_IDLE);
	}
}

float CG3SG1::GetMaxSpeed( void )
{
	if (m_pPlayer->m_iFOV == 90)
		return 200.0f;
	return 140.0f;
}

