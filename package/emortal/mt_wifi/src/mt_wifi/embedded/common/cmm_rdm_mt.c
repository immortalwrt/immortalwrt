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
 ***************************************************************************

    Module Name:
    cmm_rdm_mt.c//Jelly20140123
*/

#ifdef MT_DFS_SUPPORT
/* Remember add RDM compiler flag - Shihwei20141104 */
#include "rt_config.h"
#include "hdev/hdev.h"
#include "wlan_config/config_internal.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/


EXT_EVENT_RDD_REPORT_T g_radar_info[HW_RDD_NUM];

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

typedef int (*_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type) (UCHAR SyncNum, UCHAR monitored_Ch, UCHAR Bw);
typedef int (*_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type)(UCHAR SyncNum, UCHAR Bw, UCHAR monitored_Ch);
typedef int (*_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type) (UCHAR Bw80ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList, UCHAR Bw40ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList, UCHAR Bw20ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type radar_detected_callback_func;
_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type DfsCacTimeOutCallBack;
_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type DfsNopTimeOutCallBack;

void k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type callback_detect_collision_func)
{
    radar_detected_callback_func = callback_detect_collision_func;
}

void k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type callback_CAC_time_meet_func)
{
    DfsCacTimeOutCallBack = callback_CAC_time_meet_func;
}

void k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type callback_NOP_Timeout_func)
{
    DfsNopTimeOutCallBack = callback_NOP_Timeout_func;
}

EXPORT_SYMBOL(k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration);

static VOID ZeroWaitDfsEnable(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR bZeroWaitDfsCtrl;

	bZeroWaitDfsCtrl = msg->zerowait_dfs_ctrl_msg.Enable;

#ifdef BACKGROUND_SCAN_SUPPORT
#if (RDD_2_SUPPORTED == 0)
	DfsDedicatedDynamicCtrl(pAd, bZeroWaitDfsCtrl);
#endif /* RDD_2_SUPPORTED */
#endif /* BACKGROUND_SCAN_SUPPORT */
}

static VOID ZeroWaitDfsInitAvalChListUpdate(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];

	Bw80TotalChNum = msg->aval_channel_list_msg.Bw80TotalChNum;
	Bw40TotalChNum = msg->aval_channel_list_msg.Bw40TotalChNum;
	Bw20TotalChNum = msg->aval_channel_list_msg.Bw20TotalChNum;

	memcpy(Bw80AvalChList,
		msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw40AvalChList,
		msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw20AvalChList,
		msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20ChNum: %d\n", Bw20TotalChNum));
#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw20TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20 ChList[%d] Channel:%d\n",
			i, Bw20AvalChList[i].Channel));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40ChNum: %d\n", Bw40TotalChNum));

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw40TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40 ChList[%d] Channel:%d\n",
			i, Bw40AvalChList[i].Channel));
	}
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80ChNum: %d\n", Bw80TotalChNum));

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw80TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80 ChList[%d] Channel:%d\n",
			i, Bw80AvalChList[i].Channel));
	}
#endif
	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80TotalChNum, Bw80AvalChList,
	Bw40TotalChNum, Bw40AvalChList,
	Bw20TotalChNum, Bw20AvalChList);
}

static VOID ZeroWaitDfsMonitorChUpdate(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR SynNum;
	UCHAR Channel;
	UCHAR Bw;
	BOOLEAN doCAC;

	SynNum = msg->set_monitored_ch_msg.SyncNum;
	Channel = msg->set_monitored_ch_msg.Channel;
	Bw = msg->set_monitored_ch_msg.Bw;
	doCAC = msg->set_monitored_ch_msg.doCAC;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
	__func__, SynNum, Channel, Bw));

#ifdef BACKGROUND_SCAN_SUPPORT

	switch (SynNum) {
	case RDD_BAND0:
#if (RDD_2_SUPPORTED == 1)
	case RDD_BAND1:
#endif /* RDD_2_SUPPORTED */
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC, SynNum);
		break;

	case RDD_DEDICATED_RX:
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw, SynNum);
		break;

	default:
		break;
	}

#endif

}

static VOID ZeroWaitDfsSetNopToChList(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	Channel = msg->nop_force_set_msg.Channel;
	Bw = msg->nop_force_set_msg.Bw;
	NOPTime = msg->nop_force_set_msg.NOPTime;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Channel: %d, Bw: %d, NOP: %d \x1b[m \n",
	__FUNCTION__, Channel, Bw, NOPTime));

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

}

static VOID ZeroWaitDfsPreAssignNextTarget(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel;
	UCHAR Bw;
	USHORT CacValue;

	Channel = msg->assign_next_target.Channel;
	Bw = msg->assign_next_target.Bw;
	CacValue = msg->assign_next_target.CacValue;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Channel: %d, Bw: %d \x1b[m \n",
	__FUNCTION__, Channel, Bw));

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);
}

static VOID ZeroWaitShowTargetInfo(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR mode;

	mode = msg->target_ch_show.mode;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] mode: %d \x1b[m \n",
	__FUNCTION__, mode));

	ZeroWait_DFS_Next_Target_Show(pAd, mode);
}

static VOID ZeroWaitDfsMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case ZERO_WAIT_DFS_ENABLE:
		ZeroWaitDfsEnable(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case INIT_AVAL_CH_LIST_UPDATE:
		ZeroWaitDfsInitAvalChListUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case MONITOR_CH_ASSIGN:
		ZeroWaitDfsMonitorChUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case NOP_FORCE_SET:
		ZeroWaitDfsSetNopToChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case PRE_ASSIGN_NEXT_TARGET:
		ZeroWaitDfsPreAssignNextTarget(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case SHOW_TARGET_INFO:
		ZeroWaitShowTargetInfo(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;

	if (!wrq)
		return NDIS_STATUS_FAILURE;

	if (copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length)) {
		status = -EFAULT;
	} else {
		ZeroWaitDfsMsgHandle(pAd, (CHAR *)&msg);
	}

	return status;
}

static VOID ZeroWaitDfsQueryNopOfChList(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UCHAR ch_idx = 0, band_idx;
	DfsProvideNopOfChList(pAd, msg);

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s][RDM]: band index - %d\n", __func__, band_idx));

		for (ch_idx = 0; ch_idx < msg->nop_of_channel_list_msg.NOPTotalChNum[band_idx]; ch_idx++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
			ch_idx,
			msg->nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].Channel,
			msg->nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].Bw,
			msg->nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].NonOccupancy));
		}
	}
}


VOID ZeroWaitDfsQueryAvalChListNonDbdc(PRTMP_ADAPTER pAd, UCHAR *Bw80ChNum, UCHAR *Bw40ChNum, UCHAR *Bw20ChNum,
			DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM],
			DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM],
			DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM])
{
	UCHAR band_idx = 0;
	UINT_8 bw_idx, ch_idx, idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	if (pAd->Dot11_H[band_idx].RDMode == RD_SWITCHING_MODE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Channel list query fail during channel switch\n"));
		return;
	}

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[bw_idx][ch_idx] = 0xff;
		}
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, FALSE, band_idx);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_20][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_20][ch_idx];
			Bw20AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw20AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw20ChNum = ch_idx;

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_40][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_40][ch_idx];
			Bw40AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw40AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw40ChNum = ch_idx;

	for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
		if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_80][ch_idx] != 0xff) {
			idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_80][ch_idx];
			Bw80AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
			Bw80AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
		} else
			break;
	}
	*Bw80ChNum = ch_idx;
}
static VOID ZeroWaitDfsQueryAvalChList(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UINT_8 bw_idx, ch_idx, idx;
	UCHAR band_idx;

	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	os_zero_mem(&Bw80AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	os_zero_mem(&Bw40AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
	os_zero_mem(&Bw20AvailableChList, sizeof(DFS_REPORT_AVALABLE_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);

	if (pAd->CommonCfg.dbdc_mode == 0) {
		ZeroWaitDfsQueryAvalChListNonDbdc(pAd, &Bw80TotalChNum, &Bw40TotalChNum, &Bw20TotalChNum, Bw80AvailableChList,
									Bw40AvailableChList, Bw20AvailableChList);
	} else {
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->Dot11_H[band_idx].RDMode == RD_SWITCHING_MODE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Channel list query fail during channel switch\n"));
			return;
		}

		for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
			for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++)
				pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[bw_idx][ch_idx] = 0xff;
		}

		DfsBwChQueryAllList(pAd, BW_80, pDfsParam, FALSE, band_idx);
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_20][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_20][ch_idx];
				Bw20AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw20AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw20TotalChNum = ch_idx;

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_40][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_40][ch_idx];
				Bw40AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw40AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw40TotalChNum = ch_idx;

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_80][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[BW_80][ch_idx];
				Bw80AvailableChList[ch_idx].Channel = pChCtrl->ChList[idx].Channel;
				Bw80AvailableChList[ch_idx].RadarHitCnt = pChCtrl->ChList[idx].NOPClrCnt;
			} else
				break;
		}
		Bw80TotalChNum = ch_idx;
	}
	}
	msg->aval_channel_list_msg.Bw80TotalChNum = Bw80TotalChNum;
	msg->aval_channel_list_msg.Bw40TotalChNum = Bw40TotalChNum;
	msg->aval_channel_list_msg.Bw20TotalChNum = Bw20TotalChNum;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Bw20ChNum: %d\n", msg->aval_channel_list_msg.Bw20TotalChNum));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Bw40ChNum: %d\n", msg->aval_channel_list_msg.Bw40TotalChNum));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Bw80ChNum: %d\n", msg->aval_channel_list_msg.Bw80TotalChNum));

	memcpy(msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80AvailableChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40AvailableChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20AvailableChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw80TotalChNum; ch_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw80AvailableChList[ch_idx].Channel,
		Bw80AvailableChList[ch_idx].RadarHitCnt));
	}
	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw40TotalChNum; ch_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw40AvailableChList[ch_idx].Channel,
		Bw40AvailableChList[ch_idx].RadarHitCnt));
	}
	for (ch_idx = 0; ch_idx < msg->aval_channel_list_msg.Bw20TotalChNum; ch_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		ch_idx,
		Bw20AvailableChList[ch_idx].Channel,
		Bw20AvailableChList[ch_idx].RadarHitCnt));
	}
}

static VOID ZeroWaitDfsQueryMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case QUERY_AVAL_CH_LIST:
		ZeroWaitDfsQueryAvalChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	case QUERY_NOP_OF_CH_LIST:
		ZeroWaitDfsQueryNopOfChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

BOOLEAN DfsCheckHitBandBWDbdcMode(PRTMP_ADAPTER pAd, UCHAR bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if ((pDfsParam->DFSChHitBand != DFS_BAND_NONE) && (bw == BW_80) && !(pAd->CommonCfg.dbdc_mode))
		return true;
	else
		return false;
}


INT ZeroWaitDfsQueryCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));
#ifdef MAP_R2
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		msg.aval_channel_list_msg.Action = QUERY_AVAL_CH_LIST;
#endif
	ZeroWaitDfsQueryMsgHandle(pAd, (CHAR *)&msg);
	wrq->u.data.length = sizeof(union dfs_zero_wait_msg);

	if (copy_to_user(wrq->u.data.pointer, &msg, wrq->u.data.length)) {
		status = -EFAULT;
	}

	return status;
}

PCHANNEL_CTRL DfsGetChCtrl(
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR bw,
	IN UCHAR band_idx)
{
	PCHANNEL_CTRL pChCtrl = NULL;

	if ((pAd->CommonCfg.dbdc_mode) ||
		((bw == BW_80) && pDfsParam->DFSChHitBand != DFS_BAND_NONE)) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
	} else
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BAND0);

	return pChCtrl;
}

UCHAR DfsGetNonDfsDefaultCh(
	IN PDFS_PARAM pDfsParam,
	IN UCHAR band_idx)
{
	UCHAR channel;

	if (band_idx == DBDC_BAND0) {
		if (pDfsParam->band_ch[DBDC_BAND1] != 149)
			channel = 149;
		else
			channel = 36;
	} else if (band_idx == DBDC_BAND1) {
		if (pDfsParam->band_ch[DBDC_BAND0] != 36)
			channel = 36;
		else
			channel = 149;
	}

	return channel;
}

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
INT zero_wait_dfs_update_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PUCHAR ch
)
{
	PUCHAR ch_outband = &pAd->CommonCfg.DfsParameter.OutBandCh;
	PUCHAR phy_bw_outband = &pAd->CommonCfg.DfsParameter.OutBandBw;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UINT8 band_idx = HcGetBandByWdev(wdev);
#ifdef MAP_R2
	int i = 0;
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	UINT8 tempCh;

	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if (*ch_stat == DFS_INB_CH_INIT)
		pDfsParam->DFSChHitBand = DFS_BAND_NONE;
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): outband ch %d, ch_stat %d\n", __func__, *ch_outband, *ch_stat));

	if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): bDedicatedZeroWaitDefault != 0\n", __func__));
		return FALSE;
	}

	if (!WMODE_CAP_5G(wdev->PhyMode))
		return FALSE;
#ifdef MAP_R2
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		for (i = 0; i < MAX_BEACON_NUM; i++) {
			if ((pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
				&& (pAd->ApCfg.MBSSID[i].wdev.cac_not_required == TRUE))
					return FALSE;
		}
	}
#endif

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if (RadarChannelCheck(pAd, pDfsParam->band_ch[DBDC_BAND0])) {
		pDfsParam->DFSChHitBand = DBDC_BAND0;
		if ((pAd->CommonCfg.dbdc_mode) && (band_idx == DBDC_BAND1)) {
			pDfsParam->DFSChHitBand = DBDC_BAND1;
		}
	}
	else if (RadarChannelCheck(pAd, pDfsParam->band_ch[DBDC_BAND1])) {
		pDfsParam->DFSChHitBand = DBDC_BAND1;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s(): DFSChHitBand %d\n",
				__func__,
				pDfsParam->DFSChHitBand));
#endif

	switch (*ch_stat) {
	case DFS_INB_CH_INIT:
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (pDfsParam->DFSChHitBand != DFS_BAND_NONE)
#else
		if (RadarChannelCheck(pAd, *ch))
#endif
		{
			/* If DFS ch X is selected, CAC of DFS ch X will be checked by dedicated RX */
			/* Update new channel new channel as outband Channel */
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			if (pAd->CommonCfg.dbdc_mode)
				pAd->CommonCfg.DfsParameter.OutBandCh = *ch;
			else
				pAd->CommonCfg.DfsParameter.OutBandCh = pDfsParam->band_ch[pDfsParam->DFSChHitBand];
#else
			pAd->CommonCfg.DfsParameter.OutBandCh = *ch;
#endif
			dfs_get_outband_bw(pAd, wdev, phy_bw_outband);

			/* Stop RDD */
			mtRddControl(pAd, RDD_STOP, band_idx, 0, 0);

			/* update non-DFS ch Y as new ch */
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			if (pDfsParam->DFSChHitBand == DBDC_BAND0)
			{
				if (pDfsParam->band_ch[DBDC_BAND1] != 149)
					*ch = 149;
				else
					*ch = 36;
			}
			else if (pDfsParam->DFSChHitBand == DBDC_BAND1)
			{
				if (pDfsParam->band_ch[DBDC_BAND0] != 36)
					tempCh = 36;
				else
					tempCh = 149;

				if (pAd->CommonCfg.dbdc_mode) {
					if (pDfsParam->band_ch[DBDC_BAND0] != 149)
						*ch = 149;
					else
						*ch = 36;
				} else {
					pDfsParam->band_ch[DBDC_BAND1] = tempCh;
					wdev->vht_sec_80_channel = tempCh;
					wlan_config_set_cen_ch_2(wdev, DfsPrimToCent(tempCh, BW_80));
					wlan_operate_set_cen_ch_2(wdev, DfsPrimToCent(tempCh, BW_80));
				}
			}
#else
			*ch = FirstChannel(pAd, wdev);
#endif

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): DFS ch %d is selected, use non-DFS ch %d, ch_stat %d\n",
				__func__,
				pAd->CommonCfg.DfsParameter.OutBandCh,
				*ch,
				*ch_stat));
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): non-DFS ch %d, ch_stat %d\n", __func__, *ch, *ch_stat));

			/* 5th RX is set */
			if (*ch_outband != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): 5th RX is set to ch%d\n", __func__, *ch_outband));
				break;
			}

			pAd->CommonCfg.DfsParameter.OutBandCh = 0;

			return FALSE;
		}
		break;
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): Do not switch to DFS ch immediately\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): ch_stat %d\n", __func__, *ch_stat));
		return FALSE;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): ch_stat %d\n", __func__, *ch_stat));
		return FALSE;

	}

	return TRUE;
}

#ifdef BACKGROUND_SCAN_SUPPORT
INT zero_wait_dfs_switch_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR band_idx
)
{
	PUCHAR ch_outband = &pAd->CommonCfg.DfsParameter.OutBandCh;
	PUCHAR phy_bw_outband = &pAd->CommonCfg.DfsParameter.OutBandBw;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	PRALINK_TIMER_STRUCT set_ob_ch_timer = &pAd->BgndScanCtrl.DfsZeroWaitTimer;
	ULONG wait_time = 3000; /* Wait for 6,000 ms */
	(VOID)band_idx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): outband ch %d, ch_stat %d\n", __func__, *ch_outband, *ch_stat));

	if (!WMODE_CAP_5G(wdev->PhyMode))
		return FALSE;

	if (pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): bDedicatedZeroWaitDefault != 0\n", __func__));
		return FALSE;
	}

	switch (*ch_stat) {
	case DFS_INB_CH_INIT:
		*ch_stat = DFS_OUTB_CH_CAC;
	case DFS_INB_DFS_RADAR_OUTB_CAC_DONE:
		if (*ch_outband != 0) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): OutBandCh %d, OutBandBw %d\n", __func__, *ch_outband, *phy_bw_outband));

			if (*ch_stat == DFS_INB_DFS_RADAR_OUTB_CAC_DONE) {
				wait_time = 1000; /* Wait for 1,000 ms */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s(): Do not switch to DFS ch immediately\n", __func__));
			}


			/* Set out-band channel after calibration is done */
			RTMPSetTimer(set_ob_ch_timer, wait_time);
		}
		break;

	case DFS_OUTB_CH_CAC:
	case DFS_INB_CH_SWITCH_CH:
	case DFS_INB_DFS_OUTB_CH_CAC:
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): OUTBAND_SWITCH, ch_stat %d\n", __func__, *ch_stat));

		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): ch_stat %d\n", __func__, *ch_stat));
		return FALSE;

	}

	return TRUE;
}
#endif

#endif

#ifdef CONFIG_AP_SUPPORT
static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i])
			return TRUE;
	}

	return FALSE;
}
#endif

static inline UCHAR CentToPrim(
	UCHAR Channel)
{
	return Channel - 2;
}

