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
 * This file contains IOCTL for TXCMD debug mode specfic commands
 */
/*************************************************************************
 * ***********************************************************************
 */
/*************************************************************************
 * LEGAL DISCLAIMER
 *
 * BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
 * SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
 * DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
 * ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
 * WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
 * SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
 * WARRANTY CLAIM RELATING THERetO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
 * FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
 * CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
 * BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
 * WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
 * OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
 * THEREOF AND RELATED THERetO SHALL BE SETTLED BY ARBITRATION IN SAN
 * FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
 * (ICC).
 * ***********************************************************************
 */
#include "rt_config.h"

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG

VOID display_sxn_all(struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd)
{
	UINT16 i;
	UINT32 *data;

	data = (UINT32 *) (rsp_cmd + 1);
	rsp_cmd->data_len /= 4;
	for (i = 0; i < rsp_cmd->data_len; i++) {
		MTWF_PRINT("DW%02d: 0x%08X\n", i, *(data + i));
	}
}

VOID display_dbg_status(struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd)
{
	struct TXCMD_DBG_CMD_STATUS *status;

	status = (struct TXCMD_DBG_CMD_STATUS *) (rsp_cmd + 1);
	MTWF_PRINT("debug mode: %s\n", (status->enable) ?
			 "enable" : "disable");
	MTWF_PRINT("TXCMD TX statistics:\n"
			 "total=%d, protect=%d, txdata=%d, trigdata=%d, sw_fid=%d\n",
			 status->txcmd_tx_count, status->txcmd_protect_count,
			 status->txcmd_txdata_count,
			 status->txcmd_trigdata_count,
			 status->txcmd_swfid_count);
	MTWF_PRINT("TXCMD RX statistics:\n"
			 "SPL=%d, CMDRPT(txdata)=%d, CMDRPT(trigdata)=%d, RXRPT=%d\n",
			 status->spl_count, status->txdata_cmdrpt_count,
			 status->trigdata_cmdrpt_count, status->rxrpt_count);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventDisplaySxnAll(UINT8 *pData, UINT16 u2DataLength)
{
	UINT16 i;
	UINT32 *Data = (UINT32 *)pData;
	UINT16 DataLen = u2DataLength / 4;

	for (i = 0; i < DataLen; i++) {
		MTWF_PRINT("DW%02d: 0x%08X\n", i, *(Data + i));
	}
}

VOID  UniEventDisplayDbgStatus(struct TXCMD_DBG_CMD_STATUS *status)
{
	MTWF_PRINT("debug mode: %s\n", (status->enable) ?
			 "enable" : "disable"));
	MTWF_PRINT("TXCMD TX statistics:\n"
			 "total=%d, protect=%d, txdata=%d, trigdata=%d, sw_fid=%d\n",
			 status->txcmd_tx_count, status->txcmd_protect_count,
			 status->txcmd_txdata_count,
			 status->txcmd_trigdata_count,
			 status->txcmd_swfid_count);
	MTWF_PRINT("TXCMD RX statistics:\n"
			 "SPL=%d, CMDRPT(txdata)=%d, CMDRPT(trigdata)=%d, RXRPT=%d\n",
			 status->spl_count, status->txdata_cmdrpt_count,
			 status->trigdata_cmdrpt_count, status->rxrpt_count);
}

#endif /* WIFI_UNIFIED_COMMAND */

