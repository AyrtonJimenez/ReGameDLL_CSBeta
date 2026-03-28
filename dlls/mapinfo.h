#ifndef MAPINFO_H
#define MAPINFO_H

class CMapInfo : public CPointEntity
{
public:
	
	void Spawn( void );
	
	void KeyValue( KeyValueData *pkvd );

	int m_iBuyingStatus;		
	
	float m_flBombRadius;		
};

#endif 

