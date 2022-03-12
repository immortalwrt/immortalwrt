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
*/

#include "rt_config.h"
#include "chip/mt7915_cr.h"
#include "hdev/hdev.h"

#define CR_NUM_OF_AC 9
#define ALL_CR_NUM_OF_ALL_AC (CR_NUM_OF_AC * 4)

static EMPTY_QUEUE_INFO_T ple_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 4~7 not defined */
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0}, /* Q16 */
	{"BMC Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0},
	{"BCN Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1},
	{"BMC Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1},
	{"BCN Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1},
	{"NAF Q",   ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NAF},
	{"NBCN Q",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NBCN},
	{NULL, 0, 0}, {NULL, 0, 0}, /* 18, 19 not defined */
	{"FIXFID Q", ENUM_UMAC_LMAC_PORT_2, 0x1a},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 21~29 not defined */
	{"RLS Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7e},
	{"RLS2 Q",  ENUM_PLE_CTRL_PSE_PORT_3, 0x7f}
};

static EMPTY_QUEUE_INFO_T ple_txcmd_queue_empty_info[] = {
	{"AC00Q", ENUM_UMAC_LMAC_PORT_2, 0x40},
	{"AC01Q", ENUM_UMAC_LMAC_PORT_2, 0x41},
	{"AC02Q", ENUM_UMAC_LMAC_PORT_2, 0x42},
	{"AC03Q", ENUM_UMAC_LMAC_PORT_2, 0x43},
	{"AC10Q", ENUM_UMAC_LMAC_PORT_2, 0x44},
	{"AC11Q", ENUM_UMAC_LMAC_PORT_2, 0x45},
	{"AC12Q", ENUM_UMAC_LMAC_PORT_2, 0x46},
	{"AC13Q", ENUM_UMAC_LMAC_PORT_2, 0x47},
	{"AC20Q", ENUM_UMAC_LMAC_PORT_2, 0x48},
	{"AC21Q", ENUM_UMAC_LMAC_PORT_2, 0x49},
	{"AC22Q", ENUM_UMAC_LMAC_PORT_2, 0x4a},
	{"AC23Q", ENUM_UMAC_LMAC_PORT_2, 0x4b},
	{"AC30Q", ENUM_UMAC_LMAC_PORT_2, 0x4c},
	{"AC31Q", ENUM_UMAC_LMAC_PORT_2, 0x4d},
	{"AC32Q", ENUM_UMAC_LMAC_PORT_2, 0x4e},
	{"AC33Q", ENUM_UMAC_LMAC_PORT_2, 0x4f},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2, 0x50},
	{"TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x51},
	{"TWT TSF-TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x52},
	{"TWT DL Q0", ENUM_UMAC_LMAC_PORT_2, 0x53},
	{"TWT UL Q0", ENUM_UMAC_LMAC_PORT_2, 0x54},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2, 0x55},
	{"TF Q1", ENUM_UMAC_LMAC_PORT_2, 0x56},
	{"TWT TSF-TF Q1", ENUM_UMAC_LMAC_PORT_2, 0x57},
	{"TWT DL Q1", ENUM_UMAC_LMAC_PORT_2, 0x58},
	{"TWT UL Q1", ENUM_UMAC_LMAC_PORT_2, 0x59},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
};


static EMPTY_QUEUE_INFO_T pse_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 4~7 not defined */
	{"HIF Q0", ENUM_UMAC_HIF_PORT_0,    0}, /* Q8 */
	{"HIF Q1", ENUM_UMAC_HIF_PORT_0,    1},
	{"HIF Q2", ENUM_UMAC_HIF_PORT_0,    2},
	{"HIF Q3", ENUM_UMAC_HIF_PORT_0,    3},
	{"HIF Q4", ENUM_UMAC_HIF_PORT_0,    4},
	{"HIF Q5", ENUM_UMAC_HIF_PORT_0,    5},
	{NULL, 0, 0}, {NULL, 0, 0},  /* 14~15 not defined */
	{"LMAC Q",  ENUM_UMAC_LMAC_PORT_2,    0},
	{"MDP TX Q", ENUM_UMAC_LMAC_PORT_2, 1},
	{"MDP RX Q", ENUM_UMAC_LMAC_PORT_2, 2},
	{"SEC TX Q", ENUM_UMAC_LMAC_PORT_2, 3},
	{"SEC RX Q", ENUM_UMAC_LMAC_PORT_2, 4},
	{"SFD_PARK Q", ENUM_UMAC_LMAC_PORT_2, 5},
	{"MDP_TXIOC Q", ENUM_UMAC_LMAC_PORT_2, 6},
	{"MDP_RXIOC Q", ENUM_UMAC_LMAC_PORT_2, 7},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 24~30 not defined */
	{"RLS Q",  ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F}
};

static PCHAR sta_ctrl_reg[] = {"ENABLE", "DISABLE", "PAUSE"};

#ifdef CONFIG_ATE
static UINT8 ltf_sym_code[] = {
	0, 0, 1, 2, 2, 3, 3, 4, 4	/* SS 1~8 */
};
#endif

static UINT32 tbrx_phy_ctrl[2][3] = {
	{0x83082110, 0x83082114, 0x83082118},
	{0x83082018, 0x8308201c, 0x83082020}
};

static INT32 chip_show_tmac_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 Value = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX Stream = %d\n", GET_TMAC_TCR_TX_STREAM_NUM(Value) + 1));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX RIFS Enable = %d\n", GET_TX_RIFS_EN(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RX RIFS Mode = %d\n", GET_RX_RIFS_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP TBTT Control = %d\n", GET_TXOP_TBTT_CONTROL(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP TBTT Stop Control = %d\n", GET_TBTT_TX_STOP_CONTROL(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP Burst Stop = %d\n", GET_TXOP_BURST_STOP(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDG Mode = %d\n", GET_RDG_RA_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDG Responser Enable = %d\n", GET_RDG_RESP_EN(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Smoothing = %d\n", GET_SMOOTHING(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_PSCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save RXPE Off Time(unit 2us) = %d\n",
			 GET_APS_OFF_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save RXPE On Time(unit 2us) = %d\n", APS_ON_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save Halt Time (unit 32us) = %d\n",
			 GET_APS_HALT_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Enable = %d\n", GET_APS_EN(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC0 TXOP = 0x%x (unit: 32us)\n", GET_AC0LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC1 TXOP = 0x%x (unit: 32us)\n", GET_AC1LIMIT(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC2 TXOP = 0x%x (unit: 32us)\n", GET_AC2LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC3 TXOP = 0x%x (unit: 32us)\n", GET_AC3LIMIT(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR3, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC10 TXOP = 0x%x (unit: 32us)\n", GET_AC10LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC11 TXOP = 0x%x (unit: 32us)\n", GET_AC11LIMIT(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC12 TXOP = 0x%x (unit: 32us)\n", GET_AC12LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC13 TXOP = 0x%x (unit: 32us)\n", GET_AC13LIMIT(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SLOT Time, Band0 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SLOT Time, Band1 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value)));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ATCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Aggregation Timeout (unit: 50ns) = 0x%x\n", GET_AGG_TOUT(Value)));
	return 0;
}

static INT32 chip_show_agg_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
    RTMP_ADAPTER *pAd = ctrl->priv;
    RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
    UINT32 value = 0, idx, agg_rang_sel[15], ampdu_cnt[11] = {0}, band_offset = 0, total_ampdu = 0;
    UINT8 band_idx = 0;

	for (band_idx = 0; band_idx < pChipCap->band_cnt; band_idx++) {
		band_offset = (BN1_WF_AGG_TOP_BASE - BN0_WF_AGG_TOP_BASE) * band_idx;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band %d AGG Status\n", band_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC00 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC01 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC02 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC03 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC10 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC11 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC12 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC13 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC20 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC21 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC22 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC23 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC30 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC31 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC32 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC33 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ALTX Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_SHFT));

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR0_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize0 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize1 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize2 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize3 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR1_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize4 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize5 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize6 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize7 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR2_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize8 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize9 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WinsizeA limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WinsizeB limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR3_ADDR + band_offset, &value);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WinsizeC limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WinsizeD limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZED_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZED_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WinsizeE limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_SHFT));

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===AMPDU Related Counters===\n"));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0ARNG0_ADDR + band_offset, &value);
		agg_rang_sel[0] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_0_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_0_SHFT;
		agg_rang_sel[1] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_1_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_1_SHFT;
		agg_rang_sel[2] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_2_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_2_SHFT;
		agg_rang_sel[3] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_3_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_3_SHFT;
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0ARNG1_ADDR + band_offset, &value);
		agg_rang_sel[4] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_4_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_4_SHFT;
		agg_rang_sel[5] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_5_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_5_SHFT;
		agg_rang_sel[6] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_6_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_6_SHFT;
		agg_rang_sel[7] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_7_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_7_SHFT;
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0ARNG2_ADDR + band_offset, &value);
		agg_rang_sel[8] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_8_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_8_SHFT;
		agg_rang_sel[9] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_9_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_9_SHFT;
		agg_rang_sel[10] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_10_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_10_SHFT;
		agg_rang_sel[11] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_11_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_11_SHFT;
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0ARNG3_ADDR + band_offset, &value);
		agg_rang_sel[12] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_12_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_12_SHFT;
		agg_rang_sel[13] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_13_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_13_SHFT;
		agg_rang_sel[14] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_14_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_14_SHFT;

		/* Need to add 1 after read from AGG_RANG_SEL CR */
		for (idx = 0; idx < 15; idx++)
			agg_rang_sel[idx]++;

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR2_ADDR + band_offset, &ampdu_cnt[3]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR3_ADDR + band_offset, &ampdu_cnt[4]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR4_ADDR + band_offset, &ampdu_cnt[5]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR5_ADDR + band_offset, &ampdu_cnt[6]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR13_ADDR + band_offset, &ampdu_cnt[7]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR14_ADDR + band_offset, &ampdu_cnt[8]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR15_ADDR + band_offset, &ampdu_cnt[9]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR16_ADDR + band_offset, &ampdu_cnt[10]);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tTx Agg Range: \t%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d\n",
				 agg_rang_sel[0],
				 agg_rang_sel[0] + 1, agg_rang_sel[1],
				 agg_rang_sel[1] + 1, agg_rang_sel[2],
				 agg_rang_sel[2] + 1, agg_rang_sel[3],
				 agg_rang_sel[3] + 1, agg_rang_sel[4],
				 agg_rang_sel[4] + 1, agg_rang_sel[5],
				 agg_rang_sel[5] + 1, agg_rang_sel[6],
				 agg_rang_sel[6] + 1, agg_rang_sel[7]));

#define BIT_0_to_15_MASK 0x0000FFFF
#define BIT_15_to_31_MASK 0xFFFF0000
#define SHFIT_16_BIT 16

		for (idx = 3; idx < 11; idx++)
			total_ampdu = total_ampdu + (ampdu_cnt[idx] & BIT_0_to_15_MASK) + ((ampdu_cnt[idx] & BIT_15_to_31_MASK) >> SHFIT_16_BIT);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK,
				 (ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT,
				 (ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK,
				 (ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT,
				 (ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK,
				 (ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT,
				 (ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK,
				 (ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT));

		if (total_ampdu != 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu));
			}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\t\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~256\n",
				 agg_rang_sel[7] + 1, agg_rang_sel[8],
				 agg_rang_sel[8] + 1, agg_rang_sel[9],
				 agg_rang_sel[9] + 1, agg_rang_sel[10],
				 agg_rang_sel[10] + 1, agg_rang_sel[11],
				 agg_rang_sel[11] + 1, agg_rang_sel[12],
				 agg_rang_sel[12] + 1, agg_rang_sel[13],
				 agg_rang_sel[13] + 1, agg_rang_sel[14],
				 agg_rang_sel[14] + 1));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE8_CNT_MASK,
				 (ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_SHFT,
				 (ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE10_CNT_MASK,
				 (ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_SHFT,
				 (ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE12_CNT_MASK,
				 (ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_SHFT,
				 (ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE14_CNT_MASK,
				 (ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_SHFT));

		if (total_ampdu != 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu));
			}
	}

	return 0;
}

static INT32 chip_dump_mib_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
#define BSS_NUM 4
	RTMP_ADAPTER *pAd = ctrl->priv;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 bss_nums = BSS_NUM;
	UINT32 mac_val0 = 0, mac_val = 0, mac_val1 = 0, idx, band_idx = 0, band_offset = 0;
	UINT32 msdr6 = 0, msdr7 = 0, msdr8 = 0, msdr9 = 0, msdr10 = 0, msdr16 = 0;
	UINT32 msdr17 = 0, msdr18 = 0, msdr19 = 0, msdr20 = 0, msdr21 = 0;
	UINT32 mbxsdr[BSS_NUM][4];
	UINT32 mbtcr[16] = {0}, mbtbcr[16] = {0}, mbrcr[16] = {0}, mbrbcr[16] = {0};
	UINT32 btcr[BSS_NUM] = {0}, btbcr[BSS_NUM] = {0}, brcr[BSS_NUM] = {0};
	UINT32 brbcr[BSS_NUM] = {0}, btdcr[BSS_NUM] = {0}, brdcr[BSS_NUM] = {0};
	UINT32 mu_cnt[5] = {0};
	UINT32 ampdu_cnt[3] = {0};
	ULONG per;

	for (band_idx = 0; band_idx < pChipCap->band_cnt; band_idx++) {
#ifdef CONFIG_AP_SUPPORT
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo[band_idx];
#endif
		if (arg != NULL && band_idx != simple_strtoul(arg, 0, 10))
			continue;

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band %d MIB Status\n", band_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SCR0_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MIB Status Control=0x%x\n", mac_val));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0PBSCR_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MIB Per-BSS Status Control=0x%x\n", mac_val));

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR6_ADDR + band_offset, &msdr6);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR7_ADDR + band_offset, &msdr7);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR8_ADDR + band_offset, &msdr8);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR9_ADDR + band_offset, &msdr9);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR10_ADDR + band_offset, &msdr10);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR16_ADDR + band_offset, &msdr16);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR17_ADDR + band_offset, &msdr17);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR18_ADDR + band_offset, &msdr18);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR19_ADDR + band_offset, &msdr19);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR20_ADDR + band_offset, &msdr20);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR21_ADDR + band_offset, &msdr21);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR12_ADDR + band_offset, &ampdu_cnt[0]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR14_ADDR + band_offset, &ampdu_cnt[1]);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR15_ADDR + band_offset, &ampdu_cnt[2]);
		ampdu_cnt[1] &= BN0_WF_MIB_TOP_M0SDR14_AMPDU_MPDU_COUNT_MASK;
		ampdu_cnt[2] &= BN0_WF_MIB_TOP_M0SDR15_AMPDU_ACKED_COUNT_MASK;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("===Phy/Timing Related Counters===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tChannelIdleCnt=0x%x\n", msdr6 & BN0_WF_MIB_TOP_M0SDR6_CHANNEL_IDLE_COUNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRx_MDRDY_CNT=0x%x\n", msdr10 & BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tCCK_MDRDY_TIME=0x%x, OFDM_MDRDY_TIME=0x%x, OFDM_GREEN_MDRDY_TIME=0x%x\n",
				 msdr19 & BN0_WF_MIB_TOP_M0SDR19_CCK_MDRDY_TIME_MASK,
				 msdr20 & BN0_WF_MIB_TOP_M0SDR20_OFDM_LG_MIXED_VHT_MDRDY_TIME_MASK,
				 msdr21 & BN0_WF_MIB_TOP_M0SDR21_OFDM_GREEN_MDRDY_TIME_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tPrim CCA Time=0x%x\n", msdr16 & BN0_WF_MIB_TOP_M0SDR16_P_CCA_TIME_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tSec CCA Time=0x%x\n", msdr17 & BN0_WF_MIB_TOP_M0SDR17_S_CCA_TIME_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tPrim ED Time=0x%x\n", msdr18 & BN0_WF_MIB_TOP_M0SDR18_P_ED_TIME_MASK));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Tx Related Counters(Generic)===\n"));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR0_ADDR + band_offset, &mac_val);
#ifdef CONFIG_AP_SUPPORT
		pBcnCheckInfo->totalbcncnt += (mac_val & BN0_WF_MIB_TOP_M0SDR0_BEACONTXCOUNT_MASK);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tBeaconTxCnt=0x%x\n", pBcnCheckInfo->totalbcncnt));
		pBcnCheckInfo->totalbcncnt = 0;
#else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tBeaconTxCnt=0x%x\n", mac_val));
#endif
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR0_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tTx 20MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR0_TX_20MHZ_CNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tTx 40MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_SHFT));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR1_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tTx 80MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR1_TX_80MHZ_CNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tTx 160MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_SHFT));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAMPDU Cnt=0x%x\n", ampdu_cnt[0]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAMPDU MPDU Cnt=0x%x\n", ampdu_cnt[1]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAMPDU MPDU Ack Cnt=0x%x\n", ampdu_cnt[2]));
		per = (ampdu_cnt[2] == 0 ? 0 : 1000 * (ampdu_cnt[1] - ampdu_cnt[2]) / ampdu_cnt[1]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU MPDU PER=%ld.%1ld%%\n", per / 10, per % 10));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===MU Related Counters===\n"));
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR34_ADDR + band_offset, &mu_cnt[0]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR8_ADDR + band_offset, &mu_cnt[1]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR9_ADDR + band_offset, &mu_cnt[2]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR10_ADDR + band_offset, &mu_cnt[3]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR11_ADDR + band_offset, &mu_cnt[4]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tMUBF_TX_COUNT=0x%x\n", mu_cnt[0] & BN0_WF_MIB_TOP_M0SDR34_MUBF_TX_COUNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tMU_TX_MPDU_COUNT(Ok+Fail)=0x%x\n", mu_cnt[1]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tMU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[2]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tMU_TO_SU_PPDU_COUNT=0x%x\n", mu_cnt[3] & BN0_WF_MIB_TOP_M0DR10_MU_FAIL_PPDU_CNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tSU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[4]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Rx Related Counters(Generic)===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tVector Mismacth Cnt=0x%x\n", msdr7 & BN0_WF_MIB_TOP_M0SDR7_VEC_MISS_COUNT_MASK));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tDelimiter Fail Cnt=0x%x\n", msdr8 & BN0_WF_MIB_TOP_M0SDR8_DELIMITER_FAIL_COUNT_MASK));

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR3_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRxFCSErrCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_MASK)));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR4_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRxFifoFullCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR4_RX_FIFO_FULL_COUNT_MASK)));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR11_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRxLenMismatch=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR11_RX_LEN_MISMATCH_MASK)));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR5_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRxMPDUCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR5_RX_MPDU_COUNT_MASK)));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR22_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRx AMPDU Cnt=0x%x\n", mac_val));
		/* TODO: shiang-MT7615, is MIB_M0SDR23 used for Rx total byte count for all or just AMPDU only??? */
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR23_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRx Total ByteCnt=0x%x\n", mac_val));

		band_offset = WF_WTBLON_TOP_B1BTCRn_ADDR - WF_WTBLON_TOP_B0BTCRn_ADDR;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-BSS Related Tx/Rx Counters===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n"));

		for (idx = 0; idx < bss_nums; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BTCRn_ADDR + band_offset + idx * 4, &btcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BTBCRn_ADDR + band_offset + idx * 4, &btbcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BRCRn_ADDR + band_offset + idx * 4, &brcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BRBCRn_ADDR + band_offset + idx * 4, &brbcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BTDCRn_ADDR + band_offset + idx * 4, &btdcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0BRDCRn_ADDR + band_offset + idx * 4, &brdcr[idx]);
		}

		for (idx = 0; idx < bss_nums; idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
						idx, btcr[idx], btdcr[idx], btbcr[idx],
						brcr[idx], brdcr[idx], brbcr[idx]));
		}


		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-MBSS Related MIB Counters===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2/3Cnt\n"));

		for (idx = 0; idx < bss_nums; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0B0SDR0_ADDR + band_offset + idx * 0x10, &mbxsdr[idx][0]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0B0SDR1_ADDR + band_offset + idx * 0x10, &mbxsdr[idx][1]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0B0SDR2_ADDR + band_offset + idx * 0x10, &mbxsdr[idx][2]);
			RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0B0SDR3_ADDR + band_offset + idx * 0x10, &mbxsdr[idx][3]);
		}

		for (idx = 0; idx < bss_nums; idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x/0x%x\n",
						idx, (mbxsdr[idx][0] & BN0_WF_MIB_TOP_M0B0SDR0_RTSTXCOUNT_MASK),
						(mbxsdr[idx][0] & BN0_WF_MIB_TOP_M0B0SDR0_RTSRETRYCOUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR0_RTSRETRYCOUNT_SHFT,
						(mbxsdr[idx][1] & BN0_WF_MIB_TOP_M0B0SDR1_BAMISSCOUNT_MASK),
						(mbxsdr[idx][1] & BN0_WF_MIB_TOP_M0B0SDR1_ACKFAILCOUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR1_ACKFAILCOUNT_SHFT,
						(mbxsdr[idx][2] & BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRYCOUNT_MASK),
						(mbxsdr[idx][2] & BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRY2COUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRY2COUNT_SHFT,
						(mbxsdr[idx][3] & BN0_WF_MIB_TOP_M0B0SDR3_FRAMERETRY3COUNT_MASK)));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Dummy delimiter insertion result===\n"));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR12_ADDR + band_offset, &mac_val0);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR6_ADDR + band_offset, &mac_val);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR7_ADDR + band_offset, &mac_val1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Range0 = %d\t Range1 = %d\t Range2 = %d\t Range3 = %d\t Range4 = %d\n",
					(mac_val0 & BN0_WF_MIB_TOP_M0DR12_TX_DDLMT_RNG0_CNT_MASK),
					(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG1_CNT_MASK),
					(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_CNT_SHFT,
					(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG3_CNT_MASK),
					(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_CNT_SHFT));

		band_offset = WF_WTBLON_TOP_B1BTCRn_ADDR - WF_WTBLON_TOP_B0BTCRn_ADDR;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===Per-MBSS Related Tx/Rx Counters===\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MBSSIdx   TxCnt  TxByteCnt  RxCnt  RxByteCnt\n"));

		for (idx = 0; idx < 16; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0MBTCRn_ADDR + band_offset + idx * 4, &mbtcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0MBTBCRn_ADDR + band_offset + idx * 4, &mbtbcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0MBRCRn_ADDR + band_offset + idx * 4, &mbrcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON_TOP_B0MBRBCRn_ADDR + band_offset + idx * 4, &mbrbcr[idx]);
		}

		for (idx = 0; idx < 16; idx++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
						idx, mbtcr[idx], mbtbcr[idx], mbrcr[idx], mbrbcr[idx]));
		}
	}

