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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"
#include "dbg_txcmd.h"
#include "dbg_txcmd_framework.h"

#define DBG_TXCMD_MODNAME "TXCMDSU"

static const char cmdrpt_to_str[4][16] = {
"SPL", /*CMDRPT_TYPE_SPL*/
"TXDATA", /*CMDRPT_TYPE_TXDATA*/
"TXTRIG", /*CMDRPT_TYPE_TXTRIG*/
"RXRPT", /*CMDRPT_TYPE_RXRPT*/
};

static const char cmdrpt_order_to_str[CMDRPT_ORDER_MAX][16] = {
"TXCMD_MODUL",
"TWT",
"ACTRL",
"MURU",
"SU",
"RXV",
"RA",
"SR",
"TXCMD_TX_CTRL",
};


static INT txcmdsu_dbg_manual_mode_tx(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dbg_wmcu_request req;
	struct dbg_txcmdsu_cmd sucmd;

	INT32 recv;
	UINT32 txmode = 0, rate = 0, gi = 0, bw = 0, nss = 0, heltf = 0, ecc = 0, stbc = 0;
	UINT32 wmtype = 0;
	DBG_TXCMD_LOG("dbg_manual_mode_tx :\n");
	DBG_TXCMD_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d", &(wmtype), &(txmode), &(rate), &(bw), &(nss), &(gi), &(heltf), &(ecc), &(stbc));
	sucmd.eType = wmtype;
	sucmd.ucTxMode = txmode;
	sucmd.ucRate = rate;
	sucmd.ucGi = gi;
	sucmd.ucNss = nss;
	sucmd.ucBw = bw;
	sucmd.ucHeLtf = heltf;
	sucmd.ucEcc = ecc;
	sucmd.ucStbc = stbc;

	if (recv != 9) {
	DBG_TXCMD_LOG("Invalid input! input:%d", recv);
	DBG_TXCMD_LOG("iwpriv ra0 set dbg_txcmd=txcmdsu-1-[Type]-[TxMode]-[BW]-[RATE]-[Nss]-[GI]-[HeLTF]-[ECC]-[STBC]");
	DBG_TXCMD_LOG("[Type]0=TXDATA, 1=Protect, 2=Trigger");
	DBG_TXCMD_LOG("[TxMode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4, HE_SU=8, HE_EXT_SU=9, HE_TRIG=10");
	DBG_TXCMD_LOG("[BW]BW20=0, BW40=1, BW80=2,BW160=3");
	DBG_TXCMD_LOG("[Rate]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9");
	DBG_TXCMD_LOG("[GI]HE: 0=0.8us, 1=1.6us, 2=3.2us HT/VHT: 0=0.4us, 1=0.8us");
	DBG_TXCMD_LOG("[HeLTF] 0=1xLTF, 1=2xLTF, 2=4xLTF");
	DBG_TXCMD_LOG("[Nss]Nss= 0, 1, 2, 3");
	DBG_TXCMD_LOG("[ECC]0=BCC, 1=LDPC");
	DBG_TXCMD_LOG("[STBC]0=No STBC, 1=STBC");
	return DBG_TXCMD_STATUS_FAIL;
	}

	DBG_TXCMD_LOG("%s():Type= %d, TxMode: %d, Rate: %d, GI: %d, BW: %d\n"
			"\t\tNss: %d, HeLTF:%d, ECC: %d, STBC: %d",
			 __func__, wmtype, txmode, rate, gi, bw, nss, heltf, ecc, stbc);

	req.feature_id = ENUM_DBG_TXCMDSU;
	req.type = ENUM_DBG_TXCMDSU_TXMODE;
	req.len = sizeof(sucmd) + sizeof(wmtype);
	req.payload = (UCHAR *) &sucmd;
	req.resp_handle = NULL;
	req.resp_len = 0;
	req.resp = NULL;
	if (dbg_ut_wmcu_send(ad, &req) != DBG_TXCMD_STATUS_OK) {
		DBG_TXCMD_LOG("result,fail");
	}

	return DBG_TXCMD_STATUS_OK;
}

