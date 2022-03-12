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

*/
#include "rt_config.h"
#ifdef CFG_SUPPORT_FALCON_MURU

typedef enum _ENUM_MURU_PLATFORM_PERFORMANCE_LEVEL_T {
    MURU_PLAT_PERF_LVL_0 = 0,
    MURU_PLAT_PERF_LVL_1 = 1, /* MT7621 PCIe Gen1 */
    MURU_PLAT_PERF_LVL_2 = 2  /* MT7622 PCIe Gen2 */
} ENUM_MURU_PLATFORM_PERFORMANCE_LEVEL_T;

#ifdef CFG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL
#define CFG_MURU_PLAT_PERF_LVL  CFG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL
#else
#define CFG_MURU_PLAT_PERF_LVL  MURU_PLAT_PERF_LVL_2
#endif /* CFG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL */

static VOID muruEventDispatcher(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len);
/*MURU Start*/
/*
*
*Description:
*	Set Falcon MURU Module SND Parameter Ctrl
*
*Parameters:
*	Standard Falcon MURU SND Parameter
*
*/

INT SetMuruHeSndCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_HESND_CTRL;
	CMD_MURU_HESND_CTRL param = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:\n", __func__));

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.ucTriggerFlow = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucTriggerFlow = %u\n", __func__, param.ucTriggerFlow));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.ucInterval = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucInterval = %u\n", __func__, param.ucInterval));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		param.ucBrRuAlloc = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.ucBrRuAlloc = cpu2le16(param.ucBrRuAlloc);
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucBrRuAlloc = %u\n", __func__, param.ucBrRuAlloc));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.ucPpduDur = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.ucPpduDur = cpu2le32(param.ucPpduDur);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucPpduDur = %u\n", __func__, param.ucPpduDur));
	} else {
		Ret = 0;
		goto error;
	}



	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


/*
*
*Description:
*	Set Falcon MURU Module Bsrp Parameter Ctrl
*
*Parameters:
*	Standard Falcon MURU Bsrp Parameter
*
*/
INT SetMuruBsrpCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_BSRP_CTRL;
	CMD_MURU_BSRP_CTRL param = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		if (os_str_tol(pch, 0, 10))
			param.fgExtCmdBsrp = 1;
		else
			param.fgExtCmdBsrp = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.fgExtCmdBsrp = %u\n", __func__, param.fgExtCmdBsrp));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.u1TriggerFlow = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucTriggerFlow = %u\n", __func__, param.u1TriggerFlow));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.u2BsrpInterval = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.u2BsrpInterval = cpu2le16(param.u2BsrpInterval);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucBsrpInterval = %u\n", __func__, param.u2BsrpInterval));
	} else {
		Ret = 0;
		goto error;
	}


	pch = strsep(&arg, "-");

	if (pch != NULL) {
		param.u2BsrpRuAlloc = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.u2BsrpRuAlloc = cpu2le16(param.u2BsrpRuAlloc);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(param.ucBsrpRuAlloc = %u\n", __func__, param.u2BsrpRuAlloc));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		param.u4TriggerType = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.u4TriggerType = cpu2le32(param.u4TriggerType);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(param.u4TriggerType = %u\n", __func__, param.u4TriggerType));
	} else {
		Ret = 0;
		goto error;
	}
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(Ret = %d_)\n", __func__, Ret));
	return Ret;
}



/*
*
*Description:
*	Set Falcon MURU Module global and protect section Ctrl
*
*Parameters:
*	Standard Falcon MURU global and protect Parameters
*
*/
INT SetMuruGlobalProtSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_GLOBAL_PROT_SEC_CTRL;
	CMD_MURU_GLBOAL_PROT_SEC_CTRL param = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.ucExp = os_str_tol(pch, 0, 10);
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucTxOp = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucPdc = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucProt = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucProtRuAlloc = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucFixedRate = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucSuTx = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucTpPolicy = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucTriggerFlow = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


/*
*
*Description:
*	Set Falcon MURU Module TX-Data section Ctrl
*
*
*Parameters:
*	Standard Falcon MURU TX-Data section Parameters
*
*/
INT SetMuruTxDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_TX_DATA_SEC_CTRL;
	CMD_MURU_TX_DATA_SEC_CTRL param = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucBw = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucMuPpduDur = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

/*
*
*Description:
*	Set Falcon MURU Module TRIG-Data section Ctrl
*
*Parameters:
*	Standard Falcon MURU TRIG-Data section Parameters
*
*/
INT SetMuruTrigDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_TRIG_DATA_SEC_CTRL;
	CMD_MURU_TRIG_DATA_SEC_CTRL param = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucBaPolicy = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucGBABw = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.ucGBAMuPpduDur = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

/*
*
*Description:
*       Set Falcon MURU Module ARB OP mode
*
*Parameters:
*       Standard Falcon MURU ARB OP mode
*
*/
INT SetMuruArbOpMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	UINT8 OpMode = 0;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		OpMode = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (wifi_test_muru_set_arb_op_mode(pAd, (UINT8)OpMode) == FALSE) {
		Ret = 0;
		goto error;
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruAlgoDbgCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_ALGO_DBG_CTRL;
	CMD_MURU_ALGO_DBG_CTRL param = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.u1OpMode = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		param.u1Enable = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		param.u2Period = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		param.u2Period = cpu2le16(param.u2Period);
#endif
	} else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


#ifdef DABS_QOS

INT32 MtCmdCr4QoSSet(RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 u4ExtSize, PUINT8 pExtData)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_SET_T  CmdCr4SetSet;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdCr4SetSet) + u4ExtSize);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_SET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	CmdCr4SetSet.u4Cr4SetArg0 = cpu2le32(arg0);
	CmdCr4SetSet.u4Cr4SetArg1 = 0;
	CmdCr4SetSet.u4Cr4SetArg2 = 0;
	AndesAppendCmdMsg(msg, (char *)&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	AndesAppendCmdMsg(msg, (char *)pExtData, u4ExtSize);

	Ret = chip_cmd_tx(pAd, msg);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: (ret = %d)\n", __func__, Ret));
	return Ret;
}

BOOLEAN SendQoSCmd(RTMP_ADAPTER *pAd, UINT32 op_flag, MURU_QOS_SETTING *pqos_setting)
{
	BOOLEAN ret = TRUE;
	CMD_MURU_QOS_CFG param;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = MURU_SET_QOS_CFG;
	struct cmd_msg *msg = NULL;

	memcpy(&param.QoSSetting, pqos_setting, sizeof(MURU_QOS_SETTING));
	param.u4OpFlag = op_flag;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg alloc fail!!!\n", __func__));
		ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(op = %d)\n", __func__, op_flag));

	MtCmdFwLog2Host(pAd, 0, 2);
	MtCmdFwLog2Host(pAd, 1, 2);

	MtCmdCr4QoSSet(pAd, WA_SET_OPTION_DABS_QOS_CMD, sizeof(CMD_MURU_QOS_CFG), (PUINT8)&param);

	if (op_flag == QOS_CMD_PARAM_SETTING)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (
			"Set STA%d AC%d,DelayBound=%u,DelayReq=%u Weight=%u,Rate=%u,BWReq=%u,Dir=%u,Dropth=%u\n",
			pqos_setting->u2WlanIdx, pqos_setting->u1AC,
			pqos_setting->u2DelayBound,
			pqos_setting->u2DelayReq, pqos_setting->u1DelayWeight,
			pqos_setting->u2DataRate, pqos_setting->u2BWReq,
			pqos_setting->u1Dir,
			pqos_setting->u2DropThres
			));

	if (op_flag == QOS_CMD_PARAM_DELETE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (
			"Set STA%d AC%d, idx = %d\n",
			pqos_setting->u2WlanIdx, pqos_setting->u1AC,
			pqos_setting->u1Idx));

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(ret = %d)\n", __func__, ret));

	MtCmdFwLog2Host(pAd, 0, 0);
	MtCmdFwLog2Host(pAd, 1, 0);


	return ret;
}

INT SetMuruQoSCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = TRUE;
	UINT32 op, value[10] = {0}, rv, op_flag = 0;
	MURU_QOS_SETTING qos_setting = {0};
	/* prepare command message */

	rv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &op, &value[0], &value[1], &value[2],
		&value[3], &value[4], &value[5], &value[6], &value[7], &value[8]);

	if (rv == 0) {
		ret = FALSE;
		goto error;
	}

	switch (op) {
		case 0:
			op_flag = value[0];
			break;
		case 1:
			op_flag = QOS_CMD_PARAM_SETTING;
			qos_setting.u2WlanIdx = value[0];
			qos_setting.u1AC = value[1];
			qos_setting.u2DelayBound = value[2];
			qos_setting.u2DelayReq = value[3];
			qos_setting.u1DelayWeight = value[4];
			qos_setting.u2DataRate = value[5];
			qos_setting.u2BWReq = value[6];
			qos_setting.u1Dir = value[7];
			qos_setting.u2DropThres = value[8];
			break;
		default:

			ret = FALSE;
			goto error;

	}
	if (SendQoSCmd(pAd, op_flag, &qos_setting) == FALSE)
		ret = FALSE;
error:
	if (ret == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (
			"iwpriv ra0 set set_muru_qos=1-[wlanid]-[AC]-[DlyBound]-[DlyReq]-[DlyWeight]-[Rate]-[BWReq]-[DIR]-[DropTh]\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (
			"iwpriv ra0 set set_muru_qos=0-[enable/disable]\n"));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}
#endif /* DABS_QOS */

INT SetMuruSuTx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_SUTX;
	UINT8 forceSuTx = 0;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		forceSuTx = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(forceSuTx));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&forceSuTx, sizeof(forceSuTx));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruTxcTxStats(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_TXC_TX_STATS_EN;
	UINT8 u1TxcTxEnable = 0;

	if (arg != NULL)
		u1TxcTxEnable = os_str_tol(arg, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(u1TxcTxEnable));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&u1TxcTxEnable, sizeof(u1TxcTxEnable));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuru20MDynAlgo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_20M_DYN_ALGO;
	UINT8 dynAlgoEnable = 0;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		dynAlgoEnable = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(dynAlgoEnable));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&dynAlgoEnable, sizeof(dynAlgoEnable));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruProtFrameThr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_PROT_FRAME_THR;
	CMD_MURU_SET_PROT_FRAME_THR rSetProtFrameThr;

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		rSetProtFrameThr.u4ProtFrameThr = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		rSetProtFrameThr.u4ProtFrameThr
			= cpu2le32(rSetProtFrameThr.u4ProtFrameThr);
#endif
	} else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_PROT_FRAME_THR));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&rSetProtFrameThr, sizeof(CMD_MURU_SET_PROT_FRAME_THR));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruPlatformTypeProc(RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_PLATFORM_TYPE;
	CMD_MURU_SET_PLATFORM_TYPE param = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	param.ucPlatformType = CFG_MURU_PLAT_PERF_LVL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: param.ucPlatformType = %u\n", __func__, param.ucPlatformType));

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


INT SetMuruStatisticConfig(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_STATISTIC_CONFIG;
	CMD_MURU_STAT_RECORD_CTRL ctrl = {0};

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		if (os_str_tol(pch, 0, 10))
			ctrl.u1Mode = 1;
		else
			ctrl.u1Mode = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(ctrl.u1Mode = %u\n", __func__, ctrl.u1Mode));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		ctrl.u2StartWcid = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(ctrl.u2StartWcid = %u\n", __func__, ctrl.u2StartWcid));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		ctrl.u2EndWcid = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(ctrl.u2EndWcid = %u\n", __func__, ctrl.u2EndWcid));
	} else {
		Ret = 0;
		goto error;
	}


	pch = strsep(&arg, "-");
	if (pch != NULL) {
		ctrl.u2TimerInterval = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(ctrl.u2TimerInterval = %u\n", __func__, ctrl.u2TimerInterval));
	} else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(ctrl));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&ctrl, sizeof(ctrl));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruTxopOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_TXOP_ONOFF;
	CMD_MURU_SET_TXOP_ONOFF rTxop = {0};

	pch = strsep(&arg, "-");

	if (pch != NULL)
		rTxop.u4TxopOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, rTxop.u4TxopOnOff));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_TXOP_ONOFF));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&rTxop, sizeof(CMD_MURU_SET_TXOP_ONOFF));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruUlOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_UL_ONOFF;
	CMD_MURU_SET_UL_ONOFF rRuUl = {0};

	pch = strsep(&arg, "-");

	if (pch != NULL)
		rRuUl.u2UlBsrpOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("invalid format, iwpriv ra0 set set_muru_ul_off=[bsrp]-[data]\n"));
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		rRuUl.u2UlDataOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("invalid format, iwpriv ra0 set set_muru_ul_off=[bsrp]-[data]\n"));
		goto error;
	}
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: bsrp=%d data=%d\n", __func__, rRuUl.u2UlBsrpOnOff, rRuUl.u2UlDataOnOff));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_UL_ONOFF));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&rRuUl, sizeof(CMD_MURU_SET_UL_ONOFF));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetMuruTypeSelect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_MU_TYPE_SELECT;
	UINT8 type = 0;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		type = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, type));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(type));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&type, sizeof(type));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT32 set_muru_ignore_nav(RTMP_ADAPTER *ad, UINT8 ignore)
{
	INT32 ret = TRUE;
	UINT32 cmd = MURU_SET_IGNORE_NAV;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(ad, sizeof(cmd) + sizeof(ignore));
	if (!msg) {
		ret = 0;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&ignore, sizeof(ignore));
	AndesSendCmdMsg(ad, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, ret));
	return ret;
}

