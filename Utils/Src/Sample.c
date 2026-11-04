#include "Sample.h"

void ADC_Sample(void)
{
	rawPara.InputVol = ADC2->JDR1;
	rawPara.OutVol = ADC2->JDR2;
	rawPara.InputCurr = ADC2->JDR3;
	rawPara.OutCurr = ADC2->JDR4;

	adcPara.InputVol = (rawPara.InputVol / 4095.0f * 3.3f + 0.056f) * ADC_VIN_V_PER_COUNT;
	adcPara.OutVol = (rawPara.OutVol / 4095.0f * 3.3f + 0.056f) * ADC_VOUT_V_PER_COUNT;
	adcPara.InputCurr = (rawPara.InputCurr - appADC_IIN_ZERO_COUNTS) / 4095.0f * 3.3f / ADC_IIN_A_PER_COUNT;
//	adcPara.OutCurr = (rawPara.OutCurr - appADC_IOUT_ZERO_COUNTS) / 4095.0f * 3.3f / ADC_IOUT_A_PER_COUNT;

	adcPara.OutCurr = -((rawPara.OutCurr - appADC_IOUT_ZERO_COUNTS) / 4095.0f * 3.3f / ADC_IOUT_A_PER_COUNT);
}
uint16_t i;
HAL_StatusTypeDef state1;
HAL_StatusTypeDef state2;
uint8_t ADC_Calibration(void)
{
    uint32_t sumIin = 0U;
    uint32_t sumIout = 0U;


    PWMDisable();
    if (HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_MASTER) != HAL_OK)	{return 0U;}
    if ((state1 = HAL_ADCEx_InjectedStart(&hadc2)) != HAL_OK)	{return 0U;}
    if ((state2 = HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER)) != HAL_OK) {return 0U;}

    for (i = 0U; i < ADC_CALITICK; i++)
    {
        while (HAL_ADCEx_InjectedPollForConversion(&hadc2, 0U) != HAL_OK);

        sumIin += ADC2->JDR3;
        sumIout += ADC2->JDR4;
    }

    if (i == ADC_CALITICK)
    {
        appADC_IIN_ZERO_COUNTS = sumIin / ADC_CALITICK;
        appADC_IOUT_ZERO_COUNTS = sumIout / ADC_CALITICK;

        (void)HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_MASTER);
        (void)HAL_ADCEx_InjectedStop(&hadc2);
        HAL_HRTIM_SoftwareReset(&hhrtim1, HRTIM_TIMERRESET_MASTER);

        return 1U;
    }

    return 0U;
}
