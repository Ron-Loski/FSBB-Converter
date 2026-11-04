#include "Extra.h"

void SysInit(void)
{
	sysPara.InputVol = VIN_DSN;
	sysPara.OutVol = VOUT_DSN;
	sysPara.InputCurr = IIN_DSN;
	sysPara.OutCurr = IOUT_DSN;
}

void DualLoopInit(PIDtypedef *pI, PIDtypedef *pV)
{
	PID_Init(pI);
	PID_Init(pV);

	pV->Target = VOLTAGE_TAR;
	pV->Kp = VOLTAGE_KP;
	pV->Ki = VOLTAGE_KI;
	pV->IntegralMax = VOLTAGE_INTMAX;
	pV->IntegralMin = VOLTAGE_INTMIN;
	pV->OutMax = VOLTAGE_OUTMAX;
	pV->OutMin = VOLTAGE_OUTMIN;

	pV->Out = 0.0f;

	pI->Target = sysPara.OutCurr;
	pI->Kp = CURRENT_KP;
	pI->Ki = CURRENT_KI;
	pI->IntegralMax = CURRENT_INTMAX;
	pI->IntegralMin = CURRENT_INTMIN;
	pI->OutMax = CURRENT_OUTMAX;
	pI->OutMin = CURRENT_OUTMIN;

	pI->Out = 0.0f;

	regMode = MODE_CV;
}

void PWMDisable(void)
{
	if (__HAL_RCC_HRTIM1_IS_CLK_ENABLED()) {
		HRTIM1->sCommonRegs.ODISR = POWER_PWM_OUTPUTS;
	}
}


//  HRTIM_TIMER_Choosed&&Default compare1
//#define HRTIM_TIMERINDEX_TIMER_A 0x0U   /*!< Index used to access timer A registers */
//#define HRTIM_TIMERINDEX_TIMER_B 0x1U   /*!< Index used to access timer B registers */
//#define HRTIM_TIMERINDEX_TIMER_C 0x2U   /*!< Index used to access timer C registers */
//#define HRTIM_TIMERINDEX_TIMER_D 0x3U   /*!< Index used to access timer D registers */
//#define HRTIM_TIMERINDEX_TIMER_E 0x4U   /*!< Index used to access timer E registers */
//#define HRTIM_TIMERINDEX_TIMER_F 0x5U   /*!< Index used to access timer F registers */
//#define HRTIM_TIMERINDEX_MASTER  0x6U   /*!< Index used to access master registers  */
//#define HRTIM_TIMERINDEX_COMMON  0xFFU  /*!< Index used to access HRTIM common registers */
//
//
void HRTIM_TIMER_SetDutytoCCR(uint8_t TIMERX ,float Duty)
{

	uint32_t period = __HAL_HRTIM_GETPERIOD(&hhrtim1, TIMERX);

	uint32_t compare = (uint32_t)(Duty * (float)period);

	if (compare < 1U) {compare = 1U;}
	if (compare >= period) {compare = period - 1U;}

	__HAL_HRTIM_SETCOMPARE(&hhrtim1, TIMERX, HRTIM_COMPAREUNIT_1, compare);
}

/* 采样CCR设置 */
void HRTIM_SetSampleCCR(float Duty)
{
	uint32_t compare = (uint32_t)(Duty * (float)HRTIM_TIME_EARR);

	__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_4, compare);

}

uint8_t Power_ModeJudge(void)
{
	uint8_t state;
	if (  ((sysPara.OutVol - sysPara.InputVol) > 0)
		  && ((sysPara.OutVol - sysPara.InputVol)> V_CEOFFABV *  sysPara.InputVol)  ){
		state = MODE_BOOST;
	}
	else if (  ((sysPara.InputVol - sysPara.OutVol) > 0)
			  && ((sysPara.InputVol - sysPara.OutVol)> V_CEOFFBLE *  sysPara.InputVol)  ){
		state = MODE_BUCK;
	}
	else{
		state = MODE_BB;
	}

	return state;
}

