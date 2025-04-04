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
 ***************************************************************************

    Module Name:
    ap_wds.c

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
*/

#ifdef WDS_SUPPORT

#include "rt_config.h"


#define VAILD_KEY_INDEX(_X) ((((_X) >= 0) && ((_X) < 4)) ? (TRUE) : (FALSE))

static RTMP_STRING *WDS_MODE_STR[] = {"Disabled", "Restrict", "Bridge", "Repeater", "Lazy", "Unknown"};

UCHAR *wds_mode_2_str(UCHAR mode)
{
	if (mode > WDS_LAZY_MODE)
		mode = (WDS_LAZY_MODE+1);

	return WDS_MODE_STR[mode];
}

static inline BOOLEAN is_limited_by_security(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	return ((ad->CommonCfg.HT_DisallowTKIP) && IS_INVALID_HT_SECURITY(wdev->SecConfig.PairwiseCipher));
}

static USHORT get_wds_phymode(struct wifi_dev *wdev, UCHAR op_mode)
{
	USHORT wmode = 0;

	switch (op_mode) {
	case MODE_CCK:
		wmode = WMODE_B;
		break;

	case MODE_OFDM:
		/* should not have WMODE_B, to discuss */
		wmode = (WMODE_B | WMODE_G | WMODE_A);
		break;

	case MODE_HTMIX:
		wmode = (WMODE_B | WMODE_G | WMODE_A | WMODE_GN | WMODE_AN);
		break;

	case MODE_HTGREENFIELD:
		wmode = (WMODE_GN | WMODE_AN);
		break;

	case MODE_VHT:
		wmode = (WMODE_A | WMODE_AN | WMODE_AC);
		break;

	case MODE_HE:
		wmode = (WMODE_G | WMODE_A | WMODE_GN | WMODE_AN | WMODE_AC | WMODE_AX_24G | WMODE_AX_5G);
		break;

	default:
		wmode = wdev->PhyMode;
		break;
	}

	/* get the intersection between main wdev and peer */
	wmode &= wdev->PhyMode;

	/* in case the input parameter is illegal, use main wdev phymode */
	if (!wmode)
		wmode = wdev->PhyMode;

	return wmode;
}

BOOLEAN wds_entry_is_valid(
	struct _RTMP_ADAPTER *ad,
	UCHAR wds_index)
{
	BOOLEAN result = FALSE;
	struct _RT_802_11_WDS_ENTRY *wds_entry;
	struct _MAC_TABLE_ENTRY *peer;

	if (wds_index < MAX_WDS_ENTRY) {
		wds_entry = &ad->WdsTab.WdsEntry[wds_index];

		if (WDS_ENTRY_IS_VALID(wds_entry->flag)) {
			peer = wds_entry->peer;

			if (peer != NULL && IS_ENTRY_WDS(peer))
				result = TRUE;
		}
	}

	return result;
}

INT wds_fp_tx_pkt_allowed(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	INT allowed = FALSE;
	RT_802_11_WDS_ENTRY *wds_entry;
	UINT16 wcid = WCID_INVALID;
	UCHAR frag_nums;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (!wdev)
		return FALSE;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
		wds_entry = &pAd->WdsTab.WdsEntry[idx];

		if (wds_entry_is_valid(pAd, idx) && (wdev == (&wds_entry->wdev))) {
			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			wcid = wds_entry->peer->wcid;
			allowed = TRUE;
			break;
		}
	}

	if (!IS_WCID_VALID(pAd, wcid))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	/*  ethertype check is not offload to mcu for fragment frame*/
	if (frag_nums > 1) {
		if (!RTMPCheckEtherType(pAd, pkt, &tr_ctl->tr_entry[wcid], wdev)) {
			return FALSE;
		}
	}

	return allowed;
}

INT wds_tx_pkt_allowed(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	INT allowed = FALSE;
	RT_802_11_WDS_ENTRY *wds_entry;
	UINT16 wcid = WCID_INVALID;
	UCHAR frag_nums;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (!wdev)
		return FALSE;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
		wds_entry = &pAd->WdsTab.WdsEntry[idx];

		if (wds_entry_is_valid(pAd, idx) && (wdev == (&wds_entry->wdev))) {
			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			wcid = wds_entry->peer->wcid;
			allowed = TRUE;
			break;
		}
	}

	if (!IS_WCID_VALID(pAd, wcid))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	if (!RTMPCheckEtherType(pAd, pkt, &tr_ctl->tr_entry[wcid], wdev)) {
		return FALSE;
	}

	return allowed;
}

INT wds_rx_foward_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For WDS, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE;
}

INT WdsEntryAlloc(RTMP_ADAPTER *pAd, UCHAR band_idx, UCHAR *pAddr)
{
	INT i, WdsTabIdx = -1 , start_idx;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
			"%s(): "MACSTR", WdsMode = %d\n", __func__, MAC2STR(pAddr), pAd->WdsTab.Mode[band_idx]);
	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	start_idx = band_idx * MAX_WDS_PER_BAND;

	for (i = start_idx; i < MAX_WDS_ENTRY ; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];

		if ((pAd->WdsTab.Mode[band_idx] >= WDS_LAZY_MODE)) {
			if (WDS_IF_UP_CHECK(pAd, band_idx, i) &&
				!WDS_ENTRY_IS_ASSIGNED(wds_entry->flag) &&
				(band_idx == HcGetBandByWdev(&wds_entry->wdev))) {
				WDS_ENTRY_SET_ASSIGNED(wds_entry->flag);
				COPY_MAC_ADDR(wds_entry->PeerWdsAddr, pAddr);
				WdsTabIdx = i;
				break;
			}
		} else {
			if (!WDS_ENTRY_IS_ASSIGNED(wds_entry->flag)) {
				WDS_ENTRY_SET_ASSIGNED(wds_entry->flag);
				COPY_MAC_ADDR(wds_entry->PeerWdsAddr, pAddr);
				WdsTabIdx = i;
				break;
			} else if (MAC_ADDR_EQUAL(wds_entry->PeerWdsAddr, pAddr)) {
				WdsTabIdx = i;
				break;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);

	if (i == MAX_WDS_ENTRY)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s: Unable to allocate WdsEntry.\n", __func__);
	else
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "Alloced WdsEntry[%d]\n", i);

	return WdsTabIdx;
}

VOID WdsEntryDel(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): "MACSTR"\n", __func__,
			 MAC2STR(pAddr));

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];

		if (MAC_ADDR_EQUAL(pAddr, wds_entry->PeerWdsAddr) && WDS_ENTRY_IS_ASSIGNED(wds_entry->flag)) {
			WDS_ENTRY_SET_FLAG(wds_entry->flag, WDS_ENTRY_IS_INITED(wds_entry->flag));
			break;
		}
	}

	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
}

BOOLEAN wds_bss_linkdown(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid)
{
	MAC_TABLE_ENTRY *pEntry;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return FALSE;

	pEntry = &pAd->MacTab.Content[wcid];

	if (!pEntry || !pEntry->wdev) {
		ASSERT(FALSE);
		return FALSE;
	}

	/* check wdev is up ready */
	if (pEntry->wdev->if_up_down_state) {
		MlmeEnqueueWithWdev(pAd, WDS_STATE_MACHINE, WDS_BSS_LINKDOWN, sizeof(USHORT), &wcid, 0,
				pEntry->wdev);
		RTMP_MLME_HANDLER(pAd);
	} else
		ap_wds_wdev_linkdown(pAd, pEntry->wdev, wcid);

	return TRUE;
}