#ifdef TRACELOG_TCP_PKT
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TCP RxAck = %d\t TxData = %d",
		pAd->u4TcpRxAckCnt, pAd->u4TcpTxDataCnt));
	pAd->u4TcpRxAckCnt = 0;
	pAd->u4TcpTxDataCnt = 0;
#endif /* TRACELOG_TCP_PKT */
	return TRUE;
}

static INT32 chip_show_pse_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 pse_buf_ctrl = 0, pg_sz, pg_num;
	UINT32 pse_stat = 0, pg_flow_ctrl[22] = {0};
	UINT32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail;
	UINT32 max_q, min_q, rsv_pg, used_pg;
	INT32 i;

	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PBUF_CTRL_ADDR, &pse_buf_ctrl);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_QUEUE_EMPTY_ADDR, &pse_stat);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FREEPG_HEAD_TAIL_ADDR, &pg_flow_ctrl[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_HIF0_GROUP_ADDR, &pg_flow_ctrl[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF0_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_HIF1_GROUP_ADDR, &pg_flow_ctrl[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC0_GROUP_ADDR, &pg_flow_ctrl[8]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC0_PG_INFO_ADDR, &pg_flow_ctrl[9]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC1_GROUP_ADDR, &pg_flow_ctrl[10]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC1_PG_INFO_ADDR, &pg_flow_ctrl[11]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC2_GROUP_ADDR, &pg_flow_ctrl[12]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC2_PG_INFO_ADDR, &pg_flow_ctrl[13]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_PLE_GROUP_ADDR, &pg_flow_ctrl[14]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PLE_PG_INFO_ADDR, &pg_flow_ctrl[15]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_LMAC3_GROUP_ADDR, &pg_flow_ctrl[16]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_LMAC3_PG_INFO_ADDR, &pg_flow_ctrl[17]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_MDP_GROUP_ADDR, &pg_flow_ctrl[18]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_MDP_PG_INFO_ADDR, &pg_flow_ctrl[19]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PG_PLE1_GROUP_ADDR, &pg_flow_ctrl[20]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_PLE1_PG_INFO_ADDR, &pg_flow_ctrl[21]);
	/* Configuration Info */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PSE Configuration Info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tPacket Buffer Control(0x82068014): 0x%08x\n", pse_buf_ctrl));
	pg_sz = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tPage Offset=%d(in unit of 64KB)\n",
			 (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT));
	pg_num = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tTotal page numbers=%d pages\n", pg_num));
	/* Page Flow Control */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PSE Page Flow Control:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFree page counter(0x82068100): 0x%08x\n", pg_flow_ctrl[0]));
	fpg_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe toal page number of free=0x%03x\n", fpg_cnt));
	ffa_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFree page head and tail(0x82068104): 0x%08x\n", pg_flow_ctrl[1]));
	fpg_head = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of HIF0 group(0x82068110): 0x%08x\n", pg_flow_ctrl[2]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHIF0 group page status(0x82068114): 0x%08x\n", pg_flow_ctrl[3]));
	min_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of HIF1 group(0x82068118): 0x%08x\n", pg_flow_ctrl[4]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHIF1 group page status(0x8206811c): 0x%08x\n", pg_flow_ctrl[5]));
	min_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of CPU group(0x82068150): 0x%08x\n", pg_flow_ctrl[6]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tCPU group page status(0x82068154): 0x%08x\n", pg_flow_ctrl[7]));
	min_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of LMAC0 group(0x82068170): 0x%08x\n", pg_flow_ctrl[8]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tLMAC0 group page status(0x82068174): 0x%08x\n", pg_flow_ctrl[9]));
	min_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of LMAC1 group(0x82068178): 0x%08x\n", pg_flow_ctrl[10]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tLMAC1 group page status(0x8206817c): 0x%08x\n", pg_flow_ctrl[11]));
	min_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of LMAC2 group(0x82068180): 0x%08x\n", pg_flow_ctrl[11]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tLMAC2 group page status(0x82068184): 0x%08x\n", pg_flow_ctrl[12]));
	min_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n", used_pg, rsv_pg));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of LMAC3 group(0x82068188): 0x%08x\n", pg_flow_ctrl[16]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tLMAC3 group page status(0x8206818c): 0x%08x\n", pg_flow_ctrl[17]));
	min_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of LMAC3 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of LMAC3 group=0x%03x/0x%03x\n", used_pg, rsv_pg));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of PLE group(0x82068160): 0x%08x\n", pg_flow_ctrl[14]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tPLE group page status(0x82068164): 0x%08x\n", pg_flow_ctrl[15]));
	min_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n", used_pg, rsv_pg));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tReserved page counter of PLE1 group(0x82068168): 0x%08x\n", pg_flow_ctrl[14]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tPLE1 group page status(0x8206816c): 0x%08x\n", pg_flow_ctrl[15]));
	min_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of PLE1 group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of PLE1 group=0x%03x/0x%03x\n", used_pg, rsv_pg));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of MDP group(0x82068198): 0x%08x\n", pg_flow_ctrl[18]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tMDP group page status(0x8206819c): 0x%08x\n", pg_flow_ctrl[19]));
	min_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of MDP group=0x%03x/0x%03x\n", max_q, min_q));
	rsv_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of MDP group=0x%03x/0x%03x\n", used_pg, rsv_pg));
	/* Queue Empty Status */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PSE Queue Empty Status:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tQUEUE_EMPTY(0x820680b0): 0x%08x\n", pse_stat));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tCPU Q0/1/2/3 empty=%d/%d/%d/%d\n",
			  (pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_SHFT,
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tHIF Q0/1/2/3/4/5 empty=%d/%d/%d/%d/%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tLMAC TX Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tMDP TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tSEC TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tSFD PARK Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tMDP TXIOC Q/RXIOC Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tRLS Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_SHFT)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nonempty Q info:\n"));

	for (i = 0; i < 31; i++) {
		if (((pse_stat & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (pse_queue_empty_info[i].QueueName != NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s: ", pse_queue_empty_info[i].QueueName));
				fl_que_ctrl[0] |= WF_PSE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].Portid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].Queueid << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			fl_que_ctrl[0] |= (0x1 << 31);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt));
		}
	}

	return TRUE;
}

static INT32 chip_show_protect_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" -Proetction\n"));
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR, &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  > AGG_PCR 0x%08x\n", val));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" -RTS Threshold\n"));
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR1, &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  > AGG_PCR1 0x%08x\n", val));
	return TRUE;
}

static INT32 chip_show_cca_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_DEBUG_CR, &val);
	val |= (1 << 31); /* For Band0 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_DEBUG_CR, val);
	/* Debug CR */
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_OFF_BASE + 0x2c), 0xf);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x14), 0x1f);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x18), 0x06060606);
	MAC_IO_WRITE32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x4c), 0x1c1c1d1d);
	MAC_IO_READ32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x24), &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("CCA for BAND0 info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5)));
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_DEBUG_CR, &val);
	val &= ~(1 << 31); /* For Band1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_DEBUG_CR, val);
	MAC_IO_READ32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x24), &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("CCA for BAND1 info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5)));
	return 0;
}

static INT32 chip_set_cca_en(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	BOOLEAN enable;
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	enable = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Enable CCA on Band0 SEC40: %s\n", (enable) ? "ON" : "OFF"));
	/* RF CR for BAND0 CCA */
	PHY_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHY_CCA, &val);
	val |= ((1 << 18) | (1 << 2));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("-- Force Mode: %d, Force CCA SEC40: %d [0x%08x]\n",
			  ((val & (1 << 18)) >> 18), ((val & (1 << 2)) >> 2), val));
	PHY_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHY_CCA, val);
	/* TMAC_TCR for the normal Tx BW */
	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &val);
	val &= ~(PRE_RTS_IDLE_DET_DIS);
	val |= DCH_DET_DIS;
	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, val);
	return TRUE;
}

static INT32 chip_show_dmasch_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 value = 0;
	UINT32 ple_pkt_max_sz;
	UINT32 pse_pkt_max_sz;
	UINT32 max_quota;
	UINT32 min_quota;
	UINT32 rsv_cnt;
	UINT32 src_cnt;
	UINT32 pse_rsv_cnt = 0;
	UINT32 pse_src_cnt = 0;
	UINT32 odd_group_pktin_cnt = 0;
	UINT32 odd_group_ask_cnt = 0;
	UINT32 pktin_cnt;
	UINT32 ask_cnt;
	UINT32 total_src_cnt = 0;
	UINT32 total_rsv_cnt = 0;
	UINT32 ffa_cnt;
	UINT32 free_pg_cnt;
	UINT32 Group_Mapping_Q[16] = {0};
	UINT32 qmapping_addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR;
	UINT32 status_addr = WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR;
	UINT32 quota_addr = WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR;
	UINT32 pkt_cnt_addr = WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_ADDR;
	UINT32 mapping_mask = 0xf;
	UINT32 mapping_offset = 0;
	UINT32 mapping_qidx;
	UINT32 groupidx = 0;
	UINT8 idx = 0;
	BOOLEAN pktin_int_refill_ena;
	BOOLEAN	pdma_add_int_refill_ena;
	BOOLEAN	ple_add_int_refill_ena;
	BOOLEAN	ple_sub_ena;
	BOOLEAN	hif_ask_sub_ena;
	BOOLEAN	wacpu_mode_en;
	UINT32 ple_rpg_hif;
	UINT32 ple_upg_hif;
	UINT32 pse_rpg_hif = 0;
	UINT32 pse_upg_hif = 0;
	UCHAR is_mismatch = FALSE;

	for (mapping_qidx = 0; mapping_qidx < 32; mapping_qidx++) {
		UINT32 mapping_group;

		idx = 0;

		if (mapping_qidx == 0) {
			qmapping_addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else if ((mapping_qidx % 8) == 0) {
			qmapping_addr += 0x4;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else {
			mapping_offset += 4;
			mapping_mask = 0xf << mapping_offset;
		}

		HW_IO_READ32(pAd->hdev_ctrl, qmapping_addr, &value);
		mapping_group = (value & mapping_mask) >> mapping_offset;
		Group_Mapping_Q[mapping_group] |= 1 << mapping_qidx;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dma scheduler info:\n"));
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &value);
	pktin_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PKTIN_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	pdma_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PDMA_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_SUB_ENA_MASK) ? TRUE : FALSE;
	hif_ask_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK) ? TRUE : FALSE;
	wacpu_mode_en = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_WACPU_MODE_EN_MASK) ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DMASHDL Ctrl Signal(0x5000A018): 0x%08x\n", value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\twacpu mode en(BIT0) = %d\n", wacpu_mode_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\thif_ask_sub_ena(BIT16) = %d\n", hif_ask_sub_ena));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tple_sub_ena(BIT17) = %d\n", ple_sub_ena));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tple_add_int_refill_ena(BIT29) = %d\n", ple_add_int_refill_ena));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tpdma_add_int_refill_ena(BIT30) = %d\n", pdma_add_int_refill_ena));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tpktin_int_refill(BIT31)_ena = %d\n", pktin_int_refill_ena));
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &value);
	ple_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT;
	pse_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DMASHDL Packet_max_size(0x5000A01c): 0x%08x\n", value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PLE/PSE packet max size=0x%03x/0x%03x\n",
			  ple_pkt_max_sz, pse_pkt_max_sz));
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_ERROR_FLAG_CTRL_ADDR, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DMASHDL ERR FLAG CTRL(0x5000A09c): 0x%08x\n", value));
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR, &value);
	ffa_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_SHFT;
	free_pg_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_SHFT;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DMASHDL Status_RD(0x5000A100): 0x%08x\n", value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("free page cnt = 0x%03x, ffa cnt = 0x%03x\n", free_pg_cnt, ffa_cnt));

	for (groupidx = 0; groupidx < 16; groupidx++) {
		idx = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group %d info:", groupidx));
		HW_IO_READ32(pAd->hdev_ctrl, status_addr, &value);
		rsv_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_SHFT;
		src_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_SHFT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n", groupidx, status_addr, value));
		HW_IO_READ32(pAd->hdev_ctrl, quota_addr, &value);
		max_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT;
		min_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\tDMASHDL Group%d control(0x%08x): 0x%08x\n", groupidx, quota_addr, value));

		if ((groupidx & 0x1) == 0) {
			HW_IO_READ32(pAd->hdev_ctrl, pkt_cnt_addr, &value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n", groupidx / 2, pkt_cnt_addr, value));
			odd_group_pktin_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_PKTIN_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_PKTIN_CNT_SHFT;
			odd_group_ask_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_ASK_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP1_ASK_CNT_SHFT;
			pktin_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_PKTIN_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_PKTIN_CNT_SHFT;
			ask_cnt = (value & WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_ASK_CNT_MASK)
					>> WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_GP0_ASK_CNT_SHFT;
		} else {
			pktin_cnt = odd_group_pktin_cnt;
			ask_cnt = odd_group_ask_cnt;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\trsv_cnt = 0x%03x, src_cnt = 0x%03x\n", rsv_cnt, src_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tmax/min quota = 0x%03x/ 0x%03x\n", max_quota, min_quota));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x", pktin_cnt, ask_cnt));

		if (hif_ask_sub_ena && pktin_cnt != ask_cnt) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch!"));
			is_mismatch = TRUE;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		if (groupidx == 15 && Group_Mapping_Q[groupidx] == 0) { /* Group15 is for PSE */
			pse_src_cnt = src_cnt;
			pse_rsv_cnt = rsv_cnt;
			break;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMapping Qidx:"));

		while (Group_Mapping_Q[groupidx] != 0) {
			if (Group_Mapping_Q[groupidx] & 0x1)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("Q%d ", idx));

			Group_Mapping_Q[groupidx] >>= 1;
			idx++;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		total_src_cnt += src_cnt;
		total_rsv_cnt += rsv_cnt;
		status_addr = status_addr + 4;
		quota_addr = quota_addr + 4;

		if (groupidx & 0x1)
			pkt_cnt_addr = pkt_cnt_addr + 4;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nCounter Check:\n"));
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_PG_INFO_ADDR, &value);
	ple_rpg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	ple_upg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PLE:\n\tThe used/reserved pages of PLE HIF group=0x%03x/0x%03x\n",
			  ple_upg_hif, ple_rpg_hif));
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &value);
	pse_rpg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	pse_upg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PSE:\n\tThe used/reserved pages of PSE HIF group=0x%03x/0x%03x\n",
			  pse_upg_hif, pse_rpg_hif));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DMASHDL:\n\tThe total used pages of group0~14=0x%03x",
			  total_src_cnt));

	if (ple_upg_hif != total_src_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch!"));
		is_mismatch = TRUE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tThe total reserved pages of group0~14=0x%03x\n",
			  total_rsv_cnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tThe total ffa pages of group0~14=0x%03x\n",
			  ffa_cnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tThe total free pages of group0~14=0x%03x",
			  free_pg_cnt));

	if (free_pg_cnt != total_rsv_cnt + ffa_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch(total_rsv_cnt + ffa_cnt in DMASHDL)"));
		is_mismatch = TRUE;
	}

	if (free_pg_cnt != ple_rpg_hif) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch(reserved pages in PLE)"));
		is_mismatch = TRUE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tThe used pages of group15=0x%03x", pse_src_cnt));

	if (pse_upg_hif != pse_src_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch!"));
		is_mismatch = TRUE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tThe reserved pages of group15=0x%03x", pse_rsv_cnt));

	if (pse_rpg_hif != pse_rsv_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", mismatch!"));
		is_mismatch = TRUE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	if (!is_mismatch)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DMASHDL: no counter mismatch\n"));

	return TRUE;
}

#ifdef MT7915_FPGA
static char *HW_TX_MODE[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A", "N/A", "N/A",
							"HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU", "N/A"};
static char *HW_TX_BW[] = {"20 MHz", "40 MHz", "80 Mhz", "160/80+80 MHz"};

static INT32 chip_show_txv_info(struct hdev_ctrl *ctrl, void *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 PPDUsize = 25;
	UCHAR Idx = 0, tx_mode = 0, txusercount = 0, BW = 0;
	int i = 0;
	UINT32 reg_val = 0;
	UINT32 txv[256] = {0};
	UINT32 uid[256] = {0};
	UINT32 uidcount, ppducount;

	if(arg ==NULL);
		Idx = 1;
	else
		Idx = os_str_tol(arg, 0, 10);
	/*read remainied uid read count*/
	MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_UID_STATUS_ADDR, &reg_val);
	uidcount = ((reg_val & WF_M2M_PHY_TOP_11AX_M2M_TXMON_UID_STATUS_TXMON_UID_FIFO_RPTR_MASK) >> WF_M2M_PHY_TOP_11AX_M2M_TXMON_UID_STATUS_TXMON_UID_FIFO_RPTR_SHFT);
	/*read remainied PPDU read count*/
	MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_PPDU_STATUS_ADDR, &reg_val);/*read remainied PPDU read count*/
	ppducount = ((reg_val & WF_M2M_PHY_TOP_11AX_M2M_TXMON_PPDU_STATUS_TXMON_PPDU_FIFO_RPTR_MASK) >> WF_M2M_PHY_TOP_11AX_M2M_TXMON_PPDU_STATUS_TXMON_PPDU_FIFO_RPTR_SHFT);

	/*if uid read count != ppducount, we have to pop all data to CR,and clear the FIFO.*/
	if (uidcount != ppducount) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("the total number of uid is different to PPDU"));
		return FALSE;
	}

	switch (Idx) {
	case 1:
		MAC_IO_WRITE32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_CTRL_ADDR, 0x03000019); /*enable HW TX monitor*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX Monitor is enable\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========raw==data============\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("UID\t\tTXV\n"));
		for (i = 0; i < PPDUsize; i++) {
			MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_UID_DATA_ADDR, &uid[i]); /*read 25 DWs*/
			MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_PPDU_DATA_ADDR, &txv[i]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x  ", uid[i]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x\n", txv[i]));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========raw==data============\n"));
		tx_mode = (txv[1] & BITS(12, 15)) >> 12;
		txusercount = (txv[1] & BITS(24, 30)) >> 24;
		BW = (txv[1] & BITS(8, 10)) >> 8;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==========================================\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G1_DDW1_1: \t%08x\n", txv[1]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G1_DDW1_2: \t%08x\n", txv[2]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G1_DDW2_1: \t%08x\n", txv[3]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G1_DDW2_2: \t%08x\n", txv[4]));

		for (i = 1; i <= (txusercount * 2); i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_P_G1_N%d_DDW1: \t%08x\n", ((i + 1) / 2), txv[4 + i]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G2_DDW1_1: \t%08x\n", txv[4 + txusercount * 2 + 1]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G2_DDW1_2: \t%08x\n", txv[4 + txusercount * 2 + 2]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G2_DDW2_1: \t%08x\n", txv[4 + txusercount * 2 + 3]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_C_G2_DDW2_2: \t%08x\n", txv[4 + txusercount * 2 + 4]));

		for (i = 1; i <= (txusercount * 2); i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXV_P_G2_N%d_DDW1: \t%08x\n", ((i + 1) / 2), txv[8 + txusercount * 2 + i]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PPDU Payload: "));
		for (i = 1 + 8 + (txusercount * 4); i <= 25; i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x/ ", txv[i]));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
		/****** Common Group 1 DDW1_1 *****/
		if (tx_mode >= 12) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Unknown TXMODE!!!\n")); /*print payload data*/
			goto error;
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXMODE: (%s)\n", HW_TX_MODE[tx_mode]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Number of user: (%d)\n", txusercount));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BW: (%s)\n", HW_TX_BW[BW]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STBC: (%d)\n", (UINT32)(txv[1] & BITS(6, 7) >> 6)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
		/****** Common Group 1 DDW1_1 *****/
		break;
	case 2:
		MAC_IO_WRITE32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_CTRL_ADDR, 0x03000019); /*enable HW TX monitor*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX Monitor is enable"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========All=TXV=Data===========\n"));
		for (i = 0; i < 256; i++) {
			MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_UID_DATA_ADDR, &uid[i]); /*read 25 DWs*/
			MAC_IO_READ32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_PPDU_DATA_ADDR, &txv[i]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x  ", uid[i]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x\n", txv[i]));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========All=TXV=Data===========\n"));
		break;
	case 3:
		MAC_IO_WRITE32(pAd->hdev_ctrl, WF_M2M_PHY_TOP_11AX_M2M_TXMON_CTRL_ADDR, 0x00000019);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX Monitor is disable\n"));
		break;
	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Please select value between 1 ~ 3\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1: Enable TXV monitor and parsing first TXV\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2: Dump 8 sequential TXVs\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3: Disable TXV monitor\n"));
		break;
			}
	return TRUE;
	error:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("failed"));
	return 0;
}
#endif /*MT7915_FPGA*/


#ifdef RANDOM_PKT_GEN
INT32 RandomTxCtrl;
UINT32 Qidmapping[16] = {0};
UINT32 pause_period;
UINT8 random_drop = FALSE;

