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

*/


#ifndef _DOT11AX_HE_H_
#define _DOT11AX_HE_H_

#include "dot11_base.h"

/* HE EID EXT */
#define EID_EXT_HE_CAPS 35
#define EID_EXT_HE_OP 36
#define EID_EXT_UORA_PARAM_SET 37
#define EID_EXT_MU_EDCA_PARAM 38
#define EID_EXT_SR_PARAM_SET 39
#define EID_EXT_NDP_FB_REPORT 41
#define EID_EXT_BSS_COLOR_CHANGE_ANNOUNCE 42
#define EID_EXT_QUIET_TIME_PERIOD 43
#define EID_EXT_ESS_REPORT 45
#define EID_EXT_OPS 46
#define EID_EXT_HE_BSS_LOAD 47
#define EID_EXT_MULTI_BSSID_CFG 55
#define EID_EXT_KNOWN_BSSID 57
#define EID_EXT_SHORT_SSID_LIST 58
#define EID_EXT_HE_6G_CAPS 59
#define EID_EXT_UL_MU_PWR_CAPS 60

/*
 * HE Capabilities element
 */
/* HE MAC Capabilities Information field */
/* Fragmentation Support subfield */
enum {
	DYN_FRAG_NOT_SUPP,
	DYN_FRAG_LVL_1,
	DYN_FRAG_LVL_2,
	DYN_FRAG_LVL_3
};

/* Minimum Fragment Size subfield */
enum {
	MIN_FRAG_SZ_NO_RESTRICT,
	MIN_FRAG_SZ_128,
	MIN_FRAG_SZ_256,
	MIN_FRAG_SZ_512
};

/* Trigger Frame MAC Padding Duration subfield: non-AP STA only */
enum {
	NO_PADDING,
	PADDING_8US,
	PADDING_16US
};

/* HE Link Adaptation Capable subfield */
enum {
	NOT_SUPPORT = 0,
	NO_FEEDBACK = NOT_SUPPORT,
	UNSOLICITED,
	BOTH
};

/* mac_capinfo_1: bit0..31 */
#define DOT11AX_MAC_CAP_HTC 1
#define DOT11AX_MAC_CAP_TWT_REQ (1 << 1)
#define DOT11AX_MAC_CAP_TWT_RSP (1 << 2)
#define DOT11AX_MAC_CAP_FRAG_LVL_SHIFT 3
#define DOT11AX_MAC_CAP_FRAG_LVL_MASK (3 << 3)
#define DOT11AX_MAC_CAP_MAX_FRAG_MSDU_AMSDU_EXP_SHIFT 5
#define DOT11AX_MAC_CAP_MAX_FRAG_MSDU_AMSDU_EXP_MASK (7 << 5)
#define DOT11AX_MAC_CAP_MIN_FRAG_SIZE_SHIFT 8
#define DOT11AX_MAC_CAP_MIN_FRAG_SIZE_MASK (3 << 8)
#define DOT11AX_MAC_CAP_TF_MAC_PAD_SHIFT 10
#define DOT11AX_MAC_CAP_TF_MAC_PAD_MASK (3 << 10)
#define DOT11AX_MAC_CAP_MULTI_TID_AGG_RX_SHIFT 12
#define DOT11AX_MAC_CAP_MULTI_TID_AGG_RX_MASK (7 << 12)
#define DOT11AX_MAC_CAP_LINK_ADAPT_SHIFT 15
#define DOT11AX_MAC_CAP_LINK_ADAPT_MASK (3 << 15)
#define DOT11AX_MAC_CAP_ALL_ACK (1 << 17)
#define DOT11AX_MAC_CAP_UMRS (1 << 18)
#define DOT11AX_MAC_CAP_BSR (1 << 19)
#define DOT11AX_MAC_CAP_BROADCAST_TWT (1 << 20)
#define DOT11AX_MAC_CAP_32BA_BITMAP (1 << 21)
#define DOT11AX_MAC_CAP_MU_CASCADE (1 << 22)
#define DOT11AX_MAC_CAP_ACK_EN_AGG (1 << 23)
#define DOT11AX_MAC_CAP_RSV (1 << 24)
#define DOT11AX_MAC_CAP_OM_CTRL (1 << 25)
#define DOT11AX_MAC_CAP_OFDMA_RA (1 << 26)
#define DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT 27
#define DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_MASK (3 << 27)
#define DOT11AX_MAC_CAP_AMSDU_FRAG (1 << 29)
#define DOT11AX_MAC_CAP_FLEX_TWT_SCHDL (1 << 30)
#define DOT11AX_MAC_CAP_RX_CTRL_FRAME_2_MULTI_BSS (1 << 31)
/* mac_capinfo_2: bit32..47 */
#define DOT11AX_MAC_CAP_BSRP_BQRP_AMPDU_AGG 1
#define DOT11AX_MAC_CAP_QTP (1 << 1)
#define DOT11AX_MAC_CAP_BQR (1 << 2)
#define DOT11AX_MAC_CAP_SRP_RSP (1 << 3)
#define DOT11AX_MAC_CAP_NDP_FEEDBACK_REPORT (1 << 4)
#define DOT11AX_MAC_CAP_OPS (1 << 5)
#define DOT11AX_MAC_CAP_AMSDU_IN_ACK_EN_AMPDU (1 << 6)
#define DOT11AX_MAC_CAP_MULTI_TID_AGG_TX_SHIFT 7
#define DOT11AX_MAC_CAP_MULTI_TID_AGG_TX_MASK (7 << 7)
#define DOT11AX_MAC_CAP_HE_SUB_CHANNEL_SEL_TRANS (1 << 10)
#define DOT11AX_MAC_CAP_UL_2x996_RU (1 << 11)
#define DOT11AX_MAC_CAP_OM_CTRL_UL_MU_DIS_RX (1 << 12)
#define DOT11AX_MAC_CAP_HE_DYN_SMPS (1 << 13)
#define DOT11AX_MAC_CAP_PUNC_SND (1 << 14)
#define DOT11AX_MAC_HT_AND_VHT_TRIG_FRAME_RX (1 << 15)

