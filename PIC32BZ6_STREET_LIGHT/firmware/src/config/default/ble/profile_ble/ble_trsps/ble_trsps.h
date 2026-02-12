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
  BLE Transparent Server Profile Header File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_trsps.h

  Summary:
    Interface definitions for the BLE Transparent Server Profile.

  Description:
    This header file provides function prototypes and constants for the
    implementation of the BLE Transparent Server Profile, enabling the
    application user to interact with the BLE stack for transparent data
    communication.
 *******************************************************************************/
#ifndef BLE_TRSPS_H
#define BLE_TRSPS_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "stack_mgr.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

/** 
 * @addtogroup BLE_PROFILE BLE Profile
 * @{
 */

/** 
 * @addtogroup BLE_TRSP BLE Transparent Profile
 * @{
 */

/**
 * @defgroup BLE_TRSPS Transparent Profile Server Role (TRSPS)
 * @brief Provides a interface for the BLE Transparent Profile Server.
 * @note This section provides the API interface for the Transparent Profile
 *          Server role, facilitating the setup and management of transparent
 *          data services over BLE.
 * @{
 */
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
/**
 * @addtogroup BLE_TRSPS_DEFINES Defines
 * @{
 */

/**
 * @defgroup BLE_TRSPS_MAX_CONN_NBR TRSPS maximum connection number
 * @brief Defines the maximum number of concurrent connections.
 * @{
 */
#define BLE_TRSPS_MAX_CONN_NBR                  BLE_GAP_MAX_LINK_NBR    /**< Maximum number of concurrent connections supported by the device. */
/** @} */

/**
 * @defgroup BLE_TRSPS_STATUS TRSPS status
 * @brief Defines the status of the BLE Transparent Service.
 * @{
 */
#define BLE_TRSPS_STATUS_CTRL_DISABLED          (0x00U)                 /**< Transparent Service Control characteristic CCCD is disabled. */
#define BLE_TRSPS_STATUS_CTRL_OPENED            (0x01U)                 /**< Transparent Service Control characteristic CCCD is enabled. */
#define BLE_TRSPS_STATUS_TX_DISABLED            (0x00U)                 /**< Transparent Service TX characteristic CCCD is disabled. */
#define BLE_TRSPS_STATUS_TX_OPENED              (0x01U)                 /**< Transparent Service TX characteristic CCCD is enabled. */
/** @} */

/** @} */ //BLE_TRSPS_DEFINES


/**
 * @addtogroup BLE_TRSPS_ENUMS Enumerations
 * @{
 */

/** @brief Enumeration of BLE Transparent Profile callback event types. */
typedef enum BLE_TRSPS_EventId_T
{
    BLE_TRSPS_EVT_CTRL_STATUS=0x01,                     /**< Control Channel status update event. See @ref BLE_TRSPS_EvtCtrlStatus_T for event details. */
    BLE_TRSPS_EVT_TX_STATUS,                            /**< Data Channel transmit status event. See @ref BLE_TRSPS_EvtTxStatus_T for event details. */
    BLE_TRSPS_EVT_CBFC_ENABLED,                         /**< Credit-based flow control enabled event. See @ref BLE_TRSPS_EvtCbfcEnabled_T for event details. */
    BLE_TRSPS_EVT_CBFC_CREDIT,                          /**< Credit-based flow control credit update event. See @ref BLE_TRSPS_EvtCbfcEnabled_T for event details. */
    BLE_TRSPS_EVT_RECEIVE_DATA,                         /**< Data Channel received data event. See @ref BLE_TRSPS_EvtReceiveData_T for event details. */
    BLE_TRSPS_EVT_VENDOR_CMD,                           /**< Vendor command received event. See @ref BLE_TRSPS_EvtVendorCmd_T for event details. */
    BLE_TRSPS_EVT_ERR_UNSPECIFIED,                      /**< Unspecified internal error event. */
    BLE_TRSPS_EVT_ERR_NO_MEM,                           /**< Insufficient heap memory error event. */
    BLE_TRSPS_EVT_END
}BLE_TRSPS_EventId_T;

/** @} */ //BLE_TRSPS_ENUMS

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/**
 * @addtogroup BLE_TRSPS_STRUCTS Structures
 * @{
 */

/** @brief Structure for the @ref BLE_TRSPS_EVT_CTRL_STATUS event. */
typedef struct BLE_TRSPS_EvtCtrlStatus_T
{
    uint16_t         connHandle;                              /**< Connection handle associated with this connection. */
    uint8_t          status;                                  /**< Status of the control channel. Refer to @ref BLE_TRSPS_STATUS for possible values. */
}   BLE_TRSPS_EvtCtrlStatus_T;


/** @brief Structure for the @ref BLE_TRSPS_EVT_TX_STATUS event. */
typedef struct BLE_TRSPS_EvtTxStatus_T
{
    uint16_t         connHandle;                              /**< Connection handle associated with this connection. */
    uint8_t          status;                                  /**< Status of the data channel transmission. Refer to @ref BLE_TRSPS_STATUS for possible values.*/
}   BLE_TRSPS_EvtTxStatus_T;


/** @brief Structure for the @ref BLE_TRSPS_EVT_CBFC_ENABLED event. */
typedef struct BLE_TRSPS_EvtCbfcEnabled_T
{
    uint16_t         connHandle;                              /**< Connection handle associated with this connection. */
}BLE_TRSPS_EvtCbfcEnabled_T;