VOID eventExtCmdTxcmdShow(struct cmd_msg *msg, PCHAR rsp_payload,
		UINT16 rsp_payload_len)
{
	struct EXT_CMD_TXCMD_DBG_CTRL *get_data;

	get_data = (struct EXT_CMD_TXCMD_DBG_CTRL *) rsp_payload;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: cmdid=%x, rsp_len=%d\n", __func__,
			 get_data->cmd_id, rsp_payload_len);

	switch (get_data->cmd_id) {
	case GET_TXCMD_DBG_SXN_GLOBAL:
		MTWF_PRINT("Sxn Global(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_SXN_PROTECT:
		MTWF_PRINT("Sxn Proctect(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_SXN_TXDATA:
		MTWF_PRINT("Sxn TXDATA(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_SXN_TRIGDATA:
		MTWF_PRINT("Sxn TrigDATA(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_SXN_SW_FID:
		MTWF_PRINT("Sxn SW FID(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_SW_FID_TXD:
		MTWF_PRINT("Sxn TF TXD(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_TF_TXD:
		MTWF_PRINT("Sxn TF TXD(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_TF_BASIC:
		MTWF_PRINT("Sxn TF Basic(%d): \n", get_data->data_len);
		display_sxn_all(get_data);
		break;
	case GET_TXCMD_DBG_STATUS:
	default:
		display_dbg_status(get_data);
		break;
	}

	os_free_mem(get_data);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventTxCmdShow(struct _UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T *TlvData)
{
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: cmdid=%x, rsp_len=%d\n", __func__,
			 TlvData->u2Tag, TlvData->u2Length);

	switch (TlvData->u2Tag) {
	case UNI_EVENT_GET_TXCMD_DBG_SXN_GLOBAL:
		MTWF_PRINT("Sxn Global(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case UNI_EVENT_GET_TXCMD_DBG_SXN_PROTECT:
		MTWF_PRINT("Sxn Proctect(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_SXN_TXDATA:
		MTWF_PRINT("Sxn TXDATA(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_SXN_TRIGDATA:
		MTWF_PRINT("Sxn TrigDATA(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_SXN_SW_FID:
		MTWF_PRINT("Sxn SW FID(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_SW_FID_TXD:
		MTWF_PRINT("Sxn TF TXD(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_TF_TXD:
		MTWF_PRINT("Sxn TF TXD(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_TF_BASIC:
		MTWF_PRINT("Sxn TF Basic(%d): \n", TlvData->u2Length);
		UniEventDisplaySxnAll(TlvData->aucBuffer, TlvData->u2Length);
		break;
	case GET_TXCMD_DBG_STATUS:
	default:
		UniEventDisplayDbgStatus((struct TXCMD_DBG_CMD_STATUS *)TlvData->aucBuffer);
		break;
	}
}

#endif /* WIFI_UNIFIED_COMMAND */

INT send_cmd_msg(RTMP_ADAPTER *pAd, UINT8 *cmd_data, UINT8 *rsp_payload)
{
	INT ret = STATUS_SUCCESS;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl, *rsp_cmd;
	UINT16 cmd_ctrl_len = sizeof(*cmd_ctrl);
	UINT8 ctrl_flag;
	UINT16 rsp_payload_len = 0;
	VOID *rsp_func = NULL;

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) cmd_data;
	switch (cmd_ctrl->cmd_id) {
	case SET_TXCMD_DBG_CTRL:
	case SET_TXCMD_DBG_CLEAR:
	case SET_TXCMD_DBG_SXN_GLOBAL:
	case SET_TXCMD_DBG_SXN_PROTECT:
	case SET_TXCMD_DBG_SXN_PROTECT_RUINFO:
	case SET_TXCMD_DBG_SXN_TXDATA:
	case SET_TXCMD_DBG_SXN_TXDATA_USER_INFO:
	case SET_TXCMD_DBG_SXN_TRIGDATA:
	case SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO:
	case SET_TXCMD_DBG_TF_TXD:
	case SET_TXCMD_DBG_TF_BASIC:
	case SET_TXCMD_DBG_TF_BASIC_USER:
	case SET_TXCMD_DBG_SXN_SW_FID:
	case SET_TXCMD_DBG_SXN_SW_FID_INFO:
	case SET_TXCMD_DBG_SW_FID_TXD:
	case SET_TXCMD_DBG_SOP:
		ctrl_flag = INIT_CMD_SET_AND_RETRY;
		rsp_payload_len = 0;
		rsp_func = NULL;
		break;
	case GET_TXCMD_DBG_STATUS:
	case GET_TXCMD_DBG_SXN_GLOBAL:
	case GET_TXCMD_DBG_SXN_PROTECT:
	case GET_TXCMD_DBG_SXN_TXDATA:
	case GET_TXCMD_DBG_SXN_TRIGDATA:
	case GET_TXCMD_DBG_TF_TXD:
	case GET_TXCMD_DBG_TF_BASIC:
	case GET_TXCMD_DBG_SXN_SW_FID:
	case GET_TXCMD_DBG_SW_FID_TXD:
	default:
		ctrl_flag = INIT_CMD_QUERY_AND_WAIT_RSP;
		rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) rsp_payload;
		rsp_payload_len = cmd_ctrl_len + rsp_cmd->data_len;
		rsp_func = eventExtCmdTxcmdShow;
		break;
	}

	msg = AndesAllocCmdMsg(pAd, cmd_ctrl_len + cmd_ctrl->data_len);
	if (!msg) {
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TXCMD_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, ctrl_flag);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, rsp_payload_len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rsp_payload);
	SET_CMD_ATTR_RSP_HANDLER(attr, rsp_func);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *) cmd_data,
			cmd_ctrl_len + cmd_ctrl->data_len);
	AndesSendCmdMsg(pAd, msg);
end:
	os_free_mem(rsp_payload);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

#ifdef WIFI_UNIFIED_COMMAND
INT UniCmdSendTxCmdDbg(RTMP_ADAPTER *pAd, UINT8 *cmd_data, UINT8 *rsp_payload)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT16 cmd_ctrl_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	UNI_CMD_TXCMD_DBG_CTRL_PARAM_T TxCmdDbgCtrlParam;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) cmd_data;

	os_zero_mem(&TxCmdDbgCtrlParam, sizeof(TxCmdDbgCtrlParam));

	switch (cmd_ctrl->cmd_id) {
	case UNI_CMD_SET_TXCMD_DBG_CTRL:
	case UNI_CMD_SET_TXCMD_DBG_CLEAR:
	case UNI_CMD_SET_TXCMD_DBG_SXN_GLOBAL:
	case UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT:
	case UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT_RUINFO:
	case UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA:
	case UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA_USER_INFO:
	case UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA:
	case UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO:
	case UNI_CMD_SET_TXCMD_DBG_TF_TXD:
	case UNI_CMD_SET_TXCMD_DBG_TF_BASIC:
	case UNI_CMD_SET_TXCMD_DBG_TF_BASIC_USER:
	case UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID:
	case UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID_INFO:
	case UNI_CMD_SET_TXCMD_DBG_SW_FID_TXD:
	case UNI_CMD_SET_TXCMD_DBG_SOP:
		TxCmdDbgCtrlParam.bQuery = FALSE;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].ucUserIndex = cmd_ctrl->index;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].ucDlUlidx = cmd_ctrl->txcmd_entry_idx;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].u4DataLen = cmd_ctrl->data_len;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].pData = (UINT8 *)(cmd_data + cmd_ctrl_len);
		TxCmdDbgCtrlParam.TxCmdDbgCtrlValid[cmd_ctrl->cmd_id] = TRUE;
		break;
	case UNI_CMD_GET_TXCMD_DBG_STATUS:
	case UNI_CMD_GET_TXCMD_DBG_SXN_GLOBAL:
	case UNI_CMD_GET_TXCMD_DBG_SXN_PROTECT:
	case UNI_CMD_GET_TXCMD_DBG_SXN_TXDATA:
	case UNI_CMD_GET_TXCMD_DBG_SXN_TRIGDATA:
	case UNI_CMD_GET_TXCMD_DBG_TF_TXD:
	case UNI_CMD_GET_TXCMD_DBG_TF_BASIC:
	case UNI_CMD_GET_TXCMD_DBG_SXN_SW_FID:
	case UNI_CMD_GET_TXCMD_DBG_SW_FID_TXD:
		TxCmdDbgCtrlParam.bQuery = TRUE;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].ucUserIndex = cmd_ctrl->index;
		TxCmdDbgCtrlParam.SetTxCmdDbgEntry[cmd_ctrl->cmd_id].ucDlUlidx = cmd_ctrl->txcmd_entry_idx;
		TxCmdDbgCtrlParam.TxCmdDbgCtrlValid[cmd_ctrl->cmd_id] = TRUE;
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmd_id (0x%x) not support!\n", cmd_ctrl->cmd_id);
		return NDIS_STATUS_INVALID_DATA;
	}

	Ret = UniCmdTxCmdDbgCtrl(pAd, &TxCmdDbgCtrlParam);

	if (rsp_payload != NULL)
		os_free_mem(rsp_payload);

	return Ret;
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID set_txcmd_entry_idx(struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		cmd_ctrl->txcmd_entry_idx = 0;
	} else {
		cmd_ctrl->txcmd_entry_idx = os_str_tol(pch, 0, 16);
		if (cmd_ctrl->txcmd_entry_idx > 1) {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid txcmd_entry_idx : %d. Set to 0\n",
				 cmd_ctrl->txcmd_entry_idx);
			cmd_ctrl->txcmd_entry_idx = 0;
		}
	}

}

INT set_txcmd_dbg_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_DBG_CMD_STATUS *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CTRL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len);
	mask = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->enable = os_str_tol(pch, 0, 10);
		mask->enable = 1;
	}
	set_txcmd_entry_idx(cmd_ctrl, arg);

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_clear(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;

	cmd_len = sizeof(*cmd_ctrl);
	ret = os_alloc_mem(pAd, &ptr, cmd_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CLEAR;
	cmd_ctrl->data_len = 0;

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0) && (*pch == '1'))
		ret = send_cmd_msg(pAd, ptr, NULL);
	else
		ret = STATUS_UNSUCCESSFUL;
	os_free_mem(ptr);

