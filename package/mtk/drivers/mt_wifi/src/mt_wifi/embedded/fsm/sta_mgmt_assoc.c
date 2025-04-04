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
	sta_mgmt_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"


struct _assoc_api_ops sta_assoc_api;

static VOID set_mlme_rsn_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PMAC_TABLE_ENTRY pEntry)
{
	ULONG Idx = 0;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	/* Set New WPA information */
	Idx = BssTableSearch(ScanTab, pEntry->Addr, wdev->channel);
	if (Idx == BSS_NOT_FOUND) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ASSOC - Can't find BSS after receiving Assoc response\n");
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (ScanTab->BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;
			USHORT tmp_len = 0;

			pVIE = ScanTab->BssEntry[Idx].VarIEs;
			len = ScanTab->BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;
				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
					|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))) {
					if ((tmp_len + pEid->Len + 2) > MAX_LEN_OF_RSNIE)
						break;
					NdisMoveMemory(&pEntry->RSN_IE[tmp_len], pVIE, (pEid->Len + 2));
					tmp_len += (pEid->Len + 2);
					pEntry->RSNIE_Len = tmp_len;
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "%s():=> Store RSN_IE for WPA SM negotiation\n", __func__);
				}
				/* For WPA2/WPA2PSK/WPA3/WPA3PSK */
				else if ((pEid->Eid == IE_RSN)
					 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					 && (IS_AKM_WPA2(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)
						 || IS_AKM_OWE(wdev->SecConfig.AKMMap))) {
					if ((tmp_len + pEid->Len + 2) > MAX_LEN_OF_RSNIE)
						break;
					NdisMoveMemory(&pEntry->RSN_IE[tmp_len], pVIE, (pEid->Len + 2));
					tmp_len += (pEid->Len + 2);
					pEntry->RSNIE_Len = tmp_len;
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "%s():=> Store RSN_IE for WPA2 SM negotiation\n", __func__);
				}


				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

		}

		if (pEntry->RSNIE_Len == 0) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():=> no RSN_IE\n", __func__);
		} else {
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
		}
	}

}

/*
	==========================================================================
	Description:
		Association timeout procedure. After association timeout, this function
		will be called and it will put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_assoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ASSOC - enqueue ASSOC_FSM_ASSOC_TIMEOUT\n");

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_ASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_assoc_timeout);
BUILD_TIMER_FUNCTION(sta_assoc_timeout);


/*
	==========================================================================
	Description:
		Reassociation timeout procedure. After reassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_reassoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_REASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_reassoc_timeout);
BUILD_TIMER_FUNCTION(sta_reassoc_timeout);

/*
	==========================================================================
	Description:
		Disassociation timeout procedure. After disassociation timeout, this
		function will be called and put a message into the MLME queue
	Parameters:
		Standard timer parameters

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_disassoc_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = pContext->pAd;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_DISASSOC_TIMEOUT, 0, NULL, 0, pContext->wdev);
	RTMP_MLME_HANDLER(pAd);
}
DECLARE_TIMER_FUNCTION(sta_disassoc_timeout);
BUILD_TIMER_FUNCTION(sta_disassoc_timeout);

/* Link down report*/
static VOID sta_link_down_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (pAd != NULL) {

		if ((pStaCfg->wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED) &&
			(INFRA_ON(pStaCfg))) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LinkDownExec(): disassociate with current AP...\n");
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING);
			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
			pAd->ExtraInfo = GENERAL_LINK_DOWN;
		}
	}
}
DECLARE_TIMER_FUNCTION(sta_link_down_exec);
BUILD_TIMER_FUNCTION(sta_link_down_exec);


#ifdef WPA_SUPPLICANT_SUPPORT
VOID ApcliSendAssocIEsToWpaSupplicant(
	IN RTMP_ADAPTER *pAd,
	IN UINT ifIndex)
{
	RTMP_STRING custom[IW_CUSTOM_MAX] = {0};

	if ((pAd->StaCfg[ifIndex].ReqVarIELen + 17) <= IW_CUSTOM_MAX) {
		sprintf(custom, "ASSOCINFO_ReqIEs=");
		NdisMoveMemory(custom + 17, pAd->StaCfg[ifIndex].ReqVarIEs, pAd->StaCfg[ifIndex].ReqVarIELen);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_REQIE_EVENT_FLAG, NULL, (PUCHAR)custom, pAd->StaCfg[ifIndex].ReqVarIELen + 17);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_ASSOCINFO_EVENT_FLAG, NULL, NULL, 0);
	} else
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "pAd->StaCfg[%d].ReqVarIELen + 17 > MAX_CUSTOM_LEN\n", ifIndex);

	return;
}
#endif /* WPA_SUPPLICANT_SUPPORT */

static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN PEDCA_PARM pEdcaParm,
	IN IE_LISTS * ie_list,
	IN MAC_TABLE_ENTRY *pEntry)
{
	PSTA_ADMIN_CONFIG pApCliEntry = GetStaCfgByWdev(pAd, pEntry->wdev);
	UINT_8 OmacIdx;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;
	BOOLEAN has_ht_cap = HAS_HT_CAPS_EXIST(cmm_ies->ie_exists);

	ASSERT(pApCliEntry);
	if (!pApCliEntry)
		return;

	pApCliEntry->MlmeAux.BssType = BSS_INFRA;
	pApCliEntry->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	check_legacy_rates(rate, &pApCliEntry->MlmeAux.rate, &pApCliEntry->wdev);
#if (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, has_ht_cap ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __func__);
#else
	if (has_ht_cap) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,  "%s===> 11n HT STA\n", __func__);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,  "%s===> legacy STA\n", __func__);
	}
#endif
#ifdef DOT11_N_SUPPORT

	if (has_ht_cap && WMODE_CAP_N(pApCliEntry->wdev.PhyMode))
		RTMPCheckHt(pAd, pEntry->wcid, &cmm_ies->ht_cap, &cmm_ies->ht_op);

#endif /* DOT11_N_SUPPORT */
	OmacIdx  = HcGetOmacIdx(pAd, &pApCliEntry->wdev);
	chip_arch_set_aid(pAd, pEntry->Aid, OmacIdx);
#ifdef DOT11_VHT_AC
	RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
	RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_op, sizeof(VHT_OP_IE));
	CLR_VHT_CAPS_EXIST(pApCliEntry->MlmeAux.ie_exists);
	CLR_VHT_OP_EXIST(pApCliEntry->MlmeAux.ie_exists);

	if (WMODE_CAP_AC(pApCliEntry->wdev.PhyMode)
			&& HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)
			&& HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				"There is vht le at Assoc Rsp ifIndex=%d\n", wdev->func_idx);
		NdisMoveMemory(&pApCliEntry->MlmeAux.vht_cap,
				&(cmm_ies->vht_cap), SIZE_OF_VHT_CAP_IE);
		SET_VHT_CAPS_EXIST(pApCliEntry->MlmeAux.ie_exists);
		NdisMoveMemory(&pApCliEntry->MlmeAux.vht_op,
				&(cmm_ies->vht_op), SIZE_OF_VHT_OP_IE);
		SET_VHT_OP_EXIST(pApCliEntry->MlmeAux.ie_exists);
	}
#endif /* DOT11_VHT_AC */
}

/*
	==========================================================================
	Description:
		procedures on IEEE 802.11/1999 p.376
	Parametrs:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_assoc_post_proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN USHORT Aid,
	IN PEDCA_PARM pEdcaParm,
	IN IE_LISTS *ie_list,
	IN MAC_TABLE_ENTRY *pEntry)
{
	/* AP might use this additional ht info IE */
	ULONG Idx;
	struct wifi_dev *wdev = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	BSS_TABLE *ScanTab = NULL;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;

	if (!pEntry)
		return;

	wdev = pEntry->wdev;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	pStaCfg->MlmeAux.BssType = BSS_INFRA;
	pStaCfg->MlmeAux.Aid = Aid;
	pStaCfg->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	chip_arch_set_aid(pAd, Aid, 0);

#ifdef DOT11_N_SUPPORT
	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		/* Some HT AP might lost WMM IE. We add WMM ourselves. beacuase HT requires QoS on.*/
		if (pEdcaParm->bValid == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s(): peer doesn't have WmmIE, add it due to we are running at HT+",
					__func__);
			set_default_sta_edca_param(pEdcaParm);
		}

		/* due to peer's assoc_resp is with HT_IE, update WmmCapable flag here. */
		if (wdev->bWmmCapable == FALSE) {
			wdev->bWmmCapable = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s(): update WmmCapable flag due to we are running at HT+",
					__func__);
		}
	}

#endif /* DOT11_N_SUPPORT */

	if (pEdcaParm->bValid == TRUE)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	NdisMoveMemory(&pStaCfg->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

	check_legacy_rates(rate, &pStaCfg->MlmeAux.rate, wdev);

#ifdef DOT11_N_SUPPORT
	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists))
		check_ht(pAd, pEntry->wcid, cmm_ies);

#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) && HAS_VHT_OP_EXIST(cmm_ies->ie_exists))
		check_vht(pAd, pEntry->wcid, cmm_ies);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	/* Set New WPA information */
	Idx = BssTableSearch(ScanTab, pAddr2, pStaCfg->MlmeAux.Channel);

	if (Idx == BSS_NOT_FOUND)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ASSOC - Can't find BSS after receiving Assoc response\n");
	else if (ie_list->RSNIE_Len >= MIN_LEN_OF_RSNIE) {
		PUCHAR rsnie_from_resp = ie_list->RSN_IE;
		USHORT len = ie_list->RSNIE_Len;

		/*
		 * for OWE, AP also takes RSNE in assoc resp for take out PMKID and ECDH IE to show AP's public key,
		 * we need to check RSN_IE ie list here.
		 */
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* For WPA2/WPA2PSK/WPA3/WPA3PSK */
		if (IS_AKM_WPA2(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			NdisMoveMemory(pEntry->RSN_IE, rsnie_from_resp, len);
			pEntry->RSNIE_Len = len;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s():=> Store Assoc Resp RSN_IE for WPA2 SM negotiation\n", __func__);
		}

		if (pEntry->RSNIE_Len == 0)
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():=> no RSN_IE\n", __func__);
		else
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);

#ifdef CONFIG_OWE_SUPPORT
		if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			BOOLEAN need_process_ecdh_ie = FALSE;
			UINT8 *pmkid = NULL;
			UINT8 pmkid_count = 0;

			pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
								ie_list->RSNIE_Len,
								PMKID_LIST,
								&pmkid_count);
			if (pmkid != NULL) {
				INT idx;
				BOOLEAN FoundPMK = FALSE;

				/*	Search chched PMKID, append it if existed */
				for (idx = 0; idx < PMKID_NO; idx++) {
					if (NdisEqualMemory(pAddr2, &pStaCfg->SavedPMK[idx].BSSID, 6)) {
						FoundPMK = TRUE;
						break;
					}
				}

				if (FoundPMK == FALSE) {
					need_process_ecdh_ie = TRUE;
					MTWF_DBG(pAd, DBG_CAT_AP,
						 DBG_SUBCAT_ALL,
						 DBG_LVL_ERROR,
						 "cannot find match PMKID\n");
				} else if ((pEntry->SecConfig.pmkid) &&
					  ((RTMPEqualMemory(pmkid, pEntry->SecConfig.pmkid, LEN_PMKID)) != 0)) {
					/*
					 * if STA would like to use PMK CACHE,
					 * it stored the PMKID in assoc req stage already.
					 * no need to restore it again here.
					 */
					MTWF_DBG(pAd, DBG_CAT_AP,
						 DBG_SUBCAT_ALL,
						 DBG_LVL_ERROR,
						 "PMKID doesn't match STA sent\n");
					need_process_ecdh_ie = TRUE;
				}
			} else
				need_process_ecdh_ie = TRUE;

			if (need_process_ecdh_ie == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_AP,
					 DBG_SUBCAT_ALL,
					 DBG_LVL_INFO,
					"%s: do normal ECDH procedure\n", __func__);
				process_ecdh_element(pAd,
						pEntry,
						(EXT_ECDH_PARAMETER_IE *)&ie_list->ecdh_ie,
						ie_list->ecdh_ie.length,
						SUBTYPE_ASSOC_RSP);
			}
		}
#endif /*CONFIG_OWE_SUPPORT*/
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (ScanTab->BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;

			pVIE = ScanTab->BssEntry[Idx].VarIEs;
			len = ScanTab->BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;

				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
						|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "%s():=> Store RSN_IE for WPA SM negotiation\n", __func__);
				}
				/* For WPA2/WPA2PSK */
				else if ((pEid->Eid == IE_RSN)
						 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
						 && (IS_AKM_WPA2(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap) ||
						     IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap) ||
						     IS_AKM_OWE(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "%s():=> Store RSN_IE for WPA2 SM negotiation\n", __func__);
				}

				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

