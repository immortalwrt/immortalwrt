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
	cmm_asic_mt_fmac.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"
#include "mac/mac_mt/fmac/mt_fmac.h"
#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
#ifdef MT7986
#include "chip/mt7986_cr.h"
#endif
#ifdef MT7916
#include "chip/mt7916_cr.h"
#endif
#ifdef MT7981
#include "chip/mt7981_cr.h"
#endif
#endif

#ifdef CONFIG_AP_SUPPORT
/* because of the CR arrangement in HW are not in sequence, wrape the table to search fast.*/
static UINT32 LPON_TT0SBOR_CR_MAPPING_TABLE[] = {
	LPON_TT0TPCR,
	LPON_TT0SBOR1,
	LPON_TT0SBOR2,
	LPON_TT0SBOR3,
	LPON_TT0SBOR4,
	LPON_TT0SBOR5,
	LPON_TT0SBOR6,
	LPON_TT0SBOR7,
	LPON_TT0SBOR8,
	LPON_TT0SBOR9,
	LPON_TT0SBOR10,
	LPON_TT0SBOR11,
	LPON_TT0SBOR12,
	LPON_TT0SBOR13,
	LPON_TT0SBOR14,
	LPON_TT0SBOR15,
};
#endif /*CONFIG_AP_SUPPORT*/

static BOOLEAN WtblWaitIdle(RTMP_ADAPTER *pAd, UINT32 WaitCnt, UINT32 WaitDelay)
{
	UINT32 Value = 0, CurCnt = 0;

	do {
		MAC_IO_READ32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, &Value);

		if ((Value & WTBL_IU_BUSY)  == 0)
			break;

		CurCnt++;
		RtmpusecDelay(WaitDelay);
	} while (CurCnt < WaitCnt);

	if (CurCnt == WaitCnt) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Previous update not applied by HW yet!(reg_val=0x%x)\n",
				  Value);
		return FALSE;
	}

	return TRUE;
}

VOID MtfAsicRcpiReset(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	UINT32 u4RegVal;

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	u4RegVal = (wcid | (1 << 15));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, u4RegVal);
}

VOID asic_wrap_protinfo_in_bssinfo(struct _RTMP_ADAPTER *ad, VOID *cookie)
{
	struct prot_info *prot = (struct prot_info *)cookie;

	struct wifi_dev *wdev = prot->wdev;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct _BSS_INFO_ARGUMENT_T bss;
	struct prot_info *bss_prot = &bss.prot;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	MTWF_DBG(ad, DBG_CAT_MISC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"prepare to deliver setting to firmware\n");
	os_zero_mem(&bss, sizeof(bss));
	bss.ucBssIndex = bss_info->ucBssIndex;
	memcpy(bss.Bssid, bss_info->Bssid, MAC_ADDR_LEN);
	bss.u4BssInfoFeature = BSS_INFO_PROTECT_INFO_FEATURE;

	memcpy(bss_prot, prot, sizeof(struct prot_info));

	if (arch_ops->archSetBssid)
		arch_ops->archSetBssid(ad, &bss);
	else {
		MTWF_DBG(ad, DBG_CAT_MISC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"there is no lower layer implementation.\n");
	}

	return;
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtfAsicSwitchChannel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	/* TODO: Need to fix */
	/* TODO: shiang-usw, unify the ops */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->ChipSwitchChannel)
		ops->ChipSwitchChannel(pAd, SwChCfg);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "For this chip, no specified channel switch function!\n");
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */


static VOID RxFilterCfg2Row(UINT32 FilterMask, UINT32 *RowFilterMask)
{
	*RowFilterMask = FilterMask & RX_STBC_BCN_BC_MC ?
					 (*RowFilterMask | DROP_STBC_BCN_BC_MC) : (*RowFilterMask & ~(DROP_STBC_BCN_BC_MC));
	*RowFilterMask = FilterMask & RX_FCS_ERROR ?
					 (*RowFilterMask | DROP_FCS_ERROR_FRAME) : (*RowFilterMask & ~(DROP_FCS_ERROR_FRAME));
	*RowFilterMask = FilterMask & RX_PROTOCOL_VERSION ?
					 (*RowFilterMask | DROP_VERSION_NO_0) : (*RowFilterMask & ~(DROP_VERSION_NO_0));
	*RowFilterMask = FilterMask & RX_PROB_REQ ?
					 (*RowFilterMask | DROP_PROBE_REQ) : (*RowFilterMask & ~(DROP_PROBE_REQ));
	*RowFilterMask = FilterMask & RX_MC_ALL ?
					 (*RowFilterMask | DROP_MC_FRAME) : (*RowFilterMask & ~(DROP_MC_FRAME));
	*RowFilterMask = FilterMask & RX_BC_ALL ?
					 (*RowFilterMask | DROP_BC_FRAME) : (*RowFilterMask & ~(DROP_BC_FRAME));
	*RowFilterMask = FilterMask & RX_MC_TABLE ?
					 (*RowFilterMask | DROP_NOT_IN_MC_TABLE) : (*RowFilterMask & ~(DROP_NOT_IN_MC_TABLE));
	*RowFilterMask = FilterMask & RX_BC_MC_OWN_MAC_A3 ?
					 (*RowFilterMask | DROP_ADDR3_OWN_MAC) : (*RowFilterMask & ~(DROP_ADDR3_OWN_MAC));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A3 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A3) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A3));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A2 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A2) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A2));
	*RowFilterMask = FilterMask & RX_BCN_DIFF_BSSID ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BCN) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BCN));
	*RowFilterMask = FilterMask & RX_CTRL_RSV ?
					 (*RowFilterMask | DROP_CTRL_RSV) : (*RowFilterMask & ~(DROP_CTRL_RSV));
	*RowFilterMask = FilterMask & RX_CTS ?
					 (*RowFilterMask | DROP_CTS) : (*RowFilterMask & ~(DROP_CTS));
	*RowFilterMask = FilterMask & RX_RTS ?
					 (*RowFilterMask | DROP_RTS) : (*RowFilterMask & ~(DROP_RTS));
	*RowFilterMask = FilterMask & RX_DUPLICATE ?
					 (*RowFilterMask | DROP_DUPLICATE) : (*RowFilterMask & ~(DROP_DUPLICATE));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BSSID ?
					 (*RowFilterMask | DROP_NOT_MY_BSSID) : (*RowFilterMask & ~(DROP_NOT_MY_BSSID));
	*RowFilterMask = FilterMask & RX_NOT_OWN_UCAST ?
					 (*RowFilterMask | DROP_NOT_UC2ME) : (*RowFilterMask & ~(DROP_NOT_UC2ME));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BTIM ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BTIM) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BTIM));
	*RowFilterMask = FilterMask & RX_NDPA ?
					 (*RowFilterMask | DROP_NDPA) : (*RowFilterMask & ~(DROP_NDPA));
}


INT MtfAsicSetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter)
{
	UINT32 Value = 0;

	if (RxFilter.bPromiscuous) {
#ifdef SNIFFER_MT7615

		if (IS_MT7615(pAd))
			Value = DROP_FCS_ERROR_FRAME | RM_FRAME_REPORT_EN | RX_UNWANTED_CTL_FRM;
		else
#endif
			Value = RX_PROMISCUOUS_MODE;
	} else {
		/*disable frame report & monitor mode*/
		Value &= ~RX_PROMISCUOUS_MODE;

		if (RxFilter.bFrameReport)
			Value |= RM_FRAME_REPORT_EN;
		else
			Value &=  ~(RM_FRAME_REPORT_EN);

		/*enable drop filter by RxfilterMask*/
		RxFilterCfg2Row(RxFilter.filterMask, &Value);
	}

	if (RxFilter.u1BandIdx)
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_1, Value);
	else
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_0, Value);

	return TRUE;
}

#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2
static INT SetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
	/* UINT32 mask, time_mask; */
	/* UINT32 Value; */
	return 0;
}

INT MtfAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return SetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}

#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
INT MtfAsicGetTsfTimeByDriver(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	UINT32 Value = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR, &Value);
	Value = (Value & BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_HW_MODE_MASK) |
			BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_SW_MODE_MASK;/* keep HW mode value. */
	MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + HwBssidIdx * 4, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR0_ADDR, low_part);
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR1_ADDR, high_part);

	return TRUE;
}

#if defined(MT7986) || defined(MT7916) || defined(MT7981)
INT MtfAsicGetTsfTimeByDriver_FMAC(RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part, UCHAR HwBssidIdx)
{
	UINT32 Value = 0;
	UINT8 band_idx = 0;
	struct wifi_dev *wdev = NULL;
	UINT32 i = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HwBssidIdx)
				break;
		} else
			continue;
	}

	if (i >= WDEV_NUM_MAX)
		return FALSE;

	band_idx = HcGetBandByWdev(wdev);
	if (pAd->CommonCfg.dbdc_mode) {
		if (HwBssidIdx == 0 || band_idx == DBDC_BAND0) {
			MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), &Value);
			Value = (Value & BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_HW_MODE_MASK) |
					BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_SW_MODE_MASK;/* keep HW mode value. */
			MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), Value);
			MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR0_ADDR, low_part);
			MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR1_ADDR, high_part);
		} else {
			MAC_IO_READ32(pAd->hdev_ctrl, BN1_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), &Value);
			Value = (Value & BN1_WF_LPON_TOP_T0CR_TSF0_TIMER_HW_MODE_MASK) |
					BN1_WF_LPON_TOP_T0CR_TSF0_TIMER_SW_MODE_MASK;/* keep HW mode value. */
			MAC_IO_WRITE32(pAd->hdev_ctrl, BN1_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), Value);
			MAC_IO_READ32(pAd->hdev_ctrl, BN1_WF_LPON_TOP_UTTR0_ADDR, low_part);
			MAC_IO_READ32(pAd->hdev_ctrl, BN1_WF_LPON_TOP_UTTR1_ADDR, high_part);
		}
	} else {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), &Value);
		Value = (Value & BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_HW_MODE_MASK) |
				BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_SW_MODE_MASK;/* keep HW mode value. */
		MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + (HwBssidIdx << 6), Value);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR0_ADDR, low_part);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR1_ADDR, high_part);
	}
	return TRUE;
}
#endif
#endif

