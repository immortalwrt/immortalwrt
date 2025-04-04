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
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#include "rt_config.h"
#include "wlan_config/config_internal.h"

#include "hdev/hdev_basic.h"


/*----- 802.11H -----*/
/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT	i;
	BOOLEAN result = FALSE;

	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (Ch == pChCtrl->ChList[i].Channel) {
				result = pChCtrl->ChList[i].DfsReq;
#ifdef DFS_ADJ_BW_ZERO_WAIT
				if (!IS_ADJ_BW_ZERO_WAIT_TX80RX80(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) &&
					  IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) && IS_CH_BETWEEN(Ch, 36, 64))
					result = TRUE;
				else if (!IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)
					&& IS_CH_BETWEEN(Ch, 36, 64) && (pAd->CommonCfg.DfsParameter.band_bw[BandIdx] == BW_160 || pAd->CommonCfg.DfsParameter.OutBandBw == BW_160))
					result = TRUE;
#else
				if (pAd->CommonCfg.DfsParameter.band_bw[BandIdx] == BW_160 && IS_CH_BETWEEN(Ch, 36, 64))
					result = TRUE;
#endif
				break;
			}
		}
	}

	return result;
}

BOOLEAN RadarChannelCheckWrapper160(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT	i;
	BOOLEAN result = FALSE;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (Ch == pChCtrl->ChList[i].Channel) {
				if (Ch >= 100) {
					result = (pChCtrl->ChList[i].Flags & CHANNEL_160M_CAP) && pChCtrl->ChList[i].DfsReq;
					break;
				} else {
					if (pChCtrl->ChList[i].Flags & CHANNEL_160M_CAP) {
						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

/*
	========================================================================

	Routine Description:
		Get bw of outband

	Arguments:
		pAd	Pointer to our adapter

	Return Value:

	========================================================================
*/
BOOLEAN dfs_get_outband_bw(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PUCHAR phy_bw)
{
	struct wlan_config *cfg = NULL;

	if (wdev == NULL)
		return FALSE;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg == NULL)
		return FALSE;

	if (cfg->ht_conf.ht_bw == HT_BW_20)
		*phy_bw = BW_20;
	else if (cfg->ht_conf.ht_bw == HT_BW_40) {
		if (cfg->vht_conf.vht_bw == VHT_BW_2040)
			*phy_bw = BW_40;
		else if (cfg->vht_conf.vht_bw == VHT_BW_80)
			*phy_bw = BW_80;
		else if (cfg->vht_conf.vht_bw == VHT_BW_160)
			*phy_bw = BW_160;
		else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
			*phy_bw = BW_80;
		else
			;
	}
	return TRUE;

}

BOOLEAN CmmIsFirstBss(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	UINT_8 IdBss;
	struct wifi_dev *wdevTmp = NULL;
	BSS_STRUCT *pMbss = NULL;

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		pMbss = &pAd->ApCfg.MBSSID[IdBss];
		wdevTmp = &pMbss->wdev;
		if ((pMbss == NULL) || (wdev == NULL) || (wdev->pHObj == NULL))
			continue;

		if (wdev == wdevTmp)
			continue;

		if (HcGetBandByWdev(wdevTmp) != HcGetBandByWdev(wdev))
			continue;

		return FALSE;
	}

	return TRUE;
}



/*
	========================================================================

	Routine Description:
		Determine the current radar state

	Arguments:
		pAd	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarStateCheck(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;
	UCHAR vht_cent2 = 0;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg == NULL)
		return;

	if (cfg->ht_conf.ht_bw == HT_BW_20)
		phy_bw = BW_20;
	else if (cfg->ht_conf.ht_bw == HT_BW_40) {
		if (cfg->vht_conf.vht_bw == VHT_BW_2040)
			phy_bw = BW_40;
		else if (cfg->vht_conf.vht_bw == VHT_BW_80)
			phy_bw = BW_80;
		else if (cfg->vht_conf.vht_bw == VHT_BW_160)
			phy_bw = BW_160;
		else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
			phy_bw = BW_8080;
		else
			;
	}
	vht_cent2 = cfg->phy_conf.cen_ch_2;

	if (wdev->csa_count != 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"Wdev->csa_count(%d) != 0 \n", wdev->csa_count);
		return;
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "Zero Wait State: %d\n",
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitState);
#endif

#ifdef MT_DFS_SUPPORT
	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		DfsRadarChannelCheck(pAd, wdev, vht_cent2, phy_bw)
#ifdef DFS_ADJ_BW_ZERO_WAIT
		&& (IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) == FALSE)
#endif
	) {
#ifdef MAP_R2
		if (IS_MAP_TURNKEY_ENABLE(pAd)
#ifdef DFS_ZEROWAIT_SUPPORT
			|| pAd->ApCfg.bChSwitchNoCac
#endif
			) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m cac_not_req %d \x1b[m \n", wdev->cac_not_required);
			if (wdev->cac_not_required == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m switch back to RD_NORMAL_MODE \x1b[m \n");
				/* DFS Zero wait case, OP CH always is normal mode */
				pDot11h->RDMode = RD_NORMAL_MODE;
				return;
			}
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m RD_SILENCE_MODE \x1b[m \n");

#else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33mRD_SILENCE_MODE \x1b[m \n");
#endif
		pDot11h->RDMode = RD_SILENCE_MODE;
		if (CmmIsFirstBss(pAd, wdev))
			pDot11h->RDCount = 0;
		pDot11h->InServiceMonitorCount = 0;
		if (DfsIsOutBandAvailable(pAd, wdev)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m OutBand Available. Set into RD_NORMAL_MODE \x1b[m \n");
			pDot11h->RDMode = RD_NORMAL_MODE;
		} else if (DfsIsTargetChAvailable(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m Target Channel Bypass CAC. Set into RD_NORMAL_MODE \x1b[m \n");
			pDot11h->RDMode = RD_NORMAL_MODE;

		} else
			;
	} else
#endif
	{
		if (WMODE_CAP_5G(wdev->PhyMode))
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "\x1b[1;33m RD_NORMAL_MODE \x1b[m \n");

		/* DFS Zero wait case, OP CH always is normal mode */
		pDot11h->RDMode = RD_NORMAL_MODE;
	}
}

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR ch)
{
	INT i;
	BOOLEAN InNOP = FALSE;
	UCHAR channel = 0;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = NULL;

	if (pAd->CommonCfg.dbdc_mode)
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	else
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BAND0);

	if (ch == RDD_CHECK_NOP_BY_WDEV)
		channel = wdev->channel;
	else
		channel = ch;

#ifdef MT_DFS_SUPPORT
	/*DfsNonOccupancyUpdate(pAd);*/
#endif

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel) {
			if ((pChCtrl->ChList[i].NonOccupancy > 0) || (pChCtrl->ChList[i].NOPSaveForClear > 0)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
						 "ERROR: previous detection of a radar on this channel(Channel=%d).\n",
						  pChCtrl->ChList[i].Channel);
				InNOP = TRUE;
				break;
			}
		}
	}

	if ((InNOP == FALSE)
#ifdef MT_DFS_SUPPORT
		|| DfsStopWifiCheck(pAd, wdev)
#endif
	)
		return TRUE;
	else
		return FALSE;

}

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	UCHAR Channel5G = HcGetChannelByRf(pAd, RFIC_5GHZ);

	if (pAd->CommonCfg.RDDurRegion != JAP)
		return pAd->CommonCfg.RDDurRegion;

	for (i = 0; i < 15; i++) {
		if (IsSupport5G && Channel5G ==  Channel[i])
			break;
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/
}


UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode,
	IN struct wifi_dev *wdev)
{
	UCHAR ch = 0;
	INT ch_idx;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

#ifdef MT_DFS_SUPPORT
	/*DfsNonOccupancyUpdate(pAd);*/
#endif

	switch (mode) {
	case 1: {
		USHORT min_time = 0xFFFF;

		/* select channel with least NonOccupancy */
		for (ch_idx = 0; ch_idx <  pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].NonOccupancy < min_time) {
				min_time = pChCtrl->ChList[ch_idx].NonOccupancy;
				ch = pChCtrl->ChList[ch_idx].Channel;
			}
		}

		break;
	}

	default: {
		ch = FirstChannel(pAd, wdev);
		break;
	}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "mode = %u, ch = %u\n",
			 mode, ch);
	return ch;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Channel switching count down process upon radar detection

	Arguments:
		pAd	Pointer to our adapter

	========================================================================
*/
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev)
{
	UCHAR apIdx = 0xff;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "wdev %d not ready !!!\n",
				 wdev->wdev_idx);
		return;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Wdev(%s) Channel Switching...(%d/%d)\n",
			 wdev->if_dev->name, pDot11h->CSCount, pDot11h->CSPeriod);
	pDot11h->CSCount++;

	if (pDot11h->CSCount >= pDot11h->CSPeriod) {
		if (wdev && (wdev->wdev_type == WDEV_TYPE_AP))
			apIdx = wdev->func_idx;

		/* CSA done */
		if (apIdx > 31) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Left shifting by more than 31 bits\n");
			return;
		}
		pDot11h->csa_ap_bitmap &= ~(UINT32)(1 << apIdx);
		if (pDot11h->wdev_count > 0)
			pDot11h->wdev_count--;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				 "Type = %d, func_idx = %d, wdev_count= %d, CSCount = %d, csa_ap_bitmap = 0x%x\n",
				 wdev->wdev_type, wdev->func_idx, pDot11h->wdev_count, pDot11h->CSCount, pDot11h->csa_ap_bitmap);

		/* do channel switch only when all BSS done */
		if (pDot11h->csa_ap_bitmap == 0) {
			BOOLEAN Cancelled;
			if (timer_pending(&pDot11h->CSAEventTimer.TimerObj)) {
				RTMPCancelTimer(&pDot11h->CSAEventTimer, &Cancelled);
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, " CSAEventTimer normal end!!\n");
			}
#ifdef ZERO_LOSS_CSA_SUPPORT
			if (pAd->Zero_Loss_Enable) {
				UCHAR BandIdx = HcGetBandByWdev(wdev);

				pDot11h->CSA0EventApidx = apIdx;
				pDot11h->ChnlSwitchState = CHANNEL_SWITCH_COUNT_ZERO_EVENT;
				pAd->chan_switch_time[1] = jiffies_to_msecs(jiffies);
				/*Disable zero loss sta traffic*/
				RTEnqueueInternalCmd(pAd, CMDTHREAD_DISABLE_ZERO_LOSS_STA_TRAFFIC, &(BandIdx), sizeof(UCHAR));
				/*EXT_EVENT_CSA_NOTIFY event comes from f/w on PreTBTT
				  *while TBTT - PreTBTT ~20-30ms
				  *wait for last CSA Beacon Tx event before switching channel
				*/
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_WARN,
							"Type = %d, func_idx = %d, csa_ap_bitmap = 0x%x, set CSALastBcnTxEventTimer\n",
							wdev->wdev_type, wdev->func_idx, pDot11h->csa_ap_bitmap);
				RTMPSetTimer(&pDot11h->CSALastBcnTxEventTimer, 30);
			} else
#endif /*ZERO_LOSS_CSA_SUPPORT*/
			RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &apIdx, sizeof(UCHAR));
		}
	}
}

NTSTATUS DropRadarEventHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR BandIdx = DBDC_BAND0;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "After Dropping Radar event, enable DFS tx start\n");
	NdisMoveMemory(&BandIdx, CMDQelmt->buffer, sizeof(UCHAR));
	MtCmdSetDfsTxStart(pAd, BandIdx);
	return 0;
}

/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR apIdx, i;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR apOper = AP_BSS_OPER_ALL;
	struct DOT11_H *pDot11h = NULL;
#ifdef ZERO_LOSS_CSA_SUPPORT
	int j;
	UCHAR WcidCount = 0;
	UINT16 WcidList[3] = {0};
	ktime_t chnl_switch_init, chnl_switch_exit;
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	UCHAR BandIdx = DBDC_BAND0;
	struct wifi_dev *wdev;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	BOOLEAN isRadarCh = FALSE;
#ifdef OFFCHANNEL_SCAN_FEATURE
	OFFCHANNEL_SCAN_MSG Rsp;
	UCHAR RfIC = 0;
#endif
#ifdef CONFIG_MAP_SUPPORT
	int u = 0;
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	struct wlan_config *cfg;
#endif


	NdisMoveMemory(&apIdx, CMDQelmt->buffer, sizeof(UCHAR));

	/* check apidx valid */
	if (apIdx != 0xff) {
		pMbss = &pAd->ApCfg.MBSSID[apIdx];
		apOper = AP_BSS_OPER_BY_RF;
	}

	if (pMbss == NULL)
		goto end;

	pDot11h = pMbss->wdev.pDot11_H;
	if (pDot11h == NULL)
		goto end;

	wdev = &pMbss->wdev;
	BandIdx = HcGetBandByWdev(wdev);
#ifdef OFFCHANNEL_SCAN_FEATURE
	Rsp.Action = DRIVER_CHANNEL_SWITCH_SUCCESSFUL;
	memcpy(Rsp.ifrn_name, pAd->ScanCtrl[BandIdx].if_name, IFNAMSIZ);
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	cfg = (struct wlan_config *)wdev->wpf_cfg;
#endif

	/* Normal DFS */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DedicatedZeroWaitStop(pAd, FALSE);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	pAd->chan_switch_time[3] = jiffies_to_msecs(jiffies);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "[RDM]\x1b[1;33m Change to RD_SILENCE_MODE\x1b[m\n");
	pDot11h->RDMode = RD_SILENCE_MODE;
	if (pMbss->wdev.channel <= 14)
		pAd->CommonCfg.ChannelSwitchFor2G.CHSWMode = NORMAL_MODE;

#ifdef DOT11W_PMF_SUPPORT
	if (pMbss->wdev.quick_ch_change && pMbss->wdev.SecConfig.ocv_support)
		ap_set_wait_sa_query_for_csa(pAd, &pMbss->wdev);
#endif

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "wdev name(%s), quick %d ,ch %d\n",
	(char *)pMbss->wdev.if_dev->name, pMbss->wdev.quick_ch_change, pMbss->wdev.channel);

