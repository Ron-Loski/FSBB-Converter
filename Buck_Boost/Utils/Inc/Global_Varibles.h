#ifndef __GLOBAL_VARIBLES_H_
#define __GLOBAL_VARIBLES_H_

#include "Global_Inc.h"
#include "Protection_Config.h"

/*********************PWM底层配置********************/
#define Fpwm				200000U
#define Tpwm				(1.0f / (float)Fpwm)
#define HRTIM_TIME_EARR		27200U

/* 默认设定、实物范围及保护宏统一放在 Protection_Config.h。 */

/*********************三模式切换边界********************/
#define V_CEOFFABV			0.15f
#define V_CEOFFBLE			0.15f

/*********************软起动配置********************/

#define SOFTTIM_S			0.1f

/*********************PID初始化********************/
#define VOLTAGE_TAR		0.0f
#define VOLTAGE_KP		0.0f
#define VOLTAGE_KI		0.0f
#define VOLTAGE_INTMAX		0.0f
#define VOLTAGE_INTMIN		0.0f
#define VOLTAGE_OUTMAX		0.0f
#define VOLTAGE_OUTMIN		0.0f

#define CURRENT_KP		0.0f
#define CURRENT_KI		0.0f
#define CURRENT_INTMAX		0.0f
#define CURRENT_INTMIN		0.0f
#define CURRENT_OUTMAX		0.0f
#define CURRENT_OUTMIN		0.0f

/*********************采样校准：待硬件参数确定后填写********************/
#define ADC_VIN_ZERO_COUNTS     0.0f
#define ADC_VOUT_ZERO_COUNTS    0.0f
#define ADC_IIN_ZERO_COUNTS     0.0f
#define ADC_IOUT_ZERO_COUNTS    0.0f
#define ADC_VIN_V_PER_COUNT     0.0f
#define ADC_VOUT_V_PER_COUNT    0.0f
#define ADC_IIN_A_PER_COUNT     0.0f
#define ADC_IOUT_A_PER_COUNT    0.0f

/*********************模式切换占空比********************/
#define BUCK_HOLD_DUTY   0.94f		/* Buck模式下,输出侧上管固定占空比 */
#define BOOST_HOLD_DUTY  0.94f		/* Boost模式下,输入侧上管固定占空比 */
#define BB_HOLD_DUTY     0.80f		/* BB模式下,输入侧上管固定占空比 */

#define BUCK_U_MIN       0.01f
#define BUCK_U_MAX       0.94f

#define BB_U_MIN         0.01f
#define BB_U_MAX         0.65f

#define BOOST_U_MIN      0.06f
#define BOOST_U_MAX      0.65f

/*********************PWM输出宏定义********************/
#define POWER_PWM_OUTPUTS \
    (HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2 | \
     HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2)

/*********************变量声明********************/
extern PIDtypedef currLoop;
extern PIDtypedef volLoop;

extern volatile Sys_Stateypdef stateMode;
extern volatile Power_Modetypedef powerMode;
extern volatile Power_Modetypedef previousPowerMode;
extern volatile Reg_Modetypedef regMode;

extern Paratyedef sysPara;		/* 系统设计要求 */
extern Paratyedef rawPara;		/* 采样原始数据 */
extern Paratyedef adcPara;		/* 板载实际数据 */

extern uint8_t pwmHistoryValid;

extern float previousDutyE;   // 上一拍输入侧上管占空比
extern float previousDutyF;  // 上一拍输出侧上管占空比

#endif
