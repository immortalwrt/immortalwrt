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
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"

#ifdef DOT11_HE_AX
UINT8 peer_max_bw_cap(INT8 ch_width_set);
#endif /*DOT11_HE_AX*/

#ifdef CONFIG_STA_SUPPORT
static BOOLEAN sta_rx_peer_response_allowed(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN is_my_bssid = FALSE, is_my_ssid = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

#ifdef AUTOMATION
	rx_peer_beacon_check(pAd, bcn_ie_list, Elem);
#endif /* AUTOMATION */

	is_my_bssid = MAC_ADDR_EQUAL(bcn_ie_list->Bssid, pStaCfg->Bssid) ? TRUE : FALSE;
	is_my_ssid = SSID_EQUAL(bcn_ie_list->Ssid, bcn_ie_list->SsidLen, pStaCfg->Ssid, pStaCfg->SsidLen) ? TRUE : FALSE;

	/* ignore BEACON not for my SSID */
	if ((!is_my_ssid) && (!is_my_bssid))
		return FALSE;

	/* It means STA waits disassoc completely from this AP, ignores this beacon. */
	if (cntl_do_disassoc_now(wdev))
		return FALSE;

	return TRUE;
}

static BOOLEAN sta_rx_peer_response_updated(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *bcn_ie_list, MLME_QUEUE_ELEM *Elem,
		NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
	ULONG Bssidx, Now;
	BSS_ENTRY *pBss;
	RSSI_SAMPLE rssi_sample;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	CHAR RealRssi = 0;
	BOOLEAN is_my_bssid = MAC_ADDR_EQUAL(bcn_ie_list->Bssid, pStaCfg->Bssid) ? TRUE : FALSE;
	UINT8 ant_num;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#ifdef CONFIG_MAP_SUPPORT
	struct DOT11_H *pDot11h = wdev->pDot11_H;
#endif

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	rssi_sample.AvgRssi[0] = Elem->rssi_info.raw_rssi[0];
	rssi_sample.AvgRssi[1] = Elem->rssi_info.raw_rssi[1];
	rssi_sample.AvgRssi[2] = Elem->rssi_info.raw_rssi[2];
	rssi_sample.AvgRssi[3] = Elem->rssi_info.raw_rssi[3];
	RealRssi = rtmp_avg_rssi(pAd, &rssi_sample);
#ifdef APCLI_SUPPORT
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
		ApCliCheckPeerExistence(pAd, bcn_ie_list->Ssid, bcn_ie_list->SsidLen, bcn_ie_list->Bssid, bcn_ie_list->Channel);
#else
		ApCliCheckPeerExistence(pAd, bcn_ie_list->Ssid, bcn_ie_list->SsidLen, bcn_ie_list->Channel);
#endif

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		pEntry = &pAd->MacTab.Content[Elem->Wcid];

		if (pEntry &&
			(IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd) && (wdev->channel == bcn_ie_list->Channel)) {
				pStaCfg->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
				if (pDot11h && pDot11h->ChChangeCSA)
					pDot11h->ChChangeCSA = FALSE;
			} else if (!IS_MAP_ENABLE(pAd))
				pStaCfg->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
#else
			pStaCfg->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
#endif
#ifdef CONFIG_MAP_SUPPORT
			if (!IS_MAP_CERT_ENABLE(pAd))
				AdjustBwToSyncAp(pAd, bcn_ie_list, &pStaCfg->wdev);
#else
			AdjustBwToSyncAp(pAd, bcn_ie_list, &pStaCfg->wdev);
#endif
			ApCliCheckConConnectivity(pAd, pStaCfg, bcn_ie_list);
			{
				UCHAR RegClass;
				OVERLAP_BSS_SCAN_IE BssScan;
				BOOLEAN brc;
#ifdef DOT11_N_SUPPORT
				ADD_HT_INFO_IE *aux_add_ht = &pStaCfg->MlmeAux.AddHtInfo;
				ADD_HT_INFO_IE *addht;
				BOOLEAN bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;
				UINT16 OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
#endif /* DOT11_N_SUPPORT */

				NdisZeroMemory(&BssScan, sizeof(OVERLAP_BSS_SCAN_IE));
				brc = PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, bcn_ie_list, &RegClass);

				if (brc == TRUE) {
					pAd->CommonCfg.Dot11BssWidthTriggerScanInt = le2cpu16(BssScan.TriggerScanInt); /*APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];*/

					/*DBGPRINT(RT_DEBUG_ERROR,("Update Dot11BssWidthTriggerScanInt=%d\n", pAd->CommonCfg.Dot11BssWidthTriggerScanInt)); */
					/* out of range defined in MIB... So fall back to default value.*/
					if ((pAd->CommonCfg.Dot11BssWidthTriggerScanInt < 10) || (pAd->CommonCfg.Dot11BssWidthTriggerScanInt > 900))
						pAd->CommonCfg.Dot11BssWidthTriggerScanInt = 900;
				}
#ifdef DOT11_N_SUPPORT
				/* check Ht protection mode. and adhere to the Non-GF device indication by AP. */
				if (HAS_HT_OP_EXIST(bcn_ie_list->cmm_ies.ie_exists)) {
					if ((bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.OperaionMode != OperationMode)
						|| (bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.NonGfPresent != bNonGFExist)) {
						aux_add_ht->AddHtInfo2.OperaionMode =
							bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.OperaionMode;
						aux_add_ht->AddHtInfo2.NonGfPresent =
							bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.NonGfPresent;

						pStaCfg->wdev.protection =
							SET_PROTECT(bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.OperaionMode);

						OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
						bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;

						if (bNonGFExist) {
							pStaCfg->wdev.protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
						} else {
							pStaCfg->wdev.protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));
						}

						if (pStaCfg->wdev.channel > 14) {
							/* always no BG protection in A-band.
							* falsely happened when switching A/G band to a dual-band AP */
							pStaCfg->wdev.protection &= ~(SET_PROTECT(ERP));
						}
						addht = wlan_operate_get_addht(&pStaCfg->wdev);
						if (addht) { /* sync addht information into wlan operation addht */
							*addht = pStaCfg->MlmeAux.AddHtInfo;
						}

						HW_SET_PROTECT(pAd, &pStaCfg->wdev, PROT_PROTOCOL, 0, 0);

						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"SYNC - AP changed N OperaionMode to %d, my protection to %d\n",
							OperationMode, pStaCfg->wdev.protection);
					}
				}
#endif /* DOT11_N_SUPPORT */
			}
		}

		if (pEntry && bcn_ie_list->NewChannel != 0) {
#ifdef CONFIG_RCSA_SUPPORT
			if (pAd->CommonCfg.DfsParameter.bRCSAEn) {
				pAd->CommonCfg.DfsParameter.fSendRCSA = FALSE;
				channel_switch_action_1(pAd, &bcn_ie_list->CsaInfo);
			} else
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
			{
				/* CSA SYNC / TSF SYNC */
				if (pAd->Zero_Loss_Enable && bcn_ie_list->CsaInfo.ChSwAnnIE.ChSwCnt
					&& (pDot11h->RootApCSCountFlag == FALSE)) {
					pDot11h->OriCSCount = pDot11h->CSPeriod;
					pDot11h->CSPeriod = bcn_ie_list->CsaInfo.ChSwAnnIE.ChSwCnt - 1;
					pDot11h->RootApCSCountFlag = TRUE;

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"SYNC - CSA Count Orig = %d, Updated CSA Count = %d\n",
						pDot11h->OriCSCount, pDot11h->CSPeriod);
				}

				ApCliPeerCsaAction(pAd, pEntry->wdev, bcn_ie_list);
			}
#else
				ApCliPeerCsaAction(pAd, pEntry->wdev, bcn_ie_list);
#endif
		}
	}

#endif /* APCLI_SUPPORT */
#ifdef DOT11_N_SUPPORT

	/* Copy Control channel for this BSSID. */
	if (HAS_HT_OP_EXIST(bcn_ie_list->cmm_ies.ie_exists))
		bcn_ie_list->Channel = bcn_ie_list->cmm_ies.ht_op.ControlChan;

	if (HAS_HT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists) || HAS_PREN_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists))
		SET_HT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists);

