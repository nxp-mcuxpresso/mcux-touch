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
#ifndef _FSL_TSI_V5_DRIVER_SPECIFIC_H_
#define _FSL_TSI_V5_DRIVER_SPECIFIC_H_

/**
 * \ingroup nt_driver
 * \{
 */

#ifndef NT_FLASH_START
#ifndef NT_FLASH_END
#if defined(__IAR_SYSTEMS_ICC__)                        /* For IAR compiler   */
#elif defined(__CC_ARM)                                 /* For ARM(KEIL) version < 6 compiler */
extern uint32_t Load$$ER_m_text$$RO$$Base;
extern uint32_t Load$$ER_m_text$$RO$$Limit;
#elif defined(__MCUXPRESSO)                             /* For GCC compiler  MCUX IDE */
extern uint32_t __base_PROGRAM_FLASH;
extern uint32_t __top_PROGRAM_FLASH;
#elif defined(__GNUC__) && (__ARMCC_VERSION == 0)       /* For ARMGCC compiler */
extern uint32_t __data_end__;
extern uint32_t __data_start__;
#elif defined(__GNUC__) && (__ARMCC_VERSION >= 6010050) /* For ARM(KEIL) version >= 60 compiler */
extern uint32_t Load$$ER_m_text$$RO$$Base;
extern uint32_t Load$$ER_m_text$$RO$$Limit;
#else                                                   /* Other compiler used */
#warning "Unsupported compiler/IDE used !"
#endif
#endif /* NT_FLASH_END */
#endif /* NT_FLASH_START */
/**
 * Encapsulates SelfCap and Mutual configuration structure for TSI driver.
 *
 * Use an instance of this structure with NT_TSI_DRV_Init(). This allows you to configure the
 * most common settings of the TSI peripheral with a single function call. Settings include:
 *
 */
typedef struct
{
    tsi_selfCap_config_t configSelfCap;  /*!< Hardware configuration for self capacitance measurement */
    tsi_mutualCap_config_t configMutual; /*!< Hardware configuration for mutual capacitance measurement */
    uint16_t thresl;                     /*!< Low threshold for out-of-range interrupt (wake-up from low-power) */
    uint16_t thresh;                     /*!< High threshold for out-of-range interrupt (wake-up from low-power) */
    bool newCalc;                        /*!< Use new calculation for Self cap */
} tsi_config_t;

/*! @brief TSI status flags. */
typedef enum _tsi_sinc_status_flags
{
    kTSI_SwitchEnable     = TSI_SINC_SWITCH_ENABLE_MASK,      /*!< End-Of-Scan flag */
    kTSI_SincOverflowFlag = TSI_SINC_SINC_OVERFLOW_FLAG_MASK, /*!< Out-Of-Range flag */
    kTSI_SincValid        = TSI_SINC_SINC_VALID_MASK,         /*!< End-Of-Scan flag */
    kTSI_SscControlOut    = TSI_SINC_SSC_CONTROL_OUT_MASK     /*!< End-Of-Scan flag */
} tsi_sinc_status_flags_t;

/*! @brief TSI low power status flags. */
typedef struct _tsi_lpwr_status_flags
{
    uint8_t TSIScanCompleteFlag;
    uint8_t TSILowPower;
    uint8_t SelfLowPowerChannelBuff;
    uint8_t SelfLowPowerSavedFlag;
    uint16_t SelfLowPowerCountBuff;
} tsi_lpwr_status_flags_t;

