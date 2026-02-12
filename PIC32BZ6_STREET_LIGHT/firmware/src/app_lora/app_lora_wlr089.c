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
  Application WLR089 Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_lora_wlr089.c

  Summary:
    This file contains the WLR089 application implementation for this project.

  Description:
    This file contains the WLR089 application implementation for this project.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include <stdio.h>

#include "app_trps.h"
#include "Sensors/inc/rgb_led.h"
#include "app_timer/app_timer.h"
#include "peripheral/eic/plib_eic.h"
#include "system/console/sys_console.h"
#include "app.h"

#include "app_ble_conn_handler.h"
#include "app_ble_sensor.h"
#include "app_error_defs.h"
#include "app_lora_wlr089.h"
#include "app_pds/app_pds.h"

#include "driver/pds/include/pds.h"
#include "peripheral/gpio/plib_gpio.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************
static int APP_LORA_WLR089_Rsp_SysReset(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_GetDevEui(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_GetJoinEui(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_GetStatus_Join(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_GetStatus_Join_AutoConnect(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_JoinOtaa(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Rsp_SendData(parserCmdInfo_t* paramList);
static int APP_LORA_WLR089_Status_Join(parserCmdInfo_t* paramList);

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
static volatile int8_t txMsgCnt = LORA_PERIODIC_MSG_CNT_UNCNF_MSG;
static volatile int8_t txMsgErrCnt = 0;
static volatile int8_t txMsgMaxErrCnt = 0;
static volatile appLoraMsgConfig_t *pMsgConfig = NULL;

#define SysReset  "sys reset\r\n"
#define SysResetCmdSize (sizeof(SysReset) - 1)
static loraCmdEntry_t maParserLoraSysResetCmd =
{
    SysReset, SysResetCmdSize, APP_LORA_WLR089_Rsp_SysReset, 104, 0
};

#define GetDevEui  "mac get deveui\r\n"
#define GetDevEuiCmdSize (sizeof(GetDevEui) - 1)
static loraCmdEntry_t maParserLoraGetDevEuiCmd =
{
    GetDevEui, GetDevEuiCmdSize, APP_LORA_WLR089_Rsp_GetDevEui, 18, 0
};

#define SetDevEuiTemplate  "mac set deveui %s\r\n"
static char SetDevEui[33] = SetDevEuiTemplate;
#define SetDevEuiCmdSize (sizeof(SetDevEui))
static loraCmdEntry_t maParserLoraSetDevEuiCmd =
{
    &SetDevEui[0], SetDevEuiCmdSize, NULL, 4, 0
};

#define GetJoinEui "mac get joineui\r\n"
#define GetJoinEuiCmdSize (sizeof(GetJoinEui) - 1)
static loraCmdEntry_t maParserLoraGetJoinEuiCmd =
{
    GetJoinEui, GetJoinEuiCmdSize, APP_LORA_WLR089_Rsp_GetJoinEui, 18, 0
};

#define SetJoinEuiTemplate "mac set joineui %s\r\n"
static char SetJoinEui[34] = SetJoinEuiTemplate;
#define SetJoinEuiCmdSize (sizeof(SetJoinEui))
static loraCmdEntry_t maParserLoraSetJoinEuiCmd =
{
    &SetJoinEui[0], SetJoinEuiCmdSize, NULL, 4, 0
};

#define SetAppKeyTemplate "mac set appkey %s\r\n"
static char SetAppKey[49]  = SetAppKeyTemplate;
#define SetAppKeyCmdSize (sizeof(SetAppKey))
static loraCmdEntry_t maParserLoraSetAppKeyCmd =
{
    &SetAppKey[0], SetAppKeyCmdSize, NULL, 4, 0
};

#define GetAppKey "get appkey\r\n"
#define GetAppKeyCmdSize (sizeof(GetAppKey))
static loraCmdEntry_t maParserLoraGetAppKeyCmd =
{
    GetAppKey, GetAppKeyCmdSize, APP_LORA_WLR089_Get_AppKey, 0, 0
};

#define MacReset  "mac reset 868\r\n"
#define MacResetCmdSize (sizeof(MacReset) - 1)
static loraCmdEntry_t maParserLoraMacResetCmd =
{
    MacReset, MacResetCmdSize, NULL, 4, 0
};

#define JoinNwOtaa "mac join otaa\r\n"
#define JoinNwOtaaCmdSize (sizeof(JoinNwOtaa) - 1)
static loraCmdEntry_t maParserLoraJoinOtaaCmd =
{
    JoinNwOtaa, JoinNwOtaaCmdSize, APP_LORA_WLR089_Rsp_JoinOtaa, 4, 0
};

#define GetStatus  "mac get status\r\n"
#define GetStatusCmdSize (sizeof(GetStatus) - 1)
static loraCmdEntry_t maParserLoraGetStatusCmd =
{
    // TODO AJ: callback function depends on status bits to check,
    //          therefore NULL is used as the default callback routine
    GetStatus, GetStatusCmdSize, NULL, 4, 0
};

#define SendDataTemplate "mac tx %s %s %s\r\n"
static char SendData[50] = SendDataTemplate;
#define SendDataCmdSize (sizeof(SendData))
static loraCmdEntry_t maParserLoraSendDataCmd =
{
    &SendData[0], SendDataCmdSize, APP_LORA_WLR089_Rsp_SendData, 4, 0
};

#define SetAdaptiveDataRateTemplate  "mac set adr %s\r\n"
static char SetAdaptiveDataRate[18] = SetAdaptiveDataRateTemplate;
#define SetAdaptiveDataRateCmdSize (sizeof(SetAdaptiveDataRate))
static loraCmdEntry_t maParserLoraSetAdaptiveDataRateCmd =
{
    &SetAdaptiveDataRate[0], SetAdaptiveDataRateCmdSize, NULL, 4, 0
};

#define GetAdaptiveDataRate  "mac get adr\r\n"
#define GetAdaptiveDataRateCmdSize (sizeof(GetAdaptiveDataRate) -1)
static loraCmdEntry_t maParserLoraGetAdaptiveDataRateCmd =
{
    GetAdaptiveDataRate, GetAdaptiveDataRateCmdSize, NULL, 4, 0
};

#define SetDataRateTemplate  "mac set dr %d\r\n"
static char SetDataRate[17] = SetDataRateTemplate;
#define SetDataRateCmdSize (sizeof(SetDataRate))
static loraCmdEntry_t maParserLoraSetDataRateCmd =
{
    &SetDataRate[0], SetDataRateCmdSize, NULL, 4, 0
};

#define GetDataRate  "mac get dr\r\n"
#define GetDataRateCmdSize (sizeof(GetDataRate) -1)
static loraCmdEntry_t maParserLoraGetDataRateCmd =
{
    GetDataRate, GetDataRateCmdSize, NULL, 3, 0
};

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
int APP_LORA_WLR089_Rsp_SysReset(parserCmdInfo_t* paramList)
{
    if( paramList->pParam1 != NULL )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        //vTaskDelay( 5 / portTICK_PERIOD_MS );
        SYS_CONSOLE_MESSAGE("APP_LORA_WLR089_Rsp_SysReset\r\n");
        //SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_SysReset %s\r\n", paramList->pParam1);
        #endif

        // stop the running timer
        APP_TIMER_StopTimer(APP_TIMER_LORA_ERROR);
        loraData.bIsResetDone = true;
    }
    return 0;
}

int APP_LORA_WLR089_Rsp_GetDevEui(parserCmdInfo_t* paramList)
{
    if( (paramList->pParam1 != NULL) && (paramList->pParam2 != NULL) )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        //vTaskDelay( 5 / portTICK_PERIOD_MS );
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetDevEui %s\r\n", paramList->pParam1);
        //SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetDevEui %s, %x\r\n", paramList->pParam1, paramList->pParam2);
        #endif

        int len = strcspn(paramList->pParam1, "\r");
        appCmdEntry_t *pWlr089Cmd = (appCmdEntry_t*)paramList->pParam2;

        if( pWlr089Cmd->pLoraCmd->pCmdRespDataBuf != NULL )
        {
            memcpy(pWlr089Cmd->pLoraCmd->pCmdRespDataBuf, paramList->pParam1, len);
            memcpy(&pWlr089Cmd->pLoraCmd->pCmdRespDataBuf[len], " \0", 2);
        }
    }
    return 0;
}

int APP_LORA_WLR089_Rsp_GetJoinEui(parserCmdInfo_t* paramList)
{
    if( (paramList->pParam1 != NULL) && (paramList->pParam2 != NULL) )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        //vTaskDelay( 5 / portTICK_PERIOD_MS );
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetJoinEui %s\r\n", paramList->pParam1);
        //SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetJoinEui %s, %x\r\n", paramList->pParam1, paramList->pParam2);
        #endif

        int len = strcspn(paramList->pParam1, "\r");
        appCmdEntry_t *pWlr089Cmd = (appCmdEntry_t*)paramList->pParam2;

        if( pWlr089Cmd->pLoraCmd->pCmdRespDataBuf != NULL )
        {
            memcpy(pWlr089Cmd->pLoraCmd->pCmdRespDataBuf, paramList->pParam1, len);
            memcpy(&pWlr089Cmd->pLoraCmd->pCmdRespDataBuf[len], " \0", 2);
        }
    }
    return 0;
}

int APP_LORA_WLR089_Rsp_JoinOtaa(parserCmdInfo_t* paramList)
{
    uint8_t numRet = 0;
    if( paramList->pParam1 != NULL )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        //vTaskDelay( 5 / portTICK_PERIOD_MS );
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_JoinOtaa %s\r\n", paramList->pParam1);
        #endif

        // use strtok to split response string, e.g. response consist of "ok\r\nno_free_ch\r\n"
        // pResp will point to the beginning of "ok\r"
        char *pResp = strtok(paramList->pParam1, "\n");

        // loop over response string, as long as pResp != NULL handle the response(s))
        while( pResp != NULL )
        {
            numRet = 0;
            // "ok" - join otaa command valid, forwarded to radio transceiver, awaiting denied or accepted
            if( strncmp(pResp, "ok", 2) == 0 )
            {
                numRet = 8;   // return minimum number of bytes to receive on pending response
            }
            else if( strncmp(pResp, "accepted", 8) == 0  )
            {
                // configure data transmission parameters, e.g. uplink config, pause time, etc.
                APP_LORA_Set_MessageTxConfiguration();

                bleSensorData.loraOnOffStatus = LORA_ON;
                SYS_CONSOLE_MESSAGE(TERM_BG_GREEN"LORAWAN CONNECTED"TERM_RESET"\r\n");

                LED_RED_Clear();
                LED_GREEN_Set();
                APP_TIMER_SetTimer(APP_TIMER_LORA_SEND_STATUS, loraData.msgTimeout*1000, true);
            }
            else
            {
                APP_TIMER_StopTimer(APP_TIMER_LORA_SEND_STATUS);
                bleSensorData.loraOnOffStatus = LORA_OFF;
                LED_RED_Set();
                LED_GREEN_Clear();
                SYS_CONSOLE_MESSAGE(TERM_BG_RED"LORAWAN DISCONNECTED"TERM_RESET"\r\n");
            }

            // use strtok + NULL pointer to get the possible next response string,
            // e.g. response consist of "ok\r\nno_free_ch\r\n", pResp will now point to "no_free_ch\r"
            // if the string does not include additional response data pResp will be a NULL pointer
            pResp = strtok(NULL, "\n");

        }   // while

    }
    b_button_debounce = false;
    return numRet;
}

int APP_LORA_WLR089_Rsp_SendData(parserCmdInfo_t* paramList)
{
    uint8_t numRet = 0;
    if( paramList->pParam1 != NULL )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        //vTaskDelay( 5 / portTICK_PERIOD_MS );
        //SYS_CONSOLE_MESSAGE("APP_LORA_WLR089_Rsp_SendData\r\n");
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_SendData %s\r\n", paramList->pParam1);
        //SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_SendData: %d, %d, %s\r\n", txMsgCnt, txMsgErrCnt, paramList->pParam1);
        #endif

        // use strtok to split response string, e.g. response consist of "ok\r\nno_free_ch\r\n"
        // pResp will point to the beginning of "ok\r"
        char *pResp = strtok(paramList->pParam1, "\n");

        // loop over response string, as long as pResp != NULL handle the response(s))
        while( pResp != NULL )
        {
            numRet = 0;

            if( strncmp(pResp, "ok", 2) == 0 )
            {
                numRet = 6;
            }
            else if( strncmp(pResp, "mac_tx", 6) == 0 )
            {
                SYS_CONSOLE_MESSAGE("[LORA] Uplink successful, no RX data\r\n");

                if( txMsgCnt == 0 )
                {
                    APP_TIMER_StopTimer(APP_TIMER_LORA_ERROR);
                    LED_BLUE_Clear();

                    //switch message configuration
                    if( (pMsgConfig == &loraData.msgConfig[0]) && (loraData.msgConfig[1].counter != 0) )
                        pMsgConfig = &loraData.msgConfig[1];
                    else if( loraData.msgConfig[0].counter != 0 )
                        pMsgConfig = &loraData.msgConfig[0];
                    else{}

                    txMsgCnt = pMsgConfig->counter;
                    txMsgErrCnt = 0;
                }
            }
            else if( strncmp(pResp, "mac_rx", 6) == 0 )
            {
                SYS_CONSOLE_MESSAGE("[LORA] Uplink successful with RX data\r\n");

                char str[20];
                int portno;
                uint32_t data;

                int strLen = strlen(pResp);

                if( (strLen == 18) || (strLen == 19) )
                {
                    // get the RX data
                    sscanf(pResp, "%s %02d %lx", str, &portno, &data);

                    // RGB off
                    if((data & 0x0F000000) == 0)
                    {
                        RGB_LED_Off();
                        bleSensorData.rgbOnOffStatus = LED_OFF;
                        SYS_CONSOLE_MESSAGE("[LORA] RGB off\r\n");
                    }
                    // RGB on
                    else if((data & 0x0F000000) == 0x01000000)
                    {
                        RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
                        bleSensorData.rgbOnOffStatus = LED_ON;

                        SYS_CONSOLE_MESSAGE("[LORA] RGB on\r\n");
                    }
                    // RGB color
                    else if((data & 0x0F000000) == 0x02000000)
                    {
                        bleSensorData.RGB_color.Hue        = (uint8_t)((data & 0x00FF0000) >> 16);
                        bleSensorData.RGB_color.Saturation = (uint8_t)((data & 0x0000FF00) >> 8);
                        bleSensorData.RGB_color.Value      = (uint8_t)((data & 0x000000FF) >> 0);

                        if(bleSensorData.rgbOnOffStatus == LED_ON)
                        {
                            RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
                        }
                        SYS_CONSOLE_PRINT("[LORA] RGB color 0x%02X\r\n", bleSensorData.RGB_color.Hue);
                    }
                    // DALI light off
                    else if((data & 0x0F000000) == 0x03000000)
                    {
#ifndef WITHOUT_DALI
                        APP_Msg_T appMsg;
                        uint8_t level = 0;
                        appMsg.msgId = APP_DALI_ACTION;
                        appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                        appMsg.msgData[1] = level;
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        SYS_CONSOLE_PRINT("[LORA] DALI light off\r\n");
#else
                        SYS_CONSOLE_PRINT("[LORA] no DALI light implementation!\r\n");
#endif /* WITHOUT_DALI */
                    }
                    // DALI light on
                    else if((data & 0x0F000000) == 0x04000000)
                    {
#ifndef WITHOUT_DALI
                        APP_Msg_T appMsg;
                        uint8_t level = 0;
                        if(app_lteData.daliLightIntensity == 0)
                        {
                            level = APP_DALI_DEFAULT_INTENSITY_LEVEL;
                        }
                        else
                        {
                            level = app_lteData.daliLightIntensity;
                        }
                        appMsg.msgId = APP_DALI_ACTION;
                        appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                        appMsg.msgData[1] = level;
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        SYS_CONSOLE_PRINT("[LORA] DALI light on\r\n");
#else
                        SYS_CONSOLE_PRINT("[LORA] no DALI light implementation!\r\n");
#endif /* WITHOUT_DALI */
                    }
                    // DALI light intensity
                    else if((data & 0x0F000000) == 0x05000000)
                    {
#ifndef WITHOUT_DALI
                        APP_Msg_T appMsg;
                        uint8_t level = (uint8_t)((data & 0x000000FF) >> 0);
                        if(   (app_lteData.daliLightStatus == 1)
                           && (level != app_lteData.daliLightIntensity))
                        {
                            appMsg.msgId = APP_DALI_ACTION;
                            appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                            appMsg.msgData[1] = level;
                            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        }
                        else
                        {
                            app_lteData.daliLightIntensity = level;
                        }
                        SYS_CONSOLE_PRINT("[LORA] DALI light change intensity to %d%\r\n", level);
#else
                        SYS_CONSOLE_PRINT("[LORA] no DALI light implementation!\r\n");
#endif /* WITHOUT_DALI */
                    }
                    // External GPIO off
                    else if((data & 0x0F000000) == 0x06000000)
                    {
                        EXT_GPIO_Clear();
                        SYS_CONSOLE_PRINT("[LORA] External GPIO off\r\n");
                    }
                    // External GPIO on
                    else if((data & 0x0F000000) == 0x07000000)
                    {
                        EXT_GPIO_Set();
                        SYS_CONSOLE_PRINT("[LORA] External GPIO on\r\n");
                    }
                }
                else
                {
                    SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Error]"TERM_RESET" payload must be 8 bytes, got %d\r\n", strLen-11);
                    numRet = 0;
                }
            }
            else
            {
                SYS_CONSOLE_MESSAGE("[LORA] "TERM_YELLOW"Transmission Error\r\n"TERM_RESET);

                txMsgErrCnt++;

                //switch message configuration
                if( (pMsgConfig == &loraData.msgConfig[0]) && (loraData.msgConfig[1].counter != 0) )
                    pMsgConfig = &loraData.msgConfig[1];
                else if( loraData.msgConfig[0].counter != 0 )
                    pMsgConfig = &loraData.msgConfig[0];
                else{}

                txMsgCnt = pMsgConfig->counter;
            }

            // use strtok + NULL pointer to get the possible next response string,
            // e.g. response consist of "ok\r\nno_free_ch\r\n", pResp will now point to "no_free_ch\r"
            // if the string does not include additional response data pResp will be a NULL pointer
            pResp = strtok(NULL, "\n");

        }   // while

        if( txMsgErrCnt == txMsgMaxErrCnt )
        {
            LED_BLUE_Set();
            APP_TIMER_StopTimer(APP_TIMER_LORA_SEND_STATUS);
            APP_TIMER_SetTimer(APP_TIMER_LORA_ERROR, APP_TIMER_500MS, true);
            txMsgErrCnt = 0;

            APP_Msg_T     appMsg;
            appCmdEntry_t cmdEntry;

            maParserLoraGetStatusCmd.pfCmdRespCb = APP_LORA_WLR089_Rsp_GetStatus_Join;
            cmdEntry.pLoraCmd = &maParserLoraGetStatusCmd;
            cmdEntry.nextLoraCmdInQueue = false;
            cmdEntry.printLoraCmdRespDataBuf = false;
            OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

            appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
            OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);

            SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Error]"TERM_RESET" max number of transmission sequence retries (%d) reached.\r\n", LORA_PERIODIC_MSG_CNT_ABORT_MSG);
        }
    }
    return numRet;
}

