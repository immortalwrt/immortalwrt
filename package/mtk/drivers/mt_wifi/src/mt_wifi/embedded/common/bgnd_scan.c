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
/****************************************************************************
 ****************************************************************************

    Module Name:

    Abstract:
*/


#include "rt_config.h"
#include "bgnd_scan.h"
#ifdef CONFIG_AP_SUPPORT

/* extern MT_SWITCH_CHANNEL_CFG CurrentSwChCfg[2]; */

BOOLEAN BackgroundScanSkipChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i = 0; i < pAd->BgndScanCtrl.SkipChannelNum; i++) {
		if (Ch == pAd->BgndScanCtrl.SkipChannelList[i]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

static inline INT GetABandChOffset(
	IN INT Channel)
{
#ifdef A_BAND_SUPPORT

	if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
		return 1;
	else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			 (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
		return -1;

#endif /* A_BAND_SUPPORT */
	return 0;
}

UCHAR BgndSelectBestChannel(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	int i;
	UCHAR BestChannel = 0;
	UINT32 BestPercen = 0xffffffff, Percen = 0;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (pAd->BgndScanCtrl.GroupChList[i].SkipGroup == 0) {
#if (RDD_2_SUPPORTED == 1)
			Percen = pAd->BgndScanCtrl.GroupChList[i].max_ipi_noisy;
#else
			Percen = ((pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time) * 100) / (((pAd->BgndScanCtrl.ScanDuration) * 1000) - (pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time));
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Max_PCCA=%x, Min_PCCA=%x, Band0_Tx_Time=%x\n",
				pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time,
				pAd->BgndScanCtrl.GroupChList[i].Min_PCCA_Time,
				pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time);
#endif
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Band index=%d, ChIdx=%d control-Channle=%d cen-channel=%d, Percentage=%d\n",
				band_idx,
				i,
				pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel,
				pAd->BgndScanCtrl.GroupChList[i].CenChannel,
				Percen);

			if (Percen <= BestPercen) {
				BestPercen = Percen;
				BestChannel = pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel;
			}
		}
	}
	pAd->BgndScanCtrl.Noisy = BestPercen;

	return BestChannel;
}
VOID NextBgndScanChannel(RTMP_ADAPTER *pAd, UCHAR channel)
{
	int i;
	/* UCHAR next_channel = 0; */
	pAd->BgndScanCtrl.ScanChannel = 0;

	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (channel == pAd->BgndScanCtrl.BgndScanChList[i].Channel) {
			/* Record this channel's idx in ChannelList array.*/
			while (i < (pAd->BgndScanCtrl.ChannelListNum - 1)) {
				if (pAd->BgndScanCtrl.BgndScanChList[i + 1].SkipChannel != 1) {
					pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].Channel;
					pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].CenChannel;
					pAd->BgndScanCtrl.ChannelIdx = i + 1;
					return;
				} else
					i++;
			}

		}
	}
}

VOID FirstBgndScanChannel(RTMP_ADAPTER *pAd)
{
	int i;

	/* Find the first non skiped channel */
	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (pAd->BgndScanCtrl.BgndScanChList[i].SkipChannel != 1) {
			/* Record this channel's idx in ChannelList array.*/
			pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i].CenChannel;
			pAd->BgndScanCtrl.FirstChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ChannelIdx = i;
			break;
		}
	}

}

VOID BuildBgndScanChList(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT channel_idx = 0, ChListNum = 0;
	BOOLEAN is_aband = FALSE;
	UCHAR ch;
	UCHAR cfg_ht_bw = 0;
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = 0;
#endif
	UINT8 band_idx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	/* sanity check for null pointer */
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"NULL wdev, band idx = %d\n", band_idx);
		return;
	}

	band_idx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	is_aband = ((WMODE_CAP_5G(wdev->PhyMode)) ? TRUE : FALSE);

	/* Get BW of wdev */
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	vht_bw = wlan_config_get_vht_bw(wdev);
#endif

	/* Reset background scan channel list */
	os_zero_mem(pAd->BgndScanCtrl.BgndScanChList, sizeof(BGND_SCAN_SUPP_CH_LIST) * MAX_NUM_OF_CHANNELS);
	pAd->BgndScanCtrl.ChannelListNum = 0;

	if (is_aband) {
		/* Scan BW */
#ifdef DOT11_VHT_AC
		if (vht_bw == VHT_BW_80)
			pAd->BgndScanCtrl.ScanBW = BW_80;
		else if (vht_bw == VHT_BW_160)
			pAd->BgndScanCtrl.ScanBW = BW_160;
		else
#endif /* DOT11_VHT_AC */
			pAd->BgndScanCtrl.ScanBW = cfg_ht_bw;

		/* Build background scan channel list */
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {

			if ((ChListNum >= MAX_NUM_OF_CHANNELS) || (channel_idx >= MAX_NUM_OF_CHANNELS)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"ChListNum/channel_idx is ERROR: %d/%d\n", ChListNum, channel_idx);
				break;
			}

			ch = pChCtrl->ChList[channel_idx].Channel;

			if (cfg_ht_bw == BW_20) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_N_SUPPORT
			else if (((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
					  && (vht_bw == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
					 )
					 &&N_ChannelGroupCheck(pAd, ch, wdev)) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;

				if (GetABandChOffset(ch) == 1)
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel + 2;
				else
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel - 2;

				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_VHT_AC
			else if (vht_bw == VHT_BW_80) {
				if (vht80_channel_group(pAd, ch, wdev)) {
					UCHAR ch_band = wlan_config_get_ch_band(wdev);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = vht_cent_ch_freq(ch, VHT_BW_80, ch_band);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
					ChListNum++;
				}
			} else if (vht_bw == VHT_BW_160) {
				if (vht160_channel_group(pAd, ch, wdev)) {
					UCHAR ch_band = wlan_config_get_ch_band(wdev);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = vht_cent_ch_freq(ch, VHT_BW_160, ch_band);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
					ChListNum++;
				}
			}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		}
	} else {
		/* 2.4G only support BW20 background scan */
		/* Scan BW */
		pAd->BgndScanCtrl.ScanBW = BW_20;
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
			if ((ChListNum >= MAX_NUM_OF_CHANNELS) || (channel_idx >= MAX_NUM_OF_CHANNELS)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"ChListNum/channel_idx is ERROR: %d/%d\n", ChListNum, channel_idx);
				break;
			}
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, pChCtrl->ChList[channel_idx].Channel);
			ChListNum++;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"BandIdx=%d, IsABand=%d, ScanBW=%d\n",
		band_idx, is_aband, pAd->BgndScanCtrl.ScanBW);

	pAd->BgndScanCtrl.ChannelListNum = ChListNum;
	if (pAd->BgndScanCtrl.ChannelListNum == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"BandIdx = %d, pAd->BgndScanCtrl.ChannelListNum=%d\n",
			band_idx, ChListNum);
	}

	for (channel_idx = 0; channel_idx < pAd->BgndScanCtrl.ChannelListNum; channel_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Support channel: PrimCh=%d, CentCh=%d, DFS=%d\n",
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].Channel, pAd->BgndScanCtrl.BgndScanChList[channel_idx].CenChannel,
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].DfsReq);
	}

}

UINT8 GroupChListSearch(PRTMP_ADAPTER pAd, UCHAR CenChannel)
{
	UCHAR i;
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = pAd->BgndScanCtrl.GroupChList;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (GroupChList->CenChannel == CenChannel)
			return i;

		GroupChList++;
	}

	return 0xff;
}

VOID GroupChListInsert(PRTMP_ADAPTER pAd, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	UCHAR i = pAd->BgndScanCtrl.GroupChListNum;
	PBGND_SCAN_CH_GROUP_LIST GroupChList = &pAd->BgndScanCtrl.GroupChList[i];

	GroupChList->BestCtrlChannel = pSource->Channel;
	GroupChList->CenChannel = pSource->CenChannel;
	GroupChList->SkipGroup = pSource->SkipChannel;
	pAd->BgndScanCtrl.GroupChListNum = i + 1;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"Insert new group channel list Number=%d CenChannel=%d BestCtrlChannel=%d SkipGroup=%d\n",
		pAd->BgndScanCtrl.GroupChListNum,
		GroupChList->CenChannel,
		GroupChList->BestCtrlChannel,
		GroupChList->SkipGroup);

#if (RDD_2_SUPPORTED == 1)
	GroupChList->max_ipi_noisy = pSource->ipi_noisy;
	GroupChList->min_ipi_noisy = GroupChList->max_ipi_noisy;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"ipi_hist_free_cnt=0x%x, ipi_hist_cnt=0x%x, ipi_noisy = 0x%x\n",
		pSource->ipi_hist_free_cnt,
		pSource->ipi_hist_cnt,
		GroupChList->min_ipi_noisy);

