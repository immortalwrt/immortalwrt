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
 ***************************************************************************

	Module Name:
	wifi_sys_info.c
*/
#include	"rt_config.h"
#ifdef VOW_SUPPORT
#include <ap_vow.h>
#endif /* VOW_SUPPORT */

/*Local function*/
static VOID get_network_type_str(UINT32 Type, CHAR *str)
{
	if (Type & NETWORK_INFRA)
		snprintf(str, 128, "%s", "NETWORK_INFRA");
	else if (Type & NETWORK_P2P)
		snprintf(str, 128, "%s", "NETWORK_P2P");
	else if (Type & NETWORK_IBSS)
		snprintf(str, 128, "%s", "NETWORK_IBSS");
	else if (Type & NETWORK_MESH)
		snprintf(str, 128, "%s", "NETWORK_MESH");
	else if (Type & NETWORK_BOW)
		snprintf(str, 128, "%s", "NETWORK_BOW");
	else if (Type & NETWORK_WDS)
		snprintf(str, 128, "%s", "NETWORK_WDS");
	else
		snprintf(str, 128, "%s", "UND");
}

/*
*
*/
#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
static VOID update_igmpinfo(struct wifi_dev *wdev, BOOLEAN bActive)
{
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UINT Enable = FALSE;

#ifdef IGMP_TVM_SUPPORT
	if (bActive == TRUE) {
		wdev->pIgmpMcastTable = NULL;
		wdev->IgmpTableSize = 0;
	}
#endif /* IGMP_TVM_SUPPORT */

	if (wdev->wdev_type == WDEV_TYPE_AP) {

		if (bActive == TRUE && wdev->IgmpSnoopEnable == TRUE)
			Enable = TRUE;

#ifdef IGMP_TVM_SUPPORT
		wdev->u4AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
		if (IgmpSnEnableTVMode(ad,
			wdev,
			wdev->IsTVModeEnable,
			wdev->TVModeType))
			Enable = wdev->TVModeType;
#endif	/* IGMP_TVM_SUPPORT */
		CmdMcastCloneEnable(ad, Enable, devinfo->BandIdx, devinfo->OwnMacIdx);

#ifdef IGMP_TVM_SUPPORT
		if (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))
			MulticastFilterInitMcastTable(ad, wdev, bActive);
#endif /* IGMP_TVM_SUPPORT */

	}
}
#endif	/*IGMP_SNOOP_SUPPORT */
#endif	/*CONFIG_AP_SUPPORT*/

/*
*
*/
static VOID dump_devinfo(struct _WIFI_INFO_CLASS *class)
{
	struct _DEV_INFO_CTRL_T *devinfo = NULL;
	struct wifi_dev *wdev = NULL;

	DlListForEach(devinfo, &class->Head, DEV_INFO_CTRL_T, list) {
		wdev = (struct wifi_dev *)devinfo->priv;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("#####WdevIdx (%d)#####\n", wdev->wdev_idx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Active: %d\n", devinfo->WdevActive));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BandIdx: %d\n", devinfo->BandIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("EnableFeature: %d\n", devinfo->EnableFeature));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OwnMacIdx: %d\n", devinfo->OwnMacIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OwnMacAddr: %x:%x:%x:%x:%x:%x\n", PRINT_MAC(devinfo->OwnMacAddr)));
	}
}

/*
*
*/
static VOID dump_bssinfo(struct _WIFI_INFO_CLASS *class)
{
	BSS_INFO_ARGUMENT_T *bss = NULL;
	struct wifi_dev *wdev = NULL;
	CHAR str[128] = "";

	DlListForEach(bss, &class->Head, BSS_INFO_ARGUMENT_T, list) {
		wdev = (struct wifi_dev *)bss->priv;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("#####WdevIdx (%d)#####\n", wdev->wdev_idx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("State: %d\n", bss->bss_state));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Bssid: %x:%x:%x:%x:%x:%x\n", PRINT_MAC(bss->Bssid)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CipherSuit: %d\n", bss->CipherSuit));
		get_network_type_str(bss->NetworkType, str);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("NetworkType: %s\n", str));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OwnMacIdx: %d\n", bss->OwnMacIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BssInfoFeature: %x\n", bss->u4BssInfoFeature));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ConnectionType: %d\n", bss->u4ConnectionType));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BcMcWlanIdx: %d\n", bss->bmc_wlan_idx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BssIndex: %d\n", bss->ucBssIndex));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("PeerWlanIdx: %d\n", bss->peer_wlan_idx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("WmmIdx: %d\n", bss->WmmIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BcTransmit: (Mode/BW/MCS) %d/%d/%d\n", bss->BcTransmit.field.MODE,
			   bss->BcTransmit.field.BW, bss->BcTransmit.field.MCS));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("McTransmit: (Mode/BW/MCS) %d/%d/%d\n", bss->McTransmit.field.MODE,
			   bss->BcTransmit.field.BW, bss->BcTransmit.field.MCS));
	}
}

/*
*
*/
static VOID dump_starec(struct _WIFI_INFO_CLASS *class)
{
	struct _STA_REC_CTRL_T *starec = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;

	DlListForEach(starec, &class->Head, STA_REC_CTRL_T, list) {
		tr_entry = (STA_TR_ENTRY *)starec->priv;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("#####MacEntry (%d)#####\n", tr_entry->wcid));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("PeerAddr: %x:%x:%x:%x:%x:%x\n", PRINT_MAC(tr_entry->Addr)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("WlanIdx: %d\n", starec->WlanIdx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("BssIndex: %d\n", starec->BssIndex));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ConnectionState: %d\n", starec->ConnectionState));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ConnectionType: %d\n", starec->ConnectionType));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("EnableFeature: %x\n", starec->EnableFeature));
	}
}

