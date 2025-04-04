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
    bss.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-25      AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"

UCHAR CISCO_OUI[]       = {0x00, 0x40, 0x96};
UCHAR RALINK_OUI[]      = {0x00, 0x0c, 0x43};
UCHAR WPA_OUI[]         = {0x00, 0x50, 0xf2, 0x01};
UCHAR RSN_OUI[]         = {0x00, 0x0f, 0xac};
UCHAR WAPI_OUI[]        = {0x00, 0x14, 0x72};
UCHAR WME_INFO_ELEM[]   = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR WME_PARM_ELEM[]   = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR BROADCOM_OUI[]    = {0x00, 0x90, 0x4c};
UCHAR MARVELL_OUI[]     = {0x00, 0x50, 0x43};
UCHAR METALINK_OUI[]    = {0x00, 0x09, 0x86};
UCHAR WPS_OUI[]         = {0x00, 0x50, 0xf2, 0x04};
#if defined(WH_EZ_SETUP) || defined(MWDS) || defined(WAPP_SUPPORT)
UCHAR MTK_OUI[]         = {0x00, 0x0c, 0xe7};
#endif /* WH_EZ_SETUP || MWDS */
#ifdef CONFIG_STA_SUPPORT
#ifdef IWSC_SUPPORT
UCHAR IWSC_OUI[]        = {0x00, 0x50, 0xf2, 0x10};
UCHAR IWSC_ACTION_OUI[] = {0x50, 0x6F, 0x9A, 0x10};
#endif /* IWSC_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
UCHAR IGMP_TVM_OUI[] = {0x00, 0x0D, 0x02, 0x03};
#endif /* IGMP_TVM_SUPPORT */


#ifdef DPP_SUPPORT
UCHAR DPP_OUI[]     = {0x50, 0x6f, 0x9a, 0x02};
#endif /* DPP_SUPPORT */
extern UCHAR WPA_OUI[];
extern UCHAR SES_OUI[];

UCHAR ZeroSsid[MAX_LEN_OF_SSID] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static VOID BssCipherParse(BSS_ENTRY *pBss)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	SHORT								Length;
	UCHAR end_field = 0;
	UCHAR res;
	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	CLEAR_SEC_AKM(pBss->AKMMap);
	CLEAR_CIPHER(pBss->PairwiseCipher);
	CLEAR_CIPHER(pBss->GroupCipher);
	Length = (SHORT) pBss->VarIELen;

	while (Length > 0) {
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - ((USHORT)Length);
		pEid = (PEID_STRUCT) pTmp;

		switch (pEid->Eid) {
		case IE_WPA:
			if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7)) {
				pBss->bSES = TRUE;
				break;
			} else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1) {
				/* if unsupported vendor specific IE*/
				break;
			}

			/*
				Skip OUI, version, and multicast suite
				This part should be improved in the future when AP supported multiple cipher suite.
				For now, it's OK since almost all APs have fixed cipher suite supported.
			*/
			/* pTmp = (PUCHAR) pEid->Octet;*/
			pTmp   += 11;

			/*
				Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
				Value	   Meaning
				0			None
				1			WEP-40
				2			Tkip
				3			WRAP
				4			AES
				5			WEP-104
			*/
			/* Parse group cipher*/
			switch (*pTmp) {
			case 1:
				SET_CIPHER_WEP40(pBss->GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pBss->GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pBss->GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pBss->GroupCipher);
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
					SET_CIPHER_WEP40(pBss->PairwiseCipher);
					break;

				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					SET_CIPHER_WEP104(pBss->PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pBss->PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pBss->PairwiseCipher);
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
				SET_AKM_WPA1(pBss->AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				SET_AKM_WPA1PSK(pBss->AKMMap);
				break;

			default:
				break;
			}

			pTmp   += 1;

			/* Fixed for WPA-None*/
			if (pBss->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(pBss->AKMMap);

			break;

		case IE_RSN:
			pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
			res = wpa_rsne_sanity(pTmp, pRsnHeader->Length + 2, &end_field);

			if (res == FALSE)
				break;

			if (end_field < RSN_FIELD_GROUP_CIPHER)
				SET_CIPHER_CCMP128(pBss->GroupCipher);
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				SET_CIPHER_CCMP128(pBss->PairwiseCipher);
			if (end_field < RSN_FIELD_AKM)
				SET_AKM_WPA2(pBss->AKMMap);

			/* 0. Version must be 1*/
			if (le2cpu16(pRsnHeader->Version) != 1)
				break;

			/* 1. Check group cipher*/
			if (end_field < RSN_FIELD_GROUP_CIPHER)
				break;

			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (!RTMPEqualMemory(&pCipher->Oui, RSN_OUI, 3))
				break;

			/* Parse group cipher*/
			switch (pCipher->Type) {
			case 1:
				SET_CIPHER_WEP40(pBss->GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pBss->GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pBss->GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pBss->GroupCipher);
				break;

			case 8:
				SET_CIPHER_GCMP128(pBss->GroupCipher);
				break;

			case 9:
				SET_CIPHER_GCMP256(pBss->GroupCipher);
				break;

			case 10:
				SET_CIPHER_CCMP256(pBss->GroupCipher);
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
					SET_CIPHER_WEP40(pBss->PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pBss->PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pBss->PairwiseCipher);
					break;

				case 5:
					SET_CIPHER_WEP104(pBss->PairwiseCipher);
					break;

				case 8:
					SET_CIPHER_GCMP128(pBss->PairwiseCipher);
					break;

				case 9:
					SET_CIPHER_GCMP256(pBss->PairwiseCipher);
					break;

				case 10:
					SET_CIPHER_CCMP256(pBss->PairwiseCipher);
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

				if (RTMPEqualMemory(pTmp, RSN_OUI, 3)
#ifdef DPP_SUPPORT
					|| RTMPEqualMemory(pTmp, DPP_OUI, 3)
#endif /* DPP_SUPPORT */
					) {
					switch (pAKM->Type) {
					case 0:
						SET_AKM_WPANONE(pBss->AKMMap);
						break;

					case 1:
						SET_AKM_WPA2(pBss->AKMMap);
						break;

					case 2:
#ifdef DPP_SUPPORT
						if (RTMPEqualMemory(pTmp, RSN_OUI, 3))
							SET_AKM_WPA2PSK(pBss->AKMMap);
						if (RTMPEqualMemory(pTmp, DPP_OUI, 3)) {
							SET_AKM_DPP(pBss->AKMMap);
							pBss->IsSupportSHA256KeyDerivation = TRUE;
						}
#else
						SET_AKM_WPA2PSK(pBss->AKMMap);
#endif /* DPP_SUPPORT */
						break;

					case 3:
						SET_AKM_FT_WPA2(pBss->AKMMap);
						break;

					case 4:
						SET_AKM_FT_WPA2PSK(pBss->AKMMap);
						break;
#ifdef DOT11W_PMF_SUPPORT

					case 5:
						/* SET_AKM_WPA2_SHA256(pBss->AKMMap); */
						SET_AKM_WPA2(pBss->AKMMap);
						pBss->IsSupportSHA256KeyDerivation = TRUE;
						break;

					case 6:
						/* SET_AKM_WPA2PSK_SHA256(pBss->AKMMap); */
						SET_AKM_WPA2PSK(pBss->AKMMap);
						pBss->IsSupportSHA256KeyDerivation = TRUE;
						break;
#endif /* DOT11W_PMF_SUPPORT */

					case 7:
						SET_AKM_TDLS(pBss->AKMMap);
						break;

					case 8:
						SET_AKM_SAE_SHA256(pBss->AKMMap);
						break;

					case 9:
						SET_AKM_FT_SAE_SHA256(pBss->AKMMap);
						break;

					case 11:
						SET_AKM_SUITEB_SHA256(pBss->AKMMap);
						break;

					case 12:
						SET_AKM_SUITEB_SHA384(pBss->AKMMap);
						break;

					case 13:
						SET_AKM_FT_WPA2_SHA384(pBss->AKMMap);
						break;

					case 18:
						SET_AKM_OWE(pBss->AKMMap);
						break;

					default:
						break;
					}
				}
				pTmp   += sizeof(AKM_SUITE_STRUCT);
				Count--;
			}

			/* Fixed for WPA-None*/
			if (pBss->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(pBss->AKMMap);

			/* 6. Get RSN capability*/
			if (end_field < RSN_FIELD_RSN_CAP)
				break;
			/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
			pBss->RsnCapability = (pTmp[1] << 8) + pTmp[0];
			pTmp += sizeof(USHORT);
			break;
		case IE_RSNXE:
#ifdef DOT11_SAE_SUPPORT
			if (pEid->Octet[0] & (1 << IE_RSNXE_CAPAB_SAE_H2E)) {
				if (pEid->Octet[0] & (1 << IE_RSNXE_CAPAB_SAE_PK))
					pBss->sae_conn_type = SAE_CONNECTION_TYPE_SAEPK;
				else
					pBss->sae_conn_type = SAE_CONNECTION_TYPE_H2E;
			}
#endif
			pBss->rsnxe_len = pEid->Len + 2;
			NdisMoveMemory(pBss->rsnxe_content, (UCHAR *)pEid, pBss->rsnxe_len);
			break;

		default:
			break;
		}

		Length -= (pEid->Len + 2);
	}

	if (pBss->AKMMap == 0x0) {
		SET_AKM_OPEN(pBss->AKMMap);

		if (pBss->Privacy) {
			SET_AKM_SHARED(pBss->AKMMap);
			SET_CIPHER_WEP(pBss->PairwiseCipher);
			SET_CIPHER_WEP(pBss->GroupCipher);
		} else {
			SET_CIPHER_NONE(pBss->PairwiseCipher);
			SET_CIPHER_NONE(pBss->GroupCipher);
		}
	}
}


