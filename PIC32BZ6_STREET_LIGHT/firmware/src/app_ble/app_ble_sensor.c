/*******************************************************************************
  Application BLE Sensor Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sensor.c

  Summary:
    This file contains the Application Transparent Server Role functions for this project.

  Description:
    This file contains the Application Transparent Server Role functions for this project.
    The implementation of demo mode is included in this file.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include <stdio.h>
#include "app_trps.h"
#include "Sensors/inc/rgb_led.h"
#include "Sensors/inc/temp_sensor.h"
#include "app_timer/app_timer.h"
#include "peripheral/eic/plib_eic.h"
#include "system/console/sys_console.h"
#include "app.h"
#include "app_adv.h"
#include "app_ble_conn_handler.h"
#include "app_ble_sensor.h"
#include "app_error_defs.h"
#include "../app_lora/app_lora.h"

#include "peripheral/gpio/plib_gpio.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************

/**@brief BLE sensor read periodic timer */
APP_TRPS_SensorData_T bleSensorData = {LED_OFF, {DEFAULT_H,DEFAULT_S,DEFAULT_V}, {0,0}, LORA_OFF};
bool b_button_debounce = false;

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
/* Callback for BLE Sensor LED on/off through on board button */
void APP_TRPS_Sensor_Button_Callback(uintptr_t context)
{
    APP_Msg_T appMsg;

    if(!b_button_debounce)
    {
        if( context == 1 )
        {
            appMsg.msgId = APP_MSG_TRS_BLE_SENSOR_BTN_LED_INT;
            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
        }
        else
        {
            appMsg.msgId = APP_MSG_TRS_BLE_SENSOR_BTN_LORA_INT;
            OSAL_QUEUE_Send(&loraData.appQueue, &appMsg, 0);
        }

        b_button_debounce = true;
    }
}

/* BLE Sensor LED on/off control through on board button */
void APP_TRPS_Sensor_Button_Handler(void)
{
    if(bleSensorData.rgbOnOffStatus == LED_OFF)
    {
        RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue,bleSensorData.RGB_color.Saturation,bleSensorData.RGB_color.Value);
        bleSensorData.rgbOnOffStatus = LED_ON;
        SYS_CONSOLE_MESSAGE("[BTN] Street Light ON\r\n");
    }
    else
    {
        RGB_LED_Off();
        bleSensorData.rgbOnOffStatus = LED_OFF;
        SYS_CONSOLE_MESSAGE("[BTN] Street Light OFF\r\n");
    }

    b_button_debounce = false;
}

/* Periodic 100ms once timer handler to read sensor data*/
void APP_TRPS_Sensor_TimerHandler(void)
{
    volatile float temperature;
    uint16_t tempS = 0;

    temperature = MCP9700_Temp_Celsius();

    if(temperature < 0)
    {
        temperature = temperature * (-1.0);

        tempS = (uint16_t) (temperature * 10);  // 1 decimal place

        tempS = tempS | 0x8000;  // Set the MSB to indicate negative temperature
    }
    else
    {
        tempS = (uint16_t) (temperature * 10);  // 1 decimal place
    }

    bleSensorData.tempSens.lsb = (uint8_t) tempS;
    bleSensorData.tempSens.msb = (uint8_t) (tempS>>8);

}

/* Init BLE Sensor Specific */
void APP_TRPS_Sensor_Init(void)
{
    /* Init Periodic application timer to do BLE sensor specific measurement like read temp sensor handled in APP_TRPS_Sensor_TimerHandler() */
    APP_TIMER_SetTimer(APP_TIMER_TEMP_SENSOR_STATUS, APP_TIMER_5S, true);

    /* Register external button interrupt callback */
    EIC_CallbackRegister(BUTTON_1, APP_TRPS_Sensor_Button_Callback, 1);
    // disable for debugging (permanent interrupts, probably due to missing suppressor capacitor C805)
    //EIC_InterruptDisable(BUTTON_1);

    EIC_CallbackRegister(BUTTON_2, APP_TRPS_Sensor_Button_Callback, 2);
}

/* Fill Adv Beacon with BLE Sensor specific */
void APP_TRPS_Sensor_Beacon(uint8_t* ptr_data)
{
    uint8_t idx=0;
    //Service Data
    ptr_data[idx++] = APP_ADV_ADD_DATA_CLASS_BYTE;
    ptr_data[idx++] = APP_ADV_PROD_TYPE_BLE_SENSOR;
    ptr_data[idx++] = RGB_ONOFF_STATUS_NFY;
    ptr_data[idx++] =  bleSensorData.rgbOnOffStatus;
    ptr_data[idx++] = TEMP_SENSOR_NFY;
    ptr_data[idx++] =  bleSensorData.tempSens.msb; // MSB
    ptr_data[idx++] =  bleSensorData.tempSens.lsb;  // LSB
}