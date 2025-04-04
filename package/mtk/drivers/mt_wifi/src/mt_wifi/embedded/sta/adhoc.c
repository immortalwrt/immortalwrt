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
#ifdef CONFIG_STA_ADHOC_SUPPORT
#include "rt_config.h"

#define ADHOC_ENTRY_BEACON_LOST_TIME    (2*OS_HZ)       /* 2 sec */ /* we re-add the ad-hoc peer into our mac table */
#define ADHOC_BEACON_LOST_TIME          (8*OS_HZ)       /* 8 sec */ /* we deauth the ad-hoc peer */

BOOLEAN adhoc_add_peer_from_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *bcn_ie_list,
								   NDIS_802_11_VARIABLE_IEs *pVIE, USHORT LenVIE)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	struct legacy_rate *rate = &bcn_ie_list->cmm_ies.rate;

	if (ADHOC_ON(pAd) && (CAP_IS_IBSS_ON(bcn_ie_list->CapabilityInfo))) {
		UCHAR MaxSupportedRateIn500Kbps = 0;
		UCHAR idx;
		MAC_TABLE_ENTRY *pEntry;
#ifdef IWSC_SUPPORT
		PWSC_CTRL pWpsCtrl = &pStaCfg->WscControl;
#endif /* IWSC_SUPPORT */
		ULONG Now;

		NdisGetSystemUpTime(&Now);
		MaxSupportedRateIn500Kbps = dot11_max_sup_rate(rate);
		/* look up the existing table */
		pEntry = MacTableLookup2(pAd, bcn_ie_list->Addr2, wdev);

		/*
		   Ad-hoc mode is using MAC address as BA session. So we need to continuously find newly joined adhoc station by receiving beacon.
		   To prevent always check this, we use wcid == WCID_NO_MATCHED to recognize it as newly joined adhoc station.
		*/
		if ((ADHOC_ON(pAd) && ((!pEntry) || (pEntry && IS_ENTRY_NONE(pEntry)))) ||
			(pEntry && RTMP_TIME_AFTER(Now, pEntry->LastBeaconRxTime + ADHOC_ENTRY_BEACON_LOST_TIME))) {
			/* Another adhoc joining, add to our MAC table. */
			if (pEntry == NULL) {
				pEntry = MacTableInsertEntry(pAd, bcn_ie_list->Addr2, wdev, ENTRY_ADHOC, OPMODE_STA, FALSE);
#ifdef RT_CFG80211_SUPPORT
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Another adhoc joining, add to our MAC table ==> "MACSTR"\n",
						 MAC2STR(bcn_ie_list->Addr2));
				RT_CFG80211_JOIN_IBSS(pAd, pStaCfg->MlmeAux.Bssid);
				CFG80211OS_NewSta(pAd->net_dev, bcn_ie_list->Addr2, NULL, 0, FALSE);
#endif /* RT_CFG80211_SUPPORT */
			}

			if (pEntry == NULL)
				return FALSE;

			SET_ENTRY_CLIENT(pEntry);
#ifdef IWSC_SUPPORT
			hex_dump("Another adhoc joining - Addr2", bcn_ie_list->Addr2, 6);
			hex_dump("Another adhoc joining - WscPeerMAC", pStaCfg->WscControl.WscPeerMAC, 6);

			if ((NdisEqualMemory(bcn_ie_list->Addr2, pStaCfg->WscControl.WscPeerMAC, MAC_ADDR_LEN)) &&
				(pStaCfg->IWscInfo.bSendEapolStart == FALSE) &&
				(pWpsCtrl->bWscTrigger == TRUE))
				pStaCfg->IWscInfo.bSendEapolStart = TRUE;

#endif /* IWSC_SUPPORT */
			{
				BOOLEAN result;
				IE_LISTS *ielist = NULL;
#ifdef DOT11_VHT_AC
				os_alloc_mem(NULL, (UCHAR **)&ielist, sizeof(IE_LISTS));

				if (!ielist)
					return FALSE;

				NdisZeroMemory((UCHAR *)ielist, sizeof(IE_LISTS));

				if (HAS_VHT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists) &&
					HAS_VHT_OP_EXIST(bcn_ie_list->cmm_ies.ie_exists)) {
					NdisMoveMemory(&ielist->vht_cap, &bcn_ie_list->vht_cap_ie, sizeof(VHT_CAP_IE));
					NdisMoveMemory(&ielist->vht_op, &bcn_ie_list->vht_op_ie, sizeof(VHT_OP_IE));
					SET_VHT_CAPS_EXIST(ielist->cmm_ies.ie_exists);
					SET_VHT_OP_EXIST(ielist->cmm_ies.ie_exists);
				}

#endif /* DOT11_VHT_AC */
				result = StaUpdateMacTableEntry(pAd, pEntry, MaxSupportedRateIn500Kbps,
												&bcn_ie_list->HtCapability,
												&bcn_ie_list->AddHtInfo,
												ielist, bcn_ie_list->CapabilityInfo);

				if (ielist != NULL)
					os_free_mem(ielist);

				if (result == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ADHOC - Add Entry failed.\n");
					return FALSE;
				}

#ifdef IWSC_SUPPORT
				else
					pEntry->bUpdateInfoFromPeerBeacon = TRUE;

#endif /* IWSC_SUPPORT */
			}
			RTMPSetSupportMCS(pAd, OPMODE_STA, pEntry, rate,
#ifdef DOT11_VHT_AC
							  HAS_VHT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists), &bcn_ie_list->vht_cap_ie,
