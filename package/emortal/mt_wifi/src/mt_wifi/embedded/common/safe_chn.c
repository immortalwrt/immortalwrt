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
    safe_chn.c//Fancy20200810
*/

#ifdef WIFI_MD_COEX_SUPPORT
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

typedef enum _ENUM_LTE_SAFE_CH_OP_T {
	LTE_SAFE_CH_OP_NO_CHNG = 0,
	LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE = 1,
	LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE = 2,
	LTE_SAFE_CH_OP_UNSAFE_SWITCH = 3
} ENUM_LTE_SAFE_CH_OP_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

//#define SAFE_CHN_LOG_TRACE(...) printk(__VA_ARGS__)
#define SAFE_CHN_LOG_TRACE(...) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (__VA_ARGS__))
//#define SAFE_CHN_LOG_ERR(...) printk(__VA_ARGS__)
#define SAFE_CHN_LOG_ERR(...) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (__VA_ARGS__))
#define SAFE_CHN_LOG_FORCE(...) MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (__VA_ARGS__))

static int cmm_utl_is_bit_set(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		SAFE_CHN_LOG_ERR("%s:error !!!!\n", __FUNCTION__);
		return 0;
	}

	mask = 1 << (bit);
	return ((mask & *flags) != 0);
}

static void cmm_utl_set_bit(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		SAFE_CHN_LOG_ERR("%s:error !!!!\n", __FUNCTION__);
		return;
	}

	mask = 1 << (bit);
	*flags |= mask;
}

static void cmm_utl_clear_bit(UINT_32 *flags, UCHAR bit)
{
	int	mask;

	if (bit > 31) {
		SAFE_CHN_LOG_ERR("%s:error !!!!\n", __FUNCTION__);
		return;
	}

	mask = 1 << (bit);
	*flags &= (~mask);
}

static BOOLEAN get_chn_bitmask_pos(UCHAR channel, UCHAR *ch_type_idx, UCHAR *ch_bit_pos)
{
	UCHAR ch_step = 1, ch_idx_start = 0;

	SAFE_CHN_LOG_TRACE("%s, channel(%d).\n", __func__, channel);

	if (channel >= SAFE_CHN_START_IDX_2G4 && channel <= SAFE_CHN_END_IDX_2G4) {
		ch_step = 1;
		ch_idx_start = SAFE_CHN_START_IDX_2G4;
		*ch_type_idx = SAFE_CHN_MASK_BAND_2G4;
	} else {
		ch_step = 4;
		if (channel >= SAFE_CHN_START_IDX_5G0 && channel <= SAFE_CHN_END_IDX_5G0) {
			ch_idx_start = SAFE_CHN_START_IDX_5G0;
			*ch_type_idx = SAFE_CHN_MASK_BAND_5G_0;
		} else if (channel >= SAFE_CHN_START_IDX_5G1 && channel <= SAFE_CHN_END_IDX_5G1) {
			ch_idx_start = SAFE_CHN_START_IDX_5G1;
			*ch_type_idx = SAFE_CHN_MASK_BAND_5G_1;
		} else {
			SAFE_CHN_LOG_ERR("%s, Channel(%d) out of range!!\n", __func__, channel);
			return FALSE;
		}
	}

	*ch_bit_pos = (channel - ch_idx_start) / ch_step;

	SAFE_CHN_LOG_TRACE("%s, ch(%d) ch_type_idx=%d, ch_bit_pos=%d.\n",
		__func__, channel, *ch_type_idx, *ch_bit_pos);

	return TRUE;
}

static VOID safe_chn_list_to_bitmask(UCHAR *ch_list, UCHAR ch_cnt, UINT32 *bitmask)
{
	UCHAR i, ch_bit_pos, ch_type_idx;

	SAFE_CHN_LOG_TRACE("%s, ch_cnt(%d). \n", __func__, ch_cnt);

	NdisZeroMemory(bitmask, SAFE_CHN_MASK_IDX_NUM * sizeof(UINT32));
	for (i = 0; i < ch_cnt; i++) {
		if (get_chn_bitmask_pos(ch_list[i], &ch_type_idx, &ch_bit_pos))
			cmm_utl_set_bit(&bitmask[ch_type_idx], ch_bit_pos);
	}
}

