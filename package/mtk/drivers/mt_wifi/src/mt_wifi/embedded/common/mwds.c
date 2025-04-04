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
    mwds.c

    Abstract:
    This is MWDS feature used to process those 4-addr of connected APClient or STA.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rt_config.h"


VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN mwds_enable = FALSE;
	BOOLEAN Ret = FALSE;

	if (pEntry->bSupportMWDS && pEntry->wdev && pEntry->wdev->bSupportMWDS)
		mwds_enable = TRUE;

#ifdef CONFIG_MAP_SUPPORT
/*	MAP have higher priority,
*	If MAP is enabled and peer have MAP capability as well,
*	use MAP connection and disable MWDS
*/

	if (IS_MAP_ENABLE(pAd) &&
		(pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
		mwds_enable = FALSE;
#endif

	if (mwds_enable)
		Ret = a4_ap_peer_enable(pAd, pEntry, A4_TYPE_MWDS);

	if (Ret == FALSE)
		MWDSAPPeerDisable(pAd, pEntry);
}

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN Ret;

	Ret = a4_ap_peer_disable(pAd, pEntry, A4_TYPE_MWDS);
	if (Ret)
		pEntry->bSupportMWDS = FALSE;
}

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	BOOLEAN mwds_enable = FALSE;
	BOOLEAN Ret = FALSE;

	if (pApCliEntry->MlmeAux.bSupportMWDS && pApCliEntry->wdev.bSupportMWDS)
		mwds_enable = TRUE;

#ifdef CONFIG_MAP_SUPPORT
/*	MAP have higher priority
*	If MAP is enabled and peer have MAP capability as well,
*	use MAP connection and disable MWDS
*/
	if (IS_MAP_ENABLE(pAd) &&
		(pEntry->DevPeerRole &
			(BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS))))
		mwds_enable = FALSE;
#endif

	if (mwds_enable)
		Ret = a4_apcli_peer_enable(pAd, pApCliEntry, pEntry, A4_TYPE_MWDS);

	if (Ret == FALSE)
		MWDSAPCliPeerDisable(pAd, pApCliEntry, pEntry);
}

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry
)
{
	a4_apcli_peer_disable(pAd, pApCliEntry, pEntry, A4_TYPE_MWDS);
}
#endif /* APCLI_SUPPORT */

INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen
)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (VALID_MBSS(pAd, ifIndex)) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

			if (!wdev->bSupportMWDS) {
				wdev->bSupportMWDS = TRUE;
				a4_interface_init(pAd, ifIndex, TRUE, A4_TYPE_MWDS);

				if (!isDevOpen)
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}
		}
	}

#ifdef APCLI_SUPPORT
	else {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->StaCfg[ifIndex].wdev;

			if (!wdev->bSupportMWDS) {
				wdev->bSupportMWDS = TRUE;
				a4_interface_init(pAd, ifIndex, FALSE, A4_TYPE_MWDS);
			}
		}
	}

#endif /* APCLI_SUPPORT */

	return TRUE;
}

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose
)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (VALID_MBSS(pAd, ifIndex)) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

			if (wdev && wdev->bSupportMWDS) {
				wdev->bSupportMWDS = FALSE;
				a4_interface_deinit(pAd, ifIndex, TRUE, A4_TYPE_MWDS);

				if (!isDevClose)
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}
		}
	}

#ifdef APCLI_SUPPORT
	else {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->StaCfg[ifIndex].wdev;

			if (wdev && wdev->bSupportMWDS) {
				wdev->bSupportMWDS = FALSE;
				a4_interface_deinit(pAd, ifIndex, FALSE, A4_TYPE_MWDS);
			}
		}
	}

#endif /* APCLI_SUPPORT */

	return TRUE;
}


INT Set_Enable_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN Enable,
	IN  BOOLEAN isAP
)
{
	POS_COOKIE      pObj;
	UCHAR           ifIndex;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (isAP) {
		ifIndex = pObj->ioctl_if;
		pAd->ApCfg.MBSSID[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0) ? FALSE : TRUE;
	}

#ifdef APCLI_SUPPORT
	else {
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		pAd->StaCfg[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0) ? FALSE : TRUE;
	}

#endif /* APCLI_SUPPORT */

	if (Enable)
		MWDSEnable(pAd, ifIndex, isAP, FALSE);
	else
		MWDSDisable(pAd, ifIndex, isAP, FALSE);

	return TRUE;
}

INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg
)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	return Set_Enable_MWDS_Proc(pAd, Enable, TRUE);
}

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg
)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	return Set_Enable_MWDS_Proc(pAd, Enable, FALSE);
}


VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer
)
{
	PSTRING tmpptr = NULL;
#ifdef CONFIG_AP_SUPPORT

	/* ApMWDS */
	if (RTMPGetKeyParameter("ApMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApMWDS=%d\n", Value);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT

	/* ApCliMWDS */
	if (RTMPGetKeyParameter("ApCliMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_APCLI_NUM)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);

			pAd->StaCfg[i].wdev.bDefaultMwdsStatus = (Value == 0) ? FALSE : TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCliMWDS=%d\n", Value);
		}
	}

#endif /* APCLI_SUPPORT */
}

#endif /* MWDS */
