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
  Application BLE Profile Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_trsps_handler.c

  Summary:
    This file contains the Application BLE functions for this project.

  Description:
    This file contains the Application BLE functions for this project.
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdio.h>

#include "app_trsps_handler.h"
#include "osal/osal_freertos_extend.h"
#include "peripheral/sercom/usart/plib_sercom0_usart.h"
#include "system/console/sys_console.h"
#include "system/command/sys_command.h"

#include "app.h"
#include "app_ble_sensor.h"
#include "app_ble_conn_handler.h"
#include "app_timer/app_timer.h"
#include "app_adv.h"
#include "sensors/inc/rgb_led.h"
#include "../app_trps.h"
#include "app_ble.h"
#include "app_lora/app_lora.h"
#include "app_thread.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************
extern uint16_t conn_hdl;


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

void APP_TrspsEvtHandler(BLE_TRSPS_Event_T *p_event)
{
#ifdef BLE_DEBUG_LOGS
        SYS_CONSOLE_PRINT(TERM_BLUE"APP_TrspsEvtHandler=%d\r\n"TERM_RESET, p_event->eventId);
#endif

    switch(p_event->eventId)
    {
        case BLE_TRSPS_EVT_CTRL_STATUS:
        {
            /* TODO: implement your application code.*/
            APP_TRPS_EventHandler(p_event);
        }
        break;

        case BLE_TRSPS_EVT_TX_STATUS:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_TRSPS_EVT_CBFC_ENABLED:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_TRSPS_EVT_CBFC_CREDIT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_TRSPS_EVT_RECEIVE_DATA:
        {
            APP_Msg_T appMsg;
            uint16_t data_len;
            uint8_t bleData[256];
#ifndef WITHOUT_DALI
            int level;
#endif
            unsigned int hue, sat, value;
            char url[50];
            char pub[80];
            char sub[80];

            // Retrieve received data length
            BLE_TRSPS_GetDataLength(p_event->eventField.onReceiveData.connHandle, &data_len);

            // Retrieve received data
            BLE_TRSPS_GetData(p_event->eventField.onReceiveData.connHandle, bleData);

            // add string termination character
            memcpy(bleData + data_len, "\0", 1);

            appMsg.msgId = APP_MSG_TRS_BLE_LOG;
            sprintf((char *)appMsg.msgData, "OK");  // set default response

            if(strncmp((char *)bleData, "lorawan_", 8) == 0)
            {
                APP_LORA_Parse_Command(bleData, data_len);
            }
            else if(strncmp((char *)bleData, "help", data_len) == 0)
            {
                const SYS_CMD_DESCRIPTOR* pDcpt;

                pDcpt = builtinCmdTbl;
                SYS_CONSOLE_PRINT("---------- Built in commands ----------\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "---------- Built in commands ----------");
                    OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                }

                while(pDcpt->cmdStr != NULL)
                {
                    SYS_CONSOLE_PRINT(" *** %s %s ***\r\n", pDcpt->cmdStr, pDcpt->cmdDescr);
                    if(appBleData.bBleLogEnable == false)
                    {
                        sprintf((char *)appMsg.msgData, " *** %s %s ***", pDcpt->cmdStr, pDcpt->cmdDescr);
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                    }
                    pDcpt++;
                }

                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "OK"); // default response at the end
                }
            }
            else if(strncmp((char *)bleData, "reset", data_len) == 0)
            {
                SYS_CONSOLE_PRINT(" *** System Reboot ***\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, " *** System Reboot ***");
                    BLE_TRSPS_SendData(conn_hdl, strlen((char *)appMsg.msgData), appMsg.msgData);
                }
                vTaskDelay(20U/portTICK_PERIOD_MS); // for console and BLE response

                taskDISABLE_INTERRUPTS();
                for(;;) /* wait for WDT reset */
                {
                    __NOP();
                }
            }
#ifndef WITHOUT_DALI
            else if(strncmp((char *)bleData, "gl", data_len) == 0)
            {
                SYS_CONSOLE_PRINT("[DALI] Actual level is %d%%\r\n", app_lteData.daliLightIntensity);
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "[DALI] Actual level is %d%%", app_lteData.daliLightIntensity);
                }
            }
            else if(strncmp((char *)bleData, "gs", data_len) == 0)
            {
                APP_Msg_T appDaliMsg;
                appDaliMsg.msgId = APP_DALI_ACTION;
                appDaliMsg.msgData[0] = APP_DALI_GEAR_ACTION_GET_STATUS;
                OSAL_QUEUE_Send(&appData.appQueue, &appDaliMsg, 0);

                if(appBleData.bBleLogEnable == false)
                {
                    appDALIData.bleResponse = true;
                }
            }
            else if(sscanf((char *)bleData, "sl %d", &level) == 1)
            {
                APP_Msg_T appDaliMsg;
                appDaliMsg.msgId = APP_DALI_ACTION;
                appDaliMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                appDaliMsg.msgData[1] = level;
                OSAL_QUEUE_Send(&appData.appQueue, &appDaliMsg, 0);

                if(appBleData.bBleLogEnable == false)
                {
                    appDALIData.bleResponse = true;
                }
            }
            else if(strncmp((char *)bleData, "lamp_on", data_len) == 0)
            {
                APP_Msg_T appDaliMsg;
                appDaliMsg.msgId = APP_DALI_ACTION;
                appDaliMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                appDaliMsg.msgData[1] = app_lteData.daliLightIntensity;
                OSAL_QUEUE_Send(&appData.appQueue, &appDaliMsg, 0);

                if(appBleData.bBleLogEnable == false)
                {
                    appDALIData.bleResponse = true;
                }
            }
            else if(strncmp((char *)bleData, "lamp_off", data_len) == 0)
            {
                APP_Msg_T appDaliMsg;
                appDaliMsg.msgId = APP_DALI_ACTION;
                appDaliMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                appDaliMsg.msgData[1] = 0;
                OSAL_QUEUE_Send(&appData.appQueue, &appDaliMsg, 0);

                if(appBleData.bBleLogEnable == false)
                {
                    appDALIData.bleResponse = true;
                }
            }