static INT txcmdsu_dbg_configure(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dbg_wmcu_request req;
	struct _DBG_TXCMDSU_CMD_CFG sucmd;

	INT32 recv;
	UINT32 type = 0, val_type = 0, val = 0;

	DBG_TXCMD_LOG("SU Configure:");
	DBG_TXCMD_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%d-%d", &type, &val_type, &val);

	sucmd.eType = type;
	sucmd.eValType = val_type;
	sucmd.u4Val = val;

	if (recv != 3) {
		DBG_TXCMD_LOG("Invalid cmd input! input: %d", recv);
		DBG_TXCMD_LOG("iwpriv ra0 set dvt=txcmdsu-2-[Type][ValType][Val]");
		DBG_TXCMD_LOG("[Type]0: TXDATA, 1: Protect, 2: Trigger");
		DBG_TXCMD_LOG("[ValType]");
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_PROTECT_EN (Type: TRIG/TXDATA)", TXCMD_SU_VAL_PROTECT_EN);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_PROTECT_TYPE (Type: TRIG/TXDATA)", TXCMD_SU_VAL_PROTECT_TYPE);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_RATE_TYPE (Type: ALL)", TXCMD_SU_VAL_RATE_TYPE);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_DURATION (Type: TXDATA/TRIG)", TXCMD_SU_VAL_DURATION);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_LSIG (Type: TRIG)", TXCMD_SU_VAL_LSIG);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_EN (Type: TRIG)", TXCMD_SU_VAL_TRIG_EN);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_TYPE (Type: TRIG)", TXCMD_SU_VAL_TRIG_TYPE);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_INTERVAL (Type: TRIG)", TXCMD_SU_VAL_TRIG_INTERVAL);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_MUSPCFAC (Type: TRIG)", TXCMD_SU_VAL_TRIG_MUSPCFAC);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_CS (Type: TRIG)", TXCMD_SU_VAL_TRIG_CS);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_SPL (Type: TRIG/TXDATA)", TXCMD_SU_VAL_SPL);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_BW (Type: TRIG/TXDATA)", TXCMD_SU_VAL_BW);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_MCS (Type: TRIG/TXDATA)", TXCMD_SU_VAL_MCS);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_GI (Type: TRIG/TXDATA)", TXCMD_SU_VAL_GI);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_LTF (Type: TRIG/TXDATA)", TXCMD_SU_VAL_LTF);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_COD (Type: TRIG/TXDATA)", TXCMD_SU_VAL_COD);
		DBG_TXCMD_LOG("\t%d:TXCMD_SU_VAL_TRIG_TX_INTERNAL (Type: TRIG)", TXCMD_SU_VAL_TRIG_TX_INTERNAL);
		return DBG_TXCMD_STATUS_FAIL;
	}
	DBG_TXCMD_LOG("%s():\t Type: %d, VType: %d, Val: %d",
			__func__, type, val_type, val);

	req.feature_id = ENUM_DBG_TXCMDSU;
	req.type = ENUM_DBG_TXCMDSU_TRIG_CFG;
	req.len = sizeof(sucmd);
	req.payload = (UCHAR *) &sucmd;
	req.resp_handle = NULL;
	req.resp_len = 0;
	req.resp = NULL;
	if (dbg_ut_wmcu_send(ad, &req) != DBG_TXCMD_STATUS_OK) {
		DBG_TXCMD_LOG("result,fail");
	}

	return DBG_TXCMD_STATUS_OK;

}


