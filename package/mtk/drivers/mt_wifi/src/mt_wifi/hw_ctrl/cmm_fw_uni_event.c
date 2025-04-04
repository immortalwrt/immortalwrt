/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_fw_uni_event.c
*/

#ifdef WIFI_UNIFIED_COMMAND
#include "rt_config.h"

VOID UniCmdResultRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	P_UNI_EVENT_CMD_RESULT_T UniCmdResult = (P_UNI_EVENT_CMD_RESULT_T)payload;

	UniCmdResult->u2CID = le2cpu16(UniCmdResult->u2CID);
	UniCmdResult->u4Status = le2cpu32(UniCmdResult->u4Status);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UniCmdResult.ucCID = 0x%x\n",
			UniCmdResult->u2CID);

	if (UniCmdResult->u4Status != 0 || (UniCmdResult->u2CID != msg->attr.type)) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN, "BUG::UniCmdResult.u4Status = 0x%x cid: = 0x%x\n",
				  UniCmdResult->u4Status, UniCmdResult->u2CID);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UniCmdResult.u4Status = 0x%x\n",
				  UniCmdResult->u4Status);
	}
}

static VOID UniEventFwLogFormatProc(struct _RTMP_ADAPTER *pAd, P_UNI_EVENT_FW_LOG_FORMAT_T TlvData, EVENT_RXD *event_rxd)
{
	UCHAR *dev_name = NULL;
	UCHAR empty_name[] = " ";
	UINT16 u2MsgSize = (le2cpu16(TlvData->u2Length) - sizeof(UNI_EVENT_FW_LOG_FORMAT_T));
	UINT16 *serialID = &(pAd->fw_log_ctrl.fw_log_serialID_count);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: s2d_index = 0x%x\n", __func__,
				event_rxd->fw_rxd_2.field.s2d_index);

	dev_name = RtmpOsGetNetDevName(pAd->net_dev);
		if ((dev_name == NULL) || strlen(dev_name) >= NET_DEV_NAME_MAX_LENGTH)
			dev_name = &empty_name[0];

		if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST) {
#ifdef FW_LOG_DUMP
				P_FW_BIN_LOG_HDR_T log_hdr = (P_FW_BIN_LOG_HDR_T)TlvData->acMsg;

				if (le2cpu32(log_hdr->u4MagicNum) == FW_BIN_LOG_MAGIC_NUM) {
					log_hdr->u2SerialID = (*serialID)++;
					if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
						RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)TlvData->acMsg, u2MsgSize);

					if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
						fw_log_to_ethernet(pAd, (UINT8 *)TlvData->acMsg, u2MsgSize);
				} else
#endif /* FW_LOG_DUMP */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
				if (pAd->KtoFlashDebug)
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%s): %s", dev_name, TlvData->acMsg));
				else
#endif /*PRE_CAL_TRX_SET1_SUPPORT*/
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("N9 LOG(%s): %s\n", dev_name, TlvData->acMsg));
		} else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CR4 LOG(%s): %s\n", dev_name, TlvData->acMsg));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unknow MCU LOG(%s): %s", dev_name, TlvData->acMsg));
		}
}

VOID UniEventFwLog2HostHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_FW_LOG2HOST_T);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_FW_LOG_FORMAT:
			UniEventFwLogFormatProc(pAd, (void *)tag, event_rxd);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventAccessRegBasicProc(
	P_UNI_EVENT_ACCESS_REG_BASIC_T TlvData,
	RTMP_REG_PAIR *RegPair)
{
	if (TlvData && RegPair) {
		RegPair->Register = le2cpu32(TlvData->u4Addr);
		RegPair->Value = le2cpu32(TlvData->u4Value);

		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAC CR: 0x%08x=0x%08x\n", RegPair->Register, RegPair->Value));
	}
}

static VOID UniEventAccessRfRegBasicProc(
	P_UNI_EVENT_ACCESS_RF_REG_BASIC_T TlvData,
	MT_RF_REG_PAIR *RfRegPair)
{
	if (TlvData && RfRegPair) {
		RfRegPair->WiFiStream =  (UINT8)le2cpu16(TlvData->u2WifiStream);
		RfRegPair->Register = le2cpu32(TlvData->u4Addr);
		RfRegPair->Value = le2cpu32(TlvData->u4Value);

		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RF CR: Rf-%d 0x%08x=0x%08x\n", RfRegPair->WiFiStream,
			RfRegPair->Register, RfRegPair->Value));
	}
}

