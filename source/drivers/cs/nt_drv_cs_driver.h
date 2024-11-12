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

#ifndef _FSL_CS_DRIVER_H_
#define _FSL_CS_DRIVER_H_

/**
 * \defgroup cs_drivers CS Drivers
 * \ingroup ntapi
 *
 * Drivers represent the lowest level of abstraction in Capacitive sense (CS) peripheral control and
 * contain common and CS version specific files. There are functions like
 * \ref NT_CS_DRV_Init, \ref NT_CS_DRV_EnableElectrode, \ref NT_CS_DRV_Measure, etc.
 *
 * \{
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fsl_device_registers.h"
#include "../source/system/nt_system_prv.h"

#include "fsl_cs.h"

#if FSL_FEATURE_SOC_CS_COUNT || defined(NT_DOXYGEN)

/**
 * \brief Call back routine of CS driver.
 *
 * The function is called on end of the measure of the CS driver. The function
 * can be called from interrupt, so the code inside the callback should be short
 * and fast.
 * \param instance - instance of the CS peripheral
 * \param usrData - user data (type is void*), the user data are specified by function \ref NT_CS_DRV_SetCallBackFunc
 * \return - none
 */
typedef void (*cs_callback_t)(uint32_t instance, void *usrData);

/** Error codes for the CS driver. */
typedef enum _cs_status
{
    kStatus_CS_Success = 0,
    kStatus_CS_Busy,           /*!< CS still in progress */
    kStatus_CS_Overflow,       /*!< CS counter out of range */
    kStatus_CS_Overrun,        /*!< CS measurement overrun  */
    kStatus_CS_LowPower,       /*!< CS is in low power mode */
    kStatus_CS_InvalidChannel, /*!< Invalid CS channel */
    kStatus_CS_InvalidMode,    /*!< Invalid CS mode */
    kStatus_CS_Initialized,    /*!< The driver is initialized and ready to measure */
    kStatus_CS_Error           /*!< The general driver error */
} cs_status_t;

/**
 * User configuration structure for CS driver.
 *
 * Use an instance of this structure with NT_CS_DRV_Init(). This allows you to configure the
 * most common settings of the CS peripheral with a single function call. Settings include:
 *
 */
typedef struct _nt_cs_user_config
{
    cs_config_t *config;         /*!< A pointer to hardware configuration. Can't be NULL. */
    cs_callback_t pCallBackFunc; /*!< A pointer to call back function of end of measurement. */
    void *usrData;               /*!< A user data of call back function. */
} nt_cs_user_config_t;

/**
 * Driver operation mode definition.
 *
 * The operation name definition used for CS driver.
 *
 */
typedef enum _nt_cs_modes
{
    cs_OpModeNormal = 0, /*!< The normal active mode of CS. */
    cs_OpModeLowPower,   /*!< The low power mode of CS. */
    cs_OpModeProximity,  /*!< The proximity sensing mode of CS. */
    cs_OpModeCnt,        /*!< Count of CS modes - for internal use. */
    cs_OpModeNoChange    /*!< The special value of operation mode that allows call for example \ref
                            NT_CS_DRV_DisableLowPower function without change of operation mode. */
} nt_cs_modes_t;

/**
 * Driver operation mode data hold structure.
 *
 * This is the operation mode data hold structure. The structure is keep all needed data
 * to be driver able to switch the operation modes and properly set up HW peripheral.
 *
 */
typedef struct _nt_cs_operation_mode
{
    uint8_t enabledElectrodes; /*!< The back up of enabled electrodes for operation mode */
    cs_config_t config;        /*!< A hardware configuration. */
} nt_cs_operation_mode_t;

/**
 * Driver data storage place.
 *
 * It must be created by the application code and the pointer is handled by the \ref NT_CS_DRV_Init function
 * to driver. The driver keeps all context data for itself run. Settings include:
 *
 */
