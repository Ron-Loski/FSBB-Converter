#ifndef __PID_H_
#define __PID_H_

#include "Global_Inc.h"

typedef struct{
	float Target;
	float Actual;
	float Out;

	float Kp;
	float Ki;
	float Kd;

	float Error0;
	float Error1;
	float Integral;       /* 积分输出，已包含Ki */
	float IntegralMax;    /* 积分输出上限 */
	float IntegralMin;    /* 积分输出下限 */

	float OutMax;
	float OutMin;
}PIDtypedef;

void PID_Init(PIDtypedef *p);
void PID_Update(PIDtypedef *p);

#endif