typedef struct _SYNC_MODE_CR_TABLE_T {
	UINT32              u4ArbOpModeCR;
	UINT32              u4ArbBcnWmmCR;
	UINT32              u4LponMacTimerCr;
	UINT32              u4LponTbttCtrlCR;
	UINT32              u4LponPreTbttTime;/* set pretbtt time */
	UINT32              u4LponSyncModeCR;/* sync mode CR*/
	UINT32              u4IntEnableCR;
} SYNC_MODE_CR_TABLE_T, *PSYNC_MODE_CR_TABLE_T;

typedef struct _RTMP_WMM_PAIR {
	UINT32 Address;
	UINT32 Mask;
	UINT32 Shift;
} RTMP_WMM_PAIR, *PRTMP_WMM_PAIR;


static RTMP_WMM_PAIR wmm_txop_mask[] = {
	{TMAC_ACTXOPLR1, 0x0000ffff, 0}, /* AC0 - BK */
	{TMAC_ACTXOPLR1, 0xffff0000, 16}, /* AC1 - BE */
	{TMAC_ACTXOPLR0, 0x0000ffff, 0}, /* AC2 - VI */
	{TMAC_ACTXOPLR0, 0xffff0000, 16}, /* AC3 - VO */
};


static RTMP_WMM_PAIR wmm_aifsn_mask[] = {
	{ARB_WMMAC00, 0x0000000f, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x0000000f, 4}, /* AC1 - BE */
	{ARB_WMMAC02, 0x0000000f, 8}, /* AC2  - VI */
	{ARB_WMMAC03, 0x0000000f, 12}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmin_mask[] = {
	{ARB_WMMAC00, 0x00001f00, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x00001f00, 8}, /* AC1 - BE */
	{ARB_WMMAC02, 0x00001f00, 16}, /* AC2  - VI */
	{ARB_WMMAC03, 0x00001f00, 24}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmax_mask[] = {
	{ARB_WMMAC00, 0x001f0000, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x001f0000, 16}, /* AC1 - BE */
	{ARB_WMMAC02, 0x001f0000, 0}, /* AC2  - VI */
	{ARB_WMMAC03, 0x001f0000, 16}, /* AC3 - VO */
};


UINT32 MtfAsicGetWmmParam(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 addr = 0, cr_val = 0, mask = 0, shift = 0;

	if (AcNum <= WMM_PARAM_AC_3) {
		switch (EdcaType) {
		case WMM_PARAM_TXOP:
			addr = wmm_txop_mask[AcNum].Address;
			mask = wmm_txop_mask[AcNum].Mask;
			shift = wmm_txop_mask[AcNum].Shift;
			break;

		case WMM_PARAM_AIFSN:
			addr = wmm_aifsn_mask[AcNum].Address;
			mask = wmm_aifsn_mask[AcNum].Mask;
			shift = wmm_aifsn_mask[AcNum].Shift;
			break;

		case WMM_PARAM_CWMIN:
			addr = wmm_cwmin_mask[AcNum].Address;
			mask = wmm_cwmin_mask[AcNum].Mask;
			shift = wmm_cwmin_mask[AcNum].Shift;
			break;

		case WMM_PARAM_CWMAX:
			addr = wmm_cwmax_mask[AcNum].Address;
			mask = wmm_cwmax_mask[AcNum].Mask;
			shift = wmm_cwmax_mask[AcNum].Shift;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error type=%d\n", EdcaType);
			break;
		}
	}

	if (addr && mask) {
		MAC_IO_READ32(pAd->hdev_ctrl, addr, &cr_val);
		cr_val = (cr_val & mask) >> shift;
		return cr_val;
	}

	return 0xdeadbeef;
}

VOID MtfAsicTxCapAndRateTableUpdate(
	RTMP_ADAPTER *pAd,
	UINT16 u2Wcid,
	RA_PHY_CFG_T *prTxPhyCfg,
	UINT32 *Rate,
	BOOL fgSpeEn)
{
	struct wtbl_entry tb_entry;
	union WTBL_DW3 wtbl_wd3;
	union WTBL_DW5 wtbl_wd5;
	UINT32 u4RegVal;
	UCHAR bw;

	if (WtblWaitIdle(pAd, 100, 50) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WaitIdle failed\n");
		return;
	}

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

	if (asic_get_wtbl_entry234(pAd, u2Wcid, &tb_entry) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cannot found WTBL2/3/4 for WCID(%d)\n",
				 u2Wcid);
		return;
	}

	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 12, &(wtbl_wd3.word));

	if (fgSpeEn)
		wtbl_wd3.field.spe_idx = 24;
	else
		wtbl_wd3.field.spe_idx = 0;

	MAC_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 12, wtbl_wd3.word);
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 20, &(wtbl_wd5.word));

	if (prTxPhyCfg->ldpc & HT_LDPC)
		wtbl_wd5.field.ldpc = 1;
	else
		wtbl_wd5.field.ldpc = 0;

	if (prTxPhyCfg->ldpc & VHT_LDPC)
		wtbl_wd5.field.ldpc_vht = 1;
	else
		wtbl_wd5.field.ldpc_vht = 0;

	switch (prTxPhyCfg->BW) {
	case 4:
	case 3:
		bw = 3;
		break;

	case BW_80:
		bw = 2;
		break;

	case BW_40:
		bw = 1;
		break;

	case BW_20:

	/* case BW_10: */
	default:
		bw = 0;
		break;
	}

	wtbl_wd5.field.fcap = bw;
	wtbl_wd5.field.cbrn = 7; /* change bw as (fcap/2) if rate_idx > 7, temporary code */

	if (prTxPhyCfg->ShortGI) {
		wtbl_wd5.field.g2 = 1;
		wtbl_wd5.field.g4 = 1;
		wtbl_wd5.field.g8 = 1;
		wtbl_wd5.field.g16 = 1;
	} else {
		wtbl_wd5.field.g2 = 0;
		wtbl_wd5.field.g4 = 0;
		wtbl_wd5.field.g8 = 0;
		wtbl_wd5.field.g16 = 0;
	}

	wtbl_wd5.field.rate_idx = 0;
	wtbl_wd5.field.txpwr_offset = 0;
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RIUCR0, wtbl_wd5.word);
	u4RegVal = (Rate[0] | (Rate[1] << 12) | (Rate[2] << 24));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RIUCR1, u4RegVal);
	u4RegVal = ((Rate[2] >> 8) | (Rate[3] << 4) | (Rate[4] << 16) | (Rate[5] << 28));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RIUCR2, u4RegVal);
	u4RegVal = ((Rate[5] >> 4) | (Rate[6] << 8) | (Rate[7] << 20));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RIUCR3, u4RegVal);
	/* TODO: shiang-MT7615, shall we also clear TxCnt/RxCnt/AdmCnt here?? */
	u4RegVal = (u2Wcid | (1 << 13) | (1 << 14));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, u4RegVal);
}


/**
 * Wtbl2TxRateCounterGet
 *
 *
 *
 */
static VOID WtblTxRateCounterGet(RTMP_ADAPTER *pAd, UINT16 u2Wcid, DMAC_TX_CNT_INFO *tx_cnt_info)
{
	struct wtbl_entry tb_entry;
	UINT32 u4RegVal;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

	if (asic_get_wtbl_entry234(pAd, u2Wcid, &tb_entry) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cannot found WTBL2/3/4 for WCID(%d)\n",
				 u2Wcid);
		return;
	}

	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 56, &(tx_cnt_info->wtbl_wd14.word));
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 60, &(tx_cnt_info->wtbl_wd15.word));
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 64, &(tx_cnt_info->wtbl_wd16.word));
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 68, &(tx_cnt_info->wtbl_wd17.word));

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	u4RegVal = (u2Wcid | (1 << 14));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, u4RegVal);
}

VOID MtfAsicTxCntUpdate(RTMP_ADAPTER *pAd, UINT16 Wcid, MT_TX_COUNTER *pTxInfo)
{
	DMAC_TX_CNT_INFO tx_cnt_info;
	NdisZeroMemory(&tx_cnt_info, sizeof(DMAC_TX_CNT_INFO));

	WtblTxRateCounterGet(pAd, Wcid, &tx_cnt_info);
	pTxInfo->TxCount = tx_cnt_info.wtbl_wd16.field.current_bw_tx_cnt;
	pTxInfo->TxCount += tx_cnt_info.wtbl_wd17.field.other_bw_tx_cnt;
	pTxInfo->TxFailCount = tx_cnt_info.wtbl_wd16.field.current_bw_fail_cnt;
	pTxInfo->TxFailCount += tx_cnt_info.wtbl_wd17.field.other_bw_fail_cnt;
	pTxInfo->Rate1TxCnt = (UINT16)tx_cnt_info.wtbl_wd14.field.rate_1_tx_cnt;
	pTxInfo->Rate1FailCnt = (UINT16)tx_cnt_info.wtbl_wd14.field.rate_1_fail_cnt;
	pTxInfo->Rate2OkCnt = (UINT16)tx_cnt_info.wtbl_wd15.field.rate_2_ok_cnt;
	pTxInfo->Rate3OkCnt = (UINT16)tx_cnt_info.wtbl_wd15.field.rate_3_ok_cnt;
}