end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_muru_algo_disable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_DBG_CMD_STATUS *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CTRL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len);
	mask = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->muru_algo_disable = os_str_tol(pch, 0, 10);
		mask->muru_algo_disable = 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_ra_algo_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_DBG_CMD_STATUS *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CTRL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len);
	mask = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ra_algo_enable = os_str_tol(pch, 0, 10);
		mask->ra_algo_enable = 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_dl_ul_tp(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_DBG_CMD_STATUS *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CTRL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len);
	mask = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->dl_ul_tp = os_str_tol(pch, 0, 10);
		mask->dl_ul_tp = 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_send_trig(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_DBG_CMD_STATUS *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_CTRL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len);
	mask = (struct TXCMD_DBG_CMD_STATUS *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->send_trigger = os_str_tol(pch, 0, 10);
		mask->send_trigger = 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_dbg_sop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct _TXCMD_DEBUG_SOP_CMD_T *val;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SOP;
	cmd_ctrl->data_len = val_len;
	val = (struct _TXCMD_DEBUG_SOP_CMD_T *) (ptr + cmd_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ucAhDbgDvtTestCase = os_str_tol(pch, 0, 10);
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->u2AhDbgWcid = os_str_tol(pch, 0, 10);
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ucAhDbgAcQue = os_str_tol(pch, 0, 10);
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}


INT set_txcmd_sxn_global(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_GLOBAL *val, *mask;

	/* read parameter */
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: arg = %s\n", __func__, arg);
	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_GLOBAL;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_GLOBAL *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_GLOBAL *) (ptr + cmd_len + val_len);

	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->spl = os_str_tol(pch, 0, 16);
		mask->spl |= (1<<1) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->tv = os_str_tol(pch, 0, 16);
		mask->tv = (1<<1) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->txop = os_str_tol(pch, 0, 16);
		mask->txop = (1<<1) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d\n", __func__, ret);
	return ret;
}

