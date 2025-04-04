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
/***************************************************************************
 ***************************************************************************

*/

#ifndef __MT_FMAC_H__
#define __MT_FMAC_H__

#include "mac/mac_mt/fmac/fmac_txd.h"
#include "mac/mac_mt/fmac/fmac_rxd.h"
#include "txpwr/txpwr.h"

enum _ENUM_AHDBG_L1_INDEX_T {
	ENUM_AHDBUG_L1_RX = 0,
	ENUM_AHDBUG_L1_TX = 1,
	ENUM_AHDBUG_L1_BF = 2,
	ENUM_AHDBUG_L1_TXCMD = 3,
	ENUM_AHDBUG_L1_UMAC = 4,
	ENUM_AHDBUG_L1_WFDMA = 5,
	ENUM_AHDBUG_L1_LP = 6,
	ENUM_AHDBUG_L1_SER = 7,
	ENUM_AHDBUG_L1_NONE,
	ENUM_AHDBUG_L1_INVALID = 99,
};

struct GNU_PACKED txd_l {
	UINT32 txd_0;
	UINT32 txd_1;
	UINT32 txd_2;
	UINT32 txd_3;
	UINT32 txd_4;
	UINT32 txd_5;
	UINT32 txd_6;
	UINT32 txd_7;
};

#define TXD_MSDU_ID_MASK	0x7fff
#define TXD_MSDU_ID_VLD		BIT(15)		/* MSDU valid */
#define TXD_LEN_ML_V2		BIT(15)		/* MSDU last */
#define TXD_LEN_MASK_V2		BITS(0, 11)

#define MAX_TXP_LEN		4095		/* 0xFFF means 12 bits */
#define MAX_BUF_NUM_PER_PKT	6
#define CUT_THROUGH_PAYLOAD_LEN	72

struct GNU_PACKED txs_header {
	UINT32 txs_h_0;
	UINT32 txs_h_1;
};

struct GNU_PACKED txs_frame {
	UINT32 txs_f_0;
	UINT32 txs_f_1;
	UINT32 txs_f_2;
	UINT32 txs_f_3;
	UINT32 txs_f_4;
	UINT32 txs_f_5;
	UINT32 txs_f_6;
	UINT32 txs_f_7;
};

struct GNU_PACKED rxd_grp_0 {
	UINT32 rxd_0;
	UINT32 rxd_1;
	UINT32 rxd_2;
	UINT32 rxd_3;
	UINT32 rxd_4;
	UINT32 rxd_5;
};

struct GNU_PACKED rxd_grp_1 {
	UINT32 rxd_10;
	UINT32 rxd_11;
	UINT32 rxd_12;
	UINT32 rxd_13;
};

struct GNU_PACKED rxd_grp_2 {
	UINT32 rxd_14;
	UINT32 rxd_15;
};

struct GNU_PACKED rxd_grp_3 {
	UINT32 rxd_16;
	UINT32 rxd_17;
};

struct GNU_PACKED rxd_grp_4 {
	UINT32 rxd_6;
	UINT32 rxd_7;
	UINT32 rxd_8;
	UINT32 rxd_9;
};

struct GNU_PACKED rxd_grp_5 {
	UINT32 rxd_18;
	UINT32 rxd_19;
	UINT32 rxd_20;
	UINT32 rxd_21;
	UINT32 rxd_22;
	UINT32 rxd_23;
	UINT32 rxd_24;
	UINT32 rxd_25;
	UINT32 rxd_26;
	UINT32 rxd_27;
	UINT32 rxd_28;
	UINT32 rxd_29;
	UINT32 rxd_30;
	UINT32 rxd_31;
	UINT32 rxd_32;
	UINT32 rxd_33;
	UINT32 rxd_34;
	UINT32 rxd_35;
};

/* TXDONE DW0 */
#define TXDONE_RX_BYTE_CNT_MASK 0xffff
#define TXDONE_RX_BYTE_CNT_SHIFT 0
#define TXDONE_MSDU_ID_CNT_MASK (0x7f << 16)
#define TXDONE_E2_MSDU_ID_CNT_MASK (0x3ff << 16)
#define TXDONE_MSDU_ID_CNT_SHIFT 16
#define TXDONE_PKT_TYPE_MASK (0x1f << 27)
#define TXDONE_PKT_TYPE_SHIFT 27

