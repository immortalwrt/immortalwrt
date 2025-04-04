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
    tmr.c

    Abstract:
    Support Timing Measurment function.

    Who             When            What
    --------------  ----------      ----------------------------------------------
    Carter          2014-1120       created
*/

#ifdef MT_MAC

#include "rt_config.h"

/*
 *   the format is for QA tool, TMR as prefix,
 *   QA will latch the prefix string then output to file.
 */
void tmr_raw_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);

	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "TMR 0x%04x : ", x);

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%02x ", ((unsigned char)pt[x]));

		if (x % 16 == 15)
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
}


VOID TmrReportParser(RTMP_ADAPTER *pAd, TMR_FRM_STRUC *tmr, BOOLEAN fgFinalResult, UINT32 TOAECalibrationResult)
{
	struct _RMAC_RXD_0_TMR *ptmr_d0 = &tmr->TmrD0;
	UINT32 *ptod_0 = &tmr->ToD0;
	UINT32 *ptoa_0 = &tmr->ToA0;
	TMR_D_6 *tmr_d6 = &tmr->TmrD6;
	TMR_D_2 *ptmr_d2 = &tmr->TmrD2;
	TMR_D_1 *tmr_d1 = &tmr->TmrD1;
	UCHAR   *pta_16 = (UCHAR *)&tmr->Ta16;
	UCHAR   PeerAddr[MAC_ADDR_LEN] = {0};
	static UINT32 lastest_TOA;
	static UINT32 lastest_sn;
	UINT32 tod_fine = tmr_d1->field_init.TodFine;
	UINT32 tod_low = 0;
	UINT32 toa_low = 0;
	UINT32 delta_low = 0;
	UINT32 tod_high = 0;
	UINT32 toa_high = 0;
	UINT32 delta_high = 0;
	UCHAR dbg_lvl = DBG_LVL_INFO;
	UCHAR dbg_lvl_error = DBG_LVL_ERROR;

	if (pAd->pTmrCtrlStruct == NULL)
		return;

	if (pAd->pTmrCtrlStruct->TmrEnable == TMR_DISABLE)
		return;

	pAd->pTmrCtrlStruct->TmrCalResult = TOAECalibrationResult;

		tmr_raw_dump("TMR RAW data: ", (UCHAR *)tmr, sizeof(TMR_FRM_STRUC));

	tod_low = tmr->ToD0;
	toa_low = tmr->ToA0;
	tod_high = tmr->TmrD6.field.ToD32;
	toa_high = tmr->TmrD6.field.ToA32;

	/* calculate delta time */
	if (ptmr_d0->IR == TMR_IR1_RX) {
		delta_low = tod_low - toa_low;
		delta_high = tod_high - toa_high;
	} else {
		delta_low = toa_low - tod_low;
		delta_high = toa_high - tod_high;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
		("DWORD_0: ByteCnt=%d, NC=%d, TMF=%d, ToaVld=%d, TodVld=%d, tod_fine=%x\n",
		ptmr_d0->RxByteCnt, ptmr_d0->Nc, ptmr_d0->Tmf,
		ptmr_d0->ToaVld, ptmr_d0->TodVld, tod_fine));


	if (ptmr_d0->IR == TMR_IR1_RX) {
		/* TMR Responder, Rx case */
		if (lastest_sn == ptmr_d2->field.SnField)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl_error, "##### latest sn is same as last time\n");

		lastest_sn = ptmr_d2->field.SnField;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TYPE=%x, SUB_TYPE=%x, SN=%x\n",
			 ptmr_d0->Type, ptmr_d0->SubType, ptmr_d2->field.SnField));

		PeerAddr[0] = ptmr_d2->field.Ta0;
		PeerAddr[1] = ptmr_d2->field.Ta0 >> 8;
		NdisCopyMemory(PeerAddr + 2, (UCHAR *)pta_16, 4);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
			("Readable TA = "MACSTR"\n",
			MAC2STR(PeerAddr)));

	} else {
		/* TMR Initiator, Tx case */
		if (*ptoa_0 == lastest_TOA) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "##### latest TOA is same as last time\n");
			/* 2015.06.22 location plugfest: do not return */
			return;
		}

		lastest_TOA = *ptoa_0;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TOAECalibrationResult=0x%X\n", TOAECalibrationResult));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_4: TOD[0:31]=0x%x\n", *ptod_0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_6: TOD[32:47]=0x%x\n", tmr_d6->field.ToD32));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_5: TOA[0:31]=0x%x\n", *ptoa_0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("DWORD_6: TOA[32:47]=0x%x\n", tmr_d6->field.ToA32));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl, ("TMR Report: ir = %d, delta_high = %d, delta_low = %d\n\n",
			 ptmr_d0->IR, delta_high, delta_low));
	return;
}

