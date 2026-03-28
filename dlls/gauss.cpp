#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "shake.h"
#include "gamerules.h"

#define	GAUSS_PRIMARY_CHARGE_VOLUME	256
#define GAUSS_PRIMARY_FIRE_VOLUME	450

enum gauss_e {
    GAUSS_IDLE = 0,
    GAUSS_IDLE2,
    GAUSS_FIDGET,
    GAUSS_SPINUP,
    GAUSS_SPIN,
    GAUSS_FIRE,
    GAUSS_FIRE2,
    GAUSS_HOLSTER,
    GAUSS_DRAW
};

class CGauss : public CBasePlayerWeapon
{
public:
    int		Save(CSave& save);
    int		Restore(CRestore& restore);
    static	TYPEDESCRIPTION m_SaveData[];

    void Spawn(void);
    void Precache(void);
    int iItemSlot(void) { return 4; }
    int GetItemInfo(ItemInfo* p);
    int AddToPlayer(CBasePlayer* pPlayer);

    BOOL Deploy(void);
    void Holster(void);

    void PrimaryAttack(void);
    void SecondaryAttack(void);
    void WeaponIdle(void);

    int m_fInAttack;
    float m_flStartCharge;
    float m_flPlayAftershock;
    void StartFire(void);
    void Fire(Vector vecOrigSrc, Vector vecDirShooting, float flDamage);
    float GetFullChargeTime(void);
    int m_iBalls;
    int m_iGlow;
    int m_iBeam;
    int m_iSoundState;

    float m_flNextAmmoBurn;

    BOOL m_fPrimaryFire;
};
LINK_ENTITY_TO_CLASS(weapon_gauss, CGauss);

TYPEDESCRIPTION	CGauss::m_SaveData[] =
{
    DEFINE_FIELD(CGauss, m_fInAttack, FIELD_INTEGER),
    DEFINE_FIELD(CGauss, m_flStartCharge, FIELD_TIME),
    DEFINE_FIELD(CGauss, m_flPlayAftershock, FIELD_TIME),
    DEFINE_FIELD(CGauss, m_flNextAmmoBurn, FIELD_TIME),
    DEFINE_FIELD(CGauss, m_fPrimaryFire, FIELD_BOOLEAN),
};
IMPLEMENT_SAVERESTORE(CGauss, CBasePlayerWeapon);

float CGauss::GetFullChargeTime(void)
{
    if (g_pGameRules->IsMultiplayer())
    {
        return 1.5;
    }

    return 4;
}

void CGauss::Spawn()
{
    Precache();
    m_iId = WEAPON_GAUSS;
    SET_MODEL(ENT(pev), "models/w_gauss.mdl");

    m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;

    FallInit();
}

void CGauss::Precache(void)
{
    PRECACHE_MODEL("models/w_gauss.mdl");
    PRECACHE_MODEL("models/v_gauss.mdl");
    PRECACHE_MODEL("models/p_gauss.mdl");

    PRECACHE_SOUND("items/9mmclip1.wav");

    PRECACHE_SOUND("weapons/gauss2.wav");
    PRECACHE_SOUND("weapons/electro4.wav");
    PRECACHE_SOUND("weapons/electro5.wav");
    PRECACHE_SOUND("weapons/electro6.wav");
    PRECACHE_SOUND("ambience/pulsemachine.wav");

    m_iGlow = PRECACHE_MODEL("sprites/hotglow.spr");
    m_iBalls = PRECACHE_MODEL("sprites/hotglow.spr");
    m_iBeam = PRECACHE_MODEL("sprites/smoke.spr");
}

int CGauss::AddToPlayer(CBasePlayer* pPlayer)
{
    if (CBasePlayerWeapon::AddToPlayer(pPlayer))
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->edict());
        WRITE_BYTE(m_iId);
        MESSAGE_END();
        return TRUE;
    }
    return FALSE;
}

int CGauss::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "uranium";
    p->iMaxAmmo1 = URANIUM_MAX_CARRY;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;
    p->iSlot = 3;
    p->iPosition = 1;
    p->iId = m_iId = WEAPON_GAUSS;
    p->iFlags = 0;
    p->iWeight = GAUSS_WEIGHT;

    return 1;
}

BOOL CGauss::Deploy()
{
    return DefaultDeploy("models/v_gauss.mdl", "models/p_gauss.mdl", GAUSS_DRAW, "gauss");
}

