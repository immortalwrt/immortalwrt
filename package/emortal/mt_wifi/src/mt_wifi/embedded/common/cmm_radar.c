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
				break;
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
			*phy_bw = BW_80; /* Out-band only support BW 20/40/80 */
		else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
			*phy_bw = BW_80; /* Out-band only support BW 20/40/80 */
		else
			;
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
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s] wdev->csa_count(%d) != 0 \n", __func__, wdev->csa_count));
		return;
	}

#ifdef MT_DFS_SUPPORT
	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		DfsRadarChannelCheck(pAd, wdev, vht_cent2, phy_bw)
	) {
#ifdef MAP_R2
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]cac_not_req %d \x1b[m \n", __func__, wdev->cac_not_required));
			if (wdev->cac_not_required == TRUE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] switch back to RD_NORMAL_MODE \x1b[m \n", __func__));
				/* DFS Zero wait case, OP CH always is normal mode */
				pDot11h->RDMode = RD_NORMAL_MODE;
				return;
			}
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE \x1b[m \n", __func__));

#else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE \x1b[m \n", __func__));
#endif
		pDot11h->RDMode = RD_SILENCE_MODE;
		pDot11h->RDCount = 0;
		pDot11h->InServiceMonitorCount = 0;
		if (DfsIsOutBandAvailable(pAd, wdev)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] OutBand Available. Set into RD_NORMAL_MODE \x1b[m \n", __func__));
			pDot11h->RDMode = RD_NORMAL_MODE;
		} else if (DfsIsTargetChAvailable(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Target Channel Bypass CAC. Set into RD_NORMAL_MODE \x1b[m \n", __FUNCTION__));
			pDot11h->RDMode = RD_NORMAL_MODE;

		} else
			;
	} else
#endif
	{
		if (WMODE_CAP_5G(wdev->PhyMode))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_NORMAL_MODE \x1b[m \n", __func__));

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
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("ERROR: previous detection of a radar on this channel(Channel=%d).\n",
						  pChCtrl->ChList[i].Channel));
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

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): mode = %u, ch = %u\n",
			 __func__, mode, ch));
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
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("%s() wdev %d not ready !!!\n",
				 __func__, wdev->wdev_idx));
		return;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): Wdev(%d) Channel Switching...(%d/%d)\n",
			 __func__, wdev->wdev_idx, pDot11h->CSCount, pDot11h->CSPeriod));
	pDot11h->CSCount++;

	if (pDot11h->CSCount >= pDot11h->CSPeriod) {
		if (wdev && (wdev->wdev_type == WDEV_TYPE_AP))
			apIdx = wdev->func_idx;

		/* CSA done */
		if (apIdx > 31) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR, ("%s(): Left shifting by more than 31 bits\n", __func__));
			return;
		}
		pDot11h->csa_ap_bitmap &= ~(UINT32)(1 << apIdx);

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE,
				 ("  Type = %d, func_idx = %d, csa_ap_bitmap = 0x%x\n",
				 wdev->wdev_type, wdev->func_idx, pDot11h->csa_ap_bitmap));

		/* do channel switch only when all BSS done */
		if (pDot11h->csa_ap_bitmap == 0)
			RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &apIdx, sizeof(UCHAR));
	}
}

/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR apIdx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR apOper = AP_BSS_OPER_ALL;
	struct DOT11_H *pDot11h = NULL;
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

	wdev = &pMbss->wdev;
	BandIdx = HcGetBandByWdev(wdev);
#ifdef OFFCHANNEL_SCAN_FEATURE
	Rsp.Action = DRIVER_CHANNEL_SWITCH_SUCCESSFUL;
	memcpy(Rsp.ifrn_name, pAd->ScanCtrl[BandIdx].if_name, IFNAMSIZ);
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	cfg = (struct wlan_config *)wdev->wpf_cfg;
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

	/* Normal DFS */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DedicatedZeroWaitStop(pAd, FALSE);
#endif
	pDot11h->RDMode = RD_SILENCE_MODE;

#ifdef DOT11W_PMF_SUPPORT
	if (pMbss->wdev.quick_ch_change && pMbss->wdev.SecConfig.ocv_support)
		ap_set_wait_sa_query_for_csa(pAd, &pMbss->wdev);
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] wdev name(%s), quick %d ,ch %d\n",
		__func__, (char *)pMbss->wdev.if_dev->name, pMbss->wdev.quick_ch_change, pMbss->wdev.channel));

