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

#ifndef __FMAC_TXD_H__
#define __FMAC_TXD_H__

/* TXD DW0 */
#define TXD_TX_BYTE_COUNT_MASK 0xffff
#define TXD_TX_BYTE_COUNT_SHIFT 0
#define TXD_ETH_TYPE_OFFSET_MASK (0x7f << 16)
#define TXD_ETH_TYPE_OFFSET_SHIFT 16

enum pkt_ft_hif{
	FT_HIF_CTD = 0, /* Cut-through */
	FT_HIF_SF,      /* Store & forward  */
	FT_HIF_CMD,     /* Command frame to N9/CR4 */
	FT_HIF_FD,      /* Firmware frame to PDA */
};
enum pkt_ft_mcu{
	FT_MCU_CTD = 0, /* N9 Cut-through */
	FT_MCU_SF,      /* N9 Store & forward  */
	FT_MCU_RSV,     /* Reserved */
	FT_MCU_FW,      /* N9 to UMAC/LMAC */
};

#define TXD_PKT_FT_MASK (0x3 << 23)
#define TXD_PKT_FT_SHIFT 23

enum q_idx {
	LMAC_AC00	= 0x00,
	LMAC_AC01	= 0x01,
	LMAC_AC02	= 0x02,
	LMAC_AC03	= 0x03,
	LMAC_AC10	= 0x04,
	LMAC_AC11	= 0x05,
	LMAC_AC12	= 0x06,
	LMAC_AC13	= 0x07,
	LMAC_AC20	= 0x08,
	LMAC_AC21	= 0x09,
	LMAC_AC22	= 0x0a,
	LMAC_AC23	= 0x0b,
	LMAC_AC30	= 0x0c,
	LMAC_AC31	= 0x0d,
	LMAC_AC32	= 0x0e,
	LMAC_AC33	= 0x0f,
	ALTX0		= 0x10,
	BMC0		= 0x10,
	BCN0		= 0x12,
	NAF		= 0x18,
	NBCN		= 0x19,

	MCU_RQ0		= 0x20,
	MCU_RQ1		= 0x21,
	MCU_RQ2		= 0x22,
	MCU_RQ3		= 0x23,

	PDA_FW_DL	= 0x3e,

	TXCMD_AC00	= 0x40,
	TXCMD_AC01	= 0x41,
	TXCMD_AC02	= 0x42,
	TXCMD_AC03	= 0x43,
	TXCMD_AC10	= 0x44,
	TXCMD_AC11	= 0x45,
	TXCMD_AC12	= 0x46,
	TXCMD_AC13	= 0x47,
	TXCMD_AC20	= 0x48,
	TXCMD_AC21	= 0x49,
	TXCMD_AC22	= 0x4a,
	TXCMD_AC23	= 0x4b,
	TXCMD_AC30	= 0x4c,
	TXCMD_AC31	= 0x4d,
	TXCMD_AC32	= 0x4e,
	TXCMD_AC33	= 0x4f,
	ALTXCMD0	= 0x50,
	TF0		= 0x51,
	TWT_TSF_TF0	= 0x52,
	TWT_DL0		= 0x53,
	TWT_UL0		= 0x54,
};
#define TXD_Q_IDX_MASK (0x7f << 25)
#define TXD_Q_IDX_SHIFT 25

/* TXD DW1 */
#define TXD_WLAN_IDX_MASK 0x3ff
#define TXD_WLAN_IDX_SHIFT 0

#define TXD_VTA (1 << 10)

#define TXD_HDR_LEN_MASK (0x1f << 11)
#define TXD_HDR_LEN_SHIFT 11

#define TXD_EOSP (1 << 12)
#define TXD_AMS (1 << 13)

#define TXD_MRD (1 << 11)
#define TXD_RMVL (1 << 13)
#define TXD_VLAN (1 << 14)
#define TXD_ETYP (1 << 15)

enum {
	HF_802_3_FRAME,
	HF_CMD_FRAME,
	HF_802_11_FRAME,
	HF_802_11_EN_FRAME,
};
#define TXD_HF_MASK (0x3 << 16)
#define TXD_HF_SHIFT 16

#define TXD_HDR_PAD_MASK (0x3 << 18)
#define TXD_HDR_PAD_SHIFT 18
#define TXD_TID_MASK (0x7 << 20)
#define TXD_TID_SHIFT 20
#define TXD_AMSDU (1 << 23)

