/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	mt_tx_pwr.h
*/
#ifndef __MT_TX_PWR_H__
#define __MT_TX_PWR_H__

/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/


/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

#define BF_BACKOFF_CONTROL_REGISTER_SIZE      10
#define POWER_UP_CATEGORY_RATE_NUM            12

/*******************************************************************************
 *    MACRO
 ******************************************************************************/


/*******************************************************************************
 *    TYPES
 ******************************************************************************/

struct MT_TX_PWR_CAP {
#define INTERNAL_PA 0
#define EXTERNAL_PA 1
	UINT8 pa_type;
#define TSSI_TRIGGER_STAGE 0
#define TSSI_COMP_STAGE 1
#define TSSI_CAL_STAGE 2
	UINT8 tssi_stage;
#define TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE 0x84
#define TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE 0x83
	UINT8 tssi_0_slope_g_band;
	UINT8 tssi_1_slope_g_band;
#define TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE 0x0A
#define TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE 0x0B
	UINT8 tssi_0_offset_g_band;
	UINT8 tssi_1_offset_g_band;
#define TX_TARGET_PWR_DEFAULT_VALUE 0x26
	CHAR tx_0_target_pwr_g_band;
	CHAR tx_1_target_pwr_g_band;
	CHAR tx_0_chl_pwr_delta_g_band[3];
	CHAR tx_1_chl_pwr_delta_g_band[3];
	CHAR delta_tx_pwr_bw40_g_band;

	CHAR tx_pwr_cck_1_2;
	CHAR tx_pwr_cck_5_11;
	CHAR tx_pwr_g_band_ofdm_6_9;
	CHAR tx_pwr_g_band_ofdm_12_18;
	CHAR tx_pwr_g_band_ofdm_24_36;
	CHAR tx_pwr_g_band_ofdm_48;
	CHAR tx_pwr_g_band_ofdm_54;
	CHAR tx_pwr_ht_bpsk_mcs_0_8;
	CHAR tx_pwr_ht_bpsk_mcs_32;
	CHAR tx_pwr_ht_qpsk_mcs_1_2_9_10;
	CHAR tx_pwr_ht_16qam_mcs_3_4_11_12;
	CHAR tx_pwr_ht_64qam_mcs_5_13;
	CHAR tx_pwr_ht_64qam_mcs_6_14;
	CHAR tx_pwr_ht_64qam_mcs_7_15;
};

typedef enum _POWER_ACTION_CATEGORY_V0 {
	SKU_FEATURE_CTRL_V0 = 0x0,
	PERCENTAGE_FEATURE_CTRL_V0 = 0x1,
	PERCENTAGE_DROP_CTRL_V0 = 0x2,
	BF_POWER_BACKOFF_FEATURE_CTRL_V0 = 0x3,
	BF_TX_POWER_BACK_OFF_V0 = 0x4,
	RF_TXANT_CTRL_V0 = 0x5,
	ATEMODE_CTRL_V0 = 0x6,
	TX_POWER_SHOW_INFO_V0 = 0x7,
	TPC_FEATURE_CTRL_V0 = 0x8,
	MU_TX_POWER_CTRL_V0 = 0x9,
	BF_NDPA_TXD_CTRL_V0 = 0xa,
	TSSI_WORKAROUND_V0 = 0xb,
	THERMAL_MANUAL_CTRL_V0 = 0xc,
	THERMAL_COMPENSATION_CTRL_V0 = 0xd,
	TX_RATE_POWER_CTRL_V0 = 0xe,
	TXPOWER_UP_TABLE_CTRL_V0 = 0xf,
	TX_POWER_SET_TARGET_POWER_V0 = 0x10,
	TX_POWER_GET_TARGET_POWER_V0 = 0x11,
	ALLTXPOWER_MANUAL_CTRL_V0 = 0x12,
	POWER_ACTION_NUM_V0
} POWER_ACTION_CATEGORY_V0, *P_POWER_ACTION_CATEGORY_V0;

