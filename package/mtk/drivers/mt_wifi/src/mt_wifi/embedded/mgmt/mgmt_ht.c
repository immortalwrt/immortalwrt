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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"


INT ht_support_bw_by_channel_boundary(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, IE_LISTS *peer_ie_list)
{
	struct wifi_dev *wdev = pEntry->wdev;
	UCHAR supported_bw;
	UCHAR ext_cha;

	if (!wdev)
		return HT_BW_20;

	supported_bw = wlan_operate_get_ht_bw(wdev);
	ext_cha = wlan_operate_get_ext_cha(wdev);
	if (!peer_ie_list)
		return supported_bw;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(), Channel = %d, supported_bw = %d, extcha = %d, PeerSupChlLen = %d\n", __func__,
			 wdev->channel,
			 supported_bw,
			 ext_cha,
			 peer_ie_list->SupportedChlLen);

	/* So far, only check 2.4G band */
	if (WMODE_CAP_2G(wdev->PhyMode) && (peer_ie_list->SupportedChlLen >= 2)) {
		UCHAR i;
		UCHAR peer_sup_chl_min_2g = 1;
		UCHAR peer_sup_chl_max_2g = 14;

		/* Get Min/Max 2.4G supported channels */
		for (i = 0; i < peer_ie_list->SupportedChlLen; i += 2) {
			if (peer_ie_list->SupportedChl[i] <= 14) {
				peer_sup_chl_min_2g = peer_ie_list->SupportedChl[i];
				peer_sup_chl_max_2g = peer_ie_list->SupportedChl[i] + peer_ie_list->SupportedChl[i + 1] - 1;
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "  peer_sup_chl_min_2g = %d, peer_sup_chl_max_2g = %d\n",
						 peer_sup_chl_min_2g, peer_sup_chl_max_2g);
			}
		}

		/* Check Min/Max 2.4G channel boundary in BW40 case */
		if ((supported_bw == BW_40) && (ext_cha != EXTCHA_NONE)) {
			if (((ext_cha == EXTCHA_ABOVE) && ((wdev->channel + 4) > peer_sup_chl_max_2g)) ||
				((ext_cha == EXTCHA_BELOW) && ((wdev->channel - 4) < peer_sup_chl_min_2g)))
				supported_bw = BW_20;
		}
	}

	return supported_bw;
}

INT ht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer_ht_cap)
{
	struct wifi_dev *wdev = pEntry->wdev;
	ADD_HT_INFO_IE *addht;
	HT_CAPABILITY_IE *ht_cap;
	UCHAR band = 0;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	band = HcGetBandByWdev(wdev);

	if ((peer_ht_cap->HtCapInfo.GF) && (ht_cap->HtCapInfo.GF))
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
	else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pAd->MacTab.fAnyStationNonGF[band] = TRUE;
	}

	pEntry->MaxHTPhyMode.field.BW = (peer_ht_cap->HtCapInfo.ChannelWidth & wlan_operate_get_ht_bw(wdev));
	if (pEntry->MaxHTPhyMode.field.BW == HT_BW_40) {
		pEntry->MaxHTPhyMode.field.ShortGI =
			((ht_cap->HtCapInfo.ShortGIfor40) & (peer_ht_cap->HtCapInfo.ShortGIfor40));
	} else {
		pEntry->MaxHTPhyMode.field.ShortGI =
			((ht_cap->HtCapInfo.ShortGIfor20) & (peer_ht_cap->HtCapInfo.ShortGIfor20));
		pAd->MacTab.fAnyStation20Only = TRUE;
	}
	pEntry->MaxHTPhyMode.field.STBC = (peer_ht_cap->HtCapInfo.RxSTBC & ht_cap->HtCapInfo.TxSTBC);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "MODE = %d, BW = %d, SGI = %d, STBC = %d\n",
			 pEntry->MaxHTPhyMode.field.MODE,
			 pEntry->MaxHTPhyMode.field.BW,
			 pEntry->MaxHTPhyMode.field.ShortGI,
			 pEntry->MaxHTPhyMode.field.STBC);
	return TRUE;
}


INT set_ht_fixed_mcs(MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound)
{
	if (fixed_mcs == 32) {
		/* Fix MCS as HT Duplicated Mode */
		pEntry->MaxHTPhyMode.field.BW = 1;
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pEntry->MaxHTPhyMode.field.STBC = 0;
		pEntry->MaxHTPhyMode.field.ShortGI = 0;
		pEntry->MaxHTPhyMode.field.MCS = 32;
	} else if (pEntry->MaxHTPhyMode.field.MCS > mcs_bound) {
		/* STA supports fixed MCS */
		pEntry->MaxHTPhyMode.field.MCS = mcs_bound;
	}

	return TRUE;
}

UINT8 get_max_nss_by_htcap_ie_mcs(UCHAR *cap_mcs)
{
	UINT8 index, nss = 0;
	UCHAR bitmask;

	index = 32;
	do {
		index--;
		nss = index / 8;
		bitmask = (1 << (index - (nss * 8)));

		if (cap_mcs[nss] & bitmask)
			break;
	} while (index);

	return nss;
}

INT get_ht_max_mcs(UCHAR *desire_mcs, UCHAR *cap_mcs)
{
	INT i, j;
	UCHAR bitmask;

	for (i = 31; i >= 0; i--) {
		j = i / 8;
		bitmask = (1 << (i - (j * 8)));

		if ((desire_mcs[j] & bitmask) && (cap_mcs[j] & bitmask)) {
			/*pEntry->MaxHTPhyMode.field.MCS = i; */
			/* find it !!*/
			break;
		}

		if (i == 0)
			break;
	}

	return i;
}


INT get_ht_cent_ch(RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch, UCHAR Channel)
{
	UCHAR op_ht_bw = HT_BW_20;
	UCHAR op_ext_cha = EXTCHA_NONE;
	UCHAR i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (!wdev)
			continue;

		/*check in the same band*/
		if (Channel == wdev->channel && wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
			op_ht_bw = wlan_operate_get_ht_bw(wdev);
			op_ext_cha = wlan_operate_get_ext_cha(wdev);

			if (op_ht_bw == HT_BW_40)
				break;
		}
	}

	if ((op_ht_bw  == HT_BW_40) &&
		(op_ext_cha == EXTCHA_ABOVE)
	   ) {
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_ABOVE;
	} else if ((Channel > 2) &&
			   (op_ht_bw == HT_BW_40) &&
			   (op_ext_cha == EXTCHA_BELOW)) {
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_BELOW;

	} else
		return FALSE;

	return TRUE;
}