#else
	GroupChList->Max_PCCA_Time = GroupChList->Min_PCCA_Time = pSource->PccaTime;
	GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"Max_PCCA_TIEM=0x%x\n",
		GroupChList->Max_PCCA_Time);

#endif

}

VOID GroupChListUpdate(PRTMP_ADAPTER pAd, UCHAR index, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	/* UCHAR i; */
	PBGND_SCAN_CH_GROUP_LIST GroupChList = &pAd->BgndScanCtrl.GroupChList[index];

	if (GroupChList->SkipGroup == 0 && pSource->SkipChannel == 1)
		GroupChList->SkipGroup = pSource->SkipChannel;

#if (RDD_2_SUPPORTED == 1)
	if (pSource->ipi_noisy > GroupChList->max_ipi_noisy) {
		GroupChList->max_ipi_noisy = pSource->ipi_noisy;
	}

	if (pSource->ipi_noisy < GroupChList->min_ipi_noisy) {
		GroupChList->min_ipi_noisy = pSource->ipi_noisy;
		GroupChList->BestCtrlChannel = pSource->Channel;
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"Update group channel list index=%d CenChannel=%d BestCtrlChannel=%d ipi_noisy=%x SkipGroup=%d\n",
		pAd->BgndScanCtrl.GroupChListNum,
		GroupChList->CenChannel,
		GroupChList->BestCtrlChannel,
		GroupChList->max_ipi_noisy,
		GroupChList->SkipGroup);

#else
	if (pSource->PccaTime > GroupChList->Max_PCCA_Time) {
		GroupChList->Max_PCCA_Time = pSource->PccaTime;
		GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	}

	if (pSource->PccaTime < GroupChList->Min_PCCA_Time) {
		GroupChList->Min_PCCA_Time = pSource->PccaTime;
		GroupChList->BestCtrlChannel = pSource->Channel;
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"Update group channel list index=%d CenChannel=%d BestCtrlChannel=%d PCCA_TIEM=%x SkipGroup=%d\n",
		pAd->BgndScanCtrl.GroupChListNum,
		GroupChList->CenChannel,
		GroupChList->BestCtrlChannel,
		GroupChList->Max_PCCA_Time,
		GroupChList->SkipGroup);
#endif

}

VOID GenerateGroupChannelList(PRTMP_ADAPTER pAd)
{
	UCHAR i, ListIndex;
	PBGND_SCAN_SUPP_CH_LIST	SuppChList = pAd->BgndScanCtrl.BgndScanChList;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ChannelListNum=%d\n",
			 pAd->BgndScanCtrl.ChannelListNum);
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, MAX_NUM_OF_CHANNELS * sizeof(BGND_SCAN_CH_GROUP_LIST));
	pAd->BgndScanCtrl.GroupChListNum = 0; /* reset Group Number. */

	for (i = 0; i < pAd->BgndScanCtrl.ChannelListNum; i++) {
		ListIndex = GroupChListSearch(pAd, SuppChList->CenChannel);

		if (ListIndex == 0xff) /* Not Found */
			GroupChListInsert(pAd, SuppChList);
		else
			GroupChListUpdate(pAd, ListIndex, SuppChList);

		SuppChList++;
	}
}


VOID BackgroundScanStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, BGND_SCAN_MAX_STATE, BGND_SCAN_MAX_MSG, (STATE_MACHINE_FUNC)Drop, BGND_SCAN_IDLE, BGND_SCAN_MACHINE_BASE);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanStartAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_PARTIAL_SCAN, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)BackgroundScanTimeoutAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_DONE, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanPartialAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SWITCH_CHANNEL, (STATE_MACHINE_FUNC)BackgroundSwitchChannelAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_DEDICATE_RX_SCAN, (STATE_MACHINE_FUNC)dedicated_rx_hist_scan_timeout_action);
#ifdef MT_DFS_SUPPORT
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_DEDICATE_RDD_REQ, (STATE_MACHINE_FUNC)DedicatedZeroWaitStartAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_RADAR_FOUND, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_SWITCH, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
#endif
}

VOID BackgroundScanInit(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = 0;
#endif
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Sanity check for NULL pointer */
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "wdev is NULL, return\n");
		return;
	}
#ifdef DOT11_VHT_AC
	vht_bw = wlan_config_get_vht_bw(wdev);
#endif

	/* Initilize BgndScanCtrl*/
	if (pAd->BgndScanCtrl.init_done != TRUE) {
		os_zero_mem(&pAd->BgndScanCtrl, sizeof(BACKGROUND_SCAN_CTRL));
		pAd->BgndScanCtrl.init_done = TRUE;
	}

#ifdef MT_DFS_SUPPORT
	pAd->BgndScanCtrl.DfsZeroWaitDuration = DEFAULT_OFF_CHNL_CAC_TIME;/* 120000; 2 min */
#endif

	/*
	Based on current settings to decide support background scan or not.
	Don't support case: DBDC, 80+80
	background scan can be supported if dedicated RX is used
	*/

#if (RDD_2_SUPPORTED == 1)
	/* Dedicated RX for DFS & BackgroundScan, bgndscan can be supported when DBDC is enabled */
	pAd->BgndScanCtrl.BgndScanSupport = 1;
#else

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
#if (RDD_PROJECT_TYPE_2 == 1)
		pAd->BgndScanCtrl.BgndScanSupport = 1;
#else
		pAd->BgndScanCtrl.BgndScanSupport = 0;
#endif
	}

#ifdef DOT11_VHT_AC
	else if (vht_bw == VHT_BW_8080) {
		pAd->BgndScanCtrl.BgndScanSupport = 0;
		pAd->BgndScanCtrl.DfsZeroWaitSupport = 0;
	}
#endif /* DOT11_VHT_AC */

	else
		pAd->BgndScanCtrl.BgndScanSupport = 1;

	pAd->BgndScanCtrl.RxPath = 0xc;
	pAd->BgndScanCtrl.TxStream = 0x2;
#endif /* RDD_2_SUPPORTED */

	if (pAd->BgndScanCtrl.BgndScanSupport) {
		BackgroundScanStateMachineInit(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, pAd->BgndScanCtrl.BgndScanFunc);
		RTMPInitTimer(pAd, &pAd->BgndScanCtrl.BgndScanTimer, GET_TIMER_FUNCTION(BackgroundScanTimeout), pAd, FALSE);

		/* ToDo: Related CR initialization. */
		if (ops->bgnd_scan_cr_init)
			ops->bgnd_scan_cr_init(pAd);

		pAd->BgndScanCtrl.ScanDuration = DefaultScanDuration;
		/* pAd->BgndScanCtrl.BgndScanInterval = DefaultBgndScanInterval; */
		pAd->BgndScanCtrl.BgndScanIntervalCount = 0;
		pAd->BgndScanCtrl.SkipDfsChannel = FALSE;
		pAd->BgndScanCtrl.PartialScanInterval = DefaultBgndScanPerChInterval; /* 10 seconds */
		pAd->BgndScanCtrl.NoisyTH = DefaultNoisyThreshold;
		pAd->BgndScanCtrl.ChBusyTimeTH = DefaultChBusyTimeThreshold;
		pAd->BgndScanCtrl.DriverTrigger = FALSE;
		pAd->BgndScanCtrl.IPIIdleTimeTH = DefaultIdleTimeThreshold;
		pAd->BgndScanCtrl.ipi_th = RDD_IPI_HIST_2;
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Background scan doesn't support in current settings....\n");
}

VOID BackgroundScanDeInit(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;
	RTMPReleaseTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
	RTMPReleaseTimer(&pAd->BgndScanCtrl.DfsZeroWaitTimer, &Cancelled);
}