typedef enum _POWER_ACTION_CATEGORY_V1 {
	SKU_POWER_LIMIT_CTRL = 0x0,
	PERCENTAGE_CTRL = 0x1,
	PERCENTAGE_DROP_CTRL_V1 = 0x2,
	BACKOFF_POWER_LIMIT_CTRL = 0x3,
	POWER_LIMIT_TABLE_CTRL = 0x4,
	RF_TXANT_CTRL_V1 = 0x5,
	ATEMODE_CTRL_V1 = 0x6,
	TX_POWER_SHOW_INFO_V1 = 0x7,
	TPC_FEATURE_CTRL_V1 = 0x8,
	MU_TX_POWER_CTRL_V1 = 0x9,
	BF_NDPA_TXD_CTRL_V1 = 0xa,
	TSSI_WORKAROUND_V1 = 0xb,
	THERMAL_COMPENSATION_CTRL_V1 = 0xc,
	TX_RATE_POWER_CTRL_V1 = 0xd,
	TXPOWER_UP_TABLE_CTRL_V1 = 0xe,
	TX_POWER_SET_TARGET_POWER_V1 = 0xf,
	TX_POWER_GET_TARGET_POWER_V1 = 0x10,
#ifdef DATA_TXPWR_CTRL
	TX_POWER_SET_PER_PKT_POWER = 0x11,
	TX_POWER_SET_PER_PKT_MIN_POWER = 0x12,
#endif
	POWER_ACTION_NUM_V1
} POWER_ACTION_CATEGORY_V1, *P_POWER_ACTION_CATEGORY_V1;

typedef enum _THERMAL_PROTECT_ACTION_CATEGORY {
	THERMAL_PROTECT_PARAMETER_CTRL = 0x0,
	THERMAL_PROTECT_BASIC_INFO = 0x1,
	THERMAL_PROTECT_ENABLE = 0x2,
	THERMAL_PROTECT_DISABLE = 0x3,
	THERMAL_PROTECT_DUTY_CONFIG = 0x4,
	THERMAL_PROTECT_MECH_INFO = 0x5,
	THERMAL_PROTECT_DUTY_INFO = 0x6,
	THERMAL_PROTECT_STATE_ACT = 0x7,
	THERMAL_PROTECT_ACTION_NUM
} THERMAL_PROTECT_ACTION_CATEGORY, *P_THERMAL_PROTECT_ACTION_CATEGORY;

typedef enum _THERMAL_ACTION_CATEGORY {
	THERMAL_SENSOR_TEMPERATURE_QUERY = 0x0,
	THERMAL_SENSOR_MANUAL_CTRL = 0x1,
	THERMAL_SENSOR_BASIC_INFO_QUERY = 0x2,
	THERMAL_SENSOR_TASK_MAN_CONTROL = 0x3,
	THERMAL_ACTION_NUM
} THERMAL_ACTION_CATEGORY, *P_THERMAL_ACTION_CATEGORY;

typedef enum _THERMAL_PROTECT_EVENT_CATEGORY {
	THERMAL_PROTECT_EVENT_REASON_NOTIFY = 0x0,
	TXPOWER_EVENT_THERMAL_PROT_SHOW_INFO = 0x1,
	THERMAL_PROTECT_EVENT_DUTY_NOTIFY = 0x2,
	THERMAL_PROTECT_EVENT_RADIO_NOTIFY = 0x3,
	THERMAL_PROTECT_EVENT_MECH_INFO = 0x4,
	THERMAL_PROTECT_EVENT_DUTY_INFO = 0x5,
	THERMAL_PROTECT_EVENT_NUM
} THERMAL_PROTECT_EVENT_CATEGORY, *P_THERMAL_PROTECT_EVENT_CATEGORY;

typedef enum _TSSI_ACTION_CATEGORY {
	EPA_STATUS = 0,
	TSSI_TRACKING_ENABLE = 1,
	FCBW_ENABLE = 2,
	TSSI_COMP_BACKUP = 3,
	TSSI_COMP_CONFIG = 4
} TSSI_ACTION_CATEGORY, *P_TSSI_ACTION_CATEGORY;

