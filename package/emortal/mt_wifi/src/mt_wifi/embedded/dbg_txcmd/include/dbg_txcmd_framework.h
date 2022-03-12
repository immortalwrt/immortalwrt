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

#ifndef __FRAMEWORK_DBG_TXCMD_H__
#define __FRAMEWORK_DBG_TXCMD_H__

struct _RTMP_ADAPTER;
struct _DL_LIST;
struct dbg_txcmd_framework;

/*command/event not need to follow linux coding style*/
typedef struct GNU_PACKED _CMD_DBG_TXCMD_CTRL_EXT_T
{
	/** DWORD_0 - Common Part */
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	/** Cmd size including common part and body */
	UINT16 u2CmdLen;
	/** DWORD_N - Body Part */
	/** Feature  ID(_ENUM_SYSDVT_FEATURE_T)*/
	UINT32  u4FeatureIdx;
	/** Test case  ID (Type) */
	UINT32  u4Type;
	/** dvt parameter's data struct size (Length) */
	UINT32  u4Lth;
    /** dvt parameter's data struct (Value) */
	UINT8   u1cBuffer[0];
} CMD_DBG_TXCMD_CTRL_EXT_T, *P_CMD_DBG_TXCMD_CTRL_EXT_T;


enum {
	DBG_TXCMD_STATUS_OK,
	DBG_TXCMD_STATUS_FAIL,
	DBG_TXCMD_STATUS_RESOUCE_ERROR,
	DBG_TXCMD_STATUS_CONN_FAIL,
	DBG_TXCMD_STATUS_REQ_MCPU_FAIL,
	DBG_TXCMD_STATUS_FINAL
};

enum {
	DBG_TXCMD_NOTIFY_WSYS_CONNECT_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_DISCONN_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_CLOSE_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_OPEN_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_LINKUP_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_LINKDOWN_EVENT,
	DBG_TXCMD_NOTIFY_WSYS_STAUPDATE_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_DATA_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_CMD_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_TRX_FREE_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_TMR_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_TRXV_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_RX_TXS_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_WMM_DETECT_EVENT,
	DBG_TXCMD_NOTIFY_TRAFFIC_TPUT_DETECT_EVENT,
	DBG_TXCMD_NOTIFY_EVENT_MAX
};

struct dbg_txcmd_notify_event {
	VOID *out_data;
	ULONG threshold;
	BOOLEAN is_wait;
	BOOLEAN is_check;
	UINT sta_id;
	RTMP_OS_COMPLETION done;
};

struct dbg_txcmd_framework {
	struct _DL_LIST dbg_txcmd_head;
	BOOLEAN dbg_txcmd_framework_state;
	struct _RTMP_ADAPTER *ad;
	struct dbg_txcmd_notify_event notify_table[DBG_TXCMD_NOTIFY_EVENT_MAX];
};

#define DBG_TXCMD_LOG(_fmt, _arg...)\
		printk(KERN_INFO "[DBG_TXCMD][%10s]: %s@"_fmt"\n", DBG_TXCMD_MODNAME, __func__, ## _arg)

typedef INT (*dbg_txcmd_fun)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg);

struct dbg_txcmd_feature_entry {
	struct _DL_LIST list;
	CHAR feature_name[32];
	UINT dbg_txcmd_cnt;
	dbg_txcmd_fun *dbg_txcmd_table;
};

struct dbg_wmcu_request {
	UINT32 feature_id;
	UINT32 type;
	UINT32 len;
	UINT32 resp_len;
	UCHAR *payload;
	UCHAR *resp;
	VOID (*resp_handle)(struct cmd_msg *msg, char *data, UINT16 len);
};

/*export to features*/
INT dbg_txcmd_feature_register(struct dbg_txcmd_framework *dbg_txcmd_ctrl, struct dbg_txcmd_feature_entry *entry);
VOID dbg_txcmd_feature_unregister(struct dbg_txcmd_feature_entry *entry);
INT32 dbg_ut_wmcu_send(struct _RTMP_ADAPTER *ad, struct dbg_wmcu_request *request);

#endif

