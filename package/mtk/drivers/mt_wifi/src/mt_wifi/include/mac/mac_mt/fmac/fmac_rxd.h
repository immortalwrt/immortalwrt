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

#ifndef __FMAC_RXD_H__
#define __FMAC_RXD_H__

/* RXD DW0 */
#define RXD_RX_BYTE_COUNT_MASK 0xffff
#define RXD_RX_BYTE_COUNT_SHIFT 0
#define RXD_ETH_TYPE_OFFSET_MASK (0x7f << 16)
#define RXD_ETH_TYPE_OFFSET_SHIFT 16
#define RXD_IP (1 << 23)
#define RXD_UT (1 << 24)
#define RXD_DUP_MODE_MASK (0x3 << 25)
#define RXD_DUP_MODE_SHIFT 25

enum {
	RXD_PT_TXS,
	RXD_PT_VEC,
	RXD_PT_RX,
	RXD_PT_D_RX,
	RXD_PT_TMR,
	RXD_PT_RETRIEVE,
	RXD_PT_MSDU_ID_RPT,
	RXD_PT_SW_CMD,
	RXD_PT_SPL,
	RXD_PT_TXCMD_RPT,
	RXD_PT_RX_RPT,
};
#define RXD_PKT_TYPE_MASK (0x1f << 27)
#define RXD_PKT_TYPE_SHIFT 27

/* RXD DW1 */
#define RXD_WLAN_IDX_MASK (0x3ff)
#define RXD_WLAN_IDX_SHIFT 0
#define RXD_GROUP_VLD_MASK (0x1f << 11)
#define RXD_GROUP_VLD_SHIFT 11
#define RXD_GROUP1_VLD (1 << 11)
#define RXD_GROUP2_VLD (1 << 12)
#define RXD_GROUP3_VLD (1 << 13)
#define RXD_GROUP4_VLD (1 << 14)
#define RXD_GROUP5_VLD (1 << 15)
#define RXD_SEC_MODE_MASK (0x1f << 16)
#define RXD_SEC_MODE_SHIFT 16
#define RXD_KID_MASK (0x3 << 21)
#define RXD_KID_SHIFT 21
#define RXD_CM (1 << 23)
#define RXD_CLM (1 << 24)
#define RXD_ICV_ERR (1 << 25)
#define RXD_TKIPMIC_ERR (1 << 26)
#define RXD_FCS_ERR (1 << 27)
#define RXD_BN (1 << 28)
#define RXD_SPP_EN (1 << 29)
#define RXD_ADD_OM (1 << 30)
#define RXD_SEC_DONE (1 < 31)

/* RXD DW2 */
#define RXD_BSSID_MASK (0x3f)
#define RXD_BSSID_SHIFT 0
#define RXD_BF_CQI (1 << 7)
#define RXD_MAC_HDR_LEN_MASK (0x1f << 8)
#define RXD_MAC_HDR_LEN_SHIFT 8
#define RXD_H (1 << 13)
#define RXD_HO_MASK (0x3 << 14)
#define RXD_HO_SHIFT 14
#define RXD_TID_MASK (0xf << 16)
#define RXD_TID_SHIFT 16
#define RXD_MU_BAR (1 << 21)
#define RXD_SWBIT (1 << 22)
#define RXD_DAF (1 << 23)
#define RXD_EL (1 << 24)
#define RXD_HTF (1 << 25)
#define RXD_INTF (1 << 26)
#define RXD_FRAG (1 << 27)
#define RXD_NULL (1 << 28)
#define RXD_NDATA (1 << 29)
#define RXD_NAMP (1 << 30)
#define RXD_BF_RPT (1 << 31)

