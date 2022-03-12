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
	INT i, WdsTabIdx = -1;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
			("%s(): %02x-%02x-%02x-%02x-%02x-%02x, WdsMode = %d\n", __func__, PRINT_MAC(pAddr), pAd->WdsTab.Mode[band_idx]));
	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY ; i++) {
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
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("%s: Unable to allocate WdsEntry.\n", __func__));
	else
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, (" Alloced WdsEntry[%d]\n", i));

	return WdsTabIdx;
}

VOID WdsEntryDel(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): %02x-%02x-%02x-%02x-%02x-%02x\n", __func__,
			 PRINT_MAC(pAddr)));

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];

		if (MAC_ADDR_EQUAL(pAddr, wds_entry->PeerWdsAddr) && WDS_ENTRY_IS_ASSIGNED(wds_entry->flag)) {
			wds_entry->flag = 0;
			NdisZeroMemory(wds_entry->PeerWdsAddr, MAC_ADDR_LEN);
			wds_entry->synced = FALSE;
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
	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): WdsTabIdx = %d, Addr %02x-%02x-%02x-%02x-%02x-%02x\n", __func__,
			 WdsTabIdx, PRINT_MAC(pAddr)));

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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  OmacIdx = %d, wcid = %d, phymode = 0x%x\n",
					 tr_entry->OmacIdx, pEntry->wcid, wds_entry->phy_mode));

			/* specific Max Tx Rate for Wds link. */
			NdisZeroMemory(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));

			if (WMODE_EQUAL(phy_mode, WMODE_B)) {
				HTPhyMode.field.MODE = MODE_CCK;
				HTPhyMode.field.MCS = 3;
				pEntry->RateLen = 4;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_CCK\n"));
			} else if (WMODE_EQUAL(phy_mode, WMODE_GN | WMODE_AN)) {
				HTPhyMode.field.MODE = MODE_HTGREENFIELD;
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_HTGREENFIELD\n"));
			} else if (WMODE_CAP_AX(phy_mode)) {
				/* to check */
				HTPhyMode.field.MODE = MODE_HE;
				HTPhyMode.field.MCS = 11;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_HE\n"));
			} else if (WMODE_CAP_AC(phy_mode)) {
				HTPhyMode.field.MODE = MODE_VHT;
				HTPhyMode.field.MCS = 9;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_VHT\n"));
			} else if (WMODE_CAP_N(phy_mode)) {
				HTPhyMode.field.MODE = MODE_HTMIX;
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_HTMIX\n"));
			} else {
				HTPhyMode.field.MODE = MODE_OFDM;
				HTPhyMode.field.MCS = 7;
				pEntry->RateLen = 8;
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  MODE_OFDM\n"));
			}

			pEntry->MaxHTPhyMode.word = HTPhyMode.word;
			pEntry->MinHTPhyMode.word = wdev->MinHTPhyMode.word;
			pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(phy_mode)) {
				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
					MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("IF-wds%d : Desired MCS = %d\n", WdsTabIdx,
							 wdev->DesiredTransmitSetting.field.MCS));
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

				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
						("  Cap.MaxRAmpduFactor = %d, Cap.MpduDensity = %d\n",
						 pEntry->HTCapability.HtCapParm.MaxRAmpduFactor,
						 pEntry->HTCapability.HtCapParm.MpduDensity));
			} else
				NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(phy_mode) && (wdev->channel > 14)) {
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
				vht_mode_adjust(pAd, pEntry, &vht_cap, NULL);
				dot11_vht_mcs_to_internal_mcs(pAd, wdev, &vht_cap, &pEntry->MaxHTPhyMode);
				set_vht_cap(pAd, pEntry, &vht_cap);
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n",
							__func__,
							get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
							get_bw_str(pEntry->MaxHTPhyMode.field.BW),
							pEntry->MaxHTPhyMode.field.MCS,
							pEntry->MaxHTPhyMode.word));
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

				get_own_he_ie(wdev, &he_ie);
				update_peer_he_params(pEntry, &he_ie);
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
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): linkup fail!!\n", __func__));

			if (wdev_do_conn_act(wdev, pEntry) != TRUE)
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): connect fail!!\n", __func__));

			AsicUpdateWdsEncryption(pAd, pEntry->wcid);

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("  PhyMode = 0x%x, Channel = %d\n",
					 wdev->PhyMode, wdev->channel));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s() - allocate entry #%d(link to WCID %d)\n",
					 __func__, WdsTabIdx, pEntry->wcid));
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
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

