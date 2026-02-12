/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/*******************************************************************************
  BLE Transparent Server Profile Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_trsps.c

  Summary:
    Implements the server-side functionality of the BLE Transparent Profile.

  Description:
    This source file provides the necessary functions to initialize, configure,
    and manage the BLE Transparent Server Profile, enabling the application to
    handle data communication with a BLE Transparent Client.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "osal/osal_freertos.h"
#include "mba_error_defs.h"
#include "ble_gap.h"
#include "gatt.h"
#include "ble_util/mw_assert.h"
#include "ble_util/byte_stream.h"
#include "ble_trs/ble_trs.h"
#include "ble_trsps.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define BLE_TRSPS_INIT_CREDIT                   (0x10U)         // Initial credit value for BLE transparent profile.

// Maximum number of buffers in the incoming queue, derived from initial credits and max connections.
#define BLE_TRSPS_MAX_BUF_IN                    (BLE_TRSPS_INIT_CREDIT*BLE_TRSPS_MAX_CONN_NBR)

#define BLE_TRSPS_MAX_RETURN_CREDIT             (13U)           // Maximum number of credits that can be returned to the peer device.

#define BLE_TRSPS_CBFC_TX_ENABLED               (1U)            // Enable flag for CBFC (Credit Based Flow Control) transmission.
#define BLE_TRSPS_CBFC_RX_ENABLED               (1U << 1U)      // Enable flag for CBFC reception.

#define BLE_TRSPS_CCCD_DISABLE                  (0x0000U)       // CCCD (Client Characteristic Configuration Descriptor) value for disabling notifications.
#define BLE_TRSPS_CCCD_NOTIFY                   NOTIFICATION    // CCCD value for enabling notifications.

#define BLE_TRSPS_CBFC_OPCODE_SERVER_ENABLED    (0x14U)         // Opcode indicating the server has enabled CBFC.
#define BLE_TRSPS_CBFC_OPCODE_GIVE_CREDIT       (0x15U)         // Opcode for giving credit in CBFC.
#define BLE_TRSPS_CBFC_OPCODE_SUCCESS           (0U)            // Opcode indicating a successful operation in CBFC.

#define BLE_TRSPS_VENDOR_OPCODE_MIN             (0x20U)         // Minimum value for vendor-specific opcodes.
#define BLE_TRSPS_VENDOR_OPCODE_MAX             (0xFFU)         // Maximum value for vendor-specific opcodes.

#define BLE_TRSPS_RETRY_TYPE_RESP               (0x01U)         // Retry type for response messages.
#define BLE_TRSPS_RETRY_TYPE_ERR                (0x02U)         // Retry type for error messages.

typedef enum BLE_TRSPS_State_T
{
    BLE_TRSPS_STATE_IDLE = 0x00U,               // State indicating the service is idle and not connected.
    BLE_TRSPS_STATE_CONNECTED                   // State indicating the service is connected. 
} BLE_TRSPS_State_T;

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/* Structure for BLE transparent profile packet information. */
typedef struct BLE_TRSPS_PacketList_T
{
    uint8_t                    writeType;                       // Type of GATT write operation. See @ref BLE_GATT_WRITE_TYPES for possible values.
    uint16_t                   length;                          // Length of the data in bytes.
    uint8_t                    *p_packet;                       // Pointer to the data buffer for transmission or reception.
} BLE_TRSPS_PacketList_T;


/* Structure for managing the input queue of packets in the BLE transparent profile. */
typedef struct BLE_TRSPS_QueueIn_T
{
    uint8_t                    usedNum;                         // Number of packets currently in the input buffer.
    uint8_t                    writeIndex;                      // Index at which a new packet will be written into the buffer.
    uint8_t                    readIndex;                       // Index from which the next packet will be read from the buffer.
    BLE_TRSPS_PacketList_T     packetList[BLE_TRSPS_INIT_CREDIT];  // Array of packet buffers for queued packets.
} BLE_TRSPS_QueueIn_T;


/* Structure for storing connection-specific information in the BLE transparent profile. */
typedef struct BLE_TRSPS_ConnList_T
{
    uint8_t                    trsState;                        // Current state of the BLE transparent. See @ref BLE_TRSPS_STATUS for state definitions.
    BLE_TRSPS_State_T          state;                           // Connection state.
    uint16_t                   connHandle;                      // Connection handle associated with this connection.
    uint16_t                   attMtu;                          // The ATT Maximum Transmission Unit size for this connection.
    uint8_t                    cbfcEnable;                      // Flag indicating if credit-based flow control is enabled. See @ref BLE_TRSPS_CREDIT_BASED_FLOW_CONTROL.
    uint8_t                    peerCredit;                      // Number of credits granted by the central device to the peripheral.
    uint8_t                    localCredit;                     // Number of credits granted by the peripheral to the central device.
    uint8_t                    retryType;                       // Type of retry mechanism in use. See @ref BLE_TRSPS_RETRY_TYPE for retry types.
    uint8_t                    *p_retryData;                    // Pointer to the data buffer for retry operations.
    BLE_TRSPS_QueueIn_T        inputQueue;                      // Queue for incoming packets awaiting processing.
} BLE_TRSPS_ConnList_T;


// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************

static BLE_TRSPS_EventCb_T      bleTrspsProcess;                // Callback function type for BLE Transparent Service events processing.
static BLE_TRSPS_ConnList_T     s_trsConnList[BLE_TRSPS_MAX_CONN_NBR];// An array to keep track of the connection list for BLE Transparent Service.


// Assert to ensure that the total initial credits multiplied by the maximum number of connections
// is equal to the maximum buffer input size. This is a sanity check to ensure that the system
// resources are configured correctly and can handle the expected workload.
MW_ASSERT((BLE_TRSPS_MAX_CONN_NBR*BLE_TRSPS_INIT_CREDIT)==BLE_TRSPS_MAX_BUF_IN);

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Initialize a BLE Transparent Service connection list.
 *
 * @param p_conn Pointer to the BLE Transparent Service connection list to initialize.
 */
static void ble_trsps_InitConnList(BLE_TRSPS_ConnList_T *p_conn)
{
    (void)memset((uint8_t *)p_conn, 0, sizeof(BLE_TRSPS_ConnList_T));
    p_conn->attMtu= BLE_ATT_DEFAULT_MTU_LEN;
}


/**
 * @brief Retrieve a connection list entry by connection handle.
 *
 * @param connHandle The connection handle to search for in the connection list.
 * 
 * @retval Pointer to the connection list entry if found, otherwise NULL.
 */
static BLE_TRSPS_ConnList_T * ble_trsps_GetConnListByHandle(uint16_t connHandle)
{
    uint8_t i;

    for(i=0; i<BLE_TRSPS_MAX_CONN_NBR;i++)
    {
        if ((s_trsConnList[i].state == BLE_TRSPS_STATE_CONNECTED) && (s_trsConnList[i].connHandle == connHandle))
        {
            return &s_trsConnList[i];
        }
    }

    return NULL;
}


/**
 * @brief Get a free connection list entry.
 *
 * @retval Pointer to the free connection list entry if available, otherwise NULL.
 */
static BLE_TRSPS_ConnList_T *ble_trsps_GetFreeConnList(void)
{
    uint8_t i;

    for(i=0; i<BLE_TRSPS_MAX_CONN_NBR;i++)
    {
        if (s_trsConnList[i].state == BLE_TRSPS_STATE_IDLE)
        {
            s_trsConnList[i].state = BLE_TRSPS_STATE_CONNECTED;
            return &s_trsConnList[i];
        }
    }

    return NULL;
}


/**
 * @brief Return credit to the peer device for a given connection.
 *
 * @param p_conn Pointer to the BLE Transparent Service connection list entry.
 */
static void ble_trsps_ServerReturnCredit(BLE_TRSPS_ConnList_T *p_conn)
{
    GATTS_HandleValueParams_T hvParams;
    uint8_t *p_buf;

    if (p_conn->peerCredit == 0U)
    {
        return;
    }

    hvParams.sendType = ATT_HANDLE_VALUE_NTF;
    hvParams.charHandle = (uint16_t)TRS_HDL_CHARVAL_CTRL;
    hvParams.charLength = 0x05U;

    p_buf=hvParams.charValue;
    U8_TO_STREAM(&p_buf, BLE_TRSPS_CBFC_OPCODE_SUCCESS);
    U8_TO_STREAM(&p_buf, BLE_TRSPS_CBFC_OPCODE_SERVER_ENABLED);
    U16_TO_STREAM_BE(&p_buf, p_conn->attMtu);
    U8_TO_STREAM(&p_buf, p_conn->peerCredit);

    if (GATTS_SendHandleValue(p_conn->connHandle, &hvParams) == MBA_RES_SUCCESS)
    {
        p_conn ->peerCredit = 0;
    }
}


/**
 * @brief Handle received data for a given connection.
 *
 * @param p_conn            Pointer to the BLE Transparent Service connection list entry.
 * @param writeType         The type of write operation that was performed.
 * @param receivedLen       The length of the received data.
 * @param p_receivedValue   Pointer to the received data.
 */
