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
 *  $Id$
 *  $DateTime$
 *  Jeffrey Chang
 */
/*
     This file contains IOCTL for MU-MIMO specfic commands
 */
/*******************************************************************************
 * ******************************************************************************
 */
/******************************************************************************
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
 * ******************************************************************************
 */
#include "rt_config.h"
#ifdef CFG_SUPPORT_MU_MIMO
/* For debugging, Not for ATE */
#define SU 0
#define MU 1
#define MU_PROFILE_NUM  32 /* to be removed */
#define QD_RAW_DATA_LEN 56
static VOID eventDispatcher(struct cmd_msg *msg, char *rsp_payload,
		UINT16 rsp_payload_len);

INT SetMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pValid = NULL;
	PCHAR pIdx = NULL;
	PCHAR pBaMask = NULL;
	PCHAR pWlanIdx = NULL;
	CMD_MU_SET_MUPROFILE_ENTRY param = {0};
	UINT32 cmd = MU_SET_MUPROFILE_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "arg = %s\n", arg);
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pValid = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pBaMask = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pWlanIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.fgIsValid = (BOOLEAN)os_str_tol(pValid, 0, 10);
	param.u1Index = (UINT8)os_str_tol(pIdx, 0, 10);
	param.u1BaMask = (UINT8)os_str_tol(pBaMask, 0, 16);
	param.u1WlanIdx = (UINT8)os_str_tol(pWlanIdx, 0, 10);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	struct cmd_msg *msg = NULL;
	INT32 Ret = TRUE;
	PCHAR pIdx = NULL;
	UINT32 index = 0;
	EVENT_SHOW_MUPROFILE_ENTRY result = {0};
	UINT32 cmd = MU_GET_MUPROFILE_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIdx = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIdx, 0, 10);
	index = cpu2le32(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* | ucNumUser | ucBW | ucNS0 | ucNS1 | ucNS2 | ucNS3 |
 * | ucPFIDUser0 | ucPFIDUser1 | ucPFIDUser2 | ucPFIDUser3 |
 * | fgIsShortGI | fgIsUsed | fgIsDisable |
 * | ucInitMCSUser0 | ucInitMCSUser1 | ucInitMCSUser2 | ucIitMCSUser3|
 * | ucdMCSUser0 | ucdMCSUser1 | ucdMCSUser2 | ucdMCSUser3|
*/
/* iwpriv ra0 [index] [num_user: 0/1/2/3] [bw:/0/1/2/3] [ns0:0/1] [ns1:0/1] [ns2:0/1] [ns3:0/1]
   [pfid0] [pfid1] [pfid2] [pfid3] [sgi] [used] [dis] [initMcs0] [initMcs1]
   [initMcs2] [initMcs3] [dMcs0] [dMcs1] [dMcs2] [dMcs3]
*/
INT SetGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pNumUser = NULL;
	PCHAR pIndex = NULL;
	PCHAR pBw = NULL;
	PCHAR pNs0 = NULL;
	PCHAR pNs1 = NULL;
	PCHAR pNs2 = NULL;
	PCHAR pNs3 = NULL;
	PCHAR ppfid0 = NULL;
	PCHAR ppfid1 = NULL;
	PCHAR ppfid2 = NULL;
	PCHAR ppfid3 = NULL;
	PCHAR psgi = NULL;
	PCHAR pused = NULL;
	PCHAR pdis = NULL;
	PCHAR pinitMcsUser0 = NULL;
	PCHAR pinitMcsUser1 = NULL;
	PCHAR pinitMcsUser2 = NULL;
	PCHAR pinitMcsUser3 = NULL;
	PCHAR pdMcsUser0 = NULL;
	PCHAR pdMcsUser1 = NULL;
	PCHAR pdMcsUser2 = NULL;
	PCHAR pdMcsUser3 = NULL;
	CMD_MU_SET_GROUP_TBL_ENTRY param = {0};
	UINT32 cmd = MU_SET_GROUP_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if  (pch != NULL)
		pNumUser = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pBw = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNs3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		ppfid3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		psgi = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pused = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdis = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pinitMcsUser3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pdMcsUser2 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pdMcsUser3 = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u4Index = (UINT32)os_str_tol(pIndex, 0, 10);
	param.u1NumUser = (UINT8)os_str_tol(pNumUser, 0, 10);
	param.u1BW = (UINT8)os_str_tol(pBw, 0, 10);
	param.u1PFIDUser0 = (UINT8)os_str_tol(ppfid0, 0, 10);
	param.u1PFIDUser1 = (UINT8)os_str_tol(ppfid1, 0, 10);
	param.u1PFIDUser2 = (UINT8)os_str_tol(ppfid2, 0, 10);
	param.u1PFIDUser3 = (UINT8)os_str_tol(ppfid3, 0, 10);
	param.fgIsShortGI = (BOOLEAN) os_str_tol(psgi, 0, 10);
	param.fgIsUsed = (BOOLEAN) os_str_tol(pused, 0, 10);
	param.fgIsDisable = (BOOLEAN) os_str_tol(pdis, 0, 10);
	param.u1InitMcsUser0 = (UINT8)os_str_tol(pinitMcsUser0, 0, 10);
	param.u1InitMcsUser1 = (UINT8)os_str_tol(pinitMcsUser1, 0, 10);
	param.u1InitMcsUser2 = (UINT8)os_str_tol(pinitMcsUser2, 0, 10);
	param.u1InitMcsUser3 = (UINT8)os_str_tol(pinitMcsUser3, 0, 10);
	param.u1DMcsUser0 = (UINT8)os_str_tol(pdMcsUser0, 0, 10);
	param.u1DMcsUser1 = (UINT8)os_str_tol(pdMcsUser1, 0, 10);
	param.u1DMcsUser2 = (UINT8)os_str_tol(pdMcsUser2, 0, 10);
	param.u1DMcsUser3 = (UINT8)os_str_tol(pdMcsUser3, 0, 10);
	param.u1NS0 = (UINT8)os_str_tol(pNs0, 0, 10);
	param.u1NS1 = (UINT8)os_str_tol(pNs1, 0, 10);
	param.u1NS2 = (UINT8)os_str_tol(pNs2, 0, 10);
	param.u1NS3 = (UINT8)os_str_tol(pNs3, 0, 10);
#ifdef RT_BIG_ENDIAN
	param.u4Index = cpu2le32(param.u4Index);
#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Index:%d NumUser:%d BW:%d NS(0~3):%d %d %d %d\n",
			  param.u4Index, param.u1NumUser, param.u1BW,
			  param.u1NS0, param.u1NS1, param.u1NS2, param.u1NS3);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "PFID(0~3):%d %d %d %d SGI:%d IsUsed:%d IsDisable:%d\n",
			  param.u1PFIDUser0, param.u1PFIDUser1, param.u1PFIDUser2,
			  param.u1PFIDUser3, param.fgIsShortGI, param.fgIsUsed,
			  param.fgIsDisable);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "InitMCS(0~3):%d %d %d %d DMCS(0~3):%d %d %d %d\n",
			  param.u1InitMcsUser0, param.u1InitMcsUser1, param.u1InitMcsUser2,
			  param.u1InitMcsUser3, param.u1DMcsUser0, param.u1DMcsUser1,
			  param.u1DMcsUser2, param.u1DMcsUser3);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_GROUP_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_GROUP_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Index is: %d\n", index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*<-------------------------------------------------------------------------->*/
/* iwpriv ra0 [index] [lowMemberStatus] [highMemberStatus] [[lowUserPosition0]
	[lowUserPosition1][highUserPosistion0] [highUserPosistion1] - in hex
*/
INT SetClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	PCHAR pLowGidUserMemberStatus = NULL;
	PCHAR pHighGidUserMemberStatus = NULL;
	PCHAR pLowGidUserPosition0 = NULL;
	PCHAR pLowGidUserPosition1 = NULL;
	PCHAR pHighGidUserPosition0 = NULL;
	PCHAR pHighGidUserPosition1 = NULL;
	CMD_MU_SET_CLUSTER_TBL_ENTRY param = {0};
	UINT32 cmd = MU_SET_CLUSTER_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserMemberStatus = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pHighGidUserMemberStatus = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserPosition0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pLowGidUserPosition1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pHighGidUserPosition0 = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pHighGidUserPosition1 = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u1Index = (UINT8)os_str_tol(pIndex, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	/* do we need change edian? */
	param.au4GidUserMemberStatus[0] =
		(UINT32)os_str_tol(pLowGidUserMemberStatus, 0, 16);
	param.au4GidUserMemberStatus[1] =
		(UINT32)os_str_tol(pHighGidUserMemberStatus, 0, 16);
	param.au4GidUserPosition[0] =
		(UINT32)os_str_tol(pLowGidUserPosition0, 0, 16);
	param.au4GidUserPosition[1] =
		(UINT32)os_str_tol(pLowGidUserPosition1, 0, 16);
	param.au4GidUserPosition[2] =
		(UINT32)os_str_tol(pHighGidUserPosition0, 0, 16);
	param.au4GidUserPosition[3] =
		(UINT32)os_str_tol(pHighGidUserPosition1, 0, 16);
#ifdef RT_BIG_ENDIAN
	param.au4GidUserMemberStatus[0] = cpu2le32(param.au4GidUserMemberStatus[0]);
	param.au4GidUserMemberStatus[1] = cpu2le32(param.au4GidUserMemberStatus[1]);
	param.au4GidUserPosition[0] = cpu2le32(param.au4GidUserPosition[0]);
	param.au4GidUserPosition[1] = cpu2le32(param.au4GidUserPosition[1]);
	param.au4GidUserPosition[2] = cpu2le32(param.au4GidUserPosition[2]);
	param.au4GidUserPosition[3] = cpu2le32(param.au4GidUserPosition[3]);
#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Values: %d %d %d %d %d %d %d\n", param.u1Index,
			  param.au4GidUserMemberStatus[0], param.au4GidUserMemberStatus[1],
			  param.au4GidUserPosition[0], param.au4GidUserPosition[1],
			  param.au4GidUserPosition[2], param.au4GidUserPosition[3]);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "msg is sent\n");
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_MU_GET_CLUSTER_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_CLUSTER_TBL_ENTRY;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Index is: %d\n", index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetMuEnableProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 * @descriton: enable/1 or disable/0 the MU module
 */
