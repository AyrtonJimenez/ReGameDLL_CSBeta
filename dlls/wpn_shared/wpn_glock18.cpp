#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum glock18_e
{
	GLOCK18_IDLE1 = 0,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT1,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,			
	GLOCK18_DRAW,			
	GLOCK18_HOLSTER,		
	GLOCK18_ADD_SILENCER,	
	GLOCK18_DRAW2,			
	GLOCK18_RELOAD2,		
};

class CGLOCK18 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int iItemSlot( void ) { return 2; }

	BOOL Deploy( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Reload( void );
	void WeaponIdle( void );
	float GetMaxSpeed( void ) { return 250.0f; }

	void GLOCK18Fire( float flSpread, float flCycleTime, BOOL bFireBurst );

	BOOL m_bBurstMode;
};

LINK_ENTITY_TO_CLASS( weapon_glock18, CGLOCK18 );

void CGLOCK18::Spawn( void )
{
	pev->classname = MAKE_STRING("weapon_glock18");
	Precache();
	m_iId = WEAPON_GLOCK18;
	SET_MODEL(ENT(pev), "models/w_glock18.mdl");
	m_iDefaultAmmo = 20;
	FallInit();
}

void CGLOCK18::Precache( void )
{
	PRECACHE_MODEL("models/v_glock18.mdl");
	PRECACHE_MODEL("models/v_glock18_r.mdl");
	PRECACHE_MODEL("models/w_glock18.mdl");
	PRECACHE_MODEL("models/p_glock18.mdl");

	PRECACHE_SOUND("weapons/glock18-1.wav");
	PRECACHE_SOUND("weapons/glock18-2.wav");
	PRECACHE_SOUND("weapons/clipout1.wav");
	PRECACHE_SOUND("weapons/clipin1.wav");
	PRECACHE_SOUND("weapons/sliderelease1.wav");
	PRECACHE_SOUND("weapons/slideback1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/de_clipin.wav");
	PRECACHE_SOUND("weapons/de_clipout.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/pshell.mdl");
}

int CGLOCK18::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 20;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK18;
	p->iWeight = 5;
	return 1;
}

BOOL CGLOCK18::Deploy( void )
{
	m_bBurstMode = FALSE;

	int iAnim;
	if (RANDOM_LONG(0, 1))
		iAnim = GLOCK18_DRAW;
	else
		iAnim = GLOCK18_DRAW2;

	if (m_pPlayer->m_bLeftHanded == 1)
		return DefaultDeploy("models/v_glock18.mdl", "models/p_glock18.mdl", iAnim, "onehanded");
	else
		return DefaultDeploy("models/v_glock18_r.mdl", "models/p_glock18.mdl", iAnim, "onehanded");
}

void CGLOCK18::SecondaryAttack( void )
{
	if (m_bBurstMode)
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Switched to semi-automatic");
		m_bBurstMode = FALSE;
	}
	else
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Switched to burst-fire");
		m_bBurstMode = TRUE;
	}

	m_flNextSecondaryAttack = gpGlobals->time + 0.3f;
}

void CGLOCK18::PrimaryAttack( void )
{
	if (m_bBurstMode)
	{
		GLOCK18Fire(0.015f, 0.5f, TRUE);
		return;
	}

	float flSpread;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		flSpread = 0.165f * (1.0f - m_pPlayer->m_flAccuracy);
	else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		flSpread = 0.2f * (1.0f - m_pPlayer->m_flAccuracy);
	else if (m_pPlayer->pev->flags & FL_DUCKING)
		flSpread = 0.075f * (1.0f - m_pPlayer->m_flAccuracy);
	else
		flSpread = 0.1f * (1.0f - m_pPlayer->m_flAccuracy);

	GLOCK18Fire(flSpread, 0.225f, FALSE);
}

