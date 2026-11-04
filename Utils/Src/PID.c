#include "Global_Inc.h"

void PID_Init(PIDtypedef *p)
{
	p->Target = 0;
	p->Actual = 0;
	p->Out = 0;
	p->Error0 = 0;
	p->Error1 = 0;
	p->Integral = 0;
}

void PID_Update(PIDtypedef *p)
{
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;
	
	if (p->Ki != 0)
	{
		p->Integral += p->Ki * p->Error0;
		if (p->Integral > p->IntegralMax) { p->Integral = p->IntegralMax; }
		if (p->Integral < p->IntegralMin) { p->Integral = p->IntegralMin; }
	}
	else
	{
		p->Integral = 0;
	}
	
	p->Out = p->Kp * p->Error0 
		   + p->Integral 
		   + p->Kd * (p->Error0 - p->Error1);
	
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}

float Clampfloat(float x, float min, float max)
{
	if (x > max) {return max;}
	if (x < min) {return min;}

	return x;
}

void PI_Update(PIDtypedef *p)
{
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;

	p->Integral = Clampfloat(p->Integral, p->IntegralMin, p->IntegralMax);

	float raw = p->Kp * p->Error0 + p->Integral;

	 /*
	 * 未饱和：允许积分。
	 * 已饱和：只允许帮助退出饱和的方向积分。
	 */
	if ((raw > p->OutMin && raw < p->OutMax) ||
		(raw >= p->OutMax && p->Error0 < 0.0f) ||
		(raw <= p->OutMin && p->Error0 > 0.0f))
	{
		p->Integral += p->Ki * p->Error0;
	}

	p->Integral = Clampfloat(p->Integral,
							 p->IntegralMin,
							 p->IntegralMax);

	raw = p->Kp * p->Error0 + p->Integral;
	p->Out = Clampfloat(raw, p->OutMin, p->OutMax);
}













