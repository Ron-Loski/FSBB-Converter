#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdint.h>

/* 多个故障可按位或记录，正常 CC 调节不属于故障。 */
typedef enum {
    FAULT_NONE          = 0U,       /* 无故障 */
    FAULT_CONFIG        = 1U << 0,  /* 参数未填写、DAC 码越界或设定超范围 */
    FAULT_STOP_KEY      = 1U << 1,  /* PA12 人工关断按键 */
    FAULT_VIN_HW_OVP    = 1U << 2,  /* COMP4 输入硬件过压 */
    FAULT_VOUT_HW_OVP   = 1U << 3,  /* COMP6 输出硬件过压 */
    FAULT_VIN_UVP       = 1U << 4,  /* 输入欠压 */
    FAULT_VIN_OVP       = 1U << 5,  /* 输入软件过压 */
    FAULT_VOUT_OVP      = 1U << 6,  /* 输出软件过压 */
    FAULT_IIN_OCP       = 1U << 7,  /* 输入软件过流 */
    FAULT_IOUT_OCP      = 1U << 8,  /* 输出软件过流 */
    FAULT_ADC_DATA      = 1U << 9,  /* 采样异常或 ADC 错误 */
    FAULT_HW_INIT       = 1U << 11, /* DAC/比较器启动失败；保留原故障位编号 */
    FAULT_CONTROL       = 1U << 12  /* 控制计算或模式异常 */
} ProtectionFault_t;

extern volatile uint32_t faultFlags; /* 已锁存的故障类型，上电初始化时清零 */

uint8_t APP_ProtectionInit(void);     /* 上电时设置 DAC 并启动比较器 */
void APP_FaultTrip(uint32_t reason);  /* 记录故障并关闭 PWM */
uint8_t APP_ProtectionOnSample(void); /* 采样后检查，允许继续执行双环时返回 1 */

#endif /* PROTECTION_H */