void CGauss::Holster()
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;

    SendWeaponAnim(GAUSS_HOLSTER);
    m_fInAttack = 0;
    EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CGauss::PrimaryAttack()
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        PlayEmptySound();
        m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->time + 0.15;
        return;
    }

    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 2)
    {
        PlayEmptySound();
        m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
        return;
    }

    m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
    m_fPrimaryFire = TRUE;
    m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 2;

    StartFire();
    m_fInAttack = 0;
    m_flTimeWeaponIdle = gpGlobals->time + 1.0;
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.2;
}

void CGauss::SecondaryAttack()
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        if (m_fInAttack != 0)
        {
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
            SendWeaponAnim(GAUSS_IDLE);
            m_fInAttack = 0;
        }
        else
        {
            PlayEmptySound();
        }

        m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->time + 0.5;
        return;
    }

    if (m_fInAttack == 0)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        {
            EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
            m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
            return;
        }

        m_fPrimaryFire = FALSE;

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_flNextAmmoBurn = gpGlobals->time;

        m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

        SendWeaponAnim(GAUSS_SPINUP);
        m_fInAttack = 1;
        m_flTimeWeaponIdle = gpGlobals->time + 0.5;
        m_flStartCharge = gpGlobals->time;
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, 0, 110);
        m_iSoundState = SND_CHANGE_PITCH;
    }
    else if (m_fInAttack == 1)
    {
        if (m_flTimeWeaponIdle < gpGlobals->time)
        {
            SendWeaponAnim(GAUSS_SPIN);
            m_fInAttack = 2;
        }
    }
    else
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
        {
            StartFire();
            m_fInAttack = 0;
            m_flTimeWeaponIdle = gpGlobals->time + 1.0;
            m_pPlayer->m_flNextAttack = gpGlobals->time + 1;
            return;
        }

        if (gpGlobals->time > m_flNextAmmoBurn && m_flNextAmmoBurn != -1)
        {
            if (g_pGameRules->IsMultiplayer())
            {
                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
                m_flNextAmmoBurn = gpGlobals->time + 0.1;
            }
            else
            {
                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
                m_flNextAmmoBurn = gpGlobals->time + 0.3;
            }
        }

        if (gpGlobals->time - m_flStartCharge >= GetFullChargeTime())
        {
            m_flNextAmmoBurn = -1;
        }

        int pitch = (gpGlobals->time - m_flStartCharge) * (150 / GetFullChargeTime()) + 100;
        if (pitch > 250)
            pitch = 250;

        if (m_iSoundState == 0)
            ALERT(at_console, "sound state %d\n", m_iSoundState);

        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, m_iSoundState, pitch);

        m_iSoundState = SND_CHANGE_PITCH;

        m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

        if (m_flStartCharge < gpGlobals->time - 10)
        {
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/electro6.wav", 1.0, ATTN_NORM, 0, 75 + RANDOM_LONG(0, 0x3f));

            m_fInAttack = 0;
            m_flTimeWeaponIdle = gpGlobals->time + 1.0;
            m_pPlayer->m_flNextAttack = gpGlobals->time + 1.0;
            m_pPlayer->TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 50, DMG_SHOCK);

            UTIL_ScreenFade(m_pPlayer, Vector(255, 128, 0), 2, 0.5, 128, FFADE_IN);
            SendWeaponAnim(GAUSS_IDLE);

            return;
        }
    }
}

void CGauss::StartFire(void)
{
    float flDamage;

    UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
    Vector vecAiming = gpGlobals->v_forward;
    Vector vecSrc = m_pPlayer->GetGunPosition();

    if (gpGlobals->time - m_flStartCharge > GetFullChargeTime())
    {
        flDamage = 200;
    }
    else
    {
        flDamage = 200 * ((gpGlobals->time - m_flStartCharge) / GetFullChargeTime());
    }

    if (m_fPrimaryFire)
    {
        flDamage = gSkillData.plrDmgGauss;
        m_pPlayer->pev->punchangle.x = -2;
    }

    if (m_fInAttack != 3)
    {
        float flZVel = m_pPlayer->pev->velocity.z;

        if (!m_fPrimaryFire)
        {
            m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * flDamage * 5;
        }

        if (!g_pGameRules->IsDeathmatch())
        {

            m_pPlayer->pev->velocity.z = flZVel;
        }

        SendWeaponAnim(GAUSS_FIRE2);

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
    }
    STOP_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "ambience/pulsemachine.wav");
    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

    m_flPlayAftershock = gpGlobals->time + RANDOM_FLOAT(0.3, 0.8);

    Fire(vecSrc, vecAiming, flDamage);
}

