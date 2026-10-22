#ifndef __APP_H_
#define __APP_H_

#include "Global_Types.h"

uint8_t APP_CalibADC(Sample_Offsettypedef *p);
void APP_Sample(void);

void APP_OpenLoop_Init(void);
void APP_OpenLoop_Step(void);
void APP_OpenLoop_SetModeDuty(Mode_Statetypdef mode, float da, float db);
void APP_OpenLoop_SetModRatio(float m);

#endif
