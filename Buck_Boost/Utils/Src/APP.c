#include "Global_Inc.h"

static float ClampFloat(float val, float min_val, float max_val)
{
	if (val < min_val)
	{
		return min_val;
	}
	if (val > max_val)
	{
		return max_val;
	}
	return val;
}

static float SlewLimit(float current, float target, float step)
{
	if (step <= 0.0f)
	{
		return target;
	}
	if (target > current + step)
	{
		return current + step;
	}
	if (target < current - step)
	{
		return current - step;
	}
	return target;
}

uint8_t APP_CalibADC(Sample_Offsettypedef *p)
{
	float suminputvol = 0.0f;

	HAL_ADCEx_InjectedStart(&hadc1);
	HAL_ADCEx_InjectedStart(&hadc2);

	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER);
	for (uint16_t i = 0; i < 100U; i++)
	{
		HAL_ADCEx_InjectedStart(&hadc1);
		HAL_ADCEx_InjectedStart(&hadc2);

		HAL_ADCEx_InjectedPollForConversion(&hadc1, HAL_MAX_DELAY);
		HAL_ADCEx_InjectedPollForConversion(&hadc2, HAL_MAX_DELAY);

		suminputvol += (float)ADC2->JDR2;
	}
	p->InputVol_Offset = suminputvol / 100.0f;

	HAL_ADCEx_InjectedStop(&hadc1);
	HAL_ADCEx_InjectedStop(&hadc2);

	return 1U;
}

void APP_Sample(void)
{
	ParaRaw.sysInputVol  = (float)ADC2->JDR1;
	ParaRaw.sysOutVol    = (float)ADC2->JDR2;
	ParaRaw.sysInputCurr = (float)ADC2->JDR3;
	ParaRaw.sysOutCurr   = (float)ADC2->JDR4;
	ParaRaw.Lcurr        = (float)ADC1->JDR1;
}

/**
 * @brief  初始化开环控制与 HRTIM 发波定时器
 */
void APP_OpenLoop_Init(void)
{
	OpenLoop.da_out = 0.0f;
	OpenLoop.db_out = 1.0f;
	BSP_HRTIM_Init_PWM();
}

/**
 * @brief  手动指定工作模式与两侧上管占空比
 * @param  mode: MODE_Buck / MODE_BB / MODE_Boost / MODE_IDLE
 * @param  da:   A 侧 (Buck 臂) 上管 Q1 占空比 (0.0 ~ 1.0)
 * @param  db:   B 侧 (Boost 臂) 上管 Q3 占空比 (0.0 ~ 1.0，下管 Q4 占空比 = 1 - db)
 */
void APP_OpenLoop_SetModeDuty(Mode_Statetypdef mode, float da, float db)
{
	OpenLoop.use_mod_ratio = 0U;
	stateMode              = mode;
	OpenLoop.da_set        = ClampFloat(da, 0.0f, 1.0f);
	OpenLoop.db_set        = ClampFloat(db, 0.0f, 1.0f);
}

/**
 * @brief  按目标调制比 m = Vo/Vin = da/db 自动分配三模态 (Buck / BB / Boost) 开环占空比
 * @param  m: 目标调制比 (例如 0.5 为降压, 1.0 为过渡升降压, 1.5 为升压)
 */
void APP_OpenLoop_SetModRatio(float m)
{
	OpenLoop.use_mod_ratio = 1U;
	OpenLoop.m_set         = ClampFloat(m, 0.0f, 5.0f);
}

/**
 * @brief  开环控制步进更新函数（在主循环每 1ms 或定时器中断中调用）
 *         可通过调试器 Live Expressions 实时修改 OpenLoop 结构体与 stateMode
 */
void APP_OpenLoop_Step(void)
{
	float da_target = 0.0f;
	float db_target = 1.0f;

	/* 若总使能关闭或处于故障/空闲态，立即关断四路 PWM 输出 */
	if ((OpenLoop.enable == 0U) || (stateMode == MODE_FAULT))
	{
		BSP_HRTIM_DisableOutput();
		OpenLoop.da_out = 0.0f;
		OpenLoop.db_out = 1.0f;
		BSP_HRTIM_SetFSBBDuty(OpenLoop.da_out, OpenLoop.db_out);
		return;
	}

	if (OpenLoop.use_mod_ratio != 0U)
	{
		/* 方式 1：根据调制比 m_set 自动划分 Buck / BB过渡 / Boost 三模态（与 Simulink 逻辑一致） */
		float m      = ClampFloat(OpenLoop.m_set, 0.0f, 5.0f);
		float m_low  = ClampFloat(OpenLoop.m_low, 0.1f, 0.98f);
		float m_high = ClampFloat(OpenLoop.m_high, m_low + 0.05f, 5.0f);

		if (m <= m_low)
		{
			/* Buck 降压模式：Q1 调制，Q3 常通 (db = 1.0)，Q4 常关 */
			da_target = m;
			db_target = 1.0f;
			stateMode = MODE_Buck;
		}
		else if (m >= m_high)
		{
			/* Boost 升压模式：Q1 常通 (da = 1.0)，Q3/Q4 调制 (db = 1/m) */
			da_target = 1.0f;
			db_target = 1.0f / m;
			stateMode = MODE_Boost;
		}
		else
		{
			/* Buck-Boost 过渡模式：两侧桥臂同时调制，平滑衔接 */
			db_target = 1.0f + (m - m_low) * ((1.0f / m_high) - 1.0f) / (m_high - m_low);
			da_target = m * db_target;
			stateMode = MODE_BB;
		}
	}
	else
	{
		/* 方式 2：按 stateMode 与手动设定的 da_set / db_set 运行 */
		switch (stateMode)
		{
		case MODE_Buck:
			/* Buck 模式：A 侧按 da_set 斩波，B 侧上管 Q3 常通 (db = 1.0) */
			da_target = OpenLoop.da_set;
			db_target = 1.0f;
			break;

		case MODE_Boost:
			/* Boost 模式：A 侧上管 Q1 常通 (da = 1.0)，B 侧按 db_set 斩波 (Q4 占空比 = 1 - db_set) */
			da_target = 1.0f;
			db_target = OpenLoop.db_set;
			break;

		case MODE_BB:
			/* 四管 Buck-Boost 模式：A 侧按 da_set 斩波，B 侧按 db_set 斩波 */
			da_target = OpenLoop.da_set;
			db_target = OpenLoop.db_set;
			break;

		case MODE_IDLE:
		default:
			BSP_HRTIM_DisableOutput();
			OpenLoop.da_out = 0.0f;
			OpenLoop.db_out = 1.0f;
			BSP_HRTIM_SetFSBBDuty(OpenLoop.da_out, OpenLoop.db_out);
			return;
		}
	}

	da_target = ClampFloat(da_target, 0.0f, 1.0f);
	db_target = ClampFloat(db_target, 0.0f, 1.0f);

	/* 占空比斜坡缓变（软起动与防在线修改阶跃冲击） */
	OpenLoop.da_out = SlewLimit(OpenLoop.da_out, da_target, OpenLoop.slew_step);
	OpenLoop.db_out = SlewLimit(OpenLoop.db_out, db_target, OpenLoop.slew_step);

	/* 更新 HRTIM 比较寄存器并开启输出 */
	BSP_HRTIM_SetFSBBDuty(OpenLoop.da_out, OpenLoop.db_out);
	BSP_HRTIM_EnableOutput();
}