#endif /* DOT11_N_SUPPORT */
	/* Housekeeping "SsidBssTab" table for later-on ROAMing usage. */
	Bssidx = BssTableSearchWithSSID(&pStaCfg->MlmeAux.SsidBssTab, bcn_ie_list->Bssid,
									bcn_ie_list->Ssid, bcn_ie_list->SsidLen, bcn_ie_list->Channel);

	if (Bssidx == BSS_NOT_FOUND) {
		/* discover new AP of this network, create BSS entry */
		Bssidx = BssTableSetEntry(pAd, wdev, &pStaCfg->MlmeAux.SsidBssTab, bcn_ie_list, RealRssi, LenVIE, pVIE);

		if (Bssidx != BSS_NOT_FOUND) {
			BSS_ENTRY *pBssEntry = &pStaCfg->MlmeAux.SsidBssTab.BssEntry[Bssidx];

			NdisMoveMemory(&pBssEntry->PTSF[0], &Elem->Msg[24], 4);
			NdisMoveMemory(&pBssEntry->TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pBssEntry->TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			pBssEntry->Rssi = RealRssi;
			NdisMoveMemory(pBssEntry->MacAddr, bcn_ie_list->Addr2, MAC_ADDR_LEN);
		}
	}

	/* Update ScanTab */
	Bssidx = BssTableSearch(ScanTab, bcn_ie_list->Bssid, bcn_ie_list->Channel);

	if (Bssidx == BSS_NOT_FOUND) {
		/* discover new AP of this network, create BSS entry */
		Bssidx = BssTableSetEntry(pAd, wdev, ScanTab, bcn_ie_list, RealRssi, LenVIE, pVIE);

		if (Bssidx == BSS_NOT_FOUND) /* return if BSS table full */
			return FALSE;
		else {
			NdisMoveMemory(ScanTab->BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&ScanTab->BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&ScanTab->BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			ScanTab->BssEntry[Bssidx].MinSNR = Elem->Signal % 10;

			if (ScanTab->BssEntry[Bssidx].MinSNR == 0)
				ScanTab->BssEntry[Bssidx].MinSNR = -5;

			NdisMoveMemory(ScanTab->BssEntry[Bssidx].MacAddr, bcn_ie_list->Addr2, MAC_ADDR_LEN);
		}

#ifdef RT_CFG80211_SUPPORT

		/* Determine primary channel by IE's DSPS rather than channel of received frame */
		if (bcn_ie_list->Channel != 0)
			Elem->Channel = bcn_ie_list->Channel;

		RT_CFG80211_SCANNING_INFORM(pAd, Bssidx, Elem->Channel, Elem->Msg,
									Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_SUPPORT */
	}
	NdisGetSystemUpTime(&Now);
	pBss = &ScanTab->BssEntry[Bssidx];
	pBss->Rssi = RealRssi;          /* lastest RSSI */
	pBss->LastBeaconRxTime = Now;   /* last RX timestamp */

	/*
		if the ssid matched & bssid unmatched, we should select the bssid with large value.
		This might happened when two STA start at the same time
	*/
	if ((!is_my_bssid) && ADHOC_ON(pAd)) {
		INT i;
#ifdef IWSC_SUPPORT

		if ((pStaCfg->WscControl.WscConfMode != WSC_DISABLE) &&
			(pStaCfg->WscControl.bWscTrigger == TRUE))
			;
		else
#endif /* IWSC_SUPPORT */

			/* Add the safeguard against the mismatch of adhoc wep status */
			if ((wdev->SecConfig.PairwiseCipher != ScanTab->BssEntry[Bssidx].PairwiseCipher) ||
				(wdev->SecConfig.AKMMap != ScanTab->BssEntry[Bssidx].AKMMap)) {
				return FALSE;
			}

		/* collapse into the ADHOC network which has bigger BSSID value. */
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			if (bcn_ie_list->Bssid[i] > pStaCfg->Bssid[i]) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SYNC - merge to the IBSS with bigger BSSID="MACSTR"\n",
						 MAC2STR(bcn_ie_list->Bssid));
				AsicDisableSync(pAd, HW_BSSID_0);
				COPY_MAC_ADDR(pStaCfg->Bssid, bcn_ie_list->Bssid);

				if (wdev_do_linkdown(wdev) != TRUE)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkdown fail!!\n");

				if (wdev_do_linkup(wdev, pEntry) != TRUE)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkup fail!!\n");

				if (wdev_do_conn_act(wdev, pEntry) != TRUE)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "connect fail!!\n");

				is_my_bssid = TRUE;
				break;
			} else if (bcn_ie_list->Bssid[i] < pStaCfg->Bssid[i])
				break;
		}
	}

	/*
		BEACON from my BSSID - either IBSS or INFRA network
	*/
	if (is_my_bssid) {
		UINT link_down_type = 0;
		struct rx_signal_info signal;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		OVERLAP_BSS_SCAN_IE	BssScan;
		UCHAR RegClass = 0;
		BOOLEAN	brc = FALSE;
		/* Read Beacon's Reg Class IE if any. */
		NdisZeroMemory(&BssScan, sizeof(OVERLAP_BSS_SCAN_IE));
		brc = PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, bcn_ie_list, &RegClass);

		if (brc == TRUE) {
			UpdateBssScanParm(pAd, BssScan);
			pStaCfg->RegClass = RegClass;
		}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
		NdisZeroMemory(&signal, sizeof(struct rx_signal_info));
		pStaCfg->BeaconPeriod = bcn_ie_list->BeaconPeriod;
		pStaCfg->DtimPeriod = bcn_ie_list->DtimPeriod;
		pStaCfg->DtimCount = bcn_ie_list->DtimCount;
		pStaCfg->LastBeaconRxTime = Now;
		signal.raw_rssi[0] = Elem->rssi_info.raw_rssi[0];
		signal.raw_rssi[1] = Elem->rssi_info.raw_rssi[1];
		signal.raw_rssi[2] = Elem->rssi_info.raw_rssi[2];
		signal.raw_rssi[3] = Elem->rssi_info.raw_rssi[3];

		if (INFRA_ON(pStaCfg)) {
			if (pEntry)
				Update_Snr_Sample(pAd, &pEntry->RssiSample, &signal, 0, BW_20);
		}

		Update_Snr_Sample(pAd, &pStaCfg->RssiSample, &signal, 0, BW_20);
		ant_num = pAd->Antenna.field.RxPath;
		Update_Rssi_Sample(pAd, pStaCfg->BcnRssiLast, pStaCfg->BcnRssiAvg, ant_num, &signal);

		if ((pAd->CommonCfg.bIEEE80211H == 1) &&
			(bcn_ie_list->NewChannel != 0) &&
			(bcn_ie_list->Channel != bcn_ie_list->NewChannel)
			&& (wdev->quick_ch_change == QUICK_CH_SWICH_DISABLE)
#ifdef MAP_R2
			&& (IS_MAP_TURNKEY_ENABLE(pAd) && (wdev->channel != bcn_ie_list->NewChannel))
#endif
			) {
			UCHAR index = 0;
			/*
			   Switching to channel 1 can prevent from rescanning the current channel immediately (by auto reconnection).
			   In addition, clear the MLME queue and the scan table to discard the RX packets and previous scanning results.
			 */
			wdev->channel  = 1;
			wlan_operate_set_prim_ch(wdev, 1);
			LinkDown(pAd, link_down_type, &pStaCfg->wdev, NULL);
			MlmeResetByWdev(pAd, wdev);
			BssTableInit(ScanTab);
			RtmpusecDelay(1000000);	/* use delay to prevent STA do reassoc */

			/* channel sanity check */
			for (index = 0 ; index < pChCtrl->ChListNum; index++) {
				if (pChCtrl->ChList[index].Channel == bcn_ie_list->NewChannel) {
					ScanTab->BssEntry[Bssidx].Channel = bcn_ie_list->NewChannel;
					wdev->channel = bcn_ie_list->NewChannel;
					wlan_operate_set_prim_ch(wdev, wdev->channel);
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "PeerBeacon - STA receive channel switch announcement IE (New Channel =%d)\n", bcn_ie_list->NewChannel);
#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
					if (wdev->if_dev)
						wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), bcn_ie_list->NewChannel);
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */
					break;
				}
			}

			if (index >= pChCtrl->ChListNum) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "PeerBeacon(can not find New Channel=%d in ChannelList[%d]\n", pStaCfg->wdev.channel, pChCtrl->ChListNum);
			}
		}

#ifdef WPA_SUPPLICANT_SUPPORT

		if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
			;
		else
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT
			if (wdev->WscControl.WscState == WSC_STATE_OFF)
#endif /* WSC_STA_SUPPORT */
			{
				if (((((IS_SECURITY_Entry(wdev)) << 4) ^ bcn_ie_list->CapabilityInfo) & 0x0010)
					&& (!((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (!CAP_IS_PRIVACY_ON(bcn_ie_list->CapabilityInfo))))
				   ) {
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT

					/* When using -Dwext and trigger WPS, do not check security. */
					if (bcn_ie_list->selReg == 0)
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
					{
						/*
							To prevent STA connect to OPEN/WEP AP when STA is OPEN/NONE or
							STA connect to OPEN/NONE AP when STA is OPEN/WEP AP.
						*/
						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:AP privacy:%x is differenct from STA privacy:%x\n",
								 __func__, (bcn_ie_list->CapabilityInfo & 0x0010) >> 4,
								 IS_SECURITY_Entry(wdev));

						if (INFRA_ON(pStaCfg)) {
							LinkDown(pAd, link_down_type, &pStaCfg->wdev, NULL);
							BssTableInit(ScanTab);
						}

						return FALSE;
					}
				}
			}


		if (bcn_ie_list->AironetCellPowerLimit != 0xFF) {
			/*
			   We get the Cisco (ccx) "TxPower Limit" required
			   Changed to appropriate TxPower Limit for Ciso Compatible Extensions
			*/
			ChangeToCellPowerLimit(pAd, bcn_ie_list->AironetCellPowerLimit);
		} else {
			/*
			   AironetCellPowerLimit equal to 0xFF means the Cisco (ccx) "TxPower Limit" not exist.
			   Used the default TX Power Percentage, that set from UI.
			*/
			pAd->CommonCfg.ucTxPowerPercentage[BAND0] = pAd->CommonCfg.ucTxPowerDefault[BAND0];
#ifdef DBDC_MODE
			pAd->CommonCfg.ucTxPowerPercentage[BAND1] = pAd->CommonCfg.ucTxPowerDefault[BAND1];
#endif /* DBDC_MODE */

		}

#ifdef CONFIG_STA_ADHOC_SUPPORT /* snowpin for ap/sta */

		if (adhoc_add_peer_from_beacon(pAd, wdev, bcn_ie_list, pVIE, LenVIE) == FALSE)
			return FALSE;

#endif /* CONFIG_STA_ADHOC_SUPPORT */

		if (INFRA_ON(pStaCfg)) {
			BOOLEAN bUseShortSlot;
			BOOLEAN bUseBGProtection;
			BOOLEAN bNonGFExist =
				(pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent) ? TRUE : FALSE;
			UINT16 OperationMode =
				pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;
			/*
			   decide to use/change to -
				  1. long slot (20 us) or short slot (9 us) time
				  2. turn on/off RTS/CTS and/or CTS-to-self protection
				  3. short preamble
			*/
			/* bUseShortSlot = pAd->CommonCfg.bUseShortSlotTime && CAP_IS_SHORT_SLOT(CapabilityInfo); */
			bUseShortSlot = CAP_IS_SHORT_SLOT(bcn_ie_list->CapabilityInfo);

			if (bUseShortSlot != STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_SHORT_SLOT_INUSED))
				HW_SET_SLOTTIME(pAd, bUseShortSlot, wdev->channel, wdev);

			bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
							   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(bcn_ie_list->Erp));

			if (wdev->channel > 14) {
				/* always no BG protection in A-band.
				 * falsely happened when switching A/G band to a dual-band AP */
				bUseBGProtection = FALSE;
			}

			if ((wdev->channel <= 14) && (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))) {
				if (bUseBGProtection) {
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
					wdev->protection |= SET_PROTECT(ERP);
				} else {
					OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
					wdev->protection &= ~(SET_PROTECT(ERP));
				}

				HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "SYNC - AP changed B/G protection to %d\n", bUseBGProtection);
			}

