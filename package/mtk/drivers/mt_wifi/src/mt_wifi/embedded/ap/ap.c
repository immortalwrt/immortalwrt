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
****************************************************************************

   Module Name:
   soft_ap.c

   Abstract:
   Access Point specific routines and MAC table maintenance routines

   Revision History:
   Who         When          What
   --------    ----------    ----------------------------------------------
   John Chang  08-04-2003    created for 11g soft-AP

*/

#include "rt_config.h"
#define VLAN_HDR_LEN	4
#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT*/

#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
extern VOID vow_avg_pkt_len_reset(struct _RTMP_ADAPTER *ad);
extern VOID vow_avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry);
#endif

#ifdef IAPP_SUPPORT
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);
#endif /*IAPP_SUPPORT*/

static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad);
static VOID dynamic_sw_rps_control(struct _RTMP_ADAPTER *ad);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
extern UCHAR DfsV10BwToDfsBw(UCHAR Bw);
#endif

UCHAR IXIA_PROBE_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02};
char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated",
	"active countermeasures",
	"has disassociated with invalid PSK password"
};
UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}

static INT ap_mlme_set_capability(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN SpectrumMgmt = FALSE;
	UCHAR BandIdx;
#ifdef A_BAND_SUPPORT

	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if (WMODE_CAP_5G(wdev->PhyMode) && pAd->CommonCfg.bIEEE80211H == TRUE)
		SpectrumMgmt = TRUE;

#endif /* A_BAND_SUPPORT */
	BandIdx = HcGetBandByWdev(wdev);
	pMbss->CapabilityInfo = CAP_GENERATE(1,
								 0,
								 IS_SECURITY_Entry(wdev),
								 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
								 pAd->CommonCfg.bUseShortSlotTime[BandIdx],
								 SpectrumMgmt);
#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		pMbss->CapabilityInfo |= RRM_CAP_BIT;

#endif /* DOT11K_RRM_SUPPORT */

	if (pMbss->wdev.bWmmCapable == TRUE) {
		/*
		    In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
		    bit in the capability field of association, beacon and probe
		    management frames.
		*/
		/*          pMbss->CapabilityInfo |= 0x0200; */
	}
#ifdef UAPSD_SUPPORT
	if (pMbss->wdev.UapsdInfo.bAPSDCapable == TRUE) {
		/*
			QAPs set the APSD subfield to 1 within the Capability
			Information field when the MIB attribute
			dot11APSDOptionImplemented is true and set it to 0 otherwise.
			STAs always set this subfield to 0.
		*/
		pMbss->CapabilityInfo |= 0x0800;
	}
#endif /* UAPSD_SUPPORT */

	return TRUE;
}

static VOID do_sta_keep_action(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, BOOLEAN is_no_rx)
{
#ifdef IAPP_SUPPORT
#ifdef IXIA_C50_MODE
	if ((ad->ixia_ctl.iMode == VERIWAVE_MODE) && entry->wdev &&
		is_no_rx &&
		((ad->Mlme.PeriodicRound % STA_KEEP_ALIVE_NOTIFY_L2) == 0)) {
		IAPP_L2_Update_Frame_Send(ad, entry->Addr, entry->wdev->wdev_idx);
	}
#endif
#endif /*IAPP_SUPPORT*/
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
BOOLEAN ApAutoChannelSkipListBuild(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR chGrp = 0;
	BOOLEAN status = FALSE;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	UCHAR size = 0;

	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);

	if (pAd->ApCfg.bAutoChannelAtBootup[BandIdx]) {
		/* ACS Enable */
		if ((wdev->channel != 0) && (!IS_V10_OLD_CHNL_VALID(wdev))) {
			/* Non-Zero Channel in ACS */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Non-Zero Chnl in ACS\n");
			goto done;
		} else {
			AutoChannelSkipListClear(pAd);
			if (wlan_config_get_vht_bw(wdev) == VHT_BW_160) {
				if (pAd->CommonCfg.bCh144Enabled == FALSE) {
					AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
					AutoChannelSkipChannels(pAd, 1, 144);
				}
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
				if (pAd->CommonCfg.bCh144Enabled == FALSE) {
					AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
					AutoChannelSkipChannels(pAd, 1, 144);
				}
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
				if (pAd->CommonCfg.bCh144Enabled == FALSE)
					AutoChannelSkipChannels(pAd, 1, 144);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else
				return FALSE;
		}
	} else {
		/* ACS Disable */
		if (wdev->channel == 0) {
			/* No Channel in Non-ACS */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Zero Channel in Non ACS %d\n", wdev->channel);
			goto done;
		} else {
			/* Background ACS Algorithm = 3 */
			pAd->ApCfg.AutoChannelAlg[BandIdx] = ChannelAlgBusyTime;

			if (IS_V10_W56_GRP_VALID(pAd)) {
				if (wlan_config_get_vht_bw(wdev) == VHT_BW_160)
					chGrp = W56_160_UA;
				else
					chGrp = W56_UA;
			} else if (IS_V10_W56_VHT80_SWITCHED(pAd) && (pAd->CommonCfg.bCh144Enabled == FALSE))
				chGrp = W56_UC;
			else
				chGrp = DfsV10CheckChnlGrp(pAd, wdev, wdev->channel);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"CHgrp %d Channel %d\n", chGrp, wdev->channel);

			/* Clean Skip List */
			AutoChannelSkipListClear(pAd);

			if (chGrp == NA_GRP) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Illegal Group Number\n");
				return status;
			} else if (chGrp == W52_53) {
				if (!DfsV10CheckGrpChnlLeft(pAd, W53, V10_W53_SIZE, BandIdx)) {
					 AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				}
				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
			} else if (chGrp == W52) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "W52\n");
				AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (chGrp == W53) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "W53\n");

				if (DfsV10CheckGrpChnlLeft(pAd, W53, V10_W53_SIZE, BandIdx))
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
				else {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"W53 Channel Finish\n");
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				}

				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);

				status = TRUE;
			} else if (chGrp == W56_UA || chGrp == W56_UB || chGrp == W56_UC || chGrp == W56_160_UA || chGrp == W56_160_UB) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "W56\n");
				SET_V10_W56_GRP_VALID(pAd, FALSE);

				if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
					if (pAd->CommonCfg.bCh144Enabled)
						size = V10_W56_VHT80_SIZE;
					else
						size = V10_W56_VHT80_SIZE - V10_W56_VHT80_C_SIZE;

					if (IS_V10_W56_VHT80_SWITCHED(pAd) && (chGrp == W56_UC || chGrp == W56_160_UB))
						AutoChannelSkipChannels(pAd, size, GROUP3_LOWER);

					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					if (pAd->CommonCfg.bCh144Enabled == FALSE)
						AutoChannelSkipChannels(pAd, 1, 144);
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
					if (chGrp == W56_UC && (pAd->CommonCfg.bCh144Enabled == FALSE)) {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Incorrect Channel W56 C\n");
						return status;
					}

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[%s] W56_80\n");
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					if (pAd->CommonCfg.bCh144Enabled == FALSE) {
						AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
						AutoChannelSkipChannels(pAd, 1, 144);
					}
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_160) {
					if (chGrp == W56_160_UB && (pAd->CommonCfg.bCh144Enabled == FALSE)) {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"[%s]Incorrect Channel W56 B\n", __func__);
						return status;
					}

					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
					AutoChannelSkipChannels(pAd, 1, 144);
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				}
			}
		}
	}
done:
	return status;
}
#endif

static VOID ap_enable_rxtx(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==> ap_enable_rxtx\n");
	AsicSetRxFilter(pAd);

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<== ap_enable_rxtx\n");
}

UCHAR ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#define SINGLE_BAND 0
#define DUAL_BAND   1

	UCHAR NewChannel;
	UCHAR OriChannel;
	BOOLEAN IsAband;
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	UCHAR vht_bw;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR ht_bw;
	UCHAR phy_bw = 0;
#endif
#endif
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	BOOLEAN V10_ZW_DFS_INIT = FALSE;
#endif
#ifdef TR181_SUPPORT
	struct hdev_obj *hdev = NULL;
#endif /*TR181_SUPPORT*/
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* UINT8 BssIdx = 0; */
	PCHANNEL_CTRL pChCtrl = NULL;
#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "----------------->\n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\x1b[41m wdev is NULL !!\x1b[m\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<-----------------\n");
		return FALSE;
	}
	OriChannel = wdev->channel;
	vht_bw = wlan_config_get_vht_bw(wdev);
#ifdef TR181_SUPPORT
	hdev = (struct hdev_obj *)wdev->pHObj;
#endif /*TR181_SUPPORT*/

	BandIdx = HcGetBandByWdev(wdev);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AutoChannelBootup[%d] = %d\n",
			 BandIdx, pAd->ApCfg.bAutoChannelAtBootup[BandIdx]);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE)
				&& (IS_V10_APINTF_DOWN(pAd) == FALSE)) {
			if (ApAutoChannelSkipListBuild(pAd, wdev) == FALSE)
				return FALSE;
		}
#endif

	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* Now Enable RxTx*/
	ap_enable_rxtx(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PhyMode: %d\n", wdev->PhyMode);

	if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))
		IsAband = TRUE;
	else
		IsAband = FALSE;

	if ((wdev->channel == 0)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		|| (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE)))
#else
		)
#endif

	{
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (!pAd->ApCfg.bAutoChannelAtBootup[BandIdx] && !(IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE))) {
#else
		if (!pAd->ApCfg.bAutoChannelAtBootup[BandIdx]) {
#endif
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<-----------------\n");
			return FALSE;
		}

		/* Now we can receive the beacon and do ACS */
		if (pAutoChCtrl->AutoChSelCtrl.ACSChStat != ACS_CH_STATE_SELECTED) {
			/* Disable MibBucket during doing ACS */
			pAd->MsMibBucket.Enabled = FALSE;
			pAd->OneSecMibBucket.Enabled[BandIdx] = FALSE;

			NewChannel = APAutoSelectChannel(pAd, wdev, pAd->ApCfg.AutoChannelAlg[BandIdx], IsAband);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Auto channel selection: Selected channel = %d, IsAband = %d\n", NewChannel, IsAband);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (IS_SUPPORT_V10_DFS(pAd) && NewChannel) {
				SET_V10_BOOTACS_INVALID(pAd, TRUE);
			}
#endif
		} else {
			NewChannel = pAutoChCtrl->AutoChSelCtrl.SelCh;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[ApAutoChannelAtBootUp] ACS of BandIdx = %d is already DONE, Channel = %d\n", BandIdx, NewChannel);
		}
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		/* Update channel of wdev as new channel */
		AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);
#endif
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		ht_bw =  wlan_config_get_ht_bw(wdev);
		if (ht_bw == HT_BW_20)
			phy_bw = BW_20;
		else if (ht_bw == HT_BW_40) {
			if (vht_bw == VHT_BW_2040)
				phy_bw = BW_40;
			else if (vht_bw == VHT_BW_80)
				phy_bw = BW_80;
			else if (vht_bw == VHT_BW_160)
				phy_bw = BW_160;
			else if (vht_bw == VHT_BW_8080)
				phy_bw = BW_8080;
		}
		/*Update pAd->CommonCfg.DfsParameter*/
		DfsGetSysParameters(pAd, wdev, wdev->auto_channel_cen_ch_2, phy_bw);
#endif
		if (RadarChannelCheck(pAd, NewChannel))
			/* If DFS ch X is selected by ACS, CAC of DFS ch X will be checked by dedicated RX */
			zero_wait_dfs_update_ch(pAd, wdev, OriChannel, &NewChannel);

#endif
#endif

#ifdef DFS_ADJ_BW_ZERO_WAIT
		if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE) {
			Adj_ZeroWait_Status_Update(pAd, wdev, &NewChannel);
		}
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (!IS_SUPPORT_V10_DFS(pAd) && NewChannel)
#endif
		/*Update wdev channel again after zero-wait dfs update ch*/
		AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);

		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled[BandIdx] = TRUE;

	}
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	else {
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (!pAd->CommonCfg.dbdc_mode) {
			if (RadarChannelCheck(pAd, wdev->channel) && RadarChannelCheck(pAd, wdev->vht_sec_80_channel)) {
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"dual band are all DFS channel, disable zero-wait\n");
			}
		}
#endif

		if (WMODE_CAP_5G(wdev->PhyMode)) {
#ifdef DFS_ADJ_BW_ZERO_WAIT
			if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState)) {
				wdev->channel = 36;
				/* su forced mode */
				SetMuruSuTx(pAd, "1");
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				if (pChCtrl->ChList[4].NonOccupancy > 0) {
					if (vht_bw == VHT_BW_80) {
						pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_BW80_TX80RX80;
						pAd->CommonCfg.DfsParameter.OutBandBw = VHT_BW_80;
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Ch52 ~ ch64 detect radar before, switch to BW80\n");
					}
				}
			}
#endif
			if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0
#ifdef DFS_ADJ_BW_ZERO_WAIT
				&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == 0
#endif
				) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"zero-wait DFS is not enabled\n");
				return FALSE;
			}
		}

		if (IsAband == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "not A band\n");
			return FALSE;
		}
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		/*If synA is hit by radar at bootup time, bypass changing synB ch, redo the INBAND CH INIT*/
		if (pAd->CommonCfg.DfsParameter.inband_ch_stat == DFS_OUTB_CH_CAC) {
			V10_ZW_DFS_INIT = TRUE;
			pAd->CommonCfg.DfsParameter.inband_ch_stat = DFS_INB_CH_INIT;
			NewChannel = wdev->channel;
			AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);
		}
#endif
		/* If DFS ch X is set by profile, CAC of DFS ch X will be checked by dedicated RX */
		if (RadarChannelCheck(pAd, (wdev->channel))
#ifdef DFS_ADJ_BW_ZERO_WAIT
			&& pAd->CommonCfg.DfsParameter.BW160ZeroWaitState == 0
#endif
			) {
			if (zero_wait_dfs_update_ch(pAd, wdev, OriChannel, &(wdev->channel))) {

				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DFS ch %d is set, use non-DFS ch %d\n",
					pAd->CommonCfg.DfsParameter.OutBandCh, wdev->channel);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			/*Keep synA and synB channel are aligned.*/
			if ((pAd->CommonCfg.DfsParameter.inband_ch_stat  == DFS_INB_CH_INIT) && V10_ZW_DFS_INIT == TRUE)
				wdev->channel = NewChannel;
			else
#endif
				/* Update channel of wdev as new channel */
				AutoChSelUpdateChannel(pAd, wdev->channel, IsAband, wdev);

			} else {
				/* if zero-wait cac is ended, dedicated rx switch to new dfs ch */
				zero_wait_dfs_switch_ch(pAd, wdev, RDD_DEDICATED_RX);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<-------\n");
				return FALSE;
			}
		}
	}
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Chnl Restore Enbl %d\n", IS_V10_OLD_CHNL_VALID(wdev));
	if (!IS_V10_OLD_CHNL_VALID(wdev) && pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0)
		/* Update channel of wdev as new channel */
		AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);
#endif

	/* Check new channel is DFS channel or not */
	if (IsAband == TRUE)
		RadarStateCheck(pAd, wdev);

#endif /* MT_DFS_SUPPORT && BACKGROUND_SCAN_SUPPORT */

	if ((pAd->ApCfg.bAutoChannelAtBootup[BandIdx] == TRUE) && (vht_bw == VHT_BW_8080)) {
		wlan_config_set_cen_ch_2(wdev, wdev->auto_channel_cen_ch_2);
		wlan_operate_set_cen_ch_2(wdev, wdev->auto_channel_cen_ch_2);
    }

	/* sync to other device */
	wdev_sync_prim_ch(pAd, wdev);

	/* Update primay channel */
	wlan_operate_set_prim_ch(wdev, wdev->channel);
#ifdef TR181_SUPPORT
			pAd->ApBootACSChannelChangePerBandCount[BandIdx]++;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Boot:%d Total:%d\n",
					pAd->ApBootACSChannelChangePerBandCount[BandIdx],
					(hdev->rdev->pRadioCtrl->TotalChannelChangeCount +
						pAd->ApBootACSChannelChangePerBandCount[BandIdx]));
#endif /*TR181_SUPPORT*/

#ifdef MT_DFS_SUPPORT
	DfsBuildChannelList(pAd, wdev);
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	zero_wait_dfs_switch_ch(pAd, wdev, RDD_DEDICATED_RX);
#endif
#endif
#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)
	if (IS_MT7615(pAd) || IS_MT7626(pAd) || IS_MT7663(pAd)) {
		/*asic_enable_nf_support(pAd);*/
	}
#endif

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-----------------\n");
	return TRUE;
}


/*
	==========================================================================
	Description:
	Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
static BOOLEAN ApCheckLongPreambleSTA(RTMP_ADAPTER *pAd)
{
	UINT16   i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo)) {
			return TRUE;
		}
	}

	return FALSE;
}

/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
static VOID ap_run_at_boot(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (ApAutoChannelAtBootUp(pAd, wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"\x1b[41m ACS is disable !!\x1b[m\n");
	}

}

NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---> APOneShotSettingInitialize\n");
	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (!IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
#endif
		MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
	MulticastWLTableInit(pAd, &pAd->pMcastWLTable);
#ifdef IGMP_SNOOPING_DENY_LIST
	MulticastDLTableInit(pAd, &pAd->pMcastDLTable);
#endif
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
	AutoChBssTableInit(pAd);
	ChannelInfoInit(pAd);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerInit(pAd);
#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	/*
	    Some modules init must be called before APStartUp().
	    Or APStartUp() will make up beacon content and call
	    other modules API to get some information to fill.
	*/

#ifdef MAT_SUPPORT
	MATEngineInit(pAd);
#endif /* MAT_SUPPORT */
#ifdef CLIENT_WDS
	CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--- APOneShotSettingInitialize\n");
	return Status;
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---> APShutdown\n");
#ifdef MT_MAC
	/*	Disable RX */
	/* MtAsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE,0); */
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
	/* MlmeRadioOff(pAd); */
#else
	MlmeRadioOff(pAd);
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif
	/*remove sw related timer and table*/
	rtmp_ap_exit(pAd);


	NdisFreeSpinLock(&pAd->MacTabLock);
#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTab.WdsTabLock);
#endif /* WDS_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--- APShutdown\n");
}



/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */

VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd, struct _BSS_STRUCT *mbss)
{
	UCHAR  ErpIeContent = 0;
	UINT16 i;
	BOOLEAN bUseBGProtection;
	BOOLEAN LegacyBssExist;
	BOOLEAN bNeedBcnUpdate = FALSE;
	BOOLEAN b2GBand = TRUE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	USHORT *pCapInfo = NULL;
	struct wifi_dev *wdev = &mbss->wdev;
	UCHAR Channel = wdev->channel;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime[BandIdx];


	if (WMODE_EQUAL(PhyMode, WMODE_B))
		return;

	if (WMODE_CAP_5G(wdev->PhyMode))
		b2GBand = FALSE;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (wdev != pEntry->wdev)
			continue;


		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (b2GBand) {
				if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
				ShortSlotCapable = FALSE;
				ErpIeContent |= 0x01;
			}
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = FALSE;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;

	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
	   AUTO (0): UseProtection = 1 if any 11b STA associated
	   ON (1): always USE protection
	   OFF (2): always NOT USE protection
	   */
	if (b2GBand) {
		if (pAd->CommonCfg.UseBGProtection == 0) {
			ErpIeContent = (ErpIeContent) ? 0x03 : 0x00;

			/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
			if (LegacyBssExist) {
				ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
			}
		} else if (pAd->CommonCfg.UseBGProtection == 1)
			ErpIeContent |= 0x02;

		bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
						   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));
	} else {
		/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
		bUseBGProtection = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "-- bUseBGProtection: %s, BG_PROTECT_INUSED: %s, ERP IE Content: 0x%x\n",
			  (bUseBGProtection) ? "Yes" : "No",
			  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) ? "Yes" : "No",
			  ErpIeContent);

	if (b2GBand) {
		if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) {

			if (bUseBGProtection)
				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			else
				OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

		}
	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
		ErpIeContent |= 0x04;

	if (b2GBand) {
		if (ErpIeContent != pAd->ApCfg.ErpIeContent)
			bNeedBcnUpdate = TRUE;

		pAd->ApCfg.ErpIeContent = ErpIeContent;
	}

#ifdef A_BAND_SUPPORT

	/* Force to use ShortSlotTime at A-band */
	if (WMODE_CAP_5G(wdev->PhyMode))
		ShortSlotCapable = TRUE;

#endif /* A_BAND_SUPPORT */
	pCapInfo = &(mbss->CapabilityInfo);

	/* In A-band, the ShortSlotTime bit should be ignored. */
	if (ShortSlotCapable
#ifdef A_BAND_SUPPORT
		&& WMODE_CAP_2G(wdev->PhyMode)
#endif /* A_BAND_SUPPORT */
	   )
		(*pCapInfo) |= 0x0400;
	else
		(*pCapInfo) &= 0xfbff;

	if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
		(*pCapInfo) &= (~0x020);
	else
		(*pCapInfo) |= 0x020;


	/*update slot time only when value is difference*/
	if (pAd->CommonCfg.bUseShortSlotTime[BandIdx] != ShortSlotCapable) {
		HW_SET_SLOTTIME(pAd, ShortSlotCapable, Channel, wdev);
		pAd->CommonCfg.bUseShortSlotTime[BandIdx] = ShortSlotCapable;
	}

	if (bNeedBcnUpdate)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
}



static INT ap_hw_tb_init(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Reset WCID Table\n");
	HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Reset Sec Table\n");
	return TRUE;
}


#ifdef CONFIG_6G_SUPPORT
static INT ap_6g_cfg_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	pap_6g_cfg pap_cfg = &wdev->ap6g_cfg;
	pdiscov_iob dsc_iob = &pap_cfg->dsc_iob;
	pdiscov_oob dsc_oob = &pap_cfg->dsc_oob;

	NdisZeroMemory(pap_cfg, sizeof(ap_6g_cfg));

#ifdef DOT11V_MBSSID_SUPPORT
	/* disable discovery frame of non-transmitted bssid */
	if (IS_BSSID_11V_NON_TRANS(pAd, wdev->func_dev, HcGetBandByWdev(wdev))) {
		in_band_discovery_update(wdev, UNSOLICIT_TX_DISABLE, 0, UNSOLICIT_TXMODE_NON_HT,
			wlan_config_get_unsolicit_tx_by_cfg(wdev));
	}
#endif

	/* 6g: in-of-band buf */
	if (!dsc_iob->pkt_buf) {
		NdisAllocateSpinLock(pAd, &dsc_iob->pkt_lock);
		Status = MlmeAllocateMemory(pAd, (PVOID)&dsc_iob->pkt_buf);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "alloc discovery pkt_buf 0x%p\n", dsc_iob->pkt_buf);
		if (Status == NDIS_STATUS_FAILURE)
			return FALSE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"discovery pkt_buf allocated!\n");
	}

	/* 6g: out-of-band buf */
	if (!dsc_oob->repted_bss_list) {
		NdisAllocateSpinLock(pAd, &dsc_oob->list_lock);
		dsc_oob->repted_bss_cnt = 0;
		dsc_oob->repted_bss_list = vmalloc(sizeof(struct _repted_bss_info) * MAX_REPTED_BSS_CNT);
		if (dsc_oob->repted_bss_list == NULL)
			return FALSE;
		RTMPZeroMemory(dsc_oob->repted_bss_list, sizeof(struct _repted_bss_info) * MAX_REPTED_BSS_CNT);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "alloc repted_bss_list 0x%p\n", dsc_oob->repted_bss_list);
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"reported bss_list_buf allocated!\n");
	}

	return TRUE;
}

static INT ap_6g_cfg_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	pap_6g_cfg pap_cfg = &wdev->ap6g_cfg;
	pdiscov_iob dsc_iob = &pap_cfg->dsc_iob;
	pdiscov_oob dsc_oob = &pap_cfg->dsc_oob;

	/* de-initial for 6g in-of-band buf */
	if (dsc_iob->pkt_buf) {
		RTMP_SEM_LOCK(&dsc_iob->pkt_lock);
		MlmeFreeMemory(dsc_iob->pkt_buf);
		dsc_iob->pkt_buf = NULL;
		RTMP_SEM_UNLOCK(&dsc_iob->pkt_lock);

		NdisFreeSpinLock(&dsc_iob->pkt_lock);
	}

	/* de-initial for 6g out-of-band buf */
	if (dsc_oob->repted_bss_list) {
		RTMP_SEM_LOCK(&dsc_oob->list_lock);
		vfree(dsc_oob->repted_bss_list);
		dsc_oob->repted_bss_list = NULL;
		dsc_oob->repted_bss_cnt = 0;
		RTMP_SEM_UNLOCK(&dsc_oob->list_lock);

		NdisFreeSpinLock(&dsc_oob->list_lock);
	}

	return TRUE;
}
#endif

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */

VOID APStartUpForMbss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN bWmmCapable = FALSE;
	EDCA_PARM *pEdca = NULL, *pBssEdca = NULL;
	UCHAR phy_mode = pAd->CommonCfg.cfg_wmode;
	UCHAR ucBandIdx = 0;
	UINT8 wifi_cert_program = 0;
#ifdef TX_POWER_CONTROL_SUPPORT
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
#endif
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "===>(caller:%pS), mbss_idx:%d, CfgMode:%d\n", OS_TRACE, pMbss->mbss_idx, phy_mode);

	ucBandIdx = HcGetBandByWdev(wdev);
#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, ucBandIdx);
#endif /* LED_CONTROL_SUPPORT */

#ifdef ANTENNA_CONTROL_SUPPORT
	Antenna_Control_Init(pAd, wdev);
#endif /* ANTENNA_CONTROL_SUPPORT */

	/*Ssid length sanity check.*/
	if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
		pMbss->Ssid[5] = '0' + pMbss->mbss_idx;
		pMbss->SsidLen = 6;
	}

	if (wdev->func_idx == 0)
		MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);

	APSecInit(pAd, wdev);

#ifdef MWDS
	if (wdev->bDefaultMwdsStatus == TRUE)
		MWDSEnable(pAd, pMbss->mbss_idx, TRUE, TRUE);
#endif /* MWDS */

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	/* Deduce IPv6 Link local address for this MBSS & IPv6 format checksum for use in MLD query message*/
	calc_mldv2_gen_query_chksum(pAd, pMbss);
#endif

#if defined(WAPP_SUPPORT)
	wapp_init(pAd, pMbss);
#endif

	ap_mlme_set_capability(pAd, pMbss);
#ifdef WSC_V2_SUPPORT

	if (wdev->WscControl.WscV2Info.bEnableWpsV2) {
		/*
		    WPS V2 doesn't support Chiper WEP and TKIP.
		*/
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
			|| (pMbss->bHideSsid))
			WscOnOff(pAd, wdev->func_idx, TRUE);
		else
			WscOnOff(pAd, wdev->func_idx, FALSE);
	}

#endif /* WSC_V2_SUPPORT */

	/* If any BSS is WMM Capable, we need to config HW CRs */
	if (wdev->bWmmCapable)
		bWmmCapable = TRUE;

	if (WMODE_CAP_N(wdev->PhyMode) || bWmmCapable) {
		pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];

		/* EDCA parameters used for AP's own transmission */
		if (pEdca->bValid == FALSE)
			set_default_ap_edca_param(pEdca);

		pBssEdca = wlan_config_get_ht_edca(wdev);

		if (pBssEdca) {
			/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
			if (pBssEdca->bValid == FALSE)
				set_default_sta_edca_param(pBssEdca);
		}
	}

#ifdef DOT11_N_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif

	/*update rate info for wdev*/
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G))))
		RTMPUpdateGNRateInfo(wdev->PhyMode, &wdev->rate);
	else
#endif /*GN_MIXMODE_SUPPORT*/
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);

	if (!(WMODE_CAP_N(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)))
		wlan_config_set_ht_bw(wdev, HT_BW_20);

#ifdef DOT11N_DRAFT3
	/*
	    We only do this Overlapping BSS Scan when system up, for the
	    other situation of channel changing, we depends on station's
	    report to adjust ourself.
	*/

	if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				  pAd->CommonCfg.bBssCoexEnable,
				  pAd->CommonCfg.bForty_Mhz_Intolerant);
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
				 pAd->CommonCfg.bBssCoexEnable);
		APOverlappingBSSScan(pAd, wdev);
	}

	if (pAd->CommonCfg.wifi_cert) {
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

		wifi_cert_program = pAd->CommonCfg.wifi_cert; /* Please follow the program enum in capi.h */
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_CERT_CFG_FEATURE, &wifi_cert_program, sizeof(wifi_cert_program));

		/* respond with one Probe response per request */
		chip_cap->ProbeRspTimes = 1;
	}

#ifdef CONFIG_CPE_SUPPORT
	if (pAd->CommonCfg.powerBackoff[ucBandIdx] != 0) {
		INT8 powerBackoff = 0;
		powerBackoff = pAd->CommonCfg.powerBackoff[ucBandIdx];
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_POWER_BACKOFF_FEATURE, &powerBackoff, sizeof(powerBackoff));
	}
