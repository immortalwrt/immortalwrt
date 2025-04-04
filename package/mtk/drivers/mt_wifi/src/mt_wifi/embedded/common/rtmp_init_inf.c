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
	rtmp_init_inf.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif /* DOT11R_FT_SUPPORT */

#define PROBE2LOAD_L1PROFILE /* Capable to be turned off if not required */

#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
NDIS_STATUS WriteDatThread(RTMP_ADAPTER *pAd);
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef MULTI_PROFILE
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad);
#endif /*MULTI_PROFILE*/

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT
/* Utilities provided from NET module */
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
RTMP_PCI_CONFIG RtmpPciConfig, *pRtmpPciConfig = &RtmpPciConfig;
RTMP_USB_CONFIG RtmpUsbConfig, *pRtmpUsbConfig = &RtmpUsbConfig;

VOID RtmpDrvOpsInit(
	OUT VOID *pDrvOpsOrg,
	INOUT VOID *pDrvNetOpsOrg,
	IN RTMP_PCI_CONFIG *pPciConfig,
	IN RTMP_USB_CONFIG *pUsbConfig)
{
	RTMP_DRV_ABL_OPS *pDrvOps = (RTMP_DRV_ABL_OPS *)pDrvOpsOrg;

	/* init PCI/USB configuration in different OS */
	if (pPciConfig != NULL)
		RtmpPciConfig = *pPciConfig;

	if (pUsbConfig != NULL)
		RtmpUsbConfig = *pUsbConfig;

	/* init operators provided from us (DRIVER module) */
	pDrvOps->RTMPAllocAdapterBlock = RTMPAllocAdapterBlock;
	pDrvOps->RTMPFreeAdapter = RTMPFreeAdapter;
	pDrvOps->RtmpRaDevCtrlExit = RtmpRaDevCtrlExit;
	pDrvOps->RtmpRaDevCtrlInit = RtmpRaDevCtrlInit;
#ifdef RTMP_MAC_PCI
	pDrvOps->RTMPHandleInterrupt = mtd_isr;
#endif /* RTMP_MAC_PCI */
	pDrvOps->RTMPSendPackets = RTMPSendPackets;
#ifdef P2P_SUPPORT
	pDrvOps->P2P_PacketSend = P2P_PacketSend;
#endif /* P2P_SUPPORT */
	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pDrvOps->RTMP_STA_IoctlHandle = RTMP_STA_IoctlHandle;
#endif /* CONFIG_STA_SUPPORT */
	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->mt_wifi_init = mt_wifi_init;
	/* init operators provided from us and netif module */
	*pDrvNetOps = *pRtmpDrvNetOps;
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


INT rtmp_cfg_exit(RTMP_ADAPTER *pAd)
{
	UserCfgExit(pAd);
	return TRUE;
}


INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;
	UINT_16 aid_order_reserved = 0;

/* For Vxworks we have already configured through IOCTL instead of profile, no need to init user cfg again */
	UserCfgInit(pAd);

#ifdef WTBL_TDD_SUPPORT
	WtblTdd_Init(pAd);
#endif /* WTBL_TDD_SUPPORT */

#ifdef MBO_SUPPORT
	MboInit(pAd);
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
	OceInit(pAd);
#endif /* OCE_SUPPORT */
	CfgInitHook(pAd);
#ifdef DPP_SUPPORT
	pAd->dpp_rx_frm_counter = 0;
#endif /* DPP_SUPPORT */

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	if (pAd->RfIcType == 0) {
		/* RfIcType not assigned, should not happened! */
		pAd->RfIcType = RFIC_UNKNOWN;
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Invalid RfIcType, reset it first\n");
	}

	status = RTMPReadParametersHook(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPReadParametersHook failed, Status[=0x%08x]\n", status);
		return FALSE;
	}
#ifdef OCE_SUPPORT
	OceTimerInit(pAd);
#endif /* OCE_SUPPORT */

#ifdef PKT_BUDGET_CTRL_SUPPORT
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		pAd->pbc_bound[DBDC_BAND0][PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE_BAND0;
		pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK_BAND0;
		pAd->pbc_bound[DBDC_BAND0][PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO_BAND0;
		pAd->pbc_bound[DBDC_BAND0][PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI_BAND0;
		pAd->pbc_bound[DBDC_BAND0][PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT_BAND0;

		pAd->pbc_bound[DBDC_BAND1][PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
		pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
		pAd->pbc_bound[DBDC_BAND1][PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
		pAd->pbc_bound[DBDC_BAND1][PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
		pAd->pbc_bound[DBDC_BAND1][PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT;
	}
#endif
#endif
	/*aid bitmap needs to consider the amounts of the non-transmitted bss of 11V mbss*/
#ifdef DOT11V_MBSSID_SUPPORT
	aid_order_reserved += bssid_num_to_max_indicator(pAd->ApCfg.BssidNum);
#endif
	entrytb_aid_bitmap_reserve(&pAd->MacTab.aid_info, aid_order_reserved);

	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	/*After RTMPReadParameterHook to get MBSSNum & MSTANum*/
	HcSetMaxStaNum(pAd);
	return TRUE;
}


INT rtmp_mgmt_init(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


static INT rtmp_sys_exit(RTMP_ADAPTER *pAd)
{
#ifdef SMART_ANTENNA
	RtmpSAExit(pAd);
#endif /* SMART_ANTENNA */
	MeasureReqTabExit(pAd);
#ifdef TPC_SUPPORT
	TpcReqTabExit(pAd);
#endif
	rtmp_cfg_exit(pAd);
	HwCtrlExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
		NdisFreeSpinLock(&pAd->TimerQLock);
#endif
	return TRUE;
}


static INT rtmp_sys_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;

	wifi_sys_reset(&pAd->WifiSysInfo);
#ifdef DBG_STARVATION
	starv_log_init(&pAd->starv_log_ctrl);
#endif /*DBG_STARVATION*/

	status = RtmpMgmtTaskInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err0;

	status = HwCtrlInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err1;

	/* Initialize pAd->StaCfg[], pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	if (rtmp_cfg_init(pAd, pHostName) != TRUE)
		goto err2;

	status = MeasureReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MeasureReqTabInit failed, Status[=0x%08x]\n", status);
		goto err2;
	}
#ifdef TPC_SUPPORT
	status = TpcReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "TpcReqTabInit failed, Status[=0x%08x]\n", status);
		goto err2;
	}
#endif
#ifdef SMART_ANTENNA
	RtmpSAInit(pAd);
#endif /* SMART_ANTENNA */
#ifdef MT_MAC
	/* TxS Setting */
	InitTxSTypeTable(pAd);

	if (IS_HIF_TYPE(pAd, HIF_MT))
		InitTxSCommonCallBack(pAd);

#endif

	/* QM init */
	status = qm_init(pAd);

	if (status)
		goto err2;

	/* TM init */
	status = tm_init(pAd);

	if (status)
		goto err2;

#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_NEED_ON)
		pAd->fq_ctrl.enable = FQ_ARRAY_SCH|FQ_NO_PKT_STA_KEEP_IN_LIST|FQ_EN;
#endif


	return TRUE;
err2:
	rtmp_cfg_exit(pAd);
err1:
	HwCtrlExit(pAd);
err0:
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
	return FALSE;
}

/*
*
*/
static void mt_sys_ready(struct _RTMP_ADAPTER *ad)
{
	/* Now Enable RxTx*/
	chip_interrupt_enable(ad);
	RTMPEnableRxTx(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_START_UP);
}

#ifdef CONFIG_WLAN_SERVICE
/* Allocate memory for wlan service */
int mt_service_open(struct _RTMP_ADAPTER *ad)
{
	struct service *serv = &ad->serv;
	struct service_test *serv_test;
	struct test_wlan_info *test_winfo;
	struct test_operation *test_op;

	/* Allocate service struct memory */
	os_alloc_mem(ad, (UCHAR **)&serv_test, sizeof(struct service_test));
	os_alloc_mem(ad, (UCHAR **)&test_winfo, sizeof(struct test_wlan_info));
	os_alloc_mem(ad, (UCHAR **)&test_op, sizeof(struct test_operation));

	/* Init service struct memory */
	os_zero_mem(serv_test, sizeof(struct service_test));
	os_zero_mem(test_winfo, sizeof(struct test_wlan_info));
	os_zero_mem(test_op, sizeof(struct test_operation));

	serv_test->test_winfo = test_winfo;
	serv_test->test_op = test_op;
	serv_test->engine_offload = FALSE;

	/* Assign service type for service.git */
	serv->serv_id = SERV_HANDLE_TEST;
	serv->serv_handle = (VOID *)serv_test;

	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" wlan service opens successfully!\n");

	return NDIS_STATUS_SUCCESS;
}

/* Init value for wlan service */
int mt_service_init(struct _RTMP_ADAPTER *ad)
{
	INT32 ret;
	struct service *serv = &ad->serv;
	struct service_test *serv_test;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct serv_chip_cap *chip_cap = NULL;
	PKT_TOKEN_CB *pkt_token_cb = hc_get_ct_cb(ad->hdev_ctrl);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(pkt_token_cb, 0);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(ad->hdev_ctrl);

	serv_test = (struct service_test *)serv->serv_handle;
	chip_cap = &serv_test->test_winfo->chip_cap;

	/* Fill service_wlan_info for service internal usage */
	serv_test->test_winfo->net_dev = ad->net_dev;
	serv_test->test_winfo->chip_id = ad->ChipID;
	serv_test->test_winfo->hdev_ctrl = ad->hdev_ctrl;
	serv_test->test_winfo->dbdc_mode = ad->CommonCfg.dbdc_mode;
	serv_test->test_winfo->pkt_tx_tkid_max = que->pkt_tkid_cnt;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	serv_test->test_winfo->chip_cap.ra_offload = cap->fgRateAdaptFWOffload;
#endif
	chip_cap->swq_per_band = cap->multi_token_ques_per_band;
	chip_cap->tx_wi_size = cap->TXWISize;
	chip_cap->rx_wi_size = cap->RXWISize;
	chip_cap->tx_hw_hdr_len = cap->tx_hw_hdr_len;
	chip_cap->rx_hw_hdr_len = cap->rx_hw_hdr_len;
	chip_cap->num_of_tx_ring = hif->tx_res_num;
	chip_cap->num_of_rx_ring = hif->rx_res_num;
	chip_cap->tx_ring_size = cap->tx_ring_size;
	chip_cap->ht_ampdu_exp = cap->ppdu.ht_max_ampdu_len_exp;
	chip_cap->non_he_tx_ba_wsize = cap->ppdu.non_he_tx_ba_wsize;
#if defined(DOT11_VHT_AC)
	chip_cap->max_mpdu_len = cap->ppdu.max_mpdu_len;
	chip_cap->vht_ampdu_exp = cap->ppdu.vht_max_ampdu_len_exp;
#endif	/* DOT11_VHT_AC */
#if defined(DOT11_HE_AX)
	chip_cap->he_ampdu_exp = cap->ppdu.he_max_ampdu_len_exp;
	chip_cap->he_tx_ba_wsize = cap->ppdu.he_tx_ba_wsize;
#endif	/* DOT11_HE_AX */
	chip_cap->efuse_size = cap->EEPROM_DEFAULT_BIN_SIZE;
	if (cap->phy_caps & fPHY_CAP_6G)
		chip_cap->support_6g = TRUE;
	if (IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY))
		chip_cap->is_dual_phy = TRUE;
	serv_test->test_winfo->use_efuse = ad->bUseEfuse;
	serv_test->test_winfo->e2p_cur_mode = ad->E2pCtrl.e2pCurMode;
	serv_test->test_winfo->e2p_access_mode = ad->E2pAccessMode;
	if (ad->E2pCtrl.e2pCurMode == E2P_FLASH_MODE || ad->E2pCtrl.e2pCurMode == E2P_BIN_MODE)
		chip_cap->efuse_size = get_dev_eeprom_size(ad);

	os_move_mem(&serv_test->test_winfo->chip_cap.spe_map_list,
			&cap->spe_map_list, sizeof(cap->spe_map_list));

	os_move_mem(&serv_test->test_winfo->wm_fw_info,
		&ad->MCUCtrl.fwdl_ctrl.fw_profile[WM_CPU].fw_info,
		sizeof(struct fw_info));
	os_move_mem(&serv_test->test_winfo->chip_cap.mcs_nss,
		&cap->mcs_nss, sizeof(struct serv_mcs_nss_caps));
	if (cap->mcs_nss.max_nss[0] > ad->Antenna.field.TxPath)
		serv_test->test_winfo->chip_cap.mcs_nss.max_nss[0] =
			ad->Antenna.field.TxPath;
	os_move_mem(&serv_test->test_winfo->chip_cap.qos,
		&cap->qos, sizeof(struct qos_caps));

	ret = mt_agent_init_service(serv);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wlan service inits failed!\n");
		mt_agent_exit_service(serv);

		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" wlan service inits successfully!\n");

	return NDIS_STATUS_SUCCESS;
}

/* Free memory for wlan service */
int mt_service_close(struct _RTMP_ADAPTER *ad)
{
	INT32 ret;
	struct service *serv = &ad->serv;
	struct service_test *serv_test;

	serv_test = (struct service_test *)serv->serv_handle;

	ret = mt_agent_exit_service(serv);
	if (ret != SERV_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wlan service exits failed!\n");

		return NDIS_STATUS_FAILURE;
	}

	os_free_mem(serv_test->test_op);
	os_free_mem(serv_test->test_winfo);
	os_free_mem(serv_test);

	serv->serv_id = 0;
	serv->serv_handle = NULL;

	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" wlan service closes successfully!\n");

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_WLAN_SERVICE */

#ifdef INTERFACE_SPEED_DETECT
/* estimate the physical link speed with certain percentage decrease,
 * the default range is 20% for now and the value could be modified for
 * further customization
 */

#define ESTIMATE_INTFACE_SPD(_value_) ((_value_<<2)/5)
#endif
/*rename from rt28xx_init*/
int mt_wifi_init(VOID *pAdSrc, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
	/* UCHAR EDCCACtrl; */
	UCHAR ucBandIdx = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;
	POS_COOKIE pObj = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	CHANNEL_CTRL *pChCtrl_hwband1;
#ifdef CCAPI_API_SUPPORT
	UINT32	Time;
	ULONG	TNow;
#endif

#ifdef INTERFACE_SPEED_DETECT
	UINT IfaceSpeed = 0;
#ifdef RTMP_PCI_SUPPORT
	UINT i;
	PCI_HIF_T *pci_hif = NULL;
#endif
#endif

	if (!pAd)
		return FALSE;
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\x1b[1;33m [mt_wifi_init] Test - pObj->ioctl_if = %d, pObj->ioctl_if_type = %d \x1b[m \n", pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
	}
#endif
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[mt_wifi_init] wdev == NULL\n");
		return FALSE;
	}
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	pChCtrl_hwband1 = hc_get_channel_ctrl(pAd->hdev_ctrl, 1);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef CONFIG_FWOWN_SUPPORT
	DriverOwn(pAd);
#endif

	asic_show_mac_info(pAd);

	/* reset Adapter flags */
	RTMP_CLEAR_FLAGS(pAd);

	/*for software system initialize*/
	if (rtmp_sys_init(pAd, pHostName) != TRUE)
		goto err2;

	Status = WfInit(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WfInit faild!!, ret=%d, cap=%p\n", Status, cap);
		goto err2;
	}

	/* initialize MLME*/
	Status = MlmeInit(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MlmeInit failed, Status[=0x%08x]\n", Status);
		goto err3;
	}

	tr_ctl_init(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	NICInitializeAsic(pAd);
#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
	tx_pwr_comp_init(pAd);
#ifdef WIN_NDIS

	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);

#endif /* WIN_NDIS */
#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */
	/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
	/* STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED); */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NDIS_STATUS_MEDIA_DISCONNECT Event B!\n");
#ifdef SMART_CARRIER_SENSE_SUPPORT
	SCS_init(pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT
	DynWmm_init(pAd);
#endif /* DYNAMIC_WMM_SUPPORT */
	RTMPIoctlRvRDebug_Init(pAd);
#ifdef MAC_INIT_OFFLOAD
	AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);
#endif /*MAC_INIT_OFFLOAD*/
#ifdef WIFI_MODULE_DVT
	mdvt_init(pAd);
#endif
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(pAd->OpMode) {
		rtmp_ap_init(pAd);
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(pAd->OpMode) {
		rtmp_sta_init(pAd, &pAd->StaCfg[MAIN_MSTA_ID].wdev);
	}
#endif
#ifdef CONFIG_ATE
	rtmp_ate_init(pAd);
#endif /*CONFIG_ATE*/
#ifdef INTERFACE_SPEED_DETECT
#if defined(RTMP_PCI_SUPPORT)
	/* Set detected speed if device is PCIe */
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE) {
		pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
		for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
			if (IfaceSpeed < pci_hif->pci_hif_chip[i]->cfg.IfaceSpeed) {
				/* Return first speed back in Connac2 so far.*/
				IfaceSpeed = pci_hif->pci_hif_chip[i]->cfg.IfaceSpeed;
				break;
			}
		}
		if (IfaceSpeed != 0) {
			IfaceSpeed = ESTIMATE_INTFACE_SPD(IfaceSpeed);
			set_interface_speed(pAd, IfaceSpeed);
		}
	}
#elif defined(RTMP_RBUS_SUPPORT)
	/* Set a high value because AXI interface is fast */
	IfaceSpeed = UINT_MAX;
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		set_interface_speed(pAd, IfaceSpeed);
#endif
#endif
	/*SW prepare done, enable system ready*/
	mt_sys_ready(pAd);
#ifdef DYNAMIC_VGA_SUPPORT

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable)
		dynamic_vga_enable(pAd);

#endif /* DYNAMIC_VGA_SUPPORT */

	/* Set PHY to appropriate mode and will update the ChannelListNum in this function */

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\x1b[1;33m [mt_wifi_init] Test - BandIdx = %d, pChCtrl->ChListNum = %d, pChCtrl_hwband1->ChListNum = %d \x1b[m \n", BandIdx, pChCtrl->ChListNum, pChCtrl_hwband1->ChListNum);

	/* TBD: if (pChCtrl->ChListNum == 0) { */
	if (0) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n");
		goto err3;
	}

#ifdef UAPSD_SUPPORT
	UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */
	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
	AsicStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */
#ifdef MT_WOW_SUPPORT
	ASIC_WOW_INIT(pAd);
#endif
#ifdef USB_IOT_WORKAROUND2
	pAd->bUSBIOTReady = TRUE;
#endif
#ifdef CONFIG_AP_SUPPORT
	AutoChSelInit(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckInit(pAd);
#endif
#ifdef CFG_SUPPORT_CSI
	csi_support_init(pAd);
#endif
#ifdef CCAPI_API_SUPPORT
	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"<==== ZERO LOSS ::Disbaling FW Log =======>\n");
	MtCmdFwLog2Host(pAd, 0, 0);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

	/* Trigger MIB counter update */
	for (ucBandIdx = 0; ucBandIdx < DBDC_BAND_NUM; ucBandIdx++)
#ifdef CCAPI_API_SUPPORT
	{
#endif
		pAd->OneSecMibBucket.Enabled[ucBandIdx] = TRUE;
#ifdef CCAPI_API_SUPPORT
		/*Initialize previous Read time to current time for calculating Sampling time*/
		pAd->ChannelStats.PrevReadTime[ucBandIdx] = Time;
	}
#endif

	pAd->MsMibBucket.Enabled = TRUE;
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,  "<==== mt_wifi_init, Status=%x\n", Status);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	TxBfModuleEnCtrl(pAd);

	mt_Trigger_Sounding_Packet(pAd,
							   TRUE,
							   0,
							   BF_PROCESSING,
							   0,
							   NULL);
	AsicTxBfHwEnStatusUpdate(pAd,
							 pAd->CommonCfg.ETxBfEnCond,
							 pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);

	/* TODO: consider DBDC case, move data to wdev*/
	pAd->bfdm.bfdm_bfee_enabled = TRUE; /* BFee HW is enabled by default */
	pAd->bfdm.bfdm_bitmap = 0; /* BFee adaption disabled by default. No BFDM_BFEE_ADAPTION_BITMAP */
#ifdef TXBF_DYNAMIC_DISABLE
	pAd->CommonCfg.ucAutoSoundingCtrl = 0;/* After interface down up, BF disable will be cancelled */
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */

	HeraInitStbcPriority(pAd);
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	if (TRUE != set_datcfg_ack_cts_timeout(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR! DAT CONFIG ACK Fail!!\n");
	}
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

	if (pAd->CommonCfg.bUseVhtRateFor2g)
		MtCmdSetUseVhtRateFor2G(pAd);

	if (pAd->CommonCfg.vht_1024_qam)
		MtCmdSetVht1024QamSupport(pAd);

#ifdef PS_STA_FLUSH_SUPPORT
	MtCmdPsStaFlushCtrl(pAd);
#endif /*PS_STA_FLUSH_SUPPORT*/

	return TRUE;
err3:
	MlmeHalt(pAd);
	RTMP_AllTimerListRelease(pAd);
err2:
	rtmp_sys_exit(pAd);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!! mt_wifi_init  fail !!!\n");
	return FALSE;
}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT8 band_idx = 0;
	INT i;

#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[MAIN_MSTA_ID];
#endif
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		FT_RIC_Init(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#ifdef MT_MAC
#ifdef RT_CFG80211_SUPPORT
	CFG80211_InitTxSCallBack(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* MT_MAC */
#endif /* CONFIG_STA_SUPPORT */
	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	HcSetMaxStaNum(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	/*
		To reduce connection time,
		do auto reconnect here instead of waiting STAMlmePeriodicExec to do auto reconnect.
	*/
	if (pAd->OpMode == OPMODE_STA)
		MlmeAutoReconnectLastSSID(pAd, &pStaCfg->wdev);

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11W_PMF_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		PMF_CFG *pPmfCfg = &pStaCfg->wdev.SecConfig.PmfCfg;

		pPmfCfg->MFPC = FALSE;
		pPmfCfg->MFPR = FALSE;
		pPmfCfg->PMFSHA256 = FALSE;

		if ((IS_AKM_WPA2_Entry(&pStaCfg->wdev) ||
		     IS_AKM_WPA2PSK_Entry(&pStaCfg->wdev) ||
		     IS_AKM_OWE_Entry(&pStaCfg->wdev)) &&
		     IS_CIPHER_AES_Entry(&pStaCfg->wdev)) {
			pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

			if (pPmfCfg->Desired_MFPC) {
				pPmfCfg->MFPC = TRUE;
				pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

				if (pPmfCfg->MFPR)
					pPmfCfg->PMFSHA256 = TRUE;
			}
		} else if (pPmfCfg->Desired_MFPC)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "[PMF] Security is not WPA2/WPA2PSK AES\n");

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF] MFPC=%d, MFPR=%d, SHA256=%d\n",
				 pPmfCfg->MFPC, pPmfCfg->MFPR, pPmfCfg->PMFSHA256);
	}

#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		PWSC_CTRL pWscControl = &pStaCfg->wdev.WscControl;

		WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], 0, FALSE, FALSE);
		WscInit(pAd, FALSE, BSS0);
