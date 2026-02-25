/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for SPI slave communication.
 *
 */


/*******************************************************************************
* Copyright (C) [2025], Microchip Technology Inc., and its subsidiaries. All rights reserved.
  
* The software and documentation is provided by Microchip and its contributors 
* "as is" and any express, implied or statutory warranties, including, but not 
* limited to, the implied warranties of merchantability, fitness for a particular 
* purpose and non-infringement of third party intellectual property rights are 
* disclaimed to the fullest extent permitted by law. In no event shall Microchip 
* or its contributors be liable for any direct, indirect, incidental, special,
* exemplary, or consequential damages (including, but not limited to, procurement 
* of substitute goods or services; loss of use, data, or profits; or business 
* interruption) however caused and on any theory of liability, whether in contract, 
* strict liability, or tort (including negligence or otherwise) arising in any way 
* out of the use of the software and documentation, even if advised of the 
* possibility of such damage.
* 
* Except as expressly permitted hereunder and subject to the applicable license terms 
* for any third-party software incorporated in the software and any applicable open 
* source software license terms, no license or other rights, whether express or 
* implied, are granted under any patent or other intellectual property rights of 
* Microchip or any third party.
 *******************************************************************************/


#include <definitions.h>
#include "platform-pic32cx.h"
#include <openthread-system.h>
#include <common/code_utils.hpp>
#include <utils/code_utils.h>
#include <openthread/platform/spi-slave.h>
#include <string.h>

#if OPENTHREAD_CONFIG_NCP_SPI_ENABLE

typedef enum
{
    SPI_TRANSACTION_DONE,
    SPI_TRANSACTION_IN_PROGRESS,
} otSpiSlaveTransactionState;

/* SPI Transmit Buffer Length */
#define SPI_SLAVE_TX_BUFFER_LEN    256U
/* SPI Receive Buffer Length */
#define SPI_SLAVE_RX_BUFFER_LEN    512U
/* SPI Small Packet Length - Minimum Length of the packet received from SPI Host processor */
#define SPI_SMALL_PACKET_LEN       OPENTHREAD_CONFIG_RCP_SPI_SMALL_PACKET_LEN

/* SPI Tranmsit DMA Channel Number */
#define SPI_SLAVE_TX_DMA_CHANNEL   DMAC_CHANNEL_0
/* SPI Receive DMA Channel Number */
#define SPI_SLAVE_RX_DMA_CHANNEL   DMAC_CHANNEL_1
/* SPI Slave Select Pin */
#define SPI_SS_PIN	GPIO_PIN_


extern OSAL_QUEUE_HANDLE_TYPE OTQueue;

/* Variable holding the context of SPI transaction from NCP layer */
static void  *sContext = NULL;
/* Variable holding the pointer of Output Buffer */
static uint8_t *sOutputBuf = NULL;
/* Variable holding the Length of Output Buffer */
static uint16_t sOutputBufLen = 0;
/* Variable holding the pointer of Input Buffer */
static uint8_t  *sInputBuf = NULL;
/* Variable holding the Length of Input Buffer */
static uint16_t  sInputBufLen  = 0;
/* Function pointer holding the SPI transaction complete callback */
otPlatSpiSlaveTransactionCompleteCallback sCompleteCallback = NULL;
/* Function pointer holding the SPI transaction complete callback */
otPlatSpiSlaveTransactionProcessCallback  sProcessCallback  = NULL;

/* Buffer holding SPI Tx packet to be transmitted */
static uint8_t spiTxDummyBuffer[SPI_SLAVE_TX_BUFFER_LEN];
/* Buffer holding SPI dummy packet to be transmitted */
static uint8_t TxBuffEmptyWithDataLen[10];
/* Buffer holding SPI Tx packet to be transmitted */
static uint8_t spiTxBuffer[SPI_SLAVE_TX_BUFFER_LEN];
/* Buffer holding SPI Rx packet to be received */
static uint8_t spiRxBuffer[SPI_SLAVE_RX_BUFFER_LEN];
/* Variable holding the length of received packet */
static uint16_t receivedBytes = 0;
/* Variable holding the state of Slave initiated transaction */
static volatile bool transactionRequested = false;
/* Variable holding the state of SPI transaction */
static otSpiSlaveTransactionState transactionState = SPI_TRANSACTION_DONE;
/* Variable hodling the number of failed transaction */
static volatile uint32_t nbReqDuringSpiTrans = 0;