#endif

#ifdef CONFIG_3_WIRE_SUPPORT
	if (pAd->CommonCfg.ThreeWireFunctionEnable < 4) {
		UINT8 threeWireFunctionEnable = 0;

		threeWireFunctionEnable = pAd->CommonCfg.ThreeWireFunctionEnable;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_3WIRE_EXT_EN_CFG_FEATURE, &threeWireFunctionEnable, sizeof(threeWireFunctionEnable));
	}
#endif

#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/
	MlmeUpdateTxRates(pAd, FALSE, wdev->func_idx);
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, wdev);

#endif /* DOT11_N_SUPPORT */

	if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
		if (wdev_do_linkup(wdev, NULL) != TRUE)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "link up fail!!\n");
	}

#ifdef SINGLE_SKU_V2
	/* enable/disable SKU via profile */
	TxPowerSKUCtrl(pAd, pAd->CommonCfg.SKUenable[ucBandIdx], ucBandIdx);

	/* enable/disable BF Backoff via profile */
	TxPowerBfBackoffCtrl(pAd, pAd->CommonCfg.BFBACKOFFenable[ucBandIdx], ucBandIdx);
#endif /* SINGLE_SKU_V2*/
	/* enable/disable Power Percentage via profile */
	TxPowerPercentCtrl(pAd, pAd->CommonCfg.PERCENTAGEenable[ucBandIdx], ucBandIdx);

	/* Tx Power Percentage value via profile */
	TxPowerDropCtrl(pAd, pAd->CommonCfg.ucTxPowerPercentage[ucBandIdx], ucBandIdx);

	/* Config Tx CCK Stream */
	TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream[ucBandIdx], ucBandIdx);

#ifdef TX_POWER_CONTROL_SUPPORT
	if (arch_ops && arch_ops->arch_txpower_boost)
		arch_ops->arch_txpower_boost(pAd, ucBandIdx);
#endif /* TX_POWER_CONTROL_SUPPORT */

	/*init tx/rx stream */
	hc_set_rrm_init(wdev);
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_check_device_hook)
		wf_drv_tbl.wf_fwd_check_device_hook(wdev->if_dev, INT_MBSSID, pMbss->mbss_idx, wdev->channel, 1);

#endif /* CONFIG_WIFI_PKT_FWD */


#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	/* Update bInitMbssZeroWait for MBSS Zero Wait */
	UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
#ifdef VOW_SUPPORT
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (pAd->ApCfg.MBSSID[wdev->func_idx].APStartPseduState == AP_STATE_ALWAYS_START_AP_DEFAULT)
			vow_mbss_init(pAd, wdev);
#else
	vow_mbss_init(pAd, wdev);
#endif
#endif /* VOW_SUPPORT */

#ifdef GREENAP_SUPPORT
	greenap_check_when_ap_bss_change(pAd);
#endif /* GREENAP_SUPPORT */

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
	if (pAd->ApCfg.BandSteering) {
		PBND_STRG_CLI_TABLE table;

		table = Get_BndStrgTable(pAd, pMbss->mbss_idx);

		if (!table) {
			/* Init time table may not init as wdev not populated */
			BndStrg_TableInit(pAd, pMbss->mbss_idx);
			table = Get_BndStrgTable(pAd, pMbss->mbss_idx);
		}

		if (table) {
		    /* Inform daemon interface ready */
		    BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, TRUE);
		}
	}
#ifdef SPECIAL_11B_OBW_FEATURE
	if (wdev->channel >= 1 && wdev->channel <= 14) {
		MtCmdSetTxTdCck(pAd, TRUE);
	}
#endif /* SPECIAL_11B_OBW_FEATURE */
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */

#ifdef CONFIG_DOT11U_INTERWORKING
	pMbss->GASCtrl.b11U_enable = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */

	pMbss->ShortSSID = Crcbitbybitfast(pMbss->Ssid, pMbss->SsidLen);

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev)) {
		BOOLEAN Cancelled;
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (IS_OCE_FD_FRAME_ENABLE(wdev) &&
			IS_FD_FRAME_FW_MODE(cap)) {
			OceSendFilsDiscoveryAction(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx].wdev);
		}

		if (IS_OCE_RNR_ENABLE(wdev))
			RTMPSetTimer(&pAd->ApCfg.APAutoScanNeighborTimer, OCE_RNR_SCAN_PERIOD);
		else
			RTMPCancelTimer(&pAd->ApCfg.APAutoScanNeighborTimer, &Cancelled);
	}
#endif /* OCE_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev, WAPP_BSS_START);
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */

#ifdef CFG_SUPPORT_FALCON_MURU
	/* muru_tam_arb_op_mode(pAd); */
	muru_update_he_cfg(pAd);
#endif
#ifdef CFG_SUPPORT_FALCON_SR
	/* Spatial Reuse initialize via profile */
	SrMbssInit(pAd, wdev);
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CFG_SUPPORT_FALCON_PP
	/* Preamble puncture initialize via profile */
	pp_mbss_init(pAd, wdev);
#endif /* CFG_SUPPORT_FALCON_SR */


#ifdef DSCP_PRI_SUPPORT
	/*write CR4 for DSCP user prio and flag*/
	if (pMbss->dscp_pri_map_enable) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			MtCmdSetDscpPri(pAd, pMbss->mbss_idx);
			set_cp_support_en(pAd, "1");
		}
#endif /*MT7915*/
	}
#endif /*DSCP_PRI_SUPPORT*/

	/* EDCCA mode init */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>edcca initial\n");
	EDCCAInit(pAd, ucBandIdx);

#ifdef MGMT_TXPWR_CTRL
	/* read base pwr for CCK/OFDM rate */
	wdev->bPwrCtrlEn = FALSE;
	wdev->TxPwrDelta = 0;
	wdev->mgmt_txd_txpwr_offset = 0;
	pAd->ApCfg.MgmtTxPwr[ucBandIdx] = 0;
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);

	/* Update beacon/probe TxPwr wrt profile param */
	if (wdev->MgmtTxPwr) {
		/* wait until TX Pwr event rx*/
		if (!(pAd->ApCfg.MgmtTxPwr[ucBandIdx]))
			RtmpusecDelay(50);

		update_mgmt_frame_power(pAd, wdev);
	}
#else
#if defined(TPC_SUPPORT) && defined(TPC_MODE_CTRL)
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);
#endif
#endif

#ifdef DATA_TXPWR_CTRL
	if (pAd->ApCfg.MinBaseTxPwr[ucBandIdx]) {
		if (TxPwrMinLimitDataFrameCtrl(pAd, (INT8)pAd->ApCfg.MinBaseTxPwr[ucBandIdx], ucBandIdx))
			MTWF_PRINT("%s: Minimum power of Band %d is %d\n",
				__func__, ucBandIdx, pAd->ApCfg.MinBaseTxPwr[ucBandIdx] / 2);
		else
			MTWF_PRINT("%s: Unable to set minimum power Band = %d.\n", __func__, ucBandIdx);
	}
#endif

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===mbss_idx %d, pNetDev = %p\n",
			 pMbss->mbss_idx, wdev->if_dev);
}

VOID APStartUpByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;

#ifdef DBDC_ONE_BAND1_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>mbss_idx %d\n", pMbss->mbss_idx);
	wdev = &pMbss->wdev;

#ifdef DBDC_ONE_BAND1_SUPPORT
	if (cap->DbdcOneBand1Support && (wdev->func_idx == 0)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "no need do this for wdev0!\n");
		return;
	}
#endif

	/* setup tx preamble */
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

	/* Update devinfo for any phymode change */
	if (wdev_do_open(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open fail!!!\n");

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */
	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
	pAd->BcnCheckInfo[DBDC_BAND0].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pAd->BcnCheckInfo[DBDC_BAND1].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);

	/* start sending BEACON out */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (pMbss && (pMbss->APStartPseduState == AP_STATE_ALWAYS_START_AP_DEFAULT))
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IF_STATE_CHG);
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, wdev);

#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);
#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n");

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStart(pAd);
#endif /* SMART_ANTENNA */
	chip_interrupt_enable(pAd);

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, HcGetBandByWdev(wdev));
#endif /* LED_CONTROL_SUPPORT */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Main bssid = "MACSTR"\n",
			 MAC2STR(pAd->ApCfg.MBSSID[BSS0].wdev.bssid));
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===\n");
}

VOID ap_over_lapping_scan(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;
	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}
	wdev = &pMbss->wdev;
	if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				 pAd->CommonCfg.bBssCoexEnable,
				 pAd->CommonCfg.bForty_Mhz_Intolerant);
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
					pAd->CommonCfg.bBssCoexEnable);
		APOverlappingBSSScan(pAd, wdev);
	}
}
VOID APStartUp(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR i, j;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev_temp = NULL;
	struct wifi_dev *sta_wdev = NULL;
#endif

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid oper(%d)\n", oper);
		return;
	}

	if (pMbss)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(caller:%pS), oper(%d) bssid(%d)="MACSTR"\n",
				 OS_TRACE, oper, pMbss->mbss_idx, MAC2STR(pMbss->wdev.bssid));
	else
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(caller:%pS), oper(%d)\n",
				 OS_TRACE, oper);

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			APStartUpByBss(pAd, pCurMbss);
		}
		break;
	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s() wdev idx (%d) skip ApStart due to wrong up down state\n", __func__, pCurMbss->wdev.wdev_idx);
				continue;
			}

			if (pCurMbss->wdev.open_state == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s(), wdev idx (%d) skip ApStart as device is not open\n", __func__, pCurMbss->wdev.wdev_idx);
				continue;
			}

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != pMbss->wdev.channel) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s() wdev idx (%d) skip ApStart due to channel mismatch %d %d \n",
						__func__, pCurMbss->wdev.wdev_idx, pCurMbss->wdev.channel, pMbss->wdev.channel);
				continue;
			}

			pCurMbss->wdev.start_stop_running = TRUE;
#ifdef CONFIG_MAP_SUPPORT
			if (pCurMbss->is_bss_stop_by_map)
				continue;
#endif
			APStartUpByBss(pAd, pCurMbss);
			if (pCurMbss->wdev.open_state == FALSE)
				RTMP_OS_COMPLETE(&pCurMbss->wdev.start_stop_complete);
			pCurMbss->wdev.start_stop_running = FALSE;
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStartUpByBss(pAd, pMbss);
		if (pMbss->wdev.if_up_down_state && pMbss->wdev.channel < 14)
			ap_over_lapping_scan(pAd, pMbss);
		break;
	}
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
#ifdef CONFIG_MAP_SUPPORT
/*If this AP stop start is due to radar detect then send the radar notify event to MAP*/
	if (IS_MAP_TURNKEY_ENABLE(pAd) && pMbss) {
		ch = pMbss->wdev.channel;
		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev_temp = &pAd->ApCfg.MBSSID[i].wdev;
			if ((wdev_temp == NULL) || (wdev_temp->pDot11_H == NULL) || (!wdev_temp->if_dev)) {
				continue;
			}

			pDot11h = wdev_temp->pDot11_H;
			/* check MBSS status is up */
			if (!wdev_temp->if_up_down_state)
				continue;
			/* check MBSS work on the same RF(channel) */
			if (wdev_temp->channel != ch)
				continue;
			if (pDot11h->RDMode == RD_SILENCE_MODE) {
				if (wdev_temp->cac_not_required == TRUE)
					wapp_send_cac_period_event(pAd, RtmpOsGetNetIfIndex(wdev_temp->if_dev), wdev_temp->channel, 2, 0);
				else
					wapp_send_cac_period_event(pAd, RtmpOsGetNetIfIndex(wdev_temp->if_dev), wdev_temp->channel, 1, pDot11h->cac_time);
				for (j = 0; j < MAX_APCLI_NUM; j++) {
					sta_wdev = &pAd->StaCfg[j].wdev;
					if (sta_wdev->channel == wdev_temp->channel) {
						pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
					}
				}
			}
			if (wdev_temp->map_radar_detect) {
				wapp_send_radar_detect_notif(pAd, wdev_temp, wdev_temp->map_radar_channel, wdev_temp->map_radar_bw, 0);
				wdev_temp->map_radar_detect = 0;
				wdev_temp->map_radar_channel = 0;
				wdev_temp->map_radar_bw = 0;
			}
#ifdef WIFI_MD_COEX_SUPPORT
			if (wdev_temp->map_lte_unsafe_ch_detect) {
				wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
				wdev_temp->map_lte_unsafe_ch_detect = 0;
			}
#endif
		}
	}
#endif
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===\n");
}

/*Only first time will run it*/
VOID APInitForMain(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = NULL;
	UINT32 apidx = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>\n");
	AsicDisableSync(pAd, HW_BSSID_0);/* don't gen beacon, reset tsf timer, don't gen interrupt. */
	/*reset hw wtbl*/
	ap_hw_tb_init(pAd);
	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	pMbss->mbss_idx = MAIN_MBSSID;
	/*update main runtime attribute*/

#ifdef DOT11_N_SUPPORT
	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#ifdef PIGGYBACK_SUPPORT
	AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */
#endif /* DOT11_N_SUPPORT */
	/* Workaround end */
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, HcGetBandByWdev(&pMbss->wdev));
#endif /* LED_CONTROL_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */
	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
	pAd->BcnCheckInfo[DBDC_BAND0].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pAd->BcnCheckInfo[DBDC_BAND1].BcnInitedRnd = pAd->Mlme.PeriodicRound;
#ifdef KERNEL_RPS_ADJUST
	if (!pAd->ixia_mode_ctl.kernel_rps_en)
#endif
	{
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			UINT8 idx = 0;
			for (idx = 0; idx < DBDC_BAND_NUM; idx++)
				pAd->mcli_ctl[idx].tx_cnt_from_red = TRUE;
		} else {
			UINT8 idx = 0;
			for (idx = 0; idx < DBDC_BAND_NUM; idx++)
				pAd->mcli_ctl[idx].tx_cnt_from_red = FALSE;
		}
	}
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, &pMbss->wdev);
	/* RadarStateCheck(pAd); */

#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		OPSTATUS_SET_FLAG_WDEV(&pMbss->wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	}

#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n");

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStart(pAd);
#endif /* SMART_ANTENNA */


	chip_interrupt_enable(pAd);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Main bssid = "MACSTR"\n",
			 MAC2STR(pAd->ApCfg.MBSSID[BSS0].wdev.bssid));
}

VOID APStopByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	BOOLEAN Cancelled;
#if defined(MESH_SUPPORT) || defined(APCLI_SUPPORT)
	INT idx = 0;
	struct wifi_dev *wdev = NULL;
#endif	/* MESH_SUPPORT || CONFIG_STA_SUPPORT */
	struct wifi_dev *wdev_bss = NULL;
#ifdef WSC_INCLUDED
	PWSC_CTRL pWscControl;
#endif /* WSC_INCLUDED */

#ifdef DBDC_ONE_BAND1_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Mbss\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>mbss_idx %d\n", pMbss->mbss_idx);
	wdev_bss = &pMbss->wdev;

#ifdef DBDC_ONE_BAND1_SUPPORT
	if (cap->DbdcOneBand1Support && (wdev_bss->func_idx == 0)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "no need do this for wdev0!\n");
		return;
	}
#endif

	if (wdev_bss->if_dev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev(%d)\n", wdev_bss->wdev_idx);
		return;
	}

	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE) {
		/* make sure CarrierDetect wont send CTS */
		CarrierDetectionStop(pAd);
	}

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef WAPP_SUPPORT
	if (wdev_bss->avoid_apcli_linkdown != TRUE) {
#endif
	{
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		if (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, HcGetBandByWdev(wdev_bss)) == FALSE) {
#endif
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				wdev = &pAd->StaCfg[idx].wdev;
#ifdef WSC_INCLUDED
				/* WPS cli will disconnect and connect again */
				pWscControl = &pAd->StaCfg[idx].wdev.WscControl;
				if (pWscControl->bWscTrigger == TRUE)
					continue;
#endif /* WSC_INCLUDED */
				if (wdev->channel == wdev_bss->channel) {
					UINT8 enable = pAd->StaCfg[idx].ApcliInfStat.Enable;

					if (enable) {
						pAd->StaCfg[idx].ApcliInfStat.Enable = FALSE;
						ApCliIfDown(pAd);
						pAd->StaCfg[idx].ApcliInfStat.Enable = enable;
					}
				}
			}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		}
#endif
	}
#ifdef WAPP_SUPPORT
	}
#endif
#endif /* APCLI_SUPPORT */

#ifdef WIFI_TWT_SUPPORT
	twt_release_btwt_resource(wdev_bss);
#endif /* WIFI_TWT_SUPPORT */

	/*AP mode*/
	/*For Tgac 4.2.16h test sniffer do check, after CSA AP should*/
	/*not send further beacon with/without CSAIE even if CSA IE*/
	/*is updated in beacon template after firmware send the CSA*/
	/*done event in next preTBTT interupt beacon queue in firmware*/
	/*get enabled so first diabling the queue then update the IE*/
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>Disabling BCN\n");
	UpdateBeaconHandler(pAd, wdev_bss, BCN_UPDATE_DISABLE_TX);

	MacTableResetWdev(pAd, wdev_bss);
	OPSTATUS_CLEAR_FLAG_WDEV(wdev_bss, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

#ifdef GREENAP_SUPPORT
	greenap_check_when_ap_bss_change(pAd);
#endif /* GREENAP_SUPPORT */

	CMDHandler(pAd);
	/* Disable pre-tbtt interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);
	/* Disable piggyback */
	AsicSetPiggyBack(pAd, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		AsicDisableSync(pAd, HW_BSSID_0);
#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN, HcGetBandByWdev(wdev_bss));
#endif /* LED_CONTROL_SUPPORT */
	}

#ifdef MWDS
	MWDSDisable(pAd, pMbss->mbss_idx, TRUE, TRUE);
#endif /* MWDS */

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd))
		map_a4_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_entry_delete_hook)
		wf_drv_tbl.wf_fwd_entry_delete_hook(pAd->net_dev, wdev_bss->if_dev, 0);

	if (wf_drv_tbl.wf_fwd_check_device_hook)
		wf_drv_tbl.wf_fwd_check_device_hook(wdev_bss->if_dev,
			INT_MBSSID, pMbss->mbss_idx, wdev_bss->channel, 0);

#endif /* CONFIG_WIFI_PKT_FWD */
	/* clear protection to default */
	wdev_bss->protection = 0;

	/* clear csa cnt */
	wdev_bss->csa_count = 0;

#ifdef CONFIG_RCSA_SUPPORT
	/* When RCSA is send, ALTX is en and BF suspended, restore state*/
	rcsa_recovery(pAd, wdev_bss);
#endif

	if (wdev_do_linkdown(wdev_bss) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkdown fail!!!\n");

	if (WMODE_CAP_5G(wdev_bss->PhyMode)) {
#ifdef MT_DFS_SUPPORT /* Jelly20150217 */
		WrapDfsRadarDetectStop(pAd);
		/* Zero wait hand off recovery for CAC period + interface down case */
		DfsZeroHandOffRecovery(pAd, wdev_bss);
#endif
	}

	/* Update devinfo for any phymode change */
	if (wdev_do_close(wdev_bss) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "close fail!!!\n");

	APReleaseRekeyTimer(pAd, wdev_bss);
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		PBND_STRG_CLI_TABLE table;

		table = Get_BndStrgTable(pAd, pMbss->mbss_idx);
		if (table) {
			/* Inform daemon interface down */
			BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
		}
	}
#endif /* BAND_STEERING */

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		pAd->ApCfg.BANClass3Data = FALSE;
	}

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStop(pAd);
#endif /* SMART_ANTENNA */
#ifdef VOW_SUPPORT
	vow_reset(pAd);
#endif /* VOW_SUPPORT */
#ifdef OCE_SUPPORT
	OceDeInit(pAd, wdev_bss);
#endif /* OCE_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev_bss, WAPP_BSS_STOP);
#endif /*WAPP_SUPPORT*/
#endif /*CONFIG_MAP_SUPPORT WAPP_SUPPORT*/

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===mbss_idx %d\n", pMbss->mbss_idx);

}

VOID APStop(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid oper(%d)\n", oper);
		return;
	}

	if (pMbss)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(caller:%pS), oper(%d) bssid(%d)="MACSTR"\n",
				 OS_TRACE, oper, pMbss->mbss_idx, MAC2STR(pMbss->wdev.bssid));
	else
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(caller:%pS), oper(%d)\n",
				 OS_TRACE, oper);

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			APStopByBss(pAd, pCurMbss);
		}
		break;
	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state && !pCurMbss->wdev.bcn_buf.bBcnSntReq)
				continue;

			if (pCurMbss->wdev.open_state == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s(), wdev idx (%d) skip ApStop as device is not open\n", __func__, pCurMbss->wdev.wdev_idx);
				continue;
			}

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != ch)
				continue;
			pCurMbss->wdev.start_stop_running = TRUE;
			APStopByBss(pAd, pCurMbss);
			if (pCurMbss->wdev.open_state == FALSE)
				RTMP_OS_COMPLETE(&pCurMbss->wdev.start_stop_complete);
			pCurMbss->wdev.start_stop_running = FALSE;
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStopByBss(pAd, pMbss);
		break;
	}

	/* MacTableReset(pAd); */
	CMDHandler(pAd);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===\n");
}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	PQUEUE_ENTRY pEntry;
	PNDIS_PACKET pPacket;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(0x%08lx)...\n", (ULONG)pQueue);

	while (pQueue->Head) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%u...\n", pQueue->Number);
		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

/*
*
*/
static VOID avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
	INT8 i;
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_reset(ad);
#endif
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		ad->mcli_ctl[i].pkt_avg_len = 0;
		ad->mcli_ctl[i].sta_nums = 0;
		ad->mcli_ctl[i].pkt_rx_avg_len = 0;
		ad->mcli_ctl[i].rx_sta_nums = 0;
	}
}

/*
*
*/
static VOID avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry, UINT8 band_idx)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0;
	struct multi_cli_ctl *mctrl = NULL;
#ifdef RX_COUNT_DETECT
	UINT32 avg_rx_pkt_len = 0;
#endif
	mctrl = &ad->mcli_ctl[band_idx];

#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_calculate(entry);
#endif
	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes / entry->avg_tx_pkts);
#ifdef RX_COUNT_DETECT
	if (entry->avg_rx_pkts > 0)
		avg_rx_pkt_len = (UINT32)(entry->AvgRxBytes / entry->avg_rx_pkts);
#endif
	/*moving average for pkt avg length*/
	if ((avg_pkt_len <= VERIWAVE_INVALID_PKT_LEN_HIGH) &&
			(avg_pkt_len >= VERIWAVE_INVALID_PKT_LEN_LOW)) {
	mctrl->pkt_avg_len =
		((mctrl->pkt_avg_len * mctrl->sta_nums) + avg_pkt_len) / (mctrl->sta_nums + 1);
	mctrl->sta_nums++;
		mctrl->tot_tx_pkts += entry->avg_tx_pkts;
	}
	entry->avg_tx_pkt_len = avg_pkt_len;

#ifdef RX_COUNT_DETECT
	if (avg_rx_pkt_len > 0) {
	mctrl->pkt_rx_avg_len =
		((mctrl->pkt_rx_avg_len * mctrl->rx_sta_nums) + avg_rx_pkt_len) / (mctrl->rx_sta_nums + 1);
	mctrl->rx_sta_nums++;
		mctrl->tot_rx_pkts += entry->avg_rx_pkts;
	}
	entry->avg_rx_pkt_len = avg_rx_pkt_len;
#endif
}

#define MAX_VERIWAVE_DELTA_RSSI  (10)
#define FAR_CLIENT_RSSI	(-70)
#define FAR_CLIENT_DELTA_RSSI	(10)
static VOID CalFarClientNum(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, UINT_8 band_idx)
{
	UINT8 idx;
	CHAR avg_rssi[4];
	BOOLEAN far_client_found = FALSE, large_rssi_gap_found = FALSE;

	for (idx = 0; idx < 4; idx++)
		avg_rssi[idx] = ad->ApCfg.RssiSample.LastRssi[idx] - ad->BbpRssiToDbmDelta;

	for (idx = 0; idx < 4; idx++) {
		if (entry->RssiSample.AvgRssi[idx] != -127) {
			if (((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= MAX_VERIWAVE_DELTA_RSSI) &&
				(large_rssi_gap_found == FALSE)) {
				ad->mcli_ctl[band_idx].large_rssi_gap_num++;
				large_rssi_gap_found = TRUE;
			}

			if ((entry->RssiSample.AvgRssi[idx] < FAR_CLIENT_RSSI) && (far_client_found == FALSE))
			if ((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= FAR_CLIENT_DELTA_RSSI) {
					ad->nearfar_far_client_num[band_idx]++;
					far_client_found = TRUE;
			}
		}
	}
}

#ifdef DELAY_TCP_ACK_V2
static VOID dynamic_peak_tp_txop_aifsn_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT	BandIdx = 0;

	if (pAd->CommonCfg.bEnableTxopPeakTpEn == FALSE)
		return;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		UCHAR	wmm_idx = 0;
		UCHAR 	PeakTpBeAifsn = 0;
		UINT32	PeakTpAvgRxPktLen = pAd->CommonCfg.PeakTpAvgRxPktLen;
		UCHAR	default_be_aifsn = 0;

		if ((pAd->txop_ctl[BandIdx].cur_wdev == NULL) || (pAd->peak_tp_ctl[BandIdx].main_entry == NULL))
			continue;

		/* When hit multi-client case, don't adjust */
		if (pAd->txop_ctl[BandIdx].multi_client_nums >= pAd->multi_cli_nums_eap_th
#ifdef RX_COUNT_DETECT
			|| pAd->txop_ctl[BandIdx].multi_rx_client_nums >= pAd->multi_cli_nums_eap_th
#endif
			)
			continue;

		default_be_aifsn = pAd->CommonCfg.APEdcaParm[pAd->txop_ctl[BandIdx].cur_wdev->EdcaIdx].Aifsn[0];/*default value 0x3*/
		wmm_idx = HcGetWmmIdx(pAd, pAd->txop_ctl[BandIdx].cur_wdev);

		if (pAd->peak_tp_ctl[BandIdx].main_entry->avg_rx_pkt_len >= PeakTpAvgRxPktLen) {
			if ((pAd->peak_tp_ctl[BandIdx].main_rx_tp >= pAd->CommonCfg.PeakTpRxLowerBoundTput) &&
			(pAd->peak_tp_ctl[BandIdx].main_rx_tp < pAd->CommonCfg.PeakTpRxHigherBoundTput)) {
				PeakTpBeAifsn = 2;
			} else {
				PeakTpBeAifsn = pAd->CommonCfg.PeakTpBeAifsn; /*0x5*/
			}
			if (pAd->txop_ctl[BandIdx].peak_tcp_running_aifsn_setting != PeakTpBeAifsn) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"[%d]enable peak tp TXOP_0 & set be aifsn=%d\n", __LINE__, PeakTpBeAifsn);
					enable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev, AC_BE, PRIO_PEAK_TP, TXOP_0);
					HW_SET_PART_WMM_PARAM(pAd, pAd->txop_ctl[BandIdx].cur_wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_AIFSN, PeakTpBeAifsn);
					pAd->txop_ctl[BandIdx].peak_tcp_running_aifsn_setting = PeakTpBeAifsn;
				}
		} else {
			PeakTpBeAifsn = default_be_aifsn;
			if (pAd->txop_ctl[BandIdx].peak_tcp_running_aifsn_setting != PeakTpBeAifsn) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[%d]free peak tp TXOP & set be aifsn to default value=%d\n", __LINE__, PeakTpBeAifsn);
				disable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev, AC_BE, PRIO_PEAK_TP, TXOP_0);
				HW_SET_PART_WMM_PARAM(pAd, pAd->txop_ctl[BandIdx].cur_wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_AIFSN, PeakTpBeAifsn);
				pAd->txop_ctl[BandIdx].peak_tcp_running_aifsn_setting = PeakTpBeAifsn;
			}
		}
	}  /* for loop BandIdx */
}
#endif /* DELAY_TCP_ACK_V2 */

static VOID dynamic_txop_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT     BandIdx = 0;
	BOOLEAN	eap_mcli_check = FALSE, mcli_txop = FALSE;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
#ifdef VOW_SUPPORT
		/* always turn on TXOP if SPL is enabled */
		if ((pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) &&
		    (pAd->vow_misc_cfg.spl_sta_count > 0)) {
			if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == TRUE) {
				pAd->txop_ctl[BandIdx].multi_cli_txop_running = FALSE;
				disable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
			continue;
		}