void CGauss::Fire(Vector vecOrigSrc, Vector vecDir, float flDamage)
{
    m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;

    Vector vecSrc = vecOrigSrc;
    Vector vecDest = vecSrc + vecDir * 8192;
    edict_t* pentIgnore;
    TraceResult tr, beam_tr;
    float flMaxFrac = 1.0;
    int	nTotal = 0;
    int fHasPunched = 0;
    int fFirstBeam = 1;
    int	nMaxHits = 10;

    pentIgnore = ENT(m_pPlayer->pev);

    while (flDamage > 10 && nMaxHits > 0)
    {
        nMaxHits--;

        UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);

        if (tr.fAllSolid)
            break;

        CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

        if (pEntity == NULL)
            break;

        if (fFirstBeam)
        {
            m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
            fFirstBeam = 0;

            Vector tmpSrc = vecSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
            WRITE_BYTE(TE_BEAMENTPOINT);
            WRITE_SHORT(m_pPlayer->entindex() + 0x1000);
            WRITE_COORD(tr.vecEndPos.x);
            WRITE_COORD(tr.vecEndPos.y);
            WRITE_COORD(tr.vecEndPos.z);
            WRITE_SHORT(m_iBeam);
            WRITE_BYTE(0);
            WRITE_BYTE(0);
            WRITE_BYTE(1);

            if (m_fPrimaryFire)
            {
                WRITE_BYTE(10);
            }
            else
            {
                WRITE_BYTE(25);
            }

            WRITE_BYTE(0);

            if (m_fPrimaryFire)
            {
                WRITE_BYTE(255);
                WRITE_BYTE(128);
                WRITE_BYTE(0);

                WRITE_BYTE(128);
            }
            else
            {
                WRITE_BYTE(255);
                WRITE_BYTE(255);
                WRITE_BYTE(255);

                WRITE_BYTE(flDamage);
            }

            WRITE_BYTE(0);
            MESSAGE_END();

            nTotal += 26;
        }
        else
        {
            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecSrc);
            WRITE_BYTE(TE_BEAMPOINTS);
            WRITE_COORD(vecSrc.x);
            WRITE_COORD(vecSrc.y);
            WRITE_COORD(vecSrc.z);
            WRITE_COORD(tr.vecEndPos.x);
            WRITE_COORD(tr.vecEndPos.y);
            WRITE_COORD(tr.vecEndPos.z);
            WRITE_SHORT(m_iBeam);
            WRITE_BYTE(0);
            WRITE_BYTE(0);
            WRITE_BYTE(1);

            if (m_fPrimaryFire)
            {
                WRITE_BYTE(10);
            }
            else
            {
                WRITE_BYTE(25);
            }

            WRITE_BYTE(0);

            if (m_fPrimaryFire)
            {

                WRITE_BYTE(255);
                WRITE_BYTE(128);
                WRITE_BYTE(0);

                WRITE_BYTE(128);
            }
            else
            {
                WRITE_BYTE(255);
                WRITE_BYTE(255);
                WRITE_BYTE(255);

                WRITE_BYTE(flDamage);
            }

            WRITE_BYTE(0);
            MESSAGE_END();
            nTotal += 26;
        }

        if (pEntity->pev->takedamage)
        {
            ClearMultiDamage();
            pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDir, &tr, DMG_BULLET);
            ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
        }

        if (pEntity->ReflectGauss())
        {
            float n;

            pentIgnore = NULL;

            n = -DotProduct(tr.vecPlaneNormal, vecDir);

            if (n < 0.5)
            {
                Vector r;

                r = 2.0 * tr.vecPlaneNormal * n + vecDir;
                flMaxFrac = flMaxFrac - tr.flFraction;
                vecDir = r;
                vecSrc = tr.vecEndPos + vecDir * 8;
                vecDest = vecSrc + vecDir * 8192;

                m_pPlayer->RadiusDamage(tr.vecEndPos, pev, m_pPlayer->pev, flDamage * n, CLASS_NONE, DMG_BLAST);

                MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
                WRITE_BYTE(TE_GLOWSPRITE);
                WRITE_COORD(tr.vecEndPos.x);
                WRITE_COORD(tr.vecEndPos.y);
                WRITE_COORD(tr.vecEndPos.z);
                WRITE_SHORT(m_iGlow);
                WRITE_BYTE(flDamage * n * 0.5);
                WRITE_BYTE(2);
                WRITE_BYTE(flDamage * n);
                MESSAGE_END();

                nTotal += 13;

                MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos);
                WRITE_BYTE(TE_SPRITETRAIL);
                WRITE_COORD(tr.vecEndPos.x);
                WRITE_COORD(tr.vecEndPos.y);
                WRITE_COORD(tr.vecEndPos.z);
                WRITE_COORD(tr.vecEndPos.x + tr.vecPlaneNormal.x);
                WRITE_COORD(tr.vecEndPos.y + tr.vecPlaneNormal.y);
                WRITE_COORD(tr.vecEndPos.z + tr.vecPlaneNormal.z);
                WRITE_SHORT(m_iBalls);
                WRITE_BYTE(n * flDamage * 0.3);
                WRITE_BYTE(10);
                WRITE_BYTE(RANDOM_LONG(1, 2));
                WRITE_BYTE(10);
                WRITE_BYTE(20);
                MESSAGE_END();

                nTotal += 21;

                if (n == 0) n = 0.1;
                flDamage = flDamage * (1 - n);
            }
            else
            {

                DecalGunshot(&tr, BULLET_MONSTER_12MM);

                MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
                WRITE_BYTE(TE_GLOWSPRITE);
                WRITE_COORD(tr.vecEndPos.x);
                WRITE_COORD(tr.vecEndPos.y);
                WRITE_COORD(tr.vecEndPos.z);
                WRITE_SHORT(m_iGlow);
                WRITE_BYTE(60);
                WRITE_BYTE(10);
                WRITE_BYTE(flDamage);
                MESSAGE_END();
                nTotal += 13;

                if (fHasPunched)
                    break;
                fHasPunched = 1;

                if (!m_fPrimaryFire)
                {
                    UTIL_TraceLine(tr.vecEndPos + vecDir * 8, vecDest, dont_ignore_monsters, pentIgnore, &beam_tr);
                    if (!beam_tr.fAllSolid)
                    {
                        UTIL_TraceLine(beam_tr.vecEndPos, tr.vecEndPos, dont_ignore_monsters, pentIgnore, &beam_tr);

                        float n = (beam_tr.vecEndPos - tr.vecEndPos).Length();

                        if (n < flDamage)
                        {
                            if (n == 0) n = 1;
                            flDamage -= n;

                            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
                            WRITE_BYTE(TE_SPRITETRAIL);
                            WRITE_COORD(tr.vecEndPos.x);
                            WRITE_COORD(tr.vecEndPos.y);
                            WRITE_COORD(tr.vecEndPos.z);
                            WRITE_COORD(tr.vecEndPos.x - vecDir.x);
                            WRITE_COORD(tr.vecEndPos.y - vecDir.y);
                            WRITE_COORD(tr.vecEndPos.z - vecDir.z);
                            WRITE_SHORT(m_iBalls);
                            WRITE_BYTE(3);
                            WRITE_BYTE(10);
                            WRITE_BYTE(RANDOM_LONG(1, 2));
                            WRITE_BYTE(10);
                            WRITE_BYTE(1);
                            MESSAGE_END();
                            nTotal += 21;

                            m_pPlayer->RadiusDamage(beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, flDamage, CLASS_NONE, DMG_BLAST);
                            CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0);

                            DecalGunshot(&beam_tr, BULLET_MONSTER_12MM);
                            nTotal += 19;

                            MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, beam_tr.vecEndPos);
                            WRITE_BYTE(TE_GLOWSPRITE);
                            WRITE_COORD(beam_tr.vecEndPos.x);
                            WRITE_COORD(beam_tr.vecEndPos.y);
                            WRITE_COORD(beam_tr.vecEndPos.z);
                            WRITE_SHORT(m_iGlow);
                            WRITE_BYTE(60);
                            WRITE_BYTE(10);
                            WRITE_BYTE(flDamage);
                            MESSAGE_END();
                            nTotal += 13;

                            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, beam_tr.vecEndPos);
                            WRITE_BYTE(TE_SPRITETRAIL);
                            WRITE_COORD(beam_tr.vecEndPos.x);
                            WRITE_COORD(beam_tr.vecEndPos.y);
                            WRITE_COORD(beam_tr.vecEndPos.z);
                            WRITE_COORD(beam_tr.vecEndPos.x + vecDir.x);
                            WRITE_COORD(beam_tr.vecEndPos.y + vecDir.y);
                            WRITE_COORD(beam_tr.vecEndPos.z + vecDir.z);
                            WRITE_SHORT(m_iBalls);
                            WRITE_BYTE(flDamage * 0.3);
                            WRITE_BYTE(10);
                            WRITE_BYTE(RANDOM_LONG(1, 2));
                            WRITE_BYTE(20);
                            WRITE_BYTE(40);
                            MESSAGE_END();
                            nTotal += 21;

                            vecSrc = beam_tr.vecEndPos + vecDir;
                        }
                    }
                    else
                    {
                        flDamage = 0;
                    }
                }
                else
                {

                    if (m_fPrimaryFire)
                    {
                        MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos);
                        WRITE_BYTE(TE_GLOWSPRITE);
                        WRITE_COORD(tr.vecEndPos.x);
                        WRITE_COORD(tr.vecEndPos.y);
                        WRITE_COORD(tr.vecEndPos.z);
                        WRITE_SHORT(m_iGlow);
                        WRITE_BYTE(20);
                        WRITE_BYTE(3);
                        WRITE_BYTE(200);
                        MESSAGE_END();

                        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos);
                        WRITE_BYTE(TE_SPRITETRAIL);
                        WRITE_COORD(tr.vecEndPos.x);
                        WRITE_COORD(tr.vecEndPos.y);
                        WRITE_COORD(tr.vecEndPos.z);
                        WRITE_COORD(tr.vecEndPos.x + tr.vecPlaneNormal.x);
                        WRITE_COORD(tr.vecEndPos.y + tr.vecPlaneNormal.y);
                        WRITE_COORD(tr.vecEndPos.z + tr.vecPlaneNormal.z);
                        WRITE_SHORT(m_iBalls);
                        WRITE_BYTE(8);
                        WRITE_BYTE(6);
                        WRITE_BYTE(RANDOM_LONG(1, 2));
                        WRITE_BYTE(10);
                        WRITE_BYTE(20);
                        MESSAGE_END();
                    }

                    flDamage = 0;
                }

            }
        }
        else
        {
            vecSrc = tr.vecEndPos + vecDir;
            pentIgnore = ENT(pEntity->pev);
        }
    }

}

