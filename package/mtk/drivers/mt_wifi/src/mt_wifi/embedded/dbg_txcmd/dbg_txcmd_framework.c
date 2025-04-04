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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"
#include "dbg_txcmd_framework.h"
#include "mcu/andes_core.h"
#include "mcu/mt_cmd.h"

#define DBG_TXCMD_MODNAME "FRAMEWORK"


/*module init function section*/
VOID txcmdsu_dbg_init(struct dbg_txcmd_framework *dbg_txcmd_ctrl);
/*
struct notify_entry {
	notify_cb_t notify_call;
	struct notify_entry *next;
	INT priority;
	void *priv;
};
*/

/*dbg_txcmd framework interal use*/
/*
*
*/
static INT dbg_txcmd_framework_init(struct _RTMP_ADAPTER *ad)
{
	struct dbg_txcmd_framework *dbg_txcmd_ctrl;

	os_alloc_mem(ad, (UCHAR **)&ad->dbg_txcmd_ctrl, sizeof(struct dbg_txcmd_framework));

	if (!ad->dbg_txcmd_ctrl) {
		printk("%s(): allocate fail!\n", __func__);
		return DBG_TXCMD_STATUS_RESOUCE_ERROR;
	}

	dbg_txcmd_ctrl = ad->dbg_txcmd_ctrl;
	os_zero_mem(dbg_txcmd_ctrl, sizeof(struct dbg_txcmd_framework));
	/*initial framework related part*/
	dbg_txcmd_ctrl->ad = ad;
	DlListInit(&dbg_txcmd_ctrl->dbg_txcmd_head);
	/*start init dbg_txcmd features*/
	txcmdsu_dbg_init(dbg_txcmd_ctrl);

	return 0;
}

/*
*
*/
VOID dbg_txcmd_framework_exit(struct _RTMP_ADAPTER *ad, struct dbg_txcmd_framework *dbg_txcmd_ctrl)
{
	DlListInit(&dbg_txcmd_ctrl->dbg_txcmd_head);
	/*start to exit feature dbg_txcmd*/
	os_free_mem(dbg_txcmd_ctrl);
	ad->dbg_txcmd_ctrl = NULL;
}

/*
*
*/
INT dbg_txcmd_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	struct dbg_txcmd_framework *dbg_txcmd_ctrl;
	struct dbg_txcmd_feature_entry *entry;
	char *cur = arg;
	char *feature;
	char *test_case;
	INT test_id;
	struct os_cookie *obj = (struct os_cookie *) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad, obj->ioctl_if, obj->ioctl_if_type);

	if (!ad->dbg_txcmd_ctrl)
		dbg_txcmd_framework_init(ad);

	dbg_txcmd_ctrl = ad->dbg_txcmd_ctrl;

	if (!dbg_txcmd_ctrl)
		return 0;

	feature = strsep((char **)&cur, "-");
	test_case = strsep((char **)&cur, "-");
	if (!feature || !test_case) {
		DBG_TXCMD_LOG("input parameter error!");
		return 0;
	}
	test_id = simple_strtol(test_case, NULL, 10);
	DBG_TXCMD_LOG("feature,%s&test_id,%d", feature, test_id);
	DlListForEach(entry, &dbg_txcmd_ctrl->dbg_txcmd_head, struct dbg_txcmd_feature_entry, list) {
		if (!strncmp(feature, entry->feature_name, strlen(feature))) {
			if (test_id > entry->dbg_txcmd_cnt)
				break;
			DBG_TXCMD_LOG("search,ok&dbg_txcmd_cnt,%d", entry->dbg_txcmd_cnt);
			return entry->dbg_txcmd_table[test_id - 1](ad, wdev, cur);
		} else {
			DBG_TXCMD_LOG("search,fail!");
		}
	}
	return 0;
}

/*dbg_txcmd module usage only*/
/*
*
*/
INT dbg_txcmd_feature_register(struct dbg_txcmd_framework *dbg_txcmd_ctrl, struct dbg_txcmd_feature_entry *entry)
{
	DlListAdd(&dbg_txcmd_ctrl->dbg_txcmd_head, &entry->list);
	return 0;
}

/*
*
*/
VOID dbg_txcmd_feature_unregister(struct dbg_txcmd_feature_entry *entry)
{
	DlListDel(&entry->list);
}

/*
*
*/
static VOID dbg_txcmd_ut_wmcu_resp(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_DBG_TXCMD_CTRL_EXT_T *dbg_txcmd_res = (CMD_DBG_TXCMD_CTRL_EXT_T *) data;
	UINT16 head_len = sizeof(CMD_DBG_TXCMD_CTRL_EXT_T);

	DBG_TXCMD_LOG("feature,%d&type,%d&len,%d", dbg_txcmd_res->u4FeatureIdx, dbg_txcmd_res->u4Type, len);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, data + head_len, len - head_len);
}

/*
*
*/
INT32 dbg_ut_wmcu_send(struct _RTMP_ADAPTER *ad, struct dbg_wmcu_request *request)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UCHAR *data;
	CMD_DBG_TXCMD_CTRL_EXT_T *dbg_txcmd_req;
	UINT16 cmd_len = sizeof(CMD_DBG_TXCMD_CTRL_EXT_T) + max(request->resp_len, request->len);
	struct _CMD_ATTRIBUTE attr = {0};

	DBG_TXCMD_LOG("id,%d&type,%d", request->feature_id, request->type);
	DBG_TXCMD_LOG("len,%d&payload,%p", request->len, request->payload);
	DBG_TXCMD_LOG("resp_len,%d&resp_payload,%p", request->resp_len, request->resp);
	os_alloc_mem(ad, &data, cmd_len);

	if (!data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	dbg_txcmd_req = (CMD_DBG_TXCMD_CTRL_EXT_T *)data;
	msg = AndesAllocCmdMsg(ad, cmd_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DBG_TXCMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 10000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, request->resp_len + sizeof(CMD_DBG_TXCMD_CTRL_EXT_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, request->resp);
	if (request->resp_handle)
		SET_CMD_ATTR_RSP_HANDLER(attr, request->resp_handle);
	else
		SET_CMD_ATTR_RSP_HANDLER(attr, dbg_txcmd_ut_wmcu_resp);
	AndesInitCmdMsg(msg, attr);
	dbg_txcmd_req->u4FeatureIdx = cpu2le32(request->feature_id);
	dbg_txcmd_req->u4Type = cpu2le32(request->type);
	dbg_txcmd_req->u4Lth = cpu2le32(request->len);
	dbg_txcmd_req->u2CmdLen = cpu2le16(cmd_len);
	dbg_txcmd_req->ucCmdVer = 0;
	os_move_mem(dbg_txcmd_req->u1cBuffer, request->payload, request->len);

	AndesAppendCmdMsg(msg, (char *)dbg_txcmd_req, cmd_len);
	ret = chip_cmd_tx(ad, msg);
error:
	if (data)
		os_free_mem(data);
	DBG_TXCMD_LOG("ret,%d", ret);
	if (ret != NDIS_STATUS_SUCCESS)
		return DBG_TXCMD_STATUS_REQ_MCPU_FAIL;
	return DBG_TXCMD_STATUS_OK;
}

