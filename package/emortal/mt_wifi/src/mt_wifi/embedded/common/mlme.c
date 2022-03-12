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
	mlme.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include <stdarg.h>
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

UCHAR OfdmRateToRxwiMCS[12] = {
	0,  0,	0,  0,
	0,  1,	2,  3,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};

UCHAR RxwiMCSToOfdmRate[12] = {
	RATE_6,  RATE_9,	RATE_12,  RATE_18,
	RATE_24,  RATE_36,	RATE_48,  RATE_54,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};

extern UCHAR ZeroSsid[MAX_LEN_OF_SSID];


/* since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.*/
/* otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate*/
ULONG BasicRateMask[12] = {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
						   0xfffff01f /* 6 */, 0xfffff03f /* 9 */, 0xfffff07f /* 12 */, 0xfffff0ff /* 18 */,
						   0xfffff1ff /* 24 */, 0xfffff3ff /* 36 */, 0xfffff7ff /* 48 */, 0xffffffff /* 54 */
						  };

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than*/
/*		this value, then it's quaranteed capable of operating in 36 mbps TX rate in*/
/*		clean environment.*/
/*								  TxRate: 1   2   5.5	11	 6	  9    12	18	 24   36   48	54	 72  100*/
CHAR RssiSafeLevelForTxRate[] = {  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[] = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR SsidIe = IE_SSID;
UCHAR SupRateIe = IE_SUPP_RATES;
UCHAR ExtRateIe = IE_EXT_SUPP_RATES;
#ifdef DOT11_N_SUPPORT
UCHAR HtCapIe = IE_HT_CAP;
UCHAR AddHtInfoIe = IE_ADD_HT;
UCHAR NewExtChanIe = IE_SECONDARY_CH_OFFSET;
UCHAR BssCoexistIe = IE_2040_BSS_COEXIST;
UCHAR ExtHtCapIe = IE_EXT_CAPABILITY;
#endif /* DOT11_N_SUPPORT */
UCHAR ExtCapIe = IE_EXT_CAPABILITY;
UCHAR ErpIe = IE_ERP;
UCHAR DsIe = IE_DS_PARM;
UCHAR TimIe = IE_TIM;
UCHAR WpaIe = IE_WPA;
UCHAR Wpa2Ie = IE_WPA2;
UCHAR IbssIe = IE_IBSS_PARM;
UCHAR WapiIe = IE_WAPI;


static VOID Update_Mib_Bucket_One_Sec(RTMP_ADAPTER *pAd)
{
	UCHAR   i = 0, j = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	for (i = 0 ; i < concurrent_bands ; i++) {
		if (pAd->OneSecMibBucket.Enabled[i] == TRUE) {
			pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] = 0;
			pAd->OneSecMibBucket.ChannelBusyTime[i] = 0;
			pAd->OneSecMibBucket.OBSSAirtime[i] = 0;
			pAd->OneSecMibBucket.MyTxAirtime[i] = 0;
			pAd->OneSecMibBucket.MyRxAirtime[i] = 0;
			pAd->OneSecMibBucket.EDCCAtime[i] =  0;
			pAd->OneSecMibBucket.MdrdyCount[i] = 0;
			pAd->OneSecMibBucket.PdCount[i] = 0;
			for (j = 0 ; j < 2 ; j++) {
				pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] += pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][j];
				pAd->OneSecMibBucket.ChannelBusyTime[i] += pAd->MsMibBucket.ChannelBusyTime[i][j];
				pAd->OneSecMibBucket.OBSSAirtime[i] += pAd->MsMibBucket.OBSSAirtime[i][j];
				pAd->OneSecMibBucket.MyTxAirtime[i] += pAd->MsMibBucket.MyTxAirtime[i][j];
				pAd->OneSecMibBucket.MyRxAirtime[i] += pAd->MsMibBucket.MyRxAirtime[i][j];
				pAd->OneSecMibBucket.EDCCAtime[i] += pAd->MsMibBucket.EDCCAtime[i][j];
				pAd->OneSecMibBucket.MdrdyCount[i] += pAd->MsMibBucket.MdrdyCount[i][j];
				pAd->OneSecMibBucket.PdCount[i] += pAd->MsMibBucket.PdCount[i][j];
			}
		}
	}
}


VOID set_default_ap_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid = TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 1;
	pEdca->Aifsn[3] = 1;
	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;
	pEdca->Cwmax[0] = 6;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;
	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;
	pEdca->Txop[3]  = 47;
}


VOID set_default_sta_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid = TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 2;
	pEdca->Aifsn[3] = 2;
	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;
	pEdca->Cwmax[0] = 10;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;
	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;	/*96; */
	pEdca->Txop[3]  = 47;	/*48; */
}


UCHAR dot11_max_sup_rate(struct legacy_rate *rate)
{
	INT idx;
	UCHAR MaxSupportedRateIn500Kbps = 0;
	UINT8 sup_rate_len, ext_rate_len, *sup_rate, *ext_rate;

	sup_rate_len = rate->sup_rate_len;
	ext_rate_len = rate->ext_rate_len;
	sup_rate = rate->sup_rate;
	ext_rate = rate->ext_rate;

	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (idx = 0; idx < sup_rate_len; idx++) {
		if (MaxSupportedRateIn500Kbps < (sup_rate[idx] & 0x7f))
			MaxSupportedRateIn500Kbps = sup_rate[idx] & 0x7f;
	}

	if (ext_rate_len > 0 && ext_rate != NULL) {
		for (idx = 0; idx < ext_rate_len; idx++) {
			if (MaxSupportedRateIn500Kbps < (ext_rate[idx] & 0x7f))
				MaxSupportedRateIn500Kbps = ext_rate[idx] & 0x7f;
		}
	}

	return MaxSupportedRateIn500Kbps;
}


UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps)
{
	UCHAR MaxSupportedRate;

	switch (MaxSupportedRateIn500Kbps) {
	case 108:
		MaxSupportedRate = RATE_54;
		break;

	case 96:
		MaxSupportedRate = RATE_48;
		break;

	case 72:
		MaxSupportedRate = RATE_36;
		break;

	case 48:
		MaxSupportedRate = RATE_24;
		break;

	case 36:
		MaxSupportedRate = RATE_18;
		break;

	case 24:
		MaxSupportedRate = RATE_12;
		break;

	case 18:
		MaxSupportedRate = RATE_9;
		break;

	case 12:
		MaxSupportedRate = RATE_6;
		break;

	case 22:
		MaxSupportedRate = RATE_11;
		break;

	case 11:
		MaxSupportedRate = RATE_5_5;
		break;

	case 4:
		MaxSupportedRate = RATE_2;
		break;

	case 2:
		MaxSupportedRate = RATE_1;
		break;

	default:
		MaxSupportedRate = RATE_11;
		break;
	}

	return MaxSupportedRate;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPSuspendMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCANNING, suspend MSDU transmission ...\n"));
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */

	/* no carrier detection when scanning */
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStop(pAd);

#endif
#endif /* CONFIG_AP_SUPPORT */
	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);
	MSDU_FORBID_SET(wdev, MSDU_FORBID_CHANNEL_MISMATCH);
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPResumeMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct qm_ops *qm_ops = pAd->qm_ops;
	UINT8 idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, 0, 0);

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCAN done, resume MSDU transmission ...\n"));
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT

	/* no carrier detection when scanning*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0) {
		pAd->BbpTuning.R66CurrentValue = 0x38;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}

	bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);
	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CHANNEL_MISMATCH);

	qm_ops->schedule_tx_que(pAd, idx);
}


/*
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UINT16 AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, pAddr);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, pAddr, NULL);
#endif
	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;

	if (NState == NDIS_STATUS_SUCCESS) {
		frm_len = sizeof(HEADER_802_11);
#ifdef CONFIG_AP_SUPPORT

		/* IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
		if (pEntry && (pEntry->wdev->wdev_type == WDEV_TYPE_AP
					   || pEntry->wdev->wdev_type == WDEV_TYPE_GO)) {
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							 pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							 pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			goto body;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pNullFr->FC.FrDs = 0;
			pNullFr->FC.ToDs = 1;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			if ((pEntry != NULL) && (IS_ENTRY_TDLS(pEntry))) {
				pNullFr->FC.FrDs = 0;
				pNullFr->FC.ToDs = 0;
				COPY_MAC_ADDR(pNullFr->Addr1, pAddr);
				COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);
			}

#endif /* DOT11Z_TDLS_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
body:
#endif /* CONFIG_AP_SUPPORT */
		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull) {
			UCHAR *qos_p = ((UCHAR *)pNullFr) + frm_len;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;
			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			frm_len += 2;
		} else
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate],
				 AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (PUCHAR)pNullFr, frm_len);
		MlmeFreeMemory(pFrame);
	}
}


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
/*APPS DVT*/
VOID ApCliRTMPSendPsPollFrame(
	IN	PRTMP_ADAPTER    pAd,
	IN	UINT_8  index)
{
	PSPOLL_FRAME PsPollFrame;
	PPSPOLL_FRAME pPsPollFrame;
	PUCHAR pOutBuffer = NULL;
	UINT Length;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	struct wifi_dev *wdev;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	pPsPollFrame = &PsPollFrame;

	pApCliEntry = &pAd->StaCfg[index];
	wdev = &pApCliEntry->wdev;
	pMacEntry = MacTableLookup(pAd, wdev->bssid);
	Length = sizeof(PSPOLL_FRAME);

	if (!pMacEntry)
		return;

	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
		(tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(pPsPollFrame, Length);
	pPsPollFrame->FC.Type = FC_TYPE_CNTL;
	pPsPollFrame->FC.SubType = SUBTYPE_PS_POLL;
	pPsPollFrame->FC.PwrMgmt = PWR_SAVE;
	pPsPollFrame->Aid = pApCliEntry->MlmeAux.Aid | 0xC000;
	COPY_MAC_ADDR(pPsPollFrame->Bssid, pApCliEntry->MlmeAux.Bssid);
	COPY_MAC_ADDR(pPsPollFrame->Ta, pApCliEntry->wdev.if_addr);
	pOutBuffer = (PUCHAR)pPsPollFrame;
	hif_kickout_nullframe_tx(pAd, 0, pOutBuffer, Length);
}

VOID AppsApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN		bQosNull,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	IN	USHORT			PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	struct wifi_dev *wdev;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;


	pApCliEntry = GetStaCfgByWdev(pAd, pMacEntry->wdev);
	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];
	wdev = &pApCliEntry->wdev;

	/* WPA 802.1x secured port control */
	/* TODO: shiang-usw, check [wdev/tr_entry]->PortSecured! */
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
		(tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);
	wifi_hdr = (HEADER_802_11 *)NullFrame;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = SUBTYPE_DATA_NULL;
	wifi_hdr->FC.ToDs = 1;
	COPY_MAC_ADDR(wifi_hdr->Addr1, pMacEntry->Addr);
	COPY_MAC_ADDR(wifi_hdr->Addr2, pApCliEntry->wdev.if_addr);

	COPY_MAC_ADDR(wifi_hdr->Addr3, pMacEntry->Addr);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = PwrMgmt;

	if (pApCliEntry->PwrSaveSet == TRUE) {
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	} else {
		wifi_hdr->FC.PwrMgmt = PwrMgmt;
	}

	wifi_hdr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);
	/* sequence is increased in MlmeHardTx */
	wifi_hdr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		wifi_hdr->FC.SubType = SUBTYPE_QOS_NULL;
		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	hif_kickout_nullframe_tx(pAd, 0, NullFrame, Length);
}


VOID ApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN		bQosNull,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	IN	USHORT			PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG sta = NULL;
	struct wifi_dev *wdev;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (!pMacEntry) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): pMacEntry is null!", __func__));
		return;
	}

	wdev = pMacEntry->wdev;
	sta = GetStaCfgByWdev(pAd, wdev);
	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];

#ifdef AUTOMATION
	if (sta->PwrSaveSet == TRUE)
		return;
#endif /* AUTOMATION */

	/* WPA 802.1x secured port control */
	/* TODO: shiang-usw, check [wdev/tr_entry]->PortSecured! */
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
		(tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);
	wifi_hdr = (HEADER_802_11 *)NullFrame;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = SUBTYPE_DATA_NULL;
	wifi_hdr->FC.ToDs = 1;
	COPY_MAC_ADDR(wifi_hdr->Addr1, pMacEntry->Addr);
	COPY_MAC_ADDR(wifi_hdr->Addr2, wdev->if_addr);
	COPY_MAC_ADDR(wifi_hdr->Addr3, pMacEntry->Addr);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = PwrMgmt;

	wifi_hdr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);
	/* sequence is increased in MlmeHardTx */
	wifi_hdr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		wifi_hdr->FC.SubType = SUBTYPE_QOS_NULL;
		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	hif_kickout_nullframe_tx(pAd, 0, NullFrame, Length);
}
#endif/*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID LowPowerDebug(
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n************ Previous Setting **************************\n"));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pStaCfg(0x%p)\n", __func__, pStaCfg));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pStaCfg->WindowsBatteryPowerMode(%lu)\n", __func__,
			 pStaCfg->WindowsBatteryPowerMode));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pStaCfg->WindowsPowerMode(%lu)\n", __func__,
			 pStaCfg->WindowsPowerMode));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pAd->CommonCfg.bAPSDForcePowerSave(%d)\n", __func__,
			 pAd->CommonCfg.bAPSDForcePowerSave));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pStaCfg->PwrMgmt.bDoze(%d)\n", __func__,
			 pStaCfg->PwrMgmt.bDoze));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::pStaCfg->PwrMgmt.psm(%d)\n", __func__,
			 pStaCfg->PwrMgmt.Psm));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("*****************************************************\n\n"));
}

VOID RTMPSendNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UCHAR TxRate,
	IN BOOLEAN bQosNull,
	IN USHORT PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	/* STA_TR_ENTRY *tr_entry; */
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	STA_ADMIN_CONFIG *pStaCfg = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	ASSERT(pMacEntry);
	if (!pMacEntry)
		return;

	wdev = pMacEntry->wdev;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];

	/* WPA 802.1x secured port control */
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
		(tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);
	ComposeNullFrame(pAd, (PHEADER_802_11)&NullFrame[0],
					 pMacEntry->Addr, wdev->if_addr, pMacEntry->Addr);
	wifi_hdr = (HEADER_802_11 *)&NullFrame[0];

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else {
		wifi_hdr->FC.PwrMgmt = PwrMgmt;
#ifdef DOT11Z_TDLS_SUPPORT

		/* check TDLS condition */
		if (pAd->StaCfg[0].TdlsInfo.TdlsFlgIsKeepingActiveCountDown == TRUE)
			wifi_hdr->FC.PwrMgmt = PWR_ACTIVE;

#endif /* DOT11Z_TDLS_SUPPORT */
	}

	wifi_hdr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);
	/* sequence is increased in MlmeHardTx */
	wifi_hdr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		wifi_hdr->FC.SubType = SUBTYPE_QOS_NULL;
		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	hif_kickout_nullframe_tx(pAd, 0, NullFrame, Length);
}

VOID RTMPOffloadPm(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState)
{
	ASSERT(pStaCfg);
	if (!pStaCfg)
		return;
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s--->wcid(%d), ucPmNumber(%d), ucPmState(%d)\n",
			 __func__,
			 pStaCfg->PwrMgmt.wcid,
			 ucPmNumber,
			 ucPmState));

	switch (ucPmNumber) {
	case PM4: {
		LowPowerDebug(pAd, pStaCfg);

		if ((!pStaCfg->PwrMgmt.bDoze) && (ucPmState == ENTER_PM_STATE)) {
			/* Set Driver side protocol PS control */
			RTMP_SET_PSM_BIT(pAd, pStaCfg, PWR_SAVE);
			/* H/W enter PM4 offlaod here */
			MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::***** H/W Enter PM4 *****\n", __func__));
			AsicExtPmStateCtrl(pAd, pStaCfg, ucPmNumber, ucPmState);
			pStaCfg->PwrMgmt.bDoze = TRUE;
		} else if (pStaCfg->PwrMgmt.bDoze && (ucPmState == EXIT_PM_STATE)) {
			/* Set Driver side protocol PS control */
			RTMP_SET_PSM_BIT(pAd, pStaCfg, PWR_ACTIVE);
			/* H/W exit PM4 offlaod here */
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::***** H/W Exit PM4 *****\n", __func__));
#ifdef CONFIG_STA_SUPPORT
			NdisGetSystemUpTime(&pStaCfg->LastBeaconRxTime);
#endif
			AsicExtPmStateCtrl(pAd, pStaCfg, ucPmNumber, ucPmState);
			pStaCfg->PwrMgmt.bDoze = FALSE;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::***** No Need to handle this. *****\n", __func__));
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Unknown PM mode, ERROR!\n", __func__));
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<---\n", __func__));
}

VOID RTMPWakeUpWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (!pStaCfg)
		return;

	if (pStaCfg->CountDowntoPsm < STAY_2_SECONDS_AWAKE)
		pStaCfg->CountDowntoPsm = STAY_2_SECONDS_AWAKE;

	if ((pStaCfg->WindowsPowerMode != Ndis802_11PowerModeLegacy_PSP) &&
		(pStaCfg->WindowsPowerMode != Ndis802_11PowerModeCAM) &&
		pStaCfg->PwrMgmt.bDoze) {
		RTMP_FORCE_WAKEUP(pAd, pStaCfg);
	}
}

#endif /* CONFIG_STA_SUPPORT */

static VOID mlme_suspend(struct _MLME_STRUCT *mlme)
{
	NdisAcquireSpinLock(&mlme->TaskLock);
	mlme->suspend = TRUE;
	NdisReleaseSpinLock(&mlme->TaskLock);
}

static VOID mlme_resume(struct _MLME_STRUCT *mlme)
{
	NdisAcquireSpinLock(&mlme->TaskLock);
	mlme->suspend = FALSE;
	NdisReleaseSpinLock(&mlme->TaskLock);
}

static BOOLEAN mlme_requeue(MLME_QUEUE *Queue, struct _MLME_QUEUE_ELEM *elem)
{
	ULONG tail;
	NdisAcquireSpinLock(&(Queue->Lock));
	tail = Queue->Tail;

	/*Double check for safety in multi-thread system*/
	if (Queue->Entry[tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == Queue->MaxLen)
		Queue->Tail = 0;

	os_move_mem(&Queue->Entry[tail], elem, sizeof(Queue->Entry[tail]));
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


static INT mlme_bss_clear_by_wdev(struct _MLME_STRUCT *mlme, struct wifi_dev *wdev)
{
	struct _MLME_QUEUE_ELEM *elem = NULL;
	MLME_QUEUE *pQueue;
	INT elem_num;
	INT i;
	UCHAR idx;
	for (idx = 0; idx < MAX_NUM_OF_MLME_QUEUE; idx++) {
		switch (idx) {
			case 0:
				pQueue = &mlme->Queue;
				break;
#ifdef MLME_MULTI_QUEUE_SUPPORT
			case 1:
				pQueue = (MLME_QUEUE *) &mlme->HPQueue;
				break;
			case 2:
				pQueue = (MLME_QUEUE *) &mlme->LPQueue;
				break;
#endif
			default:
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No mlme queue matched!, idx = %d\n", __func__, idx));
				break;
		}
		elem_num = pQueue->Num;
		NdisAcquireSpinLock(&mlme->TaskLock);
		for (i = 0 ; i < elem_num ; i++) {
			if (!MlmeDequeue(pQueue, &elem))
				break;
			/*if not owned by this bss, enqueue again*/
			if (elem->wdev != wdev)
				mlme_requeue(pQueue, elem);
			/* free MLME element*/
			elem->Occupied = FALSE;
			elem->MsgLen = 0;
		}
		NdisReleaseSpinLock(&mlme->TaskLock);
	}
	return TRUE;
}

static BOOLEAN mlme_bss_clear(struct _MLME_STRUCT *mlme, struct wifi_dev *wdev)
{
	INT cnt = 0;
	BOOLEAN ret = TRUE;
	/*suspend mlme a while*/
	mlme_suspend(mlme);
	/*check mlme is idle*/
	while (mlme->bRunning && cnt < 10) {
		OS_WAIT(100);
		cnt++;
	}
	/*check mlme polling idle status*/
	if (mlme->bRunning) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("mlme can't polling to idle, timeout\n"));
		ret = FALSE;
		goto end;
	}
	/*clear bss related mlme entry*/
	mlme_bss_clear_by_wdev(mlme, wdev);
	/*resume mlme now*/
end:
	mlme_resume(mlme);
	return ret;
}
#ifdef TXRX_STAT_SUPPORT
VOID Update_LastSec_TXRX_Stats(
	IN PRTMP_ADAPTER   pAd)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT32 i, bandidx;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		ctrl->rdev[i].pRadioCtrl->LastSecTxByte.QuadPart = 0;
		ctrl->rdev[i].pRadioCtrl->LastSecRxByte.QuadPart = 0;
	}
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pAd->ApCfg.MBSSID[i].stat_bss.LastSecTxBytes.QuadPart = 0;
		pAd->ApCfg.MBSSID[i].stat_bss.LastSecRxBytes.QuadPart = 0;
	}
	for (i = 0 ; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];
#ifdef WIFI_IAP_STA_DUMP_FEATURE
		if (pEntry && pEntry->wdev && (pEntry->Sst == SST_ASSOC) &&
			(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)))
