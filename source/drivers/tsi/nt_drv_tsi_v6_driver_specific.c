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
#include "fsl_common.h"
#include "fsl_tsi_v6.h"
#include "nt_drv_tsi_driver.h"
#include "nt_module_tsi.h"
#include "../source/system/nt_system_prv.h"
#include "../source/modules/nt_modules_prv.h"
#include "../source/electrodes/nt_electrodes_prv.h"
#define NT_MODULE_TSI_NAME "nt_module_tsi_interface"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * variable definition for obtaining flash start and end adress to check if configuration is located in flash or ram
 */

#ifndef NT_FLASH_START
#ifndef NT_FLASH_END
#if defined(__IAR_SYSTEMS_ICC__) /* For IAR compiler   */
#pragma section = ".rodata"
uint32_t flash_start, flash_end;
#elif defined(__CC_ARM)                                 /* For ARM(KEIL) version < 6 compiler */
#elif defined(__MCUXPRESSO)                             /* For GCC compiler  MCUX IDE */
uint32_t flash_start;
uint32_t flash_end;
uint32_t __base_PROGRAM_FLASH;
uint32_t __top_PROGRAM_FLASH;
#elif defined(__GNUC__) && (__ARMCC_VERSION == 0)       /* For ARMGCC compiler */
uint32_t flash_start;
uint32_t flash_end;
#elif defined(__GNUC__) && (__ARMCC_VERSION >= 6010050) /* For ARM(KEIL) version >= 60 compiler */
#else                                                   /* Other compiler used */
#warning "Unsupported compiler/IDE used !"
#endif
#endif /* NT_FLASH_END */
#endif /* NT_FLASH_START */

#define FIX_MAX(aVal,bVal) ( ( (aVal) > (bVal) ) ?    (aVal)   : (bVal) )

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile tsi_lpwr_status_flags_t tsi_lpwr_status;

/******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_Init
 * Description   : Encapsulates TSI v5 init.
 *
 *END**************************************************************************/
void NT_TSI_DRV_InitSpecific(TSI_Type *base, const tsi_config_t *config)
{
    TSI_InitSelfCapMode(base, &config->configSelfCap);
    TSI_EnableModule(base, true);
#if (defined(FSL_FEATURE_TSI_HAS_M_TX_USED) && FSL_FEATURE_TSI_HAS_M_TX_USED)
    /* Clear M_TX_USED bitfield, reset values are 1 values*/
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_0);
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_1);
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_2);
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_3);
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_4);
    TSI_ClearUsedTxChannel(base, kTSI_MutualTxChannel_5);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_EnableElectrode
 * Description   : Enables/Disables the electrode for measuring.
 *
 *END**************************************************************************/