#define SN_MASK 0xfff

#ifdef STREAM_MODE_SUPPORT
UINT32 MtStreamModeRegVal(RTMP_ADAPTER *pAd)
{
	return 0x0;
}


/*
 * ========================================================================
 * Description:
 * configure the stream mode of specific MAC or all MAC and set to ASIC.
 *
 * Prameters:
 * pAd   ---
 * pMacAddr ---
 * bClear        --- disable the stream mode for specific macAddr when
 * (pMacAddr!=NULL)
 *
 * Return:
 * ========================================================================
 */
VOID MtAsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	/* TODO: shiang-7603 */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support for HIF_MT yet!\n");
}


VOID MtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support for HIF_MT yet!\n");
}
#endif /* STREAM_MODE_SUPPORT // */


#ifdef DOT11_N_SUPPORT


#endif /* DOT11_N_SUPPORT // */


static UINT32 reg_wmm_cr[4] = {ARB_TQSW0, ARB_TQSW1, ARB_TQSW2, ARB_TQSW3};
static UINT32 reg_mgmt_cr[2] = {ARB_TQSM0, ARB_TQSM1};

static INT32 MtAsicSetTxQ(RTMP_ADAPTER *pAd, INT WmmSet, INT BandIdx, BOOLEAN Enable)
{
	UINT32 reg_w = 0, mask_w = 0, val_w = 0;
	UINT32 reg_mgmt = 0, mask_m = 0, val_m = 0;

	if ((BandIdx < 2) && (WmmSet < 4)) {
		reg_mgmt = reg_mgmt_cr[BandIdx];
		mask_w = 0x0f0f0f0f;
		reg_w = reg_wmm_cr[WmmSet];
		mask_m = 0x0f000f0f;
		MAC_IO_READ32(pAd->hdev_ctrl, reg_w, &val_w);
		val_w = ((Enable) ? (val_w | mask_w) : (val_w & (~mask_w)));
		MAC_IO_WRITE32(pAd->hdev_ctrl, reg_w, val_w);
		MAC_IO_READ32(pAd->hdev_ctrl, reg_mgmt, &val_m);
		val_m = ((Enable) ? (val_m | mask_m) : (val_m & (~mask_m)));
		MAC_IO_WRITE32(pAd->hdev_ctrl, reg_mgmt, val_m);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "Set WmmSet=%d, band=%d, Enable=%d with CR[0x%x = 0x%08x, 0x%x=0x%08x]\n",
				  WmmSet, BandIdx, Enable, reg_w, val_w, reg_mgmt, val_m);
		return TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Invalid Input paramter!WmmSet=%d, BssIdx=%d, band=%d, Enable=%d\n",
			  WmmSet, BandIdx, BandIdx, Enable);
	return FALSE;
}

INT32 MtfAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 Value = 0, Value2 = 0;
	UINT32 i;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MAC_IO_READ32(pAd->hdev_ctrl, ARB_SCR, &Value);
	MAC_IO_READ32(pAd->hdev_ctrl, ARB_RQCR, &Value2);

	switch (TxRx) {
	case ASIC_MAC_TX:
		if (Enable)
			Value &= (BandIdx) ? (~MT_ARB_SCR_TX1DIS) : (~MT_ARB_SCR_TXDIS);
		else
			Value |= (BandIdx) ? MT_ARB_SCR_TX1DIS : MT_ARB_SCR_TXDIS;

		break;

	case ASIC_MAC_RX:
		if (Enable) {
			Value &= (BandIdx) ?  (~MT_ARB_SCR_RX1DIS) : (~MT_ARB_SCR_RXDIS);
			Value2 |= (BandIdx) ? ARB_RQCR_RX1_START : ARB_RQCR_RX_START;
		} else {
			Value |= (BandIdx) ? MT_ARB_SCR_RX1DIS : MT_ARB_SCR_RXDIS;
			Value2 &= (BandIdx) ?  (~ARB_RQCR_RX1_START) : (~ARB_RQCR_RX_START);
		}

		break;

	case ASIC_MAC_TXRX:
		if (Enable) {
			Value &= (BandIdx) ? (~(MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS)) : (~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS));
			Value2 |= (BandIdx) ? ARB_RQCR_RX1_START : ARB_RQCR_RX_START;
		} else {
			Value |= (BandIdx) ? (MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS) : (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value2 &= (BandIdx) ?  (~ARB_RQCR_RX1_START) : (~ARB_RQCR_RX_START);
		}

		break;

	case ASIC_MAC_TXRX_RXV:
		if (Enable) {
			Value &= (BandIdx) ?  (~(MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS)) : (~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS));
			Value2 |= (BandIdx) ?
					  ((ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN)) :
					  ((ARB_RQCR_RX_START | ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN));
		} else {
			Value |= (BandIdx) ? (MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS) : (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
			Value2 &= (BandIdx) ?
					  ~(ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
					  ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		}

		break;

	case ASIC_MAC_RXV:
		if (Enable) {
			Value &= (BandIdx) ? ~MT_ARB_SCR_RX1DIS : ~MT_ARB_SCR_RXDIS;
			Value2 |= (BandIdx) ?
					  (ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
					  (ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		} else {
			Value2 &= (BandIdx) ?
					  ~(ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
					  ~(ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV1_T_EN);
		}

		break;

	case ASIC_MAC_RX_RXV:
		if (Enable) {
			Value &= (BandIdx) ? ~MT_ARB_SCR_RX1DIS : ~MT_ARB_SCR_RXDIS;
			Value2 |= (BandIdx) ?
					  (ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
					  (ARB_RQCR_RX_START | ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		} else {
			Value |= (BandIdx) ?  MT_ARB_SCR_RX1DIS : MT_ARB_SCR_RXDIS;
			Value2 &= (BandIdx) ?
					  ~(ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START | ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
					  ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
		}

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown path (%d)\n", TxRx);
		break;
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_SCR, Value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_RQCR, Value2);

	/*Set TX Queue*/
	for (i = 0; i < cap->qos.WmmHwNum; i++)
		MtAsicSetTxQ(pAd, i, BandIdx, Enable);

	return TRUE;
}

INT MtfAsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 Value = 0;
	UINT32 Reg;

	Reg  = (BandIdx) ? TMAC_TCR1 : TMAC_TCR;
	MAC_IO_READ32(pAd->hdev_ctrl, Reg, &Value);
	Value &= ~TMAC_TCR_TX_STREAM_NUM_MASK;
	Value |= TMAC_TCR_TX_STREAM_NUM(StreamNums - 1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, Value);
	return TRUE;
}

INT MtfAsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
	UINT32 val = 0, offset;
#ifndef COMPOS_WIN
	/* TODO: shiang-usw, some CR setting in bbp_set_bw() need to take care!! */
	bbp_set_bw(pAd, bw, BandIdx);
#endif /* COMPOS_WIN */
	offset = (BandIdx == 0) ? 2 : 18;
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_BWCR, &val);
	val &= ~(3 << offset);

	switch (bw) {
	case BW_20:
		val |= (0 << offset);
		break;

	case BW_40:
		val |= (0x1 << offset);
		break;

	case BW_80:
		val |= (0x2 << offset);
		break;

	case BW_160:
	case BW_8080:
		val |= (0x3 << offset);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid BW(%d)!\n", bw);
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_BWCR, val);
	return TRUE;
}

UINT32 MtfAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	UINT32 value = 0, temp = 0;

	if (IS_MT7663(pAd)) {
		switch (type) {
		case HQA_RX_STAT_MACFCSERRCNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
			value = value & 0xFFFF; /* [15:0] FCS ERR */
			break;

		case HQA_RX_STAT_MAC_MDRDYCNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
			value = value & 0x3FFFFFF; /* [15:0] Mac Mdrdy*/
			break;

		case HQA_RX_STAT_MAC_RXLENMISMATCH:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR11, &value);
			value = value & 0xFFFF;
			break;

		case HQA_RX_STAT_MAC_FCS_OK_COUNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR51, &value);
			value = value & 0xFFFF;
			break;

		case HQA_RX_STAT_PHY_MDRDYCNT:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_RO_BAND0_PHYCTRL_STS1, &value);
			break;

		case HQA_RX_STAT_PHY_FCSERRCNT:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_RO_BAND0_PHYCTRL_STS5, &value);
			break;

		case HQA_RX_STAT_PD:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_RO_BAND0_PHYCTRL_STS0, &value);
			break;

		case HQA_RX_STAT_CCK_SIG_SFD:
			/* [31:16] SIG [15:0] SFD */
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_RO_BAND0_PHYCTRL_STS3, &value);
			break;

		case HQA_RX_STAT_OFDM_SIG_TAG:
			/* [31:16] SIG [15:0] TAG */
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_RO_BAND0_PHYCTRL_STS4, &value);
			break;

		case HQA_RX_STAT_RSSI:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX0_ADDR, &temp);
			/* WF0 IB RSSI */
			value |= ((temp & 0xFF000000) >> 16);
			/* WF0 WB RSSI */
			value |= ((temp & 0x00FF0000) >> 16);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX1_ADDR, &temp);
			/* WF1 IB RSSI */
			value |= (temp & 0xFF000000);
			/* WF1 WB RSSI */
			value |= (temp & 0x00FF0000);
			break;

		case  HQA_RX_STAT_RSSI_RX23:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX2_ADDR, &value);
			/* WF2 IB RSSI */
			value |= ((temp & 0xFF000000) >> 16);
			/* WF2 WB RSSI */
			value |= ((temp & 0x00FF0000) >> 16);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX3_ADDR, &value);
			/* WF3 IB RSSI */
			value |= (temp & 0xFF000000);
			/* WF3 WB RSSI */
			value |= (temp & 0x00FF0000);
			break;

		case HQA_RX_STAT_ACI_HITL:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX0_ADDR, &value);
			/* WF0 ACID HIT */
			value |= ((temp & 0x00020000) << 1);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX1_ADDR, &value);
			/* WF1 ACID HIT */
			value |= (temp & 0x00020000);
			break;

		case HQA_RX_STAT_ACI_HITH:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX2_ADDR, &value);
			/* WF2 ACID HIT */
			value |= ((temp & 0x00020000) << 1);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX3_ADDR, &value);
			/* WF3 ACID HIT */
			value |= (temp & 0x00020000);
			break;

		case HQA_RX_FIFO_FULL_COUNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &value);
			value = (value >> 16) & 0xffff;
			break;

		case HQA_RX_FIFO_FULL_COUNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR4, &value);
			value = (value >> 16) & 0xffff;
			break;

		case HQA_RX_RESET_PHY_COUNT:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_CR_BAND0_STSCNT_EN_CTRL, &value);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_CR_BAND1_STSCNT_EN_CTRL, &value);
			break;

		case HQA_RX_STAT_MACFCSERRCNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
			break;

		case HQA_RX_STAT_MAC_MDRDYCNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
			break;

		case HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR11, &value);
			break;

		case HQA_RX_RESET_MAC_COUNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
			break;

		case HQA_RX_STAT_PHY_MDRDYCNT_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS5, &value);
			break;

		case HQA_RX_STAT_PHY_FCSERRCNT_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS4, &value);
			break;

		case HQA_RX_STAT_PD_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS0, &value);
			break;

		case HQA_RX_STAT_CCK_SIG_SFD_BAND1:
			/* [31:16] SIG [15:0] SFD */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS1, &value);
			break;

		case HQA_RX_STAT_OFDM_SIG_TAG_BAND1:
			/* [31:16] SIG [15:0] TAG */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, &value);
			break;

		default:
			break;
		}
	} else {
		switch (type) {
		case HQA_RX_STAT_MACFCSERRCNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
			value = value & 0xFFFF; /* [15:0] FCS ERR */
			break;

		case HQA_RX_STAT_MAC_MDRDYCNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
			value = value & 0x3FFFFFF; /* [15:0] Mac Mdrdy*/
			break;

		case HQA_RX_STAT_MAC_RXLENMISMATCH:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR11, &value);
			value = value & 0xFFFF;
			break;


		case HQA_RX_STAT_PHY_MDRDYCNT:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &value);
			break;

		case HQA_RX_STAT_PHY_FCSERRCNT:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS4, &value);
			break;

		case HQA_RX_STAT_PD:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0, &value);
			break;

		case HQA_RX_STAT_CCK_SIG_SFD:
			/* [31:16] SIG [15:0] SFD */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS1, &value);
			break;

		case HQA_RX_STAT_OFDM_SIG_TAG:
			/* [31:16] SIG [15:0] TAG */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS2, &value);
			break;

		case HQA_RX_STAT_RSSI:
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &value);
			break;

		case  HQA_RX_STAT_RSSI_RX23:
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &value);
			break;

		case HQA_RX_STAT_ACI_HITL:
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_4, &value);
			break;

		case HQA_RX_STAT_ACI_HITH:
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_4, &value);
			break;

		case HQA_RX_FIFO_FULL_COUNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &value);
			value = (value >> 16) & 0xffff;
			break;

		case HQA_RX_FIFO_FULL_COUNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR4, &value);
			value = (value >> 16) & 0xffff;
			break;

		case HQA_RX_RESET_PHY_COUNT:
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &value);
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS4, &value);
			break;

		case HQA_RX_STAT_MACFCSERRCNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
			break;

		case HQA_RX_STAT_MAC_MDRDYCNT_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
			break;

		case HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR11, &value);
			break;

		case HQA_RX_RESET_MAC_COUNT:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
			break;

		case HQA_RX_STAT_PHY_MDRDYCNT_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS5, &value);
			break;

		case HQA_RX_STAT_PHY_FCSERRCNT_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS4, &value);
			break;

		case HQA_RX_STAT_PD_BAND1:
			/* [31:16] OFDM [15:0] CCK */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS0, &value);
			break;

		case HQA_RX_STAT_CCK_SIG_SFD_BAND1:
			/* [31:16] SIG [15:0] SFD */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS1, &value);
			break;

		case HQA_RX_STAT_OFDM_SIG_TAG_BAND1:
			/* [31:16] SIG [15:0] TAG */
			PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, &value);
			break;

		default:
			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Type(%d):%x\n", type, value);
	return value;
}