static void ble_trsps_RcvData(BLE_TRSPS_ConnList_T *p_conn, uint8_t writeType, uint16_t receivedLen, uint8_t *p_receivedValue)
{
    if (p_conn->inputQueue.usedNum < BLE_TRSPS_INIT_CREDIT)
    {
        BLE_TRSPS_Event_T evtPara;
        uint8_t *p_buffer = NULL;


        (void)memset((uint8_t *) &evtPara, 0, sizeof(evtPara));
        p_buffer = OSAL_Malloc(receivedLen);
        
        if (p_buffer == NULL)
        {
            evtPara.eventId = BLE_TRSPS_EVT_ERR_NO_MEM;
            if (bleTrspsProcess != NULL)
            {
                bleTrspsProcess(&evtPara);
            }
            return;
        }

        (void)memcpy(p_buffer, p_receivedValue, receivedLen);
        p_conn->inputQueue.packetList[p_conn->inputQueue.writeIndex].writeType = writeType;
        p_conn->inputQueue.packetList[p_conn->inputQueue.writeIndex].length = receivedLen;
        p_conn->inputQueue.packetList[p_conn->inputQueue.writeIndex].p_packet = p_buffer;
        p_conn->inputQueue.writeIndex++;
        if (p_conn->inputQueue.writeIndex >= BLE_TRSPS_INIT_CREDIT)
        {
            p_conn->inputQueue.writeIndex = 0;
        }

        p_conn->inputQueue.usedNum++;

        evtPara.eventId=BLE_TRSPS_EVT_RECEIVE_DATA;
        evtPara.eventField.onReceiveData.connHandle = p_conn->connHandle;
        if (bleTrspsProcess != NULL)
        {
            bleTrspsProcess(&evtPara);
        }
    }

}


/**
 * @brief Free the retry data buffer for a given connection.
 *
 * @param p_conn Pointer to the BLE Transparent Service connection list entry.
 */
static void ble_trsps_FreeRetryData(BLE_TRSPS_ConnList_T *p_conn)
{
    if (p_conn->p_retryData != NULL)
    {
        OSAL_Free(p_conn->p_retryData);
        p_conn->p_retryData = NULL;
        p_conn->retryType = 0;
    }
}


/**
 * @brief Check if there is any queued task that needs to be processed.
 *
 * @retval True if there is a queued task, false otherwise.
 */
static bool ble_trsps_CheckQueuedTask(void)
{
    uint8_t i;
    
    for(i=0; i < BLE_TRSPS_MAX_CONN_NBR; i++)
    {
        if ((s_trsConnList[i].state == BLE_TRSPS_STATE_CONNECTED) && (s_trsConnList[i].peerCredit >= BLE_TRSPS_MAX_RETURN_CREDIT))
        {
            return true;
        }
        if (s_trsConnList[i].p_retryData != NULL)
        {
            return true;
        }
    }

    return false;
}


/**
 * @brief Process queued tasks for the BLE Transparent Service Profile Server.
 *
 * @param[in] connHandle The connection handle for which the queued tasks are to be processed.
 */
static void ble_trsps_ProcessQueuedTask(uint16_t connHandle)
{
    BLE_TRSPS_ConnList_T *p_connList;
    uint16_t status;

    p_connList = ble_trsps_GetConnListByHandle(connHandle);
    if (p_connList == NULL)
    {
        BLE_TRSPS_Event_T evtPara;
        evtPara.eventId = BLE_TRSPS_EVT_ERR_UNSPECIFIED;
        if (bleTrspsProcess != NULL)
        {
            bleTrspsProcess(&evtPara);
        }
        return;
    }
    if (p_connList->p_retryData != NULL)
        {
        if (p_connList->retryType == BLE_TRSPS_RETRY_TYPE_RESP)
            {
            status = GATTS_SendWriteResponse(p_connList->connHandle, 
                (GATTS_SendWriteRespParams_T *) p_connList->p_retryData);
                if (status == MBA_RES_SUCCESS)
                {
                ble_trsps_FreeRetryData(p_connList);
                }
            }
        else if (p_connList->retryType == BLE_TRSPS_RETRY_TYPE_ERR)
            {
            status = GATTS_SendErrorResponse(p_connList->connHandle, 
                (GATTS_SendErrRespParams_T *) p_connList->p_retryData);
                if (status == MBA_RES_SUCCESS)
                {
                ble_trsps_FreeRetryData(p_connList);
                }
            }
            else
            {
            ble_trsps_FreeRetryData(p_connList);
        }
    }

    if (p_connList->state == BLE_TRSPS_STATE_CONNECTED)
    {
        if (p_connList->peerCredit >= BLE_TRSPS_MAX_RETURN_CREDIT)
        {
            ble_trsps_ServerReturnCredit(p_connList);
        }
    }
}