VOID BackgroundScanStart(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8	bgnd_band_scan_info)
{
	/* UINT32	Value; */
	/* In-band commad to notify FW(RA) background scan will start. */
	/* Switch channel for Band0 (Include Star Tx/Rx ?) */
	/* Scan channel for Band1 */
	UINT8 bgnd_scan_type = (bgnd_band_scan_info & BGND_SCAN_TYPE_MASK);
	UINT8 band_idx = ((bgnd_band_scan_info & BGND_BAND_IDX_MASK) >> BGND_BAND_IDX_SHFT);

	/* Reset Group channel list */
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, sizeof(BGND_SCAN_CH_GROUP_LIST) * MAX_NUM_OF_CHANNELS);
	pAd->BgndScanCtrl.GroupChListNum = 0;

	/* Copy channel list for background. */
	BuildBgndScanChList(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"BgndscanType=%d, band idx=%d ===============>\n",
		bgnd_scan_type, band_idx);
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

	if (bgnd_scan_type && (pAd->BgndScanCtrl.ScanType & BGND_SCAN_TYPE_MASK)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"Ap Background scan is running ===============>\n");
		return;
	}
	pAd->BgndScanCtrl.ScanType = bgnd_band_scan_info;

	/* bgnd_scan_type - 0:Disable, 1:partial scan, 2:continuous scan */
	if (bgnd_scan_type == TYPE_BGND_PARTIAL_SCAN) {	/* partial scan */
		pAd->BgndScanCtrl.PartialScanIntervalCount = DefaultBgndScanPerChInterval; /* First time hope trigger scan immediately. */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_PARTIAL_SCAN, 0, NULL, bgnd_band_scan_info);
	} else if (bgnd_scan_type == TYPE_BGND_CONTINUOUS_SCAN) /* continuous scan */

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, bgnd_band_scan_info);
	else if (bgnd_scan_type == TYPE_BGND_CONTINUOUS_SCAN_SWITCH_CH) {/* continuous scan and then switch channel*/
		pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
		bgnd_band_scan_info &= (~BGND_SCAN_TYPE_MASK);
		bgnd_band_scan_info |= TYPE_BGND_CONTINUOUS_SCAN;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, bgnd_band_scan_info);
	} else {
		pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, bgnd_band_scan_info);
	}

	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UINT8 bgnd_band_scan_info = (UINT8)(Elem->Priv);
	UINT8 bgnd_scan_type = (bgnd_band_scan_info & BGND_SCAN_TYPE_MASK);

	/* 0:Disable 1:partial scan 2:continuous scan */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ScanType=%d\n", bgnd_scan_type);
	FirstBgndScanChannel(pAd);
	pAd->BgndScanCtrl.ScanType = bgnd_band_scan_info;
#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, GREENAP_REASON_AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	BackgroundScanNextChannel(pAd, bgnd_band_scan_info);
}

VOID BackgroundScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	/* Bit[7:4]: Band index, Bit[3:0]: background scan type */
	UINT8 bgnd_band_scan_info = pAd->BgndScanCtrl.ScanType;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ScanTypeInfo=%d\n", bgnd_band_scan_info);
	MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_TIMEOUT, 0, NULL, bgnd_band_scan_info);
	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
#if (RDD_2_SUPPORTED == 1)
	EXT_EVENT_RDD_IPI_HIST rdd_ipi_hist_rlt;
	UINT8 ipi_idx = 0;
	UCHAR ipi_th = pAd->BgndScanCtrl.ipi_th;
	UINT32 ipi_hist_val = 0;
#else
	RTMP_REG_PAIR Reg[5];
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
#endif
	UINT8 bgnd_band_scan_info = (UINT8)(Elem->Priv);
	UINT8 ScanType = (bgnd_band_scan_info & BGND_SCAN_TYPE_MASK);
	UINT8 band_idx = ((bgnd_band_scan_info & BGND_BAND_IDX_MASK) >> BGND_BAND_IDX_SHFT);
	UCHAR ch_idx = pAd->BgndScanCtrl.ChannelIdx;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	PBGND_SCAN_SUPP_CH_LIST p_bgnd_scan_ch = &pAd->BgndScanCtrl.BgndScanChList[ch_idx];

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"band idx=%d, channel index=%d, ScanType=%d ==========>\n",
		band_idx, ch_idx, ScanType);

	if (p_bgnd_scan_ch == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"p_bgnd_scan_ch == NULL\n");
		return;
	}

#if (RDD_2_SUPPORTED == 1)
	os_zero_mem(&rdd_ipi_hist_rlt, sizeof(EXT_EVENT_RDD_IPI_HIST));

	/* Get IPI cnt */
	rdd_ipi_hist_rlt.band_idx = band_idx;
	mt_cmd_get_rdd_ipi_hist(pAd, RDD_IPI_HIST_ALL_CNT, &rdd_ipi_hist_rlt);
	for (ipi_idx = 0; ipi_idx < IPI_HIST_TYPE_NUM; ipi_idx++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"ChannelIdx [%d] ipi_idx = %d,\t ipi_hist_cnt=\t0x%x \n",
			ch_idx,
			ipi_idx,
			rdd_ipi_hist_rlt.ipi_hist_val[ipi_idx]);

		/* Get IPI2-IPI10 total cnt */
		if (ipi_th <= ipi_idx && ipi_idx <= RDD_IPI_HIST_10) {
			ipi_hist_val += rdd_ipi_hist_rlt.ipi_hist_val[ipi_idx];
		}

	}

	/* Get IPI free run cnt */
	p_bgnd_scan_ch->ipi_hist_free_cnt = rdd_ipi_hist_rlt.ipi_hist_val[RDD_IPI_FREE_RUN_CNT];

	/* Get IPI2-IPI10 total cnt */
	p_bgnd_scan_ch->ipi_hist_cnt = ipi_hist_val;

	/* TBD: judge noisy */
	p_bgnd_scan_ch->ipi_noisy = ipi_hist_val;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"ChannelIdx [%d], Channel=%d, ipi_th=%d, ipi_hist_cnt=0x%x <===============\n",
		ch_idx,
		p_bgnd_scan_ch->Channel,
		ipi_th,
		p_bgnd_scan_ch->ipi_hist_cnt);

#else
	/* Update channel info */
	Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
	Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
	Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
	Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
	Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMultipleMacRegAccessRead(pAd, Reg, 5);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
	p_bgnd_scan_ch->PccaTime = Reg[0].Value;
	p_bgnd_scan_ch->SccaTime = Reg[1].Value;
	p_bgnd_scan_ch->EDCCATime = Reg[2].Value;
	p_bgnd_scan_ch->Band0TxTime = Reg[3].Value;
	p_bgnd_scan_ch->Mdrdy = Reg[4].Value;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ChannelIdx [%d].PCCA_TIME=%x, SCCA_TIEM=%x, EDCCA_TIME=%x, Band0TxTime=%x Mdrdy=%x ===============>\n", pAd->BgndScanCtrl.ChannelIdx, Reg[0].Value, Reg[1].Value, Reg[2].Value, Reg[3].Value, Reg[4].Value);
#endif

	NextBgndScanChannel(pAd, pAd->BgndScanCtrl.ScanChannel);

	if (pAd->BgndScanCtrl.ScanChannel == 0 || ScanType == TYPE_BGND_CONTINUOUS_SCAN) /* Ready to stop or continuous scan */
		BackgroundScanNextChannel(pAd, bgnd_band_scan_info);
	else {	/* Next time partail scan */

		/* return to SynA only */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_BGND_TYPE);


#if (RDD_2_SUPPORTED == 0)
		/* Enable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */
#endif
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_DONE, 0, NULL, 0);
	}
}

VOID BackgroundScanWaitAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	/* Change state to Wait State. If all conditions match, will trigger partial scan */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_WAIT;
}
VOID BackgroundScanPartialAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UINT8 ScanType = (UINT8)(Elem->Priv);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ScanType = %d\n", ScanType);

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "First time\n");
		FirstBgndScanChannel(pAd);
#ifdef GREENAP_SUPPORT
		greenap_suspend(pAd, GREENAP_REASON_AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	}

	BackgroundScanNextChannel(pAd, ScanType);
}

VOID BackgroundScanCancelAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	/* Re-init related parameters */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
	pAd->BgndScanCtrl.ScanChannel = 0;
	pAd->BgndScanCtrl.ScanType = 0;
	pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
	RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, GREENAP_REASON_AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	/* RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanNextTimer, &Cancelled); */
}

VOID BackgroundScanNextChannel(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 bgnd_band_scan_info)
{

	UINT8 bgnd_scan_type = (bgnd_band_scan_info & BGND_SCAN_TYPE_MASK);
	UINT8 band_idx = ((bgnd_band_scan_info & BGND_BAND_IDX_MASK) >> BGND_BAND_IDX_SHFT);
	UCHAR BestChannel = 0;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#if (RDD_2_SUPPORTED == 0)
	RTMP_REG_PAIR Reg[5];
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
#else
	EXT_CMD_RDD_IPI_HIST_T cmd_rdd_ipi_hist;

	os_zero_mem(&cmd_rdd_ipi_hist, sizeof(EXT_CMD_RDD_IPI_HIST_T));
#endif

	/* Restore switch channel configuration */
	CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[band_idx]);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"Scan Channel=%d, band idx = %d\n",
		pAd->BgndScanCtrl.ScanChannel, band_idx);

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		/* return to SynA only */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_BGND_TYPE);

		GenerateGroupChannelList(pAd);
		BestChannel = BgndSelectBestChannel(pAd, band_idx);
		pAd->BgndScanCtrl.BestChannel = BestChannel;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"Best Channel=%d, IsSwitchChannel=%d Noisy=%d\n",
			BestChannel, pAd->BgndScanCtrl.IsSwitchChannel, pAd->BgndScanCtrl.Noisy);

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
		pAd->BgndScanCtrl.ScanType = TYPE_BGND_DISABLE_SCAN;/* Scan Complete. */

		if (BestChannel != CurrentSwChCfg->ControlChannel && pAd->BgndScanCtrl.IsSwitchChannel == TRUE) {
			pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SWITCH_CHANNEL, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}

		pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