#ifdef CONFIG_MAP_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "cacreq %d\n", pMbss->wdev.cac_not_required);
#endif
	isRadarCh = RadarChannelCheck(pAd, pMbss->wdev.channel);
	if (pMbss->wdev.quick_ch_change != QUICK_CH_SWICH_DISABLE
#ifdef CONFIG_MAP_SUPPORT
	|| (IS_MAP_ENABLE(pAd) && isRadarCh && pMbss->wdev.cac_not_required)
#endif
		) {
		struct wifi_dev *tdev = NULL;
		/* For DFS certification, After CSA done and Disable Beacon*/
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tdev = pAd->wdev_list[i];
			if (tdev
				&& HcIsRadioAcq(tdev)
				&& (BandIdx == HcGetBandByWdev(tdev))
				&& tdev->wdev_type == WDEV_TYPE_AP) {
				if (WMODE_CAP_5G(tdev->PhyMode) && (pAd->CommonCfg.bIEEE80211H == TRUE)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATCHN_DFS, DBG_LVL_NOTICE,
					"CSA done and Disable Beacon: %s\n", (char *)tdev->if_dev->name);
					UpdateBeaconHandler(pAd, tdev, BCN_UPDATE_DISABLE_TX);
				}
			}
		}
#ifdef ZERO_LOSS_CSA_SUPPORT
		pAd->chan_switch_time[4] = jiffies_to_msecs(jiffies);
#ifdef PROPRIETARY_DRIVER_SUPPORT
		{
			struct timespec64 kts64 = {0};

			ktime_get_real_ts64(&kts64);
			chnl_switch_init = timespec64_to_ktime(kts64);
		}
#else
		chnl_switch_init = ktime_get();
#endif /* PROPRIETARY_DRIVER_SUPPORT */

#endif /*#ZERO_LOSS_CSA_SUPPORT*/

		/* Update channel of wdev as new channel */
		ap_phy_rrm_init_byRf(pAd, wdev);
#ifdef ZERO_LOSS_CSA_SUPPORT
#ifdef PROPRIETARY_DRIVER_SUPPORT
		{
			struct timespec64 kts64 = {0};

			ktime_get_real_ts64(&kts64);
			chnl_switch_exit = timespec64_to_ktime(kts64);
		}
#else
		chnl_switch_exit = ktime_get();
#endif /* PROPRIETARY_DRIVER_SUPPORT */


		if (pAd->Zero_Loss_Enable) {
			/*set ChnlSwitchState = ASIC_CHANNEL_SWITCH_COMMAND_ISSUED*/
			pDot11h->ChnlSwitchState = ASIC_CHANNEL_SWITCH_COMMAND_ISSUED;
			/* CSA SYNC / TSF SYNC */
			if (pDot11h->RootApCSCountFlag) {
				pDot11h->RootApCSCountFlag = FALSE;
				pDot11h->CSPeriod = pDot11h->OriCSCount;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Original CSA count restored - %d\n", pDot11h->CSPeriod);

			}

			/*channel switched, send null frame sending request to fw*/
			for (j = 0; j < 3; j++) {
				if (pAd->ZeroLossSta[j].valid) {
					if ((pAd->ZeroLossSta[j].wcid) && (pAd->ZeroLossSta[j].band == BandIdx)) {
						WcidList[WcidCount] = pAd->ZeroLossSta[j].wcid;
						WcidCount++;
					}
				}
			}

			/*send null frame to zero loss sta, and start null ack failsafe timer*/
			if (WcidCount)
				MtCmdSetChkPeerLink(pAd, WcidCount, (UINT8 *)WcidList);

			pAd->chan_switch_time[14] = jiffies_to_msecs(jiffies);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"set sta skip tx reset timer for %dms\n", pAd->ucSTATimeout);

			//skip tx failsafe:: set timer=500ms, if skip_tx still set, reset it
			RTMPSetTimer(&pDot11h->ChnlSwitchStaNullAckWaitTimer, pAd->ucSTATimeout);
		}

		MTWF_DBG(pAd, DBG_CAT_MLME, CATCHN_DFS, DBG_LVL_NOTICE,
					"channel switch diff: %d msec\n", ktime_to_ms(ktime_sub(chnl_switch_exit, chnl_switch_init)));
#endif /*ZERO_LOSS_CSA_SUPPORT*/
		/* if zero-wait cac is ended, dedicated rx switch to new dfs ch */
		DfsBuildChannelList(pAd, wdev);
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
		zero_wait_dfs_switch_ch(pAd, wdev, RDD_DEDICATED_RX);
#endif

#ifdef CONFIG_MAP_SUPPORT

		pMbss->wdev.cac_not_required = FALSE;

		for (u = 0; u < pAd->ApCfg.BssidNum; u++) {
			struct wifi_dev *wdev_temp = NULL;
			wdev_temp = &pAd->ApCfg.MBSSID[u].wdev;
			if (wdev_temp->cac_not_required == TRUE) {
				wdev_temp->cac_not_required = FALSE;
			}
			/*need to make all the MBSS cac not required false*/
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "if: %s is_cac_requested: %d\n",
							(char *)wdev_temp->if_dev->name,
							wdev_temp->cac_not_required);
		}
#endif
	} else

	{
		APStop(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
		if (DfsStopWifiCheck(pAd, &pMbss->wdev)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "Stop AP Startup\n");
			goto end;
		}
#endif
		APStartUp(pAd, pMbss, apOper);
	}
#ifdef MT_DFS_SUPPORT
	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, HcGetBandByWdev(&pMbss->wdev));
	} else {
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (cfg)
			if (cfg->vht_conf.vht_bw == VHT_BW_8080)
				MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
#endif
	}
	DfsSetCacRemainingTime(pAd, &pMbss->wdev);
	DfsReportCollision(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT
	/*DfsDedicatedScanStart(pAd);*/
#endif
#endif
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd)
#ifdef DFS_ZEROWAIT_SUPPORT
		|| pAd->ApCfg.bChSwitchNoCac
#endif
		) {
		if (pMbss->wdev.cac_not_required) {
			pMbss->wdev.cac_not_required = FALSE;
			pDot11h->RDCount = pDot11h->cac_time;
			pDot11h->cac_not_required = TRUE;
		}
		if (pDot11h->cac_not_required) {
			int i = 0;
			struct wifi_dev *wdev_temp = NULL;
			BOOLEAN can_reset_cac = TRUE;

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev_temp = &pAd->ApCfg.MBSSID[i].wdev;
				if ((wdev_temp->pDot11_H == pDot11h) &&
					wdev_temp->cac_not_required) {
						can_reset_cac = FALSE;
						break;
				}
			}
			if (can_reset_cac) {
				pDot11h->RDCount = pDot11h->cac_time;
				pDot11h->cac_not_required = FALSE;
				if (IS_MAP_TURNKEY_ENABLE(pAd)) {
					int j;
					for (j = 0; j < MAX_APCLI_NUM; j++) {
						wdev_temp = &pAd->StaCfg[j].wdev;
						if ((wdev_temp->pDot11_H == pDot11h)
						&& (wdev_temp->WscControl.WscConfMode != WSC_DISABLE)
						&& (wdev_temp->WscControl.bWscTrigger)) {
							pAd->StaCfg[j].ApcliInfStat.Enable = TRUE;
							break;
						}
					}
				}
			}
		}
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE, "cac not req %d\n", pDot11h->cac_not_required);