#ifdef DOT11R_FT_SUPPORT

			if (pStaCfg->Dot11RCommInfo.bFtSupport &&
				pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
				(pStaCfg->MlmeAux.FtIeInfo.GtkLen != 0)) {
				/* extract GTK related information */
				FT_ExtractGTKSubIe(pAd,
								   &pAd->MacTab.Content[MCAST_WCID],
								   &pStaCfg->MlmeAux.FtIeInfo);
			}

#endif /* DOT11R_FT_SUPPORT */
		}

		if (pEntry->RSNIE_Len == 0)
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():=> no RSN_IE\n", __func__);
		else
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
	}
}

static BOOLEAN sta_block_checker(struct wifi_dev *wdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	USHORT Status;

	/* Block all authentication request durning WPA block period */
	if (pStaCfg->bBlockAssoc == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - Block ReAssoc request durning WPA block period!\n");
		Status = MLME_STATE_MACHINE_REJECT;
		cntl_auth_assoc_conf(wdev, CNTL_MLME_REASSOC_CONF, Status); /* CNTL_MLME_ASSOC_CONF */
		return TRUE;
	}

	return FALSE;
}

/*
	==========================================================================
	Description:
		mlme assoc req handling procedure
	Parameters:
		Adapter - Adapter pointer
		Elem - MLME Queue Element
	Pre:
		the station has been authenticated and the following information is stored in the config
			-# SSID
			-# supported rates and their length
			-# listen interval (Adapter->StaCfg[0].default_listen_count)
			-# Transmit power  (Adapter->StaCfg[0].tx_power)
	Post  :
		-# An association request frame is generated and sent to the air
		-# Association timer starts
		-# Association state -> ASSOC_WAIT_RSP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR ApAddr[6];
	HEADER_802_11 AssocHdr;
	USHORT ListenIntv;
	ULONG Timeout;
	USHORT CapabilityInfo;
	BOOLEAN TimerCancelled;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT VarIesOffset = 0;
	USHORT Status;
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	MAC_TABLE_ENTRY *pAPEntry = GetAssociatedAPByWdev(pAd, Elem->wdev);
	struct legacy_rate *rate = &pStaCfg->MlmeAux.rate;
	RALINK_TIMER_STRUCT *assoc_timer = NULL;
	UCHAR SsidIe    = IE_SSID;
	UCHAR SupRateIe = IE_SUPP_RATES;
#ifdef MBO_SUPPORT
	PWSC_CTRL pWscControl = &wdev->WscControl;
#endif /* MBO_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept = NULL;
#endif /* MAC_REPEATER_SUPPORT */

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(DPP_SUPPORT) || defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
	USHORT ifIndex = wdev->func_idx;
#endif

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */

	if (!pStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pStaCfg=NULL,return!\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev(type=%d,idx=%d,func_idx=%d)\n",
		wdev->wdev_type, wdev->wdev_idx, wdev->func_idx);

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (!pAPEntry) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev(type=%d,idx=%d,func_idx=%d),pAPEntry(NULL),return\n",
				wdev->wdev_type, wdev->wdev_idx, wdev->func_idx);
			return;
		}
	}

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		if (!pAPEntry) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ACPLI is not connected, return repeater connection\n");
			return;
		}
		rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		ASSERT(rept);
		assoc_timer = &rept->ApCliAssocTimer;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		assoc_timer = &pStaCfg->MlmeAux.AssocTimer;
	}
	assoc_fsm_state_transition(wdev, ASSOC_IDLE);

	if (sta_block_checker(wdev) == TRUE)
		return;

	/* check sanity first */
	if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "ASSOC - MlmeAssocReqAction() sanity check failed !!!!!!\n");
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status);
		return;
	}

	/* insert MacRepeater Mac Entry here */
#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		pAPEntry = MacTableInsertEntry(
					   pAd,
					   (PUCHAR)(pStaCfg->MlmeAux.Bssid),
					   &rept->wdev,
					   ENTRY_REPEATER,
					   OPMODE_AP,
					   TRUE);


		if (pAPEntry) {
#ifdef DOT11_SAE_SUPPORT
			/* repeater must enable PMF when AKM is WPA3PSK */
			if (IS_AKM_WPA3PSK(pStaCfg->wdev.SecConfig.AKMMap)) {
				MAC_TABLE_ENTRY *pEntryFromApcli = (MAC_TABLE_ENTRY *)pStaCfg->pAssociatedAPEntry;
				if (pEntryFromApcli) {
					pAPEntry->SecConfig.PmfCfg.igtk_cipher = pEntryFromApcli->SecConfig.PmfCfg.igtk_cipher;
					pAPEntry->SecConfig.PmfCfg.PMFSHA256 = pEntryFromApcli->SecConfig.PmfCfg.PMFSHA256;
				}
				NdisMoveMemory(pAPEntry->SecConfig.PMK, rept->rept_PMK, LEN_PMK);
				pAPEntry->SecConfig.PmfCfg.MFPR = TRUE;
				pAPEntry->SecConfig.PmfCfg.MFPC = TRUE;
				hex_dump_with_lvl("repeater mode: SecConfig.PMK:", (char *)pAPEntry->SecConfig.PMK, LEN_PMK, DBG_LVL_INFO);
			}
#endif
			pAPEntry->SecConfig.rsnxe_len = pStaCfg->MlmeAux.rsnxe_len;
			NdisMoveMemory(pAPEntry->SecConfig.rsnxe_content, pStaCfg->MlmeAux.rsnxe_content, pStaCfg->MlmeAux.rsnxe_len);
			rept->pMacEntry = pAPEntry;
			pAPEntry->pReptCli = rept;
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "repeater pEntry insert fail");
			return;
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
	{
		struct _build_ie_info ie_info = {0};

		ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
		ie_info.channel = pStaCfg->MlmeAux.Channel;
		ie_info.phy_mode = wdev->PhyMode;
		ie_info.wdev = wdev;
#ifdef WPA_SUPPLICANT_SUPPORT
		/* for dhcp issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key  */
		RTMPWPARemoveAllKeys(pAd, wdev);
#endif /* WPA_SUPPLICANT_SUPPORT */
		RTMPCancelTimer(assoc_timer, &TimerCancelled);
		/* Get an unused nonpaged memory */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "ASSOC - MlmeAssocReqAction() allocate memory failed\n");
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status);
			/*
						 ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
						 MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
								sizeof(CTRL_JOIN_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

			 */
			return;
		}

		/* Add by James 03/06/27 */
		pStaCfg->AssocInfo.Length =
			sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		/* Association don't need to report MAC address */
		pStaCfg->AssocInfo.AvailableRequestFixedIEs =
			NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;
		pStaCfg->AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
		pStaCfg->AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;
		/* Only reassociate need this */
		/*COPY_MAC_ADDR(pStaCfg->AssocInfo.RequestFixedIEs.CurrentAPAddress, ApAddr); */
		pStaCfg->AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		NdisZeroMemory(pStaCfg->ReqVarIEs, MAX_VIE_LEN);
		/* First add SSID */
		VarIesOffset = 0;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &SsidIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->MlmeAux.SsidLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
		VarIesOffset += pStaCfg->MlmeAux.SsidLen;
		/* Second add Supported rates */
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &SupRateIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &rate->sup_rate_len, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, rate->sup_rate, rate->sup_rate_len);
		VarIesOffset += rate->sup_rate_len;
		/* End Add by James */

		/*
		   CapabilityInfo already sync value with AP in PeerBeaconAtJoinAction.
		   But we need to clean Spectrum Management bit here, if we disable bIEEE80211H in infra sta
		 */
		if (!((wdev->channel > 14) &&
			  (pAd->CommonCfg.bIEEE80211H == TRUE)))
			CapabilityInfo &= (~0x0100);
#ifdef DOT11K_RRM_SUPPORT
		if ((IS_RRM_ENABLE(wdev)))
			CapabilityInfo |= RRM_CAP_BIT;
#endif
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "ASSOC - Send ASSOC request...\n");
		MgtMacHeaderInitExt(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr,
							wdev->if_addr, ApAddr);
		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &AssocHdr,
						  2, &CapabilityInfo,
						  2, &ListenIntv,
						  1, &SsidIe,
						  1, &pStaCfg->MlmeAux.SsidLen,
						  pStaCfg->MlmeAux.SsidLen, pStaCfg->MlmeAux.Ssid,
						  END_OF_ARGS);
		FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, pOutBuffer + FrameLen);

		FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
					rate->ext_rate, rate->ext_rate_len, pOutBuffer + FrameLen);

#ifdef DOT11R_FT_SUPPORT

		/* Add MDIE if we are connection to DOT11R AP */
		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->MlmeAux.MdIeInfo.Len) {
			/* MDIE */
			FT_InsertMdIE(pOutBuffer + FrameLen, &FrameLen,
						  pStaCfg->MlmeAux.MdIeInfo.MdId,
						  pStaCfg->MlmeAux.MdIeInfo.FtCapPlc);
		}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_N_SUPPORT

		/*
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP &&
			IS_INVALID_HT_SECURITY(pAPEntry->SecConfig.PairwiseCipher)) {
			/* Force to None-HT mode due to WiFi 11n policy */
			CLR_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
#ifdef DOT11_VHT_AC
			CLR_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
#endif /* DOT11_VHT_AC */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "Force STA as Non-HT mode\n");
		}

		/* HT */
		if (HAS_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)
			&& WMODE_CAP_N(wdev->PhyMode)) {
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn && (rept != NULL))
				ie_info.ReptMacTabWCID = rept->pMacEntry->wcid;
#endif /* MAC_REPEATER_SUPPORT */

			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);
		}

		/* RSN start */
		if (pAd->CommonCfg.wifi_cert) {
			/*
				Let WPA(#221) Element ID on the end of this association frame.
				Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp.
				For example: Put Vendor Specific IE on the front of WPA IE.
				This happens on AP (Model No:Linksys WRK54G)
			*/
			printk("======================================> check WPA2PSK :%d\n", IS_AKM_PSK(pAPEntry->SecConfig.AKMMap));

#if defined(CONFIG_OWE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
			/* Allow OWE STA to connect to OPEN AP */
			if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pAPEntry->SecConfig.AKMMap))) {
				pStaCfg->AKMMap = pAPEntry->SecConfig.AKMMap;
				pStaCfg->PairwiseCipher = pAPEntry->SecConfig.PairwiseCipher;
			}
#endif