#ifdef GREENAP_SUPPORT
		greenap_resume(pAd, GREENAP_REASON_AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	} else if (bgnd_scan_type == TYPE_BGND_PARTIAL_SCAN) { /* Partial Scan */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Split into SynA + SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
#if (RDD_2_SUPPORTED == 1)
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_HIST_RESET;
		cmd_rdd_ipi_hist.set_val = 0;
		cmd_rdd_ipi_hist.band_idx = band_idx;

		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);
#else
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
#endif
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else if (pAd->BgndScanCtrl.ScanChannel == pAd->BgndScanCtrl.FirstChannel) {
		/* First time to do background scan */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Split into SynA + SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
#if (RDD_2_SUPPORTED == 1)
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_HIST_RESET;
		cmd_rdd_ipi_hist.set_val = 0;
		cmd_rdd_ipi_hist.band_idx = band_idx;

		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);
#else
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
#endif

		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else {
		/* RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, 200); //200ms timer */
		/* Switch Band1 channel to pAd->BgndScanCtrl.ScanChannel */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;

		/* Switch channel of SynB */
		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_RUNNING, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
#if (RDD_2_SUPPORTED == 1)
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_HIST_RESET;
		cmd_rdd_ipi_hist.set_val = 0;
		cmd_rdd_ipi_hist.band_idx = band_idx;

		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);
#else
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
#endif

		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 500ms timer */
	}
}

VOID BackgroundSwitchChannelAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;

	wdev = pAd->wdev_list[0];
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN, "Switch to channel to %d\n", pAd->BgndScanCtrl.BestChannel);
	rtmp_set_channel(pAd, wdev, pAd->BgndScanCtrl.BestChannel);
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
}

VOID BackgroundChannelSwitchAnnouncementAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Trigger Channel Switch Announcemnet IE\n");
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN; /* Channel switch annoncement. */
	/* HcUpdateCsaCntByChannel(pAd, pAd->BgndScanCtrl.BestChannel); */
	/* pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE; //For temporary */
}

NDIS_STATUS set_dfs_dedicated_rx_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	INT32 recv = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR ch_band = 0;
	UCHAR band_idx = 0;
	UINT32 ch = 0, cen_ch = 0;
	UINT32 bw = 0;
	EXT_CMD_RDD_IPI_HIST_T cmd_rdd_ipi_hist;
	UCHAR bgnd_band_scan_info = 0;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
		"wdev is Null\n");
		return NDIS_STATUS_FAILURE;
	}

	ch_band = wlan_config_get_ch_band(wdev);
	band_idx = HcGetBandByWdev(wdev);

	os_zero_mem(&cmd_rdd_ipi_hist, sizeof(EXT_CMD_RDD_IPI_HIST_T));

	if (arg) {
		recv = sscanf(arg, "%d:%d", &(ch), &(bw));

		if (recv != 2) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Format Error! Please enter in the following format\n"
					"ch:bw(0: 20MHz, 1: 40MHz, 2: 80MHz, 3: 160MHz)\n");
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Set 0:0 to disable 5th RX\n");
		       return TRUE;
		}

		if (ch == 0 && bw == 0) {
			if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_BGND_TYPE);

				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
					"disable dedicated rx\n");

				return NDIS_STATUS_SUCCESS;
       }

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"ch %d, bw = %d\n", ch, bw);

		/* Dedicated RX */
		pAd->BgndScanCtrl.ScanChannel = ch;
		pAd->BgndScanCtrl.ScanBW = bw;
		bgnd_band_scan_info |= (band_idx << BGND_BAND_IDX_SHFT);
		pAd->BgndScanCtrl.ScanType = bgnd_band_scan_info;

		/* set central channel*/
		switch (bw) {
		case BW_20:
			cen_ch = ch;
			break;

		case BW_40:
			if (N_ChannelGroupCheck(pAd, ch, wdev) && ch_band == CMD_CH_BAND_5G) {
				if (GetABandChOffset(ch) == 1)
					cen_ch = ch + 2;
				else
					cen_ch = ch - 2;
			}
			else if (N_ChannelGroupCheck(pAd, ch, wdev) && ch_band == CMD_CH_BAND_24G) {
				UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
				if (ext_cha == EXTCHA_ABOVE)
					if (ch <= 9)
						cen_ch = ch + 2;
					/* error handling: use extcha_below */
					else
						cen_ch = ch - 2;
				else {
					if (ch >= 5)
						cen_ch = ch - 2;
					/* error handling: use extcha_above */
					else
						cen_ch = ch + 2;
				}
			} else {
				if (ch == 14)
					cen_ch = ch - 1;
				else
					cen_ch = ch;
			}
			break;

		case BW_80:
			if (vht80_channel_group(pAd, ch, wdev))
				cen_ch = vht_cent_ch_freq(ch, VHT_BW_80, ch_band);
			else
				cen_ch = ch;

			break;

		case BW_160:
			if (vht160_channel_group(pAd, ch, wdev))
				cen_ch = vht_cent_ch_freq(ch, VHT_BW_160, ch_band);
			else
				cen_ch = ch;

			break;

		default:
			cen_ch = ch;
			break;
		}
		pAd->BgndScanCtrl.ScanCenChannel = cen_ch;

		if (ops->set_off_ch_scan)
			ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_BGND_TYPE);

		/* Read clear below CR */
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_HIST_RESET;
		cmd_rdd_ipi_hist.set_val = 0;
		cmd_rdd_ipi_hist.band_idx = band_idx;

		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);


		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Arg is Null\n");
			status = NDIS_STATUS_FAILURE;
		}

		return status;
}

NDIS_STATUS set_dedicated_rx_hist_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	UCHAR band_idx = 0;
	INT32 recv = 0;
	UINT32 thres = 0;
	UINT32 period = 0;
	EXT_CMD_RDD_IPI_HIST_T cmd_rdd_ipi_hist;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	os_zero_mem(&cmd_rdd_ipi_hist, sizeof(EXT_CMD_RDD_IPI_HIST_T));

	RTMPInitTimer(pAd, &pAd->BgndScanCtrl.hist_scan_timer,
		GET_TIMER_FUNCTION(dedicated_rx_hist_scan_timeout), pAd, FALSE);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "wdev is Null\n");
		return NDIS_STATUS_FAILURE;
	}

	band_idx = HcGetBandByWdev(wdev);

	if (arg) {
		recv = sscanf(arg, "%d:%d", &(thres), &(period));

		if (recv != 2) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"Format Error! Please enter in the following format\n"
					"threshold(0-10):period(ms)\n");
		       return TRUE;
		}

		pAd->BgndScanCtrl.ipi_th = thres;
		pAd->BgndScanCtrl.dfs_ipi_period = period;
		pAd->BgndScanCtrl.band_idx = band_idx;
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"band[%d], thres %d (ipi_threshold), period %d (ms)\n",
			band_idx, thres, pAd->BgndScanCtrl.dfs_ipi_period);

		/* clear histogram CR */
		cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_HIST_RESET;
		cmd_rdd_ipi_hist.set_val = 0;
		cmd_rdd_ipi_hist.band_idx = band_idx;
		mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);

		/* Start timer */
		RTMPSetTimer(&pAd->BgndScanCtrl.hist_scan_timer, period);

		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "Arg is Null\n");
			status = NDIS_STATUS_FAILURE;
		}

		return status;
}