VOID UniEventAccessRegHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ACCESS_REG_T);
	struct _RTMP_REG_PAIR *RegPair = (struct _RTMP_REG_PAIR *)msg->attr.rsp.wb_buf_in_calbk;
	struct _MT_RF_REG_PAIR *RfRegPair = (struct _MT_RF_REG_PAIR *)msg->attr.rsp.wb_buf_in_calbk;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ACCESS_REG_BASIC:
			UniEventAccessRegBasicProc((void *)tag, RegPair);
			RegPair++;
			break;

		case UNI_EVENT_ACCESS_RF_REG_BASIC:
			UniEventAccessRfRegBasicProc((void *)tag, RfRegPair);
			RfRegPair++;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef AIR_MONITOR
static VOID UniEventSmeshInfoParamProc(
	P_UNI_CMD_SMESH_PARAM_T TlvData,
	struct cmd_msg *msg)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_CMD_SMESH_PARAM_T pSmeshResult = (P_UNI_CMD_SMESH_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		os_move_mem(pSmeshResult, TlvData, sizeof(*pSmeshResult));
	}

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ucEntryEnable = %d, fgSmeshRxA2 = %d, fgSmeshRxA1 = %d, fgSmeshRxData = %d, fgSmeshRxMgnt = %d, fgSmeshRxCtrl = %d\n",
			TlvData->ucEntryEnable, TlvData->fgSmeshRxA2, TlvData->fgSmeshRxA1,
			TlvData->fgSmeshRxData, TlvData->fgSmeshRxMgnt, TlvData->fgSmeshRxCtrl);
}

VOID UniEventSmeshInfoRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SMESH_INFO_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_CMD_SMESH_PARAM:
			UniEventSmeshInfoParamProc((void *)tag, msg);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
#endif /* AIR_MONITOR */

static VOID UniEventIECountDownCSAProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	P_UNI_EVENT_CSA_NOTIFY_T TlvData)
{
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	UCHAR Index;
	struct wifi_dev *wdevEach = NULL;

	wdev = wdev_search_by_band_omac_idx(pAd, ucBand, TlvData->ucOwnMacIdx);
	if (!wdev)
		return;

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	wdev->csa_count = TlvData->ucChannelSwitchCount;
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
static VOID UniEventIECountDownBCCProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	P_UNI_EVENT_BCC_NOTIFY_T TlvData)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, ucBand, TlvData->ucOwnMacIdx);
	if (!wdev)
		return;

	bss_color_event_handler(wdev);
}
#endif /* DOT11_HE_AX */

VOID UniEventIECountDownHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_IE_COUNTDOWN_T);
	P_UNI_EVENT_IE_COUNTDOWN_T pEventHdr = (P_UNI_EVENT_IE_COUNTDOWN_T)pData;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IE_COUNTDOWN_CSA:
			UniEventIECountDownCSAProc(pAd, pEventHdr->ucBand, (void *)tag);
			break;

#ifdef DOT11_HE_AX
		case UNI_EVENT_IE_COUNTDOWN_BCC:
			UniEventIECountDownBCCProc(pAd, pEventHdr->ucBand, (void *)tag);
			break;
#endif /* DOT11_HE_AX */

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventAssertContentProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBssIndex,
	P_UNI_EVENT_ASSERT_CONTENT_T TlvData,
	EVENT_RXD *event_rxd)
{
	UINT16 u2MsgSize = (le2cpu16(TlvData->u2Length) - sizeof(UNI_EVENT_ASSERT_CONTENT_T));

	if (u2MsgSize > 0) {
		TlvData->aucBuffer[u2MsgSize] = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("ucBssIndex(%d): %s\n", ucBssIndex, TlvData->aucBuffer));
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
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "cannot alloc mem for FW dump\n");
		}
	}

	if (pAd->fw_dump_buffer) {
		if ((pAd->fw_dump_size + u2MsgSize) <= pAd->fw_dump_max_size) {
			os_move_mem(pAd->fw_dump_buffer + pAd->fw_dump_size, TlvData->aucBuffer, u2MsgSize);
			pAd->fw_dump_size += u2MsgSize;
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "FW dump size too big\n");
		}
	}
#endif /* FW_DUMP_SUPPORT */
}

