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
    app_ble.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef APP_LORA_H
#define APP_LORA_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
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
#define STRING_TERM       '\0'  // string termination character
#define STRING_TERM_SIZE  1     // size of string termination character

// verbose levels
#define VERBOSE_LEVEL_DEBUG         3
#define VERBOSE_LEVEL_MIN           2
#define VERBOSE_LEVEL_CMDRSP_ONLY   1
#define VERBOSE_LEVEL_OFF           0
#define LORA_MSG_VERBOSE_LEVEL      VERBOSE_LEVEL_CMDRSP_ONLY

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
typedef enum
{
    eUart = 0,  // command is sent via UART interface
    eInternal,  // command is handled internally, no physical interface is used
    eUndefined
}loraCmdIntace_t;
    
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

typedef struct loraCmdEntry
{
	char* pCommand;
    uint8_t cmdSize;
	actionCbFct_t pfCmdRespCb;
    uint8_t cmdRespMinNumBytes;
    uint8_t flags;
    char* pCmdRespDataBuf;
    loraCmdIntace_t cmdInterface;
}loraCmdEntry_t;

typedef struct appCmdEntry
{
    struct loraCmdEntry* pLoraCmd;
    bool  nextLoraCmdInQueue;
    bool  printLoraCmdRespDataBuf;
}appCmdEntry_t;

typedef struct appLoraUplinkConfig
{
    char   *uplinkPayloadType;  // uplink message type (confirmed or unconfirmed)
    uint8_t uplinkCnt;          // number of consecutive messages of the same type to send
    uint8_t uplinkErrCnt;       // uplink error counter value
    uint8_t uplinkMaxErrVal;    // maximum number of uplink errors
}appLoraUplinkConfig_t;

typedef struct
{
    /* The application's current state */
    APP_STATES state;

    OSAL_QUEUE_HANDLE_TYPE appQueue;
    OSAL_QUEUE_HANDLE_TYPE wlr089CmdQueue;

    appCmdEntry_t wlr089CmdFromQueue;    
    
    bool bIsResetDone;
    bool bIsInitialized;
    bool bUartGotResp;
    bool bProcessNextCmd;
    
    bool bModuleEnable;
    
    uint8_t msgTimeout;     // defines the timeout between LORA data transmissions in seconds
    uint8_t msgNumCnf;      // number of confirmed LORA messages after sending unconfirmed message(s)
    uint8_t msgNumUncnf;    // number of unconfirmed LORA messages before sending confirmed message(s)

    appLoraUplinkConfig_t uplinkConfig[2];

}APP_DATA_LORA;

extern APP_DATA_LORA loraData;
extern TaskHandle_t xAPP_LORA_Tasks;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
void APP_LORA_Initialize(void);
void APP_LORA_Enable(void);
void APP_LORA_Disable(void);
void APP_LORA_Tasks(void);
uint8_t APP_LORA_Parse_Command(uint8_t* cmdBuf, uint8_t cmdLen);
uint8_t APP_LORA_Bypass_Command(uint8_t* cmdBuf, uint8_t cmdLen);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* APP_LORA_H */

/*******************************************************************************
 End of File
 */

