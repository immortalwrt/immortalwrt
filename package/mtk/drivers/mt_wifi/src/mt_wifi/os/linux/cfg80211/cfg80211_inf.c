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
	cfg80211_inf.c

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	YF Luo		06-28-2012		Init version
			12-26-2013		Integration of NXTC
*/
#define RTMP_MODULE_OS

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
extern struct wifi_dev_ops ap_wdev_ops;

VOID RTMP_CFG80211_AddVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pNewVifDev, sizeof(CFG80211_VIF_DEV));

	if (pNewVifDev) {
		os_zero_mem(pNewVifDev, sizeof(CFG80211_VIF_DEV));
		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		os_zero_mem(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->ucfgIfName, pNewNetDev->name, sizeof(pNewNetDev->name));

		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (RT_LIST_ENTRY *)pNewVifDev);
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType);
	} else
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Error in alloc mem in New CFG80211 VIF Function.\n");
}

static
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByName(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {

		if (!NdisCmpMemory(pDevEntry->net_dev->name, pNewNetDev->name, sizeof(pDevEntry->net_dev->name)))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

VOID RTMP_CFG80211_RemoveVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)RTMP_CFG80211_FindVifEntry_ByName(pAd, pNewNetDev);

	if (pListEntry) {
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(pListEntry);
		pListEntry = NULL;
	} else
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error in RTMP_CFG80211_RemoveVifEntry.\n");
}
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */

#if (KERNEL_VERSION(2, 6, 28) <= LINUX_VERSION_CODE)
#ifdef RT_CFG80211_SUPPORT
extern INT apcli_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, VOID *pData)
{
	PCFG80211_CTRL pCfg80211_ctrl = NULL;
	UINT newType, oldType;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM *pVifParm;

	pVifParm = (CMD_RTPRIV_IOCTL_80211_VIF_PARM *)pData;
	pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	newType = pVifParm->newIfType;
	oldType = pVifParm->oldIfType;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE

	/* After P2P NEGO phase, the device type may be change from GC to GO */
	/*  or no change. We remove the GC in VIF list if nego as GO case. */
	if ((newType == RT_CMD_80211_IFTYPE_P2P_GO) &&
		(oldType == RT_CMD_80211_IFTYPE_P2P_CLIENT))
		RTMP_CFG80211_VirtualIF_CancelP2pClient(pAd);

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	CFG80211DBG(DBG_LVL_INFO, ("80211> @@@ Change from %u  to %u Mode\n", oldType, newType));
	pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_DISABLE;

	if (newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_CLI_UP;
	else if (newType == RT_CMD_80211_IFTYPE_P2P_GO)
		pCfg80211_ctrl->P2POpStatusFlags = CFG_P2P_GO_UP;

#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
	/* Change Device Type */
#ifdef CONFIG_STA_SUPPORT

	if (newType == RT_CMD_80211_IFTYPE_ADHOC) {
#ifdef DOT11_N_SUPPORT
		SetCommonHtVht(pAd, &pAd->StaCfg[0].wdev);
#endif /* DOT11_N_SUPPORT */
		pAd->StaCfg[0].BssType = BSS_ADHOC;
	} else
#endif /* CONFIG_STA_SUPPORT */
		if ((newType == RT_CMD_80211_IFTYPE_STATION) ||
			(newType == RT_CMD_80211_IFTYPE_P2P_CLIENT)) {
			CFG80211DBG(DBG_LVL_INFO, ("80211> Change the Interface to STA Mode\n"));
#ifdef CONFIG_STA_SUPPORT

			if ((oldType == RT_CMD_80211_IFTYPE_ADHOC) &&
				(newType == RT_CMD_80211_IFTYPE_STATION)) {
				/* DeviceType Change from adhoc to infra, only in StaCfg. */
				/* CFG Todo: It should not bind by device.*/
				pAd->StaCfg[0].BssType = BSS_INFRA;
			}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
				CFG80211DRV_DisableApInterface(pAd);

#endif /* CONFIG_AP_SUPPORT */
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;
		} else if ((newType == RT_CMD_80211_IFTYPE_AP) ||
				   (newType == RT_CMD_80211_IFTYPE_P2P_GO)) {
			CFG80211DBG(DBG_LVL_INFO, ("80211> Change the Interface to AP Mode\n"));
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		}

#ifdef CONFIG_STA_SUPPORT
		else if (newType == RT_CMD_80211_IFTYPE_MONITOR) {
			/* set packet filter */
			Set_NetworkType_Proc(pAd, "Monitor");

			if (pVifParm->MonFilterFlag != 0) {
				UINT32 Filter = 0;
#ifndef MT_MAC

				if (!IS_HIF_TYPE(pAd, HIF_MT)) {
					RTMP_IO_READ32(pAd->hdev_ctrl, RX_FILTR_CFG, &Filter);

					if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_FCSFAIL) == RT_CMD_80211_FILTER_FCSFAIL)
						Filter = Filter & (~0x01);
					else
						Filter = Filter | 0x01;

					if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_PLCPFAIL) == RT_CMD_80211_FILTER_PLCPFAIL)
						Filter = Filter & (~0x02);
					else
						Filter = Filter | 0x02;

					if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_CONTROL) == RT_CMD_80211_FILTER_CONTROL)
						Filter = Filter & (~0xFF00);
					else
						Filter = Filter | 0xFF00;

					if ((pVifParm->MonFilterFlag & RT_CMD_80211_FILTER_OTHER_BSS) == RT_CMD_80211_FILTER_OTHER_BSS)
						Filter = Filter & (~0x08);
					else
						Filter = Filter | 0x08;

					RTMP_IO_WRITE32(pAd->hdev_ctrl, RX_FILTR_CFG, Filter);
				}

#endif /* MT_MAC */
				pVifParm->MonFilterFlag = Filter;
			}
		}

#endif /*CONFIG_STA_SUPPORT*/

	if ((newType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
		(newType == RT_CMD_80211_IFTYPE_P2P_GO))
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pVifParm->net_dev->dev_addr);
	else {
#ifdef RT_CFG80211_P2P_SUPPORT
		pCfg80211_ctrl->bP2pCliPmEnable = FALSE;
		pCfg80211_ctrl->bPreKeepSlient = FALSE;
		pCfg80211_ctrl->bKeepSlient = FALSE;
		pCfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
		pCfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
		pCfg80211_ctrl->CTWindows = 0;  /* CTWindows and OppPS parameter field */
#endif /* RT_CFG80211_P2P_SUPPORT */
	}

	return TRUE;
}

