#ifndef __GLOBAL_TYPES_H_
#define __GLOBAL_TYPES_H_

typedef struct {
	float Target;
	float Actual;
	float Out;
	float Kp;
	float Ki;
	float Kd;
	float Error0;
	float Error1;
	float Integral;       /* Integral output, including Ki. */
	float IntegralMax;
	float IntegralMin;
	float OutMax;
	float OutMin;
}PIDtypedef;

typedef struct {
	float InputVol_Offset;
}Sample_Offsettypedef;

typedef enum {
	SYS_IDLE = 0,
	SYS_SOFTSTART,
	SYS_RUN,
	SYS_FAULT
}Sys_Stateypdef;

typedef enum {
	MODE_BUCK = 0,
	MODE_BB,
	MODE_BOOST
}Power_Modetypedef;

typedef enum {
	MODE_CV = 0,
	MODE_CC
}Reg_Modetypedef;

typedef struct {
	float InputVol;
	float OutVol;
	float InputCurr;
	float OutCurr;
}Paratyedef;

/* 故障位类型统一定义在 Protection.h，避免重复定义 FAULT_NONE。 */

#endif