/*******************************************************************************
 * Static
 ******************************************************************************/

static void CheckSpiSlaveTransactionStatus(void);
static void HandleSpiSlaveTxRequest(uint8_t *aOutputBuf, uint16_t aOutputBufLen);
static void otSpiSlaveDmaTxCallback(SYS_DMA_TRANSFER_EVENT event, uintptr_t context);
static void HandleSpiSlaveRxRequest(uint8_t *aInputBuf, uint16_t aInputBufLen);
static void otSpiSlaveDmaRxCallback(SYS_DMA_TRANSFER_EVENT event, uintptr_t context);


/* GPIO Change Notification Callback function of SPI Slave select pin */
void otSpiSlaveSSCallback (GPIO_PIN pin, uintptr_t context)
{
    /* Read state of SS Pin */
    bool PinState = GPIO_PinRead(pin);
    
    /* If SS = 1, Transaction is done, Call the SPI transaction complet callback */
    if(PinState && transactionState!= SPI_TRANSACTION_DONE )
    {
        transactionState = SPI_TRANSACTION_DONE;
        memcpy(spiTxDummyBuffer, spiTxBuffer, sOutputBufLen + 1);        
        receivedBytes = DMAC_ChannelGetTransferredCount(SPI_SLAVE_RX_DMA_CHANNEL); 
        DMAC_ChannelDisable(SPI_SLAVE_RX_DMA_CHANNEL);
        CheckSpiSlaveTransactionStatus();
    }  
    /* If SS = 0, Host asserted the pin, Start both TX and RX DMA transfers */
    else
    {  
        transactionState = SPI_TRANSACTION_IN_PROGRESS;      
        HandleSpiSlaveTxRequest(spiTxDummyBuffer, sOutputBufLen);
        HandleSpiSlaveRxRequest(sInputBuf, sInputBufLen );
    }
}

static void CheckSpiSlaveTransactionStatus(void)
{
    OT_Msg_T otUARTMsg; 
    
    if(sInputBuf == NULL || sOutputBuf == NULL)
    {
        sOutputBufLen = 0;
        sInputBufLen = 0;
    }

    if (sCompleteCallback(sContext, sOutputBuf, sOutputBufLen, sInputBuf, sInputBufLen, receivedBytes > sOutputBufLen ? receivedBytes : sOutputBufLen ))
    {
       otUARTMsg.OTMsgId = OT_MSG_SPI_SLAVE_PROCESS;
       OSAL_QUEUE_Send(&OTQueue, &otUARTMsg,0);

    }

}

/* Handling of SPI DMA Transmit */
static void HandleSpiSlaveTxRequest(uint8_t *aOutputBuf, uint16_t aOutputBufLen)
{
    size_t size = 0;

    if(SYS_DMA_ChannelIsBusy(SPI_SLAVE_TX_DMA_CHANNEL)== false)
    {
        SYS_DMA_AddressingModeSetup(SPI_SLAVE_TX_DMA_CHANNEL, SYS_DMA_SOURCE_ADDRESSING_MODE_INCREMENTED, SYS_DMA_DESTINATION_ADDRESSING_MODE_FIXED);
        size = aOutputBufLen;
        (void) SYS_DMA_ChannelTransfer(SPI_SLAVE_TX_DMA_CHANNEL, (const void *)aOutputBuf, (const void *)&_REGS->SPIS.SERCOM_DATA, (size_t)size); 
    }
}

/* Callback of SPI Tx DMA transfer */
static void otSpiSlaveDmaTxCallback(SYS_DMA_TRANSFER_EVENT event, uintptr_t context)
{
    uint16_t transSize = DMAC_ChannelGetTransferredCount(SPI_SLAVE_TX_DMA_CHANNEL);
            
    if((transSize == sOutputBufLen) || (sOutputBufLen == 0U))
    {
        //SpiTxDone = true;
    }

}

