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
#include "txcmdsu_dvt.h"
#include "framework_dvt.h"

#define DVT_MODNAME "TXCMDSU"

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

static INT txcmdsu_dvt_rate(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;
	struct _DVT_TXCMDSU_CMD_RATE cmd;
	INT32 recv;
	UINT32 txmode = 0, rate = 0, gi = 0, bw = 0, nss = 0, heltf = 0, ecc = 0, stbc = 0;
	UINT32 type = 0;

	DVT_LOG("Rate configure :");
	DVT_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d", &type, &txmode, &bw, &rate, &nss, &gi, &heltf, &ecc, &stbc);
	cmd.eType = type;
	cmd.ucTxMode = txmode;
	cmd.ucBw = bw;
	cmd.ucRate = rate;
	cmd.ucNss = nss;
	cmd.ucGi = gi;
	cmd.ucHeLtf = heltf;
	cmd.ucEcc = ecc;
	cmd.ucStbc = stbc;


	if (recv != 8) {
		DVT_LOG("Invalid input! input:%d", recv);
		DVT_LOG("iwpriv ra0 set dvt=txcmdsu-1-[Type]-[TxMode]-[BW]-[RATE]-[Nss]-[GI]-[HeLTF]-[ECC]-[STBC]");
		DVT_LOG("[Type]0=TXDATA, 1=Protect, 2=Trigger");
		DVT_LOG("[TxMode]CCK=0, OFDM=1, HT=2, GF=3, VHT=4, HE_SU=8, HE_EXT_SU=9, HE_TRIG=10");
		DVT_LOG("[BW]BW20=0, BW40=1, BW80=2,BW160=3");
		DVT_LOG("[Rate]CCK=0~4, OFDM=0~7, HT=0~32, VHT=0~9");
		DVT_LOG("[GI]HE: 0=0.8us, 1=1.6us, 2=3.2us HT/VHT: 0=0.4us, 1=0.8us");
		DVT_LOG("[HeLTF] 0=1xLTF, 1=2xLTF, 2=4xLTF");
		DVT_LOG("[Nss]Nss= 0, 1, 2, 3");
		DVT_LOG("[ECC]0=BCC, 1=LDPC");
		DVT_LOG("[STBC]0=No STBC, 1=STBC");
		return DVT_STATUS_FAIL;
	}

	DVT_LOG("%s():Type= %d, TxMode: %d, Rate: %d, GI: %d, BW: %d\n"
			"\t\tNss: %d, HeLTF:%d, ECC: %d, STBC: %d",
			__func__, type, txmode, rate, gi, bw, nss, heltf, ecc, stbc);

	req.feature_id = ENUM_SYSDVT_TXCMDSU;
	req.type = ENUM_SYSDVT_TXCMDSU_RATE;
	req.len = sizeof(cmd);
	req.payload = (UCHAR *) &cmd;
	req.resp_handle = NULL;
	req.resp_len = 0;
	req.resp = NULL;
	if (dvt_ut_wmcu_send(ad, &req) != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
	}

	return DVT_STATUS_OK;
}
static INT txcmdsu_dvt_configure(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;
	struct _DVT_TXCMDSU_CMD_CFG cmd;

	INT32 recv;
	UINT32 type = 0, val_type = 0, val = 0;

	DVT_LOG("SU Configure:");
	DVT_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%d-%d", &type, &val_type, &val);

	cmd.eType = type;
	cmd.eValType = val_type;
	cmd.u4Val = val;

	if (recv != 3) {
		DVT_LOG("Invalid cmd input! input: %d", recv);
		DVT_LOG("iwpriv ra0 set dvt=txcmdsu-2-[Type][ValType][Val]");
		DVT_LOG("[Type]0: TXDATA, 1: Protect, 2: Trigger");
		DVT_LOG("[ValType]");
		DVT_LOG("\t%d:TXCMD_SU_VAL_PROTECT_EN (Type: TRIG/TXDATA)", TXCMD_SU_VAL_PROTECT_EN);
		DVT_LOG("\t%d:TXCMD_SU_VAL_PROTECT_TYPE (Type: TRIG/TXDATA)", TXCMD_SU_VAL_PROTECT_TYPE);
		DVT_LOG("\t%d:TXCMD_SU_VAL_RATE_TYPE (Type: ALL)", TXCMD_SU_VAL_RATE_TYPE);
		DVT_LOG("\t%d:TXCMD_SU_VAL_DURATION (Type: TXDATA/TRIG)", TXCMD_SU_VAL_DURATION);
		DVT_LOG("\t%d:TXCMD_SU_VAL_LSIG (Type: TRIG)", TXCMD_SU_VAL_LSIG);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_EN (Type: TRIG)", TXCMD_SU_VAL_TRIG_EN);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_TYPE (Type: TRIG)", TXCMD_SU_VAL_TRIG_TYPE);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_INTERVAL (Type: TRIG)", TXCMD_SU_VAL_TRIG_INTERVAL);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_MUSPCFAC (Type: TRIG)", TXCMD_SU_VAL_TRIG_MUSPCFAC);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_CS (Type: TRIG)", TXCMD_SU_VAL_TRIG_CS);
		DVT_LOG("\t%d:TXCMD_SU_VAL_SPL (Type: TRIG/TXDATA)", TXCMD_SU_VAL_SPL);
		DVT_LOG("\t%d:TXCMD_SU_VAL_BW (Type: TRIG/TXDATA)", TXCMD_SU_VAL_BW);
		DVT_LOG("\t%d:TXCMD_SU_VAL_MCS (Type: TRIG/TXDATA)", TXCMD_SU_VAL_MCS);
		DVT_LOG("\t%d:TXCMD_SU_VAL_GI (Type: TRIG/TXDATA)", TXCMD_SU_VAL_GI);
		DVT_LOG("\t%d:TXCMD_SU_VAL_LTF (Type: TRIG/TXDATA)", TXCMD_SU_VAL_LTF);
		DVT_LOG("\t%d:TXCMD_SU_VAL_COD (Type: TRIG/TXDATA)", TXCMD_SU_VAL_COD);
		DVT_LOG("\t%d:TXCMD_SU_VAL_TRIG_TX_INTERNAL (Type: TRIG)", TXCMD_SU_VAL_TRIG_TX_INTERNAL);
		return DVT_STATUS_FAIL;
	}
	DVT_LOG("%s():\t Type: %d, VType: %d, Val: %d",
			__func__, type, val_type, val);

	req.feature_id = ENUM_SYSDVT_TXCMDSU;
	req.type = ENUM_SYSDVT_TXCMDSU_CFG;
	req.len = sizeof(cmd);
	req.payload = (UCHAR *) &cmd;
	req.resp_handle = NULL;
	req.resp_len = 0;
	req.resp = NULL;
	if (dvt_ut_wmcu_send(ad, &req) != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
	}

	return DVT_STATUS_OK;
}

