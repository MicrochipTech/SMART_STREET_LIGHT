/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "definitions.h"
#include "device.h"
#include "peripheral/adchs/plib_adchs.h"

// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************

/*** FUSERID ***/
#pragma config USER_ID =      0xffffU

/*** DEVCFG0 ***/
#pragma config SWOEN =      ON
#pragma config TROEN =      OFF
#pragma config ADCPOVR =      HW
#pragma config CPENFILT =      OFF
#pragma config RTCIN0_ALTEN =      PA3
#pragma config RTCOUT_ALTEN =      PA4
#pragma config USBSSEN =      0
#pragma config EPLLHWMD =      OFF
#pragma config UPLLHWMD =      OFF
#pragma config HPLUGDIS =      OFF
#pragma config USBIDIO =      1
#pragma config VBUSONIO =      1
#pragma config SMBUS3EN2 =      0
#pragma config PCM =      SFR
#pragma config INT0E =      OFF
#pragma config INT0P =      NEG
#pragma config FECCCON =         OFF
#pragma config FRECCDIS =      OFF
#pragma config CMEN =      1



/*** DEVCFG1 ***/
#pragma config CAN0_PIN_EN =      0
#pragma config TRCEN =      ON
#pragma config ZBTWKSYS =      OFF
#pragma config CMP0_OE =      OFF
#pragma config CMP1_OE =      OFF
#pragma config SMCLR =      NO_POR
#pragma config QSCHE_EN =      OFF
#pragma config QSPI_HSEN =      PPS
#pragma config SCOM0_HSEN =      DIRECT
#pragma config SCOM1_HSEN =      DIRECT
#pragma config SCOM2_HSEN =      PPS
#pragma config CCL_OE =      OFF
#pragma config WDTPSS =      PSS1048576
#pragma config QSPIDDRM =      OFF
#pragma config CLKZBREF =      OFF
#pragma config USBDMFTRIM =      DEFAULT
#pragma config USBDMRTRIM =      DEFAULT
#pragma config USBDPFTRIM =      DEFAULT
#pragma config USBDPRTRIM =      DEFAULT
#pragma config CAN1_PIN_EN =      0

/*** DEVCFG2 ***/
#pragma config ACMP_CYCLE =      _32US
#pragma config DMTINTV =      WIN_127_128
#pragma config PMUTEST_VDD_EN =      VDD_DIV_2
#pragma config POSCMOD =      HS
#pragma config WDTRMCS =      LPRC
#pragma config SOSCSEL =      XTAL
#pragma config WAKE2SPD =      ON
#pragma config CKSWEN =      ON
#pragma config FSCMEN =      ON
#pragma config WDTPSR =      PS2048
#pragma config WDTSPGM =      STOPS
#pragma config WINDIS =      NORMAL
#pragma config WDTEN =      ON
#pragma config WDTWINSZ =    WINSZ_25
#pragma config DMTCNT =      DMT31
#pragma config DMTEN =      OFF


/*** DEVCFG4 ***/
#pragma config SOSCCFG =    0x0U
#pragma config RTCEVENT_SEL =      ONE_SEC
#pragma config RTCEVENT_EN =      OFF
#pragma config VBKP_1KCSEL =      _32K
#pragma config VBKP_32KCSEL =      FRC
#pragma config VBKP_DIVSEL =      DIV_31_25
#pragma config LPCLK_MOD =      DIV_1
#pragma config RTCEVTYPE =      OUT
#pragma config CPEN_DLY =      _8_LPRC
#pragma config DSZPBOREN =      OFF
#pragma config DSWDTPS =      DSPS5
#pragma config DSWDTOSC =    LPRC
#pragma config DSWDTEN =      OFF
#pragma config DSEN =    OFF
#pragma config UVREGROVR =      CONTROLLED
#pragma config LPOSCEN =      OFF
#pragma config RTCNTM_CSEL =      RAW

/*** FBCFG0 ***/
#pragma config BINFOVALID =      VALID
#pragma config PCSCMODE =      DUAL

/*** FCPN0 ***/
#pragma config CP =      DISABLED





// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************
/* Following MISRA-C rules are deviated in the below code block */
/* MISRA C-2012 Rule 7.2 - Deviation record ID - H3_MISRAC_2012_R_7_2_DR_1 */
/* MISRA C-2012 Rule 11.1 - Deviation record ID - H3_MISRAC_2012_R_11_1_DR_1 */
/* MISRA C-2012 Rule 11.3 - Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
/* MISRA C-2012 Rule 11.8 - Deviation record ID - H3_MISRAC_2012_R_11_8_DR_1 */
// <editor-fold defaultstate="collapsed" desc="DRV_USART Instance 1 Initialization Data">

