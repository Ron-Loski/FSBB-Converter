#ifndef __EXTRA_H_
#define __EXTRA_H_

#include "Global_Inc.h"

void SysInit(void);
void DualLoopInit(PIDtypedef *pI, PIDtypedef *pV);
void PWMDisable(void);
void HRTIM_TIMER_SetDutytoCCR(uint8_t TIMERX ,float Duty);
void HRTIM_SetSampleCCR(float Duty);
/* 170 MHz HRTIM x32; ADC2: /4, 12 bit, four ranks at 2.5 cycles.
 * These margins must be rechecked if clocks, ADC settings or ISR cost change. */
#define HRTIM_ADC_EDGE_GUARD_TICKS    1088U /* 200 ns after/before an edge */
#define HRTIM_ADC_SCAN_TICKS          8704U /* 1.6 us, including trigger margin */
#define HRTIM_ADC_CONTROL_TICKS      5440U /* 1 us reserved before next update */

/* F = input bridge, E = output bridge. Returns 0 and keeps CMP4 if no
 * complete quiet window exists. Sample quality is then not guaranteed. */
uint8_t HRTIM_UpdateSamplePoint(void);
extern volatile uint8_t adcSampleWindowValid;
extern volatile uint32_t adcSamplePointTicks;
uint8_t Power_ModeJudge(void);
void SoftStart(PIDtypedef *p);
void Power_ModeUpdate(float vin, float vout);
float SoftStartStep(float vBase);
void Power_PWMUpdate(float Don);

#endif