/* iwpriv ra0 set_mu_enable [1/0] */
INT SetMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT8 value = 0;
	UINT32 cmd = MU_SET_ENABLE;
	CMD_MU_SET_ENABLE param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	value = (UINT8)os_str_tol(arg, NULL, 10);
	param.u1IsMuEnable = value;
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MU_SET_ENABLE));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &value, sizeof(value));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_SHOW_MU_ENABLE result = {0};
	UINT32 cmd = MU_GET_ENABLE;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(EVENT_SHOW_MU_ENABLE));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	PCHAR pch = NULL;
	PCHAR pMinVal = NULL;
	PCHAR pMaxVal = NULL;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_GROUP_USER_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_GROUP_USER_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pMinVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pMaxVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.min = os_str_tol(pMinVal, 0, 10);
	param.max = os_str_tol(pMaxVal, 0, 10);

	if (param.min < 2 || param.max > 4) {
		Ret = 0;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Min < 2 and Max > 4 is NOT allowed\n");
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Values %d %d\n", param.min, param.max);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_USER_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_GROUP_USER_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetCalculateInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_CALC_INIT_MCS;
	EVENT_STATUS result = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = cpu2le32(index);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *) &index, sizeof(index));
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetGroupNssProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	PCHAR pch = NULL;
	PCHAR pMinVal = NULL;
	PCHAR pMaxVal = NULL;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_GROUP_NSS_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_GROUP_NSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pMinVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "");

	if (pch != NULL)
		pMaxVal = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.min = os_str_tol(pMinVal, 0, 10);
	param.max = os_str_tol(pMaxVal, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Values %d %d\n", param.min, param.max);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_NSS_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_GROUP_NSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetTxReqMinTime
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 */
INT SetTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_TXREQ_MIN_TIME param = {0};
	UINT32 cmd = MU_SET_TXREQ_MIN_TIME;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 16);
	param.value = cpu2le16(value);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "Values %d\n", param.value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_TXREQ_MIN_TIME result = {0};
	UINT32 cmd = MU_GET_TXREQ_MIN_TIME;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Nitesh: In the function ShowCalcInitMCSProc
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it shows the Init MCS value for the corresponding Group Index
 */
INT ShowCalcInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT32 index = 0;
	EVENT_SHOW_GROUP_TBL_ENTRY result = {0};
	UINT32 cmd = MU_GET_CALC_INIT_MCS;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	index = cpu2le32(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&index, sizeof(index));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function MuSetSuNssCheck
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it checl the nss of the primary ac owner if su nss= mu nss
 */
INT SetSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_SU_NSS_CHECK;
	CMD_SET_NSS_CHECK param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value > 1)
		return FALSE;

	param.fgIsEnable = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function MuSetSuNssCheck
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT ShowSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_GET_SU_NSS_CHECK;
	EVENT_SHOW_NSS_CHECK result = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function send GID management frame
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *it triggers the GID management frame
 */
INT SetTriggerGIDMgmtFrameProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_GID_MGMT_FRAME;
	PCHAR pch = NULL;
	PCHAR pWlanIdx = NULL;
	PCHAR pGid = NULL;
	PCHAR pUp = NULL;
	CMD_SET_TRIGGER_GID_MGMT_FRAME param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pWlanIdx = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pGid = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	pch = strsep(&arg, "");

	if  (pch != NULL)
		pUp = pch;
	else {
		Ret = FALSE;
		goto error;
	}

	param.wlanIndex = os_str_tol(pWlanIdx, 0, 10);
#ifdef RT_BIG_ENDIAN
	param.wlanIndex = cpu2le16(param.wlanIndex);
#endif
	param.gid = os_str_tol(pGid, 0, 10);
	param.up = os_str_tol(pUp, 0, 10);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(value));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetTriggerSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_SND;
	CMD_SET_TRIGGER_SND param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}

INT SetMuNdpDeltaTxPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_NDP_DELTA_TXPWR;
	CMD_SET_DYN_NDP_TXPWR param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"arg is NULL\n");
		Ret = FALSE;
		goto error;
	}

	param.i1NdpTxPwr = (INT8)simple_strtol(arg, 0, 10);

	MTWF_PRINT("%s: NdpDeltaTxPwr = %d\n", __func__, param.i1NdpTxPwr);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}

INT SetTriggerBbpProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_BBP;
	CMD_SET_TRIGGER_BBP param = {0};
	UINT16 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value > 2048) {
		Ret = FALSE;
		goto error;
	}

	param.u2GroupIndex = cpu2le16(value); /* 0~3: 1 to 4 user grouping */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetTriggerGroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_GROUP;
	CMD_SET_TRIGGER_GROUP param = {0};
	UINT8 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);

	if (value < 2) {
		Ret = FALSE;
		goto error;
	}

	param.ucNum = (value - 1); /* 0~3: 1 to 4 user grouping */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetTriggerDegroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_DEGROUP;
	CMD_SET_TRIGGER_DEGROUP param = {0};
	UINT8 value = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.ucMuProfileIndex = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,  Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT SetTriggerMuTxProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_SET_TRIGGER_MU_TX;
	UINT32 index = 0;
	CMD_SET_TRIGGER_MU_TX_FRAME param = {0};
	PCHAR pch = NULL;
	PCHAR pAcIndex = NULL;
	PCHAR pNumOfStas = NULL;
	PCHAR pRound = NULL;
	PCHAR pRandom = NULL;
	PCHAR pWlanIndex = NULL;
	PCHAR pPayloadLength = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	/* we fetch the minimum first */
	pch = strsep(&arg, "_");

	if (pch != NULL)
		pAcIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pRandom = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if (pch != NULL)
		pNumOfStas = pch;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "_");

	if  (pch != NULL)
		pRound = pch;
	else {
		Ret = 0;
		goto error;
	}

	param.u1AcIndex = (UINT8)os_str_tol(pAcIndex, 0, 10);
	param.fgIsRandomPattern = (BOOLEAN)os_str_tol(pRandom, 0, 10);
	param.u4NumOfSTAs = (UINT32)os_str_tol(pNumOfStas, 0, 10);
	param.u4Round = (UINT32)os_str_tol(pRound, 0, 10);

	for (index = 0; index < param.u4NumOfSTAs; index++) {
		pch = strsep(&arg, "_");

		if (pch != NULL)
			pWlanIndex = pch;
		else {
			Ret = 0;
			goto error;
		}

		param.au1WlanIndexArray[index] = (UINT8)os_str_tol(pWlanIndex, 0, 10);
	}

	for (index = 0; index < param.u4NumOfSTAs; index++) {
		if (index != (param.u4NumOfSTAs - 1)) {
			pch = strsep(&arg, "_");

			if (pch != NULL)
				pPayloadLength = pch;
			else {
				Ret = 0;
				goto error;
			}
		} else {
			pch = strsep(&arg, "");

			if (pch != NULL)
				pPayloadLength = pch;
			else {
				Ret = 0;
				goto error;
			}
		}

		param.au4PayloadLength[index] =
			(UINT32)os_str_tol(pPayloadLength, 0, 10);
		param.au4PayloadLength[index] =
			cpu2le32(param.au4PayloadLength[index]);
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(param));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &param);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u4NumOfSTAs = cpu2le32(param.u4NumOfSTAs);
	param.u4Round = cpu2le32(param.u4Round);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetTxopDefault
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_TXOP_DEFAULT param = {0};
	UINT32 cmd = MU_SET_TXOP_DEFAULT;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le32(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_GET_TXOP_DEFAULT result = {0};
	UINT32 cmd = MU_GET_TXOP_DEFAULT;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetSuLossThreshold
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_SU_LOSS_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_SU_LOSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_SU_LOSS_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_SU_LOSS_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetMuGainThreshold
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_GAIN_THRESHOLD param = {0};
	UINT32 cmd = MU_SET_MU_GAIN_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_MU_GAIN_THRESHOLD result = {0};
	UINT32 cmd = MU_GET_MU_GAIN_THRESHOLD;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetSecondaryAcPolicy
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_SECONDARY_AC_POLICY param = {0};
	UINT32 cmd = MU_SET_SECONDARY_AC_POLICY;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le16(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_SECONDARY_AC_POLICY result = {0};
	UINT32 cmd = MU_GET_SECONDARY_AC_POLICY;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetGroupTblDmcsMask
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_GROUP_TBL_DMCS_MASK param = {0};
	UINT32 cmd = MU_SET_GROUP_TBL_DMCS_MASK;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = value;
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_GROUP_TBL_DMCS_MASK result = {0};
	UINT32 cmd = MU_GET_GROUP_TBL_DMCS_MASK;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/*
 * @Jeffrey: In the function SetMaxGroupSearchCnt
 *
 * @params: pAd, to provide Adapter
 * @params: arg, the command line strings
 *
 *
 */
INT SetMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value = 0;
	INT32 Ret = TRUE;
	/* prepare command message */
	struct cmd_msg *msg = NULL;
	CMD_SET_MU_MAX_GROUP_SEARCH_CNT param = {0};
	UINT32 cmd = MU_SET_MAX_GROUP_SEARCH_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	value = os_str_tol(arg, 0, 10);
	param.value = cpu2le32(value);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
/* @Jeffrey: In the function SetTxopDefault
*
* @params: pAd, to provide Adapter
* @params: arg, the command line strings
*
*
*/
INT ShowMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_MU_GET_MAX_GROUP_SEARCH_CNT result = {0};
	UINT32 cmd = MU_GET_MAX_GROUP_SEARCH_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(result));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}
INT ShowMuProfileTxStsCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	PCHAR pch = NULL;
	PCHAR pIndex = NULL;
	UINT16 index = 0;
	CMD_GET_MU_PFID_TXS_CNT param = {0};
	EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT result = {0};
	UINT32 cmd = MU_GET_MU_PROFILE_TX_STATUS_CNT;
	struct _CMD_ATTRIBUTE attr = {0};

	pch = arg;

	if (pch != NULL)
		pIndex = pch;
	else {
		Ret = 0;
		goto error;
	}

	index = os_str_tol(pIndex, 0, 10);
	param.u2PfidIndex = cpu2le16(index);
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(index));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "(Ret = %d\n", Ret);
	return Ret;
}