/* RXD DW3 */
#define RXD_RXV_SN_MASK 0xff
#define RXD_RXV_SN_SHIFT 0
#define RXD_CF_MASK (0xff << 8)
#define RXD_CF_SHIFT 8
#define RXD_A1_TYPE_MASK (0x3 << 16)
#define RXD_A1_TYPE_SHIFT 16
#define RXD_HTC (1 << 18)
#define RXD_TCL (1 << 19)
#define RXD_BBM (1 << 20)
#define RXD_BU (1 << 21)
#define RXD_AMS (1 << 22)
#define RXD_MESH (1 << 23)
#define RXD_MHCP (1 << 24)
#define RXD_NO_INFO_WB (1 << 25)
#define RXD_DIS_RHTR (1 << 26)
#define RXD_PSS (1 << 27)
#define RXD_MORE (1 << 28)
#define RXD_UWAT (1 << 29)
#define RXD_RX_DROP (1 << 30)
#define RXD_VLAN2ETH (1 << 31)

/* RXD DW4 */
#define RXD_PF_MASK (0x3)
#define RXD_PF_SHIFT 0
#define RXD_DP (1 << 9)
#define RXD_CLS (1 << 10)
#define RXD_OFLD_MASK (0x3 << 11)
#define RXD_OFLD_SHIFT 11
#define RXD_MGC (1 << 13)
#define RXD_WOL_MASK (0x1f << 14)
#define RXD_WOL_SHIFT 14
#define RXD_CLS_BITMAP_MASK (0x3ff << 19)
#define RXD_CLS_BITMAP_SHIFT 19
#define RXD_PF_MODE (1 << 29)
#define RXD_PF_STS_MASK (0x3 << 30)
#define RXD_PF_STS_SHIFT 30

/* RXD DW6 */
#define RXD_FC_MASK (0xffff)
#define RXD_FC_SHIFT 0
#define RXD_TA_1_MASK (0xffff << 16)
#define RXD_TA_1_SHIFT 16

/* RXD DW7 */
#define RXD_TA_2_MASK (0xffffffff)
#define RXD_TA_2_SHIFT 0

/* RXD DW8 */
#define RXD_FN_MASK (0xf)
#define RXD_FN_SHIFT 0
#define RXD_SN_MASK (0xfff << 4)
#define RXD_SN_SHIFT 4
#define RXD_AID_MASK (0xffff)
#define RXD_AID_SHIFT 0
#define RXD_QOS_CTL_MASK (0xffff << 16)
#define RXD_QOS_CTL_SHIFT 16

/* RXD DW9 */
#define RXD_HT_CTL_MASK (0xffffffff)
#define RXD_HT_CTL_SHIFT 0

/* RXD DW10 */
#define RXD_PN_0_31_MASK (0xffffffff)
#define RXD_PN_0_31_SHIFT 0

/* RXD DW11 */
#define RXD_PN_32_47_MASK ((0xffff))
#define RXD_PN_32_47_SHIFT 0


/* P-RXVector, 1st Cycle */
#define CONNAC2X_RX_VT_RX_RATE_MASK         BITS(0, 6)
#define CONNAC2X_RX_VT_RX_RATE_OFFSET       0
#define CONNAC2X_RX_VT_NSTS_MASK            BITS(7, 9)
#define CONNAC2X_RX_VT_NSTS_OFFSET          7
#define CONNAC2X_RX_VT_BEAMFORMED_MASK      BIT(10)
#define CONNAC2X_RX_VT_BEAMFORMED_OFFSET    10
#define CONNAC2X_RX_VT_LDPC_MASK            BIT(11)
#define CONNAC2X_RX_VT_LDPC_OFFSET          11
#define CONNAC2X_RX_VT_RU_ALLOC1_MASK       BITS(28, 31)
#define CONNAC2X_RX_VT_RU_ALLOC1_OFFSET     28
#define CONNAC2X_RX_VT_RU_ALLOC2_MASK       BITS(0, 3)
#define CONNAC2X_RX_VT_RU_ALLOC2_OFFSET     0

