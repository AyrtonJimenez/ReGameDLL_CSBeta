#ifndef HINTMESSAGE_H
#define HINTMESSAGE_H

#define MAX_HINT_MESSAGES	8

class CHintMessageQueue
{
public:
	
	void Reset( void );
	
	void Update( CBaseEntity *pPlayer );
	
	BOOL AddMessage( const char *pMessage );

private:
	int m_iCount;						
	int m_iHead;						
	int m_iTail;						
	float m_flNextTime;					
	const char *m_pMessages[MAX_HINT_MESSAGES];	
};

#endif 

