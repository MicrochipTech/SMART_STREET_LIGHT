/*******************************************************************************
 System Tasks File

  File Name:
    thread_demo.c

  Summary:
    This file contains source code necessary to thread demo application.

  Description:
    This file contains source code necessary to thread demo application.
    
  Remarks:
    
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
#include <string.h>
#include <stdio.h>
#include "definitions.h"

#include "configuration.h"
#include "openthread/dataset.h"
#include "openthread/dataset_ftd.h"
#include "openthread/instance.h"
#include "openthread/thread.h"

#include "utils/uart.h"
#include "timers.h"
#include "app_thread.h"
#include "openthread_stack_config.h"
#include "app_thread_udp.h"
#include "app_pds/app_pds.h"

#include <assert.h>
// *****************************************************************************
// *****************************************************************************

extern otInstance *instance;
otOperationalDataset aDataset;
otDeviceRole state = OT_DEVICE_ROLE_DISABLED;
SYS_CONSOLE_HANDLE appConsoleHandle;

bool ftdDataInitialized = false;
bool ftdModuleEnable;

PDS_DECLARE_ITEM(PDS_MODULE_FTD_ENABLE_ITEM_ID, sizeof(ftdModuleEnable), &ftdModuleEnable,  NULL, NO_ITEM_FLAGS);

static void printIpv6Address(void)
{
    const otNetifAddress *unicastAddrs = otIp6GetUnicastAddresses(instance);
    app_printf("[FTD] Unicast Address :\n");

    for (const otNetifAddress *addr = unicastAddrs; addr; addr = addr->mNext)
    {
        char string[OT_IP6_ADDRESS_STRING_SIZE];
        otIp6AddressToString(&(addr->mAddress), string, OT_IP6_ADDRESS_STRING_SIZE);
        app_printf("%s\n", string);
    }
}

void APP_FTD_Init()
{
    if(PDS_IsAbleToRestore(PDS_MODULE_FTD_ENABLE_ITEM_ID))
    {
        PDS_Restore(PDS_MODULE_FTD_ENABLE_ITEM_ID);
    }
    else
    {   // disable FTD module by default
        ftdModuleEnable = false;
        PDS_Store(PDS_MODULE_FTD_ENABLE_ITEM_ID);
    }

    if(ftdModuleEnable)
    {
        APP_FTD_Enable();
    }
}

void APP_FTD_Enable()
{
    if(!ftdDataInitialized)
    {
        threadAppinit();
        threadConfigNwParameters();
    }
    threadNwStart();
    ftdModuleEnable = true;
    PDS_Store(PDS_MODULE_FTD_ENABLE_ITEM_ID);
}

void APP_FTD_Disable()
{
    otThreadSetEnabled(instance, false);
    otIp6SetEnabled(instance, false);
    ftdModuleEnable = false;
    PDS_Store(PDS_MODULE_FTD_ENABLE_ITEM_ID);
}

void threadAppinit()
{
    appConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);
    
    otSetStateChangedCallback(instance, threadHandleStateChange, NULL);
}

void threadConfigNwParameters()
{
    otError err = OT_ERROR_NONE;
    
    memset(&aDataset, 0, sizeof(otOperationalDataset));
  
    aDataset.mChannel = CHANNEL;
    aDataset.mChannelMask = (otChannelMask)0x7fff800;
    aDataset.mPanId = PANID;
    
    uint8_t ex_panid[OT_EXT_PAN_ID_SIZE] = EXD_PANID;
    memcpy(aDataset.mExtendedPanId.m8, ex_panid, sizeof(aDataset.mExtendedPanId));
    
    uint8_t nw_key[OT_NETWORK_KEY_SIZE] = NW_KEY;
    memcpy(aDataset.mNetworkKey.m8, nw_key, sizeof(aDataset.mNetworkKey));
    
    static char nwk_name[] = NWK_NAME;
    uint8_t nwk_len = strlen(nwk_name);
    memcpy(aDataset.mNetworkName.m8, nwk_name, nwk_len);
   
    uint8_t ml_prefix[OT_MESH_LOCAL_PREFIX_SIZE] = ML_PREFIX;
    memcpy(aDataset.mMeshLocalPrefix.m8, ml_prefix, sizeof(aDataset.mMeshLocalPrefix));
    
    aDataset.mComponents.mIsChannelMaskPresent = true;
    aDataset.mComponents.mIsExtendedPanIdPresent = true;
    aDataset.mComponents.mIsMeshLocalPrefixPresent = true;
    aDataset.mComponents.mIsNetworkKeyPresent = true;
    aDataset.mComponents.mIsNetworkNamePresent = true;
    aDataset.mComponents.mIsPanIdPresent = true;
    aDataset.mComponents.mIsChannelPresent = true;
    
    err = otDatasetSetActive(instance, &aDataset);
    if (err != OT_ERROR_NONE) {
        app_printf("[FTD] Err: Dataset update failed\n");
    }
}

void threadNwStart()
{
    otIp6SetEnabled(instance, true);

    if(OT_ERROR_NONE == otThreadSetEnabled(instance, true))
    {
        printIpv6Address();
    }
}

void threadHandleStateChange(otChangedFlags aFlags, void *aContext)
{
    // Check if Device role is changed
    if(aFlags & ((uint32_t)OT_CHANGED_THREAD_ROLE))
    {
        // get state and check if state is child, yes, print attach successful.
        state = otThreadGetDeviceRole(instance);
        if(state == OT_DEVICE_ROLE_CHILD)
        {
            //app_printf("[FTD] Device State Child.\n");
        }
        else if(state == OT_DEVICE_ROLE_ROUTER)
        {
            app_printf("[FTD] Device State Router.\n");
#if OPENTHREAD_FTD
            if(!ftdDataInitialized)
            {            
                threadInitData();
                ftdDataInitialized = true;
            }
#endif            
        }
        else if(state == OT_DEVICE_ROLE_LEADER)
        {
            //app_printf("[FTD] Device State Leader.\n");
        }
        else if(state == OT_DEVICE_ROLE_DETACHED)
        {
            //app_printf("[FTD] Detached from NWk.\n");
        }
        else if(state == OT_DEVICE_ROLE_DISABLED)
        {
            //app_printf("[FTD] Device State Disabled.\n");
        }
    }
    // not handling other flags as of now.
}

void threadInitData()
{
    threadUdpOpen();
    threadUdpBind();
}

void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    // Do nothing!!!!
}

void otPlatUartSendDone(void)
{
    // Do nothing!!!!
}


int app_printf(const char *format, ...)
{
    uint16_t ret;

    char s[APP_TX_BUFFER_LENGTH];
    va_list ap;
    va_start(ap, format);
    ret = vsprintf(s, format, ap);
    va_end(ap);
    
    SYS_CONSOLE_Message(appConsoleHandle,(const char *)&s);
    return ret;

}
/*******************************************************************************
 End of File
 */

