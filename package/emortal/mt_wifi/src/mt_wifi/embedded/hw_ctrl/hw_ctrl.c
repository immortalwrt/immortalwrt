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
	hw_ctrl.c
*/
#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"
#include "hw_ctrl/cmm_chip.h"

static NTSTATUS HwCtrlUpdateRtsThreshold(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct rts_thld *rts = (struct rts_thld *)CMDQelmt->buffer;

	AsicUpdateRtsThld(pAd, rts->wdev, rts->pkt_thld, rts->len_thld);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlUpdateProtect(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct prot_info *prot = (struct prot_info *)CMDQelmt->buffer;
	AsicUpdateProtect(pAd, prot);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlSetClientMACEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
	AsicUpdateRxWCIDTable(pAd, (UINT16)pInfo->WCID, pInfo->Addr, pInfo->IsBMC, pInfo->IsReset);
	return NDIS_STATUS_SUCCESS;
}


#ifdef TXBF_SUPPORT
static NTSTATUS HwCtrlSetClientBfCap(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMAC_TABLE_ENTRY pMacEntry;

	pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
	AsicUpdateClientBfCap(pAd, pMacEntry);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetBfRepeater(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMAC_TABLE_ENTRY pMacEntry;

	pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
#ifdef MAC_REPEATER_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	AsicTxBfReptClonedStaToNormalSta(pAd, pMacEntry->wcid, pMacEntry->pReptCli->CliIdx);
#endif /* CONFIG_AP_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAdjBfSounding(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_BF_ADJ prMtStaBfAdj = NULL;
	UCHAR ucConnState;
	struct wifi_dev *wdev = NULL;

	prMtStaBfAdj = (PMT_STA_BF_ADJ)CMDQelmt->buffer;

	if (prMtStaBfAdj) {
		ucConnState = prMtStaBfAdj->ConnectionState;
		wdev = prMtStaBfAdj->wdev;
#ifdef CONFIG_STA_SUPPORT
		mt_BfSoundingAdjust(pAd, ucConnState, wdev);
#endif /* CONFIG_STA_SUPPORT */
	}

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlTxBfTxApply(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PUCHAR pTxBfApply;

	pTxBfApply = (PUCHAR)CMDQelmt->buffer;
#ifdef BACKGROUND_SCAN_SUPPORT
	BfSwitch(pAd, *pTxBfApply);
#endif
	return NDIS_STATUS_SUCCESS;
}

#endif /* TXBF_SUPPORT */


static NTSTATUS HwCtrlDelAsicWcid(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;

	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (!IS_WCID_VALID(pAd, SetAsicWcid.WCID) && (SetAsicWcid.WCID != WCID_ALL))
		return NDIS_STATUS_FAILURE;

	AsicDelWcidTab(pAd, SetAsicWcid.WCID);
	return NDIS_STATUS_SUCCESS;
}


#ifdef HTC_DECRYPT_IOT
static NTSTATUS HwCtrlSetAsicWcidAAD_OM(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;

	SetAsicAAD_OM = *((PRT_SET_ASIC_AAD_OM)(CMDQelmt->buffer));
	AsicSetWcidAAD_OM(pAd, SetAsicAAD_OM.WCID, SetAsicAAD_OM.Value);
	return NDIS_STATUS_SUCCESS;
}
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
static NTSTATUS HwCtrlUpdate4Addr_HdrTrans(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_ASIC_4ADDR_HDR_TRANS Update_4Addr_Hdr_Trans;
	Update_4Addr_Hdr_Trans = *((PRT_ASIC_4ADDR_HDR_TRANS)(CMDQelmt->buffer));

	AsicSetWcid4Addr_HdrTrans(pAd, Update_4Addr_Hdr_Trans.Wcid, Update_4Addr_Hdr_Trans.Enable);

	return NDIS_STATUS_SUCCESS;
}
#endif

static void update_txop_level(UINT16 *dst, UINT16 *src,
							  UINT32 bitmap, UINT32 len)
{
	UINT32 prio;

	for (prio = 0; prio < len; prio++) {
		if (bitmap & (1 << prio)) {
			if (*(dst + prio) < *(src + prio))
				*(dst + prio) = *(src + prio);
		}
	}
}


static void tx_burst_arbiter(struct _RTMP_ADAPTER *pAd,
							 struct wifi_dev *curr_wdev,
							 UCHAR bss_idx)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	UINT32 idx = 0;
	UINT32 _prio_bitmap = 0;
	UINT16 txop_level = TXOP_0;
	UINT16 _txop_level[MAX_PRIO_NUM] = {0};
	UINT8 prio;
	UINT8 curr_prio = PRIO_DEFAULT;
	EDCA_PARM *edca_param = NULL;
	UCHAR wmm_idx = 0;

	edca_param = HcGetEdca(pAd, curr_wdev);

	if (edca_param == NULL)
		return;

	wmm_idx = HcGetWmmIdx(pAd, curr_wdev);

	/* judge the final prio bitmap for specific BSS */
	do {
		if (wdev[idx] == NULL)
			break;

		if (wdev[idx]->bss_info_argument.ucBssIndex == bss_idx) {
			_prio_bitmap |= wdev[idx]->prio_bitmap;
			update_txop_level(_txop_level, wdev[idx]->txop_level,
							  _prio_bitmap, MAX_PRIO_NUM);
		}

		idx++;
	} while (idx < WDEV_NUM_MAX);

	/* update specific BSS's prio bitmap & txop_level array */
	curr_wdev->bss_info_argument.prio_bitmap = _prio_bitmap;
	memcpy(curr_wdev->bss_info_argument.txop_level, _txop_level,
		   (sizeof(UINT16) * MAX_PRIO_NUM));

	/* find the highest prio module */
	for (prio = 0; prio < MAX_PRIO_NUM; prio++) {
		if (_prio_bitmap & (1 << prio))
			curr_prio = prio;
	}

	txop_level = curr_wdev->bss_info_argument.txop_level[curr_prio];

	if (pAd->CommonCfg.bEnableTxBurst)
		AsicSetWmmParam(pAd, wmm_idx, WMM_AC_BE, WMM_PARAM_TXOP, txop_level);
}

static void set_tx_burst(struct _RTMP_ADAPTER *pAd, struct _tx_burst_cfg *txop_cfg)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = NULL;
	UCHAR bss_idx = 0;

	if (txop_cfg->enable) {
		txop_cfg->wdev->prio_bitmap |= (1 << txop_cfg->prio);
		txop_cfg->wdev->txop_level[txop_cfg->prio] = txop_cfg->txop_level;
	} else
		txop_cfg->wdev->prio_bitmap &= ~(1 << txop_cfg->prio);

	bss_info = &txop_cfg->wdev->bss_info_argument;
	bss_idx = bss_info->ucBssIndex;
	tx_burst_arbiter(pAd, txop_cfg->wdev, bss_idx);
}

void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;

	if (wdev == NULL)
		return;

	txop_cfg.wdev = wdev;
	txop_cfg.prio = prio;
	txop_cfg.ac_type = ac_type;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("<caller: %pS>\n -%s: prio=%x, level=%x, enable=%x\n",
			  __builtin_return_address(0), __func__,
			  prio, level, enable));
	set_tx_burst(pAd, &txop_cfg);
}