/*! \brief initialize BSS table
 *	\param p_tab pointer to the table
 *	\return none
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
VOID BssTableInit(BSS_TABLE *Tab)
{
	UINT i;

	Tab->BssNr = 0;
	Tab->BssOverlapNr = 0;

	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
		BssEntryReset(Tab, &Tab->BssEntry[i]);
	}
}


/*! \brief search the BSS table by SSID
 *	\param p_tab pointer to the bss table
 *	\param ssid SSID string
 *	\return index of the table, BSS_NOT_FOUND if not in the table
 *	\pre
 *	\post
 *	\note search by sequential search

 IRQL = DISPATCH_LEVEL

 */
UINT BssNumByChannel(BSS_TABLE *Tab, UCHAR Channel)
{
	UINT i;
	UINT Num = 0;

	for (i = 0; i < Tab->BssNr && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if (Tab->BssEntry[i].Channel == Channel)
			Num++;
	}
	return Num;
}


ULONG BssTableSearch(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UINT i;

	for (i = 0; i < Tab->BssNr && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		/*
			Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
			We should distinguish this case.
		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UINT i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		/* Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.*/
		/* We should distinguish this case.*/
		/*		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UINT i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			 (NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			 (NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearchBySSID(BSS_TABLE *Tab, UCHAR *pSsid, UCHAR SsidLen)
{
	UINT i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


VOID BssTableDeleteEntry(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UINT i, j;

	for (i = 0; i < Tab->BssNr && Tab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		if ((Tab->BssEntry[i].Channel == Channel) &&
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))) {
			UCHAR *pOldAddr = NULL;

			for (j = i; j < Tab->BssNr - 1; j++) {
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				/* delete this entry and use the behind entry replace this entry*/
				BssEntryCopy(Tab, &(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]));

				if (pOldAddr) {
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr,
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp,
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}
			/*clear the last one entry*/
			BssEntryReset(Tab, &Tab->BssEntry[Tab->BssNr - 1]);
			Tab->BssNr -= 1;
			return;
		}
	}
}

/*! \brief
 *	\param
 *	\return
 *	\pre
 *	\post
 */
VOID BssEntrySet(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	OUT BSS_ENTRY *pBss,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)