MAC_TABLE_ENTRY *MacTableInsertWDSEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	UINT WdsTabIdx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	HTTRANSMIT_SETTING HTPhyMode;
	RT_802_11_WDS_ENTRY *wds_entry;
	struct wifi_dev *wdev;
	struct legacy_rate *rate;
	HT_CAPABILITY_IE *ht_cap = NULL;
	UCHAR phy_mode;
	BOOLEAN has_ht_cap = FALSE, has_vht_cap = FALSE;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): WdsTabIdx = %d, Addr "MACSTR"\n", __func__,
			 WdsTabIdx, MAC2STR(pAddr));

	/* if FULL, return */
	if (pAd->MacTab.Size >= GET_MAX_UCAST_NUM(pAd))
		return NULL;

	wds_entry = &pAd->WdsTab.WdsEntry[WdsTabIdx];
	wdev = &wds_entry->wdev;
	pEntry = WdsTableLookup(pAd, pAddr, TRUE);
	if ((pEntry != NULL) && wdev->DevInfo.WdevActive)
		return pEntry;

	rate = &wdev->rate.legacy_rate;
	phy_mode = wds_entry->phy_mode;

	do {
		/* allocate one MAC entry */
		pEntry = MacTableInsertEntry(pAd, pAddr, wdev, ENTRY_WDS, OPMODE_AP, TRUE);

		if (pEntry) {
			tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			tr_entry->OmacIdx = wdev->OmacIdx;
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "  OmacIdx = %d, wcid = %d, phymode = 0x%x\n",
					 tr_entry->OmacIdx, pEntry->wcid, wds_entry->phy_mode);

			/* specific Max Tx Rate for Wds link. */
			NdisZeroMemory(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));
			
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "phy_mode=0x%x\n", phy_mode);

			if (WMODE_EQUAL(phy_mode, WMODE_B)) {
				HTPhyMode.field.MODE = MODE_CCK;
				HTPhyMode.field.MCS = 3;
				pEntry->RateLen = 4;
			} else if (WMODE_EQUAL(phy_mode, WMODE_GN | WMODE_AN)) {
				HTPhyMode.field.MODE = MODE_HTGREENFIELD;
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
			} else if (WMODE_CAP_AX(phy_mode)) {
				/* to check */
				HTPhyMode.field.MODE = MODE_HE;
				HTPhyMode.field.MCS = 11;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
			} else if (WMODE_CAP_AC(phy_mode)) {
				HTPhyMode.field.MODE = MODE_VHT;
				HTPhyMode.field.MCS = 9;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
			} else if (WMODE_CAP_N(phy_mode)) {
				HTPhyMode.field.MODE = MODE_HTMIX;
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
			} else {
				HTPhyMode.field.MODE = MODE_OFDM;
				HTPhyMode.field.MCS = 7;
				pEntry->RateLen = 8;
			}

			pEntry->MaxHTPhyMode.word = HTPhyMode.word;
			pEntry->MinHTPhyMode.word = wdev->MinHTPhyMode.word;
			pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(phy_mode)) {
				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "IF-wds%d : Desired MCS = %d\n", WdsTabIdx,
							 wdev->DesiredTransmitSetting.field.MCS);
					set_ht_fixed_mcs(pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pEntry->MmpsMode = MMPS_DISABLE;
				ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
				NdisMoveMemory(&pEntry->HTCapability, ht_cap, sizeof(HT_CAPABILITY_IE));
				has_ht_cap = TRUE;

#ifdef TXBF_SUPPORT
				if (HcIsBfCapSupport(wdev) == FALSE) {
					UCHAR ucEBfCap;

					ucEBfCap = wlan_config_get_etxbf(wdev);
					wlan_config_set_etxbf(wdev, SUBF_OFF);
					mt_WrapSetETxBFCap(pAd, wdev, &pEntry->HTCapability.TxBFCap);
					wlan_config_set_etxbf(wdev, ucEBfCap);
				}
#endif

				ht_mode_adjust(pAd, pEntry, &pEntry->HTCapability);
				set_sta_ht_cap(pAd, pEntry, &pEntry->HTCapability);
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
						"  Cap.MaxRAmpduFactor = %d, Cap.MpduDensity = %d\n",
						 pEntry->HTCapability.HtCapParm.MaxRAmpduFactor,
						 pEntry->HTCapability.HtCapParm.MpduDensity);
			} else
				NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(phy_mode) && WMODE_CAP_5G(wdev->PhyMode)) {
				VHT_CAP_IE vht_cap;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
				ucETxBfCap = wlan_config_get_etxbf(wdev);

				if (HcIsBfCapSupport(wdev) == FALSE)
					wlan_config_set_etxbf(wdev, SUBF_OFF);
#endif
				build_vht_cap_ie(pAd, wdev, (UCHAR *)&vht_cap);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
				wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif
				vht_mode_adjust(pAd, pEntry, &vht_cap, NULL, NULL, NULL);
				dot11_vht_mcs_to_internal_mcs(pAd, wdev, &vht_cap, &pEntry->MaxHTPhyMode);
				set_vht_cap(pAd, pEntry, &vht_cap);
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n",
							__func__,
							get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
							get_bw_str(pEntry->MaxHTPhyMode.field.BW),
							pEntry->MaxHTPhyMode.field.MCS,
							pEntry->MaxHTPhyMode.word);
				NdisMoveMemory(&pEntry->vht_cap_ie, &vht_cap, sizeof(VHT_CAP_IE));
				has_vht_cap = TRUE;
				assoc_vht_info_debugshow(pAd, pEntry, &vht_cap, NULL);
			} else
				NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));

			pEntry->force_op_mode = FALSE;
#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
			if (WMODE_CAP_AX(phy_mode)) {
				struct he_ies he_ie;
				int i;

				NdisZeroMemory(&he_ie, sizeof(he_ie));
				get_own_he_ie(wdev, &he_ie);
				update_peer_he_params(pEntry, &he_ie);
				he_mode_adjust(pEntry->wdev, pEntry, NULL);
				for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
					if(pEntry->cap.rate.he80_rx_nss_mcs[i] == 3)
						break ;
				}
				if(i != 0) {
					i = i <= wlan_operate_get_tx_stream(wdev) ? i : wlan_operate_get_tx_stream(wdev);
					pEntry->MaxHTPhyMode.field.MCS = ((i-1) << 4);
				} else {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN,
					"%s(): STA antenna information provided is incorrect", __func__);
				}

				if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 2)
					pEntry->MaxHTPhyMode.field.MCS += HE_MCS_11 ;
				else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 1)
					pEntry->MaxHTPhyMode.field.MCS += HE_MCS_9 ;
				else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 0)
					pEntry->MaxHTPhyMode.field.MCS += HE_MCS_7;
				else
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN,
					"%s(): STA MCS information provided is incorrect", __func__);
			}
#endif

			RTMPSetSupportMCS(pAd,
							  OPMODE_AP,
							  pEntry,
							  rate,
#ifdef DOT11_VHT_AC
							  has_vht_cap,
							  &pEntry->vht_cap_ie,
#endif
							  &pEntry->HTCapability,
							  has_ht_cap);

			/* for now, we set this by default! */
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			if (wdev->bAutoTxRateSwitch == FALSE) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS;
				pEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
			} else {
				/* TODO: shiang-MT7603, fix me for this, because we may need to set this only when we have WTBL entry for tx_rate! */
				pEntry->bAutoTxRateSwitch = TRUE;
			}

			wds_entry->peer = pEntry;
			pEntry->func_tb_idx = WdsTabIdx;
			pEntry->wdev = wdev;
			COPY_MAC_ADDR(&wdev->bssid[0], &pEntry->bssid);
			/* update per wdev bw */
			wlan_operate_set_ht_bw(wdev, wdev->MaxHTPhyMode.field.BW, wlan_operate_get_ext_cha(wdev));

			if (wdev_do_linkup(wdev, pEntry) != TRUE)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN, "%s(): linkup fail!!\n", __func__);

			if (wdev_do_conn_act(wdev, pEntry) != TRUE)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN, "%s(): connect fail!!\n", __func__);

			AsicUpdateWdsEncryption(pAd, pEntry->wcid);

			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "PhyMode = 0x%x, Channel = %d\n",
					 wdev->PhyMode, wdev->channel);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s() - allocate entry #%d(link to WCID %d)\n",
					 __func__, WdsTabIdx, pEntry->wcid);
			break;
		}
	} while (FALSE);

	return pEntry;
}

MAC_TABLE_ENTRY *WdsTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount)
{
	ULONG WdsIndex;
	MAC_TABLE_ENTRY *pCurEntry = NULL, *pEntry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);

	do {
		pCurEntry = &pAd->MacTab.Content[wcid];
		WdsIndex = 0xff;

		if ((pCurEntry) && IS_ENTRY_WDS(pCurEntry))
			WdsIndex = pCurEntry->func_tb_idx;

		if (WdsIndex == 0xff)
			break;

		if (!WDS_ENTRY_IS_ASSIGNED(pAd->WdsTab.WdsEntry[WdsIndex].flag))
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddr)) {
			if (bResetIdelCount) {
				pCurEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
				tr_ctl->tr_entry[pCurEntry->tr_tb_idx].NoDataIdleCount = 0;
			}

			pEntry = pCurEntry;
			break;
		}
	} while (FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}

