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

Mode_Statetypdef stateMode = MODE_IDLE;

Sample_Offsettypedef Offset = {0.0f};
