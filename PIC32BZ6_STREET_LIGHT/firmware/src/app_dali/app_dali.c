/*******************************************************************************
  MPLAB Harmony DALI Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_dali.c

  Summary:
    This file contains the source code for the MPLAB Harmony DALI application.

  Description:
    This file contains the source code for the MPLAB Harmony DALI application.
    It implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
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

#ifndef WITHOUT_DALI

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_dali.h"

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
    This structure should be initialized by the APP_DALI_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DALI_DATA appDALIData;
dalill_bus_t* pDalill_bus_0;
dalilib_cfg_t ctrl_device_config;
static SYS_TIME_HANDLE daliTimer = SYS_TIME_HANDLE_INVALID;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

// DALI_RX interrupt handler
void DALIRX_User_Handler(uintptr_t context)
{
    //GPIO_RB12_Set();
    dalill_interruptExt(pDalill_bus_0);
    //GPIO_RB12_Clear();
}

void TC0_Callback_InterruptHandler(TC_TIMER_STATUS status, uintptr_t context)
{
    //GPIO_RB11_Set();
    dalill_timerInterrupt2(pDalill_bus_0);
    //GPIO_RB11_Clear();
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_DALI_Initialize ( void )

  Remarks:
    See prototype in app_dali.h.
 */

void APP_DALI_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appDALIData.state = APP_DALI_STATE_INIT;
}

/******************************************************************************
  Function:
    void APP_DALI_Tasks ( void )

  Remarks:
    See prototype in app_dali.h.
 */

void APP_DALI_Tasks ( void )
{
    APP_Msg_T appMsg;

    /* Check the application's current state. */
    switch ( appDALIData.state )
    {
        /* Application's initial state. */
        case APP_DALI_STATE_INIT:
        {
            //TC0 is used for DALI stack. Timer is started in dali_init()
            TC0_TimerCallbackRegister(TC0_Callback_InterruptHandler, (uintptr_t)NULL);
            // DALI RX
            EIC_CallbackRegister(EIC_PIN_0, DALIRX_User_Handler, 0);
            // DALI initializations and dali start !
            pDalill_bus_0 = dalill_createBusLine(&dalill_getBusState, &dalill_setBusStateHigh, &dalill_setBusStateLow);
            if (pDalill_bus_0 == NULL)
            {
                SYS_CONSOLE_PRINT("[DALI] pDalill_bus_0 is NULL\r\n");
                appDALIData.state = APP_DALI_STATE_INIT_ERROR;
                break;
            }
            appDALIData.setInitialIntensity = false;
            appDALIData.busPower = DALILIB_BUSPOWER_ON;
            dali_init(pDalill_bus_0);
            appDALIData.state = APP_DALI_STATE_SERVICE_TASKS;
            appDALIData.bleResponse = false;
            break;
        }

        case APP_DALI_STATE_SERVICE_TASKS:
        {
            if(appDALIData.setInitialIntensity)
            {
                // set default intensity delayed to be sure light controller is powered up
                appDALIData.setInitialIntensity = false;
                SYS_TIME_DelayMS(1500, &daliTimer);
            }
            else if(SYS_TIME_DelayIsComplete(daliTimer) == true)
            {
                daliTimer = SYS_TIME_HANDLE_INVALID;
                appMsg.msgId = APP_DALI_ACTION;
                appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                appMsg.msgData[1] = APP_DALI_DEFAULT_INTENSITY_LEVEL;
                OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
            }
            else if(   (appDALIData.ledIntensity > APP_DALI_DEFAULT_INTENSITY_LEVEL)
                    && (daliTimer == SYS_TIME_HANDLE_INVALID))
            {
                // back to default intensity after 10s
                SYS_TIME_DelayMS(10000, &daliTimer);
            }
            dali_main();
            break;
        }

        case APP_DALI_STATE_INIT_ERROR:
        {
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

uint8_t dali_check_light_intensity(uint8_t level)
{
    uint8_t ret;
    
    if(level < 0)
    {
        ret = 0;
    }
    else if(level > 50)
    {   // limit to 50% intensity to avoid melting of plastic
        ret = 50;
    }
    else
    {
        ret = level;
    }
    
    return ret;
}

#endif /* WITHOUT_DALI */
/*******************************************************************************
 End of File
 */