#else/*WIFI_IAP_STA_DUMP_FEATURE*/
		if (pEntry && pEntry->wdev && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
#endif/*NO WIFI_IAP_STA_DUMP_FEATURE*/
		{
			bandidx = HcGetBandByWdev(pEntry->wdev);
			if (pEntry->TxDataPacketCount.QuadPart >= pEntry->LastTxDataPacketCountValue.QuadPart)
				pEntry->TxDataPacketCount1SecValue.QuadPart = pEntry->TxDataPacketCount.QuadPart - pEntry->LastTxDataPacketCountValue.QuadPart;
			if (pEntry->TxDataPacketByte.QuadPart >= pEntry->LastTxDataPacketByteValue.QuadPart) {
				pEntry->TxDataPacketByte1SecValue.QuadPart = pEntry->TxDataPacketByte.QuadPart - pEntry->LastTxDataPacketByteValue.QuadPart;
				ctrl->rdev[bandidx].pRadioCtrl->LastSecTxByte.QuadPart += pEntry->TxDataPacketByte1SecValue.QuadPart;
				if (pEntry->pMbss)
				pEntry->pMbss->stat_bss.LastSecTxBytes.QuadPart += pEntry->TxDataPacketByte1SecValue.QuadPart;
			}
			if (pEntry->RxDataPacketCount.QuadPart >= pEntry->LastRxDataPacketCountValue.QuadPart)
				pEntry->RxDataPacketCount1SecValue.QuadPart = pEntry->RxDataPacketCount.QuadPart - pEntry->LastRxDataPacketCountValue.QuadPart;
			if (pEntry->RxDataPacketByte.QuadPart >= pEntry->LastRxDataPacketByteValue.QuadPart) {
				pEntry->RxDataPacketByte1SecValue.QuadPart = pEntry->RxDataPacketByte.QuadPart - pEntry->LastRxDataPacketByteValue.QuadPart;
				ctrl->rdev[bandidx].pRadioCtrl->LastSecRxByte.QuadPart += pEntry->RxDataPacketByte1SecValue.QuadPart;
				if (pEntry->pMbss)
				pEntry->pMbss->stat_bss.LastSecRxBytes.QuadPart += pEntry->RxDataPacketByte1SecValue.QuadPart;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : STA : %d \n", __func__, pEntry->wcid));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("1) TotalTxCount: %lld LastTxCnt: %lld TxPktCount1Sec: %lld\n", pEntry->TxDataPacketCount.QuadPart, pEntry->LastTxDataPacketCountValue.QuadPart, pEntry->TxDataPacketCount1SecValue.QuadPart));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("2) TotalRxCount: %lld LastRxCnt: %lld RxPktCount1Sec: %lld\n", pEntry->RxDataPacketCount.QuadPart, pEntry->LastRxDataPacketCountValue.QuadPart, pEntry->RxDataPacketCount1SecValue.QuadPart));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("3) TotalTxByte : %lld LastTxByte: %lld TxByte1secValue: %lld\n", pEntry->TxDataPacketByte.QuadPart, pEntry->LastTxDataPacketByteValue.QuadPart, pEntry->TxDataPacketByte1SecValue.QuadPart));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("4) TotalRxByte : %lld LastRxByte: %lld RxByte1secValue: %lld\n", pEntry->RxDataPacketByte.QuadPart, pEntry->LastRxDataPacketByteValue.QuadPart, pEntry->RxDataPacketByte1SecValue.QuadPart));
			pEntry->LastTxDataPacketByteValue.QuadPart = pEntry->TxDataPacketByte.QuadPart;
			pEntry->LastTxDataPacketCountValue.QuadPart = pEntry->TxDataPacketCount.QuadPart;
			pEntry->LastRxDataPacketByteValue.QuadPart = pEntry->RxDataPacketByte.QuadPart;
			pEntry->LastRxDataPacketCountValue.QuadPart = pEntry->RxDataPacketCount.QuadPart;
		}
	}
}
#endif

/*
	==========================================================================
	Description:
		main loop of the MLME
	Pre:
		Mlme has to be initialized, and there are something inside the queue
	Note:
		This function is invoked from MPSetInformation and MPReceive;
		This task guarantee only one FSM will run.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeHandler(RTMP_ADAPTER *pAd)
{
	MLME_QUEUE_ELEM *Elem = NULL;
	UINT32 process_cnt = 0;
	/* Only accept MLME and Frame from peer side, no other (control/data) frame should*/
	/* get into this state machine*/
	MLME_QUEUE * Queue = &pAd->Mlme.Queue;
#ifdef MLME_MULTI_QUEUE_SUPPORT
	BOOLEAN HighPrioQEmpty = TRUE;
	BOOLEAN LowPrioQEmpty = TRUE;
	BOOLEAN NormalPrioQEmpty = TRUE;

	UINT32 HighPrioQDeqCnt = 0;
	UINT32 LowPrioQDeqCnt = 0;
	UINT32 NormalPrioQDeqCnt = 0;
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);

	if (pAd->Mlme.bRunning) {
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} else {
		pAd->Mlme.bRunning = TRUE;
	}

	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	while (
#ifdef MLME_MULTI_QUEUE_SUPPORT
	 pAd->Mlme.MultiQEnable ||
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
	!MlmeQueueEmpty(&pAd->Mlme.Queue)) {
#ifdef MLME_MULTI_QUEUE_SUPPORT
		if (pAd->Mlme.MultiQEnable) {
			HighPrioQEmpty = MlmeQueueEmpty((MLME_QUEUE *)&pAd->Mlme.HPQueue) ? TRUE : FALSE;
			NormalPrioQEmpty = MlmeQueueEmpty(&pAd->Mlme.Queue) ? TRUE : FALSE;
			LowPrioQEmpty = MlmeQueueEmpty((MLME_QUEUE *)&pAd->Mlme.LPQueue) ? TRUE : FALSE;
			if (HighPrioQEmpty && LowPrioQEmpty && NormalPrioQEmpty) {
				break;
			} else if (!HighPrioQEmpty && (HighPrioQDeqCnt < pAd->Mlme.HPQueue.Ration || (LowPrioQEmpty && NormalPrioQEmpty))) {
				Queue = (MLME_QUEUE *) &pAd->Mlme.HPQueue;
				HighPrioQDeqCnt++;
			} else if (!NormalPrioQEmpty && (NormalPrioQDeqCnt < pAd->Mlme.Queue.Ration || (LowPrioQEmpty && HighPrioQEmpty))) {
				Queue = (MLME_QUEUE *) &pAd->Mlme.Queue;
				NormalPrioQDeqCnt++;
			} else if (!LowPrioQEmpty && (LowPrioQDeqCnt < pAd->Mlme.LPQueue.Ration || (NormalPrioQEmpty && HighPrioQEmpty))) {
				Queue = (MLME_QUEUE *) &pAd->Mlme.LPQueue;
				LowPrioQDeqCnt++;
			} else {
				HighPrioQDeqCnt = 0;
				NormalPrioQDeqCnt = 0;
				LowPrioQDeqCnt = 0;
				continue;
			}
		}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND) ||
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
		   ) {
#ifndef MLME_MULTI_QUEUE_SUPPORT
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("System halted, removed or MlmeRest, exit MlmeTask!(QNum = %ld)\n",
				  pAd->Mlme.Queue.Num));
#else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("System halted, removed or MlmeRest, exit MlmeTask!(QNum = %ld, HPQNum = %ld, LPQNum = %ld)\n",
				  pAd->Mlme.Queue.Num, pAd->Mlme.HPQueue.Num, pAd->Mlme.LPQueue.Num));
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
			break;
		}

#ifdef CONFIG_ATE

		if (ATE_ON(pAd)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Driver is in ATE mode\n", __func__));
			break;
		}

#endif /* CONFIG_ATE */

		/* For worst case, avoid process mlme.queue too long which cause RCU_sched stall */
		process_cnt++;
		if ((!in_interrupt()) && (process_cnt >= (MLME_QUEUE_SCH>>2))) {/*should avoid schedule too frequently*/
			process_cnt = 0;
			OS_SCHEDULE();
		}

		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(Queue, &Elem)) {
			struct wifi_dev *wdev = Elem->wdev;
#ifdef CONFIG_STA_SUPPORT
			PSTA_ADMIN_CONFIG pStaCfg = NULL;

			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				pStaCfg = GetStaCfgByWdev(pAd, wdev);
			}
#endif

			/* if dequeue success*/
			switch (Elem->Machine) {

			case MLME_CNTL_STATE_MACHINE: {
				ULONG CtrlCurrState;
					CtrlCurrState = wdev->cntl_machine.CurrState;

				StateMachinePerformAction(pAd, &wdev->cntl_machine,
										  Elem, CtrlCurrState);
			}
				break;

			case SYNC_FSM: {
				SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
				/* add for multi wdev reuse sync state machine,
				add independent sync state machinethe for all wdev */
				UCHAR i = 0;
				BOOLEAN bToApCli = FALSE;

				for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
					if (wdev == &pAd->StaCfg[i].wdev) {
						bToApCli = TRUE;
						break;
					}
				}

				if( (ScanCtrl->SyncFsm.CurrState != SYNC_FSM_IDLE) &&
					(wdev != ScanCtrl->CurOccupyWdev) &&
					(bToApCli == FALSE) && wdev->sync_machine_init)
					StateMachinePerformAction(pAd, &wdev->sync_machine,
											  Elem, wdev-> sync_machine.CurrState);
				else
					StateMachinePerformAction(pAd, &ScanCtrl->SyncFsm,
											  Elem, ScanCtrl->SyncFsm.CurrState);
				break;
			}

			case AUTH_FSM: {
				ULONG AuthCurrState;
					AuthCurrState = wdev->auth_machine.CurrState;
				StateMachinePerformAction(pAd, &wdev->auth_machine,
										  Elem, AuthCurrState);
				break;
			}

			case ASSOC_FSM: {
				ULONG AssocCurrState;
					AssocCurrState = wdev->assoc_machine.CurrState;
				StateMachinePerformAction(pAd, &wdev->assoc_machine,
										  Elem, AssocCurrState);
				break;
			}


				/* STA state machines*/
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT

			case FT_OTA_AUTH_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.FtOtaAuthMachine,
										  Elem, pAd->Mlme.FtOtaAuthMachine.CurrState);
				break;

			case FT_OTD_ACT_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.FtOtdActMachine,
										  Elem, pAd->Mlme.FtOtdActMachine.CurrState);
				break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT

			case TDLS_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.TdlsMachine,
										  Elem, pAd->Mlme.TdlsMachine.CurrState);
				break;
#endif /* DOT11Z_TDLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

			case ACTION_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine,
										  Elem, pAd->Mlme.ActMachine.CurrState);
				break;

			case WPA_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WpaMachine, Elem, pAd->Mlme.WpaMachine.CurrState);
				break;
#ifdef WSC_INCLUDED

			case WSC_STATE_MACHINE:
				if (pAd->pWscElme) {
					RTMP_SEM_LOCK(&pAd->WscElmeLock);
					NdisMoveMemory(pAd->pWscElme, Elem, sizeof(MLME_QUEUE_ELEM));
					RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
					RtmpOsTaskWakeUp(&(pAd->wscTask));
				}

				break;
#ifdef IWSC_SUPPORT

			case IWSC_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.IWscMachine, Elem, pAd->Mlme.IWscMachine.CurrState);
				break;
#endif /* IWSC_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef P2P_SUPPORT

			case P2P_CTRL_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PCtrlMachine, Elem,
										  pAd->P2pCfg.CtrlCurrentState);
				break;

			case P2P_DISC_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PDiscMachine, Elem,
										  pAd->P2pCfg.DiscCurrentState);
				break;

			case P2P_GO_FORM_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PGoFormMachine, Elem,
										  pAd->P2pCfg.GoFormCurrentState);
				break;

			case P2P_ACTION_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PActionMachine, Elem,
										  pAd->P2pCfg.ActionState);
				break;
#endif /* P2P_SUPPORT */
#ifdef CONFIG_HOTSPOT

			case HSCTRL_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.HSCtrlMachine, Elem,
										  HSCtrlCurrentState(pAd, Elem));
				break;
#endif
#ifdef CONFIG_DOT11U_INTERWORKING

			case GAS_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.GASMachine, Elem,
										  GASPeerCurrentState(pAd, Elem));
				break;
#endif

#ifdef DOT11K_RRM_SUPPORT
			case BCN_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.BCNMachine, Elem,
									BCNPeerCurrentState(pAd, Elem));
				break;
			case NEIGHBOR_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.NRMachine, Elem,
									NRPeerCurrentState(pAd, Elem));
				break;
#endif

#ifdef CONFIG_DOT11V_WNM

			case BTM_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.BTMMachine, Elem,
										  BTMPeerCurrentState(pAd, Elem));
				break;
#ifdef CONFIG_HOTSPOT_R2

			case WNM_NOTIFY_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WNMNotifyMachine, Elem,
										  WNMNotifyPeerCurrentState(pAd, Elem));
				break;
#endif
#endif
#ifdef BACKGROUND_SCAN_SUPPORT

			case BGND_SCAN_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, Elem,
										  pAd->BgndScanCtrl.BgndScanStatMachine.CurrState);
				break;
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef MT_DFS_SUPPORT

			case DFS_STATE_MACHINE: /* Jelly20150402 */
				StateMachinePerformAction(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, Elem,
										  pAd->CommonCfg.DfsParameter.DfsStatMachine.CurrState);
				break;
#endif /* MT_DFS_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			case AUTO_CH_SEL_STATE_MACHINE: {
				UCHAR BandIdx = HcGetBandByWdev(wdev);
				AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
				StateMachinePerformAction(pAd, &pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine, Elem,
										  pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState);
				break;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WDS_SUPPORT
			case WDS_STATE_MACHINE:
				StateMachinePerformAction(pAd, &pAd->Mlme.WdsMachine,
										  Elem, pAd->Mlme.WdsMachine.CurrState);
				break;
#endif
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
			case CH_SWITCH_MONITOR_STATE_MACHINE: {
				UCHAR band_idx = HcGetBandByWdev(wdev);
				struct ch_switch_cfg *ch_sw_cfg = &pAd->ch_sw_cfg[band_idx];
				StateMachinePerformAction(pAd, &ch_sw_cfg->ch_switch_sm, Elem,
								ch_sw_cfg->ch_switch_sm.CurrState);
				break;
			}
#endif
#ifdef WIFI_DIAG
			case WIFI_DAIG_STATE_MACHINE:
				diag_log_file_write(pAd);
				diag_conn_error_write(pAd);
				break;
#endif

			default:
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Illegal SM %ld\n",
						 __func__, Elem->Machine));
				break;
			} /* end of switch*/

			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): MlmeQ empty\n", __func__));
	}

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}

/*
========================================================================
Routine Description:
    MLME kernel thread.

Arguments:
	Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
static INT MlmeThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		goto LabelExit;

	RtmpOSTaskCustomize(pTask);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		/* lock the device pointers , need to check if required*/
		/*down(&(pAd->usbdev_semaphore)); */

		if (!pAd->PM_FlgSuspend && !pAd->Mlme.suspend)
			MlmeHandler(pAd);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
LabelExit:
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---%s\n", __func__));
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}



#ifdef CONFIG_AP_SUPPORT
static VOID ApMlmeInit(RTMP_ADAPTER *pAd)
{
#ifdef WDS_SUPPORT
	WdsStateMachineInit(pAd, &pAd->Mlme.WdsMachine, pAd->Mlme.WdsFunc);
#endif
	/* for Dot11H */
}
#endif /* CONFIG_AP_SUPPORT */

static INT mlme_for_wsys_notify_handle(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct wsys_notify_info *info = data;
	struct _MLME_STRUCT *mlme = ne->priv;
	struct wifi_dev *wdev = info->wdev;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): event_id: %d, wdev=%d\n", __func__, event_id, info->wdev->wdev_idx));

	switch (event_id) {
	case WSYS_NOTIFY_CLOSE:
		mlme_bss_clear(mlme, wdev);
		break;
	case WSYS_NOTIFY_LINKUP: {
#ifdef DOT11_HE_AX
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
			MAC_TABLE_ENTRY *pEntry = NULL;

			if (pAd) {
				pEntry = GetAssociatedAPByWdev(pAd, wdev);
				if (pEntry) {
					set_bss_color_info(wdev,
						pEntry->cap.bss_color_info.bss_color_dis,
						pEntry->cap.bss_color_info.bss_color);
				}
			}
		}
		bss_color_timer_init(wdev);
#endif
		break;
	}
	case WSYS_NOTIFY_LINKDOWN:
#ifdef DOT11_HE_AX
		bss_color_timer_release(wdev);
#endif
		break;
	case WSYS_NOTIFY_OPEN:
	case WSYS_NOTIFY_CONNT_ACT:
	case WSYS_NOTIFY_DISCONNT_ACT:
	case WSYS_NOTIFY_STA_UPDATE:
	default:
		break;
	}
	return ret;
}

static INT mlme_notify_register(struct _RTMP_ADAPTER *ad, struct _MLME_STRUCT *mlme)
{
	INT ret;
	struct notify_entry *ne = &mlme->wsys_ne;

	/*fill notify entry for wifi system chain*/
	ne->notify_call = mlme_for_wsys_notify_handle;
	ne->priority = WSYS_NOTIFY_PRIORITY_MLME;
	ne->priv = mlme;
	/*register wifi system notify chain*/
	ret = register_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}

static INT mlme_notify_unregister(struct _RTMP_ADAPTER *ad, struct _MLME_STRUCT *mlme)
{
	INT ret;
	struct notify_entry *ne = &mlme->wsys_ne;

	/*register wifi system notify chain*/
	ret = unregister_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}

/*
	==========================================================================
	Description:
		initialize the MLME task and its data structure (queue, spinlock,
		timer, state machines).

	IRQL = PASSIVE_LEVEL

	Return:
		always return NDIS_STATUS_SUCCESS

	==========================================================================
*/
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> MLME Initialize\n"));

	do {
		Status = MlmeQueueInit(pAd);

		if (Status != NDIS_STATUS_SUCCESS)
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(pAd, &pAd->Mlme.TaskLock);
		{
			UCHAR BandIdx = 0;

			for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
				sync_fsm_init(pAd, BandIdx, &pAd->ScanCtrl[BandIdx].SyncFsm,
							  pAd->ScanCtrl[BandIdx].SyncFsmFun);
		}
#ifdef CONFIG_STA_SUPPORT
		if (IF_COMBO_HAVE_STA(pAd)) {
			int i;
			UCHAR BandIdx = 0;

			for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
				BssTableInit(&pAd->ScanCtrl[BandIdx].ScanTab);

			/* init STA state machines*/

			for (i = 0; i < MAX_MULTI_STA; i++) {
				struct wifi_dev *wdev = &pAd->StaCfg[i].wdev;

				/* skip non-init wdev  */
				if (GetStaCfgByWdev(pAd, wdev) == NULL)
					continue;

				wdev->cntl_machine.CurrState = CNTL_IDLE;
				wdev->auth_machine.CurrState = AUTH_FSM_IDLE;
				wdev->assoc_machine.CurrState = ASSOC_IDLE;
				sta_auth_init(wdev);
				sta_assoc_init(wdev);
			}
		}

		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {

#ifdef DOT11Z_TDLS_SUPPORT
			TDLS_StateMachineInit(pAd, &pAd->Mlme.TdlsMachine, pAd->Mlme.TdlsFunc);
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef WSC_STA_SUPPORT
#ifdef IWSC_SUPPORT
			IWSC_StateMachineInit(pAd, &pAd->Mlme.IWscMachine, pAd->Mlme.IWscFunc);
#endif /* IWSC_SUPPORT */
#endif /* WSC_STA_SUPPORT */
			RTMPInitTimer(pAd, &pAd->StaCfg[0].StaQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(StaQuickResponeForRateUpExec), pAd,
						  FALSE);
			pAd->StaCfg[0].StaQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			ApMlmeInit(pAd);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
		/* Init Wsc state machine */
		ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
		WscStateMachineInit(pAd, &pAd->Mlme.WscMachine, pAd->Mlme.WscFunc);
#endif /* WSC_INCLUDED */
		WpaStateMachineInit(pAd, &pAd->Mlme.WpaMachine, pAd->Mlme.WpaFunc);
#ifdef CONFIG_HOTSPOT
		HSCtrlStateMachineInit(pAd, &pAd->Mlme.HSCtrlMachine, pAd->Mlme.HSCtrlFunc);
#endif /*CONFIG_HOTSPOT*/
#ifdef CONFIG_DOT11U_INTERWORKING
		GASStateMachineInit(pAd, &pAd->Mlme.GASMachine, pAd->Mlme.GASFunc);
#endif /*CONFIG_DOT11U_INTERWORKING*/

#ifdef DOT11K_RRM_SUPPORT
		RRMBcnReqStateMachineInit(pAd, &pAd->Mlme.BCNMachine, pAd->Mlme.BCNFunc);
		NRStateMachineInit(pAd, &pAd->Mlme.NRMachine, pAd->Mlme.NRFunc);
#endif
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
		ch_switch_monitor_state_machine_init(pAd);
#endif

#ifdef CONFIG_DOT11V_WNM
		WNMCtrlInit(pAd);
		BTMStateMachineInit(pAd, &pAd->Mlme.BTMMachine, pAd->Mlme.BTMFunc);
#ifdef CONFIG_HOTSPOT_R2
		WNMNotifyStateMachineInit(pAd, &pAd->Mlme.WNMNotifyMachine, pAd->Mlme.WNMNotifyFunc);
#endif /*CONFIG_HOTSPOT_R2*/
#endif
		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);
		/* Init mlme periodic timer*/
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExecTimer), pAd, TRUE);
		/* Set mlme periodic timer*/
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
		/* software-based RX Antenna diversity*/
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* Init APSD periodic timer*/
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);
			/* Init APQuickResponseForRateUp timer.*/
			RTMPInitTimer(pAd, &pAd->ApCfg.ApQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(APQuickResponeForRateUpExec), pAd,
						  FALSE);
			pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT
		/* P2P Ctrl State Machine */
		ASSERT(P2P_CTRL_FUNC_SIZE == P2P_CTRL_MAX_EVENTS * P2P_CTRL_MAX_STATES);
		P2PCtrlStateMachineInit(pAd, &pAd->P2pCfg.P2PCtrlMachine, pAd->P2pCfg.P2PCtrlFunc);
		/* P2P Discovery State Machine */
		ASSERT(P2P_DISC_FUNC_SIZE == P2P_DISC_MAX_EVENTS * P2P_DISC_MAX_STATES);
		P2PDiscoveryStateMachineInit(pAd, &pAd->P2pCfg.P2PDiscMachine, pAd->P2pCfg.P2PDiscFunc);
		/* P2P Group Formation State Machine */
		ASSERT(P2P_GO_FORM_FUNC_SIZE == P2P_GO_NEGO_MAX_EVENTS * P2P_GO_FORM_MAX_STATES);
		P2PGoFormationStateMachineInit(pAd, &pAd->P2pCfg.P2PGoFormMachine, pAd->P2pCfg.P2PGoFormFunc);
		/* P2P Action Frame State Machine */
		ASSERT(P2P_ACTION_FUNC_SIZE == MAX_P2P_MSG * MAX_P2P_STATE);
		P2PStateMachineInit(pAd, &pAd->P2pCfg.P2PActionMachine, pAd->P2pCfg.P2PActionFunc);
		/* P2P CTWindows timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pCTWindowTimer, GET_TIMER_FUNCTION(P2PCTWindowTimer), pAd, FALSE);
		/* P2P SwNOA timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pSwNoATimer, GET_TIMER_FUNCTION(P2pSwNoATimeOut), pAd, FALSE);
		/* P2P Presence Absent timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pPreAbsenTimer, GET_TIMER_FUNCTION(P2pPreAbsenTimeOut), pAd, FALSE);
		/* P2P WSC Timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pWscTimer, GET_TIMER_FUNCTION(P2pWscTimeOut), pAd, FALSE);
		/* P2P Re-Transmit Action Frame Timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pReSendTimer, GET_TIMER_FUNCTION(P2pReSendTimeOut), pAd, FALSE);
		/* P2P CLIENT Re-Connect Timer */
		RTMPInitTimer(pAd, &pAd->P2pCfg.P2pCliReConnectTimer, GET_TIMER_FUNCTION(P2pCliReConnectTimeOut), pAd, FALSE);
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			ApMlmeInit(pAd);
#ifdef APCLI_SUPPORT
			ApCliMlmeInit(pAd);