/**
 * @brief Registers a callback function for BLE Transparent Profile server events.
 *
 * @param[in] bleTranServHandler          	The client callback function to handle BLE Transparent Profile server events.
 */
void BLE_TRSPS_EventRegister(BLE_TRSPS_EventCb_T bleTranServHandler)
{
    bleTrspsProcess = bleTranServHandler;
}


/**
 * @brief Initializes the BLE Transparent Profile.
 *
 * @retval MBA_RES_SUCCESS                 	Service successfully added to the service table.
 * @retval MBA_RES_FAIL                  		Failed to add the service to the service table.
 * @retval MBA_RES_OOM                  		Internal memory allocation failure.
 */
uint16_t BLE_TRSPS_Init(void)
{
    uint8_t i;

    for (i = 0; i < BLE_TRSPS_MAX_CONN_NBR; i++)
    {
        ble_trsps_InitConnList(&s_trsConnList[i]);
    }

    return BLE_TRS_Add();
}

uint16_t BLE_TRSPS_SendVendorCommand(uint16_t connHandle, uint8_t commandID, uint8_t commandLength, uint8_t *p_commandPayload)
{
    BLE_TRSPS_ConnList_T *p_conn;
    GATTS_HandleValueParams_T *p_hvParams;
    uint16_t result;

    p_conn = ble_trsps_GetConnListByHandle(connHandle);
    if (p_conn == NULL)
    {
        return MBA_RES_FAIL;
    }

    if (commandID < BLE_TRSPS_VENDOR_OPCODE_MIN)
    {
        return MBA_RES_INVALID_PARA;
    }

    if (commandLength > (p_conn->attMtu-ATT_NOTI_INDI_HEADER_SIZE-1U))
    {
        return MBA_RES_INVALID_PARA;
    }

    p_hvParams = OSAL_Malloc(sizeof(GATTS_HandleValueParams_T));
    if (p_hvParams != NULL)
    {
        p_hvParams->charHandle = (uint16_t)TRS_HDL_CHARVAL_CTRL;
        p_hvParams->charLength = ((uint16_t)commandLength+1U);
        p_hvParams->charValue[0] = commandID;
        (void)memcpy(&p_hvParams->charValue[1], p_commandPayload, commandLength);
        p_hvParams->sendType = ATT_HANDLE_VALUE_NTF;
        result = GATTS_SendHandleValue(p_conn->connHandle, p_hvParams);
        OSAL_Free(p_hvParams);
        
        return result;
    }
    else
    {
        return MBA_RES_OOM;
    }
}

// strip color coding and leading/trailing newline
static uint16_t BLE_TRSPS_strip_response(char *s)
{
    char *src = s;
    char *dst = s;

    if(*src == '\r' && src[1] == '\n')
    {
        src += 2;                 // remove leading newline
    }
    while(*src)
    {
        if(*src == '\x1B' && src[1] == '[')
        {
            src += 2;                 // ESC [
            while(*src && (*src != 'm')     // remove color coding
                       && (*src != 'K')     // remove line deletion from cursor
                       && (*src != 'D'))    // remove curser movement left
            {
                src++;               // skip numbers
            }
            if(   (*src == 'm')     // skip 'm'
               || (*src == 'K')     // skip 'K'
               || (*src == 'D'))    // skip 'D'
            {
                src++;
            }
        }
        else
        {
            *dst++ = *src++;
        }
    }
    if(*(dst - 2) == '\r' && *(dst - 1) == '\n')
    {
        dst -= 2;                 // remove trailing newline
    }
    *dst = '\0';
    return (uint16_t)(dst - s);
}

/**
 * @brief Sends transparent data over BLE.
 *
 * @param[in] connHandle                    Connection handle associated with this connection.
 * @param[in] len                           The length of the data to be sent.
 * @param[in] p_data                        Pointer to the data to be sent.
 *
 * @retval MBA_RES_SUCCESS                  Data successfully sent.
 * @retval MBA_RES_OOM                      Internal memory allocation failure.
 * @retval MBA_RES_INVALID_PARA             Invalid parameters; data length does not meet specifications.
 */