static NTSTATUS HwCtrlSetTxBurst(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _tx_burst_cfg *txop_cfg = (struct _tx_burst_cfg *)CMDQelmt->buffer;

	if (txop_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	set_tx_burst(pAd, txop_cfg);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetPartWmmParam(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _part_wmm_cfg *part_wmm_cfg = (struct _part_wmm_cfg *)CMDQelmt->buffer;

	if (part_wmm_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	AsicSetWmmParam(pAd, part_wmm_cfg->wmm_idx, part_wmm_cfg->ac_num, part_wmm_cfg->edca_type, part_wmm_cfg->edca_value);
	return NDIS_STATUS_SUCCESS;
}
#ifdef DABS_QOS
static NTSTATUS HwCtrlSetQosParam(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct qos_param_set_del *pqos_param_set_del = (struct qos_param_set_del *)CMDQelmt->buffer;
	UINT32 idx = pqos_param_set_del->idx;
	BOOLEAN sel_del = pqos_param_set_del->sel_del;

	if (idx >= MAX_QOS_PARAM_TBL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: idx[%d] >= MAX_QOS_PARAM_TBL!!! sel_del[%d]\n", __func__, idx, sel_del));
		return NDIS_STATUS_FAILURE;
	}

	if (sel_del == TRUE) {
		enable_qos_param_tbl_by_idx(idx);
		if (set_qos_param_to_fw(pAd, idx, TRUE) == FALSE) {
			disable_qos_param_tbl_by_idx(idx);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Set to fw fail!!! idx[%d], sel_del[%d]!!!\n", __func__, idx, sel_del));
			return NDIS_STATUS_FAILURE;
		}
	} else {
		if (set_qos_param_to_fw(pAd, idx, FALSE) == FALSE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Set to fw fail!!! idx[%d], sel_del[%d]!!!\n", __func__, idx, sel_del));
			return NDIS_STATUS_FAILURE;
		} else {
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			memset(&qos_param_table[idx], 0, sizeof(struct qos_param_rec));
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlAPAdjustEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME\n"));
		RTMP_IO_WRITE32(pAd->hdev_ctrl, EXP_ACK_TIME, 0x005400ca);
	}
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlAPRecoverEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME\n"));
		RTMP_IO_WRITE32(pAd->hdev_ctrl, EXP_ACK_TIME, 0x002400ca);
	}
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


static NTSTATUS HwCtrlUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d)\n", __func__, __LINE__));
	asic_update_raw_counters(pAd);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateMibCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d)\n", __func__, __LINE__));
	asic_update_mib_bucket(pAd);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAddRemoveKeyTab(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	ASIC_SEC_INFO *pInfo;

	pInfo = (PASIC_SEC_INFO) CMDQelmt->buffer;
	AsicAddRemoveKeyTab(pAd, pInfo);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MAC_REPEATER_SUPPORT
static NTSTATUS HwCtrlAddReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PADD_REPT_ENTRY_STRUC pInfo;
	struct wifi_dev *wdev = NULL;
	UCHAR *pAddr = NULL;

	pInfo = (PADD_REPT_ENTRY_STRUC)CMDQelmt->buffer;
	wdev = pInfo->wdev;
	pAddr = pInfo->arAddr;
	RTMPInsertRepeaterEntry(pAd, wdev, pAddr);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRemoveReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PREMOVE_REPT_ENTRY_STRUC pInfo;
	UCHAR CliIdx;

	pInfo = (PREMOVE_REPT_ENTRY_STRUC)CMDQelmt->buffer;
	CliIdx = pInfo->CliIdx;
	RTMPRemoveRepeaterEntry(pAd, CliIdx);
	return NDIS_STATUS_SUCCESS;
}
#endif /*MAC_REPEATER_SUPPORT*/

#ifdef MT_MAC
#ifdef OCE_SUPPORT
static NTSTATUS HwCtrlSetFdFrameOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_FD_FRAME_OFFLOAD pSetFdFrameOffload = (PMT_SET_FD_FRAME_OFFLOAD)CMDQelmt->buffer;
	P_CMD_FD_FRAME_OFFLOAD_T pFd_frame_offload = NULL;
	struct wifi_dev *wdev = pAd->wdev_list[pSetFdFrameOffload->WdevIdx];

	os_alloc_mem(NULL, (PUCHAR *)&pFd_frame_offload, sizeof(CMD_FD_FRAME_OFFLOAD_T));

	if (!pFd_frame_offload) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("can not allocate fd_frame_offload\n"));
			return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(pFd_frame_offload, sizeof(CMD_FD_FRAME_OFFLOAD_T));

	pFd_frame_offload->ucEnable = pSetFdFrameOffload->ucEnable;
	pFd_frame_offload->ucWlanIdx = 0;
	pFd_frame_offload->ucOwnMacIdx = wdev->OmacIdx;
	pFd_frame_offload->ucBandIdx = HcGetBandByWdev(wdev);

	if (pFd_frame_offload->ucEnable) {
		pFd_frame_offload->u2TimestampFieldPos = pSetFdFrameOffload->u2TimestampFieldPos;
		pFd_frame_offload->u2PktLength = pSetFdFrameOffload->u2PktLength;
		os_move_mem(pFd_frame_offload->acPktContent, pSetFdFrameOffload->acPktContent,
			pFd_frame_offload->u2PktLength);
	}

	hex_dump_with_lvl("FD_FRAME HwCtrlSetFdFrameOffload", pFd_frame_offload->acPktContent,
		pFd_frame_offload->u2PktLength, DBG_LVL_TRACE);

	MtCmdFdFrameOffloadSet(pAd, pFd_frame_offload);

	os_free_mem(pFd_frame_offload);

	return NDIS_STATUS_SUCCESS;
}
#endif /* OCE_SUPPORT */

