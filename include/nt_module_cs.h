/*
* Copyright 2013-2016, Freescale Semiconductor, Inc.
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

#ifndef _NT_MODULE_CS_H_
#define _NT_MODULE_CS_H_

/**
 * \defgroup cs CS module
 * \ingroup modules
 *
 * The CS module describes the hardware configuration and control of the elementary functionality
 * of the CS peripheral; it covers all versions of the CS peripheral by a generic
 * low-level driver API.
 * \{
 */

#include "nt_modules.h"

#include "nt_types.h"
#include "nt_electrodes.h"

#include "../source/drivers/cs/nt_drv_cs_driver.h"

#define NT_MODULE_CS_NAME "nt_module_cs_interface"

/* ASM implemented macro                                                      */
#if defined(__CC_ARM)
#define CUSTOM_DELAY() \
    {                  \
        __nop();       \
    }
#else
#define CUSTOM_DELAY() \
    {                  \
        asm("NOP");    \
    }
#endif

struct nt_module_cs_params
{
    const cs_config_t *config; /*!< A pointer to the HW configuration. Can't be NULL. */
};

/**
 * The CS module interface structure.
 */
extern const struct nt_module_interface nt_module_cs_interface; /*!< Can't be NULL. */
void delay(void);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} */ /* end of the CS group */

#endif