MAC_TABLE_ENTRY *WdsTableLookup(RTMP_ADAPTER *pAd, UCHAR *addr, BOOLEAN bResetIdelCount)
{
	USHORT HashIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);
	HashIdx = MAC_ADDR_HASH_INDEX(addr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry) {
		if (IS_ENTRY_WDS(pEntry) && MAC_ADDR_EQUAL(pEntry->Addr, addr)) {
			if (bResetIdelCount) {
				pEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
				tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
			}

			break;
		}
		pEntry = pEntry->pNext;
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}

struct wifi_dev *wdev_search_by_address_for_WDS(RTMP_ADAPTER *pAd, UCHAR *address)
{
	UINT16 Index;
	struct wifi_dev *wdev;
	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < MAX_WDS_ENTRY; Index++) {
		wdev = &pAd->WdsTab.WdsEntry[Index].wdev;

		if (MAC_ADDR_EQUAL(address, wdev->if_addr)) {
			NdisReleaseSpinLock(&pAd->WdevListLock);
			return wdev;
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s: can not find registered wdev\n", __func__);
	return NULL;
}

MAC_TABLE_ENTRY *FindWdsEntry(
	IN RTMP_ADAPTER *pAd,
	IN struct _RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pAd->WdsTab.Mode[pRxBlk->band]) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): Wcid = %d, PhyMode = 0x%x\n", __func__,
			pRxBlk->wcid, pRxBlk->rx_rate.field.MODE);
		/* lookup the match wds entry for the incoming packet. */
		pEntry = WdsTableLookupByWcid(pAd, pRxBlk->wcid, pRxBlk->Addr2, TRUE);

		if (pEntry == NULL)
			pEntry = WdsTableLookup(pAd, pRxBlk->Addr2, TRUE);

		/*  Report to MLME, add WDS entry */
		if ((pEntry == NULL) && (pAd->WdsTab.Mode[pRxBlk->band] >= WDS_LAZY_MODE)) {
			UCHAR *pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			/*struct wifi_dev *wdev = wdev_search_by_address(pAd, pRxBlk->Addr1);*/
			struct wifi_dev *wdev = wdev_search_by_address_for_WDS(pAd, pRxBlk->Addr1);

			if (!wdev) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
					 "No matched wdev ("MACSTR"), wcid = %d!!!\n",
					 MAC2STR(pRxBlk->Addr1), pRxBlk->wcid);
				return NULL;
			}

			NdisMoveMemory(pTmpBuf, pRxBlk->FC, LENGTH_802_11);

			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  pTmpBuf,
						  pRxBlk->DataSize + LENGTH_802_11,
						  pRxBlk->rx_signal.raw_rssi[0],
						  pRxBlk->rx_signal.raw_rssi[1],
						  pRxBlk->rx_signal.raw_rssi[2],
						  pRxBlk->rx_signal.raw_rssi[3],
						  pRxBlk->channel_freq,
						  0,
						  OPMODE_AP,
						  wdev,
						  pRxBlk->rx_rate.field.MODE);

			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				 "!!! report WDS UC DATA (from "MACSTR") to MLME (len=%d) !!!\n",
				 MAC2STR(pRxBlk->Addr2), pRxBlk->DataSize);
		} else if (pEntry) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				 "%s[band%d]A2("MACSTR") found as wcid:%d!!! (Drop)\n", __func__, pRxBlk->band,
				 MAC2STR(pRxBlk->Addr2), pEntry->wcid);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
			 "!!! Recv WDS UC DATA (from "MACSTR") but WDS for band%d is not enabled !!! (Drop)\n",
			 MAC2STR(pRxBlk->Addr2), pRxBlk->band);
	}

	return pEntry;
}

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any WDS client being idle for too long and should be aged-out from MAC table
	==========================================================================
*/
VOID WdsTableMaintenance(RTMP_ADAPTER *pAd, UCHAR band_idx)
{
	UCHAR idx;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _MAC_TABLE_ENTRY *peer;

	if (pAd->WdsTab.Mode[band_idx] != WDS_LAZY_MODE)
		return;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
		if (wds_entry_is_valid(pAd, idx) == FALSE)
			continue;

		peer = pAd->WdsTab.WdsEntry[idx].peer;

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): Entry[%d], Wcid = %d, "MACSTR"\n",
				 __func__, idx, peer->wcid, MAC2STR(peer->Addr));

		NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
		NdisAcquireSpinLock(&pAd->MacTabLock);
		peer->NoDataIdleCount++;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		tr_ctl->tr_entry[peer->wcid].NoDataIdleCount++;
		NdisReleaseSpinLock(&pAd->MacTabLock);
		NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);

		/* delete those MAC entry that has been idle for a long time */
		if (peer->NoDataIdleCount >= MAC_TABLE_AGEOUT_TIME) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "ageout "MACSTR" from WDS #%d after %d-sec silence\n",
					 MAC2STR(peer->Addr), idx, MAC_TABLE_AGEOUT_TIME);
			WdsEntryDel(pAd, peer->Addr);
			wds_bss_linkdown(pAd, peer->wcid);
		}
	}
}

#define RALINK_PASSPHRASE	"Ralink"
VOID AsicUpdateWdsEncryption(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct _MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RT_802_11_WDS_ENTRY *wds_entry;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ASIC_SEC_INFO Info = {0};

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = &pAd->MacTab.Content[wcid];

	wds_entry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

	if (!WDS_ENTRY_IS_ASSIGNED(wds_entry->flag))
		return;

	if (!IS_ENTRY_WDS(pEntry))
		return;

	wdev = &wds_entry->wdev;
	pSecConfig = &wdev->SecConfig;

	if (pSecConfig->AKMMap == 0x0)
		SET_AKM_OPEN(pSecConfig->AKMMap);

	if (pSecConfig->PairwiseCipher == 0x0)
		SET_CIPHER_NONE(pSecConfig->PairwiseCipher);

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = wcid;
	Info.BssIndex = wdev->bss_info_argument.ucBssIndex;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	/* When WEP, TKIP or AES is enabled, set group key info to Asic */
	if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher)) {
		os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
	} else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
		/* Calculate Key */
		SetWPAPSKKey(pAd, pSecConfig->PSK, strlen(pSecConfig->PSK), (PUCHAR) RALINK_PASSPHRASE, sizeof(RALINK_PASSPHRASE), pSecConfig->PMK);
		os_move_mem(Info.Key.Key, pSecConfig->PMK, LEN_PMK);

		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			/*WDS: RxMic/TxMic use the same value */
			os_move_mem(&Info.Key.Key[LEN_TK + LEN_TKIP_MIC], &Info.Key.Key[LEN_TK], LEN_TKIP_MIC);
		}

		WPAInstallKey(pAd, &Info, TRUE, TRUE);
	}
}

static USHORT WdsGetPeerSuppPhyModeLegacy(
	struct wifi_dev *wdev,
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN UCHAR Channel)
{
	USHORT PeerPhyModeLegacy = 0;
	INT i;

	if ((SupRateLen > 0) && (SupRateLen < MAX_LEN_OF_SUPPORTED_RATES)) {
		for (i = 0; i < SupRateLen; i++) {
			/* CCK Rates: 1, 2, 5.5, 11 */
			if (((SupRate[i] & 0x7f) == 0x2) || ((SupRate[i] & 0x7f) == 0x4) ||
				((SupRate[i] & 0x7f) == 0xb) || ((SupRate[i] & 0x7f) == 0x16))
				PeerPhyModeLegacy |= WMODE_B;
			/* OFDM Rates: 6, 9, 12, 18, 24, 36, 48, 54 */
			else if (((SupRate[i] & 0x7f) == 0xc) || ((SupRate[i] & 0x7f) == 0x12) ||
					 ((SupRate[i] & 0x7f) == 0x18) || ((SupRate[i] & 0x7f) == 0x24) ||
					 ((SupRate[i] & 0x7f) == 0x30) || ((SupRate[i] & 0x7f) == 0x48) ||
					 ((SupRate[i] & 0x7f) == 0x60) || ((SupRate[i] & 0x7f) == 0x6c)) {
				if (WMODE_CAP_5G(wdev->PhyMode))
					PeerPhyModeLegacy |= WMODE_A;
				else
					PeerPhyModeLegacy |= WMODE_G;
			}
		}
	}

	return PeerPhyModeLegacy;
}

