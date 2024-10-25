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

#include "nt_system.h"
#include "../source/system/nt_system_mem_prv.h"
#include "../source/system/nt_system_frmstr_prv.h"
#include "nt_controls.h"
#include "../source/controls/nt_controls_prv.h"
#include "nt_electrodes.h"
#include "../source/electrodes/nt_electrodes_prv.h"
#include "nt_gpio.h"
#include "nt_safety.h"

/*! @brief Array of port clocks */
#if (defined(PORT_CLOCKS) && (FSL_FEATURE_TSI_VERSION == 5))
static const clock_ip_name_t s_portClocks[] = PORT_CLOCKS;
#else
/* PORT not defined, therefore grounding or safety feature not work" */ 
#endif

/* internal function */
struct nt_control_data *_nt_control_get_data(const struct nt_control *control)
{
    NT_ASSERT(control != NULL);
    uint32_t i = 0;

    const struct nt_control *const *controls = _nt_system_get()->rom->controls;

    while (*controls != NULL)
    {
        if (*controls == control)
        {
            return _nt_system_get()->controls[i];
        }
        i++;
        controls++;
    }
    return NULL;
}

int32_t _nt_control_check_data(const struct nt_control_data *control)
{
    int32_t result = (int32_t)NT_SUCCESS;

    if (control->rom == NULL)
    {
        result = (int32_t)NT_FAILURE;
    }

    else if (control->rom->interface == NULL)
    {
        result = (int32_t)NT_FAILURE;
    }
    else if (control->rom->electrodes == NULL)
    {
        result = (int32_t)NT_FAILURE;
    }
    else
    { /* no command to avoid Misra issue */
    }

    if (control->data.general == NULL)
    {
        result = (int32_t)NT_FAILURE;
    }

    return result;
}

struct nt_control_data *_nt_control_init(const struct nt_control *control)
{
    NT_ASSERT(control != NULL);

    struct nt_control_data *_this = _nt_mem_alloc(sizeof(struct nt_control_data));

    if (_this == NULL)
    {
        return NULL;
    }

    _this->rom             = control;
    _this->electrodes_size = (uint8_t)nt_control_count_electrodes(_this->rom);

    _this->electrodes = _nt_mem_alloc((uint32_t)sizeof(struct nt_electrode_data *) * (uint32_t)_this->electrodes_size);

    if (_this->electrodes == NULL)
    {
        return NULL;
    }

    uint32_t i;
    for (i = 0; i < _this->electrodes_size; i++)
    {
        _this->electrodes[i] = _nt_electrode_get_data(control->electrodes[i]);
    }

    if (control->interface->init != NULL)
    {
        if (control->interface->init(_this) < (int32_t)NT_SUCCESS)
        {
            return NULL; /* failure stops the entire init phase */
        }
    }