#endif
		eap_mcli_check = mcli_txop = FALSE;

		if (pAd->txop_ctl[BandIdx].multi_client_nums >= pAd->multi_cli_nums_eap_th
#ifdef RX_COUNT_DETECT
			|| pAd->txop_ctl[BandIdx].multi_rx_client_nums >= pAd->multi_cli_nums_eap_th
#endif
			)
			eap_mcli_check = TRUE;

		/* need further consider OBSS threshold */
		if ((pAd->OneSecMibBucket.Enabled[BandIdx] == TRUE) &&
			((pAd->OneSecMibBucket.OBSSAirtime[BandIdx]*2) > 1000000))
			mcli_txop = TRUE;

		/* consider ixia mode */
		if (pAd->ixia_mode_ctl.isIxiaOn || pAd->mcli_ctl[BandIdx].large_rssi_gap_num == 0)
			mcli_txop = TRUE;

		if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_PER_BAND)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Band%d, McliCheck[%d-%d/%d/%d], McliTxop(%d-Mib[%d/%d]-Ixia[%d/%d])\n",
				BandIdx, eap_mcli_check, pAd->multi_cli_nums_eap_th,
				pAd->txop_ctl[BandIdx].multi_client_nums, pAd->txop_ctl[BandIdx].multi_rx_client_nums,
				mcli_txop, pAd->OneSecMibBucket.Enabled[BandIdx], pAd->OneSecMibBucket.OBSSAirtime[BandIdx]*2,
				pAd->ixia_mode_ctl.isIxiaOn, pAd->mcli_ctl[BandIdx].large_rssi_gap_num);

		if (eap_mcli_check) {
		/*	For Real sta test, We sense different eap_mutil_client case:
		*	if eap mutilclient >= pAd->multi_cli_nums_eap_th(32)
		*		for UL:enable PRIO_MULTI_CLIENT(TXOP_0)
		*		for DL:
		*			if OBSS/IXIA
		*				disable PRIO_MULTI_CLIENT(TXOP_0)
		*			else
		*				enable PRIO_MULTI_CLIENT(TXOP_0)
		*	else
		*		disable PRIO_MULTI_CLIENT(TXOP_0)
		*no matter TCP/UDP test, adjust pAd->multi_cli_nums_eap_th(32) according scenarios
		*/
			if (pAd->peak_tp_ctl[BandIdx].main_traffc_mode == TRAFFIC_UL_MODE) {
				if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == FALSE) {
					pAd->txop_ctl[BandIdx].multi_cli_txop_running = TRUE;
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[%d]UL enable_tx_burst TXOP_0\n", __LINE__);
					enable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev,
							AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
				}
			} else if (pAd->peak_tp_ctl[BandIdx].main_traffc_mode == TRAFFIC_DL_MODE) {
				if (mcli_txop) {
					if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == TRUE) {
						pAd->txop_ctl[BandIdx].multi_cli_txop_running = FALSE;
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"[%d]DL disable_tx_burst TXOP_0\n", __LINE__);
						disable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev,
								AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
					}
				} else {
					if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == FALSE) {
						pAd->txop_ctl[BandIdx].multi_cli_txop_running = TRUE;
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"[%d]DL enable_tx_burst TXOP_0\n", __LINE__);
						enable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev,
								AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
					}
				}
			}
		} else {
		/* Non eap_mutil_client case, disable PRIO_MULTI_CLIENT(TXOP_0), enable other TXOP */
			if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == TRUE) {
				pAd->txop_ctl[BandIdx].multi_cli_txop_running = FALSE;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[%d]Non-eap_mcli disable_tx_burst TXOP_0\n", __LINE__);
				disable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev,
						 AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
		}
	} /* for loop BandIdx */
}

/*
*
*/
static VOID dynamic_amsdu_protect_adjust(struct _RTMP_ADAPTER *ad)
{
	UCHAR amsdu_cnt = 4, band_idx = 0;
	ULONG per = 0;
	ULONG tx_cnt;
	ULONG tx_fail_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	UINT16 multi_client_num_th = 0, veriwave_tp_amsdu_dis_th = 0;
	BOOLEAN mc_flg = FALSE, spl_flg = FALSE, skip_flg = FALSE;

	/*concurrent case, not adjust*/
	if ((ad->txop_ctl[DBDC_BAND0].multi_client_nums > 0) ||
		(ad->txop_ctl[DBDC_BAND0].multi_rx_client_nums > 0)) {
#if DBDC_BAND_NUM == 2
		if ((ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0) ||
			(ad->txop_ctl[DBDC_BAND1].multi_rx_client_nums > 0))
			return;
#endif
		band_idx = DBDC_BAND0;
	}
#if DBDC_BAND_NUM == 2
	 else if ((ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0) ||
		(ad->txop_ctl[DBDC_BAND1].multi_rx_client_nums > 0))
		band_idx = DBDC_BAND1;
#endif
	else
		return;

	wlan_ct = &ad->WlanCounters[band_idx];
	wdev = ad->txop_ctl[band_idx].cur_wdev;

	if (WMODE_CAP_2G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_2G_TP_AMSDU_DIS_TH;
	} else if (WMODE_CAP_5G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_TP_AMSDU_DIS_TH;
	} else {
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[%d]\n", __LINE__);
		return;
	}

	/*adjust amsdu*/
	if ((ad->txop_ctl[band_idx].multi_client_nums >= multi_client_num_th)  ||
		(ad->txop_ctl[band_idx].multi_rx_client_nums >= multi_client_num_th)) {
		mc_flg = TRUE;
	}

	if ((ad->mcli_ctl[band_idx].amsdu_cnt != amsdu_cnt) && (!IS_ASIC_CAP(ad, fASIC_CAP_HW_TX_AMSDU))) {
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "change amsdu %d to %d\n",
			ad->mcli_ctl[band_idx].amsdu_cnt, amsdu_cnt);
		ad->mcli_ctl[band_idx].amsdu_cnt = amsdu_cnt;
		amsdu_cnt = 4;
		if (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))
			MtCmdCr4Set(ad, CR4_SET_ID_CONFIG_STA_AMSDU_MAX_NUM, 255, amsdu_cnt);
		else
			ad->amsdu_max_num = amsdu_cnt;
	}

#ifdef VOW_SUPPORT
	/* enable RTS protection if SPL is enabled */
	if ((ad->vow_gen.VOW_GEN == VOW_GEN_TALOS) &&
	    (ad->vow_misc_cfg.spl_sta_count > 0))
		spl_flg = TRUE;
#endif

	/* enable RTS protection while having TCP clients */
	if (ad->txop_ctl[band_idx].multi_tcp_nums >= MULTI_TCP_NUMS_TH)
		skip_flg = TRUE;

	if (ad->mcli_ctl[band_idx].large_rssi_gap_num > 0)
		skip_flg = TRUE;

#ifdef KERNEL_RPS_ADJUST
	if (ad->ixia_mode_ctl.kernel_rps_en) {
		if (ad->ixia_mode_ctl.mode_entered == FALSE)
			skip_flg = TRUE;
	}
#endif
	/*only change protection scenario when sta more than multi_client_num_th*/
	if (!mc_flg || spl_flg || skip_flg) {
		if (ad->mcli_ctl[band_idx].tx_cnt_from_red == FALSE) {
			ad->mcli_ctl[band_idx].last_tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
			ad->mcli_ctl[band_idx].last_tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart;
		}
		if (ad->mcli_ctl[band_idx].c2s_only == TRUE) {
			asic_rts_on_off(wdev, TRUE);
			ad->mcli_ctl[band_idx].c2s_only = FALSE;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "enable RTS\n");
		}
		goto ADJ_AGG;
	}
#ifdef RED_SUPPORT
	if ((ad->mcli_ctl[band_idx].tx_cnt_from_red == TRUE) && (ad->red_en)) {
		tx_cnt = ad->mcli_ctl[band_idx].tot_tx_cnt;
		tx_fail_cnt = ad->mcli_ctl[band_idx].tot_tx_fail_cnt;

	} else {
#else
	{
#endif

		tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart - ad->mcli_ctl[band_idx].last_tx_cnt;
		tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart - ad->mcli_ctl[band_idx].last_tx_fail_cnt;

		ad->mcli_ctl[band_idx].last_tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
		ad->mcli_ctl[band_idx].last_tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart;
	}
	if (tx_cnt > 0) {
		per = (100 * tx_fail_cnt) / (tx_fail_cnt + tx_cnt);
		if ((ad->mcli_ctl[band_idx].c2s_only == FALSE) && (per < VERIWAVE_PER_RTS_DIS_TH_LOW_MARK)) {
			asic_rts_on_off(wdev, FALSE);
			ad->mcli_ctl[band_idx].c2s_only = TRUE;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "disable RTS, per=%lu\n", per);
		} else if ((ad->mcli_ctl[band_idx].c2s_only == TRUE) && (per >= VERIWAVE_PER_RTS_DIS_TH_HIGH_MARK)) {
			asic_rts_on_off(wdev, TRUE);
			ad->mcli_ctl[band_idx].c2s_only = FALSE;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "enable RTS, per=%lu\n", per);
		}
	}
ADJ_AGG:
#ifdef KERNEL_RPS_ADJUST
#ifdef CONFIG_SOC_MT7621
	{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if ((pChipCap->hw_version == 0) && (ad->ixia_mode_ctl.mode_entered)) {
		UINT32 agglimit = 64;
		BOOLEAN uplink = FALSE;

		if ((ad->txop_ctl[band_idx].multi_client_nums == 0) &&
			(ad->txop_ctl[band_idx].multi_rx_client_nums > 0) &&
			(ad->mcli_ctl[band_idx].pkt_avg_len < 64) &&
			(ad->mcli_ctl[band_idx].pkt_rx_avg_len > 64))
			uplink = TRUE;

		if ((band_idx == DBDC_BAND0) && uplink) {
			if (ad->txop_ctl[band_idx].multi_rx_client_nums == 1)
				agglimit = 0x05;

			if (ad->txop_ctl[band_idx].multi_rx_client_nums > 1)
				agglimit = 0x05;

			if (ad->txop_ctl[band_idx].multi_rx_client_nums > 14) {
				if (ad->ixia_mode_ctl.sta_nums >= 20)
					agglimit = 0x06;
				if (ad->ixia_mode_ctl.sta_nums >= 32)
					agglimit = 0x07;
			}

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len < ((88+128)>>1))
				agglimit -= 2;
			else if (ad->mcli_ctl[band_idx].pkt_rx_avg_len <= ((128+256)>>1))
				agglimit -= 1;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((128+256)>>1))
				agglimit += 3;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((256+512)>>1))
				agglimit += 3;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((1024+512)>>1))
				agglimit = 0xf;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((1518+1024)>>1))
				agglimit = 0x0;
		}
		if ((band_idx == DBDC_BAND1) && uplink) {
			if (ad->txop_ctl[band_idx].multi_rx_client_nums == 1)
				agglimit = 0x05;

			if (ad->txop_ctl[band_idx].multi_rx_client_nums > 1)
				agglimit = 0x06;

			if (ad->txop_ctl[band_idx].multi_rx_client_nums > 14) {
				if (ad->ixia_mode_ctl.sta_nums >= 20)
					agglimit = 0x07;
				if (ad->ixia_mode_ctl.sta_nums >= 32)
					agglimit = 0x08;
			}

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len < ((88+128)>>1))
				agglimit -= 2;
			else if (ad->mcli_ctl[band_idx].pkt_rx_avg_len <= ((128+256)>>1))
				agglimit -= 1;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((128+256)>>1))
				agglimit += 3;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((256+512)>>1))
				agglimit += 4;


			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((1024+512)>>1))
				agglimit = 0xf;

			if (ad->mcli_ctl[band_idx].pkt_rx_avg_len > ((1518+1024)>>1))
				agglimit = 0x0;

		}

		if (ad->mcli_ctl[band_idx].force_agglimit > 0)
			agglimit = ad->mcli_ctl[band_idx].force_agglimit;

		if ((ad->mcli_ctl[band_idx].cur_agglimit != agglimit) && uplink) {
			asic_set_agglimit(ad, band_idx, WMM_AC_BE, wdev, agglimit);
			ad->mcli_ctl[band_idx].cur_agglimit = agglimit;
		}
	}
	}
#endif
#endif
	return;

}
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
static VOID dynamic_near_far_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT32 band_idx, wcid;
	MAC_TABLE_ENTRY *pEntry;
	HTTRANSMIT_SETTING last_tx_rate_set;
	ULONG last_tx_rate;
	NEAR_FAR_CTRL_T *near_far_ctrl = &pAd->vow_misc_cfg.near_far_ctrl;
	BOOLEAN fast_sta_flg = FALSE, slow_sta_flg = FALSE;
	struct txop_ctl *txop_ctl_p = NULL;
	struct wifi_dev *wdev = NULL;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		txop_ctl_p = &pAd->txop_ctl[band_idx];

		if (txop_ctl_p->cur_wdev == NULL)
			continue;

		if (!WMODE_CAP_5G(txop_ctl_p->cur_wdev->PhyMode))
			continue;

		wdev = txop_ctl_p->cur_wdev;
	}

	if (wdev == NULL)
		return;

	if (!near_far_ctrl->adjust_en) {
		if (txop_ctl_p->near_far_txop_running == TRUE) {
			disable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = FALSE;
		}
		return;
	}

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (pEntry->wdev != wdev)
			continue;

		if (pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH) {
			last_tx_rate_set.word = (USHORT)pEntry->LastTxRate;
			getRate(last_tx_rate_set, &last_tx_rate);
			if (last_tx_rate >= near_far_ctrl->fast_phy_th)
				fast_sta_flg = TRUE;
			else if (last_tx_rate <= near_far_ctrl->slow_phy_th)
				slow_sta_flg = TRUE;
		}
	}

	if (fast_sta_flg & slow_sta_flg) {
		if (txop_ctl_p->near_far_txop_running == FALSE) {
			enable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"PRIO_NEAR_FAR: start\n");
		}
	} else {
		if (txop_ctl_p->near_far_txop_running == TRUE) {
			disable_tx_burst(pAd, wdev, AC_BE, PRIO_NEAR_FAR, TXOP_0);
			txop_ctl_p->near_far_txop_running = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"PRIO_NEAR_FAR: end\n");
		}
	}
}

static VOID dynamic_airtime_fairness_adjust(struct _RTMP_ADAPTER *ad)
{
	BOOLEAN shrink_flag = FALSE;
	BOOLEAN mc_flg = FALSE, fband_2g = FALSE;
	UCHAR band_idx = 0;
	UINT16 multi_client_num_th = 0, veriwave_tp_amsdu_dis_th = 0;
	struct wifi_dev *wdev = NULL;

	if (!ad->vow_cfg.en_airtime_fairness)
		return;

	/*concurrent case, not adjust*/
	if (ad->txop_ctl[DBDC_BAND0].multi_client_nums > 0) {
#if DBDC_BAND_NUM == 2
		if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
			return;
#endif
		band_idx = DBDC_BAND0;
	}
#if DBDC_BAND_NUM == 2
	 else if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
		band_idx = DBDC_BAND1;
#endif
	else
		return;

	wdev = ad->txop_ctl[band_idx].cur_wdev;

	if (WMODE_CAP_2G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_2G_TP_AMSDU_DIS_TH;
		fband_2g = TRUE;
	} else if (WMODE_CAP_5G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_TP_AMSDU_DIS_TH;
	} else {
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[%d]\n", __LINE__);
		return;
	}

	/*adjust amsdu*/
	if (ad->txop_ctl[band_idx].multi_client_nums >= multi_client_num_th) {
		if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
			shrink_flag = TRUE;

	} else {
		if (ad->txop_ctl[band_idx].multi_client_nums > 1) {
			if (ad->vow_mcli_ctl.pkt_avg_len > veriwave_tp_amsdu_dis_th)
				shrink_flag = TRUE;
			else if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
				shrink_flag = TRUE;
		}
	}

	if ((ad->vow_gen.VOW_GEN >= VOW_GEN_FALCON) || (ad->mcli_ctl[band_idx].large_rssi_gap_num > 0))
		shrink_flag = FALSE;

	if (ad->txop_ctl[band_idx].multi_client_nums >= multi_client_num_th)
		mc_flg = TRUE;

	if (ad->vow_sta_frr_flag != shrink_flag) {
		/* adj airtime quantum only when WATF is not enabled */
		ad->vow_sta_frr_flag = shrink_flag;
		if (ad->vow_cfg.en_airtime_fairness && ad->vow_sta_frr_quantum && !vow_watf_is_enabled(ad)) {
			if ((shrink_flag == TRUE) && (ad->nearfar_far_client_num[band_idx] <= 1) && mc_flg)
				RTMP_SET_STA_DWRR_QUANTUM(ad, FALSE, ad->vow_sta_frr_quantum);/* fast round robin */
			else
				RTMP_SET_STA_DWRR_QUANTUM(ad, TRUE, 0);/* restore */
		}

	}
	return;
}

/* Used for mcli balance/near-far case test, note that it may reduce the total T-PUT */
static VOID dynamic_mcli_param_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT	BandIdx = 0;
	BOOLEAN	mcli_tcp_check = FALSE;
	UINT8	wmm_idx, default_vow_sch_type, default_vow_sch_policy;
	UINT32	default_cwmax, default_cwmin, mcli_cwmax, mcli_cwmin;
	struct wifi_dev *wdev = NULL;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		mcli_tcp_check = FALSE;

		/* consider ixia mode */
		if (pAd->ixia_mode_ctl.isIxiaOn)
			continue;

		if ((pAd->txop_ctl[BandIdx].multi_client_nums >= pAd->multi_cli_nums_eap_th
#ifdef RX_COUNT_DETECT
			|| pAd->txop_ctl[BandIdx].multi_rx_client_nums >= pAd->multi_cli_nums_eap_th
#endif
			) && pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num[BandIdx] >= pAd->multi_cli_nums_eap_th)
			mcli_tcp_check = TRUE;

		if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_PER_BAND)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Band%d, mcli_tcp_check/mcli_tcp_num: [%d/%d]\n",
				BandIdx, mcli_tcp_check, pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num[BandIdx]);
		/* reset mcli_tcp_num for next period */
		pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num[BandIdx] = 0;

		if (pAd->txop_ctl[BandIdx].cur_wdev != NULL) {
			wdev = pAd->txop_ctl[BandIdx].cur_wdev;
			wmm_idx = HcGetWmmIdx(pAd, wdev);

			/* save default BE cwmax/cwmin value, set multi user BE cwmax/cwmin value */
			default_cwmax = pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[0];
			default_cwmin = pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[0];
			/* save default vow schedule type/policy value,
			* set multi-client vow schedule type/policy value.
			*/
			default_vow_sch_type = pAd->vow_cfg.mcli_sch_cfg.sch_type;
			default_vow_sch_policy = pAd->vow_cfg.mcli_sch_cfg.sch_policy;

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"BandIdx = %d, wmm_idx = %d, cwmax = %d, cwmin = %d !!\n",
				BandIdx, wmm_idx, default_cwmax, default_cwmin);
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"BandIdx = %d, cur_wdev is null!!\n", BandIdx);
			continue;
		}

		if (mcli_tcp_check) {/* Hit MultiClient TCP Case */
			if (pAd->peak_tp_ctl[BandIdx].main_traffc_mode == TRAFFIC_UL_MODE) {
				if (pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] == FALSE) {
					pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] = TRUE;
					mcli_cwmax = pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE][BandIdx];
					mcli_cwmin = pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE][BandIdx];
					HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMAX, mcli_cwmax);
					HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMIN, mcli_cwmin);
					if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en) {
						pAd->vow_cfg.mcli_sch_cfg.apply_cnt++;
						HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE, VOW_SCH_FOLLOW_HW, VOW_SCH_POL_WRR);
					}
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mcli scene, UL mode, Band%d Enter schedule condition !!\n", BandIdx);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mcli scene, UL mode, cwmin:cwmax=%d:%d !!\n", mcli_cwmin, mcli_cwmax);
				}
			} else if (pAd->peak_tp_ctl[BandIdx].main_traffc_mode == TRAFFIC_DL_MODE) {
				if (pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] == FALSE) {
					pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] = TRUE;
					mcli_cwmax = pAd->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE][BandIdx];
					mcli_cwmin = pAd->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE][BandIdx];
					HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMAX, mcli_cwmax);
					HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMIN, mcli_cwmin);
					if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en) {
						pAd->vow_cfg.mcli_sch_cfg.apply_cnt++;
						HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE, VOW_SCH_FOLLOW_HW, VOW_SCH_POL_WRR);
					}
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mcli scene, DL mode, Band%d Enter schedule condition !!\n", BandIdx);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mcli scene, DL mode, wrren:cwmin:cwmax=%d:%d:%d !!\n",
						pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en, mcli_cwmin, mcli_cwmax);
				}
			}
		} else {/* Exit MultiClient TCP Case */
			if (pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] == TRUE) {
				pAd->vow_cfg.mcli_sch_cfg.schedule_cond_running[BandIdx] = FALSE;
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMAX, default_cwmax);
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMIN, default_cwmin);
				if (pAd->vow_cfg.mcli_sch_cfg.dl_wrr_en) {
					pAd->vow_cfg.mcli_sch_cfg.apply_cnt--;
					/* Use apply_cnt to record DBDC multiclient case;
					* when all band exit mcli_sch condition, set default para.
					*/
					if (pAd->vow_cfg.mcli_sch_cfg.apply_cnt == 0)
						HW_SET_VOW_SCHEDULE_CTRL(pAd, TRUE, default_vow_sch_type, default_vow_sch_policy);
				}
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"no mcli scene, Band%d Enter default condition !!\n", BandIdx);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"no mcli scene, mode:cwmin:cwmax=%d:%d:%d !!\n",
					pAd->peak_tp_ctl[BandIdx].main_traffc_mode, default_cwmin, default_cwmax);
			}
		}
	}
}
#endif /* defined (VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT) */

VOID dynamic_adjust_ba_winsize(struct _RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry)
{
	UINT32 ori_idx, per, winsize, WinSizeThr;
	BA_ORI_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = &ad->tr_ctl.ba_ctl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	ori_idx = entry->BAOriWcidArray[0];//TID0
	pBAEntry = &ba_ctl->BAOriEntry[ori_idx];

	if (ori_idx == 0 || !pBAEntry) {
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"STA(%d) doesn't establish BA session, return!\n", entry->wcid);
		return;
	}

	if (chip_dbg->set_ba_winsize) {
		if (pBAEntry->BAWinSize <= BA_WIN_SZ_64)
			WinSizeThr = 0x7;
		else
			WinSizeThr = 0xE;

		if (entry->winsize_limit == 0xF)
			entry->winsize_limit = WinSizeThr;

		winsize = entry->winsize_limit;//init value
		per = entry->tx_per;

		if (per > ad->per_dn_th) {//you can adjust dn/up-kp for real test.
			if (entry->winsize_limit > 0x7)
				winsize = entry->winsize_limit >> 1;
			else if (entry->winsize_limit > ad->winsize_kp_idx)
				winsize = entry->winsize_limit - 1;
		} else if (per < ad->per_up_th) {
			if (entry->winsize_limit < WinSizeThr)
				winsize = entry->winsize_limit + 1;
		}

		if (winsize == entry->winsize_limit)
			entry->agg_err_flag = FALSE;

		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"STA(%d), per/dnth/upth(%d/%d/%d), winsize/limit/kpth/Thr(%d/%d/%d/%d)\n",
			entry->wcid, per, ad->per_dn_th, ad->per_up_th,
			winsize, entry->winsize_limit, ad->winsize_kp_idx, WinSizeThr);

		if (winsize < 0xF && entry->winsize_limit != winsize) {
			/* Only set BA_WIN_SIZE(TID0), should consider other tid? */
			chip_dbg->set_ba_winsize(ad, entry->wcid, 0, winsize);
			entry->winsize_limit = winsize;
		}
	}

}

VOID dynamic_agg_per_sta_adjust(struct _RTMP_ADAPTER *ad)
{
	UINT32 bandidx, tmpBand, wcid;
	BOOLEAN mcli_flag[DBDC_BAND_NUM] = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;

	for (bandidx = 0; bandidx < DBDC_BAND_NUM; bandidx++) {
		mcli_flag[bandidx] = FALSE;

		if (ad->txop_ctl[bandidx].multi_client_nums >= ad->multi_cli_nums_eap_th
#ifdef RX_COUNT_DETECT
			|| ad->txop_ctl[bandidx].multi_rx_client_nums >= ad->multi_cli_nums_eap_th
#endif
		)
		mcli_flag[bandidx] = TRUE;
	}

	for (wcid = 1; VALID_UCAST_ENTRY_WCID(ad, wcid); wcid++) {
		pEntry = &ad->MacTab.Content[wcid];

		if (pEntry && IS_ENTRY_CLIENT(pEntry)
			&& pEntry->Sst == SST_ASSOC) {

			tmpBand = HcGetBandByWdev(pEntry->wdev);
			/* firstly we should check per band multiclient condition */
			if (mcli_flag[tmpBand] == FALSE)
				continue;

			/* then check agg_err_flag or aggManualEn */
			if (pEntry->agg_err_flag || ad->aggManualEn) {
				MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"STA(%d) hit agg err, need adjust agg num!\n", pEntry->wcid);
				/*
				* we have calculated pEntry->tx_per from event_get_tx_statistic_handle;
				* according sta tx_per set wtbl(BA_WIN_SIZE), limit per sta agg num.
				*/
				dynamic_adjust_ba_winsize(ad, pEntry);
			}
		}
	}

}

static inline BOOLEAN is_tcp_pkt(NDIS_PACKET *pkt)
{
	PUCHAR pSrcBuf;
	USHORT TypeLen;

	if (pkt) {
		pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
		TypeLen = OS_NTOHS(*((UINT16 *)(pSrcBuf + 12)));
	} else {
		return FALSE;
	}

	if ((TypeLen == 0x0800) && /* Type: IP (0x0800) */
	    (pSrcBuf[23] == 0x06)) /* Protocol: TCP (0x06) */
		return TRUE;

	return FALSE;
}

static inline void check_sta_tcp_flow(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry,
				      UINT band_idx)
{
	int q_idx;
	PQUEUE_ENTRY q_entry;
	NDIS_PACKET *ptk = NULL;
	BOOLEAN	is_tcp = FALSE;

	if (pAd->txop_ctl[band_idx].multi_tcp_nums >= MULTI_TCP_NUMS_TH)
		return;

	for (q_idx = 0 ; q_idx < WMM_QUE_NUM; q_idx++) {
		if (tr_entry->tx_queue[q_idx].Number == 0)
			continue;

		RTMP_SPIN_LOCK(&tr_entry->txq_lock[q_idx]);
		if (tr_entry->tx_queue[q_idx].Head) {
			q_entry = tr_entry->tx_queue[q_idx].Head;
			ptk = QUEUE_ENTRY_TO_PACKET(q_entry);
			is_tcp = is_tcp_pkt(ptk);
		}
		RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[q_idx]);

		if (is_tcp) {
			pAd->txop_ctl[band_idx].multi_tcp_nums++;
			break;
		}
	}
}

#ifdef WHNAT_SUPPORT
static VOID tx_offload_counter_update(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef TR181_SUPPORT
		UCHAR bandIdx = 0;
#endif

#ifdef CONFIG_AP_SUPPORT
		if (IS_ENTRY_CLIENT(entry) && entry->pMbss) {
			entry->pMbss->TxCount += entry->one_sec_tx_pkts;
			entry->pMbss->TransmittedByteCount += entry->OneSecTxBytes;
#ifdef MAP_R2
			if (IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad))
				entry->pMbss->ucBytesTx += entry->OneSecTxBytes;
			if (IS_MAP_ENABLE(ad))
				entry->TxBytesMAP += entry->OneSecTxBytes;
#endif

#ifdef TR181_SUPPORT
			/* per STA */
			entry->TxPackets.QuadPart += entry->one_sec_tx_pkts;
			entry->TxBytes += entry->OneSecTxBytes;
			/* per Band */
			bandIdx = HcGetBandByWdev(entry->wdev);
			ad->WlanCounters[bandIdx].TxTotByteCount.QuadPart += entry->OneSecTxBytes;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "WCID=%d TxPackets=%ld TxBytes=%ld, TxTotByteCount=%ld,one_sec_tx_pkts=%ld OneSecTxBytes=%ld,bandIdx=%d\n",
				 entry->wcid, (ULONG)entry->TxPackets.QuadPart,
				 (ULONG)ad->WlanCounters[bandIdx].TxTotByteCount.QuadPart,
				 (ULONG)entry->TxBytes,
				 entry->one_sec_tx_pkts, entry->OneSecTxBytes, bandIdx);
#endif
		}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef APCLI_SUPPORT
		if ((IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry)) && entry->wdev) {
			struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(ad, entry->wdev);

			apcli->StaStatistic.TxCount += entry->one_sec_tx_pkts;
			apcli->StaStatistic.TransmittedByteCount += entry->OneSecTxBytes;
		}
#endif /*APCLI_SUPPORT*/

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry)) {
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount.QuadPart +=  entry->one_sec_tx_pkts;
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += entry->OneSecTxBytes;
	}
#endif /* WDS_SUPPORT */
}
#endif /*WHNAT_SUPPORT*/


/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/

VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int wcid, startWcid, i;
#ifdef DOT11_N_SUPPORT
	BOOLEAN	bRdgActive = FALSE;
	BOOLEAN bRalinkBurstMode;
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	MAC_TABLE *pMacTable;
	CHAR avgRssi;
	BSS_STRUCT *pMbss;
