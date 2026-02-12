/*M>---------------------------------------------------------------------------
 * Project:       DALI-Stack Main
 * Description:   Communication between Low-Level-Driver, Application
 * and Stack/Library
 *
 * Copyright (c) by mbs GmbH, Krefeld, info@mbs-software.de
 * All rights reserved.
 --------------------------------------------------------------------------<M*/
#ifndef WITHOUT_DALI

#include "dali.h"
#include "definitions.h"                // SYS function prototypes
#include "app.h"

dalilib_action_t    action;
uint8_t             bDaliStackReady;
dalilib_instance_t  pDaliStackInstance;

enum query_request
{
    QUERY_ACTUAL_LEVEL,
    QUERY_FADE_TIME
};
enum query_request q_req;

//Debugging only
char oBuf[256];

/******************************************************************************/
/* will be called by the DALI library if it wants to send a DALI message
 * to the driver
 * result: 0: success
 ******************************************************************************/

static uint8_t dali_send_callback(void * p_context, dalilib_frame_t* p_frame)
{
    uint8_t result = 0;

    if(bDaliStackReady)
    {
    	result = dalill_pushSendQueue(p_context, (dalill_frame_t* ) p_frame, pDaliStackInstance);
    }
    return result;
}


/******************************************************************************/
/* forward low level frames to dalilib
*******************************************************************************/

void dalill_toDalilib(void * p_context, dalill_frame_t* p_frame)
{
	dalilib_receive(p_context, (dalilib_frame_t*)p_frame);
}

/******************************************************************************/
/* invoked by the DALI stack to notify the application that is ready
 * to receive the actions
*******************************************************************************/
static void dali_ready_callback(void)
{
    bDaliStackReady = 1;
    SYS_CONSOLE_PRINT("[DALI] DALI Stack ready\r\n");
    appDALIData.setInitialIntensity = true;
}

/******************************************************************************/
/* invoked by the DALI stack to notify the application that is ready to receive the actions
*******************************************************************************/
static void dali_log_callback(char *pLog, uint8_t nLen)
{
  sprintf(oBuf,"[DALI] DALI LOG: <%s>\r\n",pLog);
  SYS_CONSOLE_PRINT(oBuf);
}

/******************************************************************************/
/*  will be called by the DALI stack to tell the application  shall load the persistent variables
* 
* Note. Have allocated one half of EERam for a Gear and a Device
*
******************************************************************************/
static uint8_t dali_loadmemory_callback(void* pMemory, uint16_t nSize)
{
    return 0; // TODO not implemented yet  
}

/******************************************************************************/
/* will be called by the DALI library to tell the application shall save the persistent variables
* 
* Note. Have allocated one half of EERam for a Gear and a Device
* 
*******************************************************************************/
static uint8_t dali_savememory_callback(void* pMemory, uint16_t nSize)
{
    return 0; // TODO not implemented yet
}

/******************************************************************************/
/* will be called by the DALI library to notify the application about 'something'
*******************************************************************************/
static uint8_t dali_gear_reaction_callback(dalilib_react_ctrl_gear_t * p_args)
{
    switch(p_args->reactionCode)
    {
        case DALILIB_REACT_CTRL_GEAR_QUERY_STATUS:
            {
                if (p_args->valueType != DALILIB_RESPONSE_VALUE_VALID)
                {
                    SYS_CONSOLE_PRINT("[DALI] Warning, query status response is not valid (%d)\r\n", p_args->valueType);
                    if(appDALIData.bleResponse == true)
                    {
                        APP_Msg_T appMsg;

                        appDALIData.bleResponse = false;
                        appMsg.msgId = APP_MSG_TRS_BLE_LOG;
                        sprintf((char *)appMsg.msgData, "[DALI] Warning, query status response is not valid (%d)\r\n", p_args->valueType);
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                    }
                }
                else
                {
                    SYS_CONSOLE_PRINT("[DALI] Received query status\r\n");
                    SYS_CONSOLE_PRINT("[DALI] Addr is %d, lamp is %d\r\n", p_args->gearStatus.shortAddress, p_args->gearStatus.lampOn);
                    SYS_CONSOLE_PRINT("[DALI] Gear failure is %d, lamp failure is %d\r\n", p_args->gearStatus.controlGearFailure, p_args->gearStatus.lampFailure);
                    if(appDALIData.bleResponse == true)
                    {
                        APP_Msg_T appMsg;

                        appDALIData.bleResponse = false;
                        appMsg.msgId = APP_MSG_TRS_BLE_LOG;
                        sprintf((char *)appMsg.msgData, "[DALI] Addr is %d, lamp is %d", p_args->gearStatus.shortAddress, p_args->gearStatus.lampOn);
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                        appMsg.msgId = APP_MSG_TRS_BLE_LOG;
                        sprintf((char *)appMsg.msgData, "[DALI] Gear failure is %d, lamp failure is %d", p_args->gearStatus.controlGearFailure, p_args->gearStatus.lampFailure);
                        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                    }
                }
            }
            break;
            
        case DALILIB_REACT_CTRL_GEAR_QUERIES:
            {
                if (p_args->valueType != DALILIB_RESPONSE_VALUE_VALID)
                {
                    SYS_CONSOLE_PRINT("[DALI] Warning, queries response is not valid\r\n");
                }
                else
                {
                    if (q_req == QUERY_ACTUAL_LEVEL)
                    {
                        SYS_CONSOLE_PRINT("[DALI] Actual level is %lu\r\n", p_args->reactValue);
                    }
                    else if (q_req == QUERY_FADE_TIME)
                    {
                        SYS_CONSOLE_PRINT("[DALI] Fade time is %lu\r\n", p_args->reactValue);
                    }
                }
            }
            break;
            
        case DALILIB_REACT_CTRL_GEAR_MEMORY:
            {
                if (p_args->valueType != DALILIB_RESPONSE_VALUE_VALID)
                {
                    SYS_CONSOLE_PRINT("[DALI] Warning, memory response is not valid\r\n");
                }
                else
                {
                    SYS_CONSOLE_PRINT("[DALI] Memory reaction happened, value is %ld\r\n", p_args->reactValue);
                }
            }
            break;
        default:
            break;
    }
    return (0);
}