#define TXD_OM_MASK (0x3f << 24)
#define TXD_OM_SHIFT 24

#define TXD_TGID (1 << 30)

#define TXD_FT (1 << 31)

/* TXD DW2 */
#define TXD_SUBTYPE_MASK (0xf)
#define TXD_SUBTYPE_SHIFT 0
#define TXD_TYPE_MASK (0x3 << 4)
#define TXD_TYPE_SHIFT 4
#define TXD_NDP (1 << 6)
#define TXD_NDPA (1 << 7)
#define TXD_SD (1 << 8)
#define TXD_RTS (1 << 9)
#define TXD_BM (1 << 10)
#define TXD_B (1 << 11)
#define TXD_DU (1 << 12)
#define TXD_HE (1 << 13)

enum {
	NO_FRAG,
	FIRST_FRAG,
	MIDDLE_FRAG,
	LAST_FRAG,
};
#define TXD_FRAG_MASK (0x3 << 14)
#define TXD_FRAG_SHIFT 14

#define TXD_REMAIN_TIME_MASK (0x7f << 16)	/* in unit of 64TU, MSB bit is reserved for HW ans SW should set to 0 */
#define TXD_REMAIN_TIME_SHIFT 16
#define TXD_PWR_OFFESET_MASK (0x3f << 24)
#define TXD_PWR_OFFESET_SHIFT 24
#define TXD_FRM (1 << 30)
#define TXD_FR (1 << 31)

/* TXD DW3 */
#define TXD_NA (1 << 0)
#define TXD_PF (1 << 1)
#define TXD_EMRD (1 << 2)
#define TXD_EEOSP (1 << 3)
#define TXD_DAS (1 << 4)
#define TXD_TM (1 << 5)
#define TXD_TX_CNT_MASK (0x1f << 6)
#define TXD_TX_CNT_SHIFT 6
#define TXD_REMAIN_TX_CNT_MASK (0x1f << 11)
#define TXD_REMAIN_TX_CNT_SHIFT 11
#define TXD_SN_MASK (0xfff << 16)
#define TXD_SN_SHIFT 16
#define TXD_BA_DIS (1 << 28)
#define TXD_BA_DIS_SHIFT 28
#define TXD_PM (1 << 29)
#define TXD_PN_VLD (1 << 30)
#define TXD_SN_VLD (1 << 31)

/* TXD DW4 */
#define TXD_PN1_MASK (0xffffffff)
#define TXD_PN1_SHIFT 0

/* TXD DW5 */
#define TXD_PID_MASK (0xff)
#define TXD_PID_SHIFT 0
#define TXD_TXSFM (1 << 8)
#define TXD_TXSFM_SHIFT 8
#define TXD_TXS2M (1 << 9)
#define TXD_TXS2M_SHIFT 9
#define TXD_TXS2H (1 << 10)
#define TXD_TXS2H_SHIFT 10
#define TXD_FBCZ_SHIFT 12
#define TXD_FBCZ (1 << TXD_FBCZ_SHIFT)
#define TXD_ADD_BA (1 << 14)
#define TXD_MD (1 << 15)
#define TXD_PN2_MASK (0xffff << 16)
#define TXD_PN2_SHIFT 16

/* TXD DW6 */
#define TXD_BW_MASK (0x7)
#define TXD_BW_SHIFT 0
#define TXD_DYN_BW (1 << 3)
#define TXD_ANT_ID_MASK (0xf << 4)
#define TXD_ANT_ID_SHIFT 4
#define TXD_SPE_IDX_SEL (1 << 10)
#define TXD_SPE_IDX_SEL_SHIFT 10
/* MT7915 move from DW5[11][13:12][14] */
#define TXD_LDPC (1 << 11)
#define TXD_HELTF_TYPE_MASK (0x3 << 12)
#define TXD_HELTF_TYPE_SHIFT 12
#define TXD_GI_MASK (0x3 << 14)
#define TXD_GI_SHIFT 14
#define TXD_FR_RATE_MASK (0x3fff << 16)
#define TXD_FR_RATE_SHIFT 16
#define TXD_TX_RATE_BIT_SUEXTTONE 5	/* TX format Doc. is incorrect */
#define TXD_TX_RATE_BIT_DCM 4		/* TX format Doc. is incorrect */
#define TXD_TX_RATE_BIT_STBC 13
#define TXD_TX_RATE_BIT_NSS 10
#define TXD_TX_RATE_MASK_NSS 0x7
#define TXD_TX_RATE_BIT_MODE 6
#define TXD_TX_RATE_MASK_MODE 0xf