static BOOLEAN DfsCheckChAvailableByBw(
	UCHAR Channel, UCHAR Bw, PCHANNEL_CTRL pChCtrl)
{
#define BW40_CHGRP_NUM  13
#define BW80_CHGRP_NUM  7
#define BW160_CHGRP_NUM 3

	UCHAR i = 0, j = 0, k = 0;
	UCHAR *pBwChGroup = NULL;
	UCHAR BW40_CH_GROUP[BW40_CHGRP_NUM][2] = {
	{36, 40}, {44, 48},
	{52, 56}, {60, 64},
	{100, 104}, {108, 112},
	{116, 120}, {124, 128},
	{132, 136}, {140, 144},
	{149, 153}, {157, 161}, {0, 0}
	};

	UCHAR BW80_CH_GROUP[BW80_CHGRP_NUM][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{0, 0, 0, 0}
	};

	UCHAR BW160_CH_GROUP[BW160_CHGRP_NUM][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128},
	{0, 0, 0, 0, 0, 0, 0, 0}
	};

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwChGroup = &BW40_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW40_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 2;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW40_CH_GROUP[i][0])
				break;
		}

		if (j == pChCtrl->ChListNum)
			return FALSE;
		else if (pChCtrl->ChList[j+1].Channel == BW40_CH_GROUP[i][1])
			return TRUE;
	} else if (Bw == BW_80 || Bw == BW_8080) {
		pBwChGroup = &BW80_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW80_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 4;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW80_CH_GROUP[i][0])
				break;
		}
		if (j == pChCtrl->ChListNum)
			return FALSE;
		else if ((pChCtrl->ChList[j+1].Channel == BW80_CH_GROUP[i][1])
			&& (pChCtrl->ChList[j+2].Channel == BW80_CH_GROUP[i][2])
			&& (pChCtrl->ChList[j+3].Channel == BW80_CH_GROUP[i][3])
		)
			return TRUE;

	} else if (Bw == BW_160) {
		pBwChGroup = &BW160_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i >= sizeof(BW160_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		i /= 8;
		for (j = 0; j < pChCtrl->ChListNum; j++) {
			if (pChCtrl->ChList[j].Channel == BW160_CH_GROUP[i][0])
				break;
		}
		if (j == pChCtrl->ChListNum)
			return FALSE;
		else {
			for (k = 1; k < 7 ; k++) {
				if (pChCtrl->ChList[j+k].Channel != BW160_CH_GROUP[i][k])
					return FALSE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

static BOOLEAN ByPassChannelByBw(
	UCHAR Channel, UCHAR Bw, PCHANNEL_CTRL pChCtrl)
{
	UINT_8 i;
	BOOLEAN BwSupport = FALSE;

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (Channel == pChCtrl->ChList[i].Channel) {
			if (Bw == BW_8080) {
				BwSupport = (pChCtrl->ChList[i].SupportBwBitMap) & BIT(BW_80);
			} else {
				BwSupport = (pChCtrl->ChList[i].SupportBwBitMap) & BIT(Bw);
			}
		}
	}

	if (BwSupport)
		return FALSE;
	else
		return TRUE;

}

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw)
{
	UINT_8 i = 0;

	UCHAR CH_EXT_ABOVE[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};

	UCHAR CH_EXT_BELOW[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};

	if (Bw == BW_20)
		return Channel;
	else if (Bw == BW_40) {
		while (CH_EXT_ABOVE[i] != 0) {
			if (Channel == CH_EXT_ABOVE[i]) {
				return Channel + 2;
			} else if (Channel == CH_EXT_BELOW[i]) {
				return Channel - 2;
			}
			i++;
		}
	} else if (Bw == BW_80 || Bw == BW_8080)
		return vht_cent_ch_freq(Channel, VHT_BW_80, CMD_CH_BAND_5G);
	else if (Bw == BW_160)
		return vht_cent_ch_freq(Channel, VHT_BW_160, CMD_CH_BAND_5G);

	return Channel;
}

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	switch (QueryParam) {
#if (RDD_2_SUPPORTED == 1)
	case INBAND_CH_BAND0:
		return pDfsParam->band_ch[DBDC_BAND0];

	case INBAND_CH_BAND1:
		return pDfsParam->band_ch[DBDC_BAND1];

	case INBAND_BW_BAND0:
		return pDfsParam->band_bw[DBDC_BAND0];

	case INBAND_BW_BAND1:
		return pDfsParam->band_bw[DBDC_BAND1];

#else
	case INBAND_CH:
		return pDfsParam->band_ch[DBDC_BAND0];

	case INBAND_BW:
		return pDfsParam->band_bw[DBDC_BAND0];

#endif /* RDD_2_SUPPORTED */

	case OUTBAND_CH:
		return pDfsParam->OutBandCh;

	case OUTBAND_BW:
		return pDfsParam->OutBandBw;

	case ORI_INBAND_CH:
		return pDfsParam->OrigInBandCh;

	case ORI_INBAND_BW:
		return pDfsParam->OrigInBandBw;

	default:
		return pDfsParam->band_ch[DBDC_BAND0];

	}
}

VOID DfsGetSysParameters(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR prim_ch;
	UCHAR bandIdx;
	CHANNEL_CTRL *pChCtrl;
#ifdef DOT11_VHT_AC
	UCHAR c2;
#endif /*DOT11_VHT_AC*/

	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	prim_ch = wdev->channel;
	bandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, bandIdx);

#ifdef DOT11_VHT_AC
	c2 = vht_cent2;
	if (phy_bw == BW_8080) {
		pDfsParam->PrimCh = prim_ch;
		pDfsParam->PrimBand = RDD_BAND0;

		pDfsParam->band_ch[RDD_BAND0] = (pDfsParam->PrimBand == RDD_BAND0) ? prim_ch : CentToPrim(c2);
		if (CheckNonOccupancyChannel(pAd, wdev, wdev->vht_sec_80_channel) == TRUE) {
			pDfsParam->band_ch[RDD_BAND1] = wdev->vht_sec_80_channel;
		}
	} else
#endif
	{
		pDfsParam->PrimCh = prim_ch;
		pDfsParam->PrimBand = bandIdx;
		pDfsParam->band_ch[bandIdx] = prim_ch;
	}

	if (phy_bw == BW_8080 || phy_bw == BW_160) {
		pDfsParam->band_bw[RDD_BAND1] = phy_bw;
	}

	pDfsParam->band_bw[bandIdx] = phy_bw;
	pDfsParam->Dot11_H[bandIdx].RDMode = pDot11h->RDMode;
	pDfsParam->bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	pDfsParam->bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;
}


VOID DfsParamInit(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR band_idx, rdd_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;
	PDFS_PULSE_THRESHOLD_PARAM pls_thrshld_param = NULL;
	PDFS_RADAR_THRESHOLD_PARAM radar_thrshld_param = NULL;

	os_zero_mem(pDfsParam, sizeof(DFS_PARAM));

	pDfsParam->PrimBand = RDD_BAND0;
	for (rdd_idx = 0; rdd_idx < HW_RDD_NUM; rdd_idx++) {
		pDfsParam->DfsChBand[rdd_idx] = FALSE;
		pDfsParam->RadarDetected[rdd_idx] = FALSE;
	}
	pDfsParam->bNoSwitchCh = FALSE;
	pDfsParam->bZeroWaitCacSecondHandle = FALSE;
	pDfsParam->bDedicatedZeroWaitSupport = FALSE;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	pDfsParam->bV10ChannelListValid = FALSE;
	pDfsParam->bV10BootACSValid = FALSE;
	pDfsParam->gV10OffChnlWaitTime = 0;
	pDfsParam->bV10W56APDownEnbl = FALSE;
	pDfsParam->bV10APBcnUpdateEnbl =  FALSE;
	pDfsParam->bV10W56GrpValid = FALSE;
	pDfsParam->bV10APInterfaceDownEnbl = FALSE;
	pDfsParam->bV10W56SwitchVHT80 = FALSE;
#endif
	pDfsParam->OutBandCh = 0;
	pDfsParam->OutBandBw = 0;
	pDfsParam->bZeroWaitSupport = 0;
	pDfsParam->bOutBandAvailable = FALSE;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bSetInBandCacReStart = FALSE;
	pDfsParam->bDedicatedZeroWaitDefault = FALSE;
	pDfsParam->bInitOutBandBranch = FALSE;
	pDfsParam->RadarHitReport = FALSE;
	pDfsParam->OutBandAvailableCh = 0;
	pDfsParam->targetCh = 0;
	pDfsParam->targetBw = 0;
	pDfsParam->targetCacValue = 0;

	/* Threshold parameters*/
	radar_thrshld_param = &pAd->CommonCfg.DfsParameter.radar_thrshld_param;
	pls_thrshld_param = &radar_thrshld_param->pls_thrshld_param;

	pls_thrshld_param->pls_width_max = 110; /* unit: us */
	pls_thrshld_param->pls_pwr_max = -10; /* unit: dBm */
	pls_thrshld_param->pls_pwr_min = -80; /* unit: dBm */

	pls_thrshld_param->pri_min_stgr = 40; /* unit: us */
	pls_thrshld_param->pri_max_stgr = 5200; /* unit: us */
	pls_thrshld_param->pri_min_cr = 128; /* unit: us */
	pls_thrshld_param->pri_max_cr = 5200; /* unit: us */

	pDfsParam->fcc_lpn_min = 8;

	pDfsParam->is_hw_rdd_log_en = FALSE;
	pDfsParam->is_sw_rdd_log_en = FALSE;
	pDfsParam->sw_rdd_log_cond = TRUE;
	pDfsParam->is_radar_emu = FALSE;

	/* FCC-1/JP-1 */
	radar_thrshld_param->sw_radar_type[0].rt_det = 0;
	radar_thrshld_param->sw_radar_type[0].rt_en = 1;
	radar_thrshld_param->sw_radar_type[0].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[0].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[0].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[0].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[0].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_pw_max = 13;
	radar_thrshld_param->sw_radar_type[0].rt_pri_min = 508;
	radar_thrshld_param->sw_radar_type[0].rt_pri_max = 3076;
	radar_thrshld_param->sw_radar_type[0].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[0].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[0].rt_stg_pri_diff_min = 0;
	/* FCC-2 */
	radar_thrshld_param->sw_radar_type[1].rt_det = 0;
	radar_thrshld_param->sw_radar_type[1].rt_en = 1;
	radar_thrshld_param->sw_radar_type[1].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[1].rt_crpn_min = 12;
	radar_thrshld_param->sw_radar_type[1].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[1].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[1].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_pw_max = 17;
	radar_thrshld_param->sw_radar_type[1].rt_pri_min = 140;
	radar_thrshld_param->sw_radar_type[1].rt_pri_max = 240;
	radar_thrshld_param->sw_radar_type[1].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[1].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[1].rt_stg_pri_diff_min = 0;
	/* FCC-3 */
	radar_thrshld_param->sw_radar_type[2].rt_det = 0;
	radar_thrshld_param->sw_radar_type[2].rt_en = 1;
	radar_thrshld_param->sw_radar_type[2].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[2].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[2].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[2].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[2].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_pw_max = 22;
	radar_thrshld_param->sw_radar_type[2].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[2].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[2].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[2].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[2].rt_stg_pri_diff_min = 0;
	/* FCC-4 */
	radar_thrshld_param->sw_radar_type[3].rt_det = 0;
	radar_thrshld_param->sw_radar_type[3].rt_en = 1;
	radar_thrshld_param->sw_radar_type[3].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[3].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[3].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[3].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[3].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_pw_max = 32;
	radar_thrshld_param->sw_radar_type[3].rt_pri_min = 190;
	radar_thrshld_param->sw_radar_type[3].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[3].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[3].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[3].rt_stg_pri_diff_min = 0;
	/* FCC-6 */
	radar_thrshld_param->sw_radar_type[4].rt_det = 0;
	radar_thrshld_param->sw_radar_type[4].rt_en = 1;
	radar_thrshld_param->sw_radar_type[4].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[4].rt_crpn_min = 9;
	radar_thrshld_param->sw_radar_type[4].rt_crpn_max = 255;
	radar_thrshld_param->sw_radar_type[4].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[4].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_pw_max = 13;
	radar_thrshld_param->sw_radar_type[4].rt_pri_min = 323;
	radar_thrshld_param->sw_radar_type[4].rt_pri_max = 343;
	radar_thrshld_param->sw_radar_type[4].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[4].rt_crbn_max = 32;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[4].rt_stg_pri_diff_min = 0;
	/* ETSI-1 */
	radar_thrshld_param->sw_radar_type[5].rt_det = 0;
	radar_thrshld_param->sw_radar_type[5].rt_en = 1;
	radar_thrshld_param->sw_radar_type[5].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[5].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[5].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[5].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[5].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_pw_max = 17;
	radar_thrshld_param->sw_radar_type[5].rt_pri_min = 990;
	radar_thrshld_param->sw_radar_type[5].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[5].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[5].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[5].rt_stg_pri_diff_min = 0;
	/* ETSI-2 */
	radar_thrshld_param->sw_radar_type[6].rt_det = 0;
	radar_thrshld_param->sw_radar_type[6].rt_en = 1;
	radar_thrshld_param->sw_radar_type[6].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[6].rt_crpn_min = 9;
	radar_thrshld_param->sw_radar_type[6].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[6].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[6].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_pw_max = 27;
	radar_thrshld_param->sw_radar_type[6].rt_pri_min = 615;
	radar_thrshld_param->sw_radar_type[6].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[6].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[6].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[6].rt_stg_pri_diff_min = 0;
	/* ETSI-3 */
	radar_thrshld_param->sw_radar_type[7].rt_det = 0;
	radar_thrshld_param->sw_radar_type[7].rt_en = 1;
	radar_thrshld_param->sw_radar_type[7].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[7].rt_crpn_min = 8;
	radar_thrshld_param->sw_radar_type[7].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[7].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[7].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_pw_max = 27;
	radar_thrshld_param->sw_radar_type[7].rt_pri_min = 240;
	radar_thrshld_param->sw_radar_type[7].rt_pri_max = 445;
	radar_thrshld_param->sw_radar_type[7].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[7].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[7].rt_stg_pri_diff_min = 0;
	/* ETSI-4 */
	radar_thrshld_param->sw_radar_type[8].rt_det = 0;
	radar_thrshld_param->sw_radar_type[8].rt_en = 1;
	radar_thrshld_param->sw_radar_type[8].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[8].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[8].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[8].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[8].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_pw_max = 42;
	radar_thrshld_param->sw_radar_type[8].rt_pri_min = 240;
	radar_thrshld_param->sw_radar_type[8].rt_pri_max = 510;
	radar_thrshld_param->sw_radar_type[8].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[8].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[8].rt_stg_pri_diff_min = 0;
	/* ETSI-5, 2PRI */
	radar_thrshld_param->sw_radar_type[9].rt_det = 0;
	radar_thrshld_param->sw_radar_type[9].rt_en = 1;
	radar_thrshld_param->sw_radar_type[9].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[9].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[9].rt_pri_min = 2490;
	radar_thrshld_param->sw_radar_type[9].rt_pri_max = 3343;
	radar_thrshld_param->sw_radar_type[9].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[9].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pn_min = 12;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[9].rt_stg_pri_diff_min = (131 - 5);
	/* ETSI-5, 3PRI */
	radar_thrshld_param->sw_radar_type[10].rt_det = 0;
	radar_thrshld_param->sw_radar_type[10].rt_en = 1;
	radar_thrshld_param->sw_radar_type[10].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[10].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[10].rt_pri_min = 2490;
	radar_thrshld_param->sw_radar_type[10].rt_pri_max = 3343;
	radar_thrshld_param->sw_radar_type[10].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[10].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pn_min = 15;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pr_min = 24;
	radar_thrshld_param->sw_radar_type[10].rt_stg_pri_diff_min = (131 - 5);
	/* ETSI-6, 2PRI */
	radar_thrshld_param->sw_radar_type[11].rt_det = 0;
	radar_thrshld_param->sw_radar_type[11].rt_en = 1;
	radar_thrshld_param->sw_radar_type[11].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[11].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[11].rt_pri_min = 823;
	radar_thrshld_param->sw_radar_type[11].rt_pri_max = 2510;
	radar_thrshld_param->sw_radar_type[11].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[11].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pn_min = 18;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[11].rt_stg_pri_diff_min = (59 - 5);
	/*ETSI-6, 3PRI */
	radar_thrshld_param->sw_radar_type[12].rt_det = 0;
	radar_thrshld_param->sw_radar_type[12].rt_en = 1;
	radar_thrshld_param->sw_radar_type[12].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[12].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[12].rt_pri_min = 823;
	radar_thrshld_param->sw_radar_type[12].rt_pri_max = 2510;
	radar_thrshld_param->sw_radar_type[12].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[12].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pn_min = 27;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pr_min = 24;
	radar_thrshld_param->sw_radar_type[12].rt_stg_pri_diff_min = (59 - 5);
	/* JP-2 */
	radar_thrshld_param->sw_radar_type[13].rt_det = 0;
	radar_thrshld_param->sw_radar_type[13].rt_en = 1;
	radar_thrshld_param->sw_radar_type[13].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[13].rt_crpn_min = 7;
	radar_thrshld_param->sw_radar_type[13].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[13].rt_crpr_min = 28;
	radar_thrshld_param->sw_radar_type[13].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_pw_max = 14;
	radar_thrshld_param->sw_radar_type[13].rt_pri_min = 3836;
	radar_thrshld_param->sw_radar_type[13].rt_pri_max = 3856;
	radar_thrshld_param->sw_radar_type[13].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[13].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[13].rt_stg_pri_diff_min = 0;
	/* New JP radar, JP_3 */
	radar_thrshld_param->sw_radar_type[14].rt_det = 0;
	radar_thrshld_param->sw_radar_type[14].rt_en = 1;
	radar_thrshld_param->sw_radar_type[14].rt_stgr = 0;
	radar_thrshld_param->sw_radar_type[14].rt_crpn_min = 6;
	radar_thrshld_param->sw_radar_type[14].rt_crpn_max = 32;
	radar_thrshld_param->sw_radar_type[14].rt_crpr_min = 22;
	radar_thrshld_param->sw_radar_type[14].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_pw_max = 110;
	radar_thrshld_param->sw_radar_type[14].rt_pri_min = 615;
	radar_thrshld_param->sw_radar_type[14].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[14].rt_crbn_min = 1;
	radar_thrshld_param->sw_radar_type[14].rt_crbn_max = 1;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pn_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pn_max = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pr_min = 0;
	radar_thrshld_param->sw_radar_type[14].rt_stg_pri_diff_min = 0;
	/* New JP radar, JP_4 */
	radar_thrshld_param->sw_radar_type[15].rt_det = 0;
	radar_thrshld_param->sw_radar_type[15].rt_en = 1;
	radar_thrshld_param->sw_radar_type[15].rt_stgr = 1;
	radar_thrshld_param->sw_radar_type[15].rt_crpn_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_crpn_max = 0;
	radar_thrshld_param->sw_radar_type[15].rt_crpr_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_pw_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_pw_max = 110;
	radar_thrshld_param->sw_radar_type[15].rt_pri_min = 15;
	radar_thrshld_param->sw_radar_type[15].rt_pri_max = 5010;
	radar_thrshld_param->sw_radar_type[15].rt_crbn_min = 0;
	radar_thrshld_param->sw_radar_type[15].rt_crbn_max = 0;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pn_min = 12;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pn_max = 32;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pr_min = 28;
	radar_thrshld_param->sw_radar_type[15].rt_stg_pri_diff_min = 0;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pAd->Dot11_H[band_idx].DfsZeroWaitChMovingTime = 3;
		pDfsParam->bNoAvailableCh[band_idx] = FALSE;
		pDfsParam->band_ch[band_idx] = 0;
		pDfsParam->RadarDetectState[band_idx] = FALSE;

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
		if ((pDfsParam->NeedSetNewChList[band_idx] == DFS_SET_NEWCH_INIT)
			|| (pChCtrl->ChListNum == 0))
			pDfsParam->NeedSetNewChList[band_idx] = DFS_SET_NEWCH_ENABLED;
		else
			pDfsParam->NeedSetNewChList[band_idx] = DFS_SET_NEWCH_DISABLED;

	}

#ifdef CONFIG_RCSA_SUPPORT
	pDfsParam->fSendRCSA = FALSE;
	pDfsParam->ChSwMode = 1;
#endif

	DfsStateMachineInit(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, pAd->CommonCfg.DfsParameter.DfsStateFunc);
}

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, DFS_MAX_STATE, DFS_MAX_MSG, (STATE_MACHINE_FUNC)Drop, DFS_BEFORE_SWITCH, DFS_MACHINE_BASE);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_CAC_END, (STATE_MACHINE_FUNC)DfsCacEndUpdate);
#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_OFF_CAC_END, (STATE_MACHINE_FUNC)dfs_off_cac_end_update);
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_ENBL, (STATE_MACHINE_FUNC)DfsV10W56APDownEnbl);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_FINISH,
		(STATE_MACHINE_FUNC)DfsV10W56APDownPass);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_ACS_CSA_UPDATE, (STATE_MACHINE_FUNC)DfsV10APBcnUpdate);
#endif
}

INT Set_RadarDetectMode_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
#if !defined(MT7615) && !defined(MT7622)
	UCHAR value, ret;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct DOT11_H *dot11h = NULL;

	if (!wdev)
		return FALSE;

#ifdef CONFIG_ATE
	if (!ATE_ON(pAd)) {
		UINT8 rx_stream = 1;
		UINT8 tx_stream = 1;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): normal mode - set to new T/RX\n",
				 __func__));

		/* Set to 1 TRX stream when detection mode is set */
		wlan_config_set_rx_stream(wdev, rx_stream);
		wlan_config_set_tx_stream(wdev, tx_stream);

		wlan_operate_set_rx_stream(wdev, rx_stream);
		wlan_operate_set_tx_stream(wdev, tx_stream);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			dot11h = wdev->pDot11_H;
			dot11h->RDMode = RD_SWITCHING_MODE;

			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* CONFIG_ATE */

	value = os_str_tol(arg, 0, 10);

	if (value >= RDD_DETMODE_NUM) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR, ("In Set_RadarDetectMode_Proc, invalid mode: %d\n", value));
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("In Set_RadarDetectMode_Proc, mode: %d\n", value));
		ret = mtRddControl(pAd, RDD_DET_MODE, 0, 0, value);
	}

	switch (value) {
	case RDD_DETMODE_OFF: /* Turn OFF detection mode */
		pDfsParam->bNoSwitchCh = FALSE;
		break;
	case RDD_DETMODE_ON: /* Turn ON detection mode */
	case RDD_DETMODE_DEBUG: /* Turn ON detection/debug mode */
		pDfsParam->bNoSwitchCh = TRUE;
		break;
	default:
		pDfsParam->bNoSwitchCh = FALSE;
		break;
	}
#endif /* !defined(MT7615) && !defined(MT7622) */

	return TRUE;
}

INT Set_RadarDetectStart_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	UCHAR phy_bw;
	UCHAR rd_region = 0; /* Region of radar detection */
	value = os_str_tol(arg, 0, 10);
	if (hc_radio_query_by_rf(pAd, RFIC_5GHZ, &oper) != HC_STATUS_OK) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_ERROR, ("%s(): cannot get info\n", __func__));
		return FALSE;
	}
	phy_bw = oper.bw;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc:\n"));
	rd_region = pAd->CommonCfg.RDDurRegion;

	if (value == 0) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 1) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 2) {
#ifdef DOT11_VHT_AC
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);

		if (phy_bw == BW_8080 || phy_bw == BW_160) {
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
			ret2 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
			ret2 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		} else
#endif
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc: Bandwidth not 80+80 or 160\n"));

		pDfsParam->bNoSwitchCh = TRUE;
	} else
		;

	return TRUE;
}


INT Set_RadarDetectStop_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	struct freq_oper oper;
	UCHAR phy_bw;
	if (hc_radio_query_by_rf(pAd, RFIC_5GHZ, &oper) != HC_STATUS_OK)
		return FALSE;

	phy_bw = oper.bw;
	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc:\n"));

	if (value == 0)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
	else if (value == 1)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
	else if (value == 2) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