#ifdef CONFIG_ATE
INT MtAsicSetRfFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not finish Yet!\n");
	return 0;
}


INT MtAsicSetTSSI(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	UINT32 WF0Offset = 0x10D04; /* WF_PHY_CR_FRONT CR_WF0_TSSI_1 */
	UINT32 WF1Offset = 0x11D04; /* WF_PHY_CR_FRONT CR_WF1_TSSI_1 */
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE)
		CRValue = 0xE3F3F800;
	else
		CRValue = 0xE1010800;

	if ((WFSelect == 0) || (WFSelect == 2)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set WF#%d TSSI off\n", WFSelect);
		PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd->hdev_ctrl, 0x10D18, 0x0);
		}
	}

	if ((WFSelect == 1) || (WFSelect == 2)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set WF#%d TSSI on\n", WFSelect);
		PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd->hdev_ctrl, 0x11D18, 0x0);
		}
	}

	return Ret;
}


INT MtAsicSetDPD(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	ULONG WF0Offset = 0x10A08;
	ULONG WF1Offset = 0x11A08;
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE) {
		/* WF0 */
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF0Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);
		}

		/* WF1 */
		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF1Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set WFSelect: %d DPD off\n", WFSelect);
	} else {
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF0Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);
		}

		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF1Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set WFSelect: %d DPD on\n", WFSelect);
	}

	return Ret;
}



#ifdef COMPOS_TESTMODE_WIN
/**
 * Tx Set Frequency Offset
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param iOffset value
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxSetFrequencyOffset(RTMP_ADAPTER *pAd, UINT32 iOffset, BOOLEAN HasBeenSet)
{
	UINT32 Value = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "iOffset=0x%x-->\n", iOffset);

	if (HasBeenSet == FALSE) {
		if (IS_MT7603(pAd)) {
			/* RG_XO_C2[8:14]  Set 60 (0x3C )at first */
			MAC_IO_READ32(pAd->hdev_ctrl, RG_XO_C2, &Value);
			Value = (Value & 0xFFFF80FF) | (0x3C << 8);
			MAC_IO_WRITE32(pAd->hdev_ctrl, RG_XO_C2, Value);
			/* RG_XO_C2_MANUAL [8:14]  Set 0x7F at first */
			MAC_IO_READ32(pAd->hdev_ctrl, RG_XO_C2_MANUAL, &Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd->hdev_ctrl, RG_XO_C2_MANUAL, Value);
			/* only set at first time */
		} else if (IS_MT76x6(pAd)) {
			MAC_IO_READ32(pAd->hdev_ctrl, FREQ_OFFSET_MANUAL_ENABLE, &Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd->hdev_ctrl, FREQ_OFFSET_MANUAL_ENABLE, Value);
		}
	}

	if (IS_MT7603(pAd)) {
		HW_IO_READ32(pAd->hdev_ctrl, RG_XO_C2, &Value);
		Value = (Value & 0xFFFF80FF) | (iOffset << 8);
		HW_IO_WRITE32(pAd->hdev_ctrl, RG_XO_C2, Value);
	}

	return 0;
}
/**
 * Set Tx Power Range
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param ucMaxPowerDbm, Max Power Dbm
 * @param ucMinPowerDbm, Min Power Dbm
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxConfigPowerRange(RTMP_ADAPTER *pAd, IN UCHAR ucMaxPowerDbm, IN UCHAR ucMinPowerDbm)
{
	UINT32 u4RegValue;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_FPCR, &u4RegValue);
	u4RegValue &= ~(FPCR_FRAME_POWER_MAX_DBM | FPCR_FRAME_POWER_MIN_DBM);
	u4RegValue |= ((ucMaxPowerDbm << FPCR_FRAME_POWER_MAX_DBM_OFFSET) & FPCR_FRAME_POWER_MAX_DBM);
	u4RegValue |= ((ucMinPowerDbm << FPCR_FRAME_POWER_MIN_DBM_OFFSET) & FPCR_FRAME_POWER_MIN_DBM);
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_FPCR, u4RegValue);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return 0;
}

INT MTAsicSetTMR(RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	UINT32 value = 0;
	UINT32 Reg = (BandIdx) ? RMAC_TMR_PA_BAND_1 : RMAC_TMR_PA;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	switch (enable) {
	case 1: { /* initialiter */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "enable TMR report, as Initialiter\n");
		MAC_IO_READ32(pAd->hdev_ctrl, Reg, &value);
		value = value | BIT31;
		value = value & ~BIT30;
		value = value | 0x34;/* Action frame register */
		MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, value);
	}
	break;

	case 2: { /* responder */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "enable TMR report, as Responser\n");
		MAC_IO_READ32(pAd->hdev_ctrl, Reg, &value);
		value = value | BIT31;
		value = value | BIT30;
		value = value | 0x34;/* Action frame register */
		MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, value);
	}
	break;

	case 0:/* disable */
	default: {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "disable TMR report\n");
		MAC_IO_READ32(pAd->hdev_ctrl, Reg, &value);
		value = value & ~BIT31;
		MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, value);
	}
	}

	return 0;
}
#endif
#endif /* CONFIG_ATE */

