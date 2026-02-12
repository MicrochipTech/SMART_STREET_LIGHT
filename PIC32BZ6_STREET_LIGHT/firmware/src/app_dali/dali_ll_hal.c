/*M>---------------------------------------------------------------------------
 * Project:       DALI-Stack HAL
 * Description:   Abstraction Layer between DALI-low-level driver and uC Timer, Interrupts, etc.
 *
 * Copyright (c) by mbs GmbH, Krefeld, info@mbs-software.de
 * All rights reserved.
 --------------------------------------------------------------------------<M*/
#ifndef WITHOUT_DALI

#include "dali_ll_hal.h"
#include "definitions.h"

#define TIMESCALE 32

uint32_t dalill_getCurrentTimerVal(dalill_bus_t* pDalill_bus)
{
    return TC0_Timer32bitCounterGet() / TIMESCALE; // to get microseconds
}

uint32_t dalill_getTimerPeriod(dalill_bus_t* pDalill_bus)
{
    return (TC0_Timer32bitPeriodGet() / TIMESCALE) + 1;
}

void dalill_startTimer()
{
	TC0_TimerStart();
}

void dalill_setTimerPeriod(uint16_t uPeriod,dalill_bus_t* pDalill_bus)
{
    TC0_Timer32bitPeriodSet((((uint32_t) uPeriod + 1l) * TIMESCALE) + TC0_Timer32bitCounterGet());
}

uint8_t dalill_getBusState()
{
    return DALI_RX_Get()?0:1;
}

void dalill_setBusStateHigh()
{
    DALI_TX_Clear();
}

void dalill_setBusStateLow()
{
    DALI_TX_Set();
}

uint8_t dalill_getButtonState()
{
	//return SWITCH1_Get()?0:1;
    return 0;
}

#endif /* WITHOUT_DALI */
