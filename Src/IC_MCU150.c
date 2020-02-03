/*
 * IC_MCU150.c
 *
 *  Created on: Nov 28, 2019
 *      Author: gilg
 */

#include "IC_MCU.h"
#define hi2c	hi2c2

extern I2C_HandleTypeDef hi2c;

uint8_t eepromi2c[]=
{
		  0x00,0x7a,0x41,0x02,0x03,0x88,0x00,0x75,0x00,0x00,0x02,0x00,0x00,0x00,0x06,0x05,0x00,0xa5,0x00,0xff,0x0f,0x13,0x10,0x02,0x00,0x02,0xee,0x0e,0xe0,0x11,0x22,0x21,0x1f,0x9d,0xe1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4d,0x55,0x11,0x00,0x00,0x00,0x69,0x43
};


uint8_t crc8 (uint8_t* pData,int N)
{
	unsigned char ucDataStream = 0;
	int iCRC_CRC8Poly = 0x97 ;
	unsigned char ucCRC8;
	int i = 0;
	ucCRC8 = 1; // s t a r t value ! ! !
	for (int iReg = 0;iReg<N;iReg++)
	{
		ucDataStream = pData[iReg] ;
		for ( i =0; i <=7; i ++)
		{
			if (( ucCRC8 & 0x80 )!=(ucDataStream&0x80))
				ucCRC8 = (ucCRC8 << 1) ^ iCRC_CRC8Poly;
			else
				ucCRC8 =(ucCRC8<<1);
			ucDataStream = ucDataStream<<1;
		}
	}
	return ucCRC8;
}


uint16_t crc16(uint8_t* pData, int length)
{
    uint8_t i;
    uint16_t wCrc = 1;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

uint16_t crc16_with_begin(uint16_t begin,uint8_t* pData, int length)
{
    uint8_t i;
    uint16_t wCrc = begin;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}


uint16_t ic_mu150_calc_crc(uint8_t* data)
{
	uint16_t wCrc=crc16_with_begin(1,data,33);
	wCrc=crc16_with_begin(wCrc,data+48,16);
	return wCrc;
}

uint8_t ic_mu150_calc_crc_offset(uint8_t* data)
{
	uint8_t wCrc=crc8(data+35,12);
	return wCrc;
}

void ic_mu150_set_crc(uint8_t* data)
{
	uint16_t Crc16=ic_mu150_calc_crc(data);
	data[33]=Crc16>>8;
	data[34]=Crc16&0x00ff;
	uint8_t Crc8=ic_mu150_calc_crc_offset(data);
	data[47]=Crc8;
}


void write_page_eeprom(uint8_t number_page,uint8_t* data)
{
	uint8_t tx[9];
	tx[0]=8*number_page;
	memcpy((char*)tx+1,(char*)data,8);
	HAL_I2C_Master_Transmit(&hi2c,0xa0,tx,9,0xff);
	HAL_Delay(4);
}

void write_byte_eeprom(uint8_t number_byte,uint8_t byte)
{
	uint8_t tx[2];
	tx[0]=number_byte;
	tx[1]=byte;
	HAL_I2C_Master_Transmit(&hi2c,0xa0,tx,2,0xff);
	HAL_Delay(4);
}

void read_eeprom(uint8_t number_byte,uint8_t* data,uint8_t number_of_bytes)
{
	uint8_t tx=number_byte;
	HAL_I2C_Master_Transmit(&hi2c,0xa0,&tx,1,0xff);
	HAL_I2C_Master_Receive(&hi2c,0xa0,data,number_of_bytes,0xff);
}


void get_angle(struct __ic_mu150* ic_mu)
{
	  uint32_t data;
	  uint8_t tx_data[]={SDAD_Transmission,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	  uint8_t rx_data[4];
	  float bias= ic_mu->bias;

	  HAL_GPIO_WritePin(ic_mu->CS_PORT,ic_mu->CS_PIN,GPIO_PIN_RESET);
	  HAL_SPI_TransmitReceive(ic_mu->hspi,tx_data,rx_data,4,0xff);
	  HAL_GPIO_WritePin(ic_mu->CS_PORT,ic_mu->CS_PIN,GPIO_PIN_SET);

	  data=(rx_data[1]<<11)|(rx_data[2]<<3)|((rx_data[3]&0xe0)>>5);

	  float angle =360.0*data/524287.0;

	  ic_mu->angle=360-((angle+bias)>360.0f?(angle+bias-360.0f):(angle+bias));
}
ic_mu150* ic_mu150_init(SPI_HandleTypeDef* _hspi,GPIO_TypeDef * _CS_PORT,uint16_t _CS_PIN,float _bias)
{
	ic_mu150* _ic_mu150=malloc(sizeof(ic_mu150));
	_ic_mu150->hspi=_hspi;
	_ic_mu150->CS_PORT=_CS_PORT;
	_ic_mu150->CS_PIN=_CS_PIN;
	_ic_mu150->bias=_bias;
	_ic_mu150->read_angle=get_angle;
	return _ic_mu150;
}