#ifdef WSC_V2_SUPPORT
		WscInitRegistrarPair(pAd, pWscControl, BSS0);
#endif /* WSC_V2_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */
#ifdef COEX_SUPPORT
	/* SendAndesWLANStatus(pAd,WLAN_Device_ON,0); */
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
		MT76xxMLMEHook(pAd, MT76xx_WLAN_Device_ON, 0);
#endif /* COEX_SUPPORT */
#ifdef MT_WOW_SUPPORT
	pAd->WOW_Cfg.bWoWRunning = FALSE;
#endif
#ifdef CONFIG_AP_SUPPORT
	vow_init(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef RED_SUPPORT

	if (pAd->OpMode == OPMODE_AP)
		RedInit(pAd);

#endif /* RED_SUPPORT */
#ifdef KERNEL_RPS_ADJUST
	if (pAd->ixia_mode_ctl.kernel_rps_en)
		proc_rps_file_open(pAd);
#endif
	for (band_idx = BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->rts_retrylimit[band_idx] > 0)
			asic_set_rts_retrylimit(pAd, band_idx, pAd->rts_retrylimit[band_idx]);
	}

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		for (i = 0; i < 16; i++) {
			MtCmdCr4Set(pAd, WA_SET_OPTION_MPDU_RETRY_LIMIT, i, 0);
			if (pAd->retrylimit[i] > 0) {
				MtCmdCr4Set(pAd, WA_SET_OPTION_MPDU_RETRY_LIMIT, i, pAd->retrylimit[i]);
			}
		}
	}

	cp_support_is_enabled(pAd);
