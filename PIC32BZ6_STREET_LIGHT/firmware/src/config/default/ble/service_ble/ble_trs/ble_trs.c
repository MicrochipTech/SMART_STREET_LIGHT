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
  BLE Transparent Service Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_trs.c

  Summary:
    Implements the BLE Transparent Service functions used by the application.

  Description:
    his source file provides the implementation for the BLE Transparent Service
    functions that are available to the application user. It facilitates the
    communication between the BLE stack and the application layer, enabling
    transparent data transfer over BLE.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include <stdint.h>
#include "mba_error_defs.h"
#include "gatt.h"
#include "ble_util/byte_stream.h"
#include "ble_trs.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define BLE_TRS_MIN_KEY_SIZE                                 0x10                       // The minimum key size for attribute permissions (TRS).
#define TRS_MAX_MTU_LEN                                      BLE_ATT_MAX_MTU_LEN        // Maximum Transmission Unit length for TRS.

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
/* Transparent Service Declaration */
static uint8_t s_svcUuidTrs[] = {UUID_MCHP_PROPRIETARY_SERVICE_16};
static const uint16_t s_svcUuidTrsLen = (uint16_t)sizeof (s_svcUuidTrs);

/* Transparent Tx Characteristic Declaration */
static uint8_t s_charTransTx[] = {(ATT_PROP_WRITE_REQ | ATT_PROP_NOTIFY), UINT16_TO_BYTES(TRS_HDL_CHARVAL_TX), UUID_MCHP_TRANS_TX_16};
static const uint16_t s_charTransTxLen = (uint16_t)sizeof (s_charTransTx);

/* Transparent Tx Characteristic Value */
static uint8_t s_chUuidTransTx[] = {UUID_MCHP_TRANS_TX_16};
static uint8_t s_transTxVal[1] = {0};
static uint16_t s_transTxValLen = 1;

