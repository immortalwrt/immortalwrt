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
	andes_mt.c
*/

#include	"rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include    "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#define PHY_MODE_CCK                        0
#define PHY_MODE_OFDM                       1
#define PHY_MODE_HTMIX                      2
#define PHY_MODE_HTGREENFIELD               3
#define PHY_MODE_VHT                        4
#define PHY_MODE_HESU                       8
#define PHY_MODE_HEEXTSU                    9
#define PHY_MODE_HETRIG                     10
#define PHY_MODE_HEMU                       11
#define PHY_MODE_HESU_REMAPPING             5
#define PHY_MODE_HEEXTSU_REMAPPING          6
#define PHY_MODE_HETRIG_REMAPPING           7
#define PHY_MODE_HEMU_REMAPPING             8
#define PHY_MODE_UNKNOWN_REMAPPING          9
#define TX_NSS_SHITF                        6
#define TX_NSS_MASK                         0x7
#define TX_HT_MCS_MASK                      0x3F
#define TX_NON_HT_MCS_MASK                  0xF
#define TX_MODE_SHIFT                       9
#define TX_MODE_MASK                        0xF
#define DCM_SHITF                           4
#define DCM_EN                              (1 << DCM_SHITF)
#define TX_BW_SHIFT                         13
#define TX_BW_MASK                          0x3

extern INT8 i1RxFeLossComp[RAM_BAND_NUM][MAX_ANTENNA_NUM];

static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: EventExtCmdResult.u4Status = 0x%x\n",
			  __func__, EventExtCmdResult->u4Status));
}

/* old chip (before mt7615) use this compact format for cmd header */
VOID AndesMTFillCmdHeader(struct cmd_msg *msg, VOID *pkt)
{
	FW_TXD *fw_txd = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET) pkt;

	if (Ctl->fwdl_ctrl.stage == FWDL_STAGE_FW_RUNNING)
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*fw_txd));
	else
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, 12);

	fw_txd->fw_txd_0.field.length = GET_OS_PKT_LEN(net_pkt);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->attr.type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = IS_CMD_MSG_NA_FLAG_SET(msg) ?
									   CMD_NA : IS_CMD_MSG_SET_QUERY_FLAG_SET(msg);
	fw_txd->fw_txd_1.field.seq_num = msg->seq;
	fw_txd->fw_txd_2.field.ext_cid = msg->attr.ext_type;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: fw_txd: 0x%x 0x%x 0x%x, Length=%d\n", __func__,
			  fw_txd->fw_txd_0.word, fw_txd->fw_txd_1.word,
			  fw_txd->fw_txd_2.word, fw_txd->fw_txd_0.field.length));

	if ((IS_EXT_CMD_AND_SET_NEED_RSP(msg)) && !(IS_CMD_MSG_NA_FLAG_SET(msg)))
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	else
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length,
					   fw_txd->fw_txd_0.field.pq_id, fw_txd->fw_txd_1.field.cid,
					   fw_txd->fw_txd_1.field.pkt_type_id, fw_txd->fw_txd_1.field.set_query,
					   fw_txd->fw_txd_1.field.seq_num, fw_txd->fw_txd_2.field.ext_cid,
					   fw_txd->fw_txd_2.field.ext_cid_option,
					   (char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}

static VOID EventChPrivilegeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	UINT32 Value = 0;

	if (IS_MT7603(pAd) || IS_MT7628(pAd)  || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		RTMP_IO_READ32(pAd->hdev_ctrl, RMAC_RMCR, &Value);

		if (ctl->RxStream0 == 1)
			Value |= RMAC_RMCR_RX_STREAM_0;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_0;

		if (ctl->RxStream1 == 1)
			Value |= RMAC_RMCR_RX_STREAM_1;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_1;

		RTMP_IO_WRITE32(pAd->hdev_ctrl, RMAC_RMCR, Value);
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __func__));
}

#ifdef CONFIG_STA_SUPPORT


static VOID ExtEventRoamingDetectionHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_ROAMING_DETECT_RESULT_T *pExtEventRoaming =
		(struct _EXT_EVENT_ROAMING_DETECT_RESULT_T *)Data;
	pAd->StaCfg[0].PwrMgmt.bTriggerRoaming = TRUE;
	pExtEventRoaming->u4RoamReason = le2cpu32(pExtEventRoaming->u4RoamReason);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s::FW LOG, ucBssidIdx = %d,  u4RoamReason = %d\n",
			  __func__, pExtEventRoaming->ucBssidIdx,
			  pExtEventRoaming->u4RoamReason));
}
#endif /*CONFIG_STA_SUPPORT*/

static VOID ExtEventBeaconLostHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = NULL;
#endif
	struct _EXT_EVENT_BEACON_LOSS_T *pExtEventBeaconLoss =
		(struct _EXT_EVENT_BEACON_LOSS_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 ("%s::FW EVENT (%02x:%02x:%02x:%02x:%02x:%02x), Reason 0x%x\n", __func__,
			  pExtEventBeaconLoss->aucBssid[0],
			  pExtEventBeaconLoss->aucBssid[1],
			  pExtEventBeaconLoss->aucBssid[2],
			  pExtEventBeaconLoss->aucBssid[3],
			  pExtEventBeaconLoss->aucBssid[4],
			  pExtEventBeaconLoss->aucBssid[5],
			  pExtEventBeaconLoss->ucReason));

	switch (pExtEventBeaconLoss->ucReason) {
#ifdef CONFIG_AP_SUPPORT

	case ENUM_BCN_LOSS_AP_DISABLE:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, ("  AP Beacon OFF!!!\n"));
		break;

	case ENUM_BCN_LOSS_AP_SER_TRIGGER:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, ("  SER happened!!!\n"));
		break;

	case ENUM_BCN_LOSS_AP_ERROR:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Beacon lost - Error!!! Re-issue BCN_OFFLOAD cmd\n"));
		/* update Beacon again if operating in AP mode. */
		wdev = wdev_search_by_address(pAd, pExtEventBeaconLoss->aucBssid);
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): Wrong Addr1 - %02x:%02x:%02x:%02x:%02x:%02x\n",
					  __func__, PRINT_MAC(pExtEventBeaconLoss->aucBssid)));
			break;
		}

		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			break;

		/* do BCN_UPDATE_ALL_AP_RENEW when all BSS CSA done */
		if (pDot11h->csa_ap_bitmap == 0)
			UpdateBeaconHandler(pAd, get_default_wdev(pAd), BCN_UPDATE_ALL_AP_RENEW);
		else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" CSA is runing, wait it done, RDMode=%d, csa_ap_bitmap=0x%x\n",
					pDot11h->RDMode, pDot11h->csa_ap_bitmap));
		break;
#endif
#ifdef CONFIG_STA_SUPPORT

	case ENUM_BCN_LOSS_STA: {
		UCHAR	i = 0;
		PSTA_ADMIN_CONFIG pStaCfg = NULL;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Beacon lost - STA!!!\n"));

		/* Find pStaCfg */
		for (i = 0; i < pAd->MSTANum; i++) {
			pStaCfg = &pAd->StaCfg[i];
			ASSERT(pStaCfg);

			if (pStaCfg->wdev.DevInfo.WdevActive) {
				if (NdisEqualMemory(pExtEventBeaconLoss->aucBssid, pStaCfg->Bssid, MAC_ADDR_LEN)) {
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s::Found StaCfg[%d] Bssid matching\n", __func__, i));
					break;
				}
			}
		}

		if (i == pAd->MSTANum) {
			ASSERT(0);
			return;
		}

		/* Upate pStaCfg */
		pStaCfg->PwrMgmt.bBeaconLost = TRUE;
		break;
	}

#endif

	default:
		break;
	}
}

#ifdef MT_DFS_SUPPORT
static VOID ExtEventRddReportHandler(RTMP_ADAPTER *pAd,
									 UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_RDD_REPORT_T *pExtEventRddReport =
		(struct _EXT_EVENT_RDD_REPORT_T *)Data;
	UCHAR rddidx = HW_RDD0;

	if (!pExtEventRddReport)
		return;

	rddidx = pExtEventRddReport->rdd_idx;


#if !(defined(MT7615) || defined(MT7622) || defined(MT7663))
	/* update dbg pulse info */
	dfs_update_radar_info(pExtEventRddReport);

	if (pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en == TRUE)
		dfs_dump_radar_sw_pls_info(pAd, pExtEventRddReport);

	if (pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en == TRUE)
		dfs_dump_radar_hw_pls_info(pAd, pExtEventRddReport);

	if ((pAd->CommonCfg.DfsParameter.is_radar_emu == TRUE) ||
			(pExtEventRddReport->lng_pls_detected == TRUE) ||
			(pExtEventRddReport->cr_pls_detected == TRUE) ||
			(pExtEventRddReport->stgr_pls_detected == TRUE))
#endif
		WrapDfsRddReportHandle(pAd, rddidx);
}

#endif

static VOID ExtEventIdlePwrReportHandler(RTMP_ADAPTER *pAd,
									 UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_IDLE_PWR_GET_T *pExtEventIdlePwrReport =
		(struct _EXT_EVENT_IDLE_PWR_GET_T *)Data;

	if (!pExtEventIdlePwrReport)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): obss percentage: %d, ipi percentage: %d\n",
			__func__,
			pExtEventIdlePwrReport->obss_percent,
			pExtEventIdlePwrReport->ipi_percent));
}


