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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"

BOOLEAN is_testmode_wdev(UINT32 wdev_type)
{
	BOOLEAN ret = FALSE;

	if (wdev_type == WDEV_TYPE_ATE_AP ||
		wdev_type == WDEV_TYPE_ATE_STA ||
		wdev_type == WDEV_TYPE_SERVICE_TXC ||
		wdev_type == WDEV_TYPE_SERVICE_TXD)
		ret = TRUE;

	return ret;
}

/*define extern function*/
INT wdev_edca_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	EDCA_PARM *edca;

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
#ifdef WDS_SUPPORT
	case WDEV_TYPE_WDS:
#endif /*WDS_SUPPORT*/
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT

	case WDEV_TYPE_STA: {
		struct _STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(ad, wdev);

		edca = &sta_cfg->MlmeAux.APEdcaParm;
	}
	break;
#endif /*CONFIG_STA_SUPPORT*/

	default:
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
	}

	HcAcquiredEdca(ad, wdev, edca);
	return TRUE;
}

/*define global function*/
struct wifi_dev *get_default_wdev(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(ad->OpMode) {
		return &ad->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(ad->OpMode) {
		return &ad->StaCfg[MAIN_MSTA_ID].wdev;
	}
#endif
	return NULL;
}

/*define local function*/
INT wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("unregister wdev(type:%d, idx:%d) from wdev_list\n",
					  wdev->wdev_type, wdev->wdev_idx));
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
				  wdev, wdev->wdev_type, wdev->wdev_idx));
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump wdev_list:\n"));

		for (idx = 0; idx < WDEV_NUM_MAX; idx++)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]));
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? 0 : -1);
}

#ifdef MAC_REPEATER_SUPPORT
/*
	Description:
	for record Rx Pkt's wlanIdx, TID, Seq.
	it is used for checking if there is the same A2 send to different A1.
	according the record. trigger the scoreboard update.
*/
static VOID RxTrackingInit(struct wifi_dev *wdev)
{
	UCHAR j;
	RX_TRACKING_T *pTracking = NULL;
	RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;

	pTracking = &wdev->rx_tracking;
	pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

	pTracking->TriggerNum = 0;

	pTaTidSeqMapEntry->RxDWlanIdx = 0xff;
	pTaTidSeqMapEntry->MuarIdx = 0xff;
	for (j = 0; j < 8; j++) {
		pTaTidSeqMapEntry->TID_SEQ[j] = 0xffff;
	}
	pTaTidSeqMapEntry->LatestTID = 0xff;
}
#endif

INT32 wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("wdev(type:%d) already registered and idx(%d) %smatch\n",
					  wdev->wdev_type, wdev->wdev_idx,
					  ((idx != wdev->wdev_idx) ? "mis" : "")));
			break;
		}

		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
#ifdef MAC_REPEATER_SUPPORT
			RxTrackingInit(wdev);
#endif /* MAC_REPEATER_SUPPORT */
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Assign wdev_idx=%d with OmacIdx = %d\n",
					 __func__,
					 idx,
					 wdev->OmacIdx));
			break;
		}
	}

	wdev->wdev_idx = idx;

	if (idx < WDEV_NUM_MAX)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Assign wdev_idx=%d\n", idx));

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