#ifdef MT_MAC
	BOOLEAN bPreAnyStationInPsm = FALSE;
#endif /* MT_MAC */
#ifdef PS_STA_FLUSH_SUPPORT
	UINT16 PsStaNum = 0;
#endif
	UINT     BandIdx = 0;
#ifdef SMART_CARRIER_SENSE_SUPPORT
	CHAR    tmpRssidata = 0;
	CHAR    tmpRssiba = 0;
	CHAR    tmpRssimin = 0;

#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef APCLI_SUPPORT
	ULONG	apcli_avg_tx = 0;
	ULONG	apcli_avg_rx = 0;
	struct wifi_dev *apcli_wdev = NULL;
	MAC_TABLE_ENTRY *pApcliEntry = NULL;
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
#endif /* APCLI_SUPPORT */
	struct wifi_dev *sta_wdev = NULL;
	struct wifi_dev *txop_wdev = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR    sta_hit_2g_infra_case_number = 0;
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	BOOLEAN IgmpQuerySendTickChanged = FALSE;
	BOOLEAN MldQuerySendTickChanged = FALSE;
#endif
#ifdef IGMP_SNOOPING_NON_OFFLOAD
	struct _PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct token_tx_pkt_queue *que = NULL;
	UCHAR band_id = 0;
#endif
	struct _RTMP_CHIP_CAP *cap;
	struct peak_tp_ctl *peak_tp_ctl = NULL;
	BOOLEAN is_no_rx_cnt = FALSE;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	BOOLEAN is_tx_traffic = FALSE, is_rx_traffic = FALSE;
	UINT32 tx_ratio = 0, rx_ratio = 0;
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	UINT16 u2TxTP = 0;
#endif
	UINT32 sta_idle_timeout = 0;
	TX_STAT_STRUC *p_temp = NULL;
	TX_STAT_STRUC *p_tx_statics_queue = NULL;
	UCHAR tx_statics_qcnt = 0;
	/*alloc mem to agg tx statics cmd for cr4*/
#ifdef WHNAT_SUPPORT
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
	PCR4_QUERY_STRUC cr4_query_list;
	UINT16 cr4_query_len = sizeof(*cr4_query_list) + sizeof(*cr4_query_list->list) * MAX_CR4_QUERY_NUM;

	os_alloc_mem(pAd, (UCHAR **)&cr4_query_list, cr4_query_len);
	if (cr4_query_list == NULL)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s:alloc mem fail!!!\n", __func__);
	else
		os_zero_mem(cr4_query_list, cr4_query_len);
#endif
#endif

	/*alloc mem to agg tx statics cmd*/
	os_alloc_mem(pAd, (UCHAR **)&p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);

	if (!p_tx_statics_queue) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s:alloc mem fail!!!\n", __func__);
	} else
		os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	pMacTable = &pAd->MacTab;
#ifdef MT_MAC
	bPreAnyStationInPsm = pMacTable->fAnyStationInPsm;
#endif /* MT_MAC */
	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink[0] = TRUE;
	pMacTable->fAllStationAsRalink[1] = TRUE;
	pMacTable->fCurrentStaBw40 = FALSE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */
	startWcid = 0;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Skip the Infra Side */
	startWcid = 2;/*TODO: 20190619 review it to check the P2P wcid assignment rule*/
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		/* fix "hobj is not ready" log continuous printf */
		pAd->txop_ctl[BandIdx].cur_wdev = NULL;
	}
#ifdef SMART_CARRIER_SENSE_SUPPORT

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAd->SCSCtrl.SCSMinRssi[BandIdx] =  0; /* (Reset)The minimum RSSI of STA */
		pAd->SCSCtrl.OneSecRxByteCount[BandIdx] = 0;
		pAd->SCSCtrl.OneSecTxByteCount[BandIdx] = 0;
	}

#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAd->DynWmmCtrl.OneSecRxByteCount[BandIdx] = 0;
		pAd->DynWmmCtrl.OneSecTxByteCount[BandIdx] = 0;
	}

#endif /* DYNAMIC_WMM_SUPPORT */

	/* step1. scan all wdevs, clean all wdev non_gf_sta counter */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			wlan_operate_set_non_gf_sta(wdev, 0);
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
			wdev->isStaNotSprtTpc = FALSE;
#endif
#endif
		}
	}

	for (i = 0; i < BAND_NUM_MAX; i++) {
		pMacTable->fAnyStationNonGF[i] = FALSE;
	}

	avg_pkt_len_reset(pAd);
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	tpc_req_entry_reset(pAd);
#endif
#endif
	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[wcid];
		STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[wcid];
		BOOLEAN bDisconnectSta = FALSE;
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_HE_AX
		RA_ENTRY_INFO_T *raentry;
		struct _STA_REC_CTRL_T *strec = &tr_entry->StaRec;
		struct he_mcs_info *mcs;

		raentry = &pEntry->RaEntry;
#endif
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */

		if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
			continue;

#ifdef IGMP_SNOOPING_NON_OFFLOAD
		if (pEntry->wdev) {
			band_id = HcGetBandByWdev(pEntry->wdev);
			que = token_tx_get_queue_by_band(cb, band_id);
		}
#endif

#ifdef HTC_DECRYPT_IOT

		if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
			if (pEntry->HTC_AAD_OM_CountDown > 0) /* count down to start all new pEntry->HTC_ICVErrCnt */
				pEntry->HTC_AAD_OM_CountDown--;
		}

#endif /* HTC_DECRYPT_IOT */
		/* get associated band with wdev*/
		if (pEntry->wdev)
			BandIdx = HcGetBandByWdev(pEntry->wdev);

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_VHT_AC

		if (pEntry->fgRaLimitPhyRate == FALSE) {
			BOOLEAN fgPhyModeCheck = FALSE;

			u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;

			if (pEntry->SupportRateMode & SUPPORT_VHT_MODE) {
				if (pEntry->MaxHTPhyMode.field.BW == BW_160)
					fgPhyModeCheck = TRUE;

				if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_3SS) {
					if (pEntry->SupportVHTMCS4SS)
						fgPhyModeCheck = TRUE;
				} else if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_2SS) {
					if (pEntry->SupportVHTMCS3SS)
						fgPhyModeCheck = TRUE;
				} else
					fgPhyModeCheck = TRUE;
			}

			if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck) {
				MtCmdSetMaxPhyRate(pAd, RACTRL_LIMIT_MAX_PHY_RATE);
				pEntry->fgRaLimitPhyRate = TRUE;
			}
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				for (i = 2; i < DOT11AX_MAX_STREAM; i++) {
					if ((pEntry->cap.rate.he80_rx_nss_mcs[i] == 1) ||
					    (pEntry->cap.rate.he80_rx_nss_mcs[i] == 2) ||
						(pEntry->cap.rate.he160_rx_nss_mcs[i] == 1) ||
						(pEntry->cap.rate.he160_rx_nss_mcs[i] == 2) ||
						(pEntry->cap.rate.he8080_rx_nss_mcs[i] == 1) ||
						(pEntry->cap.rate.he8080_rx_nss_mcs[i] == 2))
						fgPhyModeCheck = TRUE;
				}
			}
			if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck) {
				pEntry->fgRaLimitPhyRate = TRUE;
				raentry->u2SupportVHTMCS1SS = (pEntry->SupportVHTMCS1SS & 255);
				raentry->u2SupportVHTMCS2SS = (pEntry->SupportVHTMCS2SS & 255);
				raentry->u2SupportVHTMCS3SS = (pEntry->SupportVHTMCS3SS & 255);
				raentry->u2SupportVHTMCS4SS = (pEntry->SupportVHTMCS4SS & 255);
				mcs = &strec->he_sta.max_nss_mcs;

				for (i = 0 ; i < DOT11AX_MAX_STREAM; i++) {
					if ((mcs->bw80_mcs[i] == 1) || (mcs->bw80_mcs[i] == 2))
						mcs->bw80_mcs[i] = 0;

					if ((mcs->bw160_mcs[i] == 1) || (mcs->bw160_mcs[i] == 2))
						mcs->bw160_mcs[i] = 0;

					if ((mcs->bw8080_mcs[i] == 1) || (mcs->bw8080_mcs[i] == 2))
						mcs->bw8080_mcs[i] = 0;
				}
				pEntry->fgRaLimitPhyRate = TRUE;
				RTEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_MAXRA, pEntry, sizeof(MAC_TABLE_ENTRY));
			}
		} else {
			u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;
			if (u2TxTP < LIMIT_MAX_PHY_RATE_THRESHOLD) {
				/* restore to default rate */
				raentry->u2SupportVHTMCS1SS = pEntry->SupportVHTMCS1SS;
				raentry->u2SupportVHTMCS2SS = pEntry->SupportVHTMCS2SS;
				raentry->u2SupportVHTMCS3SS = pEntry->SupportVHTMCS3SS;
				raentry->u2SupportVHTMCS4SS = pEntry->SupportVHTMCS4SS;
				mcs = &strec->he_sta.max_nss_mcs;
				os_move_mem(mcs->bw80_mcs, pEntry->cap.rate.he80_rx_nss_mcs,
					sizeof(mcs->bw80_mcs));
				os_move_mem(mcs->bw160_mcs, pEntry->cap.rate.he160_rx_nss_mcs,
					sizeof(mcs->bw160_mcs));
				os_move_mem(mcs->bw8080_mcs, pEntry->cap.rate.he8080_rx_nss_mcs,
					sizeof(mcs->bw8080_mcs));
				RTEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_MAXRA, pEntry, sizeof(MAC_TABLE_ENTRY));
				pEntry->fgRaLimitPhyRate = FALSE;
			}
#endif
		}

#endif /* DOT11_VHT_AC */
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
#ifdef APCLI_SUPPORT

		if (IS_ENTRY_PEER_AP(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];

			pApCliEntry->StaStatistic.OneSecTxBytes = pEntry->OneSecTxBytes;
			pApCliEntry->StaStatistic.OneSecRxBytes = pEntry->OneSecRxBytes;
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			if (pEntry->OneSecTxBytes || pEntry->OneSecRxBytes) {
				pEntry->NoDataIdleCount++;
			} else {
				pEntry->NoDataIdleCount = 0;
			}
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/

		}

#endif /* APCLI_SUPPORT */
#ifdef RED_SUPPORT
		if ((pAd->mcli_ctl[BandIdx].tx_cnt_from_red == TRUE) && (pAd->red_en)) {
			pAd->mcli_ctl[BandIdx].tot_tx_cnt += pEntry->one_sec_tx_mpdu_pkts;
			if (pEntry->one_sec_tx_mpdu_pkts > pEntry->one_sec_tx_mpdu_succ_pkts)
				pAd->mcli_ctl[BandIdx].tot_tx_fail_cnt +=
					(pEntry->one_sec_tx_mpdu_pkts - pEntry->one_sec_tx_mpdu_succ_pkts);
			pEntry->one_sec_tx_mpdu_pkts = 0;
		}
#endif
#ifdef WHNAT_SUPPORT
		if ((pAd->CommonCfg.whnat_en) && (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))) {
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
			cr4_query_list->list[cr4_query_list->number++] = wcid;
			if (cr4_query_list->number == MAX_CR4_QUERY_NUM) {
				MtCmdCr4MultiQuery(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_query_list->number, 0, cr4_query_list);
				cr4_query_list->number = 0;
			}
#else
			MtCmdCr4Query(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, wcid, 0);
#endif
			tx_offload_counter_update(pAd, pEntry);
#ifdef DELAY_TCP_ACK_V2 /*for panther or cheetah*/
			if (IS_MT7986(pAd) || IS_MT7981(pAd)) {
				UINT8 tid_num = 0;
				for (tid_num = 0; tid_num < NUM_OF_TID; tid_num++) {
					pEntry->OneSecRxBytes += (pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt - pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt);
					pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt = pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_byte_cnt;
#ifdef RX_COUNT_DETECT
					pEntry->one_sec_rx_pkts += (pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt - pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt);
					pAd->wo_last_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt = pAd->wo_rxcnt[tid_num][pEntry->wcid].rx_pkt_cnt;
#endif /* RX_COUNT_DETECT */
				}
				/* for panther, report rx pkts from wo-900ms/wa-1000ms, need transfer */
				pEntry->OneSecRxBytes = (pEntry->OneSecRxBytes * 100) / (PEAK_TP_WO_REPORT_TIME * 15);
				pEntry->one_sec_rx_pkts = (pEntry->one_sec_rx_pkts * 100) / (PEAK_TP_WO_REPORT_TIME * 15);
			}
#endif /* DELAY_TCP_ACK_V2 */
		} else
#endif
		{
			pEntry->AvgTxBytes = (pEntry->AvgTxBytes == 0) ?
								 pEntry->OneSecTxBytes :
								 ((pEntry->AvgTxBytes + pEntry->OneSecTxBytes) >> 1);
			pEntry->avg_tx_pkts = (pEntry->avg_tx_pkts == 0) ? \
								  pEntry->one_sec_tx_pkts : \
								  ((pEntry->avg_tx_pkts + pEntry->one_sec_tx_pkts) >> 1);
			pEntry->one_sec_tx_pkts = 0;
		}

		pEntry->AvgRxBytes = (pEntry->AvgRxBytes == 0) ?
							 pEntry->OneSecRxBytes :
							 ((pEntry->AvgRxBytes + pEntry->OneSecRxBytes) >> 1);
#ifdef SMART_CARRIER_SENSE_SUPPORT

		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			pAd->SCSCtrl.OneSecRxByteCount[BandIdx] += pEntry->OneSecRxBytes;
			pAd->SCSCtrl.OneSecTxByteCount[BandIdx] += pEntry->OneSecTxBytes;

			if (pAd->SCSCtrl.SCSEnable[BandIdx] == SCS_ENABLE) {
				tmpRssidata = RTMPMinRssi(pAd, pEntry->ScsDataRssi[0], pEntry->ScsDataRssi[1],
							  pEntry->ScsDataRssi[2], pEntry->ScsDataRssi[3]);
				tmpRssiba = RTMPMinRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
									  pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
				tmpRssimin = min(tmpRssidata, tmpRssiba);

				if (tmpRssimin < pAd->SCSCtrl.SCSMinRssi[BandIdx])
					pAd->SCSCtrl.SCSMinRssi[BandIdx] = tmpRssimin;
			}
		}

#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT

		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			pAd->DynWmmCtrl.OneSecRxByteCount[BandIdx] += pEntry->OneSecRxBytes;
			pAd->DynWmmCtrl.OneSecTxByteCount[BandIdx] += pEntry->OneSecTxBytes;
		}

#endif /* DYNAMIC_WMM_SUPPORT */

		pEntry->OneSecRxBytes = 0;
		pEntry->OneSecTxBytes = 0;

#ifdef RX_COUNT_DETECT
		pEntry->avg_rx_pkts = (pEntry->avg_rx_pkts == 0) ? \
							  pEntry->one_sec_rx_pkts : \
							  ((pEntry->avg_rx_pkts + pEntry->one_sec_rx_pkts) >> 1);
		pEntry->one_sec_rx_pkts = 0;
#endif /* RX_COUNT_DETECT */
#ifdef KERNEL_RPS_ADJUST
		if (pAd->ixia_mode_ctl.kernel_rps_en)
			detect_1_pair_peak(pAd, pEntry, BandIdx);
#endif
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_PEER_AP(pEntry)) {
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[pEntry->func_tb_idx];

			if (pApCliEntry->ApcliInfStat.Valid == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				{
					struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

					if (cap->fgRateAdaptFWOffload == TRUE) {
						if (!pEntry->bTxPktChk &&
							RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime + (1 * OS_HZ))) {
							pEntry->bTxPktChk = TRUE;
							pEntry->TotalTxSuccessCnt = 0;
							pEntry->TxStatRspCnt = 0;
						} else if (pEntry->bTxPktChk) {
							 if ((pEntry->TxStatRspCnt >= 1) && (pEntry->TotalTxSuccessCnt)) {
									pApCliEntry->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
									pEntry->bTxPktChk = FALSE;
							 }
						}
					}
				}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			}
		}
#endif /* APCLI_SUPPORT */
#ifdef KERNEL_RPS_ADJUST
	if (!pAd->ixia_mode_ctl.mode_entered)
#endif
#ifdef ERR_RECOVERY
        if (!IsStopingPdma(&pAd->ErrRecoveryCtl))
#endif
		{
			if (p_tx_statics_queue) {
				if (tx_statics_qcnt < MAX_FW_AGG_MSG) {
					p_temp = p_tx_statics_queue + tx_statics_qcnt;
					p_temp->Field = GET_TX_STAT_ENTRY_TX_CNT;
					p_temp->Band = BandIdx;
					p_temp->Wcid = wcid;
					tx_statics_qcnt++;
				}

				if (tx_statics_qcnt == MAX_FW_AGG_MSG) {
					HW_GET_TX_STATISTIC(pAd, p_tx_statics_queue, MAX_FW_AGG_MSG);
					tx_statics_qcnt = 0;
					os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);
				}
			}
		}
	/*notify tput detect*/
	call_traffic_notifieriers(TRAFFIC_NOTIFY_TPUT_DETECT, pAd, pEntry);

#ifdef APCLI_SUPPORT

		if ((IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		   ) {
			sta_cfg = GetStaCfgByWdev(pAd, pEntry->wdev);
#ifdef MAC_REPEATER_SUPPORT

			if (IS_REPT_LINK_UP(pEntry->pReptCli)) {
				pEntry->pReptCli->ReptCliIdleCount++;

				if (IS_REPT_CLI_TYPE(pEntry->pReptCli, REPT_ETH_CLI)
					&& (pEntry->pReptCli->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME)
					&& (!IS_REPT_CLI_TYPE(pEntry->pReptCli, REPT_BRIDGE_CLI))) { /* Do NOT ageout br0 link. @2016/1/27 */
					RepeaterDisconnectRootAP(pAd, pEntry->pReptCli, APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA);
					continue;
				}
			}

#endif /* MAC_REPEATER_SUPPORT */

			if (IS_ENTRY_PEER_AP(pEntry)) {
				apcli_wdev = pEntry->wdev;
				pApcliEntry = pEntry;
			}

			apcli_avg_tx += pEntry->AvgTxBytes;
			apcli_avg_rx += pEntry->AvgRxBytes;
			if (pEntry->wdev) {
				UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT8 traffc_mode = 0;

				if ((tx_tp + rx_tp) == 0)
					traffc_mode = TRAFFIC_0;
				else if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
					traffc_mode = TRAFFIC_DL_MODE;
				else if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD)
					traffc_mode = TRAFFIC_UL_MODE;

				/* Detect main STA(Maximum TP of TX+RX) traffic mode */
				peak_tp_ctl = &pAd->peak_tp_ctl[BandIdx];
				if (peak_tp_ctl->main_wdev == NULL)
					peak_tp_ctl->main_wdev = pEntry->wdev;
				peak_tp_ctl->client_nums++;
				if ((tx_tp + rx_tp) > (peak_tp_ctl->main_tx_tp + peak_tp_ctl->main_rx_tp)) {
					peak_tp_ctl->main_tx_tp = tx_tp;
					peak_tp_ctl->main_rx_tp = rx_tp;
					peak_tp_ctl->main_traffc_mode = traffc_mode;
					peak_tp_ctl->main_entry = pEntry;
					peak_tp_ctl->main_wdev = pEntry->wdev;
				}
			}

			if (((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
#ifdef CONFIG_MULTI_CHANNEL
				&& (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			   ) {
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
					ApclibQosNull = TRUE;
				}

					if (sta_cfg && !sta_cfg->PwrMgmt.bDoze) {
						USHORT PwrMgmt = PWR_ACTIVE;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
						if (twtPlannerIsRunning(pAd, sta_cfg))
							PwrMgmt = PWR_SAVE;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
						ApCliRTMPSendNullFrame(pAd, pEntry->CurrTxRate, ApclibQosNull, pEntry, PwrMgmt);
					}
				continue;
			}
		}

#endif /* APCLI_SUPPORT */

		if (!IS_ENTRY_CLIENT(pEntry)) {
#ifdef RED_SUPPORT
			if ((pAd->mcli_ctl[BandIdx].tx_cnt_from_red == TRUE) && (pAd->red_en))
				pEntry->one_sec_tx_mpdu_succ_pkts = 0;
#endif
			pEntry->one_sec_tx_succ_pkts = 0;
			continue;
		}
		if (pEntry->fgGband256QAMSupport && (pEntry->RXBAbitmap != 0) && (pEntry->TXBAbitmap != 0) &&
			sta_hit_2g_infra_case_number <= STA_NUMBER_FOR_TRIGGER) {
			sta_wdev = pEntry->wdev;

			if (WMODE_CAP_2G(sta_wdev->PhyMode)) {
				UINT tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);

				if (tx_tp > INFRA_TP_PEEK_BOUND_THRESHOLD && ((tx_tp + rx_tp) > 0) &&
					(tx_tp * 100) / (tx_tp + rx_tp) > TX_MODE_RATIO_THRESHOLD) {
					if (sta_hit_2g_infra_case_number < STA_NUMBER_FOR_TRIGGER) {
						txop_wdev = sta_wdev;
						sta_hit_2g_infra_case_number++;
					} else
						sta_hit_2g_infra_case_number++;
				}
			}
		}

		if (pEntry->wdev) {
			UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
			UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
			ULONG avg_tx_b = pEntry->AvgTxBytes;
			ULONG avg_rx_b = pEntry->AvgRxBytes;
			UINT8 traffc_mode = TRAFFIC_0;
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			ULONG data_rate = 0;
			UINT32 tp_ratio = 0;
			UINT8 bidir_traffc_mode = 0;
#endif

			/* get associated band with wdev*/
			BandIdx = HcGetBandByWdev(pEntry->wdev);

			if ((avg_tx_b + avg_rx_b) > 0) {
				tx_ratio = TX_MODE_RATIO_THRESHOLD;
				if (pEntry->avg_rx_pkt_len < 256)
					rx_ratio = 50;
				else
					rx_ratio = RX_MODE_RATIO_THRESHOLD;
				is_tx_traffic = (avg_tx_b > (((avg_tx_b + avg_rx_b) / 10) * (tx_ratio / 10))) ? TRUE : FALSE;
				is_rx_traffic = (avg_rx_b > (((avg_tx_b + avg_rx_b) / 10) * (rx_ratio / 10))) ? TRUE : FALSE;
			} else {
				is_tx_traffic = FALSE;
				is_rx_traffic = FALSE;
				traffc_mode = TRAFFIC_0;
			}

			if (is_tx_traffic)
				traffc_mode = TRAFFIC_DL_MODE;
			else if (is_rx_traffic)
				traffc_mode = TRAFFIC_UL_MODE;

			/* Detect main STA(Maximum TP of TX+RX) traffic mode */
			peak_tp_ctl = &pAd->peak_tp_ctl[BandIdx];
			if (peak_tp_ctl->main_wdev == NULL)
				peak_tp_ctl->main_wdev = pEntry->wdev;
			peak_tp_ctl->client_nums++;
			if ((avg_tx_b + avg_rx_b) > peak_tp_ctl->main_txrx_bytes) {
				peak_tp_ctl->main_tx_tp = tx_tp;
				peak_tp_ctl->main_rx_tp = rx_tp;
#ifdef DELAY_TCP_ACK_V2
				peak_tp_ctl->main_tx_tp_record = tx_tp;
				peak_tp_ctl->main_rx_tp_record = rx_tp;
#endif /* DELAY_TCP_ACK_V2 */
				peak_tp_ctl->main_txrx_bytes = avg_tx_b + avg_rx_b;
				peak_tp_ctl->main_traffc_mode = traffc_mode;
				peak_tp_ctl->main_entry = pEntry;
				peak_tp_ctl->main_wdev = pEntry->wdev;
			}

#ifdef CONFIG_TX_DELAY
			if (IS_TX_DELAY_SW_MODE(cap) && !pAd->CommonCfg.dbdc_mode) {
				struct tx_delay_control *tx_delay_ctl = NULL;
				struct qm_ops *qm_ops = pAd->qm_ops;
				UINT8 band_idx = HcGetBandByWdev(pEntry->wdev);
				tx_delay_ctl = qm_ops->get_qm_delay_ctl(pAd, band_idx);

				if ((rx_tp >= tx_delay_ctl->min_tx_delay_en_tp)
						&& (rx_tp <= tx_delay_ctl->max_tx_delay_en_tp)) {
					if (!tx_delay_ctl->que_agg_en)
						tx_delay_ctl->que_agg_en = TRUE;
				} else {
					if (tx_delay_ctl->que_agg_en)
						tx_delay_ctl->que_agg_en = FALSE;
				}
			}
#endif /* CONFIG_TX_DELAY */

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
				if ((IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_LUMPED)
					|| IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_PER_RING))
#ifdef ERR_RECOVERY
					&& !IsStopingPdma(&pAd->ErrRecoveryCtl)
#endif
				) {
					pci_dynamic_dly_int_adjust(pAd->hdev_ctrl, tx_tp, rx_tp);
				}
			}
#endif

			if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_PER_STA) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"STA%d:avg_tx_b:%lu,avg_rx_b:%lu,avg_tx_pkts:%lu,avg_rx_pkts:%lu,rx_ratio:%u\n",
					pEntry->wcid, avg_tx_b, avg_rx_b, pEntry->avg_tx_pkts, pEntry->avg_rx_pkts, rx_ratio);
			}

			if (WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;
				if ((pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH || avg_tx_b > MCLI_TRAFFIC_TP_TH) && is_tx_traffic)
					pAd->txop_ctl[BandIdx].multi_client_nums++;
			}
#ifdef RX_COUNT_DETECT
			if (WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;
				if ((pEntry->avg_rx_pkts > VERIWAVE_5G_PKT_CNT_TH || avg_rx_b > MCLI_TRAFFIC_TP_TH) && is_rx_traffic)
					pAd->txop_ctl[BandIdx].multi_rx_client_nums++;
			}
#endif

			if (WMODE_CAP_2G(pEntry->wdev->PhyMode)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;
				if ((pEntry->avg_tx_pkts > VERIWAVE_2G_PKT_CNT_TH || avg_tx_b > MCLI_TRAFFIC_TP_TH) && is_tx_traffic)
					pAd->txop_ctl[BandIdx].multi_client_nums++;
			}
#ifdef RX_COUNT_DETECT
			if (WMODE_CAP_2G(pEntry->wdev->PhyMode)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;
				if ((pEntry->avg_rx_pkts > VERIWAVE_2G_PKT_CNT_TH || avg_rx_b > MCLI_TRAFFIC_TP_TH) && is_rx_traffic)
					pAd->txop_ctl[BandIdx].multi_rx_client_nums++;
			}
#endif
#ifdef VOW_SUPPORT
			/* check if having TCP flow per STA while SPL is disabled */
			if ((pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) &&
			    (pAd->vow_misc_cfg.spl_sta_count == 0))
				check_sta_tcp_flow(pAd, tr_entry, BandIdx);

			if (pAd->vow_cfg.mcli_schedule_en && pAd->Mlme.OneSecPeriodicRound % 3 == 0) {
				if ((is_tx_traffic && pEntry->mcliTcpCnt > pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th)
					|| (is_rx_traffic && pEntry->mcliTcpAckCnt > pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th))
					pAd->vow_cfg.mcli_sch_cfg.mcli_tcp_num[BandIdx]++;

				if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_PER_STA)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"STA%d: TxRxtraffic(%u/%u), Tcp/Ack(%d/%d)\n",
						pEntry->wcid, is_tx_traffic, is_rx_traffic,
						pEntry->mcliTcpCnt, pEntry->mcliTcpAckCnt);

				pEntry->mcliTcpCnt = 0;
				pEntry->mcliTcpAckCnt = 0;
			}
#endif

			avg_pkt_len_calculate(pEntry, BandIdx);
			CalFarClientNum(pAd, pEntry, BandIdx);

#ifdef RED_SUPPORT
			if ((pAd->mcli_ctl[BandIdx].tx_cnt_from_red == TRUE) && (pAd->red_en)) {
#ifdef KERNEL_RPS_ADJUST
				if (pAd->ixia_mode_ctl.mode_entered) {
					if (pEntry->one_sec_tx_mpdu_succ_pkts > INFRA_KEEP_STA_PKT_TH &&
						pEntry->NoDataIdleCount != 0)
						pEntry->NoDataIdleCount = 0;
#ifdef RX_COUNT_DETECT
					if (pEntry->one_sec_rx_pkts > INFRA_KEEP_STA_PKT_TH)
						is_no_rx_cnt = FALSE;
					else
#endif
						is_no_rx_cnt = TRUE;
				}
#endif
				pEntry->one_sec_tx_mpdu_succ_pkts = 0;
			}
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (pEntry->one_sec_tx_succ_pkts > 0 && pEntry->NoDataIdleCount != 0) {
#else
			if (pEntry->one_sec_tx_succ_pkts > INFRA_KEEP_STA_PKT_TH && pEntry->NoDataIdleCount != 0) {
#endif
				pEntry->NoDataIdleCount = 0;
				pEntry->one_sec_tx_succ_pkts = 0;
				is_no_rx_cnt = TRUE;
			}

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			if (IS_MAP_ENABLE(pAd)) {
				getRate(pEntry->HTPhyMode, &data_rate);
				tp_ratio = ((tx_tp + rx_tp) * 100) / data_rate;

				if (tp_ratio > STA_TP_IDLE_THRESHOLD)
					bidir_traffc_mode = TRAFFIC_BIDIR_ACTIVE_MODE;
				else
					bidir_traffc_mode = TRAFFIC_BIDIR_IDLE_MODE;

				if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_ACTIVE_MODE) &&
					(tp_ratio <= STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, INACTIVE);
				else if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_IDLE_MODE) &&
					(tp_ratio > STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, ACTIVE);

				pEntry->pre_traffic_mode = bidir_traffc_mode;
			}