#ifndef MAC_INIT_OFFLOAD
VOID MtAsicSetRxGroup(RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable)
{
	UINT32 Value;

	MCU_IO_READ32(pAd->hdev_ctrl, RXINF, &Value);

	if (Enable) {
		if (Group & RXS_GROUP1)
			Value |= RXSH_GROUP1_EN;

		if (Group & RXS_GROUP2)
			Value |= RXSH_GROUP2_EN;

		if (Group & RXS_GROUP3)
			Value |= RXSH_GROUP3_EN;
	} else {
		if (Group & RXS_GROUP1)
			Value &= ~RXSH_GROUP1_EN;

		if (Group & RXS_GROUP2)
			Value &= ~RXSH_GROUP2_EN;

		if (Group & RXS_GROUP3)
			Value &= ~RXSH_GROUP3_EN;
	}

	MCU_IO_WRITE32(pAd->hdev_ctrl, RXINF, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_DCR1, &Value);

	if (Enable) {
		if (Group & RXS_GROUP1)
			Value |= RXSM_GROUP1_EN;

		if (Group & RXS_GROUP2)
			Value |= RXSM_GROUP2_EN;

		if (Group & RXS_GROUP3)
			Value |= RXSM_GROUP3_EN;
	} else {
		if (Group & RXS_GROUP1)
			Value &= ~RXSM_GROUP1_EN;

		if (Group & RXS_GROUP2)
			Value &= ~RXSM_GROUP2_EN;

		if (Group & RXS_GROUP3)
			Value &= ~RXSM_GROUP3_EN;
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_DCR1, Value);
}



INT MtAsicSetBAWinSizeRange(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT MtAsicSetBARTxRate(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;
	/* TODO: shiang-MT7615, document mismatch!! */
	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_ACR0, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ACR0, mac_val);
	return TRUE;
}


VOID MtAsicSetBARTxCntLimit(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count)
{
	UINT32 Value;
	/* TODO: check for RTY_MODE!! */
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_MRCR, &Value);

	if (Enable) {
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(Count);
	} else {
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(0);
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_MRCR, Value);
}


VOID MtAsicSetTxSClassifyFilter(RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ,
								UINT32 AggNums, UINT32 Filter, UCHAR BandIdx)
{
	UINT32 Value;
	UINT32 Reg;

	if (Port == TXS2HOST) {
		Reg = (BandIdx) ? DMA_BN1TCFR1 : DMA_BN0TCFR1;
		MAC_IO_READ32(pAd->hdev_ctrl, Reg, &Value);
		Value &= ~TXS2H_BIT_MAP_MASK;
		Value |= TXS2H_BIT_MAP(Filter);
		Value &= ~TXS2H_AGG_CNT_MASK;
		Value |= TXS2H_AGG_CNT(AggNums);

		if (DestQ == 0)
			Value &= ~TXS2H_QID;
		else
			Value |= TXS2H_QID;

		MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, Value);
	} else if (Port == TXS2MCU) {
		Reg = (BandIdx) ? DMA_BN1TCFR0 : DMA_BN0TCFR0;
		MAC_IO_READ32(pAd->hdev_ctrl, Reg, &Value);
		Value &= ~TXS2M_BIT_MAP_MASK;
		Value |= TXS2M_BIT_MAP(Filter);
		Value &= ~TXS2M_AGG_CNT_MASK;
		Value |= TXS2M_AGG_CNT(AggNums);
		Value &= ~TXS2M_QID_MASK;
		Value |= TXS2M_QID(DestQ);
		MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, Value);
	} else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown Port(%d)\n", Port);
}
#endif /* MAC_INIT_OFFLOAD */

INT32 MtfAsicGetFwSyncValue(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->get_fw_sync_value)
			return ops->get_fw_sync_value(pAd);

	return FALSE;
}


VOID MtfAsicInitMac(RTMP_ADAPTER *pAd)
{
#ifndef BCN_OFFLOAD_SUPPORT
	UINT32 mac_val;

	MAC_IO_READ32(pAd->hdev_ctrl, PLE_RELEASE_CTRL, &mac_val);
	mac_val = mac_val |
			  SET_BCN0_RLS_QID(UMAC_PLE_CTRL_P3_Q_0X1F) |
			  SET_BCN0_RLS_PID(P_IDX_PLE_CTRL_PSE_PORT_3) |
			  SET_BCN1_RLS_QID(UMAC_PLE_CTRL_P3_Q_0X1F) |
			  SET_BCN1_RLS_PID(P_IDX_PLE_CTRL_PSE_PORT_3);
	MAC_IO_WRITE32(pAd->hdev_ctrl, PLE_RELEASE_CTRL, mac_val);
#endif
#ifndef MAC_INIT_OFFLOAD
	UINT32 mac_val, mac_val_bnd[2];

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "()-->\n");
	MtAsicSetBAWinSizeRange(pAd);
	/* TMR report queue setting */
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN0TMCFR0, &mac_val_bnd[0]);
	mac_val_bnd[0] |= BIT13;/* TMR report send to HIF q1. */
	mac_val_bnd[0] &= (~(BIT0));
	mac_val_bnd[0] &= (~(BIT1));
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0TMCFR0, mac_val_bnd[0]);
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_TMR_PA, &mac_val);
	mac_val = mac_val & ~BIT31;
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_TMR_PA, mac_val);
#ifdef DBDC_MODE
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN1TMCFR0, &mac_val_bnd[1]);
	mac_val_bnd[1] = mac_val_bnd[0];
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN1TMCFR0, mac_val_bnd[1]);
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_TMR_PA_BAND_1, &mac_val);
	mac_val = mac_val & ~BIT31;
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_TMR_PA_BAND_1, mac_val);
#endif /*DBDC_MODE*/
	/* Configure all rx packets to HIF, except WOL2M packet */
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN0RCFR0, &mac_val_bnd[0]);
	mac_val_bnd[0] = 0x00010000; /* drop duplicate */
	/* TODO: shiang-MT7615, make sure the bit31~30 setting for BN0_RX2M_QID */
	mac_val_bnd[0] |= 0xc0108000; /* receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED */
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0RCFR0, mac_val_bnd[0]);
	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN0VCFR0, &mac_val);
	mac_val &= (~(BIT0)); /* To HIF */
	mac_val |= BIT13; /* RxRing 1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0VCFR0, mac_val);
#ifdef DBDC_MODE
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN1RCFR0, &mac_val_bnd[1]);
	mac_val_bnd[1] = mac_val_bnd[0];
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN1RCFR0, mac_val_bnd[1]);
	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN1VCFR0, &mac_val);
	mac_val &= (~(BIT0)); /* To HIF */
	mac_val |= BIT13; /* RxRing 1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN1VCFR0, mac_val);
#endif /*DBDC_MODE*/
	/*RCR for Ctrl Fram can match*/
	MAC_IO_READ32(pAd->hdev_ctrl, WTBL_OFF_RCR, &mac_val);
	mac_val |= CHECK_CTRL(1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_RCR, mac_val);
	/* TODO: shiang-MT7615, need further check!! */
	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	MtAsicSetBARTxCntLimit(pAd, TRUE, 1);
	MtAsicSetBARTxRate(pAd);
	/* TODO: shiang-MT7615, document mismatch!! */
	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_ACR0, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ACR0, mac_val);
	/* Enable MIB counters for band 0 and band 1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M0SCR0, 0x7effffff);
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M0SCR1, 0x400fffe);
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M0PBSCR, 0x7f7f7f7f);
#ifdef DBDC_MODE
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M1SCR, 0x7ef3ffff);
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M1SCR1, 0xfffe);
	MAC_IO_WRITE32(pAd->hdev_ctrl, MIB_M1PBSCR, 0x7f7f7f7f);
#endif /*DBDC_MODE*/
	/* Enable RxV to HIF/MCU */
	MtAsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
	/* CCA Setting */
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR0, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR0, mac_val);
	/* TODO: shiang-MT7615, need further check!! */
#endif /* MAC_INIT_OFFLOAD */
}

