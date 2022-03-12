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
    ap_sec.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/
#include "rt_config.h"
BUILD_TIMER_FUNCTION(GroupRekeyExec);
#ifdef DOT11W_PMF_SUPPORT
BUILD_TIMER_FUNCTION(csa_timeout_proc);
#endif

#ifdef BCN_PROTECTION_SUPPORT
static VOID bcn_prot_init(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev)
{
	struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);

	/* Ausume that bcn protection is on only if pmf is on */
	if (sec_cfg->bcn_prot_cfg.desired_bcn_prot_en && chip_cap->bcn_prot_sup && sec_cfg->PmfCfg.MFPC)
		sec_cfg->bcn_prot_cfg.bcn_prot_en = TRUE;
	else {
		sec_cfg->bcn_prot_cfg.bcn_prot_en = FALSE;
		if (sec_cfg->bcn_prot_cfg.desired_bcn_prot_en)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_ERROR,
					("%s: disable bcn_prot, bcn_prot_sup(%d), MFPC(%d)\n",
					__func__, chip_cap->bcn_prot_sup, sec_cfg->PmfCfg.MFPC));
	}

	if (sec_cfg->bcn_prot_cfg.bcn_prot_en)
		sec_cfg->bcn_prot_cfg.bigtk_cipher = sec_cfg->PmfCfg.igtk_cipher;

}
#endif


#ifdef DOT11W_PMF_SUPPORT
VOID APPMFInit(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	/*
	   IEEE 802.11W/P.10 -
	   A STA that has associated with Management Frame Protection enabled
	   shall not use pairwise cipher suite selectors WEP-40, WEP-104,
	   TKIP, or "Use Group cipher suite".

	   IEEE 802.11W/P.3 -
	   IEEE Std 802.11 provides one security protocol, CCMP, for protection
	   of unicast Robust Management frames.
	 */
	pSecConfig->PmfCfg.MFPC = FALSE;
	pSecConfig->PmfCfg.MFPR = FALSE;
	pSecConfig->PmfCfg.PMFSHA256 = FALSE;

	if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
	/* In WPA3 spec, When a WPA3-Personal only BSS is configured,
	 * Protected Management Frame (PMF) shall be set to required (MFPR=1)
	 * When WPA2-Personal and WPA3-Personal are configured on the same BSS (mixed mode),
	 * Protected Management Frame (PMF) shall be set to capable (MFPC = 1, MFPR = 0)
	 */
		pSecConfig->PmfCfg.MFPC = TRUE;
		pSecConfig->PmfCfg.MFPR = (IS_AKM_WPA2PSK(pSecConfig->AKMMap)) ? FALSE : TRUE;
	} else if (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)
		|| IS_AKM_OWE(pSecConfig->AKMMap)
		|| IS_AKM_WPA3(pSecConfig->AKMMap)) {
	/* In WPA3 spec, When WPA3-Enterprise Suite B is used,
	 * Protected Management Frame (PMF) shall be set to required (MFPR=1).
	 */
		pSecConfig->PmfCfg.MFPC = TRUE;
		pSecConfig->PmfCfg.MFPR = TRUE;
		pSecConfig->PmfCfg.PMFSHA256 = IS_AKM_WPA3(pSecConfig->AKMMap) ? TRUE : FALSE;
	} else
	if ((IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA2PSK(pSecConfig->AKMMap))
		&& IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
		&& IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
		&& (pSecConfig->PmfCfg.Desired_MFPC
#ifdef OCE_SUPPORT
		|| IS_OCE_ENABLE(wdev)
#endif /* OCE_SUPPORT */
		)) {
		pSecConfig->PmfCfg.MFPC = TRUE;
		pSecConfig->PmfCfg.MFPR = pSecConfig->PmfCfg.Desired_MFPR;

		if ((pSecConfig->PmfCfg.Desired_PMFSHA256) || (pSecConfig->PmfCfg.MFPR))
			pSecConfig->PmfCfg.PMFSHA256 = TRUE;

#ifdef OCE_SUPPORT
		if (IS_OCE_ENABLE(wdev) &&
			pSecConfig->PmfCfg.Desired_MFPC == FALSE)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_OFF,
			("%s:: Force MFPC on when OCE enable\n", __func__));
#endif /* OCE_SUPPORT */
	} else if (pSecConfig->PmfCfg.Desired_MFPC)
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __func__));

	if (pSecConfig->PmfCfg.MFPC) {
		if (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap))
			SET_CIPHER_BIP_GMAC256(pSecConfig->PmfCfg.igtk_cipher);
		else
			SET_CIPHER_BIP_CMAC128(pSecConfig->PmfCfg.igtk_cipher);
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s:: apidx=%d, MFPC=%d, MFPR=%d, SHA256=%d\n",
			 __func__, wdev->func_idx, pSecConfig->PmfCfg.MFPC,
			 pSecConfig->PmfCfg.MFPR, pSecConfig->PmfCfg.PMFSHA256));
}


VOID ap_set_wait_sa_query_for_csa(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev)
{
	struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;
	UINT i, apidx;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;

	if (!sec_cfg->PmfCfg.MFPC)
		return;

	for (apidx = 0; apidx < ad->ApCfg.BssidNum; apidx++) {
		if (&ad->ApCfg.MBSSID[apidx].wdev == wdev)
			break;
	}

	if (apidx == ad->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			("%s: search apidx fail\n", __func__));
		return;
	}

	for (i = 0; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &ad->MacTab.Content[i];
		struct _SECURITY_CONFIG *sec_cfg_peer = &pEntry->SecConfig;

		if (IS_ENTRY_CLIENT(pEntry) &&
			sec_cfg_peer->Handshake.WpaState == AS_PTKINITDONE &&
			pEntry->func_tb_idx == apidx &&
			sec_cfg_peer->PmfCfg.UsePMFConnect &&
			sec_cfg_peer->ocv_support) {
			sec_cfg_peer->wait_csa_sa_query = TRUE;
			tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		}
	}

	RTMPSetTimer(&wdev->SecConfig.csa_disconnect_timer, 1500);
}

