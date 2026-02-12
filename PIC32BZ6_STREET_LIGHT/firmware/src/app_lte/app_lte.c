/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app_lte.h"
#include "../sensors/inc/rgb_led.h"
#include "app_ble_sensor.h"
#include "app_pds/app_pds.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

APP_LTE_DATA_T app_lteData;

typedef struct
{
    const char*     cmdStr;         // string identifying the command
    const char*     cmdExpResp;     // expected response string
    uint32_t        timeout;        // response timeout
    const char*     cmdDescr;       // simple command description

} APP_LTE_COMMANDS; // command descriptor

static APP_LTE_COMMANDS app_lte_command_sequence[]=
{//      command                                                        exp. response               timeout(ms) description
/* [0]*/{"",                                                            "+SYSSTART",                5000,       "Module powered up and ready"},
/* [1]*/{"AT\r",                                                        "OK",                       2000,       "communication check"},
/* [2]*/{"AT+IFC=0,0\r",                                                "OK",                       2000,       "disable hardware flow control"},
/* [3]*/{"AT+CFUN=1\r",                                                 "OK",                       2000,       "set full phone functionality"},
/* [4]*/{"AT+CEREG=1\r",                                                "OK",                       2000,       "Enable network registration unsolicited result code"},
/* [5]*/{"AT+CEREG?\r",                                                 "+CEREG: 1,5",              15000,      "network registered"},
/* [6]*/{"AT+CMEE=2\r",                                                 "OK",                       2000,       "Report Mobile Termination with result code and verbose values"},
/* [7]*/{"AT+CSQ\r",                                                    "OK",                       2000,       "check Signal Quality"},
/* [8]*/{"AT+SQNSMQTTCFG=0,\"smart_lighting_demo_lte\"\r",              "OK",                       2000,       "MQTT client configuration"},
/* [9]*/{"AT+SQNSMQTTCONNECT=0,\""LTEIOT10_MQTT_CLOUD_URL"\",1883\r",   "+SQNSMQTTONCONNECT:0,0",   15000,      "MQTT server connection"},
/*[10]*/{"AT+SQNSMQTTSUBSCRIBE=0,"LTEIOT10_MQTT_SUB_TOPIC"\r", \
                                          "+SQNSMQTTONSUBSCRIBE:0,\""LTEIOT10_MQTT_SUB_TOPIC"\",0", 5000,       "Subscribe to topic"},
};

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
volatile bool response_received;

static SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;
static char mqttMessageString[50];
static char atCmdPubTopicString[110];

PDS_DECLARE_ITEM(PDS_MODULE_LTE_ENABLE_ITEM_ID, sizeof(app_lteData.moduleEnable), &app_lteData.moduleEnable,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LTE_URL_ITEM_ID,           sizeof(app_lteData.mqttCloudUrl), &app_lteData.mqttCloudUrl,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LTE_SUB_TOPIC_ITEM_ID,     sizeof(app_lteData.mqttSubTopic), &app_lteData.mqttSubTopic,  NULL, NO_ITEM_FLAGS);
PDS_DECLARE_ITEM(PDS_LTE_PUB_TOPIC_ITEM_ID,     sizeof(app_lteData.mqttPubTopic), &app_lteData.mqttPubTopic,  NULL, NO_ITEM_FLAGS);
        
static void _APP_LTE_UartReadCallback(uintptr_t context);

void lteiot10_reset_device(void)
{
    // MikroBUS_RST
    LTE_RESETN_Clear();
    //SYS_TIME_DelayMS(100, &timer);
    //while(SYS_TIME_DelayIsComplete(timer) == false) {};
    for(int i=0; i<5000000; i++);
    LTE_RESETN_Set();
    //SYS_TIME_DelayMS(1000, &timer);
    //while(SYS_TIME_DelayIsComplete(timer) == false) {};
    for(int i=0; i<50000000; i++);
 }

static void _APP_LTE_ClearRcvBuffer(void)
{
    // Clear receive buffer and flag
    response_received = false;
    memset(app_lteData.rx_buf, 0, sizeof(app_lteData.rx_buf));
    app_lteData.rx_buf_index = 0;
}

size_t read_response(char *p_response)
{
    size_t len = 0;

    NVIC_DisableIRQ(SERCOM5_IRQn);
    if(response_received)
    {
        len = strlen((char *)app_lteData.rx_buf);
        memcpy(p_response, app_lteData.rx_buf, len);
        _APP_LTE_ClearRcvBuffer();
#ifdef APP_LTE_DEBUG_LOGS
        SYS_CONSOLE_PRINT("[LTE] : rd response: %s\r\n", p_response);
#endif
    }
    NVIC_EnableIRQ(SERCOM5_IRQn);
    return len;
}

