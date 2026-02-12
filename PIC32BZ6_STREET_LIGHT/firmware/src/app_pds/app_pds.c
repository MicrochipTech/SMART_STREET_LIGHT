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
  Application PDS Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_lora_pds.c

  Summary:
    This file contains the Application PDS implementation for this project.

  Description:
    This file contains the Application PDS implementation for this project.
 *******************************************************************************/
#include <stdio.h>

#include "app.h"
#include "app_ble.h"
#include "app_pds.h"
#include "app_lora.h"
#include "app_lora_wlr089.h"
#include "system/console/sys_console.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
static bool pdsDefaultLoraWanModuleEnable = true;
static bool pdsDefaultBleLogEnable = false;

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************
// declare default IDs and Key
char sDEVEUI[DEVEUI_ARRAY_SIZE]   = "0000000000000000";
char sJOINEUI[JOINEUI_ARRAY_SIZE] = "0000000000000000";
char sAPPKEY[APPKEY_ARRAY_SIZE]   = "00000000000000000000000000000000";

// declare PDS items
PDS_DECLARE_ITEM(PDS_BLE_BLELOG_ENABLE_ITEM_ID,  sizeof(appBleData.bBleLogEnable),  &appBleData.bBleLogEnable,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_MODULE_LORAWAN_ENABLE_ITEM_ID,  sizeof(loraData.bModuleEnable),  &loraData.bModuleEnable,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_MSG_TIMEOUT_ITEM_ID,  sizeof(loraData.msgTimeout),  &loraData.msgTimeout,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_MSG_NUM_CNF_ITEM_ID, sizeof(loraData.msgNumCnf), &loraData.msgNumCnf, NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_MSG_NUM_UNCNF_ITEM_ID,  sizeof(loraData.msgNumUncnf),  &loraData.msgNumUncnf,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_DEVEUI_ITEM_ID,  sizeof(sDEVEUI),  &sDEVEUI[0],  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_JOINEUI_ITEM_ID, sizeof(sJOINEUI), &sJOINEUI[0], NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LORA_APPKEY_ITEM_ID,  sizeof(sAPPKEY),  &sAPPKEY[0],  NULL, NO_ITEM_FLAGS);

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

#if 0
static int APP_PDS_Bin2Hex(char* destBuf, char* sourceBuf, size_t bufLen)
{
    if( destBuf && sourceBuf )
    {
        static char tmpBuf[130];
        int rv;

        for (int i = 0; i < bufLen; i++)
        {
            tmpBuf[i * 2 + 0] = "0123456789abcdef"[sourceBuf[i] >> 4];
            tmpBuf[i * 2 + 1] = "0123456789abcdef"[sourceBuf[i] & 0x0F];
            tmpBuf[i] = sourceBuf[i];
        }
        tmpBuf[20 * 2] = 0; // Add terminating null

        rv = snprintf(sourceBuf, bufLen, "%s", tmpBuf);

        if (0 < rv && rv < bufLen)
        {
            sourceBuf[rv] = 0;
            return 0;
        }
    }
    return -1;
}
#endif

static int APP_PDS_IsHexData(char* destBuf, char* sourceBuf, size_t bufLen)
{
    // TODO AJ: bufLen size check and/or limiter needed, e.g. bufLen>0 && bufLen<100?
    if( destBuf && sourceBuf )
    {
        int cnt = 0;
        for(; cnt < bufLen; cnt++)
        {
            // check for hex characters only
            if( !( (sourceBuf[cnt] >= '0' && sourceBuf[cnt] <= '9') ||
                   (sourceBuf[cnt] >= 'a' && sourceBuf[cnt] <= 'f') ||
                   (sourceBuf[cnt] >= 'A' && sourceBuf[cnt] <= 'F') ) )
            {
                return -1;
            }

            // convert a possible lowercase letter to uppercase
            //destBuf[cnt] = (char)toupper(sourceBuf[cnt]);
            destBuf[cnt] = (char)tolower(sourceBuf[cnt]);
        }

        return 0;
    }
    return -1;
}


bool APP_PDS_Set_BleLog()
{
    bool res = PDS_Store(PDS_BLE_BLELOG_ENABLE_ITEM_ID);

    if( res == true )
        SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" stored BLELOG_ENABLE: %s\r\n", (appBleData.bBleLogEnable == false ? "off" : "on"));
    else
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to store BLELOG_ENABLE: %s\r\n", (appBleData.bBleLogEnable == false ? "off" : "on"));

    return res;
}

