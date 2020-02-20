/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "IC_MCU.h"
#include <math.h>
#include <stdlib.h>
#include "../foc/foc.h"

#include "../parser_1010/parser_1010.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t message[200]={0x10,0x10};


#define size_pack	500
uint8_t Rx0[size_pack];
uint8_t Rx1[size_pack];
uint8_t* current_buf=Rx0;
uint32_t pr_CNDTR=size_pack;
uint32_t pr_read_time;
uint32_t read_period=20;





uint32_t sending_period=1;
uint32_t pr_sending_time=0;

uint32_t ADC[3];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc4;

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c2;

OPAMP_HandleTypeDef hopamp1;
OPAMP_HandleTypeDef hopamp2;
OPAMP_HandleTypeDef hopamp3;

SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim20;

UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_tx;
DMA_HandleTypeDef hdma_uart4_rx;

/* USER CODE BEGIN PV */
uint8_t str[200];
int strl;


float omega=0, omega_pr=0,omega_filtred=0,K_omega=0.0008,omega_0=360;
float phi, phi_pr,delta_phi,global_phi=0,phi_f=0;
float _phi_0=0;

float t =0;
float dt=0;

//------------------------------------------------------------------------------------------
enum
{
	moment=0,
	velo=1,
	pos=2,
	calibrate=3,
	ide,
	current_control
};

uint8_t mode=	moment;

typedef struct
{
	float P;
	float omega;
	float n;
	float phi;
	float S;
}motor_const;

motor_const m1=
{
		.P=0.9f,
		.omega =-16.00f,
		.phi=0,
		.n=10,
		.S=0
};



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_UART4_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_OPAMP1_Init(void);
static void MX_OPAMP2_Init(void);
static void MX_OPAMP3_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM20_Init(void);
static void MX_ADC4_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI3_Init(void);
static void MX_FDCAN1_Init(void);
/* USER CODE BEGIN PFP */

void send_data(__IO uint32_t *ADC_1,__IO uint32_t *ADC_2,__IO uint32_t *ADC_4,float phi,float GlobalPhi,float dt,float time,float omega)
{
	Mcdata data;
	static uint16_t cnt=0;


	data.ADC1_1=ADC_1[0];
	data.ADC1_2=ADC_1[1];
	data.ADC1_3=ADC_1[2];


	data.ADC2_1=ADC_2[0];
	data.ADC2_2=ADC_2[1];
	data.ADC2_3=ADC_2[2];

	data.ADC4_1=0;
	data.ADC4_2=0;
	data.ADC4_3=cnt;


	data.phi=phi;
	data.Globalphi=GlobalPhi;
	data.omega=omega;
	data.dt=dt;
	data.time=time;

	generate_message(str,&data,0x02,sizeof(Mcdata));
	cnt++;

	HAL_UART_Transmit_DMA(&huart4,str,sizeof(Mcdata)+2);


}


static void DWT_init(void)
{
	CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL|=DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    uint32_t us_count_tic =  us * (SystemCoreClock / 1000000);
    DWT->CYCCNT = 0U; // обнуляем счётчик
    while(DWT->CYCCNT < us_count_tic);
}

void delay_ns(uint32_t ns)
{
    uint32_t ns_count_tic =  ns * (SystemCoreClock / 1000000000);
    DWT->CYCCNT = 0U; // обнуляем счётчик
    while(DWT->CYCCNT < ns_count_tic);
}

int compare(const void * x1, const void * x2)
{
  return ( *(uint32_t*)x1 - *(uint32_t*)x2 );
}