/******************************************************************************/
/* will be called by the DALI library to notify the application about 'something'
*******************************************************************************/
static uint8_t dali_reaction_callback(dalilib_action_t * p_args)
{
    uint8_t res = 0;
    switch(p_args->actionType)
    {
        case DALILIB_CTRL_GEAR_REACTION:
            res = dali_gear_reaction_callback(&p_args->gearReact);
            break;

        case DALILIB_STATUS_REACTION:
            if (p_args->statusReact.reactionCode == DALILIB_REACT_STATUS_BUSPOWER)
            {
              switch(p_args->statusReact.reactValue)
              {
                case  DALILIB_BUSPOWER_ON:
                  SYS_CONSOLE_PRINT("[DALI] Reaction status value (LED): BUSPOWER_ON\r\n");
                  appDALIData.setInitialIntensity = true;
                  appDALIData.busPower = DALILIB_BUSPOWER_ON;
                break;

                case DALILIB_BUSPOWER_OFF:
                  SYS_CONSOLE_PRINT("[DALI] Reaction status value (LED): BUSPOWER_OFF\r\n");
                  appDALIData.busPower = DALILIB_BUSPOWER_OFF;
                  app_lteData.daliLightStatus = 0;
                break;

                case DALILIB_BUSPOWER_SYSTEMFAILURE:
                  SYS_CONSOLE_PRINT("[DALI] Reaction status value (LED): BUSPOWER_SYSTEMFAILURE\r\n");
                  appDALIData.busPower = DALILIB_BUSPOWER_SYSTEMFAILURE;
                  app_lteData.daliLightStatus = 0;
                break;

                default:
                  SYS_CONSOLE_PRINT("[DALI] Reaction status value (LED): ILLEGAL BUSPOWERVALUE !!!\r\n");
                break;
              }
            }

            if (p_args->statusReact.reactionCode == DALILIB_REACT_STATUS_BUSCOLLISION)
            {
              switch(p_args->statusReact.reactValue)
              {
                case DALILIB_BUSCOLLISION_DETECTED:
                  SYS_CONSOLE_PRINT("[DALI] Bus-collision detected\r\n");
#ifdef DEBUG_BIT_TIMING
                  timingstats_t stats;
                  giveBitTimings(&stats, ctrl_device_config.context);
                  SYS_CONSOLE_PRINT("[DALI] bit: %d min: %d max: %d\r\n", stats.currentBit, stats.minBitTime, stats.maxBitTime);
#endif
                break;

                case DALILIB_BUSCOLLISION_NONE:
                  SYS_CONSOLE_PRINT("[DALI] Bus-collision gone\r\n");
                break;
              }
            }
        break;


        default:
            res = 0xFF; //unsupported reaction type
            break;
    }

    return (res);
}