#ifdef GN_MIXMODE_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
		gn_mixmode_is_enable(pAd);
#endif /* GN_MIXMODE_SUPPORT */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	/* DfsDedicatedScanStart(pAd); */
	/* DfsSetInitDediatedScanStart(pAd); */
#endif

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
    if (pAd->ApCfg.BandSteering) {
	PBND_STRG_CLI_TABLE table;

	table = Get_BndStrgTable(pAd, BSS0);
	if (table) {
	    /* Inform daemon interface ready */
	    struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;

	    BndStrg_SetInfFlags(pAd, wdev, table, TRUE);
	}
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
}


VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct MCU_CTRL *prCtl = NULL;
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	INT j;
	struct customer_vendor_ie *ap_vendor_ie;
	struct customer_vendor_ie *apcli_vendor_ie;
	CUSTOMER_PROBE_RSP_VENDOR_IE *ap_probe_rsp_vendor_ie = NULL, *ap_probe_rsp_vendor_ie_temp = NULL;
	BSS_STRUCT *mbss = NULL;
	PDL_LIST ap_probe_rsp_vendor_ie_list = NULL;
	UINT32 ie_count;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN	InWOW = FALSE;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	prCtl = &pAd->MCUCtrl;
#ifdef CONFIG_STA_SUPPORT
#ifdef COEX_SUPPORT
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
		MT76xxMLMEHook(pAd, MT76xx_WLAN_Device_OFF, 0);