    /* Adjacent channel grounding */
    #if (defined(PORT_CLOCKS) && (FSL_FEATURE_TSI_VERSION == 5))
    if(control->adjacent_electrodes == kAdj_grounding)
    {
        bool pta0_used = false;

        for(i = 0; i < _this->electrodes_size; ++i)
        {
            bool elec_is_mutual = _this->electrodes[i]->rom->pin_input >= TF_TSI_SELF_CAP_CHANNEL_COUNT ? true : false;
            uint32_t gpio1      = _this->electrodes[i]->rom->gpio_input.gpio1;
            uint32_t gpio2      = _this->electrodes[i]->rom->gpio_input.gpio2;
            
            /* If electrode's GPIO input pins are incorrectly configurated */
            if ((gpio1 == gpio2) && (elec_is_mutual))
            {
                /* Mutual-cap electrode with same Tx and Rx channel */
                return NULL;
            }
            if ((gpio1 == 0) && (!elec_is_mutual))
            { 
                if(pta0_used == true)
                {
                    /* Already used Self-cap electrode with GPIO_PTA0 input pin */
                    return NULL;
                }
                pta0_used = true;
            }

            /* Enable clock for Self-cap mode electrode or Mutual-cap mode electrode's Rx channel */
            CLOCK_EnableClock(s_portClocks[gpio1 >> NT_GPIO_PORT_SHIFT]);
            if (elec_is_mutual)
            {
                /* Enable clock for Mutual-cap mode electrode's Tx channel */
                CLOCK_EnableClock(s_portClocks[gpio2 >> NT_GPIO_PORT_SHIFT]);
            }     
            
            /* Set electrodes's used TSI pins to GPIO output low level to improve TSI sensing */
            if (_nt_module_safety_switch_electrode_gpio_output_set_low(_this->electrodes[i]->rom) != (int32_t)NT_SUCCESS)
            {
                return NULL;
            }
            
            /* Set flag for enabled electrode's channel grounding */
            _nt_electrode_set_flag(_this->electrodes[i], (uint32_t)NT_ELECTRODE_EN_GROUND_ADJACENT_FLAG);
        }
    }
    #if defined(FSL_FEATURE_TSI_HAS_SHIELD_REGISTER) && FSL_FEATURE_TSI_HAS_SHIELD_REGISTER 
    else if(control->adjacent_electrodes == kAdj_shielding)
    {
        for(i = 0; i < _this->electrodes_size; ++i)
        {   
            /* Set flag for enabled electrode's channel grounding */
            _nt_electrode_set_flag(_this->electrodes[i], (uint32_t)NT_ELECTRODE_EN_SHIELD_ADJACENT_FLAG);
            /* If electrode shield mask is shielding yourself -  is incorrectly configurated */
            uint32_t shield_mask = _this->electrodes[i]->rom->shield_mask;
            uint32_t pin_input = _this->electrodes[i]->rom->pin_input;
            if (pin_input <= TF_TSI_SELF_CAP_CHANNEL_COUNT)
            {
                shield_mask = shield_mask & (1U<<pin_input);
                if ((bool)shield_mask)
                {
                    return NULL; 
                }    
            } 
        }

    }
    #endif /* FSL_FEATURE_TSI_HAS_SHIELD_REGISTER */
    #endif /* defined(PORT_CLOCKS) && (FSL_FEATURE_TSI_VERSION == 5) */
    
    if ((bool)_nt_control_check_data(_this) != (bool)NT_SUCCESS)
    {
        return NULL;
    }

    if ((bool)_nt_freemaster_add_variable(control->interface->name, "nt_control_interface",
                                          (const void *)control->interface,
                                          sizeof(struct nt_control_interface)) != (bool)NT_SUCCESS)
    {
        return NULL;
    }

    return _this;
}

void nt_control_enable(const struct nt_control *control)
{
    NT_ASSERT(control != NULL);

    struct nt_control_data *control_data = _nt_control_get_data(control);
    NT_ASSERT(control_data != NULL);

    _nt_control_set_flag(control_data, (int32_t)NT_CONTROL_EN_FLAG);
}

void nt_control_disable(const struct nt_control *control)
{
    NT_ASSERT(control != NULL);

    struct nt_control_data *control_data = _nt_control_get_data(control);
    NT_ASSERT(control_data != NULL);

    _nt_control_clear_flag(control_data, (int32_t)NT_CONTROL_EN_FLAG);
}

int32_t nt_control_get_touch_button(const struct nt_control *control, uint32_t index)
{
    NT_ASSERT(control != NULL);
    struct nt_control_data *control_data = _nt_control_get_data(control);
    NT_ASSERT((control_data != NULL));

    uint32_t elec_counter = control_data->electrodes_size;

    for (uint32_t i = index; i < elec_counter; i++)
    {
        if ((bool)(_nt_electrode_get_last_status(control_data->electrodes[i]) == (int32_t)NT_ELECTRODE_STATE_TOUCH))
        {
            return (int32_t)i;
        }
    }
    return (int32_t)NT_FAILURE;
}

uint64_t _nt_control_get_electrodes_state(struct nt_control_data *control)
{
    NT_ASSERT(control != NULL);
    uint32_t elec_counter  = control->electrodes_size;
    uint64_t current_state = 0U;

    while ((bool)(elec_counter--))
    {
        uint64_t electrode_state = _nt_electrode_is_touched(control->electrodes[elec_counter]);
        /* all elec status in a bit field */
        current_state |= (electrode_state << elec_counter);
    }
    return current_state;
}