#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByName(VOID *pAdSrc, CHAR ucIfName[])
{
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

		PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
		PCFG80211_VIF_DEV pDevEntry = NULL;
		RT_LIST_ENTRY *pListEntry = NULL;

		pListEntry = pCacheList->pHead;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

		while (pDevEntry != NULL) {
			if (!NdisCmpMemory(pDevEntry->ucfgIfName, ucIfName, sizeof(pDevEntry->ucfgIfName)))
				return pDevEntry->net_dev->ieee80211_ptr;

			pListEntry = pListEntry->pNext;
			pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
		}
		return NULL;
}

BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;
	INT32 MaxNumBss = 0;
	ULONG flags = 0;
	UINT32 max_num_sta = pAd->MaxMSTANum;

	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	switch (pVifInfo->vifType) {
	case RT_CMD_80211_IFTYPE_AP:
		MaxNumBss = pAd->ApCfg.BssidNum;
		if (!VALID_MBSS(pAd, MaxNumBss))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		if (pAd->CfgAPIfUseCnt >= (MaxNumBss - 1)) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Error useCut %d > supportNum %d\n", pAd->CfgAPIfUseCnt, MaxNumBss);
			return FALSE;
		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
		pAd->CfgAPIfUseCnt++;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
		break;
	case RT_CMD_80211_IFTYPE_AP_VLAN:
		break;
	case RT_CMD_80211_IFTYPE_STATION:
		max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
		if (pAd->CfgSTAIfUseCnt >= max_num_sta) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Error useCut %d > supportNum %d\n", pAd->CfgSTAIfUseCnt, max_num_sta);
			return FALSE;
		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
		pAd->CfgSTAIfUseCnt++;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
		break;
	default:
		break;
	}
	RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType, pVifInfo->flags);
	return TRUE;
}

NET_DEV_STATS *RT28xx_get_ether_stats(PNET_DEV net_dev);

VOID RTMP_CFG80211_VirtualIF_Init(
	IN VOID		*pAdSrc,
	IN CHAR * pDevName,
	IN UINT32                DevType,
	IN UINT32		flags)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook;
	PNET_DEV	new_dev_p;
	struct wifi_dev *wdev;
	UINT apidx = MAIN_MBSSID;
	CHAR preIfName[IFNAMSIZ];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = 1;
	INT32 IdBss, MaxNumBss;
	UINT32 sta_start_id = (MAIN_MSTA_ID + 1);
	UINT32 max_num_sta = pAd->MaxMSTANum;
	UINT32 inf_type = INT_MSTA;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 MC_RowID = 0, IoctlIF = 0;
	INT32 Ret;


	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"---> (%s, %s, %d)\n", pDevName, preIfName, preIfIndex);
	/* init operation functions and flags */
	switch (DevType) {

	case RT_CMD_80211_IFTYPE_AP:
			/* max bss number && avai apiadx */
			MaxNumBss = pAd->ApCfg.BssidNum;
			if (!VALID_MBSS(pAd, MaxNumBss))
				MaxNumBss = MAX_MBSSID_NUM(pAd);

			for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
				if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev == NULL) {
					preIfIndex = IdBss;
					break;
				}
			}
			pAd->ApCfg.MBSSID[preIfIndex].wdev.if_dev = NULL;
			pAd->ApCfg.MBSSID[preIfIndex].wdev.bcn_buf.BeaconPkt = NULL;
			new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MBSSID, preIfIndex,
							sizeof(struct mt_dev_priv), preIfName, TRUE);
			if (new_dev_p == NULL) {
				/* allocation fail, exit */
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Allocate network device fail (CFG80211)...\n");
				return;
			}


			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
			apidx = preIfIndex;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, new_dev_p, apidx,
							(VOID *)&pAd->ApCfg.MBSSID[apidx], (void *)pAd);

			if (!Ret) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Assign wdev idx for %s failed, free net device!\n",
						 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				return;
			}

			Ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
									cap->qos.wmm_detect_method);

			if (!Ret) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "register wdev_ops %s failed, free net device!\n",
						  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				return;
			}

			RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
			RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
			/* init operation functions and flags */
			NdisZeroMemory(&netDevHook, sizeof(netDevHook));
			netDevHook.open = mbss_virtual_if_open;	/* device opem hook point */
			netDevHook.stop = mbss_virtual_if_close;	/* device close hook point */
			netDevHook.xmit = rt28xx_send_packets;	/* hard transmit hook point */
			netDevHook.ioctl = rt28xx_ioctl;	/* ioctl hook point */
			netDevHook.priv_flags = INT_MBSSID;
			netDevHook.needProtcted = TRUE;
			netDevHook.wdev = wdev;
			netDevHook.get_stats = RT28xx_get_ether_stats;
			/* Init MAC address of virtual network interface */
			NdisMoveMemory(&netDevHook.devAddr[0], &wdev->bssid[0], MAC_ADDR_LEN);

#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				if (wdev && wdev->wdev_type == WDEV_TYPE_AP)
					map_make_vend_ie(pAd, apidx);
			}
#endif /* CONFIG_MAP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_AP;
			os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
			os_zero_mem((PUCHAR)pWdev, sizeof(*pWdev));
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = DevType;
#endif /* RT_CFG80211_SUPPORT */
			/* register this device to OS */
			if (RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, &netDevHook) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s, create IF %d (%s) failed!!\n",
						 __func__, apidx, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			}
			RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
		break;
