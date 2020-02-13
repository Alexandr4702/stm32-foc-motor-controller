/*
 * foc.c
 *
 *  Created on: Aug 21, 2019
 *      Author: gilg
 */

#include "../foc/foc.h"



//static float K_p=.5f;
//static float K_i=0.1;

static float K_p=3.0f;
static float K_i=0.1;
#define M_PI_ 	(float)M_PI

//float delta_t=0.01f;
/*
 * brif
 * transform_dq
 */

float geom_angle_to_electric_angle(__IO float angle)
{

	static float wide_ang=360.0f/10.0f;
	float temp=angle/wide_ang;
	float curr_n=(temp-floorf(temp));
	return curr_n*360.0f;
}

void vector_pwv(float *output,float angle,float v_amp)
{


	static float V=1;

	static float M_PI_3=M_PI/3.0f;

	static float Tpwm=1.0;

	float cons=v_amp*Tpwm/V;

	angle=angle-floorf(angle/M_PI/2)*M_PI*2;


	float* T_=output;

  if((0.0f<=angle)&&(angle<=M_PI_3))
  {

	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T2=t2+t0/2;

    T_[0]=T1;
    T_[1]=T2;
    T_[2]=t0/2;
  }
  else if((M_PI_3<angle)&&(angle<=2*M_PI_3))
  {
	  angle-=M_PI_3;

	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T3=t1+t0/2;

    T_[0]=T3;
    T_[1]=T1;
    T_[2]=t0/2;
  }
  else if((M_PI_3*2<angle)&&(angle<=3*M_PI_3))
  {
	  angle-=M_PI_3*2;

	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T2=t2+t0/2;

    T_[0]=t0/2;
    T_[1]=T1;
    T_[2]=T2;
  }
  else if((M_PI_3*3<angle)&&(angle<=4*M_PI_3))
  {
	  angle-=M_PI_3*3;

	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T3=t1+t0/2;

    T_[0]=t0/2;
    T_[1]=T3;
    T_[2]=T1;
  }
  else if((M_PI_3*4<angle)&&(angle<=5*M_PI_3))
  {
	  angle-=M_PI_3*4;

	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T2=t2+t0/2;

    T_[0]=T2;
    T_[1]=t0/2;
    T_[2]=T1;
  }
  else if((M_PI_3*5<angle)&&(angle<=6*M_PI_3))
  {
	  angle-=M_PI_3*5;


	  float t1=cons*sinf(M_PI_3-angle);
	  float t2=cons*sinf(angle);
	  float t0=Tpwm-t1-t2;

	  float T1=t1+t2+t0/2;
	  float T3=t1+t0/2;

    T_[0]=T1;
    T_[1]=t0/2;
    T_[2]=T3;
  }

}


I_3_phase DQ_transformation_(const I_3_phase* I, float thetta)
{
    I_3_phase ret;

    float cos_=cosf(thetta);
    float sin_=sinf(thetta);

    float dif=1.73205080f*(I->B-I->C);
    float sum=2*I->A-I->B-I->C;
    ret.A=( sum*cos_+dif*sin_)/3;
    ret.B=(-sum*sin_+dif*cos_)/3;
    ret.C=0;//(I->A+I->B+I->C)/3;
    return ret;
}

I_3_phase DQ_Inverse_transformation_(const I_3_phase* I, float thetta)
{
    I_3_phase ret;
    float cos_=cosf(thetta);
    float sin_=sinf(thetta);
    ret.A=I->C+I->A*cos_+I->B*sin_;
    ret.B=I->C+(-0.5f*I->A-0.866025f*I->B)*cos_+(0.866025f*I->A-0.5f*I->B)*sin_;
    ret.C=I->C+(-0.5f*I->A+0.866025f*I->B)*cos_+(-0.866025f*I->A-0.5f*I->B)*sin_;
    return ret;
}

I_2_phase Klark_transformation(const I_3_phase* I)
{
	I_2_phase ret;

	//ret.alpha=I->A;
	//ret.betta=(I->B-I->C)/1.732;

	ret.alpha=0.666f*(I->A*1.0f-I->B*0.5-I->C*0.5f);
	ret.betta=0.666f*(I->A*0.0f+I->B*0.866-I->C*0.866f);
	return ret;
}

I_3_phase Inverse_Klark_transformation(const I_2_phase* I)
{
	I_3_phase ret;
	ret.A=I->alpha*3/2;
	ret.B=(-I->alpha*0.5f+I->betta*0.866f)*3/2;
	ret.C=(-I->alpha*0.5f-I->betta*0.866f)*3/2;
	return ret;
}

I_2_phase Park_transformation(const I_2_phase* I,float thetta)
{
	I_2_phase ret;
	float cos_=cosf(thetta);
	float sin_=sinf(thetta);
	ret.alpha=I->alpha*cos_-I->betta*sin_;
	ret.betta=I->alpha*sin_+I->betta*cos_;
	return ret;
}

I_2_phase Inverse_Park_transformation(const I_2_phase* I,float thetta)
{
	I_2_phase ret;
	float cos_=cosf(-thetta);
	float sin_=sinf(-thetta);
	ret.alpha=I->alpha*cos_-I->betta*sin_;
	ret.betta=I->alpha*sin_+I->betta*cos_;
	return ret;
}

I_2_phase DQ_transformation(const I_3_phase* I,float thetta)
{
	I_2_phase ret=Klark_transformation(I);
	ret=Park_transformation(&ret,thetta);
	return ret;
}

I_3_phase Inverse_DQ_transformation(const I_2_phase* I,float thetta)
{
	I_2_phase temp=Inverse_Park_transformation( I,thetta);
	I_3_phase ret=Inverse_Klark_transformation(&temp);
	return ret;
}

I_2_phase PI_regulator(I_2_phase* I0,I_2_phase* I,float delta_t)
{
	I_2_phase ret;
	I_2_phase e;
	static I_2_phase I_e={.alpha=0.0,.betta=0.0};
	e.alpha=(I0->alpha-I->alpha);
	e.betta=(I0->alpha-I->alpha);
	I_e.alpha+=e.alpha;
	I_e.betta+=e.betta;

	ret.alpha=e.alpha*K_p+I_e.alpha*K_i*delta_t;//id
	ret.betta=e.betta*K_p+I_e.betta*K_i*delta_t;//iq
	return ret;
}


I_3_phase PI_regulator_3ph(I_3_phase* I0,I_3_phase* I,float delta_t)
{
	I_3_phase ret;
	I_3_phase e;
	static I_3_phase I_e={.A=0.0,.B=0.0,.C=0.0};
	e.A=(I0->A-I->A);
	e.B=(I0->B-I->B);
	e.C=(I0->C-I->C);
	delta_t*=10;
	I_e.A=(I_e.A+e.A*delta_t)>1?I_e.A:(I_e.A+e.A*delta_t)<-1?I_e.A:(I_e.A+e.A*delta_t);
	I_e.B=(I_e.B+e.B*delta_t)>1?I_e.B:(I_e.B+e.B*delta_t)<-1?I_e.B:(I_e.B+e.B*delta_t);
	I_e.C=(I_e.C+e.C*delta_t)>1?I_e.C:(I_e.C+e.C*delta_t)<-1?I_e.C:(I_e.C+e.C*delta_t);

	ret.A=e.A*K_p+I_e.A*K_i;//id
	ret.B=e.B*K_p+I_e.B*K_i;//iq
	ret.C=e.C*K_p+I_e.C*K_i;//iq

	return ret;
}

float Get_Angle (const I_3_phase* I,float thetta)
{
	return 0;
}