#endif /* APCLI_SUPPORT */
			/* Init APSD periodic timer */
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);
		}
#endif /* CONFIG_STA_SUPPORT */
#endif /* P2P_SUPPORT */
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
		/*CFG_TODO*/
		ApMlmeInit(pAd);
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		ApCliMlmeInit(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#endif /* RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */
	} while (FALSE);

	{
		RTMP_OS_TASK *pTask;
		/* Creat MLME Thread */
		pTask = &pAd->mlmeTask;
		RTMP_OS_TASK_INIT(pTask, "RtmpMlmeTask", pAd);
		Status = RtmpOSTaskAttach(pTask, MlmeThread, (ULONG)pTask);

		if (Status == NDIS_STATUS_FAILURE)
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unable to start MlmeThread\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
	}
	/*mlme is ready, register notify handle for wifi system event*/
	mlme_notify_register(pAd, &pAd->Mlme);
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- MLME Initialize\n"));
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	return Status;
}


/*
	==========================================================================
	Description:
		Destructor of MLME (Destroy queue, state machine, spin lock and timer)
	Parameters:
		Adapter - NIC Adapter pointer
	Post:
		The MLME task will no longer work properly

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID MlmeHalt(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
	RTMP_OS_TASK *pTask;
#ifdef CONFIG_STA_SUPPORT
	UINT	i = 0;
	PSTA_ADMIN_CONFIG	pStaCfg = NULL;
	BOOLEAN	InWOW = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> MlmeHalt\n"));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	/* Terminate Mlme Thread */
	pTask = &pAd->mlmeTask;

	if (RtmpOSTaskKill(pTask) == NDIS_STATUS_FAILURE)
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("kill mlme task failed!\n"));

	/*unregister notify for wifi system*/
	mlme_notify_unregister(pAd, &pAd->Mlme);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if ((pAd->WOW_Cfg.bEnable) &&
				(pAd->WOW_Cfg.bWowIfDownSupport) &&
				INFRA_ON(pStaCfg)) {
				InWOW = TRUE;
				break;
			}
		}

#endif /* WOW */
	}

	if (!InWOW)
#endif /* CONFIG_STA_SUPPORT */
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			/* disable BEACON generation and other BEACON related hardware timers*/
			AsicDisableSync(pAd, HW_BSSID_0);
		}

	RTMPReleaseTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);
#ifdef CONFIG_STA_SUPPORT
	if (IF_COMBO_HAVE_STA(pAd)) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];
			/* Cancel pending timers*/
			RTMPReleaseTimer(&pStaCfg->MlmeAux.AssocTimer, &Cancelled);
			RTMPReleaseTimer(&pStaCfg->MlmeAux.ReassocTimer, &Cancelled);
			RTMPReleaseTimer(&pStaCfg->MlmeAux.DisassocTimer, &Cancelled);
			RTMPReleaseTimer(&pStaCfg->MlmeAux.AuthTimer, &Cancelled);
			RTMPReleaseTimer(&pStaCfg->MlmeAux.JoinTimer, &Cancelled);
#ifdef DOT11R_FT_SUPPORT
			RTMPReleaseTimer(&pStaCfg->MlmeAux.FtOtaAuthTimer, &Cancelled);
			RTMPReleaseTimer(&pStaCfg->MlmeAux.FtOtdActTimer, &Cancelled);
#endif /* DOT11R_FT_SUPPORT */
			RTMPReleaseTimer(&pStaCfg->LinkDownTimer, &Cancelled);
#ifdef WSC_STA_SUPPORT

			if (pStaCfg->wdev.WscControl.WscProfileRetryTimerRunning) {
				pStaCfg->wdev.WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPCancelTimer(&pStaCfg->wdev.WscControl.WscProfileRetryTimer, &Cancelled);
			}

#endif /* WSC_STA_SUPPORT */

			if (pStaCfg->StaQuickResponeForRateUpTimerRunning) {
				RTMPReleaseTimer(&pStaCfg->StaQuickResponeForRateUpTimer, &Cancelled);
				pStaCfg->StaQuickResponeForRateUpTimerRunning = FALSE;
			}

			RTMPReleaseTimer(&pStaCfg->MlmeAux.WpaDisassocAndBlockAssocTimer, &Cancelled);
#ifdef IWSC_SUPPORT
			RTMPReleaseTimer(&pStaCfg->IWscInfo.IWscT1Timer, &Cancelled);
			RTMPCancelTimer(&pStaCfg->IWscInfo.IWscT2Timer, &Cancelled);
#endif /* IWSC_SUPPORT */
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	RTMPReleaseTimer(&pAd->Mlme.RxAntEvalTimer, &Cancelled);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR idx;

		idx = 0;
		RTMPReleaseTimer(&pAd->Mlme.APSDPeriodicTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
#ifdef APCLI_SUPPORT

		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
#if defined(APCLI_CONNECTION_TRIAL) || defined(WSC_AP_SUPPORT)
			PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[idx];
#endif
#ifdef APCLI_CONNECTION_TRIAL
			RTMPReleaseTimer(&pApCliEntry->TrialConnectTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectPhase2Timer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectRetryTimer, &Cancelled);
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef WSC_AP_SUPPORT

			if (pApCliEntry->wdev.WscControl.WscProfileRetryTimerRunning) {
				pApCliEntry->wdev.WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPReleaseTimer(&pApCliEntry->wdev.WscControl.WscProfileRetryTimer, &Cancelled);
			}

#endif /* WSC_AP_SUPPORT */
		}

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_HOTSPOT
	HSCtrlHalt(pAd);
	HSCtrlExit(pAd);
#endif
#ifdef CONFIG_DOT11U_INTERWORKING
	GASCtrlExit(pAd);
#endif
#ifdef CONFIG_DOT11V_WNM
	WNMCtrlExit(pAd);
#endif
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	ch_switch_monitor_exit(pAd);
#endif
#ifdef P2P_SUPPORT
	/* P2P CTWindows timer */
	RTMPReleaseTimer(&pAd->P2pCfg.P2pCTWindowTimer, &Cancelled);
	/* P2P SwNOA timer */
	RTMPReleaseTimer(&pAd->P2pCfg.P2pSwNoATimer, &Cancelled);
	/* P2P Presence Absent timer */
	RTMPReleaseTimer(&pAd->P2pCfg.P2pPreAbsenTimer, &Cancelled);

	if (pAd->P2pCfg.bP2pCliReConnectTimerRunning) {
		pAd->P2pCfg.bP2pCliReConnectTimerRunning = FALSE;
		RTMPReleaseTimer(&pAd->P2pCfg.P2pCliReConnectTimer, &Cancelled);
	}

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		RTMPReleaseTimer(&pAd->Mlme.APSDPeriodicTimer,	&Cancelled);
#ifdef WSC_AP_SUPPORT

		if (pAd->StaCfg[BSS0].WscControl.WscProfileRetryTimerRunning) {
			pAd->StaCfg[BSS0].WscControl.WscProfileRetryTimerRunning = FALSE;
			RTMPReleaseTimer(&pAd->StaCfg[BSS0].WscControl.WscProfileRetryTimer, &Cancelled);
		}

#endif /* WSC_AP_SUPPORT */
		{
			UCHAR BandIdx = 0;

			for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
				RTMPReleaseTimer(&pAd->ScanCtrl[BandIdx].ScanTimer, &Cancelled);
				RTMPReleaseTimer(&pAd->ScanCtrl[BandIdx].PartialScan.PartialScanTimer, &Cancelled);
		}

	}
#endif /* P2P_SUPPORT */

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef LED_CONTROL_SUPPORT
		UCHAR BandIdx = 0;

		/* Set LED*/
		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
			RTMPSetLED(pAd, LED_HALT, BandIdx);
		RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it.*/
#endif /* LED_CONTROL_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

			for (i = 0; i < MAX_MULTI_STA; i++) {
				pStaCfg = &pAd->StaCfg[i];

				if ((pAd->WOW_Cfg.bEnable == TRUE) &&
					(pAd->WOW_Cfg.bWowIfDownSupport) &&
					INFRA_ON(pStaCfg)) {
					InWOW = TRUE;
					break;
				}
			}

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */
		}

		if (!InWOW)
#endif /* CONFIG_STA_SUPPORT */
			if ((pChipOps->AsicHaltAction))
				pChipOps->AsicHaltAction(pAd);
	}

	RtmpusecDelay(5000);    /*  5 msec to gurantee Ant Diversity timer canceled*/
	MlmeQueueDestroy(&pAd->Mlme);
	NdisFreeSpinLock(&pAd->Mlme.TaskLock);
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== MlmeHalt\n"));
}


VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
		/* for performace enchanement */
		NdisZeroMemory(&pAd->RalinkCounters,
					   (LONG)&pAd->RalinkCounters.OneSecEnd -
					   (LONG)&pAd->RalinkCounters.OneSecStart);

	return;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically to -
		1. Decide if it's a right time to turn on PwrMgmt bit of all
		   outgoiing frames
		2. Calculate ChannelQuality based on statistics of the last
		   period, so that TX rate won't toggling very frequently between a
		   successful TX and a failed TX.
		3. If the calculated ChannelQuality indicated current connection not
		   healthy, then a ROAMing attempt is tried here.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  /* 8 sec*/
VOID MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MlmePeriodicExec(SystemSpecific1, FunctionContext, SystemSpecific2, SystemSpecific3);
}

#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
extern int rx_detect_flag;
#endif
#endif

VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ULONG TxTotalCnt;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
#ifdef RATE_PRIOR_SUPPORT
	INT idx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PBLACK_STA pBlackSta = NULL, tmp;
	UCHAR isexist = 0;
#endif /*RATE_PRIOR_SUPPORT*/

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#if defined(AXE_ASIC_WORKAROUND)
	if (IS_AXE(pAd)) {
		UINT32 value = 0;
		/* Toggle ARB SCR b'8 */
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x820f3080, &value);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3080, value | BIT8);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820f3080, value & ~BIT8);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Work around applied\n"));
	}
#endif

#ifdef ERR_RECOVERY
#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
	chip_sw_int_polling(pAd);
#endif
#endif
#endif

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex <= MAX_APCLI_NUM)) {
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		/* 100msec Periodic RSSI Update from WTBL */
		if (wdev && IS_VENDOR10_RSSI_VALID(wdev)) {

			RTMP_SET_UPDATE_RSSI(pAd);
			pAd->V10UpdateRssi = 1;
		}
	}
#endif


#ifdef	ETSI_RX_BLOCKER_SUPPORT
	if (pAd->fgAdaptRxBlock) {
		/* check RSSI each 100 ms  */
		if (pAd->u1TimeCnt >= pAd->u1CheckTime) {
			RTMP_CHECK_RSSI(pAd);
			pAd->u1TimeCnt = 0;
		} else
			pAd->u1TimeCnt++;
	}
#ifdef OFFCHANNEL_SCAN_FEATURE
	if (pAd->ScanCtrl[DBDC_BAND0].OffChScan != TRUE) {
		asic_calculate_nf(pAd, DBDC_BAND0);
	} else {
		if (pAd->ScanCtrl[DBDC_BAND0].OffChScan_Ongoing != TRUE) {
			pAd->ScanCtrl[DBDC_BAND0].OffChScan = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s: Scan state set to IDLE and Register reset for  Band 0  \n", __func__));
			asic_reset_enable_nf_registers(pAd, DBDC_BAND0);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s: Register reset for  Band 0  \n", __func__));
		}
	}
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (pAd->ScanCtrl[DBDC_BAND1].OffChScan != TRUE) {
			asic_calculate_nf(pAd, DBDC_BAND1);
		} else {
			if (pAd->ScanCtrl[DBDC_BAND1].OffChScan_Ongoing != TRUE) {
				pAd->ScanCtrl[DBDC_BAND1].OffChScan = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s: Scan state set to IDLE and Register reset for  Band 1  \n", __func__));
				asic_reset_enable_nf_registers(pAd, DBDC_BAND1);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s: Register reset for  Band 1  \n", __func__));
			}
		}
	}
#endif
#elif defined(NF_SUPPORT)
	if (!scan_in_run_state(pAd, NULL)) {
		asic_calculate_nf(pAd, DBDC_BAND0);
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			asic_calculate_nf(pAd, DBDC_BAND1);
#endif
	}
#endif	/*OFFCHANNEL_SCAN_FEATURE , NF_SUPPORT*/

#if defined(TXRX_STAT_SUPPORT)
	if ((!scan_in_run_state(pAd, NULL))
#ifdef OFFCHANNEL_SCAN_FEATURE
			&& (pAd->ScanCtrl[DBDC_BAND0].state == OFFCHANNEL_SCAN_INVALID)
#ifdef DBDC_MODE
			&& (pAd->CommonCfg.dbdc_mode) && (pAd->ScanCtrl[DBDC_BAND1].state == OFFCHANNEL_SCAN_INVALID)
#endif
#endif
	   )
		ReadChannelStats(pAd);
#endif
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

	/* CFG MCC */


#ifdef MICROWAVE_OVEN_SUPPORT
	/* update False CCA count to an array */
	NICUpdateRxStatusCnt1(pAd, pAd->Mlme.PeriodicRound % 10);

	if (pAd->CommonCfg.MO_Cfg.bEnable) {
		UINT8 stage = pAd->Mlme.PeriodicRound % 10;

		if (stage == MO_MEAS_PERIOD) {
			ASIC_MEASURE_FALSE_CCA(pAd);
			pAd->CommonCfg.MO_Cfg.nPeriod_Cnt = 0;
		} else if (stage == MO_IDLE_PERIOD) {
			UINT16 Idx;

			for (Idx = MO_MEAS_PERIOD + 1; Idx < MO_IDLE_PERIOD + 1; Idx++)
				pAd->CommonCfg.MO_Cfg.nFalseCCACnt += pAd->RalinkCounters.FalseCCACnt_100MS[Idx];

			/* printk("%s: fales cca1 %d\n", __func__, pAd->CommonCfg.MO_Cfg.nFalseCCACnt); */
			if (pAd->CommonCfg.MO_Cfg.nFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				ASIC_MITIGATE_MICROWAVE(pAd);
		}
	}

#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
							  fRTMP_ADAPTER_RADIO_MEASUREMENT |
							  fRTMP_ADAPTER_NIC_NOT_EXIST)))
		|| IsHcAllSupportedBandsRadioOff(pAd)) {
		return;
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Do nothing if monitor mode is on*/
		if (MONITOR_ON(pAd))
			return;
	}
#endif /* CONFIG_STA_SUPPORT */
	pAd->bUpdateBcnCntDone = FALSE;
	/*	RECBATimerTimeout(SystemSpecific1,FunctionContext,SystemSpecific2,SystemSpecific3);*/
	pAd->Mlme.PeriodicRound++;
	pAd->Mlme.GPIORound++;
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626) || defined(MT7915)

	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd)) {
#ifdef CONFIG_AP_SUPPORT
		BcnCheck(pAd);
#endif /* CONFIG_AP_SUPPORT */

		if ((pAd->Mlme.PeriodicRound % 5) == 0) { /* 500ms update */
			RTMP_UPDATE_MIB_COUNTER(pAd);
#ifdef SMART_CARRIER_SENSE_SUPPORT
			if (IS_MT7915(pAd)) {
				if ((pAd->Mlme.PeriodicRound % 10) == 0) /* one second update */
					Smart_Carrier_Sense(pAd);
			} else  /* 500ms update */
				Smart_Carrier_Sense(pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		}
	}

#endif /* MT7615 || MT7622 */
#ifdef RANDOM_PKT_GEN
	regular_pause_umac(pAd);
#endif

#ifdef BB_SOC
#ifdef TCSUPPORT_WLAN_SW_RPS
				if ((pAd->Mlme.PeriodicRound % 10) == 0) {
					if (rx_detect_flag)
						ecnt_rx_detection(pAd);
				}
#endif
#endif
	if (ops->heart_beat_check)
		ops->heart_beat_check(pAd);

	ba_timeout_monitor(pAd);
	
#ifdef WIFI_MD_COEX_SUPPORT
	if ((pAd->LteSafeChCtrl.SafeChnProcIntvl) &&
		((pAd->Mlme.PeriodicRound % (pAd->LteSafeChCtrl.SafeChnProcIntvl / 100)) == 0)) {
		ProcessSafeChannelChange(pAd);
	}
#endif

	/* by default, execute every 500ms */
	if ((pAd->ra_interval) &&
		((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0)) {
		if (RTMPAutoRateSwitchCheck(pAd) == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
			if (cap->fgRateAdaptFWOffload == TRUE) {
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					APMlmeDynamicTxRateSwitching(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				/* perform dynamic tx rate switching based on past TX history*/
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					if ((STA_STATUS_TEST_FLAG(&pAd->StaCfg[0], fSTA_STATUS_MEDIA_STATE_CONNECTED)
#ifdef P2P_SUPPORT
						|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
						|| (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
						|| RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_SUPPORT */
						)
						&& (!pAd->StaCfg[0].PwrMgmt.bDoze))
						MlmeDynamicTxRateSwitchingNew(pAd);
				}
#endif /* CONFIG_STA_SUPPORT */
			}
		} else {
#ifdef MT_MAC
			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				MAC_TABLE_ENTRY *pEntry;
				MT_TX_COUNTER TxInfo;
				UINT16 i;

				for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					if (!IS_WCID_VALID(pAd, i))
						break;

					/* point to information of the individual station */
					pEntry = &pAd->MacTab.Content[i];

					if (IS_ENTRY_NONE(pEntry))
						continue;

					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#ifdef APCLI_SUPPORT

					if (IS_ENTRY_PEER_AP(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#ifdef MAC_REPEATER_SUPPORT

					if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#endif
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

					/* if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)) */
					if (IS_ENTRY_WDS(pEntry))
						continue;

#endif /* WDS_SUPPORT */

					if (IS_VALID_ENTRY(pEntry))
						AsicTxCntUpdate(pAd, pEntry->wcid, &TxInfo);
				}
			}

#endif /* MT_MAC */
		}
	}

#ifdef CONFIG_MAP_SUPPORT
	map_rssi_status_check(pAd);
#endif

#ifdef SMART_ANTENNA
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if ((pAd->ra_interval) &&
			((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0) &&
			RTMP_SA_WORK_ON(pAd)) {
			if (RTMPAutoRateSwitchCheck(pAd) == FALSE) {
				TX_STA_CNT1_STRUC StaTx1;
				TX_STA_CNT0_STRUC TxStaCnt0;
				RTMP_SA_TRAINING_PARAM *pTrainEntry = &pAd->pSAParam->trainEntry[0];
				if (pTrainEntry->pMacEntry && (pTrainEntry->trainStage != SA_INVALID_STAGE))
					pTrainEntry->mcsStableCnt++;
			}

			/* Check if need to run antenna adaptation */
			RtmpSAChkAndGo(pAd);
		}
	}
#endif /* SMART_ANTENNA */
#ifdef ANTENNA_DIVERSITY_SUPPORT
	if ((pAd->Mlme.PeriodicRound % ANT_DIVERSITY_OPERATION_CYCLE) == 0)
		ant_diversity_periodic_exec(pAd);
#endif

	/* Normal 1 second Mlme PeriodicExec.*/
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) {
		pAd->Mlme.OneSecPeriodicRound++;

		/* add to handle wifi_sys operation race condition */
		if (pAd->MonitorSemaphore) {
			ULONG timestamp = 0;
			/* wifi sys link lock monitor */
			if ((pAd->wf_link_lock_flag == TRUE) && (pAd->wf_link_timestamp > 0)) {
				NdisGetSystemUpTime(&timestamp);
				if ( RTMP_TIME_AFTER((unsigned long)timestamp, (unsigned long)(pAd->wf_link_timestamp + WIFI_LINK_AGEOUT_TIME)) ) {
					RTMP_SEM_EVENT_UP(&pAd->wf_link_lock);
					pAd->wf_link_lock_flag = FALSE;
					pAd->wf_link_timestamp = 0;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wifi sys link action abnormal exit!!\n", __func__));
				}
			}
		}
			
		if (IS_ASIC_CAP(pAd, fASIC_CAP_WMM_PKTDETECT_OFFLOAD)) {
			MtCmdCr4QueryBssAcQPktNum(pAd, CR4_GET_BSS_ACQ_PKT_NUM_CMD_DEFAULT);
		} else {
			mt_dynamic_wmm_be_tx_op(pAd, ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
		}

		NdisGetSystemUpTime(&pAd->Mlme.Now32);
		/* add the most up-to-date h/w raw counters into software variable, so that*/
		/* the dynamic tuning mechanism below are based on most up-to-date information*/
		/* Hint: throughput impact is very serious in the function */
#ifdef TXRX_STAT_SUPPORT
		if (pAd->ApCfg.EntryClientCount && pAd->EnableTxRxStats) {
			MtCmdGetPerStaTxStat(pAd, NULL, 0); 	/*bitmap and entryCount to be used in future*/
			Update_LastSec_TXRX_Stats(pAd);
		}
#endif
#ifdef EAP_STATS_SUPPORT
		if (pAd->ApCfg.EntryClientCount) {
			MtCmdGetAllStaStats(pAd, EVENT_PHY_TX_STAT_PER_WCID);
		}
#endif
#ifdef CONFIG_MAP_SUPPORT
		if ((IS_MAP_TURNKEY_ENABLE(pAd) || IS_MAP_BS_ENABLE(pAd))
					&& (IS_MT7915(pAd))) {
			if (pAd->ApCfg.EntryClientCount) {
				MtCmdGetAllStaStats(pAd, EVENT_PHY_ALL_TX_RX_RATE);
			}
		}
#endif

		if (IS_MT7915(pAd)) {
			if (pAd->ApCfg.EntryClientCount)
				MtCmdGetAllStaStats(pAd, EVENT_PHY_TXRX_AIR_TIME);
		}
		/* NICUpdateRawCountersNew(pAd); */
		RTMP_UPDATE_RAW_COUNTER(pAd);
		RTMP_SECOND_CCA_DETECTION(pAd);
#ifdef DYNAMIC_VGA_SUPPORT
		dynamic_vga_adjust(pAd);
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef DOT11_N_SUPPORT
		/* Need statistics after read counter. So put after NICUpdateRawCountersNew*/
		ORIBATimerTimeout(pAd);
#endif /* DOT11_N_SUPPORT */
		/*
			if (pAd->RalinkCounters.MgmtRingFullCount >= 2)
				RTMP_SET_FLAG(pAd, fRTMP_HW_ERR);
			else
				pAd->RalinkCounters.MgmtRingFullCount = 0;
		*/
		/* The time period for checking antenna is according to traffic*/
		{
			if (pAd->Mlme.bEnableAutoAntennaCheck) {
				TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
							 pAd->RalinkCounters.OneSecTxRetryOkCount +
							 pAd->RalinkCounters.OneSecTxFailCount;

				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50) {
					if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
						AsicEvaluateRxAnt(pAd);
				} else {
					if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
						AsicEvaluateRxAnt(pAd);
				}
			}
		}

#ifdef DOT11_N_SUPPORT
#ifdef MT_MAC

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				/* Not RDG, update the TxOP else keep the default RDG's TxOP */
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == FALSE) {
				}
			}

#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */
		/* update RSSI each 1 second*/
#ifdef KERNEL_RPS_ADJUST
		if (!pAd->ixia_mode_ctl.mode_entered)
#endif
#ifdef ERR_RECOVERY
	if (!IsStopingPdma(&pAd->ErrRecoveryCtl))
#endif
		RTMP_SET_UPDATE_RSSI(pAd);

#ifdef LINK_TEST_SUPPORT
		LinkTestPeriodHandler(pAd);
#endif /* LINK_TEST_SUPPORT */

		/*Update some MIB counter per second */
		Update_Mib_Bucket_One_Sec(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef AP_QLOAD_SUPPORT
			/* QBSS_LoadUpdate from Mib_Bucket */
			update_ap_qload_to_bcn(pAd);
#endif /* AP_QLOAD_SUPPORT */
#ifdef WAPP_SUPPORT
#ifdef OCE_SUPPORT
			wapp_bss_load_check(pAd);
#endif /* OCE_SUPPORT */
#endif /* WAPP_SUPPORT */
			APMlmePeriodicExec(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT

			if (pAd->BgndScanCtrl.BgndScanSupport) {
				ChannelQualityDetection(pAd);
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				CCI_ACI_scenario_maintain(pAd);
#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
			Mumimo_scenario_maintain(pAd);
#endif /* MT_MAC && VHT_TXBF_SUPPORT */
			}


			if ((pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H[0].RDMode != RD_SILENCE_MODE))
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode[DBDC_BAND0] != WDS_BRIDGE_MODE)
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
			   )
				pAd->macwd++;
			else
				pAd->macwd = 0;
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode && (pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H[0].RDMode != RD_SILENCE_MODE))
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode[DBDC_BAND1] != WDS_BRIDGE_MODE)
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
			   )
				pAd->macwd++;
			else
				pAd->macwd = 0;
