#include "Global_Var.h"

PIDtypedef currLoop = {
		.Kp = 0.0f,
		.Ki = 0.0f,
		.Kd = 0.0f,

		.IntegralMax = 0.0f,
		.IntegralMin = 0.0f,

		.OutMax = 0.0f,
		.OutMin = 0.0f,
};

PIDtypedef volLoop = {
		.Kp = 0.0f,
		.Ki = 0.0f,
		.Kd = 0.0f,

		.IntegralMax = 0.0f,
		.IntegralMin = 0.0f,

		.OutMax = 0.0f,
		.OutMin = 0.0f,
};

uint32_t appADC_IIN_ZERO_COUNTS = 0U;
uint32_t appADC_IOUT_ZERO_COUNTS = 0U;

volatile Sys_Stateypdef stateMode = SYS_IDLE;
volatile Power_Modetypedef powerMode = MODE_BUCK;
volatile Power_Modetypedef previousPowerMode = MODE_BUCK;
volatile Reg_Modetypedef regMode = MODE_CV;

Paratyedef sysPara = {0.0f};
Paratyedef rawPara = {0.0f};
Paratyedef adcPara = {0.0f};

float previousDutyR = 0.0f;   // 上一拍输入侧上管占空比
float previousDutyC = 0.94f;  // 上一拍输出侧上管占空比