bool APP_PDS_Get_BleLog()
{
    bool ret = false;
    if( PDS_IsAbleToRestore(PDS_BLE_BLELOG_ENABLE_ITEM_ID) )
    {
        if( PDS_Restore(PDS_BLE_BLELOG_ENABLE_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored BLELOG_ENABLE configuration: %s\r\n", (appBleData.bBleLogEnable == false ? "off" : "on"));
            ret = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore BLELOG_ENABLE configuration, using default: %s\r\n", (pdsDefaultBleLogEnable == false ? "off" : "on"));
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore BLELOG_ENABLE configuration, using default: %s\r\n", (pdsDefaultBleLogEnable == false ? "off" : "on"));
    }

    if( ret == false )
    {
        appBleData.bBleLogEnable = pdsDefaultBleLogEnable;
        APP_PDS_Set_BleLog();
    }

    return ret;
}

bool APP_PDS_Set_LoraWanModuleEnable()
{
    bool res = PDS_Store(PDS_MODULE_LORAWAN_ENABLE_ITEM_ID);

    if( res == true )
        SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" stored LORAWAN_MODULE_ENABLE: %s\r\n", (loraData.bModuleEnable == false ? "off" : "on"));
    else
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to store LORAWAN_MODULE_ENABLE: %s\r\n", (loraData.bModuleEnable == false ? "off" : "on"));

    return res;
}

bool APP_PDS_Get_LoraWanModuleEnable()
{
    bool ret = false;
    if( PDS_IsAbleToRestore(PDS_MODULE_LORAWAN_ENABLE_ITEM_ID) )
    {
        if( PDS_Restore(PDS_MODULE_LORAWAN_ENABLE_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored LORAWAN_MODULE_ENABLE: %s\r\n", (loraData.bModuleEnable == false ? "off" : "on"));
            ret = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore LORAWAN_MODULE_ENABLE, using default: %s\r\n", (pdsDefaultLoraWanModuleEnable == false ? "off" : "on"));
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore LORAWAN_MODULE_ENABLE, using default: %s\r\n", (pdsDefaultLoraWanModuleEnable == false ? "off" : "on"));
    }

    if( ret == false )
    {
        loraData.bModuleEnable = pdsDefaultLoraWanModuleEnable;
        APP_PDS_Set_LoraWanModuleEnable();
    }

    return ret;
}

bool APP_PDS_Set_LoraUplinkConfiguration(char* pConfig)
{
    bool ret = false;

    if( pConfig != NULL )
    {
        char timeout[4];
        char numCnfMsg[4];
        char numUncnfMsg[4];

        int numParams = sscanf(pConfig, "%s %s %s", timeout, numUncnfMsg, numCnfMsg);

        if( numParams == 3 )
        {
            uint8_t pdsRet = 0;
            // check the incoming configuration data
            // message timeout value
            uint8_t tmp = atoi(timeout);
            if( (tmp >= LORA_MSG_MIN_TIMEOUT) && (tmp <= LORA_MSG_MAX_TIMEOUT) )
            {
                loraData.msgTimeout = tmp;
                pdsRet = PDS_Store(PDS_LORA_MSG_TIMEOUT_ITEM_ID);
            }
            else
                SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Invalid]"TERM_RESET" 'PauseSec' value, must be between %d and %d\r\n", LORA_MSG_MIN_TIMEOUT, LORA_MSG_MAX_TIMEOUT);

            // number of unconfirmed messages
            tmp = atoi(numUncnfMsg);
            if( (tmp >= 0) && (tmp <= LORA_MSG_MAX_NUM_UNCNF) )
            {
                loraData.msgNumUncnf = tmp;
                pdsRet += PDS_Store(PDS_LORA_MSG_NUM_UNCNF_ITEM_ID);
            }
            else
                SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Invalid]"TERM_RESET" 'UncnfMsg' message number, must be between 0 and %d\r\n", LORA_MSG_MAX_NUM_UNCNF);

            // number of confirmed messages
            tmp = atoi(numCnfMsg);
            if( (tmp >= 0) && (tmp <= LORA_MSG_MAX_NUM_CNF) )
            {
                loraData.msgNumCnf = tmp;
                pdsRet += PDS_Store(PDS_LORA_MSG_NUM_CNF_ITEM_ID);
            }
            else
                SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Invalid]"TERM_RESET" 'CnfMsg' message number, must be between 0 and %d\r\n", LORA_MSG_MAX_NUM_CNF);

            ret = pdsRet == 3 ? true : false;
        }
    }
    return ret;
}