USHORT WdsGetPeerSuppPhyMode(
	struct wifi_dev *wdev,
	IN BCN_IE_LIST * ie_list)
{
	USHORT PeerPhyMode = 0;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		if (WMODE_CAP_6G(wdev->PhyMode))
			PeerPhyMode |= WMODE_AX_6G;
		else if (WMODE_CAP_5G(wdev->PhyMode))
			PeerPhyMode |= WMODE_AX_5G;
		else
			PeerPhyMode |= WMODE_AX_24G;
	}
#endif

#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_5G(wdev->PhyMode))
		PeerPhyMode |= WMODE_AC;
#endif

	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		if (WMODE_CAP_5G(wdev->PhyMode))
			PeerPhyMode |= WMODE_AN;
		else
			PeerPhyMode |= WMODE_GN;
	}
#endif

	/* Check OFDM/CCK capability */
	PeerPhyMode |= WdsGetPeerSuppPhyModeLegacy(wdev, rate->sup_rate, rate->sup_rate_len, ie_list->Channel);
	MTWF_DBG((struct _RTMP_ADAPTER *)wdev->sys_handle, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG, "%s(): "MACSTR", PeerPhyMode = 0x%x\n", __func__,
			 MAC2STR(ie_list->Addr2), PeerPhyMode);

	if (((ie_list->Channel > 0) && (ie_list->Channel <= 14) && WMODE_CAP_5G(PeerPhyMode)) ||
		((ie_list->Channel > 14) && WMODE_CAP_2G(PeerPhyMode)))
		MTWF_DBG((struct _RTMP_ADAPTER *)wdev->sys_handle, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN, "ERROR!! Wrong PeerPhyMode 0x%x, Channel %d\n", PeerPhyMode, ie_list->Channel);

	return PeerPhyMode;
}

VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BCN_IE_LIST * ie_list)
{
	UCHAR MaxSupportedRate;
	USHORT PeerPhyMode;
	BOOLEAN bRaReInit = FALSE;
	RT_802_11_WDS_ENTRY *pWdsEntry = NULL;
	struct wifi_dev *wdev, *main_wdev;
	USHORT CapabilityInfo = 0, cmm_phy_mode, OriWdevPhyMode;
	struct _vendor_ie_cap *vendor_ie = NULL;
	HT_CAPABILITY_IE *ht_cap = NULL;
#ifdef DOT11_VHT_AC
	VHT_CAP_IE *vht_cap = NULL;
#endif
#ifdef DOT11_HE_AX
	struct he_cap_ie *he_cap = NULL;
#endif
	struct legacy_rate *rate;
#ifdef DOT11_N_SUPPORT
	UCHAR new_bw = HT_BW_20;
#endif
	struct common_ies *cmm_ies;
	struct WIFI_SYS_CTRL wsys;
	struct _BSS_INFO_ARGUMENT_T *bss = &wsys.BssInfoCtrl;
	BOOLEAN bReInitPhyMode = FALSE;
	int i;

	RETURN_IF_PAD_NULL(pAd);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Invalid pEntry!");
		return;
	}

	if (!IS_ENTRY_WDS(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Invalid WDS pEntry!");
		return;
	}

	wdev = pEntry->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Invalid wdev!");
		return;
	}

	pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

	if (!pWdsEntry) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Invalid WDS entry!");
		return;
	}

	if (!WDS_ENTRY_IS_VALID(pWdsEntry->flag)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "WDS entry not ready!");
		return;
	}

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode && (HcGetBandByWdev(wdev) == DBDC_BAND1)) {
		main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
	} else
#endif	/* DBDC_MODE */
		main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

	if (ie_list) {
		cmm_ies = &ie_list->cmm_ies;
		CapabilityInfo = ie_list->CapabilityInfo;
		vendor_ie = &ie_list->cmm_ies.vendor_ie;
		ht_cap = &ie_list->cmm_ies.ht_cap;
#ifdef DOT11_VHT_AC
		vht_cap = &ie_list->cmm_ies.vht_cap;
#endif
#ifdef DOT11_HE_AX
		he_cap = &ie_list->cmm_ies.he_caps;
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Invalid ie_list!");
		return;
	}

	PeerPhyMode = WdsGetPeerSuppPhyMode(wdev, ie_list);
	cmm_phy_mode = wdev->PhyMode & PeerPhyMode;
	if (is_limited_by_security(pAd, wdev))
		cmm_phy_mode &= (WMODE_A | WMODE_B | WMODE_G);

	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG,
			"%s(): wcid=%d, max supported rate = %d, support caps. = 0x%x, peer phy mode = 0x%x\n", __func__,
			pEntry->wcid, MaxSupportedRate, cmm_ies->ie_exists, PeerPhyMode);

	pEntry->MaxSupportedRate = min(pEntry->wdev->rate.MaxTxRate, MaxSupportedRate);
	pEntry->RateLen = MaxSupportedRateLen;

	/* Update PhyMode if Peer PhyMode gets updated */
	if ((pWdsEntry->wdev.PhyMode != PeerPhyMode) && (PeerPhyMode != WMODE_INVALID)) {
			OriWdevPhyMode = pWdsEntry->wdev.PhyMode;
			bReInitPhyMode = TRUE;
	}

	if (WMODE_EQUAL(cmm_phy_mode, WMODE_B) && WMODE_CAP(pWdsEntry->phy_mode, WMODE_B)) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = 3;
		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->RateLen = 4;
	} else if (WMODE_CAP(cmm_phy_mode, WMODE_G | WMODE_A) && WMODE_CAP(pWdsEntry->phy_mode, WMODE_G | WMODE_A)) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = 7;
		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->RateLen = 8;
	}

	/* common parts */
	pEntry->CapabilityInfo = CapabilityInfo;
	MacTableSetEntryRaCap(pAd, pEntry, vendor_ie);
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

#ifdef DOT11_N_SUPPORT
	if ((WMODE_CAP_N(cmm_phy_mode) && WMODE_CAP_N(pWdsEntry->phy_mode))
		&& HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
			/* both ap and peer support BW40 */
			if (wlan_operate_get_ht_bw(main_wdev) == HT_BW_40 &&
				ht_cap->HtCapInfo.ChannelWidth == HT_BW_40) {
				new_bw = HT_BW_40;
			}

			if (wlan_operate_get_ht_bw(wdev) != new_bw) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s(): set new_bw = %d\n", __func__, new_bw);
				wlan_operate_set_ht_bw(wdev, new_bw, wlan_operate_get_ext_cha(wdev));
			}

			ht_mode_adjust(pAd, pEntry, ht_cap);
			pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(&wdev->DesiredHtPhyInfo.MCSSet[0], &ht_cap->MCSSet[0]);

			set_sta_ht_cap(pAd, pEntry, ht_cap);
			NdisMoveMemory(&pEntry->HTCapability, ht_cap, sizeof(HT_CAPABILITY_IE));
		}
	} else {
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	if ((WMODE_CAP_AC(cmm_phy_mode) && WMODE_CAP_AC(pWdsEntry->phy_mode))
		&& HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_5G(wdev->PhyMode)) {
		RT_PHY_INFO *pDesired_ht_phy = &pEntry->wdev->DesiredHtPhyInfo;

		/* Set desired phy to VHT mode */
		if (pDesired_ht_phy && !pDesired_ht_phy->bVhtEnable) {
			pDesired_ht_phy->bVhtEnable = TRUE;
			rtmp_set_vht(pAd, wdev, pDesired_ht_phy);
		}

		vht_mode_adjust(pAd, pEntry, vht_cap, NULL, NULL, NULL);
		dot11_vht_mcs_to_internal_mcs(pAd, wdev, vht_cap, &pEntry->MaxHTPhyMode);
		set_vht_cap(pAd, pEntry, vht_cap);
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG, "%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n", __func__,
					get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
					get_bw_str(pEntry->MaxHTPhyMode.field.BW),
					pEntry->MaxHTPhyMode.field.MCS,
					pEntry->MaxHTPhyMode.word);
		NdisMoveMemory(&pEntry->vht_cap_ie, vht_cap, sizeof(VHT_CAP_IE));

		if (DebugLevel >= DBG_LVL_DEBUG)
			assoc_vht_info_debugshow(pAd, pEntry, vht_cap, NULL);
	} else
		NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));

	pEntry->force_op_mode = FALSE;