#ifdef DOT11_VHT_AC

		if (phy_bw == BW_8080 || phy_bw == BW_160)
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		else
#endif
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc: Bandwidth not 80+80 or 160\n"));
	} else
		;

	return TRUE;
}

INT Set_ByPassCac_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR value; /* CAC time */
	UCHAR band_idx;
	value = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("set CAC value to %d\n", value));
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->Dot11_H[band_idx].RDMode == RD_SILENCE_MODE)
			pAd->Dot11_H[band_idx].RDCount = pAd->Dot11_H[band_idx].cac_time;
	}

	pDfsParam->DedicatedOutBandCacCount = pDfsParam->DedicatedOutBandCacTime;
	return TRUE;
}

INT Set_RDDReport_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR value;
	value = os_str_tol(arg, 0, 10);
	pAd->CommonCfg.DfsParameter.is_radar_emu = TRUE;

#if defined(MT7615) || defined(MT7622)
	WrapDfsRddReportHandle(pAd, value);
#else
	mtRddControl(pAd, RDD_RADAR_EMULATE, value, 0, 0);
#endif
	return TRUE;
}

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UCHAR value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Current 5G channel, Band0Ch: %d, Band1Ch: %d\n",
				 pDfsParam->band_ch[DBDC_BAND0], pDfsParam->band_ch[DBDC_BAND1]));
	return TRUE;
}

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UCHAR band_idx = DBDC_BAND0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Current DFS Bw is %d\n", pDfsParam->band_bw[band_idx]));
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UCHAR i;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdevEach = NULL;
	struct DOT11_H *pDot11hEach = NULL;

	value = os_str_tol(arg, 0, 10);

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("pAd->RDMode[%d]=%d\n",
				 i,
				 pAd->Dot11_H[i].RDMode
				 ));
	}
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BssIdx: %d\n", BssIdx));
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdevEach = &pMbss->wdev;
		if (pMbss == NULL || wdevEach == NULL)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wdevIdx: %d. BandIdx: %d, channel: %d\n", wdevEach->wdev_idx, HcGetBandByWdev(wdevEach), wdevEach->channel));
		pDot11hEach = wdevEach->pDot11_H;
		if (pDot11hEach == NULL)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDMode: %d\n\n", pDot11hEach->RDMode));
	}
	return TRUE;
}
#endif

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("RDD Region is %d\n",
			 pAd->CommonCfg.RDDurRegion));
	return TRUE;
}

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT_8 ch_idx, band_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("[%s][RDM]:\n", __func__));

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("band_idx: %d\n", band_idx));

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("DfsChannelList[%d].Channel = %d, NonOccupancy = %d, NOPClrCnt = %d, NOPSetByBw = %d, NOPSaveForClear is %d, SupportBwBitMap is %d\n",
			ch_idx,
			pChCtrl->ChList[ch_idx].Channel,
			pChCtrl->ChList[ch_idx].NonOccupancy,
			pChCtrl->ChList[ch_idx].NOPClrCnt,
			pChCtrl->ChList[ch_idx].NOPSetByBw,
			pChCtrl->ChList[ch_idx].NOPSaveForClear,
			pChCtrl->ChList[ch_idx].SupportBwBitMap));
		}
	}
	return TRUE;
}

INT show_dfs_ch_info_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	UINT_8 band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct DOT11_H *pDot11h = NULL;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("[%s][RDM]: DFS channel info\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=========================================\n "));

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("band_idx: %d\n", band_idx));
		pDot11h = &pAd->Dot11_H[band_idx];
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("CH: %d,\tBW: %d,\tCAC cnt: %d,\tCAC: %d\n",
			pDfsParam->band_ch[band_idx],
			pDfsParam->band_bw[band_idx],
			pDot11h->RDCount,
			pDot11h->cac_time));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-----------------------------------------\n "));
	}

	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->bDedicatedZeroWaitDefault == TRUE)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("dedicated RX:\n"));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("CH: %d,\tBW: %d,\tCAC cnt: %d,\tCAC: %d\n",
			pDfsParam->OutBandCh,
			pDfsParam->OutBandBw,
			pDfsParam->DedicatedOutBandCacCount,
			pDfsParam->DedicatedOutBandCacTime));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=========================================\n "));

	return TRUE;
}

#if !(defined(MT7615) || defined(MT7622) || defined(MT7663))
VOID dfs_dump_radar_sw_pls_info(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 pls_idx = 0, rt_idx = 0;
	UINT32 pri_value = 0;
	BOOLEAN prd_radar_detected = FALSE;
	BOOLEAN sw_rdd_log_cond = pAd->CommonCfg.DfsParameter.sw_rdd_log_cond;

	if (prRadarReport == NULL)
		return;
	if ((prRadarReport->cr_pls_detected == 1) || (prRadarReport->stgr_pls_detected == 1))
		prd_radar_detected = TRUE;

	if ((prRadarReport->lng_pls_detected == 1) || (prd_radar_detected == TRUE) || (sw_rdd_log_cond == FALSE)) {
		if (prRadarReport->lng_pls_detected == 1) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: Long pulse radar is detected\n", prRadarReport->rdd_idx));
		} else {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("===> RDD-%d: No Long pulse radar is detected\n", prRadarReport->rdd_idx));
		}

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("LPN = %d (FCC5_LPN = %d)\n",
			prRadarReport->out_lpn,
			pAd->CommonCfg.DfsParameter.fcc_lpn_min));

		if (prRadarReport->lng_pls_num) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Long pulse buffer----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n"));

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time * 4/10)));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth * 4/10),
					(INT16)((prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr - 1024)/4)));
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->lng_pls_buff[pls_idx].lng_strt_time -
					prRadarReport->lng_pls_buff[pls_idx - 1].lng_strt_time) + RAMP_TIME) % RAMP_TIME);

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("%d\n", pri_value));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("-----------------------------------------------------------\n"));

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Long pulse raw data----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("ST-PW-Power;\n"));

			for (pls_idx = 0; pls_idx < prRadarReport->lng_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u-%u-%d;",
					(UINT32)(prRadarReport->lng_pls_buff[pls_idx].lng_strt_time),
					(UINT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_wdth),
					(INT16)(prRadarReport->lng_pls_buff[pls_idx].lng_pls_pwr)));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n-----------------------------------------------------------\n"));
		}

		if (prd_radar_detected == TRUE) {
			PSW_RADAR_TYPE_T sw_radar_type = NULL;

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: Periodic radar (RT-%d, RT_STGR = %d) is detected\n",
				prRadarReport->rdd_idx,
				prRadarReport->rt_idx,
				prRadarReport->stgr_pls_detected));

			rt_idx = prRadarReport->rt_idx;
			sw_radar_type = &pAd->CommonCfg.DfsParameter.radar_thrshld_param.sw_radar_type[rt_idx];

			if (sw_radar_type == NULL)
				return;

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("SPN = %d\n", prRadarReport->out_spn));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPN = %d \t(RT_CRPN_MIN = %d, RT_CRPN_MAX = %d)\n",
				prRadarReport->out_crpn,
				sw_radar_type->rt_crpn_min,
				sw_radar_type->rt_crpn_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPR = %d/%d \t(RT_CRPR_MIN = %d/%d)\n",
				prRadarReport->out_crpn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_crpr_min, PPB_SIZE));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_crpw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_CONST = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX = %d)\n",
				prRadarReport->out_pri_const,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRBN = %d \t(RT_CRBN_MIN = %d, RT_CRBN_MAX = %d)\n",
				prRadarReport->out_crbn,
				sw_radar_type->rt_crbn_min,
				sw_radar_type->rt_crbn_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG1 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg1,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG2 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
				prRadarReport->out_pri_stg2,
				sw_radar_type->rt_pri_min,
				sw_radar_type->rt_pri_max * 3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_DIFF12 = %d\n", prRadarReport->out_pri_stg_dmin));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
				prRadarReport->out_stg_pw,
				sw_radar_type->rt_pw_min,
				sw_radar_type->rt_pw_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPN = %d \t(RT_STGPN_MIN = %d, RT_STGPN_MAX = %d)\n",
				prRadarReport->out_stg_pn,
				sw_radar_type->rt_stg_pn_min,
				sw_radar_type->rt_stg_pn_max));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPR = %d/%d \t(RT_STGPR_MIN = %d/%d)\n",
				prRadarReport->out_stg_pn,
				prRadarReport->prd_pls_num,
				sw_radar_type->rt_stg_pr_min, PPB_SIZE));
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: No periodic radar is detected\n", prRadarReport->rdd_idx));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
						("SPN = %d\n", prRadarReport->out_spn));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPN = %d\n", prRadarReport->out_crpn));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPR = %d/%d\n",
				prRadarReport->out_crpn, prRadarReport->prd_pls_num));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPW = %d\n",
					prRadarReport->out_crpw));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_CONST = %d\n",
					prRadarReport->out_pri_const));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRBN = %d \n",
					prRadarReport->out_crbn));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG1 = %d \n",
					prRadarReport->out_pri_stg1));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG2 = %d \n",
					prRadarReport->out_pri_stg2));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG3 = %d\n", prRadarReport->out_pri_stg3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STG_PRI12_DIFF = %d\n", prRadarReport->out_pri_stg_dmin));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPW = %d \n",
					prRadarReport->out_stg_pw));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPN = %d \n",
					prRadarReport->out_stg_pn));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPR = %d/%d\n",
				prRadarReport->out_stg_pn, prRadarReport->prd_pls_num));
		}

		if (prRadarReport->prd_pls_num) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Short pulse buffer----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n"));

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t%u\t\t",
					pls_idx,
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time * 4/10)));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t\t%d\t\t",
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth * 4/10),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr - 1024)/4));
				if (pls_idx == 0)
					pri_value = 0;
				else {
					pri_value = (UINT32)(
					((prRadarReport->prd_pls_buff[pls_idx].prd_strt_time -
					prRadarReport->prd_pls_buff[pls_idx - 1].prd_strt_time + RAMP_TIME) % RAMP_TIME));

					pri_value = (pri_value * 4 / 10);
				}
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("%d\n", pri_value));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("--------------------------------------------------------------\n"));

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Short pulse raw data----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("ST-PW-Power;\n"));

			for (pls_idx = 0; pls_idx < prRadarReport->prd_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u-%u-%d;",
					(UINT32)(prRadarReport->prd_pls_buff[pls_idx].prd_strt_time),
					(UINT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_wdth),
					(INT16)(prRadarReport->prd_pls_buff[pls_idx].prd_pls_pwr)));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n--------------------------------------------------------------\n"));
		}
	}

}

VOID dfs_dump_radar_hw_pls_info(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 pls_idx = 0;

	if (prRadarReport == NULL)
		return;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n--------------------------------------------------------------\n"));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("===> RDD-%d: Interrupt\n", prRadarReport->rdd_idx));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n------------------------HW pulse buffer-----------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("Index\t | ST(us)\t | PW(us)\t | Power(dBm)\t | \tSC\t | \tReset\t | \tMDRDY | \tTX_active\n"));

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%d\t%u\t\t", pls_idx,
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time * 4/10)));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%u\t\t%d\t\t",
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width * 4/10),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr - 1024)/4));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%s\t\t%s\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_sc_pass ? "true":"false",
			prRadarReport->hw_pls_buff[pls_idx].hw_sw_reset ? "true":"false"));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%d\t\t", prRadarReport->hw_pls_buff[pls_idx].hw_mdrdy_flag));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%d\t\t\n", prRadarReport->hw_pls_buff[pls_idx].hw_tx_active));
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("--------------------------------------------------------------\n"));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n------------------------HW pulse raw data-----------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("ST-PW-Power;\n"));

	for (pls_idx = 0; pls_idx < prRadarReport->hw_pls_num; pls_idx++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%u-%u-%d;",
			(UINT32)(prRadarReport->hw_pls_buff[pls_idx].hw_start_time),
			(UINT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_width),
			(INT16)(prRadarReport->hw_pls_buff[pls_idx].hw_pls_pwr)));
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n--------------------------------------------------------------\n"));

}

VOID dfs_update_radar_info(
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 rdd_idx = HW_RDD0;

	if (prRadarReport == NULL)
		return;

	rdd_idx = prRadarReport->rdd_idx;

	switch (rdd_idx) {
	case HW_RDD0:
	case HW_RDD1:
#if (RDD_2_SUPPORTED == 1)
	case HW_RDD2:
#endif /* RDD_2_SUPPORTED */
		os_zero_mem(&g_radar_info[rdd_idx], sizeof(EXT_EVENT_RDD_REPORT_T));
		g_radar_info[rdd_idx] = *prRadarReport;
		break;

	default:
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() wrong parameter rdd_idx %d\n", __func__, rdd_idx));
		break;
	}
}

INT show_dfs_debug_proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	INT16 value = 0;
	UCHAR pls_idx = 0, rdd_idx = 0;

	value = (INT16)simple_strtol(arg, 0, 10);

	if (value == 1) {
		os_zero_mem(&g_radar_info, sizeof(EXT_EVENT_RDD_REPORT_T) * HW_RDD_NUM);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Info clear\n"));
	} else if (value == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Debug info Start\n"));
		for (rdd_idx = HW_RDD0; rdd_idx < HW_RDD_NUM; rdd_idx++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD%d INFO\n", rdd_idx));

			if (!(g_radar_info[rdd_idx].lng_pls_detected ||
				g_radar_info[rdd_idx].cr_pls_detected ||
				g_radar_info[rdd_idx].stgr_pls_detected)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNo data\n"));
				continue;
			}

			for (pls_idx = 0; pls_idx < g_radar_info[rdd_idx].lng_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t", pls_idx));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_strt_time));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_pls_wdth));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\n",
					g_radar_info[rdd_idx].lng_pls_buff[pls_idx].lng_pls_pwr));
			}

			for (pls_idx = 0; pls_idx < g_radar_info[rdd_idx].prd_pls_num; pls_idx++) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t", pls_idx));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_strt_time));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_pls_wdth));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\n",
					g_radar_info[rdd_idx].prd_pls_buff[pls_idx].prd_pls_pwr));
			}
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Debug info End\n"));
	}
	return TRUE;
}
#endif

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UINT_8 band_idx, ch_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	value = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Set NOP of all channel as %ld.\n", value));

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			pChCtrl->ChList[ch_idx].NonOccupancy = value;
			pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
			pChCtrl->ChList[ch_idx].NOPSetByBw = 0;
		}
	}

	return TRUE;
}

/* DFS Zero Wait */
INT Set_DfsZeroWaitCacTime_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR Value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	Value = (UCHAR) os_str_tol(arg, 0, 10);
	pDfsParam->DfsZeroWaitCacTime = Value;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("[%s][RDM]CacTime=%d/%d\n",
			 __func__,
			 Value,
			 pDfsParam->DfsZeroWaitCacTime));
	return TRUE;
}

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	UCHAR SynNum = 0, Channel = 0, Bw = 0, doCAC = 1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* Set Syn Num*/
			SynNum = simple_strtol(value, 0, 10);
			break;
		case 1: /* Set InBand ControlChannel */
			Channel = simple_strtol(value, 0, 10);
			break;
		case 2: /* Set InBand Bw*/
			Bw = simple_strtol(value, 0, 10);
			break;
		case 3: /* Set doCAC*/
			doCAC = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

    /* Disable zero-wait default flow */
	pDfsParam->bDedicatedZeroWaitDefault = FALSE;

#ifdef BACKGROUND_SCAN_SUPPORT
	switch (SynNum) {
	case RDD_BAND0:
#if (RDD_2_SUPPORTED == 1)
	case RDD_BAND1:
#endif /* RDD_2_SUPPORTED */
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC, SynNum);
		break;

	case RDD_DEDICATED_RX:
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw, SynNum);
		break;

	default:
		break;
	}

#endif

	return TRUE;
}

INT Set_DfsZeroWaitDynamicCtrl_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;

	Value = (UCHAR) simple_strtol(arg, 0, 10);

#ifdef BACKGROUND_SCAN_SUPPORT
#if (RDD_2_SUPPORTED == 0)
	DfsDedicatedDynamicCtrl(pAd, Value);
#endif
#endif

	return TRUE;
}

INT Set_DfsZeroWaitNOP_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			NOPTime = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

	return TRUE;
}

INT Set_DfsTargetCh_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
		INT	i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT CacValue = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			CacValue = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);

	return TRUE;
}

VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal)
{
	if (!DisableDfsCal)
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Enable DFS calibration in firmware.\n"));
	else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Disable DFS calibration in firmware.\n"));
		mtRddControl(pAd, DISABLE_DFS_CAL, HW_RDD0, 0, 0);
	}
}

VOID DfsSetZeroWaitCacSecond(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bZeroWaitCacSecondHandle = TRUE;
}

BOOLEAN DfsBypassRadarStateCheck(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = NULL;
	struct radio_dev *rdev;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return FALSE;

	obj = wdev->pHObj;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return FALSE;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	rdev = obj->rdev;

	if (pDot11h->RDMode == RD_NORMAL_MODE)
		return TRUE;
	return FALSE;
}
BOOLEAN DfsRadarChannelCheck(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN ret = FALSE;

	if (!IS_CH_ABAND(wdev->channel)) {
		return FALSE;
	}

	if (!pDfsParam->bDfsEnable)
		return FALSE;

#ifdef DOT11_VHT_AC
	if (phy_bw == BW_8080) {
		ret = (RadarChannelCheck(pAd, wdev->channel)
		|| RadarChannelCheck(pAd, CentToPrim(vht_cent2)));

	} else if ((phy_bw == BW_160) && (wdev->channel >= GROUP1_LOWER && wdev->channel <= GROUP1_UPPER)) {
		ret = TRUE;
	} else
#endif
	{
		ret = RadarChannelCheck(pAd, wdev->channel);
	}

	if (ret == TRUE || pDfsParam->bDedicatedZeroWaitSupport)
		DfsGetSysParameters(pAd, wdev, vht_cent2, phy_bw);

	return ret;

}

VOID DfsCacEndUpdate(
	RTMP_ADAPTER * pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UCHAR band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 BssIdx = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR wdev_band_index = DBDC_BAND0;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] CAC end. Enable MAC TX.\n", __func__));
	band_idx = (UCHAR)(Elem->Priv);
	mtRddControl(pAd, CAC_END, band_idx, 0, 0);

	if (DfsCacTimeOutCallBack) {
		DfsCacTimeOutCallBack(band_idx, pDfsParam->band_bw[band_idx], pDfsParam->band_ch[band_idx]);
	}

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		wdev_band_index = HcGetBandByWdev(wdev);
		if ((wdev->bAllowBeaconing) && (wdev_band_index == band_idx) && (!wdev->bcn_buf.bBcnSntReq)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] Enabling Beaconing.\n", __func__));
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
		}
	}
}

#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
VOID dfs_off_cac_end_update(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
	UCHAR band_idx = 0;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	struct wifi_dev *wdev = NULL;
	UINT_8 BssIdx = 0;
#endif

	if (pDfsParam->bDedicatedZeroWaitDefault == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() ZeroWaitDefault is not enabled\n", __func__));
		return;
	}

	if (!pDfsParam->bOutBandAvailable) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() OutBand is not available\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() zero-wait CAC end, ch_stat %d\n", __func__, *ch_stat));

	switch (*ch_stat) {
	case DFS_OUTB_CH_CAC:
		/* Assign DFS outband Channel to inband Channel */
		/* use channel to find band index */
		band_idx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() New inband channel %d bandidx %d\n",
					__func__, pDfsParam->OutBandCh, band_idx));

#ifdef DFS_CAC_R2
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		/*test -> avoid this setting of outband channel onto inband if cac time expires and no radar is detected , no need for this step at all .*/
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("OUT band channel CAC end on ch %d, but avoid switching inband channel\n", pDfsParam->OutBandCh));
		} else
#endif
		{
		/* Assign DFS outband Channel to inband Channel */
		*ch_stat = DFS_INB_CH_SWITCH_CH;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if ((pDfsParam->DFSChHitBand == DBDC_BAND0) || (pAd->CommonCfg.dbdc_mode)) {
			DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE, pDfsParam->DFSChHitBand);
		} else if (pDfsParam->DFSChHitBand == DBDC_BAND1) {
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
				if (!pAd->CommonCfg.dbdc_mode)
					wdev->vht_sec_80_channel = pDfsParam->OutBandCh;
				wlan_config_set_cen_ch_2(wdev, DfsPrimToCent(pDfsParam->OutBandCh, BW_80));
				wlan_operate_set_cen_ch_2(wdev, DfsPrimToCent(pDfsParam->OutBandCh, BW_80));
			}
			DfsDedicatedInBandSetChannel(
				pAd,
				pDfsParam->band_ch[RDD_BAND0],
				pDfsParam->OutBandBw,
				FALSE,
				DBDC_BAND0);
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Not hit any condition\n", __func__));
		}
#else
		DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE, band_idx);
#endif
		}
		break;

	case DFS_INB_DFS_OUTB_CH_CAC:
	case DFS_INB_DFS_OUTB_CH_CAC_DONE:
		/* new zero-wait CAC of outband is available */
		band_idx = dfs_get_band_by_ch(pAd, pDfsParam->OutBandCh);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() zero-wait CAC ch %d is available, bandidx %d\n",
					__func__, pDfsParam->OutBandCh, band_idx));
		*ch_stat = DFS_INB_DFS_OUTB_CH_CAC_DONE;
		break;

	default:
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() invalid input %d\n", __func__, *ch_stat));
		break;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() ch_stat %d\n", __func__, *ch_stat));
}