/*
*
*/
static VOID add_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->DevInfo.Head, DEV_INFO_CTRL_T, list) {
		if (devinfo == tmp) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): DevInfo %d already exist",
					  __func__, devinfo->OwnMacIdx));
			return;
		}
	}
	DlListAddTail(&wsys->DevInfo.Head, &devinfo->list);
	devinfo->priv = (VOID *)wdev;
	wsys->DevInfo.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID add_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument, *tmp;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->BssInfo.Head, BSS_INFO_ARGUMENT_T, list) {
		if (bss == tmp) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): BssInfo %d already exist",
					  __func__, bss->ucBssIndex));
			return;
		}
	}
	DlListAddTail(&wsys->BssInfo.Head, &bss->list);
	bss->priv = (VOID *)wdev;
	wsys->BssInfo.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID add_starec(struct _RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry)
{
	struct _WIFI_SYS_INFO *wsys = &pAd->WifiSysInfo;
	struct _STA_REC_CTRL_T *strec = &tr_entry->StaRec, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp == strec) {
			OS_SEM_UNLOCK(&wsys->lock);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): STARec %d already exist\n",
					  __func__, strec->WlanIdx));
			return;
		}
	}
	DlListAddTail(&wsys->StaRec.Head, &strec->list);
	strec->priv = (VOID *)tr_entry;
	wsys->StaRec.Num++;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
struct _STA_REC_CTRL_T *get_starec_by_wcid(struct _RTMP_ADAPTER *ad, INT wcid)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _STA_REC_CTRL_T *sta_rec = NULL, *tmp = NULL;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp->WlanIdx == wcid) {
			sta_rec = tmp;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
	return sta_rec;
}

/*
*
*/
static VOID del_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _DEV_INFO_CTRL_T *devinfo = &wdev->DevInfo;

	OS_SEM_LOCK(&wsys->lock);
	DlListDel(&devinfo->list);
	wsys->DevInfo.Num--;
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID del_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument, *tmp;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->BssInfo.Head, BSS_INFO_ARGUMENT_T, list) {
		if (tmp == bss) {
			DlListDel(&bss->list);
			wsys->BssInfo.Num--;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID del_starec(struct _RTMP_ADAPTER *ad, struct _STA_TR_ENTRY *tr_entry)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct _STA_REC_CTRL_T *starec = &tr_entry->StaRec, *tmp;

	OS_SEM_LOCK(&wsys->lock);
	DlListForEach(tmp, &wsys->StaRec.Head, STA_REC_CTRL_T, list) {
		if (tmp == starec) {
			DlListDel(&starec->list);
			wsys->StaRec.Num--;
			break;
		}
	}
	OS_SEM_UNLOCK(&wsys->lock);
}

/*
*
*/
static VOID fill_devinfo(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN act,
	struct _DEV_INFO_CTRL_T *devinfo)
{
	struct _DEV_INFO_CTRL_T *org = &wdev->DevInfo;

	os_move_mem(devinfo, org, sizeof(DEV_INFO_CTRL_T));
	devinfo->WdevActive = act;
	devinfo->OwnMacIdx = wdev->OmacIdx;
	os_move_mem(devinfo->OwnMacAddr, wdev->if_addr, MAC_ADDR_LEN);
	if (wdev->wdev_type != WDEV_TYPE_WDS)	/* WDS share DevInfo with normal AP */
		devinfo->EnableFeature = DEVINFO_ACTIVE_FEATURE;
	devinfo->BandIdx = HcGetBandByWdev(wdev);
	os_move_mem(&wdev->DevInfo, devinfo, sizeof(DEV_INFO_CTRL_T));

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	update_igmpinfo(wdev, act);
#endif
#endif
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("%s(): Active=%d,OwnMacIdx=%d,EnableFeature=%d,BandIdx=%d\n", __func__,
		devinfo->WdevActive, devinfo->OwnMacIdx, devinfo->EnableFeature, devinfo->BandIdx));
}

/*
*
*/
static inline VOID fill_bssinfo_active(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	BssInfoArgumentLink(ad, wdev, bssinfo);
	bssinfo->bss_state = BSS_ACTIVE;
	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
}

/*
*
*/
static inline VOID fill_bssinfo_deactive(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	WDEV_BSS_STATE(wdev) = BSS_INITED;
	os_move_mem(bssinfo, &wdev->bss_info_argument, sizeof(wdev->bss_info_argument));
}

/*
*
*/
static VOID fill_bssinfo(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	BOOLEAN act,
	struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	if (act)
		fill_bssinfo_active(ad, wdev, bssinfo);
	else
		fill_bssinfo_deactive(ad, wdev, bssinfo);
}

/*
*
*/
static VOID fill_starec(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	struct _STA_TR_ENTRY *tr_entry,
	struct _STA_REC_CTRL_T *starec)
{
	os_move_mem(starec, &tr_entry->StaRec, sizeof(tr_entry->StaRec));
#ifdef DOT11_HE_AX
	fill_starec_he(wdev, entry, starec);
#endif /*DOT11_HE_AX*/
}