static DRV_USART_CLIENT_OBJ drvUSART1ClientObjPool[DRV_USART_CLIENTS_NUMBER_IDX1];

/* USART transmit/receive transfer objects pool */
static DRV_USART_BUFFER_OBJ drvUSART1BufferObjPool[DRV_USART_QUEUE_SIZE_IDX1];

static const DRV_USART_PLIB_INTERFACE drvUsart1PlibAPI = {
    .readCallbackRegister = (DRV_USART_PLIB_READ_CALLBACK_REG)SERCOM5_USART_ReadCallbackRegister,
    .read_t = (DRV_USART_PLIB_READ)SERCOM5_USART_Read,
    .readIsBusy = (DRV_USART_PLIB_READ_IS_BUSY)SERCOM5_USART_ReadIsBusy,
    .readCountGet = (DRV_USART_PLIB_READ_COUNT_GET)SERCOM5_USART_ReadCountGet,
    .readAbort = (DRV_USART_PLIB_READ_ABORT)SERCOM5_USART_ReadAbort,
    .writeCallbackRegister = (DRV_USART_PLIB_WRITE_CALLBACK_REG)SERCOM5_USART_WriteCallbackRegister,
    .write_t = (DRV_USART_PLIB_WRITE)SERCOM5_USART_Write,
    .writeIsBusy = (DRV_USART_PLIB_WRITE_IS_BUSY)SERCOM5_USART_WriteIsBusy,
    .writeCountGet = (DRV_USART_PLIB_WRITE_COUNT_GET)SERCOM5_USART_WriteCountGet,
    .errorGet = (DRV_USART_PLIB_ERROR_GET)SERCOM5_USART_ErrorGet,
    .serialSetup = (DRV_USART_PLIB_SERIAL_SETUP)SERCOM5_USART_SerialSetup
};

static const uint32_t drvUsart1remapDataWidth[] = { 0x5, 0x6, 0x7, 0x0, 0x1 };
static const uint32_t drvUsart1remapParity[] = { 0x2, 0x0, 0x80000, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU };
static const uint32_t drvUsart1remapStopBits[] = { 0x0, 0xFFFFFFFFU, 0x40 };
static const uint32_t drvUsart1remapError[] = { 0x4, 0x0, 0x2 };

static const DRV_USART_INTERRUPT_SOURCES drvUSART1InterruptSources =
{
    /* Peripheral has single interrupt vector */
    .isSingleIntSrc                        = true,

    /* Peripheral interrupt line */
    .intSources.usartInterrupt             = (int32_t)SERCOM5_IRQn,
};

static const DRV_USART_INIT drvUsart1InitData =
{
    .usartPlib = &drvUsart1PlibAPI,

    /* USART Number of clients */
    .numClients = DRV_USART_CLIENTS_NUMBER_IDX1,

    /* USART Client Objects Pool */
    .clientObjPool = (uintptr_t)&drvUSART1ClientObjPool[0],

    /* Combined size of transmit and receive buffer objects */
    .bufferObjPoolSize = DRV_USART_QUEUE_SIZE_IDX1,

    /* USART transmit and receive buffer buffer objects pool */
    .bufferObjPool = (uintptr_t)&drvUSART1BufferObjPool[0],

    .interruptSources = &drvUSART1InterruptSources,

    .remapDataWidth = drvUsart1remapDataWidth,

    .remapParity = drvUsart1remapParity,

    .remapStopBits = drvUsart1remapStopBits,

    .remapError = drvUsart1remapError,

    .dataWidth = DRV_USART_DATA_8_BIT,
};

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="DRV_USART Instance 0 Initialization Data">

static DRV_USART_CLIENT_OBJ drvUSART0ClientObjPool[DRV_USART_CLIENTS_NUMBER_IDX0];

/* USART transmit/receive transfer objects pool */
static DRV_USART_BUFFER_OBJ drvUSART0BufferObjPool[DRV_USART_QUEUE_SIZE_IDX0];