#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
			RfIC = (WMODE_CAP_5G(pMbss->wdev.PhyMode)) ? RFIC_5GHZ : RFIC_24GHZ;
			Rsp.data.operating_ch_info.channel = HcGetChannelByRf(pAd, RfIC);
			Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
			Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
			Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
			Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
			if ((pAd->Antenna.field.TxPath == 4) && (pAd->Antenna.field.RxPath == 4))
				Rsp.data.operating_ch_info.is4x4Mode = 1;
			else
				Rsp.data.operating_ch_info.is4x4Mode = 0;

#ifdef ANTENNA_CONTROL_SUPPORT
			{
				if (pAd->bAntennaSetAPEnable[BandIdx]) {
					if ((pAd->TxStream[BandIdx] == 4) &&
						(pAd->RxStream[BandIdx] == 4))
						Rsp.data.operating_ch_info.is4x4Mode = 1;
					else
						Rsp.data.operating_ch_info.is4x4Mode = 0;
				}
			}
#endif /* ANTENNA_CONTROL_SUPPORT */

			RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_OFFCHANNEL_INFO,
					NULL,
					(UCHAR *) &Rsp,
					sizeof(OFFCHANNEL_SCAN_MSG));
#endif
end:
	pAd->ApCfg.set_ch_async_flag = FALSE;
#ifdef DFS_ZEROWAIT_SUPPORT
	if (pAd->ApCfg.bChSwitchNoCac == 1)
		pAd->ApCfg.bChSwitchNoCac = 0;
#endif
	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	if (pAutoChCtrl) {
		if (pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN)
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
	}
	if (pAd->ApCfg.iwpriv_event_flag) {
		RTMP_OS_COMPLETE(&pAd->ApCfg.set_ch_aync_done);
	} else {
		/*for some modules without asynchronous mode, should release ChannelOpCharge when CSA done */
		ReleaseChannelOpChargeForCurrentOwner(pAd, &pMbss->wdev);
	}
	return 0;
}

VOID CSAEventTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UCHAR i = 0, apIdx = 0;
	UCHAR band_idx = DBDC_BAND0;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = NULL;
	struct radio_dev *rdev = (struct radio_dev *)FunctionContext;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	band_idx = rdev->pRadioCtrl->BandIdx;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"CSAEventTimer abnormal end, Start Error handle!!!\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"band=%d, wdev_counter=%d, csa_ap_bitmap=0x%x\n",
			band_idx, pAd->Dot11_H[band_idx].wdev_count, pAd->Dot11_H[band_idx].csa_ap_bitmap);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev == NULL || !WDEV_WITH_BCN_ABILITY(wdev))
			continue;
		if (wdev->pDot11_H == NULL || wdev->if_up_down_state == FALSE)
			continue;
		if (band_idx != HcGetBandByWdev(wdev))
			continue;

		pDot11h = wdev->pDot11_H;
		pDot11h->CSCount = pDot11h->CSPeriod;
		pDot11h->CSCount++;
		wdev->csa_count = 0; /*csa count reset*/
		apIdx = wdev->func_idx;
		/* for csa recover */
		if (apIdx > 31) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "Left shifting by more than 31 bits\n");
			return;
		}
		if (pDot11h->csa_ap_bitmap > 0)
			pDot11h->csa_ap_bitmap &= ~(UINT32)(1 << apIdx);
		if (pDot11h->wdev_count > 0)
			pDot11h->wdev_count--;

		if (pDot11h->csa_ap_bitmap == 0)
			break;
	}

	pDot11h = &pAd->Dot11_H[band_idx];
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"\x1b[41m pAd->Dot11_H[%d] is NULL !!\x1b[m\n", band_idx);
		return;
	}
	if (pDot11h->csa_ap_bitmap != 0 || pDot11h->wdev_count != 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN, "CSA Event Timeout Reset error:counter(%d),0x%x\n",
			pDot11h->wdev_count, pDot11h->csa_ap_bitmap);
		if (pDot11h->wdev_count != 0)
			pDot11h->wdev_count = 0;
		if (pDot11h->csa_ap_bitmap != 0)
			pDot11h->csa_ap_bitmap = 0;
	}

	if (wdev != NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
					 "name = %s, func_idx = %d, wdev_count = %d, CSCount = %d\n",
					 wdev->if_dev->name, wdev->func_idx, pDot11h->wdev_count, pDot11h->CSCount);
		if (RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &wdev->func_idx, sizeof(UCHAR)) == NDIS_STATUS_FAILURE)
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "<---Enqueue CSA Cmd Fail !!!\n");
	}
}

#ifdef ZERO_LOSS_CSA_SUPPORT
/*
	==========================================================================
	Description:
		last 'CSA count=0" Bcn Sent Event Timeout handler, executed in timer thread
	==========================================================================
 */
VOID CSALastBcnTxEventTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct radio_dev *rdev = (struct radio_dev *)FunctionContext;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[%s]: All Bcns with CS Count0 sent for band:%d\n", rdev->pRadioCtrl->BandIdx);
	RTEnqueueInternalCmd(pAd, CMDTHREAD_LAST_BCN_TX_SWITCH_CHANNEL, &(rdev->pRadioCtrl->BandIdx), sizeof(UCHAR));
}

/*
    ==========================================================================
	Description:
	after channel switch, wait for "NULL ack from sta" timed out, executed in timer thread
    ==========================================================================
 */
VOID ChnlSwitchStaNullAckWaitTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct radio_dev *rdev = (struct radio_dev *)FunctionContext;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"Connected Stations Channel Switch wait: Timed out \n");

	pAd->chan_switch_time[16] = jiffies_to_msecs(jiffies);

	HANDLE_STA_NULL_ACK_TIMEOUT(pAd, rdev->pRadioCtrl->BandIdx);
}

NTSTATUS LastBcnTxChannelSwitch(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR BandIdx = 0xff;			/*set to some invalid value by default*/

	NdisMoveMemory(&BandIdx, CMDQelmt->buffer, sizeof(UCHAR));
	RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &(pAd->Dot11_H[BandIdx].CSA0EventApidx), sizeof(UCHAR));
	return 0;
}

