#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

extern void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, float* mins, float* maxs, edict_t* pEntity);

class CKnife : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int GetItemInfo(ItemInfo* p);
    BOOL Deploy(void);
    void Holster(void);
    void PrimaryAttack(void);
    void EXPORT Smack(void);
    void EXPORT SwingAgain(void);
    BOOL CanDrop(void) { return FALSE; }
    float GetMaxSpeed(void);
    int iItemSlot(void) { return 3; }
    int Swing(int fFirst);

    int m_iSwing;

    TraceResult m_trHit;
};

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);

void CKnife::Spawn()
{
    Precache();
    m_iId = WEAPON_KNIFE;
    SET_MODEL(ENT(pev), "models/w_knife.mdl");

    m_iClip = WEAPON_NOCLIP;
    FallInit();
}

void CKnife::Precache()
{
    PRECACHE_MODEL("models/v_knife.mdl");
    PRECACHE_MODEL("models/v_knife_r.mdl");
    PRECACHE_MODEL("models/w_knife.mdl");
    PRECACHE_MODEL("models/p_knife.mdl");

    PRECACHE_SOUND("weapons/knife_deploy1.wav");
    PRECACHE_SOUND("weapons/knife_hit1.wav");
    PRECACHE_SOUND("weapons/knife_hit2.wav");
    PRECACHE_SOUND("weapons/knife_hit3.wav");
    PRECACHE_SOUND("weapons/knife_hit4.wav");
    PRECACHE_SOUND("weapons/knife_slash1.wav");
    PRECACHE_SOUND("weapons/knife_slash2.wav");
    PRECACHE_SOUND("weapons/knife_hitwall1.wav");
}

int CKnife::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = WEAPON_NOCLIP;

    p->iSlot = 2;
    p->iPosition = 1;
    p->iId = WEAPON_KNIFE;
    p->iWeight = 0;
    return 1;
}

BOOL CKnife::Deploy()
{
    EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON,
        "weapons/knife_deploy1.wav", 1.0f, 0.8f, 0, 100);

    if (m_pPlayer->m_bLeftHanded == 1)
        return DefaultDeploy("models/v_knife.mdl", "models/p_knife.mdl", 3, "crowbar");
    else
        return DefaultDeploy("models/v_knife_r.mdl", "models/p_knife.mdl", 3, "crowbar");
}

void CKnife::Holster()
{
    m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5f;
}

void CKnife::PrimaryAttack()
{
    if (!Swing(TRUE))
    {
        SetThink(&CKnife::SwingAgain);
        pev->nextthink = gpGlobals->time + 0.1f;
    }
}

int CKnife::Swing(int fFirst)
{
    int fDidHit = FALSE;

    TraceResult tr;

    UTIL_MakeVectors(m_pPlayer->pev->v_angle);
    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

    UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

    if (tr.flFraction >= 1.0)
    {
        UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
        if (tr.flFraction < 1.0)
        {
            CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
            if (!pHit || pHit->IsBSPModel())
                FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
            vecEnd = tr.vecEndPos;
        }
    }

    if (tr.flFraction >= 1.0)
    {

        if (fFirst)
        {
            switch ((m_iSwing++) % 2)
            {
            case 0: SendWeaponAnim(1); break;
            case 1: SendWeaponAnim(2); break;
            }

            m_flNextPrimaryAttack = gpGlobals->time + 0.6f;

            if (RANDOM_LONG(0, 1))
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", 1, ATTN_NORM, 0, 94);
            else
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", 1, ATTN_NORM, 0, 94);

            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        }
    }
    else
    {
        fDidHit = TRUE;

        CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

        switch ((m_iSwing++) % 2)
        {
        case 0: SendWeaponAnim(1); break;
        case 1: SendWeaponAnim(2); break;
        }

        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        ClearMultiDamage();

        float flDamage;
        if ((m_flNextPrimaryAttack + 1.0f < gpGlobals->time) || g_pGameRules->IsMultiplayer())
            flDamage = 40.0f;
        else
            flDamage = 30.0f;

        pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
        ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

        float flVol = 1.0f;
        int fHitWorld = TRUE;

        m_flNextPrimaryAttack = gpGlobals->time + 0.4f;

        if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
        {
            switch (RANDOM_LONG(0, 3))
            {
            case 0: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit1.wav", 1, ATTN_NORM, 0, 100); break;
            case 1: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit2.wav", 1, ATTN_NORM, 0, 100); break;
            case 2: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit3.wav", 1, ATTN_NORM, 0, 100); break;
            case 3: EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit4.wav", 1, ATTN_NORM, 0, 100); break;
            }

            m_pPlayer->m_iWeaponVolume = 128;

            if (!pEntity->IsAlive())
                return TRUE;

            flVol = 0.1f;
            fHitWorld = FALSE;
        }

        if (fHitWorld)
        {
            float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

            if (g_pGameRules->IsMultiplayer())
                fvolbar = 1.0f;

            switch (RANDOM_LONG(0, 1))
            {
            case 0:
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
                break;
            case 1:
                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
                break;
            }
        }

        m_trHit = tr;
        SetThink(&CKnife::Smack);
        pev->nextthink = gpGlobals->time + 0.2f;

        m_pPlayer->m_iWeaponVolume = (int)(flVol * 512.0f);
    }

    return fDidHit;
}

void CKnife::Smack()
{
    DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR, false, m_pPlayer->pev, false);
}

void CKnife::SwingAgain() 
{ 
    Swing(FALSE);
}

float CKnife::GetMaxSpeed()
{

    return 250.0f;
}

