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

#ifndef _NT_GPIO_H_
#define _NT_GPIO_H_

/** Gpio scan states
 */
enum nt_gpio_scan_states
{
    NT_GPIO_MEASURED    = 0,
    NT_GPIO_IN_PROGRESS = 1,
};

/** Gpio module flags
 */
enum nt_gpio_flags
{
    NT_GPIO_INTERRUPTED_FLAG = NT_FLAGS_SPECIFIC_SHIFT(0),
};

#define NT_GPIO_PORT_SHIFT (16)

/** Gpio pin name symbols. Each symbol contains a port and a pin number.
 */
enum nt_gpio_pinname
{
    GPIO_PTA0  = ((0 << NT_GPIO_PORT_SHIFT) | 0),
    GPIO_PTA1  = ((0 << NT_GPIO_PORT_SHIFT) | 1),
    GPIO_PTA2  = ((0 << NT_GPIO_PORT_SHIFT) | 2),
    GPIO_PTA3  = ((0 << NT_GPIO_PORT_SHIFT) | 3),
    GPIO_PTA4  = ((0 << NT_GPIO_PORT_SHIFT) | 4),
    GPIO_PTA5  = ((0 << NT_GPIO_PORT_SHIFT) | 5),
    GPIO_PTA6  = ((0 << NT_GPIO_PORT_SHIFT) | 6),
    GPIO_PTA7  = ((0 << NT_GPIO_PORT_SHIFT) | 7),
    GPIO_PTA8  = ((0 << NT_GPIO_PORT_SHIFT) | 8),
    GPIO_PTA9  = ((0 << NT_GPIO_PORT_SHIFT) | 9),
    GPIO_PTA10 = ((0 << NT_GPIO_PORT_SHIFT) | 10),
    GPIO_PTA11 = ((0 << NT_GPIO_PORT_SHIFT) | 11),
    GPIO_PTA12 = ((0 << NT_GPIO_PORT_SHIFT) | 12),
    GPIO_PTA13 = ((0 << NT_GPIO_PORT_SHIFT) | 13),
    GPIO_PTA14 = ((0 << NT_GPIO_PORT_SHIFT) | 14),
    GPIO_PTA15 = ((0 << NT_GPIO_PORT_SHIFT) | 15),
    GPIO_PTA16 = ((0 << NT_GPIO_PORT_SHIFT) | 16),
    GPIO_PTA17 = ((0 << NT_GPIO_PORT_SHIFT) | 17),
    GPIO_PTA18 = ((0 << NT_GPIO_PORT_SHIFT) | 18),
    GPIO_PTA19 = ((0 << NT_GPIO_PORT_SHIFT) | 19),
    GPIO_PTA20 = ((0 << NT_GPIO_PORT_SHIFT) | 20),
    GPIO_PTA21 = ((0 << NT_GPIO_PORT_SHIFT) | 21),
    GPIO_PTA22 = ((0 << NT_GPIO_PORT_SHIFT) | 22),
    GPIO_PTA23 = ((0 << NT_GPIO_PORT_SHIFT) | 23),
    GPIO_PTA24 = ((0 << NT_GPIO_PORT_SHIFT) | 24),
    GPIO_PTA25 = ((0 << NT_GPIO_PORT_SHIFT) | 25),
    GPIO_PTA26 = ((0 << NT_GPIO_PORT_SHIFT) | 26),
    GPIO_PTA27 = ((0 << NT_GPIO_PORT_SHIFT) | 27),
    GPIO_PTA28 = ((0 << NT_GPIO_PORT_SHIFT) | 28),
    GPIO_PTA29 = ((0 << NT_GPIO_PORT_SHIFT) | 29),
    GPIO_PTA30 = ((0 << NT_GPIO_PORT_SHIFT) | 30),
    GPIO_PTA31 = ((0 << NT_GPIO_PORT_SHIFT) | 31),
    GPIO_PTB0  = ((1 << NT_GPIO_PORT_SHIFT) | 0),
    GPIO_PTB1  = ((1 << NT_GPIO_PORT_SHIFT) | 1),
    GPIO_PTB2  = ((1 << NT_GPIO_PORT_SHIFT) | 2),
    GPIO_PTB3  = ((1 << NT_GPIO_PORT_SHIFT) | 3),
    GPIO_PTB4  = ((1 << NT_GPIO_PORT_SHIFT) | 4),
    GPIO_PTB5  = ((1 << NT_GPIO_PORT_SHIFT) | 5),
    GPIO_PTB6  = ((1 << NT_GPIO_PORT_SHIFT) | 6),
    GPIO_PTB7  = ((1 << NT_GPIO_PORT_SHIFT) | 7),
    GPIO_PTB8  = ((1 << NT_GPIO_PORT_SHIFT) | 8),
    GPIO_PTB9  = ((1 << NT_GPIO_PORT_SHIFT) | 9),
    GPIO_PTB10 = ((1 << NT_GPIO_PORT_SHIFT) | 10),
    GPIO_PTB11 = ((1 << NT_GPIO_PORT_SHIFT) | 11),
    GPIO_PTB12 = ((1 << NT_GPIO_PORT_SHIFT) | 12),
    GPIO_PTB13 = ((1 << NT_GPIO_PORT_SHIFT) | 13),
    GPIO_PTB14 = ((1 << NT_GPIO_PORT_SHIFT) | 14),
    GPIO_PTB15 = ((1 << NT_GPIO_PORT_SHIFT) | 15),
    GPIO_PTB16 = ((1 << NT_GPIO_PORT_SHIFT) | 16),
    GPIO_PTB17 = ((1 << NT_GPIO_PORT_SHIFT) | 17),
    GPIO_PTB18 = ((1 << NT_GPIO_PORT_SHIFT) | 18),
    GPIO_PTB19 = ((1 << NT_GPIO_PORT_SHIFT) | 19),
    GPIO_PTB20 = ((1 << NT_GPIO_PORT_SHIFT) | 20),
    GPIO_PTB21 = ((1 << NT_GPIO_PORT_SHIFT) | 21),
    GPIO_PTB22 = ((1 << NT_GPIO_PORT_SHIFT) | 22),
    GPIO_PTB23 = ((1 << NT_GPIO_PORT_SHIFT) | 23),
    GPIO_PTB24 = ((1 << NT_GPIO_PORT_SHIFT) | 24),
    GPIO_PTB25 = ((1 << NT_GPIO_PORT_SHIFT) | 25),
    GPIO_PTB26 = ((1 << NT_GPIO_PORT_SHIFT) | 26),
    GPIO_PTB27 = ((1 << NT_GPIO_PORT_SHIFT) | 27),
    GPIO_PTB28 = ((1 << NT_GPIO_PORT_SHIFT) | 28),
    GPIO_PTB29 = ((1 << NT_GPIO_PORT_SHIFT) | 29),
    GPIO_PTB30 = ((1 << NT_GPIO_PORT_SHIFT) | 30),
    GPIO_PTB31 = ((1 << NT_GPIO_PORT_SHIFT) | 31),
    GPIO_PTC0  = ((2 << NT_GPIO_PORT_SHIFT) | 0),
    GPIO_PTC1  = ((2 << NT_GPIO_PORT_SHIFT) | 1),
    GPIO_PTC2  = ((2 << NT_GPIO_PORT_SHIFT) | 2),
    GPIO_PTC3  = ((2 << NT_GPIO_PORT_SHIFT) | 3),
    GPIO_PTC4  = ((2 << NT_GPIO_PORT_SHIFT) | 4),
    GPIO_PTC5  = ((2 << NT_GPIO_PORT_SHIFT) | 5),
    GPIO_PTC6  = ((2 << NT_GPIO_PORT_SHIFT) | 6),
    GPIO_PTC7  = ((2 << NT_GPIO_PORT_SHIFT) | 7),
    GPIO_PTC8  = ((2 << NT_GPIO_PORT_SHIFT) | 8),
    GPIO_PTC9  = ((2 << NT_GPIO_PORT_SHIFT) | 9),
    GPIO_PTC10 = ((2 << NT_GPIO_PORT_SHIFT) | 10),
    GPIO_PTC11 = ((2 << NT_GPIO_PORT_SHIFT) | 11),
    GPIO_PTC12 = ((2 << NT_GPIO_PORT_SHIFT) | 12),
    GPIO_PTC13 = ((2 << NT_GPIO_PORT_SHIFT) | 13),
    GPIO_PTC14 = ((2 << NT_GPIO_PORT_SHIFT) | 14),
    GPIO_PTC15 = ((2 << NT_GPIO_PORT_SHIFT) | 15),
    GPIO_PTC16 = ((2 << NT_GPIO_PORT_SHIFT) | 16),
    GPIO_PTC17 = ((2 << NT_GPIO_PORT_SHIFT) | 17),
    GPIO_PTC18 = ((2 << NT_GPIO_PORT_SHIFT) | 18),
    GPIO_PTC19 = ((2 << NT_GPIO_PORT_SHIFT) | 19),
    GPIO_PTC20 = ((2 << NT_GPIO_PORT_SHIFT) | 20),
    GPIO_PTC21 = ((2 << NT_GPIO_PORT_SHIFT) | 21),
    GPIO_PTC22 = ((2 << NT_GPIO_PORT_SHIFT) | 22),
    GPIO_PTC23 = ((2 << NT_GPIO_PORT_SHIFT) | 23),
    GPIO_PTC24 = ((2 << NT_GPIO_PORT_SHIFT) | 24),
    GPIO_PTC25 = ((2 << NT_GPIO_PORT_SHIFT) | 25),
    GPIO_PTC26 = ((2 << NT_GPIO_PORT_SHIFT) | 26),
    GPIO_PTC27 = ((2 << NT_GPIO_PORT_SHIFT) | 27),
    GPIO_PTC28 = ((2 << NT_GPIO_PORT_SHIFT) | 28),
    GPIO_PTC29 = ((2 << NT_GPIO_PORT_SHIFT) | 29),
    GPIO_PTC30 = ((2 << NT_GPIO_PORT_SHIFT) | 30),
    GPIO_PTC31 = ((2 << NT_GPIO_PORT_SHIFT) | 31),
    GPIO_PTD0  = ((3 << NT_GPIO_PORT_SHIFT) | 0),
    GPIO_PTD1  = ((3 << NT_GPIO_PORT_SHIFT) | 1),
    GPIO_PTD2  = ((3 << NT_GPIO_PORT_SHIFT) | 2),
    GPIO_PTD3  = ((3 << NT_GPIO_PORT_SHIFT) | 3),
    GPIO_PTD4  = ((3 << NT_GPIO_PORT_SHIFT) | 4),
    GPIO_PTD5  = ((3 << NT_GPIO_PORT_SHIFT) | 5),
    GPIO_PTD6  = ((3 << NT_GPIO_PORT_SHIFT) | 6),
    GPIO_PTD7  = ((3 << NT_GPIO_PORT_SHIFT) | 7),
    GPIO_PTD8  = ((3 << NT_GPIO_PORT_SHIFT) | 8),
    GPIO_PTD9  = ((3 << NT_GPIO_PORT_SHIFT) | 9),
    GPIO_PTD10 = ((3 << NT_GPIO_PORT_SHIFT) | 10),
    GPIO_PTD11 = ((3 << NT_GPIO_PORT_SHIFT) | 11),
    GPIO_PTD12 = ((3 << NT_GPIO_PORT_SHIFT) | 12),
    GPIO_PTD13 = ((3 << NT_GPIO_PORT_SHIFT) | 13),
    GPIO_PTD14 = ((3 << NT_GPIO_PORT_SHIFT) | 14),
    GPIO_PTD15 = ((3 << NT_GPIO_PORT_SHIFT) | 15),
    GPIO_PTD16 = ((3 << NT_GPIO_PORT_SHIFT) | 16),
    GPIO_PTD17 = ((3 << NT_GPIO_PORT_SHIFT) | 17),
    GPIO_PTD18 = ((3 << NT_GPIO_PORT_SHIFT) | 18),
    GPIO_PTD19 = ((3 << NT_GPIO_PORT_SHIFT) | 19),
    GPIO_PTD20 = ((3 << NT_GPIO_PORT_SHIFT) | 20),
    GPIO_PTD21 = ((3 << NT_GPIO_PORT_SHIFT) | 21),
    GPIO_PTD22 = ((3 << NT_GPIO_PORT_SHIFT) | 22),
    GPIO_PTD23 = ((3 << NT_GPIO_PORT_SHIFT) | 23),
    GPIO_PTD24 = ((3 << NT_GPIO_PORT_SHIFT) | 24),
    GPIO_PTD25 = ((3 << NT_GPIO_PORT_SHIFT) | 25),
    GPIO_PTD26 = ((3 << NT_GPIO_PORT_SHIFT) | 26),
    GPIO_PTD27 = ((3 << NT_GPIO_PORT_SHIFT) | 27),
    GPIO_PTD28 = ((3 << NT_GPIO_PORT_SHIFT) | 28),
    GPIO_PTD29 = ((3 << NT_GPIO_PORT_SHIFT) | 29),
    GPIO_PTD30 = ((3 << NT_GPIO_PORT_SHIFT) | 30),
    GPIO_PTD31 = ((3 << NT_GPIO_PORT_SHIFT) | 31),
    GPIO_PTE0  = ((4 << NT_GPIO_PORT_SHIFT) | 0),
    GPIO_PTE1  = ((4 << NT_GPIO_PORT_SHIFT) | 1),
    GPIO_PTE2  = ((4 << NT_GPIO_PORT_SHIFT) | 2),
    GPIO_PTE3  = ((4 << NT_GPIO_PORT_SHIFT) | 3),
    GPIO_PTE4  = ((4 << NT_GPIO_PORT_SHIFT) | 4),
    GPIO_PTE5  = ((4 << NT_GPIO_PORT_SHIFT) | 5),
    GPIO_PTE6  = ((4 << NT_GPIO_PORT_SHIFT) | 6),
    GPIO_PTE7  = ((4 << NT_GPIO_PORT_SHIFT) | 7),
    GPIO_PTE8  = ((4 << NT_GPIO_PORT_SHIFT) | 8),
    GPIO_PTE9  = ((4 << NT_GPIO_PORT_SHIFT) | 9),
    GPIO_PTE10 = ((4 << NT_GPIO_PORT_SHIFT) | 10),
    GPIO_PTE11 = ((4 << NT_GPIO_PORT_SHIFT) | 11),
    GPIO_PTE12 = ((4 << NT_GPIO_PORT_SHIFT) | 12),
    GPIO_PTE13 = ((4 << NT_GPIO_PORT_SHIFT) | 13),
    GPIO_PTE14 = ((4 << NT_GPIO_PORT_SHIFT) | 14),
    GPIO_PTE15 = ((4 << NT_GPIO_PORT_SHIFT) | 15),
    GPIO_PTE16 = ((4 << NT_GPIO_PORT_SHIFT) | 16),
    GPIO_PTE17 = ((4 << NT_GPIO_PORT_SHIFT) | 17),
    GPIO_PTE18 = ((4 << NT_GPIO_PORT_SHIFT) | 18),
    GPIO_PTE19 = ((4 << NT_GPIO_PORT_SHIFT) | 19),
    GPIO_PTE20 = ((4 << NT_GPIO_PORT_SHIFT) | 20),
    GPIO_PTE21 = ((4 << NT_GPIO_PORT_SHIFT) | 21),
    GPIO_PTE22 = ((4 << NT_GPIO_PORT_SHIFT) | 22),
    GPIO_PTE23 = ((4 << NT_GPIO_PORT_SHIFT) | 23),
    GPIO_PTE24 = ((4 << NT_GPIO_PORT_SHIFT) | 24),
    GPIO_PTE25 = ((4 << NT_GPIO_PORT_SHIFT) | 25),
    GPIO_PTE26 = ((4 << NT_GPIO_PORT_SHIFT) | 26),
    GPIO_PTE27 = ((4 << NT_GPIO_PORT_SHIFT) | 27),
    GPIO_PTE28 = ((4 << NT_GPIO_PORT_SHIFT) | 28),
    GPIO_PTE29 = ((4 << NT_GPIO_PORT_SHIFT) | 29),
    GPIO_PTE30 = ((4 << NT_GPIO_PORT_SHIFT) | 30),
    GPIO_PTE31 = ((4 << NT_GPIO_PORT_SHIFT) | 31),
};

#endif
