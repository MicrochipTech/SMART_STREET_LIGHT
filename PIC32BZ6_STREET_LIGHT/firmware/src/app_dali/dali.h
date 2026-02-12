/*M>---------------------------------------------------------------------------
 * Project:       DALI-Stack Main-Header
 *
 * Copyright (c) by mbs GmbH, Krefeld, info@mbs-software.de
 * All rights reserved.
 --------------------------------------------------------------------------<M*/

#ifndef DALI_H_
#define DALI_H_

#ifndef WITHOUT_DALI

#include "dali_ll_hal.h"
#include "libdali.h"
#include "stdint.h"
#include <string.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

typedef enum
{
    APP_ACTION_CTRL_GEAR_SET_GEAR_FAILURE = 0,
    APP_ACTION_CTRL_GEAR_SET_LAMPFAILURE,
    APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_STATUS,
    APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_FADE_TIME,
    APP_ACTION_DALILIB_CTRL_GEAR_ACTION_QUERY_LEVEL,
    APP_ACTION_DALILIB_CTRL_GEAR_ACTION_LEVEL_OFF,
    APP_ACTION_DALILIB_CTRL_GEAR_ACTION_SET_LEVEL
} APP_ACTION;

extern dalilib_cfg_t       ctrl_device_config;

void dali_init(dalill_bus_t* pDalill_bus);
void push_button_interrupt(void);
void dali_main(void);
void app_action(APP_ACTION appAction);

#endif /* WITHOUT_DALI */

#endif /* DALI_H_ */