#endif /* DOT11_VHT_AC */

	/* Re-Issue BssInfo Cmd for Phy mode Case */
	if (bReInitPhyMode && (pWdsEntry->wdev.PhyMode != OriWdevPhyMode)) {
		/* Issue EXT_CMD_ID_BSSINFO_UPDATE to N9 FW with BSS_INFO_BASIC feature to*/
		/* update Phy Mode configured at the time of Initialization */

		os_zero_mem(&wsys, sizeof(wsys));

		/*prepare bssinfo*/
		BssInfoArgumentLink(pEntry->wdev->sys_handle, pEntry->wdev, bss);
		bss->bss_state = pEntry->wdev->bss_info_argument.bss_state;

		/* Overwrite PhyMode for existing BssIndex */
		bss->ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
		bss->peer_wlan_idx = pEntry->wcid;
		bss->u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;
		bss->bmc_wlan_idx = pEntry->wcid;
		wsys.wdev = pEntry->wdev;

		/* Enque BSS Info Update Cmd to N9 */
		HW_WIFISYS_LINKUP(pEntry->wdev->sys_handle, &wsys);
	}

#ifdef DOT11_HE_AX
	if ((WMODE_CAP_AX(cmm_phy_mode) && WMODE_CAP_AX(pWdsEntry->phy_mode))
		&& HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		update_peer_he_caps(pEntry, cmm_ies);
		update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(pEntry->wdev, pEntry, NULL);
		for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
			if(pEntry->cap.rate.he80_rx_nss_mcs[i] == 3)
				break ;
		}
		if(i != 0) {
			i = i <= wlan_operate_get_tx_stream(wdev) ? i : wlan_operate_get_tx_stream(wdev);
			pEntry->MaxHTPhyMode.field.MCS = ((i-1) << 4);
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN,
			"%s(): STA antenna information provided is incorrect", __func__);
		}

		if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 2)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_11 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 1)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_9 ;
		else if(pEntry->cap.rate.he80_rx_nss_mcs[0] == 0)
			pEntry->MaxHTPhyMode.field.MCS += HE_MCS_7;
		else
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN,
			"%s(): STA MCS information provided is incorrect", __func__);
	}
#endif

	rate = &wdev->rate.legacy_rate;
	RTMPSetSupportMCS(pAd,
					  OPMODE_AP,
					  pEntry,
					  rate,
#ifdef DOT11_VHT_AC
					  HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists),
					  vht_cap,
#endif
					  ht_cap,
					  HAS_HT_CAPS_EXIST(cmm_ies->ie_exists));

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG, " Mode %d, MCS %d, STBC %d SGI %d, BW %d / MAX Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
			 pEntry->HTPhyMode.field.MODE,
			 pEntry->HTPhyMode.field.MCS,
			 pEntry->HTPhyMode.field.STBC,
			 pEntry->HTPhyMode.field.ShortGI,
			 pEntry->HTPhyMode.field.BW,
			 pEntry->MaxHTPhyMode.field.MODE,
			 pEntry->MaxHTPhyMode.field.MCS,
			 pEntry->MaxHTPhyMode.field.STBC,
			 pEntry->MaxHTPhyMode.field.ShortGI,
			 pEntry->MaxHTPhyMode.field.BW);

	if ((pEntry->HTPhyMode.field.MODE != pEntry->MaxHTPhyMode.field.MODE) ||
		(pEntry->HTPhyMode.field.STBC != pEntry->MaxHTPhyMode.field.STBC) ||
		(pEntry->HTPhyMode.field.ShortGI != pEntry->MaxHTPhyMode.field.ShortGI) ||
		(pEntry->HTPhyMode.field.BW != pEntry->MaxHTPhyMode.field.BW) ||
		(pEntry->HTPhyMode.field.MCS != pEntry->MaxHTPhyMode.field.MCS)) {
		pEntry->HTPhyMode.field.MODE = pEntry->MaxHTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.STBC = pEntry->MaxHTPhyMode.field.STBC;
		pEntry->HTPhyMode.field.ShortGI = pEntry->MaxHTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.BW = pEntry->MaxHTPhyMode.field.BW;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxHTPhyMode.field.MCS;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, " bRaReInit, New Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
				 pEntry->HTPhyMode.field.MODE,
				 pEntry->HTPhyMode.field.MCS,
				 pEntry->HTPhyMode.field.STBC,
				 pEntry->HTPhyMode.field.ShortGI,
				 pEntry->HTPhyMode.field.BW);
		bRaReInit = TRUE;
	}

	if (bRaReInit && pEntry->bAutoTxRateSwitch == TRUE) {
		/* StaRec update */
		wifi_sys_update_wds(pAd, pEntry);
		AsicUpdateWdsEncryption(pAd, pEntry->wcid);
		RAInit(pAd, pEntry);
	}

	WDS_ENTRY_SET_SYNCED(pWdsEntry->flag);
}

VOID APWdsInitialize(RTMP_ADAPTER *pAd)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	pAd->WdsTab.Mode[DBDC_BAND0] = WDS_DISABLE_MODE;
#ifdef DBDC_MODE
	pAd->WdsTab.Mode[DBDC_BAND1] = WDS_DISABLE_MODE;
#endif

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];
		wds_entry->phy_mode = 0;
		wds_entry->wdev.bAutoTxRateSwitch = TRUE;
		wds_entry->wdev.DesiredTransmitSetting.field.MCS = MCS_AUTO;
		WDS_ENTRY_SET_FLAG(wds_entry->flag, WDS_ENTRY_IS_INITED(wds_entry->flag));
		wds_entry->peer = NULL;
		wds_entry->KeyIdx = 0;
		NdisZeroMemory(&wds_entry->WdsKey, sizeof(CIPHER_KEY));
		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
	}
	NdisAllocateSpinLock(pAd, &pAd->WdsTab.WdsTabLock);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
		 "%s: WdsEntry[0~%d]\n", __func__, MAX_WDS_ENTRY-1);
}

