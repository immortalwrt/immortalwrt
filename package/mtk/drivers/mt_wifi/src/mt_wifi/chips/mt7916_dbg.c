/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************
*/

#include "rt_config.h"
#include "chip/mt7916_cr.h"
#include "hdev/hdev.h"


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

static struct BUS_DEBUG_INFO conn_infra_ahb_apb_timeout_dump[] = {
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_DBG_1_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_DBG_1_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_DBG_2_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_DBG_2_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_ON_TOP_DBG_APB_1_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_ON_TOP_DBG_APB_1_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_ON_TOP_DBG_APB_2_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_ON_TOP_DBG_APB_2_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_TOP_DBG_1_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_TOP_DBG_1_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_TOP_DBG_2_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_OFF_TOP_DBG_2_ADDR},
	{"CONN_HOST_CSR_TOP_CONN_INFRA_BUS_TIMEOUT_LOG_ADDR", CONN_HOST_CSR_TOP_CONN_INFRA_BUS_TIMEOUT_LOG_ADDR},
};

static struct BUS_DEBUG_INFO wfsys_bus_ahp_apb_timeout[] = {
	{"Get the timeout address", 0x184F0444},
	{"Get the cmd information", 0x184F0430},
	{"Get bus hang bus transaction id", 0x184F044C},
	{"Get bus hang node index", 0x184F0450}
};