INT TmrUpdateParameter(RTMP_ADAPTER *pAd, UCHAR throughold, UCHAR iter)
{
	if (pAd->pTmrCtrlStruct == NULL)
		return FALSE;

	pAd->pTmrCtrlStruct->TmrThroughold = (throughold != 0) ? throughold : ERROR_DEFAULT_DBM;
	pAd->pTmrCtrlStruct->TmrIter = (iter != 0) ? iter : TOAE_FSM_ITERATION;

	return TRUE;
}

INT TmrCtrlInit(RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver)
{
	INT Ret = NDIS_STATUS_SUCCESS;
	TMR_CTRL_STRUCT *TmrCtrlStructMem = NULL;

	if (pAd->pTmrCtrlStruct != NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
				 "pTmrCtrlStruct is allocated.\n");
	} else {
		Ret = os_alloc_mem(pAd, (PUCHAR *)&TmrCtrlStructMem, sizeof(TMR_CTRL_STRUCT));

		if (!TmrCtrlStructMem) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
					 "alloc TmrCtrl fail!\n");
			return Ret;
		}

		pAd->pTmrCtrlStruct = TmrCtrlStructMem;
		pAd->pTmrCtrlStruct->TmrThroughold = ERROR_DEFAULT_DBM;
		pAd->pTmrCtrlStruct->TmrIter = TOAE_FSM_ITERATION;
	}

	if (!pAd->pTmrCtrlStruct)
		return NDIS_STATUS_FAILURE;

	if (TmrType == TMR_INITIATOR) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "enable TMR report, as Initialiter\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "enable TMR report, as Responder\n");
	}

	pAd->pTmrCtrlStruct->TmrEnable = TmrType;
	AsicSetTmrCR(pAd, TmrType, 0);
	return Ret;
}

VOID TmrCtrlExit(RTMP_ADAPTER *pAd)
{
	if (pAd->pTmrCtrlStruct != NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "disable TMR report\n");
		pAd->pTmrCtrlStruct->TmrEnable = TMR_DISABLE;
		os_free_mem(pAd->pTmrCtrlStruct);
		pAd->pTmrCtrlStruct = NULL;
	}
}

/*
 *   Carter, for now, MT7615 TMR mode is global control.
 *   we shall refine DBDC case in someday.
*/
/* VOID MtSetTmrEnable(RTMP_ADAPTER *pAd, UCHAR TmrType) */
VOID TmrCtrl(RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver)
{
	if (TmrType != TMR_DISABLE)
		TmrCtrlInit(pAd, TmrType, Ver);
	else
		TmrCtrlExit(pAd);

	switch (TmrType) {
	case TMR_INITIATOR:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "enable TMR report, as Initialiter\n");
		AsicSetTmrCR(pAd, TmrType, DBDC_BAND0);
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			AsicSetTmrCR(pAd, TmrType, DBDC_BAND1);
#endif
		break;

	case TMR_RESPONDER:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "enable TMR report, as Responser\n");
		AsicSetTmrCR(pAd, TmrType, DBDC_BAND0);
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			AsicSetTmrCR(pAd, TmrType, DBDC_BAND1);
#endif
		break;

	case TMR_DISABLE:
	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "disable TMR report\n");
		AsicSetTmrCR(pAd, TmrType, DBDC_BAND0);
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			AsicSetTmrCR(pAd, TmrType, DBDC_BAND1);
#endif
		break;
	}

	return;
}

#endif /* MT_MAC */