VOID UniEventAssertDumpHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ASSERT_DUMP_T);
	P_UNI_EVENT_ASSERT_DUMP_T pEventHdr = (P_UNI_EVENT_ASSERT_DUMP_T)pData;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ASSERT_CONTENT_DUMP:
			UniEventAssertContentProc(pAd, pEventHdr->ucBssIndex, (void *)tag, event_rxd);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventBeaconTimeoutInfoProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIndex,
	P_UNI_EVENT_BEACON_TIMEOUT_INFO_T TlvData)
{
#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif /* CONFIG_AP_SUPPORT */
	struct wifi_dev *wdev;

	wdev = wdev_search_by_omac_idx(pAd, BssIndex);
	if (!wdev)
		return;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::FW EVENT (%02x:%02x:%02x:%02x:%02x:%02x), Reason Code: 0x%x\n", __func__,
			  PRINT_MAC(wdev->bssid), TlvData->ucReasonCode));

	switch (TlvData->ucReasonCode) {
	case UNI_ENUM_BCN_LOSS_STA:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA: Beacon loss detected!\n"));
		break;

	case UNI_ENUM_BCN_LOSS_ADHOC:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA ADHOC: Beacon loss detected!\n"));
		break;

	case UNI_ENUM_BCN_NULL_FRAME_THRESHOLD:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA TDD: Null frame life timeout due to threshold detected!\n"));
		break;

	case UNI_ENUM_BCN_PERIOD_NOT_ILLIGAL:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP: Beacon interval is illegal detected!\n"));
		break;

	case UNI_ENUM_BCN_CONNECTION_FAIL:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA: Connection timeout detected!\n"));
		break;

	case UNI_ENUM_BCN_ALLOCAT_NULL_PKT_FAIL_THRESHOLD:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA: Allocate null frame fail over threshold detected!\n"));
		break;

	case UNI_ENUM_BCN_UNSPECIF_REASON:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA: Beacon timeout due to unspecified reason detected!\n"));
		break;

	case UNI_ENUM_BCN_NULL_FRAME_LIFE_TIMEOUT:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA TDD: Null frame life timeout detected!\n"));
		break;

	case UNI_ENUM_BCN_LOSS_AP_DISABLE:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP: Beacon OFF!!!\n"));
		break;

	case UNI_ENUM_BCN_LOSS_AP_ERROR:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP: Beacon lost - Error!!! Re-issue BCN_OFFLOAD cmd!\n"));
#ifdef CONFIG_AP_SUPPORT
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			break;

		/* do BCN_UPDATE_ALL_AP_RENEW when all BSS CSA done */
		if (pDot11h->csa_ap_bitmap == 0)
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ALL_AP_RENEW);
		else
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					" CSA is runing, wait it done, RDMode=%d, csa_ap_bitmap=0x%x\n",
					pDot11h->RDMode, pDot11h->csa_ap_bitmap);
#endif /* CONFIG_AP_SUPPORT */
		break;
	}
}

VOID UniEventBeaconTimeoutHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_BEACON_TIMEOUT_T);
	P_UNI_EVENT_BEACON_TIMEOUT_T pEventHdr = (P_UNI_EVENT_BEACON_TIMEOUT_T)pData;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BEACON_TIMEOUT_INFO:
			UniEventBeaconTimeoutInfoProc(pAd, pEventHdr->ucBssIndex, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventClientPSInfoProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIndex,
	P_UNI_EVENT_CLIENT_PS_INFO_T TlvData)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct qm_ctl *qm_ctl = &pAd->qm_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;
	NDIS_PACKET *pkt = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: PsSync Event from FW APPS(BssIndex=%d) WtblIndex=%d PSBit=%d BufferSize=%d\n", __func__,
			 BssIndex, TlvData->ucWtblIndex, TlvData->ucPsBit, TlvData->ucBufferSize);

	if (IS_TR_WCID_VALID(pAd, TlvData->ucWtblIndex)) {
		tr_entry = &tr_ctl->tr_entry[TlvData->ucWtblIndex];
		RTMP_SEM_LOCK(&tr_entry->ps_sync_lock);
		tr_entry->ps_state = (TlvData->ucPsBit == 1) ? PWR_SAVE : PWR_ACTIVE;

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
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wtbl index(%d) is invalid!\n", TlvData->ucWtblIndex);
	}
}