UCHAR dfs_get_band_by_ch(
	RTMP_ADAPTER *pAd,
	UCHAR ch)
{
	UCHAR band_idx = BAND0;
	UCHAR ch_idx;
	UCHAR b_idx;
	CHANNEL_CTRL *pChCtrl = NULL;

	for (b_idx = BAND0; b_idx < BAND_NUM; b_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, b_idx);

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].Channel == ch) {
				band_idx = b_idx;
			}
		}
	}

	return band_idx;
}


#endif

#ifdef CONFIG_AP_SUPPORT
NTSTATUS DfsChannelSwitchTimeoutAction(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	struct wlan_config *cfg;
#endif

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] bandIdx: %d, BssIdx: %d, NextCh: %d\n",
		__func__, bandIdx, BssIdx, NextCh));
	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	pDfsParam->RadarHitIdxRecord = bandIdx;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	cfg = (struct wlan_config *)wdev->wpf_cfg;
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	DedicatedZeroWaitStop(pAd, FALSE);
#endif

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		wdev->quick_ch_change = QUICK_CH_SWICH_DISABLE;
#endif
	rtmp_set_channel(pAd, wdev, NextCh);

	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, bandIdx);
	} else
	{
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (cfg)
			if (cfg->vht_conf.vht_bw == VHT_BW_8080)
				MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
#endif
	}
	DfsReportCollision(pAd);
	return 0;
}

NTSTATUS DfsSwitchChAfterRadarDetected(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] bandIdx: %d, BssIdx: %d, NextCh: %d\n",
		__func__, bandIdx, BssIdx, NextCh));

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	rtmp_set_channel(pAd, wdev, NextCh);
	return 0;
}

NTSTATUS DfsAPRestart(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	struct wlan_config *cfg;
#endif

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	pDfsParam->RadarHitIdxRecord = bandIdx;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	cfg = (struct wlan_config *)wdev->wpf_cfg;
#endif

	APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);

	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, bandIdx);
	} else {
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (cfg)
			if (cfg->vht_conf.vht_bw == VHT_BW_8080)
				MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
#endif
	}

	return 0;
}
#endif

VOID DfsCacNormalStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR CompareMode)
{
	struct DOT11_H *pDot11h = NULL;
	UCHAR band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (wdev == NULL)
		return;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	band_idx = HcGetBandByWdev(wdev);
	if (band_idx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Invalid band_idx(%d)\n", __func__, band_idx));
		return;
	}

	if ((pAd->CommonCfg.RDDurRegion == CE) &&
	    DfsCacRestrictBand(pAd, pDfsParam->band_bw[band_idx], pDfsParam->band_ch[band_idx],
			       pDfsParam->band_ch[(UINT_8)(DBDC_BAND1 - band_idx)])) {
		/* Weather band channel */
		if (pDfsParam->targetCh != 0)
			pDot11h->cac_time = pDfsParam->targetCacValue;
		else
			pDot11h->cac_time = CAC_WETHER_BAND;
	} else {
		if (pDfsParam->targetCh != 0)
			pDot11h->cac_time = pDfsParam->targetCacValue;
		else
			pDot11h->cac_time = CAC_NON_WETHER_BAND;
	}

	if ((pDot11h->RDMode == RD_SILENCE_MODE) && (CompareMode == RD_SILENCE_MODE)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] CAC %d seconds start . Disable MAC TX\n",
				__func__, pDot11h->cac_time));
		mtRddControl(pAd, CAC_START, band_idx, 0, 0);
	} else if ((pDot11h->RDMode == RD_NORMAL_MODE) && (CompareMode == RD_NORMAL_MODE)) {
#if defined(MT7615) || defined(MT7622)
		if (!IS_CH_ABAND(wdev->channel)) {
			UCHAR BssIdx;
			BSS_STRUCT *pMbss = NULL;
			struct wifi_dev *wdevEach = NULL;
			struct DOT11_H *pDot11hEach = NULL;
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdevEach = &pMbss->wdev;
				if (pMbss == NULL || wdevEach == NULL)
					continue;
				if (wdevEach->pHObj == NULL)
					continue;
				pDot11hEach = wdevEach->pDot11_H;
				if (pDot11hEach == NULL)
					continue;
				if (pDot11hEach->RDMode == RD_SILENCE_MODE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] 2G channel with 5G silence channel exist\n", __func__));
					return;
				}
			}
		}
#endif
		if (RadarChannelCheck(pAd, wdev->channel))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM] Normal start. Enable MAC TX\n", __func__));

		mtRddControl(pAd, NORMAL_START, band_idx, 0, 0);
	} else
		;
}

BOOLEAN DfsCacRestrictBand(/* Weather band channel: 5600 MHz - 5650 MHz */
	IN PRTMP_ADAPTER pAd, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh)
{
	BOOLEAN ret = FALSE;
#ifdef DOT11_VHT_AC
	if (Bw == BW_8080) {
		return RESTRICTION_BAND_1(pAd, Ch, Bw) || RESTRICTION_BAND_1(pAd, SecCh, Bw);
	} else if ((Bw == BW_160) && (Ch >= GROUP3_LOWER && Ch <= RESTRICTION_BAND_HIGH)) {
		return TRUE;
	} else
#endif
	{
		if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
			ret = RESTRICTION_BAND_KOREA(pAd, Ch, Bw);
		else
			ret = RESTRICTION_BAND_1(pAd, Ch, Bw);
		return ret;
	}
}

VOID DfsBuildChannelList(
    IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR band_idx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s][RDM]: wdev is NULL.\n", __func__));
		return;
	}
	if (!WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]: wdev is not 5G \n", __func__));
		return;
	}

	band_idx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		/* Non DFS channel, no need to update channel list */
		if ((pAd->CommonCfg.dbdc_mode == TRUE) && !RadarChannelCheck(pAd, wdev->channel)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Non DFS channel\n", __func__));
			return;
		}
	}
#endif

	if (pDfsParam->NeedSetNewChList[band_idx] == DFS_SET_NEWCH_ENABLED) {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			pChCtrl->ChList[i].SupportBwBitMap = 0;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_20, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x01;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_40, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x02;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_80, pChCtrl) ||
				DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_8080, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x04;
			if (DfsCheckChAvailableByBw(pChCtrl->ChList[i].Channel, BW_160, pChCtrl))
				pChCtrl->ChList[i].SupportBwBitMap |= 0x08;
		}
	}
	DfsBuildChannelGroupByBw(pAd, wdev);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Done\n", __func__));
	pDfsParam->NeedSetNewChList[band_idx] = DFS_SET_NEWCH_DISABLED;
}

VOID DfsBuildChannelGroupByBw(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	UINT_8 ch_idx, band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	INT_8 BW40GroupIdx = -1;
	INT_8 BW80GroupIdx = -1;
	INT_8 BW160GroupIdx = -1;
	INT_8 BW40GroupMemberCnt = 0;
	INT_8 BW80GroupMemberCnt = 0;
	INT_8 BW160GroupMemberCnt = 0;
	UINT_8 PreviousBW40CentCh = 0xff;
	UINT_8 PreviousBW80CentCh = 0xff;
	UINT_8 PreviousBW160CentCh = 0xff;
	CHANNEL_CTRL *pChCtrl = NULL;

	band_idx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (!IS_CH_ABAND(pChCtrl->ChList[ch_idx].Channel))
			continue;
		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_40, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_40) != PreviousBW40CentCh) {
				BW40GroupMemberCnt = 0;
				if ((++BW40GroupIdx < DFS_BW40_GROUP_NUM) && (BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp[band_idx].Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = ch_idx;
			} else {
				if ((BW40GroupIdx >= 0) && (BW40GroupIdx < DFS_BW40_GROUP_NUM)
				 && (++BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp[band_idx].Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = ch_idx;
			}

			PreviousBW40CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_40);
		}

		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_80, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_80) != PreviousBW80CentCh) {
				BW80GroupMemberCnt = 0;
				if ((++BW80GroupIdx < DFS_BW80_GROUP_NUM) && (BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp[band_idx].Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = ch_idx;
			} else {
				if ((BW80GroupIdx >= 0) && (BW80GroupIdx < DFS_BW80_GROUP_NUM)
				 && (++BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
				pDfsParam->dfs_ch_grp[band_idx].Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = ch_idx;
			}

			PreviousBW80CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_80);
		}
		if (!ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, BW_160, pChCtrl)) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_160) != PreviousBW160CentCh) {
				BW160GroupMemberCnt = 0;
				if ((++BW160GroupIdx < DFS_BW160_GROUP_NUM) && (BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp[band_idx].Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = ch_idx;
			} else {
				if ((BW160GroupIdx >= 0) && (BW160GroupIdx < DFS_BW160_GROUP_NUM)
				 && (++BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->dfs_ch_grp[band_idx].Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = ch_idx;
			}

			PreviousBW160CentCh = DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, BW_160);
		}

	}
}

BOOLEAN DfsCheckBwGroupAllAvailable(
    UCHAR CheckChIdx, UCHAR Bw, IN PRTMP_ADAPTER pAd, IN UCHAR band_idx)
{
	UCHAR *pBwxxGroupIdx = NULL;
	UCHAR i, j;
	UCHAR GroupNum = 4, BwxxPrimNum = 4;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp[band_idx].Bw40GroupIdx[0][0];
		GroupNum = DFS_BW40_GROUP_NUM;
		BwxxPrimNum = DFS_BW40_PRIMCH_NUM;
	} else if (Bw == BW_80) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp[band_idx].Bw80GroupIdx[0][0];
		GroupNum = DFS_BW80_GROUP_NUM;
		BwxxPrimNum = DFS_BW80_PRIMCH_NUM;
	} else if (Bw == BW_160) {
		pBwxxGroupIdx = &pDfsParam->dfs_ch_grp[band_idx].Bw160GroupIdx[0][0];
		GroupNum = DFS_BW160_GROUP_NUM;
		BwxxPrimNum = DFS_BW160_PRIMCH_NUM;
	} else
		return FALSE;

	for (i = 0; i < (GroupNum * BwxxPrimNum); i++) {
		if (*pBwxxGroupIdx == CheckChIdx) {
			break;
		}
		pBwxxGroupIdx++;
	}

	if (i >= (GroupNum * BwxxPrimNum))
		return FALSE;

	j = i%BwxxPrimNum;
	i = i/BwxxPrimNum;

	pBwxxGroupIdx = pBwxxGroupIdx - j;

	for (j = 0; j < BwxxPrimNum; j++) {
		if (pChCtrl->ChList[*pBwxxGroupIdx].NonOccupancy != 0)
			return FALSE;
		if ((pChCtrl->ChList[*pBwxxGroupIdx].NonOccupancy == 0)
		 && (pChCtrl->ChList[*pBwxxGroupIdx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[*pBwxxGroupIdx].NOPSetByBw <= Bw)
		 )
			return FALSE;

		pBwxxGroupIdx++;
	}

	return TRUE;
}

BOOLEAN DfsSwitchCheck(
	IN PRTMP_ADAPTER pAd,
	UCHAR Channel,
	UCHAR bandIdx)
{
	if ((pAd->Dot11_H[bandIdx].RDMode == RD_SILENCE_MODE) && (Channel > 14)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[DfsSwitchCheck]: DFS ByPass TX calibration.\n"));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSwitchCheck]: NON DFS calibration.\n"));
		return FALSE;
	}
}

BOOLEAN DfsStopWifiCheck(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	return (pDfsParam->bNoAvailableCh[band_idx] == TRUE);
}


#ifdef CONFIG_AP_SUPPORT
VOID DfsNonOccupancyCountDown(/*NonOccupancy --*/
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 ch_idx, band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN Band0Available = FALSE, Band1Available = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;


	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].NonOccupancy > 0)
#ifdef CONFIG_MAP_SUPPORT
			{
#endif
				pChCtrl->ChList[ch_idx].NonOccupancy--;
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_TURNKEY_ENABLE(pAd)) {
					if (pChCtrl->ChList[ch_idx].NonOccupancy == 0) {
						int j = 0;

						for (j = 0; j < WDEV_NUM_MAX; j++) {
							if (pAd->wdev_list[j]) {
								if (pAd->CommonCfg.dbdc_mode) {
									if (HcGetBandByWdev(pAd->wdev_list[j])
										!= HW_RDD1)
									continue;
								} else if ((HcGetBandByWdev(pAd->wdev_list[j]) != HW_RDD0))
									continue;
								wapp_send_radar_detect_notif(pAd, pAd->wdev_list[j],
										pChCtrl->ChList[ch_idx].Channel, TRUE);
								break;
							}
						}
					}
				}
			}
#endif

			if (pChCtrl->ChList[ch_idx].NOPSaveForClear > 0) {
				pChCtrl->ChList[ch_idx].NOPSaveForClear--;
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_TURNKEY_ENABLE(pAd)) {
					if (pChCtrl->ChList[ch_idx].NOPSaveForClear == 0) {
						int j = 0;

						for (j = 0; j < WDEV_NUM_MAX; j++) {
							if (pAd->wdev_list[j]) {
								if (pAd->CommonCfg.dbdc_mode) {
									if (HcGetBandByWdev(pAd->wdev_list[j])
										!= HW_RDD1)
									continue;
								} else if ((HcGetBandByWdev(pAd->wdev_list[j]) != HW_RDD0))
									continue;
								wapp_send_radar_detect_notif(pAd, pAd->wdev_list[j],
										pChCtrl->ChList[ch_idx].Channel, TRUE);
								break;
							}
						}
					}
				}
#endif

			}

			else if ((pChCtrl->ChList[ch_idx].NOPSaveForClear == 0)
					&& (pChCtrl->ChList[ch_idx].NOPClrCnt != 0))
				pChCtrl->ChList[ch_idx].NOPClrCnt = 0;
		}


		if (pDfsParam->bNoAvailableCh[band_idx] == TRUE) {
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if ((pDfsParam->band_bw[band_idx] != BW_8080) &&
					(pChCtrl->ChList[ch_idx].Channel == pDfsParam->PrimCh)) {
					if (pChCtrl->ChList[ch_idx].NonOccupancy == 0)
						Band0Available = TRUE;

				}
				if ((pDfsParam->band_bw[band_idx] == BW_8080) &&
					(pChCtrl->ChList[ch_idx].Channel == pDfsParam->band_ch[DBDC_BAND0])) {
					if (pChCtrl->ChList[ch_idx].NonOccupancy == 0)
						Band0Available = TRUE;

				}
				if ((pDfsParam->band_bw[band_idx] == BW_8080) &&
					(pChCtrl->ChList[ch_idx].Channel == pDfsParam->band_ch[DBDC_BAND1])) {
					if (pChCtrl->ChList[ch_idx].NonOccupancy == 0)
						Band1Available = TRUE;

				}
			}

			if (((pDfsParam->band_bw[band_idx] != BW_8080) && (Band0Available == TRUE)) ||
				((pDfsParam->band_bw[band_idx] == BW_8080) && (Band0Available == TRUE) && (Band1Available == TRUE))) {
				pDfsParam->bNoAvailableCh[band_idx] = FALSE;

			}
		}
	}
}
#endif

VOID WrapDfsSetNonOccupancy(/* Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	IN UCHAR rddidx,
	IN UCHAR band_idx
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 target_ch, target_bw;
	BOOLEAN target_ch_dfsband = FALSE;
	UCHAR idx;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("[%s][RDM]: band index: %d\n",
		__func__, band_idx));

	switch (band_idx) {
	case DBDC_BAND0:
	case DBDC_BAND1:
		if (pDfsParam->Dot11_H[band_idx].RDMode == RD_SWITCHING_MODE)
			return;
		break;

	default:
		break;
	}

	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE)
		&& (pDfsParam->RadarDetected[RDD_DEDICATED_RX] == TRUE)) {
		target_ch = pDfsParam->OutBandCh;
		target_bw = pDfsParam->OutBandBw;
		target_ch_dfsband = pDfsParam->DfsChBand[RDD_DEDICATED_RX];
	} else {
		target_ch = pDfsParam->band_ch[rddidx];
		target_bw = pDfsParam->band_bw[rddidx];
		target_ch_dfsband = pDfsParam->DfsChBand[rddidx];
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("[%s][RDM]: rddidx: %d, target_ch %d, target_bw %d, target_ch_dfsband %d\n",
		__func__, rddidx, target_ch, target_bw, target_ch_dfsband));

	/* Set NOP for channel of band0 and band1 if radar is detected */
	for (idx = 0; idx < DBDC_BAND_NUM; idx++)
		DfsSetNonOccupancy(pAd, idx, target_ch, target_bw, target_ch_dfsband);
}

VOID DfsSetNonOccupancy(/* Set channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	IN UCHAR band_idx,
	IN UINT_8 target_ch,
	IN UINT_8 target_bw,
	IN BOOLEAN target_ch_dfsband
)
{
	UINT_8 ch_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	switch (target_bw) {
	case BW_20:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((target_ch == pChCtrl->ChList[ch_idx].Channel)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
			}
		}
		break;

	case BW_40:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((target_ch == pChCtrl->ChList[ch_idx].Channel)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
			} else if (((target_ch) >> 2 & 1) && ((pChCtrl->ChList[ch_idx].Channel - target_ch) == 4)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
			} else if (!((target_ch) >> 2 & 1) && ((target_ch - pChCtrl->ChList[ch_idx].Channel) == 4)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
			}
			else
				;
		}
		break;

	case BW_80:
		if (target_ch_dfsband == FALSE)
			return;

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_80, CMD_CH_BAND_5G) ==
				vht_cent_ch_freq(target_ch, VHT_BW_80, CMD_CH_BAND_5G)) {
				pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
				pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
			}
		}
		break;

	case BW_8080:
		if (pDfsParam->DfsChBand[HW_RDD0] && pDfsParam->RadarDetected[HW_RDD0]) {
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_8080, CMD_CH_BAND_5G) ==
					vht_cent_ch_freq(pDfsParam->band_ch[DBDC_BAND0], VHT_BW_8080, CMD_CH_BAND_5G)) {
					pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
					pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				}
			}
		} else if (pDfsParam->DfsChBand[HW_RDD1] && pDfsParam->RadarDetected[HW_RDD1]) {
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_8080, CMD_CH_BAND_5G) ==
					vht_cent_ch_freq(pDfsParam->band_ch[DBDC_BAND1], VHT_BW_8080, CMD_CH_BAND_5G)) {
					pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
					pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				}
			}
		}
		break;

	case BW_160:
		if (pDfsParam->DfsChBand[HW_RDD0] || pDfsParam->DfsChBand[HW_RDD1]) {
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if (vht_cent_ch_freq(pChCtrl->ChList[ch_idx].Channel, VHT_BW_160, CMD_CH_BAND_5G) ==
					vht_cent_ch_freq(pDfsParam->band_ch[DBDC_BAND0], VHT_BW_160, CMD_CH_BAND_5G)) {
					pChCtrl->ChList[ch_idx].NonOccupancy = CHAN_NON_OCCUPANCY;
					pChCtrl->ChList[ch_idx].NOPSetByBw = target_bw;
				}
			}
		}
		break;

	default:
		break;
	}

}

#ifdef CONFIG_AP_SUPPORT
VOID WrapDfsRddReportHandle(/* handle the event of EXT_EVENT_ID_RDD_REPORT */
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR NextCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR band_idx;
	UCHAR BssIdx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
	BOOLEAN RadarBandId[DBDC_BAND_NUM];
	UINT_8 i = 0;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *dot11h_param = NULL;
#if defined(DFS_VENDOR10_CUSTOM_FEATURE)
	USHORT BwChannel;
#endif
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(MAP_R2)
	OFFCHANNEL_SCAN_MSG Rsp;
#endif
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R2
		UCHAR band;
		UCHAR first_wdev = TRUE;
		UCHAR Channel_for_radar = 0;
#endif
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s][RDM]:  Radar detected !!!!!!!!!!!!!!!!!\n", __func__));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s][RDM]:  ucRddIdx: %d\n", __func__, ucRddIdx));
	if (pDfsParam->bNoSwitchCh) {
		return;
	}
#if defined(MT7615) || defined(MT7622)
	if (pDfsParam->Bw == BW_8080 || pDfsParam->Bw == BW_160)
		band_idx = HW_RDD0;
	else
#endif
		band_idx = dfs_rddidx_to_dbdc(pAd, ucRddIdx);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  ucRddIdx: %d\n", ucRddIdx));

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1) //cid WCNCR00224666
        if (pDfsParam->bDedicatedZeroWaitSupport == FALSE) {
            pDfsParam->DFSChHitBand = ucRddIdx;
        }
#endif
	
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(MAP_R2)
	if (IS_MAP_ENABLE(pAd)) {
		Rsp.Action = DFS_RADAR_HIT;
		band = (pAd->CommonCfg.dbdc_mode) ? DBDC_BAND1 : DBDC_BAND0;
		memcpy(Rsp.ifrn_name, pAd->ScanCtrl[band].if_name, IFNAMSIZ);
		pAd->radar_hit = TRUE;
		Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
		Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
		Rsp.data.operating_ch_info.is4x4Mode = 1;/* Can be used as an info from driver by default yes */

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			pMbss = &pAd->ApCfg.MBSSID[BssIdx];
			wdev = &pMbss->wdev;
			if (wdev->pHObj == NULL)
				continue;
			if (HcGetBandByWdev(wdev) != band)
				continue;
			Rsp.ifIndex = RtmpOsGetNetIfIndex(wdev->if_dev);
		}
		if (band_idx == HW_RDD2)
			Rsp.data.operating_ch_info.channel = pDfsParam->OutBandCh;
		else
			Rsp.data.operating_ch_info.channel = wdev->channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  Channel: %d %d\n", Rsp.data.operating_ch_info.channel, pDfsParam->OutBandCh));
		if (Rsp.data.operating_ch_info.channel > 14) {
			RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_OFFCHANNEL_INFO,
					NULL,
					(UCHAR *) &Rsp,
					sizeof(OFFCHANNEL_SCAN_MSG));
		}
		/* Band index: BAND0 / Band1 and Dedicated RX */
