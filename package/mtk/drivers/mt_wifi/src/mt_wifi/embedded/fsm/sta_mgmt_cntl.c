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

	Module Name:
	sta mgmt cntl

*/

#include "rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */

UCHAR CipherSuiteWpaNoneTkip[] = {
	0x00, 0x50, 0xf2, 0x01,	/* oui */
	0x01, 0x00,		/* Version */
	0x00, 0x50, 0xf2, 0x02,	/* Multicast */
	0x01, 0x00,		/* Number of unicast */
	0x00, 0x50, 0xf2, 0x02,	/* unicast */
	0x01, 0x00,		/* number of authentication method */
	0x00, 0x50, 0xf2, 0x00	/* authentication */
};
UCHAR CipherSuiteWpaNoneTkipLen =
	(sizeof(CipherSuiteWpaNoneTkip) / sizeof(UCHAR));

UCHAR CipherSuiteWpaNoneAes[] = {
	0x00, 0x50, 0xf2, 0x01,	/* oui */
	0x01, 0x00,		/* Version */
	0x00, 0x50, 0xf2, 0x04,	/* Multicast */
	0x01, 0x00,		/* Number of unicast */
	0x00, 0x50, 0xf2, 0x04,	/* unicast */
	0x01, 0x00,		/* number of authentication method */
	0x00, 0x50, 0xf2, 0x00	/* authentication */
};
UCHAR CipherSuiteWpaNoneAesLen =
	(sizeof(CipherSuiteWpaNoneAes) / sizeof(UCHAR));

/* The following MACRO is called after 1. starting an new IBSS, 2. succesfully JOIN an IBSS, */
/* or 3. succesfully ASSOCIATE to a BSS, 4. successfully RE_ASSOCIATE to a BSS */
/* All settings successfuly negotiated furing MLME state machines become final settings */
/* and are copied to pStaCfg->StaActive */
#define COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd, _pStaCfg, _pEntry)                                 \
	{                                                                                       \
		NdisZeroMemory((_pStaCfg)->Ssid, MAX_LEN_OF_SSID);							\
		(_pStaCfg)->SsidLen = (_pStaCfg)->MlmeAux.SsidLen;                                \
		NdisMoveMemory((_pStaCfg)->Ssid, (_pStaCfg)->MlmeAux.Ssid, (_pStaCfg)->MlmeAux.SsidLen); \
		COPY_MAC_ADDR((_pStaCfg)->Bssid, (_pStaCfg)->MlmeAux.Bssid);                      \
		(_pStaCfg)->wdev.channel = (_pStaCfg)->MlmeAux.Channel;                                \
		(_pStaCfg)->StaActive.Aid = (_pStaCfg)->MlmeAux.Aid;                                        \
		(_pStaCfg)->StaActive.AtimWin = (_pStaCfg)->MlmeAux.AtimWin;                                \
		(_pStaCfg)->StaActive.CapabilityInfo = (_pStaCfg)->MlmeAux.CapabilityInfo;                  \
		(_pStaCfg)->StaActive.ExtCapInfo = (_pStaCfg)->MlmeAux.ExtCapInfo;                  \
		(_pAd)->CommonCfg.BeaconPeriod[0] = (_pStaCfg)->MlmeAux.BeaconPeriod;                      \
		(_pStaCfg)->StaActive.CfpMaxDuration = (_pStaCfg)->MlmeAux.CfpMaxDuration;                  \
		(_pStaCfg)->StaActive.CfpPeriod = (_pStaCfg)->MlmeAux.CfpPeriod;                            \
		(_pStaCfg)->StaActive.rate.sup_rate_len = (_pStaCfg)->MlmeAux.rate.sup_rate_len;            \
		NdisMoveMemory((_pStaCfg)->StaActive.rate.sup_rate, (_pStaCfg)->MlmeAux.rate.sup_rate, (_pStaCfg)->MlmeAux.rate.sup_rate_len);\
		(_pStaCfg)->StaActive.rate.ext_rate_len = (_pStaCfg)->MlmeAux.rate.ext_rate_len;            \
		NdisMoveMemory((_pStaCfg)->StaActive.rate.ext_rate, (_pStaCfg)->MlmeAux.rate.ext_rate, (_pStaCfg)->MlmeAux.rate.ext_rate_len);\
		NdisMoveMemory(&(_pAd)->CommonCfg.APQosCapability, &(_pStaCfg)->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));\
		NdisMoveMemory(&(_pAd)->CommonCfg.APQbssLoad, &(_pStaCfg)->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));\
		COPY_MAC_ADDR((_pEntry)->Addr, (_pStaCfg)->MlmeAux.Bssid);      \
		(_pEntry)->SecConfig.AKMMap = (_pStaCfg)->AKMMap;\
		(_pEntry)->SecConfig.PairwiseCipher = (_pStaCfg)->PairwiseCipher;\
		COPY_MAC_ADDR((_pEntry)->bssid, (_pStaCfg)->MlmeAux.Bssid);\
		(_pEntry)->RateLen = (_pStaCfg)->StaActive.rate.sup_rate_len + (_pStaCfg)->StaActive.rate.ext_rate_len;\
	}


#define CNTL_GET_STA_CFG(__ad, __wdev, __sta_cfg) \
	{\
		__sta_cfg = GetStaCfgByWdev(__ad, __wdev); \
		ASSERT(__sta_cfg); \
		if (!__sta_cfg) \
			return; \
	}

struct _cntl_api_ops sta_cntl_api_ops;

static VOID JoinParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_JOIN_REQ_STRUCT *JoinReq,
	IN struct wifi_dev *wdev,
	IN ULONG BssIdx);

static VOID StartParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_START_REQ_STRUCT *StartReq,
	IN CHAR Ssid[],
	IN UCHAR SsidLen);

static VOID AuthParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_AUTH_REQ_STRUCT *AuthReq,
	IN PUCHAR pAddr,
	IN USHORT Alg);

static VOID join_iterate_by_cfg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MLME_JOIN_REQ_STRUCT JoinReq;
	PSTA_ADMIN_CONFIG pApCliEntry = GetStaCfgByWdev(pAd, wdev);
	USHORT ifIndex = wdev->func_idx;

#ifdef DOT11W_PMF_SUPPORT
	SCAN_CTRL *ScanCtrl;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
#endif /* DOT11W_PMF_SUPPORT */

	ASSERT(pApCliEntry);
	if (!pApCliEntry)
		return;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Probe Req Timeout.\n", __func__);

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (pApCliEntry->wdev.if_up_down_state == FALSE)
		return;

	if (scan_in_run_state(pAd, wdev) == TRUE) {
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		return;
	}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry->ApcliInfStat.ProbeReqCnt++;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Probe Req Timeout. ProbeReqCnt=%d\n",
			 __func__, pApCliEntry->ApcliInfStat.ProbeReqCnt);

	if (pApCliEntry->ApcliInfStat.ProbeReqCnt > 7) {

#ifdef CONFIG_OWE_SUPPORT
	sta_reset_owe_parameters(pAd, ifIndex);
#endif
		/*
			if exceed the APCLI_MAX_PROBE_RETRY_NUM (7),
			switch to try next candidate AP.
		*/
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pApCliEntry->ApcliInfStat.ProbeReqCnt = 0;
#ifdef DOT11W_PMF_SUPPORT

		/* Driver Trigger New Scan Mode for Sigma DUT usage */
		if ((pApCliEntry->ApCliAutoConnectType == TRIGGER_SCAN_BY_DRIVER
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
		|| (pApCliEntry->ApCliAutoConnectType == TRIGGER_SCAN_BY_USER
		&& pApCliEntry->ApcliInfStat.AutoConnectFlag == TRUE)
#endif
		) && ScanCtrl->PartialScan.bScanning == FALSE
			) {
			if (pApCliEntry->CfgSsidLen) {
				NDIS_802_11_SSID Ssid;

				NdisCopyMemory(Ssid.Ssid, &pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
				Ssid.SsidLength = pApCliEntry->CfgSsidLen;
				NdisZeroMemory(pApCliEntry->CfgApCliBssid, MAC_ADDR_LEN);
				pApCliEntry->ApCliAutoConnectRunning = TRUE;
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pApCliEntry->wdev);
				return;
			}
		}

#endif /* DOT11W_PMF_SUPPORT */

		if ((pApCliEntry->ApCliAutoConnectRunning == TRUE) &&
			(pApCliEntry->ApCliAutoConnectType != TRIGGER_SCAN_BY_DRIVER))
			ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

		return;
	}

#else

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	/* stay in same state. */
	cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
	/* retry Probe Req. */
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Retry Probe Req.\n", __func__);
	NdisZeroMemory(&JoinReq, sizeof(MLME_JOIN_REQ_STRUCT));
		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

#ifdef WSC_AP_SUPPORT

	if ((pAd->StaCfg[ifIndex].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->StaCfg[ifIndex].wdev.WscControl.bWscTrigger == TRUE)) {
		NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
		JoinReq.SsidLen = pApCliEntry->wdev.WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pApCliEntry->wdev.WscControl.WscSsid.Ssid, JoinReq.SsidLen);
	} else
#endif /* WSC_AP_SUPPORT */
	{

#ifdef CONFIG_OWE_SUPPORT
		/*Configure OWE ssid and bssid Join request parameters*/
		if (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap) && (pApCliEntry->owe_trans_ssid_len != 0)) {
			JoinReq.SsidLen = pApCliEntry->owe_trans_ssid_len;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->owe_trans_ssid, JoinReq.SsidLen);
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->owe_trans_bssid);
		} else
#endif
		if (pApCliEntry->CfgSsidLen != 0) {
			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}

#ifdef CONFIG_OWE_SUPPORT
		/*Configure OWE ssid and bssid Join request parameters*/
		if (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap) && (pApCliEntry->owe_trans_ssid_len != 0)) {
			JoinReq.SsidLen = pApCliEntry->owe_trans_ssid_len;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->owe_trans_ssid, JoinReq.SsidLen);
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->owe_trans_bssid);
		} else
#endif
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Probe Ssid=%s, Bssid="MACSTR"\n",
			 __func__, JoinReq.Ssid, MAC2STR(JoinReq.Bssid));
	JoinParmFill(pAd, &JoinReq, wdev, BSS_NOT_FOUND);
	MlmeEnqueueWithWdev(pAd, SYNC_FSM,  SYNC_FSM_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex, wdev);


}

static VOID sta_cntl_disconnect_proc(
	VOID *elem_obj);

static VOID sta_cntl_priv_disconnect(
	STA_ADMIN_CONFIG *sta_cfg,
	enum _CNTL_DISCONNECT_TYPE type,
	USHORT reason)
{
	CNTL_MLME_DISCONNECT_STRUCT disconn_req;
	MLME_QUEUE_ELEM *Elem;

	os_alloc_mem(NULL, (UCHAR **) &Elem, sizeof(MLME_QUEUE_ELEM));

	if (Elem) {
		os_zero_mem(Elem, sizeof(MLME_QUEUE_ELEM));
		disconn_req.cntl_disconn_type = type;
		disconn_req.mlme_disconn.reason = reason;
		os_move_mem(disconn_req.mlme_disconn.addr, sta_cfg->Bssid, MAC_ADDR_LEN);
		Elem->wdev = &sta_cfg->wdev;
		Elem->MsgLen = sizeof(CNTL_MLME_DISCONNECT_STRUCT);
		NdisMoveMemory(Elem->Msg, &disconn_req, Elem->MsgLen);
		sta_cntl_disconnect_proc(Elem);
		os_free_mem(Elem);
	}
}

#ifdef CONFIG_STA_ADHOC_SUPPORT
static VOID sta_cntl_priv_ibss_start(
	struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd;
	RT_PHY_INFO *rt_phy_info;
	ADD_HT_INFO_IE *addht;
	PSTA_ADMIN_CONFIG pStaCfg;
	MAC_TABLE_ENTRY *pEntry;
	struct adhoc_info *adhocInfo;
	UINT link_up_type = 0;
	struct DOT11_H *pDot11h = NULL;

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	adhocInfo = &pStaCfg->adhocInfo;
	addht = wlan_operate_get_addht(wdev);

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	/* */
	/* 5G bands rules of Japan: */
	/* Ad hoc must be disabled in W53(ch52,56,60,64) channels. */
	/* */

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		RadarChannelCheck(pAd, wdev->channel)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - Channel=%d, Start adhoc on W53(52,56,60,64) Channels are not accepted\n",
				  wdev->channel);
		return;
	}

#ifdef DOT11_N_SUPPORT
	rt_phy_info = &pStaCfg->StaActive.SupportedPhyInfo;
	os_zero_mem(&rt_phy_info->MCSSet[0], 16);

	if (WMODE_CAP_N(wdev->PhyMode)
		&& (adhocInfo->bAdhocN == TRUE)
		&& (!pAd->CommonCfg.HT_DisallowTKIP
			|| !IS_INVALID_HT_SECURITY(wdev->SecConfig.PairwiseCipher))) {
		SetCommonHtVht(pAd, wdev);
		pStaCfg->MlmeAux.CentralChannel = get_cent_ch_by_htinfo(pAd,
										  addht,
										  &pAd->CommonCfg.HtCapability);
		os_move_mem(&pStaCfg->MlmeAux.AddHtInfo,
					addht,
					sizeof(ADD_HT_INFO_IE));

		if (pEntry)
			RTMPCheckHt(pAd, pEntry->wcid,
						&pAd->CommonCfg.HtCapability,
						addht);

		rt_phy_info->bHtEnable = TRUE;
		os_move_mem(&rt_phy_info->MCSSet[0],
					&pAd->CommonCfg.HtCapability.MCSSet[0], 16);
		ASSERT(pEntry);

		if (pEntry)
			COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd, pEntry, pStaCfg);

#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(wdev->PhyMode) &&
			HAS_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists)) {
			RT_VHT_CAP *rt_vht_cap = &pStaCfg->StaActive.SupVhtCap;

			COPY_VHT_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);
			rt_vht_cap->vht_bw = BW_80;
			rt_vht_cap->sgi_80m = pStaCfg->MlmeAux.vht_cap.vht_cap.sgi_80M;
			rt_vht_cap->vht_txstbc = pStaCfg->MlmeAux.vht_cap.vht_cap.tx_stbc;
			rt_vht_cap->vht_rxstbc = pStaCfg->MlmeAux.vht_cap.vht_cap.rx_stbc;
			rt_vht_cap->vht_htc = pStaCfg->MlmeAux.vht_cap.vht_cap.htc_vht_cap;
		}

#endif /* DOT11_VHT_AC */
	} else
#endif /* DOT11_N_SUPPORT */
	{
		pStaCfg->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
	}

	adhocInfo->bAdhocCreator = TRUE;
	LinkUp(pAd, BSS_ADHOC, wdev, link_up_type, NULL);
	/* Before send beacon, driver need do radar detection */

	if ((wdev->channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, wdev->channel)) {
		pDot11h->RDMode = RD_SILENCE_MODE;
		pDot11h->RDCount = 0;
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "CNTL - start a new IBSS = "MACSTR" ...\n",
			  MAC2STR(pStaCfg->Bssid));
	RTMPSendWirelessEvent(pAd, IW_START_IBSS_FLAG, NULL, BSS0, 0);
}
#endif /* CONFIG_STA_ADHOC_SUPPORT */

static VOID sta_cntl_connect_by_cfg(
	struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd;
	MLME_JOIN_REQ_STRUCT JoinReq;
	PSTA_ADMIN_CONFIG pApCliEntry;
	USHORT ifIndex = wdev->func_idx;
	PULONG pLinkDownReason;
	UCHAR owner = CH_OP_OWNER_IDLE;
  #ifdef WSC_AP_SUPPORT
	PWSC_CTRL pWpsCtrl = NULL;
#endif /* WSC_AP_SUPPORT */

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	owner = GetCurrentChannelOpOwner(pAd, wdev);
	CNTL_GET_STA_CFG(pAd, wdev, pApCliEntry);
#ifdef WSC_AP_SUPPORT
	pWpsCtrl = &pApCliEntry->wdev.WscControl;
#endif /* WSC_AP_SUPPORT */
	pLinkDownReason = &pApCliEntry->ApcliInfStat.LinkDownReason;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Start Probe Req.\n", __func__);

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if ((owner == CH_OP_OWNER_SCAN) || (owner == CH_OP_OWNER_PARTIAL_SCAN)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, "scan is ongoing, return.\n");
		return;
	}

	if (scan_in_run_state(pAd, wdev) == TRUE)
		return;



	NdisZeroMemory(&JoinReq, sizeof(MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

#ifdef WSC_AP_SUPPORT

	if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
		(pWpsCtrl->bWscTrigger == TRUE)) {
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		ULONG bss_idx = 0;

		NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
		JoinReq.SsidLen = pAd->StaCfg[ifIndex].wdev.WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->StaCfg[ifIndex].wdev.WscControl.WscSsid.Ssid, JoinReq.SsidLen);

		if (pWpsCtrl->WscMode == 1) { /* PIN */
			bss_idx = BssSsidTableSearchBySSID(ScanTab, (PUCHAR)(JoinReq.Ssid), JoinReq.SsidLen);

			if (bss_idx == BSS_NOT_FOUND) {
				ApSiteSurvey_by_wdev(pAd, NULL, SCAN_WSC_ACTIVE, FALSE, wdev);
				return;
			} else {
				INT old_conf_mode = pWpsCtrl->WscConfMode;
				UCHAR channel = wdev->channel, RootApChannel = ScanTab->BssEntry[bss_idx].Channel;

				if (RootApChannel != channel) {
					rtmp_set_channel(pAd, wdev, RootApChannel);

					wdev->channel = RootApChannel;
					/*
						ApStop will call WscStop, we need to reset WscConfMode, WscMode & bWscTrigger here.
					*/
					pWpsCtrl->WscState = WSC_STATE_START;
					pWpsCtrl->WscStatus = STATUS_WSC_START_ASSOC;
					pWpsCtrl->WscMode = 1;
					pWpsCtrl->WscConfMode = old_conf_mode;
					pWpsCtrl->bWscTrigger = TRUE;

					return;
				}
			}
		}
	} else
#endif /* WSC_AP_SUPPORT */
	{

		if (pApCliEntry->CfgSsidLen != 0) {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
			ULONG bss_idx = BSS_NOT_FOUND;
			BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, &pApCliEntry->wdev);

			if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR)) {
				bss_idx = BssTableSearchWithSSID(ScanTab, pApCliEntry->CfgApCliBssid, (PCHAR)pApCliEntry->CfgSsid,
					pApCliEntry->CfgSsidLen, wdev->channel);
			} else {
				bss_idx = BssSsidTableSearchBySSID(ScanTab, (PCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
			}

			if (bss_idx == BSS_NOT_FOUND) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
						 "%s::  can't find SSID[%s] in ScanTab.\n", __func__, pApCliEntry->CfgSsid);
				cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
				CFG80211_checkScanTable(pAd);
#ifdef APCLI_CFG80211_SUPPORT
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, JoinReq.Bssid, ifIndex, NULL, 0, NULL, 0, 0);
				cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
#else
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, JoinReq.Bssid, NULL, 0, NULL, 0, 0);
#endif
				return;
			}

			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s::  find SSID[%ld][%s] channel[%d-%d] in ScanTab.\n", __func__,
					 bss_idx, pApCliEntry->CfgSsid, ScanTab->BssEntry[bss_idx].Channel,
					 pAd->ScanTab.BssEntry[bss_idx].CentralChannel);
			/* TODO */
			/* BssSearch Table has found the pEntry, send Prob Req. directly */
			if (wdev->channel != ScanTab->BssEntry[bss_idx].Channel)
			{
				pApCliEntry->MlmeAux.Channel = ScanTab->BssEntry[bss_idx].Channel;
#ifdef CONFIG_MULTI_CHANNEL
				pApCliEntry->wdev.CentralChannel = pApCliEntry->MlmeAux.Channel;
				/* should be check and update in in asso to check ==> ApCliCheckHt() */
				pApCliEntry->wdev.channel = pApCliEntry->wdev.CentralChannel;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef APCLI_CFG80211_SUPPORT
				rtmp_set_channel(pAd, wdev, pApCliEntry->MlmeAux.Channel);
				pApCliEntry->wdev.channel = pApCliEntry->MlmeAux.Channel;
#else
				wlan_operate_set_prim_ch(&pApCliEntry->wdev, pApCliEntry->wdev.channel);
#endif
			}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}

	if ((JoinReq.SsidLen == 0) && MAC_ADDR_EQUAL(JoinReq.Bssid, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, "(%s) SsidLen=0 & zero mac bssid, return\n",
			__func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "(%s) Probe Ssid=%s, Bssid="MACSTR"\n",
			 __func__, JoinReq.Ssid, MAC2STR(JoinReq.Bssid));

	cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
	JoinParmFill(pAd, &JoinReq, wdev, BSS_NOT_FOUND);
	MlmeEnqueueWithWdev(pAd, SYNC_FSM,  SYNC_FSM_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex, wdev);
}


static VOID sta_cntl_connect_by_ssid(
	struct wifi_dev *wdev,
	UCHAR data_len,
	UCHAR *data)
{
	RTMP_ADAPTER *pAd;
	NDIS_802_11_SSID ssid_info;
	PSTA_ADMIN_CONFIG pStaCfg;
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
	BSS_TABLE *ScanTab = NULL;

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	os_zero_mem(&ssid_info, sizeof(NDIS_802_11_SSID));

	if (data_len != sizeof(NDIS_802_11_SSID)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				" data_len %d != sizeof(NDIS_802_11_SSID)\n"
				 , data_len);
		return;
	}

	os_move_mem(&ssid_info, data, data_len);
#ifdef DOT11R_FT_SUPPORT
	pStaCfg->Dot11RCommInfo.bInMobilityDomain = FALSE;
	pStaCfg->Dot11RCommInfo.FtRspSuccess = 0;