typedef struct _nt_cs_state
{
    cs_status_t status;          /*!< Current status of the driver. */
    cs_callback_t pCallBackFunc; /*!< A pointer to call back function of end of measurement. */
    void *usrData;               /*!< A user data pointer handled by call back function. */
    nt_mutex_t lock;             /*!< Used by whole driver to secure the context data integrity. */
    nt_mutex_t lockChangeMode;   /*!< Used by change mode function to secure the context data integrity. */
    nt_cs_modes_t opMode;        /*!< Storage of current operation mode. */
    nt_cs_operation_mode_t opModesData[cs_OpModeCnt]; /*!< Data storage of individual operational modes. */
    uint16_t counters[FSL_FEATURE_CS_CHANNEL_COUNT];  /*!< The mirror of last state of counter registers */
} nt_cs_state_t;

/** Table of base addresses for CS instances. */
extern CS_Type *const g_csBase[];

/** Table to save CS IRQ enumeration numbers defined in CMSIS header file. */
extern const IRQn_Type g_csIrqId[FSL_FEATURE_SOC_CS_COUNT];

/** Table to save pointers to context data. */
extern nt_cs_state_t *g_csStatePtr[FSL_FEATURE_SOC_CS_COUNT];

/*******************************************************************************
 * API
 ******************************************************************************/

/**
 * \defgroup cs_drivers_api API Functions
 * \ingroup cs_drivers
 * General Function definition of the drivers.
 *
 * \{ */

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \brief Initializes a CS instance for operation.
*
* This function initializes the run-time state structure and prepares the
* entire peripheral to measure the capacitances on electrodes.
 \code

   static nt_cs_state_t myCSDriverStateStructure;

   nt_cs_user_config_t myCSDriveruserConfig =
   {
    .config =
       {
          ...
       },
     .pCallBackFunc = APP_myCSCallBackFunc,
     .usrData = myData,
   };

   if(NT_CS_DRV_Init(0, &myCSDriverStateStructure, &myCSDriveruserConfig) != kStatus_CS_Success)
   {
      // Error, the CS is not initialized
   }
  \endcode
*
* \param instance The CS module instance.
* \param csState A pointer to the CS driver state structure memory. The user is only
*  responsible to pass in the memory for this run-time state structure where the CS driver
*  will take care of filling out the members. This run-time state structure keeps track of the
*  current CS peripheral and driver state.
* \param csUserConfig The user configuration structure of type nt_cs_user_config_t. The user
*   populates the members of this structure and  passes the pointer of this structure
*  into the function.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_Init(uint32_t instance, nt_cs_state_t *csState, const nt_cs_user_config_t *csUserConfig);

/**
* \brief Shuts down the CS by disabling interrupts and the peripheral.
*
* This function disables the CS interrupts and the peripheral.
*
 \code
   if(NT_CS_DRV_DeInit(0) != kStatus_CS_Success)
   {
      // Error, the CS is not de-initialized
   }
  \endcode
* \param instance The CS module instance.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_DeInit(uint32_t instance);

/**
* \brief Enables/disables one electrode of the CS module.
*
* Function must be called for each used electrodes after initialization of CS driver.
*
  \code
        // On the CS instance 0, enable electrode with index 5
    if(NT_CS_DRV_EnableElectrode(0, 5, TRUE) != kStatus_CS_Success)
    {
        // Error, the CS 5'th electrode is not enabled
    }
  \endcode
* \param instance   The CS module instance.
* \param channel    Index of CS channel.
* \param enable     TRUE - for channel enable, FALSE for disable.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_EnableElectrode(uint32_t instance, const uint32_t channel, const bool enable);

/**
* \brief Returns a mask of the enabled electrodes of the CS module.
*
* The function returns the mask of the enabled electrodes of the current mode.
*
  \code
    uint32_t enabledElectrodeMask;
    enabledElectrodeMask = NT_CS_DRV_GetEnabledElectrodes(0);
  \endcode
* \param instance The CS module instance.
* \return Mask of enabled electrodes for current mode.
*/
uint8_t NT_CS_DRV_GetEnabledElectrodes(uint32_t instance);