#ifdef CONFIG_VLAN_GTK_SUPPORT
	case RT_CMD_80211_IFTYPE_AP_VLAN:
		/* max bss number && avai apiadx */
		MaxNumBss = pAd->ApCfg.BssidNum;
		if (!VALID_MBSS(pAd, MaxNumBss))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		new_dev_p = alloc_netdev(sizeof(struct vlan_dev_priv), preIfName, 0, vlan_setup);
		if (!new_dev_p) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Allocate network device fail (CFG80211)...\n");
			return;
		}

		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		wdev = CFG80211_GetWdevByVlandev(pAd, new_dev_p);
		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s(): can't find a matched wdev for AP_VLAN net_dev!\n", __func__);
			return;
		}

		if (wdev->vlan_cnt >= MAX_VLAN_NET_DEVICE) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s(): can't create AP_VLAN net_dev, only support for %d vlan! \n",
					 __func__, MAX_VLAN_NET_DEVICE);
			return;
		} else {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB;
			char *pch;
			UINT16 vlan_id;
			UINT16 vlan_bmc_idx;
			struct vlan_dev_priv *vlan;
			struct vlan_gtk_info *vg_info;

			p80211CB = pAd->pCfg80211_CB;
			pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = RT_CMD_80211_IFTYPE_AP_VLAN;

			/* acquire bmc wcid for vlan_dev */
			vlan_bmc_idx = HcAcquireGroupKeyWcid(pAd, wdev);
#ifdef SW_CONNECT_SUPPORT
			if (wdev->hw_bmc_wcid != WCID_INVALID)
				vlan_bmc_idx = wdev->hw_bmc_wcid;
#endif /* SW_CONNECT_SUPPORT */
			TRTableInsertMcastEntry(pAd, vlan_bmc_idx, wdev);
			MgmtTableSetMcastEntry(pAd, vlan_bmc_idx);

			/* init sta_rec and tr_entry for vlan bmc wtbl */
			wifi_vlan_starec_linkup(wdev, vlan_bmc_idx);

			/* parse vlan id from interface name */
			pch = strchr(new_dev_p->name, '.') + 1;
			vlan_id = simple_strtol(pch, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s(): Create AP_VLAN net_dev name=%s vlan_id=%d bmc_idx=%d\n",
					 __func__,  new_dev_p->name,
					 vlan_id, vlan_bmc_idx);

			/* register vlan device */
			new_dev_p->mtu = wdev->if_dev->mtu;
			vlan = vlan_dev_priv(new_dev_p);
			vlan->vlan_proto = htons(ETH_P_8021Q);
			vlan->vlan_id = vlan_id;
			vlan->real_dev = wdev->if_dev;
			vlan->dent = NULL;
			vlan->flags = VLAN_FLAG_REORDER_HDR;
			register_vlan_dev(new_dev_p, NULL);

			/* add the vlan_gtk_info to list*/
			vg_info = kzalloc(sizeof(struct vlan_gtk_info), GFP_KERNEL);
			if (!vg_info) {
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s(): Create vlan_gtk_info failed!\n", __func__);
				return;
			}
			vg_info->vlan_dev = new_dev_p;
			vg_info->vlan_id = vlan_id;
#ifdef SW_CONNECT_SUPPORT
			vg_info->vlan_bmc_idx = wdev->hw_bmc_wcid;
#else /* SW_CONNECT_SUPPORT */
			vg_info->vlan_bmc_idx = vlan_bmc_idx;
#endif /* !SW_CONNECT_SUPPORT */
			vg_info->vlan_tr_tb_idx = vlan_bmc_idx;
			list_add_tail(&vg_info->list, &wdev->vlan_gtk_list);

#ifdef VLAN_SUPPORT
			/* prevent vlan_tag from being removed by ap_fp_tx_pkt_vlan_tag_handle() */
			wdev->bVLAN_Tag = TRUE;
#endif

			wdev->vlan_cnt++;
			RTMP_CFG80211_AddVifEntry(pAd, vg_info->vlan_dev, DevType);
		}
		break;
#endif
	case RT_CMD_80211_IFTYPE_STATION:
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			sta_start_id = 0;
			max_num_sta = min(pAd->ApCfg.ApCliNum, (UCHAR)MAX_APCLI_NUM);
			inf_type = INT_APCLI;

			for (IdBss = 0; IdBss < max_num_sta; IdBss++) {
				if (pAd->StaCfg[IdBss].wdev.if_dev == NULL) {
					preIfIndex = IdBss;
					break;
				}
			}
			pAd->StaCfg[preIfIndex].wdev.if_dev = NULL;
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, inf_type,
					preIfIndex, sizeof(struct mt_dev_priv), preIfName, TRUE);

#ifdef DOT11_SAE_SUPPORT
		pAd->StaCfg->sae_cfg_group = SAE_DEFAULT_GROUP;
#endif
#ifdef CONFIG_OWE_SUPPORT
		pAd->StaCfg->curr_owe_group = ECDH_GROUP_256;
#endif
		wdev = &pAd->StaCfg[preIfIndex].wdev;
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, new_dev_p, preIfIndex,
						(VOID *)&pAd->StaCfg[preIfIndex], (VOID *)pAd);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_STA, &apcli_wdev_ops,
								cap->qos.wmm_detect_method);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "register wdev_ops %s failed, free net device!\n"
						, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		/* init operation functions and flags */

		NdisZeroMemory(&netDevHook, sizeof(netDevHook));
		netDevHook.open = msta_virtual_if_open;  /* device opem hook point */
		netDevHook.stop = msta_virtual_if_close; /* device close hook point */
		netDevHook.xmit = rt28xx_send_packets;	/* hard transmit hook point */
		netDevHook.ioctl = rt28xx_ioctl;	/* ioctl hook point */
		netDevHook.priv_flags = INT_APCLI;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;
		netDevHook.get_stats = RT28xx_get_ether_stats;
		/* Init MAC address of virtual network interface */
		COPY_MAC_ADDR(&pAd->StaCfg[preIfIndex].wdev.if_addr, pAd->CurrentAddress);
		AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		NdisMoveMemory(&netDevHook.devAddr[0], &wdev->if_dev, MAC_ADDR_LEN);
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if ((IF_COMBO_HAVE_AP_STA(pAd))) {
			apcli_sync_wdev(pAd, wdev);
			/*update rate info*/
			SetCommonHtVht(pAd, wdev);
			RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
			AsicSetWdevIfAddr(pAd, wdev, OPMODE_STA);
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
		NdisMoveMemory(&netDevHook.devAddr[0], pAd->StaCfg[preIfIndex].wdev.if_addr, MAC_ADDR_LEN);

		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_STATION;

			os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
			os_zero_mem((PUCHAR)pWdev, sizeof(*pWdev));
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = DevType;

			if (flags && WIPHY_FLAG_4ADDR_STATION)
				pWdev->use_4addr = true;
		}
		/* register this device to OS */
		if (RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, &netDevHook) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s, create IF %d (%s) failed!!\n",
						 __func__, apidx, RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
		}

		RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
		pAd->StaCfg->ApcliInfStat.ApCliInit = TRUE;
#ifdef MAC_REPEATER_SUPPORT
		CliLinkMapInit(pAd);
#endif
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown CFG80211 I/F Type (%d)\n", DevType);
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<---\n");
}

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN  VOID				 *pAdSrc,
	IN	PNET_DEV			  dev_p,
	IN  UINT32                DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	struct wifi_dev *wdev;
	INT32 apidx = 0, staidx = 0;
	ULONG flags = 0;

	switch (DevType) {
	case RT_CMD_80211_IFTYPE_AP:
		apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, dev_p);
		if (apidx == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev in driver MBSS.\n");
			return;
		}
		if (dev_p) {
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
			RtmpOSNetDevDetach(dev_p);
			wdev_deinit(pAd, wdev);
			wdev->if_dev = NULL;
			kfree(dev_p->ieee80211_ptr);
			dev_p->ieee80211_ptr = NULL;
		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
		pAd->CfgAPIfUseCnt--;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
		break;
#ifdef CONFIG_VLAN_GTK_SUPPORT
	case RT_CMD_80211_IFTYPE_AP_VLAN:
		if (dev_p) {
			struct vlan_gtk_info *vg_info;

			wdev = CFG80211_GetWdevByVlandev(pAd, dev_p);
			vg_info = CFG80211_GetVlanInfoByVlandev(wdev, dev_p);
			if (vg_info) {
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s(): Remove AP_VLAN net_dev name=%s vlan_id=%d bmc_idx=%d\n",
						 __func__, dev_p->name, vg_info->vlan_id, vg_info->vlan_bmc_idx);
				HcReleaseGroupKeyWcid(pAd, wdev, vg_info->vlan_bmc_idx);
				list_del(&vg_info->list);
				kfree(vg_info);
			}
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
			unregister_vlan_dev(dev_p, NULL);
			if (dev_p->ieee80211_ptr) {
				kfree(dev_p->ieee80211_ptr);
				dev_p->ieee80211_ptr = NULL;
			}
			wdev->vlan_cnt--;
		}
		break;
