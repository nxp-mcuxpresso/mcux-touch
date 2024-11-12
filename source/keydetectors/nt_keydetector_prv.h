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

#ifndef _NT_KEYDETECTOR_PRV_H_
#define _NT_KEYDETECTOR_PRV_H_

/**
 * \defgroup detectors_private Key Detectors
 * \ingroup ntapi_private
 * The key detector module determines, whether an electrode has been touched or released, based on the values
 * obtained by the capacitive sensing layer. Along with this detection, the key detector module uses a
 * debounce algorithm that prevents the library from false touches.
 * The key detector also detects, reports, and acts on fault conditions during the scanning process. Two main
 * fault conditions are identified according to the electrode short-circuited either to the supply voltage or to the
 * ground. The same conditions can be caused by a small capacitance (equal to a short circuit to supply voltage) or by a
 * big capacitance (equals to a short circuit to the ground).
 * \{
 */

/**
 * The key detector optional run-time data.
 *
 */
union nt_keydetector_data
{
    struct nt_keydetector_afid_data *afid;   /**< AFID electrode run-time data */
    struct nt_keydetector_safa_data *safa;   /**< SAFA electrode run-time data */
    struct nt_keydetector_usafa_data *usafa; /**< USAFA electrode run-time data */
    struct nt_keydetector_mbw_data *mbw;     /**< MBW electrode run-time data */    
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/** \} */ /* end of detectors_private group */

#endif /* _NT_KEYDETECTOR_PRV_H_*/