uint32_t _nt_control_get_electrodes_digital_state(struct nt_control_data *control)
{
    NT_ASSERT(control != NULL);
    uint32_t elec_counter  = control->electrodes_size;
    uint32_t current_state = 0U;

    while ((bool)(elec_counter--))
    {
        /* all electrode status in a bit field */
        if ((bool)_nt_electrode_get_flag(
                control->electrodes[elec_counter],
                (uint32_t)NT_ELECTRODE_DIGITAL_RESULT_ONLY_FLAG | (uint32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG))
        {
            current_state |= (uint32_t)1 << elec_counter;
        }
    }
    return current_state;
}

uint64_t nt_control_get_electrodes_state(struct nt_control *control)
{
    NT_ASSERT(control != NULL);
    struct nt_control_data *control_data = _nt_control_get_data(control);
    NT_ASSERT(control_data != NULL);

    return _nt_control_get_electrodes_state(control_data);
}

/* Internal function */
void _nt_control_set_flag_all_elec(struct nt_control_data *control, uint32_t flags)
{
    NT_ASSERT(control != NULL);
    uint32_t elec_counter = control->electrodes_size;

    while ((bool)(elec_counter--))
    {
        _nt_electrode_set_flag(control->electrodes[elec_counter], flags);
    }
}

/* Internal function */
void _nt_control_clear_flag_all_elec(struct nt_control_data *control, uint32_t flag)
{
    NT_ASSERT(control != NULL);
    uint32_t elec_counter = control->electrodes_size;

    while ((bool)(elec_counter--))
    {
        _nt_electrode_clear_flag(control->electrodes[elec_counter], flag);
    }
}

int32_t _nt_control_check_neighbours_electrodes(struct nt_control_data *control,
                                                uint32_t first,
                                                uint32_t second,
                                                uint32_t overrun)
{
    uint32_t result;
    uint32_t elec_size = control->electrodes_size;

    if (first > second)
    {
        result = first - second;
    }
    else
    {
        result = second - first;
    }

    if ((bool)overrun)
    {
        if ((bool)(result > 1U) && (bool)(result == (elec_size - 1U)))
        {
            result = 1; /* valid for arotary */
        }
    }

    return (result > 1U) ? (int32_t)NT_INVALID_RESULT : (int32_t)NT_SUCCESS;
}

int32_t _nt_control_check_edge_electrodes(struct nt_control_data *control, uint32_t electrode_ix)
{
    uint32_t elec_size = control->electrodes_size;

    if ((bool)(electrode_ix == 0U) || (bool)(electrode_ix == (elec_size - 1U)))
    {
        return (int32_t)NT_SUCCESS;
    }

    return (int32_t)NT_INVALID_RESULT;
}

uint32_t nt_control_count_electrodes(const struct nt_control *control)
{
    NT_ASSERT(control != NULL);
    uint32_t elec_counter = 0U;

    while (control->electrodes[elec_counter] != NULL)
    {
        elec_counter++;
    }

    return elec_counter;
}

const struct nt_electrode *nt_control_get_electrode(const struct nt_control *control, uint32_t index)
{
    NT_ASSERT((control != NULL));
    struct nt_control_data *control_data = _nt_control_get_data(control);
    NT_ASSERT((control_data != NULL));

    if (control_data->electrodes_size > index)
    {
        return control->electrodes[index];
    }
    else
    {
        return NULL;
    }
}

uint32_t _nt_control_get_first_elec_touched(uint64_t current_state)
{
    NT_ASSERT(current_state != 0U);
    uint32_t first_elec_index = 0U;

    while (((current_state >> first_elec_index) & 0x1U) == 0U)
    {
        first_elec_index++;
    }
    return first_elec_index;
}

uint32_t _nt_control_get_last_elec_touched(uint64_t current_state)
{
    NT_ASSERT(current_state != 0U);
    uint32_t last_elec_index = 0U;

    while ((bool)(current_state >> last_elec_index))
    {
        last_elec_index++;
    }
    return last_elec_index;
}

uint32_t _nt_control_get_touch_count(uint64_t current_state)
{
    uint32_t touch_count = 0U;
    uint32_t i_mask      = 1U;

    while ((bool)current_state)
    {
        if ((bool)(current_state & i_mask))
        {
            current_state &= ~(uint64_t)i_mask;
            touch_count++;
        }
        i_mask <<= 1U;
    }
    return touch_count;
}