static INT32 GetBssIdx(RTMP_ADAPTER *pAd)
{
	UINT32 BssInfoIdxBitMap;
	UCHAR i;
	INT32 no_usable_entry = -1;
#ifdef MAC_REPEATER_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UCHAR BssInfoMax = BSSINFO_NUM_MAX(cap);

	NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);
	BssInfoIdxBitMap = pAd->BssInfoIdxBitMap0;

	for (i = 0; i < 32; i++) {
		/* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
		if ((BssInfoIdxBitMap & (1 << i)) == 0) {
			pAd->BssInfoIdxBitMap0 = (BssInfoIdxBitMap | (1 << i));
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: found non-used BssInfoIdx: %d\n", __func__, i));
			NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
			return i;
		}
	}

	BssInfoIdxBitMap = pAd->BssInfoIdxBitMap1;

	for (i = 32; i < BssInfoMax; i++) {
		/* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
		if ((BssInfoIdxBitMap & (1 << (i - 32))) == 0) {
			pAd->BssInfoIdxBitMap1 = (BssInfoIdxBitMap | (1 << (i - 32)));
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: found non-used BssInfoIdx: %d\n", __func__, i));
			NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
			return i;
		}
	}

	NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);

	if (i >= BssInfoMax) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: could not find usable BssInfoIdx\n", __func__));
		return no_usable_entry;
	}

	return no_usable_entry;
}

VOID ReleaseBssIdx(RTMP_ADAPTER *pAd, UINT32 BssIdx)
{
	NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);

	if (BssIdx < 32)
		pAd->BssInfoIdxBitMap0 = pAd->BssInfoIdxBitMap0 & (0xffffffff & ~(1 << BssIdx));
	else
		pAd->BssInfoIdxBitMap1 = pAd->BssInfoIdxBitMap1 & (0xffffffff & ~(1 << (BssIdx - 32)));

	NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
}

/*
*
*/
VOID BssInfoArgumentLink(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	HTTRANSMIT_SETTING HTPhyMode;
#ifdef CONFIG_STA_SUPPORT
	struct _STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(ad, wdev);
#endif /*CONFIG_STA_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = wdev->func_dev;
#endif

	bssinfo->OwnMacIdx = wdev->OmacIdx;
	bssinfo->ucBandIdx = wdev->DevInfo.BandIdx;
	bssinfo->ucBssIndex = GetBssIdx(ad);
	os_move_mem(bssinfo->Bssid, wdev->bssid, MAC_ADDR_LEN);
	bssinfo->CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
	bssinfo->ucPhyMode = wdev->PhyMode;
	hc_radio_query_by_wdev(wdev, &bssinfo->chan_oper);

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_STA;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
#ifdef CONFIG_KEEP_ALIVE_OFFLOAD
		/* STA LP offload */
		bssinfo->rBssInfoPm.ucKeepAliveEn = TRUE;
		bssinfo->rBssInfoPm.ucKeepAlivePeriod = KEEP_ALIVE_INTERVAL_IN_SEC;
#endif
		bssinfo->rBssInfoPm.ucBeaconLossReportEn = TRUE;
		bssinfo->rBssInfoPm.ucBeaconLossCount = BEACON_OFFLOAD_LOST_TIME;
#ifdef	CONFIG_STA_SUPPORT
		if (sta_cfg) {
			bssinfo->bcn_period = sta_cfg->MlmeAux.BeaconPeriod;
			bssinfo->dtim_period = sta_cfg->MlmeAux.DtimPeriod;
			bssinfo->dbm_to_roam = sta_cfg->dBmToRoam;
#ifdef DOT11V_MBSSID_SUPPORT
			bssinfo->max_bssid_indicator =
				sta_cfg->MlmeAux.max_bssid_indicator;
			bssinfo->mbssid_index = sta_cfg->MlmeAux.mbssid_index;
#endif
		}
#ifdef UAPSD_SUPPORT
		uapsd_config_get(wdev, &bssinfo->uapsd_cfg);
#endif /*UAPSD_SUPPORT*/
#endif /*CONFIG_STA_SUPPORT*/
		break;

	case WDEV_TYPE_ADHOC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_IBSS;
		bssinfo->u4ConnectionType = CONNECTION_IBSS_ADHOC;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev);
		break;

	case WDEV_TYPE_WDS:
		bssinfo->bssinfo_type = WDS;
		bssinfo->NetworkType = NETWORK_WDS;
		bssinfo->u4ConnectionType = CONNECTION_WDS;
		break;

	case WDEV_TYPE_GO:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GO;
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
		MgmtTableSetMcastEntry(ad, bssinfo->bmc_wlan_idx);
		break;

	case WDEV_TYPE_GC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_P2P;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GC;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev);
		break;

	case WDEV_TYPE_AP:
	default:
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->bmc_wlan_idx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->bmc_wlan_idx, wdev);
		MgmtTableSetMcastEntry(ad, bssinfo->bmc_wlan_idx);
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_AP;
#ifdef CONFIG_AP_SUPPORT
		bssinfo->bcn_period = ad->CommonCfg.BeaconPeriod;

		if (pMbss) {
#ifdef DOT11V_MBSSID_SUPPORT
			UCHAR DbdcIdx = HcGetBandByWdev(&pMbss->wdev);

			if (IS_BSSID_11V_TRANSMITTED(ad, pMbss, DbdcIdx) ||
				IS_BSSID_11V_NON_TRANS(ad, pMbss, DbdcIdx)) {
				bssinfo->max_bssid_indicator = ad->ApCfg.dot11v_max_bssid_indicator[DbdcIdx];
				bssinfo->mbssid_index = pMbss->mbss_grp_idx;
			}
#endif
			bssinfo->dtim_period = pMbss->DtimPeriod;
		}
