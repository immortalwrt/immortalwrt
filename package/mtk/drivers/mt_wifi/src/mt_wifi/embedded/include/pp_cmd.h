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
	pp_cmd.h
*/

#ifndef __CMM_PP_CMD_H__
#define __CMM_PP_CMD_H__

#ifdef CFG_SUPPORT_FALCON_PP
/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "rt_config.h"
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
/** PP Command */
typedef enum _ENUM_PP_CMD_SUBID {
	/** SET **/
	PP_CMD_Reserve = 0x0,
	PP_CMD_SET_PP_CAP_CTRL,
	PP_CMD_NUM
} ENUM_PP_CMD_SUBID, *P_ENUM_PP_CMD_SUBID;

/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    TYPES
 ******************************************************************************/
typedef struct _PP_CMD_T {
	UINT_8	cmd_sub_id;
	UINT_8	dbdc_idx;
	UINT_8	pp_ctrl;
	UINT_8	pp_auto_mode;
	UINT_8	pp_bitmap;	/* for BW160 */
	UINT_8	au1reserved[3];
} PP_CMD_T, *P_PP_CMD_T;

typedef enum _ENUM_PP_CTRL_T {
    /** SET **/
    PP_CTRL_PP_DIS = 0x0,
    PP_CTRL_PP_EN,
    PP_CTRL_SU_AUTOBW_EN,
    PP_CTRL_NUM
} ENUM_PP_CTRL_T, *P_ENUM_PP_CTRL_T;

/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS pp_mbss_init(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
NDIS_STATUS pp_profile_pp_en(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS set_pp_cap_ctrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS pp_cmd_cap_ctrl(IN PRTMP_ADAPTER pAd, IN P_PP_CMD_T pp_cmd_cap);
#endif				/* CFG_SUPPORT_FALCON_PP */
#endif				/* __CMM_PP_CMD_H__ */
