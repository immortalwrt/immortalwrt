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

extern UCHAR SES_OUI[];
extern UCHAR OUI_WPA[];
extern UCHAR OUI_WPA_VERSION[];
extern UCHAR OUI_WPA2_CIPHER[];

INT Show_STASecurityInfo_Proc(
	IN	PRTMP_ADAPTER pAd,
	OUT RTMP_STRING * pBuf,
	IN	ULONG BufLen)
{
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	MTWF_PRINT("Security Infomation:\n");
	MTWF_PRINT("AuthMode\tPairwiseCipher\n");
	MTWF_PRINT("0x%x\t\t0x%x\n",
			 GET_SEC_AKM(pSecConfig), GET_PAIRWISE_CIPHER(pSecConfig));

	if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher)) {
		BOOLEAN IsASCII = TRUE;
		INT i, j = 0;
		PSEC_KEY_INFO pWepKey;

		MTWF_PRINT("Key ID=%d\n", pSecConfig->PairwiseKeyId);

		for (i = 0; i < 4; i++) {
			pWepKey = &pSecConfig->WepKey[i];
			MTWF_PRINT("Key%d", (i + 1));

			for (j = 0; j < pWepKey->KeyLen; j++) {
				if ((pWepKey->Key[j] < 0x20) || (pWepKey->Key[j] > 0x7E)) {
					IsASCII = FALSE;
					break;
				}
			}

			if (IsASCII == TRUE)
				MTWF_PRINT("%s", pWepKey->Key);
			else {
				int idx;

				for (idx = 0; idx < pWepKey->KeyLen; idx++)
					MTWF_PRINT("%02X", pWepKey->Key[idx]);
			}

			MTWF_PRINT("\n");
		}
	} else {
		int idx;

		MTWF_PRINT("PMK:");

		for (idx = 0; idx < 32; idx++)
			MTWF_PRINT("%02X", pSecConfig->PMK[idx]);

		MTWF_PRINT("\n");
	}

	return TRUE;
}


