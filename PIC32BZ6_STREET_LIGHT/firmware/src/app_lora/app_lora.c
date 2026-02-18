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
  Application LORA Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble.c

  Summary:
    This file contains the Application LORA implementation for this project.

  Description:
    This file contains the Application LORA implementation for this project.
 *******************************************************************************/
#include <stdio.h>

#include "app.h"
#include "app_lora_wlr089.h"
#include "app_pds/app_pds.h"
#include "osal/osal_freertos_extend.h"
#include "app_ble.h"
#include "app_ble_handler.h"
#include "app_trsps_handler.h"
#include "definitions.h"
#include "app_ble_sensor.h"
#include "app_timer/app_timer.h"
#include "app_error_defs.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define LORA_UART_DATA_MAX          128

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************
APP_DATA_LORA loraData;
extern uint16_t conn_hdl;

static char loraUartDataBuff[APP_PRINT_BUFFER_SIZ] __attribute__((aligned(4)));
static char loraGpDataBuff[APP_PRINT_BUFFER_SIZ] __attribute__((aligned(4)));

static uint8_t loraUartRxBuf[LORA_UART_DATA_MAX];

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

/******************************************************************************
  Function:
    void APP_LORA_Uart_CB( uintptr_t context )
 * called from within interrupt

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Uart_CB(uintptr_t context)
{
    APP_Msg_T appMsg;

    if( loraUartRxBuf[context-1] == '\n' ) // check for fully received response indicated by "\r\n"
    {
        memcpy(&loraUartDataBuff[0], &loraUartRxBuf[0], context);
        memcpy(&loraUartDataBuff[context], "\0", 1);

        loraData.bUartGotResp = true;

        appMsg.msgId = APP_MSG_LORA_UART_CMD_CB;
        OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
    }
}

/******************************************************************************
  Function:
    void APP_LORA_Uart_CB_Bypass( uintptr_t context )
 * called from within interrupt

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Uart_CB_Bypass(uintptr_t context)
{
    APP_Msg_T appMsg;

    if( loraUartRxBuf[context-1] == '\n' ) // check for fully received response indicated by "\r\n"
    {
        memcpy(&loraGpDataBuff[0], &loraUartRxBuf[0], context);
        memcpy(&loraGpDataBuff[context], "\0", 1);

        SYS_CONSOLE_PRINT(TERM_CYAN"rsp: %s"TERM_RESET, &loraGpDataBuff[0]);

        if( appBleData.bBleLogEnable == true )
        {
            sprintf((char *)appMsg.msgData, "rsp: %.240s", &loraGpDataBuff[0]);
            // send BLE response
            appMsg.msgId = APP_MSG_TRS_BLE_LOG;
            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
        }
    }
}

/*******************************************************************************
  Function:
    void APP_TimerTrig_Handler( void )

  Remarks:
    See prototype in app.h.
 */
void APP_TimerTrig_Handler(void)
{
    LED_BLUE_Toggle();
}

/*******************************************************************************
  Function:
    void APP_LORA_Enable ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Enable(void)
{
    SERCOM1_USART_Enable();

    loraData.bModuleEnable = true;
    APP_PDS_Set_LoraWanModuleEnable();
    SYS_CONSOLE_PRINT("[LORA] Module enabled\r\n");

    // state depends on WRL board has already been initialized or not
    if( loraData.bIsInitialized == true )
        loraData.state = APP_STATE_SERVICE_TASKS;
    else
        loraData.state = APP_STATE_LORA_RESET;
}

/*******************************************************************************
  Function:
    void APP_LORA_Disable ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Disable(void)
{
    SERCOM1_USART_Disable();

    loraData.bModuleEnable = false;
    APP_PDS_Set_LoraWanModuleEnable();
    SYS_CONSOLE_PRINT("[LORA] Module disabled\r\n");
    loraData.state = APP_STATE_LORAWAN_MODULE_DISABLED;
}

/*******************************************************************************
  Function:
    void APP_LORA_Bypass_Command ( uint8_t* cmdBuf, uint8_t cmdLen )

  Remarks:
    See prototype in app.h.
 */
