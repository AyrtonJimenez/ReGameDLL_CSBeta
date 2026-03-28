#ifdef REGAMEDLL_ADD
#ifndef HOSTAGE_LOCALNAV_H
#define HOSTAGE_LOCALNAV_H

#ifdef REGAMEDLL_FIXES
#define MAX_NODES           200
#else
#define MAX_NODES           100
#endif
#define MAX_HOSTAGES_NAV    20
#define HOSTAGE_STEPSIZE    26.0f

#define MaxUnitZSlope       0.7f

enum PathTraversAble
{
	PTRAVELS_EMPTY,         
	PTRAVELS_SLOPE,         
	PTRAVELS_STEP,          
	PTRAVELS_STEPJUMPABLE,  
};

typedef int node_index_t;
#define NODE_INVALID_EMPTY  (-1)

typedef struct localnode_s
{
	Vector vecLoc;           
	int offsetX;             
	int offsetY;             
	byte bDepth;             
	BOOL fSearched;          
	node_index_t nindexParent; 

} localnode_t;

class CLocalNav
{
public:
		CLocalNav(CHostage *pOwner);
		virtual ~CLocalNav();

		void SetTargetEnt(CBaseEntity *pTarget)
	{
		
		if (pTarget)
			m_pTargetEnt = pTarget->edict();
		else
			m_pTargetEnt = NULL;
	}

		node_index_t FindPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, BOOL fNoMonsters);
		int SetupPathNodes(node_index_t nindex, Vector *vecNodes, BOOL fNoMonsters);
		node_index_t GetFurthestTraversableNode(Vector &vecStartingLoc, Vector *vecNodes, int nTotalNodes, BOOL fNoMonsters);
		PathTraversAble PathTraversable(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters);
		BOOL PathClear(Vector &vecOrigin, Vector &vecDest, BOOL fNoMonsters, TraceResult &tr);
		BOOL PathClear(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters);
		node_index_t AddNode(node_index_t nindexParent, Vector &vecLoc, int offsetX = 0, int offsetY = 0, byte bDepth = 0);
		localnode_t *GetNode(node_index_t nindex);
		node_index_t NodeExists(int offsetX, int offsetY);
		void AddPathNodes(node_index_t nindexSource, BOOL fNoMonsters);
		void AddPathNode(node_index_t nindexSource, int offsetX, int offsetY, BOOL fNoMonsters);
		node_index_t GetBestNode(Vector &vecOrigin, Vector &vecDest);
		BOOL SlopeTraversable(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters, TraceResult &tr);
		BOOL LadderTraversable(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters, TraceResult &tr);
		BOOL StepTraversable(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters, TraceResult &tr);
		BOOL StepJumpable(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters, TraceResult &tr);
		node_index_t FindDirectPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, BOOL fNoMonsters);
		BOOL LadderHit(Vector &vecSource, Vector &vecDest, TraceResult &tr);

		static void Think();
		static void RequestNav(CHostage *pCaller);
		static void Reset();
		static void HostagePrethink();
	
	static float m_flStepSize;

private:
	
	static EHANDLE m_hQueue[MAX_HOSTAGES_NAV];
	
	static EHANDLE m_hHostages[MAX_HOSTAGES_NAV];
	
	static int m_CurRequest;
	
	static int m_NumRequest;
	
	static int m_NumHostages;
	
	static int m_NodeValue;
	
	static float m_flNextCvarCheck;
	
	static float m_flLastThinkTime;

	CHostage *m_pOwner;
	
	edict_t *m_pTargetEnt;
	
	BOOL m_fTargetEntHit;
	
	localnode_t *m_nodeArr;
	
	node_index_t m_nindexAvailableNode;
	
	Vector m_vecStartingLoc;
};

#endif 
#endif