#ifdef CONFIG_MAP_SUPPORT
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("cacreq %d\n", pMbss->wdev.cac_not_required));
#endif
	isRadarCh = RadarChannelCheck(pAd, pMbss->wdev.channel);
	if (pMbss->wdev.quick_ch_change != QUICK_CH_SWICH_DISABLE && (!isRadarCh
#ifdef CONFIG_MAP_SUPPORT
	|| (isRadarCh && pMbss->wdev.cac_not_required)
#endif
			)) {

		ap_phy_rrm_init_byRf(pAd, &pMbss->wdev);
#ifdef CONFIG_MAP_SUPPORT
#ifdef OFFCHANNEL_SCAN_FEATURE
		wdev->quick_ch_change = QUICK_CH_SWICH_DISABLE;
#endif
		pMbss->wdev.cac_not_required = FALSE;

		for (u = 0; u < pAd->ApCfg.BssidNum; u++) {
			struct wifi_dev *wdev_temp = NULL;
			wdev_temp = &pAd->ApCfg.MBSSID[u].wdev;
			if (wdev_temp->cac_not_required == TRUE) {
				wdev_temp->cac_not_required = FALSE;
			}
			/*need to make all the MBSS cac not required false*/
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("if: %s cac_not_required: %d\n",
							(char *)wdev_temp->if_dev->name,
							wdev_temp->cac_not_required));
		}
#endif
	} else

	{
		APStop(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
		if (DfsStopWifiCheck(pAd, &pMbss->wdev)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n", __func__));
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
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
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
						&& (wdev_temp->WscControl.bWscTrigger))
							pAd->StaCfg[j].ApcliInfStat.Enable = TRUE;
						break;
						}
					}
				}
			}
		}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] cac not req %d\n", __func__, pDot11h->cac_not_required));

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
				UINT8 BandIdx = HcGetBandByWdev(wdev);
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
	if (pAd->ApCfg.iwpriv_event_flag) {
		RTMP_OS_COMPLETE(&pAd->ApCfg.set_ch_aync_done);
		BandIdx = HcGetBandByWdev(wdev);
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		if (pAutoChCtrl)
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
	}
	return 0;
}

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
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H[0].CSPeriod));
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
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("[%s]: band index: %d, cac_time: %d\n",
			__func__,
			band_idx,
			pAd->Dot11_H[band_idx].cac_time));
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
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: Reset channel block status.\n", __func__));

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

	if (attach) {
		if (wdev) {
			bandIdx = HcGetBandByWdev(wdev);
			wdev->pDot11_H = &pAd->Dot11_H[bandIdx];
		} else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): no wdev!\n", __func__));
		}
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Detach wdev=%d_Dot11_H!\n", __func__, wdev->wdev_idx));
		wdev->pDot11_H = NULL;
		wdev->csa_count = 0;
	}
}