static const DRV_USART_PLIB_INTERFACE drvUsart0PlibAPI = {
    .readCallbackRegister = (DRV_USART_PLIB_READ_CALLBACK_REG)SERCOM1_USART_ReadCallbackRegister,
    .read_t = (DRV_USART_PLIB_READ)SERCOM1_USART_Read,
    .readIsBusy = (DRV_USART_PLIB_READ_IS_BUSY)SERCOM1_USART_ReadIsBusy,
    .readCountGet = (DRV_USART_PLIB_READ_COUNT_GET)SERCOM1_USART_ReadCountGet,
    .readAbort = (DRV_USART_PLIB_READ_ABORT)SERCOM1_USART_ReadAbort,
    .writeCallbackRegister = (DRV_USART_PLIB_WRITE_CALLBACK_REG)SERCOM1_USART_WriteCallbackRegister,
    .write_t = (DRV_USART_PLIB_WRITE)SERCOM1_USART_Write,
    .writeIsBusy = (DRV_USART_PLIB_WRITE_IS_BUSY)SERCOM1_USART_WriteIsBusy,
    .writeCountGet = (DRV_USART_PLIB_WRITE_COUNT_GET)SERCOM1_USART_WriteCountGet,
    .errorGet = (DRV_USART_PLIB_ERROR_GET)SERCOM1_USART_ErrorGet,
    .serialSetup = (DRV_USART_PLIB_SERIAL_SETUP)SERCOM1_USART_SerialSetup
};

static const uint32_t drvUsart0remapDataWidth[] = { 0x5, 0x6, 0x7, 0x0, 0x1 };
static const uint32_t drvUsart0remapParity[] = { 0x2, 0x0, 0x80000, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU };
static const uint32_t drvUsart0remapStopBits[] = { 0x0, 0xFFFFFFFFU, 0x40 };
static const uint32_t drvUsart0remapError[] = { 0x4, 0x0, 0x2 };

static const DRV_USART_INTERRUPT_SOURCES drvUSART0InterruptSources =
{
    /* Peripheral has single interrupt vector */
    .isSingleIntSrc                        = true,

    /* Peripheral interrupt line */
    .intSources.usartInterrupt             = (int32_t)SERCOM1_IRQn,
    /* DMA Tx interrupt line */
    .intSources.multi.dmaTxChannelInt      = (int32_t)DMAC_0_3_IRQn,
    /* DMA Rx interrupt line */
    .intSources.multi.dmaRxChannelInt      = (int32_t)DMAC_0_3_IRQn,
};

static const DRV_USART_INIT drvUsart0InitData =
{
    .usartPlib = &drvUsart0PlibAPI,

    /* USART Number of clients */
    .numClients = DRV_USART_CLIENTS_NUMBER_IDX0,

    /* USART Client Objects Pool */
    .clientObjPool = (uintptr_t)&drvUSART0ClientObjPool[0],

    .dmaChannelTransmit = DRV_USART_XMIT_DMA_CH_IDX0,

    .usartTransmitAddress = (void *)&(SERCOM1_REGS->USART_INT.SERCOM_DATA),

    .dmaChannelReceive = DRV_USART_RCV_DMA_CH_IDX0,

    .usartReceiveAddress = (void *)&(SERCOM1_REGS->USART_INT.SERCOM_DATA),

    /* Combined size of transmit and receive buffer objects */
    .bufferObjPoolSize = DRV_USART_QUEUE_SIZE_IDX0,

    /* USART transmit and receive buffer buffer objects pool */
    .bufferObjPool = (uintptr_t)&drvUSART0BufferObjPool[0],

    .interruptSources = &drvUSART0InterruptSources,

    .remapDataWidth = drvUsart0remapDataWidth,

    .remapParity = drvUsart0remapParity,

    .remapStopBits = drvUsart0remapStopBits,

    .remapError = drvUsart0remapError,

    .dataWidth = DRV_USART_DATA_8_BIT,
};

// </editor-fold>



// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************
/* Structure to hold the object handles for the modules in the system. */
SYSTEM_OBJECTS sysObj;

// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************
OSAL_SEM_HANDLE_TYPE semPhyInternalHandler;
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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

OSAL_API_LIST_TYPE     osalAPIList;


#define QUEUE_LENGTH_BLE        (32)
#define QUEUE_ITEM_SIZE_BLE     (sizeof(void *))
#define EXT_COMMON_MEMORY_SIZE  (31*1024)
OSAL_QUEUE_HANDLE_TYPE bleRequestQueueHandle;

/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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

OSAL_QUEUE_HANDLE_TYPE OTQueue;
#define OT_TASK_QUEUE_SIZE 128


// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************
// <editor-fold defaultstate="collapsed" desc="SYS_TIME Initialization Data">

