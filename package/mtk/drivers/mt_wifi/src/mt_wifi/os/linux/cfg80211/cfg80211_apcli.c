#ifdef MTK_LICENSE
/****************************************************************************
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
 ***************************************************************************/
#endif /* MTK_LICENSE */
/****************************************************************************

	Abstract:

	All related CFG80211 P2P function body.

	History:

***************************************************************************/
#ifdef APCLI_CFG80211_SUPPORT
#include "rt_config.h"

INT CFG80211_FindStaIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev)
{
	USHORT index = 0;
	BOOLEAN found = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg;
	struct wifi_dev *wdev;
	UINT32 max_num_sta = pAd->MaxMSTANum;

	max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);

#ifndef IWCOMMAND_CFG80211_SUPPORT
	if (pAd->flg_msta_init != FALSE) {
#endif /* IWCOMMAND_CFG80211_SUPPORT */
		for (index = 0; index < max_num_sta; index++) {
			pStaCfg = &pAd->StaCfg[index];
			wdev = &pStaCfg->wdev;
			if (pNetDev == wdev->if_dev) {
				found = TRUE;
				break;
			}
		}
#ifndef IWCOMMAND_CFG80211_SUPPORT
	}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

	return (found) ? index : WDEV_NOT_FOUND;
}


VOID CFG80211DRV_SetApCliAssocIe(VOID *pAdOrg, PNET_DEV pNetDev, VOID *pData, UINT ie_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	STA_ADMIN_CONFIG *apcli_entry;
	INT ifIndex;
	hex_dump("APCLI=", pData, ie_len);

	ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
	if (ifIndex == WDEV_NOT_FOUND) {
		printk("ifIndex for net device not found Returning \n");
		return;
	}
	apcli_entry = &pAd->StaCfg[ifIndex];

	if (ie_len > 0) {
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}

		os_alloc_mem(NULL, (UCHAR **)&apcli_entry->wpa_supplicant_info.pWpaAssocIe, ie_len);
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = ie_len;
			os_move_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe, pData, apcli_entry->wpa_supplicant_info.WpaAssocIeLen);
		} else
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	} else {
		if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
			os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
			apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		}
		apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	}
}


VOID CFG80211_LostApInform(VOID *pAdOrg, struct wifi_dev *wdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	UCHAR BandIdx = 0;

	if (wdev != NULL) {
		BandIdx = HcGetBandByWdev(wdev);
		PNET_DEV pNetDev = pAd->StaCfg[BandIdx].wdev.if_dev;


		pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
		pAd->StaCfg[BandIdx].ApcliInfStat.Enable = FALSE;
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

		if (pNetDev) {
			struct wifi_dev *pwifi_dev = NULL;

			pwifi_dev = RTMP_OS_NETDEV_GET_WDEV(pNetDev);
			if (pwifi_dev && (pwifi_dev->open_state == FALSE))
				return;

			if (STA_STATUS_TEST_FLAG(&pAd->StaCfg[BandIdx], fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 1, 52))
				cfg80211_disconnected(pNetDev, 0, NULL, 0, FALSE, GFP_KERNEL);
#else
				cfg80211_disconnected(pNetDev, 0, NULL, 0, GFP_KERNEL);
#endif
			} else if (pAd->StaCfg[BandIdx].wdev.cntl_machine.CurrState >= CNTL_WAIT_AUTH) {

				cfg80211_connect_result(pNetDev, NULL, NULL, 0, NULL, 0, WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"80211> BUG CFG80211_LostAPInform, BUT NetDevice not exist.\n");
		}
	}
}

VOID CFG80211DRV_ApClientKeyAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	PSTA_ADMIN_CONFIG pApCliEntry;
	MAC_TABLE_ENTRY	*pMacEntry = (MAC_TABLE_ENTRY *)NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	INT ifIndex;
	INT BssIdx;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
	ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pKeyInfo->pNetDev);
	if (ifIndex == WDEV_NOT_FOUND) {
		printk("Could not add key as Ifindex for netdevice not found \n");
		return;
	}
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAIN_MBSSID;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
	pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];
	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG Debug: CFG80211DRV_ApClientKeyAdd\n");
	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
			IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
			IS_MT7981(pAd)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
		if (pKeyInfo->bPairwise == FALSE)