INT ShowHqaMURxPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;

	if (pAd) {
		MTWF_PRINT("%s:(MuRxCnt=%d\n", __func__, pAd->u4RxMuPktCount);
	}

	return Ret;
}

INT ShowHqaMUTxPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT32 u4MuCnt = 0;

	if (pAd) {
		if (IS_MT7626(pAd))
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR34+0x200, &u4MuCnt);
		else
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR34, &u4MuCnt);

		if (pAd->u4TxMuPktCount == 0xFFFFFFFF)
			pAd->u4TxMuPktCount = 0;

		pAd->u4TxMuPktCount += u4MuCnt;

		MTWF_PRINT("%s:(MuTxCnt=%d\n", __func__, pAd->u4TxMuPktCount);
	}

	return Ret;
}

static VOID ShowGroupTblEntryCallback(char *rsp_payload,
	UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_GROUP_TBL_ENTRY pGentry =
		(P_EVENT_SHOW_GROUP_TBL_ENTRY)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("Resp Payload: ");

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_PRINT(" %x", rsp_payload[ucI]);
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("group table index %d\n", pGentry->u4Index);
	MTWF_PRINT("Number of User  %d\n", pGentry->u1NumUser);
	MTWF_PRINT("BW %d\n", pGentry->u1BW);
	MTWF_PRINT("NS0 %d\n", pGentry->u1NS0);
	MTWF_PRINT("NS1 %d\n", pGentry->u1NS1);
	MTWF_PRINT("NS2 %d\n", pGentry->u1NS2);
	MTWF_PRINT("NS3 %d\n", pGentry->u1NS3);
	MTWF_PRINT("PFIDUser0  %d\n", pGentry->u1PFIDUser0);
	MTWF_PRINT("PFIDUser1  %d\n", pGentry->u1PFIDUser1);
	MTWF_PRINT("PFIDUser2  %d\n", pGentry->u1PFIDUser2);
	MTWF_PRINT("PFIDUser3  %d\n", pGentry->u1PFIDUser3);
	MTWF_PRINT("SGI  %d\n", pGentry->fgIsShortGI);
	MTWF_PRINT("USED  %d\n", pGentry->fgIsUsed);
	MTWF_PRINT("DISABLED  %d\n", pGentry->fgIsDisable);
	MTWF_PRINT("initMCS0  %d\n", pGentry->u1InitMcsUser0);
	MTWF_PRINT("initMCS1  %d\n", pGentry->u1InitMcsUser1);
	MTWF_PRINT("initMCS2  %d\n", pGentry->u1InitMcsUser2);
	MTWF_PRINT("initMCS3  %d\n", pGentry->u1InitMcsUser3);
	MTWF_PRINT("dMCS0  %d\n", pGentry->u1DMcsUser0);
	MTWF_PRINT("dMCS1  %d\n", pGentry->u1DMcsUser1);
	MTWF_PRINT("dMCS2  %d\n", pGentry->u1DMcsUser2);
	MTWF_PRINT("dMCS3  %d\n", pGentry->u1DMcsUser3);
	MTWF_PRINT("END");
}
/* | Valid |BA | WLAN INDEX| */
/* usage iwpriv ra0 set_mu_profile [valid] [index] [bamask] [wlanIdex] */
static VOID ShowMuProfileEntryCallback(char *rsp_payload,
									   UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_MUPROFILE_ENTRY pMentry =
		(P_EVENT_SHOW_MUPROFILE_ENTRY)rsp_payload;
	/* TODO: replaced printk to Driver logger */
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("mu table index %d\n", pMentry->u1Index);
	MTWF_PRINT("Valid  %d\n", pMentry->fgIsValid);
	MTWF_PRINT("BAMask %x\n", pMentry->u1BaMask);
	MTWF_PRINT("wlanIdx %d\n", pMentry->u1WlanIdx);
	MTWF_PRINT("rsp payload len %d\n", rsp_payload_len);
	MTWF_PRINT("Resp Payload: ");

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_PRINT(" %x", rsp_payload[ucI]);
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("END");
}
/* usage iwpriv ra0 set_mu_profile [valid] [index] [bamask] [wlanIdex] */
static VOID ShowMuEnableCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT8 ucI = 0;
	P_EVENT_SHOW_MU_ENABLE pMentry = (P_EVENT_SHOW_MU_ENABLE)rsp_payload;

	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("MU enable:%x", pMentry->fgIsEnable);
	MTWF_PRINT("Resp Payload: ");

	for (ucI = 0; ucI < rsp_payload_len; ucI++) {
		MTWF_PRINT(" %x", rsp_payload[ucI]);
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("END");
}
static VOID ShowGroupUserThresholdCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_USER_THRESHOLD pGentry =
		(P_EVENT_MU_GET_GROUP_USER_THRESHOLD)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("Group user Threshold minimum value: %x\n", pGentry->min);
	MTWF_PRINT("Group user Threshold maximum value: %x\n", pGentry->max);
	MTWF_PRINT("END");
}
static VOID ShowSecondaryAcPolicyCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_SECONDARY_AC_POLICY pGentry =
		(P_EVENT_MU_GET_SECONDARY_AC_POLICY)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("secondary ac policy value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowGroupTblDmcsMaskCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_TBL_DMCS_MASK pGentry =
		(P_EVENT_MU_GET_GROUP_TBL_DMCS_MASK)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("group table dmcs mask value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowMaxGroupSearchCntCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MAX_GROUP_SEARCH_CNT pGentry =
		(P_EVENT_MU_GET_MAX_GROUP_SEARCH_CNT)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("max. group search cnt value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowTxReqMinTimeCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_TXREQ_MIN_TIME pGentry =
		(P_EVENT_MU_GET_TXREQ_MIN_TIME)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("TxReqMinTime value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowMuProfileTxStsCntCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT pGentry =
		(P_EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT) rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("mu profile index: %x\n", pGentry->pfIndex);
	MTWF_PRINT("c(n,2) used: %x\n", pGentry->cn2used);
	MTWF_PRINT("c(n,2) rate down: %x\n", pGentry->cn2rateDown);
	MTWF_PRINT("c(n,2) delta mcs: %x\n", pGentry->cn2deltaMcs);
	MTWF_PRINT("c(n,2) tx fail count: %x\n", pGentry->cn2TxFailCnt);
	MTWF_PRINT("c(n,2) tx succes count: %x\n", pGentry->cn2TxSuccessCnt);
	MTWF_PRINT("c(n,3) used: %x\n", pGentry->cn3used);
	MTWF_PRINT("c(n,3) rate down: %x\n", pGentry->cn3rateDown);
	MTWF_PRINT("c(n,3) delta mcs: %x\n", pGentry->cn3deltaMcs);
	MTWF_PRINT("c(n,3) tx fail count: %x\n", pGentry->cn3TxFailCnt);
	MTWF_PRINT("c(n,3) tx succes count: %x\n", pGentry->cn3TxSuccessCnt);
	MTWF_PRINT("c(n,4) used: %x\n", pGentry->cn4used);
	MTWF_PRINT("c(n,4) rate down: %x\n", pGentry->cn4rateDown);
	MTWF_PRINT("c(n,4) delta mcs: %x\n", pGentry->cn4deltaMcs);
	MTWF_PRINT("c(n,4) tx fail count: %x\n", pGentry->cn4TxFailCnt);
	MTWF_PRINT("c(n,4) tx succes count: %x\n", pGentry->cn4TxSuccessCnt);
	MTWF_PRINT("END");
}
static VOID ShowSuNssCheckCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_SHOW_NSS_CHECK ptr = (P_EVENT_SHOW_NSS_CHECK)rsp_payload;

	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("SU NSS Check value: %x\n", ptr->fgIsEnable);
	MTWF_PRINT("END");
}
static VOID ShowGroupNssThresholdCallback(char *rsp_payload,
		UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_GROUP_NSS_THRESHOLD pGentry =
		(P_EVENT_MU_GET_GROUP_NSS_THRESHOLD)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("Group Nss Threshold minimum value: %x\n", pGentry->min);
	MTWF_PRINT("Group Nss Threshold maximum value: %x\n", pGentry->max);
	MTWF_PRINT("END");
}
static VOID ShowClusterTblEntryCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_CLUSTER_TBL_ENTRY pGentry =
		(P_EVENT_MU_GET_CLUSTER_TBL_ENTRY)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("Cluster index %d\n", pGentry->u1Index);
	MTWF_PRINT("GID User Membership Status [0] = %x\n",
			  pGentry->au4GidUserMemberStatus[0]);
	MTWF_PRINT("GID User Membership Status [1] = %x\n",
			  pGentry->au4GidUserMemberStatus[1]);
	MTWF_PRINT("GID User Position [0] = %x\n", pGentry->au4GidUserPosition[0]);
	MTWF_PRINT("GID User Position [1] = %x\n", pGentry->au4GidUserPosition[1]);
	MTWF_PRINT("GID User Position [2] = %x\n", pGentry->au4GidUserPosition[2]);
	MTWF_PRINT("GID User Position [3] = %x\n", pGentry->au4GidUserPosition[3]);
	MTWF_PRINT("END");
}
static VOID ShowTxopDefaultCallback(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_GET_TXOP_DEFAULT pGentry = (P_EVENT_GET_TXOP_DEFAULT)rsp_payload;

	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("TXOP default value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowSuLossThresholdCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_SU_LOSS_THRESHOLD pGentry =
		(P_EVENT_MU_GET_SU_LOSS_THRESHOLD)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("su loss threshold value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowMuGainThresholdCallback(char *rsp_payload,
										UINT16 rsp_payload_len)
{
	P_EVENT_MU_GET_MU_GAIN_THRESHOLD pGentry =
		(P_EVENT_MU_GET_MU_GAIN_THRESHOLD)rsp_payload;
	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("mu gain threshold value: %x\n", pGentry->value);
	MTWF_PRINT("END");
}
static VOID ShowStatusOfCommand(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_STATUS pGentry = (P_EVENT_STATUS)rsp_payload;

	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("status: %x\n", pGentry->status);
	MTWF_PRINT( "END");
}
static VOID ShowStatusOfHqaCommand(char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_STATUS pGentry = (P_EVENT_HQA_STATUS)rsp_payload;

	MTWF_PRINT("%s: \n", __func__);
	MTWF_PRINT("status: %x\n", pGentry->status);
	MTWF_PRINT("END");
}

INT32
hqa_wifi_test_mu_cal_init_mcs(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_INIT_MCS pParams
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_INIT_MCS;
	CMD_HQA_SET_INIT_MCS param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (pParams == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Params is NULL!!\n");
		Ret = -1;
		goto error;
	}

	param.num_of_user = pParams->num_of_user;
	param.bandwidth = pParams->bandwidth;
	param.nss_of_user0 = pParams->nss_of_user0;
	param.nss_of_user1 = pParams->nss_of_user1;
	param.nss_of_user2 = pParams->nss_of_user2;
	param.nss_of_user3 = pParams->nss_of_user3;
	param.pf_mu_id_of_user0 = pParams->pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pParams->pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pParams->pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pParams->pf_mu_id_of_user3;
	param.spe_index = pParams->spe_index;
	param.num_of_txer = pParams->num_of_txer;
	param.group_index = (UINT16)cpu2le16(pParams->group_index);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Num_User:%u, BW:%u, Nss[0~3]:%u %u %u %u\n",
		param.num_of_user, param.bandwidth, param.nss_of_user0,
		param.nss_of_user1, param.nss_of_user2, param.nss_of_user3);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"PFID[0~3]:%u %u %u %u Num_Txer:%u GroupIndex:%u\n",
		param.pf_mu_id_of_user0, param.pf_mu_id_of_user1,
		param.pf_mu_id_of_user2, param.pf_mu_id_of_user3, param.num_of_txer,
		param.group_index);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

static VOID hqa_wifi_test_mu_get_init_mcs_callback(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_INIT_MCS pEntry = NULL;
	P_MU_STRUCT_MU_GROUP_INIT_MCS pOutput = NULL;

	if (rsp_payload == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rsp_payload is null!!\n");
		return;
	}

	pEntry = (P_EVENT_HQA_INIT_MCS)rsp_payload;

	if (msg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is null!!\n");
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Error !! buffer not specified by cmd\n");
		return;
	}

	pOutput = (P_MU_STRUCT_MU_GROUP_INIT_MCS)msg->attr.rsp.wb_buf_in_calbk;
	pOutput->user0InitMCS = cpu2le32(pEntry->rEntry.user0InitMCS);
	pOutput->user1InitMCS = cpu2le32(pEntry->rEntry.user1InitMCS);
	pOutput->user2InitMCS = cpu2le32(pEntry->rEntry.user2InitMCS);
	pOutput->user3InitMCS = cpu2le32(pEntry->rEntry.user3InitMCS);
	MTWF_PRINT("%s: EVENT_HQA_INIT_MCS\n", __func__);
	MTWF_PRINT("EventId:%u InitMCS[user0~user3]:%u %u %u %u\n",
		pEntry->u4EventId, cpu2le32(pEntry->rEntry.user0InitMCS),
		cpu2le32(pEntry->rEntry.user1InitMCS),
		cpu2le32(pEntry->rEntry.user2InitMCS),
		cpu2le32(pEntry->rEntry.user3InitMCS));
}

INT32
hqa_wifi_test_mu_get_init_mcs(
	PRTMP_ADAPTER pAd,
	UINT32 groupIndex,
	P_MU_STRUCT_MU_GROUP_INIT_MCS poutput
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_CALC_GET_INIT_MCS_ENTRY param = {0};
	UINT32 cmd = MU_HQA_GET_CALC_INIT_MCS;
	struct _CMD_ATTRIBUTE attr = {0};

	if (poutput == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"poutput is NULL\n");
		Ret = -1;
		goto error;
	}

	param.groupIndex = cpu2le32(groupIndex);

	MTWF_PRINT("%s: GroupIndex:%u poutput:%p param.groupIndex:%u\n",
		__func__, groupIndex, poutput, param.groupIndex);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_INIT_MCS));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, poutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_LQ pParams
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_LQ;
	CMD_HQA_SET_MU_CALC_LQ param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (pParams == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Params is NULL!!\n");
		Ret = -1;
		goto error;
	}

	param.u1NumOfUser = pParams->num_of_user;
	param.u1BW = pParams->bandwidth;
	param.u1NssUser0 = pParams->nss_of_user0;
	param.u1NssUser1 = pParams->nss_of_user1;
	param.u1NssUser2 = pParams->nss_of_user2;
	param.u1NssUser3 = pParams->nss_of_user3;
	param.u1PfmuIdUser0 = pParams->pf_mu_id_of_user0;
	param.u1PfmuIdUser1 = pParams->pf_mu_id_of_user1;
	param.u1PfmuIdUser2 = pParams->pf_mu_id_of_user2;
	param.u1PfmuIdUser3 = pParams->pf_mu_id_of_user3;
	param.u1SpeIndex = pParams->spe_index;
	param.u1NumTxer = pParams->num_of_txer;
	param.u2GroupIndex = cpu2le16(pParams->group_index);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"CMD_HQA_SET_MU_CALC_LQ\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Num_User:%u, BW:%u, Nss[0~3]:%u %u %u %u\n",
		param.u1NumOfUser, param.u1BW, param.u1NssUser0,
		param.u1NssUser1, param.u1NssUser2, param.u1NssUser3);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"PFID[0~3]:%u %u %u %u Num_Txer:%u GroupIndex:%u\n",
		param.u1PfmuIdUser0, param.u1PfmuIdUser1,
		param.u1PfmuIdUser2, param.u1PfmuIdUser3, param.u1NumTxer,
		param.u2GroupIndex);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_su_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_SU_CALC_LQ pParams
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_CALC_SU_LQ;
	CMD_HQA_SET_SU_CALC_LQ param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (pParams == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Params is NULL!!\n");
		Ret = -1;
		goto error;
	}

	param.num_of_user = pParams->num_of_user;
	param.bandwidth = pParams->bandwidth;
	param.nss_of_user0 = pParams->nss_of_user0;
	param.pf_mu_id_of_user0 = pParams->pf_mu_id_of_user0;
	param.num_of_txer = pParams->num_of_txer;
	param.group_index = (UINT16)cpu2le16(pParams->group_index);
	param.spe_index = pParams->spe_index;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"CMD_HQA_SET_SU_CALC_LQ\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Nu_User:%u, BW:%u, Nss0:%u PFMUID:%u NTxer:%u GroupIndex:%u\n",
		param.num_of_user, param.bandwidth, param.nss_of_user0,
		param.pf_mu_id_of_user0, param.num_of_txer, param.group_index);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