uint32_t min_value(__IO uint32_t * data)
{
	uint32_t ptr[3];
	memcpy(ptr,data,12);
	qsort(ptr, 3, sizeof(uint32_t), compare);
	return ptr[2];//ptr[1]+(ptr[2]-ptr[1])/2;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void FDCAN_Config(void);
FDCAN_TxHeaderTypeDef message_can;
//  ={
//		  .Identifier=0x32,
//		  .IdType=FDCAN_STANDARD_ID,
//		  .TxFrameType=FDCAN_DATA_FRAME,
//		  .DataLength=FDCAN_DLC_BYTES_8,
//		  .ErrorStateIndicator=FDCAN_ESI_ACTIVE,
//		  .BitRateSwitch=FDCAN_BRS_OFF,
//		  .FDFormat=FDCAN_CLASSIC_CAN,
//		  .TxEventFifoControl=FDCAN_STORE_TX_EVENTS,
//		  .MessageMarker=0
//  };

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART4_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_SPI2_Init();
  MX_TIM20_Init();
  MX_ADC4_Init();
  MX_I2C2_Init();
  MX_SPI3_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */

//  volatile int status__eeprom =ic_mu150_write_encoder_eeprom(&hi2c2);
//
//  int strlen= sprintf(str,"hello world %i\r\n",status__eeprom);
//  HAL_UART_Transmit(&huart4,str,strlen,0xff);
////
//  HAL_GPIO_WritePin(CAN_SDB_GPIO_Port, CAN_SDB_Pin, GPIO_PIN_RESET);
//
  if(0)
  {
//	  HAL_GPIO_WritePin(CAN_SDB_GPIO_Port, CAN_SDB_Pin, GPIO_PIN_RESET);
	  FDCAN_Config();
	  uint8_t data[8]={1,2,3,4,5,6,7,8};
	  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1,&message_can,data);
	  while(1)
	  {
//		  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1,&message_can,data);

	  }
  }



  ic_mu150* encoder_gearbox= ic_mu150_init(&hspi2,SPI2_CS_GPIO_Port,SPI2_CS_Pin,0);
  ic_mu150*  encoder_motor= ic_mu150_init(&hspi3,SPI3_CS_GPIO_Port,SPI3_CS_Pin,0);



  DWT_init();

  HAL_OPAMP_Start(&hopamp1);
  HAL_OPAMP_Start(&hopamp2);
  HAL_OPAMP_Start(&hopamp3);

  HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc3,ADC_SINGLE_ENDED);


  EN_H;
  HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);
  HAL_TIMEx_OCN_Start(&htim20, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_2);
  HAL_TIMEx_OCN_Start(&htim20, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_3);
  HAL_TIMEx_OCN_Start(&htim20, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start_IT(&htim20, TIM_CHANNEL_4);

  float T[3];
  m1.P=0.1;
  m1.phi=0;
  m1.S=htim20.Instance->ARR/2;
  volatile GPIO_PinState DRIVER_FAULT_STATE;
  //---P-I-D----------------------------------------------------------
  float I_error=0;
  float p_error=0;
  float error=0;
  float D_error=0;
  //---P-I-D--V--------------------------------------------------------


  //------------------------------------------------------------------



  //---------------------------------------------------------
  HAL_StatusTypeDef status= HAL_UART_Receive_DMA(&huart4,Rx0,size_pack);
  uint16_t temp_adc;
  //--encoder-frequenc-------------------------------------------------------------------------
  HAL_StatusTypeDef ok1= HAL_ADC_Start(&hadc1);
  HAL_StatusTypeDef ok2 =HAL_ADC_Start(&hadc2);
  HAL_StatusTypeDef ok3 =HAL_ADC_Start(&hadc3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_StatusTypeDef ok1= HAL_ADC_Start(&hadc1);
//	  HAL_StatusTypeDef ok2 =HAL_ADC_Start(&hadc2);
//	  HAL_StatusTypeDef ok3 =HAL_ADC_Start(&hadc3);
	  HAL_StatusTypeDef ok4 =HAL_ADC_Start(&hadc4);


	  ADC[2]=4095-hadc1.Instance->DR;//W
	  ADC[0]=4095-hadc2.Instance->DR;//U
	  ADC[1]=4095-hadc3.Instance->DR;//V
	  temp_adc =hadc4.Instance->DR;


	  dt=((float )DWT->CYCCNT/ (SystemCoreClock ));
	  DWT->CYCCNT=0;
	  t=((float)HAL_GetTick())*0.001f;


	  DRIVER_FAULT_STATE = HAL_GPIO_ReadPin(DRIVER_FAULT_GPIO_Port,DRIVER_FAULT_Pin);


//	  encoder_motor->read_angle(encoder_motor);
//	  encoder_gearbox->read_angle(encoder_gearbox);
//	  phi=encoder_motor->angle;
			  //encoder_motor->angle;


	  delta_phi=((phi_pr>270.0f)&&(phi<90.0f))?(360.0f+phi-phi_pr):
			  ((phi_pr<90.0f)&&(phi>270.0f))?(-360.0f+phi-phi_pr):(phi-phi_pr);
	  phi_pr=phi;

	  global_phi+=delta_phi;

	  omega=delta_phi/dt;
	  omega_filtred+=K_omega*(omega-omega_filtred);
//debug


	  switch(mode)
	  {

	  case moment:
	  {
		  m1.P=0.1;//fabsf(_phi_0);
		  float sign =_phi_0==0?0:_phi_0/fabsf(_phi_0);
		  m1.phi=geom_angle_to_electric_angle(phi)*M_PI_/180.0f+M_PI_/2*sign;

		  m1.phi=t*0.5;

		  vector_pwv(T,-m1.phi,m1.P);
		  htim20.Instance->CCR1=(uint32_t)(T[0]*htim20.Instance->ARR);//U
		  htim20.Instance->CCR2=(uint32_t)(T[1]*htim20.Instance->ARR);//V
		  htim20.Instance->CCR3=(uint32_t)(T[2]*htim20.Instance->ARR);//W

		  htim20.Instance->CCR4=min_value(&htim20.Instance->CCR1);
		  break;
	  }
	  case velo:
	  {
		  //-angular-velsoty-control-----------------------------------------------------------------------------
		  p_error=error;
		  omega_0=_phi_0;
		  error=omega_0-omega_filtred;

		  D_error=(error-p_error)/dt;
		  I_error+=error*dt*70;

		  float c=900.0;
		  I_error=I_error>c?c:(I_error<-c?-c:I_error);


		  float PI_er_ =(error*0.005+I_error*0.001f+D_error*0.00001f*0);
		  float PI_er =fabsf(PI_er_);

		  float sign = PI_er==0?0:PI_er_/PI_er;
		  m1.phi=geom_angle_to_electric_angle(phi)*M_PI_/180.0f+M_PI_/2*sign;
		  m1.P=PI_er>0.9?0.9:PI_er<0?0:PI_er;

		  vector_pwv(T,-m1.phi,m1.P);
		  htim20.Instance->CCR1=(uint32_t)(T[0]*htim20.Instance->ARR);//U
		  htim20.Instance->CCR2=(uint32_t)(T[1]*htim20.Instance->ARR);//V
		  htim20.Instance->CCR3=(uint32_t)(T[2]*htim20.Instance->ARR);//W
		  break;
	  }
	  case pos:
	  {
		  //position-control-------------------------------------------------------------------------------
		  {
		  p_error=error;
		  error=_phi_0-global_phi;

		  D_error=(error-p_error)/dt;
		  I_error+=error*dt*70;

		  float c=80.0;
		  I_error=I_error>c?c:(I_error<-c?-c:I_error);

		  float PI_er_ =(error*0.05f+I_error*0.008f+D_error*0.00001f);
		  float PI_er =fabsf(PI_er_);

		  float sign = PI_er==0?0:PI_er_/PI_er;
		  m1.phi=geom_angle_to_electric_angle(phi)*M_PI_/180.0f+M_PI_/2*sign;
		  m1.P=PI_er>0.9?0.9:PI_er<0?0:PI_er;


		  vector_pwv(T,-m1.phi,m1.P);
		  htim20.Instance->CCR1=(uint32_t)(T[0]*htim20.Instance->ARR);//U
		  htim20.Instance->CCR2=(uint32_t)(T[1]*htim20.Instance->ARR);//V
		  htim20.Instance->CCR3=(uint32_t)(T[2]*htim20.Instance->ARR);//W
		  break;
		  }
	  }
	  case calibrate:
	  {
		  m1.P=0.0;//fabsf(_phi_0);
		  m1.phi=0;

		  vector_pwv(T,-m1.phi,m1.P);
		  htim20.Instance->CCR1=(uint32_t)(T[0]*htim20.Instance->ARR);//U
		  htim20.Instance->CCR2=(uint32_t)(T[1]*htim20.Instance->ARR);//V
		  htim20.Instance->CCR3=(uint32_t)(T[2]*htim20.Instance->ARR);//W
		  break;
	  }

	  case ide:
	  {
		  	  htim20.Instance->CCR1=0;
		  	  htim20.Instance->CCR2=0;
		  	  htim20.Instance->CCR3=0;
			  break;
	  }
	  case current_control:
	  {


//		  I_2_phase I0=
//		  {
//				  .alpha=0.0,
//				  .betta=0.1
//		  };
//		  I_3_phase I=
//		  {
//				  .A=ADC[0]*3.3f/4096.0f*6.06063f-10,
//				  .B=ADC[1]*3.3f/4096.0f*6.06063f-10,
//				  .C=ADC[2]*3.3f/4096.0f*6.06063f-10
//		  };
//
//		  I_2_phase I_dq=DQ_transformation(&I,geom_angle_to_electric_angle(phi)*M_PI_/180.0f);
//
//		  I_2_phase U_c=
//		  {
//				  .alpha=(I0.alpha-I_dq.alpha)*0.08,
//				  .betta=(I0.betta-I_dq.betta)*0.08
//		  };
//
//		  m1.phi=atan2f(U_c.betta,U_c.alpha)-M_PI;
//		  float power=sqrtf(U_c.betta*U_c.betta+U_c.alpha*U_c.alpha);
//		  m1.P=((power<0.3)&&(power>0.0))?power:0;


		  I_3_phase I=
		  {
				  .A=ADC[0]*3.3f/4096.0f*6.06063f-10,
				  .B=ADC[1]*3.3f/4096.0f*6.06063f-10,
				  .C=ADC[2]*3.3f/4096.0f*6.06063f-10
		  };

		  I_2_phase I_ab=Klark_transformation(&I);

		  I_2_phase I0_ab=
		  {
				  .alpha=0,
				  .betta=0
		  };

		  I_2_phase I_error;
		  static I_2_phase I_error_privious=
		  {
				  .alpha=0,
				  .betta=0
		  };
		  static I_2_phase I_error_integtral=
		  {
			  .alpha=0,
			  .betta=0
		  };

		  I_error.alpha=I0_ab.alpha-I_ab.alpha;
		  I_error.betta=I0_ab.betta-I_ab.betta;






		  vector_pwv(T,-m1.phi,m1.P);
		  htim20.Instance->CCR1=(uint32_t)(T[0]*htim20.Instance->ARR);//U
		  htim20.Instance->CCR2=(uint32_t)(T[1]*htim20.Instance->ARR);//V
		  htim20.Instance->CCR3=(uint32_t)(T[2]*htim20.Instance->ARR);//W
		  break;
	  }

	  }



	  //sinusoidal--PWM------------------------------------------------------------------------------------------
//	  htim20.Instance->CCR1=(uint32_t)((m1.P*cosf(m1.phi)+1.0f)*m1.S);
//	  htim20.Instance->CCR2=(uint32_t)((m1.P*cosf(m1.phi+2.0f*M_PI_/3.0f)+1.0f)*m1.S);
//	  htim20.Instance->CCR3=(uint32_t)((m1.P*cosf(m1.phi-2.0f*M_PI_/3.0f)+1.0f)*m1.S);
	  //sending--message-------------------------------------------------------------------------
	  if(HAL_GetTick()-pr_sending_time>sending_period)
	  {

		  if(huart4.gState==HAL_UART_STATE_READY)
		  {
			  send_data(
					  ADC,
					  &htim20.Instance->CCR1,
					  ADC
					  ,phi
					  ,global_phi
					  ,dt,_phi_0,m1.phi);
		  }
		  pr_sending_time=HAL_GetTick();
	  }

	  //--------------------------------------------------------------------------------------------------------


	  		  if(huart4.RxState==HAL_UART_STATE_READY)
	  		  {
	  			  uint16_t size=pr_CNDTR-huart4.hdmarx->Instance->CNDTR;
	  			  uint8_t* buff=huart4.pRxBuffPtr+size_pack-size-huart4.hdmarx->Instance->CNDTR;


	  			  if(huart4.pRxBuffPtr==Rx0)
	  			  {
	  				  HAL_UART_Receive_DMA(&huart4,Rx1,500);
	  			  }
	  			  else if(huart4.pRxBuffPtr==Rx1)
	  			  {
	  				  HAL_UART_Receive_DMA(&huart4,Rx0,500);
	  			  }

	  				  uint8_t pa;
	  				  messageStack stack;
	  				  do
	  				  {
	  					  pa=parser(buff,&size,&stack);
	  					  if((pa&0x40)==0x40)
	  					  {
	  						  switch(pa&0x3f)
	  						  {
	  						  case defaultMessageId:
	  							  _phi_0=stack.defaultMessage_.cnt;
	  							  break;
	  						  case McdataId:
	  							  break;
	  						  }
	  					  }
	  				  }
	  				  while((pa&0x80)==0x80&&(size>0));
	  				  current_buf=huart4.pRxBuffPtr;
	  				  pr_CNDTR=size_pack;

	  		  }


	  	//-------------------------------------------------------------------------------------

	  		  /*
	  		   *Uart reading
	  		   *
	  		   *
	  		   */
	  		  if(HAL_GetTick()-pr_read_time>=read_period)
	  		  {

	  			  if(current_buf!=huart4.pRxBuffPtr)
	  			  {

	  				  uint16_t size=pr_CNDTR;
	  				  uint8_t pa;
	  				  messageStack stack;
	  				  do
	  				  {
	  					  pa=parser(current_buf+size_pack-size-huart4.hdmarx->Instance->CNDTR,&size,&stack);
	  					  if((pa&0x40)==0x40)
	  					  {
	  						  switch(pa&0x3f)
	  						  {
	  						  case defaultMessageId:
	  							  _phi_0=stack.defaultMessage_.cnt;
	  							  break;
	  						  case McdataId:
	  							  break;
	  						  }
	  					  }
	  				  }
	  				  while((pa&0x80)==0x80&&(size>0));

	  				  current_buf=huart4.pRxBuffPtr;
	  				  pr_CNDTR=size_pack;

	  				  {

	  					  uint16_t size=pr_CNDTR-huart4.hdmarx->Instance->CNDTR;
	  					  uint8_t pa;
	  					  messageStack stack;
	  					  do
	  					  {
	  						  pa=parser(current_buf+size_pack-size-huart4.hdmarx->Instance->CNDTR,&size,&stack);
	  						  if((pa&0x40)==0x40)
	  						  {
	  							  switch(pa&0x3f)
	  							  {
	  							  case defaultMessageId:
	  								  _phi_0=stack.defaultMessage_.cnt;
	  								  break;
	  							  case McdataId:
	  								  break;
	  							  }
	  						  }

	  					  }
	  					  while((pa&0x80)==0x80&&(size>0));
	  					  pr_read_time=HAL_GetTick();

	  				  }


	  			  }
	  			  else
	  			  {
	  				  uint16_t size=pr_CNDTR-huart4.hdmarx->Instance->CNDTR;
	  				  if(size>0)
	  				  {
	  					    pr_CNDTR=huart4.hdmarx->Instance->CNDTR;
	  					    uint8_t pa;
	  					    messageStack stack;
	  					    do
	  					    {
	  					         pa=parser(current_buf+size_pack-size-huart4.hdmarx->Instance->CNDTR,&size,&stack);
	  							  if((pa&0x40)==0x40)
	  							  {
	  								  switch(pa&0x3f)
	  								  {
	  								  case defaultMessageId:
	  									  _phi_0=stack.defaultMessage_.cnt;
	  									  break;
	  								  case McdataId:
	  									  break;
	  								  }
	  							  }

	  					    }while((pa&0x80)==0x80&&(size>0));
	  				  }
	  				  pr_read_time=HAL_GetTick();
	  			  }
	  		  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_ADC12|RCC_PERIPHCLK_ADC345
                              |RCC_PERIPHCLK_FDCAN;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  PeriphClkInit.Adc345ClockSelection = RCC_ADC345CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T20_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode 
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_VOPAMP1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Common config 
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T20_TRGO;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_VOPAMP2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */
  /** Common config 
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.GainCompensation = 0;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T20_TRGO;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode 
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_VOPAMP3_ADC3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief ADC4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC4_Init(void)
{

  /* USER CODE BEGIN ADC4_Init 0 */

  /* USER CODE END ADC4_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC4_Init 1 */

  /* USER CODE END ADC4_Init 1 */
  /** Common config 
  */
  hadc4.Instance = ADC4;
  hadc4.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc4.Init.Resolution = ADC_RESOLUTION_12B;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.GainCompensation = 0;
  hadc4.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc4.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc4.Init.LowPowerAutoWait = DISABLE;
  hadc4.Init.ContinuousConvMode = DISABLE;
  hadc4.Init.NbrOfConversion = 1;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc4.Init.DMAContinuousRequests = DISABLE;
  hadc4.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc4.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc4) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC4_Init 2 */

  /* USER CODE END ADC4_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV10;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 170;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 4;
  hfdcan1.Init.NominalTimeSeg2 = 3;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_QUEUE_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x30A0A7FB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief OPAMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP1_Init(void)
{

  /* USER CODE BEGIN OPAMP1_Init 0 */

  /* USER CODE END OPAMP1_Init 0 */

  /* USER CODE BEGIN OPAMP1_Init 1 */

  /* USER CODE END OPAMP1_Init 1 */
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_HIGHSPEED;
  hopamp1.Init.Mode = OPAMP_PGA_MODE;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp1.Init.InternalOutput = ENABLE;
  hopamp1.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp1.Init.PgaConnect = OPAMP_PGA_CONNECT_INVERTINGINPUT_NO;
  hopamp1.Init.PgaGain = OPAMP_PGA_GAIN_16_OR_MINUS_15;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP1_Init 2 */

  /* USER CODE END OPAMP1_Init 2 */

}

/**
  * @brief OPAMP2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP2_Init(void)
{

  /* USER CODE BEGIN OPAMP2_Init 0 */

  /* USER CODE END OPAMP2_Init 0 */

  /* USER CODE BEGIN OPAMP2_Init 1 */

  /* USER CODE END OPAMP2_Init 1 */
  hopamp2.Instance = OPAMP2;
  hopamp2.Init.PowerMode = OPAMP_POWERMODE_HIGHSPEED;
  hopamp2.Init.Mode = OPAMP_PGA_MODE;
  hopamp2.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp2.Init.InternalOutput = ENABLE;
  hopamp2.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp2.Init.PgaConnect = OPAMP_PGA_CONNECT_INVERTINGINPUT_NO;
  hopamp2.Init.PgaGain = OPAMP_PGA_GAIN_16_OR_MINUS_15;
  hopamp2.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP2_Init 2 */

  /* USER CODE END OPAMP2_Init 2 */

}