#ifdef IPI_SCAN_SUPPORT
NDIS_STATUS set_ipi_scan_ctrl_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT32 recv = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *tgt_wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR band_idx = 0;
	UINT32 ch = 0;
	UINT32 bw = 0;
	UCHAR i = 0;

	if (tgt_wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "wdev is Null\n");
		return TRUE;
	}
	band_idx = HcGetBandByWdev(tgt_wdev);

	if (arg) {
		recv = sscanf(arg, "%d:%d", &(ch), &(bw));
		if (recv != 2) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
				"ch:bw 2G(0: 20MHz, 1: 40MHz), 5G(0: 20MHz/40MHz, 1: 80MHz, 2:160MHz)\n");
			return TRUE;
		}
		if (WMODE_CAP_AC(tgt_wdev->PhyMode)) {
			if (bw > VHT_BW_160)
				bw = VHT_BW_2040;

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				struct wifi_dev *wdev;
				wdev = pAd->wdev_list[i];
				if (wdev && (band_idx == HcGetBandByWdev(wdev))) {
					wlan_config_set_vht_bw(wdev, bw);
					wlan_operate_set_vht_bw(wdev, bw);
					//SetCommonHtVht(pAd, tdev);
				}
			}
		} else {
			if ((bw != BW_40) && (bw != BW_20))
				bw = BW_20;

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				struct wifi_dev *wdev;
				wdev = pAd->wdev_list[i];
				if (wdev && (band_idx == HcGetBandByWdev(wdev))) {
					if (bw == BW_40) {
						wlan_config_set_ht_bw(wdev, BW_40);
						wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_operate_get_ext_cha(wdev));
					} else {
						wlan_config_set_ht_bw(wdev, BW_20);
						wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
					}
					if (wdev->channel == ch) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
							"%s: Same Channel %d Updating Bw %d\n", __func__, ch, bw);
						SetCommonHtVht(pAd, wdev);
					}
				}
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"%s: Channel %d Bw %d\n", __func__, ch, bw);
		rtmp_set_channel(pAd, tgt_wdev, ch);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"%s: Arg is Null\n", __func__);
	}
	return TRUE;
}

NDIS_STATUS set_ipi_scan_hist_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS status;

	RTMP_STRING *threshold, *period, *idx;
	INT8 ant_index = -1;
	UCHAR band_idx = 0;

	EXT_CMD_RDD_IPI_SCAN_T cmd_rdd_ipi_scan;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	os_zero_mem(&cmd_rdd_ipi_scan, sizeof(EXT_CMD_RDD_IPI_SCAN_T));

	RTMPInitTimer(pAd, &pAd->BgndScanCtrl.hist_scan_timer,
		GET_TIMER_FUNCTION(dedicated_rx_hist_scan_timeout), pAd, FALSE);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "wdev is Null\n");
		return TRUE;
	}

	band_idx = HcGetBandByWdev(wdev);

	if (arg) {
		threshold = rstrtok(arg, ":");
		period = rstrtok(NULL, ":");
		idx = rstrtok(NULL, ":");
		if ((threshold == NULL) || (period == NULL)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"Format Error! Please enter in the following format\n"
				"threshold:period or threshold:period:antena idx\n");
			return TRUE;
		}
		if (idx != NULL) {
			ant_index = simple_strtol(idx, 0, 10);
			if ((ant_index > 3) || (ant_index < 0)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"antena idx out of range Using all antenas\n");
				ant_index = -1;
			}
		}

		pAd->BgndScanCtrl.ipi_th = simple_strtol(threshold, 0, 10) ;
		pAd->BgndScanCtrl.dfs_ipi_period = simple_strtol(period, 0, 10);
		pAd->BgndScanCtrl.band_idx = band_idx;
		pAd->BgndScanCtrl.antena_idx = ant_index;

		if (pAd->BgndScanCtrl.ipi_th > 10) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"ipi_threshold out of range Using ipi_threshold 0\n");
		}

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
			"band[%d], thres %d (ipi_threshold), period %d (ms) antena index %d\n",
			band_idx, pAd->BgndScanCtrl.ipi_th, pAd->BgndScanCtrl.dfs_ipi_period,
			pAd->BgndScanCtrl.antena_idx);

		/* clear histogram CR */
		cmd_rdd_ipi_scan.u1mode = 1;
		status = mt_cmd_set_rdd_ipi_scan(pAd, &cmd_rdd_ipi_scan);

		if (status == NDIS_STATUS_SUCCESS) {
			/* Start timer */
			RTMPSetTimer(&pAd->BgndScanCtrl.hist_scan_timer, pAd->BgndScanCtrl.dfs_ipi_period);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"%s: ipi scan command failed\n", __func__);
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"%s: Arg is Null\n", __func__);
	}
	return TRUE;
}
#endif

VOID dedicated_rx_hist_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	BOOLEAN Cancelled;

	MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RX_SCAN, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	RTMPCancelTimer(&pAd->BgndScanCtrl.hist_scan_timer, &Cancelled);
	RTMPReleaseTimer(&pAd->BgndScanCtrl.hist_scan_timer, &Cancelled);

	return;
}


VOID dedicated_rx_hist_scan_timeout_action(
	RTMP_ADAPTER * pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UINT8 ipi_idx = 0;
	UCHAR ipi_th = pAd->BgndScanCtrl.ipi_th;
	UINT32 ipi_hist_val = 0;
	UINT32 free_cnt = 0;
	UINT32 self_idle_ratio = 0, ipi_percent = 0, ch_load = 0;
	UINT32 ipi_idle_ratio = 0, threshold = 0;
	UINT32 period = 0;
#define INC_PRECISION 1000
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
		"%s(): band[%d], show IPI histogram\n", __func__, pAd->BgndScanCtrl.band_idx);

#ifdef IPI_SCAN_SUPPORT
	if (IS_MT7986(pAd) || IS_MT7981(pAd)) {
		UINT8 start_idx = 0;
		UINT8 i, antena_count = 1;
		EXT_EVENT_RDD_IPI_SCAN rdd_ipi_scan_hist;
		os_zero_mem(&rdd_ipi_scan_hist, sizeof(EXT_EVENT_RDD_IPI_SCAN));
		mt_cmd_get_rdd_ipi_scan(pAd, &rdd_ipi_scan_hist);
		if (pAd->BgndScanCtrl.band_idx == 1)
			start_idx = 4;

		if (pAd->BgndScanCtrl.antena_idx == -1)
			antena_count = 4;
		else
			start_idx += pAd->BgndScanCtrl.antena_idx;

		for (i = 0; i < antena_count; i++) {
			free_cnt = 0;
			ipi_hist_val = 0;
			for (ipi_idx = 0; ipi_idx < PWR_INDICATE_HIST_MAX; ipi_idx++) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
					"ipi[%d],\t ipi_hist_cnt=\t%d\n",
					ipi_idx,
					rdd_ipi_scan_hist.au4IpiHistVal[start_idx][ipi_idx]);

				/* Get IPI2-IPI10 total cnt */
				if (ipi_th <= ipi_idx && ipi_idx <= RDD_IPI_HIST_10) {
					ipi_hist_val += rdd_ipi_scan_hist.au4IpiHistVal[start_idx][ipi_idx];
				}
				/* Get IPI free run cnt */
				free_cnt += rdd_ipi_scan_hist.au4IpiHistVal[start_idx][ipi_idx];
			}

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"threshold ipi[%d], ipi cnt %d, free run cnt %d\n",
				ipi_th, ipi_hist_val, free_cnt);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"tx_assert_time %d (ms)\n", rdd_ipi_scan_hist.u4TxAssertTime / 1000);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"Period %d (ms)\n", pAd->BgndScanCtrl.dfs_ipi_period);

			/* channel_load */
			ipi_percent = (INC_PRECISION * 100 * ipi_hist_val / free_cnt);
			if (ipi_percent >= 100 * INC_PRECISION)
				ipi_percent = 100 * INC_PRECISION;

			ipi_idle_ratio = ((100 * INC_PRECISION) - ipi_percent);

			period = pAd->BgndScanCtrl.dfs_ipi_period; /* ms */
			self_idle_ratio = (INC_PRECISION * 100 * (period - (rdd_ipi_scan_hist.u4TxAssertTime / 1000))) / period;
			/* Channel_Load = (Self_Idle_Ratio - Idle_Ratio)/Self_Idle_Ratio */
			if (self_idle_ratio < ipi_idle_ratio)
				ch_load = 0;
			else
				ch_load = (self_idle_ratio - ipi_idle_ratio);

			if (self_idle_ratio <= threshold * INC_PRECISION) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"band[%d] - self_idle_ratio %d%% < threshold %d%%\n",
				rdd_ipi_scan_hist.u1BandIdx, self_idle_ratio/INC_PRECISION, threshold/INC_PRECISION);

				pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
				return;
			}

			ch_load = (100 * ch_load) / self_idle_ratio;

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"ch_load %d%%, band[%d] antena idx[%d] - self_idle_ratio %d%%, idle_ratio %d%%\n",
				ch_load, pAd->BgndScanCtrl.band_idx, start_idx, self_idle_ratio/INC_PRECISION, ipi_idle_ratio/INC_PRECISION);
			start_idx++;
		}
	} else