#ifdef BCN_OFFLOAD_SUPPORT
#ifndef DOT11V_MBSSID_SUPPORT
static NTSTATUS HwCtrlSetBcnOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_BCN_OFFLOAD pSetBcnOffload = (PMT_SET_BCN_OFFLOAD)CMDQelmt->buffer;
	struct wifi_dev *wdev = pAd->wdev_list[pSetBcnOffload->WdevIdx];
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	CMD_BCN_OFFLOAD_T_V2 *bcn_offload_v2 = NULL;
#endif
	CMD_BCN_OFFLOAD_T bcn_offload;
	BCN_BUF_STRUCT *bcn_buf = NULL;
	UCHAR *buf;
	PNDIS_PACKET *pkt = NULL;

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN) {
		os_alloc_mem(NULL, (PUCHAR *)&bcn_offload_v2, sizeof(*bcn_offload_v2));
		if (!bcn_offload_v2) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can not allocate bcn_offload\n"));
				return NDIS_STATUS_FAILURE;
		}
		os_zero_mem(bcn_offload_v2, sizeof(*bcn_offload_v2));
	} else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#endif

	if ((pSetBcnOffload->OffloadPktType == PKT_V1_BCN)
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	|| (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
#endif
	) {
		bcn_buf = &wdev->bcn_buf;

		if (!bcn_buf) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): bcn_buf is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
			os_free_mem(bcn_offload_v2);
#endif
			return NDIS_STATUS_FAILURE;
		}

		pkt = bcn_buf->BeaconPkt;
	}

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN) {
			bcn_offload_v2->ucEnable = pSetBcnOffload->Enable;
			bcn_offload_v2->ucWlanIdx = 0;/* hardcode at present */
			bcn_offload_v2->ucOwnMacIdx = wdev->OmacIdx;
			bcn_offload_v2->ucBandIdx = HcGetBandByWdev(wdev);
			bcn_offload_v2->u2PktLength = pSetBcnOffload->WholeLength;
			bcn_offload_v2->ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
			bcn_offload_v2->u2TimIePos = pSetBcnOffload->TimIePos;
			bcn_offload_v2->u2CsaIePos = pSetBcnOffload->CsaIePos;
			bcn_offload_v2->ucCsaCount = wdev->csa_count;
#endif
			buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
			NdisCopyMemory(bcn_offload_v2->acPktContent, buf, pSetBcnOffload->WholeLength);
			MtCmdBcnV2OffloadSet(pAd, bcn_offload_v2);
	} else {
	bcn_offload.ucEnable = pSetBcnOffload->Enable;
	bcn_offload.ucWlanIdx = 0;/* hardcode at present */
	bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload.ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
	MtCmdBcnOffloadSet(pAd, bcn_offload);
}
#else
{
	bcn_offload.ucEnable = pSetBcnOffload->Enable;
	bcn_offload.ucWlanIdx = 0;/* hardcode at present */
	bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
	bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
	bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
	bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
	bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
	bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
	bcn_offload.ucCsaCount = wdev->csa_count;
#endif
	buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
	NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
	MtCmdBcnOffloadSet(pAd, bcn_offload);
}
#endif
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (pSetBcnOffload->OffloadPktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif

	return NDIS_STATUS_SUCCESS;
}
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /*BCN_OFFLOAD_SUPPORT*/

static NTSTATUS HwCtrlSetTREntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_SET_TR_ENTRY pInfo;
	MAC_TABLE_ENTRY *pEntry;

	pInfo = (PRT_SET_TR_ENTRY)CMDQelmt->buffer;
	pEntry = (MAC_TABLE_ENTRY *)pInfo->pEntry;
	TRTableInsertEntry(pAd, (UINT16)pInfo->WCID, pEntry);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateBssInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	BSS_INFO_ARGUMENT_T *pBssInfoArgs = (BSS_INFO_ARGUMENT_T *)CMDQelmt->buffer;
	UINT32 ret;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __func__));
	ret = AsicBssInfoUpdate(pAd, pBssInfoArgs);
	return ret;
}


static NTSTATUS HwCtrlSetBaRec(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_BA_CTRL_T *pSetBaRec = (MT_BA_CTRL_T *)CMDQelmt->buffer;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __func__));
	AsicUpdateBASession(pAd, pSetBaRec->Wcid, pSetBaRec->Tid, pSetBaRec->Sn, pSetBaRec->BaWinSize, pSetBaRec->isAdd,
						pSetBaRec->BaSessionType, pSetBaRec->amsdu);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlHandleUpdateBeacon(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_UPDATE_BEACON *prMtUpdateBeacon = (MT_UPDATE_BEACON *)CMDQelmt->buffer;
	struct wifi_dev *wdev = prMtUpdateBeacon->wdev;
	UCHAR UpdateReason = prMtUpdateBeacon->UpdateReason;
	UCHAR i;
	BOOLEAN UpdateAfterTim = FALSE;
	BCN_BUF_STRUCT *pbcn_buf = NULL;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 max_v2_bcn_num = cap->max_v2_bcn_num;
#endif

	MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			 ("%s(): Update reason: %d\n", __func__, UpdateReason));

	switch (UpdateReason) {
	case BCN_UPDATE_INIT:
	case BCN_UPDATE_IF_STATE_CHG:
	case BCN_UPDATE_IE_CHG:
	case BCN_UPDATE_CSA:
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (wdev != NULL) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
				if (wdev->func_idx < max_v2_bcn_num)
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, TRUE);
				else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, TRUE);
#else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, TRUE);
#endif
			} else {
				MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						 ("%s(): wdev = NULL, reason(%d)\n", __func__, UpdateReason));
			}
		}
	break;

	case BCN_UPDATE_ALL_AP_RENEW: {
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			/* Update/Renew all wdev */
			for (i = 0; i < WDEV_NUM_MAX; i++) {
				if (pAd->wdev_list[i] != NULL) {
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
					if (pAd->wdev_list[i]->func_idx < max_v2_bcn_num)
						UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V2_BCN, TRUE);
					else
						UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V1_BCN, TRUE);
#else
						UpdateBeaconProc(pAd, pAd->wdev_list[i], UpdateAfterTim, PKT_V1_BCN, TRUE);
#endif
				}
			}
		}
	}
	break;

	case BCN_UPDATE_ENABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;

			if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
				if (pbcn_buf->BcnUpdateMethod == BCN_GEN_BY_FW) {
					wdev->bcn_buf.bBcnSntReq = TRUE;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
				if (wdev->func_idx < max_v2_bcn_num)
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, TRUE);
				else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, TRUE);
#else
					UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, TRUE);
