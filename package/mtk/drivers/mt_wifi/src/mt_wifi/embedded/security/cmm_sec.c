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
	cmm_sec.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"

VOID SetWdevAuthMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT32 AKMMap = 0;

	CLEAR_SEC_AKM(AKMMap);
#ifdef CONFIG_HOTSPOT_R3
	pSecConfig->bIsWPA2EntOSEN = FALSE;
#endif

	if (rtstrcasecmp(arg, "OPEN") == TRUE)
		SET_AKM_OPEN(AKMMap);
	else if (rtstrcasecmp(arg, "SHARED") == TRUE)
		SET_AKM_SHARED(AKMMap);
	else if (rtstrcasecmp(arg, "WEPAUTO") == TRUE) {
		SET_AKM_OPEN(AKMMap);
		SET_AKM_AUTOSWITCH(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA") == TRUE)
		SET_AKM_WPA1(AKMMap);
	else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
		SET_AKM_WPA1PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPANONE") == TRUE)
		SET_AKM_WPANONE(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2") == TRUE)
		SET_AKM_WPA2(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2MIX") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_WPA2_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA2PSK") == TRUE)
		SET_AKM_WPA2PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPA3") == TRUE) {
		/* WPA3 code flow is same as WPA2, the usage of SEC_AKM_WPA3 is to force pmf on */
		SET_AKM_WPA2(AKMMap);
		SET_AKM_WPA3(AKMMap);
	}
#ifdef DOT11_SUITEB_SUPPORT
	else if (rtstrcasecmp(arg, "WPA3-192") == TRUE)
		SET_AKM_WPA3_192BIT(AKMMap);
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	else if (rtstrcasecmp(arg, "WPA3PSK") == TRUE)
		SET_AKM_SAE_SHA256(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2PSKWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA2PSKMIXWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		SET_AKM_WPA2PSK_SHA256(AKMMap);
	}
#endif /* DOT11_SAE_SUPPORT */
	else if (rtstrcasecmp(arg, "WPA1WPA2") == TRUE) {
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
	} else if (rtstrcasecmp(arg, "WPAPSKWPA2PSK") == TRUE) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "OWE") == TRUE) {
		SET_AKM_OWE(AKMMap);
	}
#ifdef OCE_FILS_SUPPORT
	else if (rtstrcasecmp(arg, "FILS_SHA256") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_FILS_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "FILS_SHA384") == TRUE) {
		SET_AKM_WPA2(AKMMap);
		SET_AKM_FILS_SHA384(AKMMap);
	}
#endif /* OCE_FILS_SUPPORT */
#ifdef DPP_SUPPORT
	else if (rtstrcasecmp(arg, "DPP") == TRUE) {
		SET_AKM_DPP(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA3PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
	} else if (rtstrcasecmp(arg, "DPPWPA3PSKWPA2PSK") == TRUE) {
		SET_AKM_DPP(AKMMap);
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	}
#endif /* DPP_SUPPORT */

#ifdef CONFIG_HOTSPOT_R3
	else if (rtstrcasecmp(arg, "WPA2-Ent-OSEN") == TRUE) {
		pSecConfig->bIsWPA2EntOSEN = TRUE;
		SET_AKM_WPA2(AKMMap);
		SET_AKM_OSEN(AKMMap);
	}
#endif
	else {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support (AuthMode=%s, len=%d)\n",
				 arg, (int) strlen(arg));
	}

	if (AKMMap != 0x0)
		pSecConfig->AKMMap = AKMMap;

	MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::AuthMode=0x%x\n",
			 __func__, pSecConfig->AKMMap);
}


VOID SetWdevEncrypMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT Cipher = 0;

	if (rtstrcasecmp(arg, "NONE") == TRUE)
		SET_CIPHER_NONE(Cipher);
	else if (rtstrcasecmp(arg, "WEP") == TRUE)
		SET_CIPHER_WEP(Cipher);
	else if (rtstrcasecmp(arg, "TKIP") == TRUE)
		SET_CIPHER_TKIP(Cipher);
	else if ((rtstrcasecmp(arg, "AES") == TRUE) || (rtstrcasecmp(arg, "CCMP128") == TRUE))
		SET_CIPHER_CCMP128(Cipher);
	else if (rtstrcasecmp(arg, "CCMP256") == TRUE)
		SET_CIPHER_CCMP256(Cipher);
	else if (rtstrcasecmp(arg, "GCMP128") == TRUE)
		SET_CIPHER_GCMP128(Cipher);
	else if (rtstrcasecmp(arg, "GCMP256") == TRUE)
		SET_CIPHER_GCMP256(Cipher);
	else if ((rtstrcasecmp(arg, "TKIPAES") == TRUE) || (rtstrcasecmp(arg, "TKIPCCMP128") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	}

	else {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support (EncrypType=%s, len=%d)\n",
				 arg, (int) strlen(arg));
	}

	if (Cipher != 0x0) {
		pSecConfig->PairwiseCipher = Cipher;
		CLEAR_GROUP_CIPHER(pSecConfig);
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::PairwiseCipher=0x%x\n",
			 __func__, GET_PAIRWISE_CIPHER(pSecConfig));
}


INT Set_SecAuthMode_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n",
				 arg);
		return FALSE;
	}

	SetWdevAuthMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecEncrypType_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n",
				 arg);
		return FALSE;
	}

	SetWdevEncrypMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecDefaultKeyID_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	ULONG KeyIdx;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n",
				 arg);
		return FALSE;
	}

	KeyIdx = os_str_tol(arg, 0, 10);

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==> DefaultKeyId=%d\n",
			 pSecConfig->PairwiseKeyId);
	return TRUE;
}


INT	Set_SecWPAPSK_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	INT i;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n",
				 arg);
		return FALSE;
	}

	if (strlen(arg) < 65) {
		if (strlen(arg) != strlen(pSecConfig->PSK)
			|| !RTMPEqualMemory(arg, pSecConfig->PSK, strlen(arg))
			) {

			for (i = 0; i < MAX_PMKID_COUNT; i++) {
				if ((pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid == TRUE)
					&& (pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Mbssidx == pMbss->mbss_idx)) {
					pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid = FALSE;
					MTWF_PRINT("%s():Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n", __func__, i, pMbss->mbss_idx);
				}
			}
		}
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR sta_idx = pObj->ioctl_if;
		BOOLEAN is_psk_same = 0;
		UCHAR i = 0;

		 for (i = 0; i < (LEN_PSK + 1); i++) {
				if (pSecConfig->PSK[i] != arg[i]) {
					is_psk_same = 0;
					break;
				}

				if (pSecConfig->PSK[i] == '\0') {
					is_psk_same = 1;
					break;
				}
		}

		if (!is_psk_same) {
			/*PSK has changed we need to clear store apcli pmk cache for AKM's that use PSK*/
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"():Delete pmk cache on password change\n");

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)

			sta_delete_psk_pmkid_cache_all(pAd, sta_idx);
#endif
		}
	}
#endif /* APCLI_SUPPORT */
#endif
		os_move_mem(pSecConfig->PSK, arg, strlen(arg));
		pSecConfig->PSK[strlen(arg)] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PSK = %s\n", arg);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		WSC_CTRL *pWscControl = NULL;

		if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)) {
			UCHAR apidx = pObj->ioctl_if;

			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		}

#ifdef APCLI_SUPPORT
		else if (pObj->ioctl_if_type == INT_APCLI) {
			UCHAR    apcli_idx = pObj->ioctl_if;

			pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
		}

#endif /* APCLI_SUPPORT */

		if (pWscControl) {
			NdisZeroMemory(pWscControl->WpaPsk, 64);
			pWscControl->WpaPskLen = 0;
			pWscControl->WpaPskLen = strlen(arg);
			NdisMoveMemory(pWscControl->WpaPsk, arg, pWscControl->WpaPskLen);
		}
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Set_SecWEPKey_Proc(
	IN PRTMP_ADAPTER pAd,
	IN CHAR KeyId,
	IN RTMP_STRING * arg)
{
	INT retVal = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n",
				 arg);
		return FALSE;
	}

	retVal = ParseWebKey(pSecConfig, arg, KeyId, 0);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "KeyID=%d, key=%s\n", KeyId, arg);
	return retVal;
}


INT Set_SecKey1_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 0, arg);
}

INT Set_SecKey2_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 1, arg);
}

INT Set_SecKey3_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 2, arg);
}

INT Set_SecKey4_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 3, arg);
}