/* he mac caps get */
#define GET_DOT11AX_FRAG_LVL(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_FRAG_LVL_MASK) >> DOT11AX_MAC_CAP_FRAG_LVL_SHIFT)
#define GET_DOT11AX_MAX_FRAG_MSDU_AMSDU_EXP(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_MAX_FRAG_MSDU_AMSDU_EXP_MASK) >> DOT11AX_MAC_CAP_MAX_FRAG_MSDU_AMSDU_EXP_SHIFT)
#define GET_DOT11AX_MIN_FRAG_SIZE(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_MIN_FRAG_SIZE_MASK) >> DOT11AX_MAC_CAP_MIN_FRAG_SIZE_SHIFT)
#define GET_DOT11AX_TF_MAC_PAD(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_TF_MAC_PAD_MASK) >> DOT11AX_MAC_CAP_TF_MAC_PAD_SHIFT)
#define GET_DOT11AX_MULTI_TID_AGG_TX(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_MULTI_TID_AGG_TX_MASK) >> DOT11AX_MAC_CAP_MULTI_TID_AGG_TX_SHIFT)
#define GET_DOT11AX_MULTI_TID_AGG_RX(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_MULTI_TID_AGG_RX_MASK) >> DOT11AX_MAC_CAP_MULTI_TID_AGG_RX_SHIFT)
#define GET_DOT11AX_LINK_ADPT(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_LINK_ADAPT_MASK) >> DOT11AX_MAC_CAP_LINK_ADAPT_SHIFT)
#define GET_DOT11AX_MAX_AMPDU_LEN_EXP(mac_cap)\
	(((mac_cap) & DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_MASK) >> DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT)

struct GNU_PACKED he_mac_capinfo {
	UINT32 mac_capinfo_1;
	UINT16 mac_capinfo_2;
};

/* HE PHY Capabilities Information field */
/* Channel Width Set subfield */
enum he_channel_width_set {
	SUPP_40M_CW_IN_24G_BAND = 1,
	SUPP_40M_80M_CW_IN_5G_BAND = (1 << 1),
	SUPP_160M_CW_IN_5G_BAND = (1 << 2),
	SUPP_160M_8080M_CW_IN_5G_BAND = (1 << 3),
	SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND = (1 << 4),
	SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND = (1 << 5)
};

/* Punctured Preamble Rx subfield */
enum {
	RX_80M_PREAMBLE_SEC_20M_PUNC = 1,
	RX_80M_PREAMBLE_SEC_40M_PUNC = (1 << 1),
	RX_PRIM_80M_PREAMBLE_SEC_20M_PUNC = (1 << 2),
	RX_PRIM_80M_PREAMBLE_PRIM_40M_PUNC = (1 << 3)
};

/* DCM Max Constellation Tx subfield */
enum {
	DCM_NOT_SUPPORT,
	DCM_BPSK,
	DCM_QPSK,
	DCM_16QAM
};

/* DCM Max RU */
enum {
	DCM_MAX_RU242,
	DCM_MAX_RU484,
	DCM_MAX_RU996,
	DCM_MAX_RU2x996
};