NTSTATUS DisableZeroLossStaTraffic(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	int i = 0;
	UCHAR BandIdx = 0xff;			/*set to some invalid value by default*/

	NdisMoveMemory(&BandIdx, CMDQelmt->buffer, sizeof(UCHAR));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"set sta skip tx, BandIdx:%d\n", BandIdx);
	for (i = 0; i < 3; i++)	{
		MAC_TABLE_ENTRY *pEntry = NULL;
		struct wifi_dev *wdev = NULL;

		if (pAd->ZeroLossSta[i].valid) {
			pEntry = MacTableLookup(pAd, pAd->ZeroLossSta[i].StaAddr);
			/*If entry was added before connection or sta roamed, update wcid/band */
			if (pEntry) {
				pAd->ZeroLossSta[i].wcid = pEntry->wcid;
				wdev = &(pEntry->pMbss->wdev);
				pAd->ZeroLossSta[i].band = HcGetBandByWdev(wdev);
				if ((pAd->ZeroLossSta[i].wcid) && (pAd->ZeroLossSta[i].band == BandIdx)) {
					if (!AsicReadSkipTx(pAd, pAd->ZeroLossSta[i].wcid)) {
						AsicUpdateSkipTx(pAd, pAd->ZeroLossSta[i].wcid, 1);
						pAd->ZeroLossSta[i].suspend_time = jiffies_to_msecs(jiffies);
						pAd->ZeroLossSta[i].ChnlSwitchSkipTx = 1;
						/*increase PS queue for sta to avoid pkt drop */
						MtCmdStaPsQLimit(pAd, pAd->ZeroLossSta[i].wcid, pAd->ZeroLossStaPsQLimit);
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
										"Tx Disabled for "MACSTR" band:%d\n",
										MAC2STR(pAd->ZeroLossSta[i].StaAddr), pAd->ZeroLossSta[i].band);
					} else
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"%s:Tx Disable already set for "MACSTR"\n",
							MAC2STR(pAd->ZeroLossSta[i].StaAddr));
				} else
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								"%s:pEntry not found for "MACSTR"\n",
								MAC2STR(pAd->ZeroLossSta[i].StaAddr));
			} else {
				/*Make wcid = 0, if zero loss entry is available
				 * but currently not connected*/
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"%s:pEntry not found in Connected stations for "MACSTR"\n",
					MAC2STR(pAd->ZeroLossSta[i].StaAddr));
				pAd->ZeroLossSta[i].wcid = 0;
			}
		}
	}
	return 0;
}

INT Show_ChannelSwitchTime_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg)
{
	int i = 0;

	MTWF_PRINT("CSA Beacon Update Time:%lu msec\n", pAd->chan_switch_time[0]);
	MTWF_PRINT("PreTBTT Event for CSALast :%lu msec\n", pAd->chan_switch_time[1]);
	MTWF_PRINT("Last CSA Bcn sent Event rcvd:%lu msec\n", pAd->chan_switch_time[2]);
	MTWF_PRINT("Dot11HCntDownTimeoutAction enter:%lu msec\n", pAd->chan_switch_time[3]);
	MTWF_PRINT("after UpdateBcnHndlr to disable Bcn, before ap_phy_rrm_init:%lu msec\n", pAd->chan_switch_time[4]);
	MTWF_PRINT(" ap_phy_rrm_init_byRf after udelay:%lu msec\n", pAd->chan_switch_time[5]);
	//MTWF_PRINT("in wlan_oper_init after init config:%lu msec\n", pAd->chan_switch_time[6]);
	MTWF_PRINT("before HcSuspendMSDUTx in hcRadioUpdate:%lu msec\n", pAd->chan_switch_time[7]);
	MTWF_PRINT("before asic switch channel:%lu msec\n", pAd->chan_switch_time[8]);
	MTWF_PRINT("before MtCmdChannelSwitch :%lu msec\n", pAd->chan_switch_time[9]);
	MTWF_PRINT("after MtCmdChannelSwitch :%lu msec\n", pAd->chan_switch_time[10]);
	MTWF_PRINT("after MtCmdSetTxRxPath :%lu msec\n", pAd->chan_switch_time[11]);
	MTWF_PRINT("after AsicSetBW :%lu msec\n", pAd->chan_switch_time[12]);
	MTWF_PRINT("after HcUpdateMSDUTxAllow :%lu msec\n", pAd->chan_switch_time[13]);
	MTWF_PRINT("after null frame send to sta request to fw :%lu msec\n", pAd->chan_switch_time[14]);
	MTWF_PRINT("after first sta null ack event/VHT action :%lu msec\n", pAd->chan_switch_time[15]);
	MTWF_PRINT("after sta null ack wait timeout :%lu msec\n", pAd->chan_switch_time[16]);
	for (i = 0; i < 17; i++)
		pAd->chan_switch_time[i] = 0;
	return TRUE;
}

INT Show_PerSTA_StopTx_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;

	MTWF_PRINT("ZeroLossStaCount:%d\n", pAd->ZeroLossStaCount);

	MTWF_PRINT("\n%-25s%-20s%-20s%-20s%-20s\n",
				"MAC", "WCID", "Suspend-Time", "Resume-Time", "Diff(ms)");

	for (i = 0; i < 3; i++) {
		MTWF_PRINT("%02X:%02X:%02X:%02X:%02X:%02X          ",
					PRINT_MAC(pAd->ZeroLossSta[i].StaAddr));
		MTWF_PRINT("%-20d", pAd->ZeroLossSta[i].wcid);
		MTWF_PRINT("%-20lu", pAd->ZeroLossSta[i].suspend_time);
		MTWF_PRINT("%-20lu", pAd->ZeroLossSta[i].resume_time);
		MTWF_PRINT("%-20lu\n", (pAd->ZeroLossSta[i].resume_time - pAd->ZeroLossSta[i].suspend_time));
	}
	return TRUE;
}

INT	Show_ZeroLossStaList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0;

	MTWF_PRINT("Current zero loss sta list::\n");
	for (i = 0; i < 3; i++) {
		MTWF_PRINT("index %d: valid:%d, wcid:%d band:%d addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
				i, pAd->ZeroLossSta[i].valid, pAd->ZeroLossSta[i].wcid, pAd->ZeroLossSta[i].band,
				PRINT_MAC(pAd->ZeroLossSta[i].StaAddr));
	}
	return TRUE;
}

INT	Set_STA_Tx_Unblock_Timeout_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	pAd->ucSTATimeout = (USHORT) simple_strtol(arg, 0, 10);

	MTWF_PRINT("Set_STA_TX_Unblock_Timeout_Proc::(Time = %d)\n", pAd->ucSTATimeout);

	return TRUE;
}

INT	Set_CHSWPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	int i;
	UCHAR CHSWPeriod = 0;

	CHSWPeriod = (USHORT) simple_strtol(arg, 0, 10);

	/*todo: support separate Channel Switch Period per band*/
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->Dot11_H[i].CSPeriod = CHSWPeriod;

	MTWF_PRINT("%s::(CHSWPeriod=%d) \n", __func__, CHSWPeriod);

	return TRUE;
}

INT	Set_ZeroLossStaAdd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev = NULL;

	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
			return FALSE;  /*Invalid */
		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	pEntry = MacTableLookup(pAd, macAddr);

	for (i = 0; i < 3; i++) {
		if (pAd->ZeroLossSta[i].valid) {
			if (NdisCmpMemory(pAd->ZeroLossSta[i].StaAddr, macAddr, MAC_ADDR_LEN) == 0) {
				MTWF_PRINT("Zero Loss Sta already exist: index:%d wcid:%d\n", i, pAd->ZeroLossSta[i].wcid);
				return TRUE;
			}
		}
	}

	for (i = 0; i < 3; i++) {
		if (pAd->ZeroLossSta[i].valid == 0) {
			NdisCopyMemory(pAd->ZeroLossSta[i].StaAddr, macAddr, MAC_ADDR_LEN);
			pAd->ZeroLossSta[i].valid = 1;
			if (pEntry) {
				pAd->ZeroLossSta[i].wcid = pEntry->wcid;
				wdev = &(pEntry->pMbss->wdev);
				pAd->ZeroLossSta[i].band = HcGetBandByWdev(wdev);
			}
			pAd->ZeroLossStaCount++;
			break;
		}
	}
	if (i >= 3)
		MTWF_PRINT("FAIL:::Zero Loss Sta list full, remove any existing station\n");
	else
		MTWF_PRINT("Zero Loss Sta added: index:%d wcid:%d\n", i, pAd->ZeroLossSta[i].wcid);

	MTWF_PRINT("current zero sta list::\n");
	for (i = 0; i < 3; i++) {
		MTWF_PRINT("index %d: valid:%d, wcid:%d addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
						i, pAd->ZeroLossSta[i].valid, pAd->ZeroLossSta[i].wcid, PRINT_MAC(pAd->ZeroLossSta[i].StaAddr));
	}
	return TRUE;
}