RTMP_STRING *GetAuthModeStr(
	IN UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return "OPEN";
	else if (IS_AKM_SHARED(authMode))
		return "SHARED";
	else if (IS_AKM_AUTOSWITCH(authMode))
		return "WEPAUTO";
	else if (IS_AKM_WPANONE(authMode))
		return "WPANONE";
	else if (IS_AKM_FT_WPA2PSK(authMode) && IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return "WPA1WPA2";
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return "WPAPSKWPA2PSK";
#ifdef	MAP_R3
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA3PSKWPA2PSK";
	else if (IS_AKM_WPA3PSK(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA3PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_DPP(authMode))
		return "DPPWPA2PSK";
#endif
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA2PSK_SHA256(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKMIXWPA3PSK";
	else if (IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-SAE";
	else if (IS_AKM_WPA3PSK(authMode))
		return "WPA3PSK";
	else if (IS_AKM_WPA1(authMode))
		return "WPA";
	else if (IS_AKM_WPA1PSK(authMode))
		return "WPAPSK";
	else if (IS_AKM_FT_WPA2(authMode))
		return "FT-WPA2";
	else if (IS_AKM_FT_WPA2PSK(authMode))
		return "FT-WPA2PSK";
	else if (IS_AKM_WPA3(authMode)) /* WPA3 will be always accompanied by WPA2, so it should put before the WPA2 */
		return "WPA3";
	else if (IS_AKM_WPA2(authMode))
		return "WPA2";
	else if (IS_AKM_WPA2(authMode) && IS_AKM_WPA2_SHA256(authMode))
		return "WPA2MIX";
	else if (IS_AKM_WPA2PSK(authMode))
		return "WPA2PSK";
	else if (IS_AKM_WPA3_192BIT(authMode))
		return "WPA3-192";
	else if (IS_AKM_OWE(authMode))
		return "OWE";
#ifdef DPP_SUPPORT
	else if (IS_AKM_DPP(authMode))
		return "DPP";
#endif /* DPP_SUPPORT */
	else
		return "UNKNOW";
}

RTMP_STRING *GetEncryModeStr(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return "NONE";
	else if (IS_CIPHER_WEP(encryMode))
		return "WEP";
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return "TKIPAES";
	else if (IS_CIPHER_TKIP(encryMode))
		return "TKIP";
	else if (IS_CIPHER_CCMP128(encryMode))
		return "AES";
	else if (IS_CIPHER_CCMP256(encryMode))
		return "CCMP256";
	else if (IS_CIPHER_GCMP128(encryMode))
		return "GCMP128";
	else if (IS_CIPHER_GCMP256(encryMode))
		return "GCMP256";
	else if (IS_CIPHER_BIP_CMAC128(encryMode))
		return "BIP-CMAC128";
	else if (IS_CIPHER_BIP_CMAC256(encryMode))
		return "BIP-CMAC256";
	else if (IS_CIPHER_BIP_GMAC128(encryMode))
		return "BIP-GMAC128";
	else if (IS_CIPHER_BIP_GMAC256(encryMode))
		return "BIP-GMAC256";
	else
		return "UNKNOW";
}

UINT32 SecAuthModeOldToNew(
	IN USHORT authMode)
{
	UINT32 AKMMap = 0;

	switch (authMode) {
	case Ndis802_11AuthModeOpen:
		SET_AKM_OPEN(AKMMap);
		break;

	case Ndis802_11AuthModeShared:
		SET_AKM_SHARED(AKMMap);
		break;

	case Ndis802_11AuthModeAutoSwitch:
		SET_AKM_AUTOSWITCH(AKMMap);
		break;

	case Ndis802_11AuthModeWPA:
		SET_AKM_WPA1(AKMMap);
		break;

	case Ndis802_11AuthModeWPAPSK:
		SET_AKM_WPA1PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPANone:
		SET_AKM_WPANONE(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2:
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2PSK:
		SET_AKM_WPA2PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1WPA2:
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1PSKWPA2PSK:
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		break;
	}

	return AKMMap;
}


UINT32 SecEncryModeOldToNew(
	IN USHORT encryMode)
{
	UINT32 EncryType = 0;

	switch (encryMode) {
	case Ndis802_11WEPDisabled:
		SET_CIPHER_NONE(EncryType);
		break;

	case Ndis802_11WEPEnabled:
		SET_CIPHER_WEP(EncryType);
		break;

	case Ndis802_11TKIPEnable:
		SET_CIPHER_TKIP(EncryType);
		break;

	case Ndis802_11AESEnable:
		SET_CIPHER_CCMP128(EncryType);
		break;

	case Ndis802_11TKIPAESMix:
		SET_CIPHER_TKIP(EncryType);
		SET_CIPHER_CCMP128(EncryType);
		break;
	}

	return EncryType;
}


USHORT SecAuthModeNewToOld(
	IN UINT32 authMode)
{
#ifdef CCAPI_API_SUPPORT
	if (IS_AKM_OPEN(authMode) && IS_AKM_AUTOSWITCH(authMode))
		return Ndis802_11AuthModeAutoSwitch;
	else if (IS_AKM_SHARED(authMode))
		return Ndis802_11AuthModeShared;
	else if (IS_AKM_OPEN(authMode))
		return Ndis802_11AuthModeOpen;
#else
	if (IS_AKM_OPEN(authMode))
		return Ndis802_11AuthModeOpen;
	else if (IS_AKM_SHARED(authMode))
		return Ndis802_11AuthModeShared;
	else if (IS_AKM_AUTOSWITCH(authMode))
		return Ndis802_11AuthModeAutoSwitch;
#endif
	else if (IS_AKM_WPANONE(authMode))
		return Ndis802_11AuthModeWPANone;
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA1WPA2;
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA1PSKWPA2PSK;
	else if (IS_AKM_WPA1(authMode))
		return Ndis802_11AuthModeWPA;
	else if (IS_AKM_WPA1PSK(authMode))
		return Ndis802_11AuthModeWPAPSK;

#ifdef CCAPI_API_SUPPORT
	else if (IS_AKM_WPA2(authMode) && IS_AKM_WPA2_SHA256(authMode))
		return Ndis802_11AuthModeWPA2MIX;
	else if (IS_AKM_WPA2PSK_SHA256(authMode) && IS_AKM_WPA3PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return NdisAuthModeWPA2PSKMIXWPA3PSK;
	else if (IS_AKM_WPA3PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA2PSKWPA3PSK;
	else if (IS_AKM_WPA3(authMode))
		return Ndis802_11AuthModeWPA3;
#endif

	else if (IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA2;
	else if (IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA2PSK;

#ifdef CCAPI_API_SUPPORT
	else if (IS_AKM_WPA3_192BIT(authMode))
		return Ndis802_11AuthModeWPA3_192;
	else if (IS_AKM_WPA3PSK(authMode))
		return Ndis802_11AuthModeWPA3PSK;
	else if (IS_AKM_OWE (authMode))
		return Ndis802_11AuthModeOWE;
#endif/*CCAPI_API_SUPPORT*/
	else
		return Ndis802_11AuthModeOpen;
}


USHORT SecEncryModeNewToOld(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return Ndis802_11WEPDisabled;
	else if (IS_CIPHER_WEP(encryMode))
		return Ndis802_11WEPEnabled;
	else if (IS_CIPHER_TKIP(encryMode))
		return Ndis802_11TKIPEnable;
	else if (IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11AESEnable;
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11TKIPAESMix;

	else
		return Ndis802_11WEPDisabled;
}


UINT8 SecHWCipherSuitMapping(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return CIPHER_SUIT_NONE;
	else if (IS_CIPHER_WEP(encryMode))
		return CIPHER_SUIT_WEP_40;
	else if (IS_CIPHER_TKIP(encryMode))
		return CIPHER_SUIT_TKIP_W_MIC;
	else if (IS_CIPHER_CCMP128(encryMode))
		return CIPHER_SUIT_CCMP_W_MIC;
	else if (IS_CIPHER_CCMP256(encryMode))
		return CIPHER_SUIT_CCMP_256;
	else if (IS_CIPHER_GCMP128(encryMode))
		return CIPHER_SUIT_GCMP_128;
	else if (IS_CIPHER_GCMP256(encryMode))
		return CIPHER_SUIT_GCMP_256;

	else
		return CIPHER_SUIT_NONE;
}


INT ParseWebKey(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  RTMP_STRING *buffer,
	IN  INT KeyIdx,
	IN  INT Keylength)
{
	UINT32 KeyLen = Keylength;
	SEC_KEY_INFO *pWebKey = &pSecConfig->WepKey[KeyIdx];
	UINT32 i = 0;

	if (KeyLen == 0)
		KeyLen = strlen(buffer);

	switch (KeyLen) {
	case 5: /*wep 40 Ascii type*/
	case 13: /*wep 104 Ascii type*/
	case 16: /*wep 128 Ascii type*/
		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen;
		NdisMoveMemory(pWebKey->Key, buffer, KeyLen);
		break;

	case 10: /*wep 40 Hex type*/
	case 26: /*wep 104 Hex type*/
	case 32: /*wep 128 Hex type*/
		for (i = 0; i < KeyLen; i++) {
			if (!isxdigit(*(buffer + i)))
				return FALSE;  /*Not Hex value;*/
		}

		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen / 2;
		AtoH(buffer, pWebKey->Key, pWebKey->KeyLen);
		pWebKey->Key[pWebKey->KeyLen] = '\0';
		break;

	default: /*Invalid argument */
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::(keyIdx=%d):Invalid argument (arg=%s)\n",
				 __func__, KeyIdx, buffer);
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s::(KeyIdx=%d, Alg=0x%x)\n",
			 __func__, KeyIdx, pSecConfig->PairwiseCipher);
	return TRUE;
}


#ifdef DOT1X_SUPPORT
INT SetWdevOwnIPAddr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING *arg)
{
	UINT32 ip_addr;

	if (rtinet_aton(arg, &ip_addr)) {
		pSecConfig->own_ip_addr = ip_addr;
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "own_ip_addr=%s(%x)\n", arg, pSecConfig->own_ip_addr);
	}

	return TRUE;
}

VOID ReadRadiusParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING tok_str[16], *macptr;
	UINT32 ip_addr;
	INT i = 0;
	BOOLEAN bUsePrevFormat = FALSE;
	UINT offset;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	int ret;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
#ifdef RADIUS_ACCOUNTING_SUPPORT
	BOOLEAN				bAcctUsePrevFormat = FALSE;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
	PUCHAR count;
	UCHAR srv_idx = 0;
	os_alloc_mem(NULL, (UCHAR **)&count, MAX_MBSSID_NUM(pAd));
	os_zero_mem(count, MAX_MBSSID_NUM(pAd));

	/* own_ip_addr*/
	if (RTMPGetKeyParameter("own_ip_addr", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx);
				SetWdevOwnIPAddr(pSecConfig, macptr);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
				/*
				* own_ip_addr is global setting , don't need to merge in dbdc multi profile,
				* in this point, let all bss set the same own_ip_addr for safe
				*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->own_ip_addr = pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_ip_addr;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("own_radius_port", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";");
				 (macptr && apidx < pAd->ApCfg.BssidNum);
				 macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"I/F(%s%d) ==> ", INF_MBSSID_DEV_NAME, apidx);


				pSecConfig->own_radius_port = simple_strtol(macptr, 0, 10);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
				/*
				*	own_radius_port is global setting , let all bss set the same own_radius_port
				*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->own_radius_port =
						pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_radius_port;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* session_timeout_interval*/
	if (RTMPGetKeyParameter("session_timeout_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->session_timeout_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> session_timeout_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval);
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->session_timeout_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.session_timeout_interval;
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> session_timeout_interval=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* quiet_interval */
	if (RTMPGetKeyParameter("quiet_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->quiet_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> quiet_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval);
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
					pSecConfig->quiet_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.quiet_interval;
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> quiet_interval=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* EAPifname*/
	if (RTMPGetKeyParameter("EAPifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->EAPifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->EAPifname, macptr, strlen(macptr));
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> EAPifname=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->EAPifname, pSecConfig->EAPifname_len);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuthifname*/
	if (RTMPGetKeyParameter("PreAuthifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->PreAuthifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->PreAuthifname, macptr, strlen(macptr));
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> PreAuthifname=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuth */
	if (RTMPGetKeyParameter("PreAuth", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->PreAuth = TRUE;
				else /*Disable*/
					pSecConfig->PreAuth = FALSE;

				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> PreAuth=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuth);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* IEEE8021X */
	if (RTMPGetKeyParameter("IEEE8021X", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->IEEE8021X = TRUE;
				else /*Disable*/
					pSecConfig->IEEE8021X = FALSE;

				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> IEEE8021X=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->IEEE8021X);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT

	/*radius_request_cui
	if(RTMPGetKeyParameter("radius_request_cui", tmpbuf, 32, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if(os_str_tol(macptr, 0, 10) != 0)
				pAd->ApCfg.MBSSID[i].radius_request_cui = TRUE;
			else
				pAd->ApCfg.MBSSID[i].radius_request_cui = FALSE;

			DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d), radius_request_cui=%d\n", i, pAd->ApCfg.MBSSID[i].radius_request_cui));
		}
	}*/
	/*radius_acct_authentic*/
	if (RTMPGetKeyParameter("radius_acct_authentic", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->radius_acct_authentic = os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_acct_authentic=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_authentic);
			}
		}
	}

	/*acct_interim_interval*/
	if (RTMPGetKeyParameter("acct_interim_interval", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_interim_interval = os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> acct_interim_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_interim_interval);
			}
		}
	}

	/*acct_enable*/
	if (RTMPGetKeyParameter("acct_enable", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_enable = os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> acct_enable=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_enable);
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
	/* RADIUS_Server */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_srv_info[pSecConfig->radius_srv_num].radius_ip = ip_addr;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_ip(seq-%d)=%s\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_srv_num, macptr);
				pSecConfig->radius_srv_num++;
			}
		}
	}
	/* RADIUS_Port */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			if (count[apidx] < pSecConfig->radius_srv_num) {
				srv_idx = count[apidx];
				pSecConfig->radius_srv_info[srv_idx].radius_port = (UINT32) os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_port(seq-%d)=%d\n",
					INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_srv_info[0].radius_port);
				count[apidx]++;
			}
		}
	}
	os_free_mem(count);

	/* RADIUS_Key  */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > pAd->ApCfg.BssidNum)
			bUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[0];

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len);
			}
		}
	}

	if (!bUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "RADIUS_Key%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str snprintf error!\n");

			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			pSecConfig = &wdev->SecConfig;
			srv_idx = 0;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					if (srv_idx < pSecConfig->radius_srv_num) {
						RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[srv_idx];

						p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
						NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
							INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len);
						srv_idx++;
					}
				}
			}
		}
	}

	/* NasIdX, X indicate the interface index(1~8) */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
		pSecConfig = &wdev->SecConfig;
		ret = snprintf(tok_str, sizeof(tok_str), "NasId%d", i + 1);
		if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str2 snprintf error!\n");

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
			if (strlen(tmpbuf) > 0) {
				pSecConfig->NasIdLen = strlen(tmpbuf) > IFNAMSIZ ? IFNAMSIZ : strlen(tmpbuf);
				NdisMoveMemory(pSecConfig->NasId, tmpbuf, pSecConfig->NasIdLen);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> NAS-ID=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, i, pSecConfig->NasId, pSecConfig->NasIdLen);
			}
		}
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT
	/* RADIUS_Acct_Server*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_acct_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_acct_srv_info[pSecConfig->radius_acct_srv_num].radius_ip = ip_addr;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_acct_ip(seq-%d)=%s\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_srv_num, macptr);
				pSecConfig->radius_acct_srv_num++;
			}
		}
	}

	/* RADIUS_Acct_Port*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;
			pSecConfig->radius_acct_srv_info[0].radius_port = (UINT32) os_str_tol(macptr, 0, 10);	/* TODO: idx */
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_acct_port(seq-%d)=%d\n",
					 INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_acct_srv_info[0].radius_port);
		}
	}

	/* RADIUS_Key*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > 0)
			bAcctUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];	/* TODO: idx */

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len);
			}
		}
	}

	if (!bAcctUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "RADIUS_Acct_Key%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str3 snprintf error!\n");
			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			pSecConfig = &wdev->SecConfig;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];		/* TODO: idx */

					p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
					NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len);
				}
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
}