#endif
				} else
					AsicEnableBeacon(pAd, wdev);
			}
		}
	}
	break;

	case BCN_UPDATE_DISABLE_TX: {
		if (wdev != NULL) {
			pbcn_buf = &wdev->bcn_buf;

			if (WDEV_WITH_BCN_ABILITY(wdev)) {
				if (pbcn_buf->BcnUpdateMethod == BCN_GEN_BY_FW) {
					wdev->bcn_buf.bBcnSntReq = FALSE;
					/* No need to make beacon */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
					if (wdev->func_idx < max_v2_bcn_num)
						UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V2_BCN, FALSE);
					else
						UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, FALSE);
#else
						UpdateBeaconProc(pAd, wdev, UpdateAfterTim, PKT_V1_BCN, FALSE);
#endif
				} else
					AsicDisableBeacon(pAd, wdev);
			}
		}
	}
	break;

	case BCN_UPDATE_PRETBTT: {
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT

		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
#ifdef P2P_SUPPORT
		if (P2P_INF_ON(pAd) && P2P_GO_ON(pAd))
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
#endif /* RT_CFG80211_P2P_SUPPORT */
		{
#ifdef AP_QLOAD_SUPPORT
			ULONG UpTime;
			/* update channel utilization */
			NdisGetSystemUpTime(&UpTime);
			QBSS_LoadUpdate(pAd, UpTime);
#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
			RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
			RT_CFG80211_BEACON_TIM_UPDATE(pAd);
#else
			UpdateAfterTim = TRUE;
			updateBeaconRoutineCase(pAd, UpdateAfterTim);
#endif /* RT_CFG80211_P2P_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 ("%s(): Wrong Update reason: %d\n",
				  __func__, UpdateReason));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef ERR_RECOVERY

static INT ErrRecoveryMcuIntEvent(RTMP_ADAPTER *pAd, UINT32 status)
{
	return chip_trigger_int_to_mcu(pAd, status);
}

static INT ErrRecoverySetRecovStage(
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtl,
	ERR_RECOVERY_STAGE errRecovStage)
{
	if (pErrRecoveryCtl == NULL)
		return FALSE;

	pErrRecoveryCtl->errRecovStage = errRecovStage;
	return TRUE;
}

static UINT32 ErrRecoveryTimeDiff(UINT32 time1, UINT32 time2)
{
	UINT32 timeDiff = 0;

	if (time1 > time2)
		timeDiff = (0xFFFFFFFF - time1 + 1) + time2;
	else
		timeDiff = time2 - time1;

	return timeDiff;
}

void SerTimeLogDump(RTMP_ADAPTER *pAd)
{
	UINT32 idx = 0;
	UINT32 *pSerTimes = NULL;

	if (pAd == NULL)
		return;

	pSerTimes = &pAd->HwCtrl.ser_times[0];

	for (idx = SER_TIME_ID_T0; idx < SER_TIME_ID_END; idx++) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 ("%s,::E  R  , Time[%d](us)=%u\n", __func__, idx,
				  pSerTimes[idx]));
	}

	for (idx = SER_TIME_ID_T0; idx < (SER_TIME_ID_END - 1); idx++) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 ("%s,::E  R  , T%d - T%d(us)=%u\n", __func__,
				  idx + 1, idx, ErrRecoveryTimeDiff(pSerTimes[idx],
						  pSerTimes[idx + 1])));
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			 ("%s,::E  R  , Total Time(us)=%u\n", __func__,
			  ErrRecoveryTimeDiff(pSerTimes[SER_TIME_ID_T0],
								  pSerTimes[SER_TIME_ID_T7])));
}


VOID ser_sys_reset(RTMP_STRING *arg)
{
#ifdef SDK_TIMER_WDG
	/*kernel_restart(NULL);*/
	panic(arg); /* trigger SDK WATCHDOG TIMER */
#endif /* SDK_TIMER_WDG */
}


static void ErrRecoveryEndDriverRestore(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *hdev_ctrl = pAd->hdev_ctrl;

#ifdef RTMP_MAC_PCI
		if (IS_PCI_INF(pAd)) {
			struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
			pci_rx_all(pci_hif);
		}
#endif

}


NTSTATUS HwRecoveryFromError(RTMP_ADAPTER *pAd)
{
	UINT32 Status;
	UINT32 Stage;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl;
	UINT32 Highpart = 0;
	UINT32 Lowpart = 0;
	UINT32 *pSerTimes = NULL;
	PKT_TOKEN_CB *pktTokenCb = NULL;
	UINT32 dbg_lvl_orig;
	UINT32 dbg_lvl_ser;
	UINT32 dbg_subcat_orig[DBG_LVL_MAX + 1][32] = {0};

	if (!pAd)
		return NDIS_STATUS_INVALID_DATA;
	pktTokenCb = hc_get_ct_cb(pAd->hdev_ctrl);
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 ("Ser():The driver is in ATE mode now\n"));
		return NDIS_STATUS_SUCCESS;
	}