#endif /*CONFIG_AP_SUPPORT*/
		break;
	}

	wdev_edca_acquire(ad, wdev);
	bssinfo->WmmIdx = HcGetWmmIdx(ad, wdev);
	/* Get a specific Tx rate for BMcast frame */
	os_zero_mem(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));

	if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
		(wdev->channel <= 14)
#ifdef GN_MIXMODE_SUPPORT
		&& (!(ad->CommonCfg.GNMixMode))
#endif /*GN_MIXMODE_SUPPORT*/
	    ) {
		HTPhyMode.field.MODE = MODE_CCK;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = RATE_1;
	} else {
		HTPhyMode.field.MODE = MODE_OFDM;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = MCS_RATE_6;
	}

#ifdef MCAST_RATE_SPECIFIC
	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		bssinfo->McTransmit = (wdev->channel > 14) ? (wdev->rate.MCastPhyMode_5G) : (wdev->rate.MCastPhyMode);
		bssinfo->BcTransmit = (wdev->channel > 14) ? (wdev->rate.MCastPhyMode_5G) : (wdev->rate.MCastPhyMode);
#else
		bssinfo->McTransmit = wdev->rate.mcastphymode;
		bssinfo->BcTransmit = wdev->rate.mcastphymode;
#endif

		if ((wdev->channel > 14)
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		&& (wdev->rate.MCastPhyMode_5G.field.MODE == MODE_CCK)
#else
		&& (wdev->rate.mcastphymode.field.MODE == MODE_CCK)
#endif
		) {
			bssinfo->McTransmit = HTPhyMode;
			bssinfo->BcTransmit = HTPhyMode;
		}
	} else {
		bssinfo->McTransmit = HTPhyMode;
		bssinfo->BcTransmit = HTPhyMode;
	}
#else
	bssinfo->McTransmit = HTPhyMode;
	bssinfo->BcTransmit = HTPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	raWrapperConfigSet(ad, wdev, &bssinfo->ra_cfg);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

#ifdef DOT11_HE_AX
	fill_bssinfo_he(wdev, bssinfo);
#endif /*DOT11_HE_AX*/

#ifdef BCN_PROTECTION_SUPPORT
	os_move_mem(&bssinfo->bcn_prot_cfg, &wdev->SecConfig.bcn_prot_cfg, sizeof(struct bcn_protection_cfg));
#endif
	bssinfo->bss_state = BSS_INITED;
}


VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;

	ReleaseBssIdx(pAd, bss->ucBssIndex);
	HcReleaseGroupKeyWcid(pAd, wdev, bss->bmc_wlan_idx);
	HcReleaseEdca(pAd, wdev);
#ifdef DOT11_HE_AX
	hc_bcolor_release(wdev, bss->bss_color.color);
#endif
	os_zero_mem(bss, sizeof(wdev->bss_info_argument));
	WDEV_BSS_STATE(wdev) = BSS_INIT;
}

INT wdev_ops_register(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
					  struct wifi_dev_ops *ops, UCHAR wmm_detect_method)
{
	wdev->wdev_ops = ops;

	if (wmm_detect_method == WMM_DETECT_METHOD1)
		ops->detect_wmm_traffic = mt_detect_wmm_traffic;
	else if (wmm_detect_method == WMM_DETECT_METHOD2)
		ops->detect_wmm_traffic = detect_wmm_traffic;