#ifdef WIFI_SPECTRUM_SUPPORT
/*
	==========================================================================
	Description:
	Unsolicited extend event handler of wifi-spectrum.
	Return:
	==========================================================================
*/
static VOID ExtEventWifiSpectrumHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_SPECTRUM_RESULT_T *pResult = (EXT_EVENT_SPECTRUM_RESULT_T *)pData;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: FuncIndex = %d\n", __func__, pResult->u4FuncIndex));
	pResult->u4FuncIndex = le2cpu32(pResult->u4FuncIndex);
	switch (pResult->u4FuncIndex) {
	case SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, (VOID *)pData, Length);
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Cmd queue raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
NTSTATUS WifiSpectrumRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->SpectrumEventRawDataHandler != NULL) {
		ops->SpectrumEventRawDataHandler(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/*
	==========================================================================
	Description:
	Extend event raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
VOID ExtEventWifiSpectrumUnSolicitRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	UINT32 i, CapNode, Data;
	UINT8 msg_IQ[CAP_FILE_MSG_LEN], msg_Gain[CAP_FILE_MSG_LEN];
	INT32 retval, Status;
	INT16 I_0, Q_0, LNA, LPF;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 BankNum = pChipCap->SpectrumBankNum;
	RBIST_DESC_T *pSpectrumDesc = &pChipCap->pSpectrumDesc[0];
	EXT_EVENT_RBIST_DUMP_DATA_T *pSpectrumEvent = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pSpectrumEvent */
	pSpectrumEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);

	/* If file is closed, we need to drop this packet. */
	if ((pAd->pSrcf_IQ == NULL) || (pAd->pSrcf_Gain == NULL)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: File is already closed!!\x1b[m\n", __func__));
		return;
	}

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it.*/
	if (pSpectrumEvent->u4PktNum > pAd->SpectrumEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pSpectrumEvent->u4PktNum, pAd->SpectrumEventCnt));
		return;
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	/* Dump I/Q/LNA/LPF data to file */
	for (i = 0; i < SPECTRUM_EVENT_DATA_SAMPLE; i++) {
		Data = le2cpu32(pSpectrumEvent->u4Data[i]);
		os_zero_mem(msg_IQ, CAP_FILE_MSG_LEN);
		os_zero_mem(msg_Gain, CAP_FILE_MSG_LEN);
		/* Parse I/Q/LNA/LPF data and dump these data to file */
		CapNode = Get_System_CapNode_Info(pAd);
		if ((CapNode == pChipCap->SpectrumWF0ADC) || (CapNode == pChipCap->SpectrumWF1ADC)
			|| (CapNode == pChipCap->SpectrumWF2ADC) || (CapNode == pChipCap->SpectrumWF3ADC)) { /* Dump 1-way RXADC */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : Dump 1-way RXADC\n", __func__));

			if (pSpectrumDesc->ucADCRes == 10) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0x3FF);
				I_0 = ((Data & (0x3FF << 10)) >> 10);

				if (Q_0 >= 512)
					Q_0 -= 1024;

				if (I_0 >= 512)
					I_0 -= 1024;

				sprintf(msg_IQ, "%+04d\t%+04d\n", I_0, Q_0);
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				sprintf(msg_Gain, "%+04d\t%+04d\n", LNA, LPF);
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		} else if ((CapNode == pChipCap->SpectrumWF0FIIQ) || (CapNode == pChipCap->SpectrumWF1FIIQ)
				   || (CapNode == pChipCap->SpectrumWF2FIIQ) || (CapNode == pChipCap->SpectrumWF3FIIQ)
				   || (CapNode == pChipCap->SpectrumWF0FDIQ) || (CapNode == pChipCap->SpectrumWF1FDIQ)
				   || (CapNode == pChipCap->SpectrumWF2FDIQ) || (CapNode == pChipCap->SpectrumWF3FDIQ)) { /* Dump 1-way RXIQC */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : Dump 1-way RXIQC\n", __func__));

			if (pSpectrumDesc->ucIQCRes == 12) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0xFFF);
				I_0 = ((Data & (0xFFF << 12)) >> 12);

				if (Q_0 >= 2048)
					Q_0 -= 4096;

				if (I_0 >= 2048)
					I_0 -= 4096;

				sprintf(msg_IQ, "%+05d\t%+05d\n", I_0, Q_0);
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				sprintf(msg_Gain, "%+04d\t%+04d\n", LNA, LPF);
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : 0x%08x\n", __func__, Data));
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	/* Update SpectrumEventCnt */
	pAd->SpectrumEventCnt++;

	if (IS_MT7615(pAd)) {
		/* Check whether is the last FW event of whole data query or not */
		if (pAd->SpectrumEventCnt == MT7615_SPECTRUM_TOTAL_SIZE) {
			/* Reset SpectrumEventCnt */
			pAd->SpectrumEventCnt = 0;
			/* Update SpectrumStatus */
			pAd->SpectrumStatus = CAP_SUCCESS;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\x1b[31m%s: Dump 128K done !! \x1b[m\n", __func__));
			RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
		}
	} else {
		/* Update pSpectrumDesc */
		pSpectrumDesc = &pChipCap->pSpectrumDesc[pAd->SpectrumIdx];

		/* Check whether is the last FW event of data query in the same bank */
		if (pAd->SpectrumEventCnt == pSpectrumDesc->u4BankSize) {
			/* Check whether is the last bank or not */
			if ((pAd->SpectrumIdx + 1) == BankNum) {
				/* Print log to console to indicate data process done */
				{
					UINT32 TotalSize = 0;

					for (i = 0; i < BankNum; i++) {
						/* Update pSpectrumDesc */
						pSpectrumDesc = &pChipCap->pSpectrumDesc[i];
						/* Calculate total size  */
						TotalSize = TotalSize + pSpectrumDesc->u4BankSize;
					}

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\x1b[31m%s: Dump %d K done !! \x1b[m\n", __func__, TotalSize));
				}
				/* Update status */
				pAd->SpectrumStatus = CAP_SUCCESS;
			}

			/* Reset SpectrumEventCnt */
			pAd->SpectrumEventCnt = 0;
			/* OS wait for completion done */
			RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
		}
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event I/Q data handler of wifi-spectrum.
	Return:
	==========================================================================
*/

VOID ExtEventWifiSpectrumUnSolicitIQDataHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	UINT32 Idxi, Type;
	UINT8 msg_IQ[CAP_FILE_MSG_LEN], msg_Gain[CAP_FILE_MSG_LEN], msg_Data[CAP_FILE_MSG_LEN];
	INT32 retval, Status, Data = 0, I = 0, Q = 0, LNA = 0, LPF = 0;
	RTMP_OS_FS_INFO osFSInfo;
	EXT_EVENT_RBIST_DUMP_DATA_T *pSpectrumEvent = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pSpectrumEvent */
	pSpectrumEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);
	pSpectrumEvent->u4DataLen = le2cpu32(pSpectrumEvent->u4DataLen);

	/* If file is closed, we need to drop this packet. */
	if ((pAd->pSrcf_IQ == NULL) || (pAd->pSrcf_Gain == NULL) || (pAd->pSrcf_InPhySniffer == NULL)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: File is already closed!!\x1b[m\n", __func__));
		return;
	}

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it. */
	if (pSpectrumEvent->u4PktNum > pAd->SpectrumEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pSpectrumEvent->u4PktNum, pAd->SpectrumEventCnt));
		return;
	}

	if (pSpectrumEvent->u4DataLen != 0) {
		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		Type = (pAd->SpectrumCapNode & WF_COMM_CR_CAP_NODE_TYPE_MASK) >> WF_COMM_CR_CAP_NODE_TYPE_SHFT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[32m%s: Capture Type = %d\x1b[m\n", __func__, Type));

		/* Dump data to file */
		for (Idxi = 0; Idxi < pSpectrumEvent->u4DataLen; Idxi++) {
			Data = (INT32)le2cpu32(pSpectrumEvent->u4Data[Idxi]);

			if (Type == CAP_INPHYSNIFFER_TYPE) {
				os_zero_mem(msg_Data, CAP_FILE_MSG_LEN);

				sprintf(msg_Data, "%08x\n", Data);
				retval = RtmpOSFileWrite(pAd->pSrcf_InPhySniffer, (RTMP_STRING *)msg_Data, strlen(msg_Data));
			} else {
				os_zero_mem(msg_IQ, CAP_FILE_MSG_LEN);
				os_zero_mem(msg_Gain, CAP_FILE_MSG_LEN);

				if ((Idxi % 4) == 0)
					I = Data;
				if ((Idxi % 4) == 1)
					Q = Data;
				if ((Idxi % 4) == 2)
					LPF = Data;
				if ((Idxi % 4) == 3) {
					LNA = Data;

					sprintf(msg_IQ, "%+05d\t%+05d\n", I, Q);
					retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));

					sprintf(msg_Gain, "%+04d\t%+04d\n", LNA, LPF);
					retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
				}
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : %d\n", __func__, Data));
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

	/* Check whether is the last FW event or not */
	if ((pSpectrumEvent->u4DataLen == 0)
		&& (pSpectrumEvent->u4PktNum == pAd->SpectrumEventCnt)) {

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[31m%s: Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, __func__, pAd->SpectrumEventCnt));

		/* Reset SpectrumEventCnt */
		pAd->SpectrumEventCnt = 0;

		/* Update Spectrum overall status */
		pAd->SpectrumStatus = CAP_SUCCESS;

		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
	} else {
		/* Update SpectrumEventCnt */
		pAd->SpectrumEventCnt++;
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/*
	==========================================================================
	Description:
	Cmd queue raw data handler of ICAP.
	Return:
	==========================================================================
*/
NTSTATUS ICapRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->ICapEventRawDataHandler != NULL) {
		ops->ICapEventRawDataHandler(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/*
	==========================================================================
	Description:
	Extend event of 96-bit raw data parser of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICap96BitDataParser(
	IN RTMP_ADAPTER *pAd)
{
	INT32 retval, Status;
	UINT32 i, j, StopPoint, CapNode;
	BOOLEAN Wrap;
	PUINT32 pTemp_L32Bit = NULL, pTemp_M32Bit = NULL, pTemp_H32Bit = NULL;
	RTMP_REG_PAIR RegStartAddr, RegStopAddr, RegWrap;
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Get RBIST start address */
	os_zero_mem(&RegStartAddr, sizeof(RegStartAddr));
	RegStartAddr.Register = RBISTCR2;
	MtCmdMultipleMacRegAccessRead(pAd, &RegStartAddr, 1);
	/* Get RBIST stop address */
	os_zero_mem(&RegStopAddr, sizeof(RegStopAddr));
	RegStopAddr.Register = RBISTCR9;
	MtCmdMultipleMacRegAccessRead(pAd, &RegStopAddr, 1);
	/* Calculate stop point */
	StopPoint = (RegStopAddr.Value - RegStartAddr.Value) / 4;
	/* Get RBIST wrapper */
	os_zero_mem(&RegWrap, sizeof(RegWrap));
	RegWrap.Register = RBISTCR0;
	MtCmdMultipleMacRegAccessRead(pAd, &RegWrap, 1);
	Wrap = ((RegWrap.Value & BIT(ICAP_WRAP)) >> ICAP_WRAP);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[42m RBISTCR2: 0x%08x, RBISTCR9: 0x%08x, Wrap: %d \x1b[m\n",
			RegStartAddr.Value, RegStopAddr.Value, Wrap));

	/* Re-arrange each buffer by stop address and wrapper */
	if (!Wrap) {
		UINT32 Len, Offset;

		Len = ((BankNum / 3) * BankSmplCnt - StopPoint - 1) * sizeof(UINT32);
		Offset = StopPoint + 1;
		/* Set the rest of redundant data to zero */
		os_zero_mem((pAd->pL32Bit + Offset), Len);
		os_zero_mem((pAd->pM32Bit + Offset), Len);
		os_zero_mem((pAd->pH32Bit + Offset), Len);
	} else {
		UINT32 Len, Offset;

		/* Dynamic allocate memory for pTemp_L32Bit */
		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_L32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Dynamic allocate memory for pTemp_M32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_M32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Dynamic allocate memory for pTemp_H32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_H32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Initialization of data buffer of Temp_L32Bit/Temp_M32Bit/Temp_H32Bit */
		os_zero_mem(pTemp_L32Bit, Len);
		os_zero_mem(pTemp_M32Bit, Len);
		os_zero_mem(pTemp_H32Bit, Len);
		os_move_mem(pTemp_L32Bit, pAd->pL32Bit, Len);
		os_move_mem(pTemp_M32Bit, pAd->pM32Bit, Len);
		os_move_mem(pTemp_H32Bit, pAd->pH32Bit, Len);

		for (i = 0; i < (Len / sizeof(UINT32)); i++) {
			/* Re-arrange data buffer of L32Bit/M32Bit/H32Bit */
			Offset = (StopPoint + 1 + i) % (Len / sizeof(UINT32));
			*(pAd->pL32Bit + i) = *(pTemp_L32Bit + Offset);
			*(pAd->pM32Bit + i) = *(pTemp_M32Bit + Offset);
			*(pAd->pH32Bit + i) = *(pTemp_H32Bit + Offset);
		}
	}

	/* Parse I/Q data and store these data to buffer */
	CapNode = Get_System_CapNode_Info(pAd);
	if (CapNode == pChipCap->ICapPackedADC) { /* 4-way ADC */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[42m4-Way RXADC ------> \x1b[m\n"));

		for (i = 0; i < (pChipCap->ICapADCIQCnt / 3); i++) {
			if (pICapDesc->ucADCRes == 4) {
				/* Parse I/Q data */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 8)) >> 8);         /* Parsing Q0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 4)) >> 4);     /* Parsing Q0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xF);                   /* Parsing Q0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 20)) >> 20);       /* Parsing I0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 16)) >> 16);   /* Parsing I0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 12)) >> 12);   /* Parsing I0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = (*(pAd->pM32Bit + i) & 0xF);                       /* Parsing Q1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 28)) >> 28);   /* Parsing Q1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 12)) >> 12);       /* Parsing I1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 8)) >> 8);     /* Parsing I1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 4)) >> 4);     /* Parsing I1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 24)) >> 24);       /* Parsing Q2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 20)) >> 20);   /* Parsing Q2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 16)) >> 16);   /* Parsing Q2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 4)) >> 4);         /* Parsing I2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_I_TYPE] = (*(pAd->pH32Bit + i) & 0xF);                   /* Parsing I2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);   /* Parsing I2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 16)) >> 16);       /* Parsing Q3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 12)) >> 12);   /* Parsing Q3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 8)) >> 8);     /* Parsing Q3 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 28)) >> 28);       /* Parsing I3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 24)) >> 24);   /* Parsing I3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 20)) >> 20);   /* Parsing I3 */

				/* Calculation of offset binary to decimal */
				for (j = 0; j < 3; j++) {
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 8;
				}
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[42m 4-Way IQC ------>  \x1b[m\n"));

		for (i = 0; i < pChipCap->ICapIQCIQCnt; i++) {
			if (pICapDesc->ucIQCRes == 12) {
				/* Parse I/Q data */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xFFF);                  /* Parsing Q0 */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xFFF << 12)) >> 12);  /* Parsing I0 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xFF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] |= ((*(pAd->pM32Bit + i) & 0xF) << 8);		     /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 4)) >> 4);    /* Parsing I1 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 16)) >> 16);  /* Parsing Q2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);    /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] |= ((*(pAd->pH32Bit + i) & 0xFF) << 4);           /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 8)) >> 8);    /* Parsing Q3 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 20)) >> 20);  /* Parsing I3 */

				/* Calculation of two-complement to decimal */
				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 4096;
			}
		}
	}

	/* Print log to console to indicate data process done */
	{
		UINT32 TotalSize = 0;

		for (i = 0; i < BankNum; i++) {
			/* Update pICapDesc */
			pICapDesc = &pChipCap->pICapDesc[i];
			/* Calculate total size */
			TotalSize = TotalSize + pICapDesc->u4BankSize;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[31m%s: Dump %d K done !! \x1b[m\n", __func__, TotalSize));
	}
	/* Update ICap overall status */
	pAd->ICapStatus = CAP_SUCCESS;

error:
	if (pTemp_L32Bit != NULL)
		os_free_mem(pTemp_L32Bit);

	if (pTemp_M32Bit != NULL)
		os_free_mem(pTemp_M32Bit);

	if (pTemp_H32Bit != NULL)
		os_free_mem(pTemp_H32Bit);

	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event 96-bit raw data handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICapUnSolicit96BitRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	INT32 retval, Status;
	UINT32 i, j;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pICapEvent */
	pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pICapEvent->u4FuncIndex = le2cpu32(pICapEvent->u4FuncIndex);
	pICapEvent->u4PktNum = le2cpu32(pICapEvent->u4PktNum);
	pICapEvent->u4Bank = le2cpu32(pICapEvent->u4Bank);

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it. */
	if (pICapEvent->u4PktNum > pAd->ICapEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pICapEvent->u4PktNum, pAd->ICapEventCnt));
		return;
	}

	/* Dynamic allocate memory for L32Bit/M32Bit/H32Bit buffer */
	{
		UINT32 Len;

		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		if (pAd->pL32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pL32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		if (pAd->pM32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pM32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		if (pAd->pH32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pH32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		/* Initialization of data buffer of L32Bit/M32Bit/H32Bit */
		if ((pAd->ICapL32Cnt == 0) && (pAd->ICapM32Cnt == 0)
			&& (pAd->ICapH32Cnt == 0)) {
			os_zero_mem(pAd->pL32Bit, Len);
			os_zero_mem(pAd->pM32Bit, Len);
			os_zero_mem(pAd->pH32Bit, Len);
		}
	}

	/* Store L32Bit, M32Bit, H32Bit data to each buffer */
	for (j = 0; j < (BankNum / 3); j++) {
		if (pICapEvent->u4Bank == pICapDesc->pLBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pL32Bit[pAd->ICapL32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapL32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pMBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pM32Bit[pAd->ICapM32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapM32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pHBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pH32Bit[pAd->ICapH32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapH32Cnt++;
			}
		}
	}

	/* Print ICap data to console for debugging purpose */
	for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("\x1b[42m 0x%08x\x1b[m\n", le2cpu32(pICapEvent->u4Data[i])));
	}

	/* Update ICapEventCnt */
	pAd->ICapEventCnt++;
	/* Update pICapDesc */
	pICapDesc = &pChipCap->pICapDesc[pAd->ICapIdx];

	/* Check whether is the last FW event or not */
	if (pAd->ICapEventCnt == pICapDesc->u4BankSize) {
		/* Check whether is the last bank or not */
		if ((pAd->ICapIdx + 1) == BankNum)
			ExtEventICap96BitDataParser(pAd);

		/* Reset ICapEventCnt */
		pAd->ICapEventCnt = 0;
		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->ICapDumpDataDone);
	}

error:
	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event I/Q data handler of ICAP.
	Return:
	==========================================================================
*/

VOID ExtEventICapUnSolicitIQDataHandler(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	INT32 Status;
	UINT32 Idxi = 0, Idxj = 0, Idxk = 0, Idxz = 0, u4Cnt = 0;
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = NULL;
	P_RBIST_IQ_DATA_T pIQ_Array = pAd->pIQ_Array;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pICapEvent */
	pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)pData;
	pICapEvent->u4FuncIndex = le2cpu32(pICapEvent->u4FuncIndex);
	pICapEvent->u4PktNum = le2cpu32(pICapEvent->u4PktNum);
	pICapEvent->u4DataLen = le2cpu32(pICapEvent->u4DataLen);
	pICapEvent->u4SmplCnt = le2cpu32(pICapEvent->u4SmplCnt);
	pICapEvent->u4WFCnt = le2cpu32(pICapEvent->u4WFCnt);

	/* If we receive the packet which is out of sequence, we need to drop it */
	if (pICapEvent->u4PktNum > pAd->ICapEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pICapEvent->u4PktNum, pAd->ICapEventCnt));
		return;
	}

	if (pICapEvent->u4DataLen != 0) {
		u4Cnt = pAd->ICapDataCnt + pICapEvent->u4SmplCnt;
		for (Idxi = pAd->ICapDataCnt; Idxi < u4Cnt; Idxi++) {
			for (Idxj = 0; Idxj < pICapEvent->u4WFCnt; Idxj++) {
				pIQ_Array[Idxi].IQ_Array[Idxj][CAP_I_TYPE] = (INT32)le2cpu32(pICapEvent->u4Data[Idxk++]);
				pIQ_Array[Idxi].IQ_Array[Idxj][CAP_Q_TYPE] = (INT32)le2cpu32(pICapEvent->u4Data[Idxk++]);
			}
		}
		pAd->ICapDataCnt = Idxi;

		/* Print ICap data to console for debugging purpose */
		for (Idxz = 0; Idxz < pICapEvent->u4DataLen; Idxz++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\x1b[42m Data[%d] : %d \x1b[m\n", Idxz, (INT32)le2cpu32(pICapEvent->u4Data[Idxz])));
		}

		/* Update ICapEventCnt */
		pAd->ICapEventCnt++;
	}

	/* Check whether is the last FW event or not */
	if ((pICapEvent->u4DataLen == 0)
		&& (pICapEvent->u4PktNum == pAd->ICapEventCnt)) {

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[31m%s: Dump data done, and total pkt cnts = %d!! \x1b[m\n"
				, __func__, pAd->ICapEventCnt));

		/* Reset ICapEventCnt */
		pAd->ICapEventCnt = 0;

		/* Reset ICapDataCnt */
		pAd->ICapDataCnt = 0;

		/* Update ICap overall status */
		pAd->ICapStatus = CAP_SUCCESS;

		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->ICapDumpDataDone);
	}

	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event of querying data-captured status handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICapUnSolicitStatusHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_RBIST_ADDR_T *prICapGetEvent = (EXT_EVENT_RBIST_ADDR_T *)pData;
	/* save iCap result */
	/* send iCap result to QA tool */
	UINT32 *p;

	for (p = (UINT32 *)pData; p < ((UINT32 *)pData+8); p++)
		*p = le2cpu32(*p);
#ifdef CONFIG_ATE
	NdisMoveMemory(&(pAd->ATECtrl.icap_info), prICapGetEvent, sizeof(EXT_EVENT_RBIST_ADDR_T));
#endif /* CONFIG_ATE */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4StartAddr1 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4StartAddr2 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4StartAddr3 = 0x%x\n",
			__func__, prICapGetEvent->u4StartAddr3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4EndAddr = 0x%x\n",
			__func__, prICapGetEvent->u4EndAddr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4StopAddr = 0x%x\n",
			__func__, prICapGetEvent->u4StopAddr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->u4Wrap = 0x%x\n",
			__func__, prICapGetEvent->u4Wrap));
#ifdef CONFIG_ATE
	RTMP_OS_COMPLETE(&(pAd->ATECtrl.cmd_done));
#endif /* CONFIG_ATE */
}
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID ExtEventThroughputBurst(RTMP_ADAPTER *pAd,
									UINT8 *Data, UINT32 Length)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	BOOLEAN fgEnable = FALSE;
	UCHAR pkt_num = 0;
	UINT32 length = 0;

	if (pObj->ioctl_if_type == INT_APCLI) {
#ifdef CONFIG_STA_SUPPORT
		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
#endif
		;
	} else {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
		;
	}

	if (!Data || !wdev) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: Data is NULL\n", __func__));
		return;
	}

	fgEnable = (BOOLEAN)(*Data);
	pAd->bDisableRtsProtect = fgEnable;

	if (pAd->bDisableRtsProtect) {
		pkt_num = MAX_RTS_PKT_THRESHOLD;
		length = MAX_RTS_THRESHOLD;
	} else {
		pkt_num = wlan_operate_get_rts_pkt_thld(wdev);
		length = wlan_operate_get_rts_len_thld(wdev);
	}

	HW_SET_RTS_THLD(pAd, wdev, pkt_num, length);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s::%d\n", __func__, fgEnable));
}


static VOID ExtEventGBand256QamProbeResule(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T pResult;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 u2WlanIdx = WCID_INVALID;

	if (Data != NULL) {
		pResult = (P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T)(Data);
		u2WlanIdx = WCID_GET_H_L(pResult->ucWlanIdxHnVer, pResult->ucWlanIdxL);
		pEntry = &pAd->MacTab.Content[u2WlanIdx];

		if (IS_ENTRY_NONE(pEntry)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s:: pEntry is NONE\n", __func__));
			return;
		}

		if (pResult->ucResult == RA_G_BAND_256QAM_PROBE_SUCCESS)
			pEntry->fgGband256QAMSupport = TRUE;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s::Gband256QAMSupport = %d\n", __func__, pResult->ucResult));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: Data is NULL\n", __func__));
	}
}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

UINT max_line = 130;

static VOID ExtEventAssertDumpHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	struct _EXT_EVENT_ASSERT_DUMP_T *pExtEventAssertDump =
		(struct _EXT_EVENT_ASSERT_DUMP_T *)Data;

	if (max_line) {
		if (max_line == 130)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("**************************************************\n\n"));

		max_line--;
		pExtEventAssertDump->aucBuffer[Length] = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s\n", pExtEventAssertDump->aucBuffer));
	}

#ifdef FW_DUMP_SUPPORT

	if (!pAd->fw_dump_buffer) {
		os_alloc_mem(pAd, &pAd->fw_dump_buffer, pAd->fw_dump_max_size);
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;

		if (pAd->fw_dump_buffer) {
			if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_N9");
			else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_CR4");
			else
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "\0");
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: cannot alloc mem for FW dump\n", __func__));
		}
	}

	if (pAd->fw_dump_buffer) {
		if ((pAd->fw_dump_size + Length) <= pAd->fw_dump_max_size) {
			os_move_mem(pAd->fw_dump_buffer + pAd->fw_dump_size, Data, Length);
			pAd->fw_dump_size += Length;
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: FW dump size too big\n", __func__));
		}
	}

#endif
}

static VOID ExtEventPsSyncHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_PS_SYNC_T *pExtEventPsSync =
		(struct _EXT_EVENT_PS_SYNC_T *)Data;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	UINT16 wcid = WCID_GET_H_L(pExtEventPsSync->ucWtblIndexHnVer, pExtEventPsSync->ucWtblIndexL);
	struct qm_ctl *qm_ctl = &pAd->qm_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;
	NDIS_PACKET *pkt = NULL;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: PsSync Event from FW APPS WIdx=%d PSBit=%d len=%d\n", __func__,
			wcid, pExtEventPsSync->ucPsBit, Length));

	if (IS_TR_WCID_VALID(pAd, wcid)) {
		tr_entry = &tr_ctl->tr_entry[wcid];
		RTMP_SEM_LOCK(&tr_entry->ps_sync_lock);
		tr_entry->ps_state = (pExtEventPsSync->ucPsBit == 0) ? FALSE : TRUE;

		if (tr_entry->ps_state == PWR_ACTIVE) {
			do {
				if (qm_ops->get_psq_pkt)
					pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (pkt) {
					UCHAR q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
					UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					struct wifi_dev *wdev = NULL;

					wdev = pAd->wdev_list[wdev_idx];

					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
				}
			} while (pkt);
		}
		RTMP_SEM_UNLOCK(&tr_entry->ps_sync_lock);
	} else
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wtbl index(%d) is invalid\n", __func__, wcid));
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
VOID ExtEventBfStatusRead(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_BF_STATUS_T *pExtEventBfStatus = (struct _EXT_EVENT_BF_STATUS_T *)Data;
	struct _EXT_EVENT_IBF_STATUS_T *pExtEventIBfStatus = (struct _EXT_EVENT_IBF_STATUS_T *)Data;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#if defined(CONFIG_ATE) && defined(CONFIG_QA)

	if (ATE_ON(pAd))
#ifdef DOT11_HE_AX
		TxBf_Status_Update(pAd, Data, Length);
#else
		HQA_BF_INFO_CB(pAd, Data, Length);
#endif /* DOT11_HE_AX */
#endif /* defined(CONFIG_ATE) && defined(CONFIG_QA) */

	switch (pExtEventBfStatus->ucBfDataFormatID) {
	case BF_PFMU_TAG:
		if (ops->dump_pfmu_tag)
			ops->dump_pfmu_tag(pAd,
					pExtEventBfStatus->fgBFer,
					pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_DATA:
		if (ops->dump_pfmu_data)
			ops->dump_pfmu_data(pAd,
					le2cpu16(pExtEventBfStatus->u2subCarrIdx),
					pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_PN:
		TxBfProfilePnPrint(pExtEventBfStatus->ucBw,
						   pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_MEM_ALLOC_MAP:
		TxBfProfileMemAllocMap(pExtEventBfStatus->aucBuffer);
		break;

	case BF_STAREC:
		StaRecBfRead(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_CAL_PHASE:
		ops->iBFPhaseCalReport(pAd,
									   pExtEventIBfStatus->ucGroup_L_M_H,
									   pExtEventIBfStatus->ucGroup,
									   pExtEventIBfStatus->fgSX2,
									   pExtEventIBfStatus->ucStatus,
									   pExtEventIBfStatus->ucPhaseCalType,
									   pExtEventIBfStatus->aucBuffer);
		break;

	case BF_FBRPT_DBG_INFO:
		TxBfFbRptDbgInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_TXSND_INFO:
		TxBfTxSndInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_PLY_INFO:
		TxBfPlyInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_METRIC_INFO:
		HeRaMuMetricInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_TXCMD_CFG_INFO:
		TxBfTxCmdCfgInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_EVT_SND_CNT_INFO:
		TxBfSndCntInfoPrint(pAd, pExtEventBfStatus->aucBuffer);
		break;

	default:
		break;
	}
}
#endif /* MT_MAC && TXBF_SUPPORT */

VOID MecInfoAmsduEnPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT32 pu4Buf)
{
	P_MEC_EVENT_INFO_T pMecEvtInfo = (P_MEC_EVENT_INFO_T)pu4Buf;
	UINT8 u1AlgoEn;
	UINT16 u2WlanIdx;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[+] AMSDU Algo Enable Status for all %u STAs (sync %u)\n",
														WTBL_MAX_NUM(pAd), MAX_LEN_OF_MAC_TABLE));

	for (u2WlanIdx = 0; u2WlanIdx < (wtbl_max_num / 32); u2WlanIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
			"     au4MecAlgoEnSta[%u]: 0x%08X\n", u2WlanIdx, pMecEvtInfo->rMecCtrl.au4MecAlgoEnSta[u2WlanIdx]));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[+] Connected STA AMSDU Algo Enable Status\n"));

	for (u2WlanIdx = 1; VALID_UCAST_ENTRY_WCID(pAd, u2WlanIdx); u2WlanIdx++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[u2WlanIdx];

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		u1AlgoEn = ((pMecEvtInfo->rMecCtrl.au4MecAlgoEnSta[u2WlanIdx>>0x5] & BIT(u2WlanIdx & 0x1F)) >> u2WlanIdx);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("     WlanIdx %2u, AID %2u, AMSDU Algo Enable: %u\n",
															u2WlanIdx, pEntry->Aid, u1AlgoEn));
	}
}

VOID MecInfoAmsduThrPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT32 pu4Buf)
{
	P_MEC_EVENT_INFO_T pMecEvtInfo = (P_MEC_EVENT_INFO_T)pu4Buf;
	UINT16 u2WlanIdx;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[+] PHY Rate Threshold for AMSDU Length Setting\n"));

	for (u2WlanIdx = MEC_CTRL_BA_NUM_64; u2WlanIdx < MEC_CTRL_BA_NUM_MAX; u2WlanIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("    BA %2u\n", ((u2WlanIdx == MEC_CTRL_BA_NUM_64)?64:256)));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("     Num   Len    Threshold\n"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      1    1.7K      0 Mbps\n"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      2    3.3K   %4u Mbps\n",
															pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_1_DOT_7KB]));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      3    4.8K   %4u Mbps\n",
															pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_3_DOT_3KB]));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      4    6.3K   %4u Mbps\n",
															pMecEvtInfo->rMecCtrl.au2MecAmsduThr[u2WlanIdx][MEC_CTRL_AMSDU_THR_4_DOT_8KB]));
	}
}

VOID ExtEventMecInfoRead(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MEC_INFO_T pExtEventMecInfo = (P_EXT_EVENT_MEC_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MEC Ctrl Info:\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%s] u2ReadType %2u\n", __func__, pExtEventMecInfo->u2ReadType));

	if (pExtEventMecInfo->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		pExtEventMecInfo->u2ReadType & MEC_EVENT_READ_TYPE_ALGO_EN)
		MecInfoAmsduEnPrint(pAd, pExtEventMecInfo->au4Buf);
	if (pExtEventMecInfo->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		pExtEventMecInfo->u2ReadType & MEC_EVENT_READ_TYPE_AMSDU_THR)
		MecInfoAmsduThrPrint(pAd, pExtEventMecInfo->au4Buf);
}


#define NET_DEV_NAME_MAX_LENGTH	16


static VOID ExtEventFwLog2HostHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	UCHAR *dev_name = NULL;
	UCHAR empty_name[] = " ";

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: s2d_index = 0x%x\n", __func__,
			  event_rxd->fw_rxd_2.field.s2d_index));
	dev_name = RtmpOsGetNetDevName(pAd->net_dev);

	if ((dev_name == NULL) || strlen(dev_name) >= NET_DEV_NAME_MAX_LENGTH)
		dev_name = &empty_name[0];

	if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST) {
#ifdef FW_LOG_DUMP
		UINT32 magic_number = le2cpu32(*(UINT32 *)Data);

		if (magic_number == FW_BIN_LOG_MAGIC_NUM) {
			if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
				RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)Data, Length);
			if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
				fw_log_to_ethernet(pAd, Data, Length);
		} else
#endif /* FW_LOG_DUMP */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
		if (pAd->KtoFlashDebug)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("(%s): %s", dev_name, Data));
		else
#endif /*PRE_CAL_TRX_SET1_SUPPORT*/
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("N9 LOG(%s): %s\n", dev_name, Data));
	} else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("CR4 LOG(%s): %s\n", dev_name, Data));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("unknow MCU LOG(%s): %s", dev_name, Data));
	}
}

