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
    ap_wds.c

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
    Fonchi    02-13-2007      created
*/
#define RTMP_MODULE_OS

#ifdef WDS_SUPPORT

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"


NET_DEV_STATS *RT28xx_get_wds_ether_stats(PNET_DEV net_dev);


/* Register WDS interface */
VOID RT28xx_WDS_Init(VOID *pAd, UCHAR band_idx, PNET_DEV net_dev)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;

	NdisZeroMemory((PUCHAR)&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = wds_virtual_if_open;
	netDevOpHook.stop = wds_virtual_if_close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	netDevOpHook.get_stats = RT28xx_get_wds_ether_stats;
	NdisMoveMemory(&netDevOpHook.devAddr[0], RTMP_OS_NETDEV_GET_PHYADDR(net_dev), MAC_ADDR_LEN);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "The new WDS interface MAC = "MACSTR"\n",
			 MAC2STR(netDevOpHook.devAddr));
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_INIT,
						0, &netDevOpHook, band_idx);
}


INT wds_virtual_if_open(PNET_DEV pDev)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__);

	if (VIRTUAL_IF_INIT(pAd, pDev) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, pDev) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(pDev);
	RTMP_OS_NETDEV_START_QUEUE(pDev);
	return 0;
}


INT wds_virtual_if_close(PNET_DEV pDev)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_WDS, DBG_LVL_INFO, "%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__);

	/* RTMP_OS_NETDEV_CARRIER_OFF(pDev); */
	RTMP_OS_NETDEV_STOP_QUEUE(pDev);

	VIRTUAL_IF_DOWN(pAd, pDev);

	VIRTUAL_IF_DEINIT(pAd, pDev);

	RT_MOD_HNAT_DEREG(pDev);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}


VOID RT28xx_WDS_Remove(VOID *pAd)
{
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_REMOVE, 0, NULL, 0);
}

#endif /* WDS_SUPPORT */