/**
  * @brief OPAMP3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP3_Init(void)
{

  /* USER CODE BEGIN OPAMP3_Init 0 */

  /* USER CODE END OPAMP3_Init 0 */

  /* USER CODE BEGIN OPAMP3_Init 1 */

  /* USER CODE END OPAMP3_Init 1 */
  hopamp3.Instance = OPAMP3;
  hopamp3.Init.PowerMode = OPAMP_POWERMODE_HIGHSPEED;
  hopamp3.Init.Mode = OPAMP_PGA_MODE;
  hopamp3.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp3.Init.InternalOutput = ENABLE;
  hopamp3.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp3.Init.PgaConnect = OPAMP_PGA_CONNECT_INVERTINGINPUT_NO;
  hopamp3.Init.PgaGain = OPAMP_PGA_GAIN_16_OR_MINUS_15;
  hopamp3.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP3_Init 2 */

  /* USER CODE END OPAMP3_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM20 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM20_Init(void)
{

  /* USER CODE BEGIN TIM20_Init 0 */

  /* USER CODE END TIM20_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM20_Init 1 */

  /* USER CODE END TIM20_Init 1 */
  htim20.Instance = TIM20;
  htim20.Init.Prescaler = 0;
  htim20.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim20.Init.Period = 4249;
  htim20.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim20.Init.RepetitionCounter = 0;
  htim20.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim20) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim20, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim20) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC4REF;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim20, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM2;
  sConfigOC.Pulse = 2048;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  if (HAL_TIM_PWM_ConfigChannel(&htim20, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_1;
  sBreakDeadTimeConfig.DeadTime = 16;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim20, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM20_Init 2 */

  /* USER CODE END TIM20_Init 2 */
  HAL_TIM_MspPostInit(&htim20);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 576000;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EN_ZATVOR_GPIO_Port, EN_ZATVOR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAN_SDB_GPIO_Port, CAN_SDB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : EN_ZATVOR_Pin */
  GPIO_InitStruct.Pin = EN_ZATVOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(EN_ZATVOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DRIVER_FAULT_Pin */
  GPIO_InitStruct.Pin = DRIVER_FAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DRIVER_FAULT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CAN_SDB_Pin */
  GPIO_InitStruct.Pin = CAN_SDB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CAN_SDB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI3_CS_Pin */
  GPIO_InitStruct.Pin = SPI3_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI3_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI2_CS_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart4)
	{

		if(huart->pRxBuffPtr==Rx0)
		{
			HAL_UART_Receive_DMA(huart,Rx1,500);

		}
		else if(huart->pRxBuffPtr==Rx1)
		{
			HAL_UART_Receive_DMA(huart,Rx0,500);
		}


	}
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart4)
	{


//		 send_data(
//				 ADC,
//				 &htim20.Instance->CCR1,
//				 ADC
//				 ,phi
//				 ,global_phi
//				 ,dt,t,omega);


	}
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart4)
	{

	}
}


static void FDCAN_Config(void)
{
  FDCAN_FilterTypeDef sFilterConfig;

  /* Configure Rx filter */
  sFilterConfig.IdType = FDCAN_STANDARD_ID;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = 0x321;
  sFilterConfig.FilterID2 = 0x7FF;
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure global filter:
     Filter all remote frames with STD and EXT ID
     Reject non matching frames with STD ID and EXT ID */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Prepare Tx Header */
  message_can.Identifier = 0x321;
  message_can.IdType = FDCAN_STANDARD_ID;
  message_can.TxFrameType = FDCAN_DATA_FRAME;
  message_can.DataLength = FDCAN_DLC_BYTES_2;
  message_can.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  message_can.BitRateSwitch = FDCAN_BRS_OFF;
  message_can.FDFormat = FDCAN_CLASSIC_CAN;
  message_can.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  message_can.MessageMarker = 0;
}


void HAL_CAN_TxMailbox0CompleteCallback(FDCAN_HandleTypeDef *hcan)
{
if(12)
{

}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
