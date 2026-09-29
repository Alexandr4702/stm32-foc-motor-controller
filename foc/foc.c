/*
 * foc.c
 *
 *  Created on: Aug 21, 2019
 *      Author: gilg
 */

#include "foc.h"

#include <math.h>

static const float K_p = 3.0f;
static const float K_i = 0.1f;
#define M_PI_ (float)M_PI

float geom_angle_to_electric_angle(float angle)
{
    static const float wide_ang = 360.0f / 10.0f;
    float temp = angle / wide_ang;
    float curr_n = (temp - floorf(temp));
    return curr_n * 360.0f;
}
void vector_pwm(float *output, float angle, float v_amp)
{
    static const float M_PI_3 = M_PI / 3.0f;
    static const float Tpwm = 1.0f;
    float cons = v_amp;

    angle = angle - floorf(angle / M_PI / 2) * M_PI * 2;
    float *T_ = output;

    if ((0.0f <= angle) && (angle <= M_PI_3))
    {

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T2 = t2 + t0 / 2;

        T_[0] = T1;
        T_[1] = T2;
        T_[2] = t0 / 2;
    }
    else if ((M_PI_3 < angle) && (angle <= 2 * M_PI_3))
    {
        angle -= M_PI_3;

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T3 = t1 + t0 / 2;

        T_[0] = T3;
        T_[1] = T1;
        T_[2] = t0 / 2;
    }
    else if ((M_PI_3 * 2 < angle) && (angle <= 3 * M_PI_3))
    {
        angle -= M_PI_3 * 2;

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T2 = t2 + t0 / 2;

        T_[0] = t0 / 2;
        T_[1] = T1;
        T_[2] = T2;
    }
    else if ((M_PI_3 * 3 < angle) && (angle <= 4 * M_PI_3))
    {
        angle -= M_PI_3 * 3;

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T3 = t1 + t0 / 2;

        T_[0] = t0 / 2;
        T_[1] = T3;
        T_[2] = T1;
    }
    else if ((M_PI_3 * 4 < angle) && (angle <= 5 * M_PI_3))
    {
        angle -= M_PI_3 * 4;

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T2 = t2 + t0 / 2;

        T_[0] = T2;
        T_[1] = t0 / 2;
        T_[2] = T1;
    }
    else if ((M_PI_3 * 5 < angle) && (angle <= 6 * M_PI_3))
    {
        angle -= M_PI_3 * 5;

        float t1 = cons * sinf(M_PI_3 - angle);
        float t2 = cons * sinf(angle);
        float t0 = Tpwm - t1 - t2;

        float T1 = t1 + t2 + t0 / 2;
        float T3 = t1 + t0 / 2;

        T_[0] = T1;
        T_[1] = t0 / 2;
        T_[2] = T3;
    }
}

I_2_phase clarke_transform(const I_3_phase *current)
{
    I_2_phase ret;

    ret.alpha = 0.666f * (current->A - current->B * 0.5f - current->C * 0.5f);
    ret.beta = 0.666f * (current->B * 0.866f - current->C * 0.866f);
    return ret;
}

I_3_phase inverse_clarke_transform(const I_2_phase *current)
{
    I_3_phase ret;
    ret.A = current->alpha * 3.0f / 2.0f;
    ret.B = (-current->alpha * 0.5f + current->beta * 0.866f) * 3.0f / 2.0f;
    ret.C = (-current->alpha * 0.5f - current->beta * 0.866f) * 3.0f / 2.0f;
    return ret;
}

I_2_phase park_transform(const I_2_phase *current, float theta)
{
    I_2_phase ret;
    float cos_ = cosf(theta);
    float sin_ = sinf(theta);
    ret.alpha = current->alpha * cos_ - current->beta * sin_;
    ret.beta = current->alpha * sin_ + current->beta * cos_;
    return ret;
}

I_2_phase inverse_park_transform(const I_2_phase *current, float theta)
{
    I_2_phase ret;
    float cos_ = cosf(-theta);
    float sin_ = sinf(-theta);
    ret.alpha = current->alpha * cos_ - current->beta * sin_;
    ret.beta = current->alpha * sin_ + current->beta * cos_;
    return ret;
}

I_2_phase dq_transform(const I_3_phase *current, float theta)
{
    I_2_phase ret = clarke_transform(current);
    ret = park_transform(&ret, theta);
    return ret;
}

I_3_phase inverse_dq_transform(const I_2_phase *current, float theta)
{
    I_2_phase temp = inverse_park_transform(current, theta);
    I_3_phase ret = inverse_clarke_transform(&temp);
    return ret;
}

I_2_phase pi_regulator(const I_2_phase *setpoint, const I_2_phase *current, float delta_t)
{
    I_2_phase ret;
    I_2_phase e;
    static I_2_phase I_e = {.alpha = 0.0f, .beta = 0.0f};
    e.alpha = setpoint->alpha - current->alpha;
    e.beta = setpoint->beta - current->beta;
    I_e.alpha += e.alpha;
    I_e.beta += e.beta;

    ret.alpha = e.alpha * K_p + I_e.alpha * K_i * delta_t; // id
    ret.beta = e.beta * K_p + I_e.beta * K_i * delta_t;    // iq
    return ret;
}
