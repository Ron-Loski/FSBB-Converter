#include "BSP.h"

static uint8_t s_pwm_output_enabled = 0U;

/**
 * @brief  设置单个半桥的上管占空比（对称中心对齐，下管由硬件死区自动互补生成）
 * @param  TimerIdx: HRTIM_TIMERINDEX_TIMER_E 或 HRTIM_TIMERINDEX_TIMER_F
 * @param  duty_high: 上管 CHx1 占空比 (0.0f ~ 1.0f)
 *         - Output Set   = TIMCMP1
 *         - Output Reset = TIMCMP3
 */
void BSP_HRTIM_SetHalfBridgeDuty(uint32_t TimerIdx, float duty_high)
{
	uint32_t cmp1;
	uint32_t cmp3;

	if (duty_high >= FSBB_DUTY_FULL_ON_TH)
	{
		/* 上管 100% 常通，下管 0% 常关：
		 * 计数器在 CMP_MIN 触发 CMP1 置高，CMP3 > Period 永不触发复位 */
		cmp1 = FSBB_HRTIM_CMP_MIN;
		cmp3 = FSBB_HRTIM_CMP_INACTIVE;
	}
	else if (duty_high <= FSBB_DUTY_FULL_OFF_TH)
	{
		/* 上管 0% 常关，下管 100% 常通：
		 * CMP1 > Period 永不触发置高，计数器在 CMP_MIN 触发 CMP3 复位 */
		cmp1 = FSBB_HRTIM_CMP_INACTIVE;
		cmp3 = FSBB_HRTIM_CMP_MIN;
	}
	else
	{
		/* 对称中心对齐：以周期中点 13600 为中心向两侧展开 CMP1 和 CMP3 */
		uint32_t half_ton = (uint32_t)(duty_high * (float)FSBB_HRTIM_HALF_PERIOD + 0.5f);
		uint32_t min_half = FSBB_HRTIM_CMP_MIN / 2U;
		uint32_t max_half = FSBB_HRTIM_HALF_PERIOD - FSBB_HRTIM_CMP_MIN;

		if (half_ton < min_half)
		{
			half_ton = min_half;
		}
		else if (half_ton > max_half)
		{
			half_ton = max_half;
		}

		cmp1 = FSBB_HRTIM_HALF_PERIOD - half_ton;
		cmp3 = FSBB_HRTIM_HALF_PERIOD + half_ton;
	}

	__HAL_HRTIM_SETCOMPARE(&hhrtim1, TimerIdx, HRTIM_COMPAREUNIT_1, cmp1);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, TimerIdx, HRTIM_COMPAREUNIT_3, cmp3);
}

/**
 * @brief  同时更新 FSBB 两侧桥臂上管占空比
 * @param  da: A 侧 (Buck 桥臂) 上管 Q1 占空比 (0.0f ~ 1.0f)
 * @param  db: B 侧 (Boost 桥臂) 上管 Q3 占空比 (0.0f ~ 1.0f，对应下管 Q4 占空比为 1 - db)
 */
void BSP_HRTIM_SetFSBBDuty(float da, float db)
{
	BSP_HRTIM_SetHalfBridgeDuty(FSBB_TIMER_BUCK, da);
	BSP_HRTIM_SetHalfBridgeDuty(FSBB_TIMER_BOOST, db);
}

/**
 * @brief  开启四路互补 PWM 输出
 */
void BSP_HRTIM_EnableOutput(void)
{
	if (s_pwm_output_enabled == 0U)
	{
		HAL_HRTIM_WaveformOutputStart(&hhrtim1, FSBB_OUTPUTS_ALL);
		s_pwm_output_enabled = 1U;
	}
}

/**
 * @brief  关闭四路互补 PWM 输出（四角均回到 Inactive 低电平安全态）
 */
void BSP_HRTIM_DisableOutput(void)
{
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, FSBB_OUTPUTS_ALL);
	s_pwm_output_enabled = 0U;
}

/**
 * @brief  初始化 HRTIM 计数器（先写入初始安全比较值，再同步启动 Master / Timer E / Timer F）
 */
void BSP_HRTIM_Init_PWM(void)
{
	/* 启动前先关闭输出，并将初始占空比设为安全状态 (da = 0, db = 1) */
	BSP_HRTIM_DisableOutput();
	BSP_HRTIM_SetFSBBDuty(0.0f, 1.0f);

	/* 将 Master CMP2 / CMP4 设在周期中心点 13600，供 ADC 中心采样触发使用 */
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, FSBB_HRTIM_HALF_PERIOD);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_4, FSBB_HRTIM_HALF_PERIOD);

	/* 单条寄存器写操作同步启动 MASTER, TIMER_E, TIMER_F 计数器，保证相位完全一致 */
	HAL_HRTIM_WaveformCountStart(&hhrtim1, FSBB_TIMERS_ALL);
}