/* C-RXC Vector, 1st Cycle */
#define CONNAC2X_RX_VT_STBC_MASK            BITS(0, 1)
#define CONNAC2X_RX_VT_STBC_OFFSET          0
#define CONNAC2X_RX_VT_NESS_MASK            BITS(2, 3)
#define CONNAC2X_RX_VT_NESS_OFFSET          2
#define CONNAC2X_RX_VT_RX_MODE_MASK         BITS(4, 7)
#define CONNAC2X_RX_VT_RX_MODE_OFFSET       4
#define CONNAC2X_RX_VT_FR_MODE_MASK         BITS(8, 10)
#define CONNAC2X_RX_VT_FR_MODE_OFFSET       8
#define CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED_MASK     BIT(11)
#define CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED_OFFSET   11
#define CONNAC2X_RX_VT_SHORT_GI_MASK		BITS(13, 14)
#define CONNAC2X_RX_VT_SHORT_GI_OFFSET		13
#define CONNAC2X_RX_VT_LTF_MASK             BITS(17, 18)
#define CONNAC2X_RX_VT_LTF_OFFSET           17
#define CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM_MASK     BIT(20)
#define CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM_OFFSET   20
#define CONNAC2X_RX_VT_PE_DIS_AMB_MASK      BIT(23)
#define CONNAC2X_RX_VT_PE_DIS_AMB_OFFSET    23
#define CONNAC2X_RX_VT_NUM_USER_MASK        BITS(24, 30)
#define CONNAC2X_RX_VT_NUM_USER_OFFSET      24
#define CONNAC2X_RX_VT_UL_DL_MASK           BIT(31)
#define CONNAC2X_RX_VT_UL_DL_OFFSET         31
#define CONNAC2X_RX_VT_SIGB_RU0_MASK        BITS(0, 7)
#define CONNAC2X_RX_VT_SIGB_RU0_OFFSET      0
#define CONNAC2X_RX_VT_SIGB_RU1_MASK        BITS(8, 15)
#define CONNAC2X_RX_VT_SIGB_RU1_OFFSET      8
#define CONNAC2X_RX_VT_SIGB_RU2_MASK        BITS(16, 23)
#define CONNAC2X_RX_VT_SIGB_RU2_OFFSET      16
#define CONNAC2X_RX_VT_SIGB_RU3_MASK        BITS(24, 31)
#define CONNAC2X_RX_VT_SIGB_RU3_OFFSET      24

/* C-RXC Vector, 2nd Cycle */
#define CONNAC2X_RX_VT_GROUP_ID_MASK        BITS(22, 27)
#define CONNAC2X_RX_VT_GROUP_ID_OFFSET      22
#define CONNAC2X_RX_VT_NUM_RX_MASK		BITS(28, 30)
#define CONNAC2X_RX_VT_NUM_RX_OFFSET		28

/* C-RXC Vector, 3rd Cycle */
#define CONNAC2X_RX_VT_PART_AID_MASK        BITS(20, 30)
#define CONNAC2X_RX_VT_PART_AID_OFFSET      20

/* C-RXC Vector, 4th Cycle */
#define CONNAC2X_RX_VT_RCPI0_MASK             BITS(0, 7)
#define CONNAC2X_RX_VT_RCPI0_OFFSET           0
#define CONNAC2X_RX_VT_RCPI1_MASK             BITS(8, 15)
#define CONNAC2X_RX_VT_RCPI1_OFFSET           8
#define CONNAC2X_RX_VT_RCPI2_MASK             BITS(16, 23)
#define CONNAC2X_RX_VT_RCPI2_OFFSET           16
#define CONNAC2X_RX_VT_RCPI3_MASK             BITS(24, 31)
#define CONNAC2X_RX_VT_RCPI3_OFFSET           24

/* C-RXC Vector, 5th Cycle */
#define CONNAC2X_RX_VT_SPATIAL_REUSE1_MASK      BITS(8, 11)
#define CONNAC2X_RX_VT_SPATIAL_REUSE1_OFFSET    8
#define CONNAC2X_RX_VT_SPATIAL_REUSE2_MASK      BITS(12, 15)
#define CONNAC2X_RX_VT_SPATIAL_REUSE2_OFFSET    12
#define CONNAC2X_RX_VT_SPATIAL_REUSE3_MASK      BITS(16, 19)
#define CONNAC2X_RX_VT_SPATIAL_REUSE3_OFFSET    16
#define CONNAC2X_RX_VT_SPATIAL_REUSE4_MASK      BITS(20, 23)
#define CONNAC2X_RX_VT_SPATIAL_REUSE4_OFFSET    20

