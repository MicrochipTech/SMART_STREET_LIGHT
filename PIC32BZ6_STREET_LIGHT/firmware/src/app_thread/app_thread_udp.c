/*******************************************************************************
 System Tasks File

  File Name:
    udp_demo.c

  Summary:
    This file contains source code necessary to send udp data using openthread.

  Description:
    This file contains source code necessary to send udp data using openthread.
    
  Remarks:
    
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
#include <string.h>
#include "definitions.h"
#include "configuration.h"

#include "openthread/udp.h"
#include "openthread/message.h"
#include "openthread/ip6.h"
#include "openthread/instance.h"
#include "openthread/error.h"
#include "openthread/thread.h"

#include "app_thread.h"
#include "app_thread_udp.h"
#include "timers.h"
#include "system/command/sys_command.h"

#include "../sensors/inc/rgb_led.h"
#include "app_ble_sensor.h"

#define LED_BLINK_TIME_MS               (150)


extern otInstance *instance;
otUdpSocket aSocket;
const char *msg = "Status of Interfaces";

// *****************************************************************************
// *****************************************************************************

void threadUdpOpen()
{
    otError err;
    app_printf("[FTD] UDP Open\n");
    err = otUdpOpen(instance, &aSocket, threadUdpReceiveCb, NULL);
    if(err != OT_ERROR_NONE)
    {
       app_printf("[FTD] Err: UDP Open failed\n");
        //print error code
        assert(err);
    }
}

void threadUdpSend()
{
    otError err = OT_ERROR_NONE;
    otMessageInfo msgInfo;
    const otIp6Address *mPeerAddr;
    memset(&msgInfo,0,sizeof(msgInfo));
//    otIp6AddressFromString("ff03::1",&msgInfo.mPeerAddr);
    
    mPeerAddr = otThreadGetRealmLocalAllThreadNodesMulticastAddress(instance);
    memcpy(&msgInfo.mPeerAddr, mPeerAddr, OT_IP6_ADDRESS_SIZE);
    
    msgInfo.mPeerPort = UDP_PORT_NO;
    
    do {
        otMessage *udp_msg = otUdpNewMessage(instance, NULL);
        err = otMessageAppend(udp_msg, msg, (uint16_t)strlen(msg));
        if(err != OT_ERROR_NONE)
        {
            app_printf("[FTD] Err: UDP Message Add fail\n");
            break;
        }
        
        err = otUdpSend(instance,&aSocket,udp_msg,&msgInfo);
        if(err != OT_ERROR_NONE)
        {
            app_printf("[FTD] Err: UDP Send fail\n");
            break;
        }
        app_printf("[FTD] UDP Sent data: %s\n",msg);

    }while(false);
    
}

void threadUdpSendAddress(otIp6Address mPeerAddr)
{
    otError err = OT_ERROR_NONE;
    otMessageInfo msgInfo;
    memset(&msgInfo,0,sizeof(msgInfo));
    memcpy(&msgInfo.mPeerAddr, &mPeerAddr, OT_IP6_ADDRESS_SIZE);
    msgInfo.mPeerPort = UDP_PORT_NO;
    char status[300];
    
    do {
        otMessage *udp_msg = otUdpNewMessage(instance, NULL);
        GetInterfaceStatusString(status, sizeof(status));
        err = otMessageAppend(udp_msg, status,(uint16_t)strlen(status));

        if(err != OT_ERROR_NONE)
        {
            app_printf("[FTD] Err: UDP Message Add fail\n");
            break;
        }
        
        err = otUdpSend(instance, &aSocket, udp_msg, &msgInfo);
        if(err != OT_ERROR_NONE)
        {
            app_printf("[FTD] Err: UDP Send fail\n");
            break;
        }

        //app_printf("[FTD] UDP Sent data: %s\n", msg);
    }while(false);
}

void threadUdpBind()
{
   otError err;
   otSockAddr addr;
   memset(&addr, 0, sizeof(otSockAddr));
   addr.mPort = UDP_PORT_NO;
   do
   {
        err = otUdpBind(instance, &aSocket, &addr, OT_NETIF_THREAD);
        if (err != OT_ERROR_NONE) {
            app_printf("[FTD] Err: UDP Bind fail Err: %d\n", err);
            break;
        }
        app_printf("[FTD] UDP Listening on port %d\n", UDP_PORT_NO);
   }while(false);
}

void threadUdpReceiveCb(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    uint16_t len = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    uint8_t output_buffer[len + 1];
    char string[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&(aMessageInfo->mPeerAddr), string, OT_IP6_ADDRESS_STRING_SIZE);

    otMessageRead(aMessage, otMessageGetOffset(aMessage), output_buffer, len);
    output_buffer[len] = '\0';
    //app_printf("[FTD] UDP RX from %s data: %s\n", string, output_buffer);

    if(strncmp((char *)output_buffer, "Toggle LED", len) == 0)
    {
        app_printf("[FTD] UDP RX from %s data: %s\n", string, output_buffer);
        if(bleSensorData.rgbOnOffStatus  == LED_OFF)
        {
            RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
            bleSensorData.rgbOnOffStatus = LED_ON;
        }
        else
        {
            RGB_LED_Off();
            bleSensorData.rgbOnOffStatus = LED_OFF;
        }
    }
#ifndef WITHOUT_DALI
    else if(strncmp((char *)output_buffer, "Toggle DALI", len) == 0)
    {
        APP_Msg_T appMsg;
        uint8_t level = 0;

        app_printf("[FTD] UDP RX from %s data: %s\n", string, output_buffer);
        if(app_lteData.daliLightIntensity == 0)
        {
            level = APP_DALI_DEFAULT_INTENSITY_LEVEL;
        }
        else
        {
            level = 0;
        }
        appMsg.msgId = APP_DALI_ACTION;
        appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
        appMsg.msgData[1] = level;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
#endif
    else
    {
        threadUdpSendAddress(aMessageInfo->mPeerAddr);
    }
}
/*******************************************************************************
 End of File
 */

