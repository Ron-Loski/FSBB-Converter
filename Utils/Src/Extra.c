#include "Extra.h"
#include <math.h>

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

volatile uint8_t adcSampleWindowValid = 0U;
volatile uint32_t adcSamplePointTicks = 0U;

uint8_t HRTIM_UpdateSamplePoint(void)
{
	/* All three timers must have equal periods, clocks and aligned counters.
	 * Read the newly written preload values after Power_PWMUpdate().
	 * Output 1 is set at update, reset at CMP1; complementary edges are
	 * covered by EDGE_GUARD (validate against actual gate/analog settling).
	 */
	uint32_t period = __HAL_HRTIM_GETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_MASTER);
	uint32_t inputEdge = __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1);
	uint32_t outputEdge = __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_COMPAREUNIT_1);
	uint32_t edges[4] = {0U, inputEdge, outputEdge, period};
	uint32_t bestSpan = 0U;
	uint32_t bestPoint = 0U;
	uint8_t found = 0U;
	adcSampleWindowValid = 0U;
	adcSamplePointTicks = __HAL_HRTIM_GETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_4);

	if (period <= HRTIM_ADC_SCAN_TICKS + HRTIM_ADC_CONTROL_TICKS ||
		inputEdge >= period || outputEdge >= period ||
		__HAL_HRTIM_GETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F) != period ||
		__HAL_HRTIM_GETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E) != period)
	{
		return 0U;
	}

	if (edges[1] > edges[2])
	{
		uint32_t temp = edges[1];
		edges[1] = edges[2];
		edges[2] = temp;
	}

	uint32_t latest = period - HRTIM_ADC_SCAN_TICKS - HRTIM_ADC_CONTROL_TICKS;
	for (uint32_t i = 0U; i < 3U; i++)
	{
		uint32_t width = edges[i + 1U] - edges[i];
		if (width < 2U * HRTIM_ADC_EDGE_GUARD_TICKS + HRTIM_ADC_SCAN_TICKS)
		{
			continue;
		}
		uint32_t first = edges[i] + HRTIM_ADC_EDGE_GUARD_TICKS;
		uint32_t last = edges[i + 1U] - HRTIM_ADC_EDGE_GUARD_TICKS
				- HRTIM_ADC_SCAN_TICKS;
		if (last > latest) {last = latest;}
		if (last < first) {continue;}
		uint32_t span = last - first;
		if (!found || span > bestSpan)
		{
			bestSpan = span;
			bestPoint = first + span / 2U;
			found = 1U;
		}
	}

	if (found)
	{
		/* Master preload is enabled: apply at its next repetition update.
		 * Do not force a mid-period update, which could retrigger the ADC.
		 */
		__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER,
				HRTIM_COMPAREUNIT_4, bestPoint);
		adcSamplePointTicks = bestPoint;
		adcSampleWindowValid = 1U;
	}
	return found;
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
	if (!isfinite(vin) || !isfinite(vout) || vin <= 0.1f || vout < 0.0f) {return;}
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
static uint8_t ControlLimits(float *lo, float *hi)
{
    switch (powerMode) {
    case MODE_BUCK: *lo = BUCK_D_MIN; *hi = BUCK_D_MAX; return 1U;
    case MODE_BB: *lo = BB_D_MIN; *hi = BB_D_MAX; return 1U;
    case MODE_BOOST: *lo = BOOST_D_MIN; *hi = BOOST_D_MAX; return 1U;
    default: return 0U;
    }
}

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

    Power_ModeUpdate(adcPara.InputVol, vRef);
    float lo, hi;
    if (!ControlLimits(&lo, &hi)) {
        stateMode = SYS_FAULT;
        PWMDisable();
        return 0.0f;
    }
    volLoop.OutMin = lo;
    volLoop.OutMax = hi;
    float proportional = volLoop.Kp * (vRef - volLoop.Actual);
    /* Permit P cancellation at actuator limits, including mode transfers. */
    volLoop.IntegralMin = lo - fabsf(proportional);
    volLoop.IntegralMax = hi + fabsf(proportional);
    if (powerMode != previousPowerMode) {
        float mapped = lo;
        if (previousDutyR > 0.0f && previousDutyC > 0.0f) {
            float ratio = previousDutyR / previousDutyC;
            mapped = (powerMode == MODE_BUCK) ? ratio * BUCK_HOLD_DUTY :
                1.0f - ((powerMode == MODE_BB) ? BB_HOLD_DUTY : BOOST_HOLD_DUTY) / ratio;
        }
        /* Retain the mapped command in the PI state for subsequent ticks. */
        volLoop.Out = Clampfloat(mapped, lo, hi);
        volLoop.Integral = volLoop.Out - proportional;
        volLoop.Error1 = volLoop.Error0;
        volLoop.Error0 = vRef - volLoop.Actual;
    } else {
        PI_Update(&volLoop);
    }

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

    if (!isfinite(Don) || !ControlLimits(&dMin, &dMax)) {
        stateMode = SYS_FAULT;
        PWMDisable();
        return;
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





