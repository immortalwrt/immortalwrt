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
	ra_ac_q_mgmt.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"

#ifdef RED_SUPPORT
#define BADNODE_TIMER_PERIOD	100


DECLARE_TIMER_FUNCTION(red_badnode_timeout);
VOID red_badnode_timeout(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef VOW_SUPPORT
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#else
	UINT8 fgATCorWATFEnable = 0;
#endif
	UINT16 u2WlanIdx;

	for (u2WlanIdx = 0 ; u2WlanIdx < RED_STA_REC_NUM; u2WlanIdx++) {
		pEntry = &pAd->MacTab.Content[u2WlanIdx];
		if (IS_ENTRY_NONE(pEntry))
			continue;
		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid))
			continue;

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE)
			RedBadNode(u2WlanIdx, fgATCorWATFEnable, pAd);
	}

	UpdateTargetDelay(fgATCorWATFEnable, pAd);
}
BUILD_TIMER_FUNCTION(red_badnode_timeout);

VOID RedInit(PRTMP_ADAPTER pAd)
{
	int i, j;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	P_RED_AC_ElEMENT_T prAcElm = NULL;
	UINT32 red_en_type;

	if (pChipCap->MCUType & CR4)
		pAd->red_mcu_offload = TRUE;
	else
		pAd->red_mcu_offload = FALSE;

	/* Send cmd to enable N9 MPDU timer */
	if (pAd->red_mcu_offload)
		red_en_type = RED_BY_WA_ENABLE;
	else
		red_en_type = RED_BY_HOST_ENABLE;

	MtCmdSetRedEnable(pAd, HOST2N9, ((pAd->red_en > 0) ? red_en_type : RED_DISABLE));

	/* For 7615 (CR4 offload)*/
	if (pAd->red_mcu_offload) {
		MtCmdCr4Set(pAd, CR4_SET_ID_RED_ENABLE, pAd->red_en, 0);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: set CR4/N9 RED Enable to %d.\n", __func__, pAd->red_en));
	} else {
		if (pAd->red_en) {
			/*For 7622 (No CR4), need to initial the parameter on driver */
			for (i = 0; i < RED_STA_REC_NUM; i++) {
				prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;	/*us */
				prRedSta->u4Dropth = RED_HT_BW40_DEFAULT_THRESHOLD * QLEN_SCALED;
				prRedSta->ucMultiplyNum = RED_MULTIPLE_NUM_DEFAULT;
				prRedSta->u2DriverFRCnt = 0;
				prRedSta->tx_msdu_avg_cnt = 0;
				prRedSta->tx_msdu_cnt = 0;
				prAcElm = &prRedSta->arRedElm[0];

				for (j = WMM_AC_BK; j < WMM_NUM_OF_AC; j++) {
					prAcElm->u2TotalDropCnt = 0;
					prAcElm->u2DropCnt = 0;
					prAcElm->u2EnqueueCnt = 0;
					prAcElm->u2DequeueCnt = 0;
					prAcElm->u2qEmptyCnt = 0;
					prAcElm->ucShiftBit = 0;
					prAcElm->ucGBCnt = 0;
					prAcElm++;
				}

				prRedSta++;
			}
			RTMPInitTimer(pAd, &pAd->red_badnode_timer, GET_TIMER_FUNCTION(red_badnode_timeout), pAd, TRUE);
			RTMPSetTimer(&pAd->red_badnode_timer, BADNODE_TIMER_PERIOD);

		}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set Driver/N9 RED Enable to %d.\n", __func__, pAd->red_en));

	}

	/*
	if (pAd->red_mcu_offload) {
		red_qlen_drop_setting(pAd, 0);
	}
	*/

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RED Initiailize Done.\n", __func__));
}