#ifdef DOT11_N_SUPPORT

			/* check Ht protection mode. and adhere to the Non-GF device indication by AP. */
			if (HAS_HT_OP_EXIST(bcn_ie_list->cmm_ies.ie_exists)) {
				if ((bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.OperaionMode != OperationMode)
					|| (bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.NonGfPresent != bNonGFExist)) {
					pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode
						= bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.OperaionMode;
					pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent
						= bcn_ie_list->cmm_ies.ht_op.AddHtInfo2.NonGfPresent;
					OperationMode = pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;
					bNonGFExist =
						(pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent) ? TRUE : FALSE;

					if (bNonGFExist)
						wdev->protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
					else
						wdev->protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));

					HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "SYNC - AP changed N OperaionMode to %d\n",
							  OperationMode);
				}
			}

#endif /* DOT11_N_SUPPORT */

			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) &&
				ERP_IS_USE_BARKER_PREAMBLE(bcn_ie_list->Erp)) {
				MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SYNC - AP forced to use LONG preamble\n");
			}

			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)	  &&
				(bcn_ie_list->EdcaParm.bValid == TRUE) &&
				(bcn_ie_list->EdcaParm.EdcaUpdateCount != pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SYNC - AP change EDCA parameters(from %d to %d)\n",
						 pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount,
						 bcn_ie_list->EdcaParm.EdcaUpdateCount);
				HcAcquiredEdca(pAd, wdev, &bcn_ie_list->EdcaParm);
				HcSetEdca(wdev);
				pStaCfg->MlmeAux.APEdcaParm.EdcaUpdateCount = bcn_ie_list->EdcaParm.EdcaUpdateCount;
			}

			/* copy QOS related information */
			NdisMoveMemory(&pAd->CommonCfg.APQbssLoad, &bcn_ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
			NdisMoveMemory(&pAd->CommonCfg.APQosCapability, &bcn_ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

			/*
			   2009: PF#1: 20/40 Coexistence in 2.4 GHz Band
			   When AP changes "STA Channel Width" and "Secondary Channel Offset" fields of HT Operation Element in the Beacon to 0
			*/
#ifdef CONFIG_MAP_SUPPORT
			if (!IS_MAP_CERT_ENABLE(pAd)) {
				if (INFRA_ON(pStaCfg)) {
					BOOLEAN bChangeBW = FALSE;
					bChangeBW = AdjustBwToSyncAp(pAd, bcn_ie_list, wdev);
					if (bChangeBW) {
						pAd->CommonCfg.BSSCoexist2040.word = 0;
						TriEventInit(pAd);
						BuildEffectedChannelList(pAd, wdev);
					}
				}
			}
#else
			if (INFRA_ON(pStaCfg)) {
				BOOLEAN bChangeBW = FALSE;

				bChangeBW = AdjustBwToSyncAp(pAd, bcn_ie_list, wdev);

				if (bChangeBW) {
					pAd->CommonCfg.BSSCoexist2040.word = 0;
					TriEventInit(pAd);
					BuildEffectedChannelList(pAd, wdev);
				}
			}
#endif
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

			if ((pEntry) && ((pEntry->force_op_mode != pStaCfg->MlmeAux.force_op_mode) ||
							 (NdisCmpMemory(&pEntry->operating_mode, &pStaCfg->MlmeAux.op_mode, 1) != 0))) {
				CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

				pStaCfg->MlmeAux.force_op_mode = pEntry->force_op_mode;
				NdisMoveMemory(&pStaCfg->MlmeAux.op_mode, &pEntry->operating_mode, 1);
				NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
				rRaParam.u4Field = RA_PARAM_VHT_OPERATING_MODE;
				RAParamUpdate(pAd, pEntry, &rRaParam);
			}

#endif /* DOT11_VHT_AC */
		}

		/* only INFRASTRUCTURE mode support power-saving feature */
#ifdef STA_LP_PHASE_2_SUPPORT
		/* MT7636 STA Legacy PS (PS-POLL) and WMM PS is offload to FW */
#else

		if ((INFRA_ON(pStaCfg) && (RtmpPktPmBitCheck(pAd, pStaCfg) == TRUE)) ||
			(pAd->CommonCfg.bAPSDForcePowerSave)) {
			/*
				 1. AP has backlogged unicast-to-me frame, stay AWAKE, send PSPOLL
				 2. AP has backlogged broadcast/multicast frame and we want those frames, stay AWAKE
				 3. we have outgoing frames in TxRing or MgmtRing, better stay AWAKE
				 4. Psm change to PWR_SAVE, but AP not been informed yet, we better stay AWAKE
				 5. otherwise, put PHY back to sleep to save battery.
			*/
			if (bcn_ie_list->MessageToMe) {
#ifdef UAPSD_SUPPORT
				if (pStaCfg->wdev.UapsdInfo.bAPSDCapable &&
					pAd->CommonCfg.APEdcaParm[0].bAPSDCapable &&
					pAd->CommonCfg.bAPSDAC_BE &&
					pAd->CommonCfg.bAPSDAC_BK &&
					pAd->CommonCfg.bAPSDAC_VI &&
					pAd->CommonCfg.bAPSDAC_VO) {
					pAd->CommonCfg.bNeedSendTriggerFrame = TRUE;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
					RTMPSendNullFrame(pAd, (MAC_TABLE_ENTRY *)pStaCfg->pAssociatedAPEntry,
									  pAd->CommonCfg.TxRate, TRUE, pStaCfg->PwrMgmt.Psm);
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				} else
#endif /* UAPSD_SUPPORT */
				{
					if (pStaCfg->WindowsBatteryPowerMode == Ndis802_11PowerModeFast_PSP) {
						MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);
						/* wake up and send a NULL frame with PM = 0 to the AP */
						RTMP_SET_PSM_BIT(pAd, pStaCfg, PWR_ACTIVE);
						RTMPSendNullFrame(pAd, pEntry, pAd->CommonCfg.TxRate,
										  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE : FALSE), PWR_ACTIVE);
					} else {
						/* use PS-Poll to get any buffered packet */
						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:: Send PS-POLL to retrieve buffered data\n", __func__);
						ComposePsPoll(pAd, &(pAd->PsPollFrame), pStaCfg->StaActive.Aid,
									  pStaCfg->Bssid, pStaCfg->wdev.if_addr);
						RTMP_PS_POLL_ENQUEUE(pAd, pStaCfg);
					}
				}
			} else if (bcn_ie_list->BcastFlag && (bcn_ie_list->DtimCount == 0) &&
					   OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM)) {
			} else {
				if ((pAd->CommonCfg.bACMAPSDTr[QID_AC_VO]) ||
					(pAd->CommonCfg.bACMAPSDTr[QID_AC_VI]) ||
					(pAd->CommonCfg.bACMAPSDTr[QID_AC_BK]) ||
					(pAd->CommonCfg.bACMAPSDTr[QID_AC_BE])
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
					|| (pStaCfg->FlgPsmCanNotSleep == TRUE)
					|| (RtmpPktPmBitCheck(pAd, pStaCfg) == FALSE)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				   ) {
				} else {
					USHORT NextDtim = bcn_ie_list->DtimCount;
					USHORT TbttNumToNextWakeUp;

					if (NextDtim == 0)
						NextDtim = bcn_ie_list->DtimPeriod;

					TbttNumToNextWakeUp = pStaCfg->DefaultListenCount;

					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
						TbttNumToNextWakeUp = NextDtim;


					if (!pStaCfg->PwrMgmt.bDoze) {
						/* Set a flag to go to sleep . Then after parse this RxDoneInterrupt, will go to sleep mode. */
						pStaCfg->ThisTbttNumToNextWakeUp = TbttNumToNextWakeUp;
						AsicSleepAutoWakeup(pAd, pStaCfg);
					}
				}
			}
		}

#endif /* STA_LP_PHASE_2_SUPPORT */
#ifdef CFG_TDLS_SUPPORT

		if (INFRA_ON(pAd) && (bcn_ie_list->DtimCount == 0) &&
			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bDoingPeriodChannelSwitch &&
			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bChannelSwitchInitiator) {
			BOOLEAN TimerCancelled;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "101. CFG TDLS DTIM %ld !!!\n", (jiffies * 1000) / OS_HZ);
			RTMPCancelTimer(&pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelSwitchTimer, &TimerCancelled);
			RTMPSetTimer(&pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelSwitchTimer, pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelStayTime);

			if (bcn_ie_list->MessageToMe)
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "301. %ld DTIM MessageToMe !!!\n", (jiffies * 1000) / OS_HZ);

			if (pStaCfg->wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%ld Recieve Orig AP Beacon but FW didn't inform Back to Base, force back to base !!!\n", (jiffies * 1000) / OS_HZ);
				pStaCfg->wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel = FALSE;
			}
		}

#endif /* CFG_TDLS_SUPPORT // */
	}

	return TRUE;
}

static BOOLEAN sta_probe_response_allowed(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isNeedRsp = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	/*ASSERT(pStaCfg);*/

	if (!ADHOC_ON(pAd))
		return FALSE;

	if ((ProbeReqParam->SsidLen == 0) ||
		SSID_EQUAL(ProbeReqParam->Ssid, ProbeReqParam->SsidLen, pStaCfg->Ssid, pStaCfg->SsidLen))
		isNeedRsp = TRUE;

	return isNeedRsp;
}