/******************************************************************************/
/* DALI stack configuration struct will be initialized
 * 
 * TODO - Validate these settings
 * 
*******************************************************************************/
static void dali_create_device_config(void)
{
    memset(&ctrl_device_config, 0, sizeof(ctrl_device_config));
    // stack mode
    ctrl_device_config.mode = STACK_MODE_CONTROL_DEVICE;
    ctrl_device_config.vendor.control_device_mode = CTRL_DEV_MODE_SINGLE;
    // control device information
    ctrl_device_config.vendor.control_device_instance_type = INPUT_DEVICE_INST_0;
    ctrl_device_config.vendor.control_device_instance_number = 0;
    // vendor information
    ctrl_device_config.vendor.gtin[0] = 'M';
    ctrl_device_config.vendor.gtin[1] = 'B';
    ctrl_device_config.vendor.gtin[2] = 'S';
    ctrl_device_config.vendor.gtin[3] = '0';
    ctrl_device_config.vendor.gtin[4] = '1';
    ctrl_device_config.vendor.gtin[5] = '7';
    ctrl_device_config.vendor.firmware_ver_major = 0x01;
    ctrl_device_config.vendor.firmware_ver_minor = 0x02;
    ctrl_device_config.vendor.identify_number[0] = 0x01;
    ctrl_device_config.vendor.identify_number[1] = 0x02;
    ctrl_device_config.vendor.identify_number[2] = 0x03;
    ctrl_device_config.vendor.identify_number[3] = 0x04;
    ctrl_device_config.vendor.identify_number[4] = 0x08;
    ctrl_device_config.vendor.identify_number[5] = 0x0A;
    ctrl_device_config.vendor.identify_number[6] = 0x12;
    ctrl_device_config.vendor.identify_number[7] = 0x12;
    ctrl_device_config.vendor.hw_ver_major = 0x10;
    ctrl_device_config.vendor.hw_ver_minor = 0x33;
    
    
    ctrl_device_config.callbacks.fPAppReady = dali_ready_callback;
    ctrl_device_config.callbacks.fPAppSend = dali_send_callback;
    ctrl_device_config.callbacks.fPAppLoadMem = dali_loadmemory_callback;
    ctrl_device_config.callbacks.fPAppSaveMem = dali_savememory_callback;
    ctrl_device_config.callbacks.fPAppReAction = dali_reaction_callback;
    ctrl_device_config.callbacks.fPAppLog = dali_log_callback;

    // no address, shall be set to 0xFF
    ctrl_device_config.vendor.shortAddress = 0x07;
}

/******************************************************************************/
/* application function
 * Here only the example call of the action functions is displayed.
 * The application should decide when the action functions should be called
 * 
 * Note. the DALI defines for memory bank and others are contained in libdali.h
 *       To add another action use'APP_ACTION' enums and simply copy and modify
 *       the parameter defines
*******************************************************************************/
void app_action( APP_ACTION appAction )
{    
    switch ( appAction )
    {            
        case APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_STATUS:
            action.actionType = DALILIB_CTRL_GEAR_ACTION;
            action.gearAct.adr.adrType = DALI_ADRTYPE_BROADCAST;
            action.gearAct.gearActionType = DALILIB_ACT_TYPE_CTRL_GEAR_QUERY_STATUS;
            dalilib_action(pDaliStackInstance, &action);
            break;

        case APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_FADE_TIME:
            action.actionType = DALILIB_CTRL_GEAR_ACTION;
            action.gearAct.adr.adrType = DALI_ADRTYPE_BROADCAST;
            action.gearAct.gearActionType = DALILIB_ACT_TYPE_CTRL_GEAR_QUERIES;
            action.gearAct.action.queriesAction = DALILIB_ACT_CTRL_GEAR_QUERY_FADE_TIME_RATE;
            q_req = QUERY_FADE_TIME;
            dalilib_action(pDaliStackInstance, &action);
            break;
            
        case APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_LEVEL:
            action.actionType = DALILIB_CTRL_GEAR_ACTION;
            action.gearAct.adr.adrType = DALI_ADRTYPE_BROADCAST;
            action.gearAct.gearActionType = DALILIB_ACT_TYPE_CTRL_GEAR_QUERIES;
            action.gearAct.action.queriesAction = DALILIB_ACT_CTRL_GEAR_QUERY_ACTUAL_LEVEL;
            q_req = QUERY_ACTUAL_LEVEL;
            dalilib_action(pDaliStackInstance, &action);
            break;

        case APP_ACTION_DALILIB_CTRL_GEAR_ACTION_LEVEL_OFF:
            action.actionType = DALILIB_CTRL_GEAR_ACTION;
            action.gearAct.adr.adrType = DALI_ADRTYPE_BROADCAST;
            action.gearAct.gearActionType = DALILIB_ACT_TYPE_CTRL_GEAR_LEVEL;
            action.gearAct.action.levelAction = DALILIB_ACT_CTRL_GEAR_LEVEL_OFF;
            appDALIData.lastLedIntensity = 0;
            dalilib_action(pDaliStackInstance, &action);
            break;
            
        case APP_ACTION_DALILIB_CTRL_GEAR_ACTION_SET_LEVEL:
//            SYS_CONSOLE_PRINT("[DALI] LED intensity is %ld\r\n", appData.ledIntensity);
            if(appDALIData.ledIntensity < 0)
            {
                appDALIData.ledIntensity = 0;
                SYS_CONSOLE_PRINT("[DALI] Received negative LED intensity, not accepted, forcing to 0\r\n");
            }
            else if(appDALIData.ledIntensity > 100)
            {
                appDALIData.ledIntensity = 100;
                SYS_CONSOLE_PRINT("[DALI] Received LED intensity percent %d > 100, forcing to 100%%\r\n", appDALIData.ledIntensity);
            }
            action.actionType = DALILIB_CTRL_GEAR_ACTION;
            action.gearAct.adr.adrType = DALI_ADRTYPE_BROADCAST;
            action.gearAct.gearActionType = DALILIB_ACT_TYPE_CTRL_GEAR_LEVEL;
            action.gearAct.action.levelAction = DALILIB_ACT_CTRL_GEAR_LEVEL_DAPC;
            action.gearAct.value.actionValue = appDALIData.ledIntensity * 1000;
            appDALIData.lastLedIntensity = appDALIData.ledIntensity;
            dalilib_action(pDaliStackInstance, &action);
            break;
            
        default:
            break;
    }
}