#endif /*WAPP_SUPPORT CONFIG_MAP_SUPPORT*/
		}

		if (pEntry->NoDataIdleCount == 0) {
			pEntry->StationKeepAliveCount = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			pEntry->bTxPktChk = FALSE;
			pEntry->ContinueTxFailCnt = 0;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		pEntry->LastRxTimeCount++;
#endif
		pEntry->NoDataIdleCount++;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount++;
		pEntry->StaConnectTime++;
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine) && !pEntry->sta_force_keep) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 MACSTR" fail to complete ASSOC in %lu sec\n",
					  MAC2STR(pEntry->Addr), pEntry->AssocDeadLine);
#ifdef WSC_AP_SUPPORT

			if (NdisEqualMemory(pEntry->Addr, pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
				mac_entry_delete(pAd, pEntry);
			continue;
		} else {
			do_sta_keep_action(pAd, pEntry, is_no_rx_cnt);
		}
		/*
			1. check if there's any associated STA in power-save mode. this affects outgoing
				MCAST/BCAST frames should be stored in PSQ till DtimCount=0
		*/
		if (pEntry->PsMode == PWR_SAVE) {
			pMacTable->fAnyStationInPsm = TRUE;

			if (pEntry->wdev &&
				(pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO)) {
				/* TODO: it looks like pEntry->wdev->tr_tb_idx is not assigned? */
				tr_ctl->tr_entry[pEntry->wdev->tr_tb_idx].PsMode = PWR_SAVE;

				if (tr_entry->PsDeQWaitCnt) {
					tr_entry->PsDeQWaitCnt++;

					if (tr_entry->PsDeQWaitCnt > 2)
						tr_entry->PsDeQWaitCnt = 0;
				}
			}
		}

#ifdef DOT11_N_SUPPORT

		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD) {
			if (pEntry->wdev) {
				UCHAR band = HcGetBandByWdev(pEntry->wdev);
				UINT16 non_gf_sta = wlan_operate_get_non_gf_sta(wdev);

				non_gf_sta++;
				wlan_operate_set_non_gf_sta(wdev, non_gf_sta);
				pMacTable->fAnyStationNonGF[band] = TRUE;
			}
		}

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = TRUE;

#ifdef DOT11N_DRAFT3

		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;

#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */
		/* detect the station alive status */

		/* detect the station alive status */
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime)) {
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

			    ......
			    sam> tx ok!
			    sam> count = 1!	 ==> 1 second after the Null Frame is acked
			    sam> count = 2!	 ==> 2 second after the Null Frame is acked
			    sam> count = 3!
			    sam> count = 4!
			    sam> count = 5!
			    sam> count = 6!
			    sam> count = 7!
			    sam> count = 8!
			    sam> count = 9!
			    sam> count = 10!
			    sam> count = 11!
			    sam> count = 12!
			    sam> count = 13!
			    sam> count = 14!
			    sam> count = 15! ==> 15 second after the Null Frame is acked
			    sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
			    sam> count = 1!  ==> 1 second after the Null Frame is acked
			    sam> count = 2!
			    sam> count = 3!
			    sam> count = 4!
			    ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/
			if (pEntry->StationKeepAliveCount++ == 0) {
#ifdef P2P_SUPPORT

				/* Modify for P2P test plan 6.1.12, enqueue null frame will influence the test item */
				if (pAd->P2pCfg.bSigmaEnabled == FALSE) {
#endif /* P2P_SUPPORT */
					/* use Null or QoS Null to detect the ACTIVE station */
					BOOLEAN bQosNull = FALSE;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					if (!pEntry->bTxPktChk) {
						pEntry->TotalTxSuccessCnt = 0;
						pEntry->TxStatRspCnt = 0;
						pEntry->bTxPktChk = TRUE;
					}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

					if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
						bQosNull = TRUE;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
					send_dummy_data_packet(pAd, pEntry);
#else
					RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
							pEntry->wcid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
#endif
#ifdef P2P_SUPPORT
				}

#endif /* P2P_SUPPORT */
			} else {
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime) {
					pEntry->StationKeepAliveCount = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					pEntry->bTxPktChk = FALSE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				}
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				else if (pEntry->bTxPktChk) {
					if ((pEntry->TxStatRspCnt >= 1) && (pEntry->TotalTxSuccessCnt > 0)) {
						pEntry->NoDataIdleCount = 0;
						pEntry->StationKeepAliveCount = 0;
						pEntry->ContinueTxFailCnt = 0;
						pEntry->bTxPktChk = FALSE;
					}
				}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
		/* If TWT agreement is present for this STA, add maximum TWT wake up interval in sta idle timeout. */
		if (pEntry->twt_flow_id_bitmap) {
			sta_idle_timeout = pEntry->StaIdleTimeout + pEntry->twt_interval_max;
		} else
#endif
		{
			sta_idle_timeout = pEntry->StaIdleTimeout;
		}

		/* StaIdleTimeout(TU) * 1024/1000 -> StaIdleTimeout(sec) */
		if (pEntry->NoDataIdleCount > (pEntry->StaIdleTimeout * 1024) / 1000) {
#ifdef VERIFICATION_MODE
			if (pAd->veri_ctrl.skip_ageout == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"skip ageout: "MACSTR" aleady slient for %d-sece\n",
					MAC2STR(pEntry->Addr),
					(UINT32)pEntry->NoDataIdleCount);
			} else
#endif
			{

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
				/*
					keep TWT PS only , but kick all STAs (even PS STA)
					too much PS STA may stuck the Tx(no S/W Tokens), need to drain the token in WA/WM PS flush flow.
				*/
				if (pEntry->twt_flow_id_bitmap == 0)
#else
				/*
					AP have no way to know that the PwrSaving STA is leaving or not.
					So do not disconnect for PwrSaving STA.
				*/
				if (pEntry->PsMode != PWR_SAVE)
#endif
				{
					bDisconnectSta = TRUE;
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"ageout "MACSTR" after %d-sec silence\n",
							 MAC2STR(pEntry->Addr), sta_idle_timeout);
					ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WH_EVENT_NOTIFIER
					if (pEntry) {
						EventHdlr pEventHdlrHook = NULL;

						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry);
					}
#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_DIAG
					if (pEntry && IS_ENTRY_CLIENT(pEntry))
						diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
							DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
#ifdef CONN_FAIL_EVENT
					if (IS_ENTRY_CLIENT(pEntry))
						ApSendConnFailMsg(pAd,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
							pEntry->Addr,
							REASON_DEAUTH_STA_LEAVING);
#endif
				}
			}
		} else if (
					(pEntry->per_err_times >= pAd->ApCfg.per_err_total
					&& pEntry->tx_contd_fail_cnt >= pAd->ApCfg.tx_contd_fail_total)
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				|| (pEntry->bTxPktChk &&
					(pEntry->TxStatRspCnt >= 1) &&
					(pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				/*
				 * For circumstance that STA powers off without sending a deauth
				 * frame, and it will cause ple continuous retry
				 */
#if defined(IGMP_SNOOPING_NON_OFFLOAD) && defined(WHNAT_SUPPORT)
				|| (que && (pEntry->per_err_times >= CONTD_PER_ERR_CNT_MC
					&& atomic_read(&que->free_token_cnt) <= MIN_FREE_TKN_CNT
					&& tr_entry->token_cnt >= MAX_TKN_CNT_PER_STA
					&& pAd->CommonCfg.whnat_en))
#endif
		) {

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
			/*
				keep TWT PS only , but kick all STAs (even PS STA)
				too much PS STA may stuck the Tx(no S/W Tokens), need to drain the token in WA/WM PS flush flow.
			*/
			if (pEntry->twt_flow_id_bitmap == 0)
#else
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
#endif
			{
				bDisconnectSta = TRUE;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"STA-"MACSTR" had left (%d %lu)\n", MAC2STR(pEntry->Addr),
						 pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck);
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"###Notice: %s() will delete wcid[%d] mac entry\n", __func__, wcid);
#ifdef IGMP_SNOOPING_NON_OFFLOAD
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"per_err_times = %d, token used by current wcid = %d, rest of token = %d\n",
						pEntry->per_err_times, tr_entry->token_cnt, que ? atomic_read(&que->free_token_cnt) : 0);
#endif
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"per_err_times = %d, tx continue fail cnt = %lu\n",
						pEntry->per_err_times, pEntry->tx_contd_fail_cnt);
#ifdef WH_EVENT_NOTIFIER

				if (pEntry) {
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);

					if (pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry);
				}

#endif /* WH_EVENT_NOTIFIER */
#ifdef WIFI_DIAG
				if (pEntry && IS_ENTRY_CLIENT(pEntry))
					diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_FRAME_LOST, REASON_CONTINUE_TX_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_DEAUTH_STA_LEAVING);
#endif
			}
		}

		avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

		if ( pEntry->RssiSample.Rssi_Updated && pMbss->RssiLowForStaKickOut &&
			(avgRssi < pMbss->RssiLowForStaKickOut)) {
			bDisconnectSta = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
					 "Disassoc STA "MACSTR", RSSI Kickout Thres[%d]-[%d]\n",
					  MAC2STR(pEntry->Addr), pMbss->RssiLowForStaKickOut,	avgRssi);
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_DEAUTH, REASON_RSSI_TOO_LOW);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DEAUTH_STA_LEAVING);
#endif
		}

		if (bDisconnectSta) {
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);

			if (pEntry->Sst == SST_ASSOC) {
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;
				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, " MlmeAllocateMemory fail  ..\n");
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}

				Reason = REASON_DEAUTH_STA_LEAVING;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x\n",
						 Reason, PRINT_MAC(pEntry->Addr));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								  sizeof(HEADER_802_11), &DeAuthHdr,
								  2, &Reason,
								  END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);
			}

				mac_entry_delete(pAd, pEntry);
			continue;
		}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)

		if (pEntry->BTMDisassocCount == 1) {
			PUCHAR      pOutBuffer = NULL;
			NDIS_STATUS NStatus;
			ULONG       FrameLen = 0;
			HEADER_802_11 DisassocHdr;
			USHORT      Reason;
			/*  send out a DISASSOC request frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

			if (NStatus != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " MlmeAllocateMemory fail  ..\n");
				/*NdisReleaseSpinLock(&pAd->MacTabLock); */
				continue;
			}

			Reason = REASON_DISASSOC_INACTIVE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n", Reason, pEntry->Addr[0], pEntry->Addr[1],
					  pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
			MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
			MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);
			MlmeFreeMemory(pOutBuffer);
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					Reason);
#endif

			/* JERRY */
			if (!pEntry->IsKeep) {
					mac_entry_delete(pAd, pEntry);
			}

			continue;
		}

		if (pEntry->BTMDisassocCount != 0)
			pEntry->BTMDisassocCount--;

#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the ps_queue if the STA has being idle for a while */
		if ((pEntry->PsMode == PWR_SAVE) && (tr_entry->ps_state == APPS_RETRIEVE_DONE ||
											 tr_entry->ps_state == APPS_RETRIEVE_IDLE)) {
			if (tr_entry->enqCount > 0) {
				tr_entry->PsQIdleCount++;

				if (tr_entry->PsQIdleCount > 5) {
					struct qm_ops *ops = pAd->qm_ops;

					if (ops->sta_clean_queue)
						ops->sta_clean_queue(pAd, pEntry->wcid);

					tr_entry->PsQIdleCount = 0;
					WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Clear WCID[%d] packets\n", pEntry->wcid);
				}
			}
		} else
			tr_entry->PsQIdleCount = 0;

#ifdef UAPSD_SUPPORT
		UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) {
			UCHAR band_idx;

			band_idx = HcGetBandByWdev(pEntry->wdev);
			pMacTable->fAllStationAsRalink[band_idx] = FALSE;
		}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->func_tb_idx);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
			pMacTable->fAllStationGainGoodMCS = FALSE;

		/* Check Current STA's Operation Mode is BW20 or BW40 */
		pMacTable->fCurrentStaBw40 = (pEntry->HTPhyMode.field.BW == BW_40) ? TRUE : FALSE;
#ifdef WH_EVENT_NOTIFIER

		if (pAd->ApCfg.EventNotifyCfg.bStaRssiDetect) {
			avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

			if (avgRssi < pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold) {
				if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
					&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
					&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
				   ) {
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_RSSI_TOO_LOW);

					if (pEventHdlrHook && pEntry->wdev)
						pEventHdlrHook(pAd, pEntry->wdev, pEntry->Addr, avgRssi);
				}
			}
		}
#endif
#if (defined(WH_EVENT_NOTIFIER) || (defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)))
		if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
			&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		   ) {
#ifdef WH_EVENT_NOTIFIER
			EventHdlr pEventHdlrHook = NULL;
			struct EventNotifierCfg *pEventNotifierCfg = &pAd->ApCfg.EventNotifyCfg;

			if (pEventNotifierCfg->bStaStateTxDetect && (pEventNotifierCfg->StaTxPktDetectPeriod > 0)) {
				pEventNotifierCfg->StaTxPktDetectRound++;

				if (((pEventNotifierCfg->StaTxPktDetectRound % pEventNotifierCfg->StaTxPktDetectPeriod) == 0)) {
					if ((pEntry->tx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
						(pEntry->tx_state.PacketCount < pEventNotifierCfg->StaStateTxThreshold)) {
						pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, TRUE);
					} else if ((pEntry->tx_state.CurrentState == WHC_STA_STATE_IDLE) &&
							   (pEntry->tx_state.PacketCount >= pEventNotifierCfg->StaStateTxThreshold)) {
						pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, TRUE);
					}

					pEventNotifierCfg->StaTxPktDetectRound = 0;
					pEntry->tx_state.PacketCount = 0;
				}
			}

			if (pEventNotifierCfg->bStaStateRxDetect && (pEventNotifierCfg->StaRxPktDetectPeriod > 0)) {
				pEventNotifierCfg->StaRxPktDetectRound++;

				if (((pEventNotifierCfg->StaRxPktDetectRound % pEventNotifierCfg->StaRxPktDetectPeriod) == 0)) {
					if ((pEntry->rx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
						(pEntry->rx_state.PacketCount < pEventNotifierCfg->StaStateRxThreshold)) {
						pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, FALSE);
					} else if ((pEntry->rx_state.CurrentState == WHC_STA_STATE_IDLE) &&
							   (pEntry->rx_state.PacketCount >= pEventNotifierCfg->StaStateRxThreshold)) {
						pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
						pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);

						if (pEventHdlrHook && pEntry->wdev)
							pEventHdlrHook(pAd, pEntry, FALSE);
					}

					pEventNotifierCfg->StaRxPktDetectRound = 0;
					pEntry->rx_state.PacketCount = 0;
				}
			}
#endif /* WH_EVENT_NOTIFIER */
		}

#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
		if (pMbss->a4_init && pMbss->wdev.IgmpSnoopEnable) {
									/* If Snooping & MWDS enabled*/
			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)	{
										/* Per 10 sec check */
				/* Logic implemented to do Periodic Multicast membership queries to NON-MWDS STAs
				 for update of entries in IGMP table, to avoid aging of interested members.
				 Also, in case another Membership querier detected in network, logic implemented to
				 hold internal query transmission to avoid flooding on network.
				 Decrement time tick counters for Query Sent & Query hold as applicable
				 Send the query once on each MBSS, for which both these counters are 0.
				 (Ensured that even with multiple STA on a MBSS, only one multicast query transmitted)*/

				if (pAd->bIGMPperiodicQuery == TRUE) {
					/* If Periodic IGMP query feature enabled */

					if ((pMbss->IgmpQueryHoldTick > 0) && (pMbss->IgmpQueryHoldTickChanged == FALSE)) {
						/*Decrement IgmpQueryHoldTick, only once for each MBSS*/
						pMbss->IgmpQueryHoldTick--;
						pMbss->IgmpQueryHoldTickChanged = TRUE;
					}

					if ((pAd->IgmpQuerySendTick > 0) && (IgmpQuerySendTickChanged == FALSE)) {
						/*Decrement IgmpQuerySendTick, only once for each MBSS*/
						pAd->IgmpQuerySendTick--;
						IgmpQuerySendTickChanged = TRUE;
					}

					if ((pMbss->IGMPPeriodicQuerySent == FALSE)
					   && ((pMbss->IgmpQueryHoldTick == 0) && (pAd->IgmpQuerySendTick == 0))) {
						/*transmit IGMP query on this MBSS, only once*/
						send_igmpv3_gen_query_pkt(pAd, pEntry);

						pMbss->IGMPPeriodicQuerySent = TRUE;
					}
				}

				if (pAd->bMLDperiodicQuery == TRUE) { /*If Periodic MLD query feature enabled*/

					if ((pMbss->MldQueryHoldTick > 0) && (pMbss->MldQueryHoldTickChanged == FALSE)) {
						/*Decrement MldQueryHoldTick, only once for each MBSS*/
						pMbss->MldQueryHoldTick--;
						pMbss->MldQueryHoldTickChanged = TRUE;
					}

					if ((pAd->MldQuerySendTick > 0) && (MldQuerySendTickChanged == FALSE)) {
						/*Decrement MldQuerySendTick, only once for each MBSS*/
						pAd->MldQuerySendTick--;
						MldQuerySendTickChanged = TRUE;
					}
					if ((pMbss->MLDPeriodicQuerySent == FALSE)
					   && ((pMbss->MldQueryHoldTick == 0) && (pAd->MldQuerySendTick == 0))) {
						/*transmit MLD query on this MBSS, only once*/
						send_mldv2_gen_query_pkt(pAd, pEntry);

						pMbss->MLDPeriodicQuerySent = TRUE;
					}

				}

			} else if ((pAd->Mlme.OneSecPeriodicRound % 10) == 1) { /* Per 11 sec check */

				if (pAd->IgmpQuerySendTick == 0) /* Set the period for IGMP query again */
					pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;

				if (pAd->MldQuerySendTick == 0) /* Set the period for MLD query again */
					pAd->MldQuerySendTick = QUERY_SEND_PERIOD;

				if (pMbss->IGMPPeriodicQuerySent == TRUE)
					pMbss->IGMPPeriodicQuerySent = FALSE; /* Reset flag for next period query */

				if (pMbss->MLDPeriodicQuerySent == TRUE)
					pMbss->MLDPeriodicQuerySent = FALSE; /* Reset flag for next period query */

				pMbss->IgmpQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */
				pMbss->MldQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */

			}
		}
#endif
#endif

#ifdef PS_STA_FLUSH_SUPPORT
		if (tr_entry->ps_state == PWR_SAVE)
			PsStaNum++;
#endif

#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
		tpc_req_entry_decision(pAd, pEntry);
#endif
#endif
	}

#ifdef PS_STA_FLUSH_SUPPORT
	pAd->MacTab.PsStaNum = PsStaNum;
	/* dynamic adjust ps sta msdu num */
	if ((pAd->MacTab.fPsSTAFlushManualMode == FALSE) && (pAd->Mlme.PeriodicRound % PS_FLUSH_DYNAMIC_EXCE_MULTIPE == 0)) {
		UINT16 max_msdu_num_new = 0;

		max_msdu_num_new = pAd->MacTab.PsStaNum ?
			pAd->MacTab.PsFlushThldTotalMsduNum / pAd->MacTab.PsStaNum : MAX_MSDU_NUM_IN_HW_QUEUE;

		if (max_msdu_num_new > MAX_MSDU_NUM_IN_HW_QUEUE)
			max_msdu_num_new = MAX_MSDU_NUM_IN_HW_QUEUE;

		if (max_msdu_num_new != pAd->MacTab.PsFlushPerStaMaxMsduNum) {
			pAd->MacTab.PsFlushPerStaMaxMsduNum = max_msdu_num_new;
			RTMP_SET_PS_FLOW_CTRL(pAd);
		}
	}
#endif

	/*if the number of station is less than MAX_FW_AGG_MSG*/
	if (tx_statics_qcnt && p_tx_statics_queue) {
		HW_GET_TX_STATISTIC(pAd, p_tx_statics_queue, tx_statics_qcnt);
		tx_statics_qcnt = 0;
		os_zero_mem(p_tx_statics_queue, sizeof(TX_STAT_STRUC) * MAX_FW_AGG_MSG);
	}

	if (p_tx_statics_queue)
		os_free_mem(p_tx_statics_queue);
#ifdef WHNAT_SUPPORT
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
	/*if the number of station is less than MAX_CR4_QUERY_NUM*/
	if (cr4_query_list) {
		if (cr4_query_list->number) {
			MtCmdCr4MultiQuery(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, cr4_query_list->number, 0, cr4_query_list);
		}
		os_free_mem(cr4_query_list);
	}
#endif
#endif
#ifdef CONFIG_TX_DELAY
	if (IS_TX_DELAY_HW_MODE(cap)) {
		struct qm_ops *qm_ops = pAd->qm_ops;
		UINT8 band_idx = HcGetBandByWdev(peak_tp_ctl->main_wdev);
		struct tx_delay_control *tx_delay_ctl = qm_ops->get_qm_delay_ctl(pAd, band_idx);
		UINT32 reg_val = 0;
		UINT32 ap_rx_peak_th = 0;
		/* No take DBDC case into consideration because only one PLE Delay CR */
		if (!pAd->CommonCfg.dbdc_mode) {
			peak_tp_ctl = &pAd->peak_tp_ctl[BAND0];
			if (peak_tp_ctl->main_wdev == NULL)
				ap_rx_peak_th = 0;
			else if (WMODE_CAP_5G(peak_tp_ctl->main_wdev->PhyMode))
				ap_rx_peak_th = cap->Ap5GPeakTpTH;
			else if (WMODE_CAP_2G(peak_tp_ctl->main_wdev->PhyMode))
				ap_rx_peak_th = cap->Ap2GPeakTpTH;

			/* only apply on non-wmm case */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
				ap_rx_peak_th != 0 &&
				peak_tp_ctl->main_rx_tp > ap_rx_peak_th &&
				peak_tp_ctl->main_traffc_mode == TRAFFIC_UL_MODE) {
				if (!tx_delay_ctl->hw_enabled) {
					tx_delay_ctl->hw_enabled = TRUE;
					reg_val |= SET_PLE_TX_DELAY_PAGE_THRES(tx_delay_ctl->tx_process_batch_cnt);
					reg_val |= SET_PLE_TX_DELAY_TIME_THRES(tx_delay_ctl->que_agg_timeout_value);
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TX_DELAY_CTRL, reg_val);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Enable PLE Delay with %x\n", reg_val);

				}
			} else {
				if (tx_delay_ctl->hw_enabled) {
					tx_delay_ctl->hw_enabled = FALSE;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TX_DELAY_CTRL, 0);
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Disabled PLE Delay\n");
				}
			}
		}
	}
#endif /* CONFIG_TX_DELAY */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);

#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg) {
		if (pMacTable->fAllStationAsRalink[0])
			bRdgActive = TRUE;
		else
			bRdgActive = FALSE;

		if (pAd->CommonCfg.dbdc_mode) {
			if (pMacTable->fAllStationAsRalink[1])
				; /* Not support yet... */
		}
	}

#ifdef APCLI_SUPPORT

	if (apcli_wdev) {
		UINT tx_tp = (apcli_avg_tx >> BYTES_PER_SEC_TO_MBPS);
		UINT rx_tp = (apcli_avg_rx >> BYTES_PER_SEC_TO_MBPS);

		/* 7986/7916/7981 apcli do not use old dynamic txop algo*/
		if (IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			BOOLEAN isDL = FALSE;
			BOOLEAN isVHT = FALSE;
			u32 tr_ratio = 0;

			if (!pApcliEntry)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "pApcliEntry is NULL\n");
			else
				isVHT = (pApcliEntry->cap.modes  == VHT_SUPPORT);

			if (rx_tp > 0) {
				u32 tr_ratio = tx_tp / rx_tp;
				if (tr_ratio >= 5)
					isDL = TRUE;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "tx:%d rx:%d tr_ratio=%d isDL=%d 5g=%d, isVHT=%d\n",
				  tx_tp, rx_tp, tr_ratio, isDL, WMODE_CAP_5G(apcli_wdev->PhyMode), isVHT);
#ifdef DELAY_TCP_ACK_V2  /* panther dynamic txop */
			/* only dynamic change 5g UL/BIDI txop to 0 */
			if (rx_tp > pAd->CommonCfg.PeakTpRxLowerBoundTput && !isDL && WMODE_CAP_5G(apcli_wdev->PhyMode) && isVHT) {
				enable_tx_burst(pAd, apcli_wdev, AC_BE, PRIO_APCLI_REPEATER, TXOP_0);
			} else {
				disable_tx_burst(pAd, apcli_wdev, AC_BE, PRIO_APCLI_REPEATER, TXOP_0);
			}
#endif
		} else {
			apcli_dync_txop_alg(pAd, apcli_wdev, tx_tp, rx_tp);
		}
	}

#endif /* APCLI_SUPPORT */
	if (sta_hit_2g_infra_case_number == STA_NUMBER_FOR_TRIGGER) {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == FALSE) {
			pAd->g_mode_txop_wdev = txop_wdev;
			pAd->G_MODE_INFRA_TXOP_RUNNING = TRUE;
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
		} else if (pAd->g_mode_txop_wdev != txop_wdev) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
			pAd->g_mode_txop_wdev = txop_wdev;
		}
	} else {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == TRUE) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
			pAd->g_mode_txop_wdev = NULL;
		}
	}
#ifdef KERNEL_RPS_ADJUST
	if (pAd->ixia_mode_ctl.kernel_rps_en) {
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		dynamic_proc_rps_adjust(pAd);
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
}

#endif
#ifdef KERNEL_RPS_ADJUST
	if (!pAd->ixia_mode_ctl.mode_entered)
#endif
	dynamic_txop_adjust(pAd);
#ifdef DELAY_TCP_ACK_V2
	dynamic_peak_tp_txop_aifsn_adjust(pAd);
#endif /* DELAY_TCP_ACK_V2 */
	/*dynamic adjust amsdu & protection mode*/
	dynamic_amsdu_protect_adjust(pAd);
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	dynamic_near_far_adjust(pAd);
	dynamic_airtime_fairness_adjust(pAd);
	if (pAd->vow_cfg.mcli_schedule_en
		&& pAd->Mlme.OneSecPeriodicRound % 3 == 0)
		dynamic_mcli_param_adjust(pAd);
#endif

	dynamic_agg_per_sta_adjust(pAd);

#ifdef KERNEL_RPS_ADJUST
	if (!pAd->ixia_mode_ctl.kernel_rps_en)
#endif
	dynamic_sw_rps_control(pAd);

#ifdef KERNEL_RPS_ADJUST
	if (!pAd->ixia_mode_ctl.mode_entered)
#endif
	dynamic_ampdu_efficiency_adjust_all(pAd);

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = TRUE;
	else
		bRalinkBurstMode = FALSE;

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE)) {
		/* DBDC not support yet, only using BAND_0 */
	}

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);

#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

	/*
	   4.
	   garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
	   stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	/* TODO: shiang-usw. revise this becasue now we have per-BSS McastPsQueue! */
#ifdef RT_CFG80211_SUPPORT
#else
	if (pMacTable->McastPsQueue.Head) {
		UINT bss_index;

		pMacTable->PsQIdleCount++;

		if (pMacTable->PsQIdleCount > 1) {
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

			/* clear MCAST/BCAST backlog bit for all BSS */
			for (bss_index = BSS0; bss_index < pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	} else
		pMacTable->PsQIdleCount = 0;
#endif

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
#ifdef KERNEL_RPS_ADJUST
		if (!pAd->ixia_mode_ctl.kernel_rps_en)
#endif
		{
			pAd->txop_ctl[BandIdx].last_client_num = pAd->txop_ctl[BandIdx].multi_client_nums;
			pAd->txop_ctl[BandIdx].last_rx_client_num = pAd->txop_ctl[BandIdx].multi_rx_client_nums;
		}
		pAd->mcli_ctl[BandIdx].last_large_rssi_gap_num = pAd->mcli_ctl[BandIdx].large_rssi_gap_num;
		pAd->txop_ctl[BandIdx].last_tcp_nums =  pAd->txop_ctl[BandIdx].multi_tcp_nums;
		pAd->txop_ctl[BandIdx].multi_client_nums = 0;
		pAd->txop_ctl[BandIdx].multi_rx_client_nums = 0;
		pAd->txop_ctl[BandIdx].multi_tcp_nums = 0;
		pAd->mcli_ctl[BandIdx].tot_tx_cnt = 0;
		pAd->mcli_ctl[BandIdx].tot_tx_fail_cnt = 0;
		pAd->mcli_ctl[BandIdx].tot_tx_pkts = 0;
		pAd->mcli_ctl[BandIdx].tot_rx_pkts = 0;
		pAd->mcli_ctl[BandIdx].large_rssi_gap_num = 0;
		pAd->nearfar_far_client_num[BandIdx] = 0;
	}

#ifdef RED_SUPPORT
	if (pAd->ixia_mode_ctl.fgProbeRspDetect == TRUE) {
		if (pAd->ixia_mode_ctl.fgSkipRedQLenDrop == FALSE) {
			if (pAd->red_mcu_offload)
				red_qlen_drop_setting(pAd, 1);
			pAd->ixia_mode_ctl.fgSkipRedQLenDrop = TRUE;
		}
	} else {
		if (pAd->ixia_mode_ctl.fgSkipRedQLenDrop == TRUE) {
			pAd->ixia_mode_ctl.fgSkipRedQLenDrop = FALSE;
			if (pAd->red_mcu_offload)
				red_qlen_drop_setting(pAd, 0);
		}
	}
#endif

}

UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd)
{
	UINT32 num = 0;
	UINT32 i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num++;
	}

	return num;
}

/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	OUT SST * Sst,
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) { /* mcast & broadcast address */
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID_TO_REMOVE;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	} else { /* unicast address */
		pEntry = MacTableLookup(pAd, pAddr);

		if (pEntry) {
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;

			if (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
				&& (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		} else {
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID_TO_REMOVE;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT) {
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];

		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LOG#%ld "MACSTR" %s\n",
				 pAd->EventTab.Num, MAC2STR(pAddr), pEventText[Event]);
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN bNonGFExist = FALSE;
	ADD_HT_INFO_IE *addht = NULL;
	UCHAR band = 0;
	UINT32 new_protection = 0;
	UCHAR op_mode = NON_PROTECT;

	if (wdev == NULL)
		return;

	addht = wlan_operate_get_addht(wdev);
	band = HcGetBandByWdev(wdev);

	/* non HT BSS exist within 5 sec */
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32 &&
		pAd->Mlme.Now32 != 0) {
		op_mode = NONMEMBER_PROTECT;
		bNonGFExist = TRUE; /* non-HT means nonGF support */
		new_protection = SET_PROTECT(NON_MEMBER_PROTECT);
	}

	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((op_mode == NON_PROTECT) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(wlan_config_get_ht_bw(wdev) == HT_BW_40)) {
		op_mode = BW20_PROTECT;
		new_protection = SET_PROTECT(HT20_PROTECT);
	}

	if (pAd->MacTab.fAnyStationIsLegacy || pAd->MacTab.Size >= pAd->multi_cli_nums_eap_th) {
		op_mode = NONHT_MM_PROTECT;
		new_protection = SET_PROTECT(NON_HT_MIXMODE_PROTECT);
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "OperationMode: %d, bNonGFExist: %d\n",
			  addht->AddHtInfo2.OperaionMode, bNonGFExist);

	if ((op_mode != addht->AddHtInfo2.OperaionMode)
			|| (pAd->MacTab.fAnyStationNonGF[band] != addht->AddHtInfo2.NonGfPresent)) {
		addht->AddHtInfo2.OperaionMode = op_mode;
		addht->AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF[band];

		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	}

	if (bNonGFExist == FALSE)
		bNonGFExist = pAd->MacTab.fAnyStationNonGF[band];

	if (bNonGFExist)
		new_protection |= SET_PROTECT(GREEN_FIELD_PROTECT);

	if (nonerp_protection(&pAd->ApCfg.MBSSID[wdev->func_idx]))
		new_protection |= SET_PROTECT(ERP);

	/*if no protect should enable for CTS-2-Self, WHQA_00025629*/
	if (MTK_REV_LT(pAd, MT7615, MT7615E3))
		if (pAd->CommonCfg.dbdc_mode && op_mode == NON_PROTECT)	{
			new_protection |= SET_PROTECT(HT20_PROTECT);
		}

#if defined(MT_MAC)
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((new_protection & wdev->protection) != new_protection) {
			wdev->protection = new_protection;

			HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
		}

	}
#endif
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR Apidx)
{
	BOOLEAN Result = TRUE;
#ifdef ACL_BLK_COUNT_SUPPORT
	ULONG idx;
#endif
	if (!VALID_MBSS(pAd, Apidx))
		return FALSE;

	if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
		Result = TRUE;
	else {
		ULONG i;

		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
			Result = FALSE;
		else                                              /* ACL is a negative list */
			Result = TRUE;

		for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++) {
			if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr)) {
				Result = !Result;
				break;
			}
		}
	}
#ifdef ACL_BLK_COUNT_SUPPORT
		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy != 2) {
			for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++)
				(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) = 0;
		}
#endif

	if (Result == FALSE) {
#ifdef ACL_BLK_COUNT_SUPPORT
		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 2) {
			for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++) {
				if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Addr)) {
						(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) += 1;
					break;
				}
			}
		}
#endif/*ACL_BLK_COUNT_SUPPORT*/
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, MACSTR" failed in ACL checking\n",
				 MAC2STR(pAddr));
	}

	return Result;
}


/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	USHORT      Reason = REASON_DECLINED;
	MAC_TABLE_ENTRY *pEntry;
	BSS_STRUCT *pMbss;
	BOOLEAN drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));

	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApUpdateAccessControlList : Apidx = %d\n", Apidx);
	/* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];

	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, MacIdx); MacIdx++) {
		pEntry = &pAd->MacTab.Content[MacIdx];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->func_tb_idx != Apidx)
			continue;

		drop = FALSE;
		Matched = FALSE;

		for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++) {
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr)) {
				Matched = TRUE;
				break;
			}
		}

		if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1)) {
			drop = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA not on positive ACL. remove it...\n");
		} else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2)) {
			drop = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA on negative ACL. remove it...\n");
		}

		if (drop == TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Apidx = %d\n", Apidx);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
					 pMbss->AccessControlList.Policy);

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC) {
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " MlmeAllocateMemory fail  ..\n");
					return;
				}

				Reason = REASON_DECLINED;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n",
						 Reason, PRINT_MAC(pEntry->Addr));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
								 pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);
				RtmpusecDelay(5000);
			}
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_DECLINED);
#endif
#ifdef MAP_R2
			if (IS_ENTRY_CLIENT(pEntry) && IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
				wapp_handle_sta_disassoc(pAd, pEntry->wcid, Reason);
#endif

			mac_entry_delete(pAd, pEntry);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
											 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
											      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
				=>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
					is defined to be TRUE
				=>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
					is defined to be TRUE
				=>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
					is defined to be TRUE
*/
INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE * pCoexChRange,
	IN UCHAR Channel)
{
	INT index, cntrCh = 0;
	UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));
	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT

	if (WMODE_CAP_5G(wdev->PhyMode)) {
		/* For 5GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = pChCtrl->ChList[index].Channel;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			} else {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh - 4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
					  index,
					  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
					  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
					  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
					  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
					  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel);
			return TRUE;
		}

		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n", Channel);
	} else
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				if ((index + 4) < pChCtrl->ChListNum) {
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			} else {
				if ((index - 4) >= 0) {
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh) {
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd = cntrCh + 5;
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
						  index,
						  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
						  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
						  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel);
			}

			return TRUE;
		}

		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n",
				  index, pChCtrl->ChListNum, Channel);
	}

	return FALSE;
}

#define BSS_2040_COEX_SCAN_RESULT_VALID_TIME	(60 * OS_HZ)	/* 60 seconds */

VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN needFallBack = FALSE;
	INT chStartIdx, chEndIdx, index, curPriChIdx, curSecChIdx, ret = 0;
	BSS_COEX_CH_RANGE  coexChRange;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR Channel = wdev->channel;
	UCHAR ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_config_get_ext_cha(wdev);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	P_BSS_COEX_SCAN_LAST_RESULT pScanResult = &pAd->CommonCfg.BssCoexScanLastResult[BandIdx];
	ULONG Now32;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (!WMODE_CAP_2G(PhyMode))
		return;

	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(PhyMode)) ||
		(ht_bw == BW_20)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "The wdev->PhyMode=%d, BW=%d, didn't need channel adjustment!\n", PhyMode, ht_bw);
		return;
	}

	NdisGetSystemUpTime(&Now32);
	if (RTMP_TIME_BEFORE(Now32, pScanResult->LastScanTime + BSS_2040_COEX_SCAN_RESULT_VALID_TIME) &&
		(pScanResult->LastScanTime != 0)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"leverage result of wdev(%d@BN%d), FallBack=%d (remaining %ld ms)\n",
				pScanResult->WdevIdx, BandIdx, pScanResult->bNeedFallBack,
				(BSS_2040_COEX_SCAN_RESULT_VALID_TIME - (Now32 - pScanResult->LastScanTime)) * 1000 / OS_HZ);

		needFallBack = pScanResult->bNeedFallBack;
		goto coex_scan_result_apply;
	}

	/* Build the effected channel list, if something wrong, return directly. */
	/* For 2.4GHz band */
	for (index = 0; index < pChCtrl->ChListNum; index++) {
		if (pChCtrl->ChList[index].Channel == Channel)
			break;
	}

	/* Check ext_cha invalid */
	ht_ext_cha_adjust(pAd, Channel, &ht_bw, &ext_cha, wdev);

	if (index < pChCtrl->ChListNum) {
		if (ext_cha == EXTCHA_ABOVE) {
			curPriChIdx = index;
			curSecChIdx = ((index + 4) < pChCtrl->ChListNum) ? (index + 4) : (pChCtrl->ChListNum - 1);
			chStartIdx = (curPriChIdx >= 4) ? (curPriChIdx - 4) : 0;
			chEndIdx = ((curSecChIdx + 4) < pChCtrl->ChListNum) ? (curSecChIdx + 4) :
					   (pChCtrl->ChListNum - 1);
		} else {
			curPriChIdx = index;
			curSecChIdx = ((index - 4) >= 0) ? (index - 4) : 0;
			chStartIdx = (curSecChIdx >= 4) ? (curSecChIdx - 4) : 0;
			chEndIdx =  ((curPriChIdx + 4) < pChCtrl->ChListNum) ? (curPriChIdx + 4) :
						(pChCtrl->ChListNum - 1);
		}
	} else {
		/* It should not happened! */
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n", Channel);
		return;
	}

#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, GREENAP_REASON_AP_OVERLAPPING_SCAN);
#endif /* GREENAP_SUPPORT */

	GetBssCoexEffectedChRange(pAd, wdev, &coexChRange, Channel);

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pChCtrl->ChListNum; index++)
		pChCtrl->ChList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Card still not enable Tx/Rx, enable it now!\n");
		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
			 pChCtrl->ChList[chStartIdx].Channel, pChCtrl->ChList[chEndIdx].Channel);
	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;

	for (index = chStartIdx; index <= chEndIdx; index++) {
		if (arch_ops->archSetMacTxRx) {
			ret = arch_ops->archSetMacTxRx(pAd, ASIC_MAC_TXRX, FALSE, BandIdx);
			if (ret != 0) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Disable MacTxRx failed!\n");
			}
		}
		Channel = pChCtrl->ChList[index].Channel;
		wlan_operate_scan(wdev, Channel);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP OBSS SYNC - BBP R4 to 20MHz.l\n");
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		if (arch_ops->archSetMacTxRx) {
			ret = arch_ops->archSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE, BandIdx);
			if (ret != 0) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Enable MacTxRx failed!\n");
			}
		}
		OS_WAIT(300); /* wait for 300 ms at each channel. */
	}

	pAd->CommonCfg.bOverlapScanning = FALSE;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
				 index, pChCtrl->ChList[index].Channel, pChCtrl->ChList[index].bEffectedChannel);

		if ((pChCtrl->ChList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  &&
			(index != curPriChIdx)) {
			needFallBack = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "needFallBack=TRUE due to OP/OT!\n");
		}

		if ((pChCtrl->ChList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx)) {
			needFallBack = TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "needFallBack=TRUE due to OS!\n");
		}
	}

	/* check threshold of coex AP counts */
	needFallBack = (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr) ? needFallBack : FALSE;

	/* update last scan result */
	pScanResult->WdevIdx = wdev->wdev_idx;
	pScanResult->LastScanTime = Now32;
	pScanResult->bNeedFallBack = needFallBack;

coex_scan_result_apply:
	/* If need fallback, now do it. */
	if (needFallBack == TRUE) {
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		pAd->CommonCfg.need_fallback = 1;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
	} else {
		/*restore to original channel*/
		pAd->CommonCfg.need_fallback = 0;
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, GREENAP_REASON_AP_OVERLAPPING_SCAN);
#endif /* GREENAP_SUPPORT */
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#define TP_100M_LOWER_BOUND	60
#define TP_100M_UPPER_BOUND	120
#define DBDC_PEAK_TP_PER_THRESHOLD 5

static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad)
{
	EDCA_PARM *edcaparam;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	UINT16 multi_client_num_th = 0, ap_peak_tp_th = 0;
	UCHAR band_idx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct peak_tp_ctl *peak_tp_ctl = NULL;
	UINT16 level = cap->peak_txop;

#ifdef RX_COUNT_DETECT
	BOOLEAN limit_ampdu_flag = FALSE, fband_2g = FALSE;
#endif /* RX_COUNT_DETECT */

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {

		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "BandIdx:%d\n", band_idx);
		peak_tp_ctl = &ad->peak_tp_ctl[band_idx];

		if (peak_tp_ctl->client_nums == 0)
			goto ignore_ampdu_efficiency_check;

		wlan_ct = &ad->WlanCounters[band_idx];
		wdev = peak_tp_ctl->main_wdev;
		tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;

		if (WMODE_CAP_2G(wdev->PhyMode)) {
#ifdef RX_COUNT_DETECT
			fband_2g = TRUE;
#endif /* RX_COUNT_DETECT */
			multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
			if (ad->CommonCfg.dbdc_mode)
				ap_peak_tp_th = cap->ApDBDC2GPeakTpTH;
			else
				ap_peak_tp_th = cap->Ap2GPeakTpTH;
		} else if (WMODE_CAP_5G(wdev->PhyMode)) {
			multi_client_num_th = MULTI_CLIENT_NUMS_TH;
			if (ad->CommonCfg.dbdc_mode)
				ap_peak_tp_th = cap->ApDBDC5GPeakTpTH;
			else
				ap_peak_tp_th = cap->Ap5GPeakTpTH;
		} else {
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[%d]\n", __LINE__);
			continue;
		}

#ifdef RX_COUNT_DETECT
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%d] multi_client_nums = %d\n", __LINE__, ad->txop_ctl[band_idx].multi_client_nums);
		if ((ad->MacTab.Size >= ad->multi_cli_nums_eap_th) &&
			(ad->mcli_ctl[band_idx].large_rssi_gap_num > 0)) {
				struct peak_tp_ctl *peak_tp_ctl = NULL;
				peak_tp_ctl = &ad->peak_tp_ctl[band_idx];
				if (peak_tp_ctl->main_traffc_mode == TRAFFIC_UL_MODE)
					limit_ampdu_flag = TRUE;
			}
		/* limit ampdu only for old WRR scheduler */
		if (ad->vow_gen.VOW_GEN >= VOW_GEN_FALCON)
			limit_ampdu_flag = FALSE;
#endif /* RX_COUNT_DETECT*/

		if (ad->txop_ctl[band_idx].multi_client_nums >= multi_client_num_th) {
			/* do no apply patch, it is in veriwave multi-client case */
			goto ignore_ampdu_efficiency_check;
		}

		if (tx_cnt > 0)
			per = 100 * (wlan_ct->AmpduFailCount.u.LowPart) / (wlan_ct->AmpduFailCount.u.LowPart + tx_cnt);

		if (per >= DBDC_PEAK_TP_PER_THRESHOLD) {
			/* do no apply patch, it is in noise environment */
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "per=%lu\n", per);
			goto ignore_ampdu_efficiency_check;
		}
#ifdef KERNEL_RPS_ADJUST
		if ((ad->mcli_ctl[band_idx].pkt_avg_len < 1280)
			&& (peak_tp_ctl->cli_peak_tp_running == 0)) {
			/* do no apply patch, it is not long packet case */
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"[%d]pktlen=%u\n", __LINE__,
				ad->mcli_ctl[band_idx].pkt_avg_len);
			goto ignore_ampdu_efficiency_check;
		}
#endif
		/* scenario detection, peak TP TH is pre setting on each CHIP's cap */
		if (ap_peak_tp_th != 0 &&
			(peak_tp_ctl->main_tx_tp > ap_peak_tp_th) &&
			peak_tp_ctl->main_traffc_mode == TRAFFIC_DL_MODE) {
			peak_tp_ctl->cli_peak_tp_running = 1;
			level = TXOP_FE;

			if (level != peak_tp_ctl->cli_peak_tp_txop_level)
				peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
		} else if (peak_tp_ctl->main_traffc_mode == TRAFFIC_DL_MODE &&
			(peak_tp_ctl->main_tx_tp > TP_100M_LOWER_BOUND) &&
			(peak_tp_ctl->main_tx_tp < TP_100M_UPPER_BOUND)) {
			peak_tp_ctl->cli_peak_tp_running = 1;
			level = TXOP_60;

			if (level != peak_tp_ctl->cli_peak_tp_txop_level)
				peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
		} else
			peak_tp_ctl->cli_peak_tp_running = 0;

		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%d]per=%lu, tx=%d M, (%d, %d, %d, %d)\n\r",
				 __LINE__, per, ad->peak_tp_ctl[band_idx].main_tx_tp,
				 peak_tp_ctl->cli_peak_tp_running,
				 peak_tp_ctl->cli_ampdu_efficiency_running,
				 peak_tp_ctl->cli_peak_tp_txop_enable,
				 peak_tp_ctl->main_traffc_mode);
		/* increase ampdu efficiency if running peak T.P */
		if (peak_tp_ctl->cli_peak_tp_running) {
			if (!peak_tp_ctl->cli_ampdu_efficiency_running) {
				if (peak_tp_ctl->main_entry->vendor_ie.is_mtk == FALSE && query_tx_burst_prio(ad, wdev) <= PRIO_PEAK_TP) {
					AsicAmpduEfficiencyAdjust(wdev, 0xf);
					peak_tp_ctl->cli_ampdu_efficiency_running = TRUE;
				}
			}

			if (!peak_tp_ctl->cli_peak_tp_txop_enable) {
				enable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
				peak_tp_ctl->cli_peak_tp_txop_level = level;
				peak_tp_ctl->cli_peak_tp_txop_enable = TRUE;
			}
		} else {
			/* restore to original */
			if (peak_tp_ctl->cli_ampdu_efficiency_running) {
				edcaparam = HcGetEdca(ad, wdev);

				if (edcaparam)
					AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

				peak_tp_ctl->cli_ampdu_efficiency_running = FALSE;
			}

			if (peak_tp_ctl->cli_peak_tp_txop_enable) {
				disable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
				peak_tp_ctl->cli_peak_tp_txop_enable = FALSE;
			}
		}

ignore_ampdu_efficiency_check:

		/* restore aifs adjust since dynamic txop owner is not peak throughput */
		if (peak_tp_ctl->cli_ampdu_efficiency_running) {
			if (query_tx_burst_prio(ad, wdev) > PRIO_PEAK_TP) {
				edcaparam = HcGetEdca(ad, wdev);

				if (edcaparam)
					AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

				peak_tp_ctl->cli_ampdu_efficiency_running = FALSE;
			}
		}

#ifdef RX_COUNT_DETECT
		if (ad->txop_ctl[band_idx].limit_ampdu_size_running != limit_ampdu_flag) {
			/* TODO : use in-band cmd for per wtbl or per AC */
			if (ad->txop_ctl[band_idx].limit_ampdu_size_running == FALSE) {
				/* limit ampdu count to 0xa */
				ad->txop_ctl[band_idx].limit_ampdu_size_running = TRUE;
				if (fband_2g)
					MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR0, 0x0a00);
				else
					MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR1, 0x0a00);
			} else {
				/* restore to unlimit*/
				ad->txop_ctl[band_idx].limit_ampdu_size_running = FALSE;

				if (fband_2g)
					MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR0, 0x0);
				else
					MAC_IO_WRITE32(ad->hdev_ctrl, AGG_AALCR1, 0x0);
			}
		}
#endif /* RX_COUNT_DETECT */

		/* clear some record */
		peak_tp_ctl->client_nums = 0;
		peak_tp_ctl->main_tx_tp = 0;
		peak_tp_ctl->main_rx_tp = 0;
		peak_tp_ctl->main_txrx_bytes = 0;
	}
}

#ifdef ANTENNA_DIVERSITY_SUPPORT
VOID ant_diversity_periodic_exec(RTMP_ADAPTER *pAd)
{
	ant_diversity_update_indicator(pAd);
	if (ant_diversity_allowed(pAd, DBDC_BAND1) == TRUE)
		ant_diversity_process(pAd, DBDC_BAND1);
	if (ant_diversity_allowed(pAd, DBDC_BAND0) == TRUE)
		ant_diversity_process(pAd, DBDC_BAND0);
}

VOID ant_diversity_gpio_control(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UINT8 ant)
{
/*
			|		Band0			|		band1
			|main ant	|aux ant	|main ant	|aux ant
	GPIO36	|x			|x			|0			|1
	GPIO37	|x			|x			|1			|0
	GPIO38	|0			|1			|x			|x
	GPIO39	|1			|0			|x			|x
*/
	UINT32 Value = 0;

	if (band_idx == DBDC_BAND0) {
		/*GPIO38, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT31 | BIT30 | BIT29 | BIT28);
		Value |= 0x9 << 28;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT6);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		/*GPIO39, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x7000506C, &Value);
		Value &= ~(BIT3 | BIT2 | BIT1 | BIT0);
		Value |= 0x9;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x7000506C, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT7);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		if (ant == ANT_AUX) {
			/*GPIO38 set to 0, GPIO39 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT6 | BIT7);
			Value |= 0x2 << 6;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		} else {
			/*GPIO39 set to 0, GPIO38 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT6 | BIT7);
			Value |= 0x1 << 6;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		}
	}

	if (band_idx == DBDC_BAND1) {
		/*GPIO36, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT23 | BIT22 | BIT21 | BIT20);
		Value |= 0x9 << 20;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT4);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		/*GPIO37, set pinmux to GPIO*/
		RTMP_IO_READ32(ad->hdev_ctrl, 0x70005068, &Value);
		Value &= ~(BIT27 | BIT26 | BIT25 | BIT24);
		Value |= 0x9 << 24;
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70005068, Value);

		RTMP_IO_READ32(ad->hdev_ctrl, 0x700040C0, &Value);
		Value |= (BIT5);
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040C0, Value);

		if (ant == ANT_AUX) {
			/*GPIO36 set to 0, GPIO37 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT4 | BIT5);
			Value |= 0x2 << 4;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		} else {
			/*GPIO37 set to 0, GPIO36 set to 1*/
			RTMP_IO_READ32(ad->hdev_ctrl, 0x700040B0, &Value);
			Value &= ~(BIT4 | BIT5);
			Value |= 0x1 << 4;
			RTMP_IO_WRITE32(ad->hdev_ctrl, 0x700040B0, Value);
		}
	}
}

VOID ant_diversity_restore_rxv_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		value = pAd->diversity_ctl[DBDC_BAND0].rxv_cr_value;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820e3070, value);
	}
	else {
		value = pAd->diversity_ctl[DBDC_BAND1].rxv_cr_value;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3070, value);
	}
}

VOID ant_diversity_backup_and_enable_rxv_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 value = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x820e3070, &value);
		pAd->diversity_ctl[DBDC_BAND0].rxv_cr_value = value;/* backup CR*/
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820e3070, 0x91);/* enable CR*/
	}
	else {
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x820f3070, &value);
		pAd->diversity_ctl[DBDC_BAND1].rxv_cr_value = value;/* backup CR*/
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3070, 0x91);/* enable CR*/
	}
}

VOID diversity_switch_antenna(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 ant)
{
	UINT8 target_ant = ant;

	if (target_ant == ANT_INIT)
		target_ant = ANT_MAIN;

	if (target_ant > ANT_AUX) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ant number(%d)\n", target_ant);
		return;
	}

	if (pAd->diversity_ctl[band_idx].cur_ant != target_ant) {
		pAd->diversity_ctl[band_idx].cur_ant = target_ant;
		ant_diversity_gpio_control(pAd, band_idx, target_ant);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "switch to ant(%d), band_idx(%d)\n", target_ant, band_idx);
	}
}

VOID ant_diversity_ctrl_reset(RTMP_ADAPTER *pAd)
{
	UCHAR band_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---Enter---\n");
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pAd->diversity_ctl[band_idx].diversity_force_disable = 0;
		pAd->diversity_ctl[band_idx].dbg_cn_deg_cnt_th = 0;
		pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_cnt_th = 0;
		pAd->diversity_ctl[band_idx].dbg_tp_deg_th = 0;
		pAd->diversity_ctl[band_idx].dbg_cn_deg_th_max = 0;
		pAd->diversity_ctl[band_idx].dbg_cn_deg_th_min = 0;
		pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_th = 0;
		pAd->diversity_ctl[band_idx].dbg_countdown = 0;
		pAd->diversity_ctl[band_idx].dbg_ul_tp_th = 0;
		pAd->diversity_ctl[band_idx].dbg_ul_rate_delta_th = 0;
		pAd->diversity_ctl[band_idx].dbg_flag_lvl1 = 0;
		pAd->diversity_ctl[band_idx].dbg_flag_lvl2 = 0;
		pAd->diversity_ctl[band_idx].rxv_cr_value = 0x1;
	}
	pAd->cn_rate_read_interval = 100;
}

VOID ant_diversity_ctrl_init(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 ul_mode_th = 0;
	UINT8 countdown = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---Enter---\n");

	if (band_idx == DBDC_BAND0) {
		ul_mode_th = UL_MODE_TH_2G;
		countdown = ANT_COUNTDOWN_2G;
	} else {
		ul_mode_th = UL_MODE_TH_5G;
		countdown = ANT_COUNTDOWN_5G;
	}

	if (pAd->diversity_ctl[band_idx].dbg_countdown != 0)
		countdown = pAd->diversity_ctl[band_idx].dbg_countdown;
	if (pAd->diversity_ctl[band_idx].dbg_ul_tp_th != 0)
		ul_mode_th = pAd->diversity_ctl[band_idx].dbg_ul_tp_th;

	pAd->diversity_ctl[band_idx].ul_mode_th = ul_mode_th;
	pAd->diversity_ctl[band_idx].cur_ant = ANT_INIT;
	pAd->diversity_ctl[band_idx].cur_traffic_mode = TRAFFIC_0;
	pAd->diversity_ctl[band_idx].last_traffic_mode = TRAFFIC_0;
	pAd->diversity_ctl[band_idx].cur_rx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].cur_rx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].cur_tx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].cur_tx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].last_rx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].last_rx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].last_tx_tput[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].last_tx_tput[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].last_rx_cn[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].last_rx_cn[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].last_rx_rate[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].last_rx_rate[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_MAIN] = 0;
	pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_AUX] = 0;
	pAd->diversity_ctl[band_idx].cn_deg_cnt = 0;
	pAd->diversity_ctl[band_idx].rx_delta_cn = 0;
	pAd->diversity_ctl[band_idx].ap_nss = 2;
	pAd->diversity_ctl[band_idx].sta_nss = 2;
	pAd->diversity_ctl[band_idx].sta_rssi = -127;
	pAd->diversity_ctl[band_idx].client_num = 0;
	pAd->diversity_ctl[band_idx].is_he_sta = FALSE;
	pAd->diversity_ctl[band_idx].ant_switch_countdown = countdown;
	pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_IDLE;
	pAd->rec_start_cn_flag[band_idx] = FALSE;
	pAd->rec_start_rx_rate_flag[band_idx] = FALSE;
	pAd->cn_average[band_idx] = ANT_DIVERSITY_CN_INIT_VAL;
	pAd->phy_rate_average[band_idx] = ANT_DIVERSITY_RX_RATE_INIT_VAL;
	pAd->rate_rec_sum[band_idx] = 0;
	pAd->rate_rec_num[band_idx] = 0;
	pAd->cn_rec_sum[band_idx] = 0;
	pAd->cn_rec_num[band_idx] = 0;

	diversity_switch_antenna(pAd, band_idx, ANT_INIT);
}

UINT8 ant_diversity_get_sta_nss(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT8 sta_nss = 1;

	if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00))
			sta_nss = 2;

	return sta_nss;
}

