/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _APP_LTE_H    /* Guard against multiple inclusion */
#define _APP_LTE_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "definitions.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    // SIM APN config
    #define SIM_APN         "internet"  // Set valid SIM APN

    // SMS example parameters
    #define SIM_SMSC        ""          // Set valid SMS Service Center Address - only in SMS PDU mode
    #define PHONE_NUMBER    ""          // Set Phone number to message
    #define SMS_MODE        "1"         // SMS mode: "0" - PDU, "1" - TXT

    /**
     * @brief LTE IoT 10 control commands.
     * @details Specified setting for control commands of LTE IoT 10 Click driver.
     */
    #define LTEIOT10_CMD_AT                             "AT\r"
    #define LTEIOT10_CMD_GET_MODEL_ID                   "AT+CGMM\r"
    #define LTEIOT10_CMD_GET_SW_VERSION                 "AT+CGMR\r"
    #define LTEIOT10_CMD_GET_SERIAL_NUM                 "AT+CGSN\r"
    #define LTEIOT10_CMD_FACTORY_RESET                  "AT&F\r"
    #define LTEIOT10_CMD_HARD_RESET                     "AT^RESET\r"
    #define LTEIOT10_CMD_DISABLE_ECHO                   "ATE0\r"
    #define LTEIOT10_CMD_ENABLE_ECHO                    "ATE1\r"
    #define LTEIOT10_CMD_SET_PHONE_FUNCTIONALITY        "AT+CFUN\r"
    #define LTEIOT10_CMD_NETWORK_REGISTRATION           "AT+CEREG\r"
    #define LTEIOT10_CMD_SIGNAL_QUALITY_REPORT          "AT+CSQ\r"
    #define LTEIOT10_CMD_OPERATOR_SELECTION             "AT+COPS\r"
    #define LTEIOT10_CMD_SEND_SMS                       "AT+CMGS\r"
    #define LTEIOT10_CMD_SELECT_SMS_FORMAT              "AT+CMGF\r"
    #define LTEIOT10_CMD_DEFINE_PDP_CONTEXT             "AT+CGDCONT\r"
    #define LTEIOT10_CMD_ACTIVATE_PDP_CONTEXT           "AT+CGACT\r"
    #define LTEIOT10_CMD_SHOW_PDP_ADDRESS               "AT+CGPADDR\r"
    #define LTEIOT10_CMD_OPEN_TCP_UDP_CONNECTION        "AT+SQNSD\r"
    #define LTEIOT10_CMD_SEND_DATA_VIA_CONNECTION       "AT+SQNSSEND\r"
    #define LTEIOT10_CMD_RECEIVE_DATA_VIA_CONNECTION    "AT+SQNSRECV\r"
    #define LTEIOT10_CMD_CLOSE_TCP_UDP_CONNECTION       "AT+SQNSH"

    #define LTEIOT10_MQTT_CLOUD_URL                     "broker.hivemq.com"
    #define LTEIOT10_MQTT_SUB_TOPIC                     "endpointId/dcx/token"
    #define LTEIOT10_MQTT_PUB_TOPIC                     "endpointId/dcx/token/json"


#define APP_LTE_CMD_RX_BUFFER_SIZE  300
#define APP_LTE_MAX_CON_RETRIES 3    
#define APP_LTE_NOTIF_CALLBACK_COUNT 6
//#define APP_LTE_DEBUG_LOGS
    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

typedef enum {
    APP_LTE_STATE_INIT,
    APP_LTE_STATE_INITIALIZE,
    APP_LTE_STATE_RUNNING,
    APP_LTE_STATE_TIMEOUT,
    APP_LTE_STATE_ERROR,
    APP_LTE_STATE_NOT_CONNECTED,
    APP_LTE_STATE_DISABLED,
}APP_LTE_STATE_T;

typedef enum
{
    APP_LTE_OFF = 0,
    APP_LTE_AVAILABLE = 1,          //LTE module available
    APP_LTE_CONNECTED = 2,          //LTE Connected
    APP_LTE_MQTT_CONNECTED = 4      //MQTT Connected throught LTE
} APP_LTE_STATUS;

typedef enum
{
    APP_LTE_AVAILABLE_NOTIFY,        //LTE module available
    APP_LTE_CONNECTED_NOTIFY,        //LTE Connected
    APP_LTE_DISCONNECTED_NOTIFY,     //LTE Disconnected                        
    APP_LTE_MQTT_CONNECTED_NOTIFY,   //MQTT Connected through LTE
    APP_LTE_MQTT_DISCONNECTED_NOTIFY //MQTT DisConnected through LTE
} APP_LTE_NOTIFY_EVENT;

typedef struct
{
    void (*func) (uint32_t event, void * data, uint32_t size);
} APP_LTE_NOTIF_CALLBACK;

typedef struct {
    APP_LTE_STATE_T state;
    uint32_t status;
    uint8_t rx_buf[APP_LTE_CMD_RX_BUFFER_SIZE];
    uint8_t rx_byte;
    uint8_t rx_buf_index;
    /* MQTT cloud connection status */
    bool cloudConnected;
    /* MQTT cloud connection retries */
    uint8_t connectionRetires;
    /* MQTT message counter published over LTE-Module */
    int mqttMessCntLtem;
    /* MQTT message counter received over LTE-Module */
    int mqttRecMessCntLtem;
    /* DALI light status (off/on) published over LTE-Module */
    uint8_t daliLightStatus;
    uint8_t daliLightStatusPublished;
    /* DALI light intensity (0-100) published over LTE-Module */
    uint8_t daliLightIntensity;
    uint8_t daliLightIntensityPublished;
    /* RGB LED status (off/on) published over LTE-Module */
    uint8_t rgbOnOffStatusPublished;
    /* RGB LED color published over LTE-Module */
    uint8_t rgbColorPublished;
    /* External GPIO published over LTE-Module */
    uint8_t extGpioPublished;
    /* Message reception of topic is ongoing */
    bool messageRecOngoing;
    /* Message publishing of topic is ongoing */
    bool messagePubOngoing;
    /* LTE module enable/disable */
    bool moduleEnable;
    /* MQTT cloud URL*/
    char mqttCloudUrl[50];
    char mqttConnectCmd[80];
    /* MQTT subscription topic */
    char mqttSubTopic[80];
    char mqttSubscribeCmd[110];
    char mqttSubscribeResp[110];
    /* MQTT publication topic */
    char mqttPubTopic[80];
}APP_LTE_DATA_T;

extern APP_LTE_DATA_T app_lteData;

    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************
    extern bool APP_LTE_Initialize(void);
    extern void APP_LTE_Disable(void);
    extern void APP_LTE_Enable(void);
    extern void APP_LTE_SetParameter(char *url, char *pubTopic, char *subTopic);
    extern void APP_LTE_Tasks(void);
    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _APP_LTE_H */

/* *****************************************************************************
 End of File
 */