#if defined(MT7915)
		band_idx = dfs_rddidx_to_dbdc(pAd, ucRddIdx);
		Channel_for_radar = Rsp.data.operating_ch_info.channel;
#endif
	}
#endif

	switch (band_idx) {
	case DBDC_BAND0:
	case DBDC_BAND1:
		dot11h_param = &pAd->Dot11_H[band_idx];
		pDfsParam->Dot11_H[band_idx].RDMode = dot11h_param->RDMode;
		break;

#if (RDD_2_SUPPORTED == 1)
	case RDD_DEDICATED_RX:
		break;
#endif

	default:
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: error band_idx = %d\n", __FUNCTION__, band_idx));
		return;
	}

	if (!DfsRddReportHandle(pAd, pDfsParam, ucRddIdx, band_idx))
		return;
	/* By pass these setting when dedicated DFS zero wait is enabled and radar is detected on out-band */
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE)
	&& (pDfsParam->RadarDetected[RDD_DEDICATED_RX] == TRUE))
		;
	else {
		if (!dot11h_param) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Dereferencing NULL pointer\n", __func__));
			return;
		}
		if (dot11h_param->RDMode == RD_SILENCE_MODE)
			dot11h_param->RDCount = 0;
	}
	WrapDfsSetNonOccupancy(pAd, ucRddIdx, band_idx);

#ifdef BACKGROUND_SCAN_SUPPORT
	/* Choose another channel for out-band */
	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE) {
		if (pDfsParam->RadarDetected[RDD_DEDICATED_RX] == TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("RDD%d detect. Please switch to another outBand channel\n", RDD_DEDICATED_RX));
			ZeroWait_DFS_collision_report(pAd, RDD_DEDICATED_RX, pDfsParam->OutBandCh, pDfsParam->OutBandBw);

			if (pDfsParam->bDedicatedZeroWaitDefault) {

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
				P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
				/* Change channel state */
				switch (*ch_stat) {
				case DFS_INB_CH_SWITCH_CH:
					/* radar detected during in-band ch switch */
					*ch_stat = DFS_INB_CH_INIT;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ch_stat %d\n", __func__, *ch_stat));
					break;

				default:
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ch_stat %d\n", __func__, *ch_stat));
					MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
					RTMP_MLME_HANDLER(pAd);
					break;
				}
#else
				MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
#endif
			}

			pDfsParam->RadarDetected[RDD_DEDICATED_RX] = FALSE;
#ifdef DFS_CAC_R2
			if (first_wdev && IS_MAP_TURNKEY_ENABLE(pAd)) {
				wapp_send_radar_detect_notif(pAd, wdev, Channel_for_radar, 0);
				first_wdev = FALSE;
			}
#endif
			return;
		} else if ((pDfsParam->RadarDetected[ucRddIdx] == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("RDD%d detect. OutBand channel come back to InBand\n", ucRddIdx));

			pDfsParam->OrigInBandCh = pDfsParam->band_ch[ucRddIdx];
			pDfsParam->OrigInBandBw = pDfsParam->band_bw[ucRddIdx];
			pDfsParam->RadarHitReport = TRUE;
		}
	}
#endif

#ifdef CONFIG_RCSA_SUPPORT
	if (pDfsParam->RadarDetected[ucRddIdx] == TRUE)
		pDfsParam->fSendRCSA = TRUE;
#endif

	/* Keep BW info because the BW may be changed after selecting a new channel */
	KeepBw = pDfsParam->band_bw[band_idx];
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && (pDfsParam->RadarDetected[ucRddIdx] == TRUE)) {
		BwChannel = DfsV10SelectBestChannel(pAd, HcGetChannelByRf(pAd, RFIC_5GHZ), band_idx);
		/* AP BCN Update for ACS Case */
		if (IS_V10_AP_BCN_UPDATE_ENBL(pAd)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[%s] BCN Update\n", __func__));

			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_ACS_CSA_UPDATE, sizeof(UCHAR), &band_idx, 0);
			return;
		}

		/* W56 Channel Exhausted : Ap Down for 30 Minutes */
		if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[%s] AP Down %ld\n", __func__, pDfsParam->gV10W56TrgrApDownTime));
			SET_V10_W56_AP_DOWN(pAd, FALSE);

			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			return;
		}
			pDfsParam->PrimBand = RDD_BAND0;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			pDfsParam->band_ch[pDfsParam->DFSChHitBand] = BwChannel & 0xFF;
			pDfsParam->band_bw[pDfsParam->DFSChHitBand] = BwChannel >> 8;
			if (pAd->CommonCfg.dbdc_mode)
				pDfsParam->PrimCh = pDfsParam->band_ch[pDfsParam->DFSChHitBand];
			else
				pDfsParam->PrimCh = pDfsParam->band_ch[RDD_BAND0];
#else
			pDfsParam->band_ch[RDD_BAND0] = pDfsParam->PrimCh = BwChannel & 0xFF;
			pDfsParam->band_bw[RDD_BAND0] = BwChannel >> 8;
#endif
	} else {
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	WrapDfsSelectChannel(pAd, pDfsParam->DFSChHitBand);
#else
	WrapDfsSelectChannel(pAd, band_idx);
#endif
	}
#else
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	WrapDfsSelectChannel(pAd, pDfsParam->DFSChHitBand);
#else
	WrapDfsSelectChannel(pAd, band_idx);
#endif
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]PrimCh: %d, Band0Ch:%d, Band1Ch:%d\n",
			 __func__, pDfsParam->PrimCh, pDfsParam->band_ch[DBDC_BAND0], pDfsParam->band_ch[DBDC_BAND1]));

	/* Normal DFS uniform Ch */
	NextCh = pDfsParam->PrimCh;
	for (i = 0; i < DBDC_BAND_NUM; i++)
		RadarBandId[i] = FALSE;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		SetChInfo = 0;
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
#ifdef CONFIG_MAP_SUPPORT
#ifdef DFS_CAC_R2
		if (IS_MAP_TURNKEY_ENABLE(pAd))
			wdev->quick_ch_change = QUICK_CH_SWICH_DISABLE;
#endif
#endif
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != band_idx)
			continue;
		if (RadarBandId[band_idx] == TRUE)
			continue;
		else if (IsHcRadioCurStatOffByWdev(wdev) == FALSE) //cid WCNCR00224666
			RadarBandId[band_idx] = TRUE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));
#ifdef CONFIG_MAP_SUPPORT
/*On radar detect let AP stop start happen without apcli disconnect at AP stop*/
/*Link down only after sending the radar detect notification*/
		if (IS_MAP_ENABLE(pAd)) {
			int j;
			struct wifi_dev *sta_wdev = NULL;
			wdev->map_radar_detect = 1;
			wdev->map_radar_channel = wdev->channel;
			for (j = 0; j < MAX_APCLI_NUM; j++) {
				sta_wdev = &pAd->StaCfg[j].wdev;
#ifdef CONFIG_MAP_SUPPORT
#ifdef DFS_CAC_R2
				if (IS_MAP_TURNKEY_ENABLE(pAd))
					sta_wdev->quick_ch_change = QUICK_CH_SWICH_DISABLE;
#endif
#endif
				if (sta_wdev->channel == wdev->channel) {
					pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
				}
			}
		}
#endif
		/* Adjust Bw */
#ifdef BACKGROUND_SCAN_SUPPORT
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			if ((pDfsParam->band_bw[BAND0] == BW_8080) && (pDfsParam->OutBandBw == BW_80))
			{
				NextBw = BW_8080;
			}
			else
#endif
			{
				DfsAdjustBwSetting(wdev, pDfsParam->band_bw[band_idx], pDfsParam->OutBandBw);
				NextBw = pDfsParam->OutBandBw;
			}
		} else
#endif /* BACKGROUND_SCAN_SUPPORT */
		{
			DfsAdjustBwSetting(wdev, KeepBw, pDfsParam->band_bw[band_idx]);
			NextBw = pDfsParam->band_bw[band_idx];
		}

		if (dot11h_param->RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));

#if ((DFS_ZEROWAIT_DEFAULT_FLOW == 1) && defined(BACKGROUND_SCAN_SUPPORT))
			/* Stop RDD of dedicated RX before doing ch switch */
			DedicatedZeroWaitStop(pAd, FALSE);
#endif
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			SetChInfo |= (pDfsParam->DFSChHitBand << 16);
			if (NextBw == BW_8080) {
				wdev->vht_sec_80_channel = pDfsParam->band_ch[DBDC_BAND1];
				wlan_config_set_cen_ch_2(wdev, DfsPrimToCent(wdev->vht_sec_80_channel, BW_80));
			}
#else
			SetChInfo |= (band_idx << 16);
#endif
			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_RADAR_DETECTED_SW_CH, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		} else if (dot11h_param->RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[ucRddIdx] = FALSE;
			pDfsParam->RadarDetected[ucRddIdx] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));

			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			SetChInfo |= (pDfsParam->DFSChHitBand << 16);
			if (NextBw == BW_8080)
				wdev->vht_sec_80_channel = pDfsParam->band_ch[DBDC_BAND1];
#else
			SetChInfo |= (band_idx << 16);
#endif

			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
#ifdef TR181_SUPPORT
	/*increase radio channel change count due to radar detection*/
	/*todo: find rdev using api, instead of direct access*/
	ctrl->rdev[bandIdx].pRadioCtrl->DFSTriggeredChannelChangeCount++;
	ctrl->rdev[bandIdx].pRadioCtrl->TotalChannelChangeCount++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]channel changed for Band[%d]\n", __func__, bandIdx));
#endif /*TR181_SUPPORT*/
}
#endif

BOOLEAN DfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR rddidx, UCHAR bandidx)
{
	BOOLEAN RadarDetected = FALSE;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	BOOLEAN RadarChannel = FALSE;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;

	switch (rddidx) {
	case HW_RDD0:
	case HW_RDD1:
		/* Radar is detected by RDD0 or RDD1 */
		if ((pDfsParam->RadarDetected[rddidx] == FALSE) &&
			(pDfsParam->DfsChBand[rddidx]) &&
			(pDfsParam->Dot11_H[bandidx].RDMode != RD_SWITCHING_MODE)) {

			pDfsParam->RadarDetected[rddidx] = TRUE;
			RadarDetected = TRUE;
		}

		for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
			pMbss = &pAd->ApCfg.MBSSID[BssIdx];
			wdev = &pMbss->wdev;
			if (wdev->pHObj == NULL)
				continue;

			cfg = (struct wlan_config *)wdev->wpf_cfg;
			if (cfg == NULL)
				continue;

			if (HcGetBandByWdev(wdev) != bandidx)
				continue;

			if (cfg->ht_conf.ht_bw == HT_BW_20)
				phy_bw = BW_20;
			else if (cfg->ht_conf.ht_bw == HT_BW_40) {
				if (cfg->vht_conf.vht_bw == VHT_BW_2040)
					phy_bw = BW_40;
				else if (cfg->vht_conf.vht_bw == VHT_BW_80)
					phy_bw = BW_80;
				else if (cfg->vht_conf.vht_bw == VHT_BW_160)
					phy_bw = BW_160;
				else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
					phy_bw = BW_8080;
				else
					;
			}

			if (DfsRadarChannelCheck(pAd, wdev, cfg->phy_conf.cen_ch_2, phy_bw)) {
				RadarChannel = TRUE;
				break;
			}
		}

		if (RadarChannel == FALSE) {
			RadarDetected = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s][RDM]:  No wdev work on Radar Channel!\n", __func__));
		}

		break;

#if (RDD_2_SUPPORTED == 1)
	case HW_RDD2:
		/* Radar is detected by dedicated RX */
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE &&
			(pDfsParam->RadarDetected[rddidx] == FALSE) &&
			pDfsParam->DfsChBand[rddidx])) {
			pDfsParam->RadarDetected[rddidx] = TRUE;
			RadarDetected = TRUE;
		}
		break;
#endif

	default:
		break;
	}

	return RadarDetected;
}

VOID WrapDfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd, UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

#ifdef DOT11_VHT_AC
	UCHAR cen_ch_2 = 0;
#endif /*DOT11_VHT_AC*/

	DfsSelectChannel(pAd, pDfsParam, band_idx);

#ifdef DOT11_VHT_AC

	if (pDfsParam->band_bw[band_idx] == BW_8080) {
		if (pDfsParam->PrimBand == RDD_BAND0) {
			cen_ch_2
				= vht_cent_ch_freq(pDfsParam->band_ch[RDD_BAND1], VHT_BW_8080, CMD_CH_BAND_5G);/* Central channel 2 */
		} else {
			cen_ch_2
				= vht_cent_ch_freq(pDfsParam->band_ch[RDD_BAND0], VHT_BW_8080, CMD_CH_BAND_5G);/* Central channel 2 */
		}

		wlan_config_set_cen_ch_2_all(&pAd->wpf, cen_ch_2);
	}

#endif
}

VOID DfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR band_idx)
{
	UCHAR tempCh = 0;
	UCHAR idx;
#if ((DFS_ZEROWAIT_SUPPORT_8080 == 1) && defined(BACKGROUND_SCAN_SUPPORT))
	CHANNEL_CTRL *pChCtrl;
#endif

	for (idx = 0; idx < RDD_BAND_NUM; idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s][RDM]: RadarDetected[%d]=%d, pDfsParam->DfsChBand[%d]=%d\n",
				 __func__,
				 idx,
				 pDfsParam->RadarDetected[idx],
				 idx,
				 pDfsParam->DfsChBand[idx]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s][RDM]: pDfsParam->band_bw[%d]=%d\n",
				 __func__,
				 idx,
				 pDfsParam->band_bw[idx]));

	}

	if (band_idx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Invalid band_idx(%d)\n", __func__, band_idx));
		return;
	}

	if ((pDfsParam->band_bw[band_idx] == BW_8080) && (pAd->CommonCfg.dbdc_mode == TRUE)) {
		if (IS_MT7615(pAd)) {
			if (pDfsParam->band_ch[DBDC_BAND0] < pDfsParam->band_ch[DBDC_BAND1]) {
				if (pDfsParam->RadarDetected[HW_RDD0] && pDfsParam->DfsChBand[HW_RDD0]) {
					pDfsParam->band_ch[DBDC_BAND0] = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->band_ch[DBDC_BAND1], band_idx);
				}
				if (pDfsParam->RadarDetected[HW_RDD1] && pDfsParam->DfsChBand[HW_RDD1]) {
					pDfsParam->band_ch[DBDC_BAND1] = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->band_ch[DBDC_BAND0], band_idx);
				}
			}
		} else {
			if (pDfsParam->RadarDetected[HW_RDD0] && pDfsParam->DfsChBand[HW_RDD0]) {
				pDfsParam->band_ch[DBDC_BAND0] = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->band_ch[DBDC_BAND1], band_idx);
			}
			if (pDfsParam->RadarDetected[HW_RDD1] && pDfsParam->DfsChBand[HW_RDD1]) {
				/* if single band 80+80 (RDD0+RDD1) is used, channel list is only generated @band0 */
				pDfsParam->band_ch[DBDC_BAND1] = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->band_ch[DBDC_BAND0], DBDC_BAND0);
			}
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s][RDM]: 80+80MHz band, selected is %d, %d\n",
			__func__,
			pDfsParam->band_ch[DBDC_BAND0],
			pDfsParam->band_ch[DBDC_BAND1]));

		if (pDfsParam->PrimBand == RDD_BAND0)
			pDfsParam->PrimCh = pDfsParam->band_ch[DBDC_BAND0];
		else
			pDfsParam->PrimCh = pDfsParam->band_ch[DBDC_BAND1];

		if (IS_MT7615(pAd) &&
			(pDfsParam->band_ch[DBDC_BAND1] < pDfsParam->band_ch[DBDC_BAND0])) {
			tempCh = pDfsParam->band_ch[DBDC_BAND1];
			pDfsParam->band_ch[DBDC_BAND1] = pDfsParam->band_ch[DBDC_BAND0];
			pDfsParam->band_ch[DBDC_BAND0] = tempCh;
		}

		if (pDfsParam->PrimCh == pDfsParam->band_ch[DBDC_BAND0])
			pDfsParam->PrimBand = RDD_BAND0;
		else
			pDfsParam->PrimBand = RDD_BAND1;

		return;
	} else if (pDfsParam->band_bw[band_idx] == BW_160) {
		if ((pDfsParam->RadarDetected[HW_RDD0] && pDfsParam->DfsChBand[HW_RDD0]) ||
			(pDfsParam->RadarDetected[HW_RDD1] && pDfsParam->DfsChBand[HW_RDD1])) {
			pDfsParam->band_ch[DBDC_BAND0] = WrapDfsRandomSelectChannel(pAd, FALSE, 0, DBDC_BAND0);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s][RDM]: BW_160, Single band, selected is %d\n",
				__func__,
				pDfsParam->band_ch[DBDC_BAND0]));
		}
	} else {
		if (pDfsParam->RadarDetected[band_idx] && pDfsParam->DfsChBand[band_idx]) {
#ifdef BACKGROUND_SCAN_SUPPORT
			if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) &&
				GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
				tempCh = WrapDfsRandomSelectChannel(pAd, FALSE, 0, band_idx);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("[%s][RDM]: tempCh selected is %d\n",
					__func__,
					tempCh));

				if (RadarChannelCheck(pAd, tempCh)) {
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
					P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
					pChCtrl = DfsGetChCtrl(
						pAd,
						pDfsParam,
						pDfsParam->band_bw[band_idx],
						band_idx);
#else
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
#endif
					switch (*ch_stat) {
					case DFS_INB_DFS_OUTB_CH_CAC_DONE:
						/* zero-wait CAC of out-band is ended */
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("[%s][RDM]: out-band CAC is ended, ch_stat %d\n",
							__func__,
							*ch_stat));

						/* If DFS channel is selected randomly by SynA, SynA will use the DFS channel of SynB*/
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
						pDfsParam->band_ch[band_idx] = DfsGetNonDfsDefaultCh(
													   pDfsParam,
													   band_idx);
#else
						pDfsParam->band_ch[band_idx] = pChCtrl->ChList[0].Channel;
#endif
						*ch_stat = DFS_INB_DFS_RADAR_OUTB_CAC_DONE;

						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("RDD[%d] detect. OutBand channel %d will be set to InBand\n", band_idx, pDfsParam->OutBandCh));
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("[%s][RDM]: ch_stat %d\n",
							__func__,
							*ch_stat));
						break;

					case DFS_INB_DFS_OUTB_CH_CAC:
						/* radar is detected on in-band ch and out-band CAC is not ended */
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("[%s][RDM]: out-band CAC is not ended, ch_stat %d\n",
							__func__,
							*ch_stat));
						*ch_stat = DFS_INB_CH_INIT;

						pDfsParam->band_ch[band_idx] = pDfsParam->OutBandCh;
						break;

					default:
						if (pDfsParam->bDedicatedZeroWaitDefault == FALSE) {
							pDfsParam->band_ch[band_idx] = pDfsParam->OutBandCh;
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("%s, RDD[%d] detect. zw DFS is not enabled\n", __func__, band_idx));
						}

						break;
					}
#else
					/* If DFS channel is selected randomly by SynA, SynA will use the DFS channel of SynB*/
					pDfsParam->band_ch[band_idx] = pDfsParam->OutBandCh;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("RDD[%d] detect. OutBand channel come back to InBand\n", band_idx));
#endif
				} else {
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
					P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;
					*ch_stat = DFS_INB_CH_INIT;
					pAd->CommonCfg.DfsParameter.OutBandCh = 0;
					mtRddControl(pAd, RDD_STOP, RDD_DEDICATED_RX, 0, 0);
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */

					pDfsParam->band_ch[band_idx] = tempCh;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("RDD[%d] detect. InBand channel is switched to another non-DFS channel randomly\n",
						band_idx));
				}
			}
			else
#endif
				pDfsParam->band_ch[band_idx] = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->band_ch[(UINT_8)(DBDC_BAND1 - band_idx)], band_idx);

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s][RDM]: band index: %d, selected is %d\n",
				__func__,
				band_idx,
				pDfsParam->band_ch[band_idx]));
		}
	}
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if ((pAd->CommonCfg.dbdc_mode == FALSE) && (pDfsParam->band_bw[band_idx] == BW_8080)) {
		pDfsParam->PrimCh = pDfsParam->band_ch[BAND0];
		pDfsParam->PrimBand = BAND0;
	} else {
		pDfsParam->PrimCh = pDfsParam->band_ch[band_idx];
		pDfsParam->PrimBand = band_idx;
	}
#else
	pDfsParam->PrimCh = pDfsParam->band_ch[band_idx];
	pDfsParam->PrimBand = band_idx;
#endif

}

UCHAR WrapDfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh, UCHAR avoidedCh, UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR Channel;
	if (pDfsParam->targetCh != 0) {
		if ((pDfsParam->targetCh != pDfsParam->band_ch[band_idx])
		|| (pDfsParam->targetBw != pDfsParam->band_bw[band_idx])) {
			pDfsParam->band_bw[band_idx] = pDfsParam->targetBw;
			Channel = pDfsParam->targetCh;
			return Channel;
		} else {
			pDfsParam->targetCh = 0;
			pDfsParam->targetBw = 0;
			pDfsParam->targetCacValue = 0;
		}
	}

    bSkipDfsCh = TRUE; //hongchen+, skip dfs channel
	return DfsRandomSelectChannel(pAd, pDfsParam, bSkipDfsCh, avoidedCh, band_idx);
}