VOID csa_timeout_proc(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UINT i, apidx;
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;

	for (apidx = 0; apidx < ad->ApCfg.BssidNum; apidx++) {
		if (&ad->ApCfg.MBSSID[apidx].wdev.SecConfig.csa_disconnect_timer == pTimer)
			break;
	}

	if (apidx == ad->ApCfg.BssidNum)
		return;

	for (i = 0; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &ad->MacTab.Content[i];
		struct _SECURITY_CONFIG *sec_cfg_peer = &pEntry->SecConfig;

		if (IS_ENTRY_CLIENT(pEntry) &&
			sec_cfg_peer->Handshake.WpaState == AS_PTKINITDONE &&
			pEntry->func_tb_idx == apidx &&
			sec_cfg_peer->PmfCfg.UsePMFConnect &&
			sec_cfg_peer->ocv_support &&
			sec_cfg_peer->wait_csa_sa_query == TRUE) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
						("%s: no sa req received, kickout sta(wcid = %d)\n", __func__, i));
			MlmeDeAuthAction(ad, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		}
	}
}
#endif /* DOT11W_PMF_SUPPORT */


INT APSecInit(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	if (pSecConfig->AKMMap == 0x0)
		SET_AKM_OPEN(pSecConfig->AKMMap);

	if (pSecConfig->PairwiseCipher == 0x0)
		SET_CIPHER_NONE(pSecConfig->PairwiseCipher);

	/* Decide Group cipher */
	if ((IS_AKM_OPEN(pSecConfig->AKMMap) || IS_AKM_SHARED(pSecConfig->AKMMap))
		&& (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))) {
		/* WEP */
		pSecConfig->GroupCipher = pSecConfig->PairwiseCipher;
		pSecConfig->GroupKeyId = pSecConfig->PairwiseKeyId;
	} else if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap)
			   && IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
		/* Mix mode */
		SET_CIPHER_TKIP(pSecConfig->GroupCipher);
	} else
		pSecConfig->GroupCipher = pSecConfig->PairwiseCipher;

	/* Default key index is always 2 in WPA mode */
	if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap))
		pSecConfig->GroupKeyId = 1;

#ifdef DOT11R_FT_SUPPORT

	if (wdev->FtCfg.FtCapFlag.Dot11rFtEnable) {
		if (IS_AKM_WPA2(pSecConfig->AKMMap))
			SET_AKM_FT_WPA2(pSecConfig->AKMMap);

		if (IS_AKM_WPA2PSK(pSecConfig->AKMMap))
			SET_AKM_FT_WPA2PSK(pSecConfig->AKMMap);

		if (IS_AKM_WPA3PSK(pSecConfig->AKMMap))
			SET_AKM_FT_SAE_SHA256(pSecConfig->AKMMap);

		if (IS_AKM_SUITEB_SHA384(pSecConfig->AKMMap))
			SET_AKM_FT_WPA2_SHA384(pSecConfig->AKMMap);
	}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	APPMFInit(pAd, wdev);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
	bcn_prot_init(pAd, wdev); /* bcn_prot_init should be placed after APPMFInit */
#endif
#ifdef DOT11_SAE_SUPPORT
	if (pSecConfig->pwd_id_cnt == 0) {
		pSecConfig->sae_cap.pwd_id_only = FALSE;
		DlListInit(&pSecConfig->pwd_id_list_head.list);
	}

	if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

		if (pSecConfig->sae_cap.sae_pk_en != SAE_PK_DISABLE) {
			if (pSecConfig->sae_pk.group_id != 19 &&
				pSecConfig->sae_pk.group_id != 20 &&
				pSecConfig->sae_pk.group_id != 21)
				pSecConfig->sae_pk.group_id = SAE_DEFAULT_GROUP;

			if (FALSE == sae_pk_init(pAd, &pSecConfig->sae_pk, pMbss->Ssid, pMbss->SsidLen, SAE_PK_ROLE_AUTHENICATOR, pSecConfig->PSK))
				pSecConfig->sae_cap.sae_pk_en = SAE_PK_DISABLE;
		}

		sae_derive_pt(&pAd->SaeCfg, pSecConfig->PSK, pMbss->Ssid, pMbss->SsidLen, &pSecConfig->pwd_id_list_head, &pSecConfig->pt_list);
	}
#endif
	/* Generate the corresponding RSNIE */
	WPAMakeRSNIE(wdev->wdev_type, &wdev->SecConfig, NULL);

	os_move_mem(&wdev->SecConfig.BCN_RSNE_Content[0][0], &wdev->SecConfig.RSNE_Content[0][0], wdev->SecConfig.RSNE_Len[0]);
	os_move_mem(&wdev->SecConfig.BCN_RSNE_Content[1][0], &wdev->SecConfig.RSNE_Content[1][0], wdev->SecConfig.RSNE_Len[1]);
	wdev->SecConfig.BCN_RSNE_Len[0] = wdev->SecConfig.RSNE_Len[0];
	wdev->SecConfig.BCN_RSNE_Len[1] = wdev->SecConfig.RSNE_Len[1];

	return TRUE;
}

