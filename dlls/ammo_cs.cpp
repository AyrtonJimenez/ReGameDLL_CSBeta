#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define DECLARE_CS_AMMO(className, entityName, modelPath, soundPath, ammoName, ammoGive, ammoMax) \
class className : public CBasePlayerAmmo \
{ \
public: \
	void Spawn( void ); \
	void Precache( void ); \
	BOOL AddAmmo( CBaseEntity *pOther ); \
}; \
LINK_ENTITY_TO_CLASS( entityName, className ); \
void className::Spawn( void ) \
{ \
	 \
	Precache(); \
	 \
	SET_MODEL(ENT(pev), modelPath); \
	 \
	CBasePlayerAmmo::Spawn(); \
} \
void className::Precache( void ) \
{ \
	 \
	PRECACHE_MODEL(modelPath); \
	 \
	PRECACHE_SOUND(soundPath); \
} \
BOOL className::AddAmmo( CBaseEntity *pOther ) \
{ \
	 \
	if (pOther->GiveAmmo(ammoGive, ammoName, ammoMax) == -1) \
		return FALSE; \
	 \
	EMIT_SOUND(ENT(pev), CHAN_ITEM, soundPath, 1, ATTN_NORM); \
	return TRUE; \
}

DECLARE_CS_AMMO(C9MMAmmo, ammo_9mm, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "9mm", 30, 120)

DECLARE_CS_AMMO(C45ACPAmmo, ammo_45acp, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "45acp", 12, 90)

DECLARE_CS_AMMO(C50AEAmmo, ammo_50ae, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "50AE", 7, 35)

DECLARE_CS_AMMO(C556NatoAmmo, ammo_556nato, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "556Nato", 30, 90)

DECLARE_CS_AMMO(C556NatoBoxAmmo, ammo_556natobox, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "556NatoBox", 30, 200)

DECLARE_CS_AMMO(C762NatoAmmo, ammo_762nato, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "762Nato", 30, 90)

DECLARE_CS_AMMO(C338MagnumAmmo, ammo_338magnum, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "338Magnum", 10, 30)

DECLARE_CS_AMMO(C57MMAmmo, ammo_57mm, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "57mm", 50, 100)

DECLARE_CS_AMMO(C357SIGAmmo, ammo_357sig, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "357SIG", 13, 52)

DECLARE_CS_AMMO(CBuckShotAmmo, ammo_buckshot, "models/w_shotbox.mdl", "items/9mmclip1.wav", "buckshot", 8, 32)

DECLARE_CS_AMMO(CFlashbangAmmo, ammo_flashbang, "models/w_9mmclip.mdl", "items/9mmclip1.wav", "Flashbang", 1, 2)

