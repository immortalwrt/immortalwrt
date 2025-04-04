/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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
	dynwmm.c
*/
#ifdef DYNAMIC_WMM_SUPPORT
#include "rt_config.h"
#endif


#ifdef DYNAMIC_WMM_SUPPORT

INT DynWmmSetDynamicWmmEnable(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx,
	IN UINT32 value
)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = DYN_WMM_CMD_ENABLE;
	struct _CMD_DYNAMIC_WMM_ENABLE param;

	os_zero_mem(&param, sizeof(param));

	param.u1BandIdx = BandIdx;
	param.u1DynWmmEnable = value;

	if (param.u1DynWmmEnable >= DYN_WMM_MODE_MAX) {
		Ret = 0;
		goto error;
	}

	if (value == DYN_WMM_DISABLE)
		pAd->DynWmmCtrl.DynWmmEnable[BandIdx] = DYN_WMM_DISABLE;
	else if (value == DYN_WMM_ENABLE)
		pAd->DynWmmCtrl.DynWmmEnable[BandIdx] = DYN_WMM_ENABLE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"BandIdx:%d u1DynWmmEnable:%d !!\n",
			param.u1BandIdx, param.u1DynWmmEnable);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_DYN_WMM);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret?DBG_LVL_INFO:DBG_LVL_ERROR,
			"(Ret = %d\n", Ret);

	return Ret;
}

VOID SetDynamicWmmEnable(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value)
{
	MTWF_PRINT("%s(): BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, value);

	DynWmmSetDynamicWmmEnable(pAd, BandIdx, value);
}

VOID SetDynamicWmmProcess(RTMP_ADAPTER *pAd)
{
	struct DYNAMIC_WMM_CTRL *pDynWmmCtrl;
	UCHAR BandIndex;

	pDynWmmCtrl = &pAd->DynWmmCtrl;

	for (BandIndex = 0; BandIndex < DBDC_BAND_NUM; BandIndex++) {
		if (pDynWmmCtrl->DynWmmEnable[BandIndex] == DYN_WMM_ENABLE)
			SendDynamicWmmDataProc(pAd, BandIndex);
	}
}

VOID DynWmm_init(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!\n");

	DynWmmSetDynamicWmmEnable(pAd, DBDC_BAND0, pAd->DynWmmCtrl.DynWmmEnable[DBDC_BAND0]);
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode == 1)
		DynWmmSetDynamicWmmEnable(pAd, DBDC_BAND1, pAd->DynWmmCtrl.DynWmmEnable[DBDC_BAND1]);
#endif
}

#endif /* DYNAMIC_WMM_SUPPORT */
