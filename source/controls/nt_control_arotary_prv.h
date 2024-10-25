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

#ifndef _NT_AROTARY_PRV_H_
#define _NT_AROTARY_PRV_H_

/**
 * \defgroup arotary_private Analog Rotary Control
 * \ingroup controls_private
 *
 * Analog Rotary enables the detection of jog-dial-like finger movement using three or more electrodes;
 * it is represented by the nt_control structure.
 *
 * An Analog Rotary Control uses three or more specially-shaped electrodes to enable
 * the calculation of finger position within a circular area. The position algorithm uses
 * a ratio of sibling electrode signals to estimate the finger position with required precision.
 *
 * The Analog Rotary works similarly to the "standard" Rotary, but requires less number
 * of electrodes, while achieving a higher resolution of the calculated position. For
 * example, a four-electrode analog rotary can provide finger position detection in the
 * range of 0-64. The shape of the electrodes needs to be designed specifically to achieve
 * stable signal with a linear dependence on the finger movement.
 *
 * The Analog Rotary Control provides Position, Direction, and Displacement values. It
 * is able to generate event callbacks when finger Movement, Initial-touch, or Release is detected.
 *
 * The image below shows a typical four-electrode Analog Rotary electrode placement.
 * \image html arotary.png "Analog Rotary Electrodes"
 * \image latex arotary.png "Analog Rotary Electrodes"
 *
 * \{
 */

/**
 * Value that is used to mark an invalid position of the Analog Rotary.
 */
#define NT_AROTARY_INVALID_POSITION_VALUE 65535U

/**
 * Analog Rotary flags.
 */
enum nt_control_arotary_flags
{
    NT_AROTARY_INVALID_POSITION_FLAG = 1 << NT_FLAGS_SPECIFIC_SHIFT(0), /*!< Analog Rotary invalid position flag. */
    NT_AROTARY_DIRECTION_FLAG        = 1 << NT_FLAGS_SPECIFIC_SHIFT(1), /*!< Analog Rotary direction flag. */
    NT_AROTARY_MOVEMENT_FLAG         = 1 << NT_FLAGS_SPECIFIC_SHIFT(2), /*!< Analog Rotary movement flag.*/
    NT_AROTARY_TOUCH_FLAG            = 1 << NT_FLAGS_SPECIFIC_SHIFT(3), /*!< Analog Rotary touch flag. */
};

/**
 *  Analog Rotary help structure to handle temporary values
 */
struct nt_control_arotary_temp_data
{
    uint32_t active_el_ix; /*!< Index of electrode of first active electrode. */
    uint32_t first_delta;  /*!< Value of first delta (signal - baseline). */
    uint32_t range;        /*!< Value of first delta (signal - baseline). */
};

/**
 *  Analog Rotary RAM structure used to store volatile parameters of the control.
 *
 *  You need to allocate this structure and put a pointer into the nt_control
 *  structure when it is being registered in the system.
 */
struct nt_control_arotary_data
{
    nt_control_arotary_callback callback; /*!< Analog Rotary callback handler. */
    uint8_t position;                     /*!< Position. */
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} end of arotary_private group */

#endif /* _NT_AROTARY_PRV_H_ */