INT chip_set_txctrl_proc(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 testcase = 0;
	RTMP_STRING *pfp  = NULL;
	UINT32 tmp_value;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pfp = strsep(&arg, ":");

	if (pfp == NULL)
		return FALSE;

	RandomTxCtrl = os_str_tol(pfp, 0, 10);

	if (arg != NULL)
		testcase = os_str_toul(arg, 0, 16);

	MTWF_PRINT("%s(): (RandomTxCtrl = %d) testcase: 0x%x\n", __func__, RandomTxCtrl, testcase);
	if (RandomTxCtrl == 0) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x42104210);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0x42104210);

		for (i = 0; i < cap->qos.WmmHwNum; i++) {
			Qidmapping[i] = 0;
			Qidmapping[i + 1] = 1;
			Qidmapping[i + 2] = 2;
			Qidmapping[i + 4] = 4;
		}
	} else if (RandomTxCtrl == 1) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x7654b210);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0xb210ba98);

		for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = i % 12;

			if (Qidmapping[i] == 3)
				Qidmapping[i] = 11;
		}
	} else if (RandomTxCtrl == 2) {
		UINT i;

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR, 0x89ab0124);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR, 0x01244567);

		for (i = 0; i < cap->qos.WmmHwNum * 4; i++) {
			Qidmapping[i] = (15 - i) % 12;

			if (Qidmapping[i] == 3)
				Qidmapping[i] = 4;
		}
	}

	if (testcase & BIT0) {
		/* default setting */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &tmp_value);
		tmp_value |= WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, tmp_value);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &tmp_value);
		tmp_value &= ~(WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK | WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK);
		tmp_value |= (0x1 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT);
		tmp_value |= (0x8 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, tmp_value);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x76543210);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xfedcba98);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, &tmp_value);
		tmp_value |= WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, tmp_value);
		pause_period = 0;
		random_drop = 0;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
	}

	if (testcase & BIT1) {
		/* disable cr_hif_ask_sub_ena, ple_packet_max_size = 6 */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, tmp_value);
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK;
		tmp_value |= (0x8 << WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, tmp_value);
	}

	if (testcase & BIT2) {
		/* modify schedular priority(0x5000a0b0, 0x5000a0b4, 0x5000a0c4, 0x5000a0c8) */
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR, 0x6012345f);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR, 0xedcba987);
	}

	if (testcase & BIT3) {
		/* disable User program group sequence type control (0x5000a00c[16]) */
		HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, &tmp_value);
		tmp_value &= ~WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, tmp_value);
	}

	if (testcase & BIT4) {
		if (pause_period == 0)
			pause_period = 120;
		else {
			pause_period = 0;
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
		}
	}

	if (testcase & BIT5)
		random_drop = (random_drop == 1) ? 0 : 1;

	if (testcase & BIT6)
		random_drop = (random_drop == 2) ? 0 : 2;

	return TRUE;
}

VOID chip_regular_pause_umac(struct hdev_ctrl *ctrl)
{
	RTMP_ADAPTER *pAd = ctrl->priv;

	if (pause_period == 0)
		return;

	if ((pAd->Mlme.PeriodicRound % (pause_period * 2)) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE6_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE7_ADDR, 0);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE8_ADDR, 0);
	} else if ((pAd->Mlme.PeriodicRound % pause_period) == 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE6_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE7_ADDR, 0xffffffff);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE8_ADDR, 0xffffffff);

	} else if (random_drop
			   && ((pAd->Mlme.PeriodicRound + pause_period / 2) % (pause_period * 2)) == 0) {
		UINT32 ple_stat[4] = {0};
		INT32 i, j;
		UINT32 hfid;
		UINT32 deq_fid;

		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR, &ple_stat[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR, &ple_stat[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR, &ple_stat[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR, &ple_stat[3]);

		for (j = 0; j < 4; j++) {
			for (i = 0; i < 32; i++) {
				if (((ple_stat[j] & (0x1 << i)) >> i) == 0) {
					UINT32 fl_que_ctrl[4] = {0};

					fl_que_ctrl[0] |= (0x1 << WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_SHFT);
					fl_que_ctrl[0] |= (0x2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
					fl_que_ctrl[0] |= (j << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
					fl_que_ctrl[0] |= (i << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
					hfid = fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK;

					if (hfid == 0xfff)
						continue;

					fl_que_ctrl[0] |= (0x2 << WF_PLE_TOP_C_DE_QUEUE_0_DEQ_SUB_TYPE_SHFT);

					if (random_drop == 2)
						fl_que_ctrl[0] |= (0x9 << WF_PLE_TOP_C_DE_QUEUE_0_ENQ_SUB_TYPE_SHFT);

					fl_que_ctrl[1] = (hfid << WF_PLE_TOP_C_DE_QUEUE_1_CUR_LIST_FID_END_SHFT)
								| (hfid << WF_PLE_TOP_C_DE_QUEUE_1_CUR_LIST_FID_START_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_1_ADDR, fl_que_ctrl[1]);

					if (random_drop == 2) {
						/* fl_que_ctrl[3] = 0x3 << 30; */
						fl_que_ctrl[3] |= 0x1f << WF_PLE_TOP_C_DE_QUEUE_2_DEQ_ENQ_DST_QID_SHFT;
						HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_2_ADDR, fl_que_ctrl[3]);
					}

					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_0_ADDR, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_C_DE_QUEUE_3_ADDR, &fl_que_ctrl[2]);
					deq_fid = fl_que_ctrl[2] & WF_PLE_TOP_C_DE_QUEUE_3_DEQ_HEAD_FID_MASK;

					if (deq_fid == 0xfff || random_drop == 2)
						continue;

					fl_que_ctrl[0] = WF_PLE_TOP_C_EN_QUEUE_0_EXECUTE_MASK;
					fl_que_ctrl[0] |= (0x1 << WF_PLE_TOP_C_EN_QUEUE_0_SUB_TYPE_SHFT);
					fl_que_ctrl[0] |= (0x3 << WF_PLE_TOP_C_EN_QUEUE_0_DST_PID_SHFT);
					fl_que_ctrl[0] |= (0x1f << WF_PLE_TOP_C_EN_QUEUE_0_ENQ_DST_QID_SHFT);
					fl_que_ctrl[1] = (deq_fid << WF_PLE_TOP_C_EN_QUEUE_1_CUR_LIST_FID_END_SHFT)
							| (deq_fid << WF_PLE_TOP_C_EN_QUEUE_1_CUR_LIST_FID_START_SHFT);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_EN_QUEUE_1_ADDR, fl_que_ctrl[1]);
					HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_C_EN_QUEUE_0_ADDR, fl_que_ctrl[0]);
				}
			}
		}
	}
}
#endif


static VOID chip_show_bcn_info(struct hdev_ctrl *ctrl, UCHAR bandidx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 mac_val = 0;
	UINT32 idx;
	UINT32 band_offset = 0x10000 * bandidx;

	Show_Mib_Info_Proc(pAd, "");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	show_mib_proc(pAd, (bandidx == DBDC_BAND0) ? "0" : "1");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	show_trinfo_proc(pAd, "");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	ShowPLEInfo(pAd, NULL);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

#ifdef ERR_RECOVERY
	ShowSerProc2(pAd, "");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
#endif

	ShowPseInfo(pAd, NULL);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	show_tpinfo_proc(pAd, "0");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	Show_MibBucket_Proc(pAd, "");
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_SCR_ADDR + band_offset, &mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ARB_SCR=0x%08x\n", mac_val));

	for (idx = 0; idx < 10; idx++) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_BFCR_ADDR + band_offset, &mac_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ARB_BFCR=0x%08x (loop %d)\n", mac_val, idx));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}

static UINT32 chip_show_sta_acq_info(RTMP_ADAPTER *pAd, UINT32 *ple_stat,
				   UINT32 *sta_pause, UINT32 *dis_sta_map,
				   UINT32 dumptxd)
{
	int i, j;
	UINT32 total_nonempty_cnt = 0;

	for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				UINT32 hfid, tfid, pktcnt, ac_num = j / CR_NUM_OF_AC, ctrl = 0;
				UINT32 sta_num = i + (j % CR_NUM_OF_AC) * 32, fl_que_ctrl[3] = {0};
				struct wifi_dev *wdev = wdev_search_by_wcid(pAd, sta_num);
				UINT32 wmmidx = 0;

				if (wdev)
					wmmidx = HcGetWmmIdx(pAd, wdev);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("\tSTA%d AC%d: ", sta_num, ac_num));

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt));

				if (((sta_pause[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" ctrl = %s", sta_ctrl_reg[ctrl]));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" (wmmidx=%d)\n", wmmidx));

				total_nonempty_cnt++;

				if (pktcnt > 0 && dumptxd > 0)
					ShowTXDInfo(pAd, hfid);
			}
		}
	}

	return total_nonempty_cnt;
}

static VOID chip_show_txcmdq_info(RTMP_ADAPTER *pAd, UINT32 ple_txcmd_stat)
{
	int i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nonempty TXCMD Q info:\n"));
	for (i = 0; i < 32 ; i++) {
		if (((ple_txcmd_stat & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_txcmd_queue_empty_info[i].QueueName != NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t%s: ", ple_txcmd_queue_empty_info[i].QueueName));
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[i].Portid <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[i].Queueid <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt));
		}
	}
}

static VOID chip_get_ple_acq_stat(RTMP_ADAPTER *pAd, UINT32 *ple_stat)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_QUEUE_EMPTY_ADDR, &ple_stat[0]);

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR, &ple_stat[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY1_ADDR, &ple_stat[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY2_ADDR, &ple_stat[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY3_ADDR, &ple_stat[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY4_ADDR, &ple_stat[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY5_ADDR, &ple_stat[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY6_ADDR, &ple_stat[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY7_ADDR, &ple_stat[8]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC0_QUEUE_EMPTY8_ADDR, &ple_stat[9]);

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR, &ple_stat[10]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY1_ADDR, &ple_stat[11]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY2_ADDR, &ple_stat[12]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY3_ADDR, &ple_stat[13]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY4_ADDR, &ple_stat[14]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY5_ADDR, &ple_stat[15]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY6_ADDR, &ple_stat[16]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY7_ADDR, &ple_stat[17]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC1_QUEUE_EMPTY8_ADDR, &ple_stat[18]);

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR, &ple_stat[19]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY1_ADDR, &ple_stat[20]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY2_ADDR, &ple_stat[21]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY3_ADDR, &ple_stat[22]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY4_ADDR, &ple_stat[23]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY5_ADDR, &ple_stat[24]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY6_ADDR, &ple_stat[25]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY7_ADDR, &ple_stat[26]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC2_QUEUE_EMPTY8_ADDR, &ple_stat[27]);

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR, &ple_stat[28]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY1_ADDR, &ple_stat[29]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY2_ADDR, &ple_stat[30]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY3_ADDR, &ple_stat[31]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY4_ADDR, &ple_stat[32]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY5_ADDR, &ple_stat[33]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY6_ADDR, &ple_stat[34]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY7_ADDR, &ple_stat[35]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AC3_QUEUE_EMPTY8_ADDR, &ple_stat[36]);
}

static VOID chip_get_ple_txcmd_stat(RTMP_ADAPTER *pAd, UINT32 *ple_txcmd_stat)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_NATIVE_TXCMD_QUEUE_EMPTY_ADDR, ple_txcmd_stat);
}

static VOID chip_get_dis_sta_map(RTMP_ADAPTER *pAd, UINT32 *dis_sta_map)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP0_ADDR, &dis_sta_map[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP1_ADDR, &dis_sta_map[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP2_ADDR, &dis_sta_map[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP3_ADDR, &dis_sta_map[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP4_ADDR, &dis_sta_map[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP5_ADDR, &dis_sta_map[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP6_ADDR, &dis_sta_map[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP7_ADDR, &dis_sta_map[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DIS_STA_MAP8_ADDR, &dis_sta_map[8]);
}

static VOID chip_get_sta_pause(RTMP_ADAPTER *pAd, UINT32 *sta_pause)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE0_ADDR, &sta_pause[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE1_ADDR, &sta_pause[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE2_ADDR, &sta_pause[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE3_ADDR, &sta_pause[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE4_ADDR, &sta_pause[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE5_ADDR, &sta_pause[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE6_ADDR, &sta_pause[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE7_ADDR, &sta_pause[7]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_STATION_PAUSE8_ADDR, &sta_pause[8]);
}

static INT32 chip_show_drr_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{


#define DL_AC_START 0x00
#define DL_AC_END   0x0F
#define UL_AC_START 0x10
#define UL_AC_END   0x1F

	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 drr_ctrl_def_val = 0x80220000, drr_ctrl_val = 0;
	UINT32 drr_sta_status[16] = {0};
	UINT idx = 0, sta_line = 0, sta_no = 0, max_sta_line = (MAX_LEN_OF_MAC_TABLE+31)/32;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("DRR Table STA Info:\n"));

	for (idx = DL_AC_START; idx <= DL_AC_END; idx++) {
		BOOLEAN sta[MAX_LEN_OF_MAC_TABLE] = {0};
		BOOLEAN is_show = FALSE;
		drr_ctrl_val = (drr_ctrl_def_val | idx);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);

		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7]);

		if (max_sta_line > 8) {
			drr_ctrl_val = (drr_ctrl_def_val | idx | 1<<10);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[9]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[10]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[11]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[12]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[13]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[14]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[15]);
		}


		for (sta_line = 0; sta_line < max_sta_line; sta_line++) {
			if (drr_sta_status[sta_line] > 0) {
				for (sta_no = 0; sta_no < 32; sta_no++) {
					if (((drr_sta_status[sta_line] & (0x1 << sta_no)) >> sta_no)) {
						is_show = TRUE;
						sta[sta_no + (sta_line * 32)] = TRUE;
					}
				}
			}
		}

		if (is_show) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tDL AC%02d Queue Non-Empty STA:\n", idx));

			for (sta_line = 0; sta_line < 256 && sta_line < MAX_LEN_OF_MAC_TABLE; sta_line++) {
				if (sta[sta_line])
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%d ", sta_line));
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

	}

	for (idx = UL_AC_START; idx <= UL_AC_END; idx++) {
		BOOLEAN sta[MAX_LEN_OF_MAC_TABLE] = {0};
		BOOLEAN is_show = FALSE;
		drr_ctrl_val = (drr_ctrl_def_val | idx);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);

		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7]);

		if (max_sta_line > 8) {
			drr_ctrl_val = (drr_ctrl_def_val | idx | 1<<10);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[9]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[10]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[11]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[12]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[13]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[14]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[15]);
		}


		for (sta_line = 0; sta_line < max_sta_line; sta_line++) {
			if (drr_sta_status[sta_line] > 0) {
				for (sta_no = 0; sta_no < 32; sta_no++) {
					if (((drr_sta_status[sta_line] & (0x1 << sta_no)) >> sta_no)) {
						is_show = TRUE;
						sta[sta_no + (sta_line * 32)] = TRUE;
					}
				}
			}
		}

		if (is_show) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tUL AC%02d Queue Non-Empty STA:\n", idx));

			for (sta_line = 0; sta_line < 256 && sta_line < MAX_LEN_OF_MAC_TABLE; sta_line++) {
				if (sta[sta_line])
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%d ", sta_line));
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

	}

	for (idx = DL_AC_START; idx <= DL_AC_END; idx++) {
		BOOLEAN is_show = TRUE;

		drr_ctrl_def_val = 0x80420000;
		drr_ctrl_val = (drr_ctrl_def_val | idx);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7]);

		if (max_sta_line > 8) {
			drr_ctrl_val = (drr_ctrl_def_val | idx | 1<<10);
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[9]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[10]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[11]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[12]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[13]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[14]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[15]);
		}


		if (is_show) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("BSSGrp[%d]:\n", idx));

			for (sta_line = 0; sta_line < max_sta_line; sta_line++) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("0x%08X ", drr_sta_status[sta_line]));

				if ((sta_line % 4) == 3)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		}
	}

	return TRUE;
}

static INT32 chip_show_ple_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_buf_ctrl = 0, pg_sz, pg_num;
	UINT32 ple_stat[ALL_CR_NUM_OF_ALL_AC + 1] = {0}, pg_flow_ctrl[CR_NUM_OF_AC] = {0};
	UINT32 ple_native_txcmd_stat = 0;
	UINT32 ple_txcmd_stat = 0;
	UINT32 sta_pause[CR_NUM_OF_AC] = {0}, dis_sta_map[CR_NUM_OF_AC] = {0};
	UINT32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	UINT32 rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	INT32 i, j;
	UINT32 dumptxd = 0;

	if (arg != NULL)
		dumptxd = os_str_toul(arg, 0, 16);

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PBUF_CTRL_ADDR, &ple_buf_ctrl);
	chip_get_ple_acq_stat(pAd, ple_stat);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_TXCMD_QUEUE_EMPTY_ADDR, &ple_txcmd_stat);
	chip_get_ple_txcmd_stat(pAd, &ple_native_txcmd_stat);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR, &pg_flow_ctrl[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_HIF_GROUP_ADDR, &pg_flow_ctrl[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR, &pg_flow_ctrl[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR, &pg_flow_ctrl[7]);
	chip_get_dis_sta_map(pAd, dis_sta_map);
	chip_get_sta_pause(pAd, sta_pause);

	/* Configuration Info */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PLE Configuration Info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tPacket Buffer Control(0x82060014): 0x%08x\n", ple_buf_ctrl));
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tPage Offset=%d(in unit of 2KB)\n",
			 (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT));
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tTotal Page=%d pages\n", pg_num));
	/* Page Flow Control */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("PLE Page Flow Control:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFree page counter(0x820c0100): 0x%08x\n", pg_flow_ctrl[0]));
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe toal page number of free=0x%03x\n", fpg_cnt));
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFree page head and tail(0x820c0104): 0x%08x\n", pg_flow_ctrl[1]));
	fpg_head = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of HIF group(0x820c0110): 0x%08x\n", pg_flow_ctrl[2]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHIF group page status(0x820c0114): 0x%08x\n", pg_flow_ctrl[3]));
	hif_min_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n", hif_max_q, hif_min_q));
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n", upg_hif, rpg_hif));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of HIF_TXCMD group(0x820c0118): 0x%08x\n", pg_flow_ctrl[6]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHIF_TXCMD group page status(0x820c011c): 0x%08x\n", pg_flow_ctrl[7]));
	cpu_min_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q));
	rpg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tReserved page counter of CPU group(0x820c0150): 0x%08x\n", pg_flow_ctrl[4]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tCPU group page status(0x820c0154): 0x%08x\n", pg_flow_ctrl[5]));
	cpu_min_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q));
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu));

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {


		for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) {
			if (j % CR_NUM_OF_AC == 0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("\n\tNonempty AC%d Q of STA#: ", j / CR_NUM_OF_AC));
			}

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("%d ", i + (j % CR_NUM_OF_AC) * 32));
				}
			}
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("non-native/native txcmd queue empty = %d/%d\n", ple_txcmd_stat, ple_native_txcmd_stat));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nonempty Q info:\n"));

	for (i = 0; i < 32; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_queue_empty_info[i].QueueName != NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s: ", ple_queue_empty_info[i].QueueName));
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].Portid << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].Queueid << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			if (ple_queue_empty_info[i].Queueid >= ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0 &&
				ple_queue_empty_info[i].Queueid <= ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0)
				/* band0 set TGID 0, bit31 = 0 */
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_1_ADDR, 0x0);
			else if (ple_queue_empty_info[i].Queueid >= ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1 &&
				ple_queue_empty_info[i].Queueid <= ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1)
				/* band1 set TGID 1, bit31 = 1 */
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_1_ADDR, 0x80000000);

			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt));

			if (pktcnt > 0 && dumptxd > 0)
				ShowTXDInfo(pAd, hfid);
		}
	}

	chip_show_sta_acq_info(pAd, ple_stat, sta_pause, dis_sta_map, dumptxd);
	chip_show_txcmdq_info(pAd, ple_native_txcmd_stat);

	return TRUE;
}

static INT32 chip_show_amsdu_info(struct hdev_ctrl *ctrl)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_stat[8] = {0}, total_amsdu = 0;
	UCHAR i;

	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_1_MSDU_CNT_ADDR, &ple_stat[0]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_2_MSDU_CNT_ADDR, &ple_stat[1]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_3_MSDU_CNT_ADDR, &ple_stat[2]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_4_MSDU_CNT_ADDR, &ple_stat[3]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_5_MSDU_CNT_ADDR, &ple_stat[4]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_6_MSDU_CNT_ADDR, &ple_stat[5]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_7_MSDU_CNT_ADDR, &ple_stat[6]);
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_AMSDU_PACK_8_MSDU_CNT_ADDR, &ple_stat[7]);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TXD counter status of MSDU:\n"));

	for (i = 0; i < 8; i++)
		total_amsdu += ple_stat[i];

	for (i = 0; i < 8; i++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("AMSDU pack count of %d MSDU in TXD: 0x%x ", i+1, ple_stat[i]));
		if (total_amsdu != 0)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%d%%)\n", ple_stat[i] * 100 / total_amsdu));
		else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}

#define NO_SHIFT_DEFINE 0xFFFFFFFF

static UINT32
io_r_32(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr, &value);

	return value;
}

