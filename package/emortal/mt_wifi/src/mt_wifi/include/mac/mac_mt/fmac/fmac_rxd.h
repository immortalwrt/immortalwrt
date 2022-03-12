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
#endif