INT ap_sec_deinit(
	IN struct wifi_dev *wdev)
{
#ifdef DOT11_SAE_SUPPORT
	struct _SECURITY_CONFIG *sec_cfg = &wdev->SecConfig;

	if (IS_AKM_SAE(sec_cfg->AKMMap)) {
		sae_pt_list_deinit(&sec_cfg->pt_list);
		sae_pk_deinit(&sec_cfg->sae_pk);
	}
#endif

	return TRUE;
}

INT ap_key_table_init(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev)
{
	BSS_STRUCT *pMbss = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR repeat = TRUE;

	if (wdev == NULL)
		return 0;

	/*
	Initialize security variable per entry,
	1. pairwise key table, re-set all WCID entry as NO-security mode.
	2. access control port status
	*/
	/* Init Security variables */
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pSecConfig = &wdev->SecConfig;
	if (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_DISCONNECT)
		repeat = FALSE;
	RTMPInitTimer(pAd, &pSecConfig->GroupRekeyTimer, GET_TIMER_FUNCTION(GroupRekeyExec), pAd,  repeat);
	if (pSecConfig->ocv_support)
		RTMPInitTimer(pAd, &pSecConfig->csa_disconnect_timer, GET_TIMER_FUNCTION(csa_timeout_proc), pAd,  FALSE);

	if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap))
		pSecConfig->GroupKeyId = 1;

	/* When WEP, TKIP or AES is enabled, set group key info to Asic */
	if (IS_CIPHER_WEP(pSecConfig->GroupCipher)) {
		INT i;

		/* Generate 3-bytes IV randomly for software encryption using */
		for (i = 0; i < LEN_WEP_TSC; i++)
			pSecConfig->WepKey[pSecConfig->GroupCipher].TxTsc[i] = RandomByte(pAd);
	} else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher)
			   || IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
			   || IS_CIPHER_CCMP256(pSecConfig->GroupCipher)
			   || IS_CIPHER_GCMP128(pSecConfig->GroupCipher)
			   || IS_CIPHER_GCMP256(pSecConfig->GroupCipher)) {
		/* Calculate PMK */
		SetWPAPSKKey(pAd, pSecConfig->PSK, strlen(pSecConfig->PSK), (PUCHAR) pMbss->Ssid, pMbss->SsidLen, pSecConfig->PMK);
		/* Generate GMK and GNonce randomly per MBSS */
		GenRandom(pAd, wdev->bssid, pSecConfig->GMK);
		GenRandom(pAd, wdev->bssid, pSecConfig->Handshake.GNonce);
		/* Derive GTK per BSSID */
		WpaDeriveGTK(pSecConfig->GMK,
					 (UCHAR  *) pSecConfig->Handshake.GNonce,
					 wdev->bssid,
					 (UCHAR *) pSecConfig->GTK,
					 LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT
		if (pSecConfig->PmfCfg.MFPC == TRUE) {
			/* IGTK default key index as 4 */
			pSecConfig->PmfCfg.IGTK_KeyIdx = 4;
			/* Clear IPN */
			NdisZeroMemory(&pSecConfig->PmfCfg.IPN[0][0], LEN_WPA_TSC);
			/* Derive IGTK */
			PMF_DeriveIGTK(pAd, &pSecConfig->PmfCfg.IGTK[0][0]);
		}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
		if (pSecConfig->bcn_prot_cfg.bcn_prot_en == TRUE) {
			/* BIGTK default key index as 6 */
			pSecConfig->bcn_prot_cfg.bigtk_key_idx = 6;
			/* Derive BIGTK */
			PMF_DeriveIGTK(pAd, &pSecConfig->bcn_prot_cfg.bigtk[0][0]);
		}
#endif
	}

	return TRUE;
}