uint16_t BLE_TRSPS_SendData(uint16_t connHandle, uint16_t len, uint8_t *p_data)
{
    GATTS_HandleValueParams_T  hvParams;
    BLE_TRSPS_ConnList_T *p_conn;
    uint16_t result;

    p_conn = ble_trsps_GetConnListByHandle(connHandle);
    if (p_conn == NULL)
    {
        return MBA_RES_FAIL;
    }

    if (p_conn->trsState != BLE_TRSPS_STATUS_TX_OPENED)
    {
        return MBA_RES_BAD_STATE;
    }

    if (((p_conn->cbfcEnable&BLE_TRSPS_CBFC_TX_ENABLED)!=0U) && (p_conn->localCredit == 0U))
    {
        return MBA_RES_NO_RESOURCE;
    }

    if (ble_trsps_CheckQueuedTask())
    {
        return MBA_RES_NO_RESOURCE;
    }

    if (len > (p_conn->attMtu - ATT_HANDLE_VALUE_HEADER_SIZE))
    {
        return MBA_RES_FAIL;
    }

    hvParams.charHandle = (uint16_t)TRS_HDL_CHARVAL_TX;
    hvParams.charLength = len;
    (void)memcpy(hvParams.charValue, p_data, hvParams.charLength);
    hvParams.sendType = ATT_HANDLE_VALUE_NTF;

    // strip color coding and leading/trailing newline
    hvParams.charValue[hvParams.charLength] = '\0';
    hvParams.charLength = BLE_TRSPS_strip_response((char *)hvParams.charValue);

    if(hvParams.charLength > 0)
    {
        result = GATTS_SendHandleValue(p_conn->connHandle, &hvParams);
        if (result == MBA_RES_SUCCESS)
        {
            if ((p_conn->cbfcEnable&BLE_TRSPS_CBFC_TX_ENABLED) != 0U)
            {
                p_conn->localCredit--;
            }
        }
    }
    else
    {   // empty string, no need to send
        result = MBA_RES_SUCCESS;
    }

    return result;
}


/**
 * @brief Retrieves the length of data queued for transmission.
 *
 * @param[in]    connHandle                 Connection handle associated with the queued data.
 * @param[out]   p_dataLength               Pointer to where the data length will be stored.
 */
void BLE_TRSPS_GetDataLength(uint16_t connHandle, uint16_t *p_dataLength)
{
    BLE_TRSPS_ConnList_T *p_conn = NULL;

    p_conn = ble_trsps_GetConnListByHandle(connHandle);
    if (p_conn != NULL)
    {
        if ((p_conn->inputQueue.usedNum) > 0U)
        {
            *p_dataLength = p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].length;
        }
        else
        {
            *p_dataLength = 0;
        }
    }
    else
    {
        *p_dataLength = 0;
    }
}


/**
 * @brief Retrieves queued data from the profile.
 *
 * @param[in] connHandle                    Connection handle associated with the queued data.
 * @param[out] p_data                       Pointer to the buffer where the data will be stored.
 *
 * @retval MBA_RES_SUCCESS                  Data successfully retrieved.
 * @retval MBA_RES_FAIL                     No data in the queue or the connection link could not be found.
 *
 */
uint16_t BLE_TRSPS_GetData(uint16_t connHandle, uint8_t *p_data)
{
    BLE_TRSPS_ConnList_T *p_conn = NULL;
    uint8_t writeType = 0;

    p_conn = ble_trsps_GetConnListByHandle(connHandle);
    if (p_conn!= NULL)
    {
        if ((p_conn->inputQueue.usedNum) > 0U)
        {
            if (p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet != NULL)
            {
                (void)memcpy(p_data, p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet, 
                    p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].length);
                OSAL_Free(p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet);
                p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet = NULL;
            }
            
            writeType = p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].writeType;

            p_conn->inputQueue.readIndex++;
            if (p_conn->inputQueue.readIndex >= BLE_TRSPS_INIT_CREDIT)
            {
                p_conn->inputQueue.readIndex = 0;
            }

            p_conn->inputQueue.usedNum --;
            
            if (((p_conn->cbfcEnable&BLE_TRSPS_CBFC_RX_ENABLED)!=0U)
            && (writeType == ATT_WRITE_CMD))
            {
                p_conn->peerCredit++;
                if (p_conn->peerCredit >= BLE_TRSPS_MAX_RETURN_CREDIT)
                {
                    ble_trsps_ServerReturnCredit(p_conn);
                }
            }

            return MBA_RES_SUCCESS;
        }
        else
        {
            return MBA_RES_FAIL;
        }
    }
    else
    {
        return MBA_RES_FAIL;
    }

}


/**
 * @brief Handle the write request to the TX CCCD (Client Characteristic Configuration Descriptor).
 *
 * @param p_conn Pointer to the connection list.
 * @param p_value Pointer to the value to be written to the CCCD.
 * 
 * @retval uint8_t Returns 0 on success, ATT_ERR_APPLICATION_ERROR on failure.
 */
