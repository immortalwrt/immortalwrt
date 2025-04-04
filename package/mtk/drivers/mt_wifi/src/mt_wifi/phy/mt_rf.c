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
	mt_rf.c
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "mt_rf.tmh"
#endif
#else
#include "rt_config.h"
#endif

INT32 MTShowPartialRF(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
#ifdef COMPOS_WIN
#else
	UINT32 RFIdx, Offset, Value = 0;

#ifdef MT_MAC
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++) {
		for (Offset = Start; Offset <= End; Offset = Offset + 4) {
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			MTWF_PRINT("%d 0x%04x 0x%08x\n", RFIdx, Offset, Value);
		}
	}
#endif /* MT_MAC */
#endif
	return TRUE;
}


INT32 MTShowAllRF(RTMP_ADAPTER *pAd)
{
#ifdef COMPOS_WIN
#else
	UINT32 RFIdx, Offset, Value = 0;

#ifdef MT_MAC
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++) {
		for (Offset = 0; Offset <= (cap->MaxNumOfRfId * 4); Offset = Offset + 4) {
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdRFRegAccessRead(pAd, RFIdx, Offset, &Value);
			MTWF_PRINT("%d 0x%04x 0x%08x\n", RFIdx, Offset, Value);
		}
	}
#endif /* MT_MAC */
#endif
	return TRUE;
}

