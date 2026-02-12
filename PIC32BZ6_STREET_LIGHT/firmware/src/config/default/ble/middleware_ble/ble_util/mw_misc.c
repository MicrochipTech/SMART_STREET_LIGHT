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
  Middleware Misc Source File

  Company:
    Microchip Technology Inc.

  File Name:
    mw_misc.c

  Summary:
    Implements miscellaneous utility functions for application use.

  Description:
    This source file contains a collection of utility functions that provide
    various miscellaneous tools to support application development.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "osal/osal_freertos.h"
#include "mba_error_defs.h"
#include "byte_stream.h"
#include "mw_aes.h"
#include "mw_misc.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define MW_AD_TYPE_ENC_DATA           (0x31U)

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

static void mw_misc_GenEncAdvDataRand(uint8_t *p_rand) 
{
    int32_t r;
    int32_t rQueue = 0;
    int32_t rBitCount = 0;
    uint8_t i;

    for (i = 0; i < 5; i++) {
        r = 0;
        if (rBitCount < 8) {
            int32_t need = 8 - rBitCount;
            r = rQueue << need;
            rQueue = rand();
            r ^= rQueue;  
            rQueue >>= need;
            rBitCount = 31 - need;
        } else {
            r = rQueue;
            rQueue >>= 8;
            rBitCount -= 8;
        }
        p_rand[i] = r;
    }
}


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
 */
uint16_t MW_MISC_EncryptAdvData(MW_MISC_EncAdvData_T *p_encAdvData)
{
    MW_AES_Ctx_T ctx;
    uint8_t nonce[13], key[16], randomizer[5];
    uint8_t aad = 0xEA;
    uint8_t *p_buf;

    p_buf = p_encAdvData->p_advData + *p_encAdvData->p_advDataLen;
    
    U8_TO_STREAM(&p_buf, p_encAdvData->payloadLen + 10);
    U8_TO_STREAM(&p_buf, MW_AD_TYPE_ENC_DATA);

    mw_misc_GenEncAdvDataRand(randomizer);
    VARIABLE_COPY_TO_STREAM(&p_buf, randomizer, 5);

    BUF_TO_VARIABLE(key, p_encAdvData->p_key, sizeof(key));

    memcpy(nonce, randomizer, 5);
    memcpy(&nonce[5], p_encAdvData->p_iv, 8);

    if(MW_AES_CcmEncryptInit(&ctx, key, nonce, sizeof(nonce), 4, &aad, 1, p_encAdvData->payloadLen) == MBA_RES_SUCCESS)
    {
        if (MW_AES_AesCcmEncrypt(&ctx, p_encAdvData->payloadLen, p_encAdvData->p_payload, p_buf, p_buf + p_encAdvData->payloadLen) == MBA_RES_SUCCESS)
        {
            *p_encAdvData->p_advDataLen += (11 + p_encAdvData->payloadLen);

            return MBA_RES_SUCCESS;
        }
    }

    return MBA_RES_FAIL;
}


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
uint16_t MW_MISC_DecryptAdvData(MW_MISC_DecAdvData_T *p_decAdvData)
{
    MW_AES_Ctx_T ctx;
    uint8_t nonce[13], key[16];
    uint8_t aad = 0xEA;
    uint8_t *p_encPayload, *p_buf, adType, len;

    p_buf = p_decAdvData->p_advData;
    p_encPayload = NULL;

    while((p_buf + 2) < (p_decAdvData->p_advData + p_decAdvData->advDataLen))
    {
        STREAM_TO_U8(&len, &p_buf);
        STREAM_TO_U8(&adType, &p_buf);

        if (adType == MW_AD_TYPE_ENC_DATA)
        {
            /* copy randomizer to nonce */
            STREAM_COPY_TO_VARIABLE(nonce, &p_buf, 5);
            *p_decAdvData->p_payloadLen  = (len - 10);
            p_encPayload = p_buf;

            break;
        }
        else
        {
            p_buf += (len-1);
        }
    }

    if (p_encPayload == NULL)
    {
        return MBA_RES_FAIL;
    }

    BUF_TO_VARIABLE(key, p_decAdvData->p_key, sizeof(key));
    
    memcpy(&nonce[5], p_decAdvData->p_iv, 8);

    if(MW_AES_CcmDecryptInit(&ctx, key, nonce, sizeof(nonce), 4, &aad, 1, *p_decAdvData->p_payloadLen) == MBA_RES_SUCCESS)
    {
        if (MW_AES_AesCcmDecrypt(&ctx, *p_decAdvData->p_payloadLen, p_encPayload, p_encPayload + *p_decAdvData->p_payloadLen, p_decAdvData->p_payload) == MBA_RES_SUCCESS)
        {
            return MBA_RES_SUCCESS;
        }
    }

    return MBA_RES_FAIL;
}