{
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct legacy_rate *rate = &cmm_ies->rate;

	COPY_MAC_ADDR(pBss->Bssid, ie_list->Bssid);
	/* Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID*/
	pBss->Hidden = 1;
	pBss->FromBcnReport = ie_list->FromBcnReport;

	if (ie_list->SsidLen > 0) {
		/* For hidden SSID AP, it might send beacon with SSID len equal to 0*/
		/* Or send beacon /probe response with SSID len matching real SSID length,*/
		/* but SSID is all zero. such as "00-00-00-00" with length 4.*/
		/* We have to prevent this case overwrite correct table*/
		if (NdisEqualMemory(ie_list->Ssid, ZeroSsid, ie_list->SsidLen) == 0) {
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
			pBss->SsidLen = ie_list->SsidLen;
			pBss->Hidden = 0;
		}
	} else {
		/* avoid  Hidden SSID form beacon to overwirite correct SSID from probe response */
		if (NdisEqualMemory(pBss->Ssid, ZeroSsid, pBss->SsidLen)) {
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			pBss->SsidLen = 0;
		}
	}

	pBss->BssType = ie_list->BssType;
	pBss->BeaconPeriod = ie_list->BeaconPeriod;
#ifdef TR181_SUPPORT
	pBss->DtimPeriod = ie_list->NbrDtimPeriod;
#endif

	if (ie_list->BssType == BSS_INFRA) {
		if (ie_list->CfParm.bValid) {
			pBss->CfpCount = ie_list->CfParm.CfpCount;
			pBss->CfpPeriod = ie_list->CfParm.CfpPeriod;
			pBss->CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pBss->CfpDurRemaining = ie_list->CfParm.CfpDurRemaining;
		}
	} else
		pBss->AtimWin = ie_list->AtimWin;

	NdisGetSystemUpTime(&pBss->LastBeaconRxTime);
	pBss->CapabilityInfo = ie_list->CapabilityInfo;
	/* The privacy bit indicate security is ON, it maight be WEP, TKIP or AES*/
	/* Combine with AuthMode, they will decide the connection methods.*/
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(rate->sup_rate_len <= MAX_LEN_OF_SUPPORTED_RATES);

	if (rate->sup_rate_len > MAX_LEN_OF_SUPPORTED_RATES)
		rate->sup_rate_len = MAX_LEN_OF_SUPPORTED_RATES;

	NdisMoveMemory(pBss->SupRate, rate->sup_rate, rate->sup_rate_len);
	pBss->SupRateLen = rate->sup_rate_len;
	ASSERT(rate->ext_rate_len <= MAX_LEN_OF_SUPPORTED_RATES);

	if (rate->ext_rate_len > MAX_LEN_OF_SUPPORTED_RATES)
		rate->ext_rate_len = MAX_LEN_OF_SUPPORTED_RATES;

	NdisMoveMemory(pBss->ExtRate, rate->ext_rate, rate->ext_rate_len);
	pBss->NewExtChanOffset = ie_list->NewExtChannelOffset;
	pBss->ExtRateLen = rate->ext_rate_len;
	pBss->Erp  = ie_list->Erp;
	pBss->Channel = ie_list->Channel;
	pBss->CentralChannel = ie_list->Channel;
	pBss->Rssi = Rssi;
	/* Update CkipFlag. if not exists, the value is 0x0*/
	pBss->CkipFlag = ie_list->CkipFlag;
	/* New for microsoft Fixed IEs*/
	NdisMoveMemory(pBss->FixIEs.Timestamp, &ie_list->TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = ie_list->BeaconPeriod;
	pBss->FixIEs.Capabilities = ie_list->CapabilityInfo;

	/* New for microsoft Variable IEs*/
	if (LengthVIE != 0) {
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	} else
		pBss->VarIELen = 0;

	CLR_HT_CAPS_EXIST(pBss->ie_exists);
	CLR_HT_OP_EXIST(pBss->ie_exists);
#ifdef CONFIG_MAP_SUPPORT
	pBss->map_vendor_ie_found = cmm_ies->vendor_ie.map_vendor_ie_found;
	if (pBss->map_vendor_ie_found)
		NdisMoveMemory(&pBss->map_info, &cmm_ies->vendor_ie.map_info, sizeof(struct map_vendor_ie));
#endif

#ifdef DPP_R2_SUPPORT
	pBss->cce_vendor_ie_found = cmm_ies->vendor_ie.cce_vendor_ie_found;
	if (pBss->cce_vendor_ie_found)
		NdisMoveMemory(&pBss->cce_info, &cmm_ies->vendor_ie.cce_info, sizeof(struct cce_vendor_ie));
#endif

#ifdef DOT11_N_SUPPORT

	if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists)) {
		SET_HT_CAPS_EXIST(pBss->ie_exists);
		NdisMoveMemory(&pBss->HtCapability, &cmm_ies->ht_cap, SIZE_HT_CAP_IE);
		if (HAS_HT_OP_EXIST(cmm_ies->ie_exists)) {
			SET_HT_OP_EXIST(pBss->ie_exists);
			NdisMoveMemory(&pBss->AddHtInfo, &cmm_ies->ht_op, SIZE_ADD_HT_INFO_IE);
			pBss->CentralChannel = get_cent_ch_by_htinfo(pAd, &cmm_ies->ht_op,
								   &cmm_ies->ht_cap);
		}

#ifdef DOT11_VHT_AC

		if (HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			NdisMoveMemory(&pBss->vht_cap_ie, &cmm_ies->vht_cap, SIZE_OF_VHT_CAP_IE);
			SET_VHT_CAPS_EXIST(pBss->ie_exists);
		}

		if (HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
			VHT_OP_IE *vht_op;

			NdisMoveMemory(&pBss->vht_op_ie, &cmm_ies->vht_op, SIZE_OF_VHT_OP_IE);
			SET_VHT_OP_EXIST(pBss->ie_exists);
			vht_op = &cmm_ies->vht_op;

			if ((vht_op->vht_op_info.ch_width > 0) &&
				(cmm_ies->ht_op.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
				(cmm_ies->ht_cap.HtCapInfo.ChannelWidth == BW_40) &&
				(pBss->CentralChannel != cmm_ies->ht_op.ControlChan)) {
				if (vht_op->vht_op_info.ch_width == 2 || vht_op->vht_op_info.ch_width == 3) {
				/* VHT operation definition 1*/
					if (vht_op->vht_op_info.ch_width == 2)
					/* 160M */
						pBss->CentralChannel = vht_op->vht_op_info.ccfs_0;
					else if (vht_op->vht_op_info.ch_width == 3) {
					/* 80M + 80M */
						pBss->CentralChannel = vht_op->vht_op_info.ccfs_0;
						pBss->SecCentralChannel = vht_op->vht_op_info.ccfs_1;
					}
				} else {
				/* VHT operation definition 2*/
					if (vht_op->vht_op_info.ccfs_1 == 0)
					/* 80M */
						pBss->CentralChannel = vht_op->vht_op_info.ccfs_0;
					else {
						if (((vht_op->vht_op_info.ccfs_1) - (vht_op->vht_op_info.ccfs_0)) == 8 ||
							((vht_op->vht_op_info.ccfs_1) - (vht_op->vht_op_info.ccfs_0)) == -8)
						/* 160M */
							pBss->CentralChannel = vht_op->vht_op_info.ccfs_1;
						else {
						/* 80M+80M */
							pBss->CentralChannel = vht_op->vht_op_info.ccfs_0;
							pBss->SecCentralChannel = vht_op->vht_op_info.ccfs_1;
						}
					}
				}
			}
		}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
		if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists))
			SET_HE_CAPS_EXIST(pBss->ie_exists);

		if (HAS_HE_OP_EXIST(cmm_ies->ie_exists))
			SET_HE_OP_EXIST(pBss->ie_exists);
#endif /*DOT11_HE_AX*/
	}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		NdisMoveMemory(&pBss->he_caps, &cmm_ies->he_caps, sizeof(struct he_cap_ie));
		NdisMoveMemory(&pBss->he_mcs_nss_160, &cmm_ies->mcs_nss_160, sizeof(struct he_txrx_mcs_nss));
		NdisMoveMemory(&pBss->he_mcs_nss_8080, &cmm_ies->mcs_nss_8080, sizeof(struct he_txrx_mcs_nss));
		SET_HE_CAPS_EXIST(pBss->ie_exists);
	}
	if (HAS_HE_OP_EXIST(cmm_ies->ie_exists)) {
		NdisMoveMemory(&pBss->he_ops, &cmm_ies->he_ops, sizeof(struct he_op_ie));
		SET_HE_OP_EXIST(pBss->ie_exists);
	}