#ifdef CONFIG_AP_SUPPORT
VOID Dot1xIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UCHAR apidx, srv_idx, keyidx, KeyLen = 0;
	UCHAR *mpool;
	PDOT1X_CMM_CONF pConf;
	struct _SECURITY_CONFIG *pSecConfigMain = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR main_apidx = (UCHAR) pObj->ioctl_if;
	UCHAR last_apidx = pAd->ApCfg.BssidNum - 1;


	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==>\n");

#ifdef MULTI_PROFILE

	if ((main_apidx == BSS0)
		&& (is_multi_profile_enable(pAd) == TRUE))
		last_apidx = multi_profile_get_pf1_num(pAd) - 1;

#endif

	if ((main_apidx > pAd->ApCfg.BssidNum - 1)
		|| (main_apidx > last_apidx)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid MBSSID index(%d)!\n",
			main_apidx);
		return;
	}

	pSecConfigMain = &pAd->ApCfg.MBSSID[main_apidx].wdev.SecConfig;
	/* Allocate memory */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, sizeof(DOT1X_CMM_CONF));

	if (mpool == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!!out of resource!!!\n");
		return;
	}

	NdisZeroMemory(mpool, sizeof(DOT1X_CMM_CONF));
	pConf = (PDOT1X_CMM_CONF)mpool;
	/* get MBSS number */
	pConf->mbss_num = (last_apidx - main_apidx + 1);
	/* get own ip address */
	pConf->own_ip_addr = pSecConfigMain->own_ip_addr;
	/* get own radius port */
	pConf->own_radius_port = pSecConfigMain->own_radius_port;
	/* get retry interval */
	pConf->retry_interval = pSecConfigMain->retry_interval;
	/* get session timeout interval */
	pConf->session_timeout_interval = pSecConfigMain->session_timeout_interval;
	/* Get the quiet interval */
	pConf->quiet_interval = pSecConfigMain->quiet_interval;

	for (apidx = main_apidx; apidx <= last_apidx; apidx++) {
		struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
		UCHAR apidx_locate = apidx - main_apidx;
		PDOT1X_BSS_INFO p1xBssInfo = &pConf->Dot1xBssInfo[apidx_locate];
#ifdef RADIUS_ACCOUNTING_SUPPORT
		PACCT_BSS_INFO pAcctBssInfo = &pConf->AcctBssInfo[apidx_locate];

		pAcctBssInfo->radius_srv_num = pSecConfig->radius_acct_srv_num;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->radius_srv_num = pSecConfig->radius_srv_num;

		/* prepare radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_srv_num; srv_idx++) {
			if (pSecConfig->radius_srv_info[srv_idx].radius_ip != 0) {
				p1xBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_srv_info[srv_idx].radius_ip;
				p1xBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_srv_info[srv_idx].radius_port;
				p1xBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_srv_info[srv_idx].radius_key_len > 0) {
					NdisMoveMemory(p1xBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT

		/* prepare accounting radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_acct_srv_num; srv_idx++) {
			if (pSecConfig->radius_acct_srv_info[srv_idx].radius_ip != 0) {
				pAcctBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_acct_srv_info[srv_idx].radius_ip;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_acct_srv_info[srv_idx].radius_port;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len > 0) {
					NdisMoveMemory(pAcctBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->ieee8021xWEP = (pSecConfig->IEEE8021X) ? 1 : 0;

		if (p1xBssInfo->ieee8021xWEP) {
			/* Default Key index, length and material */
			keyidx = pSecConfig->PairwiseKeyId;
			p1xBssInfo->key_index = keyidx;
			/* Determine if the key is valid. */
			KeyLen = pSecConfig->WepKey[keyidx].KeyLen;

			if (KeyLen == 5 || KeyLen == 13) {
				p1xBssInfo->key_length = KeyLen;
				NdisMoveMemory(p1xBssInfo->key_material, pSecConfig->WepKey[keyidx].Key, KeyLen);
			}
		}

		/* Get NAS-ID per BSS */
		if (pSecConfig->NasIdLen > 0) {
			p1xBssInfo->nasId_len = pSecConfig->NasIdLen;
			NdisMoveMemory(p1xBssInfo->nasId, pSecConfig->NasId, pSecConfig->NasIdLen);
		}

		/* get EAPifname */
		if (pSecConfig->EAPifname_len > 0) {
			pConf->EAPifname_len[apidx_locate] = pSecConfig->EAPifname_len;
			NdisMoveMemory(pConf->EAPifname[apidx_locate], pSecConfig->EAPifname, pSecConfig->EAPifname_len);
		}

		/* get PreAuthifname */
		if (pSecConfig->PreAuthifname_len > 0) {
			pConf->PreAuthifname_len[apidx_locate] = pSecConfig->PreAuthifname_len;
			NdisMoveMemory(pConf->PreAuthifname[apidx_locate], pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		/* pAcctBssInfo->radius_request_cui = (pSecConfig->radius_request_cui) ? 1 : 0; */
		pAcctBssInfo->radius_acct_authentic = pSecConfig->radius_acct_authentic;
		pAcctBssInfo->acct_interim_interval = pSecConfig->acct_interim_interval;
		pAcctBssInfo->acct_enable = pSecConfig->acct_enable;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
#ifdef RADIUS_MAC_ACL_SUPPORT
		/* Radius MAC Auth Config */
		pConf->RadiusAclEnable[apidx_locate] = pSecConfig->RadiusMacAuthCache.Policy;
		/* Radius MAC Auth Cache Timeout in 1XDaemon */
		pConf->AclCacheTimeout[apidx_locate] = pSecConfig->RadiusMacAuthCacheTimeout;
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}

	wrq->u.data.length = sizeof(DOT1X_CMM_CONF);

	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");

	os_free_mem(mpool);
}