/* NOMIAL PAD */
enum {
	NOMINAL_PAD_0US,
	NOMINAL_PAD_8US,
	NOMINAL_PAD_16US,
	NOMINAL_PAD_RSV
};

/* phy_capinfo_1: bit0..31 */
#define DOT11AX_PHY_CAP_CH_WIDTH_SET_SHIFT 1
#define DOT11AX_PHY_CAP_CH_WIDTH_SET_MASK (0x7F << 1)
#define DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_SHIFT 8
#define DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_MASK (0xF << 8)
#define DOT11AX_PHY_CAP_DEVICE_CLASS_A (1 << 12)
#define DOT11AX_PHY_CAP_LDPC (1 << 13)
#define DOT11AX_PHY_CAP_SU_PPDU_1x_HE_LTF_DOT8US_GI (1 << 14)
#define DOT11AX_PHY_CAP_MIDAMBLE_RX_MAX_NSTS_SHIFT 15
#define DOT11AX_PHY_CAP_MIDAMBLE_RX_MAX_NSTS_MASK (0x3 << 15)
#define DOT11AX_PHY_CAP_NDP_4x_HE_LTF_3DOT2MS_GI (1 << 17)
#define DOT11AX_PHY_CAP_TX_STBC_LE_EQ_80M (1 << 18)
#define DOT11AX_PHY_CAP_RX_STBC_LE_EQ_80M (1 << 19)
#define DOT11AX_PHY_CAP_TX_DOPPLER (1 << 20)
#define DOT11AX_PHY_CAP_RX_DOPPLER (1 << 21)
#define DOT11AX_PHY_CAP_FULL_BW_UL_MU_MIMO (1 << 22)
#define DOT11AX_PHY_CAP_PARTIAL_BW_UL_MU_MIMO (1 << 23)
#define DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_SHIFT 24
#define DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_MASK (0x3 << 24)
#define DOT11AX_PHY_CAP_TX_DCM_MAX_NSS (1 << 26)
#define DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_SHIFT 27
#define DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_MASK (0x3 << 27)
#define DOT11AX_PHY_CAP_RX_DCM_MAX_NSS (1 << 29)
#define DOT11AX_PHY_CAP_UL_HE_MU_PPDU (1 << 30)
#define DOT11AX_PHY_CAP_SU_BFER (1 << 31)

/* phy_capinfo_2: bit32..bit63 */
#define DOT11AX_PHY_CAP_SU_BFEE 1
#define DOT11AX_PHY_CAP_MU_BFER (1 << 1)
#define DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_SHIFT 2
#define DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_MASK (0x7 << 2)
#define DOT11AX_PHY_CAP_BFEE_STS_GT_80M_SHIFT 5
#define DOT11AX_PHY_CAP_BFEE_STS_GT_80M_MASK (0x7 << 5)
#define DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_SHIFT 8
#define DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_MASK (0x7 << 8)
#define DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_SHIFT 11
#define DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_MASK (0x7 << 11)
#define DOT11AX_PHY_CAP_NG16_SU_FEEDBACK (1 << 14)
#define DOT11AX_PHY_CAP_NG16_MU_FEEDBACK (1 << 15)
#define DOT11AX_PHY_CAP_CODEBOOK_SU_FEEDBACK (1 << 16)
#define DOT11AX_PHY_CAP_CODEBOOK_MU_FEEDBACK (1 << 17)
#define DOT11AX_PHY_CAP_TRIG_SU_BF_FEEDBACK (1 << 18)
#define DOT11AX_PHY_CAP_TRIG_MU_BF_PARTIAL_BW_FEEDBACK (1 << 19)
#define DOT11AX_PHY_CAP_TRIG_CQI_FEEDBACK (1 << 20)
#define DOT11AX_PHY_CAP_PARTIAL_BW_ER (1 << 21)
#define DOT11AX_PHY_CAP_PARTIAL_BW_DL_MU_MIMO (1 << 22)
#define DOT11AX_PHY_CAP_PPE_THRLD_PRESENT (1 << 23)
#define DOT11AX_PHY_CAP_SRP_BASE_SR (1 << 24)
#define DOT11AX_PHY_CAP_PWR_BOOST_FACTOR (1 << 25)
#define DOT11AX_PHY_CAP_SU_MU_PPDU_4x_HE_LTF_DOT8_US (1 << 26)
#define DOT11AX_PHY_CAP_MAX_NC_SHIFT 27
#define DOT11AX_PHY_CAP_MAX_NC_MASK (0x7 << 27)
#define DOT11AX_PHY_CAP_TX_STBC_GT_80M (1 << 30)
#define DOT11AX_PHY_CAP_RX_STBC_GT_80M (1 << 31)