#ifdef COEX_SUPPORT
static VOID ExtEventBTCoexHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{

	UINT8 SubOpCode;
	MAC_TABLE_ENTRY *pEntry;
	struct _EVENT_EXT_COEXISTENCE_T *coext_event_t =
		(struct _EVENT_EXT_COEXISTENCE_T *)Data;
	SubOpCode = coext_event_t->ucSubOpCode;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("SubOpCode: 0x%x\n", coext_event_t->ucSubOpCode));
	hex_dump("Coex Event payload ", coext_event_t->aucBuffer, Length);

	if (SubOpCode == 0x01) {
		struct _EVENT_COEX_CMD_RESPONSE_T *CoexResp =
			(struct _EVENT_COEX_CMD_RESPONSE_T *)coext_event_t->aucBuffer;
		CoexResp->u4Status = le2cpu32(CoexResp->u4Status);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Cmd_Resp=0x%x\n", CoexResp->u4Status));
	} else if (SubOpCode == 0x02) {
		struct _EVENT_COEX_REPORT_COEX_MODE_T *CoexReportMode =
			(struct _EVENT_COEX_REPORT_COEX_MODE_T *)coext_event_t->aucBuffer;
		CoexReportMode->u4SupportCoexMode = le2cpu32(CoexReportMode->u4SupportCoexMode);
		CoexReportMode->u4CurrentCoexMode = le2cpu32(CoexReportMode->u4CurrentCoexMode);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->SupportCoexMode=0x%x\n", CoexReportMode->u4SupportCoexMode));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->CurrentCoexMode=0x%x\n", CoexReportMode->u4CurrentCoexMode));
		pAd->BtCoexSupportMode = ((CoexReportMode->u4SupportCoexMode) & 0x3);
		pAd->BtCoexMode = ((CoexReportMode->u4CurrentCoexMode) & 0x3);
	} else if (SubOpCode == 0x03) {
		struct _EVENT_COEX_MASK_OFF_TX_RATE_T *CoexMaskTxRate =
			(struct _EVENT_COEX_MASK_OFF_TX_RATE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->MASK_OFF_TX_RATE=0x%x\n", CoexMaskTxRate->ucOn));
	} else if (SubOpCode == 0x04) {
		struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *CoexChgBaSize =
			(struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *)coext_event_t->aucBuffer;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Change_BA_Size ucOn=%d\n", CoexChgBaSize->ucOn));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Change_BA_Size=0x%x\n", CoexChgBaSize->ucRXBASize));

		pEntry = &pAd->MacTab.Content[BSSID_WCID];
		if (CoexChgBaSize->ucOn == 1) {
			BA_REC_ENTRY *pBAEntry = NULL;
			UCHAR Idx;
			UINT16 ba_tx_wsize = 0, ba_rx_wsize = 0;

			Idx = pEntry->BARecWcidArray[0];
			pBAEntry = &pAd->BATable.BARecEntry[Idx];
			pAd->BtCoexBASize = CoexChgBaSize->ucRXBASize;
			ba_tx_wsize = wlan_config_get_ba_tx_wsize(pEntry->wdev);
			ba_rx_wsize = wlan_config_get_ba_rx_wsize(pEntry->wdev);

			if (pBAEntry->BAWinSize == 0)
				pBAEntry->BAWinSize = ba_rx_wsize;

			if (pAd->BtCoexBASize != 0) {
				if (pAd->BtCoexBASize < ba_rx_wsize)
					ba_rx_wsize = pAd->BtCoexBASize;

				if (pAd->BtCoexBASize < ba_tx_wsize)
					ba_tx_wsize = pAd->BtCoexBASize;

				wlan_config_set_ba_txrx_wsize(pEntry->wdev, ba_tx_wsize, ba_rx_wsize);
				ba_ori_session_tear_down(pAd, BSSID_WCID, 0, FALSE);
				ba_rec_session_tear_down(pAd, BSSID_WCID, 0, FALSE);
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexBASize));
			}
		}
	} else if (SubOpCode == 0x05) {
		struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *CoexLimitBeacon =
			(struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->COEX_LIMIT_BEACON_SIZE ucOn =%d\n", CoexLimitBeacon->ucOn));
		pAd->BtCoexBeaconLimit = CoexLimitBeacon->ucOn;
	} else if (SubOpCode == 0x06) {
		struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *CoexExtendBTORoam =
			(struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->EVENT_COEX_EXTEND_BNCTO_ROAMING ucOn =%d\n",
				  CoexExtendBTORoam->ucOn));
	}
}
#endif /* COEX_SUPPORT */

#ifdef BCN_OFFLOAD_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
BOOLEAN MtUpdateBcnToMcuV2(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	struct _BSS_INFO_ARGUMENT_T bss;

	memcpy(&bss, &wdev->bss_info_argument, sizeof(bss));
	bss.u4BssInfoFeature = BSS_INFO_BCN_OFFLOAD_FEATURE;

	MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
			("%s: wdev(%d) bssIdx %d, OM 0x%x, Band %d\n",
			__func__, wdev->wdev_idx, bss.ucBssIndex, bss.OwnMacIdx, bss.ucBandIdx));

	AsicBssInfoUpdate(pAd, &bss);

	return TRUE;
}
#else
BOOLEAN MtUpdateBcnToMcu(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void)
{
	BCN_BUF_STRUCT *bcn_buf = NULL;
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_HE_AX
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;
#endif /* DOT11_HE_AX */
#endif
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	CMD_BCN_OFFLOAD_T_V2 *bcn_offload_v2 = NULL;
#endif
	CMD_BCN_OFFLOAD_T bcn_offload;
	BOOLEAN bSntReq = FALSE;
	UINT16 TimIELocation = 0, CsaIELocation = 0;
#ifdef DOT11_HE_AX
	UINT16 BccIELocation = 0;
#endif
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR UpdatePktType = PKT_V1_BCN;

#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN) {
			os_alloc_mem(NULL, (PUCHAR *)&bcn_offload_v2, sizeof(*bcn_offload_v2));
			if (!bcn_offload_v2) {
				MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
						 ("can not allocate bcn_offload\n"));
				return FALSE;
			}
			os_zero_mem(bcn_offload_v2, sizeof(*bcn_offload_v2));
	} else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#else
	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));
#endif

	if (!wdev) {
		MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 ("%s(): wdev is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
			os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	bcn_buf = &wdev->bcn_buf;

	if (!bcn_buf) {
		MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				 ("%s(): bcn_buf is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	if ((UpdatePktType == PKT_V1_BCN)
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
		|| (UpdatePktType == PKT_V2_BCN)
#endif
	){
		pkt = bcn_buf->BeaconPkt;
		bSntReq = bcn_buf->bBcnSntReq;
		TimIELocation = bcn_buf->TimIELocationInBeacon;
		CsaIELocation = bcn_buf->CsaIELocationInBeacon;
#ifdef DOT11_HE_AX
		BccIELocation = bcn_buf->bcc_ie_location;
#endif
	}

	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = bcn_buf->FrameLen + tx_hw_hdr_len;/* TXD & pkt content. */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN) {
		bcn_offload_v2->ucEnable = bSntReq;
		bcn_offload_v2->ucWlanIdx = 0;/* hardcode at present */
		bcn_offload_v2->ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload_v2->ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload_v2->u2PktLength = len;
		bcn_offload_v2->ucPktType = UpdatePktType;
		bcn_offload_v2->fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload_v2->u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload_v2->u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload_v2->ucCsaCount = wdev->csa_count;
#endif
		NdisCopyMemory(bcn_offload_v2->acPktContent, buf, len);
		MtCmdBcnV2OffloadSet(pAd, bcn_offload_v2);
} else {
		bcn_offload.ucEnable = bSntReq;
		bcn_offload.ucWlanIdx = 0;/* hardcode at present */
		bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload.u2PktLength = len;
		bcn_offload.ucPktType = UpdatePktType;
		bcn_offload.fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload.u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload.u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload.ucCsaCount = wdev->csa_count;
#endif
		NdisCopyMemory(bcn_offload.acPktContent, buf, len);
		MtCmdBcnOffloadSet(pAd, bcn_offload);
}
#else
	{
		bcn_offload.ucEnable = bSntReq;
		bcn_offload.ucWlanIdx = 0;/* hardcode at present */
		bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload.u2PktLength = len;
		bcn_offload.ucPktType = UpdatePktType;
		bcn_offload.fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload.u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload.u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload.ucCsaCount = wdev->csa_count;
#ifdef DOT11_HE_AX
		if (BccIELocation)
			bcn_offload.ucBccCount = bss_color->u.ap_ctrl.bcc_count;
		bcn_offload.u2BccIePos = BccIELocation + tx_hw_hdr_len;
#endif /* DOT11_HE_AX */
#endif
		NdisCopyMemory(bcn_offload.acPktContent, buf, len);
		MtCmdBcnOffloadSet(pAd, bcn_offload);
}
#endif /* BCN_V2_SUPPORT */
	} else {
		MTWF_LOG(DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				("%s(): BeaconPkt is NULL!\n", __func__));
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	if (UpdatePktType == PKT_V2_BCN)
		os_free_mem(bcn_offload_v2);
#endif
		return FALSE;
	}

	return TRUE;
}
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /*BCN_OFFLOAD*/

#if defined(PRETBTT_INT_EVENT_SUPPORT) || defined(BCN_OFFLOAD_SUPPORT)
static VOID ExtEventPretbttIntHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))      ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))   ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	RTMP_HANDLE_PRETBTT_INT_EVENT(pAd);
}
#endif


#if defined(MT7615) || defined(MT7622)
VOID EventThermalProtectHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
	UINT8 Reason;
	struct wifi_dev *wdev;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
#endif
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;
	HLType = EvtThermalProtect->ucHLType;
	Reason = EvtThermalProtect->ucReason;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: HLType = %d, CurrentTemp = %d, Reason = %d\n",
			  __func__, HLType, EvtThermalProtect->cCurrentTemp, Reason));

	if (Reason == THERAML_PROTECTION_REASON_RADIO) {
		/* AsicRadioOnOffCtrl(pAd, HcGetBandByWdev(wdev), WIFI_RADIO_OFF); */
		/* Set Radio off Process*/
		/* Set_RadioOn_Proc(pAd, "0"); */
		RTMP_SET_THERMAL_RADIO_OFF(pAd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Radio Off due to too high temperature.\n"));
	}

}
#else
VOID EventThermalProtDutyNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_THERMAL_PROTECT_DUTY_NOTIFY *event_buf = NULL;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_DUTY_NOTIFY *)Data;

	if (!event_buf)
		return;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("(Thermal Protect) Duty Notify.\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("band_idx: %d, level_idx: %d, duty_percent: %d\n",
		 event_buf->band_idx,
		 event_buf->level_idx,
		 event_buf->duty_percent));
	if (event_buf->temp || event_buf->act_type) {
		if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_TRIG)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				("Trigger Temp = %d\n", event_buf->temp));
		else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("Restore Temp = %d\n", event_buf->temp));
	}
}

VOID EventThermalRadioNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_THERMAL_PROTECT_RADIO_NOTIFY *event_buf = NULL;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_RADIO_NOTIFY *)Data;

	if (!event_buf)
		return;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("(Thermal Protect) Radio Notify.\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("band_idx: %d, level_idx: %d\n",
		 event_buf->band_idx,
		 event_buf->level_idx));
	if (event_buf->temp || event_buf->act_type) {
		if (event_buf->act_type == THERMAL_PROTECT_ACT_TYPE_TRIG)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Trigger Temp = %d\n", event_buf->temp));
		else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Restore Temp = %d\n", event_buf->temp));
	}
}

VOID EventThermalProtInfo(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_THERMAL_PROTECT_MECH_INFO *event_buf = NULL;
	UINT8 prot_type = 0;

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_MECH_INFO *) Data;
	if (!event_buf)
		return;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("band_idx: %d\n", event_buf->band_idx));

	for (prot_type = THERMAL_PROTECT_TYPE_NTX_CTRL;
		prot_type < THERMAL_PROTECT_TYPE_NUM; prot_type++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("prot_type: %d, trig_type: %d\n",
			 event_buf->protect_type[prot_type],
			 event_buf->trigger_type[prot_type]));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("state: %d, enable: %d\n",
			 event_buf->state[prot_type],
			 event_buf->enable[prot_type]));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("trigger_temp: %d, restore_temp: %d\n",
			 event_buf->trigger_temp[prot_type],
			 event_buf->restore_temp[prot_type]));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("recheck_time: %d\n",
			 event_buf->recheck_time[prot_type]));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("--------------------------\n"));
	}
}
VOID EventThermalProtDutyInfo(
	RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_THERMAL_PROTECT_DUTY_INFO *event_buf = NULL;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));

	/* update event buffer pointer */
	event_buf = (struct _EXT_EVENT_THERMAL_PROTECT_DUTY_INFO *) Data;
	if (!event_buf)
		return;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("band_idx: %d\n", event_buf->band_idx));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("duty0: %d, duty1: %d, duty2: %d, duty3: %d\n",
		 event_buf->duty0, event_buf->duty1,
		 event_buf->duty2, event_buf->duty3));
}

VOID EventThermalProtectHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case THERMAL_PROTECT_EVENT_REASON_NOTIFY:
		EventThermalProtectReasonNotify(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_PROT_SHOW_INFO:
		EventThermalProtectInfo(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_DUTY_NOTIFY:
		EventThermalProtDutyNotify(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_RADIO_NOTIFY:
		EventThermalRadioNotify(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_MECH_INFO:
		EventThermalProtInfo(pAd, Data, Length);
		break;

	case THERMAL_PROTECT_EVENT_DUTY_INFO:
		EventThermalProtDutyInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventThermalProtectReasonNotify(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
	UINT8 Reason;

	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;
	HLType = EvtThermalProtect->ucHLType;
	Reason = EvtThermalProtect->ucReason;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: HLType: %d, CurrentTemp: %d, Reason: %d\n",
			  __func__, HLType, EvtThermalProtect->cCurrentTemp, Reason));

	if (Reason == THERAML_PROTECTION_REASON_RADIO) {
		RTMP_SET_THERMAL_RADIO_OFF(pAd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Radio Off due to too high temperature.\n"));
	}
}

VOID EventThermalProtectInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{

	P_EXT_EVENT_THERMAL_PROT_ITEM_INFO_T prEventThermalProtectInfo;
	UINT16  u2AvrgPeriod[WH_TX_AVG_ADMIN_PERIOD_NUM] = {64, 1000};
	UINT8 u1ThermalProtItemIdx;

	/* Get pointer of Event Info Structure */
	prEventThermalProtectInfo = (P_EXT_EVENT_THERMAL_PROT_ITEM_INFO_T)Data;

	/* copy ThermalProt Item Info to Event Info */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==================================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 Thermal Protect Information\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 Admit Duty Period = %d (us)\n", prEventThermalProtectInfo->u1AdmitPeriod * u2AvrgPeriod[prEventThermalProtectInfo->u1AvrgPeriod]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==================================================================================\n"));
	for (u1ThermalProtItemIdx = 0; u1ThermalProtItemIdx < TX_DUTY_LEVEL_NUM; u1ThermalProtItemIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DutyLevel %3d       THERMAL PROTECT ADMIT TIME = %5d (us)    \n",
		u1ThermalProtItemIdx, prEventThermalProtectInfo->u2AdmitDutyLevel[u1ThermalProtItemIdx] * u2AvrgPeriod[prEventThermalProtectInfo->u1AvrgPeriod]/2));
	}
}
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef CFG_TDLS_SUPPORT
static VOID ExtEventTdlsBackToBaseHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TDLS_STATUS_T prEventExtCmdResult =
		(P_EXT_EVENT_TDLS_STATUS_T)Data;
	INT chsw_fw_resp = 0;

	chsw_fw_resp = prEventExtCmdResult->ucResultId;

	switch (chsw_fw_resp) {
	case 1:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("RX RSP_EVT_TYPE_TDLS_STAY_TIME_OUT\n"));
		PCFG_TDLS_STRUCT pCfgTdls =
			&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info;
		cfg_tdls_chsw_resp(pAd, pCfgTdls->CHSWPeerMacAddr,
						   pCfgTdls->ChSwitchTime, pCfgTdls->ChSwitchTimeout, 0);
		break;

	case 2:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("RX RSP_EVT_TYPE_TDLS_BACK_TO_BASE_CHANNEL\n"));
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel = FALSE;
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s : unknown event type %d\n", __func__, chsw_fw_resp));
		break;
	}
}
#endif /* CFG_TDLS_SUPPORT */

#define MAX_MSDU_SIZE 1544
static VOID ExtEventMaxAmsduLengthUpdate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T len_update =
		(P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T)Data;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UINT16 wcid = len_update->ucWlanIdx;
	UINT8 amsdu_len = len_update->ucAmsduLen;

	/* this is a temporary workaround to fix no amsdu at HT20 high rate */
	if (amsdu_len == 0)
		amsdu_len = 1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ExtEventMaxAmsduLengthUpdate: wlan_idx = %d,\
			  amsdu_len = %d\n",
			  wcid, amsdu_len));

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	mac_entry = &pAd->MacTab.Content[wcid];

	if (mac_entry->amsdu_limit_len != 0) {
		mac_entry->amsdu_limit_len_adjust = (mac_entry->amsdu_limit_len < (MAX_MSDU_SIZE * amsdu_len)
						? mac_entry->amsdu_limit_len : (MAX_MSDU_SIZE * amsdu_len));
	}
}

static VOID ExtEventBaTriggerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_CMD_BA_TRIGGER_EVENT_T prEventExtBaTrigger =
		(P_CMD_BA_TRIGGER_EVENT_T)Data;
	UINT16 u2WlanIdx = WCID_GET_H_L(prEventExtBaTrigger->ucWlanIdxHnVer, prEventExtBaTrigger->ucWlanIdxL);

	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;

	tr_entry = &tr_ctl->tr_entry[u2WlanIdx];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("RX P_CMD_BA_TRIGGER_EVENT_T: Wcid=%d, Tid=%d\n",
			  u2WlanIdx, prEventExtBaTrigger->ucTid));

	ba_ori_session_start(pAd, tr_entry, prEventExtBaTrigger->ucTid);
}

static VOID ExtEventTmrCalcuInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TMR_CALCU_INFO_T ptmr_calcu_info;
	TMR_FRM_STRUC *p_tmr_frm;

	ptmr_calcu_info = (P_EXT_EVENT_TMR_CALCU_INFO_T)Data;
	p_tmr_frm = (TMR_FRM_STRUC *)ptmr_calcu_info->aucTmrFrm;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)p_tmr_frm, sizeof(TMR_FRM_STRUC));
#endif
	/*Tmr pkt comes to FW event, fw already take cares of the whole calculation.*/
	TmrReportParser(pAd, p_tmr_frm, TRUE, le2cpu32(ptmr_calcu_info->u4TOAECalibrationResult));
}

static VOID ExtEventCswNotifyHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_CSA_NOTIFY_T csa_notify_event = (P_EXT_EVENT_CSA_NOTIFY_T)Data;
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	UCHAR Index;
	struct wifi_dev *wdevEach = NULL;

	wdev = wdev_search_by_band_omac_idx(pAd,
				csa_notify_event->ucBandIdx,
				csa_notify_event->ucOwnMacIdx);

	if (!wdev)
		return;
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	wdev->csa_count = csa_notify_event->ucChannelSwitchCount;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdevEach = pAd->wdev_list[Index];
		if (wdevEach == NULL)
			continue;
		if (wdevEach->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdevEach) == HcGetBandByWdev(wdev)) {
			wdevEach->csa_count = wdev->csa_count;
		}
	}

	if ((HcIsRfSupport(pAd, RFIC_5GHZ))
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode == RD_SWITCHING_MODE)) {
#ifdef CONFIG_AP_SUPPORT
		pDot11h->CSCount = pDot11h->CSPeriod;
		ChannelSwitchingCountDownProc(pAd, wdev);
#endif /*CONFIG_AP_SUPPORT*/
	}
}

#ifdef DOT11_HE_AX
static VOID ExtEventBccNotifyHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_BCC_NOTIFY_T bcca_notify_event = (P_EXT_EVENT_BCC_NOTIFY_T)Data;
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, bcca_notify_event->ucBandIdx, bcca_notify_event->ucOwnMacIdx);

	if (!wdev)
		return;

	bss_color_event_handler(wdev);
}

static VOID ExtEventMuruHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
#if defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT)

	UINT32 u4EventId = (*(UINT32 *)Data);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, Length));
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

    switch (u4EventId) {
    case MURU_EVENT_TUNE_AP_MUEDCA:

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: MURU_EVENT_TUNE_AP_MUEDCA\n", __func__));
		muru_tune_ap_muedca_handler(pAd, Data, Length);

		break;
    case MURU_EVENT_GET_MURU_STATS_MODE_A:

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: MURU_EVENT_GET_MURU_STATS_MODE_A\n", __func__));
		muru_statistic_handler(pAd, Data, Length);

		break;
    case MURU_EVENT_GET_MUMIMO_STATS_MODE_B:

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s: MURU_EVENT_GET_MUMIMO_STATS_MODE_B\n", __func__));
	muru_mimo_stat_handler(pAd, Data, Length);
	break;
    case MURU_EVENT_GET_DBG_STATS_MODE_C:

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s: MURU_EVENT_GET_DBG_STATS_MODE_C\n", __func__));
	muru_dbg_stat_handler(pAd, Data, Length);
	break;

    default:
		break;
	}
#endif /* defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT) */
}

#endif

static VOID ExtEventBssAcQPktNumHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_EVENT_BSS_ACQ_PKT_NUM_T prEventBssAcQPktNum =
		(P_EVENT_BSS_ACQ_PKT_NUM_T)Data;
	UINT8 i = 0;
	UINT32 sum = 0;
	P_EVENT_PER_BSS_ACQ_PKT_NUM_T prPerBssInfo = NULL;