static uint8_t ble_trsps_TxCccd(BLE_TRSPS_ConnList_T *p_conn, uint8_t *p_value)
{
    uint16_t cccd;
    BLE_TRSPS_Event_T evtPara;

    (void)memset((uint8_t *) &evtPara, 0, sizeof(evtPara));
    BUF_LE_TO_U16(&cccd, p_value);

    if (cccd == BLE_TRSPS_CCCD_NOTIFY)
    {
        p_conn->trsState = BLE_TRSPS_STATUS_TX_OPENED;
    }
    else if(cccd == BLE_TRSPS_CCCD_DISABLE)
    {
        p_conn->trsState = BLE_TRSPS_STATUS_TX_DISABLED;
    }
    else
    {
        return ATT_ERR_APPLICATION_ERROR;
    }

    evtPara.eventId=BLE_TRSPS_EVT_TX_STATUS;
    evtPara.eventField.onTxStatus.connHandle = p_conn->connHandle;
    evtPara.eventField.onTxStatus.status = p_conn->trsState;
    if (bleTrspsProcess != NULL)
    {
        bleTrspsProcess(&evtPara);
    }

    return 0;
}


/**
 * @brief Process the received value for the RX characteristic.
 *
 * @param p_conn    Pointer to the connection list.
 * @param writeType The type of write operation.
 * @param length    The length of the data written.
 * @param p_value   Pointer to the value written.
 */
static void ble_trsps_RxValue(BLE_TRSPS_ConnList_T *p_conn, uint8_t writeType, uint16_t length, uint8_t *p_value)
{
    ble_trsps_RcvData(p_conn, writeType, length, p_value);
}


/**
 * @brief Process the control value written to the control point.
 *
 * @param p_conn    Pointer to the connection list.
 * @param length    The length of the data written.
 * @param p_value   Pointer to the control value written.
 */
static void ble_trsps_CtrlValue(BLE_TRSPS_ConnList_T *p_conn, uint16_t length, uint8_t *p_value)
{
    BLE_TRSPS_Event_T evtPara;

    (void)memset((uint8_t *) &evtPara, 0, sizeof(evtPara));
    
    switch (p_value[0])
    {
        case BLE_TRSPS_CBFC_OPCODE_SERVER_ENABLED:
        {
            p_conn->cbfcEnable |= BLE_TRSPS_CBFC_RX_ENABLED;
            p_conn->peerCredit = BLE_TRSPS_INIT_CREDIT;
            ble_trsps_ServerReturnCredit(p_conn);
            
            if (bleTrspsProcess != NULL)
            {
                evtPara.eventId=BLE_TRSPS_EVT_CBFC_ENABLED;
                evtPara.eventField.onCbfcEnabled.connHandle = p_conn->connHandle;
                bleTrspsProcess(&evtPara);
            }
        }
        break;

        case BLE_TRSPS_CBFC_OPCODE_GIVE_CREDIT:
        {
            p_conn->cbfcEnable |= BLE_TRSPS_CBFC_TX_ENABLED;
            p_conn->localCredit += p_value[1];
            
            if (bleTrspsProcess != NULL)
            {
                evtPara.eventId = BLE_TRSPS_EVT_CBFC_CREDIT;
                evtPara.eventField.onCbfcEnabled.connHandle = p_conn->connHandle;
                bleTrspsProcess(&evtPara);
            }
        }
        break;
        
        default:
        {
            if ((p_value[0] >= BLE_TRSPS_VENDOR_OPCODE_MIN) && (bleTrspsProcess!= NULL))
            {
                evtPara.eventId = BLE_TRSPS_EVT_VENDOR_CMD;
                evtPara.eventField.onVendorCmd.connHandle = p_conn->connHandle;
                evtPara.eventField.onVendorCmd.length = length;
                evtPara.eventField.onVendorCmd.p_payLoad = p_value;
                
                bleTrspsProcess(&evtPara);
            }
        }
        break;
    }
}


/**
 * @brief Handle the write request to the Control CCCD.
 *
 * @param p_conn    Pointer to the connection list.
 * @param p_value   Pointer to the value to be written to the CCCD.
 * @retval uint8_t  Returns 0 on success, ATT_ERR_APPLICATION_ERROR on failure.
 */
static uint8_t ble_trsps_CtrlCccd(BLE_TRSPS_ConnList_T *p_conn, uint8_t *p_value)
{
    uint16_t cccd;
    BLE_TRSPS_Event_T evtPara;


    (void)memset((uint8_t *) &evtPara, 0, sizeof(evtPara));
    BUF_LE_TO_U16(&cccd, p_value);
    
    if ((cccd != 0U) && (cccd != BLE_TRSPS_CCCD_NOTIFY))
    {
        return ATT_ERR_APPLICATION_ERROR;
    }

    if (bleTrspsProcess!= NULL)
    {
        evtPara.eventId = BLE_TRSPS_EVT_CTRL_STATUS;
        evtPara.eventField.onCtrlStatus.connHandle = p_conn->connHandle;
        if (cccd == BLE_TRSPS_CCCD_NOTIFY)
        {
            evtPara.eventField.onCtrlStatus.status = BLE_TRSPS_STATUS_CTRL_OPENED;
        }
        else
        {
            evtPara.eventField.onCtrlStatus.status = BLE_TRSPS_STATUS_CTRL_DISABLED;
        }
    
        bleTrspsProcess(&evtPara);
    }
    
    return 0;
}