#ifdef APCLI_CFG80211_SUPPORT
			/*pStaCfg->ReqVarIELen = 0;*/
			/*NdisZeroMemory(pStaCfg->ReqVarIEs, MAX_VIE_LEN);*/
			if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) ==  WPA_SUPPLICANT_ENABLE) {
				ULONG TmpWpaAssocIeLen = 0;
#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
				UCHAR * wpa_ie = NULL;
				UINT wpa_ie_len = 0;
				UCHAR *assoc_ie = pStaCfg->wpa_supplicant_info.pWpaAssocIe;
				UINT assoc_ie_len = pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
#endif
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s:: APCLI WPA_ASSOC_IE FROM SUPPLICANT\n", __func__);

#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
				if (IS_AKM_WPA3PSK(pAPEntry->SecConfig.AKMMap) || IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
					UCHAR *pBuf;
					UINT8 count;

					INT idx;
					MAC_TABLE_ENTRY *pentry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];
					PBSSID_INFO psaved_pmk = &pStaCfg->SavedPMK[0];
					VOID *psaved_pmk_lock = (VOID *)&pStaCfg->SavedPMK_lock;
					UINT32 sec_akm = 0;

					if (IS_AKM_SAE_SHA256(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_SAE_SHA256(sec_akm);
					else if (IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_OWE(sec_akm);

					idx = sta_search_pmkid_cache(pAd, ApAddr, ifIndex, wdev,
							sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
					if ((idx != INVALID_PMKID_IDX) && (*assoc_ie == 0x30)) {
						wpa_ie_len = *(assoc_ie + 1);
						pBuf = WPA_ExtractSuiteFromRSNIE(assoc_ie, wpa_ie_len + 2, RSN_CAP_INFO, &count);
						os_alloc_mem(pAd, &wpa_ie, (wpa_ie_len + 2 + 22));
						if (wpa_ie && pBuf) {
							UINT16	pmk_count = 1;
							UCHAR *pos;

							wpa_ie_len = (pBuf - assoc_ie);
							os_move_mem(wpa_ie, assoc_ie, wpa_ie_len + sizeof(RSN_CAPABILITIES));
							wpa_ie_len += sizeof(RSN_CAPABILITIES);
							pos = wpa_ie + wpa_ie_len;
							if (psaved_pmk_lock)
								NdisAcquireSpinLock(psaved_pmk_lock);

							/*Update the pentry->pmkcache from the Saved PMK cache */
							pentry->SecConfig.pmkid = psaved_pmk[idx].PMKID;
							pentry->SecConfig.pmk_cache = psaved_pmk[idx].PMK;

							if (psaved_pmk_lock)
								NdisReleaseSpinLock(psaved_pmk_lock);

							pmk_count = cpu2le16(pmk_count);
							NdisMoveMemory(pos, &pmk_count, sizeof(pmk_count));
							pos += sizeof(pmk_count);
							NdisMoveMemory(pos, pentry->SecConfig.pmkid, LEN_PMKID);
							pos += LEN_PMKID;
							wpa_ie_len += (2 + LEN_PMKID);
							pBuf = WPA_ExtractSuiteFromRSNIE(assoc_ie, (*(assoc_ie + 1) + 2), G_MGMT_SUITE, &count);
							if (pBuf) {
								NdisMoveMemory(pos, pBuf, 4);
								wpa_ie_len += 4;
							}
							wpa_ie[1] = (wpa_ie_len - 2);
						}
					} else {
						pentry->SecConfig.pmkid = NULL;
						pentry->SecConfig.pmk_cache = NULL;
					}
				}

				if (wpa_ie) {
					UINT8 ie_len = *(assoc_ie + 1) + 2;
					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
								wpa_ie_len, wpa_ie, END_OF_ARGS);
					FrameLen += TmpWpaAssocIeLen;

					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
								(assoc_ie_len - ie_len), (assoc_ie + ie_len),
								END_OF_ARGS);
					FrameLen += TmpWpaAssocIeLen;
					/*VarIesOffset = 0;*/
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
								wpa_ie, wpa_ie_len);
					VarIesOffset += wpa_ie_len;
					/*VarIesOffset = 0;*/
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
								(assoc_ie + ie_len), (assoc_ie_len - ie_len));
					VarIesOffset += (assoc_ie_len - ie_len);
					/* Set Variable IEs Length */
					pStaCfg->ReqVarIELen = VarIesOffset;
					os_free_mem(wpa_ie);
				} else {
#endif
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
								pStaCfg->wpa_supplicant_info.WpaAssocIeLen, pStaCfg->wpa_supplicant_info.pWpaAssocIe,
								END_OF_ARGS);
				FrameLen += TmpWpaAssocIeLen;
				/*VarIesOffset = 0;*/
				NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
								pStaCfg->wpa_supplicant_info.pWpaAssocIe, pStaCfg->wpa_supplicant_info.WpaAssocIeLen);
				VarIesOffset += pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
				/* Set Variable IEs Length */
				pStaCfg->ReqVarIELen = VarIesOffset;
#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
}
#endif
			} else
#endif /* APCLI_CFG80211_SUPPORT */

			/* Append RSN_IE when WPAPSK OR WPA2PSK, */
			if ((IS_AKM_PSK(pAPEntry->SecConfig.AKMMap)
#ifdef WPA_SUPPLICANT_SUPPORT
				 || IS_AKM_WPA2(pAPEntry->SecConfig.AKMMap)
#endif /* WPA_SUPPLICANT_SUPPORT */
				)
#ifdef WSC_STA_SUPPORT
				&& ((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
					((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
					 !(wdev->WscControl.bWscTrigger)))
#endif /* WSC_STA_SUPPORT */
			   ) {
#ifdef WPA_SUPPLICANT_SUPPORT

				if (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == FALSE) {
					if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
						; /*do nothing, RSNE content should be from wpa_supplicant */
					} else

						/* Check for WPA PMK cache list */
						if (IS_AKM_WPA2(pAPEntry->SecConfig.AKMMap)
							|| IS_AKM_WPA3_192BIT(pAPEntry->SecConfig.AKMMap)
							|| IS_AKM_WPA3PSK(pAPEntry->SecConfig.AKMMap)
							|| IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
							INT idx;
							BOOLEAN FoundPMK = FALSE;

							/* Search chched PMKID, append it if existed */
							for (idx = 0; idx < PMKID_NO; idx++) {
								if (NdisEqualMemory(ApAddr, &pStaCfg->SavedPMK[idx].BSSID, 6)) {
									FoundPMK = TRUE;
									break;
								}
							}

							/*
							   When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
							   AP would not do PMK cache with STA after STA re-connect to AP again.
							   In this case, driver doesn't need to send PMKID to AP and WpaSupplicant.

							 */ /* TODO: pAd->CommonCfg.LastBssid */
							if (NdisEqualMemory(pStaCfg->MlmeAux.Bssid, pStaCfg->LastBssid, MAC_ADDR_LEN))
								FoundPMK = FALSE;

							if (FoundPMK)
								store_pmkid_cache_in_sec_config(pAd, pAPEntry, idx);
						}
				}
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(DPP_SUPPORT)
				if (IS_AKM_WPA3PSK(pStaCfg->wdev.SecConfig.AKMMap) ||
#ifdef DPP_SUPPORT
						IS_AKM_DPP(pStaCfg->wdev.SecConfig.AKMMap) ||
#endif /* DPP_SUPPORT */
					IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap)) {
					INT idx;
					MAC_TABLE_ENTRY *pentry = (MAC_TABLE_ENTRY *)NULL;
					PBSSID_INFO psaved_pmk = NULL;
					VOID *psaved_pmk_lock = NULL;
					UINT32 sec_akm = 0;

					if (IS_AKM_SAE_SHA256(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_SAE_SHA256(sec_akm);
					else if (IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_OWE(sec_akm);

#ifdef DPP_SUPPORT
					if (IS_AKM_DPP(pStaCfg->wdev.SecConfig.AKMMap)) {
						sec_akm = 0;
						SET_AKM_DPP(sec_akm);
					}
#endif


#ifdef MAC_REPEATER_SUPPORT
					if ((pAd->ApCfg.bMACRepeaterEn) && (wdev->wdev_type == WDEV_TYPE_REPEATER)) {
						pentry = rept->pMacEntry;
						psaved_pmk = &(rept->SavedPMK[0]);
						psaved_pmk_lock = (void *)&(rept->SavedPMK_lock);
					}
					 else
#endif /* MAC_REPEATER_SUPPORT */
					{
						pentry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];
						psaved_pmk = &pStaCfg->SavedPMK[0];
						psaved_pmk_lock = (VOID *)&pStaCfg->SavedPMK_lock;
					}

					idx = sta_search_pmkid_cache(pAd, ApAddr, ifIndex, wdev,
							sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);

					if (idx != INVALID_PMKID_IDX) {
						if (psaved_pmk_lock)
							NdisAcquireSpinLock(psaved_pmk_lock);

						/*Update the pentry->pmkcache from the Saved PMK cache */
						pentry->SecConfig.pmkid = psaved_pmk[idx].PMKID;
						pentry->SecConfig.pmk_cache = psaved_pmk[idx].PMK;

						if (psaved_pmk_lock)
							NdisReleaseSpinLock(psaved_pmk_lock);
					} else {
						printk("PMKID not found in cache: Normal Assoc\n");
						pentry->SecConfig.pmkid = NULL;
						pentry->SecConfig.pmk_cache = NULL;
					}
				}
#endif

#ifdef WPA_SUPPLICANT_SUPPORT
				/*
					Can not use SIOCSIWGENIE definition, it is used in wireless.h
					We will not see the definition in MODULE.
					The definition can be saw in UTIL and NETIF.
				*/
				if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE)
					&& (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == TRUE))
					;
				else
#endif /* WPA_SUPPLICANT_SUPPORT */
				{
					ULONG TempLen = 0;
					CHAR rsne_idx = 0;
					{ /* Todo by Eddy: It's not good code */
						struct _SECURITY_CONFIG *pSecConfig = &pAPEntry->SecConfig;
						UINT32 AKMMap = pSecConfig->AKMMap;
						UINT32 PairwiseCipher = pSecConfig->PairwiseCipher;

						UINT32 GroupCipher = pSecConfig->GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
						/* Need to fill the pSecConfig->PmfCfg to let WPAMakeRSNIE() generate correct RSNIE*/
						{
							RSN_CAPABILITIES RsnCap;

							NdisMoveMemory(&RsnCap, &pStaCfg->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
							RsnCap.word = cpu2le16(RsnCap.word);
							/* init to FALSE */
							pSecConfig->PmfCfg.UsePMFConnect = FALSE;
							pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;

							/*mismatch case*/
							if (((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
								|| ((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
								pSecConfig->PmfCfg.UsePMFConnect = FALSE;
								pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
							}

							if ((pSecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
								pSecConfig->PmfCfg.UsePMFConnect = TRUE;

								if ((pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
									pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
							}
						}
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SUPPORT
						/* ocv_support set to true temporary so that ocvc
						* bit can be set in assoc req. It will be finally set
						* when AP also supports OCV */
						if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
							pSecConfig->ocv_support = TRUE;
#endif
						WPAMakeRSNIE(pStaCfg->wdev.wdev_type, pSecConfig, pAPEntry);
#ifdef APCLI_SUPPORT
						if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
							pSecConfig->ocv_support = FALSE;
#endif
						pStaCfg->AKMMap = AKMMap;
						pStaCfg->PairwiseCipher = PairwiseCipher;
						pStaCfg->GroupCipher = GroupCipher;
					}

					for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
						if (pAPEntry->SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
							continue;

						MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
										  1, &pAPEntry->SecConfig.RSNE_EID[rsne_idx][0],
										  1, &pAPEntry->SecConfig.RSNE_Len[rsne_idx],
										  pAPEntry->SecConfig.RSNE_Len[rsne_idx], &pAPEntry->SecConfig.RSNE_Content[rsne_idx][0],
										  END_OF_ARGS);
						FrameLen += TempLen;
					}
				}

#ifdef WPA_SUPPLICANT_SUPPORT

				/*
					Can not use SIOCSIWGENIE definition, it is used in wireless.h
					We will not see the definition in MODULE.
					The definition can be saw in UTIL and NETIF.
				*/
				if (((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) !=
					 WPA_SUPPLICANT_ENABLE)
					|| (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == FALSE))
#endif /* WPA_SUPPLICANT_SUPPORT */
				{
					CHAR rsne_idx = 0;

					for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
						if (pStaCfg->wdev.SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
							continue;

						/* Append Variable IE */
						NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_EID[rsne_idx][0], 1);
						VarIesOffset += 1;
						NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx], 1);
						VarIesOffset += 1;
						NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Content[rsne_idx][0],
									   pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx]);
						VarIesOffset += pStaCfg->RSNIE_Len;
						/* Set Variable IEs Length */
						pStaCfg->ReqVarIELen = VarIesOffset;
						break;
					}
				}
			}
		}
		/* RSN end */

		/* VHT/HE */
		if (HAS_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)
			&& WMODE_CAP_N(wdev->PhyMode)) {
#ifdef DOT11_VHT_AC
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#ifdef MAC_REPEATER_SUPPORT
			else if (pAd->ApCfg.bMACRepeaterEn) {
				struct _RTMP_CHIP_CAP *cap;
				cap = hc_get_chip_cap(pAd->hdev_ctrl);

				/* BFee function is limited if there is AID HW limitation*/
				if (cap->FlgHwTxBfCap & TXBF_AID_HW_LIMIT) {
					wlan_config_set_etxbf(wdev, SUBF_BFER);

					/* Just first cloned STA has full BF capability */
					if ((rept != NULL) && (pAd->fgClonedStaWithBfeeSelected == FALSE)) {
						MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;

						MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OriginalAddress[0~5] = "MACSTR"\n",
								 MAC2STR(rept->OriginalAddress));
						pEntry = rept->pMacEntry;

						if ((pEntry) && (HcIsBfCapSupport(pEntry->wdev) == TRUE)) {
							wlan_config_set_etxbf(wdev, SUBF_ALL);
							pAd->fgClonedStaWithBfeeSelected = TRUE;
							pAd->ReptClonedStaEntry_CliIdx = rept->wdev.func_idx;
						}
					}
				}
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
			FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
			if (WMODE_CAP_AX(wdev->PhyMode) && HAS_HE_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)) {
#ifdef MAC_REPEATER_SUPPORT
				if (wdev->wdev_type != WDEV_TYPE_REPEATER)
#endif /* MAC_REPEATER_SUPPORT */
				FrameLen += add_assoc_req_he_ies(wdev, (UINT8 *)(pOutBuffer + FrameLen));
			}