#endif /* DOT11R_FT_SUPPORT */
	/* Step 1. record the desired user settings to MlmeAux */
	os_zero_mem(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
	os_move_mem(pStaCfg->MlmeAux.Ssid, ssid_info.Ssid, ssid_info.SsidLength);
	pStaCfg->MlmeAux.SsidLen = (UCHAR) ssid_info.SsidLength;

	if (pStaCfg->BssType == BSS_INFRA)
		os_zero_mem(pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);

	pStaCfg->MlmeAux.BssType = pStaCfg->BssType;
	pStaCfg->bAutoConnectByBssid = FALSE;
	/*save connect info*/
	os_zero_mem(pStaCfg->ConnectinfoSsid, MAX_LEN_OF_SSID);
	os_move_mem(pStaCfg->ConnectinfoSsid, ssid_info.Ssid, ssid_info.SsidLength);
	pStaCfg->ConnectinfoSsidLen = ssid_info.SsidLength;
	pStaCfg->ConnectinfoBssType = pStaCfg->BssType;
#ifdef WSC_STA_SUPPORT
	/* for M8 */
	os_zero_mem(pStaCfg->Ssid, MAX_LEN_OF_SSID);
	os_move_mem(pStaCfg->Ssid, ssid_info.Ssid, ssid_info.SsidLength);
	pStaCfg->SsidLen = (UCHAR) ssid_info.SsidLength;
#endif /* WSC_STA_SUPPORT */
	/*
		step 2.
		find all matching BSS in the lastest SCAN result (inBssTab)
		and log them into MlmeAux.SsidBssTab for later-on iteration. Sort by RSSI order
	*/
	BssTableSsidSort(pAd, &pStaCfg->wdev, &pStaCfg->MlmeAux.SsidBssTab,
					 (PCHAR) pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s():CNTL - %d BSS of %d BSS match the desire ",
			  __func__, pStaCfg->MlmeAux.SsidBssTab.BssNr, ScanTab->BssNr);

	if (pStaCfg->MlmeAux.SsidLen == MAX_LEN_OF_SSID)
		hex_dump("\nSSID", pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
	else
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(%d)SSID - %s\n", pStaCfg->MlmeAux.SsidLen,
				  pStaCfg->MlmeAux.Ssid);

	if (INFRA_ON(pStaCfg) &&
		STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) &&
		(pStaCfg->SsidLen == pStaCfg->MlmeAux.SsidBssTab.BssEntry[0].SsidLen)
		&& NdisEqualMemory(pStaCfg->Ssid, pStaCfg->MlmeAux.SsidBssTab.BssEntry[0].Ssid, pStaCfg->SsidLen)
		&& MAC_ADDR_EQUAL(pStaCfg->Bssid, pStaCfg->MlmeAux.SsidBssTab.BssEntry[0].Bssid)) {
		/*
			Case 1. already connected with an AP who has the desired SSID
					with highest RSSI
		*/

		/* Add checking Mode "LEAP" for CCX 1.0 */
		if ((IS_AKM_WPA_CAPABILITY_Entry(wdev)
			) &&
			(wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
			/*
				case 1.1 For WPA, WPA-PSK,
				if port is not secured, we have to redo connection process
			*/
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():CNTL - disassociate with current AP...\n",
					  __func__);
			sta_cntl_priv_disconnect(pStaCfg, CNTL_DISASSOC, REASON_DISASSOC_STA_LEAVING);
		} else if (pStaCfg->bConfigChanged == TRUE) {
			/* case 1.2 Important Config has changed, we have to reconnect to the same AP */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():CNTL - disassociate with current AP Because config changed...\n",
					  __func__);
			sta_cntl_priv_disconnect(pStaCfg, CNTL_DISASSOC, REASON_DISASSOC_STA_LEAVING);
		} else {
			/* case 1.3. already connected to the SSID with highest RSSI. */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():CNTL - already with this BSSID. ignore this SET_SSID request\n",
					  __func__);

			/*
				(HCT 12.1) 1c_wlan_mediaevents required
				media connect events are indicated when associating with the same AP
			*/
			if (INFRA_ON(pStaCfg)) {
				/*
					Since MediaState already is NdisMediaStateConnected
					We just indicate the connect event again to meet the WHQL required.
				*/
				RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
				pAd->ExtraInfo = GENERAL_LINK_UP;	/* Update extra information to link is up */
			}

			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev,
									RT_WLAN_EVENT_CGIWAP, -1,
									&pStaCfg->MlmeAux.Bssid[0], NULL,
									0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
		}
	} else if (INFRA_ON(pStaCfg)) {
		/*
			For RT61
			[88888] OID_802_11_SSID should have returned NDTEST_WEP_AP2(Returned: )
			RT61 may lost SSID, and not connect to NDTEST_WEP_AP2 and will connect to NDTEST_WEP_AP2 by Autoreconnect
			But media status is connected, so the SSID not report correctly.
		*/
		if (!SSID_EQUAL(pStaCfg->Ssid, pStaCfg->SsidLen, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen)) {
			/* Different SSID means not Roaming case, so we let LinkDown() to Indicate a disconnect event. */
			pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
		}

		/*
			case 2. active INFRA association existent
			Roaming is done within miniport driver, nothing to do with configuration
			utility. so upon a new SET(OID_802_11_SSID) is received, we just
			disassociate with the current associated AP,
			then perform a new association with this new SSID, no matter the
			new/old SSID are the same or not.
		*/
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s():CNTL - disassociate with current AP...\n",
				  __func__);
		sta_cntl_priv_disconnect(pStaCfg, CNTL_DISASSOC, REASON_DISASSOC_STA_LEAVING);
	} else {
#ifdef CONFIG_STA_ADHOC_SUPPORT
		if (ADHOC_ON(pAd)) {
			UINT link_down_type = 0;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():CNTL - drop current ADHOC\n",
					  __func__);
			LinkDown(pAd, link_down_type, &pStaCfg->wdev, NULL);
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
			pAd->ExtraInfo = GENERAL_LINK_DOWN;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():NDIS_STATUS_MEDIA_DISCONNECT Event C!\n",
					  __func__);
		}
#endif /* CONFIG_STA_ADHOC_SUPPORT */
		if ((pStaCfg->MlmeAux.SsidBssTab.BssNr == 0) &&
			(pStaCfg->bAutoReconnect == TRUE) &&
			(((pStaCfg->MlmeAux.BssType == BSS_INFRA) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			 || ((pStaCfg->MlmeAux.BssType == BSS_ADHOC) && !ScanInfo->bNotFirstScan))
			&& (MlmeValidateSSID(pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen) == TRUE)
		   ) {
			MLME_SCAN_REQ_STRUCT ScanReq;
#ifdef CONFIG_STA_ADHOC_SUPPORT
			if (pStaCfg->MlmeAux.BssType == BSS_ADHOC)
				ScanInfo->bNotFirstScan = TRUE;
#endif /* CONFIG_STA_ADHOC_SUPPORT */

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s():CNTL - No matching BSS, start a new scan\n",
					  __func__);

			if ((pStaCfg->ConnectinfoChannel != 0) && (pStaCfg->Connectinfoflag == TRUE)) {
				ScanParmFill(pAd, &ScanReq, (RTMP_STRING *) pStaCfg->MlmeAux.Ssid,
							 pStaCfg->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
				cntl_scan_request(wdev, &ScanReq);
			} else {
#ifdef WIDI_SUPPORT

				if (pStaCfg->bWIDI) {
					if (pStaCfg->MlmeAux.OldChannel > 0)
						pStaCfg->MlmeAux.Channel = pStaCfg->MlmeAux.OldChannel;

					ScanParmFill(pAd, &ScanReq, (RTMP_STRING *) pStaCfg->MlmeAux.Ssid,
								 pStaCfg->MlmeAux.SsidLen, BSS_ANY, SCAN_PASSIVE);
				} else
#endif /* WIDI_SUPPORT // */
					ScanParmFill(pAd, &ScanReq, (RTMP_STRING *) pStaCfg->MlmeAux.Ssid,
								 pStaCfg->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);

				cntl_scan_request(wdev, &ScanReq);
			}
		} else {
#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT

			/* LED indication. */
			if (pStaCfg->MlmeAux.BssType == BSS_INFRA)
				LEDConnectionStart(pAd);

#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */

			/*
				REGION_33_BG_BAND - 1-14, all active scan, 802.11g/n: ch14 disallowed.
			*/
			if ((pAd->CommonCfg.CountryRegion & 0x7f) == REGION_33_BG_BAND) {
				BSS_ENTRY *entry;

				entry = &pStaCfg->MlmeAux.SsidBssTab.BssEntry[pStaCfg->MlmeAux.BssIdx];

				/*
					Use SavedPhyMode to check phy mode shall be changed or not.
				*/
				if ((entry->Channel == 14) && (pAd->CommonCfg.SavedPhyMode == WMODE_INVALID)) {
					pAd->CommonCfg.SavedPhyMode = wdev->PhyMode;
					RTMPSetPhyMode(pAd, wdev, WMODE_B);
				} else if ((entry->Channel != 14) && pAd->CommonCfg.SavedPhyMode != WMODE_INVALID) {
					RTMPSetPhyMode(pAd, wdev, pAd->CommonCfg.SavedPhyMode);
					pAd->CommonCfg.SavedPhyMode = WMODE_INVALID;
				}
			}

			pStaCfg->MlmeAux.BssIdx = 0;
			IterateOnBssTab(pAd, wdev);
		}

#ifdef RT_CFG80211_SUPPORT

		if ((pStaCfg->MlmeAux.SsidBssTab.BssNr == 0) && (pStaCfg->MlmeAux.BssType == BSS_INFRA)
			&& (pAd->cfg80211_ctrl.FlgCfg80211Connecting == TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CFG80211_MLME: No matching BSS, Report cfg80211_layer SM to Idle --> %ld\n",
					  wdev->cntl_machine.CurrState);
			pAd->cfg80211_ctrl.Cfg_pending_SsidLen = pStaCfg->MlmeAux.SsidLen;
			os_zero_mem(pAd->cfg80211_ctrl.Cfg_pending_Ssid, MAX_LEN_OF_SSID + 1);
			os_move_mem(pAd->cfg80211_ctrl.Cfg_pending_Ssid, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
			RT_CFG80211_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, NULL, 0, NULL, 0, 0);
		}

#endif /* RT_CFG80211_SUPPORT */
	}

	/* move AutoReconnectSsid update here, due to it's cleared by previous sta_cntl_priv_disconnect  */
	/* Update Reconnect Ssid, that user desired to connect. */
	os_zero_mem(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
	os_move_mem(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.Ssid,
				pStaCfg->MlmeAux.SsidLen);
	pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->MlmeAux.SsidLen;
}

static VOID sta_cntl_connect_by_bssid(
	struct wifi_dev *wdev,
	UCHAR *data)
{
	RTMP_ADAPTER *pAd;
	ULONG BssIdx;
	MLME_JOIN_REQ_STRUCT JoinReq;
	BSS_ENTRY *pInBss = NULL;
	PSTA_ADMIN_CONFIG pStaCfg;
#ifdef WSC_STA_SUPPORT
	PWSC_CTRL pWpsCtrl;
#endif /* WSC_STA_SUPPORT */
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
	BSS_TABLE *ScanTab = NULL;

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#ifdef WSC_STA_SUPPORT
	pWpsCtrl = &pStaCfg->wdev.WscControl;
#endif /* WSC_STA_SUPPORT */
#ifdef CONFIG_ATE

	/* No need to perform this routine when ATE is running. */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	/* record user desired settings */
	COPY_MAC_ADDR(pStaCfg->MlmeAux.Bssid, data);
	pStaCfg->MlmeAux.BssType = pStaCfg->BssType;
	/*save connect info*/
	os_zero_mem(pStaCfg->ConnectinfoBssid, MAC_ADDR_LEN);
	os_move_mem(pStaCfg->ConnectinfoBssid, data, MAC_ADDR_LEN);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ANDROID IOCTL::SIOCSIWAP "MACSTR"\n",
			  MAC2STR(pStaCfg->ConnectinfoBssid));
	/* find the desired BSS in the latest SCAN result table */
	BssIdx = BssTableSearch(ScanTab, data, pStaCfg->MlmeAux.Channel);
#ifdef WPA_SUPPLICANT_SUPPORT

	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
		;
	else
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE)
		;
	else
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
			if (
#ifdef WSC_STA_SUPPORT
				(pWpsCtrl->WscConfMode == WSC_DISABLE) &&
#endif /* WSC_STA_SUPPORT */
				(BssIdx != BSS_NOT_FOUND)) {
				pInBss = &ScanTab->BssEntry[BssIdx];

				/*
					If AP's SSID has been changed, STA cannot connect to this AP.
				*/
				if (SSID_EQUAL(pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen, pInBss->Ssid, pInBss->SsidLen) == FALSE)
					BssIdx = BSS_NOT_FOUND;

				if (IS_AKM_OPEN(wdev->SecConfig.AKMMap)
					|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
					|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
					if (wdev->SecConfig.PairwiseCipher != pInBss->PairwiseCipher)
						BssIdx = BSS_NOT_FOUND;
				} else {
					/* Check AuthMode and AuthModeAux for matching, in case AP support dual-mode */
					if (wdev->SecConfig.AKMMap != pInBss->AKMMap)
						BssIdx = BSS_NOT_FOUND;
				}
			}

#ifdef WSC_STA_SUPPORT

	if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
		((pWpsCtrl->WscStatus == STATUS_WSC_START_ASSOC) || (pWpsCtrl->WscStatus == STATUS_WSC_LINK_UP)) &&
		(pStaCfg->bSkipAutoScanConn == TRUE))
		pStaCfg->bSkipAutoScanConn = FALSE;

#endif /* WSC_STA_SUPPORT */

	if (BssIdx == BSS_NOT_FOUND) {
		if (((pStaCfg->BssType == BSS_INFRA) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))) ||
			(ScanInfo->bNotFirstScan == FALSE)) {
			MLME_SCAN_REQ_STRUCT ScanReq;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - BSSID not found. reply NDIS_STATUS_NOT_ACCEPTED\n");
#ifdef CONFIG_STA_ADHOC_SUPPORT
			if (pStaCfg->BssType == BSS_ADHOC)
				ScanInfo->bNotFirstScan = TRUE;
#endif /* CONFIG_STA_ADHOC_SUPPORT */

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - BSSID not found. start a new scan\n");

			if ((pStaCfg->ConnectinfoChannel  != 0) && (pStaCfg->Connectinfoflag == TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CntlOidRTBssidProc BSSID "MACSTR"\n",
						 MAC2STR(pStaCfg->ConnectinfoBssid));
				wdev->channel = pStaCfg->ConnectinfoChannel;
				MlmeEnqueueWithWdev(pAd, SYNC_FSM,  SYNC_FSM_JOIN_REQ,
									sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, 0, wdev);
				cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
			} else	{
				ScanParmFill(pAd, &ScanReq, (RTMP_STRING *) pStaCfg->MlmeAux.Ssid,
							 pStaCfg->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
				MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
									sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0, wdev);
				cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
			}
		} else {
			MLME_START_REQ_STRUCT StartReq;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - BSSID not found. start a new ADHOC (Ssid=%s)...\n",
					  pStaCfg->MlmeAux.Ssid);
			StartParmFill(pAd, &StartReq, (PCHAR)pStaCfg->MlmeAux.Ssid,
						  pStaCfg->MlmeAux.SsidLen);
			MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
								sizeof(MLME_START_REQ_STRUCT), &StartReq, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
		}

		return;
	}

	pInBss = &ScanTab->BssEntry[BssIdx];
	/* Update Reconnect Ssid, that user desired to connect. */
	os_zero_mem(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
	pStaCfg->MlmeAux.AutoReconnectSsidLen = pInBss->SsidLen;
	os_move_mem(pStaCfg->MlmeAux.AutoReconnectSsid, pInBss->Ssid, pInBss->SsidLen);
	/* copy the matched BSS entry from ScanTab to MlmeAux.SsidBssTab. Why? */
	/* Because we need this entry to become the JOIN target in later on SYNC state machine */
	pStaCfg->MlmeAux.BssIdx = 0;
	pStaCfg->MlmeAux.SsidBssTab.BssNr = 1;
	/* fix memory leak when trigger scan continuously */
	BssEntryCopy(&pStaCfg->MlmeAux.SsidBssTab,
				&pStaCfg->MlmeAux.SsidBssTab.BssEntry[0], pInBss);
	{
		if (INFRA_ON(pStaCfg)) {
			/* disassoc from current AP first */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - disassociate with current AP ...\n");
			sta_cntl_priv_disconnect(pStaCfg, CNTL_DISASSOC, REASON_DISASSOC_STA_LEAVING);
		} else {
#ifdef CONFIG_STA_ADHOC_SUPPORT
			if (ADHOC_ON(pAd)) {
				UINT link_down_type = 0;

				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "CNTL - drop current ADHOC\n");
				LinkDown(pAd, link_down_type, wdev, NULL);
				STA_STATUS_CLEAR_FLAG(pStaCfg,
									  fSTA_STATUS_MEDIA_STATE_CONNECTED);
				RTMP_IndicateMediaState(pAd,
										NdisMediaStateDisconnected);
				pAd->ExtraInfo = GENERAL_LINK_DOWN;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "NDIS_STATUS_MEDIA_DISCONNECT Event C!\n");
			}
#endif /* CONFIG_STA_ADHOC_SUPPORT */

			pInBss = &pStaCfg->MlmeAux.SsidBssTab.BssEntry[0];
			pStaCfg->PairwiseCipher = wdev->SecConfig.PairwiseCipher;
#ifdef WPA_SUPPLICANT_SUPPORT

			if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
				pStaCfg->GroupCipher = wdev->SecConfig.GroupCipher;

			/*Check AuthMode of AP, may have more AuthMode Supported,eg WPA/WPA2*/
			/*Set the AKMMap according to sta/AP capablities */
			if (IS_AKM_WPA1(wdev->SecConfig.AKMMap) && IS_AKM_WPA2(wdev->SecConfig.AKMMap)) {
				CLEAR_SEC_AKM(pStaCfg->AKMMap);
				SET_AKM_WPA2(pStaCfg->AKMMap);
			} else if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) && IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)) {
				CLEAR_SEC_AKM(pStaCfg->AKMMap);
				SET_AKM_WPA2PSK(pStaCfg->AKMMap);
			} else if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) && IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) {
				CLEAR_SEC_AKM(pStaCfg->AKMMap);
				SET_AKM_WPA3PSK(pStaCfg->AKMMap);
			} else
				pStaCfg->AKMMap = wdev->SecConfig.AKMMap;

			/* Check cipher suite, AP must have more secured cipher than station setting */
			/* Set the Pairwise and Group cipher to match the intended AP setting */
			/* We can only connect to AP with less secured cipher setting */
			if (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
				|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)) {
				pStaCfg->GroupCipher = pInBss->GroupCipher;
				CLEAR_CIPHER(pStaCfg->PairwiseCipher);

				if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher) && IS_CIPHER_CCMP128(pInBss->PairwiseCipher))
					SET_CIPHER_CCMP128(pStaCfg->PairwiseCipher);
				else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
					SET_CIPHER_TKIP(pStaCfg->PairwiseCipher);
			}

			if (IS_AKM_WPA2(wdev->SecConfig.AKMMap)
				|| IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
				|| IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) {
				pStaCfg->GroupCipher = pInBss->GroupCipher;
				CLEAR_CIPHER(pStaCfg->PairwiseCipher);

				if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)
					&& IS_CIPHER_CCMP128(pInBss->PairwiseCipher))
					SET_CIPHER_CCMP128(pStaCfg->PairwiseCipher);
				else if (IS_CIPHER_CCMP256(wdev->SecConfig.PairwiseCipher)
						 && IS_CIPHER_CCMP256(pInBss->PairwiseCipher))
					SET_CIPHER_CCMP256(pStaCfg->PairwiseCipher);
				else if (IS_CIPHER_GCMP128(wdev->SecConfig.PairwiseCipher)
						 && IS_CIPHER_GCMP128(pInBss->PairwiseCipher))
					SET_CIPHER_GCMP128(pStaCfg->PairwiseCipher);
				else if (IS_CIPHER_GCMP256(wdev->SecConfig.PairwiseCipher)
						 && IS_CIPHER_GCMP256(pInBss->PairwiseCipher))
					SET_CIPHER_GCMP256(pStaCfg->PairwiseCipher);
				else if (!IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap))
					/* There is no PairCipher Aux, downgrade our capability to TKIP */
					SET_CIPHER_TKIP(pStaCfg->PairwiseCipher);

				/* RSN capability */
				pStaCfg->RsnCapability = pInBss->RsnCapability;
			} else if (IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap)) {
				pStaCfg->GroupCipher = pInBss->GroupCipher;
				CLEAR_CIPHER(pStaCfg->PairwiseCipher);

				if (IS_CIPHER_GCMP256(wdev->SecConfig.PairwiseCipher)
						 && IS_CIPHER_GCMP256(pInBss->PairwiseCipher))
					SET_CIPHER_GCMP256(pStaCfg->PairwiseCipher);
				else if (IS_CIPHER_CCMP256(wdev->SecConfig.PairwiseCipher)
						 && IS_CIPHER_CCMP256(pInBss->PairwiseCipher))
					SET_CIPHER_CCMP256(pStaCfg->PairwiseCipher);

				/* RSN capability */
				pStaCfg->RsnCapability = pInBss->RsnCapability;
			}

			/* No active association, join the BSS immediately */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - joining "MACSTR" ...\n",
					  MAC2STR(data));
			JoinParmFill(pAd, &JoinReq, wdev, pStaCfg->MlmeAux.BssIdx);
			MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_JOIN_REQ,
								sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
		}
	}
}

static VOID sta_cntl_connect_roaming_proc(
	struct wifi_dev *wdev)
{
	PSTA_ADMIN_CONFIG pStaCfg;
	RTMP_ADAPTER *pAd;
#ifdef DOT11R_FT_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_ENTRY *pTargetAP = NULL;
#endif /* DOT11R_FT_SUPPORT */
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ASSERT(pAd);
	if (!pAd)
		return;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
#ifdef DOT11R_FT_SUPPORT
	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	pTargetAP = &pStaCfg->MlmeAux.RoamTab.BssEntry[0];
	ASSERT(pEntry);

	if (!pEntry)
		return;

#endif
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CNTL - Roaming in MlmeAux.RoamTab...\n");
#ifdef DOT11R_FT_SUPPORT

	if (pStaCfg->Dot11RCommInfo.bFtSupport &&
		pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
		(wdev->channel != pTargetAP->Channel)) {
		if (pEntry->MdIeInfo.FtCapPlc.field.FtOverDs) {
			MLME_FT_REQ_STRUCT FtReq;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Dot11r: pBss->Channel = %d. Use OTD.\n",
					  pTargetAP->Channel);
			/*FT_OTD_ActParmFill(pAd, &FtReq, pTargetAP->Bssid,
							   pTargetAP->AuthMode,
							   &pStaCfg->Dot11RCommInfo.MdIeInfo,
							   &pStaCfg->Dot11RCommInfo.FtIeInfo,
							   pStaCfg->RSNIE_Len,
							   pStaCfg->RSN_IE);*/
			MlmeEnqueueWithWdev(pAd, FT_OTD_ACT_STATE_MACHINE,
								FT_OTD_MT2_MLME_REQ,
								sizeof(MLME_FT_REQ_STRUCT), &FtReq, 0, wdev);
		} else
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CNTL - Shall not happen!!\n");
	} else