/*
*
*/
static INT call_wsys_notifieriers(INT val, struct wifi_dev *wdev, void *v)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;
	struct wsys_notify_info info;
	INT ret;

	/*fill wsys notify info*/
	info.wdev = wdev;
	info.v = v;
	/*traversal caller for wsys notify chain*/
	ret = mt_notify_call_chain(&wsys->wsys_notify_head, val, &info);
	return ret;
}

/*export function*/
/*
*
*/
INT register_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_register(&wsys->wsys_notify_head, ne);

	return ret;
}

/*
*
*/
INT unregister_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_unregister(&wsys->wsys_notify_head, ne);
	return ret;
}

/*
*
*/
VOID wifi_sys_reset(struct _WIFI_SYS_INFO *wsys)
{
	DlListInit(&wsys->DevInfo.Head);
	DlListInit(&wsys->StaRec.Head);
	DlListInit(&wsys->BssInfo.Head);

	wsys->DevInfo.Num = 0;
	wsys->StaRec.Num = 0;
	wsys->BssInfo.Num = 0;
}

/*
*
*/
VOID wifi_sys_init(struct _RTMP_ADAPTER *ad)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;

	NdisAllocateSpinLock(ad, &wsys->lock);
	wifi_sys_reset(wsys);
	INIT_NOTIFY_HEAD(ad, &wsys->wsys_notify_head);
}


/*
*
*/
VOID wifi_sys_dump(struct _RTMP_ADAPTER *ad)
{
	struct _WIFI_SYS_INFO *wsys = &ad->WifiSysInfo;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Current DevInfo Num: %d\n", wsys->DevInfo.Num));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	dump_devinfo(&wsys->DevInfo);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Current BssInfo Num: %d\n", wsys->BssInfo.Num));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	dump_bssinfo(&wsys->BssInfo);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Current StaRec Num: %d\n", wsys->StaRec.Num));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("===============================\n"));
	dump_starec(&wsys->StaRec);
}

/*
*
*/
INT wifi_sys_update_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _DEV_INFO_CTRL_T *new)
{
	INT len = sizeof(wdev->DevInfo) - sizeof(wdev->DevInfo.list);

	os_move_mem(&wdev->DevInfo, new, len);
	if (new->WdevActive) {
		add_devinfo(ad, wdev);
		/*notify other modules, hw resouce is acquired down*/
		call_wsys_notifieriers(WSYS_NOTIFY_OPEN, wdev, NULL);
	} else {
		del_devinfo(ad, wdev);
		/*release hw resource*/
		HcReleaseRadioForWdev(wdev->sys_handle, wdev);
	}
	return 0;
}

/*
*
*/
INT wifi_sys_update_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *new)
{
	BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;
	INT len = sizeof(wdev->bss_info_argument) - sizeof(bss->list);

	if (new->bss_state >= BSS_ACTIVE) {
		os_move_mem(bss, new, len);
		add_bssinfo(ad, wdev);
		WDEV_BSS_STATE(wdev) = BSS_READY;
		/*notify other modules, bss related setting is done*/
		call_wsys_notifieriers(WSYS_NOTIFY_LINKUP, wdev, bss);
	} else {
		del_bssinfo(ad, wdev);
		BssInfoArgumentUnLink(ad, wdev);
	}
	return 0;
}

/*
*
*/
INT wifi_sys_update_starec(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new)
{
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *) new->priv;
	struct _STA_REC_CTRL_T *sta_rec = &tr_entry->StaRec;
	INT len = sizeof(tr_entry->StaRec) - sizeof(sta_rec->list);

	os_move_mem(sta_rec, new, len);
	if (new->ConnectionState == STATE_DISCONNECT) {
		/*remove starec*/
		del_starec(ad, tr_entry);
	} else {
		add_starec(ad, tr_entry);
		/*notify other modules, starec is prepeare done*/
		call_wsys_notifieriers(WSYS_NOTIFY_CONNT_ACT, tr_entry->wdev, tr_entry);
	}
	return 0;
}

/*
* for peer update usage
*/
INT wifi_sys_update_starec_info(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new)
{
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *) new->priv;
	struct _STA_REC_CTRL_T *sta_rec = &tr_entry->StaRec;
	INT len = sizeof(tr_entry->StaRec) - sizeof(sta_rec->list);

	os_move_mem(sta_rec, new, len);
	call_wsys_notifieriers(WSYS_NOTIFY_STA_UPDATE, tr_entry->wdev, tr_entry);

	return 0;
}


/*wcid: STA is peer root AP, AP is bmc wcid*/
static UINT32 bssinfo_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	UINT16 wcid,
	UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	/*basic features*/
	UINT32 features = (BSS_INFO_OWN_MAC_FEATURE
					   | BSS_INFO_BASIC_FEATURE
					   | BSS_INFO_RF_CH_FEATURE
					   | BSS_INFO_BROADCAST_INFO_FEATURE
					   | BSS_INFO_PROTECT_INFO_FEATURE);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (cap->fgRateAdaptFWOffload == TRUE)
		features |= BSS_INFO_RA_FEATURE;
#endif

	/*HW BSSID features*/
	if (wdev->OmacIdx > HW_BSSID_MAX)
		features |= BSS_INFO_EXT_BSS_FEATURE;
	else
		features |= BSS_INFO_SYNC_MODE_FEATURE;

	if (IS_ASIC_CAP(ad, fASIC_CAP_HW_TX_AMSDU))
		features |= BSS_INFO_HW_AMSDU_FEATURE;

#ifdef CONFIG_STA_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA && conn_type == CONNECTION_INFRA_STA)
		bssinfo_sta_feature_decision(wdev, wcid, &features);