#endif
	case RT_CMD_80211_IFTYPE_STATION:
		staidx = CFG80211_FindStaIdxByNetDevice(pAd, dev_p);
		if (staidx == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev in driver STA. \n");
			return;
		}

		if (dev_p) {
			RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

			wdev = &pAd->StaCfg[staidx].wdev;
			RtmpOSNetDevDetach(dev_p);
			wdev_deinit(pAd, wdev);
			wdev->if_dev = NULL;
			kfree(dev_p->ieee80211_ptr);
			dev_p->ieee80211_ptr = NULL;
		}
		OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock, &flags);
		pAd->CfgSTAIfUseCnt--;
		OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock, &flags);
		break;
	default:
		break;
	}

}
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */



#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_VIF_SET *pVifInfo;

	pVifInfo = (CMD_RTPRIV_IOCTL_80211_VIF_SET *)pData;

	/* Already one VIF in list */
	if (pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn)
		return FALSE;

	pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = TRUE;
	RTMP_CFG80211_VirtualIF_Init(pAd, pVifInfo->vifName, pVifInfo->vifType, pVifInfo->flags);
	return TRUE;
}

BOOLEAN RTMP_CFG80211_VIF_ON(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	return pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn;
}



static
PCFG80211_VIF_DEV RTMP_CFG80211_FindVifEntry_ByMac(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			return pDevEntry;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == devType)
			return pDevEntry->net_dev->ieee80211_ptr;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return NULL;
}

VOID RTMP_CFG80211_AddVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev, UINT32 DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_VIF_DEV pNewVifDev = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pNewVifDev, sizeof(CFG80211_VIF_DEV));

	if (pNewVifDev) {
		os_zero_mem(pNewVifDev, sizeof(CFG80211_VIF_DEV));
		pNewVifDev->pNext = NULL;
		pNewVifDev->net_dev = pNewNetDev;
		pNewVifDev->devType = DevType;
		os_zero_mem(pNewVifDev->CUR_MAC, MAC_ADDR_LEN);
		NdisCopyMemory(pNewVifDev->CUR_MAC, pNewNetDev->dev_addr, MAC_ADDR_LEN);
		insertTailList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, (RT_LIST_ENTRY *)pNewVifDev);
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Add CFG80211 VIF Device, Type: %d.\n", pNewVifDev->devType);
	} else
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error in alloc mem in New CFG80211 VIF Function.\n");
}

VOID RTMP_CFG80211_RemoveVifEntry(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)RTMP_CFG80211_FindVifEntry_ByMac(pAd, pNewNetDev);

	if (pListEntry) {
		delEntryList(&pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList, pListEntry);
		os_free_mem(pListEntry);
		pListEntry = NULL;
	} else
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error in RTMP_CFG80211_RemoveVifEntry.\n");
}

PNET_DEV RTMP_CFG80211_VirtualIF_Get(
	IN VOID                 *pAdSrc
)
{
	/* PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc; */
	/* return pAd->Cfg80211VifDevSet.Cfg80211VifDev[0].net_dev; */
	return NULL;
}

static INT CFG80211_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAdSrc;
	PRTMP_ADAPTER pAd;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %d,%s\n", dev_p->ifindex,
			 RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));
	/* if (VIRTUAL_IF_UP(pAd, dev_p) != 0) */
	/* return -1; */
	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(dev_p);

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
#ifdef CFG80211_MULTI_STA
		|| (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */
	   ) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCli_Open\n");
		pAd->flg_apcli_init = TRUE;
		ApCli_Open(pAd, dev_p);
		return 0;
	}

	RTMP_OS_NETDEV_START_QUEUE(dev_p);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=== %s\n", RTMP_OS_NETDEV_GET_DEVNAME(dev_p));
	return 0;
}

static INT CFG80211_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAdSrc;
	PRTMP_ADAPTER pAd;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAdSrc);
	pAd = (PRTMP_ADAPTER)pAdSrc;

	if ((dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
#ifdef CFG80211_MULTI_STA
		|| (dev_p->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION)
#endif /* CFG80211_MULTI_STA */
	   ) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCli_Close\n");
		CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
		RT_MOD_HNAT_DEREG(dev_p);
		RT_MOD_DEC_USE_COUNT();
		return ApCli_Close(pAd, dev_p);
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> %s\n", RTMP_OS_NETDEV_GET_DEVNAME(dev_p));
	RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

	if (netif_carrier_ok(dev_p))
		netif_carrier_off(dev_p);

	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(
			pAd,
			pAd->CommonCfg.BeaconPeriod,
			HW_BSSID_0,
			OPMODE_ADHOC);
	else
		AsicDisableSync(pAd, HW_BSSID_0);

	/* VIRTUAL_IF_DOWN(pAd); */
	RT_MOD_HNAT_DEREG(dev_p);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}

static INT CFG80211_PacketSend(PNDIS_PACKET pPktSrc, PNET_DEV pDev, RTMP_NET_PACKET_TRANSMIT Func)
{
	PRTMP_ADAPTER pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);

	/* To Indicate from Which VIF */
	switch (pDev->ieee80211_ptr->iftype) {
	case RT_CMD_80211_IFTYPE_AP:
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_GO:
		;

		if (!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Drop the Packet due P2P GO not in ready state\n");
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			return 0;
		}

		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	case RT_CMD_80211_IFTYPE_P2P_CLIENT:
	case RT_CMD_80211_IFTYPE_STATION:
		RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Unknown CFG80211 I/F Type(%d)\n", pDev->ieee80211_ptr->iftype);
		RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
		return 0;
	}

	MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "CFG80211 Packet Type  [%s](%d)\n",
			 pDev->name, pDev->ieee80211_ptr->iftype);
	return Func(RTPKT_TO_OSPKT(pPktSrc));
}

static INT CFG80211_VirtualIF_PacketSend(
	struct sk_buff *skb, PNET_DEV dev_p)
{
	struct wifi_dev *wdev;