tsi_status_t NT_TSI_DRV_EnableElectrode(uint32_t instance, const uint32_t channel, const bool enable)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);
    NT_ASSERT(channel < TF_TSI_TOTAL_CHANNEL_COUNT);

    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];

    if (tsiState->status != kStatus_TSI_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&tsiState->lock);

        return tsiState->status;
    }

    /* Check the condition for low power mode. */
    if ((tsiState->opMode == tsi_OpModeLowPower) || (tsiState->opMode == tsi_OpModeProximity))
    {
        tsiState->opModesData[tsiState->opMode].enabledElectrodes = 0;
    }

    if (enable)
    {
        tsiState->opModesData[tsiState->opMode].enabledElectrodes |= (1ULL << channel);
    }
    else
    {
        tsiState->opModesData[tsiState->opMode].enabledElectrodes &= ~(1ULL << channel);
    }

    /* Check self/mutual setting feasibility */
    if (channel > (TF_TSI_SELF_CAP_CHANNEL_COUNT - 1U))
    {
        NT_ASSERT(!(bool)(tsiState->opModesData[tsiState->opMode].enabledElectrodes &
                          (1ULL << ((channel - TF_TSI_SELF_CAP_CHANNEL_COUNT) / TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT))));
        NT_ASSERT(!(bool)(tsiState->opModesData[tsiState->opMode].enabledElectrodes &
                          (1ULL << (((channel - TF_TSI_SELF_CAP_CHANNEL_COUNT) % TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT) +
                                    TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT))));
#if (defined(FSL_FEATURE_TSI_HAS_M_TX_USED) && FSL_FEATURE_TSI_HAS_M_TX_USED)
        /* Set or clear M_TX_USED bitfield to make normal GPIO function for pins not used as TSI mutual TX pins */
        TSI_Type *base = g_tsiBase[instance];
        if (enable)
            TSI_SetUsedTxChannel(base, (tsi_mutual_tx_channel_t)((channel - TF_TSI_SELF_CAP_CHANNEL_COUNT) /
                                                                 TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT));
        else
            TSI_ClearUsedTxChannel(base, (tsi_mutual_tx_channel_t)((channel - TF_TSI_SELF_CAP_CHANNEL_COUNT) /
                                                                   TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT));
#endif
    }

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&tsiState->lock);

    return kStatus_TSI_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_Measure
 * Description   : This function gets (measure) capacitance of enabled electrodes
 *               from the TSI module using a non-blocking method.
 *
 *END**************************************************************************/
tsi_status_t NT_TSI_DRV_Measure(uint32_t instance)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);

    TSI_Type *base           = g_tsiBase[instance];
    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];
    struct nt_module_data *module =
        _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface, instance));
    const struct nt_electrode *const *electrodes = module->rom->electrodes;

    uint32_t mutual_rx, mutual_tx;
    uint8_t electrode_last = module->electrodes_cnt;

    if (tsiState->status != kStatus_TSI_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&tsiState->lock);

        return tsiState->status;
    }

    /* Check if at least one electrode is enabled. */
    while ((bool)(electrode_last--))
    {
        if ((bool)_nt_electrode_get_flag(module->electrodes[electrode_last], (uint32_t)NT_ELECTRODE_ENABLED))
        {
            break;
        }
        if (electrode_last == 0U)
        {
            /* OS: End of critical section. */
            NT_OSA_MutexUnlock(&tsiState->lock);

            return kStatus_TSI_InvalidChannel;
        }
    }
    tsiState->status = kStatus_TSI_Busy;

    electrode_last = module->electrode_last;
    NT_ASSERT(electrode_last <= module->electrodes_cnt);

    /* re-trigger new measure set in case the measurement was not start from handler */
    if (tsiState->status != kStatus_TSI_Overflow)
    {
        if (tsiState->opSatus != tsi_OpStatusSuspend)
        {
            if (electrode_last == 0U)
            {
                module->electrode_last = module->electrodes_cnt; /* reset electrode number */
                electrode_last         = module->electrode_last;
            }
        }
    }

    while ((bool)electrode_last--)
    {
        /* Find the last enabled electrode */
        if ((bool)_nt_electrode_get_flag(module->electrodes[electrode_last], (uint32_t)NT_ELECTRODE_ENABLED))
        {
            /* OS: End of critical section. */
            NT_OSA_MutexUnlock(&tsiState->lock);

            /* Init self-cap sensing */
            TSI_EnableModule(base, false);

            /* If this electrode is a self-cap mode electrode initialize self-cap sensing*/
            if (electrodes[electrode_last]->pin_input < TF_TSI_SELF_CAP_CHANNEL_COUNT)
            {
                TSI_InitSelfCapMode(base, &module->electrodes[electrode_last]->tsi_hw_config->configSelfCap);
                TSI_SetSelfCapMeasuredChannel(base, (uint8_t)electrodes[electrode_last]->pin_input);
            }
            /* If this electrode is a mutual-cap mdde electrode, initialize mutual-cap sensing*/
            else
            {
                /*Parse electrode number into rx, tx components*/
                mutual_tx = (electrodes[electrode_last]->pin_input - TF_TSI_SELF_CAP_CHANNEL_COUNT) /
                            TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT;
                mutual_rx = (electrodes[electrode_last]->pin_input - TF_TSI_SELF_CAP_CHANNEL_COUNT) %
                            TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT;

                TSI_InitMutualCapMode(base, &module->electrodes[electrode_last]->tsi_hw_config->configMutual);
                TSI_SetMutualCapTxChannel(base, (tsi_mutual_tx_channel_t)mutual_tx);
                TSI_SetMutualCapRxChannel(base, (tsi_mutual_rx_channel_t)mutual_rx);
            }
            TSI_EnableHardwareTriggerScan(base, false);
            TSI_EnableModule(base, true);
            if (tsiState->opSatus != tsi_OpStatusSuspend)
            {
                TSI_StartSoftwareTrigger(base);
                module->electrode_last =
                    electrode_last; /* store TSI channel triggered for sensing to be read in handler*/
            }
            return kStatus_TSI_Success;
        }
    }
    return kStatus_TSI_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_EnableLowPower
 * Description   : Enables/Disables the low power module.
 *
 *END**************************************************************************/
