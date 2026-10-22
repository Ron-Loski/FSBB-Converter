#ifndef __BSP_H_
#define __BSP_H_

#include "stm32g4xx_hal.h"
#include "hrtim.h"

/* ==========================================================================
 * HRTIM 时钟与周期参数 (170MHz * 32 = 5.44GHz, Period = 27200 -> f_sw = 200kHz)
 * ========================================================================== */
#define FSBB_HRTIM_PERIOD          27200U
#define FSBB_HRTIM_HALF_PERIOD     (FSBB_HRTIM_PERIOD / 2U)    /* 13600U: 对称中心点 */
#define FSBB_HRTIM_CMP_MIN         96U                         /* MUL32 下最小安全比较值 */
#define FSBB_HRTIM_CMP_MAX         (FSBB_HRTIM_PERIOD - FSBB_HRTIM_CMP_MIN)
#define FSBB_HRTIM_CMP_INACTIVE    0xFFDFU                     /* > Period，使对应比较事件在周期内不触发 */

/* 占空比常通/常关切换阈值 */
#define FSBB_DUTY_FULL_ON_TH       0.985f                      /* >= 98.5% 时上管常通(100%)、下管常关 */
#define FSBB_DUTY_FULL_OFF_TH      0.015f                      /* <= 1.5%  时上管常关(0%)、下管常通 */

/* ==========================================================================
 * 双向 FSBB 桥臂与 HRTIM 定时器映射
 * 默认正向：
 *   - A 侧 (输入/Buck 桥臂 Q1/Q2) -> TIMER_E (PC8: CHE1 上管 Q1, PC9: CHE2 下管 Q2)
 *   - B 侧 (输出/Boost 桥臂 Q3/Q4) -> TIMER_F (PC6: CHF1 上管 Q3, PC7: CHF2 下管 Q4)
 * 若硬件方向相反，只需将下方两个宏互换即可。
 * ========================================================================== */
#define FSBB_TIMER_BUCK            HRTIM_TIMERINDEX_TIMER_E
#define FSBB_TIMER_BOOST           HRTIM_TIMERINDEX_TIMER_F

#define FSBB_TIMERS_ALL            (HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_E | HRTIM_TIMERID_TIMER_F)
#define FSBB_OUTPUTS_ALL           (HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2 | HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2)

void BSP_HRTIM_Init_PWM(void);
void BSP_HRTIM_EnableOutput(void);
void BSP_HRTIM_DisableOutput(void);
void BSP_HRTIM_SetHalfBridgeDuty(uint32_t TimerIdx, float duty_high);
void BSP_HRTIM_SetFSBBDuty(float da, float db);

#endif