static VOID txcmdsu_dbg_state_dump(struct dbg_txcmdsu_statistic *state)
{
	UCHAR idx = 0;
	struct HE_ACTRL_OMI *om;
	DBG_TXCMD_LOG("TX CMD Setting:");
	DBG_TXCMD_LOG("SPL EN    : %d", state->spl_en);

	for (idx = 0 ; idx < DBG_TXCMD_WMM_SET; idx++) {
		DBG_TXCMD_LOG("TXCMD MODE WMM(%d): %d",
		idx,
		state->txcmd_mode[idx]);
	}

	state->omi = le2cpu16(state->omi);
	om = (struct HE_ACTRL_OMI *) &state->omi;

	DBG_TXCMD_LOG("=============================TX CTRL Status===================================\n");
	DBG_TXCMD_LOG("TX CTRL Status:");
	DBG_TXCMD_LOG("FreeQ Len     : %d", state->swq_len[0]);
	DBG_TXCMD_LOG("TxQ Len       : %d", state->swq_len[1]);
	DBG_TXCMD_LOG("TxDoneQ Len   : %d", state->swq_len[2]);
	DBG_TXCMD_LOG("ACQLen   :\n");

	for (idx = 0 ; idx < 4; idx++) {

		UCHAR start_idx = idx * 4;
		DBG_TXCMD_LOG("WMMQ(%d): %d, %d, %d, %d",
		idx,
		state->acq_len[0 + start_idx],
		state->acq_len[1 + start_idx],
		state->acq_len[2 + start_idx],
		state->acq_len[3 + start_idx]);
	}

	DBG_TXCMD_LOG("ALTX: %d,TF: %d,TWT_TSF_TF: %d,TWT_DL: %d,TWT_UL: %d",
		state->acq_len[DBG_TXCMD_WH_TXC_ALTX0],
		state->acq_len[DBG_TXCMD_WH_TXC_TF0],
		state->acq_len[DBG_TXCMD_WH_TXC_TWT_TSF_TF0],
		state->acq_len[DBG_TXCMD_WH_TXC_TWT_DL0],
		state->acq_len[DBG_TXCMD_WH_TXC_TWT_UL0]);

	DBG_TXCMD_LOG("=============================TX-Statistic=====================================\n");
	DBG_TXCMD_LOG("MU TX-DATA    : %d", state->tx_cnt[DBG_WH_TXCMD_MU_TX_DATA]);
	DBG_TXCMD_LOG("SU TX-DATA    : %d", state->tx_cnt[DBG_WH_TXCMD_SU_TX_DATA]);
	DBG_TXCMD_LOG("NON HE TX-DATA: %d", state->tx_cnt[DBG_WH_TXCMD_NONE_HE_TX_DATA]);
	DBG_TXCMD_LOG("MU TRIG-DATA  : %d", state->tx_cnt[DBG_WH_TXCMD_MU_TRIG_DATA]);
	DBG_TXCMD_LOG("SU TRIG-DATA  : %d", state->tx_cnt[DBG_WH_TXCMD_SU_TRIG_DATA]);
	DBG_TXCMD_LOG("HE Sounding   : %d", state->tx_cnt[DBG_WH_TXCMD_HE_SOUNDING]);
	DBG_TXCMD_LOG("SW PKT        : %d", state->tx_cnt[DBG_WH_TXCMD_SW_PKT]);
	DBG_TXCMD_LOG("=============================TX-Error=====================================");
	DBG_TXCMD_LOG("MU TX-DATA    : %d", state->tx_err_cnt[DBG_WH_TXCMD_MU_TX_DATA]);
	DBG_TXCMD_LOG("SU TX-DATA    : %d", state->tx_err_cnt[DBG_WH_TXCMD_SU_TX_DATA]);
	DBG_TXCMD_LOG("NON HE TX-DATA: %d", state->tx_err_cnt[DBG_WH_TXCMD_NONE_HE_TX_DATA]);
	DBG_TXCMD_LOG("MU TRIG-DATA  : %d", state->tx_err_cnt[DBG_WH_TXCMD_MU_TRIG_DATA]);
	DBG_TXCMD_LOG("SU TRIG-DATA  : %d", state->tx_err_cnt[DBG_WH_TXCMD_SU_TRIG_DATA]);
	DBG_TXCMD_LOG("HE Sounding   : %d", state->tx_err_cnt[DBG_WH_TXCMD_HE_SOUNDING]);
	DBG_TXCMD_LOG("SW PKT        : %d", state->tx_err_cnt[DBG_WH_TXCMD_SW_PKT]);
	DBG_TXCMD_LOG("=============================RX-Statistic=====================================");
	DBG_TXCMD_LOG("RX-DATA Report: %d", state->rx_cnt[DBG_CMDRPT_TYPE_RXRPT]);
	DBG_TXCMD_LOG("TX-DATA Report: %d", state->rx_cnt[DBG_CMDRPT_TYPE_TXDATA]);
	DBG_TXCMD_LOG("TX-TRIG Report: %d", state->rx_cnt[DBG_CMDRPT_TYPE_TXTRIG]);
	DBG_TXCMD_LOG("SPL Report    : %d", state->rx_cnt[DBG_CMDRPT_TYPE_SPL]);
	DBG_TXCMD_LOG("============================SPL Statistic=====================================");
	DBG_TXCMD_LOG("TX DONE     : %d", state->spl_cnt[DBG_WH_SPL_TXCMD_DONE_TX_MODE]);
	DBG_TXCMD_LOG("RX DONE     : %d", state->spl_cnt[DBG_WH_SPL_TXCMD_DONE_RX_MODE]);
	DBG_TXCMD_LOG("TWT DL DONE : %d", state->spl_cnt[DBG_WH_SPL_TXCMD_DONE_TWT_DL_MODE]);
	DBG_TXCMD_LOG("TWT UL DONE : %d", state->spl_cnt[DBG_WH_SPL_TXCMD_DONE_TWT_UL_MODE]);
	DBG_TXCMD_LOG("IO TX       : %d", state->spl_cnt[DBG_WH_SPL_IO_TX_MODE]);
	DBG_TXCMD_LOG("IO RX       : %d", state->spl_cnt[DBG_WH_SPL_IO_RX_MODE]);
	DBG_TXCMD_LOG("NON EMPTY TX: %d", state->spl_cnt[DBG_WH_SPL_CHNL_NONEMPTY_TX_MODE]);
	DBG_TXCMD_LOG("NON EMPTY RX: %d", state->spl_cnt[DBG_WH_SPL_CHNL_NONEMPTY_RX_MODE]);
	DBG_TXCMD_LOG("============================OM=====================================");
	DBG_TXCMD_LOG("OMI BW      : %d", om->bw);
	DBG_TXCMD_LOG("OMI NSS     : %d", om->tx_nss);
	DBG_TXCMD_LOG("OMI MU      : %d", om->ul_mu_disable);

}

