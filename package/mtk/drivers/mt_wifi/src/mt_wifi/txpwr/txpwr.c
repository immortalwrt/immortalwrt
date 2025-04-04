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
	txpwr.c
*/

/*******************************************************************************
 *    INCLUDED COMMON FILES
 ******************************************************************************/

#include "rt_config.h"

/*******************************************************************************
 *    INCLUDED EXTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *    INCLUDED INTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
INT32 MtCmdPwrLimitTblUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1Type,
	UINT8 u1ChannelBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel
)
{
	struct cmd_msg *msg;
	CMD_POWER_LIMIT_TABLE_CTRL_T rPwrLimitTblCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "u1Type: %d, u1BandIdx: %d, u1ChannelBand: %d, u1ControlChannel: %d, u1CentralChannel: %d\n",
			 u1Type, u1BandIdx, u1ChannelBand, u1ControlChannel, u1CentralChannel);
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&rPwrLimitTblCtrl, sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V1)
		rPwrLimitTblCtrl.u1PowerCtrlFormatId = POWER_LIMIT_TABLE_CTRL;
	rPwrLimitTblCtrl.u1PwrLimitType 	 = u1Type;
	rPwrLimitTblCtrl.u1BandIdx			 = u1BandIdx;

	if (cap->txpower_type == TX_POWER_TYPE_V1)
	/* Fill Power Limit Parameters to CMD payload */
		MtPwrFillLimitParam(pAd, u1ChannelBand, u1ControlChannel,
				u1CentralChannel, &rPwrLimitTblCtrl.uPwrLimitTbl, u1Type);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rPwrLimitTblCtrl,
						sizeof(CMD_POWER_LIMIT_TABLE_CTRL_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT32 MtCmdTxPwrUpCtrl(
	RTMP_ADAPTER *pAd,
	INT8          ucBandIdx,
	CHAR          cPwrUpCat,
	signed char   *cPwrUpValue,
	UCHAR			cPwrUpValLen)
{
	struct cmd_msg *msg;
	CMD_POWER_BOOST_TABLE_CTRL_T TxPwrUpTblCtrl;
	INT32 ret = 0;
	UCHAR i = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: ucBandIdx: %d, cPwrUpCat: %d, cPwrUpValLen: %d\n",
		 __func__, ucBandIdx, cPwrUpCat, cPwrUpValLen);

	for (i = 0; i < cPwrUpValLen; i++)
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: cPwrUpValue: %d\n", __func__, *(cPwrUpValue + i));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* init buffer structure */
	os_zero_mem(&TxPwrUpTblCtrl, sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));
	if (cap->txpower_type == TX_POWER_TYPE_V0)
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V0;
	else
		TxPwrUpTblCtrl.ucPowerCtrlFormatId = TXPOWER_UP_TABLE_CTRL_V1;
	TxPwrUpTblCtrl.ucBandIdx           = ucBandIdx;
	TxPwrUpTblCtrl.cPwrUpCat           = cPwrUpCat;

	/* update Power Up Table value to buffer structure */
	os_move_mem(TxPwrUpTblCtrl.cPwrUpValue, cPwrUpValue,
			sizeof(CHAR) * cPwrUpValLen);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TX_POWER_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&TxPwrUpTblCtrl,
		sizeof(CMD_POWER_BOOST_TABLE_CTRL_T));

	ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s:(ret = %d)\n", __func__, ret);

	return ret;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

