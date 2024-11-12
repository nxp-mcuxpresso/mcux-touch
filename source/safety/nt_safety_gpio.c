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


#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#if (FSL_FEATURE_TSI_VERSION == 5)
#include "fsl_ftm.h"
#endif
#include "nt.h"
#include "fsl_clock.h"

#if (FSL_FEATURE_TSI_VERSION == 5) 
#define USED_FTM           0
#define BOARD_FTM_BASEADDR FTM0

FGPIO_Type *g_gpioBaseAddr[] = FGPIO_BASE_PTRS;
PORT_Type *g_portBaseAddr[]  = PORT_BASE_PTRS;
FTM_Type *g_ftmBaseAddr[]    = FTM_BASE_PTRS;
#define FTM_BASE_ADDR (g_ftmBaseAddr[USED_FTM])

/* Set pin direction */
void set_pin_dir(FGPIO_Type *base, uint32_t pin, const gpio_pin_config_t *config);

/* Configure pin as gpio output */
void set_pin_ouput(uint32_t port, uint32_t pin)
{
    gpio_pin_config_t pin_cfg = {
        kGPIO_DigitalOutput,
        0,
    };

    set_pin_dir(g_gpioBaseAddr[port], pin, &pin_cfg);
}

/* Configure pin as gpio input. */
void set_pin_input(uint32_t port, uint32_t pin)
{
    gpio_pin_config_t pin_cfg = {
        kGPIO_DigitalInput,
        0,
    };

    set_pin_dir(g_gpioBaseAddr[port], pin, &pin_cfg);
}

/* Configure pin value as logic 0. */
void set_pin_low(uint32_t port, uint32_t pin)
{
    FGPIO_PinWrite(g_gpioBaseAddr[port], pin, 0U);
}

/* Configure pin value as logic 1. */
void set_pin_high(uint32_t port, uint32_t pin)
{
    FGPIO_PinWrite(g_gpioBaseAddr[port], pin, 1U);
}

/* Reads the pin value. */
uint32_t get_pin_value(uint32_t port, uint32_t pin)
{
    return FGPIO_PinRead(g_gpioBaseAddr[port], pin);
}

/* Configure pin as gpio output and set output to logic 1.  */
void set_pin_default_state(uint32_t port, uint32_t pin)
{
    set_pin_ouput(port, pin);
    set_pin_high(port, pin);
}

/* Initialize the FlexTimer.*/
void init_timer(void)
{
    ftm_config_t ftmInfo;

    FTM_GetDefaultConfig(&ftmInfo);
    (void)(status_t) FTM_Init(FTM_BASE_ADDR, &ftmInfo);

    /* Set the timer to be in free-running mode */
    BOARD_FTM_BASEADDR->MOD = 0xFFFF;
    BOARD_FTM_BASEADDR->CNT = 0U;
}

/* Start the FlexTimer. */
void start_timer(void)
{
    FTM_StartTimer(FTM_BASE_ADDR, kFTM_SystemClock);
    FTM_ClearStatusFlags(FTM_BASE_ADDR, (uint32_t)kFTM_TimeOverflowFlag);
}

/* Stop the FlexTimer.*/
void stop_timer(void)
{
    FTM_StopTimer(FTM_BASE_ADDR);
}

/* Reset the FlexTimer. */
void timer_reset_counter(void)
{
    BOARD_FTM_BASEADDR->CNT = 0U;
}

/* Reads FlexTimer counted value. */
uint32_t timer_get_counter(void)
{
    uint32_t counter_val;
    counter_val = BOARD_FTM_BASEADDR->CNT;
    return counter_val;
}

/* Reads FlexTimer overflow.*/
uint32_t timer_get_overrun(void)
{
    /* check and return TOF flag */
    if ((bool)(FTM_GetStatusFlags(FTM_BASE_ADDR) & (uint16_t)kFTM_TimeOverflowFlag))
    {
        return 1U;
    }
    else
    {
        return 0U;
    }
}

/* Reads FlexTimer overrun. */
void set_pin_pull_up(uint32_t port, uint32_t pin)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullUp, /* pull-up */
        (uint16_t)kPORT_PassiveFilterDisable,
        (uint16_t)kPORT_LowDriveStrength,
        (uint16_t)kPORT_MuxAsGpio,
        (uint16_t)kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(g_portBaseAddr[port], pin, &pcr_cfg);
}