void APP_LORA_WLR089_Set_DevEui(char* pDevEui)
{
    if( APP_PDS_Set_LoraDevEui(pDevEui) == 0 )
    {
        appCmdEntry_t cmdEntry;

        snprintf( SetDevEui, 40, SetDevEuiTemplate, sDEVEUI );

        cmdEntry.pLoraCmd = &maParserLoraSetDevEuiCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.printLoraCmdRespDataBuf = false;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
    }
}

void APP_LORA_WLR089_Get_DevEui(char* pBuf)
{
    appCmdEntry_t cmdEntry;
    cmdEntry.pLoraCmd = &maParserLoraGetDevEuiCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_WLR089_Set_JoinEui(char* pJoinEui)
{
    if( APP_PDS_Set_LoraJoinEui(pJoinEui) == 0 )
    {
        appCmdEntry_t cmdEntry;

        snprintf( SetJoinEui, 40, SetJoinEuiTemplate, sJOINEUI );

        cmdEntry.pLoraCmd = &maParserLoraSetJoinEuiCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.printLoraCmdRespDataBuf = false;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
    }
}

void APP_LORA_WLR089_Get_JoinEui(char* pBuf)
{
    appCmdEntry_t cmdEntry;
    cmdEntry.pLoraCmd = &maParserLoraGetJoinEuiCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_WLR089_Set_AppKey(char* pAppKey)
{
    if( APP_PDS_Set_LoraAppKey(pAppKey) == 0 )
    {
        appCmdEntry_t cmdEntry;

        snprintf( SetAppKey, 50, SetAppKeyTemplate, sAPPKEY );

        cmdEntry.pLoraCmd = &maParserLoraSetAppKeyCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.printLoraCmdRespDataBuf = false;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
    }
}

int APP_LORA_WLR089_Get_AppKey(parserCmdInfo_t* paramList)
{
    if( paramList->pParam2 != NULL )
    {
        appCmdEntry_t *pWlr089Cmd = (appCmdEntry_t*)paramList->pParam2;
        // add string termination sign to buffer
        APP_PDS_Get_LoraAppKey(pWlr089Cmd->pLoraCmd->pCmdRespDataBuf);
        memcpy(&pWlr089Cmd->pLoraCmd->pCmdRespDataBuf[APPKEY_SIZE], "\0", 1);
    }
    return 0;
}

bool APP_LORA_WLR089_Set_Keys(char* pKeys)
{
    bool ret = false;
    if( pKeys != NULL )
    {
        char deveui[DEVEUI_ARRAY_SIZE];
        char joineui[JOINEUI_ARRAY_SIZE];
        char appkey[APPKEY_ARRAY_SIZE];

        //int numKeys = sscanf(pKeys, "%s,%s,%s", deveui, joineui, appkey); // not working
        //int numKeys = sscanf(pKeys, "%[^,],%[^,],%s", deveui, joineui, appkey);
        int numKeys = sscanf(pKeys, "%s %s %s", deveui, joineui, appkey);

        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_DEBUG
        // check for correct data extraction from input string
        vTaskDelay( 5 / portTICK_PERIOD_MS );
        SYS_CONSOLE_PRINT(">0: %d, %s\r\n", strlen(deveui), deveui);
        SYS_CONSOLE_PRINT(">0: %d, %s\r\n", strlen(joineui), joineui);
        SYS_CONSOLE_PRINT(">0: %d, %s\r\n", strlen(appkey), appkey);
        #endif

        if( numKeys == 3 )
        {
            // check for correct length and hexadecimal characters
            numKeys  = APP_PDS_Set_LoraDevEui(deveui);
            numKeys += APP_PDS_Set_LoraJoinEui(joineui);
            numKeys += APP_PDS_Set_LoraAppKey(appkey);

            if( numKeys == 0 )
            {
                snprintf( SetDevEui,  40, SetDevEuiTemplate,  deveui );
                snprintf( SetJoinEui, 40, SetJoinEuiTemplate, joineui );
                snprintf( SetAppKey,  50, SetAppKeyTemplate,  appkey );

                appCmdEntry_t cmdEntry;

                cmdEntry.pLoraCmd = &maParserLoraSetDevEuiCmd;
                cmdEntry.nextLoraCmdInQueue = true;
                cmdEntry.printLoraCmdRespDataBuf = false;
                OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

                cmdEntry.pLoraCmd = &maParserLoraSetJoinEuiCmd;
                cmdEntry.nextLoraCmdInQueue = true;
                OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

                cmdEntry.pLoraCmd = &maParserLoraSetAppKeyCmd;
                cmdEntry.nextLoraCmdInQueue = false;
                OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

                ret = true;
            }
        }
    }
    else
        SYS_CONSOLE_MESSAGE("[LORA] "TERM_RED"[ERROR]"TERM_RESET" input data read failed\r\n");

    return ret;
}

void APP_LORA_WLR089_Get_Keys(char* pBuf)
{
    if( pBuf != NULL )
    {
        appCmdEntry_t cmdEntry;

        // preparations to read the DEVEUI over UART
        cmdEntry.pLoraCmd = &maParserLoraGetDevEuiCmd;
        cmdEntry.nextLoraCmdInQueue = true;
        cmdEntry.pLoraCmd->pCmdRespDataBuf = &pBuf[0];
        cmdEntry.printLoraCmdRespDataBuf = true;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

        // preparations to read the JOINEUI over UART
        cmdEntry.pLoraCmd = &maParserLoraGetJoinEuiCmd;
        cmdEntry.nextLoraCmdInQueue = true;
        cmdEntry.pLoraCmd->pCmdRespDataBuf = &pBuf[DEVEUI_ARRAY_SIZE];
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

        // preparations to read the APPKEY from internal memory (no UART command available)
        cmdEntry.pLoraCmd = &maParserLoraGetAppKeyCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.pLoraCmd->pCmdRespDataBuf = &pBuf[DEVEUI_ARRAY_SIZE+JOINEUI_ARRAY_SIZE];
        cmdEntry.pLoraCmd->cmdInterface = eInternal;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
    }
}

void APP_LORA_WLR089_Set_AdaptiveDataRate(char* state)
{
    if( (strcmp(state, "on") == 0)  || (strcmp(state, "off") == 0) )
    {
        appCmdEntry_t cmdEntry;
        int cmdLen = snprintf( SetAdaptiveDataRate, SetAdaptiveDataRateCmdSize, SetAdaptiveDataRateTemplate, state );
        maParserLoraSetAdaptiveDataRateCmd.cmdSize = cmdLen;

        cmdEntry.pLoraCmd = &maParserLoraSetAdaptiveDataRateCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.printLoraCmdRespDataBuf = false;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
    }
    else
    {
        SYS_CONSOLE_PRINT(TERM_RED"ERROR invalid parameter value %s\r\n"TERM_RESET, state);
    }
}

void APP_LORA_WLR089_Get_AdaptiveDataRate(void)
{
    appCmdEntry_t cmdEntry;
    cmdEntry.pLoraCmd = &maParserLoraGetAdaptiveDataRateCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_WLR089_Set_DataRate(char *pDataRate)
{
    if( pDataRate != NULL )
    {
        uint8_t dr = atoi(pDataRate);

        // TODO AJ: currently no region specific data rate value check implemented
        if( (dr >= 0) && (dr <= 15) )
        {
            appCmdEntry_t cmdEntry;
            int cmdLen = snprintf( SetDataRate, SetDataRateCmdSize, SetDataRateTemplate, dr );
            maParserLoraSetDataRateCmd.cmdSize = cmdLen;

            cmdEntry.pLoraCmd = &maParserLoraSetDataRateCmd;
            cmdEntry.nextLoraCmdInQueue = false;
            cmdEntry.printLoraCmdRespDataBuf = false;
            OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
        }
        else
        {
            SYS_CONSOLE_PRINT(TERM_RED"ERROR invalid data rate value %d\r\n"TERM_RESET, dr);
        }
    }
}

void APP_LORA_WLR089_Get_DataRate(void)
{
    appCmdEntry_t cmdEntry;
    cmdEntry.pLoraCmd = &maParserLoraGetDataRateCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_WLR089_Get_Status(void)
{
    appCmdEntry_t cmdEntry;
    SYS_CONSOLE_PRINT("[LORA] Retrieving status ...\r\n");
    cmdEntry.pLoraCmd = &maParserLoraGetStatusCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

int8_t APP_LORA_WLR089_Init_IDs(uint8_t kindOfIDs)
{
    // PDS_InitItems(PDS_LORA_DEVEUI_ITEM_ID,PDS_LORA_APPKEY_ITEM_ID);
    int8_t ret = 0;
    if( (kindOfIDs & 0x01) != 0 )
    {
        //SYS_CONSOLE_PRINT(TERM_YELLOW"[DEVEUI]"TERM_RESET" storing default: %s\r\n", DEVEUI);
        ret = APP_PDS_Set_LoraDevEui(DEVEUI);
    }

    if( (kindOfIDs & 0x02) != 0 )
    {
        //SYS_CONSOLE_PRINT(TERM_YELLOW"[JOINEUI]"TERM_RESET" storing default: %s\r\n", JOINEUI);
        ret += APP_PDS_Set_LoraJoinEui(JOINEUI);
    }

    if( (kindOfIDs & 0x04) != 0 )
    {
        //SYS_CONSOLE_PRINT(TERM_YELLOW"[APPKEY]"TERM_RESET" storing default: %s\r\n", APPKEY);
        ret += APP_PDS_Set_LoraAppKey(APPKEY);
    }
    return ret;
}

void APP_LORA_WLR089_Reset()
{
    vTaskDelay( 10 / portTICK_PERIOD_MS );
    SYS_CONSOLE_MESSAGE("\r\n***********************************\r\n");
    SYS_CONSOLE_MESSAGE("******** Resetting WLR089 *********\r\n");
    SYS_CONSOLE_MESSAGE("***********************************\r\n\n");

    appCmdEntry_t cmdEntry;

    cmdEntry.pLoraCmd = &maParserLoraSysResetCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_WLR089_Init()
{
    vTaskDelay( 10 / portTICK_PERIOD_MS );
    SYS_CONSOLE_MESSAGE("\r\n***********************************\r\n");
    SYS_CONSOLE_MESSAGE("******* Initializing WLR089 *******\r\n");
    SYS_CONSOLE_MESSAGE("***********************************\r\n\n");

    appCmdEntry_t cmdEntry;

    cmdEntry.pLoraCmd = &maParserLoraMacResetCmd;
    cmdEntry.nextLoraCmdInQueue = true;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    snprintf( SetDevEui,  40, SetDevEuiTemplate,  sDEVEUI );
    snprintf( SetJoinEui, 40, SetJoinEuiTemplate, sJOINEUI );
    snprintf( SetAppKey,  50, SetAppKeyTemplate,  sAPPKEY );

    cmdEntry.pLoraCmd = &maParserLoraGetDevEuiCmd;
    cmdEntry.nextLoraCmdInQueue = true;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    cmdEntry.pLoraCmd = &maParserLoraGetJoinEuiCmd;
    cmdEntry.nextLoraCmdInQueue = true;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    int cmdLen = snprintf( SetDataRate, SetDataRateCmdSize, SetDataRateTemplate, 5 );
    maParserLoraSetDataRateCmd.cmdSize = cmdLen;
    cmdEntry.pLoraCmd = &maParserLoraSetDataRateCmd;
    cmdEntry.nextLoraCmdInQueue = true;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    cmdEntry.pLoraCmd = &maParserLoraGetDataRateCmd;
    cmdEntry.nextLoraCmdInQueue = true;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    cmdEntry.pLoraCmd = &maParserLoraGetAdaptiveDataRateCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

    loraData.msgConfig[0].msgType = LORA_DEFINE_UNCONFIRMED;
    loraData.msgConfig[1].msgType = LORA_DEFINE_CONFIRMED;
}

int APP_LORA_WLR089_Status_Join(parserCmdInfo_t* paramList)
{
    int ret = 0;
    if( (paramList->pParam1 != NULL) && (paramList->pParam2 != NULL) )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Status_Join %s\r\n", paramList->pParam1);
        #endif

        // check bit 0 'Join status': '0' - network not joined, '1' - network joined
        if( (strtol(paramList->pParam1, NULL, 16) & 0x1) == 1 )
        {
            bleSensorData.loraOnOffStatus = LORA_ON;
            SYS_CONSOLE_MESSAGE("[LORA] "TERM_GREEN"Still connected to the network\r\n"TERM_RESET);
            LED_RED_Clear();
            LED_GREEN_Set();
            ((appCmdEntry_t*)(paramList->pParam2))->pLoraCmd->flags = 1;
            ret = 1;
        }
        else
        {
            bleSensorData.loraOnOffStatus = LORA_OFF;
            LED_RED_Set();
            LED_GREEN_Clear();
            ((appCmdEntry_t*)(paramList->pParam2))->pLoraCmd->flags = 0;
        }
    }
    return ret;
}

int APP_LORA_WLR089_Rsp_GetStatus_Join_AutoConnect(parserCmdInfo_t* paramList)
{
    if( (paramList->pParam1 != NULL) && (paramList->pParam2 != NULL) )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetStatus_Join_AutoConnect %s\r\n", paramList->pParam1);
        #endif

        // stop possible running timer and clear LED in any way
        APP_TIMER_StopTimer(APP_TIMER_LORA_SEND_STATUS);
        APP_TIMER_StopTimer(APP_TIMER_LORA_ERROR);
        LED_BLUE_Clear();

        // return value: 'Join status': '0' - network not joined, '1' - network joined
        if( APP_LORA_WLR089_Status_Join(paramList) == 0 )
            SYS_CONSOLE_MESSAGE("[LORA] "TERM_YELLOW"Trying to join the network ...\r\n"TERM_RESET);

        appCmdEntry_t cmdEntry;

        cmdEntry.pLoraCmd = &maParserLoraJoinOtaaCmd;
        cmdEntry.nextLoraCmdInQueue = false;
        cmdEntry.printLoraCmdRespDataBuf = false;
        OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);

        APP_Msg_T appMsg;
        appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
        OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
    }
    return 0;
}

int APP_LORA_WLR089_Rsp_GetStatus_Join(parserCmdInfo_t* paramList)
{
    if( (paramList->pParam1 != NULL) && (paramList->pParam2 != NULL) )
    {
        #if LORA_MSG_VERBOSE_LEVEL == VERBOSE_LEVEL_FULL
        SYS_CONSOLE_PRINT("APP_LORA_WLR089_Rsp_GetStatus_Join %s\r\n", paramList->pParam1);
        #endif

        // stop possible running timer and clear LED in any way
        APP_TIMER_StopTimer(APP_TIMER_LORA_ERROR);
        LED_BLUE_Clear();

        // return value: 'Join status': '0' - network not joined, '1' - network joined
        if( APP_LORA_WLR089_Status_Join(paramList) == 1 )
        {
            // configure data transmission parameters, e.g. uplink config, pause time, etc.
            APP_LORA_Set_MessageTxConfiguration();

            uint16_t tmrState = APP_TIMER_IsTimerActive(APP_TIMER_LORA_SEND_STATUS);

            // start timer if not already running
            if( (tmrState == APP_RES_INVALID_PARA) || (tmrState == pdFALSE) )
                APP_TIMER_SetTimer(APP_TIMER_LORA_SEND_STATUS, loraData.msgTimeout*1000, true);
        }
        else
            SYS_CONSOLE_MESSAGE("[LORA] "TERM_RED"[Error]"TERM_RESET" Network not joined\r\n");
    }
    return 0;
}

void APP_LORA_Join_Handler(void)
{
    maParserLoraGetStatusCmd.pfCmdRespCb = APP_LORA_WLR089_Rsp_GetStatus_Join_AutoConnect;
    APP_LORA_WLR089_Get_Status();
}

void APP_LORA_Get_Status(void)
{
    maParserLoraGetStatusCmd.pfCmdRespCb = APP_LORA_WLR089_Rsp_GetStatus_Join;
    APP_LORA_WLR089_Get_Status();
}

void APP_LORA_TimerTrig_Handler(void)
{
    appCmdEntry_t cmdEntry;

    int cmdLen = sprintf(maParserLoraSendDataCmd.pCommand, "mac tx %s 1 %02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n", \
                            pMsgConfig->msgType, \
                            bleSensorData.rgbOnOffStatus, \
                            bleSensorData.RGB_color.Hue, \
                            bleSensorData.RGB_color.Saturation, \
                            bleSensorData.RGB_color.Value, \
                            bleSensorData.tempSens.msb, \
                            bleSensorData.tempSens.lsb, \
                            app_lteData.daliLightStatus, \
                            app_lteData.daliLightIntensity, \
                            (unsigned int)EXT_GPIO_Get() \
    );

    txMsgCnt--;

    if( txMsgCnt < 0 )
        txMsgCnt = 0;

    maParserLoraSendDataCmd.cmdSize = cmdLen;
    cmdEntry.pLoraCmd = &maParserLoraSendDataCmd;
    cmdEntry.nextLoraCmdInQueue = false;
    cmdEntry.printLoraCmdRespDataBuf = false;
    OSAL_QUEUE_Send(&loraData.wlr089CmdQueue, &cmdEntry, 0);
}

void APP_LORA_Set_MessageTxConfiguration(void)
{
    loraData.msgConfig[0].counter = loraData.msgNumUncnf;
    loraData.msgConfig[1].counter = loraData.msgNumCnf;

    if( loraData.msgNumUncnf == 0 )
        pMsgConfig = &loraData.msgConfig[1];
    else
        pMsgConfig = &loraData.msgConfig[0];

    txMsgCnt = pMsgConfig->counter;
    txMsgMaxErrCnt = (loraData.msgNumUncnf+loraData.msgNumCnf) * LORA_PERIODIC_MSG_CNT_ABORT_MSG;
}