#endif /*CONFIG_STA_SUPPORT*/

#ifdef DOT11_HE_AX
	bssinfo_he_feature_decision(wdev, &features);
#endif /*DOT11_HE_AX*/
#ifdef BCN_PROTECTION_SUPPORT
	if (wdev->SecConfig.bcn_prot_cfg.bcn_prot_en)
		features |= BSS_INFO_BCN_PROT_FEATURE;
#endif
	*feature = features;
	return TRUE;
}

#ifdef DOT11_HE_AX
UINT32 starec_muru_feature_decision(struct wifi_dev *wdev,
		struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	UINT32 features = 0;

	if (wlan_config_get_mu_dl_ofdma(wdev) || wlan_config_get_mu_ul_ofdma(wdev)
		|| wlan_config_get_mu_dl_mimo(wdev) || wlan_config_get_mu_ul_mimo(wdev)) {

		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_STA))
			features |= STA_REC_MURU_FEATURE;
	}

	*feature |= features;

	return TRUE;
}
#endif /*DOT11_HE_AX*/

/*
*
*/
static VOID starec_security_set(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, struct _STA_REC_CTRL_T *sta_rec)
{
	ASIC_SEC_INFO *asic_sec_info = &sta_rec->asic_sec_info;
	/* Set key material to Asic */
	os_zero_mem(asic_sec_info, sizeof(ASIC_SEC_INFO));
	asic_sec_info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	asic_sec_info->Direction = SEC_ASIC_KEY_BOTH;
	asic_sec_info->Wcid = entry->wcid;
	asic_sec_info->BssIndex = entry->func_tb_idx;
	asic_sec_info->KeyIdx = entry->SecConfig.PairwiseKeyId;
	asic_sec_info->Cipher = entry->SecConfig.PairwiseCipher;
	asic_sec_info->KeyIdx = entry->SecConfig.PairwiseKeyId;
	os_move_mem(&asic_sec_info->Key, &entry->SecConfig.WepKey[entry->SecConfig.PairwiseKeyId], sizeof(SEC_KEY_INFO));
	os_move_mem(&asic_sec_info->PeerAddr[0], entry->Addr, MAC_ADDR_LEN);
}

/*sta rec feature decision*/
static UINT32 starec_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	struct _MAC_TABLE_ENTRY *entry,
	UINT32 *feature)
{
	/*basic feature*/
	UINT32 features = (STA_REC_BASIC_STA_RECORD_FEATURE
					   | STA_REC_TX_PROC_FEATURE);
	features |= STA_REC_WTBL_FEATURE;

	if (conn_type != CONNECTION_INFRA_BC) {
		/*ht features */
#ifdef DOT11_N_SUPPORT
		starec_ht_feature_decision(wdev, entry, &features);
#ifdef DOT11_VHT_AC
		starec_vht_feature_decision(wdev, entry, &features);
#endif /*DOT11_VHT_AC*/
#ifdef TXBF_SUPPORT
		starec_txbf_feature_decision(wdev, entry, &features);
#endif /* TXBF_SUPPORT */
#ifdef DOT11_HE_AX
		starec_he_feature_decision(wdev, entry, &features);
		starec_muru_feature_decision(wdev, entry, &features);
#endif /*DOT11_HE_AX*/
#endif /*DOT11_N_SUPPORT*/
	}

#ifdef CONFIG_AP_SUPPORT

	if (conn_type == CONNECTION_INFRA_STA)
		starec_ap_feature_decision(wdev, entry, &features);

#endif /*CONFIG_AP_SUPPORT*/
	/*return value, must use or operation*/
	*feature = features;
	return TRUE;
}

static UINT32 starec_security_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR *state)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry;
	UCHAR port_sec = STATE_DISCONNECT;

	/*for bmc case*/
	if (!entry) {
		port_sec = STATE_PORT_SECURE;
		goto end;
	}

	/*for uc case*/
	tr_entry = &tr_ctl->tr_entry[entry->wcid];

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
	case WDEV_TYPE_TDLS:
		if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (IS_AKM_WPA_CAPABILITY_Entry(entry)
#ifdef WSC_INCLUDED
			|| ((wdev->WscControl.WscConfMode != WSC_DISABLE)
			&& (wdev->WscControl.bWscTrigger))
#endif
			))
			port_sec = STATE_CONNECTED;
		else if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			port_sec = STATE_PORT_SECURE;

		break;

#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (IS_AKM_WPA_CAPABILITY_Entry(entry)
#ifdef DOT1X_SUPPORT
				|| IS_IEEE8021X(&entry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
				|| entry->bWscCapable))
			port_sec = STATE_CONNECTED;
		else if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			|| (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) { /* delay the port secured for open none seciruty */

#ifdef FAST_EAPOL_WAR
		/*
		*	set STATE_CONNECTED first in open security mode,
		*	after asso resp is sent out, then set STATE_PORT_SECURE.
		*/
			port_sec = STATE_CONNECTED;
#else /* FAST_EAPOL_WAR */
			port_sec = STATE_PORT_SECURE;
			CheckBMCPortSecured(ad, entry, TRUE);
#endif /* !FAST_EAPOL_WAR */
		}
	}
	break;
#endif /*CONFIG_AP_SUPPORT*/

	default:
		port_sec = STATE_PORT_SECURE;
		break;
	}

end:
	*state = port_sec;
	return TRUE;
}