VOID RedResetSta(UINT16 u2WlanIdx, UINT_8 ucMode, UINT_8 ucBW, RTMP_ADAPTER *pAd)
{
	P_RED_STA_T prRedSta = &pAd->red_sta[u2WlanIdx];
	/*P_STA_RECORD_T prStaRec = &g_arStaRec[u2WlanIdx];*/
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[u2WlanIdx];
	P_RED_AC_ElEMENT_T prAcElm = &pAd->red_sta[u2WlanIdx].arRedElm[0];
	UINT8 i;

	if (pEntry->TXBAbitmap != 0) {
		/*HT*/
		if (ucMode == MODE_HTMIX) {
			if (ucBW == BW_20)
				prRedSta->u4Dropth = RED_HT_BW20_DEFAULT_THRESHOLD * QLEN_SCALED;

			if (ucBW == BW_40)
				prRedSta->u4Dropth = RED_HT_BW40_DEFAULT_THRESHOLD * QLEN_SCALED;
		}

		/*VHT*/
		if (ucMode == MODE_VHT) {
			if (ucBW == BW_20)
				prRedSta->u4Dropth = RED_VHT_BW20_DEFAULT_THRESHOLD * QLEN_SCALED;

			if (ucBW == BW_40)
				prRedSta->u4Dropth = RED_VHT_BW40_DEFAULT_THRESHOLD * QLEN_SCALED;

			if (ucBW >= BW_80)
				prRedSta->u4Dropth = RED_VHT_BW80_DEFAULT_THRESHOLD * QLEN_SCALED;
		}
	} else {
		/*Legacy */
		prRedSta->u4Dropth = RED_LEGACY_DEFAULT_THRESHOLD * QLEN_SCALED;
	}

	for (i = WMM_AC_BK; i < WMM_NUM_OF_AC; i++) {
		prAcElm->u2qEmptyCnt = 0;
		prAcElm->ucShiftBit = 0;
		prAcElm->u2DropCnt = 0;
		prAcElm->ucGBCnt = 0;
		prAcElm++;
	}
}

VOID RedBadNode(UINT16 u2WlanIdx, UINT8 ATC_WATF_Enable, RTMP_ADAPTER *pAd)
{
	P_RED_AC_ElEMENT_T prAcElm = &pAd->red_sta[u2WlanIdx].arRedElm[0];
	P_RED_STA_T prRedSta = &pAd->red_sta[u2WlanIdx];
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[u2WlanIdx];
	UINT8 i;
	UINT8 ucGoodNodeCnt = 0;
	UINT8 ucBadNodeCnt = 0;
	UINT8 ucIsBadNode = FALSE;
	UINT8 ucIsSetDefault = FALSE;
	UINT16 pkt_buf_cnt;

	for (i = WMM_AC_BK; i < WMM_NUM_OF_AC; i++) {
		ucBadNodeCnt = (prAcElm->ucGBCnt & RED_BAD_NODE_CNT_MASK);
		ucGoodNodeCnt = (prAcElm->ucGBCnt & RED_GOOD_NODE_CNT_MASK) >> RED_GOOD_NODE_CNT_SHIFT_BIT;
		ucIsBadNode = (prAcElm->ucGBCnt & RED_IS_BAD_NODE_MASK) >> RED_IS_BAD_NODE_SHIFT_BIT;

		/* Check Is Bad Node or not */
		pkt_buf_cnt = prAcElm->u2EnqueueCnt - prAcElm->u2DequeueCnt;
		if (pkt_buf_cnt >= (prRedSta->u4Dropth >> prAcElm->ucShiftBit)) {

		/* Drop packet immediately if ATC for WATF enable to prevent slow client occupy much PLE buffer.*/
			if (ATC_WATF_Enable)
				ucBadNodeCnt = RED_MAX_BAD_NODE_CNT;
			else
				ucBadNodeCnt++;

			ucGoodNodeCnt = 0;
		} else {
			/* Good Node */
			ucGoodNodeCnt++;
			ucBadNodeCnt = 0;
		}

		if ((prAcElm->ucShiftBit > 0) &&
			(prRedSta->u4Dropth >> prAcElm->ucShiftBit) <= (RED_BAD_NODE_DROP_THRESHOLD * QLEN_SCALED)) {
			if (pEntry->TXBAbitmap != 0)
				/*HT and VHT */
				prRedSta->u4Dropth = (RED_BAD_NODE_DROP_THRESHOLD * QLEN_SCALED) << prAcElm->ucShiftBit;
			else
				/*Legacy */
				prRedSta->u4Dropth = (RED_BAD_NODE_LEGACY_DEFAULT_THRESHOLD * QLEN_SCALED) << prAcElm->ucShiftBit;

			ucIsSetDefault = TRUE;
		}

		if (ucBadNodeCnt >= RED_MAX_BAD_NODE_CNT) {
			ucBadNodeCnt = 0;
			ucIsBadNode = TRUE;
			if (ATC_WATF_Enable && !ucIsSetDefault)
				prAcElm->ucShiftBit++;
		}

		if (ucGoodNodeCnt >= RED_MAX_GOOD_NODE_CNT) {
			ucGoodNodeCnt = 0;

			if (prAcElm->ucShiftBit > 0)
				prAcElm->ucShiftBit--;
			else	/*ucShiftBit == 0 */
				ucIsBadNode = FALSE;
		}

/*
ucGBCnt
 ----------------------------------------------------
Bits |       7          |       6|      5|      4       |       3|      2|      1|      0       |
      |IsBadNode   |GoodNodeCnt                |       BadNodeCnt                    |
----------------------------------------------------
*/
		prAcElm->ucGBCnt = ((ucIsBadNode << RED_IS_BAD_NODE_SHIFT_BIT) |
							(ucGoodNodeCnt << RED_GOOD_NODE_CNT_SHIFT_BIT) |
							(ucBadNodeCnt));
		prAcElm++;
	}
}

