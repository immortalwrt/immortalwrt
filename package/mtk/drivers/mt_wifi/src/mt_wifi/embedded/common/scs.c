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
	scs.c
*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "rt_config.h"
#endif

/*
***************************************************************************
*SmartCarrierSense_Gen4
*Base on CONNAC HW Design
*Using CR offset for MT7663 temp solution , it will offload to FW soon.
***************************************************************************
*/

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD

int SCS_Set_FW_Offload(RTMP_ADAPTER *pAd, CMD_SMART_CARRIER_ENABLE Param)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SCS_ENABLE;

	pAd->SCSCtrl.SCSEnable[Param.BandIdx] = Param.SCSEnable;

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
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
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);

	error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Ret = %d_\n", Ret);
	return Ret;
}

VOID SetSCS(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value)
{
	CMD_SMART_CARRIER_ENABLE Param = {0};

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" BandIdx=%d, SCSEnable=%d\n", BandIdx, value);
	if (value > 500) /* traffic threshold.*/
		pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = value;
	else if (value == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;

		Param.BandIdx = 0;
		Param.SCSEnable = SCS_DISABLE;
		SCS_Set_FW_Offload(pAd, Param);

		if (pAd->CommonCfg.dbdc_mode) {
			Param.BandIdx = 1;
			SCS_Set_FW_Offload(pAd, Param);
		}

	} else if (value == SCS_ENABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;

		Param.BandIdx = 0;
		Param.SCSEnable = SCS_ENABLE;
		SCS_Set_FW_Offload(pAd, Param);

		if (pAd->CommonCfg.dbdc_mode) {
			Param.BandIdx = 1;
			SCS_Set_FW_Offload(pAd, Param);
		}
	}
}

VOID SmartCarrierSense_Gen5(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	BandIndex, status;

	pSCSCtrl = &pAd->SCSCtrl;
	for (BandIndex = 0; BandIndex < DBDC_BAND_NUM; BandIndex++) { /* Currently only supported Band0 */
		if (pSCSCtrl->SCSEnable[BandIndex] == SCS_ENABLE) {

			status = SendSCSDataProc(pAd);
			break;
		}
	}
}

VOID SmartCarrierSense_Gen6(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	BandIndex, status;

	pSCSCtrl = &pAd->SCSCtrl;

	for (BandIndex = 0; BandIndex < DBDC_BAND_NUM; BandIndex++) {
		if (pSCSCtrl->SCSEnable[BandIndex] == SCS_ENABLE)
			status = SendSCSDataProc_CONNAC3(pAd, BandIndex);
	}
}

static VOID scs_get_glo_addr_handler(struct cmd_msg *msg,
	char *rsp_payload, UINT16 rsp_payload_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_EVENT_GET_SCS_GLO_ADDR pEntry = NULL;
	P_EVENT_SCS_GLO pFwGlo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pEntry = (P_EVENT_GET_SCS_GLO_ADDR)rsp_payload;
	pFwGlo = &pEntry->rGloInfo;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"SCS_EVENT_GET_GLO_ADDR\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"eventId %u\n", pEntry->u4EventId);

	if (ops->check_scs_glo)
		ops->check_scs_glo(pAd, (VOID *)pFwGlo);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventSCSGetGloAddrHandler(struct cmd_msg *msg, char *rsp_payload)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)msg->priv;
	P_UNI_EVENT_GET_SCS_GLO_ADDR pEntry = (P_UNI_EVENT_GET_SCS_GLO_ADDR)rsp_payload;
	P_EVENT_SCS_GLO pFwGlo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pFwGlo = &pEntry->rGloInfo;

	if (ops->check_scs_glo)
		ops->check_scs_glo(pAd, (VOID *)pFwGlo);
}
#endif /* WIFI_UNIFIED_COMMAND */

