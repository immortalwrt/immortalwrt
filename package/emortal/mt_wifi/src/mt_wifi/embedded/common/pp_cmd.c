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
	pp_cmd.c
*/
#include "rt_config.h"

#ifdef CFG_SUPPORT_FALCON_PP

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
/** FW & DRV sync with pp_cmd.c **/
/** PP Command */
typedef enum _ENUM_PP_CMD_SUBID {
    /** SET **/
    PP_CMD_Reserve = 0x0,
    PP_CMD_SET_PP_CAP_CTRL,
    PP_CMD_NUM
} ENUM_PP_CMD_SUBID, *P_ENUM_PP_CMD_SUBID;

/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
/* For Command*/

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

NDIS_STATUS pp_mbss_init(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	UINT_8 dbdc_idx = BAND0;
	PP_CMD_T pp_cmd_cap;
	UINT_8 pp_ctrl = 0;

	if (wdev != NULL)
		dbdc_idx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	/* ap.c will call this command enable PP by profile */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		 ("%s: Preamble puncture initialize via profile.\n",
		  __func__));

	/* Set PPEnable Part */

	pp_ctrl = pAd->CommonCfg.pp_enable[dbdc_idx];

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));
	/* Assign Cmd Id */
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_CAP_CTRL;
	pp_cmd_cap.dbdc_idx = dbdc_idx;
	pp_cmd_cap.pp_ctrl = pp_ctrl;

	if (pp_ctrl == PP_CTRL_PP_EN)
		pp_cmd_cap.pp_auto_mode = TRUE;
	else
		pp_cmd_cap.pp_auto_mode = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		 ("%s: pp_en %d, pp_auto_mode %d\n",
		  __func__, pp_cmd_cap.pp_ctrl, pp_cmd_cap.pp_auto_mode));

	status = pp_cmd_cap_ctrl(pAd, &pp_cmd_cap);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s: Set pp_enable[%d]=%d Fail!\n", __func__, dbdc_idx,
		  pAd->CommonCfg.pp_enable[dbdc_idx]));
	}
	/* End - Set PPEnable Part */

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS pp_profile_pp_en(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	UINT8 band_idx;
	RTMP_STRING *ptr;

	for (band_idx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), band_idx++) {
		if (band_idx > BAND_NUM)
			return NDIS_STATUS_INVALID_DATA;

		switch (pAd->CommonCfg.eDBDC_mode) {
		case ENUM_SingleBand:
			pAd->CommonCfg.pp_enable[BAND0] = simple_strtol(ptr, 0, 10);
			break;
#ifdef DBDC_MODE
		case ENUM_DBDC_5G2G:
			if (band_idx == BAND0)
				pAd->CommonCfg.pp_enable[BAND1] = simple_strtol(ptr, 0, 10);
			else
				pAd->CommonCfg.pp_enable[BAND0] = simple_strtol(ptr, 0, 10);
			break;

		case ENUM_DBDC_2G5G:
		case ENUM_DBDC_5G5G:
#endif /* DBDC_MODE */
		default:
			pAd->CommonCfg.pp_enable[band_idx] = simple_strtol(ptr, 0, 10);
			break;
		}
	}
	return status;
}

NDIS_STATUS set_pp_cap_ctrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	PP_CMD_T pp_cmd_cap;
	INT32 recv = 0;
	UINT32 pp_ctrl = 0;
	UINT32 pp_auto = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&pp_cmd_cap, sizeof(PP_CMD_T));

	/* Assign Cmd Id */
	pp_cmd_cap.cmd_sub_id = PP_CMD_SET_PP_CAP_CTRL;
	pp_cmd_cap.dbdc_idx = u1DbdcIdx;

	if (arg) {
		recv = sscanf(arg, "%d-%d", &(pp_auto), &(pp_ctrl));

		if (recv != 2) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"PpAuto-PpEn\n"));
			return TRUE;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: pp_auto %d, pp_ctrl %d\n", __func__, pp_auto, pp_ctrl));

		if (pp_auto != 0)
			pp_cmd_cap.pp_auto_mode = 1;
		else
			pp_cmd_cap.pp_auto_mode = 0;

		pp_cmd_cap.pp_ctrl = (UINT_8)pp_ctrl;
		status = pp_cmd_cap_ctrl(pAd, &pp_cmd_cap);

	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		status = NDIS_STATUS_FAILURE;

	}

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv rax0 set ppcapctrl=[PpAuto]-[PpEn]\n", __func__));
	}

	return status;
}

/* for set/show function*/
NDIS_STATUS pp_cmd_cap_ctrl(IN PRTMP_ADAPTER pAd, IN P_PP_CMD_T pp_cmd_cap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check pp_cmd_cap not null */
	if (!pp_cmd_cap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(PP_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: cmd_sub_id = %d, dbdc_idx %d, pp_ctrl %d, pp_auto_mode %d\n",
		__func__,
		pp_cmd_cap->cmd_sub_id,
		pp_cmd_cap->dbdc_idx,
		pp_cmd_cap->pp_ctrl,
		pp_cmd_cap->pp_auto_mode));

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PP_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)pp_cmd_cap, sizeof(PP_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

#endif				/* CFG_SUPPORT_FALCON_PP */