static VOID unsafe_chn_list_to_bitmask(UCHAR *ch_list, UCHAR ch_cnt, UINT32 *bitmask)
{
	UCHAR i, ch_bit_pos, ch_type_idx;

	SAFE_CHN_LOG_TRACE("%s, ch_cnt(%d). \n", __func__, ch_cnt);

	NdisFillMemory(bitmask, SAFE_CHN_MASK_IDX_NUM * sizeof(UINT32), 0xFF);
	for (i = 0; i < ch_cnt; i++) {
		if (get_chn_bitmask_pos(ch_list[i], &ch_type_idx, &ch_bit_pos))
			cmm_utl_clear_bit(&bitmask[ch_type_idx], ch_bit_pos);
	}
}

static UCHAR get_channel_type_by_band(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	USHORT PhyMode = HcGetRadioPhyModeByBandIdx(pAd, band_idx);

	if (WMODE_CAP_5G(PhyMode))
		return SAFE_CHN_TYPE_5G;
	else if (WMODE_CAP_2G(PhyMode))
		return SAFE_CHN_TYPE_2G4;
	else {
		SAFE_CHN_LOG_TRACE("\n%s channel type out of range! pAd=%p, PhyMode=x%x, band_idx=%d\n", __func__, pAd, PhyMode, band_idx);
		return SAFE_CHN_TYPE_NONE;
	}
}

static UCHAR select_safe_channel(PRTMP_ADAPTER pAd, UCHAR band_idx)
{
	UCHAR new_chn = 0;

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

#ifdef MT_DFS_SUPPORT
	new_chn = WrapDfsRandomSelectChannel(pAd, FALSE, 0, band_idx);
#endif

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d, new_chn=%d\n", __func__, band_idx, new_chn);
	return new_chn;
}

static BOOLEAN update_safe_chn_db(RTMP_ADAPTER *pAd,
	UINT32 *channel_bit_mask, UCHAR *change_type)
{
	UCHAR idx;
	BOOLEAN bChanged = FALSE;

	SAFE_CHN_LOG_TRACE("\n%s: pAd=%p, channel_bit_mask=%x-%x-%x\n",
		__func__, pAd, pAd->LteSafeChCtrl.SafeChnBitmask[0],
		pAd->LteSafeChCtrl.SafeChnBitmask[1], pAd->LteSafeChCtrl.SafeChnBitmask[2]);

	*change_type = SAFE_CHN_TYPE_NONE;

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	for (idx = SAFE_CHN_MASK_BAND_2G4; idx < SAFE_CHN_MASK_IDX_NUM; idx++) {
		if (pAd->LteSafeChCtrl.SafeChnBitmask[idx] != channel_bit_mask[idx]) {
			pAd->LteSafeChCtrl.SafeChnBitmask[idx] = channel_bit_mask[idx];

			if (idx == SAFE_CHN_MASK_BAND_2G4)
				*change_type |= SAFE_CHN_TYPE_2G4;
			else
				*change_type |= SAFE_CHN_TYPE_5G;
		}
	}
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	bChanged = *change_type ? TRUE : FALSE;
	SAFE_CHN_LOG_TRACE("\n%s: pAd=%p, channel_bit_mask=%x-%x-%x, bChanged=%d(%d)\n",
		__func__, pAd, pAd->LteSafeChCtrl.SafeChnBitmask[0],
		pAd->LteSafeChCtrl.SafeChnBitmask[1],
		pAd->LteSafeChCtrl.SafeChnBitmask[2], bChanged, *change_type);

	return bChanged;
}

static VOID set_safe_chn_chg_flag(RTMP_ADAPTER *pAd, UCHAR band_idx)
{
	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.WaitForSafeChOpCnt[band_idx]++;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
}