/*
*
*/
INT wifi_sys_open(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));

	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}

	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(), wdev idx = %d\n", __func__, wdev->wdev_idx));
	
	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_OPEN_CLOSE)
	{
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;
		}
	}

	if (!wdev->DevInfo.WdevActive && (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID)) {
		wlan_operate_set_state(wdev, WLAN_OPER_STATE_VALID);
		/*acquire wdev related attribute*/
		wdev_attr_update(wdev->sys_handle, wdev);
		/* init/re-init wdev fsm */
		wdev_fsm_init(wdev);
		fill_devinfo(wdev->sys_handle, wdev, TRUE, &wsys->DevInfoCtrl);
		wsys->wdev = wdev;
		/*update to hwctrl*/
		HW_WIFISYS_OPEN(wdev->sys_handle, wsys);
	} else if (ad->wf_lock_op & WDEV_LOCK_OP_OPEN_CLOSE){
		/* add to handle wifi_sys operation race condition */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not enqueue!! WdevActive=%d, wpf_op=%d\n", 
		__func__, wdev->DevInfo.WdevActive, wlan_operate_get_state(wdev)));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): release wf_link_lock.\n", __func__, wdev->wdev_idx));
		if (ad->wf_link_lock_flag) {
			ad->wf_link_lock_flag = FALSE;
			ad->wf_link_timestamp = 0;
			RTMP_SEM_EVENT_UP(&ad->wf_link_lock);
		}
	}
	
	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_close(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));

	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}

	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, wdev idx = %d\n",
		__func__, wdev->wdev_idx));
	
	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_OPEN_CLOSE)
	{
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;
		}
	}

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_VALID) {

		wlan_operate_set_state(wdev, WLAN_OPER_STATE_INVALID);

		/* Un-initialize the pDot11H of wdev */
		UpdateDot11hForWdev(wdev->sys_handle, wdev, FALSE);

		/* WDS share DevInfo with normal AP */
		if (wdev->wdev_type != WDEV_TYPE_WDS)
			fill_devinfo(wdev->sys_handle, wdev, FALSE, &wsys->DevInfoCtrl);

		wsys->wdev = wdev;
		/*notify other module will release hw resource*/
		call_wsys_notifieriers(WSYS_NOTIFY_CLOSE, wdev, NULL);
		/*update to hwctrl*/
		HW_WIFISYS_CLOSE(wdev->sys_handle, wsys);
	} else if (ad->wf_lock_op & WDEV_LOCK_OP_OPEN_CLOSE) {
		/* add to handle wifi_sys operation race condition */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not enqueue!! wpf_op=%d\n", 
		__func__, wlan_operate_get_state(wdev)));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): release wf_link_lock.\n", __func__, wdev->wdev_idx));
		if (ad->wf_link_lock_flag) {
			ad->wf_link_lock_flag = FALSE;
			ad->wf_link_timestamp = 0;
			RTMP_SEM_EVENT_UP(&ad->wf_link_lock);
		}
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct WIFI_SYS_CTRL *wsys;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _STA_TR_ENTRY *tr_entry = &ad->tr_ctl.tr_entry[entry->tr_tb_idx];
	struct _STA_REC_CTRL_T *new_sta;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	new_sta = &wsys->StaRecCtrl;
	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_CONN_DISCONN)
	{
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;
		}
	}

	if (entry->EntryState == ENTRY_STATE_SYNC) {
		entry->EntryState = ENTRY_STATE_NONE;
		entry->WtblSetFlag = FALSE;
		/* Deactive StaRec in FW */
		new_sta->BssIndex = wdev->bss_info_argument.ucBssIndex;
		new_sta->WlanIdx = entry->wcid;
		new_sta->ConnectionType = entry->ConnectionType;
		new_sta->ConnectionState = STATE_DISCONNECT;
		new_sta->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		new_sta->priv = tr_entry;

		if (IS_ENTRY_CLIENT(entry)) {
			/* there is no need to set txop when sta connected */
			wsys->skip_set_txop = TRUE;
		} else if (IS_ENTRY_REPEATER(entry)) {
			/* skip disable txop for repeater case since apcli is connected */
			wsys->skip_set_txop = TRUE;
		}
		wsys->wdev = wdev;
		/*notify other module will release starec*/
		call_wsys_notifieriers(WSYS_NOTIFY_DISCONNT_ACT, wdev, tr_entry);
		/*send event for release starec*/
		HW_WIFISYS_PEER_LINKDOWN(ad, wsys);
#ifdef CONFIG_AP_SUPPORT

		if (wdev->wdev_type == WDEV_TYPE_AP)
			CheckBMCPortSecured(ad, entry, FALSE);

#endif /* CONFIG_AP_SUPPORT */
	}	else if (ad->wf_lock_op & WDEV_LOCK_OP_CONN_DISCONN) {
		/* add to handle wifi_sys operation race condition */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not enqueue!! entry->EntryState=%d\n", 
		__func__, entry->EntryState));
		if (ad->wf_link_lock_flag) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): release wf_link_lock.\n", __func__, wdev->wdev_idx));
			ad->wf_link_lock_flag = FALSE;
			ad->wf_link_timestamp = 0;
			RTMP_SEM_EVENT_UP(&ad->wf_link_lock);
		}
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/* Updating STA Connection Status */
VOID update_sta_conn_state(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[entry->tr_tb_idx];
	UCHAR state = 0;

	starec_security_decision(wdev, entry, &state);
	tr_entry->StaRec.ConnectionState = state;
}

