#include "Global_Inc.h"

/* 已锁存的故障位，可同时记录多个原因，上电初始化时清零。 */
volatile uint32_t faultFlags = FAULT_NONE;

/* reason：本次故障位。只记录并关断，不等待恢复或重新启动。 */
void APP_FaultTrip(uint32_t reason)
{
    uint32_t primask = __get_PRIMASK(); /* 保存原中断状态，保护故障位更新 */
    __disable_irq();
    APP_PWMDisable();
    faultFlags |= reason;
    stateMode = SYS_FAULT;
    __set_PRIMASK(primask);
}

/* 检查人工关断按键、输入和输出过压比较器。 */
static uint32_t ProtectionHardwareFaults(void)
{
    uint32_t faults = FAULT_NONE; /* 本次发现的硬件故障位集合 */
    if (HAL_GPIO_ReadPin(FAULT_KEY_GPIO_PORT, FAULT_KEY_GPIO_PIN) == FAULT_KEY_ACTIVE_LEVEL)
        faults |= FAULT_STOP_KEY;
    if (HAL_COMP_GetOutputLevel(&VIN_OVP_COMP_HANDLE) == COMP_OUTPUT_LEVEL_HIGH)
        faults |= FAULT_VIN_HW_OVP;
    if (HAL_COMP_GetOutputLevel(&VOUT_OVP_COMP_HANDLE) == COMP_OUTPUT_LEVEL_HIGH)
        faults |= FAULT_VOUT_HW_OVP;
    return faults;
}

/* 仅上电初始化时调用；直接使用宏中的 DAC 码，不进行电压换算。 */
uint8_t APP_ProtectionInit(void)
{
    APP_PWMDisable();
    faultFlags = FAULT_NONE;
    stateMode = SYS_IDLE;

    /* 0 为待填写占位值；拒绝非法 DAC 码和未填写的采样比例。 */
    if (PROTECTION_CONFIG_READY != 1U ||
        VIN_OVP_DAC_CODE == 0U || VIN_OVP_DAC_CODE > 4095U ||
        VOUT_OVP_DAC_CODE == 0U || VOUT_OVP_DAC_CODE > 4095U ||
        ADC_VIN_V_PER_COUNT <= 0.0f || ADC_VOUT_V_PER_COUNT <= 0.0f ||
        ADC_IIN_A_PER_COUNT <= 0.0f || ADC_IOUT_A_PER_COUNT <= 0.0f)
    {
        APP_FaultTrip(FAULT_CONFIG);
        return 0U;
    }

    /* 先设置并启动 DAC，再启动比较器；两个危险电压的码值直接由宏给出。 */
    if (HAL_DAC_SetValue(&VIN_OVP_DAC_HANDLE, VIN_OVP_DAC_CHANNEL,
                         DAC_ALIGN_12B_R, VIN_OVP_DAC_CODE) != HAL_OK ||
        HAL_DAC_SetValue(&VOUT_OVP_DAC_HANDLE, VOUT_OVP_DAC_CHANNEL,
                         DAC_ALIGN_12B_R, VOUT_OVP_DAC_CODE) != HAL_OK ||
        HAL_DAC_Start(&VIN_OVP_DAC_HANDLE, VIN_OVP_DAC_CHANNEL) != HAL_OK ||
        HAL_DAC_Start(&VOUT_OVP_DAC_HANDLE, VOUT_OVP_DAC_CHANNEL) != HAL_OK ||
        HAL_COMP_Start(&VIN_OVP_COMP_HANDLE) != HAL_OK ||
        HAL_COMP_Start(&VOUT_OVP_COMP_HANDLE) != HAL_OK)
    {
        APP_FaultTrip(FAULT_HW_INIT);
        return 0U;
    }

    uint32_t faults = ProtectionHardwareFaults(); /* 比较器启动后立即检查故障 */
    if (faults != FAULT_NONE)
        APP_FaultTrip(faults);
    return faultFlags == FAULT_NONE;
}

/* 每次采样后检查一次：出现故障立即锁存并关断，已锁存时停止双环。 */
uint8_t APP_ProtectionOnSample(void)
{
    if (PROTECTION_CONFIG_READY != 1U || faultFlags != FAULT_NONE)
    {
        APP_PWMDisable();
        return 0U;
    }

    uint32_t faults = ProtectionHardwareFaults(); /* 累积本次硬件与软件故障 */
    if (!isfinite(adcPara.InputVol) || !isfinite(adcPara.OutVol) ||
        !isfinite(adcPara.InputCurr) || !isfinite(adcPara.OutCurr) ||
        rawPara.InputVol >= 4095.0f || rawPara.OutVol >= 4095.0f ||
        rawPara.InputCurr >= 4095.0f || rawPara.OutCurr >= 4095.0f)
    {
        faults |= FAULT_ADC_DATA;
    }
    else
    {
        /* 空闲时允许未接入输入电源；运行和软启动时检查输入欠压。 */
        if ((stateMode == SYS_SOFTSTART || stateMode == SYS_RUN) && adcPara.InputVol < VIN_UVP_TRIP_V)
            faults |= FAULT_VIN_UVP;
        if (adcPara.InputVol > VIN_OVP_TRIP_V) faults |= FAULT_VIN_OVP;
        if (adcPara.OutVol > VOUT_OVP_TRIP_V) faults |= FAULT_VOUT_OVP;
        if (fabsf(adcPara.InputCurr) > IIN_OCP_TRIP_A) faults |= FAULT_IIN_OCP;
        if (fabsf(adcPara.OutCurr) > IOUT_OCP_TRIP_A) faults |= FAULT_IOUT_OCP;
    }

    if (!isfinite(sysPara.OutVol) || !isfinite(sysPara.OutCurr) ||
        sysPara.OutVol < VOUT_SETTING_MIN_V || sysPara.OutVol > VOUT_SETTING_MAX_V ||
        sysPara.OutCurr <= 0.0f || sysPara.OutCurr > IOUT_SETTING_MAX_A)
        faults |= FAULT_CONFIG;

    if (faults != FAULT_NONE)
        APP_FaultTrip(faults);
    return faultFlags == FAULT_NONE &&
           (stateMode == SYS_SOFTSTART || stateMode == SYS_RUN);
}