#ifdef RT_BIG_ENDIAN
	prEventBssAcQPktNum->u4BssMap = le2cpu32(prEventBssAcQPktNum->u4BssMap);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("RX ExtEventBssAcQPktNumHandler: u4BssMap=0x%08X\n",
			  prEventBssAcQPktNum->u4BssMap));

	for (i = 0; (i < CR4_CFG_BSS_NUM) && (prEventBssAcQPktNum->u4BssMap & (1 << i)) ; i++) {
		prPerBssInfo = &prEventBssAcQPktNum->bssPktInfo[i];
#ifdef RT_BIG_ENDIAN
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BK] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BK]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BE] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BE]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VI] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VI]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VO] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VO]);
#endif
		sum =  prPerBssInfo->au4AcqPktCnt[WMM_AC_BK]
			+ prPerBssInfo->au4AcqPktCnt[WMM_AC_VI]
			+ prPerBssInfo->au4AcqPktCnt[WMM_AC_VO];

		if (sum) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("BSS[%d], AC_BK = %d, AC_BE = %d, AC_VI = %d, AC_VO = %d\n",
					  i,
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_BK],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_BE],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_VI],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_VO]));
			pAd->tx_OneSecondnonBEpackets += sum;
		}
	}

	mt_dynamic_wmm_be_tx_op(pAd, ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
}

#ifdef CONFIG_HOTSPOT_R2
static VOID ExtEventReprocessPktHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_CMD_PKT_REPROCESS_EVENT_T prReprocessPktEvt =
		(P_CMD_PKT_REPROCESS_EVENT_T)Data;
	PNDIS_PACKET pPacket = NULL;
	UINT8 Type = 0;
	UINT16 reprocessToken = 0;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct token_tx_pkt_queue *que = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	prReprocessPktEvt->u2MsduToken = le2cpu16(prReprocessPktEvt->u2MsduToken);
	reprocessToken = prReprocessPktEvt->u2MsduToken;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("%s - Reprocess Token ID = %d\n", __func__, reprocessToken));
	que = token_tx_get_queue_by_token_id(cb, reprocessToken);
	pPacket = token_tx_deq(pAd, que, reprocessToken, &Type);

	if (pPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Unexpected Reprocessing TOKEN ID %d, pkt_buf NULL!!!!!!!\n",
				 __func__, reprocessToken));
		return;
	}

	if (que->band_idx == 0)
		que->pkt_token[reprocessToken].Reprocessed = TRUE;
	else
		que->pkt_token[reprocessToken - cap->tkn_info.band0_token_cnt].Reprocessed = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s - Band %d Token ID = %d reprocess set\n", __func__, que->band_idx, reprocessToken));

	if (hotspot_check_dhcp_arp(pAd, pPacket) == TRUE) {
		USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
		MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[Wcid];
		struct wifi_dev *wdev = pMacEntry->wdev;

		RTMP_SET_PACKET_DIRECT_TX(pPacket, 1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - Driver Direct TX\n", __func__));
		send_data_pkt(pAd, wdev, pPacket);
	} else {
		if (Type == TOKEN_TX_DATA)
			RELEASE_NDIS_PACKET_IRQ(pAd, pPacket, NDIS_STATUS_SUCCESS);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Unexpected Reprocessing Mgmt!!\n", __func__));
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
	}
}

static VOID ExtEventGetHotspotCapabilityHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T prGetCapaEvt =
		(P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T)Data;
	UINT8 i = 0;

	for (i = 0; i < 2; i++) {
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[i].HotSpotCtrl;
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[i].WNMCtrl;

		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d  CR4 ======\n", i));
		hotspot_bssflag_dump(prGetCapaEvt->ucHotspotBssFlags[i]);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d  DRIVER ======\n", i));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("pHSCtrl->HotSpotEnable = %d\n"
				  "pHSCtrl->ProxyARPEnable = %d\n"
				  "pHSCtrl->ASANEnable = %d\n"
				  "pHSCtrl->DGAFDisable = %d\n"
				  "pHSCtrl->QosMapEnable = %d\n"
				  , pHSCtrl->HotSpotEnable
				  , pWNMCtrl->ProxyARPEnable
				  , pHSCtrl->bASANEnable
				  , pHSCtrl->DGAFDisable
				  , pHSCtrl->QosMapEnable
				 ));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d END======\n", i));
	}
}
#endif /* CONFIG_HOTSPOT_R2 */

static VOID ext_event_get_cr4_tx_statistics(RTMP_ADAPTER *pAd, UINT8 *data, UINT32 len)
{
	P_EXT_EVENT_GET_CR4_TX_STATISTICS_T tx_statistics =
		(P_EXT_EVENT_GET_CR4_TX_STATISTICS_T)data;
	MAC_TABLE *table = &pAd->MacTab;
	MAC_TABLE_ENTRY *entry = &table->Content[tx_statistics->wlan_index];

	entry->OneSecTxBytes = le2cpu32(tx_statistics->one_sec_tx_bytes);
	entry->one_sec_tx_pkts = le2cpu32(tx_statistics->one_sec_tx_cnts);
	entry->AvgTxBytes = (entry->AvgTxBytes == 0) ?
						entry->OneSecTxBytes :
						((entry->AvgTxBytes + entry->OneSecTxBytes) >> 1);
	entry->avg_tx_pkts = (entry->avg_tx_pkts == 0) ? \
						 entry->one_sec_tx_pkts : \
						 ((entry->avg_tx_pkts + entry->one_sec_tx_pkts) >> 1);
}


#ifdef IGMP_TVM_SUPPORT
static VOID ext_event_get_igmp_multicast_table(RTMP_ADAPTER *pAd, UINT8 *data, UINT32 len)
{
	P_EXT_EVENT_ID_IGMP_MULTICAST_SET_GET_T IgmpMulticastGet =
		(P_EXT_EVENT_ID_IGMP_MULTICAST_SET_GET_T)data;

	if (len < sizeof(EXT_EVENT_ID_IGMP_MULTICAST_SET_GET)) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Cmd Event ength %u is less than required size %lu\n",
			__func__, len, sizeof(EXT_EVENT_ID_IGMP_MULTICAST_SET_GET)));
		return;
	}

	switch (IgmpMulticastGet->ucRspType) {
	case MCAST_RSP_ENTRY_TABLE:
		IgmpSnoopingGetMulticastTable(pAd, IgmpMulticastGet->ucOwnMacIdx, &IgmpMulticastGet->RspData.McastTable);
		break;
	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("%s: Cmd Event ID %u is not supported\n",
					__func__, IgmpMulticastGet->ucRspType));
	}
}
#endif /* IGMP_TVM_SUPPORT */


static VOID ExtEventNFAvgPwr(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT32 avgpwr[4];
	INT8 dBm[4];
	UINT8 i, msb, u1Shift = 0x1;
	UINT32 u4Mask = 0x000000FE;

	P_EXT_EVENT_ENABLE_NOISE_FLOOR_T prEventExtCmdResult = (P_EXT_EVENT_ENABLE_NOISE_FLOOR_T)Data;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Average Power\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WF0 \tWF1 \tWF2 \tWF3"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));

	for (i = 0; i < 4; i++) {
		avgpwr[i] = prEventExtCmdResult->au4avgpwr[i];
		dBm[i] = (INT8)((avgpwr[i] & u4Mask) >> u1Shift);
		msb = ((avgpwr[i] & 0x00000100) >> 8);
		dBm[i] = (dBm[i] & 0x7f);
		dBm[i] = ((0x7f ^ dBm[i]) + 1);

		if (msb)
			dBm[i] = dBm[i] * (-1);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%d \t", dBm[i]));
	}
#ifdef NF_SUPPORT_V2
	pAd->Avg_NF[BAND0] = (dBm[0]+dBm[1])/2;
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		pAd->Avg_NF[BAND1] = (dBm[2]+dBm[3])/2;
#endif
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n"));
}

#ifdef WIFI_MD_COEX_SUPPORT
static NTSTATUS ExtEventFw2apccciMsgHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_FW2APCCCI_T *fw2apccci_msg;

	os_alloc_mem(pAd, (UINT8 **)&fw2apccci_msg, sizeof(fw2apccci_msg) + Length);
	if (fw2apccci_msg == NULL)
		return STATUS_UNSUCCESSFUL;
	os_zero_mem(fw2apccci_msg, sizeof(fw2apccci_msg) + Length);

	fw2apccci_msg->dtb_idx = pAd->fw2apccci_msg.dtb_idx;
	fw2apccci_msg->len = Length;
	os_move_mem((UINT8 *)fw2apccci_msg->data, Data, Length);

	call_wifi_md_coex_notifier(FW_DRIVER_APCCCI, fw2apccci_msg);
	os_free_mem(fw2apccci_msg);
	fw2apccci_msg = NULL;
	return STATUS_SUCCESS;
}
#endif /* WIFI_MD_COEX_SUPPORT */

static VOID ExtEventGetAllStaStats(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8 event_type = *Data;

	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

	switch (event_type) {
	case EVENT_PHY_ALL_TX_RX_RATE:
	{
#ifdef CONFIG_MAP_SUPPORT
		P_EXT_EVENT_TX_RATE_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_TX_RATE_RESULT_T)Data;
		HTTRANSMIT_SETTING LastTxRate;
		HETRANSMIT_SETTING HELastTxRate;
		HTTRANSMIT_SETTING LastRxRate;
		HETRANSMIT_SETTING HELastRxRate;

		for (Idx = 0; Idx < prEventExtCmdResult->ucStaNum; Idx++) {
			pEntry = &pAd->MacTab.Content[prEventExtCmdResult->rAllTxRateResult[Idx].ucWlanIdx];
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
/*For TX rate*/
				if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE == MODE_HE_SU_REMAPPING) {
					HELastTxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE;
					HELastTxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.BW;
					HELastTxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ldpc ? 1 : 0;
					HELastTxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ShortGI ? 1 : 0;
					HELastTxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.STBC;
				} else {
					LastTxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE;
					LastTxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.BW;
					LastTxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ldpc ? 1 : 0;
					LastTxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.ShortGI ? 1 : 0;
					LastTxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.STBC;
				}

				if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE == MODE_HE_SU_REMAPPING) {
					HELastTxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.VhtNss - 1)
						& 0x3) << 5) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					pEntry->map_LastTxRate = (UINT32)HELastTxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE  == MODE_VHT) {
					LastTxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.VhtNss - 1)
						& 0x3) << 4) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MODE  == MODE_OFDM) {
					LastTxRate.field.MCS =
						getLegacyOFDMMCSIndex(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS)
						& 0x0000003F;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				} else {
					LastTxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.MCS;
					pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
				}
/*For RX rate */
				if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode == MODE_HE_SU_REMAPPING) {
					HELastRxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode;
					HELastRxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxBW;
					HELastRxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxCoding ? 1 : 0;
					HELastRxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxGi ? 1 : 0;
					HELastRxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxStbc;
				} else {
					LastRxRate.field.MODE = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode;
					LastRxRate.field.BW = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxBW;
					LastRxRate.field.ldpc = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxCoding ? 1 : 0;
					LastRxRate.field.ShortGI =
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxGi ? 1 : 0;
					LastRxRate.field.STBC = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxStbc;
				}

				if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode == MODE_HE_SU_REMAPPING) {
					HELastRxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxNsts - 1)
						& 0x3) << 5) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					pEntry->map_LastRxRate = (UINT32)HELastRxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode  == MODE_VHT) {
					LastRxRate.field.MCS =
						(((prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxNsts - 1)
						& 0x3) << 4) +
						prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				} else if (prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxMode  == MODE_OFDM) {
					LastRxRate.field.MCS =
						getLegacyOFDMMCSIndex(prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate)
						& 0x0000003F;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				} else {
					LastRxRate.field.MCS = prEventExtCmdResult->rAllTxRateResult[Idx].rEntryTxRate.u1RxRate;
					pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
				}

			}

		}
#endif
	}
	break;
	case EVENT_PHY_TX_STAT_PER_WCID:
	{
#ifdef EAP_STATS_SUPPORT
		P_EXT_EVENT_TX_STAT_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_TX_STAT_RESULT_T)Data;

		prEventExtCmdResult->u2StaNum = le2cpu16(prEventExtCmdResult->u2StaNum);
		for (Idx = 0; Idx < prEventExtCmdResult->u2StaNum; Idx++) {
			P_EXT_EVENT_ONE_TX_STAT_T rTxStatResult = &prEventExtCmdResult->rTxStatResult[Idx];

			rTxStatResult->u2WlanIdx = le2cpu16(rTxStatResult->u2WlanIdx);
			pEntry = &pAd->MacTab.Content[rTxStatResult->u2WlanIdx];
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			if (pEntry && pEntry->wdev && pEntry->Sst == SST_ASSOC &&
			(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)))
#else/*WIFI_IAP_STA_DUMP_FEATURE*/
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
#endif
			{
				pEntry->mpdu_attempts.QuadPart += le2cpu32(rTxStatResult->u4TotalTxCount);
				pEntry->mpdu_retries.QuadPart += le2cpu32(rTxStatResult->u4TotalTxFailCount);
			}

		}
#endif
	}
	break;

	case EVENT_PHY_RX_STAT:
	{
#ifdef EAP_STATS_SUPPORT
		P_EXT_EVENT_RX_STAT_RESULT_T prEventExtCmdResult = (P_EXT_EVENT_RX_STAT_RESULT_T)Data;
		UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);

		for (Idx = 0; Idx < concurrent_bands; Idx++) {
			P_EXT_EVENT_RX_STAT_T rRxStatResult = &prEventExtCmdResult->rRxStatResult[Idx];

			pAd->OneSecMibBucket.PdCount[Idx] = le2cpu16(rRxStatResult->u2PhyRxPdCck) +
												le2cpu16(rRxStatResult->u2PhyRxPdOfdm);
			pAd->OneSecMibBucket.MdrdyCount[Idx] = le2cpu16(rRxStatResult->u2PhyRxMdrdyCntCck) +
													le2cpu16(rRxStatResult->u2PhyRxMdrdyCntOfdm);
		}
#endif
	}
	break;
	case EVENT_PHY_TXRX_AIR_TIME:
	{
		int k;
		P_EXT_EVENT_TXRX_AIRTIME_INFO_T prEventExtCmdResult = (P_EXT_EVENT_TXRX_AIRTIME_INFO_T)Data;

		for (Idx = 0; Idx < prEventExtCmdResult->u2StaNum; Idx++) {
			pEntry = &pAd->MacTab.Content[prEventExtCmdResult->rTxRxAirTimeStat[Idx].u2WlanId];
			if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
				for (k = 0; k < 4; k++) {
					if (pEntry->TxRxTime[k][0] - pEntry->wrapArTxRxTime[k][0] > prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000) {
						/*Wrap around at FW has occurred*/
						/*Update the wrap around value to keep adding that*/
						pEntry->wrapArTxRxTime[k][0] = pEntry->TxRxTime[k][0];
						pEntry->TxRxTime[k][0] += prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000;
					} else {
						pEntry->TxRxTime[k][0] =
							pEntry->wrapArTxRxTime[k][0] + prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblRxTime[k]/1000;
					}
					if (pEntry->TxRxTime[k][1] - pEntry->wrapArTxRxTime[k][1] > prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000) {
						pEntry->wrapArTxRxTime[k][1] = pEntry->TxRxTime[k][1];
						pEntry->TxRxTime[k][1] += prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000;
					} else {
						pEntry->TxRxTime[k][1] =
							pEntry->wrapArTxRxTime[k][1] + prEventExtCmdResult->rTxRxAirTimeStat[Idx].u4WtblTxTime[k]/1000;
					}
					if (pEntry->TxRxTime[k][0] < pEntry->wrapArTxRxTime[k][0]) {
						pEntry->wrapArTxRxTime[k][0] = 0;
					}
				}
			}
		}
	}
	default:
		break;
	}
}

#ifdef TXRX_STAT_SUPPORT
static VOID ExtEventGetStaTxStat(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_STA_TX_STAT_RESULT_T *CmdStaTxStatResult = (EXT_EVENT_STA_TX_STAT_RESULT_T *)Data;
	UINT32 WcidIdx, i;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 band_idx;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pAd->ApCfg.MBSSID[i].stat_bss.Last1TxCnt = 0;
		pAd->ApCfg.MBSSID[i].stat_bss.Last1TxFailCnt = 0;
	}
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		ctrl->rdev[i].pRadioCtrl->Last1TxCnt = 0;
		ctrl->rdev[i].pRadioCtrl->Last1TxFailCnt = 0;
	}
	for (WcidIdx = 0; WcidIdx < wtbl_max_num; WcidIdx++) {
		pEntry = &pAd->MacTab.Content[WcidIdx];
#ifdef WIFI_IAP_STA_DUMP_FEATURE
		if (pEntry && pEntry->wdev &&  (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)) && pEntry->Sst == SST_ASSOC)
#else/*WIFI_IAP_STA_DUMP_FEATURE*/
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
#endif/*NO WIFI_IAP_STA_DUMP_FEATURE*/
		{
			pEntry->LastOneSecPER = 0;
		}
	}
	for (WcidIdx = 0; WcidIdx < wtbl_max_num; WcidIdx++) {
		pEntry = &pAd->MacTab.Content[WcidIdx];
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			band_idx = HcGetBandByWdev(pEntry->wdev);
			CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] =
				le2cpu32(CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx]);
			CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx] =
				le2cpu32(CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]);
			pEntry->LastOneSecTxTotalCountByWtbl = CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			pEntry->LastOneSecTxFailCountByWtbl = CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
#ifdef WIFI_IAP_STA_DUMP_FEATURE
			pEntry->TxFailCountByWtbl += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/
			pEntry->TxSuccessByWtbl += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] -
										CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			ctrl->rdev[band_idx].pRadioCtrl->TotalTxCnt += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			ctrl->rdev[band_idx].pRadioCtrl->TotalTxFailCnt +=  CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			if (ctrl->rdev[band_idx].pRadioCtrl->TotalTxCnt && ctrl->rdev[band_idx].pRadioCtrl->TotalTxFailCnt)
				ctrl->rdev[band_idx].pRadioCtrl->TotalPER = ((100 * (ctrl->rdev[band_idx].pRadioCtrl->TotalTxFailCnt))/
												ctrl->rdev[band_idx].pRadioCtrl->TotalTxCnt);
			ctrl->rdev[band_idx].pRadioCtrl->Last1TxCnt += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			ctrl->rdev[band_idx].pRadioCtrl->Last1TxFailCnt += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			pEntry->pMbss->stat_bss.Last1TxCnt += CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx];
			pEntry->pMbss->stat_bss.Last1TxFailCnt += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			pEntry->pMbss->stat_bss.TxRetriedPacketCount.QuadPart += CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx];
			/*PER in percentage*/
			if (CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx] && CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]) {
				pEntry->LastOneSecPER = ((100 * (CmdStaTxStatResult->PerStaTxFailPktCnt[WcidIdx]))/
													CmdStaTxStatResult->PerStaTxPktCnt[WcidIdx]);
			}
		}
	}
}
#endif

static UINT_8 getTxNss(TX_MODE_RATE eTxRate)
{
    return ((eTxRate  >> TX_NSS_SHITF) & TX_NSS_MASK) + 1;
}

static UINT_8 getTxMode(TX_MODE_RATE eTxRate)
{
    /* TODO: implement it */
    return ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);
}

static UINT_8 getTxModeRemap(TX_MODE_RATE eTxRate, UINT16 u2RuIdx, UINT16 u2Direction)
{
	UINT_8 u1TxMod = ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);

	if (u2RuIdx > 0) {
		if (u2Direction == 0)
			return PHY_MODE_HEMU_REMAPPING;
		else
			return PHY_MODE_HETRIG_REMAPPING;
	} else {
		if (u1TxMod < PHY_MODE_HESU_REMAPPING) {
			return u1TxMod;
		} else if (u1TxMod == PHY_MODE_HESU) {
			return PHY_MODE_HESU_REMAPPING;
		}
	}

	return PHY_MODE_UNKNOWN_REMAPPING;

}

static UINT_8 getTxDcm(TX_MODE_RATE eTxRate)
{
    /*TODO: Implement it*/
    return (eTxRate & DCM_EN && (getTxMode(eTxRate) > PHY_MODE_VHT)) ? 1 : 0;
}

static UINT_8 getTxMcs(TX_MODE_RATE eTxRate)
{
    if (getTxMode(eTxRate) == PHY_MODE_HTMIX) {
		return eTxRate & TX_HT_MCS_MASK;
    } else {
	    return eTxRate & TX_NON_HT_MCS_MASK;
    }
}

static UINT_8 getTxBw(TX_MODE_RATE eTxRate, UINT_8 u1MaxBw)
{
	return u1MaxBw - ((eTxRate >> TX_BW_SHIFT) & TX_BW_MASK);
}

static UINT_8 getTone(UINT16 u2RuIdx)
{
	if (u2RuIdx <= 2)
		return 0;
	else if (u2RuIdx > 2 && u2RuIdx <= 6)
		return 1;
	else if (u2RuIdx > 6 && u2RuIdx <= 14)
		return 2;
	else if (u2RuIdx > 14 && u2RuIdx <= 22)
		return 3;
	else
		return 4;
}

static VOID ExtEventGetRuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_RU_RA_INFO_T *CmdRuRaInfo = (EXT_EVENT_RU_RA_INFO_T *)Data;
	UINT16 u2WlanIdx;
	UINT16 u2RuIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;
	UINT8  u1MaxBw = 0;
	CHAR * phyMode[10] = {"CCK", "OFDM", "HT-MIX", "HT-GF", "VHT", "HE-SU", "HE-EXT-SU", "HE-TB", "HE-MU", "UnKnown"};
	CHAR * dcm[2] = {"", "DCM"};
	CHAR * bw[4] = {"BW20", "BW40", "BW80", "BW160"};
	CHAR * tone[5] = {"996-tone", "484-tone", "242-tone", "<106-tone", "UnKnown"};
	PMAC_TABLE_ENTRY pEntry;


	u2WlanIdx   = CmdRuRaInfo->u2WlanIdx;
	u2RuIdx	    = CmdRuRaInfo->u2RuIdx;
	u2Direction = CmdRuRaInfo->u2Direction;
	u2DumpGroup = CmdRuRaInfo->u2DumpGroup;

	pEntry = &pAd->MacTab.Content[u2WlanIdx];
	u1MaxBw = pEntry->MaxHTPhyMode.field.BW;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nWLAN ID : %d\n", u2WlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RU Idx : %d", u2RuIdx));

	if (u2Direction == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Downlink\n"));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Uplink\n"));

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Short-Term RA\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tCurrRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate)]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tNoRateUpCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tSuggestTxModeRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate)]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tSuggestWF : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1SuggestWF));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStartProbeUpMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS)]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tIsProbeUpPeriod : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownTotalCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownOkCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownOkCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.initRateDownMCS)]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tProbeDownPending : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgProbeDownPending));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStSucceCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStTotalTxCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuPrevRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate)]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStTotalPpduCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tGI : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1Gi));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupFailCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupCheck : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgRuTryupCheck));

		if (CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10));
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Long-Term RA\n"));

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Others\n"));
}

