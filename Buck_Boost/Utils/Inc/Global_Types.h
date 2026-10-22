#ifndef __GLOBAL_TYPES_H_
#define __GLOBAL_TYPES_H_

#include "Global_Inc.h"

typedef struct {
	float InputVol_Offset;
}Sample_Offsettypedef;

typedef enum {
	MODE_IDLE = 0,
	MODE_Buck,
	MODE_BB,
	MODE_Boost,
	MODE_FAULT,
}Mode_Statetypdef;

#endif

typedef struct {
	float sysInputVol;
	float sysOutVol;
}Sys_Paratyedef;