bool RedMarkPktDrop(UINT16 u2WlanIdx, UINT8 ucQidx, PRTMP_ADAPTER pAd)
{
	UINT8 ucAC = (ucQidx % WMM_NUM_OF_AC);
	P_RED_AC_ElEMENT_T prAcElm = &pAd->red_sta[u2WlanIdx].arRedElm[ucAC];
	P_RED_STA_T prRedSta = &pAd->red_sta[u2WlanIdx];
	UINT8 IsBadNode;
	UINT16 pkt_buf_cnt = prAcElm->u2EnqueueCnt - prAcElm->u2DequeueCnt;

	/* If WlanIdx is invaild, then drop this packet */
	if (!IS_WCID_VALID(pAd, u2WlanIdx))
		return TRUE;

	/*
	   In following condition, we don't drop traffic :
	   1. RED Disable
	   2. For WMM detection and number of InUseSta is less than 2.
	   3. Only one station is connect.
	 */
	if ((pAd->red_en == FALSE)
		|| ((pAd->is_on) && (pAd->red_in_use_sta <= 2))
		|| (pAd->red_in_use_sta <= 1)) {
		/*Set IsBadNode(bit 7) to FALSE */
		prRedSta->arRedElm[0].ucGBCnt &= 0x7F;
		prRedSta->arRedElm[1].ucGBCnt &= 0x7F;
		prRedSta->arRedElm[2].ucGBCnt &= 0x7F;
		prRedSta->arRedElm[3].ucGBCnt &= 0x7F;
	}

	IsBadNode = (prAcElm->ucGBCnt & RED_IS_BAD_NODE_MASK) >> RED_IS_BAD_NODE_SHIFT_BIT;

	if (pkt_buf_cnt == 0)
		prAcElm->u2qEmptyCnt++;

	if (IsBadNode) {
		/* Drop packet if qlen over Drop_threshold */
		if (pkt_buf_cnt >= (prRedSta->u4Dropth >> prAcElm->ucShiftBit)) {
			prAcElm->u2DropCnt++;
			prAcElm->u2TotalDropCnt++;
			return TRUE;
		}
	}

	return FALSE;
}


bool red_mark_pktdrop_cr4(UINT16 u2WlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd)
{
	return FALSE;
}

VOID red_record_data(PRTMP_ADAPTER pAd, UINT16 u2WlanIdx, PNDIS_PACKET pPacket)
{
	UINT8 ucAC;
	P_RED_AC_ElEMENT_T prAcElm;

	if (!RTMP_GET_PACKET_MGMT_PKT(pPacket)) {
		ucAC = RTMP_GET_PACKET_QUEIDX(pPacket);
		prAcElm = &pAd->red_sta[u2WlanIdx].arRedElm[ucAC];
		prAcElm->u2EnqueueCnt++;
	}
}