#endif /* DOT11_VHT_AC */
							  &bcn_ie_list->HtCapability, HAS_HT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists));
			pEntry->LastBeaconRxTime = 0;
#ifdef ADHOC_WPA2PSK_SUPPORT

			/* Adhoc support WPA2PSK by Eddy */
			if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
				&& (pEntry->WPA_Authenticator.WpaState < AS_INITPSK)
#ifdef IWSC_SUPPORT
				&& ((pStaCfg->WscControl.WscConfMode == WSC_DISABLE) ||
					(pStaCfg->WscControl.bWscTrigger == FALSE) ||
					(NdisEqualMemory(pEntry->Addr, pStaCfg->WscControl.WscPeerMAC, MAC_ADDR_LEN) == FALSE))
#ifdef IWSC_TEST_SUPPORT
				&& (pStaCfg->IWscInfo.bBlockConnection == FALSE)
#endif /* IWSC_TEST_SUPPORT */
#endif /* IWSC_SUPPORT */
			   ) {
				INT len, i;
				PEID_STRUCT	pEid;
				NDIS_802_11_VARIABLE_IEs *pVIE2 = NULL;
				BOOLEAN bHigherMAC = FALSE;

				pVIE2 = pVIE;
				len  = LenVIE;

				while (len > 0) {
					pEid = (PEID_STRUCT) pVIE;

					if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))) {
						NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pEntry->RSNIE_Len = (pEid->Len + 2);
					}

					pVIE2 += (pEid->Len + 2);
					len  -= (pEid->Len + 2);
				}

				/* pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED; */
				/* TODO: shiang-usw, need to replace upper setting with tr_entry */
				pAd->MacTab.tr_entry[pEntry->wcid].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				NdisZeroMemory(&pEntry->WPA_Supplicant.ReplayCounter, LEN_KEY_DESC_REPLAY);
				NdisZeroMemory(&pEntry->WPA_Authenticator.ReplayCounter, LEN_KEY_DESC_REPLAY);
				pEntry->WPA_Authenticator.WpaState = AS_INITPSK;
				pEntry->WPA_Supplicant.WpaState = AS_INITPSK;
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;

				for (i = 0; i < MAC_ADDR_LEN; i++) {
					if (bcn_ie_list->Addr2[i] > wdev->if_addr[i]) {
						bHigherMAC = TRUE;
						break;
					} else if (bcn_ie_list->Addr2[i] < wdev->if_addr[i])
						break;
				}

				hex_dump("PeerBeacon:: Addr2", bcn_ie_list->Addr2, MAC_ADDR_LEN);
				hex_dump("PeerBeacon:: CurrentAddress", wdev->if_addr, MAC_ADDR_LEN);
				pEntry->bPeerHigherMAC = bHigherMAC;

				if (pEntry->bPeerHigherMAC == FALSE) {
					/*
						My MAC address is higher than peer's MAC address.
					*/
					/* MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ADHOC - EnqueueStartForPSKTimer.\n")); */
					/* RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER); */
				}
			} else {
				/* pEntry->PortSecured = WPA_802_1X_PORT_SECURED; */
				/* TODO: shiang-usw, need to replace upper setting with tr_entry */
				pAd->MacTab.tr_entry[pEntry->wcid].PortSecured = WPA_802_1X_PORT_SECURED;
			}