bool APP_PDS_Get_LoraUplinkConfiguration()
{
    uint8_t ret = 0;
    bool pdsRet = false;

    if( PDS_IsAbleToRestore(PDS_LORA_MSG_TIMEOUT_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_MSG_TIMEOUT_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored 'PauseSec' uplink configuration: %ds\r\n", loraData.msgTimeout);
            pdsRet = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore 'PauseSec' uplink configuration, using minimum value: %ds\r\n", LORA_MSG_MIN_TIMEOUT);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore 'PauseSec' uplink configuration, using minimum value: %ds\r\n", LORA_MSG_MIN_TIMEOUT);
    }

    if( pdsRet == false )
    {
        loraData.msgTimeout = LORA_MSG_MIN_TIMEOUT;
        PDS_Store(PDS_LORA_MSG_TIMEOUT_ITEM_ID);
        ret += 1;
    }

    pdsRet = false;
    if( PDS_IsAbleToRestore(PDS_LORA_MSG_NUM_UNCNF_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_MSG_NUM_UNCNF_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored 'UncnfMsg' uplink configuration: %d\r\n", loraData.msgNumUncnf);
            pdsRet = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore 'UncnfMsg' uplink configuration, using minimum value: %d\r\n", LORA_MSG_MIN_NUM_UNCNF);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore 'UncnfMsg' uplink configuration, using minimum value: %d\r\n", LORA_MSG_MIN_NUM_UNCNF);
    }

    if( pdsRet == false )
    {
        loraData.msgNumUncnf = LORA_MSG_MIN_NUM_UNCNF;
        PDS_Store(PDS_LORA_MSG_NUM_UNCNF_ITEM_ID);
        ret += 1;
    }

    pdsRet = false;
    if( PDS_IsAbleToRestore(PDS_LORA_MSG_NUM_CNF_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_MSG_NUM_CNF_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored 'CnfMsg' uplink configuration: %d\r\n", loraData.msgNumCnf);
            pdsRet = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore 'CnfMsg' uplink configuration, using minimum value: %d\r\n", LORA_MSG_MIN_NUM_CNF);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore 'CnfMsg' uplink configuration, using minimum value: %d\r\n", LORA_MSG_MIN_NUM_CNF);
    }

    if( pdsRet == false )
    {
        loraData.msgNumCnf = LORA_MSG_MIN_NUM_CNF;
        PDS_Store(PDS_LORA_MSG_NUM_CNF_ITEM_ID);
        ret += 1;
    }

    return ret == 0 ? true : false;
}

int8_t APP_PDS_Init_IDs(void)
{
    int8_t ret = 0;

    if( APP_PDS_Get_LoraDevEui(NULL) == false )
    {
        ret = 1;
    }

    if( APP_PDS_Get_LoraJoinEui(NULL) == false )
    {
        ret += 2;
    }

    if( APP_PDS_Get_LoraAppKey(NULL) == false )
    {
        ret += 4;
    }
    return ret;
}

int8_t APP_PDS_Set_LoraDevEui(char* pDevEui)
{
    int ret = -2;
    uint8_t len = strlen(pDevEui);

    // empty DEVEUI ("") value passed, use default sDEVEUI value
    if( len == 0 )
        return ret;

    if( (pDevEui != NULL) && (len == DEVEUI_SIZE) )
    {
        char tmp[16];
        ret = APP_PDS_IsHexData(&tmp[0], pDevEui, DEVEUI_SIZE);

        if( ret == 0 )
        {
            //memcpy(&sDEVEUI[0], tmp, DEVEUI_SIZE);
            //PDS_Store(PDS_LORA_DEVEUI_ITEM_ID);
            SYS_CONSOLE_PRINT("[PDS] "TERM_GREEN"[Valid]"TERM_RESET" DEVEUI:  %s\r\n"TERM_RESET, pDevEui);
        }
    }

    if( ret == -2 )
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Error]"TERM_RESET" DEVEUI:  size error, must be 16 characters\r\n"TERM_RESET);
    else if( ret == -1 )
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Error]"TERM_RESET" DEVEUI:  only hex characters allowed\r\n"TERM_RESET);

    return ret;
}