static VOID txcmdsu_dvt_state_dump(struct dvt_txcmdsu_statistic *state)
{
	UCHAR idx = 0;
	struct HE_ACTRL_OMI *om;
	DVT_LOG("TX CMD Setting:");
	DVT_LOG("SPL EN    : %d", state->spl_en);

	for (idx = 0 ; idx < DVT_TXCMD_WMM_SET; idx++) {
		DVT_LOG("TXCMD MODE WMM(%d): %d",
		idx,
		state->txcmd_mode[idx]);
	}

	state->omi = le2cpu16(state->omi);
	om = (struct HE_ACTRL_OMI *) &state->omi;

	DVT_LOG("=============================TX CTRL Status===================================");
	DVT_LOG("TX CTRL Status:");
	DVT_LOG("FreeQ Len     : %d", state->swq_len[0]);
	DVT_LOG("TxQ Len       : %d", state->swq_len[1]);
	DVT_LOG("TxDoneQ Len   : %d", state->swq_len[2]);
	DVT_LOG("ACQLen   :");

	for (idx = 0 ; idx < 4; idx++) {

		UCHAR start_idx = idx * 4;
		DVT_LOG("WMMQ(%d): %d, %d, %d, %d",
		idx,
		state->acq_len[0 + start_idx],
		state->acq_len[1 + start_idx],
		state->acq_len[2 + start_idx],
		state->acq_len[3 + start_idx]);
	}

	DVT_LOG("ALTX: %d,TF: %d,TWT_TSF_TF: %d,TWT_DL: %d,TWT_UL: %d",
		state->acq_len[WH_TXC_ALTX0],
		state->acq_len[WH_TXC_TF0],
		state->acq_len[WH_TXC_TWT_TSF_TF0],
		state->acq_len[WH_TXC_TWT_DL0],
		state->acq_len[WH_TXC_TWT_UL0]);

	DVT_LOG("=============================TX-Statistic=====================================");
	DVT_LOG("MU TX-DATA    : %d", state->tx_cnt[WH_TXCMD_MU_TX_DATA]);
	DVT_LOG("SU TX-DATA    : %d", state->tx_cnt[WH_TXCMD_SU_TX_DATA]);
	DVT_LOG("NON HE TX-DATA: %d", state->tx_cnt[WH_TXCMD_NONE_HE_TX_DATA]);
	DVT_LOG("MU TRIG-DATA  : %d", state->tx_cnt[WH_TXCMD_MU_TRIG_DATA]);
	DVT_LOG("SU TRIG-DATA  : %d", state->tx_cnt[WH_TXCMD_SU_TRIG_DATA]);
	DVT_LOG("HE Sounding   : %d", state->tx_cnt[WH_TXCMD_HE_SOUNDING]);
	DVT_LOG("SW PKT        : %d", state->tx_cnt[WH_TXCMD_SW_PKT]);
	DVT_LOG("=============================TX-Error=====================================");
	DVT_LOG("MU TX-DATA    : %d", state->tx_err_cnt[WH_TXCMD_MU_TX_DATA]);
	DVT_LOG("SU TX-DATA    : %d", state->tx_err_cnt[WH_TXCMD_SU_TX_DATA]);
	DVT_LOG("NON HE TX-DATA: %d", state->tx_err_cnt[WH_TXCMD_NONE_HE_TX_DATA]);
	DVT_LOG("MU TRIG-DATA  : %d", state->tx_err_cnt[WH_TXCMD_MU_TRIG_DATA]);
	DVT_LOG("SU TRIG-DATA  : %d", state->tx_err_cnt[WH_TXCMD_SU_TRIG_DATA]);
	DVT_LOG("HE Sounding   : %d", state->tx_err_cnt[WH_TXCMD_HE_SOUNDING]);
	DVT_LOG("SW PKT        : %d", state->tx_err_cnt[WH_TXCMD_SW_PKT]);
	DVT_LOG("=============================RX-Statistic=====================================");
	DVT_LOG("RX-DATA Report: %d", state->rx_cnt[CMDRPT_TYPE_RXRPT]);
	DVT_LOG("TX-DATA Report: %d", state->rx_cnt[CMDRPT_TYPE_TXDATA]);
	DVT_LOG("TX-TRIG Report: %d", state->rx_cnt[CMDRPT_TYPE_TXTRIG]);
	DVT_LOG("SPL Report    : %d", state->rx_cnt[CMDRPT_TYPE_SPL]);
	DVT_LOG("============================SPL Statistic=====================================");
	DVT_LOG("TX DONE     : %d", state->spl_cnt[WH_SPL_TXCMD_DONE_TX_MODE]);
	DVT_LOG("RX DONE     : %d", state->spl_cnt[WH_SPL_TXCMD_DONE_RX_MODE]);
	DVT_LOG("TWT DL DONE : %d", state->spl_cnt[WH_SPL_TXCMD_DONE_TWT_DL_MODE]);
	DVT_LOG("TWT UL DONE : %d", state->spl_cnt[WH_SPL_TXCMD_DONE_TWT_UL_MODE]);
	DVT_LOG("IO TX       : %d", state->spl_cnt[WH_SPL_IO_TX_MODE]);
	DVT_LOG("IO RX       : %d", state->spl_cnt[WH_SPL_IO_RX_MODE]);
	DVT_LOG("NON EMPTY TX: %d", state->spl_cnt[WH_SPL_CHNL_NONEMPTY_TX_MODE]);
	DVT_LOG("NON EMPTY RX: %d", state->spl_cnt[WH_SPL_CHNL_NONEMPTY_RX_MODE]);
	DVT_LOG("============================OM=====================================");
	DVT_LOG("OMI BW      : %d", om->bw);
	DVT_LOG("OMI NSS     : %d", om->tx_nss);
	DVT_LOG("OMI MU      : %d", om->ul_mu_disable);
}