void CGauss::WeaponIdle(void)
{
    ResetEmptySound();

    if (m_flPlayAftershock && m_flPlayAftershock < gpGlobals->time)
    {
        switch (RANDOM_LONG(0, 3))
        {
        case 0:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
        case 1:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro5.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
        case 2:	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro6.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM); break;
        case 3:	break;
        }
        m_flPlayAftershock = 0.0;
    }

    if (m_flTimeWeaponIdle > gpGlobals->time)
        return;

    if (m_fInAttack != 0)
    {
        StartFire();
        m_fInAttack = 0;
        m_flTimeWeaponIdle = gpGlobals->time + 2.0;
    }
    else
    {
        int iAnim;
        float flRand = RANDOM_FLOAT(0, 1);
        if (flRand <= 0.5)
        {
            iAnim = GAUSS_IDLE;
            m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
        }
        else if (flRand <= 0.75)
        {
            iAnim = GAUSS_IDLE2;
            m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
        }
        else
        {
            iAnim = GAUSS_FIDGET;
            m_flTimeWeaponIdle = gpGlobals->time + 3;
        }

        return;
        SendWeaponAnim(iAnim);
    }
}

class CGaussAmmo : public CBasePlayerAmmo
{
    void Spawn(void)
    {
        Precache();
        SET_MODEL(ENT(pev), "models/w_gaussammo.mdl");
        CBasePlayerAmmo::Spawn();
    }
    void Precache(void)
    {
        PRECACHE_MODEL("models/w_gaussammo.mdl");
        PRECACHE_SOUND("items/9mmclip1.wav");
    }
    BOOL AddAmmo(CBaseEntity* pOther)
    {
        if (pOther->GiveAmmo(AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY) != -1)
        {
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
            return TRUE;
        }
        return FALSE;
    }
};
LINK_ENTITY_TO_CLASS(ammo_gaussclip, CGaussAmmo);

#endif

