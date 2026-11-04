#ifndef __PID_H_
#define __PID_H_

#include "Global_Inc.h"

void PID_Init(PIDtypedef *p);
void PID_Update(PIDtypedef *p);			/* 输出饱和时仍允许积分 */
float Clampfloat(float x, float min, float max);
void PI_Update(PIDtypedef *p);			/* 只允许退饱和方向的积分 */

#endif
