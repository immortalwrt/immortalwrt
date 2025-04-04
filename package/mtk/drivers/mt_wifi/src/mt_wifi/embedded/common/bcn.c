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
****************************************************************************

   Module Name:
   bcn.c

   Abstract:
   separate Bcn related function

   Revision History:
   Who         When          What
   --------    ----------    ----------------------------------------------
   Carter      2014-1121     created for all interface could send bcn.

*/

#include "rt_config.h"

#define MAX_TRANSMIT_POWER 30

UCHAR PowerConstraintIE[3] = {IE_POWER_CONSTRAINT, 1, 0};

/*
    ==========================================================================
    Description:
	Used to check the necessary to send Beancon.
    return value
	0: mean no necessary.
	1: mean need to send Beacon for the service.
    ==========================================================================
*/
BOOLEAN BeaconTransmitRequired(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN UpdateRoutine)
{
	BOOLEAN result = FALSE;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_ATE
	if (!WDEV_WITH_BCN_ABILITY(wdev) || ATE_ON(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
			"Bcn Tx is blocked, wdev_type=%d, ATE_ON=%d\n",
			wdev->wdev_type, ATE_ON(pAd));
		return result;
	}
#endif /*CONFIG_ATE*/
	if (bcn_info == NULL)
		return result;

	if (bcn_info->BeaconPkt == NULL) {
		MTWF_LOG(
			DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			("%s(): no BeaconPkt\n",
			 __func__));
		return result;
	}

	if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW) {
		/*
		    Beacon is FW offload,
		    we will not send template to FW in updateRoutine,
		    and there shall not be updateRoutine happened in HOST.
		*/
		if ((UpdateRoutine == TRUE) && (cap->fgIsNeedPretbttIntEvent == FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "Wrong BCN update method (%d)\n", bcn_info->BcnUpdateMethod);
			return result;
		}
	} else {
		if (UpdateRoutine == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "Wrong BCN update method (%d)\n", bcn_info->BcnUpdateMethod);
			return result;
		}
	}

	do {
#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
		/*
		 * WDS is bound on main wifi dev which should not issue Beacons
		 * when system operates as bridge mode
		 */
		if (pAd->WdsTab.Mode[HcGetBandByWdev(wdev)] == WDS_BRIDGE_MODE)
			break;

#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT

		if (isCarrierDetectExist(pAd) == TRUE)
			break;

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT

		if (IS_RRM_QUIET(wdev))
			break;

#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */
#endif /*CONFIG_AP_SUPPORT */
#ifndef BCN_OFFLOAD_SUPPORT

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			RTMP_SEM_LOCK(&pAd->BcnRingLock);

			if (wdev->bcn_buf.bcn_state != BCN_TX_IDLE) {
				if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
							 "wdev->OmacIdx = %x, != BCN_TX_IDLE\n", wdev->OmacIdx);
				}

				RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
				result = FALSE;
				break;
			}

			RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
		}

#endif

		if (bcn_info->bBcnSntReq == TRUE) {
			result = TRUE;
			break;
		}
	} while (FALSE);

	return result;
}

INT bcn_buf_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	UINT8 max_v2_bcn_num = cap->max_v2_bcn_num;
#endif

	bcn_info->cap_ie_pos = 0;
	bcn_info->pWdev = wdev;
	NdisAllocateSpinLock(pAd, &bcn_info->BcnContentLock);

	if (!bcn_info->BeaconPkt) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
		if (wdev->func_idx < max_v2_bcn_num)
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACONV2_LENGTH);
		else
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACON_LENGTH);
#else
		Status = RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACON_LENGTH);
#endif

		if (Status == NDIS_STATUS_FAILURE)
			return Status;
	} else
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
		"BcnPkt is allocated!, bcn offload=%d\n", cap->fgBcnOffloadSupport);

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		bcn_info->bcn_state = BCN_TX_IDLE;
#ifdef BCN_OFFLOAD_SUPPORT

		if (cap->fgBcnOffloadSupport == TRUE)
			bcn_info->BcnUpdateMethod = BCN_GEN_BY_FW;
		else
#endif /* BCN_OFFLOAD_SUPPORT */
		{
			bcn_info->BcnUpdateMethod = BCN_GEN_BY_HOST_IN_PRETBTT;
		}
	}

#endif /* MT_MAC */
	return Status;
}

INT bcn_buf_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BCN_BUF_STRUCT *bcn_info = &wdev->bcn_buf;

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {

		if (bcn_info->bcn_state != BCN_TX_IDLE) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "Bcn not in idle(%d) when try to free it!\n",
					 bcn_info->bcn_state);
		}

		bcn_info->bcn_state = BCN_TX_UNINIT;
	}

#endif /* MT_MAC */

	if (bcn_info->BeaconPkt) {
		RTMP_SEM_LOCK(&bcn_info->BcnContentLock);
		RTMPFreeNdisPacket(pAd, bcn_info->BeaconPkt);
		bcn_info->BeaconPkt = NULL;

		RTMP_SEM_UNLOCK(&bcn_info->BcnContentLock);
	}

	NdisFreeSpinLock(&bcn_info->BcnContentLock);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Pre-build a BEACON frame in the shared memory
    return value
	0:  mean no beacon necessary.
	>0: beacon length.
    ==========================================================================
*/
UINT16 MakeBeacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN UpdateRoutine)
{
	ULONG FrameLen = 0, UpdatePos = 0;
	UCHAR *pBeaconFrame, *tmac_info;
	HTTRANSMIT_SETTING BeaconTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	USHORT PhyMode;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *ptr = NULL;
	BSS_STRUCT *pMbss = NULL;
#endif
	BCN_BUF_STRUCT *pbcn_buf = &wdev->bcn_buf;

#ifdef RT_CFG80211_SUPPORT
	if (pAd->cfg80211_ctrl.beaconIsSetFromHostapd == TRUE)
		return -1;
#endif


	RTMP_SEM_LOCK(&pbcn_buf->BcnContentLock);
#ifdef ZERO_LOSS_CSA_SUPPORT
	/*reset CsaIELocation,	to be filled in MakeChSwitchAnnounceIEandExtend()*/
	if (pAd->Zero_Loss_Enable)
		pbcn_buf->CsaIELocationInBeacon = 0;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pbcn_buf->BeaconPkt);

	if (IS_HIF_TYPE(pAd, HIF_MT))
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	else
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);

	/* if (UpdateRoutine == FALSE) */
	/* { */
	/* not periodically update case, need take care Header and IE which is before TIM ie. */
	FrameLen = ComposeBcnPktHead(pAd, wdev, pBeaconFrame);
	pbcn_buf->TimIELocationInBeacon = (UCHAR)FrameLen;
	/* } */
	UpdatePos = pbcn_buf->TimIELocationInBeacon;
	PhyMode = wdev->PhyMode;

	if (UpdateRoutine == TRUE)
		FrameLen = UpdatePos;/* update routine, no FrameLen information, update it for later use. */

#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		/* Tim IE, AP mode only. */
		pbcn_buf->cap_ie_pos = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;
		/*
		    step 1 - update AP's Capability info, since it might be changed.
		*/
		ptr = pBeaconFrame + pbcn_buf->cap_ie_pos;
#ifdef RT_BIG_ENDIAN
		*(ptr + 1) = (UCHAR)(pMbss->CapabilityInfo & 0x00ff);
		*ptr = (UCHAR)((pMbss->CapabilityInfo & 0xff00) >> 8);
#else
		*ptr = (UCHAR)(pMbss->CapabilityInfo & 0x00ff);
		*(ptr + 1) = (UCHAR)((pMbss->CapabilityInfo & 0xff00) >> 8);
#endif
		/*
		    step 2 - update TIM IE
		    TODO: enlarge TIM bitmap to support up to 64 STAs
		    TODO: re-measure if RT2600 TBTT interrupt happens faster than BEACON sent out time
		*/
		ptr = pBeaconFrame + pbcn_buf->TimIELocationInBeacon;
		FrameLen += BcnTimUpdate(pAd, wdev, ptr);
		UpdatePos = FrameLen;
	}

#endif /* CONFIG_AP_SUPPORT */
	ComposeBcnPktTail(pAd, wdev, &UpdatePos, pBeaconFrame);
	FrameLen = UpdatePos;/* update newest FrameLen. */

#ifdef IGMP_TVM_SUPPORT
		/* ADD TV IE to this packet */
		MakeTVMIE(pAd, wdev, pBeaconFrame, &FrameLen);
#endif /* IGMP_TVM_SUPPORT */
	/* step 6. Since FrameLen may change, update TXWI. */
#ifdef A_BAND_SUPPORT

	if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}

#endif /* A_BAND_SUPPORT */
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->CommonCfg.GNMixMode
		&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
			|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
			|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}
#endif /* GN_MIXMODE_SUPPORT */
#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && WMODE_CAP_2G(wdev->PhyMode)) {
		BeaconTransmit.field.MODE =
			(BeaconTransmit.field.MODE >= MODE_OFDM) ? BeaconTransmit.field.MODE : MODE_OFDM;
	}
#endif /* OCE_SUPPORT */
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	if (wdev->eap.eap_bcnrate_en == TRUE) {
		BeaconTransmit.field.MODE = wdev->eap.bcnphymode.field.MODE;
		BeaconTransmit.field.MCS = wdev->eap.bcnphymode.field.MCS;
	}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	/*Update current FrameLen in Bcn_Buffer*/
	pbcn_buf->FrameLen = FrameLen;
