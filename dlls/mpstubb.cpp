#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"soundent.h"
#include	"nodes.h"
#include	"talkmonster.h"

float	CTalkMonster::g_talkWaitTime = 0;

CGraph	WorldGraph;

void CGraph::InitGraph(void) { }

int CGraph::FLoadGraph(char* szMapName) { return FALSE; }

int CGraph::AllocNodes(void) { return FALSE; }

int CGraph::CheckNODFile(char* szMapName) { return FALSE; }

int CGraph::FSetGraphPointers(void) { return 0; }

void CGraph::ShowNodeConnections(int iNode) { }

int	CGraph::FindNearestNode(const Vector& vecOrigin, int afNodeTypes) { return 0; }

void	CBaseMonster::ReportAIState(void) { }

float 	CBaseMonster::ChangeYaw(int speed) { return 0; }

void	CBaseMonster::MakeIdealYaw(Vector vecTarget) { }

BOOL	CBaseMonster::ShouldFadeOnDeath(void)
{
    return FALSE;
}

BOOL	CBaseMonster::FCheckAITrigger(void)
{
    return FALSE;
}