static VOID hqa_wifi_test_mu_get_su_lq_callback(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_GET_SU_CALC_LQ pEntry = NULL;
	P_SU_STRUCT_LQ_REPORT pOutput = NULL;

	if (rsp_payload == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rsp_payload is null!!\n");
		return;
	}

	pEntry = (P_EVENT_HQA_GET_SU_CALC_LQ)rsp_payload;

	if (msg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is null!!\n");
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Error !! buffer not specified by cmd\n");
		return;
	}

	pOutput = (P_SU_STRUCT_LQ_REPORT)msg->attr.rsp.wb_buf_in_calbk;
	pOutput->au4LqReport[0] = cpu2le32(pEntry->rEntry.au4LqReport[0]);
	pOutput->au4LqReport[1] = cpu2le32(pEntry->rEntry.au4LqReport[1]);
	pOutput->au4LqReport[2] = cpu2le32(pEntry->rEntry.au4LqReport[2]);
	pOutput->au4LqReport[3] = cpu2le32(pEntry->rEntry.au4LqReport[3]);
	pOutput->au4LqReport[4] = cpu2le32(pEntry->rEntry.au4LqReport[4]);
	MTWF_PRINT("%s: EVENT_HQA_GET_SU_CALC_LQ\n", __func__);
	MTWF_PRINT("eventId %u\n", pEntry->u4EventId);
	MTWF_PRINT("lq_report[0] = 0x%x\n", cpu2le32(pEntry->rEntry.au4LqReport[0]));
	MTWF_PRINT("lq_report[1] = 0x%x\n", cpu2le32(pEntry->rEntry.au4LqReport[1]));
	MTWF_PRINT("lq_report[2] = 0x%x\n", cpu2le32(pEntry->rEntry.au4LqReport[2]));
	MTWF_PRINT("lq_report[3] = 0x%x\n", cpu2le32(pEntry->rEntry.au4LqReport[3]));
	MTWF_PRINT("lq_report[4] = 0x%x\n", cpu2le32(pEntry->rEntry.au4LqReport[4]));
}

INT32
hqa_wifi_test_su_get_lq(
	PRTMP_ADAPTER pAd,
	P_SU_STRUCT_LQ_REPORT pOutput
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_GET_CALC_SU_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	if (pOutput == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pOutput is NULL\n");
		Ret = -1;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"pOutput = %p\n", pOutput);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_GET_SU_CALC_LQ));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *) &cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

