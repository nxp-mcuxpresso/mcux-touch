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

#ifndef _NT_FILTERS_H_
#define _NT_FILTERS_H_

#include "nt_types.h"

/**
 * \defgroup filter Filters
 * \ingroup ntapi
 * The filters data structure that is used in the NXP Touch library.
 *
 * \{
 */

#define NT_FILTER_MOVING_AVERAGE_MAX_ORDER 15

/* forward declaration */

/**
 * The butterworth filter input parameters.
 *
 */
struct nt_filter_fbutt
{
    int32_t cutoff; /*!< The coefficient for the implemented butterworth filter polynomial. */
};

/**
 * The IIR filter input parameters.
 *
 */
struct nt_filter_iir
{
    uint8_t coef1; /*!< Scale of the current and previous signals. When the coef is higher, the current signal has less
                      strength. */
};

/**
 * The Smooth filter input parameters.
 *
 */
struct nt_filter_smooth
{
    int16_t alpha;  /*! The Alpha coefficient calculated from time period and Tau parameter of Smooht filter */
    int16_t alpha1; /*! The Alpha1 auxiliary coefficient to simplify calculations given by expression: 32768 - alpha  */    
};
struct nt_filter_asym_smooth
{
    int16_t alpha_up;  /*! The Alpha coefficient calculated from time period and Tau parameter of Smooht filter */
    int16_t alpha_up1; /*! The Alpha1 auxiliary coefficient to simplify calculations given by expression: 32768 - alpha  */    
    int16_t alpha_down;  /*! The Alpha coefficient calculated from time period and Tau parameter of Smooht filter */
    int16_t alpha_down1; /*! The Alpha1 auxiliary coefficient to simplify calculations given by expression: 32768 - alpha  */    
};
/**
 * The DC tracker filter input parameters.
 *
 */
struct nt_filter_dctracker
{
    uint8_t rate; /*!< Rate of how fast the baseline is updated. The rate should be defined as a modulo of the system
                     period. */
};

/**
 * The moving average filter input parameters.
 *
 */
struct nt_filter_moving_average
{
    int32_t n2_order; /*!< The order 2^n moving average filter */
};

/** \} */ /* end of filters group*/

#endif