#endif
	{
		EXT_EVENT_RDD_IPI_HIST rdd_ipi_hist_rlt;
		os_zero_mem(&rdd_ipi_hist_rlt, sizeof(EXT_EVENT_RDD_IPI_HIST));
		/* Get IPI cnt */
		rdd_ipi_hist_rlt.band_idx = pAd->BgndScanCtrl.band_idx;
		mt_cmd_get_rdd_ipi_hist(pAd, RDD_IPI_HIST_ALL_CNT, &rdd_ipi_hist_rlt);
		for (ipi_idx = 0; ipi_idx < (IPI_HIST_TYPE_NUM - 1); ipi_idx++) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_WARN,
				"ipi[%d],\t ipi_hist_cnt=\t%d\n",
				ipi_idx,
				rdd_ipi_hist_rlt.ipi_hist_val[ipi_idx]);

		/* Get IPI2-IPI10 total cnt */
			if (ipi_th <= ipi_idx && ipi_idx <= RDD_IPI_HIST_10) {
				ipi_hist_val += rdd_ipi_hist_rlt.ipi_hist_val[ipi_idx];
			}

		}

		/* Get IPI free run cnt */
		free_cnt = rdd_ipi_hist_rlt.ipi_hist_val[RDD_IPI_FREE_RUN_CNT];
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"threshold ipi[%d], ipi cnt %d, free run cnt %d\n",
			ipi_th, ipi_hist_val, free_cnt);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"tx_assert_time %d (ms)\n", rdd_ipi_hist_rlt.tx_assert_time / 1000);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"Period %d (ms)\n", pAd->BgndScanCtrl.dfs_ipi_period);

		/* channel_load */
		ipi_percent = (INC_PRECISION * 100 * ipi_hist_val / free_cnt);
		if (ipi_percent >= 100 * INC_PRECISION)
			ipi_percent = 100 * INC_PRECISION;

		ipi_idle_ratio = ((100 * INC_PRECISION) - ipi_percent);

		period = pAd->BgndScanCtrl.dfs_ipi_period; /* ms */
		self_idle_ratio = (INC_PRECISION * 100 * (period - (rdd_ipi_hist_rlt.tx_assert_time / 1000))) / period;
		/* Channel_Load = (Self_Idle_Ratio - Idle_Ratio)/Self_Idle_Ratio */
		if (self_idle_ratio < ipi_idle_ratio)
			ch_load = 0;
		else
			ch_load = (self_idle_ratio - ipi_idle_ratio);

		if (self_idle_ratio <= threshold * INC_PRECISION) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"band[%d] - self_idle_ratio %d%% < threshold %d%%\n",
				rdd_ipi_hist_rlt.band_idx, self_idle_ratio/INC_PRECISION, threshold/INC_PRECISION);

			pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
			return;
		}

		ch_load = (100 * ch_load) / self_idle_ratio;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"ch_load %d%%, band[%d] - self_idle_ratio %d%%, idle_ratio %d%%\n",
			ch_load, rdd_ipi_hist_rlt.band_idx, self_idle_ratio/INC_PRECISION, ipi_idle_ratio/INC_PRECISION);
	}
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

	return;
}


#ifdef MT_DFS_SUPPORT
VOID DedicatedZeroWaitStartAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#if (RDD_2_SUPPORTED == 0)
	MT_BGND_SCAN_NOTIFY BgScNotify;
#endif
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#endif

	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;

#if (RDD_2_SUPPORTED == 0)
	/* Initialize */
	os_zero_mem(&BgScNotify, sizeof(MT_BGND_SCAN_NOTIFY));

	BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
	BgScNotify.BgndScanStatus = 1;/*start*/
	MtCmdBgndScanNotify(pAd, BgScNotify);

	/*Disable BF, MU*/
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 0);*/
	DynamicTxBfDisable(pAd, TRUE);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */
#endif /* RDD_2_SUPPORTED */

	/* Split into synA + synB */
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_START, ENUM_BGND_DFS_TYPE);

	/*Start out-band radar detection*/
	DfsDedicatedOutBandRDDStart(pAd);

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	*ch_stat = DFS_OUTB_CH_CAC;
#endif
}

VOID DedicatedZeroWaitRunningAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	CHAR OutBandCh;
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ch_stat %d\n", *ch_stat);
#endif
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "Get new outband DFS channel\n");
	DfsDedicatedOutBandRDDRunning(pAd);

	OutBandCh = GET_BGND_PARAM(pAd, OUTBAND_CH);
	if (OutBandCh == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR, "No available outband Channel\n");
		DedicatedZeroWaitStop(pAd, FALSE);
		return;
	}

	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_RUNNING, ENUM_BGND_DFS_TYPE);

	/* Start out-band radar detection */
	DfsDedicatedOutBandRDDStart(pAd);

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	/* Change channel state */
	switch (*ch_stat) {
	case DFS_INB_CH_SWITCH_CH:
	case DFS_INB_DFS_OUTB_CH_CAC:
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
		/* set new outband DFS channel stat */
		*ch_stat = DFS_INB_DFS_OUTB_CH_CAC;
		break;

	default:
		break;
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ch_stat %d\n", *ch_stat);
#endif

}

VOID DedicatedZeroWaitStop(
	IN RTMP_ADAPTER * pAd, BOOLEAN apply_cur_ch)
{
#if (RDD_2_SUPPORTED == 0)
	CHAR in_band_ch = 0;
#endif /* RDD_2_SUPPORTED */
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "start.\n");
#if (RDD_2_SUPPORTED == 0)
	if (apply_cur_ch == TRUE) {
		in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH);
	} else {
		in_band_ch = GET_BGND_PARAM(pAd, ORI_INBAND_CH);
	}

	if (in_band_ch == 0)
		return;
#endif /* RDD_2_SUPPORTED */

	if (!IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd))
		return;

	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;

	BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

	DfsDedicatedOutBandRDDStop(pAd);

	/* return to SynA only */
	if (ops->set_off_ch_scan)
		ops->set_off_ch_scan(pAd, CH_SWITCH_BACKGROUND_SCAN_STOP, ENUM_BGND_DFS_TYPE);

#if (RDD_2_SUPPORTED == 0)
	/*Enable BF, MU*/
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 1);*/
	DynamicTxBfDisable(pAd, FALSE);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */

#endif /* RDD_2_SUPPORTED */

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "end.\n");

}

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
VOID dfs_zero_wait_ch_init_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	PUCHAR ch_outband = &pAd->CommonCfg.DfsParameter.OutBandCh;
	PUCHAR phy_bw_outband = &pAd->CommonCfg.DfsParameter.OutBandBw;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UCHAR band_idx = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	struct wifi_dev *wdev = NULL;
	UINT_8 BssIdx = 0;
#endif

	BOOLEAN Cancelled;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "ch_stat %d\n", *ch_stat);
	/* Stop timer */
	RTMPCancelTimer(&pAd->BgndScanCtrl.DfsZeroWaitTimer, &Cancelled);

	switch (*ch_stat) {
	case DFS_OUTB_CH_CAC:
		DfsDedicatedOutBandSetChannel(pAd, *ch_outband, *phy_bw_outband, RDD_DEDICATED_RX);
		break;

	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		/* Assign DFS outband Channel to inband Channel */
		/* use channel to find band index */
		band_idx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "New inband channel %d bandidx %d\n",
					pDfsParam->OutBandCh, band_idx);

		/* Assign DFS outband Channel to inband Channel */
		*ch_stat = DFS_INB_CH_SWITCH_CH;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if ((pDfsParam->DFSChHitBand == DBDC_BAND0) || (pAd->CommonCfg.dbdc_mode)) {
			DfsDedicatedInBandSetChannel(
				pAd,
				pDfsParam->OutBandCh,
				pDfsParam->OutBandBw,
				FALSE,
				pDfsParam->DFSChHitBand);
		} else {
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				wdev->vht_sec_80_channel = pDfsParam->OutBandCh;
				wlan_config_set_cen_ch_2(wdev, DfsPrimToCent(pDfsParam->OutBandCh, BW_80));
			}
			DfsDedicatedInBandSetChannel(
				pAd,
				pDfsParam->band_ch[RDD_BAND0],
				pDfsParam->OutBandBw,
				FALSE,
				DBDC_BAND0);
		}
#else
		DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE, band_idx);
#endif
		break;

	default:
		break;
	}
}
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */
#endif

VOID BackgroundScanTest(
	IN PRTMP_ADAPTER pAd,
	IN MT_BGND_SCAN_CFG BgndScanCfg)
{
	/* Send Commad to MCU */
	MtCmdBgndScan(pAd, BgndScanCfg);
}