/* C-RXC Vector, 7th Cycle */
#define CONNAC2X_RX_VT_BSS_COLOR_MASK       BITS(0, 5)
#define CONNAC2X_RX_VT_BSS_COLOR_OFFSET     0
#define CONNAC2X_RX_VT_TXOP_MASK            BITS(6, 12)
#define CONNAC2X_RX_VT_TXOP_OFFSET          6
#define CONNAC2X_RX_VT_BEAM_CHANGE_MASK     BIT(13)
#define CONNAC2X_RX_VT_BEAM_CHANGE_OFFSET   13
#define CONNAC2X_RX_VT_DCM_MASK             BIT(15)
#define CONNAC2X_RX_VT_DCM_OFFSET           15
#define CONNAC2X_RX_VT_DOPPLER_MASK         BIT(16)
#define CONNAC2X_RX_VT_DOPPLER_OFFSET       16

/* Group0 */
#define HAL_MAC_CONNAC2X_RX_STATUS_IS_FCS_ERROR(_prHwMacRxStsGroup0) \
((_prHwMacRxStsGroup0->rxd_1 & RXD_FCS_ERR)?TRUE:FALSE)

#define HAL_MAC_CONNAC2X_RX_STATUS_IS_FRAG(_prHwMacRxStsGroup0) \
((_prHwMacRxStsGroup0->rxd_2 & RXD_FRAG)?TRUE:FALSE)

#define HAL_MAC_CONNAC2X_RX_STATUS_GET_RXV_SEQ_NO(_prHwMacRxStsGroup0) \
((_prHwMacRxStsGroup0->rxd_3 & RXD_RXV_SN_MASK) >> RXD_RXV_SN_SHIFT)

#define HAL_MAC_CONNAC2X_RX_STATUS_GET_CHNL_NUM(_prHwMacRxStsGroup0) \
((_prHwMacRxStsGroup0->rxd_3 & RXD_CF_MASK) >> RXD_CF_SHIFT)