INT set_txcmd_sxn_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_PROTECT *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->ruInfo);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_PROTECT;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_PROTECT *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_PROTECT *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->protect = os_str_tol(pch, 0, 16);
		mask->protect = (1<<2) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->rate = os_str_tol(pch, 0, 16);
		mask->rate = (1<<6) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->txMode = os_str_tol(pch, 0, 16);
		mask->txMode = (1<<4) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->nsts = os_str_tol(pch, 0, 16);
		mask->nsts = (1<<3) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_protect_ruinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_RU_INFO *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_PROTECT_RUINFO;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_RU_INFO *) (ptr + cmd_len);
	mask = (struct TXCMD_RU_INFO *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = (UINT8) os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index >= MAX_NUM_TXCMD_RU_INFO) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n",
				 MAX_NUM_TXCMD_RU_INFO);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ruAlloc = os_str_tol(pch, 0, 16);
		mask->ruAlloc = (1<<4) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_txdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_TX_DATA *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->txcmdUser);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TXDATA;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_TX_DATA *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_TX_DATA *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ra = os_str_tol(pch, 0, 16);
		mask->ra = (1<<1) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->txPower = os_str_tol(pch, 0, 16);
		mask->txPower = (1<<8) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->txMode = os_str_tol(pch, 0, 16);
		mask->txMode = (1<<4) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_txdata_rualloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_TX_DATA *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->txcmdUser);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TXDATA;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_TX_DATA *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_TX_DATA *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = (UINT8)os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index > 7) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n", 7);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ruAlloc[cmd_ctrl->index] = os_str_tol(pch, 0, 16);
		mask->ruAlloc[cmd_ctrl->index] = (1<<8) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_txdata_userinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_USER_INFO *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TXDATA_USER_INFO;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_USER_INFO *) (ptr + cmd_len);
	mask = (struct TXCMD_USER_INFO *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = (UINT8)os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index >= MAX_NUM_TXCMD_USER_INFO) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n",
				 MAX_NUM_TXCMD_USER_INFO);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ruAlloc = os_str_tol(pch, 0, 16);
		mask->ruAlloc = (1<<7) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->rate = os_str_tol(pch, 0, 16);
		mask->rate = (1<<6) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->nsts = os_str_tol(pch, 0, 16);
		mask->nsts = (1<<3) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->contentCh = os_str_tol(pch, 0, 16);
		mask->contentCh = (1<<1) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ackPol = os_str_tol(pch, 0, 16);
		mask->ackPol = (1<<2) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->LSigLen = os_str_tol(pch, 0, 16);
		mask->LSigLen = (1<<12) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ackRuAlloc = os_str_tol(pch, 0, 16);
		mask->ackRuAlloc = (1<<7) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ackMcs = os_str_tol(pch, 0, 16);
		mask->ackMcs = (1<<4) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ssAlloc = os_str_tol(pch, 0, 16);
		mask->ssAlloc = (1<<6) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_trigdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_TRIG_DATA *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->txcmdUserAck);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TRIGDATA;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_TRIG_DATA *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_TRIG_DATA *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->baPol = os_str_tol(pch, 0, 16);
		mask->baPol = (1<<2) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->lSigLen = os_str_tol(pch, 0, 16);
		mask->lSigLen = (1<<12) - 1;