	MTWF_DBG(NULL, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s ---> %d\n", __func__, dev_p->ieee80211_ptr->iftype);

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(dev_p))) {
		/* the interface is down */
		RELEASE_NDIS_PACKET(NULL, skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	/* The device not ready to send packt. */
	wdev = RTMP_OS_NETDEV_GET_WDEV(dev_p);
	ASSERT(wdev);

	if (!wdev)
		return -1;

	os_zero_mem((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);
	return CFG80211_PacketSend(skb, dev_p, rt28xx_packet_xmit);
}

static INT CFG80211_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p,
	IN OUT VOID			*rq_p,
	IN INT					cmd)
{
	RTMP_ADAPTER *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--->\n");
	return rt28xx_ioctl(dev_p, rq_p, cmd);
}

static struct wifi_dev_ops gc_wdev_ops  = {
	.tx_pkt_allowed = sta_tx_pkt_allowed,
	.fp_tx_pkt_allowed = sta_tx_pkt_allowed,
	.send_data_pkt = sta_send_data_pkt,
	.send_fp_data_pkt = sta_fp_send_data_pkt,
	.send_mlme_pkt = sta_send_mlme_pkt,
	.tx_pkt_handle = sta_tx_pkt_handle,
	.legacy_tx = sta_legacy_tx,
	.ampdu_tx = sta_ampdu_tx,
	.frag_tx = sta_frag_tx,
	.amsdu_tx = sta_amsdu_tx,
	.mlme_mgmtq_tx = sta_mlme_mgmtq_tx,
	.mlme_dataq_tx = sta_mlme_dataq_tx,
	.ieee_802_11_data_tx = sta_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = sta_ieee_802_3_data_tx,
	.fill_non_offload_tx_blk = sta_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = sta_fill_offload_tx_blk,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = sta_find_cipher_algorithm,
	.mac_entry_lookup = sta_mac_entry_lookup,
	.media_state_connected = sta_media_state_connected,
	.ioctl = rt28xx_sta_ioctl,
	.open = wifi_sys_open,
	.close = wifi_sys_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};

struct wifi_dev_ops go_wdev_ops = {
	.tx_pkt_allowed = ap_tx_pkt_allowed,
	.fp_tx_pkt_allowed = ap_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.fill_non_offload_tx_blk = ap_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = ap_fill_offload_tx_blk,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_ps_handle = ap_rx_ps_handle,
	.rx_pkt_foward = ap_rx_pkt_foward,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
	.media_state_connected = media_state_connected,
	.ioctl = rt28xx_ap_ioctl,
	.open = wifi_sys_open,
	.close = wifi_sys_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};

VOID RTMP_CFG80211_VirtualIF_Init(
	IN VOID		*pAdSrc,
	IN CHAR * pDevName,
	IN UINT32		DevType,
	IN UINT32		flags)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV	new_dev_p;
	STA_ADMIN_CONFIG	*pApCliEntry;
	struct wifi_dev *wdev;
    struct wifi_dev *wdev_main = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = MAIN_MBSSID;
    UINT32 Inf = INT_MBSSID;
#endif /*RT_CFG80211_P2P_SUPPORT*/
#ifdef MT_MAC
	INT32 Value;
	UCHAR MacByte = 0;
#endif /* MT_MAC */
	UINT16 tr_tb_idx = wdev_main->tr_tb_idx;
	CHAR preIfName[12];
	UINT devNameLen = strlen(pDevName);
	UINT preIfIndex = pDevName[devNameLen - 1] - 48;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	struct wireless_dev *pWdev;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	memset(preIfName, 0, sizeof(preIfName));
	NdisCopyMemory(preIfName, pDevName, devNameLen - 1);
	pNetDevOps = &netDevHook;
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "---> (%s, %s, %d)\n", pDevName, preIfName, preIfIndex);
	/* init operation functions and flags */
	os_zero_mem(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_VirtualIF_Open;	     /* device opem hook point */
	netDevHook.stop = CFG80211_VirtualIF_Close;	     /* device close hook point */
	netDevHook.xmit = CFG80211_VirtualIF_PacketSend; /* hard transmit hook point */
	netDevHook.ioctl = CFG80211_VirtualIF_Ioctl;	 /* ioctl hook point */
#if WIRELESS_EXT >= 12
	/* netDevHook.iw_handler = (void *)&rt28xx_ap_iw_handler_def; */
#endif /* WIRELESS_EXT >= 12 */
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), preIfName, TRUE);

	if (new_dev_p == NULL) {
		/* allocation fail, exit */
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate network device fail (CFG80211)...\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));

	new_dev_p->destructor =  free_netdev;
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	pNetDevOps->needProtcted = TRUE;
	os_move_mem(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
#ifdef MT_MAC
	/* TODO: shall we make choosing which byte to be selectable??? */
	Value = 0x00000000L;
	RTMP_IO_READ32(pAd->hdev_ctrl, LPON_BTEIR, &Value);/* read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address. */
	Value = Value | (0x2 << 29);/* Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, LPON_BTEIR, Value);
	MacByte = Value >> 29;
	pNetDevOps->devAddr[0] |= 0x2;

	switch (MacByte) {
	case 0x1: /* choose bit[23:20]*/
		pNetDevOps->devAddr[2] = (pNetDevOps->devAddr[2] = pNetDevOps->devAddr[2] & 0x0f);
		break;

	case 0x2: /* choose bit[31:28]*/
		pNetDevOps->devAddr[3] = (pNetDevOps->devAddr[3] = pNetDevOps->devAddr[3] & 0x0f);
		break;

	case 0x3: /* choose bit[39:36]*/
		pNetDevOps->devAddr[4] = (pNetDevOps->devAddr[4] = pNetDevOps->devAddr[4] & 0x0f);
		break;

	case 0x4: /* choose bit [47:44]*/
		pNetDevOps->devAddr[5] = (pNetDevOps->devAddr[5] = pNetDevOps->devAddr[5] & 0x0f);
		break;

	default: /* choose bit[15:12]*/
		pNetDevOps->devAddr[1] = (pNetDevOps->devAddr[1] = pNetDevOps->devAddr[1] & 0x0f);
		break;
	}

