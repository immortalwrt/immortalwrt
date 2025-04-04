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
	andes_mt.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_MT_H__
#define __ANDES_MT_H__

#include "mcu.h"
#include "mcu/mt_cmd.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

#define GET_EVENT_FW_RXD_LENGTH(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.length)
#define GET_EVENT_FW_RXD_PKT_TYPE_ID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.pkt_type_id)
#define GET_EVENT_FW_RXD_EID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_1.field.eid)
#define GET_EVENT_FW_RXD_OPTION(event_rxd) \
		(((EVENT_RXD *)(event_rxd))->fw_rxd_1.field.option)
#define GET_EVENT_FW_RXD_SEQ_NUM(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_1.field.seq_num)

#define GET_EVENT_FW_RXD_EXT_EID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_2.field.ext_eid)

#define IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(m) \
	((((struct cmd_msg *)(m))->attr.ctrl.expect_size == MT_IGNORE_PAYLOAD_LEN_CHECK) \
	? TRUE : FALSE)
#define GET_EVENT_HDR_ADDR(net_pkt) \
	(GET_OS_PKT_DATAPTR(net_pkt) + sizeof(EVENT_RXD))


#define GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.length - sizeof(EVENT_RXD))


struct _RTMP_ADAPTER;
struct cmd_msg;

#define BFBACKOFF_TABLE_SIZE            10
#define BFBACKOFF_BBPCR_SIZE             6
#define RATE_POWER_TMAC_SIZE             8
#define CR_COLUMN_SIZE                   4

#define NET_DEV_NAME_MAX_LENGTH			16