#else
			if (pKeyInfo->KeyId > 0)
#endif /* LINUX_VERSION_CODE (2, 6, 37) */
			{
				struct _ASIC_SEC_INFO *info = NULL;
				int i = 0;
				if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
					SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
				else
					SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);

				NdisCopyMemory(&pAd->cfg80211_ctrl.WepKeyInfoBackup, pKeyInfo, sizeof(CMD_RTPRIV_IOCTL_80211_KEY));
				wdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen = pKeyInfo->KeyLen;
				os_move_mem(wdev->SecConfig.WepKey[pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				wdev->SecConfig.GroupKeyId = pKeyInfo->KeyId;
				os_move_mem(wdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_GROUP_KEY;
				info->Direction = SEC_ASIC_KEY_RX;
				info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
				info->BssIndex = pMacEntry->func_tb_idx;
				/* Info.BssIndex = wdev->wdev_idx; */
				info->Cipher = wdev->SecConfig.GroupCipher;
				info->KeyIdx = pKeyInfo->KeyId;
				os_move_mem(&info->PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				info->Key.KeyLen = pKeyInfo->KeyLen;

				for (i = 0; i < pKeyInfo->KeyLen; i++)
					printk("%02x ", pKeyInfo->KeyBuf[i]);
				printk("\n");
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
			{
				struct _ASIC_SEC_INFO *info = NULL;
				int i = 0;
				pMacEntry->SecConfig.PairwiseKeyId = pKeyInfo->KeyId;
				if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
					SET_CIPHER_WEP40(pMacEntry->SecConfig.PairwiseCipher);
				else
					SET_CIPHER_WEP104(pMacEntry->SecConfig.PairwiseCipher);

				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				os_zero_mem(info, sizeof(ASIC_SEC_INFO));
				info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				info->Direction = SEC_ASIC_KEY_BOTH;
				info->Wcid = pMacEntry->wcid;
				info->BssIndex = pMacEntry->func_tb_idx;
				/* Info.BssIndex = wdev->wdev_idx; */
				info->KeyIdx = pMacEntry->SecConfig.PairwiseKeyId;
				info->Cipher = pMacEntry->SecConfig.PairwiseCipher;
				info->KeyIdx = pMacEntry->SecConfig.PairwiseKeyId;
				os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				info->Key.KeyLen = pKeyInfo->KeyLen;
				for (i = 0; i < pKeyInfo->KeyLen; i++)
					printk("%02x ", pKeyInfo->KeyBuf[i]);
				printk("\n");
				HW_ADDREMOVE_KEYTABLE(pAd, info);
				os_free_mem(info);
			}
		}
#endif /* MT7615 || MT7622 || MT7626*/
	} else if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA) {
		if (pKeyInfo->cipher == Ndis802_11AESEnable) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
		if (pKeyInfo->bPairwise == FALSE)
#else
		if (pKeyInfo->KeyId > 0)
#endif /* LINUX_VERSION_CODE (2, 6, 37) */
		{
			if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
				IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
				IS_MT7981(pAd)) {
				struct _ASIC_SEC_INFO *info = NULL;

				NdisCopyMemory(&pMacEntry->SecConfig.GTK, pKeyInfo->KeyBuf, MAX_LEN_GTK);
				SET_CIPHER_CCMP128(pMacEntry->SecConfig.GroupCipher);
				/* Set key material to Asic */
				os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
				if (info) {
					os_zero_mem(info, sizeof(ASIC_SEC_INFO));
					/*NdisCopyMemory(&pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], pKeyInfo->KeyBuf, LEN_MAX_PTK); */
					info->Operation = SEC_ASIC_ADD_GROUP_KEY;
					info->Direction = SEC_ASIC_KEY_RX;
					info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
					info->BssIndex = pMacEntry->func_tb_idx;
					info->Cipher = pMacEntry->SecConfig.GroupCipher;
					info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
					os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(info->Key.Key, &pMacEntry->SecConfig.GTK, (LEN_TK + LEN_TK2));
					if (IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap)) {
						/* set 802.1x port control */
						tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
						wdev->PortSecured = WPA_802_1X_PORT_SECURED;
						pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
						WifiSysUpdatePortSecur(pAd, pMacEntry, NULL);
					}
					WPAInstallKey(pAd, info, TRUE, TRUE);
					os_free_mem(info);
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "struct alloc fail\n");
				}
			}
		} else {
			if (pMacEntry != NULL) {
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
				if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
					IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
					IS_MT7981(pAd)) {
					struct _ASIC_SEC_INFO *info = NULL;
					/* NdisCopyMemory(&pMacEntry->SecConfig.PTK, pKeyInfo->KeyBuf, LEN_TK + LEN_TK2); */
					SET_CIPHER_CCMP128(pMacEntry->SecConfig.PairwiseCipher);
					/* Set key material to Asic */
					os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
					if (info) {
						os_zero_mem(info, sizeof(ASIC_SEC_INFO));
						/*NdisCopyMemory(&pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], pKeyInfo->KeyBuf, LEN_MAX_PTK); */
						info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						info->Direction = SEC_ASIC_KEY_BOTH;
						info->Wcid = pMacEntry->wcid;
						info->BssIndex = pMacEntry->func_tb_idx;
						info->Cipher = pMacEntry->SecConfig.PairwiseCipher;
						info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
						os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
						os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, (LEN_TK + LEN_TK2));
						WPAInstallKey(pAd, info, TRUE, TRUE);
						os_free_mem(info);
					} else {
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "struct alloc fail\n");
					}
				}