#ifdef RT_BIG_ENDIAN
		val->lSigLen = cpu2le16(val->lSigLen);
		mask->lSigLen = cpu2le16(mask->lSigLen);
#endif
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ackTxMode = os_str_tol(pch, 0, 16);
		mask->ackTxMode = (1<<4) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_trigdata_rualloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_SXN_TRIG_DATA *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->txcmdUserAck);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TRIGDATA;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_SXN_TRIG_DATA *) (ptr + cmd_len);
	mask = (struct TXCMD_SXN_TRIG_DATA *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = (UINT8)os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index > 7) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n", 7);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ruAlloc[cmd_ctrl->index] = os_str_tol(pch, 0, 16);
		mask->ruAlloc[cmd_ctrl->index] = (1<<8) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_trigdata_user_ackinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_USER_ACK_INFO *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_USER_ACK_INFO *) (ptr + cmd_len);
	mask = (struct TXCMD_USER_ACK_INFO *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = (UINT8)os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index >= MAX_NUM_TXCMD_USER_INFO) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n",
				 MAX_NUM_TXCMD_USER_INFO);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->contentCh = os_str_tol(pch, 0, 16);
		mask->contentCh = (1<<1) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->ruAlloc = os_str_tol(pch, 0, 16);
		mask->ruAlloc = (1<<7) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->rate = os_str_tol(pch, 0, 16);
		mask->rate = (1<<6) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->nsts = os_str_tol(pch, 0, 16);
		mask->nsts = (1<<3) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_tf_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TXCMD_TXD *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_TF_TXD;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TXCMD_TXD *) (ptr + cmd_len);
	mask = (struct TXCMD_TXD *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->bw = os_str_tol(pch, 0, 16);
		mask->bw = (1<<3) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);

	return ret;
}

INT set_txcmd_tf_basic(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TF_BASIC *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	val_len -= sizeof(val->arBasicUsr);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_TF_BASIC;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TF_BASIC *) (ptr + cmd_len);
	mask = (struct TF_BASIC *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->bw = os_str_tol(pch, 0, 16);
		mask->bw = (1<<2) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_tf_basic_user(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	struct TF_BASIC_USER *val, *mask;

	cmd_len = sizeof(*cmd_ctrl);
	val_len = sizeof(*val);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = SET_TXCMD_DBG_TF_BASIC_USER;
	cmd_ctrl->data_len = val_len * 2;
	val = (struct TF_BASIC_USER *) (ptr + cmd_len);
	mask = (struct TF_BASIC_USER *) (ptr + cmd_len + val_len);

	/* read parameter */
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl->index = os_str_tol(pch, 0, 16);
	if (cmd_ctrl->index >= MAX_NUM_TXCMD_TF_BASIC_USER) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n",
				 MAX_NUM_TXCMD_TF_BASIC_USER);
		ret = STATUS_UNSUCCESSFUL;
		goto release;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->tidAggrLimit = os_str_tol(pch, 0, 16);
		mask->tidAggrLimit = (1<<3) - 1;
	}
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0)) {
		val->preferredAc = os_str_tol(pch, 0, 16);
		mask->preferredAc = (1<<2) - 1;
	}

	ret = send_cmd_msg(pAd, ptr, NULL);