#if !(defined(MT7615) || defined(MT7622))
#ifdef MT_DFS_SUPPORT
INT set_radar_min_lpn_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 min_lpn_update = simple_strtol(arg, 0, 10);

	if (min_lpn_update <= LPB_SIZE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():LPN Update %d \n", __func__, min_lpn_update));

		pAd->CommonCfg.DfsParameter.fcc_lpn_min = min_lpn_update;
		mt_cmd_set_fcc5_min_lpn(pAd, min_lpn_update);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():Invalid LPN value %d, please set in range 0 to 32\n", __func__, min_lpn_update));
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
						"RadarType-RT_ENB-RT_STGR-RT_CRPN_MIN-RT_CRPN_MAX-RT_CRPR_MIN-RT_PW_MIN-RT_PW_MAX-"
						"RT_PRI_MIN-RT_PRI_MAX-RT_CRBN_MIN-RT_CRBN_MAX-RT_STGPN_MIN-RT_STGPN_MAX-RT_STGPR_MIN-RT_STGPRID_MIN\n"));
			return TRUE;
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():RadarType = %d\n RT_ENB = %d\n RT_STGR = %d\n "
			"RT_CRPN_MIN = %d\n RT_CRPN_MAX = %d\n RT_CRPR_MIN = %d\n "
			"RT_PW_MIN = %d\n RT_PW_MAX =%d\n RT_PRI_MIN = %d\n "
			"RT_PRI_MAX = %d\n RT_CRBN_MIN = %d\n RT_CRBN_MAX = %d\n"
			"RT_STGPN_MIN = %d\n RT_STGPN_MAX = %d\n RT_STGPR_MIN = %d\n"
			"RT_STGPRID_MIN = %d\n",
			__func__, radar_type_idx, rt_en, rt_stgr, rt_crpn_min,
			rt_crpn_max, rt_crpr_min, rt_pw_min, rt_pw_max,
			rt_pri_min, rt_pri_max, rt_crbn_min, rt_crbn_max,
			rt_stg_pn_min, rt_stg_pn_max, rt_stg_pr_min, rt_stg_pri_diff_min));

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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"MaxPulseWidth-MaxPulsePower-MinPulsePower-"
					"MinPRISTGR-MaxPRISTGR-MinPRICR-MaxPRICR\n"));
			return TRUE;
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():MaxPulseWidth = %d\nMaxPulsePower = %d\nMinPulsePower = %d\n"
					"MinPRISTGR = %d\nMaxPRISTGR = %d\nMinPRICR = %d\nMaxPRICR = %d\n",
					__func__, pls_width_max, pls_pwr_max, pls_pwr_min,
					pri_min_stgr, pri_max_stgr, pri_min_cr, pri_max_cr));

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
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
							"StartTime0-PulseWidth0-PulsePower0;StartTime1-PulseWidth1-PulsePower1;...\n"));
				return TRUE;
			}
		}

		pls_pattern.pls_num = pls_num;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:No of pulses = %d\n", __func__, pls_pattern.pls_num));
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"HWRDD_LOG_ENB-SWRDD_LOG_ENB-SWRDD_LOG_COND\n"));
			return TRUE;
		}

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

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():HWRDD_LOG_ENB = %d, SWRDD_LOG_ENB = %d SWRDD_LOG_COND = %d\n",
			__func__,
			pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en,
			pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en,
			pAd->CommonCfg.DfsParameter.sw_rdd_log_cond));

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

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR,
				("pAd->CommonCfg.RDDurRegion = %d", pAd->CommonCfg.RDDurRegion));
	radar_thrshld_param = &pAd->CommonCfg.DfsParameter.radar_thrshld_param;
	pls_thrshld_param = &radar_thrshld_param->pls_thrshld_param;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------Debug Log Conditions---------------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("HWRDD_LOG_ENB = %d\nSWRDD_LOG_ENB = %d\nSWRDD_LOG_COND = %d\n",
					pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en,
					pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en,
					pAd->CommonCfg.DfsParameter.sw_rdd_log_cond));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("-------------------------------Pulse Threshold Parameters-----------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("FCC5_LPN = %d\n",
					pAd->CommonCfg.DfsParameter.fcc_lpn_min));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MIN = %d\n",
														pls_thrshld_param->pls_pwr_min));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MAX = %d\n",
														pls_thrshld_param->pls_pwr_max));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("SP_PW_MAX = %d\n",
														pls_thrshld_param->pls_width_max));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_STGR = %d\n",
														pls_thrshld_param->pri_min_stgr));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_STGR = %d\n",
														pls_thrshld_param->pri_max_stgr));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_CR = %d\n",
														pls_thrshld_param->pri_min_cr));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_CR = %d\n",
														pls_thrshld_param->pri_max_cr));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------RADAR Threshold Info---------------------------------------\n"));

	for (radar_type_idx = 0; radar_type_idx < RDD_RT_NUM; radar_type_idx++) {
		radar_type = &radar_thrshld_param->sw_radar_type[radar_type_idx];
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("RT - %d: ENB = %d, STGR = %d, CRPN_MIN = %d, CRPN_MAX = %d, CRPR_MIN = %d, PW_MIN = %d, PW_MAX = %d,"
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
							));
	}
	MTWF_LOG(DBG_CAT_AP, CATPROTO_DFS, DBG_LVL_OFF,
		("---------------------------------------------------------------------------------------------\n"));

	return TRUE;
}

#endif /* MT_DFS_SUPPORT */
#endif