MAC_TABLE_ENTRY *FindWdsEntry(
	IN RTMP_ADAPTER *pAd,
	IN struct _RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pAd->WdsTab.flg_wds_init[pRxBlk->band]) {
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE, ("%s(): Wcid = %d, PhyMode = 0x%x\n", __func__,
			 pRxBlk->wcid, pRxBlk->rx_rate.field.MODE));
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
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
					 ("%s(): No matched wdev (%02x-%02x-%02x-%02x-%02x-%02x), wcid = %d!!!\n", __func__,
					 PRINT_MAC(pRxBlk->Addr1), pRxBlk->wcid));
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

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE,
				 ("!!! report WDS UC DATA (from %02x-%02x-%02x-%02x-%02x-%02x) to MLME (len=%d) !!!\n",
				 PRINT_MAC(pRxBlk->Addr2), pRxBlk->DataSize));
		} else if (pEntry) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
				 ("%s[band%d]A2(%02x-%02x-%02x-%02x-%02x-%02x) found as wcid:%d!!! (Drop)\n", __func__, pRxBlk->band,
				 PRINT_MAC(pRxBlk->Addr2), pEntry->wcid));
		}
	} else {
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE,
			 ("!!! Recv WDS UC DATA (from %02x-%02x-%02x-%02x-%02x-%02x) but WDS for band%d is not enabled !!! (Drop)\n",
			 PRINT_MAC(pRxBlk->Addr2), pRxBlk->band));
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

		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, ("%s(): Entry[%d], Wcid = %d, %02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, idx, peer->wcid, PRINT_MAC(peer->Addr)));

		NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
		NdisAcquireSpinLock(&pAd->MacTabLock);
		peer->NoDataIdleCount++;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		tr_ctl->tr_entry[peer->wcid].NoDataIdleCount++;
		NdisReleaseSpinLock(&pAd->MacTabLock);
		NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);

		/* delete those MAC entry that has been idle for a long time */
		if (peer->NoDataIdleCount >= MAC_TABLE_AGEOUT_TIME) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from WDS #%d after %d-sec silence\n",
					 PRINT_MAC(peer->Addr), idx, MAC_TABLE_AGEOUT_TIME));
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

UCHAR WdsGetPeerSuppPhyModeLegacy(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN UCHAR Channel)
{
	UCHAR PeerPhyModeLegacy = 0;
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
				if (Channel > 14)
					PeerPhyModeLegacy |= WMODE_A;
				else
					PeerPhyModeLegacy |= WMODE_G;
			}
		}
	}

	return PeerPhyModeLegacy;
}

UCHAR WdsGetPeerSuppPhyMode(
	IN BCN_IE_LIST * ie_list)
{
	UCHAR PeerPhyMode = 0;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		if (ie_list->Channel > 14)
			PeerPhyMode |= WMODE_AX_5G;
		else
			PeerPhyMode |= WMODE_AX_24G;
	}
#endif

#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) && ie_list->Channel > 14)
		PeerPhyMode |= WMODE_AC;
#endif

	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		if (ie_list->Channel > 14)
			PeerPhyMode |= WMODE_AN;
		else
			PeerPhyMode |= WMODE_GN;
	}
#endif

	/* Check OFDM/CCK capability */
	PeerPhyMode |= WdsGetPeerSuppPhyModeLegacy(rate->sup_rate, rate->sup_rate_len, ie_list->Channel);
	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, ("%s(): %02x-%02x-%02x-%02x-%02x-%02x, PeerPhyMode = 0x%x\n", __func__,
			 PRINT_MAC(ie_list->Addr2), PeerPhyMode));

	if (((ie_list->Channel > 0) && (ie_list->Channel <= 14) && WMODE_CAP_5G(PeerPhyMode)) ||
		((ie_list->Channel > 14) && WMODE_CAP_2G(PeerPhyMode)))
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("ERROR!! Wrong PeerPhyMode 0x%x, Channel %d\n", PeerPhyMode, ie_list->Channel));

	return PeerPhyMode;
}

VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BCN_IE_LIST * ie_list)
{
	UCHAR MaxSupportedRate = RATE_11;
	UCHAR PeerPhyMode;
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

	RETURN_IF_PAD_NULL(pAd);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): Invalid pEntry!", __func__));
		return;
	}

	if (!IS_ENTRY_WDS(pEntry)) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): Invalid WDS pEntry!", __func__));
		return;
	}

	wdev = pEntry->wdev;
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): Invalid wdev!", __func__));
		return;
	}

	pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

	if (!pWdsEntry) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): Invalid WDS entry!", __func__));
		return;
	}

	if (!WDS_ENTRY_IS_VALID(pWdsEntry->flag)) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): WDS entry not ready!", __func__));
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
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): Invalid ie_list!", __func__));
		return;
	}

	PeerPhyMode = WdsGetPeerSuppPhyMode(ie_list);
	cmm_phy_mode = wdev->PhyMode & PeerPhyMode;
	if (is_limited_by_security(pAd, wdev))
		cmm_phy_mode &= (WMODE_A | WMODE_B | WMODE_G);

	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);
	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO,
			("%s(): wcid=%d, max supported rate = %d, support caps. = 0x%x, peer phy mode = 0x%x\n", __func__,
			pEntry->wcid, MaxSupportedRate, cmm_ies->ie_exists, PeerPhyMode));

	pEntry->MaxSupportedRate = min(pEntry->wdev->rate.MaxTxRate, MaxSupportedRate);
	pEntry->RateLen = MaxSupportedRateLen;

	/* Update PhyMode if Peer PhyMode gets updated */
	if ((pWdsEntry->wdev.PhyMode != PeerPhyMode) && (PeerPhyMode != WMODE_INVALID)) {
			OriWdevPhyMode = pWdsEntry->wdev.PhyMode;
			bReInitPhyMode = TRUE;
	}

	if (WMODE_EQUAL(cmm_phy_mode, WMODE_B)) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = 3;
		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->RateLen = 4;
	} else if (WMODE_CAP(cmm_phy_mode, WMODE_G | WMODE_A)) {
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
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE, ("%s(): set new_bw = %d\n", __func__, new_bw));
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

		vht_mode_adjust(pAd, pEntry, vht_cap, NULL);
		dot11_vht_mcs_to_internal_mcs(pAd, wdev, vht_cap, &pEntry->MaxHTPhyMode);
		set_vht_cap(pAd, pEntry, vht_cap);
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, ("%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n",
					__func__,
					get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
					get_bw_str(pEntry->MaxHTPhyMode.field.BW),
					pEntry->MaxHTPhyMode.field.MCS,
					pEntry->MaxHTPhyMode.word));
		NdisMoveMemory(&pEntry->vht_cap_ie, vht_cap, sizeof(VHT_CAP_IE));

		if (DebugLevel >= DBG_LVL_INFO)
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
		he_mode_adjust(pEntry->wdev, pEntry);
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

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, (" Mode %d, MCS %d, STBC %d SGI %d, BW %d / MAX Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
			 pEntry->HTPhyMode.field.MODE,
			 pEntry->HTPhyMode.field.MCS,
			 pEntry->HTPhyMode.field.STBC,
			 pEntry->HTPhyMode.field.ShortGI,
			 pEntry->HTPhyMode.field.BW,
			 pEntry->MaxHTPhyMode.field.MODE,
			 pEntry->MaxHTPhyMode.field.MCS,
			 pEntry->MaxHTPhyMode.field.STBC,
			 pEntry->MaxHTPhyMode.field.ShortGI,
			 pEntry->MaxHTPhyMode.field.BW));

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
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, (" bRaReInit, New Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
				 pEntry->HTPhyMode.field.MODE,
				 pEntry->HTPhyMode.field.MCS,
				 pEntry->HTPhyMode.field.STBC,
				 pEntry->HTPhyMode.field.ShortGI,
				 pEntry->HTPhyMode.field.BW));
		bRaReInit = TRUE;
	}

	if (bRaReInit && pEntry->bAutoTxRateSwitch == TRUE) {
		/* StaRec update */
		wifi_sys_update_wds(pAd, pEntry);
		AsicUpdateWdsEncryption(pAd, pEntry->wcid);
		RAInit(pAd, pEntry);
	}

	if (!pWdsEntry->synced)
		pWdsEntry->synced = TRUE;
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
		wds_entry->flag = 0;
		wds_entry->peer = NULL;
		wds_entry->KeyIdx = 0;
		NdisZeroMemory(&wds_entry->WdsKey, sizeof(CIPHER_KEY));
		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
	}
	MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_OFF,
		 ("%s():WdsEntry[0~%d]\n", __func__, MAX_WDS_ENTRY-1));
}

