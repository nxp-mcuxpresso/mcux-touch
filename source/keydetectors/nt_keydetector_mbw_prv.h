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

#ifndef NT_KEYDETECTOR_MBW_PRV_H
#define NT_KEYDETECTOR_MBW_PRV_H

/**
 * \defgroup mbw_prv mbw Detector
 * \ingroup detectors_private
 *
 * The mbw key detector is a method for recognition of the touch or release states. It can be used for each type of
 * control.
 *
 * If the measured sample is reported as a valid sample, the module calculates the delta value from the actual signal
 * value and the baseline value. The delta value is compared to the threshold value computed from the expected signal
 * and baseline values. Based on the result, it determines the electrode state, which can be released, touched, changing
 * from released to touched, and changing from touched to released. The method uses moving average filters to determine
 * the baseline and expected signal values with a different depth of the filter, depending on the state of the
 * electrode. The deadband filters in the horizontal and vertical directions are also implemented.
 * \{
 */

#include "../source/filters/nt_filters_prv.h"
#include "../source/keydetectors/nt_keydetector_prv.h"
#include "nt_types.h"
#include "nt_keydetector_mbw.h"

/**
 * mbw RAM structure. This structure is used for internal
 * algorithms to store data, while evaluating mbw.
 * Contains the data of the resulting calculation and auxiliary variables.
 *
 * This structure manages and uses internal methods only.
 */
struct nt_keydetector_mbw_data
{
    enum nt_filter_state filter_state;                 /**< Input filter state. */
    struct nt_filter_moving_average_data base_avrg;    /**< Baseline moving average filter data. */
    struct nt_filter_moving_average_data noise_avrg;   /**< Noise signal moving average filter data. */
    struct nt_filter_moving_average_data f_noise_avrg; /**< Fast Noise signal moving average filter data. */
    struct nt_filter_moving_average_data predicted_signal_avrg; /**< Predicted signal line moving average filter data. */
    struct nt_filter_moving_average base_avrg_init;    /**< Baseline moving average filter settings. */
    struct nt_filter_moving_average noise_avrg_init;   /**< Noise moving average filter settings. */
    struct nt_filter_moving_average f_noise_avrg_init; /**< Fast Noise moving average filter settings. */
    struct nt_filter_moving_average predicted_signal_avrg_init; /**< Predicted signal moving average filter settings. */
    struct nt_filter_asym_smooth base_smooth;          /**< Baseline Asym Smooth filter */
    struct nt_filter_smooth base_smooth_release;       /**< Baseline Smooth filter After Release (faster) */
    struct nt_filter_smooth base_smooth_fix;           /**< Baseline 4x times faster filter for decrease (Asymetric smooth). */
    uint16_t *signal_buffer;                           /**< Shift register for raw signals (for min filtering) */
    uint32_t noise;                                    /**< Noise value. */
    uint32_t f_noise;                                  /**< Fast Noise value. */
    uint32_t predicted_signal;                         /**< Predicted signal value. */
    int32_t  entry_event_cnt;                          /**< Event counter value. */
    int32_t  deadband_cnt;                             /**< Deadband event counter. */
    int32_t  recovery_cnt;                             /**< Recovery counter. */
    uint32_t deadband_h;                               /**< Deadband high watermark */
    uint16_t baseline_cnt;                             /**< Counter for baseline tracking measure the baseline window */
    uint16_t baseline_cnt_max;                         /**< Maximum value of the baseline tracking counter calculated by baseline_track_window * system_time_period */
    uint16_t smooth_baseline_min;                      /**< Minimum value from actual baseline window used for next baseline window */
    uint16_t smooth_signal_max;                        /**< Maximum value of smoothed signal in baseline window */
    uint16_t smooth_signal_min;                        /**< Minimum value of smoothed signal in baseline window */
    uint16_t smooth_signal;                            /**< Signal filtered by the signal Smooth filter used as actual signal */
    uint16_t smooth_baseline;                          /**< Raw signal filtered by the baseline Smooth filter used for baseline */
    uint32_t smooth_baseline_accu;                     /**< Baseline asymetric smooth counter */
    uint16_t smooth_baseline_accu_count;               /**< Baseline asymetric smooth tau parameter */
    uint16_t baseline_add_no_touch;                    /**< Add to counter when no touch or number of touches > touch_limit, higher number will slowdown the baseline (by factor baseline_add_no_touch / baseline_add_touch) if no touch event */
    uint16_t baseline_add_touch;                       /**< Add to counter when touch, when touched, baseline tracker slows down by factor, (baseline_add_no_touch / baseline_add_touch) */
    int16_t  prev_delta;                               /**< Previous signed delta used for debouncing */
    uint8_t  shist;                                    /**< signal history counter*/
    uint8_t  debounce_cnt;                             /**< Counter of measured deltas bounced higher then delta limitation */
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} */ /* end of mbw private  group */

#endif
