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
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"

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
	UINT32 Value, CurCnt = 0;

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

VOID MtAsicRcpiReset(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	UINT32 u4RegVal;

	if (WtblWaitIdle(pAd, 100, 50) != TRUE)
		return;

	u4RegVal = (wcid | (1 << 15));
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, u4RegVal);
}

UINT32 MtAsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	UINT32	msdr16 = 0;
#ifdef CONFIG_AP_SUPPORT
#ifdef OFFCHANNEL_SCAN_FEATURE
	if (BandIdx > DBDC_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong BandIdx %d\n", BandIdx);
		return;
	}
	SCAN_CTRL *ScanCtrl = &pAd->ScanCtrl[BandIdx];
	if (ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index] != 0) {
		UINT32	OBSSAirtime, MyTxAirtime, MyRxAirtime;
		UINT32	CrValue;
		UINT32 ChBusytime;

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"[%d][%s]: Scan time : %u \n", __LINE__, __func__,
						ScanCtrl->ScanTime[ScanCtrl->CurrentGivenChan_Index]);
		if ((pAd->CommonCfg.dbdc_mode) && (pAd->ChannelInfo.bandidx == DBDC_BAND1)) {

			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d][%s]: Band 1\n", __LINE__, __func__);
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME6, &CrValue);
			OBSSAirtime = CrValue;

			/*My Tx Air time*/
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR36, &CrValue);
			MyTxAirtime = (CrValue & 0xffffff);

			/*My Rx Air time*/
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR37, &CrValue);
			MyRxAirtime = (CrValue & 0xffffff);
		} else{ /*band 0*/
			/*OBSS Air time*/
			(MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d][%s]: Band 0\n", __LINE__, __func__);
			HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME5, &CrValue);
			OBSSAirtime = CrValue;

			/*My Tx Air time*/
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR36, &CrValue);
			MyTxAirtime = (CrValue & 0xffffff);

			/*My Rx Air time*/
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR37, &CrValue);
			MyRxAirtime = (CrValue & 0xffffff);
		}
		pAd->ChannelInfo.ChStats.Tx_Time = MyTxAirtime;
		pAd->ChannelInfo.ChStats.Rx_Time = MyRxAirtime;
		pAd->ChannelInfo.ChStats.Obss_Time = OBSSAirtime;
		/*Ch Busy time*/
		ChBusytime = OBSSAirtime + MyTxAirtime + MyRxAirtime;

		/*Reset OBSS Air time*/
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
		CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
		CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
		HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"[%d][%s]: ChannelBusyTime : %u\n", __LINE__, __func__, ChBusytime);

		return ChBusytime;
	}
#endif
	if (pAd->CommonCfg.dbdc_mode == 0) {
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR16, &msdr16);
	} else {
		switch (BandIdx) {
		case BAND0:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR16, &msdr16);
			break;
		case BAND1:
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR16, &msdr16);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "DBDC BandIdx ERROR:%d\n", BandIdx);
			break;
		}
	}
	msdr16 &= 0x00ffffff;

#endif /* CONFIG_AP_SUPPORT */
	return msdr16;
}

#ifdef CONFIG_STA_SUPPORT
VOID MtAsicUpdateAutoFallBackTable(RTMP_ADAPTER *pAd, UCHAR *pRateTable)
{
	/* TODO: shiang-7603 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}
#endif /* CONFIG_STA_SUPPORT */


INT32 MtAsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	MAC_IO_READ32(pAd->hdev_ctrl, AGG_ARUCR, &Value);
	Value &= ~RATE1_UP_MPDU_LIMIT_MASK;
	Value |= RATE1_UP_MPDU_LINIT(2);
	Value &= ~RATE2_UP_MPDU_LIMIT_MASK;
	Value |= RATE2_UP_MPDU_LIMIT(2);
	Value &= ~RATE3_UP_MPDU_LIMIT_MASK;
	Value |= RATE3_UP_MPDU_LIMIT(2);
	Value &= ~RATE4_UP_MPDU_LIMIT_MASK;
	Value |= RATE4_UP_MPDU_LIMIT(2);
	Value &= ~RATE5_UP_MPDU_LIMIT_MASK;
	Value |= RATE5_UP_MPDU_LIMIT(1);
	Value &= ~RATE6_UP_MPDU_LIMIT_MASK;
	Value |= RATE6_UP_MPDU_LIMIT(1);
	Value &= ~RATE7_UP_MPDU_LIMIT_MASK;
	Value |= RATE7_UP_MPDU_LIMIT(1);
	Value &= ~RATE8_UP_MPDU_LIMIT_MASK;
	Value |= RATE8_UP_MPDU_LIMIT(1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ARUCR, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_ARDCR, &Value);
	Value &= ~RATE1_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE1_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE2_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE2_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE3_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE3_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE4_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE4_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE5_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE5_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE6_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE6_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE7_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE7_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE8_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE8_DOWN_MPDU_LIMIT(0);
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ARDCR, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, AGG_ARCR, &Value);
	Value |= INI_RATE1;
	Value |= FB_SGI_DIS;
	Value &= ~RTS_RATE_DOWN_TH_MASK;
	Value &= ~RATE_DOWN_EXTRA_RATIO_MASK;
	Value |= RATE_DOWN_EXTRA_RATIO(2);
	Value |= RATE_DOWN_EXTRA_RATIO_EN;
	Value &= ~RATE_UP_EXTRA_TH_MASK;
	Value |= RATE_UP_EXTRA_TH(4);
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ARCR, Value);
	Value = mtd_tx_rate_to_tmi_rate(MODE_HTMIX, MCS_1, 1, FALSE, 0);
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_ARCR1, Value);
	return TRUE;
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
VOID MtAsicSwitchChannel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	UINT32 val, reg;

	/* TODO: Need to fix */
	/* TODO: shiang-usw, unify the ops */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->ChipSwitchChannel)
		ops->ChipSwitchChannel(pAd, SwChCfg);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "For this chip, no specified channel switch function!\n");

	/* TODO: shiang-7615 */
	if (SwChCfg.BandIdx)
		reg = RMAC_CHFREQ1;
	else
		reg = RMAC_CHFREQ0;

	MAC_IO_READ32(pAd->hdev_ctrl, reg, &val);
	val &= (~0xff);
	val |= SwChCfg.ControlChannel;
	MAC_IO_WRITE32(pAd->hdev_ctrl, reg, val);
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

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
/* TODO: Carter/Star for Repeater can support DBDC, after define STA/APCLI/Repeater */
INT MtAsicSetReptFuncEnableByDriver(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	RMAC_MORE_STRUC rmac_more;

	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_MORE, &rmac_more.word);

	if (bEnable == 0)
		rmac_more.field.muar_mode_sel = 0;
	else
		rmac_more.field.muar_mode_sel = 1;

	/* configure band 0/band 1 into repeater mode concurrently. */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MORE, rmac_more.word);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MORE_BAND_1, rmac_more.word);
	return TRUE;
}