#endif /* DOT11R_FT_SUPPORT */
	{
		/*Let BBP register at 20MHz to do (fast) roaming. */
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		NdisMoveMemory(&pStaCfg->MlmeAux.SsidBssTab, &pStaCfg->MlmeAux.RoamTab,
					   sizeof(pStaCfg->MlmeAux.RoamTab));
		pStaCfg->MlmeAux.SsidBssTab.BssNr = pStaCfg->MlmeAux.RoamTab.BssNr;
		BssTableSortByRssi(&pStaCfg->MlmeAux.SsidBssTab, FALSE);
		pStaCfg->MlmeAux.BssIdx = 0;
		IterateOnBssTab(pAd, wdev);
	}
}

static VOID sta_cntl_connect_proc(
	struct wifi_dev *wdev,
	VOID *data,
	UINT32 data_len)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_CONNECT_STRUCT *cntl_conn;

	cntl_conn = (CNTL_MLME_CONNECT_STRUCT *)data;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	switch (cntl_conn->conn_type) {
	case CNTL_CONNECT_BY_SSID:
		sta_cntl_connect_by_ssid(wdev, cntl_conn->data_len, cntl_conn->data);
		break;

	case CNTL_CONNECT_BY_BSSID:
		sta_cntl_connect_by_bssid(wdev, cntl_conn->data);
		break;

	case CNTL_CONNECT_ROAMING_REQ:
		sta_cntl_connect_roaming_proc(wdev);
		break;

	case CNTL_CONNECT_BY_CFG:
		sta_cntl_connect_by_cfg(wdev);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Unknown conn_type(=%d)\n",
				  cntl_conn->conn_type);
		break;
	}
}

static VOID sta_cntl_disconnect_proc(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	PRTMP_ADAPTER pAd;
	PSTA_ADMIN_CONFIG pStaCfg;
	struct wifi_dev *wdev;
	CNTL_MLME_DISCONNECT_STRUCT *cntl_disconn;
	MLME_DISCONNECT_STRUCT mlme_disconn;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (PRTMP_ADAPTER)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	cntl_disconn = (CNTL_MLME_DISCONNECT_STRUCT *)Elem->Msg;
	mlme_disconn.reason = cntl_disconn->mlme_disconn.reason;
	os_move_mem(mlme_disconn.addr, cntl_disconn->mlme_disconn.addr, MAC_ADDR_LEN);

	if (cntl_disconn->cntl_disconn_type == CNTL_DEAUTH) {
		MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_MLME_DEAUTH_REQ,
							sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DEAUTH, __func__);
		RTMP_MLME_HANDLER(pAd);
	} else if (cntl_disconn->cntl_disconn_type == CNTL_DISASSOC) {
		MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_MLME_DISASSOC_REQ,
							sizeof(MLME_DISCONNECT_STRUCT), &mlme_disconn, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_DISASSOC, __func__);
		RTMP_MLME_HANDLER(pAd);
	}

#ifdef WPA_SUPPLICANT_SUPPORT

	if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) !=
		WPA_SUPPLICANT_ENABLE_WITH_WEB_UI)
#endif /* WPA_SUPPLICANT_SUPPORT */
	{
		/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
		/* Since calling this indicate user don't want to connect to that SSID anymore. */
		pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
		os_zero_mem(pStaCfg->MlmeAux.AutoReconnectSsid,
					pStaCfg->MlmeAux.AutoReconnectSsidLen);
	}
}

/* rept will enqeue join_conf to start assoc when wdev @ cntl_idle state */
static VOID sta_cntl_join_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	USHORT Reason;
	MLME_AUTH_REQ_STRUCT AuthReq;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	STA_ADMIN_CONFIG *pStaCfg;
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	MAC_TABLE_ENTRY *pAPEntry = NULL;
#endif
#ifndef CONFIG_STA_ADHOC_SUPPORT
	struct _SECURITY_CONFIG *pProfile_SecConfig;
#endif
	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	os_move_mem(&Reason, Elem->Msg, sizeof(USHORT));
#ifndef CONFIG_STA_ADHOC_SUPPORT
	pProfile_SecConfig = &wdev->SecConfig;
#endif

	if (Reason == MLME_SUCCESS) {
#ifdef CONFIG_STA_ADHOC_SUPPORT
		/* 1. joined an IBSS, we are pretty much done here */
		if (pStaCfg->MlmeAux.BssType == BSS_ADHOC) {
			UINT link_up_type = 0;
			/* */
			/* 5G bands rules of Japan: */
			/* Ad hoc must be disabled in W53(ch52,56,60,64) channels. */
			/* */

			if ((pAd->CommonCfg.bIEEE80211H == 1) &&
				RadarChannelCheck(pAd, wdev->channel)) {
				cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "CNTL - Channel=%d, Join adhoc on W53(52,56,60,64) Channels are not accepted\n",
						  wdev->channel);
				return;
			}

			LinkUp(pAd, BSS_ADHOC, wdev, link_up_type, NULL);
			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - join the IBSS = "MACSTR" ...\n",
					  MAC2STR(pStaCfg->Bssid));
			RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
			pAd->ExtraInfo = GENERAL_LINK_UP;
			RTMPSendWirelessEvent(pAd, IW_JOIN_IBSS_FLAG, NULL, BSS0, 0);
		}
		/* 2. joined a new INFRA network, start from authentication */
		else
#endif /* CONFIG_STA_ADHOC_SUPPORT	*/
		{
#ifdef DOT11R_FT_SUPPORT

			if (pStaCfg->Dot11RCommInfo.bFtSupport &&
				pStaCfg->Dot11RCommInfo.bInMobilityDomain) {
				/* MLME_FT_OTA_AUTH_REQ_STRUCT FtOtaAuthReq;

				FT_OTA_AuthParmFill(pAd,
									&FtOtaAuthReq,
									pStaCfg->MlmeAux.Bssid,
									AUTH_MODE_FT,
									&pStaCfg->Dot11RCommInfo);
				MlmeEnqueueWithWdev(pAd,
									FT_OTA_AUTH_STATE_MACHINE,
									FT_OTA_MT2_MLME_AUTH_REQ,
									sizeof(MLME_FT_OTA_AUTH_REQ_STRUCT),
									&FtOtaAuthReq, 0, wdev, NULL);*/
			} else
#endif /* DOT11R_FT_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
			pAPEntry = GetAssociatedAPByWdev(pAd, wdev);
			if ((IS_AKM_WPA3PSK(pStaCfg->wdev.SecConfig.AKMMap)) && (pAPEntry && (IS_AKM_WPA3PSK(pAPEntry->SecConfig.AKMMap)))
#ifdef WSC_INCLUDED
				&& pStaCfg->wdev.WscControl.bWscTrigger == FALSE
#endif
			) {
				UCHAR if_addr[MAC_ADDR_LEN];
				UCHAR pmkid[LEN_PMKID];
				UCHAR pmk[LEN_PMK];
				UCHAR has_pmkid = FALSE;
				UINT32 sec_akm = 0;

				NdisZeroMemory(if_addr, MAC_ADDR_LEN);

				SET_AKM_WPA3PSK(sec_akm);

#ifdef MAC_REPEATER_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
					REPEATER_CLIENT_ENTRY *rept_entry = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
					COPY_MAC_ADDR(if_addr, rept_entry->CurrentAddress);
				} else
#endif /* MAC_REPEATER_SUPPORT */
					NdisCopyMemory(if_addr, pStaCfg->wdev.if_addr, MAC_ADDR_LEN);

#ifdef SUPP_SAE_SUPPORT
		if (!pAd->CommonCfg.wifi_cert) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					"CNTL(%s) - use sae\n", __func__);
		} else
#endif
				if (sae_get_pmk_cache(&pAd->SaeCfg, if_addr, pStaCfg->MlmeAux.Bssid, pmkid, pmk)) {
					sta_add_pmkid_cache(pAd, pStaCfg->MlmeAux.Bssid, pmkid, pmk, LEN_PMK,  pStaCfg->wdev.func_idx, wdev, sec_akm
						, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
					has_pmkid = TRUE;
				}

				if (has_pmkid) {
					AuthParmFill(pAd, &AuthReq,
						     pStaCfg->MlmeAux.Bssid,
						     AUTH_MODE_OPEN);
					MlmeEnqueueWithWdev(pAd, AUTH_FSM,
									AUTH_FSM_MLME_AUTH_REQ,
									sizeof(MLME_AUTH_REQ_STRUCT),
									&AuthReq, 0, wdev);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
							"CNTL - use pmkid\n");
				} else {
					AuthParmFill(pAd, &AuthReq,
							     pStaCfg->MlmeAux.Bssid,
							     AUTH_MODE_SAE);
					auth_fsm_state_transition(wdev, AUTH_FSM_IDLE, __func__);
					MlmeEnqueueWithWdev(pAd, AUTH_FSM,
						    AUTH_FSM_SAE_AUTH_REQ,
						    sizeof(MLME_AUTH_REQ_STRUCT),
						    &AuthReq, 0, wdev);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
							"CNTL - use SAE\n");
				}
			} else
#endif /* DOT11_SAE_SUPPORT */

			{
#ifdef MAC_REPEATER_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
					REPEATER_CLIENT_ENTRY *rept = NULL;
					rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
					ASSERT(rept);
					rept->AuthReqCnt = 0;
					MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
							 "Repeater Cli Trigger Auth Req CliIdx = %d !!!\n",
							 wdev->func_idx);
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					pStaCfg->ApcliInfStat.AuthReqCnt = 0;
				}

#ifdef WEPAUTO_OPEN_FIRST

				/* Only Ndis802_11AuthModeShared try shared key first, Ndis802_11AuthModeAutoSwitch use open first */
				if (IS_SECURITY_SHARED_WEP_Entry(wdev))
#else

				/* either Ndis802_11AuthModeShared or Ndis802_11AuthModeAutoSwitch, try shared key first */
				if (IS_SECURITY_SHARED_WEP_Entry(wdev)
					|| IS_SECURITY_AUTOSWITCH_Entry(wdev))
#endif /* WEPAUTO_OPEN_FIRST */
					AuthParmFill(pAd, &AuthReq,
								 pStaCfg->MlmeAux.Bssid,
								 AUTH_MODE_KEY);
				else
					AuthParmFill(pAd, &AuthReq,
								 pStaCfg->MlmeAux.Bssid,
								 AUTH_MODE_OPEN);


#ifndef CONFIG_STA_ADHOC_SUPPORT
				/* Calculate PMK */
				if (!IS_CIPHER_WEP(pProfile_SecConfig->PairwiseCipher)
					&& (!IS_CIPHER_NONE(pProfile_SecConfig->PairwiseCipher))
					&& (!IS_AKM_OWE(pProfile_SecConfig->PairwiseCipher))
				   ) {
					SetWPAPSKKey(pAd,
						pProfile_SecConfig->PSK,
						strlen(pProfile_SecConfig->PSK),
						pStaCfg->MlmeAux.Ssid,
						pStaCfg->MlmeAux.SsidLen,
						pProfile_SecConfig->PMK);
				}
#endif

				MlmeEnqueueWithWdev(pAd, AUTH_FSM,
									AUTH_FSM_MLME_AUTH_REQ,
									sizeof(MLME_AUTH_REQ_STRUCT),
									&AuthReq, 0, wdev);
			}

			cntl_fsm_state_transition(wdev, CNTL_WAIT_AUTH, __func__);
		}
	} else if (Reason == MLME_INVALID_FORMAT) {
	  /* Do Not thing */
#ifdef APCLI_CFG80211_SUPPORT
	  pStaCfg->MlmeAux.BssIdx++;
	  IterateOnBssTab(pAd, wdev);
	  cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
	  RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd,  pStaCfg->MlmeAux.Bssid, wdev->func_idx, NULL, 0, NULL, 0, 0);
#else
	  cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
#endif
	} else {
		/* Retry Part. */
		if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef APCLI_CFG80211_SUPPORT
			pStaCfg->MlmeAux.BssIdx++;
			IterateOnBssTab(pAd, wdev);
			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
			RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd,  pStaCfg->MlmeAux.Bssid, wdev->func_idx, NULL, 0, NULL, 0, 0);
#else
			if (IF_COMBO_HAVE_AP_STA(pAd)) {
					join_iterate_by_cfg(pAd, wdev);
			} else {
				/* 3. failed, try next BSS */
				pStaCfg->MlmeAux.BssIdx++;
				IterateOnBssTab(pAd, wdev);
			}
#endif
		}
	}
}

static VOID sta_cntl_auth_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;
	USHORT Reason;
	MLME_ASSOC_REQ_STRUCT AssocReq;
	MLME_AUTH_REQ_STRUCT AuthReq;
	USHORT Timeout = ASSOC_TIMEOUT;
	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	os_move_mem(&Reason, Elem->Msg, sizeof(USHORT));

	if (Reason == MLME_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CNTL - AUTH OK and Start Assc\n");
#ifdef MAC_REPEATER_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			REPEATER_CLIENT_ENTRY *rept = NULL;
			rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
			ASSERT(rept);
			rept->AssocReqCnt = 0;
		} else
#endif /* MAC_REPEATER_SUPPORT */
			pStaCfg->ApcliInfStat.AssocReqCnt = 0;

#ifdef APCLI_CONNECTION_TRIAL

		if (pStaCfg->TrialCh != 0) {
			/* if connection trying, wait until trialtimeout and enqueue Assoc REQ then. */
			/* TrialCh == 0 means trial has not been triggered. */
		} else
#endif /* APCLI_CONNECTION_TRIAL */
		{
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			if (IS_AKM_SAE_SHA256(pStaCfg->MlmeAux.AKMMap)
				|| IS_AKM_OWE(pStaCfg->MlmeAux.AKMMap)
				|| IS_AKM_FT_SAE_SHA256(pStaCfg->MlmeAux.AKMMap)) {
					Timeout = 5000;
			}
#endif
			AssocParmFill(pAd, &AssocReq, pStaCfg->MlmeAux.Bssid,
						  pStaCfg->MlmeAux.CapabilityInfo,
						  Timeout,
						  pStaCfg->DefaultListenCount);
			if (pStaCfg->bAutoRoaming) {
				MlmeEnqueueWithWdev(pAd, ASSOC_FSM,
						ASSOC_FSM_MLME_REASSOC_REQ,
						sizeof(MLME_ASSOC_REQ_STRUCT),
						&AssocReq, 0, wdev);
				pStaCfg->bAutoRoaming = FALSE;
			} else {
				MlmeEnqueueWithWdev(pAd, ASSOC_FSM,
						ASSOC_FSM_MLME_ASSOC_REQ,
						sizeof(MLME_ASSOC_REQ_STRUCT),
						&AssocReq, 0, wdev);
			}
		}
		cntl_fsm_state_transition(wdev, CNTL_WAIT_ASSOC, __func__);
		/* assoc_fsm_state_transition(wdev, CliIdx, ASSOC_IDLE); // Check if this is needed */
	} else {
		USHORT alg;
		ULONG msg_type = AUTH_FSM_MLME_AUTH_REQ;
		/* This fail may because of the AP already keep us in its MAC table without */
		/* ageing-out. The previous authentication attempt must have let it remove us. */
		/* so try Authentication again may help. For D-Link DWL-900AP+ compatibility. */
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "CNTL - AUTH FAIL, try again...\n");

		if (IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) {
			alg = AUTH_MODE_SAE;
			msg_type = AUTH_FSM_SAE_AUTH_REQ;
		}
		/* Only Ndis802_11AuthModeShared try shared key first, Ndis802_11AuthModeAutoSwitch use open first */
		else if (IS_SECURITY_SHARED_WEP_Entry(wdev)
#ifdef WEPAUTO_OPEN_FIRST
#else
			|| IS_SECURITY_AUTOSWITCH_Entry(wdev)
#endif
		    )
			alg = AUTH_MODE_KEY;
		else
			alg = AUTH_MODE_OPEN;

		AuthParmFill(pAd,
			     &AuthReq,
			     pStaCfg->MlmeAux.Bssid,
			     alg);

		MlmeEnqueueWithWdev(pAd, AUTH_FSM,
							msg_type,
							sizeof(MLME_AUTH_REQ_STRUCT),
							&AuthReq, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_AUTH2, __func__);
	}
}

static VOID sta_cntl_auth2_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;
	USHORT Reason;
	MLME_ASSOC_REQ_STRUCT AssocReq;
	MLME_AUTH_REQ_STRUCT AuthReq;
	MAC_TABLE_ENTRY *pEntry;
	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	os_move_mem(&Reason, Elem->Msg, sizeof(USHORT));

	if (Reason == MLME_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CNTL - AUTH OK\n");
		AssocParmFill(pAd, &AssocReq, pStaCfg->MlmeAux.Bssid,
					  pStaCfg->MlmeAux.CapabilityInfo,
					  ASSOC_TIMEOUT,
					  pStaCfg->DefaultListenCount);
		MlmeEnqueueWithWdev(pAd, ASSOC_FSM,
							ASSOC_FSM_MLME_ASSOC_REQ,
							sizeof(MLME_ASSOC_REQ_STRUCT),
							&AssocReq, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_ASSOC, __func__);
	} else {
		if (IS_AKM_AUTOSWITCH(pStaCfg->wdev.SecConfig.AKMMap)
			&& IS_AKM_SHARED(pStaCfg->wdev.SecConfig.AKMMap)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - AUTH FAIL, try OPEN system...\n");
			AuthParmFill(pAd, &AuthReq, pStaCfg->MlmeAux.Bssid,
						 Ndis802_11AuthModeOpen);
			MlmeEnqueueWithWdev(pAd, AUTH_FSM,
								AUTH_FSM_MLME_AUTH_REQ,
								sizeof(MLME_AUTH_REQ_STRUCT),
								&AuthReq, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_WAIT_AUTH, __func__);
		} else {
			/* not success, try next BSS */
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - AUTH [%s, wdev_type=%d] FAIL, give up; try next BSS\n", wdev->if_dev->name, wdev->wdev_type);

			if ((wdev->wdev_type == WDEV_TYPE_STA) && pEntry) {
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			}

#ifdef MAC_REPEATER_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
				HW_REMOVE_REPT_ENTRY(pAd, wdev->func_idx);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wdev(type=%d,func_idx=%d)auth fail reason(%d) and free rept_entry\n",
					wdev->wdev_type, wdev->func_idx, Reason);
			}
#endif /* #ifdef MAC_REPEATER_SUPPORT */

			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
			pStaCfg->MlmeAux.BssIdx++;

			if (wdev->wdev_type == WDEV_TYPE_STA) {
					/*let wpa_supplicant take action on auth fail, instead of driver*/
#ifdef RT_CFG80211_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
				RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd,  pStaCfg->MlmeAux.Bssid, wdev->func_idx, NULL, 0, NULL, 0, 0);
#else
				RT_CFG80211_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, NULL, 0,
												   NULL, 0, 0);
#endif /* APCLI_CFG80211_SUPPORT */
#else
				if (IF_COMBO_HAVE_AP_STA(pAd)) {
					join_iterate_by_cfg(pAd, wdev);
				} else {
					IterateOnBssTab(pAd, wdev);
				}
#endif /*RT_CFG80211_SUPPORT*/
			}
		}
	}
}

static VOID sta_cntl_assoc_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	USHORT Reason;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;
	MAC_TABLE_ENTRY *pEntry;
#ifdef CONFIG_OWE_SUPPORT
	MLME_ASSOC_REQ_STRUCT AssocReq;
	UINT Timeout;
#endif
	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	os_move_mem(&Reason, Elem->Msg, sizeof(USHORT));

	if (Reason == MLME_SUCCESS) {
		UINT link_up_type = 0;

		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, NULL, BSS0, 0);
		LinkUp(pAd, BSS_INFRA, wdev, link_up_type, Elem);
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - Association successful on BSS #%ld\n", pStaCfg->MlmeAux.BssIdx);
	} else {
		/* not success, try next BSS */
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "CNTL - Association fails on BSS #%ld Reason(%d)\n", pStaCfg->MlmeAux.BssIdx, Reason);
#ifdef CONFIG_OWE_SUPPORT
	/*For 5.2.5  with OWE Test Plan v1.2, if STA receive assoc rsp with status code=77*/
	/*STA must try another supported group*/

	if (pEntry && (Reason == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED) &&
		(pAd->bApCliCertTest == TRUE) && (IS_AKM_OWE(pStaCfg->wdev.SecConfig.AKMMap))) {
		OWE_INFO *owe = &pEntry->SecConfig.owe;
		if (owe != NULL) {
			if (owe->last_try_group == ECDH_GROUP_256)
				pStaCfg->curr_owe_group = ECDH_GROUP_384;
			else if (owe->last_try_group == ECDH_GROUP_384) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_INFO,
						"OWE - no more group can try\n");
				pStaCfg->curr_owe_group = ECDH_GROUP_256;
			}
		}
		if (pStaCfg->curr_owe_group != ECDH_GROUP_256) {
			Timeout = 5000;
			AssocParmFill(pAd, &AssocReq, pStaCfg->MlmeAux.Bssid,
						  pStaCfg->MlmeAux.CapabilityInfo,
						  Timeout,
						  pStaCfg->DefaultListenCount);
			MlmeEnqueueWithWdev(pAd, ASSOC_FSM,
					ASSOC_FSM_MLME_ASSOC_REQ,
					sizeof(MLME_ASSOC_REQ_STRUCT),
					&AssocReq, 0, wdev);
			RTMP_MLME_HANDLER(pAd);
			return;
		}
	}
#endif
		if ((wdev->wdev_type == WDEV_TYPE_STA) && pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

#ifdef MAC_REPEATER_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			HW_REMOVE_REPT_ENTRY(pAd, wdev->func_idx);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev(type=%d,func_idx=%d)assoc fail reason(%d) and free rept_entry\n",
				wdev->wdev_type, wdev->func_idx, Reason);
		}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
		RT_CFG80211_CONN_RESULT_INFORM(pAd, pStaCfg->MlmeAux.Bssid, NULL, 0,
									   NULL, 0, 0);
#endif
#endif /* RT_CFG80211_SUPPORT */
		/* ASSERT(pEntry); */
		pStaCfg->MlmeAux.BssIdx++;
		IterateOnBssTab(pAd, wdev);
	}
}