/**
* \brief Starts the measure cycle of the enabled electrodes.
*
* The function is non blocking. Therefore, the results can be obtained after the driver completes the measure cycle.
*         The end of the measure cycle can be checked by pooling the \ref NT_CS_DRV_GetStatus function or wait for
registered callback function by using the
*         \ref NT_CS_DRV_SetCallBackFunc or \ref NT_CS_DRV_Init.
*
  \code
    // Example of the pooling style of use of NT_CS_DRV_Measure() function
    if(NT_CS_DRV_Measure(0) != kStatus_CS_Success)
    {
        // Error, the CS 5'th electrode is not enabled
    }

    while(NT_CS_DRV_GetStatus(0) != kStatus_CS_Initialized)
    {
        // Do something useful - don't waste the CPU cycle time
    }

  \endcode
* \param instance The CS module instance.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_Measure(uint32_t instance);

/**
* \brief Returns the last measured value.
*
* This function returns the last measured value in the previous measure cycle.
*           The data is buffered inside the driver.
*
  \code
    // Get the counter value from CS instance 0 and 5th channel

    uint32_t result;

    if(NT_CS_DRV_GetCounter(0, 5, &result) != kStatus_CS_Success)
    {
        // Error, the CS 5'th electrode is not read
    }

  \endcode
* \param instance The CS module instance.
* \param channel The CS electrode index.
* \param counter The pointer to 16 bit value where will be stored channel counter value.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_GetCounter(uint32_t instance, const uint32_t channel, uint16_t *counter);

/**
* \brief Returns the current status of the driver.
*
* This function returns the current working status of the driver.
*
  \code
    // Get the current status of CS driver

    cs_status_t status;

    status = NT_CS_DRV_GetStatus(0);


  \endcode
* \param instance The CS module instance.
* \return An current status of the driver.
*/
cs_status_t NT_CS_DRV_GetStatus(uint32_t instance);

/**
* \brief Enters the low power mode of the CS driver.
*
* This function switches the driver to low power mode and immediately enables the
*            low power functionality of the CS peripheral. Before calling this
*           function, the low power mode must be configured - Enable the right electrode
*           and recalibrate the low power mode to get the best performance for this mode.
*
  \code
    // Switch the driver to the low power mode
    uint16_t signal;

    // The first time is needed to configure the low power mode configuration

    (void)NT_CS_DRV_ChangeMode(0, cs_OpModeLowPower); // I don't check the result because I believe in.
    // Enable the right one electrode for low power AKE up operation
    (void)NT_CS_DRV_EnableElectrode(0, 5, true);
    // Recalibrate the mode to get the best performance for this one electrode
    (void)NT_CS_DRV_Recalibrate(0);

    if(NT_CS_DRV_EnableLowPower(0) != kStatus_CS_Success)
    {
        // Error, the CS driver can't go to low power mode
    }


  \endcode
* \param instance The CS module instance.
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_EnableLowPower(uint32_t instance);

/**
* \brief This function returns back the CS driver from the low power to standard operation
*
* Function switch the driver back form low power mode and it can immediately change
*           the operation mode to any other or keep the driver in low power
*           configuration, to be able go back to low power state.
*
  \code
    // Switch the driver from the low power mode

    if(NT_CS_DRV_DisableLowPower(0, cs_OpModeNormal) != kStatus_CS_Success)
    {
        // Error, the CS driver can't go from low power mode
    }


  \endcode
* \param instance   The CS module instance.
* \param mode       The new operation mode request
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_DisableLowPower(uint32_t instance, const nt_cs_modes_t mode);

/**
* \brief Sets the callback function that is called when the measure cycle ends.
*
* This function sets up or clears, (parameter pFuncCallBack  = NULL), the callback function pointer
*           which is called after each measure cycle ends. The user can also set the custom user data,
*           that is handled by the parameter to a call back function. One function can be called by more sources.
*
  \code
    // Clear previous call back function

    if(NT_CS_DRV_SetCallBackFunc(0, NULL, NULL) != kStatus_CS_Success)
    {
        // Error, the CS driver can't set up the call back function at the moment
    }

    // Set new call back function

    if(NT_CS_DRV_SetCallBackFunc(0, myFunction, (void*)0x12345678) != kStatus_CS_Success)
    {
        // Error, the CS driver can't set up the call back function at the moment
    }


  \endcode
* \param instance       The CS module instance.
* \param pFuncCallBack  The pointer to application call back function
* \param usrData        The user data pointer
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_SetCallBackFunc(uint32_t instance, const cs_callback_t pFuncCallBack, void *usrData);

/**
* \brief Changes the current working operation mode.
*
* This function changes the working operation mode of the driver.
*
  \code
    // Change operation mode to low power

    if(NT_CS_DRV_ChangeMode(0, cs_OpModeLowPower) != kStatus_CS_Success)
    {
        // Error, the CS driver can't change the operation mode into low power
    }

  \endcode
* \param instance       The CS module instance.
* \param mode           The requested new operation mode
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_ChangeMode(uint32_t instance, const nt_cs_modes_t mode);

/**
* \brief Returns the current working operation mode.
*
* This function returns the current working operation mode of the driver.
*
  \code
    // Gets current operation mode of CS driver
    nt_cs_modes_t mode;

    mode = NT_CS_DRV_GetMode(0);

  \endcode
* \param instance       The CS module instance.
* \return An current operation mode of CS driver.
*/
nt_cs_modes_t NT_CS_DRV_GetMode(uint32_t instance);