#endif /*DOT11_HE_AX*/
		}
		/* HE_6G */
		else if (WMODE_CAP_6G(wdev->PhyMode) && HAS_HE_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)) {
			FrameLen += add_assoc_req_he_ies(wdev, (UINT8 *)(pOutBuffer + FrameLen));
		}

#endif /* DOT11_N_SUPPORT */

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef MBO_SUPPORT
		if (IS_MBO_ENABLE(wdev))
			FrameLen += build_supp_op_class_ie(pAd, wdev, pOutBuffer + FrameLen);
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) && (pAd->MAPMode == MAP_CERT_MODE)) {
			if (IS_MBO_ENABLE(wdev) && pWscControl && !pWscControl->bWscTrigger)
				MakeMboOceIE(pAd, wdev, NULL, pOutBuffer + FrameLen, &FrameLen,
				MBO_FRAME_TYPE_ASSOC_REQ);
		} else {
			if (IS_MBO_ENABLE(wdev))
				MakeMboOceIE(pAd, wdev, NULL, pOutBuffer + FrameLen, &FrameLen,
				MBO_FRAME_TYPE_ASSOC_REQ);
		}
#else
		if (IS_MBO_ENABLE(wdev))
			MakeMboOceIE(pAd, wdev, NULL, pOutBuffer + FrameLen, &FrameLen,
			MBO_FRAME_TYPE_ASSOC_REQ);
#endif
#endif /* MBO_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
		if (IS_RRM_ENABLE(wdev)) {
			RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, wdev->func_idx);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s:: APCLI INSERT RM CAPA FrameLen = %lu\n", __func__, FrameLen);
		} else
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s:: APCLI NOT INSERT RM CAPA\n", __func__);
#endif

#ifdef CONFIG_MAP_SUPPORT
		if ((IS_MAP_ENABLE(pAd) && (pAd->MAPMode != MAP_CERT_MODE)) ||
			(!IS_MAP_ENABLE(pAd))
#ifdef MAP_6E_SUPPORT
			|| (IS_MAP_CERT_ENABLE(pAd) && WMODE_CAP_6G(wdev->PhyMode))
#endif
			)
#ifdef HOSTAPD_WPA3R3_SUPPORT
			FrameLen += build_rsnxe_ie(wdev, &wdev->SecConfig, pOutBuffer + FrameLen);
#else
			FrameLen += build_rsnxe_ie(&wdev->SecConfig, pOutBuffer + FrameLen);
#endif
#else
#ifdef HOSTAPD_WPA3R3_SUPPORT
		FrameLen += build_rsnxe_ie(wdev, &wdev->SecConfig, pOutBuffer + FrameLen);
#else
		FrameLen += build_rsnxe_ie(&wdev->SecConfig, pOutBuffer + FrameLen);
#endif
#endif

		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_REQ
		);

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);

		/* RSN start */
		if (!pAd->CommonCfg.wifi_cert) {
		/*
			Let WPA(#221) Element ID on the end of this association frame.
			Otherwise some AP will fail on parsing Element ID and set status fail on Assoc Rsp.
			For example: Put Vendor Specific IE on the front of WPA IE.
			This happens on AP (Model No:Linksys WRK54G)
		*/
		printk("======================================> check WPA2PSK :%d\n", IS_AKM_PSK(pAPEntry->SecConfig.AKMMap));

#ifdef CONFIG_OWE_SUPPORT
	/* Allow OWE STA to connect to OPEN AP */
	if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pAPEntry->SecConfig.AKMMap))) {
		pStaCfg->AKMMap = pAPEntry->SecConfig.AKMMap;
		pStaCfg->PairwiseCipher = pAPEntry->SecConfig.PairwiseCipher;
	}
#endif

#ifdef APCLI_CFG80211_SUPPORT
		/*pStaCfg->ReqVarIELen = 0;*/
		/*NdisZeroMemory(pStaCfg->ReqVarIEs, MAX_VIE_LEN);*/
		if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) ==  WPA_SUPPLICANT_ENABLE) {
			ULONG TmpWpaAssocIeLen = 0;
#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
			UCHAR *wpa_ie = NULL;
			UINT wpa_ie_len = 0;
			UCHAR *assoc_ie = pStaCfg->wpa_supplicant_info.pWpaAssocIe;
			UINT assoc_ie_len = pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
#endif
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s:: APCLI WPA_ASSOC_IE FROM SUPPLICANT\n", __func__);

#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
				if (IS_AKM_WPA3PSK(pAPEntry->SecConfig.AKMMap) || IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
					UCHAR *pBuf;
					UINT8 count;
					INT idx;
					MAC_TABLE_ENTRY *pentry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];
					PBSSID_INFO psaved_pmk = &pStaCfg->SavedPMK[0];
					VOID *psaved_pmk_lock = (VOID *)&pStaCfg->SavedPMK_lock;
					UINT32 sec_akm = 0;

					if (IS_AKM_SAE_SHA256(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_SAE_SHA256(sec_akm);
					else if (IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap))
						SET_AKM_OWE(sec_akm);

					idx = sta_search_pmkid_cache(pAd, ApAddr, ifIndex, wdev,
							sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
					if ((idx != INVALID_PMKID_IDX) && (*assoc_ie == 0x30)) {
						wpa_ie_len = *(assoc_ie + 1);
						pBuf = WPA_ExtractSuiteFromRSNIE(assoc_ie, wpa_ie_len + 2, RSN_CAP_INFO, &count);
						os_alloc_mem(pAd, &wpa_ie, (wpa_ie_len + 2 + 22));
						if (wpa_ie && pBuf) {
							UINT16	pmk_count = 1;
							UCHAR *pos;

							wpa_ie_len = (pBuf - assoc_ie);
							os_move_mem(wpa_ie, assoc_ie, wpa_ie_len + sizeof(RSN_CAPABILITIES));
							wpa_ie_len += sizeof(RSN_CAPABILITIES);
							pos = wpa_ie + wpa_ie_len;
							if (psaved_pmk_lock)
								NdisAcquireSpinLock(psaved_pmk_lock);

							/*Update the pentry->pmkcache from the Saved PMK cache */
							pentry->SecConfig.pmkid = psaved_pmk[idx].PMKID;
							pentry->SecConfig.pmk_cache = psaved_pmk[idx].PMK;

							if (psaved_pmk_lock)
								NdisReleaseSpinLock(psaved_pmk_lock);

							pmk_count = cpu2le16(pmk_count);
							NdisMoveMemory(pos, &pmk_count, sizeof(pmk_count));
							pos += sizeof(pmk_count);
							NdisMoveMemory(pos, pentry->SecConfig.pmkid, LEN_PMKID);
							pos += LEN_PMKID;
							wpa_ie_len += (2 + LEN_PMKID);
							pBuf = WPA_ExtractSuiteFromRSNIE(assoc_ie, (*(assoc_ie + 1) + 2), G_MGMT_SUITE, &count);
							if (pBuf) {
								NdisMoveMemory(pos, pBuf, 4);
								wpa_ie_len += 4;
							}
							wpa_ie[1] = (wpa_ie_len - 2);
						}
					} else {
						pentry->SecConfig.pmkid = NULL;
						pentry->SecConfig.pmk_cache = NULL;
					}
				}

				if (wpa_ie) {
					UINT8 ie_len = *(assoc_ie + 1) + 2;
					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
								wpa_ie_len, wpa_ie, END_OF_ARGS);
					FrameLen += TmpWpaAssocIeLen;

					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
								(assoc_ie_len - ie_len), (assoc_ie + ie_len),
								END_OF_ARGS);
					FrameLen += TmpWpaAssocIeLen;
					/*VarIesOffset = 0;*/
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
								wpa_ie, wpa_ie_len);
					VarIesOffset += wpa_ie_len;
					/*VarIesOffset = 0;*/
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
								(assoc_ie + ie_len), (assoc_ie_len - ie_len));
					VarIesOffset += (assoc_ie_len - ie_len);
					/* Set Variable IEs Length */
					pStaCfg->ReqVarIELen = VarIesOffset;
					os_free_mem(wpa_ie);
				} else {
#endif
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
							pStaCfg->wpa_supplicant_info.WpaAssocIeLen, pStaCfg->wpa_supplicant_info.pWpaAssocIe,
							END_OF_ARGS);
			FrameLen += TmpWpaAssocIeLen;
			/*VarIesOffset = 0;*/
			NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset,
							pStaCfg->wpa_supplicant_info.pWpaAssocIe, pStaCfg->wpa_supplicant_info.WpaAssocIeLen);
			VarIesOffset += pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
			/* Set Variable IEs Length */
			pStaCfg->ReqVarIELen = VarIesOffset;
#if defined(SUPP_SAE_SUPPORT) || defined(SUPP_OWE_SUPPORT)
}
#endif
		} else
#endif /* APCLI_CFG80211_SUPPORT */

		/* Append RSN_IE when WPAPSK OR WPA2PSK, */
		if ((IS_AKM_PSK(pAPEntry->SecConfig.AKMMap)
#ifdef WPA_SUPPLICANT_SUPPORT
			 || IS_AKM_WPA2(pAPEntry->SecConfig.AKMMap)
#endif /* WPA_SUPPLICANT_SUPPORT */
			)
#ifdef WSC_STA_SUPPORT
			&& ((wdev->WscControl.WscConfMode == WSC_DISABLE) ||
				((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				 !(wdev->WscControl.bWscTrigger)))
#endif /* WSC_STA_SUPPORT */
		   ) {
#ifdef WPA_SUPPLICANT_SUPPORT

			if (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == FALSE) {
				if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
					; /*do nothing, RSNE content should be from wpa_supplicant */
				} else

					/* Check for WPA PMK cache list */
					if (IS_AKM_WPA2(pAPEntry->SecConfig.AKMMap)
						|| IS_AKM_WPA3_192BIT(pAPEntry->SecConfig.AKMMap)
						|| IS_AKM_WPA3PSK(pAPEntry->SecConfig.AKMMap)
						|| IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
						INT idx;
						BOOLEAN FoundPMK = FALSE;

						/* Search chched PMKID, append it if existed */
						for (idx = 0; idx < PMKID_NO; idx++) {
							if (NdisEqualMemory(ApAddr, &pStaCfg->SavedPMK[idx].BSSID, 6)) {
								FoundPMK = TRUE;
								break;
							}
						}

						/*
						   When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
						   AP would not do PMK cache with STA after STA re-connect to AP again.
						   In this case, driver doesn't need to send PMKID to AP and WpaSupplicant.

						 */ /* TODO: pAd->CommonCfg.LastBssid */
						if (NdisEqualMemory(pStaCfg->MlmeAux.Bssid, pStaCfg->LastBssid, MAC_ADDR_LEN))
							FoundPMK = FALSE;

						if (FoundPMK)
							store_pmkid_cache_in_sec_config(pAd, pAPEntry, idx);
					}
			}
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(DPP_SUPPORT)
			if (IS_AKM_WPA3PSK(pStaCfg->wdev.SecConfig.AKMMap) ||
#ifdef DPP_SUPPORT
				IS_AKM_DPP(pStaCfg->wdev.SecConfig.AKMMap) ||
#endif /* DPP_SUPPORT */
				IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap)) {
				INT idx;
				MAC_TABLE_ENTRY *pentry = (MAC_TABLE_ENTRY *)NULL;
				PBSSID_INFO psaved_pmk = NULL;
				VOID *psaved_pmk_lock = NULL;
				UINT32 sec_akm = 0;

				if (IS_AKM_SAE_SHA256(pStaCfg->wdev.SecConfig.AKMMap))
					SET_AKM_SAE_SHA256(sec_akm);
				else if (IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap))
					SET_AKM_OWE(sec_akm);

#ifdef DPP_SUPPORT
				if (IS_AKM_DPP(pStaCfg->wdev.SecConfig.AKMMap)) {
					sec_akm = 0;
					SET_AKM_DPP(sec_akm);
				}
#endif


#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn) && (wdev->wdev_type == WDEV_TYPE_REPEATER)) {
					pentry = rept->pMacEntry;
					psaved_pmk = &(rept->SavedPMK[0]);
					psaved_pmk_lock = (void *)&(rept->SavedPMK_lock);
				}
				 else
#endif /* MAC_REPEATER_SUPPORT */
				{
					pentry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];
					psaved_pmk = &pStaCfg->SavedPMK[0];
					psaved_pmk_lock = (VOID *)&pStaCfg->SavedPMK_lock;
				}

				idx = sta_search_pmkid_cache(pAd, ApAddr, ifIndex, wdev,
					sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);

				if (idx != INVALID_PMKID_IDX) {
					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					/*Update the pentry->pmkcache from the Saved PMK cache */
					pentry->SecConfig.pmkid = psaved_pmk[idx].PMKID;
					pentry->SecConfig.pmk_cache = psaved_pmk[idx].PMK;

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				} else {
					printk("PMKID not found in cache: Normal Assoc\n");
					pentry->SecConfig.pmkid = NULL;
					pentry->SecConfig.pmk_cache = NULL;
				}
			}