#else

	/* CFG_TODO */
	/*
	 *	Bit1 of MAC address Byte0 is local administration bit
	 *	and should be set to 1 in extended multiple BSSIDs'
	 *	Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	 */
	if (cap->MBSSIDMode == MBSSID_MODE1)
		pNetDevOps->devAddr[0] += 2; /* NEW BSSID */
	else {
#ifdef P2P_ODD_MAC_ADJUST

		if (pNetDevOps->devAddr[5] & 0x01 == 0x01)
			pNetDevOps->devAddr[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
			pNetDevOps->devAddr[5] += FIRST_MBSSID;
	}

#endif /* MT_MAC */

	switch (DevType) {
#ifdef CONFIG_STA_SUPPORT

	case RT_CMD_80211_IFTYPE_MONITOR:
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG80211 I/F Monitor Type\n");
		/* RTMP_OS_NETDEV_SET_TYPE_MONITOR(new_dev_p); */
		break;

	case RT_CMD_80211_IFTYPE_P2P_CLIENT:
	case RT_CMD_80211_IFTYPE_STATION:
		INT32 Ret;

		pApCliEntry = &pAd->StaCfg[MAIN_MBSSID];
		wdev = &pApCliEntry->wdev;
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_GC, new_dev_p, MAIN_MBSSID, 0, (VOID *)pApCliEntry, (void *)pAd);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_GC, &gc_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		wdev_attr_update(pAd, wdev);
		wdev->bss_info_argument.OwnMacIdx = wdev->OmacIdx;
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pNetDevOps->devAddr);
		break;
#endif /*CONFIG_STA_SUPPORT*/

	case RT_CMD_80211_IFTYPE_P2P_GO:
		/* Only ForceGO init from here, */
		/* Nego as GO init on AddBeacon Ops.*/
		pNetDevOps->priv_flags = INT_P2P;
		/* The Behivaor in SetBeacon Ops */
		pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		wdev_init(pAd, wdev, WDEV_TYPE_GO, new_dev_p, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (void *)pAd);
		wdev_ops_register(wdev, WDEV_TYPE_GO, &go_wdev_ops,
						  cap->wmm_detect_method);
		wdev_attr_update(pAd, wdev);
		wdev->bss_info_argument.OwnMacIdx = wdev->OmacIdx;
		/* BC/MC Handling */
		TRTableInsertMcastEntry(pAd, tr_tb_idx, wdev);
		/* for concurrent purpose */
		wdev->hw_bssid_idx = CFG_GO_BSSID_IDX;
		bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);

		if (wdev_idx_reg(pAd, wdev) < 0) {
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pNetDevOps->devAddr);
		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pNetDevOps->devAddr);
		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
		wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
				BSS_INFO_BASIC_FEATURE |
				BSS_INFO_RF_CH_FEATURE |
				BSS_INFO_SYNC_MODE_FEATURE);
		AsicBssInfoUpdate(pAd, wdev->bss_info_argument);
		break;

	case RT_CMD_80211_IFTYPE_AP:
			pNetDevOps->priv_flags = INT_MBSSID;

			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_AP;
			apidx = preIfIndex;
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

			/* follow main wdev settings   */
			wdev->channel = wdev_main->channel;
			wdev->CentralChannel = wdev_main->CentralChannel;
			wdev->PhyMode = wdev_main->PhyMode;
			wdev->bw = wdev_main->bw;
			wdev->extcha = wdev_main->extcha;
			/* ================= */

			wdev_init(pAd, wdev, WDEV_TYPE_AP, new_dev_p, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (void *)pAd);
			wdev_attr_update(pAd, wdev);
			wdev->bss_info_argument.OwnMacIdx = wdev->OmacIdx;

			bcn_buf_init(pAd, &pAd->ApCfg.MBSSID[apidx].wdev);

			RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
			RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
			if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR, "Assign wdev idx for %s failed, free net device!\n",
					RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
				RtmpOSNetDevFree(new_dev_p);
				break;
			}

			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pNetDevOps->devAddr);
			COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pNetDevOps->devAddr);

			wifi_sys_linkup(wdev, NULL);
			break;

	default:
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown CFG80211 I/F Type (%d)\n", DevType);
	}

	/* CFG_TODO : should be move to VIF_CHG */
	if ((DevType == RT_CMD_80211_IFTYPE_P2P_CLIENT) ||
		(DevType == RT_CMD_80211_IFTYPE_P2P_GO))
		COPY_MAC_ADDR(pAd->cfg80211_ctrl.P2PCurrentAddress, pNetDevOps->devAddr);

	os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
	os_zero_mem((PUCHAR)pWdev, sizeof(*pWdev));
	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
	pWdev->netdev = new_dev_p;
	pWdev->iftype = DevType;
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	COPY_MAC_ADDR(pWdev->address, pNetDevOps->devAddr);
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	/* Record the pNetDevice to Cfg80211VifDevList */
	RTMP_CFG80211_AddVifEntry(pAd, new_dev_p, DevType);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, " <---\n");
}

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN  VOID				 *pAdSrc,
	IN	PNET_DEV			  dev_p,
	IN  UINT32                DevType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	BOOLEAN isGoOn = FALSE;
	struct wifi_dev *wdev;
#ifdef RT_CFG80211_P2P_SUPPORT
	UINT apidx = CFG_GO_BSSID_IDX;
#else
	UINT apidx = MAIN_MBSSID;
#endif /*RT_CFG80211_P2P_SUPPORT*/

	if (dev_p) {
		pAd->cfg80211_ctrl.Cfg80211VifDevSet.isGoingOn = FALSE;
		RTMP_CFG80211_RemoveVifEntry(pAd, dev_p);
		RTMP_OS_NETDEV_STOP_QUEUE(dev_p);
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		isGoOn = RTMP_CFG80211_VIF_P2P_GO_ON(pAd);

		if (isGoOn) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
			struct wifi_dev *pwdev = &pMbss->wdev;

			if (pAd->Mlme.bStartMcc == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Group remove stop mcc\n");
				Stop_MCC(pAd, 0);
				pAd->Mlme.bStartMcc = FALSE;
			}

			if (pAd->Mlme.bStartScc == TRUE) {
				pAd->Mlme.bStartScc = FALSE;
				wlan_operate_set_prim_ch(pAd->StaCfg[0].wdev, wdev->channel);
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"GO remove & switch to Infra BW = %d  pAd->StaCfg[0].wdev.CentralChannel %d\n",
					wlan_operate_get_ht_bw(&pAd->StaCfg[0].wdev),
					wlan_operate_get_cen_ch_1(&pAd->StaCfg[0].wdev));
			}

			pwdev->channel = 0;
			pwdev->CentralChannel = 0;
			wlan_operate_set_ht_bw(pwdev, HT_BW_20, EXTCHA_NONE);
			/*after p2p cli connect , neet to change to default configure*/
			wlan_operate_set_ht_bw(wdev, HT_BW_40, EXTCHA_BELOW);
			pAd->CommonCfg.HT_Disable = 0;
			SetCommonHtVht(pAd, wdev);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
			bcn_buf_deinit(pAd, &pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf);
			RtmpOSNetDevDetach(dev_p);
			wdev_deinit(pAd, wdev);
			wdev->if_dev = NULL;
		} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
			if (pAd->flg_apcli_init) {
				wdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
				/* actually not mcc still need to check this! */

				if (pAd->Mlme.bStartMcc == TRUE) {
					struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

					MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GC remove stop mcc\n");
					cap->tssi_enable = TRUE; /* let host do tssi */
					Stop_MCC(pAd, 0);
					pAd->Mlme.bStartMcc = FALSE;
				} else
					/* if (pAd->Mlme.bStartScc == TRUE) */
				{
					struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
					UCHAR ht_bw = wlan_config_get_ht_bw(p2p_dev);
					UCHAR cen_ch;

					wlan_operate_set_ht_bw(p2p_dev, ht_bw);
					cen_ch = wlan_operate_get_cen_ch_1(p2p_dev);

					MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"GC remove & switch to Infra BW = %d  CentralChannel %d\n",
						ht_bw, cen_ch);
				}

				wdev->channel = 0;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
				OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
				cfg80211_disconnected(dev_p, 0, NULL, 0, GFP_KERNEL);
				os_zero_mem(pAd->StaCfg[MAIN_MBSSID].CfgApCliBssid, MAC_ADDR_LEN);
				RtmpOSNetDevDetach(dev_p);
				wdev_deinit(pAd, wdev);
				pAd->flg_apcli_init = FALSE;
				wdev->if_dev = NULL;
			} else /* Never Opened When New Netdevice on */
				RtmpOSNetDevDetach(dev_p);

		os_free_mem(dev_p->ieee80211_ptr);
		dev_p->ieee80211_ptr = NULL;
	}
}