/* TSIv6 from MCXN9xx */
#if !(defined(FSL_FEATURE_TSI_HAS_NO_SETCLK) && FSL_FEATURE_TSI_HAS_NO_SETCLK)
#define TF_TSI_SELF_CAP_CHANNEL_0  0U
#define TF_TSI_SELF_CAP_CHANNEL_1  1U
#define TF_TSI_SELF_CAP_CHANNEL_2  2U
#define TF_TSI_SELF_CAP_CHANNEL_3  3U
#define TF_TSI_SELF_CAP_CHANNEL_4  4U
#define TF_TSI_SELF_CAP_CHANNEL_5  5U
#define TF_TSI_SELF_CAP_CHANNEL_6  6U
#define TF_TSI_SELF_CAP_CHANNEL_7  7U
#define TF_TSI_SELF_CAP_CHANNEL_8  8U
#define TF_TSI_SELF_CAP_CHANNEL_9  9U
#define TF_TSI_SELF_CAP_CHANNEL_10 10U
#define TF_TSI_SELF_CAP_CHANNEL_11 11U
#define TF_TSI_SELF_CAP_CHANNEL_12 12U
#define TF_TSI_SELF_CAP_CHANNEL_13 13U
#define TF_TSI_SELF_CAP_CHANNEL_14 14U
#define TF_TSI_SELF_CAP_CHANNEL_15 15U
#define TF_TSI_SELF_CAP_CHANNEL_16 16U
#define TF_TSI_SELF_CAP_CHANNEL_17 17U
#define TF_TSI_SELF_CAP_CHANNEL_18 18U
#define TF_TSI_SELF_CAP_CHANNEL_19 19U
#define TF_TSI_SELF_CAP_CHANNEL_20 20U
#define TF_TSI_SELF_CAP_CHANNEL_21 21U
#define TF_TSI_SELF_CAP_CHANNEL_22 22U
#define TF_TSI_SELF_CAP_CHANNEL_23 23U
#define TF_TSI_SELF_CAP_CHANNEL_24 24U

#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT 17U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_8     8U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_9     9U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_10    10U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_11    11U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_12    12U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_13    13U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_14    14U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_15    15U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_16    16U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_17    17U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_18    18U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_19    19U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_20    20U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_21    21U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_22    22U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_23    23U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_24    24U

#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_COUNT 8U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_0     0U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_1     1U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_2     2U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_3     3U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_4     4U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_5     5U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_6     6U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_7     7U

#define TF_TSI_SELF_CAP_CHANNELS_MASK   0x0000000001FFFFFFULL
#define TF_TSI_MUTUAL_CAP_CHANNELS_MASK 0x1FFFFFFFFE000000ULL

#define TF_TSI_SELF_CAP_CHANNEL_COUNT 25U
#define TF_TSI_MUTUAL_CHANNEL_COUNT   (TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT * TF_TSI_MUTUAL_CAP_TX_CHANNEL_COUNT)
#define TF_TSI_TOTAL_CHANNEL_COUNT    (TF_TSI_MUTUAL_CHANNEL_COUNT + TF_TSI_SELF_CAP_CHANNEL_COUNT)

/* This macro transforms mutual RX, TX electrode numbers into a single electrode number */
#define NT_TSI_TRANSFORM_MUTUAL(RX, TX)                                                                   \
    (((0ULL + (TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT * (TX))) + ((RX)-TF_TSI_MUTUAL_CAP_TX_CHANNEL_COUNT)) + \
     TF_TSI_SELF_CAP_CHANNEL_COUNT)

#else /* TSIv6 from MCXA5xx */