#ifdef CONFIG_6G_SUPPORT
	/* align tx rate to 6g BC.Probe.Rsp */
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		UCHAR iob_mode = wlan_config_get_unsolicit_tx_mode(wdev);

		if (iob_mode == UNSOLICIT_TXMODE_HE_SU) {
			BeaconTransmit.field.MODE = MODE_HE;
			BeaconTransmit.field.MCS = MCS_0;
		} else if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP) {
			BeaconTransmit.field.BW = BW_80;
			BeaconTransmit.field.MODE = MODE_OFDM;
			BeaconTransmit.field.MCS = MCS_RATE_6;
		}
	}
#endif

	write_tmac_info_offload_pkt(pAd, wdev, FC_TYPE_MGMT, SUBTYPE_BEACON,
								tmac_info, &BeaconTransmit, FrameLen);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, pBeaconFrame, DIR_WRITE, FALSE);
#endif /* RT_BIG_ENDIAN */
	RTMP_SEM_UNLOCK(&pbcn_buf->BcnContentLock);

#ifdef WIFI_DIAG
	diag_bcn_tx(pAd, pMbss, pBeaconFrame, FrameLen);
#endif

	return FrameLen;
}


VOID ComposeRSNIE(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TempLen = 0;
	CHAR rsne_idx = 0;

#ifdef DISABLE_HOSTAPD_BEACON
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef CONFIG_HOTSPOT_R2
	extern UCHAR			OSEN_IE[];
	extern UCHAR			OSEN_IELEN;
	if ((pMbss->HotSpotCtrl.HotSpotEnable == 0) && (pMbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		UCHAR RSNIe = IE_WPA;
		MakeOutgoingFrame(pBeaconFrame+FrameLen,		&TempLen,
							 1,							&RSNIe,
							 1,							&OSEN_IELEN,
							 OSEN_IELEN,					OSEN_IE,
							 END_OF_ARGS);
		FrameLen += TempLen;
	} else
#endif /* CONFIG_HOTSPOT_R2 */

	for (rsne_idx = 0; rsne_idx < 2; rsne_idx++) {
		if (pMbss->RSNIE_Len[rsne_idx] != 0) {
			MakeOutgoingFrame(pBeaconFrame+FrameLen,
				 &TempLen, 1,
			&pMbss->RSNIE_ID[rsne_idx], 1,
			&pMbss->RSNIE_Len[rsne_idx],
			pMbss->RSNIE_Len[rsne_idx], &pMbss->RSN_IE[rsne_idx][0],
			END_OF_ARGS);
			FrameLen += TempLen;
		}
	}
#else
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
#ifdef CONFIG_HOTSPOT_R2
	extern UCHAR            OSEN_IE[];
	extern UCHAR            OSEN_IELEN;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	if ((pMbss->HotSpotCtrl.HotSpotEnable == 0) && (pMbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		UCHAR RSNIe = IE_WPA;

		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TempLen,
						  1,                            &RSNIe,
						  1,                            &OSEN_IELEN,
						  OSEN_IELEN,                   OSEN_IE,
						  END_OF_ARGS);
		FrameLen += TempLen;
	} else
#endif /* CONFIG_HOTSPOT_R2 */
	{
		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
				continue;

			MakeOutgoingFrame(pBeaconFrame + FrameLen, &TempLen,
							  1, &pSecConfig->RSNE_EID[rsne_idx][0],
							  1, &pSecConfig->BCN_RSNE_Len[rsne_idx],
							  pSecConfig->BCN_RSNE_Len[rsne_idx], &pSecConfig->BCN_RSNE_Content[rsne_idx][0],
							  END_OF_ARGS);
			FrameLen += TempLen;
		}
	}
#endif /*DISABLE_HOSTAPD_BEACON */
    *pFrameLen = FrameLen;
}

VOID ComposeWPSIE(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#if defined(HOSTAPD_SUPPORT) || defined(WSC_AP_SUPPORT)
	BOOLEAN bHasWpsIE = FALSE;
#endif
#endif
#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP)
		pMbss = wdev->func_dev;

	if (pMbss == NULL)
		return;

#endif
#ifdef WSC_AP_SUPPORT

	/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_BEACON
    if (wdev->WscIEBeacon.ValueLen)
#else
    if (((wdev->WscControl.WscConfMode >= 1) && (wdev->WscIEBeacon.ValueLen)))
#endif
		bHasWpsIE = TRUE;

	if (bHasWpsIE) {
		ULONG WscTmpLen = 0;
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &WscTmpLen,
						  wdev->WscIEBeacon.ValueLen, wdev->WscIEBeacon.Value,
						  END_OF_ARGS);
		FrameLen += WscTmpLen;
	}

	if (pMbss && (wdev->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
		IS_IEEE8021X_Entry(&pMbss->wdev) &&
#endif /* DOT1X_SUPPORT */
		IS_CIPHER_WEP_Entry(&pMbss->wdev)) {
		ULONG TempLen = 0;
		UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TempLen,
						  7,                            PROVISION_SERVICE_IE,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}

#endif /* WSC_AP_SUPPORT */
	*pFrameLen = FrameLen;
}

VOID MakeErpIE(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pBeaconFrame
)
{
	ULONG FrameLen = *pFrameLen;
	UCHAR *ptr = NULL;
	/* fill ERP IE */
	ptr = (UCHAR *)pBeaconFrame + FrameLen;
	*ptr = IE_ERP;
	*(ptr + 1) = 1;
#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP)
		*(ptr + 2) = pAd->ApCfg.ErpIeContent;

#endif
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA)
		*(ptr + 2) = 0x04;

#endif
	FrameLen += 3;
	*pFrameLen = FrameLen;
}

#if defined(A_BAND_SUPPORT) && defined(CONFIG_AP_SUPPORT)
VOID MakeChSwitchAnnounceIEandExtend(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame, BOOLEAN bcn)
{
	UCHAR *ptr = NULL;
	ULONG FrameLen = *pFrameLen;
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	struct DOT11_H *pDot11h = NULL;
	UCHAR channel;
	USHORT PhyMode;
	UCHAR bw = BW_20;
	UCHAR reg_cap_bw;
	UCHAR cfg_ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR cfg_vht_bw;
#endif
#ifdef DOT11_HE_AX
	UCHAR cfg_he_bw;
#endif

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"wdev is NULL, return\n");
		return;
	}

	channel = wdev->channel;
	PhyMode = wdev->PhyMode;
	reg_cap_bw = get_channel_bw_cap(wdev, channel);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	cfg_vht_bw = wlan_config_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
	cfg_he_bw = wlan_config_get_he_bw(wdev);
#endif /*DOT11_HE_AX*/

	if (WMODE_CAP_N(PhyMode)) {
		if (cfg_ht_bw)
			bw = BW_40;
		else
			bw = BW_20;
	}

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		switch (cfg_vht_bw) {
		case VHT_BW_80:
			bw = BW_80;
			break;

		case VHT_BW_160:
			bw = BW_160;
			break;

		case VHT_BW_8080:
			bw = BW_8080;
			break;

		case VHT_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		switch (cfg_he_bw) {
		case HE_BW_80:
			bw = BW_80;
			break;
		case HE_BW_160:
			bw = BW_160;
			break;
		case HE_BW_8080:
			bw = BW_8080;
			break;
		case HE_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}
#endif /* DOT11_HE_AX */

/* if bw capability of NewCh is lower than .dat bw config, bw should follow reg_cap_bw*/
	if (bw > reg_cap_bw) {
		if (!(bw == BW_8080 && (reg_cap_bw == BW_80 || reg_cap_bw == BW_160))) {
			bw = reg_cap_bw;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
			"Channel=%d, bw=%d\n", channel, bw);

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"pDot11h is NULL, return\n");
		return;
	}

	if (bcn)
		wdev->bcn_buf.CsaIELocationInBeacon = FrameLen;

	ptr = pBeaconFrame + FrameLen;
	*ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	*(ptr + 1) = 3;
#ifdef ZERO_LOSS_CSA_SUPPORT
	/*if radar not detected on old channel, allow frames*/
	if ((pAd->Zero_Loss_Enable == 1) && (pDot11h->RDMode != RD_SILENCE_MODE))
		*(ptr + 2) = 0;/*frames allowed*/
	else
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	*(ptr + 2) = 1;/*No further frames*/
	*(ptr + 3) = channel;
	*(ptr + 4) = (pDot11h->CSPeriod - pDot11h->CSCount - 1);

	ptr += 5;
	FrameLen += 5;
#ifdef DOT11_N_SUPPORT
	/* Extended Channel Switch Announcement Element */
	if (pComCfg->bExtChannelSwitchAnnouncement) {
		HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

		build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe,
									channel,
									wdev->PhyMode,
									wdev
								   );
		NdisMoveMemory(ptr, &HtExtChannelSwitchIe, sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE));
		ptr += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
		FrameLen += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
	}