static VOID ExtEventGetMuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_MU_RA_INFO_T *CmdMuRaInfo = (EXT_EVENT_MU_RA_INFO_T *)Data;
	UINT16 u2MuGroupIdx;
	UINT16 u2UserIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;

	u2MuGroupIdx = CmdMuRaInfo->u2MuGroupIdx;
	u2UserIdx    = CmdMuRaInfo->u2UserIdx;
	u2Direction  = CmdMuRaInfo->u2Direction;
	u2DumpGroup  = CmdMuRaInfo->u2DumpGroup;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nMU Group ID : %d\n", u2MuGroupIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("User Idx : %d", u2UserIdx));

	if (u2Direction == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Downlink\n"));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Uplink\n"));

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Short-Term RA\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tCurrRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tNoRateUpCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tSuggestTxModeRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.rSuggestTxModeRate)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tSuggestWF : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1SuggestWF));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStartProbeUpMCS : %x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tIsProbeUpPeriod : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownTotalCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownOkCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownOkCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tInitRateDownMCS : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
			    CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS,
			    getTxMode(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.initRateDownMCS)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tProbeDownPending : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgProbeDownPending));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStSucceCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStTotalTxCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuPrevRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tStTotalPpduCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tGI : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1Gi));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupFailCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupCnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRuTryupCheck : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgRuTryupCheck));

		if (CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10));
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Long-Term RA\n"));

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group: Others\n"));
}

static VOID HeRaEventDispatcher(
	RTMP_ADAPTER *pAd,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *) rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));

	switch (u4EventId) {
	case HERA_RU_RA_INFO_EVENT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: HERA_RU_RA_INFO_EVENT\n", __func__));
		ExtEventGetRuRaInfo(pAd, pData, len);
		break;

	case HERA_MU_RA_INFO_EVENT:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: HERA_MU_RA_INFO_EVENT\n", __func__));
		ExtEventGetMuRaInfo(pAd, pData, len);
		break;

	default:
		break;
	}
}

static BOOLEAN IsUnsolicitedEvent(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_SEQ_NUM(event_rxd) == 0)                          ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_FW_LOG_2_HOST)	||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_THERMAL_PROTECT)  ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_ASSERT_DUMP) ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_PS_SYNC)  ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_HERA_INFO_CTRL))

		return TRUE;

	return FALSE;
}

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
static INT ExtEventPreCalStoreProc(RTMP_ADAPTER *pAd, UINT32 EventId, UINT8 *Data)
{
	UINT32 Offset = 0, IDOffset = 0, LenOffset = 0, HeaderSize = 0, CalDataSize = 0, BitMap = 0;
	UINT32 i, Length, TotalSize, ChGroupId = 0;
	UINT16 DoPreCal = 0;
	static int rf_count;
	int rf_countMax;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[41m %s ------------> \x1b[m\n", __func__));
	rf_countMax = pAd->ChGrpMap == 0x1FF ? 9 : 6;

	switch (EventId) {
	case PRECAL_TXLPF: {
		TXLPF_CAL_INFO_T *prTxLPFGetEvent = (TXLPF_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXLPF_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXLPF_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxLPFGetEvent->u2BitMap = le2cpu16(prTxLPFGetEvent->u2BitMap);
		BitMap = prTxLPFGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM; i++)
			prTxLPFGetEvent->au4Data[i] = le2cpu32(prTxLPFGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXIQ: {
		TXIQ_CAL_INFO_T *prTxIQGetEvent = (TXIQ_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXIQ_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXIQ_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxIQGetEvent->u2BitMap = le2cpu16(prTxIQGetEvent->u2BitMap);
		BitMap = prTxIQGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxIQGetEvent->au4Data[i] = le2cpu32(prTxIQGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXDC: {
		TXDC_CAL_INFO_T *prTxDCGetEvent = (TXDC_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXDC_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXDC_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxDCGetEvent->u2BitMap = le2cpu16(prTxDCGetEvent->u2BitMap);
		BitMap = prTxDCGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxDCGetEvent->au4Data[i] = le2cpu32(prTxDCGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFI: {
		RXFI_CAL_INFO_T *prRxFIGetEvent = (RXFI_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFI_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFI_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFIGetEvent->u2BitMap = le2cpu16(prRxFIGetEvent->u2BitMap);
		BitMap = prRxFIGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*4; i++)
			prRxFIGetEvent->au4Data[i] = le2cpu32(prRxFIGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFD: {
		RXFD_CAL_INFO_T *prRxFDGetEvent = (RXFD_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFD_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFD_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFDGetEvent->u2BitMap = le2cpu16(prRxFDGetEvent->u2BitMap);
		BitMap = prRxFDGetEvent->u2BitMap;
		/* Update group ID */
		prRxFDGetEvent->u4ChGroupId = le2cpu32(prRxFDGetEvent->u4ChGroupId);
		ChGroupId = prRxFDGetEvent->u4ChGroupId;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[33m%s: RXFD ChGroupId %d\x1b[m\n", __func__, ChGroupId));
		for (i = 0;
		i < (SCN_NUM*RX_SWAGC_LNA_NUM)+(SCN_NUM*RX_FDIQ_LPF_GAIN_NUM*RX_FDIQ_TABLE_SIZE*3);
		i++)
			prRxFDGetEvent->au4Data[i] = le2cpu32(prRxFDGetEvent->au4Data[i]);
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\x1b[41m %s: Not support this calibration item !!!! \x1b[m\n", __func__));
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[33m%s: EventID = %d, Bitmp = %x\x1b[m\n", __func__, EventId, BitMap));
	/* Update offset parameter */
	IDOffset = LenOffset = Offset = pAd->PreCalWriteOffSet;
	LenOffset += 4; /*Skip ID field*/
	Offset += 8; /*Skip (ID + Len) field*/

	if (pAd->PreCalStoreBuffer != NULL) {
		/* Store ID */
		os_move_mem(pAd->PreCalStoreBuffer + IDOffset, &EventId, sizeof(EventId));
		/* Store header */
		os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, (ULONG)HeaderSize);
		Offset += HeaderSize; /*Skip header*/
		Data += HeaderSize; /*Skip header*/

		/* Store pre-cal data */
		if (EventId == PRECAL_RXFD) {
			if ((rf_count < rf_countMax) && (BitMap & (1 << ChGroupId))) {
				rf_count++;
				os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, CalDataSize);
				Offset += CalDataSize;
			}
		} else {
			int count = 0;
			int countMax = pAd->ChGrpMap == 0x1FF ? 8 : 5;

			for (i = 0; i < CHANNEL_GROUP_NUM; i++) {
				if (count > countMax)
					break;

				if (BitMap & (1 << i)) {
					count++;
					os_move_mem(pAd->PreCalStoreBuffer + Offset, Data + i * CalDataSize, CalDataSize);
					Offset += CalDataSize;
				}
			}
		}

		/* Update current temp buffer write-offset */
		pAd->PreCalWriteOffSet = Offset;
		/* Calculate buffer size (len + header + data) for each calibration item */
		Length = Offset - LenOffset;
		/* Store buffer size for each calibration item */
		os_move_mem(pAd->PreCalStoreBuffer + LenOffset, &Length, sizeof(Length));

		/* The last event ID - update calibration data to flash */
		if (EventId == PRECAL_RXFI) {
			TotalSize = pAd->PreCalWriteOffSet;
#ifdef RTMP_FLASH_SUPPORT
			/* Write pre-cal data to flash */
			if (pAd->E2pAccessMode == E2P_FLASH_MODE)
				RtmpFlashWrite(pAd->hdev_ctrl, pAd->PreCalStoreBuffer,
					get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, TotalSize);
#endif/* RTMP_FLASH_SUPPORT */
			if (pAd->E2pAccessMode == E2P_BIN_MODE)
				rtmp_cal_write_to_bin(pAd, pAd->PreCalStoreBuffer,
					get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, TotalSize);

			/* Raise DoPreCal bits */
			if (pAd->E2pAccessMode == E2P_FLASH_MODE || pAd->E2pAccessMode == E2P_BIN_MODE) {
				RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);
				DoPreCal |= (1 << 2);
				RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);
			}

			/* Reset parameter */
			pAd->PreCalWriteOffSet = 0;
			rf_count = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\x1b[41m %s: Pre-calibration done !! \x1b[m\n", __func__));
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\x1b[41m %s: PreCalStoreBuffer is NULL !! \x1b[m\n", __func__));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[41m %s <------------ \x1b[m\n", __func__));
	return TRUE;
}

NTSTATUS PreCalTxLPFStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXLPF, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxIQStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXIQ, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxDCStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXDC, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFIStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFI, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFDStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFD, CMDQelmt->buffer);
	return 0;
}
static VOID ExtEventTxLPFCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXLPF_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXLPF_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXLPF, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxLpfInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __func__));
	}

	return;
};

static VOID ExtEventTxIQCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXIQ_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXIQ_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXIQ, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxIqInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __func__));
	}

	return;
};

static VOID ExtEventTxDCCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXDC_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXDC_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXDC, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxDcInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __func__));
	}

	return;
};

static VOID ExtEventRxFICalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFI_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFI_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFI, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFiInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __func__));
	}

	return;
};

static VOID ExtEventRxFDCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFD_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFD_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFD, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFdInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __func__));
	}

	return;
};
#endif  /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID event_get_tx_statistic_handle(struct _RTMP_ADAPTER *pAd, UINT8 *data, UINT32 Length)
{
	struct _EXT_EVENT_TX_STATISTIC_RESULT_T *event =
		(struct _EXT_EVENT_TX_STATISTIC_RESULT_T *)data;
	UINT16 wcid = WCID_GET_H_L(event->ucWlanIdxHnVer, event->ucWlanIdxL);
	struct _MAC_TABLE_ENTRY *entry;
	struct _STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	UINT32 tx_success;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	entry = &pAd->MacTab.Content[wcid];

	if (IS_ENTRY_NONE(entry))
		return;

	tr_entry = &tr_ctl->tr_entry[entry->tr_tb_idx];

	if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
		return;

	event->u4EntryTxCount = le2cpu32(event->u4EntryTxCount);
	event->u4EntryTxFailCount = le2cpu32(event->u4EntryTxFailCount);
	if (entry->TxStatRspCnt) {
		tx_success = event->u4EntryTxCount - event->u4EntryTxFailCount;
		entry->TotalTxSuccessCnt += tx_success;
		entry->one_sec_tx_succ_pkts = tx_success;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): wcid(%d), TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n",
		__func__,
		wcid,
		event->u4EntryTxCount,
		event->u4EntryTxFailCount,
		entry->TotalTxSuccessCnt,
		(entry->TxStatRspCnt) ? "Valid" : "Invalid"));
	entry->TxStatRspCnt++;
}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

static VOID EventExtEventHandler(RTMP_ADAPTER *pAd, UINT8 ExtEID, UINT8 *Data,
								 UINT32 Length, EVENT_RXD *event_rxd)
{
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test;
	UINT32 en_log = 0;
	struct test_log_dump_cb *test_log_dump;
	struct test_operation *ops;

	serv_test = (struct service_test *)(pAd->serv.serv_handle);
	ops = serv_test->test_op;
	test_log_dump = &serv_test->test_log_dump[0];
	en_log = serv_test->en_log;
#endif

	switch (ExtEID) {
	case EXT_EVENT_CMD_RESULT:
		EventExtCmdResult(NULL, Data, Length);
		break;

	case EXT_EVENT_ID_PS_SYNC:
		ExtEventPsSyncHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_FW_LOG_2_HOST:
		ExtEventFwLog2HostHandler(pAd, Data, Length, event_rxd);
		break;
#ifdef COEX_SUPPORT

	case EXT_EVENT_BT_COEX:
		ExtEventBTCoexHandler(pAd, Data, Length);
		break;
#endif /* COEX_SUPPORT */

	case EXT_EVENT_THERMAL_PROTECT:
		EventThermalProtectHandler(pAd, Data, Length);
		break;

#if defined(PRETBTT_INT_EVENT_SUPPORT) || defined(BCN_OFFLOAD_SUPPORT)

	case EXT_EVENT_PRETBTT_INT:
		ExtEventPretbttIntHandler(pAd, Data, Length);
		break;
#endif /*PRETBTT_INT_EVENT_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT

	case EXT_EVENT_ID_ROAMING_DETECTION_NOTIFICATION:
		ExtEventRoamingDetectionHandler(pAd, Data, Length);
		break;
#endif /*CONFIG_STA_SUPPORT*/

	case EXT_EVENT_BEACON_LOSS:
		ExtEventBeaconLostHandler(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	case EXT_EVENT_RA_THROUGHPUT_BURST:
		ExtEventThroughputBurst(pAd, Data, Length);
		break;

	case EXT_EVENT_G_BAND_256QAM_PROBE_RESULT:
		ExtEventGBand256QamProbeResule(pAd, Data, Length);
		break;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	case EXT_EVENT_ID_ASSERT_DUMP:
		ExtEventAssertDumpHandler(pAd, Data, Length, event_rxd);
		break;
#ifdef CFG_TDLS_SUPPORT

	case EXT_EVENT_TDLS_STATUS:
		ExtEventTdlsBackToBaseHandler(pAd, Data, Length);
		break;
#endif
#if defined(MT_MAC) && defined(TXBF_SUPPORT)

	case EXT_EVENT_ID_BF_STATUS_READ:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: EXT_EVENT_ID_BF_STATUS_READ\n", __func__));
		ExtEventBfStatusRead(pAd, Data, Length);
		break;
#endif /* MT_MAC && TXBF_SUPPORT */

	case EXT_EVENT_ID_MEC_INFO_READ:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: EXT_EVENT_ID_MEC_INFO_READ\n", __func__));
		ExtEventMecInfoRead(pAd, Data, Length);
		break;

#ifdef CONFIG_WLAN_SERVICE
	case EXT_EVENT_ID_RF_TEST:
		if (ops->op_evt_rf_test_cb)
			ops->op_evt_rf_test_cb(serv_test->test_winfo, test_log_dump, en_log, Data, Length);
		break;
#else
#ifdef CONFIG_ATE
	case EXT_EVENT_ID_RF_TEST:
		MT_ATERFTestCB(pAd, Data, Length);
		break;
#endif /* CONFIG_ATE */
#endif /* CONFIG_WLAN_SERVICE */

#ifdef MT_DFS_SUPPORT

	case EXT_EVENT_ID_RDD_REPORT:
		ExtEventRddReportHandler(pAd, Data, Length);
		break;
#endif

	case EXT_EVENT_ID_RDD_IPI_HIST_CTRL:
		ExtEventIdlePwrReportHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_MAX_AMSDU_LENGTH_UPDATE:
		ExtEventMaxAmsduLengthUpdate(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BA_TRIGGER:
		ExtEventBaTriggerHandler(pAd, Data, Length);
		break;

#ifdef WIFI_SPECTRUM_SUPPORT
	case EXT_EVENT_ID_WIFI_SPECTRUM:
		ExtEventWifiSpectrumHandler(pAd, Data, Length);
		break;
#endif /* WIFI_SPECTRUM_SUPPORT */

	case EXT_EVENT_CSA_NOTIFY:
		ExtEventCswNotifyHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_TMR_CALCU_INFO:
		ExtEventTmrCalcuInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BSS_ACQ_PKT_NUM:
		ExtEventBssAcQPktNumHandler(pAd, Data, Length);
		break;
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)

	case EXT_EVENT_ID_TXLPF_CAL_INFO:
		ExtEventTxLPFCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXIQ_CAL_INFO:
		ExtEventTxIQCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXDC_CAL_INFO:
		ExtEventTxDCCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFI_CAL_INFO:
		ExtEventRxFICalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFD_CAL_INFO:
		ExtEventRxFDCalInfoHandler(pAd, Data, Length);
		break;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#if defined(MT7615) || defined(MT7622)
#else
	case EXT_EVENT_ID_THERMAL_FEATURE_CTRL:
		EventThermalHandler(pAd, Data, Length);
		break;
#endif /* defined(MT7615) || defined(MT7622) */

	case EXT_EVENT_ID_TX_POWER_FEATURE_CTRL:
		EventTxPowerHandler(pAd, Data, Length);
		break;
#ifdef CONFIG_HOTSPOT_R2

	case EXT_EVENT_ID_INFORM_HOST_REPROCESS_PKT:
		ExtEventReprocessPktHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_GET_CR4_HOTSPOT_CAPABILITY:
		ExtEventGetHotspotCapabilityHandler(pAd, Data, Length);
		break;
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef RED_SUPPORT
	case EXT_EVENT_ID_MPDU_TIME_UPDATE:
		ExtEventMpduTimeHandler(pAd, Data, Length);
		break;
	case EXT_EVENT_ID_RED_TX_RPT:
		ExtEventRedTxReportHandler(pAd, Data, Length);
		break;
#endif
		case EXT_EVENT_ID_HERA_INFO_CTRL:
			HeRaEventDispatcher(pAd, Data, Length);
			break;

	case EXT_EVENT_GET_CR4_TX_STATISTICS:
		ext_event_get_cr4_tx_statistics(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	case EXT_EVENT_GET_TX_STATISTIC:
		event_get_tx_statistic_handle(pAd, Data, Length);
		break;
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
#ifdef DOT11_HE_AX
	case EXT_EVENT_ID_BCC_NOTIFY:
		ExtEventBccNotifyHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_MURU_CTRL:
		ExtEventMuruHandler(pAd, Data, Length);
		break;
#endif

	case EXT_EVENT_ID_TPC_INFO:
		ExtEvenTpcInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RX_STAT_INFO:
		EventRxvHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_ECC_RESULT:
		event_ecc_result(pAd, Data, Length);
		break;
#ifdef TXRX_STAT_SUPPORT
	case EXT_EVENT_ID_GET_STA_TX_STAT:
		ExtEventGetStaTxStat(pAd, Data, Length);
		break;
#endif
	case EXT_EVENT_ID_GET_ALL_STA_STATS:
		ExtEventGetAllStaStats(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_ENABLE_NOISEFLOOR:
		ExtEventNFAvgPwr(pAd, Data, Length);
		break;

#ifdef CFG_SUPPORT_FALCON_SR
	case EXT_EVENT_ID_SR_INFO:
		EventSrHandler(pAd, Data, Length);
		break;
#endif /*CFG_SUPPORT_FALCON_SR*/
	case EXT_EVENT_ID_PHY_STAT_INFO:
		EventPhyStatHandler(pAd, Data, Length);
		break;
#ifdef IGMP_TVM_SUPPORT
		case EXT_EVENT_ID_IGMP_MULTICAST_RESP:
		ext_event_get_igmp_multicast_table(pAd, Data, Length);
		break;
#endif	/* IGMP_TVM_SUPPORT */

	case EXT_EVENT_ID_RXFE_LOSS_COMP:
		EventRxFeCompHandler(pAd, Data, Length);
		break;

#ifdef WIFI_MD_COEX_SUPPORT
	case EXT_EVENT_ID_APCCCI_MSG:
		ExtEventFw2apccciMsgHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_LTE_IDC_REPORT:
		ExtEventLteSafeChnHandler(pAd, Data, Length);
		break;
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
	case EXT_EVENT_ID_CSI_CTRL:
		ExtEventCSICtrl(pAd, Data, Length);
		break;
#endif

	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Ext Event(%x)\n", __func__, ExtEID));
		break;
	}
}

static VOID EventExtGenericEventHandler(UINT8 *Data)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	if (EventExtCmdResult->u4Status == CMD_RESULT_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Success\n", __func__));
	} else if (EventExtCmdResult->u4Status == CMD_RESULT_NONSUPPORT) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Non-Support\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Fail!, EventExtCmdResult.u4Status = 0x%x\n",
				  __func__, EventExtCmdResult->u4Status));
	}
}

static VOID EventGenericEventHandler(UINT8 *Data)
{
	struct _INIT_EVENT_CMD_RESULT *EventCmdResult =
		(struct _INIT_EVENT_CMD_RESULT *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: EventCmdResult.ucCID = 0x%x\n",
			  __func__, EventCmdResult->ucCID));

	if (EventCmdResult->ucStatus == CMD_RESULT_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Success\n", __func__));
	} else if (EventCmdResult->ucStatus == CMD_RESULT_NONSUPPORT) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Non-Support\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Fail!, EventCmdResult.ucStatus = 0x%x\n",
				  __func__, EventCmdResult->ucStatus));
	}
}


static VOID GenericEventHandler(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	switch (EID) {
	case EXT_EVENT:
		EventExtGenericEventHandler(Data);
		break;

	case GENERIC_EVENT:
		EventGenericEventHandler(Data);
		break;

	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Event(%x)\n", __func__, EID));
		break;
	}
}

static VOID UnsolicitedEventHandler(RTMP_ADAPTER *pAd, UINT8 EID, UINT8 ExtEID,
									UINT8 *Data, UINT32 Length, EVENT_RXD *event_rxd)
{
	switch (EID) {
	case EVENT_CH_PRIVILEGE:
		EventChPrivilegeHandler(pAd, Data, Length);
		break;

	case EXT_EVENT:
		EventExtEventHandler(pAd, ExtEID, Data, Length, event_rxd);
		break;

	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Event(%x)\n", __func__, EID));
		break;
	}
}