#endif /* WITHOUT_DALI */
            else if(sscanf((char *)bleData, "rgb_on %x %x %x", &hue, &sat, &value) == 3)
            {
                bleSensorData.RGB_color.Hue        = (uint8_t)hue;
                bleSensorData.RGB_color.Saturation = (uint8_t)sat;
                bleSensorData.RGB_color.Value      = (uint8_t)value;
                RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
                bleSensorData.rgbOnOffStatus = LED_ON;

                SYS_CONSOLE_PRINT("RGB color 0x%02X\r\n", bleSensorData.RGB_color.Hue);
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "RGB color 0x%02X", bleSensorData.RGB_color.Hue);
                }
            }
            else if(strncmp((char *)bleData, "rgb_off", data_len) == 0)
            {
                RGB_LED_Off();
                bleSensorData.rgbOnOffStatus = LED_OFF;

                SYS_CONSOLE_PRINT("RGB off\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "RGB off");
                }
            }
            else if(strncmp((char *)bleData, "lte_on", data_len) == 0)
            {
                APP_LTE_Enable();

                SYS_CONSOLE_PRINT("LTE module enabled\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "LTE module enabled");
                }
            }
            else if(strncmp((char *)bleData, "lte_off", data_len) == 0)
            {
                APP_LTE_Disable();

                SYS_CONSOLE_PRINT("LTE module disabled\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "LTE module disabled");
                }
            }
            else if(sscanf((char *)bleData, "lte_set_params %s %s %s", url, pub, sub) == 3)
            {
                APP_LTE_SetParameter(url, pub, sub);

                SYS_CONSOLE_PRINT("LTE parameters set\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "LTE parameters set");
                }
            }
            else if(strncmp((char *)bleData, "lte_get_params", data_len) == 0)
            {
                SYS_CONSOLE_PRINT("%s %s %s\r\n", app_lteData.mqttCloudUrl, app_lteData.mqttPubTopic, app_lteData.mqttSubTopic);
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "%s %s %s", app_lteData.mqttCloudUrl, app_lteData.mqttPubTopic, app_lteData.mqttSubTopic);
                }
            }
            else if(strncmp((char *)bleData, "blelog_on", data_len) == 0)
            {
                APP_BLE_Log_Enable();
            }
            else if(strncmp((char *)bleData, "blelog_off", data_len) == 0)
            {
                APP_BLE_Log_Disable();
            }
            else if(strncmp((char *)bleData, "ext_gpio_on", data_len) == 0)
            {
                EXT_GPIO_Set();

                SYS_CONSOLE_PRINT("External GPIO on\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "External GPIO on");
                }
            }
            else if(strncmp((char *)bleData, "ext_gpio_off", data_len) == 0)
            {
                EXT_GPIO_Clear();

                SYS_CONSOLE_PRINT("External GPIO off\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "External GPIO off");
                }
            }
            else if(strncmp((char *)bleData, "ftd_on", data_len) == 0)
            {
                APP_FTD_Enable();

                SYS_CONSOLE_PRINT("FTD enabled\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "FTD enabled");
                }
            }
            else if(strncmp((char *)bleData, "ftd_off", data_len) == 0)
            {
                APP_FTD_Disable();

                SYS_CONSOLE_PRINT("FTD module disabled\r\n");
                if(appBleData.bBleLogEnable == false)
                {
                    sprintf((char *)appMsg.msgData, "FTD module disabled");
                }
            }
            else if(strncmp((char *)bleData, "status", data_len) == 0)
            {
                uint8_t status[300];

                GetInterfaceStatusString((char *)status, sizeof(status));
                SYS_CONSOLE_PRINT((char *)status);
                if(appBleData.bBleLogEnable == false)
                {
                    snprintf((char *)appMsg.msgData, sizeof(appMsg.msgData), "%.*s", sizeof(appMsg.msgData) - 1, (char *)status);
                }
            }
            // LORA bypass command must start with 'sys', 'mac' or 'radio'
            else if( (strncmp((char *)bleData, "sys ", 4) == 0 ) ||
                     (strncmp((char *)bleData, "mac ", 4) == 0 ) ||
                     (strncmp((char *)bleData, "radio ", 6) == 0 ) )
            {
                APP_LORA_Bypass_Command(bleData, data_len);
            }
            else
            {
                snprintf((char *)appMsg.msgData, sizeof(appMsg.msgData), " *** Command Processor: unknown command '%.200s' ***\r\n", (char *)bleData);
            }

            // send BLE response
            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
        }
        break;

        case BLE_TRSPS_EVT_VENDOR_CMD:
        {
            /* TODO: implement your application code.*/
            APP_TRPS_EventHandler(p_event);
        }
        break;

        case BLE_TRSPS_EVT_ERR_UNSPECIFIED:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_TRSPS_EVT_ERR_NO_MEM:
        {
            /* TODO: implement your application code.*/
        }
        break;

        default:
        break;
    }
}