VOID MtAsicInsertRepeaterEntryByDriver(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR tempMAC[MAC_ADDR_LEN];
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	COPY_MAC_ADDR(tempMAC, pAddr);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"MACSTR"\n",
			  MAC2STR(tempMAC), CliIdx);
	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = tempMAC[0] + (tempMAC[1] << 8) + (tempMAC[2] << 16) + (tempMAC[3] << 24);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = tempMAC[4];
	rmac_mcbcs1.field.addr_47_40 = tempMAC[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
	rmac_mcbcs1.field.multicast_addr_index = (CliIdx * 2);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, rmac_mcbcs1.word);
}


VOID MtAsicRemoveRepeaterEntryByDriver(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
	rmac_mcbcs1.field.multicast_addr_index = (CliIdx * 2);/* start from idx 0 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, rmac_mcbcs1.word);/* clear client entry first. */
	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
	rmac_mcbcs1.field.multicast_addr_index = ((CliIdx * 2) + 1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, rmac_mcbcs1.word);/* clear rootap entry. */
}



VOID MtAsicInsertRepeaterRootEntryByDriver(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = pAddr[0] + (pAddr[1] << 8) + (pAddr[2] << 16) + (pAddr[3] << 24);
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, rmac_mcbcs0.addr_31_0);
	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = pAddr[4];
	rmac_mcbcs1.field.addr_47_40 = pAddr[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
	rmac_mcbcs1.field.multicast_addr_index = (ReptCliIdx * 2) + 1;
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, rmac_mcbcs1.word);
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


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


INT MtAsicSetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter)
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

INT MtAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return SetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}

INT MtAsicGetTsfTime(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	UINT32 Value = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, LPON_T0CR, &Value);
	Value = (Value & TSF_TIMER_HW_MODE_MASK) | TSF_TIMER_VALUE_READ;/* keep HW mode value. */
	MAC_IO_WRITE32(pAd->hdev_ctrl, LPON_T0CR, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, LPON_UTTR0, low_part);
	MAC_IO_READ32(pAd->hdev_ctrl, LPON_UTTR1, high_part);
	return TRUE;
}

typedef struct _SYNC_MODE_CR_TABLE_T {
	UINT32              u4ArbOpModeCR;
	UINT32              u4ArbBcnWmmCR;
	UINT32              u4LponMacTimerCr;
	UINT32              u4LponTbttCtrlCR;
	UINT32              u4LponPreTbttTime;/* set pretbtt time */
	UINT32              u4LponSyncModeCR;/* sync mode CR*/
	UINT32              u4IntEnableCR;
} SYNC_MODE_CR_TABLE_T, *PSYNC_MODE_CR_TABLE_T;

static SYNC_MODE_CR_TABLE_T g_arDisableSyncModeMapTable[HW_BSSID_MAX] = {
	/*WMM cr set band0 first, change it when is checked it link to Band1. */
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR1, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR1, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR3, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR3, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicDisableSyncByDriver(RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
#ifdef CONFIG_STA_SUPPORT
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;
	UINT i;
#endif
	UINT32 value = 0;

	if (HWBssidIdx >= HW_BSSID_MAX) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "---> HW OmacIdx:%x passed in is not correct\n",
				  HWBssidIdx);
		return;
	}

	cr_set = g_arDisableSyncModeMapTable[HWBssidIdx];

	/*1. disable hif interrupt pin*/
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4IntEnableCR, &value);

		switch (HWBssidIdx) {
		case HW_BSSID_0:
			value &= ~HWIER3_TBTT0;
			value &= ~HWIER3_PRETBTT0;
			break;

		case HW_BSSID_1:
			value &= ~HWIER0_TBTT1;
			value &= ~HWIER0_PRETBTT1;
			break;

		case HW_BSSID_2:
			value &= ~HWIER0_TBTT2;
			value &= ~HWIER0_PRETBTT2;
			break;

		case HW_BSSID_3:
			value &= ~HWIER0_TBTT3;
			value &= ~HWIER0_PRETBTT3;
			break;

		default:
			ASSERT(HWBssidIdx < HW_BSSID_MAX);
			break;
		}

		MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4IntEnableCR, value);
	}

	/*2. disable BeaconPeriodEn */
	MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponTbttCtrlCR, &value);
	value &= ~TBTTn_CAL_EN;
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponTbttCtrlCR, value);
	/*3. disable MPTCR pin*/
	/*NOTE: disable is write another CR at the same bit to disable. */
	value = 0;
	value |= (TBTT_TIMEUP_EN |
			  TBTT_PERIOD_TIMER_EN |
			  PRETBTT_TIMEUP_EN |
			  PRETBTT_INT_EN);

	if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
		value = (value << 8);

	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponMacTimerCr, value);
	/*4. recover BCN AIFS, CWmin, and HW TSF sync mode.*/
	MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, &value);
	value = value & TSF_TIMER_HW_MODE_FULL;
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, value);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i] != NULL)
				if (HWBssidIdx == pAd->wdev_list[i]->hw_bssid_idx)
					wdev = pAd->wdev_list[i];
		}

		BandIdx = HcGetBandByWdev(wdev);

		if (BandIdx == 1)
			cr_set.u4ArbBcnWmmCR = ARB_WMMBCN1;
	}