#define TF_TSI_SELF_CAP_CHANNEL_0  0U
#define TF_TSI_SELF_CAP_CHANNEL_1  1U
#define TF_TSI_SELF_CAP_CHANNEL_2  2U
#define TF_TSI_SELF_CAP_CHANNEL_3  3U
#define TF_TSI_SELF_CAP_CHANNEL_4  4U
#define TF_TSI_SELF_CAP_CHANNEL_5  5U
#define TF_TSI_SELF_CAP_CHANNEL_6  6U
#define TF_TSI_SELF_CAP_CHANNEL_7  7U
#define TF_TSI_SELF_CAP_CHANNEL_8  8U
#define TF_TSI_SELF_CAP_CHANNEL_9  9U
#define TF_TSI_SELF_CAP_CHANNEL_10 10U
#define TF_TSI_SELF_CAP_CHANNEL_11 11U
#define TF_TSI_SELF_CAP_CHANNEL_12 12U
#define TF_TSI_SELF_CAP_CHANNEL_13 13U
#define TF_TSI_SELF_CAP_CHANNEL_14 14U
#define TF_TSI_SELF_CAP_CHANNEL_15 15U
#define TF_TSI_SELF_CAP_CHANNEL_16 16U
#define TF_TSI_SELF_CAP_CHANNEL_17 17U
#define TF_TSI_SELF_CAP_CHANNEL_18 18U
#define TF_TSI_SELF_CAP_CHANNEL_19 19U
#define TF_TSI_SELF_CAP_CHANNEL_20 20U
#define TF_TSI_SELF_CAP_CHANNEL_21 21U
#define TF_TSI_SELF_CAP_CHANNEL_22 22U
#define TF_TSI_SELF_CAP_CHANNEL_23 23U
#define TF_TSI_SELF_CAP_CHANNEL_24 24U
#define TF_TSI_SELF_CAP_CHANNEL_25 25U
#define TF_TSI_SELF_CAP_CHANNEL_26 26U
#define TF_TSI_SELF_CAP_CHANNEL_27 27U
#define TF_TSI_SELF_CAP_CHANNEL_28 28U
#define TF_TSI_SELF_CAP_CHANNEL_29 29U
#define TF_TSI_SELF_CAP_CHANNEL_30 30U
#define TF_TSI_SELF_CAP_CHANNEL_31 31U
#define TF_TSI_SELF_CAP_CHANNEL_32 32U
#define TF_TSI_SELF_CAP_CHANNEL_33 33U
#define TF_TSI_SELF_CAP_CHANNEL_34 34U
#define TF_TSI_SELF_CAP_CHANNEL_35 35U
#define TF_TSI_SELF_CAP_CHANNEL_36 36U
#define TF_TSI_SELF_CAP_CHANNEL_37 37U
#define TF_TSI_SELF_CAP_CHANNEL_38 38U
#define TF_TSI_SELF_CAP_CHANNEL_39 39U
#define TF_TSI_SELF_CAP_CHANNEL_40 40U
#define TF_TSI_SELF_CAP_CHANNEL_41 41U
#define TF_TSI_SELF_CAP_CHANNEL_42 42U
#define TF_TSI_SELF_CAP_CHANNEL_43 43U
#define TF_TSI_SELF_CAP_CHANNEL_44 44U
#define TF_TSI_SELF_CAP_CHANNEL_45 45U
#define TF_TSI_SELF_CAP_CHANNEL_46 46U
#define TF_TSI_SELF_CAP_CHANNEL_47 47U
#define TF_TSI_SELF_CAP_CHANNEL_48 48U
#define TF_TSI_SELF_CAP_CHANNEL_49 49U
#define TF_TSI_SELF_CAP_CHANNEL_50 50U
#define TF_TSI_SELF_CAP_CHANNEL_51 51U
#define TF_TSI_SELF_CAP_CHANNEL_52 52U
#define TF_TSI_SELF_CAP_CHANNEL_53 53U
#define TF_TSI_SELF_CAP_CHANNEL_54 54U
#define TF_TSI_SELF_CAP_CHANNEL_55 55U
#define TF_TSI_SELF_CAP_CHANNEL_56 56U
#define TF_TSI_SELF_CAP_CHANNEL_57 57U
#define TF_TSI_SELF_CAP_CHANNEL_58 58U
#define TF_TSI_SELF_CAP_CHANNEL_59 59U
#define TF_TSI_SELF_CAP_CHANNEL_60 60U
#define TF_TSI_SELF_CAP_CHANNEL_61 61U
#define TF_TSI_SELF_CAP_CHANNEL_62 62U
#define TF_TSI_SELF_CAP_CHANNEL_63 63U
#define TF_TSI_SELF_CAP_CHANNEL_64 64U
#define TF_TSI_SELF_CAP_CHANNEL_65 65U
#define TF_TSI_SELF_CAP_CHANNEL_66 66U
#define TF_TSI_SELF_CAP_CHANNEL_67 67U
#define TF_TSI_SELF_CAP_CHANNEL_68 68U
#define TF_TSI_SELF_CAP_CHANNEL_69 69U

