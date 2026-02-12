/*******************************************************************************
  Application BLE Sensor Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_sensor.h

  Summary:
    This file contains the Application Transparent Server Role functions for this project.

  Description:
    This file contains the Application Transparent Server Role functions for this project.
    The implementation of demo mode is included in this file.
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


#ifndef APP_BLE_SENSOR_H
#define APP_BLE_SENSOR_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
//#include "app_trps.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define DEFAULT_H  170  // 240deg (blue))
#define DEFAULT_S  255  // 100%
#define DEFAULT_V  255  //100%

#define BUTTON_1  EIC_PIN_1
#define BUTTON_2  EIC_PIN_3

// Define for BLE Sensor Ctrl Commands

#define APP_TRP_VENDOR_OPCODE_BLE_SENSOR  0x8A
//  Defines BLE Sensor Control Command Set APP_TRPS_CTRL_CMD
#define    RGB_ONOFF_SET_CMD    0x10
#define    RGB_ONOFF_GET_CMD    0x11    
#define    RGB_COLOR_SET_CMD    0x12
#define    RGB_COLOR_GET_CMD    0x13


//  Defines BLE Sensor Response Command Set APP_TRPS_CTRL_RSP
#define    RGB_ONOFF_SET_RSP    0x20
#define    RGB_ONOFF_GET_RSP    0x21    
#define    RGB_COLOR_SET_RSP    0x22
#define    RGB_COLOR_GET_RSP    0x23


//  Defines BLE Sensor Response Command length APP_TRPS_CTRL_RSP_LENGTH
#define    RGB_ONOFF_SET_RSP_LEN 0x0
#define    RGB_ONOFF_GET_RSP_LEN 0x1    
#define    RGB_COLOR_SET_RSP_LEN 0x0
#define    RGB_COLOR_GET_RSP_LEN 0x3


//  Defines BLE Sensor Notify Command Set APP_TRPS_CTRL_NOTIFY
#define    RGB_ONOFF_STATUS_NFY  0x40
#define    TEMP_SENSOR_NFY       0x41
#define    LORA_ONOFF_STATUS_NFY 0x42

//  Defines BLE Sensor Notify Command length APP_TRPS_CTRL_NOTIFY_LENGTH
#define    RGB_ONOFF_STATUS_NFY_LEN   0x1
#define    TEMP_SENSOR_NFY_LEN        0x2
#define    LORA_ONOFF_STATUS_NFY_LEN  0x1

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
enum APP_TRPS_SENSOR_STATUS_T
{
    LED_OFF = 0x00,
    LED_ON = 0x01,
    LORA_OFF = 0x00,
    LORA_ON = 0x01
};

/*
typedef struct parserCmdInfo_tag
{
    char* pParam1;
    //char* pParam2;
    void* pParam2;
    char* pParam3;
	char* pParam4;
	char* pParam5;
    char* pReplyCmd;
}parserCmdInfo_t;
typedef int (*actionCbFct_t)(parserCmdInfo_t* pParserCmdInfo);

typedef struct parserCmdEntry_tag
{
	const char* pCommand;
    //const struct parserCmdEntry_tag* pNextParserCmd;
    const struct parserCmdEntry_tag* pNextParserCmd;
	actionCbFct_t pActionCbFct;
    uint8_t nextParserCmdSize;
    uint8_t flags;
}parserCmdEntry_t;


typedef struct loraCmdEntry
{
	char* pCommand;
    uint8_t cmdSize; 
	actionCbFct_t pfCmdRespCb; 
    uint8_t flags;    
    const struct loraCmdEntry* pNextParserCmd;
    uint8_t nextParserCmdSize;
}loraCmdEntry_t;

typedef struct appCmdEntry
{
    struct loraCmdEntry* pLoraCmd;
    struct appCmdEntry* pNextAppCmd;
}appCmdEntry_t;
*/



/**@brief The structure contains the information about BLE sensor data */
typedef struct
{
    uint8_t rgbOnOffStatus;     /**< RGB LED On/Off Status */

    struct __attribute__ ((packed))
    {
        uint8_t    Hue;         /**The array contains the information about RGB colour value in HSV format. */
        uint8_t    Saturation;
        uint8_t    Value;
    }RGB_color;
    struct __attribute__ ((packed))
    {
        uint8_t    msb;
        uint8_t    lsb;
    }tempSens;
    
    uint8_t loraOnOffStatus;    /**< LoRa On/Off Status */
    
    
} APP_TRPS_SensorData_T;


extern bool b_button_debounce;
extern APP_TRPS_SensorData_T bleSensorData;
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
void APP_TRPS_Sensor_Init(void);

void APP_TRPS_Sensor_TimerHandler(void);

void APP_TRPS_Sensor_Button_Handler(void);

void APP_TRPS_Sensor_Beacon(uint8_t* ptr_data);

void APP_TRPS_Sensor_Button_Callback(uintptr_t type);

#endif