UCHAR cal_ht_cent_ch(UCHAR prim_ch, UCHAR phy_bw, UCHAR ext_cha, UCHAR *cen_ch)
{
	*cen_ch = prim_ch;

	if ((phy_bw  == BW_40) &&
		(ext_cha == EXTCHA_ABOVE)
	   )
		*cen_ch = prim_ch + 2;
	else if ((prim_ch > 2) &&
			 (phy_bw == BW_40) &&
			 (ext_cha == EXTCHA_BELOW)) {
		if (prim_ch == 14)
			*cen_ch = prim_ch - 1;
		else
			*cen_ch = prim_ch - 2;
	} else
		return FALSE;

	return TRUE;
}

UCHAR get_cent_ch_by_htinfo(
	RTMP_ADAPTER *pAd,
	ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap)
{
	UCHAR cent_ch;

	if ((ht_op->ControlChan > 2) &&
		(ht_op->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan - 2;
	else if ((ht_op->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) &&
			 (ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan + 2;
	else
		cent_ch = ht_op->ControlChan;

	return cent_ch;
}


UINT16 ht_max_mpdu_size[2] = {3839u, 7935u};

VOID set_sta_ht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, HT_CAPABILITY_IE *ht_ie)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR ht_gi = wlan_config_get_ht_gi(entry->wdev);
	UINT8 ba_en = wlan_config_get_ba_enable(entry->wdev);

	/* set HT capabilty flag for entry */
	CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE);

	if (ht_gi == GI_400) {
		if (ht_ie->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI20_CAPABLE);

		if (ht_ie->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI40_CAPABLE);
	}

	if (ht_ie->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_TxSTBC_CAPABLE);

	if (ht_ie->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RxSTBC_CAPABLE);

	if (ht_ie->ExtHtCapInfo.PlusHTC) {
		CLIENT_CAP_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);
	}

	if (pAd->CommonCfg.bRdg && ht_ie->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RDG_CAPABLE);

	if (ht_ie->ExtHtCapInfo.MCSFeedback == 0x03)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

	if (wlan_config_get_ht_ldpc(entry->wdev) && (cap->phy_caps & fPHY_CAP_LDPC)) {
		if (ht_ie->HtCapInfo.ht_rx_ldpc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE);
	}

	if (wlan_config_get_amsdu_en(entry->wdev) && !ba_en)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_AMSDU_INUSED);

	entry->MpduDensity = ht_ie->HtCapParm.MpduDensity;
	entry->MaxRAmpduFactor = ht_ie->HtCapParm.MaxRAmpduFactor;
	entry->MmpsMode = (UCHAR)ht_ie->HtCapInfo.MimoPs;
	entry->AMsduSize = (UCHAR)ht_ie->HtCapInfo.AMsduSize;

	if (entry->AMsduSize < (ARRAY_SIZE(ht_max_mpdu_size)))
		entry->amsdu_limit_len = ht_max_mpdu_size[entry->AMsduSize];
	else
		entry->amsdu_limit_len = 0;
	entry->amsdu_limit_len_adjust = entry->amsdu_limit_len;
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
static VOID eapsetht(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

	if (wdev->eap.eap_htsuprate_en == TRUE) {
		switch (wlan_operate_get_rx_stream(wdev)) {
		case 4:
			ht_cap->MCSSet[3] = wdev->eap.eapmcsset[3];
			/* fall through */
		case 3:
			ht_cap->MCSSet[2] = wdev->eap.eapmcsset[2];
			/* fall through */
		case 2:
			ht_cap->MCSSet[1] = wdev->eap.eapmcsset[1];
			/* fall through */
		case 1:
			ht_cap->MCSSet[0] = wdev->eap.eapmcsset[0];
			break;
		}
	}
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetHT(
	IN RTMP_ADAPTER *pAd,
	IN OID_SET_HT_PHYMODE * pHTPhyMode,
	IN struct wifi_dev *wdev)
{
	INT idx;
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_AP_SUPPORT

	/* sanity check for extention channel */
	if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
						   CHANNEL_NO_FAT_BELOW | CHANNEL_NO_FAT_ABOVE) == TRUE) {
		/* only 20MHz is allowed */
		pHTPhyMode->BW = 0;
	} else if (pHTPhyMode->ExtOffset == EXTCHA_BELOW) {
		/* extension channel below this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
							   CHANNEL_NO_FAT_BELOW) == TRUE)
			pHTPhyMode->ExtOffset = EXTCHA_ABOVE;
	} else if (pHTPhyMode->ExtOffset == EXTCHA_ABOVE) {
		/* extension channel above this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
							   CHANNEL_NO_FAT_ABOVE) == TRUE)
			pHTPhyMode->ExtOffset = EXTCHA_BELOW;
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
			 pHTPhyMode->HtMode, pHTPhyMode->ExtOffset,
			 pHTPhyMode->MCS, pHTPhyMode->BW,
			 pHTPhyMode->STBC, pHTPhyMode->SHORTGI);
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));

	/* Decide Rx MCSSet*/
	switch (wlan_operate_get_rx_stream(wdev)) {
	case 4:
		ht_cap->MCSSet[3] =  0xff;
		/* fall through */
	case 3:
		ht_cap->MCSSet[2] =  0xff;
		/* fall through */
	case 2:
		ht_cap->MCSSet[1] =  0xff;
		/* fall through */
	case 1:
	default:
		ht_cap->MCSSet[0] =  0xff;
		break;
	}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	eapsetht(pAd, wdev);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pHTPhyMode->BW == BW_40)) {
		pHTPhyMode->BW = BW_20;
		wlan_config_set_40M_intolerant(wdev, 1);
	}

	/* TODO: shiang-6590, how about the "bw" when channel 14 for JP region??? */
	if (pHTPhyMode->BW == BW_40) {
		UCHAR ext_cha = (pHTPhyMode->ExtOffset == EXTCHA_BELOW) ? (EXTCHA_BELOW) : EXTCHA_ABOVE;

		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/
		wlan_config_set_ht_bw(wdev, HT_BW_40);

		if (WMODE_CAP_2G(wdev->PhyMode))
			ht_cap->HtCapInfo.CCKmodein40 = 1;

		wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
	} else {
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
	}

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)

	if ((pHTPhyMode->BW == BW_20) && (pHTPhyMode->Channel != 0)) {
		wlan_config_set_ht_bw(wdev, HT_BW_20);
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
	} else if (INFRA_ON(pAd)) {
		struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
		UCHAR ht_bw = wlan_config_get_ht_bw(p2p_dev);
		UCHAR ext_cha = wlan_config_get_ext_cha(p2p_dev);

		wlan_operate_set_ht_bw(p2p_dev, ht_bw, ext_cha);
	}

