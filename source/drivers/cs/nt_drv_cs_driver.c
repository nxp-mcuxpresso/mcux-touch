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

#include <string.h>
#include "nt_drv_cs_driver.h"
#include "nt_setup.h"
#include "fsl_power.h"
#include "board.h"
#include "fsl_device_registers.h"

#if FSL_FEATURE_SOC_CS_COUNT

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Table of base addresses for cs instances. */
CS_Type *const g_csBase[] = CS_BASE_PTRS;

/* Table to save CS IRQ numbers defined in CMSIS files. */
const IRQn_Type g_csIrqId[FSL_FEATURE_SOC_CS_COUNT] = {CS_IRQn};

/* Pointer to cs runtime state structure.*/
nt_cs_state_t *g_csStatePtr[FSL_FEATURE_SOC_CS_COUNT] = {NULL};

/*******************************************************************************
 * Local functions
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_Init
 * Description   : Initialize whole the CS peripheral to be ready to read capacitance changes
 * To initialize the CS driver, the configuration structure should be handled.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_Init(uint32_t instance, nt_cs_state_t *csState, const nt_cs_user_config_t *csUserConfig)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    CS_Type *base       = g_csBase[instance];
    nt_cs_state_t *csSt = g_csStatePtr[instance];

    /* OS: start of critical section, critical code will not be preemted */
    NT_OSA_EnterCritical();

    /* Exit if current instance is already initialized. */
    if (csSt)
    {
        /* OS: End of critical section. */
        NT_OSA_ExitCritical();

        return kStatus_CS_Initialized;
    }
    /* Save runtime structure pointer.*/
    csSt = g_csStatePtr[instance] = csState;

    /* Clear the state structure for this instance. */
    memset(csSt, 0, sizeof(nt_cs_state_t));

    /* OS: create the mutex used by whole driver. */
    NT_OSA_MutexCreate(&csSt->lock);
    /* OS: create the mutex used by change mode function. */
    NT_OSA_MutexCreate(&csSt->lockChangeMode);

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csSt->lock, OSA_WAIT_FOREVER))
    {
        /* TODO: OS End of critical section. */
        NT_OSA_ExitCritical();
        return kStatus_CS_Error;
    }

    /* OS: End of critical section. */
    NT_OSA_ExitCritical();

    csSt->opMode = cs_OpModeNormal;

    csSt->opModesData[csSt->opMode].config = *csUserConfig->config; /* Store the hardware configuration. */

    csSt->pCallBackFunc = csUserConfig->pCallBackFunc;
    csSt->usrData       = csUserConfig->usrData;

    CS_Init(base);

    /* Defining inputs */
    struct nt_kernel *kernel   = (struct nt_kernel *)csUserConfig->usrData;
    struct nt_module **modules = (struct nt_module **)kernel->rom->modules;
    uint8_t enabled_channel    = 0U;
    while (*modules != NULL)
    {
        if ((*modules)->interface == &nt_module_cs_interface)
        {
            uint8_t i = 0;
            while ((*modules)->electrodes[i] != NULL)
            {
                enabled_channel |= 1 << (*modules)->electrodes[i]->pin_input;
                i++;
            }
        }
        modules++;
    }
    csState->opModesData[csState->opMode].config.activeChannels = enabled_channel;

    CS_Config(base, &csState->opModesData[csState->opMode].config, kCS_ActiveMode);

    /* Clear possible pending flags */
    CS_ClearStatusFlags(base, (kCS_InterruptFifoNotEmptyFlag | kCS_InterruptFifoHalfFullFlag |
                               kCS_InterruptFifoFullFlag | kCS_InterruptScanCompleteFlag));

    CS_EnableInterrupts(base, kCS_InterruptScanCompleteEnable);

    /* Disable all electrodes */
    csState->opModesData[csState->opMode].enabledElectrodes = 0;

    /* Enable CS interrupt on NVIC level. */
    EnableIRQ(g_csIrqId[instance]);

    csSt->status = kStatus_CS_Initialized;

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csSt->lock);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_DeInit
 * Description   : De initialize whole the CS peripheral and driver to be ready
 * for any future use and don't load the system.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_DeInit(uint32_t instance)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    CS_Type *base          = g_csBase[instance];
    nt_cs_state_t *csState = g_csStatePtr[instance];

    if (csState == NULL)
    {
        return kStatus_CS_Error;
    }

    csState->opModesData[csState->opMode].enabledElectrodes = 0;

    /* Disable the interrupt */
    DisableIRQ(g_csIrqId[instance]);

    /* Clear runtime structure pointer.*/
    csState = NULL;

    /* Gate CS module clock */

    /*Deinit CS module*/
    CS_DeInit(base);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_SetCallBackFunc
 * Description   : Set the CS call back function pointer for non blocking measurement
 *
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_SetCallBackFunc(uint32_t instance, const cs_callback_t pFuncCallBack, void *usrData)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);
#ifdef NT_OSA
    nt_cs_state_t *csState = g_csStatePtr[instance];