/*wifi system connection action*/
INT wifi_sys_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct WIFI_SYS_CTRL *wsys;
	struct _STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry = &ad->tr_ctl.tr_entry[entry->tr_tb_idx];
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;
	PEER_LINKUP_HWCTRL *lu_ctrl = NULL;
	UINT32 features = 0;
	UCHAR state = 0;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}

	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_CONN_DISCONN) {
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;
		}
	}

	/*indicate mac entry under sync to hw*/
	entry->EntryState = ENTRY_STATE_SYNC;
	/*star to fill wifi sys control*/
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));
	/*generic connection action*/
	/*sta rec feature & security update*/
	starec_feature_decision(wdev, entry->ConnectionType, entry, &features);
	starec_security_decision(wdev, entry, &state);
	/*prepare basic sta rec*/
	sta_rec = &wsys->StaRecCtrl;
	fill_starec(wdev, entry, tr_entry, sta_rec);
	sta_rec->BssIndex = bss->ucBssIndex;
	sta_rec->WlanIdx = entry->wcid;
	sta_rec->ConnectionType = entry->ConnectionType;
	sta_rec->ConnectionState = state;
	sta_rec->EnableFeature =  features;
	sta_rec->IsNewSTARec = TRUE;
	sta_rec->priv = tr_entry;
	/*prepare starec */
	if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE)
		starec_security_set(wdev, entry, sta_rec);

	/*specific part*/
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		wsys->skip_set_txop = TRUE;/* there is no need to set txop when sta connected.*/
		os_alloc_mem(NULL, (UCHAR **)&lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
		os_zero_mem(lu_ctrl, sizeof(PEER_LINKUP_HWCTRL));
#ifdef DOT11_N_SUPPORT

		if (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_RDG_CAPABLE))
			lu_ctrl->bRdgCap = TRUE;

#endif /*DOT11_N_SUPPORT*/
		wsys->priv = (VOID *)lu_ctrl;
	}

	wsys->wdev = wdev;
	HW_WIFISYS_PEER_LINKUP(ad, wsys);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d, AKMMap=%d\n",
			  __func__, entry->wcid, sta_rec->ConnectionState, entry->SecConfig.AKMMap));

	/* WiFi Certification config per peer */
	sta_set_wireless_sta_configs(ad, entry);

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

/*
*
*/
INT wifi_sys_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct WIFI_SYS_CTRL *wsys;
	STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry;
	UCHAR state = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss;
	UINT32 features = 0;
	UINT16 wcid;
	UCHAR *addr = NULL;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;

 	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, wdev idx = %d\n",
 		__func__, wdev->wdev_idx));

	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}

	/*if interface down up should not run ap link up (for apstop/apstart check)*/
	if (!HcIsRadioAcq(wdev))
		return TRUE;

	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));

	bss = &wsys->BssInfoCtrl;
	
	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_LINK_UP_DOWN) {
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;
		}
	}

	//if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
	if (WDEV_BSS_STATE(wdev) == BSS_INIT && HcIsRadioAcq(wdev)) { 
#ifdef CONFIG_AP_SUPPORT
		ap_key_table_init(ad, wdev);
#endif
		/*prepare bssinfo*/
		fill_bssinfo(wdev->sys_handle, wdev, TRUE, bss);
		/* Need to update ucBssIndex of wdev here immediately because
		it could be used if invoke wifi_sys_conn_act subsequently. */
		wdev->bss_info_argument.ucBssIndex = bss->ucBssIndex;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:wdev(type=%d,func_idx=%d,wdev_idx=%d),BssIndex(%d)\n",
			__func__, wdev->wdev_idx, wdev->wdev_type, wdev->func_idx,
			wdev->bss_info_argument.ucBssIndex));

#ifdef CONFIG_OWE_SUPPORT
		/* To allow OWE ApCli to connect to Open bss */
		if (((wdev->wdev_type == WDEV_TYPE_STA) && (IS_AKM_OWE(wdev->SecConfig.AKMMap)))) {
			if (IS_AKM_OPEN_ONLY(entry->SecConfig.AKMMap) && IS_CIPHER_NONE(entry->SecConfig.PairwiseCipher)) {
				bss->CipherSuit = SecHWCipherSuitMapping(entry->SecConfig.PairwiseCipher);
			}
		}
#endif
		wcid = (entry) ? entry->wcid : bss->bmc_wlan_idx;
		bssinfo_feature_decision(wdev, bss->u4ConnectionType, wcid, &features);
		bss->peer_wlan_idx = wcid;
		bss->u4BssInfoFeature = features;
		/*wds type should not this bmc starec*/
		if (wdev->wdev_type == WDEV_TYPE_WDS) {
			bss->bmc_wlan_idx = wcid;
		} else {
			/*prepare bmc starec*/
			starec_feature_decision(wdev, CONNECTION_INFRA_BC, NULL, &features);
			starec_security_decision(wdev, NULL, &state);

#ifdef MBSS_AS_WDS_AP_SUPPORT
			if (wdev->wds_enable) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("WDS Enable setting 4 address mode for %d entry \n",
					bss->bmc_wlan_idx));
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(ad, bss->bmc_wlan_idx, TRUE);
			}
#endif
			/*1. get tr entry here, since bss info is acquired above */
			tr_entry = &tr_ctl->tr_entry[wdev->tr_tb_idx];
			sta_rec = &wsys->StaRecCtrl;
			/* BC sta record should always set STATE_PORT_SECURE*/
			sta_rec->BssIndex = bss->ucBssIndex;
			sta_rec->WlanIdx = bss->bmc_wlan_idx;
			sta_rec->ConnectionState = state;
			sta_rec->ConnectionType = CONNECTION_INFRA_BC;
			sta_rec->EnableFeature = features;
			sta_rec->IsNewSTARec = TRUE;
			sta_rec->priv = tr_entry;