/******************************************************************************/
/* timer every 10-20ms 
 * Note running every 10ms
*******************************************************************************/
void dalill_timingHelper(void *pInstance,uint32_t dali_time_ticker)
{
    dalilib_timingHelper(pInstance, dali_time_ticker);
}

/******************************************************************************/
/* initialize the HAL driver and the DALI stack
*******************************************************************************/
void  dali_init(dalill_bus_t* pDalill_bus)
{
    pDaliStackInstance = NULL;
    bDaliStackReady = 0;

    dalill_base_t dalill_base;

    // create new DALI stack instance
    pDaliStackInstance = dalilib_createinstance();
    if (NULL == pDaliStackInstance)
    {
        // error
        return;
   }

    addInstance(pDalill_bus,pDaliStackInstance);

    // create DALI stack config
    dali_create_device_config();

    // add Low Level struct to DALI stack instance and vice versa
    ctrl_device_config.context = pDalill_bus;
    pDalill_bus->context = pDaliStackInstance;

    // initialize DALI stack instance
    if (R_DALILIB_OK != dalilib_init(pDaliStackInstance , &ctrl_device_config))
    {
        // error
        return;
    }
  
    // TODO - add any new memory banks here
//    strcpy((char *) &LEDBank1[10],"LED BANK1");
//    dalilib_create_other_membank(pDaliStackInstance ,10,sizeof(LEDBank1),LEDBank1);
//    strcpy((char *) &LEDBank2[10],"LED BANK2");
//    dalilib_create_other_membank(pDaliStackInstance ,11,sizeof(LEDBank2),LEDBank2);

    // start DALI stack instance
    if (R_DALILIB_OK != dalilib_start(pDaliStackInstance))
    {
        // error
        return;
    }

    // initilise callbacks for DALI Low Level Driver
    dalill_base.debug_mode         = 0;
    dalill_base.max_frame_length   = MAXFRAMELENGTH;

    //for DALI Click 2
    dalill_base.rx_rcv_high_offset = 0;//0;
    dalill_base.rx_rcv_low_offset  = 0;//0;
    dalill_base.rx_high_offset     = 35;//70;
    dalill_base.rx_low_offset      = -40;//0;
    dalill_base.tx_high_offset     = 3;//45;
    dalill_base.tx_low_offset      = -3;//35;

    dalill_base.dalilltimingHelper = &dalill_timingHelper;
    dalill_base.dalilltoDalilib = &dalill_toDalilib;
    dalill_base.getCurrentTimerVal = &dalill_getCurrentTimerVal;
    dalill_base.getTimerPeriod = &dalill_getTimerPeriod;
    dalill_base.setTimerPeriod = &dalill_setTimerPeriod;
    dalill_base.startTimer = &dalill_startTimer;
    dalill_base.signalToThread = NULL;
    dalill_base.enableIRQ = NULL;
    dalill_base.disableIRQ = NULL;

    dalill_createBase(&dalill_base);
}

void dali_main(void)
{
    dalill_processQueues();
}
#else
    #warning DALI stack has to be aquired from MBS
#endif /* WITHOUT_DALI */