INT	Set_ZeroLossStaRemove_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT i = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	pEntry = MacTableLookup(pAd, macAddr);

	for (i = 0; i < 3; i++) {
		if (pAd->ZeroLossSta[i].valid) {
			if (NdisCmpMemory(pAd->ZeroLossSta[i].StaAddr, macAddr, MAC_ADDR_LEN) == 0) {
				//restore default PS queue limit for station
				MtCmdStaPsQLimit(pAd, pAd->ZeroLossSta[i].wcid, 0);
				pAd->ZeroLossSta[i].valid = 0;
				pAd->ZeroLossSta[i].wcid = 0;
				pAd->ZeroLossSta[i].band = 0;
				NdisZeroMemory(pAd->ZeroLossSta[i].StaAddr, MAC_ADDR_LEN);
				pAd->ZeroLossStaCount--;
				break;
			}
		}
	}

	if (i >= 3)
		MTWF_PRINT("FAIL:::station not found\n");
	else
		MTWF_PRINT("Zero Loss Sta removed, remaining zeroloss sta count:%d\n", pAd->ZeroLossStaCount);

	MTWF_PRINT("current zero sta list::\n");
	for (i = 0; i < 3; i++) {
		MTWF_PRINT("index %d: valid:%d wcid:%d addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
						i, pAd->ZeroLossSta[i].valid, pAd->ZeroLossSta[i].wcid, PRINT_MAC(pAd->ZeroLossSta[i].StaAddr));
	}
	return TRUE;
}
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */




/*
    ==========================================================================
    Description:
	Set channel switch Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->Dot11_H[0].CSPeriod = (USHORT) os_str_tol(arg, 0, 10);
	MTWF_PRINT("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H[0].CSPeriod);
	return TRUE;
}

/*
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	USHORT Value;
	UCHAR band_idx;

	Value = (USHORT) os_str_tol(arg, 0, 10);

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pAd->Dot11_H[band_idx].cac_time = Value;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "band index: %d, cac_time: %d\n",
			band_idx,
			pAd->Dot11_H[band_idx].cac_time);
	}
	return TRUE;
}


/*
    ==========================================================================
    Description:
		Reset channel block status.
	Arguments:
	    pAd				Pointer to our adapter
	    arg				Not used

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	INT i;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO, "Reset channel block status.\n");

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++)
			pChCtrl->ChList[i].NonOccupancy = 0;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Initialize the pDot11H of wdev

    Parameters:

    return:
    ==========================================================================
 */
VOID UpdateDot11hForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN attach)
{
	UCHAR bandIdx = 0;
	struct DOT11_H *pDot11h = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR, "no wdev!\n");
		return;
	}

	if (attach) {
		bandIdx = HcGetBandByWdev(wdev);
		wdev->pDot11_H = &pAd->Dot11_H[bandIdx];
	} else {
		pDot11h = wdev->pDot11_H;
		if (pDot11h && pDot11h->csa_ap_bitmap) {
			pDot11h->csa_ap_bitmap &= ~(UINT32)(1 << wdev->func_idx);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
				"Clear csa_ap_bitmap = 0x%x\n", pDot11h->csa_ap_bitmap);
		}
		wdev->pDot11_H = NULL;
		wdev->csa_count = 0;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		wdev->OmacIdx = 0xff;
#endif
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_INFO,
			"Detach wdev=%d_Dot11_H\n", wdev->wdev_idx);
	}
}

#ifdef MT_DFS_SUPPORT
INT set_radar_min_lpn_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 min_lpn_update = simple_strtol(arg, 0, 10);

	if (min_lpn_update <= LPB_SIZE) {
		MTWF_PRINT("LPN Update to %d \n", min_lpn_update);

		pAd->CommonCfg.DfsParameter.fcc_lpn_min = min_lpn_update;
		mt_cmd_set_fcc5_min_lpn(pAd, min_lpn_update);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
			"Invalid LPN value %d, please set in range 0 to 32\n", min_lpn_update);
	}
	return TRUE;
}

