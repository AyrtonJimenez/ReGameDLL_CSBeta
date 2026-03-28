#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "mapinfo.h"

LINK_ENTITY_TO_CLASS(info_map_parameters, CMapInfo);

void CMapInfo::Spawn(void)
{
    pev->movetype = MOVETYPE_NONE;
    pev->solid = SOLID_NOT;
    pev->effects |= EF_NODRAW;
}

void CMapInfo::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "buying"))
    {
        m_iBuyingStatus = atoi(pkvd->szValue);

        pkvd->fHandled = TRUE;
    }

    else if (FStrEq(pkvd->szKeyName, "bombradius"))
    {
        m_flBombRadius = (float)atoi(pkvd->szValue);

        if (m_flBombRadius > 2048.0f)
            m_flBombRadius = 2048.0f;

        pkvd->fHandled = TRUE;
    }
}