static VOID sta_cntl_reassoc_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	USHORT Result;
	PSTA_ADMIN_CONFIG pStaCfg;
	UINT link_up_type = 0;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	os_move_mem(&Result, Elem->Msg, sizeof(USHORT));

	if (Result == MLME_SUCCESS) {
		/* send wireless event - for association */
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, NULL,
							  BSS0, 0);
#ifdef DOT11R_FT_SUPPORT

		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
			(pStaCfg->Dot11RCommInfo.FtRspSuccess ==
			 FT_OTD_RESPONSE))
			InitChannelRelatedValue(pAd, &pStaCfg->wdev);

#endif /* DOT11R_FT_SUPPORT */
		/* NDIS requires a new Link UP indication but no Link Down for RE-ASSOC */
		LinkUp(pAd, BSS_INFRA, Elem->wdev, link_up_type, Elem);
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - Re-assocition successful on BSS #%ld\n",
				  pStaCfg->MlmeAux.RoamIdx);
	} else {
		/* reassoc failed, try to pick next BSS in the BSS Table */
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "CNTL - Re-assocition fails on BSS #%ld\n",
				  pStaCfg->MlmeAux.RoamIdx);
#ifdef DOT11R_FT_SUPPORT

		if (pStaCfg->Dot11RCommInfo.bFtSupport &&
			pStaCfg->Dot11RCommInfo.bInMobilityDomain &&
			(pStaCfg->Dot11RCommInfo.FtRspSuccess == FT_OTD_RESPONSE)
		   ) {
			NDIS_802_11_SSID ApSsid;

			os_zero_mem(&ApSsid, sizeof(NDIS_802_11_SSID));
			ApSsid.SsidLength = pStaCfg->MlmeAux.SsidLen;
			os_move_mem(ApSsid.Ssid, pStaCfg->MlmeAux.Ssid, ApSsid.SsidLength);
			MlmeEnqueueWithWdev(pAd, MLME_CNTL_STATE_MACHINE,
								OID_802_11_SSID,
								sizeof(NDIS_802_11_SSID),
								(VOID *) &ApSsid, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		} else
#endif /* DOT11R_FT_SUPPORT */
		{
			pStaCfg->MlmeAux.RoamIdx++;
			IterateOnBssTab2(pAd, Elem->wdev);
		}
	}
}

static VOID sta_cntl_deauth_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;
	UINT link_down_type = 0;
	BOOLEAN bValid = FALSE;
	PULONG pLinkDownReason = NULL;
	BOOLEAN bDoIterate = FALSE;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "CNTL - DeAuth successful\n");
	RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);
	bValid = pStaCfg->ApcliInfStat.Valid;
	pLinkDownReason = &pStaCfg->ApcliInfStat.LinkDownReason;
#ifdef MAC_REPEATER_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;

		ASSERT(rept);
		bValid = rept->CliValid;
		pLinkDownReason = &rept->LinkDownReason;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		/*check the rept entry which is linked to the
		CliLink should be disabled, too.*/
		repeater_disconnect_by_band(pAd, HcGetBandByWdev(wdev));
	}

#endif /* MAC_REPEATER_SUPPORT */

	if (bValid) {
		*pLinkDownReason = STA_LINKDOWN_DEAUTH_REQ;
		LinkDown(pAd, link_down_type, wdev, Elem);
	}

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
		return;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pStaCfg->ApcliInfStat.Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);
		pStaCfg->MlmeAux.SsidLen = 0;
		NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pStaCfg->MlmeAux.Rssi = 0;
	}

	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);

	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
	&&
	(IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)
	 || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
#ifdef WSC_STA_SUPPORT
	&& (pStaCfg->wdev.WscControl.WscState < WSC_STATE_LINK_UP)
#endif /* WSC_STA_SUPPORT */
	)
		bDoIterate = TRUE;

	if (bDoIterate) {
		pStaCfg->MlmeAux.BssIdx++;
		IterateOnBssTab(pAd, wdev);
	}
}

static VOID sta_cntl_disassoc_conf(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;
	UINT link_down_type = 0;
	BOOLEAN bValid = FALSE;
	PULONG pLinkDownReason = NULL;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "CNTL - Dis-associate successful\n");
	RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0);
	bValid = pStaCfg->ApcliInfStat.Valid;
	pLinkDownReason = &pStaCfg->ApcliInfStat.LinkDownReason;
#ifdef MAC_REPEATER_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;

		ASSERT(rept);
		bValid = rept->CliValid;
		pLinkDownReason = &rept->LinkDownReason;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
	/*check the rept entry which is linked to the CliLink should be disabled, too.*/
		repeater_disconnect_by_band(pAd, HcGetBandByWdev(wdev));
	}

#endif /* MAC_REPEATER_SUPPORT */

	if (bValid) {
		*pLinkDownReason = STA_LINKDOWN_DEASSOC_REQ;
		LinkDown(pAd, link_down_type, wdev, Elem);
	}

#ifdef MAC_REPEATER_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);  /* don't try other BSS for Rept, simply restore state to CNTL_IDLE */
		return;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pStaCfg->ApcliInfStat.Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);
		pStaCfg->MlmeAux.SsidLen = 0;
		NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pStaCfg->MlmeAux.Rssi = 0;
	}

#ifdef CONFIG_STA_ADHOC_SUPPORT
	/* case 1. no matching BSS, and user wants ADHOC, so we just start a new one */
	if ((pStaCfg->MlmeAux.SsidBssTab.BssNr == 0)
		&& (pStaCfg->BssType == BSS_ADHOC)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - No matching BSS, start a new ADHOC (Ssid=%s)...\n",
				  pStaCfg->MlmeAux.Ssid);
		sta_cntl_priv_ibss_start(wdev);
		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
	}
	/* case 2. try each matched BSS */
	else
#endif /* CONFIG_STA_ADHOC_SUPPORT */
	{
		/*
		   Some customer would set AP1 & AP2 same SSID, AuthMode & EncrypType but different WPAPSK,
		   therefore we need to try next AP here.
		 */
		/*pStaCfg->MlmeAux.BssIdx = 0;*/
		pStaCfg->MlmeAux.BssIdx++;
#ifdef WSC_STA_SUPPORT

		if (pStaCfg->wdev.WscControl.WscState >= WSC_STATE_START) {
			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
			CntlWscIterate(pAd, pStaCfg);
		} else if ((pStaCfg->wdev.WscControl.bWscTrigger == FALSE)
				 && (pStaCfg->wdev.WscControl.WscState != WSC_STATE_INIT))
#endif /* WSC_STA_SUPPORT */
		{
			IterateOnBssTab(pAd, wdev);
		}
	}
}

static BOOLEAN sta_cntl_scan(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	ULONG BssIdx;
	BSS_ENTRY *pCurrBss = NULL;
	STA_ADMIN_CONFIG *pStaCfg;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	BSS_TABLE *ScanTab = NULL;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	ASSERT(pAd);
	if (!pAd)
		return FALSE;

	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(pStaCfg);
	if (!pStaCfg)
		return FALSE;

#ifdef CONFIG_ATE

	/* Disable scanning when ATE is running. */
	if (ATE_ON(pAd))
		return FALSE;

#endif /* CONFIG_ATE */
#ifdef P2P_SUPPORT

	if (pAd->P2pCfg.P2pCounter.bStartScan == TRUE)
		return FALSE;

#endif /* P2P_SUPPORT */
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **) &pCurrBss, sizeof(BSS_ENTRY));

	if (pCurrBss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Allocate memory fail!!!\n");
		return FALSE;
	}
	/* fix memory leak when trigger scan continuously */
	NdisZeroMemory(pCurrBss, sizeof(BSS_ENTRY));

	/* reset state machine for scan request */
	sync_cntl_fsm_to_idle_when_scan_req(pAd, wdev);

	/* record current BSS if network is connected. */
	/* 2003-2-13 do not include current IBSS if this is the only STA in this IBSS. */
	if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
		BssIdx = BssSsidTableSearch(ScanTab, pStaCfg->Bssid,
									(PUCHAR) pStaCfg->Ssid,
									pStaCfg->SsidLen,
									pStaCfg->wdev.channel);

		if (BssIdx != BSS_NOT_FOUND) {
			/* fix memory leak when trigger scan continuously */
			BssEntryCopy(NULL, pCurrBss, &ScanTab->BssEntry[BssIdx]);
		}
	}

	if (pCurrBss != NULL) {
		/* fix memory leak when trigger scan continuously */
		BssEntryReset(NULL, pCurrBss);

		os_free_mem(pCurrBss);
	}

	if (MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_SCAN_REQ,
						Elem->MsgLen, Elem->Msg, 0, wdev)) {
		RTMP_MLME_HANDLER(pAd);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
		return TRUE;
	}

	return FALSE;

}

static VOID sta_cntl_reset_all_fsm(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	PSTA_ADMIN_CONFIG pStaCfg;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	ASSERT(pAd);
	if (!pAd)
		return;
	cntl_fsm_reset(wdev);
	auth_fsm_reset(wdev);
	assoc_fsm_reset(wdev);
	/*sync_fsm_cancel_req_action(pAd, wdev);*/
	sync_fsm_reset(pAd, wdev);
	/* inform main thread fsm reset complete */
	sta_ifdown_fsm_reset_complete(pStaCfg);
}


static VOID sta_cntl_error_handle(
	VOID *elem_obj)
{
	MLME_QUEUE_ELEM *Elem;
	/* USHORT	status = MLME_SUCCESS; */
	PSTA_ADMIN_CONFIG pStaCfg;
	RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	Elem = (MLME_QUEUE_ELEM *)elem_obj;
	wdev = Elem->wdev;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ASSERT(pAd);
	CNTL_GET_STA_CFG(pAd, wdev, pStaCfg);
	wdev = Elem->wdev;
}


#ifdef WSC_STA_SUPPORT
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID CntlWscIterate(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
	struct wifi_dev *wdev = &pStaCfg->wdev;
	WSC_CTRL *wsc_ctrl;

	wsc_ctrl = &wdev->WscControl;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s():: wsc_ctrl->WscState = %d (WSC_STATE_START = %d, STATUS_WSC_SCAN_AP = %d)!!\n",
			  __func__, wsc_ctrl->WscState, WSC_STATE_START, STATUS_WSC_SCAN_AP);

	/* Connect to the WPS-enabled AP by its BSSID directly. */
	/* Note: When WscState is equal to WSC_STATE_START, */
	/*       pStaCfg->WscControl.WscAPBssid has been filled with valid BSSID. */
	if ((wsc_ctrl->WscState >= WSC_STATE_START) &&
		(wsc_ctrl->WscStatus != STATUS_WSC_SCAN_AP)) {
		/* Set WSC state to WSC_STATE_START */
		wsc_ctrl->WscState = WSC_STATE_START;
		wsc_ctrl->WscStatus = STATUS_WSC_START_ASSOC;

		if (pStaCfg->BssType != BSS_ADHOC)
			cntl_connect_request(wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, (UCHAR *)&wsc_ctrl->WscBssid[0]);
		else
			cntl_connect_request(wdev, CNTL_CONNECT_BY_SSID, sizeof(NDIS_802_11_SSID), (UCHAR *)&wsc_ctrl->WscSsid);

	}

#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(wdev));
#endif /* WSC_LED_SUPPORT */
}
#endif /* WSC_STA_SUPPORT */

#ifdef CONFIG_STA_ADHOC_SUPPORT
VOID AdhocTurnOnQos(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
	if (pAd->CommonCfg.APEdcaParm[0].bValid == FALSE) {
		pAd->CommonCfg.APEdcaParm[0].bValid = TRUE;
		pAd->CommonCfg.APEdcaParm[0].Aifsn[0] = 3;
		pAd->CommonCfg.APEdcaParm[0].Aifsn[1] = 7;
		pAd->CommonCfg.APEdcaParm[0].Aifsn[2] = 1;
		pAd->CommonCfg.APEdcaParm[0].Aifsn[3] = 1;
		pAd->CommonCfg.APEdcaParm[0].Cwmin[0] = 4;
		pAd->CommonCfg.APEdcaParm[0].Cwmin[1] = 4;
		pAd->CommonCfg.APEdcaParm[0].Cwmin[2] = 3;
		pAd->CommonCfg.APEdcaParm[0].Cwmin[3] = 2;
		pAd->CommonCfg.APEdcaParm[0].Cwmax[0] = 10;
		pAd->CommonCfg.APEdcaParm[0].Cwmax[1] = 6;
		pAd->CommonCfg.APEdcaParm[0].Cwmax[2] = 4;
		pAd->CommonCfg.APEdcaParm[0].Cwmax[3] = 3;
		pAd->CommonCfg.APEdcaParm[0].Txop[0] = 0;
		pAd->CommonCfg.APEdcaParm[0].Txop[1] = 0;
		pAd->CommonCfg.APEdcaParm[0].Txop[2] = 94;
		pAd->CommonCfg.APEdcaParm[0].Txop[3] = 47;
	}

	HcAcquiredEdca(pAd, &pStaCfg->wdev, &pAd->CommonCfg.APEdcaParm[0]);
}


VOID LinkUp_Adhoc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry)
{
	/* INT idx; */
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	/* If WEP is enabled, add key material and cipherAlg into Asic */
	/* Fill in Shared Key Table(offset: 0x6c00) and Shared Key Mode(offset: 0x7000) */
#ifdef ADHOC_WPA2PSK_SUPPORT
	else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
			 && (pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
			) {
		USHORT Wcid = 0;

		wdev->DefaultKeyId = 0;	/* always be zero */
		NdisZeroMemory(&pAd->SharedKey[BSS0][0], sizeof(CIPHER_KEY));
		pAd->SharedKey[BSS0][0].KeyLen = LEN_TK;
		NdisMoveMemory(pAd->SharedKey[BSS0][0].Key, pStaCfg->PMK, LEN_TK);
		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
		/* Generate GMK and GNonce randomly */
		GenRandom(pAd, pStaCfg->Bssid, pStaCfg->GMK);
		GenRandom(pAd, pStaCfg->Bssid, pStaCfg->GNonce);
		/* Derive GTK per BSSID */
		WpaDeriveGTK(pStaCfg->GMK,
					 pStaCfg->GNonce,
					 wdev->if_addr,
					 pStaCfg->GTK, LEN_TKIP_GTK);

		if (pStaCfg->GroupCipher == Ndis802_11AESEnable) {
			NdisZeroMemory(&pStaCfg->TxGTK, sizeof(CIPHER_KEY));
			NdisMoveMemory(pStaCfg->TxGTK.Key, pStaCfg->GTK, LEN_TK);
			pStaCfg->TxGTK.CipherAlg = CIPHER_AES;
			pStaCfg->TxGTK.KeyLen = LEN_TK;
			/* Add Pair-wise key to Asic */
			/* GET_GroupKey_WCID(pAd, Wcid, BSS0); */
			Wcid = pStaCfg->wdev.bss_info_argument.bmc_wlan_idx;
			AsicAddPairwiseKeyEntry(pAd,
									Wcid,
									&pStaCfg->TxGTK);
		}
	}

#endif /* ADHOC_WPA2PSK_SUPPORT */
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RT_CFG80211_JOIN_IBSS(pAd, pStaCfg->MlmeAux.Bssid);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

	if (wdev_do_linkup(wdev, pEntry) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " linkup fail!!\n");

	if (wdev_do_conn_act(wdev, pEntry) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " connect fail!!\n");
}
#endif /* CONFIG_STA_ADHOC_SUPPORT */

VOID LinkUp_Infra(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, UCHAR *tmpWscSsid, UCHAR tmpWscSsidLen, UINT link_up_type, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	UCHAR	BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	USHORT ifIndex = 0;
	struct _SECURITY_CONFIG *pProfile_SecConfig = NULL;
	struct _SECURITY_CONFIG *pEntry_SecConfig = NULL;
#ifdef WSC_INCLUDED
	BOOLEAN do_wsc_now = FALSE;
#endif /* WSC_INCLUDED */
	Cancelled = TRUE;

	if (!wdev)
		return;

	if (!pEntry)
		return;

	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	ifIndex = wdev->func_idx;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	if (wdev->wdev_type == WDEV_TYPE_REPEATER)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				" - !!! Rept CliIdx %d !!!\n", ifIndex);

#ifdef WSC_INCLUDED

	if ((pStaCfg->wdev.WscControl.WscConfMode != WSC_DISABLE)
		&& pStaCfg->wdev.WscControl.bWscTrigger)
		do_wsc_now = TRUE;

#endif /* WSC_INCLUDED */

#ifdef STA_LP_PHASE_2_SUPPORT
	/* Reset Beaconlost */
	pStaCfg->PwrMgmt.bBeaconLost = FALSE;
	pStaCfg->PwrMgmt.bTriggerRoaming = FALSE;
	pStaCfg->PwrMgmt.wcid = pEntry->wcid;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BeaconPeriod(%d),DtimPeriod(%d)\n",
		pStaCfg->MlmeAux.BeaconPeriod, pStaCfg->MlmeAux.DtimPeriod);
#endif /* STA_LP_PHASE_2_SUPPORT */

	COPY_MAC_ADDR(wdev->bssid, pStaCfg->Bssid);
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];

	if (IS_CIPHER_NONE_OR_WEP_Entry(wdev)) {
#ifdef WPA_SUPPLICANT_SUPPORT

		if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP &&
			(IS_CIPHER_WEP_Entry(wdev)) &&
			(wdev->SecConfig.IEEE8021X == TRUE))
			;
		else
#endif /* WPA_SUPPLICANT_SUPPORT */
		{
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;
			pStaCfg->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
		}
	}

	/*
	 *	On WPA mode, Remove All Keys if not connect to the last BSSID
	 *	Key will be set after 4-way handshake
	 */
#ifdef CONFIG_OWE_SUPPORT
	if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (IS_AKM_OPEN_ONLY(pEntry->SecConfig.AKMMap))) {
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
		pStaCfg->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	} else
#endif
	if ((IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
#ifdef WSC_INCLUDED
		 && (do_wsc_now == FALSE)
#endif /* WSC_INCLUDED */
		)
	   ) {
		/* Remove all WPA keys */
		/*
		 *		 for dhcp,issue ,wpa_supplicant ioctl too fast , at link_up, it will add key before driver remove key
		 *	 move to assoc.c
		 */
		/*			RTMPWPARemoveAllKeys(pAd, wdev);*/
		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pStaCfg->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
#ifdef SOFT_ENCRYPT
		/* There are some situation to need to encryption by software
		 *   1. The Client support PMF. It shall ony support AES cipher.
		 *   2. The Client support WAPI.
		 *   If use RT3883 or later, HW can handle the above.
		 */
#ifdef DOT11W_PMF_SUPPORT
		{
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if ((cap->FlgPMFEncrtptMode == PMF_ENCRYPT_MODE_0)
				&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE))
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);
		}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* SOFT_ENCRYPT */
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
	tr_entry->PortSecured = wdev->PortSecured;
	NdisReleaseSpinLock(&pAd->MacTabLock);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pStaCfg->MlmeAux.APEdcaParm.bValid) {
			pEntry->bACMBit[WMM_AC_BK] = pStaCfg->MlmeAux.APEdcaParm.bACM[WMM_AC_BK];
			pEntry->bACMBit[WMM_AC_BE] = pStaCfg->MlmeAux.APEdcaParm.bACM[WMM_AC_BE];
			pEntry->bACMBit[WMM_AC_VI] = pStaCfg->MlmeAux.APEdcaParm.bACM[WMM_AC_VI];
			pEntry->bACMBit[WMM_AC_VO] = pStaCfg->MlmeAux.APEdcaParm.bACM[WMM_AC_VO];
		}
	}

#endif /* MT_MAC */


	if (IF_COMBO_HAVE_AP_STA(pAd)) {
		/* insert APCLI related logic =================== */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn) {
			INVAILD_TRIGGER_MAC_ENTRY *pSkipEntry = NULL;

			if (wdev->wdev_type == WDEV_TYPE_STA) {
#ifdef LINUX
				struct net_device *pNetDev;
				struct net *net = &init_net;
				/* old kernerl older than 2.6.21 didn't have for_each_netdev()*/
#ifndef for_each_netdev

				for (pNetDev = dev_base; pNetDev != NULL; pNetDev = pNetDev->next)
#else
				for_each_netdev(net, pNetDev)
#endif
				{
					if (pNetDev->priv_flags & IFF_EBRIDGE) {
						COPY_MAC_ADDR(pAd->ApCfg.BridgeAddress, pNetDev->dev_addr);
						MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, " Bridge Addr = "MACSTR". !!!\n",
								 MAC2STR(pAd->ApCfg.BridgeAddress));
					}

					pSkipEntry = RepeaterInvaildMacLookup(pAd, pNetDev->dev_addr);
					#ifndef ANDLINK_FEATURE_SUPPORT
					if (pSkipEntry == NULL)
						InsertIgnoreAsRepeaterEntryTable(pAd, pNetDev->dev_addr);
					#endif
				}
				/* Comment this part for fix ping fail issue when repeater connect to
				 * Different 2G/5G RootAP in DBDC Mode.*/
				/*
				if (!MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, ZERO_MAC_ADDR)) {
					pSkipEntry = RepeaterInvaildMacLookup(pAd, pAd->ApCfg.BridgeAddress);

					if (pSkipEntry) {
						UCHAR MacAddr[MAC_ADDR_LEN];
						UCHAR entry_idx;

						COPY_MAC_ADDR(MacAddr, pSkipEntry->MacAddr);
						entry_idx = pSkipEntry->entry_idx;
						RepeaterRemoveIngoreEntry(pAd, entry_idx, MacAddr);
					}
				}
				*/
#endif
			}
		}

		if ((wdev->wdev_type == WDEV_TYPE_REPEATER)
#ifdef MWDS
			&& ((pStaCfg->wdev.bSupportMWDS == FALSE) ||
				(pStaCfg->MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */
		) {
			MAC_TABLE_ENTRY *pMacEntry = NULL;
			REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;

			if(!rept || !pEntry->pReptCli)
				return;

			pMacEntry = MacTableLookup(pAd, rept->OriginalAddress);

			if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
				SET_REPT_CLI_TYPE(rept, REPT_WIRELESS_CLI);
				pMacEntry->ProxySta = (VOID *)rept;
			} else
				SET_REPT_CLI_TYPE(rept, REPT_ETH_CLI);

			rept->CliValid = TRUE;
			pEntry->pReptCli->ReptCliIdleCount = 0;
			tr_entry->OmacIdx = HcGetRepeaterOmac(wdev);

			/*Check the client if it is a bridge*/
			if (IS_REPT_CLI_TYPE(rept, REPT_ETH_CLI) /*TBD : This statement might be unnecessary*/
				&& MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, rept->OriginalAddress))
				ADD_REPT_CLI_TYPE(rept, REPT_BRIDGE_CLI);

			/* consider connected, due to already port secured in StaUpdateMacTableEntry */
			rept->CliConnectState = REPT_ENTRY_CONNTED;
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
			pStaCfg->ApcliInfStat.Valid = TRUE;
			pStaCfg->MacTabWCID = pEntry->wcid;
			COPY_MAC_ADDR(&wdev->bssid[0], &pStaCfg->MlmeAux.Bssid[0]);
			os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pEntry->Addr, pStaCfg->MlmeAux.Bssid);
			pStaCfg->SsidLen = pStaCfg->MlmeAux.SsidLen;
			NdisMoveMemory(pStaCfg->Ssid, pStaCfg->MlmeAux.Ssid, pStaCfg->SsidLen);
			NdisGetSystemUpTime(&pStaCfg->ApcliInfStat.ApCliLinkUpTime);
		}
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT

		if (wdev->wdev_type == WDEV_TYPE_STA)
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (wf_drv_tbl.wf_fwd_get_rep_hook)
				wf_drv_tbl.wf_fwd_get_rep_hook(pAd->CommonCfg.EtherTrafficBand);

			if (wf_drv_tbl.wf_fwd_check_device_hook)
				wf_drv_tbl.wf_fwd_check_device_hook(pStaCfg->wdev.if_dev, INT_APCLI,
					NON_REPT_ENTRY, pStaCfg->wdev.channel, 1);

			if (wf_drv_tbl.wf_fwd_entry_insert_hook)
				wf_drv_tbl.wf_fwd_entry_insert_hook(pStaCfg->wdev.if_dev, pAd->net_dev, pAd);

			if (wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook)