INT ap_set_key_for_sta_rec(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN STA_REC_CTRL_T * sta_rec)
{
	BSS_STRUCT *pMbss = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ASIC_SEC_INFO *asic_sec_info = &sta_rec->asic_sec_info;
	USHORT Wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (wdev == NULL)
		return 0;

	/*
	Initialize security variable per entry,
	1. pairwise key table, re-set all WCID entry as NO-security mode.
	2. access control port status
	*/
	/* Init Security variables */
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pSecConfig = &wdev->SecConfig;

	/* Get a specific WCID to record this MBSS key attribute */
	Wcid = sta_rec->WlanIdx;
	tr_entry = &tr_ctl->tr_entry[Wcid];
	wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	/* Set key material to Asic */
	os_zero_mem(asic_sec_info, sizeof(ASIC_SEC_INFO));
	asic_sec_info->Operation = SEC_ASIC_ADD_GROUP_KEY;
	asic_sec_info->Direction = SEC_ASIC_KEY_TX;
	asic_sec_info->Wcid = Wcid;
	asic_sec_info->BssIndex = wdev->func_idx;
	asic_sec_info->Cipher = pSecConfig->GroupCipher;
	asic_sec_info->KeyIdx = pSecConfig->GroupKeyId;
	os_move_mem(&asic_sec_info->PeerAddr[0], wdev->bssid, MAC_ADDR_LEN);

	/* When WEP, TKIP or AES is enabled, set group key info to Asic */
	if (IS_CIPHER_WEP(pSecConfig->GroupCipher)) {
		os_move_mem(&asic_sec_info->Key, &pSecConfig->WepKey[asic_sec_info->KeyIdx], sizeof(SEC_KEY_INFO));
		sta_rec->EnableFeature |= STA_REC_INSTALL_KEY_FEATURE;
	} else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher)
			   || IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
			   || IS_CIPHER_CCMP256(pSecConfig->GroupCipher)
			   || IS_CIPHER_GCMP128(pSecConfig->GroupCipher)
			   || IS_CIPHER_GCMP256(pSecConfig->GroupCipher)) {
		struct _SEC_KEY_INFO *pGroupKey = &asic_sec_info->Key;
		/* Install Shared key */
#ifdef RT_CFG80211_SUPPORT
		if (wdev->Is_hostapd_gtk) {
			os_move_mem(pGroupKey->Key, wdev->Hostapd_GTK, LEN_MAX_GTK);
			os_move_mem(pSecConfig->GTK, wdev->Hostapd_GTK, LEN_MAX_GTK);
		} else
#endif
			os_move_mem(pGroupKey->Key, pSecConfig->GTK, LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT
		if (pSecConfig->PmfCfg.MFPC == TRUE) {
			os_move_mem(asic_sec_info->IGTK, &pSecConfig->PmfCfg.IGTK[0][0], LEN_MAX_IGTK);
			if (IS_CIPHER_BIP_CMAC128(pSecConfig->PmfCfg.igtk_cipher) ||
				IS_CIPHER_BIP_GMAC128(pSecConfig->PmfCfg.igtk_cipher))
				asic_sec_info->IGTKKeyLen = LEN_BIP128_IGTK;
			else if (IS_CIPHER_BIP_CMAC256(pSecConfig->PmfCfg.igtk_cipher) ||
				IS_CIPHER_BIP_GMAC256(pSecConfig->PmfCfg.igtk_cipher))
				asic_sec_info->IGTKKeyLen = LEN_BIP256_IGTK;

			if (asic_sec_info->IGTKKeyLen != 0)
				asic_sec_info->Cipher |= pSecConfig->PmfCfg.igtk_cipher;

			asic_sec_info->igtk_key_idx = pSecConfig->PmfCfg.IGTK_KeyIdx;
		}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
		if (pSecConfig->bcn_prot_cfg.bcn_prot_en == TRUE) {
			os_move_mem(asic_sec_info->bigtk, &pSecConfig->bcn_prot_cfg.bigtk[0][0], LEN_MAX_BIGTK);

			/* assume that pmf is on and igtk cipher is equal to bigtk cipher */
			/* do nothing for asic_sec_info->Cipher assignment due to igtk cipher == bigtk cipher*/
			asic_sec_info->bigtk_key_len = asic_sec_info->IGTKKeyLen;

			asic_sec_info->bigtk_key_idx = pSecConfig->bcn_prot_cfg.bigtk_key_idx;
		}
#endif
		WPAInstallKey(pAd, asic_sec_info, TRUE, FALSE);
		sta_rec->EnableFeature |= STA_REC_INSTALL_KEY_FEATURE;
		pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("### BSS(%d) AKM=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x\n",
			 wdev->func_idx, pSecConfig->AKMMap, pSecConfig->PairwiseCipher, pSecConfig->GroupCipher));
	return TRUE;
}

VOID GroupRekeyExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UINT i, apidx;
	ULONG temp_counter = 0;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		if (&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.GroupRekeyTimer == pTimer)
			break;
	}

	if (apidx == pAd->ApCfg.BssidNum)
		return;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (pSecConfig->GroupReKeyMethod != SEC_GROUP_REKEY_DISCONNECT) {

		if (pSecConfig->GroupReKeyInterval == 0)
			return;

		if (pSecConfig->Handshake.GTKState == REKEY_NEGOTIATING) {
			pSecConfig->GroupReKeyInstallCountDown--;

			if (pSecConfig->GroupReKeyInstallCountDown == 0)
				goto INSTALL_KEY;
		}

		if (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_TIME)
			temp_counter = (++pSecConfig->GroupPacketCounter);
		else if (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_PACKET)
			temp_counter = pSecConfig->GroupPacketCounter/1000;  /* Packet-based: kilo-packets */
		else
			return;
	}

	if (temp_counter > pSecConfig->GroupReKeyInterval ||
		pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_DISCONNECT) {
		UINT entry_count = 0;

		pSecConfig->GroupPacketCounter = 0;
		pSecConfig->Handshake.GTKState = REKEY_NEGOTIATING;
		/* change key index */
		pSecConfig->GroupKeyId = (pSecConfig->GroupKeyId == 1) ? 2 : 1;
		/* Generate GNonce randomly per MBSS */
		GenRandom(pAd, wdev->bssid, pSecConfig->Handshake.GNonce);
		/* Derive GTK per BSSID */
		WpaDeriveGTK(pSecConfig->GMK,
					 (UCHAR	*) pSecConfig->Handshake.GNonce,
					 wdev->bssid,
					 (UCHAR *) pSecConfig->GTK,
					 LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT
		if (pSecConfig->PmfCfg.MFPC == TRUE) {
			UCHAR idx;

			pSecConfig->PmfCfg.IGTK_KeyIdx = (pSecConfig->PmfCfg.IGTK_KeyIdx == 4) ? 5 : 4;
			idx = (pSecConfig->PmfCfg.IGTK_KeyIdx == 4) ? 0 : 1;
			/* Derive IGTK */
			PMF_DeriveIGTK(pAd, &pSecConfig->PmfCfg.IGTK[idx][0]);
		}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
		if (pSecConfig->bcn_prot_cfg.bcn_prot_en == TRUE) {
			UCHAR idx;

			pSecConfig->bcn_prot_cfg.bigtk_key_idx = (pSecConfig->bcn_prot_cfg.bigtk_key_idx == 6) ? 7 : 6;
			idx = get_bigtk_table_idx(&pSecConfig->bcn_prot_cfg);
			/* Derive BIGTK */
			PMF_DeriveIGTK(pAd, &pSecConfig->bcn_prot_cfg.bigtk[idx][0]);
		}
#endif

		/* Process 2-way handshaking */
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			MAC_TABLE_ENTRY  *pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry)
				&& (pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE)
				&& (pEntry->func_tb_idx == apidx)) {
#ifdef A4_CONN
				if (IS_ENTRY_A4(pEntry))
					continue;
#endif /* A4_CONN */
				entry_count++;
				RTMPSetTimer(&pEntry->SecConfig.StartFor2WayTimer, ENQUEUE_EAPOL_2WAY_START_TIMER);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rekey interval excess, Update Group Key for  %02X:%02X:%02X:%02X:%02X:%02X , DefaultKeyId= %x\n",
						 PRINT_MAC(pEntry->Addr), pSecConfig->GroupKeyId));
			}
		}

		if (entry_count == 0)
			goto INSTALL_KEY;
		else
			pSecConfig->GroupReKeyInstallCountDown = 1; /* 1 seconds */
	}

	return;