VOID Dot1xIoctlRadiusData(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR *pPkt;

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d)\n", pObj->ioctl_if);

	if (pObj->ioctl_if > pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid MBSSID index(%d)!\n",
				 pObj->ioctl_if);
		return;
	}

	os_alloc_mem(pAd, (UCHAR **)&pPkt, wrq->u.data.length);
	if (pPkt) {
		if (copy_from_user(pPkt, wrq->u.data.pointer, wrq->u.data.length) == 0) {
			pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;

			if (IS_AKM_1X(pSecConfig->AKMMap)
					|| (pSecConfig->IEEE8021X == TRUE))
				WpaSend(pAd, (PUCHAR)pPkt, wrq->u.data.length);
		}
		os_free_mem(pPkt);
	}
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlAddWPAKey(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	NDIS_AP_802_11_KEY	*pKey;
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY		*pEntry;
	UCHAR				apidx;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d)\n", apidx);
	os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
	if (pKey == NULL)
		return;

	if (copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "copy from user failed\n");
		os_free_mem(pKey);
		return;
	}

	pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (IS_AKM_1X(pSecConfig->AKMMap)) {
		if ((pKey->KeyLength == 32) || (pKey->KeyLength == 64)) {
			UCHAR key_len = LEN_PMK;

			pEntry = MacTableLookup(pAd, pKey->addr);

			if (pEntry != NULL) {
				INT k_offset = 0;
#ifdef DOT11R_FT_SUPPORT

				/* The key shall be the second 256 bits of the MSK. */
				if (IS_FT_RSN_STA(pEntry) && pKey->KeyLength == 64)
					k_offset = 32;

#endif /* DOT11R_FT_SUPPORT */
				if (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (pKey->KeyLength == 64))
					key_len = LEN_PMK_SHA384;


#ifdef OCE_FILS_SUPPORT
				if (IS_AKM_FILS_SHA384(pEntry->SecConfig.AKMMap) && pKey->KeyLength == 64)
					key_len = LEN_PMK_SHA384;
#endif /* OCE_FILS_SUPPORT */

				NdisMoveMemory(pSecConfig->PMK, pKey->KeyMaterial + k_offset, key_len);
				hex_dump("PMK", pSecConfig->PMK, key_len);
			}
		}
	} else {	/* Old WEP stuff */
		ASIC_SEC_INFO Info = {0};

		if (pKey->KeyLength > 16) {
			os_free_mem(pKey);
			return;
		}
		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4) {
			/* For Group key setting */
			if (pKey->KeyIndex & 0x80000000) {
				UINT16 Wcid;
				/* Default key for tx (shared key) */
				pSecConfig->GroupKeyId = (UCHAR) KeyIdx;

				/* set key material and key length */
				if (pKey->KeyLength > 16) {
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "-IF(ra%d) : Key length too long %d\n", apidx, pKey->KeyLength);
					pKey->KeyLength = 16;
				}

				pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

				/* Set Ciper type */
				if (pKey->KeyLength == 5)
					SET_CIPHER_WEP40(pSecConfig->GroupCipher);
				else
					SET_CIPHER_WEP104(pSecConfig->GroupCipher);

				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(wdev, Wcid);
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_TX;
				Info.Wcid = Wcid;
				Info.BssIndex = apidx;
				Info.Cipher = pSecConfig->GroupCipher;
				Info.KeyIdx = pSecConfig->GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
			} else { /* For Pairwise key setting */
				STA_TR_ENTRY *tr_entry = NULL;

				pEntry = MacTableLookup(pAd, pKey->addr);

				if (pEntry) {
					pSecConfig = &pEntry->SecConfig;
					pSecConfig->PairwiseKeyId = (UCHAR) KeyIdx;
					/* set key material and key length */
					pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
					NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

					/* Set Ciper type */
					if (pKey->KeyLength == 5)
						SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
					else
						SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);

					/* Set key material to Asic */
					os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
					Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					Info.Direction = SEC_ASIC_KEY_BOTH;
					Info.Wcid = pEntry->wcid;
					Info.BssIndex = pEntry->func_tb_idx;
					Info.Cipher = pSecConfig->PairwiseCipher;
					Info.KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
					/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
					/* open 802.1x port control and privacy filter */
					tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pEntry, &Info);
				}
			}
		}
	}
	os_free_mem(pKey);
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlStaticWepCopy(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	MAC_TABLE_ENTRY  *pEntry;
	UCHAR MacAddr[MAC_ADDR_LEN] = {0};
	UCHAR apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx);

	if (wrq->u.data.length != sizeof(MacAddr)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlStaticWepCopy: the length isn't match (%d)\n", wrq->u.data.length);
		return;
	} else {
		UINT32 len;

		len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);
		pEntry = MacTableLookup(pAd, MacAddr);

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlStaticWepCopy: the mac address isn't match\n");
			return;
		} else {
			struct _SECURITY_CONFIG *pSecConfigEnrty = NULL;
			struct _SECURITY_CONFIG *pSecConfigProfile = NULL;
			STA_TR_ENTRY *tr_entry = NULL;
			ASIC_SEC_INFO Info = {0};

#ifdef OCE_FILS_SUPPORT
			if (IS_AKM_FILS(pEntry->SecConfig.AKMMap)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RTMPIoctlStaticWepCopy: skip for FILS\n");
				return;
			}
#endif /* OCE_FILS_SUPPORT */

			pSecConfigProfile = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
			pSecConfigEnrty = &pEntry->SecConfig;
			pSecConfigEnrty->PairwiseKeyId = pSecConfigProfile->PairwiseKeyId;
			pSecConfigEnrty->PairwiseCipher = pSecConfigProfile->PairwiseCipher;
			os_move_mem(&pSecConfigEnrty->WepKey, &pSecConfigProfile->WepKey, sizeof(SEC_KEY_INFO) * SEC_KEY_NUM);
			/* Set key material to Asic */
			os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
			Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
			Info.Direction = SEC_ASIC_KEY_BOTH;
			Info.Wcid = pEntry->wcid;
			Info.BssIndex = pEntry->func_tb_idx;
			Info.Cipher = pEntry->SecConfig.PairwiseCipher;
			Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
			os_move_mem(&Info.Key, &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId], sizeof(SEC_KEY_INFO));
			os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
			/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
			/* open 802.1x port control and privacy filter */
			tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			WifiSysUpdatePortSecur(pAd, pEntry, &Info);
		}
	}

	return;
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT1X_SUPPORT */


#ifdef APCLI_SUPPORT
VOID ReadApcliSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx;
	int ret;
#ifdef DBDC_MODE
	INT apcli_idx;
	RTMP_STRING tok_str[16];
#endif
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*ApCliAuthMode*/
	if (RTMPGetKeyParameter("ApCliAuthMode", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			SetWdevAuthMode(pSecConfig, macptr);
		}
	}

	/*ApCliEncrypType*/
	if (RTMPGetKeyParameter("ApCliEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

#ifdef DBDC_MODE

	for (apcli_idx = 0; apcli_idx < MAX_APCLI_NUM; apcli_idx++) {
		pSecConfig = &pAd->StaCfg[apcli_idx].wdev.SecConfig;

		if (apcli_idx == 0) {
			ret = snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK");
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str4 snprintf error!\n");
		} else {
			ret = snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK%d", apcli_idx);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str5 snprintf error!\n");
		}
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE)) {
			if (strlen(tmpbuf) < 65) {
				os_move_mem(pSecConfig->PSK, tmpbuf, strlen(tmpbuf));
				pSecConfig->PSK[strlen(tmpbuf)] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

#else

	/*ApCliWPAPSK*/
	if (RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, 255, pBuffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;

			if (strlen(macptr) < 65) {
				os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
				pSecConfig->PSK[strlen(macptr)] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

#endif

	/*ApCliDefaultKeyID*/
	if (RTMPGetKeyParameter("ApCliDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d)) ==> DefaultKeyId=%d\n",
					 i, pSecConfig->PairwiseKeyId);
		}
	}

	/*ApCliKeyXType, ApCliKeyXStr*/
	for (idx = 0; idx < 4; idx++) {
		RTMP_STRING tok_str[16];
		ULONG KeyType[MAX_APCLI_NUM];

		ret = snprintf(tok_str, sizeof(tok_str),	"ApCliKey%dType", idx + 1);
		if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str6 snprintf error!\n");

		/*ApCliKey1Type*/
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++)
				KeyType[i] = os_str_tol(macptr, 0, 10);

#ifdef DBDC_MODE
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < MAX_APCLI_NUM; i++) {
					if (i == 0) {
						ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx + 1);
						if (os_snprintf_error(sizeof(tok_str), ret))
							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"tok_str7 snprintf error!\n");
					} else {
						ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr%d", idx + 1, i);
						if (os_snprintf_error(sizeof(tok_str), ret))
							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"tok_str8 snprintf error!\n");
					}
					if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
						pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);
					}
				}
			}
#else
			ret = snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str9 snprintf error!\n");

			/*ApCliKey1Str*/
			if (RTMPGetKeyParameter(tok_str, tmpbuf, 512, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
					pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
					ParseWebKey(pSecConfig, macptr, idx, 0);
				}
			}

#endif
		}
	}