UINT32
halWtblWriteRaw(
	RTMP_ADAPTER *pAd,
	UINT_16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	UINT_16  u2DW,
	UINT_32  u4Value
)
{
	UINT32 u4WtblVmAddr = 0;

	if (eType == WTBL_TYPE_LMAC) {
		LWTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = LWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_UMAC) {
		UWTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = UWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_KEY) {
		KEYTBL_CONFIG(u2EntryIdx);
		u4WtblVmAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else {
		/*TODO:*/
	}

	IO_W_32(u4WtblVmAddr, u4Value);

	return 0;
}

UINT32
halWtblReadRaw(
	RTMP_ADAPTER *pAd,
	UINT_16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	UINT_16  u2StartDW,
	UINT_16  u2LenInDW,
	PVOID    pBuffer
)
{
	UINT_32 *dest_cpy = (UINT_32 *)pBuffer;
	UINT_32 sizeInDW = u2LenInDW;
	UINT_32 u4SrcAddr = 0;

	if (pBuffer == NULL)
		return 0xFF;

	if (eType == WTBL_TYPE_LMAC) {
		LWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = LWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_UMAC) {
		UWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = UWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_KEY) {
		KEYTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else{
		/* TODO: */
	}

	while (sizeInDW--) {
		*dest_cpy++ = IO_R_32(u4SrcAddr);
		u4SrcAddr += 4;
	}

	return 0;
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW0[] = {
    {"MUAR_IDX",    WTBL_MUAR_IDX_MASK, WTBL_MUAR_IDX_OFFSET,	FALSE},
    {"RCA1",        WTBL_RCA1,          NO_SHIFT_DEFINE,	FALSE},
    {"KID",         WTBL_KID_MASK,      WTBL_KID_OFFSET,	FALSE},
    {"RCID",        WTBL_RCID,          NO_SHIFT_DEFINE,	FALSE},
    {"FROM_DS",     WTBL_FROM_DS,       NO_SHIFT_DEFINE,	TRUE},
    {"TO_DS",       WTBL_TO_DS,         NO_SHIFT_DEFINE,	FALSE},
    {"RV",          WTBL_RV,            NO_SHIFT_DEFINE,	FALSE},
    {"RCA2",        WTBL_RCA2,          NO_SHIFT_DEFINE,	FALSE},
    {"WPI_FLAG",    WTBL_WPI_FLAG,      NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW0_1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\n\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
			  lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]));

	/* LMAC WTBL DW 0 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 0/1\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_PEER_INFO_DW_0*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW0[i].name) {

		if (WTBL_LMAC_DW0[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW0[i].name,
					 (dw_value & WTBL_LMAC_DW0[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW0[i].name,
					  (dw_value & WTBL_LMAC_DW0[i].mask) >> WTBL_LMAC_DW0[i].shift));

		if (WTBL_LMAC_DW0[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW2[] = {
    {"AID12",               WTBL_AID12_MASK,                WTBL_AID12_OFFSET,	FALSE},
    {"SU",                  WTBL_GID_SU,                    NO_SHIFT_DEFINE,	FALSE},
    {"SPP_EN",              WTBL_SPP_EN,                    NO_SHIFT_DEFINE,	FALSE},
    {"WPI_EVEN",            WTBL_WPI_EVEN,                  NO_SHIFT_DEFINE,	TRUE},
    {"CIPHER",              WTBL_CIPHER_SUITE_MASK,         WTBL_CIPHER_SUITE_OFFSET,		FALSE},
    {"CIPHER_IGTK",         WTBL_CIPHER_SUITE_IGTK_MASK,    WTBL_CIPHER_SUITE_IGTK_OFFSET,	FALSE},
    {"AAD_OM",              WTBL_AAD_OM,                    NO_SHIFT_DEFINE,	TRUE},
    {"SW",                  WTBL_SW,                        NO_SHIFT_DEFINE,	FALSE},
    {"UL",                  WTBL_UL,                        NO_SHIFT_DEFINE,	FALSE},
    {"TX_POWER_SAVE",       WTBL_TX_POWER_SAVE_STATUS,      NO_SHIFT_DEFINE,	TRUE},
    {"QOS",                 WTBL_QOS,                       NO_SHIFT_DEFINE,	FALSE},
    {"HT",                  WTBL_HT,                        NO_SHIFT_DEFINE,	FALSE},
    {"VHT",                 WTBL_VHT,                       NO_SHIFT_DEFINE,	FALSE},
    {"HE",                  WTBL_HE,                        NO_SHIFT_DEFINE,	FALSE},
    {"MESH",                WTBL_MESH,                      NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW2(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 2 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 2\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW2[i].name) {

		if (WTBL_LMAC_DW2[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW2[i].name,
					 (dw_value & WTBL_LMAC_DW2[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW2[i].name,
					  (dw_value & WTBL_LMAC_DW2[i].mask) >> WTBL_LMAC_DW2[i].shift));
		if (WTBL_LMAC_DW2[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW3[] = {
    {"WMM_Q",           WTBL_WMM_Q_MASK,        WTBL_WMM_Q_OFFSET,	FALSE},
    {"RXD_DUP_MODE",    WTBL_RXD_DUP_MODE_MASK, WTBL_RXD_DUP_MODE_OFFSET, TRUE},
    {"VLAN2ETH",        WTBL_VLAN2ETH,          NO_SHIFT_DEFINE,	FALSE},
    {"BEAM_CHG",        WTBL_BEAM_CHG,          NO_SHIFT_DEFINE,	FALSE},
    {"DIS_BA256",       WTBL_DIS_BA256,         NO_SHIFT_DEFINE,	TRUE},
    {"PFMU_IDX",        WTBL_PFMU_IDX_MASK,     WTBL_PFMU_IDX_OFFSET,	FALSE},
    {"ULPF_IDX",        WTBL_ULPF_IDX_MASK,     WTBL_ULPF_IDX_OFFSET,	FALSE},
    {"RIBF",            WTBL_RIBF,              NO_SHIFT_DEFINE,	FALSE},
    {"ULPF",            WTBL_ULPF,              NO_SHIFT_DEFINE,	TRUE},
    {"IGN_FBK",         WTBL_IGN_FBK,           NO_SHIFT_DEFINE,	FALSE},
    {"TBF",             WTBL_TBF,               NO_SHIFT_DEFINE,	FALSE},
    {"TBF_VHT",         WTBL_TBF_VHT,           NO_SHIFT_DEFINE,	FALSE},
    {"TBF_HE",          WTBL_TBF_HE,            NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW3(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 3 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 3\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3[i].name) {

		if (WTBL_LMAC_DW3[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW3[i].name,
					 (dw_value & WTBL_LMAC_DW3[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW3[i].name,
					  (dw_value & WTBL_LMAC_DW3[i].mask) >> WTBL_LMAC_DW3[i].shift));
		if (WTBL_LMAC_DW3[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW4[] = {
    {"ANT_ID_STS0",     WTBL_ANT_ID_STS0_MASK,      WTBL_ANT_ID_STS0_OFFSET,	FALSE},
    {"STS1",            WTBL_ANT_ID_STS1_MASK,      WTBL_ANT_ID_STS1_OFFSET,	FALSE},
    {"STS2",            WTBL_ANT_ID_STS2_MASK,      WTBL_ANT_ID_STS2_OFFSET,	FALSE},
    {"STS3",            WTBL_ANT_ID_STS3_MASK,      WTBL_ANT_ID_STS3_OFFSET,	TRUE},
    {"ANT_ID_STS4",     WTBL_ANT_ID_STS4_MASK,      WTBL_ANT_ID_STS4_OFFSET,	FALSE},
    {"STS5",            WTBL_ANT_ID_STS5_MASK,      WTBL_ANT_ID_STS5_OFFSET,	FALSE},
    {"STS6",            WTBL_ANT_ID_STS6_MASK,      WTBL_ANT_ID_STS6_OFFSET,	FALSE},
    {"STS7",            WTBL_ANT_ID_STS7_MASK,      WTBL_ANT_ID_STS7_OFFSET,	TRUE},
    {"CASCAD",          WTBL_CASCAD,                NO_SHIFT_DEFINE,	FALSE},
    {"LDPC_HT",         WTBL_LDPC_HT,               NO_SHIFT_DEFINE,	FALSE},
    {"LDPC_VHT",        WTBL_LDPC_VHT,              NO_SHIFT_DEFINE,	FALSE},
    {"LDPC_HE",         WTBL_LDPC_HE,               NO_SHIFT_DEFINE,	TRUE},
    {"DIS_RHTR",        WTBL_DIS_RHTR,              NO_SHIFT_DEFINE,	FALSE},
    {"ALL_ACK",         WTBL_ALL_ACK,               NO_SHIFT_DEFINE,	FALSE},
    {"DROP",            WTBL_DROP,                  NO_SHIFT_DEFINE,	FALSE},
    {"ACK_EN",          WTBL_ACK_EN,                NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW4(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 4 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 4\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW4[i].name) {
		if (WTBL_LMAC_DW4[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW4[i].name,
					 (dw_value & WTBL_LMAC_DW4[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW4[i].name,
					  (dw_value & WTBL_LMAC_DW4[i].mask) >> WTBL_LMAC_DW4[i].shift));

		if (WTBL_LMAC_DW4[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW5[] = {
    {"AF",                  WTBL_AF_MASK,               WTBL_AF_OFFSET,		FALSE},
    {"AF_HE",               WTBL_AF_HE_MASK,            WTBL_AF_HE_OFFSET,	FALSE},
    {"RTS",                 WTBL_RTS,                   NO_SHIFT_DEFINE,	FALSE},
    {"SMPS",                WTBL_SMPS,                  NO_SHIFT_DEFINE,	FALSE},
    {"DYN_BW",              WTBL_DYN_BW,                NO_SHIFT_DEFINE,	TRUE},
    {"MMSS",                WTBL_MMSS_MASK,             WTBL_MMSS_OFFSET,	FALSE},
    {"USR",                 WTBL_USR,                   NO_SHIFT_DEFINE,	FALSE},
    {"SR_RATE",             WTBL_SR_RATE_MASK,          WTBL_SR_RATE_OFFSET,	FALSE},
    {"SR_ABORT",            WTBL_SR_ABORT,              NO_SHIFT_DEFINE,	TRUE},
    {"TX_POWER_OFFSET",     WTBL_TX_POWER_OFFSET_MASK,  WTBL_TX_POWER_OFFSET_OFFSET,	FALSE},
    {"WTBL_MPDU_SIZE",      WTBL_MPDU_SIZE_MASK,        WTBL_MPDU_SIZE_OFFSET,	TRUE},
    {"PE",                  WTBL_PE_MASK,               WTBL_PE_OFFSET,		FALSE},
    {"DOPPL",               WTBL_DOPPL,                 NO_SHIFT_DEFINE,	FALSE},
    {"TXOP_PS_CAP",         WTBL_TXOP_PS_CAP,           NO_SHIFT_DEFINE,	FALSE},
    {"DONOT_UPDATE_I_PSM",  WTBL_DONOT_UPDATE_I_PSM,    NO_SHIFT_DEFINE,	TRUE},
    {"I_PSM",               WTBL_I_PSM,                 NO_SHIFT_DEFINE,	FALSE},
    {"PSM",                 WTBL_PSM,                   NO_SHIFT_DEFINE,	FALSE},
    {"SKIP_TX",             WTBL_SKIP_TX,               NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW5(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 5 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 5\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5[i].name) {
		if (WTBL_LMAC_DW5[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW5[i].name,
					 (dw_value & WTBL_LMAC_DW5[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW5[i].name,
					  (dw_value & WTBL_LMAC_DW5[i].mask) >> WTBL_LMAC_DW5[i].shift));
		if (WTBL_LMAC_DW5[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static VOID parse_fmac_lwtbl_DW6(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	int i = 0;

	/* LMAC WTBL DW 6 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 6\n"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_6*4]);
	dw_value = *addr;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tTID 0/1/2/3/4/5/6/7 BA_WIN_SIZE:"));

	for (i = 0; i < 8; i++)
		if (i != 7)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%lu/", ((dw_value & BITS(i*4, i*4+3)) >> i*4)));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%lu\n", ((dw_value & BITS(i*4, i*4+3)) >> i*4)));
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW7[] = {
    {"CBRN",        WTBL_CBRN_MASK,     WTBL_CBRN_OFFSET,	FALSE},
    {"DBNSS_EN",    WTBL_DBNSS_EN,      NO_SHIFT_DEFINE,	FALSE},
    {"BAF_EN",      WTBL_BAF_EN,        NO_SHIFT_DEFINE,	FALSE},
    {"RDGBA",       WTBL_RDGBA,         NO_SHIFT_DEFINE,	TRUE},
    {"RDG",         WTBL_RDG,           NO_SHIFT_DEFINE,	FALSE},
    {"SPE_IDX",     WTBL_SPE_IDX_MASK,  WTBL_SPE_IDX_OFFSET,	FALSE},
    {"G2",          WTBL_G2,            NO_SHIFT_DEFINE,	FALSE},
    {"G4",          WTBL_G4,            NO_SHIFT_DEFINE,	FALSE},
    {"G8",          WTBL_G8,            NO_SHIFT_DEFINE,	FALSE},
    {"G16",         WTBL_G16,           NO_SHIFT_DEFINE,	TRUE},
    {"G2_LTF",      WTBL_G2_LTF_MASK,   WTBL_G2_LTF_OFFSET,	FALSE},
    {"G4_LTF",      WTBL_G4_LTF_MASK,   WTBL_G4_LTF_OFFSET,	FALSE},
    {"G8_LTF",      WTBL_G8_LTF_MASK,   WTBL_G8_LTF_OFFSET,	FALSE},
    {"G16_LTF",     WTBL_G16_LTF_MASK,  WTBL_G16_LTF_OFFSET,	TRUE},
    {"G2_HE",       WTBL_G2_HE_MASK,    WTBL_G2_HE_OFFSET,	FALSE},
    {"G4_HE",       WTBL_G4_HE_MASK,    WTBL_G4_HE_OFFSET,	FALSE},
    {"G8_HE",       WTBL_G8_HE_MASK,    WTBL_G8_HE_OFFSET,	FALSE},
    {"G16_HE",      WTBL_G16_HE_MASK,   WTBL_G16_HE_OFFSET,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW7(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 7 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 7\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_7*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW7[i].name) {
		if (WTBL_LMAC_DW7[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW7[i].name,
					 (dw_value & WTBL_LMAC_DW7[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW7[i].name,
					  (dw_value & WTBL_LMAC_DW7[i].mask) >> WTBL_LMAC_DW7[i].shift));
		if (WTBL_LMAC_DW7[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW8[] = {
    {"FAIL_CNT_AC0",    WTBL_FAIL_CNT_AC0_MASK, WTBL_FAIL_CNT_AC0_OFFSET,	FALSE},
    {"AC1",    WTBL_FAIL_CNT_AC1_MASK, WTBL_FAIL_CNT_AC1_OFFSET,	FALSE},
    {"AC2",    WTBL_FAIL_CNT_AC2_MASK, WTBL_FAIL_CNT_AC2_OFFSET,	FALSE},
    {"AC3",    WTBL_FAIL_CNT_AC3_MASK, WTBL_FAIL_CNT_AC3_OFFSET,	TRUE},
    {"PARTIAL_AID",     WTBL_PARTIAL_AID_MASK,  WTBL_PARTIAL_AID_OFFSET,	FALSE},
    {"CHK_PER",         WTBL_CHK_PER,           NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW8(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 8 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 8\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW8[i].name) {
		if (WTBL_LMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW8[i].name,
					 (dw_value & WTBL_LMAC_DW8[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW8[i].name,
					  (dw_value & WTBL_LMAC_DW8[i].mask) >> WTBL_LMAC_DW8[i].shift));
		if (WTBL_LMAC_DW8[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW9[] = {
    {"RX_AVG_MPDU",     WTBL_RX_AVG_MPDU_SIZE_MASK, WTBL_RX_AVG_MPDU_SIZE_OFFSET,	FALSE},
    {"PRITX_SW_MODE",   WTBL_PRITX_SW_MODE,         NO_SHIFT_DEFINE,	FALSE},
    {"PRITX_PLR",       WTBL_PRITX_PLR,             NO_SHIFT_DEFINE,	TRUE},
    {"PRITX_DCM",       WTBL_PRITX_DCM,             NO_SHIFT_DEFINE,	FALSE},
    {"PRITX_ER160",     WTBL_PRITX_ER160,           NO_SHIFT_DEFINE,	FALSE},
    {"PRITX_ERSU",      WTBL_PRITX_ERSU,            NO_SHIFT_DEFINE,	TRUE},
/*     {"FCAP(0:20 1:~40)",    WTBL_FCAP_20_TO_160_MHZ,    WTBL_FCAP_20_TO_160_MHZ_OFFSET}, */
    {"MPDU_FAIL_CNT",   WTBL_MPDU_FAIL_CNT_MASK,    WTBL_MPDU_FAIL_CNT_OFFSET,	FALSE},
    {"MPDU_OK_CNT",     WTBL_MPDU_OK_CNT_MASK,      WTBL_MPDU_OK_CNT_OFFSET,	FALSE},
    {"RATE_IDX",        WTBL_RATE_IDX_MASK,         WTBL_RATE_IDX_OFFSET,	TRUE},
    {NULL,}
};

RTMP_STRING *fcap_name[] = {"20MHz", "20/40MHz", "20/40/80MHz", "20/40/80/160/80+80MHz"};

static VOID parse_fmac_lwtbl_DW9(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 8 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 9\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_9*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW9[i].name) {
		if (WTBL_LMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW9[i].name,
					 (dw_value & WTBL_LMAC_DW9[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW9[i].name,
					  (dw_value & WTBL_LMAC_DW9[i].mask) >> WTBL_LMAC_DW9[i].shift));
		if (WTBL_LMAC_DW9[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}

	/* FCAP parser */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("FCAP:%s\n", fcap_name[(dw_value & WTBL_FCAP_20_TO_160_MHZ) >> WTBL_FCAP_20_TO_160_MHZ_OFFSET]));
}

#define HW_TX_RATE_TO_MODE(_x)			(((_x) & BITS(6, 9)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)			(((_x) & BITS(10, 12)) >> 10)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & BIT(13)) >> 13)

#define MAX_TX_MODE 12
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A", "N/A", "N/A",
							"HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2Mlong", "5.5Mlong", "11Mlong", "N/A", "2Mshort", "5.5Mshort", "11Mshort", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(UINT16 ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(UINT8 mode, UINT16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 8 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[8];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}

static VOID parse_rate(RTMP_ADAPTER *pAd, UINT16 rate_idx, UINT16 txrate)
{
	UINT16 txmode, mcs, nss, stbc;

	txmode = HW_TX_RATE_TO_MODE(txrate);
	mcs = HW_TX_RATE_TO_MCS(txrate, txmode);
	nss = HW_TX_RATE_TO_NSS(txrate);
	stbc = HW_TX_RATE_TO_STBC(txrate);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
			  rate_idx + 1, txrate,
			  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
			  mcs, hw_rate_str(txmode, mcs), nss, stbc));
}


static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW10[] = {
    {"RATE1",       WTBL_RATE1_MASK,        WTBL_RATE1_OFFSET},
    {"RATE2",       WTBL_RATE2_MASK,        WTBL_RATE2_OFFSET},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW10(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 10 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 10\n"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_1_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW10[i].name) {
		parse_rate(pAd, i, (dw_value & WTBL_LMAC_DW10[i].mask) >> WTBL_LMAC_DW10[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW11[] = {
    {"RATE3",       WTBL_RATE3_MASK,        WTBL_RATE3_OFFSET},
    {"RATE4",       WTBL_RATE4_MASK,        WTBL_RATE4_OFFSET},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW11(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 11 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 11\n"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_3_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW11[i].name) {
		parse_rate(pAd, i+2, (dw_value & WTBL_LMAC_DW11[i].mask) >> WTBL_LMAC_DW11[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW12[] = {
    {"RATE5",       WTBL_RATE5_MASK,        WTBL_RATE5_OFFSET},
    {"RATE6",       WTBL_RATE6_MASK,        WTBL_RATE6_OFFSET},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW12(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 12 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 12\n"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_5_6*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW12[i].name) {
		parse_rate(pAd, i+4, (dw_value & WTBL_LMAC_DW12[i].mask) >> WTBL_LMAC_DW12[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
} WTBL_LMAC_DW13[] = {
    {"RATE7",       WTBL_RATE7_MASK,        WTBL_RATE7_OFFSET},
    {"RATE8",       WTBL_RATE8_MASK,        WTBL_RATE8_OFFSET},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW13(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 13 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 13\n"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_7_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW13[i].name) {
		parse_rate(pAd, i+6, (dw_value & WTBL_LMAC_DW13[i].mask) >> WTBL_LMAC_DW13[i].shift);
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW3_E1[] = {
    {"WMM_Q",           WTBL_WMM_Q_MASK,        WTBL_WMM_Q_OFFSET,	FALSE},
    {"RXD_DUP_MODE",    WTBL_RXD_DUP_MODE_MASK, WTBL_RXD_DUP_MODE_OFFSET, TRUE},
    {"VLAN2ETH",        WTBL_VLAN2ETH,          NO_SHIFT_DEFINE,	FALSE},
    {"PFMU_IDX",        WTBL_PFMU_IDX_MASK,     WTBL_PFMU_IDX_OFFSET,	FALSE},
    {"RIBF",            WTBL_RIBF,              NO_SHIFT_DEFINE,	TRUE},
    {"TBF",             WTBL_TBF,               NO_SHIFT_DEFINE,	FALSE},
    {"TBF_VHT",         WTBL_TBF_VHT,           NO_SHIFT_DEFINE,	FALSE},
    {"TBF_HE",          WTBL_TBF_HE,            NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW3_E1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 3 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 3\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3_E1[i].name) {

		if (WTBL_LMAC_DW3_E1[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW3_E1[i].name,
					 (dw_value & WTBL_LMAC_DW3_E1[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW3_E1[i].name,
					  (dw_value & WTBL_LMAC_DW3_E1[i].mask) >> WTBL_LMAC_DW3_E1[i].shift));
		if (WTBL_LMAC_DW3_E1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

#define WTBL_SR_RATE_E1_MASK        BITS(12, 13)
#define WTBL_SR_RATE_E1_OFFSET      12
#define WTBL_BEAM_CHG_E1            BIT(14)

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW5_E1[] = {
    {"AF",                  WTBL_AF_MASK,               WTBL_AF_OFFSET,		FALSE},
    {"AF_HE",               WTBL_AF_HE_MASK,            WTBL_AF_HE_OFFSET,	FALSE},
    {"RTS",                 WTBL_RTS,                   NO_SHIFT_DEFINE,	FALSE},
    {"SMPS",                WTBL_SMPS,                  NO_SHIFT_DEFINE,	FALSE},
    {"DYN_BW",              WTBL_DYN_BW,                NO_SHIFT_DEFINE,	FALSE},
    {"MMSS",                WTBL_MMSS_MASK,             WTBL_MMSS_OFFSET,	TRUE},
    {"USR",                 WTBL_USR,                   NO_SHIFT_DEFINE,	FALSE},
    {"SR_RATE",             WTBL_SR_RATE_E1_MASK,       WTBL_SR_RATE_E1_OFFSET,	FALSE},
    {"BEAM_CHG",            WTBL_BEAM_CHG_E1,           NO_SHIFT_DEFINE,	FALSE},
    {"SR_ABORT",            WTBL_SR_ABORT,              NO_SHIFT_DEFINE,	TRUE},
    {"TX_POWER_OFFSET",     WTBL_TX_POWER_OFFSET_MASK,  WTBL_TX_POWER_OFFSET_OFFSET,	FALSE},
    {"WTBL_MPDU_SIZE",      WTBL_MPDU_SIZE_MASK,        WTBL_MPDU_SIZE_OFFSET,	FALSE},
    {"PE",                  WTBL_PE_MASK,               WTBL_PE_OFFSET,		TRUE},
    {"DOPPL",               WTBL_DOPPL,                 NO_SHIFT_DEFINE,	FALSE},
    {"TXOP_PS_CAP",         WTBL_TXOP_PS_CAP,           NO_SHIFT_DEFINE,	FALSE},
    {"DONOT_UPDATE_I_PSM",  WTBL_DONOT_UPDATE_I_PSM,    NO_SHIFT_DEFINE,	TRUE},
    {"I_PSM",               WTBL_I_PSM,                 NO_SHIFT_DEFINE,	FALSE},
    {"PSM",                 WTBL_PSM,                   NO_SHIFT_DEFINE,	FALSE},
    {"SKIP_TX",             WTBL_SKIP_TX,               NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW5_E1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 5 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 5\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5_E1[i].name) {
		if (WTBL_LMAC_DW5_E1[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW5_E1[i].name,
					 (dw_value & WTBL_LMAC_DW5_E1[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW5_E1[i].name,
					  (dw_value & WTBL_LMAC_DW5_E1[i].mask) >> WTBL_LMAC_DW5_E1[i].shift));
		if (WTBL_LMAC_DW5_E1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW28_E1[] = {
    {"USER_RSSI",                   WTBL_USER_RSSI_MASK,            WTBL_USER_RSSI_OFFSET,	FALSE},
    {"USER_SNR",                    WTBL_USER_SNR_MASK,             WTBL_USER_SNR_OFFSET,	FALSE},
    {"RAPID_REACTION_RATE",         WTBL_RAPID_REACTION_RATE_MASK,  WTBL_RAPID_REACTION_RATE_OFFSET,	TRUE},
    {"HT_AMSDU(Read Only)",         WTBL_HT_AMSDU,                  NO_SHIFT_DEFINE,	FALSE},
    {"AMSDU_CROSS_LG(Read Only)",   WTBL_AMSDU_CROSS_LG,            NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW28_E1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 28 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 28\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28_E1[i].name) {
		if (WTBL_LMAC_DW28_E1[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW28_E1[i].name,
					 (dw_value & WTBL_LMAC_DW28_E1[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW28_E1[i].name,
					  (dw_value & WTBL_LMAC_DW28_E1[i].mask) >> WTBL_LMAC_DW28_E1[i].shift));

		if (WTBL_LMAC_DW28_E1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW29_E1[] = {
	{"RCPI 0",	WTBL_RESP_RCPI0_MASK,		WTBL_RESP_RCPI0_OFFSET,	FALSE},
	{"RCPI 1",	WTBL_RESP_RCPI1_MASK,		WTBL_RESP_RCPI1_OFFSET,	FALSE},
	{"RCPI 2",	WTBL_RESP_RCPI2_MASK,		WTBL_RESP_RCPI2_OFFSET,	FALSE},
	{"RCPI 3",	WTBL_RESP_RCPI3_MASK,		WTBL_RESP_RCPI3_OFFSET,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW29_E1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 29 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 29\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW29_E1[i].name) {
		if (WTBL_LMAC_DW29_E1[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW29_E1[i].name,
					 (dw_value & WTBL_LMAC_DW29_E1[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW29_E1[i].name,
					  (dw_value & WTBL_LMAC_DW29_E1[i].mask) >> WTBL_LMAC_DW29_E1[i].shift));

		if (WTBL_LMAC_DW29_E1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW30_E1[] = {
	{"SNR 0",	WTBL_SNR_RX0_MASK,		WTBL_SNR_RX0_OFFSET,	FALSE},
	{"SNR 1",	WTBL_SNR_RX1_MASK,		WTBL_SNR_RX1_OFFSET,	FALSE},
	{"SNR 2",	WTBL_SNR_RX2_MASK,		WTBL_SNR_RX2_OFFSET,	FALSE},
	{"SNR 3",	WTBL_SNR_RX3_MASK,		WTBL_SNR_RX3_OFFSET,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW30_E1(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 30 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 30\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30_E1[i].name) {
		if (WTBL_LMAC_DW30_E1[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW30_E1[i].name,
					 (dw_value & WTBL_LMAC_DW30_E1[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW30_E1[i].name,
					  (dw_value & WTBL_LMAC_DW30_E1[i].mask) >> WTBL_LMAC_DW30_E1[i].shift));
		if (WTBL_LMAC_DW30_E1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW28[] = {
    {"OM_INFO",                     WTBL_OM_INFO_MASK,              WTBL_OM_INFO_OFFSET,	FALSE},
    {"OM_RXD_DUP_MODE",             WTBL_OM_RXD_DUP_MODE,           NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW28(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 28 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 28\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28[i].name) {
		if (WTBL_LMAC_DW28[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW28[i].name,
					 (dw_value & WTBL_LMAC_DW28[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW28[i].name,
					  (dw_value & WTBL_LMAC_DW28[i].mask) >> WTBL_LMAC_DW28[i].shift));

		if (WTBL_LMAC_DW28[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW29[] = {
    {"USER_RSSI",                   WTBL_USER_RSSI_MASK,            WTBL_USER_RSSI_OFFSET,	FALSE},
    {"USER_SNR",                    WTBL_USER_SNR_MASK,             WTBL_USER_SNR_OFFSET,	FALSE},
    {"RAPID_REACTION_RATE",         WTBL_RAPID_REACTION_RATE_MASK,  WTBL_RAPID_REACTION_RATE_OFFSET,	TRUE},
    {"HT_AMSDU(Read Only)",         WTBL_HT_AMSDU,                  NO_SHIFT_DEFINE,	FALSE},
    {"AMSDU_CROSS_LG(Read Only)",   WTBL_AMSDU_CROSS_LG,            NO_SHIFT_DEFINE,	TRUE},
    {NULL,}
};

static VOID parse_fmac_lwtbl_DW29(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 29 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 29\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW29[i].name) {
		if (WTBL_LMAC_DW29[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW29[i].name,
					 (dw_value & WTBL_LMAC_DW29[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW29[i].name,
					  (dw_value & WTBL_LMAC_DW29[i].mask) >> WTBL_LMAC_DW29[i].shift));

		if (WTBL_LMAC_DW29[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW30[] = {
	{"RCPI 0",	WTBL_RESP_RCPI0_MASK,		WTBL_RESP_RCPI0_OFFSET,	FALSE},
	{"RCPI 1",	WTBL_RESP_RCPI1_MASK,		WTBL_RESP_RCPI1_OFFSET,	FALSE},
	{"RCPI 2",	WTBL_RESP_RCPI2_MASK,		WTBL_RESP_RCPI2_OFFSET,	FALSE},
	{"RCPI 3",	WTBL_RESP_RCPI3_MASK,		WTBL_RESP_RCPI3_OFFSET,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW30(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 30 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 30\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30[i].name) {
		if (WTBL_LMAC_DW30[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW30[i].name,
					 (dw_value & WTBL_LMAC_DW30[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW30[i].name,
					  (dw_value & WTBL_LMAC_DW30[i].mask) >> WTBL_LMAC_DW30[i].shift));

		if (WTBL_LMAC_DW30[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_LMAC_DW31[] = {
	{"RCPI 4",	WTBL_RESP_RCPI4_MASK,		WTBL_RESP_RCPI4_OFFSET,	FALSE},
	{"RCPI 5",	WTBL_RESP_RCPI5_MASK,		WTBL_RESP_RCPI5_OFFSET,	FALSE},
	{"RCPI 6",	WTBL_RESP_RCPI6_MASK,		WTBL_RESP_RCPI6_OFFSET,	FALSE},
	{"RCPI 7",	WTBL_RESP_RCPI4_MASK,		WTBL_RESP_RCPI7_OFFSET,	TRUE},
	{NULL,}
};

static VOID parse_fmac_lwtbl_DW31(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 31 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nLWTBL DW 31\n\t"));
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_4*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW31[i].name) {
		if (WTBL_LMAC_DW31[i].shift == NO_SHIFT_DEFINE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%d", WTBL_LMAC_DW31[i].name,
					 (dw_value & WTBL_LMAC_DW31[i].mask) ? 1 : 0));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s:%u", WTBL_LMAC_DW31[i].name,
					  (dw_value & WTBL_LMAC_DW31[i].mask) >> WTBL_LMAC_DW31[i].shift));
		if (WTBL_LMAC_DW31[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static VOID parse_fmac_lwtbl_rx_stats(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	if (MTK_REV_ET(pAd, MT7915, MT7915E1)) {
		parse_fmac_lwtbl_DW28_E1(pAd, lwtbl);
		parse_fmac_lwtbl_DW29_E1(pAd, lwtbl);
		parse_fmac_lwtbl_DW30_E1(pAd, lwtbl);
	} else {
		parse_fmac_lwtbl_DW28(pAd, lwtbl);
		parse_fmac_lwtbl_DW29(pAd, lwtbl);
		parse_fmac_lwtbl_DW30(pAd, lwtbl);
		parse_fmac_lwtbl_DW31(pAd, lwtbl);
	}
}

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW0[] = {
	{"PN0",		WTBL_PN0_MASK,		WTBL_PN0_OFFSET,	FALSE},
	{"PN1",		WTBL_PN1_MASK,		WTBL_PN1_OFFSET,	FALSE},
	{"PN2",		WTBL_PN2_MASK,		WTBL_PN2_OFFSET,	TRUE},
	{"PN3",		WTBL_PN3_MASK,		WTBL_PN3_OFFSET,	FALSE},
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	UINT32 mask;
	UINT32 shift;
	UCHAR new_line;
} WTBL_UMAC_DW1[] = {
    {"PN4",     WTBL_PN4_MASK,      WTBL_PN4_OFFSET,	FALSE},
    {"PN5",     WTBL_PN5_MASK,      WTBL_PN5_OFFSET,	TRUE},
    {NULL,}
};

static VOID parse_fmac_uwtbl_pn(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* UMAC WTBL DW 0 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nUWTBL PN\n\t"));

	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_DW_0*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW0[i].name) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:%u", WTBL_UMAC_DW0[i].name,
				  (dw_value & WTBL_UMAC_DW0[i].mask) >> WTBL_UMAC_DW0[i].shift));
		if (WTBL_UMAC_DW0[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}

	i = 0;
	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW1[i].name) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:%u", WTBL_UMAC_DW1[i].name,
				  (dw_value & WTBL_UMAC_DW1[i].mask) >> WTBL_UMAC_DW1[i].shift));
		if (WTBL_UMAC_DW1[i].new_line)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t"));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("/ "));
		i++;
	}
}

static VOID parse_fmac_uwtbl_sn(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 u2SN = 0;

	/* UMAC WTBL DW SN part */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nUWTBL SN\n"));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID0_AC0_SN_MASK) >> WTBL_TID0_AC0_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID0_AC0_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID1_AC1_SN_MASK) >> WTBL_TID1_AC1_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID1_AC1_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID2_AC2_SN_0_7_MASK) >> WTBL_TID2_AC2_SN_0_7_OFFSET;
	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN |= (((*addr) & WTBL_TID2_AC2_SN_8_11_MASK) >> WTBL_TID2_AC2_SN_8_11_OFFSET) << 8;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID2_AC2_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID3_AC3_SN_MASK) >> WTBL_TID3_AC3_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID3_AC3_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID4_SN_MASK) >> WTBL_TID4_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID4_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID5_SN_0_3_MASK) >> WTBL_TID5_SN_0_3_OFFSET;
	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN |= (((*addr) & WTBL_TID5_SN_4_11_MASK) >> WTBL_TID5_SN_4_11_OFFSET) << 4;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID5_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID6_SN_MASK) >> WTBL_TID6_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID6_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID7_SN_MASK) >> WTBL_TID7_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "TID6_SN", u2SN));

	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	u2SN = ((*addr) & WTBL_COM_SN_MASK) >> WTBL_COM_SN_OFFSET;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t%s:%u\n", "COM_SN", u2SN));
}

static VOID dump_key_table(RTMP_ADAPTER *pAd, UINT16 keyloc0, UINT16 keyloc1)
{
	UINT8 keytbl[ONE_KEY_ENTRY_LEN_IN_DW*4] = {0};
	UINT16 x;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\n\t%s:%d\n", "keyloc0", keyloc0));
	if (keyloc0 != (WTBL_KEY_LINK_DW_KEY_LOC0_MASK >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET)) {

		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(pAd, keyloc0, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc0, 0)));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]));
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%d\n", "keyloc1", keyloc1));
	if (keyloc1 != (WTBL_KEY_LINK_DW_KEY_LOC1_MASK >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET)) {
		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(pAd, keyloc1, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc1, 0)));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]));
		}
	}
}

static VOID parse_fmac_uwtbl_others(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_32 amsdu_len = 0;

	/* UMAC WTBL DW 0 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nUWTBL others\n"));

	addr = (UINT_32 *)&(uwtbl[UWTBL_KEY_LINK_DW*4]);
	dw_value = *addr;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%lu/%lu\n", "Key Loc 1/2",
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC0_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET,
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC1_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET
					));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%d\n", "UWTBL_QOS",
					(dw_value & WTBL_QOS_MASK) ? 1 : 0
					));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%d\n", "UWTBL_HT_VHT_HE",
					(dw_value & WTBL_HT_VHT_HE_MASK) ? 1 : 0
					));

	addr = (UINT_32 *)&(uwtbl[UWTBL_HW_AMSDU_DW*4]);
	dw_value = *addr;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%d\n", "HW AMSDU Enable",
					(dw_value & WTBL_AMSDU_EN_MASK) ? 1 : 0
					));

	amsdu_len = (dw_value & WTBL_AMSDU_LEN_MASK) >> WTBL_AMSDU_LEN_OFFSET;
	if (amsdu_len == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t%s:invalid (WTBL value=0x%x)\n", "HW AMSDU Len",
						amsdu_len
						));
	else if (amsdu_len == 1)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						1,
						255,
						amsdu_len
						));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						256 * (amsdu_len - 1),
						256 * (amsdu_len - 1) + 255,
						amsdu_len
						));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t%s:%lu (WTBL value=0x%lx)\n", "HW AMSDU Num",
					((dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET) + 1,
					(dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET
					));

	/* Parse KEY link */
	addr = (UINT_32 *)&(uwtbl[UWTBL_KEY_LINK_DW*4]);
	dw_value = *addr;
	dump_key_table(pAd,
			(dw_value & WTBL_KEY_LINK_DW_KEY_LOC0_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET,
			(dw_value & WTBL_KEY_LINK_DW_KEY_LOC1_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET
			);
}


static VOID dump_fmac_wtbl_info(RTMP_ADAPTER *pAd, UINT8 *lwtbl, UINT8 *uwtbl)
{
	parse_fmac_lwtbl_DW0_1(pAd, lwtbl);
	parse_fmac_lwtbl_DW2(pAd, lwtbl);
	if (MTK_REV_ET(pAd, MT7915, MT7915E1))
		parse_fmac_lwtbl_DW3_E1(pAd, lwtbl);
	else
		parse_fmac_lwtbl_DW3(pAd, lwtbl);
	parse_fmac_lwtbl_DW4(pAd, lwtbl);
	if (MTK_REV_ET(pAd, MT7915, MT7915E1))
		parse_fmac_lwtbl_DW5_E1(pAd, lwtbl);
	else
		parse_fmac_lwtbl_DW5(pAd, lwtbl);
	parse_fmac_lwtbl_DW6(pAd, lwtbl);
	parse_fmac_lwtbl_DW7(pAd, lwtbl);
	parse_fmac_lwtbl_DW8(pAd, lwtbl);
	parse_fmac_lwtbl_DW9(pAd, lwtbl);
	parse_fmac_lwtbl_DW10(pAd, lwtbl);
	parse_fmac_lwtbl_DW11(pAd, lwtbl);
	parse_fmac_lwtbl_DW12(pAd, lwtbl);
	parse_fmac_lwtbl_DW13(pAd, lwtbl);
	parse_fmac_lwtbl_rx_stats(pAd, lwtbl);

	parse_fmac_uwtbl_pn(pAd, uwtbl);
	parse_fmac_uwtbl_sn(pAd, uwtbl);
	parse_fmac_uwtbl_others(pAd, uwtbl);

}

static VOID chip_dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Basic Info:\n"));
}

static VOID chip_dump_wtbl_info(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	UINT8 lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	UINT8 uwtbl[UWTBL_LEN_IN_DW*4] = {0};
	int x;
	UINT8 real_lwtbl_size = 0;

	if (MT_REV_ET(pAd, MT7915, MT7915E1))
		real_lwtbl_size = LWTBL_LEN_IN_DW_E1;
	else
		real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_WTBLON_TOP_WDUCR_ADDR,
								IO_R_32(WF_WTBLON_TOP_WDUCR_ADDR),
								LWTBL_IDX2BASE(wtbl_idx, 0)));
	for (x = 0; x < real_lwtbl_size; x++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: %02x %02x %02x %02x\n",
									x,
									lwtbl[x * 4 + 3],
									lwtbl[x * 4 + 2],
									lwtbl[x * 4 + 1],
									lwtbl[x * 4]));
	}

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW, uwtbl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("UMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_UWTBL_TOP_WDUCR_ADDR,
								IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
								UWTBL_IDX2BASE(wtbl_idx, 0)));
	for (x = 0; x < UWTBL_LEN_IN_DW; x++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: %02x %02x %02x %02x\n",
									x,
									uwtbl[x * 4 + 3],
									uwtbl[x * 4 + 2],
									uwtbl[x * 4 + 1],
									uwtbl[x * 4]));
	}

	dump_fmac_wtbl_info(pAd, lwtbl, uwtbl);
}

static VOID chip_dump_wtbl_mac(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	UINT8 lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	UINT8 real_lwtbl_size = 0;

	if (MT_REV_ET(pAd, MT7915, MT7915E1))
		real_lwtbl_size = LWTBL_LEN_IN_DW_E1;
	else
		real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("WLAN_IDX: %d Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  wtbl_idx, lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]));
}


static VOID chip_set_hw_amsdu(RTMP_ADAPTER *pAd, UINT32 wcid, UINT8 num, UINT32 len)
{
	UINT32 hw_amsdu_cfg;
	halWtblReadRaw(pAd, wcid, WTBL_TYPE_UMAC, UWTBL_HW_AMSDU_DW, 1, &hw_amsdu_cfg);

	if (len) {
		hw_amsdu_cfg &= ~WTBL_AMSDU_LEN_MASK;
		hw_amsdu_cfg |= (len << WTBL_AMSDU_LEN_OFFSET);
	}

	hw_amsdu_cfg &= ~WTBL_AMSDU_NUM_MASK;
	hw_amsdu_cfg |= (num << WTBL_AMSDU_NUM_OFFSET);

	halWtblWriteRaw(pAd, wcid, WTBL_TYPE_UMAC, UWTBL_HW_AMSDU_DW, hw_amsdu_cfg);
}

static VOID chip_set_header_translation(RTMP_ADAPTER *pAd, UINT32 wcid, BOOLEAN on)
{
	UINT32 dw;

	halWtblReadRaw(pAd, wcid, WTBL_TYPE_LMAC, 4, 1, &dw);

	if (on) {
		dw &= ~WTBL_DIS_RHTR;
	} else {
		dw |= WTBL_DIS_RHTR;
	}

	halWtblWriteRaw(pAd, wcid, WTBL_TYPE_LMAC, 4, dw);
}

static VOID dump_dma_tx_ring_info(struct hdev_ctrl *ctrl, char *s, UINT32 ring_base)
{
	UINT32 base = 0, cnt = 0, cidx = 0, didx = 0, queue_cnt;

	/* use RTMP_IO because addr need to be lookup */
	RTMP_IO_READ32(ctrl, ring_base, &base);
	RTMP_IO_READ32(ctrl, ring_base + 4, &cnt);
	RTMP_IO_READ32(ctrl, ring_base + 8, &cidx);
	RTMP_IO_READ32(ctrl, ring_base + 12, &didx);
	queue_cnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + cnt);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%20s %10x %10x %10x %10x %10x\n", s, base, cnt, cidx, didx, queue_cnt));

}

static VOID dump_dma_rx_ring_info(struct hdev_ctrl *ctrl, char *s, UINT32 ring_base)
{
	UINT32 base = 0, cnt = 0, cidx = 0, didx = 0, queue_cnt;

	/* use RTMP_IO because addr need to be lookup */
	RTMP_IO_READ32(ctrl, ring_base, &base);
	RTMP_IO_READ32(ctrl, ring_base + 4, &cnt);
	RTMP_IO_READ32(ctrl, ring_base + 8, &cidx);
	RTMP_IO_READ32(ctrl, ring_base + 12, &didx);
	queue_cnt = (didx > cidx) ? (didx - cidx - 1) : (didx - cidx + cnt - 1);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%20s %10x %10x %10x %10x %10x\n", s, base, cnt, cidx, didx, queue_cnt));

}

static VOID chip_show_dma_info(struct hdev_ctrl *ctrl)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);

	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd)) {
		UINT32 sys_ctrl[10] = {0};

		/* HOST DMA */
		HIF_IO_READ32(ctrl, MT_INT_SOURCE_CSR, &sys_ctrl[0]);
		HIF_IO_READ32(ctrl, MT_INT_MASK_CSR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[2]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[3]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_STA_ADDR, &sys_ctrl[4]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_ENA_ADDR, &sys_ctrl[5]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[6]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[7]);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HOST_DMA Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x\n",
				"Merge", sys_ctrl[0], sys_ctrl[1]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA0", sys_ctrl[2], sys_ctrl[3], sys_ctrl[6],
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA1", sys_ctrl[4], sys_ctrl[5], sys_ctrl[7],
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));

		HIF_IO_READ32(ctrl, MT_INT1_SOURCE_CSR, &sys_ctrl[0]);
		HIF_IO_READ32(ctrl, MT_INT1_MASK_CSR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_ADDR, &sys_ctrl[2]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_ENA_ADDR, &sys_ctrl[3]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_STA_ADDR, &sys_ctrl[4]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_ENA_ADDR, &sys_ctrl[5]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[6]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[7]);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x\n",
				"MergeP1", sys_ctrl[0], sys_ctrl[1]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA0P1", sys_ctrl[2], sys_ctrl[3], sys_ctrl[6],
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[6] & WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA1P1", sys_ctrl[4], sys_ctrl[5], sys_ctrl[7],
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[7] & WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HOST_DMA0 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_rx_ring_info(ctrl, "R0:Data0(MAC2H)", 	WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Data1(MAC2H)",	WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HOST_DMA0 PCIe 1 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_rx_ring_info(ctrl, "R1:Data1(MAC2H)",	WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING1_CTRL0_ADDR);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HOST_DMA1 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T16:FWDL",				WF_WFDMA_HOST_DMA1_WPDMA_TX_RING16_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T17:Cmd(H2WM)",		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING17_CTRL0_ADDR);
#ifdef WFDMA_WED_COMPATIBLE
		dump_dma_tx_ring_info(ctrl, "T18:TXD0(H2WA)",		WF_WFDMA_EXT_WRAP_CSR_WED_TX0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T19:TXD1(H2WA)",		WF_WFDMA_EXT_WRAP_CSR_WED_TX1_CTRL0_ADDR);
#else
		dump_dma_tx_ring_info(ctrl, "T18:TXD0(H2WA)",		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING18_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T19:TXD1(H2WA)",		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING19_CTRL0_ADDR);
#endif
		dump_dma_tx_ring_info(ctrl, "T20:Cmd(H2WA)",		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING20_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:Event(WM2H)",		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING0_CTRL0_ADDR);
#ifdef WFDMA_WED_COMPATIBLE
		dump_dma_rx_ring_info(ctrl, "R1:Event0(WA2H)",		WF_WFDMA_EXT_WRAP_CSR_WED_RX1_CTRL0_ADDR);
#else
		dump_dma_rx_ring_info(ctrl, "R1:Event0(WA2H)",		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING1_CTRL0_ADDR);
#endif
		dump_dma_rx_ring_info(ctrl, "R2:Event1(WA2H)",		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING2_CTRL0_ADDR);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HOST_DMA1 PCIe 1 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T19:TXD1(H2WA)",		WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_TX_RING19_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:Event1(WA2H)",		WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_RX_RING2_CTRL0_ADDR);

		/* MCU DMA information */
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[3]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_HOST_INT_STA_ADDR, &sys_ctrl[4]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_HOST_INT_ENA_ADDR, &sys_ctrl[5]);

		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, &sys_ctrl[6]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_PCIE1_HOST_INT_STA_ADDR, &sys_ctrl[7]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA1_PCIE1_HOST_INT_ENA_ADDR, &sys_ctrl[8]);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU_DMA Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA0", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA1", sys_ctrl[4], sys_ctrl[5], sys_ctrl[3],
				(sys_ctrl[3] & WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[3] & WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[3] & WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[3] & WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA1P1", sys_ctrl[7], sys_ctrl[8], sys_ctrl[6],
				(sys_ctrl[6] & WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[6] & WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[6] & WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU_DMA0 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T0:TXD(WM2MAC)",		WF_WFDMA_MCU_DMA0_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:TXCMD(WM2MAC)", 	WF_WFDMA_MCU_DMA0_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T2:TXD(WA2MAC)", 		WF_WFDMA_MCU_DMA0_WPDMA_TX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:Data(MAC2WM)", 		WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:TxDone(MAC2WM)",	WF_WFDMA_MCU_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:SPL(MAC2WM)", 		WF_WFDMA_MCU_DMA0_WPDMA_RX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TxDone(MAC2WA)",	WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:TXS(MAC2WA)", 		WF_WFDMA_MCU_DMA0_WPDMA_RX_RING4_CTRL0_ADDR);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU_DMA1 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T0:Event(WM2H)",		WF_WFDMA_MCU_DMA1_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:Event0(WA2H)",		WF_WFDMA_MCU_DMA1_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T2:Event1(WA2H)",		WF_WFDMA_MCU_DMA1_WPDMA_TX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:FWDL",				WF_WFDMA_MCU_DMA1_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Cmd(H2WM)", 		WF_WFDMA_MCU_DMA1_WPDMA_RX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:TXD0(H2WA)", 		WF_WFDMA_MCU_DMA1_WPDMA_RX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TXD1(H2WA)", 		WF_WFDMA_MCU_DMA1_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:Cmd(H2WA)", 		WF_WFDMA_MCU_DMA1_WPDMA_RX_RING4_CTRL0_ADDR);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU_DMA1 PCIe 1 Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T2:Event1(WA2H)",		WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_TX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TXD1(H2WA)", 		WF_WFDMA_MCU_DMA1_PCIE1_WPDMA_RX_RING3_CTRL0_ADDR);

		/* MEM DMA information */
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MEM_DMA Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"MEM", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MEM_DMA Ring Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%20s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt"));
		dump_dma_tx_ring_info(ctrl, "T0:CmdEvent(WM2WA)",	WF_WFDMA_MEM_DMA_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:CmdEvent(WA2WM)",	WF_WFDMA_MEM_DMA_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:CmdEvent(WM2WA)",	WF_WFDMA_MEM_DMA_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:CmdEvent(WA2WM)",	WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR);
	}
}

UINT32 chip_get_lpon_frcr(RTMP_ADAPTER *pAd)
{
	UINT32 free_cnt = 0;

	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_FRCR_ADDR, &free_cnt);
	return free_cnt;
}

#ifdef VOW_SUPPORT
UINT32 chip_get_sta_airtime(RTMP_ADAPTER *pAd, UINT16 sta, UINT16 ac, BOOLEAN tx)
{
	UINT32 airtime;
	UINT32 wtbl_offset = 20;

	if (tx)
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1), 1, &airtime);
	else
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1) + 1, 1, &airtime);

	return airtime;
}

UINT32 chip_get_sta_addr(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 addr;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 0, 1, &addr);
	return addr;
}

UINT32 chip_get_sta_rate(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 rate;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 10, 1, &rate);
	return rate;
}

UINT32 chip_get_sta_tx_cnt(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 bw)
{
	UINT32 tx_cnt;
	UINT32 wtbl_offset = 16;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + bw, 1, &tx_cnt);
	return tx_cnt;
}

INT32 chip_set_sta_psm(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 psm)
{
	UINT32 reg_val;
	UINT32 mask = WTBL_PSM;
	UINT32 field = 0;
	UINT32 cnt = 0;
	UINT8 dw = WTBL_GROUP_TRX_CAP_DW_5;

	if (psm)
		field = mask;

	IO_W_32(WF_WTBLON_TOP_WMUMR_ADDR, mask);
	IO_W_32(WF_WTBLON_TOP_WMUDR_ADDR, field);
	/* Set WTBL ID */
	reg_val = (sta << WF_WTBLON_TOP_WIUCR_WLAN_IDX_SHFT) & WF_WTBLON_TOP_WIUCR_WLAN_IDX_MASK;
	/* Set DW */
	reg_val |= (dw << WF_WTBLON_TOP_WIUCR_DW_SHFT) & WF_WTBLON_TOP_WIUCR_DW_MASK;
	/* Trigger it */
	reg_val |= WF_WTBLON_TOP_WIUCR_MASK_UPDATE_MASK;
	IO_W_32(WF_WTBLON_TOP_WIUCR_ADDR, reg_val);

	/* Wait access complete */
	do {
		reg_val = IO_R_32(WF_WTBLON_TOP_WIUCR_ADDR);
		cnt++;

		if (cnt > WTBL_MASK_UPDATE_MAX_RETRY)
			break;

		RtmpusecDelay(1);
	} while (reg_val & WF_WTBLON_TOP_WIUCR_IU_BUSY_MASK);

	if (cnt > WTBL_MASK_UPDATE_MAX_RETRY)
		return FALSE;

	return TRUE;
}

static VOID chip_get_obss_nonwifi_airtime(RTMP_ADAPTER *pAd, UINT32 *at_info)
{
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME13_RX0_AIRTIME_NONWIFI_ADDR, &at_info[0]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME13_RX0_AIRTIME_NONWIFI_ADDR + (1 << 16), &at_info[1]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME14_RX0_AIRTIME_OBSS_ADDR, &at_info[2]);
	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_AIRTIME14_RX0_AIRTIME_OBSS_ADDR + (1 << 16), &at_info[3]);
}

#endif	/* VOW_SUPPORT */

static struct {
	RTMP_STRING *name;
	UINT32 reg_addr;
	UINT32 mask;
	UINT32 shift;
} TXV[] = {
	/* CG1 DDW1 H */
	{"ant id",			BN0_WF_TMAC_TOP_DBGR5_ADDR,		BITS(8, 31),	8},
	/* CG1 DDW1 L */
	{"user count",		BN0_WF_TMAC_TOP_DBGR4_ADDR,		BITS(24, 30),	24},
	{"phy mode",		BN0_WF_TMAC_TOP_DBGR4_ADDR,		BITS(12, 15),	12},
	{"dbw",				BN0_WF_TMAC_TOP_DBGR4_ADDR,		BITS(8, 10),	8},
	{"stbc",			BN0_WF_TMAC_TOP_DBGR4_ADDR,		BITS(6, 7),		6},
	{"mu",				BN0_WF_TMAC_TOP_DBGR4_ADDR,		BIT(5),			5},
	{"spe idx",			BN0_WF_TMAC_TOP_DBGR4_ADDR,		BITS(0, 4),		0},
	/* CG1 DDW2 H */
	{"format",			BN0_WF_TMAC_TOP_DBGR7_ADDR,		BIT(30),		30},
	{"HE LTF",			BN0_WF_TMAC_TOP_DBGR7_ADDR,		BITS(28, 29),	28},
	{"HE GI",			BN0_WF_TMAC_TOP_DBGR7_ADDR,		BITS(26, 27),	26},
	{"UDL",				BN0_WF_TMAC_TOP_DBGR7_ADDR,		BIT(25),		25},
	{"Max TX NSTS",		BN0_WF_TMAC_TOP_DBGR7_ADDR,		BITS(12, 14),	12},
	/* CG1 DDW2 L */
	{"SIGB Comp.",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BIT(31),		31},
	{"SIGB Nsym.",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(24, 30),	24},
	{"SIGB DCM",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BIT(23),		23},
	{"SIGB MCS",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(20, 22),	20},
	{"MIMO user",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(16, 19),	16},
	{"Center 26",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(14, 15),	14},
	{"CH1 STAs",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(8, 13),	8},
	{"CH0 STAs",		BN0_WF_TMAC_TOP_DBGR6_ADDR,		BITS(0, 5),		0},
	/* CG2 DDW1 H */
	{"LG LEN EXTS",		BN0_WF_TMAC_TOP_DBGR9_ADDR,		BITS(20, 31),	20},
	{"PE Disamb. EXT",	BN0_WF_TMAC_TOP_DBGR9_ADDR,		BIT(15),		15},
	{"afactor",			BN0_WF_TMAC_TOP_DBGR9_ADDR,		BITS(6, 7),		6},
	/* CG2 DDW1 L */
	{"PE Disamb.",		BN0_WF_TMAC_TOP_DBGR8_ADDR,		BIT(31),		31},
	{"LTF Symbols",		BN0_WF_TMAC_TOP_DBGR8_ADDR,		BITS(28, 30),	28},
	{"Symbols count",	BN0_WF_TMAC_TOP_DBGR8_ADDR,		BITS(16, 27),	16},
	{"Max TPE",			BN0_WF_TMAC_TOP_DBGR8_ADDR,		BITS(13, 14),	13},
	{"LDPC Extr Sym",	BN0_WF_TMAC_TOP_DBGR8_ADDR,		BIT(12),		12},
	{"LG LEN",			BN0_WF_TMAC_TOP_DBGR8_ADDR,		BITS(0, 11),	0},
	/* CG2 DDW2 H, BN0_WF_TMAC_TOP_DBGR11_ADDR */
	/* CG2 DDW2 L, BN0_WF_TMAC_TOP_DBGR10_ADDR */
	/* PG1 DDW1 H */
	{"MU GROUP",		BN0_WF_TMAC_TOP_DBGR13_ADDR,	BITS(24, 28),	24},
	{"IBF MU IDX",		BN0_WF_TMAC_TOP_DBGR13_ADDR,	BITS(0, 7),		0},
	/* PG1 DDW1 L */
	{"EBF MU IDX",		BN0_WF_TMAC_TOP_DBGR12_ADDR,	BITS(24, 31),	24},
	{"RU allocation",	BN0_WF_TMAC_TOP_DBGR12_ADDR,	BITS(16, 23),	16},
	{"FEC Coding",		BN0_WF_TMAC_TOP_DBGR12_ADDR,	BIT(15),		15},
	{"NSTS",			BN0_WF_TMAC_TOP_DBGR12_ADDR,	BITS(8, 10),	8},
	{"ER-106T",			BN0_WF_TMAC_TOP_DBGR12_ADDR,	BIT(5),			5},
	{"DCM",				BN0_WF_TMAC_TOP_DBGR12_ADDR,	BIT(4),			4},
	{"Rate",			BN0_WF_TMAC_TOP_DBGR12_ADDR,	BITS(0, 3),		0},
	/* PG2 DDW1 H */
	{"AID",				BN0_WF_TMAC_TOP_DBGR15_ADDR,	BITS(0, 10),	0},
	/* PG2 DDW1 L */
	{"TX LEN",			BN0_WF_TMAC_TOP_DBGR14_ADDR,	BITS(0, 22),	0},
	{0}
};
static UCHAR dump_txv_CR(IN struct hdev_ctrl *ctrl, IN UINT32 reg_addr, OUT PUINT32 reg_val)
{
	UCHAR valid = TRUE;

	*reg_val = 0;

	MAC_IO_READ32(ctrl, reg_addr, reg_val);

	return valid;
}

static INT32 chip_check_txv(IN struct hdev_ctrl *ctrl, IN UCHAR *name, IN UINT32 value)
{
	UCHAR found = 0, txv_idx = 0;
	UINT32 reg_val = 0;

	if (strlen(name) > 0) {
		while (TXV[txv_idx].name) {
			if (!strcmp(TXV[txv_idx].name, name)) {
				dump_txv_CR(ctrl, TXV[txv_idx].reg_addr, &reg_val);

				reg_val &= TXV[txv_idx].mask;
				reg_val >>= TXV[txv_idx].shift;

				found = 1;
				break;
			}

			txv_idx++;
		};

		if (found) {
			if (reg_val == value) {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Matched] %s = %d\n", __func__, TXV[txv_idx].name, value));
			} else {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Mis-matched] %s = (%d:%d)\n", __func__, TXV[txv_idx].name, reg_val, value));
			}
		} else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %s not found!\n", __func__, TXV[txv_idx].name));
		}
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown parameter name!\n", __func__));
	}

	return 0;
}

#ifdef CONFIG_ATE
static INT32 chip_ctrl_manual_hetb_tx(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 ctrl,
	UINT8 bw,
	UINT8 ltf_gi,
	UINT8 stbc,
	struct _ATE_RU_STA *ru_sta)
{
	UINT32 cr_value = 0;
	union hetb_rx_cmm cmm;
	union hetb_tx_usr usr;
	UINT32 nss;

	if (ctrl == HETB_TX_CFG) {
		if (ru_sta == NULL) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s[%d]: invalid input\n", __func__, __LINE__));

			goto err_out;
		}
		/* setup MAC start */
		/* step 1, common info of TF */
		os_zero_mem(&cmm, sizeof(cmm));
		cmm.field.sig_a_reserved = 0x1ff;
		cmm.field.ul_length = ru_sta->l_len;
		cmm.field.t_pe = (ru_sta->afactor_init & 0x3) | ((ru_sta->pe_disamb & 0x1) << 2);
		cmm.field.ldpc_extra_sym = ru_sta->ldpc_extr_sym;
		nss = (ru_sta->ru_mu_nss > ru_sta->nss) ? ru_sta->ru_mu_nss : ru_sta->nss;
		if (ru_sta->ru_mu_nss > ru_sta->nss)
			cmm.field.mimo_ltf = 1;
		if (stbc && nss == 1)
			cmm.field.ltf_sym_midiam = ltf_sym_code[nss+1];
		else
			cmm.field.ltf_sym_midiam = ltf_sym_code[nss];
		cmm.field.gi_ltf = ltf_gi;
		cmm.field.ul_bw = bw;
		cmm.field.stbc = stbc;
		cr_value = (cmm.cmm_info & 0xffffffff);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR0_TF_COMINFO_B31B0_ADDR+(0x10000*band_idx), cr_value);
		cr_value = ((cmm.cmm_info & 0xffffffff00000000) >> 32);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR1_TF_COMINFO_B63B32_ADDR+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step1: [CMM][%x][0x%llx]\n", __func__, BN0_WF_TMAC_TOP_TTRCR0_TF_COMINFO_B31B0_ADDR+(0x10000*band_idx), cmm.cmm_info));
		/* step 1, users info */
		usr.field.aid = 0x1;
		usr.field.allocation = ru_sta->ru_index;
		usr.field.coding = ru_sta->ldpc;
		usr.field.mcs = ru_sta->rate & ~BIT5;
		usr.field.dcm = (ru_sta->rate & BIT5) >> 4;
		usr.field.ss_allocation = ((nss-1) << 3) | (ru_sta->start_sp_st & 0x7);
		cr_value = usr.usr_info;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR2_TF_USRINFO_B31B0_ADDR+(0x10000*band_idx), cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_ADDR+(0x10000*band_idx), 0xef);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step1: [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR2_TF_USRINFO_B31B0_ADDR+(0x10000*band_idx), usr.usr_info));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_ADDR+(0x10000*band_idx), 0xef));
		/*  step 2, rssi report*/
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step2: [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK));
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [BWD][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK));
		/* step 3, channel information */
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), cr_value & ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step3: [CBW Mode][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK)));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & ~BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [CBW 160NC IND][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK)));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), &cr_value);
		cr_value |= ((TESTMODE_GET_PARAM(ad, band_idx, pri_sel) << BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_SHFT) & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        [CBW PRIM20 CH][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK)));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~(BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MASK | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MAN_MASK);
		cr_value |= BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MAN_MASK;
		cr_value &= ~BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_FCC_PWR_SKU2_MASK;
		cr_value |= (0x7f | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_FCC_PWR_SKU2_MAN_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step4: [TXPWR IND/SKU][%x][0x%04x]\n", __func__, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(0x10000*band_idx), cr_value));
		/* setup MAC end */
	} else if (ctrl == HETB_TX_START) {
		/*  step 6. Set 1 to TTRCR3.TF_RESP_TEST_MODE*/
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Step6: [TF_RESP_TEST_MODE][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK));
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Proactive HETB TX turned off][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK)));
	}