static BOOLEAN IsRspLenVariableAndMatchSpecificMinLen(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size <= GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0) && IS_CMD_MSG_LEN_VAR_FLAG_SET(msg))
		return TRUE;
	else
		return FALSE;
}



static BOOLEAN IsRspLenNonZeroAndMatchExpected(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size == GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0))
		return TRUE;
	else
		return FALSE;
}

static VOID HandlSeq0AndOtherUnsolicitedEvents(RTMP_ADAPTER *pAd,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UnsolicitedEventHandler(pAd,
							GET_EVENT_FW_RXD_EID(event_rxd),
							GET_EVENT_FW_RXD_EXT_EID(event_rxd),
							GET_EVENT_HDR_ADDR(net_pkt),
							GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd), event_rxd);
}


static void CompleteWaitCmdMsgOrFreeCmdMsg(struct cmd_msg *msg)
{
	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_COMPLETE(&msg->ack_done);
	else {
		DlListDel(&msg->list);
		AndesFreeCmdMsg(msg);
	}
}

static void FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(
	EVENT_RXD *event_rxd, struct cmd_msg *msg)
{
	/* Error occurs!!! dump info for debugging */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("expect response len(%d), command response len(%zd) invalid\n",
			  msg->attr.ctrl.expect_size, GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd)));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x\n",
			  __func__, msg->attr.type, msg->attr.ext_type,
			  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));
	msg->attr.ctrl.expect_size = GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd);
}


static VOID HandleLayer1GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static void CallEventHookHandlerOrDumpErrorMsg(EVENT_RXD *event_rxd,
		struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	if (msg->attr.rsp.handler == NULL) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): rsp_handler is NULL!!!!(cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x)\n",
				  __func__, msg->attr.type, msg->attr.ext_type,
				  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));

		if (GET_EVENT_FW_RXD_EXT_EID(event_rxd) == 0) {
			HandleLayer1GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
									 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
									 GET_EVENT_HDR_ADDR(net_pkt));
		}
	} else {
		msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt),
							  GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
	}
}

static void FwDebugPurposeHandler(EVENT_RXD *event_rxd,
								  struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle FW debug purpose only */
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static VOID HandleNormalLayer1Events(EVENT_RXD *event_rxd,
									 struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static void EventLenVariableHandler(EVENT_RXD *event_rxd,
									struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle event len variable */
	HandleNormalLayer1Events(event_rxd, msg, net_pkt);
}


static void HandleLayer1Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handler normal layer 1 event */
	if (IsRspLenNonZeroAndMatchExpected(event_rxd, msg))
		HandleNormalLayer1Events(event_rxd, msg, net_pkt);
	else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
		/* hanle event len variable */
		EventLenVariableHandler(event_rxd, msg, net_pkt);
	} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
		/* hanle FW debug purpose only */
		FwDebugPurposeHandler(event_rxd, msg, net_pkt);
	} else
		FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
}

static VOID HandleLayer0GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static BOOLEAN IsNormalLayer0Events(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_EID(event_rxd) == MT_FW_START_RSP)            ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_RESTART_DL_RSP)          ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_TARGET_ADDRESS_LEN_RSP)  ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_PATCH_SEM_RSP)           ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == EVENT_ACCESS_REG))
		return TRUE;
	else
		return FALSE;
}
static void HandleLayer0Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handle layer0 generic event */
	if (GET_EVENT_FW_RXD_EID(event_rxd) == GENERIC_EVENT) {
		HandleLayer0GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
								 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
								 GET_EVENT_HDR_ADDR(net_pkt) - 4);
	} else {
		/* handle normal layer0 event */
		if (IsNormalLayer0Events(event_rxd)) {
#ifdef RT_BIG_ENDIAN
			event_rxd->fw_rxd_2.word = cpu2le32(event_rxd->fw_rxd_2.word);
#endif
			msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt) - 4,
								  GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd) + 4);
		} else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
			/* hanle event len is variable */
			EventLenVariableHandler(event_rxd, msg, net_pkt);
		} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
			/* hanle FW debug purpose only */
			FwDebugPurposeHandler(event_rxd, msg, net_pkt);
		} else
			FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
	}
}

static VOID GetMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
									   unsigned long *flags)
{
	RTMP_SPIN_LOCK_IRQSAVE(&((*ctl)->ackq_lock), flags);
}

static VOID ReleaseMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
		unsigned long *flags)
{
	RTMP_SPIN_UNLOCK_IRQRESTORE(&((*ctl)->ackq_lock), flags);
}

static UINT8 GetEventFwRxdSequenceNumber(EVENT_RXD *event_rxd)
{
	return (UINT8)(GET_EVENT_FW_RXD_SEQ_NUM(event_rxd));
}
static VOID HandleSeqNonZeroNormalEvents(RTMP_ADAPTER *pAd,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UINT8 peerSeq;
	struct cmd_msg *msg, *msg_tmp;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	unsigned long flags = 0;

	GetMCUCtrlAckQueueSpinLock(&ctl, &flags);
	DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
		peerSeq = GetEventFwRxdSequenceNumber(event_rxd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: msg->seq=%x, field.seq_num=%x, msg->attr.ctrl.expect_size=%d\n",
				  __func__, msg->seq, peerSeq, msg->attr.ctrl.expect_size));

		if (msg->seq == peerSeq) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s (seq=%d)\n", __func__, msg->seq));

			msg->receive_time_in_jiffies = jiffies;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s: CMD_ID(0x%x 0x%x),total spent %ld ms\n", __func__,
					  msg->attr.type, msg->attr.ext_type, ((msg->receive_time_in_jiffies - msg->sending_time_in_jiffies) * 1000 / OS_HZ)));

			if (GET_EVENT_FW_RXD_EID(event_rxd) == EXT_EVENT)
				HandleLayer1Events(event_rxd, msg, net_pkt);
			else
				HandleLayer0Events(event_rxd, msg, net_pkt);

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s: need_wait=%d\n", __func__,
					  IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)));
			CompleteWaitCmdMsgOrFreeCmdMsg(msg);
			break;
		}
	}
	ReleaseMCUCtrlAckQueueSpinLock(&ctl, &flags);
}

VOID AndesMTRxProcessEvent(RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_EVENT_INFO(GET_EVENT_FW_RXD_LENGTH(event_rxd),
						 GET_EVENT_FW_RXD_PKT_TYPE_ID(event_rxd),
						 GET_EVENT_FW_RXD_EID(event_rxd),
						 GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
						 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
						 GET_EVENT_HDR_ADDR(net_pkt),
						 GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
#endif
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: seq_num=%d, ext_eid=%x\n", __func__,
			  GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
			  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));

	if (IsUnsolicitedEvent(event_rxd))
		HandlSeq0AndOtherUnsolicitedEvents(pAd, event_rxd, net_pkt);
	else
		HandleSeqNonZeroNormalEvents(pAd, event_rxd, net_pkt);
}


VOID AndesMTRxEventHandler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)data;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

#ifdef RT_BIG_ENDIAN
	event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);
#endif
	msg = AndesAllocCmdMsg(pAd, GET_EVENT_FW_RXD_LENGTH(event_rxd));

	if (!msg || !msg->net_pkt)
		return;

	AndesAppendCmdMsg(msg, (char *)data, GET_EVENT_FW_RXD_LENGTH(event_rxd));
	AndesMTRxProcessEvent(pAd, msg);
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

	RTMPFreeNdisPacket(pAd, msg->net_pkt);

#endif
	AndesFreeCmdMsg(msg);
}

#if defined(MT7615) || defined(MT7622)
#else

VOID EventThermalHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case THERMAL_EVENT_THERMAL_SENSOR_BASIC_INFO:
		EventThermalSensorShowInfo(pAd, Data, Length);
		break;

	case THERMAL_EVENT_THERMAL_SENSOR_TASK_RESPONSE:
		EventThermalSensorTaskResp(pAd, Data, Length);
		break;

	default:
		break;
	}
}



VOID EventThermalSensorTaskResp(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T  prEventTheralSensorTaskResp;

	prEventTheralSensorTaskResp = (P_EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): result fp: #%x\n", __func__, prEventTheralSensorTaskResp->u4FuncPtr));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): origin fp: #%p\n", __func__, ThermalTaskAction));

	if (prEventTheralSensorTaskResp->u4FuncPtr)
		ThermalTaskAction(pAd,
							prEventTheralSensorTaskResp->u4PhyIdx,
							prEventTheralSensorTaskResp->u4ThermalTaskProp,
							prEventTheralSensorTaskResp->u1ThermalAdc);
}

VOID EventThermalSensorShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T  prEventTheralSensorItem;
	UINT8 u1ThermalItemIdx;

	prEventTheralSensorItem = (P_EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Thermal Task Num: %d\n\n", prEventTheralSensorItem->u1ThermoTaskNum));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SensorTh: %d (Low), %d (High)\n\n", prEventTheralSensorItem->u1SensorThLow, prEventTheralSensorItem->u1SensorThHigh));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==============================================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Item    Property    fgTrig    Threshold    FuncHandle    Data               \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==============================================================================================\n"));

	/* Thermal State Info Table */
	for (u1ThermalItemIdx = 0; u1ThermalItemIdx < prEventTheralSensorItem->u1ThermoTaskNum; u1ThermalItemIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  %d         %d         %d         %3d         #%x         #%x\n",
															u1ThermalItemIdx,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4ThermalTaskProp,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].fgTrigEn,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u1Thres,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4BoundHandle,
															prEventTheralSensorItem->arThermoItems[u1ThermalItemIdx].u4Data
															));
	}
}
#endif /* defined(MT7615) || defined(MT7622) */

VOID EventTxPowerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

#if defined(MT7615) || defined(MT7622)
	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TXPOWER_EVENT_SHOW_INFO:
		EventTxPowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE:
		EventTxPowerCompTable(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_EPA_STATUS:
		EventTxPowerEPAInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_SENSOR_SHOW_INFO:
		EventThermalStateShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO:
		EventPowerTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TARGET_POWER_INFO_GET:
		break;

	case TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO:
		EventThermalCompTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO:
		EventTxvBbpPowerInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
#else
/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TXPOWER_EVENT_SHOW_INFO:
		EventTxPowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE:
		EventTxPowerCompTable(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_EPA_STATUS:
		EventTxPowerEPAInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO:
		EventPowerTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TARGET_POWER_INFO_GET:
		break;

	case TXPOWER_EVENT_SHOW_ALL_RATE_TXPOWER_INFO:
		EventTxPowerAllRatePowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO:
		EventThermalCompTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO:
		EventTxvBbpPowerInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
#endif /* defined(MT7615) || defined(MT7622) */
}

VOID EventTxPowerShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops && ops->txpower_show_info)
		ops->txpower_show_info(pAd, Data, Length);
}

VOID EventTxPowerAllRatePowerShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arch_ops && arch_ops->arch_txpower_all_rate_info)
		arch_ops->arch_txpower_all_rate_info(pAd, Data, Length);
}

VOID EventTxPowerEPAInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_EPA_STATUS_T  prEventTxPowerEPAInfo;

	prEventTxPowerEPAInfo = (P_EXT_EVENT_EPA_STATUS_T)Data;
	/* update EPA status */
	pAd->fgEPA = prEventTxPowerEPAInfo->fgEPA;
}

NTSTATUS EventTxvBbpPowerInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXV_BBP_POWER_INFO_T prEventTxvBbpPowerInfo;
	UINT8 ucBandIdx = 0;
	UINT8 ucAntIdx = 0;
	UINT8 ucAbsoTxvPower = 0;
	PUINT8 pu1AbsoBbpPower = NULL;
	UINT8 ucNegTxv = 0;
	PUINT8 pu1NegBbp = NULL;
	UINT8 ucWfNum = 0;

	/* get event info buffer contents */
	prEventTxvBbpPowerInfo = (P_EXT_EVENT_TXV_BBP_POWER_INFO_T)Data;

	if (Length != sizeof(EXT_EVENT_TXV_BBP_POWER_INFO_T))
		return STATUS_UNSUCCESSFUL;

	/*WF Path*/
	ucWfNum = prEventTxvBbpPowerInfo->ucWfNum;

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pu1AbsoBbpPower, ucWfNum);

	/*Check allocated memory*/
	if (!pu1AbsoBbpPower)
		return STATUS_UNSUCCESSFUL;

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pu1NegBbp, ucWfNum);

	/*Check allocated memory and Free memory*/
	if (!pu1NegBbp) {
		os_free_mem(pu1AbsoBbpPower);
		return STATUS_UNSUCCESSFUL;
	}

	/* initinal memory */
	os_zero_mem(pu1AbsoBbpPower, ucWfNum);
	os_zero_mem(pu1NegBbp, ucWfNum);

	if (prEventTxvBbpPowerInfo->cTxvPower < 0) {
		ucAbsoTxvPower = ~prEventTxvBbpPowerInfo->cTxvPower + 1;
		ucNegTxv = 1;
	} else
		ucAbsoTxvPower = prEventTxvBbpPowerInfo->cTxvPower;

	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] < 0) {
			*(pu1AbsoBbpPower + ucAntIdx) = ~prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] + 1;
			*(pu1NegBbp + ucAntIdx) = 1;
		} else
			*(pu1AbsoBbpPower + ucAntIdx)  = prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx];
	}

	ucBandIdx = prEventTxvBbpPowerInfo->ucBandIdx;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   Target TXV and BBP POWER INFO (per packet)\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=============================================================================\n"));

	/* get cTxvPower */
	if (prEventTxvBbpPowerInfo->cTxvPower % 2) {
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1)));
	} else {
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1)));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------------------------------------------\n"));

	/* BBP POWER INFO */
	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] % 2) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1)));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1)));

		}
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------------------------------------------\n"));

	/* free allocated memory */
	os_free_mem(pu1AbsoBbpPower);
	os_free_mem(pu1NegBbp);

	return STATUS_SUCCESS;
}



 VOID EventThermalStateShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_STATE_INFO_T  prEventThermalStateInfo;
	UINT8  ucThermalItemIdx;
	CHAR * cThermalItem[THERMO_ITEM_NUM] = {"DPD_CAL          ", /*  0 */
												"OVERHEAT         ", /*  1 */
												"BB_HI            ", /*  2 */
												"BB_LO            ", /*  3 */
												"NTX_PROTECT_HI   ", /*  4 */
												"NTX_PROTECT_LO   ", /*  5 */
												"ADM_PROTECT_HI   ", /*  6 */
												"ADM_PROTECT_LO   ", /*  7 */
												"RF_PROTECT_HI    ", /*  8 */
												"_TSSI_COMP       ", /*  9 */
												"TEMP_COMP_N7_2G4 ", /* 10 */
												"TEMP_COMP_N6_2G4 ", /* 11 */
												"TEMP_COMP_N5_2G4 ", /* 12 */
												"TEMP_COMP_N4_2G4 ", /* 13 */
												"TEMP_COMP_N3_2G4 ", /* 14 */
												"TEMP_COMP_N2_2G4 ", /* 15 */
												"TEMP_COMP_N1_2G4 ", /* 16 */
												"TEMP_COMP_N0_2G4 ", /* 17 */
												"TEMP_COMP_P1_2G4 ", /* 18 */
												"TEMP_COMP_P2_2G4 ", /* 19 */
												"TEMP_COMP_P3_2G4 ", /* 20 */
												"TEMP_COMP_P4_2G4 ", /* 21 */
												"TEMP_COMP_P5_2G4 ", /* 22 */
												"TEMP_COMP_P6_2G4 ", /* 23 */
												"TEMP_COMP_P7_2G4 ", /* 24 */
												"TEMP_COMP_N7_5G  ", /* 25 */
												"TEMP_COMP_N6_5G  ", /* 26 */
												"TEMP_COMP_N5_5G  ", /* 27 */
												"TEMP_COMP_N4_5G  ", /* 28 */
												"TEMP_COMP_N3_5G  ", /* 29 */
												"TEMP_COMP_N2_5G  ", /* 30 */
												"TEMP_COMP_N1_5G  ", /* 31 */
												"TEMP_COMP_N0_5G  ", /* 32 */
												"TEMP_COMP_P1_5G  ", /* 33 */
												"TEMP_COMP_P2_5G  ", /* 34 */
												"TEMP_COMP_P3_5G  ", /* 35 */
												"TEMP_COMP_P4_5G  ", /* 36 */
												"TEMP_COMP_P5_5G  ", /* 37 */
												"TEMP_COMP_P6_5G  ", /* 38 */
												"TEMP_COMP_P7_5G  ", /* 39 */
												"DYNAMIC_G0       "	 /* 40 */
												};

	/* Get pointer of Event Info Structure */
	prEventThermalStateInfo = (P_EXT_EVENT_THERMAL_STATE_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Thermo Item Num: %d\n\n", prEventThermalStateInfo->ucThermoItemsNum));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==================================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("        Item            Type       LowEn       HighEn      LowerBnd       UpperBnd\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==================================================================================\n"));

	/* Thermal State Info Table */
	for (ucThermalItemIdx = 0; ucThermalItemIdx < prEventThermalStateInfo->ucThermoItemsNum; ucThermalItemIdx++) {

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s     %3d        %3d         %3d          %3d            %3d\n",
															cThermalItem[prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoItem],
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoType,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgLowerEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgUpperEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cLowerBound,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cUpperBound
															));
	}
}

VOID EventPowerTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T  prEventPowerTableInfo;
	UINT8  ucPowerTblIdx;
	CHAR * cPowerItem[SKU_TABLE_SIZE] = {"CCK_1M2M    ",
											"CCK_5M11M   ",
											"OFDM_6M9M   ",
											"OFDM_12M18M ",
											"OFDM_24M36M ",
											"OFDM_48M    ",
											"OFDM_54M    ",
											"HT20_MCS0   ",
											"HT20_MCS32  ",
											"HT20_MCS12  ",
											"HT20_MCS34  ",
											"HT20_MCS5   ",
											"HT20_MCS6   ",
											"HT20_MCS7   ",
											"HT40_MCS0   ",
											"HT40_MCS32  ",
											"HT40_MCS12  ",
											"HT40_MCS34  ",
											"HT40_MCS5   ",
											"HT40_MCS6   ",
											"HT40_MCS7   ",
											"VHT20_MCS0  ",
											"VHT20_MCS12 ",
											"VHT20_MCS34 ",
											"VHT20_MCS56 ",
											"VHT20_MCS7  ",
											"VHT20_MCS8  ",
											"VHT20_MCS9  ",
											"VHT40_MCS0  ",
											"VHT40_MCS12 ",
											"VHT40_MCS34 ",
											"VHT40_MCS56 ",
											"VHT40_MCS7  ",
											"VHT40_MCS8  ",
											"VHT40_MCS9  ",
											"VHT80_MCS0  ",
											"VHT80_MCS12 ",
											"VHT80_MCS34 ",
											"VHT80_MCS56 ",
											"VHT80_MCS7  ",
											"VHT80_MCS8  ",
											"VHT80_MCS9  ",
											"VHT160_MCS0 ",
											"VHT160_MCS12",
											"VHT160_MCS34",
											"VHT160_MCS56",
											"VHT160_MCS7 ",
											"VHT160_MCS8 ",
											"VHT160_MCS9 "
											};

	/* Get pointer of Event Info Structure */
	prEventPowerTableInfo = (P_EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   Phy Rate             1SS        2SS        3SS        4SS\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================================\n"));

	for (ucPowerTblIdx = 0; ucPowerTblIdx < SKU_TABLE_SIZE; ucPowerTblIdx++) {

		/* Neglect Invalid rate HT20 MCS32 */
		if (ucPowerTblIdx == HT20M32)
			continue;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s             %2d        %2d        %2d        %2d\n",
															cPowerItem[ucPowerTblIdx],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_1SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_2SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_3SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_4SS]
															));
	}
}


VOID EventThermalCompTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T prEventThermalCompTableInfo = NULL;
	UINT8 ucIdx = 0;
	CHAR * cPowerItem[THERMAL_TABLE_SIZE] = {"-7_Step_Number  ",
												"-6_Step_Number  ",
												"-5_Step_Number  ",
												"-4_Step_Number  ",
												"-3_Step_Number  ",
												"-2_Step_Number  ",
												"-1_Step_Number  ",
												" 0_Step_Number  ",
												" 1_Step_Number  ",
												" 2_Step_Number  ",
												" 3_Step_Number  ",
												" 4_Step_Number  ",
												" 5_Step_Number  ",
												" 6_Step_Number  ",
												" 7_Step_Number  "
											};

	/* Get pointer of Event Info Structure */
	prEventThermalCompTableInfo = (P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T)Data;

	/* Show Thermal Compensation Table */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=========================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       Thermal Compensation Table\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=========================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("  Band Index: %d,  Channel Band: %s\n",
		 prEventThermalCompTableInfo->ucBandIdx, (prEventThermalCompTableInfo->ucBand) ? ("5G") : ("2G")));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------\n"));

	for (ucIdx = 0; ucIdx < THERMAL_TABLE_SIZE; ucIdx++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s    = 0x%x\n", cPowerItem[ucIdx], prEventThermalCompTableInfo->cThermalComp[ucIdx]));

MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("------------------------------------------\n"));

}


VOID EventTxPowerCompTable(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_BACKUP_T  prEventTxPowerCompTable;

	prEventTxPowerCompTable = (P_EXT_EVENT_TXPOWER_BACKUP_T)Data;

	/* update power compensation value table */
	if (prEventTxPowerCompTable->ucBandIdx < DBDC_BAND_NUM)
		os_move_mem(pAd->CommonCfg.cTxPowerCompBackup[prEventTxPowerCompTable->ucBandIdx],
					prEventTxPowerCompTable->cTxPowerCompBackup,
					sizeof(INT8) * SKU_TABLE_SIZE * SKU_TX_SPATIAL_STREAM_NUM);
}

VOID ExtEvenTpcInfoHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TPC_EVENT_DOWNLINK_TABLE:
		EventTpcDownLinkTbl(pAd, Data, Length);
		break;

	case TPC_EVENT_UPLINK_TABLE:
		EventTpcUpLinkTbl(pAd, Data, Length);
		break;
	default:
		break;
	}
}

VOID EventRxvHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  u1EventCategoryID;

	/* Get Event Category ID */
	u1EventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (u1EventCategoryID) {
	case TESTMODE_RXV_EVENT_RXV_REPORT:
		EventRxvReport(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventRxFeCompHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EVENT_PHY_RXFELOSS_T prFeLossComp;
	UINT8 u1AntIdx = 0;

	prFeLossComp = (P_EVENT_PHY_RXFELOSS_T)Data;

	if ((prFeLossComp != NULL) && (prFeLossComp->u1BandIdx < RAM_BAND_NUM)) {

		for (u1AntIdx = 0; u1AntIdx < MAX_ANTENNA_NUM; u1AntIdx++) {
			i1RxFeLossComp[prFeLossComp->u1BandIdx][u1AntIdx] = prFeLossComp->i1FeLossComp[u1AntIdx];
		}
	}
}

#ifdef WIFI_MD_COEX_SUPPORT
VOID ExtEventLteSafeChnHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EVENT_LTE_SAFE_CHN_T pEventLteSafeChn = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, pAd:%p, Data:%p, Lenght:%d\n", __func__, pAd, Data, Length));

	if (Data && Length >= sizeof(P_EVENT_LTE_SAFE_CHN_T)) {
		pEventLteSafeChn = (P_EVENT_LTE_SAFE_CHN_T)Data;

	if (!pEventLteSafeChn || !pEventLteSafeChn->u4SafeChannelBitmask) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:: Error!!! Null pointer!!!\n", __func__));
		return;
	}
		/* Notify wapp */
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
			wapp_send_lte_safe_chn_event(pAd, pEventLteSafeChn->u4SafeChannelBitmask);
		}
#endif
	}

	LteSafeChnEventHandle(pAd, pEventLteSafeChn->u4SafeChannelBitmask);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s end.\n", __func__));

}
#endif

VOID EventTpcDownLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T  prTpcDownlinkInfoTbl;
	UINT8 i;

	prTpcDownlinkInfoTbl = (P_EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TPC DOWNLINK INFO TABLE\n\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		DL Tx Type			Cmd Pwr Ctrl		DL Tc Pwr\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		MU MIMO				%3d					%3d\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->DlTxPwr[TPC_DL_TX_TYPE_MU_MIMO]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		OFDMA				%3d					%3d\n\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[TPC_DL_TX_TYPE_MU_OFDMA],
														prTpcDownlinkInfoTbl->DlTxPwr[TPC_DL_TX_TYPE_MU_OFDMA]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		WLAN		TxPwrAlpha MU_MIMO		TxPwrAlpha OFDMA\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	for (i = 0; i < 32; i++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%3d				%3d					%3d\n",
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].u2WlanId,
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].DlTxPwrAlpha[TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].DlTxPwrAlpha[TPC_DL_TX_TYPE_MU_OFDMA]));
}

VOID EventTpcUpLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TPC_INFO_UPLINK_TABLE_T  prTpcUplinkInfoTbl;
	UINT8 i;

	prTpcUplinkInfoTbl = (P_EXT_EVENT_TPC_INFO_UPLINK_TABLE_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TPC UPLINK INFO TABLE\n\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP INFO: AP TX Power = %d\n", prTpcUplinkInfoTbl->u1ApTxPwr));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		WLAN		TargetRssi		PwrHeadRoom		MinPwrFlag\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	for (i = 0; i < 32; i++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%3d			%3d			%3d			%3d\n",
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].u2WlanId,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1TargetRssi,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1PwrHeadRoom,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.fgMinPwr));
}

VOID EventRxvReport(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T  prEventRxvReport;

	prEventRxvReport = (P_EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("====================================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("RXV Report                      \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("====================================================================================\n"));
	/* RXV entry content */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("RxvCallBackType   = %d\n", prEventRxvReport->u1RxvCbEntryType));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostMd            = %d\n", prEventRxvReport->u2PostMd));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostRssiRu        = %d\n", prEventRxvReport->u2PostRssiRu));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("RxCeLtfSnr        = %d\n", prEventRxvReport->u1RxCeLtfSnr));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("TftFoe            = %d\n", prEventRxvReport->u4TftFoe));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostNoiseFloorRx0 = %d\n", prEventRxvReport->u1PostNoiseFloorRx0));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostNoiseFloorRx1 = %d\n", prEventRxvReport->u1PostNoiseFloorRx1));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostNoiseFloorRx2 = %d\n", prEventRxvReport->u1PostNoiseFloorRx2));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("PostNoiseFloorRx3 = %d\n", prEventRxvReport->u1PostNoiseFloorRx3));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("DecUserNum        = %d\n", prEventRxvReport->u1DecUserNum));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("UserRate          = %d\n", prEventRxvReport->u1UserRate));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("UserStreamNum     = %d\n", prEventRxvReport->u1UserStreamNum));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("UserRuAlloc       = %d\n", prEventRxvReport->u1UserRuAlloc));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("MuAid             = %d\n", prEventRxvReport->u2MuAid));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("RxFcsErr          = %d\n", prEventRxvReport->fgRxFcsErr));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr0    = %d\n", prEventRxvReport->u4OfdmRu26Snr0));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr1    = %d\n", prEventRxvReport->u4OfdmRu26Snr1));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr2    = %d\n", prEventRxvReport->u4OfdmRu26Snr2));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr3    = %d\n", prEventRxvReport->u4OfdmRu26Snr3));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr4    = %d\n", prEventRxvReport->u4OfdmRu26Snr4));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr5    = %d\n", prEventRxvReport->u4OfdmRu26Snr5));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr6    = %d\n", prEventRxvReport->u4OfdmRu26Snr6));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr7    = %d\n", prEventRxvReport->u4OfdmRu26Snr7));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr8    = %d\n", prEventRxvReport->u4OfdmRu26Snr8));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("OfdmRu26Snr9    = %d\n", prEventRxvReport->u4OfdmRu26Snr9));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("====================================================================================\n"));
}

VOID event_ecc_result(RTMP_ADAPTER *ad, UINT8 *data, UINT32 length)
{
	EVENT_ECC_RES_T *event_rcc_res = (EVENT_ECC_RES_T *)data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ucEccCmdId = %d, ucIsResFail= %d\n", event_rcc_res->ucEccCmdId, event_rcc_res->ucIsResFail));

	hex_dump_with_lvl("Dqx", event_rcc_res->aucDqxBuffer, event_rcc_res->ucDqxDataLength, DBG_LVL_OFF);
	hex_dump_with_lvl("Dqy", event_rcc_res->aucDqyBuffer, event_rcc_res->ucDqyDataLength, DBG_LVL_OFF);
}


#ifdef LED_CONTROL_SUPPORT
#if defined(MT7915)
INT AndesLedEnhanceOP(
	RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode){

	INT32 ret = 0, i = 0;
	struct cmd_msg *msg;
	CHAR *pos, *buf;
	UCHAR *p_pattern = NULL;
	UINT32 len = 0;
	UINT32 patt_len = 0;

	struct _CMD_ATTRIBUTE attr = {0};

	led_tx_blink_pattern tx_blink_pattern;
	led_pure_blink_pattern pure_blink_pattern;
	led_mix_tx_pure_blink_pattern mix_tx_pure_blink_pattern;

	led_control_event event;

	len = sizeof(led_control_event);

	/*init led control event*/
	NdisZeroMemory(&event, len);

	/*init pattern specific structure*/
	NdisZeroMemory(&tx_blink_pattern, sizeof(led_tx_blink_pattern));
	NdisZeroMemory(&pure_blink_pattern, sizeof(led_pure_blink_pattern));
	NdisZeroMemory(&mix_tx_pure_blink_pattern, sizeof(led_mix_tx_pure_blink_pattern));

	/*fill common parameters*/
	event.led_ver = 2;
	event.led_idx = led_idx;
	event.reverse_polarity = reverse_polarity;

	/*handle led_control_mode*/
	switch (led_control_mode) {
	case LED_SOLID_ON:
		event.pattern_category = LED_CATEGORY_0_SOLID_ON;
		break;
	case LED_SOLID_OFF:
		event.pattern_category = LED_CATEGORY_1_SOLID_OFF;
		break;
	case LED_TX_BLINKING:
		event.pattern_category = LED_CATEGORY_2_TX_BLINK;
		event.band_select = band;
		tx_blink_pattern.led_combine = 0;
		tx_blink_pattern.blink_mode = blink_mode;
		tx_blink_pattern.tx_blink_on_time = 70 ; /* 70 ms */
		tx_blink_pattern.tx_blink_off_time = 30; /* 30 ms */

		p_pattern = (UCHAR *)&tx_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_tx_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_500MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 500;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 500;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_250MS_ON_250MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 250;
		pure_blink_pattern.s0_off_time = 250;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 250;
		pure_blink_pattern.s1_off_time = 250;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_170MS_ON_170MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 170;
		pure_blink_pattern.s0_off_time = 170;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 170;
		pure_blink_pattern.s1_off_time = 170;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_100MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1000;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 100;
		pure_blink_pattern.s1_total_time = 1000;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 100;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	case LED_BLINKING_500MS_ON_700MS_OFF:
		event.pattern_category = LED_CATEGORY_3_PURE_BLINK;
		pure_blink_pattern.replay_mode = 0;
		pure_blink_pattern.s0_total_time = 1200;
		pure_blink_pattern.s0_on_time = 500;
		pure_blink_pattern.s0_off_time = 700;
		pure_blink_pattern.s1_total_time = 1200;
		pure_blink_pattern.s1_on_time = 500;
		pure_blink_pattern.s1_off_time = 700;

		p_pattern = (UCHAR *)&pure_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_pure_blink_pattern);
		break;
	/*WPS*/
	/* Cases needs timer are handled at previous stage.
	case LED_WPS_3_BLINKING_PER_SECOND_FOR_4_SECONDS:
		break;
	case LED_WPS_5S_ON_3S_OFF_THEN_BLINKING:
		break;
	case LED_WPS_5S_ON:
		break; */
	/*Generic fix blinking format*/
	case LED_GENERAL_FIX_BLINKING_FORMAT:
		event.pattern_category = LED_CATEGORY_2_TX_BLINK;
		event.band_select = band;
		tx_blink_pattern.led_combine = 0;
		tx_blink_pattern.blink_mode = blink_mode;
		tx_blink_pattern.tx_blink_on_time = on_time*10;
		tx_blink_pattern.tx_blink_off_time = off_time*10;

		p_pattern = (UCHAR *)&tx_blink_pattern;
		/*update tlv len*/
		patt_len = sizeof(led_tx_blink_pattern);
		break;
	default:
		break;
	}

	len += patt_len;
	msg = AndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	os_alloc_mem(pAd, (UCHAR **)&buf, len);

	if (buf == NULL)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(buf, len);
	pos = buf;


	NdisMoveMemory(pos, &event, sizeof(led_control_event));
	if (p_pattern != NULL)
		NdisMoveMemory(pos + sizeof(led_control_event), p_pattern, patt_len);

	hex_dump("led_send_fw_cmd: ", buf, len);

	/*Debug Log*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\nled_send_fw_cmd:\n"));
	i = 0;
	while (i < sizeof(led_control_event)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%02x", buf[i]));
		i++;
	}

	if (p_pattern != NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\npattern content:\n"));
		i = 0;
		while (i < patt_len) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%02x", p_pattern[i]));
			i++;
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
	} /*End of Debug Log*/

	AndesAppendCmdMsg(msg, (char *)buf, len);
	ret = AndesSendCmdMsg(pAd, msg);
	os_free_mem(buf);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __func__, ret));
	return ret;

}

INT AndesLedGpioMap(RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type)
{
	INT32 ret = 0, i = 0;
	struct cmd_msg *msg;
	CHAR *buf;
	UINT32 len;

	struct _CMD_ATTRIBUTE attr = {0};

	/*init led control event*/
	led_control_event event;

	len = sizeof(led_control_event);
	NdisZeroMemory(&event, len);

	/*fill common parameters*/
	event.led_ver = 2;
	event.led_idx = led_index;
	event.pattern_category = LED_CATEGORY_5_GPIO_SETTING;
	event.rsvd_1 |= ctr_type;
	event.rsvd_1 |= map_index << 1;

	msg = AndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	os_alloc_mem(pAd, (UCHAR **)&buf, len);

	if (!buf) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	NdisZeroMemory(buf, len);
	NdisMoveMemory(buf, &event, sizeof(led_control_event));

	hex_dump("led_send_fw_cmd: ", buf, len);

	/*Debug Log*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nled_send_fw_cmd:\n"));
	i = 0;
	while (i < sizeof(led_control_event)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", buf[i]));
		i++;
	}
	/*End of Debug Log*/

	AndesAppendCmdMsg(msg, (char *)buf, len);
	AndesSendCmdMsg(pAd, msg);
	os_free_mem(buf);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nled_%d map to gpio_%d...\n", led_index, map_index));
	return ret;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

#endif

#endif

#ifdef FQ_SCH_SUPPORT
VOID ExtEventMpduTimeHandler_fp(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_FQ_UPDATE_T prMpduTimeFQEvt = (P_EXT_EVENT_MPDU_TIME_FQ_UPDATE_T)Data;
	PMAC_TABLE_ENTRY pEntry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	UINT16 i;
	UINT8 ucInUseSta = 0;
	UINT8 not_active = 0;
	UINT32 MpduTime = 0;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;

	RTMP_IO_READ32(pAd->hdev_ctrl, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[3]);

	if (prMpduTimeFQEvt->ucfgValid) {
		for (i = 0; i < RED_STA_REC_NUM; i++) {

			pEntry = &pAd->MacTab.Content[i];
			if (IS_ENTRY_NONE(pEntry))
				continue;

			if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid))
				continue;

			tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
			if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
				/*Calcualte in used station number */
				if ((i > 0) && (i < (WTBL_MAX_NUM(pAd) - MAX_MBSSID_NUM(pAd))) &&
					(prMpduTimeFQEvt->arMpduTime[i] > 0))
					ucInUseSta++;
			}

			if (pAd->fq_ctrl.enable & FQ_READY) {
				MpduTime = prMpduTimeFQEvt->arMpduTime[i];
				if (fq_update_thMax(pAd, tr_entry, i, MpduTime, dwrr_quantum, Value)
					!= NDIS_STATUS_SUCCESS)
					not_active++;
			}
		}

		/*update in used station number */
		pAd->red_in_use_sta = ucInUseSta;
		if (pAd->fq_ctrl.enable & FQ_READY)
			pAd->fq_ctrl.nactive = pAd->red_in_use_sta - not_active;
	}
}
#endif /* defined(FQ_SCH_SUPPORT) */

#ifdef RED_SUPPORT
VOID ExtEventMpduTimeHandler_avg(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_UPDATE_T prMpduTimeEvt = (P_EXT_EVENT_MPDU_TIME_UPDATE_T)Data;
	P_MPDU_SHORT_AVG_TIME_UPDATE_T prMpduShortTimeUpdate  = NULL;
	/* P_STA_RECORD_T prStaRec = cnmGetStaRecByIndex(0); */
	PMAC_TABLE_ENTRY pEntry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	UINT16 i;
	UINT8 ucInUseSta = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucBW = 0;
	HTTRANSMIT_SETTING tx_rate;
#ifdef VOW_SUPPORT
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#else
	UINT8 fgATCorWATFEnable = 0;
#endif
#ifdef FQ_SCH_SUPPORT
	UINT8 j;
	UINT8 active = 0, bcmc_active = 0, pow_save = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry_tmp;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;
#endif
	UINT32 *staInUseBitmap;
	UINT32 staInUseBitmap_tmp;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	UINT8 wordlen = (prMpduTimeEvt->Reserve[0] == MPDU_TIME_BITMAP_TAG) ?
			prMpduTimeEvt->Reserve[1] : ((wtbl_max_num + 31)/(sizeof(UINT32)<<3));

	prMpduShortTimeUpdate = (P_MPDU_SHORT_AVG_TIME_UPDATE_T)(Data +
				sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T) +
				(wordlen<<2));
	staInUseBitmap = (UINT32 *)(Data + sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T));
#ifdef FQ_SCH_SUPPORT
	for (j = 0; j < wordlen; j++)
		pAd->fq_ctrl.staInUseBitmap[j] = staInUseBitmap[j];

	RTMP_IO_READ32(pAd->hdev_ctrl, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[3]);
#endif
	for (i = 0; i < RED_STA_REC_NUM; i++) {
		staInUseBitmap_tmp = staInUseBitmap[i>>RED_INUSE_BITSHIFT];
#ifdef RT_BIG_ENDIAN
		staInUseBitmap_tmp = le2cpu32(staInUseBitmap_tmp);
#endif
		if ((staInUseBitmap_tmp & (1<<(i&RED_INUSE_BITMASK))) == 0) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			continue;
		}
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;
			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}
#ifdef RT_BIG_ENDIAN
		prMpduShortTimeUpdate->arN9TxARCnt = le2cpu16(prMpduShortTimeUpdate->arN9TxARCnt);
		prMpduShortTimeUpdate->arN9TxFRCnt = le2cpu16(prMpduShortTimeUpdate->arN9TxFRCnt);
		prMpduShortTimeUpdate->arMpduTime = le2cpu32(prMpduShortTimeUpdate->arMpduTime);
		prMpduShortTimeUpdate->arMpduTime_avg = le2cpu32(prMpduShortTimeUpdate->arMpduTime_avg);
#endif

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
		prRedSta = &pAd->red_sta[i];

		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			prRedSta = &pAd->red_sta[i];
			if (pAd->fq_ctrl.enable & FQ_READY) {
				prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
				prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					(prMpduShortTimeUpdate->arMpduTime) :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);
				prRedSta->tx_msdu_cnt = 0;
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS) {
					active++;
					bcmc_active++;
				}
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];

		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
			prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
			prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					prMpduShortTimeUpdate->arMpduTime :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);

			prRedSta->tx_msdu_cnt = 0;
			ucPhyMode = prMpduShortTimeUpdate->arPhymodeBW >> 4;
			tx_rate.field.MODE = ucPhyMode;
			ucBW = prMpduShortTimeUpdate->arPhymodeBW & 0x0F;
			tx_rate.field.BW = ucBW;
			tx_rate.field.MCS = prMpduShortTimeUpdate->ucMcsShortGI >> 1;
			tx_rate.field.ShortGI = prMpduShortTimeUpdate->ucMcsShortGI & 0x01;
			pEntry->LastTxRate = (UINT32)tx_rate.word;

			/*If TxForceCnt/TxCnt > 25% and not badnode, then reset to default.*/
			RedCalForceRateRatio(i,
						prMpduShortTimeUpdate->arN9TxARCnt,
						prMpduShortTimeUpdate->arN9TxFRCnt,
						 pAd);

			/*Calcualte in used station number */
			if ((i > 0) && (i < wtbl_max_num) &&
					(prMpduShortTimeUpdate->arMpduTime > 0))
				ucInUseSta++;

			if (prRedSta->i4MpduTime < 0)
				RedResetSta(i, ucPhyMode, ucBW, pAd);
			else
				UpdateThreshold(i, pAd);

			UpdateAirtimeRatio(i, prMpduShortTimeUpdate->arAirtimeRatio
						, fgATCorWATFEnable, pAd);
#ifdef FQ_SCH_SUPPORT
			if (pAd->fq_ctrl.enable & FQ_READY) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS)
					active++;
			}
#endif
		} else {
			/*fgIsInUse == FALSE, don't care PHY mode. */
			RedResetSta(i, MODE_CCK, BW_20, pAd);
			prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;
			prRedSta->tx_msdu_cnt = 0;
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
		}
#endif
		}
#ifdef FQ_SCH_SUPPORT
		if (tr_entry->PsMode == PWR_SAVE)
			pow_save++;
#endif
		prMpduShortTimeUpdate++;
	}

	/*update in used station number */
	pAd->red_in_use_sta = ucInUseSta;
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_READY) {
		pAd->fq_ctrl.nactive = active;
		pAd->fq_ctrl.nbcmc_active = bcmc_active;
		pAd->fq_ctrl.npow_save = pow_save;
		fq_clean_list(pAd, WMM_NUM_OF_AC);
	}
#endif
}

VOID ExtEventMpduTimeHandler_v3(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_UPDATE_T prMpduTimeEvt = (P_EXT_EVENT_MPDU_TIME_UPDATE_T)Data;
	P_MPDU_SHORT_TIME_V3_UPDATE_T prMpduShortTimeUpdate  = NULL;
	/* P_STA_RECORD_T prStaRec = cnmGetStaRecByIndex(0); */
	PMAC_TABLE_ENTRY pEntry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry;
	UINT16 i;
	UINT8 ucInUseSta = 0;
#ifdef FQ_SCH_SUPPORT
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	UINT8 j;
	UINT8 active = 0, bcmc_active = 0, pow_save = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry_tmp;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;
#endif
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	UINT32 *staInUseBitmap;
	UINT32 staInUseBitmap_tmp;
	HTTRANSMIT_SETTING tx_rate;
	UINT8 ucPhyMode = 0;
	UINT8 ucBW = 0;
	UINT32 tx_pkts = 0;
	UINT8 wordlen = (prMpduTimeEvt->Reserve[0] == MPDU_TIME_BITMAP_TAG) ?
			prMpduTimeEvt->Reserve[1] : ((wtbl_max_num+31)/(sizeof(UINT32)<<3));

	prMpduShortTimeUpdate = (P_MPDU_SHORT_TIME_V3_UPDATE_T)(Data +
				sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T) +
				(wordlen<<2));
	staInUseBitmap = (UINT32 *)(Data + sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T));