#ifdef ZERO_LOSS_CSA_SUPPORT
	/*add secondary channel offset IE
	* for 5Ghz, find ext chn offset using new channel
	*/
	if (pAd->Zero_Loss_Enable) {
		struct GNU_PACKED SecondaryChannelOffsetIe {
		UCHAR		ID;
		UCHAR		Length;
		UCHAR		SecondaryChannelOffset;
		};
		struct SecondaryChannelOffsetIe SecChanOffsetIe;
		UCHAR ext_cha = 0, op_ht_bw = 0;
		int idx;
		UCHAR wfa_ht_ch_ext[] = {
		36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
		44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
		52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
		60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
		100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
		108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
		116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
		124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
		132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
		140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
		149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
		157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
				0, 0};

		op_ht_bw = wlan_operate_get_ht_bw(wdev);

		if (op_ht_bw == BW_40) {
			if (wdev->channel > 14) {
				idx = 0;
				while (wfa_ht_ch_ext[idx] != 0) {
					if (wfa_ht_ch_ext[idx] == wdev->channel) {
						ext_cha = wfa_ht_ch_ext[idx + 1];
						break;
					}
					idx += 2;
				};
				if (wfa_ht_ch_ext[idx] == 0) {
					ext_cha = EXTCHA_NONE;
				}
			} else {
				/*2G band case*/
				ext_cha = wlan_operate_get_ext_cha(wdev);
			}
		}
		SecChanOffsetIe.ID = 0x3e;
		SecChanOffsetIe.Length = 0x01;
		SecChanOffsetIe.SecondaryChannelOffset = ext_cha;
		NdisMoveMemory(ptr, &SecChanOffsetIe, sizeof(struct SecondaryChannelOffsetIe));
		ptr += sizeof(struct SecondaryChannelOffsetIe);
		FrameLen += sizeof(struct SecondaryChannelOffsetIe);
	}
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		INT tp_len = 0, wb_len = 0;
		UCHAR *ch_sw_wrapper;
		VHT_TXPWR_ENV_IE txpwr_env;
		UCHAR ch_band = wlan_config_get_ch_band(wdev);

		*ptr = IE_CH_SWITCH_WRAPPER;
		ch_sw_wrapper = (UCHAR *)(ptr + 1); /* reserve for length */
		ptr += 2; /* skip len */

		if (bw >= BW_40) {
			WIDE_BW_CH_SWITCH_ELEMENT wb_info;
			*ptr = IE_WIDE_BW_CH_SWITCH;
			*(ptr + 1) = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
			ptr += 2;
			NdisZeroMemory(&wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));

			switch (bw) {
			case BW_40:
				wb_info.new_ch_width = 0;
				wb_info.center_freq_1 = vht_cent_ch_freq_40mhz(channel, VHT_BW_2040, ch_band);
				wb_info.center_freq_2 = 0;
				break;

			case BW_80:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_80, ch_band);
				wb_info.center_freq_2 = 0;
				break;

			case BW_160:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_80, ch_band);
				wb_info.center_freq_2 = vht_cent_ch_freq(channel, VHT_BW_160, ch_band);
				break;

			case BW_8080:
				wb_info.new_ch_width = 1;
				wb_info.center_freq_1 = vht_cent_ch_freq(channel, VHT_BW_8080, ch_band);
				wb_info.center_freq_2 = wlan_operate_get_cen_ch_2(wdev);
				break;
			}

			NdisMoveMemory(ptr, &wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
			wb_len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
			ptr += wb_len;
			wb_len += 2;
		}

		if (!WMODE_CAP_AX_6G(PhyMode)) {
			*ptr = IE_VHT_TXPWR_ENV;
			NdisZeroMemory(&txpwr_env, sizeof(VHT_TXPWR_ENV_IE));
			tp_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
			*(ptr + 1) = tp_len;
			ptr += 2;
			NdisMoveMemory(ptr, &txpwr_env, tp_len);
			ptr += tp_len;
			tp_len += 2;
		}
		*ch_sw_wrapper = wb_len + tp_len;
		FrameLen += (2 + wb_len + tp_len);
	}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	*pFrameLen = FrameLen;
}
#endif /* A_BAND_SUPPORT */

VOID MakeHTIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG TmpLen;
	ULONG FrameLen = *pFrameLen;
	UCHAR HtLen, HtLen1;
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	/*UCHAR i; */
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *ht_cap;
#ifdef RT_BIG_ENDIAN
	ADD_HT_INFO_IE  addHTInfoTmp;
#endif
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	/* add HT Capability IE */
	HtLen = sizeof(HT_CAPABILITY_IE);
	HtLen1 = sizeof(ADD_HT_INFO_IE);

	if (WMODE_CAP_AX_6G(wdev->PhyMode))
		return;

	ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
#ifndef RT_BIG_ENDIAN
	NdisMoveMemory(&HtCapabilityTmp, ht_cap, HtLen);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;
	MakeOutgoingFrame(pBeaconFrame + FrameLen,         &TmpLen,
					  1,                                &HtCapIe,
					  1,                                &HtLen,
					  HtLen,          &HtCapabilityTmp,
					  1,                                &AddHtInfoIe,
					  1,                                &HtLen1,
					  HtLen1,          addht,
					  END_OF_ARGS);
#else
	NdisMoveMemory(&HtCapabilityTmp, ht_cap, HtLen);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = addht->AddHtInfo.RecomWidth;
	*(UINT32 *)(&HtCapabilityTmp.TxBFCap) = cpu2le32(*(UINT32 *)(&HtCapabilityTmp.TxBFCap));
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
	NdisMoveMemory(&addHTInfoTmp, addht, HtLen1);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = cpu2le16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = cpu2le16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));
	MakeOutgoingFrame(pBeaconFrame + FrameLen,         &TmpLen,
					  1,                                &HtCapIe,
					  1,                                &HtLen,
					  HtLen,                   &HtCapabilityTmp,
					  1,                                &AddHtInfoIe,
					  1,                                &HtLen1,
					  HtLen1,                   &addHTInfoTmp,
					  END_OF_ARGS);
#endif
	FrameLen += TmpLen;
#ifdef DOT11N_DRAFT3

	/*
	    P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE
	*/
	if (WMODE_CAP_2G(wdev->PhyMode) &&
		(ht_cap->HtCapInfo.ChannelWidth == 1)) {
		OVERLAP_BSS_SCAN_IE  OverlapScanParam;
		ULONG   TmpLen;
		UCHAR   OverlapScanIE, ScanIELen;

		OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
		ScanIELen = 14;
		OverlapScanParam.ScanPassiveDwell = cpu2le16(pComCfg->Dot11OBssScanPassiveDwell);
		OverlapScanParam.ScanActiveDwell = cpu2le16(pComCfg->Dot11OBssScanActiveDwell);
		OverlapScanParam.TriggerScanInt = cpu2le16(pComCfg->Dot11BssWidthTriggerScanInt);
		OverlapScanParam.PassiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanPassiveTotalPerChannel);
		OverlapScanParam.ActiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanActiveTotalPerChannel);
		OverlapScanParam.DelayFactor = cpu2le16(pComCfg->Dot11BssWidthChanTranDelayFactor);
		OverlapScanParam.ScanActThre = cpu2le16(pComCfg->Dot11OBssScanActivityThre);
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  1,          &OverlapScanIE,
						  1,          &ScanIELen,
						  ScanIELen,  &OverlapScanParam,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11N_DRAFT3 */
	*pFrameLen = FrameLen;
}

#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
VOID MakeHotSpotIE(struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen;
	BSS_STRUCT *pMbss = wdev->func_dev;

	if (pMbss->HotSpotCtrl.HotSpotEnable) {
		RTMP_SEM_LOCK(&pMbss->HotSpotCtrl.IeLock);
		/* Indication element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->HotSpotCtrl.HSIndicationIELen,
						  pMbss->HotSpotCtrl.HSIndicationIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* Roaming Consortium element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->HotSpotCtrl.RoamingConsortiumIELen,
						  pMbss->HotSpotCtrl.RoamingConsortiumIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* P2P element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->HotSpotCtrl.P2PIELen,
						  pMbss->HotSpotCtrl.P2PIE, END_OF_ARGS);
		FrameLen += TmpLen;
		RTMP_SEM_UNLOCK(&pMbss->HotSpotCtrl.IeLock);
	}

	*pFrameLen = FrameLen;
}
#endif /*CONFIG_HOTSPOT_IE*/

VOID MakeCountryIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen, TmpLen2 = 0;
	UCHAR *TmpFrame = NULL;
	UCHAR CountryIe = IE_COUNTRY;
	UCHAR Environment = 0x20;

	if (pAd->CommonCfg.bCountryFlag ||
		(WMODE_CAP_5G(wdev->PhyMode) && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
		|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
	   ) {
		os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);

		if (TmpFrame != NULL) {
			NdisZeroMemory(TmpFrame, 256);
			/* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
			BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else
			if (WMODE_CAP_6G(wdev->PhyMode)) {
				UINT i = 0;
				UCHAR OpExtIdentifier = 0xFE;
				UCHAR CoverageClass = 0;
				UCHAR reg_class_value[5] = {0};

				get_reg_class_list_for_6g(pAd, wdev->PhyMode, reg_class_value);

				if (reg_class_value[0] == 0) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"reg_class is NULL !!!\n");
					os_free_mem(TmpFrame);
					return;
				}

				for (i = 0; reg_class_value[i] != 0; i++) {
					MakeOutgoingFrame(TmpFrame + TmpLen2,
							&TmpLen,
							1,
							&OpExtIdentifier,
							1,
							&reg_class_value[i],
							1,
							&CoverageClass,
							END_OF_ARGS);
					TmpLen2 += TmpLen;
					if (i == 4)
						break;
				}
			} else {
				UINT i = 0;
				PCH_DESC pChDesc = NULL;
				UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
				UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

				MaxTxPower = MAX_TRANSMIT_POWER;
				/* do not change sequence due to 6GHz might include AC/GN then confused */
				if (WMODE_CAP_5G(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
					if (pAd->CommonCfg.pChDesc5G != NULL)
						pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc5G;
					else
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
								 "pChDesc5G is NULL !!!\n");
				} else if (WMODE_CAP_2G(wdev->PhyMode)) {
					if (pAd->CommonCfg.pChDesc2G != NULL)
						pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc2G;
					else
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "pChDesc2G is NULL !!!\n");
				}

				if (pChDesc == NULL) {
					MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
							 "pChDesc is NULL !!!\n");
					os_free_mem(TmpFrame);
					return;
				}

				for (i = 0; pChDesc[i].FirstChannel != 0; i++) {
					MakeOutgoingFrame(TmpFrame + TmpLen2,
									  &TmpLen,
									  1,
									  &pChDesc[i].FirstChannel,
									  1,
									  &pChDesc[i].NumOfCh,
									  1,
									  &MaxTxPower,
									  END_OF_ARGS);
					TmpLen2 += TmpLen;
				}
			}
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef DOT11K_RRM_SUPPORT


#endif /* DOT11K_RRM_SUPPORT */
#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(wdev))
				Environment = MBO_AP_USE_GLOBAL_OPERATING_CLASS;
#endif /* MBO_SUPPORT */

			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2 % 2) == 0) {
				UCHAR TmpLen3 = TmpLen2 + 4;

				MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2+1, TmpFrame,
								  END_OF_ARGS);
			} else {
				UCHAR TmpLen3 = TmpLen2 + 3;

				MakeOutgoingFrame(pBeaconFrame + FrameLen,
								  &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2, TmpFrame,
								  END_OF_ARGS);
			}

			FrameLen += TmpLen;
			os_free_mem(TmpFrame);
		} else
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "Allocate memory fail!!!\n");
	}

	*pFrameLen = FrameLen;
}