static VOID band_recover_from_all_unsafe(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	BSS_STRUCT *pMbss = NULL;
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	if (!pMbss) {
	   SAFE_CHN_LOG_ERR("%s, Invalid pMbss: band_idx(%d).\n", __func__, band_idx);
	   return;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.bAllUnsafe[band_idx] = FALSE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	if (!IsChannelSafe(pAd, wdev->channel)) {
		wdev->channel = select_safe_channel(pAd, band_idx);
		wdev_sync_prim_ch(pAd, wdev);
	}

	SAFE_CHN_LOG_FORCE("Recover from all channel unsafe case! Startup AP band %d.\n", band_idx);
	APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	pAd->LteSafeChCtrl.BandUpCnt[band_idx]++;
}

static VOID band_turn_into_all_unsafe(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	BSS_STRUCT *pMbss = NULL;

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	if (!pMbss) {
	   SAFE_CHN_LOG_ERR("%s, Invalid pMbss: band_idx(%d).\n", __func__, band_idx);
	   return;
	}

	SAFE_CHN_LOG_FORCE("All channels are unsafe now! Stop AP band %d.\n", band_idx);
	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	pAd->LteSafeChCtrl.bAllUnsafe[band_idx] = TRUE;
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	pAd->LteSafeChCtrl.BandBanCnt[band_idx]++;
}

static VOID switch_unsafe_channel(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	BSS_STRUCT *pMbss = NULL;
	UCHAR new_channel;

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	if (!pMbss) {
		SAFE_CHN_LOG_ERR("%s, Invalid pMbss: band_idx(%d).\n", __func__, band_idx);
		return;
	}

	if (!IsChannelSafe(pAd, wdev->channel)) {
		new_channel = select_safe_channel(pAd, band_idx);
		SAFE_CHN_LOG_FORCE("Current channel %d is unsafe now! Switch to channel %d.\n", wdev->channel, new_channel);
		if (!rtmp_set_channel(pAd, wdev, new_channel))
			SAFE_CHN_LOG_ERR("%s, rtmp_set_channel failed: band_idx(%d), new_channel(%d).\n",
			__func__, band_idx, new_channel);
		else
			SAFE_CHN_LOG_TRACE("%s, rtmp_set_channel succeed: band_idx(%d), new_channel(%d).\n",
			__func__, band_idx, new_channel);
		
		pAd->LteSafeChCtrl.ChnSwitchCnt[band_idx]++;
	}
}

static BOOLEAN is_all_available_chn_unsafe(PRTMP_ADAPTER pAd, UCHAR band_idx)
{
	BOOLEAN b_all_unsafe = TRUE;
	UINT32 available_safe_chn_bitmask[SAFE_CHN_MASK_IDX_NUM];
	UCHAR ch_type_idx, ch_type;

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	ch_type = get_channel_type_by_band(pAd, band_idx);

	for (ch_type_idx = SAFE_CHN_MASK_BAND_2G4; ch_type_idx < SAFE_CHN_MASK_IDX_NUM; ch_type_idx++) {
		available_safe_chn_bitmask[ch_type_idx] = pAd->LteSafeChCtrl.SafeChnBitmask[ch_type_idx] & pAd->LteSafeChCtrl.AvaChnBitmask[ch_type_idx];
	}

	if (ch_type == SAFE_CHN_TYPE_2G4) {
		if (available_safe_chn_bitmask[SAFE_CHN_MASK_BAND_2G4] != 0) {
			b_all_unsafe = FALSE;
		}
	} else if (ch_type == SAFE_CHN_TYPE_5G) {
		for (ch_type_idx = SAFE_CHN_MASK_BAND_5G_0; ch_type_idx < SAFE_CHN_MASK_IDX_NUM; ch_type_idx++) {
			if (available_safe_chn_bitmask[ch_type_idx] != 0) {
				b_all_unsafe = FALSE;
			}
		}
	}

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d, b_all_unsafe=%d\n", __func__, band_idx, b_all_unsafe);
	return b_all_unsafe;
}

static BOOLEAN is_chn_op_state_idle(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	SAFE_CHN_LOG_TRACE("\n%s\n", __func__);
	if (scan_in_run_state(pAd, wdev)) {
		SAFE_CHN_LOG_TRACE("\n%s: Scan is running now!\n", __func__);
		return FALSE;
	}

	pDot11h = wdev->pDot11_H;
	if (pDot11h) {
		if (pDot11h->RDMode != RD_NORMAL_MODE) {
			SAFE_CHN_LOG_TRACE("\n%s: RDMode=%d!\n", __func__, pDot11h->RDMode);
			return FALSE;
		}
	}

	return TRUE;
}

VOID band_safe_chn_event_process(RTMP_ADAPTER *pAd, UCHAR band_idx)
{
	BOOLEAN need_enqueue = FALSE;

	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d\n", __func__, band_idx);

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	if (pAd->LteSafeChCtrl.WaitForSafeChOpCnt[band_idx] > 0) {
		pAd->LteSafeChCtrl.WaitForSafeChOpCnt[band_idx] = 0;
		need_enqueue = TRUE;
	}
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	if (need_enqueue) {
		RTEnqueueInternalCmd(pAd, CMDTHREAD_LTE_SAFE_CHN_CHG, &band_idx, sizeof(band_idx));
		SAFE_CHN_LOG_TRACE("%s, Enqueue cmd for band(%d). \n", __func__, band_idx);
	}
}

/**
* LteSafeChannelInit - Init LTE safe channel.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID LteSafeChannelInit(IN PRTMP_ADAPTER	pAd)
{
	P_LTE_SAFE_CH_CTRL pSafeChCtl = &pAd->LteSafeChCtrl;
	UINT32 band_idx;

	SAFE_CHN_LOG_TRACE("\n%s, pAd=%p\n", __func__, pAd);

	pSafeChCtl->bQueryLteDone = FALSE;
	pSafeChCtl->SafeChnProcIntvl = UNSAFE_CHN_PROC_INTVL * 3;	/* Triple interval at wifi_init phase */
	pSafeChCtl->RcvLteEventCnt = 0;
	pSafeChCtl->SafeChnChgCnt = 0;
	NdisFillMemory(pSafeChCtl->SafeChnBitmask, sizeof(pSafeChCtl->SafeChnBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->AvaChnBitmask, sizeof(pSafeChCtl->AvaChnBitmask), 0xFF);

	for (band_idx = DBDC_BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pSafeChCtl->bAllUnsafe[band_idx] = FALSE;
		pSafeChCtl->WaitForSafeChOpCnt[band_idx] = 0;
		pSafeChCtl->ChnSwitchCnt[band_idx] = 0;
		pSafeChCtl->BandBanCnt[band_idx] = 0;
		pSafeChCtl->BandUpCnt[band_idx] = 0;
		pSafeChCtl->FailCnt[band_idx] = 0;
	}
}

/**
* LteSafeChannelDeinit - Deinit LTE safe channel.
* @pAd: pointer of the RTMP_ADAPTER
**/
VOID LteSafeChannelDeinit(IN PRTMP_ADAPTER pAd)
{
	P_LTE_SAFE_CH_CTRL pSafeChCtl = &pAd->LteSafeChCtrl;
	UINT32 band_idx;

	SAFE_CHN_LOG_TRACE("\n%s, pAd=%p\n", __func__, pAd);

	pSafeChCtl->bQueryLteDone = FALSE;
	pSafeChCtl->SafeChnProcIntvl = UNSAFE_CHN_PROC_INTVL * 3;	/* Triple interval at wifi_init phase */
	pSafeChCtl->RcvLteEventCnt = 0;
	pSafeChCtl->SafeChnChgCnt = 0;
	NdisFillMemory(pSafeChCtl->SafeChnBitmask, sizeof(pSafeChCtl->SafeChnBitmask), 0xFF);
	NdisFillMemory(pSafeChCtl->AvaChnBitmask, sizeof(pSafeChCtl->AvaChnBitmask), 0xFF);

	for (band_idx = DBDC_BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pSafeChCtl->bAllUnsafe[band_idx] = FALSE;
		pSafeChCtl->WaitForSafeChOpCnt[band_idx] = 0;
		pSafeChCtl->ChnSwitchCnt[band_idx] = 0;
		pSafeChCtl->BandBanCnt[band_idx] = 0;
		pSafeChCtl->BandUpCnt[band_idx] = 0;
		pSafeChCtl->FailCnt[band_idx] = 0;
	}
}

/**
* ProcessSafeChannelChange - Process unsafe channel change.
* @pAd: pointer of the RTMP_ADAPTER
*
**/
VOID ProcessSafeChannelChange(RTMP_ADAPTER *pAd)
{
	UCHAR band_idx;

	SAFE_CHN_LOG_TRACE("\n%s: pAd=%p\n", __func__, pAd);
	for (band_idx = DBDC_BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		band_safe_chn_event_process(pAd, band_idx);
	}
	pAd->LteSafeChCtrl.SafeChnProcIntvl = UNSAFE_CHN_PROC_INTVL;
}

/**
* LteSafeBuildChnBitmask - Build available channel bit mask.
* @pAd: pointer of the RTMP_ADAPTER
* @band_idx: band index
*
* This function translate basic available channel list to bitmask, for safe channel process use.
*
**/
VOID LteSafeBuildChnBitmask(PRTMP_ADAPTER pAd, UINT8 band_idx)
{
	UCHAR ch_list[MAX_NUM_OF_CHANNELS], ch_num, idx, ch, ch_idx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	SAFE_CHN_LOG_TRACE("\n%s, pAd=%p, band_idx=%d\n", __func__, pAd, band_idx);

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
	if (!pChCtrl) {
		SAFE_CHN_LOG_ERR("%s, hc_get_channel_ctrl(band_idx=%d) failed!!\n",
			__func__, band_idx);
		return;
	}

	NdisZeroMemory(ch_list, sizeof(ch_list));
	for (idx = 0; idx < MAX_NUM_OF_CHANNELS; idx++) {
		ch = pChCtrl->ChList[idx].Channel;
		if (ch != 0) {
			ch_list[ch_idx] = ch;
			ch_idx++;
		}
	}
	ch_num = ch_idx;

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	safe_chn_list_to_bitmask(ch_list, ch_num, pAd->LteSafeChCtrl.AvaChnBitmask);
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
}

/**
* IsChannelSafe - Check whether the input channel is safe or not.
* @pAd: pointer of the RTMP_ADAPTER
* @channel: the channel number
*
* The return value is - TRUE if safe, FALSE if unsafe.
**/
BOOLEAN IsChannelSafe(PRTMP_ADAPTER pAd, UCHAR channel)
{
	BOOLEAN bSafe = FALSE;
	UCHAR ch_bit_pos, ch_type_idx;

	SAFE_CHN_LOG_TRACE("\n%s: channel=%d\n", __func__, channel);

	if (!get_chn_bitmask_pos(channel, &ch_type_idx, &ch_bit_pos)) {
		SAFE_CHN_LOG_ERR("%s, Channel(%d) out of range!!\n", __func__, channel);
		return TRUE;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);
	if (cmm_utl_is_bit_set(&(pAd->LteSafeChCtrl.SafeChnBitmask[ch_type_idx]), ch_bit_pos)) {
		bSafe = TRUE;
	}
	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	SAFE_CHN_LOG_TRACE("%s, Channel(%d) ch_type_idx(%d) ch_bit_pos(%d) is %s.\n",
		__func__, channel, ch_type_idx, ch_bit_pos, bSafe?"Safe":"Unsafe");

	return bSafe;
}

/**
* LteSafeChnEventHandle - Handle LTE safe channel event.
* @pAd: pointer of the RTMP_ADAPTER
* @channel_bit_mask: the channel bit mask
*
**/
VOID LteSafeChnEventHandle(RTMP_ADAPTER *pAd, UINT32 *channel_bit_mask)
{
	UCHAR chn_chg_type, band_chn_type, band_idx = 0;
	BOOLEAN bChanged;

	SAFE_CHN_LOG_FORCE("\n%s: pAd=%p, channel_bit_mask=%x-%x-%x\n",
		__func__, pAd, channel_bit_mask[0], channel_bit_mask[1], channel_bit_mask[2]);

	pAd->LteSafeChCtrl.RcvLteEventCnt++;
	bChanged = update_safe_chn_db(pAd, channel_bit_mask, &chn_chg_type);
	if (!bChanged)
		return;

	pAd->LteSafeChCtrl.SafeChnChgCnt++;
	for (band_idx = DBDC_BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		band_chn_type = get_channel_type_by_band(pAd, band_idx);

		SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d, band_chn_type=%d, chn_chg_type=%d\n",
			__func__, band_idx, band_chn_type, chn_chg_type);
		if (chn_chg_type & band_chn_type)
			set_safe_chn_chg_flag(pAd, band_idx);
	}
}