INT Show_WdsTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	struct _RT_802_11_WDS_ENTRY *wds_entry;
	CHAR *wmode;

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		if(i == 0 || i == MAX_WDS_PER_BAND) {
			INT band = i ? DBDC_BAND1 : DBDC_BAND0;
			MTWF_PRINT("\n[band%d]WDS mode:%s count:%d\n############## WDS List ##############\n",
					band, wds_mode_2_str(pAd->WdsTab.Mode[band]), pAd->WdsTab.wds_num[band]);
		}
		wds_entry = &pAd->WdsTab.WdsEntry[i];
		wmode = wmode_2_str(wds_entry->phy_mode);
		if (!wmode)
			continue;

		if (HcIsRadioAcq(&wds_entry->wdev)) {
			struct _SECURITY_CONFIG *p_sec_con = &wds_entry->wdev.SecConfig;

			MTWF_PRINT("IF/WDS%d(band%d)-"MACSTR"(%s,%s,%s,%s), OpState=%d \n\t\t\twmode:%s Cipher=%s, KeyId=%d\n",
				  i, HcGetBandByWdev(&wds_entry->wdev),
				  MAC2STR(wds_entry->PeerWdsAddr),
				  WDS_ENTRY_IS_INITED(wds_entry->flag) ? "initialized" : "not initialized",
				  WDS_ENTRY_IS_ASSIGNED(wds_entry->flag) ? "Assigned" : "Unassigned",
				  WDS_ENTRY_IS_SYNCED(wds_entry->flag) ? "SYNCED" : "NO SYNC",
				  WDS_ENTRY_IS_VALID(wds_entry->flag) ? "Valid" : "Invalid",
				  wlan_operate_get_state(&wds_entry->wdev),
				  wmode,
				  GetEncryModeStr(p_sec_con->PairwiseCipher),
				  wds_entry->wdev.SecConfig.PairwiseKeyId);

			if (!IS_CIPHER_NONE(p_sec_con->PairwiseCipher))  {
				UCHAR *key_str = NULL;

				if (IS_CIPHER_WEP(p_sec_con->PairwiseCipher))
					key_str = p_sec_con->WepKey[p_sec_con->PairwiseKeyId].Key;
				else
					key_str = p_sec_con->PSK;

				MTWF_PRINT("\t\t\t\tWds Key:%s (%d)\n",
						  key_str, (UINT32)strlen(key_str));
			}
		} else
			MTWF_PRINT("IF/WDS%d not occupied\n", i);

		os_free_mem(wmode);
	}

	MTWF_PRINT("\n%-19s%-4s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
			 "MAC", "IDX", "AID", "PSM", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_WDS(pEntry)) {
			MTWF_PRINT(""MACSTR"  ", MAC2STR(pEntry->Addr));
			MTWF_PRINT("%-4d", (int)pEntry->func_tb_idx);
			MTWF_PRINT("%-4d", (int)pEntry->Aid);
			MTWF_PRINT("%-4d", (int)pEntry->PsMode);
			MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[0]);
			MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[1]);
			MTWF_PRINT("%-7d", pEntry->RssiSample.AvgRssi[2]);
			MTWF_PRINT("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT) {
				MTWF_PRINT("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				MTWF_PRINT("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			} else
#endif	/* DOT11_HE_AX */
				MTWF_PRINT("%-6d", pEntry->HTPhyMode.field.MCS);

			MTWF_PRINT("%-6d", pEntry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-6d\n", pEntry->HTPhyMode.field.STBC);
			MTWF_PRINT("%-52s%-10s", "MaxCap:", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
			MTWF_PRINT("%-6s", get_bw_str(pEntry->MaxHTPhyMode.field.BW));
#ifdef DOT11_VHT_AC

			if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) {
				MTWF_PRINT("%dS-M%d",
						 ((pEntry->MaxHTPhyMode.field.MCS>>4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf));
			} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				MTWF_PRINT("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			} else
#endif	/* DOT11_HE_AX */
				MTWF_PRINT("%-6d", pEntry->MaxHTPhyMode.field.MCS);

			MTWF_PRINT("%-6d", pEntry->MaxHTPhyMode.field.ShortGI);
			MTWF_PRINT("%-6d\n", pEntry->MaxHTPhyMode.field.STBC);
		}
	}

	return TRUE;
}

#define TEMP_STR_SIZE 256

INT profile_wds_reg(
	PRTMP_ADAPTER ad,
	UCHAR band_idx,
	RTMP_STRING *buffer)
{
	CHAR *str_ptr = NULL;
	CHAR tmpbuf[TEMP_STR_SIZE] = "";
	ULONG wds_mode = 0;

	if (buffer == NULL) {
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
			 "WDS invalid buffer\n");
		return NDIS_STATUS_FAILURE;
	} else
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
			 "WDS registering.\n");

	ad->WdsTab.wds_num[band_idx] = 0;

	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, TEMP_STR_SIZE, buffer, TRUE) != TRUE) {
		MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
			 "WdsEnable not found(%s)\n", tmpbuf);
		return NDIS_STATUS_FAILURE;
	} else {
		if (kstrtol(tmpbuf, 10, &wds_mode) != 0) {
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_WARN,
				 "WdsEnable is unknow(%s)\n", tmpbuf);
		} else {
			if (wds_mode >= WDS_LAZY_MODE)
				ad->WdsTab.wds_num[band_idx] += MAX_WDS_PER_BAND;
			else if (wds_mode == WDS_DISABLE_MODE)
				ad->WdsTab.wds_num[band_idx] = 0;
			else {
				if (RTMPGetKeyParameter("WdsList", tmpbuf, TEMP_STR_SIZE, buffer, TRUE) != TRUE) {
					MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
						 "WdsList not found(%s)\n", tmpbuf);
					return NDIS_STATUS_FAILURE;
				}

				for (str_ptr = rstrtok(tmpbuf, ";");
					str_ptr; str_ptr = rstrtok(NULL, ";")) {
					MTWF_DBG(ad, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
						 "WdsList(%d):%s\n", ad->WdsTab.wds_num[band_idx], str_ptr);
					ad->WdsTab.wds_num[band_idx]++;
				}
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

VOID rtmp_read_wds_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT			i = 0, j;
	UCHAR		op_mode, macAddress[MAC_ADDR_LEN];
	PRT_802_11_WDS_ENTRY pWdsEntry;
	struct wifi_dev *wdev, *main_wdev = NULL;

	if (buffer == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "invalid buffer\n");
		return;
	}

	/*Wds Number */
	if (RTMPGetKeyParameter("WdsNum", tmpbuf, 10, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";");
		    (macptr && i < DBDC_BAND_NUM);
		     macptr = rstrtok(NULL, ";"), i++) {
			pAd->WdsTab.wds_num[i] = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				"wds_num[%d]=%d!\n", i, pAd->WdsTab.wds_num[i]);
		}
	}
#ifdef DBDC_MODE
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "WDS Number: band[0]=%d, band[1]=%d\n",
		 pAd->WdsTab.wds_num[DBDC_BAND0], pAd->WdsTab.wds_num[DBDC_BAND1]);
#else
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "WDS Number: band[0]=%d\n",
		 pAd->WdsTab.wds_num[DBDC_BAND0]);
#endif

	/*WdsEnable */
	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, 10, buffer, TRUE)) {
		UCHAR wds_idx = 0;
		RT_802_11_WDS_ENTRY *pWdsEntry;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < DBDC_BAND_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UINT32 start_idx = MAX_WDS_PER_BAND * i;
			UINT32 end_idx = MAX_WDS_PER_BAND * (i + 1);
			switch (os_str_tol(macptr, 0, 10)) {
			case WDS_BRIDGE_MODE: /* Bridge mode, DisAllow association(stop Beacon generation and Probe Req. */
				pAd->WdsTab.Mode[i] = WDS_BRIDGE_MODE;
				break;

			case WDS_RESTRICT_MODE:
			case WDS_REPEATER_MODE: /* Repeater mode */
				pAd->WdsTab.Mode[i] = WDS_REPEATER_MODE;
				break;

			case WDS_LAZY_MODE: /* Lazy mode, Auto learn wds entry by same SSID, channel, security policy */
				for (wds_idx = start_idx; wds_idx < end_idx; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[wds_idx];

					if (WDS_ENTRY_IS_ASSIGNED(pWdsEntry->flag) && (HcGetBandByWdev(&pWdsEntry->wdev) == i))
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);

					/* When Lazy mode is enabled, the all wds-link shall share the same encryption type and key material */
					if (wds_idx > start_idx) {
						os_move_mem(pWdsEntry->wdev.SecConfig.WepKey,
							    pAd->WdsTab.WdsEntry[start_idx].wdev.SecConfig.WepKey,
							    sizeof(pWdsEntry->wdev.SecConfig.WepKey));
						os_move_mem(pWdsEntry->wdev.SecConfig.PSK,
							    pAd->WdsTab.WdsEntry[start_idx].wdev.SecConfig.PSK,
							    sizeof(pWdsEntry->wdev.SecConfig.PSK));
						pWdsEntry->wdev.SecConfig.AKMMap = pAd->WdsTab.WdsEntry[start_idx].wdev.SecConfig.AKMMap;
						pWdsEntry->wdev.SecConfig.PairwiseCipher = pAd->WdsTab.WdsEntry[start_idx].wdev.SecConfig.PairwiseCipher;
						pWdsEntry->wdev.SecConfig.PairwiseKeyId = pAd->WdsTab.WdsEntry[start_idx].wdev.SecConfig.PairwiseKeyId;
					}
				}

				pAd->WdsTab.Mode[i] = WDS_LAZY_MODE;
				break;

			case WDS_DISABLE_MODE:	/* Disable mode */
			default:
				for (wds_idx = start_idx; wds_idx < end_idx; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[wds_idx];

					if (WDS_ENTRY_IS_ASSIGNED(pWdsEntry->flag) && (HcGetBandByWdev(&pWdsEntry->wdev) == i))
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);
				}
				pAd->WdsTab.Mode[i] = WDS_DISABLE_MODE;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "band%d WDS-Enable mode=%d\n", i, pAd->WdsTab.Mode[i]);
		}
	}

	/*WdsList */
	if (RTMPGetKeyParameter("WdsList", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if (strlen(macptr) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				continue;

			if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				continue;

			for (j = 0; j < MAC_ADDR_LEN; j++) {
				AtoH(macptr, &macAddress[j], 1);
				macptr = macptr+3;
			}

			if (i < pAd->WdsTab.wds_num[DBDC_BAND0])
				WdsEntryAlloc(pAd, DBDC_BAND0, macAddress);
			else
				WdsEntryAlloc(pAd, DBDC_BAND1, macAddress);
		}
	}

	/*WdsPhyMode */
	if (RTMPGetKeyParameter("WdsPhyMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode && (i >= pAd->WdsTab.wds_num[DBDC_BAND0])) {
				main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
			} else
#endif	/* DBDC_MODE */
				main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			if (rtstrcasecmp(macptr, "CCK") == TRUE)
				op_mode = MODE_CCK;
			else if (rtstrcasecmp(macptr, "OFDM") == TRUE)
				op_mode = MODE_OFDM;

#ifdef DOT11_N_SUPPORT
			else if (rtstrcasecmp(macptr, "HTMIX") == TRUE)
				op_mode = MODE_HTMIX;
			else if (rtstrcasecmp(macptr, "GREENFIELD") == TRUE)
				op_mode = MODE_HTGREENFIELD;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
			else if (rtstrcasecmp(macptr, "VHT") == TRUE)
				op_mode = MODE_VHT;

#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			else if (rtstrcasecmp(macptr, "HE") == TRUE)
				op_mode = MODE_HE;
#endif
			else
				op_mode = MODE_UNKNOWN;

			pWdsEntry->phy_mode = get_wds_phymode(main_wdev, op_mode);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "If/wds%d - PeerPhyMode=0x%x\n", i, pWdsEntry->phy_mode);
		}
	}

	/* WdsTxMode */
	if (RTMPGetKeyParameter("WdsTxMode", tmpbuf, 25, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "I/F(wds%d) Tx Mode = %d\n", i,
					 wdev->DesiredTransmitSetting.field.FixedTxMode);
		}
	}

	/* WdsTxMcs */
	if (RTMPGetKeyParameter("WdsTxMcs", tmpbuf, 50, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "I/F(wds%d) Tx MCS = AUTO\n", i);
			else {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "I/F(wds%d) Tx MCS = %d\n", i,
						 wdev->DesiredTransmitSetting.field.MCS);
			}
		}
	}