VOID MakeChReportIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	USHORT PhyMode = wdev->PhyMode;
#ifdef DOT11K_RRM_SUPPORT
	/* UCHAR i; */
#else
	UCHAR APChannelReportIe = IE_AP_CHANNEL_REPORT;
	ULONG TmpLen;
#endif
#ifdef DOT11K_RRM_SUPPORT
	InsertChannelRepIE(pAd, pBeaconFrame + FrameLen, &FrameLen,
					   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
					   get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev),
					   NULL, PhyMode, wdev->func_idx);
#else
	{
		/*
		    802.11n D2.0 Annex J, USA regulatory
			class 32, channel set 1~7
			class 33, channel set 5-11
		*/
		UCHAR rclass32[] = {32, 1, 2, 3, 4, 5, 6, 7};
		UCHAR rclass33[] = {33, 5, 6, 7, 8, 9, 10, 11};
		UCHAR rclasslen = 8; /*sizeof(rclass32); */

		if (PhyMode == (WMODE_B | WMODE_G | WMODE_GN)) {
			MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass32,
							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass33,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
	*pFrameLen = FrameLen;
}

VOID MakeExtSuppRateIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	struct legacy_rate *rate = &wdev->rate.legacy_rate;

	*pFrameLen += build_support_ext_rate_ie(wdev, rate->sup_rate_len,
		rate->ext_rate, rate->ext_rate_len, pBeaconFrame + *pFrameLen);
}

VOID MakePwrConstraintIe(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	ULONG FrameLen = *pFrameLen;
	ULONG TmpLen = 0;
#ifdef DOT11_VHT_AC
	USHORT PhyMode = wdev->PhyMode;
#endif

	/*
		Only APs that comply with 802.11h or 802.11k are required to include
		the Power Constraint element (IE=32) and
		the TPC Report element (IE=35) and
		the VHT Transmit Power Envelope element (IE=195)
		in beacon frames and probe response frames
	*/
	if ((WMODE_CAP_5G(wdev->PhyMode) && pComCfg->bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
		|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
	   ) {
		UINT8 PwrConstraintIE = IE_POWER_CONSTRAINT;
		UINT8 PwrConstraintLen = 1;
#if defined(TPC_SUPPORT) && defined(TPC_MODE_CTRL)
		UINT8 PwrConstraint = wdev->pwrCnstrnt;
#else
		UINT8 PwrConstraint = pComCfg->PwrConstraint;
#endif
		/* prepare power constraint IE */
		MakeOutgoingFrame(pBeaconFrame + FrameLen,    &TmpLen,
						  1,                          &PwrConstraintIE,
						  1,                          &PwrConstraintLen,
						  1,                          &PwrConstraint,
						  END_OF_ARGS);
		FrameLen += TmpLen;
#ifdef TPC_SUPPORT
		/* prepare TPC Report IE */
		InsertTpcReportIE(pAd,
						  pBeaconFrame + FrameLen,
						  &FrameLen,
						  GetSkuTxPwr(pAd, wdev, SUBTYPE_BEACON),
						  0);
#endif
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(PhyMode)) {
			UINT8 he_txpwr_env_ie = IE_VHT_TXPWR_ENV;
			UINT8 ie_len;
			HE_TXPWR_ENV_IE txpwr_env;

			TmpLen = 0;
			ie_len = build_he_txpwr_envelope(wdev, (UCHAR *)&txpwr_env);
			if (ie_len) {
				MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						1,                   &he_txpwr_env_ie,
						1,                   &ie_len,
						ie_len,              &txpwr_env,
						END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}	/* prepare VHT Transmit Power Envelope IE */
		else if (WMODE_CAP_AC(PhyMode)) {
			UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
			UINT8 ie_len;
			VHT_TXPWR_ENV_IE txpwr_env;

			TmpLen = 0;
			ie_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
			if (ie_len) {
				MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						1,	             &vht_txpwr_env_ie,
						1,	             &ie_len,
						ie_len,	      	     &txpwr_env,
						END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

#else
#ifdef DOT11_VHT_AC
		if (!WMODE_CAP_AX_6G(PhyMode)) {
			/* prepare VHT Transmit Power Envelope IE */
			if (WMODE_CAP_AC(PhyMode)) {
				UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
				UINT8 ie_len;
				VHT_TXPWR_ENV_IE txpwr_env;

				TmpLen = 0;
				ie_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
				if (ie_len) {
					MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
							1,                   &vht_txpwr_env_ie,
							1,                   &ie_len,
							ie_len,              &txpwr_env,
							END_OF_ARGS);
					FrameLen += TmpLen;
				}
			}
		}
#endif /* DOT11_VHT_AC */
#endif
	}

	*pFrameLen = FrameLen;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
/* 9.4.2.72 Nontransmitted BSSID Capability element, and SSID*/
static VOID make_nontransmitted_bssid_cap_ie(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_STRUCT *pMbss,
	ULONG *pFrameLen,
	UCHAR *pOutBuffer)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TempLen;
	UCHAR NontransmittedBssidCapIe = IE_NONTRANSMITTED_BSSID_CAP;
	UCHAR ie_len = 0;
	UCHAR *pSsid = NULL;
	USHORT CapabilityInfo;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"\t1 2 %s, IdBss = %d, pFrameLen = %ld\n", __func__, pMbss->mbss_idx, *pFrameLen);

	/* Capability element */
	ie_len = 2;
	CapabilityInfo = cpu2le16(pMbss->CapabilityInfo);
	MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
					1, &NontransmittedBssidCapIe,
					1, &ie_len,
					2, &CapabilityInfo,
					END_OF_ARGS);
	FrameLen += TempLen;

	/* SSID element */
	ie_len = (pMbss->bHideSsid) ? 0 : pMbss->SsidLen;
	pSsid = pMbss->Ssid;
	MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
					1, &SsidIe,
					1, &ie_len,
					ie_len, pSsid,
					END_OF_ARGS);
	FrameLen += TempLen;

	*pFrameLen = FrameLen;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"\t1 2 CapIE/SSID End pFrameLen = %ld\n", *pFrameLen);

}

/* 9.4.2.74 Multiple BSSID-Index element */
static VOID make_nontransmitted_bssid_idx_ie(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_STRUCT *pMbss,
	ULONG *pFrameLen,
	UCHAR *pOutBuffer,
	BOOLEAN isProbeRsp)
{
	ULONG FrameLen = *pFrameLen;
	ULONG TempLen;
	UCHAR NontransmittedBssidIdxIe = IE_MULTIPLE_BSSID_IDX;
	UCHAR ie_len;
	BCN_BUF_STRUCT *pbcn_buf = &pMbss->wdev.bcn_buf;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"\t3 %s, mbss_grp_idx = %d, FrameLen = %ld\n", __func__, pMbss->mbss_grp_idx, *pFrameLen);

	/* Len = 1 when IE is included in the Probe.Rsp, otherwise Len = 3 */
	if (isProbeRsp) {
		ie_len = 1;
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
						1, &NontransmittedBssidIdxIe,
						1, &ie_len,
						1, &pMbss->mbss_grp_idx,
						END_OF_ARGS);
	} else {
		ie_len = 3;
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
						1, &NontransmittedBssidIdxIe,
						1, &ie_len,
						1, &pMbss->mbss_grp_idx,
						1, &pAd->ApCfg.DtimPeriod,
						1, &pAd->ApCfg.DtimCount,
						END_OF_ARGS);

		pbcn_buf->TimIELocationInBeacon = (UINT16)FrameLen;
	}
	FrameLen += TempLen;

	*pFrameLen = FrameLen;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"\t3 BssidIdxIE End FrameLen = %ld\n", *pFrameLen);
}