	/* register wifi mlme callback function */
	wifi_mlme_ops_register(wdev);
	return TRUE;
}

/**
 * @pAd
 * @wdev wifi device
 * @wdev_type wifi device type
 * @IfDev pointer to interface NET_DEV
 * @func_idx  _STA_TR_ENTRY index for BC/MC packet
 * @func_dev function device
 * @sys_handle pointer to pAd
 *
 * Initialize a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	INT32 wdev_idx = 0;

	wdev->wdev_type = wdev_type;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = WCID_INVALID;
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;
	wdev->bAllowBeaconing = FALSE;
	wdev->radio_off_req = FALSE;
	wdev_idx = wdev_idx_reg(pAd, wdev);

	init_vie_ctrl(wdev);

	if (wdev_idx < 0)
		return FALSE;

	if (wdev_type != WDEV_TYPE_REPEATER)
		hc_obj_init(wdev, wdev_idx);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(caller:%pS), wdev(%d)\n",
			 __func__, OS_TRACE, wdev->wdev_idx));

	return TRUE;
}

VOID wdev_init_for_bound_wdev(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	wdev->wdev_type = wdev_type;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = WCID_INVALID;
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;
	wdev->bAllowBeaconing = FALSE;
	wdev->radio_off_req = FALSE;
}


INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	/* acquire band info that will used for mac address calculating */
	HcAcquireRadioForWdev(pAd, wdev);

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
	case WDEV_TYPE_GO:
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_AP);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): wdevId%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, wdev->wdev_idx, PRINT_MAC(wdev->if_addr)));

		if (wdev->if_dev) {
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
						   wdev->if_addr, MAC_ADDR_LEN);
		}

		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_STA:
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): wdevId%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
				__func__, wdev->wdev_idx, PRINT_MAC(wdev->if_addr)));

		if (wdev->if_dev)
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev), wdev->if_addr, MAC_ADDR_LEN);
		break;
#endif
	default:
		break;
	}

	return TRUE;
}


/**
 * @param pAd
 * @param wdev wifi device
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(caller:%pS), wdev(%d)\n",
			 __func__, OS_TRACE, wdev->wdev_idx));
	
	deinit_vie_ctrl(wdev);

	if (wdev->wdev_type != WDEV_TYPE_REPEATER) {
		wlan_operate_exit(wdev);
		hc_obj_exit(wdev);
	}
	wdev_idx_unreg(pAd, wdev);
	return TRUE;
}


/**
 * @param pAd
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_config_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			wdev->channel = 0;
			wdev->PhyMode = 0;
		}
	}

	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT

/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *WdevSearchByBssid(RTMP_ADAPTER *pAd, UCHAR *Address)
{
    UINT16 Index;
    struct wifi_dev *wdev;

    NdisAcquireSpinLock(&pAd->WdevListLock);
	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(Address, wdev->bssid)) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: can not find registered wdev\n",
			__func__));
	return NULL;
}
#endif

VOID wdev_fsm_init(struct wifi_dev *wdev)
{
	sync_fsm_ops_init(wdev);
	cntl_state_machine_init(wdev, &wdev->cntl_machine, wdev->cntl_func);
	/* add for multi wdev reuse sync state machine */
	sync_fsm_init_for_wdev((PRTMP_ADAPTER)wdev->sys_handle, wdev, &wdev->sync_machine, wdev->sync_func);
	auth_fsm_init((PRTMP_ADAPTER)wdev->sys_handle, wdev, &wdev->auth_machine, wdev->auth_func);
	assoc_fsm_init((PRTMP_ADAPTER)wdev->sys_handle, wdev, &wdev->assoc_machine, wdev->assoc_func);
}

/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *address)
{
	UINT16 Index;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif
	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(address, wdev->if_addr)) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
#ifdef MAC_REPEATER_SUPPORT

	/* if we cannot found wdev from A2, it might comes from Rept entry.
	 * cause rept must bind the bssid of apcli_link,
	 * search A3(Bssid) to find the corresponding wdev.
	 */
	if (pAd->ApCfg.bMACRepeaterEn) {
		rept_entry = lookup_rept_entry(pAd, address);

		if (rept_entry)
			return &rept_entry->wdev;
	}