#endif
	value = 0;
	value |= ARB_WMMBCN_AIFS_DEFAULT_VALUE;
	value |= ARB_WMMBCN_CWMIN_DEFAULT_VALUE;
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4ArbBcnWmmCR, value);
	/*5. set ARB OPMODE */
	MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4ArbOpModeCR, &value);
	value &= ~(MT_ARB_SCR_OPMODE_MASK << (HWBssidIdx * 2));/* clean opmode */
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4ArbOpModeCR, value);
}

static SYNC_MODE_CR_TABLE_T g_arEnableSyncModeMapTable[HW_BSSID_MAX] = {
	/*WMM cr set band0 first, change it when is checked it link to Band1. */
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR0, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR0, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR2, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
	{ARB_SCR, ARB_WMMBCN0, LPON_MPTCR2, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

VOID MtAsicEnableBssSyncByDriver(
	RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
#ifdef CONFIG_STA_SUPPORT
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;
	UINT i;
#endif
	UINT32 value = 0;

	if (HWBssidIdx >= HW_BSSID_MAX) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "---> HW OmacIdx:%x passed in is not correct\n",
				  HWBssidIdx);
		return;
	}

	cr_set = g_arEnableSyncModeMapTable[HWBssidIdx];
	/*1. set ARB OPMODE */
	MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4ArbOpModeCR, &value);

	if (OPMode == OPMODE_AP) {
		value |=  (MT_ARB_SCR_BM_CTRL |
				   MT_ARB_SCR_BCN_CTRL |
				   MT_ARB_SCR_BCN_EMPTY);
	}

	value |= (OPMode << (HWBssidIdx * 2));
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4ArbOpModeCR, value);
#ifdef CONFIG_STA_SUPPORT

	/*2. set tsf sync mode.*/
	if (OPMode == OPMODE_ADHOC) {
		MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, &value);
		value = value | TSF_TIMER_HW_MODE_FULL_ADHOC;
		MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, value);

		/*TODO:
		 * Carter, wmmbcn is a global control CR,
		 * what if it's in case AP and Adhoc are working at the same time,
		 * the code segment shall be reviewed.
		 */
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i] != NULL)
				if (HWBssidIdx == pAd->wdev_list[i]->hw_bssid_idx)
					wdev = pAd->wdev_list[i];
		}

		BandIdx = HcGetBandByWdev(wdev);

		if (BandIdx == 1)
			cr_set.u4ArbBcnWmmCR = ARB_WMMBCN1;

		MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4ArbBcnWmmCR, &value);
		value |= (2 << 0); /* AIFS */
		value |= (0x1f << 8); /* CWMIN */
		MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4ArbBcnWmmCR, value);
	} else if (OPMode == OPMODE_STA) {
		MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, &value);
		value = value | TSF_TIMER_HW_MODE_FULL;
		MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, value);
	} else
#endif /* CONFIG_STA_SUPPORT */
		if (OPMode == OPMODE_AP) {
			MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, &value);
			value = value | TSF_TIMER_HW_MODE_TICK_ONLY;
			MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponSyncModeCR, value);
		}

	/*3. set Pretbtt time. */
	MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4LponPreTbttTime, &value);
	value |= DEFAULT_PRETBTT_INTERVAL_IN_MS << (HWBssidIdx * 8);
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponPreTbttTime, value);
	/*4. set MPTCR */
	value = 0;
	value |= (TBTT_TIMEUP_EN |
			  TBTT_PERIOD_TIMER_EN |
			  PRETBTT_TIMEUP_EN |
			  PRETBTT_INT_EN);

	if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
		value = (value << 8);

	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponMacTimerCr, value);
	/*5. set BeaconPeriod */
	value = 0;
#ifdef CONFIG_STA_SUPPORT

	if (OPMode == OPMODE_STA) {
		wdev = &pAd->StaCfg[0].wdev;
		value |= BEACONPERIODn(wdev->ucBeaconPeriod);
		value |= DTIMPERIODn(wdev->ucDtimPeriod);
	} else