/* Configures pin to have pull down resistor. */
void set_pin_pull_down(uint32_t port, uint32_t pin)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullDown, /* pull-down */
        (uint16_t)kPORT_PassiveFilterDisable,
        (uint16_t)kPORT_LowDriveStrength,
        (uint16_t)kPORT_MuxAsGpio,
        (uint16_t)kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(g_portBaseAddr[port], pin, &pcr_cfg);
}

/* Disable pin pull resistor. */
void dis_pin_pull(uint32_t port, uint32_t pin)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullDisable, /* pull-disable */
        (uint16_t)kPORT_PassiveFilterDisable, (uint16_t)kPORT_LowDriveStrength, (uint16_t)kPORT_MuxAsGpio,
        (uint16_t)kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(g_portBaseAddr[port], pin, &pcr_cfg);
}

/* Configure pin as TSI peripheral pin. */
void set_pin_tsi_mode(uint32_t port, uint32_t pin)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullDisable,      (uint16_t)kPORT_PassiveFilterDisable,
        (uint16_t)kPORT_LowDriveStrength, (uint16_t)kPORT_PinDisabledOrAnalog, /* Analog = TSI */
        (uint16_t)kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(g_portBaseAddr[port], pin, &pcr_cfg);
}

/* Set GPIO mode */
void set_pin_gpio_mode(uint32_t port, uint32_t pin)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullDisable,      (uint16_t)kPORT_PassiveFilterDisable,
        (uint16_t)kPORT_LowDriveStrength, (uint16_t)kPORT_MuxAsGpio, /* GPIO */
        (uint16_t)kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(g_portBaseAddr[port], pin, &pcr_cfg);
}

/* GPIO interface structure */
const struct nt_module_gpio_user_interface gpio_interface = {
    .set_pin_output_ptr        = set_pin_ouput,
    .set_pin_input_ptr         = set_pin_input,
    .set_pin_low_ptr           = set_pin_low,
    .set_pin_high_ptr          = set_pin_high,
    .get_pin_value_ptr         = get_pin_value,
    .set_pin_default_state_ptr = set_pin_default_state,
    .init_timer_ptr            = init_timer,
    .start_timer_ptr           = start_timer,
    .stop_timer_ptr            = stop_timer,
    .timer_reset_counter_ptr   = timer_reset_counter,
    .timer_get_counter_ptr     = timer_get_counter,
    .timer_get_overrun_ptr     = timer_get_overrun,
    .set_pin_pull_up_ptr       = set_pin_pull_up,
    .set_pin_pull_down_ptr     = set_pin_pull_down,
    .set_pin_tsi_mode_ptr      = set_pin_tsi_mode,
    .set_pin_gpio_mode_ptr     = set_pin_gpio_mode,
};

/* Configure pin as gpio output. */
void configure_gpio_touch_sensing_pins(uint32_t instance)
{
    port_pin_config_t pcr_cfg = {
        (uint16_t)kPORT_PullDisable, (uint16_t)kPORT_PassiveFilterDisable, (uint16_t)kPORT_HighDriveStrength,
        (uint16_t)kPORT_MuxAsGpio,   (uint16_t)kPORT_UnlockRegister,
    };
    switch (instance)
    {
        case 0: /* configuration 0 */
            /* Affects PORTA_PCR4 register */
            PORT_SetPinConfig(PORTA, 4u, &pcr_cfg);
            /* Affects PORTB_PCR3 register */
            PORT_SetPinConfig(PORTB, 3u, &pcr_cfg);
            /* Affects PORTB_PCR2 register */
            PORT_SetPinConfig(PORTB, 2u, &pcr_cfg);
            /* Affects PORTB_PCR16 register */
            PORT_SetPinConfig(PORTB, 16u, &pcr_cfg);
            break;
        default:
            /*MISRA rule 16.4*/
            break;
    }
}

/* Set pin direction */
void set_pin_dir(FGPIO_Type *base, uint32_t pin, const gpio_pin_config_t *config)
{
    if (config->pinDirection == kGPIO_DigitalInput)
    {
        base->PDDR &= ~(1UL << pin);
    }
    else
    {
        base->PDDR |= (1UL << pin);
    }
}
#endif