#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef TXBF_SUPPORT

	if (cap->FlgHwTxBfCap) {
#ifdef MT_MAC
		/* Set ETxBF */
		mt_WrapSetETxBFCap(pAd, wdev, &ht_cap->TxBFCap);
		/* Check ITxBF */
#endif /* MT_MAC */
	}

#endif /* TXBF_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx);

#ifdef APCLI_SUPPORT

		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_APCLI);

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef P2P_SUPPORT

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_P2P_GO);

		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_APCLI);

#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO);

#endif /* RT_CFG80211_P2P_SUPPORT */

		/* TODO: Pat: we initialize HT Cap even when 2nd STA is not yet initialized. */
		/* for (idx = 0; idx < pAd->MSTANum; idx++) */
		for (idx = 0; idx < pAd->MaxMSTANum; idx++)
			RTMPSetIndividualHT(pAd, idx);
	}
#endif /* CONFIG_STA_SUPPORT */
	dump_ht_cap(wdev);
}

/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetIndividualHT(RTMP_ADAPTER *pAd, UCHAR apidx)
{
	RT_PHY_INFO *pDesired_ht_phy = NULL;
	UCHAR TxStream;
	UCHAR DesiredMcs = MCS_AUTO;
	UINT32 encrypt_mode = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct adhoc_info *adhocInfo = NULL;
#endif
	UCHAR cfg_ht_bw;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(): apidx=%d\n", __func__, apidx);

	do {
#ifdef RT_CFG80211_P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO) {
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO;

			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
			DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
			encrypt_mode = wdev->SecConfig.PairwiseCipher;
			wdev->bWmmCapable = TRUE;
			wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
			break;
		}

#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO) {
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_P2P_GO;

			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
			DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
			encrypt_mode = wdev->SecConfig.PairwiseCipher;
			wdev->bWmmCapable = TRUE;
			wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
			break;
		}

#endif /* P2P_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM) {
				wdev = &pAd->StaCfg[idx].wdev;
				if (!wdev)
					return;
				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
				encrypt_mode = wdev->SecConfig.PairwiseCipher;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			} else {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid idx(%d)\n", idx);
				return;
			}
		}

#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT

			if (apidx >= MIN_NET_DEVICE_FOR_WDS) {
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY) {
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					if (!wdev)
						return;
					pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
					DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
					/*encrypt_mode = pAd->WdsTab.WdsEntry[idx].WepStatus;*/
					wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				} else {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid apidx(%d)\n", apidx);
					return;
				}
			}

#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) && (VALID_MBSS(pAd, apidx))) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				if (!wdev)
					return;
				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;

				encrypt_mode = wdev->SecConfig.PairwiseCipher;
				wdev->bWmmCapable = TRUE;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid apidx(%d)\n", apidx);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (apidx < MAX_MULTI_STA) {
				wdev = &pAd->StaCfg[apidx].wdev;
				pStaCfg = &pAd->StaCfg[apidx];
				adhocInfo = &pStaCfg->adhocInfo;
				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
				encrypt_mode = wdev->SecConfig.PairwiseCipher;
			}
			break;
		}
#endif /* CONFIG_STA_SUPPORT */
	} while (FALSE);

	if (!wdev)
		return;

	if (pDesired_ht_phy == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid apidx(%d)\n", apidx);
		return;
	}

	TxStream = wlan_operate_get_tx_stream(wdev);
	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_PHY_INFO));
	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Desired MCS = %d\n", DesiredMcs);

	/* Check the validity of MCS */
	if ((TxStream == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS(%d) is invalid in 1S, reset it as MCS_7\n", __func__, DesiredMcs));
		DesiredMcs = MCS_7;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	if ((cfg_ht_bw == HT_BW_20) && (DesiredMcs == MCS_32)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n", __func__));
		DesiredMcs = MCS_0;
	}

#ifdef CONFIG_STA_SUPPORT

	if ((pAd->OpMode == OPMODE_STA) &&
		(pStaCfg && pStaCfg->BssType == BSS_INFRA) &&
		(apidx == MIN_NET_DEVICE_FOR_MBSSID))
		;
	else
#endif /* CONFIG_STA_SUPPORT */

		/*
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode)) {
#ifdef CONFIG_STA_ADHOC_SUPPORT
			if (adhocInfo)
				adhocInfo->bAdhocN = FALSE;
#endif /* CONFIG_STA_SUPPORT */
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s : Use legacy rate in WEP/TKIP encryption mode (apidx=%d)\n",
					 __func__, apidx));
			return;
		}

	if (pAd->CommonCfg.HT_Disable) {
#ifdef CONFIG_STA_ADHOC_SUPPORT
		if (adhocInfo)
			adhocInfo->bAdhocN = FALSE;
#endif /* CONFIG_STA_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT is disabled\n");
		return;
	}

	pDesired_ht_phy->bHtEnable = TRUE;

	/* Decide desired Tx MCS*/
	switch (TxStream) {
	case 1:
		if (DesiredMcs == MCS_AUTO)
			pDesired_ht_phy->MCSSet[0] = 0xff;
		else if (DesiredMcs <= MCS_7)
			pDesired_ht_phy->MCSSet[0] = 1 << DesiredMcs;

		break;

	case 2:
		if (DesiredMcs == MCS_AUTO) {
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
		} else if (DesiredMcs <= MCS_15) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 2)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;

	case 3:
		if (DesiredMcs == MCS_AUTO) {
			/* MCS0 ~ MCS23, 3 bytes */
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
			pDesired_ht_phy->MCSSet[2] = 0xff;
		} else if (DesiredMcs <= MCS_23) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 3)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;

	case 4:
		if (DesiredMcs == MCS_AUTO) {
			/* MCS0 ~ MCS31, 4 bytes */
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
			pDesired_ht_phy->MCSSet[2] = 0xff;
			pDesired_ht_phy->MCSSet[3] = 0xff;
		} else if (DesiredMcs <= MCS_31) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 4)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;
	}

	if (cfg_ht_bw == HT_BW_40) {
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	/* update HT Rate setting */
	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, wdev);

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesired_ht_phy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, wdev, pDesired_ht_phy);
	}

