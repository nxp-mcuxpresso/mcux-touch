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

// #include "board.h"

#include "main.h"

#ifdef CONFIG_LOG
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);
#else
#undef LOG_ERR
#define LOG_ERR(...)
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* for threads */
#define THREAD_STACKSIZE 512
#define THREAD_PRIORITY_TOUCH 1 /* Touch Sensor thread highest priority */

#define TSI_DEV_PRIO 0  /* device uses interrupt priority 0 (highest) */
#define TSI_IRQ_FLAGS 0 /* IRQ flags */

#define NUM_STEPS 50U /* for PWM */

#define nt_printf(...) /* do nothing - the debug lines are used by FreeMASTER */

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
// Put unknown symbols here Just to satisfy the compiler/linker
uint32_t __data_start__ = 0;
uint32_t __data_end__ = 0;

struct k_timer my_timer;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

// NXP touch static pool
uint8_t nt_memory_pool[5000] __attribute__((aligned(4))); /* GCC compiler */

uint32_t brightness_global, hue_angle_global;
uint8_t duty_cycle_red, duty_cycle_green, duty_cycle_blue;
tsi_status_t recalib_status;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
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

// Led controls
int led_red_status = 0;
#define LED_RED_ON() (led_red_status = 1, gpio_pin_configure_dt(&led0, GPIO_OUTPUT_LOW))
#define LED_RED_OFF() (led_red_status = 0, gpio_pin_configure_dt(&led0, GPIO_OUTPUT_HIGH))
#define LED_RED_TOGGLE() (led_red_status == 0 ? LED_RED_ON() : LED_RED_OFF())

int led_green_status = 0;
#define LED_GREEN_ON() (led_green_status = 1, gpio_pin_configure_dt(&led1, GPIO_OUTPUT_LOW))
#define LED_GREEN_OFF() (led_green_status = 0, gpio_pin_configure_dt(&led1, GPIO_OUTPUT_HIGH))
#define LED_GREEN_TOGGLE() (led_green_status == 0 ? LED_GREEN_ON() : LED_GREEN_OFF())

int led_blue_status = 0;
#define LED_BLUE_ON() (led_blue_status = 1, gpio_pin_configure_dt(&led2, GPIO_OUTPUT_LOW))
#define LED_BLUE_OFF() (led_blue_status = 0, gpio_pin_configure_dt(&led2, GPIO_OUTPUT_HIGH))
#define LED_BLUE_TOGGLE() (led_blue_status == 0 ? LED_BLUE_ON() : LED_BLUE_OFF())

// Zephyr touch lib scan trigger
void nt_trigger_handler(struct k_timer *dummy)
{
    nt_trigger();
}

int main(void)
{
    int32_t result;
    // bool recalib_enabled = false; /* autotunning is off */
    bool one_key_only = false; /* one key only valid is off */

    printk("Touch sensing Demo\r\n");

    if ((!gpio_is_ready_dt(&led0)) || (!gpio_is_ready_dt(&led1)) || (!gpio_is_ready_dt(&led2)))
    {
        return 0;
    }

    //----------------- NXP Touch lib init goes here --------------------------------------------
    if ((result = nt_init(&System_0, nt_memory_pool, sizeof(nt_memory_pool))) != NT_SUCCESS)
    {
        /* red colour signalizes the error, to solve is increase nt_memory_pool or debug it */
        LED_RED_ON();

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
        while (1)
            ; /* add code to handle this error */
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
    k_timer_start(&my_timer, K_MSEC(10), K_MSEC(10));

#if DT_NODE_EXISTS(DT_NODELABEL(tsi0))
    IRQ_DIRECT_CONNECT(DT_IRQN(DT_NODELABEL(tsi0)), TSI_DEV_PRIO, TSI0_IRQHandler, TSI_IRQ_FLAGS);
    irq_enable(DT_IRQN(DT_NODELABEL(tsi0)));
#endif

#if DT_NODE_EXISTS(DT_NODELABEL(tsi1))
    IRQ_DIRECT_CONNECT(DT_IRQN(DT_NODELABEL(tsi1)), TSI_DEV_PRIO, TSI1_IRQHandler, TSI_IRQ_FLAGS);
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
    switch (event)
    {
    case NT_KEYPAD_RELEASE:
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_OFF();
        break;
    case NT_KEYPAD_TOUCH:

        switch (index)
        {
        case 0:
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_OFF();

            break;
        case 1:
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_OFF();
            break;
        case 2:
            LED_RED_ON();
            LED_GREEN_ON();
            LED_BLUE_OFF();
            break;
        case 3:
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_ON();
            break;
        case 4:
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_ON();
            break;
        case 5:
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_ON();
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
        nt_printf("\n Touch: %d", position);
        if (position < 10)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_OFF();
        }
        else if ((position >= 10) && (position < 20))
        {
            LED_RED_ON();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 20) && (position < 30))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 30) && (position < 40))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_ON();
        }
        else if ((position >= 40) && (position < 50))
        {
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }        
        else if (position >= 50)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }
        break;
    case NT_ASLIDER_MOVEMENT:
        nt_printf("\n Movement: %d", position);
        if (position < 10)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_OFF();
        }
        else if ((position >= 10) && (position < 20))
        {
            LED_RED_ON();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 20) && (position < 30))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 30) && (position < 40))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_ON();
        }
        else if ((position >= 40) && (position < 50))
        {
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }        
        else if (position >= 50)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }
        break;
    case NT_ASLIDER_ALL_RELEASE:
        nt_printf("\n Release: %d", position);
        LED_RED_OFF();
        LED_GREEN_OFF();
        LED_BLUE_OFF();
        break;
    }
}

static void arotary_callback(const struct nt_control *control, enum nt_control_arotary_event event, uint32_t position)
{
    switch (event)
    {
    case NT_AROTARY_MOVEMENT:
        if (position < 10)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_OFF();
        }
        else if ((position >= 10) && (position < 20))
        {
            LED_RED_ON();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 20) && (position < 30))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 30) && (position < 40))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_ON();
        }
        else if ((position >= 40) && (position < 50))
        {
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }        
        else if (position >= 50)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }
        break;
    case NT_AROTARY_ALL_RELEASE:
        LED_RED_OFF();
        LED_GREEN_OFF();
        LED_BLUE_OFF();
        break;
    case NT_AROTARY_INITIAL_TOUCH:
        if (position < 10)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_OFF();
        }
        else if ((position >= 10) && (position < 20))
        {
            LED_RED_ON();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 20) && (position < 30))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_OFF();
        }
        else if ((position >= 30) && (position < 40))
        {
            LED_RED_OFF();
            LED_GREEN_ON();
            LED_BLUE_ON();
        }
        else if ((position >= 40) && (position < 50))
        {
            LED_RED_OFF();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }        
        else if (position >= 50)
        {
            LED_RED_ON();
            LED_GREEN_OFF();
            LED_BLUE_ON();
        }
        break;
    default:
        break;
    }
}

/* Call on the TSI CNTR overflow 16-bit range (65535) */
void system_callback(uint32_t event, union nt_system_event_context *context)
{
    switch (event)
    {
    case NT_SYSTEM_EVENT_OVERRUN:
    {
        /* red colour signalize the error, to solve it increase nt_kernel_data.rom->time_period  */
        // LED_RED_ON();
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

void TSI0_IRQHandler(void)
{
    TSI_DRV_IRQHandler(0);
}

void TSI1_IRQHandler(void)
{
    TSI_DRV_IRQHandler(1);
}