VOID UpdateThreshold(UINT16 u2WlanIdx, RTMP_ADAPTER *pAd)
{
	P_RED_STA_T prRedSta = &pAd->red_sta[u2WlanIdx];
	UINT16 u2Dropth = 0;

	if (prRedSta->i4MpduTime == 0)
		prRedSta->i4MpduTime = 1;

	u2Dropth = pAd->red_targetdelay / prRedSta->i4MpduTime;

	if (u2Dropth <= RED_DROP_TH_LOWER_BOUND)
		u2Dropth = RED_DROP_TH_LOWER_BOUND;
	else if (u2Dropth >= RED_DROP_TH_UPPER_BOUND)
		u2Dropth = RED_DROP_TH_UPPER_BOUND;

	prRedSta->u4Dropth = (u2Dropth * prRedSta->ucMultiplyNum * QLEN_SCALED / 10);
}

VOID UpdateAirtimeRatio(UINT16 u2WlanIdx, UINT8 ucAirtimeRatio, UINT8 ATC_WATF_Enable, RTMP_ADAPTER *pAd)
{
	P_RED_STA_T prRedSta = &pAd->red_sta[u2WlanIdx];
	UINT8 ucNum;
	UINT8 ucIndex;

	if (ATC_WATF_Enable
		&& (prRedSta->u4Dropth >= RED_BAD_NODE_DROP_THRESHOLD * QLEN_SCALED)) {
		ucIndex = ucAirtimeRatio / 10;
		ucNum = ucIndex * 15 / 10;
	} else
		ucNum = 15;

	prRedSta->ucMultiplyNum = 15 + ucNum;
}

VOID UpdateTargetDelay(UINT8 ATC_WATF_Enable, RTMP_ADAPTER *pAd)
{
	if (ATC_WATF_Enable)
		pAd->red_targetdelay = pAd->red_atm_on_targetdelay;
	else
		pAd->red_targetdelay = pAd->red_atm_off_targetdelay;
}
VOID appShowRedDebugMessage(RTMP_ADAPTER *pAd)
{
	UINT16 uRedStaNum, i;
	UINT8 j;
	P_RED_STA_T prRedSta;
	P_RED_AC_ElEMENT_T prAcElm;
	UINT16 pkt_buf_cnt;
	/* Show Debug Message by cmd */
	uRedStaNum = pAd->red_sta_num;
	prRedSta = &pAd->red_sta[1];

	for (i = 1; i <= uRedStaNum; i++) {
		prAcElm = &prRedSta->arRedElm[WMM_AC_BK];

		for (j = WMM_AC_BK; j <= WMM_AC_VO; j++) {
			/*Only if EnqueueCnt >0 or DropCnt >0, show this AC's message */
			pkt_buf_cnt = prAcElm->u2EnqueueCnt - prAcElm->u2DequeueCnt;
			if ((pkt_buf_cnt > 0) || (prAcElm->u2DropCnt > 0)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("STA %d, AC %d, Len %d, Thres %d, Drop %d, ShiftBit %d, qEmpty %d, ATRatio %d, G/Bcnt %d/%d, IsBad %d, msdu %d msducnt (%d %d)\n",
					  (i),
					  (j),
					  (pkt_buf_cnt),
					  (prRedSta->u4Dropth / QLEN_SCALED) >> prAcElm->ucShiftBit,
					  (prAcElm->u2DropCnt),
					  (prAcElm->ucShiftBit),
					  (prAcElm->u2qEmptyCnt),
					  (prRedSta->ucMultiplyNum),
					  (prAcElm->ucGBCnt & RED_GOOD_NODE_CNT_MASK) >> 4,
					  (prAcElm->ucGBCnt & RED_BAD_NODE_CNT_MASK),
					  (prAcElm->ucGBCnt & RED_IS_BAD_NODE_MASK) >> 7,
					  (prRedSta->i4MpduTime),
					  (prRedSta->tx_msdu_avg_cnt),
					  (prRedSta->tx_msdu_cnt)
					 ));
				prAcElm->u2DropCnt = 0;
				prAcElm->u2qEmptyCnt = 0;
			}
			prAcElm++;
		}

		prRedSta++;
	}
}