#endif /*DOT11_HE_AX*/
	BssCipherParse(pBss);

	/* new for QOS*/
	if (ie_list->EdcaParm.bValid)
		NdisMoveMemory(&pBss->EdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;

	if (ie_list->QosCapability.bValid)
		NdisMoveMemory(&pBss->QosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;

	if (ie_list->QbssLoad.bValid)
		NdisMoveMemory(&pBss->QbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

	{
		PEID_STRUCT pEid;
		USHORT Length = 0;
#ifdef WSC_INCLUDED
		pBss->WpsAP = 0x00;
		pBss->WscDPIDFromWpsAP = 0xFFFF;
#endif /* WSC_INCLUDED */
#ifdef CONFIG_STA_SUPPORT
		NdisZeroMemory(&pBss->WpaIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->RsnIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->WpsIE.IE[0], MAX_CUSTOM_LEN);
		pBss->WpaIE.IELen = 0;
		pBss->RsnIE.IELen = 0;
		pBss->WpsIE.IELen = 0;
#ifdef EXT_BUILD_CHANNEL_LIST
		NdisZeroMemory(&pBss->CountryString[0], 3);
		pBss->bHasCountryIE = FALSE;
#endif /* EXT_BUILD_CHANNEL_LIST */
#if defined(DOT11R_FT_SUPPORT) || defined(DOT11K_RRM_SUPPORT)

		pBss->bHasMDIE = FALSE;
		NdisZeroMemory(&pBss->FT_MDIE, sizeof(FT_MDIE));
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		pEid = (PEID_STRUCT) pVIE;

		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE) {
#define WPS_AP		0x01

			switch (pEid->Eid) {
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
#ifdef IWSC_SUPPORT
					|| NdisEqualMemory(pEid->Octet, IWSC_OUI, 4)
#endif /* IWSC_SUPPORT */
				   ) {
#ifdef WSC_INCLUDED
					pBss->WpsAP |= WPS_AP;
					WscCheckWpsIeFromWpsAP(pAd,
										   pEid,
										   &pBss->WscDPIDFromWpsAP);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_STA_SUPPORT

					if ((pEid->Len + 2) > MAX_CUSTOM_LEN) {
						pBss->WpsIE.IELen = 0;
						break;
					}

					pBss->WpsIE.IELen = pEid->Len + 2;
					NdisMoveMemory(pBss->WpsIE.IE, pEid, pBss->WpsIE.IELen);
#endif /* CONFIG_STA_SUPPORT */
					break;
				}

#ifdef CONFIG_STA_SUPPORT
				if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) {
					if ((pEid->Len + 2) > MAX_CUSTOM_LEN) {
						pBss->WpaIE.IELen = 0;
						break;
					}

					pBss->WpaIE.IELen = pEid->Len + 2;
					NdisMoveMemory(pBss->WpaIE.IE, pEid, pBss->WpaIE.IELen);
				}

#endif /* CONFIG_STA_SUPPORT */
#ifdef MWDS
				check_vendor_ie(pAd, (UCHAR *)pEid, &(ie_list->cmm_ies.vendor_ie));

				if (ie_list->cmm_ies.vendor_ie.mtk_cap_found) {
					if (ie_list->cmm_ies.vendor_ie.support_mwds)
						pBss->bSupportMWDS = TRUE;
					else
						pBss->bSupportMWDS = FALSE;
				}
#endif /* MWDS */
#ifdef CONFIG_OWE_SUPPORT
				 if (NdisEqualMemory(pEid->Octet, OWE_TRANS_OUI, 4)) {
					NdisZeroMemory(pBss->owe_trans_ie, MAX_VIE_LEN);
					NdisMoveMemory(pBss->owe_trans_ie, pEid->Octet + 4, pEid->Len - 4);
					pBss->owe_trans_ie_len = pEid->Len - 4;
				}
				break;
#endif

#ifdef CONFIG_STA_SUPPORT

			case IE_RSN:
				if (parse_rsn_ie(pEid) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))) {
					if ((pEid->Len + 2) > MAX_CUSTOM_LEN) {
						pBss->RsnIE.IELen = 0;
						break;
					}

					pBss->RsnIE.IELen = pEid->Len + 2;
					NdisMoveMemory(pBss->RsnIE.IE, pEid, pBss->RsnIE.IELen);
				} else /* func_no_retrun_value */
					return;
				break;
#ifdef EXT_BUILD_CHANNEL_LIST

			case IE_COUNTRY:
				if (parse_country_ie(pEid)) {
					NdisMoveMemory(&pBss->CountryString[0], pEid->Octet, 3);
					pBss->bHasCountryIE = TRUE;
				} else /* func_no_retrun_value */
					return;
				break;
#endif /* EXT_BUILD_CHANNEL_LIST */
#if defined(DOT11R_FT_SUPPORT) || defined(DOT11K_RRM_SUPPORT)


			case IE_FT_MDIE:
				if (parse_md_ie(pEid)) {
					pBss->bHasMDIE = TRUE;
					NdisMoveMemory(&pBss->FT_MDIE, pEid->Octet, pEid->Len);
				} else /* func_no_retrun_value */
					return;

				break;
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
			}

			Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
		}
	}
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	CustomerBssEntrySet(pAd, wdev, ie_list, pBss, LengthVIE, pVIE);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	pBss->is_oce_ap = ie_list->is_oce_ap;
	pBss->is_11bonly_ap = ie_list->is_11bonly_ap;
#endif
#endif /* OCE_SUPPORT */
#ifdef MAP_6E_SUPPORT
	pBss->rnr_info.channel = ie_list->rnr_info.channel;
	pBss->rnr_info.op = ie_list->rnr_info.op;
#endif
#ifdef CONFIG_6G_SUPPORT
	pBss->rnr_channel = ie_list->rnr_channel;
#endif
}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
UINT8 peer_max_bw_cap(INT8 ch_width_set);
VOID CustomerBssEntrySet(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	IN BCN_IE_LIST *ie_list,
	IN BSS_ENTRY * pBss,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	UINT8 MODE = 0;
	UINT8 ShortGI = 0;
	UINT8 BW = 0;
	UINT8 MAX_MCS = 0;
	UINT8 Antenna = 1;
	UINT8 hetmp = 0;

	struct customer_bss_entry *CustomerBssEntry;

	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	RTMP_SPIN_LOCK(&ScanTab->event_bss_entry_lock);

	CustomerBssEntry = &pBss->CustomerBssEntry;

	NdisMoveMemory(CustomerBssEntry->ssid, pBss->Ssid, pBss->SsidLen);
	CustomerBssEntry->ssid_len = pBss->SsidLen;
	COPY_MAC_ADDR(CustomerBssEntry->bssid, pBss->Bssid);
	CustomerBssEntry->channel = pBss->Channel;
	CustomerBssEntry->beacon_period = pBss->BeaconPeriod;
	CustomerBssEntry->rssi = pBss->Rssi;

	/*CustomerBssEntry->noise = ie_list->noise;*/ /*neet to check */

	CustomerBssEntry->ht_ch_bandwidth =
		ie_list->cmm_ies.ht_cap.HtCapInfo.ChannelWidth;



#ifdef DOT11_VHT_AC
	CustomerBssEntry->vht_ch_bandwidth =
				ie_list->cmm_ies.vht_op.vht_op_info.ch_width;

#endif

	CustomerBssEntry->PairwiseCipher = pBss->PairwiseCipher;
	CustomerBssEntry->phy_mode = WMODE_INVALID;

	if (ie_list->Channel > 14) {
		if (!HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) && !HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
			CustomerBssEntry->phy_mode |= WMODE_A;
			MODE = MODE_OFDM;
			MAX_MCS = 7;
		}
		if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
			CustomerBssEntry->phy_mode |= WMODE_AN;

		if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
			CustomerBssEntry->phy_mode |= WMODE_AC;
	} else {
/*
			if (ie_list->sup_dsss) {
			CustomerBssEntry->phy_mode |= WMODE_B;
			MODE = MODE_CCK;
			MAX_MCS = 1;
		}
		if (ie_list->sup_erp) {
			CustomerBssEntry->phy_mode |= WMODE_G;
			MODE = MODE_OFDM;
			MAX_MCS = 7;
		}
*/
		if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists))
			CustomerBssEntry->phy_mode |= WMODE_GN;
		else if (ie_list->cmm_ies.rate.ext_rate_len != 0)
			/*resolve the 2.4G bg AP's maxbit rate is 0*/
			CustomerBssEntry->phy_mode |= WMODE_G;
		else
			CustomerBssEntry->phy_mode |= WMODE_B;
	}
	if (HAS_HT_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		MODE = MODE_HTMIX;
		if (ie_list->cmm_ies.ht_cap.HtCapInfo.ShortGIfor20 == TRUE ||
		    ie_list->cmm_ies.ht_cap.HtCapInfo.ShortGIfor40 == TRUE)
			ShortGI = 1;

		BW = CustomerBssEntry->ht_ch_bandwidth;

		if ((ie_list->cmm_ies.ht_cap.MCSSet[3] & 0xff) == 0xff) {
			MAX_MCS = 31;
			CustomerBssEntry->ht_tx_ss = 4;
			CustomerBssEntry->ht_rx_ss = 4;
		} else if ((ie_list->cmm_ies.ht_cap.MCSSet[2] & 0xff) == 0xff) {
			MAX_MCS = 23;
			CustomerBssEntry->ht_tx_ss = 3;
			CustomerBssEntry->ht_rx_ss = 3;
		} else if ((ie_list->cmm_ies.ht_cap.MCSSet[1] & 0xff) == 0xff) {
			MAX_MCS = 15;
			CustomerBssEntry->ht_tx_ss = 2;
			CustomerBssEntry->ht_rx_ss = 2;
		} else if ((ie_list->cmm_ies.ht_cap.MCSSet[0] & 0xff) == 0xff) {
			MAX_MCS = 7;
			CustomerBssEntry->ht_tx_ss = 1;
			CustomerBssEntry->ht_rx_ss = 1;
		}
	}
