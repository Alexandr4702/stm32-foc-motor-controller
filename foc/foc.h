/*
 * foc.h
 *
 *  Created on: Aug 21, 2019
 *      Author: gilg
 */

#ifndef FOC_H_
#define FOC_H_

typedef struct
{
    float A;
    float B;
    float C;
} I_3_phase;

typedef struct
{
    float alpha;
    float beta;
} I_2_phase;

I_2_phase clarke_transform(const I_3_phase *current);
I_3_phase inverse_clarke_transform(const I_2_phase *current);

I_2_phase park_transform(const I_2_phase *current, float theta);
I_2_phase inverse_park_transform(const I_2_phase *current, float theta);

I_2_phase dq_transform(const I_3_phase *current, float theta);
I_3_phase inverse_dq_transform(const I_2_phase *current, float theta);

I_2_phase pi_regulator(const I_2_phase *setpoint, const I_2_phase *current, float delta_t);

void vector_pwm(float *output, float angle, float v_amp);
float geom_angle_to_electric_angle(float angle);

#endif /* FOC_H_ */
