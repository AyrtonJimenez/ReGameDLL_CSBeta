#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "hintmessage.h"

void CHintMessageQueue::Reset(void)
{
    m_iCount = 0;
    m_iHead = 0;
    m_iTail = 1;
    m_flNextTime = 0;

    for (int i = 0; i < MAX_HINT_MESSAGES; i++)
    {
        m_pMessages[i] = NULL;
    }
}

void CHintMessageQueue::Update(CBaseEntity* pPlayer)
{
    if (m_flNextTime > gpGlobals->time || !m_iCount)
        return;

    m_pMessages[m_iHead] = NULL;
    m_iCount--;

    int oldHead = m_iHead;
    m_iHead = (m_iHead + 1) % MAX_HINT_MESSAGES;
    m_flNextTime = gpGlobals->time + 6.0f;

    const char* pMsg = m_pMessages[m_iHead];
    if (pMsg)
    {
        int len = strlen(pMsg);
        float flHoldTime = (float)(len / 25);
        if (flHoldTime < 1.0f)
            flHoldTime = 1.0f;

        hudtextparms_t textParms;
        textParms.x = -1;
        textParms.y = 0.7f;
        textParms.effect = 2;
        textParms.r1 = 40; textParms.g1 = 255; textParms.b1 = 40; textParms.a1 = 200;
        textParms.r2 = 0; textParms.g2 = 255; textParms.b2 = 0; textParms.a2 = 200;
        textParms.fadeinTime = 0.01f;
        textParms.fadeoutTime = 0.7f;
        textParms.holdTime = flHoldTime;
        textParms.fxTime = 0.07f;
        textParms.channel = 1;

        UTIL_HudMessage(pPlayer, textParms, pMsg);
    }
}

BOOL CHintMessageQueue::AddMessage(const char* pMessage)
{
    if (m_iCount >= MAX_HINT_MESSAGES)
        return FALSE;

    m_pMessages[m_iTail] = pMessage;
    m_iTail = (m_iTail + 1) % MAX_HINT_MESSAGES;
    m_iCount++;

    return TRUE;
}