/* Handling of SPI DMA Receive */
static void HandleSpiSlaveRxRequest(uint8_t *aInputBuf, uint16_t aInputBufLen)
{  

    if(SYS_DMA_ChannelIsBusy(SPI_SLAVE_RX_DMA_CHANNEL) == false)
    {
        /* Configure the RX DMA channel - to receive data in receive buffer */
        SYS_DMA_AddressingModeSetup(SPI_SLAVE_RX_DMA_CHANNEL, SYS_DMA_SOURCE_ADDRESSING_MODE_FIXED, SYS_DMA_DESTINATION_ADDRESSING_MODE_INCREMENTED);
        (void) SYS_DMA_ChannelTransfer(SPI_SLAVE_RX_DMA_CHANNEL,(const void *)&_REGS->SPIS.SERCOM_DATA, (const void *)spiRxBuffer, SPI_SMALL_PACKET_LEN);
    }

}

/* Callback of SPI Rx DMA transfer */
static void otSpiSlaveDmaRxCallback(SYS_DMA_TRANSFER_EVENT event, uintptr_t context)
{
    receivedBytes = DMAC_ChannelGetTransferredCount(SPI_SLAVE_RX_DMA_CHANNEL);
     if(sInputBuf != NULL){
        memcpy(sInputBuf, spiRxBuffer, receivedBytes);
        }
}

/*******************************************************************************
 * Platform
 ******************************************************************************/
 /* Initialization of spi slave module */
void pic32cxSpiInit(void)
{
    size_t SpiTxDummyDataIdx;

    for (SpiTxDummyDataIdx = 0; SpiTxDummyDataIdx < sizeof(spiTxDummyBuffer); SpiTxDummyDataIdx++)
    {
        spiTxDummyBuffer[SpiTxDummyDataIdx] = 0xFF;
        spiTxBuffer[SpiTxDummyDataIdx] = 0xFf;
    }
    
}

/* SPI Slave Process function - Invoked after a transaction complete callback 
 * is called and returns `TRUE` to do any further processing required */
void pic32cxSpiSlaveProcess(OT_MsgId_T otUartMsgId)
{    
    if (OT_MSG_SPI_SLAVE_PROCESS == otUartMsgId)
    {
        sProcessCallback(sContext);
    }
}

/*******************************************************************************
 * SPI Slave
 ******************************************************************************/
/**
 * Shutdown and disable the SPI slave interface.
 */
void otPlatSpiSlaveDisable(void)
{
    sCompleteCallback = NULL;
    sProcessCallback  = NULL;
    sContext          = NULL;
}

/**
 * Initialize the SPI slave interface.

 * Note that SPI slave is not fully ready until a transaction is prepared using `otPlatSPISlavePrepareTransaction()`.
 *
 * If `otPlatSPISlavePrepareTransaction() is not called before the master begins a transaction, the resulting SPI
 * transaction will send all `0xFF` bytes and discard all received bytes.
 *
 * @param[in] aCompleteCallback  Pointer to transaction complete callback.
 * @param[in] aProcessCallback   Pointer to process callback.
 * @param[in] aContext           Context pointer to be passed to callbacks.
 *
 * @retval OT_ERROR_NONE     Successfully enabled the SPI Slave interface.
 * @retval OT_ERROR_ALREADY  SPI Slave interface is already enabled.
 * @retval OT_ERROR_FAILED   Failed to enable the SPI Slave interface.
 */
