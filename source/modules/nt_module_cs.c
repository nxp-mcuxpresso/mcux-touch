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


#include "../source/system/nt_system_prv.h"
#include "../source/modules/nt_module_cs_prv.h"
#include "../source/electrodes/nt_electrodes_prv.h"
#include "nt_modules.h"
#include "nt_module_cs.h"
#include "../source/filters/nt_filters_prv.h"
#include "../source/modules/nt_modules_prv.h"

/* Call back function of MCUXpresso SDK CS driver */
static void _nt_module_cs_driver_callback(uint32_t instance, void *usrData);

/* Local functions */
static void _nt_module_cs_measure(struct nt_module_data *module);
static int32_t _nt_module_cs_init(struct nt_module_data *module);
static int32_t _nt_module_cs_trigger(struct nt_module_data *module);
static int32_t _nt_module_cs_process(struct nt_module_data *module);
static int32_t _nt_module_cs_electrode_enable(struct nt_module_data *module, uint32_t elec_index);
static int32_t _nt_module_cs_electrode_disable(struct nt_module_data *module, uint32_t elec_index);
static int32_t _nt_module_cs_change_mode(struct nt_module_data *module,
                                         const enum nt_module_mode mode,
                                         const struct nt_electrode *electrode);

static int32_t _nt_module_cs_load_configuration(struct nt_module_data *module,
                                                const enum nt_module_mode mode,
                                                const void *config); /*!<  Load the configuration for select mode. */
static int32_t _nt_module_cs_save_configuration(struct nt_module_data *module,
                                                const enum nt_module_mode mode,
                                                void *config); /*!<  Save the configuration of select mode. */

static int32_t _nt_get_cs_mode(const enum nt_module_mode mode);

/** interface cs module */
const struct nt_module_interface nt_module_cs_interface = {
    .init               = _nt_module_cs_init,
    .trigger            = _nt_module_cs_trigger,
    .process            = _nt_module_cs_process,
    .electrode_enable   = _nt_module_cs_electrode_enable,
    .electrode_disable  = _nt_module_cs_electrode_disable,
    .change_mode        = _nt_module_cs_change_mode,
    .load_configuration = _nt_module_cs_load_configuration,
    .save_configuration = _nt_module_cs_save_configuration,
    .name               = NT_MODULE_CS_NAME,
    .params_size        = sizeof(struct nt_module_cs_params),
};

/*******************************************************************************
 *                       CS MODULE functions
 *******************************************************************************/