UCHAR DfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	IN BOOLEAN bSkipDfsCh,
	IN UCHAR avoidedCh,
	IN UCHAR band_idx)
{
	UINT_8 i, cnt, ch;
	UINT_8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	PCHANNEL_CTRL pChCtrl = NULL;

	cnt = 0;

	pChCtrl = DfsGetChCtrl(pAd, pDfsParam, pDfsParam->band_bw[band_idx], band_idx);

	if ((pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) || (pChCtrl->ChListNum <= 0)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: incorrect ChListNum(%d)\n", __func__, pChCtrl->ChListNum));
		return FALSE;
	}

	if (!(pDfsParam->bIEEE80211H)) {
#ifdef WIFI_MD_COEX_SUPPORT
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (!IsChannelSafe(pAd, pChCtrl->ChList[i].Channel)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("The channel %d is in unsafe channel list!! %s\n ", pChCtrl->ChList[i].Channel, __func__));
				continue;
			}
			TempChList[cnt++] = pChCtrl->ChList[i].Channel;
		}
		if (cnt)
			ch = TempChList[(UINT_8)(RandomByte(pAd) % cnt)];
		else {
			ch =  pChCtrl->ChList[0].Channel;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("No available channel to use, return the first ch for use\n"));
		}
		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
		return ch;
#else
		ch = pChCtrl->ChList[(UINT_8)(RandomByte(pAd) % pChCtrl->ChListNum)].Channel;

		if (ch == 0)
			ch = pChCtrl->ChList[0].Channel;

		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
		return ch;
#endif
	}

	for (i = 0; i < pChCtrl->ChListNum; i++) {

		if (pChCtrl->ChList[i].NonOccupancy)
			continue;

#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[i].Channel) == TRUE)
			continue;
#endif

		if (!IS_CH_ABAND(pChCtrl->ChList[i].Channel))
			continue;

		/* Skip DFS channel for DFS zero wait using case */
		if (bSkipDfsCh) {
			if (RadarChannelCheck(pAd, pChCtrl->ChList[i].Channel))
				continue;
		}

		if (ByPassChannelByBw(pChCtrl->ChList[i].Channel, pDfsParam->band_bw[band_idx], pChCtrl))
			continue;

		/* BW8080 */
		if ((avoidedCh != 0) &&
			(pDfsParam->band_bw[band_idx] == BW_8080) &&
			DfsPrimToCent(pChCtrl->ChList[i].Channel, BW_80) == DfsPrimToCent(avoidedCh, BW_80))
			continue;

		/* 5G + 5G case */
		if ((avoidedCh != 0) &&
			(pAd->CommonCfg.dbdc_mode == TRUE) &&
			DfsPrimToCent(pChCtrl->ChList[i].Channel, pDfsParam->band_bw[band_idx]) == DfsPrimToCent(avoidedCh, pDfsParam->band_bw[band_idx]))
			continue;

		if (!DfsDedicatedCheckChBwValid(pAd, pChCtrl->ChList[i].Channel, pDfsParam->band_bw[band_idx], band_idx))
			continue;

#ifdef WIFI_MD_COEX_SUPPORT
		if (!IsChannelSafe(pAd, pChCtrl->ChList[i].Channel)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("The channel %d is in unsafe channel list!! %s\n ", pChCtrl->ChList[i].Channel, __func__));
			continue;
		}
#endif

		/* Store available channel to temp list */
		TempChList[cnt++] = pChCtrl->ChList[i].Channel;
	}

	if (cnt)
		ch = TempChList[(UINT_8)(RandomByte(pAd) % cnt)];
	else {
		USHORT MinTime = 0xFFFF;
		UINT_16 BwChannel = 0;

		ch = 0;
		pDfsParam->bNoAvailableCh[band_idx] = FALSE;

		if (pDfsParam->band_bw[band_idx] != BW_8080) {
			BwChannel = DfsBwChQueryByDefault(pAd, BW_160, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, FALSE, band_idx);
			ch = BwChannel & 0xff;
			pDfsParam->band_bw[band_idx] = BwChannel>>8;

			if (ch == 0) { /* No available channel to use */
				pDfsParam->bNoAvailableCh[band_idx] = TRUE;

				for (i = 0; i < pChCtrl->ChListNum; i++) {
					if (pChCtrl->ChList[i].NonOccupancy < MinTime) {
						if (!IS_CH_ABAND(pChCtrl->ChList[i].Channel))
							continue;
						if (ByPassChannelByBw(pChCtrl->ChList[i].Channel, pDfsParam->band_bw[band_idx], pChCtrl))
							continue;
						if ((avoidedCh != 0)
							&& DfsPrimToCent(pChCtrl->ChList[i].Channel, BW_80) == DfsPrimToCent(avoidedCh, BW_80))
							continue;
						MinTime = pChCtrl->ChList[i].NonOccupancy;
						ch = pChCtrl->ChList[i].Channel;
					}
				}
			}
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s][RDM]:When no available Ch, new pDfsParam->band_bw[%d]: %d\n",
				__func__,
				band_idx,
				pDfsParam->band_bw[band_idx]));

		} else {
			pDfsParam->bNoAvailableCh[band_idx] = TRUE;

			i = RandomByte(pAd) % (pChCtrl->ChListNum);
			while (ByPassChannelByBw(pChCtrl->ChList[i].Channel, BW_8080, pChCtrl))
				i = RandomByte(pAd) % (pChCtrl->ChListNum);

			ch = pChCtrl->ChList[i].Channel;
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]:Currently no immediately available Channel. Choose Ch %d\n",
				 __func__, ch));
	}

	return ch;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT_8 DFS_V10_W52_LIST[V10_W52_SIZE] = {36, 40, 44, 48};
UINT_8 DFS_V10_W53_LIST[V10_W53_SIZE] = {52, 56, 60, 64};
UINT_8 DFS_V10_W56_VHT80_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT80_C_SIZE] = {100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
UINT_8 DFS_V10_W56_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT80_C_SIZE] = {100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144};
UINT_8 DFS_V10_W56_VHT80_LISTA[V10_W56_VHT80_A_SIZE] = {100, 104, 108, 112};
UINT_8 DFS_V10_W56_VHT80_LISTB[V10_W56_VHT80_B_SIZE] = {116, 120, 124, 128};
UINT_8 DFS_V10_W56_VHT80_LISTC[V10_W56_VHT80_C_SIZE] = {132, 136, 140, 144};
UINT_8 DFS_V10_W56_VHT20_LIST[V10_W56_VHT20_SIZE] = {132, 136, 140};

UINT_8 DfsV10FindNonNopChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth)
{
	UCHAR ChIdx = 0;
	UINT_8 channel = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if ((chGrp == W53 || chGrp == W56) && grpWidth && wdev) {
		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			channel = (chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) : (DFS_V10_W56_LIST[ChIdx]);
			if (CheckNonOccupancyChannel(pAd, wdev, channel))
				return channel;
		}
	}

	return 0;
}

UINT_8 DfsV10W56FindMaxNopDuration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ChIdx = 0;
	USHORT channelNopTime = 0;
	UCHAR upperBoundCh = 0;
	PCHANNEL_CTRL pChCtrl = NULL;
	UCHAR band_idx;

	if (pAd->CommonCfg.bCh144Enabled)
		upperBoundCh = 144;
	else
		upperBoundCh = 140;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
			if (pChCtrl->ChList[ChIdx].Channel >= 100 &&
				pChCtrl->ChList[ChIdx].Channel <= upperBoundCh) {
				if (channelNopTime < pChCtrl->ChList[ChIdx].NonOccupancy)
					channelNopTime = pChCtrl->ChList[ChIdx].NonOccupancy;
			}
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] MAX NOP %d\n", __func__, channelNopTime));
	return channelNopTime;
}

BOOLEAN DfsV10CheckGrpChnlLeft(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth,
	IN UCHAR		 band_idx)
{
	UCHAR ChIdx = 0, ChCnt = 0;
	UCHAR idx, BandIdx;
	BOOLEAN status = FALSE;
	struct wifi_dev *wdev;

	if (chGrp == W53 || chGrp == W56 || chGrp == W56_UAB || chGrp == W56_UC) {

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			if (CheckNonOccupancyChannel(pAd, wdev,
				((chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) :
				((wlan_config_get_vht_bw(wdev) == VHT_BW_80) ?
					(DFS_V10_W56_VHT80_LIST[ChIdx]) : ((chGrp == W56) ?
					(DFS_V10_W56_LIST[ChIdx]) : ((pAd->CommonCfg.bCh144Enabled == FALSE) ?
					DFS_V10_W56_VHT20_LIST[ChIdx] : 0)))))) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] ChCnt++\n", __func__));
				ChCnt++;
			}
		}
	}

	if (ChCnt)
		status =  TRUE;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]ChGrp %d VHT20 ChCnt %d Empty\n", __func__, chGrp, ChCnt));
		status = FALSE;
	}
	return status;
}

BOOLEAN DfsV10CheckChnlGrpW52(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W52_SIZE && Channel != DFS_V10_W52_LIST[i])
		i++;

	if (i < V10_W52_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW53(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W53_SIZE && Channel != DFS_V10_W53_LIST[i])
		i++;

	if (i < V10_W53_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UA(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_A_SIZE && Channel != DFS_V10_W56_VHT80_LISTA[i])
		i++;

	if (i < V10_W56_VHT80_A_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UB(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_B_SIZE && Channel != DFS_V10_W56_VHT80_LISTB[i])
		i++;

	if (i < V10_W56_VHT80_B_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UC(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	UCHAR i = 0;

	if (pAd->CommonCfg.bCh144Enabled == FALSE) {
		while (i < V10_W56_VHT20_SIZE && Channel != DFS_V10_W56_VHT20_LIST[i])
			i++;

		if (i < V10_W56_VHT20_SIZE)
			return TRUE;
		else
			return FALSE;

	} else {
		while (i < V10_W56_VHT80_C_SIZE && Channel != DFS_V10_W56_VHT80_LISTC[i])
			i++;
		if (i < V10_W56_VHT80_C_SIZE)
			return TRUE;
		else
			return FALSE;
	}
}

BOOLEAN DfsV10CheckW56Grp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	BOOLEAN isW56 = FALSE;

	if (DfsV10CheckChnlGrpW56UA(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UB(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UC(pAd, channel))
		isW56 = TRUE;
	else
		isW56 = FALSE;

	return isW56;
}

UCHAR DfsV10CheckChnlGrp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	if (DfsV10CheckChnlGrpW52(Channel))
		return W52;
	else if (DfsV10CheckChnlGrpW53(Channel))
		return W53;
	else if (DfsV10CheckChnlGrpW56UA(Channel))
		return W56_UA;
	else if (DfsV10CheckChnlGrpW56UB(Channel))
		return W56_UB;
	else if (DfsV10CheckChnlGrpW56UC(pAd, Channel))
		return W56_UC;
	else
		return NA_GRP;
}

BOOLEAN DfsV10W56APDownStart(
	IN PRTMP_ADAPTER pAd,
	IN PAUTO_CH_CTRL pAutoChCtrl,
	IN ULONG	     V10W56TrgrApDownTime,
	IN UCHAR	     band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx = band_idx;

	if (pAutoChCtrl == NULL)
		return FALSE;

	/* Disable AP 30 Minutes */
	pDfsParam->gV10W56TrgrApDownTime = V10W56TrgrApDownTime;
	SET_V10_W56_AP_DOWN(pAd, TRUE);

	/* ReEnable Boot ACS */
	SET_V10_BOOTACS_INVALID(pAd, FALSE);

	SET_V10_W56_GRP_VALID(pAd, TRUE);

	MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_ENBL, sizeof(UCHAR), &BandIdx, 0);

	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;

	return TRUE;
}

USHORT DfsV10SelectBestChannel(/*Select the Channel from Rank List by ACS*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oldChannel,
	IN UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	UCHAR idx;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* New Channel Identification */
	if (pAd->ApCfg.bAutoChannelAtBootup[BandIdx]) {
		/* Pick AutoCh2 Update from List */
		BwChannel = SelectBestV10Chnl_From_List(pAd, BandIdx);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] ACS Disable\n", __func__));
		/* Push CSA BCN Update out of interrupt context */
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, TRUE);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s Select channel = %d from V10 list\n", __func__, BwChannel));

	BwChannel |= (pDfsParam->band_bw[BandIdx] << 8);
	return BwChannel;
}

/* Weighing Factor for W56>W52>W53 Priority */
VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR channelIdx = 0, chnlGrp = 0;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* ACS Disable: Weighing Factor Not Required */
	if (!pAd->ApCfg.bAutoChannelAtBootup[BandIdx])
		return;

	for (channelIdx = 0; channelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; channelIdx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel %3d : Busy Time = %6u\n",
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel,
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx]));

	chnlGrp = DfsV10CheckChnlGrp(pAd, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel);

	if (chnlGrp == W52) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W52;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W52 * 10);
	} else if (chnlGrp == W53) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W53;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W53 * 10);
	} else if (chnlGrp >= W56_UA && chnlGrp <= W56_UC) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W56;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W56 * 10);
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("[%s] Error Group Ch%d", __func__, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel));
	}
}

VOID DfsV10W56APDownTimeCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)
		&& pDfsParam->gV10W56TrgrApDownTime > 0) {
		pDfsParam->gV10W56TrgrApDownTime--;
		if (!pDfsParam->gV10W56TrgrApDownTime) {
			/* Bring Up AP */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] AP Down Pass\n", __func__));
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_FINISH, 0, NULL, 0);
		}
	}
}

VOID DfsV10W56APDownEnbl(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem)
{
	struct DOT11_H *pDot11hTest = NULL;
	struct wifi_dev *wdev;
	UCHAR BandIdx, idx, band_idx;
	BSS_STRUCT *pMbss;

	NdisMoveMemory(&band_idx, pElem->Msg, pElem->MsgLen);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pMbss = &pAd->ApCfg.MBSSID[idx];

	pDot11hTest = &pAd->Dot11_H[BandIdx];

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s] W56 Down Time Start %d\n", __func__, IS_V10_W56_AP_DOWN_ENBLE(pAd)));

	if (IS_SUPPORT_V10_DFS(pAd) && (!IS_V10_W56_AP_DOWN_ENBLE(pAd) || IS_V10_APINTF_DOWN(pAd))) {
		pDot11hTest->RDCount = 0;
		MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
		pDot11hTest->RDMode = RD_NORMAL_MODE;

		SET_V10_W56_AP_DOWN(pAd, TRUE);
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}

VOID DfsV10W56APDownPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] W56 Down Time Pass\n", __func__));

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		SET_V10_W56_AP_DOWN(pAd, FALSE);
		SET_V10_APINTF_DOWN(pAd, FALSE);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}

VOID DfsV10APBcnUpdate(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	UCHAR NextCh = 0, CurCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR BssIdx;
	UCHAR idx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
	UCHAR GrpSize;
	UCHAR band_idx;

	NdisMoveMemory(&band_idx, pElem->Msg, pElem->MsgLen);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		if (band_idx == BandIdx)
			break;
	}

	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* Backup Original channel as we are doing off Channel scan */
	CurCh = wdev->channel;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]CurCh %d\n", __func__, CurCh));

	if (IS_V10_AP_BCN_UPDATE_ENBL(pAd))
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, FALSE);

	ApAutoChannelSkipListBuild(pAd, wdev);
	if (DfsV10CheckW56Grp(pAd, wdev->channel)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]group 56\n", __func__));
		if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]VHT_BW_2040\n", __func__));
			if (pAd->CommonCfg.bCh144Enabled)
				GrpSize = V10_W56_SIZE;
			else
				GrpSize = V10_W56_SIZE - 1;

			if ((DfsV10CheckGrpChnlLeft(pAd, W56, GrpSize, BandIdx) == FALSE)
				|| (IS_V10_W56_VHT80_SWITCHED(pAd) &&
				DfsV10CheckGrpChnlLeft(pAd, W56_UC, V10_W56_VHT20_SIZE, BandIdx) == FALSE)) {
				if (IS_V10_W56_VHT80_SWITCHED(pAd)) {
					/* VHT 20 -> VHT 80 */
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] BW Switched to VHT80\n", __func__));
					wlan_config_set_ht_bw(wdev, HT_BW_40);
					wlan_config_set_vht_bw(wdev, VHT_BW_80);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_40;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_40;
#else
					pAd->CommonCfg.mcastphymode.field.BW = HT_BW_40;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
					SET_V10_W56_VHT80_SWITCH(pAd, FALSE);
				} else
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] BW Not Switched to VHT80 %d\n", __func__,
						IS_V10_W56_VHT80_SWITCHED(pAd)));

				/* No Channel Left in W53/ W56_UC VHT20 Case */
				if (DfsV10W56APDownStart(pAd, pAutoChCtrl, V10_W56_APDOWN_TIME, BandIdx))
					goto W56APDOWN;
				else
					ASSERT(BwChannel);
			}
		} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {

			if (pAd->CommonCfg.bCh144Enabled)
				GrpSize = V10_W56_VHT80_SIZE;
			else
				GrpSize = V10_W56_VHT80_SIZE - V10_W56_VHT80_C_SIZE;

			if (DfsV10CheckGrpChnlLeft(pAd, W56_UAB, GrpSize, BandIdx) == FALSE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]56 Channel Left\n", __func__));

				if (pAd->CommonCfg.bCh144Enabled) {
					if (DfsV10W56APDownStart(pAd, pAutoChCtrl, V10_W56_APDOWN_TIME, BandIdx))
						goto W56APDOWN;
				} else {
				/* VHT80 -> VHT20 */
					wlan_config_set_ht_bw(wdev, HT_BW_20);
					wlan_config_set_vht_bw(wdev, VHT_BW_2040);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("[%s] BW Switched to VHT20\n", __func__));
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#else
					pAd->CommonCfg.mcastphymode.field.BW = HT_BW_20;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
					SET_V10_W56_VHT80_SWITCH(pAd, TRUE);
					ApAutoChannelSkipListBuild(pAd, wdev);
				}
			}
		}
	}

	/* Perform Off Channel Scan to find channel */
	SET_V10_OFF_CHNL_TIME(pAd, V10_BGND_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	BwChannel = MTAPAutoSelectChannel(pAd, wdev, ChannelAlgBusyTime, TRUE);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]BwChannel = %d\n", __func__, BwChannel));
	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;

	/* Return to Original RADAR Hit Channel */
	/* Update channel of wdev as new channel */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]calling AutoChSelUpdateChannel\n", __func__));
	AutoChSelUpdateChannel(pAd, CurCh, TRUE, wdev);

	/* Update primay channel */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s]calling wlan_operate_set_prim_ch wdev->channel = %d\n", __func__, wdev->channel));
	wlan_operate_set_prim_ch(wdev, wdev->channel);

W56APDOWN:
	/* W56 Channel Exhausted : Ap Down for 30 Minutes */
	if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] AP Down %ld\n", __func__, pDfsParam->gV10W56TrgrApDownTime));
		SET_V10_W56_AP_DOWN(pAd, FALSE);

		pDfsParam->DfsChBand[0] = FALSE;
		pDfsParam->DfsChBand[1] = FALSE;
		pDfsParam->RadarDetected[0] = FALSE;
		pDfsParam->RadarDetected[1] = FALSE;
		return;
	}

	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->band_ch[BandIdx] = pDfsParam->PrimCh = BwChannel & 0xFF;
	pDfsParam->band_bw[BandIdx] = BwChannel >> 8;

	NextCh = pDfsParam->PrimCh;
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != BandIdx)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));
		/*Adjust Bw*/
