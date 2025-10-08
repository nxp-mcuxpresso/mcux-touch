/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <stdio.h>

//#include "board.h"

#include "main.h"

#ifdef CONFIG_LOG
	#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
	#include <zephyr/logging/log.h>
	LOG_MODULE_REGISTER(app);
#else
	#undef  LOG_ERR
	#define LOG_ERR(...)
#endif


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* for threads */
#define THREAD_STACKSIZE	  512
#define THREAD_PRIORITY_TOUCH	1	/* Touch Sensor thread highest priority */

#define TSI_DEV_PRIO  2       /* device uses interrupt priority 2 */
#define TSI_IRQ_FLAGS 0       /* IRQ flags */

#define NUM_STEPS	50U       /* for PWM */

#define nt_printf(...) /* do nothing - the debug lines are used by FreeMASTER */

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)


/*******************************************************************************
 * Global Variables
 ******************************************************************************/
//Put unknown symbols here Just to satisfy the compiler/linker
uint32_t __data_start__ = 0;
uint32_t __data_end__ = 0;

struct k_timer my_timer;

// TODO: Add pwm-led1,2 to board.DTS
//#define PWM_LEDS
#ifdef PWM_LEDS
  static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
  static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));
  static const struct pwm_dt_spec pwm_led2 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led2));
#else
  static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
  static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#endif

// NXP touch static pool
uint8_t nt_memory_pool[5000] __attribute__((aligned(4))); /* GCC compiler */

uint32_t brightness_global, hue_angle_global;
uint8_t duty_cycle_red, duty_cycle_green, duty_cycle_blue;
tsi_status_t recalib_status;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SetHueBrightness(uint32_t hue_angle, uint32_t brightness);
extern void nt_trigger_handler(struct k_timer *timer_id);

extern void TSI_DRV_IRQHandler(uint32_t instance);
void TSI0_IRQHandler(void);
void TSI1_IRQHandler(void);

/*******************************************************************************
 * Touch lib callbacks
 ******************************************************************************/
static void keypad_callback(const struct nt_control *control, enum nt_control_keypad_event event, uint32_t index);
static void aslider_callback(const struct nt_control *control, enum nt_control_aslider_event event, uint32_t position);
static void arotary_callback(const struct nt_control *control, enum nt_control_arotary_event event, uint32_t position);
/* Call when the TSI counter overflows 65535 */
static void system_callback(uint32_t event, union nt_system_event_context *context);


// Zephyr touch lib scan trigger
void nt_trigger_handler(struct k_timer *dummy)
{
    nt_trigger();
}


