/*M>---------------------------------------------------------------------------
 * Project:       DALI-Stack HAL-Header
 *
 * Copyright (c) by mbs GmbH, Krefeld, info@mbs-software.de
 * All rights reserved.
 --------------------------------------------------------------------------<M*/

#ifndef DALI_LL_HAL_H_
#define DALI_LL_HAL_H_

#ifndef WITHOUT_DALI

#include "stdint.h"
#include "dali_ll.h"

uint32_t dalill_getCurrentTimerVal(dalill_bus_t* pDalill_bus);
uint32_t dalill_getTimerPeriod(dalill_bus_t* pDalill_bus);
void dalill_startTimer(void);
void dalill_setTimerPeriod(uint16_t period,dalill_bus_t* pDalill_bus);

uint8_t dalill_getBusState(void);
uint8_t dalill_getButtonState(void);
void dalill_setBusStateHigh(void);
void dalill_setBusStateLow(void);

#endif /* WITHOUT_DALI */

#endif /* DALI_LL_HAL_H_ */