#endif

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    if (g_csStatePtr[instance]->status != kStatus_CS_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&csState->lock);

        return g_csStatePtr[instance]->status;
    }

    g_csStatePtr[instance]->pCallBackFunc = pFuncCallBack;
    g_csStatePtr[instance]->usrData       = usrData;

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_GetEnabledElectrodes
 * Description   : Get Enables electrodes for measuring.
 *
 *END**************************************************************************/
uint8_t NT_CS_DRV_GetEnabledElectrodes(uint32_t instance)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    nt_cs_state_t *csState = g_csStatePtr[instance];

    return csState->opModesData[csState->opMode].enabledElectrodes;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CS_DRV_IsBusy
 * Description   : Function returns the busy state of the driver
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_GetStatus(uint32_t instance)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    return g_csStatePtr[instance]->status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_DisableLowPower
 * Description   : Enables/Disables the low power module.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_DisableLowPower(uint32_t instance, const nt_cs_modes_t mode)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    nt_cs_state_t *csState = g_csStatePtr[instance];
    cs_status_t status;

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    if (csState->status != kStatus_CS_LowPower)
    {
        /* TODO: OS End of critical section. */
        NT_OSA_MutexUnlock(&csState->lock);

        return csState->status;
    }

    csState->status = kStatus_CS_Initialized;

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    status = NT_CS_DRV_ChangeMode(instance, mode);

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_GetMode
 * Description   : Function returns the current mode of the driver.
 *
 *END**************************************************************************/
nt_cs_modes_t NT_CS_DRV_GetMode(uint32_t instance)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    return g_csStatePtr[instance]->opMode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_SaveConfiguration
 * Description   : The function save the configuration for one mode of operation.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_SaveConfiguration(uint32_t instance,
                                        const nt_cs_modes_t mode,
                                        nt_cs_operation_mode_t *operationMode)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);
    NT_ASSERT(operationMode);
    nt_cs_state_t *csState = g_csStatePtr[instance];

    if (mode >= cs_OpModeCnt)
    {
        return kStatus_CS_InvalidMode;
    }

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    *operationMode = csState->opModesData[mode];

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_EnableElectrode
 * Description   : Enables/Disables the electrode for measuring.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_EnableElectrode(uint32_t instance, const uint32_t channel, const bool enable)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);
    NT_ASSERT(channel < FSL_FEATURE_CS_CHANNEL_COUNT);

    nt_cs_state_t *csState = g_csStatePtr[instance];

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    if (csState->status != kStatus_CS_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&csState->lock);

        return csState->status;
    }

    /* Check the condition for low power mode. */
    if ((csState->opMode == cs_OpModeLowPower) || (csState->opMode == cs_OpModeProximity))
    {
        csState->opModesData[csState->opMode].enabledElectrodes = 0;
    }

    if (enable)
    {
        csState->opModesData[csState->opMode].enabledElectrodes |= (1ULL << channel);
    }
    else
    {
        csState->opModesData[csState->opMode].enabledElectrodes &= ~(1ULL << channel);
    }

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_GetCounter
 * Description   : Function returns the counter value of selected channel
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_GetCounter(uint32_t instance, const uint32_t channel, uint16_t *counter)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);
    NT_ASSERT(channel < FSL_FEATURE_CS_CHANNEL_COUNT);
    NT_ASSERT(counter);

    nt_cs_state_t *csState = g_csStatePtr[instance];

    if (!((1ULL << channel) &
          (csState->opModesData[csState->opMode].enabledElectrodes))) /* Check the channel number. */
    {
        return kStatus_CS_InvalidChannel;
    }

    *counter = csState->counters[channel];

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_Measure
 * Description   : This function gets (measure) capacitance of enabled electrodes
 *               from the CS module using a non-blocking method.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_Measure(uint32_t instance)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    CS_Type *base          = g_csBase[instance];
    nt_cs_state_t *csState = g_csStatePtr[instance];

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    if (csState->status != kStatus_CS_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&csState->lock);

        return csState->status;
    }

    if (!csState->opModesData[csState->opMode].enabledElectrodes)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&csState->lock);

        return kStatus_CS_InvalidChannel;
    }

    csState->status = kStatus_CS_Busy;

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    CS_Enable(base);

    nt_printf("NT_CS_DRV_Measure\n\r");

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_EnableLowPower
 * Description   : Enables/Disables the low power module.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_EnableLowPower(uint32_t instance)
{
    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_ChangeMode
 * Description   : The function change the current mode.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_ChangeMode(uint32_t instance, const nt_cs_modes_t mode)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);

    CS_Type *base          = g_csBase[instance];
    nt_cs_state_t *csState = g_csStatePtr[instance];

    if ((mode == csState->opMode) || (mode == cs_OpModeNoChange))
    {
        return kStatus_CS_Success;
    }

    if (mode >= cs_OpModeCnt)
    {
        return kStatus_CS_InvalidMode;
    }

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lockChangeMode, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    if (csState->status != kStatus_CS_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&csState->lockChangeMode);

        return csState->status;
    }

    csState->opMode = mode;

    CS_Config(base, &csState->opModesData[csState->opMode].config,
              (mode == cs_OpModeLowPower) ? kCS_WakeupMode : kCS_ActiveMode);

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lockChangeMode);

    return kStatus_CS_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_CS_DRV_LoadConfiguration
 * Description   : The function load the configuration for one mode of operation.
 *
 *END**************************************************************************/