#ifdef DOT11_SAE_SUPPORT
	/*ApCliPweMethod*/
	if (RTMPGetKeyParameter("ApCliPweMethod", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig->sae_cap.gen_pwe_method = os_str_tol(macptr, 0, 10);
		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
	}

	/*ApCliSAEPK*/
	if (RTMPGetKeyParameter("ApCliSAEPK", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig->sae_cap.sae_pk_en = os_str_tol(macptr, 0, 10);
		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
	}

	/*ApCliSAEGroup*/
	if (RTMPGetKeyParameter("ApCliSAEGroup", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UCHAR group;
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			group = os_str_tol(macptr, 0, 10);
			if (group != 19 && group != 20 && group != 21)
				group = SAE_DEFAULT_GROUP;
			pAd->StaCfg[i].sae_cfg_group = group;
		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].sae_cfg_group = SAE_DEFAULT_GROUP;
	}
#endif
	/* ApCliTransDisableSupported */
	if (RTMPGetKeyParameter("ApCliTransDisableSupported", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==>", i);

			pAd->StaCfg[i].ApCliTransDisableSupported = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "i:%d ApCliTransDisableSupported :%d\n",
					i, pAd->StaCfg[i].ApCliTransDisableSupported);

			if (pAd->StaCfg[i].ApCliTransDisableSupported)
				NdisZeroMemory(&(pAd->StaCfg[i].ApCli_tti_bitmap), sizeof(struct transition_disable_bitmap));
		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].ApCliTransDisableSupported = 0;
	}
	/* ApCliOCVSupport */
	if (RTMPGetKeyParameter("ApCliOCVSupport", tmpbuf, 255, pBuffer, TRUE)) {

		RTMP_STRING *orig_tmpbuf;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;

			if (pSecConfig)
				pSecConfig->apcli_ocv_support = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
				DBG_LVL_INFO, "i:%d  pSecConfig->ocv_support:%d\n",
				i,  pSecConfig->apcli_ocv_support);

		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].wdev.SecConfig.apcli_ocv_support = 0;
	}
	/* ApCliPESupport */
	if (RTMPGetKeyParameter("ApCliPESupport", tmpbuf, 255, pBuffer, TRUE)) {

		RTMP_STRING *orig_tmpbuf;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;

			if (pSecConfig)
				pSecConfig->apcli_pe_support = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
				DBG_LVL_INFO, "i:%d  pSecConfig->apcli_pe_support:%d\n",
				i,  pSecConfig->apcli_pe_support);

		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			pAd->StaCfg[i].wdev.SecConfig.apcli_pe_support = 0;
			NdisZeroMemory(pAd->StaCfg[i].wdev.SecConfig.pe_latest_connected_macaddr, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->StaCfg[i].wdev.SecConfig.pe_latest_connected_Ssid, MAX_LEN_OF_SSID);
			pAd->StaCfg[i].wdev.SecConfig.pe_latest_connected_SsidLen = 0;
		}
	}
#ifdef BCN_PROTECTION_SUPPORT
	/* ApCliBcnProt Support */
	if (RTMPGetKeyParameter("ApCliBcnProt", tmpbuf, 255, pBuffer, TRUE)) {

		RTMP_STRING *orig_tmpbuf;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ==> ", i);
			pSecConfig = &pAd->StaCfg[i].wdev.SecConfig;

			if (pSecConfig)
				pSecConfig->apcli_bcnprot = os_str_tol(macptr, 0, 10);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
				DBG_LVL_INFO, "i:%d  pSecConfig->apcli_bcnprot:%d\n",
				i,  pSecConfig->apcli_bcnprot);

		}
	} else {
		for (i = 0; i < MAX_APCLI_NUM; i++)
			pAd->StaCfg[i].wdev.SecConfig.apcli_bcnprot = 0;
	}
#endif
}
INT Set_ApCli_Trans_Disable_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pTransDisable = NULL;
	UCHAR TransDisable = 0;
	UINT32 staidx = 0;

	if (strlen(arg) == 0)
	return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		 MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
			return FALSE;
	}

	staidx = pObj->ioctl_if;
	pTransDisable = &pAd->StaCfg[staidx].ApCliTransDisableSupported;

	TransDisable = os_str_tol(arg, 0, 10);

	*pTransDisable = (UCHAR) TransDisable;
	NdisZeroMemory(&(pAd->StaCfg[staidx].ApCli_tti_bitmap), sizeof(struct transition_disable_bitmap));
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[SAE] ApCliTransdisable is=%d \n",
				TransDisable);

	return TRUE;
}

#endif /* APCLI_SUPPORT */


#ifdef WDS_SUPPORT
VOID ReadWDSSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx;
	BOOLEAN	bUsePrevFormat = FALSE;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	int ret;
#ifdef DBDC_MODE
	INT band1_wds_start_index = 0;

	band1_wds_start_index = pAd->WdsTab.wds_num[DBDC_BAND0];
#endif
	/* WDS direct insert Key to Asic, not need do 4-way */
	/* WdsEncrypType */
	if (RTMPGetKeyParameter("WdsEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(wds%d) ==> ", i);
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

	/*WdsKey*/
	if (RTMPGetKeyParameter("WdsKey", tmpbuf, 255, pBuffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;

			if ((strlen(macptr) > 0) && (strlen(macptr) < 65)) {
				os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
				pSecConfig->PSK[strlen(macptr)] = '\0';
				bUsePrevFormat = TRUE;
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

	/*WdsDefaultKeyID*/
	if (RTMPGetKeyParameter("WdsDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			if(i == pAd->WdsTab.wds_num[DBDC_BAND0])
				i = MAX_WDS_PER_BAND;
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(wds%d)) ==> DefaultKeyId=%d\n",
					 i, pSecConfig->PairwiseKeyId);
		}
	}

	/*WdsXKey */
	if (bUsePrevFormat == FALSE) {
		i = 0;
		for (idx = 0; idx < MAX_WDS_PER_BAND; idx++) {
			RTMP_STRING tok_str[16];

			ret = snprintf(tok_str, sizeof(tok_str),	"Wds%dKey", idx);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str10 snprintf error!\n");

			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
#ifdef DBDC_MODE
				INT wds_entry_idx;
				if(strpbrk(tmpbuf, ";") == tmpbuf)
					i = MAX_WDS_PER_BAND;
				else
					i = 0;
				for (macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i += MAX_WDS_PER_BAND) {
					wds_entry_idx = i + idx;
					pSecConfig = &pAd->WdsTab.WdsEntry[wds_entry_idx].wdev.SecConfig;
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "I/F(wds%d)) ==> Key string=%s\n", wds_entry_idx, macptr);
#else
				for (macptr = rstrtok(tmpbuf, ";"); macptr && (i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
					pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"I/F(wds%d)) ==> Key string=%s\n", i, macptr);
#endif
							if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
								ParseWebKey(pSecConfig, macptr, pSecConfig->PairwiseKeyId, 0);
					else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
							 || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
						if (strlen(macptr) < 65) {
							os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
							pSecConfig->PSK[strlen(macptr)] = '\0';
						} else
							pSecConfig->PSK[0] = '\0';
					}
				}
			}
		}
	}
}
#endif /* WDS_SUPPORT */

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
UCHAR str_to_bin(
	IN RTMP_STRING *str,
	OUT UCHAR *bin,
	INOUT UINT32 *bin_sz)
{
	UINT32 i = 0;
	UCHAR v = 0;
	UINT32 len = 0;
	UINT32 max_len = *bin_sz;

	for (i = 0; str[i] != '\0' && len <= *bin_sz; i++) {
		if (str[i] >= 'a')
			v += str[i] - 'a' + 10;
		else
			v += str[i] - '0';

		if (i % 2 == 0)
			v <<= 4;
		else {
			bin[i / 2] = v;
			len++;
			v = 0;
		}
	}

	*bin_sz = len;

	return (len == max_len && str[i] != '\0') ? FALSE : TRUE;
}

VOID sae_pwd_id_deinit(IN PRTMP_ADAPTER pAd)
{
#ifdef CONFIG_AP_SUPPORT
	UINT i = 0;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		struct _SECURITY_CONFIG *sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
		struct pwd_id_list *pwd_id_ins = &sec_cfg->pwd_id_list_head;
		struct pwd_id_list *pwd_id_tmp = NULL;

		if (sec_cfg->pwd_id_cnt == 0)
			continue;
		while (DlListEmpty(&pwd_id_ins->list)) {
			pwd_id_tmp = DlListFirst(&pwd_id_ins->list, struct pwd_id_list, list);
			if (pwd_id_tmp != NULL) {/*ALPS05331068*/
				DlListDel(&pwd_id_tmp->list);
				os_free_mem(pwd_id_tmp);
			}
		}

		sec_cfg->pwd_id_cnt = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID insert_pwd_id(struct _SECURITY_CONFIG *sec_cfg, struct pwd_id_list *pwd_id_list_head, RTMP_STRING *arg)
{
	RTMP_STRING *pwdid;
	RTMP_STRING *pwd;
	ULONG len = 0;
	struct pwd_id_list *pwd_id_ins = NULL;

	if (arg == NULL)
		return;

	pwd = rstrtok(arg, ":");
	pwdid = rstrtok(NULL, ":");

	if (pwd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pwd\n");
		return;
	}
	if (pwdid == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pwdid\n");
		os_zero_mem(sec_cfg->PSK, sizeof(sec_cfg->PSK));
		os_move_mem(sec_cfg->PSK, pwd, strlen(pwd));
		sec_cfg->sae_cap.pwd_id_only = FALSE;
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&pwd_id_ins, sizeof(struct pwd_id_list));
	os_zero_mem(pwd_id_ins, sizeof(struct pwd_id_list));

	len = strlen(pwd);
	os_move_mem(pwd_id_ins->pwd, pwd, len);
	pwd_id_ins->pwd[len] = '\0';

	len = strlen(pwdid);
	os_move_mem(pwd_id_ins->pwd_id, pwdid, len);
	pwd_id_ins->pwd_id[len] = '\0';

	DlListAddTail(&pwd_id_list_head->list, &pwd_id_ins->list);
	sec_cfg->pwd_id_cnt++;
}

static VOID rm_new_line(
	RTMP_STRING * str)
{
	UINT32 i;
	UINT32 j;

	for (i = 0, j = 0; i < strlen(str); i++) {
		if (str[i] == '\n')
			continue;
		str[j++] = str[i];
	}
	str[j] = '\0';
}


static VOID read_sae_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
	int ret;

	if (RTMPGetKeyParameter("PweMethod", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwe_method = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwe_method = os_str_tol(macptr, 0, 10);

				if (pwe_method > MAX_PWE_METHOD) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"pwe method should not be %d",
						 pwe_method);
				}
				sec_cfg->sae_cap.gen_pwe_method = pwe_method;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwe_method = 0;

				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (pwe_method > MAX_PWE_METHOD) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"pwe method should not be %d",
						 pwe_method);
				}
				sec_cfg->sae_cap.gen_pwe_method = pwe_method;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
			}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				i = 0;
				while (i < MAX_MULTI_STA)
					pAd->StaCfg[i++].wdev.SecConfig.sae_cap.gen_pwe_method = PWE_MIXED;
			}