static VOID hqa_wifi_test_mu_get_lq_callback(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_GET_MU_CALC_LQ pEntry = NULL;
	P_MU_STRUCT_LQ_REPORT pOutput = NULL;

	if (rsp_payload == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rsp_payload is null!!\n");
		return;
	}

	pEntry = (P_EVENT_HQA_GET_MU_CALC_LQ)rsp_payload;

	if (msg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is null!!\n");
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Error !! buffer not specified by cmd\n");
		return;
	}

	pOutput = (P_MU_STRUCT_LQ_REPORT)msg->attr.rsp.wb_buf_in_calbk;
	pOutput->au4LqReport[0][0] = cpu2le32(pEntry->rEntry.au4LqReport[0][0]);
	pOutput->au4LqReport[0][1] = cpu2le32(pEntry->rEntry.au4LqReport[0][1]);
	pOutput->au4LqReport[0][2] = cpu2le32(pEntry->rEntry.au4LqReport[0][2]);
	pOutput->au4LqReport[0][3] = cpu2le32(pEntry->rEntry.au4LqReport[0][3]);
	pOutput->au4LqReport[0][4] = cpu2le32(pEntry->rEntry.au4LqReport[0][4]);
	pOutput->au4LqReport[1][0] = cpu2le32(pEntry->rEntry.au4LqReport[1][0]);
	pOutput->au4LqReport[1][1] = cpu2le32(pEntry->rEntry.au4LqReport[1][1]);
	pOutput->au4LqReport[1][2] = cpu2le32(pEntry->rEntry.au4LqReport[1][2]);
	pOutput->au4LqReport[1][3] = cpu2le32(pEntry->rEntry.au4LqReport[1][3]);
	pOutput->au4LqReport[1][4] = cpu2le32(pEntry->rEntry.au4LqReport[1][4]);
	pOutput->au4LqReport[2][0] = cpu2le32(pEntry->rEntry.au4LqReport[2][0]);
	pOutput->au4LqReport[2][1] = cpu2le32(pEntry->rEntry.au4LqReport[2][1]);
	pOutput->au4LqReport[2][2] = cpu2le32(pEntry->rEntry.au4LqReport[2][2]);
	pOutput->au4LqReport[2][3] = cpu2le32(pEntry->rEntry.au4LqReport[2][3]);
	pOutput->au4LqReport[2][4] = cpu2le32(pEntry->rEntry.au4LqReport[2][4]);
	pOutput->au4LqReport[3][0] = cpu2le32(pEntry->rEntry.au4LqReport[3][0]);
	pOutput->au4LqReport[3][1] = cpu2le32(pEntry->rEntry.au4LqReport[3][1]);
	pOutput->au4LqReport[3][2] = cpu2le32(pEntry->rEntry.au4LqReport[3][2]);
	pOutput->au4LqReport[3][3] = cpu2le32(pEntry->rEntry.au4LqReport[3][3]);
	pOutput->au4LqReport[3][4] = cpu2le32(pEntry->rEntry.au4LqReport[3][4]);

	MTWF_PRINT("%s:EVENT_HQA_GET_MU_CALC_LQ\n", __func__);
	MTWF_PRINT("eventId %u\n", pEntry->u4EventId);
	MTWF_PRINT("lq_report[0][0] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[0][0]));
	MTWF_PRINT("lq_report[0][1] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[0][1]));
	MTWF_PRINT("lq_report[0][2] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[0][2]));
	MTWF_PRINT("lq_report[0][3] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[0][3]));
	MTWF_PRINT("lq_report[0][4] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[0][4]));
	MTWF_PRINT("lq_report[1][0] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[1][0]));
	MTWF_PRINT("lq_report[1][1] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[1][1]));
	MTWF_PRINT("lq_report[1][2] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[1][2]));
	MTWF_PRINT("lq_report[1][3] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[1][3]));
	MTWF_PRINT("lq_report[1][4] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[1][4]));
	MTWF_PRINT("lq_report[2][0] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[2][0]));
	MTWF_PRINT("lq_report[2][1] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[2][1]));
	MTWF_PRINT("lq_report[2][2] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[2][2]));
	MTWF_PRINT("lq_report[2][3] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[2][3]));
	MTWF_PRINT("lq_report[2][4] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[2][4]));
	MTWF_PRINT("lq_report[3][0] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[3][0]));
	MTWF_PRINT("lq_report[3][1] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[3][1]));
	MTWF_PRINT("lq_report[3][2] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[3][2]));
	MTWF_PRINT("lq_report[3][3] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[3][3]));
	MTWF_PRINT("lq_report[3][4] = 0x%x\n",
			  cpu2le32(pEntry->rEntry.au4LqReport[3][4]));
}

INT32
hqa_wifi_test_mu_get_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_LQ_REPORT pOutput
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_GET_CALC_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	if (pOutput == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pOutput is NULL\n");
		Ret = -1;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"pOutput = %p\n", pOutput);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_GET_MU_CALC_LQ));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_snr_offset_set(
	PRTMP_ADAPTER pAd,
	UINT8 val
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_SNR_OFFSET;
	CMD_HQA_SET_MU_SNR_OFFSET param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	param.u1SnrOffset = (UINT8)val;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"offset: 0x%x\n", val);
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_set_zero_nss(
	PRTMP_ADAPTER pAd,
	UINT8 val
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_NSS_ZERO param = {0};
	UINT32 cmd = MU_HQA_SET_ZERO_NSS;
	struct _CMD_ATTRIBUTE attr = {0};

	param.u1NssZero = val;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"zero_nss:%u\n",  val);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_speed_up_lq(
	PRTMP_ADAPTER pAd,
	UINT32 val
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_LQ_SPEED_UP param = {0};
	UINT32 cmd = MU_HQA_SET_SPEED_UP_LQ;
	struct _CMD_ATTRIBUTE attr = {0};

	param.u4SpeedUp = val;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"val:%d\n", val);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	param.u4SpeedUp = cpu2le32(param.u4SpeedUp);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_table_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_TABLE ptr
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = 0;
	P_CMD_HQA_SET_MU_METRIC_TABLE pMuParam = NULL;
	P_CMD_HQA_SET_SU_METRIC_TABLE pSuParam = NULL;
	UINT32 index = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	if (ptr == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ptr is NULL\n");
		Ret = -1;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"type = %u, length = %u\n",
		ptr->type, ptr->length);

	cmd = (ptr->type == SU) ? MU_HQA_SET_SU_TABLE : MU_HQA_SET_MU_TABLE;

	if (ptr->type == SU) {
		os_alloc_mem(pAd, (UCHAR **)&pSuParam, sizeof(*pSuParam));
		if (pSuParam == NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pSuParam memory allocation failed\n");
			Ret = -1;
			goto error;
		}
		NdisZeroMemory(pSuParam, sizeof(*pSuParam));
	} else if (ptr->type == MU) {
		os_alloc_mem(pAd, (UCHAR **)&pMuParam, sizeof(*pMuParam));
		if (pMuParam == NULL) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pMuParam memory allocation failed\n");
			Ret = -1;
			goto error;
		}
		NdisZeroMemory(pMuParam, sizeof(*pMuParam));
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error type!\n");
		Ret = -1;
		goto error;
	}

	if (ptr->type == SU) {
		if (ptr->length <= sizeof(pSuParam->au1MetricTable))
			NdisCopyMemory(pSuParam->au1MetricTable, ptr->prTable, ptr->length);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"length(%u) error!!should < su_metric_tbl(%zu)\n",
				ptr->length, sizeof(pSuParam->au1MetricTable));
			Ret = -1;
			goto error;
		}
	} else {
		if (ptr->length <= sizeof(pMuParam->au1MetricTable))
			NdisCopyMemory(pMuParam->au1MetricTable, ptr->prTable, ptr->length);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"length(%u) error!!should < mu_metric_tbl(%zu)\n",
				ptr->length, sizeof(pSuParam->au1MetricTable));
			Ret = -1;
			goto error;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s\n", ptr->type == SU ? "su_metric_table:":"mu_metric_table:");

	if (ptr->type == MU) {
		for (index = 0; index < ptr->length; index += 8) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				pMuParam->au1MetricTable[index + 0],
				pMuParam->au1MetricTable[index + 1],
				pMuParam->au1MetricTable[index + 2],
				pMuParam->au1MetricTable[index + 3],
				pMuParam->au1MetricTable[index + 4],
				pMuParam->au1MetricTable[index + 5],
				pMuParam->au1MetricTable[index + 6],
				pMuParam->au1MetricTable[index + 7]);
		}
	} else {
		for (index = 0; index < ptr->length; index += 3) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"0x%x 0x%x 0x%x\n",
				pSuParam->au1MetricTable[index + 0],
				pSuParam->au1MetricTable[index + 1],
				pSuParam->au1MetricTable[index + 2]);
		}
	}

	msg = AndesAllocCmdMsg(pAd, (sizeof(cmd) + ((ptr->type == SU) ?
			(sizeof(*pSuParam)) : (sizeof(*pMuParam)))));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (ptr->type == SU) ? (char *)pSuParam : (char *)pMuParam,
		(ptr->type == SU) ? (sizeof(*pSuParam)) : (sizeof(*pMuParam)));
	AndesSendCmdMsg(pAd, msg);
error:
	if (pSuParam)
		os_free_mem(pSuParam);
	if (pMuParam)
		os_free_mem(pMuParam);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_group_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_GROUP mu_group
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MU_HQA_SET_GROUP;
	CMD_HQA_SET_MU_GROUP param = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (mu_group == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"mu_group params is NULL!!\n");
		Ret = -1;
		goto error;
	}

	param.u2GroupIndex = (UINT16)(cpu2le16(mu_group->groupIndex));
	param.u1NumOfUser = mu_group->numOfUser;
	param.fgUser0Ldpc = mu_group->user0Ldpc;
	param.fgUser1Ldpc = mu_group->user1Ldpc;
	param.fgUser2Ldpc = mu_group->user2Ldpc;
	param.fgUser3Ldpc = mu_group->user3Ldpc;
	param.u1BW = mu_group->bw;
	param.fgIsSGI = mu_group->shortGI;
	param.u1User0Nss = mu_group->user0Nss;
	param.u1User1Nss = mu_group->user1Nss;
	param.u1User2Nss = mu_group->user2Nss;
	param.u1User3Nss = mu_group->user3Nss;
	param.u1GID = mu_group->groupId;
	param.u1User0InitMCS = mu_group->user0InitMCS;
	param.u1User1InitMCS = mu_group->user1InitMCS;
	param.u1User2InitMCS = mu_group->user2InitMCS;
	param.u1User3InitMCS = mu_group->user3InitMCS;
	param.u1User0MuPfId = mu_group->user0MuPfId;
	param.u1User1MuPfId = mu_group->user1MuPfId;
	param.u1User2MuPfId = mu_group->user2MuPfId;
	param.u1User3MuPfId = mu_group->user3MuPfId;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"MU_STRUCT_MU_GROUP Content\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Gindex:%u Num_User:%u BW:%u LDPC[0~3]:%u %u %u %u\n",
		param.u2GroupIndex, param.u1NumOfUser, param.u1BW, param.fgUser0Ldpc,
		param.fgUser1Ldpc, param.fgUser2Ldpc, param.fgUser3Ldpc);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Nss[0~3]:%u %u %u %u GID:%u SGI:%u\n",
		param.u1User0Nss, param.u1User1Nss, param.u1User2Nss, param.u1User3Nss,
		param.u1GID, param.fgIsSGI);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"InitMCS[0~3]:%u %u %u %u PFID[0~3]:%u %u %u %u\n",
		param.u1User0InitMCS, param.u1User1InitMCS, param.u1User2InitMCS,
		param.u1User3InitMCS, param.u1User0MuPfId, param.u1User1MuPfId,
		param.u1User2MuPfId, param.u1User3MuPfId);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

static VOID hqa_wifi_test_mu_get_qd_callback(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	P_EVENT_HQA_MU_QD pEntry = NULL;
	P_MU_STRUCT_MU_QD pOutput;

	if (rsp_payload == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rsp_payload is null!!\n");
		return;
	}

	pEntry = (P_EVENT_HQA_MU_QD)rsp_payload;

	if (msg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is null!!\n");
		return;
	}

	if (msg->attr.rsp.wb_buf_in_calbk == NULL) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Error !! buffer not specified by cmd\n");
		return;
	}

	pOutput = (P_MU_STRUCT_MU_QD)msg->attr.rsp.wb_buf_in_calbk;
	pOutput->qd_report[0] = cpu2le32(pEntry->rEntry.qd_report[0]);
	pOutput->qd_report[1] = cpu2le32(pEntry->rEntry.qd_report[1]);
	pOutput->qd_report[2] = cpu2le32(pEntry->rEntry.qd_report[2]);
	pOutput->qd_report[3] = cpu2le32(pEntry->rEntry.qd_report[3]);
	pOutput->qd_report[4] = cpu2le32(pEntry->rEntry.qd_report[4]);
	pOutput->qd_report[5] = cpu2le32(pEntry->rEntry.qd_report[5]);
	pOutput->qd_report[6] = cpu2le32(pEntry->rEntry.qd_report[6]);
	pOutput->qd_report[7] = cpu2le32(pEntry->rEntry.qd_report[7]);
	pOutput->qd_report[8] = cpu2le32(pEntry->rEntry.qd_report[8]);
	pOutput->qd_report[9] = cpu2le32(pEntry->rEntry.qd_report[9]);
	pOutput->qd_report[10] = cpu2le32(pEntry->rEntry.qd_report[10]);
	pOutput->qd_report[11] = cpu2le32(pEntry->rEntry.qd_report[11]);
	pOutput->qd_report[12] = cpu2le32(pEntry->rEntry.qd_report[12]);
	pOutput->qd_report[13] = cpu2le32(pEntry->rEntry.qd_report[13]);

	MTWF_PRINT("%s: EVENT_HQA_MU_QD\n", __func__);
	MTWF_PRINT("eventId:%u\n", pEntry->u4EventId);
	MTWF_PRINT("qd_report[0~6] = %u %u %u %u %u %u %u\n",
		pOutput->qd_report[0], pOutput->qd_report[1],
		pOutput->qd_report[2], pOutput->qd_report[3],
		pOutput->qd_report[4], pOutput->qd_report[5],
		pOutput->qd_report[6]);
	MTWF_PRINT("qd_report[7~13] = %u %u %u %u %u %u %u\n",
		pOutput->qd_report[7], pOutput->qd_report[8],
		pOutput->qd_report[9], pOutput->qd_report[10],
		pOutput->qd_report[11], pOutput->qd_report[12],
		pOutput->qd_report[13]);
}

INT32
hqa_wifi_test_mu_get_qd(
	PRTMP_ADAPTER pAd,
	INT8 subcarrierIndex,
	P_MU_STRUCT_MU_QD pOutput
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_GET_QD param = {0};
	UINT32 cmd = MU_HQA_GET_QD;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!pOutput) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pOutput is NULL\n");
		Ret = -1;
		goto error;
	}

	param.scIdx = subcarrierIndex;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"param.scIdx:%d\n", param.scIdx);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_HQA_MU_QD));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pOutput);
	SET_CMD_ATTR_RSP_HANDLER(attr, eventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_set_enable(
	PRTMP_ADAPTER pAd,
	BOOLEAN val
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_HQA_SET_MU_ENABLE param = {0};
	UINT32 cmd = MU_HQA_SET_ENABLE;
	struct _CMD_ATTRIBUTE attr = {0};

	param.fgIsEnable = val;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"param.fgIsEnable:%d\n", param.fgIsEnable);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));
	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