INSTALL_KEY:
	/* If no sta connect, directly install group rekey, else install key after 2 way completed or 1 seconds */
	group_key_install(pAd, wdev);
	pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
}


VOID WPAGroupRekeyByWdev(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)

{
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	if (IS_CIPHER_TKIP(pSecConfig->GroupCipher)
		|| IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
		|| IS_CIPHER_CCMP256(pSecConfig->GroupCipher)
		|| IS_CIPHER_GCMP128(pSecConfig->GroupCipher)
		|| IS_CIPHER_GCMP256(pSecConfig->GroupCipher)) {
		/* Group rekey related */
		if ((pSecConfig->GroupReKeyInterval != 0)
			&& ((pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_TIME)
				|| (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_PACKET))) {
			pSecConfig->GroupPacketCounter = 0;
			RTMPSetTimer(&pSecConfig->GroupRekeyTimer, GROUP_KEY_UPDATE_EXEC_INTV);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s : Group rekey method= %d , interval = 0x%lx\n",
					 __func__, pSecConfig->GroupReKeyMethod, pSecConfig->GroupReKeyInterval));
		}
	}
}

/*
	Set group re-key timer if necessary.
	It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
*/
VOID APStartRekeyTimer(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	if (HcIsRadioAcq(wdev)) {
		WPAGroupRekeyByWdev(pAd, wdev);
	}
}


VOID APStopRekeyTimer(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	BOOLEAN Cancelled;
	RTMPCancelTimer(&wdev->SecConfig.GroupRekeyTimer, &Cancelled);
	if (wdev->SecConfig.ocv_support)
		RTMPCancelTimer(&wdev->SecConfig.csa_disconnect_timer, &Cancelled);
}


VOID APReleaseRekeyTimer(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	BOOLEAN Cancelled;

	RTMPReleaseTimer(&wdev->SecConfig.GroupRekeyTimer, &Cancelled);
	if (wdev->SecConfig.ocv_support)
		RTMPReleaseTimer(&wdev->SecConfig.csa_disconnect_timer, &Cancelled);
}


static PCHAR portsecured[] = {"NONE", "PORT_SECURED", "NOT_SECURED"};
INT Show_APSecurityInfo_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg)
{
	UINT16 idx;
	USHORT Wcid;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;
	struct tx_rx_ctl *tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR apidx;
	UINT16 wtbl_max_num;

	if (!pAd)
		return FALSE;

	tr_ctl = &pAd->tr_ctl;
	wtbl_max_num = WTBL_MAX_NUM(pAd);

	if (arg != NULL) {
		apidx = os_str_toul(arg, 0, 10);

		if (apidx >= pAd->ApCfg.BssidNum) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Invalid apidx\n"));
		} else {
			pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			GET_GroupKey_WCID(wdev, Wcid);
			tr_entry = &tr_ctl->tr_entry[Wcid];
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("BSS(%d), WCID(%d), AuthMode(%s, %x), PairwiseCipher(%s)\nGroupCipher(%s), GroupKeyId(%d), PortSecured(%s)\n",
				 apidx,
				 Wcid,
				 GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
				 GET_SEC_AKM(pSecConfig),
				 GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
				 GetEncryModeStr(GET_GROUP_CIPHER(pSecConfig)),
				 pSecConfig->GroupKeyId,
				 portsecured[tr_entry->PortSecured]));
#ifdef DOT11W_PMF_SUPPORT
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("[PMF]:MFPC = %d, MFPR = %d, SHA256 = %d\n",
						pSecConfig->PmfCfg.MFPC, pSecConfig->PmfCfg.MFPR, pSecConfig->PmfCfg.PMFSHA256));
#endif
#ifdef BCN_PROTECTION_SUPPORT
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("BCN_PROT_EN = %d\n", pSecConfig->bcn_prot_cfg.bcn_prot_en));
#endif
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("ocv_support = %d, td_value_fixed_en = %d, td_value = %d\n",
						pSecConfig->ocv_support, pSecConfig->td_value_fixed_en, pSecConfig->td_value));
			if (IS_CIPHER_WEP(GET_PAIRWISE_CIPHER(pSecConfig))) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("keyid(%d), Key = %s keylen = %d\n",
					pSecConfig->PairwiseKeyId,
					pSecConfig->WepKey[pSecConfig->PairwiseKeyId].Key,
					pSecConfig->WepKey[pSecConfig->PairwiseKeyId].KeyLen));
				hex_dump_with_lvl("hex key", pSecConfig->WepKey[pSecConfig->PairwiseKeyId].Key, pSecConfig->WepKey[pSecConfig->PairwiseKeyId].KeyLen, 1);
			} else if (IS_CIPHER_TKIP(GET_PAIRWISE_CIPHER(pSecConfig)) ||
				   IS_CIPHER_CCMP128(GET_PAIRWISE_CIPHER(pSecConfig)) ||
				   IS_CIPHER_CCMP256(GET_PAIRWISE_CIPHER(pSecConfig)) ||
				   IS_CIPHER_GCMP128(GET_PAIRWISE_CIPHER(pSecConfig)) ||
				   IS_CIPHER_GCMP256(GET_PAIRWISE_CIPHER(pSecConfig))) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Key = %s\n", pSecConfig->PSK));
			}