static VOID scsEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT32 u4EventId = (*(UINT32 *)rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 " u4EventId = %u, len = %u\n", u4EventId, len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif

	switch (u4EventId) {
	case SCS_EVENT_GET_GLO_ADDR:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 " SCS_EVENT_GET_GLO_ADDR\n");
		scs_get_glo_addr_handler(msg, pData, len);
		break;
	default:
		break;
	}
}

INT ShowScsGloAddr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EVENT_GET_SCS_GLO_ADDR result = {0};
	UINT32 cmd = SCS_GET_GLO_ADDR;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "->");

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &result);
	SET_CMD_ATTR_RSP_HANDLER(attr, scsEventDispatcher);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Ret = %d\n", Ret);
	return Ret;
}

INT ShowSCSinfo_ver2_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_scs_info)
		ops->show_scs_info(pAd);
	else
		return FALSE;

	return TRUE;
}

#else /* SCS_FW_OFFLOAD */

/*
***************************************************************************
*SmartCarrierSense_Gen4
*Base on MT7663 HW Design
***************************************************************************
*/
VOID SmartCarrierSense_Gen4(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	BandIndex;
	UCHAR	BssIndex;
	UINT32 MaxRtsRtyCount = 0;
	UINT32 MaxRtsCount = 0;
	UINT32	CrValue = 0;
	UINT32 TempValue = 0;
	UINT32	PdCount = 0;
	UINT32	MdrdyCount = 0;
	INT32	CckPdBlkBundry = 0;
	INT32	OfdmPdBlkBundry = 0;
	BOOL	Main_Tx = TRUE;
	BOOL	WriteCCKCr = FALSE;
	BOOL	WriteOFDMCr = FALSE;
	BOOL	WriteDefault = FALSE;

	pSCSCtrl = &pAd->SCSCtrl;
	for (BandIndex = 0; BandIndex < BandNum; BandIndex++) { /* Currently only supported Band0 */
		if (pSCSCtrl->SCSEnable[BandIndex] == SCS_ENABLE) {
			for (BssIndex = 0; BssIndex < BssNumScs; BssIndex++) {
				HW_IO_READ32(pAd->hdev_ctrl, MIB_MB0SDR0 + (BssIndex * BssOffset) + (BandIndex * BandOffset), &CrValue);
			TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;
			if (TempValue > MaxRtsRtyCount) {
				MaxRtsRtyCount = TempValue;
				MaxRtsCount = CrValue & RtsCountMask;
			}
		}
			pSCSCtrl->RtsCount[BandIndex] = MaxRtsCount;
			pSCSCtrl->RtsRtyCount[BandIndex] = MaxRtsRtyCount;
			PdCount = pAd->MsMibBucket.PdCount[BandIndex][pAd->MsMibBucket.CurIdx];
			MdrdyCount = pAd->MsMibBucket.MdrdyCount[BandIndex][pAd->MsMibBucket.CurIdx];
			pSCSCtrl->CckFalseCcaCount[BandIndex] = (PdCount & 0xffff) - (MdrdyCount & 0xffff);
			pSCSCtrl->OfdmFalseCcaCount[BandIndex] = ((PdCount & 0xffff0000) >> 16) - ((MdrdyCount & 0xffff0000) >> 16);
			if (pSCSCtrl->OneSecTxByteCount[BandIndex] * TxTrafficTh <  pSCSCtrl->OneSecRxByteCount[BandIndex] || pSCSCtrl->OneSecTxByteCount[BandIndex] == 0)
				Main_Tx = FALSE;
			/*Update Drop Count for Develope SCS Gen4 , Only Support Band0  */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_MPDU_SR0, &CrValue);
			TempValue = (CrValue >> RtsDropCountOffset) & RTSDropCountMask;
			pAd->SCSCtrl.RTS_MPDU_DROP_CNT = TempValue;
			pAd->SCSCtrl.Retry_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			HW_IO_READ32(pAd->hdev_ctrl, MIB_MPDU_SR1, &CrValue);
			pAd->SCSCtrl.LTO_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			if (Main_Tx == TRUE && (pSCSCtrl->RtsCount[BandIndex] > 0 || pSCSCtrl->RtsRtyCount[BandIndex] > 0)) {
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if (pSCSCtrl->CckFalseCcaCount[BandIndex] > pSCSCtrl->CckFalseCcaUpBond[BandIndex] && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.CckPdBlkTh[BandIndex] == PdBlkCckThDefault) {
						pAd->SCSCtrl.CckPdBlkTh[BandIndex] = FastInitTh;
						WriteCCKCr = TRUE;
					} else {
						pSCSCtrl->CckPdBlkTh[BandIndex] += OneStep;
						WriteCCKCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->CckFalseCcaCount[BandIndex] < pSCSCtrl->CckFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->CckPdBlkTh[BandIndex] - OneStep >= PdBlkCckThDefault) {
							pSCSCtrl->CckPdBlkTh[BandIndex] -= OneStep;
							WriteCCKCr = TRUE;
						} else {
							WriteCCKCr = FALSE;
						}
				} else {
					WriteCCKCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				CckPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) + 256), pSCSCtrl->CckFixedRssiBond[BandIndex]);
				if (pSCSCtrl->CckPdBlkTh[BandIndex] > CckPdBlkBundry) {
					pSCSCtrl->CckPdBlkTh[BandIndex] = CckPdBlkBundry;
					WriteCCKCr = TRUE;
					}
				if (WriteCCKCr) {
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pSCSCtrl->PHY_RXTD_CCKPD_7_OFFSET, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pSCSCtrl->PHY_RXTD_CCKPD_7_OFFSET, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pSCSCtrl->PHY_RXTD_CCKPD_8_OFFSET, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pSCSCtrl->PHY_RXTD_CCKPD_8_OFFSET, CrValue);
				}
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if ((pSCSCtrl->OfdmFalseCcaCount[BandIndex] > pSCSCtrl->OfdmFalseCcaUpBond[BandIndex]) && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] == PdBlkOfmdThDefault) {
						pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] = FastInitThOfdm;
						WriteOFDMCr = TRUE;
					} else {
						pSCSCtrl->OfdmPdBlkTh[BandIndex] += OneStep;
						WriteOFDMCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->OfdmFalseCcaCount[BandIndex] < pSCSCtrl->OfdmFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->OfdmPdBlkTh[BandIndex] - OneStep >= PdBlkOfmdThDefault) {
							pSCSCtrl->OfdmPdBlkTh[BandIndex] -= OneStep;
							WriteOFDMCr = TRUE;
						} else {
							WriteOFDMCr = FALSE;
						}
				} else {
					WriteOFDMCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) * 2 + 512),  pSCSCtrl->OfdmFixedRssiBond[BandIndex]);
				if (pSCSCtrl->OfdmPdBlkTh[BandIndex] > OfdmPdBlkBundry) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = OfdmPdBlkBundry;
					WriteOFDMCr = TRUE;
				}
				if (WriteOFDMCr) {
					if (BandIndex == 0) {
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pSCSCtrl->PHY_MIN_PRI_PWR_OFFSET, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pSCSCtrl->PHY_MIN_PRI_PWR_OFFSET, CrValue);
					}
#ifdef DBDC_MODE
					if (BandIndex == 1) {
						HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
#endif
				}
			} else {
					MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Disable SCS due to RtsCount=%d RtsRtyCount=%d Main_Tx=%d\n", pSCSCtrl->RtsCount[BandIndex], pSCSCtrl->RtsRtyCount[BandIndex], Main_Tx);
					WriteDefault = TRUE;
				}
		} else {
			WriteDefault = TRUE;
		}
		if (WriteDefault) {
			if (pSCSCtrl->CckPdBlkTh[BandIndex] != PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[BandIndex] = PdBlkCckThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pSCSCtrl->PHY_RXTD_CCKPD_7_OFFSET, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pSCSCtrl->PHY_RXTD_CCKPD_7_OFFSET, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pSCSCtrl->PHY_RXTD_CCKPD_8_OFFSET, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pSCSCtrl->PHY_RXTD_CCKPD_8_OFFSET, CrValue);
				}
			if (pSCSCtrl->OfdmPdBlkTh[BandIndex] != PdBlkOfmdThDefault) {
				if (BandIndex == 0) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pSCSCtrl->PHY_MIN_PRI_PWR_OFFSET, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pSCSCtrl->PHY_MIN_PRI_PWR_OFFSET, CrValue);
			}
#ifdef DBDC_MODE
				if (BandIndex == 1) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
					HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
				}