bool APP_PDS_Get_LoraDevEui(char* pBuf)
{
    bool ret = false;
    if( PDS_IsAbleToRestore(PDS_LORA_DEVEUI_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_DEVEUI_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored DEVEUI: %s\r\n", sDEVEUI);
            ret = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore DEVEUI, using default: %s\r\n", sDEVEUI);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore DEVEUI, using default: %s\r\n", sDEVEUI);
    }

    if( pBuf != NULL )
        memcpy(pBuf, sDEVEUI, sizeof(sDEVEUI));

    return ret;
}

int8_t APP_PDS_Set_LoraJoinEui(char* pJoinEui)
{
    int ret = -2;
    uint8_t len = strlen(pJoinEui);

    // empty JOINEUI ("") value passed, use default sJOINEUI value
    if( len == 0 )
        return ret;

    if( (pJoinEui != NULL) && (len == JOINEUI_SIZE) )
    {
        char tmp[16];
        ret = APP_PDS_IsHexData(&tmp[0], pJoinEui, JOINEUI_SIZE);

        if( ret == 0 )
        {
            //memcpy(&sJOINEUI[0], tmp, JOINEUI_SIZE);
            //PDS_Store(PDS_LORA_JOINEUI_ITEM_ID);
            SYS_CONSOLE_PRINT("[PDS] "TERM_GREEN"[Valid]"TERM_RESET" JOINEUI: %s\r\n"TERM_RESET, pJoinEui);
        }
    }

    if( ret == -2 )
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Error]"TERM_RESET" JOINEUI: size error, must be 16 characters\r\n"TERM_RESET);
    else if( ret == -1 )
        SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Error]"TERM_RESET" JOINEUI: only hex characters allowed\r\n"TERM_RESET);

    return ret;
}

bool APP_PDS_Get_LoraJoinEui(char* pBuf)
{
    bool ret = false;
    if( PDS_IsAbleToRestore(PDS_LORA_JOINEUI_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_JOINEUI_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored JOINEUI: %s\r\n", sJOINEUI);
            ret = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore JOINEUI, using default: %s\r\n", sJOINEUI);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore JOINEUI, using default: %s\r\n", sJOINEUI);
    }

    if( pBuf != NULL )
        memcpy(pBuf, sJOINEUI, sizeof(sJOINEUI));

    return ret;
}

int8_t APP_PDS_Set_LoraAppKey(char* pAppKey)
{
    int ret = -2;
    uint8_t len = strlen(pAppKey);

    // empty APPKEY ("") value passed, use default sAPPKEY value
    if( len == 0 )
        return ret;

    if( (pAppKey != NULL) && (len == APPKEY_SIZE) )
    {
        char tmp[32];
        ret = APP_PDS_IsHexData(&tmp[0], pAppKey, APPKEY_SIZE);

        if( (ret == 0) && ( strcmp(sAPPKEY, tmp) != 0 ) )
        {
            memcpy(&sAPPKEY[0], tmp, APPKEY_SIZE);
            PDS_Store(PDS_LORA_APPKEY_ITEM_ID);
            SYS_CONSOLE_PRINT("[PDS] "TERM_GREEN"[Valid]"TERM_RESET" APPKEY:  %s\r\n", sAPPKEY);
        }
    }

    if( ret == -2 )
        SYS_CONSOLE_MESSAGE("[PDS] "TERM_RED"[Error]"TERM_RESET" APPKEY:  size error, must be 32 characters\r\n");
    else if( ret == -1 )
        SYS_CONSOLE_MESSAGE("[PDS] "TERM_RED"[Error]"TERM_RESET" APPKEY:  only hex characters allowed\r\n");

    return ret;
}

bool APP_PDS_Get_LoraAppKey(char* pBuf)
{
    bool ret = false;
    if( PDS_IsAbleToRestore(PDS_LORA_APPKEY_ITEM_ID) )
    {
        if( PDS_Restore(PDS_LORA_APPKEY_ITEM_ID) )
        {
            SYS_CONSOLE_PRINT_PDS("[PDS] "TERM_GREEN"[Successfully]"TERM_RESET" restored APPKEY: %s\r\n", sAPPKEY);
            ret = true;
        }
        else
            SYS_CONSOLE_PRINT("[PDS] "TERM_RED"[Failed]"TERM_RESET" to restore APPKEY, using default: %s\r\n", sAPPKEY);
    }
    else
    {
        SYS_CONSOLE_PRINT("[PDS] "TERM_YELLOW"[Unable]"TERM_RESET" to restore APPKEY, using default: %s\r\n", sAPPKEY);
    }

    if( pBuf != NULL )
        memcpy(pBuf, sAPPKEY, sizeof(sAPPKEY));

    return ret;
}