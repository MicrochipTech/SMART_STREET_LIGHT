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

/**
 * @file
 *    This file implements the Crypto platform APIs.
 */

#include <openthread/platform/crypto.h>

#include <openthread-core-config.h>
#include <openthread/config.h>

#include "configuration.h"

#include <assert.h>
#include <utils/code_utils.h>

#include "driver/security/cryptosym/internal.h"
#include "driver/security/cryptosym/blkcipher_api.h"
#include "driver/security/cryptosym/keyref_api.h"
#include "driver/security/cryptosym/statuscodes.h"
#include "driver/security/cryptosym/hmac_api.h"
#include "driver/security/cryptosym/mac_api.h"
#include "driver/security/cryptosym/sha2_api.h"
#include "driver/security/cryptosym/hash_api.h"
#include "driver/security/api_table.h"

#define AES_BLOCK_SIZE   (16)

#if OPENTHREAD_BARCO_SHA256_C
struct crmmac hmacsha256Ctx;
struct crmhash shash256;
#endif

struct crmblkcipher blkciph;
struct crmkeyref keyref;


otError otPlatCryptoAesInit(otCryptoContext *aContext)
{
    OT_UNUSED_VARIABLE(aContext);

    return OT_ERROR_NONE;
}
otError otPlatCryptoAesSetKey(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    int ret = 0;

    OT_UNUSED_VARIABLE(aContext);

    otEXPECT(aKey->mKey != NULL);
    otEXPECT(aKey->mKeyLength == 16);
    
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    keyref = CRM_KEYREF_LOAD_MATERIAL(aKey->mKeyLength,(const char *)aKey->mKey);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();

    otEXPECT(ret == 0);

    return OT_ERROR_NONE;
    
exit:
    return OT_ERROR_FAILED;
}

otError otPlatCryptoAesEncrypt(otCryptoContext *aContext, const uint8_t *aInput, uint8_t *aOutput)
{
    int ret = -1;
	OT_UNUSED_VARIABLE(aContext);
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    
    // Starts AES ECB Encryption 
    ret = CRM_BLKCIPHER_CREATE_AESECB_ENC(&blkciph, &keyref);
        
    if(CRM_OK == ret)
        ret = CRM_BLKCIPHER_CRYPT(&blkciph, (const char *)aInput, AES_BLOCK_SIZE, (char *)aOutput);
    if(CRM_OK == ret)          
        ret = CRM_BLKCIPHER_RUN(&blkciph);
    if(CRM_OK == ret)          
        ret  = CRM_BLKCIPHER_WAIT(&blkciph);
    
    /* Disable Silex/BA457 Clock */    
    CRYPTO_CLK_DISABLE(); 
    
    otEXPECT(ret == 0);
    
    return OT_ERROR_NONE;

exit:
    return OT_ERROR_FAILED;
}

otError otPlatCryptoAesFree(otCryptoContext *aContext)
{
    OT_UNUSED_VARIABLE(aContext);
    return OT_ERROR_NONE;
}

#if !OPENTHREAD_RADIO
#if OPENTHREAD_BARCO_SHA256_C
// HMAC implementations
otError otPlatCryptoHmacSha256Init(otCryptoContext *aContext)
{
    return OT_ERROR_NONE;
}

otError otPlatCryptoHmacSha256Deinit(otCryptoContext *aContext)
{
    return OT_ERROR_NONE;
}

otError otPlatCryptoHmacSha256Start(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    struct crmkeyref keyref = CRM_KEYREF_LOAD_MATERIAL(aKey->mKeyLength, (const char *)aKey->mKey);
    //Prepares a HMAC SHA256 MAC operation
    CRM_MAC_CREATE_HMAC_SHA2_256(&hmacsha256Ctx, &keyref);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();

    return OT_ERROR_NONE;
}

otError otPlatCryptoHmacSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    //Feeds data to be used for MAC generation
    CRM_MAC_FEED(&hmacsha256Ctx, (const char *)aBuf, aBufLength);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();
    return OT_ERROR_NONE;
}

otError otPlatCryptoHmacSha256Finish(otCryptoContext *aContext, uint8_t *aBuf, size_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBufLength);

    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    //Starts MAC generation operation
    CRM_MAC_GENERATE(&hmacsha256Ctx,(char *)aBuf);
    CRM_MAC_WAIT(&hmacsha256Ctx);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();
    return OT_ERROR_NONE;
}

// SHA256 platform implementations
otError otPlatCryptoSha256Init(otCryptoContext *aContext)
{
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Deinit(otCryptoContext *aContext)
{
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Start(otCryptoContext *aContext)
{
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    //Prepares a SHA256 hash operation context
    CRM_HASH_CREATE_SHA256(&shash256, sizeof(shash256));
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    //Assign data to be hashed
    CRM_HASH_FEED(&shash256, (const char *)aBuf, aBufLength);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();
    return OT_ERROR_NONE;
}

otError otPlatCryptoSha256Finish(otCryptoContext *aContext, uint8_t *aHash, uint16_t aHashSize)
{
    OT_UNUSED_VARIABLE(aHashSize);

    /* Enable Silex/BA457 Clock */
    CRYPTO_CLK_ENABLE();
    //Starts the hashing operation
    CRM_HASH_DIGEST(&shash256, (char *)aHash);
    //Waits until the given hash operation has finished
    CRM_HASH_WAIT(&shash256);
    /* Disable Silex/BA457 Clock */
    CRYPTO_CLK_DISABLE();

    return OT_ERROR_NONE;
}
#endif
#endif