static BOOLEAN sta_probe_response_xmit(struct _RTMP_ADAPTER *pAd,
									   struct wifi_dev *wdev, struct _PEER_PROBE_REQ_PARAM *ProbeReqParam, MLME_QUEUE_ELEM *Elem)
{
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS   NStatus;
	PUCHAR		  pOutBuffer = NULL;
	ULONG		  FrameLen = 0;
	ULONG		  tmp_len = 0;
	LARGE_INTEGER FakeTimestamp;
	UCHAR		  DsLen = 1, IbssLen = 2;
	UCHAR		  LocalErpIe[3] = {IE_ERP, 1, 0};
	BOOLEAN	  Privacy;
	USHORT		  CapabilityInfo;
	struct _build_ie_info ie_info = {0};
	ADD_HT_INFO_IE *addht;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	struct legacy_rate *rate = &pStaCfg->StaActive.rate;

	/*ASSERT(pStaCfg);*/

	if (!pStaCfg)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = wdev->PhyMode;
	ie_info.wdev = wdev;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS)
		return FALSE;

	MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP,
					 0, ProbeReqParam->Addr2,
					 pStaCfg->wdev.if_addr,
					 pStaCfg->Bssid);
	Privacy = IS_SECURITY(&wdev->SecConfig);
	CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);
	MakeOutgoingFrame(pOutBuffer,                   &FrameLen,
					  sizeof(HEADER_802_11),        &ProbeRspHdr,
					  TIMESTAMP_LEN,                &FakeTimestamp,
					  2,                            &pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
					  2,                            &CapabilityInfo,
					  1,                            &SsidIe,
					  1,                            &pStaCfg->SsidLen,
					  pStaCfg->SsidLen,             pStaCfg->Ssid,
					  END_OF_ARGS);

	FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, pOutBuffer + FrameLen);

	MakeOutgoingFrame(pOutBuffer + FrameLen,                   &tmp_len,
					  1,                            &DsIe,
					  1,                            &DsLen,
					  1,                            &wdev->channel,
					  1,                            &IbssIe,
					  1,                            &IbssLen,
					  2,                            &pStaCfg->StaActive.AtimWin,
					  END_OF_ARGS);
	FrameLen += tmp_len;

	FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
				rate->ext_rate, rate->ext_rate_len, pOutBuffer + FrameLen);

	{
		ULONG tmp;

		MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
						  3,                            LocalErpIe,
						  END_OF_ARGS);
		FrameLen += tmp;
	}
	FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
				rate->ext_rate, rate->ext_rate_len, pOutBuffer + FrameLen);

	FrameLen += build_rsn_ie(pAd, wdev, (UCHAR *)(pOutBuffer + FrameLen));
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);
	}

#endif /* DOT11_N_SUPPORT */
#ifdef WSC_STA_SUPPORT
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_STA_SUPPORT */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}

static BOOLEAN sta_join_peer_response_matched(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN ssidEqualFlag = FALSE;
	BOOLEAN ssidEmptyFlag = FALSE;
	BOOLEAN bssidEqualFlag = FALSE;
	BOOLEAN bssidEmptyFlag = FALSE;
	BOOLEAN matchFlag = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ULONG Bssidx;
	BSS_ENTRY *pInBss;
	struct legacy_rate *rate = &ie_list->cmm_ies.rate;

	/*ASSERT(pStaCfg);*/
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (!pStaCfg)
		return FALSE;

	/* Disqualify 11b only adhoc when we are in 11g only adhoc mode */
	if ((ie_list->BssType == BSS_ADHOC) && WMODE_EQUAL(wdev->PhyMode, WMODE_G) &&
		((rate->sup_rate_len + rate->ext_rate_len) < 12))
		return FALSE;

	if (!MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ZERO_MAC_ADDR) &&
		!MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ie_list->Bssid)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "ERROR: AP BSSID not equal\n");
		return FALSE;
	}

	/* Check the Probe-Rsp's Bssid. */
	if (!MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ZERO_MAC_ADDR))
		bssidEqualFlag = MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ie_list->Bssid);
	else
		bssidEmptyFlag = TRUE;

#ifdef WSC_AP_SUPPORT
	if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger == TRUE)) {
		if (wdev->WscControl.WscSsid.SsidLength != 0)
			ssidEqualFlag = SSID_EQUAL(wdev->WscControl.WscSsid.Ssid,
						wdev->WscControl.WscSsid.SsidLength, ie_list->Ssid, ie_list->SsidLen);
		else
			ssidEmptyFlag = TRUE;
	}
#endif /* WSC_AP_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
		if (pStaCfg->owe_trans_ssid_len > 0) {
			ssidEqualFlag = SSID_EQUAL(pStaCfg->owe_trans_ssid, pStaCfg->owe_trans_ssid_len, ie_list->Ssid, ie_list->SsidLen);
			bssidEqualFlag = MAC_ADDR_EQUAL(pStaCfg->owe_trans_bssid, ie_list->Bssid);
			if (ssidEqualFlag == TRUE)
				ssidEmptyFlag = FALSE;
			if (bssidEqualFlag == TRUE)
				bssidEmptyFlag = FALSE;
		} else {
#endif
				if (ssidEqualFlag == FALSE) {
					/* Check the Probe-Rsp's Ssid. */
					if (pStaCfg->CfgSsidLen != 0) {
						if ((ie_list->SsidLen == 0) && (bssidEqualFlag == TRUE)) {
							ssidEqualFlag = TRUE;
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "SYNC - Probe-Rsp's Ssid is NULL But Bssid was matched, So set ssidEqualFlag=1 \n");
						} else
							ssidEqualFlag = SSID_EQUAL(pStaCfg->CfgSsid, pStaCfg->CfgSsidLen, ie_list->Ssid, ie_list->SsidLen);
					}
					else
						ssidEmptyFlag = TRUE;
				}
#ifdef CONFIG_OWE_SUPPORT
		}
#endif

	/* By config way to set SSID/BSSID */
	if ((ssidEmptyFlag == FALSE) || (bssidEmptyFlag == FALSE)) {
#ifdef WSC_AP_SUPPORT

		if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
			(wdev->WscControl.bWscTrigger == TRUE)) {

			/* BSSID match */
			if (bssidEqualFlag)
				matchFlag = TRUE;
			/* ssid match but bssid doesn't be indicate. */
			else if (ssidEqualFlag && bssidEmptyFlag)
				matchFlag = TRUE;

		} else
#endif /* WSC_AP_SUPPORT */
		{
			if (!bssidEmptyFlag) {
				if (bssidEqualFlag && ssidEqualFlag)
					matchFlag = TRUE;
				else
					matchFlag = FALSE;
			} else if (ssidEqualFlag)
				matchFlag = TRUE;
			else
				matchFlag = FALSE;
		}

		/* if the Bssid doesn't be indicated then you need to decide which AP to connect by most strong Rssi signal strength. */
		if ((matchFlag == TRUE) && (bssidEqualFlag == FALSE)) {
			UCHAR RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
										 ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
										 ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "SYNC - previous Rssi = %ld current Rssi=%ld\n", pStaCfg->MlmeAux.Rssi, (LONG)RealRssi);

			if (pStaCfg->MlmeAux.Rssi > (LONG)RealRssi)
				return FALSE;
			else
				pStaCfg->MlmeAux.Rssi = RealRssi;
		}

		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "SYNC - bssidEqualFlag=%d, ssidEqualFlag=%d, matchFlag=%d\n",
				 bssidEqualFlag, ssidEqualFlag, matchFlag);
	} else {
		if (MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, ie_list->Bssid))
			matchFlag = TRUE;
	}

	if (ie_list->Channel != pStaCfg->MlmeAux.Channel) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "SYNC - current ie channel=%d, apcli channel=%d!\n",
				 ie_list->Channel, pStaCfg->MlmeAux.Channel);
		return FALSE;
	}

	if ((ScanTab != NULL)
#ifdef WSC_STA_SUPPORT
		&&
			((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
			 (wdev->WscControl.bWscTrigger == FALSE))
#endif
			 ) {
		Bssidx = BssTableSearch(ScanTab, ie_list->Bssid, ie_list->Channel);
		if (Bssidx != BSS_NOT_FOUND) {
			pInBss = &ScanTab->BssEntry[Bssidx];
#ifdef CONFIG_OWE_SUPPORT
			if (matchFlag) {
				if (sta_handle_owe_trans(pAd, wdev, pInBss))
					return FALSE;
				else
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "OWE TRANS IE HANDLING FAILED OR NOT REQUIRED\n");
			}
#endif
			if ((!((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pInBss->AKMMap)))) &&
				(!((IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) &&
				   (IS_AKM_WPA2PSK_ONLY(pInBss->AKMMap)) &&
				   ((wdev->SecConfig.PairwiseCipher & pInBss->PairwiseCipher) != 0))) &&
				(((wdev->SecConfig.AKMMap & pInBss->AKMMap) == 0)
					|| ((wdev->SecConfig.PairwiseCipher & pInBss->PairwiseCipher) == 0))) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"RSN IE VALIDATE FAIL\n");
				return FALSE;
			}

			if (WMODE_CAP_6G(wdev->PhyMode)) {
				/* 6G band only support WPA3PSK, DPP or OWE. */
				if (!IS_AKM_OWE(pInBss->AKMMap) &&
					!((IS_AKM_WPA3PSK(pInBss->AKMMap)
#ifdef MAP_R3_SUPPORT
					|| IS_AKM_DPP(pInBss->AKMMap)
#endif
					)
					&& !IS_AKM_WPA2PSK(pInBss->AKMMap))) {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Only support DPP,WPA3 SAE or WPA3 OWE for 6G band\n");
					return FALSE;
				}

				/* For WiFi 6E cert 5.2.4_6G, cannot connect to AP with WPA3PSK SAE with Hunting and Pecking. */
				if (IS_AKM_WPA3PSK(pInBss->AKMMap) && !IS_AKM_WPA2PSK(pInBss->AKMMap) &&
					(pInBss->sae_conn_type != SAE_CONNECTION_TYPE_H2E)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"WPA3 SAE connection type is not hash to element (sae_conn_type: %x)\n",
						pInBss->sae_conn_type);
					return FALSE;
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RSN IE VALIDATE FAIL -2 \n");
		}
	}
	return matchFlag;
}