INT32
hqa_wifi_test_mu_trigger_mu_tx(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM pParam
)
{
	INT32 Ret = 0;
	struct cmd_msg *msg = NULL;
	CMD_SET_TRIGGER_MU_TX_FRAME param = {0};
	UINT32 cmd = MU_SET_TRIGGER_MU_TX;
	struct _CMD_ATTRIBUTE attr = {0};

	if (pParam == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Param is NULL!!\n");
		Ret = -1;
		goto error;
	}

	param.u1AcIndex = 1;
	param.fgIsRandomPattern = pParam->fgIsRandomPattern;
	param.u4NumOfSTAs = cpu2le32(pParam->u4NumOfSTAs + 1);
	param.u4Round = cpu2le32(pParam->u4MuPacketCount);
	param.au4PayloadLength[0] = cpu2le32(pParam->msduPayloadLength0);
	param.au4PayloadLength[1] = cpu2le32(pParam->msduPayloadLength1);
	param.au4PayloadLength[2] = cpu2le32(pParam->msduPayloadLength2);
	param.au4PayloadLength[3] = cpu2le32(pParam->msduPayloadLength3);
	param.au1WlanIndexArray[0] = 1;
	param.au1WlanIndexArray[1] = 2;
	param.au1WlanIndexArray[2] = 3;
	param.au1WlanIndexArray[3] = 4;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"MD_SET_TRIGGER_MU_TX_FRAME\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"RndmPattern:%u Payload_Len[0~3]:%u %u %u %u Round:%u NumSta:%u\n",
		param.fgIsRandomPattern, param.au4PayloadLength[0],
		param.au4PayloadLength[1], param.au4PayloadLength[2],
		param.au4PayloadLength[3], param.u4Round, param.u4NumOfSTAs);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AC:%u WlanIndex[0~3]:%u %u %u %u\n",
		param.u1AcIndex, param.au1WlanIndexArray[0],
		param.au1WlanIndexArray[1], param.au1WlanIndexArray[2],
		param.au1WlanIndexArray[3]);

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(param));

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"msg is NULL\n");
		Ret = -1;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MU_MIMO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, (Ret == 0) ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"Ret=%d\n", Ret);
	return Ret;
}

static VOID eventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *)rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u4EventId = %u, len = %u\n", u4EventId, len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

	switch (u4EventId) {
	case MU_EVENT_MU_ENABLE:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_MU_ENABLE\n");
		ShowMuEnableCallback(pData, len);
		break;

	case MU_EVENT_MUPROFILE_ENTRY:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_MUPROFILE_ENTRY\n");
		ShowMuProfileEntryCallback(pData, len);
		break;

	case MU_EVENT_GROUP_TBL_ENTRY:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_GROUP_TBL_ENTRY\n");
		ShowGroupTblEntryCallback(pData, len);
		break;

	case MU_EVENT_CALC_INIT_MCS:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_CALC_INIT_MCS\n");
		ShowGroupTblEntryCallback(pData, len);
		break;

	case MU_EVENT_GROUP_NSS_THRESHOLD:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_GROUP_NSS_THRESHOLD\n");
		ShowGroupNssThresholdCallback(pData, len);
		break;

	case MU_EVENT_TXREQ_MIN_TIME:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_TXREQ_MIN_TIME\n");
		ShowTxReqMinTimeCallback(pData, len);
		break;

	case MU_EVENT_GROUP_USER_THRESHOLD:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_GROUP_USER_THRESHOLD\n");
		ShowGroupUserThresholdCallback(pData, len);
		break;

	case MU_EVENT_CLUSTER_TBL_ENTRY:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_CLUSTER_TBL_ENTRY\n");
		ShowClusterTblEntryCallback(pData, len);
		break;

	case MU_EVENT_SU_NSS_CHECK:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_SU_NSS_CHECK\n");
		ShowSuNssCheckCallback(pData, len);
		break;

	case MU_EVENT_TXOP_DEFAULT:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_TXOP_DEFAULT\n");
		ShowTxopDefaultCallback(pData, len);
		break;

	case MU_EVENT_SU_LOSS_THRESHOLD:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_SU_LOSS_THRESHOLD\n");
		ShowSuLossThresholdCallback(pData, len);
		break;

	case MU_EVENT_MU_GAIN_THRESHOLD:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_MU_GAIN_THRESHOLD\n");
		ShowMuGainThresholdCallback(pData, len);
		break;

	case MU_EVENT_SECONDARY_AC_POLICY:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_SECONDARY_AC_POLICY\n");
		ShowSecondaryAcPolicyCallback(pData, len);
		break;

	case MU_EVENT_GROUP_TBL_DMCS_MASK:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_GROUP_TBL_DMCS_MASK\n");
		ShowGroupTblDmcsMaskCallback(pData, len);
		break;

	case MU_EVENT_MAX_GROUP_SEARCH_CNT:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_MAX_GROUP_SEARCH_CNT\n");
		ShowMaxGroupSearchCntCallback(pData, len);
		break;

	case MU_EVENT_MUPROFILE_TX_STS_CNT:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_MUPROFILE_TX_STS_CNT\n");
		ShowMuProfileTxStsCntCallback(pData, len);
		break;

	case MU_EVENT_STATUS:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_STATUS\n");
		ShowStatusOfCommand(pData, len);
		break;

	case MU_EVENT_HQA_STATUS:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_HQA_STATUS\n");
		ShowStatusOfHqaCommand(pData, len);
		break;

	case MU_EVENT_HQA_GET_INIT_MCS:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_HQA_GET_INIT_MCS\n");
		hqa_wifi_test_mu_get_init_mcs_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_QD:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_HQA_GET_QD\n");
		hqa_wifi_test_mu_get_qd_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_LQ:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_HQA_GET_LQ\n");
		hqa_wifi_test_mu_get_lq_callback(msg, pData, len);
		break;

	case MU_EVENT_HQA_GET_SU_LQ:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MU_EVENT_HQA_GET_SU_LQ\n");
		hqa_wifi_test_mu_get_su_lq_callback(msg, pData, len);
		break;

	default:
		break;
	}
}

