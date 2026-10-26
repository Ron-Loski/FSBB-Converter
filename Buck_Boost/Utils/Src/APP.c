#include "APP.h"

/* 系统设置要求指标 */
void APP_SysInit(void)
{
	sysPara.InputVol = VIN_DSN;
	sysPara.OutVol = VOUT_DSN;
	sysPara.InputCurr = IIN_DSN;
	sysPara.OutCurr = IOUT_DSN;
}

void APP_DualLoopInit(PIDtypedef *pI, PIDtypedef *pV)
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

void APP_CtrlSampleInst(float Dutyon)
{
	/* ADC 触发器 1 连接至主定时器 CMP4 事件 */
	uint32_t period = __HAL_HRTIM_GETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_MASTER);
	if (period < 2U) {
		return;
	}
	float duty = Clampfloat(Dutyon, 0.0f, 1.0f);

	uint32_t compare = (uint32_t)((1.0f - duty) * (float)period * 0.5f);

	if (compare < 1U) compare = 1U;
	if (compare >= period) compare = period - 1U;

	__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_4, compare);
}

uint32_t APP_HRTIM_DutytoCCR(float Duty)
{
	uint32_t period = __HAL_HRTIM_GETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E);

	return (uint32_t)(Clampfloat(Duty, 0.0f, 1.0f) * (float)period);
}

uint8_t APP_ModeJudge(void)
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

void APP_Sample(void)
{
	rawPara.InputVol = (float)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
	rawPara.OutVol = (float)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2);
	rawPara.InputCurr = (float)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_3);
	rawPara.OutCurr = (float)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_4);

	/* Subtract in floating point: a negative offset must not wrap unsigned. */
	adcPara.InputVol = (rawPara.InputVol - ADC_VIN_ZERO_COUNTS) * ADC_VIN_V_PER_COUNT;
	adcPara.OutVol = (rawPara.OutVol - ADC_VOUT_ZERO_COUNTS) * ADC_VOUT_V_PER_COUNT;
	adcPara.InputCurr = (rawPara.InputCurr - ADC_IIN_ZERO_COUNTS) * ADC_IIN_A_PER_COUNT;
	adcPara.OutCurr = (rawPara.OutCurr - ADC_IOUT_ZERO_COUNTS) * ADC_IOUT_A_PER_COUNT;
}

static uint32_t softStartTick = 0U;

void APP_SoftStartReset(void)
{
	softStartTick = 0U;
}

void APP_SoftStart(PIDtypedef *p)
{
	uint32_t totalTicks = (uint32_t)(SOFTTIM_S / Tpwm + 0.5f);
	if (totalTicks == 0U) totalTicks = 1U;
	if (softStartTick < totalTicks) softStartTick++;
	p->Target = sysPara.OutVol * ((float)softStartTick / (float)totalTicks);
}

float APP_SoftStartStep(float vBase)
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

void APP_PWMDisable(void)
{
	pwmHistoryValid = 0U;
	/* 也允许初始化早期的 Error_Handler 调用，避免空句柄访问。 */
	if (__HAL_RCC_HRTIM1_IS_CLK_ENABLED()) {
		HRTIM1->sCommonRegs.ODISR = POWER_PWM_OUTPUTS;
	}
}

void APP_PowerModeUpdate(float vin, float vref)
{
	if (!isfinite(vin) || !isfinite(vref) ||
	    vin <= 0.0f || vref < 0.0f)
	{
	    APP_FaultTrip(FAULT_ADC_DATA);
	    return;
	}

	float ratio = vref / vin;

	switch (powerMode)
	{
		case MODE_BUCK:
			if (ratio > 1.25){
				powerMode = MODE_BOOST;
			}
			else if (ratio > 0.85f){
				powerMode = MODE_BB;
			}
			break;

		case MODE_BB:
			if (ratio < 0.8f){
				powerMode = MODE_BUCK;
			}
			else if (ratio > 1.2){
				powerMode = MODE_BOOST;
			}
			break;

		case MODE_BOOST:
			if (ratio < 0.80f){
				powerMode = MODE_BUCK;
			}
			else if (ratio < 1.15){
				powerMode = MODE_BB;
			}
			break;

		default:
			APP_FaultTrip(FAULT_CONTROL);
			break;
	}
}