#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(ie_list->cmm_ies.ie_exists) && ie_list->Channel > 14) {
		UCHAR mcs_ss = 9;

		MODE = MODE_VHT;
		if (ie_list->cmm_ies.vht_cap.vht_cap.sgi_80M == TRUE ||
		    ie_list->cmm_ies.vht_cap.vht_cap.sgi_160M == TRUE)
			ShortGI = 1;

		BW = CustomerBssEntry->vht_ch_bandwidth;

		if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss8 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 8;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss8;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss7 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 7;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss7;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss6 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 6;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss6;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss5 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 5;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss5;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 4;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss4;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 3;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss3;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 2;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss2;
		} else if (ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss1 != VHT_MCS_CAP_NA) {
			CustomerBssEntry->vht_tx_ss = 1;
			mcs_ss = ie_list->cmm_ies.vht_cap.mcs_set.tx_mcs_map.mcs_ss1;
		}

		if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss8 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 8;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss7 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 7;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss6 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 6;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss5 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 5;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 4;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 3;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 2;
		else if (ie_list->cmm_ies.vht_cap.mcs_set.rx_mcs_map.mcs_ss1 != VHT_MCS_CAP_NA)
			CustomerBssEntry->vht_rx_ss = 1;

		Antenna = CustomerBssEntry->vht_tx_ss;

		if (mcs_ss == VHT_MCS_CAP_7)
			mcs_ss = 7;
		else if (mcs_ss == VHT_MCS_CAP_8)
			mcs_ss = 8;
		else if (mcs_ss == VHT_MCS_CAP_9)
			mcs_ss = 9;
		if ((BW == BW_20) && (mcs_ss >= 8))
			mcs_ss = 8;
		MAX_MCS = mcs_ss;
	}
#endif
#ifdef DOT11_HE_AX
	if (HAS_HE_CAPS_EXIST(ie_list->cmm_ies.ie_exists)) {
		CustomerBssEntry->phy_mode |= WMODE_AX;
		MODE = MODE_HE;
		ShortGI = GI_HE_800;
		BW = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(ie_list->cmm_ies.he_caps.phy_cap.phy_capinfo_1));
		if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC000) != 0xC000) {
			Antenna = 8;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC000) >> 14;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x3000) != 0x3000) {
			Antenna = 7;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x3000) >> 12;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC00) != 0xC00) {
			Antenna = 6;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC00) >> 10;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x300) != 0x300) {
			Antenna = 5;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x300) >> 8;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC0) != 0xC0) {
			Antenna = 4;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC0) >> 6;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x30) != 0x30) {
			Antenna = 3;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x30) >> 4;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC) != 0xC) {
			Antenna = 2;
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0xC) >> 2;
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
		} else if ((ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x3) != 0x3) {
			hetmp = 0;
			hetmp = (ie_list->cmm_ies.he_caps.txrx_mcs_nss.max_tx_mcs_nss & 0x3);
			if (hetmp == HE_MCS_0_7)
				MAX_MCS = 7;
			else if (hetmp == HE_MCS_0_9)
				MAX_MCS = 9;
			else
				MAX_MCS = 11;
			Antenna = 1;
		} else
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			"%s he_caps.txrx_mcs_nss.max_tx_mcs_nss parse fail\n", __func__);
	}
#endif
	if (CustomerBssEntry->phy_mode == WMODE_INVALID)
		CustomerBssEntry->max_bit_rate = 0;
	else if (CustomerBssEntry->phy_mode == WMODE_B)
		CustomerBssEntry->max_bit_rate = 11;
	else if ((CustomerBssEntry->phy_mode & WMODE_G) ||
		 CustomerBssEntry->phy_mode == WMODE_A)
		CustomerBssEntry->max_bit_rate = 54;
	if (CustomerBssEntry->phy_mode & WMODE_GN || CustomerBssEntry->phy_mode & WMODE_AN
		|| CustomerBssEntry->phy_mode & WMODE_AC || CustomerBssEntry->phy_mode & WMODE_AX) {
		RtmpDrvMaxRateGet(pAd, MODE, ShortGI, BW, MAX_MCS, Antenna, &CustomerBssEntry->max_bit_rate);
		do_div(CustomerBssEntry->max_bit_rate, 1000000);/*use 64bits dividor*/
	}

	/* fix memory leak when trigger scan continuously*/
	if (ie_list->CustomerVendorIE.pointer && ie_list->CustomerVendorIE.length > 0) {
		int ret;
		CHAR *vendor_ie_temp;

		ret = os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, ie_list->CustomerVendorIE.length);
		if (ret == NDIS_STATUS_FAILURE) {
			/* keep the last vendor_ie pointer */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				"\n\n\nALLOC Memory fail\n\n\n");
		} else {
			os_move_mem(vendor_ie_temp, ie_list->CustomerVendorIE.pointer,
				ie_list->CustomerVendorIE.length);

			if (CustomerBssEntry->vendor_ie.pointer != NULL)
				os_free_mem(CustomerBssEntry->vendor_ie.pointer);

			CustomerBssEntry->vendor_ie.pointer = vendor_ie_temp;
	CustomerBssEntry->vendor_ie.length = ie_list->CustomerVendorIE.length;
		}
	} else {
		if (CustomerBssEntry->vendor_ie.pointer != NULL)
			os_free_mem(CustomerBssEntry->vendor_ie.pointer);
		CustomerBssEntry->vendor_ie.pointer = NULL;
		CustomerBssEntry->vendor_ie.length = 0;
	}

	RTMP_SPIN_UNLOCK(&ScanTab->event_bss_entry_lock);
}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