INT32 hqa_mu_get_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT32 gid = 0;
	MU_STRUCT_MU_GROUP_INIT_MCS init_mcs;

	if (arg != NULL)
		gid = os_str_toul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	os_zero_mem(&init_mcs, sizeof(init_mcs));
	MTWF_PRINT("%s: gid:%u\n", __func__, gid);

	if (hqa_wifi_test_mu_get_init_mcs(pAd, gid, &init_mcs)) {
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: InitMCS0:%u InitMCS1:%u InitMCS2:%u InitMCS3:%u\n",
		__func__, init_mcs.user0InitMCS, init_mcs.user1InitMCS,
		init_mcs.user2InitMCS, init_mcs.user3InitMCS);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_cal_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_SET_CALC_INIT_MCS param = {0};
	PCHAR temp_ptr_use_to_check = NULL;
	UINT8 num_of_user = 0, bandwidth = 0;
	UINT8 nss_of_user0 = 0, nss_of_user1 = 0;
	UINT8 nss_of_user2 = 0, nss_of_user3 = 0;
	UINT8 pf_mu_id_of_user0 = 0, pf_mu_id_of_user1 = 0;
	UINT8 pf_mu_id_of_user2 = 0, pf_mu_id_of_user3 = 0;
	UINT8 num_of_txer = 0;
	UINT16 group_index = 0;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_user = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfUser is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		bandwidth = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Bandwidth is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user1 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user2 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user3 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser0 is NULL\n");
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user1 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user2 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user3 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_txer = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfTxer is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL)
		group_index = (UINT16)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupIndex is NULL\n");
		Ret = FALSE;
		goto error;
	}

	os_zero_mem(&param, sizeof(param));
	param.num_of_user = num_of_user;
	param.bandwidth = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.nss_of_user1 = nss_of_user1;
	param.nss_of_user2 = nss_of_user2;
	param.nss_of_user3 = nss_of_user3;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pf_mu_id_of_user3;
	param.num_of_txer = num_of_txer;
	param.group_index = group_index;
	MTWF_PRINT("%s: Num_User:%u, BW:%u, Nss[0~3]:%u %u %u %u\n",
		__func__, param.num_of_user, param.bandwidth, param.nss_of_user0,
		param.nss_of_user1, param.nss_of_user2, param.nss_of_user3);
	MTWF_PRINT("%s: PFID[0~3]:%u %u %u %u Num_Txer:%u GroupIndex:%u\n",
		__func__, param.pf_mu_id_of_user0, param.pf_mu_id_of_user1,
		param.pf_mu_id_of_user2, param.pf_mu_id_of_user3, param.num_of_txer,
		param.group_index);

	if (hqa_wifi_test_mu_cal_init_mcs(pAd, &param))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	PCHAR temp_ptr_use_to_check = NULL;
	UINT8 num_of_user = 0, bandwidth = 0, num_of_txer = 0;
	UINT8 nss_of_user0 = 0, nss_of_user1 = 0;
	UINT8 nss_of_user2 = 0, nss_of_user3 = 0;
	UINT8 pf_mu_id_of_user0 = 0, pf_mu_id_of_user1 = 0;
	UINT8 pf_mu_id_of_user2 = 0, pf_mu_id_of_user3 = 0;
	UINT16 group_index = 0;

	MU_STRUCT_SET_CALC_LQ param = {0};

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_user = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfUser is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		bandwidth = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Bandwidth is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user1 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user2 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user3 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user1 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user2 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user3 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PfmuIdOfUser3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_txer = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfTxer is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL)
		group_index = (UINT16)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupIndex is NULL\n");
		Ret = FALSE;
		goto error;
	}

	os_zero_mem(&param, sizeof(param));
	param.num_of_user = num_of_user;
	param.bandwidth = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.nss_of_user1 = nss_of_user1;
	param.nss_of_user2 = nss_of_user2;
	param.nss_of_user3 = nss_of_user3;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.pf_mu_id_of_user1 = pf_mu_id_of_user1;
	param.pf_mu_id_of_user2 = pf_mu_id_of_user2;
	param.pf_mu_id_of_user3 = pf_mu_id_of_user3;
	param.num_of_txer = num_of_txer;
	param.group_index = group_index;
	MTWF_PRINT("%s: Num_User:%u, BW:%u, Nss[0~3]:%u %u %u %u\n",
		__func__, param.num_of_user, param.bandwidth, param.nss_of_user0,
		param.nss_of_user1, param.nss_of_user2, param.nss_of_user3);
	MTWF_PRINT("%s: PFID[0~3]:%u %u %u %u Num_Txer:%u GroupIndex:%u\n",
		__func__, param.pf_mu_id_of_user0, param.pf_mu_id_of_user1,
		param.pf_mu_id_of_user2, param.pf_mu_id_of_user3, param.num_of_txer,
		param.group_index);

	if (hqa_wifi_test_mu_cal_lq(pAd, (P_MU_STRUCT_SET_CALC_LQ)&param))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_LQ_REPORT lq;

	os_zero_mem(&lq, sizeof(lq));
	if (hqa_wifi_test_mu_get_lq(pAd, (P_MU_STRUCT_LQ_REPORT)&lq)) {
		Ret = FALSE;
		goto error;
	}