VOID AndesMTFillCmdHeaderWithTXD(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
VOID AndesMTRxEventHandler(struct _RTMP_ADAPTER *pAd, UCHAR *data);
INT32 AndesMTLoadFw(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTEraseFw(struct _RTMP_ADAPTER *pAd);

VOID AndesMTRxProcessEvent(struct _RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg);

#ifdef TXBF_SUPPORT
VOID ExtEventBfStatusRead(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif

#ifdef LED_CONTROL_SUPPORT
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
INT AndesLedEnhanceOP(
	struct _RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode
);

typedef struct _led_tx_blink_pattern {
	UINT8 led_combine; /* combine band0 and band1 LED actions on the same physical LED */
	UINT8 blink_mode; /*0: all tx frames, 1: Exclude TX beacon and TIM broadcast frames, 2: Only data frames */
	UINT8 rsvd_1;
	UINT8 rsvd_2;
	UINT16 tx_blink_on_time;  /* in ms */
	UINT16 tx_blink_off_time; /* in ms */
	UINT8 rsvd[4];
} led_tx_blink_pattern, *p_led_tx_blink_pattern; /*tx_blink*/

typedef struct _led_pure_blink_pattern {
	UINT8 replay_mode;/* 0: Repeat last:S0->S1->S1->S1->S1->S1 1: Repeat all:S0->S1->S0->S1*/
	UINT8 rsvd_1;
	UINT8 rsvd_2;
	UINT8 rsvd_3;
	UINT32 s0_total_time; /* in ms */
	UINT16 s0_on_time; /* in ms */
	UINT16 s0_off_time; /* in ms */
	UINT32 s1_total_time; /* in ms */
	UINT16 s1_on_time; /* in ms */
	UINT16 s1_off_time; /* in ms */
	UINT8 rsvd[4];
} led_pure_blink_pattern, *p_led_pure_blink_pattern;/*pure_blink*/

typedef struct _led_mix_tx_pure_blink_pattern {
	UINT8 led_combine; /*combine band0 and band1 LED actions on the same physical LED*/
	UINT8 blink_mode; /*0: all tx frames, 1: Exclude TX beacon and TIM broadcast frames, 2: Only data frames */
	UINT8 replay_mode;/* 0: Repeat last:S0->S1->S1->S1->S1->S1 1: Repeat all:S0->S1->S0->S1*/
	UINT8 rsvd_1;
	UINT32 s0_total_time; /* in ms */
	UINT16 s0_on_time; /* in ms */
	UINT16 s0_off_time; /* in ms */
	UINT32 s1_total_time; /* in ms */
	UINT16 s1_on_time; /* in ms */
	UINT16 s1_off_time; /* in ms */
	UINT16 tx_blink_on_time;  /* in ms */
	UINT16 tx_blink_off_time; /* in ms */
	UINT8 rsvd[4];
} led_mix_tx_pure_blink_pattern, *p_led_mix_tx_pure_blink_pattern;/*mix_tx_pure_blink*/


enum LED_IDX {
	LED_IDX_0 = 0,
	LED_IDX_1,
	LED_IDX_2,
	LED_MAX_NUM
};

enum LED_control_type {
	HW_LED = 0,
	FW_LED
};

#define GPIO(_x)		_x

/*led gpio setting for usr*/
typedef struct _LED_INIT_TABLE {
	VOID (*gpio_inti_func)(struct _RTMP_ADAPTER *pAd, UINT8 led_index);
	UINT8 led_idx;
	UINT16 map_idx;
	BOOLEAN control_type; /*0:HW 1:FW*/
} LED_INIT_TABLE, *PLED_INIT_TABLE;

enum LED_CATEGORY {
	LED_CATEGORY_0_SOLID_ON = 0,
	LED_CATEGORY_1_SOLID_OFF,
	LED_CATEGORY_2_TX_BLINK,
	LED_CATEGORY_3_PURE_BLINK,
	LED_CATEGORY_4_MIX_TX_PURE_BLINK,
	LED_CATEGORY_5_GPIO_SETTING
};

typedef struct _led_control_event {
	UINT8 led_ver;
	UINT8 pattern_category;
	UINT8 led_idx;
	UINT8 reverse_polarity;
	UINT8 band_select;/* 0:band 0, 1: band1*/
	UINT8 rsvd_1;
	UINT8 rsvd_2;
	UINT8 rsvd_3;
} led_control_event, *p_led_control_event;
INT AndesLedGpioMap(RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type);
#endif /* defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981) */
#endif

INT32 AndesMTLoadRomPatch(struct _RTMP_ADAPTER *ad);
INT32 AndesMTEraseRomPatch(struct _RTMP_ADAPTER *ad);

#ifdef PHY_ICS_SUPPORT
NTSTATUS PhyIcsRawDataHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
VOID ExtEventPhyIcsUnSolicitDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* PHY_ICS_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
NTSTATUS WifiSpectrumRawDataHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
VOID ExtEventWifiSpectrumUnSolicitRawDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
VOID ExtEventWifiSpectrumUnSolicitIQDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
NTSTATUS ICapRawDataHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
VOID ExtEventICap96BitDataParser(struct _RTMP_ADAPTER  *pAd);
VOID ExtEventICapUnSolicit96BitRawDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
VOID ExtEventICapUnSolicitIQDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
VOID ExtEventICapUnSolicitStatusHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* INTERNAL_CAPTURE_SUPPORT */


VOID EventThermalProtectHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalProtectReasonNotify(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalProtectInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalSensorShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalSensorTaskResp(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerEPAInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalStateShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventPowerTableShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerCompTable(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalCompTableShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID ExtEvenTpcInfoHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTpcDownLinkTbl(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTpcUpLinkTbl(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventRxvHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventRxFeCompHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventRxvReport(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerAllRatePowerShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID event_ecc_result(struct _RTMP_ADAPTER *ad, UINT8 *data, UINT32 length);
NTSTATUS EventTxvBbpPowerInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
NTSTATUS PreCalTxLPFStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalTxIQStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalTxDCStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalRxFIStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalRxFDStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef RED_SUPPORT
VOID ExtEventMpduTimeHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID ExtEventRedTxReportHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif

#ifdef WIFI_MD_COEX_SUPPORT
NTSTATUS ExtEventFw2apccciMsgHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID ExtEventLteSafeChnHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID ExtEventIdcEventHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif

typedef struct _TX_RATE_POWER_TABLE_T {
	UINT8  TxRateModulation;
	UINT8  CRValue;
	CHAR   PowerDecimal;
} TX_RATE_POWER_TABLE_T, *P_TX_RATE_POWER_TABLE_T;

typedef struct _TX_POWER_BOUND_TABLE_T {
	UINT8  MaxMinType;
	UINT8  CRValue;
	CHAR   PowerDecimal;
} TX_POWER_BOUND_TABLE_T, *P_TX_POWER_BOUND_TABLE_T;

typedef enum _TX_POWER_SKU_TABLE {
	CCK1M2M,
	CCK5M11M,
	OFDM6M9M,
	OFDM12M18M,
	OFDM24M36M,
	OFDM48M,
	OFDM54M,
	HT20M0,
	HT20M32,
	HT20M1M2,
	HT20M3M4,
	HT20M5,
	HT20M6,
	HT20M7,
	HT40M0,
	HT40M32,
	HT40M1M2,
	HT40M3M4,
	HT40M5,
	HT40M6,
	HT40M7,
	VHT20M0,
	VHT20M1M2,
	VHT20M3M4,
	VHT20M5M6,
	VHT20M7,
	VHT20M8,
	VHT20M9,
	VHT40M0,
	VHT40M1M2,
	VHT40M3M4,
	VHT40M5M6,
	VHT40M7,
	VHT40M8,
	VHT40M9,
	VHT80M0,
	VHT80M1M2,
	VHT80M3M4,
	VHT80M5M6,
	VHT80M7,
	VHT80M8,
	VHT80M9,
	VHT160M0,
	VHT160M1M2,
	VHT160M3M4,
	VHT160M5M6,
	VHT160M7,
	VHT160M8,
	VHT160M9,
    TXPOWER_1SS_OFFSET,
    TXPOWER_2SS_OFFSET,
    TXPOWER_3SS_OFFSET,
    TXPOWER_4SS_OFFSET
} TX_POWER_SKU_TABLE, *P_TX_POWER_SKU_TABLE;


typedef enum _ENUM_MAX_MIN_TYPE_T {
	MAX_POWER_BAND0,
	MIN_POWER_BAND0,
	MAX_POWER_BAND1,
	MIN_POWER_BAND1
} ENUM_MAX_MIN_TYPE_T, *P_ENUM_MAX_MIN_TYPE_T;

typedef enum _ENUM_TX_RATE_MODULATION_T {
	OFDM_48M,
	OFDM_24M_36M,
	OFDM_12M_18M,
	OFDM_6M_9M,
	HT20_MCS5,
	HT20_MCS3_4,
	HT20_MCS1_2,
	HT20_MCS0,
	HT40_MCS5,
	HT40_MCS3_4,
	HT40_MCS1_2,
	HT40_MCS0,
	HT40_MCS32,
	CCK_5M11M,
	OFDM_54M,
	CCK_1M2M,
	HT40_MCS7,
	HT40_MCS6,
	HT20_MCS7,
	HT20_MCS6,
	VHT20_MCS5_6,
	VHT20_MCS3_4,
	VHT20_MCS1_2,
	VHT20_MCS0,
	VHT20_MCS9,
	VHT20_MCS8,
	VHT20_MCS7,
	VHT160,
	VHT80,
	VHT40
} ENUM_TX_RATE_MODULATION_T, *P_ENUM_TX_RATE_MODULATION_T;

#ifdef CFG_SUPPORT_CSI
#define CSI_INFO_RSVD1 BIT(0)
#define CSI_INFO_RSVD2 BIT(1)

/*for filter mode type*/
enum CSI_FILTER_MODE_T {
	CSI_LENGTH_FILTER,
	CSI_MAC_FILTER,
	CSI_TS_FILTER,
	CSI_FILTER_MODE_NUM
};

/*for mac filter*/
enum CSI_STA_MAC_MODE_T {
	CSI_STA_MAC_DEL,
	CSI_STA_MAC_ADD,
	CSI_STA_MAC_SHOW,
	CSI_STA_MAC_MODE_NUM
};

enum CSI_CONTROL_MODE_T {
	CSI_CONTROL_MODE_STOP,
	CSI_CONTROL_MODE_START,
	CSI_CONTROL_MODE_SET,
	CSI_CONTROL_MODE_NUM
};

enum CSI_CONFIG_ITEM_T {
	CSI_CONFIG_RSVD1,
	CSI_CONFIG_WF,
	CSI_CONFIG_RSVD2,
	CSI_CONFIG_FRAME_TYPE,
	CSI_CONFIG_TX_PATH,
	CSI_CONFIG_OUTPUT_FORMAT,
	CSI_CONFIG_INFO,
	CSI_CONFIG_CHAIN_NUMBER,
	CSI_CONFIG_FILTER_MODE,
	CSI_CONFIG_ITEM_NUM
};

struct CMD_CSI_CONTROL_T {
	UINT_8 BandIdx;
	UINT_8 ucMode;
	UINT_8 ucCfgItem;
	UINT_8 ucValue1;
	UINT_32 ucValue2;
	UINT_8 mac_addr[MAC_ADDR_LEN];
	UINT_8 aucResrved0[34];
};

enum CSI_OUTPUT_FORMAT_T {
	CSI_OUTPUT_RAW,
	CSI_OUTPUT_TONE_MASKED,
	CSI_OUTPUT_TONE_MASKED_SHIFTED,
	CSI_OUTPUT_FORMAT_NUM
};

enum CSI_EVENT_TLV_TAG {
	CSI_EVENT_FW_VER,
	CSI_EVENT_CBW,
	CSI_EVENT_RSSI,
	CSI_EVENT_SNR,
	CSI_EVENT_BAND,
	CSI_EVENT_CSI_NUM,
	CSI_EVENT_CSI_I_DATA,
	CSI_EVENT_CSI_Q_DATA,
	CSI_EVENT_DBW,
	CSI_EVENT_CH_IDX,
	CSI_EVENT_TA,
	CSI_EVENT_EXTRA_INFO,
	CSI_EVENT_RX_MODE,
	CSI_EVENT_RSVD1,
	CSI_EVENT_RSVD2,
	CSI_EVENT_RSVD3,
	CSI_EVENT_RSVD4,
	CSI_EVENT_H_IDX,
	CSI_EVENT_TX_RX_IDX,
	CSI_EVENT_TS,
	CSI_EVENT_TLV_TAG_NUM,
};

ULONG NBytesAlign(ULONG len, ULONG nBytesAlign);
INT AndesCSICtrl(RTMP_ADAPTER *pAd, struct CMD_CSI_CONTROL_T *prCSICtrl);
VOID ExtEventCSICtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID event_twt_resume_info(struct _RTMP_ADAPTER *pAd, UINT8 *data, UINT32 length);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#endif /* __ANDES_MT_H__ */

