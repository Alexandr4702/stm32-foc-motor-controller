/*
 * foc.h
 *
 *  Created on: Aug 21, 2019
 *      Author: gilg
 */

#ifndef FOC_H_
#define FOC_H_

#include <math.h>
#include "stm32g4xx_hal.h"

typedef struct
{
    float A;
    float B;
    float C;
} I_3_phase;

typedef struct
{
    float alpha;
    float betta;
} I_2_phase;

I_2_phase Klark_transformation(const I_3_phase *I);
I_3_phase Inverse_Klark_transformation(const I_2_phase *I);

I_2_phase Park_transformation(const I_2_phase *I, float thetta);
I_2_phase Inverse_Park_transformation(const I_2_phase *I, float thetta);

I_2_phase DQ_transformation(const I_3_phase *I, float thetta);
I_3_phase Inverse_DQ_transformation(const I_2_phase *I, float thetta);

I_2_phase PI_regulator(I_2_phase *I0, I_2_phase *I, float delta_t);
I_3_phase PI_regulator_3ph(I_3_phase *I0, I_3_phase *I, float delta_t);

I_3_phase DQ_transformation_(const I_3_phase *I, float thetta);
I_3_phase DQ_Inverse_transformation_(const I_3_phase *I, float thetta);

void vector_pwv(float *exit, float angle, float v_amp);
float geom_angle_to_electric_angle(__IO float angle);

#endif /* FOC_H_ */