/* TXDONE DW1 */
enum {
	MT7615_TXDONE,
	MT7622_TXDONE,
	MT7915_E1_TXDONE,
	MT7915_E2_TXDONE,
	MT7986_TXDONE,
	MT7916_TXDONE = MT7986_TXDONE,
	MT7981_TXDONE = MT7986_TXDONE,
};

#define TXDONE_TXD_CNT_MASK 0xff
#define TXDONE_TXD_CNT_SHIFT 0
#define TXDONE_VER_MASK (0x7 << 16)
#define TXDONE_VER_SHIFT 16

#define TXDONE_PKT_TYPE_SHIFT_V0 29
#define TXDONE_PKT_TYPE_MASK_VALUE_V0 0x7
#define TXDONE_PKT_TYPE_MASK_V0 (TXDONE_PKT_TYPE_MASK_VALUE_V0 << TXDONE_PKT_TYPE_SHIFT_V0)

/*  TXDONE E1 DW2~DWN */
#define TXDONE_MSDU_ID_MASK 0x7fff
#define TXDONE_MSDU_ID_SHIFT 0
#define TXDONE_WLAN_ID_MASK (0x3ff << 15)
#define TXDONE_WLAN_ID_SHIFT 15
#define TXDONE_QID_MASK (0x7f << 25)
#define TXDONE_QID_SHIFT 25

/* TXDONE E2 DW2~DWN */
#define TXDONE_E2_WLAN_ID_MASK (0x3ff << 14)
#define TXDONE_E2_WLAN_ID_SHIFT 14
#define TXDONE_E2_QID_MASK (0x7f << 24)
#define TXDONE_E2_QID_SHIFT 24
#define TXDONE_P (1 << 31)

#define TXDONE_TX_LATENCY_CNT_MASK 0x1fff
#define TXDONE_TX_LATENCY_CNT_SHIFT 0
#define TXDONE_STAT_MASK (0x3 << 13)
#define TXDONE_STAT_SHIFT  13
#define TXDONE_H (1 << 15)
#define TXDONE_E2_MSDU_ID_MASK (0x7fff << 16)
#define TXDONE_E2_MSDU_ID_SHIFT 16

/* MT7986_TXDONE DW2~DWN */
#define TXDONE_MT7986_WLAN_ID_MASK (0x3ff << 14)
#define TXDONE_MT7986_WLAN_ID_SHIFT 14
#define TXDONE_MT7986_QID_MASK (0x7f << 24)
#define TXDONE_MT7986_QID_SHIFT 24
#define TXDONE_MT7986_P (1 << 31)
#define TXDONE_MT7986_TX_DELAY_MASK 0xfff
#define TXDONE_MT7986_TX_DELAY_SHIFT 0
#define TXDONE_MT7986_AIR_DELAY_MASK (0xfff << 12)
#define TXDONE_MT7986_AIR_DELAY_SHIFT 12
#define TXDONE_MT7986_TX_CNT_MASK (0xf << 24)
#define TXDONE_MT7986_TX_CNT_SHIFT 24
#define TXDONE_MT7986_STAT_MASK (0x3 << 28)
#define TXDONE_MT7986_STAT_SHIFT  28
#define TXDONE_MT7986_H (1 << 30)
#define TXDONE_MT7986_MSDU_ID_MASK (0x7fff)
#define TXDONE_MT7986_MSDU_ID_SHIFT 15
#define TXDONE_MT7986_P (1 << 31)


struct GNU_PACKED txdone_event {
	UINT32 txdone_0;
	UINT32 txdone_1;
	UINT32 txdone_2;
};

#define MT7615_MT_WTBL_SIZE	128
#define MT7622_MT_WTBL_SIZE	128
#define MT_DMAC_BA_AGG_RANGE		8
#define MT_DMAC_BA_STAT_RANGE	8

#define MT_PSE_BASE_ADDR		0xa0000000
#define MT_PSE_PAGE_SIZE		128

#define MT_CBTOP_HW_BND		0x70010020
#define MT_CBTOP_HW_VER		0x70010204
#define MT_CBTOP_FW_VER		0x70010208
#define MT_CBTOP_HW_CODE	0x70010200

#define CBTOP_RESV_CID		0x70010210

#define MTF_BND_OPT_1	BIT5

