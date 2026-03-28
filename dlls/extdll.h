#ifndef EXTDLL_H
#define EXTDLL_H

#ifdef _DEBUG
#define DEBUG 1
#endif

#pragma warning(disable : 4244)		
#pragma warning(disable : 4305)		
#pragma warning(disable : 4201)		
#pragma warning(disable : 4514)		
#pragma warning(disable : 4100)		

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#include "WINDOWS.H"

#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

typedef int	func_t;					
typedef int	string_t;				
typedef float vec_t;				

#include "vector.h"

#define vec3_t Vector

#include "const.h"
#include "progs.h"

#include "eiface.h"

#include "cdll_dll.h"

#endif 