err_out:
	return 0;
}


static INT32 chip_ctrl_manual_hetb_rx(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	BOOLEAN start,
	UINT8 bw,
	UINT8 gi_ltf,
	UINT8 stbc,
	ULONGLONG csd,
	struct _ATE_RU_STA *pri_sta,
	struct _ATE_RU_STA *sta_list)
{
	ULONGLONG mac_cr_value = 0;
	UINT32 cr_value = 0, usr_grp_idx = 0, sta_idx = 0, nss;
	union hetb_rx_cmm cmm;
	union hetb_rx_usr usr;
	UINT32 *phy_rx_ctrl = NULL;

	if (IS_MT7915_FW_VER_E1(ad))
		phy_rx_ctrl = tbrx_phy_ctrl[0];
	else
		phy_rx_ctrl = tbrx_phy_ctrl[1];

	if (start) {
		/* setup MAC start */
		for (sta_idx = 0 ; sta_idx < 16 ; sta_idx++) {
			ULONGLONG ru_mac_coding = 0;
			UINT32 ru_idx = 0;

			if (sta_list[sta_idx].valid) {
				ru_idx = (sta_list[sta_idx].ru_index >> 1);

				if (ru_idx < 69) {
					if (ru_idx < 37)
						ru_mac_coding = 0x0;
					else if (ru_idx < 53)
						ru_mac_coding = 0x1;
					else if (ru_idx < 61)
						ru_mac_coding = 0x2;
					else if (ru_idx < 65)
						ru_mac_coding = 0x3;
					else if (ru_idx < 67)
						ru_mac_coding = 0x4;
					else if (ru_idx < 68)
						ru_mac_coding = 0x5;
					else
						ru_mac_coding = 0x6;
				} else
					ru_mac_coding = 0x7;
			} else
				ru_mac_coding = 0x0;

			mac_cr_value |= (ru_mac_coding << (sta_idx*3));
		}
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_RMAC_TOP_TF_USERTONE1_ADDR+(0x10000*band_idx), (((mac_cr_value & 0xffff00000000) >> 32) | BIT31));
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_RMAC_TOP_TF_USERTONE0_ADDR+(0x10000*band_idx), (mac_cr_value & 0xffffffff));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [MAC]0x%x=0x%llx\n", __func__, BN0_WF_RMAC_TOP_TF_USERTONE0_ADDR+(0x10000*band_idx), mac_cr_value));
		/* end MAC start */
		/* setup PHY start */
		/* cycle 0: start manual hetb rx (without TF) */
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x1;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Start]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		cr_value |= 0x2;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:        0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		/* cycle 1:CSD part */
		cr_value = (csd & 0xffffffff);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [CSD_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value));
		cr_value = (csd & 0xffffffff00000000) >> 32;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [CSD_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value));

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		/* cycle 2:common info of TF */
		os_zero_mem(&cmm, sizeof(cmm));
		cmm.field.sig_a_reserved = 0x1ff;
		cmm.field.ul_length = pri_sta->l_len;
		cmm.field.t_pe = (pri_sta->afactor_init & 0x3) | ((pri_sta->pe_disamb & 0x1) << 2);
		cmm.field.ldpc_extra_sym = pri_sta->ldpc_extr_sym;
		nss = (pri_sta->ru_mu_nss > pri_sta->nss) ? pri_sta->ru_mu_nss : pri_sta->nss;
		cmm.field.ltf_sym_midiam = ltf_sym_code[nss];
		cmm.field.gi_ltf = gi_ltf;
		cmm.field.ul_bw = bw;
		cmm.field.stbc = stbc;
		cr_value = cmm.cmm_info & 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [CMM_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), cr_value));
		cr_value = (cmm.cmm_info >> 32);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [CMM_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), cr_value));

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		/* cycle 3:users info */
		for (usr_grp_idx = 0 ; usr_grp_idx < 8 ; usr_grp_idx++) {
			os_zero_mem(&usr, sizeof(usr));

			if (sta_list[usr_grp_idx*2].valid) {
				usr.field.uid = usr_grp_idx*2;
				if (sta_list[usr_grp_idx*2].ru_mu_nss > sta_list[usr_grp_idx*2].nss)
					usr.field.nss = sta_list[usr_grp_idx*2].start_sp_st;	/* OFDMA:0, 1~7:MIMO; */
				else
					usr.field.nss = 0;	/* OFDMA:0, 1~7:MIMO; */
				usr.field.allocation = sta_list[usr_grp_idx*2].ru_index;
				usr.field.coding = sta_list[usr_grp_idx*2].ldpc;
				usr.field.mcs = sta_list[usr_grp_idx*2].rate & ~BIT5;
				usr.field.dcm = (sta_list[usr_grp_idx*2].rate & BIT5) >> 5;
				usr.field.ss_allocation = ((sta_list[usr_grp_idx*2].nss-1) << 3) | (sta_list[usr_grp_idx*2].start_sp_st & 0x7);
			} else
				usr.usr_info = 0xffffffff;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), usr.usr_info);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2, phy_rx_ctrl[2]+(0x10000*band_idx), usr.usr_info));

			os_zero_mem(&usr, sizeof(usr));
			if (sta_list[usr_grp_idx*2+1].valid) {
				usr.field.uid = usr_grp_idx*2+1;
				if (sta_list[usr_grp_idx*2+1].ru_mu_nss > sta_list[usr_grp_idx*2+1].nss)
					usr.field.nss = sta_list[usr_grp_idx*2+1].start_sp_st;	/* OFDMA:0, 1~7:MIMO; */
				else
					usr.field.nss = 0;	/* OFDMA:0, 1~7:MIMO; */
				usr.field.allocation = sta_list[usr_grp_idx*2+1].ru_index;
				usr.field.coding = sta_list[usr_grp_idx*2+1].ldpc;
				usr.field.mcs = sta_list[usr_grp_idx*2+1].rate & ~BIT5;
				usr.field.dcm = (sta_list[usr_grp_idx*2+1].rate & BIT5) >> 5;
				usr.field.ss_allocation = ((sta_list[usr_grp_idx*2+1].nss-1) << 3) | (sta_list[usr_grp_idx*2+1].start_sp_st & 0x7);
			} else
				usr.usr_info = 0xffffffff;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), usr.usr_info);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2+1, phy_rx_ctrl[1]+(0x10000*band_idx), usr.usr_info));

			RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
			cr_value |= 0x8;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
			cr_value &= 0xfffffff3;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		}

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value |= 0x4;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Submit]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		/* setup PHY end */
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), &cr_value);
		cr_value &= 0xfffffff0;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(0x10000*band_idx), 0);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(0x10000*band_idx), 0);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [Stop]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(0x10000*band_idx), cr_value));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(0x10000*band_idx), 0));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(0x10000*band_idx), 0));
	}

	return 0;
}

