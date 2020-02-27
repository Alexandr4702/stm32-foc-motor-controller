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

#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/* Endianness */
# define BYTE_ORDER	__BYTE_ORDER

#ifdef BYTE_ORDER
#if BYTE_ORDER == LITTLE_ENDIAN
    #define CO_LITTLE_ENDIAN
#else
    #define CO_BIG_ENDIAN
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
#endif /* BYTE_ORDER */

/* Critical sections */
#ifdef CO_SINGLE_THREAD
    #define CO_LOCK_CAN_SEND()
    #define CO_UNLOCK_CAN_SEND()

    #define CO_LOCK_EMCY()
    #define CO_UNLOCK_EMCY()

    #define CO_LOCK_OD()
    #define CO_UNLOCK_OD()

    #define CANrxMemoryBarrier()
#else
    #define CO_LOCK_CAN_SEND()      /* not needed */
    #define CO_UNLOCK_CAN_SEND()

//    extern pthread_mutex_t CO_EMCY_mtx;
    #define CO_LOCK_EMCY()         // {if(pthread_mutex_lock(&CO_EMCY_mtx) != 0) CO_errExit("Mutex lock CO_EMCY_mtx failed");}
    #define CO_UNLOCK_EMCY()       // {if(pthread_mutex_unlock(&CO_EMCY_mtx) != 0) CO_errExit("Mutex unlock CO_EMCY_mtx failed");}

//    extern pthread_mutex_t CO_OD_mtx;
    #define CO_LOCK_OD()           // {if(pthread_mutex_lock(&CO_OD_mtx) != 0) CO_errExit("Mutex lock CO_OD_mtx failed");}
    #define CO_UNLOCK_OD()         // {if(pthread_mutex_unlock(&CO_OD_mtx) != 0) CO_errExit("Mutex unlock CO_OD_mtx failed");}

    #define CANrxMemoryBarrier()    {__sync_synchronize();}
#endif /* CO_SINGLE_THREAD */

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

#define CANrxMemoryBarrier()    {__sync_synchronize();}
#define IS_CANrxNew(rxNew) ((uintptr_t)rxNew)
#define SET_CANrxNew(rxNew) {CANrxMemoryBarrier(); rxNew = (void*)1L;}
#define CLEAR_CANrxNew(rxNew) {CANrxMemoryBarrier(); rxNew = (void*)0L;}
void can_interrupt_rx(CO_CANmodule_t *CANmodule,CO_CANrxMsg_t* message);

#endif /* CO_DRIVER_TARGET_H_ */