INT Show_WdsTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	struct _RT_802_11_WDS_ENTRY *wds_entry;
	CHAR *wmode;

	for (i = DBDC_BAND0; i < DBDC_BAND_NUM; i++) {
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("\n[band%d]WDS mode=%s\n", i, wds_mode_2_str(pAd->WdsTab.Mode[i])));

		if (pAd->WdsTab.Mode[i])
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
				 ("\tWDS for band[%d] is %s(count:%d)\n\n", i,
				  (pAd->WdsTab.flg_wds_init[i]) ? "initialized" : "not initialized",
				   (UINT32)pAd->WdsTab.wds_num[i]));
	}

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("############## WDS List ##############\n"));
	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];
		wmode = wmode_2_str(wds_entry->phy_mode);

		if (!wmode)
			return FALSE;

		if (HcIsRadioAcq(&wds_entry->wdev)) {
			struct _SECURITY_CONFIG *p_sec_con = NULL;
			p_sec_con = &wds_entry->wdev.SecConfig;

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
				 ("IF/WDS%d(band%d)-%02x:%02x:%02x:%02x:%02x:%02x(%s,%s), OpState=%d Sync:%s,\n\t\t\twmode:%s Cipher=%s, KeyId=%d\n",
				  i, HcGetBandByWdev(&wds_entry->wdev),
				  PRINT_MAC(wds_entry->PeerWdsAddr),
				  WDS_ENTRY_IS_ASSIGNED(wds_entry->flag) ? "Assigned" : "Unassigned",
				  WDS_ENTRY_IS_VALID(wds_entry->flag) ? "Valid" : "Invalid",
				  wlan_operate_get_state(&wds_entry->wdev),
				  (wds_entry->synced) ? "TRUE" : "FALSE",
				  wmode,
				  GetEncryModeStr(p_sec_con->PairwiseCipher),
				  wds_entry->wdev.SecConfig.PairwiseKeyId));

			if (!IS_CIPHER_NONE(p_sec_con->PairwiseCipher))  {
				UCHAR *key_str = NULL;

				if (IS_CIPHER_WEP(p_sec_con->PairwiseCipher))
					key_str = p_sec_con->WepKey[p_sec_con->PairwiseKeyId].Key;
				else
					key_str = p_sec_con->PSK;

				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
						 ("\t\t\t\tWds Key:%s (%d)\n",
						  key_str, (UINT32)strlen(key_str)));
			}
		} else
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("IF/WDS%d not occupied\n", i));

		os_free_mem(wmode);
	}

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("\n%-19s%-4s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
			 "MAC", "IDX", "AID", "PSM", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC"));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_WDS(pEntry)) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr)));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-4d", (int)pEntry->func_tb_idx));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-4d", (int)pEntry->Aid));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-4d", (int)pEntry->PsMode));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[0]));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[1]));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[2]));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT) {
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf)));
			} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf)));
			} else
#endif	/* DOT11_HE_AX */
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d\n", pEntry->HTPhyMode.field.STBC));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-52s%-10s", "MaxCap:", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6s", get_bw_str(pEntry->MaxHTPhyMode.field.BW)));
#ifdef DOT11_VHT_AC

			if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) {
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%dS-M%d",
						 ((pEntry->MaxHTPhyMode.field.MCS>>4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf)));
			} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			if (pEntry->HTPhyMode.field.MODE == MODE_HE) {
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf)));
			} else