#endif /* CONFIG_STA_SUPPORT */
		}

	if (RTMPGetKeyParameter("PWDIDR", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwd_id_only = 0;
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwd_id_only = os_str_tol(macptr, 0, 10);
				sec_cfg->sae_cap.pwd_id_only = (pwd_id_only) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR pwd_id_only = 0;
				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					pwd_id_only = os_str_tol(macptr, 0, 10);
				sec_cfg->sae_cap.pwd_id_only = (pwd_id_only) ? TRUE : FALSE;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.pwd_id_only = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				i = 0;
				while (i < MAX_MULTI_STA)
					pAd->StaCfg[i++].wdev.SecConfig.sae_cap.pwd_id_only = FALSE;
			}
#endif /* CONFIG_STA_SUPPORT */
		}
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING *macptr2;
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "PWDID%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str11 snprintf error!\n");

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				DlListInit(&sec_cfg->pwd_id_list_head.list);
				macptr = tmpbuf;
				do {
					macptr = rstrtok(macptr, ";");
					macptr2 = rstrtok(NULL, "\0");

					insert_pwd_id(sec_cfg, &sec_cfg->pwd_id_list_head, macptr);
					macptr = macptr2;
				} while (macptr);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/

	if (RTMPGetKeyParameter("SAEPK", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_en = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					sae_pk_en = os_str_tol(macptr, 0, 10);

				if (sae_pk_en >= MAX_SAE_PK_EN) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"sae_pk_en should not be %d",
						 sae_pk_en);
				}
				sec_cfg->sae_cap.sae_pk_en = sae_pk_en;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_en = 0;

				sec_cfg = &pAd->StaCfg[i].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (sae_pk_en >= MAX_SAE_PK_EN) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"sae_pk_en should not be %d",
						 sae_pk_en);
				}
				sec_cfg->sae_cap.sae_pk_en = sae_pk_en;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			i = 0;
			while (i < MAX_MULTI_STA)
				pAd->StaCfg[i++].wdev.SecConfig.sae_cap.sae_pk_en = SAE_PK_DISABLE;
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKInputMode", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR key_input_mode = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					key_input_mode = os_str_tol(macptr, 0, 10);

				if (key_input_mode >= MAX_SAE_PK_KEY_INPUT_MODE) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"key_input_mode should not be %d",
						 key_input_mode);
				}
				sec_cfg->sae_cap.key_input_mode = key_input_mode;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
	} else {
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /*HOSTAPD_WPA3_SUPPORT*/
	}

#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[19];
		RTMP_STRING head_str[] = "-----BEGIN EC PRIVATE KEY-----";
		RTMP_STRING tail_str[] = "-----END EC PRIVATE KEY-----";

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKKeyFilePath%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str12 snprintf error!\n");

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				RTMP_OS_FD_EXT srcf;
				INT retval;
				ULONG buf_size = 500;
				RTMP_STRING *buffer = NULL;
				RTMP_STRING *ptr = NULL;
				UCHAR is_found = FALSE;
				UCHAR out[130] = {0};
				UINT32 out_len;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_FILE_EC_PRIVATE)
					continue;

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "Open file \"%s\"\n", tmpbuf);

				os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);

				if (!buffer)
					continue;

				srcf = os_file_open(tmpbuf, O_RDONLY, 0);

				if (srcf.Status)
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Open file \"%s\" failed!\n", tmpbuf);
				else {
					retval = os_file_read(srcf, buffer, buf_size - 1);

					if (retval > 0) {
						ptr = rtstrstr(buffer, tail_str);

						if (NdisEqualMemory(buffer, head_str, strlen(head_str)) &&
							ptr != NULL) {
							ptr[0] = '\0';
							ptr = buffer + strlen(head_str);
							rm_new_line(ptr);
							is_found = TRUE;
						}

					if (is_found && !sae_pk_pem_decode(&sec_cfg->sae_pk, ptr, out, &out_len))
								sec_cfg->sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;

						retval = NDIS_STATUS_SUCCESS;
					} else {
						sec_cfg->sae_cap.key_input_mode = SAE_PK_KEY_INPUT_MODE_HEX;
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Read file \"%s\" failed(errCode=%d)!\n", tmpbuf, retval);
					}

					if (os_file_close(srcf) != 0) {
						retval = NDIS_STATUS_FAILURE;
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "Close file \"%s\" failed(errCode=%d)!\n", tmpbuf, retval);
					}
				}

				os_free_mem(buffer);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKKey%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str13 snprintf error!\n");

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				UCHAR pri_key_bin[68];
				UINT32 pri_key_len = sizeof(pri_key_bin);

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_HEX)
					continue;

				str_to_bin(tmpbuf, pri_key_bin, &pri_key_len);

				if (pri_key_len != 0) {
					os_alloc_mem(NULL, (UCHAR **) &sec_cfg->sae_pk.fixed_pri_key, pri_key_len);
					os_move_mem(sec_cfg->sae_pk.fixed_pri_key, pri_key_bin, pri_key_len);
					sec_cfg->sae_pk.fixed_pri_key_len = pri_key_len;
				}

				hex_dump_with_cat_and_lvl("SAEPKKey", pri_key_bin, pri_key_len, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		RTMP_STRING tok_str[16];

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			ret = snprintf(tok_str, sizeof(tok_str), "SAEPKStartM%d", i + 1);
			if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str14 snprintf error!\n");

			if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
				UCHAR modifier[SAE_PK_MODIFIER_BYTES_LEN];
				UINT32 modifier_len = SAE_PK_MODIFIER_BYTES_LEN;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				str_to_bin(tmpbuf, modifier, &modifier_len);

				if (modifier_len == SAE_PK_MODIFIER_BYTES_LEN) {
					os_alloc_mem(NULL, (UCHAR **) &sec_cfg->sae_pk.fixed_start_modifier, SAE_PK_MODIFIER_BYTES_LEN);
					os_move_mem(sec_cfg->sae_pk.fixed_start_modifier, modifier, SAE_PK_MODIFIER_BYTES_LEN);
				}


				hex_dump_with_cat_and_lvl("SAEPKStartM", modifier, modifier_len, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("SAEPKSec", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.sec = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.sec = SAE_PK_AUTO_GEN_DEF_SEC;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKLambda", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.lambda = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.lambda = SAE_PK_AUTO_GEN_DEF_LAMBDA;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("SAEPKGroup", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;

				if (sec_cfg->sae_cap.key_input_mode != SAE_PK_KEY_INPUT_MODE_HEX)
					continue;

				sec_cfg->sae_pk.group_id = os_str_tol(macptr, 0, 10);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.group_id = SAE_DEFAULT_GROUP;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
	/* test or testbed behavior only */
	if (RTMPGetKeyParameter("SAEPKCfg", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR sae_pk_test = 0;

				sae_pk_test = os_str_tol(macptr, 0, 16);
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				sec_cfg->sae_pk.sae_pk_test_ctrl = sae_pk_test;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			i = 0;
			while (i < MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.sae_pk.sae_pk_test_ctrl = 0;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /*HOSTAPD_WPA3_SUPPORT*/
}
#endif

static VOID read_ocv_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("OCVSupport", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				UCHAR ocv_support = 0;

				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr)
					ocv_support = os_str_tol(macptr, 0, 10);

				sec_cfg->ocv_support = ocv_support;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.ocv_support = 0;
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}

static VOID read_td_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT i = 0;
	struct _SECURITY_CONFIG *sec_cfg = NULL;
	RTMP_STRING *macptr;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMPGetKeyParameter("TransitionDisable", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
				sec_cfg = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, i);
				if (macptr) {
					sec_cfg->td_value_fixed_en = TRUE;
					sec_cfg->td_value = os_str_tol(macptr, 0, 10);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				i = 0;
				while (i < MAX_MBSSID_NUM(pAd))
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i++)].wdev.SecConfig.td_value_fixed_en = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}


VOID ReadSecurityParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	int ret;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	INT staidx;
#endif /* CONFIG_STA_SUPPORT */
	INT idx;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*AuthMode*/
	if (RTMPGetKeyParameter("AuthMode", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx);
				SetWdevAuthMode(pSecConfig, macptr);
				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				wdev = &pAd->StaCfg[staidx].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, staidx);
				SetWdevAuthMode(pSecConfig, macptr);
				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/*EncrypType*/
	if (RTMPGetKeyParameter("EncrypType", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, staidx);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* Web DefaultKeyID */
	if (RTMPGetKeyParameter("DefaultKeyID", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.SecConfig;
				KeyIdx = os_str_tol(macptr, 0, 10);

				if ((KeyIdx >= 1) && (KeyIdx <= 4))
					pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
				else
					pSecConfig->PairwiseKeyId = 0;

				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d)) ==> DefaultKeyId=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PairwiseKeyId);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
				pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
				KeyIdx = os_str_tol(tmpbuf, 0, 10);

				if ((KeyIdx >= 1) && (KeyIdx <= 4))
					pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
				else
					pSecConfig->PairwiseKeyId = 0;

				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d)) ==> DefaultKeyId=%d\n",
						 INF_MBSSID_DEV_NAME, staidx, pSecConfig->PairwiseKeyId);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* KeyType, KeyStr for WEP  */
	for (idx = 0; idx < 4; idx++) {
		INT i = 0;
		RTMP_STRING tok_str[16];
		ULONG KeyType[MAX_BEACON_NUM];

		ret = snprintf(tok_str, sizeof(tok_str), "Key%dType", idx + 1);
		if (os_snprintf_error(sizeof(tok_str), ret))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"tok_str15 snprintf error!\n");

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
				if (i < MAX_MBSSID_NUM(pAd))
					KeyType[i] = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				BOOLEAN bKeyxStryIsUsed = FALSE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					ret = snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", idx + 1, i + 1);
					if (os_snprintf_error(sizeof(tok_str), ret))
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"tok_str16 snprintf error!\n");
					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);

						if (bKeyxStryIsUsed == FALSE)
							bKeyxStryIsUsed = TRUE;
					}
				}

				if (bKeyxStryIsUsed == FALSE) {
					ret = snprintf(tok_str, sizeof(tok_str), "Key%dStr", idx + 1);
					if (os_snprintf_error(sizeof(tok_str), ret))
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"tok_str17 snprintf error!\n");

					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						if (pAd->ApCfg.BssidNum == 1) {
							pSecConfig = &pAd->ApCfg.MBSSID[BSS0].wdev.SecConfig;
							ParseWebKey(pSecConfig, tmpbuf, idx, 0);
						} else {
							/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								pSecConfig = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.SecConfig;
								ParseWebKey(pSecConfig, macptr, idx, 0);
							}
						}
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				BOOLEAN bKeyxStryIsUsed = FALSE;

				for (staidx = 0; staidx < MAX_MULTI_STA; staidx++) {
					ret = snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", idx + 1, i + 1);
					if (os_snprintf_error(sizeof(tok_str), ret))
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"tok_str18 snprintf error!\n");
					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);

						if (bKeyxStryIsUsed == FALSE)
							bKeyxStryIsUsed = TRUE;
					}
				}

				if (bKeyxStryIsUsed == FALSE) {
					ret = snprintf(tok_str, sizeof(tok_str), "Key%dStr", idx + 1);
					if (os_snprintf_error(sizeof(tok_str), ret))
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"tok_str19 snprintf error!\n");
					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						if (pAd->MSTANum == 1) {
							pSecConfig = &pAd->StaCfg[MAIN_MSTA_ID].wdev.SecConfig;
							ParseWebKey(pSecConfig, tmpbuf, idx, 0);
						} else {
							/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							for (staidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && staidx < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), staidx++) {
								pSecConfig = &pAd->StaCfg[staidx].wdev.SecConfig;
								ParseWebKey(pSecConfig, macptr, idx, 0);
							}
						}
					}
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
	}

	ReadWPAParameterFromFile(pAd, tmpbuf, pBuffer);