#endif /* DOT11_VHT_AC */
	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"HtEn = %d, VhtEn = %d, vht_bw = %d\n",
			 wdev->DesiredHtPhyInfo.bHtEnable, wdev->DesiredHtPhyInfo.bVhtEnable,
			 wdev->DesiredHtPhyInfo.vht_bw);
}

/*
	========================================================================
	Routine Description:
		Clear the desire HT info per interface

	Arguments:

	========================================================================
*/
VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
		RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
}


INT	SetCommonHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	OID_SET_HT_PHYMODE SetHT;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (!WMODE_CAP_N(wdev->PhyMode)) {
		/* Clear previous HT information */
		RTMPDisableDesiredHtInfo(pAd, wdev);
		return FALSE;
	}

	SetHT.PhyMode = (RT_802_11_PHY_MODE)(wdev->PhyMode);
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			SetHT.TransmitNo = pAd->dbdc_band0_tx_path;
		else
			SetHT.TransmitNo = pAd->dbdc_band1_tx_path;
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			SetHT.TransmitNo = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = ext_cha;
	SetHT.MCS = MCS_AUTO;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, HcGetBandByWdev(wdev))) {
		SetHT.BW = wlan_config_get_ht_bw(wdev);
		SetHT.ExtOffset = (SetHT.BW == BW_20) ? (0) : (ext_cha);
	} else {
#endif
		SetHT.BW = (UCHAR)op_ht_bw;
		SetHT.STBC = wlan_config_get_ht_stbc(wdev);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	}
#endif
	SetHT.SHORTGI = (UCHAR)wlan_config_get_ht_gi(wdev);
	SetHT.Channel = wdev->channel;
	SetHT.BandIdx = 0;
	RTMPSetHT(pAd, &SetHT, wdev);
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.bBssCoexEnable && pAd->CommonCfg.Bss2040NeedFallBack) {
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
	}

#endif /* DOT11N_DRAFT3 */
	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Update HT IE from our capability.

	Arguments:
		Send all HT IE in beacon/probe rsp/assoc rsp/action frame.


	========================================================================
*/
VOID RTMPUpdateHTIE(
	IN UCHAR *pMcsSet,
	IN struct wifi_dev *wdev,
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT ADD_HT_INFO_IE *pAddHtInfo)
{
	UCHAR cfg_ht_bw  = wlan_config_get_ht_bw(wdev);
	UCHAR op_ht_bw = wlan_config_get_ht_bw(wdev);
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	struct _ADD_HT_INFO_IE *add_ht = wlan_operate_get_addht(wdev);

	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	pHtCapability->HtCapInfo.ChannelWidth = cfg_ht_bw;
	pHtCapability->HtCapInfo.MimoPs = ht_cap->HtCapInfo.MimoPs;
	pHtCapability->HtCapInfo.GF = ht_cap->HtCapInfo.GF;
	pHtCapability->HtCapInfo.ShortGIfor20 = ht_cap->HtCapInfo.ShortGIfor20;
	pHtCapability->HtCapInfo.ShortGIfor40 = ht_cap->HtCapInfo.ShortGIfor40;
	pHtCapability->HtCapInfo.TxSTBC = ht_cap->HtCapInfo.TxSTBC;
	pHtCapability->HtCapInfo.RxSTBC = ht_cap->HtCapInfo.RxSTBC;
	pHtCapability->HtCapInfo.AMsduSize = ht_cap->HtCapInfo.AMsduSize;
	pHtCapability->HtCapParm.MaxRAmpduFactor = ht_cap->HtCapParm.MaxRAmpduFactor;
	pHtCapability->HtCapParm.MpduDensity = ht_cap->HtCapParm.MpduDensity;
	pAddHtInfo->AddHtInfo.ExtChanOffset = add_ht->AddHtInfo.ExtChanOffset;
	pAddHtInfo->AddHtInfo.RecomWidth = op_ht_bw;
	pAddHtInfo->AddHtInfo2.OperaionMode = add_ht->AddHtInfo2.OperaionMode;
	pAddHtInfo->AddHtInfo2.NonGfPresent = add_ht->AddHtInfo2.NonGfPresent;
	RTMPMoveMemory(pAddHtInfo->MCSSet, pMcsSet, 4); /* rt2860 only support MCS max=32, no need to copy all 16 uchar.*/
	MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPUpdateHTIE <==\n");
}

/*sta rec ht features decision*/
UINT32 starec_ht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 features = 0;
	UCHAR amsdu_en = wlan_config_get_amsdu_en(wdev);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (
#ifdef SW_CONNECT_SUPPORT
		(entry->bSw == FALSE) &&
#endif /* SW_CONNECT_SUPPORT */
		(cap->ppdu.tx_amsdu_support == TRUE) && amsdu_en
		/*
			H/W AMSDU will use the Tx HT rate instead of Legacy Rate even the WTBL's rate is Legacy Rate. (TxV rate is HT rate)
			then this may let Rx Side of Legacy STA can't Recv the Data.
			We only set the AMSDU cap on TxDW7 bit[30] in below HT/VHT cases:
		*/
		&&((CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE)) || (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_VHT_CAPABLE)
			|| (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_HE_CAPABLE))))
		&& !(IS_CIPHER_TKIP_Entry(entry) || IS_CIPHER_WEP_Entry(entry))
	) {
		features |= STA_REC_AMSDU_FEATURE;
#ifdef HW_TX_AMSDU_SUPPORT
		/* notify FW the entry is support HW AMSDU and config HW AMSDU parameter (length and number) */
		if (IS_ASIC_CAP(ad, fASIC_CAP_HW_TX_AMSDU))
			features |= STA_REC_HW_AMSDU_FEATURE;
#endif
		entry->tx_amsdu_bitmap = 0xffff;
	} else {
		entry->tx_amsdu_bitmap = 0x0;
	}

	if (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE))
		features |= STA_REC_BASIC_HT_INFO_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for HT phy type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