uint8_t APP_LORA_Bypass_Command(uint8_t* cmdBuf, uint8_t cmdLen)
{
    uint8_t ret = 0;
    if( loraData.bModuleEnable == true )
    {
        if( cmdBuf != NULL )
        {
            SERCOM1_USART_ReadAbort();

            // TODO AJ: use different buffer, e.g. loraUartDataBuff?
            memset(loraGpDataBuff, 0, sizeof(loraGpDataBuff));
            SERCOM1_USART_ReadCallbackRegister(APP_LORA_Uart_CB_Bypass, (uintptr_t)NULL);
            SERCOM1_USART_Read(&loraUartRxBuf, 2);

            SYS_CONSOLE_PRINT(TERM_GREEN"cby: %s\r\n"TERM_RESET, (char*)cmdBuf);
            snprintf((char*)&cmdBuf[cmdLen], 3, "\r\n");
            SERCOM1_USART_Write(cmdBuf, cmdLen+2);
        }
    }
    else
    {
        SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Error]"TERM_RESET" module disabled, impossible to transmit command: %s\r\n", cmdBuf);
    }
    return ret;
}

/*******************************************************************************
  Function:
    void APP_LORA_Parse_Command ( uint8_t* cmdBuf, uint8_t cmdLen )

  Remarks:
    See prototype in app.h.
 */
uint8_t APP_LORA_Parse_Command(uint8_t* cmdBuf, uint8_t cmdLen)
{
    uint8_t ret = 0;

    if( cmdBuf != NULL )
    {
        char *pCmd = strtok((char*)cmdBuf, " ");
        char *pSubCmd = strtok(NULL, "\r");

        // enable the LoRaWan module
        if( strncmp(pCmd, "lorawan_on", 10) == 0 )
        {
            APP_LORA_Enable();
        }
        // disable the LoRaWan module
        else if( strncmp(pCmd, "lorawan_off", 11) == 0 )
        {
            APP_LORA_Disable();
        }
        // set regular uplink behavior configuration
        else if( strncmp(pCmd, "lorawan_set_uplink", 18) == 0 )
        {
            if( APP_PDS_Set_LoraUplinkConfiguration(pSubCmd) == true )
                SYS_CONSOLE_PRINT("[LORA] "TERM_GREEN"[Successfully]"TERM_RESET" set uplink configuration\r\n"TERM_RESET);
            else
                SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Failed]"TERM_RESET" to set uplink configuration\r\n"TERM_RESET);
        }
        // get regular uplink behavior configuration
        else if( strncmp(pCmd, "lorawan_get_uplink", 18) == 0 )
        {
            sprintf(&loraGpDataBuff[0], "<%d> <%d> <%d>", loraData.msgTimeout, loraData.msgNumUncnf, loraData.msgNumCnf);
            SYS_CONSOLE_PRINT("[LORA] Uplink configuration %s\r\n", loraGpDataBuff);
        }
        // stop regular uplink behavior
        else if( strncmp(pCmd, "lorawan_stop_uplink", 19) == 0 )
        {
            APP_TIMER_StopTimer(APP_TIMER_LORA_SEND_STATUS);
            SYS_CONSOLE_PRINT("[LORA] "TERM_GREEN"[Regular]"TERM_RESET" message uplink stopped\r\n");
        }
        else if( loraData.bModuleEnable == false )
        {
            SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Error]"TERM_RESET" module disabled, impossible to execute command '%s'\r\n", pCmd);
        }
        // start regular uplink behavior
        else if( strncmp(pCmd, "lorawan_start_uplink", 20) == 0 )
        {
            // has LORAWAN network already joined
            if( bleSensorData.loraOnOffStatus == LORA_ON )
            {
                uint16_t tmrState = APP_TIMER_IsTimerActive(APP_TIMER_LORA_SEND_STATUS);

                // start regular uplink only if timer is inactive
                if( (tmrState == APP_RES_INVALID_PARA) || (tmrState == pdFALSE) )
                {
                    // stop possible running timer and clear LED in any way
                    APP_TIMER_StopTimer(APP_TIMER_LORA_ERROR);
                    LED_BLUE_Clear();

                    // configure data transmission parameters, e.g. uplink config, pause time, etc. and start uplink
                    APP_LORA_Uplink_ConfigureMessageAndStart();
                }
                else
                    SYS_CONSOLE_PRINT("[LORA] "TERM_YELLOW"[Message]"TERM_RESET" uplink still ongoing ...\r\n");
            }
            else    // check LORAWAN join state
            {
                APP_TIMER_StopTimer(APP_TIMER_LORA_SEND_STATUS);
                APP_LORA_Get_Status(LORA_DEFINE_JOIN_ACTION_START_UPLINK);
                ret = 1;
            }
        }
        // set DEVEUI, JOINEUI and APPKEY
        else if( strncmp(pCmd, "lorawan_set_keys", 16) == 0 )
        {
            if( APP_LORA_WLR089_Set_Keys(pSubCmd) == true )
            {
                SYS_CONSOLE_PRINT("[LORA] Setting the keys ...\r\n");
                ret = 1;
            }
            else
                SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Error]"TERM_RESET" occurred while setting keys\r\n");
        }
        // get DEVEUI, JOINEUI and APPKEY
        else if( strncmp(pCmd, "lorawan_get_keys", 16) == 0 )
        {
            SYS_CONSOLE_PRINT("[LORA] Getting the keys ...\r\n");
            APP_LORA_WLR089_Get_Keys(loraGpDataBuff);
            ret = 1;
        }
        // try to join the LoRaWAN network
        else if( strncmp(pCmd, "lorawan_join", 12) == 0 )
        {
            APP_LORA_Join_Handler();
            ret = 1;
        }
        else
        {
            SYS_CONSOLE_PRINT("[LORA] "TERM_RED"[Unsupported]"TERM_RESET" command: %s\r\n"TERM_RESET, pCmd);
        }

        if( ret == 1 )
        {
            APP_Msg_T appMsg;
            appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
            OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
            loraData.state = APP_STATE_SERVICE_TASKS;
        }
    }   // if( cmdBuf != NULL )

    return ret;
}