#define TXD_TXEBF (1 << 30)
#define TXD_TXIBF (1 << 31)

/* TXD DW7 */
#define TXD_SW_TX_TIME_MASK (0x3ff)
#define TXD_SW_TX_TIME_SHIFT 0
#define TXD_TAT (0x3ff)
#define TXD_TAT_SHIFT 0
#define TXD_HW_AMSDU_CAP (1 << 10)
#define TXD_SPE_IDX_MASK (0x1f << 11)
#define TXD_SPE_IDX_SHIFT 11
#define TXD_PSE_FID_MASK (0xfff << 16)
#define TXD_PSE_FID_SHIFT 16
#define TXD_PP_SUBTYPE_MASK (0xf << 16)
#define TXD_PP_SUBTYPE_SHIFT 16
#define TXD_PP_TYPE_MASK (0x3 << 20)
#define TXD_PP_TYPE_SHIFT 20
#define TXD_CTXD_CNT_MASK (0x7 << 23)
#define TXD_CTXD_CNT_SHIFT 23
#define TXD_CTXD (1 << 26)
#define TXD_IP_CHKSUM (1 << 28)
#define TXD_UT (1 << 29)

#define TXDLEN_PAGE_SIZE 64
enum {
	TXDLEN_1_PAGE,
	TXDLEN_2_PAGE,
	TXDLEN_3_PAGE,
	TXDLEN_4_PAGE,
};
#define TXD_TXD_LEN_MASK (0x3 << 30)
#define TXD_TXD_LEN_SHIFT 30

/* TXD DW8 */
#define TXD_MSDU_ID0_MASK (0x7fff)
#define TXD_MSDU_ID0_SHIFT 0
#define TXD_MSDU_VLD0 (1 << 15)
#define TXD_MSDU_ID1_MASK (0x7fff << 16)
#define TXD_MSDU_ID1_SHIFT 16
#define TXD_MSDU_VLD1 (1 << 31)

/* TXD DW9 */
#define TXD_MSDU_ID2_MASK (0x7fff)
#define TXD_MSDU_ID2_SHIFT 0
#define TXD_MSDU_VLD2 (1 << 15)
#define TXD_MSDU_ID3_MASK (0x7fff << 16)
#define TXD_MSDU_ID3_SHIFT 16
#define TXD_SSN (0xfff << 16)
#define TXD_SSN_SHIFT 16
#define TXD_MSDU_VLD3 (1 << 31)

/* TXD DW10 */
#define TXD_TXP_ADDR0_MASK (0xffffffff)
#define TXD_TXP_ADDR0_SHIFT 0

/* TXD DW11 */
#define TXD_TXP_LEN0_MASK (0x3fff)
#define TXD_TXP_LEN0_SHIFT 0
#define TXD_ML0 (1 << 14)
#define TXD_AL0 (1 << 15)
#define TXD_TXP_LEN1_MASK (0X3fff << 16)
#define TXD_TXP_LEN1_SHIFT 16
#define TXD_ML1 (1 << 30)
#define TXD_AL1 (1 << 31)

/* TXD DW12 */
#define TXD_TXP_ADDR1_MASK (0xffffffff)
#define TXD_TXP_ADDR1_SHIFT 0

/* TXD DW13 */
#define TXD_TXP_ADDR2_MASK (0xffffffff)
#define TXD_TXP_ADDR2_SHIFT 0

/* TXD DW14 */
#define TXD_TXP_LEN2_MASK (0x3fff)
#define TXD_TXP_LEN2_SHIFT 0
#define TXD_ML2 (1 << 14)
#define TXD_AL2 (1 << 15)
#define TXD_TXP_LEN3_MASK (0X3fff << 16)
#define TXD_TXP_LEN3_SHIFT 16
#define TXD_ML3 (1 << 30)
#define TXD_AL3 (1 << 31)

/* TXD DW15 */
#define TXD_TXP_ADDR3_MASK (0xffffffff)
#define TXD_TXP_ADDR3_SHIFT 0

/* TXD DW16 */
#define TXD_TXP_ADDR4_MASK (0xffffffff)
#define TXD_TXP_ADDR4_SHIFT 0