#ifdef FQ_SCH_SUPPORT
	for (j = 0; j < wordlen; j++)
		pAd->fq_ctrl.staInUseBitmap[j] = staInUseBitmap[j];

	RTMP_IO_READ32(pAd->hdev_ctrl, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[3]);
#endif
	for (i = 0; i < RED_STA_REC_NUM; i++) {
		staInUseBitmap_tmp = staInUseBitmap[i>>RED_INUSE_BITSHIFT];
#ifdef RT_BIG_ENDIAN
		staInUseBitmap_tmp = le2cpu32(staInUseBitmap_tmp);
#endif
		if ((staInUseBitmap_tmp & (1<<(i&RED_INUSE_BITMASK))) == 0) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			continue;
		}
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;
			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}
#ifdef RT_BIG_ENDIAN
		prMpduShortTimeUpdate->arN9TxARCnt = le2cpu16(prMpduShortTimeUpdate->arN9TxARCnt);
		prMpduShortTimeUpdate->arN9TxFRCnt = le2cpu16(prMpduShortTimeUpdate->arN9TxFRCnt);
		prMpduShortTimeUpdate->arN9TxFailCnt = le2cpu16(prMpduShortTimeUpdate->arN9TxFailCnt);
		prMpduShortTimeUpdate->arMpduTime = le2cpu32(prMpduShortTimeUpdate->arMpduTime);
		prMpduShortTimeUpdate->arMpduTime_avg = le2cpu32(prMpduShortTimeUpdate->arMpduTime_avg);
#endif

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
#ifdef FQ_SCH_SUPPORT
		prRedSta = &pAd->red_sta[i];
#endif
		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			prRedSta = &pAd->red_sta[i];
			if (pAd->fq_ctrl.enable & FQ_READY) {
				prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
				prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					(prMpduShortTimeUpdate->arMpduTime) :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);
				prRedSta->tx_msdu_cnt = 0;
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS) {
					active++;
					bcmc_active++;
				}
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];

		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
			tx_pkts = prMpduShortTimeUpdate->arN9TxARCnt + prMpduShortTimeUpdate->arN9TxFRCnt;
			pEntry->one_sec_tx_mpdu_pkts += tx_pkts;
			pEntry->one_sec_tx_mpdu_succ_pkts += tx_pkts - prMpduShortTimeUpdate->arN9TxFailCnt;

			ucPhyMode = prMpduShortTimeUpdate->arPhymodeBW >> 4;
			tx_rate.field.MODE = ucPhyMode;
			ucBW = prMpduShortTimeUpdate->arPhymodeBW & 0x0F;
			tx_rate.field.BW = ucBW;
			tx_rate.field.MCS = prMpduShortTimeUpdate->ucMcsShortGI >> 1;
			tx_rate.field.ShortGI = prMpduShortTimeUpdate->ucMcsShortGI & 0x01;
			pEntry->LastTxRate = (UINT32)tx_rate.word;

			/*Calcualte in used station number */
			if ((i > 0) && IS_WCID_VALID(pAd, i) &&
					(prMpduShortTimeUpdate->arMpduTime > 0))
				ucInUseSta++;

#ifdef FQ_SCH_SUPPORT
			if (pAd->fq_ctrl.enable & FQ_READY) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS)
					active++;
			}
#endif
		} else {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
		}
#endif
		}
#ifdef FQ_SCH_SUPPORT
		if (tr_entry->PsMode == PWR_SAVE)
			pow_save++;
#endif
		prMpduShortTimeUpdate++;
	}

	/*update in used station number */
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_READY) {
		pAd->fq_ctrl.nactive = active;
		pAd->fq_ctrl.nbcmc_active = bcmc_active;
		pAd->fq_ctrl.npow_save = pow_save;
		fq_clean_list(pAd, WMM_NUM_OF_AC);
	}
#endif
}

VOID ExtEventRedTxReportHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 i;
	UINT32 sta_num = 0;
	UINT8 BandIdx = 0;
	P_EXT_EVENT_RED_TX_RPT_T prTxRptEvt;
	P_RED_TX_RPT_T prTxRpt  = NULL;
	UINT32 *staInUseBitmap;
	PMAC_TABLE_ENTRY pEntry = NULL;
#ifdef VOW_SUPPORT
	UINT32 multi_tcp_nums[DBDC_BAND_NUM] = {0};
	UINT32 mcli_sch_th = pAd->vow_cfg.mcli_sch_cfg.tcp_cnt_th;
#endif
	prTxRptEvt = (P_EXT_EVENT_RED_TX_RPT_T) Data;
	staInUseBitmap = &prTxRptEvt->staInUseBitmap[0];
	prTxRpt = (P_RED_TX_RPT_T)(Data + sizeof(EXT_EVENT_RED_TX_RPT_T));

	sta_num = prTxRptEvt->wordlen << 5;

	for (i = 1; i < sta_num; i++) {
		UINT32 th;

		if ((staInUseBitmap[i >> RED_INUSE_BITSHIFT] & (1 << (i & RED_INUSE_BITMASK))) == 0)
			continue;

		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			BandIdx = HcGetBandByWdev(pEntry->wdev);

			th = VERIWAVE_5G_PKT_CNT_TH;
			if (WMODE_CAP_2G(pEntry->wdev->PhyMode))
				th = VERIWAVE_2G_PKT_CNT_TH;

			if ((prTxRpt->u4TCPCnt > th) || (prTxRpt->u4TCPAckCnt > th)) {
				pEntry = &pAd->MacTab.Content[i];
				BandIdx = HcGetBandByWdev(pEntry->wdev);
				pAd->txop_ctl[BandIdx].multi_tcp_nums++;
#ifdef VOW_SUPPORT
				if (pAd->vow_cfg.mcli_schedule_en && prTxRpt->u4TCPCnt > mcli_sch_th) {
					multi_tcp_nums[BandIdx]++;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(), Band%d, u4TCPCnt = %d, multi_tcp_nums = %d !!\n",
						__func__, BandIdx, prTxRpt->u4TCPCnt, multi_tcp_nums[BandIdx]));
				}
#endif
			}
		}
		prTxRpt++;
	}

#ifdef VOW_SUPPORT
	if (multi_tcp_nums[BandIdx] > pAd->multi_cli_nums_eap_th)
		pAd->vow_cfg.mcli_sch_cfg.schedule_cond_en[BandIdx] = TRUE;
#endif

	MtCmdSetRedTxReport(pAd, 0, Data, Length);
}
#endif

#ifdef RED_SUPPORT
VOID ExtEventMpduTimeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_UPDATE_T prMpduTimeEvt = (P_EXT_EVENT_MPDU_TIME_UPDATE_T)Data;
	P_MPDU_SHORT_TIME_UPDATE_T prMpduShortTimeUpdate  = NULL;
	/* P_STA_RECORD_T prStaRec = cnmGetStaRecByIndex(0); */
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	UINT16 i;
	UINT8 ucInUseSta = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucBW = 0;
#ifdef VOW_SUPPORT
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#else
	UINT8 fgATCorWATFEnable = 0;
#endif
#ifdef FQ_SCH_SUPPORT
	UINT8 j;
	UINT8 active = 0, bcmc_active = 0, pow_save = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry_tmp;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;
#endif
	UINT32 *staInUseBitmap;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	if (prMpduTimeEvt->ucfgValid == MPDU_TIME_FORMAT_VER)
		return ExtEventMpduTimeHandler_avg(pAd, Data, Length);

	if (prMpduTimeEvt->ucfgValid == MPDU_TIME_FORMAT_V3)
		return ExtEventMpduTimeHandler_v3(pAd, Data, Length);

	staInUseBitmap = (UINT32 *)(Data + sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T));
#ifdef FQ_SCH_SUPPORT
	for (j = 0; j < FQ_BITMAP_DWORD; j++)
		pAd->fq_ctrl.staInUseBitmap[j] = staInUseBitmap[j];

	if (IS_MT7615(pAd))
		return ExtEventMpduTimeHandler_fp(pAd, Data, Length);
#endif
#ifdef FQ_SCH_SUPPORT
	RTMP_IO_READ32(pAd->hdev_ctrl, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value[3]);
#endif
	prMpduShortTimeUpdate = (P_MPDU_SHORT_TIME_UPDATE_T)(Data +
				sizeof(EXT_EVENT_MPDU_TIME_UPDATE_T) +
				 (((wtbl_max_num + 31)/(sizeof(UINT32)<<3))<<2));

	for (i = 0; i < RED_STA_REC_NUM; i++) {
		if ((staInUseBitmap[i>>RED_INUSE_BITSHIFT]&
						(1<<(i&RED_INUSE_BITMASK))) == 0) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			continue;
		}
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;
			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			prMpduShortTimeUpdate++;
				continue;
		}

		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			prRedSta = &pAd->red_sta[i];
			if (pAd->fq_ctrl.enable & FQ_READY) {
				prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
				prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					(prMpduShortTimeUpdate->arMpduTime) :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);
				prRedSta->tx_msdu_cnt = 0;
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS) {
					active++;
					bcmc_active++;
				}
			}
#endif
			prMpduShortTimeUpdate++;
				continue;
		}

		tr_entry = &tr_ctl->tr_entry[pEntry->tr_tb_idx];
		prRedSta = &pAd->red_sta[i];
		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
			prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);

			prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
						prMpduShortTimeUpdate->arMpduTime :
						((prRedSta->tx_msdu_avg_cnt > 0) ?
						(prMpduShortTimeUpdate->arMpduTime/
						prRedSta->tx_msdu_avg_cnt) :
						prRedSta->i4MpduTime);

			prRedSta->tx_msdu_cnt = 0;

			ucPhyMode = prMpduShortTimeUpdate->arPhymodeBW >> 4;
			ucBW = prMpduShortTimeUpdate->arPhymodeBW & 0x0F;
			/*If TxForceCnt/TxCnt > 25% and not badnode, then reset to default.*/
			RedCalForceRateRatio(i,
					prMpduShortTimeUpdate->arN9TxARCnt,
					prMpduShortTimeUpdate->arN9TxFRCnt,
					 pAd);

			/*Calcualte in used station number */
			if ((i > 0) && (i < wtbl_max_num) &&
				(prMpduShortTimeUpdate->arMpduTime > 0))
					ucInUseSta++;

			if (prRedSta->i4MpduTime < 0)
				RedResetSta(i, ucPhyMode, ucBW, pAd);
			else
				UpdateThreshold(i, pAd);

			UpdateAirtimeRatio(i, prMpduShortTimeUpdate->arAirtimeRatio
						, fgATCorWATFEnable, pAd);
#ifdef FQ_SCH_SUPPORT
			if (pAd->fq_ctrl.enable & FQ_READY) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS)
					active++;
			}
#endif
		} else {
			/*fgIsInUse == FALSE, don't care PHY mode. */
			RedResetSta(i, MODE_CCK, BW_20, pAd);
			prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;
			prRedSta->tx_msdu_cnt = 0;
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
		}
#endif
		}
#ifdef FQ_SCH_SUPPORT
		if (tr_entry->PsMode == PWR_SAVE)
			pow_save++;
#endif
	prMpduShortTimeUpdate++;
	}

	/*update in used station number */
	pAd->red_in_use_sta = ucInUseSta;
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_READY) {
		pAd->fq_ctrl.nactive = active;
		pAd->fq_ctrl.nbcmc_active = bcmc_active;
		pAd->fq_ctrl.npow_save = pow_save;
		fq_clean_list(pAd, WMM_NUM_OF_AC);
	}
#endif
}
#endif /* defined(RED_SUPPORT) */
#ifdef CFG_SUPPORT_CSI
ULONG NBytesAlign(ULONG len, ULONG nBytesAlign)
{
	if (nBytesAlign > 0xFFFF)
		return 0xFFFFFFFF;

	if (len%nBytesAlign == 0)
		return len;

	len += nBytesAlign - len%nBytesAlign;

	return len;
}

INT AndesCSICtrl(RTMP_ADAPTER *pAd, struct CMD_CSI_CONTROL_T *prCSICtrl)
{
	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};

	/*allocate mem*/
	msg = AndesAllocCmdMsg(pAd, sizeof(struct CMD_CSI_CONTROL_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/*set attr*/
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CSI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	/*Debug Log*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\nCSI CMD DEBUG:%d,%d,%d,%d,%d\n",
	prCSICtrl->BandIdx, prCSICtrl->ucMode, prCSICtrl->ucCfgItem, prCSICtrl->ucValue1, prCSICtrl->ucValue2));

	AndesAppendCmdMsg(msg, (char *)prCSICtrl, sizeof(struct CMD_CSI_CONTROL_T));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

VOID ExtEventCSICtrl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT_32 u4IsCck = 0;
	UINT_32 u4cbw = 0;
	UINT_32 u4Rssi = 0;
	UINT_32 u4SNR = 0;
	UINT_32 u4BandIdx = 0;
	UINT_32 u4DataCount = 0;
	UINT_32 u4DataBw = 0;
	UINT_32 u4PrimaryChIdx = 0;
	UINT_32 u4RxMode = 0;
	UINT_32 u4Rsvd4 = 0;
	UINT_16 *pru2Tmp = NULL;
	UINT_32 *p32tmp = NULL;
	INT_16 i2Idx = 0;
	struct CSI_DATA_T *prCSIData = NULL;
	TLV_ELEMENT_T *prCSITlvData = NULL;
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	ULONG time_jiff = 0;
	UINT8 *prBuf = Data;
	/* u2Offset is 8 bytes currently, tag 4 bytes + length 4 bytes */
	UINT_16 u2Offset = Offsetof(TLV_ELEMENT_T, aucbody);

	/*debug for csi*/
	/*hex_dump_with_lvl("CSIEventDump:\n ", (UCHAR *)prBuf, (UINT)Length, DBG_LVL_ERROR);*/

	os_alloc_mem(NULL, (UCHAR **)&prCSIData, sizeof(struct CSI_DATA_T));

	if (!prCSIData) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s:allocate memory for prCSIData failed!\n", __func__));
		return;
	}

	os_zero_mem(prCSIData, sizeof(struct CSI_DATA_T));

	/*TimesStamp: msec*/
	NdisGetSystemUpTime(&time_jiff);
	prCSIData->u8TimeStamp = jiffies_to_msecs(time_jiff);

	while (Length >= u2Offset) {
		prCSITlvData = (struct TLV_ELEMENT *)prBuf;
		prCSITlvData->tag_type = le2cpu32(prCSITlvData->tag_type);
		prCSITlvData->body_len = le2cpu32(prCSITlvData->body_len);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("tag_type=%d,body_len=%d,length=%d\n",
		prCSITlvData->tag_type, prCSITlvData->body_len, Length));

		switch (prCSITlvData->tag_type) {
		case CSI_EVENT_IS_CCK:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid IS_CCK len %u", prCSITlvData->body_len));
				goto out;
			}
			u4IsCck = le2cpu32(*((UINT32 *)prCSITlvData->aucbody));
			prCSIData->bIsCck = (u4IsCck) ? TRUE : FALSE;
			break;
		case CSI_EVENT_CBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CBW len %u", prCSITlvData->body_len));
				goto out;
			}
			u4cbw = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucBw = (UINT8)u4cbw;
			break;
		case CSI_EVENT_RSSI:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid RSSI len %u", prCSITlvData->body_len));
				goto out;
			}
			u4Rssi = le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			prCSIData->cRssi = (UINT8)u4Rssi;
			break;
		case CSI_EVENT_SNR:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid SNR len %u", prCSITlvData->body_len));
				goto out;
			}
			u4SNR = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucSNR = (UINT8)u4SNR;
			break;
		case CSI_EVENT_BAND:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid BAND len %u", prCSITlvData->body_len));
				goto out;
			}
			u4BandIdx = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucDbdcIdx = (UINT8)u4BandIdx;
			break;
		case CSI_EVENT_CSI_NUM:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CSI num len %u", prCSITlvData->body_len));
				goto out;
			}
			u4DataCount = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->u2DataCount = (UINT16)u4DataCount;
			break;
		case CSI_EVENT_CSI_I_DATA:
			if (prCSIData->u2DataCount > CSI_MAX_DATA_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CSI count %u\n", prCSIData->u2DataCount));
				goto out;
			}
			if (prCSITlvData->body_len != sizeof(INT_16) * CSI_MAX_DATA_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CSI num len %u", prCSITlvData->body_len));
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < prCSIData->u2DataCount; i2Idx++)
				prCSIData->ac2IData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_CSI_Q_DATA:
			if (prCSIData->u2DataCount > CSI_MAX_DATA_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CSI count %u\n", prCSIData->u2DataCount));
				goto out;
			}
			if (prCSITlvData->body_len != sizeof(INT_16) * CSI_MAX_DATA_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CSI num len %u", prCSITlvData->body_len));
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < prCSIData->u2DataCount; i2Idx++)
				prCSIData->ac2QData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_DBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid DBW len %u", prCSITlvData->body_len));
				goto out;
			}
			u4DataBw = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucDataBw = (UINT8)u4DataBw;
			break;
		case CSI_EVENT_CH_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid CH IDX len %u", prCSITlvData->body_len));
				goto out;
			}
			u4PrimaryChIdx = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucPrimaryChIdx = (UINT8)u4PrimaryChIdx;
			break;
		case CSI_EVENT_TA:
			/*
			 * TA length is 8-byte long (MAC addr 6 bytes +
			 * 2 bytes padding), the 2-byte padding keeps
			 * the next Tag at a 4-byte aligned address.
			 */
			if (prCSITlvData->body_len != NBytesAlign(sizeof(prCSIData->aucTA), 4)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid TA len %u", prCSITlvData->body_len));
				goto out;
			}
			os_move_mem(prCSIData->aucTA, prCSITlvData->aucbody, sizeof(prCSIData->aucTA));
			break;
		case CSI_EVENT_EXTRA_INFO:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid Error len %u", prCSITlvData->body_len));
				goto out;
			}
			prCSIData->u4ExtraInfo = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RX_MODE:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid Rx Mode len %u", prCSITlvData->body_len));
				goto out;
			}
			u4RxMode = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucRxMode = (UINT8)u4RxMode;
			break;
		case CSI_EVENT_RSVD1:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD1_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid RSVD1 len %u", prCSITlvData->body_len));
				goto out;
			}
			prCSIData->ucRsvd1Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < prCSIData->ucRsvd1Cnt; i2Idx++)
				prCSIData->ai4Rsvd1[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD2:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD2_COUNT) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid RSVD2 len %u", prCSITlvData->body_len));
				goto out;
			}
			prCSIData->ucRsvd2Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < prCSIData->ucRsvd2Cnt; i2Idx++)
				prCSIData->au4Rsvd2[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD3:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid RSVD3 len %u", prCSITlvData->body_len));
				goto out;
			}
			prCSIData->i4Rsvd3 = le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RSVD4:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid RSVD4 len %u", prCSITlvData->body_len));
				goto out;
			}
			u4Rsvd4 = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			prCSIData->ucRsvd4 = (UINT8)u4Rsvd4;
			break;
		case CSI_EVENT_H_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Invalid Antenna_pattern len %u", prCSITlvData->body_len));
			goto out;
			}
			prCSIData->Antenna_pattern = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));

			/*to drop pkt that usr do not need*/
			if ((prCSIInfo->Matrix_Get_Bit) &&
				!(prCSIInfo->Matrix_Get_Bit & (1 << (prCSIData->Antenna_pattern & 0xf))))
				goto out;
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unsupported CSI tag %d\n",
			prCSITlvData->tag_type));
		};

			Length -= (u2Offset + prCSITlvData->body_len);

			if (Length >= u2Offset)
				prBuf += (u2Offset + prCSITlvData->body_len);
		}

		/*mask the null tone && pilot tone*/
		if ((prCSIInfo->ucValue1[CSI_CONFIG_OUTPUT_FORMAT] ==
			CSI_OUTPUT_TONE_MASKED ||
			prCSIInfo->ucValue1[CSI_CONFIG_OUTPUT_FORMAT] ==
			CSI_OUTPUT_TONE_MASKED_SHIFTED) &&
			!prCSIData->bIsCck) {
			wlanApplyCSIToneMask(prCSIData->ucRxMode,
				prCSIData->ucBw, prCSIData->ucDataBw,
				prCSIData->ucPrimaryChIdx,
				prCSIData->ac2IData, prCSIData->ac2QData);
		}
		/*reoder the tone */
		if (prCSIInfo->ucValue1[CSI_CONFIG_OUTPUT_FORMAT] ==
			CSI_OUTPUT_TONE_MASKED_SHIFTED &&
			!prCSIData->bIsCck) {
			os_move_mem(prCSIInfo->ai2TempIData,
				prCSIData->ac2IData,
				sizeof(INT_16) * prCSIData->u2DataCount);
			os_move_mem(prCSIInfo->ai2TempQData,
				prCSIData->ac2QData,
				sizeof(INT_16) * prCSIData->u2DataCount);

			wlanShiftCSI(prCSIData->ucRxMode,
				prCSIData->ucBw, prCSIData->ucDataBw,
				prCSIData->ucPrimaryChIdx,
				prCSIInfo->ai2TempIData,
				prCSIInfo->ai2TempQData,
				prCSIData->ac2IData,
				prCSIData->ac2QData);

			if (prCSIData->ucDataBw == RX_VT_FR_MODE_20)
				prCSIData->u2DataCount = 64;
			else if (prCSIData->ucDataBw == RX_VT_FR_MODE_40)
				prCSIData->u2DataCount = 128;
			else
				prCSIData->u2DataCount = 256;

			}

	wlanPushCSIData(pAd, prCSIData);
	wake_up_interruptible(&(pAd->rCSIInfo.waitq));

out:
	if (prCSIData)
		os_free_mem(prCSIData);
}
#endif
