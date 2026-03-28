#include "extdll.h"
#include "util.h"

#include "cbase.h"

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
    }

    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }

    return TRUE;
}

enginefuncs_t g_engfuncs;
globalvars_t* gpGlobals;

void DLLEXPORT GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals)
{
    memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));

    gpGlobals = pGlobals;
}