static INT txcmdsu_dbg_state_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dbg_wmcu_request req;
	struct dbg_txcmdsu_statistic respon;

	DBG_TXCMD_LOG("txcmd statistic:\n");
	DBG_TXCMD_LOG("arg,%s", arg);

	req.feature_id = ENUM_DBG_TXCMDSU;
	req.type = ENUM_DBG_TXCMD_STAT;
	req.len = 0;
	req.payload = NULL;
	req.resp_handle = NULL;
	req.resp_len = sizeof(respon);
	req.resp = (UCHAR *) &respon;
	if (dbg_ut_wmcu_send(ad, &req) != DBG_TXCMD_STATUS_OK) {
		DBG_TXCMD_LOG("result,fail");
	}
	txcmdsu_dbg_state_dump(&respon);
	return DBG_TXCMD_STATUS_OK;
}

static VOID txcmdsu_dbg_rate_dump(struct _DBG_RATE_INFO_RSP_T *rate)
{
	DBG_TXCMD_LOG("TXMODE: %d, BW: %d, MCS:%d", rate->eDataTxMode, rate->u1Bw, rate->u1Mcs);
	DBG_TXCMD_LOG("NSS: %d, GI: %d, LTF: %d", rate->u1Nss, rate->u1GI, rate->u1HeLtf);
	DBG_TXCMD_LOG("ECC: %d, STBC: %d", rate->u1Ecc, rate->u1Stbc);
}