#ifdef CONFIG_AP_SUPPORT
static VOID MtfAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_SBTOR1;
	BOOLEAN any_other_mbss_enable = FALSE;

	ASSERT(mbss_idx >= 1);

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value |= SBSS_TBTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value |= SUB_BSSID0_TIME_OFFSET_n(mbss_idx * BCN_TRANSMIT_ESTIMATE_TIME);
		Value |= TBTT0_n_INT_EN;
		Value |= PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
		pAd->ApCfg.ext_mbss_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_enable_bitmap)
			any_other_mbss_enable = TRUE;

		/* if there is any ext bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);

		if (any_other_mbss_enable == TRUE)
			Value |= SBSS_TBTT0_TSF0_EN;
		else
			Value &= ~SBSS_TBTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value &= ~TBTT0_n_INT_EN;
		Value &= ~PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
	}
}

VOID MtfDmacSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 regValue = 0;

	if (enable) {
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_ACBEN, &regValue);
		regValue |=  (1 << mbss_idx);
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_ACBEN, regValue);
	} else {
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_ACBEN, &regValue);
		regValue &=  ~(1 << mbss_idx);
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_ACBEN, regValue);
	}
}

VOID MtfDmacSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtfAsicSetMbssLPOffset(pAd, mbss_idx, enable);
}

static VOID MtfAsicSetExtTTTTLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[mbss_idx];
	UINT32 first_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[1];
	BOOLEAN any_other_mbss_tttt_enable = FALSE;

	if (mbss_idx == 0)
		return;

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, first_reg_base, &Value);
		Value |= SBSS_TTTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, first_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value &= ~SUB_BSSID0_TTTT_OFFSET_n_MASK;
		Value |= DEFAULT_TTTT_OFFSET_IN_MS;
		Value |= TTTT0_n_INT_EN;
		Value |= PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_tttt_enable_bitmap)
			any_other_mbss_tttt_enable = TRUE;

		/* if there is any ext bssid is enable. this bit shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, first_reg_base, &Value);

		if (any_other_mbss_tttt_enable == TRUE)
			Value |= SBSS_TTTT0_TSF0_EN;
		else
			Value &= ~SBSS_TTTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd->hdev_ctrl, first_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value &= ~TTTT0_n_INT_EN;
		Value &= ~PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
	}
}

VOID MtfDmacSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtfAsicSetExtTTTTLPOffset(pAd, mbss_idx, enable);
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef DBDC_MODE
static INT32 GetBctrlBitBaseByType(UCHAR Type, UINT8 Index)
{
	INT32 Bit = 0;

	switch (Type) {
	case  DBDC_TYPE_REPEATER: {
		Bit = REPEATER_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_PTA: {
		Bit = PTA_BAND_SELPTA_BIT_BASE;
	}
	break;

	case DBDC_TYPE_BF: {
		Bit = BF_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_MU: {
		Bit = MU_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_BSS: {
		Bit = BSS_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_MBSS: {
		Bit = MBSS_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_MGMT: {
		Bit = MNG_BAND_SEL_BIT_BASE;
	}
	break;

	case DBDC_TYPE_WMM: {
		Bit = WMM_BAND_SEL_BIT_BASE;
	}
	break;

	default:
		Bit = -1;
		break;
	}

	if (Bit >= 0)
		Bit += Index;

	return Bit;
}

INT32 MtAsicGetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 Value = 0, i = 0, j = 0;

	HW_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL0, &Value);
	/*DBDC enable will not need BctrlEntries so minus 1*/
	pbInfo->TotalNum = MAX_BCTRL_ENTRY-1;
	/*DBDC Enable*/
	pbInfo->DBDCEnable = (Value  >> DBDC_EN_BIT_BASE) & 0x1;
	/*PTA*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_PTA;
	pbInfo->BctrlEntries[i].Index = 0;
	pbInfo->BctrlEntries[i].BandIdx = (Value  >> PTA_BAND_SELPTA_BIT_BASE) & 0x1;
	i++;
	/*MU*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MU;
	pbInfo->BctrlEntries[i].Index = 0;
	pbInfo->BctrlEntries[i].BandIdx = (Value  >> MU_BAND_SEL_BIT_BASE) & 0x1;
	i++;

	/*BF*/
	for (j = 0; j < 3; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BF;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+BF_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	/*WMM*/
	for (j = 0; j < 4; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_WMM;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+WMM_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	/*MGMT*/
	for (j = 0; j < 2; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MGMT;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+MNG_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	/*MBSS*/
	for (j = 0; j < 15; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MBSS;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+MBSS_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	/*BSS*/
	for (j = 0; j < 5; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BSS;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j + BSS_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	HW_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL1, &Value);

	/*Repeater*/
	for (j = 0; j < 32; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_REPEATER;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+REPEATER_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	return 0;
}


INT32 MtAsicSetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 Value1 = 0, Value2 = 0;
	UINT32 i = 0;
	INT32 shift = 0; /* GetBctrlBitBaseByType may return -1 , declare signed variable:shift */
	BCTRL_ENTRY_T *pEntry = NULL;

	/*Clock Control for Band1, Band0 is enabled by MacInit*/
	if (pbInfo->DBDCEnable) {
		MAC_IO_READ32(pAd->hdev_ctrl, CFG_CCR, &Value1);
		Value1 |= (BIT30 | BIT24);
		MAC_IO_WRITE32(pAd->hdev_ctrl, CFG_CCR, Value1);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "MAC D1 2x 1x initial(val=%x)\n", Value1);
	}

	/*BandCtrl*/
	MAC_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL0, &Value1);
	MAC_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL1, &Value2);
	Value1  |= DBDC_EN(pbInfo->DBDCEnable);

	for (i = 0; i < pbInfo->TotalNum; i++) {
		pEntry = &pbInfo->BctrlEntries[i];
		shift = GetBctrlBitBaseByType(pEntry->Type, pEntry->Index);

		if (shift < 0)
			continue;

		if (pEntry->Type != DBDC_TYPE_REPEATER) {
			Value1 &= ~(0x1 << shift);
			Value1 |= (pEntry->BandIdx & 0x1) << shift;
		} else {
			Value2 &= ~(0x1 << shift);
			Value2 |= (pEntry->BandIdx & 0x1) << shift;
		}
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, CFG_DBDC_CTRL0, Value1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, CFG_DBDC_CTRL1, Value2);
	return 0;
}
#endif /*DBDC_MODE*/


#ifndef COMPOS_TESTMODE_WIN
VOID MtfSetTmrCal(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR TmrType,
	IN  UCHAR Channel,
	IN UCHAR Bw)
{
	UINT32  value = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_B0BRR0, &value);

	if (TmrType == TMR_DISABLE)
		/* Enanle Spatial Extension for ACK/BA/CTS after TMR Disable*/
		value |= BSSID00_RESP_SPE_EN;
	else
		/* Disable Spatial Extension for ACK/BA/CTS when TMR Enable*/
		value &= ~BSSID00_RESP_SPE_EN;

	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_B0BRR0, value);
}
#endif

/*
 * enable/disable beaconQ,
 * return status for disable beaconQ.
 */
static BOOLEAN MtDmacAsicSetBeaconQ(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OmacIdx,
	UCHAR BandIdx,
	BOOLEAN enable)
{
	UINT32 cr_collection[2][4] = {
		{ARB_TQSM0, ARB_TQSE0, ARB_TQFM0, ARB_TQFE0}, /*band 0*/
		{ARB_TQSM1, ARB_TQSE1, ARB_TQFM1, ARB_TQFE1}
	};
	UINT32 Value = 0;
	UINT32 cr_base = 0;

	if (BandIdx >= DBDC_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "BandIdx >= 2\n");
		return FALSE;
	}

	if (enable) {
		if (OmacIdx > HW_BSSID_MAX)
			cr_base = cr_collection[BandIdx][1];
		else
			cr_base = cr_collection[BandIdx][0];
	} else {
		if (OmacIdx > HW_BSSID_MAX)
			cr_base = cr_collection[BandIdx][3];
		else
			cr_base = cr_collection[BandIdx][2];
	}

	MAC_IO_READ32(pAd->hdev_ctrl, cr_base, &Value);

	if (OmacIdx > HW_BSSID_MAX)
		Value |= (1 << OmacIdx);
	else
		Value |= (1 << (0x10 + OmacIdx));

	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_base, Value);
	return TRUE;
}

BOOLEAN MtfDmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;
	UINT8 OmacIdx = wdev->OmacIdx;
	UCHAR BandIdx = HcGetBandByWdev(wdev);

	if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW) {
		/* FW help to disable beacon. */
		return TRUE;
	} else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT)
		return MtDmacAsicSetBeaconQ(pAd, OmacIdx, BandIdx, TRUE);

	return TRUE;
}

BOOLEAN MtfDmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;
	UINT8 OmacIdx = wdev->OmacIdx;
	UCHAR BandIdx = HcGetBandByWdev(wdev);

	if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW) {
		/* FW help to disable beacon. */
		return TRUE;
	} else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT)
		return MtDmacAsicSetBeaconQ(pAd, OmacIdx, BandIdx, FALSE);

	return TRUE;
}

INT MtfAsicTOPInit(RTMP_ADAPTER *pAd)
{
	return TRUE;
}

VOID MtfAsicGetTxTscByDriver(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	USHORT Wcid = 0;
	struct wtbl_entry tb_entry;
	UINT32 val = 0;

	GET_GroupKey_WCID(wdev, Wcid);
	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

	if (asic_get_wtbl_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Cannot found WTBL2/3/4 for WCID(%d)\n", Wcid);
		return;
	}

	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + (4 * 9), &val);
	*pTxTsc     = val & 0xff;
	*(pTxTsc+1) = (val >> 8) & 0xff;
	*(pTxTsc+2) = (val >> 16) & 0xff;
	*(pTxTsc+3) = (val >> 24) & 0xff;
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + (4 * 10), &val);
	*(pTxTsc+4) = val & 0xff;
	*(pTxTsc+5) = (val >> 8) & 0xff;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WCID(%d) TxTsc "MACSTR"\n",
			 Wcid,
			 MAC2STR(pTxTsc));
}