#endif /* ADHOC_WPA2PSK_SUPPORT */

			if (pEntry /*&& (Elem->Wcid == WCID_NO_MATCHED(pAd))*/) {
				ASIC_SEC_INFO Info = {0};
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_BOTH;
				Info.Wcid = pEntry->wcid;
				Info.BssIndex = wdev->func_idx;
				Info.Cipher = wdev->SecConfig.GroupCipher;
				Info.KeyIdx = wdev->SecConfig.GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
				idx = wdev->SecConfig.GroupKeyId;
				/* HW_SET_WCID_SEC_INFO(pAd, BSS0, idx, */
				/* pEntry->SecConfig.GroupCipher, */
				/* pEntry->wcid, */
				/* SHAREDKEYTABLE); */
			}
		}

		if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
			pEntry->LastBeaconRxTime = Now;
#ifdef IWSC_SUPPORT

			if (pEntry->bUpdateInfoFromPeerBeacon == FALSE) {
				if (StaUpdateMacTableEntry(pAd,
										   pEntry,
										   MaxSupportedRateIn500Kbps,
										   &bcn_ie_list->HtCapability,
										   &bcn_ie_list->AddHtInfo,
										   bcn_ie_list->AddHtInfoLen,
										   NULL,
										   bcn_ie_list->CapabilityInfo) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ADHOC 2 - Add Entry failed.\n");
					return;
				}

				if (ADHOC_ON(pAd) && pEntry) {
					RTMPSetSupportMCS(pAd,
									  OPMODE_STA,
									  pEntry,
									  rate,
#ifdef DOT11_VHT_AC
									  HAS_VHT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists),
									  &bcn_ie_list->vht_cap_ie,
#endif /* DOT11_VHT_AC */
									  &bcn_ie_list->HtCapability,
									  HAS_HT_CAPS_EXIST(bcn_ie_list->cmm_ies.ie_exists));
				}

				pEntry->bUpdateInfoFromPeerBeacon = TRUE;
			}

#endif /* IWSC_SUPPORT */
		}

		/* At least another peer in this IBSS, declare MediaState as CONNECTED */
		if (!STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
			STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
			RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
			pAd->ExtraInfo = GENERAL_LINK_UP;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ADHOC  fSTA_STATUS_MEDIA_STATE_CONNECTED.\n");
		}

#ifdef IWSC_SUPPORT

		if (pStaCfg->IWscInfo.bSendEapolStart &&
			(pAd->Mlme.IWscMachine.CurrState != IWSC_WAIT_PIN) &&
			(pStaCfg->WscControl.WscConfMode == WSC_ENROLLEE)) {
			pStaCfg->IWscInfo.bSendEapolStart = FALSE;
			pWpsCtrl->WscState = WSC_STATE_LINK_UP;
			pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
			NdisMoveMemory(pWpsCtrl->EntryAddr, pWpsCtrl->WscPeerMAC, MAC_ADDR_LEN);
			WscSendEapolStart(pAd, pWpsCtrl->WscPeerMAC, STA_MODE);
		}

#endif /* IWSC_SUPPORT */
	}

	return TRUE;
}

VOID Adhoc_checkPeerBeaconLost(RTMP_ADAPTER *pAd)
{
}

#endif /* CONFIG_STA_ADHOC_SUPPORT */