#endif
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

struct wifi_dev *wdev_search_by_omac_idx(RTMP_ADAPTER *pAd, UINT8 BssIndex)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (wdev->OmacIdx == BssIndex) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

struct wifi_dev *wdev_search_by_band_omac_idx(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 omac_idx)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (wdev->DevInfo.BandIdx == band_idx && wdev->OmacIdx == omac_idx) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

inline struct wifi_dev *wdev_search_by_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	struct wifi_dev *wdev = NULL;
	UINT32 wdev_idx = RTMP_GET_PACKET_WDEV(pkt);

	wdev = pAd->wdev_list[wdev_idx];

	if (!wdev)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("error: wdev(wdev_idx = %d) is null from pkt\n", wdev_idx));

	return wdev;
}

inline struct wifi_dev *wdev_search_by_idx(RTMP_ADAPTER *pAd, UINT32 idx)
{
	struct wifi_dev *wdev = NULL;

	if (idx < WDEV_NUM_MAX)
		wdev = pAd->wdev_list[idx];

	if (!wdev)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("error: wdev(wdev_idx = %d) is null from idx\n", idx));

	return wdev;
}

inline struct wifi_dev *wdev_search_by_wcid(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct _STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[wcid];
	struct wifi_dev *wdev = NULL;

	if (tr_entry)
		wdev = tr_entry->wdev;
	else {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s: can not a valid wdev by wcid (%u)\n",
				  __func__, wcid));
	}

	return wdev;
}

struct wifi_dev *wdev_search_by_netdev(RTMP_ADAPTER *pAd, VOID *pDev)
{
	UCHAR i = 0;
	struct net_device *pNetDev = (struct net_device *)pDev;
	struct wifi_dev *wdev = NULL;

	if (pNetDev != NULL) {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev = pAd->wdev_list[i];

			if (wdev == NULL)
				continue;

			if (wdev->if_dev == pNetDev)
				return wdev;
		}
	}

	return wdev;
}

UCHAR decide_phy_bw_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel)
{
	int i;
	struct wifi_dev *wdev;
	UCHAR phy_bw = BW_20;
	UCHAR wdev_bw;
	UCHAR rfic;

	if (channel <= 14)
		rfic = RFIC_24GHZ;
	else
		rfic = RFIC_5GHZ;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		/*only when wdev is up & operting init done can join to decision*/
		if (wdev && (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_INVALID) && (rfic & wmode_2_rfic(wdev->PhyMode))) {
			wdev_bw = wlan_operate_get_bw(wdev);

			if (wdev_bw > phy_bw)
				phy_bw = wdev_bw;
		}
	}

	if (rfic == RFIC_24GHZ && phy_bw > BW_40)
		phy_bw = BW_40;

	return phy_bw;
}