#define MT_WIFI_MCUSYS_HW_VER	0x88000000
#define MT_WIFI_MCUSYS_FW_VER	0x88000004

#ifdef MAC_REPEATER_SUPPORT
#define MAX_EXT_MAC_ADDR_SIZE	32
#endif /* MAC_REPEATER_SUPPORT */

#define MCU_CFG_BASE		0x80000000
#define MCU_COM_REG1	    (MCU_CFG_BASE + 0x204)
#ifdef ERR_RECOVERY
#define MCU_COM_REG1_SER_PSE		BIT(0)
#define MCU_COM_REG1_SER_PLE		BIT(1)
#define MCU_COM_REG1_SER_PCIE		BIT(2)
#define MCU_COM_REG1_SER_PDMA		BIT(3)
#define MCU_COM_REG1_SER_LMAC_TX	BIT(4)
#define MCU_COM_REG1_SER_SEC_RF_RX	BIT(5)
#endif  /* ERR_RECOVERY */

/* MT7986_TXDONE DW3 Bit[29:28] Tx Stat*/
#define FLG_SUCCESS		0
#define FLG_HW_DROP		1
#define FLG_MCU_DROP	2

/* leo: defined in top.h #define TOP_CFG_BASE        0x0000 */

#define XTAL_CTL4           (TOP_CFG_BASE + 0x1210)
#define XTAL_CTL13          (TOP_CFG_BASE + 0x1234)
#define XTAL_CTL14          (TOP_CFG_BASE + 0x1238)
#define DA_XO_C2_MASK (0x7f << 8)
#define DA_XO_C2(p) (((p) & 0x7f) << 8)


#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE		1


#define TSO_SIZE		0

#define EXP_ACK_TIME	0x1380

#define E2PROM_CSR          0x0004
#define GPIO_CTRL_CFG	0x0228
#define WSC_HDR_BTN_GPIO_0			((UINT32)0x00000001) /* bit 0 for RT2860/RT2870 */
#define WSC_HDR_BTN_GPIO_3			((UINT32)0x00000008) /* bit 3 for RT2860/RT2870 */

#undef RMAC_RXD_0_PKT_TYPE_MASK
#undef RMAC_RX_PKT_TYPE
#define RMAC_RXD_0_PKT_TYPE_MASK (0x1f << 27)
#define RMAC_RX_PKT_TYPE(_x) (((_x) & RMAC_RXD_0_PKT_TYPE_MASK) >> 27)

#define RMAC_RX_BYTE_CNT(_x) (((_x) & RXD_RX_BYTE_COUNT_MASK) >> RXD_RX_BYTE_COUNT_SHIFT)

struct _RTMP_ADAPTER;

VOID mtf_update_mib_bucket(struct _RTMP_ADAPTER *pAd);

#ifdef OFFCHANNEL_ZERO_LOSS
VOID mtf_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat);
#endif

typedef struct _HAL_FRAME_POWER_SET_T {
    INT8 i1FramePowerDbm;
} HAL_FRAME_POWER_SET_T, *P_HAL_FRAME_POWER_SET_T;

typedef struct _FRAME_POWER_CONFIG_INFO_T {
    HAL_FRAME_POWER_SET_T ai1FramePowerConfig[TXPOWER_RATE_NUM][2];
} FRAME_POWER_CONFIG_INFO_T, *P_FRAME_POWER_CONFIG_INFO_T;

typedef struct _EXT_EVENT_TXPOWER_ALL_RATE_POWER_INFO_T {
	UINT8   u1TxPowerCategory;
	UINT8   u1BandIdx;
	UINT8   u1ChBand;
	UINT8   u1EpaFeGain;

	/* Rate power info */
	FRAME_POWER_CONFIG_INFO_T rRatePowerInfo;

	/* tx Power Max/Min Limit info */
	CHAR	i1PwrMaxBnd;
	CHAR	i1PwrMinBnd;
	UINT8   au1Reserved4;
} EXT_EVENT_TXPOWER_ALL_RATE_POWER_INFO_T, *P_EXT_EVENT_TXPOWER_ALL_RATE_POWER_INFO_T;

struct _kfree_def {
	UINT8 count;
	UINT16 offsets[60];
};

VOID mtf_dump_wtbl_info(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
VOID mtf_dump_wtbl_base_info(struct _RTMP_ADAPTER *pAd);
UINT16 mtf_tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble);
UCHAR mtf_get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss);