#endif

#ifdef WPA_SUPPLICANT_SUPPORT
			/*
				Can not use SIOCSIWGENIE definition, it is used in wireless.h
				We will not see the definition in MODULE.
				The definition can be saw in UTIL and NETIF.
			*/
			if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE)
				&& (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == TRUE))
				;
			else
#endif /* WPA_SUPPLICANT_SUPPORT */
			{
				ULONG TempLen = 0;
				CHAR rsne_idx = 0;
				{ /* Todo by Eddy: It's not good code */
					struct _SECURITY_CONFIG *pSecConfig = &pAPEntry->SecConfig;
					UINT32 AKMMap = pSecConfig->AKMMap;
					UINT32 PairwiseCipher = pSecConfig->PairwiseCipher;

					UINT32 GroupCipher = pSecConfig->GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
					/* Need to fill the pSecConfig->PmfCfg to let WPAMakeRSNIE() generate correct RSNIE*/
					{
						RSN_CAPABILITIES RsnCap;

						NdisMoveMemory(&RsnCap, &pStaCfg->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
						RsnCap.word = cpu2le16(RsnCap.word);
						/* init to FALSE */
						pSecConfig->PmfCfg.UsePMFConnect = FALSE;
						pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;

						/*mismatch case*/
						if (((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
							|| ((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
							pSecConfig->PmfCfg.UsePMFConnect = FALSE;
							pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
						}

						if ((pSecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
							pSecConfig->PmfCfg.UsePMFConnect = TRUE;

							if ((pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
								pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
						}
					}
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SUPPORT
					/* ocv_support set to true temporary so that ocvc
					* bit can be set in assoc req. It will be finally set
					* when AP also supports OCV */
					if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
						pSecConfig->ocv_support = TRUE;
#endif
					WPAMakeRSNIE(pStaCfg->wdev.wdev_type, pSecConfig, pAPEntry);
#ifdef APCLI_SUPPORT
					if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
						pSecConfig->ocv_support = FALSE;
#endif
					pStaCfg->AKMMap = AKMMap;
					pStaCfg->PairwiseCipher = PairwiseCipher;
					pStaCfg->GroupCipher = GroupCipher;
				}

				for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
					if (pAPEntry->SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
						continue;

					MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
									  1, &pAPEntry->SecConfig.RSNE_EID[rsne_idx][0],
									  1, &pAPEntry->SecConfig.RSNE_Len[rsne_idx],
									  pAPEntry->SecConfig.RSNE_Len[rsne_idx], &pAPEntry->SecConfig.RSNE_Content[rsne_idx][0],
									  END_OF_ARGS);
					FrameLen += TempLen;
				}
			}

#ifdef WPA_SUPPLICANT_SUPPORT

			/*
				Can not use SIOCSIWGENIE definition, it is used in wireless.h
				We will not see the definition in MODULE.
				The definition can be saw in UTIL and NETIF.
			*/
			if (((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) !=
				 WPA_SUPPLICANT_ENABLE)
				|| (pStaCfg->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant == FALSE))
#endif /* WPA_SUPPLICANT_SUPPORT */
			{
				CHAR rsne_idx = 0;

				for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
					if (pStaCfg->wdev.SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
						continue;

					/* Append Variable IE */
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_EID[rsne_idx][0], 1);
					VarIesOffset += 1;
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx], 1);
					VarIesOffset += 1;
					NdisMoveMemory(pStaCfg->ReqVarIEs + VarIesOffset, &pStaCfg->wdev.SecConfig.RSNE_Content[rsne_idx][0],
								   pStaCfg->wdev.SecConfig.RSNE_Len[rsne_idx]);
					VarIesOffset += pStaCfg->RSNIE_Len;
					/* Set Variable IEs Length */
					pStaCfg->ReqVarIELen = VarIesOffset;
					break;
				}
			}
		}
		}
		/* RSN end */

		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen +=  build_extra_ie(pAd, &ie_info);
#ifdef WSC_INCLUDED
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
		/* ADD TV IE to this packet */
		MakeTVMIE(pAd, wdev, pOutBuffer, &FrameLen);
#endif /* IGMP_TVM_SUPPORT */


#ifdef CONFIG_OWE_SUPPORT
		if (IS_AKM_OWE(pAPEntry->SecConfig.AKMMap)) {
			OWE_INFO *owe = &pAPEntry->SecConfig.owe;
			UCHAR group = pStaCfg->curr_owe_group;
			owe->last_try_group = group;
			if (init_owe_group(owe, group) == 0) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
					"init_owe_group failed. shall not happen!\n");
				MlmeFreeMemory(pOutBuffer);
				return;
			}

			FrameLen +=  build_owe_dh_ie(pAd, pAPEntry, (UCHAR *)(pOutBuffer + FrameLen), group);
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		}
#endif /*CONFIG_OWE_SUPPORT*/


		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set assoc Timeout(%ld)ms\n", Timeout);
		RTMPSetTimer(assoc_timer, Timeout);
		assoc_fsm_state_transition(wdev, ASSOC_WAIT_RSP);
	}
}

/*
	==========================================================================
	Description:
		mlme reassoc req handling procedure
	Parameters:
		Elem -
	Pre:
		-# SSID  (Adapter->StaCfg[0].ssid[])
		-# BSSID (AP address, Adapter->StaCfg[0].bssid)
		-# Supported rates (Adapter->StaCfg[0].supported_rates[])
		-# Supported rates length (Adapter->StaCfg[0].supported_rates_len)
		-# Tx power (Adapter->StaCfg[0].tx_power)

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_reassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR ApAddr[6];
	HEADER_802_11 ReassocHdr;
	USHORT CapabilityInfo, ListenIntv;
	ULONG Timeout;
	ULONG FrameLen = 0;
	BOOLEAN TimerCancelled;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	USHORT Status;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct legacy_rate *rate = &pStaCfg->MlmeAux.rate;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	assoc_fsm_state_transition(wdev, ASSOC_IDLE);

	if (sta_block_checker(Elem->wdev) == TRUE)
		return;

	/* the parameters are the same as the association */
	if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - MlmeReassocReqAction() sanity check failed. BUG!!!!\n");
		Status = MLME_INVALID_FORMAT;
		cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status);
	}

	{
		struct _build_ie_info ie_info = {0};

		ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
		ie_info.channel = pStaCfg->MlmeAux.Channel;
		ie_info.phy_mode = wdev->PhyMode;
		ie_info.wdev = wdev;
		/*for dhcp,issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key  */
		RTMPWPARemoveAllKeys(pAd, wdev);
		RTMPCancelTimer(&pStaCfg->MlmeAux.ReassocTimer, &TimerCancelled);
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ASSOC - MlmeReassocReqAction() allocate memory failed\n");
			Status = MLME_FAIL_NO_RESOURCE;
			cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status);
			return;
		}
#ifdef DOT11K_RRM_SUPPORT
		if ((IS_RRM_ENABLE(wdev)))
			CapabilityInfo |= RRM_CAP_BIT;
#endif

		/* make frame, use bssid as the AP address?? */
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - Send RE-ASSOC request...\n");

		MgtMacHeaderInitExt(pAd, &ReassocHdr, SUBTYPE_REASSOC_REQ, 0, ApAddr,
							pStaCfg->wdev.if_addr, ApAddr);
		MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11),
						  &ReassocHdr, 2, &CapabilityInfo, 2,
						  &ListenIntv, MAC_ADDR_LEN, pStaCfg->LastBssid, 1, &SsidIe,
						  1, &pStaCfg->MlmeAux.SsidLen,
						  pStaCfg->MlmeAux.SsidLen, pStaCfg->MlmeAux.Ssid,
						  END_OF_ARGS);

		FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, pOutBuffer + FrameLen);

		FrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
					rate->ext_rate, rate->ext_rate_len, pOutBuffer + FrameLen);
		{
			/* Insert RSNIE */
			MAC_TABLE_ENTRY *pAPEntry = GetAssociatedAPByWdev(pAd, Elem->wdev);
			UCHAR rsne_idx = 0;
			ULONG TempLen = 0;
			struct _SECURITY_CONFIG *pSecConfig = &pAPEntry->SecConfig;
			UINT32 AKMMap = pSecConfig->AKMMap;
			UINT32 PairwiseCipher = pSecConfig->PairwiseCipher;
			UINT32 GroupCipher = pSecConfig->GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
			/* Need to fill the pSecConfig->PmfCfg to let WPAMakeRSNIE() generate correct RSNIE*/
			{
				RSN_CAPABILITIES RsnCap;

				NdisMoveMemory(&RsnCap, &pStaCfg->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
				RsnCap.word = cpu2le16(RsnCap.word);
				/* init to FALSE */
				pSecConfig->PmfCfg.UsePMFConnect = FALSE;
				pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;

				/*mismatch case*/
				if (((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
						|| ((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
					pSecConfig->PmfCfg.UsePMFConnect = FALSE;
					pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
				}
				if ((pSecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
					pSecConfig->PmfCfg.UsePMFConnect = TRUE;

					if ((pStaCfg->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
						pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
				}
			}
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SUPPORT
			/* ocv_support set to true temporary so that ocvc
			* bit can be set in assoc req. It will be finally set
			* when AP also supports OCV */
			if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
				pSecConfig->ocv_support = TRUE;
#endif
			WPAMakeRSNIE(pStaCfg->wdev.wdev_type, pSecConfig, pAPEntry);
#ifdef APCLI_SUPPORT
			if (pStaCfg->wdev.SecConfig.apcli_ocv_support)
				pSecConfig->ocv_support = FALSE;
#endif
			pStaCfg->AKMMap = AKMMap;
			pStaCfg->PairwiseCipher = PairwiseCipher;
			pStaCfg->GroupCipher = GroupCipher;
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				if (pAPEntry->SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
						1, &pAPEntry->SecConfig.RSNE_EID[rsne_idx][0],
						1, &pAPEntry->SecConfig.RSNE_Len[rsne_idx],
						pAPEntry->SecConfig.RSNE_Len[rsne_idx], &pAPEntry->SecConfig.RSNE_Content[rsne_idx][0],
						END_OF_ARGS);
				FrameLen += TempLen;
			}
		}

#ifdef MBO_SUPPORT
		/* Insert Supported Operating Class IE */
		if (IS_MBO_ENABLE(wdev))
			FrameLen += build_supp_op_class_ie(pAd, wdev, pOutBuffer + FrameLen);
		/* Insert MBO_OCE IE */
		if (IS_MBO_ENABLE(wdev))
			MakeMboOceIE(pAd, wdev, NULL, pOutBuffer + FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_REQ);
#endif /* MBO_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
		/* Insert RRMEnable IE */
		if (IS_RRM_ENABLE(wdev))
			RRM_InsertRRMEnCapIE(pAd, wdev, pOutBuffer + FrameLen, &FrameLen, wdev->func_idx);
#endif
#ifdef DOT11R_FT_SUPPORT

		/* Add MDIE if we are connection to DOT11R AP */
		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->MlmeAux.MdIeInfo.Len) {
			PUINT8 mdie_ptr;
			UINT mdie_len = 0;
			/* MDIE */
			mdie_ptr = pOutBuffer + FrameLen;
			mdie_len = 5;
			FT_InsertMdIE(pOutBuffer + FrameLen, &FrameLen,
						  pStaCfg->MlmeAux.MdIeInfo.MdId,
						  pStaCfg->MlmeAux.MdIeInfo.FtCapPlc);

			/* Indicate the FT procedure */
			if (pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
				!IS_CIPHER_NONE(pStaCfg->wdev.SecConfig.PairwiseCipher)) {
				UINT8 FtIeLen = 0;
				PMAC_TABLE_ENTRY pEntry;
				FT_MIC_CTR_FIELD mic_ctr;
				PUINT8 rsnie_ptr;
				UINT rsnie_len = 0;
				PUINT8 ftie_ptr;
				UINT ftie_len = 0;
				UINT8 ft_mic[16];
				UCHAR rsnxe_ie[MAX_LEN_OF_RSNXEIE];
				UCHAR rsnxe_ie_len;

#ifdef HOSTAPD_WPA3R3_SUPPORT
				rsnxe_ie_len = build_rsnxe_ie(wdev, &wdev->SecConfig, rsnxe_ie);
#else
				rsnxe_ie_len = build_rsnxe_ie(&wdev->SecConfig, rsnxe_ie);
#endif

				pEntry = &pAd->MacTab.Content[MCAST_WCID];
				/* Insert RSNIE[PMK-R1-NAME] */
				rsnie_ptr = pOutBuffer + FrameLen;
				rsnie_len = 2 + pStaCfg->RSNIE_Len + 2 + LEN_PMK_NAME;
				WPAInsertRSNIE(pOutBuffer + FrameLen,
								&FrameLen,
								pStaCfg->RSN_IE,
								pStaCfg->RSNIE_Len,
								pEntry->FT_PMK_R1_NAME,
								LEN_PMK_NAME);
				/* Insert FTIE[MIC, ANONCE, SNONCE, R1KH-ID, R0KH-ID] */
				FtIeLen = sizeof(FT_FTIE) +
						  (2 + MAC_ADDR_LEN) +
						  (2 + pStaCfg->Dot11RCommInfo.R0khIdLen);
				ftie_ptr = pOutBuffer + FrameLen;
				ftie_len = (2 + FtIeLen);
				mic_ctr.field.IECnt = (rsnxe_ie_len != 0) ? 4 : 3;
				NdisZeroMemory(ft_mic, 16);
				FT_InsertFTIE(pOutBuffer + FrameLen,
							  &FrameLen,
							  FtIeLen,
							  mic_ctr,
							  ft_mic,
							  pEntry->FtIeInfo.ANonce, pEntry->FtIeInfo.SNonce);
				FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
										&FrameLen,
										FT_R1KH_ID,
										pStaCfg->MlmeAux.Bssid,
										MAC_ADDR_LEN);
				FT_FTIE_InsertKhIdSubIE(pOutBuffer + FrameLen,
										&FrameLen,
										FT_R0KH_ID,
										&pStaCfg->Dot11RCommInfo.R0khId[0],
										pStaCfg->Dot11RCommInfo.R0khIdLen);

				/* RIC-Request */
				if (pStaCfg->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap
					&& pStaCfg->Dot11RCommInfo.bSupportResource) {
				}

				/* Calculate MIC */
				if (mic_ctr.field.IECnt) {
					UINT8 ft_mic[16];
					PFT_FTIE pFtIe;

					FT_CalculateMIC(pAd->CurrentAddress,
									pStaCfg->MlmeAux.Bssid,
									pEntry->SecConfig.PTK,
									5,
									rsnie_ptr,
									rsnie_len,
									mdie_ptr,
									mdie_len,
									ftie_ptr,
									ftie_len,
									NULL,
									0,
									rsnxe_ie,
									rsnxe_ie_len,
									ft_mic);
					/* Update the MIC field of FTIE */
					pFtIe = (PFT_FTIE) (ftie_ptr + 2);
					NdisMoveMemory(pFtIe->MIC, ft_mic, 16);
				}
			}
		}

#endif /* DOT11R_FT_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
#ifdef DOT11_N_SUPPORT

		/* HT */
		if (HAS_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)
			&& WMODE_CAP_N(wdev->PhyMode)) {
			ULONG TmpLen;
			UCHAR ht_cap_len = SIZE_HT_CAP_IE;
			UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
			PHT_CAPABILITY_IE pHtCapability;
#ifdef RT_BIG_ENDIAN
			HT_CAPABILITY_IE HtCapabilityTmp;

			NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(&HtCapabilityTmp, &pStaCfg->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
			*(USHORT *) (&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *) (&HtCapabilityTmp.HtCapInfo));
			*(USHORT *) (&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *) (&HtCapabilityTmp.ExtHtCapInfo));
			pHtCapability = &HtCapabilityTmp;
#else
			pHtCapability = &pStaCfg->MlmeAux.HtCapability;
#endif

			if (pStaCfg->StaActive.SupportedPhyInfo.bPreNHt == TRUE) {
				ht_cap_len = SIZE_HT_CAP_IE + 4;
				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &TmpLen, 1, &WpaIe, 1, &ht_cap_len,
								  4, &BROADCOM[0],
								  SIZE_HT_CAP_IE,
								  pHtCapability, END_OF_ARGS);
			} else {
				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &TmpLen, 1, &HtCapIe, 1,
								  &ht_cap_len,
								  SIZE_HT_CAP_IE,
								  pHtCapability, END_OF_ARGS);
			}

			FrameLen += TmpLen;
#ifdef DOT11_VHT_AC
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
			FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef HOSTAPD_WPA3R3_SUPPORT
		FrameLen += build_rsnxe_ie(wdev, &wdev->SecConfig, pOutBuffer + FrameLen);
#else
		FrameLen += build_rsnxe_ie(&wdev->SecConfig, pOutBuffer + FrameLen);
#endif
		/* add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION */
		/* Case I: (Aggregation + Piggy-Back) */
		/* 1. user enable aggregation, AND */
		/* 2. Mac support piggy-back */
		/* 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON */
		/* Case II: (Aggregation) */
		/* 1. user enable aggregation, AND */
		/* 2. AP annouces it's AGGREGATION-capable in BEACON */
		FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_REQ);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		RTMPSetTimer(&pStaCfg->MlmeAux.ReassocTimer, Timeout * 2);	/* in mSec */
		assoc_fsm_state_transition(Elem->wdev, REASSOC_WAIT_RSP);
	}
}