VOID show_ant_diversity_debug_log(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	("---mode(%d,%d),Ant(%d),CnAvg(%d),CurRxTp(%d,%d),LastRxTp(%d,%d),CurCN(%d,%d),LastCN(%d,%d),DeltaCN(%d),CurRate(%d,%d),LastRate(%d,%d),Status(%d),(%d,%d,%d,%d,%d)---\n",
	band_idx,
	pAd->diversity_ctl[band_idx].cur_traffic_mode,
	pAd->diversity_ctl[band_idx].cur_ant,
	pAd->cn_average[band_idx],
	pAd->diversity_ctl[band_idx].cur_rx_tput[ANT_MAIN],
	pAd->diversity_ctl[band_idx].cur_rx_tput[ANT_AUX],
	pAd->diversity_ctl[band_idx].last_rx_tput[ANT_MAIN],
	pAd->diversity_ctl[band_idx].last_rx_tput[ANT_AUX],
	pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_MAIN],
	pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_AUX],
	pAd->diversity_ctl[band_idx].last_rx_cn[ANT_MAIN],
	pAd->diversity_ctl[band_idx].last_rx_cn[ANT_AUX],
	pAd->diversity_ctl[band_idx].rx_delta_cn,
	pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_MAIN],
	pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_AUX],
	pAd->diversity_ctl[band_idx].last_rx_rate[ANT_MAIN],
	pAd->diversity_ctl[band_idx].last_rx_rate[ANT_AUX],
	pAd->diversity_ctl[band_idx].diversity_status,
	pAd->diversity_ctl[band_idx].ap_nss,
	pAd->diversity_ctl[band_idx].sta_nss,
	pAd->diversity_ctl[band_idx].client_num,
	pAd->diversity_ctl[band_idx].is_he_sta,
	pAd->diversity_ctl[band_idx].sta_rssi);
}

UINT8 ant_diversity_update_indicator(RTMP_ADAPTER *pAd)
{
	CHAR avg_rssi = -127;
	UINT8 cur_ant = ANT_MAIN;
	UINT8 traffc_mode = 0;
	UINT8 band_idx = 0;
	UINT16 client_2g = 0;
	UINT16 client_5g = 0;
	UINT16 wcid = 0;
	UINT32 tx_tp = 0;
	UINT32 rx_tp = 0;
	ULONG rx_bytes = 0;
	ULONG tx_bytes = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE *pMacTable = &pAd->MacTab;

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pMacTable->Content[wcid];
		if ((!pEntry) || (!(IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))))
			continue;

		if (pEntry->wdev) {
			rx_bytes = pEntry->ant_div_rx_bytes;
			rx_tp = (rx_bytes >> 16);/*per 500ms */
			tx_bytes = pEntry->ant_div_tx_bytes;
			tx_tp = (tx_bytes >> 16);/*per 500ms */

			if ((tx_tp + rx_tp) == 0)
				traffc_mode = TRAFFIC_0;
			else if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_DL_MODE;
			else if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_UL_MODE;
			else
				traffc_mode = TRAFFIC_0;

			band_idx = HcGetBandByWdev(pEntry->wdev);
			cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
			pAd->diversity_ctl[band_idx].cur_traffic_mode = traffc_mode;
			pAd->diversity_ctl[band_idx].cur_rx_tput[cur_ant] = rx_tp;
			pAd->diversity_ctl[band_idx].cur_tx_tput[cur_ant] = tx_tp;
			pAd->diversity_ctl[band_idx].ap_nss = wlan_config_get_tx_stream(pEntry->wdev);
			pAd->diversity_ctl[band_idx].sta_nss = ant_diversity_get_sta_nss(pAd, pEntry);
			if ((pEntry->MaxHTPhyMode.field.MODE >= MODE_HE) && (pEntry->MaxHTPhyMode.field.MODE != MODE_UNKNOWN))
				pAd->diversity_ctl[band_idx].is_he_sta = TRUE;
			else
				pAd->diversity_ctl[band_idx].is_he_sta = FALSE;
			if (WMODE_CAP_2G(pEntry->wdev->PhyMode))
				client_2g++;
			if (WMODE_CAP_5G(pEntry->wdev->PhyMode))
				client_5g++;
			avg_rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
			pAd->diversity_ctl[band_idx].sta_rssi = avg_rssi;
			if (pAd->diversity_ctl[band_idx].dbg_flag_lvl2 != 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"mode(%d), tx_tp(%d), rx_tp(%d), client(%d,%d), avgRssi(%d), ap_nss(%d), sta_nss(%d), he_sta(%d)\n",
				traffc_mode, tx_tp, rx_tp, client_2g, client_5g, avg_rssi, pAd->diversity_ctl[band_idx].ap_nss,
				pAd->diversity_ctl[band_idx].sta_nss, pAd->diversity_ctl[band_idx].is_he_sta);
			}
			pEntry->ant_div_rx_bytes = 0;
			pEntry->ant_div_tx_bytes = 0;
		}
	}
	pAd->diversity_ctl[DBDC_BAND0].client_num = client_2g;
	pAd->diversity_ctl[DBDC_BAND1].client_num = client_5g;
	return 0;
}

UINT8 ant_diversity_allowed(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	BOOLEAN allowed = FALSE;
	BOOLEAN is_cn_ready = FALSE;
	BOOLEAN is_rx_rate_ready = FALSE;
	UINT8 cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
	UINT8 countdown = 0;
	UINT32 tp_th_ul = 0;
	UINT32 tp_th_dl = 0;
	UINT32 cur_rx_tp = pAd->diversity_ctl[band_idx].cur_rx_tput[cur_ant];
	CHAR rssi_th = -127;
	UINT8 cur_mode = pAd->diversity_ctl[band_idx].cur_traffic_mode;
	UINT8 last_mode = pAd->diversity_ctl[band_idx].last_traffic_mode;
	UINT32 cur_cn = pAd->cn_average[band_idx];
	UINT32 cur_rx_rate = pAd->phy_rate_average[band_idx];
	UINT8 cur_status = pAd->diversity_ctl[band_idx].diversity_status;

	if (pAd->diversity_ctl[band_idx].diversity_force_disable != 0)
		return FALSE;

	if ((cur_mode == TRAFFIC_UL_MODE) && (cur_status != ANT_DIVERSITY_STATUS_IDLE)) {
		if (cur_cn != ANT_DIVERSITY_CN_INIT_VAL)
			is_cn_ready = TRUE;
		if (cur_rx_rate != ANT_DIVERSITY_RX_RATE_INIT_VAL)
			is_rx_rate_ready = TRUE;

		if ((is_cn_ready == TRUE) && (is_rx_rate_ready == TRUE)) {
			if (pAd->diversity_ctl[band_idx].dbg_flag_lvl1 != 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"band(%d) AvgCN(%d,%d,%d), AvgRxRate(%d,%d,%d)\n", band_idx,
						cur_cn, pAd->cn_rec_sum[band_idx], pAd->cn_rec_num[band_idx],
						cur_rx_rate, pAd->rate_rec_sum[band_idx], pAd->rate_rec_num[band_idx]);
			}
			pAd->diversity_ctl[band_idx].cur_rx_cn[cur_ant] = cur_cn;
			pAd->cn_rec_sum[band_idx] = 0;
			pAd->cn_rec_num[band_idx] = 0;
			pAd->cn_average[band_idx] = ANT_DIVERSITY_CN_INIT_VAL;
			pAd->diversity_ctl[band_idx].cur_rx_rate[cur_ant] = cur_rx_rate;
			pAd->rate_rec_sum[band_idx] = 0;
			pAd->rate_rec_num[band_idx] = 0;
			pAd->phy_rate_average[band_idx] = ANT_DIVERSITY_RX_RATE_INIT_VAL;
			pAd->rec_start_cn_flag[band_idx] = TRUE;
			pAd->rec_start_rx_rate_flag[band_idx] = TRUE;
		} else {
			if (pAd->diversity_ctl[band_idx].dbg_flag_lvl1 != 0) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"band(%d) AvgCN is %s ready(%d,%d), AvgRxRate is %s ready(%d,%d)!!!\n", band_idx,
					(is_cn_ready == FALSE)?"not":"", pAd->cn_rec_sum[band_idx], pAd->cn_rec_num[band_idx],
					(is_rx_rate_ready == FALSE)?"not":"", pAd->rate_rec_sum[band_idx], pAd->rate_rec_num[band_idx]);
			}
			return FALSE;
		}
	}

	if (band_idx == DBDC_BAND0) {
		tp_th_ul = UL_MODE_TH_2G;
		tp_th_dl = DL_MODE_TH_2G;
		countdown = ANT_COUNTDOWN_2G;
		rssi_th = ANT_DIVERSITY_RSSI_TH_2G;
	} else if (band_idx == DBDC_BAND1) {
		tp_th_ul= UL_MODE_TH_5G;
		tp_th_dl = DL_MODE_TH_5G;
		countdown = ANT_COUNTDOWN_5G;
		rssi_th = ANT_DIVERSITY_RSSI_TH_5G;
	}

	if (pAd->diversity_ctl[band_idx].dbg_countdown != 0)
		countdown = pAd->diversity_ctl[band_idx].dbg_countdown;
	if (pAd->diversity_ctl[band_idx].dbg_ul_tp_th != 0)
		tp_th_ul = pAd->diversity_ctl[band_idx].dbg_ul_tp_th;
	if (pAd->diversity_ctl[band_idx].dbg_flag_lvl1 != 0)
		show_ant_diversity_debug_log(pAd, band_idx);

	if ((cur_mode != last_mode) && (last_mode!= TRAFFIC_0)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"band(%d) condition from %d to %d!\n",
			band_idx, last_mode, cur_mode);
		ant_diversity_ctrl_init(pAd, band_idx);
		ant_diversity_restore_rxv_cr(pAd, band_idx);
		pAd->diversity_ctl[band_idx].last_traffic_mode = cur_mode;
		return FALSE;
	}

	if ((pAd->diversity_ctl[band_idx].client_num == 1)
		&& (pAd->diversity_ctl[band_idx].sta_rssi > rssi_th)
		&& (pAd->diversity_ctl[band_idx].ap_nss > 1)
		&& (pAd->diversity_ctl[band_idx].sta_nss > 1)
		&& (pAd->diversity_ctl[band_idx].is_he_sta)
		&& (((cur_mode == TRAFFIC_UL_MODE) && (cur_rx_tp > tp_th_ul))
			|| ((cur_mode == TRAFFIC_DL_MODE) && (pAd->diversity_ctl[band_idx].cur_tx_tput[cur_ant] > tp_th_dl)))) {
		if (cur_status == ANT_DIVERSITY_STATUS_IDLE) {
			pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_STREAM_START;
			pAd->diversity_ctl[band_idx].ant_switch_countdown = countdown;
			if (cur_mode == TRAFFIC_UL_MODE) {
				ant_diversity_backup_and_enable_rxv_cr(pAd, band_idx);
				pAd->rec_start_cn_flag[band_idx] = TRUE;
				pAd->rec_start_rx_rate_flag[band_idx] = TRUE;
			}
		}
		return TRUE;
	} else {
		/* reset indicators when conditions are not met */
		if (cur_status != ANT_DIVERSITY_STATUS_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d) condition mismatch, not allowed!\n", band_idx);
			ant_diversity_ctrl_init(pAd, band_idx);
			ant_diversity_restore_rxv_cr(pAd, band_idx);
			allowed = FALSE;
		}
	}

	return allowed;
}


VOID diversity_sel_ant_by_higher_tp(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 tp_main = pAd->diversity_ctl[band_idx].cur_tx_tput[ANT_MAIN];
	UINT32 tp_aux = pAd->diversity_ctl[band_idx].cur_tx_tput[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
	UINT8 sel_ant = 0;
	UINT8 cur_mode = pAd->diversity_ctl[band_idx].cur_traffic_mode;

	if (cur_mode != TRAFFIC_DL_MODE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong traffic mode(%d)! Keep cur_ant(%d)\n",
			cur_mode, cur_ant);
		return;
	}

	if (tp_main > tp_aux)
		sel_ant = ANT_MAIN;
	else if (tp_main < tp_aux)
		sel_ant = ANT_AUX;
	else
		sel_ant = cur_ant;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"tp_main(%dM), tp_aux(%dM), cur_ant(%d), select ant(%d)\n",
		tp_main, tp_aux, cur_ant, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
}

VOID diversity_sel_ant_by_better_cn(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 cn_main = pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_MAIN];
	UINT32 cn_aux = pAd->diversity_ctl[band_idx].cur_rx_cn[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
	UINT8 sel_ant = 0;
	UINT32 delta_cn = 0;

	if (cn_aux == cn_main) {
		pAd->diversity_ctl[band_idx].rx_delta_cn = 0;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"CN main & aux are the same value(%d, %d), keep current ant(%d)\n",
		cn_main, cn_aux, cur_ant);
		return;
	}

	if (cn_aux > cn_main) {
		delta_cn = cn_aux - cn_main;
		sel_ant = ANT_MAIN;
	}
	else {
		delta_cn = cn_main - cn_aux;
		sel_ant = ANT_AUX;
	}

	pAd->diversity_ctl[band_idx].rx_delta_cn = delta_cn;
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"cn_main(%d), cn_aux(%d), delta_cn(%d), cur_ant(%d), select ant(%d)\n",
		cn_main, cn_aux, delta_cn, cur_ant, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
}

BOOLEAN diversity_sel_ant_by_better_rx_rate(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT32 rate_main = pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_MAIN];
	UINT32 rate_aux = pAd->diversity_ctl[band_idx].cur_rx_rate[ANT_AUX];
	UINT8 cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
	UINT8 sel_ant = 0;
	UINT32 delta_rate_th = 0;

	if (band_idx == DBDC_BAND0)
		delta_rate_th = RX_RATE_DELTA_TH_2G;
	else
		delta_rate_th = RX_RATE_DELTA_TH_5G;

	if (pAd->diversity_ctl[band_idx].dbg_ul_rate_delta_th != 0) //dbg
		delta_rate_th = pAd->diversity_ctl[band_idx].dbg_ul_rate_delta_th;

	if ((rate_main > rate_aux) && ((rate_main - rate_aux) > delta_rate_th))
		sel_ant = ANT_MAIN;
	else if ((rate_aux > rate_main) && ((rate_aux - rate_main) > delta_rate_th))
		sel_ant = ANT_AUX;
	else {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"band(%d), Rx rate is similar:main(%d,%d),delta_th(%d),select by CN alg!\n",
		band_idx, rate_main, rate_aux, delta_rate_th);
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band(%d), rate_main(%d), rate_aux(%d), delta_th(%d), select ant(%d)\n",
		band_idx, rate_main, rate_aux, delta_rate_th, sel_ant);

	if (sel_ant != cur_ant)
		diversity_switch_antenna(pAd, band_idx, sel_ant);
	return TRUE;
}

BOOLEAN ant_diversity_tp_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	BOOLEAN tp_degrade = FALSE;
	UINT32 last_tp;
	UINT8 cur_mode = pAd->diversity_ctl[band_idx].cur_traffic_mode;
	UINT32 cur_tp = 0;
	UINT8 degrade_th = 0;

	if (cur_mode != TRAFFIC_DL_MODE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Band(%d), Error!!! Cur_mode is incorrect!!!(%d)\n",
			band_idx, cur_mode);
		return tp_degrade;

		cur_tp = pAd->diversity_ctl[band_idx].cur_tx_tput[cur_ant];
		last_tp = pAd->diversity_ctl[band_idx].last_tx_tput[cur_ant];
		if (band_idx == DBDC_BAND0)
			degrade_th = DL_DEGRADE_TH_2G;
		else
			degrade_th = DL_DEGRADE_TH_5G;
	}

	if (pAd->diversity_ctl[band_idx].dbg_tp_deg_th != 0)
		degrade_th = pAd->diversity_ctl[band_idx].dbg_tp_deg_th;

	if ((cur_tp * 100) < (last_tp * degrade_th)) {
		tp_degrade = TRUE;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"cur_ant(%d), band_idx(%d), last_tp(%dM), cur_tp(%dM), tp_degrade(%d)\n",
			cur_ant, band_idx, last_tp, cur_tp, tp_degrade);
	}
	return tp_degrade;
}

UINT8 ant_diversity_cn_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	UINT8 cn_degrade = ANT_DIVERSITY_CN_DEGRADE_FALSE;
	UINT32 last_cn = 0;
	UINT32 cur_cn = 0;
	UINT32 delta_cn = 0;
	UINT8 delta_cn_max_th = 0;
	UINT8 delta_cn_min_th = 0;
	UINT32 peer_ant_cn = 0;
	UINT8 cn_deg_cnt = pAd->diversity_ctl[band_idx].cn_deg_cnt;
	UINT8 cn_deg_cnt_th = 0;

	cur_cn = pAd->diversity_ctl[band_idx].cur_rx_cn[cur_ant];
	last_cn = pAd->diversity_ctl[band_idx].last_rx_cn[cur_ant];

	if (band_idx == DBDC_BAND0) {
		delta_cn_max_th = UL_CN_DEGRADE_TH_MAX_2G;
		delta_cn_min_th = UL_CN_DEGRADE_TH_MIN_2G;
		cn_deg_cnt_th = ANT_DIVERSITY_CN_DEGRADE_CNT_TH_2G;
	}
	else {
		delta_cn_max_th = UL_CN_DEGRADE_TH_MAX_5G;
		delta_cn_min_th = UL_CN_DEGRADE_TH_MIN_5G;
		cn_deg_cnt_th = ANT_DIVERSITY_CN_DEGRADE_CNT_TH_5G;
	}

	if (pAd->diversity_ctl[band_idx].dbg_cn_deg_th_max != 0) //dbg
		delta_cn_max_th = pAd->diversity_ctl[band_idx].dbg_cn_deg_th_max;
	if (pAd->diversity_ctl[band_idx].dbg_cn_deg_th_min != 0)//dbg
		delta_cn_min_th = pAd->diversity_ctl[band_idx].dbg_cn_deg_th_min;
	if (pAd->diversity_ctl[band_idx].dbg_cn_deg_cnt_th != 0)//dbg
		cn_deg_cnt_th = pAd->diversity_ctl[band_idx].dbg_cn_deg_cnt_th;

	if (cur_cn > last_cn) {
		delta_cn = pAd->diversity_ctl[band_idx].rx_delta_cn;
		if (cur_ant == ANT_MAIN)
			peer_ant_cn = pAd->diversity_ctl[band_idx].last_rx_cn[ANT_AUX];
		else
			peer_ant_cn = pAd->diversity_ctl[band_idx].last_rx_cn[ANT_MAIN];

		if ((cur_cn - last_cn) >= delta_cn_max_th) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[Case 1]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, cn_degrade);
		} else if ((delta_cn > delta_cn_min_th) && (cur_cn > peer_ant_cn)) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[Case 2]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), delta_cn(%d), peer_ant_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, delta_cn, peer_ant_cn, cn_degrade);
		} else if ((delta_cn <= delta_cn_min_th) && (cur_cn >= (peer_ant_cn + delta_cn_min_th))) {
			cn_deg_cnt++;
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[Case 3]--> cur_ant(%d), band_idx(%d), last_cn(%d), cur_cn(%d), delta_cn(%d), peer_ant_cn(%d), cn_degrade(%d)\n",
			cur_ant, band_idx, last_cn, cur_cn, delta_cn, peer_ant_cn, cn_degrade);
		}
	}

	if (cn_degrade == ANT_DIVERSITY_CN_DEGRADE_TRUE) {
		if (cn_deg_cnt < cn_deg_cnt_th) {
			cn_degrade = ANT_DIVERSITY_CN_DEGRADE_WAITING;
			pAd->diversity_ctl[band_idx].cn_deg_cnt++;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band_idx(%d), cn_deg_cnt(%d) < cn_deg_cnt_th(%d), keep current ant(%d)!\n",
			band_idx, cn_deg_cnt, cn_deg_cnt_th, cur_ant);
		} else {
			pAd->diversity_ctl[band_idx].cn_deg_cnt = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band_idx(%d), CN degrade continuously for %d times at ant(%d)!\n",
			band_idx, cn_deg_cnt, cur_ant);
		}
	} else
		pAd->diversity_ctl[band_idx].cn_deg_cnt = 0;

	return cn_degrade;
}

UINT8 ant_diversity_rx_rate_degrade(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 cur_ant)
{
	UINT32 rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_FALSE;
	UINT32 last_rx_rate = 0;
	UINT32 cur_rx_rate = 0;
	UINT32 delta_rate = 0;
	UINT32 delta_rate_th = 0;
	UINT32 peer_ant_rx_rate = 0;
	UINT8 rx_rate_deg_cnt = pAd->diversity_ctl[band_idx].rx_rate_deg_cnt;
	UINT8 rx_rate_deg_cnt_th = 0;

	cur_rx_rate = pAd->diversity_ctl[band_idx].cur_rx_rate[cur_ant];
	last_rx_rate = pAd->diversity_ctl[band_idx].last_rx_rate[cur_ant];

	if (band_idx == DBDC_BAND0) {
		delta_rate_th = UL_RX_RATE_DEGRADE_TH_2G;
		rx_rate_deg_cnt_th = ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_2G;
	}
	else {
		delta_rate_th = UL_RX_RATE_DEGRADE_TH_5G;
		rx_rate_deg_cnt_th = ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_5G;
	}

	if (pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_th != 0) //dbg
		delta_rate_th = pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_th;
	if (pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_cnt_th != 0)//dbg
		rx_rate_deg_cnt_th = pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_cnt_th;

	if (cur_rx_rate < last_rx_rate) {
		delta_rate = last_rx_rate - cur_rx_rate;
		if (cur_ant == ANT_MAIN)
			peer_ant_rx_rate = pAd->diversity_ctl[band_idx].last_rx_rate[ANT_AUX];
		else
			peer_ant_rx_rate = pAd->diversity_ctl[band_idx].last_rx_rate[ANT_MAIN];

		if ((delta_rate >= delta_rate_th) && (cur_rx_rate < peer_ant_rx_rate)) {
			rx_rate_deg_cnt++;
			rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[Case 1]--> cur_ant(%d), band_idx(%d), last_rx_rate(%d), cur_rx_rate(%d), peer_ant_rx_rate(%d)\n",
			cur_ant, band_idx, last_rx_rate, cur_rx_rate, peer_ant_rx_rate);
		}
	}

	if (rx_rate_degrade == ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE) {
		if (rx_rate_deg_cnt < rx_rate_deg_cnt_th) {
			rx_rate_degrade = ANT_DIVERSITY_RX_RATE_DEGRADE_WAITING;
			pAd->diversity_ctl[band_idx].rx_rate_deg_cnt++;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band_idx(%d), rx_rate_deg_cnt(%d) < rx_rate_deg_cnt_th(%d), keep current ant(%d)!\n",
			band_idx, rx_rate_deg_cnt, rx_rate_deg_cnt_th, cur_ant);
		} else {
			pAd->diversity_ctl[band_idx].rx_rate_deg_cnt = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band_idx(%d), CN degrade continuously for %d times at ant(%d)!Need switch to another ant!\n",
			band_idx, rx_rate_deg_cnt, cur_ant);
		}
	} else
		pAd->diversity_ctl[band_idx].rx_rate_deg_cnt = 0;

	return rx_rate_degrade;
}

VOID ant_diversity_process(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	UINT8 cur_ant = pAd->diversity_ctl[band_idx].cur_ant;
	UINT32 cur_tp_ul = pAd->diversity_ctl[band_idx].cur_rx_tput[cur_ant];
	UINT32 cur_tp_dl = pAd->diversity_ctl[band_idx].cur_tx_tput[cur_ant];
	UINT8 countdown = 0;
	UINT8 cur_mode = pAd->diversity_ctl[band_idx].cur_traffic_mode;
	UINT32 cur_cn = pAd->diversity_ctl[band_idx].cur_rx_cn[cur_ant];
	UINT32 cur_status = pAd->diversity_ctl[band_idx].diversity_status;
	UINT32 cur_rx_rate = pAd->diversity_ctl[band_idx].cur_rx_rate[cur_ant];
	UINT8 cn_degrade_status = ANT_DIVERSITY_CN_DEGRADE_FALSE;
	UINT8 rx_rate_degrade_status = ANT_DIVERSITY_RX_RATE_DEGRADE_FALSE;

	if (cur_status == ANT_DIVERSITY_STATUS_IDLE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"should not happen!!! band_idx(%d)\n", band_idx);
		return;
	}

	if (band_idx == DBDC_BAND0)
		countdown = ANT_COUNTDOWN_2G;
	else
		countdown = ANT_COUNTDOWN_5G;
	if (pAd->diversity_ctl[band_idx].dbg_countdown != 0)
		countdown = pAd->diversity_ctl[band_idx].dbg_countdown;

	/* if T-put or CN degrade more than TH, need to switch to another ant, then compare*/
	if (cur_status == ANT_DIVERSITY_STATUS_TP_RUNNING) {
		if (cur_mode == TRAFFIC_UL_MODE) {
			if (pAd->diversity_ctl[band_idx].ant_switch_countdown == 0) {
				if (ant_diversity_rx_rate_degrade(pAd, band_idx, cur_ant) == ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE) {
					if (cur_ant == ANT_MAIN)
						diversity_switch_antenna(pAd, band_idx, ANT_AUX);
					else
						diversity_switch_antenna(pAd, band_idx, ANT_MAIN);
					pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
					pAd->diversity_ctl[band_idx].ant_switch_countdown = countdown;
				}
			} else
				pAd->diversity_ctl[band_idx].ant_switch_countdown--;
		} else {
			if (ant_diversity_tp_degrade(pAd, band_idx, cur_ant) == TRUE) {
				if (cur_ant == ANT_MAIN)
					diversity_switch_antenna(pAd, band_idx, ANT_AUX);
				else
					diversity_switch_antenna(pAd, band_idx, ANT_MAIN);
				pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
			}
		}
	}
	/* for DL: compare T-put; for UL: compare Rx rate & CN base on main & aux, then select the better one*/
	if (cur_status == ANT_DIVERSITY_STATUS_TP_COMPARE) {
		if (cur_mode == TRAFFIC_UL_MODE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band(%d) compare stage! countdown(%d).\n",
				band_idx, pAd->diversity_ctl[band_idx].ant_switch_countdown);
			if (pAd->diversity_ctl[band_idx].ant_switch_countdown == 0) {
				if (diversity_sel_ant_by_better_rx_rate(pAd, band_idx) == FALSE) {
					diversity_sel_ant_by_better_cn(pAd, band_idx);
				}
				pAd->diversity_ctl[band_idx].ant_switch_countdown = countdown;
				pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_TP_RUNNING;
			} else
				pAd->diversity_ctl[band_idx].ant_switch_countdown--;
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band(%d) T-put compare\n", band_idx);
			diversity_sel_ant_by_higher_tp(pAd, band_idx);
			pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_TP_RUNNING;
		}
	}
	/* at the beginning of T-put streaming, forcedly switch to ANT_AUX to calculate the T-put/CN/Rx rate, then compare with ANT_MAIN*/
	if (cur_status == ANT_DIVERSITY_STATUS_STREAM_START) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band(%d) streaming start, countdown(%d), cur_traf_mode(%d)\n",
			band_idx, pAd->diversity_ctl[band_idx].ant_switch_countdown, pAd->diversity_ctl[band_idx].cur_traffic_mode);
		if (pAd->diversity_ctl[band_idx].ant_switch_countdown == 0) {
			diversity_switch_antenna(pAd, band_idx, ANT_AUX);
			pAd->diversity_ctl[band_idx].diversity_status = ANT_DIVERSITY_STATUS_TP_COMPARE;
			pAd->diversity_ctl[band_idx].ant_switch_countdown = countdown;
		} else {
			pAd->diversity_ctl[band_idx].ant_switch_countdown--;
		}
	}

	pAd->diversity_ctl[band_idx].last_rx_tput[cur_ant] = cur_tp_ul;
	pAd->diversity_ctl[band_idx].last_tx_tput[cur_ant] = cur_tp_dl;
	if (cn_degrade_status != ANT_DIVERSITY_CN_DEGRADE_WAITING)
		pAd->diversity_ctl[band_idx].last_rx_cn[cur_ant] = cur_cn;
	if (rx_rate_degrade_status != ANT_DIVERSITY_RX_RATE_DEGRADE_WAITING)
		pAd->diversity_ctl[band_idx].last_rx_rate[cur_ant] = cur_rx_rate;
	return;
}