otError otPlatSpiSlaveEnable(otPlatSpiSlaveTransactionCompleteCallback aCompleteCallback,
                             otPlatSpiSlaveTransactionProcessCallback  aProcessCallback,
                             void                                     *aContext)
{   
   
    /*
     * SPI properties (based on openthread SPI recommendations):
     * CS is active low.
     * CLK is active high.
     * Data is valid on leading edge of CLK.
     * Data is sent in multiples of 8-bits (bytes).
     * Bytes are sent most-significant bit first.
     */
    
    otError result = OT_ERROR_NONE;
    
    if(aCompleteCallback != NULL && aProcessCallback != NULL)
    {
        // Check if SPI Slave interface is already enabled.
        otEXPECT_ACTION(sCompleteCallback == NULL, result = OT_ERROR_ALREADY);
        
        GPIO_PinInterruptCallbackRegister(SPI_SS_PIN, otSpiSlaveSSCallback, (uintptr_t)0 );
        GPIO_PinIntEnable(SPI_SS_PIN, GPIO_INTERRUPT_ON_BOTH_EDGES);
        
        /* Register callbacks for DMA */
        SYS_DMA_ChannelCallbackRegister(SPI_SLAVE_TX_DMA_CHANNEL, otSpiSlaveDmaTxCallback, 0);
        SYS_DMA_ChannelCallbackRegister(SPI_SLAVE_RX_DMA_CHANNEL, otSpiSlaveDmaRxCallback, 0);
        
        SYS_DMA_DataWidthSetup(SPI_SLAVE_TX_DMA_CHANNEL, SYS_DMA_WIDTH_8_BIT);
        SYS_DMA_DataWidthSetup(SPI_SLAVE_RX_DMA_CHANNEL, SYS_DMA_WIDTH_8_BIT);


        sCompleteCallback = aCompleteCallback;
        sProcessCallback  = aProcessCallback;
        sContext          = aContext;
        return result;
    }

exit:
    return result;
}

/**
 * Prepare data for the next SPI transaction. Data pointers MUST remain valid until the transaction complete callback
 * is called by the SPI slave driver, or until after the next call to `otPlatSpiSlavePrepareTransaction()`.
 *
 * Any call to this function while a transaction is in progress will cause all of the arguments to be ignored and the
 * return value to be `OT_ERROR_BUSY`.
 *
 * @param[in] aOutputBuf              Data to be written to MISO pin
 * @param[in] aOutputBufLen           Size of the output buffer, in bytes
 * @param[in] aInputBuf               Data to be read from MOSI pin
 * @param[in] aInputBufLen            Size of the input buffer, in bytes
 * @param[in] aRequestTransactionFlag Set to true if host interrupt should be set
 *
 * @retval OT_ERROR_NONE           Transaction was successfully prepared.
 * @retval OT_ERROR_BUSY           A transaction is currently in progress.
 * @retval OT_ERROR_INVALID_STATE  otPlatSpiSlaveEnable() hasn't been called.
 */
 
otError otPlatSpiSlavePrepareTransaction(uint8_t *aOutputBuf,
                                         uint16_t aOutputBufLen,
                                         uint8_t *aInputBuf,
                                         uint16_t aInputBufLen,
                                         bool     aRequestTransactionFlag)
{
    otError result = OT_ERROR_NONE;
    static bool isRequestFrameSent = true;
       
    if(sCompleteCallback != NULL)
    {
        otEXPECT_ACTION(transactionState !=SPI_TRANSACTION_IN_PROGRESS , result=OT_ERROR_BUSY);

        if (transactionState == SPI_TRANSACTION_DONE)
        {
            if (aInputBuf != NULL)
            {
                sInputBuf    = aInputBuf;
                sInputBufLen = aInputBufLen;
            }

            if(aOutputBuf != NULL)
            {
                sOutputBuf    = aOutputBuf;  
                sOutputBufLen = aOutputBufLen;
                
                uint16_t txDataLen = aOutputBuf[4] << 8 | aOutputBuf[3];         
            
                if (txDataLen > SPI_SMALL_PACKET_LEN && isRequestFrameSent)                    
                {
                    for (int i = 0; i < 10; i++)
                    {
                        if (i < 5)
                            TxBuffEmptyWithDataLen[i] = aOutputBuf[i];
                        else
                            TxBuffEmptyWithDataLen[i] = 0xFF;
                        
                    }
                    memcpy(spiTxDummyBuffer, TxBuffEmptyWithDataLen, 10);
                         
                    isRequestFrameSent = false;

                }
                else
                    {
                        memcpy(spiTxDummyBuffer, sOutputBuf, sOutputBufLen);
                        isRequestFrameSent = true;
                    }
                }

            if(aRequestTransactionFlag)
            {
                _SPI_Ready();
            }
  
        }     
        
    }

 exit:
    if (result == OT_ERROR_BUSY)
    {      
        nbReqDuringSpiTrans++;
    }
    return result;
}

#endif  //OPENTHREAD_CONFIG_NCP_SPI_ENABLE



