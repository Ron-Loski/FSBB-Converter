#ifndef __EXTRA_H_
#define __EXTRA_H_

#include "Global_Inc.h"

void SysInit(void);
void DualLoopInit(PIDtypedef *pI, PIDtypedef *pV);
void PWMDisable(void);
void HRTIM_TIMER_SetDutytoCCR(uint8_t TIMERX ,float Duty);
void HRTIM_SetSampleCCR(float Duty);
uint8_t Power_ModeJudge(void);
void SoftStart(PIDtypedef *p);
void Power_ModeUpdate(float vin, float vout);
float SoftStartStep(float vBase);
void Power_PWMUpdate(float Don);

#endif