#ifdef WDS_VLAN_SUPPORT

	/* WdsVlan */
	if (RTMPGetKeyParameter("WDS_VLANID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID = os_str_tol(macptr, 0, 10);
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_Priority = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "If/wds%d - WdsVlanId=%d\n", i, pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID);
		}
	}

#endif /* WDS_VLAN_SUPPORT */
}

VOID wds_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	pTxBlk->CipherAlg = CIPHER_NONE;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) {
		pTxBlk->pKey =  NULL;
	} else if (pMacEntry) {
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
			struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
			TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

			pTxBlk->CipherAlg = pSecConfig->PairwiseCipher;
			pTxBlk->KeyIdx =  pSecConfig->PairwiseKeyId;

			if (IS_CIPHER_WEP40(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pSecConfig->PairwiseKeyId]);
				pTxBlk->CipherAlg = CIPHER_WEP64;
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else if (IS_CIPHER_WEP104(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pSecConfig->PairwiseKeyId]);
				pTxBlk->CipherAlg = CIPHER_WEP128;
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else {
				pTxBlk->pKey = &(pSecConfig->SwPairwiseKey);
				pTxBlk->CipherAlg = pMacEntry->SecConfig.SwPairwiseKey.CipherAlg;
				if (pTxBlk->CipherAlg == CIPHER_AES)
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
				else if (pTxBlk->CipherAlg == CIPHER_TKIP)
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
			}
		}
	}
#else /*  SOFT_ENCRYPT || SW_CONNECT_SUPPORT */
	pTxBlk->CipherAlg = CIPHER_NONE;
#endif /* !SOFT_ENCRYPT && !SW_CONNECT_SUPPORT */

}

static struct wifi_dev_ops wds_wdev_ops = {
	.tx_pkt_allowed = wds_tx_pkt_allowed,
	.fp_tx_pkt_allowed = wds_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.fill_non_offload_tx_blk = ap_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = ap_fill_offload_tx_blk,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_pkt_foward = wds_rx_foward_handle,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = wds_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
	.media_state_connected = media_state_connected,
	.ioctl = rt28xx_ap_ioctl,
	.open = wds_inf_open,
	.close = wds_inf_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};

VOID WDS_Init(RTMP_ADAPTER *pAd, UCHAR band_idx, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	INT if_idx = -1, count = 0, netdev_idx = 0;
	PNET_DEV pWdsNetDev;
	struct wifi_dev *wdev, *main_wdev = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (band_idx >= DBDC_BAND_NUM)
		goto error_out;

	if_idx = band_idx * MAX_WDS_PER_BAND;
	count = pAd->WdsTab.wds_num[band_idx];

	if ((if_idx+count) > MAX_WDS_ENTRY) {
		MTWF_DBG(pAd, DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR,
			"Invalid WDS amount of entries(%d, max=%d), try to alloac %d\n",
			 if_idx+count, MAX_WDS_ENTRY, count);
		goto error_out;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, CATAP_WDS, DBG_LVL_INFO,
		 "wds_num[%d]=%d, MAX_WDS_ENTRY=%d, if_idx=%d\n",
		  band_idx, count, MAX_WDS_ENTRY, if_idx);

	/* sanity check to avoid redundant virtual interfaces are created */
	for (; count ; if_idx++, count-- , netdev_idx++) {
		wds_entry = &pAd->WdsTab.WdsEntry[if_idx];

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode && (band_idx > DBDC_BAND0))
			main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
		else
#endif	/* DBDC_MODE */
			main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

		wdev = &wds_entry->wdev;
		wdev->PhyMode = main_wdev->PhyMode;
		wdev->sys_handle  = main_wdev->sys_handle;
		os_move_mem(wdev->if_addr, main_wdev->if_addr, MAC_ADDR_LEN);
		os_move_mem(wdev->bssid, wdev->if_addr, MAC_ADDR_LEN);
		os_move_mem(wdev->bss_info_argument.Bssid, wdev->if_addr, MAC_ADDR_LEN);

		if (!WDS_ENTRY_IS_INITED(wds_entry->flag)) {
			UINT32 MC_RowID = 0, IoctlIF = 0;
			char *dev_name;
			INT32 Ret;
#if defined(MULTI_PROFILE)
			UCHAR wds_dev_name[IFNAMSIZ] = {"\0"};
			if (pAd->CommonCfg.dbdc_mode) {
				multi_profile_wds_devname_req(pAd, wds_dev_name, if_idx);
				dev_name = wds_dev_name;
			} else
#endif	/* DBDC_MODE */
			{
				dev_name = get_dev_name_prefix(pAd, INT_WDS);
				if (dev_name == NULL) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
							"Get name prefix fail\n");
					break;
				}
			}

			pWdsNetDev = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_WDS, netdev_idx,
							sizeof(struct mt_dev_priv), dev_name, TRUE);


			if (pWdsNetDev == NULL) {
				/* allocation fail, exit */
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Allocate network device fail (WDS)...\n");
				break;
			}

			NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_WDS, pWdsNetDev, if_idx, wds_entry, (VOID *)pAd);

			if (Ret == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "Assign wdev idx for %s failed, free net device!\n",
						 RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev));
				RtmpOSNetDevFree(pWdsNetDev);
				break;
			}

			Ret = wdev_ops_register(wdev, WDEV_TYPE_WDS, &wds_wdev_ops,
									cap->qos.wmm_detect_method);

			if (!Ret) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
						 "register wdev_ops %s failed, free net device!\n",
						  RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev));
				RtmpOSNetDevFree(pWdsNetDev);
				break;
			}
			update_att_from_wdev(wdev, main_wdev);
			MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;
			/*update rate info for wdev*/
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

			/* transfor RXBLK Addr1 according this adddress */
			RTMP_OS_NETDEV_SET_PRIV(pWdsNetDev, pAd);
			RTMP_OS_NETDEV_SET_WDEV(pWdsNetDev, wdev);
			pNetDevOps->priv_flags = INT_WDS;
			pNetDevOps->needProtcted = TRUE;
			pNetDevOps->wdev = wdev;
			/* Register this device */
			RtmpOSNetDevAttach(pAd->OpMode, pWdsNetDev, pNetDevOps);
			WDS_ENTRY_SET_INITED(wds_entry->flag);
		} else {
			update_att_from_wdev(wdev, main_wdev);
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		}
	}

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
		 "Total allocated %d WDS(es) for band%d!\n", netdev_idx, band_idx);