/**
* LteSafeChannelChangeProcess - Process lte safe channel change event after dequeue from cmd queue.
* @pAd: pointer of the RTMP_ADAPTER
* @CMDQelmt: band index
*
**/
NTSTATUS LteSafeChannelChangeProcess(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	BOOLEAN bUnsafe = TRUE, bOpDone = FALSE;
	BOOLEAN bFoundWdev = FALSE;
	UCHAR i, band_idx;
	struct wifi_dev *wdev;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR idx = 0;
#endif
	ENUM_LTE_SAFE_CH_OP_T safe_ch_op = LTE_SAFE_CH_OP_NO_CHNG;

	SAFE_CHN_LOG_TRACE("\n%s, pAd=%p\n", __func__, pAd);

	if (!pAd || !CMDQelmt) {
		SAFE_CHN_LOG_ERR("%s, Invalid input!\n", __func__);
		return STATUS_UNSUCCESSFUL;
   }

	NdisMoveMemory(&band_idx, CMDQelmt->buffer, sizeof(UCHAR));
	SAFE_CHN_LOG_TRACE("\n%s: band_idx=%d, dbdc_mode=%d\n", __func__, band_idx,
		pAd->CommonCfg.dbdc_mode);

	/* Get any proper wdev on the band;  */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			SAFE_CHN_LOG_TRACE("%s: type=%d, state=%d, band_idx=%d\n", __func__,
				wdev->wdev_type, wdev->if_up_down_state, HcGetBandByWdev(wdev));
			if ((wdev->wdev_type == WDEV_TYPE_AP)
				&& (wdev->if_up_down_state == TRUE)) {
				if (band_idx == HcGetBandByWdev(wdev)) {
					bFoundWdev = TRUE;
					break;
				}
			}
		}
	}

	if (!bFoundWdev) {
		SAFE_CHN_LOG_ERR("%s, No any active wdev (band_idx:%d).\n",
				 __func__, band_idx);
		return STATUS_SUCCESS;
	}

	/* TakeChannelOpCharge(); TBD */

	/* Check whether init ready */
	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		if (!pAd->LteSafeChCtrl.bAllUnsafe[band_idx]) {
			SAFE_CHN_LOG_TRACE("%s, wdev(%d) bss not ready (state:%d)!!\n",
					 __func__, wdev->wdev_idx, WDEV_BSS_STATE(wdev));
			goto end;
		} else
			SAFE_CHN_LOG_TRACE("%s: In all unsafe case, band_idx=%d\n",
			__func__, band_idx);
	}

	/* Check related features state, only do unsafe channel check/switch when all other featues' state is idle. */
	if (!is_chn_op_state_idle(pAd, wdev) || pAd->IoctlHandleFlag || pAd->ApCfg.iwpriv_event_flag) {
		SAFE_CHN_LOG_TRACE("%s: Other channel op is running on wdev(%d), IoctlHandleFlag=%d, iwpriv_event_flag=%d!!\n",
				 __func__, wdev->wdev_idx, pAd->IoctlHandleFlag, pAd->ApCfg.iwpriv_event_flag);
		goto end;
	}

	NdisAcquireSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	/* Classify the operation type */
	bUnsafe = is_all_available_chn_unsafe(pAd, band_idx);
	if (pAd->LteSafeChCtrl.bAllUnsafe[band_idx] && !bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE;
	else if (!pAd->LteSafeChCtrl.bAllUnsafe[band_idx] && bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE;
	else if (!pAd->LteSafeChCtrl.bAllUnsafe[band_idx] && !bUnsafe)
		safe_ch_op = LTE_SAFE_CH_OP_UNSAFE_SWITCH;

	NdisReleaseSpinLock(&pAd->LteSafeChCtrl.SafeChDbLock);

	/* Process unsafe: 1. All turns into unsafe; 2. Recover from all unsafe; 3. Switch to safe if current channel is unsafe; */
	SAFE_CHN_LOG_TRACE("%s: Channel operation type on wdev(%d) is %d.\n",
			 __func__, wdev->wdev_idx, safe_ch_op);
	switch (safe_ch_op) {
	case LTE_SAFE_CH_OP_RECOVER_FROM_ALL_UNSAFE:
		band_recover_from_all_unsafe(pAd, wdev, band_idx);
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
			wapp_send_band_status_event(pAd, wdev, TRUE);
		}
#endif
		break;
	case LTE_SAFE_CH_OP_TURN_INTO_ALL_UNSAFE:
#ifdef CONFIG_MAP_SUPPORT
	/*let AP stop happen without apcli disconnect at AP stop*/
		if (IS_MAP_ENABLE(pAd)) {
			int j;
			struct wifi_dev *sta_wdev = NULL;
			for (j = 0; j < MAX_APCLI_NUM; j++) {
				sta_wdev = &pAd->StaCfg[j].wdev;
#ifdef DFS_CAC_R2
				if (IS_MAP_TURNKEY_ENABLE(pAd))
					sta_wdev->quick_ch_change = FALSE;
#endif
				if (sta_wdev->channel == wdev->channel) {
					pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
				}
			}
		}
#endif
		band_turn_into_all_unsafe(pAd, wdev, band_idx);
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pAd->LteSafeChCtrl.SafeChnBitmask);
			wapp_send_band_status_event(pAd, wdev, FALSE);

			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				if (HcGetBandByWdev(&pAd->StaCfg[idx].wdev) == band_idx) {
					pAd->StaCfg[idx].ApcliInfStat.Enable = FALSE;
					ApCliIfDown(pAd);
				}
			}
		}