VOID RTMP_CFG80211_AllVirtualIF_Remove(
	IN VOID		*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV           pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while ((pDevEntry != NULL) && (pCacheList->size != 0)) {
		RtmpOSNetDevProtect(1);
		RTMP_CFG80211_VirtualIF_Remove(pAd, pDevEntry->net_dev, pDevEntry->net_dev->ieee80211_ptr->iftype);
		RtmpOSNetDevProtect(0);
		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

#ifdef RT_CFG80211_P2P_SUPPORT
BOOLEAN RTMP_CFG80211_VIF_P2P_GO_ON(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;

	pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_GO);

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pNetDev != NULL))
		return TRUE;
	else
		return FALSE;

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE

	if (CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_GO_UP))
		return TRUE;
	else
		return FALSE;

#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
	return FALSE;
}

BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	PNET_DEV pNetDev = NULL;

	pNetDev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_P2P_CLIENT);

	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pNetDev != NULL))
		return TRUE;
	else
		return FALSE;

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE

	if (CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.P2POpStatusFlags, CFG_P2P_CLI_UP))
		return TRUE;
	else
		return FALSE;

#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
	return FALSE;
}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID RTMP_CFG80211_VirtualIF_CancelP2pClient(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV               pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==> \n");
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (pDevEntry->devType == RT_CMD_80211_IFTYPE_P2P_CLIENT) {
			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "==> RTMP_CFG80211_VirtualIF_CancelP2pClient HIT.\n");
			pDevEntry->devType = RT_CMD_80211_IFTYPE_P2P_GO;
			wdev_deinit(pAd, &pAd->StaCfg[MAIN_MBSSID].wdev);
			break;
		}

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	pAd->flg_apcli_init = FALSE;
	pAd->StaCfg[MAIN_MBSSID].wdev.if_dev = NULL;
	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<==\n");
}

static INT CFG80211_DummyP2pIf_Open(
	IN PNET_DEV		dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;

	if (!wdev)
		return -EINVAL;

	wdev->wiphy->interface_modes |= (BIT(NL80211_IFTYPE_P2P_CLIENT)
									 | BIT(NL80211_IFTYPE_P2P_GO));
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	wdev->wiphy->interface_modes |=  BIT(RT_CMD_80211_IFTYPE_P2P_DEVICE);
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	wdev->iftype = RT_CMD_80211_IFTYPE_P2P_CLIENT;
	return 0;
}

static INT CFG80211_DummyP2pIf_Close(
	IN PNET_DEV		dev_p)
{
	struct wireless_dev *wdev = dev_p->ieee80211_ptr;

	if (!wdev)
		return -EINVAL;

	wdev->wiphy->interface_modes = (wdev->wiphy->interface_modes)
								   & (~(BIT(NL80211_IFTYPE_P2P_CLIENT)
										| BIT(NL80211_IFTYPE_P2P_GO)
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
										| BIT(RT_CMD_80211_IFTYPE_P2P_DEVICE)
#endif /* LINUX_VERSION_CODE: 3.7.0 */
									   ));
	return 0;
}

static INT CFG80211_DummyP2pIf_Ioctl(
	IN PNET_DEV				dev_p,
	IN OUT VOID			*rq_p,
	IN INT					cmd)
{
	RTMP_ADAPTER *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		return -ENETDOWN;

	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, " --->\n");
	return rt28xx_ioctl(dev_p, rq_p, cmd);
}

static INT CFG80211_DummyP2pIf_PacketSend(
	IN PNDIS_PACKET	skb_p,
	IN PNET_DEV			dev_p)
{
	return 0;
}

VOID RTMP_CFG80211_DummyP2pIf_Remove(
	IN VOID		*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	PNET_DEV dummy_p2p_net_dev = (PNET_DEV)cfg80211_ctrl->dummy_p2p_net_dev;
	struct wifi_dev *wdev = &cfg80211_ctrl->dummy_p2p_wdev;

	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "=====>\n");
	RtmpOSNetDevProtect(1);

	if (dummy_p2p_net_dev) {
		RTMP_OS_NETDEV_STOP_QUEUE(dummy_p2p_net_dev);
		RtmpOSNetDevDetach(dummy_p2p_net_dev);
		wdev_deinit(pAd, wdev);
		wdev->if_dev = NULL;
		os_free_mem(dummy_p2p_net_dev->ieee80211_ptr);
		dummy_p2p_net_dev->ieee80211_ptr = NULL;
		RtmpOSNetDevProtect(0);
		RtmpOSNetDevFree(dummy_p2p_net_dev);
		RtmpOSNetDevProtect(1);
		cfg80211_ctrl->flg_cfg_dummy_p2p_init = FALSE;
	}

	RtmpOSNetDevProtect(0);
	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=====\n");
}

static struct wifi_dev_ops p2p_wdev_ops = {
	.tx_pkt_allowed = sta_tx_pkt_allowed,
	.fp_tx_pkt_allowed = sta_tx_pkt_allowed,
	.send_data_pkt = sta_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = sta_send_mlme_pkt,
	.tx_pkt_handle = sta_tx_pkt_handle,
	.legacy_tx = sta_legacy_tx,
	.ampdu_tx = sta_ampdu_tx,
	.frag_tx = sta_frag_tx,
	.amsdu_tx = sta_amsdu_tx,
	.fill_non_offload_tx_blk = sta_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = sta_fill_offload_tx_blk,
	.mlme_mgmtq_tx = sta_mlme_mgmtq_tx,
	.mlme_dataq_tx = sta_mlme_dataq_tx,
	.ieee_802_11_data_tx = sta_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = sta_ieee_802_3_data_tx,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.ieee_802_3_data_rx = sta_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = sta_ieee_802_11_data_rx,
	.find_cipher_algorithm = sta_find_cipher_algorithm,
	.mac_entry_lookup = sta_mac_entry_lookup,
	.media_state_connected = sta_media_state_connected,
	.ioctl = rt28xx_sta_ioctl,
	.open = wifi_sys_open,
	.close = wifi_sys_close,
	.linkup = wifi_sys_linkup,
	.linkdown = wifi_sys_linkdown,
	.conn_act = wifi_sys_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};