#endif	/* DBDC_MODE */



			if (pAd->macwd > 1) {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MAC specific condition\n"));
				AsicSetMacWD(pAd);
#ifdef AP_QLOAD_SUPPORT
				Show_QoSLoad_Proc(pAd, NULL);
#endif /* AP_QLOAD_SUPPORT */
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			int i;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			RtmpPsActiveExtendCheck(pAd);
#ifdef TDLS_AUTOLINK_SUPPORT

			/* TDLS discovery link maintenance */
			if (IS_TDLS_SUPPORT(pAd) && (pAd->StaCfg[0].TdlsInfo.TdlsAutoLink))
				TDLS_MaintainDiscoveryEntryList(pAd);

#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

			for (i = 0; i < pAd->MSTANum; i++)
				if (pAd->StaCfg[i].wdev.DevInfo.WdevActive)
					STAMlmePeriodicExec(pAd, &pAd->StaCfg[i].wdev);
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			UINT8 MaxNumApcli = min(pAd->ApCfg.ApCliNum, (UINT8)MAX_APCLI_NUM);
			UINT8 i;

			for (i = 0; i < MaxNumApcli; i++)
				if (pAd->StaCfg[i].ApcliInfStat.Valid) {
					if (pAd->CommonCfg.wifi_cert)
						sta_2040_coex_scan_check(pAd, &pAd->StaCfg[i].wdev);
					MlmeCheckPsmChange(pAd, &pAd->StaCfg[i].wdev);
			}
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		RTMP_SECOND_CCA_DETECTION(pAd);
		MlmeResetRalinkCounters(pAd);
		RTMP_MLME_HANDLER(pAd);
	}
	scan_partial_trigger_checker(pAd); //reduce parscan time
#ifdef WSC_INCLUDED
	WSC_HDR_BTN_MR_HANDLE(pAd);
#endif /* WSC_INCLUDED */
#ifdef P2P_SUPPORT

	if (P2P_INF_ON(pAd))
		P2pPeriodicExec(SystemSpecific1, FunctionContext, SystemSpecific2, SystemSpecific3);

#endif /* P2P_SUPPORT */
	pAd->bUpdateBcnCntDone = FALSE;

#ifdef RATE_PRIOR_SUPPORT
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0 && pAd->LowRateCtrl.RatePrior == 1) {

		RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
		DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
			if (time_after(jiffies, pBlackSta->Jiff + (pAd->LowRateCtrl.BlackListTimeout)*HZ)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("Remove from blklist, %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBlackSta->Addr)));
				tmp = pBlackSta;
				pBlackSta = DlListEntry(pBlackSta->List.Prev, BLACK_STA, List);
				DlListDel(&(tmp->List));
				os_free_mem(tmp);
			}
		}
		RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
	}
	if (pAd->Mlme.PeriodicRound % (MLME_TASK_EXEC_MULTIPLE * pAd->LowRateCtrl.LowRateCountPeriod) == 0
		&& pAd->LowRateCtrl.RatePrior == 1) {
		UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
		for (idx = 1; idx < wtbl_max_num; idx++) {
			pEntry = &(pAd->MacTab.Content[idx]);
			if (IS_ENTRY_CLIENT(pEntry)) {
				if (pEntry->McsTotalRxCount > pAd->LowRateCtrl.TotalCntThreshold) {
					if (pAd->LowRateCtrl.LowRateRatioThreshold * pEntry->McsTotalRxCount <
						10 * pEntry->McsLowRateRxCount) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("LowRateRatio exceed Threshold!Send DEAUTH frame!!\n"));
						/*check if the sta has been blocked*/
						RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
						DlListForEach(pBlackSta, &pAd->LowRateCtrl.BlackList, BLACK_STA, List) {
							if (NdisCmpMemory(pBlackSta->Addr, pEntry->Addr, MAC_ADDR_LEN) == 0) {
								pBlackSta->Jiff = jiffies;
								isexist = 1;
								break;
							}
						}
						RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
						/*if the sta is not blocked, then add to list*/
						if (!isexist) {
							os_alloc_mem(NULL, (UCHAR **)&pBlackSta, sizeof(BLACK_STA));
							NdisZeroMemory(pBlackSta, sizeof(BLACK_STA));

							NdisCopyMemory(pBlackSta->Addr, pEntry->Addr, MAC_ADDR_LEN);
							pBlackSta->Jiff = jiffies;
							RTMP_SEM_LOCK(&pAd->LowRateCtrl.BlackListLock);
							DlListAdd(&pAd->LowRateCtrl.BlackList, &(pBlackSta->List));
							RTMP_SEM_UNLOCK(&pAd->LowRateCtrl.BlackListLock);
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("Add to blklist, %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pBlackSta->Addr)));
							MlmeDeAuthAction(pAd, pEntry, REASON_UNSPECIFY, FALSE);
						}
					}
				}
				pEntry->McsTotalRxCount = 0;
				pEntry->McsLowRateRxCount = 0;
			} else {
				if (pEntry != NULL) {
					pEntry->McsTotalRxCount = 0;
					pEntry->McsLowRateRxCount = 0;
				}
			}
		}
	}
#endif /*RATE_PRIOR_SUPPORT*/


}


/*
	==========================================================================
	Validate SSID for connection try and rescan purpose
	Valid SSID will have visible chars only.
	The valid length is from 0 to 32.
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
BOOLEAN MlmeValidateSSID(UCHAR *pSsid, UCHAR SsidLen)
{
	int index;

	if (SsidLen > MAX_LEN_OF_SSID)
		return FALSE;

	/* Check each character value*/
	for (index = 0; index < SsidLen; index++) {
		if (pSsid[index] < 0x20)
			return FALSE;
	}

	/* All checked*/
	return TRUE;
}

#ifdef CONFIG_STA_SUPPORT
VOID STAMlmePeriodicExec(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	ULONG TxTotalCnt;
	int i;
	BOOLEAN bCheckBeaconLost = TRUE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef STA_LP_PHASE_2_SUPPORT
	/*
		We go to sleep mode only when count down to zero.
		Count down counter is set after link up. So within 10 seconds after link up, we never go to sleep.
		10 seconds period, we can get IP, finish 802.1x authenticaion. and some critical , timing protocol.
	*/
	if (pStaCfg->CountDowntoPsm > 0)
		pStaCfg->CountDowntoPsm--;
#endif /* STA_LP_PHASE_2_SUPPORT */

#ifdef P2P_SUPPORT
	WSC_CTRL *pWscControl;

	if (P2P_CLI_ON(pAd))
		pWscControl = &pAd->StaCfg[0].WscControl;
	else
		pWscControl = &pAd->ApCfg.MBSSID[0].wdev.WscControl;

#endif /* P2P_SUPPORT */
	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pStaCfg->RssiSample);
#ifdef WPA_SUPPLICANT_SUPPORT

	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
	{
		/* WPA MIC error should block association attempt for 60 seconds*/
		if (pStaCfg->bBlockAssoc &&
			RTMP_TIME_AFTER(pAd->Mlme.Now32, pStaCfg->LastMicErrorTime + (60 * OS_HZ)))
			pStaCfg->bBlockAssoc = FALSE;
	}

#ifdef ETH_CONVERT_SUPPORT

	if ((pAd->EthConvert.ECMode & ETH_CONVERT_MODE_CLONE)
		&& (pAd->EthConvert.CloneMacVaild == TRUE)
		&& (NdisEqualMemory(pAd->CurrentAddress, pAd->EthConvert.EthCloneMac, MAC_ADDR_LEN) == FALSE)) {
		/* Link down first	*/
		if (INFRA_ON(pStaCfg)) {
			if (pStaCfg->MlmeAux.SsidLen) {
				NdisZeroMemory(pAd->EthConvert.SSIDStr, MAX_LEN_OF_SSID);
				NdisMoveMemory(pAd->EthConvert.SSIDStr, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
				pAd->EthConvert.SSIDStrLen = pStaCfg->MlmeAux.SsidLen;
			}

			pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING);

		} else if (ADHOC_ON(pAd)) {
		/*should dispatch to adhoc state machine*/
		/*
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IF_STATE_CHG);
			AsicEnableIbssSync(
				pAd,
				pAd->CommonCfg.BeaconPeriod,
				HW_BSSID_0,
				OPMODE_ADHOC);
			LinkDown(pAd, FALSE, wdev);
		*/
		} else {
			/* Copy the new Mac address to ASIC and start to re-connect to AP.*/
			NdisMoveMemory(&pAd->CurrentAddress[0], &pAd->EthConvert.EthCloneMac[0], MAC_ADDR_LEN);
			NdisMoveMemory(wdev->if_addr, pAd->CurrentAddress, MAC_ADDR_LEN);
			wdev_attr_update(pAd, wdev);
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Write EthCloneMac to ASIC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
					 PRINT_MAC(pAd->CurrentAddress)));

			if (pAd->EthConvert.SSIDStrLen != 0) {
				/*MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("copy MlmeAux.Ssid to AutoReconnect!\n"));*/
				NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid,
							   pAd->EthConvert.SSIDStr,
							   pAd->EthConvert.SSIDStrLen);
				pStaCfg->MlmeAux.AutoReconnectSsidLen = pAd->EthConvert.SSIDStrLen;
				NdisMoveMemory(pStaCfg->MlmeAux.Ssid, pAd->EthConvert.SSIDStr, pAd->EthConvert.SSIDStrLen);
				pStaCfg->MlmeAux.SsidLen = pAd->EthConvert.SSIDStrLen;
			}
		}
	}

#endif /* ETH_CONVERT_SUPPORT */

	if (ADHOC_ON(pAd)) {
	} else
		AsicStaBbpTuning(pAd, pStaCfg);

	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
				 pAd->RalinkCounters.OneSecTxRetryOkCount +
				 pAd->RalinkCounters.OneSecTxFailCount;

	if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) &&
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))) {
		/* update channel quality for Roaming/Fast-Roaming and UI LinkQuality display*/
		/* bImprovedScan True means scan is not completed */
		if (ScanInfo->bImprovedScan)
			bCheckBeaconLost = FALSE;

#ifdef P2P_SUPPORT

		if (NdisEqualMemory(ZERO_MAC_ADDR, pAd->P2pCfg.ConnectingMAC, MAC_ADDR_LEN) == FALSE) {
			UCHAR p2pindex;

			p2pindex = P2pGroupTabSearch(pAd, pAd->P2pCfg.ConnectingMAC);

			if (p2pindex != P2P_NOT_FOUND) {
				if (pAd->P2pTable.Client[p2pindex].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
					bCheckBeaconLost = FALSE;
			}
		}

#endif /* P2P_SUPPORT */

		/* 7636 psm, Beacon lost will be handled by F/W */
		if (bCheckBeaconLost) {
			/* The NIC may lost beacons during scaning operation.*/
			MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, wdev);

			if (pEntry)
				MlmeCalculateChannelQuality(pAd, pEntry, pAd->Mlme.Now32);
		}
	}

	/* must be AFTER MlmeDynamicTxRateSwitching() because it needs to know if*/
	/* Radio is currently in noisy environment*/
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		RTMP_CHIP_ASIC_ADJUST_TX_POWER(pAd);
		RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(pAd);
	}

	/*
		Driver needs to up date value of LastOneSecTotalTxCount here;
		otherwise UI couldn't do scanning sometimes when STA doesn't connect to AP or peer Ad-Hoc.
	*/
	pAd->RalinkCounters.LastOneSecTotalTxCount = TxTotalCnt;
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)

	/* MAC table maintenance */
	if ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) &&
#ifdef RT_CFG80211_P2P_SUPPORT
		(pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
		P2P_GO_ON(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
	   ) {
		/* one second timer */
#ifdef RT_CFG80211_P2P_SUPPORT
		MacTableMaintenance(pAd);
#else
		P2PMacTableMaintenance(pAd);
#endif /* RT_CFG80211_P2P_SUPPORT */
	}

#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */

	if (INFRA_ON(pStaCfg)) {
	} else if (ADHOC_ON(pAd)) {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pEntry->LastBeaconRxTime + ADHOC_BEACON_LOST_TIME)
#ifdef IWSC_SUPPORT
				/*
					2011/09/05:
					Broadcom test bed doesn't broadcast beacon when Broadcom is Enrollee.
				*/
				&& (pStaCfg->WscControl.bWscTrigger == FALSE)
#endif /* IWSC_SUPPORT */
			   ) {
				MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
#ifdef RT_CFG80211_SUPPORT
				CFG80211OS_DelSta(pAd->net_dev, pStaCfg->Bssid);
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: del this ad-hoc %02x:%02x:%02x:%02x:%02x:%02x\n",
						 __func__, PRINT_MAC(pStaCfg->Bssid)));
#endif /* RT_CFG80211_SUPPORT */
			}
		}

		if (pAd->MacTab.Size == 0) {
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
		}
	} else { /* no INFRA nor ADHOC connection*/
#ifdef P2P_SUPPORT

		if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
			goto SKIP_AUTO_SCAN_CONN;

#endif /* P2P_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT

		if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
			goto SKIP_AUTO_SCAN_CONN;

#endif /* WPA_SUPPLICANT_SUPPORT */

		if (pStaCfg->bSkipAutoScanConn &&
			RTMP_TIME_BEFORE(pAd->Mlme.Now32, pStaCfg->LastScanTime + (30 * OS_HZ)))
			goto SKIP_AUTO_SCAN_CONN;
		else
			pStaCfg->bSkipAutoScanConn = FALSE;


		if ((pStaCfg->bAutoReconnect == TRUE)
			&& !IsHcRadioCurStatOffByWdev(&pStaCfg->wdev)
			&& RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
			&& (MlmeValidateSSID(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen) == TRUE) && (pStaCfg->wdev.channel != 0)) {
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier*/

			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE) {
				if ((pAd->Mlme.OneSecPeriodicRound % 5) == 0)
					MlmeAutoReconnectLastSSID(pAd, &pStaCfg->wdev);
			} else
#endif /* CARRIER_DETECTION_SUPPORT */
				if ((pAd->Mlme.OneSecPeriodicRound % 3) == 0)
					MlmeAutoReconnectLastSSID(pAd, &pStaCfg->wdev);
		}


	}

SKIP_AUTO_SCAN_CONN:
	/* YF_TODO */
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
#ifdef P2P_SUPPORT

	if (P2P_CLI_ON(pAd))
#else
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
#else
	if (RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev))
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* P2P_SUPPORT */
	{
		if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
			ApCliIfMonitor(pAd);

		if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
			ApCliIfUp(pAd);

#ifdef CONFIG_MULTI_CHANNEL

		if (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			ApCliSimulateRecvBeacon(pAd);
	}

#endif /* P2P_SUPPORT || RT_CFG80211_P2P_CONCURRENT_DEVICE */

	if (pAd->CommonCfg.wifi_cert)
		sta_2040_coex_scan_check(pAd, wdev);

	return;
}

VOID MlmeAutoScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	MLME_SCAN_REQ_STRUCT ScanReq; /* snowpin for cntl mgmt */

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	os_zero_mem(&ScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
	ScanParmFill(pAd,
				 &ScanReq,
				 (RTMP_STRING *) pStaCfg->MlmeAux.AutoReconnectSsid,
				 pStaCfg->MlmeAux.AutoReconnectSsidLen,
				 BSS_ANY, SCAN_ACTIVE);
	cntl_scan_request(wdev, &ScanReq);
}


VOID MlmeAutoReconnectLastSSID(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef WSC_STA_SUPPORT
	WSC_CTRL *pWscControl = NULL;
#endif
	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef WSC_STA_SUPPORT
	pWscControl = &wdev->WscControl;

	if ((pWscControl->WscConfMode != WSC_DISABLE) &&
		(pWscControl->bWscTrigger) &&
		(pWscControl->WscMode == WSC_PBC_MODE) &&
		(pWscControl->WscPBCBssCount != 1))
		return;

	/* PIN: bssid is set before pin wps, so no need to try iterate wps AP */
	if ((pWscControl->WscConfMode != WSC_DISABLE) &&
		(pWscControl->bWscTrigger) &&
		(pWscControl->WscMode == WSC_PIN_MODE) &&
		(pWscControl->WscPINBssCount != 1))
		return;

	if ((pWscControl->WscConfMode != WSC_DISABLE) &&
		(pWscControl->WscState >= WSC_STATE_START)) {
		ULONG ApIdx = 0;

		ApIdx = WscSearchWpsApBySSID(pAd,
									 pWscControl->WscSsid.Ssid,
									 pWscControl->WscSsid.SsidLength,
									 pWscControl->WscMode,
									 wdev);

		if ((ApIdx != BSS_NOT_FOUND) &&
			(pStaCfg->BssType == BSS_INFRA)) {
			BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
			NdisMoveMemory(pWscControl->WscBssid, ScanTab->BssEntry[ApIdx].Bssid, MAC_ADDR_LEN);
			pStaCfg->MlmeAux.Channel = ScanTab->BssEntry[ApIdx].Channel;
		}

		CntlWscIterate(pAd, pStaCfg);
	} else
#endif /* WSC_STA_SUPPORT */
		if (pStaCfg->bAutoConnectByBssid) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("Driver auto reconnect to last OID_802_11_BSSID setting - %02X:%02X:%02X:%02X:%02X:%02X\n",
					  PRINT_MAC(pStaCfg->MlmeAux.Bssid)));
			pStaCfg->MlmeAux.Channel = pStaCfg->wdev.channel;
			cntl_connect_request(wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, pStaCfg->MlmeAux.Bssid);
		} else if (MlmeValidateSSID(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen) == TRUE) {
			NDIS_802_11_SSID OidSsid;

			OidSsid.SsidLength = pStaCfg->MlmeAux.AutoReconnectSsidLen;
			NdisMoveMemory(OidSsid.Ssid, pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
			cntl_connect_request(wdev, CNTL_CONNECT_BY_SSID, sizeof(NDIS_802_11_SSID), (UCHAR *)&OidSsid);
		}
	}

/*
	==========================================================================
	Description:
		This routine checks if there're other APs out there capable for
		roaming. Caller should call this routine only when link up in INFRA mode
		and channel quality is below CQI_GOOD_THRESHOLD.

	IRQL = DISPATCH_LEVEL

	Output:
	==========================================================================
 */
BOOLEAN MlmeCheckForFastRoaming(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	USHORT		i;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	BSS_TABLE	 *pRoamTab = &pStaCfg->MlmeAux.RoamTab;
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	BSS_ENTRY	 *pBss;
	CHAR max_rssi;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return FALSE;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> MlmeCheckForFastRoaming\n"));
	/* put all roaming candidates into RoamTab, and sort in RSSI order*/
	BssTableInit(pRoamTab);

	for (i = 0; i < ScanTab->BssNr && i < MAX_LEN_OF_BSS_TABLE; i++) {
			pBss = &ScanTab->BssEntry[i];

		if ((pBss->Rssi <= -50) && (pBss->Channel == wdev->channel))
			continue;	 /* RSSI too weak. forget it.*/

		if (MAC_ADDR_EQUAL(pBss->Bssid, pStaCfg->Bssid))
			continue;	 /* skip current AP*/

		if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, pStaCfg->Ssid, pStaCfg->SsidLen))
			continue;	 /* skip different SSID*/

		max_rssi = RTMPMaxRssi(pAd, pStaCfg->RssiSample.LastRssi[0], pStaCfg->RssiSample.LastRssi[1],
							   pStaCfg->RssiSample.LastRssi[2]);

		if (pBss->Rssi < (max_rssi + RSSI_DELTA))
			continue;	 /* skip AP without better RSSI*/

		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("max_rssi = %d, pBss->Rssi = %d\n", max_rssi, pBss->Rssi));
		/* AP passing all above rules is put into roaming candidate table		 */
		/* fix memory leak when trigger scan continuously */
		BssEntryCopy(pRoamTab, &pRoamTab->BssEntry[pRoamTab->BssNr], pBss);
		pRoamTab->BssNr += 1;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== MlmeCheckForFastRoaming (BssNr=%d)\n", pRoamTab->BssNr));

	if (pRoamTab->BssNr > 0) {
		cntl_connect_request(wdev, CNTL_CONNECT_ROAMING_REQ, 0, NULL);
	}

	return FALSE;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically inside MlmePeriodicExecTimer() after
		association with an AP.
		It checks if StaCfg[0].PwrMgmt.Psm is consistent with user policy (recorded in
		StaCfg[0].WindowsPowerMode). If not, enforce user policy. However,
		there're some conditions to consider:
		1. we don't support power-saving in ADHOC mode, so Psm=PWR_ACTIVE all
		   the time when Mibss==TRUE
		2. When link up in INFRA mode, Psm should not be switch to PWR_SAVE
		   if outgoing traffic available in TxRing or MgmtRing.
	Output:
		1. change pAd->StaCfg[0].Psm to PWR_SAVE or leave it untouched

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeCheckPsmChange(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	ULONG PowerMode = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	/*
		condition -
		1. Psm maybe ON only happen in INFRASTRUCTURE mode
		2. user wants either MAX_PSP or FAST_PSP
		3. but current psm is not in PWR_SAVE
		4. CNTL state machine is not doing SCANning
		5. no TX SUCCESS event for the past 1-sec period
	*/
	PowerMode = pStaCfg->WindowsPowerMode;

	if (pStaCfg->CountDowntoPsm > 0)
		pStaCfg->CountDowntoPsm--;

	if (PowerMode != Ndis802_11PowerModeCAM) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(%d,%ld,%d,%d,%d)\n",
			__func__,
			INFRA_ON(pStaCfg),
			PowerMode,
			cntl_idle(wdev), /* snowpin for cntl mgmt */
			pStaCfg->CountDowntoPsm,
			pStaCfg->PwrMgmt.Psm));
	}

	if (INFRA_ON(pStaCfg) &&
		(PowerMode != Ndis802_11PowerModeCAM) &&
		cntl_idle(wdev) &&
		(pStaCfg->CountDowntoPsm == 0) &&
		(pStaCfg->PwrMgmt.Psm == PWR_ACTIVE)) {

		NdisGetSystemUpTime(&pAd->Mlme.LastSendNULLpsmTime);
		pAd->RalinkCounters.RxCountSinceLastNULL = 0;

#ifdef STA_LP_PHASE_2_SUPPORT
		/* Enter PS and NULL frame(PM=1) is sent by FW */
		RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAd, pStaCfg);
