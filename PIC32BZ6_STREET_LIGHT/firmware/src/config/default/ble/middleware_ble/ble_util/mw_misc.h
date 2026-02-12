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
  Middleware Misc Header File

  Company:
    Microchip Technology Inc.

  File Name:
    mw_misc.h

  Summary:
    Interface for miscellaneous utility functions in the BLE middleware.

  Description:
    This header file declares various utility functions used within the
    BLE middleware.
 *******************************************************************************/

#ifndef MW_MISC_H
#define MW_MISC_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

/**
 * @addtogroup MW_MISC
 * @{
 * @brief Provides misc tool functions for BLE applications.
 * @note  This section declares the API for the misc component of the BLE middleware.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/**
 * @addtogroup MW_MISC_STRUCTS Structures
 * @{
 */

/** @brief Structure for handling the encryption of advertising data. */
typedef struct MW_MISC_EncAdvData_T
{
    uint8_t   *p_key;                             /**< Pointer to the 16-byte encryption key. */
    uint8_t   *p_iv;                              /**< Pointer to the 8-byte initialization vector (IV). */
    uint8_t   payloadLen;                         /**< The length of the unencrypted payload. */
    uint8_t   *p_payload;                         /**< Pointer to the unencrypted payload. */
    uint16_t  *p_advDataLen;                      /**< Pointer to the the advertising data length. */
    uint8_t   *p_advData;                         /**< Pointer to the advertising data. */
} MW_MISC_EncAdvData_T;


/** @brief Structure for handling the decryption of advertising data. */
typedef struct MW_MISC_DecAdvData_T
{
    uint8_t   *p_key;                             /**< Pointer to the 16-byte encryption key. */
    uint8_t   *p_iv;                              /**< Pointer to the 8-byte initialization vector (IV). */
    uint8_t   *p_payloadLen;                      /**< Pointer to the unencrypted payload length. */
    uint8_t   *p_payload;                         /**< Pointer to the unencrypted payload. */
    uint16_t  advDataLen;                         /**< The length of the advertising data. */
    uint8_t   *p_advData;                         /**< Pointer to the advertising data. */
} MW_MISC_DecAdvData_T;
/** @} */ //MW_MISC_STRUCTS


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/**
 * @addtogroup MW_MISC_FUNS Functions
 * @{ */

/**
 * @brief Encrypt advertising data.
 * @note This function encrypts p_payload, package it as an Encrypted Data data type, and 
 *        append the AD structure to p_advData. The length will increase by 11 bytes. The 
 *        application must ensure that p_advData has sufficient space for the appended data.
 *
 * @param[in] p_encAdvData         Pointer to the @ref MW_MISC_EncAdvData_T structure.
 *
 * @retval MBA_RES_SUCCESS         Encryption successfully.
 * @retval MBA_RES_FAIL            Encryption failed.
 * @retval MBA_RES_OOM             Internal memory allocation failure occurred.
 */
uint16_t MW_MISC_EncryptAdvData(MW_MISC_EncAdvData_T *p_encAdvData);


/**
 * @brief Decrypt advertising data.
 * @note This function search for the AD structure with Encrypted Data data type in the p_advData 
 *        and decrypts the data to p_payload.
 *
 * @param[in] p_encAdvData         Pointer to the @ref MW_MISC_DecAdvData_T structure.
 *
 * @retval MBA_RES_SUCCESS         Decryption successfully.
 * @retval MBA_RES_FAIL            Decryption failed.
 * @retval MBA_RES_OOM             Internal memory allocation failure occurred.
 */
uint16_t MW_MISC_DecryptAdvData(MW_MISC_DecAdvData_T *p_decAdvData);


/** @} */ //MW_AES_FUNS

/** @} */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif //MW_MISC_H