/*
	==========================================================================
	Description:
		Upper layer issues disassoc request
	Parameters:
		Elem -

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
static VOID sta_mlme_disassoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_DISCONNECT_STRUCT *pDisassocReq; /* snowpin for cntl mgmt */
	HEADER_802_11 DisassocHdr;
	PHEADER_802_11 pDisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	BOOLEAN TimerCancelled;
	ULONG Timeout = 500;
	USHORT Status = MLME_SUCCESS;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
	RALINK_TIMER_STRUCT *disassoc_timer = NULL;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		ASSERT(rept);
		disassoc_timer = &rept->ApCliAssocTimer;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		disassoc_timer = &pStaCfg->MlmeAux.DisassocTimer;
	}

#ifdef DOT11Z_TDLS_SUPPORT

	if (IS_TDLS_SUPPORT(pAd)) {
		if (pStaCfg->bRadio == TRUE)
			TDLS_LinkTearDown(pAd, TRUE);
		else {
			UCHAR		idx;
			BOOLEAN		TimerCancelled;
			PRT_802_11_TDLS	pTDLS = NULL;

			/* tear down tdls table entry */
			for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++) {
				pTDLS = &pStaCfg->TdlsInfo.TDLSEntry[idx];

				if (pTDLS->Valid && (pTDLS->Status >= TDLS_MODE_CONNECTED)) {
					pTDLS->Status = TDLS_MODE_NONE;
					pTDLS->Valid	= FALSE;
					pTDLS->Token = 0;
					RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

					if (!IS_WCID_VALID(pAd, pTDLS->MacTabMatchWCID))
						return;

					MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
				} else if (pTDLS->Valid) {
					pTDLS->Status = TDLS_MODE_NONE;
					pTDLS->Valid	= FALSE;
					pTDLS->Token = 0;
					RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
				}
			}
		}
	}

#endif /* DOT11Z_TDLS_SUPPORT */
	/* skip sanity check */
	pDisassocReq = (MLME_DISCONNECT_STRUCT *) (Elem->Msg); /* snowpin for cntl mgmt */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "ASSOC - MlmeDisassocReqAction() allocate memory failed\n");
		Status = MLME_FAIL_NO_RESOURCE;
		goto SEND_EVENT_TO_CNTL;
	}

	RTMPCancelTimer(&pStaCfg->MlmeAux.DisassocTimer, &TimerCancelled);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "ASSOC - Send DISASSOC request[BSSID::"MACSTR" (Reason=%d)\n",
			  MAC2STR(pDisassocReq->addr), pDisassocReq->reason); /* snowpin for cntl mgmt */
	MgtMacHeaderInitExt(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->addr, /* snowpin for cntl mgmt */
						wdev->if_addr,
						pDisassocReq->addr);	/* patch peap ttls switching issue */ /* snowpin for cntl mgmt */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2, &pDisassocReq->reason, END_OF_ARGS); /* snowpin for cntl mgmt */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	/* To patch Instance and Buffalo(N) AP */
	/* Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine */
	/* Therefore, we send both of them. */
	pDisassocHdr = (PHEADER_802_11) pOutBuffer;
	pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	pStaCfg->DisassocReason = REASON_DISASSOC_STA_LEAVING;
	COPY_MAC_ADDR(pStaCfg->DisassocSta, pDisassocReq->addr); /* snowpin for cntl mgmt */
#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type != WDEV_TYPE_REPEATER)
#endif /* MAC_REPEATER_SUPPORT */
	{
		RTMPSetTimer(&pStaCfg->MlmeAux.DisassocTimer, Timeout);	/* in mSec */
		assoc_fsm_state_transition(wdev, DISASSOC_WAIT_RSP);
	}

SEND_EVENT_TO_CNTL:
	/* linkdown should be done after DisAssoc frame is sent */
	cntl_auth_assoc_conf(wdev, CNTL_MLME_DISASSOC_CONF, Status);
#ifdef WPA_SUPPLICANT_SUPPORT

	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
		/*send disassociate event to wpa_supplicant */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
								RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}

#endif /* WPA_SUPPLICANT_SUPPORT */
	RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);
}

/*
	==========================================================================
	Description:
		peer sends assoc rsp back
	Parameters:
		Elme - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_assoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT CapabilityInfo, Status, Aid = 0;
	UCHAR Addr2[MAC_ADDR_LEN];
	BOOLEAN TimerCancelled;
	UCHAR CkipFlag;
	EDCA_PARM EdcaParm;
	UCHAR NewExtChannelOffset = 0xff;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	MAC_TABLE_ENTRY *pEntry;
	IE_LISTS *ie_list = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	struct wifi_dev *wdev = Elem->wdev;
#if defined(DOT11_SAE_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	USHORT ifIndex = wdev->func_idx;
#endif
	struct legacy_rate *rate;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "mem alloc failed!\n");
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));
	os_zero_mem(&EdcaParm, sizeof(EDCA_PARM));

	if (PeerAssocRspSanity(&pStaCfg->wdev, Elem->Msg, Elem->MsgLen,
						   Addr2, &CapabilityInfo, &Status, &Aid,
						   &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
						   &CkipFlag, ie_list)) {
		/* The frame is for me ? */
		if (MAC_ADDR_EQUAL(Addr2, pStaCfg->MlmeAux.Bssid)) {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
			PFRAME_802_11 pFrame =	(PFRAME_802_11) (Elem->Msg);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
					 "ASSOC - receive ASSOC_RSP to me (status=%d)\n", Status);
			/* Store the AssocRsp Frame to wpa_supplicant via CFG80211 */
			NdisZeroMemory(pAd->StaCfg[ifIndex].ResVarIEs, MAX_VIE_LEN);
			pAd->StaCfg[ifIndex].ResVarIELen = 0;
			pAd->StaCfg[ifIndex].ResVarIELen = Elem->MsgLen - 6 - sizeof(HEADER_802_11);
			NdisCopyMemory(pAd->StaCfg[ifIndex].ResVarIEs, &pFrame->Octet[6], pAd->StaCfg[ifIndex].ResVarIELen);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef MAC_REPEATER_SUPPORT

			if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
				rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
				RTMPCancelTimer(&rept->ApCliAssocTimer, &TimerCancelled);
			} else
#endif /* MAC_REPEATER_SUPPORT */
				RTMPCancelTimer(&pStaCfg->MlmeAux.AssocTimer, &TimerCancelled);

#ifdef DOT11R_FT_SUPPORT

			if (pStaCfg->Dot11RCommInfo.bFtSupport && pStaCfg->MlmeAux.FtIeInfo.Len) {
				FT_FTIE_INFO *pFtInfo = &pStaCfg->MlmeAux.FtIeInfo;

				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - FTIE\n");
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "MIC Countrol IECnt: %x\n", pFtInfo->MICCtr.field.IECnt);
				hex_dump("ANonce", pFtInfo->ANonce, 32);
				hex_dump("SNonce", pFtInfo->SNonce, 32);

				if (pFtInfo->R1khIdLen)
					hex_dump("R1KH-ID", pFtInfo->R1khId, pFtInfo->R1khIdLen);

				if (pFtInfo->R0khIdLen)
					hex_dump("R0KH-ID", pFtInfo->R0khId, pFtInfo->R0khIdLen);

				if ((pStaCfg->Dot11RCommInfo.R0khIdLen != pFtInfo->R0khIdLen)
					|| (!NdisEqualMemory(pFtInfo->R0khId, pFtInfo->R0khId,
										 pStaCfg->Dot11RCommInfo.R0khIdLen))) {
					if (pStaCfg->Dot11RCommInfo.bInMobilityDomain)
						Status = MLME_INVALID_FORMAT;
				}
			}