void CGLOCK18::GLOCK18Fire(float flSpread, float flCycleTime, BOOL bFireBurst)
{
	if (bFireBurst)
		m_pPlayer->m_iBurstShotsFired = 0;

	if (m_pPlayer->m_flLastFire != 0.0f)
	{
		m_pPlayer->m_flAccuracy = (gpGlobals->time - m_pPlayer->m_flLastFire) * 0.2f + 0.6f;
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

	if (m_iClip == 0)
		SendWeaponAnim(GLOCK18_SHOOT_EMPTY);
	else if (bFireBurst)
	{
		if (RANDOM_LONG(0, 1))
			SendWeaponAnim(GLOCK18_SHOOT1);
		else
			SendWeaponAnim(GLOCK18_SHOOT2);
	}
	else
    {
        SendWeaponAnim(GLOCK18_SHOOT3);
	}

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	if (bFireBurst && m_iClip > 0)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glock18-1.wav",
			RANDOM_FLOAT(0.92f, 1.0f), 0.8f, 0, 100);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glock18-2.wav",
			RANDOM_FLOAT(0.92f, 1.0f), 0.8f, 0, 100);

	Vector vecAiming = gpGlobals->v_forward;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;

	m_pPlayer->pev->punchangle.x -= 2.0f;

	if (bFireBurst)
	{
		m_pPlayer->m_flBurstShootTime = gpGlobals->time + 0.1f;
		m_pPlayer->m_iBurstShotsFired++;
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();

	if (m_pPlayer->m_bLeftHanded == 1)
	{
		Vector vecShellVelocity;
		float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
		vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * flRightScale;

		float flUpScale = RANDOM_FLOAT(100.0f, 360.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

		float flForwardScale = RANDOM_FLOAT(25.0f, 200.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

		Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 12.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 16.0f;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_right * 6.0f;

		EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
	}
	else
	{
		Vector vecShellVelocity;
		float flRightScale = RANDOM_FLOAT(40.0f, 150.0f);
		vecShellVelocity = m_pPlayer->pev->velocity - gpGlobals->v_right * flRightScale;

		float flUpScale = RANDOM_FLOAT(100.0f, 360.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_up * flUpScale;

		float flForwardScale = RANDOM_FLOAT(25.0f, 200.0f);
		vecShellVelocity = vecShellVelocity + gpGlobals->v_forward * flForwardScale;

		Vector vecShellOrigin = pev->origin + m_pPlayer->pev->view_ofs;
		vecShellOrigin = vecShellOrigin - gpGlobals->v_up * 12.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_forward * 16.0f;
		vecShellOrigin = vecShellOrigin + gpGlobals->v_right * 6.0f;

		EjectBrass2(vecShellOrigin, vecShellVelocity, pev->angles.y, m_iShell, 1, m_pPlayer->pev);
	}

	m_pPlayer->FireBullets3(
		vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread),
		8192, 1, BULLET_PLAYER_9MM, 24, 0.87f, 0, true);
}

void CGLOCK18::Reload( void )
{
	int iAnim;
	if (RANDOM_LONG(0, 1))
		iAnim = GLOCK18_RELOAD;
	else
		iAnim = GLOCK18_RELOAD2;

	if (DefaultReload(20, iAnim, 2.2f))
	{
		m_flTimeWeaponIdle = RANDOM_FLOAT(10.0f, 15.0f) + gpGlobals->time;
	}
}

void CGLOCK18::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_iClip)
	{
		float flRand = RANDOM_FLOAT(0.0f, 1.0f);

		if (flRand <= 0.3f)
		{
			m_flTimeWeaponIdle = gpGlobals->time + 3.0625f;
			SendWeaponAnim(GLOCK18_IDLE3);
		}
		else if (flRand <= 0.6f)
		{
			m_flTimeWeaponIdle = gpGlobals->time + 3.75f;
			SendWeaponAnim(GLOCK18_IDLE1);
		}
		else
		{
			m_flTimeWeaponIdle = gpGlobals->time + 2.5f;
			SendWeaponAnim(GLOCK18_IDLE2);
		}
	}
}

