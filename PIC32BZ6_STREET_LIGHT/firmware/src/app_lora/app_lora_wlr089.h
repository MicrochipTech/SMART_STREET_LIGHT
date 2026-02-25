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
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_lora_wlr089.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef APP_LORA_WLR089_H
#define APP_LORA_WLR089_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "app_lora.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
// declare application/user specific IDs and Keys
#define JOINEUI   ""
#define DEVEUI    ""
#define APPKEY    ""

#define LORA_DEFINE_ON          "on"
#define LORA_DEFINE_OFF         "off"
#define LORA_DEFINE_CONFIRMED   "cnf"
#define LORA_DEFINE_UNCONFIRMED "uncnf"

#define LORA_DEFINE_JOIN_ACTION_START_UPLINK 0
#define LORA_DEFINE_JOIN_ACTION_INFO_ONLY    1

#define LORA_MSG_MIN_TIMEOUT    10  // ~10 seconds timeout
#define LORA_MSG_MAX_TIMEOUT    60  // ~60 seconds timeout
#define LORA_MSG_MIN_NUM_CNF    1   // minimum number of confirmed messages
#define LORA_MSG_MAX_NUM_CNF    60  // maximum number of confirmed messages
#define LORA_MSG_MIN_NUM_UNCNF  1   // minimum number of unconfirmed messages
#define LORA_MSG_MAX_NUM_UNCNF  60  // maximum number of unconfirmed messages

#define LORA_UPLINK_DEFAULT_CNT       2   // set to 0 to send confirmed messages only
#define LORA_UPLINK_DEFAULT_MAX_ERROR 3   // message error counter value

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
typedef enum
{
    eUSEPDS  = 0,
    eDEVEUI  = 1,   // initialize default DEVEUI (in PDS) with application/user default
    eJOINEUI = 2,   // initialize default JOINEUI (in PDS) with application/user default
    eAPPKEY  = 4    // initialize default APPKEY (in PDS) with application/user default
}eInit_AppSpecIdsKeys;

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
int8_t APP_LORA_WLR089_Init_IDs(uint8_t kindOfIDs);
void APP_LORA_WLR089_Reset(void);
void APP_LORA_WLR089_Init(void);
void APP_LORA_WLR089_Set_DevEui(char* pDevEui);
void APP_LORA_WLR089_Get_DevEui(char* pBuf);
void APP_LORA_WLR089_Set_JoinEui(char* pJoinEui);
void APP_LORA_WLR089_Get_JoinEui(char* pBuf);
void APP_LORA_WLR089_Set_AppKey(char* pAppKey);
int APP_LORA_WLR089_Get_AppKey(parserCmdInfo_t* paramList);
bool APP_LORA_WLR089_Set_Keys(char* pKeys);
void APP_LORA_WLR089_Get_Keys(char* pKeys);
void APP_LORA_WLR089_Set_AdaptiveDataRate(char* state);
void APP_LORA_WLR089_Get_AdaptiveDataRate(void);
void APP_LORA_WLR089_Set_DataRate(char* dataRate);
void APP_LORA_WLR089_Get_DataRate(void);
void APP_LORA_WLR089_Get_Status(void);

void APP_LORA_TimerTrig_Handler(void);
void APP_LORA_Join_Handler(void);
void APP_LORA_Get_Status(uint8_t action);
bool APP_LORA_Uplink_ConfigureMessageAndStart(void);

#endif  // APP_LORA_WLR089_H