VOID UniEventPSSyncHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_PS_SYNC_T);
	P_UNI_EVENT_PS_SYNC_T pEventHdr = (P_UNI_EVENT_PS_SYNC_T)pData;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CLIENT_PS_INFO:
			UniEventClientPSInfoProc(pAd, pEventHdr->ucBssIndex, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventECCCalGroupPointResultProc(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_EVENT_ECC_CAL_RES_T TlvData)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ucEccCmdId = %d, ucIsResFail= %d\n", TlvData->ucEccCmdId, TlvData->ucIsResFail));

	hex_dump_with_lvl("Dqx", TlvData->aucDqxBuffer, TlvData->ucDqxDataLength, DBG_LVL_OFF);
	hex_dump_with_lvl("Dqy", TlvData->aucDqyBuffer, TlvData->ucDqyDataLength, DBG_LVL_OFF);
}

VOID UniEventECCCalHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ECC_CAL_T);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ECC_CAL_GROUP_POINT_RESULT:
			UniEventECCCalGroupPointResultProc(pAd, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventSerQueryCmmProc(
	P_UNI_EVENT_SER_QUERY_CMM_T TlvData)
{
	UINT32 i;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ucEnableSER = %d, ucSerL1RecoverCnt = %d, ucSerL2RecoverCnt = %d\n", __func__,
			TlvData->ucEnableSER, TlvData->ucSerL1RecoverCnt, TlvData->ucSerL1RecoverCnt));

	for (i = 0; i < 32; i++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: i = %d => u2PSEErrorCnt = %d, u2PSEError1Cnt = %d, u2PLEErrorCnt = %d, u2PLEError1Cnt = %d, u2PLEErrorAmsduCnt = %d\n", __func__,
		i, TlvData->u2PSEErrorCnt[i], TlvData->u2PSEError1Cnt[i], TlvData->u2PLEErrorCnt[i],
		TlvData->u2PLEError1Cnt[i], TlvData->u2PLEErrorAmsduCnt[i]));
	}
}

static VOID UniEventSerQueryBandProc(
	P_UNI_EVENT_SER_QUERY_BAND_T TlvData)
{
	UINT32 i;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ucBandIdx = %d, ucL3RxAbortCnt = %d, ucL3TxAbortCnt = %d, ucL3TxDisableCnt = %d, ucL4RecoverCnt = %d\n", __func__,
			TlvData->ucBandIdx, TlvData->ucL3RxAbortCnt, TlvData->ucL3TxAbortCnt, TlvData->ucL3TxDisableCnt, TlvData->ucL4RecoverCnt));

	for (i = 0; i < 32; i++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i = %d => au2LMACError6Cnt = %d, au2LMACError7Cnt = %d\n", __func__,
		i, le2cpu16(TlvData->au2LMACError6Cnt[i]), le2cpu16(TlvData->au2LMACError7Cnt[i])));
	}
}

static VOID UniEventSerQueryWFDMAProc(
	P_UNI_EVENT_SER_QUERY_WFDMA_T TlvData)
{
	UINT32 i;

	for (i = 0; i < 4; i++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i = %d => au2WfdmaTxBusyCnt = %d, au2WfdmaRxBusyCnt = %d\n", __func__,
		i, le2cpu16(TlvData->au2WfdmaTxBusyCnt[i]), le2cpu16(TlvData->au2WfdmaRxBusyCnt[i])));
	}
}

VOID UniEventSERHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SER_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SER_QUERY_CMM:
			UniEventSerQueryCmmProc((void *)tag);
			break;

		case UNI_EVENT_SER_QUERY_BAND:
			UniEventSerQueryBandProc((void *)tag);
			break;

		case UNI_EVENT_SER_QUERY_WFDMA:
			UniEventSerQueryWFDMAProc((void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventMacInfoTSFProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_MAC_INFO_TSF_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		TSF_RESULT_T *pTsfResult = (TSF_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;
		pTsfResult->u4TsfBit0_31 = le2cpu32(TlvData->u4TsfBit0_31);
		pTsfResult->u4TsfBit63_32 = le2cpu32(TlvData->u4TsfBit63_32);
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"ucDbdcIdx = %d, ucHwBssidIndex = %d, u4TsfBit0_31 = %d, u4TsfBit63_32 = %d\n",
			TlvData->ucDbdcIdx, TlvData->ucHwBssidIndex, le2cpu32(TlvData->u4TsfBit0_31), le2cpu32(TlvData->u4TsfBit63_32));
}

VOID UniEventMACInfoHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_MAC_IFNO_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MAC_INFO_TSF:
			UniEventMacInfoTSFProc(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
VOID UniEventTxCmdDbgHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_TXCMD_CTRL_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		UniEventTxCmdShow((void *)tag);
	}
}
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