#endif

	}
}


VOID MlmeSetPsmBit(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg, USHORT psm)
{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	USHORT PsmOld = pStaCfg->PwrMgmt.Psm;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
	pStaCfg->PwrMgmt.Psm = psm;
#ifdef DOT11Z_TDLS_SUPPORT
	/* pAd->StaCfg[0].Psm must be updated before calling the function */
	TDLS_UAPSDP_PsmModeChange(pAd, PsmOld, psm);
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef CFG_TDLS_SUPPORT
#ifdef UAPSD_SUPPORT
	cfg_tdls_UAPSDP_PsmModeChange(pAd, PsmOld, psm);
#endif /*UAPSD_SUPPORT*/
#endif /* CFG_TDLS_SUPPORT */
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeSetPsmBit = %d\n", psm));
}
#endif /* CONFIG_STA_SUPPORT */

/*
	==========================================================================
	Description:
		This routine calculates TxPER, RxPER of the past N-sec period. And
		according to the calculation result, ChannelQuality is calculated here
		to decide if current AP is still doing the job.

		If ChannelQuality is not good, a ROAMing attempt may be tried later.
	Output:
		StaCfg[0].ChannelQuality - 0..100

	IRQL = DISPATCH_LEVEL

	NOTE: This routine decide channle quality based on RX CRC error ratio.
		Caller should make sure a function call to NICUpdateRawCountersNew(pAd)
		is performed right before this routine, so that this routine can decide
		channel quality based on the most up-to-date information
	==========================================================================
 */
VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	RSSI_SAMPLE *pRssiSample = NULL;
	UINT32 OneSecTxNoRetryOkCount = 0;
	UINT32 OneSecTxRetryOkCount = 0;
	UINT32 OneSecTxFailCount = 0;
	UINT32 OneSecRxOkCnt = 0;
	UINT32 OneSecRxFcsErrCnt = 0;
	ULONG ChannelQuality = 0;  /* 0..100, Channel Quality Indication for Roaming*/
#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, pMacEntry->wdev);
	ULONG BeaconLostTime = 0;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	BeaconLostTime = pStaCfg->BeaconLostTime;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_MULTI_CHANNEL

	if (pAd->Mlme.bStartMcc)
		BeaconLostTime += (8 * OS_HZ); /* increase 8 seconds */
	else
		BeaconLostTime = pStaCfg->BeaconLostTime;

#endif /* CONFIG_MULTI_CHANNEL */
#ifdef CONFIG_STA_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier*/

	/* longer beacon lost time when carrier detection enabled*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		BeaconLostTime = pStaCfg->BeaconLostTime + (pStaCfg->BeaconLostTime / 2);

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef APCLI_SUPPORT

	if (pMacEntry && IS_ENTRY_PEER_AP(pMacEntry) && (pStaCfg->wdev.func_idx < MAX_APCLI_NUM))
		LastBeaconRxTime = pStaCfg->ApcliInfStat.ApCliRcvBeaconTime;
	else
#endif /*APCLI_SUPPORT*/
		LastBeaconRxTime = pStaCfg->LastBeaconRxTime;

#endif /* CONFIG_STA_SUPPORT */

	if (pMacEntry != NULL) {
		pRssiSample = &pMacEntry->RssiSample;
		OneSecTxNoRetryOkCount = pMacEntry->OneSecTxNoRetryOkCount;
		OneSecTxRetryOkCount = pMacEntry->OneSecTxRetryOkCount;
		OneSecTxFailCount = pMacEntry->OneSecTxFailCount;
		OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
		OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
	} else {
		pRssiSample = &pAd->MacTab.Content[0].RssiSample;
		OneSecTxNoRetryOkCount = pAd->RalinkCounters.OneSecTxNoRetryOkCount;
		OneSecTxRetryOkCount = pAd->RalinkCounters.OneSecTxRetryOkCount;
		OneSecTxFailCount = pAd->RalinkCounters.OneSecTxFailCount;
		OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
		OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
	}

	if (pRssiSample == NULL)
		return;

	MaxRssi = RTMPMaxRssi(pAd, pRssiSample->LastRssi[0],
						  pRssiSample->LastRssi[1],
						  pRssiSample->LastRssi[2]);
	/*
		calculate TX packet error ratio and TX retry ratio - if too few TX samples,
		skip TX related statistics
	*/
	TxOkCnt = OneSecTxNoRetryOkCount + OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + OneSecTxFailCount;

	if (TxCnt < 5) {
		TxPER = 0;
		TxPRR = 0;
	} else {
		TxPER = (OneSecTxFailCount * 100) / TxCnt;
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	/* calculate RX PER - don't take RxPER into consideration if too few sample*/
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;

	if (RxCnt < 5)
		RxPER = 0;
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef STA_LP_PHASE_2_SUPPORT
	if (INFRA_ON(pStaCfg) && pMacEntry &&
		pStaCfg->PwrMgmt.bBeaconLost &&
		pStaCfg->PwrMgmt.bDoze) {
		pStaCfg->ChannelQuality = 0;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::STA/APCLI BEACON lost meet\n", __func__));
	} else
#endif /* STA_LP_PHASE_2_SUPPORT */
#endif /* CONFIG_STA_SUPPORT && APCLI_SUPPORT */
	{
		/* decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER*/
#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) &&
			INFRA_ON(pStaCfg) &&
#if defined(CONFIG_STA_SUPPORT) && defined(STA_LP_PHASE_2_SUPPORT)
			(pStaCfg->PwrMgmt.Psm == PWR_ACTIVE) &&
#endif /* CONFIG_STA_SUPPORT && STA_LP_PHASE_2_SUPPORT */
			(OneSecTxNoRetryOkCount < 2) && /* no heavy traffic*/
#ifdef CONFIG_MULTI_CHANNEL
			(pAd->Mlme.bStartMcc == FALSE) &&
#endif /* CONFIG_MULTI_CHANNEL */
			RTMP_TIME_AFTER(Now32, LastBeaconRxTime + BeaconLostTime)) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

			if ((pMacEntry->TxStatRspCnt > 1) && pMacEntry->TotalTxSuccessCnt) {
				/*
					Beacon Lost but Tx success is increasing, keep connection by update LastBeaconRxTime to currect time
				*/
				pStaCfg->LastBeaconRxTime = Now32;
				ChannelQuality = 90;
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Tx success cnt increasing, update LastBeaconRxTime to %ld\n",
						 LastBeaconRxTime));
			} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BEACON lost > %ld msec with TxOkCnt=%ld -> CQI=0\n",
						 BeaconLostTime * (1000 / OS_HZ), TxOkCnt));
				ChannelQuality = 0;
#if defined(MT7636_FPGA) || defined(MT7637_FPGA) || defined(AXE_FPGA) || defined(MT7915_FPGA)
				/*
					During 7636 M2M, 7636 AP can't send AP, so 7636 STA
					need to disable this to maintain connection.
				*/
				ChannelQuality = 90;
#endif /* MT7636_FPGA || MT7637_FPGA */
			}
		} else
#endif /* CONFIG_STA_SUPPORT */
		{
			/* Normalize Rssi*/
			if (MaxRssi > -40)
				NorRssi = 100;
			else if (MaxRssi < -90)
				NorRssi = 0;
			else
				NorRssi = (MaxRssi + 90) * 2;

			/* ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER	 (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)*/
			ChannelQuality = (RSSI_WEIGHTING * NorRssi +
							  TX_WEIGHTING * (100 - TxPRR) +
							  RX_WEIGHTING * (100 - RxPER)) / 100;
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d), ChannelQuality(%lu)\n", __func__, __LINE__,
					 ChannelQuality));
		}

	}

#ifdef CONFIG_STA_SUPPORT

	if (pAd->OpMode == OPMODE_STA)
		pStaCfg->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		if (pMacEntry != NULL)
			pMacEntry->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
	}

#endif /* CONFIG_AP_SUPPORT */
}

VOID MlmeSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	if (TxPreamble == Rt802_11PreambleLong)
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	else {
		/* NOTE: 1Mbps should always use long preamble*/
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeSetTxPreamble = %s PREAMBLE\n",
			 ((TxPreamble == Rt802_11PreambleLong) ? "LONG" : "SHORT")));
}


/*
    ==========================================================================
    Description:
	Update basic rate bitmap
    ==========================================================================
*/
VOID UpdateBasicRateBitmap(RTMP_ADAPTER *pAdapter, struct wifi_dev *wdev)
{
	INT  i, j;
	/* 1  2  5.5, 11,  6,  9, 12, 18, 24, 36, 48,  54 */
	UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
	UCHAR *sup_p = wdev->rate.legacy_rate.sup_rate;
	UCHAR *ext_p = wdev->rate.legacy_rate.ext_rate;
	ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;

	/* if A mode, always use fix BasicRateBitMap */
	/*if (wdev->channel)*/
	if (wdev->channel > 14) {
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF) {
			/* no 11b rate in 5G band */
			pAdapter->CommonCfg.BasicRateBitmapOld = \
					pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in a-band */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
	}
#ifdef GN_MIXMODE_SUPPORT
	else if (pAdapter->CommonCfg.GNMixMode
			&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
				|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
				|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF) {
			/* no 11b rate in GN MODE */
			pAdapter->CommonCfg.BasicRateBitmapOld = pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in GN MODE */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
	}
#endif /* GN_MIXMODE_SUPPORT */
	else {
		/* no need to modify in 2.4G (bg mixed) */
		pAdapter->CommonCfg.BasicRateBitmap = \
											  pAdapter->CommonCfg.BasicRateBitmapOld;
	}

	if (pAdapter->CommonCfg.BasicRateBitmap > 4095) {
		/* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
		return;
	}

	bitmap = pAdapter->CommonCfg.BasicRateBitmap;  /* renew bitmap value */

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		sup_p[i] &= 0x7f;
		ext_p[i] &= 0x7f;
	}

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (bitmap & (1 << i)) {
			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (sup_p[j] == rate[i])
					sup_p[j] |= 0x80;
			}

			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (ext_p[j] == rate[i])
					ext_p[j] |= 0x80;
			}
		}
	}
}

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRates(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, UCHAR apidx)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;
	struct wifi_dev *wdev = NULL;
	struct legacy_rate *rate;
#ifdef CONFIG_STA_SUPPORT
    PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif
	wdev = get_wdev_by_idx(pAd, apidx);

	if (!wdev)
		return;

#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd, wdev);
	num = 0;
	auto_rate_cur_p = NULL;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		switch (wdev->rate.DesireRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;
			num++;
			break;

		case 4:
			Rate = RATE_2;
			num++;
			break;

		case 11:
			Rate = RATE_5_5;
			num++;
			break;

		case 22:
			Rate = RATE_11;
			num++;
			break;

		case 12:
			Rate = RATE_6;
			num++;
			break;

		case 18:
			Rate = RATE_9;
			num++;
			break;

		case 24:
			Rate = RATE_12;
			num++;
			break;

		case 36:
			Rate = RATE_18;
			num++;
			break;

		case 48:
			Rate = RATE_24;
			num++;
			break;

		case 72:
			Rate = RATE_36;
			num++;
			break;

		case 96:
			Rate = RATE_48;
			num++;
			break;

		case 108:
			Rate = RATE_54;
			num++;
			break;
			/*default: Rate = RATE_1;   break;*/
		}

		if (MaxDesire < Rate)
			MaxDesire = Rate;
	}

	/*===========================================================================*/
	do {
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT

		/* CFG TODO */
		if (apidx >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO) {
			printk("%s: Update for GO\n", __func__);
			wdev = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX].wdev;
			break;
		}

#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO) {
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_P2P_GO;

			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			break;
		}

#endif /* P2P_SUPPORT */
#ifdef APCLI_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM) {
				wdev = &pAd->StaCfg[idx].wdev;
				break;
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid idx(%d)\n", __func__, idx));
				return;
			}
		}

#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT

			if (apidx >= MIN_NET_DEVICE_FOR_WDS) {
				UCHAR idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY) {
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				} else {
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __func__, apidx));
					return;
				}
			}

#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) &&
				(VALID_MBSS(pAd, apidx)))
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __func__, apidx));

			break;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if ((pStaCfg->BssType == BSS_ADHOC) &&
				WMODE_EQUAL(wdev->PhyMode, WMODE_B) &&
				(MaxDesire > RATE_11)) {
				MaxDesire = RATE_11;
			}

			break;
		}
#endif /* CONFIG_STA_SUPPORT */
	} while (FALSE);

	if (wdev) {
		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;
		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
		HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	}

	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;

	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

#ifdef CONFIG_STA_SUPPORT
	if (pStaCfg && (ADHOC_ON(pAd) || INFRA_ON(pStaCfg))
		&& ((wdev->wdev_type == WDEV_TYPE_STA) || (wdev->wdev_type == WDEV_TYPE_ADHOC)))
	{
		rate = &pStaCfg->StaActive.rate;
	} else
#endif /* CONFIG_STA_SUPPORT */
	{
		rate = &wdev->rate.legacy_rate;
	}

	pSupRate = &rate->sup_rate[0];
	pExtRate = &rate->ext_rate[0];
	SupRateLen = rate->sup_rate_len;
	ExtRateLen = rate->ext_rate_len;

	/* find max supported rate */
	for (i = 0; i < SupRateLen; i++) {
		switch (pSupRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i = 0; i < ExtRateLen; i++) {
		switch (pExtRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}


	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;

		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
			 __func__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));

	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;

	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p) {
		short dbm = 0;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		dbm = pAd->StaCfg[0].RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		dbm = 0;
#endif /* CONFIG_AP_SUPPORT */

		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;
	} else {
		wdev->rate.TxRate = wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6) {
			if (HtMcs <= HT_MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		} else {
			if (HtMcs <= HT_MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (bLinkUp) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				pEntry = GetAssociatedAPByWdev(pAd, wdev);
				pEntry->HTPhyMode.field.STBC = pHtPhy->field.STBC;
				pEntry->HTPhyMode.field.ShortGI = pHtPhy->field.ShortGI;
				pEntry->HTPhyMode.field.MCS = pHtPhy->field.MCS;
				pEntry->HTPhyMode.field.MODE	= pHtPhy->field.MODE;
			}
		}
#endif
	}

	if (wdev->rate.TxRate <= RATE_11) {
		pMaxHtPhy->field.MODE = MODE_CCK;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pMaxHtPhy->field.MCS = pAd->CommonCfg.TxRate;
			pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = MaxDesire;

#endif /* P2P_SUPPORT */
	} else {
		pMaxHtPhy->field.MODE = MODE_OFDM;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.TxRate];

			if (pAd->CommonCfg.MinTxRate >= RATE_6 && (pAd->CommonCfg.MinTxRate <= RATE_54))
				pMinHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MinTxRate];
			else
				pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];

#endif /* P2P_SUPPORT */
	}

	pHtPhy->word = (pMaxHtPhy->word);

	if (bLinkUp && (pAd->OpMode == OPMODE_STA)) {
#ifdef CONFIG_STA_SUPPORT
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		pEntry->HTPhyMode.word = pHtPhy->word;
		pEntry->MaxHTPhyMode.word = pMaxHtPhy->word;
		pEntry->MinHTPhyMode.word = pMinHtPhy->word;
#endif
	} else {
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			(wdev->channel <= 14)
#ifdef GN_MIXMODE_SUPPORT
			&& (!(pAd->CommonCfg.GNMixMode))
#endif /*GN_MIXMODE_SUPPORT*/
		) {
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		} else {
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (wdev->eap.eap_mgmrate_en == TRUE) {
			wdev->rate.TxRate = wdev->eap.mgmphymode.field.MCS;
			wdev->rate.MlmeTransmit = wdev->eap.mgmphymode;
		}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;

		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;

		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
			HTTRANSMIT_SETTING tPhyMode, *transmit;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
			transmit  = (wdev->channel > 14) ? (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
			transmit  = &wdev->rate.mcastphymode;
#endif
			if (memcmp(transmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0) {
				memmove(transmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
						sizeof(HTTRANSMIT_SETTING));
			}

			/*
			printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __func__,
				(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
				pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
			*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
			  __func__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
			  RateIdToMbps[wdev->rate.MaxTxRate],
			  RateIdToMbps[wdev->rate.MinTxRate],
			  /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
			 __func__, RateIdToMbps[wdev->rate.TxRate],
			 RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (bLinkUp) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			pEntry = GetAssociatedAPByWdev(pAd, wdev);
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): (MlmeTransmit=0x%x, MinHTPhyMode=%x, MaxHTPhyMode=0x%x, HTPhyMode=0x%x)\n",
					  __func__, wdev->rate.MlmeTransmit.word,
					  pEntry->MinHTPhyMode.word,
					  pEntry->MaxHTPhyMode.word,
					  pEntry->HTPhyMode.word));
		}
	}
#endif
}


/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRatesWdev(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, struct wifi_dev *wdev)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;
	struct legacy_rate *rate;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif

	if (!wdev)
		return;

	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd, wdev);
	num = 0;
	auto_rate_cur_p = NULL;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		switch (wdev->rate.DesireRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;
			num++;
			break;

		case 4:
			Rate = RATE_2;
			num++;
			break;

		case 11:
			Rate = RATE_5_5;
			num++;
			break;

		case 22:
			Rate = RATE_11;
			num++;
			break;

		case 12:
			Rate = RATE_6;
			num++;
			break;

		case 18:
			Rate = RATE_9;
			num++;
			break;

		case 24:
			Rate = RATE_12;
			num++;
			break;

		case 36:
			Rate = RATE_18;
			num++;
			break;

		case 48:
			Rate = RATE_24;
			num++;
			break;

		case 72:
			Rate = RATE_36;
			num++;
			break;

		case 96:
			Rate = RATE_48;
			num++;
			break;

		case 108:
			Rate = RATE_54;
			num++;
			break;
			/*default: Rate = RATE_1;   break;*/
		}

		if (MaxDesire < Rate)
			MaxDesire = Rate;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if ((pStaCfg->BssType == BSS_ADHOC) &&
			WMODE_EQUAL(wdev->PhyMode, WMODE_B) &&
			(MaxDesire > RATE_11)) {
			MaxDesire = RATE_11;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	pHtPhy = &wdev->HTPhyMode;
	pMaxHtPhy = &wdev->MaxHTPhyMode;
	pMinHtPhy = &wdev->MinHTPhyMode;
	auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;

	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

#ifdef CONFIG_STA_SUPPORT
	if (pStaCfg && (ADHOC_ON(pAd) || INFRA_ON(pStaCfg))
		&& ((wdev->wdev_type == WDEV_TYPE_STA) || (wdev->wdev_type == WDEV_TYPE_ADHOC)))
	{
		rate = &pStaCfg->StaActive.rate;
	} else
#endif /* CONFIG_STA_SUPPORT */
	{
		rate = &wdev->rate.legacy_rate;
	}

	pSupRate = &rate->sup_rate[0];
	pExtRate = &rate->ext_rate[0];
	SupRateLen = rate->sup_rate_len;
	ExtRateLen = rate->ext_rate_len;

	/* find max supported rate */
	for (i = 0; i < SupRateLen; i++) {
		switch (pSupRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pSupRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pSupRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i = 0; i < ExtRateLen; i++) {
		switch (pExtRate[i] & 0x7f) {
		case 2:
			Rate = RATE_1;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 0;

			break;

		case 4:
			Rate = RATE_2;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 1;

			break;

		case 11:
			Rate = RATE_5_5;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 2;

			break;

		case 22:
			Rate = RATE_11;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 3;

			break;

		case 12:
			Rate = RATE_6;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 4;
			break;

		case 18:
			Rate = RATE_9;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 5;

			break;

		case 24:
			Rate = RATE_12;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 6;
			break;

		case 36:
			Rate = RATE_18;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 7;

			break;

		case 48:
			Rate = RATE_24;
			/*if (pExtRate[i] & 0x80)*/
			BasicRateBitmap |= 1 << 8;
			break;

		case 72:
			Rate = RATE_36;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 9;

			break;

		case 96:
			Rate = RATE_48;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 10;

			break;

		case 108:
			Rate = RATE_54;

			if (pExtRate[i] & 0x80)
				BasicRateBitmap |= 1 << 11;

			break;

		default:
			Rate = RATE_1;
			break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}


	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;

		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
			 __func__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));

	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;

	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p) {
		short dbm = 0;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		dbm = pAd->StaCfg[0].RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		dbm = 0;
#endif /* CONFIG_AP_SUPPORT */

		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;
	} else {
		wdev->rate.TxRate = wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6) {
			if (HtMcs <= HT_MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		} else {
			if (HtMcs <= HT_MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (bLinkUp) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				pEntry = GetAssociatedAPByWdev(pAd, wdev);
				pEntry->HTPhyMode.field.STBC = pHtPhy->field.STBC;
				pEntry->HTPhyMode.field.ShortGI = pHtPhy->field.ShortGI;
				pEntry->HTPhyMode.field.MCS = pHtPhy->field.MCS;
				pEntry->HTPhyMode.field.MODE	= pHtPhy->field.MODE;
			}
		}
#endif
	}

	if (wdev->rate.TxRate <= RATE_11) {
		pMaxHtPhy->field.MODE = MODE_CCK;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pMaxHtPhy->field.MCS = pAd->CommonCfg.TxRate;
			pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = MaxDesire;

#endif /* P2P_SUPPORT */
	} else {
		pMaxHtPhy->field.MODE = MODE_OFDM;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.TxRate];

			if (pAd->CommonCfg.MinTxRate >= RATE_6 && (pAd->CommonCfg.MinTxRate <= RATE_54))
				pMinHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MinTxRate];
			else
				pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];