#endif /* COEX_SUPPORT */
#ifdef CREDENTIAL_STORE

	if (pAd->IndicateMediaState == NdisMediaStateConnected)
		StoreConnectInfo(pAd);
	else {
		RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
		pAd->StaCtIf.Changeable = FALSE;
		RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
	}

#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UINT32 i = 0;

		/* If dirver doesn't wake up firmware here,*/
		/* NICLoadFirmware will hang forever when interface is up again.*/
		for (i = 0; i < MAX_MULTI_STA; i++) {
			pStaCfg = &pAd->StaCfg[i];

			if (INFRA_ON(pStaCfg) && pStaCfg->PwrMgmt.bDoze)
				RTMP_FORCE_WAKEUP(pAd, pStaCfg);
		}

#ifdef RTMP_MAC_PCI
		{
			PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

			pci_hif->bPCIclkOff = FALSE;
		}
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef MT_MAC
	/* clear extend bss mac address */
	NdisZeroMemory(pAd->ExtendMBssAddr, sizeof(pAd->ExtendMBssAddr));
	NdisZeroMemory(pAd->ApcliAddr, sizeof(pAd->ApcliAddr));

	if (IS_HIF_TYPE(pAd, HIF_MT))
#endif /* MT_MAC */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		ap_vendor_ie = &pAd->ApCfg.MBSSID[j].ap_vendor_ie;
		if (ap_vendor_ie->pointer != NULL)
			os_free_mem(ap_vendor_ie->pointer);
		ap_vendor_ie->pointer = NULL;
		ap_vendor_ie->length = 0;

		mbss = &pAd->ApCfg.MBSSID[j];
		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		ie_count = DlListLen(ap_probe_rsp_vendor_ie_list);
		if (ie_count) {
			DlListForEachSafe(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_temp, ap_probe_rsp_vendor_ie_list,
				CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
				if (ap_probe_rsp_vendor_ie) {
					DlListDel(&ap_probe_rsp_vendor_ie->List);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s remove MAC["MACSTR"]\n",
						__func__, MAC2STR(ap_probe_rsp_vendor_ie->stamac));
					if (ap_probe_rsp_vendor_ie->pointer){
						os_free_mem(ap_probe_rsp_vendor_ie->pointer);
					}
					os_free_mem(ap_probe_rsp_vendor_ie);
				}
			}
		}
		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);
	}
	pAd->ApCfg.ap_probe_rsp_vendor_ie_count = 0;