/* A-F对应0x00 - 0x05 */
void APP_WriteBridgeDuty(uint8_t time, float duty)
{
	uint32_t cmp1 = 0U;
	uint32_t cmp3 = 0U;
	uint32_t period = __HAL_HRTIM_GETPERIOD(&hhrtim1, time);
	uint32_t width = (uint32_t)(duty * (float)period + 0.5f);

	cmp1 = (period - width) / 2U;
	cmp3 = cmp1 + width;

	__HAL_HRTIM_SETCOMPARE(&hhrtim1, time, HRTIM_COMPAREUNIT_1, cmp1);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, time, HRTIM_COMPAREUNIT_3, cmp3);
}

/* u为输入侧上管占空比 */
void APP_PWMUpdate(float u)
{
	/* 故障发生后保持关闭输出，不执行恢复等待或自动重启。 */
	if (PROTECTION_CONFIG_READY != 1U || faultFlags != FAULT_NONE) {
		APP_PWMDisable();
		return;
	}
	if (stateMode != SYS_SOFTSTART && stateMode != SYS_RUN){
		APP_PWMDisable();
		return;
	}
	if (!isfinite(u))
	{
	    APP_FaultTrip(FAULT_CONTROL);
	    return;
	}

	float uMin;
	float uMax;

	switch (powerMode)
	{
		case MODE_BUCK:
			uMin = BUCK_U_MIN;
			uMax = BUCK_U_MAX;
			break;

		case MODE_BB:
			uMin = BB_U_MIN;
			uMax = BB_U_MAX;
			break;

		case MODE_BOOST:
			uMin = BOOST_U_MIN;
			uMax = BOOST_U_MAX;
			break;
		default:
			APP_FaultTrip(FAULT_CONTROL);
			return;
	}

	uint8_t modeChanged = pwmHistoryValid && (powerMode != previousPowerMode);

	if (modeChanged)
	{
		if (!isfinite(previousDutyE) || !isfinite(previousDutyF) ||
			previousDutyE <= 0.0f || previousDutyF <= 0.0f) {
			APP_FaultTrip(FAULT_CONTROL);
			return;
		}
		float ratio = previousDutyE / previousDutyF;

		if (powerMode == MODE_BUCK){
			u = ratio * BUCK_HOLD_DUTY;
		}
		else if (powerMode == MODE_BB){
			u = 1.0f - BB_HOLD_DUTY / ratio;
		}
		else{
			u = 1.0f - BOOST_HOLD_DUTY / ratio;
		}
	}

	float limitU = Clampfloat(u, uMin, uMax);

	if (modeChanged || limitU != u)
	{
	    float trackedIntegral =
	        limitU - volLoop.Kp * volLoop.Error0;

	    volLoop.Integral = Clampfloat(
	        trackedIntegral,
	        volLoop.IntegralMin,
	        volLoop.IntegralMax);
	}

	volLoop.OutMax = uMax;
	volLoop.OutMin = uMin;

	volLoop.Out = limitU;

	float dutyE, dutyF;

	switch (powerMode)
	{
		case MODE_BUCK:
			dutyE = limitU;
			dutyF = BUCK_HOLD_DUTY;
			break;

		case MODE_BB:
			dutyE = BB_HOLD_DUTY;
			dutyF = 1.0f - limitU;
			break;

		case MODE_BOOST:
			dutyE = BOOST_HOLD_DUTY;
			dutyF = 1.0f - limitU;
			break;

		default:
			APP_FaultTrip(FAULT_CONTROL);
			return;
	}

	 /*
	 * 写入四个比较值期间，暂时禁止预装载值转入活动寄存器。
	 * 本函数应作为 E/F 比较值和更新控制的唯一写入者。
	 */
	uint32_t updateMask = HRTIM_CR1_TEUDIS | HRTIM_CR1_TFUDIS;
	uint32_t oldUpdateBits = hhrtim1.Instance->sCommonRegs.CR1 & updateMask;	/* 记住原来的 E/F 更新禁止状态 */

	SET_BIT(hhrtim1.Instance->sCommonRegs.CR1, updateMask);		/* 暂时禁止新参数进入活动寄存器 */

	APP_WriteBridgeDuty(HRTIM_TIMERINDEX_TIMER_E, dutyE);
	APP_WriteBridgeDuty(HRTIM_TIMERINDEX_TIMER_F, dutyF);

	/* 只恢复 CR1 中 E/F 更新禁止位原来的状态，不改变其他位 */
	MODIFY_REG(hhrtim1.Instance->sCommonRegs.CR1,
			updateMask,
			oldUpdateBits);

	previousDutyE = dutyE;
	previousDutyF = dutyF;
	previousPowerMode = powerMode;
	pwmHistoryValid = 1U;
}
















