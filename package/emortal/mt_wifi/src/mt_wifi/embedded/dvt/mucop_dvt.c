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
#include "mucop_dvt.h"
#include "framework_dvt.h"

#define DVT_MODNAME "MUCOP"

#define DVT_TYPE_DL_AC_LEN              0
#define DVT_TYPE_UL_TID_LEN             1
#define DVT_TYPE_PFID_TABLE             2
#define DVT_TYPE_GROUP_TABLE            3
#define DVT_TYPE_CLUSTER_TABLE          4
#define DVT_TYPE_GROUP_SELECT_CONTROL   5
#define DVT_TYPE_TRIGGER_TABLE          6
#define DVT_TYPE_OUTPUT_TABLE           7

struct dvt_apps_info {
	UINT32 p0;
	UINT32 p1;
	UINT32 p2;
	UINT32 p3;
	UINT32 p4;
	UINT32 p5;
	UINT32 p6;
	UINT32 p7;
	UINT32 p8;
	UINT32 p9;
};
/*
 *  ==========================================================================
 *  Description:
 *  Send the all parameter packet.
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_send(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg, UINT32 dvt_type)
{
	struct dvt_wmcu_request request;
	struct dvt_apps_info    apps;
	struct dvt_apps_info    apps_resp;
	char                    *str;
	UINT32                  *get_apps;
	UINT32                  get_arg;
	INT32                   ret;

	memset(&apps, 0, sizeof(apps));
	memset(&apps_resp, 0, sizeof(apps_resp));

	get_apps = &apps.p0;

	DVT_LOG("arg:%s", arg);

	while ((str = strsep((char **)&arg, "-")) != NULL) {
		get_arg = simple_strtol(str, NULL, 10);
		DVT_LOG("get_arg,%d", get_arg);
		*get_apps++ = get_arg;
	}

	request.feature_id = ENUM_SYSDVT_MURU;
	request.type = dvt_type;
	request.len = sizeof(apps);
	request.payload = (UCHAR *) &apps;
	request.resp_handle = NULL;
	request.resp = (UCHAR *) &apps_resp;
	/*request.resp_len = sizeof(apps_resp) + 4;*/
	request.resp_len = sizeof(apps_resp);

	ret = dvt_ut_wmcu_send(ad, &request);
	if (ret != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
		return ret;
	}

	return ret;
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill DownLink AC table
 *  iwpriv ra0 set dvt=mucop-1-wlanIdx-AC1Len-AC2Len-AC3Len-AC4Len
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_0_get_dlacqlen(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_DL_AC_LEN * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill DownLink AC table
 *  iwpriv ra0 set dvt=mucop-2-wlanIdx-AC1Len-AC2Len-AC3Len-AC4Len
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_1_get_ultidqlen(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_UL_TID_LEN * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill PFID table
 *  iwpriv ra0 set dvt=mucop-3-PFID-WlanIdx
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_2_pfid_table(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_PFID_TABLE * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill Group table
 *  iwpriv ra0 set dvt=mucop-4-0-NumUser-GI-AX-PFID0-PFID1-PFID2-PFID3-DLVLD-ULVLD
 *  iwpriv ra0 set dvt=mucop-4-1-RUAlloc-NS0-NS1-NS2-NS3
 *  iwpriv ra0 set dvt=mucop-4-2-DLMCSU0-DLMCSU1-DLMCSU2-DLMCSU3-DLWFU0-DLWFU1-DLWFU2-DLWFU3
 *  iwpriv ra0 set dvt=mucop-4-3-ULMCSU0-ULMCSU1-ULMCSU2-ULMCSU3-ULWFU0-ULWFU1-ULWFU2-ULWFU3
 *  RUAlloc
 *   1:0x82 = 130
 *   2:0x84 = 132
 *   3:0x86 = 134
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_3_group_table(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_GROUP_TABLE * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill Cluster table
 *  iwpriv ra0 set dvt=mucop-5-PFID-GID-UP
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_4_cluster_table(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_CLUSTER_TABLE * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill Cluster table
 *  iwpriv ra0 set dvt=mucop-4-PFID-GID-UP
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_get_arg(RTMP_STRING *arg)
{
	return 0;
}
/*
 *  ==========================================================================
 *  Description:
 *  Fill MUCOP trigger CR
 *  iwpriv ra0 set dvt=mucop-6-0-MaxGrpSrch-priSecMcsRatio-2ndAcPolicy-MaxNss-MinNss-BsrSizeOffset-TXOPLimit
 *  iwpriv ra0 set dvt=mucop-6-1-PFIDBitmap
 *  iwpriv ra0 set dvt=mucop-6-2-ax-ULDL-RUAlloc-PrimPFID-PriAC
 *  iwpriv ra0 set dvt=mucop-6-3-VLD-MX-AC-SPL-SU-2MU-3MU-4MU-TRG
 *  RUAlloc
 *   1:0x82
 *   2:0x84
 *   3:0x86
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_5_group_select_control(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_GROUP_SELECT_CONTROL * 10 + sub_type;

	if (sub_type == 2)
		mucop_dvt_get_arg(arg);

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Trigger table.
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_6_trigger_table(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_TRIGGER_TABLE * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Output table.
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static INT mucop_dvt_7_output_table(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	char    *str;
	UINT8   type;
	UINT8   sub_type;

	DVT_LOG("arg,%s", arg);

	str = strsep((char **)&arg, "-");
	sub_type = simple_strtol(str, NULL, 10);
	DVT_LOG("sub_type,%d", sub_type);
	type = DVT_TYPE_OUTPUT_TABLE * 10 + sub_type;

	return mucop_dvt_send(ad, arg, type);
}
/*
 *  ==========================================================================
 *  Description:
 *  Feature table definition.
 *
 *  Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *  ==========================================================================
 */
static dvt_fun mucop_dvt_table[] = {
	mucop_dvt_0_get_dlacqlen,
	mucop_dvt_1_get_ultidqlen,
	mucop_dvt_2_pfid_table,
	mucop_dvt_3_group_table,
	mucop_dvt_4_cluster_table,
	mucop_dvt_5_group_select_control,
	mucop_dvt_6_trigger_table,
	mucop_dvt_7_output_table,
};
/*
 *  ==========================================================================
 *  Description:
 *  Feature entry.
 *
 *  Return:
 *  N/A
 *  ==========================================================================
 */
static struct dvt_feature_entry mucop_dvt = {
	.feature_name = "mucop",
	.dvt_cnt = sizeof(mucop_dvt_table)/sizeof(dvt_fun),
	.dvt_table = mucop_dvt_table,
};
/*
 *  ==========================================================================
 *  Description:
 *  Initial feature function.
 *
 *  Return:
 *  N/A
 *  ==========================================================================
 */
VOID mucop_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &mucop_dvt);
	DVT_LOG("init,ok");
}

