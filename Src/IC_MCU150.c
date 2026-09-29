/*
 * IC_MCU150.c
 *
 *  Created on: Nov 28, 2019
 *      Author: gilg
 */

#include "IC_MCU.h"

#include <stddef.h>
#include <string.h>

static const uint8_t eeprom_image[] = {
    0x00, 0x7a, 0x41, 0x02, 0x03, 0x88, 0x00, 0x75, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x05,
    0x00, 0xa5, 0x00, 0xff, 0x0f, 0x13, 0x10, 0x02, 0x00, 0x02, 0xee, 0x0e, 0xe0, 0x11, 0x22, 0x21,
    0x1f, 0x9d, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x55, 0x11, 0x00, 0x00, 0x00, 0x69, 0x43};

static HAL_StatusTypeDef write_byte_eeprom(uint8_t address, uint8_t value, I2C_HandleTypeDef *hi2c)
{
    uint8_t tx[2] = {address, value};
    if (HAL_I2C_Master_Transmit(hi2c, 0xa0, tx, sizeof(tx), 0xff) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_Delay(4);
    return HAL_OK;
}

static HAL_StatusTypeDef read_eeprom(uint8_t address, uint8_t *data, uint8_t size,
                                     I2C_HandleTypeDef *hi2c)
{
    if (HAL_I2C_Master_Transmit(hi2c, 0xa0, &address, 1, 0xff) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Receive(hi2c, 0xa0, data, size, 0xff);
}

static void get_angle(struct __ic_mu150 *device)
{
    uint8_t tx_data[4] = {SDAD_Transmission, 0x00, 0x00, 0x00};
    uint8_t rx_data[4] = {0};

    HAL_GPIO_WritePin(device->CS_PORT, device->CS_PIN, GPIO_PIN_RESET);
    HAL_StatusTypeDef status =
        HAL_SPI_TransmitReceive(device->hspi, tx_data, rx_data, sizeof(tx_data), 0xff);
    HAL_GPIO_WritePin(device->CS_PORT, device->CS_PIN, GPIO_PIN_SET);
    if (status != HAL_OK)
    {
        return;
    }

    uint32_t raw_angle = (rx_data[1] << 11) | (rx_data[2] << 3) | ((rx_data[3] & 0xe0) >> 5);
    float angle = 360.0f * raw_angle / 524287.0f;
    float corrected_angle = angle + device->bias;
    device->angle =
        360.0f - (corrected_angle > 360.0f ? corrected_angle - 360.0f : corrected_angle);
}

void ic_mu150_init(ic_mu150 *device, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port,
                   uint16_t cs_pin, float bias)
{
    if (device == NULL)
    {
        return;
    }

    device->hspi = hspi;
    device->CS_PORT = cs_port;
    device->CS_PIN = cs_pin;
    device->bias = bias;
    device->angle = 0.0f;
    device->read_angle = get_angle;
}

int ic_mu150_write_encoder_eeprom(I2C_HandleTypeDef *hi2c)
{
    for (size_t i = 0; i < sizeof(eeprom_image); i++)
    {
        if (write_byte_eeprom((uint8_t)i, eeprom_image[i], hi2c) != HAL_OK)
        {
            return -1;
        }
    }

    uint8_t received[sizeof(eeprom_image)];
    if (read_eeprom(0, received, sizeof(received), hi2c) != HAL_OK)
    {
        return -1;
    }

    return memcmp(received, eeprom_image, sizeof(eeprom_image));
}
