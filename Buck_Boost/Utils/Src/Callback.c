#include "Callback.h"

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC2)
	{
		APP_Sample();
	}
}

void HAL_HRTIM_Fault1Callback(HRTIM_HandleTypeDef *hhrtim)
{
	(void)hhrtim;
	OpenLoop.enable = 0U;
	stateMode = MODE_FAULT;
	BSP_HRTIM_DisableOutput();
}

void HAL_HRTIM_Fault2Callback(HRTIM_HandleTypeDef *hhrtim)
{
	(void)hhrtim;
	OpenLoop.enable = 0U;
	stateMode = MODE_FAULT;
	BSP_HRTIM_DisableOutput();
}

void HAL_HRTIM_Fault3Callback(HRTIM_HandleTypeDef *hhrtim)
{
	(void)hhrtim;
	OpenLoop.enable = 0U;
	stateMode = MODE_FAULT;
	BSP_HRTIM_DisableOutput();
}