#ifdef CONFIG_STA_SUPPORT
BOOLEAN RTMPCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN HT_CAPABILITY_IE * pHtCap,
	IN ADD_HT_INFO_IE * pAddHtInfo)
{
	MAC_TABLE_ENTRY *sta;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	UCHAR cfg_ht_bw;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	sta = &pAd->MacTab.Content[Wcid];
	pStaCfg = GetStaCfgByWdev(pAd, sta->wdev);
	ASSERT(pStaCfg);
	if (pStaCfg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "StaCfg NULL!! wcid %d entryType %d\n", Wcid, sta->EntryType);
		if (sta->wdev)
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wdev %s, idx %d\n", sta->wdev->if_dev->name, sta->wdev->wdev_idx);
		return FALSE;
	}

	/* If use AMSDU, set flag.*/
	if (wlan_config_get_amsdu_en(sta->wdev))
		CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_AMSDU_INUSED);

	/* Save Peer Capability*/
	if (wlan_config_get_ht_ldpc(sta->wdev) && (cap->phy_caps & fPHY_CAP_LDPC)) {
		if (pHtCap->HtCapInfo.ht_rx_ldpc)
			CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE);
	}

	if (wlan_config_get_ht_gi(sta->wdev) == GI_400) {
		if (pHtCap->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_SGI20_CAPABLE);

		if (pHtCap->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_SGI40_CAPABLE);
	}

	if (pHtCap->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_TxSTBC_CAPABLE);

	if (pHtCap->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_RxSTBC_CAPABLE);

	if (pAd->CommonCfg.bRdg && pHtCap->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(sta, fCLIENT_STATUS_RDG_CAPABLE);

	sta->MpduDensity = pHtCap->HtCapParm.MpduDensity;

	/* Will check ChannelWidth for MCSSet[4] below*/
	NdisZeroMemory(&pStaCfg->MlmeAux.HtCapability.MCSSet[0], 16);
	cfg_ht_bw = wlan_config_get_ht_bw(sta->wdev);
	if (cfg_ht_bw != HT_BW_20)
		pStaCfg->MlmeAux.HtCapability.MCSSet[4] = 0x1;

	switch (wlan_operate_get_rx_stream(sta->wdev)) {
	case 4:
		pStaCfg->MlmeAux.HtCapability.MCSSet[3] = 0xff;
		/* fall through */
	case 3:
		pStaCfg->MlmeAux.HtCapability.MCSSet[2] = 0xff;
		/* fall through */
	case 2:
		pStaCfg->MlmeAux.HtCapability.MCSSet[1] = 0xff;
		/* fall through */
	case 1:
	default:
		pStaCfg->MlmeAux.HtCapability.MCSSet[0] = 0xff;
		break;
	}

	pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & cfg_ht_bw;

	/*
		If both station and AP use 40MHz, still need to check if the 40MHZ band's legality in my country region
		If this 40MHz wideband is not allowed in my country list, use bandwidth 20MHZ instead,
	*/
	if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40) {
		if (RTMPCheckChannel(pAd, pStaCfg->MlmeAux.CentralChannel, pStaCfg->MlmeAux.Channel, sta->wdev) == FALSE)
			pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
/*
In TGac Test Case 5.2.66,*
TGac testbed AP uses extended channel above for channel 64 which violates spec.*
since we follow spec and set extended channel below for channel 64.
Added Work around to detect this case and set bw20 for Apcli*
*/

		if ((pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40) && (wlan_operate_get_ext_cha(sta->wdev) != pAddHtInfo->AddHtInfo.ExtChanOffset)) {
			UCHAR extcha = 0;
			UCHAR htbw = pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;

			ht_ext_cha_adjust(pAd, pStaCfg->MlmeAux.Channel, &htbw, &extcha, sta->wdev);

			if ((extcha != pAddHtInfo->AddHtInfo.ExtChanOffset) && (sta->wdev->channel > 14)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "RTMPCheckHt : channel=%u, my extcha=%u, root ap extcha=%u, inconsistent!!\n",
						 sta->wdev->channel, wlan_operate_get_ext_cha(sta->wdev), pAddHtInfo->AddHtInfo.ExtChanOffset);
				pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
				wlan_operate_set_ht_bw(sta->wdev, HT_BW_20, EXTCHA_NONE);

			}

		}



		/*BW setting limited by EEPROM setting*/
		if (pStaCfg->MlmeAux.Channel > 14) { /* a-band*/
			if (pAd->NicConfig2.field.BW40MAvailForA)  /* 1: OFF; 0: ON */
				pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
		} else {
			if (pAd->NicConfig2.field.BW40MAvailForG)  /* 1: OFF; 0: ON */
				pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;

#ifdef COEX_SUPPORT

			if ((pAd->BtCoexSupportMode & MT76xx_COEX_MODE_FDD) && (pAd->BtSkipFDDFix20MH == 0))
				pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;

#endif

		}
	}

	MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			  "RTMPCheckHt:: HtCapInfo.ChannelWidth=%d, RecomWidth=%d, Configure HT_BW=%d, BW40MAvailForA/G=%d/%d\n",
			  pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth, pAddHtInfo->AddHtInfo.RecomWidth, cfg_ht_bw,
			  pAd->NicConfig2.field.BW40MAvailForA, pAd->NicConfig2.field.BW40MAvailForG);

	if (sta->wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCap->HtCapInfo.GF;
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RTMPCheckHt:: pHtCap->HtCapInfo.GF=%d\n", pHtCap->HtCapInfo.GF);
	} else {
		pStaCfg->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCap->HtCapInfo.GF &
			wlan_config_get_greenfield(sta->wdev);
	}
	/* Send Assoc Req with my HT capability.*/
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.AMsduSize =  pHtCap->HtCapInfo.AMsduSize;
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.MimoPs =  wlan_config_get_mmps(sta->wdev);
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 = wlan_config_get_ht_gi(sta->wdev) &
			(pHtCap->HtCapInfo.ShortGIfor20);
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 = wlan_config_get_ht_gi(sta->wdev) &
			(pHtCap->HtCapInfo.ShortGIfor40);
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.TxSTBC = wlan_config_get_ht_stbc(sta->wdev) & (pHtCap->HtCapInfo.RxSTBC);
	pStaCfg->MlmeAux.HtCapability.HtCapInfo.RxSTBC = wlan_config_get_ht_stbc(sta->wdev) & (pHtCap->HtCapInfo.TxSTBC);

	if (CLIENT_STATUS_TEST_FLAG(sta, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE))
		pStaCfg->MlmeAux.HtCapability.HtCapInfo.ht_rx_ldpc = 1;
	else
		pStaCfg->MlmeAux.HtCapability.HtCapInfo.ht_rx_ldpc = 0;

	pStaCfg->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = pHtCap->HtCapParm.MaxRAmpduFactor;
	pStaCfg->MlmeAux.HtCapability.HtCapParm.MpduDensity = pHtCap->HtCapParm.MpduDensity;
	pStaCfg->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = pHtCap->ExtHtCapInfo.PlusHTC;
	sta->HTCapability.ExtHtCapInfo.PlusHTC = pHtCap->ExtHtCapInfo.PlusHTC;

	if (pAd->CommonCfg.bRdg) {
		pStaCfg->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = pHtCap->ExtHtCapInfo.RDGSupport;
		pStaCfg->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = 1;
	}

	if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_20)
		pStaCfg->MlmeAux.HtCapability.MCSSet[4] = 0x0;  /* BW20 can't transmit MCS32*/

