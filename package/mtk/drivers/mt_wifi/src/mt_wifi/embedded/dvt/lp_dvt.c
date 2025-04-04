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
#include "lp_dvt.h"
#include "framework_dvt.h"

#define DVT_MODNAME "LP"


static INT lp_dvt_1_basic_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;

	DVT_LOG("arg,%s", arg);

	req.feature_id = ENUM_SYSDVT_LP;
	req.type = os_str_tol(arg, 0, 10);
	req.len = 0;
	req.payload = NULL;
	req.resp_handle = NULL;
	req.resp_len = 0;

	dvt_ut_wmcu_send(ad, &req);

	return DVT_STATUS_OK;
}


static dvt_fun lp_dvt_table[] = {
	lp_dvt_1_basic_test,
};

static struct dvt_feature_entry lp_dvt = {
	.feature_name = "lp",
	.dvt_cnt = sizeof(lp_dvt_table)/sizeof(dvt_fun),
	.dvt_table = lp_dvt_table,
};

VOID lp_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &lp_dvt);
	DVT_LOG("lp init,ok");
}