release:
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

void set_dw_usage(int mode)
{
	MTWF_PRINT("usage:\n\n");

	if (mode == 0) {
		MTWF_PRINT("\tiwpriv ra0 set txcmd_sxn_dw=%s%s%s%s%s%s\n",
			 "[SXN#]-",
			 "[DW#]-",
			 "[START_BIT]-",
			 "[BIT_NUM]-",
			 "[VALUE]-",
			 "[DL_UL_IDX]");
	} else {
		MTWF_PRINT("\tiwpriv ra0 set txcmd_sxn_user_dw=%s%s%s%s%s%s%s\n",
			 "[SXN#]-",
			 "[USER_INDEX]-",
			 "[DW#]-",
			 "[START_BIT]-",
			 "[BIT_NUM]-",
			 "[VALUE]-",
			 "[DL_UL_IDX]");
	}
}

INT set_txcmd_sxn_dw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	union txcmd_sxn_all section;
	UINT8 cmd_id, sxn_idx, dw_idx, s_bit, bit_num, txcmd_entry_idx;
	UINT32 rdata, *val_ptr, *mask_ptr;

	/* read parameter: [sxn#] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(0);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	sxn_idx = os_str_tol(pch, 0, 10);
	/* read parameter: [DW#] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(0);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	dw_idx = os_str_tol(pch, 0, 10);
	/* read parameter: [start bit] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(0);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	s_bit = os_str_tol(pch, 0, 10);
	/* read parameter: [bit number] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(0);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	bit_num = os_str_tol(pch, 0, 10);
	/* read parameter: [value] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(0);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	rdata = os_str_tol(pch, 0, 16);

	/* read parameter: [dl_ul_idx] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		txcmd_entry_idx = 0;
	} else {
		txcmd_entry_idx = os_str_tol(pch, 0, 16);
		if (txcmd_entry_idx > 1) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid txcmd_entry_idx : %d. Set to 0\n",
				 txcmd_entry_idx);
			txcmd_entry_idx = 0;
		}
	}

	switch (sxn_idx) {
	case TF_BASIC_ID:
		val_len = sizeof(*section.tf_basic);
		val_len -= sizeof(section.tf_basic->arBasicUsr);
		cmd_id = SET_TXCMD_DBG_TF_BASIC;
		break;
	case TF_TXD_ID:
		val_len = sizeof(*section.tf_txd);
		cmd_id = SET_TXCMD_DBG_TF_TXD;
		break;
	case SXN_SW_FID_ID:
		val_len = sizeof(*section.sw_fid);
		val_len -= sizeof(section.sw_fid->swFidInfo);
		cmd_id = SET_TXCMD_DBG_SXN_SW_FID;
		break;
	case SXN_TRIG_DATA_ID:
		val_len = sizeof(*section.trig_data);
		val_len -= sizeof(section.trig_data->txcmdUserAck);
		cmd_id = SET_TXCMD_DBG_SXN_TRIGDATA;
		break;
	case SXN_TX_DATA_ID:
		val_len = sizeof(*section.tx_data);
		val_len -= sizeof(section.tx_data->txcmdUser);
		cmd_id = SET_TXCMD_DBG_SXN_TXDATA;
		break;
		break;
	case SXN_PROTECT_ID:
		val_len = sizeof(*section.protect);
		val_len -= sizeof(section.protect->ruInfo);
		cmd_id = SET_TXCMD_DBG_SXN_PROTECT;
		break;
	case SXN_GLOBAL_ID:
		val_len = sizeof(*section.global);
		cmd_id = SET_TXCMD_DBG_SXN_GLOBAL;
		break;
	default:
		val_len = 0;
		cmd_id = 0xff;
	}

	cmd_len = sizeof(*cmd_ctrl);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = cmd_id;
	cmd_ctrl->data_len = val_len * 2;
	cmd_ctrl->txcmd_entry_idx = txcmd_entry_idx;
	val_ptr = (UINT32 *) (ptr + cmd_len);
	val_ptr += dw_idx;
	mask_ptr = (UINT32 *) (ptr + cmd_len + val_len);
	mask_ptr += dw_idx;
	(*val_ptr) |= rdata<<s_bit;
	if (bit_num >= 32)
		(*mask_ptr) = 0xffffffff;
	else
		(*mask_ptr) |= ((1<<bit_num) - 1)<<s_bit;

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT set_txcmd_sxn_user_dw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	PCHAR pch = NULL;
	UINT8 *ptr;
	UINT16 cmd_len, val_len;
	struct EXT_CMD_TXCMD_DBG_CTRL *cmd_ctrl;
	union txcmd_sxn_all section;
	UINT8 cmd_id, sxn_idx, user_idx, dw_idx, s_bit, bit_num, txcmd_entry_idx;
	UINT32 rdata, *val_ptr, *mask_ptr;

	/* read parameter: [sxn#] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	sxn_idx = os_str_tol(pch, 0, 10);
	/* read parameter: [user_idx] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	user_idx = os_str_tol(pch, 0, 10);
	/* read parameter: [DW#] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	dw_idx = os_str_tol(pch, 0, 10);
	/* read parameter: [start bit] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	s_bit = os_str_tol(pch, 0, 10);
	/* read parameter: [bit number] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	bit_num = os_str_tol(pch, 0, 10);
	/* read parameter: [value] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		set_dw_usage(1);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}
	rdata = os_str_tol(pch, 0, 16);

	/* read parameter: [dl_ul_idx] */
	pch = strsep(&arg, "-");
	if ((pch == NULL) || (strlen(pch) == 0)) {
		txcmd_entry_idx = 0;
	} else {
		txcmd_entry_idx = os_str_tol(pch, 0, 16);
		if (txcmd_entry_idx > 1) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid txcmd_entry_idx : %d. Set to 0\n",
				 txcmd_entry_idx);
			txcmd_entry_idx = 0;
		}
	}

	switch (sxn_idx) {
	case TF_BASIC_ID:
		val_len = sizeof(*section.tf_basic_user);
		if (user_idx >= MAX_NUM_TXCMD_TF_BASIC_USER) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_TF_BASIC_USER);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_TF_BASIC_USER;
		break;
	case SXN_SW_FID_ID:
		val_len = sizeof(*section.sw_fid_info);
		if (user_idx >= MAX_NUM_TXCMD_TF_BASIC_USER) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_TF_BASIC_USER);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_SXN_SW_FID_INFO;
		break;
	case SW_FID_TXD_ID:
		val_len = sizeof(*section.sw_fid_txd);
		if (user_idx >= MAX_NUM_TXCMD_SW_FID_INFO) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_SW_FID_INFO);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_SW_FID_TXD;
		break;
	case SXN_TRIG_DATA_ID:
		val_len = sizeof(*section.trig_data_user);
		if (user_idx >= MAX_NUM_TXCMD_USER_INFO) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_USER_INFO);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO;
		break;
	case SXN_TX_DATA_ID:
		val_len = sizeof(*section.tx_data_user);
		if (user_idx >= MAX_NUM_TXCMD_USER_INFO) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_USER_INFO);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_SXN_TXDATA_USER_INFO;
		break;
	case SXN_PROTECT_ID:
		val_len = sizeof(*section.ru_info);
		if (user_idx >= MAX_NUM_TXCMD_RU_INFO) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"index exceed than %d\n",
					 MAX_NUM_TXCMD_RU_INFO);
			ret = STATUS_UNSUCCESSFUL;
			goto end;
		}
		cmd_id = SET_TXCMD_DBG_SXN_PROTECT_RUINFO;
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"index exceed than %d\n",
				 MAX_NUM_TXCMD_RU_INFO);
		ret = STATUS_UNSUCCESSFUL;
		goto end;
	}

	cmd_len = sizeof(*cmd_ctrl);
	ret = os_alloc_mem(pAd, &ptr, cmd_len + val_len * 2);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(ptr, cmd_len + val_len * 2);

	cmd_ctrl = (struct EXT_CMD_TXCMD_DBG_CTRL *) ptr;
	cmd_ctrl->cmd_id = cmd_id;
	cmd_ctrl->data_len = val_len * 2;
	cmd_ctrl->index = user_idx;
	cmd_ctrl->txcmd_entry_idx = txcmd_entry_idx;
	val_ptr = (UINT32 *) (ptr + cmd_len);
	val_ptr += dw_idx;
	mask_ptr = (UINT32 *) (ptr + cmd_len + val_len);
	mask_ptr += dw_idx;
	(*val_ptr) |= rdata<<s_bit;
	if (bit_num >= 32)
		(*mask_ptr) = 0xffffffff;
	else
		(*mask_ptr) |= ((1<<bit_num) - 1)<<s_bit;

	ret = send_cmd_msg(pAd, ptr, NULL);
	os_free_mem(ptr);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_dbg_status(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_STATUS;
	cmd_ctrl.data_len = 0;
	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_DBG_CMD_STATUS);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);

	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_STATUS;
	rsp_cmd->data_len = sizeof(struct TXCMD_DBG_CMD_STATUS);

	ret = send_cmd_msg(pAd, (UINT8 *)&cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sxn_global(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SXN_GLOBAL;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_SXN_GLOBAL);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SXN_GLOBAL;
	rsp_cmd->data_len = sizeof(struct TXCMD_SXN_GLOBAL);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sxn_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SXN_PROTECT;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_SXN_PROTECT);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SXN_PROTECT;
	rsp_cmd->data_len = sizeof(struct TXCMD_SXN_PROTECT);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sxn_txdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SXN_TXDATA;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_SXN_TX_DATA);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SXN_TXDATA;
	rsp_cmd->data_len = sizeof(struct TXCMD_SXN_TX_DATA);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sxn_trigdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SXN_TRIGDATA;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_SXN_TRIG_DATA);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SXN_TRIGDATA;
	rsp_cmd->data_len = sizeof(struct TXCMD_SXN_TRIG_DATA);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sw_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SXN_SW_FID;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_SXN_SW_FID);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SXN_SW_FID;
	rsp_cmd->data_len = sizeof(struct TXCMD_SXN_SW_FID);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_sw_fid_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_SW_FID_TXD;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_TXD) * MAX_NUM_TXCMD_SW_FID_INFO;
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_SW_FID_TXD;
	rsp_cmd->data_len = sizeof(struct TXCMD_TXD)*MAX_NUM_TXCMD_SW_FID_INFO;

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_tf_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_TF_TXD;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TXCMD_TXD);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_TF_TXD;
	rsp_cmd->data_len = sizeof(struct TXCMD_TXD);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