#ifdef APCLI_SUPPORT
	for (j = 0; j < MAX_APCLI_NUM; j++) {
		apcli_vendor_ie = &pAd->StaCfg[j].apcli_vendor_ie;
		if (apcli_vendor_ie->pointer != NULL)
			os_free_mem(apcli_vendor_ie->pointer);
		apcli_vendor_ie->pointer = NULL;
		apcli_vendor_ie->length = 0;
	}
#endif /* APCLI_SUPPORT */
	for (j = 0; j < DBDC_BAND_NUM; j++) {
		UINT32 i = 0;

		for (i = 0; i < pAd->ScanCtrl[j].ScanTab.BssNr; i++) {
			if (pAd->ScanCtrl[j].ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer != NULL) {
				os_free_mem(pAd->ScanCtrl[j].ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer);
				pAd->ScanCtrl[j].ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.pointer = NULL;
			}
			pAd->ScanCtrl[j].ScanTab.BssEntry[i].CustomerBssEntry.vendor_ie.length = 0;
		}
	}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef EXT_BUILD_CHANNEL_LIST

	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(pAd->CommonCfg.pChDesp);

	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		BOOLEAN Cancelled = FALSE;
#ifdef DOT11N_DRAFT3

		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED) {
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}

#endif /* DOT11N_DRAFT3 */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(MT_WOW_SUPPORT) && defined(WOW_IFDOWN_SUPPORT))

		if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(&pAd->StaCfg[0])))