#endif /* P2P_SUPPORT */
	}

	pHtPhy->word = (pMaxHtPhy->word);

	if (bLinkUp && (pAd->OpMode == OPMODE_STA)) {
#ifdef CONFIG_STA_SUPPORT
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		pEntry->HTPhyMode.word = pHtPhy->word;
		pEntry->MaxHTPhyMode.word = pMaxHtPhy->word;
		pEntry->MinHTPhyMode.word = pMinHtPhy->word;
#endif
	} else {
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			(wdev->channel <= 14)
#ifdef GN_MIXMODE_SUPPORT
			&& (!(pAd->CommonCfg.GNMixMode))
#endif /*GN_MIXMODE_SUPPORT*/
		) {
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		} else {
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;

		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;

		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
			HTTRANSMIT_SETTING tPhyMode, *transmit;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));

#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
			transmit  = (wdev->channel > 14) ? (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
			transmit  = &wdev->rate.mcastphymode;
#endif
			if (memcmp(transmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0) {
				memmove(transmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
						sizeof(HTTRANSMIT_SETTING));
			}

			/*
			printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __func__,
					(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
					pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
			*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
			  __func__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
			  RateIdToMbps[wdev->rate.MaxTxRate],
			  RateIdToMbps[wdev->rate.MinTxRate],
			  /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
			 __func__, RateIdToMbps[wdev->rate.TxRate],
			 RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (bLinkUp) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			pEntry = GetAssociatedAPByWdev(pAd, wdev);
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): (MlmeTransmit=0x%x, MinHTPhyMode=%x, MaxHTPhyMode=0x%x, HTPhyMode=0x%x)\n",
					  __func__, wdev->rate.MlmeTransmit.word,
					  pEntry->MinHTPhyMode.word,
					  pEntry->MaxHTPhyMode.word,
					  pEntry->HTPhyMode.word));
		}
	}
#endif
}


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		This function update HT Rate setting.
		Input Wcid value is valid for 2 case :
		1. it's used for Station in infra mode that copy AP rate to Mactable.
		2. OR Station	in adhoc mode to copy peer's HT rate to Mactable.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR StbcMcs;
	HT_CAPABILITY_IE *curr_ht_cap;
	RT_PHY_INFO *pActiveHtPhy = NULL;
	ULONG BasicMCS;
	RT_PHY_INFO *pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING pHtPhy = NULL;
	PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING pMinHtPhy = NULL;
#ifdef DOT11_HE_AX
	PHE_TRANSMIT_SETTING pHEPhy = NULL;
	PHE_TRANSMIT_SETTING pMaxHePhy = NULL;
#endif /*DOT11_HE_AX*/
	BOOLEAN *auto_rate_cur_p;
	ADD_HT_INFO_IE *addht;
	UCHAR ht_bw;
	UCHAR gf, stbc;
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()===>\n", __func__));
	auto_rate_cur_p = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid wdev (%p)\n", __func__, wdev));
		return;
	}

#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	pDesireHtPhy = &wdev->DesiredHtPhyInfo;
	pActiveHtPhy = &wdev->DesiredHtPhyInfo;
	pHtPhy = &wdev->HTPhyMode;
	pMaxHtPhy = &wdev->MaxHTPhyMode;
	pMinHtPhy = &wdev->MinHTPhyMode;
	auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	addht = wlan_operate_get_addht(wdev);

	/* For STA mode, take out ht_bw from operation parameter else from config parameter. */
	if ((wdev->wdev_type == WDEV_TYPE_STA) || (wdev->wdev_type == WDEV_TYPE_REPEATER))
		ht_bw = wlan_operate_get_ht_bw(wdev);
	else
		ht_bw = wlan_config_get_ht_bw(wdev);

	curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			TxPath = pAd->dbdc_band0_tx_path;
		else
			TxPath = pAd->dbdc_band1_tx_path;
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
{
	UINT8 BandIdx = HcGetBandByWdev(wdev);
	if (pAd->bAntennaSetAPEnable[BandIdx])
		TxPath = pAd->TxStream[BandIdx];
}
#endif /* ANTENNA_CONTROL_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	if (pStaCfg && (ADHOC_ON(pAd) || INFRA_ON(pStaCfg))
		&& ((wdev->wdev_type == WDEV_TYPE_STA) || (wdev->wdev_type == WDEV_TYPE_ADHOC)))
	{
		RT_HT_CAPABILITY *pRtHtCap = NULL;

		if (pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
			return;

		pRtHtCap = &pStaCfg->StaActive.SupportedHtPhy;
		pActiveHtPhy = &pStaCfg->StaActive.SupportedPhyInfo;
		StbcMcs = (UCHAR)pStaCfg->MlmeAux.AddHtInfo.AddHtInfo3.StbcMcs;
		BasicMCS = pStaCfg->MlmeAux.AddHtInfo.MCSSet[0] + (pStaCfg->MlmeAux.AddHtInfo.MCSSet[1] << 8) + (StbcMcs << 16);

		stbc = pRtHtCap->RxSTBC;
		gf = pRtHtCap->GF;
		if ((curr_ht_cap->HtCapInfo.TxSTBC) && stbc && (TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	} else
#endif /* CONFIG_STA_SUPPORT */
	{
		if ((!pDesireHtPhy) || pDesireHtPhy->bHtEnable == FALSE)
			return;

		curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
		stbc = curr_ht_cap->HtCapInfo.RxSTBC;
		gf = curr_ht_cap->HtCapInfo.GF;
		StbcMcs = (UCHAR) addht->AddHtInfo3.StbcMcs;
		BasicMCS = addht->MCSSet[0] + (addht->MCSSet[1] << 8) + (StbcMcs << 16);
		if ((curr_ht_cap->HtCapInfo.TxSTBC) && (TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	/* Decide MAX ht rate.*/
	if (gf)
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

	if (ht_bw)
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

	if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = curr_ht_cap->HtCapInfo.ShortGIfor20;
	else
		pMaxHtPhy->field.ShortGI = curr_ht_cap->HtCapInfo.ShortGIfor40;

	if (pDesireHtPhy->MCSSet[4] != 0)
		pMaxHtPhy->field.MCS = 32;

	pMaxHtPhy->field.MCS = get_ht_max_mcs(&pDesireHtPhy->MCSSet[0],
										  &pActiveHtPhy->MCSSet[0]);
	/* Copy MIN ht rate.  rt2860???*/
	pMinHtPhy->field.BW = BW_20;
	pMinHtPhy->field.MCS = 0;
	pMinHtPhy->field.STBC = 0;
	pMinHtPhy->field.ShortGI = 0;
	/*If STA assigns fixed rate. update to fixed here.*/
#ifdef CONFIG_STA_SUPPORT

	if ((pAd->OpMode == OPMODE_STA) && (pDesireHtPhy->MCSSet[0] != 0xff)
#ifdef P2P_SUPPORT
		&& (apidx == BSS0)
#endif /* P2P_SUPPORT */
	   ) {
		CHAR i;
		UCHAR j, bitmask;

		if (pDesireHtPhy->MCSSet[4] != 0) {
			pMaxHtPhy->field.MCS = 32;
			pMinHtPhy->field.MCS = 32;
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():<=== Use Fixed MCS = %d\n", __func__,
					 pMinHtPhy->field.MCS));
		}

		for (i = 31; (CHAR)i >= 0; i--) {
			j = i / 8;
			bitmask = (1 << (i - (j * 8)));

			if ((pDesireHtPhy->MCSSet[j] & bitmask) && (pActiveHtPhy->MCSSet[j] & bitmask)) {
				pMaxHtPhy->field.MCS = i;
				pMinHtPhy->field.MCS = i;
				break;
			}

			if (i == 0)
				break;
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	/* Decide ht rate*/
	pHtPhy->field.STBC = pMaxHtPhy->field.STBC;
	pHtPhy->field.BW = pMaxHtPhy->field.BW;
	pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
	pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
	pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;

	/* use default now. rt2860*/
	if (pDesireHtPhy->MCSSet[0] != 0xff)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesireHtPhy->bVhtEnable = TRUE;

		/* For STA mode, take out vht_bw from operation parameter else from config parameter. */
		if ((wdev->wdev_type == WDEV_TYPE_STA) || (wdev->wdev_type == WDEV_TYPE_REPEATER))
			pDesireHtPhy->vht_bw = wlan_operate_get_vht_bw(wdev);
		else
			rtmp_set_vht(pAd, wdev, pDesireHtPhy);

		if (pDesireHtPhy->bVhtEnable == TRUE) {
			PHTTRANSMIT_SETTING pHtPhy = NULL;
			PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
			PHTTRANSMIT_SETTING pMinHtPhy = NULL;

			pHtPhy = &wdev->HTPhyMode;
			pMaxHtPhy = &wdev->MaxHTPhyMode;
			pMinHtPhy = &wdev->MinHTPhyMode;
			pMaxHtPhy->field.MODE = MODE_VHT;

			if (pDesireHtPhy->vht_bw == VHT_BW_2040) {/* use HT BW setting */
				if (pHtPhy->field.BW == BW_20)
					pMaxHtPhy->field.MCS = 8;
				else
					pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_80) {
				pMaxHtPhy->field.BW = BW_80;
				pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_160) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			} else if (pDesireHtPhy->vht_bw == VHT_BW_8080) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			}

			pMaxHtPhy->field.ShortGI = wlan_config_get_vht_sgi(wdev);
			/* Decide ht rate*/
			pHtPhy->field.BW = pMaxHtPhy->field.BW;
			pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
			pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
			pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;
		}
	}

#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode)) {

		pHEPhy = &wdev->HEPhyMode;
		pMaxHePhy = &wdev->MaxHEPhyMode;
		pMaxHePhy->field.MODE = MODE_HE;
		pMaxHePhy->field.MCS = 11;
		pMaxHePhy->field.ShortGI = wdev->HTPhyMode.field.ShortGI;
		pMaxHePhy->field.BW = wlan_config_get_he_bw(wdev);
		pMaxHePhy->field.STBC = wlan_config_get_he_tx_stbc(wdev);
		pMaxHePhy->field.Nss = wlan_config_get_he_tx_nss(wdev);

		pHEPhy->field.BW = pMaxHePhy->field.BW;
		pHEPhy->field.MODE = pMaxHePhy->field.MODE;
		pHEPhy->field.MCS = pMaxHePhy->field.MCS;
		pHEPhy->field.ShortGI = pMaxHePhy->field.ShortGI;
		pHEPhy->field.STBC = pMaxHePhy->field.STBC;
		pHEPhy->field.Nss = pMaxHePhy->field.Nss;
	}
#endif /*DOT11_HE_AX*/

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s():<---.AMsduSize = %d\n", __func__,
			 curr_ht_cap->HtCapInfo.AMsduSize));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,\n", pActiveHtPhy->MCSSet[0], pHtPhy->field.MCS,
			  pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():<===\n", __func__));
}


#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_VHT_AC
VOID MlmeUpdateVhtTxRates(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, struct wifi_dev *wdev)
{
	UCHAR ucMaxMcs = 0, ucMaxNss = 0;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid wdev (%p)\n", __func__, wdev));
		return;
	}

	pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		(wdev->channel > 14) &&
		(pEntry->SupportRateMode & SUPPORT_VHT_MODE)) {
		if (pEntry->SupportVHTMCS4SS != 0) {
			if (pEntry->SupportVHTMCS4SS & (1 << 9))
				ucMaxMcs = 9;
			else if (pEntry->SupportVHTMCS4SS & (1 << 8))
				ucMaxMcs = 8;
			else
				ucMaxMcs = 7;

			ucMaxNss = 3;
		} else if (pEntry->SupportVHTMCS3SS != 0) {
			if (pEntry->SupportVHTMCS3SS & (1 << 9))
				ucMaxMcs = 9;
			else if (pEntry->SupportVHTMCS3SS & (1 << 8))
				ucMaxMcs = 8;
			else
				ucMaxMcs = 7;

			ucMaxNss = 2;
		} else if (pEntry->SupportVHTMCS2SS != 0) {
			if (pEntry->SupportVHTMCS2SS & (1 << 9))
				ucMaxMcs = 9;
			else if (pEntry->SupportVHTMCS2SS & (1 << 8))
				ucMaxMcs = 8;
			else
				ucMaxMcs = 7;

			ucMaxNss = 1;
		} else if (pEntry->SupportVHTMCS1SS != 0) {
			if (pEntry->SupportVHTMCS1SS & (1 << 9))
				ucMaxMcs = 9;
			else if (pEntry->SupportVHTMCS1SS & (1 << 8))
				ucMaxMcs = 8;
			else
				ucMaxMcs = 7;

			ucMaxNss = 0;
		}

		wdev->MaxHTPhyMode.field.MODE = MODE_VHT;
		wdev->MaxHTPhyMode.field.MCS = (ucMaxNss << 4) + ucMaxMcs;

		if (pStaCfg->StaActive.SupportedPhyInfo.vht_bw == VHT_BW_80)
			wdev->MaxHTPhyMode.field.BW = BW_80;

		if ((pStaCfg->StaActive.SupportedPhyInfo.vht_bw == VHT_BW_160) ||
			(pStaCfg->StaActive.SupportedPhyInfo.vht_bw == VHT_BW_8080))
			wdev->MaxHTPhyMode.field.BW = BW_160;

		wdev->HTPhyMode.word = wdev->MaxHTPhyMode.word;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() MaxHTPhyMode=%x\n", __func__, wdev->MaxHTPhyMode.word));
	}
}
#endif /* DOT11_VHT_AC */
#endif /* CONFIG_STA_SUPPORT */
#endif /* DOT11_N_SUPPORT */

VOID MlmeRadioOff(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
	MTRadioOff(pAd, wdev);
}

VOID MlmeRadioOn(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTRadioOn(pAd, wdev);
	RTMP_OS_NETDEV_START_QUEUE(wdev->if_dev);
}

VOID MlmeLpEnter(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return;
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		if (IS_MT7622(pAd)) {
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				MSTAStop(pAd, wdev);
			}
#endif /* CONFIG_STA_SUPPORT */
			MlmeRadioOff(pAd, wdev);
		} else
			MTMlmeLpEnter(pAd, wdev);
	}

#endif /* RTMP_MAC_PCI */
}


VOID MlmeLpExit(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif /* CONFIG_STA_SUPPORT */
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: wdev is NULL\n", __func__));
		return;
	}
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		if (wdev) {
			if (IS_MT7622(pAd))
				MlmeRadioOn(pAd, wdev);
			else
				MTMlmeLpExit(pAd, wdev);
		}
	}

#endif /* RTMP_MAC_PCI */
}

/*! \brief generates a random mac address value for IBSS BSSID
 *	\param Addr the bssid location
 *	\return none
 *	\pre
 *	\post
 */
VOID MacAddrRandomBssid(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;

	for (i = 0; i < MAC_ADDR_LEN; i++)
		pAddr[i] = RandomByte(pAd);

	pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  /* the first 2 bits must be 01xxxxxxxx*/
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT * AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}

/*! \brief init the management mac frame header
 *	\param p_hdr mac header
 *	\param subtype subtype of the frame
 *	\param p_ds destination address, don't care if it is a broadcast address
 *	\return none
 *	\pre the station has the following information in the pAd->StaCfg[0]
 *	 - bssid
 *	 - station address
 *	\post
 *	\note this function initializes the following field
 */
VOID MgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	COPY_MAC_ADDR(pHdr80211->Addr2, pSA);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	COPY_MAC_ADDR(pHdr80211->Addr2, pSA);
#endif /* CONFIG_STA_SUPPORT */
#endif /* P2P_SUPPORT */
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


VOID MgtMacHeaderInitExt(
	IN RTMP_ADAPTER *pAd,
	IN OUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pAddr1,
	IN UCHAR *pAddr2,
	IN UCHAR *pAddr3)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pAddr1);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAddr2);
	COPY_MAC_ADDR(pHdr80211->Addr3, pAddr3);
}


/*!***************************************************************************
 * This routine build an outgoing frame, and fill all information specified
 * in argument list to the frame body. The actual frame size is the summation
 * of all arguments.
 * input params:
 *		Buffer - pointer to a pre-allocated memory segment
 *		args - a list of <int arg_size, arg> pairs.
 *		NOTE NOTE NOTE!!!! the last argument must be NULL, otherwise this
 *						   function will FAIL!!!
 * return:
 *		Size of the buffer
 * usage:
 *		MakeOutgoingFrame(Buffer, output_length, 2, &fc, 2, &dur, 6, p_addr1, 6,p_addr2, END_OF_ARGS);

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 ****************************************************************************/
ULONG MakeOutgoingFrame(UCHAR *Buffer, ULONG *FrameLen, ...)
{
	UCHAR   *p;
	int	leng;
	ULONG	TotLeng;
	va_list Args;
	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);

	do {
		leng = va_arg(Args, int);

		if (leng == END_OF_ARGS)
			break;

		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while (TRUE);

	va_end(Args); /* clean up */
	*FrameLen = TotLeng;
	return TotLeng;
}


/*! \brief	Initialize The MLME Queue, used by MLME Functions
 *	\param	*Queue	   The MLME Queue
 *	\return Always	   Return NDIS_STATE_SUCCESS in this implementation
 *	\pre
 *	\post
 *	\note	Because this is done only once (at the init stage), no need to be locked

 IRQL = PASSIVE_LEVEL

 */
NDIS_STATUS MlmeQueueInit(RTMP_ADAPTER *pAd)
{
	INT i;
	UCHAR idx;
	ULONG max_len;
	MLME_QUEUE *pQueue;
	for (idx = 0; idx < MAX_NUM_OF_MLME_QUEUE; idx++) {
		switch (idx) {
			case 0:
				pQueue = &pAd->Mlme.Queue;
				max_len = MAX_LEN_OF_MLME_QUEUE;
				break;
#ifdef MLME_MULTI_QUEUE_SUPPORT
			case 1:
				pQueue = (MLME_QUEUE *) &pAd->Mlme.HPQueue;
				max_len = MAX_LEN_OF_MLME_HP_QUEUE;
				break;
			case 2:
				pQueue = (MLME_QUEUE *) &pAd->Mlme.LPQueue;
				max_len = MAX_LEN_OF_MLME_LP_QUEUE;
				break;
#endif
			default:
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No mlme queue matched!, idx = %d\n", __func__, idx));
				break;
		}
		NdisAllocateSpinLock(pAd, &pQueue->Lock);
		pQueue->Num = 0;
		pQueue->Head = 0;
		pQueue->Tail = 0;
		pQueue->MaxLen = max_len;
		for (i = 0; i < pQueue->MaxLen; i++) {
			pQueue->Entry[i].Occupied = FALSE;
			pQueue->Entry[i].MsgLen = 0;
			NdisZeroMemory(pQueue->Entry[i].Msg, MAX_MGMT_PKT_LEN);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT

/*! \brief	 Enqueue a message for other threads, if they want to send messages to MLME thread
 *	\param	*Queue	  The MLME Queue
 *	\param	 Machine  The State Machine Id
 *	\param	 MsgType  The Message Type
 *	\param	 MsgLen   The Message length
 *	\param	*Msg	  The message pointer
 *	\return  TRUE if enqueue is successful, FALSE if the queue is full
 *	\pre
 *	\post
 *	\note	 The message has to be initialized
 */
BOOLEAN MlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv)
{
	ULONG Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef MLME_MULTI_QUEUE_SUPPORT
	if (pAd->Mlme.MultiQEnable)
		Queue = (MLME_QUEUE *) MlmeQueueSelByMach(pAd, Machine, MsgType, NULL);
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == Queue->MaxLen)
		Queue->Tail = 0;

	Queue->Entry[Tail].Wcid = WCID_NO_MATCHED(pAd);
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;
	Queue->Entry[Tail].wdev = NULL;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

#endif

BOOLEAN MlmeEnqueueWithWdev(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv,
	IN struct wifi_dev *wdev)
{
	INT Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef MLME_MULTI_QUEUE_SUPPORT
	if (pAd->Mlme.MultiQEnable)
		Queue = (MLME_QUEUE *) MlmeQueueSelByMach(pAd, Machine, MsgType, NULL);
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

	ASSERT(wdev);
	if (!wdev)
		return FALSE;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/*check wdev is not ready*/
	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev(%d) state: if down!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == Queue->MaxLen)
		Queue->Tail = 0;

	Queue->Entry[Tail].Wcid = WCID_NO_MATCHED(pAd);
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;
	Queue->Entry[Tail].wdev = wdev;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


/*! \brief	 This function is used when Recv gets a MLME message
 *	\param	*Queue			 The MLME Queue
 *	\param	 TimeStampHigh	 The upper 32 bit of timestamp
 *	\param	 TimeStampLow	 The lower 32 bit of timestamp
 *	\param	 Rssi			 The receiving RSSI strength
 *	\param	 MsgLen		 The length of the message
 *	\param	*Msg			 The message pointer
 *	\return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *	\pre
 *	\post
 */
BOOLEAN MlmeEnqueueForRecv(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN struct raw_rssi_info *rssi_info,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN UCHAR OpMode,
	IN struct wifi_dev *wdev,
	IN UCHAR RxPhyMode)
{
	ULONG Tail;
	INT Machine = 0xff;
	UINT32 TimeStampHigh, TimeStampLow;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT MsgType = 0x0;
	MLME_QUEUE *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
	UCHAR *pData=NULL; 
	UCHAR SSID_lENS=0; 
	UCHAR SSID_IES; 
	UCHAR SSIDS[MAX_LEN_OF_SSID]={0};
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	BOOLEAN bToApCli = FALSE;
	UCHAR ApCliIdx = 0;
	BOOLEAN ApCliIdx_find = FALSE;
#ifdef MBSS_SUPPORT
	UCHAR MBSSIdx = 0;
	BOOLEAN ApBssIdx_find = FALSE;
#endif /* MBSS_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	struct wifi_dev *msg_recv_wdev = NULL;

#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return FALSE;

#endif /* CONFIG_ATE */

	/*
		Do nothing if the driver is starting halt state.
		This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __func__));
		return FALSE;
	}

	if (!wdev) {
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():wdev can't find!\n", __func__));
			NdisReleaseSpinLock(&(Queue->Lock));
			return FALSE;
		}
#endif
		return FALSE;
	}

	/*check if wdev is ready except for BMC packet, because it doesn't need wdev */
	if (!wdev->if_up_down_state && Wcid != WCID_NO_MATCHED(pAd)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): wdev (%d) not ready\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame too large, size = %ld\n", __func__, MsgLen));
		return FALSE;
	}

	if (Msg == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame is Null\n", __func__));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 0)
#ifdef MLME_MULTI_QUEUE_SUPPORT
		&& !pAd->Mlme.MultiQEnable
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
	) {
		RTMP_MLME_HANDLER(pAd);
		return FALSE;
	}

	msg_recv_wdev = wdev;
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)

	if (OpMode == OPMODE_AP)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
#ifdef P2P_SUPPORT

		if (P2P_CLI_ON(pAd)) {
			if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_REQ)
				return FALSE;
		}