/* Group3 P-B-0 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_RX_RATE(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_16 & CONNAC2X_RX_VT_RX_RATE_MASK) >> \
	CONNAC2X_RX_VT_RX_RATE_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_NSTS(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_16 & CONNAC2X_RX_VT_NSTS_MASK) >> \
	CONNAC2X_RX_VT_NSTS_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_BEAMFORMED(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_16 & CONNAC2X_RX_VT_BEAMFORMED_MASK) >> \
	CONNAC2X_RX_VT_BEAMFORMED_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_LDPC(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_16 & CONNAC2X_RX_VT_LDPC_MASK) >> \
	CONNAC2X_RX_VT_LDPC_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_RU_ALLOC1(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_16 & CONNAC2X_RX_VT_RU_ALLOC1_MASK) >> \
	CONNAC2X_RX_VT_RU_ALLOC1_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_RU_ALLOC2(_prHwMacRxStsGroup3)	\
(((_prHwMacRxStsGroup3)->rxd_17 & CONNAC2X_RX_VT_RU_ALLOC2_MASK) >> \
	CONNAC2X_RX_VT_RU_ALLOC2_OFFSET)

/* Group5 C-B-0 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_STBC(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_STBC_MASK) >> \
	CONNAC2X_RX_VT_STBC_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_NESS(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_NESS_MASK) >> \
	CONNAC2X_RX_VT_NESS_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_RX_MODE(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_RX_MODE_MASK) >> \
	CONNAC2X_RX_VT_RX_MODE_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_FR_MODE(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_FR_MODE_MASK) >> \
	CONNAC2X_RX_VT_FR_MODE_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & \
	CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED_MASK) >> \
	CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_SHORT_GI(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_SHORT_GI_MASK) >> \
	CONNAC2X_RX_VT_SHORT_GI_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_LTF(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_LTF_MASK) >> \
	CONNAC2X_RX_VT_LTF_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & \
	CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM_MASK) >> \
	CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_PE_DIS_AMB(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_PE_DIS_AMB_MASK) >> \
	CONNAC2X_RX_VT_PE_DIS_AMB_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_NUM_USER(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_NUM_USER_MASK) >> \
	CONNAC2X_RX_VT_NUM_USER_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_UL_DL(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_18 & CONNAC2X_RX_VT_UL_DL_MASK) >> \
	CONNAC2X_RX_VT_UL_DL_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU0(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_19 & CONNAC2X_RX_VT_SIGB_RU0_MASK) >> \
	CONNAC2X_RX_VT_SIGB_RU0_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU1(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_19 & CONNAC2X_RX_VT_SIGB_RU1_MASK) >> \
	CONNAC2X_RX_VT_SIGB_RU1_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU2(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_19 & CONNAC2X_RX_VT_SIGB_RU2_MASK) >> \
	CONNAC2X_RX_VT_SIGB_RU2_OFFSET)

#define HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU3(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_19 & CONNAC2X_RX_VT_SIGB_RU3_MASK) >> \
	CONNAC2X_RX_VT_SIGB_RU3_OFFSET)

/* Group5 C-B-1 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_GROUP_ID(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_20 & CONNAC2X_RX_VT_GROUP_ID_MASK) >> \
	CONNAC2X_RX_VT_GROUP_ID_OFFSET)

/* Group5 C-B-2 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_PART_AID(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_23 & CONNAC2X_RX_VT_PART_AID_MASK) >> \
	CONNAC2X_RX_VT_PART_AID_OFFSET)

/* Group5 C-B-3 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_RCPI0(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_24 & CONNAC2X_RX_VT_RCPI0_MASK) >> \
	CONNAC2X_RX_VT_RCPI0_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_RCPI1(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_24 & CONNAC2X_RX_VT_RCPI1_MASK) >> \
	CONNAC2X_RX_VT_RCPI1_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_RCPI2(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_24 & CONNAC2X_RX_VT_RCPI2_MASK) >> \
	CONNAC2X_RX_VT_RCPI0_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_RCPI3(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_24 & CONNAC2X_RX_VT_RCPI3_MASK) >> \
	CONNAC2X_RX_VT_RCPI1_OFFSET)

/* Group5 C-B-4 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE1(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_27 & \
	CONNAC2X_RX_VT_SPATIAL_REUSE1_MASK) >> \
	CONNAC2X_RX_VT_SPATIAL_REUSE1_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE2(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_27 & \
	CONNAC2X_RX_VT_SPATIAL_REUSE2_MASK) >> \
	CONNAC2X_RX_VT_SPATIAL_REUSE2_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE3(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_27 & \
	CONNAC2X_RX_VT_SPATIAL_REUSE3_MASK) >> \
	CONNAC2X_RX_VT_SPATIAL_REUSE3_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE4(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_27 & \
	CONNAC2X_RX_VT_SPATIAL_REUSE4_MASK) >> \
	CONNAC2X_RX_VT_SPATIAL_REUSE4_OFFSET)

/* Group5 C-B-6 */
#define HAL_MAC_CONNAC2X_RX_VT_GET_BSS_COLOR(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_30 & CONNAC2X_RX_VT_BSS_COLOR_MASK) >> \
	CONNAC2X_RX_VT_BSS_COLOR_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_TXOP(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_30 & CONNAC2X_RX_VT_TXOP_MASK) >> \
	CONNAC2X_RX_VT_TXOP_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_BEAM_CHANGE(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_30 & CONNAC2X_RX_VT_BEAM_CHANGE_MASK) >> \
	CONNAC2X_RX_VT_BEAM_CHANGE_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_DCM(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_30 & CONNAC2X_RX_VT_DCM_MASK) >> \
	CONNAC2X_RX_VT_DCM_OFFSET)
#define HAL_MAC_CONNAC2X_RX_VT_GET_DOPPLER(_prHwMacRxStsGroup5)	\
(((_prHwMacRxStsGroup5)->rxd_30 & CONNAC2X_RX_VT_DOPPLER_MASK) >> \
	CONNAC2X_RX_VT_DOPPLER_OFFSET)

#endif
