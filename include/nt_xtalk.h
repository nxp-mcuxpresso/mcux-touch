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

#ifndef _NT_XTALK_H_
#define _NT_XTALK_H_

/**
 * \defgroup xtalk Crosstalk
 * \ingroup ntapi
 * Crosstalk represents the layer for the cross-talk compensation and sensing.
 * \{
 */

#include <stdint.h>
#include "nt_setup.h"
#include "../source/electrodes/nt_electrodes_prv.h"

#define NT_SYSTEM_XTALK_NAME "nt_system_xtalk_interface"
#define NT_INV_32(x)   ( (int32_t)(0x7FFFFFFF) / (int16_t)(x) )
#define NT_MUL_16(x,y) ( ( (int32_t)(x) * (int16_t)(y) ) >> (15) )

/* Forward declaration */
struct nt_system_xtalk_interface;

/** Interface structure for the cross-talk  */
struct nt_system_xtalk_params
{
/* Number of sensors used for cross-talk reduction
 * Code supports only neighbours K = 1, 2, 3, 4 */   
    uint16_t *actMat;                    /*!< Crosstalk activation matrix pointer (one dimesion array size xtalk electrode counter squared). */
    uint8_t  nt_xtalk_neighbours;        /*!< Number of sensors used for cross-talk reduction */
    uint8_t  nt_xtalk_adapt_touch_time;  /*!< Adaptation touch time */
};
/**
 * \defgroup gxtalk General API
 * \ingroup Crosstalk
 * General Function definition of the Crosstalk.
 *
 * \{
 */

/**
 * The Crosstalk interface structure.
 */
extern const struct nt_system_xtalk_interface nt_system_xtalk_interface; /*!< Can't be NULL. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Calculate improved activation level for given sensor based on measured activation vector.
 * \return none
 *  \code
    struct nt_module_data *module = _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface,
    instance));
    NT_ASSERT(module != NULL);
    nt_xtalk_reduction(module); 
  \endcode
 */
void nt_xtalk_reduction(struct nt_kernel *system);

/* Full transformation matrix calculation */
/**
 * \brief Full transformation matrix calculation
 * \return none
 *  \code
    struct nt_module_data *module = _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface,
    instance));
    NT_ASSERT(module != NULL);
    nt_xtalk_optimize_transform(module); 
  \endcode
 */
void nt_xtalk_optimize_transform(struct nt_kernel *system);

/**
 * \brief STransformation matrix calculation divided in sub-steps
 * \return none
 *  \code
    struct nt_module_data *module = _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface,
    instance));
    NT_ASSERT(module != NULL);
    nt_xtalk_optimize_transform_step(module); 
  \endcode
 */
void nt_xtalk_optimize_transform_step(struct nt_kernel *system);

/**
 * \brief Main process of adaptation of actMat.
 * \return none
 *  \code
    struct nt_module_data *module = _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface,
    instance));
    NT_ASSERT(module != NULL);
    nt_xtalk_adapt_model_process(module); 
  \endcode
 */
/* Main process of adaptation of actMat
*/
uint16_t nt_xtalk_adapt_model_process(struct nt_kernel *system);
void xtalk_rest_transform(struct nt_kernel *system);

#ifdef __cplusplus
}
#endif

/** \} end of gxtalk_api group */
/** \} end of Crosstalk group */

#endif