/**
 * @brief Process the GATT server write requests for the BLE Transparent Service.
 *
 * @param p_event Pointer to the GATT event structure.
 */
static void ble_trsps_GattsWriteProcess(GATT_Event_T *p_event)
{
    uint8_t error = 0;
    BLE_TRSPS_ConnList_T *p_conn = NULL;
    uint16_t status;
    GATTS_SendWriteRespParams_T *p_trsRespParams = NULL;
    GATTS_SendErrRespParams_T *p_trsErrParams = NULL;

    if ((p_event->eventField.onWrite.attrHandle <= (uint16_t)TRS_START_HDL) ||
        (p_event->eventField.onWrite.attrHandle > (uint16_t)TRS_END_HDL))
    {
        /* Not BLE Trs characteristic. */
        return;
    }

    p_conn = ble_trsps_GetConnListByHandle(p_event->eventField.onWrite.connHandle);
        
    if (p_conn == NULL)
    {
        error = ATT_ERR_APPLICATION_ERROR;
    }

    if (p_event->eventField.onWrite.writeType == ATT_PREPARE_WRITE_REQ)
    {
        error = ATT_ERR_REQUEST_NOT_SUPPORT;
    }

    if (error==0U)
    {
        switch(p_event->eventField.onWrite.attrHandle)
        {
            case (uint16_t)TRS_HDL_CCCD_TX:
            {
                error=ble_trsps_TxCccd(p_conn, p_event->eventField.onWrite.writeValue);
            }
            break;

            case (uint16_t)TRS_HDL_CHARVAL_RX:
            {
                ble_trsps_RxValue(p_conn, p_event->eventField.onWrite.writeType,
                                  p_event->eventField.onWrite.writeDataLength,
                                  p_event->eventField.onWrite.writeValue);
            }
            break;

            case (uint16_t)TRS_HDL_CHARVAL_CTRL:
            {
                ble_trsps_CtrlValue(p_conn, p_event->eventField.onWrite.writeDataLength, 
                    p_event->eventField.onWrite.writeValue);
            }
            break;

            case (uint16_t)TRS_HDL_CCCD_CTRL:
            {
                error=ble_trsps_CtrlCccd(p_conn, p_event->eventField.onWrite.writeValue);
            }
            break;

            default:
            {
                //Do nothing
            }
            break;
        }
    }

    if ((p_event->eventField.onWrite.writeType == ATT_WRITE_REQ)
    || (p_event->eventField.onWrite.writeType == ATT_PREPARE_WRITE_REQ))
    {
        if (p_conn->p_retryData != NULL)
        {
            BLE_TRSPS_Event_T evtPara;
            evtPara.eventId = BLE_TRSPS_EVT_ERR_UNSPECIFIED;
            if (bleTrspsProcess != NULL)
            {
                bleTrspsProcess(&evtPara);
            }
            return;
        }
        if (error==0U)
        {
            p_conn->p_retryData = OSAL_Malloc(sizeof(GATTS_SendWriteRespParams_T));
            if (p_conn->p_retryData == NULL)
            {
                BLE_TRSPS_Event_T evtPara;
                evtPara.eventId = BLE_TRSPS_EVT_ERR_NO_MEM;
                if (bleTrspsProcess != NULL)
                {
                    bleTrspsProcess(&evtPara);
                }
                return;
            }
            p_trsRespParams = (GATTS_SendWriteRespParams_T *)p_conn->p_retryData;
            p_trsRespParams->responseType = ATT_WRITE_RSP;
            p_conn->retryType = BLE_TRSPS_RETRY_TYPE_RESP;
            status = GATTS_SendWriteResponse(p_event->eventField.onWrite.connHandle, p_trsRespParams);
            if (status == MBA_RES_SUCCESS)
            {
                ble_trsps_FreeRetryData(p_conn);
            }
        }
        else
        {
            p_conn->p_retryData = OSAL_Malloc(sizeof(GATTS_SendErrRespParams_T));
            if (p_conn->p_retryData == NULL)
            {
                BLE_TRSPS_Event_T evtPara;
                evtPara.eventId = BLE_TRSPS_EVT_ERR_NO_MEM;
                if (bleTrspsProcess != NULL)
                {
                    bleTrspsProcess(&evtPara);
                }
                return;
            }
            p_trsErrParams = (GATTS_SendErrRespParams_T *)p_conn->p_retryData;
            p_trsErrParams->reqOpcode = p_event->eventField.onWrite.writeType;
            p_trsErrParams->attrHandle = p_event->eventField.onWrite.attrHandle;
            p_trsErrParams->errorCode = error;
            p_conn->retryType = BLE_TRSPS_RETRY_TYPE_ERR;
            status = GATTS_SendErrorResponse(p_event->eventField.onWrite.connHandle, p_trsErrParams);
            if (status == MBA_RES_SUCCESS)
            {
                ble_trsps_FreeRetryData(p_conn);
            }
        }
    }
}