#ifdef TXBF_SUPPORT
{
	if (cap->FlgHwTxBfCap)
#ifdef MT_MAC
		mt_WrapSetETxBFCap(pAd, sta->wdev, &pStaCfg->MlmeAux.HtCapability.TxBFCap);
#endif /*MT_MAC*/
}
#endif /* TXBF_SUPPORT */
	COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCap, sta);
	return TRUE;
}
#endif

#if defined(CONFIG_STA_SUPPORT)
/* refined from RTMPCheckHt, will replace it when fully testing */
BOOLEAN check_ht(
	struct _RTMP_ADAPTER *ad,
	UINT16 wcid,
	struct common_ies *cmm_ies)
{
	MAC_TABLE_ENTRY *sta;
	struct _STA_ADMIN_CONFIG *sta_cfg = NULL;
	UCHAR cfg_ht_bw;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct _HT_CAPABILITY_IE *ht_cap = &cmm_ies->ht_cap;
	struct _ADD_HT_INFO_IE *ht_op = &cmm_ies->ht_op;

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return FALSE;

	sta = &ad->MacTab.Content[wcid];
	sta_cfg = GetStaCfgByWdev(ad, sta->wdev);
	ASSERT(sta_cfg);
	if (sta_cfg == NULL) {
		MTWF_DBG(ad, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "StaCfg NULL!! wcid %d entryType %d\n", wcid, sta->EntryType);
		if (sta->wdev)
			MTWF_DBG(ad, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wdev %s, idx %d\n", sta->wdev->if_dev->name, sta->wdev->wdev_idx);
		return FALSE;
	}

	/* Will check ChannelWidth for MCSSet[4] below */
	NdisZeroMemory(&sta_cfg->MlmeAux.HtCapability.MCSSet[0], 16);
	sta_cfg->MlmeAux.HtCapability.MCSSet[4] = 0x1;

	switch (wlan_operate_get_rx_stream(sta->wdev)) {
	case 4:
		sta_cfg->MlmeAux.HtCapability.MCSSet[3] = 0xff;
		/* fall through */
	case 3:
		sta_cfg->MlmeAux.HtCapability.MCSSet[2] = 0xff;
		/* fall through */
	case 2:
		sta_cfg->MlmeAux.HtCapability.MCSSet[1] = 0xff;
		/* fall through */
	case 1:
	default:
		sta_cfg->MlmeAux.HtCapability.MCSSet[0] = 0xff;
		break;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(sta->wdev);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = ht_op->AddHtInfo.RecomWidth & cfg_ht_bw;

	/*
		If both station and AP use 40MHz, still need to check if the 40MHZ band's legality in my country region
		If this 40MHz wideband is not allowed in my country list, use bandwidth 20MHZ instead,
	*/
	if (sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40) {
		if (RTMPCheckChannel(ad, sta_cfg->MlmeAux.CentralChannel, sta_cfg->MlmeAux.Channel, sta->wdev) == FALSE)
			sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;

		/*BW setting limited by EEPROM setting*/
		if (sta_cfg->MlmeAux.Channel > 14) { /* a-band*/
			if (ad->NicConfig2.field.BW40MAvailForA)  /* 1: OFF; 0: ON */
				sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
		} else {
			if (ad->NicConfig2.field.BW40MAvailForG)  /* 1: OFF; 0: ON */
				sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;

#ifdef COEX_SUPPORT
			if ((ad->BtCoexSupportMode & MT76xx_COEX_MODE_FDD) && (ad->BtSkipFDDFix20MH == 0))
				sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
#endif
		}
	}

	MTWF_DBG(ad, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			  "HtCapInfo.ChannelWidth=%d, RecomWidth=%d, Configure HT_BW=%d, BW40MAvailForA/G=%d/%d\n",
			  sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth, ht_op->AddHtInfo.RecomWidth,
			  cfg_ht_bw, ad->NicConfig2.field.BW40MAvailForA, ad->NicConfig2.field.BW40MAvailForG);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.GF = ht_cap->HtCapInfo.GF & wlan_config_get_greenfield(sta->wdev);
	/* Send Assoc Req with my HT capability.*/
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.AMsduSize = ht_cap->HtCapInfo.AMsduSize;
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.MimoPs =  wlan_config_get_mmps(sta->wdev);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 =
		wlan_config_get_ht_gi(sta->wdev) & (ht_cap->HtCapInfo.ShortGIfor20);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 =
		wlan_config_get_ht_gi(sta->wdev) & (ht_cap->HtCapInfo.ShortGIfor40);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.TxSTBC = wlan_config_get_ht_stbc(sta->wdev) & (ht_cap->HtCapInfo.RxSTBC);
	sta_cfg->MlmeAux.HtCapability.HtCapInfo.RxSTBC = wlan_config_get_ht_stbc(sta->wdev) & (ht_cap->HtCapInfo.TxSTBC);
	if (cap->phy_caps & fPHY_CAP_LDPC)
		sta_cfg->MlmeAux.HtCapability.HtCapInfo.ht_rx_ldpc = wlan_config_get_ht_ldpc(sta->wdev) & (ht_cap->HtCapInfo.ht_rx_ldpc);
	else
		sta_cfg->MlmeAux.HtCapability.HtCapInfo.ht_rx_ldpc = 0;

	sta_cfg->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = ht_cap->HtCapParm.MaxRAmpduFactor;
	sta_cfg->MlmeAux.HtCapability.HtCapParm.MpduDensity = ht_cap->HtCapParm.MpduDensity;
	sta_cfg->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = ht_cap->ExtHtCapInfo.PlusHTC;
	/* sta->HTCapability.ExtHtCapInfo.PlusHTC = pHtCap->ExtHtCapInfo.PlusHTC; */

	if (ad->CommonCfg.bRdg) {
		sta_cfg->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = ht_cap->ExtHtCapInfo.RDGSupport;
		sta_cfg->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = 1;
	}

	if (sta_cfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_20)
		sta_cfg->MlmeAux.HtCapability.MCSSet[4] = 0x0;  /* BW20 can't transmit MCS32 */

#ifdef TXBF_SUPPORT
{
	if (cap->FlgHwTxBfCap)
#ifdef MT_MAC
		mt_WrapSetETxBFCap(ad, sta->wdev, &sta_cfg->MlmeAux.HtCapability.TxBFCap);
#else
		setETxBFCap(ad, &sta_cfg->MlmeAux.HtCapability.TxBFCap);
#endif /*MT_MAC*/
}
#endif /* TXBF_SUPPORT */
	return TRUE;
}
#endif	/* CONFIG_STA_SUPPORT */

static INT build_bcm_ht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	const UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
	UCHAR HtLen;
	INT len = 0;
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	HtLen = SIZE_HT_CAP_IE + 4; /* including BCM IE */
	NdisMoveMemory(&HtCapabilityTmp, curr_ht_cap, SIZE_HT_CAP_IE);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

#ifdef TXBF_SUPPORT
	if (HcIsBfCapSupport(wdev) == FALSE) {
		UCHAR ucEBfCap;

		ucEBfCap = wlan_config_get_etxbf(wdev);
		wlan_config_set_etxbf(wdev, SUBF_OFF);
		mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
		wlan_config_set_etxbf(wdev, ucEBfCap);
	}
#endif /* TXBF_SUPPORT */

#ifdef RT_BIG_ENDIAN
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#endif /* RT_BIG_ENDIAN */

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &HtLen, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM, 4, len);
	MAKE_IE_TO_BUF(buf, &HtCapabilityTmp, SIZE_HT_CAP_IE, len);