#endif	/* DOT11_HE_AX */
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d", pEntry->MaxHTPhyMode.field.MCS));

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d", pEntry->MaxHTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%-6d\n", pEntry->MaxHTPhyMode.field.STBC));
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
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
			 ("%s(): WDS Profile parsing dismissed, invalid buffer\n", __func__));
		return NDIS_STATUS_FAILURE;
	} else
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE,
			 ("%s(): WDS registering.\n", __func__));

	ad->WdsTab.wds_num[band_idx] = 0;

	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, TEMP_STR_SIZE, buffer, TRUE) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
			 ("WdsEnable not found(%s)\n", tmpbuf));
		return NDIS_STATUS_FAILURE;
	} else {
		if (kstrtol(tmpbuf, 10, &wds_mode) != 0) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
				 ("WdsEnable is unknow(%s)\n", tmpbuf));
		} else {
			if (wds_mode >= WDS_LAZY_MODE)
				ad->WdsTab.wds_num[band_idx] += MAX_WDS_PER_BAND;
			else {
				if (RTMPGetKeyParameter("WdsList", tmpbuf, TEMP_STR_SIZE, buffer, TRUE) != TRUE) {
					MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
						 ("WdsList not found(%s)\n", tmpbuf));
					return NDIS_STATUS_FAILURE;
				}

				for (str_ptr = rstrtok(tmpbuf, ";");
					str_ptr; str_ptr = rstrtok(NULL, ";")) {
					MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
						 ("WdsList(%ld):%s\n", ad->WdsTab.wds_num[band_idx], tmpbuf));
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
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): WDS Profile parsing dismissed, invalid buffer\n", __func__));
		return;
	} else
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): WDS Profile\n", __func__));

	/*Wds Number */
	if (RTMPGetKeyParameter("WdsNum", tmpbuf, 10, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";");
			(macptr && i < DBDC_BAND_NUM); macptr = rstrtok(NULL, ";"), i++)
			if (kstrtoul(macptr, 10, &pAd->WdsTab.wds_num[i]) != 0)
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("WDS Number defined by WdsList!\n"));
	}
#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("WDS Number: band[0]=%ld, band[1]=%ld\n",
		 pAd->WdsTab.wds_num[DBDC_BAND0], pAd->WdsTab.wds_num[DBDC_BAND1]));
#else
	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("WDS Number: band[0]=%ld\n",
		 pAd->WdsTab.wds_num[DBDC_BAND0]));
#endif

	/*WdsEnable */
	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, 10, buffer, TRUE)) {
		UCHAR wds_idx = 0;
		RT_802_11_WDS_ENTRY *pWdsEntry;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < DBDC_BAND_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UINT32 start_idx = pAd->WdsTab.wds_num[DBDC_BAND0]*i;
			switch (os_str_tol(macptr, 0, 10)) {
			case WDS_BRIDGE_MODE: /* Bridge mode, DisAllow association(stop Beacon generation and Probe Req. */
				pAd->WdsTab.Mode[i] = WDS_BRIDGE_MODE;
				break;

			case WDS_RESTRICT_MODE:
			case WDS_REPEATER_MODE: /* Repeater mode */
				pAd->WdsTab.Mode[i] = WDS_REPEATER_MODE;
				break;

			case WDS_LAZY_MODE: /* Lazy mode, Auto learn wds entry by same SSID, channel, security policy */
				for (wds_idx = start_idx; wds_idx < MAX_WDS_ENTRY; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[i];

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
				for (wds_idx = 0; wds_idx < MAX_WDS_ENTRY; wds_idx++) {
					pWdsEntry = &pAd->WdsTab.WdsEntry[i];

					if (WDS_ENTRY_IS_ASSIGNED(pWdsEntry->flag) && (HcGetBandByWdev(&pWdsEntry->wdev) == i))
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);
				}
				pAd->WdsTab.Mode[i] = WDS_DISABLE_MODE;
				break;
			}

			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("WDS-Enable mode=%d\n", pAd->WdsTab.Mode[i]));
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
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
			wdev = &pWdsEntry->wdev;
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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("If/wds%d - PeerPhyMode=0x%x\n", i, pWdsEntry->phy_mode));
		}
	}

	/* WdsTxMode */
	if (RTMPGetKeyParameter("WdsTxMode", tmpbuf, 25, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("I/F(wds%d) Tx Mode = %d\n", i,
					 wdev->DesiredTransmitSetting.field.FixedTxMode));
		}
	}

	/* WdsTxMcs */
	if (RTMPGetKeyParameter("WdsTxMcs", tmpbuf, 50, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("I/F(wds%d) Tx MCS = AUTO\n", i));
			else {
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("I/F(wds%d) Tx MCS = %d\n", i,
						 wdev->DesiredTransmitSetting.field.MCS));
			}
		}
	}