static VOID UniEventEDCCAEnableProc(
	P_UNI_EVENT_EDCCA_ENABLE_T TlvData)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FW Status => fgEDCCAEnable = %d\n", __func__, TlvData->fgEDCCAEnable));
}

static VOID UniEventEDCCAThresholdProc(
	P_UNI_EVENT_EDCCA_THRESHOLD_T TlvData)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: FW Status =>fginit = %d,  u1EDCCAThreshold[0] = %d, u1EDCCAThreshold[1] = %d, u1EDCCAThreshold[2] = %d\n",
			__func__, TlvData->fginit, TlvData->u1EDCCAThreshold[0], TlvData->u1EDCCAThreshold[1], TlvData->u1EDCCAThreshold[2]));
}

VOID UniEventEDCCAHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_EDCCA_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_EDCCA_ENABLE:
			UniEventEDCCAEnableProc((void *)tag);
			break;

		case UNI_EVENT_EDCCA_THRESHOLD:
			UniEventEDCCAThresholdProc((void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

VOID UniEventMibHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_MIB_T);
	RTMP_MIB_PAIR *pMibResult = (RTMP_MIB_PAIR *)msg->attr.rsp.wb_buf_in_calbk;
	P_UNI_EVENT_MIB_DATA_T pUniEventMibData = NULL;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MIB_DATA:
			pUniEventMibData = (P_UNI_EVENT_MIB_DATA_T)tag;
			pMibResult->Counter = le2cpu32(pUniEventMibData->u4Counter);
			pMibResult->Value = le2cpu64(pUniEventMibData->u8Data);
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Tag=0x%x, Counter=%d, Value=%lld\n",
					pUniEventMibData->u2Tag, pMibResult->Counter, pMibResult->Value);
			pMibResult++;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef SCS_FW_OFFLOAD
#ifdef SMART_CARRIER_SENSE_SUPPORT
static VOID UniEventSCSGetGloAddrProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_GET_SCS_GLO_ADDR TlvData)
{
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s: UNI_EVENT_SCS_GET_GLO_ADDR\n", __func__);

	UniEventSCSGetGloAddrHandler(msg, (char *)TlvData);
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

VOID UniEventSCSHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SCS_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
#ifdef SMART_CARRIER_SENSE_SUPPORT
		case UNI_EVENT_SCS_GET_GLO_ADDR:
			UniEventSCSGetGloAddrProc(msg, (void *)tag);
			break;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
#endif /* SCS_FW_OFFLOAD */

static VOID UniEventTPCDlTableProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T TlvData)
{
	UINT8 i;
	P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T  prTpcDownlinkInfoTbl = TlvData;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TPC DOWNLINK INFO TABLE\n\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		DL Tx Type			Cmd Pwr Ctrl		DL Tc Pwr\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		MU MIMO				%3d					%3d\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_MIMO]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		OFDMA				%3d					%3d\n\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_OFDMA],
														prTpcDownlinkInfoTbl->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_OFDMA]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		WLAN		TxPwrAlpha MU_MIMO		TxPwrAlpha OFDMA\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%3d				%3d					%3d\n",
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].u2WlanId,
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_OFDMA]));
}

static VOID UniEventTPCUlTableProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T TlvData)
{
	UINT8 i;
	P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T  prTpcUplinkInfoTbl = TlvData;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TPC UPLINK INFO TABLE\n\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP INFO: AP TX Power = %d\n", prTpcUplinkInfoTbl->u1ApTxPwr));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STA INFO\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		WLAN		TargetRssi		PwrHeadRoom		MinPwrFlag\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================================================================\n"));
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%3d			%3d			%3d			%3d\n",
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].u2WlanId,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1TargetRssi,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1PwrHeadRoom,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.fgMinPwr));
}

VOID UniEventTPCHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_TPC_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_TPC_DOWNLINK_TABLE:
			UniEventTPCDlTableProc(msg, (void *)tag);
			break;

		case UNI_EVENT_TPC_UPLINK_TABLE:
			UniEventTPCUlTableProc(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}	
}