/** @brief Structure for the @ref BLE_TRSPS_EVT_RECEIVE_DATA event. */
typedef struct BLE_TRSPS_EvtReceiveData_T
{
    uint16_t         connHandle;                              /**< Connection handle associated with this connection. */
}BLE_TRSPS_EvtReceiveData_T;


/** @brief Structure for the @ref BLE_TRSPS_EVT_VENDOR_CMD event. */
typedef struct BLE_TRSPS_EvtVendorCmd_T
{
    uint16_t         connHandle;                              /**< Connection handle associated with this connection. */
    uint16_t         length;																  /**< Length of the vendor command payload. */
    uint8_t          *p_payLoad;                              /**< Pointer to the vendor command payload. */
}BLE_TRSPS_EvtVendorCmd_T;


/** @brief Union of BLE Transparent profile server event types. */
typedef union
{
    BLE_TRSPS_EvtCtrlStatus_T        onCtrlStatus;            /**< Data for @ref BLE_TRSPS_EVT_CTRL_STATUS event. */
    BLE_TRSPS_EvtTxStatus_T          onTxStatus;              /**< Data for @ref BLE_TRSPS_EVT_TX_STATUS event. */
    BLE_TRSPS_EvtCbfcEnabled_T       onCbfcEnabled;           /**< Data for @ref BLE_TRSPS_EVT_CBFC_ENABLED event. */
    BLE_TRSPS_EvtReceiveData_T       onReceiveData;           /**< Data for @ref BLE_TRSPS_EVT_RECEIVE_DATA event. */
    BLE_TRSPS_EvtVendorCmd_T         onVendorCmd;             /**< Data for @ref BLE_TRSPS_EVT_VENDOR_CMD event. */
} BLE_TRSPS_EventField_T;


/** @brief BLE Transparent profile server callback event. */
typedef struct  BLE_TRSPS_Event_T
{
    BLE_TRSPS_EventId_T             eventId;                  /**< Identifier of the event. See @ref BLE_TRSPS_EventId_T. */
    BLE_TRSPS_EventField_T          eventField;               /**< Data associated with the event. See @ref BLE_TRSPS_EventField_T. */
} BLE_TRSPS_Event_T;


/** 
 * @brief Callback type for BLE Transparent profile server events.
 * 
 * @note This callback function is used to send events from the BLE Transparent profile server to the application.
 * 
 * @param p_event Pointer to the event structure containing details about the event.
 */
typedef void(*BLE_TRSPS_EventCb_T)(BLE_TRSPS_Event_T *p_event);

/** @} */ //BLE_TRSPS_STRUCTS

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/**
 * @addtogroup BLE_TRSPS_FUNS Functions
 * @{
 */

/**
 * @brief Registers a callback function for BLE Transparent Profile server events.
 *
 * @param[in] bleTranServHandler          	The client callback function to handle BLE Transparent Profile server events.
 */
void BLE_TRSPS_EventRegister(BLE_TRSPS_EventCb_T bleTranServHandler);


/**
 * @brief Initializes the BLE Transparent Profile Server.
 *
 * @retval MBA_RES_SUCCESS                 	Service successfully added to the service table.
 * @retval MBA_RES_FAIL                  		Failed to add the service to the service table.
 * @retval MBA_RES_OOM                  		Internal memory allocation failure.
 */
uint16_t BLE_TRSPS_Init(void);


/**
 * @brief Sends a vendor-specific command over BLE.
 *
 * @param[in] connHandle                    Connection handle associated with this connection.
 * @param[in] commandID                     The identifier of the vendor-specific command.
 * @param[in] commandLength                 The length of the command payload.
 * @param[in] p_commandPayload              Pointer to the command payload data.
 *
 * @retval MBA_RES_SUCCESS                  Command successfully sent.
 * @retval MBA_RES_FAIL                     Failed due to an invalid connection handle.
 * @retval MBA_RES_OOM                      Internal memory allocation failure.
 * @retval MBA_RES_INVALID_PARA             Invalid parameters; incorrect commandID, commandLength, or CCCD of TCP not enabled.
 */
uint16_t BLE_TRSPS_SendVendorCommand(uint16_t connHandle, uint8_t commandID, uint8_t commandLength, uint8_t *p_commandPayload);


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
uint16_t BLE_TRSPS_SendData(uint16_t connHandle, uint16_t len, uint8_t *p_data);


/**
 * @brief Retrieves the length of data queued for transmission.
 *
 * @param[in]    connHandle                 Connection handle associated with the queued data.
 * @param[out]   p_dataLength               Pointer to where the data length will be stored.
 */
void BLE_TRSPS_GetDataLength(uint16_t connHandle, uint16_t *p_dataLength);


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
uint16_t BLE_TRSPS_GetData(uint16_t connHandle, uint8_t *p_data);


/**
 * @brief Handles BLE_Stack events.
 * 
 * @note This function should be called by the application when BLE stack events occur.
 *
 * @param[in] p_stackEvent        					Pointer to the BLE stack event data.
 *
*/
void BLE_TRSPS_BleEventHandler(STACK_Event_T *p_stackEvent);

/** @} */ //BLE_TRSPS_FUNS

/** @} */

/** @} */

/** @} */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif//BLE_TRSPS_H