int main(void)
{
	int32_t ret;
	int32_t result;
    bool recalib_enabled = false; /* autotunning is off */
    bool one_key_only    = false; /* one key only valid is off */

	printk("Touch sensing Demo\r\n");

#ifdef PWM_LEDS
//----------------- LED PWM init ------------------------------------------------------------
   	if ((!pwm_is_ready_dt(&pwm_led0))||(!pwm_is_ready_dt(&pwm_led1))||(!pwm_is_ready_dt(&pwm_led2))) 
    {
		printk("Error: PWM device is not ready\n");
		return 0;
	}

#else
   // Just use LED blink instead of PWM
   if ((!gpio_is_ready_dt(&led0))||(!gpio_is_ready_dt(&led1))) {
     return 0;
   }
#endif


//----------------- NXP Touch lib init goes here --------------------------------------------
    if ((result = nt_init(&System_0, nt_memory_pool, sizeof(nt_memory_pool))) != NT_SUCCESS)
    {
        /* red colour signalizes the error, to solve is increase nt_memory_pool or debug it */
        hue_angle_global  = 0;
        brightness_global = 120;
        SetHueBrightness(hue_angle_global, brightness_global);
        
        switch (result)
        {
            case NT_FAILURE:
                nt_printf("\nCannot initialize NXP Touch due to a non-specific error.\n");
                break;
            case NT_OUT_OF_MEMORY:
                nt_printf("\nCannot initialize NXP Touch due to a lack of free memory.\n");
                printf("\nCannot initialize NXP Touch due to a non-specific error.\n");
                break;
        }
    while(1); /* add code to handle this error */
    }
    /* Get free memory size of the nt_memory_pool  */
    volatile uint32_t free_mem;
    free_mem = nt_mem_get_free_size();

    nt_printf("\nNXP Touch is successfully initialized.\n");
    nt_printf("Unused memory: %d bytes, you can make the memory pool smaller without affecting the functionality.\n",
              free_mem);
    printf("Unused memory: %d bytes, you can make the memory pool smaller without affecting the functionality.\n",
          (int)free_mem);

    /* Enable electrodes and controls */
    nt_enable();

/* Disable FRDM-TOUCH board electrodes and controls if FRDM-TOUCH board is not connected */
#if (NT_FRDM_TOUCH_SUPPORT) == 0
    nt_electrode_disable(&El_3);
    nt_electrode_disable(&El_4);
    nt_electrode_disable(&El_5);
    nt_electrode_disable(&El_6);
    nt_electrode_disable(&El_7);
    nt_electrode_disable(&El_8);
    nt_electrode_disable(&El_9);
    nt_electrode_disable(&El_10);
    nt_electrode_disable(&El_11);
    nt_electrode_disable(&El_12);
#endif

    /* Keypad electrodes*/
    nt_control_keypad_set_autorepeat_rate(&Keypad_1, 100, 1000);
    nt_control_keypad_register_callback(&Keypad_1, &keypad_callback);

    /* Slider electrodes */
    nt_control_aslider_register_callback(&ASlider_2, &aslider_callback);

    /* Rotary electrodes */
    nt_control_arotary_register_callback(&ARotary_3, &arotary_callback);

    /* System TSI overflow warning callback */
    nt_system_register_callback(&system_callback);

    if (one_key_only)
        nt_control_keypad_only_one_key_valid(&Keypad_1, true);

//----------------- Zephyr stuffs went here --------------------------------------
    // Zephyr timer init
    k_timer_init(&my_timer, nt_trigger_handler, NULL);

    // Set NXP touch trigger period according to the HW scan time needed
    k_timer_start(&my_timer, K_MSEC(50), K_MSEC(50));

#if DT_NODE_EXISTS(DT_NODELABEL(tsi0))
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(tsi0)), TSI_DEV_PRIO, TSI0_IRQHandler, NULL, TSI_IRQ_FLAGS);
    irq_enable(DT_IRQN(DT_NODELABEL(tsi0)));
#endif

#if DT_NODE_EXISTS(DT_NODELABEL(tsi1))
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(tsi1)), TSI_DEV_PRIO, TSI1_IRQHandler, NULL, TSI_IRQ_FLAGS);
    irq_enable(DT_IRQN(DT_NODELABEL(tsi1)));
#endif
//---------------------------------------------------------------------------------
	while (1)
	{   
        nt_task(); 
		k_sleep(K_MSEC(1)); // Go sleep to allow other tasks
	}
}

void SystemInitHook(void)
{

}

static void keypad_callback(const struct nt_control *control, enum nt_control_keypad_event event, uint32_t index)
{
    int32_t ret;
    
    switch (event)
    {
        case NT_KEYPAD_RELEASE:

            switch (index)
            {
                case 0:
#ifndef PWM_LEDS
                ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_HIGH);
#endif
                    break;
                case 1:
#ifndef PWM_LEDS
                ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_HIGH);
#endif
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;
                default:
                    break;
            }
            break;
        case NT_KEYPAD_TOUCH:

            switch (index)
            {
                case 0:
                    /* YELLOW on, full brightness */
#ifndef PWM_LEDS
                    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_LOW);
#endif
                    hue_angle_global  = 12;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                case 1:
#ifndef PWM_LEDS
                    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_LOW);
#endif               
                    /* CYAN on, full brightness */
                    hue_angle_global  = 36;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                case 2:
                    /* RED on, full brightness */
                    hue_angle_global  = 0;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                case 3:
                    /* GREEN on, full brightness */
                    hue_angle_global  = 24;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                case 4:
                    /* BLUE on, full brightness */
                    hue_angle_global  = 48;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                case 5:
                    /* WHITE on, full brightness */
                    hue_angle_global  = 73;
                    brightness_global = 120;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                default:
                    break;
            }
            break;

        case NT_KEYPAD_AUTOREPEAT:

            switch (index)
            {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    break;
                default:
                    break;
            }
            break;

        case NT_KEYPAD_MULTI_TOUCH:

            switch (index)
            {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                case 5:
                    /* LED off +  switch xtalk_reduction*/
                    brightness_global = 0;
                    SetHueBrightness(hue_angle_global, brightness_global);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

static void aslider_callback(const struct nt_control *control, enum nt_control_aslider_event event, uint32_t position)
{
    switch (event)
    {
        case NT_ASLIDER_INITIAL_TOUCH:
            break;
        case NT_ASLIDER_MOVEMENT:
            break;
        case NT_ASLIDER_ALL_RELEASE:
            break;
        default:
            break;
    }

    /* Recalculate and set RGB LED */
    brightness_global = position;
    SetHueBrightness(hue_angle_global, brightness_global);
}

static void arotary_callback(const struct nt_control *control, enum nt_control_arotary_event event, uint32_t position)
{
    switch (event)
    {
        case NT_AROTARY_MOVEMENT:
            break;
        case NT_AROTARY_ALL_RELEASE:
            break;
        case NT_AROTARY_INITIAL_TOUCH:
            break;
        default:
            break;
    }

    /* Recalculate and set RGB LED */
    hue_angle_global = position;
    SetHueBrightness(hue_angle_global, brightness_global);
}

/* Call on the TSI CNTR overflow 16-bit range (65535) */
void system_callback(uint32_t event, union nt_system_event_context *context)
{
    switch (event)
    {
        case NT_SYSTEM_EVENT_OVERRUN:
        {
          /* red colour signalize the error, to solve it increase nt_kernel_data.rom->time_period  */
          hue_angle_global  = 0;
          brightness_global = 120;
          SetHueBrightness(hue_angle_global, brightness_global); 
          nt_printf("\n Overrun occurred increase nt_kernel_data.rom->time_period param \n");
          printf("\n Overrun occurred increase nt_kernel_data.rom->time_period param \n");
        }
        break;
        case NT_SYSTEM_EVENT_DATA_READY:
            // your code
        break;
        case NT_SYSTEM_EVENT_MODULE_DATA_READY:
            // your code
        break;
        case NT_SYSTEM_EVENT_DATA_OVERFLOW:
            // your code
        break;
        case NT_SYSTEM_EVENT_SIGNAL_LOW:
            // your code
        break;
        case NT_SYSTEM_EVENT_SIGNAL_HIGH:
            // your code
        break;
        case NT_SYSTEM_EVENT_ELEC_SHORT_VDD:
            // your code
        break;
        case NT_SYSTEM_EVENT_ELEC_SHORT_GND:
            // your code
        break;
        case NT_SYSTEM_EVENT_ELEC_SHORT_ADJ:
            // your code
        break;
    }
}

/*!
 * @brief Function calculating RGB values based on Hue and Brightness
 */
static void SetHueBrightness(uint32_t hue_angle, uint32_t brightness)
{
    /* Calculate Hue */
    if (hue_angle <= 24)
    {
        duty_cycle_red   = 24 - hue_angle;
        duty_cycle_green = hue_angle;
        duty_cycle_blue  = 0;
    }
    if (hue_angle > 24 && hue_angle <= 48)
    {
        duty_cycle_red   = 0;
        duty_cycle_green = 48 - hue_angle;
        duty_cycle_blue  = hue_angle - 24;
    }
    if (hue_angle > 48)
    {
        duty_cycle_red   = hue_angle - 48;
        duty_cycle_green = 0;
        duty_cycle_blue  = 72 - hue_angle;
    }

    /* If hue_angle > 72, set RGB to white */
    if (hue_angle > 72)
    {
        duty_cycle_red   = 16;
        duty_cycle_green = 16;
        duty_cycle_blue  = 16;
    }

    /* Slider never returns position < 60 */
    brightness = brightness < 60 ? 0 : (brightness - 60);

#ifdef PWM_LEDS
    /* Set Hue multiplied by Brightness */
    pwm_set_pulse_dt(&pwm_led0, (duty_cycle_blue * brightness));
    pwm_set_pulse_dt(&pwm_led1, (duty_cycle_green * brightness));
    pwm_set_pulse_dt(&pwm_led2, (duty_cycle_green * brightness));
#endif
}

void TSI0_IRQHandler(void)
 {
    TSI_DRV_IRQHandler(0);
 }

void TSI1_IRQHandler(void)
 {
     TSI_DRV_IRQHandler(1);
 }