VOID ChannelQualityDetection(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ChBusyTime = 0;
	UINT32 MyTxAirTime = 0;
	UINT32 MyRxAirTime = 0;
	UCHAR BandIdx = 0;
	UINT32 lv0 = 0, lv1 = 0, lv2 = 0, lv3 = 0, lv4 = 0, lv5 = 0;
	UINT32 lv6 = 0, lv7 = 0, lv8 = 0, lv9 = 0, lv10 = 0, CrValue = 0;
	UINT32 Noisy = 0;
	UINT32 TotalIPI = 0;
	/* RTMP_REG_PAIR Reg[11]; */
	/* Phase 1: No traffic stat */
	/* Check IPI & Channel Busy Time */
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	ChBusyTime = pAd->OneSecMibBucket.ChannelBusyTime[BandIdx];
	MyTxAirTime = pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
	MyRxAirTime = pAd->OneSecMibBucket.MyRxAirtime[BandIdx];
	/* 1. Check Open enviroment via IPI (Band0) */
	HW_IO_READ32(pAd->hdev_ctrl, 0x12250, &lv0);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12254, &lv1);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12258, &lv2);
	HW_IO_READ32(pAd->hdev_ctrl, 0x1225c, &lv3);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12260, &lv4);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12264, &lv5);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12268, &lv6);
	HW_IO_READ32(pAd->hdev_ctrl, 0x1226c, &lv7);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12270, &lv8);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12274, &lv9);
	HW_IO_READ32(pAd->hdev_ctrl, 0x12278, &lv10);
	TotalIPI = lv0 + lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 + lv9 + lv10;

	if (TotalIPI != 0)
		Noisy = ((lv9 + lv10) * 100 / (TotalIPI));

	pAd->BgndScanCtrl.Noisy = Noisy;
	pAd->BgndScanCtrl.IPIIdleTime = TotalIPI;
	/* MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
	/* "Band0:lv0~5 %d, %d, %d, %d, %d, %d  lv6~10 %d, %d, %d, %d, %d tatol=%d, Noisy=%d, BusyTime=%d, MyTxAir=%d, MyRxAir=%d\n", */
	/* lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, TotalIPI, Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime); */
	HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_12, &CrValue);
	CrValue |= (1 << B0IrpiSwCtrlOnlyOffset); /*29*/
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, CrValue);/* Cleaer */
	HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_12, CrValue);/* Trigger again */

	if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_LISTEN)
		return;

	/* MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,"Noise =%d, ChBusy=%d, MyTxAirTime=%d, MyRxAirTime=%d\n", Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime); */
	if ((pAd->BgndScanCtrl.DriverTrigger) && ((Noisy > pAd->BgndScanCtrl.NoisyTH) && (TotalIPI > pAd->BgndScanCtrl.IPIIdleTimeTH))) {
		if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_IDLE) {
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
		} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, 0);
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
			/* BackgroundScanStart(pAd, 2); */
		}
	} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
		pAd->BgndScanCtrl.PartialScanIntervalCount++;

		if  (pAd->BgndScanCtrl.PartialScanIntervalCount >= pAd->BgndScanCtrl.PartialScanInterval
			 && (MyTxAirTime + MyRxAirTime < DefaultMyAirtimeUsageThreshold)) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_PARTIAL_SCAN);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}


#if OFF_CH_SCAN_SUPPORT
VOID mt_off_ch_scan(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR reason,
	IN UCHAR bgnd_scan_type)
{
#if (RDD_2_SUPPORTED == 0)
	EXT_CMD_OFF_CH_SCAN_CTRL_T offch_cmd_cfg;
	MT_BGND_SCAN_NOTIFY bgnd_scan_notify;
	MT_SWITCH_CHANNEL_CFG *curnt_swchcfg;
	UCHAR rx_stream_num = 0;
	UCHAR rx_path = 0;
	UCHAR rx_idx = 0;
	UCHAR control_ch_inband = 0, central_ch_inband = 0, bw_inband = 0;
	UCHAR control_ch_outband = 0, central_ch_outband = 0, bw_outband = 0;
#ifdef MT_DFS_SUPPORT
	CHAR out_band_ch = GET_BGND_PARAM(pAd, OUTBAND_CH);
	CHAR out_band_bw = GET_BGND_PARAM(pAd, OUTBAND_BW);
	CHAR in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH);
	CHAR in_band_bw = GET_BGND_PARAM(pAd, INBAND_BW);
#endif

	/* Initialize */
	os_zero_mem(&offch_cmd_cfg, sizeof(EXT_CMD_OFF_CH_SCAN_CTRL_T));
	os_zero_mem(&bgnd_scan_notify, sizeof(MT_BGND_SCAN_NOTIFY));

	/* Restore switch channel configuration */
	curnt_swchcfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);

	if (bgnd_scan_type == ENUM_BGND_BGND_TYPE) {
		/* SynA */
		control_ch_inband = curnt_swchcfg->ControlChannel;
		central_ch_inband = curnt_swchcfg->CentralChannel;
		bw_inband = curnt_swchcfg->Bw;

		/* SynB */
		control_ch_outband = pAd->BgndScanCtrl.ScanChannel;
		central_ch_outband = pAd->BgndScanCtrl.ScanCenChannel;
		bw_outband = pAd->BgndScanCtrl.ScanBW;
	}
#ifdef MT_DFS_SUPPORT
	else if (bgnd_scan_type == ENUM_BGND_DFS_TYPE) {
		/* SynA */
		control_ch_inband = in_band_ch;
		central_ch_inband = DfsPrimToCent(in_band_ch, in_band_bw);
		bw_inband = in_band_bw;

		/* SynB */
		control_ch_outband = out_band_ch;
		central_ch_outband = DfsPrimToCent(out_band_ch, out_band_bw);
		bw_outband = out_band_bw;
	}
#endif

	switch (reason) {
	case CH_SWITCH_BACKGROUND_SCAN_STOP:
		/* Return to SynA (3x3) only */
		/* RxStream to RxPath */
		rx_stream_num = curnt_swchcfg->RxStream;

		if (rx_stream_num > 3) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					 "illegal RxStreamNums(%d)\n", rx_stream_num);
			rx_stream_num = 3;
		}

		for (rx_idx = 0; rx_idx < rx_stream_num; rx_idx++)
			rx_path |= 1 << rx_idx;

		/* Fill synA offch_cmd_cfg */
		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;
		offch_cmd_cfg.work_tx_strm_pth = curnt_swchcfg->TxStream;
		offch_cmd_cfg.work_rx_strm_pth = rx_path;
		offch_cmd_cfg.dbdc_idx = 1;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_stop;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		/* Notify RA background scan stop */
		bgnd_scan_notify.NotifyFunc = (curnt_swchcfg->TxStream << 5 | 0xf);
		bgnd_scan_notify.BgndScanStatus = 0; /* stop */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "Background scan Notify NotifyFunc=%x, Status=%d\n",
				 bgnd_scan_notify.NotifyFunc, bgnd_scan_notify.BgndScanStatus);

		MtCmdBgndScanNotify(pAd, bgnd_scan_notify);

		/* Enable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */
		break;

	case CH_SWITCH_BACKGROUND_SCAN_START:
		bgnd_scan_notify.NotifyFunc =  (0x2 << 5 | 0xf);
		bgnd_scan_notify.BgndScanStatus = 1;/* start */
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				"Background scan Notify NotifyFunc=%x, Status=%d\n",
				bgnd_scan_notify.NotifyFunc, bgnd_scan_notify.BgndScanStatus);

		MtCmdBgndScanNotify(pAd, bgnd_scan_notify);

		/* Disable BF, MU */
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 0);
#endif

#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */

		/* Split into synA + synB */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;
		offch_cmd_cfg.mntr_tx_strm_pth = 1;
		offch_cmd_cfg.mntr_rx_strm_pth = 0x4; /* WF2 only */

		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;
		offch_cmd_cfg.work_tx_strm_pth = 2;
		offch_cmd_cfg.work_rx_strm_pth = 0x3; /* WF0 and WF1 */

		offch_cmd_cfg.dbdc_idx = 1;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_start;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	case CH_SWITCH_BACKGROUND_SCAN_RUNNING:
		/* Switch channel of synB */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;
		offch_cmd_cfg.mntr_tx_strm_pth = 1;
		offch_cmd_cfg.mntr_rx_strm_pth = 0x4; /* WF2 only */

		offch_cmd_cfg.dbdc_idx = 1;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_running;
		offch_cmd_cfg.is_aband = 1;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				 "ERROR reason=%d\n", reason);
		break;
	}
#endif

}
#endif