static VOID txcmdsu_dbg_txdata_dump(struct _TXM_DBG_TXCMD_SU_TXDATA_REC *txdata)
{
	DBG_TXCMD_LOG("ACQ          : %d", txdata->ucAcQ);
	DBG_TXCMD_LOG("WCID         : %d", le2cpu16(txdata->u2Wcid));
	DBG_TXCMD_LOG("RA Type      : %d", le2cpu32(txdata->eRateType));
	DBG_TXCMD_LOG("Duration     : %d", le2cpu32(txdata->u4Duration));
	DBG_TXCMD_LOG("Protect      : %d", txdata->fgProtect);
	DBG_TXCMD_LOG("Protect Type : %d", le2cpu32(txdata->eProtectType));
	txcmdsu_dbg_rate_dump(&txdata->rDataRate);
}

static VOID txcmdsu_dbg_trig_dump(struct _TXM_DBG_TXCMD_SU_TRIG_REC *trig)
{
	DBG_TXCMD_LOG("RateType    : %d", le2cpu32(trig->eRateType));
	DBG_TXCMD_LOG("TrigType    : %d", le2cpu32(trig->eTrigType));
	DBG_TXCMD_LOG("Protect     : %d", trig->fgProtect);
	DBG_TXCMD_LOG("ProtectType : %d", le2cpu32(trig->eProtectType));
	DBG_TXCMD_LOG("TrigEn      : %d", trig->fgTrigEn);
	DBG_TXCMD_LOG("Length      : %d", le2cpu32(trig->u4Length));
	DBG_TXCMD_LOG("RxTotQLen   : %d", le2cpu32(trig->u4RxTotQLen));
	DBG_TXCMD_LOG("TimerCnt    : %d", le2cpu32(trig->u4TimerCnt));
	DBG_TXCMD_LOG("CS          : %d", trig->fgCs);
	DBG_TXCMD_LOG("MU Spacing  : %d", trig->u1MuSpacingFactor);
	DBG_TXCMD_LOG("Duration    : %d", trig->u2Duration);
	DBG_TXCMD_LOG("u1TxInternal: %d", trig->u1TxInternal);
	txcmdsu_dbg_rate_dump(&trig->rRate);
}

static VOID txcmdsu_dbg_pfm_rec_dump(struct _PFM_REC *rec)
{
	UINT32 total_tick;

	if (rec->fUse == TRUE) {
		total_tick = rec->u4Tick*rec->u4Cnt;
		printk("Tick: %8d Cnt: %6d Total: %10d Name: 0x%08x\n",
		rec->u4Tick, rec->u4Cnt, total_tick, rec->prFun);
	}
}

static VOID txcmdsu_dbg_pfm_dump(VOID *ptr, UINT32 addr)
{
	UINT32 idx = 0;
	struct _PFM_CTRL *ctrl = ptr;
	struct _PFM_REC *rec;

	for (idx = 0 ; idx < PFM_MAX_REC; idx++) {
		rec = &ctrl->rPfmRec[idx];
		if (addr == 0)
			txcmdsu_dbg_pfm_rec_dump(rec);
		else if (rec->prFun == addr)
			txcmdsu_dbg_pfm_rec_dump(rec);
	}
}

static VOID txcmdsu_dbg_protect_dump(struct _TXM_DBG_TXCMD_SU_PROTECT_REC *protect)
{
	DBG_TXCMD_LOG("RateType    : %d", protect->eRateType);
	txcmdsu_dbg_rate_dump(&protect->rRate);
}