/*!
 *	\brief insert an entry into the bss table
 *	\param p_tab The BSS table
 *	\param Bssid BSSID
 *	\param ssid SSID
 *	\param ssid_len Length of SSID
 *	\param bss_type
 *	\param beacon_period
 *	\param timestamp
 *	\param p_cf
 *	\param atim_win
 *	\param cap
 *	\param rates
 *	\param rates_len
 *	\param channel_idx
 *	\return none
 *	\pre
 *	\post
 *	\note If SSID is identical, the old entry will be replaced by the new one

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSetEntry(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	ULONG	Idx;
#ifdef APCLI_SUPPORT
	BOOLEAN bInsert = FALSE;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	UCHAR i;
#endif /* APCLI_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif

	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	Idx = BssTableSearch(Tab, ie_list->Bssid, ie_list->Channel);

	if (Idx == BSS_NOT_FOUND) {
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE) {
			/*
				It may happen when BSS Table was full.
				The desired AP will not be added into BSS Table
				In this case, if we found the desired AP then overwrite BSS Table.
			*/
#ifdef APCLI_SUPPORT
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				pApCliEntry = &pAd->StaCfg[i];

				if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid)
					|| SSID_EQUAL(pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen, ie_list->Ssid, ie_list->SsidLen)) {
					bInsert = TRUE;
					break;
				}
			}

#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				for (i = 0; i < MAX_PROFILE_CNT; i++) {
					if(wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[i].SsidLen &&
						ie_list->SsidLen &&
						SSID_EQUAL(wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[i].ssid,
						wdev->MAPCfg.scan_bh_ssids.scan_SSID_val[i].SsidLen,
						ie_list->Ssid, ie_list->SsidLen)) {
						bInsert = TRUE;
						break;
					}
				}
			}
#endif

#endif /* APCLI_SUPPORT */

			if (
#ifdef CONFIG_STA_SUPPORT
				((pStaCfg != NULL) && !STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) ||
#endif
#ifdef CONFIG_AP_SUPPORT
				!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) ||
#endif
				!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
				if (MAC_ADDR_EQUAL(ScanCtrl->Bssid, ie_list->Bssid) ||
					SSID_EQUAL(ScanCtrl->Ssid, ScanCtrl->SsidLen, ie_list->Ssid, ie_list->SsidLen)
#ifdef APCLI_SUPPORT
					|| bInsert
#endif /* APCLI_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
					/* YF: Driver ScanTable full but supplicant the SSID exist on supplicant */
					|| SSID_EQUAL(pAd->cfg80211_ctrl.Cfg_pending_Ssid, pAd->cfg80211_ctrl.Cfg_pending_SsidLen, ie_list->Ssid,
								  ie_list->SsidLen)
#endif /* RT_CFG80211_SUPPORT */
				) {
					Idx = Tab->BssOverlapNr;

					/* fix memory leak when trigger scan continuously */
					BssEntryReset(Tab, &Tab->BssEntry[Idx]);
					BssEntrySet(pAd, wdev, &Tab->BssEntry[Idx], ie_list,
                                                    Rssi, LengthVIE, pVIE);
					Tab->BssOverlapNr += 1;
					Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE;
#ifdef RT_CFG80211_SUPPORT
					pAd->cfg80211_ctrl.Cfg_pending_SsidLen = 0;
					NdisZeroMemory(pAd->cfg80211_ctrl.Cfg_pending_Ssid, MAX_LEN_OF_SSID + 1);
#endif /* RT_CFG80211_SUPPORT */
				}

				return Idx;
			} else
				return BSS_NOT_FOUND;
		}

		Idx = Tab->BssNr;
		BssEntrySet(pAd, wdev, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE);
		Tab->BssNr++;
	} else if (Idx < MAX_LEN_OF_BSS_TABLE)
		BssEntrySet(pAd, wdev, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE);
	else
		MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s(error):Idx is larger than MAX_LEN_OF_BSS_TABLE", __func__);

	return Idx;
}

/* fix memory leak when trigger scan continuously */
VOID BssEntryReset(
	IN struct _BSS_TABLE *Tab,
	IN OUT struct _BSS_ENTRY *pBss)
{
	if (pBss) {
		UCHAR *pOldAddr = pBss->pVarIeFromProbRsp;
		USHORT VarIeFromProbeRspLen = pBss->VarIeFromProbeRspLen;

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		if (Tab)
			RTMP_SPIN_LOCK(&Tab->event_bss_entry_lock);

		if (pBss->CustomerBssEntry.vendor_ie.pointer != NULL) {
			os_free_mem(pBss->CustomerBssEntry.vendor_ie.pointer);
			pBss->CustomerBssEntry.vendor_ie.pointer = NULL;
		}
		pBss->CustomerBssEntry.vendor_ie.length = 0;

		if (Tab)
			RTMP_SPIN_UNLOCK(&Tab->event_bss_entry_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

		NdisZeroMemory(pBss, sizeof(BSS_ENTRY));
		/* initial the rssi as a minimum value */
		pBss->Rssi = -127;

		if (pOldAddr && VarIeFromProbeRspLen < MAX_VIE_LEN) {
			RTMPZeroMemory(pOldAddr, VarIeFromProbeRspLen);
			pBss->pVarIeFromProbRsp = pOldAddr;
		}
	}
}

VOID BssEntryCopy(
	IN struct _BSS_TABLE *TabDst,
	OUT struct _BSS_ENTRY *pBssDst,
	IN struct _BSS_ENTRY *pBssSrc)
{
	if (pBssDst && pBssSrc) {
		BssEntryReset(TabDst, pBssDst);

		NdisMoveMemory(pBssDst, pBssSrc, sizeof(BSS_ENTRY));
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		{
			int ret;
			CHAR *vendor_ie_temp = NULL;

			if ((pBssSrc->CustomerBssEntry.vendor_ie.pointer != NULL) &&
				(pBssSrc->CustomerBssEntry.vendor_ie.length > 0)) {
				ret = os_alloc_mem(NULL, (UCHAR **)&vendor_ie_temp,
						pBssSrc->CustomerBssEntry.vendor_ie.length);
				if (ret == NDIS_STATUS_FAILURE) {
					MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						"\n\n\nALLOC Memory fail\n\n\n");
				} else {
					os_move_mem(vendor_ie_temp,
						pBssSrc->CustomerBssEntry.vendor_ie.pointer,
						pBssSrc->CustomerBssEntry.vendor_ie.length);
				}
			}

			pBssDst->CustomerBssEntry.vendor_ie.pointer = vendor_ie_temp;
		}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
	}
}

