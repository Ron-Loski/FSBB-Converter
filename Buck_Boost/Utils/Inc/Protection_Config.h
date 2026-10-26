#ifndef PROTECTION_CONFIG_H
#define PROTECTION_CONFIG_H

/* 电压单位 V、电流单位 A；0.0f/0U 的待填写项不是实物允许值。 */

/*********************默认设定*********************/
#define VIN_DSN                       24.0f  /* 默认/标称输入电压，不是过压阈值 */
#define VOUT_DSN                      24.0f  /* 上电默认输出电压设定 */
#define IIN_DSN                        3.0f  /* 输入电流设计值，不代表已实现输入限流 */
#define IOUT_DSN                       2.0f  /* 默认恒流设定，不是过流关断阈值 */

/*********************实物工作范围：待填写*********************/
#define VIN_OPERATING_MIN_V            0.0f  /* 正常最低输入电压，供设计核对 */
#define VIN_OPERATING_MAX_V            0.0f  /* 正常最高输入电压，供设计核对 */
#define VOUT_SETTING_MIN_V             0.0f  /* 用户允许设置的最低输出电压 */
#define VOUT_SETTING_MAX_V             0.0f  /* 用户允许设置的最高输出电压 */
#define IOUT_SETTING_MAX_A             0.0f  /* 用户允许设置的最大恒流值 */
#define IIN_CONTINUOUS_MAX_A           0.0f  /* 输入持续电流能力，供设计核对 */
#define IL_PEAK_MAX_A                  0.0f  /* 电感/MOS 峰值能力，当前没有对应的峰值电流保护通道 */

/*********************软件保护阈值：待填写*********************/
#define VIN_UVP_TRIP_V                 0.0f  /* 运行/软启动时输入低于此值，记录欠压并关断 */
#define VIN_OVP_TRIP_V                 0.0f  /* 输入高于此值，记录过压并关断 */
#define VOUT_OVP_TRIP_V                0.0f  /* 输出高于此值，记录过压并关断 */
#define IIN_OCP_TRIP_A                 0.0f  /* 输入电流绝对值超过此值，记录过流并关断 */
#define IOUT_OCP_TRIP_A                0.0f  /* 输出电流绝对值超过此值，记录过流并关断；与恒流设定区分 */
/* 软启动和 CC 期间输出降低可能正常，不设置通用输出欠压故障。 */

/*********************硬件过压：直接填写 12 位 DAC 码*********************/
#define VIN_OVP_DAC_CODE               0U    /* 待填写：危险输入电压对应的 DAC3_CH2 码，范围 1～4095 */
#define VOUT_OVP_DAC_CODE              0U    /* 待填写：危险输出电压对应的 DAC4_CH2 码，范围 1～4095 */
/* 按分压和参考电压确定码值后直接填入，不在程序中换算或保存为参数变量。 */

/* PB0 输入电压信号 -> COMP4 -> 内部 Fault 2。 */
#define VIN_OVP_COMP_HANDLE            hcomp4
#define VIN_OVP_DAC_HANDLE             hdac3
#define VIN_OVP_DAC_CHANNEL            DAC_CHANNEL_2

/* PB11 输出电压信号 -> COMP6 -> 内部 Fault 3。 */
#define VOUT_OVP_COMP_HANDLE           hcomp6
#define VOUT_OVP_DAC_HANDLE            hdac4
#define VOUT_OVP_DAC_CHANNEL           DAC_CHANNEL_2
/* 两个比较器检测电压，当前没有硬件瞬时过流保护。 */

/*********************人工故障关断按键*********************/
#define FAULT_KEY_GPIO_PORT            GPIOA
#define FAULT_KEY_GPIO_PIN             GPIO_PIN_12
#define FAULT_KEY_ACTIVE_LEVEL         GPIO_PIN_RESET /* 当前 FLT1 低有效，确认松开有可靠上拉 */

/*********************未配置时禁止输出*********************/
#define PROTECTION_CONFIG_READY       0U
/* 填写核对采样比例、软件保护阈值和 DAC 码后设为 1；不会自动开启 PWM。 */

#endif /* PROTECTION_CONFIG_H */