/* 9.4.2.46 Optional subelement IDs for Multiple BSSID */
static VOID make_nontransmitted_bssid_sub_ie(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_STRUCT *pMbss,
	ULONG *pFrameLen,
	UCHAR *pOutBuffer,
	BOOLEAN isProbeRsp)
{
	P_MULTIPLE_BSSID_SUB_IE_T pmbss_sub_ie = NULL;
	ULONG FrameLen = *pFrameLen;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"%s, IdBss = %d, FrameLen = %ld\n", __func__, pMbss->mbss_idx, *pFrameLen);

	/* pointer to Multiple BSSID subelement */
	pmbss_sub_ie = (P_MULTIPLE_BSSID_SUB_IE_T)(pOutBuffer + FrameLen);

	/* Nontransmitted BSSID Profile */
	pmbss_sub_ie->sub_eid = SUB_IE_NON_TRANS_PROFILE;

	/* move pointer to NonTransProfiles */
	*pFrameLen = FrameLen + 2; /* SubEID, LEN */

	/* make Nontransmitted BSSID Profile */
	make_nontransmitted_bssid_cap_ie(pAd, pMbss, pFrameLen, pOutBuffer);
	make_nontransmitted_bssid_idx_ie(pAd, pMbss, pFrameLen, pOutBuffer, isProbeRsp);

	/* TBD - others subIEs for Nontransmitted BSSID Profile */
	ComposeRSNIE(pAd, &pMbss->wdev, pFrameLen, pOutBuffer);
	ComposeWPSIE(pAd, &pMbss->wdev, pFrameLen, pOutBuffer);

	pmbss_sub_ie->len = *pFrameLen - FrameLen - 2; /* -2 byte: SubEID, LEN */

	/* TBD - Vendor Specific */

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"NonTransSubIE End FrameLen = %ld, IE Len = %d\n", *pFrameLen, pmbss_sub_ie->len);

	/* *pFrameLen = FrameLen; */
}

/* 9.4.2.46 Multiple BSSID element */
VOID make_multiple_bssid_ie(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pOutBuffer,
	UINT32 Bitmap,
	BOOLEAN isProbeRsp)
{
	P_MULTIPLE_BSSID_IE_T pmbss_ie = NULL;
	ULONG FrameLen;
	INT32 IdBss;
	BSS_STRUCT *pMbss = NULL;
	UINT8 DbdcIdx = HcGetBandByWdev(wdev);

	if (!IS_MBSSID_IE_NEEDED(pAd, wdev->func_dev, DbdcIdx))
		return;

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"%s, MBSSID Bitmap = 0x%08x, FrameLen = %ld\n", __func__, Bitmap, *pFrameLen);

	/* create multiple bssid IEs for each non-transmitted BSSID by bitmap setting */
	for (IdBss = FIRST_MBSSID; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		pMbss = &pAd->ApCfg.MBSSID[IdBss];
		if (BeaconTransmitRequired(pAd, &pMbss->wdev, FALSE) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"IdBss %d not ready for beaconing!\n", IdBss);
			continue;
		}

		/* check bss band */
		if (HcGetBandByWdev(&pMbss->wdev) != DbdcIdx) {
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"IdBss %d at diff band(%d)!\n", IdBss, HcGetBandByWdev(&pMbss->wdev));
			continue;
		}

		if ((Bitmap & (1 << pMbss->mbss_grp_idx)) && IS_BSSID_11V_NON_TRANS(pAd, pMbss, DbdcIdx)) {
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"Add IdBss %d IE: (mbss_grp_idx=%d)\n", IdBss, pMbss->mbss_grp_idx);

			FrameLen = *pFrameLen;

			/* pointer to Multiple BSSID element */
			pmbss_ie = (P_MULTIPLE_BSSID_IE_T)(pOutBuffer + FrameLen);

			pmbss_ie->eid = IE_MULTIPLE_BSSID;
			pmbss_ie->dot11v_max_bssid_indicator = pAd->ApCfg.dot11v_max_bssid_indicator[DbdcIdx];

			/* move pointer to MBSSID's SubIEs */
			*pFrameLen = FrameLen + 3; /* EID, LEN, MaxBssidIndicator*/

			/* build optional subIE */
			make_nontransmitted_bssid_sub_ie(pAd, pMbss, pFrameLen, pOutBuffer, isProbeRsp);

			pmbss_ie->len = *pFrameLen - FrameLen - 2; /* -2 byte: EID, LEN */

			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"MbssIe IE Len = %d\n", pmbss_ie->len);
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
		"\tMultipleBssidIE End FrameLen = %ld\n", *pFrameLen);

}
#endif
#endif /* CONFIG_AP_SUPPORT */

VOID ComposeBcnPktTail(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = *pFrameLen;
	struct _build_ie_info vht_ie_info;
#if defined(A_BAND_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
#endif
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	UCHAR apidx = 0;
	/* BOOLEAN HotSpotEnable = FALSE; */
#ifdef A_BAND_SUPPORT
	struct DOT11_H *pDot11h = wdev->pDot11_H;
#endif
#endif
	USHORT PhyMode = wdev->PhyMode;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = NULL;
#endif /*AP_QLOAD_SUPPORT*/
#ifdef QOS_R2
	ULONG	TmpLen = 0;
	UCHAR	tmpbuf[50] = {0}, ielen = 0;
#endif

#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;
		apidx = wdev->func_idx;
	}

	/* fix klockwork issue */
	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "=unexpected pMbss NULL, please check\n");
		return;
	}

#endif /* CONFIG_AP_SUPPORT */
	vht_ie_info.frame_subtype = SUBTYPE_BEACON;
	vht_ie_info.channel = wdev->channel;
	vht_ie_info.phy_mode = PhyMode;
	vht_ie_info.wdev = wdev;

	MakeCountryIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef CONFIG_AP_SUPPORT
	MakePwrConstraintIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef A_BAND_SUPPORT
	if (pDot11h == NULL)
		return;
	/* fill up Channel Switch Announcement Element */
	if (wpa3_test_ctrl == 6 || wpa3_test_ctrl == 7)
		MakeChSwitchAnnounceIEandExtend(pAd, wdev, &FrameLen, pBeaconFrame, TRUE);
	else if (WMODE_CAP_5G(wdev->PhyMode)
		&& (pComCfg->bIEEE80211H == 1)
		&& (pDot11h->RDMode == RD_SWITCHING_MODE)
	   )
		MakeChSwitchAnnounceIEandExtend(pAd, wdev, &FrameLen, pBeaconFrame, TRUE);
	else if ((wdev->channel <= 14) && (pComCfg->ChannelSwitchFor2G.CHSWMode == CHANNEL_SWITCHING_MODE))
		MakeChSwitchAnnounceIEandExtend(pAd, wdev, &FrameLen, pBeaconFrame, TRUE);
	else
		wdev->bcn_buf.CsaIELocationInBeacon = 0;

#endif /* A_BAND_SUPPORT */

#ifdef CONFIG_6G_SUPPORT
	FrameLen += add_he_6g_rnr_ie(wdev, pBeaconFrame, FrameLen, 0);
#endif
#ifdef DOT11V_MBSSID_SUPPORT
	make_multiple_bssid_ie(pAd, wdev, &FrameLen, pBeaconFrame,
				pAd->ApCfg.dot11v_mbssid_bitmap[HcGetBandByWdev(wdev)], FALSE);
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, wdev, pBeaconFrame + FrameLen, &FrameLen, apidx);

#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	MakeChReportIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef DOT11R_FT_SUPPORT

	/*
	    The Mobility Domain information element (MDIE) is present in Beacon
	    frame when dot11FastBssTransitionEnable is set to true.
	*/
	if (pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable) {
		PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;
		FT_CAP_AND_POLICY FtCap;

		NdisZeroMemory(&FtCap, sizeof(FT_CAP_AND_POLICY));
		FtCap.field.FtOverDs = pFtCfg->FtCapFlag.FtOverDs;
		FtCap.field.RsrReqCap = pFtCfg->FtCapFlag.RsrReqCap;
		FT_InsertMdIE(pBeaconFrame + FrameLen, &FrameLen,
					  pFtCfg->FtMdId, FtCap);
	}

#endif /* DOT11R_FT_SUPPORT */

	/* Update ERP */
	if ((wdev->rate.legacy_rate.ext_rate_len) && (PhyMode != WMODE_B)) {
		if (WMODE_CAP_2G(wdev->PhyMode))
			MakeErpIE(pAd, wdev, &FrameLen, pBeaconFrame);
	}

	MakeExtSuppRateIe(pAd, wdev, &FrameLen, pBeaconFrame);
	ComposeRSNIE(pAd, wdev, &FrameLen, pBeaconFrame);
	ComposeWPSIE(pAd, wdev, &FrameLen, pBeaconFrame);

#ifdef HOSTAPD_OWE_SUPPORT
	if (pMbss->TRANSIE_Len) {
		ULONG TmpLen;

		MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
			pMbss->TRANSIE_Len, pMbss->TRANS_IE, END_OF_ARGS);
		FrameLen += TmpLen;
	}
#endif

#ifdef AP_QLOAD_SUPPORT
	pQloadCtrl = (struct _QLOAD_CTRL *) hc_get_qload_by_wdev(wdev);

	if (pQloadCtrl && pQloadCtrl->FlgQloadEnable != 0) {
#ifdef CONFIG_HOTSPOT_R2
		if (pMbss->HotSpotCtrl.QLoadTestEnable == 1)
			FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pBeaconFrame+FrameLen, apidx);
		else if (pMbss->HotSpotCtrl.QLoadTestEnable == 0)
#endif
		FrameLen += QBSS_LoadElementAppend(pAd, pBeaconFrame+FrameLen, pQloadCtrl, apidx);
	}
#endif /* AP_QLOAD_SUPPORT */
#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)

	if (pMbss->GASCtrl.b11U_enable)
		MakeHotSpotIE(wdev, &FrameLen, pBeaconFrame);

#endif /*CONFIG_HOTSPOT*/

#ifdef CONFIG_DOT11U_INTERWORKING
	if (pMbss->GASCtrl.b11U_enable) {
		ULONG TmpLen;
		RTMP_SEM_LOCK(&pMbss->GASCtrl.IeLock);
		/* Interworking element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->GASCtrl.InterWorkingIELen,
						  pMbss->GASCtrl.InterWorkingIE, END_OF_ARGS);
		FrameLen += TmpLen;
		/* Advertisement Protocol element */
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
						  pMbss->GASCtrl.AdvertisementProtoIELen,
						  pMbss->GASCtrl.AdvertisementProtoIE, END_OF_ARGS);
		FrameLen += TmpLen;
		RTMP_SEM_UNLOCK(&pMbss->GASCtrl.IeLock);
	}