VOID PaserSecurityIE(
	IN BCN_IE_LIST * ie_list,
	IN USHORT * LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE,
	OUT UINT32 * AKMMap,
	OUT UINT32 * PairwiseCipher,
	OUT UINT32 * GroupCipher,
#ifdef DOT11W_PMF_SUPPORT
	OUT UINT32 *IntegrityGroupCipher,
#endif
	OUT USHORT * RsnCapability,
	OUT USHORT * IsSHA256)
{
	PEID_STRUCT pEid;
	PUCHAR pTmp;
	PRSN_IE_HEADER_STRUCT pRsnHeader;
	PCIPHER_SUITE_STRUCT pCipher;
	PAKM_SUITE_STRUCT pAKM;
	USHORT Count;
	SHORT Length;
	UCHAR end_field = 0;
	UCHAR res;
	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	CLEAR_SEC_AKM(*AKMMap);
	CLEAR_CIPHER(*PairwiseCipher);
	CLEAR_CIPHER(*GroupCipher);
	Length = (SHORT) *LengthVIE;

	while (Length > 0) {
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pVIE) + *LengthVIE - ((USHORT)Length);
		pEid = (PEID_STRUCT) pTmp;

		switch (pEid->Eid) {
		case IE_WPA:
			if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
				break;
			else if (NdisEqualMemory(pEid->Octet, OUI_WPA_VERSION, 4) != 1) {
				/* if unsupported vendor specific IE*/
				break;
			}

			/*
			 *	Skip OUI, version, and multicast suite
			 *	This part should be improved in the future when AP supported multiple cipher suite.
			 *	For now, it's OK since almost all APs have fixed cipher suite supported.
			 */
			/* pTmp = (PUCHAR) pEid->Octet;*/
			pTmp   += 11;

			/*
			 *	Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
			 *	Value	   Meaning
			 *	0			None
			 *	1			WEP-40
			 *	2			Tkip
			 *	3			WRAP
			 *	4			AES
			 *	5			WEP-104
			 */
			/* Parse group cipher*/
			switch (*pTmp) {
			case 1:
				SET_CIPHER_WEP40(*GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(*GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(*GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(*GroupCipher);
				break;

			default:
				break;
			}

			/* number of unicast suite*/
			pTmp   += 1;
			/* skip all unicast cipher suites*/
			/*Count = *(PUSHORT) pTmp;				*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pTmp += 3;

				switch (*pTmp) {
				case 1:
					SET_CIPHER_WEP40(*PairwiseCipher);
					break;

				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					SET_CIPHER_WEP104(*PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(*PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(*PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp++;
				Count--;
			}

			/* 4. get AKM suite counts*/
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			pTmp   += 3;

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				SET_AKM_WPA1(*AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				SET_AKM_WPA1PSK(*AKMMap);
				break;

			default:
				break;
			}

			pTmp   += 1;

			/* Fixed for WPA-None*/
			if (ie_list->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(*AKMMap);

			break;

		case IE_RSN:
			pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

			res = wpa_rsne_sanity(pTmp, pRsnHeader->Length + 2, &end_field);

			if (res == FALSE)
				break;

			if (end_field < RSN_FIELD_GROUP_CIPHER)
				SET_CIPHER_CCMP128(*GroupCipher);
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				SET_CIPHER_CCMP128(*PairwiseCipher);
			if (end_field < RSN_FIELD_AKM)
				SET_AKM_WPA2(*AKMMap);

			/* 0. Version must be 1*/
			if (le2cpu16(pRsnHeader->Version) != 1)
				break;

			/* 1. Check group cipher*/
			if (end_field < RSN_FIELD_GROUP_CIPHER)
				break;

			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (!RTMPEqualMemory(&pCipher->Oui, OUI_WPA2_CIPHER, 3))
				break;

			/* Parse group cipher*/
			switch (pCipher->Type) {
			case 1:
				SET_CIPHER_WEP40(*GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(*GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(*GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(*GroupCipher);
				break;

			case 8:
				SET_CIPHER_GCMP128(*GroupCipher);
				break;

			case 9:
				SET_CIPHER_GCMP256(*GroupCipher);
				break;

			case 10:
				SET_CIPHER_CCMP256(*GroupCipher);
				break;

			default:
				break;
			}

			/* set to correct offset for next parsing*/
			pTmp   += sizeof(CIPHER_SUITE_STRUCT);
			/* 2. Get pairwise cipher counts*/
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				break;
			/*Count = *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 3. Get pairwise cipher*/
			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

				switch (pCipher->Type) {
				case 1:
					SET_CIPHER_WEP40(*PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(*PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(*PairwiseCipher);
					break;

				case 5:
					SET_CIPHER_WEP104(*PairwiseCipher);
					break;

				case 8:
					SET_CIPHER_GCMP128(*PairwiseCipher);
					break;

				case 9:
					SET_CIPHER_GCMP256(*PairwiseCipher);
					break;

				case 10:
					SET_CIPHER_CCMP256(*PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp += sizeof(CIPHER_SUITE_STRUCT);
				Count--;
			}

			/* 4. get AKM suite counts*/
			if (end_field < RSN_FIELD_AKM)
				break;
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 5. Get AKM ciphers*/
			/* Parsing all AKM ciphers*/
			while (Count > 0) {
				pAKM = (PAKM_SUITE_STRUCT) pTmp;

				if (RTMPEqualMemory(pTmp, OUI_WPA2_CIPHER, 3)
#ifdef DPP_SUPPORT
					|| RTMPEqualMemory(pTmp, DPP_OUI, 3)
#endif /* DPP_SUPPORT */
					) {
					switch (pAKM->Type) {
					case 0:
						SET_AKM_WPANONE(*AKMMap);
						break;

					case 1:
						SET_AKM_WPA2(*AKMMap);
						break;

					case 2:
#ifdef DPP_SUPPORT
						if (RTMPEqualMemory(pTmp, RSN_OUI, 3))
							SET_AKM_WPA2PSK(*AKMMap);
						if (RTMPEqualMemory(pTmp, DPP_OUI, 3))
							SET_AKM_DPP(*AKMMap);
#else
						SET_AKM_WPA2PSK(*AKMMap);
#endif /* DPP_SUPPORT */
						break;

					case 3:
						SET_AKM_FT_WPA2(*AKMMap);
						break;

					case 4:
						SET_AKM_FT_WPA2PSK(*AKMMap);
						break;
#ifdef DOT11W_PMF_SUPPORT

					case 5:
						/* SET_AKM_WPA2_SHA256(*AKMMap); */
						SET_AKM_WPA2(*AKMMap);
						*IsSHA256 = TRUE;
						break;

					case 6:
						/* SET_AKM_WPA2PSK_SHA256(*AKMMap); */
						SET_AKM_WPA2PSK(*AKMMap);
						*IsSHA256 = TRUE;
						break;
#endif /* DOT11W_PMF_SUPPORT */

					case 7:
						SET_AKM_TDLS(*AKMMap);
						break;

					case 8:
						SET_AKM_SAE_SHA256(*AKMMap);
						break;

					case 9:
						SET_AKM_FT_SAE_SHA256(*AKMMap);
						break;

					case 11:
						SET_AKM_SUITEB_SHA256(*AKMMap);
						break;

					case 12:
						SET_AKM_SUITEB_SHA384(*AKMMap);
						break;

					case 13:
						SET_AKM_FT_WPA2_SHA384(*AKMMap);
						break;
					case 18:
						SET_AKM_OWE(*AKMMap);
						break;
					default:
						break;
					}
				}
				pTmp   += sizeof(AKM_SUITE_STRUCT);
				Count--;
			}

			/* Fixed for WPA-None*/
			if (ie_list->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(*AKMMap);

			/* 6. Get RSN capability*/
			if (end_field < RSN_FIELD_RSN_CAP)
				break;
			/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
			*RsnCapability = (pTmp[1] << 8) + pTmp[0];
			pTmp += sizeof(USHORT);

#ifdef DOT11W_PMF_SUPPORT
			if ((*RsnCapability) && (end_field < RSN_FIELD_GROUP_MGMT_CIPHER))
				SET_CIPHER_BIP_CMAC128(*IntegrityGroupCipher);

			/* 7. get PMKID counts*/
			if (end_field < RSN_FIELD_PMKID)
				break;
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 8. ignore PMKID */
			pTmp += 16 * Count;

			/* 9. Get Group Management Cipher Suite*/
			if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER)
				break;
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
			if (!RTMPEqualMemory(&pCipher->Oui, OUI_WPA2_CIPHER, 3))
				break;
			switch (pCipher->Type) {
			case 6:
				SET_CIPHER_BIP_CMAC128(*IntegrityGroupCipher);
				break;

			case 11:
				SET_CIPHER_BIP_GMAC128(*IntegrityGroupCipher);
				break;

			case 12:
				SET_CIPHER_BIP_GMAC256(*IntegrityGroupCipher);
				break;

			case 13:
				SET_CIPHER_BIP_CMAC256(*IntegrityGroupCipher);
				break;
			default:
				MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"unknown Group Management Cipher Suite %d\n", pCipher->Type);
				break;
			}
			pTmp += sizeof(CIPHER_SUITE_STRUCT);
#endif
			break;

		default:
			break;
		}

		Length -= (pEid->Len + 2);
	}

	if (*AKMMap == 0x0) {
		SET_AKM_OPEN(*AKMMap);

		if (CAP_IS_PRIVACY_ON(ie_list->CapabilityInfo)) {
			SET_AKM_SHARED(*AKMMap);
			SET_CIPHER_WEP(*PairwiseCipher);
			SET_CIPHER_WEP(*GroupCipher);
		} else {
			SET_CIPHER_NONE(*PairwiseCipher);
			SET_CIPHER_NONE(*GroupCipher);
		}
	}
}