INT set_radar_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CMD_RDM_RADAR_THRESHOLD_UPDATE_T RadarThreshold = {0};
	PSW_RADAR_TYPE_T sw_radar_type = NULL;
	INT32 recv = 0;
	UINT32 radar_type_idx = 0;
	UINT32 rt_en = 0, rt_stgr = 0;
	UINT32 rt_crpn_min = 0, rt_crpn_max = 0, rt_crpr_min = 0;
	UINT32 rt_pw_min = 0, rt_pw_max = 0;
	UINT32 rt_crbn_min = 0, rt_crbn_max = 0;
	UINT32 rt_stg_pn_min = 0, rt_stg_pn_max = 0, rt_stg_pr_min = 0;
	UINT32 rt_pri_min = 0, rt_pri_max = 0;
	UINT32 rt_stg_pri_diff_min = 0;

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
						&(radar_type_idx), &(rt_en), &(rt_stgr), &(rt_crpn_min),
						&(rt_crpn_max), &(rt_crpr_min), &(rt_pw_min), &(rt_pw_max),
						&(rt_pri_min), &(rt_pri_max), &(rt_crbn_min), &(rt_crbn_max),
						&(rt_stg_pn_min), &(rt_stg_pn_max), &(rt_stg_pr_min), &(rt_stg_pri_diff_min));

		if (recv != 16) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"Format Error! Please enter in the following format\n"
						"RadarType-RT_ENB-RT_STGR-RT_CRPN_MIN-RT_CRPN_MAX-RT_CRPR_MIN-RT_PW_MIN-RT_PW_MAX-"
						"RT_PRI_MIN-RT_PRI_MAX-RT_CRBN_MIN-RT_CRBN_MAX-RT_STGPN_MIN-RT_STGPN_MAX-RT_STGPR_MIN-RT_STGPRID_MIN\n");
			return TRUE;
		}
		MTWF_PRINT("RadarType = %d\n RT_ENB = %d\n RT_STGR = %d\n "
			"RT_CRPN_MIN = %d\n RT_CRPN_MAX = %d\n RT_CRPR_MIN = %d\n "
			"RT_PW_MIN = %d\n RT_PW_MAX =%d\n RT_PRI_MIN = %d\n "
			"RT_PRI_MAX = %d\n RT_CRBN_MIN = %d\n RT_CRBN_MAX = %d\n"
			"RT_STGPN_MIN = %d\n RT_STGPN_MAX = %d\n RT_STGPR_MIN = %d\n"
			"RT_STGPRID_MIN = %d\n",
			radar_type_idx, rt_en, rt_stgr, rt_crpn_min,
			rt_crpn_max, rt_crpr_min, rt_pw_min, rt_pw_max,
			rt_pri_min, rt_pri_max, rt_crbn_min, rt_crbn_max,
			rt_stg_pn_min, rt_stg_pn_max, rt_stg_pr_min, rt_stg_pri_diff_min);

		memset(&RadarThreshold, 0, sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));
		RadarThreshold.radar_type_idx = radar_type_idx;
		RadarThreshold.rt_en  = rt_en;
		RadarThreshold.rt_stgr = rt_stgr;
		RadarThreshold.rt_crpn_min =  rt_crpn_min;
		RadarThreshold.rt_crpn_max = rt_crpn_max;
		RadarThreshold.rt_crpr_min = rt_crpr_min;
		RadarThreshold.rt_pw_min = rt_pw_min;
		RadarThreshold.rt_pw_max = rt_pw_max;
		RadarThreshold.rt_pri_min = rt_pri_min;
		RadarThreshold.rt_pri_max = rt_pri_max;
		RadarThreshold.rt_crbn_min = rt_crbn_min;
		RadarThreshold.rt_crbn_max = rt_crbn_max;
		RadarThreshold.rt_stg_pn_min = rt_stg_pn_min;
		RadarThreshold.rt_stg_pn_max = rt_stg_pn_max;
		RadarThreshold.rt_stg_pr_min = rt_stg_pr_min;
		RadarThreshold.rt_stg_pri_diff_min = rt_stg_pri_diff_min;

		sw_radar_type = &pAd->CommonCfg.DfsParameter.radar_thrshld_param.sw_radar_type[radar_type_idx];
		sw_radar_type->rt_en = RadarThreshold.rt_en;
		sw_radar_type->rt_stgr = RadarThreshold.rt_stgr;
		sw_radar_type->rt_crpn_min = RadarThreshold.rt_crpn_min;
		sw_radar_type->rt_crpn_max = RadarThreshold.rt_crpn_max;
		sw_radar_type->rt_crpr_min = RadarThreshold.rt_crpr_min;
		sw_radar_type->rt_pw_min = RadarThreshold.rt_pw_min;
		sw_radar_type->rt_pw_max = RadarThreshold.rt_pw_max;
		sw_radar_type->rt_pri_min = RadarThreshold.rt_pri_min;
		sw_radar_type->rt_pri_max = RadarThreshold.rt_pri_max;
		sw_radar_type->rt_crbn_min = RadarThreshold.rt_crbn_min;
		sw_radar_type->rt_crbn_max = RadarThreshold.rt_crbn_max;
		sw_radar_type->rt_stg_pn_min = RadarThreshold.rt_stg_pn_min;
		sw_radar_type->rt_stg_pn_max = RadarThreshold.rt_stg_pn_max;
		sw_radar_type->rt_stg_pr_min = RadarThreshold.rt_stg_pr_min;
		sw_radar_type->rt_stg_pri_diff_min = RadarThreshold.rt_stg_pri_diff_min;

		mt_cmd_set_radar_thres_param(pAd, &RadarThreshold);
	}

	return TRUE;

}

INT set_radar_pls_thres_param_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 recv = 0, pls_pwr_max = 0, pls_pwr_min = 0;
	UINT32 pls_width_max = 0, pri_min_stgr = 0, pri_max_stgr = 0;
	UINT32 pri_min_cr = 0, pri_max_cr = 0;
	CMD_RDM_PULSE_THRESHOLD_UPDATE_T pls_thres_update = {0};
	PDFS_PULSE_THRESHOLD_PARAM pls_thrshld_param = NULL;

	pls_thrshld_param = &pAd->CommonCfg.DfsParameter.radar_thrshld_param.pls_thrshld_param;

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d",
							&(pls_width_max), &(pls_pwr_max), &(pls_pwr_min),
							&(pri_min_stgr), &(pri_max_stgr), &(pri_min_cr), &(pri_max_cr));

		if (recv != 7) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"MaxPulseWidth-MaxPulsePower-MinPulsePower-"
					"MinPRISTGR-MaxPRISTGR-MinPRICR-MaxPRICR\n");
			return TRUE;
		}
		MTWF_PRINT("MaxPulseWidth = %d\nMaxPulsePower = %d\nMinPulsePower = %d\n"
					"MinPRISTGR = %d\nMaxPRISTGR = %d\nMinPRICR = %d\nMaxPRICR = %d\n",
					pls_width_max, pls_pwr_max, pls_pwr_min,
					pri_min_stgr, pri_max_stgr, pri_min_cr, pri_max_cr);

		pls_thres_update.prd_pls_width_max = pls_width_max;
		pls_thres_update.pls_pwr_max = pls_pwr_max;
		pls_thres_update.pls_pwr_min = pls_pwr_min;
		pls_thres_update.pri_min_stgr = pri_min_stgr;
		pls_thres_update.pri_max_stgr = pri_max_stgr;
		pls_thres_update.pri_min_cr = pri_min_cr;
		pls_thres_update.pri_max_cr = pri_max_cr;

		pls_thrshld_param->pls_width_max = pls_width_max;
		pls_thrshld_param->pls_pwr_max = pls_pwr_max;
		pls_thrshld_param->pls_pwr_min = pls_pwr_min;
		pls_thrshld_param->pri_min_stgr = pri_min_stgr;
		pls_thrshld_param->pri_max_stgr = pri_max_stgr;
		pls_thrshld_param->pri_min_cr = pri_min_cr;
		pls_thrshld_param->pri_max_cr = pri_max_cr;

		mt_cmd_set_pls_thres_param(pAd, &pls_thres_update);
	}

	return TRUE;
}

INT	set_radar_test_pls_pattern_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 recv = 0;
	CHAR *p_pls_params = 0;
	UINT32 pls_num = 0;

	CMD_RDM_TEST_RADAR_PATTERN_T pls_pattern = {0};
	PPERIODIC_PULSE_BUFFER_T pr_pls_buff = NULL;
	/*
	 Ex: 29151901-28-748;29153127-29-760;29154352-29-748;29155577-28-760;29156652-29-751
	*/
	if (arg) {
		for (pls_num = 0, p_pls_params = rstrtok(arg, ";"); (p_pls_params != NULL) && (pls_num < PPB_SIZE); p_pls_params = rstrtok(NULL, ";"), pls_num++) {
			pr_pls_buff = &pls_pattern.prd_pls_buff[pls_num];

			recv = sscanf(p_pls_params, "%d-%hu-%hi",
				&(pr_pls_buff->prd_strt_time),
				&(pr_pls_buff->prd_pls_wdth),
				&(pr_pls_buff->prd_pls_pwr));

			if (recv != 3) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
						"Format Error! Please enter in the following format\n"
							"StartTime0-PulseWidth0-PulsePower0;StartTime1-PulseWidth1-PulsePower1;...\n");
				return TRUE;
			}
		}

		pls_pattern.pls_num = pls_num;
		MTWF_PRINT("%s:No of pulses = %d\n", __func__, pls_pattern.pls_num);
		mt_cmd_set_test_radar_pattern(pAd, &pls_pattern);
	}

	return TRUE;
}

