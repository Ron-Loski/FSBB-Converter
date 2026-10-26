#ifndef __APP_H_
#define __APP_H_

#include "Global_Inc.h"

void APP_SysInit(void);
void APP_DualLoopInit(PIDtypedef *pI, PIDtypedef *pV);
void APP_FaultTrip(uint32_t reason);
void APP_CtrlSampleInst(float Dutyon);
uint8_t APP_ModeJudge(void);
void APP_Sample(void);
void APP_SoftStart(PIDtypedef *p);
float APP_SoftStartStep(float vBase);
void APP_PWMDisable(void);
void APP_PowerModeUpdate(float vin, float vout);
void APP_WriteBridgeDuty(uint8_t time, float duty);
void APP_PWMUpdate(float u);





#endif