INT set_ant_diversity_debug_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 log_enable_1st = 0;
	UINT32 log_enable_2nd = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(band_idx), &(log_enable_1st), &(log_enable_2nd));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, log_enable_1st = %d, log_enable_2nd = %d\n",
					 band_idx, log_enable_1st, log_enable_2nd);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((log_enable_1st != 0) && (log_enable_1st != 1)) || ((log_enable_2nd != 0) && (log_enable_2nd != 1)) || (i4Recv != 3)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "set band(%d) log_enable_1st %s, log_enable_2nd %s success!\n",
					 band_idx, ((log_enable_1st == 1)?"enable":"disable"), ((log_enable_2nd == 1)?"enable":"disable"));
			if (band_idx == DBDC_BAND0) {
				pAd->diversity_ctl[DBDC_BAND0].dbg_flag_lvl1 = log_enable_1st;
				pAd->diversity_ctl[DBDC_BAND0].dbg_flag_lvl2 = log_enable_2nd;
			}
			else {
				pAd->diversity_ctl[DBDC_BAND1].dbg_flag_lvl1 = log_enable_1st;
				pAd->diversity_ctl[DBDC_BAND1].dbg_flag_lvl2 = log_enable_2nd;
			}
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_disable_forcedly(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 disable = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(disable));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, disable = %d\n",
					 band_idx, disable);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((disable != 0) && (disable != 1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set diversity %s success!\n",
					 band_idx, ((disable == 0)?"enable":"disable"));
			pAd->diversity_ctl[band_idx].diversity_force_disable = disable;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_tp_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 degrade_th = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(degrade_th));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, degrade_th = %d\n",
					 band_idx, degrade_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set T-put degrade TH(%d) success!\n",
					 band_idx, degrade_th);
			pAd->diversity_ctl[band_idx].dbg_tp_deg_th = degrade_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_cn_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 max_cn_th = 0;
	UINT32 min_cn_th = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d-%d", &(band_idx), &(max_cn_th), &(min_cn_th));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, max_cn_th = %d, min_cn_th = %d\n",
					 band_idx, max_cn_th, min_cn_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| (max_cn_th < 0) || (min_cn_th < 0) || (max_cn_th >255) || (min_cn_th >255)
			|| (i4Recv != 3)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set CN degrade threshold(%d:%d) success!\n",
					 band_idx, max_cn_th, min_cn_th);
			pAd->diversity_ctl[band_idx].dbg_cn_deg_th_min = min_cn_th;
			pAd->diversity_ctl[band_idx].dbg_cn_deg_th_max = max_cn_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_rx_rate_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 rx_rate_deg_th = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enter!\n");

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(rx_rate_deg_th));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, rx_rate_deg_th = %d\n",
					 band_idx, rx_rate_deg_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set Rx rate degrade threshold(%d) success!\n",
					 band_idx, rx_rate_deg_th);
			pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_th = rx_rate_deg_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_read_interval_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;

	value = (UINT16)simple_strtol(arg, 0, 10);
	pAd->cn_rate_read_interval = value;
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"cn_rate_read_interval = %d\n", value);
	return TRUE;
}

INT set_ant_diversity_dbg_countdown_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 count_down = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(count_down));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, count_down = %d\n",
					 band_idx, count_down);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set countdown TH(%d) success!\n",
					 band_idx, count_down);
			pAd->diversity_ctl[band_idx].dbg_countdown = count_down;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_dbg_ul_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 tp_th = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(tp_th));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, ul_tp_th = %d\n",
					band_idx, tp_th);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set uplink T-put TH(%d) success!\n",
					 band_idx, tp_th);
			pAd->diversity_ctl[band_idx].dbg_ul_tp_th = tp_th;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_select_antenna(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 ant_idx = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(ant_idx));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, ant_idx = %d\n",
					 band_idx, ant_idx);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1))
			|| ((ant_idx != ANT_MAIN) && (ant_idx != ANT_AUX))
			|| (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), switch to ant(%d) success!\n",
					 band_idx, ant_idx);
			ant_diversity_gpio_control(pAd, band_idx, ant_idx);
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_rx_rate_delta_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 delta = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(delta));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, delta_th = %d\n",
					 band_idx, delta);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set rx rate delta TH(%d) success!\n",
					 band_idx, delta);
			pAd->diversity_ctl[band_idx].dbg_ul_rate_delta_th = delta;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_cn_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 threshold = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(threshold));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, threshold = %d\n",
					 band_idx, threshold);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (threshold == 0) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set CN continuous degrade threshold(%d) success!\n",
					 band_idx, threshold);
			pAd->diversity_ctl[band_idx].dbg_cn_deg_cnt_th = threshold;
			return TRUE;
		}
	}
	return -1;
}

INT set_ant_diversity_rx_rate_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 band_idx = 0;
	UINT32 threshold = 0;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(band_idx), &(threshold));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx = %d, threshold = %d\n",
					 band_idx, threshold);
		} while (0);

		if (((band_idx != DBDC_BAND0) && (band_idx != DBDC_BAND1)) || (threshold == 0) || (i4Recv != 2)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band(%d), set Rx rate continuous degrade threshold(%d) success!\n",
					 band_idx, threshold);
			pAd->diversity_ctl[band_idx].dbg_rx_rate_deg_cnt_th = threshold;
			return TRUE;
		}
	}
	return -1;
}
#endif

static inline VOID dynamic_sw_rps_control(struct _RTMP_ADAPTER *ad)
{
	UINT32 ap_peak_tp_th_ul = 0;
	UINT32 ap_peak_tp_th_dl = 0;
	UCHAR band_idx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct peak_tp_ctl *peak_tp_ctl = NULL;

	if (!cap->RxSwRpsEnable || !cap->rx_qm_en)
		return;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {

		peak_tp_ctl = &ad->peak_tp_ctl[band_idx];

		if (peak_tp_ctl->client_nums == 0)
			continue;

		ap_peak_tp_th_ul = cap->RxSwRpsTpThreshold;
		ap_peak_tp_th_dl = cap->sw_rps_tp_thd_dl;

		/* scenario detection, sw rps dispatch to another CPU if TP is more than threshold */
		if (((peak_tp_ctl->main_traffc_mode == TRAFFIC_UL_MODE)
				&& (peak_tp_ctl->main_rx_tp > ap_peak_tp_th_ul)) ||
			((peak_tp_ctl->main_traffc_mode == TRAFFIC_DL_MODE)
				&& (peak_tp_ctl->main_tx_tp > ap_peak_tp_th_dl)))
			ad->rx_dequeue_sw_rps_enable = TRUE;
		else
			ad->rx_dequeue_sw_rps_enable = FALSE;
	}
}

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
	IN  PRTMP_ADAPTER	pAd,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN UINT8 cmd)
{
	UCHAR 			apidx = MAIN_MBSSID;
	UCHAR			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
#ifdef OCE_FILS_SUPPORT
#define OCE_FILS_CMD_PAYLOAD_LEN 1500
#else
#define OCE_FILS_CMD_PAYLOAD_LEN 0
#endif /* OCE_FILS_SUPPORT */
#ifdef RADIUS_ACCOUNTING_SUPPORT
	DOT1X_QUERY_STA_DATA	data;
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + sizeof(DOT1X_QUERY_STA_DATA) + OCE_FILS_CMD_PAYLOAD_LEN)
#else
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + OCE_FILS_CMD_PAYLOAD_LEN)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

	UINT			frame_len = 0;
	PCHAR			PFrameBuf;
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;
	/* Init the frame buffer */
	os_alloc_mem(pAd, (UCHAR **)&PFrameBuf, FRAME_BUF_LEN);
	if (PFrameBuf == NULL)
		return FALSE;
	else
		os_zero_mem(PFrameBuf, FRAME_BUF_LEN);

	if (pEntry) {
#ifdef RADIUS_ACCOUNTING_SUPPORT
		NdisMoveMemory(data.StaAddr, pEntry->Addr, MAC_ADDR_LEN);
		data.rx_bytes = pEntry->RxBytes;
		data.tx_bytes = pEntry->TxBytes;
		data.rx_packets = pEntry->RxPackets.u.LowPart;
		data.tx_packets = pEntry->TxPackets.u.LowPart;
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rx_byte:%lu  tx_byte:%lu rx_pkt:%lu tx_pkt:%lu\n",
				 data.rx_bytes, data.tx_bytes, data.rx_packets, data.rx_packets);
#endif /* RADIUS_ACCOUNTING_SUPPORT */
		apidx = pEntry->func_tb_idx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	} else {
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
		bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
		bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
		frame_len = (LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN);
		MAKE_802_3_HEADER(PFrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, s_addr, EAPOL_IE);
		offset += LENGTH_802_3 - 2;
		NdisMoveMemory((PFrameBuf + offset), TPID, 2);
		offset += 2;
		TCI = (bss_Vlan & 0x0FFF) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif
		*(USHORT *)(PFrameBuf + offset) = TCI;
		offset += 2;
		NdisMoveMemory((PFrameBuf + offset), EAPOL_IE, 2);
		offset += 2;
	} else {
		frame_len = (LENGTH_802_3 + sizeof(RalinkIe));
		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(PFrameBuf,
						  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
						  s_addr,
						  EAPOL_IE);
		offset += LENGTH_802_3;
	}

	/* Prepare the specific header of internal command */
	NdisMoveMemory((PFrameBuf + offset), RalinkIe, sizeof(RalinkIe));
#ifdef RADIUS_ACCOUNTING_SUPPORT
	offset += sizeof(RalinkIe);
	/*add accounting info*/
	NdisMoveMemory((PFrameBuf + offset), (unsigned char *)&data, sizeof(DOT1X_QUERY_STA_DATA));
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

#ifdef OCE_FILS_SUPPORT
	if ((cmd == DOT1X_MLME_EVENT) ||
		(cmd == DOT1X_AEAD_ENCR_EVENT)) {
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pEntry is NULL\n");
			os_free_mem(PFrameBuf);
			return FALSE;
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		offset += sizeof(DOT1X_QUERY_STA_DATA);
#else
		offset += sizeof(RalinkIe);
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
		frame_len += pEntry->filsInfo.pending_ie_len;
		if (frame_len > FRAME_BUF_LEN) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"frame_len(%d) > FRAME_BUF_LEN(%d) (cmd=%d)\n",
				frame_len, (UINT32)FRAME_BUF_LEN, cmd);
			os_free_mem(PFrameBuf);
			return FALSE;
		}

		NdisMoveMemory((PFrameBuf + offset), pEntry->filsInfo.pending_ie, pEntry->filsInfo.pending_ie_len);
	} else if (cmd == DOT1X_AEAD_DECR_EVENT) {
		PHEADER_802_11 pHeader = NULL;
		UCHAR hdr_len = LENGTH_802_11;

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pEntry is NULL\n");
			os_free_mem(PFrameBuf);
			return FALSE;
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		offset += sizeof(DOT1X_QUERY_STA_DATA);
#else
		offset += sizeof(RalinkIe);
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
		/* PTK Info */
		frame_len += pEntry->filsInfo.PTK_len;
		NdisMoveMemory((PFrameBuf + offset), pEntry->filsInfo.PTK, pEntry->filsInfo.PTK_len);
		offset += pEntry->filsInfo.PTK_len;

		/* EAPOL Packet */
		pHeader = (PHEADER_802_11)pEntry->filsInfo.pending_ie;
#ifdef A4_CONN
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif /* A4_CONN */

		frame_len += (pEntry->filsInfo.pending_ie_len - hdr_len - LENGTH_802_1_H);
		if (frame_len > FRAME_BUF_LEN) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"frame_len(%d) > FRAME_BUF_LEN(%d) (cmd=%d)\n",
				frame_len, (UINT32)FRAME_BUF_LEN, cmd);
			os_free_mem(PFrameBuf);
			return FALSE;
		}
		NdisMoveMemory((PFrameBuf + offset), &pEntry->filsInfo.pending_ie[hdr_len + LENGTH_802_1_H],
			(pEntry->filsInfo.pending_ie_len - hdr_len - LENGTH_802_1_H));
	}
#endif /* OCE_FILS_SUPPORT */

	/* Report to upper layer */
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==================================================\n");
	hex_dump_with_lvl("PFrameBuf", (char *)PFrameBuf, frame_len, DBG_LVL_INFO);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==================================================\n");
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, (char *)PFrameBuf, frame_len) == FALSE) {
		os_free_mem(PFrameBuf);
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "done. (cmd=%d)\n", cmd);
	os_free_mem(PFrameBuf);
	return TRUE;
}

/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	/* TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx */
	UCHAR			apidx = MAIN_MBSSID;
	UCHAR			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			*FrameBuf = NULL;
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;

	if (!pEntry)
		return FALSE;
	if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
		/* Init the frame buffer */
		os_alloc_mem(pAd, (UCHAR **)&FrameBuf, (frame_len+32));
		NdisZeroMemory(FrameBuf, frame_len);
		/* Assign apidx */
		apidx = pEntry->func_tb_idx;

		if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
			/*Prepare 802.3 header including VLAN tag*/
			bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
			bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
			frame_len += VLAN_HDR_LEN;
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL)
			offset += LENGTH_802_3 - 2;
			NdisMoveMemory((FrameBuf + offset), TPID, 2);
			offset += 2;
			TCI = (bss_Vlan & 0x0fff) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef RT_BIG_ENDIAN
			TCI = SWAP16(TCI);
#endif
			*(USHORT *)(FrameBuf + offset) = TCI;
			offset += 2;
			NdisMoveMemory((FrameBuf + offset), EAPOL, 2);
			offset += 2;
		} else {
			/* Prepare the 802.3 header */
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
			offset += LENGTH_802_3;
		}

		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry) {
			BSS_STRUCT *pMbss = pEntry->pMbss;

			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (IS_AKM_WPA2_Entry(&pMbss->wdev)) &&
				(pEntry->hs_info.ppsmo_exist == 1)) {
				UCHAR HS2_Header[4] = {0x50, 0x6f, 0x9a, 0x12};

				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4 + sizeof(struct _sta_hs_info);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DOT1X_EapTriggerAction: event eapol start, %x:%x:%x:%x\n",
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 5],
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 6], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 7]);
			}
		}

#endif

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)  {
			os_free_mem(FrameBuf);
			return FALSE;
		}
		os_free_mem(FrameBuf);

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Notify 8021.x daemon to trigger EAP-SM for this sta("MACSTR")\n", MAC2STR(pEntry->Addr));
	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */


INT rtmp_ap_init(RTMP_ADAPTER *pAd)
{

#if defined(CFG_SUPPORT_FALCON_MURU) && defined(WIFI_CSI_CN_INFO_SUPPORT)
		UINT8 u1BandIdx;
#endif /* CFG_SUPPORT_FALCON_MURU */

#ifdef WSC_AP_SUPPORT
	UCHAR j;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;
	PWSC_CTRL pWscControl;
#ifdef WIFI_CSI_CN_INFO_SUPPORT
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
		mbss = &pAd->ApCfg.MBSSID[j];
		wdev = &pAd->ApCfg.MBSSID[j].wdev;
		{
			pWscControl = &wdev->WscControl;
			pWscControl->WscRxBufLen = 0;
			pWscControl->pWscRxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscRxBuf)
				NdisZeroMemory(pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscTxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscTxBuf)
				NdisZeroMemory(pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);
		}
	}

#endif /* WSC_AP_SUPPORT */
	APOneShotSettingInitialize(pAd);
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "apstart up "MACSTR"\n",
			  MAC2STR(pAd->CurrentAddress));
	APInitForMain(pAd);

	/* Set up the Mac address*/
#ifndef P2P_APCLI_SUPPORT
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
#endif /* P2P_APCLI_SUPPORT */
#ifdef CFG_SUPPORT_MU_MIMO_RA

	if (IS_MT7615(pAd)) {
	/*Send In-Band Command to N9 about Platform Type 7621/7623*/
		SetMuraPlatformTypeProc(pAd);
		SetMuraEnableHwSwPatch(pAd);
	}
#endif

#ifdef CFG_SUPPORT_FALCON_MURU
	if (IS_MT7915(pAd)) {
		SetMuruPlatformTypeProc(pAd);
	}
#endif
#ifdef MT_FDB
	fdb_enable(pAd);
#endif /* MT_FDB */
#ifdef CFG_SUPPORT_FALCON_MURU
	/* max ru cnt/mu-mimo cnt */
	{
		UINT8 u1BandIdx;

		for (u1BandIdx = 0; u1BandIdx < DBDC_BAND_NUM; u1BandIdx++)
			muru_cfg_dlul_limits(pAd, u1BandIdx);
	}
#endif /* CFG_SUPPORT_FALCON_MURU */

#ifdef WIFI_CSI_CN_INFO_SUPPORT
	for (u1BandIdx = 0; u1BandIdx < DBDC_BAND_NUM; u1BandIdx++)
		if (ops->set_csi_cn_info)
			ops->set_csi_cn_info(pAd, u1BandIdx);
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


VOID rtmp_ap_exit(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
	INT loop;
	PRRM_CONFIG pRrmCfg;
#endif
#endif /* DOT11K_RRM_SUPPORT */

#ifdef HOSTAPD_HS_R2_SUPPORT
	INT idx;
#endif
	RTMPReleaseTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPReleaseTimer(&pAd->ApCfg.IDSTimer, &Cancelled);
#endif /* IDS_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].wdev.RrmCfg;

		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
	}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	for (loop = 0; loop < MAX_MULTI_STA; loop++) {
		pRrmCfg = &pAd->StaCfg[loop].wdev.RrmCfg;

		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietOffsetTimer, &Cancelled);
		RTMPCancelTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
		RTMPReleaseTimer(&pRrmCfg->QuietCB.QuietTimer, &Cancelled);
	}
#endif /* CONFIG_STA_SUPPORT*/
#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

#ifdef HOSTAPD_HS_R2_SUPPORT
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[idx];
		PHOTSPOT_CTRL pHSCtrl = &mbss->HotSpotCtrl;
		PGAS_CTRL pGasCtrl = &mbss->GASCtrl;

		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->HSIndicationIE) {
			os_free_mem(pHSCtrl->HSIndicationIE);
			pHSCtrl->HSIndicationIE = NULL;
		}
		if (pHSCtrl->P2PIE) {
			os_free_mem(pHSCtrl->P2PIE);
			pHSCtrl->P2PIE = NULL;
		}
		if (pHSCtrl->RoamingConsortiumIE) {
			os_free_mem(pHSCtrl->RoamingConsortiumIE);
			pHSCtrl->RoamingConsortiumIE = NULL;
		}
		if (pHSCtrl->QosMapSetIE) {
			os_free_mem(pHSCtrl->QosMapSetIE);
			pHSCtrl->QosMapSetIE = NULL;
		}
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);

		RTMP_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->InterWorkingIE) {
			os_free_mem(pGasCtrl->InterWorkingIE);
			pGasCtrl->InterWorkingIE = NULL;
		}
		if (pGasCtrl->AdvertisementProtoIE) {
			os_free_mem(pGasCtrl->AdvertisementProtoIE);
			pGasCtrl->AdvertisementProtoIE = NULL;
		}
		RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);

	}

#endif
#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerRelease(pAd);
#endif /*DOT11N_DRAFT3*/
#endif /*DOT11_N_SUPPORT*/
	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);
#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
	MultiCastWLTableReset(pAd, &pAd->pMcastWLTable);
#ifdef IGMP_SNOOPING_DENY_LIST
	MulticastDLTableReset(pAd, &pAd->pMcastDLTable);
#endif
#endif /* IGMP_SNOOP_SUPPORT */
}

/*
* system security decision for ap mode
*/
UINT32 starec_ap_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	UINT32 features = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		features |= STA_REC_AP_PS_FEATURE;

	/*temport used for security will integrate to CMD*/
	if (IS_CIPHER_WEP(entry->SecConfig.PairwiseCipher))
		features |= STA_REC_INSTALL_KEY_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}


/*system for ap mode*/
/*
*
*/
INT ap_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "(caller:%pS), wdev(%d)\n",
			OS_TRACE, wdev->wdev_idx);

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		wifi_sys_linkup(wdev, NULL);
		APStartRekeyTimer(wdev->sys_handle, wdev);
	}

	OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	return TRUE;
}

/*
*
*/
INT ap_link_down(struct wifi_dev *wdev)
{

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, "(caller:%pS), wdev(%d)\n",
			 OS_TRACE, wdev->wdev_idx);

	APStopRekeyTimer(wdev->sys_handle, wdev);
	ap_sec_deinit(wdev);

	if (WDEV_BSS_STATE(wdev) >= BSS_ACTIVE) {
		/* bss not ready for mlme */
		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
		/* kick out all stas behind the Bss */
		MbssKickOutStas(wdev->sys_handle, wdev->func_idx, REASON_DISASSOC_INACTIVE);
		UpdateBeaconHandler(
			wdev->sys_handle,
			wdev,
			BCN_UPDATE_DISABLE_TX);

		/*linkdown bssinfo*/
		if (wifi_sys_linkdown(wdev) != TRUE) {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "linkdown fail!\n");
			return FALSE;
		}
	}

	return TRUE;
}

/*
*
*/
INT ap_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	/*generic connection action*/
	if (wifi_sys_conn_act(wdev, entry) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "connect action fail!\n");
	}

	/* Indicating Complete status for wait completion at WpaSend */

	if (entry->EntryState == ENTRY_STATE_SYNC) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"**calling RTMP_OS_COMPLETE(WtblSetDone)\n");
		entry->WtblSetFlag = TRUE;
		RTMP_OS_COMPLETE(&entry->WtblSetDone);
	}

	chip_ra_init(ad, entry);

#if defined(CONFIG_RA_PHY_RATE_SUPPORT) && defined(DOT11_HE_AX)
	if (wdev->eap.eap_hesuprate_en) {
		eaprawrapperentryset(ad, entry, &entry->RaEntry);
		RTEnqueueInternalCmd(ad, CMDTHREAD_UPDATE_MAXRA, entry,
				     sizeof(MAC_TABLE_ENTRY));
	}
#endif

	/* WiFi Certification config per peer */
	ap_set_wireless_sta_configs(ad, entry);
	/* WiFi Certification config per bss */
	ap_set_wireless_bss_configs(ad, entry->wdev);

	return TRUE;
}

/*
*
*/
INT ap_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (wdev->state)
		return TRUE;

	wdev->state = 1;
	/* mbss_idx required for Mac address assignment */
	pAd->ApCfg.MBSSID[wdev->func_idx].mbss_idx = wdev->func_idx;
#ifdef DBDC_ONE_BAND_SUPPORT
	/*dont open second band wdev, if dbdc is set to one band only */
	if (pAd->CommonCfg.DbdcBandSupport) {
		if ((pAd->CommonCfg.DbdcBandSupport == 1) && (WMODE_CAP_5G(wdev->PhyMode))) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"skip wdev open for 5G band wdev\n");
			return TRUE;
		} else if ((pAd->CommonCfg.DbdcBandSupport == 2) && (WMODE_CAP_2G(wdev->PhyMode))) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"skip wdev open for 5G band wdev\n");
			return TRUE;
		}
	}
#endif /*DBDC_ONE_BAND_SUPPORT*/
	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open fail!!!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP inf up for ra_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx);

	MlmeRadioOn(pAd, wdev);

	/* action for ap interface up */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pAd->ApCfg.MBSSID[wdev->func_idx].APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
		wdev->bAllowBeaconing = FALSE;
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable Beaconing\n");
	} else {
		wdev->bAllowBeaconing = TRUE;
	}
#else
	wdev->bAllowBeaconing = TRUE;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
	wdev->bRejectAuth = FALSE;
	auto_ch_select_reset_sm(pAd, wdev);
/*bndstrg init need after auto channel selection*/
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering)
		BndStrg_Init(pAd);
#endif /* BAND_STEERING */

#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanInit(pAd, wdev);
#endif /* BACKGROUND_SCAN_SUPPORT */
	ap_run_at_boot(pAd, wdev);

	APStartUpForMbss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
	/* Logic to perform OBSS scan for 2.4G only and
	 * one time for all  MBSS configured to same channel.
	 */
	{
	BSS_STRUCT *pMbss = NULL;

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	/*
	* GBandChanBitMap is used to store the 2.4Ghz channel for which
	* BSS overlap scan is done.
	* Purpose: In case of MBSS, to avoid repeated
	* overlapped scan for the same channel.
	* No need for obss scan if RF up is on 5Ghz
	*/
	if (pMbss && (pMbss->wdev.channel < 14) && WMODE_CAP_2G(pMbss->wdev.PhyMode)) {
		if (!(pAd->ApCfg.ObssGBandChanBitMap & (1 << pMbss->wdev.channel))) {
			pAd->ApCfg.ObssGBandChanBitMap |= (1 << pMbss->wdev.channel);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wdev->func_idx:%d channel:%d ApCfg.BssidNum:%d \
			 wdev.if_up_down_state:%d GBandChanBitMap:0x%x\n",
			wdev->func_idx, wdev->channel,
			pAd->ApCfg.BssidNum, pMbss->wdev.if_up_down_state,
			pAd->ApCfg.ObssGBandChanBitMap);

			if (pMbss->wdev.if_up_down_state) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"calling ap_over_lapping_scan\n");
				ap_over_lapping_scan(pAd, pMbss);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"2.4G Band CHAN already Set:: GBandChanBitMap:0x%x pMbss->wdev.channel:%d\n",
				pAd->ApCfg.ObssGBandChanBitMap, pMbss->wdev.channel);
			if (pAd->CommonCfg.need_fallback == 1) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Need fallback to 20 MHz: pMbss->wdev.channel:%d\n",
						 pMbss->wdev.channel);
				wlan_operate_set_ht_bw(&pMbss->wdev, HT_BW_20, EXTCHA_NONE);
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"CHAN is of 5G Band: pMbss->wdev.channel:%d\n",
			pMbss->wdev.channel);
	}
	}

#ifdef CONFIG_6G_SUPPORT
	ap_6g_cfg_init(pAd, wdev);
	bssmnger_reg_bmg_entry(wdev);
#endif

#ifdef DOT11_VHT_AC
	/* RTS Bandwidth Signaling Setting for VHT mode*/
	if (wlan_config_get_vht_bw_sig(wdev) > BW_SIGNALING_DISABLE) {
		EXT_CMD_CFG_SET_RTS_SIGTA_EN_T ExtCmdRtsSigTaCfg = {0};
		EXT_CMD_CFG_SET_SCH_DET_DIS_T ExtCmdSchDetDisCfg = {0};

		ExtCmdRtsSigTaCfg.Enable = TRUE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_RTS_SIGTA_EN_FEATURE, &ExtCmdRtsSigTaCfg, sizeof(ExtCmdRtsSigTaCfg));

		ExtCmdSchDetDisCfg.Disable = FALSE;
		CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_SCH_DET_DIS_FEATURE, &ExtCmdSchDetDisCfg, sizeof(ExtCmdSchDetDisCfg));
	}
#endif

#ifdef WSC_INCLUDED
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"apidx %d for WscUUIDInit\n", wdev->func_idx);
	WscUUIDInit(pAd, wdev->func_idx, FALSE);
#endif /* WSC_INCLUDED */

#ifdef WIFI_DIAG
	diag_proc_init(pAd, wdev);
#endif

#ifdef DPP_SUPPORT
	DlListInit(&wdev->dpp_frame_event_list);
#endif /* DPP_SUPPORT */


	return TRUE;
}

/*
*
*/
INT ap_inf_close(struct wifi_dev *wdev)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif	/* CONFIG_AP_SUPPORT && CONFIG_STA_SUPPORT */

	if (!wdev->state)
		return TRUE;
	wdev->state = 0;

#ifdef WIFI_DIAG
	diag_proc_exit(pAd, wdev);
#endif

	/* Move orig RTMPInfClose here */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev->bAllowBeaconing = FALSE;
		wdev->bRejectAuth = TRUE;

#ifdef CONFIG_6G_SUPPORT
		bssmnger_dereg_bmg_entry(wdev);
		ap_6g_cfg_deinit(pAd, wdev);
#endif

		if (wdev_do_linkdown(wdev) != TRUE)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkdown fail!!!\n");

		if (wifi_sys_close(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "close fail!!!\n");
			return FALSE;
		}

#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering) {
			PBND_STRG_CLI_TABLE table;

			table = Get_BndStrgTable(pAd, BSS0);
			if (table) {
				/* Inform daemon interface down */
				BndStrg_SetInfFlags(pAd, wdev, table, FALSE);
			}
		}
#endif /* BAND_STEERING */
		auto_ch_select_reset_sm(pAd, wdev);
	}


	return TRUE;
}

BOOLEAN media_state_connected(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	return (OPSTATUS_TEST_FLAG(ad, fOP_STATUS_MEDIA_STATE_CONNECTED)
			&& OPSTATUS_TEST_FLAG(ad, fOP_AP_STATUS_MEDIA_STATE_CONNECTED));
}

VOID ap_fsm_ops_hook(struct wifi_dev *wdev)
{
	ap_cntl_init(wdev);
	ap_auth_init(wdev);
	ap_assoc_init(wdev);
}

#ifdef CONN_FAIL_EVENT
void ApSendConnFailMsg(
	PRTMP_ADAPTER pAd,
	CHAR *Ssid,
	UCHAR SsidLen,
	UCHAR *StaAddr,
	USHORT ReasonCode)
{
	struct CONN_FAIL_MSG msg;

	if (!pAd)
		return;

	memset(&msg, 0, sizeof(msg));

	if (SsidLen > 32)
		msg.SsidLen = 32;
	else
		msg.SsidLen = SsidLen;

	if (Ssid && (msg.SsidLen > 0))
		memcpy(msg.Ssid, Ssid, msg.SsidLen);

	if (StaAddr)
		memcpy(msg.StaAddr, StaAddr, 6);

	msg.ReasonCode = ReasonCode;

	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_802_11_CONN_FAIL_MSG,
		NULL,
		(UCHAR *)&msg,
		sizeof(msg));

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"SsidLen=%d, Ssid=%s, StaAddr="MACSTR", ReasonCode=%d\n",
		msg.SsidLen, msg.Ssid, MAC2STR(msg.StaAddr), msg.ReasonCode);
}
#endif