static INT32 chip_ctrl_asic_spe(RTMP_ADAPTER *ad,
											   UINT8 band_idx,
											   UINT8 tx_mode,
											   UINT8 spe_idx)
{
	UINT32 cr_value = 0;

	if (tx_mode == MODE_HE_TRIG) {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK;
		cr_value |= ((spe_idx << BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_SHFT) & BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Ste3.1:[SPE index][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK)));
	} else if (tx_mode == MODE_HE_MU && IS_MT7915_FW_VER_E1(ad)) {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_MASK;
		cr_value |= ((spe_idx << BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_SHFT) & BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_ADDR+(0x10000*band_idx), cr_value);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: [SPE index][%x][0x%04x]\n", __func__, BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_ADDR+(0x10000*band_idx), ((cr_value & BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_MASK) >> BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_SHFT)));
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), cr_value);
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_SPE_IDX_B0_ADDR+(0x10000*band_idx), cr_value);
	}

	return 0;
}
#endif

static UINT32 chip_show_asic_rx_stat(RTMP_ADAPTER *ad, UINT type)
{
	UINT32 value = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("mt7915: %s, Type(%d)\n", __func__, type));

	switch (type) {
	case HQA_RX_RESET_MAC_COUNT:
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR3_ADDR, &value);
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_ADDR, &value);
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_MIB_TOP_M0SDR3_ADDR, &value);
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_ADDR, &value);
		break;

	default:
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type(%d):%x\n", __func__, type, value));
	return value;
}