#ifdef BACKGROUND_SCAN_SUPPORT
#ifdef ONDEMAND_DFS
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) &&
			GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#else
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#endif
			DfsAdjustBwSetting(wdev, pDfsParam->band_bw[BandIdx], pDfsParam->OutBandBw);
			NextBw = pDfsParam->OutBandBw;
		} else {
#else
		{
#endif /* BACKGROUND_SCAN_SUPPORT */

			DfsAdjustBwSetting(wdev, KeepBw, pDfsParam->band_bw[BandIdx]);
			NextBw = pDfsParam->band_bw[BandIdx];
		}

		if (pDfsParam->Dot11_H[BandIdx].RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (pDfsParam->Dot11_H[BandIdx].RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\x1b[1;33m [%s]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);
			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif


USHORT DfsBwChQueryByDefault(/*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Bw,
	IN PDFS_PARAM pDfsParam,
	IN UCHAR level,
	IN BOOLEAN bDefaultSelect,
	IN BOOLEAN SkipNonDfsCh,
	IN UCHAR band_idx)
{
	USHORT BwChannel = 0;
	UINT_8 ch = 0;
	UINT_8 ch_idx, SelectIdx;
	UINT_8 AvailableChCnt = 0;
	BOOLEAN nonWetherBandChExist = FALSE;
	BOOLEAN isSelectWetherBandCh = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = DfsGetChCtrl(pAd, pDfsParam, pDfsParam->band_bw[band_idx], band_idx);

	if ((pChCtrl->ChListNum > MAX_NUM_OF_CHANNELS) || (pChCtrl->ChListNum <= 0)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: incorrect ChListNum(%d)\n", __func__, pChCtrl->ChListNum));
		return FALSE;
	}

	if (pDfsParam->bIEEE80211H == FALSE) {
		ch = pChCtrl->ChList[(UINT_8)(RandomByte(pAd)%pChCtrl->ChListNum)].Channel;
		BwChannel |= ch;
		BwChannel |= (Bw << 8);
		return BwChannel;
	}

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {

#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[ch_idx].Channel) == TRUE)
			continue;
#endif
		if ((SkipNonDfsCh == TRUE) && (!RadarChannelCheck(pAd, pChCtrl->ChList[ch_idx].Channel)))
			continue;
		if (!IS_CH_ABAND(pChCtrl->ChList[ch_idx].Channel))
			continue;

		if (ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, Bw, pChCtrl))
			continue;

		if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0)
		 && (pChCtrl->ChList[ch_idx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)
		)
			continue;

		if (DfsCheckBwGroupAllAvailable(ch_idx, Bw, pAd, band_idx) == FALSE)
			continue;

		if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, pDfsParam->band_bw[band_idx]) ==
			DfsPrimToCent(pDfsParam->band_ch[band_idx], pDfsParam->band_bw[band_idx]))
			continue;

		if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, Bw) ==
			DfsPrimToCent(pDfsParam->band_ch[band_idx], Bw))
			continue;

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (pDfsParam->band_bw[band_idx] == BW_8080)
		{
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, Bw) ==
				DfsPrimToCent(pDfsParam->band_ch[BAND1], Bw))
				continue;
		}
#endif

		if ((level == DFS_BW_CH_QUERY_LEVEL1)
		&& ((pChCtrl->ChList[ch_idx].NonOccupancy == 0) && (pChCtrl->ChList[ch_idx].NOPClrCnt == 0)))
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;

		if ((level == DFS_BW_CH_QUERY_LEVEL2)
		&& (pChCtrl->ChList[ch_idx].NonOccupancy == 0))
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;

		if (AvailableChCnt >= DFS_AVAILABLE_LIST_CH_NUM)
			break;
	}

	if (AvailableChCnt > 0) {

		for (ch_idx = 0; ch_idx < AvailableChCnt; ch_idx++) {
			SelectIdx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][ch_idx];
			if ((pAd->CommonCfg.RDDurRegion != CE)
				|| !DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0)) {
				nonWetherBandChExist = TRUE;
				break;
			}
		}
		/*randomly select a ch for this BW*/
		SelectIdx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][(UINT_8)(RandomByte(pAd)%AvailableChCnt)];

		if ((pAd->CommonCfg.RDDurRegion == CE)
			&& DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0))
			isSelectWetherBandCh = TRUE;
		while (isSelectWetherBandCh == TRUE && nonWetherBandChExist == TRUE) {
			SelectIdx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][(UINT_8)(RandomByte(pAd)%AvailableChCnt)];

			if ((pAd->CommonCfg.RDDurRegion == CE)
				&& DfsCacRestrictBand(pAd, Bw, pChCtrl->ChList[SelectIdx].Channel, 0))
				isSelectWetherBandCh = TRUE;
			else
				isSelectWetherBandCh = FALSE;
		}
		BwChannel |= pChCtrl->ChList[SelectIdx].Channel;
		BwChannel |= (Bw << 8);
		return BwChannel;
	} else if (level == DFS_BW_CH_QUERY_LEVEL1)
		BwChannel = DfsBwChQueryByDefault(pAd, Bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL2, bDefaultSelect, SkipNonDfsCh, band_idx);

	else if (level == DFS_BW_CH_QUERY_LEVEL2) {
		if (Bw > BW_20) {
			/*Clear NOP of the current BW*/
			for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
				if ((pChCtrl->ChList[ch_idx].NonOccupancy != 0) && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)) {
					pChCtrl->ChList[ch_idx].NOPSaveForClear = pChCtrl->ChList[ch_idx].NonOccupancy;
					pChCtrl->ChList[ch_idx].NonOccupancy = 0;
					pChCtrl->ChList[ch_idx].NOPClrCnt++;
				}
			}
			/*reduce BW*/
			BwChannel = DfsBwChQueryByDefault(pAd, Bw - 1, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, bDefaultSelect, SkipNonDfsCh, band_idx);
		} else
			;/*Will return BwChannel = 0*/
	} else
		;
	return BwChannel;

}

VOID DfsBwChQueryAllList(/*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Bw,
	IN PDFS_PARAM pDfsParam,
	IN BOOLEAN SkipWorkingCh,
	IN UCHAR band_idx)
{
	UINT_8 ch_idx;
	UINT_8 AvailableChCnt = 0;
	PCHANNEL_CTRL pChCtrl = NULL;

	if (pDfsParam->bIEEE80211H == FALSE)
		return ;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
#ifdef CONFIG_AP_SUPPORT
		if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[ch_idx].Channel) == TRUE)
			continue;
#endif
		if (!IS_CH_ABAND(pChCtrl->ChList[ch_idx].Channel))
			continue;

		if (ByPassChannelByBw(pChCtrl->ChList[ch_idx].Channel, Bw, pChCtrl))
			continue;

		if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0)
		 && (pChCtrl->ChList[ch_idx].NOPClrCnt != 0)
		 && (pChCtrl->ChList[ch_idx].NOPSetByBw <= Bw)
		)
			continue;

		if (DfsCheckBwGroupAllAvailable(ch_idx, Bw, pAd, band_idx) == FALSE)
			continue;

		if (SkipWorkingCh == TRUE) {
			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, pDfsParam->band_bw[band_idx]) ==
				DfsPrimToCent(pDfsParam->band_ch[band_idx], pDfsParam->band_bw[band_idx]))
				continue;

			if (DfsPrimToCent(pChCtrl->ChList[ch_idx].Channel, Bw) ==
				DfsPrimToCent(pDfsParam->band_ch[band_idx], Bw))
				continue;
		}

		if (pChCtrl->ChList[ch_idx].NonOccupancy == 0) {
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[Bw][AvailableChCnt++] = ch_idx;
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NOP ! =0 (%d)\n",
					 __func__, pChCtrl->ChList[ch_idx].NonOccupancy));
	}

	if (Bw > BW_20) {
		/*Clear NOP of the current BW*/
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if ((pChCtrl->ChList[ch_idx].NonOccupancy != 0) && (pChCtrl->ChList[ch_idx].NOPSetByBw == Bw)) {
				pChCtrl->ChList[ch_idx].NOPSaveForClear = pChCtrl->ChList[ch_idx].NonOccupancy;
				pChCtrl->ChList[ch_idx].NonOccupancy = 0;
				pChCtrl->ChList[ch_idx].NOPClrCnt++;
			}
		}
		DfsBwChQueryAllList(pAd, Bw - 1, pDfsParam, SkipWorkingCh, band_idx);
	}

}

BOOLEAN DfsDedicatedCheckChBwValid(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 i, j, idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return TRUE;

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j] = 0xff;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, FALSE, band_idx);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j] != 0xff) {
				idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j];

				if ((pChCtrl->ChList[idx].Channel == Channel)
				 && (Bw == i)) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;

}

VOID DfsAdjustBwSetting(
	struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw)
{
	UCHAR HtBw;
	UCHAR VhtBw;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (NewBw == CurrentBw)
		return;

	switch (NewBw) {
	case BW_20:
		HtBw = BW_20;
		VhtBw = VHT_BW_2040;
		break;
	case BW_40:
		HtBw = BW_40;
		VhtBw = VHT_BW_2040;
		break;
	case BW_80:
		HtBw = BW_40;
		VhtBw = VHT_BW_80;
		break;
	case BW_160:
		HtBw = BW_40;
		VhtBw = VHT_BW_160;
		break;
	default:
		return;
	}

	cfg->ht_conf.ht_bw = HtBw;
	cfg->vht_conf.vht_bw = VhtBw;
}

VOID WrapDfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	struct DOT11_H *pDot11h = NULL;
	UCHAR band_idx;

	if (wdev == NULL)
		return;

	if (!IS_CH_ABAND(wdev->channel)) {
		return;
	}

	if (hc_radio_query_by_wdev(wdev, &oper)) {
		return;
	}

	if (wdev) {
		pDot11h = wdev->pDot11_H;
	}

	if (pDot11h == NULL)
		return;

	band_idx = HcGetBandByWdev(wdev);
	pDfsParam->DfsChBand[band_idx] = RadarChannelCheck(pAd, pDfsParam->band_ch[band_idx]);

#ifdef DOT11_VHT_AC
	if (pDfsParam->band_bw[band_idx] == BW_8080)
		pDfsParam->DfsChBand[HW_RDD1] = RadarChannelCheck(pAd, pDfsParam->band_ch[DBDC_BAND1]);

	if (pDfsParam->band_bw[band_idx] == BW_160)
		pDfsParam->DfsChBand[HW_RDD1] = pDfsParam->DfsChBand[HW_RDD0];

	if ((pDfsParam->band_bw[band_idx] == BW_160) &&
		(pDfsParam->PrimCh >= GROUP1_LOWER && pDfsParam->PrimCh <= GROUP1_UPPER))
		pDfsParam->DfsChBand[HW_RDD1] = TRUE;

#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]: band_ch[0]: %d, band_ch[1]: %d\n",
		__func__,
		pDfsParam->band_ch[HW_RDD0],
		pDfsParam->band_ch[HW_RDD1]));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]: DfsChBand[0]: %d, DfsChBand[1]: %d\n",
		__func__,
		pDfsParam->DfsChBand[HW_RDD0],
		pDfsParam->DfsChBand[HW_RDD1]));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM] BandIdx: %d, BW: %d, RDMode: %d\n",
		 __func__,
		 band_idx,
		 pDfsParam->band_bw[band_idx],
		 pDot11h->RDMode));
	DfsRadarDetectStart(pAd, pDfsParam, wdev);
}

VOID DfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	PDFS_PARAM pDfsParam,
	struct wifi_dev *wdev
)
{
	INT ret1 = TRUE;
	UCHAR band_idx;
	UCHAR rd_region = 0; /* Region of radar detection */
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	band_idx = HcGetBandByWdev(wdev);
	rd_region = pAd->CommonCfg.RDDurRegion;

	if (pDot11h == NULL)
		return;

	if (scan_in_run_state(pAd, NULL) || (pDot11h->RDMode == RD_SWITCHING_MODE))
		return;

	if (pDot11h->RDMode == RD_SILENCE_MODE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]:ZeroWaitState:%d\n",
				 __func__,
				 GET_MT_ZEROWAIT_DFS_STATE(pAd)));

		if (pDfsParam->RadarDetectState[band_idx] == FALSE) {
			if (pAd->CommonCfg.dbdc_mode) {
				/* DBDC mode */
				/* RddSel=0: Use band1/RX2 to detect radar */
				ret1 = mtRddControl(pAd, RDD_START, band_idx, RXSEL_0, rd_region);
			}

#ifdef DOT11_VHT_AC
			else if (pDfsParam->band_bw[band_idx] == BW_160) {
				{
					if ((pDfsParam->band_ch[DBDC_BAND0] >= GROUP1_LOWER &&
						pDfsParam->band_ch[DBDC_BAND0] <= GROUP1_UPPER))
						;
					else
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);

					ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
				}
			} else if (pDfsParam->band_bw[band_idx] == BW_8080) {
				{/*Prim in idx 0~3*/
					if (pDfsParam->DfsChBand[HW_RDD0])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, rd_region);
					if (pDfsParam->DfsChBand[HW_RDD1])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, rd_region);
				}
			}

#endif
			else
				ret1 = mtRddControl(pAd, RDD_START, band_idx, RXSEL_0, rd_region);
		}

		pDfsParam->RadarDetectState[band_idx] = TRUE;
	} else if (DfsIsOutBandAvailable(pAd, wdev) && pDfsParam->bDedicatedZeroWaitSupport) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]: out-band is available\n",
				 __func__));
		ret1 = mtRddControl(pAd, RDD_START, band_idx, RXSEL_0, rd_region);
	}
}

VOID WrapDfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsRadarDetectStop(pAd, pDfsParam);
}

VOID DfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
	INT ret1 = TRUE, ret2 = TRUE;
	pDfsParam->RadarDetectState[DBDC_BAND0] = FALSE;
	pDfsParam->RadarDetectState[DBDC_BAND1] = FALSE;

	if (!pDfsParam->bDfsEnable)
		return;

	ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
	ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
}

VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR rd_region = pAd->CommonCfg.RDDurRegion; /* Region of radar detection */

	pDfsParam->RadarDetected[RDD_DEDICATED_RX] = FALSE;
	pDfsParam->DfsChBand[RDD_DEDICATED_RX] = RadarChannelCheck(pAd, pDfsParam->OutBandCh);
	if (pDfsParam->DfsChBand[RDD_DEDICATED_RX]) {
		mtRddControl(pAd, RDD_START, RDD_DEDICATED_RX, RXSEL_0, rd_region);
		DfsOutBandCacReset(pAd);

		if ((pAd->CommonCfg.RDDurRegion == CE)
		 && DfsCacRestrictBand(pAd, pDfsParam->OutBandBw, pDfsParam->OutBandCh, 0))
			pDfsParam->DedicatedOutBandCacTime = CAC_WETHER_BAND;
		else
			pDfsParam->DedicatedOutBandCacTime = CAC_NON_WETHER_BAND;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[%s][RDM]: Dedicated CAC time: %d\n",
			__func__, pDfsParam->DedicatedOutBandCacTime));
	}
}

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT bw_ch = 0;
	UCHAR band_idx;
	USHORT bw_ch_band[DBDC_BAND_NUM];
	UCHAR bw_band[DBDC_BAND_NUM];
	UCHAR bw = 0;
	CHANNEL_CTRL *pChCtrl = NULL;
	UCHAR ch_idx = 0;
	BOOLEAN fg_in_band_use = FALSE;
	BOOLEAN fg_radar_detect = FALSE;

	mtRddControl(pAd, RDD_STOP, RDD_DEDICATED_RX, 0, 0);

	if (pDfsParam->bDedicatedZeroWaitDefault == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s() DedicatedZeroWaitDefault is not enabled\n", __func__));
		return;
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		bw_ch_band[band_idx] = 0;
		bw_band[band_idx] = 0;
		bw = pDfsParam->band_bw[band_idx];

		pChCtrl = DfsGetChCtrl(pAd, pDfsParam, bw, band_idx);

		/* Check A band */
		if (pChCtrl->ChList[0].Channel < 36) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s() not A-band, channel %d\n", __func__, pChCtrl->ChList[0].Channel));
			continue;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): In-band channel %d, outband ch %d\n",
				  __func__, pDfsParam->band_ch[band_idx], pDfsParam->OutBandCh));

		/* Check NOP of current outband ch */
		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].Channel == pDfsParam->OutBandCh) {
				if ((pChCtrl->ChList[ch_idx].NonOccupancy == 0) &&
					(pChCtrl->ChList[ch_idx].NOPSaveForClear == 0)) {

					if (DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->OutBandBw) ==
						DfsPrimToCent(pDfsParam->band_ch[band_idx], pDfsParam->band_bw[band_idx])) {
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								 ("%s(): In-band %d is using this channel %d\n",
								  __func__, band_idx, pDfsParam->band_ch[band_idx]));

						fg_in_band_use = TRUE;
						break;
					}
				}
				else {
					fg_radar_detect = TRUE;
				}
			}
		}

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (bw == BW_8080)
		{
			bw = BW_80;
			bw_ch_band[band_idx] = DfsBwChQueryByDefault(pAd, bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, BAND0);
		}
		else
#endif
		{
			bw_ch_band[band_idx] = DfsBwChQueryByDefault(pAd, bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, band_idx);
		}

		bw_band[band_idx] = bw_ch_band[band_idx]>>8;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s() bw_ch_band[%d] 0x%x\n", __func__, band_idx, bw_ch_band[band_idx]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s() bw_band[%d] 0x%x\n", __func__, band_idx, bw_band[band_idx]));

		bw_ch = bw_ch_band[band_idx];
		bw = bw_ch >> 8;
	}

	/* no in-band use ch same as out-band, keep use out-band */
	if ((fg_in_band_use == FALSE) && (fg_radar_detect == FALSE))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s(): NOP of Ch%d is clear, keep using this ch\n",
							  __func__, pDfsParam->OutBandCh));
		return;
	}

	pDfsParam->OutBandCh = bw_ch & 0xff;
	pDfsParam->OutBandBw = bw_ch>>8;

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if (pDfsParam->OutBandBw == BW_8080)
	{
		pDfsParam->OutBandBw = BW_80;
	}
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\x1b[1;33m%s() OutBandCh %d, OutBandBw %d \x1b[m\n",
		__func__, pDfsParam->OutBandCh, pDfsParam->OutBandBw));
}

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd)
{
#if RDD_PROJECT_TYPE_2
	mtRddControl(pAd, RDD_IRQ_OFF, RDD_DEDICATED_RX, 0, 0);
#else
	mtRddControl(pAd, RDD_STOP, RDD_DEDICATED_RX, 0, 0);
#endif

}

BOOLEAN DfsIsRadarHitReport(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	return pDfsParam->RadarHitReport == TRUE;
}

VOID DfsRadarHitReportReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->RadarHitReport = FALSE;
}

VOID DfsReportCollision(
	IN PRTMP_ADAPTER pAd)
{
#ifdef BACKGROUND_SCAN_SUPPORT
	if (IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd)
	&& DfsIsRadarHitReport(pAd)) {
		ZeroWait_DFS_collision_report(pAd, HW_RDD0,
		GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW));
		DfsRadarHitReportReset(pAd);
	}
#endif
}

BOOLEAN DfsIsTargetChAvailable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if ((pDfsParam->targetCh != 0) && (pDfsParam->targetCacValue == 0))
		return TRUE;

	return FALSE;
}

BOOLEAN DfsIsOutBandAvailable(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN bAvailable;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	bAvailable = ((pDfsParam->bOutBandAvailable == TRUE) &&
		(pDfsParam->bSetInBandCacReStart == FALSE));

	if (bAvailable == TRUE)
		pDfsParam->OutBandAvailableCh = pDfsParam->band_ch[band_idx];

	if (pDfsParam->band_ch[band_idx] == pDfsParam->OutBandAvailableCh)
		bAvailable = TRUE;
	else
		pDfsParam->OutBandAvailableCh = 0;

	return bAvailable;
}

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bOutBandAvailable = FALSE;
}

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE) {
		if ((pDot11h->RDMode == RD_SILENCE_MODE) && (pDfsParam->bSetInBandCacReStart == FALSE)) {
			pDot11h->RDCount = pDfsParam->DedicatedOutBandCacCount;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Remaining CAC time is %d \x1b[m \n",
			__func__, pDot11h->cac_time - pDot11h->RDCount));
		}
	}

	pDfsParam->bSetInBandCacReStart = FALSE;
	DfsOutBandCacReset(pAd);

}

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#ifdef DFS_CAC_R2
	UCHAR band_idx;
	UCHAR BssIdx;
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *pMbss = NULL;
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;
#endif

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;

	if (pDfsParam->bOutBandAvailable != FALSE)
		return;

	/* detection mode is enabled */
	if (pDfsParam->bNoSwitchCh == TRUE)
		return;

	if (pDfsParam->RadarDetected[RDD_DEDICATED_RX] == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() radar is detected by dedicated RX.\n", __func__));
		return;
	}

	if (pDfsParam->DedicatedOutBandCacCount++ > pDfsParam->DedicatedOutBandCacTime) {
		pDfsParam->bOutBandAvailable = TRUE;
		pDfsParam->DedicatedOutBandCacCount = 0;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() OutBand(SynB) CAC complete and is available now.\n", __func__));
#ifdef DFS_CAC_R2
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			/*add for non radar detected case by dedicated radio
			For Harrier we have done DFS by dedicated radio still send ifindex for 5G Radio*/
			band_idx = HW_RDD1;
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdev = &pMbss->wdev;
				if (wdev->pHObj == NULL)
					continue;
				if (HcGetBandByWdev(wdev) != band_idx)
					continue;
			}
			wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), pDfsParam->OutBandCh, TRUE);
		}
#endif
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
		if (pDfsParam->bDedicatedZeroWaitDefault == TRUE) {
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_OFF_CAC_END, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}
#else
		if (DfsCacTimeOutCallBack) {
			DfsCacTimeOutCallBack(RDD_BAND1, pDfsParam->OutBandBw, pDfsParam->OutBandCh);
		}
#endif
	}
}

VOID DfsDedicatedExamineSetNewCh(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	UCHAR InputCentCh = DfsPrimToCent(Channel, pDfsParam->band_bw[band_idx]);

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;

	if (InputCentCh == DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->band_bw[band_idx]))
		pDfsParam->bSetInBandCacReStart = FALSE;
	else
		pDfsParam->bSetInBandCacReStart = TRUE;

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
	DfsDedicatedSetNewChStat(pAd, wdev);
#endif /* DFS_ZEROWAIT_DEFAULT_FLOW */

}