#define HIF_PORT 1
#define MCU_PORT 2
#define MT_TX_RETRY_UNLIMIT		0x1f
#define MT_TX_SHORT_RETRY		0x07
#define MT_TX_LONG_RETRY		0x0f

INT mt_nic_asic_init(struct _RTMP_ADAPTER *pAd);

struct _TX_BLK;
struct _RX_BLK;
struct _MAC_TX_INFO;
union _HTTRANSMIT_SETTING;

INT32 mtf_write_txp_info_by_host(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT32 mtf_write_txp_info_by_wa(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtf_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtf_write_tmac_info_by_host(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtf_write_tmac_info_by_wa(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtf_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);
VOID mtf_dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
BOOLEAN in_altx_filter_list(HEADER_802_11 *pHeader);
VOID mtf_write_tmac_info_fixed_rate(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, struct _MAC_TX_INFO *info,
					union _HTTRANSMIT_SETTING *pTransmit);
UINT32 mtf_get_packet_type(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);

VOID mtf_dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info);
UINT32 mtf_get_packet_rxbytes(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
BOOLEAN check_rx_pkt_type(UINT32  rx_pkt_type);

#ifdef SNIFFER_RADIOTAP_SUPPORT
UINT32 mtf_trans_rxd_into_radiotap(RTMP_ADAPTER *pAd, VOID *rx_packet, struct _RX_BLK *rx_blk);
#endif

INT32 mtf_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket);
UINT32 mtf_txdone_handle(struct _RTMP_ADAPTER *pAd, VOID *ptr, UINT8 resource_idx);
INT32 mtf_txs_handler(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
VOID mtf_rx_event_handler(struct _RTMP_ADAPTER *pAd, UCHAR *data);
#ifdef WIFI_UNIFIED_COMMAND
VOID mtf_fill_uni_cmd_header(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *pkt);
#endif /* WIFI_UNIFIED_COMMAND */
VOID mtf_fill_cmd_header(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *pkt);
UINT32 mtf_rxv_handler(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_packet);

extern const UCHAR altx_filter_list[];

#ifdef WFDMA_WED_COMPATIBLE
VOID mtf_wa_cpu_update(struct _RTMP_ADAPTER *ad);
#endif
VOID mtf_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y);
VOID mtf_dump_rxinfo(struct _RTMP_ADAPTER *pAd, UCHAR *pRxInfo);
INT mtf_dump_dmac_amsdu_info(struct _RTMP_ADAPTER *pAd);
VOID mtf_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data);
VOID mtf_dump_rmac_info_for_ICVERR(struct _RTMP_ADAPTER *pAd, UCHAR *rxinfo);
UINT32 mtf_get_hwq_from_ac(UINT8 wmm_idx, UINT8 wmm_ac);
VOID mtf_show_mac_info(struct _RTMP_ADAPTER *pAd);
INT mtf_init_wtbl(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);
INT mtf_get_wtbl_entry234(struct _RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent);
VOID mtf_update_raw_counters(struct _RTMP_ADAPTER *pAd);

VOID mtf_txpower_show_info(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID mtf_txpower_all_rate_info(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#ifdef MGMT_TXPWR_CTRL
INT wtbl_update_pwr_offset(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif

#ifdef TX_POWER_CONTROL_SUPPORT
VOID mtf_txpower_boost(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx);
VOID mtf_txpower_boost_ctrl(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx, CHAR cPwrUpCat, PUCHAR pcPwrUpValue);
BOOLEAN mtf_txpower_boost_info(struct _RTMP_ADAPTER *pAd, POWER_BOOST_TABLE_CATEGORY_V1 ePowerBoostRateType);
BOOLEAN mtf_txpower_boost_power_cat_type(struct _RTMP_ADAPTER *pAd, UINT8 u1PhyMode, UINT8 u1Bw, PUINT8 pu1PowerBoostRateType);
BOOLEAN mtf_txpower_boost_rate_type(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 u1PowerBoostRateType);
VOID mtf_txpower_boost_profile(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer);
#endif
#ifdef SINGLE_SKU_V2
VOID mtf_txpower_sku_cfg_para(struct _RTMP_ADAPTER *pAd);
#endif
#endif

