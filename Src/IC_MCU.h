/*
 * IC_MCU.h
 *
 *  Created on: Nov 28, 2019
 *      Author: gilg
 */

#ifndef IC_MCU_H_
#define IC_MCU_H_

#include "stm32g4xx_hal.h"
#include "string.h"
#include <stdlib.h>

enum opcode
{
	ACTIVATE =0xB0,
	SDAD_Transmission=0xA6,
	SDAD_Status=0xF5,
	Read_register=0x97,
	Write_register=0xD2,
	Register_status=0xAD
};

typedef struct __ic_mu150
{
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef * CS_PORT;
	uint16_t CS_PIN;
	float bias;
	float angle;
	float (*read_angle)(struct __ic_mu150 *ic_mu150);
}ic_mu150;


void read_eeprom(uint8_t number_byte,uint8_t* data,uint8_t number_of_bytes);
void write_page_eeprom(uint8_t number_page,uint8_t* data);
void write_byte_eeprom(uint8_t number_byte,uint8_t byte);
ic_mu150* ic_mu150_init(SPI_HandleTypeDef* _hspi,GPIO_TypeDef * _CS_PORT,uint16_t _CS_PIN,float bias);



#endif /* IC_MCU_H_ */