#endif
		{
			MacTableReset(pAd);
		}

#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef DOT11V_MBSSID_SUPPORT
		UCHAR BandIdx = 0;

		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
			pAd->ApCfg.dot11v_mbssid_bitmap[BandIdx] = 0;
#endif
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */
#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */
		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);
		/*#ifdef AUTO_CH_SELECT_ENHANCE*/
		/* Free BssTab & ChannelInfo tabbles.*/
		/*		AutoChBssTableDestroy(pAd); */
		/*		ChannelInfoDestroy(pAd); */
		/*#endif  AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_ATE
	ATEExit(pAd);
#endif /*CONFIG_ATE*/
	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	/* Close net tasklets*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		UINT32 i = 0;

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
		NICRestartFirmware(pAd);

	hif_poll_txrx_empty(pAd->hdev_ctrl, ALL_DMA);

#ifdef TPC_SUPPORT
	TpcReqTabExit(pAd);
#endif
#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_MAC_PCI
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE)) {
			DISABLE_TX_RX(pAd, RTMP_HALT);
			chip_interrupt_disable(pAd);
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/
#ifdef RTMP_PCI_SUPPORT

		if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE) {
			BOOLEAN brc = TRUE;

			brc = RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);

			if (brc == FALSE)
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "call RT28xxPciAsicRadioOff fail !!\n");
		}

#endif /* RTMP_PCI_SUPPORT */
#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	}