#ifdef DOT11_SAE_SUPPORT
			if (!DlListEmpty(&pSecConfig->pwd_id_list_head.list)) {
				struct pwd_id_list *list = NULL;
				UCHAR i = 0;

				DlListForEach(list, &pSecConfig->pwd_id_list_head.list, struct pwd_id_list, list) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d pwdid = %s, pwd = %s\n", i++, list->pwd_id, list->pwd));
					hex_dump_with_lvl("pwdid", list->pwd_id, 40, 1);
					hex_dump_with_lvl("pwd", list->pwd, LEN_PSK, 1);
				}
			}

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("PWDID Required = %d\n", pSecConfig->sae_cap.pwd_id_only));

			if (IS_AKM_SAE(pSecConfig->AKMMap))
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("SAE: gen_pwe_method(%d), pwd_id_only(%d), sae_pk_en(%d), sae_pk_test_ctrl = %d\n",
					pSecConfig->sae_cap.gen_pwe_method,
					pSecConfig->sae_cap.pwd_id_only,
					pSecConfig->sae_cap.sae_pk_en,
					pSecConfig->sae_pk.sae_pk_test_ctrl));
#endif
		}
		return TRUE;
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: AP\n"));
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\tWCID\tAuthMode\tPairwiseCipher\tGroupCipher\tGroupKeyId\tPortSecured"));
#ifdef DOT11W_PMF_SUPPORT
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPMF"));
#endif
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		pSecConfig = &pAd->ApCfg.MBSSID[idx].wdev.SecConfig;
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		GET_GroupKey_WCID(wdev, Wcid);
		tr_entry = &tr_ctl->tr_entry[Wcid];
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d\t%d\t%s\t\t%s\t\t%s\t\t%d\t\t%s",
				 idx,
				 Wcid,
				 GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
				 GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
				 GetEncryModeStr(GET_GROUP_CIPHER(pSecConfig)),
				 pSecConfig->GroupKeyId,
				 portsecured[tr_entry->PortSecured]));
#ifdef DOT11W_PMF_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", pSecConfig->PmfCfg.UsePMFConnect));
#endif
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
#ifdef APCLI_SUPPORT
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: AP Client\n"));
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\tWCID\tAuthMode\tPairwiseCipher\tPortSecured\n"));

	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		PSTA_ADMIN_CONFIG  pApCliEntry = &pAd->StaCfg[idx];

		pSecConfig = &pApCliEntry->wdev.SecConfig;
		wdev = &pApCliEntry->wdev;
		tr_entry = &tr_ctl->tr_entry[wdev->bss_info_argument.bmc_wlan_idx];

		if (pApCliEntry->ApcliInfStat.Enable == TRUE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d\t%d\t%s\t\t%s\t\t%s\n",
					 idx,
					 wdev->bss_info_argument.bmc_wlan_idx,
					 GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
					 GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
					 portsecured[tr_entry->PortSecured]));
		}
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
#endif
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: STA\n"));
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\t\t\tAID\tWCID\tAuthMode\tPairwiseCipher\tPortSecured\n"));

	for (idx = 0; idx < wtbl_max_num; idx++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[idx];

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
		pSecConfig = &pEntry->SecConfig;

		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%d\t%s\t\t%s\t\t%s\n",
					 PRINT_MAC(pEntry->Addr),
					 pEntry->Aid,
					 pEntry->wcid,
					 GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
					 GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
					 portsecured[tr_entry->PortSecured]));
		}
	}

	return TRUE;
}

VOID CheckBMCPortSecured(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN BOOLEAN isConnect)
{
	UINT32 bss_index = pEntry->func_tb_idx;
	UINT32 wcid;
	UCHAR PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[bss_index].wdev;

	if (wdev->tr_tb_idx == WCID_NO_MATCHED(pAd))
		return;/* skip uninit tr_tb_idx. */

	if (isConnect)
		PortSecured = WPA_802_1X_PORT_SECURED;
	else {
		for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
			pMacEntry = &pAd->MacTab.Content[wcid];
			tr_entry = &tr_ctl->tr_entry[wcid];

			if (wcid == pEntry->wcid)
				continue;

			if (((pMacEntry)
				 && (IS_ENTRY_CLIENT(pMacEntry))
				 && (pMacEntry->Sst == SST_ASSOC)
				 && (pMacEntry->func_tb_idx == bss_index)
				 && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			   ) {
				PortSecured = WPA_802_1X_PORT_SECURED;
				break;
			}
		}

		if (wdev->SecConfig.GroupReKeyMethod == SEC_GROUP_REKEY_DISCONNECT) {
			RTMPSetTimer(&wdev->SecConfig.GroupRekeyTimer, GROUP_KEY_UPDATE_EXEC_INTV);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: trigger rekey\n", __func__));
		}
	}

	if (wdev->PortSecured != PortSecured) {
		tr_entry = &tr_ctl->tr_entry[wdev->bss_info_argument.bmc_wlan_idx];
		tr_entry->PortSecured = PortSecured;
		wdev->PortSecured = PortSecured;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: bss_index = %d, wcid = %d, PortSecured = %d\n",
				  __func__,
				  bss_index,
				  wdev->bss_info_argument.bmc_wlan_idx,
				  PortSecured));
	}
}