void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2)
{
	UCHAR ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR vht_bw;
#endif /*DOT11_VHT_AC*/
	UCHAR ext_cha;
	UCHAR stbc;
	UCHAR ldpc;
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR ba_en;
#ifdef TXBF_SUPPORT
	UCHAR txbf;
#endif

	/*update configure*/
	if (wlan_config_get_ext_cha(dev1) == EXTCHA_NOASSIGN) {
		ext_cha = wlan_config_get_ext_cha(dev2);
		wlan_config_set_ext_cha(dev1, ext_cha);
	}

#ifdef TXBF_SUPPORT
	txbf = wlan_config_get_etxbf(dev2);
	wlan_config_set_etxbf(dev1, txbf);
	txbf = wlan_config_get_itxbf(dev2);
	wlan_config_set_itxbf(dev1, txbf);
#endif
	stbc = wlan_config_get_ht_stbc(dev2);
	wlan_config_set_ht_stbc(dev1, stbc);
	ldpc = wlan_config_get_ht_ldpc(dev2);
	wlan_config_set_ht_ldpc(dev1, ldpc);
	stbc = wlan_config_get_vht_stbc(dev2);
	wlan_config_set_vht_stbc(dev1, stbc);
	ldpc = wlan_config_get_vht_ldpc(dev2);
	wlan_config_set_vht_ldpc(dev1, ldpc);

	ht_bw = wlan_config_get_ht_bw(dev2);
	vht_bw = wlan_config_get_vht_bw(dev2);
	wlan_config_set_ht_bw(dev1, ht_bw);
	wlan_config_set_vht_bw(dev1, vht_bw);
	wlan_config_set_cen_ch_2(dev1, wlan_config_get_cen_ch_2(dev2));

	tx_stream = wlan_config_get_tx_stream(dev2);
	wlan_config_set_tx_stream(dev1, tx_stream);
	rx_stream = wlan_config_get_rx_stream(dev2);
	wlan_config_set_rx_stream(dev1, rx_stream);

	/* HT_BAWinSize */
	wlan_config_set_ba_txrx_wsize(dev1,
		wlan_config_get_ba_tx_wsize(dev2),
		wlan_config_get_ba_rx_wsize(dev2));

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	/* STA/APCLI TWT */
	wlan_config_set_he_twt_support(dev1, wlan_config_get_he_twt_support(dev2));
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	ba_en = wlan_config_get_ba_enable(dev2);
	wlan_config_set_ba_enable(dev1, (ba_en > 0) ? ba_en : 0);
	dev1->channel = dev2->channel;
	wlan_config_set_ch_band(dev1, dev2->PhyMode);
	dev1->bWmmCapable = dev2->bWmmCapable;
	wlan_operate_update_ht_cap(dev1);

#ifdef MCAST_RATE_SPECIFIC
	/* temporary soluation due to WDS interface acutally reference */
	if (dev1->wdev_type == WDEV_TYPE_WDS) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
		if (dev1->channel > 14)
			dev1->rate.MCastPhyMode_5G = dev2->rate.MCastPhyMode_5G;
		else
			dev1->rate.MCastPhyMode = dev2->rate.MCastPhyMode;
#else
		dev1->rate.mcastphymode = dev2->rate.mcastphymode;
#endif
	}
#endif
}


void wdev_sync_prim_ch(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UCHAR i = 0;
	struct wifi_dev *tdev;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = ad->wdev_list[i];

		if (tdev && HcIsRadioAcq(tdev) && (band_idx == HcGetBandByWdev(tdev)))
#ifdef CONFIG_MAP_SUPPORT
		{
#endif
			tdev->channel = wdev->channel;
#ifdef CONFIG_MAP_SUPPORT
			if (tdev->wdev_type == WDEV_TYPE_AP)
				tdev->quick_ch_change = wdev->quick_ch_change;
		}
#endif
		else if ((wdev->wdev_type == WDEV_TYPE_AP) &&
				(tdev != NULL) &&
				(band_idx == HcGetBandByWdev(tdev)) &&
				(tdev->PhyMode == wdev->PhyMode))
			tdev->channel = wdev->channel;

		/* Fix for Apcli linkdown issue when AP interface brinup happens after linkup */
		else if ((wdev->wdev_type == WDEV_TYPE_STA) &&
				(tdev != NULL) &&
				(tdev->wdev_type == WDEV_TYPE_AP) &&
				(tdev->if_up_down_state == 0) &&
				(tdev->PhyMode == wdev->PhyMode))
		{
			tdev->channel = wdev->channel;
#ifdef CONFIG_MAP_SUPPORT
			tdev->quick_ch_change = wdev->quick_ch_change;
#endif
		}
	}
}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
#ifdef DOT11_N_SUPPORT
void wdev_sync_ht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ADD_HTINFO *add_ht_info)
{
	UCHAR mbss_idx = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	BOOLEAN adjustBW = FALSE;
	struct wifi_dev *mbss_wdev = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("[%s] Entry Bw %d Ch %d==> \n", __func__, add_ht_info->RecomWidth, add_ht_info->ExtChanOffset));

	/*Moving all same band Soft AP interfaces to new BW proposed by RootAP */
	for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
		mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		if (mbss_wdev == NULL)
			continue;

		if (HcIsRadioAcq(mbss_wdev) && (band_idx == HcGetBandByWdev(mbss_wdev)))
			/* Same Band */
			adjustBW = TRUE;
		else if ((mbss_wdev->wdev_type == WDEV_TYPE_AP) &&
				(mbss_wdev->if_up_down_state == 0) &&
				(((mbss_wdev->channel <= 14) && (wdev->channel <= 14))))
			/* Different Band */
			adjustBW = TRUE;

		if (adjustBW) {
			wlan_config_set_ht_bw(mbss_wdev,
				add_ht_info->RecomWidth);
			wlan_config_set_ext_cha(mbss_wdev,
				add_ht_info->ExtChanOffset);

			/* Reset for other wdev's */
			adjustBW = FALSE;
		}
	}
}
#endif