static BOOLEAN sta_join_peer_response_updated(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem,
		NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
		MAC_TABLE_ENTRY *pEntry = NULL;
		BOOLEAN bAllowNrate = FALSE;
		BOOLEAN bssidEqualFlag = FALSE, isGoingToConnect = FALSE;
		UCHAR CentralChannel;
		RSSI_SAMPLE rssi_sample;
		struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		ULONG Idx = BSS_NOT_FOUND;
		struct common_ies *cmm_ies = &ie_list->cmm_ies;
		struct legacy_rate *rate = &cmm_ies->rate;
		/*ASSERT(pStaCfg);*/
		UINT16 cfg_ht_bw = 0;

		if (!pStaCfg)
			return FALSE;

		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		/*
			BEACON from desired BSS/IBSS found. We should be able to decide most
			BSS parameters here.
			Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
				Do we need to receover back all parameters belonging to previous BSS?
			A. Should be not. There's no back-door recover to previous AP. It still need
				a new JOIN-AUTH-ASSOC sequence.
		*/
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "receive desired BEACON,Channel=%d, (%d, %d)\n",
				 ie_list->Channel, ie_list->BeaconPeriod, ie_list->DtimPeriod);
		/* Update RSSI to prevent No signal display when cards first initialized */
		pStaCfg->RssiSample.LastRssi[0] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0);
		pStaCfg->RssiSample.LastRssi[1] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1);
		pStaCfg->RssiSample.LastRssi[2] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2);
		pStaCfg->RssiSample.AvgRssi[0] = pStaCfg->RssiSample.LastRssi[0];
		pStaCfg->RssiSample.AvgRssiX8[0] = pStaCfg->RssiSample.AvgRssi[0] << 3;
		pStaCfg->RssiSample.AvgRssi[1] = pStaCfg->RssiSample.LastRssi[1];
		pStaCfg->RssiSample.AvgRssiX8[1] = pStaCfg->RssiSample.AvgRssi[1] << 3;
		pStaCfg->RssiSample.AvgRssi[2] = pStaCfg->RssiSample.LastRssi[2];
		pStaCfg->RssiSample.AvgRssiX8[2] = pStaCfg->RssiSample.AvgRssi[2] << 3;

		/*
		  We need to check if SSID only set to any, then we can record the current SSID.
		  Otherwise will cause hidden SSID association failed.
		*/
		if (pStaCfg->MlmeAux.SsidLen == 0) {
			NdisMoveMemory(pStaCfg->MlmeAux.Ssid, ie_list->Ssid, ie_list->SsidLen);
			pStaCfg->MlmeAux.SsidLen = ie_list->SsidLen;
		} else {
			CHAR Rssi;

			Idx = BssSsidTableSearch(ScanTab, ie_list->Bssid,
									 pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen,
									 ie_list->Channel);

			if (Idx == BSS_NOT_FOUND) {
				rssi_sample.AvgRssi[0] = Elem->rssi_info.raw_rssi[0];
				rssi_sample.AvgRssi[1] = Elem->rssi_info.raw_rssi[1];
				rssi_sample.AvgRssi[2] = Elem->rssi_info.raw_rssi[2];
				rssi_sample.AvgRssi[3] = Elem->rssi_info.raw_rssi[3];
				Rssi = rtmp_avg_rssi(pAd, &rssi_sample);
				Idx = BssTableSetEntry(pAd, &pStaCfg->wdev, ScanTab, ie_list, Rssi, LenVIE, pVIE);

				if (Idx != BSS_NOT_FOUND) {
					NdisMoveMemory(ScanTab->BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
					NdisMoveMemory(&ScanTab->BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
					NdisMoveMemory(&ScanTab->BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
					ie_list->CapabilityInfo = ScanTab->BssEntry[Idx].CapabilityInfo;
					ScanTab->BssEntry[Idx].MinSNR = Elem->Signal % 10;

					if (ScanTab->BssEntry[Idx].MinSNR == 0)
						ScanTab->BssEntry[Idx].MinSNR = -5;

					NdisMoveMemory(ScanTab->BssEntry[Idx].MacAddr, ie_list->Addr2, MAC_ADDR_LEN);
				}
			} else {
#ifdef WPA_SUPPLICANT_SUPPORT

				if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
					;
				else
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT
					if (wdev->WscControl.WscState != WSC_STATE_OFF)
						;
					else
#endif /* WSC_STA_SUPPORT */
					{
						/*
							Check if AP privacy is different Staion, if yes,
							start a new scan and ignore the frame
							(often happen during AP change privacy at short time)
						*/
						if (((((IS_SECURITY_Entry(wdev)) << 4) ^ ie_list->CapabilityInfo) & 0x0010)

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT

							/* When using -Dwext and trigger WPS, do not check security. */
							&& (ie_list->selReg == 0)
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
							&& (!((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (!CAP_IS_PRIVACY_ON(ie_list->CapabilityInfo))))) {
								MLME_SCAN_REQ_STRUCT ScanReq;

								MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:AP privacy %d is differenct from STA privacy%d\n",
										 __func__, (ie_list->CapabilityInfo & 0x0010) >> 4, IS_SECURITY_Entry(wdev));
								ScanParmFill(pAd, &ScanReq, (RTMP_STRING *) pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
								cntl_scan_request(wdev, &ScanReq);
								return FALSE;
						}
					}

				/* Multiple SSID case, used correct CapabilityInfo */
				ie_list->CapabilityInfo = ScanTab->BssEntry[Idx].CapabilityInfo;
#ifdef DOT11V_MBSSID_SUPPORT
				pStaCfg->MlmeAux.max_bssid_indicator = ScanTab->BssEntry[Idx].max_bssid_indicator;
				pStaCfg->MlmeAux.mbssid_index = ScanTab->BssEntry[Idx].mbssid_index;
#endif
			}
#ifdef MWDS
			if (Idx != BSS_NOT_FOUND) {
				ScanTab->BssEntry[Idx].bSupportMWDS = FALSE;
				pStaCfg->MlmeAux.bSupportMWDS = FALSE;
				if (ie_list->vendor_ie.mtk_cap_found) {
					BOOLEAN bSupportMWDS = FALSE;

					if (ie_list->vendor_ie.support_mwds)
						bSupportMWDS = TRUE;
					if (ScanTab->BssEntry[Idx].bSupportMWDS != bSupportMWDS)
						ScanTab->BssEntry[Idx].bSupportMWDS = bSupportMWDS;
					if (ScanTab->BssEntry[Idx].bSupportMWDS)
						pStaCfg->MlmeAux.bSupportMWDS = TRUE;
					else
						pStaCfg->MlmeAux.bSupportMWDS = FALSE;
				}
			}
#endif /* MWDS */
		}

		pStaCfg->MlmeAux.CapabilityInfo = ie_list->CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
		pStaCfg->MlmeAux.BssType = ie_list->BssType;
		pStaCfg->MlmeAux.BeaconPeriod = ie_list->BeaconPeriod;
		pStaCfg->MlmeAux.DtimPeriod = ie_list->DtimPeriod;
		NdisMoveMemory(pStaCfg->MlmeAux.Bssid, ie_list->Bssid, MAC_ADDR_LEN);

		/*
			Some AP may carrys wrong beacon interval (ex. 0) in Beacon IE.
			We need to check here for preventing divided by 0 error.
		*/
		if (pStaCfg->MlmeAux.BeaconPeriod == 0)
			pStaCfg->MlmeAux.BeaconPeriod = 100;

		pStaCfg->MlmeAux.Channel = ie_list->Channel;
		pStaCfg->MlmeAux.AtimWin = ie_list->AtimWin;
		pStaCfg->MlmeAux.CfpPeriod = ie_list->CfParm.CfpPeriod;
		pStaCfg->MlmeAux.CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
		NdisMoveMemory(&pStaCfg->MlmeAux.vendor_ie,
					   &ie_list->cmm_ies.vendor_ie, sizeof(struct _vendor_ie_cap));
		/*
			Copy AP's supported rate to MlmeAux for creating assoication request
			Also filter out not supported rate
		*/
		check_legacy_rates(rate, &pStaCfg->MlmeAux.rate, wdev);
		NdisZeroMemory(pStaCfg->StaActive.SupportedPhyInfo.MCSSet, 16);
#ifdef DOT11R_FT_SUPPORT

		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			FT_GetMDIE(pVIE, LenVIE, &pStaCfg->MlmeAux.MdIeInfo)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PeerBeaconAtJoinAction! (MdId=%x%x, FtOverDs=%d, RsrReqCap=%d)\n",
					 pStaCfg->MlmeAux.MdIeInfo.MdId[0],
					 pStaCfg->MlmeAux.MdIeInfo.MdId[1],
					 pStaCfg->MlmeAux.MdIeInfo.FtCapPlc.field.FtOverDs,
					 pStaCfg->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap);
		}

#endif /* DOT11R_FT_SUPPORT */
		/*  Get the ext capability info element */
		NdisMoveMemory(&pStaCfg->MlmeAux.ExtCapInfo, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
#ifdef DOT11_VHT_AC
		pStaCfg->StaActive.SupportedPhyInfo.bVhtEnable = FALSE;
		pStaCfg->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MlmeAux.ExtCapInfo=%d\n", pStaCfg->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport);

		if (pAd->CommonCfg.bBssCoexEnable == TRUE)
			pAd->CommonCfg.ExtCapIE.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
		pStaCfg->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
		if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
			SET_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
		CentralChannel = ie_list->Channel;
		RTMPZeroMemory(&pStaCfg->MlmeAux.HtCapability, SIZE_HT_CAP_IE);

		if (!pEntry) {
			/* The first time MacEntry is go to be used. We have to create the MacEntry. */
			pEntry = MacTableInsertEntry(pAd, ie_list->Bssid, &pStaCfg->wdev, ENTRY_INFRA, OPMODE_STA, TRUE);
			ASSERT(pEntry);
		}

		/* Decide Pairwise and group cipher with AP */
		if (pEntry) {
			struct _SECURITY_CONFIG *pProfile_SecConfig = &wdev->SecConfig;
			struct _SECURITY_CONFIG *pEntry_SecConfig = &pEntry->SecConfig;
			USHORT RsnCapability = 0;
			USHORT IsSHA256 = FALSE;
			UCHAR Privacy = CAP_IS_PRIVACY_ON(ie_list->CapabilityInfo);
			pStaCfg->MlmeAux.VarIELen = 0;
			NdisZeroMemory(pStaCfg->MlmeAux.VarIEs, MAX_VIE_LEN);
#ifdef WSC_INCLUDED
			if ((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
				(wdev->WscControl.bWscTrigger == FALSE))
#endif /* WSC_INCLUDED */

#ifdef APCLI_CFG80211_SUPPORT
			if (!(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS))
#endif /* APCLI_CFG80211_SUPPORT */

			{
				if ((IS_SECURITY(pProfile_SecConfig) && (Privacy == 0))
					&& (!(IS_AKM_OWE(pProfile_SecConfig->AKMMap)))
				   ) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
							 "ERROR: The RSN IE of this received Probe-resp is dis-match : Peer no SEC\n");
					return FALSE; /* None matched*/
				} else if (IS_NO_SECURITY(pProfile_SecConfig) && (Privacy == 1)) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
							 "ERROR: The RSN IE of this received Probe-resp is dis-match : wdev Config no SEC\n");
					return FALSE; /* None matched*/
				}
			}

			PaserSecurityIE(ie_list,
							&LenVIE,
							(PNDIS_802_11_VARIABLE_IEs) pVIE,
							&pEntry_SecConfig->AKMMap,
							&pEntry_SecConfig->PairwiseCipher,
							&pEntry_SecConfig->GroupCipher,
#ifdef DOT11W_PMF_SUPPORT
							&pEntry_SecConfig->PmfCfg.igtk_cipher,
#endif
							&RsnCapability,
							&IsSHA256);

			if ((pEntry_SecConfig->AKMMap == 0x0) && (Privacy == 1)) {
				/* WEP mode adjust */
				if (IS_AKM_AUTOSWITCH(pProfile_SecConfig->AKMMap))
					SET_AKM_AUTOSWITCH(pEntry_SecConfig->AKMMap);
				else if (IS_AKM_OPEN(pProfile_SecConfig->AKMMap))
					SET_AKM_OPEN(pEntry_SecConfig->AKMMap);
				else if (IS_AKM_SHARED(pProfile_SecConfig->AKMMap))
					SET_AKM_SHARED(pEntry_SecConfig->AKMMap);
				else
					SET_AKM_OPEN(pEntry_SecConfig->AKMMap);

				SET_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher);
				SET_CIPHER_WEP(pEntry_SecConfig->GroupCipher);
			}

			/*STACFG cipher set for WEP or Open*/
			CLEAR_SEC_AKM(pStaCfg->AKMMap);
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);
			CLEAR_CIPHER(pStaCfg->GroupCipher);
			pStaCfg->PairwiseCipher = pProfile_SecConfig->PairwiseCipher;
			pStaCfg->GroupCipher = pProfile_SecConfig->GroupCipher;
			pStaCfg->AKMMap = pProfile_SecConfig->AKMMap;

			/* Check the Authmode first*/
			pEntry_SecConfig->Handshake.WpaState = SS_NOTUSE;
			pEntry_SecConfig->Handshake.GTKState = REKEY_NEGOTIATING;
			os_zero_mem(pEntry_SecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);

			if ((!((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pEntry_SecConfig->AKMMap)))) &&
				(!((IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) && (IS_AKM_WPA2PSK_ONLY(pEntry_SecConfig->AKMMap))))) {
				pEntry_SecConfig->AKMMap &=  pProfile_SecConfig->AKMMap;
				pEntry_SecConfig->PairwiseCipher &=  pProfile_SecConfig->PairwiseCipher;
			}

			/*WPS: WPS Connection need to set HT Capablities and channel, so dont skip, if WPS is triggered*/
			if (((pEntry_SecConfig->AKMMap == 0) || (pEntry_SecConfig->PairwiseCipher == 0))
#ifdef WSC_STA_SUPPORT
				&& ((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
					(wdev->WscControl.bWscTrigger == FALSE))
#endif /* WSC_STA_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT
				&& (!(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS))
#endif /* WPA_SUPPLICANT_SUPPORT */
			   ) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
						 "%s, %u pEntry_Sec AKM %x Ciper %x, return due to desired BSS sec not mach\n",
						 __func__, __LINE__,
						 pEntry_SecConfig->AKMMap,
						 pEntry_SecConfig->PairwiseCipher);
				return FALSE; /* None matched*/
			}

			if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pEntry_SecConfig->AKMMap))) {
				 MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							"OWE STA connecting to OPEN AP)\n");
			} else
			if ((IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) && (IS_AKM_WPA2PSK_ONLY(pEntry_SecConfig->AKMMap))) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
							"WPA3PSK STA connecting to WPA2PSK AP\n");
			} else
			if (IS_AKM_WPA1(pEntry_SecConfig->AKMMap) &&
				IS_AKM_WPA2(pEntry_SecConfig->AKMMap)) {
				CLEAR_SEC_AKM(pEntry_SecConfig->AKMMap);
				SET_AKM_WPA2(pEntry_SecConfig->AKMMap);
			} else if (IS_AKM_WPA1PSK(pEntry_SecConfig->AKMMap) &&
					   IS_AKM_WPA2PSK(pEntry_SecConfig->AKMMap)) {
				CLEAR_SEC_AKM(pEntry_SecConfig->AKMMap);
				SET_AKM_WPA2PSK(pEntry_SecConfig->AKMMap);
#ifdef DPP_SUPPORT
			} else if ((IS_AKM_WPA2PSK(pEntry_SecConfig->AKMMap) || IS_AKM_WPA3PSK(pEntry_SecConfig->AKMMap))
				&& IS_AKM_DPP(pEntry_SecConfig->AKMMap)) {
				/* Setting DPP AKM in mixed mode if DPP is included with WPA2-PSK or WPA3-PSK */
				CLEAR_SEC_AKM(pEntry_SecConfig->AKMMap);
				SET_AKM_DPP(pEntry_SecConfig->AKMMap);
#endif /* DPP_SUPPORT */
			} else if (IS_AKM_WPA2PSK(pEntry_SecConfig->AKMMap) && IS_AKM_WPA3PSK(pEntry_SecConfig->AKMMap)) {
				CLEAR_SEC_AKM(pEntry_SecConfig->AKMMap);
				SET_AKM_WPA3PSK(pEntry_SecConfig->AKMMap);
			} else if (IS_AKM_OWE(pEntry_SecConfig->AKMMap)) {
				CLEAR_SEC_AKM(pEntry_SecConfig->AKMMap);
				SET_AKM_OWE(pEntry_SecConfig->AKMMap);

				CLEAR_CIPHER(pEntry_SecConfig->PairwiseCipher);
				SET_CIPHER_CCMP128(pEntry_SecConfig->PairwiseCipher);
				CLEAR_CIPHER(pEntry_SecConfig->GroupCipher);
				SET_CIPHER_CCMP128(pEntry_SecConfig->GroupCipher);
			}

			if ((IS_CIPHER_TKIP(pEntry_SecConfig->PairwiseCipher)) &&
				(IS_CIPHER_CCMP128(pEntry_SecConfig->PairwiseCipher))) {
				CLEAR_CIPHER(pEntry_SecConfig->PairwiseCipher);
				SET_CIPHER_CCMP128(pEntry_SecConfig->PairwiseCipher);
			}

			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Candidate Security AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s\n",
					 GetAuthModeStr(pEntry_SecConfig->AKMMap),
					 GetEncryModeStr(pEntry_SecConfig->PairwiseCipher),
					 GetEncryModeStr(pEntry_SecConfig->GroupCipher));
