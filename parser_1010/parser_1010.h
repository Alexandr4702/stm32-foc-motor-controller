/*
 * parser_1010.h
 *
 *  Created on: Sep 11, 2019
 *      Author: gilg
 */

#ifndef PARSER_1010_H_
#define PARSER_1010_H_

#include "main.h"
/*
 * list all id
 */

typedef enum
{
    defaultMessageId = 0x01,
    McdataId = 0x02,
} messageTypeid;

#pragma pack(push, 1)
/*
 * id struct for message id 0..127
 */
typedef struct
{
    uint8_t id; //=defaultMessageId;
    float cnt;
    uint8_t crc;
} defaultMessage;

typedef struct
{
    uint8_t id;
    uint16_t ADC1_1;
    uint16_t ADC1_2;
    uint16_t ADC1_3;

    uint16_t ADC2_1;
    uint16_t ADC2_2;
    uint16_t ADC2_3;

    uint16_t ADC4_1;
    uint16_t ADC4_2;
    uint16_t ADC4_3;
    float phi;
    float Globalphi;
    float omega;
    float dt;
    float time;
    uint8_t crc;
} Mcdata;

#pragma pack(pop)

/*
 *  message stack
 */

typedef struct
{
    defaultMessage defaultMessage_;
    Mcdata Mcdata_;
} messageStack;

uint8_t parser(const uint8_t *data, uint16_t *size, messageStack *stack);
void generate_message(void *message, const void *data, uint8_t id, uint16_t size);

#endif /* PARSER_1010_H_ */