/**
 * @brief Process GATT events for the BLE Transparent Service.
 * 
 * @param p_event Pointer to the GATT event structure containing details about the event.
 */
static void ble_trsps_GattEventProcess(GATT_Event_T *p_event)
{
    BLE_TRSPS_ConnList_T *p_conn = NULL;


    switch (p_event->eventId)
    {
        case ATT_EVT_TIMEOUT:
        {
        }
        break;

        case ATT_EVT_UPDATE_MTU:
        {
            p_conn = ble_trsps_GetConnListByHandle(p_event->eventField.onUpdateMTU.connHandle);
            if (p_conn == NULL)
            {
                break;
            }
            p_conn->attMtu = p_event->eventField.onUpdateMTU.exchangedMTU;
        }
        break;

        case GATTS_EVT_WRITE:
        {
            ble_trsps_GattsWriteProcess(p_event);
        }
        break;

        default:
        {
            //Do nothing
        }
        break;
    }
}


/**
 * @brief Process GAP events for the BLE Transparent Service.
 * 
 * @param p_event Pointer to the BLE GAP event structure containing details about the event.
 */
static void ble_trsps_GapEventProcess(BLE_GAP_Event_T *p_event)
{
    switch(p_event->eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            if ((p_event->eventField.evtConnect.status == GAP_STATUS_SUCCESS))
            {
                BLE_TRSPS_ConnList_T    *p_conn;
                
                p_conn=ble_trsps_GetFreeConnList();
                if(p_conn==NULL)
                {
                    BLE_TRSPS_Event_T evtPara;
                    evtPara.eventId = BLE_TRSPS_EVT_ERR_UNSPECIFIED;
                    if (bleTrspsProcess != NULL)
                    {
                        bleTrspsProcess(&evtPara);
                    }
                    return;
                }

                p_conn->connHandle=p_event->eventField.evtConnect.connHandle;
            }
        }
        break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
            BLE_TRSPS_ConnList_T    *p_conn;

            p_conn=ble_trsps_GetConnListByHandle(p_event->eventField.evtDisconnect.connHandle);

            if (p_conn != NULL)
            {
                // Flush all queued data.
                while (p_conn->inputQueue.usedNum > 0U)
                {
                    if (p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet != NULL)
                    {
                        OSAL_Free(p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet);
                        p_conn->inputQueue.packetList[p_conn->inputQueue.readIndex].p_packet = NULL;
                    }

                    p_conn->inputQueue.readIndex++;
                    if (p_conn->inputQueue.readIndex >= BLE_TRSPS_INIT_CREDIT)
                    {
                        p_conn->inputQueue.readIndex = 0;
                    }

                    p_conn->inputQueue.usedNum --;
                }
                // Free retry data
                if (p_conn->p_retryData != NULL)
                {
                    ble_trsps_FreeRetryData(p_conn);
                }
                ble_trsps_InitConnList(p_conn);
            }
        }
        break;

        case BLE_GAP_EVT_TX_BUF_AVAILABLE:
        {
            ble_trsps_ProcessQueuedTask(p_event->eventField.evtTxBufAvailable.connHandle);
        }
        break;
       
        default:
        {
            //Do nothing
        }
        break;
    }
}


/**
 * @brief Handles BLE_Stack events.
 * This function should be called by the application when BLE stack events occur.
 *
 * @param[in] p_stackEvent        					Pointer to the BLE stack event data.
 *
*/
void BLE_TRSPS_BleEventHandler(STACK_Event_T *p_stackEvent)
{
    switch (p_stackEvent->groupId)
    {
        case STACK_GRP_BLE_GAP:
        {
            ble_trsps_GapEventProcess((BLE_GAP_Event_T *)p_stackEvent->p_event);
        }
        break;

        case STACK_GRP_GATT:
        {
            ble_trsps_GattEventProcess((GATT_Event_T *)p_stackEvent->p_event);
        }
        break;

        default:
        {
            //Do nothing
        }
        break;
    }
}