tsi_status_t NT_TSI_DRV_EnableLowPower(uint32_t instance)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);

    TSI_Type *base           = g_tsiBase[instance];
    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];
    tsi_status_t status;

    if ((status = NT_TSI_DRV_ChangeMode(instance, tsi_OpModeLowPower)) != kStatus_TSI_Success)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&tsiState->lock);

        return status;
    }

    if (tsiState->opModesData[tsiState->opMode].enabledElectrodes == 0U)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&tsiState->lock);

        return kStatus_TSI_InvalidChannel;
    }

    tsiState->status = kStatus_TSI_LowPower;

    /* Enable TSI to run in STOP and VLPS mode */
    TSI_EnableModule(base, false);
    TSI_EnableLowPower(base, true);
    TSI_EnableModule(base, true);

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&tsiState->lock);

    return kStatus_TSI_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_ChangeMode
 * Description   : The function change the current mode.
 *
 *END**************************************************************************/
tsi_status_t NT_TSI_DRV_ChangeMode(uint32_t instance, const nt_tsi_modes_t mode)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);

    TSI_Type *base           = g_tsiBase[instance];
    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];

    if ((mode == tsiState->opMode) || (mode == tsi_OpModeNoChange))
    {
        return kStatus_TSI_Success;
    }

    if (mode >= tsi_OpModeCnt)
    {
        return kStatus_TSI_InvalidMode;
    }

    if (tsiState->status != kStatus_TSI_Initialized)
    {
        /* OS: End of critical section. */
        NT_OSA_MutexUnlock(&tsiState->lockChangeMode);

        return tsiState->status;
    }

    tsiState->opMode = mode;

    NT_TSI_DRV_InitSpecific(base, &tsiState->opModesData[mode].config);

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&tsiState->lockChangeMode);

    return kStatus_TSI_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : NT_TSI_DRV_LoadConfiguration
 * Description   : The function load the configuration for one mode of operation.
 *
 *END**************************************************************************/
tsi_status_t NT_TSI_DRV_LoadConfiguration(uint32_t instance,
                                          const nt_tsi_modes_t mode,
                                          const nt_tsi_operation_mode_t *operationMode)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);
    NT_ASSERT(operationMode != NULL);
    TSI_Type *base;
    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];

    if (mode >= tsi_OpModeCnt)
    {
        return kStatus_TSI_InvalidMode;
    }

    tsiState->opModesData[mode] = *operationMode;

    /* In case that the loaded configuration is active one, update the HW also. */
    if (mode == tsiState->opMode)
    {
        base = g_tsiBase[instance];

        NT_TSI_DRV_InitSpecific(base, &tsiState->opModesData[mode].config);
        TSI_EnableInterrupts(base, (uint32_t)kTSI_GlobalInterruptEnable);
        TSI_EnableInterrupts(base, (uint32_t)kTSI_EndOfScanInterruptEnable);
    }

    /* OS: End of critical section. */
    NT_OSA_MutexUnlock(&tsiState->lock);

    return kStatus_TSI_Success;
}