static const SYS_TIME_PLIB_INTERFACE sysTimePlibAPI = {
    .timerCallbackSet = (SYS_TIME_PLIB_CALLBACK_REGISTER)RTC_Timer32CallbackRegister,
    .timerStart = (SYS_TIME_PLIB_START)RTC_Timer32Start,
    .timerStop = (SYS_TIME_PLIB_STOP)RTC_Timer32Stop,
    .timerFrequencyGet = (SYS_TIME_PLIB_FREQUENCY_GET)RTC_Timer32FrequencyGet,
    .timerPeriodSet = (SYS_TIME_PLIB_PERIOD_SET)NULL,
};

static const SYS_TIME_INIT sysTimeInitData =
{
    .timePlib = &sysTimePlibAPI,
    .hwTimerIntNum = RTC_IRQn,
};

// </editor-fold>
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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




// <editor-fold defaultstate="collapsed" desc="SYS_CONSOLE Instance 0 Initialization Data">


static const SYS_CONSOLE_UART_PLIB_INTERFACE sysConsole0UARTPlibAPI =
{
    .read_t = (SYS_CONSOLE_UART_PLIB_READ)SERCOM0_USART_Read,
    .readCountGet = (SYS_CONSOLE_UART_PLIB_READ_COUNT_GET)SERCOM0_USART_ReadCountGet,
    .readFreeBufferCountGet = (SYS_CONSOLE_UART_PLIB_READ_FREE_BUFFFER_COUNT_GET)SERCOM0_USART_ReadFreeBufferCountGet,
    .write_t = (SYS_CONSOLE_UART_PLIB_WRITE)SERCOM0_USART_Write,
    .writeCountGet = (SYS_CONSOLE_UART_PLIB_WRITE_COUNT_GET)SERCOM0_USART_WriteCountGet,
    .writeFreeBufferCountGet = (SYS_CONSOLE_UART_PLIB_WRITE_FREE_BUFFER_COUNT_GET)SERCOM0_USART_WriteFreeBufferCountGet,
};

static const SYS_CONSOLE_UART_INIT_DATA sysConsole0UARTInitData =
{
    .uartPLIB = &sysConsole0UARTPlibAPI,
};

static const SYS_CONSOLE_INIT sysConsole0Init =
{
    .deviceInitData = (const void*)&sysConsole0UARTInitData,
    .consDevDesc = &sysConsoleUARTDevDesc,
    .deviceIndex = 0,
};



// </editor-fold>


static const SYS_CMD_INIT sysCmdInit =
{
    .moduleInit = {0},
    .consoleCmdIOParam = (uint8_t) SYS_CMD_SINGLE_CHARACTER_READ_CONSOLE_IO_PARAM,
	.consoleIndex = 0,
};




// *****************************************************************************
// *****************************************************************************
// Section: Local initialization functions
// *****************************************************************************
// *****************************************************************************

/* MISRAC 2012 deviation block end */

/*******************************************************************************
  Function:
    void SYS_Initialize ( void *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
 */

