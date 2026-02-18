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
  Application BLE Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble.c

  Summary:
    This file contains the Application BLE implementation for this project.

  Description:
    This file contains the Application BLE implementation for this project.
 *******************************************************************************/
#include <stdio.h>

#include "osal/osal_freertos_extend.h"
#include "app_ble.h"
#include "app_ble_handler.h"
#include "app_trsps_handler.h"
#include "definitions.h"
#include "app_pds/app_pds.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************
APP_BLE_DATA   appBleData;
BLE_DD_Config_T g_ddConfig;

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

static void APP_BleStackCb(STACK_Event_T *p_stack)
{
    STACK_Event_T stackEvent;
    APP_Msg_T   appMsg;
    APP_Msg_T   *p_appMsg;

    (void)memcpy((uint8_t *)&stackEvent, (uint8_t *)p_stack, sizeof(STACK_Event_T));
    stackEvent.p_event=OSAL_Malloc(p_stack->evtLen);

    if(stackEvent.p_event==NULL)
    {
        return;
    }

    (void)memcpy(stackEvent.p_event, p_stack->p_event, p_stack->evtLen);

    stackEvent.p_event=stackEvent.p_event;
#ifdef BLE_DEBUG_LOGS
    SYS_CONSOLE_PRINT("APP_BleStackCb: 0x%x, %d, %d\r\n", stackEvent.p_event, *stackEvent.p_event, stackEvent.groupId);
#endif

    if (p_stack->groupId==STACK_GRP_GATT)
    {
        GATT_Event_T *p_evtGatt = (GATT_Event_T *)stackEvent.p_event;

        if (p_evtGatt->eventId == GATTS_EVT_CLIENT_CCCDLIST_CHANGE)
        {
            uint8_t *p_payload;

            p_payload = (uint8_t *)OSAL_Malloc((p_evtGatt->eventField.onClientCccdListChange.numOfCccd*4));
            if (p_payload != NULL)
            {
                (void)memcpy(p_payload, (uint8_t *)p_evtGatt->eventField.onClientCccdListChange.p_cccdList, (p_evtGatt->eventField.onClientCccdListChange.numOfCccd*4));
                p_evtGatt->eventField.onClientCccdListChange.p_cccdList = (GATTS_CccdList_T *)p_payload;
            }
        }
    }

    appMsg.msgId=APP_MSG_BLE_STACK_EVT;

    ((STACK_Event_T *)appMsg.msgData)->groupId=p_stack->groupId;
    ((STACK_Event_T *)appMsg.msgData)->evtLen=p_stack->evtLen;
    ((STACK_Event_T *)appMsg.msgData)->p_event=stackEvent.p_event;

    p_appMsg = &appMsg;
    OSAL_QUEUE_Send(&appData.appQueue, p_appMsg, 0);
}

void APP_BleStackEvtHandler(STACK_Event_T *p_stackEvt)
{
#ifdef BLE_DEBUG_LOGS
    SYS_CONSOLE_PRINT("grpId=%d\r\n", p_stackEvt->groupId);
#endif

    switch(p_stackEvt->groupId)
    {
        case STACK_GRP_BLE_GAP:
        {
            APP_BleGapEvtHandler((BLE_GAP_Event_T *)p_stackEvt->p_event);
        }
        break;

        case STACK_GRP_BLE_L2CAP:
        {
            APP_BleL2capEvtHandler((BLE_L2CAP_Event_T *)p_stackEvt->p_event);
        }
        break;

        case STACK_GRP_BLE_SMP:
        {
            APP_BleSmpEvtHandler((BLE_SMP_Event_T *)p_stackEvt->p_event);
        }
        break;

        case STACK_GRP_GATT:
        {
            APP_GattEvtHandler((GATT_Event_T *)p_stackEvt->p_event);
        }
        break;

        default:
        break;
    }

    //Direct event to BLE middleware
    BLE_DM_BleEventHandler(p_stackEvt);

    BLE_DD_BleEventHandler(&g_ddConfig, p_stackEvt);

    //Direct event to BLE profiles
    /* Transparent Profile */
    BLE_TRSPS_BleEventHandler(p_stackEvt);

    OSAL_Free(p_stackEvt->p_event);
}

void APP_DdEvtHandler(BLE_DD_Event_T *p_event)
{

}