static INT32 chip_show_tmac_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 Value = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &Value);
	MTWF_PRINT("TX Stream = %d\n", GET_TMAC_TCR_TX_STREAM_NUM(Value) + 1);
	MTWF_PRINT("TX RIFS Enable = %d\n", GET_TX_RIFS_EN(Value));
	MTWF_PRINT("RX RIFS Mode = %d\n", GET_RX_RIFS_MODE(Value));
	MTWF_PRINT("TXOP TBTT Control = %d\n", GET_TXOP_TBTT_CONTROL(Value));
	MTWF_PRINT("TXOP TBTT Stop Control = %d\n", GET_TBTT_TX_STOP_CONTROL(Value));
	MTWF_PRINT("TXOP Burst Stop = %d\n", GET_TXOP_BURST_STOP(Value));
	MTWF_PRINT("RDG Mode = %d\n", GET_RDG_RA_MODE(Value));
	MTWF_PRINT("RDG Responser Enable = %d\n", GET_RDG_RESP_EN(Value));
	MTWF_PRINT("Smoothing = %d\n", GET_SMOOTHING(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_PSCR, &Value);
	MTWF_PRINT("AP Power Save RXPE Off Time(unit 2us) = %d\n",
			 GET_APS_OFF_TIME(Value));
	MTWF_PRINT("AP Power Save RXPE On Time(unit 2us) = %d\n", APS_ON_TIME(Value));
	MTWF_PRINT("AP Power Save Halt Time (unit 32us) = %d\n",
			 GET_APS_HALT_TIME(Value));
	MTWF_PRINT("AP Power Enable = %d\n", GET_APS_EN(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR1, &Value);
	MTWF_PRINT("AC0 TXOP = 0x%x (unit: 32us)\n", GET_AC0LIMIT(Value));
	MTWF_PRINT("AC1 TXOP = 0x%x (unit: 32us)\n", GET_AC1LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR0, &Value);
	MTWF_PRINT("AC2 TXOP = 0x%x (unit: 32us)\n", GET_AC2LIMIT(Value));
	MTWF_PRINT("AC3 TXOP = 0x%x (unit: 32us)\n", GET_AC3LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR3, &Value);
	MTWF_PRINT("AC10 TXOP = 0x%x (unit: 32us)\n", GET_AC10LIMIT(Value));
	MTWF_PRINT("AC11 TXOP = 0x%x (unit: 32us)\n", GET_AC11LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ACTXOPLR2, &Value);
	MTWF_PRINT("AC12 TXOP = 0x%x (unit: 32us)\n", GET_AC12LIMIT(Value));
	MTWF_PRINT("AC13 TXOP = 0x%x (unit: 32us)\n", GET_AC13LIMIT(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_0, &Value);
	MTWF_PRINT("EIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value));
	MTWF_PRINT("RIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value));
	MTWF_PRINT("SIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value));
	MTWF_PRINT("SLOT Time, Band0 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ICR_BAND_1, &Value);
	MTWF_PRINT("EIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value));
	MTWF_PRINT("RIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value));
	MTWF_PRINT("SIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value));
	MTWF_PRINT("SLOT Time, Band1 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value));
	RTMP_IO_READ32(pAd->hdev_ctrl, TMAC_ATCR, &Value);
	MTWF_PRINT("Aggregation Timeout (unit: 50ns) = 0x%x\n", GET_AGG_TOUT(Value));
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

		MTWF_PRINT("Band %d AGG Status\n", band_idx);
		MTWF_PRINT("===============================\n");
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset, &value);
		MTWF_PRINT("AC00 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC01 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC02 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC03 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset, &value);
		MTWF_PRINT("AC10 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC11 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC12 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC13 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset, &value);
		MTWF_PRINT("AC20 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC21 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC22 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC23 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset, &value);
		MTWF_PRINT("AC30 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC31 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC32 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_SHFT);
		MTWF_PRINT("AC33 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset, &value);
		MTWF_PRINT("ALTX Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_SHFT);

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR0_ADDR + band_offset, &value);
		MTWF_PRINT("Winsize0 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_SHFT);
		MTWF_PRINT("Winsize1 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_SHFT);
		MTWF_PRINT("Winsize2 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_SHFT);
		MTWF_PRINT("Winsize3 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR1_ADDR + band_offset, &value);
		MTWF_PRINT("Winsize4 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_SHFT);
		MTWF_PRINT("Winsize5 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_SHFT);
		MTWF_PRINT("Winsize6 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_SHFT);
		MTWF_PRINT("Winsize7 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR2_ADDR + band_offset, &value);
		MTWF_PRINT("Winsize8 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_SHFT);
		MTWF_PRINT("Winsize9 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_SHFT);
		MTWF_PRINT("WinsizeA limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_SHFT);
		MTWF_PRINT("WinsizeB limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_AGG_TOP_AWSCR3_ADDR + band_offset, &value);
		MTWF_PRINT("WinsizeC limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_SHFT);
		MTWF_PRINT("WinsizeD limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZED_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZED_SHFT);
		MTWF_PRINT("WinsizeE limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_SHFT);

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_PRINT("===AMPDU Related Counters===\n");
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

		MTWF_PRINT("\tTx Agg Range: \t%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d\n",
				 agg_rang_sel[0],
				 agg_rang_sel[0] + 1, agg_rang_sel[1],
				 agg_rang_sel[1] + 1, agg_rang_sel[2],
				 agg_rang_sel[2] + 1, agg_rang_sel[3],
				 agg_rang_sel[3] + 1, agg_rang_sel[4],
				 agg_rang_sel[4] + 1, agg_rang_sel[5],
				 agg_rang_sel[5] + 1, agg_rang_sel[6],
				 agg_rang_sel[6] + 1, agg_rang_sel[7]);

#define BIT_0_to_15_MASK 0x0000FFFF
#define BIT_15_to_31_MASK 0xFFFF0000
#define SHFIT_16_BIT 16

		for (idx = 3; idx < 11; idx++)
			total_ampdu = total_ampdu + (ampdu_cnt[idx] & BIT_0_to_15_MASK) + ((ampdu_cnt[idx] & BIT_15_to_31_MASK) >> SHFIT_16_BIT);

		MTWF_PRINT("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK,
				 (ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT,
				 (ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK,
				 (ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT,
				 (ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK,
				 (ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT,
				 (ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK,
				 (ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT);

		if (total_ampdu != 0) {
			MTWF_PRINT("\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu);
			}

		MTWF_PRINT("\t\t\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~256\n",
				 agg_rang_sel[7] + 1, agg_rang_sel[8],
				 agg_rang_sel[8] + 1, agg_rang_sel[9],
				 agg_rang_sel[9] + 1, agg_rang_sel[10],
				 agg_rang_sel[10] + 1, agg_rang_sel[11],
				 agg_rang_sel[11] + 1, agg_rang_sel[12],
				 agg_rang_sel[12] + 1, agg_rang_sel[13],
				 agg_rang_sel[13] + 1, agg_rang_sel[14],
				 agg_rang_sel[14] + 1);

		MTWF_PRINT("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE8_CNT_MASK,
				 (ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_SHFT,
				 (ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE10_CNT_MASK,
				 (ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_SHFT,
				 (ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE12_CNT_MASK,
				 (ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_SHFT,
				 (ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE14_CNT_MASK,
				 (ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_SHFT);

		if (total_ampdu != 0) {
			MTWF_PRINT("\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu);
			}
	}

	return 0;
}

static INT32 chip_dump_mib_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 bss_nums = pChipCap->BssNums;
	UINT32 mac_val0 = 0, mac_val = 0, mac_val1 = 0, idx, band_idx = 0, band_offset = 0;
	UINT32 msdr6 = 0, msdr7 = 0, msdr8 = 0, msdr9 = 0, msdr10 = 0;
	UINT32 msdr16 = 0, msdr17 = 0, msdr18 = 0, msdr19 = 0, msdr20 = 0, msdr21 = 0;
	UINT32 mbxsdr[4][7];
	UINT32 mbtocr[16] = {0}, mbtbcr[16] = {0}, mbrocr[16] = {0}, mbrbcr[16] = {0};
	UINT32 btocr[4], btbcr[4], brocr[4], brbcr[4], btdcr[4], brdcr[4];
	UINT32 mu_cnt[5] = {0};
	UINT32 ampdu_cnt[3] = {0};
	ULONG per;

	/* currently all bss_nums is 4, our variable will not overflow, in case of bss_num exceeds 4 so add judge here */
	if (bss_nums > 4) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "bss_nums(%d) exceeds 4!\n", bss_nums);
		return FALSE;
	}

	os_zero_mem(mbxsdr, bss_nums * 7 * sizeof(UINT32));
	os_zero_mem(btocr, bss_nums * sizeof(UINT32));
	os_zero_mem(btdcr, bss_nums * sizeof(UINT32));
	os_zero_mem(brocr, bss_nums * sizeof(UINT32));
	os_zero_mem(btbcr, bss_nums * sizeof(UINT32));
	os_zero_mem(brbcr, bss_nums * sizeof(UINT32));
	os_zero_mem(brdcr, bss_nums * sizeof(UINT32));

	for (band_idx = 0; band_idx < pChipCap->band_cnt; band_idx++) {
#ifdef CONFIG_AP_SUPPORT
		PBCN_CHECK_INFO_STRUC pBcnCheckInfo = &pAd->BcnCheckInfo[band_idx];
#endif
		if (arg != NULL && band_idx != simple_strtoul(arg, 0, 10))
			continue;

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_PRINT("Band %d MIB Status\n", band_idx);
		MTWF_PRINT("===============================\n");
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SCR0_ADDR + band_offset, &mac_val);
		MTWF_PRINT("MIB Status Control=0x%x\n", mac_val);
		/* Panther Default All ON */
		/*
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0PBSCR_ADDR + band_offset, &mac_val);
		MTWF_PRINT("MIB Per-BSS Status Control=0x%x\n", mac_val);
		*/

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

		MTWF_PRINT("===Phy/Timing Related Counters===\n");
		MTWF_PRINT("\tChannelIdleCnt=0x%x\n", msdr6 & BN0_WF_MIB_TOP_M0SDR6_CHANNEL_IDLE_COUNT_MASK);
		MTWF_PRINT("\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK);
		MTWF_PRINT("\tRx_MDRDY_CNT=0x%x\n", msdr10 & BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_MASK);
		MTWF_PRINT("\tCCK_MDRDY_TIME=0x%x, OFDM_MDRDY_TIME=0x%x, OFDM_GREEN_MDRDY_TIME=0x%x\n",
				 msdr19 & BN0_WF_MIB_TOP_M0SDR19_CCK_MDRDY_TIME_MASK,
				 msdr20 & BN0_WF_MIB_TOP_M0SDR20_OFDM_LG_MIXED_VHT_MDRDY_TIME_MASK,
				 msdr21 & BN0_WF_MIB_TOP_M0SDR21_OFDM_GREEN_MDRDY_TIME_MASK);
		MTWF_PRINT("\tPrim CCA Time=0x%x\n", msdr16 & BN0_WF_MIB_TOP_M0SDR16_P_CCA_TIME_MASK);
		MTWF_PRINT("\tSec CCA Time=0x%x\n", msdr17 & BN0_WF_MIB_TOP_M0SDR17_S_CCA_TIME_MASK);
		MTWF_PRINT("\tPrim ED Time=0x%x\n", msdr18 & BN0_WF_MIB_TOP_M0SDR18_P_ED_TIME_MASK);

		MTWF_PRINT("===Tx Related Counters(Generic)===\n");
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR0_ADDR + band_offset, &mac_val);
#ifdef CONFIG_AP_SUPPORT
		pBcnCheckInfo->totalbcncnt += (mac_val & BN0_WF_MIB_TOP_M0SDR0_BEACONTXCOUNT_MASK);
		MTWF_PRINT("\tBeaconTxCnt=0x%x\n", pBcnCheckInfo->totalbcncnt);
		pBcnCheckInfo->totalbcncnt = 0;
#else
		MTWF_PRINT("\tBeaconTxCnt=0x%x\n", mac_val);
#endif
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR0_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tTx 20MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR0_TX_20MHZ_CNT_MASK);
		MTWF_PRINT("\tTx 40MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_SHFT);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR1_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tTx 80MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR1_TX_80MHZ_CNT_MASK);
		MTWF_PRINT("\tTx 160MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_SHFT);
		MTWF_PRINT("\tAMPDU Cnt=0x%x\n", ampdu_cnt[0]);
		MTWF_PRINT("\tAMPDU MPDU Cnt=0x%x\n", ampdu_cnt[1]);
		MTWF_PRINT("\tAMPDU MPDU Ack Cnt=0x%x\n", ampdu_cnt[2]);
		per = (ampdu_cnt[2] == 0 ? 0 : 1000 * (ampdu_cnt[1] - ampdu_cnt[2]) / ampdu_cnt[1]);
		MTWF_PRINT("\tAMPDU MPDU PER=%ld.%1ld%%\n", per / 10, per % 10);

		MTWF_PRINT("===MU Related Counters===\n");

		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR33_ADDR + band_offset, &mu_cnt[0]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR8_ADDR + band_offset, &mu_cnt[1]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR9_ADDR + band_offset, &mu_cnt[2]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR10_ADDR + band_offset, &mu_cnt[3]);
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR11_ADDR + band_offset, &mu_cnt[4]);

		MTWF_PRINT("\tMUBF_TX_COUNT=0x%x\n", mu_cnt[0] & BN0_WF_MIB_TOP_M0SDR33_MUBF_TX_COUNT_MASK);
		MTWF_PRINT("\tMU_TX_MPDU_COUNT(Ok+Fail)=0x%x\n", mu_cnt[1]);
		MTWF_PRINT("\tMU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[2]);
		MTWF_PRINT("\tMU_TO_SU_PPDU_COUNT=0x%x\n", mu_cnt[3] & BN0_WF_MIB_TOP_M0DR10_MU_FAIL_PPDU_COUNT_MASK);
		MTWF_PRINT("\tSU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[4]);

		MTWF_PRINT("===Rx Related Counters(Generic)===\n");
		MTWF_PRINT("\tVector Mismacth Cnt=0x%x\n", msdr7 & BN0_WF_MIB_TOP_M0SDR7_VEC_MISS_COUNT_MASK);
		MTWF_PRINT("\tDelimiter Fail Cnt=0x%x\n", msdr8 & BN0_WF_MIB_TOP_M0SDR8_DELIMITER_FAIL_COUNT_MASK);

		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR3_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRxFCSErrCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_MASK));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR4_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRxFifoFullCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR4_RX_FIFO_FULL_COUNT_MASK));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR11_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRxLenMismatch=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR11_RX_LEN_MISMATCH_MASK));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR5_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRxMPDUCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR5_RX_MPDU_COUNT_MASK));
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR22_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRx AMPDU Cnt=0x%x\n", mac_val);
		/* TODO: shiang-MT7615, is MIB_M0SDR23 used for Rx total byte count for all or just AMPDU only??? */
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR23_ADDR + band_offset, &mac_val);
		MTWF_PRINT("\tRx Total ByteCnt=0x%x\n", mac_val);

		band_offset = (BN1_WF_MIB_TOP_BTOCR_ADDR - BN0_WF_MIB_TOP_BTOCR_ADDR) * band_idx;
		MTWF_PRINT("===Per-BSS Related Tx/Rx Counters===\n");
		MTWF_PRINT("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n");

		for (idx = 0; idx < bss_nums; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BTOCR_ADDR + band_offset + (idx >> 1) * 4), &btocr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BTDCR_ADDR + band_offset + (idx >> 1) * 4), &btdcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + (idx * 4)), &btbcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BROCR_ADDR + band_offset + (idx >> 1) * 4), &brocr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BRDCR_ADDR + band_offset + (idx >> 1) * 4), &brdcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BRBCR_ADDR + band_offset + (idx * 4)), &brbcr[idx]);
			if ((idx % 2) == 0) {
				btocr[idx] = ((btocr[idx] & BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2n_SHFT);
				btdcr[idx] = ((btdcr[idx] & BN0_WF_MIB_TOP_BTDCR_TX_DATA_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BTDCR_TX_DATA_COUNT2n_SHFT);
				brocr[idx] = ((brocr[idx] & BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2n_SHFT);
				brdcr[idx] = ((brdcr[idx] & BN0_WF_MIB_TOP_BRDCR_RX_DATA_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BRDCR_RX_DATA_COUNT2n_SHFT);
			} else {
				btocr[idx] = ((btocr[idx] & BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2np1_SHFT);
				btdcr[idx] = ((btdcr[idx] & BN0_WF_MIB_TOP_BTDCR_TX_DATA_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BTDCR_TX_DATA_COUNT2np1_SHFT);
				brocr[idx] = ((brocr[idx] & BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2np1_SHFT);
				brdcr[idx] = ((brdcr[idx] & BN0_WF_MIB_TOP_BRDCR_RX_DATA_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BRDCR_RX_DATA_COUNT2np1_SHFT);
			}
		}

		for (idx = 0; idx < bss_nums; idx++) {
			MTWF_PRINT("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
						idx, btocr[idx], btdcr[idx], btbcr[idx], brocr[idx], brdcr[idx], brbcr[idx]);
		}

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		MTWF_PRINT("===Per-MBSS Related MIB Counters===\n");
		MTWF_PRINT("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2/3Cnt\n");

		for (idx = 0; idx < bss_nums; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR0_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][0]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR1_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][1]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR2_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][2]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR3_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][3]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR4_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][4]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR5_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][5]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BSDR6_ADDR + band_offset + (idx >> 1) * 4), &mbxsdr[idx][6]);

			if ((idx % 2) == 0) {
				mbxsdr[idx][0] = ((mbxsdr[idx][0] & BN0_WF_MIB_TOP_BSDR0_RTSTXCOUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR0_RTSTXCOUNT2n_SHFT);
				mbxsdr[idx][1] = ((mbxsdr[idx][1] & BN0_WF_MIB_TOP_BSDR1_RTSRETRYCOUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR1_RTSRETRYCOUNT2n_SHFT);
				mbxsdr[idx][2] = ((mbxsdr[idx][2] & BN0_WF_MIB_TOP_BSDR2_BAMISSCOUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR2_BAMISSCOUNT2n_SHFT);
				mbxsdr[idx][3] = ((mbxsdr[idx][3] & BN0_WF_MIB_TOP_BSDR3_ACKFAILCOUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR3_ACKFAILCOUNT2n_SHFT);
				mbxsdr[idx][4] = ((mbxsdr[idx][4] & BN0_WF_MIB_TOP_BSDR4_FRAMERETRYCOUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR4_FRAMERETRYCOUNT2n_SHFT);
				mbxsdr[idx][5] = ((mbxsdr[idx][5] & BN0_WF_MIB_TOP_BSDR5_FRAMERETRY2COUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR5_FRAMERETRY2COUNT2n_SHFT);
				mbxsdr[idx][6] = ((mbxsdr[idx][6] & BN0_WF_MIB_TOP_BSDR6_FRAMERETRY3COUNT2n_MASK) >> BN0_WF_MIB_TOP_BSDR6_FRAMERETRY3COUNT2n_SHFT);
			} else {
				mbxsdr[idx][0] = ((mbxsdr[idx][0] & BN0_WF_MIB_TOP_BSDR0_RTSTXCOUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR0_RTSTXCOUNT2np1_SHFT);
				mbxsdr[idx][1] = ((mbxsdr[idx][1] & BN0_WF_MIB_TOP_BSDR1_RTSRETRYCOUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR1_RTSRETRYCOUNT2np1_SHFT);
				mbxsdr[idx][2] = ((mbxsdr[idx][2] & BN0_WF_MIB_TOP_BSDR2_BAMISSCOUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR2_BAMISSCOUNT2np1_SHFT);
				mbxsdr[idx][3] = ((mbxsdr[idx][3] & BN0_WF_MIB_TOP_BSDR3_ACKFAILCOUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR3_ACKFAILCOUNT2np1_SHFT);
				mbxsdr[idx][4] = ((mbxsdr[idx][4] & BN0_WF_MIB_TOP_BSDR4_FRAMERETRYCOUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR4_FRAMERETRYCOUNT2np1_SHFT);
				mbxsdr[idx][5] = ((mbxsdr[idx][5] & BN0_WF_MIB_TOP_BSDR5_FRAMERETRY2COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR5_FRAMERETRY2COUNT2np1_SHFT);
				mbxsdr[idx][6] = ((mbxsdr[idx][6] & BN0_WF_MIB_TOP_BSDR6_FRAMERETRY3COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BSDR6_FRAMERETRY3COUNT2np1_SHFT);
			}
		}

		for (idx = 0; idx < bss_nums; idx++) {
			MTWF_PRINT("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x/0x%x\n",
						idx, mbxsdr[idx][0], mbxsdr[idx][1], mbxsdr[idx][2], mbxsdr[idx][3], mbxsdr[idx][4], mbxsdr[idx][5], mbxsdr[idx][6]);
		}

		MTWF_PRINT("===Dummy delimiter insertion result===\n");
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR12_ADDR + band_offset, &mac_val0);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR6_ADDR + band_offset, &mac_val);
		RTMP_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR7_ADDR + band_offset, &mac_val1);
		MTWF_PRINT("Range0 = %d\t Range1 = %d\t Range2 = %d\t Range3 = %d\t Range4 = %d\n",
					(mac_val0 & BN0_WF_MIB_TOP_M0DR12_TX_DDLMT_RNG0_COUNT_MASK),
					(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG1_COUNT_MASK),
					(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_COUNT_MASK) >> BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_COUNT_SHFT,
					(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG3_COUNT_MASK),
					(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_COUNT_MASK) >> BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_COUNT_SHFT);

		band_offset = (BN1_WF_MIB_TOP_BTOCR_ADDR - BN0_WF_MIB_TOP_BTOCR_ADDR) * band_idx;
		MTWF_PRINT("===Per-MBSS Related Tx/Rx Counters===\n");
		MTWF_PRINT("MBSSIdx   TxCnt  TxByteCnt  RxCnt  RxByteCnt\n");

		for (idx = 0; idx < 16; idx++) {
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BTOCR_ADDR + band_offset + ((bss_nums >> 1) * 4) + ((idx >> 1) * 4)), &mbtocr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + ((bss_nums >> 1) * 4) + (idx * 4)), &mbtbcr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BROCR_ADDR + band_offset + ((bss_nums >> 1) * 4) + ((idx >> 1) * 4)), &mbrocr[idx]);
			RTMP_IO_READ32(pAd->hdev_ctrl, (BN0_WF_MIB_TOP_BRBCR_ADDR + band_offset + ((bss_nums >> 1) * 4) + (idx * 4)), &mbrbcr[idx]);

			if ((idx % 2) == 0) {
				mbtocr[idx] = ((mbtocr[idx] & BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2n_SHFT);
				mbrocr[idx] = ((mbrocr[idx] & BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2n_MASK) >> BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2n_SHFT);
			} else {
				mbtocr[idx] = ((mbtocr[idx] & BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BTOCR_TX_OK_COUNT2np1_SHFT);
				mbrocr[idx] = ((mbrocr[idx] & BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2np1_MASK) >> BN0_WF_MIB_TOP_BROCR_RX_OK_COUNT2np1_SHFT);
			}
		}

		for (idx = 0; idx < 16; idx++) {
			MTWF_PRINT("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
						idx, mbtocr[idx], mbtbcr[idx], mbrocr[idx], mbrbcr[idx]);
		}
	}

#ifdef TRACELOG_TCP_PKT
	MTWF_PRINT("TCP RxAck = %d\t TxData = %d",
		pAd->u4TcpRxAckCnt, pAd->u4TcpTxDataCnt);
	pAd->u4TcpRxAckCnt = 0;
	pAd->u4TcpTxDataCnt = 0;
#endif /* TRACELOG_TCP_PKT */
	return TRUE;
}

static VOID chip_show_peekcr_info(RTMP_ADAPTER *pAd, UINT32 start_addr, UINT32 end_addr)
{
	INT32 j = 0;
	UINT32 addr = start_addr;
	UINT32 ple_peekcr = 0;
	MTWF_PRINT("PeekCR info:\n");

	for (j = 0; addr <= end_addr; j++, addr += 4) {
		HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_peekcr);
		MTWF_PRINT("\tPEEK_CR%02d:0x%x\n", j, ple_peekcr);
	}
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
	MTWF_PRINT("PSE Configuration Info:\n");
	MTWF_PRINT("\tPacket Buffer Control(0x82068014): 0x%08x\n", pse_buf_ctrl);
	pg_sz = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_PRINT("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128));
	MTWF_PRINT("\t\tPage Offset=%d(in unit of 64KB)\n",
			 (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_PRINT("\t\tTotal page numbers=%d pages\n", pg_num);
	/* Page Flow Control */
	MTWF_PRINT("PSE Page Flow Control:\n");
	MTWF_PRINT("\tFree page counter(0x82068100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_PRINT("\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_PRINT("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	MTWF_PRINT("\tFree page head and tail(0x82068104): 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_PRINT("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	MTWF_PRINT("\tReserved page counter of HIF0 group(0x82068110): 0x%08x\n", pg_flow_ctrl[2]);
	MTWF_PRINT("\tHIF0 group page status(0x82068114): 0x%08x\n", pg_flow_ctrl[3]);
	min_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of HIF1 group(0x82068118): 0x%08x\n", pg_flow_ctrl[4]);
	MTWF_PRINT("\tHIF1 group page status(0x8206811c): 0x%08x\n", pg_flow_ctrl[5]);
	min_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of CPU group(0x82068150): 0x%08x\n", pg_flow_ctrl[6]);
	MTWF_PRINT("\tCPU group page status(0x82068154): 0x%08x\n", pg_flow_ctrl[7]);
	min_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC0 group(0x82068170): 0x%08x\n", pg_flow_ctrl[8]);
	MTWF_PRINT("\tLMAC0 group page status(0x82068174): 0x%08x\n", pg_flow_ctrl[9]);
	min_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC1 group(0x82068178): 0x%08x\n", pg_flow_ctrl[10]);
	MTWF_PRINT("\tLMAC1 group page status(0x8206817c): 0x%08x\n", pg_flow_ctrl[11]);
	min_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	MTWF_PRINT("\tReserved page counter of LMAC2 group(0x82068180): 0x%08x\n", pg_flow_ctrl[11]);
	MTWF_PRINT("\tLMAC2 group page status(0x82068184): 0x%08x\n", pg_flow_ctrl[12]);
	min_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of LMAC3 group(0x82068188): 0x%08x\n", pg_flow_ctrl[16]);
	MTWF_PRINT("\tLMAC3 group page status(0x8206818c): 0x%08x\n", pg_flow_ctrl[17]);
	min_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of LMAC3 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of LMAC3 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of PLE group(0x82068160): 0x%08x\n", pg_flow_ctrl[14]);
	MTWF_PRINT("\tPLE group page status(0x82068164): 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of PLE1 group(0x82068168): 0x%08x\n", pg_flow_ctrl[14]);
	MTWF_PRINT("\tPLE1 group page status(0x8206816c): 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of PLE1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of PLE1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	MTWF_PRINT("\tReserved page counter of MDP group(0x82068198): 0x%08x\n", pg_flow_ctrl[18]);
	MTWF_PRINT("\tMDP group page status(0x8206819c): 0x%08x\n", pg_flow_ctrl[19]);
	min_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of MDP group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of MDP group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	/* Queue Empty Status */
	MTWF_PRINT("PSE Queue Empty Status:\n");
	MTWF_PRINT("\tQUEUE_EMPTY(0x820680b0): 0x%08x\n", pse_stat);
	MTWF_PRINT("\t\tCPU Q0/1/2/3 empty=%d/%d/%d/%d\n",
			  (pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_SHFT,
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_SHFT));
	MTWF_PRINT("\t\tHIF Q0/1/2/3/4/5 empty=%d/%d/%d/%d/%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_SHFT));
	MTWF_PRINT("\t\tLMAC TX Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tMDP TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tSEC TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tSFD PARK Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tMDP TXIOC Q/RXIOC Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_SHFT));
	MTWF_PRINT("\t\tRLS Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_SHFT));
	MTWF_PRINT("Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((pse_stat & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (pse_queue_empty_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", pse_queue_empty_info[i].QueueName);
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
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}
	chip_show_peekcr_info(pAd, WF_PSE_TOP_PSE_SEEK_CR_00_ADDR, WF_PSE_TOP_PSE_SEEK_CR_09_ADDR);

	return TRUE;
}

static INT32 chip_show_protect_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	MTWF_PRINT(" -Proetction\n");
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR, &val);
	MTWF_PRINT("  > AGG_PCR 0x%08x\n", val);
	MTWF_PRINT(" -RTS Threshold\n");
	RTMP_IO_READ32(pAd->hdev_ctrl, AGG_PCR1, &val);
	MTWF_PRINT("  > AGG_PCR1 0x%08x\n", val);
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
	MTWF_PRINT("CCA for BAND0 info:\n");
	MTWF_PRINT("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5));
	MAC_IO_READ32(pAd->hdev_ctrl, RMAC_DEBUG_CR, &val);
	val &= ~(1 << 31); /* For Band1 */
	MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_DEBUG_CR, val);
	MAC_IO_READ32(pAd->hdev_ctrl, (WF_CFG_BASE + 0x24), &val);
	MTWF_PRINT("CCA for BAND1 info:\n");
	MTWF_PRINT("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
			  ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
			  ((val & (1 << 5)) >> 5));
	return 0;
}

static INT32 chip_set_cca_en(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{
	BOOLEAN enable;
	UINT32 val = 0;
	RTMP_ADAPTER *pAd = ctrl->priv;

	enable = os_str_tol(arg, 0, 10);
	MTWF_PRINT("Enable CCA on Band0 SEC40: %s\n", (enable) ? "ON" : "OFF");
	/* RF CR for BAND0 CCA */
	PHY_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHY_CCA, &val);
	val |= ((1 << 18) | (1 << 2));
	MTWF_PRINT("-- Force Mode: %d, Force CCA SEC40: %d [0x%08x]\n",
			  ((val & (1 << 18)) >> 18), ((val & (1 << 2)) >> 2), val);
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

	MTWF_PRINT("Dma scheduler info:\n");
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR, &value);
	pktin_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PKTIN_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	pdma_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PDMA_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_add_int_refill_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_ADD_INT_REFILL_ENA_MASK) ? TRUE : FALSE;
	ple_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_PLE_SUB_ENA_MASK) ? TRUE : FALSE;
	hif_ask_sub_ena = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_HIF_ASK_SUB_ENA_MASK) ? TRUE : FALSE;
	wacpu_mode_en = (value & WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_WACPU_MODE_EN_MASK) ? TRUE : FALSE;
	MTWF_PRINT("DMASHDL Ctrl Signal(0x5000A018): 0x%08x\n", value);
	MTWF_PRINT("\twacpu mode en(BIT0) = %d\n", wacpu_mode_en);
	MTWF_PRINT("\thif_ask_sub_ena(BIT16) = %d\n", hif_ask_sub_ena);
	MTWF_PRINT("\tple_sub_ena(BIT17) = %d\n", ple_sub_ena);
	MTWF_PRINT("\tple_add_int_refill_ena(BIT29) = %d\n", ple_add_int_refill_ena);
	MTWF_PRINT("\tpdma_add_int_refill_ena(BIT30) = %d\n", pdma_add_int_refill_ena);
	MTWF_PRINT("\tpktin_int_refill(BIT31)_ena = %d\n", pktin_int_refill_ena);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &value);
	ple_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT;
	pse_pkt_max_sz = (value & WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK)
				>> WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT;
	MTWF_PRINT("DMASHDL Packet_max_size(0x5000A01c): 0x%08x\n", value);
	MTWF_PRINT("PLE/PSE packet max size=0x%03x/0x%03x\n",
			  ple_pkt_max_sz, pse_pkt_max_sz);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_ERROR_FLAG_CTRL_ADDR, &value);
	MTWF_PRINT("DMASHDL ERR FLAG CTRL(0x5000A09c): 0x%08x\n", value);
	HW_IO_READ32(pAd->hdev_ctrl, WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR, &value);
	ffa_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_SHFT;
	free_pg_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_MASK) >> WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_SHFT;
	MTWF_PRINT("DMASHDL Status_RD(0x5000A100): 0x%08x\n", value);
	MTWF_PRINT("free page cnt = 0x%03x, ffa cnt = 0x%03x\n", free_pg_cnt, ffa_cnt);

	for (groupidx = 0; groupidx < 16; groupidx++) {
		idx = 0;
		MTWF_PRINT("Group %d info:", groupidx);
		HW_IO_READ32(pAd->hdev_ctrl, status_addr, &value);
		rsv_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_SHFT;
		src_cnt = (value & WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_MASK)
				>> WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_SHFT;
		MTWF_PRINT("\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n", groupidx, status_addr, value);
		HW_IO_READ32(pAd->hdev_ctrl, quota_addr, &value);
		max_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT;
		min_quota = (value & WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK)
				>> WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT;
		MTWF_PRINT("\tDMASHDL Group%d control(0x%08x): 0x%08x\n", groupidx, quota_addr, value);

		if ((groupidx & 0x1) == 0) {
			HW_IO_READ32(pAd->hdev_ctrl, pkt_cnt_addr, &value);
			MTWF_PRINT("\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n", groupidx / 2, pkt_cnt_addr, value);
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

		MTWF_PRINT("\trsv_cnt = 0x%03x, src_cnt = 0x%03x\n", rsv_cnt, src_cnt);
		MTWF_PRINT("\tmax/min quota = 0x%03x/ 0x%03x\n", max_quota, min_quota);
		MTWF_PRINT("\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x", pktin_cnt, ask_cnt);

		if (hif_ask_sub_ena && pktin_cnt != ask_cnt) {
			MTWF_PRINT(", mismatch!");
			is_mismatch = TRUE;
		}

		MTWF_PRINT("\n");

		if (groupidx == 15 && Group_Mapping_Q[groupidx] == 0) { /* Group15 is for PSE */
			pse_src_cnt = src_cnt;
			pse_rsv_cnt = rsv_cnt;
			break;
		}

		MTWF_PRINT("\tMapping Qidx:");

		while (Group_Mapping_Q[groupidx] != 0) {
			if (Group_Mapping_Q[groupidx] & 0x1)
				MTWF_PRINT("Q%d ", idx);

			Group_Mapping_Q[groupidx] >>= 1;
			idx++;
		}

		MTWF_PRINT("\n");
		total_src_cnt += src_cnt;
		total_rsv_cnt += rsv_cnt;
		status_addr = status_addr + 4;
		quota_addr = quota_addr + 4;

		if (groupidx & 0x1)
			pkt_cnt_addr = pkt_cnt_addr + 4;
	}

	MTWF_PRINT("\nCounter Check:\n");
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_HIF_PG_INFO_ADDR, &value);
	ple_rpg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	ple_upg_hif = (value & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_PRINT("PLE:\n\tThe used/reserved pages of PLE HIF group=0x%03x/0x%03x\n",
			  ple_upg_hif, ple_rpg_hif);
	MAC_IO_READ32(pAd->hdev_ctrl, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &value);
	pse_rpg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	pse_upg_hif = (value & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	MTWF_PRINT("PSE:\n\tThe used/reserved pages of PSE HIF group=0x%03x/0x%03x\n",
			  pse_upg_hif, pse_rpg_hif);
	MTWF_PRINT("DMASHDL:\n\tThe total used pages of group0~14=0x%03x",
			  total_src_cnt);

	if (ple_upg_hif != total_src_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe total reserved pages of group0~14=0x%03x\n",
			  total_rsv_cnt);
	MTWF_PRINT("\tThe total ffa pages of group0~14=0x%03x\n",
			  ffa_cnt);
	MTWF_PRINT("\tThe total free pages of group0~14=0x%03x",
			  free_pg_cnt);

	if (free_pg_cnt != total_rsv_cnt + ffa_cnt) {
		MTWF_PRINT(", mismatch(total_rsv_cnt + ffa_cnt in DMASHDL)");
		is_mismatch = TRUE;
	}

	if (free_pg_cnt != ple_rpg_hif) {
		MTWF_PRINT(", mismatch(reserved pages in PLE)");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe used pages of group15=0x%03x", pse_src_cnt);

	if (pse_upg_hif != pse_src_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\tThe reserved pages of group15=0x%03x", pse_rsv_cnt);

	if (pse_rpg_hif != pse_rsv_cnt) {
		MTWF_PRINT(", mismatch!");
		is_mismatch = TRUE;
	}

	MTWF_PRINT("\n");

	if (!is_mismatch)
		MTWF_PRINT("DMASHDL: no counter mismatch\n");

	return TRUE;
}

#ifdef MT7916_FPGA
#endif /*MT7916_FPGA*/


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
	MTWF_PRINT("\n");

	show_mib_proc(pAd, (bandidx == DBDC_BAND0) ? "0" : "1");
	MTWF_PRINT("\n");

	show_trinfo_proc(pAd, "");
	MTWF_PRINT("\n");

	ShowPLEInfo(pAd, NULL);
	MTWF_PRINT("\n");

#ifdef ERR_RECOVERY
	ShowSerProc2(pAd, "");
	MTWF_PRINT("\n");
#endif

	ShowPseInfo(pAd, NULL);
	MTWF_PRINT("\n");

	Show_MibBucket_Proc(pAd, "");
	MTWF_PRINT("\n");

	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_SCR_ADDR + band_offset, &mac_val);
	MTWF_PRINT("ARB_SCR=0x%08x\n", mac_val);

	for (idx = 0; idx < 10; idx++) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_ARB_TOP_BFCR_ADDR + band_offset, &mac_val);
		MTWF_PRINT("ARB_BFCR=0x%08x (loop %d)\n", mac_val, idx);
	}

	MTWF_PRINT("\n");
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

				MTWF_PRINT("\tSTA%d AC%d: ", sta_num, ac_num);

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
				MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				if (((sta_pause[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_PRINT(" ctrl = %s", sta_ctrl_reg[ctrl]);
				MTWF_PRINT(" (wmmidx=%d)\n", wmmidx);

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

	MTWF_PRINT("Nonempty TXCMD Q info:\n");
	for (i = 0; i < 32; i++) {
		if (((ple_txcmd_stat & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_txcmd_queue_empty_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", ple_txcmd_queue_empty_info[i].QueueName);
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
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}
}

static VOID chip_get_ple_acq_stat(RTMP_ADAPTER *pAd, UINT32 *ple_stat)
{
	INT32 j = 0;
	UINT32 addr = 0;
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_QUEUE_EMPTY_ADDR, &ple_stat[0]);

	addr = WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR;
	for (j = 0; j < CR_NUM_OF_AC; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);

	addr = WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR;
	for (; j < CR_NUM_OF_AC * 2; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);

	addr = WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR;
	for (; j < CR_NUM_OF_AC * 3; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);

	addr = WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR;
	for (; j < CR_NUM_OF_AC * 4; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &ple_stat[j + 1]);
}

static VOID chip_get_ple_txcmd_stat(RTMP_ADAPTER *pAd, UINT32 *ple_txcmd_stat)
{
	HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_NATIVE_TXCMD_QUEUE_EMPTY_ADDR, ple_txcmd_stat);
}

static VOID chip_get_dis_sta_map(RTMP_ADAPTER *pAd, UINT32 *dis_sta_map)
{
	INT32 j = 0;
	UINT32 addr = WF_PLE_TOP_DIS_STA_MAP0_ADDR;

	for (j = 0; j < CR_NUM_OF_AC; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &dis_sta_map[j]);
}

static VOID chip_get_sta_pause(RTMP_ADAPTER *pAd, UINT32 *sta_pause)
{
	INT32 j = 0;
	UINT32 addr = WF_PLE_TOP_STATION_PAUSE0_ADDR;

	for (j = 0; j < CR_NUM_OF_AC; j++, addr += 4)
		HW_IO_READ32(pAd->hdev_ctrl, addr, &sta_pause[j]);
}

static INT32 chip_show_drr_info(struct hdev_ctrl *ctrl, RTMP_STRING *arg)
{


#define DL_AC_START 0x00
#define DL_AC_END   39
#define UL_AC_START 0x10
#define UL_AC_END   0x1F
#define MAX_STA_WORD_NUM ((MAX_LEN_OF_MAC_TABLE + 31)/32)

	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 drr_ctrl_def_val = 0x80220000, drr_ctrl_val = 0;
	UINT32 drr_sta_status[((MAX_STA_WORD_NUM+7)/8)*8] = {0};
	UINT j, idx = 0, sta_line = 0, sta_no = 0;

	MTWF_PRINT("DRR Table STA Info:\n");

	for (idx = DL_AC_START; idx <= DL_AC_END; idx++) {
		BOOLEAN sta[MAX_LEN_OF_MAC_TABLE] = {0};
		BOOLEAN is_show = FALSE;
		for (j = 0; j < (MAX_LEN_OF_MAC_TABLE+255)/256; j++) {
			drr_ctrl_val = (drr_ctrl_def_val | idx | (j << 10));

		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);

			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7+j*8]);
		}

		for (sta_line = 0; sta_line < MAX_STA_WORD_NUM; sta_line++) {
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
			MTWF_PRINT("\tDL AC%02d Queue Non-Empty STA:\n", idx);

			for (sta_line = 0; sta_line < MAX_LEN_OF_MAC_TABLE; sta_line++) {
				if (sta[sta_line])
					MTWF_PRINT("%d ", sta_line);
			}

			MTWF_PRINT("\n");
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

		for (sta_line = 0; sta_line < MAX_STA_WORD_NUM; sta_line++) {
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
			MTWF_PRINT("\tUL AC%02d Queue Non-Empty STA:\n", idx);

			for (sta_line = 0; sta_line < MAX_LEN_OF_MAC_TABLE; sta_line++) {
				if (sta[sta_line])
					MTWF_PRINT("%d ", sta_line);
			}

			MTWF_PRINT("\n");
		}

	}

	for (idx = DL_AC_START; idx <= DL_AC_END; idx++) {
		BOOLEAN is_show = TRUE;

		drr_ctrl_def_val = 0x80420000;
		for (j = 0; j < (MAX_LEN_OF_MAC_TABLE+255)/256; j++) {

			drr_ctrl_val = (drr_ctrl_def_val | idx | (j << 10));
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
			udelay(500);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7+j*8]);
		}
		if (is_show) {
			MTWF_PRINT("\nBSSGrp[%d]:\n", idx);

			for (sta_line = 0; sta_line < MAX_STA_WORD_NUM; sta_line++) {
				MTWF_PRINT("0x%08X ", drr_sta_status[sta_line]);

				if ((sta_line % 4) == 3)
					MTWF_PRINT("\n");
			}
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
	MTWF_PRINT("PLE Configuration Info:\n");
	MTWF_PRINT("\tPacket Buffer Control(0x82060014): 0x%08x\n", ple_buf_ctrl);
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	MTWF_PRINT("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64));
	MTWF_PRINT("\t\tPage Offset=%d(in unit of 2KB)\n",
			 (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	MTWF_PRINT("\t\tTotal Page=%d pages\n", pg_num);
	/* Page Flow Control */
	MTWF_PRINT("PLE Page Flow Control:\n");
	MTWF_PRINT("\tFree page counter(0x820c0100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	MTWF_PRINT("\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	MTWF_PRINT("\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	MTWF_PRINT("\tFree page head and tail(0x820c0104): 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	MTWF_PRINT("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	MTWF_PRINT("\tReserved page counter of HIF group(0x820c0110): 0x%08x\n", pg_flow_ctrl[2]);
	MTWF_PRINT("\tHIF group page status(0x820c0114): 0x%08x\n", pg_flow_ctrl[3]);
	hif_min_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n", hif_max_q, hif_min_q);
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n", upg_hif, rpg_hif);

	MTWF_PRINT("\tReserved page counter of HIF_TXCMD group(0x820c0118): 0x%08x\n", pg_flow_ctrl[6]);
	MTWF_PRINT("\tHIF_TXCMD group page status(0x820c011c): 0x%08x\n", pg_flow_ctrl[7]);
	cpu_min_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	MTWF_PRINT("\tReserved page counter of CPU group(0x820c0150): 0x%08x\n", pg_flow_ctrl[4]);
	MTWF_PRINT("\tCPU group page status(0x820c0154): 0x%08x\n", pg_flow_ctrl[5]);
	cpu_min_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	MTWF_PRINT("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	MTWF_PRINT("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {


		for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) {
			if (j % CR_NUM_OF_AC == 0) {
				MTWF_PRINT("\n\tNonempty AC%d Q of STA#: ", j / CR_NUM_OF_AC);
			}

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
					MTWF_PRINT("%d ", i + (j % CR_NUM_OF_AC) * 32);
				}
			}
		}

		MTWF_PRINT("\n");
	}

	MTWF_PRINT("non-native/native txcmd queue empty = %d/%d\n", ple_txcmd_stat, ple_native_txcmd_stat);

	MTWF_PRINT("Nonempty Q info:\n");

	for (i = 0; i < 32; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_queue_empty_info[i].QueueName != NULL) {
				MTWF_PRINT("\t%s: ", ple_queue_empty_info[i].QueueName);
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
			MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);

			if (pktcnt > 0 && dumptxd > 0)
				ShowTXDInfo(pAd, hfid);
		}
	}

	chip_show_sta_acq_info(pAd, ple_stat, sta_pause, dis_sta_map, dumptxd);
	chip_show_txcmdq_info(pAd, ple_native_txcmd_stat);
	chip_show_peekcr_info(pAd, WF_PLE_TOP_PEEK_CR_00_PEEK_CR_00_ADDR, WF_PLE_TOP_PEEK_CR_11_PEEK_CR_11_ADDR);

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

	MTWF_PRINT("TXD counter status of MSDU:\n");

	for (i = 0; i < 8; i++)
		total_amsdu += ple_stat[i];

	for (i = 0; i < 8; i++) {
		MTWF_PRINT("AMSDU pack count of %d MSDU in TXD: 0x%x ", i+1, ple_stat[i]);
		if (total_amsdu != 0)
			MTWF_PRINT("(%d%%)\n", ple_stat[i] * 100 / total_amsdu);
		else
			MTWF_PRINT("\n");
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

static UINT32
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

static UINT32
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

	MTWF_PRINT("\n\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
			  lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);

	/* LMAC WTBL DW 0 */
	MTWF_PRINT("\nLWTBL DW 0/1\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_PEER_INFO_DW_0*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW0[i].name) {

		if (WTBL_LMAC_DW0[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW0[i].name,
					 (dw_value & WTBL_LMAC_DW0[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW0[i].name,
					  (dw_value & WTBL_LMAC_DW0[i].mask) >> WTBL_LMAC_DW0[i].shift);

		if (WTBL_LMAC_DW0[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 2\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW2[i].name) {

		if (WTBL_LMAC_DW2[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW2[i].name,
					 (dw_value & WTBL_LMAC_DW2[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW2[i].name,
					  (dw_value & WTBL_LMAC_DW2[i].mask) >> WTBL_LMAC_DW2[i].shift);
		if (WTBL_LMAC_DW2[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 3\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3[i].name) {

		if (WTBL_LMAC_DW3[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW3[i].name,
					 (dw_value & WTBL_LMAC_DW3[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW3[i].name,
					  (dw_value & WTBL_LMAC_DW3[i].mask) >> WTBL_LMAC_DW3[i].shift);
		if (WTBL_LMAC_DW3[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 4\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW4[i].name) {
		if (WTBL_LMAC_DW4[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW4[i].name,
					 (dw_value & WTBL_LMAC_DW4[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW4[i].name,
					  (dw_value & WTBL_LMAC_DW4[i].mask) >> WTBL_LMAC_DW4[i].shift);

		if (WTBL_LMAC_DW4[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 5\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5[i].name) {
		if (WTBL_LMAC_DW5[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW5[i].name,
					 (dw_value & WTBL_LMAC_DW5[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW5[i].name,
					  (dw_value & WTBL_LMAC_DW5[i].mask) >> WTBL_LMAC_DW5[i].shift);
		if (WTBL_LMAC_DW5[i].new_line) {
			MTWF_PRINT("\n");
			MTWF_PRINT("\t");
		} else
			MTWF_PRINT("/ ");
		i++;
	}
}

static VOID parse_fmac_lwtbl_DW6(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	int i = 0;

	/* LMAC WTBL DW 6 */
	MTWF_PRINT("\nLWTBL DW 6\n");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_6*4]);
	dw_value = *addr;

	MTWF_PRINT("\tTID 0/1/2/3/4/5/6/7 BA_WIN_SIZE:");

	for (i = 0; i < 8; i++)
		if (i != 7)
			MTWF_PRINT("%lu/", ((dw_value & BITS(i*4, i*4+3)) >> i*4));
		else
			MTWF_PRINT("%lu\n", ((dw_value & BITS(i*4, i*4+3)) >> i*4));
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
	MTWF_PRINT("\nLWTBL DW 7\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_7*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW7[i].name) {
		if (WTBL_LMAC_DW7[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW7[i].name,
					 (dw_value & WTBL_LMAC_DW7[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW7[i].name,
					  (dw_value & WTBL_LMAC_DW7[i].mask) >> WTBL_LMAC_DW7[i].shift);
		if (WTBL_LMAC_DW7[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 8\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW8[i].name) {
		if (WTBL_LMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW8[i].name,
					 (dw_value & WTBL_LMAC_DW8[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW8[i].name,
					  (dw_value & WTBL_LMAC_DW8[i].mask) >> WTBL_LMAC_DW8[i].shift);
		if (WTBL_LMAC_DW8[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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

static RTMP_STRING *fcap_name[] = {"20MHz", "20/40MHz", "20/40/80MHz", "20/40/80/160/80+80MHz"};

static VOID parse_fmac_lwtbl_DW9(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_16 i = 0;

	/* LMAC WTBL DW 8 */
	MTWF_PRINT("\nLWTBL DW 9\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_9*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW9[i].name) {
		if (WTBL_LMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW9[i].name,
					 (dw_value & WTBL_LMAC_DW9[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW9[i].name,
					  (dw_value & WTBL_LMAC_DW9[i].mask) >> WTBL_LMAC_DW9[i].shift);
		if (WTBL_LMAC_DW9[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
		i++;
	}

	/* FCAP parser */
	MTWF_PRINT("FCAP:%s\n", fcap_name[(dw_value & WTBL_FCAP_20_TO_160_MHZ) >> WTBL_FCAP_20_TO_160_MHZ_OFFSET]);
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

	MTWF_PRINT("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
			  rate_idx + 1, txrate,
			  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
			  mcs, hw_rate_str(txmode, mcs), nss, stbc);
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
	MTWF_PRINT("\nLWTBL DW 10\n");
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
	MTWF_PRINT("\nLWTBL DW 11\n");
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
	MTWF_PRINT("\nLWTBL DW 12\n");
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
	MTWF_PRINT("\nLWTBL DW 13\n");
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
	MTWF_PRINT("\nLWTBL DW 28\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28[i].name) {
		if (WTBL_LMAC_DW28[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW28[i].name,
					 (dw_value & WTBL_LMAC_DW28[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW28[i].name,
					  (dw_value & WTBL_LMAC_DW28[i].mask) >> WTBL_LMAC_DW28[i].shift);

		if (WTBL_LMAC_DW28[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 29\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW29[i].name) {
		if (WTBL_LMAC_DW29[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW29[i].name,
					 (dw_value & WTBL_LMAC_DW29[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW29[i].name,
					  (dw_value & WTBL_LMAC_DW29[i].mask) >> WTBL_LMAC_DW29[i].shift);

		if (WTBL_LMAC_DW29[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 30\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30[i].name) {
		if (WTBL_LMAC_DW30[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW30[i].name,
					 (dw_value & WTBL_LMAC_DW30[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW30[i].name,
					  (dw_value & WTBL_LMAC_DW30[i].mask) >> WTBL_LMAC_DW30[i].shift);

		if (WTBL_LMAC_DW30[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
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
	MTWF_PRINT("\nLWTBL DW 31\n\t");
	addr = (UINT_32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_4*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW31[i].name) {
		if (WTBL_LMAC_DW31[i].shift == NO_SHIFT_DEFINE)
			MTWF_PRINT("%s:%d", WTBL_LMAC_DW31[i].name,
					 (dw_value & WTBL_LMAC_DW31[i].mask) ? 1 : 0);
		else
			MTWF_PRINT("%s:%u", WTBL_LMAC_DW31[i].name,
					  (dw_value & WTBL_LMAC_DW31[i].mask) >> WTBL_LMAC_DW31[i].shift);
		if (WTBL_LMAC_DW31[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
		i++;
	}
}

static VOID parse_fmac_lwtbl_rx_stats(RTMP_ADAPTER *pAd, UINT8 *lwtbl)
{
	parse_fmac_lwtbl_DW28(pAd, lwtbl);
	parse_fmac_lwtbl_DW29(pAd, lwtbl);
	parse_fmac_lwtbl_DW30(pAd, lwtbl);
	parse_fmac_lwtbl_DW31(pAd, lwtbl);
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
	MTWF_PRINT("\nUWTBL PN\n\t");

	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_DW_0*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW0[i].name) {
		MTWF_PRINT("%s:%u", WTBL_UMAC_DW0[i].name,
				  (dw_value & WTBL_UMAC_DW0[i].mask) >> WTBL_UMAC_DW0[i].shift);
		if (WTBL_UMAC_DW0[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
		i++;
	}

	i = 0;
	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW1[i].name) {
		MTWF_PRINT("%s:%u", WTBL_UMAC_DW1[i].name,
				  (dw_value & WTBL_UMAC_DW1[i].mask) >> WTBL_UMAC_DW1[i].shift);
		if (WTBL_UMAC_DW1[i].new_line)
			MTWF_PRINT("\n\t");
		else
			MTWF_PRINT("/ ");
		i++;
	}
}

static VOID parse_fmac_uwtbl_sn(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 u2SN = 0;

	/* UMAC WTBL DW SN part */
	MTWF_PRINT("\nUWTBL SN\n");

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID0_AC0_SN_MASK) >> WTBL_TID0_AC0_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID0_AC0_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID1_AC1_SN_MASK) >> WTBL_TID1_AC1_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID1_AC1_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID2_AC2_SN_0_7_MASK) >> WTBL_TID2_AC2_SN_0_7_OFFSET;
	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN |= (((*addr) & WTBL_TID2_AC2_SN_8_11_MASK) >> WTBL_TID2_AC2_SN_8_11_OFFSET) << 8;
	MTWF_PRINT("\t%s:%u\n", "TID2_AC2_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID3_AC3_SN_MASK) >> WTBL_TID3_AC3_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID3_AC3_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID4_SN_MASK) >> WTBL_TID4_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID4_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID5_SN_0_3_MASK) >> WTBL_TID5_SN_0_3_OFFSET;
	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN |= (((*addr) & WTBL_TID5_SN_4_11_MASK) >> WTBL_TID5_SN_4_11_OFFSET) << 4;
	MTWF_PRINT("\t%s:%u\n", "TID5_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID6_SN_MASK) >> WTBL_TID6_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID6_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID7_SN_MASK) >> WTBL_TID7_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "TID6_SN", u2SN);

	addr = (UINT_32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	u2SN = ((*addr) & WTBL_COM_SN_MASK) >> WTBL_COM_SN_OFFSET;
	MTWF_PRINT("\t%s:%u\n", "COM_SN", u2SN);
}

static VOID dump_key_table(RTMP_ADAPTER *pAd, UINT16 keyloc0, UINT16 keyloc1)
{
	UINT8 keytbl[ONE_KEY_ENTRY_LEN_IN_DW*4] = {0};
	UINT16 x;

	MTWF_PRINT("\n\t%s:%d\n", "keyloc0", keyloc0);
	if (keyloc0 != (WTBL_KEY_LINK_DW_KEY_LOC0_MASK >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET)) {

		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(pAd, keyloc0, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_PRINT("KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc0, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]);
		}
	}

	MTWF_PRINT("\t%s:%d\n", "keyloc1", keyloc1);
	if (keyloc1 != (WTBL_KEY_LINK_DW_KEY_LOC1_MASK >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET)) {
		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(pAd, keyloc1, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		MTWF_PRINT("KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc1, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]);
		}
	}
}

static VOID parse_fmac_uwtbl_others(RTMP_ADAPTER *pAd, UINT8 *uwtbl)
{
	UINT_32 *addr = 0;
	UINT_32 dw_value = 0;
	UINT_32 amsdu_len = 0;

	/* UMAC WTBL DW 0 */
	MTWF_PRINT("\nUWTBL others\n");

	addr = (UINT_32 *)&(uwtbl[UWTBL_KEY_LINK_DW*4]);
	dw_value = *addr;

	MTWF_PRINT("\t%s:%lu/%lu\n", "Key Loc 1/2",
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC0_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET,
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC1_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET
					);

	MTWF_PRINT("\t%s:%d\n", "UWTBL_QOS",
					(dw_value & WTBL_QOS_MASK) ? 1 : 0
					);

	MTWF_PRINT("\t%s:%d\n", "UWTBL_HT_VHT_HE",
					(dw_value & WTBL_HT_VHT_HE_MASK) ? 1 : 0
					);

	addr = (UINT_32 *)&(uwtbl[UWTBL_HW_AMSDU_DW*4]);
	dw_value = *addr;

	MTWF_PRINT("\t%s:%d\n", "HW AMSDU Enable",
					(dw_value & WTBL_AMSDU_EN_MASK) ? 1 : 0
					);

	amsdu_len = (dw_value & WTBL_AMSDU_LEN_MASK) >> WTBL_AMSDU_LEN_OFFSET;
	if (amsdu_len == 0)
		MTWF_PRINT("\t%s:invalid (WTBL value=0x%x)\n", "HW AMSDU Len",
						amsdu_len
						);
	else if (amsdu_len == 1)
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						1,
						255,
						amsdu_len
						);
	else
		MTWF_PRINT("\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						256 * (amsdu_len - 1),
						256 * (amsdu_len - 1) + 255,
						amsdu_len
						);

	MTWF_PRINT("\t%s:%lu (WTBL value=0x%lx)\n", "HW AMSDU Num",
					((dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET) + 1,
					(dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET
					);

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
	parse_fmac_lwtbl_DW3(pAd, lwtbl);
	parse_fmac_lwtbl_DW4(pAd, lwtbl);
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
	MTWF_PRINT("WTBL Basic Info:\n");
}

static VOID chip_dump_wtbl_info(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	UINT8 lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	UINT8 uwtbl[UWTBL_LEN_IN_DW*4] = {0};
	int x;
	UINT8 real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);
	MTWF_PRINT("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx);
	MTWF_PRINT("LMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_WTBLON_TOP_WDUCR_ADDR,
								IO_R_32(WF_WTBLON_TOP_WDUCR_ADDR),
								LWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < real_lwtbl_size; x++) {
		MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
									x,
									lwtbl[x * 4 + 3],
									lwtbl[x * 4 + 2],
									lwtbl[x * 4 + 1],
									lwtbl[x * 4]);
	}

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW, uwtbl);
	MTWF_PRINT("UMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_UWTBL_TOP_WDUCR_ADDR,
								IO_R_32(WF_UWTBL_TOP_WDUCR_ADDR),
								UWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < UWTBL_LEN_IN_DW; x++) {
		MTWF_PRINT("DW%02d: %02x %02x %02x %02x\n",
									x,
									uwtbl[x * 4 + 3],
									uwtbl[x * 4 + 2],
									uwtbl[x * 4 + 1],
									uwtbl[x * 4]);
	}

	dump_fmac_wtbl_info(pAd, lwtbl, uwtbl);
}

static VOID chip_dump_wtbl_mac(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	UINT8 lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	UINT8 real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(pAd, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);

	MTWF_PRINT("WLAN_IDX: %d Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  wtbl_idx, lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);
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

	MTWF_PRINT("%22s %10x %10x %10x %10x %10x\n",
		s, base, cnt, cidx, didx, queue_cnt);
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

	MTWF_PRINT("%22s %10x %10x %10x %10x %10x\n",
		s, base, cnt, cidx, didx, queue_cnt);
}

static VOID chip_show_dma_info(struct hdev_ctrl *ctrl)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);

	if (IS_PCI_INF(pAd)) {
		UINT32 sys_ctrl[10] = {0};

		/* HOST DMA */
		HIF_IO_READ32(ctrl, MT_INT_SOURCE_CSR, &sys_ctrl[0]);
		HIF_IO_READ32(ctrl, MT_INT_MASK_CSR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[2]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[3]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[4]);

		MTWF_PRINT("HOST_DMA Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x\n",
				"Merge", sys_ctrl[0], sys_ctrl[1]);
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA0", sys_ctrl[2], sys_ctrl[3], sys_ctrl[4],
				(sys_ctrl[4] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[4] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[4] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[4] & WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("HOST_DMA0 Ring Configuration\n");
		MTWF_PRINT("%22s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T17:Cmd (H2WM)", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_CTRL0_ADDR);
#ifdef WFDMA_WED_COMPATIBLE
		dump_dma_tx_ring_info(ctrl, "T18:TXD (H2WA)", WF_WFDMA_EXT_WRAP_CSR_WED_TX0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T19:TXD1 (H2WA)", WF_WFDMA_EXT_WRAP_CSR_WED_TX1_CTRL0_ADDR);
#else
		dump_dma_tx_ring_info(ctrl, "T18:TXD (H2WA)", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T19:TXD1 (H2WA)", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL0_ADDR);
#endif /* WFDMA_WED_COMPATIBLE */

		if (MT7916_get_rid_value() != DEFAULT_RID)
			dump_dma_tx_ring_info(ctrl, "T19:TXD1_PCI1 (H2WA)", WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_TX_RING19_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T20:Cmd (H2WA)", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING20_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:Event (WM2H)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Event (WA2H)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);
#ifdef WFDMA_WED_COMPATIBLE
		dump_dma_rx_ring_info(ctrl, "R2:TxDone (WA2H)", WF_WFDMA_EXT_WRAP_CSR_WED_RX1_CTRL0_ADDR);
#else
		dump_dma_rx_ring_info(ctrl, "R2:TxDone (WA2H)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR);
#endif /* WFDMA_WED_COMPATIBLE */
		dump_dma_rx_ring_info(ctrl, "R3:TxDone1 (WA2H)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR);
		if (MT7916_get_rid_value() != DEFAULT_RID)
			dump_dma_rx_ring_info(ctrl, "R3:TxDone1_PCI1 (WA2H)", WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:Data (MAC2H)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R5:Data1 (MAC2H)",	WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR);
		if (MT7916_get_rid_value() != DEFAULT_RID)
			dump_dma_rx_ring_info(ctrl, "R5:Data1_PCI1 (MAC2H)", WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING5_CTRL0_ADDR);

		/* MCU DMA information */
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MCU_DMA0_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("MCU_DMA Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"DMA0", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MCU_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("MCU_DMA0 Ring Configuration\n");
		MTWF_PRINT("%22s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T0:Event (WM2H)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:Event (WA2H)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T2:TxDone (WA2H)",	WF_WFDMA_MCU_DMA0_WPDMA_TX_RING2_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T3:TxDone1 (WA2H)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING3_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T4:TXD (WM2MAC)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING4_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T5:TXCMD (WM2MAC)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING5_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T6:TXD (WA2MAC)", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING6_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:FWDL", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:Cmd (H2WM)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R2:TXD (H2WA)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING2_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R3:TXD1 (H2WA)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R4:Cmd (H2WA)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING4_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R5:Data (MAC2WM)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING5_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R6:TxDone/STS (MAC2WM)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING6_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R7:RPT (MAC2WM)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING7_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R8:TxDone/STS (MAC2WA)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING8_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R9:Data1 (MAC2WM)", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING9_CTRL0_ADDR);

		/* MEM DMA information */
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_ADDR, &sys_ctrl[0]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_HOST_INT_STA_ADDR, &sys_ctrl[1]);
		RTMP_IO_READ32(ctrl, WF_WFDMA_MEM_DMA_HOST_INT_ENA_ADDR, &sys_ctrl[2]);

		MTWF_PRINT("MEM_DMA Configuration\n");
		MTWF_PRINT("%10s %10s %10s %10s %10s %10s\n",
				"DMA", "IntCSR", "IntMask", "Glocfg", "Tx/RxEn", "Tx/RxBusy");
		MTWF_PRINT("%10s %10x %10x %10x %4x/%5x %4x/%5x\n",
				"MEM", sys_ctrl[1], sys_ctrl[2], sys_ctrl[0],
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_EN_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_TX_DMA_BUSY_SHFT,
				(sys_ctrl[0] & WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK) >> WF_WFDMA_MEM_DMA_WPDMA_GLO_CFG_RX_DMA_BUSY_SHFT);

		MTWF_PRINT("MEM_DMA Ring Configuration\n");
		MTWF_PRINT("%22s %10s %10s %10s %10s %10s\n",
				"Name", "Base", "Cnt", "CIDX", "DIDX", "QCnt");
		dump_dma_tx_ring_info(ctrl, "T0:CmdEvent (WM2WA)", WF_WFDMA_MEM_DMA_WPDMA_TX_RING0_CTRL0_ADDR);
		dump_dma_tx_ring_info(ctrl, "T1:CmdEvent (WA2WM)", WF_WFDMA_MEM_DMA_WPDMA_TX_RING1_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R0:CmdEvent (WM2WA)", WF_WFDMA_MEM_DMA_WPDMA_RX_RING0_CTRL0_ADDR);
		dump_dma_rx_ring_info(ctrl, "R1:CmdEvent (WA2WM)", WF_WFDMA_MEM_DMA_WPDMA_RX_RING1_CTRL0_ADDR);
	}
}

#ifdef VOW_SUPPORT
static UINT32 chip_get_sta_airtime(RTMP_ADAPTER *pAd, UINT16 sta, UINT16 ac, BOOLEAN tx)
{
	UINT32 airtime;
	UINT32 wtbl_offset = 20;

	if (tx)
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1), 1, &airtime);
	else
		halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + (ac << 1) + 1, 1, &airtime);

	return airtime;
}

static UINT32 chip_get_sta_addr(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 addr;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 0, 1, &addr);
	return addr;
}

static UINT32 chip_get_sta_rate(RTMP_ADAPTER *pAd, UINT32 sta)
{
	UINT32 rate;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, 10, 1, &rate);
	return rate;
}

static UINT32 chip_get_sta_tx_cnt(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 bw)
{
	UINT32 tx_cnt;
	UINT32 wtbl_offset = 16;

	halWtblReadRaw(pAd, sta, WTBL_TYPE_LMAC, wtbl_offset + bw, 1, &tx_cnt);
	return tx_cnt;
}

static INT32 chip_set_sta_psm(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 psm)
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

static VOID chip_show_bss_bitmap(RTMP_ADAPTER *pAd, UINT32 start, UINT32 end)
{
	UINT32 drr_ctrl_def_val, drr_ctrl_val;
#define MAX_STA_WORD_NUM ((MAX_LEN_OF_MAC_TABLE + 31)/32)
	UINT32 drr_sta_status[((MAX_STA_WORD_NUM+7)/8)*8] = {0};
	UINT j, idx = 0, sta_line = 0;

	start = start + UMAC_BWC_GROUP_MIN;
	end = end + UMAC_BWC_GROUP_MIN;

	for (idx = start; idx <= end; idx++) {
		drr_ctrl_def_val = 0x80420000;
		for (j = 0; j < (MAX_LEN_OF_MAC_TABLE+255)/256; j++) {

			drr_ctrl_val = (drr_ctrl_def_val | idx | (j << 10));
			HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
			udelay(500);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_sta_status[0+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_sta_status[1+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_sta_status[2+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_sta_status[3+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA4_ADDR, &drr_sta_status[4+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA5_ADDR, &drr_sta_status[5+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA6_ADDR, &drr_sta_status[6+j*8]);
			HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA7_ADDR, &drr_sta_status[7+j*8]);
		}
		MTWF_PRINT("\nBWCGrp[%d]:\n", idx);

		for (sta_line = 0; sta_line < MAX_STA_WORD_NUM; sta_line++) {
			MTWF_PRINT("0x%08X ", drr_sta_status[sta_line]);
			if ((sta_line % 4) == 3)
				MTWF_PRINT("\n");
		}
	}

}

static VOID chip_show_bss_setting(RTMP_ADAPTER *pAd, UINT32 start, UINT32 end)
{
	UINT32 drr_ctrl_def_val, drr_ctrl_val;
	UINT32 drr_setting_status[4] = {0};
	UINT idx = 0;

	for (idx = start; idx <= end; idx++) {
		drr_ctrl_def_val = 0x80400000;

		drr_ctrl_val = (drr_ctrl_def_val | idx);
		HW_IO_WRITE32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_CTRL_ADDR, drr_ctrl_val);
		udelay(500);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA0_ADDR, &drr_setting_status[0]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA1_ADDR, &drr_setting_status[1]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA2_ADDR, &drr_setting_status[2]);
		HW_IO_READ32(pAd->hdev_ctrl, WF_PLE_TOP_DRR_TABLE_RDATA3_ADDR, &drr_setting_status[3]);
		MTWF_PRINT("\nBWCGrpSet[%d]:\n", idx);
		MTWF_PRINT("0x%08X 0x%08X 0x%08X 0x%08X\n", drr_setting_status[0],
				drr_setting_status[1], drr_setting_status[2], drr_setting_status[3]);
	}
}

#endif	/* VOW_SUPPORT */

static UINT32 chip_get_lpon_frcr(RTMP_ADAPTER *pAd)
{
	UINT32 free_cnt = 0;

	HW_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_FRCR_ADDR, &free_cnt);
	return free_cnt;
}

static struct {
	RTMP_STRING *name;
	UINT32 reg_addr;
	UINT32 mask;
	UINT32 shift;
} TXV[] = {
	/* CG1 DDW1 H */
	{"ant id",			BN0_WF_TMAC_TOP_TXV1_ADDR,		BITS(8, 31),	8},
	/* CG1 DDW1 L */
	{"user count",		BN0_WF_TMAC_TOP_TXV0_ADDR,		BITS(24, 30),	24},
	{"phy mode",		BN0_WF_TMAC_TOP_TXV0_ADDR,		BITS(12, 15),	12},
	{"dbw",				BN0_WF_TMAC_TOP_TXV0_ADDR,		BITS(8, 10),	8},
	{"stbc",			BN0_WF_TMAC_TOP_TXV0_ADDR,		BITS(6, 7),		6},
	{"mu",				BN0_WF_TMAC_TOP_TXV0_ADDR,		BIT(5),			5},
	{"spe idx",			BN0_WF_TMAC_TOP_TXV0_ADDR,		BITS(0, 4),		0},
	/* CG1 DDW2 H */
	{"format",			BN0_WF_TMAC_TOP_TXV3_ADDR,		BIT(30),		30},
	{"HE LTF",			BN0_WF_TMAC_TOP_TXV3_ADDR,		BITS(28, 29),	28},
	{"HE GI",			BN0_WF_TMAC_TOP_TXV3_ADDR,		BITS(26, 27),	26},
	{"UDL",				BN0_WF_TMAC_TOP_TXV3_ADDR,		BIT(25),		25},
	{"Max TX NSTS",		BN0_WF_TMAC_TOP_TXV3_ADDR,		BITS(12, 14),	12},
	/* CG1 DDW2 L */
	{"SIGB Comp.",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BIT(31),		31},
	{"SIGB Nsym.",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(24, 30),	24},
	{"SIGB DCM",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BIT(23),		23},
	{"SIGB MCS",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(20, 22),	20},
	{"MIMO user",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(16, 19),	16},
	{"Center 26",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(14, 15),	14},
	{"CH1 STAs",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(8, 13),	8},
	{"CH0 STAs",		BN0_WF_TMAC_TOP_TXV2_ADDR,		BITS(0, 5),		0},
	/* CG2 DDW1 H */
	{"LG LEN EXTS",		BN0_WF_TMAC_TOP_TXV5_ADDR,		BITS(20, 31),	20},
	{"PE Disamb. EXT",	BN0_WF_TMAC_TOP_TXV5_ADDR,		BIT(15),		15},
	{"afactor",			BN0_WF_TMAC_TOP_TXV5_ADDR,		BITS(6, 7),		6},
	/* CG2 DDW1 L */
	{"PE Disamb.",		BN0_WF_TMAC_TOP_TXV4_ADDR,		BIT(31),		31},
	{"LTF Symbols",		BN0_WF_TMAC_TOP_TXV4_ADDR,		BITS(28, 30),	28},
	{"Symbols count",	BN0_WF_TMAC_TOP_TXV4_ADDR,		BITS(16, 27),	16},
	{"Max TPE",			BN0_WF_TMAC_TOP_TXV4_ADDR,		BITS(13, 14),	13},
	{"LDPC Extr Sym",	BN0_WF_TMAC_TOP_TXV4_ADDR,		BIT(12),		12},
	{"LG LEN",			BN0_WF_TMAC_TOP_TXV4_ADDR,		BITS(0, 11),	0},
	/* CG2 DDW2 H, BN0_WF_TMAC_TOP_TXV7_ADDR */
	/* CG2 DDW2 L, BN0_WF_TMAC_TOP_TXV6_ADDR */
	/* PG1 DDW1 H, BN0_WF_TMAC_TOP_TXV9_ADDR*/
	/* PG1 DDW1 L, BN0_WF_TMAC_TOP_TXV8_ADDR */
	{"EBF MU IDX",		BN0_WF_TMAC_TOP_TXV8_ADDR,		BITS(24, 31),	24},
	{"RU allocation",	BN0_WF_TMAC_TOP_TXV8_ADDR,		BITS(16, 23),	16},
	{"MU GROUP",		BN0_WF_TMAC_TOP_TXV8_ADDR,		BITS(11, 15),	11},
	{"FEC Coding",		BN0_WF_TMAC_TOP_TXV8_ADDR,		BIT(7),			7},
	{"NSTS",			BN0_WF_TMAC_TOP_TXV8_ADDR,		BITS(8, 10),	8},
	{"ER-106T",			BN0_WF_TMAC_TOP_TXV8_ADDR,		BIT(5),			5},
	{"DCM",				BN0_WF_TMAC_TOP_TXV8_ADDR,		BIT(4),			4},
	{"Rate",			BN0_WF_TMAC_TOP_TXV8_ADDR,		BITS(0, 3),		0},
	/* PG2 DDW1 H */
	{"AID",				BN0_WF_TMAC_TOP_TXV11_ADDR,		BITS(0, 10),	0},
	/* PG2 DDW1 L */
	{"TX LEN",			BN0_WF_TMAC_TOP_TXV10_ADDR,		BITS(0, 22),	0},
	{0}
};
static UCHAR dump_txv_CR(IN struct hdev_ctrl *ctrl, IN UINT32 reg_addr, OUT PUINT32 reg_val)
{
	UCHAR valid = TRUE;

	*reg_val = 0;

	MAC_IO_READ32(ctrl, reg_addr, reg_val);

	return valid;
}

static INT32 chip_check_txv(struct hdev_ctrl *ctrl, UCHAR *name, UINT32 value, UINT8 band_idx)
{
	UCHAR found = 0, txv_idx = 0;
	UINT32 reg_val = 0;
	UINT32 band_addr_offset = 0x10000;


	if (strlen(name) > 0) {
		while (TXV[txv_idx].name) {
			if (!strcmp(TXV[txv_idx].name, name)) {
				dump_txv_CR(ctrl, TXV[txv_idx].reg_addr + (band_addr_offset * band_idx), &reg_val);

				reg_val &= TXV[txv_idx].mask;
				reg_val >>= TXV[txv_idx].shift;

				found = 1;
				break;
			}

			txv_idx++;
		};

		if (found) {
			if (reg_val == value) {
				MTWF_PRINT("%s: [Matched] %s = %d\n", __func__, TXV[txv_idx].name, value);
			} else {
				MTWF_PRINT("%s: [Mis-matched] %s = (%d:%d)\n", __func__, TXV[txv_idx].name, reg_val, value);
			}
		} else {
			MTWF_PRINT("%s: %s not found!\n", __func__, TXV[txv_idx].name);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: Unknown parameter name!\n", __func__);
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
	UINT32 phy1_cr_off = 0x100000;

	if (ctrl == HETB_TX_CFG) {
		if (ru_sta == NULL) {
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s[%d]: invalid input\n", __func__, __LINE__);

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
		MTWF_PRINT("%s: Step1: [CMM][%x][0x%llx]\n", __func__, BN0_WF_TMAC_TOP_TTRCR0_TF_COMINFO_B31B0_ADDR+(0x10000*band_idx), cmm.cmm_info);
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
		MTWF_PRINT("%s: Step1: [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR2_TF_USRINFO_B31B0_ADDR+(0x10000*band_idx), usr.usr_info);
		MTWF_PRINT("%s:        [USR][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_ADDR+(0x10000*band_idx), 0xef);
		/*  step 2, rssi report*/
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK);
		MTWF_PRINT("%s: Step2: [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR4_TF_RX_RSSI_20M_B26B0_MASK);
		MTWF_PRINT("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR5_TF_RX_RSSI_20M_B53B27_MASK);
		MTWF_PRINT("%s:        [RSSI][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_RSSI_20M_B71B54_MASK);
		cr_value = 0xffffffff;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK);
		MTWF_PRINT("%s:        [BWD][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_ADDR+(0x10000*band_idx), cr_value & BN0_WF_TMAC_TOP_TTRCR6_TF_RX_BWD_20M_MASK);
		/* step 3, channel information */
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), cr_value & ~BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK);
		MTWF_PRINT("%s: Step3: [CBW Mode][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_STA_CBW_MODE_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & ~BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK));
		MTWF_PRINT("%s:        [CBW 160NC IND][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_CBW_160NC_IND_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), &cr_value);
		cr_value |= ((TESTMODE_GET_PARAM(ad, band_idx, pri_sel) << BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_SHFT) & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), cr_value);
		MTWF_PRINT("%s:        [CBW PRIM20 CH][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_PRIM20M_CH_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(phy1_cr_off*band_idx), &cr_value);
		cr_value &= ~(BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MASK | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MAN_MASK);
		cr_value |= BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_TOTAL_TXPWR_IND_MAN_MASK;
		cr_value &= ~BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_FCC_PWR_SKU2_MASK;
		cr_value |= (0x7f | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_CR_BAND_TPC_FCC_PWR_SKU2_MAN_MASK);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: Step4: [TXPWR IND/SKU][%x][0x%04x]\n", __func__, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_01_ADDR+(phy1_cr_off*band_idx), cr_value);
		/* setup MAC end */
	} else if (ctrl == HETB_TX_START) {
		/*  step 6. Set 1 to TTRCR3.TF_RESP_TEST_MODE*/
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK);
		MTWF_PRINT("%s: Step6: [TF_RESP_TEST_MODE][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value | BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK);
		/* SA TXPG: 2021/05/19, RU26~RU106 TPC workaround */
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), cr_value | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_MASK);
		MTWF_PRINT("%s: Step6: [MAC2PHY TOTAL-POWER][%x][0x%04x]\n", __func__, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), cr_value | BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_MASK);
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK));
		MTWF_PRINT("%s: [Proactive HETB TX turned off][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_ADDR+(0x10000*band_idx), cr_value & ~(BN0_WF_TMAC_TOP_TTRCR3_TF_RESP_TEST_MODE_MASK | BN0_WF_TMAC_TOP_TTRCR3_TF_USRINFO_B39B32_MASK));
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), &cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), cr_value & ~(BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_MASK));
		MTWF_PRINT("%s: [MAC2PHY TOTAL-POWER off][%x][0x%04x]\n", __func__, BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_ADDR+(phy1_cr_off*band_idx), cr_value & ~BN0_PHYDFE_CTRL_CR_BAND_TPC_CTL_00_CR_BAND_TPC_TOTAL_PWR_HETB_MAN_MASK);
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
	UINT32 phy1_cr_off = 0x100000;
	UINT32 *phy_rx_ctrl = NULL;

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
		MTWF_PRINT("%s: [MAC]0x%x=0x%llx\n", __func__, BN0_WF_RMAC_TOP_TF_USERTONE0_ADDR+(0x10000*band_idx), mac_cr_value);
		/* end MAC start */
		/* setup PHY start */
		/* cycle 0: start manual hetb rx (without TF) */
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
		cr_value |= 0x1;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [Start]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		cr_value |= 0x2;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s:        0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		/* cycle 1:CSD part */
		cr_value = (csd & 0xffffffff);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [CSD_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), cr_value);
		cr_value = (csd & 0xffffffff00000000) >> 32;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [CSD_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), cr_value);

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
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
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [CMM_H]0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), cr_value);
		cr_value = (cmm.cmm_info >> 32);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [CMM_L]0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), cr_value);

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
		cr_value |= 0x8;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		cr_value &= 0xfffffff3;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
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
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), usr.usr_info);
			MTWF_PRINT("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), usr.usr_info);

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
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), usr.usr_info);
			MTWF_PRINT("%s: [USR%d]0x%x=[0x%x]\n", __func__, usr_grp_idx*2+1, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), usr.usr_info);

			RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
			cr_value |= 0x8;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
			MTWF_PRINT("%s: [Assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
			cr_value &= 0xfffffff3;
			RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
			MTWF_PRINT("%s: [De-assert Write]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		}

		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
		cr_value |= 0x4;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s: [Submit]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		/* setup PHY end */
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), &cr_value);
		cr_value &= 0xfffffff0;
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), 0);
		RTMP_IO_WRITE32(ad->hdev_ctrl, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), 0);
		MTWF_PRINT("%s: [Stop]0x%x=0x%x\n", __func__, phy_rx_ctrl[0]+(phy1_cr_off*band_idx), cr_value);
		MTWF_PRINT("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[1]+(phy1_cr_off*band_idx), 0);
		MTWF_PRINT("%s:       0x%x=0x%x\n", __func__, phy_rx_ctrl[2]+(phy1_cr_off*band_idx), 0);
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
		MTWF_PRINT("%s: Ste3.1:[SPE index][%x][0x%04x]\n", __func__, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), (cr_value & BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK));
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TFCR0_HETB_SPE_IDX_ADDR+(0x10000*band_idx), cr_value);
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_NSS_FNL_ADDR+(0x10000*band_idx), &cr_value);
		cr_value &= ~BN0_WF_AGG_TOP_MUCR0_MU_NSS_FNL_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, BN0_WF_AGG_TOP_MUCR0_MU_NSS_FNL_ADDR+(0x10000*band_idx), cr_value);
	}

	return 0;
}
#endif

static UINT32 chip_show_asic_rx_stat(RTMP_ADAPTER *ad, UINT type)
{
	UINT32 value = 0;

	MTWF_PRINT("mt7916: %s, Type(%d)\n", __func__, type);

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

	MTWF_PRINT("%s, Type(%d):%x\n", __func__, type, value);
	return value;
}

static INT32 chip_show_ple_info_by_idx(struct hdev_ctrl *ctrl, UINT16 wtbl_idx)
{
	RTMP_ADAPTER *pAd = ctrl->priv;
	UINT32 ple_stat[ ALL_CR_NUM_OF_ALL_AC + 1] = {0};
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

				MTWF_PRINT("\tSTA%d AC%d: ", sta_num, ac_num);

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
				MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				if (((sta_pause[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				MTWF_PRINT(" ctrl = %s", sta_ctrl_reg[ctrl]);
				MTWF_PRINT(" (wmmidx=%d)\n", wmmidx);
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
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown DBW:%d\n", dbw);
		}
	} else {
		RTMP_IO_READ32(ad->hdev_ctrl, BN0_WF_MIB_TOP_M0DR8_ADDR + band_offset, &mac_val);
	}

	return mac_val;
}

static INT chip_chk_exception_type(RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UCHAR exp_assert_proc_entry_cnt = 0;
	UINT32 g1_exp_counter_addr = 0x022050BC;

	HW_IO_READ32(pAd->hdev_ctrl, g1_exp_counter_addr, &macVal);
	exp_assert_proc_entry_cnt = (macVal & 0xff);

#ifdef WHNAT_SUPPORT
	if (pAd->CommonCfg.whnat_en && MTK_REV_LT(pAd, MT7916, MT7916E2)) {
		UINT32 macVal_130, macVal_154;

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_AXI0_R2A_DMARD_PROBE_ADDR, &macVal);
		if ((macVal & BIT27) && !(macVal & BIT26)) {
			RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_DBG_IDX_ADDR, 0x130);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_DBG_PROBE_ADDR, &macVal_130);

			RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_MCU_DMA0_WPDMA_DBG_IDX_ADDR, 0x154);
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_MCU_DMA0_WPDMA_DBG_PROBE_ADDR, &macVal_154);

			if ((macVal_130 & BIT29) && ((macVal_154 & 0x1F) == 2) &&
				(((macVal_154 & 0xF000) == 0x2000) || ((macVal_154 & 0xF000) == 0x3000)))
					MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\nCTXD hang hit!!!\n");
		}
	}
#endif

	if (exp_assert_proc_entry_cnt == 0)
		return 0;
	else
		return 1;
}

static VOID ShowCpuUtilSum(RTMP_ADAPTER *pAd)
{
	UINT32 busy_perc = 0;
	UINT32 peak_busy_perc = 0;

	HW_IO_READ32(pAd->hdev_ctrl, 0x7C053B20, &busy_perc);
	HW_IO_READ32(pAd->hdev_ctrl, 0x7C053B24, &peak_busy_perc);

	MTWF_PRINT("\n\n       cpu ultility\n");
	MTWF_PRINT("       Busy:%d%% Peak:%d%%\n\n",
				busy_perc, peak_busy_perc);
}

#define SYSIRQ_INTERRUPT_HISTORY_NUM 10
static VOID ShowIrqHistory(RTMP_ADAPTER *pAd)
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
	HW_IO_READ32(pAd->hdev_ctrl, 0x02205288, &macVal);
	ucIrqResIdx = (macVal & 0xff);
	ucIrqDisIdx = ((macVal >> 8) & 0xff);

	MTWF_PRINT("\n\n\n       Irq Idx (Dis=%d Res=%d):\n",
		ucIrqDisIdx, ucIrqResIdx);

	HW_IO_READ32(pAd->hdev_ctrl, 0x02205290, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_dis_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_dis_lp[i] = macVal;
	}

	HW_IO_READ32(pAd->hdev_ctrl, 0x0220528C, &start);

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8)), &macVal);
		irq_res_time[i] = macVal;
		HW_IO_READ32(pAd->hdev_ctrl, (start + (i * 8) + 4), &macVal);
		irq_res_lp[i] = macVal;
	}

	MTWF_PRINT("\n       Dis Irq history (from old to new):\n");

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqDisIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_PRINT("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_dis_lp[idx], irq_dis_time[idx]);
	}

	MTWF_PRINT("\n       Restore Irq history (from old to new):\n");

	for (i = 0; i < SYSIRQ_INTERRUPT_HISTORY_NUM; i++) {
		idx = (i + ucIrqResIdx) % SYSIRQ_INTERRUPT_HISTORY_NUM;
		MTWF_PRINT("      [%d].LP = 0x%x   time=%u\n",
			idx, irq_res_lp[idx], irq_res_time[idx]);
	}

}

static VOID ShowLpHistory(RTMP_ADAPTER *pAd, BOOLEAN fgIsExp)
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

	MTWF_PRINT("       lp history (from old to new):\n");

	for (i = 0; i < 16; i++) {
		idx = ((oldest_idx + 2*i + 1)%32);
		HW_IO_READ32(pAd->hdev_ctrl, (0x89050204 + idx*4), &macVal);
		MTWF_PRINT("       %d: 0x%x\n", i, macVal);
	}

	if (!fgIsExp) {
		/* enable LP recored */
		HW_IO_READ32(pAd->hdev_ctrl, 0x89050200, &macVal);
		macVal |= 0x1;
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89050200, macVal);
	}
}

#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_PTR_ADDR             0x02205250
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_NUM_ADDR             0x02205254
#define WF_MCU_WM_SW_DEF_CR_MSG_TRACE_IDX_ADDR             0x02205254
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_PTR_ADDR           0x02206C80
#define WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_NUM_ADDR           0x02206C7C

#define MSG_HISTORY_NUM 64
#define MAX_TASK_NUM 2

#define WM_SW_DEF_PROGRAM_TRACE_BASE_ADDR                    0x02205210
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR               0x02205214
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_NUM_ADDR               0x02205218
#define WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR               0x02205210

static VOID MemSectionRead(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 length, UINT32 addr)
{
	UINT32 idx = 0;
	void *ptr = buf;

	while (idx < length) {
		HW_IO_READ32(pAd->hdev_ctrl, (addr + idx), ptr);
		idx += 4;
		ptr += 4;
	}
}

static VOID ShowMsgTrace(RTMP_ADAPTER *pAd)
{
	cos_msg_trace_t *msg_trace = NULL;
	UINT32 ptr_addr = 0;
	UINT32 length = 0;
	UINT32 idx = 0;
	UINT32 cnt = 0;
	UINT32 msg_history_num = 0;

	os_alloc_mem(pAd, (UCHAR **)&msg_trace, MSG_HISTORY_NUM * sizeof(cos_msg_trace_t));
	if (!msg_trace) {
		MTWF_PRINT("can not allocate cmd msg_trace\n");
		return;
	}
	os_zero_mem(msg_trace, MSG_HISTORY_NUM * sizeof(cos_msg_trace_t));

	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_TRACE_PTR_ADDR, &ptr_addr);
	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_TRACE_NUM_ADDR, &msg_history_num);

	idx = (msg_history_num >> 8) & 0xff;
	msg_history_num = msg_history_num & 0xff;

	if (idx >= msg_history_num) {
		os_free_mem(msg_trace);
		return;
	}

	length = msg_history_num * sizeof(cos_msg_trace_t);
	MemSectionRead(pAd, (UCHAR *)&(msg_trace[0]), length, ptr_addr);

	MTWF_PRINT("\n");
	MTWF_PRINT("       msg trace:\n");
	MTWF_PRINT("       format: t_id=task_id/task_prempt_cnt/msg_read_idx\n");

	while (1) {

		MTWF_PRINT("       (m_%d)t_id=%x/%d/%d, m_id=%d, ts_en=%u, ts_de = %u, ts_fin=%u, wait=%d, exe=%d\n",
			idx,
			msg_trace[idx].dest_id,
			msg_trace[idx].pcount,
			msg_trace[idx].qread,
			msg_trace[idx].msg_id,
			msg_trace[idx].ts_enq,
			msg_trace[idx].ts_deq,
			msg_trace[idx].ts_finshq,
			(msg_trace[idx].ts_deq - msg_trace[idx].ts_enq),
			(msg_trace[idx].ts_finshq - msg_trace[idx].ts_deq));

		if (++idx >= msg_history_num)
			idx = 0;

		if (++cnt >= msg_history_num)
			break;
	}
	if (msg_trace)
		os_free_mem(msg_trace);
}

static VOID ShowSchduleTrace(RTMP_ADAPTER *pAd)
{
	task_info_struct  task_info_g[MAX_TASK_NUM];
	UINT32 length = 0;
	UINT32 idx = 0;
	UINT32 km_total_time = 0;
	UINT32 addr = 0;
	cos_task_type tcb;
	cos_task_type *tcb_ptr;
	CHAR   name[2][15] = {
		"WIFI   ", "WIFI2   "
	};

	length = MAX_TASK_NUM * sizeof(task_info_struct);
	MemSectionRead(pAd, (UCHAR *)&(task_info_g[0]), length, 0x02202ACC);

	HW_IO_READ32(pAd->hdev_ctrl, 0x0220527C, &km_total_time);

	if (!km_total_time) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" get km_total_time zero!\n");
		return;
	}

	MTWF_PRINT("\n\n\n       TASK    XTIME    RATIO    PREMPT CNT\n");

	for (idx = 0 ;  idx < MAX_TASK_NUM ; idx++) {
		addr = task_info_g[idx].task_id;

		MemSectionRead(pAd, (UCHAR *)&(tcb), sizeof(cos_task_type), addr);
		tcb_ptr = &(tcb);

		if (tcb_ptr) {
			MTWF_PRINT("       %s    %d    %d       %d\n",
				name[idx],
				tcb_ptr->tc_exe_time,
				(tcb_ptr->tc_exe_time*100/km_total_time),
				tcb_ptr->tc_pcount);
		}
	}

}

#ifndef CONFIG_CPE_SUPPORT
#define MAX_MSG_INFO_RANGE_NUM 1
static VOID ShowMsgWatch(RTMP_ADAPTER *pAd)
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
	UINT32 task_range_id[MAX_MSG_INFO_RANGE_NUM] = {1};
	UINT32 task_range_base[MAX_MSG_INFO_RANGE_NUM] = {1};

	HW_IO_READ32(pAd->hdev_ctrl, 0x0220527C, &km_total_time);

	if (!km_total_time) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" get km_total_time zero!\n");
		return;
	}

	ms = (((km_total_time*30)+(km_total_time*52/100))/1000);
	MTWF_PRINT("\n\n\n       Monitor Duration: %d unit = %d ms (unit 30.52us)\n",
					km_total_time, ms);
	MTWF_PRINT("       MSG_ID     XTIME    RATIO    MAX_XTM      CNT\n");

	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_NUM_ADDR, &(cos_msg_num[0]));
	cos_msg_num[0] = (cos_msg_num[0] >> 8) & 0xff;

	HW_IO_READ32(pAd->hdev_ctrl, WF_MCU_WM_SW_DEF_CR_MSG_INFO_R0_PTR_ADDR, &(msg_info_addr[0]));

	for (r_idx = 0; r_idx < MAX_MSG_INFO_RANGE_NUM; r_idx++) {

		length[r_idx] = cos_msg_num[r_idx] * sizeof(cos_msg_type);
		os_alloc_mem(NULL, (PUCHAR *)&(msg), length[r_idx]);
		MemSectionRead(pAd, (UCHAR *)(msg), length[r_idx], (msg_info_addr[r_idx]));

		MTWF_PRINT("       MSG WATCH TASK: %d\n", task_range_id[r_idx]);

		for (idx = 0 ;  idx < cos_msg_num[r_idx] ; idx++) {
			ptr = (cos_internal_msgid)(msg + (idx * sizeof(cos_msg_type)));
			MTWF_PRINT("       %d       %d        %d          %d       %d\n",
				(idx + task_range_base[r_idx]),
				ptr->exe_time,
				ptr->exe_time*100/km_total_time,
				ptr->exe_peak,
				ptr->finish_cnt);
		}

		os_free_mem(msg);
	}

}
#endif

#define PROGRAM_TRACE_HISTORY_NUM 32
static VOID ShowProgTrace(RTMP_ADAPTER *pAd)
{
	cos_program_trace_t *cos_program_trace_ptr = NULL;
	UINT32 trace_ptr = 0;
	UINT32 idx = 0;
	UINT32 old_idx = 0;
	UINT32 old_idx_addr = 0;
	UINT32 prev_idx = 0;
	UINT32 prev_time = 0;
	UINT32 curr_time = 0;
	UINT32 diff = 0;

	os_alloc_mem(pAd, (UCHAR **)&cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));
	if (!cos_program_trace_ptr) {
		MTWF_PRINT("can not allocate cos_program_trace_ptr memory\n");
		return;
	}
	os_zero_mem(cos_program_trace_ptr, PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t));

	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_PTR_ADDR, &trace_ptr);
	HW_IO_READ32(pAd->hdev_ctrl, WM_SW_DEF_PROGRAM_TRACE_TRACE_IDX_ADDR, &old_idx_addr);

	old_idx = (old_idx_addr >> 8) & 0xff;

	MemSectionRead(pAd, (UCHAR *)&cos_program_trace_ptr[0], PROGRAM_TRACE_HISTORY_NUM * sizeof(cos_program_trace_t), trace_ptr);

	MTWF_PRINT("\n");
	MTWF_PRINT("       program trace:\n");

	for (idx = 0 ; idx < PROGRAM_TRACE_HISTORY_NUM ; idx++) {

		prev_idx = ((old_idx + 32 - 1) % 32);

		MTWF_PRINT("       (p_%d)t_id=%x/%d, m_id=%d, LP=0x%x, name=%s, ts2=%d, ",
			old_idx,
			cos_program_trace_ptr[old_idx].dest_id,
			cos_program_trace_ptr[old_idx].msg_sn,
			cos_program_trace_ptr[old_idx].msg_id,
			cos_program_trace_ptr[old_idx].LP,
			cos_program_trace_ptr[old_idx].name,
			cos_program_trace_ptr[old_idx].ts_gpt2);

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
			MTWF_PRINT("diff2=NA, \n");
		else
			MTWF_PRINT("diff2=%8d\n", diff);

		old_idx++;

		if (old_idx >= 32)
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

	os_alloc_mem(NULL, (UCHAR **)&msg, 256);

	if (!msg)
		return 0;

	NdisZeroMemory(msg, 256);

	addr = 0x00400000;

	ptr = msg;
	for (idx = 0 ; idx < 32; idx++) {
		macVal = 0;
		HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);
		NdisCopyMemory(ptr, &macVal, 4);
		addr += 4;
		ptr += 4;
	}
	*ptr = 0;

	MTWF_DBG_NP(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n\n");
	MTWF_DBG_NP(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	"       Assert line\n");
	MTWF_DBG_NP(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	"       %s\n", msg);

	os_free_mem(msg);
	return 0;
}

#define CORE_DUMP_INFO_BASE (0x00411480)

static INT32 chip_show_fw_debg_info(struct _RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0;
	UCHAR exp_assert_proc_entry_cnt = 0;
	UCHAR exp_assert_state = 0;
	UINT32 g_exp_type = 0;
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
	UINT32 g1_exp_counter_addr = 0;
	UINT32 g_exp_type_addr = 0;
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
	INT ret;

	g_exp_type_addr = 0x022051A4;
	exp_assert_state_addr = 0x02204C14;
	g1_exp_counter_addr = 0x022050BC;
	cos_interrupt_count_addr = 0x022001AC;
	processing_irqx_addr = 0x02204F84;
	processing_lisr_addr = 0x022050D0;
	Current_Task_Id_addr = 0x0220406C;
	Current_Task_Indx_addr = 0x0220500C;
	last_dequeued_msg_id_addr = 0x02204FE8;
	km_irq_info_idx_addr = 0x02205264;
	km_eint_info_idx_addr = 0x0220525C;
	km_sched_info_idx_addr = 0x0220526C;
	g_sched_history_num_addr = 0x0220516C;
	km_sched_trace_ptr_addr = 0x02205268;
	km_irq_trace_ptr_addr = 0x02205260;
	km_total_time_addr = 0x0220517C;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, exp_assert_state_addr, &macVal);
	exp_assert_state = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, g1_exp_counter_addr, &macVal);
	exp_assert_proc_entry_cnt = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, g_exp_type_addr, &macVal);
	g_exp_type = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, cos_interrupt_count_addr, &macVal);
	COS_Interrupt_Count = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, processing_irqx_addr, &macVal);
	processing_irqx = (macVal & 0xffff);

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
	HW_IO_READ32(pAd->hdev_ctrl, km_eint_info_idx_addr, &macVal);
	km_eint_info_idx = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, g_sched_history_num_addr, &macVal);
	g_sched_history_num = (macVal & 0xff);
	km_sched_info_idx = ((macVal >> 8) & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_sched_trace_ptr_addr, &macVal);
	km_sched_trace_ptr = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_irq_info_idx_addr, &macVal);
	km_irq_info_idx = ((macVal >> 16) & 0xff);
	g_irq_history_num = (macVal & 0xff);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_irq_trace_ptr_addr, &macVal);
	km_irq_trace_ptr = macVal;

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, km_total_time_addr, &macVal);
	km_total_time = macVal;

	HW_IO_READ32(pAd->hdev_ctrl, 0x022028C8, &(TaskStart[0]));
	HW_IO_READ32(pAd->hdev_ctrl, 0x022028C4, &(TaskEnd[0]));
	HW_IO_READ32(pAd->hdev_ctrl, 0x02202A38, &(TaskStart[1]));
	HW_IO_READ32(pAd->hdev_ctrl, 0x02202934, &(TaskEnd[1]));

	MTWF_PRINT("================FW DBG INFO===================\n");
	MTWF_PRINT("       exp_assert_proc_entry_cnt = 0x%x\n",
		exp_assert_proc_entry_cnt);
	MTWF_PRINT("       exp_assert_state = 0x%x\n",
		exp_assert_state);

	if (exp_assert_proc_entry_cnt == 0) {
		ret = snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Normal");
		if (os_snprintf_error(sizeof(exp_type), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " exp_type Snprintf failed!\n");
			return 0;
		}
	} else if (exp_assert_proc_entry_cnt == 1 &&
		exp_assert_state > 1 && g_exp_type == 5) {
		ret = snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Assert");
		if (os_snprintf_error(sizeof(exp_type), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " exp_type Snprintf failed!\n");
			return 0;
		}
		fgIsExp = TRUE;
		fgIsAssert = TRUE;
	} else if (exp_assert_proc_entry_cnt == 1 && exp_assert_state > 1) {
		ret = snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Exception");
		if (os_snprintf_error(sizeof(exp_type), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " exp_type Snprintf failed!\n");
			return 0;
		}
		fgIsExp = TRUE;
	} else if (exp_assert_proc_entry_cnt > 1) {
		ret = snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Exception re-entry");
		if (os_snprintf_error(sizeof(exp_type), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " exp_type Snprintf failed!\n");
			return 0;
		}
		fgIsExp = TRUE;
	} else {
		ret = snprintf(exp_type, sizeof(exp_type), "%s", "exp_type : Unknown'?");
		if (os_snprintf_error(sizeof(exp_type), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " exp_type Snprintf failed!\n");
			return 0;
		}
	}


	MTWF_PRINT("       COS_Interrupt_Count = 0x%x\n",
		COS_Interrupt_Count);
	MTWF_PRINT("       processing_irqx = 0x%x\n",
		processing_irqx);
	MTWF_PRINT("       processing_lisr = 0x%x\n",
		processing_lisr);
	MTWF_PRINT("       Current_Task_Id = 0x%x\n",
		Current_Task_Id);
	MTWF_PRINT("       Current_Task_Indx = 0x%x\n",
		Current_Task_Indx);
	MTWF_PRINT("       last_dequeued_msg_id = %d\n",
		last_dequeued_msg_id);

	MTWF_PRINT("       km_irq_info_idx = 0x%x\n",
		km_irq_info_idx);
	MTWF_PRINT("       km_eint_info_idx = 0x%x\n",
		km_eint_info_idx);
	MTWF_PRINT("       km_sched_info_idx = 0x%x\n",
		km_sched_info_idx);
	MTWF_PRINT("       g_sched_history_num = %d\n",
		g_sched_history_num);
	MTWF_PRINT("       km_sched_trace_ptr = 0x%x\n",
		km_sched_trace_ptr);

	if (fgIsExp) {
		MTWF_PRINT("\n        <1>print sched trace\n");


		if (g_sched_history_num > 60)
			g_sched_history_num = 60;

		idx = km_sched_info_idx;

		for (i = 0 ; i < g_sched_history_num ; i++) {
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)), &(t1));
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)+4), &(t2));
			HW_IO_READ32(pAd->hdev_ctrl, (km_sched_trace_ptr+(idx*12)+8), &(t3));
			MTWF_PRINT("       (sched_info_%d)sched_t=0x%x, sched_start=%d, PC=0x%x\n",
				idx, t1, t2, t3);

			idx++;

			if (idx >= g_sched_history_num)
				idx = 0;
		}

		MTWF_PRINT("\n        <2>print irq trace\n");


		if (g_irq_history_num > 60)
			g_irq_history_num = 60;

		idx = km_irq_info_idx;

		for (i = 0 ; i < g_irq_history_num ; i++) {
			HW_IO_READ32(pAd->hdev_ctrl, (km_irq_trace_ptr+(idx*16)), &(t1));
			HW_IO_READ32(pAd->hdev_ctrl, (km_irq_trace_ptr+(idx*16)+4), &(t2));

			MTWF_PRINT("       (irq_info_%d)irq_t=%x, sched_start=%d\n",
				idx, t1, t2);

			idx++;

			if (idx >= g_irq_history_num)
				idx = 0;
		}
	}

	MTWF_PRINT("\n       <3>task q_id.read q_id.write\n");
	MTWF_PRINT("       (WIFI )1 0x%x 0x%x\n",
		TaskStart[0], TaskEnd[0]);
	MTWF_PRINT("       (WIFI2 )2 0x%x 0x%x\n",
		TaskStart[1], TaskEnd[1]);


	MTWF_PRINT("\n       <4>TASK STACK INFO (size in byte)\n");

	MTWF_PRINT("       TASK  START       END       SIZE  PEAK  INTEGRITY\n");

	for (i = 0 ; i < 2 ; i++) {
		HW_IO_READ32(pAd->hdev_ctrl, 0x0220286C+(i*368), &t1);
		HW_IO_READ32(pAd->hdev_ctrl, 0x02202870+(i*368), &t2);
		HW_IO_READ32(pAd->hdev_ctrl, 0x02202878+(i*368), &t3);

		if (i == 0) {
			ret = snprintf(str, sizeof(str), "%s", "WIFI");
			if (os_snprintf_error(sizeof(str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " str Snprintf failed!\n");
				return 0;
			}
		} else if (i == 1) {
			ret = snprintf(str, sizeof(str), "%s", "WIFI2");
			if (os_snprintf_error(sizeof(str), ret)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " str Snprintf failed!\n");
				return 0;
			}
		}

		MTWF_PRINT("       %s  0x%x  0x%x  %d\n",
			str, t1, t2, t3);
	}

	MTWF_PRINT("\n       <5>fw state\n");
	MTWF_PRINT("       %s\n", exp_type);

	if (COS_Interrupt_Count > 0)
		MTWF_PRINT("       FW in Interrupt CIRQ index (0x%x) CIRQ handler(0x%x)\n"
			, processing_irqx, processing_lisr);
	else {
		if (Current_Task_Id == 0 && Current_Task_Indx == 3)
			MTWF_PRINT("       FW in IDLE\n");

		if (Current_Task_Id != 0 && Current_Task_Indx != 3)
			MTWF_PRINT("       FW in Task , Task id(0x%x) Task index(0x%x)\n",
				Current_Task_Id, Current_Task_Indx);
	}

	macVal = 0;

	HW_IO_READ32(pAd->hdev_ctrl, g1_exp_counter_addr, &macVal);
	MTWF_PRINT("       EXCP_CNT = 0x%x\n", macVal);

	MTWF_PRINT("       EXCP_TYPE = 0x%x\n", g_exp_type);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x022051A8, &macVal);
	MTWF_PRINT("       CPU_ITYPE = 0x%x\n", macVal);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x022051B0, &macVal);
	MTWF_PRINT("       CPU_EVA = 0x%x\n", macVal);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x022051AC, &macVal);
	MTWF_PRINT("       CPU_IPC = 0x%x\n", macVal);

	macVal = 0;
	HW_IO_READ32(pAd->hdev_ctrl, 0x7C060204, &macVal);
	MTWF_PRINT("       PC = 0x%x\n\n\n", macVal);

	ShowLpHistory(pAd, fgIsExp);
	ShowIrqHistory(pAd);

	ShowCpuUtilSum(pAd);
	ShowMsgTrace(pAd);
	//ShowMsgWatch(pAd);
	ShowSchduleTrace(pAd);
	ShowProgTrace(pAd);

	if (fgIsAssert)
		ShowAssertLine(pAd);

	MTWF_PRINT("============================================\n");

	return 0;

}

static INT32 chip_show_bus_debg_info(struct _RTMP_ADAPTER *pAd)
{
	UINT32 macVal = 0, i = 0;
	BOOL  AP2CONN_INFRA_ON_OK = TRUE, AP2CONN_INFRA_OFF_OK = TRUE, DUMP_WFSYS = TRUE;

	if (AP2CONN_INFRA_ON_OK == TRUE) {
		MTWF_PRINT("================[AP2CONN_INFRA_OFF readable check]==================\n");
		/* Check conn_infra off bus clock */
		macVal = 0;
		RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_BUS_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_ADDR, &macVal);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_BUS_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_ADDR,
			(macVal | CONN_HOST_CSR_TOP_BUS_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_MASK));
		macVal = 0;
		RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_BUS_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_ADDR, &macVal);
		if (((macVal & CONN_HOST_CSR_TOP_BUS_MCU_STAT_HCLK_FR_CK_DETECT_MASK) >>
			CONN_HOST_CSR_TOP_BUS_MCU_STAT_HCLK_FR_CK_DETECT_SHFT) == 1 &&
			((macVal & CONN_HOST_CSR_TOP_BUS_MCU_STAT_OSC_CLK_DETECT_MASK) >>
			CONN_HOST_CSR_TOP_BUS_MCU_STAT_OSC_CLK_DETECT_SHFT) == 1) {
			MTWF_PRINT("Check conn_infra off bus clock is OK\n");
		} else {
			MTWF_PRINT("Check conn_infra off bus clock is NOT OK, ");
			MTWF_PRINT("conn_infra off bus clock=0x%x, osc clock=0x%x\n",
				((macVal & CONN_HOST_CSR_TOP_BUS_MCU_STAT_HCLK_FR_CK_DETECT_MASK) >>
				CONN_HOST_CSR_TOP_BUS_MCU_STAT_HCLK_FR_CK_DETECT_SHFT),
				((macVal & CONN_HOST_CSR_TOP_BUS_MCU_STAT_OSC_CLK_DETECT_MASK) >>
				CONN_HOST_CSR_TOP_BUS_MCU_STAT_OSC_CLK_DETECT_SHFT));
			AP2CONN_INFRA_OFF_OK = FALSE;
		}
		/* Check conn_infra IP version */
		macVal = 0;
		RTMP_IO_READ32(pAd->hdev_ctrl, CONN_CFG_IP_VERSION_ADDR, &macVal);
		if (macVal == 0x02070000) {
			MTWF_PRINT("Check conn_infra IP version is OK\n");
		} else {
			MTWF_PRINT("Check conn_infra IP version is NOT OK, CONN_CFG_IP_VERSION_VAL = 0x%08x\n", macVal);
			AP2CONN_INFRA_OFF_OK = FALSE;
		}

		/* Check conn_infra off domain bus hang irq status */
		macVal = 0;
		RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_DBG_DUMMY_5_CONN_INFRA_BUS_TIMEOUT_IRQ_B_ADDR,
			&macVal);
		if (((macVal & CONN_HOST_CSR_TOP_DBG_DUMMY_5_CONN_INFRA_BUS_TIMEOUT_IRQ_B_MASK) >>
			CONN_HOST_CSR_TOP_DBG_DUMMY_5_CONN_INFRA_BUS_TIMEOUT_IRQ_B_SHFT) == 1) {
			MTWF_PRINT("Check conn_infra off domain bus hang irq status is OK\n");
		} else {
			MTWF_PRINT("Check conn_infra off domain bus hang irq status is NOT OK, ");
			MTWF_PRINT("conn_infra_bus_timeout_irq_b = 0x%x\n",
				(macVal & CONN_HOST_CSR_TOP_DBG_DUMMY_5_CONN_INFRA_BUS_TIMEOUT_IRQ_B_MASK)
				>> CONN_HOST_CSR_TOP_DBG_DUMMY_5_CONN_INFRA_BUS_TIMEOUT_IRQ_B_SHFT);
			DUMP_WFSYS = FALSE;
		}

		if (AP2CONN_INFRA_OFF_OK == FALSE) {
			MTWF_PRINT("================[conn_infra_top_ctrl_check]==================\n");
			macVal = 0;
			RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_CONN_INFRA_SYSSTRAP_DBG_ADDR, &macVal);
			MTWF_PRINT("Dump2Excel[conn_infra_systrap_parse]: CONN_HOST_CSR_TOP_CONN_INFRA_SYSSTRAP_DBG_ADDR = 0x%08x\n",
				macVal);
			for (i = 0; i < 5; i++) {
				RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_CONN_INFRA_CFG_DBG_SEL_ADDR, i);
				macVal = 0;
				RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_DBG_DUMMY_2_ADDR,
					&macVal);
				MTWF_PRINT("Dump2Excel[conn_infra_cfg_clk_parse]: No.%d VALUE = 0x%08x\n", i+1, macVal);
			}
			MTWF_PRINT("================[conn_infra_bus_debug]==================\n");
			/* ahb_apb_timeout_dump */
			for (i = 0; i < sizeof(conn_infra_ahb_apb_timeout_dump) / sizeof(struct BUS_DEBUG_INFO); i++) {
				macVal = 0;
				RTMP_IO_READ32(pAd->hdev_ctrl, conn_infra_ahb_apb_timeout_dump[i].reg_address, &macVal);
				MTWF_PRINT("Dump2Excel[ahb_apb_timeout_dump]: No.%d. %s = 0x%08x\n",
					i+1, conn_infra_ahb_apb_timeout_dump[i].reg_name, macVal);
			}
			{
				/* debug_ctrl_setting */
				/* Debug log, No CODA */
				static UINT32 conn_infra_debug_log_reg[] = {
				0x1800F408, 0x1800F40C, 0x1800F410, 0x1800F414, 0x1800F418,
				0x1800F41C, 0x1800F420, 0x1800F424, 0x1800F428, 0x1800F42C,
				0x1800F430
				};
				for (i = 0; i < sizeof(conn_infra_debug_log_reg) / sizeof(UINT32); i++) {
					macVal = 0;
					RTMP_IO_READ32(pAd->hdev_ctrl, conn_infra_debug_log_reg[i], &macVal);
					MTWF_PRINT("Dump2Excel[debug_ctrl_setting][Debug log]: NO.%d. 0x%08x VAL = 0x%08x\n",
						i+1, conn_infra_debug_log_reg[i], macVal);
				}
			}

			/* Real time log */
			/* enable setting, No CODA */
			macVal = 0;
			RTMP_IO_READ32(pAd->hdev_ctrl, 0x1800F000, &macVal);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x1800F000, (macVal | 0x00000010));

			{
				static UINT32 conn_infra_real_time_log_set_val[] = {
				0x00010001, 0x00020001, 0x00010002, 0x00020002, 0x00030002,
				0x00010003, 0x00020003, 0x00030003, 0x00010004, 0x00020004,
				0x00010005
				};
				for (i = 0; i < sizeof(conn_infra_real_time_log_set_val) / sizeof(UINT32); i++) {
					RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_CONN_INFRA_ON_DEBUG_AO_DEBUGSYS_ADDR,
						conn_infra_real_time_log_set_val[i]);
					macVal = 0;
					RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_CONN_INFRA_ON_DEBUG_CTRL_AO2SYS_OUT_ADDR,
						&macVal);
					MTWF_PRINT("Dump2Excel[debug_ctrl_setting][Real time log]: No.%d. VAL = 0x%08x\n",
						i+1, macVal);
				}
			}
		}

		if (DUMP_WFSYS == TRUE) {
			MTWF_PRINT("================[WFSYS BUS Top ctrl]==================\n");
			{
				static UINT32 wfsys_sleep_wakeup_debug_val[] = {
				0x00100000, 0x00108421, 0x00184210, 0x001BDEF7, 0x001EF7BD,
				};
				for (i = 0; i < sizeof(wfsys_sleep_wakeup_debug_val) / sizeof(UINT32); i++) {
					RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x000E0094, wfsys_sleep_wakeup_debug_val[i]);
					macVal = 0;
					RTMP_IO_READ32(pAd->hdev_ctrl, 0x000E021C, &macVal);
					MTWF_PRINT("Dump2Excel[wfsys_sleep_wakeup_debug_PCIE]: No.%d VALUE = 0x%08x\n", i+1, macVal);
				}
			}

			MTWF_PRINT("================[WFSYS BUS debug method]==================\n");
			{
				/* CR table */
				static UINT32 enable_debug_set_val[] = {
				0x00010001, 0x00020001, 0x00030001, 0x00040001, 0x00050001,
				0x00060001, 0x00070001, 0x00080001, 0x00090001, 0x000A0001,
				0x000B0001, 0x000C0001, 0x000D0001, 0x000E0001, 0x000F0001,
				0x00100001, 0x00010002, 0x00010003
				};
				for (i = 0; i < sizeof(enable_debug_set_val) / sizeof(UINT32); i++) {
					RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_MCUSY_VDNR_BUS_DBG_SEL_ADDR,
						enable_debug_set_val[i]);
					macVal = 0;
					RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_MCUSY_VDNR_BUS_DBG_OUT_ADDR,
						&macVal);
					MTWF_PRINT("Dump2Excel[CR_table]: #%d VALUE = 0x%08x\n", i+1, macVal);
				}
			}
			/* bus_ahb_apb_timeout */
			for (i = 0; i < sizeof(wfsys_bus_ahp_apb_timeout) / sizeof(struct BUS_DEBUG_INFO); i++) {
				macVal = 0;
				RTMP_IO_READ32(pAd->hdev_ctrl, wfsys_bus_ahp_apb_timeout[i].reg_address, &macVal);
				MTWF_PRINT("Dump2Excel[bus ahb apb timeout]: #%d. %s = 0x%08x\n",
					i+1, wfsys_bus_ahp_apb_timeout[i].reg_name, macVal);
			}
		}
	}
	return 0;
}

static struct {
	PCHAR Name;
	UINT32 StartAddr;
	UINT32 DumpSize;
} COREDUMP_QUEUE_INFO[] = {
    {"_ROM.bin",        0x00800000,     0x00060000},
    {"ULM1",            0x00900000,     0x00014000},
    {"ULM2",            0x02200000,     0x00050000},
    {"ULM3",            0x02300000,     0x00050000},
    {"SRAM",            0x00400000,     0x00028000},
    {"CRAM",            0xE0000000,     0x00158000},
    {NULL,}
};

static INT32 chip_show_coredump_proc(struct _RTMP_ADAPTER *pAd)
{
	RTMP_STRING *msg;
	UCHAR fileName[64];
	struct file *file_w;
	mm_segment_t orig_fs;
	UINT32 addr = 0;
	UINT32 end_addr = 0;
	UINT32 macVal = 0;
	UINT32 i = 0;
	INT ret;

	os_alloc_mem(NULL, (UCHAR **)&msg, 4);

	if (!msg)
		return TRUE;

	NdisZeroMemory(msg, 4);

	orig_fs = get_fs();
	set_fs(KERNEL_DS);

	while (COREDUMP_QUEUE_INFO[i].Name != NULL) {
		ret = snprintf(fileName, sizeof(fileName), "/etc/%s.bin", COREDUMP_QUEUE_INFO[i].Name);
		if (os_snprintf_error(sizeof(fileName), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " fileName Snprintf failed!\n");
			goto  done;
		}
		/* open file */
		file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

		if (IS_ERR(file_w)) {
			MTWF_PRINT("-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName);
		} else {
			if (file_w->f_op)
				file_w->f_pos = 0;
			else
				goto  done;


			addr = COREDUMP_QUEUE_INFO[i].StartAddr;
			end_addr = addr + COREDUMP_QUEUE_INFO[i].DumpSize - 1;
			MTWF_PRINT("%s open success  addr:0x%x\n", fileName, addr);

			while (addr <= end_addr) {

				HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);
				NdisCopyMemory(msg, &macVal, 4);
				addr += 4;
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
				if (file_w->f_op->write)
					file_w->f_op->write(file_w, msg, 4, &file_w->f_pos);
				else
					MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no file write method\n"));
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
						kernel_write(file_w, msg, 4, &file_w->f_pos);
#else
						__vfs_write(file_w, msg, 4, &file_w->f_pos);
#endif
			}

			filp_close(file_w, NULL);
			MTWF_PRINT("%s write done\n", fileName);
		}
		i ++;
	}

done:
	set_fs(orig_fs);
	os_free_mem(msg);

	return TRUE;

}

static INT chip_set_fw_cp_util_en(RTMP_ADAPTER *pAd, UINT en)
{
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"en(%d)\n", en);

	if (en > 0) {
		//DIC_CPU_UTLZ_INIT
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x7C053A50, 0x1300);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010108, 0x00080000);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010118, 0x00080000);
		mdelay(10);

		//DIC_CPU_UTLZ_EN
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x7C053A50, 0x1301);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010108, 0x00080000);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010118, 0x00080000);
	} else {
		//DIC_CPU_UTLZ_DIS
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010108, 0x00080000);
		HW_IO_WRITE32(pAd->hdev_ctrl, 0x89010118, 0x00080000);
	}

	return 0;
}

VOID mt7916_chip_dbg_init(struct _RTMP_CHIP_DBG *dbg_ops)
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
	dbg_ops->show_bss_bitmap = chip_show_bss_bitmap;
	dbg_ops->show_bss_setting = chip_show_bss_setting;
#endif	/* VOW_SUPPORT */
	dbg_ops->get_lpon_frcr = chip_get_lpon_frcr;

#ifdef MT7916_FPGA
#endif /*MT7916_FPGA*/
#ifdef CONFIG_ATE
	dbg_ops->ctrl_manual_hetb_tx = chip_ctrl_manual_hetb_tx;
	dbg_ops->ctrl_manual_hetb_rx = chip_ctrl_manual_hetb_rx;
	dbg_ops->chip_ctrl_spe = chip_ctrl_asic_spe;
	dbg_ops->get_tx_mibinfo = chip_get_tx_mibinfo;
#endif
	dbg_ops->show_asic_rx_stat = chip_show_asic_rx_stat;
	dbg_ops->show_ple_info_by_idx = chip_show_ple_info_by_idx;
	dbg_ops->show_fw_dbg_info = chip_show_fw_debg_info;
	dbg_ops->show_bus_dbg_info = chip_show_bus_debg_info;
	dbg_ops->show_coredump_proc = chip_show_coredump_proc;
	dbg_ops->set_cpu_util_en = chip_set_fw_cp_util_en;
	dbg_ops->chk_exception_type = chip_chk_exception_type;
}