	return len;
}

static INT build_bcm_ht_op_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;

	const UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};
	UCHAR AddHtLen = 0;
	ADD_HT_INFO_IE *addht;
#ifdef RT_BIG_ENDIAN
	ADD_HT_INFO_IE	addHTInfoTmp;
#endif /* RT_BIG_ENDIAN */
	UCHAR epigram_ie_len = 0;

	addht = wlan_operate_get_addht(wdev);
	AddHtLen = sizeof(ADD_HT_INFO_IE);
	epigram_ie_len = AddHtLen + 4;

	/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &epigram_ie_len, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM_AHTINFO, 4, len)
	MAKE_IE_TO_BUF(buf, &addHTInfoTmp, AddHtLen, len);
#else

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &epigram_ie_len, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM_AHTINFO, 4, len)
	MAKE_IE_TO_BUF(buf, addht, AddHtLen, len);
#endif /* RT_BIG_ENDIAN */

	return len;
}

static INT build_ht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 ReptMacTabWCID, UCHAR *buf)
{
	UCHAR HtLen = 0;
	INT len = 0;
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *curr_ht_cap = NULL;
	UCHAR cfg_ht_bw = 0;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg;
#endif /* CONFIG_STA_SUPPORT */

	if (wdev == NULL) {/*ALPS05330340*/
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null\n");
		return 0;
	}
	curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
	HtLen = sizeof(HT_CAPABILITY_IE);
	NdisMoveMemory(&HtCapabilityTmp, curr_ht_cap, HtLen);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

#ifdef CONFIG_STA_SUPPORT
	/*Set Green Field in APCLI Assoc Req HTCapIE,if AP's Beacon has GF 1*/
	if (wdev && (wdev->wdev_type == WDEV_TYPE_STA) && pStaCfg
		&& (pStaCfg->MlmeAux.HtCapability.HtCapInfo.GF == 1)) {
		HtCapabilityTmp.HtCapInfo.GF = 1;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Setting Green Field for Ap-Cli\n");
	} else {
		if (!pStaCfg)
			MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s : pStaCfg is NULL\n", __func__);
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef TXBF_SUPPORT
	if (wdev && (HcIsBfCapSupport(wdev) == FALSE)) {/*ALPS05330254*/
		UCHAR ucEBfCap;

		ucEBfCap = wlan_config_get_etxbf(wdev);
		wlan_config_set_etxbf(wdev, SUBF_OFF);
		mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
		wlan_config_set_etxbf(wdev, ucEBfCap);
	}
#ifdef MAC_REPEATER_SUPPORT
	else if (pAd->ApCfg.bMACRepeaterEn) {
		struct _RTMP_CHIP_CAP *cap;

		cap = hc_get_chip_cap(pAd->hdev_ctrl);
		 /* BFee function is limited if there is AID HW limitation*/
		if (cap->FlgHwTxBfCap & TXBF_AID_HW_LIMIT) {
			UCHAR ucEBfCap;

			ucEBfCap = wlan_config_get_etxbf(wdev);
			wlan_config_set_etxbf(wdev, SUBF_BFER);

			/* Just first cloned STA has full BF capability */
			if ((ReptMacTabWCID != 0) && (pAd->fgClonedStaWithBfeeSelected == FALSE)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[ReptMacTabWCID];

				if (pEntry == NULL) {
					MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "HT This entry isn't belong to cloned STA!!!===============\n");
				}

				if ((pEntry) && (HcIsBfCapSupport(pEntry->wdev) == TRUE)) {
						wlan_config_set_etxbf(wdev, SUBF_ALL);

					if (WMODE_HT_ONLY(wdev->PhyMode)) {
						pAd->fgClonedStaWithBfeeSelected = TRUE;
						pAd->ReptClonedStaEntry_CliIdx	 = pEntry->pReptCli->CliIdx;
					}
				}
			}

			mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
			wlan_config_set_etxbf(wdev, ucEBfCap);
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* TXBF_SUPPORT */

#ifdef RT_BIG_ENDIAN
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#endif /* RT_BIG_ENDIAN */

	MAKE_IE_TO_BUF(buf, &HtCapIe, 1, len);
	MAKE_IE_TO_BUF(buf, &HtLen, 1, len);
	MAKE_IE_TO_BUF(buf, &HtCapabilityTmp, HtLen, len);

	return len;
}

static INT build_ht_oper_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR AddHtLen = 0;
	ADD_HT_INFO_IE *addht;
#ifdef RT_BIG_ENDIAN
	ADD_HT_INFO_IE	addHTInfoTmp;
#endif /* RT_BIG_ENDIAN */

	addht = wlan_operate_get_addht(wdev);
	AddHtLen = sizeof(ADD_HT_INFO_IE);

	/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

	MAKE_IE_TO_BUF(buf, &AddHtInfoIe, 1, len);
	MAKE_IE_TO_BUF(buf, &AddHtLen, 1, len);
	MAKE_IE_TO_BUF(buf, &addHTInfoTmp, AddHtLen, len);
#else

	MAKE_IE_TO_BUF(buf, &AddHtInfoIe, 1, len);
	MAKE_IE_TO_BUF(buf, &AddHtLen, 1, len);
	MAKE_IE_TO_BUF(buf, addht, AddHtLen, len);
#endif /* RT_BIG_ENDIAN */

	return len;
}

static INT build_overlapping_bss_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel, UCHAR *buf)
{
	INT len = 0;
	UCHAR cfg_ht_bw = HT_BW_20;

#ifdef DOT11N_DRAFT3
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	/* P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE */
	if (WMODE_CAP_2G(wdev->PhyMode) && (cfg_ht_bw == HT_BW_40))	{
		OVERLAP_BSS_SCAN_IE  OverlapScanParam;
		UCHAR OverlapScanIE, ScanIELen;

		OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
		ScanIELen = 14;
		OverlapScanParam.ScanPassiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveDwell);
		OverlapScanParam.ScanActiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveDwell);
		OverlapScanParam.TriggerScanInt = cpu2le16(pAd->CommonCfg.Dot11BssWidthTriggerScanInt);
		OverlapScanParam.PassiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel);
		OverlapScanParam.ActiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel);
		OverlapScanParam.DelayFactor = cpu2le16(pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
		OverlapScanParam.ScanActThre = cpu2le16(pAd->CommonCfg.Dot11OBssScanActivityThre);

		MAKE_IE_TO_BUF(buf, &OverlapScanIE, 1, len);
		MAKE_IE_TO_BUF(buf, &ScanIELen, 1, len);
		MAKE_IE_TO_BUF(buf, &OverlapScanParam, ScanIELen, len);
	}
