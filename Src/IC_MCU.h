/*
 * IC_MCU.h
 *
 *  Created on: Nov 28, 2019
 *      Author: gilg
 */

#ifndef IC_MCU_H_
#define IC_MCU_H_

#include "stm32g4xx_hal.h"

enum opcode
{
    ACTIVATE = 0xB0,
    SDAD_Transmission = 0xA6,
    SDAD_Status = 0xF5,
    Read_register = 0x97,
    Write_register = 0xD2,
    Register_status = 0xAD
};

typedef struct __ic_mu150
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *CS_PORT;
    uint16_t CS_PIN;
    float bias;
    float angle;
    void (*read_angle)(struct __ic_mu150 *device);
} ic_mu150;

void ic_mu150_init(ic_mu150 *device, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                   uint16_t cs_pin, float bias);
int ic_mu150_write_encoder_eeprom(I2C_HandleTypeDef *hi2c);

#endif /* IC_MCU_H_ */