#if (defined(CONFIG_STA_SUPPORT) || defined(WH_EZ_SETUP))
VOID BssTableSsidSort(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *OutTab,
	IN CHAR Ssid[],
	IN UCHAR SsidLen)
{
	UINT i;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef WSC_STA_SUPPORT
	PWSC_CTRL   pWpsCtrl = &pStaCfg->wdev.WscControl;
#endif /* WSC_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#if defined(DOT11W_PMF_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	struct _SECURITY_CONFIG *pSecConfig  = &wdev->SecConfig;
#endif /* defined(DOT11W_PMF_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	/* Rakesh: no need for easy enabled checks below */
	BssTableInit(OutTab);
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA) {

		if ((SsidLen == 0) &&
			(pStaCfg->bAutoConnectIfNoSSID == FALSE))
			return;
	}

#endif /* CONFIG_STA_SUPPORT */

	for (i = 0; i < ScanTab->BssNr && ScanTab->BssNr <= MAX_LEN_OF_BSS_TABLE; i++) {
		BSS_ENTRY *pInBss = &ScanTab->BssEntry[i];
		BOOLEAN	bIsHiddenApIncluded = FALSE;

#ifdef CONFIG_OWE_SUPPORT
		if (pInBss->hide_open_owe_bss) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"skip "MACSTR", SSID:%s, AKM:0x%x by OWE transition\n",
				MAC2STR(pInBss->Bssid),
				pInBss->Ssid,
				pInBss->AKMMap);
			continue;
		}
#endif


		if (((pAd->CommonCfg.bIEEE80211H == 1)
       && (pStaCfg->MlmeAux.Channel > 14)
			 && RadarChannelCheck(pAd, pInBss->Channel))
#ifdef WIFI_REGION32_HIDDEN_SSID_SUPPORT
			|| ((pInBss->Channel == 12) || (pInBss->Channel == 13))
#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier             */
			|| (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
		   ) {
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
		}

#ifdef CONFIG_STA_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

		if ((wdev->wdev_type == WDEV_TYPE_STA) &&
			!(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS))
#endif /* WPA_SUPPLICANT_SUPPORT */
		{
			/* Check the Authmode first*/
			if (((pSecConfig->AKMMap & pInBss->AKMMap) == 0)
				&& ((pSecConfig->PairwiseCipher & pInBss->PairwiseCipher) == 0)) {
				/* None matched*/
				continue;
			}
		}

#endif /* CONFIG_STA_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT

		if ((IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) || IS_AKM_WPA3PSK(pSecConfig->AKMMap) || IS_AKM_WPA3(pSecConfig->AKMMap))
			&& (pInBss)) {
			RSN_CAPABILITIES RsnCap;

			NdisMoveMemory(&RsnCap, &pInBss->RsnCapability, sizeof(RSN_CAPABILITIES));
			RsnCap.word = cpu2le16(RsnCap.word);

			/* if use WPA3/WPA3PSK, force to use pmf connection, ignore the pmf parameter in profile*/
			if (RsnCap.field.MFPC == FALSE) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_DEBUG, "[PMF]%s : Peer's MPFC isn't used.\n", __func__);
				continue;
			}
		} else if ((IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA2PSK(pSecConfig->AKMMap))
			&& (pInBss)) {
			RSN_CAPABILITIES RsnCap;

			NdisMoveMemory(&RsnCap, &pInBss->RsnCapability, sizeof(RSN_CAPABILITIES));
			RsnCap.word = cpu2le16(RsnCap.word);

			if ((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE)) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_DEBUG, "[PMF]%s : Peer's MPFC isn't used.\n", __func__);
				continue;
			}

			if (((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPC == FALSE))
				|| (pSecConfig->PmfCfg.MFPC && RsnCap.field.MFPC && (pSecConfig->PmfCfg.MFPR == FALSE) &&
					(RsnCap.field.MFPR == FALSE))) {
				if ((pSecConfig->PmfCfg.PMFSHA256) && (pInBss->IsSupportSHA256KeyDerivation == FALSE)) {
					MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_DEBUG, "[PMF]%s : Peer is not sha256.\n", __func__);
					continue;
				}
			}
		}


#endif /* DOT11W_PMF_SUPPORT */
#if (defined(DBDC_MODE) && defined(WH_EZ_SETUP))

		if ((pAd->CommonCfg.dbdc_mode) && (wdev->enable_easy_setup)) {
			if ((WMODE_2G_ONLY(wdev->PhyMode) && (pInBss->Channel <= 14)) ||
				(WMODE_5G_ONLY(wdev->PhyMode) && (pInBss->Channel > 14))) {
				/* Own band network */
			} else {
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Ignore Other Band network.\n"));
				continue;
			}
		}

#endif

		if ((((pInBss->SsidLen <= MAX_LEN_OF_SSID) && SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen)) ||
			 bIsHiddenApIncluded)
      && (pInBss->BssType == pStaCfg->BssType)
			&& (OutTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];
#ifdef CONFIG_STA_SUPPORT

			if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef WPA_SUPPLICANT_SUPPORT

				if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x80) {
					/* copy matching BSS from InTab to OutTab*/
					NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
					OutTab->BssNr++;
					continue;
				}

#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT

				if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					pWpsCtrl->bWscTrigger) {
					/* copy matching BSS from InTab to OutTab*/
					NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
					OutTab->BssNr++;
					continue;
				}

#endif /* WSC_STA_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST

				/* If no Country IE exists no Connection will be established when IEEE80211dClientMode is strict.*/
				if ((pStaCfg->IEEE80211dClientMode == Rt802_11_D_Strict) &&
					(pInBss->bHasCountryIE == FALSE)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "StaCfg.IEEE80211dClientMode == Rt802_11_D_Strict, but this AP doesn't have country IE.\n");
					continue;
				}

#endif /* EXT_BUILD_CHANNEL_LIST */
			}

#endif /* CONFIG_STA_SUPPORT */
#ifdef DOT11_N_SUPPORT

			/* 2.4G/5G N only mode*/
			if (!HAS_HT_CAPS_EXIST(pInBss->ie_exists) &&
				(WMODE_HT_ONLY(wdev->PhyMode))) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n");
				continue;
			}

			if ((wdev->PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA is in GN-only Mode, this AP is in B mode.\n");
				continue;
			}

#endif /* DOT11_N_SUPPORT */

			/* Since the AP is using hidden SSID, and we are trying to connect to ANY*/
			/* It definitely will fail. So, skip it.*/
			/* CCX also require not even try to connect it!!*/
			if (SsidLen == 0)
				continue;

			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "-------->(%d), AKMMap=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x, CapabilityInfo=0x%x\n",
					  __LINE__, pInBss->AKMMap, pInBss->PairwiseCipher, pInBss->GroupCipher, pInBss->CapabilityInfo);
			OutTab->BssNr++;
		} else if ((SsidLen == 0)
      && (pInBss->BssType == pStaCfg->BssType)
      && OutTab->BssNr < MAX_LEN_OF_BSS_TABLE) {
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];
#ifdef CONFIG_STA_SUPPORT
#ifdef WSC_STA_SUPPORT

			if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && pWpsCtrl->bWscTrigger) {
				/* copy matching BSS from InTab to OutTab*/
				NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
				OutTab->BssNr++;
				continue;
			}

#endif /* WSC_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef DOT11_N_SUPPORT

			/* 2.4G/5G N only mode*/
			if (!HAS_HT_CAPS_EXIST(pInBss->ie_exists) &&
				WMODE_HT_ONLY(wdev->PhyMode)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n");
				continue;
			}

			if ((wdev->PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA is in GN-only Mode, this AP is in B mode.\n");
				continue;
			}

#endif /* DOT11_N_SUPPORT */
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "-------->(%d), AKMMap=0x%x, PairwiseCipher=0x%x\n",
					  __LINE__, pInBss->AKMMap, pInBss->PairwiseCipher);
			OutTab->BssNr++;
		}