#endif /* DOT11N_DRAFT3 */

	return len;
}

INT build_ht_ies(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (WMODE_CAP_AX_6G(info->wdev->PhyMode))
		return len;

	if (info->is_draft_n_type == TRUE)
		len += build_bcm_ht_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else
		len += build_ht_cap_ie(pAd, info->wdev, info->ReptMacTabWCID, (UCHAR *)(info->frame_buf + len));

	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
		(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
		(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
		if (info->is_draft_n_type == TRUE)
			len += build_bcm_ht_op_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		else
			len += build_ht_oper_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	}

	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
		(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
		(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
		len += build_overlapping_bss_ie(pAd, info->wdev, info->channel, (UCHAR *)(info->frame_buf + len));
	}

	return len;
}


UINT8 *build_ht_caps_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

UINT8 *build_ht_op_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct ht_op_info ht_opinfo;
	UINT8 op_info_1 = 0;
	UINT32 op_info_2 = 0;

	ht_opinfo.field_1 = op_info_1;
	ht_opinfo.field_2 = cpu_to_le32(op_info_2);
	NdisMoveMemory(pos, (UINT8 *)&ht_opinfo, sizeof(ht_opinfo));
	pos += sizeof(ht_opinfo);

	return pos;
}

UINT8 *build_basic_ht_mcs_set(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct ht_support_mcs support_mcs;

	pos += sizeof(support_mcs);

	return pos;
}

UINT8 *build_ht_op_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 prim_ch = wdev->channel;

	/*primary channel*/
	NdisMoveMemory(pos, (UINT8 *)&prim_ch, sizeof(prim_ch));
	pos += sizeof(prim_ch);
	pos = build_ht_op_info(wdev, pos);
	pos = build_basic_ht_mcs_set(wdev, pos);

	return pos;
}
/*
 * Build up HT IEs for AP
 */
UINT32 add_beacon_ht_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *l_buf = f_buf;
	UINT32 offset = 0;

	l_buf = build_ht_caps_ie(wdev, l_buf);
	l_buf = build_ht_op_ie(wdev, l_buf);
	offset = (UINT32)(l_buf - f_buf);

	return offset;
}

VOID ie_field_value_decision(struct wifi_dev *wdev, BCN_IE_LIST *ie_list)
{
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		he_mac_cap_af_decision(wdev, ie_list->cmm_ies.he_caps.mac_cap.mac_capinfo_1);
	}
#endif /* DOT11_HE_AX */
}