static uint32_t softStartTick = 0U; /* 本次软启动已执行的 ADC 控制拍数 */
void SoftStart(PIDtypedef *p)
{
	uint32_t totalTicks = (uint32_t)(SOFTTIM_S / Tpwm + 0.5f);
	if (totalTicks == 0U) {totalTicks = 1U;}
	if (softStartTick < totalTicks) {softStartTick++;}

	p->Target = sysPara.OutVol * ((float)softStartTick / (float)totalTicks);
}

void Power_ModeUpdate(float vin, float vout)
{
	float ratio = vout / vin;

	switch (powerMode)
	{
		case MODE_BUCK:
			if (ratio > 1.25f){
				powerMode = MODE_BOOST;
			}
			else if (ratio > 0.85){
				powerMode = MODE_BB;
			}
			break;
		case MODE_BB:
			if (ratio < 0.8f){
				powerMode = MODE_BUCK;
			}
			else if (ratio > 1.2f){
				powerMode = MODE_BOOST;
			}
			break;
		case MODE_BOOST:
			if (ratio < 0.8f){
				powerMode = MODE_BB;
			}
			else if (ratio < 1.15){
				powerMode = MODE_BB;
			}
			break;
		default:
			break;
	}
}

/* 两个运行阶段共用：电流 PI 修正电压目标，电压 PI 计算调制量。 */
float SoftStartStep(float vBase)
{
	if (vBase < 0){
		vBase = 0.0f;
	}

    currLoop.Target = sysPara.OutCurr;
	currLoop.Actual = adcPara.OutCurr;
	currLoop.OutMax = 0.0f;
	currLoop.OutMin = -vBase;
	currLoop.IntegralMax = 0.0f;
	currLoop.IntegralMin = -vBase;
	PI_Update(&currLoop);

	float vRef = vBase + currLoop.Out;

	if (vRef > vBase){
		vRef = vBase;
	}
	if (vRef < 0.0f){
		vRef = 0.0f;
	}

	volLoop.Target = vRef;
//	volLoop.Target = vBase;
	volLoop.Actual = adcPara.OutVol;

	PI_Update(&volLoop);

	float reduction = vBase - vRef;

	if (regMode == MODE_CV && reduction > 0.05f){
			regMode = MODE_CC;
	}
	else if (reduction < 0.02f){
			regMode = MODE_CV;
		}

	return volLoop.Out;
}

void Power_PWMUpdate(float Don)
{
	float dMin;
	float dMax;

	switch (powerMode)
	{
		case MODE_BUCK:
			dMin = BUCK_D_MIN;
			dMax = BUCK_D_MAX;
			break;

		case MODE_BB:
			dMin = BB_D_MIN;
			dMax = BB_D_MAX;
			break;

		case MODE_BOOST:
			dMin = BOOST_D_MIN;
			dMax = BOOST_D_MAX;
			break;

		default:
			break;
	}

	uint8_t modeChanged = (powerMode != previousPowerMode);

	if (modeChanged)
	{
		float ratio = previousDutyR / previousDutyC;

		if (powerMode == MODE_BUCK){
			Don = ratio * BUCK_HOLD_DUTY;
		}
		else if (powerMode == MODE_BB){
			Don = 1.0f - BB_HOLD_DUTY / ratio;
		}
		else{
			Don = 1.0f - BOOST_HOLD_DUTY / ratio;
		}
	}

	float limitU = Clampfloat(Don, dMin, dMax);

	float dutyR, dutyC;

	switch (powerMode)
	{
		case MODE_BUCK:
			dutyR = limitU;
			dutyC = BUCK_HOLD_DUTY;
			break;

		case MODE_BB:
			dutyR = BB_HOLD_DUTY;
			dutyC = 1.0f - limitU;
			break;

		case MODE_BOOST:
			dutyR = BOOST_HOLD_DUTY;
			dutyC = 1.0f - limitU;
			break;

		default:
			break;
	}

	HRTIM_TIMER_SetDutytoCCR(HRTIM_TIMERINDEX_TIMER_F, dutyR);
	HRTIM_TIMER_SetDutytoCCR(HRTIM_TIMERINDEX_TIMER_E, dutyC);

	previousDutyR = dutyR;
	previousDutyC = dutyC;
	previousPowerMode = powerMode;
}





