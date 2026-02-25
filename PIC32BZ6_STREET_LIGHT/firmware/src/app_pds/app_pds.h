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
    app_pds.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef APP_PDS_H
#define APP_PDS_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "app.h"
#include "driver/pds/include/pds.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************   
#define DEVEUI_SIZE         16
#define JOINEUI_SIZE        16
#define APPKEY_SIZE         32
#define DEVEUI_ARRAY_SIZE   DEVEUI_SIZE  + STRING_TERM_SIZE
#define JOINEUI_ARRAY_SIZE  JOINEUI_SIZE + STRING_TERM_SIZE
#define APPKEY_ARRAY_SIZE   APPKEY_SIZE  + STRING_TERM_SIZE

// introduce ID`s
typedef enum
{
    PDS_MODULE_LORAWAN_ENABLE_ITEM_ID = PDS_MODULE_APP_OFFSET,
    PDS_MODULE_LTE_ENABLE_ITEM_ID,
    PDS_LTE_URL_ITEM_ID,
    PDS_LTE_SUB_TOPIC_ITEM_ID,
    PDS_LTE_PUB_TOPIC_ITEM_ID,
    PDS_BLE_BLELOG_ENABLE_ITEM_ID,
    PDS_LORA_DEVEUI_ITEM_ID,
    PDS_LORA_JOINEUI_ITEM_ID,
    PDS_LORA_APPKEY_ITEM_ID,
    PDS_LORA_MSG_TIMEOUT_ITEM_ID,
    PDS_LORA_MSG_NUM_CNF_ITEM_ID,
    PDS_LORA_MSG_NUM_UNCNF_ITEM_ID,
    PDS_MODULE_FTD_ENABLE_ITEM_ID,
    APP_MAX_PDS_ITEMS_ID
}eIDsAndKeys;

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
extern bool bLoraWanModule;
extern char sDEVEUI[DEVEUI_ARRAY_SIZE];
extern char sJOINEUI[JOINEUI_ARRAY_SIZE];
extern char sAPPKEY[APPKEY_ARRAY_SIZE];

#if LORA_MSG_VERBOSE_LEVEL >= VERBOSE_LEVEL_MIN
    #define SYS_CONSOLE_PRINT_PDS(fmt, ...)     SYS_CONSOLE_Print(SYS_CONSOLE_DEFAULT_INSTANCE, fmt, ##__VA_ARGS__)
#else
    #define SYS_CONSOLE_PRINT_PDS(fmt, ...)
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
bool APP_PDS_Set_BleLog(void);
bool APP_PDS_Get_BleLog(void);

bool   APP_PDS_Set_LoraWanModuleEnable(void);
bool   APP_PDS_Get_LoraWanModuleEnable(void);
bool   APP_PDS_Set_LoraUplinkConfiguration(char* pConfig);
bool   APP_PDS_Get_LoraUplinkConfiguration();
int8_t APP_PDS_Init_IDs(void);
int8_t APP_PDS_Set_LoraDevEui(char* pDevEui);
bool   APP_PDS_Get_LoraDevEui(char* pBuf);
int8_t APP_PDS_Set_LoraJoinEui(char* pJoinEui);
bool   APP_PDS_Get_LoraJoinEui(char* pBuf);
int8_t APP_PDS_Set_LoraAppKey(char* pAppKey);
bool   APP_PDS_Get_LoraAppKey(char* pBuf);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* APP_PDS_H */

/*******************************************************************************
 End of File
 */

