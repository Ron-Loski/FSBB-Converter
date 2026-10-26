#include "Global_Inc.h"

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc2)
	{
		APP_Sample();
		if (!APP_ProtectionOnSample()) {
			APP_PWMDisable();
			return;
		}

		float vBase;

		if (stateMode == SYS_SOFTSTART)
		{
		    APP_SoftStart(&volLoop);
		    vBase = volLoop.Target;

		    if (vBase >= sysPara.OutVol)
		    {
		        uint32_t primask = __get_PRIMASK();
		        __disable_irq();
		        if (stateMode == SYS_SOFTSTART && faultFlags == FAULT_NONE)
		            stateMode = SYS_RUN;
		        __set_PRIMASK(primask);
		    }
		}
		else if (stateMode == SYS_RUN)
		{
		    vBase = sysPara.OutVol;
		}
		else
		{
		    APP_PWMDisable();
		    return;
		}

		float modulation = APP_SoftStartStep(vBase);

		APP_PowerModeUpdate(adcPara.InputVol, volLoop.Target);

		APP_PWMUpdate(modulation);
	}
}

/* RM0440 的内部 Fault 2/3 分别接 COMP4/COMP6，当前均配置为高有效。 */
void HAL_HRTIM_Fault1Callback(HRTIM_HandleTypeDef *hhrtim)
{
	if (hhrtim == &hhrtim1) APP_FaultTrip(FAULT_STOP_KEY);
}

void HAL_HRTIM_Fault2Callback(HRTIM_HandleTypeDef *hhrtim)
{
	if (hhrtim == &hhrtim1) APP_FaultTrip(FAULT_VIN_HW_OVP);
}

void HAL_HRTIM_Fault3Callback(HRTIM_HandleTypeDef *hhrtim)
{
	if (hhrtim == &hhrtim1) APP_FaultTrip(FAULT_VOUT_HW_OVP);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc2) APP_FaultTrip(FAULT_ADC_DATA);
}

void HAL_ADCEx_InjectedQueueOverflowCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc2) APP_FaultTrip(FAULT_ADC_DATA);
}