INT show_txcmd_tf_basic(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = STATUS_SUCCESS;
	struct EXT_CMD_TXCMD_DBG_CTRL cmd_ctrl;
	struct EXT_CMD_TXCMD_DBG_CTRL *rsp_cmd;
	UINT8 *qurry_data;
	UINT16 qurry_data_len;
	PCHAR pch = NULL;

	cmd_ctrl.cmd_id = GET_TXCMD_DBG_TF_BASIC;
	cmd_ctrl.data_len = 0;
	pch = strsep(&arg, "-");
	if ((pch != NULL) && (strlen(pch) > 0))
		cmd_ctrl.index = os_str_tol(pch, 0, 10);
	else
		cmd_ctrl.index = 0;

	set_txcmd_entry_idx(&cmd_ctrl, arg);

	qurry_data_len = sizeof(struct EXT_CMD_TXCMD_DBG_CTRL);
	qurry_data_len += sizeof(struct TF_BASIC);
	ret = os_alloc_mem(pAd, &qurry_data, qurry_data_len);
	if (ret == STATUS_UNSUCCESSFUL)
		goto end;
	os_zero_mem(qurry_data, qurry_data_len);
	rsp_cmd = (struct EXT_CMD_TXCMD_DBG_CTRL *) qurry_data;
	rsp_cmd->cmd_id = GET_TXCMD_DBG_TF_BASIC;
	rsp_cmd->data_len = sizeof(struct TF_BASIC);

	ret = send_cmd_msg(pAd, (UINT8 *) &cmd_ctrl, qurry_data);
end:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Ret = %d\n", ret);
	return ret;
}

#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */
