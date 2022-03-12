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

#ifndef __FRAMEWORK_DVT_H__
#define __FRAMEWORK_DVT_H__

struct _RTMP_ADAPTER;
struct _DL_LIST;
struct dvt_framework;

enum {
	DVT_STATUS_OK,
	DVT_STATUS_FAIL,
	DVT_STATUS_RESOUCE_ERROR,
	DVT_STATUS_CONN_FAIL,
	DVT_STATUS_REQ_MCPU_FAIL,
	DVT_STATUS_FINAL
};

enum {
	DVT_NOTIFY_WSYS_CONNECT_EVENT,
	DVT_NOTIFY_WSYS_DISCONN_EVENT,
	DVT_NOTIFY_WSYS_CLOSE_EVENT,
	DVT_NOTIFY_WSYS_OPEN_EVENT,
	DVT_NOTIFY_WSYS_LINKUP_EVENT,
	DVT_NOTIFY_WSYS_LINKDOWN_EVENT,
	DVT_NOTIFY_WSYS_STAUPDATE_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_DATA_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_CMD_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_TRX_FREE_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_TMR_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_TRXV_EVENT,
	DVT_NOTIFY_TRAFFIC_RX_TXS_EVENT,
	DVT_NOTIFY_TRAFFIC_WMM_DETECT_EVENT,
	DVT_NOTIFY_TRAFFIC_TPUT_DETECT_EVENT,
	DVT_NOTIFY_EVENT_MAX
};

enum {
	DVT_STA_WMM_CAP = 1 << 0,
};

struct dvt_seudo_sta {
	struct wifi_dev *wdev;
	UINT32 cap_flag;
	USHORT phy_mode;
	UCHAR addr[MAC_ADDR_LEN];
	/*for maintain*/
	struct _MAC_TABLE_ENTRY *mac_entry;
};

struct dvt_notify_event {
	VOID *out_data;
	ULONG threshold;
	BOOLEAN is_wait;
	BOOLEAN is_check;
	UINT sta_id;
	RTMP_OS_COMPLETION done;
};

struct dvt_framework {
	struct _DL_LIST dvt_head;
	BOOLEAN dvt_framework_state;
	struct _RTMP_ADAPTER *ad;
	struct dvt_notify_event notify_table[DVT_NOTIFY_EVENT_MAX];
};

#define DVT_LOG(_fmt, _arg...)\
	printk(KERN_INFO "[DVT][%10s]: %s@"_fmt"\n", DVT_MODNAME, __func__, ## _arg)


typedef INT (*dvt_fun)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg);

struct dvt_feature_entry {
	struct _DL_LIST list;
	CHAR feature_name[32];
	UINT dvt_cnt;
	dvt_fun *dvt_table;
};

struct dvt_wmcu_request {
	UINT32 feature_id;
	UINT32 type;
	UINT32 len;
	UINT32 resp_len;
	UCHAR *payload;
	UCHAR *resp;
	VOID (*resp_handle)(struct cmd_msg *msg, char *data, UINT16 len);
};

/*export to features*/
INT dvt_feature_register(struct dvt_framework *dvt_ctrl, struct dvt_feature_entry *entry);
VOID dvt_feature_unregister(struct dvt_feature_entry *entry);
INT dvt_ut_seudo_sta_connect(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta);
VOID dvt_ut_seudo_sta_template_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct dvt_seudo_sta *sta);
VOID dvt_ut_seudo_sta_disconnect(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta);
VOID dvt_ut_seudo_sta_security_set(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta);
VOID *dvt_ut_notify_wait(struct _RTMP_ADAPTER *ad, UINT signal);
VOID *dvt_ut_notify_wait_threshold(struct _RTMP_ADAPTER *ad, UINT signal, UINT sta_id, ULONG threshold);
INT32 dvt_ut_wmcu_send(struct _RTMP_ADAPTER *ad, struct dvt_wmcu_request *request);
INT32 dvt_txcmd_wmcu_send(struct _RTMP_ADAPTER *ad, struct dvt_wmcu_request *request);

#endif
