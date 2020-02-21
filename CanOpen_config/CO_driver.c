/*
 * CO_driver.c
 *
 *  Created on: Feb 21, 2020
 *      Author: gilg
 */

#include "CO_driver_target.h"



void CO_CANsetConfigurationMode(void *CANdriverState)
{

}

void CO_CANsetNormalMode(CO_CANmodule_t *CANmodule)
{

}

CO_ReturnError_t CO_CANmodule_init(
        CO_CANmodule_t         *CANmodule,
        void                   *CANdriverState,
        CO_CANrx_t              rxArray[],
        uint16_t                rxSize,
        CO_CANtx_t              txArray[],
        uint16_t                txSize,
        uint16_t                CANbitRate)
{

}


void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{

}


uint16_t CO_CANrxMsg_readIdent(const CO_CANrxMsg_t *rxMsg)
{

}

CO_ReturnError_t CO_CANrxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        uint16_t                mask,
        bool_t                  rtr,
        void                   *object,
        void                  (*pFunct)(void *object, const CO_CANrxMsg_t *message))
{

}


CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        bool_t                  rtr,
        uint8_t                 noOfBytes,
        bool_t                  syncFlag)
{

}

CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{

}

void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{

}

void CO_CANverifyErrors(CO_CANmodule_t *CANmodule)
{

}