#endif /* CONFIG_ATE */
	pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
	Status = pAd->HwCtrl.ser_status;
	Stage = ErrRecoveryCurStage(pErrRecoveryCtrl);
	pSerTimes = &pAd->HwCtrl.ser_times[0];
	MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN, ("Ser                       ,::E  R  , Stage(%d)\n", Stage));

	/* store debug level */
	dbg_lvl_orig = DebugLevel;
	memcpy(dbg_subcat_orig, DebugSubCategory, sizeof(dbg_subcat_orig));

	/* force align to SER debug level, if SER Lvl >= WARN(2) */
	dbg_lvl_ser = get_dbg_lvl_by_cat_subcat(DBG_CAT_HW, CATHW_SER);
	set_dbg_lvl_all((dbg_lvl_ser >= DBG_LVL_WARN) ? dbg_lvl_ser : DBG_LVL_OFF);

	/*wlan hook for ser*/
	WLAN_HOOK_CALL(WLAN_HOOK_SER, pAd, pErrRecoveryCtrl);

	switch (Stage) {
	case ERR_RECOV_STAGE_STOP_IDLE:			/* Stage 0 */
	case ERR_RECOV_STAGE_EVENT_REENTRY:
		if (Status & ERROR_DETECT_STOP_PDMA) {
			PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
			ULONG flags = 0;

			os_zero_mem(pSerTimes,
						(sizeof(pSerTimes[SER_TIME_ID_T0]) * SER_TIME_ID_END));
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T0] = Lowpart;
			/* Stop access PDMA. */
			ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_STOP_IDLE_DONE);
			/* send PDMA0 stop to N9 through interrupt. */
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_STOP_DONE);

			/* all mmio need to be stop till hw reset done. */
			RTMP_SPIN_LOCK_IRQSAVE(&pci_hif->io_remap_lock, &flags);
			pci_hif->bPCIclkOff = TRUE;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&pci_hif->io_remap_lock, &flags);
			RtmpusecDelay(100 * 1000); /* delay for 100 ms to wait reset done. */
			RTMP_SPIN_LOCK_IRQSAVE(&pci_hif->io_remap_lock, &flags);
			pci_hif->bPCIclkOff = FALSE;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&pci_hif->io_remap_lock, &flags);

			/*re-call for change status to stop dma0*/
			HwRecoveryFromError(pAd);
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T1] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
					 ("!!! SER CurStage=%u Event=%x!!!\n", ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STAGE_STOP_PDMA0:		/* Stage 1 */
		if (Status & ERROR_DETECT_RESET_DONE) {
			ULONG flags = pAd->Flags;

			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T2] = Lowpart;

			/* clear flags for isr and cmd */
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP | fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);

			tm_exit(pAd);
			qm_exit(pAd);
			hif_reset_task_group(pAd->hdev_ctrl);
			hif_reset_txrx_mem(pAd->hdev_ctrl);
			token_deinit((PKT_TOKEN_CB **)&pktTokenCb);

			WfHifInit(pAd);
			qm_init(pAd);
			tm_init(pAd);

			/* restore flags, must before intr enabled */
			RTMP_SET_FLAG(pAd, flags & (fRTMP_ADAPTER_START_UP | fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD));

			/* Enable Interrupt*/
			chip_interrupt_enable(pAd);
			hif_dma_enable(pAd->hdev_ctrl);

			if (IS_MT7615(pAd) || IS_MT7622(pAd))
				HIF_IO_WRITE32(pAd->hdev_ctrl, MT_WPDMA_MEM_RNG_ERR, 0);

			ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_RESET_PDMA0);

			/* send PDMA0 reinit done to N9 through interrupt. */
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_INIT_DONE);
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);

			pSerTimes[SER_TIME_ID_T3] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
					 ("!!! SER CurStage=%u Event=%x!!!\n", ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STAGE_RESET_PDMA0:		/* Stage 2 */
		if (Status & ERROR_DETECT_RECOVERY_DONE) {
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T4] = Lowpart;
			ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_WAIT_N9_NORMAL);
			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_RECOVERY_DONE);
			pSerTimes[SER_TIME_ID_T5] = Lowpart;
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
					 ("!!! SER CurStage=%u Event=%x!!!\n",
					  ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STAGE_WAIT_N9_NORMAL:		/* Stage 4 */
		if (Status & ERROR_DETECT_N9_NORMAL_STATE) {
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;
			ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_STOP_IDLE);
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;
			ErrRecoveryEndDriverRestore(pAd);

			/* send BAR to all STAs */
			ba_refresh_bar_all(pAd);

			/* update Beacon frame if operating in AP mode. */
			UpdateBeaconHandler(
				pAd,
				get_default_wdev(pAd),
				BCN_UPDATE_ALL_AP_RENEW);
		} else if (Status & ERROR_DETECT_STOP_PDMA) {
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;
			MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
					 ("!!! ERROR SER re-entry  CurStage=%u Event=%x!!!\n",
					  ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
			AsicGetTsfTime(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;
			ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_EVENT_REENTRY);
			HwRecoveryFromError(pAd);
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
					 ("!!! SER CurStage=%u Event=%x!!!\n", ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
		}

		break;

	case ERR_RECOV_STAGE_STOP_IDLE_DONE:		/* Stage 3 */
		ErrRecoverySetRecovStage(pErrRecoveryCtrl, ERR_RECOV_STAGE_STOP_PDMA0);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 ("!!! SER CurStage=%u Event=%x!!!\n", ErrRecoveryCurStage(pErrRecoveryCtrl), Status));
		break;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN, ("\tStage(%d) END\n", Stage));

	/* restore debug level */
	DebugLevel = dbg_lvl_orig;
	memcpy(DebugSubCategory, dbg_subcat_orig, sizeof(dbg_subcat_orig));

	/* dump TimeLog if exceed SER_TIME_THRESHOLD */
	if ((Stage == ERR_RECOV_STAGE_WAIT_N9_NORMAL) &&
		(ErrRecoveryTimeDiff(pSerTimes[SER_TIME_ID_T0], pSerTimes[SER_TIME_ID_T7]) > SER_TIME_THRESHOLD)) {
		/*print out ser log timing*/
		SerTimeLogDump(pAd);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* ERR_RECOVERY */

#endif

/*STA part*/
#ifdef CONFIG_STA_SUPPORT

static NTSTATUS HwCtrlPwrMgtBitWifi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_PWR_MGT_BIT_WIFI_T	rpMtPwrMgtBitWifi = (PMT_PWR_MGT_BIT_WIFI_T)(CMDQelmt->buffer);

	AsicExtPwrMgtBitWifi(pAd, rpMtPwrMgtBitWifi->u2WlanIdx, rpMtPwrMgtBitWifi->ucPwrMgtBit);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwEnterPsNull(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);
	PSTA_ADMIN_CONFIG pStaCfg = prStaCfgPtr->pStaCfg;
	PMAC_TABLE_ENTRY pMacEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);

	RTMPSendNullFrame(pAd, pMacEntry, pAd->CommonCfg.TxRate, TRUE, PWR_SAVE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlWakeUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);

	AsicWakeup(pAd, TRUE, prStaCfgPtr->pStaCfg);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSleepAutoWakeup(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_STA_CFG_PTR_T	prStaCfgPtr = (PMT_STA_CFG_PTR_T)(CMDQelmt->buffer);

	AsicSleepAutoWakeup(pAd, prStaCfgPtr->pStaCfg);
	return NDIS_STATUS_SUCCESS;
}