#ifdef CONFIG_AP_SUPPORT

			if (wdev->wdev_type == WDEV_TYPE_AP)
				ap_set_key_for_sta_rec(ad, wdev, sta_rec);
#endif /*CONFIG_AP_SUPPORT*/

			/*BMC STAREC*/
			addr  = (entry) ? entry->Addr : wdev->bssid;
			os_move_mem(tr_entry->Addr, addr, MAC_ADDR_LEN);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
				  __func__, bss->bmc_wlan_idx, sta_rec->ConnectionState));
		}
		/*update to hw ctrl*/
		wsys->wdev = wdev;
		HW_WIFISYS_LINKUP(ad, wsys);
	}	else if (ad->wf_lock_op & WDEV_LOCK_OP_LINK_UP_DOWN) {
		/* add to handle wifi_sys operation race condition */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not enqueue!! bss_state=%d\n", 
		__func__, WDEV_BSS_STATE(wdev)));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): release wf_link_lock.\n", __func__, wdev->wdev_idx));
		if (ad->wf_link_lock_flag) {
			ad->wf_link_lock_flag = FALSE;
			ad->wf_link_timestamp = 0;
			RTMP_SEM_EVENT_UP(&ad->wf_link_lock);
		}
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

INT wifi_sys_linkdown(struct wifi_dev *wdev)
{
	struct WIFI_SYS_CTRL *wsys;
	struct _STA_REC_CTRL_T *sta_rec;
	struct _STA_TR_ENTRY *tr_entry;
	struct _BSS_INFO_ARGUMENT_T *bss;
	UINT32 features = 0;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	/* add to handle wifi_sys operation race condition */
	UINT32 ret = 0;

 	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), wdev idx = %d\n",
 		__func__, wdev->wdev_idx));
	os_alloc_mem(NULL, (UCHAR **)&wsys, sizeof(struct WIFI_SYS_CTRL));
	if (!wsys) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Allocate memory failed!", __func__));
		return FALSE;
	}
	os_zero_mem(wsys, sizeof(struct WIFI_SYS_CTRL));

	bss = &wsys->BssInfoCtrl;
	
	/* add to handle wifi_sys operation race condition */
	if (ad->wf_lock_op & WDEV_LOCK_OP_LINK_UP_DOWN) {
		RTMP_SEM_EVENT_TIMEOUT(&ad->wf_link_lock, WIFI_LINK_MAX_TIME, ret);
		if (!ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): get wf_link_lock.\n", __func__, wdev->wdev_idx));
			NdisGetSystemUpTime(&ad->wf_link_timestamp);
			ad->wf_link_lock_flag = TRUE; 
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d): get wf_link_lock fail!!\n", __func__, wdev->wdev_idx));
			return FALSE;	
		}
	}

	if (WDEV_BSS_STATE(wdev) >= BSS_INITED) {
		fill_bssinfo(ad, wdev, FALSE, bss);

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:wdev(type=%d,func_idx=%d,wdev_idx=%d),BssIndex(%d)\n",
			__func__, wdev->wdev_type, wdev->func_idx, wdev->wdev_idx,
			wdev->bss_info_argument.ucBssIndex));

		bssinfo_feature_decision(wdev, bss->u4ConnectionType, bss->bmc_wlan_idx, &features);
		bss->u4BssInfoFeature = features;
		/* refill BandIdx that be cleared in fill_bssinfo */
		bss->ucBandIdx = HcGetBandByWdev(wdev);
		/*wds should not bmc starec*/
		if (wdev->wdev_type != WDEV_TYPE_WDS) {
			/*update sta rec.*/
			/*1. get tr entry here, since bss info is acquired above */
			sta_rec = &wsys->StaRecCtrl;
			tr_entry = &tr_ctl->tr_entry[wdev->tr_tb_idx];
			if (tr_entry->StaRec.ConnectionState) {
				sta_rec->ConnectionState = STATE_DISCONNECT;
				sta_rec->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
				sta_rec->BssIndex = bss->ucBssIndex;
				sta_rec->ConnectionType = CONNECTION_INFRA_BC;
				sta_rec->WlanIdx = bss->bmc_wlan_idx;
				sta_rec->priv = tr_entry;
			}
		}
		/*update to hwctrl for hw seting*/
		wsys->wdev = wdev;
		/*notify other module will leave bss*/
		call_wsys_notifieriers(WSYS_NOTIFY_LINKDOWN, wdev, NULL);
		HW_WIFISYS_LINKDOWN(ad, wsys);
	} else if (ad->wf_lock_op & WDEV_LOCK_OP_LINK_UP_DOWN) {
		/* add to handle wifi_sys operation race condition */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not enqueue!! bss_state=%d\n", 
		__func__, WDEV_BSS_STATE(wdev)));
		if (ad->wf_link_lock_flag) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): release wf_link_lock.\n", __func__, wdev->wdev_idx));
			ad->wf_link_lock_flag = FALSE;
			ad->wf_link_timestamp = 0;
			RTMP_SEM_EVENT_UP(&ad->wf_link_lock);
		}
	}

	if (wsys)
		os_free_mem(wsys);

	return TRUE;
}