#ifdef DOT1X_SUPPORT
/*
    ========================================================================

    Routine Description:
	Sending EAP Req. frame to station in authenticating state.
	These frames come from Authenticator deamon.

    Arguments:
	pAdapter        Pointer to our adapter
	pPacket     Pointer to outgoing EAP frame body + 8023 Header
	Len             length of pPacket

    Return Value:
	None
    ========================================================================
*/
VOID WpaSend(RTMP_ADAPTER *pAdapter, UCHAR *pPacket, ULONG Len)
{
	PEAP_HDR pEapHdr;
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR Header802_3[LENGTH_802_3];
	MAC_TABLE_ENTRY *pEntry;
	struct tx_rx_ctl *tr_ctl = &pAdapter->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	PUCHAR pData;

	NdisMoveMemory(Addr, pPacket, 6);
	NdisMoveMemory(Header802_3, pPacket, LENGTH_802_3);
	pEapHdr = (EAP_HDR *)(pPacket + LENGTH_802_3);
	pData = (pPacket + LENGTH_802_3);
	pEntry = MacTableLookup(pAdapter, Addr);
	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WpaSend - No such MAC - %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(Addr)));
		return;
	}

	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];

	/* OS wait for completion time out */
	if (!pEntry->WtblSetFlag) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("##[%s] calling OS_WAIT\n", __func__));
		/*
		*  Here waiting for Wtbl enrty to be created before sending
		*  EAP packet. Max it will wait for 500msec
		*/
		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pEntry->WtblSetDone,
			RTMPMsecsToJiffies(500))) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR
			, ("#[%s]ERR:Wtbl entry not created 500msec\n", __func__));
		}
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE
			, ("###[%s] wait is over\n", __func__));
	}
	/* Send EAP frame to STA */
	if ((IS_AKM_WPA_CAPABILITY_Entry(pEntry) && (pEapHdr->ProType != EAPOLKey)) ||
		(IS_IEEE8021X(&pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig)))
		RTMPToWirelessSta(pAdapter,
						  pEntry,
						  Header802_3,
						  LENGTH_802_3,
						  pData,
						  Len - LENGTH_802_3,
						  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d), pEapHdr->code=%d, pEntry->SecConfig.Handshake.WpaState=%d\n", __func__, __LINE__, pEapHdr->code, pEntry->SecConfig.Handshake.WpaState));

	if (RTMPEqualMemory((pPacket+12), EAPOL, 2)) {
		switch (pEapHdr->code) {
		case EAP_CODE_REQUEST:
			if ((pEntry->SecConfig.Handshake.WpaState >= AS_PTKINITDONE) && (pEapHdr->ProType == EAPPacket)) {
				pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Start to re-authentication by 802.1x daemon\n"));
			}

			break;

		/* After receiving EAP_SUCCESS, trigger state machine */
		case EAP_CODE_SUCCESS:
			if (IS_AKM_WPA_CAPABILITY_Entry(pEntry) && (pEapHdr->ProType != EAPOLKey)) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Send EAP_CODE_SUCCESS\n\n"));

				if (pEntry->Sst == SST_ASSOC) {
					UINT8 pmk_len = LEN_PMK;

					pEntry->SecConfig.Handshake.WpaState = AS_INITPMK;
					pEntry->SecConfig.Handshake.MsgRetryCounter = 0;
					os_move_mem(&pEntry->SecConfig.Handshake.AAddr, pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN);
					os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr, MAC_ADDR_LEN);
					if (pEntry->SecConfig.key_deri_alg == SEC_KEY_DERI_SHA384)
						pmk_len = LEN_PMK_SHA384;
					os_move_mem(&pEntry->SecConfig.PMK,
						    &pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PMK,
						    pmk_len);
					WPABuildPairMsg1(pAdapter, &pEntry->SecConfig, pEntry);
				}
			} else {
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				pEntry->SecConfig.Handshake.WpaState = AS_PTKINITDONE;
				/* 1x+WEP will update port secured in key install stage, todo: change the below code to WifiSysUpdatePortSecur?  */
				if (IS_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher)) {
					pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.PortSecured = WPA_802_1X_PORT_SECURED;
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
				}
#ifdef WSC_AP_SUPPORT

				if (pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE)
					WscInformFromWPA(pEntry);

#endif /* WSC_AP_SUPPORT */
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IEEE8021X-WEP : Send EAP_CODE_SUCCESS\n\n"));
			}

			break;

		case EAP_CODE_FAILURE:
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Send Deauth, Reason : REASON_8021X_AUTH_FAIL\n"));
			MlmeDeAuthAction(pAdapter, pEntry, REASON_8021X_AUTH_FAIL, FALSE);
			break;

		default:
			break;
		}
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Send Deauth, Reason : REASON_NO_LONGER_VALID\n"));
		MlmeDeAuthAction(pAdapter, pEntry, REASON_NO_LONGER_VALID, FALSE);
	}
}