#ifdef DOT11W_PMF_SUPPORT
			/* OWE STA & OPEN AP */
			if (!((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pEntry_SecConfig->AKMMap)))
#ifdef DPP_SUPPORT
			|| (IS_AKM_DPP(pEntry_SecConfig->AKMMap))
#endif /* DPP_SUPPORT */
			) {
#ifdef WSC_INCLUDED

				if ((wdev->WscControl.WscConfMode == WSC_DISABLE)
					|| (wdev->WscControl.bWscTrigger == FALSE))
#endif /* WSC_INCLUDED */
#ifdef APCLI_CFG80211_SUPPORT
				if (!(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS))
#endif /* WPA_SUPPLICANT_SUPPORT */

				{
					RSN_CAPABILITIES RsnCap;

					NdisMoveMemory(&RsnCap, &RsnCapability, sizeof(RSN_CAPABILITIES));
					RsnCap.word = cpu2le16(RsnCap.word);

					if (Idx != BSS_NOT_FOUND) {
						BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
						BSS_ENTRY *pInBss = NULL;

						pStaCfg->MlmeAux.RsnCap.word = 0;
						pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation = FALSE;

						pInBss = &ScanTab->BssEntry[Idx];
						if (pInBss) {
							NdisMoveMemory(&pStaCfg->MlmeAux.RsnCap, &pInBss->RsnCapability, sizeof(RSN_CAPABILITIES));
							pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation = pInBss->IsSupportSHA256KeyDerivation;
#ifdef DOT11_SAE_SUPPORT
							pStaCfg->MlmeAux.sae_conn_type = pInBss->sae_conn_type;
#endif
							pStaCfg->MlmeAux.rsnxe_len = pInBss->rsnxe_len;
							NdisMoveMemory(pStaCfg->MlmeAux.rsnxe_content, pInBss->rsnxe_content, pInBss->rsnxe_len);
						}
					}
#ifdef CONFIG_MAP_SUPPORT
				if ((IS_MAP_ENABLE(pAd) && (IS_AKM_WPA3PSK(pEntry_SecConfig->AKMMap) ||
						IS_AKM_WPA3_192BIT(pEntry_SecConfig->AKMMap) ||
						IS_AKM_WPA3(pEntry_SecConfig->AKMMap) ||
						IS_AKM_OWE(pEntry_SecConfig->AKMMap) ||
						IS_AKM_OPEN(pEntry_SecConfig->AKMMap))) || !IS_MAP_ENABLE(pAd)) {
#endif
						if (IS_AKM_WPA3PSK(pProfile_SecConfig->AKMMap) ||
						IS_AKM_WPA3_192BIT(pProfile_SecConfig->AKMMap) ||
						IS_AKM_WPA3(pProfile_SecConfig->AKMMap) ||
						IS_AKM_OWE(pProfile_SecConfig->AKMMap)) {
						/* if use WPA3/WPA3PSK/OWE, force to PMF connect */
						pProfile_SecConfig->PmfCfg.MFPC = TRUE;
							if (IS_AKM_WPA2PSK_ONLY(pEntry_SecConfig->AKMMap)) {
							/*If AP is WPA2PSK only, set MFP Required to 0 to support both MFP capable/noncapable AP*/
								pProfile_SecConfig->PmfCfg.MFPR = FALSE;
							}
							else
								pProfile_SecConfig->PmfCfg.MFPR = TRUE;
						}

						if (IS_AKM_OPEN(pProfile_SecConfig->AKMMap)) {
						/* if use OPEN security, force disable PMF flags */
							pProfile_SecConfig->PmfCfg.MFPC = FALSE;
							pProfile_SecConfig->PmfCfg.MFPR = FALSE;
						}
#ifdef CONFIG_MAP_SUPPORT
					}
#endif

					if (((pProfile_SecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
						|| ((pProfile_SecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										"PMF fail: peer MFPR = %d, MFPC = %d\n",
										 RsnCap.field.MFPR, RsnCap.field.MFPC);
						return FALSE; /* None matched*/
					}

					if ((pProfile_SecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
						pEntry_SecConfig->PmfCfg.UsePMFConnect = TRUE;

						if (IS_AKM_SHA384(pEntry_SecConfig->AKMMap))
							pEntry_SecConfig->key_deri_alg = SEC_KEY_DERI_SHA384;
						else if ((IsSHA256)
								|| (RsnCap.field.MFPR)
								|| (IS_AKM_SHA256(pEntry_SecConfig->AKMMap)))
							pEntry_SecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;

						pEntry_SecConfig->PmfCfg.MFPC = RsnCap.field.MFPC;
						/* PMF Test Case 5.2: Requirement */
						pEntry_SecConfig->PmfCfg.MFPR = pProfile_SecConfig->PmfCfg.MFPR;
						if (IS_AKM_WPA3PSK(pEntry_SecConfig->AKMMap)
							&& (RsnCap.field.MFPC)) {
								pEntry_SecConfig->PmfCfg.MFPR = TRUE;
						}
					}
				}
			}
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SUPPORT
			if (pStaCfg->ApCliTransDisableSupported) {
				struct transition_disable_bitmap *bitmap = &(pStaCfg->ApCli_tti_bitmap);
				/* Valid Combinations */
				if ((bitmap->wpa3_psk && (IS_AKM_WPA3PSK(pEntry_SecConfig->AKMMap)))
#ifdef DOT11_SAE_SUPPORT
				|| (bitmap->sae_pk && pStaCfg->wdev.SecConfig.sae_cap.sae_pk_en &&
					(pStaCfg->MlmeAux.sae_conn_type == SAE_CONNECTION_TYPE_SAEPK) && (IS_AKM_SAE(pEntry_SecConfig->AKMMap)))
#endif
#ifdef DOT11_SUITEB_SUPPORT
				|| (bitmap->wpa3_ent && (IS_AKM_SUITEB_SHA256(pEntry_SecConfig->AKMMap)))
#endif
#ifdef CONFIG_OWE_SUPPORT
				|| (bitmap->enhanced_open && (IS_AKM_OWE(pEntry_SecConfig->AKMMap)))
#endif
				) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"Valid Combo tti_bitmap: wpa3_psk:%d sae_pk:%d wpa3_ent:%d OWE:%d APs AKM:0x%x\n",
					bitmap->wpa3_psk, bitmap->sae_pk,
					bitmap->wpa3_ent, bitmap->enhanced_open,
					pEntry_SecConfig->AKMMap);
				} else if (bitmap->wpa3_psk || bitmap->sae_pk ||
					bitmap->wpa3_ent || bitmap->enhanced_open) {
					/* Other Combinations which are not valid */
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"Transition Disabled:Invalid combo tti_bitmap: wpa3_psk:%d sae_pk:%d wpa3_ent:%d OWE:%d APs AKM:0x%x\n",
					bitmap->wpa3_psk, bitmap->sae_pk,
					bitmap->wpa3_ent, bitmap->enhanced_open,
					pEntry_SecConfig->AKMMap);

					return FALSE;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"Transition Disable active but no valid bit is set tti_bitmap: wpa3_psk:%d sae_pk:%d wpa3_ent:%d OWE:%d APs AKM:0x%x\n",
					bitmap->wpa3_psk, bitmap->sae_pk,
					bitmap->wpa3_ent, bitmap->enhanced_open,
					pEntry_SecConfig->AKMMap);
				}
			}