#ifdef WDS_VLAN_SUPPORT

	/* WdsVlan */
	if (RTMPGetKeyParameter("WDS_VLANID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID = os_str_tol(macptr, 0, 10);
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_Priority = 0;
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("If/wds%d - WdsVlanId=%d\n", i, pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID));
		}
	}

#endif /* WDS_VLAN_SUPPORT */
}

VOID wds_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) {
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
		pTxBlk->pKey =  NULL;
	} else if (pMacEntry) {
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		pTxBlk->CipherAlg = pSecConfig->PairwiseCipher;
		pTxBlk->KeyIdx =  pSecConfig->PairwiseKeyId;
		if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
			pTxBlk->pKey = pSecConfig->WepKey[pSecConfig->PairwiseKeyId].Key;
		else
			pTxBlk->pKey = &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK];
	}

	/* For  BMcast pMacEntry is not initial */
	if (pTxBlk->CipherAlg == 0x0)
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
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
	INT if_idx = -1, count = 0, netdev_idx;
	PNET_DEV pWdsNetDev;
	struct wifi_dev *wdev, *main_wdev = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (band_idx >= DBDC_BAND_NUM)
		goto error_out;

	if (band_idx == DBDC_BAND0) {
		if_idx = 0;
		count = pAd->WdsTab.wds_num[DBDC_BAND0];
	} else if (band_idx == DBDC_BAND1) {
		if_idx = pAd->WdsTab.wds_num[DBDC_BAND0];
		count = pAd->WdsTab.wds_num[DBDC_BAND1];
	} else {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_OFF,
			 ("%s(): Invalid band(%d)\n", __func__, band_idx));
		goto error_out;
	}

	if ((if_idx+count) > MAX_WDS_ENTRY) {
		MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_OFF,
			 ("%s(): Invalid WDS amount of entries(%d, max=%d), try to alloac %d\n",
			__func__, if_idx+count, MAX_WDS_ENTRY, count));
		goto error_out;
	}

	MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_OFF,
		 ("%s():wds_num[%d]=%d, count=%d, MAX_WDS_ENTRY=%d, if_idx=%d, flg_wds_init=%d\n",
		  __func__, band_idx, count - if_idx, count, MAX_WDS_ENTRY, if_idx,
		  pAd->WdsTab.flg_wds_init[band_idx]));

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->WdsTab.flg_wds_init[band_idx] != FALSE) {
		for (; count ; if_idx++, count--) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode && (band_idx > DBDC_BAND0))
				main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
			else
#endif	/* DBDC_MODE */
				main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
			wds_entry = &pAd->WdsTab.WdsEntry[if_idx];
			wdev = &wds_entry->wdev;
			wdev->PhyMode = main_wdev->PhyMode;
			update_att_from_wdev(wdev, main_wdev);
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
			os_move_mem(wdev->if_addr, main_wdev->if_addr, MAC_ADDR_LEN);
			os_move_mem(wdev->bss_info_argument.Bssid, wdev->if_addr, MAC_ADDR_LEN);
		}

		return;
	}

	netdev_idx = 0;
	for (; count ; if_idx++, count--) {
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
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
						("Get name prefix fail\n"));
				break;
			}
		}

		pWdsNetDev = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_WDS, netdev_idx,
						sizeof(struct mt_dev_priv), dev_name, TRUE);

		wds_entry = &pAd->WdsTab.WdsEntry[if_idx];
		wdev = &wds_entry->wdev;

		if (pWdsNetDev == NULL) {
			/* allocation fail, exit */
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("Allocate network device fail (WDS)...\n"));
			break;
		}

		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_WDS, pWdsNetDev, if_idx, wds_entry, (VOID *)pAd);

		if (Ret == FALSE) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev)));
			RtmpOSNetDevFree(pWdsNetDev);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_WDS, &wds_wdev_ops,
								cap->qos.wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev)));
			RtmpOSNetDevFree(pWdsNetDev);
			break;
		}

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode && (band_idx > DBDC_BAND0))
			main_wdev = &pAd->ApCfg.MBSSID[pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]].wdev;
		else
