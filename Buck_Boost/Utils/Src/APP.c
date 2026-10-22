#include "APP.h"

uint8_t APP_CalibADC(Sample_Offsettypedef *p)
{
	uint16_t tick = 0;
	float suminputvol = 0.0f;

	HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedStart(&hadc2);

	HAL_HRTIM_WaveformOutputStart(&hhrtim1m, HRTIM_TIMERID_MASTER);
	for (uint16_t i = 0; i < 100; i ++)
	{
		HAL_ADCEx_InjectedStart(&hadc1);
		HAL_ADCEx_InjectedStart(&hadc2);

		HAL_ADCEx_InjectedPollForConversion(&hadc1, HAL_MAX_DELAY);
		HAL_ADCEx_InjectedPollForConversion(&hadc2, HAL_MAX_DELAY);

		suminputvol += ADC2->JDR2;
	}
	p->InputVol_Offset = suminpsuminputvolut / 100.0f;

	HAL_ADCEx_InjectedStop(&hadc1);
	HAL_ADCEx_InjectedStop(&hadc2);

	return 1;
}

void APP_Sample(void)
{
	ParaRaw.sysInputVol = ADC2->JDR1;
	ParaRaw.sysOutVol = ADC2->JDR2;
	ParaRaw.sysInputCurr = ADC2->JDR3;
	ParaRaw.sysOutCurr = ADC2->JDR4;
	ParaRaw.Lcurr = ADC1->JDR1;


}








