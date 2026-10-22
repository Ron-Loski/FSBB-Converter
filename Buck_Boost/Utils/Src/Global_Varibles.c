#include "Global_Varibles.h"

PIDtypedef currLoop = {
		.Kp = 0.0f,
		.Ki = 0.0f,
		.Kd = 0.0f,

		.IntegralMax = 0.0f,
		.IntegralMin = 0.0f,

		.OutMax = 0.0f,
		.OutMin = 0.0f,
};

PIDtypedef volLoop = {
		.Kp = 0.0f,
		.Ki = 0.0f,
		.Kd = 0.0f,

		.IntegralMax = 0.0f,
		.IntegralMin = 0.0f,

		.OutMax = 0.0f,
		.OutMin = 0.0f,
};

/* 默认进入 Buck 开环模式，可在调试时修改为 MODE_BB 或 MODE_Boost */
Mode_Statetypdef stateMode = MODE_Buck;

Sample_Offsettypedef Offset = {0};

Sys_Paratyedef ParaRaw = {0};
Sys_Paratyedef ParaReal = {0};

/* 开环控制默认参数 */
OpenLoop_Paramtypedef OpenLoop = {
		.enable        = 1U,     /* 1: 开启发波, 0: 关闭发波 */
		.use_mod_ratio = 0U,     /* 0: 按 stateMode + da_set/db_set 直接给定; 1: 按调制比 m_set 自动分配三模态 */
		.m_set         = 0.50f,  /* 目标调制比 M = da/db */
		.m_low         = 0.80f,  /* Buck 与 BB 过渡模式边界 */
		.m_high        = 1.25f,  /* BB 过渡与 Boost 模式边界 */
		.da_set        = 0.50f,  /* A 侧上管 Q1 占空比设定 (0.0 ~ 1.0) */
		.db_set        = 0.80f,  /* B 侧上管 Q3 占空比设定 (0.0 ~ 1.0，Boost 模式下下管 Q4 占空比 = 1 - db_set) */
		.da_out        = 0.0f,   /* 实际当前输出 Q1 占空比（缓变后） */
		.db_out        = 1.0f,   /* 实际当前输出 Q3 占空比（缓变后） */
		.slew_step     = 0.005f, /* 每次更新允许的最大占空比变化步长（软起动/防突变） */
};