VOID wifi_mlme_ops_register(struct wifi_dev *wdev)
{
	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT
	case WDEV_TYPE_AP:
		ap_fsm_ops_hook(wdev);
		break;
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_STA:
		sta_fsm_ops_hook(wdev);
		break;
#ifdef MAC_REPEATER_SUPPORT
case WDEV_TYPE_REPEATER:
		sta_fsm_ops_hook(wdev);
		break;
#endif /* MAC_REPEATER_SUPPORT */
#endif /*CONFIG_STA_SUPPORT*/
	}

}

/*
*
*/
VOID WifiSysUpdatePortSecur(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, ASIC_SEC_INFO *asic_sec_info)
{
	struct WIFI_SYS_CTRL wsys;
	struct wifi_dev *wdev = pEntry->wdev;
	struct _STA_REC_CTRL_T *sta_ctrl = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx];
	struct _STA_REC_CTRL_T *org = &tr_entry->StaRec;

	if (org->ConnectionState) {
		os_zero_mem(&wsys, sizeof(wsys));
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = STATE_PORT_SECURE;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->IsNewSTARec = FALSE;
		sta_ctrl->priv = tr_entry;
		if (asic_sec_info) {
			sta_ctrl->EnableFeature |= STA_REC_INSTALL_KEY_FEATURE;
			NdisMoveMemory(&sta_ctrl->asic_sec_info, asic_sec_info, sizeof(ASIC_SEC_INFO));
		}

		wsys.wdev = wdev;
		HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
#ifdef CONFIG_AP_SUPPORT
		CheckBMCPortSecured(pAd, pEntry, TRUE);
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_AS_WDS_STA_SUPPORT
	if (IS_ENTRY_PEER_AP(pEntry)) {
		pEntry->bEnable4Addr = TRUE;
			if (wdev->wds_enable)
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
	}
#endif /* APCLI_AS_WDS_STA_SUPPORT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (IS_ENTRY_CLIENT(pEntry)) {
		pEntry->bEnable4Addr = TRUE;
		if (wdev->wds_enable)
			HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
		else if (MAC_ADDR_EQUAL(pAd->ApCfg.wds_mac, pEntry->Addr))
			HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
		}
#endif
#ifdef HOSTAPD_MAP_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry)) {
			BOOLEAN map_a4_peer_en = FALSE;
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif /* MWDS */
#if defined(CONFIG_MAP_SUPPORT)
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MAP_ENABLE\n"));
#if defined(A4_CONN)
			map_a4_peer_en = map_a4_peer_enable(pAd, pEntry, TRUE);
#endif /* A4_CONN */
			/*comment:map_send_bh_sta_wps_done_event(pAd, pEntry, TRUE);*/
#endif /* CONFIG_MAP_SUPPORT */
#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif /* WAPP_SUPPORT */
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT) */
		}
#endif /* HOSTAPD_MAP_SUPPORT */

	}
}

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
/*
*
*/
VOID WifiSysRaInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct WIFI_SYS_CTRL wsys;
	struct _STA_REC_CTRL_T *sta_rec = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx];

	os_zero_mem(&wsys, sizeof(wsys));
	sta_rec->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_rec->WlanIdx = pEntry->wcid;
	sta_rec->ConnectionType = pEntry->ConnectionType;
	sta_rec->ConnectionState = STATE_CONNECTED;
	sta_rec->EnableFeature = STA_REC_RA_FEATURE;
	sta_rec->priv = tr_entry;
	wsys.wdev = pEntry->wdev;
	HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
}


/*
*
*/
VOID WifiSysUpdateRa(RTMP_ADAPTER *pAd,
					 MAC_TABLE_ENTRY *pEntry,
					 P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam
					)
{
	struct WIFI_SYS_CTRL wsys;
	struct _STA_REC_CTRL_T *sta_rec = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx];
	CMD_STAREC_AUTO_RATE_UPDATE_T *ra_parm = NULL;

	os_zero_mem(&wsys, sizeof(wsys));
	sta_rec->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_rec->WlanIdx = pEntry->wcid;
	sta_rec->ConnectionType = pEntry->ConnectionType;
	sta_rec->ConnectionState = STATE_CONNECTED;
	sta_rec->EnableFeature = STA_REC_RA_UPDATE_FEATURE;
	sta_rec->priv = tr_entry;
	os_alloc_mem(NULL, (UCHAR **)&ra_parm, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	if (!ra_parm) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): mem alloc ra_parm failed\n", __func__));
		return;
	}
	os_move_mem(ra_parm, prParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	wsys.priv = (VOID *)ra_parm;
	wsys.wdev = pEntry->wdev;
	HW_WIFISYS_RA_UPDATE(pAd, &wsys);
}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/



VOID wifi_sys_update_wds(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct WIFI_SYS_CTRL wsys;
	struct wifi_dev *wdev = pEntry->wdev;
	struct _STA_REC_CTRL_T *sta_ctrl = &wsys.StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = &pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx];
	struct _STA_REC_CTRL_T *org = &tr_entry->StaRec;
	UINT32 features = 0;

	if (org->ConnectionState) {
		os_zero_mem(&wsys, sizeof(wsys));
		starec_feature_decision(wdev, pEntry->ConnectionType, pEntry, &features);
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = org->ConnectionState;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->EnableFeature =  features;
		sta_ctrl->IsNewSTARec = FALSE;
		sta_ctrl->priv = tr_entry;
		wsys.wdev = wdev;
		HW_WIFISYS_PEER_UPDATE(pAd, &wsys);
	}
}