#endif /* CONFIG_STA_SUPPORT*/
	{
		value |= BEACONPERIODn(BeaconPeriod);
	}

	value |= TBTTn_CAL_EN;
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4LponTbttCtrlCR, value);
#ifndef BCN_OFFLOAD_SUPPORT

	/*6. enable HOST interrupt pin.*/
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		MAC_IO_READ32(pAd->hdev_ctrl, cr_set.u4IntEnableCR, &value);

		switch (HWBssidIdx) {
		case HW_BSSID_0:
			value |= HWIER3_TBTT0;
			value |= HWIER3_PRETBTT0;
			break;

		case HW_BSSID_1:
			value |= HWIER0_TBTT1;
			value |= HWIER0_PRETBTT1;
			break;

		case HW_BSSID_2:
			value |= HWIER0_TBTT2;
			value |= HWIER0_PRETBTT2;
			break;

		case HW_BSSID_3:
			value |= HWIER0_TBTT3;
			value |= HWIER0_PRETBTT3;
			break;

		default:
			ASSERT(HWBssidIdx < HW_BSSID_MAX);
			break;
		}

		MAC_IO_WRITE32(pAd->hdev_ctrl, cr_set.u4IntEnableCR, value);
	}

#endif
}



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


UINT32 MtAsicGetWmmParam(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 addr = 0, cr_val, mask = 0, shift = 0;

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
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __func__, __LINE__, EdcaType));
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

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
#define RX_PKT_MAX_LENGTH   0x400 /* WORD(4 Bytes) unit */

VOID MtAsicTxCapAndRateTableUpdate(
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

VOID MtAsicTxCntUpdate(RTMP_ADAPTER *pAd, UINT16 Wcid, MT_TX_COUNTER *pTxInfo)
{
	DMAC_TX_CNT_INFO tx_cnt_info;

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

VOID MtFeLossGet(RTMP_ADAPTER *pAd, UCHAR channel, CHAR *RssiOffset)
{
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	UINT rssi_idx;
	UCHAR eBand, ColumIdx;

	eBand = (channel > 14) ? BAND_5G : BAND_24G;

	if (channel <= 14)
		ColumIdx = EFUSE_2G4_WF_PATH_FE_LOSS_CATEGORY_FE_LOSS;
	else if ((channel >= 36) && (channel <= 64))
		ColumIdx = EFUSE_5G_WF_PATH_FE_LOSS_CATEGORY_FE_LOSS_GROUP_0;
	else if ((channel >= 68) && (channel <= 128))
		ColumIdx = EFUSE_5G_WF_PATH_FE_LOSS_CATEGORY_FE_LOSS_GROUP_1;
	else if ((channel >= 149) && (channel <= 165))
		ColumIdx = EFUSE_5G_WF_PATH_FE_LOSS_CATEGORY_FE_LOSS_GROUP_2;
	else
		ColumIdx = EFUSE_5G_WF_PATH_FE_LOSS_CATEGORY_ITEM_NUM;

	for (rssi_idx = 0; rssi_idx < MAX_ANTENNA_NUM; rssi_idx++) {
		rtmp_eeprom_WfPath_update(pAd, eBand, EFUSE_WF_PATH_CATEGORY_FE_LOSS, rssi_idx, ENUM_EFUSE_INFO_GET, CH_GROUP_NOT_CARE, ColumIdx, &RssiOffset[rssi_idx]);
		RssiOffset[rssi_idx] &= 0xF;
	}
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */
}

/* TODO: shiang-MT7615, use MMPS_DYNAMIC instead for "smps" */
VOID MtAsicSetSMPSByDriver(RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR Smps)
{
	struct wtbl_entry tb_entry;
	UINT32 dw;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

	if (asic_get_wtbl_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cannot found WTBL2/3/4 for WCID(%d)\n",
				 Wcid);
		return;
	}

	/* WTBL.dw3 bit 23, support Dynamic SMPS */
	HW_IO_READ32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 12, &dw);

	if (Smps)
		dw |= (0x1 << 23);
	else
		dw &= (~(0x1 << 23));

	HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 12, dw);
}

VOID MtAsicSetSMPS(RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR Smps)
{
	MtAsicSetSMPSByDriver(pAd, Wcid, Smps);
}


static BOOLEAN IsMtAsicWtblIndirectAccessIdle(RTMP_ADAPTER *pAd)
{
	UINT32 wiucr_val, cnt;
	BOOLEAN is_idle = FALSE;

	wiucr_val = (1<<31);
	cnt = 0;

	do {
		HW_IO_READ32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, &wiucr_val);

		if ((wiucr_val & (1<<31)) == 0)
			break;
		cnt++;
	} while (cnt < 100);

	if ((wiucr_val & (1<<31)) == 0) {
		is_idle = TRUE;
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): WIUCR is idle!wiucr_val=0x%x\n",
				 __func__, wiucr_val);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WIUCR is busy!wiucr_val=0x%x\n",
				 __func__, wiucr_val));
	}

	return is_idle;
}


/* WTBL Access for DW0~DW1, Perr unique key */
static INT MtAsicWtblWriteRxInfo(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 *val)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE) {
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RICR0, val[0]);	/* DW 0 */
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_RICR1, val[1]);	/* DW 1 */
		wiucr_val = (WTBL_RXINFO_UPDATE | WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	} else
		return FALSE;
};

static INT MtAsicWtblWritePsm(RTMP_ADAPTER *pAd, UINT16 wcid, UINT32 addr, UINT8 psm)
{
	UINT32 val;

	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_WTBLOR, WTBL_OR_PSM_W_FLAG);
	HW_IO_READ32(pAd->hdev_ctrl, addr, &val);
	val &= (~((1 << 30)));
	val |= ((psm << 30));
	HW_IO_WRITE32(pAd->hdev_ctrl, addr, val);
	MAC_IO_WRITE32(pAd->hdev_ctrl, WTBL_ON_WTBLOR, 0);
	return TRUE;
};