#ifdef SINGLE_SKU_V2
	RTMPResetSkuParam(pAd);
	RTMPResetBackOffParam(pAd);
#endif

	/* tm exit */
	tm_exit(pAd);

	/* qm exit */
	qm_exit(pAd);

	tr_ctl_exit(pAd);

	/*remove hw related system info*/
	WfSysPosExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */
	UserCfgExit(pAd); /* must after ba_reordering_resource_release */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		ExitTxSTypeTable(pAd);

#endif
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		FT_RIC_Release(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanDeInit(pAd);
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	AutoChSelRelease(pAd);
#endif/* CONFIG_AP_SUPPORT */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	/*+++Modify by woody to solve the bulk fail+++*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_Table_Destory(pAd);
#ifdef TDLS_AUTOLINK_SUPPORT
		TDLS_ClearEntryList(&pStaCfg->TdlsInfo.TdlsDiscovPeerList);
		NdisFreeSpinLock(&pStaCfg->TdlsInfo.TdlsDiscovPeerListSemLock);
		TDLS_ClearEntryList(&pStaCfg->TdlsInfo.TdlsBlackList);
		NdisFreeSpinLock(&pStaCfg->TdlsInfo.TdlsBlackListSemLock);
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
#ifdef MAC_REPEATER_SUPPORT
	RepeaterCliReset(pAd);
#endif /* MAC_REPEATER_SUPPORT */
	/* release all timers */
	RtmpusecDelay(2000);
	RTMP_AllTimerListRelease(pAd);
	MeasureReqTabExit(pAd);
	/* WCNCR00034259: moved from RTMP{Reset, free}TxRxRingMemory() */
	NdisFreeSpinLock(&pAd->CmdQLock);
#ifdef RTMP_TIMER_TASK_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
		NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
	/* Close Hw ctrl*/
	HwCtrlExit(pAd);
#ifdef KERNEL_RPS_ADJUST
	if (pAd->ixia_mode_ctl.kernel_rps_en)
		proc_rps_file_close(pAd);
#endif
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckExit(pAd);
#endif
#ifdef WIFI_MODULE_DVT
	mdvt_exit(pAd);
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		if (pAd->CalDCOCImage != NULL)
			os_free_mem(pAd->CalDCOCImage);

		if (pAd->CalDPDAPart1Image != NULL)
			os_free_mem(pAd->CalDPDAPart1Image);

		if (pAd->CalDPDAPart2Image != NULL)
			os_free_mem(pAd->CalDPDAPart2Image);
	}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->PreCalStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalStoreBuffer);
		pAd->PreCalStoreBuffer = NULL;
	}

	if (pAd->PreCalReStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalReStoreBuffer);
		pAd->PreCalReStoreBuffer = NULL;
	}

#endif/* PRE_CAL_TRX_SET2_SUPPORT */
	os_free_mem(pAd->EEPROMImage);
	pAd->EEPROMImage = NULL;

	/* free memory for rxv entry raw data */
	if (pAd->rxv_raw_data.rxv_pkt) {
		/* free memory for rxv pkt */
		os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
		os_free_mem(pAd->rxv_raw_data.rxv_pkt);
		pAd->rxv_raw_data.rxv_pkt = NULL;
		pAd->rxv_raw_data.rxv_byte_cnt = 0;
	}

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	if (pAd->pIQ_Array != NULL)
		os_free_mem(pAd->pIQ_Array);

	if (pAd->pL32Bit != NULL)
		os_free_mem(pAd->pL32Bit);

	if (pAd->pM32Bit != NULL)
		os_free_mem(pAd->pM32Bit);

	if (pAd->pH32Bit != NULL)
		os_free_mem(pAd->pH32Bit);
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
	/*multi profile release*/
#ifdef MULTI_PROFILE
	multi_profile_exit(pAd);
#endif /*MULTI_PROFILE*/
#ifdef OCE_SUPPORT
	OceRelease(pAd);
#endif /* OCE_SUPPORT */
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(NF_SUPPORT)
	/*pAd->Avg_NF[DBDC_BAND0] = pAd->Avg_NFx16[DBDC_BAND0] = 0;
	if (pAd->CommonCfg.dbdc_mode)
		pAd->Avg_NF[DBDC_BAND1] = pAd->Avg_NFx16[DBDC_BAND1] = 0;
	*/
#endif
#ifdef DABS_QOS
	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	NdisFreeSpinLock(&qos_param_table_lock);
#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s Nop_List_Backup call!!\n", __func__);
	Nop_List_Backup(pAd);
}

PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;
#ifdef DBDC_ONE_BAND1_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#if defined(MT_WIFI_MODULE) && defined(PROBE2LOAD_L1PROFILE)
	if (load_dev_l1profile(pAd) == NDIS_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "load l1profile succeed!\n");
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("load l1profile failed!\n"));
#endif
#ifdef DBDC_ONE_BAND1_SUPPORT
	if (cap->DbdcOneBand1Support)
		dev_name = get_dbdcdev_name_prefix(pAd, INT_MAIN);
	else
#endif
		dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	if (!dev_name)
		return NULL;

	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					 INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name, FALSE);
	return pDevNew;
}


#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
static void WriteConfToDatFile(RTMP_ADAPTER *pAd)
{
	char	*cfgData = 0, *offset = 0;
	RTMP_STRING *fileName = NULL, *pTempStr = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-----> WriteConfToDatFile\n");
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS)
		fileName = STA_PROFILE_PATH_RBUS;
	else
#endif /* RTMP_RBUS_SUPPORT */
		fileName = STA_PROFILE_PATH;

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);

	if (IS_FILE_OPEN_ERR(file_r)) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->1) %s: Error opening file %s\n", __func__, fileName);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return;
	} else {
		char tempStr[64] = {0};

		while ((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
			fileLen += rv;

		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);

		if (cfgData == NULL) {
			RtmpOSFileClose(file_r);
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CfgData mem alloc fail. (fileLen = %ld)\n", fileLen);
			goto out;
		}

		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (RTMP_STRING *)cfgData, fileLen);
		RtmpOSFileClose(file_r);

		if (rv != fileLen) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CfgData mem alloc fail, fileLen = %ld\n", fileLen);
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_TRUNC, 0);

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		offset = (PCHAR) rtstrstr((RTMP_STRING *) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (RTMP_STRING *)cfgData, (int)(offset - cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);

		if (!pTempStr) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pTempStr mem alloc fail. (512)\n");
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		for (;;) {
			int i = 0;
			RTMP_STRING *ptr;

			NdisZeroMemory(pTempStr, 512);
			ptr = (RTMP_STRING *) offset;

			while (*ptr && *ptr != '\n')
				pTempStr[i++] = *ptr++;

			pTempStr[i] = 0x00;

			if ((size_t)(offset - cfgData) < fileLen) {
				offset += strlen(pTempStr) + 1;

				if (strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0) {
					NdisZeroMemory(pTempStr, 512);
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					NdisMoveMemory(pTempStr + 5, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				} else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0) {
					struct _SECURITY_CONFIG *pSecConfig = &pStaCfg->wdev.SecConfig;

					NdisZeroMemory(pTempStr, 512);

					if (IS_AKM_OPEN(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=OPEN");
					else if (IS_AKM_SHARED(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=SHARED");
					else if (IS_AKM_AUTOSWITCH(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WEPAUTO");
					else if (IS_AKM_WPA1PSK(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WPAPSK");
					else if (IS_AKM_WPA2PSK(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WPA2PSK");
					else if (IS_AKM_WPA1(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WPA");
					else if (IS_AKM_WPA2(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WPA2");
					else if (IS_AKM_WPANONE(pSecConfig->AKMMap))
						sprintf(pTempStr, "AuthMode=WPANONE");
				} else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0) {
					struct _SECURITY_CONFIG *pSecConfig = &pAd->StaCfg[0].wdev.SecConfig;

					NdisZeroMemory(pTempStr, 512);

					if (IS_CIPHER_NONE(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=NONE");
					else if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=WEP");
					else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=TKIP");
					else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=AES");
					else if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=CCMP256");
					else if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=GCMP128");
					else if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher))
						sprintf(pTempStr, "EncrypType=GCMP256");
				}

				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			} else
				break;
		}

		RtmpOSFileClose(file_w);
	}

WriteErr:

	if (pTempStr)
		os_free_mem(pTempStr);

ReadErr:
WriteFileOpenErr:

	if (cfgData)
		os_free_mem(cfgData);

out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<----- WriteConfToDatFile\n");
	return;
}


INT write_dat_file_thread(
	IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	/* int	Status = 0; */
	pTask = (RTMP_OS_TASK *)Context;

	if (pTask == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: pTask is NULL\n", __func__);
		return 0;
	}

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: pAd is NULL\n", __func__);
		return 0;
	}

	RtmpOSTaskCustomize(pTask);
	/* Update ssid, auth mode and encr type to DAT file */
	WriteConfToDatFile(pAd);
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}

NDIS_STATUS WriteDatThread(
	IN  RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	RTMP_OS_TASK *pTask;

	if (pAd->bWriteDat == FALSE)
		return 0;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->WriteDatThreadInit()\n");
	pTask = &pAd->WriteDatTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpWriteDatTask", pAd);
	status = RtmpOSTaskAttach(pTask, write_dat_file_thread, (ULONG)&pAd->WriteDatTask);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--WriteDatThreadInit(), status=%d!\n", status);
	return status;
}
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT32 Enable;

	Enable = os_str_tol(arg, 0, 10);
	CmdExtGeneralTestOn(pAd, (Enable == 0) ? (FALSE) : (TRUE));
	return TRUE;
}

INT	Set_ErrDetectMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT8 mode = 0;
	UINT8 sub_mode = 0;
	PCHAR seg_str;

	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		sub_mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	CmdExtGeneralTestMode(pAd, mode, sub_mode);
	return TRUE;
}
#endif /* ERR_RECOVERY */