VOID RTMP_CFG80211_DummyP2pIf_Init(
	IN VOID		*pAdSrc)
{
#define INF_CFG80211_DUMMY_P2P_NAME "p2p"
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	RTMP_OS_NETDEV_OP_HOOK	netDevHook, *pNetDevOps;
	PNET_DEV	new_dev_p;
	UINT32 MC_RowID = 0, IoctlIF = 0, Inf = INT_P2P;
	UINT preIfIndex = 0;
	struct wireless_dev *pWdev;
	struct wifi_dev *wdev = NULL;
	INT32 Ret;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "=====>\n");

	if (cfg80211_ctrl->flg_cfg_dummy_p2p_init != FALSE)
		return;

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	cfg80211_ctrl->P2POpStatusFlags	= CFG_P2P_DISABLE;
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE*/
	cfg80211_ctrl->bP2pCliPmEnable = FALSE;
	cfg80211_ctrl->bPreKeepSlient = FALSE;
	cfg80211_ctrl->bKeepSlient = FALSE;
	cfg80211_ctrl->NoAIndex = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->MyGOwcid = MAX_LEN_OF_MAC_TABLE;
	cfg80211_ctrl->CTWindows = 0;	/* CTWindows and OppPS parameter field */
	pNetDevOps = &netDevHook;
	/* init operation functions and flags */
	os_zero_mem(&netDevHook, sizeof(netDevHook));
	netDevHook.open = CFG80211_DummyP2pIf_Open;	         /* device opem hook point */
	netDevHook.stop = CFG80211_DummyP2pIf_Close;	     /* device close hook point */
	netDevHook.xmit = CFG80211_DummyP2pIf_PacketSend;    /* hard transmit hook point */
	netDevHook.ioctl = CFG80211_DummyP2pIf_Ioctl;	     /* ioctl hook point */
	new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, Inf, preIfIndex, sizeof(PRTMP_ADAPTER), INF_CFG80211_DUMMY_P2P_NAME);

	if (new_dev_p == NULL) {
		/* allocation fail, exit */
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate network device fail (CFG80211: Dummy P2P IF)...\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Register CFG80211 I/F (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));

	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	os_move_mem(&pNetDevOps->devAddr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	pNetDevOps->needProtcted = TRUE;
	os_alloc_mem_suspend(NULL, (UCHAR **)&pWdev, sizeof(*pWdev));
	os_zero_mem((PUCHAR)pWdev, sizeof(*pWdev));

	if (unlikely(!pWdev)) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Could not allocate wireless device\n");
		return;
	}

	new_dev_p->ieee80211_ptr = pWdev;
	pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
	SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
	pWdev->netdev = new_dev_p;
#if (KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE)
	pWdev->iftype = RT_CMD_80211_IFTYPE_P2P_DEVICE;
#else
	pWdev->iftype = RT_CMD_80211_IFTYPE_P2P_CLIENT;
#endif /* LINUX_VERSION_CODE: 3.7.0 */
	wdev = &cfg80211_ctrl->dummy_p2p_wdev;
	Ret = wdev_init(pAd, wdev, WDEV_TYPE_P2P_DEVICE, new_dev_p, 0, NULL, (VOID *)pAd);

	if (!Ret) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===============> fail register the wdev for dummy p2p\n");
		RtmpOSNetDevFree(new_dev_p);
		return;
	}

	Ret = wdev_ops_register(wdev, WDEV_TYPE_P2P_DEVICE, &p2p_wdev_ops,
							cap->wmm_detect_method);

	if (!Ret) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "register wdev_ops %s failed, free net device!\n",
				  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p));
		RtmpOSNetDevFree(new_dev_p);
		return;
	}

	wdev_attr_update(pAd, wdev);
	COPY_MAC_ADDR(wdev->if_addr, pNetDevOps->devAddr);
	RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
	RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
	RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	cfg80211_ctrl->dummy_p2p_net_dev = new_dev_p;
	cfg80211_ctrl->flg_cfg_dummy_p2p_init = TRUE;
	MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<=====\n");
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef CFG80211_MULTI_STA
BOOLEAN RTMP_CFG80211_MULTI_STA_ON(VOID *pAdSrc, PNET_DEV pNewNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV pDevEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	if (!pNewNetDev)
		return FALSE;

	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;

	while (pDevEntry != NULL) {
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN)
			&& (pNewNetDev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_STATION))
			return TRUE;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}

	return FALSE;
}

VOID RTMP_CFG80211_MutliStaIf_Init(VOID *pAdSrc)
{
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
#define INF_CFG80211_MULTI_STA_NAME "muti-sta0"
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	CMD_RTPRIV_IOCTL_80211_VIF_SET vifInfo;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	vifInfo.vifType = RT_CMD_80211_IFTYPE_STATION;
	vifInfo.vifNameLen = strlen(INF_CFG80211_MULTI_STA_NAME);
	os_zero_mem(vifInfo.vifName, sizeof(vifInfo.vifName));
	NdisCopyMemory(vifInfo.vifName, INF_CFG80211_MULTI_STA_NAME, vifInfo.vifNameLen);

	if (RTMP_DRIVER_80211_VIF_ADD(pAd, &vifInfo) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "VIF Add error.\n");
		return;
	}

	cfg80211_ctrl->multi_sta_net_dev = RTMP_CFG80211_FindVifEntry_ByType(pAd, RT_CMD_80211_IFTYPE_STATION);
	cfg80211_ctrl->flg_cfg_multi_sta_init = TRUE;
}

VOID RTMP_CFG80211_MutliStaIf_Remove(VOID *pAdSrc)
{
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (cfg80211_ctrl->multi_sta_net_dev) {
		RtmpOSNetDevProtect(1);
		RTMP_DRIVER_80211_VIF_DEL(pAd, cfg80211_ctrl->multi_sta_net_dev,
								  RT_CMD_80211_IFTYPE_STATION);
		RtmpOSNetDevProtect(0);
		cfg80211_ctrl->flg_cfg_multi_sta_init = FALSE;
	}
}
#endif /* CFG80211_MULTI_STA */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE: 2.6.28 */

