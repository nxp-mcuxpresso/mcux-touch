/*
 * Copyright 2013 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022, 2024 NXP
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
#ifndef _NT_XTALK_PRV_H_
#define _NT_XTALK_PRV_H_

#include "nt_types.h"

/**
 * \defgroup xtalk_private Crosstalk
 * \ingroup ntapi_private
 * Cross-talk represent the cross-talk check layer in the NXP Touch system,
 * it is the layer that is tightly coupled to hardware available on
 * the NXP MCU device.
 *
 * Each cross-talk implements a set of private functions contained in the nt_xtalk_prv.h
 * file.
 *
 * \{
 */
/**
 * \defgroup gxtalk_private General API
 * \ingroup xtalk_private
 * General API and definition over all cross-talk.
 *
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

extern struct nt_xtalk_data xtalk_data; 
  
/**
 *  The cross-talk RAM structure is used to store the temporary data of the cross-talk.
 *
 *  Allocate this structure and put a pointer into the nt_control_proxi
 *  structure when it is being registered in the system.
 */
struct nt_xtalk_data
{
    int32_t  *adjCol;                            /*!< Internal data for transform optimization */
    int32_t  *actToConfRowGain;                  /*!< Internal data for transform optimization */
    int32_t  *confMat;                           /*!< Cross-talk confusion matrix */
    int32_t  *transformVec;                      /*!< Cross-talk reduction transformation vectors */
    int32_t  *delta_reduced;                     /*!< Cross-talk delta signal array for cross-talk calculation. */    
    int32_t  *delta;                             /*!< Delta signal array without cross-talk calculation, inputs for xtalk */
    uint16_t *sensInd;                           /*!< Vector of k largest co-activations (for internal computations) */
    uint16_t *indButtons;                        /*!< Matrix of k largest co-activations for each electrode */
    int16_t  *profile_buffer;                    /*!< Buffer for accumulation of single activation profile */
    uint8_t  *state_prev;                        /*!< Vector of electrodes' previous state */    
    uint8_t optCounter;                          /*!< State used for internal algorithm scheduling */
    uint8_t optGainReady;                        /*!< State used for internal algorithm scheduling */
    uint8_t optReady;                            /*!< State used for internal algorithm scheduling */
};  
  
/**
 *  Cross-talk interface structure; cross-talk uses this structure to register the entry points
 *  to its algorithms. This approach enables a kind-of polymorphism in the touch System.
 *  Crosstalk functions are processed the same way from the System layer, regardless of the specific
 *  implementation.
 */
struct nt_system_xtalk_interface
{
    int32_t (*init)(struct nt_kernel *system);    /*!< The initialization of the cross-talk functions */
    int32_t (*process)(struct nt_kernel *system); /*!< Process the cross-talk functions */
    const char *name;           /*!< A name of the variable of this type, used for FreeMASTER support purposes. */
    const uint32_t params_size; /*!< Structure size */
};

/**
 * \brief Init the cross-talk electrode for cross-talk processes.
 * \param system Pointer to the where the system cross-talk electrodes to be initialized.
 * \return The result of the operation.
 */
int32_t _nt_system_xtalk_init(struct nt_kernel *system);
/**
 * \brief Init the cross-talk electrode for cross-talk processes.
 * \param system Pointer to the where the system cross-talk electrodes to be initialized.
 * \return The result of the operation.
 */

int32_t _nt_system_xtalk_electrode_init(struct nt_kernel *system);
/**
 * \brief Process the cross-talk on the system.
 * \param system Pointer to the where the system cross-talk to be processed.
 * \return The result of the operation.
 */
int32_t _nt_system_xtalk_process(struct nt_kernel *system);

#ifdef __cplusplus
}
#endif

/** \} */ /* end of gxtalk_private group */
/** \} */ /* end of xtalk_private group */

#endif /* _NT_XTALK_PRV_H_ */