/*******************************************************************************
  Function:
    void APP_LORA_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Initialize( void )
{
    APP_PDS_Get_LoraWanModuleEnable();
    APP_PDS_Get_LoraAppKey(NULL);

    loraData.appQueue        = xQueueCreate( 32, sizeof(APP_Msg_T) );
    loraData.wlr089CmdQueue  = xQueueCreate( 32, sizeof(appCmdEntry_t) );
    loraData.bIsResetDone    = false;
    loraData.bIsInitialized  = false;
    loraData.bUartGotResp    = false;
    loraData.bProcessNextCmd = false;

    if( loraData.bModuleEnable == true )
    {
        loraData.state = APP_STATE_LORA_RESET;
        SYS_CONSOLE_PRINT("[LORA] Module enabled\r\n");
    }
    else
    {
        loraData.state = APP_STATE_LORAWAN_MODULE_DISABLED;
        SYS_CONSOLE_PRINT("[LORA] Module disabled\r\n");
    }
}

/******************************************************************************
  Function:
    void APP_LORA_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_LORA_Tasks( void )
{
    APP_Msg_T appMsg;
    APP_Msg_T *p_appMsg;
    p_appMsg = &appMsg;

    switch ( loraData.state )
    {
        case APP_STATE_LORA_RESET:
        {
            // Register the UART RX callback function
            SERCOM1_USART_ReadCallbackRegister(APP_LORA_Uart_CB, (uintptr_t)NULL);

            // Reset the UART5 buffer
            memset(loraUartRxBuf, 0, sizeof(loraUartRxBuf));

            LED_RED_Set();
            LED_GREEN_Clear();
            LED_BLUE_Clear();

            APP_LORA_WLR089_Reset();

            loraData.bProcessNextCmd = true;
            loraData.state = APP_STATE_SERVICE_TASKS;

            appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
            OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);

            // start 3s hardware detection timeout
            APP_TIMER_SetTimer(APP_TIMER_LORA_ERROR, APP_TIMER_3S, false);
            break;
        }
        case APP_STATE_INIT:
        {
            // Reset the UART5 buffer
            memset(loraUartRxBuf, 0, sizeof(loraUartRxBuf));

            if( loraData.bIsResetDone == true )
            {
                APP_LORA_WLR089_Init();
                APP_PDS_Get_LoraUplinkConfiguration();

                // initialize LORA specific IDs/Keys stored in PDS
                // with application/user specific IDs/Keys
                APP_LORA_WLR089_Init_IDs(eAPPKEY);

                // pre-indicate initialization is done
                loraData.bIsInitialized = true;

                loraData.bProcessNextCmd = true;
                loraData.state = APP_STATE_SERVICE_TASKS;

                appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
                OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
            }
            else
                loraData.state = APP_STATE_LORA_RESET;
            break;
        }
        case APP_STATE_SERVICE_TASKS:
        {
            //SYS_CONSOLE_MESSAGE(TERM_CYAN"    RDY\r"TERM_RESET);

            if( OSAL_QUEUE_Receive(&loraData.appQueue, &appMsg, OSAL_WAIT_FOREVER) )
            {
                if(p_appMsg->msgId == APP_MSG_LORA_TRIGGER_CMD)
                {
                    //SYS_CONSOLE_MESSAGE(TERM_YELLOW"."TERM_RESET);

                    if( loraData.bProcessNextCmd == true )
                    {
                        if( OSAL_QUEUE_Receive(&loraData.wlr089CmdQueue, &loraData.wlr089CmdFromQueue, 50) )
                        {
                            if( loraData.wlr089CmdFromQueue.pLoraCmd->cmdInterface == eUart )
                            {
                                #if LORA_MSG_VERBOSE_LEVEL >= VERBOSE_LEVEL_CMDRSP_ONLY
                                SYS_CONSOLE_PRINT(TERM_GREEN"cmd: %s"TERM_RESET, loraData.wlr089CmdFromQueue.pLoraCmd->pCommand);
                                #endif

                                SERCOM1_USART_ReadAbort();
                                memset(loraUartDataBuff, 0, sizeof(loraUartDataBuff));

                                SERCOM1_USART_ReadCallbackRegister(APP_LORA_Uart_CB, (uintptr_t)NULL);
                                SERCOM1_USART_Read(&loraUartRxBuf, loraData.wlr089CmdFromQueue.pLoraCmd->cmdRespMinNumBytes);
                                SERCOM1_USART_Write((uint8_t *)loraData.wlr089CmdFromQueue.pLoraCmd->pCommand, loraData.wlr089CmdFromQueue.pLoraCmd->cmdSize);
                                loraData.bUartGotResp = false;
                            }
                            else if( loraData.wlr089CmdFromQueue.pLoraCmd->cmdInterface == eInternal )
                            {
                                #if LORA_MSG_VERBOSE_LEVEL >= VERBOSE_LEVEL_CMDRSP_ONLY
                                SYS_CONSOLE_PRINT(TERM_LIGHT_GREEN"cmd: %s"TERM_RESET, loraData.wlr089CmdFromQueue.pLoraCmd->pCommand);
                                #endif

                                appMsg.msgId = APP_MSG_LORA_INTERNAL_CMD;
                                OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
                            }

                            // prevent other_user tasks being "blocked"
                            loraData.bProcessNextCmd = false;
                            loraData.state = APP_STATE_SERVICE_TASKS;
                        }
                    }
                    else
                    {
                        OSAL_QUEUE_Receive(&loraData.wlr089CmdQueue, &loraData.wlr089CmdFromQueue, 50);
                        SYS_CONSOLE_PRINT("[LORA] "TERM_YELLOW"[Cmd]"TERM_RESET" still in progress, discarding command: %s", loraData.wlr089CmdFromQueue.pLoraCmd->pCommand);
                    }
                }
                else if(p_appMsg->msgId == APP_MSG_LORA_UART_CMD_CB)
                {
                    //SYS_CONSOLE_MESSAGE(TERM_RED"."TERM_RESET);

                    if( loraData.bUartGotResp == true )
                    {
                        loraData.bUartGotResp = false;
                        SERCOM1_USART_ReadAbort();

                        appCmdEntry_t *pWlr089Cmd = &loraData.wlr089CmdFromQueue;
                        parserCmdInfo_t paramList;
                        paramList.pParam1 = &loraUartDataBuff[0];
                        paramList.pParam2 = pWlr089Cmd;

                        if( pWlr089Cmd->printLoraCmdRespDataBuf == false )
                        {
                            #if LORA_MSG_VERBOSE_LEVEL >= VERBOSE_LEVEL_CMDRSP_ONLY
                                SYS_CONSOLE_PRINT(TERM_CYAN"rsp: %s"TERM_RESET, &loraUartDataBuff[0]);
                            #endif
                        }

                        if( pWlr089Cmd->pLoraCmd->pfCmdRespCb != NULL )
                        {
                            int recBytes = pWlr089Cmd->pLoraCmd->pfCmdRespCb(&paramList);
                            if( recBytes != 0 )
                            {
                                SERCOM1_USART_Read(&loraUartRxBuf, recBytes);
                                loraData.bProcessNextCmd = false;

                                // prevent other tasks being "blocked"
                                loraData.state = APP_STATE_SERVICE_TASKS;
                                break;
                            }
                        }

                        if( pWlr089Cmd->nextLoraCmdInQueue == true )
                        {
                            APP_Msg_T appMsg;
                            appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
                            OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
                        }
                        else
                        {
                            if( loraData.bIsInitialized == true )
                                loraData.state = APP_STATE_SERVICE_TASKS;
                            else
                                loraData.state = APP_STATE_INIT;

                            if( pWlr089Cmd->printLoraCmdRespDataBuf == true )
                            {
                                #if LORA_MSG_VERBOSE_LEVEL >= VERBOSE_LEVEL_CMDRSP_ONLY
                                    SYS_CONSOLE_PRINT(TERM_CYAN"rsp: %s\r\n"TERM_RESET, &loraGpDataBuff[0]);
                                #endif
                            }
                        }

                        loraData.bProcessNextCmd = true;
                    }
                }
                else if(p_appMsg->msgId == APP_MSG_LORA_INTERNAL_CMD)
                {
                    //SYS_CONSOLE_MESSAGE(TERM_PURPLE"."TERM_RESET);

                    appCmdEntry_t *pWlr089Cmd = &loraData.wlr089CmdFromQueue;
                    parserCmdInfo_t paramList;
                    paramList.pParam1 = &loraUartDataBuff[0];
                    paramList.pParam2 = pWlr089Cmd;

                    if( pWlr089Cmd->printLoraCmdRespDataBuf == false )
                    {
                        #if LORA_MSG_VERBOSE_LEVEL != VERBOSE_LEVEL_OFF
                            SYS_CONSOLE_PRINT(TERM_LIGHT_CYAN"rsp: %s\r\n"TERM_RESET, &loraUartDataBuff[0]);
                        #endif
                    }

                    if( pWlr089Cmd->pLoraCmd->pfCmdRespCb != NULL )
                    {
                        int recBytes = pWlr089Cmd->pLoraCmd->pfCmdRespCb(&paramList);
                        if( recBytes != 0 )
                        {
                            loraData.bProcessNextCmd = false;

                            // prevent other tasks being "blocked"
                            loraData.state = APP_STATE_SERVICE_TASKS;
                            break;
                        }
                    }

                    if( pWlr089Cmd->nextLoraCmdInQueue == true )
                    {
                        APP_Msg_T appMsg;
                        appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
                        OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
                    }
                    else
                    {
                        loraData.state = APP_STATE_SERVICE_TASKS;

                        if( pWlr089Cmd->printLoraCmdRespDataBuf == true )
                        {
                            #if LORA_MSG_VERBOSE_LEVEL != VERBOSE_LEVEL_OFF
                                SYS_CONSOLE_PRINT(TERM_LIGHT_CYAN"rsp: %s\r\n"TERM_RESET, &loraGpDataBuff[0]);
                            #endif
                        }
                    }

                    loraData.bProcessNextCmd = true;
                }
                else if(p_appMsg->msgId == APP_MSG_TRS_BLE_SENSOR_BTN_LORA_INT)
                {
                    APP_LORA_Join_Handler();

                    appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
                    OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
                }
                else if(p_appMsg->msgId == APP_TIMER_LORA_SEND_STATUS_MSG)
                {
                    APP_LORA_TimerTrig_Handler();

                    appMsg.msgId = APP_MSG_LORA_TRIGGER_CMD;
                    OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
                }
                else if(p_appMsg->msgId == APP_TIMER_LORA_ERROR_MSG)
                {
                    if( APP_TIMER_IsTimerActive(APP_TIMER_LORA_ERROR) == true )
                        LED_BLUE_Toggle();
                    else
                        LED_BLUE_Clear();
                }

                else if(p_appMsg->msgId == APP_TIMER_LORA_MODULE_DETECT_ERROR_MSG)
                {
                    SYS_CONSOLE_MESSAGE(TERM_YELLOW"[LORA] Module detection timer expired\r\n"TERM_RESET);
                    loraData.state = APP_STATE_LORAWAN_MODULE_NOT_AVAILABLE;
                }
                else
                {
                    //SYS_CONSOLE_PRINT("msgId=%d\r\n", p_appMsg->msgId);
                }
            }
            break;
        }
        /* TODO: Handle error in application's state machine. */
        case APP_STATE_LORAWAN_MODULE_DISABLED:
        case APP_STATE_LORAWAN_MODULE_NOT_AVAILABLE:
        {
            break;
        }
        /* The default state should never be executed. */
        default:
        {
            //SYS_CONSOLE_PRINT("=%d=", p_appMsg->msgId);
            //SYS_CONSOLE_MESSAGE(TERM_PURPLE".\r\n"TERM_RESET);
            break;
        }
    }
}