#if defined(CONFIG_STA_SUPPORT) && defined(WSC_STA_SUPPORT)
		else if ((wdev->wdev_type == WDEV_TYPE_STA) &&
				 (pWpsCtrl->WscConfMode != WSC_DISABLE) &&
				 (pWpsCtrl->bWscTrigger) &&
				 MAC_ADDR_EQUAL(pWpsCtrl->WscBssid, pInBss->Bssid) &&
				 OutTab->BssNr < MAX_LEN_OF_BSS_TABLE) {
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
			/*
				Linksys WRT610N WPS AP will change the SSID from linksys to linksys_WPS_<four random characters>
				when the Linksys WRT610N is in the state 'WPS Unconfigured' after set to factory default.
			*/
			NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.Ssid, pInBss->Ssid, pInBss->SsidLen);
			pStaCfg->MlmeAux.SsidLen = pInBss->SsidLen;
			/* Update Reconnect Ssid, that user desired to connect.*/
			NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
			pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->MlmeAux.SsidLen;
			OutTab->BssNr++;
			continue;
		}

#endif /* defined(CONFIG_STA_SUPPORT) && defined(WSC_STA_SUPPORT) */

		if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			break;
	}

	BssTableSortByRssi(OutTab, FALSE);
}

#endif

VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder)
{
	UINT i, j;
	BSS_ENTRY *pTmpBss = NULL;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pTmpBss, sizeof(BSS_ENTRY));

	if (pTmpBss == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	if (OutTab->BssNr == 0) {
		MTWF_DBG(NULL, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BssNr=%d!!!\n", OutTab->BssNr);
		os_free_mem(pTmpBss);
		return;
	}

	for (i = 0; i < OutTab->BssNr - 1; i++) {
		for (j = i + 1; j < OutTab->BssNr; j++) {
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi ?
				!isInverseOrder : isInverseOrder) {
				if (OutTab->BssEntry[j].Rssi != OutTab->BssEntry[i].Rssi) {
					NdisMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
				}
			}
		}
	}

	if (pTmpBss != NULL)
		os_free_mem(pTmpBss);
}

#ifdef CONFIG_STA_SUPPORT
VOID bss_table_maintenance(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN OUT BSS_TABLE *Tab,
	IN ULONG MaxRxTimeDiff,
	IN UCHAR MaxSameRxTimeCount)
{
	UINT	i, j;
	UINT	total_bssNr = Tab->BssNr;
	BOOLEAN	bDelEntry = FALSE;
	ULONG	now_time = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (total_bssNr > MAX_LEN_OF_BSS_TABLE)
		total_bssNr = MAX_LEN_OF_BSS_TABLE;
	for (i = 0; i < total_bssNr;) {
		BSS_ENTRY *pBss = &Tab->BssEntry[i];

		bDelEntry = FALSE;

		if (pBss->LastBeaconRxTimeA != pBss->LastBeaconRxTime) {
			pBss->LastBeaconRxTimeA = pBss->LastBeaconRxTime;
			pBss->SameRxTimeCount = 0;
		} else
			pBss->SameRxTimeCount++;

		NdisGetSystemUpTime(&now_time);

		if (RTMP_TIME_AFTER(now_time, pBss->LastBeaconRxTime + (MaxRxTimeDiff * OS_HZ)))
			bDelEntry = TRUE;
		else if (pBss->SameRxTimeCount > MaxSameRxTimeCount)
			bDelEntry = TRUE;

		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)
			&& NdisEqualMemory(pBss->Ssid, pStaCfg->Ssid, pStaCfg->SsidLen))
			bDelEntry = FALSE;

		if (bDelEntry) {
			UCHAR *pOldAddr = NULL;

			for (j = i; j < total_bssNr - 1; j++) {
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				/* delete this entry and use the behind entry replace this entry*/
				BssEntryCopy(Tab, &(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]));

				if (pOldAddr) {
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr,
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp,
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}

			/*clear the last one entry*/
			BssEntryReset(Tab, &Tab->BssEntry[Tab->BssNr - 1]);

			total_bssNr -= 1;
		} else {
			i++;
		}
	}

	Tab->BssNr = total_bssNr;
}
#endif /* CONFIG_STA_SUPPORT */


BOOLEAN bss_coex_insert_effected_ch_list(
		RTMP_ADAPTER *pAd, UCHAR Channel,
		BCN_IE_LIST *ie_list,
		struct wifi_dev *wdev)
{
	BOOLEAN Inserted = FALSE;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.bOverlapScanning == TRUE) {
		INT index, secChIdx;
		ADD_HTINFO *pAdd_HtInfo;
		struct freq_oper oper;
		UCHAR BandIdx;
		CHANNEL_CTRL *pChCtrl;
		struct common_ies *cmm_ies = &ie_list->cmm_ies;

		os_zero_mem(&oper, sizeof(oper));
		hc_radio_query_by_channel(pAd, Channel, &oper);

		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (index = 0; index < pChCtrl->ChListNum; index++) {
			/* found the effected channel, mark that. */
			if (pChCtrl->ChList[index].Channel == ie_list->Channel) {
				secChIdx = -1;

				if (HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) && HAS_HT_OP_EXIST(cmm_ies->ie_exists)) {
					/* This is a 11n AP. */
					pChCtrl->ChList[index].bEffectedChannel |= EFFECTED_CH_PRIMARY; /* 2;	// 2 for 11N 20/40MHz AP with primary channel set as this channel. */
					pAdd_HtInfo = &cmm_ies->ht_op.AddHtInfo;

					if (pAdd_HtInfo->ExtChanOffset == EXTCHA_BELOW) {
						if (ie_list->Channel > 14)
							secChIdx = ((index > 0) ? (index - 1) : -1);
						else
							secChIdx = ((index >= 4) ? (index - 4) : -1);
					} else if (pAdd_HtInfo->ExtChanOffset == EXTCHA_ABOVE) {
						if (ie_list->Channel > 14)
							secChIdx = (((index + 1) < pChCtrl->ChListNum) ? (index + 1) : -1);
						else
							secChIdx = (((index + 4) < pChCtrl->ChListNum) ? (index + 4) : -1);
					}

					if (secChIdx >= 0)
						pChCtrl->ChList[secChIdx].bEffectedChannel |= EFFECTED_CH_SECONDARY; /* 1; */

					if ((Channel == ie_list->Channel) || ((Channel != ie_list->Channel)
						 && (pAdd_HtInfo->ExtChanOffset != oper.ext_cha))) {
						pAd->CommonCfg.BssCoexApCnt++;
						Inserted = TRUE;
					}
				} else {
					/* This is a legacy AP. */
					pChCtrl->ChList[index].bEffectedChannel |=  EFFECTED_CH_LEGACY; /* 4; 1 for legacy AP. */
					pAd->CommonCfg.BssCoexApCnt++;
					Inserted = TRUE;
				}
			}
		}
	}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	return Inserted;
}