VOID RedSetTargetDelay(INT16 i2TarDelay, PRTMP_ADAPTER pAd)
{
#ifdef VOW_SUPPORT
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#else
	UINT8 fgATCorWATFEnable = 0;
#endif
	if (fgATCorWATFEnable)
		pAd->red_atm_on_targetdelay = i2TarDelay;
	else
		pAd->red_atm_off_targetdelay = i2TarDelay;
}

VOID RedCalForceRateRatio(UINT16 u2Wcid, UINT16 u2N9ARCnt, UINT16 u2N9FRCnt, RTMP_ADAPTER *pAd)
{
	P_RED_STA_T prRedSta = &pAd->red_sta[u2Wcid];
	P_RED_AC_ElEMENT_T prAcElm = &prRedSta->arRedElm[0];
	UINT16 u2TxTotalCnt = 0;
	UINT8 ucRatio = 0;
	UINT8 ucIsBadNode = FALSE;
	UINT8 i;

	for (i = WMM_AC_BK; i < WMM_NUM_OF_AC; i++) {
		ucIsBadNode |= (prAcElm->ucGBCnt & RED_IS_BAD_NODE_MASK) >> RED_IS_BAD_NODE_SHIFT_BIT;
		prAcElm++;
	}

	if (ucIsBadNode == FALSE) {
		/* TotalCnt = AutoRateCnt(N9) + ForceRateCnt(N9) +  ForceRateCnt(Driver) */
		u2TxTotalCnt = u2N9ARCnt + u2N9FRCnt + prRedSta->u2DriverFRCnt;
		/* Ratio = [ForceRateCnt(N9) +  ForceRateCnt(Driver)]/TotalCnt */
		if (u2TxTotalCnt > 0)
		ucRatio = (u2N9FRCnt + prRedSta->u2DriverFRCnt) * 100 / u2TxTotalCnt;

		if (ucRatio >= FORCE_RATIO_THRESHOLD)
			prRedSta->i4MpduTime = -1;
	}

	prRedSta->u2DriverFRCnt = 0;
}

VOID red_tx_free_handle(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UINT8 ucAC;
	UINT16 u2WlanIdx;
	P_RED_AC_ElEMENT_T prAcElm;

	/* red only handles data packets */
	if (!RTMP_GET_PACKET_MGMT_PKT(pkt)) {
		ucAC = RTMP_GET_PACKET_QUEIDX(pkt);
		u2WlanIdx = RTMP_GET_PACKET_WCID(pkt);
		pAd->red_sta[u2WlanIdx].tx_msdu_cnt++;
		prAcElm = &pAd->red_sta[u2WlanIdx].arRedElm[ucAC];
		prAcElm->u2DequeueCnt++;
	}
}

VOID RedRecordForceRateFromDriver(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	P_RED_STA_T prRedSta = &pAd->red_sta[wcid];
	prRedSta->u2DriverFRCnt++;
}

/* Command */

