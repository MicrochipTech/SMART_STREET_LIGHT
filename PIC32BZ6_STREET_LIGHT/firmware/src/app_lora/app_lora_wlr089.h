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

#define LORA_MSG_MIN_TIMEOUT    10  // ~10 seconds timeout
#define LORA_MSG_MAX_TIMEOUT    60  // ~60 seconds timeout
#define LORA_MSG_MIN_NUM_CNF    1   // minimum number of confirmed messages
#define LORA_MSG_MAX_NUM_CNF    60  // maximum number of confirmed messages
#define LORA_MSG_MIN_NUM_UNCNF  1   // minimum number of unconfirmed messages
#define LORA_MSG_MAX_NUM_UNCNF  60  // maximum number of unconfirmed messages

#define LORA_PERIODIC_MSG_CNT_UNCNF_MSG 2   // set to 0 to send confirmed messages only
#define LORA_PERIODIC_MSG_CNT_ABORT_MSG 3

//  Defines BLE Sensor Notify Command Set APP_TRPS_CTRL_NOTIFY
#define LORA_ONOFF_STATUS_NFY 0x42

//  Defines BLE Sensor Notify Command length APP_TRPS_CTRL_NOTIFY_LENGTH
#define LORA_ONOFF_STATUS_NFY_LEN  0x1


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
void APP_LORA_Get_Status(void);
void APP_LORA_Set_MessageTxConfiguration(void);

#endif  // APP_LORA_WLR089_H