#endif
		break;
	case LTE_SAFE_CH_OP_UNSAFE_SWITCH:
#ifdef CONFIG_MAP_SUPPORT
		/*On lte detect let AP stop start happen without apcli disconnect at AP stop*/
		if (IS_MAP_ENABLE(pAd)) {
			int j;
			struct wifi_dev *sta_wdev = NULL;
			wdev->map_lte_unsafe_ch_detect = 1;
			for (j = 0; j < MAX_APCLI_NUM; j++) {
				sta_wdev = &pAd->StaCfg[j].wdev;
#ifdef DFS_CAC_R2
				if (IS_MAP_TURNKEY_ENABLE(pAd))
					sta_wdev->quick_ch_change = FALSE;
#endif
				if (sta_wdev->channel == wdev->channel) {
					pAd->StaCfg[j].ApcliInfStat.Enable = FALSE;
				}
			}
			wapp_send_band_status_event(pAd, wdev, TRUE);
		}
#endif
		switch_unsafe_channel(pAd, wdev, band_idx);
		break;
	default:
		break;
	}

	bOpDone = TRUE;

end:
	/* ReleaseChannelOpCharge(); TBD */

	/* Not done yet, enqueue again; */
	if (!bOpDone) {
		set_safe_chn_chg_flag(pAd, band_idx);
		pAd->LteSafeChCtrl.FailCnt[band_idx]++;
		SAFE_CHN_LOG_TRACE("%s(band_idx=%d), Not processed, line up again. \n", __func__, band_idx);
	}

	SAFE_CHN_LOG_TRACE("%s end. \n", __func__);

	return STATUS_SUCCESS;
}