error_out:
	if (if_idx < 0)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "WDS initialized with invalid band index:%d\n", band_idx);
}


VOID WDS_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		wdev = &pAd->WdsTab.WdsEntry[index].wdev;
		
		MTWF_DBG(pAd, DBG_CAT_ALL, CATAP_WDS, DBG_LVL_INFO, "%s: index:%d\n", __func__, index);

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);
			/* Clear it as NULL to prevent latter access error. */
			WDS_ENTRY_CLEAR_FLAG(pAd->WdsTab.WdsEntry[index].flag);
			wdev->if_dev = NULL;
		}
	}

	for (index = DBDC_BAND0 ; index < DBDC_BAND_NUM ; index++)
		pAd->WdsTab.wds_num[index] = 0;
}


BOOLEAN WDS_StatsGet(RTMP_ADAPTER *pAd, RT_CMD_STATS *pStats)
{
	INT WDS_apidx = 0, index;
	RT_802_11_WDS_ENTRY *wds_entry;

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pStats->pNetDev) {
			WDS_apidx = index;
			break;
		}
	}

	if (index >= MAX_WDS_ENTRY) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "can not find wds I/F\n");
		return FALSE;
	}

	wds_entry = &pAd->WdsTab.WdsEntry[WDS_apidx];
	pStats->pStats = pAd->stats;
	pStats->rx_packets = wds_entry->WdsCounter.ReceivedFragmentCount.QuadPart;
	pStats->tx_packets = wds_entry->WdsCounter.TransmittedFragmentCount.QuadPart;
	pStats->rx_bytes = wds_entry->WdsCounter.ReceivedByteCount;
	pStats->tx_bytes = wds_entry->WdsCounter.TransmittedByteCount;
	pStats->rx_errors = wds_entry->WdsCounter.RxErrorCount;
	pStats->tx_errors = wds_entry->WdsCounter.TxErrors;
	pStats->multicast = wds_entry->WdsCounter.MulticastReceivedFrameCount.QuadPart;   /* multicast packets received */
	pStats->collisions = 0;  /* Collision packets */
	pStats->rx_over_errors = wds_entry->WdsCounter.RxNoBuffer;                   /* receiver ring buff overflow */
	pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;     // recved pkt with crc error */
	pStats->rx_frame_errors = 0; /* recv'd frame alignment error */
	pStats->rx_fifo_errors = wds_entry->WdsCounter.RxNoBuffer;                   /* recv'r fifo overrun */
	return TRUE;
}


/*
* WDS_Open
*/
INT wds_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	struct _MAC_TABLE_ENTRY *peer;


	if (wdev) {
		pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		wds_entry = &pAd->WdsTab.WdsEntry[wdev->func_idx];
		if (wifi_sys_open(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "open fail!!!\n");
			return FALSE;
		}

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "WDS inf up for wds_%x(func_idx) OmacIdx=%d\n",
			wdev->func_idx, wdev->OmacIdx);

		RTMPSetIndividualHT(pAd, wdev->func_idx + MIN_NET_DEVICE_FOR_WDS);

		if (strcmp(wds_entry->PeerWdsAddr, "00:00:00:00:00:00") != 0)
			WdsEntryAlloc(pAd, HcGetBandByWdev(wdev), wds_entry->PeerWdsAddr);

		if (WDS_ENTRY_IS_ASSIGNED(wds_entry->flag)) {
			peer = MacTableInsertWDSEntry(pAd, wds_entry->PeerWdsAddr, wdev->func_idx);

			if (!peer) {
				MTWF_DBG(pAd, DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, "can't insert a new WDS entry!");
				return FALSE;
			}

			RAInit(pAd, peer);
			WDS_ENTRY_SET_VALID(wds_entry->flag);
			wds_entry->wdev.DevInfo.WdevActive = TRUE;
		}
#ifdef DPP_SUPPORT
		DlListInit(&wdev->dpp_frame_event_list);
#endif /* DPP_SUPPORT */
	}


	return TRUE;
}

/*
* WDS_Close
*/
INT wds_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	UINT16 wcid;


	if (wdev) {
		pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		if (wdev_do_linkdown(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "linkdown fail!!\n");
			return FALSE;
		}

		wds_entry = &pAd->WdsTab.WdsEntry[wdev->func_idx];
		if (WdsTableLookup(pAd, wds_entry->PeerWdsAddr, TRUE)) {
			wcid = wds_entry->peer->wcid;
			WdsEntryDel(pAd, wds_entry->PeerWdsAddr);
			MacTableResetWdev(pAd, wdev);
		}

		wds_entry->wdev.DevInfo.WdevActive = FALSE;
		if (wifi_sys_close(wdev) != TRUE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "close fail!!!\n");
			return FALSE;
		}
	}


	return TRUE;
}

/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN WdsMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT PINT Machine,
	OUT PINT MsgType)
{
	/* wds data packet */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
		if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG,
				"%s, AP WDS recv UC data from "MACSTR"\n",
				__func__, MAC2STR(pFrame->Hdr.Addr2));

			*Machine = WDS_STATE_MACHINE;
			*MsgType = APMT2_WDS_RECV_UC_DATA;

			return TRUE;
		}
	}

	return FALSE;
}

VOID ap_wds_rcv_uc_data_action(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
	MAC_TABLE_ENTRY *pEntry;
	RT_802_11_WDS_ENTRY *wds_entry;
	struct wifi_dev *main_wdev;

	RETURN_IF_PAD_NULL(pAd);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_DEBUG, "%s: PhyMode = %d\n", __func__, Elem->RxPhyMode);

	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, Elem->Wcid, pFrame->Hdr.Addr2, TRUE);
	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pFrame->Hdr.Addr2, TRUE);

	/* Only Lazy mode will auto learning, match with FrDs=1 and ToDs=1 */
	if ((pEntry == NULL) && (pAd->WdsTab.Mode[HcGetBandByWdev(Elem->wdev)] >= WDS_LAZY_MODE)) {
		INT WdsIdx = WdsEntryAlloc(pAd, HcGetBandByWdev(Elem->wdev), pFrame->Hdr.Addr2);

		if (WdsIdx >= 0 && WdsIdx < MAX_WDS_ENTRY) {
			wds_entry = &pAd->WdsTab.WdsEntry[WdsIdx];

			/* configure peer phymode according to the received frame */
#ifdef DBDC_MODE
			if (WdsIdx >= pAd->WdsTab.wds_num[DBDC_BAND0])
				main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
			else
#endif	/* DBDC_MODE */
				main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

			/* only update phymode once valid value from peer, else refer to configuration */
			if (Elem->RxPhyMode)
				wds_entry->phy_mode = get_wds_phymode(main_wdev, Elem->RxPhyMode);
			pEntry = MacTableInsertWDSEntry(pAd, pFrame->Hdr.Addr2, (UCHAR)WdsIdx);

			if (!pEntry) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
					"can't insert new pEntry\n");
				return;
			}

			RAInit(pAd, pEntry);
			WDS_ENTRY_SET_VALID(wds_entry->flag);
		}
	}
}

VOID ap_wds_wdev_linkdown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN USHORT wcid)
{
	MAC_TABLE_ENTRY *pEntry;

	if (wdev_do_linkdown(wdev) != TRUE)
		 MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, "linkdown fail!!\n");

	if (!IS_WCID_VALID(pAd, wcid))
		return;

	pEntry = &pAd->MacTab.Content[wcid];
	if (pEntry && IS_ENTRY_WDS(pEntry))
		mac_entry_delete(pAd, pEntry);
}

VOID ap_wds_bss_linkdown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	USHORT wcid;

	wdev = Elem->wdev;
	NdisMoveMemory(&wcid, Elem->Msg, sizeof(USHORT));
	ap_wds_wdev_linkdown(pAd, wdev, wcid);
}

VOID WdsStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, WDS_MAX_STATE, WDS_MAX_MSG,
		(STATE_MACHINE_FUNC)Drop, WDS_IDLE, WDS_MACHINE_BASE);
	StateMachineSetAction(Sm, WDS_IDLE, APMT2_WDS_RECV_UC_DATA,
		(STATE_MACHINE_FUNC)ap_wds_rcv_uc_data_action);
	StateMachineSetAction(Sm, WDS_IDLE, WDS_BSS_LINKDOWN,
		(STATE_MACHINE_FUNC)ap_wds_bss_linkdown);
}

#endif /* WDS_SUPPORT */