error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_su_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	PCHAR temp_ptr_use_to_check = NULL;
	UINT8 num_of_user = 0, bandwidth = 0;
	UINT8 nss_of_user0 = 0, num_of_txer = 0;
	UINT8 pf_mu_id_of_user0 = 0;
	UINT16 group_index = 0;
	CMD_HQA_SET_SU_CALC_LQ param = {0};

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_user = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfUser is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		bandwidth = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Bandwidth is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		nss_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NssOfUser0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		pf_mu_id_of_user0 = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PFMUIDOfUser0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		num_of_txer = (UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfTxer is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL)
		group_index = (UINT16)os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupIndex is NULL\n");
		Ret = FALSE;
		goto error;
	}

	os_zero_mem(&param, sizeof(param));
	param.num_of_user = num_of_user;
	param.bandwidth = bandwidth;
	param.nss_of_user0 = nss_of_user0;
	param.pf_mu_id_of_user0 = pf_mu_id_of_user0;
	param.num_of_txer = num_of_txer;
	param.group_index = group_index;

	MTWF_PRINT("%s: NumUser:%u, BW:%u, Nss0:%u PFMUID:%u NTxer:%u GroupIndex:%u\n",
		__func__, param.num_of_user, param.bandwidth, param.nss_of_user0,
		param.pf_mu_id_of_user0, param.num_of_txer, param.group_index);

	if (hqa_wifi_test_su_cal_lq(pAd, (P_MU_STRUCT_SET_SU_CALC_LQ)&param))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_su_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	SU_STRUCT_LQ_REPORT lq;

	os_zero_mem(&lq, sizeof(lq));
	MTWF_PRINT("%s \n", __func__);
	if (hqa_wifi_test_su_get_lq(pAd, (P_SU_STRUCT_LQ_REPORT)&lq)) {
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: SU_STRUCT_LQ_REPORT\n", __func__);
	MTWF_PRINT("%s lq_report[0~4] = 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
		cpu2le32(lq.au4LqReport[0]), cpu2le32(lq.au4LqReport[1]),
		cpu2le32(lq.au4LqReport[2]), cpu2le32(lq.au4LqReport[3]),
		cpu2le32(lq.au4LqReport[4]));
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_snr_offset(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT8 offset = 0;

	if (arg != NULL)
		offset = (UINT8)os_str_tol(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: SNRoffset:0x%x\n", __func__, offset);
	if (hqa_wifi_test_snr_offset_set(pAd, offset))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_zero_nss(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT8 zero_nss = 0;

	if (arg != NULL)
		zero_nss = (UINT8)os_str_toul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: zero_nss:0x%x\n",  __func__, zero_nss);
	if (hqa_wifi_test_mu_set_zero_nss(pAd, zero_nss))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
			 "CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_speedup_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	UINT32 spdup_lq = 0;

	if (arg != NULL)
		spdup_lq = (UINT32)os_str_tol(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: spdup_lq:0x%x\n", __func__, spdup_lq);
	if (hqa_wifi_test_mu_speed_up_lq(pAd, spdup_lq))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_mu_table(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_MU_TABLE info;
	PCHAR temp_ptr_use_to_check = NULL;
	UINT32 type = 0, length = 0, index = 0;
	UINT8 specific_metric_content = 0;
	struct _CMD_HQA_SET_MU_METRIC_TABLE mu_metric_table;
	struct _CMD_HQA_SET_SU_METRIC_TABLE su_metric_table;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		type = os_str_toul(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Type (SU/MU) is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL) {
		specific_metric_content =
			(UINT8)os_str_toul(temp_ptr_use_to_check, 0, 10);
		specific_metric_content = (UINT8)cpu2le32(specific_metric_content);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"specific_metric_content is NULL\n");
		Ret = FALSE;
		goto error;
	}

	if (type == MU) {
		length = sizeof(mu_metric_table);
		NdisZeroMemory(&mu_metric_table, length);
		NdisFillMemory(&mu_metric_table, length, specific_metric_content);
	} else if (type == SU) {
		length = sizeof(su_metric_table);
		NdisZeroMemory(&su_metric_table, length);
		NdisFillMemory(&su_metric_table, length, specific_metric_content);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Type error(%u)!!! neither MU nor SU\n", type);
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: type:%u, length:%u, data = %zu\n",
		__func__, type, length, type == MU ?
		sizeof(mu_metric_table) : sizeof(su_metric_table));
	MTWF_PRINT("%s\n", type == MU ? "mu_metric_table:":"su_metric_table:");

	if (type == MU) {
		for (index = 0; index < length; index += 8) {
			MTWF_PRINT("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				mu_metric_table.au1MetricTable[index + 0],
				mu_metric_table.au1MetricTable[index + 1],
				mu_metric_table.au1MetricTable[index + 2],
				mu_metric_table.au1MetricTable[index + 3],
				mu_metric_table.au1MetricTable[index + 4],
				mu_metric_table.au1MetricTable[index + 5],
				mu_metric_table.au1MetricTable[index + 6],
				mu_metric_table.au1MetricTable[index + 7]);
		}
	} else {
		for (index = 0; index < length; index += 3) {
			MTWF_PRINT("0x%x 0x%x 0x%x\n",
				su_metric_table.au1MetricTable[index + 0],
				su_metric_table.au1MetricTable[index + 1],
				su_metric_table.au1MetricTable[index + 2]);
		}
	}

	info.type = type;
	info.length = length;
	info.prTable = (type == MU) ?
		(char *)&mu_metric_table : (char *)&su_metric_table;
	if (hqa_wifi_test_mu_table_set(pAd, (P_MU_STRUCT_MU_TABLE)&info))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_group(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_MU_GROUP mu_group;
	PCHAR temp_ptr_use_to_check = NULL;
	UINT16 groupIndex;
	UINT8 numOfUser, bw;
	BOOLEAN user0Ldpc, user1Ldpc, user2Ldpc, user3Ldpc;
	UINT8 user0Nss, user1Nss, user2Nss, user3Nss;
	UINT8 groupId;
	UINT8 user0InitMCS, user1InitMCS, user2InitMCS, user3InitMCS;
	UINT8 user0MuPfId, user1MuPfId, user2MuPfId, user3MuPfId;
	BOOLEAN shortGI;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		groupIndex = (UINT16)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupIndex is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if	(temp_ptr_use_to_check != NULL) {
		numOfUser = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfUser is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if	(temp_ptr_use_to_check != NULL) {
		bw = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Bandwidth is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user0Ldpc = (BOOLEAN)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User0Ldpc is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user1Ldpc = (BOOLEAN)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User1Ldpc is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user2Ldpc = (BOOLEAN)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"User2Ldpc is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user3Ldpc = (BOOLEAN)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User3Ldpc is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user0Nss = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User0Nss is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user1Nss = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User1Nss is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user2Nss = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User2Nss is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user3Nss = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User3Nss is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		groupId = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupId is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user0InitMCS = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User0InitMCS is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user1InitMCS = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User1InitMCS is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user2InitMCS = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User2InitMCS is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user3InitMCS = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User3InitMCS is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user0MuPfId = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User0MuPfId is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user1MuPfId = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User1MuPfId is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user2MuPfId = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User2MuPfId is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL) {
		user3MuPfId = (UINT8)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"User3MuPfId is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL) {
		shortGI = (BOOLEAN)simple_strtol(temp_ptr_use_to_check, 0, 10);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"SGI is NULL\n");
		Ret = FALSE;
		goto error;
	}

	os_zero_mem(&mu_group, sizeof(mu_group));
	mu_group.groupIndex = groupIndex;
	mu_group.numOfUser = numOfUser;
	mu_group.bw = bw;
	mu_group.user0Ldpc = user0Ldpc;
	mu_group.user1Ldpc = user1Ldpc;
	mu_group.user2Ldpc = user2Ldpc;
	mu_group.user3Ldpc = user3Ldpc;
	mu_group.user0Nss = user0Nss;
	mu_group.user1Nss = user1Nss;
	mu_group.user2Nss = user2Nss;
	mu_group.user3Nss = user3Nss;
	mu_group.groupId = groupId;
	mu_group.user0InitMCS = user0InitMCS;
	mu_group.user1InitMCS = user1InitMCS;
	mu_group.user2InitMCS = user2InitMCS;
	mu_group.user3InitMCS = user3InitMCS;
	mu_group.user0MuPfId = user0MuPfId;
	mu_group.user1MuPfId = user1MuPfId;
	mu_group.user2MuPfId = user2MuPfId;
	mu_group.user3MuPfId = user3MuPfId;
	mu_group.shortGI = shortGI;

	MTWF_PRINT("Gindex:%u Num_User:%u BW:%u LDPC[0~3]:%u %u %u %u\n",
		mu_group.groupIndex, mu_group.numOfUser,
		mu_group.bw, mu_group.user0Ldpc, mu_group.user1Ldpc,
		mu_group.user2Ldpc, mu_group.user3Ldpc);
	MTWF_PRINT("Nss[0~3]:%u %u %u %u GID:%u SGI:%u\n",
		mu_group.user0Nss, mu_group.user1Nss,
		mu_group.user2Nss, mu_group.user3Nss,
		mu_group.groupId, mu_group.shortGI);
	MTWF_PRINT("InitMCS[0~3]:%u %u %u %u PFID[0~3]:%u %u %u %u\n",
		mu_group.user0InitMCS, mu_group.user1InitMCS,
		mu_group.user2InitMCS, mu_group.user3InitMCS,
		mu_group.user0MuPfId, mu_group.user1MuPfId,
		mu_group.user2MuPfId, mu_group.user3MuPfId);

	if (hqa_wifi_test_mu_group_set(pAd, (P_MU_STRUCT_MU_GROUP)&mu_group))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_get_qd(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT32 subcarrier_idx = 0;
	MU_STRUCT_MU_QD qd;

	if (arg != NULL)
		subcarrier_idx = os_str_toul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: subcarrier_idx = %d\n", __func__, subcarrier_idx);

	RTMPZeroMemory(&qd, sizeof(MU_STRUCT_MU_QD));

	if (hqa_wifi_test_mu_get_qd(pAd, subcarrier_idx,
				(P_MU_STRUCT_MU_QD)&qd)) {
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: MU_STRUCT_MU_QD\n", __func__);
	MTWF_PRINT("qd_report[0~6] = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		cpu2le32(qd.qd_report[0]), cpu2le32(qd.qd_report[1]),
		cpu2le32(qd.qd_report[2]), cpu2le32(qd.qd_report[3]),
		cpu2le32(qd.qd_report[4]), cpu2le32(qd.qd_report[5]),
		cpu2le32(qd.qd_report[6]));
	MTWF_PRINT("qd_report[7~13] = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		cpu2le32(qd.qd_report[7]), cpu2le32(qd.qd_report[8]),
		cpu2le32(qd.qd_report[9]), cpu2le32(qd.qd_report[10]),
		cpu2le32(qd.qd_report[11]), cpu2le32(qd.qd_report[12]),
		cpu2le32(qd.qd_report[13]));
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	BOOLEAN IsMuEnable = 0;

	if (arg != NULL)
		IsMuEnable = (BOOLEAN)os_str_toul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	MTWF_PRINT("%s: MU %s %u\n", __func__, IsMuEnable == 1 ?
		 "Enable":"Disable", IsMuEnable);
	if (hqa_wifi_test_mu_set_enable(pAd, IsMuEnable))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_gid_up(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_MU_STA_PARAM param;
	PCHAR temp_ptr_use_to_check = NULL;
	UINT32 gid_0 = 0, gid_1 = 0;
	UINT32 up_0 = 0, up_1 = 0, up_2 = 0, up_3 = 0;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		gid_0 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupID0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		gid_1 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"GroupID1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		up_0 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UserPosition0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		up_1 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UserPosition1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		up_2 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UserPosition2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL)
		up_3 = os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"UserPosition3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	param.gid[0] = gid_0;
	param.gid[1] = gid_1;
	param.up[0] = up_0;
	param.up[1] = up_1;
	param.up[2] = up_2;
	param.up[3] = up_3;
	MTWF_PRINT("%s: GID[0~1]=0x%x 0x%x UP[0~3]=0x%x 0x%x 0x%x 0x%x\n",
		__func__, param.gid[0], param.gid[1], param.up[0],
		param.up[1], param.up[2], param.up[3]);

	if (hqa_wifi_test_mu_set_sta_gid_and_up(pAd, &param))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_set_trigger_mu_tx(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM param = {0};
	PCHAR temp_ptr_use_to_check     = NULL;
	BOOLEAN fgIsRandomPattern       = 0;
	UINT32 u4PayloadLen0 = 0, u4PayloadLen1 = 0;
	UINT32 u4PayloadLen2 = 0, u4PayloadLen3 = 0;
	UINT32 u4MuPacketCount = 0, u4NumOfSTAs = 0;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		fgIsRandomPattern =
			(BOOLEAN)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"RandomPattern is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		u4PayloadLen0 = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MsduPayloadLength0 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		u4PayloadLen1 = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MsduPayloadLength1 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		u4PayloadLen2 = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MsduPayloadLength2 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		u4PayloadLen3 = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MsduPayloadLength3 is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = strsep(&arg, ":");
	if (temp_ptr_use_to_check != NULL)
		u4MuPacketCount = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"MuPacketCount is NULL\n");
		Ret = FALSE;
		goto error;
	}

	temp_ptr_use_to_check = NULL;
	temp_ptr_use_to_check = arg;
	if (temp_ptr_use_to_check != NULL)
		u4NumOfSTAs = (UINT32)os_str_tol(temp_ptr_use_to_check, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"NumOfSTA is NULL\n");
		Ret = FALSE;
		goto error;
	}

	param.fgIsRandomPattern = fgIsRandomPattern;
	param.msduPayloadLength0 = u4PayloadLen0;
	param.msduPayloadLength1 = u4PayloadLen1;
	param.msduPayloadLength2 = u4PayloadLen2;
	param.msduPayloadLength3 = u4PayloadLen3;
	param.u4MuPacketCount = u4MuPacketCount;
	param.u4NumOfSTAs = u4NumOfSTAs;
	MTWF_PRINT("%s: RndPtrn:%u PayloadLen[0~3]:%u %u %u %u PktCnt:%u NumSta:%u\n",
		__func__, param.fgIsRandomPattern, param.msduPayloadLength0,
		param.msduPayloadLength1, param.msduPayloadLength2,
		param.msduPayloadLength3, param.u4MuPacketCount, param.u4NumOfSTAs);

	if (hqa_wifi_test_mu_trigger_mu_tx(pAd, &param))
		Ret = FALSE;
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, Ret ? DBG_LVL_DEBUG : DBG_LVL_ERROR,
		"CMD %s\n", Ret ? "Success":"Fail");
	return Ret;
}

INT32 hqa_mu_reset_murx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 u4Reset = 0;

	if (arg != NULL)
		u4Reset = simple_strtoul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		return FALSE;
	}

	MTWF_PRINT("%s: Reset:%u\n", __func__, u4Reset);

	if (u4Reset) {
		pAd->u4RxMuPktCount = 0;
		MTWF_PRINT("%s: Reset MURX pkt cnt success!\n", __func__);
	}

	return TRUE;
}

INT32 hqa_mu_reset_mutx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 u4Reset = 0;

	if (arg != NULL)
		u4Reset = simple_strtoul(arg, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Argument is NULL\n");
		return FALSE;
	}

	MTWF_PRINT("%s: Reset:%u\n", __func__, u4Reset);

	if (u4Reset) {
		pAd->u4TxMuPktCount = 0;
		MTWF_PRINT("%s: Reset MUTX pkt cnt success!\n", __func__);

	}

	return TRUE;
}

#endif /* CFG_SUPPORT_MU_MIMO */