#if (RDD_2_SUPPORTED == 1)
VOID mt_off_ch_scan_dedicated(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR reason,
	IN UCHAR bgnd_scan_type)
{
	EXT_CMD_OFF_CH_SCAN_CTRL_T offch_cmd_cfg;
	MT_SWITCH_CHANNEL_CFG *curnt_swchcfg;
	UCHAR rx_stream_num = 0;
	UCHAR rx_path = 0;
	UCHAR rx_idx = 0;
	UCHAR control_ch_inband = 0, central_ch_inband = 0, bw_inband = 0;
	UCHAR control_ch_outband = 0, central_ch_outband = 0, bw_outband = 0;
    BOOLEAN is_aband = FALSE;
	UCHAR bgnd_band_scan_info = pAd->BgndScanCtrl.ScanType;
	UINT8 band_idx = 0;
	USHORT PhyMode = 0;
#ifdef MT_DFS_SUPPORT
	CHAR out_band_ch = GET_BGND_PARAM(pAd, OUTBAND_CH);
	CHAR out_band_bw = GET_BGND_PARAM(pAd, OUTBAND_BW);
	CHAR in_band_ch = 0;
	CHAR in_band_bw = 0;

	band_idx = ((bgnd_band_scan_info & BGND_BAND_IDX_MASK) >> BGND_BAND_IDX_SHFT);

	if (band_idx == BAND0) {
		in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH_BAND0);
		in_band_bw = GET_BGND_PARAM(pAd, INBAND_BW_BAND0);
	} else if (band_idx == BAND1) {
		in_band_ch = GET_BGND_PARAM(pAd, INBAND_CH_BAND1);
		in_band_bw = GET_BGND_PARAM(pAd, INBAND_BW_BAND1);
	}

#endif

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
	/* Initialize */
	os_zero_mem(&offch_cmd_cfg, sizeof(EXT_CMD_OFF_CH_SCAN_CTRL_T));

	/* Restore switch channel configuration */
	curnt_swchcfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[band_idx]);
	PhyMode = HcGetRadioPhyModeByBandIdx(pAd, band_idx);

	if (bgnd_scan_type == ENUM_BGND_BGND_TYPE) {
		/* SynA */
		control_ch_inband = curnt_swchcfg->ControlChannel;
		central_ch_inband = curnt_swchcfg->CentralChannel;
		bw_inband = curnt_swchcfg->Bw;

		/* Dedicated RX */
		control_ch_outband = pAd->BgndScanCtrl.ScanChannel;
		central_ch_outband = pAd->BgndScanCtrl.ScanCenChannel;
		bw_outband = pAd->BgndScanCtrl.ScanBW;
	}
#ifdef MT_DFS_SUPPORT
	else if (bgnd_scan_type == ENUM_BGND_DFS_TYPE) {
		/* SynA */
		control_ch_inband = in_band_ch;
		central_ch_inband = DfsPrimToCent(in_band_ch, in_band_bw);
		bw_inband = in_band_bw;

		/* Dedicated RX */
		control_ch_outband = out_band_ch;
		central_ch_outband = DfsPrimToCent(out_band_ch, out_band_bw);
		bw_outband = out_band_bw;
	}
#endif

    is_aband = (IsChABand(PhyMode, control_ch_outband) ? TRUE : FALSE);

	switch (reason) {
	case CH_SWITCH_BACKGROUND_SCAN_STOP:
		rx_stream_num = curnt_swchcfg->RxStream;

		for (rx_idx = 0; rx_idx < rx_stream_num; rx_idx++)
			rx_path |= 1 << rx_idx;

		/* Fill synA offch_cmd_cfg */
		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;
		offch_cmd_cfg.work_tx_strm_pth = curnt_swchcfg->TxStream;
		offch_cmd_cfg.work_rx_strm_pth = rx_path;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_stop;
		offch_cmd_cfg.is_aband = is_aband;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);
		break;

	case CH_SWITCH_BACKGROUND_SCAN_START:
		/* Dedicated RX */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;

		offch_cmd_cfg.work_prim_ch = control_ch_inband;
		offch_cmd_cfg.work_cntrl_ch = central_ch_inband;
		offch_cmd_cfg.work_bw = bw_inband;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_start;
		offch_cmd_cfg.is_aband = is_aband;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;
		offch_cmd_cfg.dbdc_idx = band_idx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
				 offch_cmd_cfg.work_prim_ch, offch_cmd_cfg.work_bw, offch_cmd_cfg.work_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	case CH_SWITCH_BACKGROUND_SCAN_RUNNING:
		/* Switch channel of dedicated RX */
		/* Fill in ext_cmd_param */
		offch_cmd_cfg.mntr_prim_ch = control_ch_outband;
		offch_cmd_cfg.mntr_cntrl_ch = central_ch_outband;
		offch_cmd_cfg.mntr_bw = bw_outband;

		offch_cmd_cfg.scan_mode = off_ch_scan_mode_running;
		offch_cmd_cfg.is_aband = is_aband;
		offch_cmd_cfg.off_ch_scn_type = off_ch_scan_simple_rx;
		offch_cmd_cfg.dbdc_idx = band_idx;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
				 offch_cmd_cfg.mntr_prim_ch, offch_cmd_cfg.mntr_bw, offch_cmd_cfg.mntr_cntrl_ch);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
				 "dbdc_idx:%d scan_mode:%d is_aband:%d\n",
				 offch_cmd_cfg.dbdc_idx, offch_cmd_cfg.scan_mode, offch_cmd_cfg.is_aband);

		mt_cmd_off_ch_scan(pAd, &offch_cmd_cfg);

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				 "ERROR reason=%d\n", reason);
		break;
	}

}

VOID bgnd_scan_ipi_cr_init(
	IN PRTMP_ADAPTER pAd)
{
	INT32 ret = 0;
	EXT_CMD_RDD_IPI_HIST_T cmd_rdd_ipi_hist;

	os_zero_mem(&cmd_rdd_ipi_hist, sizeof(EXT_CMD_RDD_IPI_HIST_T));

	/* clear histogram CR */
	cmd_rdd_ipi_hist.ipi_hist_idx = RDD_SET_IPI_CR_INIT;
	cmd_rdd_ipi_hist.set_val = GET_BGND_PARAM(pAd, OUTBAND_BW);
	ret = mt_cmd_set_rdd_ipi_hist(pAd, &cmd_rdd_ipi_hist);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
			"BW: %d, ret = %d\n", cmd_rdd_ipi_hist.set_val, ret);
}
#endif

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
VOID BfSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	struct wtbl_tx_rx_cap *trx_cap = &wtbl->trx_cap;
	/*struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);*/
	/* Search BF STA and record it. */
	start_idx = 1;
	end_idx = WTBL_MAX_NUM(pAd) - 1;

	if (enabled == 0) { /* Disable */
		wtbl_len = sizeof(WTBL_STRUC);
		os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

		if (!wtbl_raw_dw) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					 "AllocMem fail!\n");
			return;
		}

		for (idx = start_idx; idx <= end_idx; idx++) {
			wtbl_ent.wtbl_idx = idx;
			wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

			/* Read WTBL Entries */
			for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
				addr = wtbl_ent.wtbl_addr + wtbl_offset;
				HW_IO_READ32(pAd->hdev_ctrl, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
			}

			NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));

			if (trx_cap->wtbl_d2.field.tibf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 1; /* iBF */
			else if (trx_cap->wtbl_d2.field.tebf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 2; /* eBF */
			else
				pAd->BgndScanCtrl.BFSTARecord[idx] = 0; /* No BF */

			if (pAd->BgndScanCtrl.BFSTARecord[idx] != 0) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						 "Disable wcid %d BF!\n", idx);
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 0, 0, 0); /* Disable BF */
			}
		}

		os_free_mem(wtbl_raw_dw);
	} else {/* enable */
		for (idx = start_idx; idx <= end_idx; idx++) {
			if (pAd->BgndScanCtrl.BFSTARecord[idx] == 1) {/* iBF */
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						 "Enable wcid %d iBF!\n", idx);
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 1, 0, 0); /* enable iBF */
			} else if (pAd->BgndScanCtrl.BFSTARecord[idx] == 2) { /* eBF */
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO,
						 "Enable wcid %d eBF!\n", idx);
				CmdTxBfTxApplyCtrl(pAd, idx, 1, 0, 0, 0); /* enable eBF */
			}
		}
	}
}
#endif
#ifdef CFG_SUPPORT_MU_MIMO
VOID MuSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	if (enabled == 0)   /* Disable */
		SetMuEnableProc(pAd, "0");
	else   /* Enable */
		SetMuEnableProc(pAd, "1");
}
#endif /* CFG_SUPPORT_MU_MIMO */
#endif /* CONFIG_AP_SUPPORT */