static INT txcmdsu_dvt_state_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;
	struct dvt_txcmdsu_statistic respon;


	DVT_LOG("txcmd statistic:\n");
	DVT_LOG("arg,%s", arg);

	req.feature_id = ENUM_SYSDVT_TXCMDSU;
	req.type = ENUM_SYSDVT_TXCMD_STAT;
	req.len = 0;
	req.payload = NULL;
	req.resp_handle = NULL;
	req.resp_len = sizeof(respon);
	req.resp = (UCHAR *) &respon;
	if (dvt_ut_wmcu_send(ad, &req) != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
	}
	txcmdsu_dvt_state_dump(&respon);
	return DVT_STATUS_OK;
}

static VOID txcmdsu_rate_dump(struct _RATE_INFO_RSP_T *rate)
{
	DVT_LOG("TXMODE: %d, BW: %d, MCS:%d", rate->eDataTxMode, rate->u1Bw, rate->u1Mcs);
	DVT_LOG("NSS: %d, GI: %d, LTF: %d", rate->u1Nss, rate->u1GI, rate->u1HeLtf);
	DVT_LOG("ECC: %d, STBC: %d", rate->u1Ecc, rate->u1Stbc);
}

static VOID txcmdsu_dvt_txdata_dump(struct _TXM_TXCMD_SU_TXDATA_REC *txdata)
{
	DVT_LOG("ACQ          : %d", txdata->ucAcQ);
	DVT_LOG("WCID         : %d", le2cpu16(txdata->u2Wcid));
	DVT_LOG("RA Type      : %d", le2cpu32(txdata->eRateType));
	DVT_LOG("Duration     : %d", le2cpu32(txdata->u4Duration));
	DVT_LOG("Protect      : %d", txdata->fgProtect);
	DVT_LOG("Protect Type : %d", le2cpu32(txdata->eProtectType));
	txcmdsu_rate_dump(&txdata->rDataRate);
}