#endif /* CONFIG_DOT11U_INTERWORKING */

#ifdef DOT11_N_SUPPORT
	/* step 5. Update HT. Since some fields might change in the same BSS. */
	if (WMODE_CAP_N(PhyMode) && (wdev->DesiredHtPhyInfo.bHtEnable)) {
		MakeHTIe(pAd, wdev, &FrameLen, pBeaconFrame);
#ifdef DOT11_VHT_AC
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		FrameLen += build_vht_ies(pAd, &vht_ie_info);
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

#ifdef QOS_R2
	if (pAd->ApCfg.MBSSID[apidx].bDSCPPolicyEnable) {
		QoS_Build_WFACapaIE(tmpbuf, &ielen, pAd->ApCfg.MBSSID[apidx].bDSCPPolicyEnable);
		MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen, ielen, tmpbuf, END_OF_ARGS);
		FrameLen += TmpLen;
	}
#endif

#ifdef CONFIG_AP_SUPPORT
	/* 7.3.2.27 Extended Capabilities IE */
	vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
	FrameLen += build_extended_cap_ie(pAd, &vht_ie_info);
#endif /*CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	if (wdev->bWmmCapable) {
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &vht_ie_info);
	}

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev)) {
#ifdef QUIET_SUPPORT
		PRRM_QUIET_CB pQuietCB = &pMbss->wdev.RrmCfg.QuietCB;

		RRM_InsertQuietIE(pAd, pBeaconFrame + FrameLen, &FrameLen,
						  pQuietCB->QuietCnt, pQuietCB->QuietPeriod,
						  pQuietCB->QuietDuration, pQuietCB->QuietOffset);
#endif
#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pBeaconFrame + FrameLen, &FrameLen);
		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pBeaconFrame + FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */
	}
#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(wdev->PhyMode) && wdev->DesiredHtPhyInfo.bHtEnable)
		FrameLen += add_beacon_he_ies(wdev, pBeaconFrame, FrameLen);
#endif /* DOT11_HE_AX */

	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
	FrameLen += build_vendor_ie(pAd, wdev, (pBeaconFrame + FrameLen), VIE_BEACON
								);
#if defined(MBO_SUPPORT) || defined(OCE_SUPPORT)
	if (IS_MBO_ENABLE(wdev) || IS_OCE_ENABLE(wdev))
		MakeMboOceIE(pAd, wdev, NULL, pBeaconFrame+FrameLen, &FrameLen, MBO_FRAME_TYPE_BEACON);
#endif /* OCE_SUPPORT MBO_SUPPORT */

#endif /*CONFIG_AP_SUPPORT*/
#ifdef P2P_SUPPORT
	if (P2P_GO_ON(pAd)) {
		PUCHAR  pP2pNoAIE = NULL;
		ULONG   P2pTmpLen;
		UCHAR   P2pCapId = SUBID_P2P_CAP, P2pDevId = SUBID_P2P_DEVICE_ID;
		USHORT  P2pCapIdLen = 2, P2pDevIdLen = 6;
		UCHAR   P2pIEFixed[6] = {0xdd, 0x12, 0x50, 0x6f, 0x9a, 0x9};

		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &P2pTmpLen,
						  6,                                            &P2pIEFixed[0],
						  1,                                            &P2pCapId,
						  2,                                            &P2pCapIdLen,
						  2,                                            &pAd->P2pCfg.P2pCapability,
						  END_OF_ARGS);
		FrameLen += P2pTmpLen;
		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &P2pTmpLen,
						  1,                                            &P2pDevId,
						  2,                                            &P2pDevIdLen,
						  6,                                            &pAd->P2pCfg.CurrentAddress,
						  END_OF_ARGS);
		FrameLen += P2pTmpLen;
		/* NoA */
		pP2pNoAIE = pBeaconFrame + FrameLen;
		P2pTmpLen = P2pUpdateNoABeacon(pAd, apidx, pP2pNoAIE);
		FrameLen += P2pTmpLen;
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}

#ifdef WFD_SUPPORT
	{
		ULONG TmpLen;

		ptr = pBeaconFrame + FrameLen;
		WfdMakeWfdIE(pAd, SUBTYPE_BEACON, ptr, &TmpLen);
		FrameLen += TmpLen;
	}
#endif /* WFD_SUPPORT */
#endif /* P2P_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && wdev->MAPCfg.vendor_ie_len) {
		ULONG MAPIeTmpLen = 0;

		MakeOutgoingFrame(pBeaconFrame + FrameLen, &MAPIeTmpLen,
						wdev->MAPCfg.vendor_ie_len, wdev->MAPCfg.vendor_ie_buf,
						END_OF_ARGS);
		FrameLen += MAPIeTmpLen;
	}
#endif /*WAPP_SUPPORT*/
#endif

#ifdef DPP_R2_SUPPORT
		if (wdev->DPPCfg.cce_ie_len) {
			ULONG DPPIeTmpLen = 0;

			MakeOutgoingFrame(pBeaconFrame + FrameLen, &DPPIeTmpLen,
							wdev->DPPCfg.cce_ie_len, wdev->DPPCfg.cce_ie_buf,
							END_OF_ARGS);
			FrameLen += DPPIeTmpLen;
		}
#endif


/*Vendor IE should be final IE to be added, so we can determine the maximum length of Beacon*/
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	RTMP_SPIN_LOCK(&pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.vendor_ie_lock);

	if (pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.pointer != NULL) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		ULONG TmpMaxBeaconLen;

#ifdef BCN_V2_SUPPORT
		if (apidx < cap->max_v2_bcn_num)
			TmpMaxBeaconLen = 1520 - cap->tx_hw_hdr_len;/*FW limitation*/
		else
			TmpMaxBeaconLen = 512 - cap->tx_hw_hdr_len;
#else
		TmpMaxBeaconLen = cap->BcnMaxLength - cap->tx_hw_hdr_len;
#endif

		if (FrameLen + pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.length > TmpMaxBeaconLen)
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				"BCN is too long, can't add vendor ie!\n");
		else {

			ULONG TmpLen;

			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
				"BCN add vendor ie\n");
			MakeOutgoingFrame(pBeaconFrame + FrameLen,
					  &TmpLen,
					  pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.length,
					  pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.pointer,
					  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
	RTMP_SPIN_UNLOCK(&pAd->ApCfg.MBSSID[apidx].ap_vendor_ie.vendor_ie_lock);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
/*Reduced neighbor report IE should be final IE to be added, so we can determine the maximum length of Beacon*/
#ifdef OCE_FILS_SUPPORT
		vht_ie_info.frame_buf = (UCHAR *)(pBeaconFrame + FrameLen);
		vht_ie_info.pos = FrameLen;
		FrameLen += oce_build_ies(pAd, &vht_ie_info, TRUE);
#endif

#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef MAP_R3
	if ((IS_MAP_ENABLE(pAd) && !IS_MAP_CERT_ENABLE(pAd))
		|| !IS_MAP_ENABLE(pAd))
#endif
		FrameLen +=  build_rsnxe_ie(&wdev->SecConfig,
				    (UCHAR *)pBeaconFrame + FrameLen);
#endif /* HOSTAPD_WPA3_SUPPORT*/
#ifdef HOSTAPD_WPA3R3_SUPPORT
	/* Add Rsnxe ie in beacon*/
	FrameLen +=  build_rsnxe_ie(wdev, &wdev->SecConfig,
			(UCHAR *)pBeaconFrame + FrameLen);
#endif

#ifdef BCN_PROTECTION_SUPPORT
	FrameLen +=  build_bcn_mmie(&wdev->SecConfig.bcn_prot_cfg,
				    (UCHAR *)pBeaconFrame + FrameLen); /* mmie should be the last of the beacon */
#endif

	*pFrameLen = FrameLen;
}
VOID updateBeaconRoutineCase(RTMP_ADAPTER *pAd, BOOLEAN UpdateAfterTim)
{
	INT     i;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN FlgQloadIsAlarmIssued = FALSE;
	UCHAR cfg_ht_bw;
	UCHAR cfg_ext_cha;
	UCHAR op_ht_bw;
	UCHAR op_ext_cha;
#ifdef MBSS_DTIM_SUPPORT
	UINT bssidx;
	UCHAR minDtimCount = pAd->ApCfg.MBSSID[0].DtimCount;
#endif

	wdev = get_default_wdev(pAd);
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev = NULL\n");
		return;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	cfg_ext_cha = wlan_config_get_ext_cha(wdev);
	op_ht_bw = wlan_operate_get_ht_bw(wdev);
	op_ext_cha = wlan_operate_get_ext_cha(wdev);

#ifdef MBSS_DTIM_SUPPORT
	for (bssidx = 0; bssidx < pAd->ApCfg.BssidNum; bssidx++) {
		if (pAd->ApCfg.MBSSID[bssidx].DtimCount == 0)
			pAd->ApCfg.MBSSID[bssidx].DtimCount = pAd->ApCfg.MBSSID[bssidx].DtimPeriod - 1;
		else
			pAd->ApCfg.MBSSID[bssidx].DtimCount -= 1;

		if (pAd->ApCfg.MBSSID[bssidx].DtimCount < minDtimCount)
			minDtimCount = pAd->ApCfg.MBSSID[bssidx].DtimCount;
	}
#else
	if (pAd->ApCfg.DtimCount == 0)
		pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
	else
		pAd->ApCfg.DtimCount -= 1;
#endif

#ifdef AP_QLOAD_SUPPORT
	FlgQloadIsAlarmIssued = QBSS_LoadIsAlarmIssued(pAd);
#endif /* AP_QLOAD_SUPPORT */

	if (
#ifdef MBSS_DTIM_SUPPORT
		(minDtimCount == 0)
#else
		(pAd->ApCfg.DtimCount == 0)
#endif
		&& (((pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_SYNC) &&
		  (pAd->CommonCfg.bForty_Mhz_Intolerant == FALSE)) ||
		 (FlgQloadIsAlarmIssued == TRUE))) {
		UCHAR   prevBW, prevExtChOffset;

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "DTIM Period reached, BSS20WidthReq=%d, Intolerant40=%d!\n",
				  pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq,
				  pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40);
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);
		prevBW = wlan_operate_get_ht_bw(wdev);
		prevExtChOffset = wlan_operate_get_ext_cha(wdev);

		if (pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq ||
			pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40 ||
			(pAd->MacTab.fAnyStaFortyIntolerant == TRUE) ||
			(FlgQloadIsAlarmIssued == TRUE)) {
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);


		} else{
			wlan_operate_set_ht_bw(wdev, cfg_ht_bw, cfg_ext_cha);


		}
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "\tNow RecomWidth=%d, ExtChanOffset=%d, prevBW=%d, prevExtOffset=%d\n",
				  wlan_operate_get_ht_bw(wdev),
				  wlan_operate_get_ext_cha(wdev),
				  prevBW, prevExtChOffset);
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_NOTIFY;
	}