#ifdef MAC_REPEATER_SUPPORT
				wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook(pAd, &pAd->ApCfg.ReptCliEntryLock,
				&pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0],
				&pAd->StaCfg[ifIndex].wdev.if_addr);
#else
				wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook(pAd,
				NULL, NULL, NULL, &pAd->StaCfg[ifIndex].wdev.if_addr);
#endif /* MAC_REPEATER_SUPPORT */
		}
#endif /* CONFIG_WIFI_PKT_FWD */
		pProfile_SecConfig = &wdev->SecConfig;
		pEntry_SecConfig = &pEntry->SecConfig;


		if (IS_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher)) {
			os_move_mem(pEntry_SecConfig->WepKey, pProfile_SecConfig->WepKey,
					sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
			pProfile_SecConfig->GroupKeyId = pProfile_SecConfig->PairwiseKeyId;
			pEntry_SecConfig->PairwiseKeyId = pProfile_SecConfig->PairwiseKeyId;
		} else {
			/* Calculate PMK */
#ifdef WSC_INCLUDED
			if (!do_wsc_now)
#endif /* WSC_INCLUDED */
#ifndef CONFIG_STA_ADHOC_SUPPORT
#ifdef DOT11_SAE_SUPPORT
			if (!((IS_AKM_SAE_SHA256(pEntry_SecConfig->AKMMap) || IS_AKM_OWE(pEntry_SecConfig->AKMMap)
					|| IS_AKM_OPEN(pEntry_SecConfig->AKMMap))))
#endif
				NdisCopyMemory(pEntry_SecConfig->PMK, pProfile_SecConfig->PMK, LEN_PMK);
#else
			SetWPAPSKKey(pAd, pProfile_SecConfig->PSK, strlen(pProfile_SecConfig->PSK),
							pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen, pEntry_SecConfig->PMK);
#endif

				os_move_mem(pEntry_SecConfig->Handshake.AAddr, pEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(pEntry_SecConfig->Handshake.SAddr, wdev->if_addr, MAC_ADDR_LEN);

			os_zero_mem(pEntry_SecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);
			pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
		}

		pEntry_SecConfig->GroupKeyId = pProfile_SecConfig->GroupKeyId;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(apcli%d) connect AKM(0x%x)=%s, PairwiseCipher(0x%x)=%s, GroupCipher(0x%x)=%s\n",
				  ifIndex,
				  pEntry_SecConfig->AKMMap, GetAuthModeStr(pEntry_SecConfig->AKMMap),
				  pEntry_SecConfig->PairwiseCipher, GetEncryModeStr(pEntry_SecConfig->PairwiseCipher),
				  pEntry_SecConfig->GroupCipher, GetEncryModeStr(pEntry_SecConfig->GroupCipher));
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 " PairwiseKeyId=%d, GroupKeyId=%d\n",
				  pEntry_SecConfig->PairwiseKeyId, pEntry_SecConfig->GroupKeyId);


#ifdef MWDS
	if ((wdev->wdev_type == WDEV_TYPE_STA) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		MWDSAPCliPeerEnable(pAd, pStaCfg, pEntry);
	}
#endif /* MWDS */

#ifdef MT_MAC
#ifdef MAC_REPEATER_SUPPORT

		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			AsicInsertRepeaterRootEntry(
				pAd,
				pEntry->wcid,
				(PUCHAR)(pStaCfg->MlmeAux.Bssid),
				wdev->func_idx);
		}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* MT_MAC */
#ifdef CONFIG_AP_SUPPORT
		pAd->ApCfg.ApCliInfRunned++;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (pStaCfg->ApCliMode == APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION) {
#ifdef WAPP_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, " APCLI Linkup Event send to wapp\n");
		wapp_send_apcli_association_change_vendor10(WAPP_APCLI_ASSOCIATED, pAd, pStaCfg);
#endif
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */



#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA)
#endif
			wf_apcli_active_links++;

#ifdef CONFIG_FAST_NAT_SUPPORT
			if ((wf_apcli_active_links >= 2)
				&& (ra_sw_nat_hook_rx != NULL)
				&& (wf_ra_sw_nat_hook_rx_bkup == NULL)) {
				wf_ra_sw_nat_hook_rx_bkup = ra_sw_nat_hook_rx;
				ra_sw_nat_hook_rx = NULL;
			}
#endif /*CONFIG_FAST_NAT_SUPPORT*/
#endif

#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if (pStaCfg->ApCliAutoConnectRunning == FALSE)
#endif
		{
			UCHAR ext_cha = wlan_config_get_ext_cha(wdev);
			ULONG Bssidx = 0;
			BSS_TABLE *ScanTab = NULL;

			ScanTab = get_scan_tab_by_wdev(pAd, wdev);
			Bssidx = BssTableSearch(ScanTab, pEntry->bssid, wdev->channel);

			if (Bssidx != BSS_NOT_FOUND)
				ext_cha = ScanTab->BssEntry[Bssidx].AddHtInfo.AddHtInfo.ExtChanOffset;
			else
				MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						 "ERROR! can't find link up BSS, use config BW setting instead\n");

			if (pEntry->MaxHTPhyMode.field.BW < BW_80)
				wlan_operate_set_ht_bw(wdev, pEntry->MaxHTPhyMode.field.BW, ext_cha);
			else {
#ifdef DOT11_VHT_AC

				switch (pEntry->MaxHTPhyMode.field.BW) {
				case BW_80:
					wlan_operate_set_vht_bw(wdev, VHT_BW_80);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;

				case BW_160:
					wlan_operate_set_vht_bw(wdev, VHT_BW_160);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;

				default:
					wlan_operate_set_vht_bw(wdev, VHT_BW_2040);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;
				}

#endif
			}
		}

#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if ((pStaCfg->ApCliAutoConnectRunning == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
			(wdev->wdev_type == WDEV_TYPE_STA) &&
#endif /* MAC_REPEATER_SUPPORT */
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ApCli auto connected: ApCliLinkUp()\n");
			pStaCfg->ApCliAutoConnectRunning = FALSE;
		}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		pStaCfg->ApcliInfStat.Valid = TRUE;
#ifdef CONFIG_MAP_SUPPORT

	if (IS_MAP_ENABLE(pAd) &&(pEntry->DevPeerRole & (BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS))))
		pStaCfg->PeerMAPEnable = 1;
	else
		pStaCfg->PeerMAPEnable = 0;

#if defined(WAPP_SUPPORT)
		if (IS_MAP_ENABLE(pAd) && (wdev->wdev_type == WDEV_TYPE_STA) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			/*For security NONE & WEP case*/
			wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pStaCfg);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					"APCLIENT MAP_ENABLE (No Security)\n");
#ifdef A4_CONN
			map_a4_peer_enable(pAd, pEntry, FALSE);
#endif
			if (pStaCfg->wdev.WscControl.bWscTrigger == FALSE)
				map_send_bh_sta_wps_done_event(pAd, pEntry, FALSE);
		}

#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */


	}
	/* insert APCLI related logic  end =================== */

	/*only non-repeater should linkup*/
	if ((wdev->wdev_type == WDEV_TYPE_STA) && wdev_do_linkup(wdev, pEntry) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " linkup fail!!\n");

	/* Use main sta's phy cap */
	if (wdev_do_conn_act(&pStaCfg->wdev, pEntry) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " connect fail!!\n");

#ifdef WH_EVENT_NOTIFIER
	if ((result == TRUE) && pEntry &&
		tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		EventHdlr pEventHdlrHook = NULL;

		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
		if (pEventHdlrHook && pEntry->wdev)
			pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
	}
#endif /* WH_EVENT_NOTIFIER */

	/* Check the new SSID with last SSID */
	while (TRUE) {
		if (pStaCfg->LastSsidLen == pStaCfg->SsidLen) {
			if (RTMPCompareMemory(pStaCfg->LastSsid, pStaCfg->Ssid, pStaCfg->LastSsidLen) == 0) {
				/* Link to the old one no linkdown is required. */
				break;
			}
		}

		/* Send link down event before set to link up */
		RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
		pAd->ExtraInfo = GENERAL_LINK_DOWN;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NDIS_STATUS_MEDIA_DISCONNECT Event AA!\n");
		break;
	}

#ifdef DOT11R_FT_SUPPORT
	/*
	 *   11R only supports OPEN/NONE and WPA2PSK/TKIPAES now.
	 */
	if (pStaCfg->Dot11RCommInfo.bFtSupport &&
		pStaCfg->MlmeAux.MdIeInfo.Len &&
		IS_SECURITY_OPEN_NONE_Entry(wdev))
		pStaCfg->Dot11RCommInfo.bInMobilityDomain = TRUE;

#endif /* DOT11R_FT_SUPPORT */
	/* NOTE: */
	/* the decision of using "short slot time" or not may change dynamically due to */
	/* new STA association to the AP. so we have to decide that upon parsing BEACON, not here */
	/* NOTE: */
	/* the decision to use "RTC/CTS" or "CTS-to-self" protection or not may change dynamically */
	/* due to new STA association to the AP. so we have to decide that upon parsing BEACON, not here */
	ComposePsPoll(pAd, &(pAd->PsPollFrame), pStaCfg->StaActive.Aid,
				  pStaCfg->Bssid, pStaCfg->wdev.if_addr);
	ComposeNullFrame(pAd, &(pAd->NullFrame), pStaCfg->Bssid,
					 pStaCfg->wdev.if_addr, pStaCfg->Bssid);
#ifdef P2P_SUPPORT
		pAd->P2pCfg.GroupChannel = wdev->channel;

	if (P2P_GO_ON(pAd)) {
		UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

		AsicEnableP2PGoSync(pAd);

		if (wdev->channel != pAd->P2PChannel) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel change , old channel:%d new channel:%d !!\n"
					 , pAd->P2PChannel, wdev->channel);
#ifdef DOT11_N_SUPPORT
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Old ExtCh:%d new ExtCh:%d !!\n"
					 , pAd->P2PExtChOffset, ext_cha);
#endif /* DOT11_N_SUPPORT */
			P2P_GoStop(pAd);
			P2P_GoStartUp(pAd, MAIN_MBSSID);
		}
	} else if (P2P_CLI_ON(pAd)) {
		AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
			HW_BSSID_0, OPMODE_STA);

		if (wdev->channel != pAd->P2PChannel) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel change , old channel:%d new channel:%d !!\n"
					 , pAd->P2PChannel, wdev->channel);
#ifdef DOT11_N_SUPPORT
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Old ExtCh:%d new ExtCh:%d !!\n"
					 , pAd->P2PExtChOffset, ext_cha);
#endif /* DOT11_N_SUPPORT */
			MlmeEnqueueWithWdev(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, 0, wdev);
		}
	} else
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		if (CFG_P2PGO_ON(pAd))
			AsicEnableApBssSync(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)]);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
			AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
				HW_BSSID_0, OPMODE_STA);

	MacTableSetEntryRaCap(pAd, pEntry, &pStaCfg->MlmeAux.vendor_ie);
#ifdef DOT11_VHT_AC
	if (HAS_VHT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists))
		MacTableEntryCheck2GVHT(pAd, pEntry);
#endif /* DOT11_VHT_AC */

	/* +++Add by shiang for debug */
	{
		STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[pEntry->wcid];
		/* TODO: shiang-usw, revise this! */
		COPY_MAC_ADDR(tr_entry->bssid, wdev->bssid);
	}
	/* ---Add by shiang */
	/* If WEP is enabled, add paiewise and shared key */
#ifdef WPA_SUPPLICANT_SUPPORT

	if (((pStaCfg->wpa_supplicant_info.WpaSupplicantUP) &&
		 (IS_CIPHER_WEP_Entry(wdev)) &&
		 (wdev->PortSecured == WPA_802_1X_PORT_SECURED)) ||
		((pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE) &&
		 (IS_CIPHER_WEP_Entry(wdev))))
#else
	if (IS_CIPHER_WEP_Entry(wdev))
#endif /* WPA_SUPPLICANT_SUPPORT */
	{
		ASIC_SEC_INFO Info = {0};
		struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
		/* Set Group key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
		Info.Direction = SEC_ASIC_KEY_RX;
		Info.Wcid = wdev->bss_info_argument.bmc_wlan_idx;
		Info.BssIndex = wdev->wdev_idx;
		Info.Cipher = pSecConfig->GroupCipher;
		Info.KeyIdx = pSecConfig->GroupKeyId;
		os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
		os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
		/* Set Pairwise key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
		Info.Direction = SEC_ASIC_KEY_BOTH;
		Info.Wcid = pEntry->wcid;
		Info.BssIndex = wdev->wdev_idx;
		Info.Cipher = pSecConfig->PairwiseCipher;
		Info.KeyIdx = pSecConfig->PairwiseKeyId;
		os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
		os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
	}


	/* For GUI ++ */
	if (IS_AKM_OPEN(wdev->SecConfig.AKMMap) || IS_AKM_SHARED(wdev->SecConfig.AKMMap) || IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
#ifdef WPA_SUPPLICANT_SUPPORT

		if (((pStaCfg->wpa_supplicant_info.WpaSupplicantUP) && (IS_CIPHER_WEP_Entry(wdev)) && (wdev->PortSecured == WPA_802_1X_PORT_SECURED))
			|| ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE) && (IS_CIPHER_WEP_Entry(wdev)))
			|| (IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)))
#endif /* WPA_SUPPLICANT_SUPPORT */
		{
			pAd->ExtraInfo = GENERAL_LINK_UP;
			RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
		}
	} else if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) ||
			   IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)) {
		if (
#ifdef WPA_SUPPLICANT_SUPPORT

		(pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
		(!(IF_COMBO_HAVE_AP_STA(pAd) && (wdev->wdev_type == WDEV_TYPE_STA)))
		)
			RTMPSetTimer(&pStaCfg->LinkDownTimer, LINK_DOWN_TIMEOUT);

#ifdef WSC_STA_SUPPORT

		if (wdev->WscControl.WscProfileRetryTimerRunning) {
			RTMPSetTimer(&wdev->WscControl.WscProfileRetryTimer,
						 WSC_PROFILE_RETRY_TIME_OUT);
		}

#endif /* WSC_STA_SUPPORT */
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! LINK UP !!!  ClientStatusFlags=%lx)\n",
			 pEntry->ClientStatusFlags);
#ifdef WIDI_SUPPORT

	/*
		Send L2SDTA Association status here
	*/
	if (wdev->WscControl.bWscTrigger)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPS is in Prgress; Not sending this LINK UP\n");
	else {
		if ((wdev->AuthMode != Ndis802_11AuthModeWPA2PSK)
			&& (wdev->AuthMode != Ndis802_11AuthModeWPAPSK)) {
			WidiUpdateStateToDaemon(pAd, MIN_NET_DEVICE_FOR_MBSSID,
									WIDI_MSG_TYPE_ASSOC_STATUS,
									pEntry->Addr, NULL, 0,
									WIDI_ASSOCIATED);
		} else
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Auth Mode is WPA2PSK or WPAPSK; Not sending this LINK UP\n");
	}

#endif /* WIDI_SUPPORT */
#ifdef WSC_STA_SUPPORT

	/*
	 *   1. WSC initial connect to AP, set the correct parameters
	 *   2. When security of Marvell WPS AP is OPEN/NONE, Marvell AP will send EAP-Req(ID) to STA immediately.
	 *   STA needs to receive this EAP-Req(ID) on time, because Marvell AP will not send again after STA sends EAPOL-Start.
	 */
	if (do_wsc_now) {
#ifdef DOT11R_FT_SUPPORT
		pStaCfg->Dot11RCommInfo.bInMobilityDomain = FALSE;
		pStaCfg->Dot11RCommInfo.FtRspSuccess = 0;
#endif /* DOT11R_FT_SUPPORT */
		RTMPCancelTimer(&pStaCfg->LinkDownTimer, &Cancelled);
		wdev->WscControl.WscState = WSC_STATE_LINK_UP;
		wdev->WscControl.WscStatus = WSC_STATE_LINK_UP;
		NdisZeroMemory(pStaCfg->Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pStaCfg->Ssid, tmpWscSsid,
					   tmpWscSsidLen);
		pStaCfg->SsidLen = tmpWscSsidLen;
	} else {
#ifdef CONFIG_AP_SUPPORT
		/*
		* 1. Since context is LinkUp_Infra, which is called for ApCli Mode
		* 2. Setting second parameter which indicates ApCliMode as TRUE.
		*/
		WscStop(pAd, TRUE, &wdev->WscControl);
#else /* CONFIG_AP_SUPPORT */
		WscStop(pAd, &wdev->WscControl);
#endif
	}

#endif /* WSC_STA_SUPPORT */
	MlmeUpdateTxRatesWdev(pAd, TRUE, wdev);
#ifdef DOT11_N_SUPPORT
	MlmeUpdateHtTxRates(pAd, wdev);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! LINK UP !! (StaActive.bHtEnable =%d)\n",
			 pStaCfg->StaActive.SupportedPhyInfo.bHtEnable);
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	MlmeUpdateVhtTxRates(pAd, pEntry, wdev);
#endif /* DOT11_VHT_AC */

	/* TODO: shiang-usw, revise this! */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)) {
		AsicSetPiggyBack(pAd, TRUE);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Turn on Piggy-Back\n");
	}

#ifdef DOT11_N_SUPPORT

	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE)
		&& CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) {
		/* AsicSetRDG(pAd, pEntry->wcid, HcGetBandByWdev(pEntry->wdev), 1, 1); */
	}

#endif /* DOT11_N_SUPPORT */
}


/*
 *	==========================================================================
 *	Description:
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *	==========================================================================
 */
VOID LinkUp(RTMP_ADAPTER *pAd, UCHAR BssType, struct wifi_dev *wdev, UINT link_up_type, MLME_QUEUE_ELEM *Elem)
{
	ULONG Now;
	BOOLEAN Cancelled, bDoTxRateSwitch = FALSE;
	/* UCHAR idx = 0; */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UCHAR tmpWscSsid[MAX_LEN_OF_SSID] = { 0 };
	UCHAR tmpWscSsidLen = 0;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	UCHAR BandIdx;
	ADD_HT_INFO_IE *addht;
	struct DOT11_H *pDot11h = NULL;
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#ifdef RT_CFG80211_SUPPORT
	UCHAR idx;
#endif

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;
	if (wdev == NULL)
		return;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	BandIdx = HcGetBandByWdev(wdev);
#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable[BandIdx])
			TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[MAIN_MBSSID];
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *p2p_wdev = NULL;
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
	/* Init ChannelQuality to prevent DEAD_CQI at initial LinkUp */
	pStaCfg->ChannelQuality = 50;
	/* init to not doing improved scan */
	ScanInfo->bImprovedScan = FALSE;
	pStaCfg->bNotFirstScan = TRUE;
	pStaCfg->bAutoConnectByBssid = FALSE;
#ifdef STA_LP_PHASE_2_SUPPORT
	pStaCfg->CountDowntoPsm = STAY_10_SECONDS_AWAKE;
#else
#endif /* STA_LP_PHASE_2_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	pStaCfg->Dot11RCommInfo.FtRspSuccess = 0;

	if ((BssType == BSS_INFRA) && (pStaCfg->Dot11RCommInfo.bFtSupport)) {
		if (pStaCfg->Dot11RCommInfo.bInMobilityDomain == FALSE) {
			FT_SET_MDID(pStaCfg->Dot11RCommInfo.MdIeInfo.MdId,
						pStaCfg->MlmeAux.MdIeInfo.MdId);
			NdisZeroMemory(pStaCfg->Dot11RCommInfo.R0khId,
						   FT_ROKH_ID_LEN);
			NdisMoveMemory(pStaCfg->Dot11RCommInfo.R0khId,
						   pStaCfg->MlmeAux.FtIeInfo.R0khId,
						   pStaCfg->MlmeAux.FtIeInfo.R0khIdLen);
			pStaCfg->Dot11RCommInfo.R0khIdLen = pStaCfg->MlmeAux.FtIeInfo.R0khIdLen;

			/*if (pStaCfg->AuthMode == Ndis802_11AuthModeWPA2PSK) {*/
			if (IS_AKM_WPA2PSK(pStaCfg->wdev.SecConfig.AKMMap)) {
				/* Update PMK. */
				if (pStaCfg->WpaPassPhraseLen == 64)
					AtoH(pStaCfg->WpaPassPhrase, pStaCfg->PMK, 32);
				else {
					MAC_TABLE_ENTRY *pMacEntry = GetAssociatedAPByWdev(pAd, wdev);
					struct _SECURITY_CONFIG *pProfile_SecConfig = &wdev->SecConfig;
					struct _SECURITY_CONFIG *pEntry_SecConfig = &pMacEntry->SecConfig;
					/*RtmpPasswordHash(pStaCfg->WpaPassPhrase,
									 pStaCfg->MlmeAux.Ssid,
									 pStaCfg->MlmeAux.SsidLen,
									 keyMaterial);
					NdisMoveMemory(pStaCfg->PMK, keyMaterial, 32);
					*/
					SetWPAPSKKey(pAd, pProfile_SecConfig->PSK,
						strlen(pProfile_SecConfig->PSK), pStaCfg->MlmeAux.Ssid,
						pStaCfg->MlmeAux.SsidLen, pEntry_SecConfig->PMK);

				}
			}
		} else {
			/* Update Reconnect Ssid, that user desired to connect. */
			NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid,
						   pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
			pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->MlmeAux.SsidLen;
		}
	}

#endif /* DOT11R_FT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		REPEATER_CLIENT_ENTRY *rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
		pEntry = rept->pMacEntry;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pEntry = GetAssociatedAPByWdev(pAd, wdev);


	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev(type=%d,func_idx=%d),pEntry=NULL,return!\n",
			wdev->wdev_type, wdev->func_idx);
		return;
	}

	addht = wlan_operate_get_addht(wdev);
	tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
	/*
		ASSOC - DisassocTimeoutAction
		CNTL - Dis-associate successful
		!!! LINK DOWN !!!
		[88888] OID_802_11_SSID should have returned NDTEST_WEP_AP2(Returned: )
	*/
	/*
		To prevent DisassocTimeoutAction to call Link down after we link up,
		cancel the DisassocTimer no matter what it start or not.
	*/
	RTMPCancelTimer(&pStaCfg->MlmeAux.DisassocTimer, &Cancelled);