static VOID txcmdsu_dvt_trig_dump(struct _TXM_TXCMD_SU_TRIG_REC *trig)
{
	DVT_LOG("RateType    : %d", le2cpu32(trig->eRateType));
	DVT_LOG("TrigType    : %d", le2cpu32(trig->eTrigType));
	DVT_LOG("Protect     : %d", trig->fgProtect);
	DVT_LOG("ProtectType : %d", le2cpu32(trig->eProtectType));
	DVT_LOG("TrigEn      : %d", trig->fgTrigEn);
	DVT_LOG("Length      : %d", le2cpu32(trig->u4Length));
	DVT_LOG("RxTotQLen   : %d", le2cpu32(trig->u4RxTotQLen));
	DVT_LOG("TimerCnt    : %d", le2cpu32(trig->u4TimerCnt));
	DVT_LOG("CS          : %d", trig->fgCs);
	DVT_LOG("MU Spacing  : %d", trig->u1MuSpacingFactor);
	DVT_LOG("Duration    : %d", trig->u2Duration);
	DVT_LOG("u1TxInternal: %d", trig->u1TxInternal);
	txcmdsu_rate_dump(&trig->rRate);
}

static VOID txcmdsu_dvt_pfm_rec_dump(struct _PFM_REC *rec)
{
	UINT32 total_tick;

	if (rec->fUse == TRUE) {
		total_tick = rec->u4Tick*rec->u4Cnt;
		printk("Tick: %8d Cnt: %6d Total: %10d Name: 0x%08x\n",
		rec->u4Tick, rec->u4Cnt, total_tick, rec->prFun);
	}
}