/*!
 * @brief Get Sinc status flags
 * This function get tsi Sinc status flags.
 *
 * @param    base  TSI peripheral base address.
 * @return         The mask of these status flags combination.
 */
static inline uint32_t TSI_GetSincFlags(TSI_Type *base)
{
    return (base->SINC & ((uint32_t)kTSI_EndOfScanFlag | (uint32_t)kTSI_OutOfRangeFlag));
}

/*!
 * @brief Interrupt handler for TSI.
 * This handler uses the tsi State structure to handle the instance depend data.
 * This is not a public API as it is called whenever an interrupt occurs.
 */
void TSI_DRV_IRQHandler(uint32_t instance)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);
    uint32_t mutual_rx;
    uint32_t mutual_tx;
    TSI_Type *base           = g_tsiBase[instance];
    nt_tsi_state_t *tsiState = g_tsiStatePtr[instance];
    struct nt_module_data *module =
        _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_tsi_interface, instance));
    const struct nt_electrode *const *electrodes = module->rom->electrodes;
    uint8_t electrode_last                       = module->electrode_last;
    NT_ASSERT(electrode_last <= module->electrodes_cnt);

    if ((bool)(TSI_GetSincFlags(base) & (uint32_t)kTSI_SincOverflowFlag))
    {
        tsiState->status = kStatus_TSI_Overflow;
        /* If overflow execute following commands */
        if ((bool)(tsiState->pCallBackFunc))
        {
            tsiState->pCallBackFunc(instance, tsiState->usrData);
        }
        tsiState->status = kStatus_TSI_Success;
    }

    if ((bool)(tsi_lpwr_status.TSILowPower))
    {
        tsi_lpwr_status.SelfLowPowerCountBuff   = TSI_GetCounter(base);
        tsi_lpwr_status.SelfLowPowerChannelBuff = TSI_GetSelfCapMeasuredChannel(base);
        tsi_lpwr_status.TSILowPower             = 0;
    }

    /* Clear interrupt flags. */
    TSI_ClearStatusFlags(base, (uint32_t)kTSI_OutOfRangeFlag);
    TSI_ClearStatusFlags(base, (uint32_t)kTSI_EndOfScanFlag);

    /* reading the measured value */
    if ((bool)(tsiState->status != kStatus_TSI_Overflow))
    {
        if ((bool)(tsiState->opSatus != tsi_OpStatusSuspend))
        {                                          /* Save TSICNT value */
        uint16_t nstep1 = ( uint32_t )TSI_GetCounter( base );  
        if (!(bool)(TSI_GetSensingMode(base))) /* self-cap electrode was measured */
        {
          if(module->electrodes[electrode_last]->tsi_hw_config->newCalc == true)
          {
            uint32_t aux1;
            uint8_t cnt_shift = module->electrodes[electrode_last]->rom->tsicnt_shift;
            module->electrodes[electrode_last]->nstep = nstep1;
            if(cnt_shift > 11)
            {
              cnt_shift = 11;
            }
            /* raw_signal is uint16, nstep is [16 2048] */
            aux1 = ((( uint32_t )0x100000 << cnt_shift) / FIX_MAX(( uint32_t )nstep1, 16 ));
            _nt_electrode_set_raw_signal( module->electrodes[electrode_last], aux1 );
          }
          else
          {
            _nt_electrode_set_raw_signal(module->electrodes[electrode_last], (uint32_t)0xFFFF - (uint32_t)nstep1);
          }
        }
        else /* mutual-cap electrode was measured */
        {
          _nt_electrode_set_raw_signal(module->electrodes[electrode_last], nstep1);
          module->electrodes[electrode_last]->nstep = nstep1;
        }
            if ((bool)(electrode_last == 0U)) /* The last channel was read */
            {
                module->electrode_last              = module->electrodes_cnt; /* reset electrode number */
                tsi_lpwr_status.TSIScanCompleteFlag = 1;                      /* set the complete flag */
            }
            else
            {
                tsi_lpwr_status.TSIScanCompleteFlag = 0;
            }
        }
    }
    /* Find the next enabled electrode and assigned it for next channel measurement */
    if ((bool)(tsiState->opSatus != tsi_OpStatusSuspend))
    {
        while ((bool)electrode_last--)
        { /* Check if the electrode is enabled */
            if ((bool)(_nt_electrode_get_flag(module->electrodes[electrode_last], (uint32_t)NT_ELECTRODE_ENABLED)))
            { /* OS: End of critical section. */
                NT_OSA_MutexUnlock(&tsiState->lock);

                /* Disable TSI module, the same for self or mutual sensing */
                TSI_EnableModule(base, false);
                /* If this electrode is a self-cap mode electrode initialize self-cap sensing */
                if ((bool)(electrodes[electrode_last]->pin_input < TF_TSI_SELF_CAP_CHANNEL_COUNT))
                { /* Init self-cap sensing */
                    TSI_InitSelfCapMode(base, &module->electrodes[electrode_last]->tsi_hw_config->configSelfCap);
                    TSI_SetSelfCapMeasuredChannel(base, (uint8_t)electrodes[electrode_last]->pin_input);
                }
                else /* If this electrode is a mutual-cap mdde electrode, initialize mutual-cap sensing */
                {
                    /*Parse electrode number into rx, tx components*/
                    mutual_tx = (electrodes[electrode_last]->pin_input - TF_TSI_SELF_CAP_CHANNEL_COUNT) /
                                TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT;
                    mutual_rx = (electrodes[electrode_last]->pin_input - TF_TSI_SELF_CAP_CHANNEL_COUNT) %
                                TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT;
                    /* Init mutual-cap sensing */
                    TSI_InitMutualCapMode(base, &module->electrodes[electrode_last]->tsi_hw_config->configMutual);
                    TSI_SetMutualCapTxChannel(base, (tsi_mutual_tx_channel_t)mutual_tx);
                    TSI_SetMutualCapRxChannel(base, (tsi_mutual_rx_channel_t)mutual_rx);
                }
                TSI_EnableHardwareTriggerScan(base, false); /* Set HW trigger, the same for self or mutual sensing */
                TSI_EnableModule(base, true);               /* Enable TSI module, the same for self or mutual sensing */

                if ((bool)(tsiState->opSatus != tsi_OpStatusSuspend)) /* If TSI not Suspended */
                {
                    TSI_StartSoftwareTrigger(base); /* Set SW trigger, the same for self or mutual sensing */
                }
                module->electrode_last =
                    electrode_last; /* Store TSI channel triggered for sensing to be read in handler */
                return;             /* After new measurement assigned go out from handler to measure new sample */
            }
        }
    }
    /* If all samples have been measured execute following commands */
   if ((bool)(tsiState->pCallBackFunc))
   {
       tsiState->pCallBackFunc(instance, tsiState->usrData);
   }

    if ((bool)(tsiState->status != kStatus_TSI_LowPower))
    { /* Return status of the driver to initialized state */
        tsiState->status = kStatus_TSI_Initialized;
    }
}

#if (NT_SAFETY_SUPPORT == 1)
tsi_status_t NT_TSI_DRV_Recalibrate(uint32_t instance, void *configuration)
{
    NT_ASSERT(instance < (uint32_t)FSL_FEATURE_SOC_TSI_COUNT);
    NT_ASSERT(configuration != NULL);
    return kStatus_TSI_Success;
}
#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