#ifdef WSC_STA_SUPPORT
	tmpWscSsidLen = pStaCfg->SsidLen;
	NdisMoveMemory(tmpWscSsid, pStaCfg->Ssid, tmpWscSsidLen);
#endif /* WSC_STA_SUPPORT */
	COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd, pStaCfg, pEntry);
#ifdef APCLI_SUPPORT
	/* If PE is supported then save the connected SSID and corresponding MAC
	 * when connected SSID is not matching with the saved one
	 */
	if (wdev->SecConfig.apcli_pe_support) {
		UCHAR ifIndex = wdev->func_idx;

		if (!SSID_EQUAL(pAd->StaCfg[ifIndex].Ssid,
				pAd->StaCfg[ifIndex].SsidLen,
				wdev->SecConfig.pe_latest_connected_Ssid,
				wdev->SecConfig.pe_latest_connected_SsidLen)) {
			NdisMoveMemory(wdev->SecConfig.pe_latest_connected_Ssid,
					pAd->StaCfg[ifIndex].Ssid, pAd->StaCfg[ifIndex].SsidLen);
			wdev->SecConfig.pe_latest_connected_SsidLen = pAd->StaCfg[ifIndex].SsidLen;
			NdisMoveMemory(wdev->SecConfig.pe_latest_connected_macaddr, wdev->if_addr, MAC_ADDR_LEN);
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"PE: saved pe_latest_connected_macaddr:%02X:%02X:%02X:%02X:%02X:%02X\n",
				MAC2STR(wdev->SecConfig.pe_latest_connected_macaddr));
		}
	}
#endif
#ifdef DOT11_N_SUPPORT
	COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd, pEntry, pStaCfg);
#endif /* DOT11_N_SUPPORT */
#ifdef P2P_SUPPORT
	{
		PSTA_ADMIN_CONFIG pApCliEntry = NULL;

		pApCliEntry = &pAd->StaCfg[BSS0];

		if (P2P_GO_ON(pAd) || (pApCliEntry->ApcliInfStat.Valid == TRUE)) {
#ifdef DOT11_N_SUPPORT

			if (pStaCfg->MlmeAux.bBwFallBack == TRUE)
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);

#endif /* DOT11_N_SUPPORT */
		}
	}
#endif /* P2P_SUPPORT */
#ifdef CONFIG_STA_ADHOC_SUPPORT
	if (BssType == BSS_ADHOC) {
		struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;

		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
#ifdef CARRIER_DETECTION_SUPPORT	/* Roger sync Carrier */
		/* No carrier detection when adhoc */
		/* CarrierDetectionStop(pAd); */
		pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(wdev->PhyMode)
			&& (adhocInfo->bAdhocN == TRUE))
			AdhocTurnOnQos(pAd, pStaCfg);

#endif /* DOT11_N_SUPPORT */
	} else
#endif /* CONFIG_STA_ADHOC_SUPPORT */
	{
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
#ifdef COEX_SUPPORT
		if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd)) {
			if ((pAd->BtWlanStatus & COEX_STATUS_LINK_UP) != 0)
				MT76xxMLMEHook(pAd, MT76xx_WLAN_LINK_DONE, 0);
		}
#endif /* COEX_SUPPORT */

	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!!%s LINK UP !!!\n", (BssType == BSS_ADHOC ? "ADHOC" : "Infra"));
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 "!!! LINK UP !!! wdev(name=%s,type=%d,func_idx=%d,PortSecured=%d%d)\n",
			  wdev->if_dev->name, wdev->wdev_type, wdev->func_idx, tr_entry->PortSecured, wdev->PortSecured);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 "BssType=%d, AID=%d, ssid=%s, Channel=%d, CentralChannel = %d\n",
			  BssType, pStaCfg->StaActive.Aid, pStaCfg->Ssid, wdev->channel, wlan_operate_get_cen_ch_1(wdev));
	pStaCfg->ApcliInfStat.ApCliRcvBeaconTime = pAd->Mlme.Now32;
	NdisGetSystemUpTime(&Now);
	pStaCfg->LastBeaconRxTime = Now;	/* last RX timestamp */
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "!!! LINK UP !!! (Density =%d, )\n", pEntry->MpduDensity);
#endif /* DOT11_N_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
	/*  Enable stream mode for BSSID MAC Address */
	pEntry->StreamModeMACReg = TX_CHAIN_ADDR1_L;
	AsicSetStreamMode(pAd, &pStaCfg->Bssid[0], 1, TRUE);
#endif /* STREAM_MODE_SUPPORT */
	HW_SET_SLOTTIME(pAd, TRUE, wdev->channel, wdev);
	/*
	 *	Call this for RTS protectionfor legacy rate, we will always enable RTS threshold,
	 *	but normally it will not hit
	 */
#ifdef DOT11_N_SUPPORT

	if (pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == TRUE) {
		ADD_HTINFO2 *ht_info2 = &pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2;

		wdev->protection &= ~(SET_PROTECT(ERP));
		wdev->protection |= SET_PROTECT(ht_info2->OperaionMode);

		if (ht_info2->NonGfPresent == 1)
			wdev->protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
		else
			wdev->protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));

		HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
	}

#endif /* DOT11_N_SUPPORT */

	if ((pAd->CommonCfg.TxPreamble != Rt802_11PreambleLong) &&
		CAP_IS_SHORT_PREAMBLE_ON(pStaCfg->StaActive.CapabilityInfo)) {
		MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
	}

	pDot11h->RDMode = RD_NORMAL_MODE;

#ifdef CONFIG_STA_ADHOC_SUPPORT
	if (BssType == BSS_ADHOC)
		LinkUp_Adhoc(pAd, wdev, pEntry);
	else
#endif /* CONFIG_STA_ADHOC_SUPPORT */
		LinkUp_Infra(pAd, wdev, pEntry, tmpWscSsid, tmpWscSsidLen, link_up_type, Elem);

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT

	/*
	 *	LED indication.
	 *
	 *
	 *	LEDConnectionCompletion(pAd, TRUE);
	 *
	 *	If we call LEDConnectionCompletion, it will enqueue cmdthread to send asic mcu
	 *	command, this may be happen that sending the mcu command with
	 *	LED_NORMAL_CONNECTION_WITH_SECURITY will be later than sending the mcu
	 *	command with LED_LINK_UP.
	 *
	 *	It will cause Tx power LED be unusual after scanning, since firmware
	 *	only do Tx power LED in link up state.
	 */
	/*if (pStaCfg->WscControl.bWPSSession == FALSE) */
	if (wdev->WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) {	/* LED mode 9. */
			UCHAR led_status;

			if (IS_NO_SECURITY_Entry(wdev))
				led_status = LED_NORMAL_CONNECTION_WITHOUT_SECURITY;
			else
				led_status = LED_NORMAL_CONNECTION_WITH_SECURITY;

			RTMPSetLED(pAd, led_status, HcGetBandByWdev(wdev));
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LedConnSecurityStatus(%d)\n",
					 led_status);
		}
	}

#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11_N_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "NDIS_STATUS_MEDIA_CONNECT Event B!.BACapability = %x. ClientStatusFlags = %lx\n",
			  pAd->CommonCfg.BACapability.word,
			  pEntry->ClientStatusFlags);
#endif /* DOT11_N_SUPPORT */
#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP, HcGetBandByWdev(wdev));
#endif /* LED_CONTROL_SUPPORT */
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->BcnCheckInfo[DBDC_BAND0].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pAd->BcnCheckInfo[DBDC_BAND1].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pStaCfg->bConfigChanged = FALSE;	/* Reset config flag */
	pAd->ExtraInfo = GENERAL_LINK_UP;	/* Update extra information to link is up */
	NdisAcquireSpinLock(&pAd->MacTabLock);
	pEntry->HTPhyMode.word = wdev->HTPhyMode.word;

#ifdef RT_CFG80211_SUPPORT
	for (idx = 0; idx < BAND_NUM_MAX; idx++) {
		if (pAd->BcnCheckInfo[idx].BcnInitedRnd > pAd->Mlme.PeriodicRound) {
			pAd->BcnCheckInfo[idx].BcnInitedRnd = pAd->Mlme.PeriodicRound;
		}
	}
#endif

	if (wdev->bAutoTxRateSwitch == FALSE) {
		bDoTxRateSwitch = FALSE;
#ifdef DOT11_N_SUPPORT

		if (pEntry->HTPhyMode.field.MCS == 32)
			pEntry->HTPhyMode.field.ShortGI = GI_800;

		if ((pEntry->HTPhyMode.field.MCS > HT_MCS_7)
			|| (pEntry->HTPhyMode.field.MCS == 32))
			pEntry->HTPhyMode.field.STBC = STBC_NONE;

#endif /* DOT11_N_SUPPORT */

		/* If the legacy mode is set, overwrite the transmit setting of this entry. */
		if (pEntry->HTPhyMode.field.MODE <= MODE_OFDM)
			RTMPUpdateLegacyTxSetting((UCHAR) wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
	} else
		bDoTxRateSwitch = TRUE;

	NdisReleaseSpinLock(&pAd->MacTabLock);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (bDoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->bAutoTxRateSwitch = FALSE;

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}
		}

		RAInit(pAd, pEntry);
#ifdef TXBF_SUPPORT
		/* Check whether disable/enable BF souding timer for STA mode only */
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Send HW_STA_BF_SOUNDING_ADJUST Cmd\n");
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			HW_STA_BF_SOUNDING_ADJUST(pAd, STATE_CONNECTED, wdev);
		}
#endif /* TXBF_SUPPORT */
	}

#endif /* MT_MAC */
	/*  Let Link Status Page display first initial rate. */
	pAd->LastTxRate = (USHORT) (pEntry->HTPhyMode.word);
	AsicSetTxStream(pAd, TxPath, OPMODE_STA, TRUE, BandIdx);
	pAd->CommonCfg.IOTestParm.bLastAtheros = FALSE;
#ifdef WPA_SUPPLICANT_SUPPORT

	/*
	 *   If STA connects to different AP, STA couldn't send EAPOL_Start for WpaSupplicant.
	 */
	if ((pStaCfg->BssType == BSS_INFRA) &&
		(IS_AKM_WPA2_Entry(wdev)) &&
		(NdisEqualMemory(pStaCfg->Bssid, pStaCfg->LastBssid, MAC_ADDR_LEN) == FALSE) &&
		(pStaCfg->wpa_supplicant_info.bLostAp == TRUE))
		pStaCfg->wpa_supplicant_info.bLostAp = FALSE;

#endif /* WPA_SUPPLICANT_SUPPORT */
	/*
	 *   Need to check this COPY. This COPY is from Windows Driver.
	 */
	COPY_MAC_ADDR(pStaCfg->LastBssid, pStaCfg->Bssid);
	/* BSSID add in one MAC entry too.  Because in Tx, ASIC need to check Cipher and IV/EIV, BAbitmap */
	/* Pther information in MACTab.Content[BSSID_WCID] is not necessary for driver. */
	/* Note: As STA, The MACTab.Content[BSSID_WCID]. PairwiseKey and Shared Key for BSS0 are the same. */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	/*RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW); */
#ifdef WSC_STA_SUPPORT

	/* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */
	if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger
		)) {
		RtmpusecDelay(100000);	/* 100 ms */

		if (pStaCfg->BssType == BSS_INFRA) {
			wdev->WscControl.WscState = WSC_STATE_LINK_UP;
			wdev->WscControl.WscStatus = WSC_STATE_LINK_UP;
			wdev->WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
			NdisMoveMemory(wdev->WscControl.WscPeerMAC,
						   pStaCfg->Bssid, MAC_ADDR_LEN);
			NdisMoveMemory(wdev->WscControl.EntryAddr,
						   pStaCfg->Bssid, MAC_ADDR_LEN);
			WscSendEapolStart(pAd,
							  wdev->WscControl.WscPeerMAC,
							  STA_MODE,
							  wdev);
		}
#ifdef CONFIG_MAP_SUPPORT
		if ((IS_MAP_TURNKEY_ENABLE(pAd)) && (pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
				(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_STA))) {
			wdev->WscControl.entryBackhaulSta = TRUE;
		}
#endif /* CONFIG_MAP_SUPPORT */

	}

#endif /* WSC_STA_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (INFRA_ON(pStaCfg)) {
		if ((pAd->CommonCfg.bBssCoexEnable == TRUE)
			&& (wdev->channel <= 14)
			&& (pStaCfg->StaActive.SupportedPhyInfo.bHtEnable == TRUE)
			&& (pStaCfg->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport == 1)) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SCAN_2040);
			BuildEffectedChannelList(pAd, wdev);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "LinkUP AP supports 20/40 BSS COEX, Dot11BssWidthTriggerScanInt[%d]\n",
					  pAd->CommonCfg.Dot11BssWidthTriggerScanInt);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "not supports 20/40 BSS COEX !!!\n");
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "pAd->CommonCfg Info: bBssCoexEnable=%d, Channel=%d, CentralChannel=%d, PhyMode=%d\n",
					  pAd->CommonCfg.bBssCoexEnable, wdev->channel,
					  wlan_operate_get_cen_ch_1(wdev), wdev->PhyMode);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "pStaCfg->StaActive.SupportedHtPhy.bHtEnable=%d\n",
					  pStaCfg->StaActive.SupportedPhyInfo.bHtEnable);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "pStaCfg->MlmeAux.ExtCapInfo.BssCoexstSup=%d\n",
					  pStaCfg->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport);
		}
	}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT

	/*
	 *   When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
	 *   WpaSupplicant would not send EapolStart to AP after STA re-connect to AP again.
	 *   In this case, driver would send EapolStart to AP.
	 */
	if ((pStaCfg->BssType == BSS_INFRA) &&
		(IS_AKM_WPA2_Entry(wdev)) &&
		(NdisEqualMemory
		 (pStaCfg->Bssid, pStaCfg->LastBssid, MAC_ADDR_LEN))
		&& (pStaCfg->wpa_supplicant_info.bLostAp == TRUE)) {
		WpaSendEapolStart(pAd, pStaCfg->Bssid, &pStaCfg->wdev);
		pStaCfg->wpa_supplicant_info.bLostAp = FALSE;
	}

#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef CONFIG_MULTI_CHANNEL
	/*if INFRA connect and GO not ready , make pAd->Mlme.BeaconSyncafterAP = TRUE for */
#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
		p2p_wdev = &pMbss->wdev;
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
		p2p_wdev = &pApCliEntry->wdev;

	/* start MCC when Infra connect */
	if (p2p_wdev
		&& ((wdev->AuthMode == Ndis802_11AuthModeOpen) && !(pStaCfg->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)) /* WPS not under goining! */
	   ) {
		UCHAR op_ht_bw1 = wlan_operate_get_ht_bw(wdev);
		UCHAR op_ht_bw2 = wlan_operate_get_ht_bw(p2p_wdev);

		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "p2p_wdev->channel %d wdev->channel %d\n", p2p_wdev->channel, wdev->channel);

		if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) {
			if ((op_ht_bw1 != op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))) {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "start bw !=  && P2P GO SCC\n");
				pAd->Mlme.bStartScc = TRUE;
			} else if ((((op_ht_bw1 == op_ht_bw2) && (wdev->channel != p2p_wdev->channel))
						 || !((op_ht_bw1 == op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))))) {
				LONG timeDiff;
				INT starttime = pAd->Mlme.channel_1st_staytime;

				NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
				timeDiff = (pAd->Mlme.BeaconNow32 - pStaCfg->LastBeaconRxTime) %
					(pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)]);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "#####pAd->Mlme.Now32 %lu pStaCfg->LastBeaconRxTime %lu\n", pAd->Mlme.BeaconNow32, pStaCfg->LastBeaconRxTime);
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "####    timeDiff %ld\n", timeDiff);
				AsicDisableSync(pAd, HW_BSSID_0);

				if (starttime > timeDiff)
					OS_WAIT((starttime - timeDiff));
				else
					OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)] - timeDiff)));

				AsicEnableApBssSync(pAd, pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)]);
				Start_MCC(pAd);
				/* pAd->MCC_DHCP_Protect = TRUE; */
			}
		} else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)) {
			pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

			if (pMacEntry) {
				if (IS_ENTRY_PEER_AP(pMacEntry) && (pMacEntry->PairwiseKey.KeyLen == LEN_TK)) { /* P2P GC will have security */
					if ((op_ht_bw1 != op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))) {
						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "start bw !=  && P2P GC SCC\n");
						pAd->Mlme.bStartScc = TRUE;
					}

					if ((((op_ht_bw1 == op_ht_bw2) && (wdev->channel != p2p_wdev->channel))
						 || !((op_ht_bw1 == op_ht_bw2) && ((wdev->channel == p2p_wdev->channel))))) {
						Start_MCC(pAd);
						/* pAd->MCC_DHCP_Protect = TRUE; */
					}
				}
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#endif /*RT_CFG80211_SUPPORT */
#endif /* CONFIG_MULTI_CHANNEL */
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
#ifdef MICROWAVE_OVEN_SUPPORT
	pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef COEX_SUPPORT
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd)) {
		AndesCoexProtectionFrameOP(pAd, COEX_Legacy_CCK, LONG_PREAMBLE_1M);

		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
			AndesCoexBSSInfo(pAd, 1, 1);
		else
			AndesCoexBSSInfo(pAd, 1, 0);
	}

#endif /* COEX_SUPPORT */

	pStaCfg->wdev.bLinkUpDone = TRUE;
#ifdef RT_CFG80211_SUPPORT

	if (pStaCfg->wdev.bGotEapolPkt && pStaCfg->wdev.pEapolPktFromAP) {
		indicate_802_11_pkt(pAd, pStaCfg->wdev.pEapolPktFromAP, pStaCfg->wdev.wdev_idx);
		pStaCfg->wdev.bGotEapolPkt = FALSE;
	}

#endif /* RT_CFG80211_SUPPORT */

	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"!!! LINK UP !!! wdev(name=%s,type=%d,PortSecured=%d/%d),Root Ap:"MACSTR"\n",
		wdev->if_dev->name, wdev->wdev_type, tr_entry->PortSecured, wdev->PortSecured, MAC2STR(tr_entry->Addr));
#ifdef ZERO_LOSS_CSA_SUPPORT
	/* CSA SYNC / TSF SYNC */
	if (wdev->wdev_type == WDEV_TYPE_STA
		&& IS_SECURITY_OPEN_NONE_Entry(pEntry) && IS_MAP_TURNKEY_ENABLE(pAd)) {

		wdev->bss_info_argument.u4BssInfoFeature = BSS_INFO_APCLI_TSF_SYNC_FEATURE;
		wdev->bss_info_argument.bss_state = BSS_ACTIVE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s Apply the bssinfo, BSSID=%d!, feature = %d\n", __func__,
			wdev->bss_info_argument.ucBssIndex, wdev->bss_info_argument.u4BssInfoFeature);


		if (AsicBssInfoUpdate(pAd, &wdev->bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Fail to apply the bssinfo, BSSID!\n");
	}
#endif

#ifdef MTFWD
#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type != WDEV_TYPE_REPEATER)
#endif
		RTMP_OS_NETDEV_CARRIER_ON(wdev->if_dev);
#endif
}

#ifdef CONFIG_STA_ADHOC_SUPPORT
INT LinkDown_Adhoc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef ADHOC_WPA2PSK_SUPPORT
	INT i;
#endif /* ADHOC_WPA2PSK_SUPPORT */
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, "!!! LINK DOWN 1!!!\n");
#ifdef ADHOC_WPA2PSK_SUPPORT

	/* In an IBSS, a STA's SME responds to Deauthenticate frames from a STA by */
	/* deleting the PTKSA associated with that STA. (Spec. P802.11i/D10 P.19) */
	if (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK) {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]))
				MlmeDeAuthAction(pAd,
								 &pAd->MacTab.Content[i],
								 REASON_DEAUTH_STA_LEAVING,
								 FALSE);
		}
	}

#endif /* ADHOC_WPA2PSK_SUPPORT */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
	pAd->ExtraInfo = GENERAL_LINK_DOWN;
	BssTableDeleteEntry(ScanTab, pStaCfg->Bssid, wdev->channel);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MacTab.Size=%d\n", pAd->MacTab.Size);

	if (wdev_do_linkdown(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkdown fail!");

#ifdef RT_CFG80211_SUPPORT
	CFG80211OS_DelSta(pAd->net_dev, pStaCfg->Bssid);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "del this ad-hoc "MACSTR"\n",
			 MAC2STR(pStaCfg->Bssid));
#endif /* RT_CFG80211_SUPPORT */
	return TRUE;
}
#endif /* CONFIG_STA_ADHOC_SUPPORT */


INT LinkDown_Infra(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN ReqByAP, MLME_QUEUE_ELEM *Elem)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, wdev);
#endif
	USHORT ifIndex = wdev->func_idx;
#ifndef APCLI_CFG80211_SUPPORT
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
#endif
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept = NULL;
#endif

	ASSERT(pStaCfg);