/* phy_capinfo_3: bit64..bit71 */
#define DOT11AX_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US 1
#define DOT11AX_PHY_CAP_20M_IN_40M_HE_PPDU_24G (1 << 1)
#define DOT11AX_PHY_CAP_20M_IN_160M_8080M_HE_PPDU (1 << 2)
#define DOT11AX_PHY_CAP_80M_IN_160M_8080M_HE_PPDU (1 << 3)
#define DOT11AX_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US (1 << 4)
#define DOT11AX_PHY_CAP_MIDAMBLE_TXRX_2X_1X_HE_LTF (1 << 5)
#define DOT11AX_PHY_CAP_DCM_MAX_RU_SHIFT 6
#define DOT11AX_PHY_CAP_DCM_MAX_RU_MASK (0x3 << 6)
/* phy_capinfo_4: bit72..bit79 */
#define DOT11AX_PHY_CAP_LONGER_16_HE_SIGB_OFDM_SYM 1
#define DOT11AX_PHY_CAP_NON_TRIG_CQI_FEEDBACK (1 << 1)
#define DOT11AX_PHY_CAP_TX_1024QAM_LT_242_TONE_RU (1 << 2)
#define DOT11AX_PHY_CAP_RX_1024QAM_LT_242_TONE_RU (1 << 3)
#define DOT11AX_PHY_CAP_RX_FULL_BW_HE_MU_PPDU_W_COMPRESS_SIGB (1 << 4)
#define DOT11AX_PHY_CAP_RX_FULL_BW_HE_MU_PPDU_W_NON_COMPRESS_SIGB (1 << 5)
#define DOT11AX_PHY_CAP_NOMINAL_PKT_PAD_SHIFT 6
#define DOT11AX_PHY_CAP_NOMINAL_PKT_PAD_MASK (0x3 << 6)
/* phy_capinfo_5: bit80..87 */
/*Reserved*/

/* he phy caps get */
#define GET_DOT11AX_CH_WIDTH(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_CH_WIDTH_SET_MASK) >> DOT11AX_PHY_CAP_CH_WIDTH_SET_SHIFT)
#define GET_DOT11AX_PUNC_PREAMBLE_RX(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_MASK) >> DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_SHIFT)
#define GET_DOT11AX_MIDAMBLE_RX_NSTS(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_MIDAMBLE_RX_MAX_NSTS_MASK) >> DOT11AX_PHY_CAP_MIDAMBLE_RX_MAX_NSTS_SHIFT)
#define GET_DOT11AX_DCM_MAX_CONSTELLATION_TX(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_MASK) >> DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_SHIFT)
#define GET_DOT11AX_DCM_MAX_CONSTELLATION_RX(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_MASK) >> DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_SHIFT)
#define GET_DOT11AX_DCM_MAX_RU(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_DCM_MAX_RU_MASK) >> DOT11AX_PHY_CAP_DCM_MAX_RU_SHIFT)
#define GET_DOT11AX_BFEE_STS_LE_EQ_80M(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_MASK) >> DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_SHIFT)
#define GET_DOT11AX_BFEE_STS_GT_80M(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_BFEE_STS_GT_80M_MASK) >> DOT11AX_PHY_CAP_BFEE_STS_GT_80M_SHIFT)
#define GET_DOT11AX_SND_DIM_LE_EQ_80M(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_MASK) >> DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_SHIFT)
#define GET_DOT11AX_SND_DIM_GT_80M(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_MASK) >> DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_SHIFT)
#define GET_DOT11AX_BFEE_MAX_NC(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_MAX_NC_MASK) >> DOT11AX_PHY_CAP_MAX_NC_SHIFT)

struct GNU_PACKED he_phy_capinfo {
	UINT32 phy_capinfo_1;
	UINT32 phy_capinfo_2;
	UINT8 phy_capinfo_3;
	UINT8 phy_capinfo_4;
	UINT8 phy_capinfo_5;
};

/* Supported HE-MCS and Nss Set field */

/* Highest MCS Supported subfield */
enum {
	HE_MCS_0_7,
	HE_MCS_0_9,
	HE_MCS_0_11,
	HE_MCS_NOT_SUPPORT
};

#define FIXED_RATE_CMD_AUTO_MCS 33

enum {
	HE_MCS_0,
	HE_MCS_1,
	HE_MCS_2,
	HE_MCS_3,
	HE_MCS_4,
	HE_MCS_5,
	HE_MCS_6,
	HE_MCS_7,
	HE_MCS_8,
	HE_MCS_9,
	HE_MCS_10,
	HE_MCS_11,
	HE_MCS_MAX = HE_MCS_11,
};