#endif
			}
		}
	}
}

/*
***************************************************************************
*SmartCarrierSense_Gen3
*Base on MT7622 HW Design
***************************************************************************
*/
VOID SmartCarrierSense_Gen3(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	BandIndex;
	UCHAR	BssIndex;
	UINT32 MaxRtsRtyCount = 0;
	UINT32 MaxRtsCount = 0;
	UINT32	CrValue = 0;
	UINT32 TempValue = 0;
	UINT32	PdCount = 0;
	UINT32	MdrdyCount = 0;
	INT32	CckPdBlkBundry = 0;
	INT32	OfdmPdBlkBundry = 0;
	BOOL	Main_Tx = TRUE;
	BOOL	WriteCCKCr = FALSE;
	BOOL	WriteOFDMCr = FALSE;
	BOOL	WriteDefault = FALSE;

	pSCSCtrl = &pAd->SCSCtrl;
	for (BandIndex = 0; BandIndex < BandNum; BandIndex++) { /* Currently only supported Band0 */
		if (pSCSCtrl->SCSEnable[BandIndex] == SCS_ENABLE) {
			for (BssIndex = 0; BssIndex < BssNumScs; BssIndex++) {
				HW_IO_READ32(pAd->hdev_ctrl, MIB_MB0SDR0 + (BssIndex * BssOffset) + (BandIndex * BandOffset), &CrValue);
			TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;
			if (TempValue > MaxRtsRtyCount) {
				MaxRtsRtyCount = TempValue;
				MaxRtsCount = CrValue & RtsCountMask;
			}
		}
			pSCSCtrl->RtsCount[BandIndex] = MaxRtsCount;
			pSCSCtrl->RtsRtyCount[BandIndex] = MaxRtsRtyCount;
			PdCount = pAd->MsMibBucket.PdCount[BandIndex][pAd->MsMibBucket.CurIdx];
			MdrdyCount = pAd->MsMibBucket.MdrdyCount[BandIndex][pAd->MsMibBucket.CurIdx];
			pSCSCtrl->CckFalseCcaCount[BandIndex] = (PdCount & 0xffff) - (MdrdyCount & 0xffff);
			pSCSCtrl->OfdmFalseCcaCount[BandIndex] = ((PdCount & 0xffff0000) >> 16) - ((MdrdyCount & 0xffff0000) >> 16);
			if (pSCSCtrl->OneSecTxByteCount[BandIndex] * TxTrafficTh <  pSCSCtrl->OneSecRxByteCount[BandIndex] || pSCSCtrl->OneSecTxByteCount[BandIndex] == 0)
				Main_Tx = FALSE;
			/*Update Drop Count for Develope SCS Gen3 , Only Support Band0  */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_MPDU_SR0, &CrValue);
			TempValue = (CrValue >> RtsDropCountOffset) & RTSDropCountMask;
			pAd->SCSCtrl.RTS_MPDU_DROP_CNT = TempValue;
			pAd->SCSCtrl.Retry_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			HW_IO_READ32(pAd->hdev_ctrl, MIB_MPDU_SR1, &CrValue);
			pAd->SCSCtrl.LTO_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			if (Main_Tx == TRUE && (pSCSCtrl->RtsCount[BandIndex] > 0 || pSCSCtrl->RtsRtyCount[BandIndex] > 0)) {
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if (pSCSCtrl->CckFalseCcaCount[BandIndex] > pSCSCtrl->CckFalseCcaUpBond[BandIndex] && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.CckPdBlkTh[BandIndex] == PdBlkCckThDefault) {
						pAd->SCSCtrl.CckPdBlkTh[BandIndex] = FastInitTh;
						WriteCCKCr = TRUE;
					} else {
						pSCSCtrl->CckPdBlkTh[BandIndex] += OneStep;
						WriteCCKCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->CckFalseCcaCount[BandIndex] < pSCSCtrl->CckFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->CckPdBlkTh[BandIndex] - OneStep >= PdBlkCckThDefault) {
							pSCSCtrl->CckPdBlkTh[BandIndex] -= OneStep;
							WriteCCKCr = TRUE;
						} else {
							WriteCCKCr = FALSE;
						}
				} else {
					WriteCCKCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				CckPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) + 256), pSCSCtrl->CckFixedRssiBond[BandIndex]);
				if (pSCSCtrl->CckPdBlkTh[BandIndex] > CckPdBlkBundry) {
					pSCSCtrl->CckPdBlkTh[BandIndex] = CckPdBlkBundry;
					WriteCCKCr = TRUE;
					}
				if (WriteCCKCr) {
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
				}
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if ((pSCSCtrl->OfdmFalseCcaCount[BandIndex] > pSCSCtrl->OfdmFalseCcaUpBond[BandIndex]) && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] == PdBlkOfmdThDefault) {
						pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] = FastInitThOfdm;
						WriteOFDMCr = TRUE;
					} else {
						pSCSCtrl->OfdmPdBlkTh[BandIndex] += OneStep;
						WriteOFDMCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->OfdmFalseCcaCount[BandIndex] < pSCSCtrl->OfdmFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->OfdmPdBlkTh[BandIndex] - OneStep >= PdBlkOfmdThDefault) {
							pSCSCtrl->OfdmPdBlkTh[BandIndex] -= OneStep;
							WriteOFDMCr = TRUE;
						} else {
							WriteOFDMCr = FALSE;
						}
				} else {
					WriteOFDMCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) * 2 + 512),  pSCSCtrl->OfdmFixedRssiBond[BandIndex]);
				if (pSCSCtrl->OfdmPdBlkTh[BandIndex] > OfdmPdBlkBundry) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = OfdmPdBlkBundry;
					WriteOFDMCr = TRUE;
				}
				if (WriteOFDMCr) {
					if (BandIndex == 0) {
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
					} else if (BandIndex == 1) {
						HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			} else {
					MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Disable SCS due to RtsCount=%d RtsRtyCount=%d Main_Tx=%d\n", pSCSCtrl->RtsCount[BandIndex], pSCSCtrl->RtsRtyCount[BandIndex], Main_Tx);
					WriteDefault = TRUE;
				}
		} else {
			WriteDefault = TRUE;
		}
		if (WriteDefault) {
			if (pSCSCtrl->CckPdBlkTh[BandIndex] != PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[BandIndex] = PdBlkCckThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
				}
			if (pSCSCtrl->OfdmPdBlkTh[BandIndex] != PdBlkOfmdThDefault) {
				if (BandIndex == 0) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
			}
				if (BandIndex == 1) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
					HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
				}
			}
		}
	}
}