#else
				os_zero_mem(&pMacEntry->PairwiseKey, sizeof(CIPHER_KEY));
				pMacEntry->PairwiseKey.KeyLen = LEN_TK;

				NdisCopyMemory(&pMacEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
				os_move_mem(pMacEntry->PairwiseKey.Key, &pMacEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);

				pMacEntry->PairwiseKey.CipherAlg = CIPHER_AES;

				AsicAddPairwiseKeyEntry(pAd, (UCHAR)pMacEntry->Aid, &pMacEntry->PairwiseKey);
				RTMPSetWcidSecurityInfo(pAd, BssIdx, 0, pMacEntry->PairwiseKey.CipherAlg, pMacEntry->Aid, PAIRWISEKEYTABLE);

#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT)
				RTMP_ADDREMOVE_KEY(pAd, 0, pMacEntry->func_tb_idx, 0, pMacEntry->wcid,
							    PAIRWISEKEYTABLE, &pMacEntry->PairwiseKey,
							    pMacEntry->Addr);
#endif /* MT_MAC*/
#endif /* MT_MAC*/
				} else {
					printk("APCLI: Set AES Security Set. (PAIRWISE) But pMacEntry NULL\n");
				}
			}
		} else if (pKeyInfo->cipher == Ndis802_11TKIPEnable) {
				/* TKIP */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
		if (pKeyInfo->bPairwise == FALSE)
#else
		if (pKeyInfo->KeyId > 0)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
			{
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
					if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
						IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
						IS_MT7981(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;
						NdisCopyMemory(&pMacEntry->SecConfig.GTK, pKeyInfo->KeyBuf, MAX_LEN_GTK);
						SET_CIPHER_TKIP(pMacEntry->SecConfig.GroupCipher);
						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
						if (info) {
							os_zero_mem(info, sizeof(ASIC_SEC_INFO));
							info->Operation = SEC_ASIC_ADD_GROUP_KEY;
							info->Direction = SEC_ASIC_KEY_RX;
							info->Wcid = wdev->bss_info_argument.bmc_wlan_idx;
							info->BssIndex = pMacEntry->func_tb_idx;
							info->Cipher = pMacEntry->SecConfig.GroupCipher;
							info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
							os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
							/* Install Shared key */
							os_move_mem(info->Key.Key, &pMacEntry->SecConfig.GTK, LEN_MAX_GTK);
							if (IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap)) {
								/* set 802.1x port control */
								tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
								wdev->PortSecured = WPA_802_1X_PORT_SECURED;
								pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
								WifiSysUpdatePortSecur(pAd, pMacEntry, NULL);
							}
							WPAInstallKey(pAd, info, TRUE, TRUE);
							wdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
							os_free_mem(info);
						} else {
							MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "struct alloc fail\n");
						}
				}
