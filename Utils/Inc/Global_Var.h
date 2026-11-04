#ifndef __GLOBAL_VAR_H_
#define __GLOBAL_VAR_H_

#include "Global_Types.h"
#include "stdint.h"

/*********************PWM底层配置********************/
#define Fpwm				200000U  /* PWM 开关频率，单位：Hz */
#define Tpwm				(1.0f / (float)Fpwm)  /* PWM 周期，单位：s；用于计算软启动步数 */
#define HRTIM_TIME_EARR		27200U  /* HRTIM 定时器 E 的周期计数配置值，单位：计数 */


#define POWER_PWM_OUTPUTS \
    (HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2 | \
     HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2)		/* 功率级四路 PWM 输出掩码：输入桥臂 TF1/TF2、输出桥臂 TE1/TE2。 */

/*********************功率设计指标********************/
#define VIN_DSN		24.0f;		/* 输入侧电压 */
#define VOUT_DSN	48.0f		/* 输出侧电压 */
#define IIN_DSN		3.0f		/* 输入侧直流源电流限流 */
#define IOUT_DSN	0.5f		/* 输出侧电流限流 */

/*********************三模式切换边界********************/
#define V_CEOFFABV			0.15f  /* APP_ModeJudge 升压判定系数：(Vout - Vin) / Vin > 此值 */
#define V_CEOFFBLE			0.15f  /* APP_ModeJudge 降压判定系数：(Vin - Vout) / Vin > 此值 */

#define SOFTTIM_S			0.1f  /* 输出电压参考值从 0 上升至设定值的软启动时间，单位：s */

/* 1：上电后由 ADC 回调检查条件并启动输出；0：只采样和检查保护。
 * 故障状态不会自动重启。 */
#define POWER_START_ENABLE     1U

/*********************PID底层配置********************/
#define VOLTAGE_TAR			0.0f  /* 电压环初始目标值，单位：V；启动后由软启动参考值更新 */
#define VOLTAGE_KP			0.012f  /* 电压环比例增益 */
#define VOLTAGE_KI			1.0f * Tpwm  /* 电压环每次控制更新的积分增益，已包含采样周期因素 */
#define VOLTAGE_INTMAX		0.94f  /* 电压环积分项初始上限，控制量 u */
#define VOLTAGE_INTMIN		0.0f  /* 电压环积分项初始下限，控制量 u */
#define VOLTAGE_OUTMAX		0.94f  /* 电压环输出初始上限，运行时按功率模式更新 */
#define VOLTAGE_OUTMIN		0.0f  /* 电压环输出初始下限，运行时按功率模式更新 */

#define CURRENT_KP			3.0f  /* 电流限幅环比例增益，输出为电压参考修正量 */
#define CURRENT_KI			100.0f * Tpwm  /* 电流限幅环每次控制更新的积分增益，已包含采样周期因素 */
#define CURRENT_INTMAX		0.94f  /* 电流环积分项初始上限，单位：V；运行时设为 0 */
#define CURRENT_INTMIN		0.f  /* 电流环积分项初始下限，单位：V；运行时设为 -vBase */
#define CURRENT_OUTMAX		0.94f  /* 电流环输出初始上限，单位：V；运行时设为 0 */
#define CURRENT_OUTMIN		0.0f  /* 电流环输出初始下限，单位：V；运行时设为 -vBase */

/*********************采样偏置配置********************/
#define ADC_CALITICK			300U			/* ADC上电初始化采样次数 */
#define ADC_VIN_ZERO_COUNTS     2048U  			/* 输入电压通道零点偏置，单位：ADC 计数 */
#define ADC_VOUT_ZERO_COUNTS    2048U  			/* 输出电压通道零点偏置，单位：ADC 计数 */
extern uint32_t appADC_IIN_ZERO_COUNTS;   		/* 输入电流通道零点偏置，单位：ADC 计数 */
extern uint32_t appADC_IOUT_ZERO_COUNTS;  		/* 输出电流通道零点偏置，单位：ADC 计数 */
#define ADC_VIN_V_PER_COUNT     24.0f  			/* 输入电压换算系数，单位：V/计数 */
#define ADC_VOUT_V_PER_COUNT    24.0f  			/* 输出电压换算系数，单位：V/计数 */
#define ADC_IIN_A_PER_COUNT     20.0f/0.005f	/* 输入电流换算系数，单位：A/计数 */
#define ADC_IOUT_A_PER_COUNT    20.0f/0.005f	/* 输出电流换算系数，单位：A/计数 */

#define BUCK_HOLD_DUTY   0.94f  /* Buck模式下,输出侧上管固定占空比 */
#define BOOST_HOLD_DUTY  0.94f  /* Boost模式下,输入侧上管固定占空比 */
#define BB_HOLD_DUTY     0.80f  /* BB模式下,输入侧上管固定占空比 */

#define BUCK_D_MIN       0.01f  /* Buck 模式控制量 u 下限，u 为输入侧上管占空比 */
#define BUCK_D_MAX       0.94f  /* Buck 模式控制量 u 上限，u 为输入侧上管占空比 */

#define BB_D_MIN         0.01f  /* Buck-Boost 模式控制量 u 下限，输出侧上管占空比为 1-u */
#define BB_D_MAX         0.85f  /* Buck-Boost 模式控制量 u 上限，输出侧上管占空比为 1-u */

#define BOOST_D_MIN      0.06f  /* Boost 模式控制量 u 下限，输出侧上管占空比为 1-u */
#define BOOST_D_MAX      0.80f  /* Boost 模式控制量 u 上限，输出侧上管占空比为 1-u */

/*********************参数声明********************/
extern PIDtypedef currLoop;
extern PIDtypedef volLoop;

extern volatile Sys_Stateypdef stateMode;
extern volatile Power_Modetypedef powerMode;
extern volatile Power_Modetypedef previousPowerMode;
extern volatile Reg_Modetypedef regMode;

extern Paratyedef sysPara;
extern Paratyedef rawPara;
extern Paratyedef adcPara;

extern float previousDutyR;   // 上一拍输入侧上管占空比
extern float previousDutyC;  // 上一拍输出侧上管占空比

#endif
