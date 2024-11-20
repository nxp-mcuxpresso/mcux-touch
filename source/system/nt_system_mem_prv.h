/*
 * Copyright 2013 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021, 2024 NXP
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


#ifndef _NT_MEM_PRV_H_
#define _NT_MEM_PRV_H_

#include "nt_types.h"

/**
 * \defgroup memory_private Memory Management
 * \ingroup ntapi_private
 *
 * \{
 */

/**
 *
 * This structure contains the memory pool for all RAM data of the NXP touch
 * volatile data structures.
 * This structure can be allocated in RAM.
 */
struct nt_mem
{
    uint8_t *pool;         /*!< Pointer to the allocated memory pool for the NXP touch. */
    uint32_t pool_size;    /*!< Size of the allocated memory pool for the NXP touch. */
    uint8_t *free_pointer; /*!< Pointer to the last free position in the memory pool. */
};

/**
 * \defgroup memory_api_private API functions
 * \ingroup memory_private
 * General Private Function definition of the memory support.
 *
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialized the NXP touch memory management system.
 * \param pool - pointer to the allocated memory place to be used by the system (the size must be aligned by 4).
 * \param size - size of the memory pool handled by the pool parameter.
 * \return The result of the operation.
 */
int32_t _nt_mem_init(uint8_t *pool, const uint32_t size);

/**
 * \brief Allocation of memory from the memory pool.
 * \param size - size of the memory block to allocate.
 * \return The pointer to the new allocated block, NULL in case there is not enough space in the pool.
 */
void *_nt_mem_alloc(const uint32_t size);

/**
 * \brief De-initialized the NXP touch memory management system.
 * \return The result of the operation.
 */
int32_t _nt_mem_deinit(void);

#ifdef __cplusplus
}
#endif

/** \} */ /* end of memory_api_private group */
/** \} */ /* end of memory_private group */

#endif /* NT_MEM_PRV_H_ */