static PROCESS_RX_UNSOLICIT_EVENT_FUNCTION arEventTable[UNI_EVENT_ID_MAX_NUM] = {
	[0 ... UNI_EVENT_ID_MAX_NUM - 1] = NULL,
	[UNI_EVENT_ID_FW_LOG_2_HOST] = UniEventFwLog2HostHandler, /* 0x04 */
	[UNI_EVENT_ID_IE_COUNTDOWN] = UniEventIECountDownHandler, /* 0x09 */
	[UNI_EVENT_ID_ASSERT_DUMP] = UniEventAssertDumpHandler, /* 0x0A */
	[UNI_EVENT_ID_BEACON_TIMEOUT] = UniEventBeaconTimeoutHandler, /* 0x0C */
	[UNI_EVENT_ID_PS_SYNC] = UniEventPSSyncHandler, /* 0x0D */
	[UNI_EVENT_ID_ECC_CAL] = UniEventECCCalHandler, /* 0x10 */
};

VOID UniEventUnsolicitMainHandler(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET net_pkt)
{
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
	UINT8 eid = GET_EVENT_FW_RXD_EID(event_rxd);
	ULONG len = GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s: seq_num=%d, eid=0x%x, len=%lu\n", __func__,
				 GET_EVENT_FW_RXD_SEQ_NUM(event_rxd), eid, len);

	if (eid < UNI_EVENT_ID_MAX_NUM && arEventTable[eid] != NULL)
		arEventTable[eid](pAd, GET_EVENT_HDR_ADDR(net_pkt), len, event_rxd);
	else
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Unsolicit Event(0x%x)\n", __func__, eid));
}

/*****************************************
 * UNI_CMD_ID_VOW (Tag 0x37)
 *****************************************/
/* UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME (0x14) */
static VOID UniEventVowRxAtReportRxNonwifiTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_CMD_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pVowRxAtReportRxNonwifiTime = &(pVOWParam->EventVowRxAtReportRxNonwifiTime);

		pVowRxAtReportRxNonwifiTime->ucRxNonWiFiBandIdx = TlvData->ucRxNonWiFiBandIdx;
		pVowRxAtReportRxNonwifiTime->u4RxNonWiFiBandTimer = le2cpu32(TlvData->u4RxNonWiFiBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"ucRxNonWiFiBandIdx = %d, u4RxNonWiFiBandTimer = %d\n",
			TlvData->ucRxNonWiFiBandIdx, le2cpu32(TlvData->u4RxNonWiFiBandTimer));
}

/* UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME (0x15) */
static VOID UniEventVowRxAtReportRxObssTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_CMD_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pEventVowRxAtReportRxObssTime = &(pVOWParam->EventVowRxAtReportRxObssTime);

		pEventVowRxAtReportRxObssTime->ucRxObssBandIdx = TlvData->ucRxObssBandIdx;
		pEventVowRxAtReportRxObssTime->u4RxObssBandTimer = le2cpu32(TlvData->u4RxObssBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"ucRxObssBandIdx = %d, u4RxObssBandTimer = %d\n",
			TlvData->ucRxObssBandIdx, le2cpu32(TlvData->u4RxObssBandTimer));
}

/* UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME (0x16) */
static VOID UniEventVowRxAtReportMibObssTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_CMD_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T pEventVowRxAtReportRxObssTime = &(pVOWParam->EventVowRxAtReportMibObssTime);

		pEventVowRxAtReportRxObssTime->ucRxMibObssBandIdx = TlvData->ucRxMibObssBandIdx;
		pEventVowRxAtReportRxObssTime->u4RxMibObssBandTimer = le2cpu32(TlvData->u4RxMibObssBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"ucRxObssBandIdx = %d, u4RxObssBandTimer = %d\n",
			TlvData->ucRxMibObssBandIdx, le2cpu32(TlvData->u4RxMibObssBandTimer));
}

VOID UniEventVowHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_VOW_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s::Tag(%d, %d)\n",
			__func__, TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME:
			UniEventVowRxAtReportRxNonwifiTime(msg, (void *)tag);
			break;
		case UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME:
			UniEventVowRxAtReportRxNonwifiTime(msg, (void *)tag);
			break;
		case UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME:
			UniEventVowRxAtReportMibObssTime(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#endif /* WIFI_UNIFIED_COMMAND */