typedef enum _POWER_INFO_CATEGORY {
	TXPOWER_BASIC_INFO = 0,
	TXPOWER_BACKUP_TABLE_INFO,
	TXPOWER_ALL_RATE_POWER_INFO,
	TXPOWER_THERMAL_COMP_TABLE_INFO,
	TXPOWER_TXV_BBP_PER_PACKET_INFO,
	POWER_INFO_NUM
} POWER_INFO_CATEGORY, *P_POWER_INFO_CATEGORY;


typedef enum _THERMAL_EVENT_CATEGORY {
	THERMAL_EVENT_TEMPERATURE_INFO = 0x0,
	THERMAL_EVENT_THERMAL_SENSOR_BASIC_INFO = 0x1,
	THERMAL_EVENT_THERMAL_SENSOR_TASK_RESPONSE = 0x2,
	THERMAL_EVENT_NUM
} THERMAL_EVENT_CATEGORY, *P_THERMAL_EVENT_CATEGORY;

typedef enum _POWER_EVENT_CATEGORY {
	TXPOWER_EVENT_SHOW_INFO = 0x0,
	TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE = 0x1,
	TXPOWER_EVENT_UPDATE_EPA_STATUS = 0x2,
	TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO = 0x3,
	TXPOWER_EVENT_TARGET_POWER_INFO_GET = 0x4,
	TXPOWER_EVENT_SHOW_ALL_RATE_TXPOWER_INFO = 0x5,
	TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO = 0x6,
	TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO = 0x7,
	POWER_EVENT_NUM
} POWER_EVENT_CATEGORY, *P_POWER_EVENT_CATEGORY;

typedef enum _BF_BACKOFF_MODE_T {
	BF_BACKOFF_1T_MODE = 1,
	BF_BACKOFF_2T_MODE = 2,
	BF_BACKOFF_3T_MODE = 3,
	BF_BACKOFF_4T_MODE = 4,
} BF_BACKOFF_MODE_T, *P_BF_BACKOFF_MODE_T;


typedef enum _ENUM_TXPOWER_LG_VHT_POWER_BW_OFFSET_T {
    MODULATION_SYSTEM_LG_VHT40_POWER_BW_OFFSET = 0,
    MODULATION_SYSTEM_LG_VHT80_POWER_BW_OFFSET,
    MODULATION_SYSTEM_LG_VHT160_POWER_BW_OFFSET,
    MODULATION_SYSTEM_LG_POWER_BW_OFFSET_NUM,
} ENUM_TXPOWER_LG_VHT_POWER_BW_OFFSET_T, *P_ENUM_TXPOWER_LG_VHT_POWER_BW_OFFSET_T;

typedef enum _ENUM_TXPOWER_VHT_POWER_MAX_BOUND_T {
    MODULATION_SYSTEM_VHT20_POWER_MAX_BOUND = 0,
    MODULATION_SYSTEM_VHT40_POWER_MAX_BOUND,
    MODULATION_SYSTEM_VHT80_POWER_MAX_BOUND,
    MODULATION_SYSTEM_VHT160_POWER_MAX_BOUND,
    MODULATION_SYSTEM_VHT_POWER_MAX_BOUND_NUM,
} ENUM_TXPOWER_VHT_POWER_MAX_BOUND_T, *P_ENUM_TXPOWER_VHT_POWER_MAX_BOUND_T;

typedef enum _ENUM_TXPOWER_POWER_MAX_MIN_BOUND_T {
    MODULATION_SYSTEM_POWER_MAX_BOUND = 0,
    MODULATION_SYSTEM_POWER_MIN_BOUND,
    MODULATION_SYSTEM_POWER_BOUND_NUM,
} ENUM_TXPOWER_POWER_MAX_MIN_BOUND_T, *P_ENUM_TXPOWER_POWER_MAX_MIN_BOUND_T;
/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/

#endif /* __MT_TX_PWR_H__ */