void SYS_Initialize ( void* data )
{

    /* MISRAC 2012 deviation block start */
    /* MISRA C-2012 Rule 2.2 deviated in this file.  Deviation record ID -  H3_MISRAC_2012_R_2_2_DR_1 */

/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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

    // Initialize the RF Clock Generator
    SYS_ClkGen_Config();

    BT_SYS_Cfg_T        btSysCfg;
    BT_SYS_Option_T     btOption;
  
    CLOCK_Initialize();



	GPIO_Initialize();

    TC0_TimerInitialize();

    RTC_Initialize();

    NVM_Initialize();

    ADCHS_Initialize();

    SERCOM1_USART_Initialize();

    EVSYS_Initialize();

    SERCOM0_USART_Initialize();

    DMAC_Initialize();

    SERCOM5_USART_Initialize();

    EIC_Initialize();

    TCC0_CompareInitialize();


    /* MISRAC 2012 deviation block start */
    /* Following MISRA-C rules deviated in this block  */
    /* MISRA C-2012 Rule 11.3 - Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* MISRA C-2012 Rule 11.8 - Deviation record ID - H3_MISRAC_2012_R_11_8_DR_1 */

    // Initialize RF System
    SYS_Load_Cal(WSS_ENABLE_BLE_ZB);
 
    // Set up OSAL for RF Stack Library usage
    osalAPIList.OSAL_CRIT_Enter      = OSAL_CRIT_Enter;
    osalAPIList.OSAL_CRIT_Leave      = OSAL_CRIT_Leave;

    osalAPIList.OSAL_SEM_Create      = OSAL_SEM_Create;
    osalAPIList.OSAL_SEM_Pend        = OSAL_SEM_Pend;
    osalAPIList.OSAL_SEM_Post        = OSAL_SEM_Post;
    osalAPIList.OSAL_SEM_PostISR     = OSAL_SEM_PostISR;
    osalAPIList.OSAL_SEM_GetCount    = OSAL_SEM_GetCount;

    osalAPIList.OSAL_QUEUE_Create    = OSAL_QUEUE_Create;
    osalAPIList.OSAL_QUEUE_Send      = OSAL_QUEUE_Send;
    osalAPIList.OSAL_QUEUE_SendISR   = OSAL_QUEUE_SendISR;
    osalAPIList.OSAL_QUEUE_Receive   = OSAL_QUEUE_Receive;
    osalAPIList.OSAL_QUEUE_IsFullISR = OSAL_QUEUE_IsFullISR;
    osalAPIList.OSAL_QUEUE_CreateSet = OSAL_QUEUE_CreateSet;
    osalAPIList.OSAL_QUEUE_AddToSet  = OSAL_QUEUE_AddToSet;
    osalAPIList.OSAL_QUEUE_SelectFromSet = OSAL_QUEUE_SelectFromSet;

    osalAPIList.OSAL_MemAlloc = OSAL_Malloc;
    osalAPIList.OSAL_MemFree = OSAL_Free;


/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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
    // Initialize PDS- Persistent Data Server
    PDS_Init(MAX_PDS_ITEMS_COUNT, MAX_PDS_DIRECTORIES_COUNT);


    sysObj.drvUsart1 = DRV_USART_Initialize(DRV_USART_INDEX_1, (SYS_MODULE_INIT *)&drvUsart1InitData);

    sysObj.drvUsart0 = DRV_USART_Initialize(DRV_USART_INDEX_0, (SYS_MODULE_INIT *)&drvUsart0InitData);


    /* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -  
    H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/
        
    sysObj.sysTime = SYS_TIME_Initialize(SYS_TIME_INDEX_0, (SYS_MODULE_INIT *)&sysTimeInitData);
    
    /* MISRAC 2012 deviation block end */
    /* Initialization for IEEE_802154_PHY */
    OSAL_SEM_Create(&semPhyInternalHandler, OSAL_SEM_TYPE_COUNTING, 20, 0);

    PHY_Init();
    
    /* End of Initialization for IEEE_802154_PHY */

    /* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -  
     H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/
    sysObj.sysConsole0 = SYS_CONSOLE_Initialize(SYS_CONSOLE_INDEX_0, (SYS_MODULE_INIT *)&sysConsole0Init);
   /* MISRAC 2012 deviation block end */
    sysObj.sysCommand = (uint32_t) SYS_CMD_Initialize((SYS_MODULE_INIT*)&sysCmdInit);


    // Create BLE Stack Message QUEUE
    OSAL_QUEUE_Create(&bleRequestQueueHandle, QUEUE_LENGTH_BLE, QUEUE_ITEM_SIZE_BLE);

    // Retrieve BLE calibration data
    (void)memset(&btSysCfg, 0, sizeof(BT_SYS_Cfg_T));
    btSysCfg.addrValid = IB_GetBdAddr(&btSysCfg.devAddr[0]);
    btSysCfg.rssiOffsetValid =IB_GetRssiOffset(&btSysCfg.rssiOffset);

    if (!IB_GetAntennaGain(&btSysCfg.antennaGain))
    {
        btSysCfg.antennaGain = 3;
    }

    btSysCfg.adcTimingValid =IB_GetAdcTiming(&btSysCfg.adcTiming08, &btSysCfg.adcTiming51);

    //Configure BLE option
    (void)memset(&btOption, 0, sizeof(BT_SYS_Option_T));
    btOption.hciMode = false;
    btOption.cmnMemSize = EXT_COMMON_MEMORY_SIZE;
    //Configure BLE option
    btOption.p_cmnMemAddr = OSAL_Malloc(btOption.cmnMemSize);
    btOption.deFeatMask = 0;

    // Initialize BLE Stack
    BT_SYS_Init(&bleRequestQueueHandle, &osalAPIList, &btOption, &btSysCfg);

    /*Open Thread System Initialization*/
    otSysInit(0U, 0U);
    
    /* Creation of openthread Task Queue */
    OSAL_QUEUE_Create(&OTQueue, OT_TASK_QUEUE_SIZE, sizeof(OT_Msg_T));


    /* MISRAC 2012 deviation block end */
    APP_Initialize();
    APP_LORA_Initialize();
#ifndef WITHOUT_DALI
    APP_DALI_Initialize();
#endif
    APP_LTE_Initialize();

    NVIC_Initialize();


    /* MISRAC 2012 deviation block end */
}

/*******************************************************************************
 End of File
*/