#endif /*#ifdef APCLI_SUPPORT*/

			if ((IS_NO_SECURITY(pEntry_SecConfig)
				 || ((!IS_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher)) && (!IS_CIPHER_TKIP(pEntry_SecConfig->PairwiseCipher))))
				|| (pAd->CommonCfg.HT_DisallowTKIP == FALSE)) {
				if ((pStaCfg->BssType == BSS_INFRA) ||
					((pStaCfg->BssType == BSS_ADHOC) && (adhocInfo->bAdhocN == TRUE)))
					bAllowNrate = TRUE;
			}

			if (IS_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher)) {
				os_move_mem(pEntry_SecConfig->WepKey, pProfile_SecConfig->WepKey,  sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
				pProfile_SecConfig->GroupKeyId = pProfile_SecConfig->PairwiseKeyId;
				pEntry_SecConfig->PairwiseKeyId = pProfile_SecConfig->PairwiseKeyId;
			} else {
				/* Calculate PMK */
				/* DO not call this */
				SetWPAPSKKey(pAd, pProfile_SecConfig->PSK, strlen(pProfile_SecConfig->PSK), pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen, pEntry_SecConfig->PMK);
				os_move_mem(pEntry_SecConfig->Handshake.AAddr, pEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(pEntry_SecConfig->Handshake.SAddr, wdev->if_addr, MAC_ADDR_LEN);
			}

			pEntry_SecConfig->GroupKeyId = pProfile_SecConfig->GroupKeyId;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): Final Security AKM = 0x%x, PairwiseCipher = 0x%x, GroupCipher = 0x%x, bAllowNrate=%d\n",
					 __func__, pEntry_SecConfig->AKMMap, pEntry_SecConfig->PairwiseCipher, pEntry_SecConfig->GroupCipher, bAllowNrate);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): pEntry_SecConfig->PairwiseKeyId=%d, pEntry_SecConfig->GroupKeyId=%d\n",
					 __func__, pEntry_SecConfig->PairwiseKeyId, pEntry_SecConfig->GroupKeyId);
#ifdef DOT11W_PMF_SUPPORT

			if (pEntry_SecConfig->PmfCfg.UsePMFConnect)
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] Use PMF Connect\n");

			if (pEntry_SecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_INFO, "[PMF] Use SHA256\n");

#endif /* DOT11W_PMF_SUPPORT */
			pStaCfg->MlmeAux.VarIELen = LenVIE;
			NdisMoveMemory(pStaCfg->MlmeAux.VarIEs, pVIE, pStaCfg->MlmeAux.VarIELen);
		}


		NdisZeroMemory(pStaCfg->StaActive.RxMcsSet, sizeof(pStaCfg->StaActive.RxMcsSet));

		/* filter out un-supported ht rates */
		if (((HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) || (HAS_PREN_CAPS_EXIST(ie_list->cmm_ies.ie_exists))) &&
			(wdev->DesiredHtPhyInfo.bHtEnable) &&
			(WMODE_CAP_N(wdev->PhyMode) && bAllowNrate)) {
			RTMPMoveMemory(&pStaCfg->MlmeAux.AddHtInfo, &ie_list->cmm_ies.ht_op, SIZE_ADD_HT_INFO_IE);
			/* StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability */
			NdisMoveMemory(pStaCfg->StaActive.SupportedPhyInfo.MCSSet, ie_list->cmm_ies.ht_cap.MCSSet, 16);
			pStaCfg->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
			SET_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
			pStaCfg->StaActive.SupportedPhyInfo.bHtEnable = TRUE;

			cfg_ht_bw = ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth;
			if (cfg_ht_bw)
				wlan_operate_set_ht_bw(wdev, BW_40, wlan_operate_get_ext_cha(wdev));
			else
				wlan_operate_set_ht_bw(wdev, BW_20, EXTCHA_NONE);

			if (HAS_PREN_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
				pStaCfg->StaActive.SupportedPhyInfo.bPreNHt = TRUE;

			/* Copy AP Parameter to StaActive.  This is also in LinkUp. */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s():(MpduDensity=%d, MaxRAmpduFactor=%d, BW=%d)\n",
					 __func__, pStaCfg->StaActive.SupportedHtPhy.MpduDensity,
					 pStaCfg->StaActive.SupportedHtPhy.MaxRAmpduFactor,
					 ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth);

			if (HAS_HT_OP_EXIST(ie_list->cmm_ies.ie_exists)) {
				/* Check again the Bandwidth capability of this AP. */
				CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->cmm_ies.ht_op,
						&ie_list->cmm_ies.ht_cap);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"HT-CtrlChannel=%d, CentralChannel=>%d\n",
						 ie_list->cmm_ies.ht_op.ControlChan, CentralChannel);
			}

#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode) &&
				(pStaCfg->MlmeAux.Channel > 14) &&
				HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
				struct vht_opinfo *vht_op = &ie_list->cmm_ies.vht_op.vht_op_info;
				UCHAR cap_vht_bw = wlan_config_get_vht_bw(wdev);
				struct _op_info op_info = {0};

				NdisMoveMemory(&pStaCfg->MlmeAux.vht_cap, &ie_list->cmm_ies.vht_cap,
						SIZE_OF_VHT_CAP_IE);
				SET_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
				pStaCfg->StaActive.SupportedPhyInfo.bVhtEnable = TRUE;
				update_vht_op_info(cap_vht_bw, vht_op, &op_info);
				wlan_operate_set_vht_bw(wdev, op_info.bw);

				if (op_info.bw != VHT_BW_2040)
					CentralChannel = op_info.cent_ch;

				pStaCfg->StaActive.SupportedPhyInfo.vht_bw = op_info.bw;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CentralChannel=>%d\n",
						 CentralChannel);
			}