INT32 set_muru_cert_send_frame_ctrl(RTMP_ADAPTER *ad,
	UINT32 ppdu_dur,
	UINT16 target_wcid,
	UINT8  interval)
{
	INT32 ret = TRUE;
	UINT32 cmd = MURU_CERT_SEND_FRAME_CTRL;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_MURU_CERT_SEND_FRAME_CTRL rSetParam;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(ad, sizeof(cmd) + sizeof(CMD_MURU_CERT_SEND_FRAME_CTRL));
	if (!msg) {
		ret = 0;
		goto error;
	}

	os_zero_mem(&rSetParam, sizeof(CMD_MURU_CERT_SEND_FRAME_CTRL));

	/* assign packet duration and interval and sedn to target wcid */
	rSetParam.u4PpduDur = ppdu_dur;
	rSetParam.u2TargetWcid = target_wcid;
	rSetParam.u1Interval = interval;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&rSetParam, sizeof(rSetParam));
	AndesSendCmdMsg(ad, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, ret));
	return ret;
}

INT ShowMuruHeSndCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_HESND_CTRL result = {0};
	UINT32 cmd = MURU_GET_HESND_CTRL;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:", __func__));
	pch = arg;


	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}


INT ShowMuruBsrpCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_BSRP_CTRL result = {0};
	UINT32 cmd = MURU_GET_BSRP_CTRL;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:", __func__));
	pch = arg;


	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}


INT ShowMuruGlobalProtSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_GLOBAL_PROT_SEC_CTRL result = {0};
	UINT32 cmd = MURU_GET_GLOBAL_PROT_SEC_CTRL;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

INT ShowMuruTxDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_TX_DATA_SEC_CTRL result = {0};
	UINT32 cmd = MURU_GET_TX_DATA_SEC_CTRL;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

INT ShowMuruTxcTxStats(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	EVENT_MURU_TXCMD_TX_STATS result = {0};
	UINT32 cmd = MURU_GET_TXC_TX_STATS;
	UINT8 u1BandIdx = 0;

	if (arg != NULL)
		u1BandIdx = os_str_tol(arg, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(u1BandIdx));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);

#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&u1BandIdx, sizeof(u1BandIdx));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT ShowMuruTrigDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_TRIG_DATA_SEC_CTRL result = {0};
	UINT32 cmd = MURU_GET_TRIG_DATA_SEC_CTRL;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

/*
*
*Description:
*       Set Falcon MU-MIMO DBG Ctrl
*
*Parameters:
*       Standard Falcon MU-MIMO DBG Ctrl
*
*/
INT ShowMuruMumCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	char *subType;
	char *cur = arg;
	char *idxStr;
	UINT32 cmd;
	UINT16 index;
	MUCOP_TABLE_DISPLAY result;
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;

	subType = strsep((char **)&cur, "-");
	if (strncmp(subType, "grouptbl", strlen(subType)) == 0) {
		cmd = MU_MIMO_GET_GROUP_TBL_ENTRY;
	} else if (strncmp(subType, "profile", strlen(subType)) == 0) {
		cmd = MU_MIMO_GET_PROFILE_ENTRY;
	} else if (strncmp(subType, "clustertbl", strlen(subType)) == 0) {
		cmd = MU_MIMO_GET_CLUSTER_TBL_ENTRY;
	} else if (strncmp(subType, "dlac", strlen(subType)) == 0) {
		cmd = MU_MIMO_GET_DL_AC_TABLE;
	} else if (strncmp(subType, "ultid", strlen(subType)) == 0) {
		cmd = MU_MIMO_GET_UL_TID_TABLE;
	} else {
		Ret = 0;
		goto error;
	}
	idxStr = strsep((char **)&cur, "-");
	if (idxStr == NULL)
		goto error;
	index = simple_strtol(idxStr, NULL, 10);
#ifdef RT_BIG_ENDIAN
	index = cpu2le16(index);
#endif

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT ShowMuruSplCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	EVENT_MURU_GET_SPL_CNT result = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = MURU_GET_SPL_CNT;
	struct cmd_msg *msg = NULL;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT ShowMuruGloAddr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_GET_MURU_GLO_ADDR result = {0};
	UINT32 cmd = MURU_GET_GLO_ADDR;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:", __func__));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

INT ShowMuruUlRuStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	EVENT_MURU_GET_SPL_CNT result = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = MURU_SHOW_ULRU_STATUS;
	struct cmd_msg *msg = NULL;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT ShowMuruLocalData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_muru_local_data)
		ops->show_muru_local_data(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT ShowMuruTxInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_muru_tx_info)
		ops->show_muru_tx_info(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT ShowMuruSharedData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_muru_shared_data)
		ops->show_muru_shared_data(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT ShowMuruManCfgData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_muru_mancfg_data)
		ops->show_muru_mancfg_data(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT ShowMuruStaCapInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_muru_stacap_info)
		ops->show_muru_stacap_info(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

INT ShowMuruMuEdcaParam(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;

	if (os_str_tol(arg, 0, 10) > 0)
		pAd->CommonCfg.bShowMuEdcaParam = TRUE;
	else
		pAd->CommonCfg.bShowMuEdcaParam = FALSE;

	return Ret;
}

INT SetMuruData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_muru_data)
		ops->set_muru_data(pAd, arg);
	else
		return FALSE;

	return TRUE;
}

/*MURU END*/

VOID muru_get_fw_blacklist_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_GET_MURU_FW_BLACKLIST_CTRL pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_GET_MURU_FW_BLACKLIST_CTRL)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:EVENT_GET_MURU_FW_BLACKLIST_CTRL\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("u1FwBlackListDlOfdmaTestFailCnt:%u, u1FwBlackListUlOfdmaTestFailCnt:%u\n",
			pEntry->u1FwBlackListDlOfdmaTestFailCnt,
			pEntry->u1FwBlackListUlOfdmaTestFailCnt));

	/* handler TBD */

}

VOID muru_get_txc_tx_stats_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_EVENT_MURU_TXCMD_TX_STATS pEntry = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_TXCMD_TX_STATS)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:MURU_EVENT_GET_TXC_TX_STATS\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));

	if (ops->show_muru_txc_tx_stats)
		ops->show_muru_txc_tx_stats(pAd, (VOID *)pEntry);
}

static VOID muru_get_hesnd_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_HESND_CTRL pEntry = NULL;


	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_HESND_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg is null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:MURU_EVENT_GET_HESND_CTRL\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ucTriggerFlow = %u, ucInterval:%u, ucBrRuAlloc:%u, ppduDur:%u\n",
		pEntry->rEntry.ucTriggerFlow,
		pEntry->rEntry.ucInterval,
		pEntry->rEntry.ucBrRuAlloc,
		pEntry->rEntry.ucPpduDur));

}


static VOID muru_get_bsrp_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_BSRP_CTRL pEntry = NULL;


	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_BSRP_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg is null!!\n", __func__));
		return;
	}
	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:MURU_EVENT_GET_BSRP_CTRL\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("u1TriggerFlow:%u, u2BsrpInterval:%u, u2BsrpRuAlloc:%u, u4TriggerType:%u, fgExtCmdBsrp:%d\n",
				pEntry->rEntry.u1TriggerFlow,
				pEntry->rEntry.u2BsrpInterval,
				pEntry->rEntry.u2BsrpRuAlloc,
				pEntry->rEntry.u4TriggerType,
				pEntry->rEntry.fgExtCmdBsrp));
}


static VOID muru_get_global_prot_sec_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_GLOBAL_PROT_SEC_CTRL pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_GLOBAL_PROT_SEC_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: msg is null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:EVENT_HQA_GET_SU_CALC_LQ\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucExp = 0x%x\n", cpu2le32(pEntry->rEntry.ucExp)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucTxOp = 0x%x\n", cpu2le32(pEntry->rEntry.ucTxOp)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucPdc = 0x%x\n", cpu2le32(pEntry->rEntry.ucPdc)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucProt = 0x%x\n", cpu2le32(pEntry->rEntry.ucProt)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucProtRuAlloc = 0x%x\n", cpu2le32(pEntry->rEntry.ucProtRuAlloc)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucFixedRate = 0x%x\n", cpu2le32(pEntry->rEntry.ucFixedRate)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucSuTx = 0x%x\n", cpu2le32(pEntry->rEntry.ucSuTx)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucTpPolicy = 0x%x\n", cpu2le32(pEntry->rEntry.ucTpPolicy)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucTriggerFlow = 0x%x\n", cpu2le32(pEntry->rEntry.ucTriggerFlow)));
}

static VOID muru_get_tx_data_sec_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_TX_DATA_SEC_CTRL pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_TX_DATA_SEC_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg is null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:EVENT_HQA_GET_SU_CALC_LQ\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucBw = 0x%x\n", cpu2le32(pEntry->rEntry.ucBw)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("ucMuPpduDur = 0x%x\n", cpu2le32(pEntry->rEntry.ucMuPpduDur)));

}

static VOID muru_get_trig_data_sec_ctrl_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_TRIG_DATA_SEC_CTRL pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_TRIG_DATA_SEC_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg is null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:EVENT_HQA_GET_SU_CALC_LQ\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ucBaPolicy = 0x%x\n", cpu2le32(pEntry->rEntry.ucBaPolicy)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ucGBABw = 0x%x\n", cpu2le32(pEntry->rEntry.ucGBABw)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ucGBAMuPpduDur = 0x%x\n", cpu2le32(pEntry->rEntry.ucGBAMuPpduDur)));
}

static VOID muru_get_ul_tx_cnt_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_UL_TX_CNT pEntry = NULL;
	INT32 idx = 0;

	pEntry = (P_EVENT_MURU_GET_UL_TX_CNT)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:MURU_EVENT_GET_ULTX_CNT\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("StaCnt = %u\n", pEntry->u1StaCnt));

	for (idx = 0; idx <= pEntry->u1StaCnt; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("WCID: %u, Packet Sent: %u, ok: %u\n",
			idx,
			cpu2le32(pEntry->u4TotSentPktCnt[idx]),
			cpu2le32(pEntry->u4TotOkCnt[idx])));
	}
}

static VOID muru_get_glo_addr_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_EVENT_GET_MURU_GLO_ADDR pEntry = NULL;
	P_EVENT_MURU_GLO pFwGlo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pEntry = (P_EVENT_GET_MURU_GLO_ADDR)rsp_payload;
	pFwGlo = &pEntry->rGloInfo;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:MURU_EVENT_GET_GLO_ADDR\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));

	if (ops->check_muru_glo)
		ops->check_muru_glo(pAd, (VOID *)pFwGlo);
}

static VOID muru_get_mum_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_GET_MUM_CTRL pEntry = NULL;

	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: rsp_payload is null!!\n", __func__));
		return;
	}

	pEntry = (P_EVENT_MURU_GET_MUM_CTRL)rsp_payload;

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: msg is null!!\n", __func__));
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Error !! buffer not specified by cmd\n", __func__));
		return;
	}

	switch (pEntry->u1SubType) {
	case MU_MIMO_GET_PROFILE_ENTRY:
		break;
	case MU_MIMO_GET_GROUP_TBL_ENTRY:
		break;
	case MU_MIMO_GET_CLUSTER_TBL_ENTRY:
		break;
	case MU_MIMO_GET_DL_AC_TABLE:
		break;
	case MU_MIMO_GET_UL_TID_TABLE:
		break;
	default:
		break;
	}
}