VOID DfsDedicatedSetNewChStat(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	P_ENUM_DFS_INB_CH_SWITCH_STAT_T ch_stat = &pAd->CommonCfg.DfsParameter.inband_ch_stat;

	if (!WMODE_CAP_5G(wdev->PhyMode))
		return;

	if (pDfsParam->bDedicatedZeroWaitDefault == FALSE)
		return;

	if (!RadarChannelCheck(pAd, wdev->channel))
		return;

	*ch_stat = DFS_INB_CH_INIT;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() set to DFS channel - initialize stat\n", __func__));
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     Configure (Enable/Disable) HW RDD and RDD wrapper module
*
* \param[in] ucRddCtrl
*            ucRddIdex
*
*
* \return    None
*/
/*----------------------------------------------------------------------------*/

INT mtRddControl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddRxSel,
	IN UCHAR ucSetVal)
{
	INT ret = TRUE;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[mtRddControl]RddCtrl=%d, RddIdx=%d, RddRxSel=%d\n", ucRddCtrl, ucRddIdex, ucRddRxSel));
	ret = MtCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddRxSel, ucSetVal);
	return ret;
}

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR bw, struct wifi_dev *wdev)
{
	UCHAR CentCh = 0;

	if (bw == BW_20)
		CentCh = Channel;

#ifdef DOT11_N_SUPPORT
	else if ((bw == BW_40) && N_ChannelGroupCheck(pAd, Channel, wdev)) {
#ifdef A_BAND_SUPPORT

		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			CentCh = Channel + 2;
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			CentCh = Channel - 2;
#endif /* A_BAND_SUPPORT */
	}

#ifdef DOT11_VHT_AC
	else if (bw == BW_80) {
		if (vht80_channel_group(pAd, Channel, wdev))
			CentCh = vht_cent_ch_freq(Channel, VHT_BW_80, CMD_CH_BAND_5G);
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s][RDM]Error!Unexpected Bw=%d!!\n",
				 __func__,
				 bw));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][RDM]Control/Central Ch=%d/%d;Bw=%d\n",
			 __func__,
			 Channel,
			 CentCh,
			 bw));
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	return CentCh;
}

#ifdef BACKGROUND_SCAN_SUPPORT
VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd)
{
	UCHAR bw_band0, bw_band1, idx;
	USHORT bw_ch, bw_ch_band0, bw_ch_band1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\n",
			 __func__));

	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) &&
		(pDfsParam->bDedicatedZeroWaitDefault == TRUE)) {

			bw_ch_band0 = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, RDD_BAND0);
			bw_ch_band1 = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE, RDD_BAND1);

			bw_band0 = bw_ch_band0>>8;
			bw_band1 = bw_ch_band1>>8;

			if (bw_band0 > bw_band1)
				bw_ch = bw_band0;

			else if (bw_band0 < bw_band1)
				bw_ch = bw_band1;

			else {
				/* bw_band0 == bw_band1 */
				idx = RandomByte(pAd) % 2;
				bw_ch = (idx) ? bw_band0 : bw_band1;
			}

			pDfsParam->OutBandCh = bw_ch & 0xff;
			pDfsParam->OutBandBw = bw_ch>>8;

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
			if (pDfsParam->OutBandBw == BW_8080)
			{
				pDfsParam->OutBandBw = BW_80;
			}
#endif

		if (pDfsParam->OutBandCh == 0) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] No available Outband BW\n",
					 __func__));
			return;
		}

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	}
}

VOID DfsInitDedicatedScanStart(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if (pDfsParam->bInitOutBandBranch == TRUE) {
		pDfsParam->bInitOutBandBranch = FALSE;
		DfsDedicatedScanStart(pAd);
	}
}

VOID DfsSetInitDediatedScanStart(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bInitOutBandBranch = TRUE;
}

VOID DfsDedicatedInBandSetChannel(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC, UCHAR band_idx)
{
	UCHAR NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *dot11h_param = NULL;
	UINT_32 SetChInfo = 0;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	UCHAR vht_bw;
#endif
	UCHAR tempBand;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[1;33m [%s][RDM] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
		__func__, band_idx, Channel, Bw));

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s][RDM] DedicatedZeroWaitSupport is not enabled\n",
			__func__));
		return;
	}

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if ((pDfsParam->DFSChHitBand != DFS_BAND_NONE) && (Bw == BW_80))
#endif
	{
		if (pAd->CommonCfg.dbdc_mode) {
			if (!DfsDedicatedCheckChBwValid(pAd, Channel, Bw, band_idx)) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Not a Valid InBand Channel. Fail. \x1b[m \n", __FUNCTION__));
					return;
			}
		}
		else {
			if (!DfsDedicatedCheckChBwValid(pAd, Channel, Bw, RDD_BAND0)) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Not a Valid InBand Channel. Fail. \x1b[m \n", __FUNCTION__));
					return;
			}
		}
	}

	if (Channel == 0 ||
		((Channel == pDfsParam->OutBandCh) && (Bw == pDfsParam->OutBandBw))) {
		Channel = pDfsParam->OutBandCh;
		Bw = pDfsParam->OutBandBw;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("InBand set to OutBand Channel %d, Bw :%d\n", Channel, Bw));
	} else {
		pDfsParam->bSetInBandCacReStart = TRUE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("InBand set to non-OutBand Channel %d, Bw %d\n", Channel, Bw));
	}

	if (pAd->CommonCfg.dbdc_mode) {
		dot11h_param = &pAd->Dot11_H[band_idx];
	} else {
		dot11h_param = &pAd->Dot11_H[RDD_BAND0];
	}

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if (DfsCheckHitBandBWDbdcMode(pAd, Bw))
#endif
	{
		if ((Channel == pDfsParam->band_ch[band_idx])  &&  (Bw == pDfsParam->band_bw[band_idx])) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m This is current Ch %d, Bw %d \x1b[m \n", Channel, Bw));
			if ((doCAC == FALSE) && (dot11h_param->RDMode == RD_SILENCE_MODE)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Enable beacon now \x1b[m \n"));
				dot11h_param->RDCount = dot11h_param->cac_time;
			}
			return;
		}
	}

	if (doCAC == FALSE) {
		pDfsParam->bSetInBandCacReStart = FALSE;
		pDfsParam->bOutBandAvailable = TRUE;
	}

	tempBand = band_idx;
	if (pDfsParam->DFSChHitBand != DFS_BAND_NONE)
		tempBand = pDfsParam->DFSChHitBand;

	pDfsParam->OrigInBandCh = pDfsParam->band_ch[tempBand];
	pDfsParam->OrigInBandBw = pDfsParam->band_bw[tempBand];
	pDfsParam->band_ch[tempBand] = Channel;
	pDfsParam->PrimCh = pDfsParam->band_ch[band_idx];
	pDfsParam->PrimBand = band_idx;
	NextCh = pDfsParam->PrimCh;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		vht_bw = wlan_config_get_vht_bw(wdev);

		if ((Bw == BW_80) && (vht_bw == VHT_BW_8080))
		{
			Bw = BW_8080;
		}
#endif

		if (wdev->pHObj == NULL)
			continue;
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
		if (pAd->CommonCfg.dbdc_mode) {
			if (HcGetBandByWdev(wdev) != band_idx)
				continue;
		}
#endif

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));

		/*Adjust Bw*/
		DfsAdjustBwSetting(wdev, pDfsParam->band_bw[band_idx], Bw);

		if (dot11h_param->RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[band_idx] = FALSE;
			pDfsParam->RadarDetected[band_idx] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
						 __func__,
						 NextCh,
						 Bw));

			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (dot11h_param->RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[band_idx] = FALSE;
			pDfsParam->RadarDetected[band_idx] = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
						 __func__,
						 NextCh,
						 Bw));
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (band_idx << 16);

			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
}

VOID DfsDedicatedOutBandSetChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN UCHAR Bw,
	IN UCHAR band_idx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	if (Bw == BW_8080)
	{
		Bw = BW_80;
	}
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] SynNum: %d, Channel: %d, Bw: %d\n",
		__func__, band_idx, Channel, Bw));

	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s][RDM] DedicatedZeroWaitSupport is not enabled\n",
			__func__));
		return;
	}

	if (!(DfsDedicatedCheckChBwValid(pAd, Channel, Bw, RDD_BAND0))
#if (RDD_2_SUPPORTED == 1)
		&& !(DfsDedicatedCheckChBwValid(pAd, Channel, Bw, RDD_BAND1))
#endif
		) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Get new outband DFS channel\n", __func__));
		DfsDedicatedOutBandRDDRunning(pAd);

		if (pDfsParam->OutBandCh != 0) {
			Channel = pDfsParam->OutBandCh;
			Bw = pDfsParam->OutBandBw;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[1;33m [%s][RDM] No valid OutBand Channel. Fail. \x1b[m \n", __func__));
			return;
		}
	}
	if (!RadarChannelCheck(pAd, Channel)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\x1b[1;33m [%s][RDM] Not a DFS Channel. No need for Radar Detection. \x1b[m \n", __func__));
		return;
	}

	if (Channel != 0) {
		pDfsParam->OutBandCh = Channel;
		pDfsParam->OutBandBw = Bw;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pick OutBand Ch by internal Alogorithm\n"));
	}

	if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dediated Running: OutBand set Channel to %d\n", Channel));
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else if (GET_BGND_STATE(pAd, BGND_SCAN_IDLE)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dedicated Start: OutBand set Channel to %d\n", Channel));
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Wrong state. OutBand Set Channel Fail\n"));
	}
}

#if (RDD_2_SUPPORTED == 0)
VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] DfsDedicatedOnOff: %d \x1b[m \n",
		__FUNCTION__, DfsDedicatedOnOff));

	if (DfsDedicatedOnOff == DYNAMIC_ZEROWAIT_OFF) {
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			pDfsParam->OrigInBandCh = pDfsParam->PrimCh;
			pDfsParam->OrigInBandBw = pDfsParam->band_bw[RDD_BAND0];
			DedicatedZeroWaitStop(pAd, FALSE);
			DfsOutBandCacReset(pAd);
			pDfsParam->RadarDetected[1] = FALSE;
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Already in 4x4 mode \x1b[m \n", __FUNCTION__));
	} else	{
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] Already in 2x2 mode \x1b[m \n", __FUNCTION__));
		else if (pDfsParam->OutBandCh == 0)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s][RDM] No SynB Info Recorded. Fail. \x1b[m \n", __FUNCTION__));
		else {

			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif /* RDD_2_SUPPORTED */
#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	UCHAR Bw80Num = 4;
	UCHAR Bw40Num = 10;
	UCHAR Bw20Num = 11;

	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[4]
	= {{116, 0}, {120, 0}, {124, 0}, {128, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[10]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[11]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0}, {140, 0} };

	Value = (UCHAR) simple_strtol(arg, 0, 10);

	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80Num, &Bw80AvailableChList[0],
	Bw40Num, &Bw40AvailableChList[0],
	Bw20Num, &Bw20AvailableChList[0]);

	return TRUE;
}

INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR band_idx;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("band_idx: %d\n", band_idx));
		DfsProvideAvailableChList(pAd, band_idx);
	}

	return TRUE;
}

INT Show_NOP_Of_ChList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	union dfs_zero_wait_msg msg;
	UCHAR ch_idx = 0, band_idx;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));

	DfsProvideNopOfChList(pAd, &msg);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("[%s][RDM]\n", __func__));

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("band_idx: %d\n", band_idx));

		for (ch_idx = 0; ch_idx < msg.nop_of_channel_list_msg.NOPTotalChNum[band_idx]; ch_idx++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
			ch_idx,
			msg.nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].Channel,
			msg.nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].Bw,
			msg.nop_of_channel_list_msg.NopReportChList[band_idx][ch_idx].NonOccupancy));
		}
	}
	return TRUE;
}

INT Show_Target_Ch_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ZeroWait_DFS_Next_Target_Show(pAd, 1);
	return TRUE;
}

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN PRTMP_ADAPTER pAd,
	UCHAR Bw80Num, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	UCHAR Bw40Num, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	UCHAR Bw20Num, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList)
{
	UINT_8 band_idx, i = 0, j = 0, k = 0;
	UINT_8 ChIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR SupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	UCHAR OrigSupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	PCHANNEL_CTRL pChCtrl = NULL;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->Dot11_H[band_idx].RDMode == RD_SWITCHING_MODE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Channel list init fail during channel switch\n"));
			return;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("band_idx: %d\n", band_idx));

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
			if (pBw80AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
				SupportBwBitMap[ChIdx] |= 0x04;
				if (i++ < Bw80Num)
					pBw80AvailableChList++;
			}
			if (pBw40AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
				SupportBwBitMap[ChIdx] |= 0x02;
				if (j++ < Bw40Num)
					pBw40AvailableChList++;
			}
			if (pBw20AvailableChList->Channel == pChCtrl->ChList[ChIdx].Channel) {
				SupportBwBitMap[ChIdx] |= 0x01;
				if (k++ < Bw20Num)
					pBw20AvailableChList++;
			}
		}

		for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
			OrigSupportBwBitMap[ChIdx] = pChCtrl->ChList[ChIdx].SupportBwBitMap;

			if (OrigSupportBwBitMap[ChIdx] >= 0x07) {
				if (SupportBwBitMap[ChIdx] == 0x07)
					;
				else if (SupportBwBitMap[ChIdx] == 0x03) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_80;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else if (SupportBwBitMap[ChIdx] == 0x01) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_40;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else if (SupportBwBitMap[ChIdx] == 0x0) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else
					;
			} else if (OrigSupportBwBitMap[ChIdx] == 0x03) {
				if (SupportBwBitMap[ChIdx] == 0x03)
					;
				else if (SupportBwBitMap[ChIdx] == 0x01) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_40;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else if (SupportBwBitMap[ChIdx] == 0x0) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else
					;
			} else if (OrigSupportBwBitMap[ChIdx] == 0x01) {
				if (SupportBwBitMap[ChIdx] == 0x01)
					;
				else if (SupportBwBitMap[ChIdx] == 0x0) {
					pChCtrl->ChList[ChIdx].NOPSetByBw = BW_20;
					pChCtrl->ChList[ChIdx].NOPClrCnt = 1;
					pChCtrl->ChList[ChIdx].NOPSaveForClear = 1800;
				} else
					;
			} else
				;
		}

		for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
			for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
				pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j] = 0xff;
		}

		DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE, band_idx);

		for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
#ifdef DFS_DBG_LOG_0
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n", i));
#endif
			for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
				if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j] != 0xff) {
					ChIdx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[i][j];
#ifdef DFS_DBG_LOG_0
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
					ChIdx,
					pChCtrl->ChList[ChIdx].Channel,
					pChCtrl->ChList[ChIdx].NOPClrCnt));
#endif
				}
			}
		}
	}
}

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd, IN UCHAR band_idx)
{
	UINT_8 bw_idx, ch_idx, idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {
		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++)
			pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[bw_idx][ch_idx] = 0xff;
	}

	for (idx = 0; idx < DBDC_BAND_NUM; idx++) {
		if (pAd->Dot11_H[idx].RDMode == RD_SWITCHING_MODE)
			return;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE, band_idx);

	for (bw_idx = 0; bw_idx < DFS_AVAILABLE_LIST_BW_NUM; bw_idx++) {

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n", bw_idx));

		for (ch_idx = 0; ch_idx < DFS_AVAILABLE_LIST_CH_NUM; ch_idx++) {
			if (pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[bw_idx][ch_idx] != 0xff) {
				idx = pDfsParam->dfs_ch_grp[band_idx].AvailableBwChIdx[bw_idx][ch_idx];
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				idx, pChCtrl->ChList[idx].Channel,
				pChCtrl->ChList[idx].NOPClrCnt));
			}
		}
	}
}

VOID DfsProvideNopOfChList(
	IN PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg)
{
	UINT_8 ch_idx;
	UINT_8 nop_ch_idx = 0;
	UINT_8 band_idx = 0;
	PCHANNEL_CTRL pChCtrl = NULL;

	NOP_REPORT_CH_LIST NopReportChList[DFS_AVAILABLE_LIST_CH_NUM];

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		os_zero_mem(&NopReportChList, sizeof(NOP_REPORT_CH_LIST) * DFS_AVAILABLE_LIST_CH_NUM);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].NonOccupancy != 0) {
				NopReportChList[nop_ch_idx].Channel = pChCtrl->ChList[ch_idx].Channel;
				NopReportChList[nop_ch_idx].Bw = pChCtrl->ChList[ch_idx].NOPSetByBw;
				NopReportChList[nop_ch_idx].NonOccupancy = pChCtrl->ChList[ch_idx].NonOccupancy;
				nop_ch_idx++;
			} else if (pChCtrl->ChList[ch_idx].NOPSaveForClear != 0) {
				NopReportChList[nop_ch_idx].Channel = pChCtrl->ChList[ch_idx].Channel;
				NopReportChList[nop_ch_idx].Bw = pChCtrl->ChList[ch_idx].NOPSetByBw;
				NopReportChList[nop_ch_idx].NonOccupancy = pChCtrl->ChList[ch_idx].NOPSaveForClear;
				nop_ch_idx++;
			}
		}

		for (ch_idx = 0; ch_idx < nop_ch_idx; ch_idx++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("Local NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
				ch_idx, NopReportChList[ch_idx].Channel, NopReportChList[ch_idx].Bw,
				NopReportChList[ch_idx].NonOccupancy));
		}

		msg->nop_of_channel_list_msg.NOPTotalChNum[band_idx] = nop_ch_idx;
		memcpy(&(msg->nop_of_channel_list_msg.NopReportChList[band_idx][0]),
		NopReportChList,
		nop_ch_idx * sizeof(NOP_REPORT_CH_LIST));
	}
}

VOID ZeroWait_DFS_set_NOP_to_Channel_List(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, UCHAR Bw, USHORT NOPTime)
{
	UINT_8 ch_idx;
	UCHAR band_idx;
	PCHANNEL_CTRL pChCtrl = NULL;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] Channel: %d, Bw: %d, NOP: %d\n",
	__FUNCTION__, Channel, Bw, NOPTime));

	if (Bw > BW_80) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] Not a valid BW for ZeroWait\n",
		__func__));
		return;
	}
	if (!RadarChannelCheck(pAd, Channel)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] Ch %d is not a DFS channel. InValid\n",
		__func__, Channel));
		return;
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		if (ByPassChannelByBw(Channel, Bw, pChCtrl)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][RDM] Ch%d doesn't support BW %d\n",
			__func__, Channel, Bw));
			return;
		}

		for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
			if (Channel == pChCtrl->ChList[ch_idx].Channel) {
				pChCtrl->ChList[ch_idx].NOPSetByBw = Bw;
				pChCtrl->ChList[ch_idx].NOPClrCnt++;

				switch (Bw) {
				case BW_80:
				case BW_40:
					pChCtrl->ChList[ch_idx].NOPSaveForClear = NOPTime;
					break;

				case BW_20:
					pChCtrl->ChList[ch_idx].NonOccupancy = NOPTime;
					break;

				default:
					break;
				}
			}
		}
	}
}

VOID ZeroWait_DFS_Pre_Assign_Next_Target_Channel(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR Bw, IN USHORT CacValue)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->targetCh = Channel;
	pDfsParam->targetBw = Bw;
	pDfsParam->targetCacValue = CacValue;
}

VOID ZeroWait_DFS_Next_Target_Show(
	IN PRTMP_ADAPTER pAd, IN UCHAR mode)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (mode != 0)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s][RDM] Target Channel: %d, Target Bw: %d, Target CAC value:%d \x1b[m \n",
		__FUNCTION__, pDfsParam->targetCh, pDfsParam->targetBw, pDfsParam->targetCacValue));

}

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd, IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("\x1b[1;33m[%s][RDM] SynNum: %d, Channel: %d, Bw:%d \x1b[m \n",
	__func__, SynNum, Channel, Bw));

	if (radar_detected_callback_func) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\x1b[1;33m[%s][RDM] Call back func \x1b[m \n", __func__));

		radar_detected_callback_func(SynNum, Channel, Bw);
	}

}

VOID DfsZeroHandOffRecovery(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	if (pDot11h) {
		if (pDot11h->RDMode == RD_SILENCE_MODE) {
			mtRddControl(pAd, RDD_RESUME_BF, HW_RDD0, 0, 0);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Resume BF.\n", __func__));
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief        Mapping RDD index to DBDC index
* @param[in]    PRTMP_ADAPTER pAd
* @param[in]    rddidx: RDD index
* @return       bandIdx: DBDC index
*/
/*----------------------------------------------------------------------------*/
UCHAR dfs_rddidx_to_dbdc(IN PRTMP_ADAPTER pAd, IN UINT8 rddidx)
{
	UCHAR bandidx = rddidx;

#if (RDD_PROJECT_TYPE_1 == 1)
	/* Single PHY, DBDC, RDD0/RDD1 */
	if (IS_SUPPORT_SINGLE_PHY_DBDC_DUAL_RDD(pAd)) {
		PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
		if (pDfsParam->band_bw[DBDC_BAND0] == BW_8080 || pDfsParam->band_bw[DBDC_BAND0] == BW_160)
			bandidx = DBDC_BAND0;
	}

#if (RDD_2_SUPPORTED == 1)
	if (IS_SUPPORT_RDD2_DEDICATED_RX(pAd)) {
		/*PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;*/

		switch (rddidx) {
		case HW_RDD0:
		case HW_RDD1:
			break;

		case HW_RDD2:
			bandidx = RDD_DEDICATED_RX;
			break;

		default:
			break;
		}
	}
#endif /* RDD_2_SUPPORTED */
#endif /* RDD_PROJECT_TYPE_1 */

#if (RDD_PROJECT_TYPE_2 == 1)
	/* Dual PHY dual band */
	/* DBDC 1 is for 5G (RDD0/RDD1) */
	if (IS_MT7626(pAd))
		bandidx = DBDC_BAND1;
#endif /* RDD_PROJECT_TYPE_2 */

	return bandidx;
}

VOID DfsSetNewChInit(IN PRTMP_ADAPTER pAd)
{
	UCHAR band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++)
		pDfsParam->NeedSetNewChList[band_idx] = DFS_SET_NEWCH_INIT;
}

#endif /*MT_DFS_SUPPORT*/