/*
***************************************************************************
*SmartCarrierSense_Gen2
*Base on MT7615 HW Design
***************************************************************************
*/
VOID SmartCarrierSense_Gen2(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	BOOL	RxOnly = FALSE;
	UINT32 TotalTP = 0, CrValue = 0;
	INT32	 CckPdBlkBundry = 0, OfdmPdBlkBundry = 0;
	UCHAR i;
	UCHAR idx;
	UINT32 MaxRtsRtyCount = 0;
	UINT32 MaxRtsCount = 0;
	UINT32 TempValue = 0;
	BOOL WriteCr = FALSE;
	UINT32 PdCount = 0, MdrdyCount = 0;

	pSCSCtrl = &pAd->SCSCtrl;

	/* 2. Tx/Rx */
	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
	/* ("%s Band0:Tx/Rx=%d/%d MinRSSI=%d, Band1:Tx/Rx=%d/%d, MinRSSI=%d\n", */
	/* __FUNCTION__, pAd->SCSCtrl.OneSecTxByteCount[0], pAd->SCSCtrl.OneSecRxByteCount[0], pAd->SCSCtrl.SCSMinRssi[0], */
	/* pAd->SCSCtrl.OneSecTxByteCount[1], pAd->SCSCtrl.OneSecRxByteCount[1], pAd->SCSCtrl.SCSMinRssi[1])); */

	/* 3. based on minRssi to adjust PD_BLOCK_TH */
	for (i = 0; i < 1; i++) { /* NO DBDC support. */
		for (idx = 0; idx < 4; idx++) {
			HW_IO_READ32(pAd->hdev_ctrl, MIB_MB0SDR0 + (idx * BssOffset) + (i * BandOffset), &CrValue);
			TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;

			if (TempValue > MaxRtsRtyCount) {
				MaxRtsRtyCount = TempValue;
				MaxRtsCount = CrValue & RtsCountMask;
			}
		}

		pSCSCtrl->RtsCount[i] = MaxRtsCount;
		pSCSCtrl->RtsRtyCount[i] = MaxRtsRtyCount;
		PdCount = pAd->MsMibBucket.PdCount[i][pAd->MsMibBucket.CurIdx];
		MdrdyCount = pAd->MsMibBucket.MdrdyCount[i][pAd->MsMibBucket.CurIdx];
		/* printk("PD_count=%x, MDRSY_count=%x\n", CrValue, CrValue2); */
		pSCSCtrl->CckFalseCcaCount[i] = (PdCount & 0xffff) - (MdrdyCount & 0xffff);
		pSCSCtrl->OfdmFalseCcaCount[i] = ((PdCount & 0xffff0000) >> 16) - ((MdrdyCount & 0xffff0000) >> 16);

		if (pSCSCtrl->SCSEnable[i] == SCS_ENABLE) {
			TotalTP = (pSCSCtrl->OneSecTxByteCount[i] + pSCSCtrl->OneSecRxByteCount[i]);

			if ((pSCSCtrl->OneSecTxByteCount[i]) * 9 <  pSCSCtrl->OneSecRxByteCount[i])
				RxOnly = TRUE;

			/* if (1 TotalTP > pSCSCtrl->SCSTrafficThreshold[i]) {*/ /* default 2M */
			if ((pSCSCtrl->RtsCount[i] > 0 || pSCSCtrl->RtsRtyCount[i] > 0) && RxOnly == FALSE) {
				/* Set PD_BLOCKING_BOUNDARY */
				CckPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i]) + 256), pSCSCtrl->CckFixedRssiBond[i]);

				/* CCK part */
				if ((pSCSCtrl->CckFalseCcaCount[i] > pSCSCtrl->CckFalseCcaUpBond[i])) { /* Decrease coverage */
					if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {/* RTS PER < 40% */
						if (pAd->SCSCtrl.CckPdBlkTh[i] == PdBlkCckThDefault && CckPdBlkBundry > FastInitTh) {
							pAd->SCSCtrl.CckPdBlkTh[i] = FastInitTh;
							WriteCr = TRUE;
						}
						/* pSCSCtrl->CckPdBlkTh[i] += 2; //One step is 2dB. */
						else if ((pSCSCtrl->CckPdBlkTh[i] + OneStep) <= CckPdBlkBundry) {
							pSCSCtrl->CckPdBlkTh[i] += OneStep;
							/* Write to CR */
							WriteCr = TRUE;
						} else  if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) {
							pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
							/* Write to CR */
							WriteCr = TRUE;
						}
					}
				} else if (pSCSCtrl->CckFalseCcaCount[i] < pSCSCtrl->CckFalseCcaLowBond[i] || (MaxRtsCount + (MaxRtsCount >> 1)) < MaxRtsRtyCount) { /* Increase coverage */
					if (pSCSCtrl->CckPdBlkTh[i] - OneStep >= PdBlkCckThDefault) {
						pSCSCtrl->CckPdBlkTh[i] -= OneStep;

						if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) /* Tracking mini RSSI to prevent out of service rage. */
							pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;

						/* Write to CR */
						WriteCr = TRUE;
					}
				} else { /* Stable stat */
					if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) {/* Tracking mini RSSI to prevent out of service rage. */
						pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
						WriteCr = TRUE;
					}
				}

				if (WriteCr) { /* Write for CCK PD blocking */
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
					CrValue |= (pSCSCtrl->CckPdBlkTh[i]  << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
					CrValue |= (pSCSCtrl->CckPdBlkTh[i]  << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
				}

				WriteCr = FALSE; /* Clear */
				/* OFDM part */
				/* Set PD_BLOCKING_BOUNDARY */
				OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i]) * 2 + 512),  pSCSCtrl->OfdmFixedRssiBond[i]);

				if (pSCSCtrl->OfdmFalseCcaCount[i] > pSCSCtrl->OfdmFalseCcaUpBond[i]) { /* Decrease coverage */
					if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {/* RTS PER < 40% */
						if (pAd->SCSCtrl.OfdmPdBlkTh[i] == PdBlkOfmdThDefault && OfdmPdBlkBundry > FastInitThOfdm) {
							pAd->SCSCtrl.OfdmPdBlkTh[i] = FastInitThOfdm;
							WriteCr = TRUE;
						}

						if ((pSCSCtrl->OfdmPdBlkTh[i] + OneStep) <= OfdmPdBlkBundry) {
							pSCSCtrl->OfdmPdBlkTh[i] += OneStep;
							/* Write to CR */
							WriteCr = TRUE;
						} else  if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) {
							pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
							/* Write to CR */
							WriteCr = TRUE;
						}
					}
				} else if (pSCSCtrl->OfdmFalseCcaCount[i] < pSCSCtrl->OfdmFalseCcaLowBond[i] || (MaxRtsCount + (MaxRtsCount >> 1)) < MaxRtsRtyCount) {
					/* Increase coverage */
					if (pSCSCtrl->OfdmPdBlkTh[i] - OneStep >= PdBlkOfmdThDefault) {
						pSCSCtrl->OfdmPdBlkTh[i] -= OneStep;

						if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry)/* Tracking mini RSSI to prevent out of service rage. */
							pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;

						/* Write to CR */
						WriteCr = TRUE;
					}
				} else { /* Stable stat */
					if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) {/* Tracking mini RSSI to prevent out of service rage. */
						pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
						WriteCr = TRUE;
					}
				}

				if (WriteCr) {/* Write for OFDM PD blocking */
					if (i == 0) {
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
					} else if (i == 1) { /* DBDC */
						HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			} else { /* Disable SCS  No traffic */
				if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {
					MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Disable SCS due to RtsCount=%d RxOnly=%d\n", pSCSCtrl->RtsCount[i], RxOnly);
					pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);

					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					/* 0x92 is default value -110dBm */
					CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
					/* Bit[31:24] */
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					/* 0x92 is default value -110dBm */
					CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
				}

				if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
					if (i == 0) {
						pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
					} else if (i == 1) {
						pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			}
		} else {
			if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
				HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue)
				/* Bit[8:1] */;
				CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
				CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
				 /* 0x92 is default value -110dBm */
				HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
				HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
				/* Bit[31:24] */
				CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
				/* 0x92 is default value -110dBm */
				CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
				HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
			}

			if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
				if (i == 0) {
					pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
					/* OFDM PD BLOCKING TH */
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
					HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
				} else if (i == 1) {
					pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
					 /* OFDM PD BLOCKING TH */
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
					HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
				}
			}
		}
	}
}