#endif

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
static NTSTATUS hw_ctrl_host_resume_done_ack(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->HostResumeDoneAck != NULL)
		ops->HostResumeDoneAck(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static NTSTATUS HwCtrlMlmeDynamicTxRateSwitching(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MlmeDynamicTxRateSwitching(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT */

static NTSTATUS HwCtrlNICUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	asic_update_raw_counters(pAd);
	return NDIS_STATUS_SUCCESS;
}

/*Pheripheral Handler*/
static NTSTATUS HwCtrlCheckGPIO(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

#ifdef LED_CONTROL_SUPPORT
static NTSTATUS HwCtrlSetLEDStatus(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_LED_STS	prLedStatus = (PMT_SET_LED_STS)(CMDQelmt->buffer);

	RTMPSetLEDStatus(pAd, prLedStatus->Status, prLedStatus->BandIdx);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: CMDTHREAD_SET_LED_STATUS (LEDStatus = %d, BandIdx = %d)\n",
			 __func__, prLedStatus->Status, prLedStatus->BandIdx));
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlLedGpioMap(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_LED_GPIO_MAP prLedMapping = (PMT_LED_GPIO_MAP)(CMDQelmt->buffer);

	AndesLedGpioMap(pAd, prLedMapping->led_index, prLedMapping->map_index, prLedMapping->ctr_type);
	return NDIS_STATUS_SUCCESS;
}

#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS HwCtrlLEDWPSMode10(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PMT_SET_LED_STS	prLedStatus = (PMT_SET_LED_STS)(CMDQelmt->buffer);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", prLedStatus->Status));

	switch (prLedStatus->Status) {
	case LINK_STATUS_WPS_MODE10_TURN_ON:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_ON, prLedStatus->BandIdx);
		break;

	case LINK_STATUS_WPS_MODE10_FLASH:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_FLASH, prLedStatus->BandIdx);
		break;

	case LINK_STATUS_WPS_MODE10_TURN_OFF:
		RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_OFF, prLedStatus->BandIdx);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10:: No this status %d!!!\n", prLedStatus->Status));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRR(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_VOW_STA_GROUP *pVoW  =  (MT_VOW_STA_GROUP *)(CMDQelmt->buffer);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: group %d, staid %d\n", __func__, pVoW->GroupIdx,
			 pVoW->StaIdx));
	vow_set_client(pAd, pVoW->GroupIdx, pVoW->StaIdx, pVoW->WmmIdx);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRRQuantum(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	INT32 ret;
	MT_VOW_STA_QUANTUM *pVoW  =  (MT_VOW_STA_QUANTUM *)(CMDQelmt->buffer);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\x1b[31m%s: restore %d, quantum %d\x1b[m\n", __func__, pVoW->restore, pVoW->quantum));

	if (pVoW->restore) {
		if (vow_watf_is_enabled(pAd)) {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
		} else {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
		}
	} else {
		UINT8 ac;
		/* 4 ac with the same quantum */
		for (ac = 0; ac < WMM_NUM_OF_AC; ac++)
			pAd->vow_cfg.vow_sta_dwrr_quantum[ac] = pVoW->quantum;
	}

	ret = vow_set_sta(pAd, 0, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\x1b[31m%s: ret %d\x1b[m\n", __func__, ret));

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetVOWScheduleCtrl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _VOW_SCH_CFG_T *vow_sch_cfg = (struct _VOW_SCH_CFG_T *)CMDQelmt->buffer;

	pAd->vow_sch_cfg.apply_sch_ctrl = vow_sch_cfg->apply_sch_ctrl;
	pAd->vow_sch_cfg.sch_type = vow_sch_cfg->sch_type;
	pAd->vow_sch_cfg.sch_policy = vow_sch_cfg->sch_policy;

	return vow_set_feature_all(pAd);
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

static NTSTATUS HwCtrlThermalProtRadioOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:\n", __func__));
	/* Set Radio off Process*/
	Set_RadioOn_Proc(pAd, "0");
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	if (pAd->V10UpdateRssi == 1) {
		Vendor10RssiUpdate(pAd, NULL, FALSE, 0);
		pAd->V10UpdateRssi = 0;
	} else
#endif
		RssiUpdate(pAd);

	return NDIS_STATUS_SUCCESS;
}


#ifdef ETSI_RX_BLOCKER_SUPPORT
static NTSTATUS HwCtrlCheckRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	/* Check RSSI on evey 100 ms */
	CheckRssi(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

static NTSTATUS HwCtrlGetTemperature(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT32 temperature = 0;
	UINT8 uBandIdx = *(UINT8*)CMDQelmt->buffer;

	/*ActionIdx 0 means get temperature*/
	MtCmdGetThermalSensorResult(pAd, 0, uBandIdx ,&temperature);
	os_move_mem(CMDQelmt->RspBuffer, &temperature, CMDQelmt->RspBufferLen);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlGetTxStatistic(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	TX_STAT_STRUC *pTxStat = (PTX_STAT_STRUC)CMDQelmt->buffer;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	UCHAR dbdc_idx = 0;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	if (!VALID_UCAST_ENTRY_WCID(pAd, pTxStat->Wcid))
		return NDIS_STATUS_SUCCESS;

	pEntry = &pAd->MacTab.Content[pTxStat->Wcid];

	if (IS_ENTRY_NONE(pEntry))
		return NDIS_STATUS_SUCCESS;

	wdev = pEntry->wdev;

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	dbdc_idx = HcGetBandByWdev(wdev);

	tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];

	if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
		return NDIS_STATUS_SUCCESS;

	mt_cmd_get_sta_tx_statistic(pAd, pTxStat->Wcid, dbdc_idx, pTxStat->Field);
#endif
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRadioOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRADIO_ON_OFF_T pRadioOnOff = (PRADIO_ON_OFF_T)CMDQelmt->buffer;

	AsicRadioOnOffCtrl(pAd, pRadioOnOff->ucDbdcIdx, pRadioOnOff->ucRadio);
	return NDIS_STATUS_SUCCESS;
}

#ifdef LINK_TEST_SUPPORT
static NTSTATUS HwCtrlAutoLinkTest(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	/* Link Test Time Slot Handler */
	LinkTestTimeSlotLinkHandler(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef GREENAP_SUPPORT
static NTSTATUS HwCtrlGreenAPOnOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PGREENAP_ON_OFF_T pGreenAP = (PGREENAP_ON_OFF_T)CMDQelmt->buffer;

	AsicGreenAPOnOffCtrl(pAd, pGreenAP->ucDbdcIdx, pGreenAP->ucGreenAPOn);
	return NDIS_STATUS_SUCCESS;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
static NTSTATUS hw_ctrl_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	P_PCIE_ASPM_DYM_CTRL_T ppcie_aspm_dym_ctrl = (P_PCIE_ASPM_DYM_CTRL_T)CMDQelmt->buffer;

	asic_pcie_aspm_dym_ctrl(
		pAd,
		ppcie_aspm_dym_ctrl->ucDbdcIdx,
		ppcie_aspm_dym_ctrl->fgL1Enable,
		ppcie_aspm_dym_ctrl->fgL0sEnable);

	return NDIS_STATUS_SUCCESS;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
static NTSTATUS hw_ctrl_twt_agrt_update(struct _RTMP_ADAPTER *ad, HwCmdQElmt *CMDQelmt)
{
	struct twt_agrt_para twt_agrt_para = {0};

	os_move_mem(&twt_agrt_para,
		(struct twt_agrt_para *)CMDQelmt->buffer,
		sizeof(struct twt_agrt_para));
	asic_twt_agrt_update(ad, twt_agrt_para);

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

static NTSTATUS HwCtrlSetSlotTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	SLOT_CFG *pSlotCfg = (SLOT_CFG *)CMDQelmt->buffer;

	AsicSetSlotTime(pAd, pSlotCfg->bUseShortSlotTime, pSlotCfg->Channel, pSlotCfg->wdev);
	return NDIS_STATUS_SUCCESS;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
/*
*
*/
static NTSTATUS HwCtrlSetPbc(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct pbc_ctrl *pbc = (struct pbc_ctrl *)CMDQelmt->buffer;
	INT32 ret = 0;
	UINT8 bssid = (pbc->wdev) ?  (pbc->wdev->bss_info_argument.ucBssIndex) : PBC_BSS_IDX_FOR_ALL;
	UINT16 wcid = (pbc->entry) ? (pbc->entry->wcid) : PBC_WLAN_IDX_FOR_ALL;

	ret = MtCmdPktBudgetCtrl(pAd, bssid, wcid, pbc->type);
	return ret;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

/*
*
*/
static NTSTATUS HwCtrlWifiSysOpen(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_open)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_open(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysClose(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_close)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_close(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_up(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_link_down(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_disconnt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_connt_act(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerUpdate(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct WIFI_SYS_CTRL *wsys = (struct WIFI_SYS_CTRL *)CMDQelmt->buffer;

	if (pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update)
		return pAd->HwCtrl.hwctrl_ops.wifi_sys_peer_update(wsys);
	else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

#ifdef MBO_SUPPORT
static NTSTATUS HwCtrlBssTermination(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct wifi_dev *wdev = (struct wifi_dev *)CMDQelmt->buffer;
	UCHAR RfIC = 0;

	RfIC = wmode_2_rfic(wdev->PhyMode);

		if (!wdev->if_up_down_state) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>HwCtrlBssTermination (%s) but IF is done, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx));
			return TRUE;
		}

		if (IsHcRadioCurStatOffByChannel(pAd, wdev->channel)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>HwCtrlBssTermination (%s) equal to current state, ignore!!! (wdev_idx %d)\n",
				"OFF", wdev->wdev_idx));
			return TRUE;
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
		MlmeRadioOff(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>HwCtrlBssTermination (OFF)\n"));

	return NDIS_STATUS_SUCCESS;
}
#endif

BOOLEAN hwctrl_cmd_q_empty(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	UINT16 time_out_cnt = 100;
	UINT16 current_cnt = 0;
	UINT32 hw_ctrl_q_len = 0;
	UINT32 txcmd_q_len = 0;

	while(current_cnt < time_out_cnt) {
		hw_ctrl_q_len = hwctrl_queue_len(pAd);
		txcmd_q_len = AndesQueueLen(ctl, &ctl->txq);
		if ((hw_ctrl_q_len != 0) || (txcmd_q_len != 0))
			msleep(100);
		else
			break;
		current_cnt++;
	}

	if (current_cnt == time_out_cnt)
		return FALSE;
	else
		return TRUE;
}

#ifdef NF_SUPPORT_V2
static NTSTATUS HwCtrlUpdateNF(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR en = *((UCHAR *)CMDQelmt->buffer);
	EnableNF(pAd, en, 100, 5); /*read CR per 100ms, 5 counts, about 500ms*/
	return NDIS_STATUS_SUCCESS;
}
#endif

/* For wifi and md coex in colgin project */
#ifdef WIFI_MD_COEX_SUPPORT
static NTSTATUS HwCtrlWifiCoexApccci2fw(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _MT_WIFI_COEX_APCCCI2FW *apccci2fw_msg =
			(struct _MT_WIFI_COEX_APCCCI2FW *)CMDQelmt->buffer;
	SendApccci2fwMsg(pAd, apccci2fw_msg);
	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef WIFI_MD_COEX_SUPPORT
static NTSTATUS HwCtrlQueryLteSafeChannel(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	QueryLteSafeChannel(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CFG_SUPPORT_CSI
static NTSTATUS HwCtrlGetCSIData(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct CMD_CSI_CONTROL_T *prCSICtrl;

	prCSICtrl = (struct CMD_CSI_CONTROL_T *)CMDQelmt->buffer;
	AndesCSICtrl(pAd, prCSICtrl);
	return NDIS_STATUS_SUCCESS;
}
#endif

/*HWCMD_TYPE_RADIO*/
static HW_CMD_TABLE_T HwCmdRadioTable[] = {
	{HWCMD_ID_UPDATE_DAW_COUNTER, HwCtrlUpdateRawCounters, 0},
#ifdef MT_MAC
	{HWCMD_ID_SET_CLIENT_MAC_ENTRY, HwCtrlSetClientMACEntry, 0},
#ifdef TXBF_SUPPORT
	{HWCMD_ID_SET_APCLI_BF_CAP, HwCtrlSetClientBfCap, 0},
	{HWCMD_ID_SET_APCLI_BF_REPEATER, HwCtrlSetBfRepeater, 0},
	{HWCMD_ID_ADJUST_STA_BF_SOUNDING, HwCtrlAdjBfSounding, 0},
	{HWCMD_ID_TXBF_TX_APPLY_CTRL, HwCtrlTxBfTxApply, 0},
#endif
	{HWCMD_ID_SET_TR_ENTRY, HwCtrlSetTREntry, 0},
	{HWCMD_ID_SET_BA_REC, HwCtrlSetBaRec, 0},
	{HWCMD_ID_UPDATE_BSSINFO, HwCtrlUpdateBssInfo, 0},
	{HWCMD_ID_UPDATE_BEACON, HwCtrlHandleUpdateBeacon, 0},
	{HWCMD_ID_SET_TX_BURST, HwCtrlSetTxBurst, 0},
#endif /*MT_MAC*/
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_AP_ADJUST_EXP_ACK_TIME, HwCtrlAPAdjustEXPAckTime, 0},
	{HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,	HwCtrlAPRecoverEXPAckTime, 0},
#endif
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_SET_STA_DWRR, HwCtrlSetStaDWRR, 0},
#ifdef VOW_SUPPORT
	{HWCMD_ID_SET_STA_DWRR_QUANTUM, HwCtrlSetStaDWRRQuantum, 0},
	{HWCMD_ID_SET_VOW_SCHEDULE_CTRL, HwCtrlSetVOWScheduleCtrl, 0},
#endif /* VOW_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{HWCMD_ID_UPDATE_RSSI, HwCtrlUpdateRssi, 0},
	{HWCMD_ID_GET_TEMPERATURE, HwCtrlGetTemperature, 0},
	{HWCMD_ID_SET_SLOTTIME, HwCtrlSetSlotTime, 0},
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{HWCMD_RX_CHECK_RSSI, HwCtrlCheckRssi, 0},
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
#ifdef BCN_OFFLOAD_SUPPORT
#ifndef DOT11V_MBSSID_SUPPORT
	{HWCMD_ID_SET_BCN_OFFLOAD, HwCtrlSetBcnOffload, 0},
#endif /* DOT11V_MBSSID_SUPPORT */
#endif
#ifdef OCE_SUPPORT
	{HWCMD_ID_SET_FD_FRAME_OFFLOAD, HwCtrlSetFdFrameOffload, 0},
#endif /* OCE_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	{HWCMD_ID_ADD_REPT_ENTRY, HwCtrlAddReptEntry, 0},
	{HWCMD_ID_REMOVE_REPT_ENTRY, HwCtrlRemoveReptEntry, 0},
#endif
	{HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, HwCtrlThermalProtRadioOff, 0},
	{HWCMD_ID_RADIO_ON_OFF, HwCtrlRadioOnOff, 0},
#ifdef LINK_TEST_SUPPORT
	{HWCMD_ID_AUTO_LINK_TEST, HwCtrlAutoLinkTest, 0},
#endif /* LINK_TEST_SUPPORT */
#ifdef GREENAP_SUPPORT
	{HWCMD_ID_GREENAP_ON_OFF, HwCtrlGreenAPOnOff, 0},
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	{HWCMD_ID_BSS_TERMINATION, HwCtrlBssTermination, 0},
#endif /* MBO_SUPPORT */
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	{HWCMD_ID_UPDATE_4ADDR_HDR_TRANS, HwCtrlUpdate4Addr_HdrTrans, 0},
#endif
	{HWCMD_ID_UPDATE_MIB_COUNTER, HwCtrlUpdateMibCounters, 0},
#ifdef NF_SUPPORT_V2
	{HWCMD_ID_GET_NF_BY_FW, HwCtrlUpdateNF, 0},
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	{HWCMD_ID_WIFI_COEX_APCCCI2FW, HwCtrlWifiCoexApccci2fw, 0},
	{HWCMD_ID_QUERY_LTE_SAFE_CHANNEL, HwCtrlQueryLteSafeChannel, 0},
#endif /* WIFI_MD_COEX_SUPPORT */
#ifdef CFG_SUPPORT_CSI
	{HWCMD_ID_GET_CSI_RAW_DATA, HwCtrlGetCSIData, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_SECURITY*/
static HW_CMD_TABLE_T HwCmdSecurityTable[] = {
	{HWCMD_ID_ADDREMOVE_ASIC_KEY, HwCtrlAddRemoveKeyTab, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PERIPHERAL*/
static HW_CMD_TABLE_T HwCmdPeripheralTable[] = {
	{HWCMD_ID_GPIO_CHECK, HwCtrlCheckGPIO, 0},
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	{HWCMD_ID_LED_WPS_MODE10, HwCtrlLEDWPSMode10, 0},
#endif
#endif
#ifdef LED_CONTROL_SUPPORT
	{HWCMD_ID_SET_LED_STATUS, HwCtrlSetLEDStatus, 0},
	{HWCMD_ID_LED_GPIO_MAP, HwCtrlLedGpioMap, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_HT_CAP*/
static HW_CMD_TABLE_T HwCmdHtCapTable[] = {
	{HWCMD_ID_DEL_ASIC_WCID, HwCtrlDelAsicWcid, 0},
#ifdef HTC_DECRYPT_IOT
	{HWCMD_ID_SET_ASIC_AAD_OM, HwCtrlSetAsicWcidAAD_OM, 0},
#endif /* HTC_DECRYPT_IOT */
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PS*/
static HW_CMD_TABLE_T HwCmdPsTable[] = {
#ifdef MT_MAC
#endif
#ifdef CONFIG_STA_SUPPORT
	{HWCMD_ID_PWR_MGT_BIT_WIFI, HwCtrlPwrMgtBitWifi, 0},
	{HWCMD_ID_FORCE_WAKE_UP, HwCtrlWakeUp, 0},
	{HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP, HwCtrlSleepAutoWakeup, 0},
	{HWCMD_ID_ENTER_PS_NULL, HwEnterPsNull, 0},
#endif
#ifdef CONFIG_STA_SUPPORT
	{HWCMD_ID_PERODIC_CR_ACCESS_MLME_DYNAMIC_TX_RATE_SWITCHING, HwCtrlMlmeDynamicTxRateSwitching, 0},
#endif /* CONFIG_STA_SUPPORT */
	{HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, HwCtrlNICUpdateRawCounters, 0},
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	{HWCMD_ID_HOST_RESUME_DONE_ACK, hw_ctrl_host_resume_done_ack, 0},
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{HWCMD_ID_PCIE_ASPM_DYM_CTRL, hw_ctrl_pcie_aspm_dym_ctrl, 0},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	{HWCMD_ID_TWT_AGRT_UPDATE, hw_ctrl_twt_agrt_update, 0},
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	{HWCMD_ID_END, NULL, 0}
};


/*HWCMD_TYPE_WIFISYS*/
static HW_CMD_TABLE_T HwCmdWifiSysTable[] = {
	{HWCMD_ID_WIFISYS_LINKDOWN, HwCtrlWifiSysLinkDown, 0},
	{HWCMD_ID_WIFISYS_LINKUP, HwCtrlWifiSysLinkUp, 0},
	{HWCMD_ID_WIFISYS_OPEN, HwCtrlWifiSysOpen, 0},
	{HWCMD_ID_WIFISYS_CLOSE, HwCtrlWifiSysClose, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKDOWN, HwCtrlWifiSysPeerLinkDown, 0},
	{HWCMD_ID_WIFISYS_PEER_LINKUP, HwCtrlWifiSysPeerLinkUp, 0},
	{HWCMD_ID_WIFISYS_PEER_UPDATE, HwCtrlWifiSysPeerUpdate, 0},
	{HWCMD_ID_GET_TX_STATISTIC, HwCtrlGetTxStatistic, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_WMM*/
static HW_CMD_TABLE_T HwCmdWmmTable[] = {
	{HWCMD_ID_PART_SET_WMM, HwCtrlSetPartWmmParam, 0},
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{HWCMD_ID_PBC_CTRL, HwCtrlSetPbc, 0},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef DABS_QOS
	{HWCMD_ID_SET_DEL_QOS, HwCtrlSetQosParam, 0},
#endif
	{HWCMD_ID_END, NULL, 0}
};

/*HWCMD_TYPE_PROTECT*/
static HW_CMD_TABLE_T HwCmdProtectTable[] = {
	{HWCMD_ID_RTS_THLD, HwCtrlUpdateRtsThreshold, 0},
	{HWCMD_ID_HT_PROTECT, HwCtrlUpdateProtect, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*Order can't be changed, follow HW_CMD_TYPE order definition*/
HW_CMD_TABLE_T *HwCmdTable[] = {
	HwCmdRadioTable,
	HwCmdSecurityTable,
	HwCmdPeripheralTable,
	HwCmdHtCapTable,
	HwCmdPsTable,
	HwCmdWifiSysTable,
	HwCmdWmmTable,
	HwCmdProtectTable,
	NULL
};