static VOID muru_get_spl_cnt_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_EVENT_MURU_GET_SPL_CNT pEntry = NULL;
	INT32 idx = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	pEntry = (P_EVENT_MURU_GET_SPL_CNT)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:MURU_EVENT_GET_SPL_CNT\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("eventId %u\n", pEntry->u4EventId));

	for (idx = 0; idx < wtbl_max_num; idx++) {
		if (pEntry->u2SplCnt[idx]) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("WCID: %u, SPL Count: %u\n",
				idx, pEntry->u2SplCnt[idx]));
		}
	}
}

#ifdef DOT11_HE_AX
VOID muru_tune_ap_muedca_handler(PRTMP_ADAPTER pAd,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT BssIdx;
	UINT8 timer_value = 0;
	struct wifi_dev *wdev = NULL;
	P_EVENT_MURU_TUNE_AP_MUEDCA pEvent = NULL;
	/*RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);*/

	pEvent = (P_EVENT_MURU_TUNE_AP_MUEDCA)rsp_payload;

	timer_value = pEvent->u2MuEdcaSetting;

	if (pAd->CommonCfg.bShowMuEdcaParam == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("EVENT_MURU_TUNE_AP_MUEDCA setting %d tune Band %d BCN MU_EDCA timer to %d\n",
					pEvent->u2MuEdcaSetting, pEvent->u1BandIdx, timer_value));

	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("EVENT_MURU_TUNE_AP_MUEDCA setting %d tune Band %d BCN MU_EDCA timer to %d\n",
				pEvent->u2MuEdcaSetting, pEvent->u1BandIdx, timer_value));
	}

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		if (wdev->bAllowBeaconing && (HcGetBandByWdev(wdev) == pEvent->u1BandIdx)) {
			struct mu_edca_cfg *mu_edca = wlan_config_get_he_mu_edca(wdev);

			if (mu_edca == NULL) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s:(get mu_edca fail\n", __func__));
				return;
			}

			mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer = timer_value;
			mu_edca->mu_ac_rec[ACI_AC_BK].mu_edca_timer = timer_value;
			mu_edca->mu_ac_rec[ACI_AC_VI].mu_edca_timer = timer_value;
			mu_edca->mu_ac_rec[ACI_AC_VO].mu_edca_timer = timer_value;
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		}
	}
}

INT set_muedca_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);
	ULONG Value = os_str_tol(arg, 0, 10);

	chip_cap->mu_edca_timer = (UINT8)Value;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("set chip cap MU_EDCA timer to %d\n",
				chip_cap->mu_edca_timer));

	return TRUE;
}

VOID ShowRateStatToString(
	UCHAR phy_mode,
	UCHAR guard_interval,
	UCHAR rate,
	UCHAR bandwidth,
	UCHAR stbc,
	UCHAR coding,
	UCHAR nss
)
{
	CHAR * phyMode[6] = {"CCK", "OFDM", "HT_MM", "HT_GF", "VHT", "HE"};
	CHAR * bwMode[4] = {"BW20", "BW40", "BW80", "BW160/80+80"};
	CHAR * cck_rate[4] = {"1M", "2M", "5.5M", "11M"};
	CHAR * ofdm_rate[8] = {"48M", "24M", "12M", "6M", "54M", "36M", "18M", "9M"};
	CHAR * cck_preamble[2] = {"Long Preamble", "Short Preamble"};
	CHAR * FecCoding[2] = {"BCC", "LDPC"};
	CHAR * HeGi[4] = {"0.8us", "1.6us", "3.2us", " "};

	if (phy_mode > MODE_HE_MU) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Undefined for %u\n", phy_mode));
		return;
	}

	if (phy_mode == MODE_CCK) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s %s %s %s\n",
				phyMode[phy_mode], cck_rate[(rate & 0x3)],
				cck_preamble[((rate & 0x4) >> 2)], bwMode[bandwidth]));
	} else if (phy_mode == MODE_OFDM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s %s %s\n",
				phyMode[phy_mode], ofdm_rate[rate & 0x7], bwMode[bandwidth]));
	} else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s MCS%u %s %cGI %s %s\n",
				phyMode[phy_mode], rate, bwMode[bandwidth],
				((guard_interval & 0x1) ? 'S' : 'L'),
				((stbc) ? "STBC" : ""), FecCoding[coding]));
	} else if (phy_mode == MODE_VHT) {
		rate = rate & 0xF;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s NSS%u_MCS%u %s %cGI %s %s\n",
			phyMode[phy_mode], nss, rate, bwMode[bandwidth],
			((guard_interval & 0x1) ? 'S' : 'L'),
			((stbc) ? "STBC" : ""), FecCoding[coding]));
	} else {
		rate = rate & 0xF;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s NSS%u_MCS%u %s %sGI %s %s\n",
			phyMode[MODE_HE], nss, rate, bwMode[bandwidth],
			HeGi[guard_interval],
			((stbc) ? "STBC" : ""), FecCoding[coding]));
	}
}

VOID muru_statistic_handler(PRTMP_ADAPTER pAd,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT8 idx;
	P_EVENT_MURU_STAT_MODE_A pEvent = NULL;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	pEvent = (P_EVENT_MURU_STAT_MODE_A)rsp_payload;

    for (idx = 0; idx < MURU_MAX_STA_CNT_PER_EVENT; idx++) {

		P_EVENT_MURU_STAT_PER_WCID prEvt = &pEvent->arRuStatPerWcid[idx];
		P_MURU_STAT_PER_WCID_RECORD prRecord = &prEvt->rRuStat;

		if (prEvt->u2Wcid == 0)
			break;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("wcid: %d au4DlAvgQlenBytes: %d,%d,%d,%d  au4DlLatestQlenBytes: %d,%d,%d,%d\n",
					prEvt->u2Wcid,
					prRecord->au4DlAvgQlenBytes[0],
					prRecord->au4DlAvgQlenBytes[1],
					prRecord->au4DlAvgQlenBytes[2],
					prRecord->au4DlAvgQlenBytes[3],
					prRecord->au4DlLatestQlenBytes[0],
					prRecord->au4DlLatestQlenBytes[1],
					prRecord->au4DlLatestQlenBytes[2],
					prRecord->au4DlLatestQlenBytes[3]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("TX::Mode:%u Mcs:%u Nss:%u Gi:%u Code:%u Per:%u Bw:%u Stbc:%u HeLtf:%u\n",
					prRecord->u1TxMode, prRecord->u1TxMcs, prRecord->u1TxNss,
					prRecord->u1TxGi, prRecord->u1TxCoding, prRecord->u1TxPER,
					prRecord->u1TxBw, prRecord->u1TxStbc, prRecord->u1TxHeLtf));

		ShowRateStatToString(
			prRecord->u1TxMode,
			prRecord->u1TxGi,
			prRecord->u1TxMcs,
			prRecord->u1TxBw,
			prRecord->u1TxStbc,
			prRecord->u1TxCoding,
			prRecord->u1TxNss);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Contention RX::Mode:%u Mcs:%u Nsts:%u Gi:%u Code:%u Per:%u Bw:%u Stbc:%u\n",
					prRecord->u1ContentionRxMode, prRecord->u1ContentionRxMcs, prRecord->u1ContentionRxNsts,
					prRecord->u1ContentionRxGi, prRecord->u1ContentionRxCoding, prRecord->u1ContentionRxPER,
					prRecord->u1ContentionRxBw, prRecord->u1ContentionRxStbc));

		ShowRateStatToString(
			prRecord->u1ContentionRxMode,
			prRecord->u1ContentionRxGi,
			prRecord->u1ContentionRxMcs,
			prRecord->u1ContentionRxBw,
			prRecord->u1ContentionRxStbc,
			prRecord->u1ContentionRxCoding,
			((prRecord->u1ContentionRxNsts + 1) / (prRecord->u1ContentionRxStbc + 1)));

		if (prRecord->u1TrigRxMode >= MODE_HE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Trigger::Mode:%u Mcs:%u Nsts:%u Gi:%u Code:%u Per:%u Bw:%u Stbc:%u\n",
						prRecord->u1TrigRxMode, prRecord->u1TrigRxMcs, prRecord->u1TrigRxNsts,
						prRecord->u1TrigRxGi, prRecord->u1TrigRxCoding, prRecord->u1TrigRxPER,
						prRecord->u1TrigRxBw, prRecord->u1TrigRxStbc));

			ShowRateStatToString(
				prRecord->u1TrigRxMode,
				prRecord->u1TrigRxGi,
				prRecord->u1TrigRxMcs,
				prRecord->u1TrigRxBw,
				prRecord->u1TrigRxStbc,
				prRecord->u1TrigRxCoding,
				((prRecord->u1TrigRxNsts + 1) / (prRecord->u1TrigRxStbc + 1)));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Trigger:: No data\n"));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("TPC:: LastRuIdx:%u UlPwrHeadroom(0.5 dB):%d LastPerUserRssi(0.5 dB):%d\n",
					prRecord->u1LastRuIdx, prRecord->i1UlPwrHeadroom_dB, prRecord->i2LastPerUserRssi_dBm));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("ChBusyPercent[Bit1-Bit8]:: %u-%u-%u-%u-%u-%u-%u-%u\n",
				pEvent->rChBusyPercent.u1Tx1stChBusyPCT,
				pEvent->rChBusyPercent.u1Tx2ndChBusyPCT,
				pEvent->rChBusyPercent.u1Tx3rdChBusyPCT,
				pEvent->rChBusyPercent.u1Tx4thChBusyPCT,
				pEvent->rChBusyPercent.u1Tx5thChBusyPCT,
				pEvent->rChBusyPercent.u1Tx6thChBusyPCT,
				pEvent->rChBusyPercent.u1Tx7thChBusyPCT,
				pEvent->rChBusyPercent.u1Tx8thChBusyPCT));
}

VOID muru_mimo_stat_handler(PRTMP_ADAPTER pAd,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_STAT_MODE_B pEvent = NULL;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __func__));

	pEvent = (P_EVENT_MURU_STAT_MODE_B)rsp_payload;

   /* TBD */

}

VOID muru_dbg_stat_handler(PRTMP_ADAPTER pAd,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MURU_STAT_MODE_C pEvent = NULL;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __func__));

	pEvent = (P_EVENT_MURU_STAT_MODE_C)rsp_payload;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		   ("TxBW[Percent]:: BW80[%u] BW160[%u] BW80S20[%u] BW80S40[%u] BW160S20[%u] BW160S40[%u]\n",
			   pEvent->rMuRuDbgRecord.u1TxBw80PCT,
			   pEvent->rMuRuDbgRecord.u1TxBw160PCT,
			   pEvent->rMuRuDbgRecord.u1TxBw80_Pp_Sec20PCT,
			   pEvent->rMuRuDbgRecord.u1TxBw80_Pp_Sec40PCT,
			   pEvent->rMuRuDbgRecord.u1TxBw160_Pp_Sec20PCT,
			   pEvent->rMuRuDbgRecord.u1TxBw160_Pp_Sec40PCT));
}


#endif /*DOT11_HE_AX*/
static VOID muruEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *)rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

	switch (u4EventId) {
	case MURU_EVENT_GET_BSRP_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_BSRP_CTRL\n", __func__));
		muru_get_bsrp_ctrl_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_GLOBAL_PROT_SEC_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_GLOBAL_PROT_SEC_CTRL\n", __func__));
		muru_get_global_prot_sec_ctrl_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_TX_DATA_SEC_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_TX_DATA_SEC_CTRL\n", __func__));
		muru_get_tx_data_sec_ctrl_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_TRIG_DATA_SEC_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_TRIG_DATA_SEC_CTRL\n", __func__));
		muru_get_trig_data_sec_ctrl_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_HESND_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_HESND_CTRL\n", __func__));
		muru_get_hesnd_ctrl_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_MUM_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_MUM_CTRL\n", __func__));
		muru_get_mum_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_ULTX_CNT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_ULTX_CNT\n", __func__));
		muru_get_ul_tx_cnt_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_SPL_CNT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_SPL_CNT\n", __func__));
		muru_get_spl_cnt_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_GLO_ADDR:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_GLO_ADDR\n", __func__));
		muru_get_glo_addr_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_TXC_TX_STATS:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_TXC_TX_STATS\n", __func__));
		muru_get_txc_tx_stats_handler(msg, pData, len);
		break;
	case MURU_EVENT_GET_FW_BLACKLIST_CTRL:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MURU_EVENT_GET_FW_BLACKLIST_CTRL\n", __func__));
		muru_get_fw_blacklist_ctrl_handler(msg, pData, len);
		break;
	default:
		break;
	}
}

static UINT32 gu4MuruManCfgUsrListDl;
static UINT32 gu4MuruManCfgUsrListUl;

static CMD_MURU_MANCFG_INTERFACER grCmdMuruManCfgInf;

INT32 hqa_muru_parse_cmd_param_dltx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	P_CMD_MURU_MANCFG_INTERFACER pMuruManCfg
)
{
	INT32	status = FALSE;
#if defined(CONFIG_WLAN_SERVICE)
	UINT8	user_idx = 0;
	UINT8	tmpValue = 0;
	INT32	loop_cnt, loop_idx, ru_idx, c26_idx;
	PCHAR	pch = NULL;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpDl;
	UINT32	*pu4ManCfgBmpCmm;
	struct wifi_dev *wdev = NULL;

	P_MURU_DL_MANUAL_CONFIG pCfgDl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;

	pu4UsrList = &gu4MuruManCfgUsrListDl;
	pu4ManCfgBmpDl = &pMuruManCfg->u4ManCfgBmpDl;
	pu4ManCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pCfgDl = &pMuruManCfg->rCfgDl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;

	/* comm_cfg:[Band]:[BW]:[GI]:[LTF]:[total User#]:[VHT/HE]:[SPE] */
	if (strcmp("comm_cfg", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1Band = (UINT8)os_str_tol(pch, 0, 10);

			if (pCfgCmm->u1Band < TESTMODE_BAND_NUM) {
				/* set TXCMD mode */
				wdev = &pAd->ate_wdev[pCfgCmm->u1Band][1];
				pCfgCmm->u1WmmSet = HcGetWmmIdx(pAd, wdev);
			} else {
				status = FALSE;
				goto error;
			}

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_BAND | MURU_FIXED_CMM_WMM_SET);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Bw = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1GI = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Ltf = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(UINT8)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(UINT8)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_DL;
			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			if (strcmp("VHT", pch) == 0)
				pCfgDl->u1TxMode = TX_MODE_VHT;
			else if (strcmp("HE", pch) == 0)
				pCfgDl->u1TxMode = TX_MODE_HE;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1SpeIdx = (UINT8)os_str_tol(pch, 0, 10);

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_SPE_IDX);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_cfg: band=%u, AC=%u, bw=%u, GI=%u, UserCnt=%u, TxMode=%u, SpeIdx=%u\n", __func__,
				pCfgCmm->u1Band, pCfgCmm->u1WmmSet, pCfgDl->u1Bw,
				pCfgDl->u1GI, pCfgDl->u1UserCnt, pCfgDl->u1TxMode,
				pCfgCmm->u1SpeIdx));

		*pu4UsrList = (1 << pCfgDl->u1UserCnt) - 1;
		*pu4ManCfgBmpDl |= (MURU_FIXED_BW | MURU_FIXED_GI | MURU_FIXED_LTF | MURU_FIXED_TOTAL_USER_CNT | MURU_FIXED_TX_MODE);

		status = TRUE;
	}

	/* comm_sigb_cfg:[sigb MCS]:[sigb DCM]:[sigb Compression] */
	if (strcmp("comm_sigb_cfg", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (UINT8)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigBMcs = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
			}
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (UINT8)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigBDcm = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
			}
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (UINT8)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigBCmprs = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_CMPRS;
			}
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_sigb_cfg: sigb mcs=%u, sigb dcm=%u, sigb compress=%u\n", __func__,
				pCfgDl->u1SigBMcs, pCfgDl->u1SigBDcm, pCfgDl->u1SigBDcm));

		status = TRUE;
	}

	/* comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("comm_toneplan", type) == 0) {

		ru_idx = c26_idx = 0;

		switch (pCfgDl->u1Bw) {
		case 0:
			loop_cnt = 1;
			break; /* 20MHz */

		case 1:
			loop_cnt = 2;
			break; /* 40MHz */

		case 2:
			loop_cnt = 5;
			break; /* 80MHz */

		case 3:
			loop_cnt = 10;
			break;/* 160MHz */

		default:
			loop_cnt = 1;
			break;
		}

		for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

			pch = strsep(&val, ":");
			if (pch != NULL) {
				if ((loop_idx % 5) == 4) {
					pCfgDl->au1C26[c26_idx] = (UINT8)os_str_tol(pch, 0, 10);
					c26_idx++;
				} else {
					pCfgDl->au1RU[ru_idx] = (UINT8)os_str_tol(pch, 0, 10);
					ru_idx++;
				}
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_toneplan: RU1=%u,RU2=%u,RU3=%u,RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
				__func__,
				pCfgDl->au1RU[0], pCfgDl->au1RU[1], pCfgDl->au1RU[2], pCfgDl->au1RU[3], pCfgDl->au1C26[0],
				pCfgDl->au1RU[4], pCfgDl->au1RU[5], pCfgDl->au1RU[6], pCfgDl->au1RU[7], pCfgDl->au1C26[1]));

		*pu4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
		status = TRUE;
	}

	/* user:[user #1]:[WLAN_ID]:[RBN]:[RU allocation]:[LDPC]:[Nsts]:[MCS]:[MU group]:[GID]:[UP]:[StartStream]:[MuMimoSpatial]:[AckPol] */
	if (strcmp("user", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			user_idx = (UINT8)(os_str_tol(pch, 0, 10) - 1);
			if (user_idx >= MAX_NUM_TXCMD_USER_INFO)
				goto error;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u2WlanIdx = (UINT16)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocBn = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocIdx = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Ldpc = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Nss = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Mcs = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx = (UINT8)os_str_tol(pch, 0, 10);

			if (pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx > 0)
				*pu4ManCfgBmpDl |= MURU_FIXED_USER_MUMIMO_GRP;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1VhtGid = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1VhtUp = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1HeStartStream = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1HeMuMimoSpatial = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->arUserInfoDl[user_idx].u1AckPolicy = (UINT8)os_str_tol(pch, 0, 10);

			*pu4ManCfgBmpDl |= MURU_FIXED_USER_ACK_POLICY;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=user, user#%u, WlanIdx=%u, RBN=%u, RuAlloc=%u, Ldpc=%u, Nss=%u, Mcs=%u, MuGroup=%u, VhtGid=%u, VhtUp=%u, HeStartStream=%u, HeMuMimoSpatial=%u, AckPolicy=%u\n",
				__func__, user_idx+1,
				pCfgDl->arUserInfoDl[user_idx].u2WlanIdx,
				pCfgDl->arUserInfoDl[user_idx].u1RuAllocBn,
				pCfgDl->arUserInfoDl[user_idx].u1RuAllocIdx,
				pCfgDl->arUserInfoDl[user_idx].u1Ldpc,
				pCfgDl->arUserInfoDl[user_idx].u1Nss,
				pCfgDl->arUserInfoDl[user_idx].u1Mcs,
				pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx,
				pCfgDl->arUserInfoDl[user_idx].u1VhtGid,
				pCfgDl->arUserInfoDl[user_idx].u1VhtUp,
				pCfgDl->arUserInfoDl[user_idx].u1HeStartStream,
				pCfgDl->arUserInfoDl[user_idx].u1HeMuMimoSpatial,
				pCfgDl->arUserInfoDl[user_idx].u1AckPolicy
				));

		*pu4ManCfgBmpDl |= (MURU_FIXED_USER_WLAN_ID | MURU_FIXED_USER_RU_ALLOC | MURU_FIXED_USER_COD | MURU_FIXED_USER_MCS | MURU_FIXED_USER_NSS);
		*pu4UsrList &= ~(1 << user_idx);

		status = TRUE;
	}

error:
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s:(status = %d\n", __func__, status));

	return status;
}

INT32 hqa_muru_parse_cmd_param_ultx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	P_CMD_MURU_MANCFG_INTERFACER pMuruManCfg
)
{
	INT32	status = FALSE;
#if defined(CONFIG_WLAN_SERVICE)
	UINT8	user_idx = 0;
	PCHAR	pch = NULL;
	INT32	loop_idx;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpUl;
	UINT32	*pu4ManCfgBmpCmm;
	struct wifi_dev *wdev = NULL;

	P_MURU_UL_MANUAL_CONFIG pCfgUl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;

	pu4UsrList = &gu4MuruManCfgUsrListUl;
	pu4ManCfgBmpUl = &pMuruManCfg->u4ManCfgBmpUl;
	pu4ManCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pCfgUl = &pMuruManCfg->rCfgUl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;

	/* comm_cfg:[Band]:[BW]:[GI&LTF]:[total User#]*/
	if (strcmp("comm_cfg", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1Band = (UINT8)os_str_tol(pch, 0, 10);

			if (pCfgCmm->u1Band < TESTMODE_BAND_NUM) {
				/* set TXCMD mode */
				wdev = &pAd->ate_wdev[pCfgCmm->u1Band][1];
				pCfgCmm->u1WmmSet = HcGetWmmIdx(pAd, wdev);
			} else {
				status = FALSE;
				goto error;
			}

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_BAND | MURU_FIXED_CMM_WMM_SET);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1UlBw = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1UlGiLtf = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(UINT8)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(UINT8)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_UL;
			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_cfg: Band=%u, AC=%u, UlBw=%u, UlGiLtf=%u, UserCnt=%u\n", __func__,
				pCfgCmm->u1Band, pCfgCmm->u1WmmSet, pCfgUl->u1UlBw,
				pCfgUl->u1UlGiLtf, pCfgUl->u1UserCnt));

		*pu4UsrList = (1 << pCfgUl->u1UserCnt) - 1;
		*pu4ManCfgBmpUl |= (MURU_FIXED_UL_BW | MURU_FIXED_UL_GILTF | MURU_FIXED_UL_TOTAL_USER_CNT);

		status = TRUE;
	}

	/* comm_ta:[00]:[00]:[00]:[00]:[00]:[00] */
	if (strcmp("comm_ta", type) == 0) {

		for (loop_idx = 0 ; loop_idx < MAC_ADDR_LEN ; loop_idx++) {
			pch = strsep(&val, ":");

			if (pch != NULL) {
				pCfgUl->u1TrigTa[loop_idx] = (UINT8)os_str_tol(pch, 0, 16);
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_ta:%02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
				pCfgUl->u1TrigTa[0],
				pCfgUl->u1TrigTa[1],
				pCfgUl->u1TrigTa[2],
				pCfgUl->u1TrigTa[3],
				pCfgUl->u1TrigTa[4],
				pCfgUl->u1TrigTa[5]));

		*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_TA;

		status = TRUE;
	}

	/* ul_trig_cfg:[HE_TRIG cnt]:[HE_TRIG interval] */
	if (strcmp("ul_trig_cfg", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2TrigCnt = (UINT16)os_str_tol(pch, 0, 10);
			if (pCfgUl->u2TrigCnt)
				*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_CNT;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2TrigIntv = (UINT16)os_str_tol(pch, 0, 10);
			if (pCfgUl->u2TrigIntv)
				*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_INTV;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=ul_trig_cfg: TrigCnt=%u, TrigIntv=%u\n", __func__, pCfgUl->u2TrigCnt, pCfgUl->u2TrigIntv));

		status = TRUE;
	}

	/* user:[user #1]:[WLAN_ID]:[RBN]:[RU allocation]:[LDPC]:[Nsts]:[MCS]:[packet size] */
	if (strcmp("user", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			user_idx = (UINT8)(os_str_tol(pch, 0, 10) - 1);
			if (user_idx >= MAX_NUM_TXCMD_USER_INFO)
				goto error;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u2WlanIdx = (UINT16)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocBn = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocIdx = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Ldpc = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Nss = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Mcs = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		if (MURU_MANUAL_CFG_CHK(*pu4ManCfgBmpUl, MURU_FIXED_TRIG_CNT)) {
			pCfgUl->arUserInfoUl[user_idx].u4TrigPktSize = (UINT32)os_str_tol(pch, 0, 10);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=user, user#%u, PktSize=%u\n", __func__, user_idx+1, pCfgUl->arUserInfoUl[user_idx].u4TrigPktSize));

			*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_PKT_SIZE;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=user, user#%u, WlanIdx=%u, RBN=%u, RuAlloc=%u, Ldpc=%u, Nss=%u, Mcs=%u\n",
				__func__, user_idx+1,
				pCfgUl->arUserInfoUl[user_idx].u2WlanIdx,
				pCfgUl->arUserInfoUl[user_idx].u1RuAllocBn,
				pCfgUl->arUserInfoUl[user_idx].u1RuAllocIdx,
				pCfgUl->arUserInfoUl[user_idx].u1Ldpc,
				pCfgUl->arUserInfoUl[user_idx].u1Nss,
				pCfgUl->arUserInfoUl[user_idx].u1Mcs));

		*pu4ManCfgBmpUl |= (MURU_FIXED_USER_UL_WLAN_ID | MURU_FIXED_USER_UL_RU_ALLOC | MURU_FIXED_USER_UL_COD | MURU_FIXED_USER_UL_NSS | MURU_FIXED_USER_UL_MCS);
		*pu4UsrList &= ~(1 << user_idx);

		status = TRUE;
	}

error:
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s:(status = %d\n", __func__, status));

	return status;
}

INT32 muru_parse_cmd_param_muru_manual_config(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	P_CMD_MURU_MANCFG_INTERFACER pMuruManCfg
)
{
	INT32	status = FALSE;
	INT32	loop_cnt, loop_idx, ru_idx, c26_idx;
	PCHAR	pch = NULL;

	P_MURU_DL_MANUAL_CONFIG pCfgDl = NULL;
	P_MURU_UL_MANUAL_CONFIG pCfgUl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;
	PUINT32 pCfgBmpDl, pCfgBmpUl, pCfgBmpCmm;
	PUINT32 pUsrLstDl, pUsrLstUl;

	pCfgDl = &pMuruManCfg->rCfgDl;
	pCfgUl = &pMuruManCfg->rCfgUl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;
	pCfgBmpDl = &pMuruManCfg->u4ManCfgBmpDl;
	pCfgBmpUl = &pMuruManCfg->u4ManCfgBmpUl;
	pCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pUsrLstDl = &gu4MuruManCfgUsrListDl;
	pUsrLstUl = &gu4MuruManCfgUsrListUl;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:\n", __func__));

	/********** Common **********/
	/* global_comm_band */
	if (strcmp("global_comm_band", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgCmm->u1Band = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=global_comm_band: %u\n", pCfgCmm->u1Band));

		*pCfgBmpCmm |= MURU_FIXED_CMM_BAND;

		status = TRUE;
	}

	/* global_comm_wmm */
		if (strcmp("global_comm_wmm", type) == 0) {
			pch = strsep(&val, ":");
			if (pch != NULL)
				pCfgCmm->u1WmmSet = (UINT8)os_str_tol(pch, 0, 10);
			else {
				status = FALSE;
				goto error;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=global_comm_wmm: %u\n", pCfgCmm->u1WmmSet));

			*pCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;

			status = TRUE;
		}


	/* dl_comm_bw */
	if (strcmp("dl_comm_bw", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Bw = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_bw: %u\n", pCfgDl->u1Bw));

		*pCfgBmpDl |= MURU_FIXED_BW;

		status = TRUE;
	}

	/* dl_comm_user_cnt */
	if (strcmp("dl_comm_user_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(UINT8)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(UINT8)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_DL;
			*pCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_user_cnt: %u\n", pCfgDl->u1UserCnt));

		*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;

		status = TRUE;
	}

	/* dl_user_wlan_idx */
	if (strcmp("dl_user_wlan_idx", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u2WlanIdx = (UINT16)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("cmd=dl_user_wlan_idx: user %u, value=%u\n", loop_idx + 1, pCfgDl->arUserInfoDl[loop_idx].u2WlanIdx));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpDl |= MURU_FIXED_USER_WLAN_ID;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_wlan_idx: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("dl_comm_toneplan", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_BW)) {
			ru_idx = c26_idx = 0;

			switch (pCfgDl->u1Bw) {
			case 0:
				loop_cnt = 1;
				break; /* 20MHz */

			case 1:
				loop_cnt = 2;
				break; /* 40MHz */

			case 2:
				loop_cnt = 5;
				break; /* 80MHz */

			case 3:
				loop_cnt = 10;
				break; /* 160MHz */

			default:
				loop_cnt = 1;
				break;
			}

			for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

				pch = strsep(&val, ":");
				if (pch != NULL) {
					if ((loop_idx % 5) == 4) {
						pCfgDl->au1C26[c26_idx] = (UINT8)os_str_tol(pch, 0, 10);
						c26_idx++;
					} else {
						pCfgDl->au1RU[ru_idx] = (UINT8)os_str_tol(pch, 0, 10);
						ru_idx++;
					}
				} else {
					status = FALSE;
					goto error;
				}
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_comm_toneplan: RU1=%u,RU2=%u,RU3=%u,RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
					pCfgDl->au1RU[0], pCfgDl->au1RU[1], pCfgDl->au1RU[2], pCfgDl->au1RU[3], pCfgDl->au1C26[0],
					pCfgDl->au1RU[4], pCfgDl->au1RU[5], pCfgDl->au1RU[6], pCfgDl->au1RU[7], pCfgDl->au1C26[1]));

			*pCfgBmpDl |= MURU_FIXED_TONE_PLAN;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_comm_toneplan: set dl_comm_bw before config dl_comm_toneplan\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_ack_polocy */
	if (strcmp("dl_user_ack_policy", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u1AckPolicy = (UINT8)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("cmd=dl_user_ack_policy: user %u, value=%u\n", loop_idx + 1, pCfgDl->arUserInfoDl[loop_idx].u1AckPolicy));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpDl |= MURU_FIXED_USER_ACK_POLICY;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_ack_policy: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_ru_alloc :[RBN]:[RU alloc]*/
	if (strcmp("dl_user_ru_alloc", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgDl->arUserInfoDl[loop_idx].u1RuAllocBn = (UINT8)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgDl->arUserInfoDl[loop_idx].u1RuAllocIdx = (UINT8)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("cmd=dl_user_ru_alloc:[RBN]:[RU alloc]= user %u, RBN=%u, RU alloc idx=%u\n", loop_idx + 1,
				pCfgDl->arUserInfoDl[loop_idx].u1RuAllocBn, pCfgDl->arUserInfoDl[loop_idx].u1RuAllocIdx));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_RU_ALLOC;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_ru_alloc: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/********** Uplink **********/

	/* ul_comm_user_cnt */
	if (strcmp("ul_comm_user_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(UINT8)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(UINT8)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_UL;
			*pCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_user_cnt: %u\n", pCfgUl->u1UserCnt));

		*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;

		status = TRUE;
	}

	/* ul_comm_ack_type */
	if (strcmp("ul_comm_ack_type", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1BaType = (UINT8)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_ack_type: %u\n", pCfgUl->u1BaType));

		*pCfgBmpUl |= MURU_FIXED_UL_ACK_TYPE;

		status = TRUE;
	}

	/* ul_comm_trig_intv */
	if (strcmp("ul_comm_trig_intv", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u2TrigIntv = (UINT16)os_str_tol(pch, 0, 10);

		if (pCfgUl->u2TrigIntv) {
			*pCfgBmpUl |= MURU_FIXED_TRIG_INTV;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_trig_intv: %u\n", pCfgUl->u2TrigIntv));

		status = TRUE;
	}

	/* ul_comm_trig_cnt */
	if (strcmp("ul_comm_trig_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u2TrigCnt = (UINT16)os_str_tol(pch, 0, 10);

		if (pCfgUl->u2TrigCnt) {
			*pCfgBmpUl |= MURU_FIXED_TRIG_CNT;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_trig_cnt: %u\n", pCfgUl->u2TrigCnt));

		status = TRUE;
	}

	/* ul_comm_trig_type */
	if (strcmp("ul_comm_trig_type", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1TrigType = (UINT8)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_trig_type: %u\n", pCfgUl->u1TrigType));

		*pCfgBmpUl |= MURU_FIXED_TRIG_TYPE;

		status = TRUE;
	}

	/* ul_comm_ta:[00]:[00]:[00]:[00]:[00]:[00] */
	if (strcmp("ul_comm_ta", type) == 0) {

		for (loop_idx = 0 ; loop_idx < MAC_ADDR_LEN ; loop_idx++) {
			pch = strsep(&val, ":");

			if (pch != NULL) {
				pCfgUl->u1TrigTa[loop_idx] = (UINT8)os_str_tol(pch, 0, 16);
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=comm_ta:%02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
				pCfgUl->u1TrigTa[0],
				pCfgUl->u1TrigTa[1],
				pCfgUl->u1TrigTa[2],
				pCfgUl->u1TrigTa[3],
				pCfgUl->u1TrigTa[4],
				pCfgUl->u1TrigTa[5]));

		*pCfgBmpUl |= MURU_FIXED_TRIG_TA;

		status = TRUE;
	}

	/* ul_comm_bw */
	if (strcmp("ul_comm_bw", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UlBw = (UINT8)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_bw: %u\n", pCfgUl->u1UlBw));

		*pCfgBmpUl |= MURU_FIXED_UL_BW;

		status = TRUE;
	}

	/* ul_comm_gi_ltf */
	if (strcmp("ul_comm_gi_ltf", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UlGiLtf = (UINT8)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_gi_ltf: %u\n", pCfgUl->u1UlGiLtf));

		*pCfgBmpUl |= MURU_FIXED_UL_GILTF;

		status = TRUE;
	}

	/* ul_comm_length */
	if (strcmp("ul_comm_length", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2UlLength = (UINT16)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_length: %u\n", pCfgUl->u2UlLength));

		*pCfgBmpUl |= MURU_FIXED_UL_LENGTH;

		status = TRUE;
	}

	/* ul_comm_tf_pad */
	if (strcmp("ul_comm_tf_pad", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;
			} else {
				pCfgUl->u1TfPad = (UINT8)os_str_tol(pch, 0, 10);
			}
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_tf_pad: %u\n", pCfgUl->u1TfPad));

		*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;

		status = TRUE;
	}

    /* HETB RX Debug: ul_comm_rx_hetb_cfg1 */
	if (strcmp("ul_comm_rx_hetb_cfg1", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->rx_hetb_cfg[0] = (UINT32)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_rx_hetb_cfg1: %u\n", pCfgUl->rx_hetb_cfg[0]));

		*pCfgBmpUl |= MURU_FIXED_RX_HETB_CFG1;

		status = TRUE;
	}

	 /* HETB RX Debug:ul_comm_rx_hetb_cfg2 */
	if (strcmp("ul_comm_rx_hetb_cfg2", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->rx_hetb_cfg[1] = (UINT32)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_rx_hetb_cfg2: %u\n", pCfgUl->rx_hetb_cfg[1]));

		*pCfgBmpUl |= MURU_FIXED_RX_HETB_CFG2;

		status = TRUE;
	}

	/* ul_user_wlan_idx */
	if (strcmp("ul_user_wlan_idx", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u2WlanIdx = (UINT16)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_wlan_idx: user %u, value=%u\n", loop_idx + 1, pCfgUl->arUserInfoUl[loop_idx].u2WlanIdx));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_WLAN_ID;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_wlan_idx: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}
	/* ul_user_cod */
	if (strcmp("ul_user_cod", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Ldpc = (UINT8)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_cod: user %u, value=%u\n", loop_idx + 1, pCfgUl->arUserInfoUl[loop_idx].u1Ldpc));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_COD;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_cod: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_mcs */
	if (strcmp("ul_user_mcs", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Mcs = (UINT8)os_str_tol(pch, 0, 10);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_mcs: user %u, value=%u\n", loop_idx + 1, pCfgUl->arUserInfoUl[loop_idx].u1Mcs));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_MCS;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_mcs: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_ssAlloc_raru */
	if (strcmp("ul_user_ssAlloc_raru", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Nss = (UINT8)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_ssAlloc_raru: user %u, value=%u\n", loop_idx + 1, pCfgUl->arUserInfoUl[loop_idx].u1Nss));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_NSS;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_ssAlloc_raru: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_rssi */
	if (strcmp("ul_user_rssi", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1TargetRssi = (UINT8)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_rssi: user %u, value=%u\n", loop_idx + 1, pCfgUl->arUserInfoUl[loop_idx].u1TargetRssi));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_TARGET_RSSI;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_rssi: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("ul_comm_toneplan", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_BW)) {
			ru_idx = c26_idx = 0;

			switch (pCfgUl->u1UlBw) {
			case 0:
				loop_cnt = 1;
				break; /* 20MHz */

			case 1:
				loop_cnt = 2;
				break; /* 40MHz */

			case 2:
				loop_cnt = 5;
				break; /* 80MHz */

			case 3:
				loop_cnt = 10;
				break; /* 160MHz */

			default:
				loop_cnt = 1;
				break;
			}

			for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

				pch = strsep(&val, ":");
				if (pch != NULL) {
					if ((loop_idx % 5) == 4) {
						pCfgUl->au1UlC26[c26_idx] = (UINT8)os_str_tol(pch, 0, 10);
						c26_idx++;
					} else {
						pCfgUl->au1UlRU[ru_idx] = (UINT8)os_str_tol(pch, 0, 10);
						ru_idx++;
					}
				} else {
					status = FALSE;
					goto error;
				}
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_comm_toneplan: RU1=%u,RU2=%u,RU3=%u,RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
					pCfgUl->au1UlRU[0], pCfgUl->au1UlRU[1], pCfgUl->au1UlRU[2], pCfgUl->au1UlRU[3], pCfgUl->au1UlC26[0],
					pCfgUl->au1UlRU[4], pCfgUl->au1UlRU[5], pCfgUl->au1UlRU[6], pCfgUl->au1UlRU[7], pCfgUl->au1UlC26[1]));

			*pCfgBmpUl |= MURU_FIXED_TONE_PLAN;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_comm_toneplan: set dl_comm_bw before config dl_comm_toneplan\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_ru_alloc :[RBN]:[RU alloc]*/
	if (strcmp("ul_user_ru_alloc", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgUl->arUserInfoUl[loop_idx].u1RuAllocBn = (UINT8)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgUl->arUserInfoUl[loop_idx].u1RuAllocIdx = (UINT8)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_ru_alloc:[RBN]:[RU alloc]= user %u, RBN=%u, RU alloc idx=%u\n", loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u1RuAllocBn, pCfgUl->arUserInfoUl[loop_idx].u1RuAllocIdx));
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_RU_ALLOC;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_ru_alloc: set ul_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_rx_nonsf_en_bitmap */
	if (strcmp("ul_user_rx_nonsf_en_bitmap", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->rx_hetb_nonsf_en_bitmap = (UINT32)os_str_tol(pch, 0, 10);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_rx_nonsf_en_bitmap: value=0x%x\n", pCfgUl->rx_hetb_nonsf_en_bitmap));
		} else {
			status = FALSE;
			goto error;
		}
		*pCfgBmpUl |= MURU_FIXED_NONSF_EN_BITMAP;
		status = TRUE;
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s:(status = %d\n", __func__, status));

	return status;
}

INT32 hqa_muru_set_dl_tx_muru_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpDl;
	UINT32	*pu4ManCfgBmpCmm;

	P_MURU_DL_MANUAL_CONFIG pCfgDl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;

	pu4UsrList = &gu4MuruManCfgUsrListDl;
	pu4ManCfgBmpDl = &grCmdMuruManCfgInf.u4ManCfgBmpDl;
	pu4ManCfgBmpCmm = &grCmdMuruManCfgInf.u4ManCfgBmpCmm;
	pCfgDl = &grCmdMuruManCfgInf.rCfgDl;
	pCfgCmm = &grCmdMuruManCfgInf.rCfgCmm;

	param_type = arg;
	if (strlen(param_type)) {

		if (strcmp("init", param_type) == 0) {

			/* init */
			*pu4UsrList = 0;
			*pu4ManCfgBmpDl = 0;
			*pu4ManCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_DL;
			os_zero_mem(pCfgDl, sizeof(MURU_DL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgDl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pu4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (UINT8)(pAd->CommonCfg.HE_PpduFmt);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (UINT8)(pAd->CommonCfg.HE_OfdmaSchType);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {

			/* update */
			if (*pu4UsrList != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s:cmd=update, target_updated_user_bmp=0x%x, not_yet_updated_user_bmp:0x%x\n", __func__,
						 ((1 << pCfgDl->u1UserCnt) - 1), *pu4UsrList));
			} else {
				Ret = wifi_test_muru_set_manual_config(pAd, &grCmdMuruManCfgInf);
			}
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = hqa_muru_parse_cmd_param_dltx(pAd, param_type, param_val, &grCmdMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n", __func__, param_type, Ret));
	return Ret;
}

INT32 hqa_muru_set_ul_tx_muru_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpUl;
	UINT32	*pu4ManCfgBmpCmm;

	P_MURU_UL_MANUAL_CONFIG pCfgUl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;

	pu4UsrList = &gu4MuruManCfgUsrListUl;
	pu4ManCfgBmpUl = &grCmdMuruManCfgInf.u4ManCfgBmpUl;
	pu4ManCfgBmpCmm = &grCmdMuruManCfgInf.u4ManCfgBmpCmm;
	pCfgUl = &grCmdMuruManCfgInf.rCfgUl;
	pCfgCmm = &grCmdMuruManCfgInf.rCfgCmm;

	param_type = arg;
	if (strlen(param_type)) {

		if (strcmp("init", param_type) == 0) {
			/* init */
			*pu4UsrList = 0;
			*pu4ManCfgBmpUl = 0;
			*pu4ManCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_UL;
			os_zero_mem(pCfgUl, sizeof(MURU_UL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgUl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pu4ManCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (UINT8)(pAd->CommonCfg.HE_PpduFmt);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (UINT8)(pAd->CommonCfg.HE_OfdmaSchType);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;

				*pu4ManCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {

			if (*pu4UsrList != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s:cmd=update, target_updated_user_bmp=0x%x, not_yet_updated_user_bmp:0x%x\n", __func__,
						 ((1 << pCfgUl->u1UserCnt) - 1), *pu4UsrList));
			} else {
				Ret = wifi_test_muru_set_manual_config(pAd, &grCmdMuruManCfgInf);
			}
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = hqa_muru_parse_cmd_param_ultx(pAd, param_type, param_val, &grCmdMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n", __func__, param_type, Ret));
	return Ret;
}

INT32 hqa_muru_set_ul_tx_trigger(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	BOOLEAN IsUlTxTrigger = 0;

	if (arg != NULL)
		IsUlTxTrigger = (BOOLEAN)os_str_toul(arg, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Argument is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: MU %s %u\n", __func__, IsUlTxTrigger == 1 ?
		 "Enable":"Disable", IsUlTxTrigger));

	Ret = wifi_test_muru_ul_tx_trigger(pAd, IsUlTxTrigger);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: CMD %s\n", __func__, Ret ? "Success":"Fail"));
	return Ret;
}

INT32 hqa_muru_reset_ul_tx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	BOOLEAN fgRst = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_ULTX_CNT_RESET;

	if (arg != NULL)
		fgRst = (BOOLEAN)os_str_toul(arg, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Argument is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(fgRst));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: msg is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&fgRst, sizeof(fgRst));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: CMD %s\n", __func__, Ret ? "Success":"Fail"));
	return Ret;
}

INT32 hqa_muru_get_ul_tx_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MURU_TRIG_DATA_SEC_CTRL result = {0};
	UINT32 cmd = MURU_GET_ULTX_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("Index is: %d\n", index));
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));
	return Ret;
}

INT32 hqa_muru_set_mu_tx_pkt_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 *value = NULL;
	CMD_MURU_SET_MU_TX_PKT_CNT rSetMuTxPktEn;

	os_zero_mem(&rSetMuTxPktEn, sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));

	value = strsep(&arg, ":");
	if (value == NULL)
		return 0;

	rSetMuTxPktEn.u1BandIdx = simple_strtol(value, 0, 10);

	value = strsep(&arg, "");
	if (value == NULL)
		return 0;

	rSetMuTxPktEn.u1MuTxEn = simple_strtol(value, 0, 10);

	Ret = set_muru_mu_tx_pkt_en(pAd, &rSetMuTxPktEn);

	return Ret;
}

INT32 hqa_muru_set_mu_tx_pkt_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 *value = NULL;
	CMD_MURU_SET_MU_TX_PKT_CNT rSetMuTxPktCnt;

	os_zero_mem(&rSetMuTxPktCnt, sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));

	value = strsep(&arg, ":");
	rSetMuTxPktCnt.u1BandIdx = simple_strtol(value, 0, 10);

	value = strsep(&arg, "");
	rSetMuTxPktCnt.u4MuTxPktCnt = simple_strtol(value, 0, 10);

	Ret = set_muru_mu_tx_pkt_cnt(pAd, &rSetMuTxPktCnt);

	return Ret;
}

INT32 set_muru_mu_tx_pkt_en(
	RTMP_ADAPTER *pAd,
	P_CMD_MURU_SET_MU_TX_PKT_CNT prSetMuTxPktEn)
{
	INT32 Ret = TRUE;
	UINT32 cmd = MURU_SET_MU_TX_PKT_EN;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: u1BandIdx = %d, u1MuTxEn = %d\n"
			, __func__, prSetMuTxPktEn->u1BandIdx, prSetMuTxPktEn->u1MuTxEn));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	prSetMuTxPktEn->u4MuTxPktCnt = cpu2le32(prSetMuTxPktEn->u4MuTxPktCnt);

#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)prSetMuTxPktEn, sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Ret = %d\n", __func__, Ret));
	return Ret;
}

INT32 set_muru_mu_tx_pkt_cnt(
	RTMP_ADAPTER *pAd,
	P_CMD_MURU_SET_MU_TX_PKT_CNT prSetMuTxPktCnt)
{
	INT32 Ret = TRUE;
	UINT32 cmd = MURU_SET_MU_TX_PKT_CNT;
	CMD_MURU_SET_MU_TX_PKT_CNT rSetParam;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&rSetParam, sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));

	rSetParam.u1BandIdx = prSetMuTxPktCnt->u1BandIdx;
	rSetParam.u4MuTxPktCnt = cpu2le32(prSetMuTxPktCnt->u4MuTxPktCnt);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: u1BandIdx = %d, u4MuTxPktCnt = %d\n"
			, __func__, rSetParam.u1BandIdx, rSetParam.u4MuTxPktCnt));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&rSetParam, sizeof(CMD_MURU_SET_MU_TX_PKT_CNT));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, Ret));
	return Ret;
}

INT32 set_muru_mudl_ack_policy(RTMP_ADAPTER *ad, UINT8 policy_num)
{
	INT32 ret = TRUE;
	UINT32 cmd = MURU_SET_MUDL_ACK_POLICY;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 ack_policy = policy_num;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(ad, sizeof(cmd) + sizeof(ack_policy));
	if (!msg) {
		ret = 0;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&ack_policy, sizeof(ack_policy));
	AndesSendCmdMsg(ad, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, ret));
	return ret;
}

INT32 set_muru_trig_type(RTMP_ADAPTER *ad, UINT8 type)
{
	INT32 ret = TRUE;
	UINT32 cmd = MURU_SET_TRIG_TYPE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 trig_type = type;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(ad, sizeof(cmd) + sizeof(trig_type));
	if (!msg) {
		ret = 0;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&trig_type, sizeof(trig_type));
	AndesSendCmdMsg(ad, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, ret));
	return ret;
}

INT32 set_muru_cert_muedca_override(RTMP_ADAPTER *ad, UINT8 capi_override)
{
	INT32 ret = TRUE;
	UINT32 cmd = MURU_SET_CERT_MU_EDCA_OVERRIDE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 override = (capi_override) ? TRUE : FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: override = %d\n", __func__, override));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(ad, sizeof(cmd) + sizeof(override));
	if (!msg) {
		ret = 0;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&override, sizeof(override));
	AndesSendCmdMsg(ad, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, ret));
	return ret;
}

INT32 set_muru_manual_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	P_MURU_DL_MANUAL_CONFIG pCfgDl = NULL;
	P_MURU_UL_MANUAL_CONFIG pCfgUl = NULL;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = NULL;
	PUINT32 pCfgBmpDl, pCfgBmpUl, pCfgBmpCmm;
	PUINT32 pUsrLstDl, pUsrLstUl;

	pCfgDl = &grCmdMuruManCfgInf.rCfgDl;
	pCfgUl = &grCmdMuruManCfgInf.rCfgUl;
	pCfgCmm = &grCmdMuruManCfgInf.rCfgCmm;
	pCfgBmpDl = &grCmdMuruManCfgInf.u4ManCfgBmpDl;
	pCfgBmpUl = &grCmdMuruManCfgInf.u4ManCfgBmpUl;
	pCfgBmpCmm = &grCmdMuruManCfgInf.u4ManCfgBmpCmm;
	pUsrLstDl = &gu4MuruManCfgUsrListDl;
	pUsrLstUl = &gu4MuruManCfgUsrListUl;

	param_type = arg;
	if (strlen(param_type)) {
		if (strcmp("dl_init", param_type) == 0) {
			/* dl_init */
			*pUsrLstDl = 0;
			*pCfgBmpDl = 0;

			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_DL;
			*pCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			os_zero_mem(pCfgDl, sizeof(MURU_DL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgDl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (UINT8)(pAd->CommonCfg.HE_PpduFmt);
				*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (UINT8)(pAd->CommonCfg.HE_OfdmaSchType);
				*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			Ret = TRUE;
		} else if (strcmp("ul_init", param_type) == 0) {
			/* ul_init */
			*pUsrLstUl = 0;
			*pCfgBmpUl = 0;

			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_UL;
			*pCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			os_zero_mem(pCfgUl, sizeof(MURU_UL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgUl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (UINT8)(pAd->CommonCfg.HE_PpduFmt);
				*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (UINT8)(pAd->CommonCfg.HE_OfdmaSchType);
				*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;

				*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {
			/* update */
			Ret = wifi_test_muru_set_manual_config(pAd, &grCmdMuruManCfgInf);
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = muru_parse_cmd_param_muru_manual_config(pAd, param_type, param_val, &grCmdMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n", __func__, param_type, Ret));
	return Ret;
}

INT32 set_muru_debug_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	CMD_MURU_MANCFG_INTERFACER *param;
	INT32 loop_idx;

	os_alloc_mem(pAd, (UCHAR **)&param, sizeof(CMD_MURU_MANCFG_INTERFACER));
	if (param == NULL) {
		Ret = FALSE;
		return Ret;
	}
	os_move_mem(param, &grCmdMuruManCfgInf, sizeof(CMD_MURU_MANCFG_INTERFACER));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("gu4MuruManCfgUsrListDl: 0x%x\n", gu4MuruManCfgUsrListDl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("gu4MuruManCfgUsrListUl: 0x%x\n", gu4MuruManCfgUsrListUl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("----- CMM PROFILE -----\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4ManCfgBmpCmm: 0x%x\n", param->u4ManCfgBmpCmm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgCmm.u1PpduFmt: %u\n", param->rCfgCmm.u1PpduFmt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgCmm.u1SchType: %u\n", param->rCfgCmm.u1SchType));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgCmm.u1Band: %u\n", param->rCfgCmm.u1Band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgCmm.u1WmmSet: %u\n", param->rCfgCmm.u1WmmSet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgCmm.u1SpeIdx: %u\n", param->rCfgCmm.u1SpeIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("----- DL TX CONFIG -----\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4ManCfgBmpDl: 0x%x\n", param->u4ManCfgBmpDl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1UserCnt: %u\n", param->rCfgDl.u1UserCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1TxMode: %u\n", param->rCfgDl.u1TxMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1Bw: %u\n", param->rCfgDl.u1Bw));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1GI: %u\n", param->rCfgDl.u1GI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1Ltf: %u\n", param->rCfgDl.u1Ltf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1SigBMcs: %u\n", param->rCfgDl.u1SigBMcs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1SigBDcm: %u\n", param->rCfgDl.u1SigBDcm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.u1SigBCmprs: %u\n", param->rCfgDl.u1SigBCmprs));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgDl.toneplan: RU1=%u,RU2=%u,RU3=%u,RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
			 param->rCfgDl.au1RU[0], param->rCfgDl.au1RU[1], param->rCfgDl.au1RU[2], param->rCfgDl.au1RU[3], param->rCfgDl.au1C26[0],
			 param->rCfgDl.au1RU[4], param->rCfgDl.au1RU[5], param->rCfgDl.au1RU[6], param->rCfgDl.au1RU[7], param->rCfgDl.au1C26[1]));

	for (loop_idx = 0; loop_idx < param->rCfgDl.u1UserCnt; loop_idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("User:%u, WlanIdx:%u, RBN:%u, RBIdx:%u, LDPC:%u, Nsts:%u, MCS:%u, MUGrpIdx:%u, VhtGid:%u, VhtUp:%u, HeStartStream:%u, HeMuMimoSpatial:%u, AckPolicy:%u\n",
				 loop_idx+1, param->rCfgDl.arUserInfoDl[loop_idx].u2WlanIdx, param->rCfgDl.arUserInfoDl[loop_idx].u1RuAllocBn, param->rCfgDl.arUserInfoDl[loop_idx].u1RuAllocIdx,
				 param->rCfgDl.arUserInfoDl[loop_idx].u1Ldpc, param->rCfgDl.arUserInfoDl[loop_idx].u1Nss, param->rCfgDl.arUserInfoDl[loop_idx].u1Mcs,
				 param->rCfgDl.arUserInfoDl[loop_idx].u1MuGroupIdx, param->rCfgDl.arUserInfoDl[loop_idx].u1VhtGid, param->rCfgDl.arUserInfoDl[loop_idx].u1VhtUp,
				 param->rCfgDl.arUserInfoDl[loop_idx].u1HeStartStream, param->rCfgDl.arUserInfoDl[loop_idx].u1HeMuMimoSpatial, param->rCfgDl.arUserInfoDl[loop_idx].u1AckPolicy));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("----- UL TX CONFIG -----\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4ManCfgBmpUl: 0x%x\n", param->u4ManCfgBmpUl));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1UserCnt: %u\n", param->rCfgUl.u1UserCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1TrigType: %u\n", param->rCfgUl.u1TrigType));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u2TrigCnt: %u\n", param->rCfgUl.u2TrigCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u2TrigIntv: %u\n", param->rCfgUl.u2TrigIntv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1TrigTa: %02x:%02x:%02x:%02x:%02x:%02x\n",
					param->rCfgUl.u1TrigTa[0], param->rCfgUl.u1TrigTa[1], param->rCfgUl.u1TrigTa[2],
					param->rCfgUl.u1TrigTa[3], param->rCfgUl.u1TrigTa[4], param->rCfgUl.u1TrigTa[5]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1UlBw: %u\n", param->rCfgUl.u1UlBw));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1UlGiLtf: %u\n", param->rCfgUl.u1UlGiLtf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u2UlLength: %u\n", param->rCfgUl.u2UlLength));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rCfgUl.u1TfPad: %u\n", param->rCfgUl.u1TfPad));

	for (loop_idx = 0; loop_idx < param->rCfgUl.u1UserCnt; loop_idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("User:%u, WlanIdx:%u, RBN:%u, RBIdx:%u, LDPC:%u, Nsts:%u, MCS:%u, TargetRSSI:%u, PktSize=%u\n",
				 loop_idx+1, param->rCfgUl.arUserInfoUl[loop_idx].u2WlanIdx, param->rCfgUl.arUserInfoUl[loop_idx].u1RuAllocBn, param->rCfgUl.arUserInfoUl[loop_idx].u1RuAllocIdx,
				 param->rCfgUl.arUserInfoUl[loop_idx].u1Ldpc, param->rCfgUl.arUserInfoUl[loop_idx].u1Nss, param->rCfgUl.arUserInfoUl[loop_idx].u1Mcs,
				 param->rCfgUl.arUserInfoUl[loop_idx].u1TargetRssi, param->rCfgUl.arUserInfoUl[loop_idx].u4TrigPktSize));
	}

	os_free_mem(param);

	if (arg) {
	UINT32 input = os_str_tol(arg, 0, 10);
	UINT32 cmd = MURU_SET_FW_DUMP_MANCFG;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 fw_dump_on = (input) ? TRUE : FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: fw dump = %s\n", __func__, (fw_dump_on)?"TRUE":"FALSE"));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(fw_dump_on));
	if (!msg) {
	    Ret = FALSE;
	    goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&fw_dump_on, sizeof(fw_dump_on));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Ret = %d\n", __func__, Ret));
	}
	return Ret;
}


INT32 set_disable_contention_tx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_DIS_CNT_TX;
	UINT8 DisConTx = 0;

	DisConTx = os_str_tol(arg, 0, 10);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(DisConTx));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&DisConTx, sizeof(DisConTx));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


INT32 wifi_test_muru_set_manual_config(
	PRTMP_ADAPTER pAd,
	P_CMD_MURU_MANCFG_INTERFACER pMuruManCfg
)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_MANUAL_CONFIG;
	CMD_MURU_MANCFG_INTERFACER param = {0};
	INT32 loop_idx;

	param.u4ManCfgBmpCmm		= (UINT32)(cpu2le32(pMuruManCfg->u4ManCfgBmpCmm));
	param.rCfgCmm.u1PpduFmt		= pMuruManCfg->rCfgCmm.u1PpduFmt;
	param.rCfgCmm.u1SchType		= pMuruManCfg->rCfgCmm.u1SchType;
	param.rCfgCmm.u1Band		= pMuruManCfg->rCfgCmm.u1Band;
	param.rCfgCmm.u1WmmSet		= pMuruManCfg->rCfgCmm.u1WmmSet;
	param.rCfgCmm.u1SpeIdx		= pMuruManCfg->rCfgCmm.u1SpeIdx;

	/* DL */
	param.u4ManCfgBmpDl			= (UINT32)(cpu2le32(pMuruManCfg->u4ManCfgBmpDl));
	param.rCfgDl.u1UserCnt		= pMuruManCfg->rCfgDl.u1UserCnt;
	param.rCfgDl.u1TxMode		= pMuruManCfg->rCfgDl.u1TxMode;
	param.rCfgDl.u1Bw			= pMuruManCfg->rCfgDl.u1Bw;
	param.rCfgDl.u1GI			= pMuruManCfg->rCfgDl.u1GI;
	param.rCfgDl.u1Ltf			= pMuruManCfg->rCfgDl.u1Ltf;
	param.rCfgDl.u1SigBMcs		= pMuruManCfg->rCfgDl.u1SigBMcs;
	param.rCfgDl.u1SigBDcm		= pMuruManCfg->rCfgDl.u1SigBDcm;
	param.rCfgDl.u1SigBCmprs	= pMuruManCfg->rCfgDl.u1SigBCmprs;

	for (loop_idx = 0; loop_idx < 8; loop_idx++) {
		param.rCfgDl.au1RU[loop_idx] = pMuruManCfg->rCfgDl.au1RU[loop_idx];
	}

	for (loop_idx = 0; loop_idx < 2; loop_idx++) {
		param.rCfgDl.au1C26[loop_idx]	= pMuruManCfg->rCfgDl.au1C26[loop_idx];
	}

	for (loop_idx = 0; loop_idx < param.rCfgDl.u1UserCnt; loop_idx++) {
		param.rCfgDl.arUserInfoDl[loop_idx].u2WlanIdx		= (UINT16)(cpu2le16(pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u2WlanIdx));
		param.rCfgDl.arUserInfoDl[loop_idx].u1RuAllocBn		= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1RuAllocBn;
		param.rCfgDl.arUserInfoDl[loop_idx].u1RuAllocIdx	= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1RuAllocIdx;
		param.rCfgDl.arUserInfoDl[loop_idx].u1Ldpc			= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1Ldpc;
		param.rCfgDl.arUserInfoDl[loop_idx].u1Nss			= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1Nss;
		param.rCfgDl.arUserInfoDl[loop_idx].u1Mcs			= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1Mcs;
		param.rCfgDl.arUserInfoDl[loop_idx].u1MuGroupIdx	= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1MuGroupIdx;
		param.rCfgDl.arUserInfoDl[loop_idx].u1VhtGid		= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1VhtGid;
		param.rCfgDl.arUserInfoDl[loop_idx].u1VhtUp			= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1VhtUp;
		param.rCfgDl.arUserInfoDl[loop_idx].u1AckPolicy		= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1AckPolicy;
		param.rCfgDl.arUserInfoDl[loop_idx].u2TxPwrAlpha	= (UINT16)(cpu2le16(pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u2TxPwrAlpha));
		param.rCfgDl.arUserInfoDl[loop_idx].u1HeStartStream		= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1HeStartStream;
		param.rCfgDl.arUserInfoDl[loop_idx].u1HeMuMimoSpatial	= pMuruManCfg->rCfgDl.arUserInfoDl[loop_idx].u1HeMuMimoSpatial;
	}

	/* UL */
	param.u4ManCfgBmpUl			= (UINT32)(cpu2le32(pMuruManCfg->u4ManCfgBmpUl));
	param.rCfgUl.u1UserCnt		= pMuruManCfg->rCfgUl.u1UserCnt;
	param.rCfgUl.u1TrigType		= pMuruManCfg->rCfgUl.u1TrigType;
	param.rCfgUl.u2TrigCnt		= (UINT16)(cpu2le16(pMuruManCfg->rCfgUl.u2TrigCnt));
	param.rCfgUl.u2TrigIntv		= (UINT16)(cpu2le16(pMuruManCfg->rCfgUl.u2TrigIntv));
	param.rCfgUl.u1UlBw			= pMuruManCfg->rCfgUl.u1UlBw;
	param.rCfgUl.u1UlGiLtf		= pMuruManCfg->rCfgUl.u1UlGiLtf;
	param.rCfgUl.u2UlLength		= (UINT16)(cpu2le16(pMuruManCfg->rCfgUl.u2UlLength));
	param.rCfgUl.u1TfPad		= pMuruManCfg->rCfgUl.u1TfPad;
	param.rCfgUl.u1BaType		= pMuruManCfg->rCfgUl.u1BaType;

	/* HETB RX */
	param.rCfgUl.rx_hetb_cfg[0] = cpu2le32(pMuruManCfg->rCfgUl.rx_hetb_cfg[0]);
	param.rCfgUl.rx_hetb_cfg[1] = cpu2le32(pMuruManCfg->rCfgUl.rx_hetb_cfg[1]);
	param.rCfgUl.rx_hetb_nonsf_en_bitmap = cpu2le32(pMuruManCfg->rCfgUl.rx_hetb_nonsf_en_bitmap);

	for (loop_idx = 0; loop_idx < MAC_ADDR_LEN; loop_idx++)
		param.rCfgUl.u1TrigTa[loop_idx] = pMuruManCfg->rCfgUl.u1TrigTa[loop_idx];

	for (loop_idx = 0; loop_idx < param.rCfgUl.u1UserCnt; loop_idx++) {
		param.rCfgUl.arUserInfoUl[loop_idx].u2WlanIdx		= (UINT16)(cpu2le16(pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u2WlanIdx));
		param.rCfgUl.arUserInfoUl[loop_idx].u1RuAllocBn		= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1RuAllocBn;
		param.rCfgUl.arUserInfoUl[loop_idx].u1RuAllocIdx	= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1RuAllocIdx;
		param.rCfgUl.arUserInfoUl[loop_idx].u1Ldpc			= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1Ldpc;
		param.rCfgUl.arUserInfoUl[loop_idx].u1Nss			= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1Nss;
		param.rCfgUl.arUserInfoUl[loop_idx].u1Mcs			= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1Mcs;
		param.rCfgUl.arUserInfoUl[loop_idx].u1TargetRssi	= pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u1TargetRssi;
		param.rCfgUl.arUserInfoUl[loop_idx].u4TrigPktSize	= (UINT32)(cpu2le32(pMuruManCfg->rCfgUl.arUserInfoUl[loop_idx].u4TrigPktSize));
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}


INT32 wifi_test_muru_ul_tx_trigger(
	PRTMP_ADAPTER pAd,
	BOOLEAN IsUlTxTrigger
)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_ULTX_TRIGGER;
	BOOLEAN fgIsUlTxTrigger = FALSE;

	fgIsUlTxTrigger = IsUlTxTrigger;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: fgIsUlTxTrigger:%d\n",
		__func__, fgIsUlTxTrigger));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(fgIsUlTxTrigger));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: msg is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&fgIsUlTxTrigger, sizeof(fgIsUlTxTrigger));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Ret=%d\n", __func__, Ret));

	return Ret;
}

INT32 wifi_test_muru_set_arb_op_mode(
	PRTMP_ADAPTER pAd,
	UINT8 arbOpMode
)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_ARB_OP_MODE;
	UINT8 OpMode = 0;

	OpMode = arbOpMode;

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(OpMode));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&OpMode, sizeof(OpMode));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));

	return Ret;
}


VOID muru_tam_arb_op_mode(PRTMP_ADAPTER pAd)
{
	INT32 Ret = TRUE;

	if (pAd->CommonCfg.TamArbOpMode) {
		if (wifi_test_muru_set_arb_op_mode(pAd, (UINT8)pAd->CommonCfg.TamArbOpMode) == FALSE) {
			Ret = FALSE;
			goto error;
		}
	}
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
}

VOID muru_update_he_cfg(PRTMP_ADAPTER pAd)
{
	INT32 Ret = TRUE;
	P_MURU_DL_MANUAL_CONFIG pCfgDl = &grCmdMuruManCfgInf.rCfgDl;
	P_MURU_UL_MANUAL_CONFIG pCfgUl = &grCmdMuruManCfgInf.rCfgUl;
	P_MURU_CMM_MANUAL_CONFIG pCfgCmm = &grCmdMuruManCfgInf.rCfgCmm;
	PUINT32 pCfgBmpDl = &grCmdMuruManCfgInf.u4ManCfgBmpDl;
	PUINT32 pCfgBmpUl = &grCmdMuruManCfgInf.u4ManCfgBmpUl;
	PUINT32 pCfgBmpCmm = &grCmdMuruManCfgInf.u4ManCfgBmpCmm;
	INT32 updateCfg = FALSE;

	if (pAd->CommonCfg.HE_OfdmaUserNum) {
		pCfgDl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
		*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;

		pCfgUl->u1UserCnt = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
		*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;

		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_PpduFmt) {
		pCfgCmm->u1PpduFmt = (UINT8)(pAd->CommonCfg.HE_PpduFmt);
		*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_OfdmaSchType) {
		pCfgCmm->u1SchType = (UINT8)(pAd->CommonCfg.HE_OfdmaSchType);
		*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_TrigPadding) {
		if (pAd->CommonCfg.HE_TrigPadding == 8)
			pCfgUl->u1TfPad = 1;
		else if (pAd->CommonCfg.HE_TrigPadding == 16)
			pCfgUl->u1TfPad = 2;
		else
			pCfgUl->u1TfPad = 2;

		*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
		updateCfg |= TRUE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("set muru_update_he_cfg()!!!!\n"));

	if (updateCfg == TRUE) {
		if (wifi_test_muru_set_manual_config(pAd, &grCmdMuruManCfgInf) == FALSE) {
			Ret = FALSE;
			goto error;
		}
	}
error:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s:(Ret = %d_\n", __func__, Ret));
}

INT32 muru_cfg_dlul_limits(
	PRTMP_ADAPTER pAd,
	UINT8 u1BandIdx
)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_CFG_DLUL_LIMIT;
	CMD_MURU_CMM_DLUL_CFG param = {0};

	param.u1BandIdx     = u1BandIdx;
	param.u1Dis160RuMu  = pAd->CommonCfg.Dis160RuMu[u1BandIdx];
	param.u1MaxRuOfdma  = pAd->CommonCfg.MaxRuOfdma[u1BandIdx];
	param.u1MaxDLMuMimo = pAd->CommonCfg.MaxDLMuMimo[u1BandIdx];
	param.u1MaxULMuMimo = pAd->CommonCfg.MaxULMuMimo[u1BandIdx];

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

INT MtCmdSetMuruCfgDlUlVal(
	PRTMP_ADAPTER pAd,
	UINT_8 BandIdx,
	UINT_8 DlUlUpdList,
	UINT_8 DlUlVal
)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_DLUL_EN;
	CMD_MURU_SET_DLUL_VAL param;

	os_zero_mem(&param, sizeof(param));
	param.u1BandIdx = BandIdx;
	param.u1DlUlUpdList = DlUlUpdList;
	param.u1DlUlVal = DlUlVal;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: BandIdx:%d ParamUpdList:%d ParamUpdVal:%d !!\n",
			__func__, param.u1BandIdx, param.u1DlUlUpdList, param.u1DlUlVal));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

INT SetMuruCfgDlUlVal(
	PRTMP_ADAPTER pAd,
	RTMP_STRING * arg
)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	UINT_8 BandIdx, DlUlUpdList, DlUlVal;

	pch = strsep(&arg, ":");
	if (pch != NULL) {
		BandIdx = os_str_toul(pch, 0, 10);
		/* sanity check for Band index */
		if (BandIdx >= DBDC_BAND_NUM) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Invalid Band Index :%d !!\n", __func__, BandIdx));
			Ret = 0;
			goto error;
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Empty Band Index !!\n", __func__));
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, ":");
	if (pch != NULL) {
		DlUlUpdList = os_str_toul(pch, 0, 10);
		if ((DlUlUpdList == 0) || (DlUlUpdList >= (1 << MURU_CFG_DLUL_OFDMA_MIMO_MAX))) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Invalid ParamUpdList :%d !!\n", __func__, DlUlUpdList));
			Ret = 0;
			goto error;
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Empty ParamUpdList !!\n", __func__));
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL) {
		DlUlVal = os_str_toul(pch, 0, 10);
		if (DlUlVal >= (1 << MURU_CFG_DLUL_OFDMA_MIMO_MAX)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Invalid ParamUpdVal :%d !!\n", __func__, DlUlVal));
			Ret = 0;
			goto error;
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Empty ParamUpdVal !!\n", __func__));
		Ret = 0;
		goto error;
	}

	Ret = MtCmdSetMuruCfgDlUlVal(pAd, BandIdx, DlUlUpdList, DlUlVal);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

INT32 wifi_test_agg_policy(
	PRTMP_ADAPTER pAd,
	UINT8 agg_policy,
	UINT8 dur_comp
)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_AGGPOLICY;

	UINT8 param[2] = {0};

	param[0] = agg_policy;
	param[1] = dur_comp;

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(UINT8)*2);
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: msg is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)param, sizeof(UINT8)*2);
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Ret=%d\n", __func__, Ret));

	return Ret;
}

INT32 hqa_muru_set_agg_policy(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE, rv = 0;
	UINT8 agg_policy = 0, dur_comp = 0;

	if (arg != NULL) {
		rv = sscanf(arg, "%hhu-%hhu", &agg_policy, &dur_comp);
		if (rv <= 0) {
			Ret = FALSE;
			goto error;
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Argument is NULL\n", __func__));
		Ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: MURU aggpolicy:%u, duration comp:%u\n", __func__, agg_policy, dur_comp));

	Ret = wifi_test_agg_policy(pAd, agg_policy, dur_comp);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: CMD %s\n", __func__, Ret ? "Success":"Fail"));
	return Ret;
}

INT32 ShowMuruLastSplByQid(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT32 cmd = MURU_GET_LAST_SPL;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 u1Qid = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: u1Qid = %d\n", __func__, u1Qid));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(UINT_8));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&u1Qid, sizeof(u1Qid));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Ret = %d\n", __func__, Ret));
	return Ret;
}

INT32 wifi_muru_get_fw_black_list_ctrl(
	RTMP_ADAPTER *pAd,
	UINT16 u2Wcid
)
{
	INT32 Ret = TRUE;
	UINT32 cmd = MURU_GET_FW_BLACKLIST_CTRL;
	EVENT_GET_MURU_FW_BLACKLIST_CTRL result = {0};

	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(u2Wcid));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, muruEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&u2Wcid, sizeof(UINT16));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, Ret));
	return Ret;
}

INT32 wifi_muru_set_drv_black_list_ctrl(
	RTMP_ADAPTER *pAd,
	P_CMD_SET_MURU_DRV_BLACKLIST_CTRL prMuruDrvBlackCtrl
)
{
	INT32 Ret = TRUE;
	UINT32 cmd = MURU_SET_DRV_BLACKLIST_CTRL;
	CMD_SET_MURU_DRV_BLACKLIST_CTRL param = {0};

	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	param.u2WlanId = (UINT16)(cpu2le16(prMuruDrvBlackCtrl->u2WlanId));
	param.fgDrvBlackListDlOfdmaDisable = prMuruDrvBlackCtrl->fgDrvBlackListDlOfdmaDisable;
	param.fgDrvBlackListUlOfdmaDisable = prMuruDrvBlackCtrl->fgDrvBlackListUlOfdmaDisable;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(CMD_SET_MURU_DRV_BLACKLIST_CTRL));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Ret = %d\n", __func__, Ret));
	return Ret;
}

INT get_muru_fw_black_list_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	UINT16 u2Wcid = 0;

	pch = strsep(&arg, ":");

	if (pch != NULL)
		u2Wcid = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: u2WlanId=%u\n",
					__func__,
					u2Wcid
					));

	Ret = wifi_muru_get_fw_black_list_ctrl(pAd, u2Wcid);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT set_muru_drv_black_list_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	CMD_SET_MURU_DRV_BLACKLIST_CTRL rMuruDrvBlackCtrl = {0};
	UINT_8 temp = 0;

	pch = strsep(&arg, ":");
	if (pch != NULL)
		rMuruDrvBlackCtrl.u2WlanId = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, ":");
	if (pch != NULL)
		temp = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	rMuruDrvBlackCtrl.fgDrvBlackListDlOfdmaDisable = (temp & MURU_DRV_BLACK_LIST_DL_OFDMA_DISABLE) ? TRUE : FALSE;
	rMuruDrvBlackCtrl.fgDrvBlackListUlOfdmaDisable = (temp & MURU_DRV_BLACK_LIST_UL_OFDMA_DISABLE) ? TRUE : FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: u2WlanId=%u, fgDrvDlOfdmaDis=%u,fgDrvUlOfdmaDis=%u\n",
					__func__,
					rMuruDrvBlackCtrl.u2WlanId,
					rMuruDrvBlackCtrl.fgDrvBlackListDlOfdmaDisable,
					rMuruDrvBlackCtrl.fgDrvBlackListUlOfdmaDisable
					));

	Ret = wifi_muru_set_drv_black_list_ctrl(pAd, &rMuruDrvBlackCtrl);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


#endif /* CFG_SUPPORT_FALCON_MURU */
