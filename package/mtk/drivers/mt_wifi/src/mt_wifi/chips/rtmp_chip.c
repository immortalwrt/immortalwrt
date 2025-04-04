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
	rt_chip.c

	Abstract:
	Ralink Wireless driver CHIP related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

/*
========================================================================
Routine Description:
	write memory

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"
Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteMemory(
	IN	RTMP_ADAPTER	*pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit)
{
	switch (Unit) {

	case 4:
		mac_io_write32(pAd, Offset, Value);

	default:
		break;
	}
}

UINT8 NICGetBandSupported(RTMP_ADAPTER *pAd)
{
	if (BOARD_IS_5G_ONLY(pAd))
		return RFIC_5GHZ;
	else if (BOARD_IS_2G_ONLY(pAd))
		return RFIC_24GHZ;
	else if (RFIC_IS_5G_BAND(pAd))
		return (RFIC_24GHZ | RFIC_5GHZ);
	else
		return RFIC_24GHZ;
}


INT WaitForAsicReady(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT AsicGetMacVersion(RTMP_ADAPTER *pAd)
{
	UINT32 reg = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* TODO: shiang-7603 */
	if (cap->hif_type == HIF_MT) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(%d): Not support for HIF_MT yet!\n",
				 __LINE__);
		return FALSE;
	}

	if (WaitForAsicReady(pAd) == TRUE) {
		RTMP_IO_READ32(pAd->hdev_ctrl, reg, &pAd->MACVersion);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MACVersion[Ver:Rev]=0x%08x : 0x%08x\n",
				 pAd->MACVersion, pAd->ChipID);
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "failed!\n");
		return FALSE;
	}
}


/*
========================================================================
Routine Description:
	Initialize chip related information.

Arguments:
	pCB				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
int RtmpChipOpsHook(VOID *pCB)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pCB;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	int ret = 0;

#ifdef MT_DFS_SUPPORT
	/* Initial new DFS channel list */
	DfsSetNewChInit(pAd);
#endif

	/* sanity check */
	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	/* TODO: shiang-7603 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7615(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "(%d): Not support for HIF_MT yet! MACVersion=0x%x\n",
				  __LINE__, pAd->MACVersion);
	}

	if (pAd->MACVersion == 0xffffffff)
		return -1;

	/* default init */
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_LEGACY);
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
		/* wilsonl, need DE provide info */
#ifndef MT7986
#ifndef MT7981
		RTMP_SYS_IO_READ32(0xb000000c, &pAd->CommonCfg.CID);
		RTMP_SYS_IO_READ32(0xb0000000, &pAd->CommonCfg.CN);
#endif
#endif
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CN: %lx\tCID = %lx\n",
				 pAd->CommonCfg.CN, pAd->CommonCfg.CID);
	}

#endif /* RTMP_RBUS_SUPPORT */
	/*initial chip hook function*/
	WfSysPreInit(pAd);

#ifdef DOT11W_PMF_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF] Encryption mode = %d\n", pChipCap->FlgPMFEncrtptMode);
#endif /* DOT11W_PMF_SUPPORT */
	return ret;
}