static INT MtAsicWtblDwWrite(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 base, UINT8 dw, UINT32 val)
{
	switch (dw) {
	case 0:
	case 1:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): Err!Please call MtAsicWtblWriteRxInfo() for this!\n", __func__));
		break;

	case 2:
	case 3:
	case 4:
		HW_IO_WRITE32(pAd->hdev_ctrl, base + dw * 4, val);
		break;

	case 5:
	case 6:
	case 7:
	case 8:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): Err!Please call MtAsicWtblWriteRateInfo() for this!\n", __func__));
		break;

	default:
		HW_IO_WRITE32(pAd->hdev_ctrl, base + dw * 4, val);
		break;
	}

	return TRUE;
}


/* WTBL Access for Rx Counter clear */
static INT MtAsicWtblRxCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE) {
		wiucr_val = (WTBL_RX_CNT_CLEAR | WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	} else
		return FALSE;
};


static INT MtAsicWtblTxCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE) {
		wiucr_val = (WTBL_TX_CNT_CLEAR | WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	} else
		return FALSE;
};


static INT MtAsicWtblAdmCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE) {
		wiucr_val = (WTBL_ADM_CNT_CLEAR | WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd->hdev_ctrl, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	} else
		return FALSE;
};


static INT MtAsicWtblFieldsReset(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 base)
{
	UINT32 Index;
	/* Clear BA Information */
	MtAsicWtblDwWrite(pAd, wlan_idx, base, 4, 0);
	/* RX Counter Clear */
	MtAsicWtblRxCounterClear(pAd, wlan_idx);
	/* TX Counter Clear */
	MtAsicWtblTxCounterClear(pAd, wlan_idx);

	/* Clear Cipher Key */
	for (Index = 0; Index < 16; Index++)
		HW_IO_WRITE32(pAd->hdev_ctrl, base + (4 * (Index + 30)), 0x0);

	/* Admission Control Counter Clear */
	MtAsicWtblAdmCounterClear(pAd, wlan_idx);
	return TRUE;
}


#define SN_MASK 0xfff

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtAsicDelWcidTabByDriver(RTMP_ADAPTER *pAd, UINT16 wcid_idx)
{
	UINT16 cnt, cnt_s, cnt_e;
	struct wtbl_entry tb_entry;
	UINT32 dw[5] = {0};

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid_idx=0x%x\n",
			 wcid_idx);

	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WTBL_MAX_NUM(pAd) - 1);
	} else
		cnt_s = cnt_e = wcid_idx;

	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++) {
		cnt = cnt_s;
		NdisZeroMemory(&tb_entry, sizeof(tb_entry));

		if (asic_get_wtbl_entry234(pAd, cnt, &tb_entry) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cannot found WTBL2/3/4 for WCID(%d)\n",
					 cnt);
			return;
		}

		/* TODO: shiang-MT7615, why we need to set rc_a2/rv as 1 even for delTab case?? */
		/* dw0->field.rc_a2 = 1; */
		/* dw0->field.rv = 1; */

		MtAsicWtblWriteRxInfo(pAd, wcid_idx, &dw[0]); /* DW0~1 */
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 4 * 2, dw[2]); /* DW 2 */
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 4 * 3, dw[3]); /* DW 3 */
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr + 4 * 4, dw[4]); /* DW 4 */
		MtAsicWtblWritePsm(pAd, wcid_idx, tb_entry.wtbl_addr + 4 * 3, 0);
		MtAsicWtblFieldsReset(pAd, wcid_idx, tb_entry.wtbl_addr);
	}
}

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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}


VOID MtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
			 __func__, __LINE__));
}
#endif /* STREAM_MODE_SUPPORT // */


static UINT32 reg_wmm_cr[4] = {ARB_TQSW0, ARB_TQSW1, ARB_TQSW2, ARB_TQSW3};
static UINT32 reg_mgmt_cr[2] = {ARB_TQSM0, ARB_TQSM1};

static INT32 MtAsicSetTxQ(RTMP_ADAPTER *pAd, INT WmmSet, INT BandIdx, BOOLEAN Enable)
{
	UINT32 reg_w = 0, mask_w = 0, val_w;
	UINT32 reg_mgmt = 0, mask_m = 0, val_m;

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

INT32 MtAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 Value, Value2;
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

INT MtAsicResetWPDMARGU(RTMP_ADAPTER *pAd)
{
#ifdef ERR_RECOVERY
	/* do not reset RGU if in SER L1 procedure*/
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return TRUE;
#endif /* ERR_RECOVERY */

	return TRUE;
}

INT MtAsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 Value;
	UINT32 Reg;

	Reg  = (BandIdx) ? TMAC_TCR1 : TMAC_TCR;
	MAC_IO_READ32(pAd->hdev_ctrl, Reg, &Value);
	Value &= ~TMAC_TCR_TX_STREAM_NUM_MASK;
	Value |= TMAC_TCR_TX_STREAM_NUM(StreamNums - 1);
	MAC_IO_WRITE32(pAd->hdev_ctrl, Reg, Value);
	return TRUE;
}

INT MtAsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
	UINT32 val, offset;
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
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s():Invalid BW(%d)!\n", __func__, bw));
	}

	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_BWCR, val);
	return TRUE;
}