static void APP_BleConfigBasic(void)
{
    int8_t                          connTxPower;
    int8_t                          advTxPower;
    BLE_GAP_AdvParams_T             advParam;
    uint8_t advData[]=CONFIG_BLE_GAP_ADV_DATA;
    BLE_GAP_AdvDataParams_T         appAdvData;
    uint8_t scanRspData[]=CONFIG_BLE_GAP_SCAN_RSP_DATA;
    BLE_GAP_AdvDataParams_T         appScanRspData;
    BLE_GAP_Addr_T devAddr;
    devAddr.addrType = BLE_GAP_ADDR_TYPE_PUBLIC;
    devAddr.addr[0] = 0xA1;
    devAddr.addr[1] = 0xA2;
    devAddr.addr[2] = 0xA3;
    devAddr.addr[3] = 0xA4;
    devAddr.addr[4] = 0xA5;
    devAddr.addr[5] = 0xA6;

    BLE_GAP_GetDeviceAddr(&devAddr);

    //Configure Device Address
    BLE_GAP_SetDeviceAddr(&devAddr);


    // Configure advertising parameters
    BLE_GAP_SetAdvTxPowerLevel(CONFIG_BLE_GAP_ADV_TX_PWR,&advTxPower);      /* Advertising TX Power */

    (void)memset(&advParam, 0, sizeof(BLE_GAP_AdvParams_T));
    advParam.intervalMin = CONFIG_BLE_GAP_ADV_INTERVAL_MIN;     /* Advertising Interval Min */
    advParam.intervalMax = CONFIG_BLE_GAP_ADV_INTERVAL_MAX;     /* Advertising Interval Max */
    advParam.type = CONFIG_BLE_GAP_ADV_TYPE;        /* Advertising Type */
    advParam.advChannelMap = CONFIG_BLE_GAP_ADV_CHANNEL_MAP;        /* Advertising Channel Map */
    advParam.filterPolicy = CONFIG_BLE_GAP_ADV_FILT_POLICY;     /* Advertising Filter Policy */
    BLE_GAP_SetAdvParams(&advParam);

    // Configure advertising data
    appAdvData.advLen = CONFIG_BLE_GAP_ADV_DATA_ORIG_LEN;

    (void)memcpy(appAdvData.advData, advData, appAdvData.advLen);     /* Advertising Data */

    BLE_GAP_SetAdvData(&appAdvData);

    //Configure advertising scan response data
    appScanRspData.advLen = CONFIG_BLE_GAP_SCAN_RSP_DATA_ORIG_LEN;
    (void)memcpy(appScanRspData.advData, scanRspData, appScanRspData.advLen);     /* Scan Response Data */
    BLE_GAP_SetScanRspData(&appScanRspData);

    BLE_GAP_SetConnTxPowerLevel(CONFIG_BLE_GAP_CONN_TX_PWR, &connTxPower);      /* Connection TX Power */
}