#ifdef DOT11_SAE_SUPPORT
	read_sae_parma_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef DOT1X_SUPPORT
	ReadRadiusParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT1X_SUPPORT */
#ifdef APCLI_SUPPORT
	ReadApcliSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	ReadWDSSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	rtmp_read_pmf_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef BCN_PROTECTION_SUPPORT
	read_bcn_prot_parma_from_file(pAd, tmpbuf, pBuffer);
#endif
	read_ocv_parma_from_file(pAd, tmpbuf, pBuffer);
	read_td_parma_from_file(pAd, tmpbuf, pBuffer);
}


INT32 fill_wtbl_key_info_struc(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_T * rWtblSecurityKey)
{
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
		rWtblSecurityKey->ucKeyLen = sizeof(rWtblSecurityKey->aucKeyMaterial);
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		rWtblSecurityKey->ucKeyId = pInfo->KeyIdx;

		if (pSecKey->KeyLen > sizeof(rWtblSecurityKey->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			rWtblSecurityKey->ucKeyLen = pSecKey->KeyLen;

		os_move_mem(rWtblSecurityKey->aucKeyMaterial, pSecKey->Key, rWtblSecurityKey->ucKeyLen);

		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (rWtblSecurityKey->ucKeyLen == LEN_WEP40)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_40;
			else if (rWtblSecurityKey->ucKeyLen == LEN_WEP104)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_104;
			else if (rWtblSecurityKey->ucKeyLen == LEN_WEP128)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_TKIP_W_MIC;
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_W_MIC;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_256;

		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support Cipher[0x%x]\n",
					 pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_TX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 0;
			rWtblSecurityKey->ucIkv = 0;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_RX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 1;

			if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher)
				|| ((IS_CIPHER_CCMP128(pInfo->Cipher)) && (rWtblSecurityKey->ucKeyLen == 32)))
				rWtblSecurityKey->ucIkv = 1;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


#ifdef WIFI_UNIFIED_COMMAND
INT32 fill_uni_cmd_wtbl_key_info_struc_v2(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT P_CMD_WTBL_SECURITY_KEY_V2_T rWtblSecurityKey)
{
	rWtblSecurityKey->ucEntryCount = 0;
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_cipher = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;
		UCHAR is_igtk_exist = FALSE;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_cipher->ucKeyIdx = pInfo->KeyIdx;
		wtbl_cipher->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);

		if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher) ||
			(IS_CIPHER_CCMP128(pInfo->Cipher) && pSecKey->KeyLen == 32)) {
			is_igtk_exist = TRUE;
			pSecKey->KeyLen = LEN_CCMP128_TK;
		}

		if (pSecKey->KeyLen > sizeof(wtbl_cipher->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			wtbl_cipher->ucKeyLength = pSecKey->KeyLen;

		os_move_mem(wtbl_cipher->aucKeyMaterial, pSecKey->Key, wtbl_cipher->ucKeyLength);

		hex_dump_with_cat_and_lvl("install key:", wtbl_cipher->aucKeyMaterial, wtbl_cipher->ucKeyLength,
						DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO);

		if ((pInfo->Cipher)) {
			if (pSecKey->KeyLen == LEN_WEP40)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP40;
			else if (pSecKey->KeyLen == LEN_WEP104)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP104;
			else if (pSecKey->KeyLen == LEN_WEP128)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_TKIP;
			os_move_mem(&wtbl_cipher->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&wtbl_cipher->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP128;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP256;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support Cipher[0x%x]\n",
					 pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: ucCipherId = %d, ucKeyIdx = %d\n", __func__, wtbl_cipher->ucCipherId, wtbl_cipher->ucKeyIdx);

		/* TODO: Check following values */
		wtbl_cipher->u2WlanIndex = cpu2le16(0);
		wtbl_cipher->ucMgmtProtection = 0;
		wtbl_cipher->fgNeedRsp = 0;

		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_cipher->ucSubLength;

		if (is_igtk_exist) {
			UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_bip = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;

			wtbl_bip->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->IGTKKeyLen;
			wtbl_bip->ucKeyIdx = pInfo->igtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->IGTK, pInfo->IGTKKeyLen);
			hex_dump_with_cat_and_lvl("install igtk key:", wtbl_bip->aucKeyMaterial, pInfo->IGTKKeyLen,
							DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO);

			/* TODO: Check following values */
			wtbl_cipher->u2WlanIndex = cpu2le16(0);
			wtbl_cipher->ucMgmtProtection = 0;
			wtbl_cipher->fgNeedRsp = 0;


			rWtblSecurityKey->ucEntryCount++;

			buf += wtbl_bip->ucSubLength;
		}

		if (pInfo->bigtk_key_len) {
			UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_bip = (UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;

			wtbl_bip->ucSubLength = sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->bigtk_key_len;
			wtbl_bip->ucKeyIdx = pInfo->bigtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->bigtk, pInfo->bigtk_key_len);
			hex_dump_with_cat_and_lvl("install bigtk key:", wtbl_bip->aucKeyMaterial, pInfo->bigtk_key_len,
							DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_INFO);

			/* TODO: Check following values */
			wtbl_cipher->u2WlanIndex = cpu2le16(0);
			wtbl_cipher->ucMgmtProtection = 0;
			wtbl_cipher->fgNeedRsp = 0;

			rWtblSecurityKey->ucEntryCount++;
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_UNIFIED_COMMAND */

INT32 fill_wtbl_key_info_struc_v2(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_V2_T * rWtblSecurityKey)
{
	rWtblSecurityKey->ucEntryCount = 0;
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_REMOVE_KEY_OP;
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;
		UINT8 *buf = rWtblSecurityKey->aucBuffer;
		CMD_WTBL_SEC_CIPHER_GENERAL_T *wtbl_cipher = (CMD_WTBL_SEC_CIPHER_GENERAL_T *)buf;
		UCHAR is_igtk_exist = FALSE;

		rWtblSecurityKey->ucAddRemove = CMD_SEC_KEY_ADD_KEY_OP;
		wtbl_cipher->ucKeyIdx = pInfo->KeyIdx;
		wtbl_cipher->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_GENERAL_T);

		if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher) ||
			(IS_CIPHER_CCMP128(pInfo->Cipher) && pSecKey->KeyLen == 32)) {
			is_igtk_exist = TRUE;
			pSecKey->KeyLen = LEN_CCMP128_TK;
		}

		if (pSecKey->KeyLen > sizeof(wtbl_cipher->aucKeyMaterial)) {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"KeyLen is larger than the aucKeyMaterial\n");
			return NDIS_STATUS_FAILURE;
		} else
			wtbl_cipher->ucKeyLength = pSecKey->KeyLen;

		os_move_mem(wtbl_cipher->aucKeyMaterial, pSecKey->Key, wtbl_cipher->ucKeyLength);

		hex_dump_with_cat_and_lvl("install key:", wtbl_cipher->aucKeyMaterial, wtbl_cipher->ucKeyLength,
						DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO);

		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (pSecKey->KeyLen == LEN_WEP40)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP40;
			else if (pSecKey->KeyLen == LEN_WEP104)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP104;
			else if (pSecKey->KeyLen == LEN_WEP128)
				wtbl_cipher->ucCipherId = SEC_CIPHER_ID_WEP128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_TKIP;
			os_move_mem(&wtbl_cipher->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&wtbl_cipher->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP128;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_CCMP256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			wtbl_cipher->ucCipherId = SEC_CIPHER_ID_GCMP256;
		else {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Not support Cipher[0x%x]\n",
					 pInfo->Cipher);
			return NDIS_STATUS_FAILURE;
		}

		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: ucCipherId = %d, ucKeyIdx = %d\n", __func__, wtbl_cipher->ucCipherId, wtbl_cipher->ucKeyIdx);

		rWtblSecurityKey->ucEntryCount++;
		buf += wtbl_cipher->ucSubLength;

		if (is_igtk_exist) {
			CMD_WTBL_SEC_CIPHER_BIP_T *wtbl_bip = (CMD_WTBL_SEC_CIPHER_BIP_T *)buf;

			wtbl_bip->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->IGTKKeyLen;
			wtbl_bip->ucKeyIdx = pInfo->igtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->IGTK, pInfo->IGTKKeyLen);
			hex_dump_with_cat_and_lvl("install igtk key:", wtbl_bip->aucKeyMaterial, pInfo->IGTKKeyLen,
							DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO);
			rWtblSecurityKey->ucEntryCount++;

			buf += wtbl_bip->ucSubLength;
		}

		if (pInfo->bigtk_key_len) {
			CMD_WTBL_SEC_CIPHER_BIP_T *wtbl_bip = (CMD_WTBL_SEC_CIPHER_BIP_T *)buf;

			wtbl_bip->ucSubLength = sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);
			wtbl_bip->ucCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
			wtbl_bip->ucKeyLength = pInfo->bigtk_key_len;
			wtbl_bip->ucKeyIdx = pInfo->bigtk_key_idx;
			os_move_mem(wtbl_bip->aucKeyMaterial, pInfo->bigtk, pInfo->bigtk_key_len);
			hex_dump_with_cat_and_lvl("install bigtk key:", wtbl_bip->aucKeyMaterial, pInfo->bigtk_key_len,
							DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_INFO);
			rWtblSecurityKey->ucEntryCount++;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

