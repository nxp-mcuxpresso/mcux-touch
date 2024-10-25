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

#ifndef _NT_MODULE_CS_PRV_H_
#define _NT_MODULE_CS_PRV_H_

/**
 * \defgroup cs_private CS module
 * \ingroup modules_private
 *
 * The Capacitive sense (CS) module describes the hardware configuration and control of elementary functionality
 * of the CS peripheral, it covers all versions of the CS peripheral by a generic
 * low-level driver API.
 *
 * The CS module is designed for processors that have a hardware CS module.
 * \{
 */
#include "../source/drivers/cs/nt_drv_cs_driver.h"
#include "../source/electrodes/nt_electrodes_prv.h"
#include "nt_modules.h"
#include "nt_types.h"
#include "nt_electrodes.h"
#include "../source/filters/nt_filters_prv.h"

struct nt_module_cs_data
{
    nt_cs_state_t cs_state; /*!< main NT driver data structure with state variables */
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} */ /* end of cs_private group */

#endif /* _NT_MODULE_CS_PRV_H_ */