static VOID txcmdsu_dvt_pfm_dump(VOID *ptr, UINT32 addr)
{
	UINT32 idx = 0;
	struct _PFM_CTRL *ctrl = ptr;
	struct _PFM_REC *rec;

	for (idx = 0 ; idx < PFM_MAX_REC; idx++) {
		rec = &ctrl->rPfmRec[idx];
		if (addr == 0)
			txcmdsu_dvt_pfm_rec_dump(rec);
		else if (rec->prFun == addr)
			txcmdsu_dvt_pfm_rec_dump(rec);
	}
}


static VOID txcmdsu_dvt_protect_dump(struct _TXM_TXCMD_SU_PROTECT_REC *protect)
{
	DVT_LOG("RateType    : %d", protect->eRateType);
	txcmdsu_rate_dump(&protect->rRate);
}

static INT txcmdsu_dvt_txctrl_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct dvt_wmcu_request req;
	UCHAR respon[1600];
	struct _DVT_TXCMDSU_CMD_GET_CTRL ctrl;
	INT32 recv;
	UINT32 type = 0;
	UINT32 resp_size = 0;
	UINT32 addr = 0;

	DVT_LOG("txcmd txctrl:\n");
	DVT_LOG("arg,%s", arg);

	recv = sscanf(arg, "%d-%x", &type, &addr);

	if (recv < 1) {
		DVT_LOG("Invalid cmd input! input: %d", recv);
		DVT_LOG("iwpriv ra0 set dvt=txcmdsu-4-[Type]");
		DVT_LOG("[Type]0: TXDATA, 1: Protect, 2: Trigger");
	}

	ctrl.eType = type;

	os_zero_mem(&respon, sizeof(respon));
	switch (type) {
	case TXCMD_SU_CFG_TXDATA:
		resp_size = sizeof(struct _TXM_TXCMD_SU_TXDATA_REC);
	break;
	case TXCMD_SU_CFG_PROTECT:
		resp_size = sizeof(struct _TXM_TXCMD_SU_PROTECT_REC);
	break;
	case TXCMD_SU_CFG_TRIG:
		resp_size = sizeof(struct _TXM_TXCMD_SU_TRIG_REC);
	break;
	case TXCMD_SU_CFG_GLOBAL:
		resp_size = sizeof(struct _PFM_CTRL);
	}
	DVT_LOG("response size:%d", resp_size);

	req.feature_id = ENUM_SYSDVT_TXCMDSU;
	req.type = ENUM_SYSDVT_TXCMD_TXCTRL;
	req.len = sizeof(ctrl);
	req.payload = (UCHAR *)&ctrl;
	req.resp_handle = NULL;
	req.resp_len = resp_size;
	req.resp = (UCHAR *) respon;
	if (dvt_ut_wmcu_send(ad, &req) != DVT_STATUS_OK) {
		DVT_LOG("result,fail");
	}

	switch (type) {
	case TXCMD_SU_CFG_TXDATA:
		txcmdsu_dvt_txdata_dump((struct _TXM_TXCMD_SU_TXDATA_REC *) &respon);
	break;
	case TXCMD_SU_CFG_PROTECT:
		txcmdsu_dvt_protect_dump((struct _TXM_TXCMD_SU_PROTECT_REC *) &respon);
	break;
	case TXCMD_SU_CFG_TRIG:
		txcmdsu_dvt_trig_dump((struct _TXM_TXCMD_SU_TRIG_REC *) &respon);
	break;
	case TXCMD_SU_CFG_GLOBAL:
		txcmdsu_dvt_pfm_dump(&respon, addr);
	break;
	}
	return DVT_STATUS_OK;

}



static dvt_fun txcmdsu_table[] = {
	txcmdsu_dvt_rate,
	txcmdsu_dvt_configure,
	txcmdsu_dvt_state_get,
	txcmdsu_dvt_txctrl_get,
};

static struct dvt_feature_entry txcmdsu_dvt = {
	.feature_name = "txcmdsu",
	.dvt_cnt = sizeof(txcmdsu_table)/sizeof(dvt_fun),
	.dvt_table = txcmdsu_table,
};

VOID txcmdsu_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &txcmdsu_dvt);
	DVT_LOG("ok, TX CMD SU init done, ok");
}