#ifdef DOT11_VHT_AC
void wdev_sync_vht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR bw, UINT8 channel)
{
	UCHAR mbss_idx = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	BOOLEAN adjustCh = FALSE, adjustBw = TRUE;
	struct wifi_dev *mbss_wdev;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("[%s] Entry bw %d ch %d==> \n", __func__, bw, channel));

	if (bw >= VHT_BW_160)
		adjustBw = FALSE;

	/*Moving all same band Soft AP interfaces to new BW proposed by RootAP */
	for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
		mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		if (mbss_wdev == NULL)
			continue;

		if (HcIsRadioAcq(mbss_wdev) && (band_idx == HcGetBandByWdev(mbss_wdev)))
			/* Same Band */
			adjustCh = TRUE;
		else if ((mbss_wdev->wdev_type == WDEV_TYPE_AP) &&
				(mbss_wdev->if_up_down_state == 0) &&
				(((mbss_wdev->channel > 14) && (wdev->channel > 14))))
			/* Different Band */
			adjustCh = TRUE;

		if (adjustCh) {
			wlan_operate_set_cen_ch_2(mbss_wdev, channel);

			if (adjustBw)
				wlan_operate_set_vht_bw(mbss_wdev, bw);

			/* Reset for other wdev's */
			adjustCh = FALSE;
		}
	}
}
#endif

BOOLEAN IS_SYNC_BW_POLICY_VALID(struct _RTMP_ADAPTER *pAd, BOOLEAN isHTPolicy, UCHAR policy)
{
	BOOLEAN status = FALSE;

	if (isHTPolicy) {
		if (1 & (pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncHTSupport >> policy))
			status = TRUE;
	} else {
		if (1 & (pAd->ApCfg.ApCliAutoBWRules.minorPolicy.ApCliBWSyncVHTSupport >> policy))
			status = TRUE;
	}

	return status;
}
#endif


VOID wdev_if_up_down(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN if_up_down_state)
{
	if (wdev == NULL)
		return;

	wdev->if_up_down_state = if_up_down_state;
}

void wdev_sync_ch_by_rfic(struct _RTMP_ADAPTER *ad, UCHAR rfic, UCHAR ch)
{
	UCHAR i;
	struct wifi_dev *dev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		dev = ad->wdev_list[i];

		if (dev &&
			(wmode_2_rfic(dev->PhyMode) & rfic) &&
			(dev->channel != ch)
		   )
			dev->channel = ch;
	}
}

/*
* wifi sys operate action, must used in dispatch level task, do not use in irq/tasklet/timer context
*/

INT wdev_do_open(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->open)
		return wdev->wdev_ops->open(wdev);

	return FALSE;
}

INT wdev_do_close(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->close)
		return wdev->wdev_ops->close(wdev);

	return FALSE;
}

INT wdev_do_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->linkup)
		return wdev->wdev_ops->linkup(wdev, entry);

	return FALSE;
}

INT wdev_do_linkdown(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->linkdown)
		return wdev->wdev_ops->linkdown(wdev);

	return FALSE;
}

INT wdev_do_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->conn_act)
		return wdev->wdev_ops->conn_act(wdev, entry);

	return FALSE;
}

INT wdev_do_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->disconn_act)
		return wdev->wdev_ops->disconn_act(wdev, entry);

	return FALSE;
}