static INT txcmdsu_dbg_txctrl_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dbg_wmcu_request req;
	UCHAR *respon = NULL;
	struct _DBG_TXCMDSU_CMD_GET_CTRL ctrl;
	INT32 recv;
	UINT32 type = 0;
	UINT32 resp_size = 0;
	UINT32 addr = 0;

	DBG_TXCMD_LOG("txcmd txctrl:\n");
	DBG_TXCMD_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%x", &type, &addr);

	if (recv < 1) {
		DBG_TXCMD_LOG("Invalid cmd input! input: %d", recv);
		DBG_TXCMD_LOG("iwpriv ra0 set dbg_txcmd=txcmdsu-4-[Type]");
		DBG_TXCMD_LOG("[Type]0: TXDATA, 1: Protect, 2: Trigger");
	}

	ctrl.eType = type;

	os_alloc_mem_suspend(ad, &respon, 1600);
	if (!respon) {
		DBG_TXCMD_LOG("allocate respon memory fail\n");
		return DBG_TXCMD_STATUS_FAIL;
	}
	os_zero_mem(respon, 1600);
	switch (type) {
	case TXCMD_SU_CFG_TXDATA:
		resp_size = sizeof(struct _TXM_DBG_TXCMD_SU_TXDATA_REC);
	break;
	case TXCMD_SU_CFG_PROTECT:
		resp_size = sizeof(struct _TXM_DBG_TXCMD_SU_PROTECT_REC);
	break;
	case TXCMD_SU_CFG_TRIG:
		resp_size = sizeof(struct _TXM_DBG_TXCMD_SU_TRIG_REC);
	break;
	case TXCMD_SU_CFG_GLOBAL:
		resp_size = sizeof(struct _PFM_CTRL);
	}
	DBG_TXCMD_LOG("response size:%d", resp_size);

	req.feature_id = ENUM_DBG_TXCMDSU;
	req.type = ENUM_DBG_TXCMD_TXCTRL;
	req.len = sizeof(ctrl);
	req.payload = (UCHAR *) &ctrl;
	req.resp_handle = NULL;
	req.resp_len = resp_size;
	req.resp = respon;
	if (dbg_ut_wmcu_send(ad, &req) != DBG_TXCMD_STATUS_OK) {
		DBG_TXCMD_LOG("result,fail");
	}

	switch (type) {
	case TXCMD_SU_CFG_TXDATA:
		txcmdsu_dbg_txdata_dump((struct _TXM_DBG_TXCMD_SU_TXDATA_REC *)respon);
	break;
	case TXCMD_SU_CFG_PROTECT:
		txcmdsu_dbg_protect_dump((struct _TXM_DBG_TXCMD_SU_PROTECT_REC *)respon);
	break;
	case TXCMD_SU_CFG_TRIG:
		txcmdsu_dbg_trig_dump((struct _TXM_DBG_TXCMD_SU_TRIG_REC *)respon);
	break;
	case TXCMD_SU_CFG_GLOBAL:
		txcmdsu_dbg_pfm_dump(respon, addr);
	break;
	}
	if (respon)
		os_free_mem(respon);
	return DBG_TXCMD_STATUS_OK;
}

static dbg_txcmd_fun txcmdsu_table[] = {
	txcmdsu_dbg_manual_mode_tx,
	txcmdsu_dbg_configure,
	txcmdsu_dbg_state_get,
	txcmdsu_dbg_txctrl_get,
};

static struct dbg_txcmd_feature_entry txcmdsu_dbg = {
	.feature_name = "txcmdsu",
	.dbg_txcmd_cnt = sizeof(txcmdsu_table)/sizeof(dbg_txcmd_fun),
	.dbg_txcmd_table = txcmdsu_table,
};

VOID txcmdsu_dbg_init(struct dbg_txcmd_framework *dbg_txcmd_ctrl)
{
	dbg_txcmd_feature_register(dbg_txcmd_ctrl, &txcmdsu_dbg);
	DBG_TXCMD_LOG("ok, TX CMD SU init done, ok");
}