UINT32 MtAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	UINT32 value = 0, temp = 0;

	if (IS_MT7663(pAd) || IS_MT7626(pAd)) {
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
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX2_ADDR, &temp);
			/* WF2 IB RSSI */
			value |= ((temp & 0xFF000000) >> 16);
			/* WF2 WB RSSI */
			value |= ((temp & 0x00FF0000) >> 16);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_0_RX3_ADDR, &temp);
			/* WF3 IB RSSI */
			value |= (temp & 0xFF000000);
			/* WF3 WB RSSI */
			value |= (temp & 0x00FF0000);
			break;

		case HQA_RX_STAT_ACI_HITL:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX0_ADDR, &temp);
			/* WF0 ACID HIT */
			value |= ((temp & 0x00020000) << 1);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX1_ADDR, &temp);
			/* WF1 ACID HIT */
			value |= (temp & 0x00020000);
			break;

		case HQA_RX_STAT_ACI_HITH:
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX2_ADDR, &temp);
			/* WF2 ACID HIT */
			value |= ((temp & 0x00020000) << 1);
			PHY_IO_READ32(pAd->hdev_ctrl, TALOS_WF_RXTD_RO_AGC_DEBUG_2_RX3_ADDR, &temp);
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
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR11, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR4, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR11, &value);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not finish Yet!\n",
			 __func__, __LINE__));
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

INT32 MtAsicGetFwSyncValue(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->get_fw_sync_value)
			return ops->get_fw_sync_value(pAd);

	return FALSE;
}


VOID MtAsicInitMac(RTMP_ADAPTER *pAd)
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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
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
static VOID MtAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
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

VOID MtDmacSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 regValue;

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

VOID MtDmacSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtAsicSetMbssLPOffset(pAd, mbss_idx, enable);
}

static VOID MtAsicSetExtTTTTLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
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

VOID MtDmacSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtAsicSetExtTTTTLPOffset(pAd, mbss_idx, enable);
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
VOID MtSetTmrCal(
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

BOOLEAN MtDmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void)
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

BOOLEAN MtDmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void)
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

INT MtAsicTOPInit(RTMP_ADAPTER *pAd)
{
	return TRUE;
}

VOID MtAsicGetTxTscByDriver(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 pn_type_mask, UCHAR *pTxTsc)
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
			 Wcid, MAC2STR(pTxTsc));
}

INT mt_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en)
{
	UINT32 cr;
	UINT32 value;
	/*adjust cts/rts rate*/
	cr = (band_idx == BAND1) ? TMAC_PCR1 : TMAC_PCR;
	value = (rts_en == TRUE) ? TMAC_PCR_AUTO_RATE : TMAC_PCR_FIX_OFDM_6M_RATE;
	MAC_IO_WRITE32(ad->hdev_ctrl, cr, value);
	/*adjust rts rts threshold*/
	cr = (band_idx == BAND1) ? AGG_PCR2 : AGG_PCR1;
	value = RTS_THRESHOLD(rts_len) | RTS_PKT_NUM_THRESHOLD(rts_num);
	MAC_IO_WRITE32(ad->hdev_ctrl, cr, value);
	return 0;
}


INT MtAsicAMPDUEfficiencyAdjustbyFW(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, UCHAR wmm_idx, UCHAR aifs_adjust)
{
	HW_SET_PART_WMM_PARAM(ad, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_AIFSN, aifs_adjust);
	return 0;
}

#ifdef DOT11_VHT_AC
INT MtAsicSetRtsSignalTA(RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable)
{
	UINT32 Value = 0;

	if (BandIdx)
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR1, &Value);
	else
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &Value);

	if (Enable)
		Value |= RTS_SIGTA_EN;
	else
		Value &= ~(RTS_SIGTA_EN);

	if (BandIdx)
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR1, Value);
	else
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, Value);

	return TRUE;
}
INT MtAsicAMPDUEfficiencyAdjust(struct _RTMP_ADAPTER *ad, UCHAR	wmm_idx, UCHAR aifs_adjust)
{
	UINT32 cr;
	UINT32 value;

	cr = ARB_WMMAC01+(wmm_idx*16);

	MAC_IO_READ32(ad->hdev_ctrl, cr, &value);

	value = ((value & 0xffffff00) | aifs_adjust);

	MAC_IO_WRITE32(ad->hdev_ctrl, cr, value);

	return 0;
}
#endif /* DOT11_VHT_AC */

UINT32 mtd_get_mib_bcn_tx_cnt(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;
	UINT32 band_offset = 0x200 * band_idx;

	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR0 + band_offset, &value);

	return (value & 0xffff);
}

#ifdef VLAN_SUPPORT
INT32 mt_asic_update_vlan_id(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid)
{
	UINT16 vid;
	UINT32 addr, vlantag;

	addr = DMA_VTR_GET_ADDR(omac_idx);
	MAC_IO_READ32(ad->hdev_ctrl, addr, &vlantag);
	vlantag = DMA_VTR_SET_VID(omac_idx, vlantag, vid);
	MAC_IO_WRITE32(ad->hdev_ctrl, addr, vlantag);

	return 0;
}

INT32 mt_asic_update_vlan_priority(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority)
{
	UINT8 priority;
	UINT32 addr, vlantag;

	addr = DMA_VTR_GET_ADDR(omac_idx);
	MAC_IO_READ32(ad->hdev_ctrl, addr, &vlantag);
	vlantag = DMA_VTR_SET_PCP(omac_idx, vlantag, priority);
	MAC_IO_WRITE32(ad->hdev_ctrl, addr, vlantag);

	return 0;
}
#endif

#ifdef AIR_MONITOR

