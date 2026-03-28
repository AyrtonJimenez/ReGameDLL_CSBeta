/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef PROGDEFS_H
#define PROGDEFS_H


typedef struct
{	
	float		time;
	float		frametime;
	float		force_retouch;
	string_t	mapname;
	string_t	startspot;
	float		deathmatch;
	float		coop;
	float		teamplay;
	float		serverflags;
	float		found_secrets;
	vec3_t		v_forward;
	vec3_t		v_up;
	vec3_t		v_right;
	float		trace_allsolid;
	float		trace_startsolid;
	float		trace_fraction;
	vec3_t		trace_endpos;
	vec3_t		trace_plane_normal;
	float		trace_plane_dist;
	edict_t		*trace_ent;
	float		trace_inopen;
	float		trace_inwater;
	int			trace_hitgroup;
	int			trace_flags;
	int			msg_entity;
	int			cdAudioTrack;
	int			maxClients;
	int			maxEntities;
	const char	*pStringBase;

	void		*pSaveData;
	vec3_t		vecLandmarkOffset;
} globalvars_t;


typedef struct
{
	string_t	classname;
	string_t	globalname;

	vec3_t		origin;
	vec3_t		oldorigin;
	vec3_t		velocity;
	vec3_t		basevelocity;
	vec3_t      clbasevelocity;  
							     
	vec3_t		movedir;

	vec3_t		angles;		
	vec3_t		avelocity;	
	vec3_t		punchangle;	
	vec3_t		v_angle;	
	int			fixangle;	
	float		idealpitch;
	float		pitch_speed;
	float		ideal_yaw;
	float		yaw_speed;

	int			modelindex;
	string_t	model;

	int			viewmodel;	
	int			weaponmodel;
	
	vec3_t		absmin;		
	vec3_t		absmax;		
	vec3_t		mins;		
	vec3_t		maxs;		
	vec3_t		size;		

	float		ltime;
	float		nextthink;

	int			movetype;
	int			solid;

	int			skin;			
	int			body;			
	int 		effects;
	
	float		gravity;		
	float		friction;		
	
	int			light_level;

	int			sequence;		
	int			gaitsequence;	
	float		frame;			
	float		animtime;		
	float		framerate;		
	byte		controller[4];	
	byte		blending[2];	
	byte		_pad_blending[2]; 

	float		scale;			

	int			rendermode;
	float		renderamt;
	vec3_t		rendercolor;
	int			renderfx;

	float		health;
	float		frags;
	int			weapons;  
	float		takedamage;

	int			deadflag;
	vec3_t		view_ofs;	

	int			button;
	int			impulse;

	edict_t		*chain;			
	edict_t		*dmg_inflictor;
	edict_t		*enemy;
	edict_t		*aiment;		
	edict_t		*owner;
	edict_t		*groundentity;

	int			spawnflags;
	int			flags;
	
	int			colormap;		
	int			team;

	float		max_health;
	float		teleport_time;
	float		armortype;
	float		armorvalue;
	int			waterlevel;
	int			watertype;
	
	string_t	target;
	string_t	targetname;
	string_t	netname;
	string_t	message;

	float		dmg_take;
	float		dmg_save;
	float		dmg;
	float		dmgtime;
	
	string_t	noise;
	string_t	noise1;
	string_t	noise2;
	string_t	noise3;
	
	float		speed;
	float		air_finished;
	float		pain_finished;
	float		radsuit_finished;
	
	edict_t		*pContainingEntity;
} entvars_t;


#endif // PROGDEFS_H