INT set_red_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv;
	BOOLEAN Cancelled;
	UINT32 red_en_type;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if ((rv > 0) && (en <= 1)) {
			pAd->red_en = en;
			/* to CR4 & N9 */
			if (pAd->red_mcu_offload)
				red_en_type = RED_BY_WA_ENABLE;
			else
				red_en_type = RED_BY_HOST_ENABLE;
#ifdef FQ_SCH_SUPPORT
			if (!(pAd->fq_ctrl.enable & FQ_READY) || (pAd->red_en == 1))
#endif
				MtCmdSetRedEnable(pAd, HOST2N9, ((en > 0) ? red_en_type : RED_DISABLE));

			if (en == 0) {
				if (pAd->red_badnode_timer.Valid)
					RTMPReleaseTimer(&pAd->red_badnode_timer, &Cancelled);
			} else {
				if (!pAd->red_badnode_timer.Valid) {
					RTMPInitTimer(pAd, &pAd->red_badnode_timer,
						GET_TIMER_FUNCTION(red_badnode_timeout), pAd, TRUE);
					RTMPSetTimer(&pAd->red_badnode_timer, BADNODE_TIMER_PERIOD);
				}
			}

			if (pAd->red_mcu_offload) {
				MtCmdCr4Set(pAd, CR4_SET_ID_RED_ENABLE, pAd->red_en, 0);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set CR4/N9 RED Enable to %d.\n", __func__, pAd->red_en));
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set Driver/N9 RED Enable to %d.\n", __func__, pAd->red_en));
			}
		} else if (en == 2) {
			if (pAd->red_mcu_offload)
				red_qlen_drop_setting(pAd, 0);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_red_target_delay(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 rv, tarDelay;

	if (arg) {
		rv = sscanf(arg, "%d", &tarDelay);

		if ((rv > 0) && (tarDelay >= 1) && (tarDelay <= 32767)) {
			if (pAd->red_mcu_offload) {
				MtCmdCr4Set(pAd, CR4_SET_ID_RED_TARGET_DELAY, tarDelay, 0);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set CR4 RED TARGET_DELAY to %d.\n", __func__, tarDelay));
			} else {
				RedSetTargetDelay(tarDelay, pAd);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set Driver RED TARGET_DELAY to %d.\n", __func__, tarDelay));
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_red_show_sta(PRTMP_ADAPTER pAd,	RTMP_STRING *arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			if (pAd->red_mcu_offload) {
				MtCmdCr4Set(pAd, CR4_SET_ID_RED_SHOW_STA, sta, 0);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set CR4 RED show sta to %d.\n", __func__, sta));
			} else {
				pAd->red_sta_num = sta;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: set Driver RED show sta to %d.\n", __func__, sta));
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_red_debug_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if ((pAd->red_mcu_offload == FALSE) && (rv > 0) && (en <= 1)) {
			pAd->red_debug_en = en;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: set RED Debug Message Enable to %d.\n", __func__, pAd->red_debug_en));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT show_red_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT16 uRedStaNum, i, j;
	P_RED_AC_ElEMENT_T prAcElm;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	uRedStaNum = pAd->red_sta_num;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("======== RED(per-STA Tail Drop) Information ========\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("RED Enbale: %d\n", pAd->red_en));

	if (pAd->red_mcu_offload == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("RED Target Delay: %d(us)\n", pAd->red_targetdelay));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("RED Monitor STA: %d\n", pAd->red_sta_num));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Dump RED Total Drop Count:\n"));
	for (i = 1; i <= (wtbl_max_num - MAX_MBSSID_NUM(pAd)); i++) {
		tr_entry = &tr_ctl->tr_entry[i];
		if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
			continue;
		prAcElm = &(((P_RED_STA_T)&(pAd->red_sta[i]))->arRedElm[WMM_AC_BK]);
		for (j = WMM_AC_BK; j <= WMM_AC_VO; j++, prAcElm++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("STA%d[AC%d]:%u \n",
				i, j, prAcElm->u2TotalDropCnt));
	}

	return TRUE;
}

INT set_red_dump_reset(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 i, j;
	P_RED_STA_T prRedSta;
	P_RED_AC_ElEMENT_T prAcElm;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	prRedSta = &pAd->red_sta[0];

	for (i = 0; i < wtbl_max_num; i++) {
		prAcElm = &prRedSta->arRedElm[WMM_AC_BK];
		for (j = WMM_AC_BK; j <= WMM_AC_VO; j++, prAcElm++)
			prAcElm->u2TotalDropCnt = 0;
		prRedSta++;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RED dump reset\n", __func__));


	return TRUE;
}

INT set_red_drop(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 cmd = 3, rv, ret = 1;
	UINT32 param[2] = {0};

	if (!pAd->red_mcu_offload)
		return ret;

	MtCmdFwLog2Host(pAd, 1, 2);

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &cmd, &param[0], &param[1]);

		if (rv == 0)
			cmd = 3;

		switch (cmd) {
		case 0:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set band%u total token:%u\n", __func__, param[0], param[1]));
			MtCmdCr4Set(pAd, WA_SET_OPTION_RED_QLEN_DROP_TOKEN, param[0], param[1]);
			break;
		case 1:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set band%u free token bound:%u\n", __func__, param[0], param[1]));
			MtCmdCr4Set(pAd, WA_SET_OPTION_RED_QLEN_DROP_FREE_BOUND, param[0], param[1]);
			break;
		case 2:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set band%u qlen threshold:%u/100\n", __func__, param[0], param[1]));
			MtCmdCr4Set(pAd, WA_SET_OPTION_RED_QLEN_DROP_THRESHOLD, param[0], param[1]);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: qlen drop dump\n", __func__));
			MtCmdCr4Set(pAd, WA_SET_OPTION_RED_QLEN_DROP_DUMP, 0, 0);
			break;
		}
	} else
		MtCmdCr4Set(pAd, WA_SET_OPTION_RED_QLEN_DROP_DUMP, 0, 0);

	MtCmdFwLog2Host(pAd, 1, 0);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RED set drop config\n", __func__));

	return ret;
}

VOID red_qlen_drop_setting(PRTMP_ADAPTER pAd, UINT8 op)
{
	struct pci_hif_chip *hif_chip = NULL;
	struct _PCI_HIF_T *pci_hif = NULL;
	struct hif_pci_tx_ring *tx_ring = NULL;
	RTMP_CHIP_CAP *pChipCap = NULL;
	UINT8 tx_res_num = 0, ctxd_num;
	UCHAR buf[32] = {0};
	UINT8 band_num = 1, ucBandIdx = 0, threshold = 100;
	UINT16 token_cnt = 0, ring_occupy = 0, upbound = 0;
	INT32 i;
	BOOLEAN fgDisable = TRUE;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	hif_chip = pci_hif->main_hif_chip;
	pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	tx_res_num = pci_hif->tx_res_num;

	if ((!pAd->CommonCfg.dbdc_mode) && (WMODE_CAP_2G(wdev->PhyMode)))
		return;

	if (pAd->CommonCfg.dbdc_mode)
		band_num = 2;
	else
		band_num = 1;

#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	ctxd_num = hif_chip->max_ctxd_agg_num;
#else
	ctxd_num = 1;
#endif

	for (ucBandIdx = 0; ucBandIdx < band_num; ucBandIdx++) {
		token_cnt = pChipCap->tkn_info.token_tx_cnt;
		upbound = token_cnt;
		threshold = 100;

		/* find out tx data ring's resource index */
		for (i = 0; i < tx_res_num; i++) {
			tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);
			if (tx_ring->ring_attr == HIF_TX_DATA && tx_ring->band_idx == ucBandIdx)
				break;
		}
		if (i == tx_res_num)
			continue;

		fgDisable = TRUE;
#ifdef WHNAT_SUPPORT
		token_cnt = pAd->CommonCfg.whnat_en ? pChipCap->tkn_info.hw_tx_token_cnt : token_cnt;
		ring_occupy = pAd->CommonCfg.whnat_en ? (1024*2) : ((tx_ring->ring_size*ctxd_num)*1)/2;
#else
		ring_occupy = ((tx_ring->ring_size*ctxd_num)*1)/2;
#endif
		switch (ucBandIdx) {
		case BAND0:
			/* if 5G+5G : TBD */
			if (pAd->CommonCfg.dbdc_mode)
				continue;
			else {
				if (token_cnt > ring_occupy) {
				token_cnt -= ring_occupy;
				threshold = 70;
				upbound = (token_cnt > 0) ? token_cnt >> 2 : 0;
					fgDisable = FALSE;
				}
			}
			break;
		case BAND1:
			if (!pAd->CommonCfg.dbdc_mode)
				continue;
			if (token_cnt > (pChipCap->tkn_info.band0_token_cnt + ring_occupy)) {
			token_cnt -= (pChipCap->tkn_info.band0_token_cnt + ring_occupy);
			threshold = 70;
			upbound = (token_cnt > 0) ? token_cnt >> 2 : 0;
				fgDisable = FALSE;
			}

			break;
		default:
			break;
		}

		if ((op == 1) || fgDisable) {
			token_cnt = 8192;
			upbound = 0;
			threshold = 100;
		}

		sprintf(buf, "0-%u-%u\n", ucBandIdx, token_cnt);
		set_red_drop(pAd, buf);
		sprintf(buf, "1-%u-%u\n", ucBandIdx, upbound);
		set_red_drop(pAd, buf);
		sprintf(buf, "2-%u-%u\n", ucBandIdx, threshold);
		set_red_drop(pAd, buf);
	}


}
#endif /* RED_SUPPORT */