void apply_mntr_ruleset(PRTMP_ADAPTER pAd, UINT32 *pu4SMESH)
{
	if (pAd->MntRuleBitMap & RULE_CTL)
		*pu4SMESH |= SMESH_RX_CTL;
	else
		*pu4SMESH &= ~SMESH_RX_CTL;

	if (pAd->MntRuleBitMap & RULE_MGT)
		*pu4SMESH |= SMESH_RX_MGT;
	else
		*pu4SMESH &= ~SMESH_RX_MGT;

	if (pAd->MntRuleBitMap & RULE_DATA)
		*pu4SMESH |= SMESH_RX_DATA;
	else
		*pu4SMESH &= ~SMESH_RX_DATA;

	if (pAd->MntRuleBitMap & RULE_A1)
		*pu4SMESH |= SMESH_RX_A1;
	else
		*pu4SMESH &= ~SMESH_RX_A1;

	if (pAd->MntRuleBitMap & RULE_A2)
		*pu4SMESH |= SMESH_RX_A2;
	else
		*pu4SMESH &= ~SMESH_RX_A2;
}


INT mtd_set_air_monitor_enable(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx)
{
	UCHAR flush_all = FALSE;
	UINT32 i, u4SMESH = 0, u4MAR0 = 0, u4MAR1 = 0;
	UCHAR *p = ZERO_MAC_ADDR, muar_idx = 0;
	BOOLEAN bSMESHEn = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;


	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> \n");

	if (band_idx == BAND0) {
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH, &u4SMESH);

		if ((u4SMESH & SMESH_ADDR_EN)) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	} else if (band_idx == BAND1) {
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH_B1, &u4SMESH);

		if ((u4SMESH & SMESH_ADDR_EN)) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	}

	if (bSMESHEn != enable) {
		if (enable == 0) {
			if (flush_all == TRUE) {
				for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
					pMuarGroup = &pAd->MntGroupTable[i];

					if (pMuarGroup->bValid && (pMuarGroup->Band == band_idx))
						NdisZeroMemory(pMuarGroup, sizeof(*pMuarGroup));
				}

				u4MAR0 = ((UINT32)p[0]) | ((UINT32)p[1]) << 8 |
						 ((UINT32)p[2]) << 16 | ((UINT32)p[3]) << 24;
				u4MAR1 = ((UINT32)p[4]) | ((UINT32)p[5]) << 8;

				for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
					pMntEntry = &pAd->MntTable[i];

					if (!pMntEntry->bValid || (pMntEntry->Band != band_idx))
						continue;

					muar_idx = pMntEntry->muar_idx;
					u4MAR1 |= MAR1_WRITE | MAR1_ACCESS_START_STATUS | (muar_idx << MAR1_ADDR_INDEX_OFFSET)
							  | (((UINT32)0 << MAR1_MAR_GROUP_OFFSET) & MAR1_MAR_GROUP_MASK);

					HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, u4MAR0);
					HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, u4MAR1);

					pMacEntry = pMntEntry->pMacEntry;

					if (pMacEntry) {
						if (band_idx == BAND1)
							pMacEntry->mnt_band &= ~MNT_BAND1;
						else
							pMacEntry->mnt_band &= ~MNT_BAND0;

						if (pMacEntry->mnt_band == 0) { /* no more use for other band */
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
									 ("%s::call MacTableDeleteEntry(WCID=%d)- "MACSTR"\n",
									  __func__, pMacEntry->wcid, MAC2STR(pMacEntry->Addr)));
							MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
						}
					}

					pAd->MonitrCnt[band_idx]--;
					NdisZeroMemory(pMntEntry, sizeof(*pMntEntry));
				}

				if (band_idx == DBDC_BAND0) {
					HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH, &u4SMESH);
					u4SMESH &= ~SMESH_ADDR_EN;
					HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH, u4SMESH);
				} else if (band_idx == DBDC_BAND1) {
					HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH_B1, &u4SMESH);
					u4SMESH &= ~SMESH_ADDR_EN;
					HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH_B1, u4SMESH);
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
							 __func__, band_idx));
					return FALSE;
				}
			}

			if ((pAd->MonitrCnt[BAND0] + pAd->MonitrCnt[BAND1]) == 0) {
				pAd->MntEnable = 0;
				NdisZeroMemory(&pAd->MntTable, sizeof(pAd->MntTable));
				NdisZeroMemory(&pAd->MntGroupTable, sizeof(pAd->MntGroupTable));
			}
		} else {
			if (band_idx == DBDC_BAND0) {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH, &u4SMESH);
				apply_mntr_ruleset(pAd, &u4SMESH);
				u4SMESH |= SMESH_ADDR_EN | u4SMESH;
				HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH, u4SMESH);
			} else if (band_idx == DBDC_BAND1) {
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH_B1, &u4SMESH);
				apply_mntr_ruleset(pAd, &u4SMESH);
				u4SMESH |= SMESH_ADDR_EN | u4SMESH;
				HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH_B1, u4SMESH);
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
						 __func__, band_idx));
				return FALSE;
			}

			pAd->MntEnable = 1;
		}
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- \n");
	return TRUE;
}


INT mtd_set_air_monitor_rule(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx)
{
	INT ret = TRUE;
	UINT32 u4SMESH = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--> %s()\n", __func__));

	if (!rule)
		return FALSE;

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

		HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH, &u4SMESH);
		u4SMESH &= ~SMESH_RX_DATA;
		u4SMESH &= ~SMESH_RX_MGT;
		u4SMESH &= ~SMESH_RX_CTL;
		u4SMESH |= (SMESH_RX_DATA & ((UINT32)rule[0] << SMESH_RX_DATA_OFFSET))
				   | (SMESH_RX_MGT & ((UINT32)rule[1] << SMESH_RX_MGT_OFFSET))
				   | (SMESH_RX_CTL & ((UINT32)rule[2] << SMESH_RX_CTL_OFFSET));
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
			u4SMESH &= ~SMESH_RX_A1;
		}