#else
				os_move_mem(pAd->SharedKey[apidx][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId,
					&pAd->SharedKey[apidx][pKeyInfo->KeyId]);
				GET_GroupKey_WCID(pWdev, Wcid);
				RTMPSetWcidSecurityInfo(pAd, apidx, (UINT8)(pKeyInfo->KeyId),
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg, Wcid, SHAREDKEYTABLE);

#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT)
				RTMP_ADDREMOVE_KEY(pAd, 0, apidx, pKeyInfo->KeyId, Wcid, SHAREDKEYTABLE,
					&pAd->SharedKey[apidx][pKeyInfo->KeyId], BROADCAST_ADDR);
#endif /* MT_MAC */
#endif /* MT7615 */
				}
			 else {
				if (pMacEntry != NULL) {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG: Set TKIP Security Set. (PAIRWISE) %d\n", pKeyInfo->KeyLen);
					NdisCopyMemory(&pMacEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
#if defined(MT7615) || defined(MT7622) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
						if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7626(pAd) ||
							IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) ||
							IS_MT7981(pAd)) {
							struct _ASIC_SEC_INFO *info = NULL;

							/*NdisCopyMemory(&pMacEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);*/
							SET_CIPHER_TKIP(pMacEntry->SecConfig.PairwiseCipher);
							/* Set key material to Asic */
							os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
							if (info) {
								os_zero_mem(info, sizeof(ASIC_SEC_INFO));
								/*NdisCopyMemory(&pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], pKeyInfo->KeyBuf, LEN_MAX_PTK);*/
								info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
								info->Direction = SEC_ASIC_KEY_BOTH;
								info->Wcid = pMacEntry->wcid;
								info->BssIndex = pMacEntry->func_tb_idx;
								info->Cipher = pMacEntry->SecConfig.PairwiseCipher;
								info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);/*pEntry->SecConfig.PairwiseKeyId;*/
								os_move_mem(&info->PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
								/*os_move_mem(info->Key.Key,&pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));*/
								os_move_mem(info->Key.Key, pKeyInfo->KeyBuf, (LEN_TK + LEN_TK2));
								WPAInstallKey(pAd, info, TRUE, TRUE);
								os_free_mem(info);
							} else {
								MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "struct alloc fail\n");
							}
						}
#else
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					NdisCopyMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					os_move_mem(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);

					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);
					RTMPSetWcidSecurityInfo(pAd, pEntry->apidx, (UINT8)(pKeyInfo->KeyId & 0x0fff), pEntry->PairwiseKey.CipherAlg, pEntry->Aid, PAIRWISEKEYTABLE);

#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT)
					RTMP_ADDREMOVE_KEY(pAd, 0, apidx, pKeyInfo->KeyId, pEntry->wcid, PAIRWISEKEYTABLE,
						&pEntry->PairwiseKey, pEntry->Addr);
#endif /* MT_MAC */
#endif /* MT7615 */
			} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CFG: Set TKIP Security Set. (PAIRWISE) But pEntry NULL\n");
				}
			}
		}
	}
}
#endif /* APCLI_CFG80211_SUPPORT */