static INT32 chip_show_ple_info_by_idx(struct hdev_ctrl *ctrl, UINT16 wtbl_idx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_stat[ALL_CR_NUM_OF_ALL_AC + 1] = {0};
	UINT32 sta_pause[CR_NUM_OF_AC] = {0}, dis_sta_map[CR_NUM_OF_AC] = {0};
	struct wifi_dev *wdev = NULL;
	INT32 i, j;
	UINT32 wmmidx = 0;

	chip_get_ple_acq_stat(pAd, ple_stat);
	chip_get_dis_sta_map(pAd, dis_sta_map);
	chip_get_sta_pause(pAd, sta_pause);

	for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				UINT32 hfid, tfid, pktcnt, ac_num = j / CR_NUM_OF_AC, ctrl = 0;
				UINT32 sta_num = i + (j % CR_NUM_OF_AC) * 32, fl_que_ctrl[3] = {0};

				if (sta_num != wtbl_idx)
					continue;

				wdev = wdev_search_by_wcid(pAd, sta_num);

				if (wdev)
					wmmidx = HcGetWmmIdx(pAd, wdev);

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("\tSTA%d AC%d: ", sta_num, ac_num));

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR, &fl_que_ctrl[1]);
				HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR, &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt));

				if (((sta_pause[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" ctrl = %s", sta_ctrl_reg[ctrl]));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 (" (wmmidx=%d)\n", wmmidx));
			}
		}
	}

	return TRUE;
}

static UINT32 chip_get_tx_mibinfo(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UINT8 tx_mode, UINT8 dbw)
{
	UINT32 mac_val = 0, band_offset = 0;

	band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;

	if (tx_mode < MODE_HE_MU) {
		switch (dbw) {
		case BW_20:
			RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR0_ADDR + band_offset, &mac_val);
			mac_val &= BN0_WF_MIB_TOP_M0DR0_TX_20MHZ_CNT_MASK;
			break;
		case BW_40:
			RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR0_ADDR + band_offset, &mac_val);
			mac_val &= BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_MASK;
			mac_val >>= BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_SHFT;
			break;
		case BW_80:
			RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR1_ADDR + band_offset, &mac_val);
			mac_val &= BN0_WF_MIB_TOP_M0DR1_TX_80MHZ_CNT_MASK;
			break;
		case BW_160:
			RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR1_ADDR + band_offset, &mac_val);
			mac_val &= BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_MASK;
			mac_val >>= BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_SHFT;
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown DBW:%d\n", dbw));
		}
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR8_ADDR + band_offset, &mac_val);
	}

	return mac_val;
}

INT ChkExceptionType(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UCHAR exp_assert_proc_entry_cnt = 0;
	UCHAR exp_assert_state = 0;
	UINT32 macAddr = 0;

	macVal = 0;

	if (IS_MT7915_FW_VER_E2(pAd))
		macAddr = 0x219848;
	else
		macAddr = 0x21A870;

	HW_IO_READ32(pAd->hdev_ctrl, macAddr, &macVal);
	exp_assert_state = (macVal & 0xff);
	exp_assert_proc_entry_cnt = ((macVal >> 8) & 0xff);

	if (exp_assert_proc_entry_cnt == 0)
		return 0;
	else
		return 1;
}


#define WF_SW_DEF_DBG_CNT    WF_SW_DEF_CR_RSVD_DBG_6
#define WF_SW_DEF_DBG_CNT1    WF_SW_DEF_CR_RSVD_DBG_7
VOID ShowFwDbgCnt(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UINT32 inrtVal = 0;
	UINT32 cmd_cnt = 0;
	UINT32 event_cnt = 0;
	UINT32 msdu_miss_cnt = 0;
	UINT32 alloc_fail_cnt = 0;
	UINT32 intr_cnt = 0;
	UINT32 dequeue_cnt = 0;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, WF_SW_DEF_DBG_CNT, &macVal);
	msdu_miss_cnt = ((msdu_miss_cnt >> 16) & 0xff);
	alloc_fail_cnt = ((alloc_fail_cnt >> 24) & 0xff);
	cmd_cnt = ((macVal >> 8) & 0xff);
	event_cnt = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, WF_SW_DEF_DBG_CNT1, &macVal);
	intr_cnt = ((macVal >> 8) & 0xff);
	dequeue_cnt = (macVal & 0xff);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n       fw dbg counter\n"));

	HW_IO_READ32(pAd->hdev_ctrl, 0x58000204, &inrtVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       inrt enable = 0x%x\n",
					inrtVal));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       CMD:%d EVENT:%d\n",
					cmd_cnt, event_cnt));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       Msdu_Miss:%d Alloc_Fail:%d\n",
					msdu_miss_cnt, alloc_fail_cnt));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       Intr:%d Dequeue:%d\n",
					intr_cnt, dequeue_cnt));

}

VOID ShowCpuUtilSum(RTMP_ADAPTER *pAd)
{
	UINT32 busy_perc = 0;
	UINT32 peak_busy_perc = 0;
	UINT32 idle_cnt = 0;
	UINT32 peak_idle_cnt = 0;

	HW_IO_READ32(pAd->hdev_ctrl, 0x41F030, &busy_perc);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F034, &peak_busy_perc);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F038, &idle_cnt);
	HW_IO_READ32(pAd->hdev_ctrl, 0x41F03c, &peak_idle_cnt);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n       cpu ultility\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       Busy:%d%% Peak:%d%%\n",
				busy_perc, peak_busy_perc));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       IDLE:%d Peak_ILDE:%d\n",
				idle_cnt, peak_idle_cnt));
}

#define SYSIRQ_INTERRUPT_HISTORY_NUM 10
VOID ShowIrqHistory(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UINT32 i = 0;
	UINT32 start = 0;
	UINT32 idx = 0;
	UINT8 ucIrqDisIdx = 0;
	UINT8 ucIrqResIdx = 0;
	UINT32 irq_dis_time[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_dis_lp[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_res_time[SYSIRQ_INTERRUPT_HISTORY_NUM];
	UINT32 irq_res_lp[SYSIRQ_INTERRUPT_HISTORY_NUM];

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x2170BC, &macVal);
	ucIrqResIdx = (macVal & 0xff);
	ucIrqDisIdx = ((macVal >> 8) & 0xff);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n\n\n       Irq Idx (Dis=%d Res=%d):\n",
		ucIrqDisIdx, ucIrqResIdx));

	HW_IO_READ32(pAd->hdev_ctrl, 0x2170B8, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_dis_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_dis_lp[i] = macVal;
	}

	HW_IO_READ32(pAd->hdev_ctrl, 0x2170B4, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_res_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_res_lp[i] = macVal;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n       Dis Irq history (from old to new):\n"));

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqDisIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_dis_lp[idx], irq_dis_time[idx]));
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n       Restore Irq history (from old to new):\n"));

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqResIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_res_lp[idx], irq_res_time[idx]));
	}

}

VOID ShowLpHistory(RTMP_ADAPTER *pAd, BOOLEAN fgIsExp)
{
	UINT32 macVal = 0;
	UINT32 gpr_log_idx = 0;
	UINT32 oldest_idx = 0;
	UINT32 idx = 0;
	UINT32 i = 0;

	if (!fgIsExp) {
		/* disable LP recored */
		HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
		macVal &= (~0x1);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89050200, macVal);
		udelay(100);
	}

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
	gpr_log_idx = ((macVal >> 16) & 0x1f);
	oldest_idx = gpr_log_idx + 2;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       lp history (from old to new):\n"));

	for (i = 0; i < 16; i++) {
		idx = ((oldest_idx + 2*i + 1)%32);
		HW_IO_READ32(pAd->hdev_ctrl, (0x89050204 + idx*4), &macVal);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       %d: 0x%x\n", i, macVal));
	}

	if (!fgIsExp) {
		/* enable LP recored */
		HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
		macVal |= 0x1;
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89050200, macVal);
	}
}

#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_PTR_ADDR             0x41F054
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_NUM_ADDR             0x41F058
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_IDX_ADDR             0x41F05C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_PTR_ADDR             0x41F060
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_NUM_ADDR             0x41F064
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_PTR_ADDR             0x41F068
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_NUM_ADDR             0x41F06C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_PTR_ADDR             0x41F070
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_NUM_ADDR             0x41F074
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_PTR_ADDR             0x41F078
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_NUM_ADDR             0x41F07C
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R4_PTR_ADDR             0x41F080
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R4_NUM_ADDR             0x41F084

#define MSG_HISTORY_NUM 255
#define MAX_TASK_NUM 3

#define WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR                 (0x41F0E0)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x00)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_NUM_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x04)
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR                (WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR + 0x08)

VOID MemSectionRead(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 length, UINT32 addr)
{
	UINT32 idx = 0;
	void *ptr = buf;

	while (idx < length) {
		HW_IO_READ32(pAd->hdev_ctrl, (addr + idx), ptr);
		idx += 4;
		ptr += 4;
	}
}

UINT8 MemReadOneByte(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 val = 0;
	UINT8 tmpval = 0;

	HW_IO_READ32(pAd->hdev_ctrl, (addr & ~(0x3)), &val);
	tmpval = (val >> (8 * (addr & (0x3)))) & 0xff;
	return tmpval;
}

VOID ShowMsgTrace(RTMP_ADAPTER *pAd)
{
	cos_msg_trace_t *msg_trace;
	UINT32 ptr_addr = 0;
	UINT32 num_addr = 0;
	UINT32 length = 0;
	UINT8 idx = 0;
	UINT32 cnt = 0;
	UINT32 msg_history_num = 0;

	os_alloc_mem(pAd, (UCHAR **)&msg_trace, MSG_HISTORY_NUM * sizeof(cos_msg_trace_t));
	if (!msg_trace) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can not allocate cmd msg_trace\n"));
		return;
	}

	os_zero_mem(msg_trace, MSG_HISTORY_NUM * sizeof(cos_msg_trace_t));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_TRACE_PTR_ADDR, &ptr_addr);
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_TRACE_IDX_ADDR, &num_addr);
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_TRACE_NUM_ADDR, &msg_history_num);

	idx = MemReadOneByte(pAd, num_addr) & 0xff;
	msg_history_num = msg_history_num & 0xff;

	if (idx >= msg_history_num) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: idx(%d) >= msg_history_num(%d)\n", __func__, idx, msg_history_num));
		os_free_mem(msg_trace);
		return;
	}

	length = msg_history_num * sizeof(cos_msg_trace_t);
	MemSectionRead(pAd, (UCHAR *)msg_trace, length, ptr_addr);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       msg trace:\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       format: t_id=task_id/task_prempt_cnt/msg_read_idx\n"));

	while (1) {

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("       (m_%d)t_id=%x/%d/%d, m_id=%d, ts_en=%u, ts_de = %u, ts_fin=%u, wait=%d, exe=%d\n",
			idx,
			msg_trace[idx].dest_id,
			msg_trace[idx].pcount,
			msg_trace[idx].qread,
			msg_trace[idx].msg_id,
			msg_trace[idx].ts_enq,
			msg_trace[idx].ts_deq,
			msg_trace[idx].ts_finshq,
			(msg_trace[idx].ts_deq - msg_trace[idx].ts_enq),
			(msg_trace[idx].ts_finshq - msg_trace[idx].ts_deq))
		);
		if (++idx >= msg_history_num)
			idx = 0;

		if (++cnt >= msg_history_num)
			break;
	}
	if (msg_trace)
		os_free_mem(msg_trace);
}

VOID ShowSchduleTrace(RTMP_ADAPTER *pAd)
{
	task_info_struct  task_info_g[MAX_TASK_NUM] = {0};
	UINT32 length = 0;
	UINT32 idx = 0;
	UINT32 km_total_time = 0;
	UINT32 addr = 0;
	cos_task_type tcb;
	cos_task_type *tcb_ptr;
	CHAR   name[3][15] = {
		"WMT   ", "WIFI   ", "WIFI2   "
	};

	length = MAX_TASK_NUM * sizeof(task_info_struct);
	MemSectionRead(pAd, (UCHAR *)&(task_info_g[0]), length, 0x215400);

	HW_IO_READ32(pAd->hdev_ctrl, 0x219838, &km_total_time);
	if (km_total_time == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: km_total_time zero!\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\n\n\n       TASK    XTIME    RATIO    PREMPT CNT\n"));

	for (idx = 0 ;  idx < MAX_TASK_NUM ; idx++) {
		addr = task_info_g[idx].task_id;
		MemSectionRead(pAd, (UCHAR *)&(tcb), sizeof(cos_task_type), addr);
		tcb_ptr = &(tcb);

		if (tcb_ptr) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       %s    %d    %d       %d\n",
				name[idx],
				tcb_ptr->tc_exe_time,
				(tcb_ptr->tc_exe_time*100/km_total_time),
				tcb_ptr->tc_pcount));
		}
	}

}

#define MAX_MSG_INFO_RANGE_NUM 4
VOID ShowMsgWatch(RTMP_ADAPTER *pAd)
{
	UINT32 length[MAX_MSG_INFO_RANGE_NUM] = {0};
	UINT32 idx = 0;
	UINT32 r_idx = 0;
	UINT32 km_total_time = 0;
	UINT32 ms;
	UINT32 cos_msg_num[MAX_MSG_INFO_RANGE_NUM] = {0};
	UINT32 msg_info_addr[MAX_MSG_INFO_RANGE_NUM] = {0};
	UCHAR *msg = NULL;
	cos_internal_msgid ptr = NULL;
	UINT32 task_range_id[MAX_MSG_INFO_RANGE_NUM] = {1, 1, 2, 2};
	UINT32 task_range_base[MAX_MSG_INFO_RANGE_NUM] = {1, 100, 100, 138};

	HW_IO_READ32(pAd->hdev_ctrl, 0x219838, &km_total_time);
	if (km_total_time == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: km_total_time zero!\n", __func__));
		return;
	}
	
	ms = (((km_total_time*30)+(km_total_time*52/100))/1000);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\n\n\n       Monitor Duration: %d unit = %d ms (unit 30.52us)\n",
					km_total_time, ms));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("       MSG_ID     XTIME    RATIO    MAX_XTM      CNT\n"));

	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_NUM_ADDR, &(cos_msg_num[0]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_NUM_ADDR, &(cos_msg_num[1]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_NUM_ADDR, &(cos_msg_num[2]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_NUM_ADDR, &(cos_msg_num[3]));

	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_PTR_ADDR, &(msg_info_addr[0]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R1_PTR_ADDR, &(msg_info_addr[1]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R2_PTR_ADDR, &(msg_info_addr[2]));
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R3_PTR_ADDR, &(msg_info_addr[3]));

	for (r_idx = 0; r_idx < MAX_MSG_INFO_RANGE_NUM; r_idx++) {

		length[r_idx] = cos_msg_num[r_idx] * sizeof(cos_msg_type);
		os_alloc_mem(NULL, (PUCHAR *)&(msg), length[r_idx]);
		MemSectionRead(pAd, (UCHAR *)(msg), length[r_idx], (msg_info_addr[r_idx]));

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("       MSG WATCH TASK: %d\n", task_range_id[r_idx]));

		for (idx = 0 ;  idx < cos_msg_num[r_idx] ; idx++) {
			ptr = (cos_internal_msgid)(msg + (idx * sizeof(cos_msg_type)));
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       %d       %d        %d          %d       %d\n",
				(idx + task_range_base[r_idx]),
				ptr->exe_time,
				ptr->exe_time*100/km_total_time,
				ptr->exe_peak,
				ptr->finish_cnt));
		}

		os_free_mem(msg);
	}

}

