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
	fwdl.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "fwdl.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

INT NICLoadRomPatch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_load_patch(ad);

	return ret;
}


INT NICLoadFirmware(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_load_fw(ad);

	return ret;
}

VOID NICRestartFirmware(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_restart_fw(ad);
}


INT FwdlHookInit(RTMP_ADAPTER *pAd)
{
	int ret = NDIS_STATUS_SUCCESS;

	ret = mt_fwdl_hook_init(pAd);

	return ret;

}