cs_status_t NT_CS_DRV_LoadConfiguration(uint32_t instance,
                                        const nt_cs_modes_t mode,
                                        const nt_cs_operation_mode_t *operationMode)
{
    NT_ASSERT(instance < FSL_FEATURE_SOC_CS_COUNT);
    NT_ASSERT(operationMode);
    CS_Type *base;
    nt_cs_state_t *csState = g_csStatePtr[instance];

    if (mode >= cs_OpModeCnt)
    {
        return kStatus_CS_InvalidMode;
    }

    /* OS: Critical section. Access to global variable */
    if (knt_Status_OSA_Success != NT_OSA_MutexLock(&csState->lock, OSA_WAIT_FOREVER))
    {
        return kStatus_CS_Error;
    }

    csState->opModesData[mode] = *operationMode;

    /* In case that the loaded configuration is active one, update the HW also. */
    if (mode == csState->opMode)
    {
        base = g_csBase[instance];

        CS_Config(base, &csState->opModesData[csState->opMode].config,
                  (mode == cs_OpModeLowPower) ? kCS_WakeupMode : kCS_ActiveMode);
        CS_EnableInterrupts(base, kCS_InterruptScanCompleteEnable);
    }

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&csState->lock);

    return kStatus_CS_Success;
}

/*!
 * @brief Interrupt handler for CS.
 * This handler uses the cs State structure to handle the instance depend data.
 * This is not a public API as it is called whenever an interrupt occurs.
 */
void CS_DRV_IRQHandler(uint32_t instance)
{
    CS_Type *base             = g_csBase[instance];
    nt_cs_state_t *csState    = g_csStatePtr[instance];
    uint8_t activeCounterGain = csState->opModesData[csState->opMode].config.activeCounterGain;

    uint32_t val;

    for (uint32_t i = 0; i < FSL_FEATURE_CS_CHANNEL_COUNT; i++)
    {
        if (CS_GetStatusFlags(base) & kCS_InterruptFifoNotEmptyFlag)
        {
            val                                   = CS_ReadData(base);
            csState->counters[(val >> 16) & 0x07] = ((uint16_t)0xFFFF - (val & 0xFFFF) * activeCounterGain);
            nt_printf("CH:%d  DAT:%d    ", (val >> 16) & 0x07, val & 0xFFFF);
        }
    }

    nt_printf("********\n\r");

    /* Clear interrupt flags. */
    CS_ClearStatusFlags(base, (kCS_InterruptFifoNotEmptyFlag | kCS_InterruptFifoHalfFullFlag |
                               kCS_InterruptFifoFullFlag | kCS_InterruptScanCompleteFlag));

    CS_Disable(base);

    if (csState->pCallBackFunc)
    {
        csState->pCallBackFunc(instance, csState->usrData);
    }

    if (csState->status != kStatus_CS_LowPower)
    {
        /* Return status of the driver to initialized state */
        csState->status = kStatus_CS_Initialized;
    }
}

#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