/**
* Set_UnsafeChannel_Proc - Configure unsafe channel list.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: unsafe channel list (ex. 1:36:40)
*
* This function is for feature debug
*
**/
INT Set_UnsafeChannel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 unsafe_ch_cnt = 0;
	int i, buf_size;
	char *tmp = NULL;
	char *buf = NULL;
	EVENT_LTE_SAFE_CHN_T EventLteSafeChn;
	UCHAR *unsafe_ch_list;

	SAFE_CHN_LOG_TRACE("\n%s\n", __func__);
	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	buf_size = strlen(arg) * sizeof(RTMP_STRING);
	os_alloc_mem(NULL, (UCHAR **)&buf, buf_size);
	tmp = buf;
	strncpy(buf, arg, buf_size);
	while (strsep(&buf, ":"))
		unsafe_ch_cnt++;

	os_free_mem(tmp);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("Length of the unsafe channel list : %d\n", unsafe_ch_cnt));

	os_alloc_mem(NULL, (UCHAR **)&unsafe_ch_list, unsafe_ch_cnt*sizeof(UCHAR));
	for (i = 0; i < unsafe_ch_cnt; i++) {
		tmp = strsep(&arg, ":");
		if (tmp) {
			unsafe_ch_list[i] = (UINT8) os_str_tol(tmp, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("Channel %d\n", unsafe_ch_list[i]));
		}
	}

	NdisZeroMemory(&EventLteSafeChn, sizeof(EventLteSafeChn));
	unsafe_chn_list_to_bitmask(unsafe_ch_list, unsafe_ch_cnt, EventLteSafeChn.u4SafeChannelBitmask);
	ExtEventLteSafeChnHandler(pAd, (UINT8 *)&EventLteSafeChn, sizeof(EventLteSafeChn));

	os_free_mem(unsafe_ch_list);
	return TRUE;
}