static void APP_BleConfigAdvance(void)
{
    uint8_t devName[]=CONFIG_BLE_GAP_DEV_NAME_VALUE;

    BLE_SMP_Config_T                smpParam;

    BLE_DM_Config_T                 dmConfig;
    BLE_GAP_ServiceOption_T         gapServiceOptions;


    // Configure Device Name
    BLE_GAP_SetDeviceName(sizeof(devName), devName);    /* Device Name */


    // GAP Service option
    gapServiceOptions.charDeviceName.enableWriteProperty = CONFIG_BLE_GAP_SVC_DEV_NAME_WRITE;             /* Enable Device Name Write Property */
    gapServiceOptions.charAppearance.appearance = CONFIG_BLE_GAP_SVC_APPEARANCE;                          /* Appearance */
    gapServiceOptions.charPeriPreferConnParam.enable = CONFIG_BLE_GAP_SVC_PERI_PRE_CP;                    /* Enable Peripheral Preferred Connection Parameters */
    gapServiceOptions.charEncDataKeyMatl.enable = CONFIG_BLE_GAP_SVC_ENC_DATA_KEY_MATL;                   /* Enable Encrypted Data Key Material */
    gapServiceOptions.charLeGattSecLvls.enable = CONFIG_BLE_GAP_SVC_LE_GATT_SEC_LVLS;                     /* Enable LE GATT Security Levels */
    BLE_GAP_ConfigureBuildInService(&gapServiceOptions);



    // Configure SMP parameters
    (void)memset(&smpParam, 0, sizeof(BLE_SMP_Config_T));
    smpParam.ioCapability = CONFIG_BLE_SMP_IOCAP_TYPE;                  /* IO Capability */
    smpParam.authReqFlag = CONFIG_BLE_SMP_OPTION;             /* Authentication Setting */

    BLE_SMP_Config(&smpParam);

    // Configure BLE_DM middleware parameters
    dmConfig.secAutoAccept = CONFIG_BLE_DM_SEC_AUTO_ACCEPT;                          /* Auto Accept Security Request */
    dmConfig.connConfig.autoReplyUpdateRequest = CONFIG_BLE_DM_AUTO_REPLY_UPD_REQ;      /* Auto Accept Connection Parameter Update Request */
    dmConfig.connConfig.minAcceptConnInterval = CONFIG_BLE_DM_MIN_CONN_INTERVAL;    /* Minimum Connection Interval */
    dmConfig.connConfig.maxAcceptConnInterval = CONFIG_BLE_DM_MAX_CONN_INTERVAL;    /* Maximum Connection Interval */
    dmConfig.connConfig.minAcceptPeripheralLatency = CONFIG_BLE_DM_MIN_CONN_LATENCY;    /* Minimum Connection Latency */
    dmConfig.connConfig.maxAcceptPeripheralLatency = CONFIG_BLE_DM_MAX_CONN_LATENCY;    /* Maximum Connection Latency */
    BLE_DM_Config(&dmConfig);


    // Configure BLE_DD middleware parameters
    g_ddConfig.waitForSecurity = CONFIG_BLE_GCM_DD_WAIT_FOR_SEC;
    g_ddConfig.initDiscInCentral = CONFIG_BLE_GCM_DD_INIT_DISC_IN_CNTRL;
    g_ddConfig.initDiscInPeripheral = CONFIG_BLE_GCM_DD_INIT_DISC_IN_PER;
    g_ddConfig.disableConnectedDisc = CONFIG_BLE_GCM_DD_DIS_CONN_DISC;
}

void APP_BleStackInitBasic(void)
{
    BLE_GAP_Init();

    BLE_GAP_AdvInit();  /* Advertising */

    BLE_GAP_ConnPeripheralInit();   /* Peripheral */
}

void APP_BleStackInitAdvance(void)
{
    uint16_t gattsInitParam=GATTS_CONFIG_NONE;

    uint16_t gattcInitParam=GATTC_CONFIG_NONE;

    STACK_EventRegister(APP_BleStackCb);

    /* GAP (BLE_GAP_Init())shall be initialized before SMP */
    BLE_SMP_Init();

    /* GAP/SMP shall be initialized before L2CAP */
    BLE_L2CAP_Init();

    /* GAP/SMP/L2CAP shall be initialized before GATTS */
    GATTS_Init(gattsInitParam);

    /* GATTS shall be initialized before GATTC */
    GATTC_Init(gattcInitParam);     /* Enable Client Role */


    //Initialize BLE middleware
    BLE_DM_Init();
    BLE_DM_EventRegister(APP_DmEvtHandler);

    BLE_DD_Init();
    BLE_DD_EventRegister(APP_DdEvtHandler);

    //Initialize BLE services

    //Initialize BLE profiles
    /* Transparent Profile */
    BLE_TRSPS_Init();                                   /* Enable Server Role */
    BLE_TRSPS_EventRegister(APP_TrspsEvtHandler);   /* Enable Server Role */

    APP_BleConfigAdvance();
}

/*******************************************************************************
  Function:
    void APP_BleStackInit ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_BleStackInit(void)
{
    APP_PDS_Get_BleLog();

    APP_BleStackInitBasic();
    APP_BleConfigBasic();
    APP_BleStackInitAdvance();
}

/*******************************************************************************
  Function:
    void APP_BLE_Log_Enable ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_BLE_Log_Enable(void)
{
    appBleData.bBleLogEnable = true;
    APP_PDS_Set_BleLog();
    SYS_CONSOLE_MESSAGE("[BLE] Log over BLE enabled\r\n");
}

/*******************************************************************************
  Function:
    void APP_BLE_Log_Disable ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_BLE_Log_Disable(void)
{
    SYS_CONSOLE_MESSAGE("[BLE] Log over BLE disabled\r\n");
    appBleData.bBleLogEnable = false;
    APP_PDS_Set_BleLog();
}