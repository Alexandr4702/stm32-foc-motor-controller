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
    CO_ReturnError_t ret = CO_ERROR_NO;
    uint16_t i;
    /* verify arguments */
    if(CANmodule==NULL || CANdriverState==NULL || rxArray==NULL || txArray==NULL){
        ret = CO_ERROR_ILLEGAL_ARGUMENT;
    }
    /* Configure object variables */
    if(ret == CO_ERROR_NO){
        CANmodule->CANdriverState = CANdriverState;
        CANmodule->rxArray = rxArray;
        CANmodule->rxSize = rxSize;
        CANmodule->txArray = txArray;
        CANmodule->txSize = txSize;
        CANmodule->CANnormal = false;
        CANmodule->useCANrxFilters = true;
        CANmodule->bufferInhibitFlag = false;
        CANmodule->firstCANtxMessage = true;
        CANmodule->CANtxCount = 0U;
        CANmodule->errOld = 0U;
        CANmodule->em = NULL;

#ifdef CO_LOG_CAN_MESSAGES
        CANmodule->useCANrxFilters = false;
#endif

        for(i=0U; i<rxSize; i++){
            rxArray[i].ident = 0U;
            rxArray[i].mask = 0xFFFFFFFF;
            rxArray[i].object = NULL;
            rxArray[i].pFunct = NULL;
        }
        for(i=0U; i<txSize; i++){
            txArray[i].bufferFull = false;
        }
    }

	FDCAN_HandleTypeDef* caninit=CANdriverState;
	caninit->Instance = FDCAN1;
	caninit->Init.ClockDivider = FDCAN_CLOCK_DIV1;
	caninit->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
	caninit->Init.Mode = FDCAN_MODE_NORMAL;
	caninit->Init.AutoRetransmission = DISABLE;
	caninit->Init.TransmitPause = DISABLE;
	caninit->Init.ProtocolException = DISABLE;
	caninit->Init.NominalPrescaler = 170;
	caninit->Init.NominalSyncJumpWidth = 1;
	caninit->Init.NominalTimeSeg1 = 4;
	caninit->Init.NominalTimeSeg2 = 3;
	caninit->Init.DataPrescaler = 1;
	caninit->Init.DataSyncJumpWidth = 1;
	caninit->Init.DataTimeSeg1 = 1;
	caninit->Init.DataTimeSeg2 = 1;
	caninit->Init.StdFiltersNbr = 0;
	caninit->Init.ExtFiltersNbr = 0;
	caninit->Init.TxFifoQueueMode = FDCAN_TX_QUEUE_OPERATION;
	if (HAL_FDCAN_Init(caninit) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_Start(caninit) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(caninit, FDCAN_IT_RX_FIFO0_NEW_MESSAGE|FDCAN_IT_RX_FIFO1_NEW_MESSAGE|FDCAN_IT_TX_COMPLETE, 0) != HAL_OK)
	{
		Error_Handler();
	}
}


void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{
	FDCAN_HandleTypeDef* caninit=CANmodule->CANdriverState;


	HAL_FDCAN_DeInit(caninit);
	if (HAL_FDCAN_Stop(caninit) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_DeactivateNotification(caninit,FDCAN_IT_RX_FIFO0_NEW_MESSAGE ) != HAL_OK)
	{
		Error_Handler();
	}
}


uint16_t CO_CANrxMsg_readIdent(const CO_CANrxMsg_t *rxMsg)
{
	return rxMsg->ident;
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
    //safety
    if (!CANmodule || !object || !pFunct || index >= CANmodule->rxSize)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }
    CO_CANrx_t* rx_buffer=CANmodule->rxArray+index;
    rx_buffer->object=object;
    rx_buffer->pFunct=pFunct;


    rx_buffer->ident=rtr?ident|CAN_RTR_FLAG:ident&CAN_SFF_MASK;//ident;
    rx_buffer->mask=mask;
}


CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        bool_t                  rtr,
        uint8_t                 noOfBytes,
        bool_t                  syncFlag)
{

    CO_CANtx_t *buffer = NULL;

    if((CANmodule != NULL) && (index < CANmodule->txSize))
    {
        /* get specific buffer */
        buffer = &CANmodule->txArray[index];

        /* CAN identifier, bit aligned with CAN module registers */

        buffer->ident = ident & CAN_SFF_MASK;
        if(rtr)
        {
            buffer->ident |= CAN_RTR_FLAG;
        }

        buffer->DLC = noOfBytes;
        buffer->bufferFull = false;
        buffer->syncFlag = syncFlag;
    }

    return buffer;

}

CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{

	FDCAN_TxHeaderTypeDef tx_prop;

	tx_prop.Identifier = buffer->ident&CAN_SFF_MASK;
	tx_prop.IdType = FDCAN_STANDARD_ID;
	tx_prop.TxFrameType = ((buffer->ident&CAN_RTR_FLAG)==CAN_RTR_FLAG)?FDCAN_REMOTE_FRAME:FDCAN_DATA_FRAME;

	if(buffer->DLC>8)return CO_ERROR_ILLEGAL_ARGUMENT;
	tx_prop.DataLength = buffer->DLC<<16;


	tx_prop.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_prop.BitRateSwitch = FDCAN_BRS_OFF;
	tx_prop.FDFormat = FDCAN_CLASSIC_CAN;
	tx_prop.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_prop.MessageMarker = 0;

	HAL_StatusTypeDef ok=HAL_FDCAN_AddMessageToTxFifoQ(CANmodule->CANdriverState,&tx_prop,buffer->data);

	return ok==HAL_OK?CO_ERROR_NO:CO_ERROR_TX_BUSY;
}

void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{

}

void CO_CANverifyErrors(CO_CANmodule_t *CANmodule)
{

}
void CO_errExit(char *str)
{

}


void can_interrupt_rx(CO_CANmodule_t *CANmodule,CO_CANrxMsg_t* message)
{



    if(CANmodule == NULL)
    {
        CO_errExit("CO_CANreceive - CANmodule not configured.");

    }

    /* Read socket and pre-process message */


    if(CANmodule->CANnormal)
    {
		CO_CANrxMsg_t *rcvMsg;      /* pointer to received message in CAN module */
		uint32_t rcvMsgIdent;       /* identifier of the received message */
		CO_CANrx_t *buffer;         /* receive message buffer from CO_CANmodule_t object. */
		int i;
		bool_t msgMatched = false;

		rcvMsg = (CO_CANrxMsg_t *) &message;
		rcvMsgIdent = rcvMsg->ident;

		/* Search rxArray form CANmodule for the matching CAN-ID. */
		buffer = &CANmodule->rxArray[0];
		for(i = CANmodule->rxSize; i > 0U; i--)
		{
			if(((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U)
			{
				msgMatched = true;
				break;
			}
			buffer++;
		}
		/* Call specific function, which will process the message */
		if(msgMatched && (buffer->pFunct != NULL))
		{
			buffer->pFunct(buffer->object, rcvMsg);
		}
    }


}