#define PROGRAM_TRACE_HISTORY_NUM 255
VOID ShowProgTrace(RTMP_ADAPTER *pAd)
{
	cos_program_trace_t *cos_program_trace_ptr = NULL;
	UINT32 trace_ptr = 0;
	UINT32 idx = 0;
	UINT8 old_idx = 0;
	UINT32 old_idx_addr = 0;
	UINT32 prev_idx = 0;
	UINT32 prev_time = 0;
	UINT32 curr_time = 0;
	UINT32 diff = 0;
	UINT32 length = 0;
	UINT32 trace_num_addr = 0;
	UINT8 trace_num = 0;

	os_alloc_mem(pAd, (UCHAR **)&cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));
	if (!cos_program_trace_ptr) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can not allocate cos_program_trace_ptr memory\n"));
		return;
	}
	os_zero_mem(cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR, &trace_ptr);
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR, &old_idx_addr);
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_NUM_ADDR, &trace_num_addr);

	old_idx = MemReadOneByte(pAd, old_idx_addr) & 0xff;

	trace_num = MemReadOneByte(pAd, trace_num_addr) & 0xff;

	length = trace_num * sizeof(cos_program_trace_t);
	MemSectionRead(pAd, (UCHAR *)cos_program_trace_ptr, length, trace_ptr);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       prog trace:\n"));

	for (idx = 0 ; idx < trace_num ; idx++) {

		prev_idx = ((old_idx + trace_num - 1) % trace_num);

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("       (p_%d)t_id=%x/%d, m_id=%d, LP=0x%x, name=%s, ts4=%d, ",
			old_idx,
			cos_program_trace_ptr[old_idx].dest_id,
			cos_program_trace_ptr[old_idx].msg_sn,
			cos_program_trace_ptr[old_idx].msg_id,
			cos_program_trace_ptr[old_idx].LP,
			cos_program_trace_ptr[old_idx].name,
			cos_program_trace_ptr[old_idx].ts_gpt4));

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ts2=%d, ", cos_program_trace_ptr[old_idx].ts_gpt2));

		/* diff for gpt4 */
		prev_time = cos_program_trace_ptr[prev_idx].ts_gpt4;
		curr_time = cos_program_trace_ptr[old_idx].ts_gpt4;

		if (prev_time) {
			if ((cos_program_trace_ptr[prev_idx].dest_id == cos_program_trace_ptr[old_idx].dest_id) &&
				(cos_program_trace_ptr[prev_idx].msg_sn == cos_program_trace_ptr[old_idx].msg_sn)) {
				if (curr_time > prev_time)
					diff = curr_time - prev_time;
				else
					diff = 0xFFFFFFFF - prev_time + curr_time + 1;
			} else
				diff = 0xFFFFFFFF;
		} else
			diff = 0xFFFFFFFF;

		if (diff == 0xFFFFFFFF)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("diff4=NA, "));
		else
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("diff4=%d, ", diff));

		/* diff for gpt2 */
		prev_time = cos_program_trace_ptr[prev_idx].ts_gpt2;
		curr_time = cos_program_trace_ptr[old_idx].ts_gpt2;

		if (prev_time) {
			if ((cos_program_trace_ptr[prev_idx].dest_id == cos_program_trace_ptr[old_idx].dest_id) &&
				(cos_program_trace_ptr[prev_idx].msg_sn == cos_program_trace_ptr[old_idx].msg_sn)) {
				if (curr_time > prev_time)
					diff = curr_time - prev_time;
				else
					diff = 0xFFFFFFFF - prev_time + curr_time + 1;
			} else
				diff = 0xFFFFFFFF;
		} else
			diff = 0xFFFFFFFF;

		if (diff == 0xFFFFFFFF)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("diff2=NA\n"));
		else
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("diff2=%d\n", diff));

		old_idx++;

		if (old_idx >= trace_num)
			old_idx = 0;
	}
	if (cos_program_trace_ptr)
		os_free_mem(cos_program_trace_ptr);
}

static INT32 ShowAssertLine(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *msg;
	UINT32 addr;
	UINT32 macVal = 0;
	UCHAR *ptr;
	UCHAR idx;
	UCHAR *ret = NULL;

	os_alloc_mem(NULL, (UCHAR **)&msg, 256);

	if (!msg)
		return 0;

	NdisZeroMemory(msg, 256);

	addr = 0xE003B400;

	ptr = msg;
	for (idx = 0 ; idx < 32; idx++) {
		macVal = 0;
		HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);
		NdisCopyMemory(ptr, &macVal, 4);
		addr += 4;
		ptr += 4;
	}

	ret = strchr(msg, '\n');
	if (ret != NULL) {
		while (ret < (UCHAR *)(msg + 256)) {
			*ret = 0;
			ret++;
		}
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n\n"));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       Assert line\n"));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       %s\n", msg));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       --end--\n"));
	os_free_mem(msg);
	return 0;
}

#define CORE_DUMP_INFO_BASE (0x41F080)

static INT32 chip_show_fw_debg_info(struct _RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UCHAR exp_assert_proc_entry_cnt = 0;
	UCHAR exp_assert_state = 0;
	UCHAR dic_exp_type = 0;
	UINT32 COS_Interrupt_Count = 0;
	UINT16 processing_irqx = 0;
	UINT32 processing_lisr = 0;
	UINT32 Current_Task_Id = 0;
	UINT32 Current_Task_Indx = 0;
	UINT32 last_dequeued_msg_id = 0;
	UCHAR km_irq_info_idx = 0;
	UCHAR km_eint_info_idx = 0;
	UCHAR km_sched_info_idx = 0;
	UCHAR g_sched_history_num = 0;
	UINT32 km_sched_trace_ptr = 0;
	UCHAR g_irq_history_num = 0;
	UINT32 km_irq_trace_ptr = 0;
	UINT32 km_total_time  = 0;
	UCHAR exp_type[64];
	UINT32 TaskStart[3] = {0};
	UINT32 TaskEnd[3] = {0};
	BOOLEAN fgIsExp = FALSE;
	BOOLEAN fgIsAssert = FALSE;
	UINT32 exp_assert_state_addr = 0;
	UINT32 dic_exp_type_addr = 0;
	UINT32 cos_interrupt_count_addr = 0;
	UINT32 processing_irqx_addr = 0;
	UINT32 processing_lisr_addr = 0;
	UINT32 Current_Task_Id_addr = 0;
	UINT32 Current_Task_Indx_addr = 0;
	UINT32 last_dequeued_msg_id_addr = 0;
	UINT32 km_irq_info_idx_addr = 0;
	UINT32 km_eint_info_idx_addr = 0;
	UINT32 km_sched_info_idx_addr = 0;
	UINT32 g_sched_history_num_addr = 0;
	UINT32 km_sched_trace_ptr_addr = 0;
	UINT32 km_irq_trace_ptr_addr = 0;
	UINT32 km_total_time_addr  = 0;
	UINT32 i = 0;
	UINT32 t1 = 0;
	UINT32 t2 = 0;
	UINT32 t3 = 0;
	UCHAR idx = 0;
	UCHAR str[32];

	if (IS_MT7915_FW_VER_E2(pAd)) {
		exp_assert_state_addr = 0x219848;
		dic_exp_type_addr = 0x21987c;
		cos_interrupt_count_addr = 0x216F94;
		processing_irqx_addr = 0x216EF8;
		processing_lisr_addr = 0x2170AC;
		Current_Task_Id_addr = 0x216F90;
		Current_Task_Indx_addr = 0x216F9C;
		last_dequeued_msg_id_addr = 0x216F70;
		km_irq_info_idx_addr = 0x219820;
		km_eint_info_idx_addr = 0x219818;
		km_sched_info_idx_addr = 0x219828;
		g_sched_history_num_addr = 0x219828;
		km_sched_trace_ptr_addr = 0x219824;
		km_irq_trace_ptr_addr = 0x21981C;
		km_total_time_addr = 0x219838;
	} else {
		exp_assert_state_addr = 0x21A870;
		dic_exp_type_addr = 0x21A8A4;
		cos_interrupt_count_addr = 0x2173B8;
		processing_irqx_addr = 0x217320;
		processing_lisr_addr = 0x2174D8;
		Current_Task_Id_addr = 0x2173B4;
		Current_Task_Indx_addr = 0x2173C0;
		last_dequeued_msg_id_addr = 0x217388;
		km_irq_info_idx_addr = 0x21A848;
		km_eint_info_idx_addr = 0x21A840;
		km_sched_info_idx_addr = 0x21A850;
		g_sched_history_num_addr = 0x21A850;
		km_sched_trace_ptr_addr = 0x219824;
		km_irq_trace_ptr_addr = 0x21981C;
		km_total_time_addr = 0x219838;
	}

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, exp_assert_state_addr, &macVal);
	exp_assert_state = (macVal & 0xff);
	exp_assert_proc_entry_cnt = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, dic_exp_type_addr, &macVal);
	dic_exp_type = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, cos_interrupt_count_addr, &macVal);
	COS_Interrupt_Count = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, processing_irqx_addr, &macVal);
	processing_irqx = ((macVal >> 16) & 0xffff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, processing_lisr_addr, &macVal);
	processing_lisr = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, Current_Task_Id_addr, &macVal);
	Current_Task_Id = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, Current_Task_Indx_addr, &macVal);
	Current_Task_Indx = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, last_dequeued_msg_id_addr, &macVal);
	last_dequeued_msg_id = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_irq_info_idx_addr, &macVal);
	km_irq_info_idx = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_eint_info_idx_addr, &macVal);
	km_eint_info_idx = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_sched_info_idx_addr, &macVal);
	km_sched_info_idx = (macVal & 0xff);
	g_sched_history_num = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_sched_trace_ptr_addr, &macVal);
	km_sched_trace_ptr = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_irq_info_idx_addr, &macVal);
	km_irq_info_idx = (macVal & 0xff);
	g_irq_history_num = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_irq_trace_ptr_addr, &macVal);
	km_irq_trace_ptr = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_total_time_addr, &macVal);
	km_total_time = macVal;

	if (IS_MT7915_FW_VER_E2(pAd)) {
		HW_IO_READ32(pAd->hdev_ctrl, 0x2195A0, &(TaskStart[0]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x21959C, &(TaskEnd[0]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x219680, &(TaskStart[1]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x21967C, &(TaskEnd[1]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x219760, &(TaskStart[2]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x21975C, &(TaskEnd[2]));
	} else {
		HW_IO_READ32(pAd->hdev_ctrl, 0x2191C8, &(TaskStart[0]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x2191C4, &(TaskEnd[0]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x219CA8, &(TaskStart[1]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x219CA4, &(TaskEnd[1]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x21A788, &(TaskStart[2]));
		HW_IO_READ32(pAd->hdev_ctrl, 0x21A784, &(TaskEnd[2]));
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("================FW DBG INFO===================\n"));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       exp_assert_proc_entry_cnt = 0x%x\n", exp_assert_proc_entry_cnt));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       exp_assert_state = 0x%x\n", exp_assert_state));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       dic_exp_type = 0x%x\n", dic_exp_type));

	if (exp_assert_proc_entry_cnt == 0)
		snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Normal");
	else if (exp_assert_proc_entry_cnt == 1 &&
		exp_assert_state > 1 && dic_exp_type == 5) {
		snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Assert");
		fgIsExp = TRUE;
		fgIsAssert = TRUE;
	} else if (exp_assert_proc_entry_cnt == 1 && exp_assert_state > 1) {
		snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Exception");
		fgIsExp = TRUE;
	} else if (exp_assert_proc_entry_cnt > 1) {
		snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Exception re-entry");
		fgIsExp = TRUE;
	} else
		snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Unknown'?");


	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       COS_Interrupt_Count = 0x%x\n", COS_Interrupt_Count));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       processing_irqx = 0x%x\n", processing_irqx));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       processing_lisr = 0x%x\n", processing_lisr));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       Current_Task_Id = 0x%x\n", Current_Task_Id));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       Current_Task_Indx = 0x%x\n", Current_Task_Indx));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       last_dequeued_msg_id = %d\n", last_dequeued_msg_id));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       km_irq_info_idx = 0x%x\n", km_irq_info_idx));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       km_eint_info_idx = 0x%x\n", km_eint_info_idx));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       km_sched_info_idx = 0x%x\n", km_sched_info_idx));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       g_sched_history_num = %d\n", g_sched_history_num));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       km_sched_trace_ptr = 0x%x\n", km_sched_trace_ptr));

	if (fgIsExp) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\n        <1>print sched trace\n"));


		if (g_sched_history_num > 60)
			g_sched_history_num = 60;

		idx = km_sched_info_idx;

		for (i = 0 ; i < g_sched_history_num ; i++) {
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)), &(t1));
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)+4), &(t2));
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)+8), &(t3));
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       (sched_info_%d)sched_t=0x%x, sched_start=%d, PC=0x%x\n",
				idx, t1, t2, t3));

			idx++;

			if (idx >= g_sched_history_num)
				idx = 0;
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\n        <2>print irq trace\n"));


		if (g_irq_history_num > 60)
			g_irq_history_num = 60;

		idx = km_irq_info_idx;

		for (i = 0 ; i < g_irq_history_num ; i++) {
			HW_IO_READ32(pAd->hdev_ctrl, (km_irq_trace_ptr+(idx*16)), &(t1));
			HW_IO_READ32(pAd->hdev_ctrl, (km_irq_trace_ptr+(idx*16)+4), &(t2));

			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       (irq_info_%d)irq_t=%x, sched_start=%d\n",
				idx, t1, t2));

			idx++;

			if (idx >= g_irq_history_num)
				idx = 0;
		}
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n       <3>task q_id.read q_id.write\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       (WMT )0 0x%x 0x%x\n", TaskStart[0], TaskEnd[0]));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       (WIFI )1 0x%x 0x%x\n", TaskStart[1], TaskEnd[1]));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       (WIFI2 )2 0x%x 0x%x\n", TaskStart[2], TaskEnd[2]));


	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n       <4>TASK STACK INFO (size in byte)\n"));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       TASK  START       END       SIZE  PEAK  INTEGRITY\n"));

	for (i = 0 ; i < 3 ; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, 0x219558+(i*224), &t1);
		HW_IO_READ32(pAd->hdev_ctrl, 0x219554+(i*224), &t2);
		HW_IO_READ32(pAd->hdev_ctrl, 0x219560+(i*224), &t3);

		if (i == 0)
			snprintf(str, sizeof(str), "%s", "WMT");
		else if (i == 1)
			snprintf(str, sizeof(str), "%s", "WIFI");
		else if (i == 2)
			snprintf(str, sizeof(str), "%s", "WIFI2");

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("       %s  0x%x  0x%x  %d\n",
			str, t1, t2, t3));
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n       <5>fw state\n"));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       %s\n", exp_type));

	if (COS_Interrupt_Count > 0)
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       FW in Interrupt CIRQ index (0x%x) CIRQ handler(0x%x)\n"
			, processing_irqx, processing_lisr));
	else {
		if (Current_Task_Id == 0 && Current_Task_Indx == 3)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("FW in IDLE\n"));

		if (Current_Task_Id != 0 && Current_Task_Indx != 3)
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("       FW in Task , Task id(0x%x) Task index(0x%x)\n",
				Current_Task_Id, Current_Task_Indx));
	}

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       EXCP_CNT = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x04, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       EXCP_TYPE = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x08, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       CPU_ITYPE = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x0c, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       CPU_EVA = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x10, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       CPU_MERR = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x14, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       CPU_IPC = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x18, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       1ST_EXCP_TYPE = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x1c, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       1ST_CPU_ITYPE = 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x20, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       1ST_CPU_EVA= 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x24, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       1ST_CPU_MERR= 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, CORE_DUMP_INFO_BASE+0x28, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       1ST_CPU_IPC= 0x%x\n", macVal));

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x7C060204, &macVal);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       PC = 0x%x\n\n\n", macVal));


	ShowLpHistory(pAd, fgIsExp);
	ShowIrqHistory(pAd);

	ShowFwDbgCnt(pAd);
	ShowCpuUtilSum(pAd);
	ShowMsgTrace(pAd);
	ShowMsgWatch(pAd);
	ShowSchduleTrace(pAd);
	ShowProgTrace(pAd);

	if (fgIsAssert)
		ShowAssertLine(pAd);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("============================================\n"));

	return 0;

}

static INT chip_set_fw_cp_util_en(RTMP_ADAPTER *pAd, UINT en)
{
	UINT32 macVal = 0;

    MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s(): line(%d), en(%d)\n", __func__, __LINE__, en));

	HW_IO_READ32(pAd->hdev_ctrl, 0x41F04c, &macVal);
	macVal &= ~(BIT(0));
	HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F04c, macVal);

	if (en > 0) {
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2004);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	} else {
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2005);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	}

	return 0;
}

static INT chip_set_fw_cp_util_mode(RTMP_ADAPTER *pAd, UINT mode)
{
	UINT32 macVal = 0;

    MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s(): line(%d), mode(%d)\n", __func__, __LINE__, mode));

	HW_IO_READ32(pAd->hdev_ctrl, 0x41F04c, &macVal);
	macVal &= ~(BIT(0));
	HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F04c, macVal);

	switch (mode) {
	case 1:
	    {
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2007);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	        break;
	    }

	case 2:
	    {
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2008);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	        break;
	    }

	case 3:
	    {
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x41F010, 0x2009);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010080, 0x20);
	        HW_IO_WRITE32(pAd->hdev_ctrl, 0x890100c0, 0x20);
	        break;
	    }


	default: /* fatal error */
		break;	/* default: AC1 (BE) stream */
	} /* End of switch */

	return 0;
}

VOID mt7915_chip_dbg_init(struct _RTMP_CHIP_DBG *dbg_ops)
{
	dbg_ops->dump_ps_table = NULL;
	dbg_ops->dump_mib_info = chip_dump_mib_info;
	dbg_ops->show_tmac_info = chip_show_tmac_info;
	dbg_ops->show_agg_info = chip_show_agg_info;
	dbg_ops->show_dmasch_info = chip_show_dmasch_info;
	dbg_ops->show_pse_info = chip_show_pse_info;
	dbg_ops->show_pse_data = NULL; /* read PSE data from host is not supported */
	dbg_ops->show_protect_info = chip_show_protect_info;
	dbg_ops->show_cca_info = chip_show_cca_info;
	dbg_ops->set_cca_en = chip_set_cca_en;
	dbg_ops->show_txv_info = NULL;
	dbg_ops->check_txv = chip_check_txv;
	dbg_ops->show_bcn_info = chip_show_bcn_info;
	dbg_ops->show_ple_info = chip_show_ple_info;
	dbg_ops->show_drr_info = chip_show_drr_info;
	dbg_ops->dump_wtbl_info = chip_dump_wtbl_info;
	dbg_ops->dump_wtbl_mac = chip_dump_wtbl_mac;
	dbg_ops->dump_wtbl_base_info = chip_dump_wtbl_base_info;
	dbg_ops->dump_ple_amsdu_count_info = chip_show_amsdu_info;
	dbg_ops->set_hw_amsdu = chip_set_hw_amsdu;
	dbg_ops->set_header_translation = chip_set_header_translation;
	dbg_ops->show_dma_info = chip_show_dma_info;
#ifdef RANDOM_PKT_GEN
	dbg_ops->set_txctrl_proc = chip_set_txctrl_proc;
	dbg_ops->regular_pause_umac = chip_regular_pause_umac;
#endif /* RANDOM_PKT_GEN */
	dbg_ops->get_lpon_frcr = chip_get_lpon_frcr;
#ifdef VOW_SUPPORT
	dbg_ops->show_sta_acq_info = chip_show_sta_acq_info;
	dbg_ops->show_txcmdq_info = chip_show_txcmdq_info;
	dbg_ops->get_ple_acq_stat = chip_get_ple_acq_stat;
	dbg_ops->get_ple_txcmd_stat = chip_get_ple_txcmd_stat;
	dbg_ops->get_dis_sta_map = chip_get_dis_sta_map;
	dbg_ops->get_sta_pause = chip_get_sta_pause;
	dbg_ops->get_obss_nonwifi_airtime = chip_get_obss_nonwifi_airtime;
	dbg_ops->get_sta_airtime = chip_get_sta_airtime;
	dbg_ops->get_sta_addr = chip_get_sta_addr;
	dbg_ops->get_sta_rate = chip_get_sta_rate;
	dbg_ops->get_sta_tx_cnt = chip_get_sta_tx_cnt;
	dbg_ops->set_sta_psm = chip_set_sta_psm;
#endif	/* VOW_SUPPORT */
#ifdef MT7915_FPGA
	dbg_ops->show_txv_info = chip_show_txv_info;
#endif /*MT7915_FPGA*/
#ifdef CONFIG_ATE
	dbg_ops->ctrl_manual_hetb_tx = chip_ctrl_manual_hetb_tx;
	dbg_ops->ctrl_manual_hetb_rx = chip_ctrl_manual_hetb_rx;
	dbg_ops->chip_ctrl_spe = chip_ctrl_asic_spe;
	dbg_ops->get_tx_mibinfo = chip_get_tx_mibinfo;
#endif
	dbg_ops->show_asic_rx_stat = chip_show_asic_rx_stat;
	dbg_ops->show_ple_info_by_idx = chip_show_ple_info_by_idx;
	dbg_ops->show_fw_dbg_info = chip_show_fw_debg_info;
	dbg_ops->set_cpu_util_en = chip_set_fw_cp_util_en;
	dbg_ops->set_cpu_util_mode = chip_set_fw_cp_util_mode;
}