/* Transparent Tx Client Characteristic Configuration Descriptor */
static uint8_t s_descCccTransTx[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t s_descCccTransTxLen = (uint16_t)sizeof (s_descCccTransTx);

/* Transparent Rx Characteristic Declaration */
static uint8_t s_charTransRx[] = {(ATT_PROP_WRITE_REQ | ATT_PROP_WRITE_CMD), UINT16_TO_BYTES(TRS_HDL_CHARVAL_RX), UUID_MCHP_TRANS_RX_16};
static const uint16_t s_charTransRxLen = (uint16_t)sizeof (s_charTransRx);

/* Transparent Rx Characteristic Value */
static uint8_t s_chUuidTransRx[] = {UUID_MCHP_TRANS_RX_16};
static uint8_t s_transRxVal[1] = {0};
static uint16_t s_transRxValLen = 1;

/* Transparent Ctrl Point Characteristic Declaration */
static uint8_t s_charTransCtrl[] = {(ATT_PROP_WRITE_CMD | ATT_PROP_WRITE_REQ | ATT_PROP_NOTIFY), UINT16_TO_BYTES(TRS_HDL_CHARVAL_CTRL), UUID_MCHP_TRANS_CTRL_16};
static const uint16_t s_charTransCtrlLen = (uint16_t)sizeof (s_charTransCtrl);

/* Transparent Ctrl Point Characteristic Value */
static uint8_t s_chUuidTransCtrl[] = {UUID_MCHP_TRANS_CTRL_16};
static uint8_t s_transCtrlVal[1] = {0};
static uint16_t s_transCtrlValLen = 1;

/* Transparent Ctrl Point Client Characteristic Configuration Descriptor */
static uint8_t s_descCccTransCtrl[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t s_descCccTransCtrlLen = (uint16_t)sizeof (s_descCccTransCtrl);

/* Attribute list for Transparent service */
static GATTS_Attribute_T s_trsList[] = {
    /* Service Declaration */
    {
        (uint8_t *) g_gattUuidPrimSvc,
        (uint8_t *) s_svcUuidTrs,
        (uint16_t *) & s_svcUuidTrsLen,
        (uint16_t)sizeof (s_svcUuidTrs),
        0,
        PERMISSION_READ
    },
    /* Characteristic Declaration */
    {
        (uint8_t *) g_gattUuidChar,
        (uint8_t *) s_charTransTx,
        (uint16_t *) & s_charTransTxLen,
        (uint16_t)sizeof (s_charTransTx),
        0,
        PERMISSION_READ
    },

    /* Characteristic Value */
    {
        (uint8_t *) s_chUuidTransTx,
        (uint8_t *) s_transTxVal,
        (uint16_t *) & s_transTxValLen,
        TRS_MAX_MTU_LEN,
        (SETTING_MANUAL_WRITE_RSP | SETTING_UUID_16 | SETTING_VARIABLE_LEN),
        PERMISSION_WRITE
    },
    /* Client Characteristic Configuration Descriptor */
    {
        (uint8_t *) g_descUuidCcc,
        (uint8_t *) s_descCccTransTx,
        (uint16_t *) & s_descCccTransTxLen,
        (uint16_t)sizeof (s_descCccTransTx),
        (SETTING_MANUAL_WRITE_RSP | SETTING_CCCD),
        (PERMISSION_READ | PERMISSION_WRITE)
    },
    /* Characteristic Declaration */
    {
        (uint8_t *) g_gattUuidChar,
        (uint8_t *) s_charTransRx,
        (uint16_t *) & s_charTransRxLen,
        (uint16_t)sizeof (s_charTransRx),
        0,
        PERMISSION_READ
    },
    /* Characteristic Value */
    {
        (uint8_t *) s_chUuidTransRx,
        (uint8_t *) s_transRxVal,
        (uint16_t *) & s_transRxValLen,
        TRS_MAX_MTU_LEN,
        (SETTING_MANUAL_WRITE_RSP | SETTING_UUID_16 | SETTING_VARIABLE_LEN),
        PERMISSION_WRITE
    },
    /* Characteristic Declaration */
    {
        (uint8_t *) g_gattUuidChar,
        (uint8_t *) s_charTransCtrl,
        (uint16_t *) & s_charTransCtrlLen,
        (uint16_t)sizeof (s_charTransCtrl),
        0,
        PERMISSION_READ
    },
    /* Characteristic Value */
    {
        (uint8_t *) s_chUuidTransCtrl,
        (uint8_t *) s_transCtrlVal,
        (uint16_t *) & s_transCtrlValLen,
        TRS_MAX_MTU_LEN,
        (SETTING_MANUAL_WRITE_RSP | SETTING_UUID_16 | SETTING_VARIABLE_LEN),
        PERMISSION_WRITE
    },
    /* Client Characteristic Configuration Descriptor */
    {
        (uint8_t *) g_descUuidCcc,
        (uint8_t *) s_descCccTransCtrl,
        (uint16_t *) & s_descCccTransCtrlLen,
        (uint16_t)sizeof (s_descCccTransCtrl),
        (SETTING_MANUAL_WRITE_RSP | SETTING_CCCD),
        (PERMISSION_READ | PERMISSION_WRITE)
    }
};

/* CCCD settings for the Transparent Service characteristics. */
static const GATTS_CccdSetting_T s_trsCccdSetting[] =
{
    {(uint16_t)TRS_HDL_CCCD_TX, (NOTIFICATION)},
    {(uint16_t)TRS_HDL_CCCD_CTRL, (NOTIFICATION)}
};

/* Transparent Service structure */
static GATTS_Service_T s_svcTrs = 
{
    NULL,
    (GATTS_Attribute_T *) s_trsList,
    (GATTS_CccdSetting_T *)s_trsCccdSetting,
    (uint16_t)TRS_START_HDL,
    (uint16_t)TRS_END_HDL,
    2
};

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Adds the BLE Transparent Service to the GATT server.
 *
 * This function adds the BLE Transparent Service to the BLE stack's GATT server,
 * enabling the service to be discovered and accessed by remote BLE devices.
 * 
 * @retval MBA_RES_SUCCESS                    The BLE Transparent service was successfully added.
 * @retval MBA_RES_NO_RESOURCE                Insufficient resource to add the BLE Transparent service.
 */
uint16_t BLE_TRS_Add(void) 
{
    return GATTS_AddService(&s_svcTrs, (uint8_t)((uint16_t)TRS_END_HDL - (uint16_t)TRS_START_HDL + 1U));
}


/**
 * @brief Configures the permissions for a characteristic within the transparent service.
 *
 * This function sets the access permissions for a given characteristic's attribute handle within the transparent service.
 * It should be called after initializing the transparent service with BLE_TRS_Add().
 *
 * @param[in] attrHdl                         Transparent service attribute handle.
 *                                            Refer to @ref BLE_TRS_AttributeHandle_T for possible values.
 * @param[in] permissions                     Attribute permissions to be set. Refer to @ref GATT_ATTRIBUTE_PERMISSIONS for possible values.
 *
 * @retval MBA_RES_SUCCESS                    The permissions were successfully configured.
 * @retval MBA_RES_INVALID_PARA               The provided attribute handle does not belong to the transparent service.
 *
 */
uint16_t BLE_TRS_PermissionConfig(uint16_t attrHdl, uint8_t permissions)
{
    if (attrHdl >= (uint16_t)TRS_HDL_SVC && attrHdl <= (uint16_t)TRS_HDL_CCCD_CTRL)
    {
        s_trsList[attrHdl - (uint16_t)TRS_START_HDL].permissions |= permissions;

        return MBA_RES_SUCCESS;
    }
    else
    {
        return MBA_RES_INVALID_PARA;
    }
}