#ifdef MAC_REPEATER_SUPPORT
	if (!pEntry) {
		if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
			rept = (REPEATER_CLIENT_ENTRY *) wdev->func_dev;
			if (VALID_UCAST_ENTRY_WCID(pAd, rept->pMacEntry->wcid))
			pEntry = rept->pMacEntry;
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	if (!pStaCfg)
		return FALSE;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"pEntry NULL, this is possible in this flow\n");
		return FALSE;
	}
#endif
	/* Infra structure mode */
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! LINK DOWN INFRA !!!\n");
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "!!! wdev_type = %u, Idx %d !!!\n", wdev->wdev_type, ifIndex);
#ifdef STA_LP_PHASE_2_SUPPORT
	pStaCfg->PwrMgmt.bBeaconLost = FALSE;
	pStaCfg->PwrMgmt.bTriggerRoaming = FALSE;
#endif /* STA_LP_PHASE_2_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT

	if (IS_TDLS_SUPPORT(pAd))
		TDLS_LinkTearDown(pAd, TRUE);

#endif /* DOT11Z_TDLS_SUPPORT */

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
		wdev->rx_drop_long_len = 0;
	}
	/* Saved last SSID for linkup comparison */
	pStaCfg->LastSsidLen = pStaCfg->SsidLen;
	NdisMoveMemory(pStaCfg->LastSsid,
				   pStaCfg->Ssid,
				   pStaCfg->LastSsidLen);
	COPY_MAC_ADDR(pStaCfg->LastBssid,
				  pStaCfg->Bssid);

#ifndef APCLI_CFG80211_SUPPORT
	if (pStaCfg->MlmeAux.CurrReqIsFromNdis == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NDIS_STATUS_MEDIA_DISCONNECT Event A!\n");
		pStaCfg->MlmeAux.CurrReqIsFromNdis = FALSE;
	} else {
		if ((wdev->PortSecured == WPA_802_1X_PORT_SECURED)
			|| IS_AKM_OPEN(wdev->SecConfig.AKMMap)
			|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
			|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
			/*
			 *	If disassociation request is from NDIS, then we don't need
			 *	to delete BSSID from entry. Otherwise lost beacon or receive
			 *	De-Authentication from AP, then we should delete BSSID
			 *	from BssTable.
			 *
			 *	If we don't delete from entry, roaming will fail.
			 */
			BssTableDeleteEntry(ScanTab, pStaCfg->Bssid, wdev->channel);
		}
	}
#endif

	/*
	 *	restore back to -
	 *	1. long slot (20 us) or short slot (9 us) time
	 *	2. turn on/off RTS/CTS and/or CTS-to-self protection
	 *	3. short preamble
	 */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
#ifdef EXT_BUILD_CHANNEL_LIST

	/* Country IE of the AP will be evaluated and will be used. */
	if (pStaCfg->IEEE80211dClientMode != Rt802_11_D_None) {
		NdisMoveMemory(&pAd->CommonCfg.CountryCode[0],
					   &pStaCfg->StaOriCountryCode[0], 2);
		pAd->CommonCfg.Geography = pStaCfg->StaOriGeography;
		BuildChannelListEx(pAd, wdev);
	}

#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef COEX_SUPPORT
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
		AndesCoexBSSInfo(pAd, 0, 0);

#endif /* COEX_SUPPORT */
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	MAC_TABLE_ENTRY	*pMacEntry = (MAC_TABLE_ENTRY *)NULL;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	struct wifi_dev *pWdev = &pMbss->wdev;
	PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[MAIN_MBSSID];

	pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
	/* pStaCfg->MlmeAux.InfraChannel = 0; */
	pStaCfg->wdev.channel = 0;
	pStaCfg->wdev.CentralChannel = 0;
	wlan_operate_set_ht_bw(pStaCfg, HT_BW_20, EXTCHA_NONE);

	if (pAd->Mlme.bStartMcc == TRUE) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		cap->tssi_enable = TRUE; /* let host do tssi */
		pAd->Mlme.bStartMcc = FALSE;

		if (((pMacEntry->PairwiseKey.KeyLen == LEN_TK) && IS_ENTRY_PEER_AP(pMacEntry)) || RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) { /* yiwei : PairwiseKey seems not clear in MacTableDeleteEntry() */
			/* need to park at GC channel */
			Stop_MCC(pAd, 1);
		} else {
			/* TODO :  need to care the wps case. */
			/* tmply don't care */
			Stop_MCC(pAd, 0);
		}
	}

	if (pAd->Mlme.bStartScc == TRUE) {
		pAd->Mlme.bStartScc = FALSE;

		if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) {
			UCHAR cfg_ht_bw = wlan_config_get_ht_bw(pWdev);
			UCHAR ext_cha = wlan_config_get_ext_cha(pWdev);

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, "link down and switch to GO bw %d   pWdev->CentralChannel  %d\n", op_ht_bw, pWdev->CentralChannel);
			wlan_operate_set_ht_bw(pWdev, cfg_ht_bw, ext_cha);
		}

		/*if p2p_cli*/
	}

#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (IS_HIF_TYPE(pAd, HIF_MT))
		pEntry->RaEntry.fgRaValid = FALSE;

#endif

	if (IF_COMBO_HAVE_AP_STA(pAd)) {
	/* apcli related action============================= */
		if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			|| (wdev->wdev_type == WDEV_TYPE_REPEATER)
#endif /* MAC_REPEATER_SUPPORT */
		   ) {

#ifdef MAC_REPEATER_SUPPORT
			if (wdev->wdev_type != WDEV_TYPE_REPEATER)
#endif /* MAC_REPEATER_SUPPORT */
			{
#ifdef MAC_REPEATER_SUPPORT
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
				if (wf_drv_tbl.wf_fwd_entry_delete_hook)
					wf_drv_tbl.wf_fwd_entry_delete_hook(pStaCfg->wdev.if_dev, pAd->net_dev, 1);

				if (wf_drv_tbl.wf_fwd_check_device_hook)
					wf_drv_tbl.wf_fwd_check_device_hook(pStaCfg->wdev.if_dev,
									INT_APCLI, NON_REPT_ENTRY, pStaCfg->wdev.channel, 0);

				wf_apcli_active_links--;
#endif /* defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE) */
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pStaCfg->ApCliMode == APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION) {
#ifdef WAPP_SUPPORT
			MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s() :APCLI LinkDown Event send to wapp\n", __func__);
			wapp_send_apcli_association_change_vendor10(WAPP_APCLI_DISASSOCIATED, pAd, pStaCfg);
#endif
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef CONFIG_FAST_NAT_SUPPORT
					if ((wf_apcli_active_links < 2) && (ra_sw_nat_hook_rx == NULL) && (wf_ra_sw_nat_hook_rx_bkup != NULL)) {
						ra_sw_nat_hook_rx = wf_ra_sw_nat_hook_rx_bkup;
						wf_ra_sw_nat_hook_rx_bkup = NULL;
					}
#endif /*CONFIG_FAST_NAT_SUPPORT*/
#endif /* defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE) */

#ifdef CONFIG_AP_SUPPORT
				pAd->ApCfg.ApCliInfRunned--;
#endif /* CONFIG_AP_SUPPORT */
				pStaCfg->ApcliInfStat.Valid = FALSE;	/* This link doesn't associated with any remote-AP */
				MSDU_FORBID_SET(&pStaCfg->wdev, MSDU_FORBID_CONNECTION_NOT_READY);
				pStaCfg->wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
#ifdef WIFI_IAP_STA_DUMP_FEATURE
				NdisZeroMemory(&pStaCfg->StaStatistic, sizeof(STAT_COUNTERS));/*clear statics*/
#endif /*WIFI_IAP_STA_DUMP_FEATURE*/
#ifndef APCLI_CFG80211_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
				BssTableDeleteEntry(ScanTab, pStaCfg->MlmeAux.Bssid, wdev->channel);
#endif /* DOT11W_PMF_SUPPORT */
#endif
#ifdef DOT11_N_SUPPORT
				wlan_operate_set_ht_bw(&pStaCfg->wdev,
									   wlan_config_get_ht_bw(&pStaCfg->wdev),
									   wlan_config_get_ext_cha(&pStaCfg->wdev));
#endif
#ifdef DOT11_VHT_AC
				wlan_operate_set_vht_bw(&pStaCfg->wdev, wlan_config_get_vht_bw(&pStaCfg->wdev));
#endif
				/* linkdown complete will be called in sta_cntl_disassoc_conf */
				/* RTMP_OS_COMPLETE(&pStaCfg->linkdown_complete); */
				pStaCfg->ApcliInfStat.bPeerExist = FALSE;
#ifdef MWDS
				MWDSAPCliPeerDisable(pAd, pStaCfg, &pAd->MacTab.Content[pStaCfg->MacTabWCID]);
#endif /* MWDS */

#ifdef CONFIG_MAP_SUPPORT
				map_a4_peer_disable(pAd, &pAd->MacTab.Content[pStaCfg->MacTabWCID], FALSE);
				if (IS_MAP_TURNKEY_ENABLE(pAd)) {
					pStaCfg->ApcliInfStat.Enable = FALSE;
				}

				if (IS_MAP_ENABLE(pAd))
					wapp_send_apcli_association_change(WAPP_APCLI_DISASSOCIATED, pAd, pStaCfg);
#endif /*WAPP_SUPPORT*/
			}

#ifdef MAC_REPEATER_SUPPORT
			else {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
				if (wf_drv_tbl.packet_source_delete_entry_hook)
							wf_drv_tbl.packet_source_delete_entry_hook(100);
#endif /*defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)*/
				NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				if (IS_REPT_IN_DISCONNECTING(rept)) {
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev(type=%d,fun_idx=%d) in disconnecting, return\n",
						wdev->wdev_type, wdev->func_idx);
					NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
					return FALSE;
				}

				if (IS_REPT_LINK_UP(rept) && !IS_REPT_IN_DISCONNECTING(rept)) {
					rept->CliDisconnectState = REPT_ENTRY_DISCONNT_STATE_DISCONNTING;
				}
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				HW_REMOVE_REPT_ENTRY(pAd, wdev->func_idx);
			}
#endif /* MAC_REPEATER_SUPPORT */
			/* MacTableDeleteEntry & wdev_do_link_down is done in LinkDown() */
		}
	}
	/* apcli related action end ============================= */
	return TRUE;
}

#ifdef CONFIG_MAP_SUPPORT
VOID ApCliCheckConConnectivity(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pApCliEntry, BCN_IE_LIST *ie_list)
{
	struct _vendor_ie_cap *vendor_ie = &ie_list->vendor_ie;
	UINT32 TotalLen = 0;
	UCHAR *msg;
	struct wifi_dev *wdev;
	struct wapp_event *event;
	wdev = &pApCliEntry->wdev;

	if (!IS_MAP_TURNKEY_ENABLE(pAd))
		return;
	if (!wdev || !wdev->if_dev)
		return;
	if (pApCliEntry->last_controller_connectivity != vendor_ie->map_info.connectivity_to_controller) {
		TotalLen = sizeof(CHAR) * 2 + sizeof(struct map_vendor_ie) + sizeof(UINT32);
		os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);
		if (msg == NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"failed to allocate memory\n");
			return;
		}
		event = (struct wapp_event *)msg;
		event->event_id = WAPP_MAP_VENDOR_IE;
		event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		NdisCopyMemory(&event->data, &vendor_ie->map_info, sizeof(struct map_vendor_ie));
		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"send connectivity change event to user space %u %u\n",
					pApCliEntry->last_controller_connectivity,
					vendor_ie->map_info.connectivity_to_controller);
		pApCliEntry->last_controller_connectivity = vendor_ie->map_info.connectivity_to_controller;
		os_free_mem((PUCHAR)msg);
	}
}
#else
VOID ApCliCheckConConnectivity(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pApCliEntry, BCN_IE_LIST *ie_list)
{ }
#endif
/*
 *	==========================================================================
 *
 *	Routine	Description:
 *		Disconnect current BSSID
 *
 *	Arguments:
 *		pAd				- Pointer to our adapter
 *		IsReqFromAP		- Request from AP
 *
 *	Return Value:
 *		None
 *
 *	IRQL = DISPATCH_LEVEL
 *
 *	Note:
 *		We need more information to know it's this requst from AP.
 *		If yes! we need to do extra handling, for example, remove the WPA key.
 *		Otherwise on 4-way handshaking will faied, since the WPA key didn't be
 *		remove while auto reconnect.
 *		Disconnect request from AP, it means we will start afresh 4-way handshaking
 *		on WPA mode.
 *
 *	==========================================================================
 */
VOID LinkDown(RTMP_ADAPTER *pAd, UINT linkdown_type, struct wifi_dev *wdev, MLME_QUEUE_ELEM *Elem)
{
	UCHAR i;
	BOOLEAN req_from_ap = linkdown_type & LINK_REQ_FROM_AP;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	SCAN_INFO *ScanInfo = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept = NULL;
#endif
	UINT8 TxPath = pAd->Antenna.field.TxPath;

	if (wdev) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wdev(type=%d,func_idx=%d)(caller:%pS)\n",
			wdev->wdev_type, wdev->func_idx, OS_TRACE);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" wdev(null)(caller:%pS)\n",
			OS_TRACE);
		return;
	}

	pStaCfg = GetStaCfgByWdev(pAd, wdev);

#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_REPEATER) {
		rept = (REPEATER_CLIENT_ENTRY *)wdev->func_dev;

		if (!rept)
			return;

		if (!rept->pMacEntry)
			return;

		if (VALID_UCAST_ENTRY_WCID(pAd, rept->pMacEntry->wcid))
			pEntry = rept->pMacEntry;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pEntry = GetAssociatedAPByWdev(pAd, wdev);
		if (pEntry != &pAd->MacTab.Content[pStaCfg->MacTabWCID])
			return;
	}

	if (!pStaCfg)
		return;

	if (!pEntry) /* Nothig to linkdown */
		return;

	/* Do nothing if monitor mode is on */
	if (MONITOR_ON(pAd))
		return;

	if (wdev != pEntry->wdev)
		return;

	ScanInfo = &pStaCfg->wdev.ScanInfo;

	BandIdx = HcGetBandByWdev(wdev);
#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable[BandIdx])
			TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */


	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" wdev(type=%d,func_idx=%d),pEntry=%p,wcid=%d,addr="MACSTR"\n",
		 wdev->wdev_type, wdev->func_idx, (PVOID)pEntry, pEntry->wcid, MAC2STR(pEntry->Addr));
#ifdef MT_WOW_SUPPORT

	if (pAd->WOW_Cfg.bWoWRunning) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " WoW is running, skip!\n");
		return;
	}

#endif
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (pStaCfg->PwrMgmt.bDoze
			|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)) {
			RTMP_FORCE_WAKEUP(pAd, pStaCfg);
			pStaCfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
		}
	}

#endif /* CONFIG_ATE */

	RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, NULL, BSS0, 0);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! LINK DOWN !!!\n");
	/* reset to not doing improved scan */
	ScanInfo->bImprovedScan = FALSE;
#ifdef RT_CFG80211_SUPPORT

	if (CFG80211DRV_OpsScanRunning(pAd))
		CFG80211DRV_OpsScanInLinkDownAction(pAd);

#endif /* RT_CFG80211_SUPPORT */

#ifdef MTFWD
#ifdef MAC_REPEATER_SUPPORT
	if (wdev->wdev_type != WDEV_TYPE_REPEATER)
#endif
		RTMP_OS_NETDEV_CARRIER_OFF(wdev->if_dev);
#endif

#ifdef APCLI_CFG80211_SUPPORT
	RT_CFG80211_LOST_AP_INFORM(pAd, wdev);
#endif

#ifdef CONFIG_STA_ADHOC_SUPPORT
	if (ADHOC_ON(pAd))
		LinkDown_Adhoc(pAd, wdev);
	else
#endif /* CONFIG_STA_ADHOC_SUPPORT */
		LinkDown_Infra(pAd, wdev, req_from_ap, Elem);



	/*
	 *	if link down come from AP, we need to remove all WPA keys on WPA mode.
	 *	otherwise will cause 4-way handshaking failed, since the WPA key not empty.
	 */
	if ((req_from_ap) && (IS_AKM_WPA_CAPABILITY_Entry(wdev))) {
		/* Remove all WPA keys */
		RTMPWPARemoveAllKeys(pAd, wdev);
	}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	/* Shihwei @20160509 */
	/* Check whether disable/enable BF souding timer for STA mode only */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s():Send HW_STA_BF_SOUNDING_ADJUST Cmd\n", __func__);
		HW_STA_BF_SOUNDING_ADJUST(pAd, STATE_DISCONNECT, wdev);
	}
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

#ifdef WH_EVENT_NOTIFIER
	{
	EventHdlr pEventHdlrHook = NULL;

	pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
	if (pEventHdlrHook && pEntry->wdev)
	    pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_DISCONNECT);
    }
#endif /* WH_EVENT_NOTIFIER */

	TRTableResetEntry(pAd, wdev->bss_info_argument.bmc_wlan_idx);

	/*for repeater, it will delete mac entry when it delete repeater entry*/
	if (wdev->wdev_type != WDEV_TYPE_REPEATER)
		MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

	/*delete ppe entry when apcli disconnect, avoid 4-way handshaking failure*/
#ifdef WHNAT_SUPPORT
	if (pAd->CommonCfg.whnat_en) {
#ifdef CONFIG_FAST_NAT_SUPPORT
		if (ppe_del_entry_by_mac != NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				"WHNAT Enable, Need Delete HNAT Entry When Disconnect!\n");
			ppe_del_entry_by_mac(wdev->if_addr);
		}
#endif
	}
#endif

#ifdef MT_MAC

	if (ADHOC_ON(pAd)) {
		/* TODO: Need to add. */
	} else {
		/* only non-repeater should do wdev_linkdown */
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			if ((wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) || (wdev_do_linkdown(wdev) != TRUE))
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " link down fail!!\n");
                }
	}

#endif /* MT_MAC */
	HW_SET_SLOTTIME(pAd, TRUE, wdev->channel, wdev);	/*FALSE); */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE

	if ((!RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) && (!RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)))
#else
#ifdef P2P_SUPPORT
	if ((!P2P_GO_ON(pAd)) && (!P2P_CLI_ON(pAd)))
#endif /* P2P_SUPPORT */
#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN, HcGetBandByWdev(wdev));

	pAd->LedCntl.LedIndicatorStrength = 0xF0;
	RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it. */
#endif /* LED_CONTROL_SUPPORT */
#ifdef P2P_SUPPORT
		if (!P2P_GO_ON(pAd))
#endif /* P2P_SUPPORT */
			AsicDisableSync(pAd, HW_BSSID_0);

	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->BcnCheckInfo[DBDC_BAND0].BcnInitedRnd = pAd->Mlme.PeriodicRound;
	pAd->BcnCheckInfo[DBDC_BAND1].BcnInitedRnd = pAd->Mlme.PeriodicRound;

	/* 802.1x port control */
#ifdef WPA_SUPPLICANT_SUPPORT

	/* Prevent clear PortSecured here with static WEP */
	/* NetworkManger set security policy first then set SSID to connect AP. */
	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP &&
		(IS_CIPHER_WEP_Entry(wdev)) &&
		(wdev->SecConfig.IEEE8021X == FALSE))
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
	else
#endif /* WPA_SUPPLICANT_SUPPORT */
	{
		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
	/* Pat: we cannot zero MacEntry, because we just enqueue a STAREC update job to execute later and it will refer to pEntry */
	/* It is not a good way, need to refine the STARec update flow */
	/* Rmove this. TODO: refine flow. NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY)); */
	tr_ctl->tr_entry[pEntry->wcid].PortSecured = wdev->PortSecured;
	NdisReleaseSpinLock(&pAd->MacTabLock);

	/* Restore MlmeRate */
	pAd->CommonCfg.MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	pAd->CommonCfg.RtsRate = pAd->CommonCfg.BasicMlmeRate;

	BandIdx  = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	AsicSetTxStream(pAd, TxPath, OPMODE_STA, FALSE, BandIdx);
	/* After Link down, reset piggy-back setting in ASIC. Disable RDG. */
	AsicSetPiggyBack(pAd, FALSE);
#ifdef DOT11_N_SUPPORT
	pAd->CommonCfg.BACapability.word = pAd->CommonCfg.REGBACapability.word;
#endif /* DOT11_N_SUPPORT */
	/* Restore all settings in the following. */
	wdev->protection = 0; /* link down set to default */
	HW_SET_PROTECT(pAd, wdev, PROT_PROTOCOL, 0, 0);
#ifdef DOT11_N_SUPPORT
	/* AsicSetRDG(pAd, pEntry->wcid, BandIdx, 0, 0); */
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SCAN_2040);
	pAd->CommonCfg.BSSCoexist2040.word = 0;
	TriEventInit(pAd);

	for (i = 0; i < (pChCtrl->ChListNum - 1); i++)
		pChCtrl->ChList[i].bEffectedChannel = FALSE;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP) {
		/*send disassociate event to wpa_supplicant */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
								RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}

#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL,
							NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
	RT_CFG80211_LOST_AP_INFORM(pAd, wdev);
#endif
#endif /* RT_CFG80211_SUPPORT */

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		pStaCfg->wdev.bLinkUpDone = FALSE;
#ifdef DOT11_N_SUPPORT
		NdisZeroMemory(&pStaCfg->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
		NdisZeroMemory(&pStaCfg->MlmeAux.AddHtInfo, sizeof(ADD_HT_INFO_IE));
		CLR_HT_CAPS_EXIST(pStaCfg->MlmeAux.ie_exists);
		pStaCfg->MlmeAux.NewExtChannelOffset = 0xff;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LinkDownCleanMlmeAux.ExtCapInfo!\n");
		NdisZeroMemory((PUCHAR) (&pStaCfg->MlmeAux.ExtCapInfo),
					   sizeof(EXT_CAP_INFO_ELEMENT));
#endif /* DOT11_N_SUPPORT */

	/* Reset WPA-PSK state. Only reset when supplicant enabled */
		if (pStaCfg->WpaState != SS_NOTUSE) {
			pStaCfg->WpaState = SS_START;
			/* Clear Replay counter */
			NdisZeroMemory(pStaCfg->ReplayCounter, 8);
		}

		pStaCfg->MicErrCnt = 0;
		RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
		/* Update extra information to link is up */
		pAd->ExtraInfo = GENERAL_LINK_DOWN;
		pStaCfg->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
		/* Clean association information */
		NdisZeroMemory(&pStaCfg->AssocInfo,
					   sizeof(NDIS_802_11_ASSOCIATION_INFORMATION));
		pStaCfg->AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		pStaCfg->ReqVarIELen = 0;
		pStaCfg->ResVarIELen = 0;
		/* Reset RSSI value after link down */
		NdisZeroMemory((PUCHAR) (&pStaCfg->RssiSample),
					   sizeof(pStaCfg->RssiSample));

#ifdef WIDI_SUPPORT
		/*
		 *	Send L2SDTA Disassociation status here
		 */
		if (pStaCfg->WscControl.bWscTrigger)
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPS is in Prgress; Not sending this LINK DOWN\n");
		else {
			/* LastBssid is zero; Not sending this LINK DOWN */
			if (!MAC_ADDR_EQUAL(pAd->CommonCfg.LastBssid, ZERO_MAC_ADDR)) {
				WidiUpdateStateToDaemon(pAd, MIN_NET_DEVICE_FOR_MBSSID, WIDI_MSG_TYPE_ASSOC_STATUS,
										pAd->CommonCfg.LastBssid, NULL, 0, WIDI_DISASSOCIATED);
			} else
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN, "LastBssid is zero; Not sending this LINK DOWN\n");
		}