#endif	/* DBDC_MODE */
			main_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

		if (pAd->WdsTab.Mode[band_idx] >= WDS_LAZY_MODE)
			wds_entry->phy_mode = get_wds_phymode(main_wdev, MODE_UNKNOWN);

		wdev->PhyMode = main_wdev->PhyMode;
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
		os_move_mem(wdev->if_addr, main_wdev->if_addr, MAC_ADDR_LEN);
		os_move_mem(wdev->bssid, wdev->if_addr, MAC_ADDR_LEN);
		os_move_mem(wdev->bss_info_argument.Bssid, wdev->if_addr, MAC_ADDR_LEN);
		RTMP_OS_NETDEV_SET_PRIV(pWdsNetDev, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pWdsNetDev, wdev);
		pNetDevOps->priv_flags = INT_WDS;
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		/* Register this device */
		RtmpOSNetDevAttach(pAd->OpMode, pWdsNetDev, pNetDevOps);
		netdev_idx++;
	}

	NdisAllocateSpinLock(pAd, &pAd->WdsTab.WdsTabLock);

	if (if_idx > 0) {
		NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
		pAd->WdsTab.flg_wds_init[band_idx] = TRUE;
		NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	}

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF,
		 ("Total allocated %d WDS(es) for band%d!\n", netdev_idx, band_idx));

error_out:
	if (if_idx < 0)
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("WDS initialized with invalid band index:%d\n", band_idx));
}


VOID WDS_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_OFF, ("%s():\n", __func__));

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		wdev = &pAd->WdsTab.WdsEntry[index].wdev;

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);
			/* Clear it as NULL to prevent latter access error. */
			pAd->WdsTab.flg_wds_init[HcGetBandByWdev(wdev)] = FALSE;
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
		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR, ("%s(): can not find wds I/F\n", __func__));
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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));
			return FALSE;
		}

		MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("WDS inf up for wds_%x(func_idx) OmacIdx=%d\n",
			wdev->func_idx, wdev->OmacIdx));

		RTMPSetIndividualHT(pAd, wdev->func_idx + MIN_NET_DEVICE_FOR_WDS);

		if (WDS_ENTRY_IS_ASSIGNED(wds_entry->flag)) {
			peer = MacTableInsertWDSEntry(pAd, wds_entry->PeerWdsAddr, wdev->func_idx);

			if (!peer) {
				MTWF_LOG(DBG_CAT_ALL, CATAP_WDS, DBG_LVL_ERROR, ("%s(): can't insert a new WDS entry!", __func__));
				return FALSE;
			}

			RAInit(pAd, peer);
			WDS_ENTRY_SET_VALID(wds_entry->flag);
			wds_entry->wdev.DevInfo.WdevActive = TRUE;
		}
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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): linkdown fail!!\n", __func__));
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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));
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
			MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_TRACE,
				("%s, AP WDS recv UC data from %02x-%02x-%02x-%02x-%02x-%02x\n",
				__func__, PRINT_MAC(pFrame->Hdr.Addr2)));

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

	MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): PhyMode = %d\n", __func__, Elem->RxPhyMode));

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
				MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_ERROR,
					("%s(): can't insert new pEntry\n", __func__));
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
		 MTWF_LOG(DBG_CAT_AP, CATAP_WDS, DBG_LVL_OFF, ("%s(): linkdown fail!!\n", __func__));

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