INT	set_radar_dbg_log_config_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 recv = 0;
	UINT32 hw_rdd_log_en = 0;
	UINT32 sw_rdd_log_en = 0;
	UINT32 sw_rdd_log_cond = 1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR rd_det_mode;

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d", &(hw_rdd_log_en), &(sw_rdd_log_en), &(sw_rdd_log_cond));

		if (recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
					"HWRDD_LOG_ENB-SWRDD_LOG_ENB-SWRDD_LOG_COND\n");
			return TRUE;
		}
#ifdef ZWDFS_AX7800
#ifdef MULTI_INF_SUPPORT
	struct wifi_dev *temp_wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PRTMP_ADAPTER pOpposAd = NULL;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT opposBandIdx = !multi_inf_get_idx(pAd);

	if (WMODE_CAP_5G(wdev->PhyMode)) {
		pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
		pAd = pOpposAd;
			if (pOpposAd != NULL) {
				MTWF_PRINT("%s Now: %s, Oppos: %s\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name);
			} else
				MTWF_PRINT("%s Now: %s\n", __func__, pAd->net_dev->name);
		}
#endif
#endif
		if (hw_rdd_log_en != 0)
			pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en = TRUE;
		else
			pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en = FALSE;

		if (sw_rdd_log_en != 0)
			pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en = TRUE;
		else
			pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en = FALSE;

		if (sw_rdd_log_cond == 0) {
			pAd->CommonCfg.DfsParameter.sw_rdd_log_cond = FALSE;
			rd_det_mode = RDD_DETMODE_DEBUG;
		} else {
			pAd->CommonCfg.DfsParameter.sw_rdd_log_cond = TRUE;
			rd_det_mode = RDD_DETMODE_ON;
		}

		/* Turn ON detection mode */
		pDfsParam->bNoSwitchCh = TRUE;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_WARN,
			"HWRDD_LOG_ENB = %d, SWRDD_LOG_ENB = %d SWRDD_LOG_COND = %d\n",
			pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en,
			pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en,
			pAd->CommonCfg.DfsParameter.sw_rdd_log_cond);

		mtRddControl(pAd, RDD_DET_MODE, 0, 0, rd_det_mode);
		mt_cmd_set_rdd_log_config(pAd, hw_rdd_log_en, sw_rdd_log_en, sw_rdd_log_cond);
	}

	return TRUE;
}

INT show_radar_threshold_param_proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	UINT8 radar_type_idx = 0;
	PDFS_RADAR_THRESHOLD_PARAM radar_thrshld_param = NULL;
	PSW_RADAR_TYPE_T radar_type = NULL;
	PDFS_PULSE_THRESHOLD_PARAM pls_thrshld_param = NULL;
	UCHAR current_rddregion = 0;
	UINT_8 fr_radar_d = 0;
	UINT_8 bk_radar_d = 0;

	MTWF_PRINT("pAd->CommonCfg.RDDurRegion = %d\n", pAd->CommonCfg.RDDurRegion);

	radar_thrshld_param = &pAd->CommonCfg.DfsParameter.radar_thrshld_param;
	pls_thrshld_param = &radar_thrshld_param->pls_thrshld_param;
	current_rddregion = pAd->CommonCfg.RDDurRegion;

	MTWF_PRINT("---------------------------------Debug Log Conditions---------------------------------------\n");
	MTWF_PRINT("HWRDD_LOG_ENB = %d\nSWRDD_LOG_ENB = %d\nSWRDD_LOG_COND = %d\n",
					pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en,
					pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en,
					pAd->CommonCfg.DfsParameter.sw_rdd_log_cond);
	MTWF_PRINT("-------------------------------Pulse Threshold Parameters-----------------------------------\n");
	MTWF_PRINT("FCC5_LPN = %d\n", pAd->CommonCfg.DfsParameter.fcc_lpn_min);
	MTWF_PRINT("PLS_POWER_MIN = %d\n", pls_thrshld_param->pls_pwr_min);
	MTWF_PRINT("PLS_POWER_MAX = %d\n", pls_thrshld_param->pls_pwr_max);
	MTWF_PRINT("SP_PW_MAX = %d\n", pls_thrshld_param->pls_width_max);
	MTWF_PRINT("PRI_MIN_STGR = %d\n", pls_thrshld_param->pri_min_stgr);
	MTWF_PRINT("PRI_MAX_STGR = %d\n", pls_thrshld_param->pri_max_stgr);
	MTWF_PRINT("PRI_MIN_CR = %d\n", pls_thrshld_param->pri_min_cr);
	MTWF_PRINT("PRI_MAX_CR = %d\n", pls_thrshld_param->pri_max_cr);

	MTWF_PRINT("---------------------------------RADAR Threshold Info---------------------------------------\n");
	switch(current_rddregion)
	{
		case ENUM_RDM_CE:
			fr_radar_d = ENUM_RDM_ETSI_1;
			bk_radar_d = ENUM_RDM_ETSI_6_3PRI;

			break;
		case ENUM_RDM_FCC:
			fr_radar_d = ENUM_RDM_FCC_1_JP_1;
			bk_radar_d = ENUM_RDM_FCC_6;

			break;
		case ENUM_RDM_JAP:
			fr_radar_d = ENUM_RDM_JP_2;
			bk_radar_d = ENUM_RDM_JP_4;

			break;
		case ENUM_RDM_JAP_W53:
		case ENUM_RDM_JAP_W56:
        case ENUM_RDM_KR:
        	fr_radar_d = ENUM_RDM_KR_1;
			bk_radar_d = ENUM_RDM_KR_3;

			break;

		default:
			break;
	}
	for (radar_type_idx = fr_radar_d; radar_type_idx <= bk_radar_d; radar_type_idx++) {
		radar_type = &radar_thrshld_param->sw_radar_type[radar_type_idx];
		MTWF_PRINT("RT - %d: ENB = %d, STGR = %d, CRPN_MIN = %d, CRPN_MAX = %d, CRPR_MIN = %d, PW_MIN = %d, PW_MAX = %d,"
					"PRI_MIN = %d, PRI_MAX = %d, CRBN_MIN = %d, CRBN_MAX = %d\n\t"
					"STGPN_MIN = %d, STGPN_MAX = %d, STGPR_MIN = %d, RT_STGPRID_MIN = %d\n",
							radar_type_idx,
							radar_type->rt_en,
							radar_type->rt_stgr,
							radar_type->rt_crpn_min,
							radar_type->rt_crpn_max,
							radar_type->rt_crpr_min,
							radar_type->rt_pw_min,
							radar_type->rt_pw_max,
							radar_type->rt_pri_min,
							radar_type->rt_pri_max,
							radar_type->rt_crbn_min,
							radar_type->rt_crbn_max,
							radar_type->rt_stg_pn_min,
							radar_type->rt_stg_pn_max,
							radar_type->rt_stg_pr_min,
							radar_type->rt_stg_pri_diff_min
							);
	}
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");

	return TRUE;
}

#endif /* MT_DFS_SUPPORT */