#endif /* P2P_SUPPORT */

		/*
			Beacon must be handled by ap-sync state machine.
			Probe-rsp must be handled by apcli-sync state machine.
			Those packets don't need to check its MAC address
		*/
		do {
			UCHAR i;
#ifdef APCLI_SUPPORT
			/*
			   1. When P2P GO On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: when GO On skip preCheckMsgTypeSubset redirect to APMsgTypeSubst
			   2. When P2P Cli On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: handle MsgType == APCLI_MT2_PEER_PROBE_RSP on ApCli Sync state machine
					when ApCli on idle state.
			*/
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->StaCfg[i].MlmeAux.Bssid, pFrame->Hdr.Addr2) && pAd->StaCfg[i].wdev.if_up_down_state) {
					/* APCLI_CONNECTION_TRIAL don't need to seperate the ApCliIdx, otherwise the ApCliIdx will be wrong on apcli DBDC mode. */
					ApCliIdx = i;
					bToApCli = TRUE;
					break;
				}
			}

			/* check if da is to apcli */
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->StaCfg[i].wdev.if_addr, pFrame->Hdr.Addr1)) {
					ApCliIdx = i;
					bToApCli = TRUE;
					ApCliIdx_find = TRUE;
					msg_recv_wdev = &pAd->StaCfg[i].wdev;
					break;
				}
			}

			/* check if da is to ap */
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, pFrame->Hdr.Addr1)) {
#if defined(CONFIG_AP_SUPPORT) && defined(MBSS_SUPPORT)
					MBSSIdx = i;
					ApBssIdx_find = TRUE;
#endif
					msg_recv_wdev = &pAd->ApCfg.MBSSID[i].wdev;
					break;
				}
			}

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn) && (bToApCli == TRUE) &&
				(pFrame->Hdr.FC.SubType == SUBTYPE_AUTH)) {
				REPEATER_CLIENT_ENTRY *rept = NULL;
				/* At this moment, for auth rps wcid and wdev will be those of apcli, */
				/* mac table inserted when assoc. So we need to do sw search for rept */
				rept = RTMPLookupRepeaterCliEntry(
											pAd,
											FALSE,
											pFrame->Hdr.Addr1,
											TRUE);
				if (rept && rept->wdev.if_up_down_state)
					msg_recv_wdev = &rept->wdev;
			}
#endif /* MAC_REPEATER_SUPPORT */

			if ((ApBssIdx_find == FALSE) &&
#ifdef P2P_SUPPORT
				!P2P_GO_ON(pAd) &&
#endif /* P2P_SUPPORT */
				preCheckMsgTypeSubset(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case and apcli case*/
				if (bToApCli == TRUE) {
					if ((Machine == SYNC_FSM) &&
						(MsgType == SYNC_FSM_PEER_BEACON)) {
						ULONG Now32;

						NdisGetSystemUpTime(&Now32);
						pAd->StaCfg[ApCliIdx].ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv = Now32;
						pAd->StaCfg[ApCliIdx].ApcliInfStat.ApCliRcvBeaconTime_MlmeEnqueueForRecv_2 = pAd->Mlme.Now32;
					}
				} else {
					/* adjust the ProbeRsp/Beacon to right wdev in same band */
					UCHAR WdevBandIdx = DBDC_BAND0, ChBandIdx = DBDC_BAND0;
					UCHAR RecvCh = (rssi_info->Channel == 0) ? pAd->LatchRfRegs.Channel : rssi_info->Channel;

					ChBandIdx = HcGetBandInfoByChannel(pAd, RecvCh);

					/* assign the wdev to the scanning STA wdev */
					if ((pAd->ScanCtrl[ChBandIdx].Channel != 0) &&
						(pAd->ScanCtrl[ChBandIdx].ScanReqwdev) &&
						(pAd->ScanCtrl[ChBandIdx].ScanReqwdev->wdev_type == WDEV_TYPE_STA)) {
						msg_recv_wdev = pAd->ScanCtrl[ChBandIdx].ScanReqwdev;
					} else {
						for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
							WdevBandIdx = HcGetBandByWdev(&pAd->ApCfg.MBSSID[i].wdev);

							if (ChBandIdx == WdevBandIdx) {
								msg_recv_wdev = &pAd->ApCfg.MBSSID[i].wdev;
								if (msg_recv_wdev->DevInfo.WdevActive)
								break;
							}
						}
					}
				}

				break;
			}

			if ((ApBssIdx_find == FALSE) &&
				bToApCli) {
				if (ApCliMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
					/* apcli and repeater case */
					break;
				}
			}
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_WDS && WdsMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
				/* wds case */
				break;
			}
#endif

			if (APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case */
				if (!msg_recv_wdev->DevInfo.WdevActive) {
					struct wifi_dev *wdev_temp;

					for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
						wdev_temp = &pAd->ApCfg.MBSSID[i].wdev;
						if (msg_recv_wdev == wdev_temp)
							continue;

						if (wdev_temp->DevInfo.WdevActive) {
							msg_recv_wdev = wdev_temp;
							break;
						}
					}
				}
				break;
			}

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): un-recongnized mgmt->subtype=%d, STA-%02x:%02x:%02x:%02x:%02x:%02x\n",
					  __func__, pFrame->Hdr.FC.SubType, PRINT_MAC(pFrame->Hdr.Addr2)));
			return FALSE;
		} while (FALSE);

	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)

	if (OpMode == OPMODE_STA)
#else
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
	{
		if (!MsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
#ifdef WIDI_SUPPORT

			if (!pAd->StaCfg[0].WscControl.bWscTrigger)
#endif /* WIDI_SUPPORT */
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): un-recongnized mgmt->subtype=%d\n",
						 __func__, pFrame->Hdr.FC.SubType));

			return FALSE;
		}
	}

#endif /* CONFIG_STA_SUPPORT */
		{
			BOOLEAN isNewFsm = FALSE;

			/* AP/APCLIENT/STA */
			if (Machine == SYNC_FSM)
				isNewFsm = TRUE;

			if (sync_fsm_msg_pre_checker(pAd, pFrame, &Machine, &MsgType) == TRUE) {
				if (isNewFsm == FALSE)
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NEED CHECK IN %d\n", __func__, __LINE__));
			}
		}
	TimeStampHigh = TimeStampLow = 0;
#ifdef RTMP_MAC_PCI

	if ((!IS_USB_INF(pAd)) && !IS_HIF_TYPE(pAd, HIF_MT))
		AsicGetTsfTime(pAd, &TimeStampHigh, &TimeStampLow, HW_BSSID_0);

#endif /* RTMP_MAC_PCI */

#ifdef MLME_MULTI_QUEUE_SUPPORT 
	if (pAd->Mlme.MultiQEnable) {  
		Queue = (MLME_QUEUE *) MlmeQueueSelByMach(pAd, Machine, MsgType, pFrame);
		//MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pFrame->Hdr.FC.SubType %d\n", __func__, pFrame->Hdr.FC.SubType));
		if(pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_REQ)
		{
			pData=Msg;
			pData+=LENGTH_802_11;//LENGTH_802_11
			SSID_IES=*pData;
			pData+=1;
			SSID_lENS=*pData;
			if((SSID_IES==0)&&(SSID_lENS!=0))
			{
				UCHAR i;
				pData+=1;
				memcpy(SSIDS, pData, SSID_lENS);
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				{
					if (RTMPEqualMemory(SSIDS, pAd->ApCfg.MBSSID[i].Ssid, SSID_lENS))
					{
						Queue = (VOID *)&pAd->Mlme.HPQueue;
						break;
					}
				}
			}
		}
		if ((Queue == (MLME_QUEUE *)&pAd->Mlme.Queue && MlmeQueueFull(Queue, 0)) ||
			 (Queue != (MLME_QUEUE *)&pAd->Mlme.Queue && MlmeQueueFull(Queue, 1))) {
			RTMP_MLME_HANDLER(pAd);
			return FALSE;
		}
	}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/
	/* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == Queue->MaxLen)
		Queue->Tail = 0;

	Queue->Entry[Tail].wdev = msg_recv_wdev;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
	Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
	NdisMoveMemory(&Queue->Entry[Tail].rssi_info, rssi_info, sizeof(struct raw_rssi_info));
	Queue->Entry[Tail].Signal = rssi_info->raw_snr;
	Queue->Entry[Tail].Wcid = Wcid;
	Queue->Entry[Tail].OpMode = (ULONG)OpMode;
	Queue->Entry[Tail].Channel = (rssi_info->Channel == 0) ? pAd->LatchRfRegs.Channel : rssi_info->Channel;
	Queue->Entry[Tail].Priv = 0;
	Queue->Entry[Tail].RxPhyMode = RxPhyMode;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#endif /*CONFIG_AP_SUPPORT*/

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	NdisReleaseSpinLock(&(Queue->Lock));
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}


#ifdef WSC_INCLUDED
/*! \brief   Enqueue a message for other threads, if they want to send messages to MLME thread
 *  \param  *Queue    The MLME Queue
 *  \param   TimeStampLow    The lower 32 bit of timestamp, here we used for eventID.
 *  \param   Machine  The State Machine Id
 *  \param   MsgType  The Message Type
 *  \param   MsgLen   The Message length
 *  \param  *Msg      The message pointer
 *  \return  TRUE if enqueue is successful, FALSE if the queue is full
 *  \pre
 *  \post
 *  \note    The message has to be initialized
 */
BOOLEAN MlmeEnqueueForWsc(
	IN RTMP_ADAPTER *pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN struct wifi_dev *wdev)
{
	INT Tail;
	MLME_QUEUE	 *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----> MlmeEnqueueForWsc\n"));

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return FALSE;

	/*check wdev is not ready*/
	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev(%d) state: if down!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueueForWsc: msg too large, size = %ld\n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1)) {
		return FALSE;
	}

	/* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;

	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	Queue->Tail++;
	Queue->Num++;

	if (Queue->Tail == Queue->MaxLen)
		Queue->Tail = 0;

	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = eventID;
	Queue->Entry[Tail].TimeStamp.u.HighPart = senderID;

	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	Queue->Entry[Tail].wdev = wdev;
	NdisReleaseSpinLock(&(Queue->Lock));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- MlmeEnqueueForWsc\n"));
	return TRUE;
}
#endif /* WSC_INCLUDED */


/*! \brief	 Dequeue a message from the MLME Queue
 *	\param	*Queue	  The MLME Queue
 *	\param	*Elem	  The message dequeued from MLME Queue
 *	\return  TRUE if the Elem contains something, FALSE otherwise
 *	\pre
 *	\post
 */
BOOLEAN MlmeDequeue(MLME_QUEUE *Queue, MLME_QUEUE_ELEM **Elem)
{
	NdisAcquireSpinLock(&(Queue->Lock));
	if (Queue->Num == 0) {
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}

	*Elem = &(Queue->Entry[Queue->Head]);
	Queue->Num--;
	Queue->Head++;

	if (Queue->Head == Queue->MaxLen)
		Queue->Head = 0;

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


VOID MlmeRestartStateMachine(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM *Elem = NULL;
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN Cancelled;
#ifdef P2P_SUPPORT
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeRestartStateMachine\n"));
#ifdef RTMP_MAC_PCI
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);

	if (pAd->Mlme.bRunning) {
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} else
		pAd->Mlme.bRunning = TRUE;

	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	/* Remove all Mlme queues elements*/
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) {
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) {
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeRestartStateMachine: MlmeQueue empty\n"));
	}
#ifdef MLME_MULTI_QUEUE_SUPPORT
	while (!MlmeQueueEmpty((MLME_QUEUE *)&pAd->Mlme.HPQueue)) {
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue((MLME_QUEUE *)&pAd->Mlme.HPQueue, &Elem)) {
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeRestartStateMachine: MlmeHPQueue empty\n"));
	}
	while (!MlmeQueueEmpty((MLME_QUEUE *)&pAd->Mlme.LPQueue)) {
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue((MLME_QUEUE *)&pAd->Mlme.LPQueue, &Elem)) {
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;
		} else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeRestartStateMachine: MlmeLPQueue empty\n"));
	}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Cancel all timer events*/
		/* Be careful to cancel new added timer*/
		RTMPCancelTimer(&pStaCfg->MlmeAux.AssocTimer,	  &Cancelled);
		RTMPCancelTimer(&pStaCfg->MlmeAux.ReassocTimer,   &Cancelled);
		RTMPCancelTimer(&pStaCfg->MlmeAux.DisassocTimer,  &Cancelled);
		RTMPCancelTimer(&pStaCfg->MlmeAux.AuthTimer,	   &Cancelled);
		RTMPCancelTimer(&pStaCfg->MlmeAux.JoinTimer,	   &Cancelled);
	}
#endif /* CONFIG_STA_SUPPORT */
	/* Change back to original channel in case of doing scan*/
#ifdef P2P_SUPPORT
	pApCliEntry = &pAd->StaCfg[BSS0];

	if (!P2P_GO_ON(pAd) && (pApCliEntry->ApcliInfStat.Valid == FALSE))
#endif /* P2P_SUPPORT */
	{
		hc_reset_radio(pAd);
	}

	cntl_fsm_reset(wdev);
	auth_fsm_reset(wdev);
	assoc_fsm_reset(wdev);
	sync_fsm_reset(pAd, wdev);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Set all state machines back IDLE*/
		pAd->Mlme.ActMachine.CurrState    = ACT_IDLE;
#ifdef DOT11Z_TDLS_SUPPORT
		pAd->Mlme.TdlsMachine.CurrState    = TDLS_IDLE;
#endif /* DOT11Z_TDLS_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef RTMP_MAC_PCI
	/* Remove running state*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif /* RTMP_MAC_PCI */
	/* CFG_TODO for SCAN */
#ifdef RT_CFG80211_SUPPORT
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* RT_CFG80211_SUPPORT */
}


/*! \brief	test if the MLME Queue is empty
 *	\param	*Queue	  The MLME Queue
 *	\return TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueEmpty(MLME_QUEUE *Queue)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));
	return Ans;
}

/*! \brief	 test if the MLME Queue is full
 *	\param	 *Queue	 The MLME Queue
 *	\return  TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueFull(MLME_QUEUE *Queue, UCHAR SendId)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));

	if (SendId == 0)
		Ans = ((Queue->Num >= (Queue->MaxLen / 2)) || Queue->Entry[Queue->Tail].Occupied);
	else
		Ans = ((Queue->Num == Queue->MaxLen) || Queue->Entry[Queue->Tail].Occupied);

	NdisReleaseSpinLock(&(Queue->Lock));
	return Ans;
}


/*! \brief	 The destructor of MLME Queue
 *	\param
 *	\return
 *	\pre
 *	\post
 *	\note	Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL

 */
VOID MlmeQueueDestroy(struct _MLME_STRUCT *mlme)
{
	UCHAR idx;
	MLME_QUEUE *pQueue;
	for (idx = 0; idx < MAX_NUM_OF_MLME_QUEUE; idx++) {
		switch (idx) {
			case 0:
				pQueue = &mlme->Queue;
				break;
#ifdef MLME_MULTI_QUEUE_SUPPORT
			case 1:
				pQueue = (MLME_QUEUE *) &mlme->HPQueue;
				break;
			case 2:
				pQueue = (MLME_QUEUE *) &mlme->LPQueue;
				break;
#endif
			default:
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No mlme queue matched!, idx = %d\n", __func__, idx));
				break;
		}
		NdisAcquireSpinLock(&(pQueue->Lock));
		pQueue->Num  = 0;
		pQueue->Head = 0;
		pQueue->Tail = 0;
		pQueue->MaxLen = 0;
		NdisReleaseSpinLock(&(pQueue->Lock));
		NdisFreeSpinLock(&(pQueue->Lock));
	}
}

#ifdef MLME_MULTI_QUEUE_SUPPORT
VOID*  MlmeQueueSelByMach(
       IN RTMP_ADAPTER *pAd,
       IN ULONG Machine,
       IN ULONG MsgType,
       IN VOID *pHeader)
{
	VOID *Queue = (VOID *)&pAd->Mlme.Queue;
	PFRAME_802_11 pH = (PFRAME_802_11)pHeader;
	switch (Machine) {
		case ASSOC_FSM:
		case AUTH_FSM:
		case WPA_STATE_MACHINE:
			Queue = (VOID *)&pAd->Mlme.HPQueue;
			break;
		case SYNC_FSM:
			if (MsgType == SYNC_FSM_PEER_BEACON) {
				//Queue = (VOID *)&pAd->Mlme.LPQueue;
				if(pAd->ScanCurrState==2)
				{
					//MTWF_LOG(DBG_CAT_MLME,DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pAd->ScanCurrState=%d\n", __func__,pAd->ScanCurrState));
					Queue = (VOID *)&pAd->Mlme.HPQueue;
				}else
					Queue = (VOID *)&pAd->Mlme.LPQueue;
			} else if (MsgType == SYNC_FSM_PEER_PROBE_REQ) {
				if (pH && MAC_ADDR_IS_GROUP(pH->Hdr.Addr1))  {
					Queue = (VOID *)&pAd->Mlme.LPQueue;
				}
			}
			else if (MsgType == SYNC_FSM_PEER_PROBE_RSP) {
				Queue = (VOID *)&pAd->Mlme.HPQueue;
			}
			break;
		default:
			break;
	}
	return Queue;
}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/


/*! \brief	 To substitute the message type if the message is coming from external
 *	\param	pFrame		   The frame received
 *	\param	*Machine	   The state machine
 *	\param	*MsgType	   the message type for the state machine
 *	\return TRUE if the substitution is successful, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
#ifdef CONFIG_STA_SUPPORT
BOOLEAN MsgTypeSubst(RTMP_ADAPTER *pAd, FRAME_802_11 *pFrame, INT *Machine, INT *MsgType)
{
	USHORT	Seq, Alg;
	UCHAR	EAPType;
	PUCHAR	pData;
	BOOLEAN bRV = FALSE;
#ifdef WSC_STA_SUPPORT
	UCHAR EAPCode;
#endif /* WSC_STA_SUPPORT */
	/* Pointer to start of data frames including SNAP header*/
	pData = (PUCHAR) pFrame + LENGTH_802_11;

	/* The only data type will pass to this function is EAPOL frame*/
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef DOT11Z_TDLS_SUPPORT

		if (NdisEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2)) {
			UCHAR	TDLSType;
			/* ieee802.11z TDLS SNAP header*/
			*Machine = TDLS_STATE_MACHINE;
			TDLSType = *((UCHAR *)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 2);
			return TDLS_MsgTypeSubst(TDLSType, (INT *)MsgType);
		} else
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef WSC_STA_SUPPORT

			/* check for WSC state machine first*/
			if (pAd->StaCfg[0].wdev.WscControl.WscState >= WSC_STATE_LINK_UP) {
				*Machine = WSC_STATE_MACHINE;
				EAPType = *((UCHAR *)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
				EAPCode = *((UCHAR *)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
				bRV = WscMsgTypeSubst(EAPType, EAPCode, MsgType);

				if (bRV)
					return bRV;
			}

#endif /* WSC_STA_SUPPORT */

		if (bRV == FALSE) {
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
			return WpaMsgTypeSubst(EAPType, (INT *) MsgType);
		}
	}

	switch (pFrame->Hdr.FC.SubType) {
	case SUBTYPE_ASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_ASSOC_REQ;
		break;

	case SUBTYPE_ASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_ASSOC_RSP;
		break;

	case SUBTYPE_REASSOC_REQ:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_REASSOC_REQ;
		break;

	case SUBTYPE_REASSOC_RSP:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_REASSOC_RSP;
		break;

	case SUBTYPE_PROBE_REQ:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_REQ;
		break;

	case SUBTYPE_PROBE_RSP:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_PROBE_RSP;
		break;

	case SUBTYPE_BEACON:
		*Machine = SYNC_FSM;
		*MsgType = SYNC_FSM_PEER_BEACON;
		break;
	case SUBTYPE_DISASSOC:
		*Machine = ASSOC_FSM;
		*MsgType = ASSOC_FSM_PEER_DISASSOC_REQ;
		break;

	case SUBTYPE_AUTH:
		/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm*/
		NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
		NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#ifdef DOT11_SAE_SUPPORT

		if (Alg == AUTH_MODE_SAE) {
			*Machine = AUTH_FSM;
			*MsgType = AUTH_FSM_SAE_AUTH_RSP;
		} else
#endif /* DOT11_SAE_SUPPORT */
			if (Seq == 1 || Seq == 3) {
				*Machine = AUTH_FSM;
				*MsgType = AUTH_FSM_PEER_AUTH_ODD;
			} else if (Seq == 2 || Seq == 4) {
#ifdef DOT11R_FT_SUPPORT

				if (Alg == AUTH_MODE_FT) {
					*Machine = FT_OTA_AUTH_STATE_MACHINE;
					*MsgType = FT_OTA_MT2_PEER_AUTH_EVEN;
				} else
#endif /* DOT11R_FT_SUPPORT */
					if (Alg == AUTH_MODE_OPEN || Alg == AUTH_MODE_KEY) {
						*Machine = AUTH_FSM;
						*MsgType = AUTH_FSM_PEER_AUTH_EVEN;
					}
			} else
				return FALSE;

		break;

	case SUBTYPE_DEAUTH:
		*Machine = AUTH_FSM;
		*MsgType = AUTH_FSM_PEER_DEAUTH;
		break;

	case SUBTYPE_ACTION:
	case SUBTYPE_ACTION_NO_ACK:
		*Machine = ACTION_STATE_MACHINE;
#ifdef DOT11R_FT_SUPPORT

		if ((pFrame->Octet[0] & 0x7F) == FT_CATEGORY_BSS_TRANSITION) {
			*Machine = FT_OTD_ACT_STATE_MACHINE;
			*MsgType = FT_OTD_MT2_PEER_EVEN;
		} else
#endif /* DOT11R_FT_SUPPORT */

			/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support*/
			if ((pFrame->Octet[0] & 0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else
				*MsgType = (pFrame->Octet[0] & 0x7F);

		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */


/*! \brief Initialize the state machine.
 *	\param *S			pointer to the state machine
 *	\param	Trans		State machine transition function
 *	\param	StNr		number of states
 *	\param	MsgNr		number of messages
 *	\param	DefFunc	default function, when there is invalid state/message combination
 *	\param	InitState	initial state of the state machine
 *	\param	Base		StateMachine base, internal use only
 *	\pre p_sm should be a legal pointer
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineInit(
	IN STATE_MACHINE * S,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base)
{
	ULONG i, j;
	/* set number of states and messages*/
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;
	S->TransFunc  = Trans;

	/* init all state transition to default function*/
	for (i = 0; i < StNr; i++) {
		for (j = 0; j < MsgNr; j++)
			S->TransFunc[i * MsgNr + j] = DefFunc;
	}

	/* set the starting state*/
	S->CurrState = InitState;
}


/*! \brief This function fills in the function pointer into the cell in the state machine
 *	\param *S	pointer to the state machine
 *	\param St	state
 *	\param Msg	incoming message
 *	\param f	the function to be executed when (state, message) combination occurs at the state machine
 *	\pre *S should be a legal pointer to the state machine, st, msg, should be all within the range, Base should be set in the initial state
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineSetAction(
	IN STATE_MACHINE * S,
	IN ULONG St,
	IN ULONG Msg,
	IN STATE_MACHINE_FUNC Func)
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg) {
		/* boundary checking before setting the action*/
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	}
}

VOID StateMachineSetMsgChecker(STATE_MACHINE *S, STATE_MACHINE_MSG_CHECKER MsgCheckFun)
{
		S->MsgChecker = MsgCheckFun;
}

/*! \brief	 This function does the state transition
 *	\param	 *Adapter the NIC adapter pointer
 *	\param	 *S	  the state machine
 *	\param	 *Elem	  the message to be executed
 *	\return   None
 */
VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem,
		IN ULONG CurrState) {
		if ((S->MsgChecker != NULL) &&
			(S->MsgChecker(pAd, Elem) == TRUE))
			return;

	if (S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base])
		(*(S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}


/*
	==========================================================================
	Description:
		The drop function, when machine executes this, the message is simply
		ignored. This function does nothing, the message is freed in
		StateMachinePerformAction()
	==========================================================================
 */
VOID Drop(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}


/*
	==========================================================================
	Description:
	==========================================================================
 */
UCHAR RandomByte(RTMP_ADAPTER *pAd)
{
	ULONG i;
	UCHAR R, Result;

	R = 0;

	if (pAd->Mlme.ShiftReg == 0)
		NdisGetSystemUpTime((ULONG *)&pAd->Mlme.ShiftReg);

	for (i = 0; i < 8; i++) {
		if (pAd->Mlme.ShiftReg & 0x00000001) {
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		} else {
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}

		R = (R << 1) | Result;
	}

	return R;
}


UCHAR RandomByte2(RTMP_ADAPTER *pAd)
{
	UINT32 a, b;
	UCHAR value, seed = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
		return 0;
	}

	/*MAC statistic related*/
	a = AsicGetCCACnt(pAd, 0);
	a &= 0x0000ffff;
	b = AsicGetCrcErrCnt(pAd);
	b &= 0x0000ffff;
	value = (a << 16) | b;
	/*get seed by RSSI or SNR related info */
	seed = get_random_seed_by_phy(pAd);
	return value ^ seed ^ RandomByte(pAd);
}