bool read_response_and_compare(const char *p_exp_response)
{
    bool ret_val = false;

    NVIC_DisableIRQ(SERCOM5_IRQn);
    if(response_received)
    {
        if(strstr((char *)app_lteData.rx_buf, p_exp_response) != NULL)
        {   // rx_buf contains expected string
#ifdef APP_LTE_DEBUG_LOGS
            SYS_CONSOLE_PRINT("[LTE] : rdcmp pass: %s\r\n", (char *)app_lteData.rx_buf);
#endif
            _APP_LTE_ClearRcvBuffer();
            ret_val = true;
        }
        else
        {
#ifdef APP_LTE_DEBUG_LOGS
            SYS_CONSOLE_PRINT("[LTE] : rdcmp fail: %s\r\n", (char *)app_lteData.rx_buf);
#endif
            response_received = false;
        }
    }
    NVIC_EnableIRQ(SERCOM5_IRQn);
    return ret_val;
}
bool send_cmd(const char *p_cmd)
{
    SERCOM5_USART_Write((uint8_t *)p_cmd, strlen(p_cmd));
#ifdef APP_LTE_DEBUG_LOGS
        SYS_CONSOLE_PRINT("[LTE] : send: %s\n", p_cmd);
#endif
    return true;
}

void _APP_LTE_UartReadCallback(uintptr_t context)
{
    response_received = false;
    app_lteData.rx_buf[app_lteData.rx_buf_index] = app_lteData.rx_byte;
    if (app_lteData.rx_buf_index > 0) {
        if(   (app_lteData.rx_buf[app_lteData.rx_buf_index - 1] == '\r')
           && (app_lteData.rx_buf[app_lteData.rx_buf_index] == '\n'))
        {   // response complete
            app_lteData.rx_buf[app_lteData.rx_buf_index - 1] = 0; // remove \r
            app_lteData.rx_buf[app_lteData.rx_buf_index] = 0;   // remove \n
            app_lteData.rx_buf_index -= 2;
            response_received = true;
        }
        else if(   (app_lteData.rx_buf[app_lteData.rx_buf_index - 1] == '>')
                && (app_lteData.rx_buf[app_lteData.rx_buf_index] == ' '))
        {   // prompt received
            response_received = true;
        }
    }
    app_lteData.rx_buf_index++;
#ifdef APP_LTE_DEBUG_LOGS
    //SYS_CONSOLE_PRINT("[LTE] : read callback: %c\r\n", app_lteData.rx_buf[app_lteData.rx_buf_index - 1]);
#endif
    SERCOM5_USART_Read(&app_lteData.rx_byte, 1);
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
bool APP_LTE_Initialize(void)
{
    SERCOM5_USART_ReadCallbackRegister(_APP_LTE_UartReadCallback, (uintptr_t)NULL);

    _APP_LTE_ClearRcvBuffer();
    SERCOM5_USART_Read(&app_lteData.rx_byte, 1);

    app_lteData.state = APP_LTE_STATE_INIT;

    //LTE_RTS_Clear(); // RTS not connected
    LTE_WAKEUP_Set();

    lteiot10_reset_device();

    app_lteData.status = APP_LTE_OFF;
    app_lteData.cloudConnected = false;
    app_lteData.mqttMessCntLtem = 0;
    app_lteData.mqttRecMessCntLtem = 0;
    app_lteData.daliLightStatusPublished = -1;
    app_lteData.daliLightIntensityPublished = -1;
    app_lteData.rgbOnOffStatusPublished = -1;
    app_lteData.rgbColorPublished = -1;
    app_lteData.extGpioPublished = -1;
    app_lteData.messageRecOngoing = false;
    app_lteData.messagePubOngoing = false;

    // start timer for timeout detection
    SYS_TIME_DelayMS(5000, &timer);

    return true;
}

void APP_LTE_Disable(void)
{
    SERCOM5_USART_Disable();
    LTE_RESETN_Clear();
    PPS_REGS->PPS_RPC10G5R = 5U; //RGB LED green can be used
    app_lteData.state = APP_LTE_STATE_DISABLED;
    app_lteData.moduleEnable = false;
    PDS_Store(PDS_MODULE_LTE_ENABLE_ITEM_ID);
}

void APP_LTE_Enable(void)
{
    PPS_REGS->PPS_RPC10G5R = 0U; //RGB LED green can NOT be used
    SERCOM5_USART_Enable();
    app_lteData.moduleEnable = true;
    PDS_Store(PDS_MODULE_LTE_ENABLE_ITEM_ID);
    APP_LTE_Initialize();
}

static void APP_Parse_Rx_Message(char *mqttRxTopic, char *mqttRxMessage)
{
    char *parts[10];
    int count = 0;

    char *str_copy = strdup(mqttRxTopic);
    if (!str_copy)
    {
        return;
    }

    char *token = strtok(str_copy, "/");
    while(token != NULL)
    {
        parts[count++] = token;
        token = strtok(NULL, "/");
    }

    if(strcmp(parts[count - 2], "set_rgb_lte") == 0)
    {
        const char *rgbled_str = strstr(mqttRxMessage, "\"rgb_led\":");
        const char *color_str = strstr(mqttRxMessage, "\"color\":\"");

        // toggle RGB LED
        if(rgbled_str != NULL)
        {
            if(strstr(rgbled_str, "\"rgb_led\":0") != NULL)
            {
                RGB_LED_Off();
                bleSensorData.rgbOnOffStatus = LED_OFF;
                SYS_CONSOLE_PRINT("[LTE] : RGB off\r\n");
            }
            else if(strstr(rgbled_str, "\"rgb_led\":1") != NULL)
            {
                RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
                bleSensorData.rgbOnOffStatus = LED_ON;
                SYS_CONSOLE_PRINT("[LTE] : RGB on\r\n");
            }
        }

        if(color_str != NULL)
        {
            unsigned int temp_color;

            sscanf(color_str, "\"color\":\"%x\"", &temp_color);
            bleSensorData.RGB_color.Hue = (uint8_t)temp_color;
            if(bleSensorData.rgbOnOffStatus == LED_ON)
            {
                RGB_LED_SetLedColorHSV(bleSensorData.RGB_color.Hue, bleSensorData.RGB_color.Saturation, bleSensorData.RGB_color.Value);
            }
            SYS_CONSOLE_PRINT("[LTE] : RGB color 0x%02X\r\n", bleSensorData.RGB_color.Hue);
        }
    }

    if(strcmp(parts[count - 2], "switch_light") == 0)
    {
#ifndef WITHOUT_DALI
        // toggle DALI light
        if(strstr(mqttRxMessage, "\"dali_light\":") != NULL)
        {
            APP_Msg_T appMsg;
            uint8_t level = 0;

            if(strstr(mqttRxMessage, "\"dali_light\":0") != NULL)
            {
                SYS_CONSOLE_PRINT("[LTE] : DALI light off\r\n");
                level = 0;
            }
            else if(strstr(mqttRxMessage, "\"dali_light\":1") != NULL)
            {
                SYS_CONSOLE_PRINT("[LTE] : DALI light on\r\n");
                if(app_lteData.daliLightIntensity == 0)
                {
                    level = APP_DALI_DEFAULT_INTENSITY_LEVEL;
                }
                else
                {
                    level = app_lteData.daliLightIntensity;
                }
            }
            appMsg.msgId = APP_DALI_ACTION;
            appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
            appMsg.msgData[1] = level;
            OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
        }

        // DALI light intensity
        if(strstr(mqttRxMessage, "\"dali_intensity\":") != NULL)
        {
            APP_Msg_T appMsg;
            unsigned int temp_level;
            uint8_t level;
            char *p = strstr(mqttRxMessage, "\"dali_intensity\":");

            if(sscanf(p, "\"dali_intensity\":%d", &temp_level) == 1)
            {
                level = (uint8_t)temp_level;

                SYS_CONSOLE_PRINT("[LTE] : DALI light change intensity to %d%\r\n", level);
                if(   (app_lteData.daliLightStatus == 1)
                   && (level != app_lteData.daliLightIntensity))
                {
                    appMsg.msgId = APP_DALI_ACTION;
                    appMsg.msgData[0] = APP_DALI_GEAR_ACTION_SET_LEVEL;
                    appMsg.msgData[1] = level;
                    OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                }
                else
                {
                    app_lteData.daliLightIntensity = level;
                }
            }
            else
            {
                SYS_CONSOLE_PRINT("[LTE] : DALI light intensity - invalid value\r\n");
            }
        }
#else
        SYS_CONSOLE_PRINT("[LTE] : no DALI light implementation!\r\n");
#endif /* WITHOUT_DALI */
    }

    if(strcmp(parts[count - 2], "switch_gpio") == 0)
    {
        if(strstr(mqttRxMessage, "\"ext_gpio\":0") != NULL)
        {
            EXT_GPIO_Clear();
            SYS_CONSOLE_PRINT("[LTE] : External GPIO off\r\n");
        }
        else if(strstr(mqttRxMessage, "\"ext_gpio\":1") != NULL)
        {
            EXT_GPIO_Set();
            SYS_CONSOLE_PRINT("[LTE] : External GPIO on\r\n");
        }
    }
    free(str_copy);
}

void APP_LTE_SetParameter(char *url, char *pubTopic, char *subTopic)
{
    // copy input strings
    strncpy(app_lteData.mqttCloudUrl, url, sizeof(app_lteData.mqttCloudUrl));
    strncpy(app_lteData.mqttSubTopic, subTopic, sizeof(app_lteData.mqttSubTopic));
    strncpy(app_lteData.mqttPubTopic, pubTopic, sizeof(app_lteData.mqttPubTopic));
    // store items to PDS
    PDS_Store(PDS_LTE_URL_ITEM_ID);
    PDS_Store(PDS_LTE_SUB_TOPIC_ITEM_ID);
    PDS_Store(PDS_LTE_PUB_TOPIC_ITEM_ID);
    // restart LTE statemachine with updated parameters if module is enabled
    if(app_lteData.moduleEnable)
    {
        APP_LTE_Initialize();
    }
}

void APP_LTE_Tasks(void)
{
    static uint8_t seq_index = 0;

    /* Check the application's current state. */
    switch(app_lteData.state)
    {
        case APP_LTE_STATE_INIT:
            seq_index = 0;
            if(PDS_GetPendingItemsCount() != 0)
            {
                break;
            }
            if(PDS_IsAbleToRestore(PDS_MODULE_LTE_ENABLE_ITEM_ID))
            {
                PDS_Restore(PDS_MODULE_LTE_ENABLE_ITEM_ID);
            }
            else
            {   // enable LTE module by default
                app_lteData.moduleEnable = true;
                PDS_Store(PDS_MODULE_LTE_ENABLE_ITEM_ID);
            }

            if(app_lteData.moduleEnable)
            {
                app_lteData.state = APP_LTE_STATE_INITIALIZE;
            }
            else
            {
                APP_LTE_Disable();
            }

            // Restore LTE parameters
            if(   (PDS_IsAbleToRestore(PDS_LTE_URL_ITEM_ID))
               && (PDS_IsAbleToRestore(PDS_LTE_SUB_TOPIC_ITEM_ID))
               && (PDS_IsAbleToRestore(PDS_LTE_PUB_TOPIC_ITEM_ID)))
            {
                PDS_Restore(PDS_LTE_URL_ITEM_ID);
                PDS_Restore(PDS_LTE_SUB_TOPIC_ITEM_ID);
                PDS_Restore(PDS_LTE_PUB_TOPIC_ITEM_ID);
            }
            else
            {
                strncpy(app_lteData.mqttCloudUrl, LTEIOT10_MQTT_CLOUD_URL, sizeof(app_lteData.mqttCloudUrl));
                strncpy(app_lteData.mqttSubTopic, LTEIOT10_MQTT_SUB_TOPIC, sizeof(app_lteData.mqttSubTopic));
                strncpy(app_lteData.mqttPubTopic, LTEIOT10_MQTT_PUB_TOPIC, sizeof(app_lteData.mqttPubTopic));
            }
            // put together MQTT server connection command
            snprintf(app_lteData.mqttConnectCmd, sizeof(app_lteData.mqttConnectCmd), "AT+SQNSMQTTCONNECT=0,\"%s\",1883\r", app_lteData.mqttCloudUrl);
            app_lte_command_sequence[9].cmdStr = app_lteData.mqttConnectCmd;
            // put together MQTT subscription topic command
            snprintf(app_lteData.mqttSubscribeCmd, sizeof(app_lteData.mqttSubscribeCmd), "AT+SQNSMQTTSUBSCRIBE=0,\"%s\"\r", app_lteData.mqttSubTopic);
            app_lte_command_sequence[10].cmdStr = app_lteData.mqttSubscribeCmd;
            // put together MQTT subscription topic response
            snprintf(app_lteData.mqttSubscribeResp, sizeof(app_lteData.mqttSubscribeResp), "+SQNSMQTTONSUBSCRIBE:0,\"%s\",0", app_lteData.mqttSubTopic);
            app_lte_command_sequence[10].cmdExpResp = app_lteData.mqttSubscribeResp;
            break;

        case APP_LTE_STATE_INITIALIZE:
        {
            if(strcmp(app_lte_command_sequence[seq_index].cmdStr, "AT+CEREG?\r") == 0) // wait for network registration
            {   // special handling because multiple responses are allowed
                char response[400] = {0};
                size_t len;

                len = read_response(response);
                if(len != 0)
                {
                    if(   (strcmp((char *)response, "+CEREG: 5") == 0) // Registered, roaming
                       || (strcmp((char *)response, "+CEREG: 1") == 0) // Registered, home network
                       || (strcmp((char *)response, "+CEREG: 1,5") == 0) // Registered, roaming
                       || (strcmp((char *)response, "+CEREG: 1,1") == 0)) // Registered, home network
                    {
                        seq_index++;

                        if(strcmp(app_lte_command_sequence[seq_index].cmdStr, "") != 0)
                        {   // send next command
                            send_cmd(app_lte_command_sequence[seq_index].cmdStr);
                        }

                        // start timeout
                        if(timer != SYS_TIME_HANDLE_INVALID)
                        {
                            SYS_TIME_TimerDestroy(timer);
                        }
                        SYS_TIME_DelayMS(app_lte_command_sequence[seq_index].timeout, &timer);
                    }
                }
            }
            else if(read_response_and_compare(app_lte_command_sequence[seq_index].cmdExpResp))
            {
                seq_index++;
                if(seq_index >= (sizeof(app_lte_command_sequence)/sizeof(APP_LTE_COMMANDS)))
                {   // end of command list reached
                    SYS_CONSOLE_PRINT("[LTE] : Initialization sequence completed\r\n");
                    app_lteData.state = APP_LTE_STATE_RUNNING;
                    if(timer != SYS_TIME_HANDLE_INVALID)
                    {
                        SYS_TIME_TimerDestroy(timer);
                    }
                    SYS_TIME_DelayMS(2000, &timer);
                    break;
                }

                if(strcmp(app_lte_command_sequence[seq_index].cmdStr, "") != 0)
                {   // send next command
                    send_cmd(app_lte_command_sequence[seq_index].cmdStr);
                }

                // start timeout
                if(timer != SYS_TIME_HANDLE_INVALID)
                {
                    SYS_TIME_TimerDestroy(timer);
                }
                SYS_TIME_DelayMS(app_lte_command_sequence[seq_index].timeout, &timer);
            }

            if(   (timer != SYS_TIME_HANDLE_INVALID)
               && (SYS_TIME_DelayIsComplete(timer) == true))
            {   // timeout reached
                SYS_CONSOLE_PRINT("[LTE] : %s - timeout at seq state %d\r\n", app_lte_command_sequence[seq_index].cmdDescr, seq_index);
                app_lteData.state = APP_LTE_STATE_TIMEOUT;
                SYS_TIME_TimerDestroy(timer);
                SYS_TIME_DelayMS(20000, &timer);
            }
            break;
        }

        case APP_LTE_STATE_TIMEOUT:
        {
            if(seq_index > 0)
            {
                if(app_lteData.connectionRetires >= APP_LTE_MAX_CON_RETRIES)
                {
                    SYS_CONSOLE_PRINT("[LTE] : %s - error\r\n", app_lte_command_sequence[seq_index].cmdDescr);
                    app_lteData.connectionRetires = 0;
                    app_lteData.state = APP_LTE_STATE_NOT_CONNECTED;
                }
                else if(SYS_TIME_DelayIsComplete(timer) == true)
                {   // retry communication check if +SYSSTART has been received
                    SYS_TIME_TimerDestroy(timer);
                    APP_LTE_Initialize();
                    app_lteData.connectionRetires++;
                }
            }
            else
            {
                SYS_CONSOLE_PRINT("[LTE] : module not connected\r\n");
                app_lteData.state = APP_LTE_STATE_NOT_CONNECTED;
            }
            break;
        }

        case APP_LTE_STATE_RUNNING:
        {
            char response[300] = {0};
            size_t len;
            static char topic[100];

            len = read_response(response);
            if(len != 0)
            {
                if(strstr((char *)response, "+SQNSMQTTONMESSAGE:") != NULL)
                {   // message notification
                    const char* start = strchr(response, '"');
                    start++;  // first "
                    const char* end = strchr(start, '"');
                    size_t len = end - start;
                    strncpy(topic, start, len);
                    topic[len] = '\0'; // mark end of string

                    if(strstr((char *)topic, "json/error") == NULL)
                    {   // ask for message of topic
                        char atCmdRecMessageString[200];
                        sprintf(atCmdRecMessageString, "AT+SQNSMQTTRCVMESSAGE=0,\"%s\"\r", topic);
                        send_cmd(atCmdRecMessageString);
                        app_lteData.messageRecOngoing = true;
                        app_lteData.mqttRecMessCntLtem++;
                    }
                }
                else if(   (app_lteData.messageRecOngoing)
                        && (strstr((char *)response, "\"payload\":") != NULL))
                {
                    APP_Parse_Rx_Message(topic, response);
                }
                else if(strstr((char *)response, "+CEREG:") != NULL)
                {
                    SYS_CONSOLE_PRINT("[LTE] : network status change: %s\r\n", response);
                }

                if(strstr((char *)response, "ERROR") != NULL)
                {
                    SYS_CONSOLE_PRINT("[LTE] : %s\r\n", response);
                    app_lteData.messageRecOngoing = false;
                    app_lteData.messagePubOngoing = false;
                }
                else if(strstr((char *)response, ">") != NULL)
                {   // publish topic message after prompt
                    send_cmd(mqttMessageString);
                }
                else if(strstr((char *)response, "OK") != NULL)
                {
                    app_lteData.messageRecOngoing = false;
                    app_lteData.messagePubOngoing = false;
                }
            }
            else if(   (SYS_TIME_DelayIsComplete(timer) == true)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // increase and publish LTE module counter
                app_lteData.mqttMessCntLtem++;
                sprintf(mqttMessageString, "{\"mqttMessCntLtem\":%d}\r", app_lteData.mqttMessCntLtem);
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);

                if(timer != SYS_TIME_HANDLE_INVALID)
                {
                    SYS_TIME_TimerDestroy(timer);
                }
                // resend after delay
                SYS_TIME_DelayMS(60000, &timer);
            }
            else if(   (app_lteData.daliLightStatus != app_lteData.daliLightStatusPublished)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // publish DALI light status
                app_lteData.daliLightStatusPublished = app_lteData.daliLightStatus;
                sprintf(mqttMessageString, "{\"dali_light\":%d}\r", app_lteData.daliLightStatus);
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);
            }
            else if(   (app_lteData.daliLightIntensity != app_lteData.daliLightIntensityPublished)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // publish DALI light intensity
                app_lteData.daliLightIntensityPublished = app_lteData.daliLightIntensity;
                sprintf(mqttMessageString, "{\"dali_intensity\":%d}\r", app_lteData.daliLightIntensity);
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);
            }
            else if(   (bleSensorData.rgbOnOffStatus != app_lteData.rgbOnOffStatusPublished)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // publish RGB status
                app_lteData.rgbOnOffStatusPublished = bleSensorData.rgbOnOffStatus;
                sprintf(mqttMessageString, "{\"rgb_status\":%d}\r", bleSensorData.rgbOnOffStatus);
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);
            }
            else if(   (bleSensorData.RGB_color.Hue != app_lteData.rgbColorPublished)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // publish RGB color
                app_lteData.rgbColorPublished = bleSensorData.RGB_color.Hue;
                sprintf(mqttMessageString, "{\"rgb_color\":\"0x%x\"}\r", bleSensorData.RGB_color.Hue);
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);
            }
            else if(   (EXT_GPIO_Get() != app_lteData.extGpioPublished)
                    && (app_lteData.messageRecOngoing == false)
                    && (app_lteData.messagePubOngoing == false))
            {
                // publish GPIO status
                app_lteData.extGpioPublished = EXT_GPIO_Get();
                sprintf(mqttMessageString, "{\"ext_gpio\":%ld}\r", EXT_GPIO_Get());
                sprintf(atCmdPubTopicString, "AT+SQNSMQTTPUBLISH=0,\"%s\",0,%d\r", app_lteData.mqttPubTopic, strlen((char *)mqttMessageString) - 1);
                app_lteData.messagePubOngoing = true;
                send_cmd(atCmdPubTopicString);
            }
            break;
        }

        case APP_LTE_STATE_NOT_CONNECTED:
        case APP_LTE_STATE_DISABLED:
        {   // do nothing
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            break;
        }
    }
}

/* *****************************************************************************
 End of File
 */