#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT 70U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_0  0U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_1  1U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_2  2U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_3  3U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_4  4U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_5  5U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_6  6U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_7  7U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_8  8U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_9  9U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_10 10U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_11 11U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_12 12U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_13 13U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_14 14U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_15 15U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_16 16U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_17 17U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_18 18U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_19 19U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_20 20U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_21 21U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_22 22U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_23 23U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_24 24U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_25 25U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_26 26U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_27 27U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_28 28U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_29 29U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_30 30U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_31 31U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_32 32U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_33 33U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_34 34U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_35 35U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_36 36U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_37 37U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_38 38U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_39 39U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_40 40U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_41 41U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_42 42U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_43 43U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_44 44U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_45 45U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_46 46U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_47 47U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_48 48U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_49 49U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_50 50U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_51 51U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_52 52U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_53 53U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_54 54U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_55 55U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_56 56U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_57 57U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_58 58U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_59 59U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_60 60U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_61 61U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_62 62U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_63 63U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_64 64U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_65 65U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_66 66U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_67 67U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_68 68U
#define TF_TSI_MUTUAL_CAP_RX_CHANNEL_69 69U

#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_COUNT 70U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_0  0U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_1  1U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_2  2U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_3  3U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_4  4U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_5  5U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_6  6U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_7  7U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_8  8U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_9  9U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_10 10U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_11 11U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_12 12U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_13 13U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_14 14U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_15 15U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_16 16U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_17 17U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_18 18U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_19 19U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_20 20U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_21 21U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_22 22U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_23 23U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_24 24U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_25 25U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_26 26U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_27 27U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_28 28U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_29 29U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_30 30U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_31 31U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_32 32U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_33 33U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_34 34U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_35 35U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_36 36U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_37 37U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_38 38U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_39 39U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_40 40U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_41 41U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_42 42U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_43 43U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_44 44U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_45 45U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_46 46U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_47 47U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_48 48U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_49 49U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_50 50U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_51 51U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_52 52U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_53 53U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_54 54U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_55 55U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_56 56U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_57 57U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_58 58U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_59 59U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_60 60U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_61 61U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_62 62U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_63 63U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_64 64U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_65 65U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_66 66U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_67 67U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_68 68U
#define TF_TSI_MUTUAL_CAP_TX_CHANNEL_69 69U

#define TF_TSI_SELF_CAP_CHANNEL_COUNT 70U
#define TF_TSI_MUTUAL_CHANNEL_COUNT (TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT * TF_TSI_MUTUAL_CAP_TX_CHANNEL_COUNT)
#define TF_TSI_TOTAL_CHANNEL_COUNT  (TF_TSI_MUTUAL_CHANNEL_COUNT + TF_TSI_SELF_CAP_CHANNEL_COUNT)

/* This macro transforms mutual RX, TX electrode numbers into a single electrode number */
#define NT_TSI_TRANSFORM_MUTUAL(RX, TX)                                                                   \
    (((0ULL + (TF_TSI_MUTUAL_CAP_RX_CHANNEL_COUNT * (TX))) + (RX)) + \
     TF_TSI_SELF_CAP_CHANNEL_COUNT)
#endif /* End of MCXA5xx */

void TSI_DRV_IRQHandler(uint32_t instance);

extern volatile tsi_lpwr_status_flags_t tsi_lpwr_status;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

/* \} */
#endif /* _FSL_TSI_V5_DRIVER_SPECIFIC_H_ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