void check_legacy_rates(
	struct legacy_rate *rate, struct legacy_rate *mlme_rate, struct wifi_dev *wdev)
{
	mlme_rate->sup_rate_len = rate->sup_rate_len;
	NdisMoveMemory(mlme_rate->sup_rate, rate->sup_rate, rate->sup_rate_len);
	RTMPCheckRates(mlme_rate->sup_rate, &mlme_rate->sup_rate_len, wdev->PhyMode);
	mlme_rate->ext_rate_len = rate->ext_rate_len;
	NdisMoveMemory(mlme_rate->ext_rate, rate->ext_rate, rate->ext_rate_len);
	RTMPCheckRates(mlme_rate->ext_rate, &mlme_rate->ext_rate_len, wdev->PhyMode);
}

/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPCheckRates(UCHAR SupRate[], UCHAR *SupRateLen, UCHAR PhyMode)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12], NewRateLen;

	NdisZeroMemory(NewRate, sizeof(NewRate));
	NewRateLen = 0;

	if (WMODE_EQUAL(PhyMode, WMODE_B))
		RateIdx = 4;
	else
		RateIdx = 12;

	/* Check for support rates exclude basic rate bit */
	for (i = 0; i < *SupRateLen; i++)
		for (j = 0; j < RateIdx; j++)
			if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
				NewRate[NewRateLen++] = SupRate[i];

	*SupRateLen = NewRateLen;
	NdisMoveMemory(SupRate, NewRate, NewRateLen);
}

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
BOOLEAN RTMPCheckChannel(RTMP_ADAPTER *pAd, UCHAR CentralCh, UCHAR ch, struct wifi_dev *wdev)
{
	UCHAR		k;
	UCHAR		UpperChannel = 0, LowerChannel = 0;
	UCHAR		NoEffectChannelinList = 0;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	/* Find upper and lower channel according to 40MHz current operation. */
	if (CentralCh < ch) {
		UpperChannel = ch;

		if (CentralCh > 2)
			LowerChannel = CentralCh - 2;
		else
			return FALSE;
	} else if (CentralCh > ch) {
		UpperChannel = CentralCh + 2;
		LowerChannel = ch;
	}

	for (k = 0; k < pChCtrl->ChListNum; k++) {
		if (pChCtrl->ChList[k].Channel == UpperChannel) {
			NoEffectChannelinList++;
		}

		if (pChCtrl->ChList[k].Channel == LowerChannel) {
			NoEffectChannelinList++;
		}
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Total Channel in Channel List = [%d]\n",
			 NoEffectChannelinList));

	if (NoEffectChannelinList == 2)
		return TRUE;
	else
		return FALSE;
}
#endif /* DOT11_N_SUPPORT */


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPUpdateMlmeRate(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR MinimumRate;
	UCHAR ProperMlmeRate; /*= RATE_54; */
	UCHAR i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	struct legacy_rate *rate = &pStaCfg->MlmeAux.rate;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	switch (wdev->PhyMode) {
	case (WMODE_B):
		ProperMlmeRate = RATE_11;
		MinimumRate = RATE_1;
		break;

	case ((enum WIFI_MODE)(WMODE_B | WMODE_G)):
#ifdef DOT11_N_SUPPORT
	case ((enum WIFI_MODE)(WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN)):
	case ((enum WIFI_MODE)(WMODE_B | WMODE_G | WMODE_GN)):
#ifdef DOT11_VHT_AC
	case ((enum WIFI_MODE)(WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC)):
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		if ((rate->sup_rate_len == 4) &&
			(rate->ext_rate_len == 0))
			ProperMlmeRate = RATE_11; /* B only AP */
		else
			ProperMlmeRate = RATE_24;

		if (pStaCfg->MlmeAux.Channel <= 14)
			MinimumRate = RATE_1;
		else
			MinimumRate = RATE_6;

		break;

	case (WMODE_A):
#ifdef DOT11_N_SUPPORT
	case (WMODE_GN):
	case ((enum WIFI_MODE)(WMODE_G | WMODE_GN)):
	case ((enum WIFI_MODE)(WMODE_A | WMODE_G | WMODE_GN | WMODE_AN)):
	case ((enum WIFI_MODE)(WMODE_A | WMODE_AN)):
	case (WMODE_AN):
#ifdef DOT11_VHT_AC
	case ((enum WIFI_MODE)(WMODE_A | WMODE_G | WMODE_GN | WMODE_AN | WMODE_AC)):
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		ProperMlmeRate = RATE_24;
		MinimumRate = RATE_6;
		break;

	case ((enum WIFI_MODE)(WMODE_A | WMODE_B | WMODE_G)):
		ProperMlmeRate = RATE_24;

		if (pStaCfg->MlmeAux.Channel <= 14)
			MinimumRate = RATE_1;
		else
			MinimumRate = RATE_6;

		break;

	default:
		ProperMlmeRate = RATE_1;
		MinimumRate = RATE_1;
		break;
	}

#ifdef DOT11_VHT_AC

	if (WMODE_EQUAL(wdev->PhyMode, WMODE_B)) {
		ProperMlmeRate = RATE_11;
		MinimumRate = RATE_1;
	} else {
		if (WMODE_CAP(wdev->PhyMode, WMODE_B)) {
			if ((rate->sup_rate_len == 4) && (rate->ext_rate_len == 0))
				ProperMlmeRate = RATE_11; /* B only AP */
			else
				ProperMlmeRate = RATE_24;

			if (pStaCfg->MlmeAux.Channel <= 14)
				MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
		} else {
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
		}
	}

#endif /* DOT11_VHT_AC */

	for (i = 0; i < rate->sup_rate_len; i++) {
		for (j = 0; j < RateIdx; j++) {
			if ((rate->sup_rate[i] & 0x7f) == RateIdTo500Kbps[j]) {
				if (j == ProperMlmeRate) {
					bMatch = TRUE;
					break;
				}
			}
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE) {
		for (i = 0; i < rate->ext_rate_len; i++) {
			for (j = 0; j < RateIdx; j++) {
				if ((rate->ext_rate[i] & 0x7f) == RateIdTo500Kbps[j]) {
					if (j == ProperMlmeRate) {
						bMatch = TRUE;
						break;
					}
				}
			}

			if (bMatch)
				break;
		}
	}

	if (bMatch == FALSE)
		ProperMlmeRate = MinimumRate;

	pAd->CommonCfg.MlmeRate = MinimumRate;
	pAd->CommonCfg.RtsRate = ProperMlmeRate;

	if (pAd->CommonCfg.MlmeRate >= RATE_6) {
		wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
		wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
	} else {
		wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
		wdev->rate.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=>MlmeTransmit=0x%x\n",
			 __func__, wdev->rate.MlmeTransmit.word));
}
#endif /* CONFIG_STA_SUPPORT */


CHAR RTMPAvgRssi(RTMP_ADAPTER *pAd, RSSI_SAMPLE *pRssi)
{
	CHAR Rssi;
	INT Rssi_temp;
	UINT32	rx_stream = 0;
	UINT8 i;

	for (i = 0; i < pAd->Antenna.field.RxPath; i++) {
		if ((pRssi->AvgRssi[i] > -127) && (pRssi->AvgRssi[i] < 0))
			rx_stream++;
	}

	if (rx_stream == 4)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2] + pRssi->AvgRssi[3]) >> 2;
	else if (rx_stream == 3)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2]) / 3;
	else if (rx_stream == 2)
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1]) >> 1;
	else
		Rssi_temp = pRssi->AvgRssi[0];

	Rssi = (CHAR)Rssi_temp;
	return Rssi;
}


CHAR rtmp_avg_rssi(RTMP_ADAPTER *pad, RSSI_SAMPLE *prssi)
{
	CHAR rssi = 0;
	INT rssi_temp = 0;
	UINT32	rx_stream = 0;
	UINT8 i;

	for (i = 0; i < pad->Antenna.field.RxPath; i++) {
		if ((prssi->AvgRssi[i] > -127) && (prssi->AvgRssi[i] < 0))
			rx_stream++;
	}

	if (rx_stream == 4)
		rssi_temp = (prssi->AvgRssi[0] + prssi->AvgRssi[1] + prssi->AvgRssi[2] + prssi->AvgRssi[3]) >> 2;
	else if (rx_stream == 3)
		rssi_temp = (prssi->AvgRssi[0] + prssi->AvgRssi[1] + prssi->AvgRssi[2]) / 3;
	else if (rx_stream == 2)
		rssi_temp = (prssi->AvgRssi[0] + prssi->AvgRssi[1]) >> 1;
	else
		rssi_temp = prssi->AvgRssi[0];

	rssi = (CHAR)rssi_temp;
	return rssi;
}



CHAR RTMPMaxRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2)
{
	CHAR larger = -127;

	if ((pAd->Antenna.field.RxPath >= 1) && (Rssi0 > -127) && (Rssi0 < 0))
		larger = Rssi0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 > -127) && (Rssi1 < 0))
		larger = max(larger, Rssi1);

	if ((pAd->Antenna.field.RxPath >= 3) && (Rssi2 > -127) && (Rssi2 < 0))
		larger = max(larger, Rssi2);

	if (larger == -127)
		larger = 0;

	return larger;
}

CHAR RTMPMinRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2, CHAR Rssi3)
{
	CHAR smaller = -127;

	if ((pAd->Antenna.field.RxPath >= 1) && (Rssi0 > -127) && (Rssi0 < 0))
		smaller = Rssi0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 > -127) && (Rssi1 < 0))
		smaller = min(smaller, Rssi1);

	if ((pAd->Antenna.field.RxPath >= 3) && (Rssi2 > -127) && (Rssi2 < 0))
		smaller = min(smaller, Rssi2);

	if ((pAd->Antenna.field.RxPath >= 4) && (Rssi3 > -127) && (Rssi3 < 0))
		smaller = min(smaller, Rssi3);

	if (smaller == -127)
		smaller = 0;

	return smaller;
}

CHAR RTMPMinSnr(RTMP_ADAPTER *pAd, CHAR Snr0, CHAR Snr1)
{
	CHAR smaller = Snr0;

	if (pAd->Antenna.field.RxPath >= 1)
		smaller = Snr0;

	if ((pAd->Antenna.field.RxPath >= 2) && (Snr1 != 0))
		smaller = min(smaller, Snr1);

	return smaller;
}


/*
    ========================================================================
    Routine Description:
	Periodic evaluate antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID AsicEvaluateRxAnt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_STA_SUPPORT
	UINT8 i = 0;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
					   fRTMP_ADAPTER_NIC_NOT_EXIST |
					   fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
		|| IsHcAllSupportedBandsRadioOff(pAd)
	   )
		return;

#ifdef CONFIG_STA_SUPPORT
	for (i = 0; i < pAd->MSTANum; i++)
		if ((pAd->StaCfg[i].wdev.DevInfo.WdevActive)
			&& (pAd->StaCfg[i].PwrMgmt.bDoze))
			return;
#endif /* CONFIG_STA_SUPPORT */

#ifdef MT_MAC

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT))
		return;

#endif /* MT_MAC */
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			/* for SmartBit 64-byte stream test */
			if (pAd->MacTab.Size > 0)
				APAsicEvaluateRxAnt(pAd);

			return;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {

			if (pAd->StaCfg[0].PwrMgmt.Psm == PWR_SAVE)
				return;

			bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

			if (STA_STATUS_TEST_FLAG(&pAd->StaCfg[0], fSTA_STATUS_MEDIA_STATE_CONNECTED)
#ifdef P2P_SUPPORT
				|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
			   ) {
				ULONG TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
								   pAd->RalinkCounters.OneSecTxRetryOkCount +
								   pAd->RalinkCounters.OneSecTxFailCount;

				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50) {
					RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
					pAd->Mlme.bLowThroughput = FALSE;
				} else {
					RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
					pAd->Mlme.bLowThroughput = TRUE;
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}
}


/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER	 *pAd = (RTMP_ADAPTER *)FunctionContext;
#ifdef CONFIG_STA_SUPPORT
	CHAR larger = -127, rssi0, rssi1, rssi2;
	UINT8 i = 0 ;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
					   fRTMP_ADAPTER_NIC_NOT_EXIST)
		|| IsHcAllSupportedBandsRadioOff(pAd)
	   )
		return;

#ifdef CONFIG_STA_SUPPORT
	for (i = 0; i < pAd->MSTANum; i++)
		if ((pAd->StaCfg[i].wdev.DevInfo.WdevActive)
			&& (pAd->StaCfg[i].PwrMgmt.bDoze))
			return;
#endif /* CONFIG_STA_SUPPORT */

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			APAsicRxAntEvalTimeout(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (pAd->StaCfg[0].PwrMgmt.Psm == PWR_SAVE)
				return;

			/* if the traffic is low, use average rssi as the criteria*/
			if (pAd->Mlme.bLowThroughput == TRUE) {
				rssi0 = pAd->StaCfg[0].RssiSample.LastRssi[0];
				rssi1 = pAd->StaCfg[0].RssiSample.LastRssi[1];
				rssi2 = pAd->StaCfg[0].RssiSample.LastRssi[2];
			} else {
				rssi0 = pAd->StaCfg[0].RssiSample.AvgRssi[0];
				rssi1 = pAd->StaCfg[0].RssiSample.AvgRssi[1];
				rssi2 = pAd->StaCfg[0].RssiSample.AvgRssi[2];
			}

			if (pAd->Antenna.field.RxPath == 3) {
				larger = max(rssi0, rssi1);

				if (larger > (rssi2 + 20))
					pAd->Mlme.RealRxPath = 2;
				else
					pAd->Mlme.RealRxPath = 3;
			} else if (pAd->Antenna.field.RxPath == 2) {
				if (rssi0 > (rssi1 + 20))
					pAd->Mlme.RealRxPath = 1;
				else
					pAd->Mlme.RealRxPath = 2;
			}

			bbp_set_rxpath(pAd, pAd->Mlme.RealRxPath);
		}
#endif /* CONFIG_STA_SUPPORT */
	}
}


VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
}


/*
    ========================================================================
    Routine Description:
	check if this entry need to switch rate automatically

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN result = TRUE;

	if ((!pEntry) || (!(pEntry->wdev))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): entry(%p) or wdev(%p) is NULL!\n",
				 __func__, pEntry,
				 pEntry ? pEntry->wdev : NULL));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		result = pEntry->wdev->bAutoTxRateSwitch;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* only associated STA counts*/
#ifdef P2P_SUPPORT
		if (pEntry && IS_P2P_GO_ENTRY(pEntry))
			result = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bAutoTxRateSwitch;
		else if (pEntry && IS_ENTRY_PEER_AP(pEntry))
			result = pAd->StaCfg[pEntry->func_tb_idx].wdev.bAutoTxRateSwitch;
		else
#endif /* P2P_SUPPORT */
			if ((pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				|| (pEntry && IS_ENTRY_TDLS(pEntry))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			   )
				result = pAd->StaCfg[0].wdev.bAutoTxRateSwitch;
			else
				result = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */
	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT	apidx = 0;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if (pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#ifdef WDS_SUPPORT

		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++) {
			if (pAd->WdsTab.WdsEntry[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT

		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++) {
			if (pAd->StaCfg[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pAd->StaCfg[0].wdev.bAutoTxRateSwitch)
			return TRUE;
	}
#endif /* CONFIG_STA_SUPPORT */
	return FALSE;
}


/*
    ========================================================================
    Routine Description:
	check if this entry need to fix tx legacy rate

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
UCHAR RTMPStaFixedTxMode(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UCHAR tx_mode = FIXED_TXMODE_HT;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)

	if (pEntry) {
		if (IS_ENTRY_CLIENT(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;

#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			tx_mode = (UCHAR)pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			tx_mode = (UCHAR)pEntry->wdev->DesiredTransmitSetting.field.FixedTxMode;

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef P2P_SUPPORT

		if (IS_P2P_GO_ENTRY(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
		else if (IS_ENTRY_PEER_AP(pEntry))
			tx_mode = (UCHAR)pEntry->wdev->DesiredTransmitSetting.field.FixedTxMode;
		else
#endif /* P2P_SUPPORT */
			tx_mode = (UCHAR)pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode;
	}
#endif /* CONFIG_STA_SUPPORT */
	return tx_mode;
}


/*
    ========================================================================
    Routine Description:
	Overwrite HT Tx Mode by Fixed Legency Tx Mode, if specified.

    Arguments:
	pAd
	pEntry

    Return Value:
	TURE
	FALSE

    ========================================================================
*/
VOID RTMPUpdateLegacyTxSetting(UCHAR fixed_tx_mode, MAC_TABLE_ENTRY *pEntry)
{
	HTTRANSMIT_SETTING TransmitSetting;

	if ((fixed_tx_mode != FIXED_TXMODE_CCK) &&
		(fixed_tx_mode != FIXED_TXMODE_OFDM)
#ifdef DOT11_VHT_AC
		&& (fixed_tx_mode != FIXED_TXMODE_VHT)
#endif /* DOT11_VHT_AC */
	   )
		return;

	TransmitSetting.word = 0;
	TransmitSetting.field.MODE = pEntry->HTPhyMode.field.MODE;
	TransmitSetting.field.MCS = pEntry->HTPhyMode.field.MCS;
#ifdef DOT11_VHT_AC

	if (fixed_tx_mode == FIXED_TXMODE_VHT) {
		UCHAR nss, mcs;

		TransmitSetting.field.MODE = MODE_VHT;
		TransmitSetting.field.BW = pEntry->MaxHTPhyMode.field.BW;
		nss = (TransmitSetting.field.MCS >> 4) & 0x3;
		mcs = (TransmitSetting.field.MCS & 0xf);

		if ((TransmitSetting.field.BW == BW_20) && (mcs > MCS_8))
			mcs = MCS_8;

		if ((TransmitSetting.field.BW == BW_40) && (mcs > MCS_9))
			mcs = MCS_9;

		if (TransmitSetting.field.BW == BW_80) {
			if (mcs > MCS_9)
				mcs = MCS_9;

			if ((nss == 2) && (mcs == MCS_6))
				mcs = MCS_5;
		}

		TransmitSetting.field.MCS = ((nss << 4) + mcs);
	} else
#endif /* DOT11_VHT_AC */
		if (fixed_tx_mode == FIXED_TXMODE_CCK) {
			TransmitSetting.field.MODE = MODE_CCK;

			/* CCK mode allow MCS 0~3*/
			if (TransmitSetting.field.MCS > MCS_3)
				TransmitSetting.field.MCS = MCS_3;
		} else {
			TransmitSetting.field.MODE = MODE_OFDM;

			/* OFDM mode allow MCS 0~7*/
			if (TransmitSetting.field.MCS > MCS_7)
				TransmitSetting.field.MCS = MCS_7;
		}

	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE) {
		pEntry->HTPhyMode.word = TransmitSetting.word;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d\n",
				 pEntry->wcid, get_phymode_str(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the fixed TxMode is invalid\n", __func__));
}

#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		dynamic tune BBP R66 to find a balance between sensibility and
		noise isolation

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicStaBbpTuning(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT))
		return;

}
#endif /* CONFIG_STA_SUPPORT */

/*
========================================================================
Routine Description:
	Check if the channel has the property.

Arguments:
	pAd				- WLAN control block pointer
	ChanNum			- channel number
	Property		- channel property, CHANNEL_PASSIVE_SCAN, etc.

Return Value:
	TRUE			- YES
	FALSE			- NO

Note:
========================================================================
*/
BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property)
{
	UINT32 IdChan;
	UCHAR BandIdx = HcGetBandByChannel(pAd, ChanNum);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	/* look for all registered channels */
	for (IdChan = 0; IdChan < pChCtrl->ChListNum; IdChan++) {
		if (pChCtrl->ChList[IdChan].Channel == ChanNum) {
			if ((pChCtrl->ChList[IdChan].Flags & Property) == Property)
				return TRUE;

			break;
		}
	}

	return FALSE;
}