VOID SetSCS(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value)
{
	UINT32 CrValue;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"BandIdx=%d, SCSEnable=%d\n", BandIdx, value);
	if (value > 500) /* traffic threshold.*/
		pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = value;
	else if (value == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;
		/* Disable PD blocking and reset related CR */
		HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET, &CrValue);
		/* CrValue &= ~(0x1 << PdBlkEnabeOffset);  Bit[19] */
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pAd->SCSCtrl.PHY_RXTD_CCKPD_7_OFFSET, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
		CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7 + pAd->SCSCtrl.PHY_RXTD_CCKPD_7_OFFSET, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pAd->SCSCtrl.PHY_RXTD_CCKPD_8_OFFSET, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
		CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8 + pAd->SCSCtrl.PHY_RXTD_CCKPD_8_OFFSET, CrValue);
	} else if (value == SCS_ENABLE)
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;
}
#endif

VOID SCS_init(RTMP_ADAPTER *pAd)
{
	UINT32 CrValue = 0;
	UCHAR num = BandNum;

		/* Enable Band0 PD_BLOCKING */
		HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET, &CrValue);
		CrValue |= (0x1 << PdBlkEnabeOffset);/* Bit[19] */
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR + pAd->SCSCtrl.PHY_MIN_PRI_PWR_OFFSET, CrValue);
		if (pAd->SCSCtrl.SCSGeneration == SCS_Gen2 && num > 1) {
			/* Enable Band1 PD_BLOCKING & initail PD_BLOCKING_TH */
			HW_IO_READ32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, &CrValue);
			CrValue |= (0x1 << PdBlkEnabeOffsetB1);/* Bit[25] */
			CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);/* OFDM PD BLOCKING TH */
			CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
			HW_IO_WRITE32(pAd->hdev_ctrl, BAND1_PHY_MIN_PRI_PWR, CrValue);
		}
		if (pAd->SCSCtrl.SCSGeneration >= SCS_Gen3) {
			/* Enable Band0 RTS Drop Count */
			HW_IO_READ32(pAd->hdev_ctrl, M0_MISC_CR, &CrValue);
			CrValue |= (0x7 << RTSDropRdClrEnabeOffset);/* Bit[10:8] */
			CrValue |= (0x7 << RTSDropCntEnabeOffset);/* Bit[2:0] */
			HW_IO_WRITE32(pAd->hdev_ctrl, M0_MISC_CR, CrValue);
		}

#ifdef SCS_FW_OFFLOAD
		if (pAd->SCSCtrl.SCSEnable[DBDC_BAND0] == 1)
			Set_SCSDefaultEnable(pAd, DBDC_BAND0, SCS_ENABLE);
		else
			Set_SCSDefaultEnable(pAd, DBDC_BAND0, SCS_DISABLE);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"INIT SCSEnable [0]= %d\n", pAd->SCSCtrl.SCSEnable[DBDC_BAND0]);
#ifdef DBDC_MODE
		if (pAd->SCSCtrl.SCSEnable[DBDC_BAND1] == 1)
			Set_SCSDefaultEnable(pAd, DBDC_BAND1, SCS_ENABLE);
		else
			Set_SCSDefaultEnable(pAd, DBDC_BAND1, SCS_DISABLE);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"INIT SCSEnable [1]= %d\n", pAd->SCSCtrl.SCSEnable[DBDC_BAND1]);
#endif

#else
		Set_SCSEnable_Proc(pAd, "1");
#endif /*SCS_FW_OFFLOAD*/
}

#endif /* SMART_CARRIER_SENSE_SUPPORT */
