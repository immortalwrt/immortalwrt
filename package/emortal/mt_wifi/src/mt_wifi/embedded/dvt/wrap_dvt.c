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
#include "wrap_dvt.h"
#include "framework_dvt.h"

#define DVT_MODNAME "WRAP"

static struct dvt_seudo_sta sta;

static INT wrap_dvt_1_6_basic_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	/*create a template seudo sta*/
	dvt_ut_seudo_sta_template_get(ad, wdev, &sta);
	if (dvt_ut_seudo_sta_connect(ad, &sta) != DVT_STATUS_OK) {
		DVT_LOG("create a seudo sta,fail!");
	}
	return DVT_STATUS_OK;
}

static INT wrap_dvt_7_10_sw_rx_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	dvt_ut_seudo_sta_disconnect(ad, &sta);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_11_15_sw_tx_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	INT wcid;
	struct _STA_TR_ENTRY *tr_entry;
	struct _MAC_TABLE_ENTRY *entry;
	ULONG threshold_tput = 1;

	DVT_LOG("arg,%s", arg);
	wcid = simple_strtol(arg, NULL, 10);
	tr_entry = dvt_ut_notify_wait(ad, DVT_NOTIFY_WSYS_CONNECT_EVENT);
	if (tr_entry && tr_entry->wcid == wcid)
		DVT_LOG("result,ok");
	else
		DVT_LOG("result,fail");

	entry = dvt_ut_notify_wait_threshold(ad, DVT_NOTIFY_TRAFFIC_TPUT_DETECT_EVENT, wcid, threshold_tput);
	if (!entry) {
		return DVT_STATUS_FAIL;
	}
	DVT_LOG("wcid,%d&one_sec_pkt_cnt,%lu", entry->wcid, entry->one_sec_tx_pkts);
	return DVT_STATUS_OK;
}

struct dvt_apps_info {
	UINT32 test1;
	UINT32 test2;
	UINT32 test3;
	UINT32 test4;
};

#define APPS_TYPE_TEST 0

static INT wrap_dvt_16_24_hw_tx_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request request;
	struct dvt_apps_info apps, apps_resp;
	DVT_LOG("arg,%s", arg);
	apps.test1 = 35;
	apps.test2 = 36;
	apps.test3 = 37;
	apps.test4 = 38;
	request.feature_id = ENUM_SYSDVT_APPS;
	request.len = sizeof(apps);
	request.payload = (UCHAR *) &apps;
	request.resp_handle = NULL;
	request.resp = (UCHAR *) &apps_resp;
	request.resp_len = sizeof(apps_resp);
	request.type = APPS_TYPE_TEST;
	if (dvt_ut_wmcu_send(ad, &request) != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
	}

	DVT_LOG("resp: %d,%d,%d,%d",
		apps_resp.test1,
		apps_resp.test2,
		apps_resp.test3,
		apps_resp.test4);

	return DVT_STATUS_OK;
}

static INT wrap_dvt_25_28_dynamc_txbuf_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_29_30_cr_mirror_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_31_48_peak_tput_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_49_52_reset_flow_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_53_60_dual_card_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static INT wrap_dvt_61_64_advance_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	DVT_LOG("arg,%s", arg);
	return DVT_STATUS_OK;
}

static dvt_fun wrap_dvt_table[] = {
	wrap_dvt_1_6_basic_test,
	wrap_dvt_7_10_sw_rx_test,
	wrap_dvt_11_15_sw_tx_test,
	wrap_dvt_16_24_hw_tx_test,
	wrap_dvt_25_28_dynamc_txbuf_test,
	wrap_dvt_29_30_cr_mirror_test,
	wrap_dvt_31_48_peak_tput_test,
	wrap_dvt_49_52_reset_flow_test,
	wrap_dvt_53_60_dual_card_test,
	wrap_dvt_61_64_advance_test
};

static struct dvt_feature_entry wrap_dvt = {
	.feature_name = "wrap",
	.dvt_cnt = sizeof(wrap_dvt_table)/sizeof(dvt_fun),
	.dvt_table = wrap_dvt_table,
};

VOID wrap_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &wrap_dvt);
	DVT_LOG("init,ok");
}
