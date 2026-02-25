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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "app.h"
#include "definitions.h"
#include "app_ble.h"
#include "app_adv.h"
#include "ble_trsps/ble_trsps.h"
#include "app_timer/app_timer.h"
#include "app_ble_sensor.h"
#include "app_ble_conn_handler.h"
#include "app_lora/app_lora.h"
#include "app_lte/app_lte.h"
#include "app_thread.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************





// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_Initialize ( void )
{
    SYS_CONSOLE_MESSAGE("\r\n***********************************\r\n");
    SYS_CONSOLE_MESSAGE("** Starting Street Lighting Demo **\r\n");
    SYS_CONSOLE_PRINT("**** Software Version: %s ****\r\n", LORA_APP_VERSION);
    SYS_CONSOLE_MESSAGE("***********************************\r\n\n");

    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    //appData.appQueue = xQueueCreate( 64, sizeof(APP_Msg_T) );
    appData.appQueue = xQueueCreate( 32, sizeof(APP_Msg_T) );
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}
#define UART_DATA_MAX   25
uint16_t conn_hdl;// connection handle info captured @BLE_GAP_EVT_CONNECTED event
uint16_t ret;
uint8_t uart_data;
uint8_t uartBuf[UART_DATA_MAX];
uint8_t uartBufNum;

void uart_cb(SERCOM_USART_EVENT event, uintptr_t context)
{
    APP_Msg_T   appMsg;

    // If RX data from UART reached threshold (previously set to 1)
    if( event == SERCOM_USART_EVENT_READ_THRESHOLD_REACHED )
    {
        // Read 1 byte data from UART
        SERCOM0_USART_Read(&uart_data, 1);
        appMsg.msgId = APP_MSG_UART_CB;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
}

void APP_SendUartData()
{
    // Send the uartBuf to connected device through Transparent service
    if(uartBufNum == 0)
        return;

    BLE_TRSPS_SendData(conn_hdl, uartBufNum, uartBuf);
    memset(uartBuf, 0 , sizeof(uartBuf));
    uartBufNum = 0;
}

void APP_UartCBHandler()
{
    uartBuf[uartBufNum] = uart_data;
    if(++uartBufNum == UART_DATA_MAX)
    {
        APP_TIMER_StopTimer(APP_TIMER_SEND_UART);
        APP_SendUartData();
    }
    else
        APP_TIMER_SetTimer(APP_TIMER_SEND_UART,APP_TIMER_500MS, false);
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_Msg_T appMsg[1];
    APP_Msg_T *p_appMsg;
    p_appMsg = appMsg;

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;

            // Enable UART0 Read
            //SERCOM0_USART_ReadNotificationEnable(true, true);

            // Set UART RX notification threshold to be 1
            //SERCOM0_USART_ReadThresholdSet(1);
            //SERCOM0_USART_ReadThresholdSet(10);

            // Register the UART RX callback function
            //SERCOM0_USART_ReadCallbackRegister(uart_cb, (uintptr_t)NULL);

            APP_BleStackInit();
            /* Add BLE Service */
            //BLE_DIS_Add();
            APP_UpdateLocalName(0, NULL);
            // Start Advertisement
            APP_InitConnList();
            APP_ADV_Init();
            APP_TRPS_Sensor_Init();

            // Reset the UART0 buffer
            memset(uartBuf, 0, sizeof(uartBuf));
            uartBufNum = 0;

            LED_RED_Set();
            LED_GREEN_Clear();

            APP_FTD_Init();

            if( appInitialized )
            {
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }
        case APP_STATE_SERVICE_TASKS:
        {
            //SYS_CONSOLE_MESSAGE(TERM_PURPLE"RDY    \r"TERM_RESET);

            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, OSAL_WAIT_FOREVER))
            {
                //SYS_CONSOLE_PRINT("msgId=%d\r\n", p_appMsg->msgId);

                if(p_appMsg->msgId == APP_MSG_BLE_STACK_EVT)
                {
                    // Pass BLE Stack Event Message to User Application for handling
                    APP_BleStackEvtHandler((STACK_Event_T *)p_appMsg->msgData);
                }
                else if(p_appMsg->msgId == APP_TIMER_ADV_CTRL_MSG)
                {
                    APP_BLE_Adv_TimerHandler();
                }
                else if(p_appMsg->msgId == APP_MSG_UART_CB)
                {
                    //SERCOM0_USART_Write((uint8_t *)"Msg to BLE stack\r\n", 18);
                    // Pass BLE UART Data transmission target BLE UART Device handling
                    APP_UartCBHandler();
                }
                else if(p_appMsg->msgId == APP_TIMER_SEND_UART_MSG)
                {
                    //SERCOM0_USART_Write((uint8_t *)"Msg from timer\r\n", 16);
                    APP_SendUartData();
                }
                else if(p_appMsg->msgId == APP_TIMER_TEMP_SENSOR_MSG)
                {
                    //SERCOM0_USART_Write((uint8_t *)"Msg from Temp sensor\r\n", 22);
                    APP_TRPS_Sensor_TimerHandler();
                }
                else if(p_appMsg->msgId == APP_MSG_TRS_BLE_SENSOR_BTN_LED_INT)
                {
                    //SERCOM0_USART_Write((uint8_t *)"Msg from button\r\n", 17);
                    APP_TRPS_Sensor_Button_Handler();
                }
#ifndef WITHOUT_DALI
                else if(p_appMsg->msgId == APP_DALI_ACTION)
                {

                    if(appDALIData.busPower != DALILIB_BUSPOWER_ON)
                    {
                        SYS_CONSOLE_PRINT("[DALI] No bus power!\r\n");
                        if(appDALIData.bleResponse == true)
                        {
                            APP_Msg_T appMsg;

                            appDALIData.bleResponse = false;
                            appMsg.msgId = APP_MSG_TRS_BLE_LOG;
                            sprintf((char *)appMsg.msgData, "[DALI] No bus power!");
                            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        }
                        break;
                    }

                    if(p_appMsg->msgData[0] == APP_DALI_GEAR_ACTION_SET_LEVEL)
                    {
                        appDALIData.ledIntensity = dali_check_light_intensity(p_appMsg->msgData[1]);

                        SYS_CONSOLE_PRINT("[DALI] Setting Gear Level to %ld%%\r\n", appDALIData.ledIntensity);
                        if(appDALIData.bleResponse == true)
                        {
                            APP_Msg_T appMsg;

                            appDALIData.bleResponse = false;
                            appMsg.msgId = APP_MSG_TRS_BLE_LOG;
                            sprintf((char *)appMsg.msgData, "[DALI] Setting Gear Level to %ld%%", appDALIData.ledIntensity);
                            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        }

                        if(appDALIData.ledIntensity == 0)
                        {
                            app_action(APP_ACTION_DALILIB_CTRL_GEAR_ACTION_LEVEL_OFF);
                            app_lteData.daliLightStatus = 0;
                        } else
                        {
                            app_action(APP_ACTION_DALILIB_CTRL_GEAR_ACTION_SET_LEVEL);
                            app_lteData.daliLightStatus = 1;
                            app_lteData.daliLightIntensity = appDALIData.ledIntensity;
                        }
                    }
                    else if(p_appMsg->msgData[0] == APP_DALI_GEAR_ACTION_GET_STATUS)
                    {
                        app_action(APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_STATUS);
                    }
                    else if(p_appMsg->msgData[0] == APP_DALI_GEAR_ACTION_GET_LEVEL)
                    {
                        app_action(APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_LEVEL);
                    }
                    else if(p_appMsg->msgData[0] == APP_DALI_GEAR_ACTION_SEND_TEST_FRAME)
                    {
                        SYS_CONSOLE_PRINT("[DALI] Sending 8 bit test frame\r\n");
                        dalill_stopReceiver();
                        dalill_send8bitTestFrame();
                        dalill_startReceiver();
                    }
                }
#endif /* WITHOUT_DALI */
                else if( p_appMsg->msgId == APP_MSG_TRS_BLE_LOG )
                {
                    BLE_TRSPS_SendData(conn_hdl, strlen((char *)p_appMsg->msgData), p_appMsg->msgData);
                }
                else
                {}
            }
            break;
        }
        /* TODO: implement your application state machine.*/
        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */
