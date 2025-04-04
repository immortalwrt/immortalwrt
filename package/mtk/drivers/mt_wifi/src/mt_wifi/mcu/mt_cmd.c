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
	mt_cmd.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "mt_cmd.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || \
	defined(PRE_CAL_MT7622_SUPPORT) || defined(PRE_CAL_MT7626_SUPPORT) || \
	defined(PRE_CAL_MT7915_SUPPORT) || defined(PRE_CAL_MT7986_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

/* #include "hdev/hdev.h" */

/*  */
/* Define for chip_cmd_tx retry count */
/*  */
#define MT_CMD_RETRY_COUNT 5
#define MT_CMD_RETRY_INTEVAL 200    /* usec */

#ifdef WIFI_EAP_FEATURE
static VOID eapEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
		UINT16 rsp_payload_len);
#endif

UINT16 GetRealPortQueueID(struct cmd_msg *msg, UINT8 cmd_type)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)(msg->priv);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (IS_HIF_TYPE(ad, HIF_MT)) {
		if (cmd_type == MT_FW_SCATTER)
			return (UINT16)cap->PDA_PORT;
		else
			return (UINT16)P1_Q0;
	}

	return (UINT16)CPU_TX_PORT;
}

VOID MtAndesFreeCmdMsg(struct cmd_msg *msg)
{
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#ifdef COMPOS_WIN
	os_free_mem(msg);
#endif /* COMPOS_WIN */
#ifdef COMPOS_TESTMODE_WIN
	TestModeFreeCmdMsg(msg);
#endif /* COMPOS_TESTMODE_WIN */
#else
	AndesFreeCmdMsg(msg);
#endif
}

struct cmd_msg *MtAndesAllocCmdMsg(RTMP_ADAPTER *ad, unsigned int length)
{
	return AndesAllocCmdMsg(ad, length);
}

#ifdef WIFI_UNIFIED_COMMAND
struct cmd_msg *MtAndesAllocUniCmdMsg(RTMP_ADAPTER *ad, unsigned int length)
{
	return AndesAllocUniCmdMsg(ad, length);
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID MtAndesInitCmdMsg(struct cmd_msg *msg, CMD_ATTRIBUTE attr)
{
	AndesInitCmdMsg(msg, attr);
}

VOID MtAndesAppendCmdMsg(struct cmd_msg *msg, char *data, UINT32 len)
{
	AndesAppendCmdMsg(msg, data, len);
}

static  VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult.u4Status = 0x%x\n",
			  __func__, EventExtCmdResult->u4Status);

#ifdef MT7663_RFB_MANUAL_CAL
	if (EventExtCmdResult->ucExTenCID == EXT_CMD_CHANNEL_SWITCH) {
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) msg->priv;
		mt7663_manual_cal(pAd);
	}
#endif /* MT7663_RFB_MANUAL_CAL */

#ifdef LINUX
	/* RTMP_OS_TXRXHOOK_CALL(WLAN_CALIB_TEST_RSP,NULL,EventExtCmdResult->u4Status,pAd); */
#endif /* LINUX */
}

/*****************************************
 *	ExT_CID = 0x01
 *****************************************/
#ifdef RTMP_EFUSE_SUPPORT
static VOID CmdEfuseAccessReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_ACCESS_EFUSE_T *pCmdAccessEfuse = (CMD_ACCESS_EFUSE_T *)data;
	EFUSE_ACCESS_DATA_T *pEfuseValue = NULL;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEfuse->u4Address),
			  le2cpu32(pCmdAccessEfuse->u4Valid));
	pEfuseValue = (EFUSE_ACCESS_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;
	*pEfuseValue->pIsValid = le2cpu32(pCmdAccessEfuse->u4Valid);
	os_move_mem(&pEfuseValue->pValue[0],
				&pCmdAccessEfuse->aucData[0], EFUSE_BLOCK_SIZE);
}

INT32 MtCmdEfuseAccessRead(RTMP_ADAPTER *pAd, USHORT offset,
						   PUCHAR pData, PUINT pIsValid)
{
	struct cmd_msg *msg;
	CMD_ACCESS_EFUSE_T CmdAccessEfuse;
	EFUSE_ACCESS_DATA_T efuseAccessData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_ACCESS_EFUSE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&efuseAccessData, sizeof(efuseAccessData));
	efuseAccessData.pValue = (PUSHORT)pData;
	efuseAccessData.pIsValid = pIsValid;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EFUSE_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(CMD_ACCESS_EFUSE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &efuseAccessData);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdEfuseAccessReadCb);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	CmdAccessEfuse.u4Address = (offset / EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
#ifdef RT_BIG_ENDIAN
	CmdAccessEfuse.u4Address = cpu2le32(CmdAccessEfuse.u4Address);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

static VOID CmdEfuseAccessWriteCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_ACCESS_EFUSE_T *pCmdAccessEfuse = (CMD_ACCESS_EFUSE_T *)data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEfuse->u4Address), le2cpu32(pCmdAccessEfuse->u4Valid));
}

VOID MtCmdEfuseAccessWrite(RTMP_ADAPTER *pAd, USHORT offset, PUCHAR pData)
{
	struct cmd_msg *msg;
	CMD_ACCESS_EFUSE_T CmdAccessEfuse;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_ACCESS_EFUSE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EFUSE_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(CMD_ACCESS_EFUSE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdEfuseAccessWriteCb);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	os_move_mem(&CmdAccessEfuse.aucData[0], &pData[0], EFUSE_BLOCK_SIZE);
	CmdAccessEfuse.u4Address = (offset / EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
#ifdef RT_BIG_ENDIAN
	CmdAccessEfuse.u4Address = cpu2le32(CmdAccessEfuse.u4Address);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&CmdAccessEfuse, sizeof(CmdAccessEfuse));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
}

static VOID CmdEfuseFreeBlockCountCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *pCmdEfuseFreeBlockCount = (EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *)data;
	EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *pEfuseFreeCount = NULL;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s\n", __func__);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "ucFreeBlockNum:%d,ucTotalBlockNum:%d\n",
			  (pCmdEfuseFreeBlockCount->ucFreeBlockNum),
			  (pCmdEfuseFreeBlockCount->ucTotalBlockNum));

	pEfuseFreeCount = (EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *)msg->attr.rsp.wb_buf_in_calbk;
	pEfuseFreeCount->ucFreeBlockNum = pCmdEfuseFreeBlockCount->ucFreeBlockNum;
	pEfuseFreeCount->ucTotalBlockNum = pCmdEfuseFreeBlockCount->ucTotalBlockNum;

}

INT32 MtCmdEfuseFreeBlockCount(struct _RTMP_ADAPTER *pAd, PVOID GetFreeBlock, PVOID Result)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 Version = 0;
	struct _EXT_CMD_EFUSE_FREE_BLOCK_T CmdEfuseFreeBlock;
	struct _EXT_CMD_EFUSE_FREE_BLOCK_T *pCmdFreeBlock = (struct _EXT_CMD_EFUSE_FREE_BLOCK_T *)GetFreeBlock;
#if defined(MT7663) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7663(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
		Version = pCmdFreeBlock->ucVersion;
#endif

	msg = MtAndesAllocCmdMsg(pAd, sizeof(struct _EXT_CMD_EFUSE_FREE_BLOCK_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EFUSE_FREE_BLOCK);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);

	if (Version == 0) {
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_EFUSE_FREE_BLOCK_T));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, Result);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	} else if (Version == 1) {
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_EFUSE_FREE_BLOCK_V1_T));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, Result);
		SET_CMD_ATTR_RSP_HANDLER(attr, CmdEfuseFreeBlockCountCb);
	}

	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdEfuseFreeBlock, sizeof(CmdEfuseFreeBlock));

	if (Version == 0) {
		UINT8 *FreeBlock = GetFreeBlock;

		CmdEfuseFreeBlock.ucGetFreeBlock = *FreeBlock;

	} else if (Version == 1) {
		CmdEfuseFreeBlock.ucVersion = pCmdFreeBlock->ucVersion;
		CmdEfuseFreeBlock.ucDieIndex = pCmdFreeBlock->ucDieIndex;

		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Version = %x  DieIndex:%d\n",
				CmdEfuseFreeBlock.ucVersion, CmdEfuseFreeBlock.ucDieIndex);
	}

	MtAndesAppendCmdMsg(msg, (char *)&CmdEfuseFreeBlock, sizeof(CmdEfuseFreeBlock));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef AIR_MONITOR
static VOID MtCmdGetTxSmeshRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_SMESH_T prEventExtCmdResult =
		(P_EXT_EVENT_SMESH_T)Data;
	P_EXT_EVENT_SMESH_T prSmesh =
		(P_EXT_EVENT_SMESH_T)msg->attr.rsp.wb_buf_in_calbk;
	prSmesh->ucBand = prEventExtCmdResult->ucBand;

	prSmesh->u4SmeshVal =
			le2cpu32(prEventExtCmdResult->u4SmeshVal);
}

/*****************************************
 *    ExT_CID = 0xAE
 *****************************************/
INT32 MtCmdSmeshConfigSet(RTMP_ADAPTER *pAd, UCHAR *pdata, P_EXT_EVENT_SMESH_T prSmeshResult)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	EXT_CMD_SMESH_T *pconfig_smesh = (EXT_CMD_SMESH_T *)pdata;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(EXT_CMD_SMESH_T);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:ucBand = %d, ucAccessMode = %d, ucSmeshEn = %d, fgSmeshRxA2 = %d, fgSmeshRxA1 = %d, fgSmeshRxData = %d, fgSmeshRxMgnt = %d, fgSmeshRxCtrl = %d\n",
			  __func__, pconfig_smesh->ucBand,
			  pconfig_smesh->ucAccessMode, pconfig_smesh->ucSmeshEn,
			  pconfig_smesh->fgSmeshRxA2, pconfig_smesh->fgSmeshRxA1,
			  pconfig_smesh->fgSmeshRxData, pconfig_smesh->fgSmeshRxMgnt,
			  pconfig_smesh->fgSmeshRxCtrl);

	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CONFIG_SMESH);
	if (pconfig_smesh->ucAccessMode == 0) {
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_SMESH_T));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prSmeshResult);
		SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetTxSmeshRsp);
	} else {
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	}
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pconfig_smesh, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* AIR_MONITOR */


INT32 MtCmdEfuseAccessCheck(RTMP_ADAPTER *pAd, UINT32 offset, PUCHAR pData)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_ACCESS_EFUSE_CHECK_T CmdEfuseAccessCheck;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: offset = %x\n", __func__, offset);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(struct _EXT_CMD_ACCESS_EFUSE_CHECK_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EFUSE_ACCESS_CHECK);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_ACCESS_EFUSE_CHECK_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdEfuseAccessCheck, sizeof(CmdEfuseAccessCheck));
	CmdEfuseAccessCheck.u4Address = (offset / EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
	os_move_mem(&CmdEfuseAccessCheck.aucData[0], &pData[0], EFUSE_BLOCK_SIZE);
	CmdEfuseAccessCheck.u4Address = cpu2le32(CmdEfuseAccessCheck.u4Address);
	MtAndesAppendCmdMsg(msg, (char *)&CmdEfuseAccessCheck, sizeof(CmdEfuseAccessCheck));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#endif /* RTMP_EFUSE_SUPPORT */

/*****************************************
 *	ExT_CID = 0x02
 *****************************************/
static VOID CmdRFRegAccessReadCb(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _CMD_RF_REG_ACCESS_T *RFRegAccess =
		(struct _CMD_RF_REG_ACCESS_T *)Data;
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &RFRegAccess->Data, Len - 8);
	*msg->attr.rsp.wb_buf_in_calbk = le2cpu32(*msg->attr.rsp.wb_buf_in_calbk);
}

INT32 MtCmdRFRegAccessWrite(RTMP_ADAPTER *pAd, UINT32 RFIdx,
							UINT32 Offset, UINT32 Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: RFIdx = %d, Offset = %x, Value = %x\n",
			  __func__, RFIdx, Offset, Value);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_REG_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&RFRegAccess, sizeof(RFRegAccess));
	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);
	RFRegAccess.Data = cpu2le32(Value);
	MtAndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdRFRegAccessRead(RTMP_ADAPTER *pAd, UINT32 RFIdx,
						   UINT32 Offset, UINT32 *Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: RFIdx = %d, Offset = %x\n", __func__, RFIdx, Offset);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_REG_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 12);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, Value);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdRFRegAccessReadCb);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&RFRegAccess, sizeof(RFRegAccess));
	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);
	MtAndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

static VOID MtCmdRfTestTxToneCb(struct cmd_msg *pMsg, char *pData, UINT16 u2Len)
{
	PEXT_EVENT_RF_TEST_RESULT_T prEvent = (PEXT_EVENT_RF_TEST_RESULT_T)pData;

	switch (prEvent->u4FuncIndex) {
	case GET_TX_TONE_GAIN_OFFSET:
		os_move_mem(pMsg->attr.rsp.wb_buf_in_calbk,
					&prEvent->aucEvent[0],
					sizeof(prEvent->aucEvent[0]));
		*pMsg->attr.rsp.wb_buf_in_calbk = le2cpu32(*pMsg->attr.rsp.wb_buf_in_calbk);
		break;
	}
}

/*****************************************
 *	ExT_CID = 0x04
 *****************************************/
/* unify */
/* send command */
static VOID MtCmdATERFTestResp(struct cmd_msg *msg, char *data, UINT16 len)
{
	EXT_EVENT_RF_TEST_RESULT_T *result = (EXT_EVENT_RF_TEST_RESULT_T *)data;

	switch (le2cpu32(result->u4FuncIndex)) {
	case RDD_TEST_MODE:
	case RE_CALIBRATION:
	case CALIBRATION_BYPASS:
		break;
#ifdef INTERNAL_CAPTURE_SUPPORT
	case GET_ICAP_CAPTURE_STATUS:
		MtCmdRfTestSolicitICapStatusCb(msg, (PINT32)data, len);
		break;

	case GET_ICAP_RAW_DATA:
		MtCmdRfTestSolicitICapIQDataCb(msg, (PINT32)data, len);
		break;
#endif/* INTERNAL_CAPTURE_SUPPORT */

	case GET_TX_TONE_GAIN_OFFSET:
		MtCmdRfTestTxToneCb(msg, data, len);
		break;
	default:
		break;
	}
}

static INT32 MtCmdRfTest(RTMP_ADAPTER *pAd, CMD_TEST_CTRL_T TestCtrl,
						 char *rsp_payload, UINT16 rsp_len)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Action = %d\n", TestCtrl.ucAction);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(TestCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 30000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, rsp_len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rsp_payload);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdATERFTestResp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/* IcapLen only be used when OpMode is OPERATION_ICAP_OVERLAP */
INT32 MtCmdRfTestSwitchMode(RTMP_ADAPTER *pAd, UINT32 OpMode,
							UINT8 IcapLen, UINT16 rsp_len)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_SWITCH_TO_RFTEST;
	TestCtrl.u.u4OpMode = cpu2le32(OpMode);

	if (OpMode == OPERATION_ICAP_OVERLAP)
		TestCtrl.ucIcapLen = IcapLen;

	ret = MtCmdRfTest(pAd, TestCtrl, NULL, rsp_len);
	return ret;
}

#ifdef PHY_ICS_SUPPORT
/*
	==========================================================================
	Description:
	FW in-band CMD of PHY ICS.
	Return:
	==========================================================================
*/
static INT32 MtCmdPhyIcs(
	IN PRTMP_ADAPTER pAd,
	IN EXT_CMD_PHY_ICS_CTRL_T PhyIcsCtrl,
	IN UINT8 ctrl_flag,
	IN UINT16 rsp_wait_time,
	IN MSG_RSP_HANDLER rsp_handler,
	IN INT8 * rsp_payload,
	IN UINT16 rsp_len)
{
	struct cmd_msg *msg;
	UINT32 FuncIndex = PhyIcsCtrl.u4FuncIndex;
	INT32 Status;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"FuncIndex = %d\n", FuncIndex); /* only use funcIdx = 3 */

	msg = MtAndesAllocCmdMsg(pAd, sizeof(PhyIcsCtrl));

	if (!msg) {
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_CTRL_FLAGS(attr, ctrl_flag);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, rsp_wait_time);
	SET_CMD_ATTR_RSP_HANDLER(attr, rsp_handler);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WIFI_SPECTRUM);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, rsp_len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rsp_payload);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PhyIcsCtrl, sizeof(PhyIcsCtrl));
	Status = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Start for phy ics data capture.
	Return:
	==========================================================================
*/
INT32 MtCmdPhyIcsStart(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData)
{
	INT32 Status;
	EXT_CMD_PHY_ICS_CTRL_T PhyIcsCtrl;
	PHY_ICS_START_T *pPhyIcsInfo = NULL;
	PHY_ICS_START_T *prPhyIcsCmd = (PHY_ICS_START_T *)pData;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of PhyIcsCtrl */
	os_zero_mem(&PhyIcsCtrl, sizeof(PhyIcsCtrl));
	PhyIcsCtrl.u4FuncIndex = cpu2le32(SPECTRUM_CTRL_FUNCID_SET_PHY_ICS_PARAMETER);
	pPhyIcsInfo = &PhyIcsCtrl.rPhyIcsInfo;
	pPhyIcsInfo->fgTrigger = cpu2le32(prPhyIcsCmd->fgTrigger);
	pPhyIcsInfo->fgRingCapEn = cpu2le32(prPhyIcsCmd->fgRingCapEn);
	pPhyIcsInfo->u4PhyIdx = cpu2le32(prPhyIcsCmd->u4PhyIdx);
	pPhyIcsInfo->u4BandIdx = cpu2le32(prPhyIcsCmd->u4BandIdx);
	pPhyIcsInfo->u4BW = cpu2le32(prPhyIcsCmd->u4BW);
	pPhyIcsInfo->u4PhyIcsType = cpu2le32(prPhyIcsCmd->u4PhyIcsType);
	pPhyIcsInfo->u4PhyIcsEventGroup = cpu2le32(prPhyIcsCmd->u4PhyIcsEventGroup);
	pPhyIcsInfo->u4PhyIcsEventID[0] = cpu2le32(prPhyIcsCmd->u4PhyIcsEventID[0]);
	pPhyIcsInfo->u4PhyIcsEventID[1] = cpu2le32(prPhyIcsCmd->u4PhyIcsEventID[1]);
	pPhyIcsInfo->u4PhyIcsTimer = cpu2le32(prPhyIcsCmd->u4PhyIcsTimer);

	if (IS_MT7986(pAd))
		pPhyIcsInfo->u4EnBitWidth = cpu2le32(CAP_BW_128B_TO_128B); /* keep original */
	else
		pPhyIcsInfo->u4EnBitWidth = cpu2le32(CAP_96_BIT);
	pPhyIcsInfo->u4Architech = cpu2le32(CAP_ON_CHIP);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"
			" pPhyIcsInfo->fgTrigger = 0x%08x\n pPhyIcsInfo->fgRingCapEn = 0x%08x\n"
			" pPhyIcsInfo->u4BandIdx = 0x%08x\n pPhyIcsInfo->u4PhyIdx = 0x%08x, pPhyIcsInfo->u4BW = 0x%08x\n"
			" pPhyIcsInfo->u4PhyIcsType = 0x%08x\n pPhyIcsInfo->u4PhyIcsEventGroup = 0x%08x\n"
			" pPhyIcsInfo->u4PhyIcsEventID[0] = 0x%08x\n pPhyIcsInfo->u4PhyIcsEventID[1] = 0x%08x\n"
			" pPhyIcsInfo->u4PhyIcsTimer = %d\n", pPhyIcsInfo->fgTrigger, pPhyIcsInfo->fgRingCapEn,
			pPhyIcsInfo->u4BandIdx, pPhyIcsInfo->u4PhyIdx, pPhyIcsInfo->u4BW,
			pPhyIcsInfo->u4PhyIcsType, pPhyIcsInfo->u4PhyIcsEventGroup,
			pPhyIcsInfo->u4PhyIcsEventID[0], pPhyIcsInfo->u4PhyIcsEventID[1],
			pPhyIcsInfo->u4PhyIcsTimer);

    /* Dump Ics data from RBIST sysram by Unsolicited event */
	Status = MtCmdPhyIcs(pAd, PhyIcsCtrl, INIT_CMD_SET, SPECTRUM_DEFAULT_WAIT_RESP_TIME
								, NULL, NULL, SPECTRUM_DEFAULT_RESP_LEN);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}
#endif /* PHY_ICS_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
/*
	==========================================================================
	Description:
	Solicited event handler of wifi-spectrum.
	Return:
	==========================================================================
*/
static VOID MtCmdWifiSpectrumResp(
	IN struct cmd_msg *msg,
	IN char *pData,
	IN UINT16 Len)
{
	EXT_EVENT_SPECTRUM_RESULT_T *pResult = (EXT_EVENT_SPECTRUM_RESULT_T *)pData;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "----------------->\n");

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "FuncIndex = %d\n", pResult->u4FuncIndex);

	switch (le2cpu32(pResult->u4FuncIndex)) {
	case SPECTRUM_CTRL_FUNCID_GET_CAPTURE_STATUS:
		MtCmdWifiSpectrumSolicitCapStatusCb(msg, pData, Len);
		break;

	default:
		break;
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	FW in-band CMD of wifi-spectrum.
	Return:
	==========================================================================
*/
static INT32 MtCmdWifiSpectrum(
	IN PRTMP_ADAPTER pAd,
	IN EXT_CMD_SPECTRUM_CTRL_T * SpectrumCtrl,
	IN UINT8 ctrl_flag,
	IN UINT16 rsp_wait_time,
	IN MSG_RSP_HANDLER rsp_handler,
	IN INT8 * rsp_payload,
	IN UINT16 rsp_len)
{
	struct cmd_msg *msg;
	UINT32 FuncIndex = SpectrumCtrl->u4FuncIndex;
	INT32 Status;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"FuncIndex = %d\n", FuncIndex);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(IN EXT_CMD_SPECTRUM_CTRL_T));

	if (!msg) {
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_CTRL_FLAGS(attr, ctrl_flag);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, rsp_wait_time);
	SET_CMD_ATTR_RSP_HANDLER(attr, rsp_handler);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WIFI_SPECTRUM);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, rsp_len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rsp_payload);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)SpectrumCtrl, sizeof(IN EXT_CMD_SPECTRUM_CTRL_T));
	Status = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Start for wifi-spectrum data capture.
	Return:
	==========================================================================
*/
INT32 MtCmdWifiSpectrumStart(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData)
{
	INT32 Status;
	EXT_CMD_SPECTRUM_CTRL_T SpectrumCtrl;
	RBIST_CAP_START_T *pSpectrumInfo = NULL;
	RBIST_CAP_START_T *prRBISTInfo = (RBIST_CAP_START_T *)pData;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data of pAd */
	pAd->SpectrumCapNode = prRBISTInfo->u4CaptureNode;
	/* Update data structure of SpectrumCtrl */
	os_zero_mem(&SpectrumCtrl, sizeof(SpectrumCtrl));
	SpectrumCtrl.u4FuncIndex = cpu2le32(SPECTRUM_CTRL_FUNCID_SET_PARAMETER);
	pSpectrumInfo = &SpectrumCtrl.rSpectrumInfo;
	pSpectrumInfo->fgTrigger = cpu2le32(prRBISTInfo->fgTrigger);
	pSpectrumInfo->fgRingCapEn = cpu2le32(prRBISTInfo->fgRingCapEn);
	pSpectrumInfo->u4TriggerEvent = cpu2le32(prRBISTInfo->u4TriggerEvent);
	pSpectrumInfo->u4CaptureNode = cpu2le32(prRBISTInfo->u4CaptureNode);
	pSpectrumInfo->u4CaptureLen = cpu2le32(prRBISTInfo->u4CaptureLen);
	pSpectrumInfo->u4CapStopCycle = cpu2le32(prRBISTInfo->u4CapStopCycle);
	pSpectrumInfo->u4BW = cpu2le32(prRBISTInfo->u4BW);
	//pSpectrumInfo->u4PdEnable = cpu2le32(prRBISTInfo->u4PdEnable);
	pSpectrumInfo->u4FixRxGain = cpu2le32(prRBISTInfo->u4FixRxGain);
	pSpectrumInfo->u4PhyIdx = cpu2le32(prRBISTInfo->u4PhyIdx);
	if (pSpectrumInfo->u4FixRxGain == 0) {
		pSpectrumInfo->u4PdEnable = 1;
		pAd->SpectrumFixGain = 0;
	} else {
		pSpectrumInfo->u4PdEnable = 0;
		pAd->SpectrumFixGain = 1;
	}
	pSpectrumInfo->u4WifiPath = cpu2le32(prRBISTInfo->u4WifiPath);
	pSpectrumInfo->u4BandIdx = cpu2le32(prRBISTInfo->u4BandIdx);
	if (IS_MT7986(pAd))
		pSpectrumInfo->u4EnBitWidth = cpu2le32(CAP_BW_128B_TO_128B); /* keep original */
	else
		pSpectrumInfo->u4EnBitWidth = cpu2le32(CAP_96_BIT);
	pSpectrumInfo->u4Architech = cpu2le32(CAP_ON_CHIP);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"
			" pSpectrumInfo->fgTrigger = 0x%08x\n pSpectrumInfo->fgRingCapEn = 0x%08x\n"
			" pSpectrumInfo->u4TriggerEvent = 0x%08x\n pSpectrumInfo->u4CaptureNode = 0x%08x\n"
			" pSpectrumInfo->u4CaptureLen = 0x%08x\n pSpectrumInfo->u4CapStopCycle = 0x%08x\n"
			" pSpectrumInfo->ucBW = 0x%08x\n pSpectrumInfo->u4PdEnable = 0x%08x\n"
			" pSpectrumInfo->u4FixRxGain = 0x%08x\n pSpectrumInfo->u4WifiPath = 0x%08x\n"
			" pSpectrumInfo->u4BandIdx = 0x%08x  pSpectrumInfo->u4PhyIdx = 0x%08x\n", pSpectrumInfo->fgTrigger, pSpectrumInfo->fgRingCapEn,
			pSpectrumInfo->u4TriggerEvent, pSpectrumInfo->u4CaptureNode, pSpectrumInfo->u4CaptureLen,
			pSpectrumInfo->u4CapStopCycle, pSpectrumInfo->u4BW, pSpectrumInfo->u4PdEnable,
			pSpectrumInfo->u4FixRxGain, pSpectrumInfo->u4WifiPath, pSpectrumInfo->u4BandIdx, pSpectrumInfo->u4PhyIdx);

	Status = MtCmdWifiSpectrum(pAd, &SpectrumCtrl, INIT_CMD_SET, SPECTRUM_DEFAULT_WAIT_RESP_TIME
								, NULL, NULL, SPECTRUM_DEFAULT_RESP_LEN);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Query status of wifi-spectrum data capture.
	Return:
	==========================================================================
*/
INT32 MtCmdWifiSpectrumUnSolicitCapStatus(
	IN RTMP_ADAPTER *pAd)
{
	INT32 Status = CAP_BUSY, retval;
	PRTMP_REG_PAIR pRegCapture = NULL, pRegInterface = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	retval = os_alloc_mem(pAd, (UCHAR **)&pRegCapture, sizeof(RTMP_REG_PAIR));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pRegCapture, sizeof(RTMP_REG_PAIR));

	retval = os_alloc_mem(pAd, (UCHAR **)&pRegInterface, sizeof(RTMP_REG_PAIR));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pRegInterface, sizeof(RTMP_REG_PAIR));

	/* Get RBIST capture-stop bit */
	pRegCapture->Register = RBISTCR0;
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMultipleMacRegAccessRead(pAd, pRegCapture, 1);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMacRegAccessRead(pAd, pRegCapture, 1);

	if ((pRegCapture->Value & BIT(CR_RBIST_CAPTURE)) == 0) { /* Stop capture */
		/* Change backdoor interface to AHB interface => RBISTCR10[28:26] = 3b'000 */
		pRegInterface->Register = RBISTCR10;
		pRegInterface->Value = ((pRegInterface->Value) & ~BITS(SYSRAM_INTF_SEL1, SYSRAM_INTF_SEL3));
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessWrite(pAd, pRegInterface, 1);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessWrite(pAd, pRegInterface, 1);
		/* Update status*/
		Status = CAP_SUCCESS;
	}

error:
	if (pRegCapture != NULL)
		os_free_mem(pRegCapture);

	if (pRegInterface != NULL)
		os_free_mem(pRegInterface);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != CAP_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

INT32 MtCmdWifiSpectrumSolicitCapStatus(
	IN RTMP_ADAPTER *pAd)
{
	INT32 Status = CAP_BUSY;
	UINT32 CapDone = 0;
	EXT_CMD_SPECTRUM_CTRL_T SpectrumCtrl;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of TestCtrl */
	os_zero_mem(&SpectrumCtrl, sizeof(SpectrumCtrl));
	SpectrumCtrl.u4FuncIndex = cpu2le32(SPECTRUM_CTRL_FUNCID_GET_CAPTURE_STATUS);

	/* Query current captured status by solicited event */
	MtCmdWifiSpectrum(pAd, &SpectrumCtrl, INIT_CMD_SET_AND_WAIT_RETRY_RSP
						, SPECTRUM_WAIT_RESP_TIME, &MtCmdWifiSpectrumResp
						, (INT8 *)&CapDone, sizeof(EXT_EVENT_RBIST_CAP_STATUS_T));

	if (CapDone)
		Status = CAP_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != CAP_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Callback function of querying status of wifi-spectrum data capture.
	Return:
	==========================================================================
*/
VOID MtCmdWifiSpectrumSolicitCapStatusCb(
	IN struct cmd_msg *msg,
	IN INT8 *pData,
	IN UINT16 Length)
{
	EXT_EVENT_RBIST_CAP_STATUS_T *pEventdata = (EXT_EVENT_RBIST_CAP_STATUS_T *)pData;
	UINT32 *pCapDone = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pCapDone */
	*pCapDone = le2cpu32(pEventdata->u4CapDone);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[42m%s\x1b[m\n", (*pCapDone == TRUE) ?
			"Capture done!!" : "Not yet!!");

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return;
}

/*
	==========================================================================
	Description:
	Raw data process of wifi-spectrum.
	Return:
	==========================================================================
*/
INT32 MtCmdWifiSpectrumUnSolicitRawDataProc(
	IN RTMP_ADAPTER *pAd)
{
	INT32 i, Status, retval;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	EXT_CMD_SPECTRUM_CTRL_T SpectrumCtrl;
	RBIST_DUMP_RAW_DATA_T *pSpectrumDump = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of SpectrumCtrl */
	os_zero_mem(&SpectrumCtrl, sizeof(SpectrumCtrl));
	SpectrumCtrl.u4FuncIndex = cpu2le32(SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA);
	/* Initialization of OS wait for completion */
	RTMP_OS_INIT_COMPLETION(&pAd->SpectrumDumpDataDone);
	/* Initialization of status */
	Status = pAd->SpectrumStatus = CAP_FAIL;
	/* Initialization of SpectrumEventCnt */
	pAd->SpectrumEventCnt = 0;

	/* Create pSrcf_IQ file descriptor */
	pAd->pSrcf_IQ = RtmpOSFileOpen(pAd->pSrc_IQ, O_WRONLY | O_CREAT, 0);
	if (IS_FILE_OPEN_ERR(pAd->pSrcf_IQ)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"--> Error opening %s\n", pAd->pSrc_IQ);
		goto error;
	}

	/* Create pSrcf_Gain file descriptor */
	pAd->pSrcf_Gain = RtmpOSFileOpen(pAd->pSrc_Gain, O_WRONLY | O_CREAT, 0);
	if (IS_FILE_OPEN_ERR(pAd->pSrcf_Gain)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"--> Error opening %s\n", pAd->pSrc_Gain);
		goto error;
	}

	/* Create pSrcf_InPhySniffer file descriptor */
	pAd->pSrcf_InPhySniffer = RtmpOSFileOpen(pAd->pSrc_InPhySniffer, O_WRONLY | O_CREAT, 0);
	if (IS_FILE_OPEN_ERR(pAd->pSrcf_InPhySniffer)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"--> Error opening %s\n", pAd->pSrc_InPhySniffer);
		goto error;
	}

	if (IS_MT7622(pAd)) {
		/* Query whole RBIST data by bank */
		for (i = 0; i < pChipCap->SpectrumBankNum; i++) {
			RBIST_DESC_T *pSpectrumDesc = &pChipCap->pSpectrumDesc[i];

			/* Update data structure of SpectrumCtrl */
			pSpectrumDump = &SpectrumCtrl.rSpectrumDump;
			pSpectrumDump->u4Address = cpu2le32(pSpectrumDesc->u4Address);
			pSpectrumDump->u4AddrOffset = cpu2le32(pSpectrumDesc->u4AddrOffset);
			pSpectrumDump->u4Bank = cpu2le32(pSpectrumDesc->u4Bank);
			pSpectrumDump->u4BankSize = cpu2le32(pSpectrumDesc->u4BankSize);
			/* Update SpectrumIdx */
			pAd->SpectrumIdx = i;

			MtCmdWifiSpectrum(pAd, &SpectrumCtrl, INIT_CMD_SET, SPECTRUM_DEFAULT_WAIT_RESP_TIME
								, NULL, NULL, SPECTRUM_DEFAULT_RESP_LEN);

			/* OS wait for completion time out */
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->SpectrumDumpDataDone,
					RTMPMsecsToJiffies(CAP_DUMP_DATA_EXPIRE))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Spectrum dump data timeout !!\x1b[m\n");
				goto error;
			}
		}
	} else {
		/* Query whole RBIST data at a time */
		{
			MtCmdWifiSpectrum(pAd, &SpectrumCtrl, INIT_CMD_SET, SPECTRUM_DEFAULT_WAIT_RESP_TIME
								, NULL, NULL, SPECTRUM_DEFAULT_RESP_LEN);

			/* OS wait for completion time out */
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->SpectrumDumpDataDone,
					RTMPMsecsToJiffies(CAP_DUMP_DATA_EXPIRE))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Spectrum dump data timeout !!\x1b[m\n");
				goto error;
			}
		}
	}

error:
	/* Close pSrcf_IQ file descriptor */
	if (!IS_FILE_OPEN_ERR(pAd->pSrcf_IQ)) {
		retval = RtmpOSFileClose(pAd->pSrcf_IQ);
		if (retval) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"--> Error %d closing %s\n", -retval, pAd->pSrc_IQ);
		}
		pAd->pSrcf_IQ = NULL;
	}

	/* Close pSrcf_Gain file descriptor */
	if (!IS_FILE_OPEN_ERR(pAd->pSrcf_Gain)) {
		retval = RtmpOSFileClose(pAd->pSrcf_Gain);
		if (retval) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"--> Error %d closing %s\n", -retval, pAd->pSrc_Gain);
		}
		pAd->pSrcf_Gain = NULL;
	}

	/* Close pSrcf_InPhySniffer file descriptor */
	if (!IS_FILE_OPEN_ERR(pAd->pSrcf_InPhySniffer)) {
		retval = RtmpOSFileClose(pAd->pSrcf_InPhySniffer);
		if (retval) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"--> Error %d closing %s\n", -retval, pAd->pSrc_InPhySniffer);
		}
		pAd->pSrcf_InPhySniffer = NULL;
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != CAP_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/*
	==========================================================================
	Description:
	Start for ICAP data capture.
	Return:
	==========================================================================
*/
INT32 MtCmdRfTestICapStart(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData)
{
	INT32 Status;
	CMD_TEST_CTRL_T TestCtrl;
	RBIST_CAP_START_T *prICapInfo = NULL;
	RBIST_CAP_START_T *prRBISTInfo = (RBIST_CAP_START_T *)pData;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data of pAd */
	pAd->ICapCapLen = prRBISTInfo->u4CaptureLen;
	pAd->ICapCapNode = prRBISTInfo->u4CaptureNode;
	pAd->ICapCapSrc = prRBISTInfo->u4CapSource;
	/* Update data structure of TestCtrl */
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = cpu2le32(ACTION_IN_RFTEST);
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(SET_ICAP_CAPTURE_START);
	prICapInfo = &TestCtrl.u.rRfATInfo.Data.rICapInfo;
	prICapInfo->fgTrigger = cpu2le32(prRBISTInfo->fgTrigger);
	prICapInfo->u4TriggerEvent = cpu2le32(prRBISTInfo->u4TriggerEvent);
	prICapInfo->u4CaptureNode = cpu2le32(prRBISTInfo->u4CaptureNode);
	prICapInfo->u4CaptureLen = cpu2le32(prRBISTInfo->u4CaptureLen);
	prICapInfo->u4CapStopCycle = cpu2le32(prRBISTInfo->u4CapStopCycle);
	prICapInfo->u4BW = cpu2le32(prRBISTInfo->u4BW);
	prICapInfo->u4PdEnable = cpu2le32(prRBISTInfo->u4PdEnable);
	prICapInfo->u4FixRxGain = cpu2le32(prRBISTInfo->u4FixRxGain);
	prICapInfo->u4WifiPath = cpu2le32(prRBISTInfo->u4WifiPath);
	prICapInfo->u4BandIdx = cpu2le32(prRBISTInfo->u4BandIdx);
	prICapInfo->u4PhyIdx = cpu2le32(prRBISTInfo->u4PhyIdx);
	prICapInfo->u4CapSource = cpu2le32(prRBISTInfo->u4CapSource);
	if (IS_MT7626(pAd))
		prICapInfo->u4EnBitWidth = cpu2le32(CAP_128_BIT);
	else if (IS_MT7986(pAd))
		prICapInfo->u4EnBitWidth = cpu2le32(CAP_BW_128B_TO_128B); /* keep original */
	else
		prICapInfo->u4EnBitWidth = cpu2le32(CAP_96_BIT);
	prICapInfo->u4Architech = cpu2le32(CAP_ON_CHIP);

	if (prICapInfo->u4TriggerEvent == CAP_FREE_RUN)
		prICapInfo->fgRingCapEn = cpu2le32(CAP_RING_MODE_DISABLE);
	else
		prICapInfo->fgRingCapEn = cpu2le32(CAP_RING_MODE_ENABLE);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"
			" prICapInfo->fgTrigger = 0x%08x\n prICapInfo->fgRingCapEn = 0x%08x\n"
			" prICapInfo->u4TriggerEvent = 0x%08x\n prICapInfo->u4CaptureNode = 0x%08x\n"
			" prICapInfo->u4CaptureLen = 0x%08x\n prICapInfo->u4CapStopCycle = 0x%08x\n"
			" prICapInfo->ucBW = 0x%08x\n prICapInfo->u4PdEnable = 0x%08x\n"
			" prICapInfo->u4FixRxGain = 0x%08x\n prICapInfo->u4WifiPath = 0x%08x\n"
			" prICapInfo->u4BandIdx = 0x%08x\n prICapInfo->u4PhyIdx = 0x%08x\n prICapInfo->u4CapSource = 0x%08x\n"
			" prICapInfo->u4EnBitWidth = 0x%08x\n prICapInfo->u4Architech = 0x%08x\n",
			prICapInfo->fgTrigger, prICapInfo->fgRingCapEn, prICapInfo->u4TriggerEvent,
			prICapInfo->u4CaptureNode, prICapInfo->u4CaptureLen, prICapInfo->u4CapStopCycle,
			prICapInfo->u4BW, prICapInfo->u4PdEnable, prICapInfo->u4FixRxGain,
			prICapInfo->u4WifiPath, prICapInfo->u4BandIdx, prICapInfo->u4PhyIdx,
			prICapInfo->u4CapSource, prICapInfo->u4EnBitWidth, prICapInfo->u4Architech);

	/* Set ICap parameter to HW */
	Status = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Query status of ICAP data capture.
	Return:
	==========================================================================
*/
INT32 MtCmdRfTestUnSolicitICapStatus(
	IN RTMP_ADAPTER *pAd)
{
	INT32 Status;
	CMD_TEST_CTRL_T TestCtrl;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of TestCtrl */
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = cpu2le32(ACTION_IN_RFTEST);
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(GET_ICAP_CAPTURE_STATUS);

	/* Query current captured status by unsolicited event */
	Status = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

INT32 MtCmdRfTestSolicitICapStatus(
	IN RTMP_ADAPTER *pAd)
{
	INT32 Status = CAP_BUSY;
	UINT32 CapDone = 0;
	CMD_TEST_CTRL_T TestCtrl;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of TestCtrl */
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = cpu2le32(ACTION_IN_RFTEST);
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(GET_ICAP_CAPTURE_STATUS);

	/* Query current captured status by solicited event */
	MtCmdRfTest(pAd, TestCtrl, (INT8 *)&CapDone, sizeof(EXT_EVENT_RBIST_CAP_STATUS_T));

	if (IS_MT7622(pAd) || IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7626(pAd)) {
		if (CapDone) { /* Stop capture */
			/* Dump captured data and store it to data buffer */
			if (ops->ICapCmdUnSolicitRawDataProc != NULL)
				Status = ops->ICapCmdUnSolicitRawDataProc(pAd);
			else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"The function is not hooked !!\n");
			}
		}
	} else {
		if (CapDone)
			Status = CAP_SUCCESS;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != CAP_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Callback function of querying status of ICAP data capture.
	Return:
	==========================================================================
*/
VOID MtCmdRfTestSolicitICapStatusCb(
	IN struct cmd_msg *msg,
	IN PINT32 pData,
	IN UINT16 Length)
{
	EXT_EVENT_RBIST_CAP_STATUS_T *pEventdata = (EXT_EVENT_RBIST_CAP_STATUS_T *)pData;
	UINT32 *pCapDone = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pCapDone */
	*pCapDone = le2cpu32(pEventdata->u4CapDone);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[42m%s\x1b[m\n", (*pCapDone == TRUE) ?
			"Capture done!!" : "Not yet!!");

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");
}

/*
	==========================================================================
	Description:
	Unsolicited raw data process of ICAP.
	Return:
	==========================================================================
*/
INT32 MtCmdRfTestUnSolicitICapRawDataProc(
	IN RTMP_ADAPTER *pAd)
{
	INT32 i, Status;
	CMD_TEST_CTRL_T TestCtrl;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;
	RBIST_DUMP_RAW_DATA_T *pICapDump = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update data structure of TestCtrl */
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = cpu2le32(ACTION_IN_RFTEST);
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(GET_ICAP_RAW_DATA);
	/* Initialization of OS wait completion */
	RTMP_OS_INIT_COMPLETION(&pAd->ICapDumpDataDone);
	/* Initialization of ICap status */
	Status = pAd->ICapStatus = CAP_BUSY;
	/* Initialization of ICapEventCnt */
	pAd->ICapEventCnt = 0;
	/* Initialization of ICapDataCnt */
	pAd->ICapDataCnt = 0;
	/* Initialization of L32/M32/H32 counter */
	pAd->ICapL32Cnt = pAd->ICapM32Cnt = pAd->ICapH32Cnt = 0;
	/* Initialization of IQ_Array */
	os_zero_mem(pIQ_Array, pChipCap->ICapMaxIQCnt * sizeof(RBIST_IQ_DATA_T));

	if (IS_MT7622(pAd)) {/* Query whole RBIST data by bank */
		for (i = 0; i < pChipCap->ICapBankNum; i++) {
			RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[i];

			/* Update data structure of TestCtrl */
			pICapDump = &TestCtrl.u.rRfATInfo.Data.rICapDump;
			pICapDump->u4Address = cpu2le32(pICapDesc->u4Address);
			pICapDump->u4AddrOffset = cpu2le32(pICapDesc->u4AddrOffset);
			pICapDump->u4Bank = cpu2le32(pICapDesc->u4Bank);
			pICapDump->u4BankSize = cpu2le32(pICapDesc->u4BankSize);
			/* Update ICapIdx which is referenced by pICapDesc */
			pAd->ICapIdx = i;

			/* Dump ICap data from RBIST sysram by unsolicited event */
			MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);

			/* OS wait for completion time out */
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ICapDumpDataDone,
					RTMPMsecsToJiffies(CAP_DUMP_DATA_EXPIRE))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m ICap dump data timeout !!\x1b[m\n");
				/* Update overall ICap status */
				pAd->ICapStatus = CAP_FAIL;
				goto error;
			}
		}
	} else {/* Query whole RBIST data at a time */
			/* Dump ICap data from RBIST sysram by unsolicited event */
			MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);

			/* OS wait for completion time out */
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ICapDumpDataDone,
					RTMPMsecsToJiffies(CAP_DUMP_DATA_EXPIRE))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m ICap dump data timeout !!\x1b[m\n");
				/* Update overall ICap status */
				pAd->ICapStatus = CAP_FAIL;
				goto error;
			}
		}

error:
	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Status != CAP_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Solicited raw data process of ICAP.
	Return:
	==========================================================================
*/
INT32 MtCmdRfTestSolicitICapRawDataProc(
	IN PRTMP_ADAPTER pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num)
{
	INT32 i, Status = 0, retval;
	CMD_TEST_CTRL_T TestCtrl;
	RBIST_DUMP_RAW_DATA_T *pICapDump = NULL;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Dynamic allocate memory for ICap event */
	retval = os_alloc_mem(pAd, (UCHAR **)&pICapEvent, sizeof(EXT_EVENT_RBIST_DUMP_DATA_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory for dynamic allocating !!\n");
		goto error;
	}
	os_zero_mem(pICapEvent, sizeof(EXT_EVENT_RBIST_DUMP_DATA_T));

	/* Update data structure of TestCtrl */
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = cpu2le32(ACTION_IN_RFTEST);
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(GET_ICAP_RAW_DATA);

	/* Update data structure of TestCtrl */
	pICapDump = &TestCtrl.u.rRfATInfo.Data.rICapDump;
	pICapDump->u4WFNum = WF_Num;
	pICapDump->u4IQType = IQ_Type;

	/* Dump ICap data from RBIST sysram by solicited event */
	Status = MtCmdRfTest(pAd, TestCtrl, (INT8 *)pICapEvent, sizeof(EXT_EVENT_RBIST_DUMP_DATA_T));

	/* If we receive the packet which is out of sequence, we need to drop it */
	if (pICapEvent->u4PktNum != pAd->ICapEventCnt) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"\x1b[31m Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				pICapEvent->u4PktNum, pAd->ICapEventCnt);
		goto error;
	}

	/* Store I/Q data to data buffer */
	*pDataLen = pICapEvent->u4DataLen;
	for (i = 0; i < *pDataLen; i++)
		pData[i] = (INT32)le2cpu32(pICapEvent->u4Data[i]);

	/* Check whether is the last FW event or not */
	if ((pICapEvent->u4DataLen == 0)
		&& (pICapEvent->u4PktNum == pAd->ICapEventCnt)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[31m: Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, pAd->ICapEventCnt);
	}

error:

	if (pICapEvent != NULL) {
		if (pICapEvent->u4DataLen == 0) {
			/* Reset ICapEventCnt */
			pAd->ICapEventCnt = 0;
		} else {
			/* Update ICapEventCnt */
			pAd->ICapEventCnt++;
		}

		os_free_mem(pICapEvent);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");

	return Status;
}

/*
	==========================================================================
	Description:
	Callback function of querying I/Q data of ICAP.
	Return:
	==========================================================================
*/
VOID MtCmdRfTestSolicitICapIQDataCb(
	IN struct cmd_msg *msg,
	IN PINT32 pData,
	IN UINT16 Length)
{
	EXT_EVENT_RBIST_DUMP_DATA_T *pEventdata = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pICapEvent */
	os_move_mem(pICapEvent, pEventdata, sizeof(EXT_EVENT_RBIST_DUMP_DATA_T));

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");
}
#endif /* INTERNAL_CAPTURE_SUPPORT */

INT32 MtCmdRfTestSetADC(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT32	ChannelFreq,
	IN	UINT8	AntIndex,
	IN	UINT8	BW,
	IN	UINT8	SX,
	IN	UINT8	DbdcIdx,
	IN	UINT8	RunType,
	IN	UINT8	FType)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	SET_ADC_T	*prSetADC;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = SET_ADC;
#ifdef RT_BIG_ENDIAN
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(TestCtrl.u.rRfATInfo.u4FuncIndex);
#endif
	prSetADC = &TestCtrl.u.rRfATInfo.Data.rSetADC;
	prSetADC->u4ChannelFreq = cpu2le32(ChannelFreq);
	prSetADC->ucAntIndex = AntIndex;
	prSetADC->ucBW = BW;
	prSetADC->ucSX = SX;
	prSetADC->ucDbdcIdx = DbdcIdx;
	prSetADC->ucRunType = RunType;
	prSetADC->ucFType = FType;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<SetADC> Structure parser Checking Log\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "--------------------------------------------------------------\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ChannelFreq = %d, AntIndex = %d, BW = %d, SX= %d, DbdcIdx = %d, RunType = %d, FType = %d\n",
			  ChannelFreq, prSetADC->ucAntIndex, prSetADC->ucBW, prSetADC->ucSX, prSetADC->ucDbdcIdx, prSetADC->ucRunType,
			  prSetADC->ucFType);
	ret = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdRfTestSetRxGain(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT8	LPFG,
	IN	UINT8	LNA,
	IN	UINT8	DbdcIdx,
	IN	UINT8	AntIndex)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	SET_RX_GAIN_T *prSetRxGain;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = SET_RX_GAIN;
#ifdef RT_BIG_ENDIAN
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(TestCtrl.u.rRfATInfo.u4FuncIndex);
#endif
	prSetRxGain = &TestCtrl.u.rRfATInfo.Data.rSetRxGain;
	prSetRxGain->ucLPFG = LPFG;
	prSetRxGain->ucLNA = LNA;
	prSetRxGain->ucDbdcIdx = DbdcIdx;
	prSetRxGain->ucAntIndex = AntIndex;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<SetRxGain> Structure parser Checking Log\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "--------------------------------------------------------------\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LPFG = %d, LNA = %d, DbdcIdx = %d, AntIndex= %d\n",
			 prSetRxGain->ucLPFG, prSetRxGain->ucLNA, prSetRxGain->ucDbdcIdx, prSetRxGain->ucAntIndex);
	ret = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdRfTestSetTTG(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT32	ChannelFreq,
	IN	UINT32	ToneFreq,
	IN	UINT8	TTGPwrIdx,
	IN	UINT8	XtalFreq,
	IN	UINT8	DbdcIdx)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	SET_TTG_T	*prSetTTG;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = SET_TTG;
#ifdef RT_BIG_ENDIAN
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(TestCtrl.u.rRfATInfo.u4FuncIndex);
#endif
	prSetTTG = &TestCtrl.u.rRfATInfo.Data.rSetTTG;
	prSetTTG->u4ChannelFreq = cpu2le32(ChannelFreq);
	prSetTTG->u4ToneFreq = cpu2le32(ToneFreq);
	prSetTTG->ucTTGPwrIdx = TTGPwrIdx;
	prSetTTG->ucXtalFreq = XtalFreq;
	prSetTTG->ucDbdcIdx = DbdcIdx;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<SetTTG> Structure parser Checking Log\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "--------------------------------------------------------------\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ChannelFreq = %d, ToneFreq = %d, TTGPwrIdx = %d, DbdcIdx= %d\n",
			 ChannelFreq, ToneFreq, prSetTTG->ucTTGPwrIdx, prSetTTG->ucDbdcIdx);
	ret = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdRfTestSetTTGOnOff(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT8	TTGEnable,
	IN	UINT8	DbdcIdx,
	IN	UINT8	AntIndex)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	TTG_ON_OFF_T *prTTGOnOff;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = TTG_ON_OFF;
#ifdef RT_BIG_ENDIAN
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(TestCtrl.u.rRfATInfo.u4FuncIndex);
#endif
	prTTGOnOff = &TestCtrl.u.rRfATInfo.Data.rTTGOnOff;
	prTTGOnOff->ucTTGEnable = TTGEnable;
	prTTGOnOff->ucDbdcIdx = DbdcIdx;
	prTTGOnOff->ucAntIndex = AntIndex;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<SetTTGOnOff> Structure parser Checking Log\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "--------------------------------------------------------------\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TTGEnable = %d, DbdcIdx = %d, AntIndex = %d\n",
			 prTTGOnOff->ucTTGEnable, prTTGOnOff->ucDbdcIdx, prTTGOnOff->ucAntIndex);
	ret = MtCmdRfTest(pAd, TestCtrl, NULL, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

static INT32 MtCmdRfTestTrigger(RTMP_ADAPTER *pAd,
								PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo, UINT16 rsp_len)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.ucIcapLen = RF_TEST_ICAP_LEN;
	os_move_mem(&TestCtrl.u.rRfATInfo, &rRfATInfo, sizeof(rRfATInfo));
	ret = MtCmdRfTest(pAd, TestCtrl, NULL, rsp_len);
	return ret;
}

INT32 MtCmdDoCalibration(RTMP_ADAPTER *pAd, UINT32 func_idx,
						 UINT32 item, UINT32 band_idx)
{
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	INT32 ret = 0;
	UINT16 rsp_len = RF_TEST_DEFAULT_RESP_LEN;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	rRfATInfo.u4FuncIndex = cpu2le32(func_idx);
#if defined(MT7615) ||  defined(MT7663) ||  defined(MT7626) || defined(AXE) || \
	defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)

	if (IS_MT7615(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd) || IS_AXE(pAd) ||
		IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		if ((IS_MT7615(pAd) || IS_MT7626(pAd) || IS_AXE(pAd)) && (item == CAL_ALL))
			rsp_len = RC_CAL_RESP_LEN;

		rRfATInfo.Data.rCalParam.u4FuncData = cpu2le32(item);
		rRfATInfo.Data.rCalParam.ucDbdcIdx = band_idx;
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "func_idx:%x, func_data:%x, band_idx:%x\n",
				  func_idx, item,
				  band_idx);
	} else
#endif
	{
		rRfATInfo.Data.u4FuncData = cpu2le32(item);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "else case\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Call RfTest\n");

	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, rsp_len);
	return ret;
}

INT32 MtCmdTxContinous(RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW,
					   UINT32 PriCh, UINT32 CentralCh, UINT32 Mcs, UINT32 WFSel,
					   UINT32 TxfdMode, UINT8 Band, UINT8 onoff)
{
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	INT32 ret = 0;
	UINT8 TXDRate = 0;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "mode:0x%x, bw:0x%x, prich(Control CH):0x%x, mcs:0x%x\n",
			  PhyMode, BW, PriCh, Mcs);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wfsel:0x%x, TxfdMode:0x%x, Band:0x%xon/off:0x%x\n",
			  WFSel, TxfdMode, Band, onoff);

	if (onoff == 0) {
		rRfATInfo.u4FuncIndex = CONTINUOUS_TX_STOP;
		rRfATInfo.Data.u4FuncData = Band;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
		rRfATInfo.Data.u4FuncData = cpu2le32(rRfATInfo.Data.u4FuncData);
#endif
	} else {
		rRfATInfo.u4FuncIndex = CONTINUOUS_TX_START;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
		/* 0: All 1:TX0 2:TX1 */
		rRfATInfo.Data.rConTxParam.ucCentralCh = (UINT8)CentralCh;
		rRfATInfo.Data.rConTxParam.ucCtrlCh = (UINT8)PriCh;
		rRfATInfo.Data.rConTxParam.ucAntIndex = (UINT8)WFSel;
		rRfATInfo.Data.rConTxParam.ucBW = (UINT8)BW;

		if (PhyMode == 0) { /* CCK */
			switch (Mcs) {
			/* long preamble */
			case 0:
				TXDRate = 0;
				break;

			case 1:
				TXDRate = 1;
				break;

			case 2:
				TXDRate = 2;
				break;

			case 3:
				TXDRate = 3;
				break;

			/* short preamble */
			case 9:
				TXDRate = 5;
				break;

			case 10:
				TXDRate = 6;
				break;

			case 11:
				TXDRate = 7;
				break;
			}
		} else if (PhyMode == 1) { /* OFDM */
			switch (Mcs) {
			case 0:
				TXDRate = 11;
				break;

			case 1:
				TXDRate = 15;
				break;

			case 2:
				TXDRate = 10;
				break;

			case 3:
				TXDRate = 14;
				break;

			case 4:
				TXDRate = 9;
				break;

			case 5:
				TXDRate = 13;
				break;

			case 6:
				TXDRate = 8;
				break;

			case 7:
				TXDRate = 12;
				break;
			}
		} else if (2 == PhyMode || 3 == PhyMode || 4 == PhyMode) {
			/* 2. MODULATION_SYSTEM_HT20 ||
			  *  3.MODULATION_SYSTEM_HT40 || 4. VHT
			  */
			TXDRate = (UINT8)Mcs;
		}

		rRfATInfo.Data.rConTxParam.u2RateCode = PhyMode << 6 | TXDRate;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.Data.rConTxParam.u2RateCode = cpu2le16(rRfATInfo.Data.rConTxParam.u2RateCode);
#endif
		rRfATInfo.Data.rConTxParam.ucBand = Band;
		rRfATInfo.Data.rConTxParam.ucTxfdMode = TxfdMode;
	}

	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdTxTone(RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 Control,
				  UINT8 AntIndex, UINT8 ToneType, UINT8 ToneFreq,
				  INT32 DcOffset_I, INT32 DcOffset_Q, UINT32 Band)
{
	INT32 ret = 0;
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Control:%d, AntIndex:%d, ToneType:%d, ToneFreq:%d\n",
			  Control, AntIndex, ToneType, ToneFreq);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx:%d, DcOffset_I:%d, DcOffset_Q:%d, Band:%d\n",
			  BandIdx, DcOffset_I, DcOffset_Q, Band);

	if (Control) {
		rRfATInfo.u4FuncIndex = TX_TONE_START;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
		rRfATInfo.Data.rTxToneParam.ucAntIndex = AntIndex;
		rRfATInfo.Data.rTxToneParam.ucToneType = ToneType;
		rRfATInfo.Data.rTxToneParam.ucToneFreq = ToneFreq;
		rRfATInfo.Data.rTxToneParam.ucDbdcIdx = BandIdx;
		rRfATInfo.Data.rTxToneParam.i4DcOffsetI = cpu2le32(DcOffset_I);
		rRfATInfo.Data.rTxToneParam.i4DcOffsetQ = cpu2le32(DcOffset_Q);
		rRfATInfo.Data.rTxToneParam.u4Band = cpu2le32(Band);
	} else {
		rRfATInfo.u4FuncIndex = TX_TONE_STOP;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
		rRfATInfo.Data.u4FuncData = cpu2le32(Band);
	}

	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}
/*type:
 * RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN
 * RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN
 * SET_TX_TONE_GAIN_OFFSET
 */
INT32 MtCmdTxTonePower(RTMP_ADAPTER *pAd, INT32 type,
					   INT32 dec, UINT8 TxAntennaSel, UINT8 Band)
{
	INT32 ret = 0;
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "type:%d, dec:%d, TxAntennaSel: %d\n",
			  type, dec, TxAntennaSel);
	rRfATInfo.u4FuncIndex = cpu2le32(type);

	/* 0: All 1:TX0 2:TX1 */
	switch (TxAntennaSel) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		rRfATInfo.Data.rTxToneGainParam.ucAntIndex = TxAntennaSel;
		break;

	default:    /* for future more than 3*3 ant */
		rRfATInfo.Data.rTxToneGainParam.ucAntIndex = TxAntennaSel - 1;
		break;
	}

	rRfATInfo.Data.rTxToneGainParam.ucTonePowerGain = (UINT8)dec;
	rRfATInfo.Data.rTxToneGainParam.ucBand = Band;
	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdRfTestGetTxTonePower(RTMP_ADAPTER *pAd, INT32 *pPower, UINT8 TxAntennaSel, UINT8 Band)
{
	INT32 ret = 0;
	CMD_TEST_CTRL_T TestCtrl;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TxAntennaSel: %d\n", TxAntennaSel);

	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = GET_TX_TONE_GAIN_OFFSET;

	switch (TxAntennaSel) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = TxAntennaSel;
		break;
	default:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = TxAntennaSel - 1;
		break;
	}

	TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucBand = Band;

	ret = MtCmdRfTest(pAd, TestCtrl, (char *)pPower, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdSetRDDTestExt(RTMP_ADAPTER *pAd, UINT32 rdd_idx,
						 UINT32 rdd_rx_sel, UINT32 is_start)
{
	INT32 ret = 0;
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "rdd_num:%u, IsStart:%d\n",
			  rdd_idx, is_start);


#ifdef MT_DFS_SUPPORT
	if (IS_SUPPORT_MULTIPLE_RDD_TEST(pAd)) {
		rRfATInfo.u4FuncIndex = RDD_TEST_MODE;
#ifdef RT_BIG_ENDIAN
		rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
		rRfATInfo.Data.rRDDParam.ucDfsCtrl = is_start;
		rRfATInfo.Data.rRDDParam.ucRddIdx = rdd_idx;
		rRfATInfo.Data.rRDDParam.ucRddRxSel = rdd_rx_sel;
		ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	} else
#endif
		ret = MtCmdSetRDDTest(pAd, is_start);

	return ret;
}

INT32 MtCmdSetRDDTest(RTMP_ADAPTER *pAd, UINT32 IsStart)
{
	INT32 ret = 0;
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IsStart:%d\n", IsStart);
	rRfATInfo.u4FuncIndex = RDD_TEST_MODE;
#ifdef RT_BIG_ENDIAN
	rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
	rRfATInfo.Data.u4FuncData = cpu2le32(IsStart);/* 0 Stop, 1 Start */
	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

INT32 MtCmdSetCalDump(RTMP_ADAPTER *pAd, UINT32 IsEnable)
{
	INT32 ret = 0;
	struct _PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	os_zero_mem(&rRfATInfo, sizeof(rRfATInfo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IsEnable = %d\n", IsEnable);
	rRfATInfo.u4FuncIndex = CAL_RESULT_DUMP_FLAG;
#ifdef RT_BIG_ENDIAN
	rRfATInfo.u4FuncIndex = cpu2le32(rRfATInfo.u4FuncIndex);
#endif
	rRfATInfo.Data.u4FuncData = cpu2le32(IsEnable);/* 0 Disable, 1 Enable */
	ret = MtCmdRfTestTrigger(pAd, rRfATInfo, RF_TEST_DEFAULT_RESP_LEN);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x05
 *	1: On
 *	2: Off
 *****************************************/
INT32 MtCmdRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 On)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_RADIO_ON_OFF_CTRL_T RadioOnOffCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: On = %d\n", __func__, On);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(RadioOnOffCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RADIO_ON_OFF_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&RadioOnOffCtrl, sizeof(RadioOnOffCtrl));

	if (On == WIFI_RADIO_ON)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_ON;
	else if (On == WIFI_RADIO_OFF)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_OFF;
	else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "unknown state, On=%d\n", On);
	}

	MtAndesAppendCmdMsg(msg, (char *)&RadioOnOffCtrl, sizeof(RadioOnOffCtrl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x06
 *****************************************/
INT32 MtCmdWiFiRxDisable(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_WIFI_RX_DISABLE_T WiFiRxDisable;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: WiFiRxDisable = %d\n", __func__, WIFI_RX_DISABLE);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(WiFiRxDisable));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_WIFI_RX_DISABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&WiFiRxDisable, sizeof(WiFiRxDisable));
	WiFiRxDisable.ucWiFiRxDisableCtrl = WIFI_RX_DISABLE;
	MtAndesAppendCmdMsg(msg, (char *)&WiFiRxDisable, sizeof(WiFiRxDisable));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x07
 *****************************************/
/*TODO: Star check to Hanmin*/

static VOID CmdExtPmMgtBitRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
}

INT32 MtCmdExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, MT_PWR_MGT_BIT_WIFI_T rPwrMgtBitWifi)
{
	struct cmd_msg *msg;
	EXT_CMD_PWR_MGT_BIT_T PwrMgtBitWifi = {0};
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PWR_MGT_BIT_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	WCID_SET_H_L(PwrMgtBitWifi.ucWlanIdxHnVer, PwrMgtBitWifi.ucWlanIdxL, rPwrMgtBitWifi.u2WlanIdx);
	PwrMgtBitWifi.ucPwrMgtBit = rPwrMgtBitWifi.ucPwrMgtBit;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "WlanIdx(%d), ucPwrMgtBit(%d)\n",
			  rPwrMgtBitWifi.u2WlanIdx, rPwrMgtBitWifi.ucPwrMgtBit);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PWR_MGT_BIT_WIFI);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtPmMgtBitRsp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PwrMgtBitWifi, sizeof(PwrMgtBitWifi));
	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
static VOID cmd_host_resume_done_ack_rsp(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _EVENT_EXT_CMD_RESULT_T *event_ext_cmd_result =
		(struct _EVENT_EXT_CMD_RESULT_T *)data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  event_ext_cmd_result->ucExTenCID);
	event_ext_cmd_result->u4Status = le2cpu32(event_ext_cmd_result->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "EventExtCmdResult.u4Status = 0x%x\n",
			 event_ext_cmd_result->u4Status);
}

INT32 mt_cmd_host_resume_done_ack(struct _RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	EXT_CMD_HOST_RESUME_DONE_ACK_T host_resume_done_ack = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_HOST_RESUME_DONE_ACK_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HOST_RESUME_DONE_ACK);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, cmd_host_resume_done_ack_rsp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&host_resume_done_ack, sizeof(host_resume_done_ack));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n" ret);
	return ret;
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

static VOID CmdExtPmStateCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
}

INT32 MtCmdExtPmStateCtrl(RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl)
{
#ifdef CONFIG_STA_SUPPORT
	UINT32	u4Feature = 0;
#endif /* CONFIG_STA_SUPPORT */
	struct cmd_msg *msg = NULL;
	EXT_CMD_PM_STATE_CTRL_T CmdPmStateCtrl = {0};
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PM_STATE_CTRL_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Fill parameter here*/
	WCID_SET_H_L(CmdPmStateCtrl.ucWlanIdxHnVer, CmdPmStateCtrl.ucWlanIdxL, PmStatCtrl.WlanIdx);
	CmdPmStateCtrl.ucOwnMacIdx = PmStatCtrl.OwnMacIdx;
	CmdPmStateCtrl.ucPmNumber = PmStatCtrl.PmNumber;
	CmdPmStateCtrl.ucPmState = PmStatCtrl.PmState;
	CmdPmStateCtrl.ucDbdcIdx = PmStatCtrl.DbdcIdx;
	os_move_mem(CmdPmStateCtrl.aucBssid, PmStatCtrl.Bssid, MAC_ADDR_LEN);

#ifdef CONFIG_STA_SUPPORT
	if ((PmStatCtrl.PmNumber == PM4) && (PmStatCtrl.PmState == ENTER_PM_STATE)) {
		CmdPmStateCtrl.ucDtimPeriod = PmStatCtrl.DtimPeriod;
		CmdPmStateCtrl.u2BcnInterval = cpu2le16(PmStatCtrl.BcnInterval);
		CmdPmStateCtrl.u4Aid = cpu2le32(PmStatCtrl.Aid);
		/* TODO: shiang-MT7615, fix me for this!! */
		/*
		 * TODO::7615::Hanmin::
		 * 1. Current, we need to provide a CR value or H/W will get a lot dummy packet when Fast_PSP
		 * 2. In the future, FW will control RxFilter totally, driver no need to provide this vlaue, when this
		 * feature is ready, this filed will be reserved.
		 */

#ifdef STA_LP_PHASE_2_SUPPORT
		CmdPmStateCtrl.u4RxFilter = cpu2le32(0);
		u4Feature = (PM_CMD_FEATURE_PSPOLL_OFFLOAD |
					 PM_CMD_FEATURE_PS_TX_REDIRECT |
					 PM_CMD_FEATURE_SMART_BCN_SP |
					 PM_CMD_FEATURE_SEND_NULL_FRAME);
#endif /* STA_LP_PHASE_2_SUPPORT */

#ifdef MT_WOW_SUUPORT
		if (pAd->WOW_Cfg.bEnable == TRUE)
			u4Feature |= PM_CMD_FEATURE_SEND_NULL_FRAME;
#endif

		CmdPmStateCtrl.u4Feature = cpu2le32(u4Feature);
		CmdPmStateCtrl.ucWmmIdx = PmStatCtrl.WmmIdx;
		CmdPmStateCtrl.ucBcnLossCount = PmStatCtrl.BcnLossCount; /* 2.5sec */
		CmdPmStateCtrl.ucBcnSpDuration = 0;
	} else if ((PmStatCtrl.PmNumber == PM4) && (PmStatCtrl.PmState == EXIT_PM_STATE)) {

		/* Need to provide previous setting to let fw restore original setting */
#ifdef STA_LP_PHASE_2_SUPPORT
		CmdPmStateCtrl.u4RxFilter = cpu2le32(0);
		u4Feature = (PM_CMD_FEATURE_PSPOLL_OFFLOAD |
					 PM_CMD_FEATURE_PS_TX_REDIRECT |
					 PM_CMD_FEATURE_SMART_BCN_SP |
					 PM_CMD_FEATURE_SEND_NULL_FRAME);
#endif /* STA_LP_PHASE_2_SUPPORT */

		CmdPmStateCtrl.u4Feature = cpu2le32(u4Feature);
	}

	MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Dbdc(%d),Wcid(%d),OwnMac(%d),BSSID(%x,%x,%x,%x,%x,%x),BcnPrd(%d),DtimPrd(%d),AID(%d),Feature(0x%x)\n",
			  CmdPmStateCtrl.ucDbdcIdx,
			  WCID_GET_H_L(CmdPmStateCtrl.ucWlanIdxHnVer, CmdPmStateCtrl.ucWlanIdxL),
			  CmdPmStateCtrl.ucOwnMacIdx,
			  PRINT_MAC((PmStatCtrl.Bssid)),
			  PmStatCtrl.BcnInterval,
			  PmStatCtrl.DtimPeriod,
			  PmStatCtrl.Aid,
			  CmdPmStateCtrl.u4Feature);

#endif /*CONFIG_STA_SUPPORT*/
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_PM_STATE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtPmStateCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdPmStateCtrl, sizeof(CmdPmStateCtrl));
	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

/*****************************************
 *	ExT_CID = 0x08
 *****************************************/
UCHAR GetCfgBw2RawBw(UCHAR CfgBw)
{
	switch (CfgBw) {
	case BW_20:
		return CMD_BW_20;

	case BW_40:
		return CMD_BW_40;

	case BW_80:
		return CMD_BW_80;

	case BW_160:
		return CMD_BW_160;

	case BW_10:
		return CMD_BW_10;

	case BW_5:
		return CMD_BW_5;

	case BW_8080:
		return CMD_BW_8080;

	default:
		return CMD_BW_20;
	}

	return CMD_BW_20;
}

#ifdef NEW_SET_RX_STREAM
/* TODO: temporary to keep channel setting */
MT_SWITCH_CHANNEL_CFG CurrentSwChCfg[2];
#endif

INT32 MtCmdChannelSwitch(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 TxPowerDrop = 0;
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;
#ifdef SINGLE_SKU_V2
	UCHAR fg5Gband = 0;
#endif
	UINT8 SKUIdx = 0;
#if defined(CONFIG_ATE)
	UINT8 rx_stream_num = 0, ant_seq = 0;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif	/* CONFIG_ATE */

	UCHAR tx_ant_num = 0, rx_ant_num = 0;
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter; */
#endif

	if (SwChCfg.CentralChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "central channel = 0 is invalid\n");
		return -1;
	}

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (SwChCfg.BandIdx == DBDC_BAND0) {
			ucTxPath = pAd->dbdc_band0_tx_path;
			SwChCfg.TxStream = pAd->dbdc_band0_tx_path;
			SwChCfg.RxStream = pAd->dbdc_band0_rx_path;
		} else {
			ucTxPath = pAd->dbdc_band1_tx_path;
			SwChCfg.TxStream = pAd->dbdc_band1_tx_path;
			SwChCfg.RxStream = pAd->dbdc_band1_rx_path;
		}
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable[SwChCfg.BandIdx]) {
			ucTxPath = pAd->TxStream[SwChCfg.BandIdx];
			SwChCfg.TxStream = ucTxPath;
			SwChCfg.RxStream = pAd->RxStream[SwChCfg.BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */


	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_CHANNEL_SWITCH);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 5000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdChanSwitch, sizeof(CmdChanSwitch));
	CmdChanSwitch.ucPrimCh = SwChCfg.ControlChannel;
	CmdChanSwitch.ucCentralCh = SwChCfg.CentralChannel;
	CmdChanSwitch.ucCentralCh2 = SwChCfg.ControlChannel2;
	CmdChanSwitch.ucTxStreamNum = SwChCfg.TxStream;
#if defined(CONFIG_ATE)
	if (ATE_ON(pAd)) {
		/* transform from bitwise to total */
		for (ant_seq = 0 ; ant_seq < GET_MAX_PATH(chip_cap, SwChCfg.BandIdx, 1) ; ant_seq++)
			if (SwChCfg.RxStream & (0x1 << ant_seq))
				rx_stream_num++;

		CmdChanSwitch.ucRxStreamNum = rx_stream_num;
	} else
#endif	/* CONFIG_ATE */
	{
		CmdChanSwitch.ucRxStreamNum = SwChCfg.RxStream;
	}
	CmdChanSwitch.ucDbdcIdx = SwChCfg.BandIdx;
	CmdChanSwitch.ucBW = GetCfgBw2RawBw(SwChCfg.Bw);
	CmdChanSwitch.ucBand = SwChCfg.Channel_Band;
	CmdChanSwitch.u4OutBandFreq = cpu2le32(SwChCfg.OutBandFreq);
	CmdChanSwitch.ucAPBW = GetCfgBw2RawBw(SwChCfg.ap_bw);
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* while BW160 zero-wait case, althought BW is 80, we need to set BW160 for listen RDD */
	if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) && (CmdChanSwitch.ucBW == BW_80)) {

		CmdChanSwitch.ucBW = BW_160;
		CmdChanSwitch.ucAPBW = BW_160;
		CmdChanSwitch.ucCentralCh = 50;
	}
#endif
#if defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
	if (pAd->CommonCfg.DfsParameter.BW160DedicatedZWSupport == TRUE
		&& pAd->CommonCfg.DfsParameter.BW160DedicatedZWState == DFS_BW160_TX80RX160
		&& (CmdChanSwitch.ucBW == BW_80)) {
		CmdChanSwitch.ucBW = BW_160;
		CmdChanSwitch.ucAPBW = BW_160;
		CmdChanSwitch.ucCentralCh = 50;
	}
#endif
	CmdChanSwitch.ucAPCentralCh = SwChCfg.ap_central_channel;
#ifdef COMPOS_TESTMODE_WIN

	if (SwChCfg.isMCC) {
		/* MCC */
		CmdChanSwitch.ucSwitchReason = CH_SWITCH_INTERNAL_USED_BY_FW_3;
	} else
#endif
		CmdChanSwitch.ucSwitchReason = CH_SWITCH_BY_NORMAL_TX_RX;

	if (SwChCfg.bScan) {

#ifdef MT_DFS_SUPPORT
		if (RadarChannelCheck(pAd, SwChCfg.ControlChannel))
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_DFS;
#endif

#if defined(MT7615) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)

		if (IS_MT7615(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_SCAN_BYPASS_DPD;

#endif

	}

#ifdef MT_DFS_SUPPORT
	else {
		if (SwChCfg.bDfsCheck)
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_DFS;

		if (SwChCfg.bDnlCal)
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_MP_LINE_DNL_CAL;
	}
#endif

	/* check Tx Power setting from UI. */
	if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 90) &&
		(pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] < 100))
		TxPowerDrop = 0;
	else if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 60) &&
			 (pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] <= 90))  /* reduce Pwr for 1 dB. */
		TxPowerDrop = 1;
	else if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 30) &&
			 (pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] <= 60))  /* reduce Pwr for 3 dB. */
		TxPowerDrop = 3;
	else if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 15) &&
			 (pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] <= 30))  /* reduce Pwr for 6 dB. */
		TxPowerDrop = 6;
	else if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 9) &&
			 (pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] <= 15))   /* reduce Pwr for 9 dB. */
		TxPowerDrop = 9;
	else if ((pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] > 0) &&
			 (pAd->CommonCfg.ucTxPowerPercentage[SwChCfg.BandIdx] <= 9))   /* reduce Pwr for 12 dB. */
		TxPowerDrop = 12;

	CmdChanSwitch.cTxPowerDrop = TxPowerDrop;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " TxPowerDrop = 0x%x, ucTxPath:%u\n", CmdChanSwitch.cTxPowerDrop, ucTxPath);

	for (SKUIdx = 0; SKUIdx < SKU_SIZE; SKUIdx++)
		CmdChanSwitch.acTxPowerSKU[SKUIdx] = 0x3F;

#ifdef SINGLE_SKU_V2

	if (SwChCfg.Channel_Band == 0) { /* Not 802.11j */
		if (SwChCfg.ControlChannel <= 14)
			fg5Gband = 0;
		else
			fg5Gband = 1;
	} else
		fg5Gband = 1;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " fg5Gband = 0x%x\n", fg5Gband);


	for (SKUIdx = 0; SKUIdx < SKU_TOTAL_SIZE; SKUIdx++)
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: CmdChanSwitch.aucTxPowerSKU[%d]: 0x%x\n", __func__, SKUIdx,
				 CmdChanSwitch.acTxPowerSKU[SKUIdx]);

	os_move_mem(pAd->TxPowerSKU, CmdChanSwitch.acTxPowerSKU, SKU_SIZE);

	for (SKUIdx = 0; SKUIdx < SKU_SIZE; SKUIdx++)
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: pAd->TxPowerSKU[%d]: 0x%x\n", __func__, SKUIdx,
				 pAd->TxPowerSKU[SKUIdx]);
#endif /* SINGLE_SKU_V2 */

	/* ant num error handling */
	tx_ant_num = SwChCfg.TxStream;
	rx_ant_num = SwChCfg.RxStream;


	{
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				 "ctrl_chl=%d, ctrl_ch2=%d, cent_ch=%d DBDCIdx=%d, ChBand=%d, BW=%d, TXStream=%d, RXStream=%d, scan(%d)\n",
				  SwChCfg.ControlChannel, SwChCfg.ControlChannel2,
				  SwChCfg.CentralChannel, SwChCfg.BandIdx, SwChCfg.Channel_Band, SwChCfg.Bw,
				  CmdChanSwitch.ucTxStreamNum, CmdChanSwitch.ucRxStreamNum, SwChCfg.bScan);

		if (SwChCfg.ap_bw) {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP BW = %d, AP central_chn = %d\n", SwChCfg.ap_bw, SwChCfg.ap_central_channel);
		}
	}

#ifdef SCAN_RADAR_COEX_SUPPORT
	if (SwChCfg.bScan && (pAd->oper_ch != CmdChanSwitch.ucCentralCh)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: Ignore RADAR Event for Channel %d\n", __func__,
				 CmdChanSwitch.ucCentralCh);
		CmdChanSwitch.ucSwitchReason += CH_IGNORE_RADAR;
	} else if (!DfsRadarChannelCheckForCMD(pAd, CmdChanSwitch.ucPrimCh, CmdChanSwitch.ucCentralCh2, SwChCfg.Bw)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: Ignore RADAR Event for Channel %d by DfsRadarChannelCheckForCMD\n", __func__, CmdChanSwitch.ucPrimCh);
		CmdChanSwitch.ucSwitchReason += CH_IGNORE_RADAR;
	}
#endif /* SCAN_RADAR_COEX_SUPPORT */

	MtAndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));
#ifdef NEW_SET_RX_STREAM
	/* TODO: temporary to keep channel setting */
	os_move_mem(&CurrentSwChCfg[SwChCfg.BandIdx], &SwChCfg, sizeof(MT_SWITCH_CHANNEL_CFG));
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	/* Backup swtich channel configuration for background scan */
	os_move_mem(&pAd->BgndScanCtrl.CurrentSwChCfg[SwChCfg.BandIdx], &SwChCfg, sizeof(MT_SWITCH_CHANNEL_CFG));
#endif
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef NEW_SET_RX_STREAM
INT MtCmdSetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 Path, UCHAR BandIdx)
{
	MT_SWITCH_CHANNEL_CFG *pSwChCfg = &CurrentSwChCfg[BandIdx];
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
#ifdef SINGLE_SKU_V2
	UINT8 ucTxPath = pAd->Antenna.field.TxPath;
	UCHAR fg5Gband = 0;
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		if (pAd->bAntennaSetAPEnable[BandIdx])
			ucTxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	if (pSwChCfg->CentralChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "central channel = 0 is invalid\n");
		return -1;
	}

	/* TODO: Pat: Update new path. It is rx path actually. */
	pSwChCfg->RxStream = Path;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "control_chl = %d,control_ch2=%d, central_chl = %d, BW = %d,TXStream = %d, RXStream = %d, BandIdx =%d,  scan(%d), Channel_Band = %d\n",
			  pSwChCfg->ControlChannel, pSwChCfg->ControlChannel2, pSwChCfg->CentralChannel,
			  pSwChCfg->Bw, pSwChCfg->TxStream, pSwChCfg->RxStream, pSwChCfg->BandIdx, pSwChCfg->bScan, pSwChCfg->Channel_Band);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RX_PATH);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdChanSwitch, sizeof(CmdChanSwitch));
	CmdChanSwitch.ucPrimCh = pSwChCfg->ControlChannel;
	CmdChanSwitch.ucCentralCh = pSwChCfg->CentralChannel;
	CmdChanSwitch.ucCentralCh2 = pSwChCfg->ControlChannel2;
	CmdChanSwitch.ucTxStreamNum = pSwChCfg->TxStream;
	CmdChanSwitch.ucRxStreamNum = pSwChCfg->RxStream;
	CmdChanSwitch.ucDbdcIdx = pSwChCfg->BandIdx;
	CmdChanSwitch.ucBW = GetCfgBw2RawBw(pSwChCfg->Bw);
	CmdChanSwitch.ucBand = pSwChCfg->Channel_Band;
	CmdChanSwitch.ucAPBW = GetCfgBw2RawBw(pSwChCfg->ap_bw);
	CmdChanSwitch.ucAPCentralCh = pSwChCfg->ap_central_channel;
	if (pSwChCfg->ap_bw) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP BW = %d, AP central_chn = %d\n", SwChCfg.ap_bw, SwChCfg.ap_central_channel);
	}

	MtAndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif

INT MtCmdSetTxRxPath(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0, i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MT_SWITCH_CHANNEL_CFG *pSwChCfg = &SwChCfg;
	UCHAR RxPath = 0;
	UCHAR tx_ant_num = 0, rx_ant_num = 0;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pSwChCfg->CentralChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "central channel = 0 is invalid\n");
		return -1;
	}

	/*for normal case*/
	tx_ant_num = pSwChCfg->TxStream;
	rx_ant_num = pSwChCfg->RxStream;

	/* ant num error handling */


	/* fPHY_CAP_BW160C_STD indicates support for BW160C on single stream rather than
	 * gathering two streams to achieve BW160. In this condition, we have to configure
	 * double ant num properly.
	 */
	if ((pSwChCfg->Bw == BW_160 || pSwChCfg->Bw == BW_8080) &&
		!IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160C_STD)) {
		/*if bw 160, 1 stream use WIFI (0,2), 2 stream use WIFI (0,1,2,3)*/
		tx_ant_num = (tx_ant_num > 1) ? 4 : 2;
		rx_ant_num = (rx_ant_num > 1) ? 4 : 2;

		RxPath = (rx_ant_num > 2) ? 0xf : 0x5;
	} else {
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			/*for normal case*/
			for (i = 0; i < SwChCfg.RxStream ; i++)
				RxPath |= (1 << i);
		} else {
			for (i = 0; i < SwChCfg.RxStream ; i++) {
				if ((pChipCap->phy_caps & fPHY_CAP_DUALPHY) == fPHY_CAP_DUALPHY)
					RxPath |= (1 << i);
#ifdef CONFIG_ATE
				else {
					if (!ATE_ON(pAd))
						RxPath |= (1 << (i + (SwChCfg.BandIdx * (2))));
				}
#endif
			}
		}
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RX_PATH);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdChanSwitch, sizeof(CmdChanSwitch));
	CmdChanSwitch.ucPrimCh = pSwChCfg->ControlChannel;
	CmdChanSwitch.ucCentralCh = pSwChCfg->CentralChannel;
	CmdChanSwitch.ucCentralCh2 = pSwChCfg->ControlChannel2;
	CmdChanSwitch.ucTxStreamNum = tx_ant_num;
	CmdChanSwitch.ucAPBW = GetCfgBw2RawBw(SwChCfg.ap_bw);
	/* For normal mode, use rx path which means rx stream capability.
	    For test mode, use rx path with bit wise.
	    Need to modiy tx/rx path and set channel flow here.
	 */
#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		CmdChanSwitch.ucRxStreamNum = pSwChCfg->RxStream;
	else
#endif /*CONFIG_ATE*/
		CmdChanSwitch.ucRxStreamNum = RxPath;

	CmdChanSwitch.ucDbdcIdx = pSwChCfg->BandIdx;
	CmdChanSwitch.ucBW = GetCfgBw2RawBw(pSwChCfg->Bw);
	CmdChanSwitch.ucBand = pSwChCfg->Channel_Band;
	CmdChanSwitch.u2CacCase = 0;
#ifdef DFS_ADJ_BW_ZERO_WAIT
	/* while BW160 zero-wait case, althought BW is 80, we need to set BW160 for listen RDD */
	if (IS_ADJ_BW_ZERO_WAIT_TX80RX160(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) && (CmdChanSwitch.ucBW == BW_80)) {

		CmdChanSwitch.ucBW = BW_160;
		CmdChanSwitch.ucAPBW = BW_160;
		CmdChanSwitch.ucCentralCh = 50;
	}
#endif
#if defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
	if (pAd->CommonCfg.DfsParameter.BW160DedicatedZWSupport == TRUE
		&& pAd->CommonCfg.DfsParameter.BW160DedicatedZWState == DFS_BW160_TX80RX160
		&& (CmdChanSwitch.ucBW == BW_80)) {
		CmdChanSwitch.ucBW = BW_160;
		CmdChanSwitch.ucAPBW = BW_160;
		CmdChanSwitch.ucCentralCh = 50;
	}
#endif

#ifdef COMPOS_TESTMODE_WIN

	if (SwChCfg.isMCC) {
		/* MCC */
		CmdChanSwitch.ucSwitchReason = CH_SWITCH_INTERNAL_USED_BY_FW_3;
	} else
#endif
		CmdChanSwitch.ucSwitchReason = CH_SWITCH_BY_NORMAL_TX_RX;

	if (SwChCfg.bScan) {
#ifdef MT_DFS_SUPPORT
		if (RadarChannelCheck(pAd, SwChCfg.ControlChannel))
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_DFS;
#endif

#ifdef OFFCHANNEL_ZERO_LOSS
#if defined(MT7615) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7615(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_SCAN_BYPASS_DPD;
#endif
#else
#if defined(MT7615) || defined(MT7915) || defined(MT7986) || defined(MT7916)

		if (IS_MT7615(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd))
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_SCAN_BYPASS_DPD;

#endif
#endif

	}

#ifdef MT_DFS_SUPPORT
	else {
		if (SwChCfg.bDfsCheck)
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_DFS;

		if (SwChCfg.bDnlCal)
			CmdChanSwitch.ucSwitchReason = CH_SWITCH_MP_LINE_DNL_CAL;
	}

#endif
	CmdChanSwitch.u2CacCase = cpu2le16(CmdChanSwitch.u2CacCase);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "ctrl_chl=%d, ctrl_ch2=%d, cent_ch=%d, RxPath=%x, BandIdx=%d, ChBand=%d, BW=%d,TXStream=%d, RXStream=%d, scan(%d)\n",
			  pSwChCfg->ControlChannel, pSwChCfg->ControlChannel2,
			  pSwChCfg->CentralChannel, RxPath, pSwChCfg->BandIdx, pSwChCfg->Channel_Band,
			  pSwChCfg->Bw, CmdChanSwitch.ucTxStreamNum, CmdChanSwitch.ucRxStreamNum, pSwChCfg->bScan);

#ifdef SCAN_RADAR_COEX_SUPPORT
	if (SwChCfg.bScan && (pAd->oper_ch != CmdChanSwitch.ucCentralCh)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: Ignore RADAR Event for Channel %d\n", __func__,
				 CmdChanSwitch.ucCentralCh);
		CmdChanSwitch.ucSwitchReason += CH_IGNORE_RADAR;
	} else if (!DfsRadarChannelCheckForCMD(pAd, CmdChanSwitch.ucPrimCh, CmdChanSwitch.ucCentralCh2, SwChCfg.Bw)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: Ignore RADAR Event for Channel %d by DfsRadarChannelCheckForCMD\n", __func__, CmdChanSwitch.ucPrimCh);
		CmdChanSwitch.ucSwitchReason += CH_IGNORE_RADAR;
	}
#endif /* SCAN_RADAR_COEX_SUPPORT */

	MtAndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));
	ret = chip_cmd_tx(pAd, msg);
error:
	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x0e
 *****************************************/
static VOID CmdMultipleMacRegAccessReadCb(struct cmd_msg *msg,
		char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len - 20) / sizeof(EXT_EVENT_MULTI_CR_ACCESS_RD_T);
	EXT_EVENT_MULTI_CR_ACCESS_RD_T *EventMultiCRAccessRD =
		(EXT_EVENT_MULTI_CR_ACCESS_RD_T *)(data + 20);
	RTMP_REG_PAIR *RegPair = (RTMP_REG_PAIR *)msg->attr.rsp.wb_buf_in_calbk;
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	RTMP_REG_PAIR *Start = RegPair;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

	for (Index = 0; Index < Num; Index++) {
		RegPair->Register = le2cpu32(EventMultiCRAccessRD->u4Addr);
		RegPair->Value = le2cpu32(EventMultiCRAccessRD->u4Data);
		EventMultiCRAccessRD++;
		RegPair++;
	}

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	RegPair = Start;
	for (Index = 0; Index < Num; Index++) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "0x%08x=0x%08x\n", RegPair->Register, RegPair->Value);
		RegPair++;
	}
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
}

INT32 MtCmdMultipleMacRegAccessRead(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair,
									UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_MULTIPLE_REG_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, ((12 * Num) + 20));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, RegPair);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdMultipleMacRegAccessReadCb);
	MtAndesInitCmdMsg(msg, attr);

	for (Index = 0; Index < Num; Index++) {
		os_zero_mem(&MultiCR, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32(MAC_CR);
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MtAndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
}

static VOID CmdMultipleMacRegAccessWriteCb(struct cmd_msg *msg,
		char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR =
		(EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);
	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 " fail\n");
	}
}

INT32 MtCmdMultipleMacRegAccessWrite(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair,
									 UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_MULTIPLE_REG_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 32);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdMultipleMacRegAccessWriteCb);
	MtAndesInitCmdMsg(msg, attr);

	for (Index = 0; Index < Num; Index++) {
		os_zero_mem(&MultiCR, sizeof(MultiCR));
		MultiCR.u4Type = MAC_CR;
#ifdef RT_BIG_ENDIAN
		MultiCR.u4Type = cpu2le32(MultiCR.u4Type);
#endif
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "offset: = %x\n", RegPair[Index].Register);
		MtAndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
}

static VOID CmdMultipleRfRegAccessWriteCb(struct cmd_msg *msg,
		char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR =
		(EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);
	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 " fail\n");
	}
}

INT32 MtCmdMultipleRfRegAccessWrite(RTMP_ADAPTER *pAd,
									MT_RF_REG_PAIR *RegPair, UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_MULTIPLE_REG_ACCESS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 32);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdMultipleRfRegAccessWriteCb);
	MtAndesInitCmdMsg(msg, attr);

	for (Index = 0; Index < Num; Index++) {
		os_zero_mem(&MultiCR, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32((RF_CR & 0xff) |
								  ((RegPair->WiFiStream & 0xffffff) << 8));
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);
		MtAndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
}

static VOID CmdMultipleMibRegAccessReadCb(struct cmd_msg *msg,
		char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len - 20) / sizeof(EXT_EVENT_MULTI_MIB_ACCESS_RD_T);

	EXT_EVENT_MULTI_MIB_ACCESS_RD_T *EventMultiMibAccessRD =
		(EXT_EVENT_MULTI_MIB_ACCESS_RD_T *)(data);

	RTMP_MIB_PAIR *RegPair = (RTMP_MIB_PAIR *)msg->attr.rsp.wb_buf_in_calbk;

	for (Index = 0; Index < Num; Index++) {
		RegPair->Counter = le2cpu32(EventMultiMibAccessRD->u4Counter);
		RegPair->Value = le2cpu64(EventMultiMibAccessRD->u8Data);
		EventMultiMibAccessRD++;
		RegPair++;
	}
}

INT32 MtCmdMultipleMibRegAccessRead(RTMP_ADAPTER *pAd, UCHAR ChIdx, RTMP_MIB_PAIR *RegPair,
									UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_MIB_ACCESS_T MultiMIB;
	INT32 Ret;
	UINT32 Index;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_MIB_ACCESS_T) * Num);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MIB_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, ((sizeof(CMD_MULTI_MIB_ACCESS_T) * Num) + 20));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, RegPair);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdMultipleMibRegAccessReadCb);
	MtAndesInitCmdMsg(msg, attr);

	for (Index = 0; Index < Num; Index++) {
		os_zero_mem(&MultiMIB, sizeof(MultiMIB));
		MultiMIB.u4Band = cpu2le32(ChIdx);
		MultiMIB.u4Counter = cpu2le32(RegPair[Index].Counter);
		MtAndesAppendCmdMsg(msg, (char *)&MultiMIB, sizeof(MultiMIB));
	}

	Ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
}

/*****************************************
 *	ExT_CID = 0x10
 *****************************************/
static VOID CmdSecKeyRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_SEC_ADDREMOVE_STRUC_T EvtSecKey;
	UINT32 Status;
	UINT32 WlanIndex;
	EvtSecKey = (struct _EVENT_SEC_ADDREMOVE_STRUC_T *)Data;
	Status = le2cpu32(EvtSecKey->u4Status);
	WlanIndex = le2cpu32(EvtSecKey->u4WlanIdx);

	if (Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "error set key, wlan idx(%d), status: 0x%x\n",
				  WlanIndex, Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wlan idx(%d), status: 0x%x\n",
				  WlanIndex, Status);
	}
}

INT32 MtCmdSecKeyReq(RTMP_ADAPTER *pAd, UINT8 AddRemove, UINT8 Keytype,
					 UINT8 *pAddr, UINT8 Alg, UINT8 KeyID, UINT8 KeyLen,
					 UINT8 WlanIdx, UINT8 *KeyMaterial)
{
	struct cmd_msg *msg;
	struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T CmdSecKey;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdSecKey));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_SEC_ADDREMOVE_KEY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_SEC_ADDREMOVE_STRUC_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdSecKeyRsp);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdSecKey, sizeof(CmdSecKey));
	CmdSecKey.ucAddRemove = AddRemove;
	CmdSecKey.ucKeyType = Keytype;
	os_move_mem(CmdSecKey.aucPeerAddr, pAddr, 6);
	CmdSecKey.ucAlgorithmId = Alg;
	CmdSecKey.ucKeyId = KeyID;
	CmdSecKey.ucKeyLen = KeyLen;
	os_move_mem(CmdSecKey.aucKeyMaterial, KeyMaterial, KeyLen);
	CmdSecKey.ucWlanIndex = WlanIdx;
	MtAndesAppendCmdMsg(msg, (char *)&CmdSecKey, sizeof(CmdSecKey));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x11
 *****************************************/
#ifndef COMPOS_WIN
#ifdef CONFIG_ATE
static INT32 MtCmdFillTxPowerInfo(RTMP_ADAPTER *pAd, EXT_CMD_TX_POWER_CTRL_T *CmdTxPwrCtrl, ATE_TXPOWER TxPower)
{
	INT32 ret = 0;
	/*
	UINT32 Group = MtATEGetTxPwrGroup(TxPower.Channel, TxPower.Band_idx, TxPower.Ant_idx);
	*/

	return ret;
}

static VOID MtCmdGetTxPowerRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_ID_GET_TX_POWER_T prEventExtCmdResult = (P_EXT_EVENT_ID_GET_TX_POWER_T)Data;
	P_EXT_EVENT_ID_GET_TX_POWER_T prTxPower = (P_EXT_EVENT_ID_GET_TX_POWER_T)msg->attr.rsp.wb_buf_in_calbk;

	prTxPower->i1TargetPower = prEventExtCmdResult->i1TargetPower;
	prTxPower->u1BandIdx = prEventExtCmdResult->u1BandIdx;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Power: 0x%x Band: %d\n",
				prTxPower->i1TargetPower, prTxPower->u1BandIdx);
}

INT32 MtCmdSetTxPowerCtrl(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	struct cmd_msg *msg;
	EXT_CMD_TX_POWER_CTRL_T CmdTxPwrCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx: %d, Power: %d, AntIdx: %d\n",
			 TxPower.Dbdc_idx, TxPower.Power, TxPower.Ant_idx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TX_POWER_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CmdTxPwrCtrl, sizeof(EXT_CMD_TX_POWER_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		CmdTxPwrCtrl.u1PowerCtrlFormatId = TX_POWER_SET_TARGET_POWER_V0;
	else
		CmdTxPwrCtrl.u1PowerCtrlFormatId = TX_POWER_SET_TARGET_POWER_V1;
	CmdTxPwrCtrl.u1DbdcIdx = TxPower.Dbdc_idx;
	CmdTxPwrCtrl.i1TargetPower = TxPower.Power;
	CmdTxPwrCtrl.u1AntIdx = TxPower.Ant_idx;
	CmdTxPwrCtrl.u1CenterChannel = TxPower.Channel;
	MtCmdFillTxPowerInfo(pAd, &CmdTxPwrCtrl, TxPower);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdTxPwrCtrl,
						sizeof(EXT_CMD_TX_POWER_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdGetTxPower(RTMP_ADAPTER *pAd, UINT8 u1DbDcIdx, UINT8 u1CenterCh, UINT8 u1AntIdx, P_EXT_EVENT_ID_GET_TX_POWER_T prTxPwrResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_TX_POWER_T CmdTxPwrGetCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u1CenterCh: %d, u1BandIdx: %d, u1AntIdx: %d\n", u1CenterCh, u1DbDcIdx, u1AntIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_TX_POWER_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CmdTxPwrGetCtrl, sizeof(EXT_CMD_GET_TX_POWER_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		CmdTxPwrGetCtrl.u1PowerCtrlFormatId = TX_POWER_GET_TARGET_POWER_V0;
	else
		CmdTxPwrGetCtrl.u1PowerCtrlFormatId = TX_POWER_GET_TARGET_POWER_V1;
	CmdTxPwrGetCtrl.u1DbdcIdx           = u1DbDcIdx;
	CmdTxPwrGetCtrl.u1AntIdx            = u1AntIdx;
	CmdTxPwrGetCtrl.u1CenterCh          = u1CenterCh;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_ID_GET_TX_POWER_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prTxPwrResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetTxPowerRsp);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdTxPwrGetCtrl,
						sizeof(EXT_CMD_GET_TX_POWER_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetForceTxPowerCtrl(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, INT8 cTxPower, UINT8 ucPhyMode, UINT8 ucTxRate, UINT8 ucBW)
{
	struct cmd_msg *msg;
	CMD_POWER_RATE_TXPOWER_CTRL_T CmdTxPwrCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
			ucBandIdx, ucPhyMode, ucTxRate, ucBW, cTxPower);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_RATE_TXPOWER_CTRL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&CmdTxPwrCtrl, sizeof(CmdTxPwrCtrl));

	if (cap->txpower_type == TX_POWER_TYPE_V0)
		CmdTxPwrCtrl.ucPowerCtrlFormatId  = TX_RATE_POWER_CTRL_V0;
	else
		CmdTxPwrCtrl.ucPowerCtrlFormatId  = TX_RATE_POWER_CTRL_V1;
	CmdTxPwrCtrl.ucPhyMode			= ucPhyMode;
	CmdTxPwrCtrl.ucTxRate			 = ucTxRate;
	CmdTxPwrCtrl.ucBW				 = ucBW;
	CmdTxPwrCtrl.ucBandIdx			= ucBandIdx;
	CmdTxPwrCtrl.cTxPower			 = cTxPower;

	MtAndesAppendCmdMsg(msg, (char *)&CmdTxPwrCtrl, sizeof(CmdTxPwrCtrl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* CONFIG_ATE */
#endif /* COMPOS_WIN */

/*****************************************
 *	ExT_CID = 0x12 Thermal Cal
 *
 *	ucEnable            0:Disable; 1:Enable
 *	ucSourceMode       0:EFuse; 1:Buffer mode.
 *	ucRFDiffTemp		 Indicate the temperature difference to trigger RF re-calibration.
 *	The default value in MT7603 is +/- 40.
 *
 *	ucHiBBPHT         Set the high temperature threshold to trigger HT BBP calibration.
 *	ucHiBBPNT         Set the normal Temperature threshold to trigger NT BBP calibration.
 *
 *	cLoBBPLT          Set the low temperature threshold to trigger LT BBP calibration.
 *			    It's might be a negative value.
 *	cLoBBPNT          Set the normal temperature threshold to trigger NT BBP calibration.
 *			    It's might be a negative value.
 *	For default setting please set ucRFDiffTemp/ ucHiBBPHT/ucHiBBPNT/cLoBBPLT/cLoBBPNT to 0xFF,
 *	Otherwise, FW will set calibration as these input parameters.
 *****************************************/
INT32 MtCmdThermoCal(RTMP_ADAPTER *pAd, UINT8 IsEnable, UINT8 SourceMode,
					 UINT8 RFDiffTemp, UINT8 HiBBPHT, UINT8 HiBBPNT, INT8 LoBBPLT, INT8 LoBBPNT)
{
	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _CMD_SET_THERMO_CAL_T Thermo;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&Thermo, sizeof(Thermo));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IsEnable = %d, SourceMode = %d, RFDiffTemp = %d\n",
			  IsEnable, SourceMode, RFDiffTemp);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "sizeof(Thermo) = %lu\n", (ULONG)sizeof(Thermo));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(Thermo));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMO_CAL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	Thermo.ucEnable = IsEnable;
	Thermo.ucSourceMode = SourceMode;
	Thermo.ucRFDiffTemp = RFDiffTemp;
	Thermo.ucHiBBPHT = HiBBPHT;
	Thermo.ucHiBBPNT = HiBBPNT;
	Thermo.cLoBBPLT = LoBBPLT;
	Thermo.cLoBBPNT = LoBBPNT;
#ifndef COMPOS_WIN/* windows ndis does not have EEPROMImage */
	Thermo.ucThermoSetting[0].u2Addr = 0x53;
	Thermo.ucThermoSetting[0].ucValue =
		pAd->EEPROMImage[Thermo.ucThermoSetting[0].u2Addr];
	Thermo.ucThermoSetting[1].u2Addr = 0x54;
	Thermo.ucThermoSetting[1].ucValue =
		pAd->EEPROMImage[Thermo.ucThermoSetting[1].u2Addr];
	Thermo.ucThermoSetting[2].u2Addr  = 0x55;
	Thermo.ucThermoSetting[2].ucValue =
		pAd->EEPROMImage[Thermo.ucThermoSetting[2].u2Addr];
#ifdef RT_BIG_ENDIAN
	Thermo.ucThermoSetting[0].u2Addr = cpu2le16(Thermo.ucThermoSetting[0].u2Addr);
	Thermo.ucThermoSetting[1].u2Addr = cpu2le16(Thermo.ucThermoSetting[1].u2Addr);
	Thermo.ucThermoSetting[2].u2Addr  = cpu2le16(Thermo.ucThermoSetting[2].u2Addr);
#endif
#endif
	MtAndesAppendCmdMsg(msg, (char *)&Thermo, sizeof(Thermo));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x13
 *****************************************/
INT32 MtCmdFwLog2Host(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT8 FWLog2HostCtrl)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_FW_LOG_2_HOST_CTRL_T CmdFwLog2HostCtrl;
	struct _CMD_ATTRIBUTE attr = {0};

	RTMP_STRING *Dest[] = {
		"N9(WM)",
		"CR4(WA)",
		"WO"
	};
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "McuDest(%d): %s\n", McuDest, Dest[McuDest]);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdFwLog2HostCtrl));

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "AllocCmdMsg FAIL!!\n");
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}
	if (McuDest < 2) {
		SET_CMD_ATTR_MCU_DEST(attr, McuDest == 1 ? HOST2CR4 : McuDest);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_FW_LOG_2_HOST);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	}
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdFwLog2HostCtrl, sizeof(CmdFwLog2HostCtrl));
	CmdFwLog2HostCtrl.ucFwLog2HostCtrl = FWLog2HostCtrl;
	MtAndesAppendCmdMsg(msg, (char *)&CmdFwLog2HostCtrl,
						sizeof(CmdFwLog2HostCtrl));
	if (McuDest < 2)
		Ret = chip_cmd_tx(pAd, msg);
	else {
		Ret = call_fw_cmd_notifieriers(WO_CMD_FW_LOG_CTRL, pAd, msg->net_pkt);
		AndesFreeCmdMsg(msg);
	}
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s (ret = %d)\n", Dest[McuDest], Ret);

	return Ret;
}

/*****************************************
 *	ExT_CID = 0x95
 *****************************************/
INT32 MtCmdFwDbgCtrl(RTMP_ADAPTER *pAd, UINT8 dbg_lvl, UINT32 module_idx)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	EXT_CMD_FW_DBG_CTRL_T fw_dbg_ctrl;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(fw_dbg_ctrl));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_FW_DBG_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&fw_dbg_ctrl, sizeof(fw_dbg_ctrl));
	fw_dbg_ctrl.ucDbgClass = dbg_lvl;
	fw_dbg_ctrl.u4DbgModuleIdx = module_idx;
#ifdef RT_BIG_ENDIAN
	fw_dbg_ctrl.u4DbgModuleIdx = cpu2le32(fw_dbg_ctrl.u4DbgModuleIdx);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&fw_dbg_ctrl,
						sizeof(fw_dbg_ctrl));
	Ret = chip_cmd_tx(pAd, msg);

	return Ret;
}

/*****************************************
 *	ExT_CID = 0x15
 *****************************************/
#ifdef CONFIG_MULTI_CHANNEL
INT MtCmdMccStart(struct _RTMP_ADAPTER *pAd, UINT32 Num,
				  MT_MCC_ENTRT_T *MccEntries, USHORT IdleTime, USHORT NullRepeatCnt, ULONG StartTsf)
{
	struct cmd_msg *msg;
	EXT_CMD_MCC_START_T mcc_start_msg;
	int ret;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[31m @@@@@ channel(%u,%u), bw(%u,%u), role(%u,%u)\n",
			  MccEntries[0].Channel, MccEntries[1].Channel,
			  MccEntries[0].Bw, MccEntries[1].Bw, MccEntries[0].Role,
			  MccEntries[1].Role);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "cycle_time(%u,%u), wait_time(%u), null_cnt(%u), start_tsf(%ld)\x1b[m\n",
			  MccEntries[0].StayTime, MccEntries[1].StayTime,
			  IdleTime, NullRepeatCnt, StartTsf);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_MCC_START_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MCC_OFFLOAD_START);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&mcc_start_msg, sizeof(EXT_CMD_MCC_START_T));
	mcc_start_msg.u2IdleInterval = cpu2le16(IdleTime); /* ms */
	mcc_start_msg.ucRepeatCnt = (UINT8)NullRepeatCnt;
	mcc_start_msg.ucStartIdx = 0;
	mcc_start_msg.u4StartInstant = cpu2le32(StartTsf);
	mcc_start_msg.u2FreePSEPageTh = 0x11; /* 0:  Disable PSE threshold check */
#ifdef RT_BIG_ENDIAN
	mcc_start_msg.u2FreePSEPageTh = cpu2le16(mcc_start_msg.u2FreePSEPageTh);
#endif
	mcc_start_msg.ucPreSwitchInterval = 0; /* for SDIO */
	mcc_start_msg.ucWlanIdx0 = MccEntries[0].WlanIdx;
	mcc_start_msg.ucPrimaryChannel0 =  MccEntries[0].Channel;
	mcc_start_msg.ucCenterChannel0Seg0 = MccEntries[0].CentralSeg0;
	mcc_start_msg.ucCenterChannel0Seg1 = MccEntries[0].CentralSeg1;
	mcc_start_msg.ucBandwidth0 = MccEntries[0].Bw;
	mcc_start_msg.ucTrxStream0 = 0; /* 2T2R  */
	mcc_start_msg.u2StayInterval0 = cpu2le16(MccEntries[0].StayTime);
	mcc_start_msg.ucRole0 = MccEntries[0].Role;
	mcc_start_msg.ucOmIdx0 = MccEntries[0].OwnMACAddressIdx;
	mcc_start_msg.ucBssIdx0 = MccEntries[0].BssIdx;
	mcc_start_msg.ucWmmIdx0 = MccEntries[0].WmmIdx;
	mcc_start_msg.ucWlanIdx1 = MccEntries[1].WlanIdx;
	mcc_start_msg.ucPrimaryChannel1 = MccEntries[1].Channel;
	mcc_start_msg.ucCenterChannel1Seg0 = MccEntries[1].CentralSeg0;
	mcc_start_msg.ucCenterChannel1Seg1 = MccEntries[1].CentralSeg1;
	mcc_start_msg.ucBandwidth1 = MccEntries[1].Bw;
	mcc_start_msg.ucTrxStream1 = 0; /* 2T2R */
	mcc_start_msg.u2StayInterval1 = cpu2le16(MccEntries[1].StayTime);
	mcc_start_msg.ucRole1 = MccEntries[1].Role;
	mcc_start_msg.ucOmIdx1 = MccEntries[1].OwnMACAddressIdx;
	mcc_start_msg.ucBssIdx1 = MccEntries[1].BssIdx;
	mcc_start_msg.ucWmmIdx1 = MccEntries[1].WmmIdx;
	MtAndesAppendCmdMsg(msg, (char *)&mcc_start_msg, sizeof(EXT_CMD_MCC_START_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	return ret;
}
#endif /*CONFIG_MULTI_CHANNEL*/

/*****************************************
 *	ExT_CID = 0x16
 *****************************************/
#ifdef CONFIG_MULTI_CHANNEL
INT32 MtCmdMccStop(struct _RTMP_ADAPTER *pAd, UCHAR ParkingIndex,
				   UCHAR AutoResumeMode, UINT16 AutoResumeInterval, ULONG AutoResumeTsf)
{
	struct cmd_msg *msg;
	EXT_CMD_MCC_STOP_T mcc_stop_msg;
	int ret;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[32m @@@@@  parking_channel_idx(%u)\n",
			  ParkingIndex);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "auto_resume_mode(%u), auto_resume_tsf(0x%08x) \x1b[m\n",
			  AutoResumeMode, AutoResumeTsf);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_MCC_STOP_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&mcc_stop_msg, sizeof(EXT_CMD_MCC_STOP_T));
	mcc_stop_msg.ucParkIdx = ParkingIndex;
	mcc_stop_msg.ucAutoResumeMode = AutoResumeMode;
	mcc_stop_msg.u2AutoResumeInterval = cpu2le16(AutoResumeInterval);
	mcc_stop_msg.u4AutoResumeInstant = cpu2le32(AutoResumeTsf);
	mcc_stop_msg.u2IdleInterval  = 0; /* no resume */
	mcc_stop_msg.u2StayInterval0 = 0; /* no resume */
	mcc_stop_msg.u2StayInterval1 = 0; /* no resume */
#ifdef RT_BIG_ENDIAN
	mcc_stop_msg.u2IdleInterval  = cpu2le16(mcc_stop_msg.u2IdleInterval);
	mcc_stop_msg.u2StayInterval0 = cpu2le16(mcc_stop_msg.u2StayInterval0);
	mcc_stop_msg.u2StayInterval1 = cpu2le16(mcc_stop_msg.u2StayInterval1);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&mcc_stop_msg, sizeof(EXT_CMD_MCC_STOP_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	return ret;
}
#endif /* CONFIG_MULTI_CHANNEL */

/*****************************************
 *	ExT_CID = 0x17
 *****************************************/
INT32 MtCmdLEDCtrl(
	RTMP_ADAPTER *pAd,
	UINT32    LEDNumber,
	UINT32    LEDBehavior)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_ID_LED_T    ExtLedCMD;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_LED_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		return ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&ExtLedCMD, sizeof(EXT_CMD_ID_LED_T));
	/* Filled RF Reg Access CMD setting */
	ExtLedCMD.u4LedNo = cpu2le32(LEDNumber);
	ExtLedCMD.u4LedCtrl = cpu2le32(LEDBehavior);
	MtAndesAppendCmdMsg(msg, (char *)&ExtLedCMD,
						sizeof(EXT_CMD_ID_LED_T));
	ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "LEDNumber: %x, LEDBehavior: %d\n",
			  LEDNumber, LEDBehavior);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x1e
 *****************************************/


#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
INT32 CmdETxBfAidSetting(
	RTMP_ADAPTER *pAd,
	UINT_16       u2Aid,
	UINT_8        u1BandIdx,
	UINT_8        u1OwnMacIdx)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_PEER_AID_T peer_aid = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Aid:%u, Band:%u, OwnMac:%u\n", u2Aid, u1BandIdx, u1OwnMacIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PEER_AID_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	peer_aid.u1CmdCategoryID = BF_AID_SET;
	peer_aid.u1WlanIdxL= 0; /* not use*/
	peer_aid.u1WlanIdxHnVer = 0; /* not use*/
	peer_aid.u2Aid = cpu2le32(u2Aid);
	peer_aid.u1OwnMacIdx = u1OwnMacIdx;
	peer_aid.u1BandIdx = u1BandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&peer_aid, sizeof(EXT_CMD_PEER_AID_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfApClientCluster(
	RTMP_ADAPTER *pAd,
	UINT16       u2WlanIdx,
	UINT_8       ucCmmWlanId)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_TXBf_APCLIENT_CLUSTER_T rBfApClientCluster;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "WlanIdx = %d, ucPfmuIdx = %d\n", u2WlanIdx, ucCmmWlanId);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBf_APCLIENT_CLUSTER_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rBfApClientCluster.ucPfmuProfileFormatId = BF_APCLIENT_CLUSTER;
	WCID_SET_H_L(rBfApClientCluster.ucWlanIdxHnVer, rBfApClientCluster.ucWlanIdxL, u2WlanIdx);
	rBfApClientCluster.ucCmmWlanId           = ucCmmWlanId;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rBfApClientCluster, sizeof(EXT_CMD_TXBf_APCLIENT_CLUSTER_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *pAd,
	UINT16       u2WlanIdx,
	UINT_8       ucCliIdx)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_REPT_CLONED_STA_BF_T rBfReptClonedStaToNormalSta;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "WlanIdx = %d, ucCliIdx = %d\n", u2WlanIdx, ucCliIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_REPT_CLONED_STA_BF_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rBfReptClonedStaToNormalSta.ucCmdCategoryID = BF_REPT_CLONED_STA_TO_NORMAL_STA;
	WCID_SET_H_L(rBfReptClonedStaToNormalSta.ucWlanIdxHnVer, rBfReptClonedStaToNormalSta.ucWlanIdxL, u2WlanIdx);
	rBfReptClonedStaToNormalSta.ucCliIdx        = ucCliIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rBfReptClonedStaToNormalSta, sizeof(EXT_CMD_REPT_CLONED_STA_BF_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdITxBfPhaseCal(
	RTMP_ADAPTER *pAd,
	UCHAR        ucGroup,
	UCHAR        ucGroup_L_M_H,
	BOOLEAN	   fgSX2,
	UCHAR        ucPhaseCalType,
	UCHAR        ucPhaseVerifyLnaGainLevel)
{
	struct cmd_msg *msg;
	EXT_CMD_ITXBf_PHASE_CAL_CTRL_T aucIBfPhaseCal;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Enable iBF phase calibration : ucGroup = %d, ucGroup_L_M_H = %d, fgSX2 = %d\n",
			  ucGroup, ucGroup_L_M_H, fgSX2);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ITXBf_PHASE_CAL_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&aucIBfPhaseCal, sizeof(EXT_CMD_ITXBf_PHASE_CAL_CTRL_T));
	aucIBfPhaseCal.ucCmdCategoryID = BF_PHASE_CALIBRATION;
	aucIBfPhaseCal.ucGroup         = ucGroup;
	aucIBfPhaseCal.ucGroup_L_M_H   = ucGroup_L_M_H;
	aucIBfPhaseCal.fgSX2           = fgSX2;
	aucIBfPhaseCal.ucPhaseCalType  = ucPhaseCalType;
	aucIBfPhaseCal.ucPhaseVerifyLnaGainLevel = ucPhaseVerifyLnaGainLevel;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucIBfPhaseCal, sizeof(EXT_CMD_ITXBf_PHASE_CAL_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfLnaGain(
	RTMP_ADAPTER *pAd,
	UCHAR        ucLnaGain)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UCHAR aucCmdBuf[4];
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LNA gain setting for iBF phase calibration : %d\n",
			 ucLnaGain);
	msg = MtAndesAllocCmdMsg(pAd, 4);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	aucCmdBuf[0] = BF_LNA_GAIN_CONFIG;
	aucCmdBuf[1] = ucLnaGain;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)aucCmdBuf, 4);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdITxBfPhaseComp(
	RTMP_ADAPTER *pAd,
	UCHAR        ucBW,
	UCHAR        ucBand,
	UCHAR        ucDbdcBandIdx,
	UCHAR	       ucGroup,
	BOOLEAN      fgRdFromE2p,
	BOOLEAN      fgDisComp)
{
	struct cmd_msg *msg;
	EXT_CMD_ITXBf_PHASE_COMP_CTRL_T aucIBfPhaseComp;
	INT32 ret = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Enable iBF phase compensation : fgRdFromE2p = %d, ucBW = %d, ucDbdcBandIdx = %d\n",
			  fgRdFromE2p, ucBW, ucDbdcBandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ITXBf_PHASE_COMP_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&aucIBfPhaseComp, sizeof(EXT_CMD_ITXBf_PHASE_COMP_CTRL_T));
	aucIBfPhaseComp.ucCmdCategoryID = BF_IBF_PHASE_COMP;
	aucIBfPhaseComp.ucBW            = ucBW;
	aucIBfPhaseComp.ucBand          = ucBand;
	aucIBfPhaseComp.ucDbdcBandIdx   = ucDbdcBandIdx;
	aucIBfPhaseComp.fgRdFromE2p     = fgRdFromE2p;
	aucIBfPhaseComp.fgDisComp       = fgDisComp;

	if (ops->iBFPhaseComp)
		ops->iBFPhaseComp(pAd, ucGroup, aucIBfPhaseComp.aucBuf);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 1000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucIBfPhaseComp, sizeof(EXT_CMD_ITXBf_PHASE_COMP_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 cmd_txbf_config(
	RTMP_ADAPTER *pAd,
	UINT8 config_type,
	UINT8 config_para[])
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_BF_CONFIG_T txbf_config;
	UINT8 i;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"config_type=%d\n", config_type);

	for (i = 0 ; i < 6 ; i++) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"config_para[%d]=%d\n", i, config_para[i]);
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	txbf_config.cmd_category_id = BF_CONFIG;
	txbf_config.config_type = config_type;
	os_move_mem(txbf_config.config_para, config_para, 6);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&txbf_config, sizeof(EXT_CMD_BF_CONFIG_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfTxApplyCtrl(
	RTMP_ADAPTER *pAd,
	UINT16        u2WlanId,
	BOOLEAN       fgETxBf,
	BOOLEAN       fgITxBf,
	BOOLEAN       fgMuTxBf,
	BOOLEAN       fgPhaseCali)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_TXBf_TX_APPLY_CTRL_T aucTxBfTxApplyCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TxBf Tx Apply ucWLanId = %d, fgETxBf = %d, fgITxBf = %d, fgMuTxBf = %d\n",
			  u2WlanId, fgETxBf, fgITxBf, fgMuTxBf);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBf_TX_APPLY_CTRL_T));
	os_zero_mem(&aucTxBfTxApplyCtrl, sizeof(EXT_CMD_TXBf_TX_APPLY_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	aucTxBfTxApplyCtrl.ucCmdCategoryID = BF_DATA_PACKET_APPLY;
	WCID_SET_H_L(aucTxBfTxApplyCtrl.ucWlanIdxHnVer, aucTxBfTxApplyCtrl.ucWlanIdxL, u2WlanId);
	aucTxBfTxApplyCtrl.fgETxBf         = fgETxBf;
	aucTxBfTxApplyCtrl.fgITxBf         = fgITxBf;
	aucTxBfTxApplyCtrl.fgMuTxBf        = fgMuTxBf;
	aucTxBfTxApplyCtrl.fgPhaseCali     = fgPhaseCali;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucTxBfTxApplyCtrl, sizeof(EXT_CMD_TXBf_TX_APPLY_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfSoundingPeriodicTriggerCtrl(
	RTMP_ADAPTER *pAd,
	UCHAR			SndgEn,
	UINT32        u4SNDPeriod,
	UCHAR         ucSu_Mu,
	UCHAR         ucMuNum,
	PUCHAR        pwlanidx)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_ETXBf_SND_PERIODIC_TRIGGER_CTRL_T ETxBfSndPeriodicTriggerCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Sounding trigger enable = %d\n", SndgEn);

	if (ucSu_Mu < SOUNDING_MAX) {
		msg = MtAndesAllocCmdMsg(pAd, sizeof(ETxBfSndPeriodicTriggerCtrl));
		os_zero_mem(&ETxBfSndPeriodicTriggerCtrl, sizeof(ETxBfSndPeriodicTriggerCtrl));
	} else {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (SndgEn) {
		if (ucSu_Mu < SOUNDING_MAX) {
			ETxBfSndPeriodicTriggerCtrl.ucCmdCategoryID       = BF_SOUNDING_ON;
			ETxBfSndPeriodicTriggerCtrl.ucSuMuSndMode         = ucSu_Mu;
			ETxBfSndPeriodicTriggerCtrl.u4SoundingInterval    = cpu2le32(u4SNDPeriod);
			if (pwlanidx) {
				ETxBfSndPeriodicTriggerCtrl.ucWlanIdx[0]          = pwlanidx[0];
				ETxBfSndPeriodicTriggerCtrl.ucWlanIdx[1]          = pwlanidx[1];
				ETxBfSndPeriodicTriggerCtrl.ucWlanIdx[2]          = pwlanidx[2];
				ETxBfSndPeriodicTriggerCtrl.ucWlanIdx[3]          = pwlanidx[3];
			}
			ETxBfSndPeriodicTriggerCtrl.ucStaNum              = ucMuNum;
		}
	} else {
		ETxBfSndPeriodicTriggerCtrl.ucCmdCategoryID   = BF_SOUNDING_OFF;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&ETxBfSndPeriodicTriggerCtrl,
						sizeof(ETxBfSndPeriodicTriggerCtrl));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdPfmuMemAlloc(
	RTMP_ADAPTER *pAd,
	UCHAR        ucSu_Mu,
	UINT16        u2WlanId)
{
	struct cmd_msg *msg;
	UCHAR aucCmdBuf[4];
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: u2WlanId = %d\n", __func__, u2WlanId);
	msg = MtAndesAllocCmdMsg(pAd, 4);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	aucCmdBuf[0] = BF_PFMU_MEM_ALLOCATE;
	aucCmdBuf[1] = ucSu_Mu;
	WCID_SET_H_L(aucCmdBuf[3], aucCmdBuf[2], u2WlanId);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucCmdBuf[0], 4);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdPfmuMemRelease(
	RTMP_ADAPTER *pAd,
	UINT16       u2WlanId)
{
	struct cmd_msg *msg;
	UCHAR aucCmdBuf[4];
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u2WlanId = %d\n", u2WlanId);
	msg = MtAndesAllocCmdMsg(pAd, 2);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	aucCmdBuf[0] = BF_PFMU_MEM_RELEASE;
	aucCmdBuf[1] = 0; /* Redundancy */
	WCID_SET_H_L(aucCmdBuf[3], aucCmdBuf[2], u2WlanId);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucCmdBuf[0], 4);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdPfmuMemAllocMapRead(
	RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	UCHAR aucCmdBuf[4];
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, 1);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	aucCmdBuf[0] = BF_PFMU_MEM_ALLOC_MAP_READ;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&aucCmdBuf[0], 4);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfileTagRead(
	RTMP_ADAPTER *pAd,
	UCHAR PfmuIdx,
	BOOLEAN fgBFer)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_TAG_R_T ETxBfPfmuProfileTag;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(ETxBfPfmuProfileTag));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileTag, sizeof(ETxBfPfmuProfileTag));
	ETxBfPfmuProfileTag.ucPfmuProfileFormatId = BF_PFMU_TAG_READ;
	ETxBfPfmuProfileTag.ucPfmuIdx = PfmuIdx;
	ETxBfPfmuProfileTag.fgBFer = fgBFer;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */
	ETxBfPfmuProfileTag.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, BFer: %x, Band:%d\n",
			 ETxBfPfmuProfileTag.ucPfmuIdx, ETxBfPfmuProfileTag.fgBFer, ETxBfPfmuProfileTag.ucBandIdx);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileTag,
						sizeof(ETxBfPfmuProfileTag));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}


INT32 CmdETxBfPfmuProfileTagWrite(
	RTMP_ADAPTER *pAd,
	PUCHAR       prPfmuTag1,
	PUCHAR       prPfmuTag2,
	UINT_8       tag1_len,
	UINT_8       tag2_len,
	UCHAR        PfmuIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_TAG_W_T ETxBfPfmuProfileTag;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
		if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(ETxBfPfmuProfileTag));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileTag, sizeof(ETxBfPfmuProfileTag));
	ETxBfPfmuProfileTag.ucPfmuProfileFormatId = BF_PFMU_TAG_WRITE;
	ETxBfPfmuProfileTag.ucPfmuIdx = PfmuIdx;
	ETxBfPfmuProfileTag.fgBFer = TRUE;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfileTag.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, Band:%d\n",
			 ETxBfPfmuProfileTag.ucPfmuIdx, ETxBfPfmuProfileTag.ucBandIdx);

	os_move_mem(ETxBfPfmuProfileTag.ucBuf,
				prPfmuTag1,
				tag1_len);
	os_move_mem(ETxBfPfmuProfileTag.ucBuf + tag1_len,
				prPfmuTag2,
				tag2_len);
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(ETxBfPfmuProfileTag.ucBuf, tag1_len + tag2_len);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileTag,
						sizeof(ETxBfPfmuProfileTag));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfileDataRead(
	RTMP_ADAPTER *pAd,
	UCHAR        PfmuIdx,
	BOOLEAN      fgBFer,
	USHORT       subCarrIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_DATA_R_T ETxBfPfmuProfileData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(ETxBfPfmuProfileData));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileData, sizeof(ETxBfPfmuProfileData));
	ETxBfPfmuProfileData.ucPfmuProfileFormatId = BF_PROFILE_READ;
	ETxBfPfmuProfileData.ucPfmuIdx = PfmuIdx;
	ETxBfPfmuProfileData.fgBFer = fgBFer;
	ETxBfPfmuProfileData.u2SubCarrIdx = cpu2le16(subCarrIdx);

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfileData.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, BFer:%x, Subcarrier:%d, Band:%d\n",
			 ETxBfPfmuProfileData.ucPfmuIdx, ETxBfPfmuProfileData.fgBFer,
			 ETxBfPfmuProfileData.u2SubCarrIdx, ETxBfPfmuProfileData.ucBandIdx);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileData,
						sizeof(ETxBfPfmuProfileData));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfileDataWrite(
	RTMP_ADAPTER *pAd,
	UCHAR  PfmuIdx,
	USHORT SubCarrIdx,
	PUCHAR pProfileData)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T ETxBfPfmuProfileData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	if (!pProfileData) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileData, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T));
	ETxBfPfmuProfileData.ucPfmuProfileFormatId = BF_PROFILE_WRITE;
	ETxBfPfmuProfileData.ucPfmuIdx = PfmuIdx;
	ETxBfPfmuProfileData.u2SubCarr = cpu2le16(SubCarrIdx);

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfileData.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, Subcarrier:%d, Band:%d\n",
			 ETxBfPfmuProfileData.ucPfmuIdx, ETxBfPfmuProfileData.u2SubCarr, ETxBfPfmuProfileData.ucBandIdx);

	os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, 16);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Buf[0~3]= %x:%x:%x:%x\n",
			 ETxBfPfmuProfileData.ucBuf[0], ETxBfPfmuProfileData.ucBuf[1],
			 ETxBfPfmuProfileData.ucBuf[2], ETxBfPfmuProfileData.ucBuf[3]);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Buf[4~7]= %x:%x:%x:%x\n",
			 ETxBfPfmuProfileData.ucBuf[4], ETxBfPfmuProfileData.ucBuf[5],
			 ETxBfPfmuProfileData.ucBuf[6], ETxBfPfmuProfileData.ucBuf[7]);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Buf[8~11]= %x:%x:%x:%x\n",
			 ETxBfPfmuProfileData.ucBuf[8], ETxBfPfmuProfileData.ucBuf[9],
			 ETxBfPfmuProfileData.ucBuf[10], ETxBfPfmuProfileData.ucBuf[11]);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Buf[12~15]= %x:%x:%x:%x\n",
			 ETxBfPfmuProfileData.ucBuf[12], ETxBfPfmuProfileData.ucBuf[13],
			 ETxBfPfmuProfileData.ucBuf[14], ETxBfPfmuProfileData.ucBuf[15]);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileData,
						sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuFullDimDataWrite(
	RTMP_ADAPTER *pAd,
	UCHAR        PfmuIdx,
	USHORT       SubCarrIdx,
	BOOLEAN      bfer,
	PUCHAR       pProfileData,
	UCHAR        DataLength)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_FULL_DIM_DATA_W_T ETxBfPfmuData;
	INT32 ret = 0, CmdLength = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	if (!pProfileData) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* 8 bytes for EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T except ucBuf */
	CmdLength = DataLength + 8;

	msg = MtAndesAllocCmdMsg(pAd, CmdLength);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuData, CmdLength);
	ETxBfPfmuData.u1PfmuProfileFormatId = BF_PFMU_DATA_WRITE;
	ETxBfPfmuData.u1PfmuIdx = PfmuIdx;
	ETxBfPfmuData.u2SubCarr = cpu2le16(SubCarrIdx);

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif /*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuData.u1BandIdx = BandIdx;
	ETxBfPfmuData.fgBfer = bfer;

	os_move_mem(ETxBfPfmuData.aucBuf, pProfileData, DataLength);
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(ETxBfPfmuData.aucBuf, DataLength);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuData, CmdLength);
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfileDataWrite20MAll(
	RTMP_ADAPTER     *pAd,
	UCHAR            PfmuIdx,
	PUCHAR           pProfileData)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T ETxBfPfmuProfileData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif /*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileData, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T));
	ETxBfPfmuProfileData.ucPfmuProfileFormatId = BF_PROFILE_WRITE_20M_ALL;
	ETxBfPfmuProfileData.ucPfmuIdx             = PfmuIdx;
	os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, 512);

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif /*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfileData.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, Band:%d\n",
			 ETxBfPfmuProfileData.ucPfmuIdx, ETxBfPfmuProfileData.ucBandIdx);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileData,
						sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfilePnRead(
	RTMP_ADAPTER *pAd,
	UCHAR        PfmuIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T ETxBfPfmuProfilePn;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfilePn, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T));
	ETxBfPfmuProfilePn.ucPfmuProfileFormatId = BF_PN_READ;
	ETxBfPfmuProfilePn.ucPfmuIdx             = PfmuIdx;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfilePn.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, Band:%d\n",
			 ETxBfPfmuProfilePn.ucPfmuIdx, ETxBfPfmuProfilePn.ucBandIdx);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfilePn,
						sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPfmuProfilePnWrite(
	RTMP_ADAPTER *pAd,
	UCHAR        PfmuIdx,
	UCHAR        ucBw,
	PUCHAR       pProfileData)
{
	struct cmd_msg *msg;
	EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T ETxBfPfmuProfileData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfPfmuProfileData, sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T));
	ETxBfPfmuProfileData.ucPfmuProfileFormatId = BF_PN_WRITE;
	ETxBfPfmuProfileData.ucPfmuIdx             = PfmuIdx;
	ETxBfPfmuProfileData.ucBW                  = ucBw;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfPfmuProfileData.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "PFMU ID = %d, Band:%d\n",
			 ETxBfPfmuProfileData.ucPfmuIdx, ETxBfPfmuProfileData.ucBandIdx);

	switch (ucBw) {
	case P_DBW20M:
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange(pProfileData, sizeof(PFMU_PN_DBW20M));
#endif
		os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, sizeof(PFMU_PN_DBW20M));
		break;

	case P_DBW40M:
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange(pProfileData, sizeof(PFMU_PN_DBW40M));
#endif
		os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, sizeof(PFMU_PN_DBW40M));
		break;

	case P_DBW80M:
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange(pProfileData, sizeof(PFMU_PN_DBW80M));
#endif
		os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, sizeof(PFMU_PN_DBW80M));
		break;

	case P_DBW160M:
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange(pProfileData, sizeof(PFMU_PN_DBW80_80M));
#endif
		os_move_mem(ETxBfPfmuProfileData.ucBuf, pProfileData, sizeof(PFMU_PN_DBW80_80M));
		break;

	default:
		return 1;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfPfmuProfileData,
						sizeof(EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfQdRead(
	RTMP_ADAPTER *pAd,
	INT8         subCarrIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_TXBf_QD_R_T ETxBfQdData;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBf_QD_R_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfQdData, sizeof(EXT_CMD_TXBf_QD_R_T));
	ETxBfQdData.ucCmdCategoryID = BF_GET_QD;
	ETxBfQdData.cSubCarr = subCarrIdx;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	ETxBfQdData.ucBandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "subCarrIdx = %d,  Band:%d\n", ETxBfQdData.cSubCarr, ETxBfQdData.ucBandIdx);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ETxBfQdData, sizeof(EXT_CMD_TXBf_QD_R_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfFbRptDbgInfo(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif /*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 if_idx = pObj->ioctl_if;
	struct wifi_dev *pwdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UINT8 BandIdx = HcGetBandByWdev(pwdev);
	P_EXT_CMD_TXBF_FBRPT_DBG_INFO_T prETxBfFbRptData = (P_EXT_CMD_TXBF_FBRPT_DBG_INFO_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_FBRPT_DBG_INFO_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prETxBfFbRptData->u1CmdCategoryID = BF_FBRPT_DBG_INFO_READ;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
#ifdef CONFIG_WLAN_SERVICE
	{
		BandIdx = serv_test->ctrl_band_idx;
	}
#else
	{
		BandIdx = ATECtrl->control_band_idx;
	}
#endif /*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */

	prETxBfFbRptData->u1BandIdx = BandIdx;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Action:%u, BandIdx:%u, PollPFMUIntrStatTimeOut:%u, FbRptDeQInterval:%u, WlanIdx:%u, PFMUUpdateEn:%u\n"
			, prETxBfFbRptData->u1Action, prETxBfFbRptData->u1BandIdx
			, prETxBfFbRptData->u1PollPFMUIntrStatTimeOut
			, prETxBfFbRptData->u1FbRptDeQInterval
			, prETxBfFbRptData->u2WlanIdx
			, prETxBfFbRptData->u1PFMUUpdateEn);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prETxBfFbRptData, sizeof(EXT_CMD_TXBF_FBRPT_DBG_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfTxSndInfo(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_EXT_CMD_TXBF_SND_CMD_T prTxSndCmd = (P_EXT_CMD_TXBF_SND_CMD_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_SND_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prTxSndCmd->ucCmdCategoryID = BF_CMD_TXSND_INFO;
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucAction=0x%02X, ucReadClr=0x%02X, ucVhtOpt=0x%02X, ucHeOpt=0x%02X,\n",
			prTxSndCmd->ucAction,
			prTxSndCmd->ucReadClr,
			prTxSndCmd->ucVhtOpt,
			prTxSndCmd->ucHeOpt);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucGloOpt=0x%02X, u2WlanIdx=0x%04X, ucSndIntv=0x%02X, ucSndStop:0x%02X\n",
			prTxSndCmd->ucGloOpt,
			prTxSndCmd->u2WlanIdx,
			prTxSndCmd->ucSndIntv,
			prTxSndCmd->ucSndStop);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucMaxSndStas=0x%02X, ucTxTime=0x%02X, ucMcs=0x%02X, fgLDPC:0x%02X\n",
			prTxSndCmd->ucMaxSndStas,
			prTxSndCmd->ucTxTime,
			prTxSndCmd->ucMcs,
			prTxSndCmd->fgLDPC);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucInf=0x%02X\n", prTxSndCmd->ucInf);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	prTxSndCmd->u2WlanIdx = cpu2le16(prTxSndCmd->u2WlanIdx);
#endif
	MtAndesAppendCmdMsg(msg, (char *)prTxSndCmd, sizeof(EXT_CMD_TXBF_SND_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfPlyInfo(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_EXT_CMD_TXBF_PLY_CMD_T prTxSndCmd = (P_EXT_CMD_TXBF_PLY_CMD_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_PLY_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prTxSndCmd->ucCmdCategoryID = BF_CMD_PLY_INFO;
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucAction=0x%02X, ucGloOpt=0x%02X, ucGrpIBfOpt=0x%02X, ucGrpEBfOpt=0x%02X,\n",
			prTxSndCmd->ucAction,
			prTxSndCmd->ucGloOpt,
			prTxSndCmd->ucGrpIBfOpt,
			prTxSndCmd->ucGrpEBfOpt);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u2WlanIdx=0x%04X, ucNss=0x%02X, ucSSPly=0x%02X\n",
			prTxSndCmd->u2WlanIdx,
			prTxSndCmd->ucNss,
			prTxSndCmd->ucSSPly);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prTxSndCmd, sizeof(EXT_CMD_TXBF_PLY_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfTxCmd(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_EXT_CMD_TXBF_TXCMD_CMD_T prTxBfTxCmdCmd = (P_EXT_CMD_TXBF_TXCMD_CMD_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_TXCMD_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prTxBfTxCmdCmd->ucCmdCategoryID = BF_CMD_TXCMD;
	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ucAction=0x%02X, fgTxCmdBfManual=0x%02X, ucTxCmdBfBit=0x%02X\n",
			prTxBfTxCmdCmd->ucAction,
			prTxBfTxCmdCmd->fgTxCmdBfManual,
			prTxBfTxCmdCmd->ucTxCmdBfBit);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prTxBfTxCmdCmd, sizeof(EXT_CMD_TXBF_TXCMD_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfSndCnt(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_EXT_CMD_TXBF_SND_CNT_CMD_T prSndCntCmd = (P_EXT_CMD_TXBF_SND_CNT_CMD_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_SND_CNT_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prSndCntCmd->ucCmdCategoryID = BF_CMD_SND_CNT;
	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1Action=%u, u2SndCntLmtMan=%u\n",
			prSndCntCmd->u1Action,
			prSndCntCmd->u2SndCntLmtMan);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prSndCntCmd, sizeof(EXT_CMD_TXBF_SND_CNT_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfCfgBfPhy(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_EXT_CMD_TXBF_CFG_BF_PHY_T prTxBfCfgBfPhy = (P_EXT_CMD_TXBF_CFG_BF_PHY_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TXBF_CFG_BF_PHY_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prTxBfCfgBfPhy->ucCmdCategoryID = BF_CMD_CFG_PHY;
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"ucAction=0x%02X, ucBandIdx=0x%02X, ucSmthIntlBypass=0x%02X\n",
			prTxBfCfgBfPhy->ucAction,
			prTxBfCfgBfPhy->ucBandIdx,
			prTxBfCfgBfPhy->ucSmthIntlBypass);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prTxBfCfgBfPhy, sizeof(EXT_CMD_TXBF_CFG_BF_PHY_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdHeRaMuMetricInfo(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_HERA_MU_METRIC_CMD_T prMuMetCmd = (P_HERA_MU_METRIC_CMD_T)pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(HERA_MU_METRIC_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	prMuMetCmd->u1CmdCategoryID = BF_CMD_MU_METRIC;
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u1ReadClr=0x%02X, u1Band=0x%02X, u1NUser=0x%02X, u1DBW=0x%02X,\n",
			prMuMetCmd->u1ReadClr,
			prMuMetCmd->u1Band,
			prMuMetCmd->u1NUser,
			prMuMetCmd->u1DBW);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u1NTxer=0x%02X, u1PFD=0x%02X, u1RuSize=0x%02X, u1RuIdx=0x%02X\n",
			prMuMetCmd->u1NTxer,
			prMuMetCmd->u1PFD,
			prMuMetCmd->u1RuSize,
			prMuMetCmd->u1RuIdx);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u1SpeIdx=0x%02X, u1SpeedUp=0x%02X, u1LDPC=0x%02X, u1PollingTime=0x%02X\n",
			prMuMetCmd->u1SpeIdx,
			prMuMetCmd->u1SpeedUp,
			prMuMetCmd->u1LDPC,
			prMuMetCmd->u1PollingTime);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u1NStsUser0=0x%02X, u1NStsUser1=0x%02X, u1NStsUser2=0x%02X, u1NStsUser3=0x%02X,\n",
			prMuMetCmd->u1NStsUser[0],
			prMuMetCmd->u1NStsUser[1],
			prMuMetCmd->u1NStsUser[2],
			prMuMetCmd->u1NStsUser[3]);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"u2PfidUser0=0x%04X, u2PfidUser1=0x%04X, u2PfidUser2=0x%04X, u2PfidUser3=0x%04X,\n",
			prMuMetCmd->u2PfidUser[0],
			prMuMetCmd->u2PfidUser[1],
			prMuMetCmd->u2PfidUser[2],
			prMuMetCmd->u2PfidUser[3]);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prMuMetCmd, sizeof(HERA_MU_METRIC_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdETxBfStaRecRead(
	RTMP_ADAPTER *pAd,
	UINT16        u2WlanId)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	CHAR ucCmd[8];
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "WLAN ID = %d\n", u2WlanId);
	msg = MtAndesAllocCmdMsg(pAd, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	ucCmd[0] = BF_STA_REC_READ;
	WCID_SET_H_L(ucCmd[2], ucCmd[1], u2WlanId);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, &ucCmd[0], 8);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfAwareCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN       fgBfAwareCtrl)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_BF_AWARE_CTRL_T rTxBfAwareCtrl = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgBfAwareCtrl = %d\n", fgBfAwareCtrl);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_BF_AWARE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rTxBfAwareCtrl.ucCmdCategoryID = BF_AWARE_CTRL;
	rTxBfAwareCtrl.fgBfAwareCtrl   = fgBfAwareCtrl;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rTxBfAwareCtrl, sizeof(EXT_CMD_BF_AWARE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 cmd_txbf_en_dynsnd_intr(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN is_intr_en)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct dynsnd_en_intr_info cmd_info;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "is_intr_en = %d\n", is_intr_en);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(cmd_info));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	cmd_info.category_id = BF_DYNSND_EN_INTR;
	cmd_info.is_intr_en   = is_intr_en;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef CFG_SUPPORT_MU_MIMO
INT32 cmd_txbf_cfg_dynsnd_dmcsth(
	struct _RTMP_ADAPTER *pAd,
	UINT8 mcs_index,
	UINT8 mcsth)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct dynsnd_cfg_dmcsth_info cmd_info = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "mcs_index = %d, mcsth = %d\n", mcs_index, mcsth);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(cmd_info));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	cmd_info.category_id = BF_DYNSND_CFG_DMCS_TH;
	cmd_info.mcs_index = mcs_index;
	cmd_info.mcs_th = mcsth;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_info, sizeof(cmd_info));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 cmd_txbf_en_dynsnd_pfid_intr(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN mu_intr_en,
	UINT8 pfid)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct dynsnd_en_mu_intr_info cmd_info = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "mu_intr_en = %d, pfid = %d\n", mu_intr_en, pfid);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(cmd_info));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	cmd_info.category_id = BF_DYNSND_EN_PFID_INTR;
	cmd_info.mu_intr_en = mu_intr_en;
	cmd_info.pfid = pfid;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_info, sizeof(cmd_info));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif

INT32 CmdTxBfHwEnableStatusUpdate(
	RTMP_ADAPTER *pAd,
	BOOLEAN       fgEBf,
	BOOLEAN       fgIBf)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T rTxBfHwEnStatusUpdate = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgEBfHwEnable = %d, fgIBfHwEnable = %d\n", fgEBf, fgIBf);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rTxBfHwEnStatusUpdate.ucCmdCategoryID = BF_HW_ENABLE_STATUS_UPDATE;
	rTxBfHwEnStatusUpdate.fgEBfHwEnStatus = fgEBf;
	rTxBfHwEnStatusUpdate.fgIBfHwEnStatus = fgIBf;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rTxBfHwEnStatusUpdate, sizeof(EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[])
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_BF_MOD_EN_CTRL_T rTxBfModEnCtrl = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u1BfNum = %d, u1BfBitmap = %d, u1BfSelBand[0] = %d, u1BfSelBand[1] = %d\n",
		u1BfNum, u1BfBitmap, u1BfSelBand[0], u1BfSelBand[1]);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rTxBfModEnCtrl.u1CmdCategoryID = BF_MOD_EN_CTRL;
	rTxBfModEnCtrl.u1BfNum = u1BfNum;
	rTxBfModEnCtrl.u1BfBitmap = u1BfBitmap;
	os_move_mem(rTxBfModEnCtrl.au1BFSel, u1BfSelBand, 8);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rTxBfModEnCtrl, sizeof(EXT_CMD_BF_MOD_EN_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxBfeeHwCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN       fgBfeeHwEn)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_BFEE_HW_CTRL_T rTxBfeeHwCtrl = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgTxBfeeHwEnable = %d\n", fgBfeeHwEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_BFEE_HW_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rTxBfeeHwCtrl.ucCmdCategoryID = BF_BFEE_HW_CTRL;
	rTxBfeeHwCtrl.fgBfeeHwCtrl    = fgBfeeHwEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rTxBfeeHwCtrl, sizeof(EXT_CMD_BFEE_HW_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 CmdETxBfPseudoTagWrite(
	struct _RTMP_ADAPTER *pAd,
	EXT_CMD_ETXBF_PFMU_SW_TAG_T rEBfPfmuSwTag)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32  ret = 0;
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ETXBF_PFMU_SW_TAG_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BF_ACTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rEBfPfmuSwTag, sizeof(EXT_CMD_ETXBF_PFMU_SW_TAG_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}

#endif /* MT_MAC && TXBF_SUPPORT */

INT32 CmdMecCtrl(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	P_CMD_MEC_CTRL_CMD_T prMecCtrlCmd = (P_CMD_MEC_CTRL_CMD_T)pucData;
	UINT8 u1Idx;
	UINT8 *pu1Ptr = (UINT8 *) pucData;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MEC_CTRL_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "u2Action=0x%02X\n",
			prMecCtrlCmd->u2Action);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "u2WlanIdx=0x%02X, u1AmsduAlgoEn=0x%02X\n",
			prMecCtrlCmd->mecCmdPara.mec_algo_en_sta_t.u2WlanIdx,
			prMecCtrlCmd->mecCmdPara.mec_algo_en_sta_t.u1AmsduAlgoEn);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"u2WlanIdx=0x%02X, u1AmsduEn=0x%02X, u1AmsduNum=0x%02X, u2AmsduLen=0x%02X\n",
			prMecCtrlCmd->mecCmdPara.mec_amsdu_para_sta_t.u2WlanIdx,
			prMecCtrlCmd->mecCmdPara.mec_amsdu_para_sta_t.u1AmsduEn,
			prMecCtrlCmd->mecCmdPara.mec_amsdu_para_sta_t.u1AmsduNum,
			prMecCtrlCmd->mecCmdPara.mec_amsdu_para_sta_t.u2AmsduLen);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "u1BaNum=0x%02X, u1AmsduNum=0x%02X, u2AmsduRateThr=0x%02X\n",
			prMecCtrlCmd->mecCmdPara.mec_amsdu_algo_thr.u1BaNum,
			prMecCtrlCmd->mecCmdPara.mec_amsdu_algo_thr.u1AmsduNum,
			prMecCtrlCmd->mecCmdPara.mec_amsdu_algo_thr.u2AmsduRateThr);

	for (u1Idx = 0 ; u1Idx < sizeof(CMD_MEC_CTRL_CMD_T) ; u1Idx++) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "[%u]: %u\n", u1Idx, pu1Ptr[u1Idx]);
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MEC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 500);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)prMecCtrlCmd, sizeof(CMD_MEC_CTRL_CMD_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x21
 *****************************************/
static VOID CmdEfuseBufferModeRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult.u4Status = 0x%x\n",
			  __func__, EventExtCmdResult->u4Status);
}

static VOID CmdHwcfgRDRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	int i;
	struct _EXT_EVENT_HWCFG_READ_T *EventExtCmdResult =
		(struct _EXT_EVENT_HWCFG_READ_T *)Data;
	CHAR *pEepromData =
		(CHAR *)msg->attr.rsp.wb_buf_in_calbk;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Len = 0x%x\n",
			  Len);

	EventExtCmdResult->u2Offset = le2cpu16(EventExtCmdResult->u2Offset);
	/*MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ("EventExtCmdResult->u2Offset = 0x%x, 0x%x\n", EventExtCmdResult->u2Offset, &EventExtCmdResult->u2Offset));
	EventExtCmdResult->u2Count = le2cpu16(EventExtCmdResult->u2Count);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ("EventExtCmdResult->ucCount = 0x%x, 0x%x\n", EventExtCmdResult->u2Count, &EventExtCmdResult->u2Count));
*/
	for (i = 0; i < EventExtCmdResult->u2Count; i++) {
		if ((i % 32) == 0) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "\n\r");
		}
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%02x ", EventExtCmdResult->BinContent[i]);
		*(pEepromData+i) = EventExtCmdResult->BinContent[i];
	}
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "\n\r");

}

#ifdef EEPROM_RETRIEVE_SUPPORT
static VOID CmdEfuseBufferRDRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
    int i;
	struct _EXT_EVENT_EFUSE_BUFFER_MODE_READ_T *EventExtCmdResult =
		(struct _EXT_EVENT_EFUSE_BUFFER_MODE_READ_T *)Data;
	CHAR *pEepromData =
		(CHAR *)msg->attr.rsp.wb_buf_in_calbk;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: Len = 0x%x\n",
			  __func__, Len);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult->ucSourceMode = 0x%x, 0x%x\n",
			  __func__, EventExtCmdResult->ucSourceMode, &EventExtCmdResult->ucSourceMode);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult->ucContentFormat = 0x%x, 0x%x\n",
			  __func__, EventExtCmdResult->ucContentFormat, &EventExtCmdResult->ucContentFormat);
	EventExtCmdResult->u2Offset = le2cpu16(EventExtCmdResult->u2Offset);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult->u2Offset = 0x%x, 0x%x\n",
			  __func__, EventExtCmdResult->u2Offset, &EventExtCmdResult->u2Offset);
	EventExtCmdResult->u2Count = le2cpu16(EventExtCmdResult->u2Count);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: EventExtCmdResult->ucCount = 0x%x, 0x%x\n",
			  __func__, EventExtCmdResult->u2Count, &EventExtCmdResult->u2Count);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "EEProm Content: 0x%x\n\r",
			 &EventExtCmdResult->BinContent[i]);
	for (i = 0; i < EventExtCmdResult->u2Count; i++) {
		if ((i % 32) == 0) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "\n\r");
		}
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%02x ", EventExtCmdResult->BinContent[i]);
		*(pEepromData+i) = EventExtCmdResult->BinContent[i];
    }
    MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "\n\r");
}
#endif /* EEPROM_RETRIEVE_SUPPORT */

VOID MtCmdEfusBufferModeSet(RTMP_ADAPTER *pAd, UINT8 EepromType)
{
	struct cmd_msg *msg = NULL;
	union _EXT_CMD_EFUSE_BUFFER_MODE_T *CmdEfuseBufferMode = NULL;
	UINT32 cmd_size;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT8 total_seq = 0, cmd_seq = 0;

	cmd_size = sizeof(union _EXT_CMD_EFUSE_BUFFER_MODE_T);
	cmd_size += sizeof(UCHAR) * EEPROM_BUFFER_MODE_DATA_LIMIT;
	os_alloc_mem(pAd, (UCHAR **)&CmdEfuseBufferMode, cmd_size);

	if (!CmdEfuseBufferMode) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (EepromType == EEPROM_EFUSE)
		total_seq = 1;
	else if (EepromType == EEPROM_FLASH) {
		total_seq = cap->EFUSE_BUFFER_CONTENT_SIZE / EEPROM_BUFFER_MODE_DATA_LIMIT;
		if ((cap->EFUSE_BUFFER_CONTENT_SIZE % EEPROM_BUFFER_MODE_DATA_LIMIT) != 0)
			total_seq++;
	}

	for (cmd_seq = 0; cmd_seq < total_seq; cmd_seq++) {
		msg = MtAndesAllocCmdMsg(pAd, cmd_size);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_EFUSE_BUFFER_MODE);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, CmdEfuseBufferModeRsp);
		MtAndesInitCmdMsg(msg, attr);
		os_zero_mem(CmdEfuseBufferMode, cmd_size);

		if (ops->ee_gen_cmd)
			ops->ee_gen_cmd(pAd, CmdEfuseBufferMode, cmd_seq, total_seq);
		else {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "invalid apply ops, dismissed!\n");
			goto error;
		}

		MtAndesAppendCmdMsg(msg, (char *)CmdEfuseBufferMode, cmd_size);
		ret = chip_cmd_tx(pAd, msg);
	}
	goto done;

error:
	if (msg)
		MtAndesFreeCmdMsg(msg);

done:
	if (CmdEfuseBufferMode)
		os_free_mem(CmdEfuseBufferMode);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s:(ret = %d)\n", __func__, ret);
}

#ifdef EEPROM_RETRIEVE_SUPPORT
VOID MtCmdEfusBufferModeGet(RTMP_ADAPTER *pAd, UINT8 EepromType,
		UINT16 dump_offset, UINT16 dump_size, UINT8 *epprom_content)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_EFUSE_BUFFER_MODE_READ_T *CmdEfuseBufferModeRead = NULL;
	UINT32 cmd_size;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	cmd_size = sizeof(EXT_CMD_EFUSE_BUFFER_MODE_READ_T);
	os_alloc_mem(pAd, (UCHAR **)&CmdEfuseBufferModeRead, cmd_size);

	if (!CmdEfuseBufferModeRead) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, cmd_size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EFUSE_BUFFER_RD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_EFUSE_BUFFER_MODE_READ_T) + dump_size);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);

	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, epprom_content);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdEfuseBufferRDRsp);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(CmdEfuseBufferModeRead, cmd_size);

	switch (EepromType) {
	case EEPROM_EFUSE:
			CmdEfuseBufferModeRead->ucSourceMode = EEPROM_MODE_EFUSE;
			CmdEfuseBufferModeRead->u2Offset = dump_offset;
			CmdEfuseBufferModeRead->u2Count = dump_size;
			break;
	case EEPROM_FLASH:
			CmdEfuseBufferModeRead->ucSourceMode = EEPROM_MODE_BUFFER;
			CmdEfuseBufferModeRead->u2Offset = dump_offset;
			CmdEfuseBufferModeRead->u2Count = dump_size;
			break;
	default:
			ret = NDIS_STATUS_FAILURE;
			goto error;
	}

#ifdef RT_BIG_ENDIAN
	CmdEfuseBufferModeRead->u2Count = cpu2le16(CmdEfuseBufferModeRead->u2Count);
	CmdEfuseBufferModeRead->u2Offset = cpu2le16(CmdEfuseBufferModeRead->u2Offset);
#endif
	MtAndesAppendCmdMsg(msg, (char *)CmdEfuseBufferModeRead, cmd_size);
	ret = chip_cmd_tx(pAd, msg);
	goto done;
error:

	if (msg)
		MtAndesFreeCmdMsg(msg);

done:

	if (CmdEfuseBufferModeRead)
		os_free_mem(CmdEfuseBufferModeRead);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
}
#endif /* EEPROM_RETRIEVE_SUPPORT */

VOID MtCmdHwcfgGet(RTMP_ADAPTER *pAd, UINT16 dump_offset, UINT16 dump_size, UINT8 *epprom_content)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_HWCFG_READ_T *CmdEfuseBufferModeRead = NULL;
	UINT32 cmd_size;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	cmd_size = sizeof(EXT_CMD_HWCFG_READ_T);
	os_alloc_mem(pAd, (UCHAR **)&CmdEfuseBufferModeRead, cmd_size);

	if (!CmdEfuseBufferModeRead) {
		ret = NDIS_STATUS_RESOURCES;
		goto done;
	}

	msg = MtAndesAllocCmdMsg(pAd, cmd_size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto done;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HWCFG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_HWCFG_READ_T) + dump_size);

	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, epprom_content);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdHwcfgRDRsp);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(CmdEfuseBufferModeRead, cmd_size);

	CmdEfuseBufferModeRead->u2Offset = dump_offset;
	CmdEfuseBufferModeRead->u2Count = dump_size;

#ifdef RT_BIG_ENDIAN
	CmdEfuseBufferModeRead->u2Count = cpu2le16(CmdEfuseBufferModeRead->u2Count);
	CmdEfuseBufferModeRead->u2Offset = cpu2le16(CmdEfuseBufferModeRead->u2Offset);
#endif
	MtAndesAppendCmdMsg(msg, (char *)CmdEfuseBufferModeRead, cmd_size);
	ret = chip_cmd_tx(pAd, msg);

done:

	if (CmdEfuseBufferModeRead)
		os_free_mem(CmdEfuseBufferModeRead);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
}

/*****************************************
 *	ExT_CID = 0x27
 *****************************************/
INT32 MtCmdEdcaParameterSet(RTMP_ADAPTER *pAd, MT_EDCA_CTRL_T *EdcaParam)
{
	struct cmd_msg *msg;
#ifdef RT_BIG_ENDIAN
	P_TX_AC_PARAM_T pAcParam;
	INT32 i = 0;
#endif
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	size = 4 + sizeof(TX_AC_PARAM_T) * EdcaParam->ucTotalNum;
#ifdef RT_BIG_ENDIAN

	for (i = 0; i < EdcaParam->ucTotalNum; i++) {
		pAcParam = &EdcaParam->rAcParam[i];
		pAcParam->u2Txop = cpu2le16(pAcParam->u2Txop);
		pAcParam->u2WinMax = cpu2le16(pAcParam->u2WinMax);
	}

#endif
	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EDCA_SET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);

	if (size <= sizeof(MT_EDCA_CTRL_T))
		MtAndesAppendCmdMsg(msg, (char *)EdcaParam, size);

	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

/*****************************************
 * ExT_CID = 0x28
 *****************************************/
INT32 MtCmdSlotTimeSet(RTMP_ADAPTER *pAd, UINT8 SlotTime,
					   UINT8 SifsTime, UINT8 RifsTime, UINT16 EifsTime, UCHAR BandIdx)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	CMD_SLOT_TIME_SET_T cmdSlotTime;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&cmdSlotTime, sizeof(CMD_SLOT_TIME_SET_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SLOT_TIME_SET_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SLOT_TIME_SET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	cmdSlotTime.u2Eifs = cpu2le16(EifsTime);
	cmdSlotTime.ucRifs = RifsTime;
	cmdSlotTime.ucSifs = SifsTime;
	cmdSlotTime.ucSlotTime = SlotTime;
	cmdSlotTime.ucBandNum = (UINT8)BandIdx;
	MtAndesAppendCmdMsg(msg, (char *)&cmdSlotTime, sizeof(CMD_SLOT_TIME_SET_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x23
 *****************************************/

INT32 MtCmdThermalProtect(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType
)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_THERMAL_PROTECT_T ThermalProtect;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ucBand:%d, HighEn:%d, HighTempTh:%d, LowEn:%d, LowTempTh:%d, RechkTimer:%d\n",
		ucBand, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "RFOffEn: %d, RFOffTh: %d, ucType: %d\n", RFOffEn, RFOffTh, ucType);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_THERMAL_PROTECT_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	/* Init firmware command content */
	os_zero_mem(&ThermalProtect, sizeof(ThermalProtect));

	/* config command content */
	ThermalProtect.u1ThermalCtrlFormatId = THERMAL_PROTECT_PARAMETER_CTRL;
	ThermalProtect.u1BandIdx = ucBand;
	ThermalProtect.u1HighEnable = HighEn;
	ThermalProtect.i1HighTempThreshold = HighTempTh;
	ThermalProtect.u1LowEnable = LowEn;
	ThermalProtect.i1LowTempThreshold = LowTempTh;
	ThermalProtect.u4RecheckTimer = cpu2le32(RechkTimer);
	ThermalProtect.u1RFOffEnable = RFOffEn;
	ThermalProtect.i1RFOffThreshold = RFOffTh;
	ThermalProtect.u1Type = ucType;

	MtAndesAppendCmdMsg(msg, (char *)&ThermalProtect, sizeof(ThermalProtect));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdThermalProtectAdmitDutyInfo(
	RTMP_ADAPTER *pAd
)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_THERMAL_PROTECT_T ThermalProtectInfo;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_THERMAL_PROTECT_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&ThermalProtectInfo, sizeof(ThermalProtectInfo));
	ThermalProtectInfo.u1ThermalCtrlFormatId = THERMAL_PROTECT_BASIC_INFO;

	MtAndesAppendCmdMsg(msg, (char *)&ThermalProtectInfo, sizeof(ThermalProtectInfo));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_THERMAL_PROTECT_T ThermalProtect;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_THERMAL_PROTECT_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&ThermalProtect, sizeof(ThermalProtect));
	ThermalProtect.u1ExtraTag = THERAML_PROTECTION_TAG_SET_ADMIT_DUTY;
	ThermalProtect.u1BandIdx = ucBand;
	ThermalProtect.u1Lv0Duty = (UINT8)u4Lv0Duty;
	ThermalProtect.u1Lv1Duty = (UINT8)u4Lv1Duty;
	ThermalProtect.u1Lv2Duty = (UINT8)u4Lv2Duty;
	ThermalProtect.u1Lv3Duty = (UINT8)u4Lv3Duty;
	MtAndesAppendCmdMsg(msg, (char *)&ThermalProtect, sizeof(ThermalProtect));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}


INT32
MtCmdThermalProtectEnable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protection_type,
	UINT8 trigger_type,
	INT32 trigger_temp,
	INT32 restore_temp,
	UINT16 recheck_time)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_ENABLE ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d, protect_type: %d\n",
		band_idx, protection_type);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"trigger_type: %d, trigger_temp: %d\n",
		trigger_type, trigger_temp);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"restore_temp: %d, recheck_time: %d\n",
		restore_temp, recheck_time);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_ENABLE));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_ENABLE));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_ENABLE;
	ext_cmd_buf.band_idx = band_idx;
	ext_cmd_buf.protection_type = protection_type;
	ext_cmd_buf.trigger_type = trigger_type;
	ext_cmd_buf.trigger_temp = trigger_temp;
	ext_cmd_buf.restore_temp = restore_temp;
	ext_cmd_buf.recheck_time = recheck_time;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_ENABLE));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectDisable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protection_type,
	UINT8 trigger_type)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_DISABLE ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d, protect_type: %d\n",
		band_idx, protection_type);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"trigger_type: %d\n",
		trigger_type);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DISABLE));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DISABLE));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_DISABLE;
	ext_cmd_buf.band_idx = band_idx;
	ext_cmd_buf.protection_type = protection_type;
	ext_cmd_buf.trigger_type = trigger_type;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DISABLE));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectDutyCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 level_idx,
	UINT8 duty)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_DUTY_CFG ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d, level_idx: %d\n",
		band_idx, level_idx);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"duty: %d\n", duty);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_CFG));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_CFG));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_DUTY_CONFIG;
	ext_cmd_buf.band_idx = band_idx;
	ext_cmd_buf.level_idx = level_idx;
	ext_cmd_buf.duty = duty;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_CFG));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	struct THERMAL_PROTECT_MECH_INFO *info_buf)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_INFO ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d\n", band_idx);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_INFO));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_INFO));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_MECH_INFO;
	ext_cmd_buf.band_idx = band_idx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_INFO));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectDutyInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	struct THERMAL_PROTECT_DUTY_INFO *info_buf)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_DUTY_INFO ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d\n", band_idx);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_INFO));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_INFO));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_DUTY_INFO;
	ext_cmd_buf.band_idx = band_idx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_DUTY_INFO));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32
MtCmdThermalProtectStateAct(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protect_type,
	UINT8 trig_type,
	UINT8 state)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_THERMAL_PROTECT_STATE_ACT ext_cmd_buf;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx: %d, protect_type: %d\n",
		band_idx, protect_type);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"trig_type: %d, state: %d\n",
		trig_type, state);

	msg = MtAndesAllocCmdMsg(pAd,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_STATE_ACT));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_STATE_ACT));
	ext_cmd_buf.sub_cmd_id = THERMAL_PROTECT_STATE_ACT;
	ext_cmd_buf.band_idx = band_idx;
	ext_cmd_buf.protect_type = protect_type;
	ext_cmd_buf.trig_type = trig_type;
	ext_cmd_buf.state = state;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_THERMAL_PROTECT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ext_cmd_buf,
		sizeof(struct _EXT_CMD_THERMAL_PROTECT_STATE_ACT));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x2c
 *****************************************/
static VOID MtCmdThemalSensorRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_THERMAL_SENSOR_INFO_T prEventExtCmdResult = (P_EXT_EVENT_THERMAL_SENSOR_INFO_T)Data;
	prEventExtCmdResult->u4SensorResult = le2cpu32(prEventExtCmdResult->u4SensorResult);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &prEventExtCmdResult->u4SensorResult, sizeof(prEventExtCmdResult->u4SensorResult));
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"ThemalSensor = 0x%x\n", prEventExtCmdResult->u4SensorResult);
}

INT32 MtCmdGetThermalSensorResult(RTMP_ADAPTER *pAd, UINT8 ActionIdx, UINT8 ucDbdcIdx, UINT32 *SensorResult)
{
	struct cmd_msg *msg;
	CMD_THERMAL_SENSOR_INFO_T ThermalSensorInfo;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ActionIdx: %d, uBandIdx: %d\n", ActionIdx, ucDbdcIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_THERMAL_SENSOR_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ThermalSensorInfo, sizeof(CMD_THERMAL_SENSOR_INFO_T));
	ThermalSensorInfo.u1ThermalCtrlFormatId = THERMAL_SENSOR_TEMPERATURE_QUERY ;
	ThermalSensorInfo.u1ActionIdx = ActionIdx;
	ThermalSensorInfo.u1BandIdx = ucDbdcIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_THERMAL_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, SensorResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdThemalSensorRsp);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ThermalSensorInfo, sizeof(EXT_EVENT_THERMAL_SENSOR_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x2d
 *****************************************/
INT32 MtCmdTmrCal(RTMP_ADAPTER *pAd, UINT8 Enable, UINT8 Band,
				  UINT8 Bw, UINT8 Ant, UINT8 Role)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_TMR_CAL_T TmrCal;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TMR_CAL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_TMR_CAL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&TmrCal, sizeof(TmrCal));
	TmrCal.ucEnable = Enable;
	TmrCal.ucBand = Band;
	TmrCal.ucBW = Bw;
	TmrCal.ucAnt = Ant;/* only ant 0 support at present. */
	TmrCal.ucRole = Role;
	MtAndesAppendCmdMsg(msg, (char *)&TmrCal, sizeof(TmrCal));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x2E
 *****************************************/
#ifdef MT_WOW_SUPPORT
static VOID EventExtCmdPacketFilterRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_PF_GENERAL_T pPFRsp = (P_EXT_EVENT_PF_GENERAL_T)Data;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u4PfCmdType = 0x%x u4Status = 0x%x\n",
			  le2cpu32(pPFRsp->u4PfCmdType), le2cpu32(pPFRsp->u4Status));
}

static VOID EventExtCmdWakeupOptionRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_WAKEUP_OPTION_T pWakeOptRsp = (P_EXT_EVENT_WAKEUP_OPTION_T)Data;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u4PfCmdType = 0x%x u4Status = 0x%x\n",
			  le2cpu32(pWakeOptRsp->u4PfCmdType), le2cpu32(pWakeOptRsp->u4Status));
}

VOID MT76xxAndesWOWEnable(
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
	/* hw-enable cmd */
	/* 1. magic, parameter=enable */
	/* 2. eapol , param = enable */
	/* 3. bssid , param = bssid[3:0] */
	/* 4. mode, parm = white */
	/* 5. PF, param = enable */
	/* wakeup command param = choose usb. others dont' care */
	struct wifi_dev *wdev = &pStaCfg->wdev;
	UINT32 BandIdx = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_PACKET_FILTER_GLOBAL_T CmdPFGlobal;
	CMD_PACKET_FILTER_GTK_T CmdGTK;
	CMD_PACKET_FILTER_ARPNS_T CmdArpNs;
	CMD_PACKET_FILTER_WAKEUP_OPTION_T CmdWakeupOption;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	ASSERT(pEntry);
#ifdef DBDC_MODE
	BandIdx = HcGetBandByWdev(wdev);
#endif /* DBDC_MODE */

	/* Security configuration */
	if (IS_AKM_PSK(pEntry->SecConfig.AKMMap)) {
		os_zero_mem(&CmdGTK, sizeof(CmdGTK));
		msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GTK_T));

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PACKET_FILTER);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PF_GENERAL_T));
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdPacketFilterRsp);
		AndesInitCmdMsg(msg, attr);
		CmdGTK.PFType = cpu2le32(_ENUM_TYPE_GTK_REKEY);

		if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))
			CmdGTK.WPAVersion = cpu2le32(PF_WPA);
		else
			CmdGTK.WPAVersion = cpu2le32(PF_WPA2);

		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Bssid("MACSTR"), Wcid(%d, %d), McMcIdx(%d)\n",
				  MAC2STR(wdev->bssid),
				  pEntry->wcid, wdev->tr_tb_idx,
				  wdev->bss_info_argument.bmc_wlan_idx);
		/* TODO: Pat: how if big endian */
		NdisCopyMemory(CmdGTK.PTK, pEntry->SecConfig.PTK, 64);
		CmdGTK.BssidIndex = cpu2le32(wdev->bss_info_argument.ucBssIndex);
		CmdGTK.OwnMacIndex = cpu2le32(wdev->OmacIdx);
		CmdGTK.WmmIndex = cpu2le32(PF_WMM_0);

		if (IS_AKM_PSK(pEntry->SecConfig.AKMMap)) {
			NdisCopyMemory(CmdGTK.ReplayCounter, pEntry->SecConfig.Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);
			CmdGTK.GroupKeyIndex = cpu2le32(wdev->bss_info_argument.bmc_wlan_idx);
			CmdGTK.PairKeyIndex = cpu2le32(pEntry->wcid);
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "GTK offload::BssidIndex %d, GroupKeyIndex %d, OwnMacIndex %d, PairKeyIndex %d, WmmIndex %d\n",
				   CmdGTK.BssidIndex, CmdGTK.GroupKeyIndex, CmdGTK.OwnMacIndex, CmdGTK.PairKeyIndex, CmdGTK.WmmIndex);
		AndesAppendCmdMsg(msg, (char *)&CmdGTK, sizeof(CMD_PACKET_FILTER_GTK_T));
		ret = AndesSendCmdMsg(pAd, msg);
	}

	/* ARP/NS offlaod */
	os_zero_mem(&CmdArpNs, sizeof(CMD_PACKET_FILTER_ARPNS_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_ARPNS_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PACKET_FILTER);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PF_GENERAL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdPacketFilterRsp);
	AndesInitCmdMsg(msg, attr);
	CmdArpNs.PFType = cpu2le32(_ENUM_TYPE_ARPNS);
	CmdArpNs.IPIndex = cpu2le32(PF_ARP_NS_SET_0);
	CmdArpNs.Enable = cpu2le32(PF_ARP_NS_ENABLE);
	CmdArpNs.BssidEnable = cpu2le32(PF_BSSID_0);
	CmdArpNs.Offload = cpu2le32(PF_ARP_OFFLOAD);
	CmdArpNs.Type = cpu2le32(PF_ARP_NS_ALL_PKT);
	CmdArpNs.IPAddress[0] = pAd->WOW_Cfg.IPAddress[0];	/* 192 */
	CmdArpNs.IPAddress[1] = pAd->WOW_Cfg.IPAddress[1];	/* 168 */
	CmdArpNs.IPAddress[2] = pAd->WOW_Cfg.IPAddress[2];	/* 2 */
	CmdArpNs.IPAddress[3] = pAd->WOW_Cfg.IPAddress[3];	/* 10 */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ARP offload::IP(%d:%d:%d:%d)\n",
			  CmdArpNs.IPAddress[0], CmdArpNs.IPAddress[1], CmdArpNs.IPAddress[2], CmdArpNs.IPAddress[3]);
	AndesAppendCmdMsg(msg, (char *)&CmdArpNs, sizeof(CMD_PACKET_FILTER_ARPNS_T));
	ret = AndesSendCmdMsg(pAd, msg);
	/* Wakeup option */
	os_zero_mem(&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WAKEUP_OPTION);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_WAKEUP_OPTION_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdWakeupOptionRsp);
	AndesInitCmdMsg(msg, attr);
	CmdWakeupOption.WakeupInterface = cpu2le32(pAd->WOW_Cfg.nWakeupInterface);

	if (pAd->WOW_Cfg.nWakeupInterface == WOW_WAKEUP_BY_GPIO) {
		UINT32 GpioParameter = 0;
		CmdWakeupOption.GPIONumber = cpu2le32(pAd->WOW_Cfg.nSelectedGPIO); /* which GPIO */
		CmdWakeupOption.GPIOTimer = cpu2le32(pAd->WOW_Cfg.nHoldTime); /* unit is us */

		if (pAd->WOW_Cfg.bGPIOHighLow == WOW_GPIO_LOW_TO_HIGH)
			GpioParameter = WOW_GPIO_LOW_TO_HIGH_PARAMETER;
		else
			GpioParameter = WOW_GPIO_HIGH_TO_LOW_PARAMETER;

		CmdWakeupOption.GpioParameter = cpu2le32(GpioParameter);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Wakeup Option::nWakeupInterface(%d), GPIONumber(%d), GPIOTimer(%d), GpioParameter(0x%x)\n",
			  CmdWakeupOption.WakeupInterface,
			  CmdWakeupOption.GPIONumber,
			  CmdWakeupOption.GPIOTimer,
			  CmdWakeupOption.GpioParameter);
	AndesAppendCmdMsg(msg, (char *)&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));
	ret = AndesSendCmdMsg(pAd, msg);
	/* WOW enable */
	os_zero_mem(&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PACKET_FILTER);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PF_GENERAL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdPacketFilterRsp);
	AndesInitCmdMsg(msg, attr);
	CmdPFGlobal.PFType = cpu2le32(_ENUM_TYPE_GLOBAL_EN);
	CmdPFGlobal.FunctionSelect = cpu2le32(_ENUM_GLOBAL_WOW_EN);
	CmdPFGlobal.Enable = cpu2le32(PF_BSSID_0);
	CmdPFGlobal.Band = cpu2le32(BandIdx);/* cpu2le32(PF_BAND_0); */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Wakeup option::Band(%d)\n",
			  BandIdx);
	AndesAppendCmdMsg(msg, (char *)&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
}

VOID MT76xxAndesWOWDisable(
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
	struct wifi_dev *wdev = &pStaCfg->wdev;
	UINT32 BandIdx = 0;
	CMD_PACKET_FILTER_GLOBAL_T CmdPFGlobal;
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ASSERT(wdev);
#ifdef DBDC_MODE
	BandIdx = HcGetBandByWdev(wdev);
#endif /* DBDC_MODE */
	/* WOW disable */
	os_zero_mem(&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PACKET_FILTER);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PF_GENERAL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdPacketFilterRsp);
	AndesInitCmdMsg(msg, attr);
	CmdPFGlobal.PFType = cpu2le32(_ENUM_TYPE_GLOBAL_EN);
	CmdPFGlobal.FunctionSelect = cpu2le32(_ENUM_GLOBAL_WOW_EN);
	CmdPFGlobal.Enable = cpu2le32(PF_BSSID_DISABLE);
	CmdPFGlobal.Band = cpu2le32(BandIdx);/* cpu2le32(PF_BAND_0); */
	AndesAppendCmdMsg(msg, (char *)&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
}

#endif

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
/*****************************************
 *	ExT_CID = 0x30
 *****************************************/
static VOID MtCmdGetTxStatisticRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T p_event_hdr =
		(P_EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)Data;
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat =
		(P_EXT_EVENT_TX_STATISTIC_RESULT_T)msg->attr.rsp.wb_buf_in_calbk;

	prTxStat->ucField = p_event_hdr->ucField;

	if (prTxStat->ucField & GET_TX_STAT_TOTAL_TX_CNT) {

		P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		prTxStat->u4TotalTxCount =
			le2cpu32(p_event_data->u4TotalTxCount);
		prTxStat->u4TotalTxFailCount =
			le2cpu32(p_event_data->u4TotalTxFailCount);
		prTxStat->u4CurrBwTxCnt =
			le2cpu32(p_event_data->u4CurrBwTxCnt);
		prTxStat->u4OtherBwTxCnt =
			le2cpu32(p_event_data->u4OtherBwTxCnt);
	}

	if (prTxStat->ucField & GET_TX_STAT_LAST_TX_RATE) {

		P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		os_move_mem(&prTxStat->rLastTxRate,
					&p_event_data->rLastTxRate, sizeof(RA_PHY_CFG_T));
	}

	if (prTxStat->ucField & GET_TX_STAT_ENTRY_TX_RATE) {

		P_EXT_EVENT_TX_STATISTIC_WLAN_RATE_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_RATE_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		os_move_mem(&prTxStat->rEntryTxRate,
					&p_event_data->rEntryTxRate, sizeof(RA_PHY_CFG_T));
	}

	if (prTxStat->ucField & GET_TX_STAT_ENTRY_TX_CNT) {

		P_EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		prTxStat->u4EntryTxCount =
			le2cpu32(p_event_data->u4EntryTxCount);
		prTxStat->u4EntryTxFailCount =
			le2cpu32(p_event_data->u4EntryTxFailCount);
	}

	if (prTxStat->ucField & GET_TX_STAT_ENTRY_TX_PER) {
		P_EXT_EVENT_TX_STATISTIC_WLAN_PER_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_PER_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		prTxStat->ucEntryTxPer =
			le2cpu32(p_event_data->ucEntryTxPer);
	}
}


INT32 MtCmdGetTxStatistic(struct _RTMP_ADAPTER *pAd, UINT8 ucField, UINT8 ucBand,
	UINT16 u2Wcid, P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStatResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_TX_STATISTIC_T rTxStatCmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&rTxStatCmd, sizeof(rTxStatCmd));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u4Field=0x%8x, Wcid=%d\n", ucField, u2Wcid);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(rTxStatCmd));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_TX_STATISTICS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prTxStatResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetTxStatisticRsp);
	MtAndesInitCmdMsg(msg, attr);
	rTxStatCmd.ucField = ucField;
	rTxStatCmd.u2WlanIdx = u2Wcid;
	rTxStatCmd.ucBandIdx = ucBand;
	MtAndesAppendCmdMsg(msg, (char *)&rTxStatCmd, sizeof(rTxStatCmd));
	ret = chip_cmd_tx(pAd, msg);
error:

	if (ret)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
				 "(ret = %d)\n", ret);

	return ret;
}

/*not need to wait response for a specific STA txcount*/
INT32 mt_cmd_get_sta_tx_statistic(
	struct _RTMP_ADAPTER *ad, TX_STAT_STRUC *p_buff, UCHAR num)
{
	struct cmd_msg *msg = NULL;
	INT32 ret = 0, i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	EXT_CMD_GET_TX_STATISTIC_T *p_cmd_msg = NULL;
	TX_STAT_STRUC *p_temp = NULL;

	os_alloc_mem_suspend(ad, (UCHAR **)&p_cmd_msg, sizeof(EXT_CMD_GET_TX_STATISTIC_T));

	if (!p_cmd_msg) {
		ret = -1;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(ad, sizeof(EXT_CMD_GET_TX_STATISTIC_T) * num);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_TX_STATISTICS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	MtAndesInitCmdMsg(msg, attr);

	for (i = 0; i < num; i++) {
		p_temp = p_buff + i;

		os_zero_mem(p_cmd_msg, sizeof(EXT_CMD_GET_TX_STATISTIC_T));
		p_cmd_msg->ucField = p_temp->Field;
		p_cmd_msg->ucBandIdx = p_temp->Band;
		p_cmd_msg->u2WlanIdx = p_temp->Wcid;

		MtAndesAppendCmdMsg(msg, (char *)p_cmd_msg, sizeof(EXT_CMD_GET_TX_STATISTIC_T));
	}

	ret = chip_cmd_tx(ad, msg);
error:

	if (ret)
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s:(ret = %d)\n", __func__, ret);

	if (p_cmd_msg)
		os_free_mem(p_cmd_msg);

	return ret;
}

static VOID MtCmdMutiGetTxStatisticRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT32 index;

	P_EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T p_event_hdr =
		(P_EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)Data;
	P_TX_STATISTIC_RESULT_PAIR prTxStatPair = (P_TX_STATISTIC_RESULT_PAIR) msg->attr.rsp.wb_buf_in_calbk;
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = NULL;
	UINT32 num = 0;
	UCHAR field = p_event_hdr->ucField;

	/*for every category to do handler*/
	if (field & GET_TX_STAT_ENTRY_TX_CNT) {
		P_EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		num = (Len - sizeof(EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)) / sizeof(EXT_EVENT_TX_STATISTIC_WLAN_CNT_RESULT_T);

		for (index = 0; index < num; index++) {

			prTxStatPair->ucField = field;

			prTxStat = &prTxStatPair->txStatisticRes;
			prTxStat->ucField = field;

			prTxStatPair->u2WlanIdx =
				le2cpu16(p_event_data->u2WlanIdx);
			prTxStat->u4EntryTxCount =
				le2cpu32(p_event_data->u4EntryTxCount);
			prTxStat->u4EntryTxFailCount =
				le2cpu32(p_event_data->u4EntryTxFailCount);

			p_event_data++;
			prTxStatPair++;
		}
	} else if (field & GET_TX_STAT_ENTRY_TX_RATE) {
		P_EXT_EVENT_TX_STATISTIC_WLAN_RATE_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_RATE_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		num = (Len - sizeof(EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)) / sizeof(EXT_EVENT_TX_STATISTIC_WLAN_RATE_RESULT_T);

		for (index = 0; index < num; index++) {

			prTxStatPair->ucField = field;
			prTxStat = &prTxStatPair->txStatisticRes;
			prTxStat->ucField = field;

			prTxStatPair->u2WlanIdx =
				le2cpu16(p_event_data->u2WlanIdx);
			os_move_mem(&prTxStat->rEntryTxRate,
						&p_event_data->rEntryTxRate, sizeof(RA_PHY_CFG_T));

			p_event_data++;
			prTxStatPair++;
		}
	} else if (field & GET_TX_STAT_ENTRY_TX_PER) {
		P_EXT_EVENT_TX_STATISTIC_WLAN_PER_RESULT_T p_event_data =
		(P_EXT_EVENT_TX_STATISTIC_WLAN_PER_RESULT_T)(p_event_hdr->aucTxStatisticResult);

		num = (Len - sizeof(EXT_EVENT_TX_STATISTIC_RESULT_HEADER_T)) / sizeof(EXT_EVENT_TX_STATISTIC_WLAN_PER_RESULT_T);

		for (index = 0; index < num; index++) {

			prTxStatPair->ucField = field;
			prTxStat = &prTxStatPair->txStatisticRes;
			prTxStat->ucField = field;

			prTxStatPair->u2WlanIdx =
				le2cpu16(p_event_data->u2WlanIdx);
			prTxStat->ucEntryTxPer = p_event_data->ucEntryTxPer;

			p_event_data++;
			prTxStatPair++;
		}
	} else {

		if (field & GET_TX_STAT_TOTAL_TX_CNT) {
			P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T p_event_data =
			(P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T)(p_event_hdr->aucTxStatisticResult);

			prTxStatPair->ucBand = p_event_data->ucBandIdx;
			prTxStatPair->ucField = field;
			prTxStat = &prTxStatPair->txStatisticRes;
			prTxStat->ucField = field;

			prTxStat->u4TotalTxCount =
				le2cpu32(p_event_data->u4TotalTxCount);
			prTxStat->u4TotalTxFailCount =
				le2cpu32(p_event_data->u4TotalTxFailCount);
			prTxStat->u4CurrBwTxCnt =
				le2cpu32(p_event_data->u4CurrBwTxCnt);
			prTxStat->u4OtherBwTxCnt =
				le2cpu32(p_event_data->u4OtherBwTxCnt);
		}

		if (field & GET_TX_STAT_LAST_TX_RATE) {
			P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T p_event_data =
			(P_EXT_EVENT_TX_STATISTIC_BAND_RESULT_T)(p_event_hdr->aucTxStatisticResult);

			prTxStatPair->ucBand = p_event_data->ucBandIdx;
			prTxStatPair->ucField = field;
			prTxStat = &prTxStatPair->txStatisticRes;
			prTxStat->ucField = field;

			os_move_mem(&prTxStat->rLastTxRate,
						&p_event_data->rLastTxRate, sizeof(RA_PHY_CFG_T));
		}
	}
}

INT32 MtCmdGetMutiTxStatistic(struct _RTMP_ADAPTER *pAd, TX_STATISTIC_RESULT_PAIR *TxStatResultPair, UINT32 Num)
{
	UINT32 index;
	struct cmd_msg *msg;
	EXT_CMD_GET_TX_STATISTIC_T rTxStatCmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(rTxStatCmd) * Num);
	if (!msg || !TxStatResultPair) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_TX_STATISTICS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, TxStatResultPair);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdMutiGetTxStatisticRsp);
	MtAndesInitCmdMsg(msg, attr);

	for (index = 0; index < Num; index++){
		os_zero_mem(&rTxStatCmd, sizeof(EXT_CMD_GET_TX_STATISTIC_T));
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s:u4Field=0x%8x, Wcid=%d\n", __func__,
		TxStatResultPair[index].ucField, TxStatResultPair[index].u2WlanIdx);
		rTxStatCmd.ucField = TxStatResultPair[index].ucField;
		rTxStatCmd.u2WlanIdx = TxStatResultPair[index].u2WlanIdx;
		rTxStatCmd.ucBandIdx = TxStatResultPair[index].ucBand;
		MtAndesAppendCmdMsg(msg, (char *)&rTxStatCmd, sizeof(EXT_CMD_GET_TX_STATISTIC_T));
	}
	ret = chip_cmd_tx(pAd, msg);

error:
	if (ret)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
				 "(ret = %d)\n", ret);

	return ret;
}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#ifdef PRETBTT_INT_EVENT_SUPPORT
/*****************************************
 *    ExT_CID = 0x33
 *****************************************/
INT32 MtCmdTrgrPretbttIntEventSet(RTMP_ADAPTER *pAd,
								  CMD_TRGR_PRETBTT_INT_EVENT_T trgr_pretbtt_int_event)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(CMD_TRGR_PRETBTT_INT_EVENT_T);
	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
				 "Error Allocate Fail\n");
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TRGR_PRETBTT_INT_EVENT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "bcn_update.ucEnable = %d\n",
			  trgr_pretbtt_int_event.ucEnable);
#ifdef RT_BIG_ENDIAN
	trgr_pretbtt_int_event.u2BcnPeriod = cpu2le16(trgr_pretbtt_int_event.u2BcnPeriod);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&trgr_pretbtt_int_event, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

VOID MtSetTriggerPretbttIntEvent(RTMP_ADAPTER *pAd, INT apidx,
								 UCHAR HWBssidIdx, BOOLEAN Enable, UINT16 BeaconPeriod)
{
	CMD_TRGR_PRETBTT_INT_EVENT_T trgr_pretbtt_int_event;
	os_zero_mem(&trgr_pretbtt_int_event, sizeof(CMD_TRGR_PRETBTT_INT_EVENT_T));
	trgr_pretbtt_int_event.ucHwBssidIdx = HWBssidIdx;

	if (HWBssidIdx > 0) /* HWBssid > 0 case, no extendable bssid. */
		trgr_pretbtt_int_event.ucExtBssidIdx = 0;
	else
		trgr_pretbtt_int_event.ucExtBssidIdx = (UINT8)apidx;

	trgr_pretbtt_int_event.ucEnable = Enable;
	trgr_pretbtt_int_event.u2BcnPeriod = BeaconPeriod;
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "trgr_pretbtt_int_event.ucHwBssidIdx = %d\n",
			  trgr_pretbtt_int_event.ucHwBssidIdx);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "trgr_pretbtt_int_event.ucExtBssidIdx = %d\n",
			  trgr_pretbtt_int_event.ucExtBssidIdx);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "trgr_pretbtt_int_event.u2BcnPeriod = %d\n",
			  trgr_pretbtt_int_event.u2BcnPeriod);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_USB, DBG_LVL_INFO,
			 "trgr_pretbtt_int_event.ucEnable = %d\n",
			  trgr_pretbtt_int_event.ucEnable);
	MtCmdTrgrPretbttIntEventSet(pAd, trgr_pretbtt_int_event);
}
#endif /*PRETBTT_INT_EVENT_SUPPORT*/

/*****************************************
 *    ExT_CID = 0x48
 *****************************************/
INT32 MtCmdMuarConfigSet(RTMP_ADAPTER *pAd, UCHAR *pdata)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	EXT_CMD_MUAR_T *pconfig_muar = (EXT_CMD_MUAR_T *)pdata;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(EXT_CMD_MUAR_T);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:ucMuarModeSel = %d, ucForceClear = %d, ucEntryCnt = %d, ucAccessMode = %d\n",
			  __func__, pconfig_muar->ucMuarModeSel,
			  pconfig_muar->ucForceClear, pconfig_muar->ucEntryCnt,
			  pconfig_muar->ucAccessMode);
	size = size + (pconfig_muar->ucEntryCnt * sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CONFIG_MUAR);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pconfig_muar, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef OCE_SUPPORT
INT32 MtCmdFdFrameOffloadSet(RTMP_ADAPTER *pAd, P_CMD_FD_FRAME_OFFLOAD_T fdFrame_offload)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(CMD_FD_FRAME_OFFLOAD_T);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Enable = %d, OwnMacIdx = %d, WlanIdx = %d, Band = %d, Len = %d\n",
			  fdFrame_offload->ucEnable, fdFrame_offload->ucOwnMacIdx,
			  fdFrame_offload->ucWlanIdx, fdFrame_offload->ucBandIdx, fdFrame_offload->u2PktLength);
	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_FD_FRAME_OFFLOAD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	fdFrame_offload->u2PktLength = cpu2le16(fdFrame_offload->u2PktLength);
	fdFrame_offload->u2TimestampFieldPos = cpu2le16(fdFrame_offload->u2TimestampFieldPos);
#endif
	MtAndesAppendCmdMsg(msg, (char *)fdFrame_offload, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* OCE_SUPPORT */

#ifdef BCN_OFFLOAD_SUPPORT
/*****************************************
 *    ExT_CID = 0x49
 *****************************************/
INT32 MtCmdBcnOffloadSet(RTMP_ADAPTER *pAd, CMD_BCN_OFFLOAD_T *bcn_offload)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(CMD_BCN_OFFLOAD_T);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			"Enable=%d, OM=%d, WlanIdx=%d, Band=%d, Len=%d, TimOffset=%d\n",
			bcn_offload->ucEnable, bcn_offload->ucOwnMacIdx,
			bcn_offload->ucWlanIdx, bcn_offload->ucBandIdx,
			bcn_offload->u2PktLength, bcn_offload->u2TimIePos);

	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BCN_OFFLOAD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	bcn_offload->u2PktLength = cpu2le16(bcn_offload->u2PktLength);
	bcn_offload->u2TimIePos = cpu2le16(bcn_offload->u2TimIePos);
	bcn_offload->u2CsaIePos = cpu2le16(bcn_offload->u2CsaIePos);
	bcn_offload->u2BccIePos = cpu2le16(bcn_offload->u2BccIePos);
#endif
	MtAndesAppendCmdMsg(msg, (char *)bcn_offload, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
INT32 MtCmdBcnV2OffloadSet(RTMP_ADAPTER *pAd, CMD_BCN_OFFLOAD_T_V2 *bcn_offload_v2)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	size = sizeof(CMD_BCN_OFFLOAD_T_V2);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			 "Enable = %d, OwnMacIdx = %d, WlanIdx = %d, Band = %d, Len = %d\n",
			  bcn_offload_v2->ucEnable, bcn_offload_v2->ucOwnMacIdx,
			  bcn_offload_v2->ucWlanIdx, bcn_offload_v2->ucBandIdx, bcn_offload_v2->u2PktLength);
	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BCN_OFFLOAD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	bcn_offload_v2->u2PktLength = cpu2le16(bcn_offload_v2->u2PktLength);
	bcn_offload_v2->u2TimIePos = cpu2le16(bcn_offload_v2->u2TimIePos);
	bcn_offload_v2->u2CsaIePos = cpu2le16(bcn_offload_v2->u2CsaIePos);
#endif
	MtAndesAppendCmdMsg(msg, (char *)bcn_offload_v2, size);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif
#endif /*BCN_OFFLOAD_SUPPORT*/

/********************************/
/* EXT_EVENT_ID_DRR_CTRL = 0x36 */
/********************************/
static VOID MtCmdSetVoWDRRCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_CMD_VOW_DRR_CTRL_T *EventExtCmdResult = (struct _EXT_CMD_VOW_DRR_CTRL_T *)Data;
#if (NEW_MCU_INIT_CMD_API)
	NdisCopyMemory(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(struct _EXT_CMD_VOW_DRR_CTRL_T));
#else
	NdisCopyMemory(msg->rsp_payload, Data, sizeof(struct _EXT_CMD_VOW_DRR_CTRL_T));
#endif /* NEW_MCU_INIT_CMD_API */
#if (NEW_MCU_INIT_CMD_API)
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->attr.ext_type);
#else
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->ext_cmd_type);
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: ucCtrlStatus = 0x%x\n",
			 __func__, EventExtCmdResult->ucCtrlStatus);
}

/*************************************/
/* EXT_EVENT_ID_BSSGROUP_CTRL = 0x37 */
/*************************************/
static VOID MtCmdSetVoWGroupCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_CMD_BSS_CTRL_T *EventExtCmdResult = (struct _EXT_CMD_BSS_CTRL_T *)Data;
#if (NEW_MCU_INIT_CMD_API)
	NdisCopyMemory(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(struct _EXT_CMD_BSS_CTRL_T));
#else
	NdisCopyMemory(msg->rsp_payload, Data, sizeof(struct _EXT_CMD_BSS_CTRL_T));
#endif /* NEW_MCU_INIT_CMD_API */
#if (NEW_MCU_INIT_CMD_API)
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, le2cpu32(EventExtCmdResult->u4CtrlFieldID), msg->attr.ext_type);
#else
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, le2cpu32(EventExtCmdResult->u4CtrlFieldID), msg->ext_cmd_type);
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: ucCtrlStatus = 0x%x\n",
			 __func__, EventExtCmdResult->ucCtrlStatus);
}
/****************************************/
/* EXT_EVENT_ID_VOW_FEATURE_CTRL = 0x38 */
/***************************************/
static VOID MtCmdSetVoWFeatureCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	/* RTMP_ADAPTER *pad = (RTMP_ADAPTER *)(msg->priv); */
	struct _EXT_CMD_VOW_FEATURE_CTRL_T *EventExtCmdResult = (struct _EXT_CMD_VOW_FEATURE_CTRL_T *)Data;
#ifdef RT_BIG_ENDIAN
	UINT16 *u16p = NULL;
	EventExtCmdResult->u2IfApplyBss_0_to_16_CtrlFlag = le2cpu16(EventExtCmdResult->u2IfApplyBss_0_to_16_CtrlFlag);
	u16p = &(EventExtCmdResult->u2IfApplyBss_0_to_16_CtrlFlag) + 1;
	*u16p = cpu2le16(*u16p);
	EventExtCmdResult->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = le2cpu16(EventExtCmdResult->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag);
	EventExtCmdResult->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = le2cpu16(EventExtCmdResult->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag);
#endif
#if (NEW_MCU_INIT_CMD_API)
	NdisCopyMemory(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(struct _EXT_CMD_VOW_FEATURE_CTRL_T));
#else
	NdisCopyMemory(msg->rsp_payload, Data, sizeof(struct _EXT_CMD_VOW_FEATURE_CTRL_T));
#endif /* NEW_MCU_INIT_CMD_API */
#if (NEW_MCU_INIT_CMD_API)
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: ExtCmd = 0x%x\n",
			 __func__, msg->attr.ext_type);
#else
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: ExtCmd = 0x%x\n",
			 __func__, msg->ext_cmd_type);
#endif /* NEW_MCU_INIT_CMD_API */

	/* if (EventExtCmdResult->ucCtrlStatus == FALSE) */
	if (1) {
#if (NEW_MCU_INIT_CMD_API)

		if (IS_CMD_ATTR_SET_QUERY_FLAG_SET(msg->attr))
#else
		if (msg->set_query == CMD_QUERY)
#endif /* NEW_MCU_INIT_CMD_API */
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Get fail!\n", __func__);
		else
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Set fail!\n", __func__);

		if (EventExtCmdResult->u2IfApplyBss_0_to_16_CtrlFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2Bss_0_to_16_CtrlValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2Bss_0_to_16_CtrlValue);
		}

		if (EventExtCmdResult->u2IfApplyRefillPerildFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2RefillPerildValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2RefillPerildValue);
		}

		if (EventExtCmdResult->u2IfApplyDbdc1SearchRuleFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2Dbdc1SearchRuleValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2Dbdc1SearchRuleValue);
		}

		if (EventExtCmdResult->u2IfApplyDbdc0SearchRuleFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2Dbdc0SearchRuleValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2Dbdc0SearchRuleValue);
		}

		if (EventExtCmdResult->u2IfApplyEnTxopNoChangeBssFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2EnTxopNoChangeBssValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2EnTxopNoChangeBssValue);
		}

		if (EventExtCmdResult->u2IfApplyAirTimeFairnessFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2AirTimeFairnessValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2AirTimeFairnessValue);
		}

		if (EventExtCmdResult->u2IfApplyEnbwrefillFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2EnbwrefillValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2EnbwrefillValue);
		}

		if (EventExtCmdResult->u2IfApplyEnbwCtrlFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2EnbwCtrlValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2EnbwCtrlValue);
		}

		if (EventExtCmdResult->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2BssCheckTimeToken_0_to_16_CtrlValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2BssCheckTimeToken_0_to_16_CtrlValue);
		}

		if (EventExtCmdResult->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u2BssCheckLengthToken_0_to_16_CtrlValue = 0x%0x\n",
					 __func__, EventExtCmdResult->u2BssCheckLengthToken_0_to_16_CtrlValue);
		}
	}

	/* need to ask FW to add ExtendCID and CtrlStatus */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: i am here~\n", __func__);
}

/***************************************/
/* EXT_EVENT_ID_RX_AIRTIME_CTRL = 0x4a */
/***************************************/
static VOID MtCmdSetVoWRxAirtimeCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_CMD_RX_AT_CTRL_T *EventExtCmdResult = (struct _EXT_CMD_RX_AT_CTRL_T *)Data;
#ifdef RT_BIG_ENDIAN
	EventExtCmdResult->u4CtrlFieldID = le2cpu16(EventExtCmdResult->u4CtrlFieldID);
	EventExtCmdResult->u4CtrlSubFieldID = le2cpu16(EventExtCmdResult->u4CtrlSubFieldID);
	EventExtCmdResult->u4CtrlSetStatus = le2cpu32(EventExtCmdResult->u4CtrlSetStatus);
	EventExtCmdResult->u4CtrlGetStatus = le2cpu32(EventExtCmdResult->u4CtrlGetStatus);
	EventExtCmdResult->u4ReserveDW[0] = le2cpu32(EventExtCmdResult->u4ReserveDW[0]);
	EventExtCmdResult->u4ReserveDW[1] = le2cpu32(EventExtCmdResult->u4ReserveDW[1]);
#endif
	NdisMoveMemory(msg->attr.rsp.wb_buf_in_calbk, Data, Len);
#if (NEW_MCU_INIT_CMD_API)
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->attr.ext_type);
#else
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->ext_cmd_type);
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlGetStatus = 0x%x\n",
			 __func__, EventExtCmdResult->u4CtrlGetStatus);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlSetStatus = 0x%x\n",
			 __func__, EventExtCmdResult->u4CtrlSetStatus);

	/* show get RX Non Wi-Fi and OBSS counter */
	if (EventExtCmdResult->u4CtrlFieldID == EMUM_RX_AT_REPORT_CTRL) {
		if (EventExtCmdResult->u4CtrlSubFieldID == ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Non Wi-Fi for band%d = 0x%x\n",
					 __func__, EventExtCmdResult->rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxNonWiFiBandIdx,
					 EventExtCmdResult->rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer);
		} else if (EventExtCmdResult->u4CtrlSubFieldID == ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME) {
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: OBSS for band%d = 0x%x\n",
					 __func__, EventExtCmdResult->rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxObssBandIdx,
					 EventExtCmdResult->rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer);
		}
	}
}

/**************************************/
/* EXT_EVENT_ID_AT_PROC_MODULE = 0x4b */
/**************************************/
static VOID MtCmdSetVoWModuleCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_CMD_RX_AT_CTRL_T *EventExtCmdResult = (struct _EXT_CMD_RX_AT_CTRL_T *)Data;
#ifdef RT_BIG_ENDIAN
	EventExtCmdResult->u4CtrlFieldID = le2cpu16(EventExtCmdResult->u4CtrlFieldID);
	EventExtCmdResult->u4CtrlSubFieldID = le2cpu16(EventExtCmdResult->u4CtrlSubFieldID);
	EventExtCmdResult->u4CtrlSetStatus = le2cpu32(EventExtCmdResult->u4CtrlSetStatus);
	EventExtCmdResult->u4CtrlGetStatus = le2cpu32(EventExtCmdResult->u4CtrlGetStatus);
	EventExtCmdResult->u4ReserveDW[0] = le2cpu32(EventExtCmdResult->u4ReserveDW[0]);
	EventExtCmdResult->u4ReserveDW[1] = le2cpu32(EventExtCmdResult->u4ReserveDW[1]);
#endif
#if (NEW_MCU_INIT_CMD_API)
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->attr.ext_type);
#else
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlFieldID = 0x%x, ExtCmd (0x%02x)\n",
			 __func__, EventExtCmdResult->u4CtrlFieldID, msg->ext_cmd_type);
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlGetStatus = 0x%x\n",
			 __func__, EventExtCmdResult->u4CtrlGetStatus);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: u4CtrlSetStatus = 0x%x\n",
			 __func__, EventExtCmdResult->u4CtrlSetStatus);
}

/******************************/
/* EXT_CMD_ID_DRR_CTRL = 0x36 */
/******************************/
/* For station DWRR configuration */
INT32 MtCmdSetVoWDRRCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_VOW_DRR_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	INT32 retry_times = 0;
	EXT_CMD_VOW_DRR_CTRL_T result;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
retry:
	NdisZeroMemory(&result, sizeof(result));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DRR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWDRRCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_DRR_CTRL, TRUE, /* need wait is FALSE */
					  0, TRUE, TRUE, sizeof(result), (char *)&result, MtCmdSetVoWDRRCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
#ifdef RT_BIG_ENDIAN
	param->u4CtrlFieldID = cpu2le32(param->u4CtrlFieldID);
	param->u4ReserveDW = cpu2le32(param->u4ReserveDW);
#endif
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);

	/* command TX result */
	if (ret != NDIS_STATUS_SUCCESS)
		goto error;

	/* FW response event result */
	if (result.ucCtrlStatus == TRUE)
		ret = NDIS_STATUS_SUCCESS;
	else
		ret = NDIS_STATUS_FAILURE;

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);

	if (ret == NDIS_STATUS_FAILURE) {
		retry_times++;
		if (retry_times <= 2)
			goto retry;
	}

	return ret;
}

/***********************************/
/* EXT_CMD_ID_BSSGROUP_CTRL = 0x37 */
/***********************************/
/* for BSS configuration */
INT32 MtCmdSetVoWGroupCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_BSS_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	INT32 retry_times = 0;
	EXT_CMD_BSS_CTRL_T result;
#ifdef RT_BIG_ENDIAN
	P_BW_BSS_TOKEN_SETTING_T pSetting = NULL;
	INT32 i = 0;
#endif
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
retry:
	NdisZeroMemory(&result, sizeof(result));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BSSGROUP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_BSS_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWGroupCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_BSSGROUP_CTRL, TRUE,
					  0, TRUE, TRUE, sizeof(result), (char *)&result, MtCmdSetVoWGroupCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
#ifdef RT_BIG_ENDIAN
	param->u4CtrlFieldID = cpu2le32(param->u4CtrlFieldID);
	param->u4ReserveDW = cpu2le32(param->u4ReserveDW);
	param->u4SingleFieldIDValue = cpu2le32(param->u4SingleFieldIDValue);

	for (i = 0; i < (ARRAY_SIZE(param->arAllBssGroupMultiField)); i++) {
		UINT32 *u32p = NULL;
		pSetting = &param->arAllBssGroupMultiField[i];
		pSetting->u2MinRateToken = cpu2le16(pSetting->u2MinRateToken);
		pSetting->u2MaxRateToken = cpu2le16(pSetting->u2MaxRateToken);
		u32p = (UINT32 *)(&(pSetting->u2MinRateToken) + 2);
		*u32p = cpu2le32(*u32p);
		u32p++;
		*u32p = cpu2le32(*u32p);
		u32p++;
		*u32p = cpu2le32(*u32p);
	}

#endif
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);

	/* printk("\x1b[31m%s: ret %d\x1b[m\n", __FUNCTION__, ret); */
	/* command TX result */
	if (ret != NDIS_STATUS_SUCCESS)
		goto error;

	/* printk("\x1b[31m%s: ucCtrlStatus %d\x1b[m\n", __FUNCTION__, result.ucCtrlStatus); */
	/* FW response event result */
	if (result.ucCtrlStatus == TRUE)
		ret = NDIS_STATUS_SUCCESS;
	else
		ret = NDIS_STATUS_FAILURE;

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);

	if (ret == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(rty = %u)\n", retry_times);
		retry_times++;
		if (retry_times <= 2)
			goto retry;
	}

	return ret;
}

/**************************************/
/* EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38 */
/**************************************/
/* for VOW feature control configuration */
INT32 MtCmdSetVoWFeatureCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_VOW_FEATURE_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	INT32 retry_times = 0;
	EXT_CMD_VOW_FEATURE_CTRL_T result;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
#ifdef RT_BIG_ENDIAN
	UINT16 *u16p = NULL;
	UINT32 *u32p = NULL;
#endif
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
retry:
	NdisZeroMemory(&result, sizeof(result));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_VOW_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_VOW_FEATURE_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWFeatureCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_VOW_FEATURE_CTRL, TRUE,
					  0, TRUE, TRUE, sizeof(result), (char *)&result, MtCmdSetVoWFeatureCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
#ifdef RT_BIG_ENDIAN
	param->u2IfApplyBss_0_to_16_CtrlFlag = cpu2le16(param->u2IfApplyBss_0_to_16_CtrlFlag);
	u16p = &param->u2IfApplyBss_0_to_16_CtrlFlag + 1;
	*u16p = cpu2le16(*u16p);
	param->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = cpu2le16(param->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag);
	param->u2Resreve1Flag = cpu2le16(param->u2Resreve1Flag);
	param->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = cpu2le16(param->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag);
	param->u2Resreve2Flag = cpu2le16(param->u2Resreve2Flag);
	param->u2ResreveBackupFlag[0] = cpu2le32(param->u2ResreveBackupFlag[0]);
	param->u2ResreveBackupFlag[1] = cpu2le32(param->u2ResreveBackupFlag[1]);
	param->u2Bss_0_to_16_CtrlValue = cpu2le16(param->u2Bss_0_to_16_CtrlValue);
	u16p = &param->u2Bss_0_to_16_CtrlValue + 1;
	*u16p = cpu2le16(*u16p);
	param->u2BssCheckTimeToken_0_to_16_CtrlValue = cpu2le16(param->u2BssCheckTimeToken_0_to_16_CtrlValue);
	param->u2Resreve1Value = cpu2le16(param->u2Resreve1Value);
	param->u2BssCheckLengthToken_0_to_16_CtrlValue = cpu2le16(param->u2BssCheckLengthToken_0_to_16_CtrlValue);
	param->u2Resreve2Value = cpu2le16(param->u2Resreve2Value);
	u32p = (UINT32 *)(&(param->u2Resreve2Value) + 1);
	*u32p = cpu2le32(*u32p);
	u32p++;
	*u32p = cpu2le32(*u32p);
#endif
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);

	if (ret == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(rty = %u)\n", retry_times);
		retry_times++;
		if (retry_times <= 2)
			goto retry;
	}

	return ret;
}

/*************************************/
/* EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a */
/*************************************/
/* RX airtime */
INT32 MtCmdSetVoWRxAirtimeCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RX_AT_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_RX_AT_CTRL_T result;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	EXT_CMD_RX_AT_CTRL_T tparam;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_AIRTIME_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_RX_AT_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWRxAirtimeCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_RX_AIRTIME_CTRL, TRUE,
					  0, TRUE, TRUE, sizeof(result), (char *)&result, MtCmdSetVoWRxAirtimeCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
	NdisCopyMemory(&tparam, param,	sizeof(*param));
#ifdef RT_BIG_ENDIAN
	tparam.u4CtrlFieldID = cpu2le16(tparam.u4CtrlFieldID);
	tparam.u4CtrlSubFieldID = cpu2le16(tparam.u4CtrlSubFieldID);
	tparam.u4CtrlSetStatus = cpu2le32(tparam.u4CtrlSetStatus);
	tparam.u4CtrlGetStatus = cpu2le32(tparam.u4CtrlGetStatus);
	tparam.u4ReserveDW[0] = cpu2le32(tparam.u4ReserveDW[0]);
	tparam.u4ReserveDW[1] = cpu2le32(tparam.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[0]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[1]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[2]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[2]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[3]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[3]);
#endif
	MtAndesAppendCmdMsg(msg, (char *)(&tparam), sizeof(tparam));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdGetVoWRxAirtimeCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RX_AT_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	P_EXT_CMD_RX_AT_CTRL_T p_result = param;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	EXT_CMD_RX_AT_CTRL_T tparam;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_AIRTIME_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_RX_AT_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, p_result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWRxAirtimeCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_QUERY, EXT_CMD_ID_RX_AIRTIME_CTRL, TRUE,
					  0, TRUE, TRUE, sizeof(EXT_CMD_RX_AT_CTRL_T), (char *)p_result, MtCmdSetVoWRxAirtimeCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
	NdisCopyMemory(&tparam, param,  sizeof(*param));
#ifdef RT_BIG_ENDIAN
	tparam.u4CtrlFieldID = cpu2le16(tparam.u4CtrlFieldID);
	tparam.u4CtrlSubFieldID = cpu2le16(tparam.u4CtrlSubFieldID);
	tparam.u4CtrlSetStatus = cpu2le32(tparam.u4CtrlSetStatus);
	tparam.u4CtrlGetStatus = cpu2le32(tparam.u4CtrlGetStatus);
	tparam.u4ReserveDW[0] = cpu2le32(tparam.u4ReserveDW[0]);
	tparam.u4ReserveDW[1] = cpu2le32(tparam.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[0]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.u4ReserveDW[1]);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff
		= cpu2le16(tparam.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[0]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[0]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[1]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[1]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[2]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[2]);
	tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[3]
		= cpu2le32(tparam.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4StaAcRxTimer[3]);
#endif
	MtAndesAppendCmdMsg(msg, (char *)(&tparam), sizeof(tparam));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/************************************/
/* EXT_CMD_ID_AT_PROC_MODULE = 0x4b */
/************************************/
/* N9 VOW module */
INT32 MtCmdSetVoWModuleCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_AT_PROC_MODULE_CTRL_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_AT_PROC_MODULE_CTRL_T result;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %zu\n", __func__, sizeof(*param));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_AT_PROC_MODULE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdSetVoWModuleCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_AT_PROC_MODULE, TRUE,
					  0, TRUE, TRUE, sizeof(result), (char *)&result, MtCmdSetVoWModuleCtrlRsp);
#endif /* NEW_MCU_INIT_CMD_API */
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/************************************/
/* EXT_CMD_ID_AT_COUNTER_TEST = 0x?? */
/************************************/
#ifdef RED_SUPPORT
INT32 MtCmdSetRedTxReport(struct _RTMP_ADAPTER *pAd, UCHAR cmd, PUCHAR buffer, UINT16 len)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* no wait and no response */
#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RED_TX_RPT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_RED_TX_RPT, FALSE,
					  0, TRUE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	MtAndesAppendCmdMsg(msg, (char *)buffer, len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif

/************************************/
/* EXT_CMD_ID_AT_COUNTER_TEST = 0x?? */
/************************************/
INT32 MtCmdSetVoWCounterCtrl(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR val)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UCHAR param[2];
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	param[0] = cmd;
	param[1] = val;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "cmd = %d, val = %d)\n",
			 cmd, val);
	/* no wait and no reponse */
#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, 0x4c);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, 0x4c, FALSE,
					  0, TRUE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}


#ifdef DSCP_PRI_SUPPORT
/*    ExT_CID = 0xB4  in MT7915*/
INT32 MtCmdSetDscpPri(struct _RTMP_ADAPTER *pAd, UINT8 bss_idx)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _CMD_SET_DSCP_PRI_T dscp;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif
	UINT32 size = sizeof(dscp);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "bssid(%d)\n", bss_idx);

	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}
#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_DSCP_PRI);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2CR4, EXT_CID, CMD_SET, EXT_CMD_ID_SET_DSCP_PRI,
		FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */

	os_zero_mem(&dscp, size);

	dscp.bss_id = pAd->ApCfg.MBSSID[bss_idx].wdev.bss_info_argument.ucBssIndex;
	dscp.dscp_pri_enable = pAd->ApCfg.MBSSID[bss_idx].dscp_pri_map_enable;
	NdisCopyMemory(dscp.dscpPriMap, pAd->ApCfg.MBSSID[bss_idx].dscp_pri_map, 64);

	MtAndesAppendCmdMsg(msg, (char *)&dscp,
						size);
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /*DSCP_PRI_SUPPORT*/

/*****************************************/
/*    ExT_CID = 0x92 */
/*****************************************/
#ifdef GN_MIXMODE_SUPPORT
INT32 MtCmdSetGNMixModeEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */

	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GN_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_GN_ENABLE,
		FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */

	Val = cpu2le32(en);
	MtAndesAppendCmdMsg(msg, (char *)&Val, sizeof(Val));

	Ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}
#endif /* GN_MIXMODE_SUPPORT */

#ifdef RED_SUPPORT
/*****************************************
 *    ExT_CID = 0x68
 *****************************************/
INT32 MtCmdSetRedEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RED_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_RED_ENABLE,
					  FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	Val = cpu2le32(en);
	MtAndesAppendCmdMsg(msg, (char *)&Val,
						sizeof(Val));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}

/*****************************************
 *    ExT_CID = 0x69
 *****************************************/
INT32 MtCmdSetRedShowSta(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 Num)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RED_SHOW_STA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_RED_SHOW_STA,
					  FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	Val = cpu2le32(Num);
	MtAndesAppendCmdMsg(msg, (char *)&Val,
						sizeof(Val));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}

/*****************************************
 *    ExT_CID = 0x6A
 *****************************************/
INT32 MtCmdSetRedTargetDelay(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 Num)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RED_TARGET_DELAY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_RED_TARGET_DELAY,
					  FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	Val = cpu2le32(Num);
	MtAndesAppendCmdMsg(msg, (char *)&Val,
						sizeof(Val));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}
#endif/*RED_SUPPORT*/
#if defined(A4_CONN) || defined(MBSS_AS_WDS_AP_SUPPORT)

/*****************************************
    ExT_CID = 0x80
*****************************************/
INT32 MtCmdSetA4Enable(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT8 Enable)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MWDS_SUPPORT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_MWDS_SUPPORT,
					  FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	Val = cpu2le32(Enable);
	MtAndesAppendCmdMsg(msg, (char *)&Val, sizeof(Val));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __FUNCTION__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}

#endif

/*****************************************
 *    ExT_CID = 0x75
 *****************************************/
INT32 MtCmdSetCPSEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 Mode)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	UINT32 Val;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif /* NEW_MCU_INIT_CMD_API */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, McuDest);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CP_SUPPORT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, McuDest, EXT_CID, CMD_SET, EXT_CMD_ID_CP_SUPPORT,
					  FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
#endif /* NEW_MCU_INIT_CMD_API */
	Val = cpu2le32(Mode);
	MtAndesAppendCmdMsg(msg, (char *)&Val,
						sizeof(Val));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, Ret);
	return Ret;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}

#ifdef CFG_TDLS_SUPPORT
/*****************************************
 *	ExT_CID = 0x34
 *****************************************/
static VOID cfg_tdls_send_CH_SW_SETUP_callback(struct cmd_msg *msg,
		INT8 *Data, UINT16 Len)
{
	INT32 chsw_fw_resp = 0;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	P_EXT_EVENT_TDLS_SETUP_T prEventExtCmdResult =
		(P_EXT_EVENT_TDLS_SETUP_T)Data;
	chsw_fw_resp = prEventExtCmdResult->ucResultId;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			   "===> CHSW rsp(%d) u4StartTime(%d) u4EndTime(%d) u4TbttTime(%d) u4StayTime(%d) u4RestTime(%d)\n"
				, chsw_fw_resp, le2cpu32(prEventExtCmdResult->u4StartTime),
				le2cpu32(prEventExtCmdResult->u4EndTime), le2cpu32(prEventExtCmdResult->u4TbttTime)
				, le2cpu32(prEventExtCmdResult->u4StayTime), le2cpu32(prEventExtCmdResult->u4RestTime));

	if (chsw_fw_resp == 0) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "FW response!! %ld !!!\n", (jiffies * 1000) / OS_HZ);
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel = TRUE;
	} else {
		BOOLEAN TimerCancelled;
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel = FALSE;
		RTMPCancelTimer(&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.BaseChannelSwitchTimer, &TimerCancelled);
	}
}

INT cfg_tdls_send_CH_SW_SETUP(
	RTMP_ADAPTER *ad,
	UCHAR cmd,
	UCHAR offch_prim,
	UCHAR offch_center,
	UCHAR bw_off,
	UCHAR role,
	UINT16 stay_time,
	UINT32 start_time_tsf,
	UINT16 switch_time,
	UINT16 switch_timeout
)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CFG_TDLS_CHSW_T CmdChanSwitch;
	INT32 ret = 0, i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(ad, sizeof(EXT_CMD_CFG_TDLS_CHSW_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_TDLS_CHSW);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 24);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, cfg_tdls_send_CH_SW_SETUP_callback);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&CmdChanSwitch, sizeof(CmdChanSwitch));
	/* CmdChanSwitch.cmd = cmd; */
	CmdChanSwitch.ucOffBandwidth = bw_off;
	CmdChanSwitch.ucOffPrimaryChannel = offch_prim;
	CmdChanSwitch.ucOffCenterChannelSeg0 = offch_center;
	CmdChanSwitch.ucOffCenterChannelSeg1 = 0;
	CmdChanSwitch.ucRole = role;
	CmdChanSwitch.u4StartTimeTsf = cpu2le32(start_time_tsf);
	CmdChanSwitch.u4SwitchTime = switch_time;
	CmdChanSwitch.u4SwitchTimeout = switch_timeout;
#ifdef RT_BIG_ENDIAN
	CmdChanSwitch.u4SwitchTime = cpu2le32(CmdChanSwitch.u4SwitchTime);
	CmdChanSwitch.u4SwitchTimeout = cpu2le32(CmdChanSwitch.u4SwitchTimeout);
#endif
	CmdChanSwitch.ucBssIndex = BSS0;
	MtAndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "==========================send cmd=============================\n");
			ret = chip_cmd_tx(ad, msg);
			 error:
			 MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					  "%s:(ret = %d)\n", __func__, ret);
			 return ret;
}
#endif /* CFG_TDLS_SUPPORT */

#ifdef CONFIG_HW_HAL_OFFLOAD
/*****************************************
 *	ExT_CID = 0x3D
 *****************************************/
VOID MtCmdATETestResp(struct cmd_msg *msg, char *data, UINT16 len)
{
	/* RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv; */
	/* struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl; */
}

INT32 MtCmdATETest(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_ATE_TEST_MODE_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:struct size %lu\n", __func__, (ULONG)sizeof(*param));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ATE_TEST_MODE);
	/* SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP); */
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET); /* for iBF phase calibration */

	/* Make sure FW command configuration completed for store TX packet in PLE first
	  *    Use aucReserved[1] for uxATEIdx extension feasibility
	  */
	if (param->aucReserved[1] == INIT_CMD_SET_AND_WAIT_RETRY_RSP) {
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
		param->aucReserved[1] = 0;
	}

	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdATETestResp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdCfgOnOff(RTMP_ADAPTER *pAd, UINT8 Type, UINT8 Enable, UINT8 Band)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Type:%d Enable:%d Band:%d\n",
			  Type, Enable, Band);
	ATE_param.ucAteTestModeEn = testmode_en;

	/* =======================================================
	 * Type:
	 *	 0: TSSI
	 *	 1: DPD
	 *	 2: Rate power offset
	 *	 3: Temperature compensation
	 *	 4: Thernal Sensor
	 *	 5: Tx Power Ctrl
	 *	 6: Single SKU
	 *	 7: Power Percentage
	 * ========================================================
	 */
	switch (Type) {
	case EXT_CFG_ONOFF_TSSI:
		ATE_param.ucAteIdx = EXT_ATE_SET_TSSI;
		break;

	case EXT_CFG_ONOFF_DPD:
		ATE_param.ucAteIdx = EXT_ATE_SET_DPD;
		break;

	case EXT_CFG_ONOFF_RATE_POWER_OFFSET:
		ATE_param.ucAteIdx = EXT_ATE_SET_RATE_POWER_OFFSET;
		break;

	case EXT_CFG_ONOFF_TEMP_COMP:
		ATE_param.ucAteIdx = EXT_ATE_SET_THERNAL_COMPENSATION;
		break;

	case EXT_CFG_ONOFF_THERMAL_SENSOR:
		ATE_param.ucAteIdx = EXT_ATE_CFG_THERMAL_ONOFF;
		break;

	case EXT_CFG_ONOFF_TXPOWER_CTRL:
		ATE_param.ucAteIdx = EXT_ATE_SET_TX_POWER_CONTROL_ALL_RF;
		break;

	case EXT_CFG_ONOFF_SINGLE_SKU:
		ATE_param.ucAteIdx = EXT_ATE_SET_SINGLE_SKU;
		break;

	case EXT_CFG_ONOFF_POWER_PERCENTAGE:
		ATE_param.ucAteIdx = EXT_ATE_SET_POWER_PERCENTAGE;
		break;

	default:
		break;
	}

	if (Type == 5) {
		ATE_param.Data.u4Data = Enable;
#ifdef RT_BIG_ENDIAN
		ATE_param.Data.u4Data = cpu2le32(ATE_param.Data.u4Data);
#endif
	} else {
		ATE_param.Data.rCfgOnOff.ucEnable = Enable;
		ATE_param.Data.rCfgOnOff.ucBand = Band;
	}

	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdSetAntennaPort(RTMP_ADAPTER *pAd, UINT8 RfModeMask,
						  UINT8 RfPortMask, UINT8 AntPortMask)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "RfModeMask:%d RfPortMask:%d AntPortMask:%d\n",
			  RfModeMask, RfPortMask, AntPortMask);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_ANTENNA_PORT;
	ATE_param.Data.rCfgRfAntPortSetting.ucRfModeMask = RfModeMask;
	ATE_param.Data.rCfgRfAntPortSetting.ucRfPortMask = RfPortMask;
	ATE_param.Data.rCfgRfAntPortSetting.ucAntPortMask = AntPortMask;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdATESetSlotTime(RTMP_ADAPTER *pAd, UINT8 SlotTime,
						  UINT8 SifsTime, UINT8 RifsTime, UINT16 EifsTime, UCHAR BandIdx)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_SLOT_TIME;
	ATE_param.Data.rSlotTimeSet.u2Eifs = cpu2le16(EifsTime);
	ATE_param.Data.rSlotTimeSet.ucRifs = RifsTime;
	ATE_param.Data.rSlotTimeSet.ucSifs = SifsTime;
	ATE_param.Data.rSlotTimeSet.ucSlotTime = SlotTime;
	ATE_param.Data.rSlotTimeSet.ucBandNum = BandIdx;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdATESetPowerDropLevel(RTMP_ADAPTER *pAd, UINT8 PowerDropLevel, UCHAR BandIdx)
{
	INT32 ret = 0;
	INT8 TxPowerDrop = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif

	/* update Tx Power Drop value according to Power Drop Level */
	if ((PowerDropLevel > 90) && (PowerDropLevel < 100))
		TxPowerDrop = 0;
	else if ((PowerDropLevel > 60) && (PowerDropLevel <= 90))  /* reduce Pwr for 1 dB. */
		TxPowerDrop = 1;
	else if ((PowerDropLevel > 30) && (PowerDropLevel <= 60))  /* reduce Pwr for 3 dB. */
		TxPowerDrop = 3;
	else if ((PowerDropLevel > 15) && (PowerDropLevel <= 30))  /* reduce Pwr for 6 dB. */
		TxPowerDrop = 6;
	else if ((PowerDropLevel >  9) && (PowerDropLevel <= 15))  /* reduce Pwr for 9 dB. */
		TxPowerDrop = 9;
	else if ((PowerDropLevel >  0) && (PowerDropLevel <=  9))  /* reduce Pwr for 12 dB. */
		TxPowerDrop = 12;

	os_zero_mem(&ATE_param, sizeof(ATE_param));
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_POWER_PERCENTAGE_LEVEL;
	ATE_param.Data.rPowerLevelSet.cPowerDropLevel = TxPowerDrop;
	ATE_param.Data.rPowerLevelSet.ucBand = BandIdx;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdRxFilterPktLen(RTMP_ADAPTER *pAd, UINT8 Enable,
						  UINT8 Band, UINT32 RxPktLen)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Enable:%d Band:%d RxPktLen:%d\n",
			  Enable, Band, RxPktLen);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_RX_FILTER_PKT_LEN;
	ATE_param.Data.rRxFilterPktLen.ucEnable = Enable;
	ATE_param.Data.rRxFilterPktLen.ucBand = Band;
	ATE_param.Data.rRxFilterPktLen.u4RxPktLen = cpu2le32(RxPktLen);
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdSetFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset, UINT8 BandIdx)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx:%d, FreqOffset:%d\n", BandIdx, FreqOffset);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_FREQ_OFFSET;
	ATE_param.Data.rFreqOffset.ucBandIdx = BandIdx;
	ATE_param.Data.rFreqOffset.u4FreqOffset = cpu2le32(FreqOffset);
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

static VOID MtCmdGetFreqOffsetRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EVENT_EXT_GET_FREQOFFSET_T *Result = (EVENT_EXT_GET_FREQOFFSET_T *)Data;
	Result->u4FreqOffset = le2cpu32(Result->u4FreqOffset);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk,
				&Result->u4FreqOffset, sizeof(Result->u4FreqOffset));
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "FreqOffset:%d\n", Result->u4FreqOffset);
}

INT32 MtCmdGetFreqOffset(RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT32 *pFreqOffsetResult)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	INT32 ret = 0;
	UINT8 testmode_en = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(ATE_param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ATE_TEST_MODE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_EXT_GET_FREQOFFSET_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pFreqOffsetResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetFreqOffsetRsp);
	MtAndesInitCmdMsg(msg, attr);
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	NdisZeroMemory(&ATE_param, sizeof(ATE_param));
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_GET_FREQ_OFFSET;
	ATE_param.Data.rFreqOffset.ucBandIdx = BandIdx;
#ifdef RT_BIG_ENDIAN
	ATE_param.Data.rFreqOffset.u4FreqOffset  = cpu2le32(ATE_param.Data.rFreqOffset.u4FreqOffset);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&ATE_param, sizeof(ATE_param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

static VOID MtCmdGetCfgStatRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_ATE_TEST_MODE_T *Result = (EXT_EVENT_ATE_TEST_MODE_T *)Data;
	GET_TSSI_STATUS_T *TSSI_Status = (GET_TSSI_STATUS_T *)&Result->aucAteResult[0];
	GET_DPD_STATUS_T *DPD_Status = (GET_DPD_STATUS_T *)&Result->aucAteResult[0];
	GET_THERMO_COMP_STATUS_T *THER_Status = (GET_THERMO_COMP_STATUS_T *)&Result->aucAteResult[0];

	switch (Result->ucAteIdx) {
	case EXT_ATE_GET_TSSI:
		os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &TSSI_Status->ucEnable, sizeof(TSSI_Status->ucEnable));
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TSSI Enable:%d Band:%d\n", TSSI_Status->ucEnable,
				 TSSI_Status->ucBand);
		break;

	case EXT_ATE_GET_DPD:
		os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &DPD_Status->ucEnable, sizeof(DPD_Status->ucEnable));
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DPD Enable:%d Band:%d\n", DPD_Status->ucEnable,
				 DPD_Status->ucBand);
		break;

	case EXT_ATE_GET_THERNAL_COMPENSATION:
		os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &THER_Status->ucEnable, sizeof(THER_Status->ucEnable));
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "THER Enable:%d\n", THER_Status->ucEnable);
		break;

	default:
		break;
	}
}

INT32 MtCmdGetCfgOnOff(RTMP_ADAPTER *pAd, UINT32 Type, UINT8 Band, UINT32 *Status)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	INT32 ret = 0;
	UINT8 testmode_en = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(ATE_param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ATE_TEST_MODE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, Status);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetCfgStatRsp);
	MtAndesInitCmdMsg(msg, attr);
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	NdisZeroMemory(&ATE_param, sizeof(ATE_param));
	ATE_param.ucAteTestModeEn = testmode_en;

	/* Type: 0: TSSI  1: DPD  2: Rate power offset  3: Temperature compensation */
	switch (Type) {
	case 0:
		ATE_param.ucAteIdx = EXT_ATE_GET_TSSI;
		break;

	case 1:
		ATE_param.ucAteIdx = EXT_ATE_GET_DPD;
		break;

	case 2:
		ATE_param.ucAteIdx = EXT_ATE_GET_RATE_POWER_OFFSET;
		break;

	case 3:
		ATE_param.ucAteIdx = EXT_ATE_GET_THERNAL_COMPENSATION;
		break;

	default:
		break;
	}

	ATE_param.Data.u4Data = Band;
#ifdef RT_BIG_ENDIAN
	ATE_param.Data.u4Data = cpu2le32(ATE_param.Data.u4Data);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&ATE_param, sizeof(ATE_param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
INT32 MtCmdSetPhyCounter(RTMP_ADAPTER *pAd, UINT32 Control, UINT8 band_idx)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 1;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Control:%d\n", Control);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_PHY_COUNT;
	ATE_param.Data.rPhyStatusCnt.ucEnable = Control;
	ATE_param.Data.rPhyStatusCnt.ucBand = band_idx;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdSetRxvIndex(RTMP_ADAPTER *pAd, UINT8 Group_1,
					   UINT8 Group_2, UINT8 band_idx)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Group_1:%d Group_2:%d Band:%d\n",
			  Group_1, Group_2, band_idx);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_RXV_INDEX;
	ATE_param.Data.rSetRxvIdx.ucValue1 = Group_1;
	ATE_param.Data.rSetRxvIdx.ucValue2 = Group_2;
	ATE_param.Data.rSetRxvIdx.ucDbdcIdx = band_idx;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}

INT32 MtCmdSetFAGCPath(RTMP_ADAPTER *pAd, UINT8 Path, UINT8 band_idx)
{
	INT32 ret = 0;
	struct _EXT_CMD_ATE_TEST_MODE_T ATE_param;
	UINT8 testmode_en = 0;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		testmode_en = 1;

#endif
	os_zero_mem(&ATE_param, sizeof(ATE_param));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Path:%d Band:%d\n", Path, band_idx);
	ATE_param.ucAteTestModeEn = testmode_en;
	ATE_param.ucAteIdx = EXT_ATE_SET_FAGC_PATH;
	ATE_param.Data.rSetFagcRssiPath.ucValue = Path;
	ATE_param.Data.rSetFagcRssiPath.ucDbdcIdx = band_idx;
	ret = MtCmdATETest(pAd, &ATE_param);
	return ret;
}
#endif /* CONFIG_HW_HAL_OFFLOAD */

INT32 MtCmdClockSwitchDisable(RTMP_ADAPTER *pAd, UINT8 isDisable)
{
	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _CMD_MCU_CLK_SWITCH_DISABLE_T clockDisable;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_MCU_CLK_SWITCH_DISABLE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "isDisable: %d\n", isDisable);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CLOCK_SWITCH_DISABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&clockDisable, sizeof(clockDisable));
	clockDisable.disable = isDisable;
	MtAndesAppendCmdMsg(msg, (char *)&clockDisable, sizeof(clockDisable));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 *    ExT_CID = 0x3e
 *****************************************/
INT32 MtCmdUpdateProtect(struct _RTMP_ADAPTER *pAd,
						 struct _EXT_CMD_UPDATE_PROTECT_T *param)
{
	struct cmd_msg *msg = NULL;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PROTECT_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:
	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "(ret = %d)\n", ret);

	return ret;
}

/*****************************************
 *    ExT_CID = 0x3f
 *****************************************/
INT32 MtCmdSetRdg(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RDG_CTRL_T *param)
{
	struct cmd_msg *msg = NULL;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDG_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	param->u4TxOP = cpu2le32(param->u4TxOP);
#endif
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(ret = %d)\n", ret);
	}

	return ret;
}

/*****************************************
 *	ExT_CID = 0x42
 *****************************************/
INT32 MtCmdSetSnifferMode(struct _RTMP_ADAPTER *pAd,
						  struct _EXT_CMD_SNIFFER_MODE_T *param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(*param));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SNIFFER_MODE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)param, sizeof(*param));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d) sniffer_mode:%d\n",
			  ret, param->ucSnifferEn);
	return ret;
}

/*****************************************
 *	ExT_CID = 0x57
 *****************************************/
static VOID CmdMemDumpRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_CMD_EVENT_DUMP_MEM_T PktBufEvt = (P_EXT_CMD_EVENT_DUMP_MEM_T)Data;
	MEM_DUMP_DATA_T *MemDumpData = NULL;
	UINT32 datasz = sizeof(PktBufEvt->ucData);
	MemDumpData = (MEM_DUMP_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;
	os_move_mem(&MemDumpData->pValue[0], &PktBufEvt->ucData[0], datasz);
}

VOID MtCmdMemDump(RTMP_ADAPTER *pAd, UINT32 Addr, PUINT8 pData)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_EVENT_DUMP_MEM_T *CmdMemDump = NULL;
	MEM_DUMP_DATA_T MemDumpData;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&MemDumpData, sizeof(MemDumpData));
	MemDumpData.pValue = pData;
	os_alloc_mem(pAd, (UCHAR **)&CmdMemDump, sizeof(EXT_CMD_EVENT_DUMP_MEM_T));

	if (!CmdMemDump) {
		ret = NDIS_STATUS_RESOURCES;
		goto done;
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EVENT_DUMP_MEM_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto done;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DUMP_MEM);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_CMD_EVENT_DUMP_MEM_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &MemDumpData);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdMemDumpRsp);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(CmdMemDump, sizeof(EXT_CMD_EVENT_DUMP_MEM_T));
	CmdMemDump->u4MemAddr = cpu2le32(Addr);
	MtAndesAppendCmdMsg(msg, (char *)CmdMemDump, sizeof(EXT_CMD_EVENT_DUMP_MEM_T));
	ret = chip_cmd_tx(pAd, msg);
	goto done;

done:

	if (CmdMemDump)
		os_free_mem(CmdMemDump);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
}
#ifdef TXRX_STAT_SUPPORT
INT32 MtCmdGetPerStaTxStat(RTMP_ADAPTER *pAd, UINT8 *ucEntryBitmap, UINT8 ucEntryCount)
{

	struct cmd_msg *msg;
	INT32 Ret = 0;
	EXT_CMD_GET_STA_TX_STAT_T StaTxStatCmd;
/*	EXT_EVENT_STA_TX_STAT_RESULT_T CmdStaTxStatResult; */
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(StaTxStatCmd));
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_STA_TX_STAT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&StaTxStatCmd, sizeof(StaTxStatCmd));
	if (ucEntryBitmap)
		NdisCopyMemory(StaTxStatCmd.ucEntryBitmap, ucEntryBitmap, 16);	/*16*8 = 128 client entry bitmap*/
	else
		NdisFillMemory(StaTxStatCmd.ucEntryBitmap, 16, 0xff);
	StaTxStatCmd.ucEntryCount = ucEntryCount;
	MtAndesAppendCmdMsg(msg, (char *)&StaTxStatCmd, sizeof(StaTxStatCmd));
	Ret  = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __FUNCTION__, Ret);
	return Ret;
}

#endif

INT32 MtCmdGetAllStaStats(struct _RTMP_ADAPTER *pAd, UINT8 subevent_type)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_ALL_STA_STAT_T rAllStaStatCmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&rAllStaStatCmd, sizeof(rAllStaStatCmd));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(rAllStaStatCmd));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_ALL_STA_STATS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	rAllStaStatCmd.ucEventType = subevent_type;
	MtAndesAppendCmdMsg(msg, (char *)&rAllStaStatCmd, sizeof(rAllStaStatCmd));
	ret = chip_cmd_tx(pAd, msg);
error:

	if (ret)
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s:(ret = %d)\n", __func__, ret);

	return ret;
}

/*****************************************
 *	CID
 *****************************************/

/*****************************************
 *	CID = 0x01
 *****************************************/

/*****************************************
 *	CID = 0x02
 *****************************************/

/*****************************************
 *	CID = 0x3
 *****************************************/

/*****************************************
 *	CID = 0x05
 *****************************************/

/*****************************************
 *	CID = 0x07
 *****************************************/

/*****************************************
 *	CID = 0x10
 *****************************************/

/*****************************************
 *	CID = 0x20
 *****************************************/
#if !defined(COMPOS_WIN)
INT32 MtCmdHIFLoopBackTest(
	IN  RTMP_ADAPTER *pAd, BOOLEAN IsEnable, UINT8 RxQ
)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	CMD_HIF_LOOPBACK CmdMsg;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&CmdMsg, sizeof(CmdMsg));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdMsg));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_HIF_LOOPBACK);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	CmdMsg.Loopback_Enable = IsEnable;
	CmdMsg.DestinationQid = RxQ;
#ifdef RT_BIG_ENDIAN
	CmdMsg.Loopback_Enable = cpu2le16(CmdMsg.Loopback_Enable);
	CmdMsg.DestinationQid = cpu2le16(CmdMsg.DestinationQid);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&CmdMsg, sizeof(CmdMsg));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif
/*****************************************
 *	CID = 0xC2
 *****************************************/

/*****************************************
 *	CID = 0xED
 *****************************************/

/*****************************************
 *	CID = 0xEE
 *****************************************/

/*****************************************
 *	CID = 0xEF
 *****************************************/

/******************************************
 *	ROM CODE CMD
 *******************************************/

static VOID CmdReStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;
	Status = *Data;

	switch (Status) {
	case WIFI_FW_DOWNLOAD_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "WiFI FW Download Success\n");
		break;

	case WIFI_FW_DOWNLOAD_INVALID_PARAM:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Invalid Parameter\n");
		break;

	case WIFI_FW_DOWNLOAD_INVALID_CRC:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Invalid CRC\n");
		break;

	case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Decryption Fail\n");
		break;

	case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Unknown CMD\n");
		break;

	case WIFI_FW_DOWNLOAD_TIMEOUT:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Timeout\n");
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Unknow Status(%d)\n", Status);
		break;
	}
}

static INT32 MtCmdRestartDLReqWithRsp(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(ad, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_RESTART_DL_REQ);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 7000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdReStartDLRsp);
	MtAndesInitCmdMsg(msg, attr);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}


static INT32 MtCmdRestartDLReqNoRsp(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct {
		UINT_8 ucPowerMode;
		UINT_8 aucReserved[3];
	} req = {
		.ucPowerMode = 1,
	};

	msg = MtAndesAllocCmdMsg(ad, sizeof(req));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, CMD_ID_NIC_POWER_CTRL);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&req, sizeof(req));
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdRestartDLReq(RTMP_ADAPTER *ad)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (IS_ASIC_CAP(ad, fASIC_CAP_FW_RESTART_POLLING_MODE)) {
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			return MtUniCmdRestartDLReqNoRsp(ad);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return MtCmdRestartDLReqNoRsp(ad);
	} else {
		return MtCmdRestartDLReqWithRsp(ad);
	}
}

static VOID CmdAddrellLenRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;
	Status = *Data;

	switch (Status) {
	case TARGET_ADDRESS_LEN_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s: Request target address and length success\n", __func__);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Unknown Status(%d)\n", Status);
		break;
	}
}

INT32 MtCmdAddressLenReq(RTMP_ADAPTER *ad, UINT32 address,
						 UINT32 len, UINT32 data_mode)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;
	struct _CMD_ATTRIBUTE attr = {0};
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Start address = 0x%x, DL length = %d, Data mode = 0x%x\n",
			  address, len, data_mode);
	msg = MtAndesAllocCmdMsg(ad, 12);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, (address == cap->rom_patch_offset) ?
					  MT_PATCH_START_REQ : MT_TARGET_ADDRESS_LEN_REQ);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdAddrellLenRsp);
	MtAndesInitCmdMsg(msg, attr);
	/* start address */
	value = cpu2le32(address);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	/* dl length */
	value = cpu2le32(len);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	/* data mode */
	value = cpu2le32(data_mode);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdFwScatter(RTMP_ADAPTER *ad, UINT8 *dl_payload,
					 UINT32 dl_len, UINT32 count)
{
	struct cmd_msg *msg;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(ad, dl_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_FW_SCATTER);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)dl_payload, dl_len);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(scatter = %d, ret = %d)\n", __func__, count, ret);
	return ret;
}

static VOID CmdPatchSemRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	Ctl->fwdl_ctrl.sem_status = *Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Patch SEM Status = %d\n", Ctl->fwdl_ctrl.sem_status);
}

INT32 MtCmdPatchSemGet(RTMP_ADAPTER *ad, UINT32 Semaphore)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(ad, 4);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_PATCH_SEM_CONTROL);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdPatchSemRsp);
	MtAndesInitCmdMsg(msg, attr);
	/* Semaphore */
	value = cpu2le32(Semaphore);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID CmdPatchFinishRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;
	Status = *Data;

	switch (Status) {
	case WIFI_FW_DOWNLOAD_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "WiFI ROM Patch Download Success\n");
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi ROM Patch Fail (%d)\n", Status);
		break;
	}
}

INT32 MtCmdPatchFinishReq(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_PATCH_FINISH_T CmdPatchFinish = {0};
	MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	msg = MtAndesAllocCmdMsg(ad, sizeof(CMD_PATCH_FINISH_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_PATCH_FINISH_REQ);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdPatchFinishRsp);
	MtAndesInitCmdMsg(msg, attr);
	/* Don't check CRC of Patch */
	CmdPatchFinish.ucCheckCrc = 0;
	MtAndesAppendCmdMsg(msg, (char *)&CmdPatchFinish, sizeof(CMD_PATCH_FINISH_T));
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

static VOID CmdStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;
	Status = *Data;

	switch (Status) {
	case WIFI_FW_DOWNLOAD_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "WiFI FW Download Success\n");
		break;

	case WIFI_FW_DOWNLOAD_INVALID_PARAM:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Invalid Parameter\n");
		break;

	case WIFI_FW_DOWNLOAD_INVALID_CRC:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Invalid CRC\n");
		break;

	case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Decryption Fail\n");
		break;

	case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Unknown CMD\n");
		break;

	case WIFI_FW_DOWNLOAD_TIMEOUT:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "WiFi FW Download Timeout\n");
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Unknow Status(%d)\n", Status);
		break;
	}
}

INT32 MtCmdFwStartReq(RTMP_ADAPTER *ad, UINT32 override, UINT32 address)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "override = 0x%x, address = 0x%x\n", override, address);
	msg = MtAndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_FW_START_REQ);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdStartDLRsp);
	MtAndesInitCmdMsg(msg, attr);
	/* override */
	value = cpu2le32(override);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	/* entry point address */
	value = cpu2le32(address);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdFwDecompressStart(
	RTMP_ADAPTER *ad,
	P_INIT_CMD_WIFI_START_WITH_DECOMPRESSION decompress_info)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value, i;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(ad, sizeof(INIT_CMD_WIFI_START_WITH_DECOMPRESSION));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_WIFI_DECOMPRESSION_START);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdStartDLRsp);
	MtAndesInitCmdMsg(msg, attr);
	/* override */
	value = cpu2le32(decompress_info->u4Override);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	/* entry point address */
	value = cpu2le32(decompress_info->u4Address);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);

	value = cpu2le32(decompress_info->u4DecompressTmpAddress);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);

	value = cpu2le32(decompress_info->u4BlockSize);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);

	value = cpu2le32(decompress_info->u4RegionNumber);
	MtAndesAppendCmdMsg(msg, (char *)&value, 4);

	for (i = 0; i < decompress_info->u4RegionNumber; i++) {
		value = cpu2le32(decompress_info->aucDecompRegion[i].u4RegionAddress);
		MtAndesAppendCmdMsg(msg, (char *)&value, 4);

		value = cpu2le32(decompress_info->aucDecompRegion[i].u4Regionlength);
		MtAndesAppendCmdMsg(msg, (char *)&value, 4);

		value = cpu2le32(decompress_info->aucDecompRegion[i].u4RegionCRC);
		MtAndesAppendCmdMsg(msg, (char *)&value, 4);
	}

	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef DBDC_MODE
/*****************************************
 *	ExT_CID = 0x45
 *****************************************/
static VOID MtCmdGetDbdcCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_DBDC_CTRL_T *pDbdcCmdResult = (EXT_EVENT_DBDC_CTRL_T *)Data;
	BCTRL_INFO_T *pDbdcRspResult = (BCTRL_INFO_T *)msg->attr.rsp.wb_buf_in_calbk;
	INT i;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Enable=%d,TotalNum=%d\n", pDbdcCmdResult->ucDbdcEnable,
			  pDbdcCmdResult->ucTotalNum);
	pDbdcRspResult->TotalNum = pDbdcCmdResult->ucTotalNum;
	pDbdcRspResult->DBDCEnable = pDbdcCmdResult->ucDbdcEnable;

	for (i = 0; i < pDbdcCmdResult->ucTotalNum; i++) {
		pDbdcRspResult->BctrlEntries[i].Index =
			pDbdcCmdResult->aBCtrlEntry[i].ucIndex;
		pDbdcRspResult->BctrlEntries[i].Type =
			pDbdcCmdResult->aBCtrlEntry[i].ucType;
		pDbdcRspResult->BctrlEntries[i].BandIdx =
			pDbdcCmdResult->aBCtrlEntry[i].ucBandIdx;
	}
}

INT32 MtCmdGetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pDbdcInfo)
{
	struct cmd_msg *msg;
	EXT_CMD_DBDC_CTRL_T DbdcCtrlCmd;
	INT32 ret = 0;
	INT32 Len = 0, i;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&DbdcCtrlCmd, sizeof(EXT_CMD_DBDC_CTRL_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_DBDC_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = 4 + sizeof(BAND_CTRL_ENTRY_T) * pDbdcInfo->TotalNum;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DBDC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pDbdcInfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetDbdcCtrlRsp);
	MtAndesInitCmdMsg(msg, attr);
	DbdcCtrlCmd.ucDbdcEnable = pDbdcInfo->DBDCEnable;
	DbdcCtrlCmd.ucTotalNum = pDbdcInfo->TotalNum;

	for (i = 0; i < pDbdcInfo->TotalNum; i++) {
		DbdcCtrlCmd.aBCtrlEntry[i].ucType = pDbdcInfo->BctrlEntries[i].Type;
		DbdcCtrlCmd.aBCtrlEntry[i].ucIndex = pDbdcInfo->BctrlEntries[i].Index;
	}

	MtAndesAppendCmdMsg(msg, (char *)&DbdcCtrlCmd, sizeof(EXT_CMD_DBDC_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pBandInfo)
{
	struct cmd_msg *msg;
	EXT_CMD_DBDC_CTRL_T DbdcCtrlCmd;
	INT32 ret = 0;
	INT32 i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&DbdcCtrlCmd, sizeof(EXT_CMD_DBDC_CTRL_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_DBDC_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DBDC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	DbdcCtrlCmd.ucDbdcEnable = pBandInfo->DBDCEnable;
	DbdcCtrlCmd.ucTotalNum = pBandInfo->TotalNum;

	for (i = 0; i < pBandInfo->TotalNum; i++) {
		DbdcCtrlCmd.aBCtrlEntry[i].ucBandIdx = pBandInfo->BctrlEntries[i].BandIdx;
		DbdcCtrlCmd.aBCtrlEntry[i].ucType = pBandInfo->BctrlEntries[i].Type;
		DbdcCtrlCmd.aBCtrlEntry[i].ucIndex = pBandInfo->BctrlEntries[i].Index;
	}

	MtAndesAppendCmdMsg(msg, (char *)&DbdcCtrlCmd, sizeof(EXT_CMD_DBDC_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#endif /*DBDC_MODE*/

/*****************************************
 *	ExT_CID = 0x3C
 *****************************************/

static VOID MtCmdGetChBusyCntRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_MAC_INFO_T *pChBusyCntCmdResult = (EXT_EVENT_MAC_INFO_T *)Data;
	UINT32 *pChBusyCnt = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;
	*pChBusyCnt =
		le2cpu32(pChBusyCntCmdResult->aucMacInfoResult.ChBusyCntResult.u4ChBusyCnt);
}

INT32 MtCmdGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR ChIdx, UINT32 *pChBusyCnt)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_MAC_INFO_T MacInfoCmd;
	EXTRA_ARG_CH_BUSY_CNT_T  *pChBusyCntArg =
		&MacInfoCmd.aucExtraArgument.ChBusyCntArg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	os_zero_mem(&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_MAC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = 4 + sizeof(GET_CH_BUSY_CNT_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pChBusyCnt);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetChBusyCntRsp);
	MtAndesInitCmdMsg(msg, attr);
	MacInfoCmd.u2MacInfoId = cpu2le16(MAC_INFO_TYPE_CHANNEL_BUSY_CNT);
	pChBusyCntArg->ucBand = ChIdx;
	MtAndesAppendCmdMsg(msg, (char *)&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdGetTsfTimeRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_MAC_INFO_T *pTsfCmdResult = (EXT_EVENT_MAC_INFO_T *)Data;
	TSF_RESULT_T *pTsfResult = (TSF_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;
	pTsfResult->u4TsfBit0_31 =
		le2cpu32(pTsfCmdResult->aucMacInfoResult.TsfResult.u4TsfBit0_31);
	pTsfResult->u4TsfBit63_32 =
		le2cpu32(pTsfCmdResult->aucMacInfoResult.TsfResult.u4TsfBit63_32);
}

INT32 MtCmdGetTsfTime(RTMP_ADAPTER *pAd, UCHAR HwBssidIdx, TSF_RESULT_T *pTsfResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_MAC_INFO_T MacInfoCmd;
	EXTRA_ARG_TSF_T  *pTsfArg = &MacInfoCmd.aucExtraArgument.TsfArg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	os_zero_mem(&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_MAC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = 4 + sizeof(TSF_RESULT_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pTsfResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetTsfTimeRsp);
	MtAndesInitCmdMsg(msg, attr);
	MacInfoCmd.u2MacInfoId = cpu2le16(MAC_INFO_TYPE_TSF);
	pTsfArg->ucHwBssidIndex = HwBssidIdx;
	MtAndesAppendCmdMsg(msg, (char *)&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdGetPartialMibInfoCntRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_MAC_INFO_T *pPartialMibInfoCntCmdResult = (EXT_EVENT_MAC_INFO_T *)Data;
	MT_PARTIAL_MIB_INFO_CNT_CTRL_T *pPartialMibInfoCntRspResult = (MT_PARTIAL_MIB_INFO_CNT_CTRL_T *)msg->attr.rsp.wb_buf_in_calbk;
	MIB_INFO_CNT_PARAM_T *pMibInfoParm;

	pPartialMibInfoCntRspResult->ucBand =
		pPartialMibInfoCntCmdResult->aucMacInfoResult.PartialMibInfoCntResult.ucBand;

	pMibInfoParm = &pPartialMibInfoCntCmdResult->aucMacInfoResult.PartialMibInfoCntResult.rMibInfoParam;

	pPartialMibInfoCntRspResult->rMibInfoParam.u4RxFcsErrCnt
		= le2cpu32(pMibInfoParm->u4RxFcsErrCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4RxFifoOverflowCnt
		= le2cpu32(pMibInfoParm->u4RxFifoOverflowCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4RxMpduCnt
		= le2cpu32(pMibInfoParm->u4RxMpduCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4RxChannelIdleCnt
		= le2cpu32(pMibInfoParm->u4RxChannelIdleCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4CcaNavTxTimeCnt
		= le2cpu32(pMibInfoParm->u4CcaNavTxTimeCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4MdrdyCnt
		= le2cpu32(pMibInfoParm->u4MdrdyCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4SCcaCnt
		= le2cpu32(pMibInfoParm->u4SCcaCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4PEdCnt
		= le2cpu32(pMibInfoParm->u4PEdCnt);
	pPartialMibInfoCntRspResult->rMibInfoParam.u4RxTotalByteCnt
		= le2cpu32(pMibInfoParm->u4RxTotalByteCnt);
}

INT32 MtCmdGetPartialMibInfoCnt(RTMP_ADAPTER *pAd, UCHAR ChIdx, MT_PARTIAL_MIB_INFO_CNT_CTRL_T *pPartialMibInfoCtrl)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_MAC_INFO_T MacInfoCmd;
	EXTRA_ARG_PARTIAL_MIB_INFO_CNT_T *pPartialMibInfoCntArg = &MacInfoCmd.aucExtraArgument.PartialMibInfoCntArg;
	INT32 ret = 0;
	INT32 Len = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_MAC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = 8 + sizeof(MIB_INFO_CNT_PARAM_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pPartialMibInfoCtrl);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetPartialMibInfoCntRsp);
	MtAndesInitCmdMsg(msg, attr);
	MacInfoCmd.u2MacInfoId = cpu2le16(MAC_INFO_TYPE_MIB);
	pPartialMibInfoCntArg->ucBand = ChIdx;

	MtAndesAppendCmdMsg(msg, (char *)&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdGetEdcaRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_MAC_INFO_T *pEdcaCmdResult = (EXT_EVENT_MAC_INFO_T *)Data;
	MT_EDCA_CTRL_T *pEdcaRspResult =
		(MT_EDCA_CTRL_T *)msg->attr.rsp.wb_buf_in_calbk;
	UINT32 i = 0;
	TX_AC_PARAM_T *pAcParm;
	pEdcaRspResult->ucTotalNum =
		pEdcaCmdResult->aucMacInfoResult.EdcaResult.ucTotalNum;

	for (i = 0; i < pEdcaRspResult->ucTotalNum; i++) {
		pAcParm = &pEdcaCmdResult->aucMacInfoResult.EdcaResult.rAcParam[i];
		pEdcaRspResult->rAcParam[i].u2Txop = le2cpu16(pAcParm->u2Txop);
		pEdcaRspResult->rAcParam[i].u2WinMax = le2cpu16(pAcParm->u2WinMax);
		pEdcaRspResult->rAcParam[i].ucAcNum = pAcParm->ucAcNum;
		pEdcaRspResult->rAcParam[i].ucAifs = pAcParm->ucAifs;
		pEdcaRspResult->rAcParam[i].ucWinMin = pAcParm->ucWinMin;
	}
}

INT32 MtCmdGetEdca(RTMP_ADAPTER *pAd, MT_EDCA_CTRL_T *pEdcaCtrl)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_MAC_INFO_T MacInfoCmd;
	EXTRA_ARG_EDCA_T *pEdcaArg = &MacInfoCmd.aucExtraArgument.EdcaArg;
	INT32 ret = 0;
	INT32 Len = 0, i;
	struct _CMD_ATTRIBUTE attr = {0};
	os_zero_mem(&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_MAC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = 8 + sizeof(TX_AC_PARAM_T) * pEdcaCtrl->ucTotalNum;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pEdcaCtrl);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetEdcaRsp);
	MtAndesInitCmdMsg(msg, attr);
	MacInfoCmd.u2MacInfoId = cpu2le16(MAC_INFO_TYPE_EDCA);
	pEdcaArg->ucTotalAcNum = pEdcaCtrl->ucTotalNum;

	for (i = 0; i < pEdcaCtrl->ucTotalNum; i++)
		pEdcaArg->au4AcIndex[i] = cpu2le32(pEdcaCtrl->rAcParam[i].ucAcNum);

	MtAndesAppendCmdMsg(msg, (char *)&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdGetWifiInterruptCntRsp(struct cmd_msg *msg,
										char *Data, UINT16 Len)
{
	INT32 i;
	EXT_EVENT_MAC_INFO_T *pWifiInterruptCntCmdResult =
		(EXT_EVENT_MAC_INFO_T *)Data;
	UINT32 *pWifiInterruptCnt = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;
	UINT32 *pResultWifiInterruptCounter =
		pWifiInterruptCntCmdResult->aucMacInfoResult.WifiIntCntResult.u4WifiInterruptCounter;

	if (pWifiInterruptCntCmdResult->u2MacInfoId ==
		cpu2le16(MAC_INFO_TYPE_WIFI_INT_CNT)) {
		for (i = 0; i < pWifiInterruptCntCmdResult->aucMacInfoResult.WifiIntCntResult.ucWifiInterruptNum; i++) {
			*pWifiInterruptCnt = le2cpu32(*pResultWifiInterruptCounter);
			pWifiInterruptCnt++;
			pResultWifiInterruptCounter++;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Get Wifi Interrupt Counter Error!\n");
	}
}

INT32 MtCmdGetWifiInterruptCnt(RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR WifiIntNum,
							   UINT32 WifiIntMask, UINT32 *pWifiInterruptCnt)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_MAC_INFO_T MacInfoCmd;
	EXTRA_ARG_WF_INTERRUPT_CNT_T *pWifiInterruptCntArg =
		&MacInfoCmd.aucExtraArgument.WifiInterruptCntArg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	os_zero_mem(&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_MAC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = (sizeof(UINT32) * WifiIntNum) + sizeof(GET_WF_INTERRUPT_CNT_T) + 4;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pWifiInterruptCnt);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetWifiInterruptCntRsp);
	MtAndesInitCmdMsg(msg, attr);
	MacInfoCmd.u2MacInfoId = cpu2le16(MAC_INFO_TYPE_WIFI_INT_CNT);
	pWifiInterruptCntArg->ucBand = ChIdx;
	pWifiInterruptCntArg->ucWifiInterruptNum = WifiIntNum;
	pWifiInterruptCntArg->u4WifiInterruptMask = cpu2le32(WifiIntMask);
	MtAndesAppendCmdMsg(msg, (char *)&MacInfoCmd, sizeof(EXT_CMD_GET_MAC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetMacTxRx(RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bEnable)
{
	struct cmd_msg *msg;
	EXT_CMD_MAC_ENABLE_CTRL_T  MacEnableCmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	os_zero_mem(&MacEnableCmd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_MAC_ENABLE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MacEnableCmd.ucBand = BandIdx;

	if (bEnable)
		MacEnableCmd.ucMacEnable = ENUM_MAC_ENABLE;
	else
		MacEnableCmd.ucMacEnable = ENUM_MAC_DISABLE;

	MtAndesAppendCmdMsg(msg, (char *)&MacEnableCmd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetRxvFilter(RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bEnable)
{
	struct cmd_msg *msg;
	EXT_CMD_RXV_ENABLE_CTRL_T  RxvEnableCmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	os_zero_mem(&RxvEnableCmd, sizeof(EXT_CMD_RXV_ENABLE_CTRL_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RXV_ENABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RXV_ENABLE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	RxvEnableCmd.ucBandIdx = BandIdx;

	if (bEnable)
		RxvEnableCmd.ucRxvEnable = TRUE;
	else
		RxvEnableCmd.ucRxvEnable = FALSE;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ucRxvEnable = %d)\n", RxvEnableCmd.ucRxvEnable);

	MtAndesAppendCmdMsg(msg, (char *)&RxvEnableCmd, sizeof(EXT_CMD_RXV_ENABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef MT_DFS_SUPPORT
INT32 MtCmdSetDfsTxStart(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_MAC_ENABLE_CTRL_T  MacEnableCmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	os_zero_mem(&MacEnableCmd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_MAC_ENABLE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MacEnableCmd.ucBand = BandIdx;
	MacEnableCmd.ucMacEnable = ENUM_MAC_DFS_TXSTART;
	MtAndesAppendCmdMsg(msg, (char *)&MacEnableCmd, sizeof(EXT_CMD_MAC_ENABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif
#ifdef PRE_CAL_MT7622_SUPPORT
INT32 MtCmdRfTestRecal(RTMP_ADAPTER *pAd, UINT32 u4CalId, UINT16 rsp_len)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	CMD_TEST_CTRL_T RfTestCtrl;

	if (IS_MT7622(pAd)) {

		os_zero_mem(&RfTestCtrl, sizeof(CMD_TEST_CTRL_T));
		RfTestCtrl.u.rRfATInfo.Data.rCalParam.u4FuncData = u4CalId;
		RfTestCtrl.u.rRfATInfo.Data.rCalParam.ucDbdcIdx = ENUM_BAND_0;

		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Action = %d\n", RfTestCtrl.ucAction);

		msg = MtAndesAllocCmdMsg(pAd, sizeof(RfTestCtrl));

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RFTEST_RECAL);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

		MtAndesInitCmdMsg(msg, attr);
		MtAndesAppendCmdMsg(msg, (char *)&RfTestCtrl, sizeof(RfTestCtrl));
		ret = chip_cmd_tx(pAd, msg);
	error:
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(ret = %d)\n", ret);
	}
	return ret;
}
#endif /* PRE_CAL_MT7622_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
static VOID MtCmdGetRXDCOCCalResultRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_CMD_GET_RXDCOC_RESULT_T *pDCOCResult = (EXT_CMD_GET_RXDCOC_RESULT_T *)Data;

	if (pDCOCResult->DirectionToCR == TRUE) {	/* Flash/BinFile to CR */
		if (pDCOCResult->RxDCOCResult.ResultSuccess)
			;
		else
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "(ret = %d) write CR for CH %d ,BW %d FAILED!\n"
					  , pDCOCResult->RxDCOCResult.ResultSuccess
					  , le2cpu16(pDCOCResult->RxDCOCResult.u2ChFreq), pDCOCResult->RxDCOCResult.ucBW);
	} else { /* CR to Flash/BinFile */
		if (pDCOCResult->RxDCOCResult.ResultSuccess) {
			os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &pDCOCResult->RxDCOCResult, sizeof(RXDCOC_RESULT_T));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "========== %s GOT result ========\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "(ret = %d) calibration for CH %d ,BW %d FAILED!\n"
					  , pDCOCResult->RxDCOCResult.ResultSuccess
					  , le2cpu16(pDCOCResult->RxDCOCResult.u2ChFreq), pDCOCResult->RxDCOCResult.ucBW);
		}
	}
}

INT32 MtCmdGetRXDCOCCalResult(RTMP_ADAPTER *pAd, BOOLEAN DirectionToCR
							  , UINT16 CentralFreq, UINT8 BW, UINT8 Band, BOOLEAN IsSecondary80, BOOLEAN DoRuntimeCalibration,
							  RXDCOC_RESULT_T *pRxDcocResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RXDCOC_RESULT_T CmdDCOCResult;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		goto error;
	}

	os_zero_mem(&CmdDCOCResult, sizeof(EXT_CMD_GET_RXDCOC_RESULT_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RXDCOC_RESULT_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = sizeof(EXT_CMD_GET_RXDCOC_RESULT_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RXDCOC_CAL_RESULT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pRxDcocResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetRXDCOCCalResultRsp);
	MtAndesInitCmdMsg(msg, attr);
	CmdDCOCResult.DirectionToCR = DirectionToCR;
	CmdDCOCResult.RxDCOCResult.u2ChFreq = cpu2le16(CentralFreq);
	CmdDCOCResult.RxDCOCResult.ucBW = BW;
	CmdDCOCResult.RxDCOCResult.ucBand = Band;
	CmdDCOCResult.RxDCOCResult.DBDCEnable = pAd->CommonCfg.dbdc_mode;
	CmdDCOCResult.RxDCOCResult.bSecBW80 = IsSecondary80;
	CmdDCOCResult.ucDoRuntimeCalibration = DoRuntimeCalibration;

	if (DirectionToCR == TRUE) {
		os_move_mem(&CmdDCOCResult.RxDCOCResult.ucDCOCTBL_I_WF0_SX0_LNA[0],
					&pRxDcocResult->ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
	}

#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)(&CmdDCOCResult.RxDCOCResult.ucDCOCTBL_I_WF0_SX0_LNA[0]), RXDCOC_SIZE);
#endif
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "send cmd Direction[%s] Freq [%d] Input Cent[%d] BW[%d] Band[%d] SecBW80[%d]\n"
			  , (CmdDCOCResult.DirectionToCR == TRUE) ? "ToCR" : "FromCR"
			  , le2cpu16(CmdDCOCResult.RxDCOCResult.u2ChFreq), CentralFreq, CmdDCOCResult.RxDCOCResult.ucBW
			  , CmdDCOCResult.RxDCOCResult.ucBand, CmdDCOCResult.RxDCOCResult.bSecBW80);
	MtAndesAppendCmdMsg(msg, (char *)&CmdDCOCResult, sizeof(EXT_CMD_GET_RXDCOC_RESULT_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdGetTXDPDCalResultRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_CMD_GET_TXDPD_RESULT_T *pDPDResult = (EXT_CMD_GET_TXDPD_RESULT_T *)Data;

	if (pDPDResult->DirectionToCR == TRUE) { /* Flash/BinFile to CR */
		if (pDPDResult->TxDpdResult.ResultSuccess)
			;
		else
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "(ret = %d) write CR for CH %d ,BW %d FAILED!\n"
					  , pDPDResult->TxDpdResult.ResultSuccess
					  , le2cpu16(pDPDResult->TxDpdResult.u2ChFreq), pDPDResult->TxDpdResult.ucBW);
	} else { /* CR to Flash/BinFile */
		if (pDPDResult->TxDpdResult.ResultSuccess) {
			os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &pDPDResult->TxDpdResult, sizeof(TXDPD_RESULT_T));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "========== %s GOT result ========\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "(ret = %d) calibration for CH %d ,BW %d FAILED!\n"
					  , pDPDResult->TxDpdResult.ResultSuccess
					  , le2cpu16(pDPDResult->TxDpdResult.u2ChFreq), pDPDResult->TxDpdResult.ucBW);
		}
	}
}

INT32 MtCmdGetTXDPDCalResult(RTMP_ADAPTER *pAd, BOOLEAN DirectionToCR
							 , UINT16 CentralFreq, UINT8 BW, UINT8 Band, BOOLEAN IsSecondary80, BOOLEAN DoRuntimeCalibration,
							 TXDPD_RESULT_T *pTxDPDResult)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_TXDPD_RESULT_T CmdDPDResult;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		goto error;
	}

	os_zero_mem(&CmdDPDResult, sizeof(EXT_CMD_GET_TXDPD_RESULT_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_TXDPD_RESULT_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = sizeof(EXT_CMD_GET_TXDPD_RESULT_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXDPD_CAL_RESULT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pTxDPDResult);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdGetTXDPDCalResultRsp);
	MtAndesInitCmdMsg(msg, attr);
	CmdDPDResult.DirectionToCR = DirectionToCR;
	CmdDPDResult.TxDpdResult.u2ChFreq = cpu2le16(CentralFreq);
	CmdDPDResult.TxDpdResult.ucBW = BW;
	CmdDPDResult.TxDpdResult.ucBand = Band;
	CmdDPDResult.TxDpdResult.DBDCEnable = pAd->CommonCfg.dbdc_mode;
	CmdDPDResult.TxDpdResult.bSecBW80 = IsSecondary80;
	CmdDPDResult.ucDoRuntimeCalibration = DoRuntimeCalibration;

	if (DirectionToCR == TRUE) {
		os_move_mem(&CmdDPDResult.TxDpdResult.u4DPDG0_WF0_Prim,
					&pTxDPDResult->u4DPDG0_WF0_Prim, TXDPD_SIZE);
	}

#ifdef RT_BIG_ENDIAN
	CmdDPDResult.TxDpdResult.u4DPDG0_WF0_Prim = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF0_Prim);
	CmdDPDResult.TxDpdResult.u4DPDG0_WF1_Prim = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF1_Prim);
	CmdDPDResult.TxDpdResult.u4DPDG0_WF2_Prim = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF2_Prim);
	CmdDPDResult.TxDpdResult.u4DPDG0_WF2_Sec = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF2_Sec);
	CmdDPDResult.TxDpdResult.u4DPDG0_WF3_Prim = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF3_Prim);
	CmdDPDResult.TxDpdResult.u4DPDG0_WF3_Sec = cpu2le32(CmdDPDResult.TxDpdResult.u4DPDG0_WF3_Sec);
#endif
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "send cmd Direction[%s] Freq [%d] Input Cent[%d] BW[%d] Band[%d] SecBW80[%d]\n"
			  , (CmdDPDResult.DirectionToCR == TRUE) ? "ToCR" : "FromCR"
			  , le2cpu16(CmdDPDResult.TxDpdResult.u2ChFreq), CentralFreq, CmdDPDResult.TxDpdResult.ucBW
			  , CmdDPDResult.TxDpdResult.ucBand, CmdDPDResult.TxDpdResult.bSecBW80);
	MtAndesAppendCmdMsg(msg, (char *)&CmdDPDResult, sizeof(EXT_CMD_GET_TXDPD_RESULT_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID MtCmdRDCERsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_CMD_RDCE_VERIFY_T *pRDCEresult = (EXT_CMD_RDCE_VERIFY_T *)Data;

	if (pRDCEresult->Result) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(ret = %d) RDCE VERIFY [PASS]\n",  pRDCEresult->Result);
	} else {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "(ret = %d) RDCE VERIFY [FAIL]\n",  pRDCEresult->Result);
	}
}

INT32 MtCmdRDCE(RTMP_ADAPTER *pAd, UINT8 type, UINT8 BW, UINT8 Band)
{
	struct cmd_msg *msg;
	EXT_CMD_RDCE_VERIFY_T CmdRDCE;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	os_zero_mem(&CmdRDCE, sizeof(EXT_CMD_RDCE_VERIFY_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RDCE_VERIFY_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = sizeof(EXT_CMD_RDCE_VERIFY_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDCE_VERIFY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdRDCERsp);
	MtAndesInitCmdMsg(msg, attr);
	CmdRDCE.Result = FALSE;
	CmdRDCE.ucType = type;
	CmdRDCE.ucBW = BW;
	CmdRDCE.ucBand = Band;
	MtAndesAppendCmdMsg(msg, (char *)&CmdRDCE, sizeof(EXT_CMD_RDCE_VERIFY_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_MT7622_SUPPORT
INT32 MtCmdSetTxLpfCal_7622(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	VOID *pCmdTxLpfInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	if (IS_MT7622(pAd)) {
		Len = TxLpfCalInfoAlloc_7622(pAd, &pCmdTxLpfInfo);

		if (Len == 0) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		msg = MtAndesAllocCmdMsg(pAd, Len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXLPF_CAL_INFO);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
		MtAndesInitCmdMsg(msg, attr);
		MtAndesAppendCmdMsg(msg, (char *)pCmdTxLpfInfo, Len);
		ret = chip_cmd_tx(pAd, msg);
	error:
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
				 "(ret = %d)\n", ret);

		if (pCmdTxLpfInfo != NULL)
			os_free_mem(pCmdTxLpfInfo);
	}
	return ret;
}

INT32 MtCmdSetTxDcIqCal_7622(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	VOID *pCmdTxDcInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = TxDcIqCalInfoAlloc_7622(pAd, &pCmdTxDcInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x\x1b[m\n",
			  Len,
			  ((P_TXDCIQ_CAL_INFO_T)pCmdTxDcInfo)->ucDataToFromFlash,
			  ((P_TXDCIQ_CAL_INFO_T)pCmdTxDcInfo)->ucDataValid,
			  ((P_TXDCIQ_CAL_INFO_T)pCmdTxDcInfo)->u2BitMap);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXDC_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdTxDcInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdTxDcInfo != NULL)
		os_free_mem(pCmdTxDcInfo);

	return ret;
}
INT32 MtCmdSetTxDpdCal_7622(RTMP_ADAPTER *pAd, UINT32 chan)
{
	struct cmd_msg *msg;
	VOID *pCmdTxDpdInfo;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = TxDpdCalInfoAlloc_7622(pAd, &pCmdTxDpdInfo, chan);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x, u4Chan %d\x1b[m\n",
			  Len,
			  ((P_TXDPD_CAL_INFO_T)pCmdTxDpdInfo)->ucDataToFromFlash,
			  ((P_TXDPD_CAL_INFO_T)pCmdTxDpdInfo)->ucDataValid,
			  ((P_TXDPD_CAL_INFO_T)pCmdTxDpdInfo)->u2BitMap,
			  ((P_TXDPD_CAL_INFO_T)pCmdTxDpdInfo)->u4Chan);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXDPD_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdTxDpdInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdTxDpdInfo != NULL)
		os_free_mem(pCmdTxDpdInfo);

	return ret;
}

#endif /* PRE_CAL_MT7622_SUPPORT */

#ifdef PRE_CAL_MT7626_SUPPORT
INT32 MtCmdSetGroupPreCal_7626(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdGroupPreCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = GroupPreCalInfoAlloc_7626(pAd, &pCmdGroupPreCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GROUP_PRE_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdGroupPreCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdGroupPreCalInfo != NULL)
		os_free_mem(pCmdGroupPreCalInfo);

	return ret;
}

INT32 MtCmdSetDpdFlatnessCal_7626(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdDpdFlatnessCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = DpdFlatnessCalInfoAlloc_7626(pAd, &pCmdDpdFlatnessCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DPD_FLATNESS_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdDpdFlatnessCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdDpdFlatnessCalInfo != NULL)
		os_free_mem(pCmdDpdFlatnessCalInfo);

	return ret;
}
#endif /* PRE_CAL_MT7626_SUPPORT */

#ifdef PRE_CAL_MT7915_SUPPORT
INT32 MtCmdSetGroupPreCal_7915(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdGroupPreCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = GroupPreCalInfoAlloc_7915(pAd, &pCmdGroupPreCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GROUP_PRE_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdGroupPreCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdGroupPreCalInfo != NULL)
		os_free_mem(pCmdGroupPreCalInfo);

	return ret;
}

INT32 MtCmdSetDpdFlatnessCal_7915(
	RTMP_ADAPTER *pAd,
	UINT16       idx,
	UINT32       length,
	BOOLEAN      bSecBw80
	)
{
	struct cmd_msg *msg;
	VOID *pCmdDpdFlatnessCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = DpdFlatnessCalInfoAlloc_7915(pAd, &pCmdDpdFlatnessCalInfo, idx, length, bSecBw80);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DPD_FLATNESS_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdDpdFlatnessCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdDpdFlatnessCalInfo != NULL)
		os_free_mem(pCmdDpdFlatnessCalInfo);

	return ret;
}
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
INT32 MtCmdSetGroupPreCal_7986(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdGroupPreCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = GroupPreCalInfoAlloc_7986(pAd, &pCmdGroupPreCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GROUP_PRE_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdGroupPreCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdGroupPreCalInfo != NULL)
		os_free_mem(pCmdGroupPreCalInfo);

	return ret;
}

INT32 MtCmdSetDpdFlatnessCal_7986(
	RTMP_ADAPTER *pAd,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_offset
	)
{
	struct cmd_msg *msg;
	VOID *pCmdDpdFlatnessCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = DpdFlatnessCalInfoAlloc_7986(pAd, &pCmdDpdFlatnessCalInfo, idx, length, eeprom_offset);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DPD_FLATNESS_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdDpdFlatnessCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdDpdFlatnessCalInfo != NULL)
		os_free_mem(pCmdDpdFlatnessCalInfo);

	return ret;
}
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
INT32 MtCmdSetGroupPreCal_7916(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdGroupPreCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = GroupPreCalInfoAlloc_7916(pAd, &pCmdGroupPreCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GROUP_PRE_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdGroupPreCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdGroupPreCalInfo != NULL)
		os_free_mem(pCmdGroupPreCalInfo);

	return ret;
}

INT32 MtCmdSetDpdFlatnessCal_7916(
	RTMP_ADAPTER *pAd,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_ofst
	)
{
	struct cmd_msg *msg;
	VOID *pCmdDpdFlatnessCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = DpdFlatnessCalInfoAlloc_7916(pAd, &pCmdDpdFlatnessCalInfo, idx, length, eeprom_ofst);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DPD_FLATNESS_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdDpdFlatnessCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdDpdFlatnessCalInfo != NULL)
		os_free_mem(pCmdDpdFlatnessCalInfo);

	return ret;
}
#endif /* PRE_CAL_MT7916_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
INT32 MtCmdSetGroupPreCal_7981(RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length)
{
	struct cmd_msg *msg;
	VOID *pCmdGroupPreCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = GroupPreCalInfoAlloc_7981(pAd, &pCmdGroupPreCalInfo, idx, length);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GROUP_PRE_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdGroupPreCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdGroupPreCalInfo != NULL)
		os_free_mem(pCmdGroupPreCalInfo);

	return ret;
}

INT32 MtCmdSetDpdFlatnessCal_7981(
	RTMP_ADAPTER *pAd,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_offset
	)
{
	struct cmd_msg *msg;
	VOID *pCmdDpdFlatnessCalInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;

	Len = DpdFlatnessCalInfoAlloc_7981(pAd, &pCmdDpdFlatnessCalInfo, idx, length, eeprom_offset);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DPD_FLATNESS_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdDpdFlatnessCalInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdDpdFlatnessCalInfo != NULL)
		os_free_mem(pCmdDpdFlatnessCalInfo);

	return ret;
}
#endif /* PRE_CAL_MT7981_SUPPORT */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
static INT32 MtCmdSetTxLpfCal(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	struct cmd_msg *msg;
	VOID *pCmdTxLpfInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
#ifdef RT_BIG_ENDIAN
	P_TXLPF_CAL_INFO_T pTxLpfCalInfo;
	UINT32 i;
#endif

	Len = TxLpfCalInfoAlloc(pAd, rlmCache, &pCmdTxLpfInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x, cPreCalTemp %d\x1b[m\n",
			  Len,
			  ((P_TXLPF_CAL_INFO_T)pCmdTxLpfInfo)->ucDataToFromFlash,
			  ((P_TXLPF_CAL_INFO_T)pCmdTxLpfInfo)->ucDataValid,
			  ((P_TXLPF_CAL_INFO_T)pCmdTxLpfInfo)->u2BitMap,
			  ((P_TXLPF_CAL_INFO_T)pCmdTxLpfInfo)->cPreCalTemp);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXLPF_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	pTxLpfCalInfo = (P_TXLPF_CAL_INFO_T)pCmdTxLpfInfo;
	pTxLpfCalInfo->u2BitMap = cpu2le16(pTxLpfCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM * SCN_NUM; i++)
		pTxLpfCalInfo->au4Data[i] = cpu2le32(pTxLpfCalInfo->au4Data[i]);
#endif
	MtAndesAppendCmdMsg(msg, (char *)pCmdTxLpfInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdTxLpfInfo != NULL)
		os_free_mem(pCmdTxLpfInfo);

	return ret;
}

static INT32 MtCmdSetTxIqCal(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	struct cmd_msg *msg;
	VOID *pCmdTxIqInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	/*Len = TxIqCalInfoAlloc(pAd, pAd->rlmCalCache, &pCmdTxIqInfo);*/
	Len = TxIqCalInfoAlloc(pAd, rlmCache, &pCmdTxIqInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x\x1b[m\n",
			  Len,
			  ((P_TXIQ_CAL_INFO_T)pCmdTxIqInfo)->ucDataToFromFlash,
			  ((P_TXIQ_CAL_INFO_T)pCmdTxIqInfo)->ucDataValid,
			  ((P_TXIQ_CAL_INFO_T)pCmdTxIqInfo)->u2BitMap);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXIQ_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdTxIqInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdTxIqInfo != NULL)
		os_free_mem(pCmdTxIqInfo);

	return ret;
}

static INT32 MtCmdSetTxDcCal(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	struct cmd_msg *msg;
	VOID *pCmdTxDcInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
#ifdef RT_BIG_ENDIAN
	P_TXDC_CAL_INFO_T pTxDcCalInfo;
	UINT32 i;
#endif

	/*Len = TxDcCalInfoAlloc(pAd, pAd->rlmCalCache, &pCmdTxDcInfo);*/
	Len = TxDcCalInfoAlloc(pAd, rlmCache, &pCmdTxDcInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x\x1b[m\n",
			  Len,
			  ((P_TXDC_CAL_INFO_T)pCmdTxDcInfo)->ucDataToFromFlash,
			  ((P_TXDC_CAL_INFO_T)pCmdTxDcInfo)->ucDataValid,
			  ((P_TXDC_CAL_INFO_T)pCmdTxDcInfo)->u2BitMap);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXDC_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	pTxDcCalInfo = (P_TXDC_CAL_INFO_T)pCmdTxDcInfo;
	pTxDcCalInfo->u2BitMap = cpu2le16(pTxDcCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM * SCN_NUM * 6; i++)
		pTxDcCalInfo->au4Data[i] = cpu2le32(pTxDcCalInfo->au4Data[i]);
#endif

	MtAndesAppendCmdMsg(msg, (char *)pCmdTxDcInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdTxDcInfo != NULL)
		os_free_mem(pCmdTxDcInfo);

	return ret;
}

static INT32 MtCmdSetRxFiCal(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	struct cmd_msg *msg;
	VOID *pCmdRxFiInfo = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
#ifdef RT_BIG_ENDIAN
	P_RXFI_CAL_INFO_T pRxFiCalInfo;
	UINT32 i;
#endif

	/*Len = RxFiCalInfoAlloc(pAd, pAd->rlmCalCache, &pCmdRxFiInfo);*/
	Len = RxFiCalInfoAlloc(pAd, rlmCache, &pCmdRxFiInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x\x1b[m\n",
			  Len,
			  ((P_RXFI_CAL_INFO_T)pCmdRxFiInfo)->ucDataToFromFlash,
			  ((P_RXFI_CAL_INFO_T)pCmdRxFiInfo)->ucDataValid,
			  ((P_RXFI_CAL_INFO_T)pCmdRxFiInfo)->u2BitMap);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RXFI_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	pRxFiCalInfo = (P_RXFI_CAL_INFO_T)pCmdRxFiInfo;
	pRxFiCalInfo->u2BitMap = cpu2le16(pRxFiCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM * SCN_NUM * 4; i++)
		pRxFiCalInfo->au4Data[i] = cpu2le32(pRxFiCalInfo->au4Data[i]);
#endif

	MtAndesAppendCmdMsg(msg, (char *)pCmdRxFiInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdRxFiInfo != NULL)
		os_free_mem(pCmdRxFiInfo);

	return ret;
}

static INT32 MtCmdSetRxFdCal(RTMP_ADAPTER *pAd, VOID *rlmCache, UINT32 chGroup)
{
	struct cmd_msg *msg;
	VOID *pCmdRxFdInfo;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
#ifdef RT_BIG_ENDIAN
	P_RXFD_CAL_INFO_T pRxFdCalInfo;
	UINT32 i;
#endif

	/*Len = RxFdCalInfoAlloc(pAd, pAd->rlmCalCache, &pCmdRxFdInfo, chGroup);*/
	Len = RxFdCalInfoAlloc(pAd, rlmCache, &pCmdRxFdInfo, chGroup);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[35m size %d, ucDataToFromFlash %d, ucDataValid %d, u2BitMap %x, u4ChGroupId %d\x1b[m\n",
			  Len,
			  ((P_RXFD_CAL_INFO_T)pCmdRxFdInfo)->ucDataToFromFlash,
			  ((P_RXFD_CAL_INFO_T)pCmdRxFdInfo)->ucDataValid,
			  ((P_RXFD_CAL_INFO_T)pCmdRxFdInfo)->u2BitMap,
			  ((P_RXFD_CAL_INFO_T)pCmdRxFdInfo)->u4ChGroupId);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RXFD_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	pRxFdCalInfo = (P_RXFD_CAL_INFO_T)pCmdRxFdInfo;
	pRxFdCalInfo->u2BitMap = cpu2le16(pRxFdCalInfo->u2BitMap);
	pRxFdCalInfo->u4ChGroupId = cpu2le32(pRxFdCalInfo->u4ChGroupId);
	for (i = 0; i < (SCN_NUM * RX_SWAGC_LNA_NUM) + (SCN_NUM * RX_FDIQ_LPF_GAIN_NUM * RX_FDIQ_TABLE_SIZE * 3); i++)
		pRxFdCalInfo->au4Data[i] = cpu2le32(pRxFdCalInfo->au4Data[i]);
#endif

	MtAndesAppendCmdMsg(msg, (char *)pCmdRxFdInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);

	if (pCmdRxFdInfo != NULL)
		os_free_mem(pCmdRxFdInfo);

	return ret;
}

static INT32 MtCmdSetRlmPorCal(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	struct cmd_msg *msg;
	VOID *pCmdRlmPorInfo;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	Len = RlmPorCalInfoAlloc(pAd, pAd->rlmCalCache, &pCmdRlmPorInfo);

	if (Len == 0) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, Len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_POR_CAL_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)pCmdRlmPorInfo, Len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "\x1b[41m (ret = %d)\x1b[m \n", ret);

	if (pCmdRlmPorInfo != NULL)
		os_free_mem(pCmdRlmPorInfo);

	return ret;
}

VOID rlmCalCacheApply(RTMP_ADAPTER *pAd, VOID *rlmCache)
{
	UINT32 chGroup;

	if (pAd->CommonCfg.CalCacheApply == 0)
		return;

	if (!rlmCalCacheDone(rlmCache)) {
		MtCmdSetRlmPorCal(pAd, rlmCache);
		return;
	}

	MtCmdSetTxLpfCal(pAd, rlmCache);
	MtCmdSetTxIqCal(pAd, rlmCache);
	MtCmdSetTxDcCal(pAd, rlmCache);
	MtCmdSetRxFiCal(pAd, rlmCache);

	for (chGroup = 0; chGroup < 9; chGroup++)
		MtCmdSetRxFdCal(pAd, rlmCache, chGroup);

	MtCmdSetRlmPorCal(pAd, rlmCache);
}
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT32 MtCmdGetPreCalResult(RTMP_ADAPTER *pAd, UINT8 CalId, UINT16 PreCalBitMap)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_PRECAL_RESULT_T PreCalCtrl;
	INT32 ret = 0;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "----------------->\n");
	PreCalCtrl.u2PreCalBitMap = cpu2le16(PreCalBitMap);
	PreCalCtrl.ucCalId = CalId;
	msg = MtAndesAllocCmdMsg(pAd, sizeof(PreCalCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PRE_CAL_RESULT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PreCalCtrl, sizeof(PreCalCtrl));
	ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "<-----------------\n");
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)\n", ret));
	return ret;
}

INT32 MtCmdPreCalReStoreProc(RTMP_ADAPTER *pAd, INT32 *pPreCalBuffer)
{
	UINT32 IDOffset, LenOffset, Offset, Length;
	UINT32 CalDataSize, HeaderSize, chGroup;
	UINT16 BitMap;
	RLM_CAL_CACHE *prlmFlash = NULL;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "----------------->\n");
	/* Initialization */
	IDOffset = 0;
	LenOffset = 1; /* Skip ID field */

	/* Allocate memory for temp cahce buffer*/
	if (os_alloc_mem(pAd, (UCHAR **)&prlmFlash, sizeof(RLM_CAL_CACHE))) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\x1b[41m Not enough memory for dynamic allocating !!!! \x1b[m\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(prlmFlash, sizeof(RLM_CAL_CACHE));

	if (*(pPreCalBuffer + IDOffset) == PRECAL_TXLPF) {
		P_TXLPF_CAL_INFO_T pCalData = &prlmFlash->txLpfCalInfo;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[41m  PRECAL_TXLPF ------------>\x1b[m\n");
		/* Update header size and calibration data size */
		HeaderSize = (UINT32)(uintptr_t)&((TXLPF_CAL_INFO_T *)NULL)->au4Data[0];
		CalDataSize = TXLPF_PER_GROUP_DATA_SIZE;
		/* Query length of pre-cal item */
		Length = *(pPreCalBuffer + LenOffset);
		/* Update the header */
		Offset = LenOffset + 1;
		os_move_mem(&pCalData->ucDataToFromFlash, pPreCalBuffer + Offset, HeaderSize);
		/* Get bitmap */
		BitMap = pCalData->u2BitMap;
		/* Update the calibration data */
		Offset += HeaderSize / sizeof(INT32);

		for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++) {
			if (BitMap & (1 << chGroup)) {
				os_move_mem(&pCalData->au4Data[chGroup * CalDataSize / sizeof(UINT32)], pPreCalBuffer + Offset, CalDataSize);
				Offset += CalDataSize / sizeof(INT32);
			}
		}

		/* Update offset of parameter */
		IDOffset = LenOffset + (Length / sizeof(INT32));
		LenOffset = IDOffset + 1; /* Skip ID field */
		RLM_CAL_CACHE_TXLPF_CAL_DONE(prlmFlash);
		hex_dump("PRECAL_TXLPF: ", (char *)pCalData, sizeof(TXLPF_CAL_INFO_T));
	}

	if (*(pPreCalBuffer + IDOffset) == PRECAL_TXIQ) {
		P_TXIQ_CAL_INFO_T pCalData = &prlmFlash->txIqCalInfo;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[41m PRECAL_TXIQ ------------>\x1b[m\n");
		/* Update header size and calibration data size */
		HeaderSize = (UINT32)(uintptr_t)&((TXIQ_CAL_INFO_T *)NULL)->au4Data[0];
		CalDataSize = TXIQ_PER_GROUP_DATA_SIZE;
		/* Query length of pre-cal item */
		Length = *(pPreCalBuffer + LenOffset);
		/* Update the header */
		Offset = LenOffset + 1;
		os_move_mem(&pCalData->ucDataToFromFlash, pPreCalBuffer + Offset, HeaderSize);
		/* Get bitmap */
		BitMap = pCalData->u2BitMap;
		/* Update the calibration data */
		Offset += HeaderSize / sizeof(INT32);

		for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++) {
			if (BitMap & (1 << chGroup)) {
				os_move_mem(&pCalData->au4Data[chGroup * CalDataSize / sizeof(UINT32)], pPreCalBuffer + Offset, CalDataSize);
				Offset += CalDataSize / sizeof(INT32);
			}
		}

		/* Update offset of parameter */
		IDOffset = LenOffset + (Length / sizeof(INT32));
		LenOffset = IDOffset + 1; /* Skip ID field */
		RLM_CAL_CACHE_TXIQ_CAL_DONE(prlmFlash);
		hex_dump("PRECAL_TXIQ: ", (char *)pCalData, sizeof(TXIQ_CAL_INFO_T));
	}

	if (*(pPreCalBuffer + IDOffset) == PRECAL_TXDC) {
		P_TXDC_CAL_INFO_T pCalData = &prlmFlash->txDcCalInfo;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[41m PRECAL_TXDC ------------>\x1b[m\n");
		/* Update header size and calibration data size */
		HeaderSize = (UINT32)(uintptr_t)&((TXDC_CAL_INFO_T *)NULL)->au4Data[0];
		CalDataSize = TXDC_PER_GROUP_DATA_SIZE;
		/* Query length of pre-cal item */
		Length = *(pPreCalBuffer + LenOffset);
		/* Update the header */
		Offset = LenOffset + 1;
		os_move_mem(&pCalData->ucDataToFromFlash, pPreCalBuffer + Offset, HeaderSize);
		/* Get bitmap */
		BitMap = pCalData->u2BitMap;
		/* Update the calibration data */
		Offset += HeaderSize / sizeof(INT32);

		for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++) {
			if (BitMap & (1 << chGroup)) {
				os_move_mem(&pCalData->au4Data[chGroup * CalDataSize / sizeof(UINT32)], pPreCalBuffer + Offset, CalDataSize);
				Offset += CalDataSize / sizeof(INT32);
			}
		}

		/* Update offset of parameter */
		IDOffset = LenOffset + (Length / sizeof(INT32));
		LenOffset = IDOffset + 1; /* Skip ID field */
		RLM_CAL_CACHE_TXDC_CAL_DONE(prlmFlash);
		hex_dump("PRECAL_TXDC: ", (char *)pCalData, sizeof(TXDC_CAL_INFO_T));
	}

	for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++) {
		if (*(pPreCalBuffer + IDOffset) == PRECAL_RXFD) {
			P_RXFD_CAL_CACHE_T pCalData = &(prlmFlash->rxFdCalInfo[chGroup]);
			UINT32 chGroupID;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "\x1b[41m PRECAL_RXFD group %d------------>\x1b[m\n", chGroup));
			/* Update header size and calibration data size */
			HeaderSize = (UINT32)(uintptr_t)&((RXFD_CAL_INFO_T *)NULL)->au4Data[0];
			CalDataSize = RXFD_PER_GROUP_DATA_SIZE;
			/* Query length of pre-cal item */
			Length = *(pPreCalBuffer + LenOffset);
			/* Update the header */
			Offset = LenOffset + 1;
			os_move_mem(&pCalData->ucDataToFromFlash, pPreCalBuffer + Offset, HeaderSize);
			/* Get bitmap */
			BitMap = pCalData->u2BitMap;
			/* Get chgroup ID */
			chGroupID = pCalData->u4ChGroupId;
			/* Update the calibration data */
			Offset += HeaderSize / sizeof(INT32);

			if (BitMap & (1 << chGroupID)) {
				os_move_mem(&pCalData->au4Data[0], pPreCalBuffer + Offset, CalDataSize);
				Offset += CalDataSize / sizeof(INT32);
			}

			/* Update offset of parameter */
			IDOffset = LenOffset + (Length / sizeof(INT32));
			LenOffset = IDOffset + 1; /* Skip ID field */
			RLM_CAL_CACHE_RXFD_CAL_DONE(prlmFlash, chGroup);
			hex_dump("PRECAL_RXFD: ", (char *)pCalData, sizeof(RXFD_CAL_CACHE_T));
		}
	}

	if (*(pPreCalBuffer + IDOffset) == PRECAL_RXFI) {
		P_RXFI_CAL_INFO_T pCalData = &prlmFlash->rxFiCalInfo;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[41m PRECAL_RXFI ------------>\x1b[m\n");
		/* Update header size and calibration data size */
		HeaderSize = (UINT32)(uintptr_t)&((RXFI_CAL_INFO_T *)NULL)->au4Data[0];
		CalDataSize = RXFI_PER_GROUP_DATA_SIZE;
		/* Query length of pre-cal item */
		Length = *(pPreCalBuffer + LenOffset);
		/* Update the header */
		Offset = LenOffset + 1;
		os_move_mem(&pCalData->ucDataToFromFlash, pPreCalBuffer + Offset, HeaderSize);
		/* Get bitmap */
		BitMap = pCalData->u2BitMap;
		/* Update the calibration data */
		Offset += HeaderSize / sizeof(INT32);

		for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++) {
			if (BitMap & (1 << chGroup)) {
				os_move_mem(&pCalData->au4Data[chGroup * CalDataSize / sizeof(UINT32)], pPreCalBuffer + Offset, CalDataSize);
				Offset += CalDataSize / sizeof(INT32);
			}
		}

		/* Update offset of parameter */
		IDOffset = LenOffset + (Length / sizeof(INT32));
		LenOffset = IDOffset + 1; /* Skip ID field */
		RLM_CAL_CACHE_RXFI_CAL_DONE(prlmFlash);
		hex_dump("PRECAL_RXFI: ", (char *)pCalData, sizeof(RXFI_CAL_INFO_T));
	}

	/* Send restored calibration data to FW*/
	MtCmdSetTxLpfCal(pAd, prlmFlash);
	MtCmdSetTxIqCal(pAd, prlmFlash);
	MtCmdSetTxDcCal(pAd, prlmFlash);
	MtCmdSetRxFiCal(pAd, prlmFlash);

	for (chGroup = 0; chGroup < CHANNEL_GROUP_NUM; chGroup++)
		MtCmdSetRxFdCal(pAd, prlmFlash, chGroup);

	os_free_mem(prlmFlash);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "----------------->\n");
	return NDIS_STATUS_SUCCESS;
}
#endif /* PRE_CAL_TRX_SET2_SUPPORT */


INT32 MtCmdThermalMode(RTMP_ADAPTER *pAd, UINT8 Mode, UINT8 Action)
{
	struct cmd_msg *msg;
	EXT_CMD_THERMAL_MODE_CTRL_T ThermalModeCtrl = {0};
	INT32 ret = 0;

	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	"----------------->\n");

	ThermalModeCtrl.ucMode = Mode;
	ThermalModeCtrl.ucAction = Action;

	msg = MtAndesAllocCmdMsg(pAd, sizeof(ThermalModeCtrl));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_THERMAL_DBG_CMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&ThermalModeCtrl, sizeof(ThermalModeCtrl));
	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
	"(ret = %d)\n", ret);

	return ret;
}

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
INT32 MtCmdCalReStoreFromFileProc(RTMP_ADAPTER *pAd, CAL_RESTORE_FUNC_IDX FuncIdx)
{
	INT32 Status = NDIS_STATUS_FAILURE;

	if (IS_MT7615(pAd)) {
		switch (FuncIdx) {
		case CAL_RESTORE_PA_TRIM:
			Status = MtCmdPATrimReStoreProc(pAd);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\x1b[41m Not support for restoring this item !!\x1b[m\n");
			break;
		}
	}
	return Status;
}

INT32 MtCmdPATrimReStoreProc(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	EXT_CMD_PA_TRIM_T PATrimCtrl;
	INT32 Status = NDIS_STATUS_FAILURE, i;
	UINT8 idx;
	UINT16 rdAddr;
	USHORT *pPATrimData = (USHORT *)&PATrimCtrl.u4Data[0];

#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif
	if (IS_MT7615(pAd)) {
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"----------------->\n");

	os_zero_mem(&PATrimCtrl, sizeof(EXT_CMD_PA_TRIM_T));
	PATrimCtrl.Header.ucFuncIndex = CAL_RESTORE_PA_TRIM;
	PATrimCtrl.Header.u4DataLen = PA_TRIM_SIZE;

	/* Load data from EEPROM */
	rdAddr = PA_TRIM_START_ADDR1;
	for (idx = 0 ; idx < PA_TRIM_BLOCK_SIZE; idx++) {
		RT28xx_EEPROM_READ16(pAd, rdAddr, *pPATrimData);
		pPATrimData++;
		rdAddr += 2;
	}

	rdAddr = PA_TRIM_START_ADDR2;

	for (idx = 0; idx < PA_TRIM_BLOCK_SIZE; idx++) {
		RT28xx_EEPROM_READ16(pAd, rdAddr, *pPATrimData);
		pPATrimData++;
		rdAddr += 2;
	}


	for (i = 0; i < (PA_TRIM_SIZE/sizeof(UINT32)); i++) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[32m WF%d = 0x%08x \x1b[m\n", i, PATrimCtrl.u4Data[i]);
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(PATrimCtrl));
	if (!msg) {
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CAL_RESTORE_FROM_FILE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 60000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);

	MtAndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	PATrimCtrl.Header.u4DataLen = cpu2le32(PATrimCtrl.Header.u4DataLen);
	for (idx = 0; idx < 4; idx++)
		PATrimCtrl.u4Data[idx] = cpu2le32(PATrimCtrl.u4Data[idx]);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&PATrimCtrl, sizeof(PATrimCtrl));

	Status = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"(Status = %d)\n", Status);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<-----------------\n");
		}
	return Status;
}
#endif /* CAL_BIN_FILE_SUPPORT */

#ifdef ZERO_LOSS_CSA_SUPPORT
INT32 MtCmdSetChkPeerLink(RTMP_ADAPTER *pAd, UINT8 WcidCount, UINT8 *wcidlist)
{
	struct cmd_msg *msg;
	EXT_CMD_CHK_PEER_STA_LINK_T ChkPeerStaLink = {0};
	INT32 ret = 0, i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT16 *pWcidList = (UINT16 *)wcidlist;

	ChkPeerStaLink.u1NumOfSta = WcidCount;
	for (i = 0; i < WcidCount; i++)
		ChkPeerStaLink.u2Wcid[i] = pWcidList[i];

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"set wcid list count:%d wcid: %d, %d, %d for null ack event\n",
				WcidCount, ChkPeerStaLink.u2Wcid[0], ChkPeerStaLink.u2Wcid[1],
				ChkPeerStaLink.u2Wcid[2]);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(ChkPeerStaLink));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CHECK_PEER_STA_LINK);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&ChkPeerStaLink, sizeof(ChkPeerStaLink));

	ret  = chip_cmd_tx(pAd, msg);

	return ret;
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetZeroPktLossVariable(RTMP_ADAPTER *pAd, ENUM_ZERO_PKT_LOSS_VARIABLE eVariable, UINT8 Value)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_SET_ZERO_PKT_LOSS_VARIABLE_T SetZeroPktLossVariable = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	SetZeroPktLossVariable.u1ZeroPktLossVariable = eVariable;
	SetZeroPktLossVariable.u1Value = Value;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"set ZeroPktLossVariable: %d value: %d\n",
				SetZeroPktLossVariable.u1ZeroPktLossVariable, SetZeroPktLossVariable.u1Value);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(SetZeroPktLossVariable));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_ZERO_PKT_LOSS_VARIABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&SetZeroPktLossVariable, sizeof(SetZeroPktLossVariable));

	ret  = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s:(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetMacTxEnable(RTMP_ADAPTER *pAd, UINT8 enable)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Set Mac Tx Enable %d\n", enable);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT8));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_MAC_TX_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&enable, sizeof(UINT8));

	ret  = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(ret = %d)\n", ret);
	return ret;
}
#endif /*ZERO_LOSS_CSA_SUPPORT*/

static VOID CmdWifiHifCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "EventExtCmdResult.u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, Data, Len);
}

INT32 MtCmdWifiHifCtrl(RTMP_ADAPTER *ad, UINT8 ucDbdcIdx, UINT8 ucHifCtrlId, VOID *pRsult)
{
	struct cmd_msg *msg;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	if (IS_MT7636(ad) || IS_MT7637(ad) || IS_MT7615(ad) || IS_MT7622(ad)) {
		EXT_CMD_WIFI_HIF_CTRL_T	rCmdWifiHifCtrl = {0};
		msg = MtAndesAllocCmdMsg(ad, sizeof(EXT_CMD_WIFI_HIF_CTRL_T));

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WIFI_HIF_CTRL);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pRsult);
		SET_CMD_ATTR_RSP_HANDLER(attr, CmdWifiHifCtrlRsp);
		MtAndesInitCmdMsg(msg, attr);
		/* Need to conside eidden */
		/* Wifi Hif control ID */
		rCmdWifiHifCtrl.ucHifCtrlId = ucHifCtrlId;
		rCmdWifiHifCtrl.ucDbdcIdx = ucDbdcIdx;
		MtAndesAppendCmdMsg(msg, (char *)&rCmdWifiHifCtrl,
							sizeof(EXT_CMD_WIFI_HIF_CTRL_T));
		ret = chip_cmd_tx(ad, msg);
error:
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(ret = %d)\n", ret);
		return ret;
	} else
		return ret;
}

/*****************************************
 *	FW loading
 ******************************************/
NTSTATUS MtCmdPowerOnWiFiSys(RTMP_ADAPTER *pAd)
{
	NTSTATUS status = 0;
	return status;
}

VOID CmdExtEventRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	INT i;
	UINT8 *pPayload = Data;
	UINT16 u2PayloadLen = Len;
	/* print event raw data */
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "CmdEID=0x%x, EVENT[%d] = ", msg->attr.ext_type, u2PayloadLen);

	for (i = 0; i < u2PayloadLen; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "0x%x ", pPayload[i]);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
}

INT32 MtCmdSendRaw(RTMP_ADAPTER *pAd, UCHAR ExtendID, UCHAR *Input,
				   INT len, UCHAR SetQuery)
{
	BOOLEAN ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	/* send cmd to fw */
	msg = MtAndesAllocCmdMsg(pAd, len);

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "AndesAllocCmdMsg error !!!\n");
		return NDIS_STATUS_RESOURCES;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, ExtendID);
	SET_CMD_ATTR_CTRL_FLAGS(attr, (SetQuery) ?
							INIT_CMD_SET_AND_WAIT_RSP : INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtEventRsp);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)Input, len);
	ret = chip_cmd_tx(pAd, msg);
	return ret;
}

#ifdef MT_DFS_SUPPORT
/* Remember add a RDM compiler flag - Jelly20150205 */
INT32 MtCmdRddCtrl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucDfsCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddRxSel,
	IN UCHAR ucSetVal)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UINT_16 timeOut;
	struct _CMD_ATTRIBUTE attr = {0};
	EXT_CMD_RDD_ON_OFF_CTRL_T rRddOnOffCtrl;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "[MtCmdRddCtrl] dispath CMD start\n");
	msg = MtAndesAllocCmdMsg(pAd, sizeof(rRddOnOffCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/*extend the timeout limit of CAC_END because this command will do calibration*/
	if (ucDfsCtrl == CAC_END) {
		timeOut = 10000;
	} else
		timeOut = 0;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDD_ON_OFF_CTRL);
#ifdef CONFIG_RCSA_SUPPORT
	if ((pAd->CommonCfg.DfsParameter.bRCSAEn == TRUE)
		&& (ucDfsCtrl == RDD_DETECT_INFO  || ucDfsCtrl == RDD_ALTX_CTRL))
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	else
#endif
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, timeOut);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&rRddOnOffCtrl, sizeof(rRddOnOffCtrl));
	rRddOnOffCtrl.ucDfsCtrl = ucDfsCtrl;
	rRddOnOffCtrl.ucRddIdx = ucRddIdex;
	rRddOnOffCtrl.ucRddRxSel = ucRddRxSel;
	rRddOnOffCtrl.ucSetVal = ucSetVal;
	MtAndesAppendCmdMsg(msg, (char *)&rRddOnOffCtrl, sizeof(rRddOnOffCtrl));
	ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "[MtCmdRddCtrl] dispath CMD complete\n");
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "[MtCmdRddCtrl] ret = %d\n", ret);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 mt_cmd_set_fcc5_min_lpn(RTMP_ADAPTER *pAd, UINT16 min_lpn_update)
{
	struct cmd_msg *msg;
	CMD_RDM_FCC5_LPN_UPDATE_T cmd_set_lpn_update;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RDM_FCC5_LPN_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&cmd_set_lpn_update, sizeof(CMD_RDM_FCC5_LPN_UPDATE_T));

	cmd_set_lpn_update.tag = ENUM_RDM_FCC5_LPN_UPDATE;
	cmd_set_lpn_update.fcc_lpn_min = min_lpn_update;
#ifdef RT_BIG_ENDIAN
	cmd_set_lpn_update.tag = cpu2le32(cmd_set_lpn_update.tag);
	cmd_set_lpn_update.fcc_lpn_min = cpu2le16(cmd_set_lpn_update.fcc_lpn_min);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RDM_RADAR_THRES);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_set_lpn_update,
			sizeof(CMD_RDM_FCC5_LPN_UPDATE_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;

}

INT32 mt_cmd_set_radar_thres_param(RTMP_ADAPTER *pAd, P_CMD_RDM_RADAR_THRESHOLD_UPDATE_T p_radar_threshold)
{
	struct cmd_msg *msg;
	CMD_RDM_RADAR_THRESHOLD_UPDATE_T cmd_radar_thres_update;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&cmd_radar_thres_update, sizeof(CMD_RDM_FCC5_LPN_UPDATE_T));
	NdisCopyMemory(&cmd_radar_thres_update, p_radar_threshold, sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));
	cmd_radar_thres_update.tag = ENUM_RDM_RADAR_THRESHOLD_UPDATE;
#ifdef RT_BIG_ENDIAN
	cmd_radar_thres_update.tag
		= cpu2le32(cmd_radar_thres_update.tag);
	cmd_radar_thres_update.radar_type_idx
		= cpu2le16(cmd_radar_thres_update.radar_type_idx);
	cmd_radar_thres_update.rt_pri_max
		= cpu2le32(cmd_radar_thres_update.rt_pri_max);
	cmd_radar_thres_update.rt_pri_min
		= cpu2le32(cmd_radar_thres_update.rt_pri_min);
	cmd_radar_thres_update.rt_stg_pri_diff_min
		= cpu2le32(cmd_radar_thres_update.rt_stg_pri_diff_min);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RDM_RADAR_THRES);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_radar_thres_update,
			sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;

}

INT32 mt_cmd_set_pls_thres_param(RTMP_ADAPTER *pAd, P_CMD_RDM_PULSE_THRESHOLD_UPDATE_T p_pls_threshold)
{
	struct cmd_msg *msg;
	CMD_RDM_PULSE_THRESHOLD_UPDATE_T cmd_set_pls_thres_update;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RDM_PULSE_THRESHOLD_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&cmd_set_pls_thres_update, sizeof(CMD_RDM_PULSE_THRESHOLD_UPDATE_T));
	NdisCopyMemory(&cmd_set_pls_thres_update, p_pls_threshold, sizeof(CMD_RDM_PULSE_THRESHOLD_UPDATE_T));
	cmd_set_pls_thres_update.tag = ENUM_RDM_PULSE_THRESHOLD_UPDATE;
#ifdef RT_BIG_ENDIAN
	cmd_set_pls_thres_update.tag
		= cpu2le32(cmd_set_pls_thres_update.tag);
	cmd_set_pls_thres_update.prd_pls_width_max
		= cpu2le32(cmd_set_pls_thres_update.prd_pls_width_max);
	cmd_set_pls_thres_update.pls_pwr_max
		= cpu2le32(cmd_set_pls_thres_update.pls_pwr_max);
	cmd_set_pls_thres_update.pls_pwr_min
		= cpu2le32(cmd_set_pls_thres_update.pls_pwr_min);
	cmd_set_pls_thres_update.pri_max_cr
		= cpu2le32(cmd_set_pls_thres_update.pri_max_cr);
	cmd_set_pls_thres_update.pri_min_cr
		= cpu2le32(cmd_set_pls_thres_update.pri_min_cr);
	cmd_set_pls_thres_update.pri_max_stgr
		= cpu2le32(cmd_set_pls_thres_update.pri_max_stgr);
	cmd_set_pls_thres_update.pri_min_stgr
		= cpu2le32(cmd_set_pls_thres_update.pri_min_stgr);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RDM_RADAR_THRES);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_set_pls_thres_update,
			sizeof(CMD_RDM_PULSE_THRESHOLD_UPDATE_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;

}

INT32 mt_cmd_set_test_radar_pattern(RTMP_ADAPTER *pAd, P_CMD_RDM_TEST_RADAR_PATTERN_T p_test_pls_pattern)
{
	struct cmd_msg *msg;
	CMD_RDM_TEST_RADAR_PATTERN_T cmd_set_test_pls_pattern;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
#ifdef RT_BIG_ENDIAN
	int i = 0;
#endif

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RDM_TEST_RADAR_PATTERN_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&cmd_set_test_pls_pattern, sizeof(CMD_RDM_TEST_RADAR_PATTERN_T));
	NdisCopyMemory(&cmd_set_test_pls_pattern, p_test_pls_pattern, sizeof(CMD_RDM_TEST_RADAR_PATTERN_T));
#ifdef RT_BIG_ENDIAN
	for (i = 0; i < cmd_set_test_pls_pattern.pls_num; i++) {
		cmd_set_test_pls_pattern.prd_pls_buff[i].prd_strt_time
			= cpu2le32(cmd_set_test_pls_pattern.prd_pls_buff[i].prd_strt_time);
		cmd_set_test_pls_pattern.prd_pls_buff[i].prd_pls_wdth
			= cpu2le16(cmd_set_test_pls_pattern.prd_pls_buff[i].prd_pls_wdth);
		cmd_set_test_pls_pattern.prd_pls_buff[i].prd_pls_pwr
			= cpu2le16(cmd_set_test_pls_pattern.prd_pls_buff[i].prd_pls_pwr);
	}
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RDM_TEST_PATTERN);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_set_test_pls_pattern,
			sizeof(CMD_RDM_TEST_RADAR_PATTERN_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;

}

INT32 mt_cmd_set_rdd_log_config(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 hw_rdd_log_en,
	IN UINT8 sw_rdd_log_en,
	IN UINT8 sw_rdd_log_cond)
{
	struct cmd_msg *msg;
	CMD_RDM_RDD_LOG_CONFIG_UPDATE_T cmd_set_rdd_log_config;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RDM_RDD_LOG_CONFIG_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&cmd_set_rdd_log_config, sizeof(CMD_RDM_RDD_LOG_CONFIG_UPDATE_T));

	cmd_set_rdd_log_config.tag = ENUM_RDM_RDD_LOG_CONFIG_UPDATE;
#ifdef RT_BIG_ENDIAN
	cmd_set_rdd_log_config.tag = cpu2le16(cmd_set_rdd_log_config.tag);
#endif
	cmd_set_rdd_log_config.hw_rdd_log_en = hw_rdd_log_en;
	cmd_set_rdd_log_config.sw_rdd_log_en = sw_rdd_log_en;
	cmd_set_rdd_log_config.sw_rdd_log_cond = sw_rdd_log_cond;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RDM_RADAR_THRES);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_set_rdd_log_config,
			sizeof(CMD_RDM_RDD_LOG_CONFIG_UPDATE_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);

	return ret;

}
#endif

#if OFF_CH_SCAN_SUPPORT
INT32 mt_cmd_off_ch_scan(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _EXT_CMD_OFF_CH_SCAN_CTRL_T *ext_cmd_param)
{
	struct cmd_msg *msg;
	INT32 ret = 0;

	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "dispath CMD start\n");
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_OFF_CH_SCAN_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_OFF_CH_SCAN_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)ext_cmd_param, sizeof(EXT_CMD_OFF_CH_SCAN_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
INT32 MtCmdBgndScan(RTMP_ADAPTER *pAd, MT_BGND_SCAN_CFG BgScCfg)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0, i = 0;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif

	if (BgScCfg.CentralChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "central channel = 0 is invalid\n");
		return -1;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "control_ch = %d, central_chl = %d, BW = %d,TXStream = %d, RXPath = %d, BandIdx = %d, Reason(%d)\n",
			  BgScCfg.ControlChannel, BgScCfg.CentralChannel, BgScCfg.Bw, BgScCfg.TxStream, BgScCfg.RxPath, BgScCfg.BandIdx,
			  BgScCfg.Reason);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_RX_PATH);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_SET_RX_PATH/*EXT_CMD_CHANNEL_SWITCH*/, TRUE, 0,
					  TRUE, TRUE, 8, NULL, EventExtCmdResult);
#endif
	os_zero_mem(&CmdChanSwitch, sizeof(CmdChanSwitch));
	CmdChanSwitch.ucPrimCh = BgScCfg.ControlChannel;
	CmdChanSwitch.ucCentralCh = BgScCfg.CentralChannel;
	CmdChanSwitch.ucTxStreamNum = BgScCfg.TxStream;
	CmdChanSwitch.ucRxStreamNum = BgScCfg.RxPath;/* Rx Path */
	CmdChanSwitch.ucDbdcIdx = BgScCfg.BandIdx;
	CmdChanSwitch.ucBW = GetCfgBw2RawBw(BgScCfg.Bw);
	CmdChanSwitch.ucSwitchReason = BgScCfg.Reason;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "control_ch = %d, central_chl = %d, BW = %d,TXStream = %d, RXStream = %d, BandIdx=%d, Reason(%d)\n",
			  CmdChanSwitch.ucPrimCh, CmdChanSwitch.ucCentralCh, CmdChanSwitch.ucBW, CmdChanSwitch.ucTxStreamNum,
			  CmdChanSwitch.ucRxStreamNum, CmdChanSwitch.ucDbdcIdx, CmdChanSwitch.ucSwitchReason);

	for (i = 0; i < SKU_SIZE; i++)
		CmdChanSwitch.acTxPowerSKU[i] = 0x3f;

	MtAndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdBgndScanNotify(RTMP_ADAPTER *pAd, MT_BGND_SCAN_NOTIFY BgScNotify)
{
	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _EXT_CMD_BGND_SCAN_NOTIFY_T CmdBgndScNotify;
#if (NEW_MCU_INIT_CMD_API)
	struct _CMD_ATTRIBUTE attr = {0};
#endif
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NotifyFunc = %d, BgndScanStatus = %d\n",
			 BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdBgndScNotify));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

#if (NEW_MCU_INIT_CMD_API)
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BGND_SCAN_NOTIFY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
#else
	MtAndesInitCmdMsg(msg, HOST2N9, EXT_CID, CMD_SET, EXT_CMD_ID_BGND_SCAN_NOTIFY, TRUE, 0,
					  TRUE, TRUE, 8, NULL, EventExtCmdResult);
#endif
	os_zero_mem(&CmdBgndScNotify, sizeof(CmdBgndScNotify));
	CmdBgndScNotify.ucNotifyFunc = BgScNotify.NotifyFunc;
	CmdBgndScNotify.ucBgndScanStatus = BgScNotify.BgndScanStatus;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucNotifyFunc = %d, ucBgndScanStatus = %d\n",
			 CmdBgndScNotify.ucNotifyFunc, CmdBgndScNotify.ucBgndScanStatus);
	MtAndesAppendCmdMsg(msg, (char *)&CmdBgndScNotify, sizeof(CmdBgndScNotify));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* BACKGROUND_SCAN_SUPPORT */

INT32 MtCmdCr4Query(RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_QUERY_T  CmdCr4SetQuery;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ":%s: option(%d)\n", __func__, arg0);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdCr4SetQuery));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_QUERY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetQuery, sizeof(CmdCr4SetQuery));
	CmdCr4SetQuery.u4Cr4QueryOptionArg0 = cpu2le32(arg0);
	CmdCr4SetQuery.u4Cr4QueryOptionArg1 = cpu2le32(arg1);
	CmdCr4SetQuery.u4Cr4QueryOptionArg2 = cpu2le32(arg2);
	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetQuery,
						sizeof(CmdCr4SetQuery));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 MtCmdCr4MultiQuery(RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2, void *para)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_MULTI_QUERY_T  CmdCr4SetMultiQuery;
	struct _CMD_ATTRIBUTE attr = {0};
	PCR4_QUERY_STRUC cr4_query = (PCR4_QUERY_STRUC)para;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: option(%d,%d)\n", __func__, arg0, arg1);

	msg = MtAndesAllocCmdMsg(pAd,
			sizeof(CmdCr4SetMultiQuery) + arg1 * sizeof(*(cr4_query->list)));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_QUERY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetMultiQuery, sizeof(CmdCr4SetMultiQuery));
	CmdCr4SetMultiQuery.u4Cr4QueryOptionArg0 = cpu2le32(arg0);
	CmdCr4SetMultiQuery.u4Cr4QueryOptionArg1 = cpu2le32(arg1);
	CmdCr4SetMultiQuery.u4Cr4QueryOptionArg2 = cpu2le32(arg2);

	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetMultiQuery, sizeof(CmdCr4SetMultiQuery));
	MtAndesAppendCmdMsg(msg, (char *)cr4_query->list, arg1 * sizeof(*(cr4_query->list)));

	Ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: (ret = %d)\n", __func__, Ret);

	return Ret;
}

INT32 MtCmdCr4QueryBssAcQPktNum(
	struct _RTMP_ADAPTER *pAd,
	UINT32 u4bssbitmap)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_QUERY_T  CmdCr4SetQuery;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ":%s: u4bssbitmap(0x%08X)\n", __func__, u4bssbitmap);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdCr4SetQuery));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_QUERY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetQuery, sizeof(CmdCr4SetQuery));
	CmdCr4SetQuery.u4Cr4QueryOptionArg0 = CR4_QUERY_OPTION_GET_BSS_ACQ_PKT_NUM;
	CmdCr4SetQuery.u4Cr4QueryOptionArg1 = cpu2le32(u4bssbitmap);
	CmdCr4SetQuery.u4Cr4QueryOptionArg2 = 0;
#ifdef RT_BIG_ENDIAN
	CmdCr4SetQuery.u4Cr4QueryOptionArg0 = cpu2le32(CmdCr4SetQuery.u4Cr4QueryOptionArg0);
	CmdCr4SetQuery.u4Cr4QueryOptionArg2 = cpu2le32(CmdCr4SetQuery.u4Cr4QueryOptionArg2);
#endif
	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetQuery, sizeof(CmdCr4SetQuery));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 MtCmdCr4Set(RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_SET_T  CmdCr4SetSet;
	struct _CMD_ATTRIBUTE attr = {0};
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ":%s: arg0(%d) arg1(%d) arg2(%d)\n",
			  __func__, arg0, arg1, arg2);

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#ifdef RED_SUPPORT
	if (!IS_MT7915(pAd) && !IS_MT7986(pAd) && !IS_MT7916(pAd) && !IS_MT7981(pAd)) {
		BOOLEAN cmd_id_path = FALSE;

		switch (arg0) {
		case CR4_SET_ID_RED_ENABLE:
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				Ret = UniCmdSetRedEnable(pAd, HOST2CR4, arg1);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				Ret = MtCmdSetRedEnable(pAd, HOST2CR4, arg1);
			cmd_id_path = TRUE;
			break;
		case CR4_SET_ID_RED_TARGET_DELAY:
			Ret = MtCmdSetRedTargetDelay(pAd, HOST2CR4, arg1);
			cmd_id_path = TRUE;
			break;
		case CR4_SET_ID_RED_SHOW_STA:
			Ret = MtCmdSetRedShowSta(pAd, HOST2CR4, arg1);
			cmd_id_path = TRUE;
			break;
		default:
			break;
		}

		if (cmd_id_path)
			return Ret;
	}
#endif
#endif
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdCr4SetSet));

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
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	CmdCr4SetSet.u4Cr4SetArg0 = cpu2le32(arg0);
	CmdCr4SetSet.u4Cr4SetArg1 = cpu2le32(arg1);
	CmdCr4SetSet.u4Cr4SetArg2 = cpu2le32(arg2);
	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetSet, sizeof(CmdCr4SetSet));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 MtCmdCr4Capability(RTMP_ADAPTER *pAd, UINT32 option)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_CAPABILITY_T CmdCr4SetCapability;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ":option(%d)\n", option);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdCr4SetCapability));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_CAPABILITY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetCapability, sizeof(CmdCr4SetCapability));
	CmdCr4SetCapability.u4Cr4Capability = cpu2le32(option);
	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetCapability,
						sizeof(CmdCr4SetCapability));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 MtCmdCr4Debug(RTMP_ADAPTER *pAd, UINT32 option)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _EXT_CMD_CR4_DEBUG_T CmdCr4SetDebug;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ": option(%d)\n", option);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CmdCr4SetDebug));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ID_CR4);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CR4_DEBUG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&CmdCr4SetDebug, sizeof(CmdCr4SetDebug));
	CmdCr4SetDebug.u4Cr4Debug = cpu2le32(option);
	MtAndesAppendCmdMsg(msg, (char *)&CmdCr4SetDebug,
						sizeof(CmdCr4SetDebug));
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 mt_cmd_wo_query(RTMP_ADAPTER *pAd, UINT32 option, UINT32 param0, UINT32 param1)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _EXT_CMD_WO_QUERY_T  cmd_wo_query;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s: option(0x%x)\n", __func__, option);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(cmd_wo_query));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		return ret;
	}

	os_zero_mem(&cmd_wo_query, sizeof(cmd_wo_query));
	cmd_wo_query.query_arg0 = cpu2le32(param0);
	cmd_wo_query.query_arg1 = cpu2le32(param1);
	MtAndesAppendCmdMsg(msg, (char *)&cmd_wo_query, sizeof(cmd_wo_query));

	ret = call_fw_cmd_notifieriers(option, pAd, msg->net_pkt);

	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);
	AndesFreeCmdMsg(msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"option = %d (ret = %d)\n", option, ret);
	return ret;
}

static VOID mt_cmd_support_rate_ctrl_rsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
}

INT32 mt_cmd_support_rate_table_ctrl(
	RTMP_ADAPTER *pAd,
	UINT8 tx_mode,
	UINT8 tx_nss,
	UINT8 tx_bw,
	UINT16 *mcs_cap,
	BOOLEAN set
)
{
	struct cmd_msg *msg;
	CMD_RA_SUPPORT_MCS_CAP_CTRL_T mcs_cap_ctrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"tx_mode: %d, tx_nss: %d, tx_bw: %d, mcs_cap: %d, set: %d\n",
		tx_mode, tx_nss, tx_bw, *mcs_cap, set);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RA_SUPPORT_MCS_CAP_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&mcs_cap_ctrl, sizeof(CMD_RA_SUPPORT_MCS_CAP_CTRL_T));

	mcs_cap_ctrl.rate_ctrl_format_id = SUPPORT_RATE_MCS_CAP_CTRL;
	mcs_cap_ctrl.tx_mode = tx_mode;
	mcs_cap_ctrl.tx_nss = tx_nss;
	mcs_cap_ctrl.tx_bw = tx_bw;
	mcs_cap_ctrl.mcs_cap = *mcs_cap;
	mcs_cap_ctrl.set = set;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RA_CTRL);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);

	if (set) {
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	} else {
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, mcs_cap);
		SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_support_rate_ctrl_rsp);
	}

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&mcs_cap_ctrl,
		sizeof(CMD_RA_SUPPORT_MCS_CAP_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_ra_dbg_ctrl(
	RTMP_ADAPTER *pAd,
	UINT8 param_num,
	UINT32 *param
)
{
	struct cmd_msg *msg;
	CMD_RA_DBG_CTRL_T ra_dbg_ctrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 i;

	MTWF_PRINT("param_num: %d\n",
		 param_num);

	MTWF_PRINT("param:");

	for (i = 0; i < 20; i++)
		MTWF_PRINT(" %d", *(param + i));

	MTWF_PRINT("\n");

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_RA_DBG_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ra_dbg_ctrl, sizeof(CMD_RA_DBG_CTRL_T));

	ra_dbg_ctrl.rate_ctrl_format_id = RA_DBG_CTRL;
	ra_dbg_ctrl.param_num = param_num;
	for (i = 0 ; i < 20; i++)
		ra_dbg_ctrl.param[i] = *(param + i);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RA_CTRL);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ra_dbg_ctrl,
		sizeof(CMD_RA_DBG_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxPowerSKUCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      tx_pwr_sku_en,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_SKU_CTRL_T PowerSKUCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "tx_pwr_sku_en: %d, BandIdx: %d\n", tx_pwr_sku_en, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_SKU_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PowerSKUCtrl, sizeof(CMD_POWER_SKU_CTRL_T));

	if (cap->txpower_type == TX_POWER_TYPE_V0)
		PowerSKUCtrl.ucPowerCtrlFormatId = SKU_FEATURE_CTRL_V0;
	else
		PowerSKUCtrl.ucPowerCtrlFormatId = SKU_POWER_LIMIT_CTRL;
	PowerSKUCtrl.ucSKUEnable         = tx_pwr_sku_en;
	PowerSKUCtrl.ucBandIdx           = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PowerSKUCtrl,
						sizeof(CMD_POWER_SKU_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcManCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcManual
)
{
	struct cmd_msg *msg;
	CMD_TPC_MAN_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgTpcManual: %d \n", fgTpcManual);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_MAN_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_MAN_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_MANUAL_MODE;
	if (fgTpcManual == TRUE)
		TpcCtrl.u1TpcManual = TPC_PARAM_MAN_MODE;
	else
		TpcCtrl.u1TpcManual = TPC_PARAM_AUTO_MODE;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
						sizeof(CMD_TPC_MAN_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcEnableCfg(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcEnable
)
{
	struct cmd_msg *msg;
	CMD_TPC_MAN_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_MAN_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_MAN_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_ENABLE_CFG;
	TpcCtrl.fgTpcEnable = fgTpcEnable;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
						sizeof(CMD_TPC_MAN_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcWlanIdCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink,
	UINT8   u1EntryIdx,
	UINT16  u2WlanId,
	UINT8 u1DlTxType
)
{
	struct cmd_msg *msg;
	CMD_TPC_MAN_WLAN_ID_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"fgUplink: %d, u1EntryIdx: %d, u2WlanId: %d, u1DlTxType: %d\n",
				 fgUplink, u1EntryIdx, u2WlanId, u1DlTxType);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_MAN_WLAN_ID_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_MAN_WLAN_ID_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_WLANID_CTRL;
	TpcCtrl.u1EntryIdx = u1EntryIdx;
	TpcCtrl.u2WlanId = u2WlanId;
#ifdef RT_BIG_ENDIAN
	TpcCtrl.u2WlanId = cpu2le16(TpcCtrl.u2WlanId);
#endif
	TpcCtrl.fgUplink = fgUplink;
	TpcCtrl.u1DlTxType = (ENUM_TPC_DL_TX_TYPE)u1DlTxType;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
					sizeof(CMD_TPC_MAN_WLAN_ID_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcUlAlgoCtrl(
	RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	UINT8	u1ApTxPwr,
	UINT8	u1EntryIdx,
	UINT8	u1TargetRssi,
	UINT8	u1UPH,
	BOOLEAN	fgMinPwrFlag
)
{
	struct cmd_msg *msg;
	CMD_TPC_UL_ALGO_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1TpcCmd: %d, u1ApTxPwr: %d\n", u1TpcCmd, u1ApTxPwr);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1EntryIdx: %d, u1TargetRssi: %d, u1UPH: %d, fgMinPwrFlag: %d\n",
				u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_UL_ALGO_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_UL_ALGO_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = u1TpcCmd+1;
	TpcCtrl.u1ApTxPwr = u1ApTxPwr;
	TpcCtrl.u1EntryIdx = u1EntryIdx;
	TpcCtrl.u1TargetRssi = u1TargetRssi;
	TpcCtrl.u1UPH = u1UPH;
	TpcCtrl.fgMinPwrFlag = fgMinPwrFlag;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
						sizeof(CMD_TPC_UL_ALGO_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcDlAlgoCtrl(
	RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	BOOLEAN	fgCmdCtrl,
	UINT8	u1DlTxType,
	CHAR	DlTxPwr,
	UINT8	u1EntryIdx,
	INT16	DlTxpwrAlpha
)
{
	struct cmd_msg *msg;
	CMD_TPC_DL_ALGO_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1TpcCmd: %d, fgCmdCtrl: %d\n", u1TpcCmd, fgCmdCtrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1DlTxType: %d, DlTxPwr: %d, u1EntryIdx: %d, DlTxpwrAlpha: %d\n",
				u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_DL_ALGO_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_DL_ALGO_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = u1TpcCmd+4;
	TpcCtrl.DlTxPwr = DlTxPwr;
	TpcCtrl.fgDlTxPwrCmdCtrl = fgCmdCtrl;
	TpcCtrl.u1EntryIdx = u1EntryIdx;
	TpcCtrl.DlTxPwrAlpha = DlTxpwrAlpha;
	TpcCtrl.u1DlTxType = (ENUM_TPC_DL_TX_TYPE)u1DlTxType;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
						sizeof(CMD_TPC_DL_ALGO_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcManTblInfo(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink
)
{
	struct cmd_msg *msg;
	CMD_TPC_MAN_TBL_INFO_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgUplink: %d \n", fgUplink);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_MAN_TBL_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_MAN_TBL_INFO_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_MAN_TBL_INFO;
	TpcCtrl.fgUplink = fgUplink;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	/* TODO: event */
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
					sizeof(CMD_TPC_MAN_TBL_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcUlUtVarCfg(
	RTMP_ADAPTER *pAd,
	UINT8	u1EntryIdx,
	UINT8	u1VarType,
	INT16	i2Value)
{
	struct cmd_msg *msg;
	CMD_TPC_UL_UT_VAR_CFG_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};


	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1EntryIdx: %d, VarType:%d, Value: %d\n",
				u1EntryIdx, u1VarType, i2Value);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_UL_UT_VAR_CFG_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_UL_UT_VAR_CFG_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_UL_UNIT_TEST_CONFIG;
	TpcCtrl.u1EntryIdx = u1EntryIdx;
	TpcCtrl.u1VarType = u1VarType;
	TpcCtrl.i2Value = i2Value;
#ifdef RT_BIG_ENDIAN
	TpcCtrl.i2Value = cpu2le16(TpcCtrl.i2Value);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	/* TODO: event */
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
					sizeof(CMD_TPC_UL_UT_VAR_CFG_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTpcUlUtGo(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcUtGo
)
{
	struct cmd_msg *msg;
	CMD_TPC_UL_UT_CTRL_T TpcCtrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fgTpcUtGo: %d \n", fgTpcUtGo);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TPC_UL_UT_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcCtrl, sizeof(CMD_TPC_UL_UT_CTRL_T));
	TpcCtrl.u1TpcCtrlFormatId = TPC_ACT_UL_UNIT_TEST_GO;
	TpcCtrl.fgTpcUtGo = fgTpcUtGo;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TPC_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	/* TODO: event */
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcCtrl,
					sizeof(CMD_TPC_UL_UT_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxPowerPercentCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      fgTxPowerPercentEn,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_PERCENTAGE_CTRL_T PowerPercentCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgTxPowerPercentEn: %d, BandIdx: %d\n", fgTxPowerPercentEn, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_PERCENTAGE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PowerPercentCtrl, sizeof(CMD_POWER_PERCENTAGE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		PowerPercentCtrl.ucPowerCtrlFormatId = PERCENTAGE_FEATURE_CTRL_V0;
	else
		PowerPercentCtrl.ucPowerCtrlFormatId = PERCENTAGE_CTRL;
	PowerPercentCtrl.ucPercentageEnable  = fgTxPowerPercentEn;
	PowerPercentCtrl.ucBandIdx           = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PowerPercentCtrl,
						sizeof(CMD_POWER_PERCENTAGE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxPowerDropCtrl(
	RTMP_ADAPTER *pAd,
	INT8         cPowerDropLevel,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_PERCENTAGE_DROP_CTRL_T PowerDropCtrl;
	INT32 ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "cPowerDropLevel: %d, BandIdx: %d\n", cPowerDropLevel, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_PERCENTAGE_DROP_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PowerDropCtrl, sizeof(CMD_POWER_PERCENTAGE_DROP_CTRL_T));

	if (cap->txpower_type == TX_POWER_TYPE_V0)
		PowerDropCtrl.ucPowerCtrlFormatId = PERCENTAGE_DROP_CTRL_V0;
	else
		PowerDropCtrl.ucPowerCtrlFormatId = PERCENTAGE_DROP_CTRL_V1;
	PowerDropCtrl.cPowerDropLevel     = cPowerDropLevel;
	PowerDropCtrl.ucBandIdx           = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PowerDropCtrl,
						sizeof(CMD_POWER_PERCENTAGE_DROP_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxCCKStream(
	RTMP_ADAPTER *pAd,
	UINT8        u1CCKTxStream,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_TX_CCK_STREAM_CTRL_T CCKTxStreamCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u1CCKTxStream: %d, BandIdx: %d\n", u1CCKTxStream, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TX_CCK_STREAM_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CCKTxStreamCtrl, sizeof(CMD_TX_CCK_STREAM_CTRL_T));

	CCKTxStreamCtrl.u1CCKTxStream = u1CCKTxStream;
	CCKTxStreamCtrl.ucBandIdx = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_CCK_STREAM_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CCKTxStreamCtrl,
						sizeof(CMD_TX_CCK_STREAM_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxBfBackoffCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      fgTxBFBackoffEn,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_BF_BACKOFF_CTRL_T TxBFBackoffCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgTxBFBackoffEn: %d, BandIdx: %d\n", fgTxBFBackoffEn, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_BF_BACKOFF_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TxBFBackoffCtrl, sizeof(CMD_POWER_BF_BACKOFF_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TxBFBackoffCtrl.ucPowerCtrlFormatId = BF_POWER_BACKOFF_FEATURE_CTRL_V0;
	else
		TxBFBackoffCtrl.ucPowerCtrlFormatId = BACKOFF_POWER_LIMIT_CTRL;
	TxBFBackoffCtrl.ucBFBackoffEnable   = fgTxBFBackoffEn;
	TxBFBackoffCtrl.ucBandIdx           = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TxBFBackoffCtrl,
						sizeof(CMD_POWER_BF_BACKOFF_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdThermoCompCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      fgThermoCompEn,
	UCHAR        BandIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_THERMAL_COMP_CTRL_T ThermoCompCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgThermoCompEn: %d, BandIdx: %d\n", fgThermoCompEn, BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_THERMAL_COMP_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ThermoCompCtrl, sizeof(CMD_POWER_THERMAL_COMP_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		ThermoCompCtrl.ucPowerCtrlFormatId = THERMAL_COMPENSATION_CTRL_V0;
	else
		ThermoCompCtrl.ucPowerCtrlFormatId = THERMAL_COMPENSATION_CTRL_V1;
	ThermoCompCtrl.fgThermalCompEn     = fgThermoCompEn;
	ThermoCompCtrl.ucBandIdx           = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ThermoCompCtrl,
						sizeof(CMD_POWER_THERMAL_COMP_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxPwrRfTxAntCtrl(
	RTMP_ADAPTER *pAd,
	UINT8        ucTxAntIdx
)
{
	struct cmd_msg *msg;
	CMD_POWER_RF_TXANT_CTRL_T PowerRfTxAntCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ucTxAntIdx = 0x%x \n", ucTxAntIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_RF_TXANT_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PowerRfTxAntCtrl, sizeof(CMD_POWER_RF_TXANT_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		PowerRfTxAntCtrl.ucPowerCtrlFormatId = RF_TXANT_CTRL_V0;
	else
		PowerRfTxAntCtrl.ucPowerCtrlFormatId = RF_TXANT_CTRL_V1;
	PowerRfTxAntCtrl.ucTxAntIdx          = ucTxAntIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&PowerRfTxAntCtrl,
						sizeof(CMD_POWER_RF_TXANT_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdTxPwrShowInfo(
	RTMP_ADAPTER *pAd,
	UCHAR        ucTxPowerInfoCatg,
	UINT8        ucBandIdx
)
{
	struct cmd_msg *msg;
	CMD_TX_POWER_SHOW_INFO_T TxPowerShowInfoCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "ucTxPowerInfoCatg: %d, BandIdx: %d \n", ucTxPowerInfoCatg, ucBandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TX_POWER_SHOW_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TxPowerShowInfoCtrl, sizeof(CMD_TX_POWER_SHOW_INFO_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TxPowerShowInfoCtrl.ucPowerCtrlFormatId = TX_POWER_SHOW_INFO_V0;
	else
		TxPowerShowInfoCtrl.ucPowerCtrlFormatId = TX_POWER_SHOW_INFO_V1;
	TxPowerShowInfoCtrl.ucTxPowerInfoCatg   = ucTxPowerInfoCatg;
	TxPowerShowInfoCtrl.ucBandIdx           = ucBandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TxPowerShowInfoCtrl,
						sizeof(CMD_TX_POWER_SHOW_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef WIFI_EAP_FEATURE
RATE_TABLE_UPDATE arRateTableData[] = {
	{eRateSwitchTable,   eRateSwTbl11b,             "RateSwitchTableAGBS11B"},
	{eRateSwitchTable,   eRateSwTbl11g,             "RateSwitchTableAGBS11G"},
	{eRateSwitchTable,   eRateSwTbl11bg,            "RateSwitchTableAGBS11BG"},
	{eRateSwitchTable,   eRateSwTbl11n1ss,          "RateSwitchTableAGBS11N1SS"},
	{eRateSwitchTable,   eRateSwTbl11n2ss,          "RateSwitchTableAGBS11N2SS"},
	{eRateSwitchTable,   eRateSwTbl11n3ss,          "RateSwitchTableAGBS11N3SS"},
	{eRateSwitchTable,   eRateSwTbl11n4ss,          "RateSwitchTableAGBS11N4SS"},
	{eRateSwitchTable,   eRateSwTblvht1ss,          "RateSwitchTableAGBSVht1SS"},
	{eRateSwitchTable,   eRateSwTblvht2ss,          "RateSwitchTableAGBSVht2SS"},
	{eRateSwitchTable,   eRateSwTblvht3ss,          "RateSwitchTableAGBSVht3SS"},
	{eRateSwitchTable,   eRateSwTblvht4ss,          "RateSwitchTableAGBSVht4SS"},
	{eRateSwitchTable,   eRateSwTblvht2ssbccbw80,   "RateSwitchTableAGBSVht2SSBccBw80"},
	{eRateSwitchTable,   eRateSwTblhe1ss,           "RateSwitchTableAGBSHe1SS"},
	{eRateSwitchTable,   eRateSwTblhe2ss,           "RateSwitchTableAGBSHe2SS"},

	{eRateHwFbTable,     eRateHwFbTbl11b,	        "HwFallbackTable11B"},
	{eRateHwFbTable,     eRateHwFbTbl11g,	        "HwFallbackTable11G"},
	{eRateHwFbTable,     eRateHwFbTbl11bg,	        "HwFallbackTable11BG"},
	{eRateHwFbTable,     eRateHwFbTbl11n1ss,        "HwFallbackTable11N1SS"},
	{eRateHwFbTable,     eRateHwFbTbl11n2ss,        "HwFallbackTable11N2SS"},
	{eRateHwFbTable,     eRateHwFbTbl11n3ss,        "HwFallbackTable11N3SS"},
	{eRateHwFbTable,     eRateHwFbTbl11n4ss,        "HwFallbackTable11N4SS"},
	{eRateHwFbTable,     eRateHwFbTblbgn1ss,        "HwFallbackTableBGN1SS"},
	{eRateHwFbTable,     eRateHwFbTblbgn2ss,        "HwFallbackTableBGN2SS"},
	{eRateHwFbTable,     eRateHwFbTblbgn3ss,        "HwFallbackTableBGN3SS"},
	{eRateHwFbTable,     eRateHwFbTblbgn4ss,        "HwFallbackTableBGN4SS"},
	{eRateHwFbTable,     eRateHwFbTblvht1ss,        "HwFallbackTableVht1SS"},
	{eRateHwFbTable,     eRateHwFbTblvht2ss,        "HwFallbackTableVht2SS"},
	{eRateHwFbTable,     eRateHwFbTblvht3ss,        "HwFallbackTableVht3SS"},
	{eRateHwFbTable,     eRateHwFbTblvht4ss,        "HwFallbackTableVht4SS"},
	{eRateHwFbTable,     eRateHwFbTblvht2ssbccbw80, "HwFallbackTableVht2SSBccBw80"},
	{eRateHwFbTable,     eRateHwFbTblhe1ss,         "HwFallbackTableHe1SS"},
	{eRateHwFbTable,     eRateHwFbTblhe2ss,         "HwFallbackTableHe2SS"},

	{eRateTableMax,      0,                         "\0"}
};

PCHAR getRaTableName(UINT8 TblType, UINT8 TblIdx)
{
	UINT8 TblArrayIdx = 0;
	PCHAR TblName = "\0";

	if (TblType >= eRateTableMax)
		return TblName;

	while (arRateTableData[TblArrayIdx].u1RaTblType < eRateTableMax) {
		if ((TblType == arRateTableData[TblArrayIdx].u1RaTblType)
			&& (TblIdx == arRateTableData[TblArrayIdx].u1RaTblIdx)) {
			TblName = arRateTableData[TblArrayIdx].acTableName;
			break;
		}

		TblArrayIdx++;
	}

	return TblName;
}

UINT8 getRaTableIndex(UINT8 TblType, CHAR *TblName)
{
	UINT8 Index = 0;

	while (arRateTableData[Index].u1RaTblType < eRateTableMax) {
		if ((arRateTableData[Index].u1RaTblType == TblType)
			&& (strcmp(arRateTableData[Index].acTableName, TblName) == FALSE))
			return arRateTableData[Index].u1RaTblIdx;

		Index++;
	}

	return RA_TBL_INDEX_INVALID;
}
INT32 MtCmdInitIPICtrl(
	RTMP_ADAPTER *pAd,
	UINT8 BandIdx
)
{
	struct cmd_msg *msg;
	CMD_INIT_IPI_CTRL_T rInitIPICtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx: %d\n", BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(rInitIPICtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rInitIPICtrl, sizeof(rInitIPICtrl));
	rInitIPICtrl.u4EapCtrlCmdId = INIT_IPI_CTRL;
	rInitIPICtrl.u1BandIdx = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	rInitIPICtrl.u4EapCtrlCmdId = cpu2le32(rInitIPICtrl.u4EapCtrlCmdId);

	MtAndesAppendCmdMsg(msg, (char *)&rInitIPICtrl, sizeof(rInitIPICtrl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID ShowEapRaTblInfoCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_RATE_TABLE pTblentry = (P_EVENT_SHOW_RATE_TABLE)rsp_payload;
	UINT8 NumofRow, NumofCol, RowIndex, ElemIdx;
	PCHAR TblName;
	CHAR *fname = NULL;
	UINT32 write_size, buf_size = 512;
	UCHAR buf[512];
	RTMP_OS_FD_EXT srcf;
	INT retval;

	TblName = getRaTableName(pTblentry->u1RaTblTypeIdx, pTblentry->u1RaTblIdx);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s TableType:%u TableIdx:%u RW:%u\n", __func__,
			 pTblentry->u1RaTblTypeIdx, pTblentry->u1RaTblIdx, pTblentry->u1RW);

	if (pTblentry->u1RW) {
		if (pTblentry->u1RaTblTypeIdx == eRateSwitchTable)
			fname = EAP_FW_RA_SWITCH_TBL_PATH;
		else if (pTblentry->u1RaTblTypeIdx == eRateHwFbTable)
			fname = EAP_FW_RA_HW_FB_TBL_PATH;
		else
			fname = NULL;

		if (!fname) {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					" fname is NULL\n");
			pTblentry->u1RW = 0;
		} else {
			memset(buf, 0, sizeof(buf));
			srcf = os_file_open(fname, O_WRONLY | O_CREAT | O_APPEND, 0);

			if (srcf.Status) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Open file \"%s\" failed!\n", fname);
				pTblentry->u1RW = 0;
			}
		}
	}

	if (pTblentry->u1RaTblTypeIdx == eRateSwitchTable) {
		PUINT8 Buf = NULL;

		Buf = (PUINT8) pTblentry->ucBuf;
		NumofCol = NUM_OF_COL_RATE_SWITCH_TABLE;
		NumofRow = (pTblentry->u2RaTblLength) / (NumofCol * sizeof(*Buf));

		MTWF_PRINT("NumofRow:%u\n", NumofRow);
		MTWF_PRINT("Table:%s\nItem\tMode\tMCS\tTrnUp\tTrnDn\tUpIdx\tDnIdx"
				 "\tNssUp\tNssDn\tDnIdx2\tCngBw\tAM24\tAM816\tTxCnt\tCBRN\n",
				 TblName);

		if (pTblentry->u1RW) {
			retval = snprintf(buf + strlen(buf), buf_size - strlen(buf),
					"Table:%s\nItem\tMode\tMCS\tTrnUp\tTrnDn\tUpIdx\tDnIdx"
					"\tNssUp\tNssDn\tDnIdx2\tCngBw\tAM24\tAM816\tTxCnt\tCBRN\n",
					TblName);
			if (os_snprintf_error((buf_size - strlen(buf)), retval)) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " buffer Snprintf failed!\n");
				return;
			}


			write_size = strlen(buf);
			retval = os_file_write(srcf, buf, write_size);

			if (retval <= 0)
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Write file \"%s\" failed for header!\n", fname);

			memset(buf, 0, buf_size);
		}

		for (RowIndex = 0; RowIndex < NumofRow; RowIndex++) {
			ElemIdx = RowIndex * NumofCol;
			MTWF_PRINT("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u"
					 "\t%u\t%u\t%u\t%u\n",
					 Buf[ElemIdx], Buf[ElemIdx + 1], Buf[ElemIdx + 2],
					 Buf[ElemIdx + 3], Buf[ElemIdx + 4], Buf[ElemIdx + 5],
					 Buf[ElemIdx + 6], Buf[ElemIdx + 7], Buf[ElemIdx + 8],
					 Buf[ElemIdx + 9], Buf[ElemIdx + 10], Buf[ElemIdx + 11],
					 Buf[ElemIdx + 12], Buf[ElemIdx + 13], Buf[ElemIdx + 14]);

			if (pTblentry->u1RW) {
				retval = snprintf(buf + strlen(buf), buf_size - strlen(buf),
						"%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u"
						"\t%u\t%u\t%u\n",
						Buf[ElemIdx], Buf[ElemIdx + 1], Buf[ElemIdx + 2],
						Buf[ElemIdx + 3], Buf[ElemIdx + 4], Buf[ElemIdx + 5],
						Buf[ElemIdx + 6], Buf[ElemIdx + 7], Buf[ElemIdx + 8],
						Buf[ElemIdx + 9], Buf[ElemIdx + 10], Buf[ElemIdx + 11],
						Buf[ElemIdx + 12], Buf[ElemIdx + 13], Buf[ElemIdx + 14]);
				if (os_snprintf_error((buf_size - strlen(buf)), retval)) {
					MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " buffer Snprintf failed!\n");
					return;
				}

				write_size = strlen(buf);
				retval = os_file_write(srcf, buf, write_size);

				if (retval <= 0)
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Write file \"%s\" failed for row:%d!\n",
							 fname, RowIndex);

				memset(buf, 0, buf_size);
			}
		}
	} else if (pTblentry->u1RaTblTypeIdx == eRateHwFbTable) {
		PUINT16 Buf = NULL;

		Buf = (PUINT16)pTblentry->ucBuf;
		NumofCol = NUM_OF_COL_RATE_HWFB_TABLE;
		NumofRow = (pTblentry->u2RaTblLength) / (NumofCol * sizeof(*Buf));

		MTWF_PRINT("NumofRow:%u\n", NumofRow);
		MTWF_PRINT("Table:%s\nRate1\tRate2\tRate3\tRate4"
				 "\tRate5\tRate6\tRate7\tRate8\n",
				 TblName);

		if (pTblentry->u1RW) {
			snprintf(buf + strlen(buf), buf_size - strlen(buf),
					"Table:%s\nRate1\tRate2\tRate3\tRate4"
					"\tRate5\tRate6\tRate7\tRate8\n",
					TblName);
			write_size = strlen(buf);
			retval = os_file_write(srcf, buf, write_size);

			if (retval <= 0)
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Write file \"%s\" failed for header!\n", fname);

			memset(buf, 0, buf_size);
		}

		for (RowIndex = 0; RowIndex < NumofRow; RowIndex++) {
			ElemIdx = RowIndex * NumofCol;
			MTWF_PRINT("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
					 Buf[ElemIdx], Buf[ElemIdx + 1], Buf[ElemIdx + 2],
					 Buf[ElemIdx + 3], Buf[ElemIdx + 4], Buf[ElemIdx + 5],
					 Buf[ElemIdx + 6], Buf[ElemIdx + 7]);

			if (pTblentry->u1RW) {
				snprintf(buf + strlen(buf), buf_size - strlen(buf),
						"%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
						Buf[ElemIdx], Buf[ElemIdx + 1], Buf[ElemIdx + 2],
						Buf[ElemIdx + 3], Buf[ElemIdx + 4], Buf[ElemIdx + 5],
						Buf[ElemIdx + 6], Buf[ElemIdx + 7]);

				write_size = strlen(buf);
				retval = os_file_write(srcf, buf, write_size);

				if (retval <= 0)
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Write file \"%s\" failed for row:%d!\n",
							 fname, RowIndex);

				memset(buf, 0, buf_size);
			}
		}
	}

	if (pTblentry->u1RW) {
		if (os_file_close(srcf) != 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Close file \"%s\" failed!\n", fname);
		} else {
			MTWF_PRINT("Successfully written in \"%s\"!\n", fname);
		}
	}
}

static VOID ShowEapIPIValueCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_GET_IPI_VALUE pMentry = (P_EVENT_GET_IPI_VALUE)rsp_payload;

	MTWF_PRINT("%s\n", __func__);
	MTWF_PRINT("IPI value:[0] %x [1] %x [2] %x [3] %x",
			 le2cpu32(pMentry->au4IPIValue[0]), le2cpu32(pMentry->au4IPIValue[1]),
			 le2cpu32(pMentry->au4IPIValue[2]), le2cpu32(pMentry->au4IPIValue[3]));
	MTWF_PRINT("IPI value:[4] %x [5] %x [6] %x [7] %x",
			 le2cpu32(pMentry->au4IPIValue[4]), le2cpu32(pMentry->au4IPIValue[5]),
			 le2cpu32(pMentry->au4IPIValue[6]), le2cpu32(pMentry->au4IPIValue[7]));
	MTWF_PRINT("IPI value:[8] %x [9] %x [10] %x",
			 le2cpu32(pMentry->au4IPIValue[8]), le2cpu32(pMentry->au4IPIValue[9]),
			 le2cpu32(pMentry->au4IPIValue[10]));

}

static VOID eapEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *)rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u4EventId = %u, len = %u\n", u4EventId, len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

	switch (u4EventId) {
	case EAP_EVENT_IPI_VALUE:
		MTWF_PRINT("EAP_EVENT_IPI_VALUE\n");
		ShowEapIPIValueCallback(pData, len);
		break;

	case EAP_EVENT_SHOW_RATE_TABLE:
		MTWF_PRINT("EAP_EVENT_SHOW_RATE_TABLE\n");
		ShowEapRaTblInfoCallback(pData, len);
		break;

	default:
		break;
	}
}

INT32 MtCmdGetIPIValue(
	RTMP_ADAPTER *pAd,
	UINT8 BandIdx
)
{
	struct cmd_msg *msg;
	CMD_GET_IPI_VALUE rGetIPIVal;
	EVENT_GET_IPI_VALUE rIPIValue;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx: %d\n", BandIdx);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(rGetIPIVal));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&rGetIPIVal, sizeof(rGetIPIVal));
	rGetIPIVal.u4EapCtrlCmdId = GET_IPI_VALUE;
	rGetIPIVal.u1BandIdx = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(rIPIValue));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &rIPIValue);
	SET_CMD_ATTR_RSP_HANDLER(attr, eapEventDispatcher);
	MtAndesInitCmdMsg(msg, attr);
	rGetIPIVal.u4EapCtrlCmdId = cpu2le32(rGetIPIVal.u4EapCtrlCmdId);
	MtAndesAppendCmdMsg(msg, (char *)&rGetIPIVal,
						sizeof(rGetIPIVal));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetDataTxPwrOffset(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 WlanIdx,
	IN INT8 TxPwr_Offset,
	IN UINT8 BandIdx
)
{
	struct cmd_msg *msg;
	CMD_SET_DATA_TXPWR_OFFSET rSetDataTxPwrOffset;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(rSetDataTxPwrOffset));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rSetDataTxPwrOffset, sizeof(rSetDataTxPwrOffset));
	rSetDataTxPwrOffset.u4EapCtrlCmdId = SET_DATA_TXPWR_OFFSET;
	rSetDataTxPwrOffset.u1WlanIdx = (UINT8)WlanIdx;
	rSetDataTxPwrOffset.i1TxPwrOffset = TxPwr_Offset;
	rSetDataTxPwrOffset.u1BandIdx = BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	rSetDataTxPwrOffset.u4EapCtrlCmdId = cpu2le32(rSetDataTxPwrOffset.u4EapCtrlCmdId);
	MtAndesAppendCmdMsg(msg, (char *)&rSetDataTxPwrOffset,
						sizeof(rSetDataTxPwrOffset));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetRaTable(
	PRTMP_ADAPTER pAd,
	UINT8 BandIdx,
	UINT8 TblType,
	UINT8 TblIndex,
	UINT16 TblLength,
	PUCHAR Buffer
)
{
	struct cmd_msg *msg;
	CMD_SET_RA_TABLE rSetRaTblParams;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT_16 Length = 0;

	if (!Buffer) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	Length = sizeof(rSetRaTblParams) + TblLength - sizeof(rSetRaTblParams.ucBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TblType:%u TblIdx:%u BandIdx: %u, Length:%u\n",
			  TblType, TblIndex, BandIdx, Length);

	msg = MtAndesAllocCmdMsg(pAd, Length);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rSetRaTblParams, Length);
	rSetRaTblParams.u4EapCtrlCmdId = SET_RA_TABLE_DATA;
	rSetRaTblParams.u1RaTblTypeIdx = TblType;
	rSetRaTblParams.u1RaTblIdx = TblIndex;
	rSetRaTblParams.u1BandIdx = BandIdx;
	rSetRaTblParams.u2RaTblLength = TblLength;
	os_move_mem(rSetRaTblParams.ucBuf, Buffer, TblLength);
	rSetRaTblParams.u4EapCtrlCmdId = cpu2le32(rSetRaTblParams.u4EapCtrlCmdId);
	rSetRaTblParams.u2RaTblLength = cpu2le32(rSetRaTblParams.u2RaTblLength);
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(rSetRaTblParams.ucBuf, rSetRaTblParams.u2RaTblLength);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rSetRaTblParams, Length);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdGetRaTblInfo(
	RTMP_ADAPTER *pAd,
	UINT8 BandIdx,
	UINT8 TblType,
	UINT8 TblIndex,
	UINT8 ReadnWrite
)
{
	struct cmd_msg *msg;
	CMD_SHOW_RATE_TABLE rGetRaTbl;
	EVENT_SHOW_RATE_TABLE rRaTblInfo;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(rGetRaTbl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rGetRaTbl, sizeof(rGetRaTbl));
	rGetRaTbl.u4EapCtrlCmdId = GET_RATE_INFO;
	rGetRaTbl.u1RaTblTypeIdx = TblType;
	rGetRaTbl.u1RaTblIdx = TblIndex;
	rGetRaTbl.u1BandIdx = BandIdx;
	rGetRaTbl.u1RW = ReadnWrite;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(rRaTblInfo));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &rRaTblInfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, eapEventDispatcher);
	MtAndesInitCmdMsg(msg, attr);
	rGetRaTbl.u4EapCtrlCmdId = cpu2le32(rGetRaTbl.u4EapCtrlCmdId);
	MtAndesAppendCmdMsg(msg, (char *)&rGetRaTbl, sizeof(rGetRaTbl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* WIFI_EAP_FEATURE */

INT32 MtCmdSetEDCCAThreshold(
	struct _RTMP_ADAPTER *pAd,
	UINT8 edcca_threshold[],
	UINT8 BandIdx
)
{
	struct cmd_msg *msg;
	EXT_CMD_EDCCA_CMD_T EdccaCmd;
	INT32 ret = 0;
	INT8 i = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EDCCA_CMD_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&EdccaCmd, sizeof(EdccaCmd));
	EdccaCmd.u1CmdIdx = SET_EDCCA_CTRL_THRES;
	EdccaCmd.u1BandIdx = BandIdx;
	os_move_mem(EdccaCmd.u1Val, edcca_threshold, 3);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EDCCA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	EdccaCmd.u1CmdIdx = cpu2le32(EdccaCmd.u1CmdIdx);
	for (i = 0 ; i < 3 ; i++)
		EdccaCmd.u1Val[i] = cpu2le32(EdccaCmd.u1Val[i]);

	MtAndesAppendCmdMsg(msg, (char *)&EdccaCmd,
						sizeof(EdccaCmd));
	ret = chip_cmd_tx(pAd, msg);
error:
	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdSetEDCCACEnable(
	RTMP_ADAPTER *pAd,
	UCHAR        BandIdx,
	UCHAR        EDCCACtrl,
	UINT8        u1EDCCAStd,
	INT8		 i1compensation
)
{
	struct cmd_msg *msg;
	EXT_CMD_EDCCA_CMD_T EdccaCmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EDCCA_CMD_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&EdccaCmd, sizeof(EXT_CMD_EDCCA_CMD_T));

	EdccaCmd.u1CmdIdx = SET_EDCCA_CTRL_EN;
	EdccaCmd.u1BandIdx = BandIdx;
	EdccaCmd.u1Val[0] = EDCCACtrl;
	EdccaCmd.u1EDCCAStd = u1EDCCAStd;
	EdccaCmd.icompensation = i1compensation;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EDCCA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	EdccaCmd.u1CmdIdx = cpu2le32(EdccaCmd.u1CmdIdx);

	EdccaCmd.u1Val[0] = cpu2le32(EdccaCmd.u1Val[0]);
	MtAndesAppendCmdMsg(msg, (char *)&EdccaCmd,
						sizeof(EdccaCmd));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdGetEDCCAThreshold(
	RTMP_ADAPTER *pAd,
	UCHAR        BandIdx,
	BOOLEAN fginit
)
{
	struct cmd_msg *msg;
	EXT_CMD_EDCCA_CMD_T EdccaCmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EDCCA_CMD_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&EdccaCmd, sizeof(EXT_CMD_EDCCA_CMD_T));
	EdccaCmd.u1CmdIdx = GET_EDCCA_CTRL_THRES;
	EdccaCmd.u1BandIdx = BandIdx;
	EdccaCmd.fginit = fginit;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EDCCA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	EdccaCmd.u1CmdIdx = cpu2le32(EdccaCmd.u1CmdIdx);
	MtAndesAppendCmdMsg(msg, (char *)&EdccaCmd,
						sizeof(EdccaCmd));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdGetEDCCAEnable(
	RTMP_ADAPTER *pAd,
	UCHAR        BandIdx
)
{
		struct cmd_msg *msg;
		EXT_CMD_EDCCA_CMD_T EdccaCmd;
		INT32 ret = 0;
		struct _CMD_ATTRIBUTE attr = {0};

		msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EDCCA_CMD_T));

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		os_zero_mem(&EdccaCmd, sizeof(EXT_CMD_EDCCA_CMD_T));
		EdccaCmd.u1CmdIdx = GET_EDCCA_CTRL_EN;
		EdccaCmd.u1BandIdx = BandIdx;
		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_EDCCA);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
		MtAndesInitCmdMsg(msg, attr);
		EdccaCmd.u1CmdIdx = cpu2le32(EdccaCmd.u1CmdIdx);
		MtAndesAppendCmdMsg(msg, (char *)&EdccaCmd,
							sizeof(EdccaCmd));
		ret = chip_cmd_tx(pAd, msg);
error:
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(ret = %d)\n", ret);
		return ret;
}
#ifdef WIFI_GPIO_CTRL
INT32 MtCmdSetGpioCtrl(
	RTMP_ADAPTER *pAd,
	UINT8 GpioIdx,
	BOOLEAN GpioEn
)
{
	struct cmd_msg *msg;
	CMD_SET_GPIO_ENABLE rSetGpioCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(rSetGpioCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rSetGpioCtrl, sizeof(rSetGpioCtrl));
	rSetGpioCtrl.u4GpioCtrlCmdId = GPIO_GPO_SET_ENABLE;
#ifdef RT_BIG_EMDIAN
	rSetGpioCtrl.u4GpioCtrlCmdId = cpu2le32(rSetGpioCtrl.u4GpioCtrlCmdId);
#endif
	rSetGpioCtrl.u1GpioIdx = GpioIdx;
	rSetGpioCtrl.fgEnable = GpioEn;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GPIO_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rSetGpioCtrl,
						sizeof(rSetGpioCtrl));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_INFO:DBG_LVL_ERROR,
			 "(ret = %d)\n",  ret);
	return ret;
}

INT32 MtCmdSetGpioVal(
	RTMP_ADAPTER *pAd,
	UINT8 GpioIdx,
	UINT8 GpioVal
)
{
	struct cmd_msg *msg;
	CMD_SET_GPIO_VALUE rSetGpioVal;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(rSetGpioVal));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rSetGpioVal, sizeof(rSetGpioVal));
	rSetGpioVal.u4GpioCtrlCmdId = GPIO_GPO_SET_VALUE;
#ifdef RT_BIG_ENDIAN
	rSetGpioVal.u4GpioCtrlCmdId = cpu2le32(rSetGpioVal.u4GpioCtrlCmdId);
#endif
	rSetGpioVal.u1GpioIdx = GpioIdx;
	rSetGpioVal.u1Value = GpioVal;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GPIO_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rSetGpioVal,
						sizeof(rSetGpioVal));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* WIFI_GPIO_CTRL */

INT32 MtCmdTOAECalCtrl(
	RTMP_ADAPTER *pAd,
	UCHAR        TOAECtrl)
{
	struct cmd_msg *msg;
	CMD_TOAE_ON_OFF_CTRL TOAECalCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TOAECtrl = %d\n", TOAECtrl);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_TOAE_ON_OFF_CTRL));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TOAECalCtrl, sizeof(CMD_TOAE_ON_OFF_CTRL));
	TOAECalCtrl.fgTOAEEnable = TOAECtrl;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TOAE_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TOAECalCtrl,
						sizeof(CMD_TOAE_ON_OFF_CTRL));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdMuPwrCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      fgMuTxPwrManEn,
	CHAR         cMuTxPwr,
	UINT8        u1BandIdx)
{
	struct cmd_msg *msg;
	CMD_POWER_MU_CTRL_T rMUPowerCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "u1BandIdx: %d, fgMuTxPwrManEn: %d, cMuTxPwr: %d\n", u1BandIdx, fgMuTxPwrManEn, cMuTxPwr);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_MU_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rMUPowerCtrl, sizeof(CMD_POWER_MU_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rMUPowerCtrl.ucPowerCtrlFormatId = MU_TX_POWER_CTRL_V0;
	else
		rMUPowerCtrl.ucPowerCtrlFormatId = MU_TX_POWER_CTRL_V1;
	rMUPowerCtrl.fgMuTxPwrManEn      = fgMuTxPwrManEn;
	rMUPowerCtrl.cMuTxPwr            = cMuTxPwr;
	rMUPowerCtrl.u1BandIdx           = u1BandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rMUPowerCtrl,
						sizeof(CMD_POWER_MU_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef DATA_TXPWR_CTRL
INT32 MtCmdTxPwrDataPktCtrl(
	RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT8 i1MaxBasePwr,
	IN UINT8 u1BandIdx
)
{
	struct cmd_msg *msg;
	CMD_SET_PER_PKT_TX_POWER_T rDataPktPwrCtrl;
	INT32 ret = 0;
	UINT8 u1BwIdx = 0, u1McsIdx = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_PER_PKT_TX_POWER_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rDataPktPwrCtrl, sizeof(CMD_SET_PER_PKT_TX_POWER_T));

	rDataPktPwrCtrl.u1PowerCtrlFormatId = TX_POWER_SET_PER_PKT_POWER;
	rDataPktPwrCtrl.u2WlanIdx = pEntry->wcid;
	rDataPktPwrCtrl.u1BandIdx = u1BandIdx;
	rDataPktPwrCtrl.i1MaxBasePwr = i1MaxBasePwr;

	for (u1BwIdx = 0; u1BwIdx < DATA_TXPOWER_MAX_BW_NUM; u1BwIdx++) {
		for (u1McsIdx = 0; u1McsIdx < DATA_TXPOWER_MAX_MCS_NUM; u1McsIdx++) {
			rDataPktPwrCtrl.i1PowerOffset[u1BwIdx][u1McsIdx] = pEntry->PowerOffset[u1BwIdx][u1McsIdx];
			MTWF_PRINT("PowerOffset[%d][%d]: %d\n", u1BwIdx, u1McsIdx, rDataPktPwrCtrl.i1PowerOffset[u1BwIdx][u1McsIdx]);
		}
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rDataPktPwrCtrl,
						sizeof(CMD_SET_PER_PKT_TX_POWER_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_PRINT("%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdTxPwrMinDataPktCtrl(
	RTMP_ADAPTER *pAd,
	IN INT8 i1MinBasePwr,
	IN UINT8 u1BandIdx
)
{
	struct cmd_msg *msg;
	CMD_SET_MIN_TX_POWER_T rDataPktMinPwrCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_MIN_TX_POWER_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rDataPktMinPwrCtrl, sizeof(CMD_SET_MIN_TX_POWER_T));

	rDataPktMinPwrCtrl.u1PowerCtrlFormatId = TX_POWER_SET_PER_PKT_MIN_POWER;
	rDataPktMinPwrCtrl.u1BandIdx = u1BandIdx;
	rDataPktMinPwrCtrl.i1MinBasePwr = i1MinBasePwr;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rDataPktMinPwrCtrl,
						sizeof(CMD_SET_MIN_TX_POWER_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_PRINT("%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif

INT32 MtCmdBFNDPATxDCtrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN      fgNDPA_ManualMode,
	UINT8        ucNDPA_TxMode,
	UINT8        ucNDPA_Rate,
	UINT8        ucNDPA_BW,
	UINT8        ucNDPA_PowerOffset)
{
	struct cmd_msg *msg;
	CMD_BF_NDPA_TXD_CTRL_T rBFNDPATxDCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgNDPA_ManualMode: %d, ucNDPA_TxMode: %d, ucNDPA_Rate: %d, ucNDPA_BW: %d, ucNDPA_PowerOffset: %d\n",
			  fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_BF_NDPA_TXD_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rBFNDPATxDCtrl, sizeof(CMD_BF_NDPA_TXD_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rBFNDPATxDCtrl.ucPowerCtrlFormatId = BF_NDPA_TXD_CTRL_V0;
	else
		rBFNDPATxDCtrl.ucPowerCtrlFormatId = BF_NDPA_TXD_CTRL_V1;
	rBFNDPATxDCtrl.fgNDPA_ManualMode   = fgNDPA_ManualMode;
	rBFNDPATxDCtrl.ucNDPA_TxMode       = ucNDPA_TxMode;
	rBFNDPATxDCtrl.ucNDPA_Rate         = ucNDPA_Rate;
	rBFNDPATxDCtrl.ucNDPA_BW           = ucNDPA_BW;
	rBFNDPATxDCtrl.ucNDPA_PowerOffset  = ucNDPA_PowerOffset;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rBFNDPATxDCtrl,
						sizeof(CMD_BF_NDPA_TXD_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtEPAcheck(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	CMD_SET_TSSI_TRAINING_T PA;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_TSSI_TRAINING_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&PA, sizeof(PA));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		PA.ucPowerCtrlFormatId = TSSI_WORKAROUND_V0;
	else
		PA.ucPowerCtrlFormatId = TSSI_WORKAROUND_V1;
	PA.ucSubFuncId         = EPA_STATUS;
	MtAndesAppendCmdMsg(msg, (char *)&PA, sizeof(PA));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtATETSSITracking(RTMP_ADAPTER *pAd, BOOLEAN fgEnable)
{
	struct cmd_msg *msg;
	CMD_SET_TSSI_TRAINING_T rTSSITracking;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgEnable: %d\n", fgEnable);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_TSSI_TRAINING_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&rTSSITracking, sizeof(rTSSITracking));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rTSSITracking.ucPowerCtrlFormatId = TSSI_WORKAROUND_V0;
	else
		rTSSITracking.ucPowerCtrlFormatId = TSSI_WORKAROUND_V1;
	rTSSITracking.ucSubFuncId         = TSSI_TRACKING_ENABLE;
	rTSSITracking.fgEnable            = fgEnable;
	MtAndesAppendCmdMsg(msg, (char *)&rTSSITracking, sizeof(rTSSITracking));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtATEFCBWCfg(RTMP_ADAPTER *pAd, BOOLEAN fgEnable)
{
	struct cmd_msg *msg;
	CMD_SET_TSSI_TRAINING_T rFCBWEnable;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgEnable: %d\n", fgEnable);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_TSSI_TRAINING_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&rFCBWEnable, sizeof(rFCBWEnable));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rFCBWEnable.ucPowerCtrlFormatId = TSSI_WORKAROUND_V0;
	else
		rFCBWEnable.ucPowerCtrlFormatId = TSSI_WORKAROUND_V1;
	rFCBWEnable.ucSubFuncId         = FCBW_ENABLE;
	rFCBWEnable.fgEnable            = fgEnable;
	MtAndesAppendCmdMsg(msg, (char *)&rFCBWEnable, sizeof(rFCBWEnable));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtTSSICompBackup(RTMP_ADAPTER *pAd, BOOLEAN fgEnable)
{
	struct cmd_msg *msg;
	CMD_SET_TSSI_TRAINING_T rTSSICompBackup;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgEnable: %d\n", fgEnable);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_TSSI_TRAINING_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&rTSSICompBackup, sizeof(rTSSICompBackup));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rTSSICompBackup.ucPowerCtrlFormatId = TSSI_WORKAROUND_V0;
	else
		rTSSICompBackup.ucPowerCtrlFormatId = TSSI_WORKAROUND_V1;
	rTSSICompBackup.ucSubFuncId         = TSSI_COMP_BACKUP;
	rTSSICompBackup.fgEnable            = fgEnable;
	MtAndesAppendCmdMsg(msg, (char *)&rTSSICompBackup, sizeof(rTSSICompBackup));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtTSSICompCfg(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	CMD_SET_TSSI_TRAINING_T rTSSICompCfg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_TSSI_TRAINING_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	NdisZeroMemory(&rTSSICompCfg, sizeof(rTSSICompCfg));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		rTSSICompCfg.ucPowerCtrlFormatId = TSSI_WORKAROUND_V0;
	else
		rTSSICompCfg.ucPowerCtrlFormatId = TSSI_WORKAROUND_V1;
	rTSSICompCfg.ucSubFuncId         = TSSI_COMP_CONFIG;
	MtAndesAppendCmdMsg(msg, (char *)&rTSSICompCfg, sizeof(rTSSICompCfg));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 MtCmdThermalManCtrl(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 u1BandIdx,
	IN BOOLEAN fgManualMode,
	IN UINT8 u1ThermalAdc)
{
	struct cmd_msg *msg;
	CMD_THERMAL_MAN_CTRL_T rThermalManCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1BandIdx: %d, fgManualMode: %d, u1ThermalAdc: %d\n", u1BandIdx, fgManualMode, u1ThermalAdc);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_THERMAL_MAN_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rThermalManCtrl, sizeof(CMD_THERMAL_MAN_CTRL_T));
	rThermalManCtrl.u1PowerCtrlFormatId = THERMAL_SENSOR_MANUAL_CTRL;
	rThermalManCtrl.fgManualMode = fgManualMode;
	rThermalManCtrl.u1ThermalAdc = u1ThermalAdc;
	rThermalManCtrl.u1BandIdx = u1BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_THERMAL_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rThermalManCtrl,
						sizeof(CMD_THERMAL_MAN_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdThermalTaskCtrl(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 u1BandIdx,
	IN BOOLEAN fgTrigEn,
	IN UINT8 u1Thres,
	IN UINT32 u4FuncPtr)
{
	struct cmd_msg *msg;
	CMD_THERMAL_SENSOR_TASK_T rThermalSensorTask;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1BandIdx: %d, fgTrigEn: %d, u1Thres: %d\n", u1BandIdx, fgTrigEn, u1Thres);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_THERMAL_SENSOR_TASK_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rThermalSensorTask, sizeof(CMD_THERMAL_SENSOR_TASK_T));
	rThermalSensorTask.u1ThermalCtrlFormatId = THERMAL_SENSOR_TASK_MAN_CONTROL;
	rThermalSensorTask.fgTrigEn = fgTrigEn;
	rThermalSensorTask.u1Thres = u1Thres;
	rThermalSensorTask.u4FuncPtr = u4FuncPtr;
	rThermalSensorTask.u1BandIdx = u1BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_THERMAL_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rThermalSensorTask,
						sizeof(CMD_THERMAL_SENSOR_TASK_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdThermalBasicInfo(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 u1BandIdx
	)
{
	struct cmd_msg *msg;
	CMD_THERMAL_BASIC_INFO_T rThermalBasicInfo;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_THERMAL_BASIC_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rThermalBasicInfo, sizeof(CMD_THERMAL_BASIC_INFO_T));
	rThermalBasicInfo.u1PowerCtrlFormatId = THERMAL_SENSOR_BASIC_INFO_QUERY;
	rThermalBasicInfo.u1BandIdx = u1BandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_THERMAL_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rThermalBasicInfo,
						sizeof(CMD_THERMAL_BASIC_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef GREENAP_SUPPORT
INT32 MtCmdExtGreenAPOnOffCtrl(
	RTMP_ADAPTER *pAd,
	MT_GREENAP_CTRL_T GreenAPCtrl)
{
	struct cmd_msg *msg;
	EXT_CMD_GREENAP_CTRL_T rGreenAPCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GREENAP_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rGreenAPCtrl, sizeof(EXT_CMD_GREENAP_CTRL_T));
	rGreenAPCtrl.ucDbdcIdx = GreenAPCtrl.ucDbdcIdx;
	rGreenAPCtrl.ucGreenAPOn = GreenAPCtrl.ucGreenAPOn;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GREENAP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rGreenAPCtrl, sizeof(EXT_CMD_GREENAP_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 mt_cmd_ext_pcie_aspm_dym_ctrl(
	RTMP_ADAPTER *pAd,
	MT_PCIE_ASPM_DYM_CTRL_T PcieAspmDymCtrl)
{
	struct cmd_msg *msg;
	EXT_CMD_PCIE_ASPM_DYM_CTRL_T pcie_aspm_dym_ctrl = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PCIE_ASPM_DYM_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&pcie_aspm_dym_ctrl, sizeof(EXT_CMD_PCIE_ASPM_DYM_CTRL_T));
	pcie_aspm_dym_ctrl.ucDbdcIdx = PcieAspmDymCtrl.ucDbdcIdx;
	pcie_aspm_dym_ctrl.fgL1Enable = PcieAspmDymCtrl.fgL1Enable;
	pcie_aspm_dym_ctrl.fgL0sEnable = PcieAspmDymCtrl.fgL0sEnable;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PCIE_ASPM_DYM_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&pcie_aspm_dym_ctrl, sizeof(EXT_CMD_PCIE_ASPM_DYM_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 mt_cmd_ext_twt_agrt_update(
	struct _RTMP_ADAPTER *ad,
	struct mt_twt_agrt_para mt_twt_agrt_para)
{
	struct cmd_msg *msg;
	struct ext_cmd_twt_agrt_update ext_cmd_twt_agrt_update = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
#ifdef RT_BIG_ENDIAN
	int i = 0;
#endif

	msg = MtAndesAllocCmdMsg(ad, sizeof(struct ext_cmd_twt_agrt_update));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ext_cmd_twt_agrt_update,
		sizeof(struct ext_cmd_twt_agrt_update));
	os_move_mem((UINT8 *)&ext_cmd_twt_agrt_update,
		(UINT8 *)&mt_twt_agrt_para,
		sizeof(struct mt_twt_agrt_para));
#ifdef RT_BIG_ENDIAN
	ext_cmd_twt_agrt_update.peer_id_grp_id
		= cpu2le16(ext_cmd_twt_agrt_update.peer_id_grp_id);
	ext_cmd_twt_agrt_update.agrt_sp_start_tsf_low
		= cpu2le32(ext_cmd_twt_agrt_update.agrt_sp_start_tsf_low);
	ext_cmd_twt_agrt_update.agrt_sp_start_tsf_high
		= cpu2le32(ext_cmd_twt_agrt_update.agrt_sp_start_tsf_high);
	ext_cmd_twt_agrt_update.agrt_sp_wake_intvl_mantissa
		= cpu2le16(ext_cmd_twt_agrt_update.agrt_sp_wake_intvl_mantissa);
	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++)
		ext_cmd_twt_agrt_update.sta_list[i]
			= cpu2le16(ext_cmd_twt_agrt_update.sta_list[i]);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TWT_AGRT_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg,
		(char *)&ext_cmd_twt_agrt_update,
		sizeof(struct ext_cmd_twt_agrt_update));
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef TPC_SUPPORT
INT32 MtCmdTpcFeatureCtrl(
	RTMP_ADAPTER *pAd,
	INT8 TpcPowerValue,
	UINT8 ucBandIdx,
	UINT8 CentralChannel)
{
	struct cmd_msg *msg;
	CMD_POWER_TPC_CTRL_T TpcMaxPwrCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if (CentralChannel == 0) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(X) invalid Channel setting\n");
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_POWER_TPC_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&TpcMaxPwrCtrl, sizeof(CMD_POWER_TPC_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TpcMaxPwrCtrl.ucPowerCtrlFormatId = TPC_FEATURE_CTRL_V0;
	else
		TpcMaxPwrCtrl.ucPowerCtrlFormatId = TPC_FEATURE_CTRL_V1;
	TpcMaxPwrCtrl.cTPCPowerValue = TpcPowerValue;
	TpcMaxPwrCtrl.ucBand = ucBandIdx;
	TpcMaxPwrCtrl.ucCentralChannel = CentralChannel;
	TpcMaxPwrCtrl.ucChannelBand = TxPowerGetChBand(ucBandIdx, CentralChannel);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "pwr=%d=0x%02X, BandIdx=%d, CentralChannel=%d, ChBand=%d\n",
			  TpcPowerValue, TpcPowerValue, TpcMaxPwrCtrl.ucBand,
			  TpcMaxPwrCtrl.ucCentralChannel, TpcMaxPwrCtrl.ucChannelBand);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&TpcMaxPwrCtrl,
						sizeof(CMD_POWER_TPC_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* TPC_SUPPORT */

INT32 MtCmdATEModeCtrl(
	RTMP_ADAPTER *pAd,
	UCHAR        ATEMode)
{
	struct cmd_msg *msg;
	CMD_ATE_MODE_CTRL_T ATEModeCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ATEMode = %d\n", ATEMode);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_ATE_MODE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ATEModeCtrl, sizeof(CMD_ATE_MODE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		ATEModeCtrl.ucPowerCtrlFormatId = ATEMODE_CTRL_V0;
	else
		ATEModeCtrl.ucPowerCtrlFormatId = ATEMODE_CTRL_V1;
	ATEModeCtrl.fgATEModeEn         = ATEMode;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&ATEModeCtrl,
						sizeof(CMD_ATE_MODE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT32 MtCmdPktBudgetCtrl(struct _RTMP_ADAPTER *pAd, UINT8 bss_idx, UINT16 wcid, UCHAR type)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _CMD_PKT_BUDGET_CTRL_T  pbc;
	struct _CMD_PKT_BUDGET_CTRL_ENTRY_T *entry;
	struct _CMD_ATTRIBUTE attr = {0};
	UCHAR i, j;
	UINT32 size = sizeof(pbc);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "bssid(%d),wcid(%d),type(%d)\n", bss_idx, wcid, type);

	if (type >= PBC_TYPE_END) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "set wrong type (%d) for PBC!\n", type);
		return Ret;
	}

	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PKT_BUDGET_CTRL_CFG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&pbc, size);
	pbc.wlan_idx = cpu2le16(wcid);
	pbc.bss_id = bss_idx;
	pbc.queue_num = PBC_NUM_OF_PKT_BUDGET_CTRL_QUE * DBDC_BAND_NUM;

	switch (type) {
	case PBC_TYPE_NORMAL: {
		for (j = 0; j < DBDC_BAND_NUM; j++) {
			for (i = 0; i < PBC_NUM_OF_PKT_BUDGET_CTRL_QUE; i++) {
				entry = &pbc.aacQue[i + j * PBC_NUM_OF_PKT_BUDGET_CTRL_QUE];
				entry->lower_bound = cpu2le16(PBC_BOUNDARY_RESET_TO_DEFAULT);
				entry->upper_bound = cpu2le16(PBC_BOUNDARY_RESET_TO_DEFAULT);
			}
		}
	}
	break;

	case PBC_TYPE_WMM: {
		for (j = 0; j < DBDC_BAND_NUM; j++) {
			for (i = 0; i < PBC_NUM_OF_PKT_BUDGET_CTRL_QUE; i++) {
				entry = &pbc.aacQue[i + j * PBC_NUM_OF_PKT_BUDGET_CTRL_QUE];
				entry->lower_bound = cpu2le16(PBC_BOUNDARY_RESET_TO_DEFAULT);
				entry->upper_bound = cpu2le16(pAd->pbc_bound[j][i]);
			}
		}
	}
	break;
	}

	MtAndesAppendCmdMsg(msg, (char *)&pbc,
						size);
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: (ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef PS_STA_FLUSH_SUPPORT
INT32 MtCmdPsStaFlushCtrl(struct _RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _CMD_PS_FLUSH_CTRL_T ps_ctrl;
	UINT32 size = sizeof(ps_ctrl);

	msg = MtAndesAllocCmdMsg(pAd, size);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_AP_PWR_SAVING_CAPABILITY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);

	os_zero_mem(&ps_ctrl, size);
	ps_ctrl.fgPsSTAFlushEnable = pAd->MacTab.fPsSTAFlushEnable;
	ps_ctrl.u2FlushThldTotalMsduNum = pAd->MacTab.PsFlushThldTotalMsduNum;
	ps_ctrl.u2PerStaMaxMsduNum = pAd->MacTab.PsFlushPerStaMaxMsduNum;
	MtAndesAppendCmdMsg(msg, (char *)&ps_ctrl, size);

	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"fgPsSTAFlushEnable(%d), u2FlushThldTotalMsduNum(%d), u2PerStaMaxMsduNum(%d)\n",
			ps_ctrl.fgPsSTAFlushEnable, ps_ctrl.u2FlushThldTotalMsduNum, ps_ctrl.u2PerStaMaxMsduNum);

	return Ret;
}
#endif /*PS_STA_FLUSH_SUPPORT*/

#ifdef ZERO_LOSS_CSA_SUPPORT
INT32 MtCmdStaPsQLimit(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT16 PsQLimit)
{
	struct cmd_msg *msg;
	INT32 Ret = 0;
	struct _CMD_STA_PS_Q_LIMIT_T  StaPsLimit;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 size = sizeof(StaPsLimit);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"wcid(%d),PsQLimit(%d)\n", wcid, PsQLimit);

	msg = MtAndesAllocCmdMsg(pAd, size);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		return Ret;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_STA_PS_Q_LIMIT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&StaPsLimit, size);
	StaPsLimit.u2Wcid = cpu2le16(wcid);
	StaPsLimit.u2PsQLimit = cpu2le16(PsQLimit);

	MtAndesAppendCmdMsg(msg, (char *)&StaPsLimit, size);
	Ret = chip_cmd_tx(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", Ret);
	return Ret;
}
#endif /*ZERO_LOSS_CSA_SUPPORT*/

INT32 MtCmdSetBWFEnable(RTMP_ADAPTER *pAd, UINT8 Enable)
{
	struct cmd_msg *msg;
	EXT_CMD_ID_BWF_LWC_ENABLE_T CmdBWFEnable;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	INT32 Len = 0;
	os_zero_mem(&CmdBWFEnable, sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T));
	/* send to N9 */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	Len = sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BWF_LWC_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	CmdBWFEnable.ucBwfLwcEnable = Enable;
	MtAndesAppendCmdMsg(msg, (char *)&CmdBWFEnable, sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "send cmd to N9 CmdBWFEnable.ucBwfLwcEnable [%d] Enable[%d]\n"
			  , CmdBWFEnable.ucBwfLwcEnable, Enable);
	chip_cmd_tx(pAd, msg);
	/* send the same msg to CR4 */
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BWF_LWC_ENABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	CmdBWFEnable.ucBwfLwcEnable = Enable;
	MtAndesAppendCmdMsg(msg, (char *)&CmdBWFEnable, sizeof(EXT_CMD_ID_BWF_LWC_ENABLE_T));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 " send cmd to CR4 CmdBWFEnable.ucBwfLwcEnable [%d] Enable[%d]\n"
			  , CmdBWFEnable.ucBwfLwcEnable, Enable);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, (ret == NDIS_STATUS_SUCCESS) ? DBG_LVL_INFO:DBG_LVL_ERROR,
			 "(ret = %d)\n", ret);
	return ret;
}
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
INT32 MtCmdHotspotInfoUpdate(RTMP_ADAPTER *pAd, MT_HOTSPOT_INFO_UPDATE_T *InfoUpdateT)
{
	struct cmd_msg *msg;
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T CmdHotspotInfoUpdate;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	UINT8 idx = 0;

	if (!IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"skip update to cr4\n");
		return 0;
	}
	os_zero_mem(&CmdHotspotInfoUpdate, sizeof(EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	CmdHotspotInfoUpdate.ucUpdateType = InfoUpdateT->ucUpdateType;
	CmdHotspotInfoUpdate.ucHotspotBssFlags = InfoUpdateT->ucHotspotBssFlags;
	CmdHotspotInfoUpdate.ucHotspotBssId = InfoUpdateT->ucHotspotBssId;

	CmdHotspotInfoUpdate.ucStaQosMapFlagAndIdx = InfoUpdateT->ucStaQosMapFlagAndIdx;
	CmdHotspotInfoUpdate.ucPoolID = InfoUpdateT->ucPoolID;
	CmdHotspotInfoUpdate.ucTableValid = InfoUpdateT->ucTableValid;
	CmdHotspotInfoUpdate.ucPoolDscpExceptionCount = InfoUpdateT->ucPoolDscpExceptionCount;
	CmdHotspotInfoUpdate.u4Ac = InfoUpdateT->u4Ac;

	for (idx = 0; idx < 8; idx++)
		CmdHotspotInfoUpdate.au2PoolDscpRange[idx] = InfoUpdateT->au2PoolDscpRange[idx];

	for (idx = 0; idx < 21; idx++)
		CmdHotspotInfoUpdate.au2PoolDscpException[idx] = InfoUpdateT->au2PoolDscpException[idx];

	WCID_SET_H_L(CmdHotspotInfoUpdate.ucStaWcidHnVer, CmdHotspotInfoUpdate.ucStaWcidL, InfoUpdateT->u2StaWcid);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HOTSPOT_INFO_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdHotspotInfoUpdate, sizeof(EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T));
	/* MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO," send to CR4\n"); */
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#endif /* CONFIG_HOTSPOT_R2 */

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
/*****************************************
 *    ExT_CID = 0x74
 *****************************************/
INT32 MtCmdSetMaxPhyRate(RTMP_ADAPTER *pAd, UINT16 u2MaxPhyRate)
{
	struct cmd_msg *msg;
	CMD_SET_MAX_PHY_RATA CmdSetMaxPhyRate;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Max Phy rate = %d\n", u2MaxPhyRate);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_MAX_PHY_RATA));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CmdSetMaxPhyRate, sizeof(CMD_SET_MAX_PHY_RATA));
	CmdSetMaxPhyRate.u2MaxPhyRate = cpu2le16(u2MaxPhyRate);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_MAX_PHY_RATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdSetMaxPhyRate,
						sizeof(CMD_SET_MAX_PHY_RATA));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */

/*****************************************
 *    Ext_CID = 0x90
 *****************************************/
INT32 MtCmdSetUseVhtRateFor2G(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	CMD_SET_VHT_RATE_FOR_2G CmdSetUseVhtRateFor2G;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_SET_VHT_RATE_FOR_2G));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CmdSetUseVhtRateFor2G, sizeof(CMD_SET_VHT_RATE_FOR_2G));
	CmdSetUseVhtRateFor2G.fgUseVhtRateFor2G = pAd->CommonCfg.bUseVhtRateFor2g;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Use VHT Rate for 2G = %d\n",
			 CmdSetUseVhtRateFor2G.fgUseVhtRateFor2G);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_USE_VHTRATE_FOR_2G);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CmdSetUseVhtRateFor2G,
						sizeof(CMD_SET_VHT_RATE_FOR_2G));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 MtCmdLinkTestTxCsdCtrl(
		RTMP_ADAPTER	*pAd,
		BOOLEAN	fgTxCsdConfigEn,
		UINT8	ucDbdcBandIdx,
		UINT8	ucBandIdx)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_TX_CSD_CTRL_T CMWTxCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgTxCsdConfigEn = %d\n", fgTxCsdConfigEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_TX_CSD_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWTxCtrl, sizeof(CMD_LINK_TEST_TX_CSD_CTRL_T));

	CMWTxCtrl.ucLinkTestCtrlFormatId = LINK_TEST_TX_CSD;
	CMWTxCtrl.fgTxCsdConfigEn = fgTxCsdConfigEn;
	CMWTxCtrl.ucDbdcBandIdx = ucDbdcBandIdx;
	CMWTxCtrl.ucBandIdx = ucBandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWTxCtrl,
			sizeof(CMD_LINK_TEST_TX_CSD_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestRxCtrl(
		RTMP_ADAPTER	*pAd,
		UINT8	ucRxAntIdx,
		UINT8	ucBandIdx)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_RX_CTRL_T CMWRxCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ucRxAntIdx: %d, ucBandIdx: %d\n", ucRxAntIdx, ucBandIdx);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_RX_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWRxCtrl, sizeof(CMD_LINK_TEST_RX_CTRL_T));

	CMWRxCtrl.ucLinkTestCtrlFormatId = LINK_TEST_RX;
	CMWRxCtrl.ucRxAntIdx = ucRxAntIdx;
	CMWRxCtrl.ucBandIdx  = ucBandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWRxCtrl,
			sizeof(CMD_LINK_TEST_RX_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestTxPwrCtrl(
		RTMP_ADAPTER	*pAd,
		BOOLEAN	fgTxPwrConfigEn,
		UINT8	ucDbdcBandIdx,
		UINT8	ucBandIdx)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_TXPWR_CTRL_T CMWTxPwrCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgTxPwrConfigEn = %d\n", fgTxPwrConfigEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_TXPWR_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWTxPwrCtrl, sizeof(CMD_LINK_TEST_TXPWR_CTRL_T));

	CMWTxPwrCtrl.ucLinkTestCtrlFormatId = LINK_TEST_TXPWR;
	CMWTxPwrCtrl.fgTxPwrConfigEn = fgTxPwrConfigEn;
	CMWTxPwrCtrl.ucDbdcBandIdx = ucDbdcBandIdx;
	CMWTxPwrCtrl.ucBandIdx = ucBandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWTxPwrCtrl,
			sizeof(CMD_LINK_TEST_TXPWR_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestTxPwrUpTblCtrl(
		RTMP_ADAPTER	*pAd,
		UINT8	ucTxPwrUpCat,
		PUINT8	pucTxPwrUpValue)
{
	UINT8 ucRateIdx;
	struct cmd_msg *msg;
	CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T CMWTxPwrUpTblCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucTxPwrUpCat: %d\n", ucTxPwrUpCat);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucTxPwrUpRate: ");
	for (ucRateIdx = 0; ucRateIdx < CMW_POWER_UP_RATE_NUM; ucRateIdx++)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " %d", *(pucTxPwrUpValue + ucRateIdx));

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWTxPwrUpTblCtrl, sizeof(CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T));

	CMWTxPwrUpTblCtrl.ucLinkTestCtrlFormatId = LINK_TEST_TXPWR_UP_TABLE;
	CMWTxPwrUpTblCtrl.ucTxPwrUpCat = ucTxPwrUpCat;

	/* update command content with Tx Power up Table */
	os_move_mem(&(CMWTxPwrUpTblCtrl.ucTxPwrUpValue), pucTxPwrUpValue, CMW_POWER_UP_RATE_NUM);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWTxPwrUpTblCtrl,
			sizeof(CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestACRCtrl(
		RTMP_ADAPTER	*pAd,
		BOOLEAN	fgACRConfigEn,
		UINT8	ucDbdcBandIdx)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_ACR_CTRL_T CMWACICtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgACIConfigEn = %d\n", fgACRConfigEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_ACR_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWACICtrl, sizeof(CMD_LINK_TEST_ACR_CTRL_T));

	CMWACICtrl.ucLinkTestCtrlFormatId = LINK_TEST_ACR;
	CMWACICtrl.fgACRConfigEn = fgACRConfigEn;
	CMWACICtrl.ucDbdcBandIdx = ucDbdcBandIdx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWACICtrl,
			sizeof(CMD_LINK_TEST_ACR_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestRcpiCtrl(
		RTMP_ADAPTER	*pAd,
		BOOLEAN	fgRCPIConfigEn)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_RCPI_CTRL_T CMWRCPICtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgRCPIConfigEn = %d\n", fgRCPIConfigEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_RCPI_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWRCPICtrl, sizeof(CMD_LINK_TEST_RCPI_CTRL_T));

	CMWRCPICtrl.ucLinkTestCtrlFormatId = LINK_TEST_RCPI;
	CMWRCPICtrl.fgRCPIConfigEn = fgRCPIConfigEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWRCPICtrl,
			sizeof(CMD_LINK_TEST_RCPI_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestSeIdxCtrl(
		RTMP_ADAPTER *pAd,
		BOOLEAN fgSeIdxConfigEn)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_SEIDX_CTRL_T CMWSeIdxCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgSeIdxConfigEn = %d\n", fgSeIdxConfigEn);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_SEIDX_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWSeIdxCtrl, sizeof(CMD_LINK_TEST_SEIDX_CTRL_T));

	CMWSeIdxCtrl.ucLinkTestCtrlFormatId = LINK_TEST_SEIDX;
	CMWSeIdxCtrl.fgSeIdxConfigEn = fgSeIdxConfigEn;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWSeIdxCtrl,
			sizeof(CMD_LINK_TEST_SEIDX_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

INT32 MtCmdLinkTestRcpiMACtrl(
		RTMP_ADAPTER	*pAd,
		UINT8	ucMAParameter)
{
	struct cmd_msg *msg;
	CMD_LINK_TEST_RCPI_MA_CTRL_T CMWRcpiMACtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ucMAParameter = %d\n", ucMAParameter);

	msg = MtAndesAllocCmdMsg(pAd, sizeof(CMD_LINK_TEST_RCPI_MA_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&CMWRcpiMACtrl, sizeof(CMD_LINK_TEST_RCPI_MA_CTRL_T));

	/* Transform the parameter to bitwise represetation */
	switch (ucMAParameter) {
	case 1:
		ucMAParameter = 3;
		break;

	case 2:
		ucMAParameter = 2;
		break;

	case 4:
		ucMAParameter = 1;
		break;

	case 8:
		ucMAParameter = 0;
		break;

	default:
		break;
	}

	CMWRcpiMACtrl.ucLinkTestCtrlFormatId = LINK_TEST_RCPI_MA;
	CMWRcpiMACtrl.ucMAParameter = ucMAParameter;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LINK_TEST_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&CMWRcpiMACtrl,
			sizeof(CMD_LINK_TEST_RCPI_MA_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"(ret = %d)\n", ret);

	return ret;
}

/*****************************************
 *	ExT_CID = 0x94
 *****************************************/
INT32 MtCmdPhyShapingFilterDisable(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_PHY_SHAPING_FILTER_DISABLE_T phyConfig;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: phyShapingFilterDisable = %d\n", __func__, PHY_SHAPING_FILTER_DISABLE);
	msg = MtAndesAllocCmdMsg(pAd, sizeof(phyConfig));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SHAPING_FILTER_DISABLE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	MtAndesInitCmdMsg(msg, attr);
	os_zero_mem(&phyConfig, sizeof(phyConfig));
	phyConfig.ucPhyShapingFilterDisable = PHY_SHAPING_FILTER_DISABLE;
	MtAndesAppendCmdMsg(msg, (char *)&phyConfig, sizeof(phyConfig));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT ShowHeraRuRaInfoProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = HERA_RA_RU_INFO_CMD;
	CMD_GET_RU_RA_INFO param = {0};

	PCHAR pch = NULL;
	PCHAR pWlanIdx = NULL;
	PCHAR pRuIdx = NULL;
	PCHAR pDirection = NULL;
	PCHAR pDumpGroup = NULL;

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pWlanIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if  (pch != NULL)
		pRuIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pDirection = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL)
		pDumpGroup = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u2WlanIdx   = simple_strtol(pWlanIdx, 0, 10);
	param.u2RuIdx     = simple_strtol(pRuIdx, 0, 10);
	param.u2Direction = simple_strtol(pDirection, 0, 10);
	param.u2DumpGroup = simple_strtol(pDumpGroup, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "WCID: %d RuIdx: %d Direction: %d DumpGroup: %d\n",
				param.u2WlanIdx, param.u2RuIdx, param.u2Direction, param.u2DumpGroup);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u2WlanIdx = cpu2le16(param.u2WlanIdx);
	param.u2RuIdx = cpu2le16(param.u2RuIdx);
	param.u2Direction = cpu2le16(param.u2Direction);
	param.u2DumpGroup = cpu2le16(param.u2DumpGroup);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT SetHeraIara_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4Value;
	UINT32 u4cmd = HERA_CFG_FLAG_CMD;

	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Null Parameters\n");
		return FALSE;
	}

	u4Value = os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enable=%d \n", u4Value);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4cmd) + sizeof(u4Value));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		u4cmd = cpu2le32(u4cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4cmd, sizeof(u4cmd));
	AndesAppendCmdMsg(msg, (char *)&u4Value, sizeof(u4Value));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT SetHeraOptionDyncBW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4cmd = HERA_OPTION_CMD;
	UINT8 u1Value;
	CMD_RA_OPTION_CTRL_T param = {0};

	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: Null Parameters\n", __func__);
		return FALSE;
	}

	u1Value = os_str_tol(arg, 0, 10);

	param.u1Value = u1Value;
	param.u1OptionType = RA_CTRL_OPTION_DYNAMIC_BW;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: Enable=%d\n", __func__, u1Value);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4cmd) + sizeof(param));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		u4cmd = cpu2le32(u4cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4cmd, sizeof(u4cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT SetHeraOptionFrequecyDup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4cmd = HERA_OPTION_CMD;
	UINT8 u1Value;
	CMD_RA_OPTION_CTRL_T param = {0};

	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Null Parameters\n");
		return FALSE;
	}

	u1Value = os_str_tol(arg, 0, 10);

	param.u1Value = u1Value;
	param.u1OptionType = RA_CTRL_OPTION_LEGACY_FREQ_DUP;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Enable=%d \n", u1Value);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4cmd) + sizeof(param));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		u4cmd = cpu2le32(u4cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4cmd, sizeof(u4cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT ShowHeraMuRaInfoProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = HERA_MU_RA_INFO_CMD;
	CMD_GET_MU_RA_INFO param = {0};

	PCHAR pch = NULL;
	PCHAR pMuGroupIdx = NULL;
	PCHAR pUserIdx = NULL;
	PCHAR pDirection = NULL;
	PCHAR pDumpGroup = NULL;

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pMuGroupIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if  (pch != NULL)
		pUserIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pDirection = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL)
		pDumpGroup = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u2MuGroupdx = simple_strtol(pMuGroupIdx, 0, 10);
	param.u2UserIdx   = simple_strtol(pUserIdx, 0, 10);
	param.u2Direction = simple_strtol(pDirection, 0, 10);
	param.u2DumpGroup = simple_strtol(pDumpGroup, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "MuGroupIdx: %d UserIdx: %d Direction: %d DumpGroup: %d\n",
				param.u2MuGroupdx, param.u2UserIdx, param.u2Direction, param.u2DumpGroup);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u2MuGroupdx = cpu2le16(param.u2MuGroupdx);
	param.u2UserIdx = cpu2le16(param.u2UserIdx);
	param.u2Direction = cpu2le16(param.u2Direction);
	param.u2DumpGroup = cpu2le16(param.u2DumpGroup);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT32 CmdHeraStbcPriorityCtrl(
	PRTMP_ADAPTER pAd,
	PUINT8 pucData)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4HeraCmdType = HERA_STBC_PRIORITY_CMD;
	P_CMD_HERA_STBC_PRIORITY_T prHeraStbcPriority = (P_CMD_HERA_STBC_PRIORITY_T)pucData;

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"u1BandIdx=%u, u1Operation=%u, u1StbcPriority=%u\n",
			prHeraStbcPriority->u1BandIdx,
			prHeraStbcPriority->u1Operation,
			prHeraStbcPriority->u1StbcPriority);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4HeraCmdType) + sizeof(CMD_HERA_STBC_PRIORITY_T));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	u4HeraCmdType = cpu2le32(u4HeraCmdType);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4HeraCmdType, sizeof(u4HeraCmdType));
	AndesAppendCmdMsg(msg, (char *)prHeraStbcPriority, sizeof(CMD_HERA_STBC_PRIORITY_T));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT32 MtCmdSetVht1024QamSupport(
	PRTMP_ADAPTER pAd)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4HeraCmdType = HERA_VHT_1024QAM_CMD;
	CMD_SET_VHT_1024_QAM_T rHeraSetVht1024Qam;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4HeraCmdType) + sizeof(rHeraSetVht1024Qam));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	os_zero_mem(&rHeraSetVht1024Qam, sizeof(rHeraSetVht1024Qam));
	rHeraSetVht1024Qam.fgVht1024QamSupport = pAd->CommonCfg.vht_1024_qam;

	MTWF_DBG(pAd, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Vht1024QamSupport=%u\n",
			rHeraSetVht1024Qam.fgVht1024QamSupport);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	u4HeraCmdType = cpu2le32(u4HeraCmdType);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4HeraCmdType, sizeof(u4HeraCmdType));
	AndesAppendCmdMsg(msg, (char *)&rHeraSetVht1024Qam, sizeof(rHeraSetVht1024Qam));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

INT SetHeraProtectionPerPpduDis(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = HERA_CFG_PTEC_PER_PPDU_CMD;
	CMD_CFG_PTEC_PER_PPDU_T param;

	os_zero_mem(&param, sizeof(param));

	pch = strsep(&arg, ":");
	if (pch != NULL) {
		param.u1BandIdx = os_str_toul(pch, 0, 10);
		/* sanity check for Band index */
		if (param.u1BandIdx >= DBDC_BAND_NUM) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Invalid Band Index !!\n");
			Ret = 0;
			goto error;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invalid Band Index !!\n");
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL) {
		param.fgPtecPerPpduDis = os_str_toul(pch, 0, 10) ? 1 : 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Empty ProtectionPerPpduDis !!\n");
		Ret = 0;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"BandIdx:%d ProtectionPerPpduDis:%d !!\n",
			param.u1BandIdx, param.fgPtecPerPpduDis);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
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

INT SetHeraMuInitRateInterval(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = HERA_CFG_MU_INIT_RATE_INTV_CMD;
	CMD_CFG_MU_INIT_RATE_INTV_T param;

	os_zero_mem(&param, sizeof(param));

	pch = strsep(&arg, "");
	if (pch != NULL) {
		param.u1IntvInUnit50Ms = os_str_toul(pch, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Empty u1IntvInUnit50Ms !!\n");
		Ret = 0;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"u1IntvInUnit50Ms:%d !!\n",
			param.u1IntvInUnit50Ms);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
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
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", Ret);

	return Ret;
}

INT SetHeraMuDisableSwitchSu(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = HERA_CFG_MU_DIS_SWITCH_SU_CMD;
	CMD_CFG_MU_DIS_SWITCH_SU_T param;

	os_zero_mem(&param, sizeof(param));

	pch = strsep(&arg, "");
	if (pch != NULL) {
		param.fgDisSwitchSU = os_str_toul(pch, 0, 10) ? 1 : 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Empty fgDisSwitchSU !!\n");
		Ret = 0;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"fgDisSwitchSU:%d !!\n",
			param.fgDisSwitchSU);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
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
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", Ret);

	return Ret;
}

INT SetHeraSingleNssTxEnable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = HERA_SINGLE_NSS_TX_EN_CMD;
	struct _CMD_CFG_SINGLE_NSS_TX_EN_T param;

	os_zero_mem(&param, sizeof(param));

	pch = strsep(&arg, ":");
	if (pch != NULL) {
		param.u1BandIdx = os_str_toul(pch, 0, 10);
		/* sanity check for Band index */
		if (param.u1BandIdx >= DBDC_BAND_NUM) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Invalid Band Index !!\n");
			Ret = 0;
			goto error;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invalid Band Index !!\n");
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL) {
		param.fgSingleNssTxEn = os_str_toul(pch, 0, 10) ? 1 : 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Empty fgSingleNssTxEn !!\n");
		Ret = 0;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"BandIdx:%d fgSingleNssTxEn:%d !!\n",
			param.u1BandIdx, param.fgSingleNssTxEn);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
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

INT32 mt_cmd_set_rdd_ipi_hist(
	PRTMP_ADAPTER pAd,
	P_EXT_CMD_RDD_IPI_HIST_T p_cmd_rdd_ipi_hist)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RDD_IPI_HIST_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDD_IPI_HIST_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)p_cmd_rdd_ipi_hist,
						sizeof(EXT_CMD_RDD_IPI_HIST_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

static VOID mt_cmd_get_rdd_ipi_hist_rsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_RDD_IPI_HIST pr_evt_ext_cmd_rlt = (P_EXT_EVENT_RDD_IPI_HIST)Data;
	P_EXT_EVENT_RDD_IPI_HIST pr_evt_rdd_ipi_hist = (P_EXT_EVENT_RDD_IPI_HIST)msg->attr.rsp.wb_buf_in_calbk;
#ifdef RT_BIG_ENDIAN
	int i = 0;
#endif

	/* Update ipi_hist */
	os_move_mem(pr_evt_rdd_ipi_hist, pr_evt_ext_cmd_rlt, sizeof(EXT_EVENT_RDD_IPI_HIST));

#ifdef RT_BIG_ENDIAN
	for (i = 0; i < IPI_HIST_TYPE_NUM; i++)
		pr_evt_rdd_ipi_hist->ipi_hist_val[i]
			= le2cpu32(pr_evt_rdd_ipi_hist->ipi_hist_val[i]);
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ipi_hist_idx: 0x%x\n",
		pr_evt_rdd_ipi_hist->ipi_hist_idx);
}

INT32 mt_cmd_get_rdd_ipi_hist(
	PRTMP_ADAPTER pAd,
	UINT8 rdd_ipi_hist_idx,
	P_EXT_EVENT_RDD_IPI_HIST p_rdd_ipi_hist_rlt)
{
	struct cmd_msg *msg;
	EXT_CMD_RDD_IPI_HIST_T rdd_ipi_hist_cmd;

	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	os_zero_mem(&rdd_ipi_hist_cmd, sizeof(EXT_CMD_RDD_IPI_HIST_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RDD_IPI_HIST_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDD_IPI_HIST_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_RDD_IPI_HIST));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, p_rdd_ipi_hist_rlt);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rdd_ipi_hist_rsp);
	MtAndesInitCmdMsg(msg, attr);

	rdd_ipi_hist_cmd.ipi_hist_idx = rdd_ipi_hist_idx;
	rdd_ipi_hist_cmd.band_idx = p_rdd_ipi_hist_rlt->band_idx;
	MtAndesAppendCmdMsg(msg, (char *)&rdd_ipi_hist_cmd, sizeof(EXT_CMD_RDD_IPI_HIST_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef IPI_SCAN_SUPPORT
INT32 mt_cmd_set_rdd_ipi_scan(
	PRTMP_ADAPTER pAd,
	P_EXT_CMD_RDD_IPI_SCAN_T p_cmd_rdd_ipi_scan)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RDD_IPI_SCAN_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDD_IPI_SCAN_HIST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)p_cmd_rdd_ipi_scan,
						sizeof(EXT_CMD_RDD_IPI_SCAN_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

static VOID mt_cmd_get_rdd_ipi_scan_rsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_RDD_IPI_SCAN pr_evt_ext_cmd_ipi = (P_EXT_EVENT_RDD_IPI_SCAN)Data;
	P_EXT_EVENT_RDD_IPI_SCAN pr_evt_rdd_ipi_scan = (P_EXT_EVENT_RDD_IPI_SCAN)msg->attr.rsp.wb_buf_in_calbk;
#ifdef RT_BIG_ENDIAN
	int i = 0;
#endif

	/* Update ipi_hist */
	os_move_mem(pr_evt_rdd_ipi_scan, pr_evt_ext_cmd_ipi, sizeof(EXT_EVENT_RDD_IPI_SCAN));

#ifdef RT_BIG_ENDIAN
	for (i = 0; i < PWR_INDICATE_HIST_MAX; i++)
		pr_evt_rdd_ipi_scan->ipi_hist_val[i]
			= le2cpu32(pr_evt_rdd_ipi_scan->ipi_hist_val[i]);
#endif
}

INT32 mt_cmd_get_rdd_ipi_scan(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_IPI_SCAN p_rdd_ipi_hist_rlt)
{
	struct cmd_msg *msg;
	EXT_CMD_RDD_IPI_SCAN_T rdd_ipi_scan_cmd;

	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;
	os_zero_mem(&rdd_ipi_scan_cmd, sizeof(EXT_CMD_RDD_IPI_SCAN_T));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_RDD_IPI_SCAN_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RDD_IPI_SCAN_HIST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_RDD_IPI_SCAN));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, p_rdd_ipi_hist_rlt);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rdd_ipi_scan_rsp);
	MtAndesInitCmdMsg(msg, attr);

	rdd_ipi_scan_cmd.u1mode = 0;

	MtAndesAppendCmdMsg(msg, (char *)&rdd_ipi_scan_cmd, sizeof(EXT_CMD_RDD_IPI_SCAN_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif
static VOID mt_cmd_get_rx_stat_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
	P_TESTMODE_STATISTIC_INFO pr_get_rx_stat_rlt = (P_TESTMODE_STATISTIC_INFO)Data;
	P_TESTMODE_STATISTIC_INFO pr_get_rx_stat = (P_TESTMODE_STATISTIC_INFO)msg->attr.rsp.wb_buf_in_calbk;
#ifdef RT_BIG_ENDIAN
	int i = 0;
#endif

	/* Update rx stat */
	os_move_mem(pr_get_rx_stat, pr_get_rx_stat_rlt, sizeof(TESTMODE_STATISTIC_INFO));
#ifdef RT_BIG_ENDIAN
	pr_get_rx_stat->mac_rx_fcs_err_cnt
		= le2cpu16(pr_get_rx_stat->mac_rx_fcs_err_cnt);
	pr_get_rx_stat->mac_rx_len_mismatch
		= le2cpu16(pr_get_rx_stat->mac_rx_len_mismatch);
	pr_get_rx_stat->mac_rx_fcs_ok_cnt
		= le2cpu16(pr_get_rx_stat->mac_rx_fcs_ok_cnt);
	pr_get_rx_stat->mac_rx_fifo_full
		= le2cpu16(pr_get_rx_stat->mac_rx_fifo_full);
	pr_get_rx_stat->mac_rx_mdrdy_cnt
		= le2cpu32(pr_get_rx_stat->mac_rx_mdrdy_cnt);
	pr_get_rx_stat->phy_rx_fcs_err_cnt_cck
		= le2cpu16(pr_get_rx_stat->phy_rx_fcs_err_cnt_cck);
	pr_get_rx_stat->phy_rx_fcs_err_cnt_ofdm
		= le2cpu16(pr_get_rx_stat->phy_rx_fcs_err_cnt_ofdm);
	pr_get_rx_stat->phy_rx_pd_cck
		= le2cpu16(pr_get_rx_stat->phy_rx_pd_cck);
	pr_get_rx_stat->phy_rx_pd_ofdm
		= le2cpu16(pr_get_rx_stat->phy_rx_pd_ofdm);
	pr_get_rx_stat->phy_rx_sig_err_cck
		= le2cpu16(pr_get_rx_stat->phy_rx_sig_err_cck);
	pr_get_rx_stat->phy_rx_sfd_err_cck
		= le2cpu16(pr_get_rx_stat->phy_rx_sfd_err_cck);
	pr_get_rx_stat->phy_rx_sig_err_ofdm
		= le2cpu16(pr_get_rx_stat->phy_rx_sig_err_ofdm);
	pr_get_rx_stat->phy_rx_tag_err_ofdm
		= le2cpu16(pr_get_rx_stat->phy_rx_tag_err_ofdm);
	pr_get_rx_stat->phy_rx_mdrdy_cnt_cck
		= le2cpu16(pr_get_rx_stat->phy_rx_mdrdy_cnt_cck);
	pr_get_rx_stat->phy_rx_mdrdy_cnt_ofdm
		= le2cpu16(pr_get_rx_stat->phy_rx_mdrdy_cnt_ofdm);
	pr_get_rx_stat->aci_hit_low
		= le2cpu32(pr_get_rx_stat->aci_hit_low);
	pr_get_rx_stat->aci_hit_high
		= le2cpu32(pr_get_rx_stat->aci_hit_high);

	for (i = 0; i < 4; i++) {
		pr_get_rx_stat->rcpi[i] = le2cpu16(pr_get_rx_stat->rcpi[i]);
		pr_get_rx_stat->rssi[i] = le2cpu16(pr_get_rx_stat->rssi[i]);
		pr_get_rx_stat->snr[i] = le2cpu16(pr_get_rx_stat->snr[i]);
	}
#endif
}

static VOID mt_cmd_get_rx_stat_band_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
#ifdef RT_BIG_ENDIAN
	P_TESTMODE_STATISTIC_INFO_BAND dst
		= (P_TESTMODE_STATISTIC_INFO_BAND)msg->attr.rsp.wb_buf_in_calbk;
#endif

	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(TESTMODE_STATISTIC_INFO_BAND));
#ifdef RT_BIG_ENDIAN
	dst->mac_rx_fcs_err_cnt = le2cpu16(dst->mac_rx_fcs_err_cnt);
	dst->mac_rx_len_mismatch = le2cpu16(dst->mac_rx_len_mismatch);
	dst->mac_rx_fcs_ok_cnt = le2cpu16(dst->mac_rx_fcs_ok_cnt);
	dst->mac_rx_mdrdy_cnt = le2cpu32(dst->mac_rx_mdrdy_cnt);
	dst->phy_rx_fcs_err_cnt_cck = le2cpu16(dst->phy_rx_fcs_err_cnt_cck);
	dst->phy_rx_fcs_err_cnt_ofdm = le2cpu16(dst->phy_rx_fcs_err_cnt_ofdm);
	dst->phy_rx_pd_cck = le2cpu16(dst->phy_rx_pd_cck);
	dst->phy_rx_pd_ofdm = le2cpu16(dst->phy_rx_pd_ofdm);
	dst->phy_rx_sig_err_cck = le2cpu16(dst->phy_rx_sig_err_cck);
	dst->phy_rx_sfd_err_cck = le2cpu16(dst->phy_rx_sfd_err_cck);
	dst->phy_rx_sig_err_ofdm = le2cpu16(dst->phy_rx_sig_err_ofdm);
	dst->phy_rx_tag_err_ofdm = le2cpu16(dst->phy_rx_tag_err_ofdm);
	dst->phy_rx_mdrdy_cnt_cck = le2cpu16(dst->phy_rx_mdrdy_cnt_cck);
	dst->phy_rx_mdrdy_cnt_ofdm = le2cpu16(dst->phy_rx_mdrdy_cnt_ofdm);
#endif
}

static VOID mt_cmd_get_rx_stat_path_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(TESTMODE_STATISTIC_INFO_PATH));
}

static VOID mt_cmd_get_rx_stat_user_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(TESTMODE_STATISTIC_INFO_USER));
}

static VOID mt_cmd_get_rx_stat_comm_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
#ifdef RT_BIG_ENDIAN
	P_TESTMODE_STATISTIC_INFO_COMM dst
		= (P_TESTMODE_STATISTIC_INFO_COMM)msg->attr.rsp.wb_buf_in_calbk;
#endif

	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, Data, sizeof(TESTMODE_STATISTIC_INFO_COMM));
#ifdef RT_BIG_ENDIAN
	dst->mac_rx_fifo_full = le2cpu16(dst->mac_rx_fifo_full);
	dst->aci_hit_low = le2cpu32(dst->aci_hit_low);
	dst->aci_hit_high = le2cpu32(dst->aci_hit_high);
#endif
}

INT32 mt_cmd_get_rx_stat(
	PRTMP_ADAPTER pAd,
	UCHAR band_idx,
	P_TESTMODE_STATISTIC_INFO p_rx_stat_rlt)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RX_STATISTIC_INFO rx_stat_cmd;

	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	os_zero_mem(&rx_stat_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_GET_RX_STAT;
	rx_stat_cmd.band_idx = band_idx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(TESTMODE_STATISTIC_INFO));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, p_rx_stat_rlt);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rx_stat_rsp);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO));
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_get_rx_stat_band(
	PRTMP_ADAPTER pAd,
	UCHAR band_idx,
	TESTMODE_STATISTIC_INFO_BAND *rx_stat_band)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RX_STATISTIC_INFO_BAND rx_stat_band_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	os_zero_mem(&rx_stat_band_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_BAND));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_BAND));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_band_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_GET_RX_STAT_BAND;
	rx_stat_band_cmd.band_idx = band_idx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(TESTMODE_STATISTIC_INFO_BAND));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rx_stat_band);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rx_stat_band_rsp);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_band_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_BAND));
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_get_rx_stat_path(
	PRTMP_ADAPTER pAd,
	UCHAR path_idx,
	UCHAR band_idx,
	TESTMODE_STATISTIC_INFO_PATH *rx_stat_path)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RX_STATISTIC_INFO_PATH rx_stat_path_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	os_zero_mem(&rx_stat_path_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_PATH));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_PATH));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_path_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_GET_RX_STAT_PATH;
	rx_stat_path_cmd.path_idx = path_idx;
	rx_stat_path_cmd.band_idx = band_idx;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(TESTMODE_STATISTIC_INFO_PATH));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rx_stat_path);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rx_stat_path_rsp);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_path_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_PATH));
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_get_rx_stat_user(
	PRTMP_ADAPTER pAd,
	UCHAR user_idx,
	TESTMODE_STATISTIC_INFO_USER *rx_stat_user)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RX_STATISTIC_INFO_USER rx_stat_user_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	os_zero_mem(&rx_stat_user_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_USER));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_USER));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_user_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_GET_RX_STAT_USER;
	rx_stat_user_cmd.user_idx = user_idx;
#ifdef RT_BIG_ENDIAN
	rx_stat_user_cmd.user_idx = cpu2le16(rx_stat_user_cmd.user_idx);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(TESTMODE_STATISTIC_INFO_USER));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rx_stat_user);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rx_stat_user_rsp);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_user_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_USER));
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_get_rx_stat_comm(
	PRTMP_ADAPTER pAd,
	TESTMODE_STATISTIC_INFO_COMM *rx_stat_comm)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_RX_STATISTIC_INFO_COMM rx_stat_comm_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	os_zero_mem(&rx_stat_comm_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_COMM));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_COMM));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_comm_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_GET_RX_STAT_COMM;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(TESTMODE_STATISTIC_INFO_COMM));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rx_stat_comm);
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_rx_stat_comm_rsp);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_comm_cmd, sizeof(EXT_CMD_GET_RX_STATISTIC_INFO_COMM));
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_set_rx_stat_user_idx(
	PRTMP_ADAPTER pAd,
	UCHAR band_idx,
	UINT16 user_idx)
{
	struct cmd_msg *msg;
	EXT_CMD_SET_RX_STAT_USER rx_stat_user_idx;

	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx: %d, user_idx: 0x%x\n", band_idx, user_idx);
	os_zero_mem(&rx_stat_user_idx, sizeof(EXT_CMD_SET_RX_STAT_USER));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_SET_RX_STAT_USER));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rx_stat_user_idx.band_idx = band_idx;
	rx_stat_user_idx.user_idx = user_idx;
#ifdef RT_BIG_ENDIAN
	rx_stat_user_idx.user_idx = cpu2le16(rx_stat_user_idx.user_idx);
#endif

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_USER_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, (char *)&rx_stat_user_idx, sizeof(EXT_CMD_SET_RX_STAT_USER));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_set_rxv_ctrl(
	RTMP_ADAPTER *pAd,
	BOOLEAN fgRxvEnable
)
{
	struct cmd_msg *msg;
	EXT_CMD_TESTMODE_RXV_CTRL rxv_ctrl_cmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "fgRxvEnable: %d\n", fgRxvEnable);

	os_zero_mem(&rxv_ctrl_cmd, sizeof(EXT_CMD_TESTMODE_RXV_CTRL));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TESTMODE_RXV_CTRL));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rxv_ctrl_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_SET_RXV_CTRL;
	rxv_ctrl_cmd.fgRxvEnable = fgRxvEnable;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rxv_ctrl_cmd,
						sizeof(EXT_CMD_TESTMODE_RXV_CTRL));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 mt_cmd_set_rxv_ru_ctrl(
	RTMP_ADAPTER *pAd,
	UINT8 rxv_ru_idx
)
{
	struct cmd_msg *msg;
	EXT_CMD_TESTMODE_RXV_RU_CTRL rxv_ru_ctrl_cmd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s: rxv_ru_idx: %d\n", __func__, rxv_ru_idx);

	os_zero_mem(&rxv_ru_ctrl_cmd, sizeof(EXT_CMD_TESTMODE_RXV_RU_CTRL));
	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TESTMODE_RXV_RU_CTRL));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	rxv_ru_ctrl_cmd.u1RxvCtrlFormatId = TESTMODE_RXV_CMD_SET_RXV_RU_CTRL;
	rxv_ru_ctrl_cmd.u1RxvRuIdx = rxv_ru_idx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);
	MtAndesAppendCmdMsg(msg, (char *)&rxv_ru_ctrl_cmd,
						sizeof(EXT_CMD_TESTMODE_RXV_RU_CTRL));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

struct _EC_GROUP_LENGTH_MAP_T ec_group_map[] = {
	{ECDH_GROUP_ID_256BIT, ECDH_LENGTH_256BIT},
	{ECDH_GROUP_ID_384BIT, ECDH_LENGTH_384BIT},
	{ECDH_GROUP_ID_521BIT, ECDH_LENGTH_521BIT},
	{ECDH_GROUP_ID_192BIT, ECDH_LENGTH_192BIT},
	{ECDH_GROUP_ID_224BIT, ECDH_LENGTH_224BIT},
	{0, 0}
};

#define SIZE_EC_GROUP_MAP   (sizeof(ec_group_map) / sizeof(struct _EC_GROUP_LENGTH_MAP_T))

/*****************************************
 *    ExT_CID = 0x9C
 *****************************************/
INT32 cmd_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0, i, element_len = 0, cal_mode = ECC_CAL_DG_MODE;
	UINT32 offset = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _CMD_ECC_OP_T *ecc_op = NULL;
	static UINT8 ecc_cmd_id;

	size = sizeof(CMD_ECC_OP_T);
	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"oper = %d, group = %d\n", oper, group);

	for (i = 0; i < SIZE_EC_GROUP_MAP; i++) {
		if (group == ec_group_map[i].group_id) {
			element_len = ec_group_map[i].element_len;
			break;
		}

		if (ec_group_map[i].group_id == 0) {
			ret = NDIS_STATUS_INVALID_DATA;
			goto error;
		}
	}

	if (scalar)
		size += element_len;
	else {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	if ((point_x) && (point_y)) {
		size += (element_len * 2);
		cal_mode = ECC_CAL_DQ_MODE;
	} else if (((point_x) && (point_y == NULL)) ||
		((point_x == NULL) && (point_y))) {
		/*we don't support pass x or y coordinate only. need to pass whole x & y coordingnates.*/
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	msg = MtAndesAllocCmdMsg(ad, size);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_alloc_mem(ad, (UCHAR **)&ecc_op, size);

	if (!ecc_op) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ECC_OPER);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	ecc_op->eEccOperation = ECC_OP_CAL_GROUP_POINT;
	ecc_op->ucGroupID = group;
	ecc_op->ucDataLength = size;
	ecc_op->ucDataType = cal_mode;
	ecc_op->ucEccCmdId = ecc_cmd_id;
	ecc_op->u2CmdLen = cpu2le16(sizeof(struct _CMD_ECC_OP_T) + ecc_op->ucDataLength);
	ecc_cmd_id++;

	NdisMoveMemory(&(ecc_op->aucBuffer[offset]), scalar, element_len);

	if (cal_mode == ECC_CAL_DQ_MODE) {
		offset += element_len;
		NdisMoveMemory(&(ecc_op->aucBuffer[offset]), point_x, element_len);
		offset += element_len;
		NdisMoveMemory(&(ecc_op->aucBuffer[offset]), point_y, element_len);
	}

	MtAndesAppendCmdMsg(msg, (char *)ecc_op, size);
	ret = chip_cmd_tx(ad, msg);
	os_free_mem(ecc_op);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#ifdef VLAN_SUPPORT
/*****************************************
 *    ExT_CID = 0xBF
 *****************************************/
INT32 cmd_vlan_update(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 op_code, UINT16 value)
{
	struct cmd_msg *msg;
	INT32 ret = 0, size = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _CMD_VLAN_INFO_UPDATE_T vlan_cmd = {0};

	size = sizeof(CMD_VLAN_INFO_UPDATE_T);
	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx = %d, omac_idx = %d, op_code = %d, value = %d\n",
		band_idx, omac_idx, op_code, value);

	msg = MtAndesAllocCmdMsg(ad, size);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_VLAN_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	vlan_cmd.u2CmdLen = cpu2le16(size);
	vlan_cmd.ucOwnMacIdx = omac_idx;
	vlan_cmd.ucBandIdx = band_idx;
	vlan_cmd.ucOpCode = op_code;
	vlan_cmd.u2Value = cpu2le16(value);

	MtAndesAppendCmdMsg(msg, (UCHAR *)&vlan_cmd, size);
	ret = chip_cmd_tx(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif

INT ShowHeraRelatedInfoProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 cmd = HERA_DUMP_INFO_CMD;
	CMD_GET_HERA_RELATED_INFO param = {0};

	PCHAR pch = NULL;
	PCHAR pPara1 = NULL;
	PCHAR pPara2 = NULL;
	PCHAR pPara3 = NULL;
	PCHAR pPara4 = NULL;

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pPara1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if  (pch != NULL)
		pPara2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL)
		pPara3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");
	if (pch != NULL)
		pPara4 = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u2Para1 = simple_strtol(pPara1, 0, 10);
	param.u2Para2 = simple_strtol(pPara2, 0, 10);
	param.u2Para3 = simple_strtol(pPara3, 0, 10);
	param.u2Para4 = simple_strtol(pPara4, 0, 10);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Para1: %d Para2: %d Para3: %d Para4: %d\n",
				param.u2Para1, param.u2Para2, param.u2Para3, param.u2Para4);

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_HE_RA_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u2Para1 = cpu2le16(param.u2Para1);
	param.u2Para2 = cpu2le16(param.u2Para2);
	param.u2Para3 = cpu2le16(param.u2Para3);
	param.u2Para4 = cpu2le16(param.u2Para4);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, Ret);

	return Ret;
}

#ifdef WIFI_MD_COEX_SUPPORT

static VOID mt_cmd_get_lte_safe_channel_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) msg->priv;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd:%p, Data:%p, Len:%d\n", pAd, Data, Len);
	ExtEventLteSafeChnHandler(pAd, Data, Len);
}

VOID QueryLteSafeChannel(struct _RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	msg = MtAndesAllocCmdMsg(pAd, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_LTE_CHN);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_LTE_SAFE_CHN_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_lte_safe_channel_rsp);
	MtAndesInitCmdMsg(msg, attr);
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return;
}

static VOID mt_cmd_get_idc_info_rsp(
	struct cmd_msg *msg,
	char *Data,
	UINT16 Len)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) msg->priv;

	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd:%p, Data:%p, Len:%d\n", pAd, Data, Len);
	ExtEventIdcEventHandler(pAd, Data, Len);
}

VOID MtCmdIdcInfoQuery(struct _RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	msg = MtAndesAllocCmdMsg(pAd, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GET_IDC_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(COEX_IDC_INFO));
	SET_CMD_ATTR_RSP_HANDLER(attr, mt_cmd_get_idc_info_rsp);
	MtAndesInitCmdMsg(msg, attr);
	ret = chip_cmd_tx(pAd, msg);
error:

	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return;
}

INT32 MtCmdIdcStateUpdate(struct _RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UINT32 Val;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	msg = MtAndesAllocCmdMsg(pAd, sizeof(UINT32));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SET_IDC_STATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	Val = cpu2le32(enable);
	MtAndesAppendCmdMsg(msg, (char *)&Val, sizeof(Val));
	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef COEX_DIRECT_PATH
INT32 CoexUpdate3WireGrp(struct _RTMP_ADAPTER *pAd, VOID *pBuf, UINT32 len)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = MtAndesAllocCmdMsg(pAd, sizeof(EXT_CMD_DBDC_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_UPDATE_3WIRE_GRP);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	MtAndesInitCmdMsg(msg, attr);

	MtAndesAppendCmdMsg(msg, pBuf, len);
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ret = %d\n", ret);
	return ret;
}
#endif

#endif