/* max_mcs_nss */
#define DOT11AX_MCS_NSS_SHIFT 2
#define DOT11AX_MCS_NSS_MASK 0x3
#define DOT11AX_MCS_1SS_SHIFT 0
#define DOT11AX_MCS_1SS_MASK DOT11AX_MCS_NSS_MASK
#define DOT11AX_MCS_2SS_SHIFT 2
#define DOT11AX_MCS_2SS_MASK (DOT11AX_MCS_NSS_MASK << 2)
#define DOT11AX_MCS_3SS_SHIFT 4
#define DOT11AX_MCS_3SS_MASK (DOT11AX_MCS_NSS_MASK << 4)
#define DOT11AX_MCS_4SS_SHIFT 6
#define DOT11AX_MCS_4SS_MASK (DOT11AX_MCS_NSS_MASK << 6)
#define DOT11AX_MCS_5SS_SHIFT 8
#define DOT11AX_MCS_5SS_MASK (DOT11AX_MCS_NSS_MASK << 8)
#define DOT11AX_MCS_6SS_SHIFT 10
#define DOT11AX_MCS_6SS_MASK (DOT11AX_MCS_NSS_MASK << 10)
#define DOT11AX_MCS_7SS_SHIFT 12
#define DOT11AX_MCS_7SS_MASK (DOT11AX_MCS_NSS_MASK << 12)
#define DOT11AX_MCS_8SS_SHIFT 14
#define DOT11AX_MCS_8SS_MASK (DOT11AX_MCS_NSS_MASK << 14)
#define DOT11AX_MAX_STREAM 8

