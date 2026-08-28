/*
 * Math_cal.h
 *
 *  Created on: 2026楠?閺?8閺?
 *      Author: Super_burger
 */

#ifndef CODE_MATH_CAL_H_
#define CODE_MATH_CAL_H_

float LowPassFilter(float current, float last, float alpha);

float StepApproach(float target, float step_size);

float AngleErrorNormalize(float error);

float StepApproachAngleMode(float target, float real_angle, float step_size, int mode);

#endif /* CODE_MATH_CAL_H_ */
