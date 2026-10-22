#ifndef __GLOBAL_TYPES_H_
#define __GLOBAL_TYPES_H_

#include <stdint.h>

typedef struct {
	float InputVol_Offset;
	float OutVol_Offset;
	float InputCurr_Offset;
	float OutCurr_Offset;
	float Lcurr_Offset;
} Sample_Offsettypedef;

typedef enum {
	MODE_IDLE = 0,
	MODE_Buck,
	MODE_BB,
	MODE_Boost,
	MODE_FAULT,
} Mode_Statetypdef;

typedef struct {
	float sysInputVol;
	float sysOutVol;
	float sysInputCurr;
	float sysOutCurr;
	float Lcurr;
} Sys_Paratyedef;

/**
 * @brief 开环控制参数结构体（可在调试器 Live Expressions 中实时修改）
 */
typedef struct {
	uint8_t enable;         /* 发波总使能：0-关闭输出，1-开启输出 */
	uint8_t use_mod_ratio;  /* 控制方式：0-按 stateMode 与手动占空比(da_set/db_set)运行，1-按调制比 m_set 自动分配三模态占空比 */
	float   m_set;          /* 目标调制比 M = Vo/Vin = da/db（use_mod_ratio=1 时生效，如 0.5 为降压，1.0 为过渡，1.5 为升压） */
	float   m_low;          /* Buck 与 BB 过渡模式边界（默认 0.80f） */
	float   m_high;         /* BB 过渡与 Boost 模式边界（默认 1.25f） */
	float   da_set;         /* 手动设定的 A 侧上管 Q1 占空比 (0.0 ~ 1.0) */
	float   db_set;         /* 手动设定的 B 侧上管 Q3 占空比 (0.0 ~ 1.0，注：Boost 下管占空比 = 1 - db_set) */
	float   da_out;         /* 当前实际输出的 Q1 占空比（只读观测） */
	float   db_out;         /* 当前实际输出的 Q3 占空比（只读观测） */
	float   slew_step;      /* 每次调用开环更新时的占空比最大缓变步长（软起动/防突变保护） */
} OpenLoop_Paramtypedef;

#endif