static int32_t _nt_module_cs_init(struct nt_module_data *module)
{
    module->special_data.cs = _nt_mem_alloc(sizeof(struct nt_module_cs_data));

    if (module->special_data.cs == NULL)
    {
        return NT_OUT_OF_MEMORY;
    }

    nt_cs_user_config_t cs_config;

    /* Create the CS structure */
    cs_config.config        = module->rom->config;
    cs_config.pCallBackFunc = _nt_module_cs_driver_callback;
    cs_config.usrData       = (void *)_nt_system_get();

    if (NT_CS_DRV_Init(module->rom->instance, &module->special_data.cs->cs_state, &cs_config) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    return NT_SUCCESS;
}

static int32_t _nt_module_cs_trigger(struct nt_module_data *module)
{
    cs_status_t result = kStatus_CS_Success;

    if (result == kStatus_CS_Success)
    {
        result = NT_CS_DRV_Measure(module->rom->instance);
    }

    switch (result)
    {
        case kStatus_CS_Success:
            return NT_SUCCESS;

        case kStatus_CS_Busy:
            return NT_SCAN_IN_PROGRESS;

        default:
            return NT_FAILURE;
    }
}

static void _nt_module_cs_measure(struct nt_module_data *module)
{
    uint32_t elec_counter = module->electrodes_cnt;
    while (elec_counter--)
    {
        struct nt_electrode_data *elec = module->electrodes[elec_counter];
        uint32_t raw_signal            = _nt_electrode_get_raw_signal(elec);

        elec->rom->keydetector_interface->nt_keydetector_measure(elec, raw_signal);
    }
}

static int32_t _nt_module_cs_process(struct nt_module_data *module)
{
    uint32_t elec_counter;

    _nt_module_cs_measure(module);
    uint32_t cap_process = 1U;

    if (cap_process)
    {
        _nt_module_clear_flag(module, NT_MODULE_DIGITAL_RESULTS_FLAG);
        elec_counter = module->electrodes_cnt;

        while (elec_counter--)
        {
            struct nt_electrode_data *elec = module->electrodes[elec_counter];
            elec->rom->keydetector_interface->nt_keydetector_process(elec);
            _nt_electrode_clear_flag(elec, NT_ELECTRODE_DIGITAL_RESULT_ONLY_FLAG);
        }
    }
    return NT_SUCCESS;
}

static int32_t _nt_module_cs_electrode_enable(struct nt_module_data *module, const uint32_t elec_index)
{
    if (NT_CS_DRV_EnableElectrode(module->rom->instance, elec_index, true) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    return NT_SUCCESS;
}

static int32_t _nt_module_cs_electrode_disable(struct nt_module_data *module, const uint32_t elec_index)
{
    if (NT_CS_DRV_EnableElectrode(module->rom->instance, elec_index, false) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    return NT_SUCCESS;
}

static int32_t _nt_get_cs_mode(const enum nt_module_mode mode)
{
    struct modes_cross_table
    {
        enum nt_module_mode nt_modes;
        nt_cs_modes_t cs_modes;
    };

    const struct modes_cross_table modes_table[3] = {
        {NT_MODULE_MODE_NORMAL, cs_OpModeNormal},
        {NT_MODULE_MODE_PROXIMITY, cs_OpModeProximity},
        {NT_MODULE_MODE_LOW_POWER, cs_OpModeLowPower},
    };

    for (int32_t mode_ix = 0; mode_ix < 3; mode_ix++)
    {
        if (modes_table[mode_ix].nt_modes == mode)
        {
            return modes_table[mode_ix].cs_modes;
        }
    }

    return NT_FAILURE;
}

static int32_t _nt_module_cs_change_mode(struct nt_module_data *module,
                                         const enum nt_module_mode mode,
                                         const struct nt_electrode *electrode)
{
    NT_ASSERT(module != NULL);
    NT_ASSERT(electrode != NULL);

    int32_t cs_mode = _nt_get_cs_mode(mode);

    if (cs_mode == NT_FAILURE)
    {
        return NT_FAILURE;
    }

    if (NT_CS_DRV_GetMode(module->rom->instance) == cs_OpModeLowPower)
    {
        /* Disable the low power mode and switch back driver to normal mode */
        if (NT_CS_DRV_DisableLowPower(module->rom->instance, cs_OpModeNoChange) != kStatus_CS_Success)
        {
            return NT_FAILURE;
        }
    }

    /* Change mode of CS driver */
    if (NT_CS_DRV_ChangeMode(module->rom->instance, (nt_cs_modes_t)cs_mode) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    if ((cs_mode == cs_OpModeLowPower) || (cs_mode == cs_OpModeProximity))
    {
        /* Enable the right electrode for Low Power and Proximity */
        if (NT_CS_DRV_EnableElectrode(module->rom->instance, electrode->pin_input, true) != kStatus_CS_Success)
        {
            return NT_FAILURE;
        }
    }

    /* It should be in separate API to allow setup/recalibrate/manage low power */
    if (cs_mode == cs_OpModeLowPower)
    {
        /* Enable the low power functionality */
        if (NT_CS_DRV_EnableLowPower(module->rom->instance) != kStatus_CS_Success)
        {
            return NT_FAILURE;
        }
    }

    return NT_SUCCESS;
}

static int32_t _nt_module_cs_load_configuration(struct nt_module_data *module,
                                                const enum nt_module_mode mode,
                                                const void *config)
{
    NT_ASSERT(module != NULL);
    NT_ASSERT(config != NULL);

    int32_t cs_mode = _nt_get_cs_mode(mode);

    if (cs_mode == NT_FAILURE)
    {
        return NT_FAILURE;
    }

    nt_cs_operation_mode_t cs_op_mode;

    cs_op_mode.enabledElectrodes = NT_CS_DRV_GetEnabledElectrodes(module->rom->instance);
    cs_op_mode.config            = *((cs_config_t *)config);

    if (NT_CS_DRV_LoadConfiguration(module->rom->instance, (nt_cs_modes_t)cs_mode, &cs_op_mode) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    return NT_SUCCESS;
}

static int32_t _nt_module_cs_save_configuration(struct nt_module_data *module,
                                                const enum nt_module_mode mode,
                                                void *config)
{
    NT_ASSERT(module != NULL);
    NT_ASSERT(config != NULL);

    int32_t cs_mode = _nt_get_cs_mode(mode);

    if (cs_mode == NT_FAILURE)
    {
        return NT_FAILURE;
    }

    nt_cs_operation_mode_t cs_op_mode;

    if (NT_CS_DRV_SaveConfiguration(module->rom->instance, (nt_cs_modes_t)cs_mode, &cs_op_mode) != kStatus_CS_Success)
    {
        return NT_FAILURE;
    }

    *((cs_config_t *)config) = cs_op_mode.config;

    return NT_SUCCESS;
}

static void _nt_module_cs_driver_callback(uint32_t instance, void *usrData)
{
    struct nt_module_data *module =
        _nt_module_get_data(_nt_system_get_module((uint32_t)&nt_module_cs_interface, instance));
    NT_ASSERT(module != NULL);
    struct nt_electrode_data *elec;
    const uint8_t enabled_electrodes = NT_CS_DRV_GetEnabledElectrodes(instance);
    uint32_t elec_size               = module->electrodes_cnt;

    while (elec_size--)
    {
        elec = module->electrodes[elec_size];

        if (enabled_electrodes & (0x01U << (elec->rom->pin_input)))
        {
            uint16_t data;
            NT_CS_DRV_GetCounter(instance, elec->rom->pin_input, &data);
            _nt_electrode_set_raw_signal(elec, data);
        }
    }

    if (module->special_data.cs->cs_state.status == kStatus_CS_Overflow)
    {
        _nt_module_set_flag(module, NT_MODULE_OVERFLOW_FLAG);
        _nt_system_invoke_callback(NT_SYSTEM_EVENT_DATA_OVERFLOW, NULL);
    }

    _nt_module_set_flag(module, NT_MODULE_NEW_DATA_FLAG);
    _nt_system_modules_data_ready(module);
}

void delay(void)
{
    volatile int j = 0;
    volatile int k = 0;

    for (j = 0; j < 100; j++)
        for (k = 0; k < 2000; k++)
        {
            CUSTOM_DELAY();
        }
}
