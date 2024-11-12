/*
* Copyright 2013-2016 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may
* only be used strictly in accordance with the applicable license terms. 
* By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that
* you have read, and that you agree to comply with and are bound by,
* such license terms.  If you do not agree to be bound by the applicable
* license terms, then you may not retain, install, activate or otherwise
* use the software.
*/

#ifndef _NT_MODULE_GPIOINT_PRV_H_
#define _NT_MODULE_GPIOINT_PRV_H_

/**
 * \defgroup gpioint_private GPIO interrupt module
 * \ingroup modules_private
 *
 * The GPIO interrupt module describes the hardware configuration and control of the elementary functionality
 * of the method that is using standard GPIO pins of the MCU with the GPIO and timer interrupts.
 *
 * The GPIO interrupt method is designed for all general processors that have a GPIO module with interrupt capability.
 *
 * \{
 */

#include "../source/electrodes/nt_electrodes_prv.h"
#include "nt_modules.h"

#include "nt_types.h"
#include "nt_electrodes.h"
#include "../source/filters/nt_filters_prv.h"

/** GPIO interrupt module's RAM. This structure contains
 */
struct nt_module_gpioint_data
{
    uint32_t pen;         /*!< PEN - enablement of all modules' electrodes */
    uint8_t measured_pin; /*!< The currently measured pin */
    uint8_t scan_status;  /*!< Module's scanning status - see enum nt_gpio_scan_states */
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} */ /* end of gpioint_private group */

#endif /* _NT_MODULE_GPIOINT_PRV_H_*/