#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
INT mtf_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en)
{
	UINT32 cr;
	UINT32 value;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_ADDR :
		BN0_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_ADDR;
	value = (rts_len & BN0_WF_AGG_TOP_PCR1_RTS0_PKT_LEN_THRESHOLD_MASK) |
		(rts_num << BN1_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_SHFT);

	MAC_IO_WRITE32(ad->hdev_ctrl, cr, value);
	return 0;
}

INT mtf_asic_set_agglimit(RTMP_ADAPTER *pAd, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit)
{
	UINT32 cr;
	UINT32 value = 0;
	UINT8 wmm_set;

	if (wdev)
		wmm_set = HcGetWmmIdx(pAd, wdev);
	else
		return 1;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_AALCR0_ADDR :
		BN0_WF_AGG_TOP_AALCR0_ADDR;

	cr += (wmm_set << 2);

	MAC_IO_READ32(pAd->hdev_ctrl, cr, &value);

	if ((value & (0x0FF << (ac << 3))) == ((agg_limit & 0x0FF) << (ac << 3)))
		return 0;

	value &= ~(0x0FF << (ac << 3));
	value |= ((agg_limit & 0x0FF) << (ac << 3));
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr, value);

	return 0;
}

INT mtf_asic_set_rts_retrylimit(RTMP_ADAPTER *pAd, UCHAR band_idx, UINT32 limit)
{
	UINT32 cr;
	UINT32 value = 0;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_MRCR_ADDR :
		BN0_WF_AGG_TOP_MRCR_ADDR;

	MAC_IO_READ32(pAd->hdev_ctrl, cr, &value);

	if ((value & (0x01F << 7)) == ((limit & 0x01F) << 7))
		return 0;

	value &= ~(0x01F << 7);
	value |= ((limit & 0x01F) << 7);
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr, value);

	return 0;
}
#ifdef DOT11_VHT_AC
INT mtf_asic_set_rts_signal_ta(RTMP_ADAPTER *ad, UINT8 band_idx, BOOLEAN enable)
{
	UINT32 tcr_value = 0, ducr0_value = 0;
	UINT32 rts_sig_mask = 0, opt_rts_data_mask = 0;

	if (band_idx) {
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, &tcr_value);
		rts_sig_mask = BN1_WF_TMAC_TOP_TCR0_RTS_SIGTA_EN_MASK;
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_DUCR0_ADDR, &ducr0_value);
		opt_rts_data_mask = BN1_WF_TMAC_TOP_DUCR0_OPT_DUR_RTS_DATA_MASK;
	} else {
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, &tcr_value);
		rts_sig_mask = BN0_WF_TMAC_TOP_TCR0_RTS_SIGTA_EN_MASK;
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_DUCR0_ADDR, &ducr0_value);
		opt_rts_data_mask = BN0_WF_TMAC_TOP_DUCR0_OPT_DUR_RTS_DATA_MASK;
	}

	/* If enable RTS signaling then use duration mode 1 for PPDU protection */
	if (enable) {
		tcr_value |= rts_sig_mask;
		ducr0_value |= opt_rts_data_mask;
	} else {
		tcr_value &= ~(rts_sig_mask);
		ducr0_value &= ~(opt_rts_data_mask);
	}

	if (band_idx) {
		MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, tcr_value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_DUCR0_ADDR, ducr0_value);
	} else {
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, tcr_value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_DUCR0_ADDR, ducr0_value);
	}

	return TRUE;
}
#endif

UINT32 mtf_get_mib_bcn_tx_cnt(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	if (band_idx)
		MAC_IO_READ32(pAd->hdev_ctrl, BN1_WF_MIB_TOP_M0SDR0_ADDR, &value);
	else
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR0_ADDR, &value);

	return (value & BN0_WF_MIB_TOP_M0SDR0_BEACONTXCOUNT_MASK);
}
#endif

#ifdef AIR_MONITOR
void apply_mntr_ruleset_smesh(PRTMP_ADAPTER pAd, EXT_CMD_SMESH_T *pconfig_smesh)
{
	if (pAd->MntRuleBitMap & RULE_CTL)
		pconfig_smesh->fgSmeshRxCtrl = 1;
	else
		pconfig_smesh->fgSmeshRxCtrl = 0;

	if (pAd->MntRuleBitMap & RULE_MGT)
		pconfig_smesh->fgSmeshRxMgnt = 1;
	else
		pconfig_smesh->fgSmeshRxMgnt = 0;

	if (pAd->MntRuleBitMap & RULE_DATA)
		pconfig_smesh->fgSmeshRxData = 1;
	else
		pconfig_smesh->fgSmeshRxData = 0;

	if (pAd->MntRuleBitMap & RULE_A1)
		pconfig_smesh->fgSmeshRxA1 = 1;
	else
		pconfig_smesh->fgSmeshRxA1 = 0;

	if (pAd->MntRuleBitMap & RULE_A2)
		pconfig_smesh->fgSmeshRxA2 = 1;
	else
		pconfig_smesh->fgSmeshRxA2 = 0;
}

INT mtf_set_air_monitor_enable(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx)
{
	UCHAR flush_all = FALSE;
	UINT32 i = 0;
	BOOLEAN bSMESHEn = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	EXT_CMD_SMESH_T *pconfig_smesh = NULL;
	EXT_EVENT_SMESH_T rSmeshResult;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	UCHAR *pdata_muar = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_SMESH_PARAM_T rSmeshGetResult;

	NdisZeroMemory(&rSmeshGetResult, sizeof(UNI_CMD_SMESH_PARAM_T));
#endif /* WIFI_UNIFIED_COMMAND */
	NdisZeroMemory(&rSmeshResult, sizeof(EXT_EVENT_SMESH_T));
	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata_muar,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	if (!pdata_muar)
		return FALSE;

	os_alloc_mem(pAd, (UCHAR **)&pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	if (!pconfig_smesh) {
		os_free_mem(pdata_muar);
		return FALSE;
	}

	NdisZeroMemory(pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");


	if (band_idx == BAND0) {
		pconfig_smesh->ucBand = BAND0;
		pconfig_smesh->ucAccessMode = 0;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
			if (rSmeshGetResult.ucEntryEnable) {
				bSMESHEn = TRUE;
				flush_all = TRUE;
			}
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
			if ((rSmeshResult.u4SmeshVal & SMESH_ADDR_EN)) {
				bSMESHEn = TRUE;
				flush_all = TRUE;
			}
		}

	} else if (band_idx == BAND1) {

		pconfig_smesh->ucBand = BAND1;
		pconfig_smesh->ucAccessMode = 0;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support) {
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
			if ((rSmeshResult.u4SmeshVal & SMESH_ADDR_EN)) {
				bSMESHEn = TRUE;
				flush_all = TRUE;
			}
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
			if ((rSmeshResult.u4SmeshVal & SMESH_ADDR_EN)) {
				bSMESHEn = TRUE;
				flush_all = TRUE;
			}
		}
	}

	if (bSMESHEn != enable) {
		if (enable == 0) {
			if (flush_all == TRUE) {
				for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
					pMuarGroup = &pAd->MntGroupTable[band_idx][i];

					if (pMuarGroup->bValid && (pMuarGroup->Band == band_idx))
						NdisZeroMemory(pMuarGroup, sizeof(*pMuarGroup));
				}

				for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {

					pMntEntry = &pAd->MntTable[band_idx][i];

					if (!pMntEntry->bValid || (pMntEntry->Band != band_idx))
						continue;


					muar_entry.ucMuarIdx = pMntEntry->muar_idx;
					config_muar.ucAccessMode = MUAR_WRITE;
					config_muar.ucMuarModeSel = MUAR_NORMAL;
					config_muar.ucEntryCnt = 1;
					config_muar.ucBand = band_idx;

					NdisMoveMemory(pdata_muar, (UCHAR *)&config_muar, sizeof(EXT_CMD_MUAR_T));
					NdisMoveMemory(pdata_muar + sizeof(EXT_CMD_MUAR_T),
								   (UCHAR *)&muar_entry,
								   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support)
						UniCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);
					else
#endif /* WIFI_UNIFIED_COMMAND */
						MtCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);

					pMacEntry = pMntEntry->pMacEntry;

					if (pMacEntry) {
						if (band_idx == BAND1)
							pMacEntry->mnt_band &= ~MNT_BAND1;
						else
							pMacEntry->mnt_band &= ~MNT_BAND0;

						if (pMacEntry->mnt_band == 0) { /* no more use for other band */
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									 " call MacTableDeleteEntry(WCID=%d)- "MACSTR"\n",
									  pMacEntry->wcid, MAC2STR(pMacEntry->Addr));
							MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
						}
					}

					pAd->MonitrCnt[band_idx]--;
					NdisZeroMemory(pMntEntry, sizeof(*pMntEntry));
				}

				if (band_idx == DBDC_BAND0) {
					pconfig_smesh->ucBand = BAND0;
					pconfig_smesh->ucAccessMode = 1;
					pconfig_smesh->ucSmeshEn = 0;
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support)
						MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
					else
#endif /* WIFI_UNIFIED_COMMAND */
						MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
				} else if (band_idx == DBDC_BAND1) {
					pconfig_smesh->ucBand = BAND1;
					pconfig_smesh->ucAccessMode = 1;
					pconfig_smesh->ucSmeshEn = 0;
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support)
						MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
					else