/* TXD DW17 */
#define TXD_TXP_LEN4_MASK (0x3fff)
#define TXD_TXP_LEN4_SHIFT 0
#define TXD_ML4 (1 << 14)
#define TXD_AL4 (1 << 15)
#define TXD_TXP_LEN5_MASK (0X3fff << 16)
#define TXD_TXP_LEN5_SHIFT 16
#define TXD_ML5 (1 << 30)
#define TXD_AL5 (1 << 31)

/* TXD DW18 */
#define TXD_TXP_ADDR5_MASK (0xffffffff)
#define TXD_TXP_ADDR5_SHIFT 0

/* TXD DW19 */
#define TXD_TXP_ADDR6_MASK (0xffffffff)
#define TXD_TXP_ADDR6_SHIFT 0

/* TXD DW20 */
#define TXD_TXP_LEN6_MASK (0x3fff)
#define TXD_TXP_LEN6_SHIFT 0
#define TXD_ML6 (1 << 14)
#define TXD_AL6 (1 << 15)
#define TXD_TXP_LEN7_MASK (0X3fff << 16)
#define TXD_TXP_LEN7_SHIFT 16
#define TXD_ML7 (1 << 30)
#define TXD_AL7 (1 << 31)

/* TXD DW21 */
#define TXD_TXP_ADDR7_MASK (0xffffffff)
#define TXD_TXP_ADDR7_SHIFT 0

/* TXD DW22 */
#define TXD_TXP_ADDR8_MASK (0xffffffff)
#define TXD_TXP_ADDR8_SHIFT 0

/* TXD DW23 */
#define TXD_TXP_LEN8_MASK (0x3fff)
#define TXD_TXP_LEN8_SHIFT 0
#define TXD_ML8 (1 << 14)
#define TXD_AL8 (1 << 15)
#define TXD_TXP_LEN9_MASK (0X3fff << 16)
#define TXD_TXP_LEN9_SHIFT 16
#define TXD_ML9 (1 << 30)
#define TXD_AL9 (1 << 31)

/* TXD DW24 */
#define TXD_TXP_ADDR9_MASK (0xffffffff)
#define TXD_TXP_ADDR9_SHIFT 0

/* TXD DW25 */
#define TXD_TXP_ADDR10_MASK (0xffffffff)
#define TXD_TXP_ADDR10_SHIFT 0

/* TXD DW26 */
#define TXD_TXP_LEN10_MASK (0x3fff)
#define TXD_TXP_LEN10_SHIFT 0
#define TXD_ML10 (1 << 14)
#define TXD_AL10 (1 << 15)
#define TXD_TXP_LEN11_MASK (0X3fff << 16)
#define TXD_TXP_LEN11_SHIFT 16
#define TXD_ML11 (1 << 30)
#define TXD_AL11 (1 << 31)

/* TXD DW27 */
#define TXD_TXP_ADDR11_MASK (0xffffffff)
#define TXD_TXP_ADDR11_SHIFT 0

/* TXD DW28 */
#define TXD_TXP_ADDR12_MASK (0xffffffff)
#define TXD_TXP_ADDR12_SHIFT 0

/* TXD DW29 */
#define TXD_TXP_LEN12_MASK (0x3fff)
#define TXD_TXP_LEN12_SHIFT 0
#define TXD_ML12 (1 << 14)
#define TXD_AL12 (1 << 15)
#define TXD_TXP_LEN13_MASK (0X3fff << 16)
#define TXD_TXP_LEN13_SHIFT 16
#define TXD_ML13 (1 << 30)
#define TXD_AL13 (1 << 31)

/* TXD DW30 */
#define TXD_TXP_ADDR13_MASK (0xffffffff)
#define TXD_TXP_ADDR13_SHIFT 0


/* TXS Header */
#define TXS_RX_BYTE_CNT_MASK (0xffff)
#define TXS_RX_BYTE_CNT_SHIFT 0
#define TXS_CNT_MASK (0x1f << 16)
#define TXS_CNT_SHIFT 16
#define TXS_PKT_TYPE_MASK (0x1f << 27)
#define TXS_PKT_TYPE_SHIFT 27