#endif /* WIDI_SUPPORT */

		if (pStaCfg->BssType != BSS_ADHOC)
			ScanInfo->bNotFirstScan = FALSE;
#ifdef CONFIG_STA_ADHOC_SUPPORT
		else {
			struct adhoc_info *adhocInfo = NULL;

			adhocInfo = &pStaCfg->adhocInfo;
			adhocInfo->bAdhocCreator = FALSE;
		}
#endif /* CONFIG_STA_ADHOC_SUPPORT */

		/*After change from one ap to another , we need to re-init rssi for AdjustTxPower  */
		pStaCfg->RssiSample.AvgRssi[0] = -127;
		pStaCfg->RssiSample.AvgRssi[1] = -127;
		pStaCfg->RssiSample.AvgRssi[2] = -127;
		NdisZeroMemory(pStaCfg->ConnectinfoSsid, MAX_LEN_OF_SSID);
		NdisZeroMemory(pStaCfg->ConnectinfoBssid, MAC_ADDR_LEN);
		pStaCfg->ConnectinfoSsidLen  = 0;
		pStaCfg->ConnectinfoBssType  = 1;
		pStaCfg->ConnectinfoChannel = 0;
#ifdef DOT11_VHT_AC
		pStaCfg->MlmeAux.force_op_mode = 0;
		NdisZeroMemory(&pStaCfg->MlmeAux.op_mode, sizeof(OPERATING_MODE));
#endif
#ifdef APCLI_CFG80211_SUPPORT
		/* Reset Pairwise cipher for next configuration */
		CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
#endif
		/* inform main thread linkdown is complete */
		sta_link_down_complete(pStaCfg);

	}

#ifdef APCLI_CFG80211_SUPPORT
	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
#endif
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID IterateOnBssTab(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_STA_ADHOC_SUPPORT
	MLME_START_REQ_STRUCT StartReq;
#endif /* CONFIG_STA_ADHOC_SUPPORT */
	MLME_JOIN_REQ_STRUCT JoinReq;
	ULONG BssIdx;
	BSS_ENTRY *pInBss = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	/* Change the wepstatus to original wepstatus */
	pStaCfg->PairwiseCipher = wdev->SecConfig.PairwiseCipher;
#ifdef WPA_SUPPLICANT_SUPPORT

	if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
		pStaCfg->GroupCipher = wdev->SecConfig.GroupCipher;

	BssIdx = pStaCfg->MlmeAux.BssIdx;
#ifdef CONFIG_STA_ADHOC_SUPPORT
	if (pStaCfg->BssType == BSS_ADHOC) {
		if (BssIdx < pStaCfg->MlmeAux.SsidBssTab.BssNr) {
			pInBss = &pStaCfg->MlmeAux.SsidBssTab.BssEntry[BssIdx];
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "CNTL - iterate BSS %ld of %d\n", BssIdx,
					  pStaCfg->MlmeAux.SsidBssTab.BssNr);
			JoinParmFill(pAd, &JoinReq, wdev, BssIdx);
			MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_JOIN_REQ,
								sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
#ifdef WSC_STA_SUPPORT

			if (wdev->WscControl.WscState >= WSC_STATE_START) {
				NdisMoveMemory(wdev->WscControl.WscPeerMAC, pInBss->MacAddr,
							   MAC_ADDR_LEN);
				NdisMoveMemory(wdev->WscControl.EntryAddr,
							   pInBss->MacAddr, MAC_ADDR_LEN);
			}

#endif /* WSC_STA_SUPPORT */
			return;
		}

#ifdef IWSC_SUPPORT

		if (wdev->WscControl.bWscTrigger == TRUE) {
			cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
			MlmeEnqueueWithWdev(pAd, IWSC_STATE_MACHINE, IWSC_MT2_MLME_RECONNECT, 0, NULL, 0, wdev);
			RTMP_MLME_HANDLER(pAd);
		} else
#endif /* IWSC_SUPPORT */
		{
			MLME_START_REQ_STRUCT StartReq;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CNTL - All BSS fail; start a new ADHOC (Ssid=%s)...\n", pStaCfg->MlmeAux.Ssid);
			StartParmFill(pAd, &StartReq, (PCHAR) pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
			MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_ADHOC_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq, 0, wdev);
			cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
		}
	} else
#endif /* CONFIG_STA_ADHOC_SUPPORT */
			if ((BssIdx < pStaCfg->MlmeAux.SsidBssTab.BssNr) &&
			   (pStaCfg->BssType == BSS_INFRA)) {
		pInBss = &pStaCfg->MlmeAux.SsidBssTab.BssEntry[BssIdx];
		/*Check AuthMode of AP, may have more AuthMode Supported,eg WPA/WPA2*/
		/*Set the AKMMap according to sta/AP capablities */
		pStaCfg->AKMMap = wdev->SecConfig.AKMMap &  pInBss->AKMMap;

		if (IS_AKM_WPA1(pStaCfg->AKMMap) && IS_AKM_WPA2(pStaCfg->AKMMap)) {
			CLEAR_SEC_AKM(pStaCfg->AKMMap);
			SET_AKM_WPA2(pStaCfg->AKMMap);
		} else if (IS_AKM_WPA1PSK(pStaCfg->AKMMap) && IS_AKM_WPA2PSK(pStaCfg->AKMMap)) {
			CLEAR_SEC_AKM(pStaCfg->AKMMap);
			SET_AKM_WPA2PSK(pStaCfg->AKMMap);
		} else if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) && IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)) {
			CLEAR_SEC_AKM(pStaCfg->AKMMap);
			SET_AKM_WPA3PSK(pStaCfg->AKMMap);
		} else if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			CLEAR_SEC_AKM(pStaCfg->AKMMap);
			SET_AKM_OWE(pStaCfg->AKMMap);
		}

		/* Check cipher suite, AP must have more secured cipher than station setting */
		/* Set the Pairwise and Group cipher to match the intended AP setting */
		/* We can only connect to AP with less secured cipher setting */
		if (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
			|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)) {
			pStaCfg->GroupCipher = pInBss->GroupCipher;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);

			if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher) && IS_CIPHER_CCMP128(pInBss->PairwiseCipher))
				SET_CIPHER_CCMP128(pStaCfg->PairwiseCipher);
			else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
				SET_CIPHER_TKIP(pStaCfg->PairwiseCipher);
		}

		if (IS_AKM_WPA2(wdev->SecConfig.AKMMap)
			|| IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)
			|| IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap)
			|| IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			pStaCfg->GroupCipher = pInBss->GroupCipher;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);

			if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)
				&& IS_CIPHER_CCMP128(pInBss->PairwiseCipher))
				SET_CIPHER_CCMP128(pStaCfg->PairwiseCipher);
			else if (IS_CIPHER_CCMP256(wdev->SecConfig.PairwiseCipher)
					 && IS_CIPHER_CCMP256(pInBss->PairwiseCipher))
				SET_CIPHER_CCMP256(pStaCfg->PairwiseCipher);
			else if (IS_CIPHER_GCMP128(wdev->SecConfig.PairwiseCipher)
					 && IS_CIPHER_GCMP128(pInBss->PairwiseCipher))
				SET_CIPHER_GCMP128(pStaCfg->PairwiseCipher);
			else if (IS_CIPHER_GCMP256(wdev->SecConfig.PairwiseCipher)
					 && IS_CIPHER_GCMP256(pInBss->PairwiseCipher))
				SET_CIPHER_GCMP256(pStaCfg->PairwiseCipher);
			else if (!IS_AKM_WPA3PSK(wdev->SecConfig.AKMMap))
				/* There is no PairCipher Aux, downgrade our capability to TKIP */
				SET_CIPHER_TKIP(pStaCfg->PairwiseCipher);

			/* RSN capability */
			pStaCfg->RsnCapability = pInBss->RsnCapability;
		} else if (IS_AKM_WPA3_192BIT(wdev->SecConfig.AKMMap)) {
			pStaCfg->GroupCipher = pInBss->GroupCipher;
			CLEAR_CIPHER(pStaCfg->PairwiseCipher);

			if (IS_CIPHER_GCMP256(wdev->SecConfig.PairwiseCipher)
				&& IS_CIPHER_GCMP256(pInBss->PairwiseCipher))
				SET_CIPHER_GCMP256(pStaCfg->PairwiseCipher);
			else if (IS_CIPHER_CCMP256(wdev->SecConfig.PairwiseCipher)
				&& IS_CIPHER_CCMP256(pInBss->PairwiseCipher))
				SET_CIPHER_CCMP256(pStaCfg->PairwiseCipher);

			/* RSN capability */
			pStaCfg->RsnCapability = pInBss->RsnCapability;
		}

		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - iterate BSS %ld of %d\n", BssIdx,
				  pStaCfg->MlmeAux.SsidBssTab.BssNr);
		JoinParmFill(pAd, &JoinReq, wdev, BssIdx);
		MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_JOIN_REQ,
							sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_SYNC, __func__);
	} else {
		/* no more BSS */
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - All roaming failed, restore to channel %d, Total BSS[%02d]\n",
				  wdev->channel, ScanTab->BssNr);
#ifdef P2P_SUPPORT

		if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
			; /* Do NOT delete this entry here because ra interface cannot do all channel scan in this case. */
		else
#endif /* P2P_SUPPORT */
#ifdef APCLI_CFG80211_SUPPORT
		if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
			pStaCfg->ApcliInfStat.Enable = FALSE;
		else
#endif
		{
			pStaCfg->MlmeAux.SsidBssTab.BssNr = 0;
			BssTableDeleteEntry(ScanTab,
								pStaCfg->MlmeAux.SsidBssTab.BssEntry[0].Bssid,
								pStaCfg->MlmeAux.SsidBssTab.BssEntry[0].Channel);
		}

		cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
		/* LED indication. */
		LEDConnectionCompletion(pAd, FALSE);
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */
	}
}


/* for re-association only */
/* IRQL = DISPATCH_LEVEL */
VOID IterateOnBssTab2(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MLME_REASSOC_REQ_STRUCT ReassocReq;
	ULONG BssIdx;
	BSS_ENTRY *pBss;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef APCLI_CFG80211_SUPPORT
		if (pStaCfg->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_ENABLE)
			return;
#endif

	BssIdx = pStaCfg->MlmeAux.RoamIdx;
	pBss = &pStaCfg->MlmeAux.RoamTab.BssEntry[BssIdx];

	if (BssIdx < pStaCfg->MlmeAux.RoamTab.BssNr) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CNTL - iterate BSS %ld of %d\n", BssIdx,
				  pStaCfg->MlmeAux.RoamTab.BssNr);
		wdev->channel = pBss->Channel;
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		/* reassociate message has the same structure as associate message */
		AssocParmFill(pAd, &ReassocReq, pBss->Bssid,
					  pBss->CapabilityInfo, ASSOC_TIMEOUT,
					  pStaCfg->DefaultListenCount);
		MlmeEnqueueWithWdev(pAd, ASSOC_FSM, ASSOC_FSM_MLME_REASSOC_REQ,
							sizeof(MLME_REASSOC_REQ_STRUCT), &ReassocReq, 0, wdev);
		cntl_fsm_state_transition(wdev, CNTL_WAIT_ASSOC, __func__);
	} else {
		/* no more BSS */
		struct freq_cfg freq;

		os_zero_mem(&freq, sizeof(freq));
		freq.prim_ch = wdev->channel;
		freq.ext_cha = EXTCHA_NONE;
		freq.ht_bw = HT_BW_20;
		wlan_operate_set_phy(wdev, &freq);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%s():CNTL - All roaming failed, restore to Channel(Ctrl=%d, Central = %d)\n",
				  __func__, wdev->channel,
				  wlan_operate_get_cen_ch_1(wdev));
	}

	cntl_fsm_state_transition(wdev, CNTL_IDLE, __func__);
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
static VOID JoinParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_JOIN_REQ_STRUCT *JoinReq,
	IN struct wifi_dev *wdev,
	IN ULONG BssIdx)
{
	JoinReq->BssIdx = BssIdx;

#ifdef CONFIG_6G_SUPPORT
	if (BssIdx == BSS_NOT_FOUND && WMODE_CAP_6G(wdev->PhyMode)) {
		PSTA_ADMIN_CONFIG pApCliEntry = GetStaCfgByWdev(pAd, wdev);
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, &pApCliEntry->wdev);
		ULONG bss_idx = BSS_NOT_FOUND;

		bss_idx = BssSsidTableSearchBySSID(ScanTab, (PCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
		if (bss_idx != BSS_NOT_FOUND) {
			if (IS_AKM_OWE(ScanTab->BssEntry[bss_idx].AKMMap) ||
				(IS_AKM_WPA3PSK(ScanTab->BssEntry[bss_idx].AKMMap) &&
				!IS_AKM_WPA2PSK(ScanTab->BssEntry[bss_idx].AKMMap))) {
				/*
				 * For 6G Apcli, BSSID shall be filled in probe request and the
				 * target BSS shall support WPA3PSK.
				 * */
				COPY_MAC_ADDR(JoinReq->Bssid, ScanTab->BssEntry[bss_idx].Bssid);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Target Ssid does not support WPA3PSK! Ssid=%s, AKMMap=%s\n",
					JoinReq->Ssid, GetAuthModeStr(ScanTab->BssEntry[bss_idx].AKMMap));
			}
		}
	}
#endif
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
static VOID StartParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_START_REQ_STRUCT *StartReq,
	IN CHAR Ssid[],
	IN UCHAR SsidLen)
{
	ASSERT(SsidLen <= MAX_LEN_OF_SSID);

	if (SsidLen > MAX_LEN_OF_SSID)
		SsidLen = MAX_LEN_OF_SSID;

	NdisMoveMemory(StartReq->Ssid, Ssid, SsidLen);
	StartReq->SsidLen = SsidLen;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
static VOID AuthParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_AUTH_REQ_STRUCT *AuthReq,
	IN PUCHAR pAddr,
	IN USHORT Alg)
{
	COPY_MAC_ADDR(AuthReq->Addr, pAddr);
	AuthReq->Alg = Alg;
	AuthReq->Timeout = AUTH_TIMEOUT;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */

VOID InitChannelRelatedValue(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 ht_bw, ext_ch;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	UCHAR cen_ch = pStaCfg->MlmeAux.CentralChannel;

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

#ifdef RTMP_MAC_PCI
	/* In power save , We will force use 1R. */
	/* So after link up, check Rx antenna # again. */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);
#endif /* RTMP_MAC_PCI */
	wdev->channel = pStaCfg->MlmeAux.Channel;
#ifdef DOT11_N_SUPPORT

	/* Change to AP channel */
	if ((cen_ch > wdev->channel)
		&& (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)) {
		ht_bw = BW_40;
		ext_ch = EXTCHA_ABOVE;
	} else if ((cen_ch < wdev->channel)
			   && (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)) {
		ht_bw = BW_40;
		ext_ch = EXTCHA_BELOW;
	} else
#endif /* DOT11_N_SUPPORT */
	{
		ht_bw = BW_20;
		ext_ch = EXTCHA_NONE;
	}

	{
		struct freq_cfg freq;

		os_zero_mem(&freq, sizeof(freq));
		freq.prim_ch = wdev->channel;
		freq.ht_bw = ht_bw;
		freq.ext_cha = ext_ch;
		wlan_operate_set_phy(wdev, &freq);
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s():BW_%s, CtrlChannel=%d, CentralChannel=%d\n",
			  __func__, (ht_bw == HT_BW_40 ? "40" : "20"),
			  wdev->channel,
			  cen_ch);
	/* Save BBP_R66 value, it will be used in RTUSBResumeMsduTransmission */
	bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);
}


VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN ht_bw,
	IN UCHAR PriCh,
	IN UCHAR cen_ch_1,
	IN struct wifi_dev *wdev)
{
	struct freq_cfg cfg;
	UCHAR ext_cha;
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = VHT_BW_2040;
#endif /* DOT11_VHT_AC */
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return;

	*pBwFallBack = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return;

	wdev->channel = PriCh;
#ifdef DOT11_N_SUPPORT

	/* Change to AP channel */
	if ((cen_ch_1 > wdev->channel) && (ht_bw == HT_BW_40))
		ext_cha = EXTCHA_ABOVE;
	else if ((cen_ch_1 < wdev->channel) && (ht_bw == HT_BW_40))
		ext_cha = EXTCHA_BELOW;
	else
#endif /* DOT11_N_SUPPORT */
	{
		ht_bw = HT_BW_20;
		ext_cha = EXTCHA_NONE;
	}

#ifdef DOT11_VHT_AC

	if (ht_bw == HT_BW_40 &&
		pStaCfg->StaActive.SupportedPhyInfo.bVhtEnable == TRUE &&
		pStaCfg->StaActive.SupportedPhyInfo.vht_bw >= VHT_BW_80)
		vht_bw = pStaCfg->StaActive.SupportedPhyInfo.vht_bw;

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_6G(wdev->PhyMode)) {
		ext_cha = EXTCHA_ABOVE;
		ht_bw = wlan_operate_get_ht_bw(wdev);
		if (pStaCfg->StaActive.SupportedPhyInfo.vht_bw >= VHT_BW_80)
			vht_bw = pStaCfg->StaActive.SupportedPhyInfo.vht_bw;
	}
#endif /*DOT11_HE_AX*/

	/*for STA connection usage*/
	os_zero_mem(&cfg, sizeof(cfg));
	cfg.prim_ch = wdev->channel;
	cfg.ht_bw = ht_bw;
	cfg.ext_cha = ext_cha;
	cfg.vht_bw = vht_bw;
	cfg.cen_ch_2 = wlan_operate_get_cen_ch_2(wdev);
	cfg.ch_band = wlan_operate_get_ch_band(wdev);
	wlan_operate_set_phy(wdev, &cfg);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 " RF-Ch=%d, CtrlCh=%d, CentralCh=%d, CentralCh2=%d, BW=%d\n",
			  pAd->LatchRfRegs.Channel,
			  wdev->channel,
			  wlan_operate_get_cen_ch_1(wdev),
			  wlan_operate_get_cen_ch_2(wdev),
			  wlan_operate_get_bw(wdev));
}

VOID sta_cntl_init(
	struct wifi_dev *wdev)
{
	sta_cntl_api_ops.cntl_connect_proc = sta_cntl_connect_proc;
	sta_cntl_api_ops.cntl_disconnect_proc = sta_cntl_disconnect_proc;
	sta_cntl_api_ops.cntl_join_conf = sta_cntl_join_conf;
	sta_cntl_api_ops.cntl_auth_conf = sta_cntl_auth_conf;
	sta_cntl_api_ops.cntl_auth2_conf = sta_cntl_auth2_conf;
	sta_cntl_api_ops.cntl_deauth_conf = sta_cntl_deauth_conf;
	sta_cntl_api_ops.cntl_assoc_conf = sta_cntl_assoc_conf;
	sta_cntl_api_ops.cntl_reassoc_conf = sta_cntl_reassoc_conf;
	sta_cntl_api_ops.cntl_disassoc_conf = sta_cntl_disassoc_conf;
	sta_cntl_api_ops.cntl_scan_proc = sta_cntl_scan;
	sta_cntl_api_ops.cntl_error_handle = sta_cntl_error_handle;
	sta_cntl_api_ops.cntl_reset_all_fsm_proc = sta_cntl_reset_all_fsm;
	wdev->cntl_api = &sta_cntl_api_ops;
}



void rept_muar_read(PRTMP_ADAPTER pAd, UINT id)
{
	/* UINT32	mar_val; */
	RMAC_MAR0_STRUC mar0_val;
	RMAC_MAR1_STRUC mar1_val;

	memset(&mar0_val, 0x0, sizeof(mar0_val));
	memset(&mar1_val, 0x0, sizeof(mar1_val));
	mar1_val.field.access_start = 1;
	mar1_val.field.multicast_addr_index = id * 2;
	/* Issue a read command */
	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

	/* wait acess complete*/
	do {
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
		/* delay */
	} while (mar1_val.field.access_start == 1);

	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
	printk("%02x:%02x:%02x:%02x:%02x:%02x  ",
		   (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
		   (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
		   (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
		   (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
		   (UINT8)mar1_val.field.addr_39_32,
		   (UINT8)mar1_val.field.addr_47_40
		  );
	memset(&mar0_val, 0x0, sizeof(mar0_val));
	memset(&mar1_val, 0x0, sizeof(mar1_val));
	mar1_val.field.access_start = 1;
	mar1_val.field.multicast_addr_index = id * 2 + 1;
	/* Issue a read command */
	HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32)mar1_val.word);

	/* wait acess complete*/
	do {
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR1, (UINT32 *)&mar1_val);
		/* delay */
	} while (mar1_val.field.access_start == 1);

	HW_IO_READ32(pAd->hdev_ctrl, RMAC_MAR0, (UINT32 *)&mar0_val);
	printk("%02x:%02x:%02x:%02x:%02x:%02x\n",
		   (UINT8)(mar0_val.addr_31_0 & 0x000000ff),
		   (UINT8)((mar0_val.addr_31_0 & 0x0000ff00) >> 8),
		   (UINT8)((mar0_val.addr_31_0 & 0x00ff0000) >> 16),
		   (UINT8)((mar0_val.addr_31_0 & 0xff000000) >> 24),
		   (UINT8)mar1_val.field.addr_39_32,
		   (UINT8)mar1_val.field.addr_47_40
		  );
}