/**
* Show_UnsafeChannel_Info - Display unsafe channel info
* @pAd: pointer of the RTMP_ADAPTER
* @arg: Null
*
* This function is for feature debug
*
**/
INT Show_UnsafeChannel_Info(PRTMP_ADAPTER	 pAd, RTMP_STRING *arg)
{
	UCHAR band_idx;

	SAFE_CHN_LOG_TRACE("\n%s\n", __func__);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("LTE unsafe channel Info\n---------\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("bQueryLteDone=%d, SafeChnProcIntvl=%d, RcvLteEventCnt=%d, SafeChnChgCnt=%d\n", 
			 pAd->LteSafeChCtrl.bQueryLteDone, pAd->LteSafeChCtrl.SafeChnProcIntvl, pAd->LteSafeChCtrl.RcvLteEventCnt, pAd->LteSafeChCtrl.SafeChnChgCnt));
	for (band_idx = DBDC_BAND0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("bAllUnsafe(band %d)=%d\n", band_idx, pAd->LteSafeChCtrl.bAllUnsafe[band_idx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("WaitForSafeChOpCnt(band %d)=%d\n", band_idx, pAd->LteSafeChCtrl.WaitForSafeChOpCnt[band_idx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ChnSwitchCnt(band %d)=%d\n", band_idx, pAd->LteSafeChCtrl.ChnSwitchCnt[band_idx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("BandBanCnt(band %d)=%d\n", band_idx, pAd->LteSafeChCtrl.BandBanCnt[band_idx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("BandUpCnt(band %d)=%d\n", band_idx, pAd->LteSafeChCtrl.BandUpCnt[band_idx]));
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("SafeChnBitmask=%x-%x-%x\n", pAd->LteSafeChCtrl.SafeChnBitmask[0],
			 pAd->LteSafeChCtrl.SafeChnBitmask[1], pAd->LteSafeChCtrl.SafeChnBitmask[2]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("AvaChnBitmask=%x-%x-%x\n", pAd->LteSafeChCtrl.AvaChnBitmask[0],
			 pAd->LteSafeChCtrl.AvaChnBitmask[1], pAd->LteSafeChCtrl.AvaChnBitmask[2]));

	return TRUE;
}

#endif