#endif /* CONFIG_AP_SUPPORT */

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL)
			MakeBeacon(pAd, wdev, UpdateAfterTim);
	}
}

VOID UpdateBeaconHandler(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BCN_UPDATE_REASON reason)
{
	struct DOT11_H *pDot11h = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR, "wdev = NULL, (caller:%pS)\n",
				 OS_TRACE);
		return;
	}


	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			 "wdev(%d) devname:%s, reason = %d (caller:%pS)\n",
			 wdev->wdev_idx, wdev->if_dev->name, reason, OS_TRACE);

	if (!WDEV_WITH_BCN_ABILITY(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) beacon needless (type:%d, caller:%pS)\n",
				 wdev->wdev_idx, wdev->wdev_type, OS_TRACE);
		goto end;
	}

	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "wdev(%d) bss not ready (state:%d, caller:%pS)!!\n",
				 wdev->wdev_idx, WDEV_BSS_STATE(wdev), OS_TRACE);
		goto end;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h) {
		/* ignore non-CSA beacon update during CSA counting period */
		if ((pDot11h->csa_ap_bitmap != 0) && (reason != BCN_UPDATE_CSA)) {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					"wdev(%d) CSA counting, ignore!! (caller:%pS)!!\n",
					wdev->wdev_idx, OS_TRACE);
			goto end;
		}
	}

	if (reason == BCN_UPDATE_INIT) {
		UCHAR bandidx = HcGetBandByWdev(wdev);
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo[bandidx];

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE, "BCN_UPDATE_INIT, OmacIdx = %x (%s)\n",
				 wdev->OmacIdx, wdev->if_dev->name);

		if (bcn_buf_init(pAd, wdev) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "bcn_buf_init fail!!\n");
			goto end;
		}

		wdev->bcn_buf.bBcnSntReq = TRUE;

		/* record beacon active PeriodicRound */
		if (pBcnCheckInfo->BcnInitedRnd == 0) {
			pBcnCheckInfo->BcnInitedRnd = pAd->Mlme.PeriodicRound;
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
					 "\tBand%d BcnInitedRnd = %ld\n", bandidx, pBcnCheckInfo->BcnInitedRnd);
		}
	} else if (reason == BCN_UPDATE_ENABLE_TX) {
		UCHAR bandidx = HcGetBandByWdev(wdev);
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo[bandidx];

		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE, "BCN_UPDATE_ENABLE_TX, OmacIdx = %x (%s)\n",
				 wdev->OmacIdx, wdev->if_dev->name);

		/* record beacon active PeriodicRound */
		pBcnCheckInfo->BcnInitedRnd = pAd->Mlme.PeriodicRound;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_NOTICE,
				 "\tBand%d BcnInitedRnd = %ld\n", bandidx, pBcnCheckInfo->BcnInitedRnd);
	}

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	{
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
			if (pMbss->APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
				MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO, "StopBeaconing:\n");
				return;
			}
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */



	HW_BEACON_UPDATE(pAd, wdev, (UCHAR)reason);

end:
	return;
}

BOOLEAN UpdateBeaconProc(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine,
	UCHAR UpdatePktType,
	BOOLEAN bMakeBeacon,
	UCHAR UpdateReason)
{
	BCN_BUF_STRUCT *pbcn_buf = NULL;
	BOOLEAN bPauseBcnQ;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
	UCHAR DbdcIdx;
	UINT ifIndex;
#endif
#endif

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 "no wdev\n");
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
	DbdcIdx = HcGetBandByWdev(wdev);
	ifIndex = wdev->func_idx;
	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "error AP index \n");
		return FALSE;
	}

	/* if BSSID is non-transmitted, must do update by transmitted BSSID */
	if (IS_BSSID_11V_NON_TRANS(pAd, &pAd->ApCfg.MBSSID[ifIndex], DbdcIdx)) {
		UCHAR OrigWdevIdx = wdev->wdev_idx;

		ifIndex = pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx];
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "error AP index \n");
			return FALSE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) is Nontransmitted Bssid, update to BssIdx %d wdev(%d)\n",
				  OrigWdevIdx, pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx], wdev->wdev_idx);

		/* make new beacon with this MBSSID's IE add/removed */
		bMakeBeacon = TRUE;
	}
#endif
#endif /* CONFIG_AP_SUPPORT */

	if ((WDEV_BSS_STATE(wdev) < BSS_READY) || !WDEV_WITH_BCN_ABILITY(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) bss not ready (%d) or not required (%d) !!\n",
				 wdev->wdev_idx, WDEV_BSS_STATE(wdev), wdev->wdev_type);
		return FALSE;
	}

	pbcn_buf = &wdev->bcn_buf;

	if (BeaconTransmitRequired(pAd, wdev, UpdateRoutine) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
				 "wdev(%d) NO BeaconTransmitRequired\n", wdev->wdev_idx);

		/* change bcn update function to ensure bcn can disable Normally. */
		if (pbcn_buf->BcnUpdateMethod == BCN_GEN_BY_FW) {
			AsicUpdateBeacon(pAd, wdev, FALSE, UpdateReason);
		}

		return FALSE;
	}

	/* CSA count down start */
	if (wdev->csa_count != 0) {
		struct DOT11_H *pDot11h = wdev->pDot11_H;
		if (pDot11h != NULL) {
			pDot11h->csa_ap_bitmap |= (UINT32)(1 << wdev->func_idx);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
					 "wdev(%d) csa_ap_bitmap = 0x%x\n",
					 wdev->wdev_idx, pDot11h->csa_ap_bitmap);
		} else {
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
					 "This interface(wdev) is down !!!\n");
			return FALSE;
		}
	}

	/* if Beacon offload, FW will help to pause BcnQ */
	bPauseBcnQ = (pbcn_buf->BcnUpdateMethod != BCN_GEN_BY_FW) ? TRUE : FALSE;

	if (bPauseBcnQ)
		AsicDisableBeacon(pAd, wdev);

	if (bMakeBeacon)
		pbcn_buf->FrameLen = MakeBeacon(pAd, wdev, UpdateRoutine);

#ifdef CONFIG_6G_SUPPORT
	ap_6g_build_discovery_frame(pAd, wdev);
#endif

	/* set Beacon to Asic/Mcu */
	AsicUpdateBeacon(pAd, wdev, TRUE, UpdateReason);

	if (bPauseBcnQ)
		AsicEnableBeacon(pAd, wdev);

	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