/**
* \brief Loads the new configuration into a specific mode.
*
* This function loads the new configuration into a specific mode.
*           This can be used when the calibrated data are stored in any NVM
*           to load after startup of the MCU to avoid run recalibration that takes
*           more time.
*
  \code
    // Load operation mode configuration

    extern const nt_cs_operation_mode_t * myCSNvmLowPowerConfiguration;

    if(NT_CS_DRV_LoadConfiguration(0, cs_OpModeLowPower, myCSNvmLowPowerConfiguration) != kStatus_CS_Success)
    {
        // Error, the CS driver can't load the configuration
    }

  \endcode
* \param instance       The CS module instance.
* \param mode           The requested new operation mode
* \param operationMode  The pointer to storage place of the configuration that should be loaded
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_LoadConfiguration(uint32_t instance,
                                        const nt_cs_modes_t mode,
                                        const nt_cs_operation_mode_t *operationMode);

/**
* \brief Saves the CS driver configuration for a specific mode.
*
* This function saves the configuration of a specific mode.
*           This can be used when the calibrated data should be stored in any backup memory
*           to load after the start of the MCU to avoid running the recalibration that takes
*           more time.
*
  \code
    // Save operation mode configuration

    extern nt_cs_operation_mode_t  myCSNvmLowPowerConfiguration;

    if(NT_CS_DRV_SaveConfiguration(0, cs_OpModeLowPower, &myCSNvmLowPowerConfiguration) != kStatus_CS_Success)
    {
        // Error, the CS driver can't save the configuration
    }

  \endcode
* \param instance       The CS module instance.
* \param mode           The requested new operation mode
* \param operationMode  The pointer to storage place of the configuration that should be save
* \return An error code or kStatus_CS_Success.
*/
cs_status_t NT_CS_DRV_SaveConfiguration(uint32_t instance,
                                        const nt_cs_modes_t mode,
                                        nt_cs_operation_mode_t *operationMode);

#if defined(__cplusplus)
}
#endif

/** \} */ /* end of cs_drivers_api group */
/** \} */ /* end of cs_drivers group */

#endif
#endif /* _FSL_CS_DRIVER_H_ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