#endif /* DOT11_VHT_AC */
		} else
#endif /* DOT11_N_SUPPORT */
		{

			/* To prevent error, let legacy AP must have same CentralChannel and Channel. */
			if (!HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) && !HAS_PREN_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
				pStaCfg->MlmeAux.CentralChannel = pStaCfg->MlmeAux.Channel;

			pStaCfg->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
#ifdef DOT11_VHT_AC
			pStaCfg->StaActive.SupportedPhyInfo.bVhtEnable = FALSE;
			if (WMODE_CAP_6G(wdev->PhyMode) && HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
				UINT8 ch_width = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(cmm_ies->he_caps.phy_cap.phy_capinfo_1));
				UINT8 he6g_op_present = 0;
				UCHAR cap_vht_bw = wlan_config_get_vht_bw(wdev);

				he6g_op_present = wlan_config_get_he6g_op_present(wdev);
				if (he6g_op_present) {
					UINT8 op_ch_width = HE_6G_OP_CTRL_GET_CH_WIDTH(cmm_ies->he6g_opinfo.ctrl);
					if (op_ch_width >= HE_BW_80) {
						/* mapping HE_BW to VHT_BW for 6G APCLI BW*/
						pStaCfg->StaActive.SupportedPhyInfo.vht_bw = op_ch_width - 1;

						/*check bw should below or equal the cap*/
						if (pStaCfg->StaActive.SupportedPhyInfo.vht_bw > cap_vht_bw)
							pStaCfg->StaActive.SupportedPhyInfo.vht_bw = cap_vht_bw;
					} else {
						pStaCfg->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
					}
				} else {
					if (ch_width >= HE_BW_80) {
						/* mapping HE_BW to VHT_BW for 6G APCLI BW*/
						pStaCfg->StaActive.SupportedPhyInfo.vht_bw = ch_width - 1;
						/*check bw should below or equal the cap*/
						if (pStaCfg->StaActive.SupportedPhyInfo.vht_bw > cap_vht_bw)
							pStaCfg->StaActive.SupportedPhyInfo.vht_bw = cap_vht_bw;
					} else {
						pStaCfg->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
					}
				}
			}
#endif /* DOT11_VHT_AC */
			pStaCfg->MlmeAux.NewExtChannelOffset = 0xff;
			CLR_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
			CLR_HT_OP_EXIST(pStaCfg->MlmeAux.ie_exists);
			RTMPZeroMemory(&pStaCfg->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
			RTMPZeroMemory(&pStaCfg->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
		}

		pStaCfg->MlmeAux.CentralChannel = CentralChannel;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set CentralChannel=%d\n", pStaCfg->MlmeAux.CentralChannel);

		if (pEntry)
			RTMPCheckHt(pAd, pEntry->wcid, &ie_list->cmm_ies.ht_cap, &ie_list->cmm_ies.ht_op);

		RTMPUpdateMlmeRate(pAd, wdev);

		/* copy QOS related information */
		if ((wdev->bWmmCapable)
#ifdef DOT11_N_SUPPORT
			|| WMODE_CAP_N(wdev->PhyMode)
#endif /* DOT11_N_SUPPORT */
		   ) {
			NdisMoveMemory(&pStaCfg->MlmeAux.APEdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
			NdisMoveMemory(&pStaCfg->MlmeAux.APQbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
			NdisMoveMemory(&pStaCfg->MlmeAux.APQosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
		} else {
			NdisZeroMemory(&pStaCfg->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
			NdisZeroMemory(&pStaCfg->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
			NdisZeroMemory(&pStaCfg->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
		}

		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): - after JOIN, SupRateLen=%d, ExtRateLen=%d\n",
				 __func__, pStaCfg->MlmeAux.rate.sup_rate_len,
				 pStaCfg->MlmeAux.rate.ext_rate_len);

		if (ie_list->AironetCellPowerLimit != 0xFF) {
			/* We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
			ChangeToCellPowerLimit(pAd, ie_list->AironetCellPowerLimit);
		} else {
			/* Used the default TX Power Percentage. */
			/* Used the default TX Power Percentage. */
			pAd->CommonCfg.ucTxPowerPercentage[BAND0] = pAd->CommonCfg.ucTxPowerDefault[BAND0];
#ifdef DBDC_MODE
			pAd->CommonCfg.ucTxPowerPercentage[BAND1] = pAd->CommonCfg.ucTxPowerDefault[BAND1];
#endif /* DBDC_MODE */

		}

		if (pStaCfg->BssType == BSS_INFRA) {
			BOOLEAN InfraAP_BW;
			UCHAR BwFallBack = 0;
#ifdef WSC_AP_SUPPORT
#ifdef DOT11_N_SUPPORT

			if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				(wdev->WscControl.bWscTrigger == TRUE)) {
				ADD_HTINFO RootApHtInfo;
				UCHAR ext_cha = wlan_config_get_ext_cha(wdev);
				UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

				RootApHtInfo = ie_list->cmm_ies.ht_op.AddHtInfo;

				if ((cfg_ht_bw == HT_BW_40) &&
					(RootApHtInfo.RecomWidth) &&
					(RootApHtInfo.ExtChanOffset != ext_cha)) {
					if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
						set_extcha_for_wdev(pAd, wdev, 1);
					else
						set_extcha_for_wdev(pAd, wdev, 0);

					return FALSE;
				}
			}

#endif /* DOT11_N_SUPPORT */
#endif /* WSC_AP_SUPPORT */

			if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
				InfraAP_BW = TRUE;
			else
				InfraAP_BW = FALSE;

			AdjustChannelRelatedValue(pAd, &BwFallBack, BSS0, InfraAP_BW, pStaCfg->MlmeAux.Channel,
									  pStaCfg->MlmeAux.CentralChannel, wdev);
		}

		/* By Cfg Bssid Join Way */
		if (!MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ZERO_MAC_ADDR))
			bssidEqualFlag = MAC_ADDR_EQUAL(pStaCfg->CfgApCliBssid, ie_list->Bssid);
		else
			bssidEqualFlag = MAC_ADDR_EQUAL(pStaCfg->MlmeAux.Bssid, ie_list->Bssid);

		if (bssidEqualFlag == TRUE) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
			if (1)
#else
			/* follow root ap setting while ApCliAutoConnectRunning is active */
			if ((pStaCfg->ApCliAutoConnectRunning == TRUE)
#ifdef BT_APCLI_SUPPORT
			|| (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE)
#endif
			)
#endif
			{
				ULONG Bssidx = 0;
				BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

				Bssidx = BssTableSearch(ScanTab, pStaCfg->MlmeAux.Bssid, pStaCfg->wdev.channel);

				if (Bssidx != BSS_NOT_FOUND) {
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
					UCHAR ret = ApCliAutoConnectBWAdjust(pAd, &pStaCfg->wdev, &ScanTab->BssEntry[Bssidx]);

					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG, "Bssidx:%lu\n", Bssidx);

					if (ScanTab->BssEntry[Bssidx].SsidLen)
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_DEBUG, "Root AP SSID: %s\n", ScanTab->BssEntry[Bssidx].Ssid);

					if (ret != AUTO_BW_PARAM_ERROR)
						isGoingToConnect = TRUE;

					if (ret == AUTO_BW_NEED_TO_ADJUST)
#endif /* APCLI_AUTO_BW_TMP */
					{
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Switch to channel :%d\n", ScanTab->BssEntry[Bssidx].Channel);
						rtmp_set_channel(pAd, &pStaCfg->wdev, ScanTab->BssEntry[Bssidx].Channel);
						isGoingToConnect = TRUE;

						/* MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);	*/
					}
				} else
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "Can not find BssEntry\n");
			} else
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			{
#ifndef APCLI_CFG80211_SUPPORT
				isGoingToConnect = TRUE;
#endif
			}
#ifdef MWDS
			{
				ULONG Bssidx = 0;
				BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

				Bssidx = BssTableSearch(ScanTab, pStaCfg->MlmeAux.Bssid, pStaCfg->wdev.channel);

				if (Bssidx != BSS_NOT_FOUND) {
					ScanTab->BssEntry[Bssidx].bSupportMWDS = FALSE;
					pStaCfg->MlmeAux.bSupportMWDS = FALSE;

					if (ie_list->vendor_ie.mtk_cap_found) {
						BOOLEAN bSupportMWDS = FALSE;

						if (ie_list->vendor_ie.support_mwds)
							bSupportMWDS = TRUE;

						if (ScanTab->BssEntry[Bssidx].bSupportMWDS != bSupportMWDS)
							ScanTab->BssEntry[Bssidx].bSupportMWDS = bSupportMWDS;

						if (ScanTab->BssEntry[Bssidx].bSupportMWDS) {
							pStaCfg->MlmeAux.bSupportMWDS = TRUE;
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, "%s AP(%ld) supports MWDS\n", __func__, Bssidx);
						} else {
							pStaCfg->MlmeAux.bSupportMWDS = FALSE;
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, "%s AP(%ld) don't supports MWDS\n", __func__, Bssidx);
						}
					}
				}
			}
#endif /* MWDS */
		}
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
			" %s, %u  isGoingToConnect %d\n", __func__, __LINE__, isGoingToConnect);

		return isGoingToConnect;

}

struct sync_fsm_ops sta_fsm_ops = {
	.tx_probe_response_allowed = sta_probe_response_allowed,
	.tx_probe_response_xmit = sta_probe_response_xmit,

	.rx_peer_response_allowed = sta_rx_peer_response_allowed,
	.rx_peer_response_updated = sta_rx_peer_response_updated,

	.join_peer_response_matched = sta_join_peer_response_matched,
	.join_peer_response_updated = sta_join_peer_response_updated,
};
#endif /* CONFIG_STA_SUPPORT */