#define HE_MAX_MCS_NSS(nss, mcs)\
	((mcs) << (DOT11AX_MCS_ ## nss ## SS_SHIFT))

struct GNU_PACKED he_txrx_mcs_nss {
	UINT16 max_rx_mcs_nss;
	UINT16 max_tx_mcs_nss;
};

/* PPE Threshold field */
enum {
	PPE_NSTS1,
	PPE_NSTS2,
	PPE_NSTS3,
	PPE_NSTS4,
	PPE_NSTS5,
	PPE_NSTS6,
	PPE_NSTS7,
	PPE_NSTS8,
	PPE_NSTS_NUM
};

#define DOT11AX_PPE_NSTS_MASK 0x7
#define DOT11AX_PPE_RU_IDX_BITMASK_SHIFT 3
#define DOT11AX_PPE_RU_IDX_BITMASK_MASK (0xF << 3)
#define DOT11AX_PPE_PPET8_PPET16_BITS_NUM 6
#define DOT11AX_PPE_RU_IDX_MSK(ru_alloc_idx) \
	((ru_alloc_idx) << DOT11AX_PPE_RU_IDX_BITMASK_SHIFT)

enum {
	RU_IDX_ALLOC_SZ_242,
	RU_IDX_ALLOC_SZ_484,
	RU_IDX_ALLOC_SZ_996,
	RU_IDX_ALLOC_SZ_2x996,
	RU_IDX_ALLOC_NUM
};

#define DOT11AX_RU242_ALLOC_IDX_BITMSK (1 << RU_IDX_ALLOC_SZ_242)
#define DOT11AX_RU484_ALLOC_IDX_BITMSK (1 << RU_IDX_ALLOC_SZ_484)
#define DOT11AX_RU996_ALLOC_IDX_BITMSK (1 << RU_IDX_ALLOC_SZ_996)
#define DOT11AX_RU2x996_ALLOC_IDX_BITMSK (1 << RU_IDX_ALLOC_SZ_2x996)

enum {
	CONSTELLATION_BPSK_IDX,
	CONSTELLATION_QPSK_IDX,
	CONSTELLATION_16QAM_IDX,
	CONSTELLATION_64QAM_IDX,
	CONSTELLATION_256QAM_IDX,
	CONSTELLATION_1024QAM_IDX,
	CONSTELLATION_RESV_IDX,
	CONSTELLATION_NONE_IDX,
};

#define DOT11AX_PPET8_CONSTELLATION_IDX_SHIFT 3
#define DOT11AX_PPET_CONSTELLATION_IDX_MSK 0x7

struct GNU_PACKED he_cap_ie {
	struct he_mac_capinfo mac_cap;
	struct he_phy_capinfo phy_cap;
	struct he_txrx_mcs_nss txrx_mcs_nss;
};

typedef struct srg_sr_info {
	UINT8 ucObssPdMinOffset;
	UINT8 ucObssPdMaxOffset;
	UINT64 u8BSSColorBitmap;
	UINT64 u8PartialBSSIDBitmap;
} SRG_SR_INFO;

typedef struct he_sr_ie {
	UINT8  sr_control;
	UINT8  non_srg_obss_pd_max_offset;
	struct srg_sr_info srg_sr_info;
} HE_SR_IE;

/*
 * HE Operation Element
 */
/* he_op_parm1 b0..b15*/
#define DOT11AX_OP_DEFAULT_PE_DURATION_MASK 0x7
#define DOT11AX_OP_TWT_REQUIRED (1 << 3)
#define DOT11AX_OP_TXOP_DUR_RTS_THLD_SHIFT 4
#define DOT11AX_OP_TXOP_DUR_RTS_THLD_MASK (0x3ff << 4)
#define DOT11AX_OP_VHT_OPINFO_PRESENT (1 << 14)
#define DOT11AX_OP_CO_HOSTED_BSS (1 << 15)
/* he_op_parm2: b16..b23*/
#define DOT11AX_OP_ER_SU_DISABLE 0x1
#define DOT11AX_OP_6G_OPINFO_PRESENT (1 << 1)
/* BSS color information */
#define DOT11AX_BSS_COLOR_MASK 0x3f
#define DOT11AX_PARTIAL_BSS_COLOR (1 << 6)
#define DOT11AX_BSS_COLOR_DISABLE (1 << 7)
/* WMM-2.2.1 WMM QoS Info field */
#define WMM_QOS_INFO_PARAM_SET_CNT          BITS(0, 3)	/* Sent by AP */
#define WMM_QOS_INFO_UAPSD                  BIT(7)
/* WMM-2.2.2 WMM ACI/AIFSN field */
/* -- subfields in the ACI/AIFSN field */
#define WMM_ACIAIFSN_AIFSN                  BITS(0, 3)
#define WMM_ACIAIFSN_ACM                    BIT(4)
#define WMM_ACIAIFSN_ACI                    BITS(5, 6)
#define WMM_ACIAIFSN_ACI_OFFSET             5
/* -- definitions for ECWmin/ECWmax field */
#define WMM_ECW_WMIN_MASK                   BITS(0, 3)
#define WMM_ECW_WMAX_MASK                   BITS(4, 7)
#define WMM_ECW_WMAX_OFFSET                 4

/* Spatial Reuse Parameter Set element - SR Control field */
#define SR_PARAM_SRP_DISALLOWED                        BIT(0)
#define SR_PARAM_SRP_DISALLOWED_SHFT                   0
#define SR_PARAM_NON_SRG_OBSS_PD_SR_DISALLOWED         BIT(1)
#define SR_PARAM_NON_SRG_OBSS_PD_SR_DISALLOWED_SHFT    1
#define SR_PARAM_NON_SRG_OFFSET_PRESENT                BIT(2)
#define SR_PARAM_NON_SRG_OFFSET_PRESENT_SHFT           2
#define SR_PARAM_SRG_INFO_PRESENT                      BIT(3)
#define SR_PARAM_SRG_INFO_PRESENT_SHFT                 3
#define SR_PARAM_HESIGA_SR_VALUE15_ALLOWED             BIT(4)
#define SR_PARAM_HESIGA_SR_VALUE15_SHFT                4

/*HE 6 GHz Operation Information: Control field*/
#define HE_6G_OP_CONTROL_CH_WIDTH_MASK 0x3
#define HE_6G_OP_CONTROL_DUP_BCN_SHIFT 2
#define HE_6G_OP_CONTROL_DUP_BCN_MASK (1 << 2)

#define HE_6G_OP_CTRL_SET_CH_WIDTH(val) \
	((val) & HE_6G_OP_CONTROL_CH_WIDTH_MASK)
#define HE_6G_OP_CTRL_GET_CH_WIDTH(val) \
	((val) & HE_6G_OP_CONTROL_CH_WIDTH_MASK)
#define HE_6G_OP_CTRL_GET_DUP_BCN(val) \
	(((val) & HE_6G_OP_CONTROL_DUP_BCN_MASK) >> HE_6G_OP_CONTROL_DUP_BCN_SHIFT)

/*HE 6 GHz Operation Information: Minimum Rate field*/
/*unit: 1 Mb/s*/

/* MU EDCA parameters for each AC */
struct GNU_PACKED he_mu_edca_params {
	UINT8 ucECWmin;	/* CWmin */
	UINT8 ucECWmax;	/* CWmax */
	UINT8 ucAifsn;		/* AIFSN */
	UINT8 ucIsACMSet;
	UINT8 ucMUEdcaTimer;
	UINT8 aucPadding[3];
};

struct GNU_PACKED he_op_params {
	UINT16 param1;
	UINT8 param2;
};

struct GNU_PACKED he_op_ie {
	struct he_op_params he_op_param;
	UINT8 bss_color_info;
	UINT16 he_basic_mcs_nss;
};

struct GNU_PACKED he_6g_op_info {
	UINT8 prim_ch;
	UINT8 ctrl;
	UINT8 ccfs_0;
	UINT8 ccfs_1;
	UINT8 min_rate;
};

/*
 * Spatial Reuse Parameter Set Element
 */
/* SR Control */
#define DOT11AX_SR_CTRL_SRP_DISALLOW 0x1
#define DOT11AX_SR_CTRL_NONSRG_OBSS_PD_SR_DISALLOW (1 << 1)
#define DOT11AX_SR_CTRL_NONSRG_OFFSET_PRESENT (1 << 2)
#define DOT11AX_SR_CTRL_SRG_INFO_PRESENT (1 << 3)
#define DOT11AX_SR_CTRL_SRG_HESIGA_SR_VALUE15_ALLOW (1 << 4)

/*
 * MU EDCA Parameter Set Element
 */
/* aci_aifsn */
#define DOT11AX_MU_EDCA_ACI_AIFSN_MASK 0xf
#define DOT11AX_MU_EDCA_ACI_AIFSN_ACM (1 << 4)
#define DOT11AX_MU_EDCA_ACI_AIFSN_ACI_SHIFT 5
#define DOT11AX_MU_EDCA_ACI_AIFSN_ACI_MASK (0x3 << 5)
/*ecwmax_ecwmin*/
#define DOT11AX_MU_EDCA_ECWIN_MIN_MASK 0xf
#define DOT11AX_MU_EDCA_ECWIN_MAX_SHIFT 4
#define DOT11AX_MU_EDCA_ECWIN_MAX_MASK (0xf << 4)
struct GNU_PACKED mu_ac_param_record {
	UINT8 aci_aifsn;
	UINT8 ec_wmax_wmin;
	UINT8 mu_edca_timer;
};

/* QoS Info field for AP */
#define DOT11AX_AP_QOS_INFO_EDCA_UPDATE_CNT_MASK 0xF
#define DOT11AX_AP_QOS_INFO_QACK (1 << 4)
#define DOT11AX_AP_QOS_INFO_QUE_REQ (1 << 5)
#define DOT11AX_AP_QOS_INFO_TXOP_REQ (1 << 6)
/* QoS Info field for non-AP STA */
#define DOT11AX_NON_AP_QOS_INFO_VO_UAPSD 1
#define DOT11AX_NON_AP_QOS_INFO_VI_UAPSD (1 << 1)
#define DOT11AX_NON_AP_QOS_INFO_BK_UAPSD (1 << 2)
#define DOT11AX_NON_AP_QOS_INFO_BE_UAPSD (1 << 3)
#define DOT11AX_NON_AP_QOS_INFO_QACK (1 << 4)
#define DOT11AX_NON_AP_QOS_INFO_QUE_REQ_SHIFT 5
#define DOT11AX_NON_AP_QOS_INFO_QUE_REQ_MASK (0x3 << 5)
#define DOT11AX_NON_AP_QOS_INFO_MORE_DATA_ACK (1 << 7)

struct GNU_PACKED mu_edca_params {
	UINT8 qos_info;
	struct mu_ac_param_record ac_rec[ACI_AC_NUM];
};

/*
 * Max_tx_pwr_interpretation:
 * 0: EIRP
 * 1: EIRP PSD
 * 2: Regulatory client EIRP
 * 3: Regulatory client EIRP PSD
 * 4~7: rsv
 */
#define TX_PWR_INTERPRET_EIRP                  0
#define TX_PWR_INTERPRET_EIRP_PSD              1
#define TX_PWR_INTERPRET_REG_CLIENT_EIRP       2
#define TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD   3
#define TX_PWR_CATEGORY_DEFAULT                0
#define TX_PWR_CATEGORY_SUBORDINATE_DEV        1

typedef struct GNU_PACKED _HE_TX_PWR_INFO_ {
#ifdef RT_BIG_ENDIAN
	UINT8 max_tx_pwr_category:2;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 max_tx_pwr_cnt:3;
#else
	UINT8 max_tx_pwr_cnt:3;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 max_tx_pwr_category:2;
#endif
} HE_TX_PWR_INFO;

/*
 * IEEE 802.11AX , sec 9.4.2.164
 * Transmit Power Envelope element
 *
 * max_txpwr: Maximum Transmit Power
 * Define the maximum transmit power limit of the tx bandwidth defined
 * by the Transmit Power Envelop element. The Maximum Transmit
 * Power field is a 8 bit 2's complement signed integer in the range of
 * -64 dBm to 63.5 dBm with a 0.5 dB step.
 */
typedef struct GNU_PACKED _HE_TXPWR_ENV_IE {
	HE_TX_PWR_INFO tx_pwr_info;
	UINT8 tx_pwr_bw[8];
} HE_TXPWR_ENV_IE;

/*
 * BSS Color Change Announcement Element
 */
#define DOT11AX_NEW_BSS_COLOR_MASK 0x3F
struct GNU_PACKED bss_color_change {
	UINT8 color_switch_cnt_down;
	UINT8 new_bss_color_info;
};

/*
 * OFDMA-based Random Access Parameter Set (RAPS) Element
 */
#define DOT11AX_RAPS_OCW_EOCWMIN_MASK 0x7
#define DOT11AX_RAPS_OCW_EOCWMAX_SHIFT 3
#define DOT11AX_RAPS_OCW_EOCWMAX_MASK (0x7 << 3)
struct GNU_PACKED ul_ofdma_random_access {
	UINT8 ocw_range;
};

/*
 * HE 6 GHz Band Capabilities element
 */
#define DOT11AX_6G_CAP_MIN_MPDU_START_SPACE_MASK 0x7
#define DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_SHIFT 3
#define DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_MASK (0x7 << 3)
#define DOT11AX_6G_CAP_MAX_MPDU_LEN_SHIFT 6
#define DOT11AX_6G_CAP_MAX_MPDU_LEN_MASK (0x3 << 6)
#define DOT11AX_6G_CAP_SMPS_SHIFT 9
#define DOT11AX_6G_CAP_SMPS_MASK (0x3 << 9)
#define DOT11AX_6G_CAP_RD_RESP_SHIFT 11
#define DOT11AX_6G_CAP_RD_RESP_MASK (1 << 11)
#define DOT11AX_6G_CAP_RX_ANT_PATTERN_CONSIST_SHIFT 12
#define DOT11AX_6G_CAP_RX_ANT_PATTERN_CONSIST_MASK (1 << 12)
#define DOT11AX_6G_CAP_TX_ANT_PATTERN_CONSIST_SHIFT 13
#define DOT11AX_6G_CAP_TX_ANT_PATTERN_CONSIST_MASK (1 << 13)

struct GNU_PACKED he_6g_cap_ie {
	UINT16 caps_info;
};

/*SET*/
#define DOT11AX_6G_SET_MAX_MPDU_START_SPACE(val) \
	((val) & DOT11AX_6G_CAP_MIN_MPDU_START_SPACE_MASK)
#define DOT11AX_6G_SET_MAX_AMPDU_LEN_EXP(val) \
	(((val) << DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_SHIFT) & DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_MASK)
#define DOT11AX_6G_SET_MPDU_LEN(val) \
	(((val) << DOT11AX_6G_CAP_MAX_MPDU_LEN_SHIFT) & DOT11AX_6G_CAP_MAX_MPDU_LEN_MASK)
#define DOT11AX_6G_SET_SMPS(val) \
	(((val) << DOT11AX_6G_CAP_SMPS_SHIFT) & DOT11AX_6G_CAP_SMPS_MASK)
/*GET*/
#define DOT11AX_6G_GET_MAX_MPDU_START_SPACE(val) \
	((val) & DOT11AX_6G_CAP_MIN_MPDU_START_SPACE_MASK)
#define DOT11AX_6G_GET_MAX_AMPDU_LEN_EXP(val) \
	(((val) & DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_MASK) >> DOT11AX_6G_CAP_MAX_AMPDU_LEN_EXP_SHIFT)
#define DOT11AX_6G_GET_MPDU_LEN(val) \
	(((val) & DOT11AX_6G_CAP_MAX_MPDU_LEN_MASK) >> DOT11AX_6G_CAP_MAX_MPDU_LEN_SHIFT)
#define DOT11AX_6G_GET_SMPS(val) \
	(((val) & DOT11AX_6G_CAP_SMPS_MASK) >> DOT11AX_6G_CAP_SMPS_SHIFT)


/* HE 6G RNR TBTT Information */
struct GNU_PACKED he_6g_rnr_tbtt_info_set_w_ssid {
	UINT8 nap_tbtt_offset;
	UINT8 bssid[6];
	UINT32 short_ssid;
	UINT8 bss_param;
	UINT8 psd_bw20;
};

struct GNU_PACKED he_6g_rnr_tbtt_info_set_wo_ssid {
	UINT8 nap_tbtt_offset;
	UINT8 bssid[6];
	UINT8 bss_param;
	UINT8 psd_bw20;
};

#endif /* _DOT11AX_HE_H_ */