#endif /* DOT11R_FT_SUPPORT */


			if (Status == MLME_SUCCESS) {
				UCHAR MaxSupportedRateIn500Kbps = 0;
				UCHAR op_mode = OPMODE_AP;

				rate = &ie_list->cmm_ies.rate;
				MaxSupportedRateIn500Kbps = dot11_max_sup_rate(rate);

				pEntry = MacTableLookup2(pAd, Addr2, wdev);

				ASSERT(pEntry);

				if (!pEntry) {
					if (ie_list != NULL)
						os_free_mem(ie_list);
					return;
				}

				set_mlme_rsn_ie(pAd, wdev, pEntry);
#ifdef APCLI_SUPPORT
				if (pStaCfg->wdev.SecConfig.apcli_ocv_support) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
					DBG_LVL_INFO, "ApCli OCV: pEntry->RSNIE_Len:%d\n", pEntry->RSNIE_Len);
					if (pEntry->RSNIE_Len) {
						pStaCfg->wdev.SecConfig.ocv_support = TRUE;
						if (wpa_check_rsn_cap(&pStaCfg->wdev.SecConfig,
									&pEntry->SecConfig,
									pEntry->RSN_IE,
									pEntry->RSNIE_Len)) {
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_INFO, "ApCli OCV: wpa_check_rsn_cap return TRUE\n");
						} else {
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_WARN, "ApCli OCV: wpa_check_rsn_cap return FALSE\n");
						}
						if (pEntry->SecConfig.ocv_support == FALSE)
							pStaCfg->wdev.SecConfig.ocv_support = FALSE;

						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
						DBG_LVL_INFO, "ApCli OCV: Self.ocv_support:%d pEntry.ocv_support:%d\n",
						pStaCfg->wdev.SecConfig.ocv_support, pEntry->SecConfig.ocv_support);
					}
				}
#endif
				if (IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) {
					op_mode = OPMODE_AP;
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_ENABLE(pAd)) {
						pEntry->DevPeerRole = ie_list->MAP_AttriValue;
#ifdef MAP_R2
						pEntry->profile = ie_list->MAP_ProfileValue;
						if (IS_VALID_VID(ie_list->MAP_default_vid)) {
							pStaCfg->wdev.MAPCfg.primary_vid = ie_list->MAP_default_vid;
						}
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_INFO, "pEntry=%p, profile=%02x, ie_vid:%d\n",
							pEntry, pEntry->profile, ie_list->MAP_default_vid);
#endif
					}
#endif /* CONFIG_MAP_SUPPORT */
						ApCliAssocPostProc(pAd, wdev, Addr2, CapabilityInfo,
										   &EdcaParm, ie_list, pEntry);
						pStaCfg->MlmeAux.Aid = Aid;
#ifdef CONFIG_OWE_SUPPORT
						if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
							BOOLEAN need_process_ecdh_ie = FALSE;
							UINT8 *pmkid = NULL;
							UINT8 pmkid_count = 0;
							UINT32 sec_akm = 0;
							INT idx;

							SET_AKM_OWE(sec_akm);

							pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
											ie_list->RSNIE_Len,
											PMKID_LIST,
											&pmkid_count);
								if (pmkid != NULL) {
									BOOLEAN FoundPMK = FALSE;

									/*  Search cached PMKID, append it if existed */
									idx = sta_search_pmkid_cache(pAd, Addr2, wdev->func_idx, wdev,
									sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);

									if (idx != INVALID_PMKID_IDX)
										FoundPMK = TRUE;

									if (FoundPMK == FALSE) {
										need_process_ecdh_ie = TRUE;
										MTWF_DBG(pAd, DBG_CAT_AP,
														DBG_SUBCAT_ALL,
														DBG_LVL_ERROR,
														"cannot find match PMKID\n");
									} else if ((pEntry->SecConfig.pmkid) &&
												((RTMPEqualMemory(pmkid, pEntry->SecConfig.pmkid, LEN_PMKID)) != 0)) {
											/*
											 * if STA would like to use PMK CACHE,
											 * it stored the PMKID in assoc req stage already.
											 * no need to restore it again here.
											 */
											MTWF_DBG(pAd, DBG_CAT_AP,
															DBG_SUBCAT_ALL,
															DBG_LVL_ERROR,
															"PMKID doesn't match STA sent\n");
											need_process_ecdh_ie = TRUE;
									}
								} else
										need_process_ecdh_ie = TRUE;

								if (need_process_ecdh_ie == TRUE) {
									/*
									 *  Fix IOT issue with Maxlinear AP in HE-5.75.1_6G. Maxlinear
									 *	AP does't not carry pmkid in RSNIE in association response,
									 *	so delete pmkid_cache here to solve connecting issue with
									 *  Maxlinear AP.
									 */
									idx = sta_search_pmkid_cache(pAd, Addr2, wdev->func_idx, wdev,
										sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
									if (idx != INVALID_PMKID_IDX) {
										sta_delete_pmkid_cache(pAd, Addr2, wdev->func_idx, wdev,
											sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
									}

									MTWF_DBG(pAd, DBG_CAT_AP,
													DBG_SUBCAT_ALL,
													DBG_LVL_INFO,
													"%s: do normal ECDH procedure\n", __func__);
/* XXX: TRACE_NM: Should we process this during EAPOL-1_of_4 handling */
									process_ecdh_element(pAd,
													pEntry,
													(EXT_ECDH_PARAMETER_IE *)&ie_list->ecdh_ie,
													ie_list->ecdh_ie.length,
													SUBTYPE_ASSOC_RSP);
								}
						}
#endif /*CONFIG_OWE_SUPPORT*/
#ifdef DOT11_VHT_AC
						RTMPZeroMemory(&pStaCfg->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
						RTMPZeroMemory(&pStaCfg->MlmeAux.vht_op, sizeof(VHT_OP_IE));
						CLR_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
						CLR_VHT_OP_EXIST(pStaCfg->MlmeAux.ie_exists);

						if (WMODE_CAP_AC(pStaCfg->wdev.PhyMode)
								&& HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)
								&& HAS_VHT_OP_EXIST(ie_list->cmm_ies.ie_exists)) {
							NdisMoveMemory(&pStaCfg->MlmeAux.vht_cap,
									&(ie_list->cmm_ies.vht_cap),
									SIZE_OF_VHT_CAP_IE);
							SET_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
							NdisMoveMemory(&pStaCfg->MlmeAux.vht_op,
									&(ie_list->cmm_ies.vht_op),
									SIZE_OF_VHT_OP_IE);
							SET_VHT_OP_EXIST(pStaCfg->MlmeAux.ie_exists);
						}

#endif /* DOT11_VHT_AC */

					/* For Repeater get correct wmm valid setting */
					pStaCfg->MlmeAux.APEdcaParm.bValid = EdcaParm.bValid;
#ifdef APCLI_AS_WDS_STA_SUPPORT
					{
						PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pStaCfg->MacTabWCID];

						if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
								|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
								|| pEntry->wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
								|| pEntry->bWscCapable)) {
							pEntry->bEnable4Addr = TRUE;
							if (pStaCfg->wdev.wds_enable)
								HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pStaCfg->MacTabWCID, TRUE);
						}
					}
#endif /* APCLI_AS_WDS_STA_SUPPORT */

					/*
						In roaming case, LinkDown wouldn't be invoked.
						For preventing finding MacTable Hash index malfunction,
						we need to do MacTableDeleteEntry here.
					*/
				} else if (wdev->wdev_type == WDEV_TYPE_STA) {
					op_mode = OPMODE_STA;
					/* go to procedure listed on page 376 */
					sta_assoc_post_proc(pAd, Addr2, CapabilityInfo, Aid,
										&EdcaParm, ie_list, pEntry);
				}

				if (EdcaParm.bValid && wdev->bWmmCapable)
					CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
				else
					CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

				StaUpdateMacTableEntry(pAd,
									   pEntry,
									   MaxSupportedRateIn500Kbps,
									   ie_list,
									   CapabilityInfo);
					TRTableInsertEntry(pAd, pEntry->wcid, pEntry);

				/* TRTableEntryDump(pAd, pEntry->wcid, __FUNCTION__, __LINE__); */
				/* ---Add by shiang for debug */
				RTMPSetSupportMCS(pAd, op_mode, pEntry, rate,
#ifdef DOT11_VHT_AC
								  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
								  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
								  &ie_list->cmm_ies.ht_cap,
								  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));

#ifdef DOT11_HE_AX
				if (HAS_HE_OP_EXIST(ie_list->cmm_ies.ie_exists))
					parse_he_bss_color_info(wdev, ie_list);
				if (HAS_HE_MU_EDCA_EXIST(ie_list->cmm_ies.ie_exists))
					update_peer_he_muedca_ies(pEntry, &(ie_list->cmm_ies));
#endif

#ifdef APCLI_CFG80211_SUPPORT
			CFG80211_checkScanTable(pAd);
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, ifIndex,
					pStaCfg->ReqVarIEs, pStaCfg->ReqVarIELen,
					pStaCfg->ResVarIEs, pStaCfg->ResVarIELen, TRUE);
#endif
			} else {
#ifdef FAST_EAPOL_WAR
				ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
				/* TRACE_NM: Should we use pEntry ? */
				if ((IS_AKM_SAE(pStaCfg->AKMMap)) && (Status == MLME_INVALID_PMKID || Status == MLME_INVALID_INFORMATION_ELEMENT)) {
					UCHAR if_addr[6];
					INT CachedIdx;
					SAE_INSTANCE *pSaeIns = NULL;
					UINT32 sec_akm = 0;


					SET_AKM_SAE_SHA256(sec_akm);

#ifdef MAC_REPEATER_SUPPORT
					if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
						REPEATER_CLIENT_ENTRY *rept_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
						COPY_MAC_ADDR(if_addr, rept_entry->CurrentAddress);
					} else
#endif /* MAC_REPEATER_SUPPORT */
					NdisCopyMemory(if_addr, pStaCfg->wdev.if_addr, MAC_ADDR_LEN);

					CachedIdx = sta_search_pmkid_cache(pAd, Addr2, ifIndex, wdev,
						sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
					if (CachedIdx != INVALID_PMKID_IDX) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
										"Delete pmkid on assoc fail(incorrect pmkid)\n");
						sta_delete_pmkid_cache(pAd, Addr2, ifIndex, wdev,
							sec_akm, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
					}
					pSaeIns = search_sae_instance(&pAd->SaeCfg, if_addr, Addr2);
					if (pSaeIns != NULL) {
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
										"Delete Existing sae instance on assoc fail(incorrect pmkid)\n");
						delete_sae_instance(pSaeIns);
					}
				}
#endif
#ifdef APCLI_CFG80211_SUPPORT
					CFG80211_checkScanTable(pAd);
#ifdef SUPP_OWE_SUPPORT
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, ifIndex, NULL, 0, NULL, Status, 0);
#else
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, ifIndex, NULL, 0, NULL, 0, 0);
#endif
#endif
			}
			assoc_fsm_state_transition(Elem->wdev, ASSOC_IDLE);
			cntl_auth_assoc_conf(Elem->wdev,  CNTL_MLME_ASSOC_CONF, Status);

#ifdef LINUX
#ifndef APCLI_CFG80211_SUPPORT
#ifdef RT_CFG80211_SUPPORT

			if (Status == MLME_SUCCESS) {
				PFRAME_802_11 pFrame =  (PFRAME_802_11) (Elem->Msg);

				RTEnqueueInternalCmd(pAd, CMDTHREAD_CONNECT_RESULT_INFORM,
									 &pFrame->Octet[6], Elem->MsgLen - 6 - sizeof(HEADER_802_11));
			}

#endif /* RT_CFG80211_SUPPORT */
#endif
#endif /* LINUX */
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, "ASSOC - sanity check fail\n");

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list != NULL)
		os_free_mem(ie_list);
}


