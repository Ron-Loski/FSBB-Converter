#include "Callback.h"

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	ADC_Sample();

	float vBase;

	if (stateMode == SYS_SOFTSTART)
	{
		SoftStart(&volLoop);
		vBase = volLoop.Target;

		if (vBase >= sysPara.OutVol)
		{
			uint32_t primask = __get_PRIMASK();
			__disable_irq();
			if (stateMode == SYS_SOFTSTART )//&& faultFlags == FAULT_NONE
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
		PWMDisable();
		return;
	}

	float modulation = SoftStartStep(vBase);

	Power_ModeUpdate(adcPara.InputVol, volLoop.Target);

	Power_PWMUpdate(modulation);

}
