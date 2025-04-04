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
	mt_phy.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "mt_phy.tmh"
#endif
#else
#include "rt_config.h"
#endif

/*define local variable*/
static struct phy_ops MtPhyOp;

static INT32 MTBbpInit(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(): Init BBP Registers\n");

	if (ops->AsicBbpInit != NULL)
		ops->AsicBbpInit(pAd);

	return NDIS_STATUS_SUCCESS;
}


INT32 MTShowPartialBBP(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 Offset, Value = 0;

	for (Offset = Start; Offset <= End; Offset += 4) {
		PHY_IO_READ32(pAd->hdev_ctrl, Offset, &Value);
		MTWF_PRINT("%s():0x%04x 0x%08x\n", __func__, Offset, Value);
	}

	return TRUE;
}


INT32 MTShowAllBBP(RTMP_ADAPTER *pAd)
{
	UINT32 Offset, Value = 0;

	for (Offset = 0x10000; Offset <= 0x20000; Offset += 4) {
		PHY_IO_READ32(pAd->hdev_ctrl, Offset, &Value);
		MTWF_PRINT("%s():0x%04x 0x%08x\n", __func__, Offset, Value);
	}

	return TRUE;
}


#ifdef SMART_CARRIER_SENSE_SUPPORT
INT MTSmartCarrierSense(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->SmartCarrierSense != NULL)
		ops->SmartCarrierSense(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef DYNAMIC_WMM_SUPPORT
INT MTDynamicWmmProcess(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->DynamicWmmProcess != NULL)
		ops->DynamicWmmProcess(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* DYNAMIC_WMM_SUPPORT */

static VOID mt_phy_ops(VOID)
{
	os_zero_mem(&MtPhyOp, sizeof(struct phy_ops));
	MtPhyOp.bbp_init = MTBbpInit;
	MtPhyOp.bbp_set_bw = NULL;
	MtPhyOp.ShowPartialBBP = MTShowPartialBBP;
	MtPhyOp.ShowAllBBP = MTShowAllBBP;
	MtPhyOp.ShowPartialRF = MTShowPartialRF;
	MtPhyOp.ShowAllRF = MTShowAllRF;
#ifdef CONFIG_AP_SUPPORT
	MtPhyOp.AutoCh = MTAPAutoSelectChannel;
#endif/*CONFIG_AP_SUPPORT*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
	MtPhyOp.Smart_Carrier_Sense = MTSmartCarrierSense;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT
	MtPhyOp.Dynamic_Wmm_Process = MTDynamicWmmProcess;
#endif /* DYNAMIC_WMM_SUPPORT */
}

INT mt_phy_probe(RTMP_ADAPTER *pAd)
{
	mt_phy_ops();
	pAd->phy_op = &MtPhyOp;

	return TRUE;
}