/*
	==========================================================================
	Description:
		peer sends reassoc rsp
	Parametrs:
		Elem - MLME message cntaining the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_reassoc_rsp_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT CapabilityInfo;
	USHORT Status;
	USHORT Aid = 0;
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR CkipFlag;
	BOOLEAN TimerCancelled;
	EDCA_PARM EdcaParm = {0};
	UCHAR NewExtChannelOffset = 0xff;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	IE_LISTS *ie_list = NULL;
	struct wifi_dev *wdev = Elem->wdev;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	struct legacy_rate *rate;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "mem alloc failed!\n");
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));

	if (PeerAssocRspSanity(&pStaCfg->wdev, Elem->Msg, Elem->MsgLen, Addr2,
						   &CapabilityInfo, &Status, &Aid,
						   &NewExtChannelOffset, &EdcaParm, &ExtCapInfo,
						   &CkipFlag, ie_list)) {
		if (MAC_ADDR_EQUAL(Addr2, pStaCfg->MlmeAux.Bssid)) {	/* The frame is for me ? */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "REASSOC - receive REASSOC_RSP to me (status=%d)\n", Status);
			RTMPCancelTimer(&pStaCfg->MlmeAux.ReassocTimer,
							&TimerCancelled);

			if (Status == MLME_SUCCESS) {
				UCHAR MaxSupportedRateIn500Kbps = 0;
				PMAC_TABLE_ENTRY pEntry = NULL;
				rate = &ie_list->cmm_ies.rate;
#ifdef MBO_SUPPORT
				if (IS_MBO_ENABLE(wdev)) {
					pEntry = MacTableLookup2(pAd, Addr2, wdev);
					ASSERT(pEntry);

					if (!pEntry) {
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
						/* fix memory leak when trigger scan continuously*/
						if (ie_list && ie_list->CustomerVendorIE.pointer)
							os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

						if (ie_list)
							os_free_mem(ie_list);
						return;
					}
				} else
#endif
				{
					/*
					   In roaming case, LinkDown wouldn't be invoked.
					   For preventing finding MacTable Hash index malfunction,
					   we need to do MacTableDeleteEntry here.
					 */
					pEntry = MacTableLookup2(pAd, pStaCfg->Bssid, wdev);

					if (pEntry) {
						MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
						pEntry = NULL;
					}

					pEntry = MacTableLookup2(pAd, Addr2, wdev);

					if (pEntry) {
						MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
						pEntry = NULL;
					}

					pEntry = MacTableInsertEntry(pAd, Addr2, wdev, ENTRY_INFRA, OPMODE_STA, TRUE);
					ASSERT(pEntry);
					if (!pEntry) {
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
						/* fix memory leak when trigger scan continuously*/
						if (ie_list && ie_list->CustomerVendorIE.pointer)
							os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

						if (ie_list)
							os_free_mem(ie_list);
						return;
					}
				}
				MaxSupportedRateIn500Kbps = dot11_max_sup_rate(rate);
#ifdef APCLI_SUPPORT
				if (pStaCfg->wdev.SecConfig.apcli_ocv_support) {
					UINT8   rsnie_len = ie_list->RSNIE_Len;
					PUINT8  rsnie_ptr = ie_list->RSN_IE;
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
					DBG_LVL_INFO, "ApCli OCV: pEntry->RSNIE_Len:%d rsnie_len:%d\n",
					pEntry->RSNIE_Len, rsnie_len);

					if (rsnie_len) {
						pStaCfg->wdev.SecConfig.ocv_support = TRUE;
						if (wpa_check_rsn_cap(&pStaCfg->wdev.SecConfig,
									&pEntry->SecConfig,
									rsnie_ptr,
									rsnie_len)) {
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_INFO, "ApCli OCV: wpa_check_rsn_cap return TRUE\n");
						} else {
							MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
							DBG_LVL_INFO, "ApCli OCV: wpa_check_rsn_cap return FALSE\n");
						}
						if (pEntry->SecConfig.ocv_support == FALSE)
							pStaCfg->wdev.SecConfig.ocv_support = FALSE;

						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI,
						DBG_LVL_INFO, "ApCli OCV: Self.ocv_support:%d pEntry.ocv_support:%d\n",
						pStaCfg->wdev.SecConfig.ocv_support, pEntry->SecConfig.ocv_support);
					}
				}
#endif
				/* go to procedure listed on page 376 */
				sta_assoc_post_proc(pAd, Addr2, CapabilityInfo, Aid,
									&EdcaParm, ie_list, pEntry);
				StaUpdateMacTableEntry(pAd,
									   pEntry,
									   MaxSupportedRateIn500Kbps,
									   ie_list,
									   CapabilityInfo);
				RTMPSetSupportMCS(pAd,
								  OPMODE_STA,
								  pEntry,
								  rate,
#ifdef DOT11_VHT_AC
								  HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists),
								  &ie_list->cmm_ies.vht_cap,
#endif /* DOT11_VHT_AC */
								  &ie_list->cmm_ies.ht_cap,
								  HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists));

#ifdef DOT11_HE_AX
				if (HAS_HE_OP_EXIST(ie_list->cmm_ies.ie_exists))
					parse_he_bss_color_info(wdev, ie_list);
				if (HAS_HE_MU_EDCA_EXIST(ie_list->cmm_ies.ie_exists))
					update_peer_he_muedca_ies(pEntry, &(ie_list->cmm_ies));
#endif

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

				if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
					SendAssocIEsToWpaSupplicant(pAd->net_dev,
												pStaCfg->ReqVarIEs,
												pStaCfg->ReqVarIELen);
					RtmpOSWrielessEventSend(pAd->net_dev,
											RT_WLAN_EVENT_CUSTOM,
											RT_ASSOC_EVENT_FLAG,
											NULL, NULL, 0);
				}

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
				{
					wext_notify_event_assoc(pAd->net_dev,
											pStaCfg->ReqVarIEs,
											pStaCfg->ReqVarIELen);
					RtmpOSWrielessEventSend(pAd->net_dev,
											RT_WLAN_EVENT_CGIWAP,
											-1,
											&pStaCfg->MlmeAux.Bssid[0], NULL,
											0);
				}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
			}

			/* CkipFlag is no use for reassociate */
			assoc_fsm_state_transition(wdev, ASSOC_IDLE);
			cntl_auth_assoc_conf(wdev, CNTL_MLME_REASSOC_CONF, Status);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "REASSOC - %s() sanity check fail\n", __func__);
	}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/* fix memory leak when trigger scan continuously*/
	if (ie_list && ie_list->CustomerVendorIE.pointer)
		os_free_mem(ie_list->CustomerVendorIE.pointer);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

	if (ie_list)
		os_free_mem(ie_list);
}


/*
	==========================================================================
	Description:
		left part of IEEE 802.11/1999 p.374
	Parameters:
		Elem - MLME message containing the received frame

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_peer_disassoc_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	PSTA_ADMIN_CONFIG pStaCfg;
	ULONG *pDisconnect_Sub_Reason = NULL;
#ifdef FAST_EAPOL_WAR
	USHORT ifIndex = wdev->func_idx;
#endif /* FAST_EAPOL_WAR */
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef WSC_STA_SUPPORT
	{
		struct wifi_dev *wdev = &pStaCfg->wdev;
		WSC_CTRL *wsc_ctrl = &wdev->WscControl;

		if (wsc_ctrl->WscState == WSC_STATE_WAIT_EAPFAIL) {
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&wsc_ctrl->WscEAPHandshakeCompleted,
								DISASSOC_WAIT_EAP_SUCCESS))
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:: Time out!!!\n", __func__);
		}
	}
#endif


#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		pDisconnect_Sub_Reason = &rept->Disconnect_Sub_Reason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pDisconnect_Sub_Reason = &pStaCfg->ApcliInfStat.Disconnect_Sub_Reason;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - PeerDisassocAction()\n");

	if (PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - PeerDisassocAction() Reason = %d\n",
				  Reason);
#ifdef FAST_EAPOL_WAR
		ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */

		if (INFRA_ON(pStaCfg)
			&& MAC_ADDR_EQUAL(pStaCfg->Bssid, Addr2)) {
			RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL,
								  BSS0, 0);
			/*
			   It is possible that AP sends dis-assoc frame(PeerDisassocAction) to STA
			   after driver enqueue ASSOC_FSM_MLME_DISASSOC_REQ (MlmeDisassocReqAction)
			   and set CntlMachine.CurrState = CNTL_WAIT_DISASSOC.
			   DisassocTimer is useless because AssocMachine.CurrState will set to ASSOC_IDLE here.
			   Therefore, we need to check CntlMachine.CurrState here and enqueue MT2_DISASSOC_CONF to
			   reset CntlMachine.CurrState to CNTL_IDLE state again.
			 */
			cntl_fsm_state_transition(wdev, CNTL_WAIT_DISASSOC, __func__);

			if (cntl_auth_assoc_conf(wdev, CNTL_MLME_DISASSOC_CONF, Reason) == FALSE) {
				UINT link_down_type = 0;

				link_down_type |= LINK_REQ_FROM_AP;
				LinkDown(pAd, link_down_type, wdev, Elem);
			}
			assoc_fsm_state_transition(Elem->wdev, ASSOC_IDLE);
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT

			if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
				/*send disassociate event to wpa_supplicant */
				RtmpOSWrielessEventSend(pAd->net_dev,
										RT_WLAN_EVENT_CUSTOM,
										RT_DISASSOC_EVENT_FLAG,
										NULL, NULL, 0);
			}

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "ASSOC - PeerDisassocAction() sanity check fail\n");
	}
}

/*
	==========================================================================
	Description:
		what the state machine will do after assoc timeout
	Parameters:
		Elme -

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_assoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	struct wifi_dev *wdev = Elem->wdev;
#ifdef FAST_EAPOL_WAR
	USHORT ifIndex = wdev->func_idx;
#endif /* FAST_EAPOL_WAR */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ASSOC - AssocTimeoutAction\n");
	/* rept use different timer assigned in InsertRepeaterEntry */
#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	assoc_fsm_state_transition(wdev, ASSOC_IDLE);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(wdev, CNTL_MLME_ASSOC_CONF, Status);
}

/*
	==========================================================================
	Description:
		what the state machine will do after reassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_reassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - ReassocTimeoutAction\n");
	assoc_fsm_state_transition(Elem->wdev, ASSOC_IDLE);
	Status = MLME_REJ_TIMEOUT;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_REASSOC_CONF, Status);
}

/*
	==========================================================================
	Description:
		what the state machine will do after disassoc timeout

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
static VOID sta_mlme_disassoc_req_timeout_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, Elem->wdev);
	/* rept doesn't have disassoc_req_timeout? */
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ASSOC - DisassocTimeoutAction\n");
	assoc_fsm_state_transition(Elem->wdev, ASSOC_IDLE);
	Status = MLME_SUCCESS;
	cntl_auth_assoc_conf(Elem->wdev, CNTL_MLME_DISASSOC_CONF, Status);
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
}

VOID sta_assoc_init(struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	sta_assoc_api.mlme_assoc_req_action = sta_mlme_assoc_req_action;
	sta_assoc_api.peer_assoc_rsp_action = sta_peer_assoc_rsp_action;
	sta_assoc_api.mlme_assoc_req_timeout_action = sta_mlme_assoc_req_timeout_action;
	sta_assoc_api.mlme_reassoc_req_action = sta_mlme_reassoc_req_action;
	sta_assoc_api.peer_reassoc_rsp_action = sta_peer_reassoc_rsp_action;
	sta_assoc_api.mlme_reassoc_req_timeout_action = sta_mlme_reassoc_req_timeout_action;
	sta_assoc_api.mlme_disassoc_req_action = sta_mlme_disassoc_req_action;
	sta_assoc_api.peer_disassoc_action =     sta_peer_disassoc_action;
	sta_assoc_api.mlme_disassoc_req_timeout_action = sta_mlme_disassoc_req_timeout_action;
	wdev->assoc_api = &sta_assoc_api;
	wdev->assoc_machine.CurrState = ASSOC_IDLE;

	/* initialize the timer */
	if (!pStaCfg->MlmeAux.AssocTimer.Valid) {
		pStaCfg->MlmeAux.AssocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.AssocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.AssocTimer,
					  GET_TIMER_FUNCTION(sta_assoc_timeout), &pStaCfg->MlmeAux.AssocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->MlmeAux.ReassocTimer.Valid) {
		pStaCfg->MlmeAux.ReassocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.ReassocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.ReassocTimer,
					  GET_TIMER_FUNCTION(sta_reassoc_timeout), &pStaCfg->MlmeAux.ReassocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->MlmeAux.DisassocTimer.Valid) {
		pStaCfg->MlmeAux.DisassocTimerFuncContext.pAd = pAd;
		pStaCfg->MlmeAux.DisassocTimerFuncContext.wdev = wdev;
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.DisassocTimer,
					  GET_TIMER_FUNCTION(sta_disassoc_timeout), &pStaCfg->MlmeAux.DisassocTimerFuncContext, FALSE);
	}

	if (!pStaCfg->LinkDownTimer.Valid)
		RTMPInitTimer(pAd, &pStaCfg->LinkDownTimer,
					  GET_TIMER_FUNCTION(sta_link_down_exec), wdev, FALSE);

	if (!pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer.Valid)
		RTMPInitTimer(pAd, &pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer,
					  GET_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc), wdev, FALSE);
}

