#include	"extdll.h"
#include	"util.h"
#include	"skill.h"

skilldata_t	gSkillData;

float GetSkillCvar(char* pName)
{
    int		iCount;
    float	flValue;
    char	szBuffer[64];

    iCount = sprintf(szBuffer, "%s%d", pName, gSkillData.iSkillLevel);

    flValue = CVAR_GET_FLOAT(szBuffer);

    if (flValue <= 0)
    {
        ALERT(at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer);
    }

    return flValue;
}