#endif /* WIFI_UNIFIED_COMMAND */
						MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
				} else {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wrong band index(%d)\n",
							 band_idx);
					os_free_mem(pconfig_smesh);
					os_free_mem(pdata_muar);
					return FALSE;
				}

			if (pAd->MonitrCnt[band_idx] == 0) {
				pAd->MntEnable[band_idx] = 0;
				NdisZeroMemory(&pAd->MntTable[band_idx], sizeof(MNT_STA_ENTRY));
				NdisZeroMemory(&pAd->MntGroupTable[band_idx], sizeof(MNT_MUAR_GROUP));
			}
			}
		} else {
			if (band_idx == DBDC_BAND0) {
				pconfig_smesh->ucBand = BAND0;
				pconfig_smesh->ucAccessMode = 1;
				pconfig_smesh->ucSmeshEn = enable;
				apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
				if (cap->uni_cmd_support)
					MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
				else
#endif /* WIFI_UNIFIED_COMMAND */
					MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
			} else if (band_idx == DBDC_BAND1) {

				pconfig_smesh->ucBand = BAND1;
				pconfig_smesh->ucAccessMode = 1;
				pconfig_smesh->ucSmeshEn = enable;
				apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
				if (cap->uni_cmd_support)
					MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
				else
#endif /* WIFI_UNIFIED_COMMAND */
					MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
			} else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " wrong band index(%d)\n",
						 band_idx);
					os_free_mem(pconfig_smesh);
					os_free_mem(pdata_muar);
				return FALSE;
			}

		}
	}

	os_free_mem(pconfig_smesh);
	os_free_mem(pdata_muar);


	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

INT mtf_set_air_monitor_rule(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx)
{
	INT ret = TRUE;
	EXT_EVENT_SMESH_T rSmeshResult;
	EXT_CMD_SMESH_T *pconfig_smesh = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_SMESH_PARAM_T rSmeshGetResult;
#endif /* WIFI_UNIFIED_COMMAND */

	if (!rule)
		return FALSE;

	os_alloc_mem(pAd, (UCHAR **)&pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	if (!pconfig_smesh)
		return FALSE;

	MTWF_PRINT("--> %s()\n", __func__);

	NdisZeroMemory(pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	pAd->MntRuleBitMap &= ~RULE_CTL;
	pAd->MntRuleBitMap &= ~RULE_MGT;
	pAd->MntRuleBitMap &= ~RULE_DATA;
	pAd->MntRuleBitMap |= (RULE_DATA & ((UINT32)rule[0] << RULE_DATA_OFFSET))
					| (RULE_MGT & ((UINT32)rule[1] << RULE_MGT_OFFSET))
					| (RULE_CTL & ((UINT32)rule[2] << RULE_CTL_OFFSET));
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
		pAd->MntRuleBitMap &= ~SMESH_RX_A1;
	}
#endif

	if (band_idx == DBDC_BAND0) {
		pconfig_smesh->ucBand = BAND0;
		pconfig_smesh->ucAccessMode = 1;
		pconfig_smesh->ucSmeshEn = 1;
		apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
	} else if (band_idx == DBDC_BAND1) {
		pconfig_smesh->ucBand = BAND1;
		pconfig_smesh->ucAccessMode = 1;
		pconfig_smesh->ucSmeshEn = 1;
		apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wrong band index(%d)\n",
				 band_idx);
		os_free_mem(pconfig_smesh);
		return FALSE;
	}

	MTWF_PRINT("<-- %s()\n", __func__);

	os_free_mem(pconfig_smesh);


	return ret;
}


INT mtf_set_air_monitor_idx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx)
{
	INT ret, i;
	UCHAR *p = ZERO_MAC_ADDR, muar_idx = 0, muar_group_base = 0;
	BOOLEAN bCreate = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	UCHAR *pdata_muar = NULL;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	UCHAR mnt_enable = 0;
	UCHAR totalMonitrCnt = 0, value = 0, remainder = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata_muar,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));


	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");


	if (mnt_idx < MAX_NUM_OF_MONITOR_STA) {
		pAd->MntIdx[band_idx] = mnt_idx;
		pMntEntry = &pAd->MntTable[band_idx][mnt_idx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "The index is over the maximum limit.\n");
		os_free_mem(pdata_muar);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(ZERO_MAC_ADDR, pAd->curMntAddr)) {
		if (pMntEntry->bValid) {
			pMacEntry = pMntEntry->pMacEntry;

			if (pMacEntry) {
				if (band_idx == BAND1)
					pMacEntry->mnt_band &= ~MNT_BAND1;
				else
					pMacEntry->mnt_band &= ~MNT_BAND0;

				if (pMacEntry->mnt_band == 0) /* no more use for other band */
					MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
			}

			if (pMntEntry->muar_group_idx < MAX_NUM_OF_MONITOR_GROUP)
				pMuarGroup = &pAd->MntGroupTable[band_idx][pMntEntry->muar_group_idx];

			if (pMuarGroup) {
				pMuarGroup->Count--;

				if (pMuarGroup->Count == 0)
					pMuarGroup->bValid = FALSE;
			}

			pMntEntry->bValid = FALSE;

			if (pAd->MonitrCnt[band_idx] > 0)
				pAd->MonitrCnt[band_idx]--;

			muar_idx = pMntEntry->muar_idx;
		} else {
			os_free_mem(pdata_muar);
			return TRUE;
		}
	} else {
		if (pMntEntry->bValid) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The index of existed monitor entry.\n");
			os_free_mem(pdata_muar);
			return TRUE;
		}

		/*Comments: 7915 dual band have 2 seperated MUAR table, can monitor 16 + 16 STAs.*/
		if ((pAd->MonitrCnt[band_idx]) >= MAX_NUM_OF_MONITOR_STA) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The monitor number extends to maximum limit(%d).\n", MAX_NUM_OF_MONITOR_STA);
			os_free_mem(pdata_muar);
			return FALSE;
		}
		/*muar_group_idx: 	0	1	2	3	4	5	6	7*/
		/*MUAR Index:		32	36	40	44	48	52	56	60*/
		/*					34	38	42	46	50	54	58	62*/
		for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
			pMuarGroup = &pAd->MntGroupTable[band_idx][i];

			if (!pMuarGroup->bValid) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMuarGroup->MuarGroupBase = MONITOR_MUAR_BASE_INDEX + i * MAX_NUM_PER_GROUP;
				pMuarGroup->bValid = TRUE;
				pMuarGroup->Band = band_idx;
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + i * 2);
				pMuarGroup->Count++;
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			} else if ((pMuarGroup->Count < MAX_NUM_PER_GROUP) &&
					   (pMuarGroup->Band == band_idx)) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + 2 * (i + 1));
				pMuarGroup->Count++;
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			}
		}

		if (bCreate) {
			COPY_MAC_ADDR(pMntEntry->addr, pAd->curMntAddr);
			pMacEntry = MacTableLookup(pAd, pMntEntry->addr);

			if (pMacEntry == NULL) {
				pMacEntry = MacTableInsertEntry(
								pAd,
								pMntEntry->addr,
								wdev,
								ENTRY_CAT_MONITOR,
								OPMODE_STA,
								TRUE);

				if (wdev_do_conn_act(wdev, pMacEntry) != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "connect fail!!\n");
					os_free_mem(pdata_muar);
					return FALSE;
				}
			}

			if (pMacEntry) {
				p = pMntEntry->addr;
				pAd->MonitrCnt[band_idx]++;
				pMntEntry->bValid = TRUE;
				pMntEntry->Band = band_idx;
				pMacEntry->mnt_idx[band_idx] = mnt_idx;

				if (band_idx == BAND1)
					pMacEntry->mnt_band |= MNT_BAND1;
				else
					pMacEntry->mnt_band |= MNT_BAND0;

				pMntEntry->pMacEntry = pMacEntry;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't create a monitor entry!\n");

			os_free_mem(pdata_muar);
			return FALSE;
		}
	}

	MTWF_PRINT("index: %d\n", mnt_idx);
	MTWF_PRINT("entry: "MACSTR"\n", MAC2STR(p));

	muar_entry.ucMuarIdx = muar_idx;
	NdisMoveMemory(muar_entry.aucMacAddr, p, MAC_ADDR_LEN);
	config_muar.ucAccessMode = MUAR_WRITE;
	config_muar.ucMuarModeSel = MUAR_NORMAL;
	config_muar.ucEntryCnt = 1;
	config_muar.ucBand = band_idx;

	NdisMoveMemory(pdata_muar, (UCHAR *)&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata_muar + sizeof(EXT_CMD_MUAR_T),
				   (UCHAR *)&muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);

	/*total 16 STA, 1 otects control CR 820e5048[7:0]*/
	/*MonitrCnt:		1,2  3,4  5,6  7,8	9,10   11,12  13,14	15,16*/
	/*Bit:				 0	  1    2	3	 4		5	   6	  7*/
	/*MUAR Index:		32	 36   40	44	 48 	52	   56	  60*/
	/*					34	 38   42	46	 50 	54	   58	  62*/
	totalMonitrCnt = pAd->MonitrCnt[band_idx];
	if (totalMonitrCnt > 0) {
		value = (totalMonitrCnt / 2);
		remainder = (totalMonitrCnt % 2);
		if (remainder == 1) {
			pAd->MntEnable[band_idx] |= (1 << value);
		}
	}
	mnt_enable = pAd->MntEnable[band_idx];
	ret = mtf_set_air_monitor_enable(pAd, mnt_enable, band_idx);

	MTWF_PRINT("<-- %s()[Band%d]MntEnable=0x%x\n", __func__, band_idx, mnt_enable);

	os_free_mem(pdata_muar);
	return ret;
}
#endif
