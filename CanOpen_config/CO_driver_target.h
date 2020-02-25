/*
 * CO_driver_target.h
 *
 *  Created on: Feb 21, 2020
 *      Author: gilg
 */

#ifndef CO_DRIVER_TARGET_H_
#define CO_DRIVER_TARGET_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "CO_types.h"

typedef bool bool_t;
typedef float                   float32_t;
typedef double                  float64_t;
typedef char                    char_t;
typedef unsigned char           oChar_t;
typedef unsigned char           domain_t;


typedef struct
{
    uint32_t            ident;
    uint8_t             DLC ;
    uint8_t             data[8];
}CO_CANrxMsg_t;


typedef struct{
    uint16_t            ident;
    uint16_t            mask;
    void               *object;
    void              (*pFunct)(void *object, const CO_CANrxMsg_t *message);
}CO_CANrx_t;


typedef struct{
    uint32_t            ident;
    uint8_t             DLC ;
    uint8_t             data[8];
    volatile bool_t     bufferFull;
    volatile bool_t     syncFlag;
}CO_CANtx_t;

typedef struct
{
	FDCAN_HandleTypeDef *CANdriverState;
    CO_CANrx_t         *rxArray;
    uint16_t            rxSize;
    CO_CANtx_t         *txArray;
    uint16_t            txSize;
    volatile bool_t     CANnormal;
    volatile bool_t     useCANrxFilters;
    volatile bool_t     bufferInhibitFlag;
    volatile bool_t     firstCANtxMessage;
    volatile uint16_t   CANtxCount;
    uint32_t            errOld;
    void               *em;
}CO_CANmodule_t;



#endif /* CO_DRIVER_TARGET_H_ */