#endif
		HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH, u4SMESH);
	} else if (band_idx == DBDC_BAND1) {

		HW_IO_READ32(pAd->hdev_ctrl, RMAC_SMESH_B1, &u4SMESH);
		u4SMESH &= ~SMESH_RX_DATA;
		u4SMESH &= ~SMESH_RX_MGT;
		u4SMESH &= ~SMESH_RX_CTL;
		u4SMESH |= (SMESH_RX_DATA & ((UINT32)rule[0] << SMESH_RX_DATA_OFFSET))
				   | (SMESH_RX_MGT & ((UINT32)rule[1] << SMESH_RX_MGT_OFFSET))
				   | (SMESH_RX_CTL & ((UINT32)rule[2] << SMESH_RX_CTL_OFFSET));
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
			u4SMESH &= ~SMESH_RX_A1;
		}
#endif
		HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_SMESH_B1, u4SMESH);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():: wrong band index(%d)\n",
				 __func__, band_idx));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- %s()\n", __func__));
	return ret;
}

INT mtd_set_air_monitor_idx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx)
{
	INT ret = TRUE, i;
	UINT32 u4MAR0 = 0, u4MAR1 = 0, u4DBDC_CTRL = 0;
	UCHAR *p = ZERO_MAC_ADDR, muar_idx = 0, muar_group_base = 0;
	BOOLEAN bCreate = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	UCHAR mnt_enable = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> \n");

	if (mnt_idx < MAX_NUM_OF_MONITOR_STA) {
		pAd->MntIdx = mnt_idx;
		pMntEntry = &pAd->MntTable[mnt_idx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "The index is over the maximum limit.\n");
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
				pMuarGroup = &pAd->MntGroupTable[pMntEntry->muar_group_idx];

			if (pMuarGroup) {
				pMuarGroup->Count--;

				if (pMuarGroup->Count == 0)
					pMuarGroup->bValid = FALSE;
			}

			pMntEntry->bValid = FALSE;

			if (pAd->MonitrCnt[band_idx] > 0)
				pAd->MonitrCnt[band_idx]--;

			muar_idx = pMntEntry->muar_idx;
		} else
			return TRUE;

	} else {
		if (pMntEntry->bValid) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The index of existed monitor entry.\n");
			return TRUE;
		}

		if ((pAd->MonitrCnt[BAND0] + pAd->MonitrCnt[BAND1]) >= MAX_NUM_OF_MONITOR_STA) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The monitor number extends to maximum limit(%d).\n", MAX_NUM_OF_MONITOR_STA);
			return FALSE;
		}

		for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
			pMuarGroup = &pAd->MntGroupTable[i];

			if (!pMuarGroup->bValid) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMuarGroup->MuarGroupBase = MONITOR_MUAR_BASE_INDEX + i * MAX_NUM_PER_GROUP;
				pMuarGroup->bValid = TRUE;
				pMuarGroup->Band = band_idx;
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + pMuarGroup->Count++);
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			} else if ((pMuarGroup->Count < MAX_NUM_PER_GROUP) &&
					   (pMuarGroup->Band == band_idx)) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + pMuarGroup->Count++);
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

			HW_IO_READ32(pAd->hdev_ctrl, CFG_DBDC_CTRL1, &u4DBDC_CTRL);

			if (band_idx == BAND1)
				u4DBDC_CTRL |= (1 << (muar_group_base / MAX_NUM_PER_GROUP));
			else
				u4DBDC_CTRL &= ~(1 << (muar_group_base / MAX_NUM_PER_GROUP));

			HW_IO_WRITE32(pAd->hdev_ctrl, CFG_DBDC_CTRL1, u4DBDC_CTRL);

		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't create a monitor entry!\n");
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("index: %d\n", mnt_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("entry: "MACSTR"\n", MAC2STR(p)));
	u4MAR0 = ((UINT32)p[0]) | ((UINT32)p[1]) << 8 |
			 ((UINT32)p[2]) << 16 | ((UINT32)p[3]) << 24;
	u4MAR1 = ((UINT32)p[4]) | ((UINT32)p[5]) << 8;
	u4MAR1 |= MAR1_WRITE | MAR1_ACCESS_START_STATUS | (muar_idx << MAR1_ADDR_INDEX_OFFSET)
			  | (((UINT32)0 << MAR1_MAR_GROUP_OFFSET) & MAR1_MAR_GROUP_MASK);


	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR0, u4MAR0);
	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, u4MAR1);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("WriteCR 0x%08X:0x%08X 0x%08X:0x%08X\n", RMAC_MAR0, u4MAR0, RMAC_MAR1, u4MAR1));
	if (pAd->MonitrCnt[band_idx] > 0)
		mnt_enable = 1 ;

	ret = mtd_set_air_monitor_enable(pAd, mnt_enable, band_idx);



	do {
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, &u4MAR1);
	} while (u4MAR1 & MAR1_ACCESS_START_STATUS);

	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, &u4MAR0);

	p[0] = u4MAR0 & 0xFF;
	p[1] = (u4MAR0 & 0xFF00) >> 8;
	p[2] = (u4MAR0 & 0xFF0000) >> 16;
	p[3] = (u4MAR0 & 0xFF000000) >> 24;
	p[4] = u4MAR1 & 0xFF;
	p[5] = (u4MAR1 & 0xFF00) >> 8;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("entry: "MACSTR"\n", MAC2STR(p)));
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");

	return ret;
}


#endif