INT BcnTimUpdate(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *ptr)
{
	INT tim_len = 0, i;
	UCHAR ID_1B, TimFirst, TimLast, *pTim;
	/* BSS_STRUCT *pMbss = wdev->func_dev; */
	BCN_BUF_STRUCT *bcn_buf = &wdev->bcn_buf;
	*ptr = IE_TIM;

#ifdef MBSS_DTIM_SUPPORT
	*(ptr + 2) = pAd->ApCfg.MBSSID[wdev->func_idx].DtimCount;
	*(ptr + 3) = pAd->ApCfg.MBSSID[wdev->func_idx].DtimPeriod;
#else
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;
#endif

	/* find the smallest AID (PS mode) */
	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */
	pTim = bcn_buf->TimBitmaps;

	for (ID_1B = 0; ID_1B < WLAN_MAX_NUM_OF_TIM; ID_1B++) {
		/* get the TIM indicating PS packets for 8 stations */
		UCHAR tim_1B = pTim[ID_1B];

		if (ID_1B == 0)
			tim_1B &= 0xfe; /* skip bit0 bc/mc */

		if (tim_1B == 0)
			continue; /* find next 1B */

		if (TimFirst == 0)
			TimFirst = ID_1B;

		TimLast = ID_1B;
	}

	/* fill TIM content to beacon buffer */
	if (TimFirst & 0x01)
		TimFirst--; /* find the even offset byte */

	*(ptr + 1) = 3 + (TimLast - TimFirst + 1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	for (i = TimFirst; i <= TimLast; i++)
		*(ptr + 5 + i - TimFirst) = pTim[i];

	/* bit0 means backlogged mcast/bcast */
#ifdef MBSS_DTIM_SUPPORT
	if (pAd->ApCfg.MBSSID[wdev->func_idx].DtimCount == 0)
#else
	if (pAd->ApCfg.DtimCount == 0)
#endif
		*(ptr + 4) |= (bcn_buf->TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);

	/* adjust BEACON length according to the new TIM */
	tim_len = (2 + *(ptr + 1));
	return tim_len;
}
#endif

ULONG ComposeBcnPktHead(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pBeaconFrame)
{
	ULONG FrameLen = 0;
	ULONG TmpLen;
	UCHAR DsLen = 1, SsidLen = 0;
	HEADER_802_11 BcnHdr;
	LARGE_INTEGER FakeTimestamp;
	USHORT PhyMode;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;
#endif /* CONFIG_AP_SUPPORT */
	/* INT apidx = wdev->func_idx; */
	UCHAR *Addr2 = NULL, *Addr3 = NULL, *pSsid = NULL;
	USHORT CapabilityInfo, *pCapabilityInfo = &CapabilityInfo;
	BOOLEAN ess = FALSE;
#ifdef CONFIG_STA_SUPPORT
	UCHAR IbssLen = 2;
	BOOLEAN Privacy, ibss = FALSE, need_ibss_ie = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	struct legacy_rate *rate;
	UCHAR Channel;
	UCHAR DefaultAddr[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (wdev == NULL)
		return FALSE;

	PhyMode = wdev->PhyMode;
#ifdef ZERO_LOSS_CSA_SUPPORT
	/*fix channel in ds param, during csa, Channel = wdev->channel;*/
	if (pAd->Zero_Loss_Enable)
		Channel = wlan_operate_get_prim_ch(wdev);
	else
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	Channel = wdev->channel;
	Addr2 = Addr3 = pSsid = DefaultAddr;
#ifdef CONFIG_AP_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		pMbss = wdev->func_dev;

		if (pMbss == NULL)
			return FALSE;

		SsidLen = (pMbss->bHideSsid) ? 0 : pMbss->SsidLen;
		Addr2 = wdev->if_addr;
		Addr3 = wdev->bssid;
		pSsid = pMbss->Ssid;
		ess = TRUE;
		pCapabilityInfo = &pMbss->CapabilityInfo;

#ifdef ZERO_LOSS_CSA_SUPPORT
		/*fix channel in ds param, during csa, Channel = wdev->channel;*/
		if (!(pAd->Zero_Loss_Enable)) {
#endif /*ZERO_LOSS_CSA_SUPPORT*/
		/*for 802.11H in Switch mode should take current channel*/
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			return FALSE;
		if (pAd->CommonCfg.bIEEE80211H == TRUE && pDot11h->RDMode == RD_SWITCHING_MODE)
			Channel = (pDot11h->org_ch != 0) ? pDot11h->org_ch : Channel;
#ifdef ZERO_LOSS_CSA_SUPPORT
		}
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	}

#endif
#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if (pStaCfg == NULL)
			return FALSE;

		SsidLen = pStaCfg->SsidLen;
		Addr2 = wdev->if_addr;
		Addr3 = pStaCfg->Bssid;
		pSsid = pStaCfg->Ssid;
		ibss = TRUE;
		need_ibss_ie = TRUE;
		Privacy = IS_SECURITY(&wdev->SecConfig);
		CapabilityInfo = CAP_GENERATE(
							 ess, ibss, Privacy,
							 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
							 FALSE, FALSE);
		/*TODO: Carter, I think the capability of Adhoc could be decided in earlier,
			why it is decided here?? Whose masterpiece?
		*/
	}

#endif
	MgtMacHeaderInit(pAd,
					 &BcnHdr,
					 SUBTYPE_BEACON,
					 0,
					 BROADCAST_ADDR,
					 Addr2,
					 Addr3);
	MakeOutgoingFrame(
		pBeaconFrame,           &FrameLen,
		sizeof(HEADER_802_11),  &BcnHdr,
		TIMESTAMP_LEN,          &FakeTimestamp,
		2,                      &pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
		2,                      pCapabilityInfo,
		1,                      &SsidIe,
		1,                      &SsidLen,
		SsidLen,                pSsid,
		END_OF_ARGS);
	/*
	  if wdev is AP, SupRateLen is global setting,
	  shall check each's wdev setting to update SupportedRate.
	*/
	rate = &wdev->rate.legacy_rate;

	FrameLen += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, pBeaconFrame + FrameLen);
	TmpLen = 0;

	if (!WMODE_CAP_AX_6G(wdev->PhyMode)) {
		MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TmpLen,
						  1,                              &DsIe,
						  1,                              &DsLen,
						  1,                              &Channel,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (!pStaCfg)
			pStaCfg = GetStaCfgByWdev(pAd, wdev);

		if (need_ibss_ie == TRUE) {
			TmpLen = 0;
			MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TmpLen,
							  1,                              &IbssIe,
							  1,                              &IbssLen,
							  2,                              &pStaCfg->StaActive.AtimWin,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
#endif
	return FrameLen;
}

#ifdef CONFIG_AP_SUPPORT
static BOOLEAN is_beacon_active(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	UINT32 index;
	struct wifi_dev *wdev;
	BOOLEAN bcnactive = FALSE;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) ||
		RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
		return FALSE;

#ifdef ERR_RECOVERY
	if (IsErrRecoveryInIdleStat(pAd) == FALSE)
		return FALSE;
#endif

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.bOverlapScanning)
		return FALSE;
#endif
#endif

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return FALSE;
#endif

#ifdef MT_DFS_SUPPORT
	if (pAd->Dot11_H[BandIdx].RDMode != RD_NORMAL_MODE)
		return FALSE;
#endif

#ifdef WDS_SUPPORT
	/*
	 * WDS is bound on main wifi dev which should not issue Beacons
	 * when system operates as bridge mode
	 */
	if (pAd->WdsTab.Mode[BandIdx] == WDS_BRIDGE_MODE)
		return FALSE;
#endif/* WDS_SUPPORT */

	for (index = 0; index < WDEV_NUM_MAX; index++) {
		wdev = pAd->wdev_list[index];
		if (wdev == NULL)
			continue;

		if (HcIsRadioAcq(wdev)
			&& (!IsHcRadioCurStatOffByWdev(wdev))
			&& (HcGetBandByWdev(wdev) == BandIdx)
			&& (WDEV_BSS_STATE(wdev) == BSS_READY)
			&& (wdev->bcn_buf.bBcnSntReq)) {
			bcnactive = TRUE;
			break;
		}
	}

	return bcnactive;
}

#define BCN_CHECK_PERIOD		50 /* 5s */
#define PRE_BCN_CHECK_PERIOD	25 /* 2.5s */

VOID BcnCheck(RTMP_ADAPTER *pAd)
{
	UCHAR bandidx;
	ULONG PeriodicRound = pAd->Mlme.PeriodicRound;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if ((PeriodicRound % PRE_BCN_CHECK_PERIOD) == 0) {
		for (bandidx = 0; bandidx < HcGetAmountOfBand(pAd) ; bandidx++) {
			PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo[bandidx];
			UINT32 *nobcncnt, *prebcncnt, *totalbcncnt;
			UINT32 bcn_cnt = 0;
			UINT32 recoverext = 0;

			if (is_beacon_active(pAd, bandidx) == FALSE)
				continue;

			/* start checking after a while (5s) to avoid nobcn false alarm */
			if (PeriodicRound < (pBcnCheckInfo->BcnInitedRnd + BCN_CHECK_PERIOD)) {
				MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
						 ("%s start after %ld00 ms (%s)\n", __func__,
						 (pBcnCheckInfo->BcnInitedRnd + BCN_CHECK_PERIOD) - PeriodicRound,
						 RtmpOsGetNetDevName(pAd->net_dev)));
				continue;
			}

			nobcncnt = &pBcnCheckInfo->nobcncnt;
			prebcncnt = &pBcnCheckInfo->prebcncnt;
			totalbcncnt = &pBcnCheckInfo->totalbcncnt;

			bcn_cnt = asic_get_bcn_tx_cnt(pAd, bandidx);
			*totalbcncnt += bcn_cnt;	/* Save total bcn count for MibInfo query */

			if ((PeriodicRound % BCN_CHECK_PERIOD) == 0) {
				bcn_cnt += *prebcncnt;
				*prebcncnt = 0;
			} else {
				*prebcncnt = bcn_cnt;
				continue;
			}

			if (bcn_cnt == 0) {
				(*nobcncnt)++;

				if (*nobcncnt > 4) {
					if (*nobcncnt % 6 == 0) /* 6*5=30s */
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
								 "nobcn still occur within %d sec for band %d (%s)!!\n",
								  (*nobcncnt) * 5, bandidx,
								  RtmpOsGetNetDevName(pAd->net_dev));

					if (*nobcncnt == 5) {
#ifdef WIFI_UNIFIED_COMMAND
						if (cap->uni_cmd_support)
							MtUniCmdFwLog2Host(pAd, HOST2N9, ENUM_CMD_FW_LOG_2_HOST_CTRL_OFF);
						else
#endif /* WIFI_UNIFIED_COMMAND */
							MtCmdFwLog2Host(pAd, 0, 0);
					}

					continue;
				}
			} else if (*nobcncnt != 0) {
				recoverext = 1;
				*nobcncnt = 0;
			} else {
				*nobcncnt = 0;
				continue;
			}

			if ((*nobcncnt != 0 || recoverext == 1) && DebugLevel >= DBG_LVL_ERROR) {
				if (recoverext == 1) {
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_WARN,
							 "bcn recover for band %d (%s)!!\n",
							  bandidx, RtmpOsGetNetDevName(pAd->net_dev));
				} else {
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
							 "nobcn occurs within %d sec for band %d (%s)!!\n",
							  (*nobcncnt) * 5, bandidx, RtmpOsGetNetDevName(pAd->net_dev));
				}

			}
		}
	}
}
#endif