VOID process_pmkid(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *entry,
	INT CacheIdx)
{
	if (CacheIdx != INVALID_PMKID_IDX) {
		/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
		if ((entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
			)
#ifdef WSC_AP_SUPPORT
			&& !entry->bWscCapable
#endif /* WSC_AP_SUPPORT */
			) {
			/* Enqueue a EAPOL-start message with the pEntry */
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
			entry->SecConfig.Handshake.WpaState = AS_INITPSK;
			os_move_mem(&entry->SecConfig.Handshake.AAddr,
				wdev->bssid,
				MAC_ADDR_LEN);
			os_move_mem(&entry->SecConfig.Handshake.SAddr,
				entry->Addr,
				MAC_ADDR_LEN);
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer,
				ENQUEUE_EAPOL_START_TIMER);
		}

		store_pmkid_cache_in_sec_config(pAd, entry, CacheIdx);

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"ASSOC - 2.PMKID matched and start key cache algorithm\n");
	} else {
		store_pmkid_cache_in_sec_config(pAd, entry, INVALID_PMKID_IDX);
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "ASSOC - 2.PMKID not found\n");

		/* Enqueue a EAPOL-start message to trigger EAP SM */
		if (entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
		) {
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
		}
	}
}

VOID store_pmkid_cache_in_sec_config(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT32 cache_idx)
{
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pEntry is null\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "EntryType = %d\n", pEntry->EntryType);

	if (cache_idx == INVALID_PMKID_IDX) {
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
	} else {
		if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
				pEntry->SecConfig.pmkid = pAd->ApCfg.PMKIDCache.BSSIDInfo[cache_idx].PMKID;
				pEntry->SecConfig.pmk_cache = pAd->ApCfg.PMKIDCache.BSSIDInfo[cache_idx].PMK;
#endif
#ifdef CONFIG_STA_SUPPORT
			{
				PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

				if (pStaCfg) {
					pEntry->SecConfig.pmkid = pStaCfg->SavedPMK[cache_idx].PMKID;
					pEntry->SecConfig.pmk_cache = pStaCfg->SavedPMK[cache_idx].PMK;
				} else
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"pStaCfg is null\n");
			}
#endif
		}
	}
}

UCHAR is_pmkid_cache_in_sec_config(
	IN struct _SECURITY_CONFIG *pSecConfig)
{
	if (pSecConfig && pSecConfig->pmkid && pSecConfig->pmk_cache)
		return TRUE;
	else
		return FALSE;
}

/* input: wdev->SecConfig */
#ifdef HOSTAPD_WPA3R3_SUPPORT
INT build_rsnxe_ie(
	IN struct wifi_dev *wdev,
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *buf)
#else
INT build_rsnxe_ie(
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *buf)
#endif
{
	INT extend_length = 0;
	UCHAR ie = IE_RSNXE;
	UCHAR ie_len = 1;
	UCHAR cap = 0;

#ifdef HOSTAPD_WPA3R3_SUPPORT
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev is NULL\n");
		return 0;
	}
	/* Add RSNXE capability only for interface operating in AP mode
	 * This capability is received from hostapd */
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		cap = sec_cfg->RSNXE_Val;
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s :RSNXE_Val:%d\n", __func__, sec_cfg->RSNXE_Val);
	} else {
#endif
	/* remove it if any other authmode also use rsnxe */
	if (!IS_AKM_SAE(sec_cfg->AKMMap))
		return 0;

#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(sec_cfg->AKMMap) &&
		sec_cfg->sae_cap.gen_pwe_method != PWE_LOOPING_ONLY)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_H2E);

	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_cap.sae_pk_en != SAE_PK_DISABLE)
		cap |= (1 << IE_RSNXE_CAPAB_SAE_PK);
#endif /* DOT11_SAE_SUPPORT */
#ifdef HOSTAPD_WPA3R3_SUPPORT
	}
#endif

	if (cap == 0)
		return 0;

	if (buf == NULL)
		return sizeof(ie) + sizeof(ie_len) + sizeof(cap);

	NdisMoveMemory(buf + extend_length, &ie, sizeof(ie));
	extend_length += sizeof(ie);
	NdisMoveMemory(buf + extend_length, &ie_len, sizeof(ie_len));
	extend_length += sizeof(ie_len);
	NdisMoveMemory(buf + extend_length, &cap, sizeof(cap));
	extend_length += sizeof(cap);

	return extend_length;
}


/* input: pEntry->SecConfig */
UINT parse_rsnxe_ie(
	IN struct _SECURITY_CONFIG *sec_cfg,
	IN UCHAR *rsnxe_ie,
	IN UCHAR rsnxe_ie_len,
	IN UCHAR need_copy)
{
#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_conn_type) {
		if (rsnxe_ie[1] == 0 || !(rsnxe_ie[2] & (1 << IE_RSNXE_CAPAB_SAE_H2E))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"IE_RSNXE_CAPAB_SAE_H2E should not be 0\n");
			return MLME_UNSPECIFY_FAIL;
		}
	}

	if (IS_AKM_SAE(sec_cfg->AKMMap) && sec_cfg->sae_conn_type == SAE_CONNECTION_TYPE_SAEPK) {
		if (rsnxe_ie[1] == 0 || !(rsnxe_ie[2] & (1 << IE_RSNXE_CAPAB_SAE_PK))) {
			MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"IE_RSNXE_CAPAB_SAE_PK should not be 0\n");
			return MLME_UNSPECIFY_FAIL;
		}
	}

	if (need_copy) {
		NdisMoveMemory(sec_cfg->rsnxe_content, rsnxe_ie, rsnxe_ie_len);
		sec_cfg->rsnxe_len = rsnxe_ie_len;
	} else if (sec_cfg->rsnxe_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"fail due to sec_cfg->rsnxe_len == 0\n");
		return MLME_UNSPECIFY_FAIL;
	} else if (NdisCmpMemory(sec_cfg->rsnxe_content, rsnxe_ie, rsnxe_ie_len) != 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"rsnxe compare fail\n");
		return MLME_UNSPECIFY_FAIL;
	}
#endif
	return MLME_SUCCESS;
}

/* not support oct now */
VOID build_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	OUT UCHAR *buf,
	OUT ULONG *buf_len)
{
	buf[0] = get_regulatory_class(ad, wdev->channel, wdev->PhyMode, wdev); /* Operating Class */
	buf[1] = (wpa3_test_ctrl == 12) ? wpa3_test_ctrl2 : wdev->channel; /*wlan_operate_get_prim_ch(wdev)*/ /* Primary Channel Number */
	buf[2] = wlan_operate_get_cen_ch_2(wdev); /* Frequency Segment 1 Channel Number */

	*buf_len = 3;

	if (oct_support)
		; /* not support now */
}

VOID build_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	OUT UCHAR *buf,
	OUT ULONG *buf_len)
{
	ULONG len;

	if (wdev == NULL)
		return;

	buf[0] = IE_WLAN_EXTENSION;
	buf[2] = EID_EXT_OCI;

	build_oci_common_field(ad, wdev, TRUE, buf + 3, &len);
	buf[1] = len + 1;
	*buf_len = len + 3;

	hex_dump_with_cat_and_lvl("oci ie", buf,
		*buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO); /* todo: change to trace*/
}

UCHAR parse_oci_common_field(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR oct_support,
	IN UCHAR *buf,
	IN ULONG buf_len)
{
	UCHAR bw = wlan_operate_get_bw(wdev);
	UCHAR spacing;

	hex_dump_with_cat_and_lvl("peer oci", buf, buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_INFO);

	if (buf_len < 3) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"invalid oci len(%lu)\n",
			buf_len);
		return FALSE;
	}

	if (!is_channel_in_channelset_by_reg_class(ad, buf[0], wdev->PhyMode, wdev->channel)) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"operating class(%d) check fail, chanel isn't in channel list\n",
			buf[0]);
		return FALSE;
	}

	if (get_spacing_by_reg_class(ad, buf[0], wdev->PhyMode, &spacing) == FALSE) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"get reg_class table fail(op class = %d)\n",
			buf[0]);
		return FALSE;
	}

	if (bw > spacing) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"bandwidth mismatch (peer: %d, our: %d)\n",
			spacing, bw);
		return FALSE;
	}

	if (buf[1] != wdev->channel) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"primary ch(%d) check fail, primary ch(%d)\n",
			buf[1], wdev->channel);
		return FALSE;
	}

	if (buf[2] != wlan_operate_get_cen_ch_2(wdev)) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"second central ch(%d) check fail, second central ch(%d)\n",
			buf[2], wlan_operate_get_cen_ch_2(wdev));
		return FALSE;
	}

	if (!oct_support && buf_len > 3) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
			"oci field len is wrong\n");
		return FALSE;
	}

	return TRUE;
}

UCHAR parse_oci_ie(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN UCHAR *buf,
	IN ULONG buf_len)
{

	if (buf_len < MIN_OCI_LEN) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR,
				"buf_len(%lu) is less than min oci len\n", buf_len);
		return FALSE;
	}

	if (buf[0] != IE_WLAN_EXTENSION ||
		buf[2] != EID_EXT_OCI) {
		hex_dump_with_cat_and_lvl("oci_ie fail, peer oci ie", buf,
			buf_len, DBG_CAT_SEC, CATSEC_OCV, DBG_LVL_ERROR);
		return FALSE;
	}

	return parse_oci_common_field(ad, wdev, TRUE, buf + 3, buf[1] - 1);
}


