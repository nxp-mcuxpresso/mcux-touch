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

#include "fsl_device_registers.h"
#include "nt_drv_tsi_driver.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Table of base addresses for tsi instances. */
TSI_Type *const g_tsiBase[] = TSI_BASE_PTRS;

/* Table to save TSI IRQ numbers defined in CMSIS files. */
#if (FSL_FEATURE_TSI_VERSION == 6)
const IRQn_Type g_tsiIrqId[FSL_FEATURE_SOC_TSI_COUNT] = {TSI_END_OF_SCAN_IRQn};
#elif (FSL_FEATURE_TSI_VERSION < 5 || defined(MKE17Z9_SERIES) ||defined(MKE17Z7_SERIES) || defined(MKE13Z7_SERIES) || defined(MKE12Z7_SERIES))
const IRQn_Type g_tsiIrqId[FSL_FEATURE_SOC_TSI_COUNT] = {TSI0_IRQn};
#else
const IRQn_Type g_tsiIrqId[FSL_FEATURE_SOC_TSI_COUNT] = {TSI_IRQn};
#endif

/* Pointer to tsi runtime state structure.*/
nt_tsi_state_t *g_tsiStatePtr[FSL_FEATURE_SOC_TSI_COUNT] = {NULL};

/*******************************************************************************
 * EOF
 ******************************************************************************/