INT RTMPAddPMKIDCache(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *PMKID,
	IN UCHAR *PMK,
	IN UCHAR is_ft,
	IN UINT8 pmk_len)
{
	INT i, CacheIdx;
	/* Update PMKID status */
	CacheIdx = RTMPSearchPMKIDCache(pPMKIDCache, apidx, pAddr, is_ft);

	if (CacheIdx != INVALID_PMKID_IDX) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): cache found and renew it(%d)\n",
				  __func__, CacheIdx));
	} else {
		ULONG ts = 0;
		INT old_entry = 0;

		/* Add a new PMKID */
		for (i = 0; i < MAX_PMKID_COUNT; i++) {
			if (pPMKIDCache->BSSIDInfo[i].Valid == FALSE) {
				CacheIdx = i;
				break;
			}
			if ((ts == 0) || (ts > pPMKIDCache->BSSIDInfo[i].RefreshTime)) {
				ts = pPMKIDCache->BSSIDInfo[i].RefreshTime;
				old_entry = i;
			}
		}

		if (i == MAX_PMKID_COUNT) {
			CacheIdx = old_entry;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("%s():Cache full, replace oldest(%d)\n",
					  __func__, old_entry));
		}
	}

	pPMKIDCache->BSSIDInfo[CacheIdx].Valid = TRUE;
	pPMKIDCache->BSSIDInfo[CacheIdx].Mbssidx = apidx;
	pPMKIDCache->BSSIDInfo[CacheIdx].is_ft = is_ft;
	NdisGetSystemUpTime(&(pPMKIDCache->BSSIDInfo[CacheIdx].RefreshTime));
	COPY_MAC_ADDR(&pPMKIDCache->BSSIDInfo[CacheIdx].MAC, pAddr);
	NdisMoveMemory(&pPMKIDCache->BSSIDInfo[CacheIdx].PMKID, PMKID, LEN_PMKID);
	NdisMoveMemory(&pPMKIDCache->BSSIDInfo[CacheIdx].PMK, PMK, pmk_len);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): add %02x:%02x:%02x:%02x:%02x:%02x cache(%d) for ra%d\n",
			  __func__, PRINT_MAC(pAddr), CacheIdx, apidx));
	return CacheIdx;
}


INT RTMPSearchPMKIDCache(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR is_ft)
{
	INT	i = 0;

	for (i = 0; i < MAX_PMKID_COUNT; i++) {
		if ((pPMKIDCache->BSSIDInfo[i].Valid == TRUE)
			&& (pPMKIDCache->BSSIDInfo[i].Mbssidx == apidx)
			&& MAC_ADDR_EQUAL(&pPMKIDCache->BSSIDInfo[i].MAC, pAddr)
			&& (pPMKIDCache->BSSIDInfo[i].is_ft == is_ft)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s():%02x:%02x:%02x:%02x:%02x:%02x cache(%d) is_ft(%d) from IF(ra%d)\n",
					  __func__, PRINT_MAC(pAddr), i, is_ft, apidx));
			break;
		}
	}

	if (i >= MAX_PMKID_COUNT) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): - IF(%d) not found\n", __func__, apidx));
		return INVALID_PMKID_IDX;
	}

	return i;
}

INT RTMPSearchPMKIDCacheByPmkId(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPmkId)
{
	INT	i = 0;

	for (i = 0; i < MAX_PMKID_COUNT; i++) {
		if ((pPMKIDCache->BSSIDInfo[i].Valid == TRUE)
			&& (pPMKIDCache->BSSIDInfo[i].Mbssidx == apidx)
			&& MAC_ADDR_EQUAL(&pPMKIDCache->BSSIDInfo[i].MAC, pAddr)
			&& RTMPEqualMemory(pPmkId, &pPMKIDCache->BSSIDInfo[i].PMKID, LEN_PMKID)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s():%02x:%02x:%02x:%02x:%02x:%02x cache(%d) from IF(ra%d)\n",
					  __func__, PRINT_MAC(pAddr), i, apidx));
			break;
		}
	}

	if (i >= MAX_PMKID_COUNT) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): - IF(%d) not found\n", __func__, apidx));
		return INVALID_PMKID_IDX;
	}

	return i;
}

INT RTMPValidatePMKIDCache(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPMKID)
{
	INT CacheIdx = RTMPSearchPMKIDCache(pPMKIDCache, apidx, pAddr, FALSE);

	if (CacheIdx == INVALID_PMKID_IDX)
		return INVALID_PMKID_IDX;

	if (RTMPEqualMemory(pPMKID, &pPMKIDCache->BSSIDInfo[CacheIdx].PMKID, LEN_PMKID))
		return CacheIdx;
	else
		return INVALID_PMKID_IDX;
}

VOID RTMPDeletePMKIDCache(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN INT idx)
{
	PAP_BSSID_INFO pInfo = &pPMKIDCache->BSSIDInfo[idx];

	if (pInfo->Valid && (pInfo->Mbssidx == apidx)) {
		pInfo->Valid = FALSE;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():(IF(%d), del PMKID CacheIdx=%d\n", __func__, apidx, idx));
	}
}


VOID RTMPMaintainPMKIDCache(
	IN RTMP_ADAPTER *pAd)
{
	INT i;
	ULONG Now;

	for (i = 0; i < MAX_PMKID_COUNT; i++) {
		PAP_BSSID_INFO pBssInfo = &pAd->ApCfg.PMKIDCache.BSSIDInfo[i];

		NdisGetSystemUpTime(&Now);

		if ((pBssInfo->Valid)
			&& /*((Now - pBssInfo->RefreshTime) >= pMbss->PMKCachePeriod)*/
			(RTMP_TIME_AFTER(Now, (pBssInfo->RefreshTime + pAd->ApCfg.MBSSID[pBssInfo->Mbssidx].PMKCachePeriod))))
			RTMPDeletePMKIDCache(&pAd->ApCfg.PMKIDCache, pBssInfo->Mbssidx, i);
	}
}


UCHAR is_rsne_pmkid_cache_match(
	IN UINT8 *rsnie,
	IN UINT	rsnie_len,
	IN NDIS_AP_802_11_PMKID * pmkid_cache,
	IN INT apidx,
	IN UCHAR *addr,
	OUT INT *cacheidx)
{
	UINT8 *pmkid = NULL;
	UINT8 pmkid_count;

	pmkid = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PMKID_LIST, &pmkid_count);

	if (pmkid != NULL) {
		*cacheidx = RTMPValidatePMKIDCache(pmkid_cache, apidx, addr, pmkid);
		return TRUE;
	}
	return FALSE;
}
#endif /* DOT1X_SUPPORT */