/* DW0 */
#define TXS_TX_RATE_MASK (0x3fff)
#define TXS_TX_RATE_SHIFT 0
#define TXS_TXS2M (1 << 14)
#define TXS_TXS2H (1 << 15)
#define TXS_F_ME (1 << 16)
#define TXS_F_RE (1 << 17)
#define TXS_F_LE (1 << 18)
#define TXS_F_BE (1 << 19)
#define TXS_F_TXOP_LIMITE (1 << 20)
#define TXS_F_PS (1 << 21)
#define TXS_F_BAF (1 << 22)
#define TXS_TXSFM_MASK (0x3 << 23)
#define TXS_TXSFM_SHIFT 23
#define TXS_AM (1 << 25)
#define TXS_TID_MASK (0x7 << 26)
#define TXS_TID_SHIFT 26
#define TXS_TBW_MASK (0x3 << 29)
#define TXS_TBW_SHIFT 29
#define TXS_FR (1 << 31)

/* DW1 */
#define TXS_TX_PWR_MASK (0xff)
#define TXS_TX_PWR_SHIFT 0
#define TXS_RX_VECTOR_MASK (0xff << 8)
#define TXS_RX_VECTOR_SHIFT 8
#define TXS_RESP_LEGACY_RATE_MASK (0xf << 16)
#define TXS_RESP_LEGACY_RATE_SHIFT 16
#define TXS_SN_MASK (0xfff << 20)
#define TXS_SN_SHIFT 20

/* DW2 */
#define TXS_TX_DELAY_MASK 0xffff
#define TXS_TX_DELAY_SHIFT 0
#define TXS_WLAN_IDX_MASK (0x3ff << 16)
#define TXS_WLAN_IDX_SHIFT 16
#define TXS_LAST_MCS_IDX_MASK (0x7 << 27)
#define TXS_LAST_MCS_IDX_SHIFT 27
#define TXS_ETXBF (1 << 30)
#define TXS_ITXBF (1 << 31)

/* DW3 */
#define TXS_ANT_ID_MASK (0xffffff << 0)
#define TXS_ANT_ID_SHIFT 0
#define TXS_PID_MASK (0xff << 24)
#define TXS_PID_SHIFT 24

/* DW4 */
#define TXS_TIMESTAMP_MASK 0xffffffff
#define TXS_TIMESTAMP_SHIFT 0
#define TXS_TSSI0_MASK (0xfff)
#define TXS_TSSI0_SHIFT 0
#define TXS_TSSI1_MASK (0xfff << 12)
#define TXS_TSSI1_SHIFT 12

/* DW5 */
#define TXS_FRONT_TIME_MASK (0x1ffffff)
#define TXS_FRONT_TIME_SHIFT 0
#define TXS_MPDU_TX_CNT_MASK (0x1f << 25)
#define TXS_MPDU_TX_CNT_SHIFT 25
#define TXS_QOS (1 << 30)
#define TXS_FM (1 << 31)
#define TXS_TSSI2_MASK (0xfff)
#define TXS_TSSI2_SHIFT 0
#define TXS_TSSI3_MASK (0xfff << 12)
#define TXS_TSSI3_SHIFT 12

/* DW5 PPDU Format */
#define TXS_PPDU_MPDU_TX_BYTE_MASK (0x7fffff)
#define TXS_PPDU_MPDU_TX_BYTE_SHIFT 0
#define TXS_PPDU_MPDU_TX_CNT_MASK (0x1ff << 23)
#define TXS_PPDU_MPDU_TX_CNT_SHIFT 23

/* DW6 */
#define TXS_NOISE_MASK_MASK 0xffffffff
#define TXS_NOISE_MASK_SHIFT 0

/* DW6 PPDU Format */
#define TXS_PPDU_MPDU_FAIL_BYTE_MASK (0x7fffff)
#define TXS_PPDU_MPDU_FAIL_BYTE_SHIFT 0
#define TXS_PPDU_MPDU_FAIL_CNT_MASK (0x1ff << 23)
#define TXS_PPDU_MPDU_FAIL_CNT_SHIFT 23

/* DW7 */
#define TXS_RCPI_MASK 0xffffff
#define TXS_RCPI_SHIFT 0

/* DW7 PPDU Format */
#define TXS_PPDU_MPDU_RTY_BYTE_MASK (0x7fffff)
#define TXS_PPDU_MPDU_RTY_BYTE_SHIFT 0
#define TXS_PPDU_MPDU_RTY_CNT_MASK (0x1ff << 23)
#define TXS_PPDU_MPDU_RTY_CNT_SHIFT 23

#endif
