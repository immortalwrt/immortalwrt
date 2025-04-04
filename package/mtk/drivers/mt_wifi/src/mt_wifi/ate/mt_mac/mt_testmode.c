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
	mt_testmode.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif







#ifdef MT7986
#include "chip/mt7986.h"
#endif

#ifdef MT7916
#include "chip/mt7916.h"
#endif

#ifdef MT7981
#include "chip/mt7981.h"
#endif

#define MCAST_WCID_TO_REMOVE 0  /* Pat: TODO */

static struct _ATE_DATA_RATE_MAP cck_mode_mcs_to_data_rate_map[] = {
	{0, 1000},
	{1, 2000},
	{2, 5500},
	{3, 11000},
	{9, 2000},
	{10, 5500},
	{11, 11000},
};

static struct _ATE_DATA_RATE_MAP ofdm_mode_mcs_to_data_rate_map[] = {
	{0, 6000},
	{1, 9000},
	{2, 12000},
	{3, 18000},
	{4, 24000},
	{5, 36000},
	{6, 48500},
	{7, 54000},
};

static struct _ATE_DATA_RATE_MAP n_mode_mcs_to_data_rate_map[] = {
	{0, 6500},
	{1, 13000},
	{2, 19500},
	{3, 26000},
	{4, 39000},
	{5, 52000},
	{6, 58500},
	{7, 65000},
	{32, 6000},  /* MCS32 */
};

static struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw20[] = {
	{0, 65},    /* in unit of 100k */
	{1, 130},
	{2, 195},
	{3, 260},
	{4, 390},
	{5, 520},
	{6, 585},
	{7, 650},
	{8, 780},
};

static struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw40[] = {
	{0, 135},   /* in unit of 100k */
	{1, 270},
	{2, 405},
	{3, 540},
	{4, 810},
	{5, 1080},
	{6, 1215},
	{7, 1350},
	{8, 1620},
	{9, 1800},
};


static struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw80[] = {
	{0, 293},   /* in unit of 100k */
	{1, 585},
	{2, 878},
	{3, 1170},
	{4, 1755},
	{5, 2340},
	{6, 2633},
	{7, 2925},
	{8, 3510},
	{9, 3900},
};

static struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw160[] = {
	{0, 585},   /* in unit of 100k */
	{1, 1170},
	{2, 1755},
	{3, 2340},
	{4, 3510},
	{5, 4680},
	{6, 5265},
	{7, 5850},
	{8, 7020},
	{9, 7800},
};

#if defined(DOT11_HE_AX)
/* phy rates comes from NSS = 1, Long GI */
static struct _ATE_DATA_RATE_MAP he_su_mode_mcs_to_data_rate_map_bw20[] = {
	/*index, nss1, nss2, nss3, nss4*/
	{0,		73},    /* in unit of 100k */
	{1,		146},
	{2,		219},
	{3,		293},
	{4,		439},
	{5,		585},
	{6,		658},
	{7,		731},
	{8,		878},
	{9,		975},
	{10,	1097},
	{11,	1219},
	{33,	36},	/* MCS0 DCM */
	{34,	73},	/* MCS1 DCM */
};

static struct _ATE_DATA_RATE_MAP he_su_mode_mcs_to_data_rate_map_bw40[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi}, nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}*/
	{0,		146},    /* in unit of 100k */
	{1,		293},
	{2,		439},
	{3,		585},
	{4,		878},
	{5,		1170},
	{6,		1316},
	{7,		1463},
	{8,		1755},
	{9,		1950},
	{10,	2194},
	{11,	2438},
	{33,	73},	/* MCS0 DCM */
	{34,	146},	/* MCS1 DCM */
};


static struct _ATE_DATA_RATE_MAP he_su_mode_mcs_to_data_rate_map_bw80[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi}, nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}*/
	{0,		306},    /* in unit of 100k */
	{1,		613},
	{2,		919},
	{3,		1225},
	{4,		1838},
	{5,		2450},
	{6,		2756},
	{7,		3063},
	{8,		3675},
	{9,		4083},
	{10,	4594},
	{11,	5104},
	{33,	153},	/* MCS0 DCM */
	{34,	306},	/* MCS1 DCM */
};

static struct _ATE_DATA_RATE_MAP he_su_mode_mcs_to_data_rate_map_bw160[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi}, nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}*/
	{0,		613},    /* in unit of 100k */
	{1,		1225},
	{2,		1838},
	{3,		2450},
	{4,		3675},
	{5,		4900},
	{6,		5513},
	{7,		6125},
	{8,		7350},
	{9,		8166},
	{10,	9188},
	{11,	10208},
	{33,	306},	/* MCS0 DCM */
	{34,	613},	/* MCS1 DCM */
};

static struct _ATE_HE_PHY_RU_CONST ru_const[] = {
	{37, 24, 12, 6, 2},
	{53, 48, 24, 12, 6},
	{61, 102, 51, 24, 12},
	{65, 234, 117, 60, 30},
	{67, 468, 234, 120, 60},
	{68, 980, 490, 240, 120},
	{69, 1960, 980, 492, 246}
};

static UINT8 he_bpscs[] = {
	1, 2, 2, 4, 4, 6, 6, 6, 8, 8, 10, 10	/* MCS0~11 */
};

static UINT8 he_rate_density[] = {
	2, 2, 4, 2, 4, 3, 4, 6, 4, 6, 4, 6		/* MCS0~11 */
};

static UINT8 ltf_sym[] = {
	0, 1, 2, 4, 4, 6, 6, 8, 8	/* SS 1~8 */
};

static UINT8 ltf_sym_code[] = {
	0, 0, 1, 2, 2, 3, 3, 4, 4	/* SS 1~8 */
};

static UINT8 he_t_ltf_sym_x5[] = {
	24,	40, 80								/* 3.2+1.6 us, 6.4+1.6, 12.8+3.2 */
};

static UINT8 he_t_sym_x5[] = {
	68,	72, 80								/* base GI, double GI, quadruple GI */
};

static UINT8 he_t_pe_x5[] = {
	0, 20, 40, 60, 80						/* 0us, 4us, 8us, 12us, 16us */
};

#define ate_min(_a, _b) ((_a > _b) ? _b : _a)

#define ate_max(_a, _b) ((_a > _b) ? _a : _b)

#define ate_ceil(_a, _b) (((_a%_b) > 0) ? ((_a/_b)+1) : (_a/_b))
#endif /* DOT11_HE_AX */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7915) || defined(MT7986) || defined(MT7916) || \
	defined(MT7981)
#define ATE_ANT_USER_SEL 0x80000000
/* BIT[0]:all, BIT[X==1]:ant(X-1) sel */
struct _ATE_ANT_MAP ant_to_spe_idx_map[] = {
	/* All */
	{0x0, 0},
	{0xf, 0},
	/* 1 Ant */
	{0x1, 0},	/* Tx0 */
	{0x2, 1},	/* Tx1 */
	{0x4, 3},	/* Tx2 */
	{0x8, 9},	/* Tx3 */
	/* 2 Ant */
	{0x3, 0},
	{0x5, 2},
	{0x9, 8},
	{0x6, 4},
	{0xa, 6},
	{0xc, 16},
	/* 3 Ant */
	{0x7, 0},	/* 0_1_2 */
	{0xb, 10},	/* 0_1_3 */
	{0xd, 12},	/* 0_2_3 */
	{0xe, 18},	/* 1_2_3 */
};
#else
#endif

/* todo: efuse structure need unify, MT7636 will fail in this flow */
struct _ATE_TXPWR_GROUP_MAP txpwr_group_map[] = {
	{0},
};

#if defined(MT7663) || defined(AXE) || defined(MT7626) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
#define CONNAC_TXTONE_POWER_OFFSET 1
#endif

static UINT32 *_rssi_eeprom_band_offset[] = {NULL};
static UINT32 _n_band_offset[] = {0};

enum HETB_CTRL {
	HETB_TX_CFG = 0,
	HETB_TX_START = 1,
	HETB_TX_STOP = 2
};

#ifdef PRE_CAL_TRX_SET2_SUPPORT
static UINT16 PreCalGroupList[] = {
	0x00ED, /* 0 - Ch group 0,2,3,5,6,7 */
	0x01FF, /*All group 0 ~ 8*/
};
static UINT8 PreCalItemList[] = {
	0x1F, /* 0 - Pre-cal Bit[0]:TXLPF, Bit[1]:TXIQ, Bit[2]:TXDC, Bit[3]:RXFI, Bit[4]:RXFD */
};
#endif /* PRE_CAL_TRX_SET2_SUPPORT */

struct rssi_offset_eeprom eeprom_rssi_offset = {
#ifndef COMPOS_TESTMODE_WIN
	.rssi_eeprom_band_offset = _rssi_eeprom_band_offset,
	.n_band_offset = _n_band_offset,
	.n_band = ARRAY_SIZE(_rssi_eeprom_band_offset),
#else
	_rssi_eeprom_band_offset,
	_n_band_offset,
	sizeof(_rssi_eeprom_band_offset) / sizeof(_rssi_eeprom_band_offset[0]),
#endif
};

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
UINT8 g_EBF_certification;
UINT8 BF_ON_certification;
extern UCHAR TemplateFrame[32];
#endif /* TXBF_SUPPORT && MT_MAC */

#if defined(DOT11_HE_AX)
static INT32 mt_ate_show_ru_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
static INT32 mt_ate_set_ru_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UCHAR *str);
#endif
static INT32 mt_ate_tx_subscribe(RTMP_ADAPTER *ad);
static INT32 mt_ate_tx_unsubscribe(RTMP_ADAPTER *ad);
static BOOLEAN is_mt_ate_mac_tbl_stack_full(struct _RTMP_ADAPTER *ad,
							  UINT8 band_idx)
{
	BOOLEAN ret = FALSE;
	UINT8 stack_boundary = 0;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(ad))
		stack_boundary = MAX_MULTI_TX_STA/TESTMODE_BAND_NUM;
	else
#endif
		stack_boundary = MAX_MULTI_TX_STA;

	if (stack->index == stack_boundary)
		ret = TRUE;

	return ret;
}

static BOOLEAN is_mt_ate_mac_tbl_stack_empty(struct _RTMP_ADAPTER *ad,
								UINT8 band_idx)
{
	BOOLEAN ret = FALSE;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

	if (stack->index == 0)
		ret = TRUE;

	return ret;
}


static INT32 mt_ate_push_mac_tbl_entry(struct _RTMP_ADAPTER *ad,
						  UINT8 band_idx,
						  struct wifi_dev *wdev,
						  UINT8 *da,
						  struct _MAC_TABLE_ENTRY *entry)
{
	INT32 ret = -1;
	UINT8 stack_boundary = 0;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(ad))
		stack_boundary = MAX_MULTI_TX_STA/TESTMODE_BAND_NUM;
	else
#endif
		stack_boundary = MAX_MULTI_TX_STA;

	if (stack->index < stack_boundary) {
		os_move_mem(stack->da[stack->index], da, MAC_ADDR_LEN);
		stack->wdev[stack->index] = wdev;
		stack->mac_tbl_entry[stack->index] = entry;
		ret = stack->index;

		stack->index++;
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Stack for MAC_TABL_ENRTY is full!\n");
	}

	return ret;
}

static NDIS_STATUS mt_ate_pop_mac_tbl_entry(struct _RTMP_ADAPTER *ad,
							UINT8 band_idx,
							UINT8 **da,
							struct _MAC_TABLE_ENTRY **entry)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

	if (stack->index > 0) {
		*entry = stack->mac_tbl_entry[stack->index-1];
		stack->mac_tbl_entry[stack->index-1] = NULL;
		stack->wdev[stack->index-1] = NULL;
		*da = stack->da[stack->index-1];

		stack->index--;
	} else {
		ret = NDIS_STATUS_FAILURE;
		*entry = NULL;
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Stack for MAC_TABL_ENRTY is empty!\n");
	}

	return ret;
}

static INT32 mt_ate_search_mac_tbl_entry(struct _RTMP_ADAPTER *ad,
							 UINT8 band_idx,
							 UINT16 wcid,
							 struct _MAC_TABLE_ENTRY **entry)
{
	INT32 ret = -1, traversal_idx = 0;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

	if (stack->index == 0) {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Stack for MAC_TABL_ENRTY is empty!\n");

		goto end;
	} else {
		for (traversal_idx = 0 ; traversal_idx < stack->index ; traversal_idx++) {
			if (stack->mac_tbl_entry[traversal_idx]) {
				struct _MAC_TABLE_ENTRY *mac_tbl_entry = stack->mac_tbl_entry[traversal_idx];

				if (mac_tbl_entry->wcid == wcid) {
					*entry = mac_tbl_entry;
					ret = traversal_idx;
					break;
				}
			}
		}
	}

end:
	return ret;
}


static void fill_header_address(struct _RTMP_ADAPTER *ad, PUCHAR buf, UINT32 band_idx, UINT16 sta_idx)
{
	PUCHAR addr1, addr2, addr3;

	addr1 = TESTMODE_GET_PARAM(ad, band_idx, addr1[sta_idx]);
	addr2 = TESTMODE_GET_PARAM(ad, band_idx, addr2[sta_idx]);
	addr3 = TESTMODE_GET_PARAM(ad, band_idx, addr3[sta_idx]);

	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tDA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tSA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(addr1), PRINT_MAC(addr2),
			  PRINT_MAC(addr3));

	NdisMoveMemory(buf + 4, addr1, MAC_ADDR_LEN);
	NdisMoveMemory(buf + 10, addr2, MAC_ADDR_LEN);
	NdisMoveMemory(buf + 16, addr3, MAC_ADDR_LEN);
}


VOID MtATEDummyFunc(VOID)
{
}
/* Private Function Prototype */
static INT32 MT_ATEMPSRelease(RTMP_ADAPTER *pAd);
static INT32 MT_ATEMPSInit(RTMP_ADAPTER *pAd);
static INT32 MT_ATEMPSLoadSetting(RTMP_ADAPTER *pAd, UINT32 band_idx);
#ifdef ATE_TXTHREAD
static INT32 MT_ATEMPSRunStatCheck(RTMP_ADAPTER *pAd, UINT32 band_idx);
#endif
#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT fd);
static INT MT_ATEWriteFd(RTMP_STRING *log, RTMP_OS_FD_EXT srcf);
#else
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx);
#endif
static INT32 MT_MPSTxStop(RTMP_ADAPTER *pAd);
static INT32 MT_ATELogOnOff(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 size);
static INT32 MT_ATESetICapStart(RTMP_ADAPTER *pAd, PUINT8 pData);
static INT32 MT_ATEGetICapStatus(RTMP_ADAPTER *pAd);
static INT32 MT_ATEGetICapIQData(RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
#if defined(DOT11_HE_AX)
static NDIS_STATUS mt_ate_calc_symbol_by_bytes(struct _ATE_RU_STA *ru_sta, UCHAR stbc, UCHAR rate_den, UINT32 apep_length);
static struct _ATE_RU_STA *mt_ate_dominate_ru(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
static INT32 mt_ate_calc_afactor(struct _ATE_RU_STA *ru_sta);
static UINT32 mt_ate_calc_l_ldpc(UINT32 avbits, UINT32 pld, UCHAR rate_den, UINT32 *cw, UINT32 *l_ldpc);
static BOOLEAN mt_ate_calc_extr_sym(struct _ATE_RU_STA *ru_sta, UCHAR stbc, UCHAR rate_den);
static UINT32 mt_ate_calc_pe_disamb(struct _ATE_RU_STA *ru_sta, UCHAR ltf_gi, UCHAR pe);
static UINT8 mt_ate_translate_ltf(UINT8 tx_mode, UINT32 ltf_gi);
static UINT8 mt_ate_translate_gi(UINT8 tx_mode, UINT32 ltf_gi);
#endif /* DOT11_HE_AX */
/* static INT32 MT_ATEInsertLog(RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len); */

/* #if CFG_eBF_Sportan_Certification */
INT32 MT_ATEComposePkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx, UINT16 sta_idx);
/* #else */
/* static INT32 MT_ATEComposePkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx); */
/* #endif */

#if defined(DOT11_HE_AX)
INT MtATESetRxMUAid(RTMP_ADAPTER *ad, UCHAR band_idx, UINT16 mu_rx_aid)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = ENUM_ATE_SET_MU_RX_AID;
	param.Data.set_mu_rx_aid.band_idx = band_idx;
	param.Data.set_mu_rx_aid.aid = cpu2le16(mu_rx_aid);

	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Set to decode MU accodring to AID:%d (61696 means disable)\n",
		param.Data.set_mu_rx_aid.aid);
	ret = MtCmdATETest(ad, &param);
#endif
	return ret;
}

static INT MtCmdSetPhyManualTx(RTMP_ADAPTER *ad, BOOLEAN on, UINT32 apep_length, struct phy_params *phy_info, struct _ATE_RU_STA *ru_sta)
{
	INT ret = 0;
	struct _ATE_CTRL *ate_ctrl = &ad->ATECtrl;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;
	UINT32 stbc = TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), stbc);

	os_zero_mem(&param, sizeof(param));

	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_PHY_MANUAL_TX;

	if (on) {
		param.Data.set_phy_manual_tx.band_idx = ate_ctrl->control_band_idx;
		param.Data.set_phy_manual_tx.cg1_ddw1.user_cnt = 1;			/* HE_TB always 1 */
		param.Data.set_phy_manual_tx.cg1_ddw1.tx_mode = phy_info->phy_mode;
		param.Data.set_phy_manual_tx.cg1_ddw1.bw = phy_info->bw;
		param.Data.set_phy_manual_tx.cg1_ddw1.stbc = stbc;
		param.Data.set_phy_manual_tx.cg1_ddw1.ru_mu = 0;
		param.Data.set_phy_manual_tx.cg1_ddw1.spatial_ext = phy_info->ant_pri;
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[band:%d][TXV Common1 DDW1] total_pwr_dbm:0x%x pwr_dbm:%d user_cnt:%d, tx_mode:0x%x, bw:%d, stbc:%d, ru_mu:%d, spatial_ext:%d\n",
				ate_ctrl->control_band_idx, param.Data.set_phy_manual_tx.cg1_ddw1.total_pwr_dbm,
				param.Data.set_phy_manual_tx.cg1_ddw1.pwr_dbm, param.Data.set_phy_manual_tx.cg1_ddw1.user_cnt,
				param.Data.set_phy_manual_tx.cg1_ddw1.tx_mode, param.Data.set_phy_manual_tx.cg1_ddw1.bw,
				param.Data.set_phy_manual_tx.cg1_ddw1.stbc, param.Data.set_phy_manual_tx.cg1_ddw1.ru_mu,
				param.Data.set_phy_manual_tx.cg1_ddw1.spatial_ext);

		param.Data.set_phy_manual_tx.cg1_ddw2.trigger_frame_ind = 0;	/* HE_TB always 0 */
		param.Data.set_phy_manual_tx.cg1_ddw2.format = 0;				/* HE_TB always 0 */
		param.Data.set_phy_manual_tx.cg1_ddw2.ltf = phy_info->ltf_type;
		param.Data.set_phy_manual_tx.cg1_ddw2.gi = phy_info->gi_type;
		param.Data.set_phy_manual_tx.cg1_ddw2.sig_a_rsvd = 0;			/* testmode ignored */
		param.Data.set_phy_manual_tx.cg1_ddw2.tf_rsp_ind = 0;
		param.Data.set_phy_manual_tx.cg1_ddw2.cfo_ind = 0;
		param.Data.set_phy_manual_tx.cg1_ddw2.precomp_cfo_idx = 0;
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[band:%d][TXV Common1 DDW2] trigger_frame_ind:%d format:%d ltf:%d, gi:%d, sig_a_rsvd:0x%x, total_pwr_ind:%d, tf_rsp_ind:%d, cfo_ind:%d, precomp_cfo_idx:%d\n",
				ate_ctrl->control_band_idx, param.Data.set_phy_manual_tx.cg1_ddw2.trigger_frame_ind,
				param.Data.set_phy_manual_tx.cg1_ddw2.format, param.Data.set_phy_manual_tx.cg1_ddw2.ltf,
				param.Data.set_phy_manual_tx.cg1_ddw2.gi, param.Data.set_phy_manual_tx.cg1_ddw2.sig_a_rsvd,
				param.Data.set_phy_manual_tx.cg1_ddw2.total_pwr_ind, param.Data.set_phy_manual_tx.cg1_ddw2.tf_rsp_ind,
				param.Data.set_phy_manual_tx.cg1_ddw2.cfo_ind, param.Data.set_phy_manual_tx.cg1_ddw2.precomp_cfo_idx);

		param.Data.set_phy_manual_tx.cg2_ddw1.mimo_ltf = 0;	/* SSP:0 ; MES:1 */
		param.Data.set_phy_manual_tx.cg2_ddw1.afactor = ru_sta->afactor_init;
		param.Data.set_phy_manual_tx.cg2_ddw1.txop = 0;
		param.Data.set_phy_manual_tx.cg2_ddw1.pe_disamb = ru_sta->pe_disamb;
		param.Data.set_phy_manual_tx.cg2_ddw1.ltf_symbol = ltf_sym_code[ru_sta->nss];
		param.Data.set_phy_manual_tx.cg2_ddw1.ppdu_sym_cnt = ru_sta->symbol_init;
		param.Data.set_phy_manual_tx.cg2_ddw1.ldpc_extra_symbol = ru_sta->ldpc_extr_sym;
		param.Data.set_phy_manual_tx.cg2_ddw1.lg_txlen = ate_ceil((ru_sta->tx_time_x5-20*5), (4*5))*3-3-2;
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[band:%d][TXV Common2 DDW1] mimo_ltf:%d afactor:%d txop:%d, pe_disamb:%d, ltf_symbol:%d ppdu_sym_cnt:0x%x, ldpc_extra_symbol:%d, lg_txlen:0x%x\n",
				ate_ctrl->control_band_idx, param.Data.set_phy_manual_tx.cg2_ddw1.mimo_ltf,
				param.Data.set_phy_manual_tx.cg2_ddw1.afactor, param.Data.set_phy_manual_tx.cg2_ddw1.txop,
				param.Data.set_phy_manual_tx.cg2_ddw1.pe_disamb, param.Data.set_phy_manual_tx.cg2_ddw1.ltf_symbol,
				param.Data.set_phy_manual_tx.cg2_ddw1.ppdu_sym_cnt, param.Data.set_phy_manual_tx.cg2_ddw1.ldpc_extra_symbol,
				param.Data.set_phy_manual_tx.cg2_ddw1.lg_txlen);

		param.Data.set_phy_manual_tx.user_g1.start_spatial_stream = ru_sta->start_sp_st;
		param.Data.set_phy_manual_tx.user_g1.ru_size = ru_sta->ru_index;

		param.Data.set_phy_manual_tx.user_g1.fec_coding = ru_sta->ldpc;
		param.Data.set_phy_manual_tx.user_g1.nsts = ru_sta->nss-1;
		param.Data.set_phy_manual_tx.user_g1.rate = ((ru_sta->rate & BIT(5)) ? ((ru_sta->rate & ~BIT(5)) | BIT(4)) : ru_sta->rate);
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[band:%d][TXV Common2 DDW1] start_spatial_stream:%d ru_size:%d fec_coding:%d, nsts:%d, rate:%d\n",
				ate_ctrl->control_band_idx, param.Data.set_phy_manual_tx.user_g1.start_spatial_stream,
				param.Data.set_phy_manual_tx.user_g1.ru_size, param.Data.set_phy_manual_tx.user_g1.fec_coding,
				param.Data.set_phy_manual_tx.user_g1.nsts, param.Data.set_phy_manual_tx.user_g1.rate);
		param.Data.set_phy_manual_tx.user_g2.length = apep_length+((ru_sta->ldpc) ? 2 : 3);
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[band:%d][TXV User2 DDW1] apep_length:0x%x\n", ate_ctrl->control_band_idx, param.Data.set_phy_manual_tx.user_g2.length);
		param.Data.set_phy_manual_tx.param.start = TRUE;
		param.Data.set_phy_manual_tx.param.ipg = 0;		/* make sure instrument's requirement*/
		param.Data.set_phy_manual_tx.param.pkt_cnt = 0;	/* continuous mode */
	} else {
		param.Data.set_phy_manual_tx.param.start = FALSE;
	}

	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	ret = MtCmdATETest(ad, &param);
#endif
	return ret;
}

static INT32 mt_ate_get_subcarriers(UINT8 ru_index, UINT8 dcm)
{
	INT32 subcarriers = 0, idx = 0;

	for (idx = 0 ; idx < ARRAY_SIZE(ru_const) ; idx++) {
		if (ru_index < ru_const[idx].max_index) {
			if (dcm)
				subcarriers = ru_const[idx].sd_d;
			else
				subcarriers = ru_const[idx].sd;

			break;
		}
	}

	return subcarriers;
}

static INT32 mt_ate_get_subcarriers_short(UINT8 ru_index, UINT8 dcm)
{
	INT32 subcarriers_short = 0, idx = 0;

	for (idx = 0 ; idx < ARRAY_SIZE(ru_const) ; idx++) {
		if (ru_index < ru_const[idx].max_index) {
			if (dcm)
				subcarriers_short = ru_const[idx].sd_s_d;
			else
				subcarriers_short = ru_const[idx].sd_s;

			break;
		}
	}

	return subcarriers_short;
}

static NDIS_STATUS mt_ate_calc_symbol_by_bytes(struct _ATE_RU_STA *ru_sta, BOOLEAN stbc, UCHAR rate_den, UINT32 apep_length)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	UINT32 m_stbc = 1, tail = 6, rate = 0;
	INT32 data_subcarriers = 0, data_subcarriers_short = 0;

	data_subcarriers = mt_ate_get_subcarriers(ru_sta->ru_index >> 1, (ru_sta->rate & BIT5));

	if (data_subcarriers)
		data_subcarriers_short = mt_ate_get_subcarriers_short(ru_sta->ru_index >> 1, (ru_sta->rate & BIT5));
	else {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"unknown RU Index:[%d]!\n", ru_sta->ru_index >> 1);
		ret = NDIS_STATUS_FAILURE;
		goto err_out;
	}

	rate = ru_sta->rate & (~BIT5);

	if (stbc)
		m_stbc++;

	if (ru_sta->ldpc)
		tail = 0;

	ru_sta->cbps = data_subcarriers * ru_sta->nss * he_bpscs[rate];
	ru_sta->dbps = ru_sta->cbps * (rate_den-1) / rate_den;
	ru_sta->cbps_s = data_subcarriers_short * ru_sta->nss * he_bpscs[rate];
	ru_sta->dbps_s = ru_sta->cbps_s * (rate_den-1) / rate_den;

	ru_sta->symbol_init = m_stbc * ate_ceil((8 * apep_length + 16 + tail), (m_stbc * ru_sta->dbps));
	ru_sta->excess = ((8 * apep_length + 16 + tail) % (m_stbc * ru_sta->dbps));


	MTWF_PRINT("%s: RU index[%d], apep length:%d symbol_init:%d, \n",
							__func__, ru_sta->ru_index >> 1, apep_length, ru_sta->symbol_init);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "R[%d/%d], cbps:%d, dbps:%d, cbps_s:%d, dbps_s:%d excess:%d\n",
							rate_den-1, rate_den, ru_sta->cbps, ru_sta->dbps,
							ru_sta->cbps_s, ru_sta->dbps_s, ru_sta->excess);

err_out:
	return ret;
}

static UINT32 mt_ate_calc_bytes_by_time(UCHAR tx_mode, UCHAR nss, UCHAR t_pe, UCHAR ltf, UCHAR gi, UINT32 dbps, UINT32 tx_time)
{
	UINT32 symbol_cnt = 0, psdu_length = 0;
	UINT8 m_stbc = 1, tail = 6;
	UINT32 ltf_time = ltf_sym[nss]*he_t_ltf_sym_x5[ltf];

	symbol_cnt = ate_ceil((tx_time*5 - (5 * 20 + (20+40+40+ltf_time) + he_t_pe_x5[t_pe])), he_t_sym_x5[gi]);

	if (symbol_cnt > 0x3fff)	/* H/W limitation */
		symbol_cnt = 0x3fff;

	psdu_length = ((symbol_cnt / m_stbc) * dbps - 16 - tail) / 8;

	MTWF_PRINT("\t%s: \tsymbol=%d, PSDU length:%d (0x3fff is H/W limiation)\n", __func__, symbol_cnt, psdu_length);

	return psdu_length;
}


static INT32 mt_ate_calc_afactor(struct _ATE_RU_STA *ru_sta)
{
	UINT32 ret = 0, m_stbc = 1;

	if (ru_sta->excess == 0) {
		ru_sta->excess = m_stbc * ru_sta->dbps;
		ru_sta->afactor_init = 4;
	} else {
		ru_sta->afactor_init = ate_min(4, ate_ceil(ru_sta->excess, (m_stbc * ru_sta->dbps_s)));
	}

	/* prepare for caculate ldpc extra symbol */
	if (ru_sta->afactor_init == 4) {
		ru_sta->dbps_last = ru_sta->dbps;
		ru_sta->cbps_last = ru_sta->cbps;
	} else {
		ru_sta->dbps_last = ru_sta->afactor_init * ru_sta->dbps_s;
		ru_sta->cbps_last = ru_sta->afactor_init * ru_sta->cbps_s;
	}

	ru_sta->pld = (ru_sta->symbol_init - m_stbc) * ru_sta->dbps + m_stbc * ru_sta->dbps_last;
	ru_sta->avbits = (ru_sta->symbol_init - m_stbc) * ru_sta->cbps + m_stbc * ru_sta->cbps_last;

	MTWF_PRINT("\t%s: \tafactor=%d, symbol cnt=%d\n", __func__, ru_sta->afactor_init, ru_sta->symbol_init);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "cbps_l:%d, dbps_l:%d, pld:%d, avbits:%d\n",
															ru_sta->cbps_last, ru_sta->dbps_last, ru_sta->pld, ru_sta->avbits);

	return ret;
}

static UINT32 mt_ate_calc_l_ldpc(UINT32 avbits, UINT32 pld, UCHAR rate_den, UINT32 *cw, UINT32 *l_ldpc)
{
	if (avbits <= 648) {
		*cw = 1;
		*l_ldpc = ((avbits >= (pld + 912/rate_den)) ? 2 : 1) * 648;
	} else if (avbits <= (648 * 2)) {
		*cw = 1;
		*l_ldpc = ((avbits >= (pld + 1464/rate_den)) ? 3 : 2) * 648;
	} else if (avbits <= (648 * 3)) {
		*cw = 1;
		*l_ldpc = (648 * 3);
	} else if (avbits <= (648 * 4)) {
		*cw = 2;
		*l_ldpc = ((avbits >= (pld + 2916/rate_den)) ? 3 : 2) * 648;
	} else {
		*cw = ate_ceil((pld * rate_den), ((648 * 3) * (rate_den-1)));
		*l_ldpc = (648 * 3);
	}

	return 0;
}

static BOOLEAN mt_ate_calc_extr_sym(struct _ATE_RU_STA *ru_sta, BOOLEAN stbc, UCHAR rate_den)
{
	BOOLEAN ret = FALSE;
	UINT32 cw = 0, l_ldpc = 0, shrt = 0;

	mt_ate_calc_l_ldpc(ru_sta->avbits, ru_sta->pld, rate_den, &cw, &l_ldpc);

	shrt = ate_max(0, (INT32)(cw * l_ldpc * (rate_den-1) / rate_den - ru_sta->pld));
	ru_sta->punc = ate_max(0, (INT32)(cw * l_ldpc - ru_sta->avbits - shrt));

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"cw:%d, avbits:%d, punc:%d, l_ldpc:%d, shrt:%d\n", cw, ru_sta->avbits, ru_sta->punc, l_ldpc, shrt);

	if (((10 * ru_sta->punc > cw * l_ldpc / rate_den) &&
		(5 * shrt < 6 * ru_sta->punc * (rate_den-1))) ||
		(10 * ru_sta->punc > 3 * cw * l_ldpc / rate_den))
		ret = TRUE;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LDPC extra symbol:%d\n", ret);

	return ret;
}

static UINT32 mt_ate_calc_pe_disamb(struct _ATE_RU_STA *ru_sta, UCHAR ltf_gi, UCHAR max_pe)
{
	INT32 ret = 0, gi = 0;
	UINT32 t_pe = ru_sta->afactor_init;
	UINT32 nss = ate_max(ru_sta->ru_mu_nss, ru_sta->nss);
	UINT32 ltf_time = ltf_sym[nss]*he_t_ltf_sym_x5[ltf_gi];

	if (ltf_gi == 2) {
		gi = GI_32;
	} else {
		gi = GI_16;
	}

	/* txtime = 20 + T_HE-PREAMBLE + N_SYM*T_SYM + N_MA*N_HE-LTF*T_HE-LTF-SYM + T_PE + SignalExtension (28-135)
		T_HE-PREAMBLE = T_RL-SIG + T_HE-SIG-A + T_HE-STF-T + N_HE-LTF*T_HE-LTF-SYM, for an HE TB PPDU
		Accoding to Table 28-12 of 802.11ax D3.0, T_RL-SIG = 4, T_HE-SIG-A = 8, T_HE-STF-T = 8, N_HE-LTF*T_HE-LTF-SYM (N_HE-LTF = {1,2,4,6}))
		N_MA = 0 due to doppler is not support, and SignalExtension = 0 due to not supported */
	ru_sta->tx_time_x5 =  5 * 20 + (20+40+40+ltf_time) + ru_sta->symbol_init * he_t_sym_x5[gi] + 0 + he_t_pe_x5[t_pe] + 0;
	ru_sta->l_len = ate_ceil((ru_sta->tx_time_x5-20*5), (4*5))*3-3-2;

	if ((he_t_pe_x5[t_pe] + 4 * (((ru_sta->tx_time_x5 - 20 * 5)%20) ? 1 : 0)) >= he_t_sym_x5[gi])
		ru_sta->pe_disamb = 1;
	else
		ru_sta->pe_disamb = 0;

	ru_sta->t_pe = t_pe;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "L-Len=%d, PE Disambiguilty=%d\n",
															ru_sta->l_len, ru_sta->pe_disamb);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "tx_time(x5)=%d, tx_ltf_sym(x5):%d, tx_sym(x5):%d, tx_pe(x5):%d\n",
															ru_sta->tx_time_x5, ltf_time, he_t_sym_x5[gi], he_t_pe_x5[t_pe]);

	return ret;
}

static NDIS_STATUS mt_ate_recalc_phy_info(struct _ATE_RU_STA *ru_sta, UINT8 stbc, UINT8 ltf_gi, UINT8 max_pe)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	UCHAR m_stbc = (stbc) ? 2 : 1;
	UINT32 cw = 0, l_ldpc = 0, shrt = 0;
	UCHAR rate_den = 0;

	rate_den = he_rate_density[ru_sta->rate & ~BIT5];

	if (ru_sta->afactor_init == 3)
		ru_sta->avbits += m_stbc * (ru_sta->cbps - (ru_sta->afactor_init * ru_sta->cbps_s));
	else
		ru_sta->avbits += m_stbc * ru_sta->cbps_s;

	mt_ate_calc_l_ldpc(ru_sta->avbits, ru_sta->pld, rate_den, &cw, &l_ldpc);

	shrt = ate_max(0, (INT32)(cw * l_ldpc * (rate_den-1) / rate_den - ru_sta->pld));
	ru_sta->punc = ate_max(0, (INT32)(cw * l_ldpc - ru_sta->avbits - shrt));

	if (ru_sta->afactor_init == 4) {
		ru_sta->symbol_init += m_stbc;
		ru_sta->afactor_init = 1;
	} else
		ru_sta->afactor_init++;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(re)afactor:%d, (re)cw:%d, (re)avbits:%d, \n\t\t\t(re)punc:%d, (re)l_ldpc:%d, (re)shrt:%d\n",
				ru_sta->afactor_init, cw, ru_sta->avbits, ru_sta->punc, l_ldpc, shrt);

	mt_ate_calc_pe_disamb(ru_sta, ltf_gi, max_pe);

	return ret;
}

static INT32 mt_ate_calc_phy_info(struct _ATE_RU_STA *ru_sta,
						UINT32 apep_length,
						UINT8 stbc,
						UINT8 ltf_gi,
						UINT8 max_pe)
{
	UCHAR rate_den = 0;

	rate_den = he_rate_density[ru_sta->rate & ~BIT5];
	mt_ate_calc_symbol_by_bytes(ru_sta, stbc, rate_den, apep_length);
	mt_ate_calc_afactor(ru_sta);
	mt_ate_calc_pe_disamb(ru_sta, ltf_gi, max_pe);

	if (ru_sta->ldpc && mt_ate_calc_extr_sym(ru_sta, stbc, rate_den)) {
		ru_sta->ldpc_extr_sym = 1;

		mt_ate_recalc_phy_info(ru_sta, stbc, ltf_gi, max_pe);
	}

	return 0;
}

static struct _ATE_RU_STA *mt_ate_dominate_ru(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	UINT16 sta_idx = 0;
	UINT8 dominate_user_idx = 0;
	UINT32 max_tx_time = 0;
	struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[0]);

	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		if (ru_sta[sta_idx].valid) {
			if (ru_sta[sta_idx].tx_time_x5 > max_tx_time) {
				max_tx_time = ru_sta[sta_idx].tx_time_x5;
				dominate_user_idx = sta_idx;
			}
		}
	}

	TESTMODE_SET_PARAM(ad, band_idx, dmnt_ru_idx, dominate_user_idx);
	ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[dominate_user_idx]);
	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "dominated by user[%d], RU index:%d\n",
								dominate_user_idx, ru_sta->ru_index >> 1);

	return ru_sta;
}
#endif /* DOT11_HE_AX */

INT MtATESetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_TRX;
	param.Data.rAteSetTrx.ucType = TxRx;
	param.Data.rAteSetTrx.ucEnable = Enable;
	param.Data.rAteSetTrx.ucBand = BandIdx; /* TODO::Fix it after 7615 merge */
	/* Make sure FW command configuration completed for store TX packet in PLE first
	 * Use aucReserved[1] for uxATEIdx extension feasibility
	 */
	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	MTWF_PRINT("%s: TxRx:%x, Enable:%x, BandIdx:%x\n",
		__func__, param.Data.rAteSetTrx.ucType,
		param.Data.rAteSetTrx.ucEnable,
		param.Data.rAteSetTrx.ucBand);
	ret = MtCmdATETest(pAd, &param);
#else
	ret = AsicSetMacTxRx(pAd, TxRx, Enable, BandIdx);
#endif
	return ret;
}


INT MtATESetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_TX_STREAM;
	param.Data.rAteSetTxStream.ucStreamNum = StreamNums;
	param.Data.rAteSetTxStream.ucBand = BandIdx;
	ret =  MtCmdATETest(pAd, &param);
	MTWF_PRINT("%s: StreamNum:%x BandIdx:%x\n", __func__, StreamNums, BandIdx);
#else
	ret = MtAsicSetTxStream(pAd, StreamNums, BandIdx);
#endif
	return ret;
}


INT MtATESetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR u1BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_RX_PATH;

	/* Set Rx Ant 2/3 for Band 1 */
	if (u1BandIdx)
		RxPathSel = RxPathSel << 2;

	param.Data.rAteSetRxPath.ucType = RxPathSel;
	param.Data.rAteSetRxPath.ucBand = u1BandIdx;
	ret = MtCmdATETest(pAd, &param);
	MTWF_PRINT("%s: RxPathSel:%x BandIdx:%x\n", __func__, RxPathSel, u1BandIdx);
#else
	ret = AsicSetRxPath(pAd, RxPathSel, u1BandIdx);
#endif
	return ret;
}


INT MtATESetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T filter)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_RX_FILTER;
	param.Data.rAteSetRxFilter.ucBand = filter.u1BandIdx; /* TODO::Fix it after 7615 merge */

	if (filter.bPromiscuous)
		param.Data.rAteSetRxFilter.ucPromiscuousMode = 1;
	else {
		param.Data.rAteSetRxFilter.ucReportEn = (UCHAR)filter.bFrameReport;
		param.Data.rAteSetRxFilter.u4FilterMask = cpu2le32(filter.filterMask);
	}

	ret =  MtCmdATETest(pAd, &param);
	MTWF_PRINT("%s: BandIdx:%x\n", __func__, filter.u1BandIdx);
#else
	ret = AsicSetRxFilter(pAd, filter);
#endif
	return ret;
}


INT MtATESetCleanPerStaTxQueue(RTMP_ADAPTER *pAd, BOOLEAN sta_pause_enable)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, control_band_idx, stack);

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_CLEAN_PERSTA_TXQUEUE;
	param.Data.rAteSetCleanPerStaTxQueue.fgStaPauseEnable = sta_pause_enable;
	/* Give a same STA ID */
	param.Data.rAteSetCleanPerStaTxQueue.ucStaID = 0;

	/* omac binding is fixed, need to get from wdev_idx*/
	wdev = (struct wifi_dev *)stack->wdev[0];

	param.Data.rAteSetCleanPerStaTxQueue.ucBand = control_band_idx;
	param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0] = HcGetOmacIdx(pAd, wdev); /* use omac*/

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"StaPauseEnable:%d, StaID:%d, Band:%d, Reserved[0]:%d\n",
			param.Data.rAteSetCleanPerStaTxQueue.fgStaPauseEnable,
			param.Data.rAteSetCleanPerStaTxQueue.ucStaID,
			param.Data.rAteSetCleanPerStaTxQueue.ucBand,
			param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0]);

	ret =  MtCmdATETest(pAd, &param);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"sta_pause_enable:%x\n", sta_pause_enable);
#else
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"Function do not support yet.\n");
#endif
	return ret;
}

#ifdef ARBITRARY_CCK_OFDM_TX
VOID MtATEInitCCK_OFDM_Path(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 i = 0;
	UINT32 value = 0;
	UINT32 MAC_RDVLE, MAC_ADDR, MAC_WRMASK, MAC_WRVALUE;

	if (IS_ATE_DBDC(pAd)) {
		UINT32 idx = 0;

		/* PTA mux */
		if (BandIdx == 0) {
			/* 0x810600D0[4:0] = 0x8; */
			MAC_ADDR = ANT_SWITCH_CON3;
			MAC_WRMASK = 0xFFFFFFE0;
			MAC_WRVALUE = 0x8 << 0;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			/* 0x810600D4[20:16] = 0xE; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFE0FFFF;
			MAC_WRVALUE = 0xE << 16;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		} else {
			/* 0x810600E0[11:8] = 0x5; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFFFF0FF;
			MAC_WRVALUE = 0x5 << 8;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			/* 0x810600E4[27:24] = 0xB; */
			MAC_ADDR = ANT_SWITCH_CON8;
			MAC_WRMASK = 0xF0FFFFFF;
			MAC_WRVALUE = 0xB << 24;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		/* iPA */
		for (i = 0; i < 2; i++) {
			idx = BandIdx * 2 + i;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Reset WF_%d\n", idx);
			ATEOp->RfRegRead(pAd, idx, 0x48, &value);
			value &= ~(0x3FF << 20); /* bit[29:20] */
			value |= (3 << 20);
			ATEOp->RfRegWrite(pAd, idx, 0x48, value);
		}
	} else {
		/* PTA mux */
		/* Gband */
		/* 0x810600D0[4:0] = 0x8; */
		MAC_ADDR = ANT_SWITCH_CON3;
		MAC_WRMASK = 0xFFFFFFE0;
		MAC_WRVALUE = 0x8 << 0;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600D4[20:16] = 0xE; */
		MAC_ADDR = ANT_SWITCH_CON4;
		MAC_WRMASK = 0xFFE0FFFF;
		MAC_WRVALUE = 0xE << 16;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600DC[3:0] = 0x0; */
		MAC_ADDR = ANT_SWITCH_CON6;
		MAC_WRMASK = 0xFFFFFFF0;
		MAC_WRVALUE = 0x0 << 0;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E0[19:16] = 0x6; */
		MAC_ADDR = ANT_SWITCH_CON7;
		MAC_WRMASK = 0xFFF0FFFF;
		MAC_WRVALUE = 0x6 << 16;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* Aband */
		/* 0x810600D4[12:8] = 0xD; */
		MAC_ADDR = ANT_SWITCH_CON4;
		MAC_WRMASK = 0xFFFFE0FF;
		MAC_WRVALUE = 0xD << 8;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600CC[28:24] = 0x13; */
		MAC_ADDR = ANT_SWITCH_CON2;
		MAC_WRMASK = 0xE0FFFFFF;
		MAC_WRVALUE = 0x13 << 24;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E0[11:8] = 0x5; */
		MAC_ADDR = ANT_SWITCH_CON7;
		MAC_WRMASK = 0xFFFFF0FF;
		MAC_WRVALUE = 0x5 << 8;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E4[27:24] = 0xB; */
		MAC_ADDR = ANT_SWITCH_CON8;
		MAC_WRMASK = 0xF0FFFFFF;
		MAC_WRVALUE = 0xB << 24;
		MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);

		for (i = 0; i < 4; i++) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Reset WF_%d\n", i);
			ATEOp->RfRegRead(pAd, i, 0x48, &value);
			value &= ~(0x3FF << 20); /* bit[29:20] */
			value |= (3 << 20);
			ATEOp->RfRegWrite(pAd, i, 0x48, value);
		}
	}
}


VOID MtATESetCCK_OFDM_Path(RTMP_ADAPTER *pAd, UINT32 TxPathSel, UCHAR BandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 i = 0;
	UINT32 value = 0;
	UINT32 MAC_RDVLE, MAC_ADDR, MAC_WRMASK, MAC_WRVALUE;

	if (IS_ATE_DBDC(pAd)) {
		UINT32 idx = 0;

		/* PTA mux */
		if (BandIdx == 0) {
			if ((TxPathSel & (1 << 0)) == 0) {
				/* 0x810600D0[4:0] = 0x1B; */
				MAC_ADDR = ANT_SWITCH_CON3;
				MAC_WRMASK = 0xFFFFFFE0;
				MAC_WRVALUE = 0x1B << 0;
				MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			}

			if ((TxPathSel & (1 << 1)) == 0) {
				/* 0x810600D4[20:16] = 0x1B; */
				MAC_ADDR = ANT_SWITCH_CON4;
				MAC_WRMASK = 0xFFE0FFFF;
				MAC_WRVALUE = 0x1B << 16;
				MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			}
		} else {
			if ((TxPathSel & (1 << 0)) == 0) {
				/* 0x810600E0[11:8] = 0xF; */
				MAC_ADDR = ANT_SWITCH_CON7;
				MAC_WRMASK = 0xFFFFF0FF;
				MAC_WRVALUE = 0xF << 8;
				MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			}

			if ((TxPathSel & (1 << 1)) == 0) {
				/* 0x810600E4[27:24] = 0xF; */
				MAC_ADDR = ANT_SWITCH_CON8;
				MAC_WRMASK = 0xF0FFFFFF;
				MAC_WRVALUE = 0xF << 24;
				MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
			}
		}

		for (i = 0; i < 2; i++) {
			if ((TxPathSel & (1 << i)) == 0) {
				idx = BandIdx * 2 + i;
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Disable WF_%d, TxSel=%x\n", idx, TxPathSel);
				ATEOp->RfRegRead(pAd, idx, 0x48, &value);
				value &= ~(0x3FF << 20); /* bit[29:20] */
				value = value | (2 << 28) | (2 << 26) | (8 << 20);
				ATEOp->RfRegWrite(pAd, idx, 0x48, value);
			}
		}
	} else {
		/* PTA mux */
		/* Gband */
		if ((TxPathSel & (1 << 0)) == 0) {
			/* 0x810600D0[4:0] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON3;
			MAC_WRMASK = 0xFFFFFFE0;
			MAC_WRVALUE = 0x1B << 0;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 1)) == 0) {
			/* 0x810600D4[20:16] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFE0FFFF;
			MAC_WRVALUE = 0x1B << 16;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 2)) == 0) {
			/* 0x810600DC[3:0] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON6;
			MAC_WRMASK = 0xFFFFFFF0;
			MAC_WRVALUE = 0xF << 0;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 3)) == 0) {
			/* 0x810600E0[19:16] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFF0FFFF;
			MAC_WRVALUE = 0xF << 16;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		/* Aband */
		if ((TxPathSel & (1 << 0)) == 0) {
			/* 0x810600D4[12:8] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFFFE0FF;
			MAC_WRVALUE = 0x1B << 8;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 1)) == 0) {
			/* 0x810600CC[28:24] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON2;
			MAC_WRMASK = 0xE0FFFFFF;
			MAC_WRVALUE = 0x1B << 24;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 2)) == 0) {
			/* 0x810600E0[11:8] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFFFF0FF;
			MAC_WRVALUE = 0xF << 8;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 3)) == 0) {
			/* 0x810600E4[27:24] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON8;
			MAC_WRMASK = 0xF0FFFFFF;
			MAC_WRVALUE = 0xF << 24;
			MCU_IO_READ32(pAd->hdev_ctrl, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd->hdev_ctrl, MAC_ADDR, MAC_RDVLE);
		}

		for (i = 0; i < 4; i++) {
			if ((TxPathSel & (1 << i)) == 0) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Disable WF_%d, TxSel=%x\n", i, TxPathSel);
				ATEOp->RfRegRead(pAd, i, 0x48, &value);
				value &= ~(0x3FF << 20); /* bit[29:20] */
				value = value | (2 << 28) | (2 << 26) | (8 << 20);
				ATEOp->RfRegWrite(pAd, i, 0x48, value);
			}
		}
	}
}
#endif

#if defined(TESTMODE_TX_CONTROL)
#endif

static VOID  mt_ate_sx_inc_cal_ext(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
#define B0_WF2   0x2
#define B1_WF2   0x12
#define RO_AD_WF_SX_FCAL_INC_DN   BIT(1)
#define RO_AD_WF_SX_FCAL_INC_UP   BIT(0)
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR control_band_idx;
	UINT32 regC = 0, regE3C = 0;
	UINT32 WfSel;
	INT i;

	for (control_band_idx = TESTMODE_BAND0; control_band_idx <= TESTMODE_BAND1; control_band_idx++) {
		WfSel = (control_band_idx == TESTMODE_BAND0?B0_WF2 : B1_WF2);
		ATEOp->RfRegRead(pAd, WfSel, 0xE3C, &regE3C);
		ATEOp->RfRegRead(pAd, WfSel, 0xC, &regC);

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"(%d), rE3C=%x, regC:%x, unlock:%d\n", __LINE__,
				 regE3C, regC, regE3C & RO_AD_WF_SX_FCAL_INC_UP || regE3C & RO_AD_WF_SX_FCAL_INC_DN);

		if (regE3C & RO_AD_WF_SX_FCAL_INC_UP || regE3C & RO_AD_WF_SX_FCAL_INC_DN) {
			regC |= BIT(17);  /* enable SPI2RF_SX_INC-CAL_MAN */

			for (i = 0; i < 10; i++) {
				ATEOp->RfRegWrite(pAd, WfSel, 0xc, regC);
				regC |= BIT(16); /* toggle SPI2RF_SX_INC_CAL */
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"(%d), toggle #%d, WfSel:%d, regC:0x%x\n", __LINE__, i, WfSel, regC);
				ATEOp->RfRegWrite(pAd, WfSel, 0xc, regC);
				regC &= ~BIT(16);

				ATEOp->RfRegWrite(pAd, WfSel, 0xc, regC);
			}

			/* manual incremental cal off  */
			regC &= ~(BIT(17) || BIT(16));
			ATEOp->RfRegWrite(pAd, (control_band_idx == TESTMODE_BAND0?B0_WF2 : B1_WF2), 0xc, regC);
		}
	}
}
#ifdef ATE_TXTHREAD
static INT TestMode_TxThread(ULONG Context);
static INT32 TESTMODE_TXTHREAD_INIT(RTMP_ADAPTER *pAd, INT thread_idx);
static INT32 TESTMODE_TXTHREAD_RELEASE(RTMP_ADAPTER *pAd, INT thread_idx);
static VOID TESTMODEThreadProceedTx(RTMP_ADAPTER *pAd, UINT32 band_idx);
static VOID TESTMODEThreadStopTx(RTMP_ADAPTER *pAd, INT thread_idx);

#if defined(DBDC_MODE)
#endif /* defined(DBDC_MODE) */

static INT MT_ATETxHandler(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	UINT32 mode = TESTMODE_GET_PARAM(pAd, band_idx, op_mode);
	USHORT q_idx = TESTMODE_GET_PARAM(pAd, band_idx, ac_idx);
	INT32 dequeue_size = ate_ctrl->deq_cnt;
	INT32 multi_users = 0;
	UINT32 txed_cnt = 0;
	UINT32 tx_cnt = 0;
	UCHAR hwq_idx = q_idx;
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);
#if defined(MT_MAC) && defined(CUT_THROUGH)
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
	UINT32 ipg = ipg_param->ipg;
#endif /* defined(MT7615) || defined(MT7622) */
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, band_idx, stack);

#if defined(MT_MAC)
	if (IS_HIF_TYPE(pAd, HIF_MT))
		hwq_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, TX_MGMT, q_idx);
#endif
	txed_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TXED_CNT);
	tx_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TX_CNT);

	do {
		ULONG free_num;
ate_thread_dequeue:
#ifdef COMPOS_TESTMODE_WIN
		free_num = 1;
#else
		free_num = hif_get_tx_resource_free_num(pAd->hdev_ctrl, hwq_idx);
#endif /* COMPOS_TESTMODE_WIN */

		if (free_num < stack->index) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[band%d] insificient ring buffer(resource=%ld:request=%d), thread dissmised!\n", band_idx, free_num, stack->index);
			break;
		}

/*		if (multi_users > 0) {
			UCHAR *pate_pkt = TESTMODE_GET_PARAM(pAd, band_idx, test_pkt);

			ate_ctrl->wcid_ref = multi_users;
			ret = MT_ATEComposePkt(pAd, pate_pkt, band_idx, 0);
		}*/
		mode = TESTMODE_GET_PARAM(pAd, band_idx, op_mode);

		if (mode & ATE_STOP)
			break;

		if (!(mode & ATE_TXFRAME))
			break;

		if (!free_num)
			break;

#if defined(MT_MAC) && defined(CUT_THROUGH)
round_tx:
		if (((pkt_tx_time > 0) || (ipg > 0)) &&
			(pAd->mgmt_que[0].Number >= MGMT_QUE_MAX_NUMS)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[band%d] mgmt queue is full, thread dissmised!\n", band_idx);
			break;
		}

#endif /* defined(MT7615) || defined(MT7622) */

		/* For ATE TX thread TX packet counter control */
		if (tx_cnt <= txed_cnt) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"[band%d] reach counter(%d), thread dissmised!\n", band_idx, txed_cnt*stack->index);
			break;
		}

		ret = mt_ate_enq_pkt(pAd, band_idx, stack->q_idx);

		if (ret)
			break;

#if defined(MT_MAC) && defined(CUT_THROUGH)
		stack->q_idx++;
		if (stack->quota)
			stack->quota--;
		if (stack->q_idx == stack->index) {
			txed_cnt++;
			stack->q_idx = 0;
		}

		if (((pkt_tx_time > 0) && need_ampdu) || (ipg > 0)) {
			PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
			struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
			UINT32 free_token_cnt =	atomic_read(&que->free_token_cnt);
			UINT32 pkt_tx_token_id_max = que->pkt_tkid_end;

			free_num = hif_get_tx_resource_free_num(pAd->hdev_ctrl, hwq_idx);

			if ((free_token_cnt > (pkt_tx_token_id_max - ATE_ENQUEUE_PACKET_NUM)) && (free_num > 0)) {
				if (stack->quota)
					goto round_tx;
			}
		}
#endif /* defined(MT7615) || defined(MT7622) */
		dequeue_size--;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"band_idx:%u, tx_cnt:%u, txed_cnt:%u, deque:%d, multi_user:%u, free:%lu\n",
			band_idx, tx_cnt, txed_cnt,
			dequeue_size, multi_users, free_num);

		if (!dequeue_size) {
			multi_users--;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "Dequeue %d finish, multi_user:%d\n",
				  dequeue_size, multi_users);
		} else
			goto ate_thread_dequeue;
	} while (multi_users > 0);

	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXED_CNT, txed_cnt);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TX_CNT, tx_cnt);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "band_idx:%u, tx_cnt:%u, txed_cnt:%u, deque:%d\n",
			  band_idx, tx_cnt, txed_cnt, dequeue_size);
	return ret;
}


static INT32 Mt_ATEThreadGetBandIdx(RTMP_ADAPTER *pAd, UINT8 *stat)
{
	UINT8 mask = 0;

	mask = 1 << TESTMODE_BAND0;

	if (*stat & mask) {
		*stat &= ~mask;
		return TESTMODE_BAND0;
	}

	mask = 1 << TESTMODE_BAND1;

	if (IS_ATE_DBDC(pAd) && (*stat & mask)) {
		*stat &= ~mask;
		return TESTMODE_BAND1;
	}

	return -1;
}

static INT TestMode_TxThread(ULONG Context)
{
	int status;
	INT32 ret = 0;
	RTMP_OS_TASK *pTask = (RTMP_OS_TASK *)Context;
	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_CTRL *ate_ctrl = NULL;
	struct _ATE_TXTHREAD_CB *tx_thread_cb = NULL;
	INT32 band_idx = 0;
	UINT8 service_stat = 0;
	UINT32 mode = 0;

	if (!pTask)
		goto err1;

	pAd = (RTMP_ADAPTER *)RTMP_OS_TASK_DATA_GET(pTask);
	ate_ctrl = &pAd->ATECtrl;
	tx_thread_cb = &ate_ctrl->tx_thread[0];

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Init thread %u for band %u\n",
		ate_ctrl->current_init_thread, band_idx);
	RTMP_OS_COMPLETE(&ate_ctrl->cmd_done);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		service_stat = 0;
		RTMP_SEM_LOCK(&tx_thread_cb->lock);
		service_stat = tx_thread_cb->service_stat;

		/* chip_set_hif_dma(pAd, DMA_TX_RX, 0); */
		do {
			if (!service_stat)
				break;

			band_idx = Mt_ATEThreadGetBandIdx(pAd, &service_stat);

			if (band_idx == -1)
				break;

			ret = MT_ATETxHandler(pAd, band_idx);
		} while (1);

		tx_thread_cb->service_stat = service_stat;
		/* chip_set_hif_dma(pAd, DMA_TX_RX, 1); */
		RTMP_SEM_UNLOCK(&tx_thread_cb->lock);

		if (band_idx == -1)
			goto err1;

		mode = TESTMODE_GET_PARAM(pAd, band_idx, op_mode);

		if (mode & fATE_MPS) {
			MT_ATEMPSRunStatCheck(pAd, band_idx);
			MT_ATEMPSLoadSetting(pAd, band_idx);
		}

		schedule();

		if (ret)
			break;
	}

err1:
	RtmpOSTaskNotifyToExit(pTask);
	if (pAd) {
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, band_idx);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	}
	if (tx_thread_cb)
		tx_thread_cb->is_init = FALSE;

	if (ret)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"abnormal leave err %d\n", ret);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"leave\n");
	return ret;
}


static INT32 TESTMODE_TXTHREAD_INIT(RTMP_ADAPTER *pAd, INT thread_idx)
{
	INT32 Ret = 0;
	CHAR thread_name[64] = "ATE_Thread";
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];

	ate_ctrl->deq_cnt = 1;

	if (!ate_ctrl->tx_thread[thread_idx].is_init) {
		NdisZeroMemory(tx_cb, sizeof(*tx_cb));
		/* sprintf(thread_name, "ATE_Thread%d", thread_idx); */
		RTMP_OS_TASK_INIT(&tx_cb->task, thread_name, pAd);
		ate_ctrl->current_init_thread = thread_idx;
		NdisAllocateSpinLock(pAd, &tx_cb->lock);
		Ret =  RtmpOSTaskAttach(&tx_cb->task, TestMode_TxThread, (ULONG)&tx_cb->task);

		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire))
			goto err0;

		if (Ret != STATUS_SUCCESS)
			goto err0;

		tx_cb->is_init = TRUE;
	} else {
		tx_cb->txed_cnt = 0;
		tx_cb->tx_cnt = 0;
	}

	tx_cb->service_stat = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Initialize thread_idx=%d\n", thread_idx);
	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"tx thread create fail\n");
	return Ret;
}


static INT32 TESTMODE_TXTHREAD_RELEASE(RTMP_ADAPTER *pAd, INT thread_idx)
{
	INT32 Ret = 0;
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];

	if (&tx_cb->task)
		Ret = RtmpOSTaskKill(&tx_cb->task);

	if (Ret == NDIS_STATUS_FAILURE)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"kill ATE Tx task failed!\n");
	else
		tx_cb->is_init = FALSE;

	NdisFreeSpinLock(&tx_cb->lock);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Release thread_idx=%d\n",
		thread_idx);
	return Ret;
}


static INT MT_ATEThreadSetService(RTMP_ADAPTER *pAd, UINT32 band_idx, UINT8 *stat)
{
	UINT8 mask = 0;

	if (IS_ATE_DBDC(pAd) && (band_idx == TESTMODE_BAND1))
		mask = 1 << TESTMODE_BAND1;
	else
		mask = 1 << TESTMODE_BAND0;

	*stat |= mask;
	return 0;
}


static VOID TESTMODEThreadProceedTx(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_TXTHREAD_CB *tx_cb = NULL;
	tx_cb = &pAd->ATECtrl.tx_thread[0];
	RTMP_SEM_LOCK(&tx_cb->lock);
	MT_ATEThreadSetService(pAd, band_idx, &tx_cb->service_stat);
	RTMP_SEM_UNLOCK(&tx_cb->lock);
	RtmpOsTaskWakeUp(&tx_cb->task);
}


static VOID TESTMODEThreadStopTx(RTMP_ADAPTER *pAd, INT thread_idx)
{
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];
	tx_cb->txed_cnt = 0;
	tx_cb->tx_cnt = 0;
}

static INT TestMode_PeriodicThread(ULONG Context);
static INT32 TESTMODE_PeriodicThread_INIT(RTMP_ADAPTER *pAd);
static INT32 TESTMODE_PeriodicThread_RELEASE(RTMP_ADAPTER *pAd);

static INT32 TESTMODE_PeriodicThread_INIT(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	CHAR thread_name[64] = "ATE_PThread";
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	struct _ATE_PERIODIC_THREAD_CB *p_cb = &pAd->ATECtrl.periodic_thread;

	if (!p_cb->is_init) {
		NdisZeroMemory(p_cb, sizeof(*p_cb));
		/* sprintf(thread_name, "ATE_Thread%d", thread_idx); */
		RTMP_OS_TASK_INIT(&p_cb->task, thread_name, pAd);
		NdisAllocateSpinLock(pAd, &p_cb->lock);
		Ret =  RtmpOSTaskAttach(&p_cb->task, TestMode_PeriodicThread, (ULONG)&p_cb->task);

		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire))
			goto err0;

		if (Ret != STATUS_SUCCESS)
			goto err0;

		p_cb->is_init = TRUE;
		p_cb->service_stat = TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Initialize Periodic thread Fail\n");
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Initialize Periodic thread OK\n");

	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"tx thread Periodic create fail\n");
	return Ret;
}


static INT32 TESTMODE_PeriodicThread_RELEASE(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _ATE_PERIODIC_THREAD_CB *p_cb = &pAd->ATECtrl.periodic_thread;

	if (&p_cb->task)
		Ret = RtmpOSTaskKill(&p_cb->task);

	if (Ret == NDIS_STATUS_FAILURE)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"kill ATE Periodic task failed!\n");
	else {
		p_cb->is_init = FALSE;
		p_cb->service_stat  = FALSE;
	}

	NdisFreeSpinLock(&p_cb->lock);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Release Done\n");
	return Ret;
}

static INT TestMode_PeriodicThread(ULONG Context)
{
	int status;
	RTMP_OS_TASK *pTask = (RTMP_OS_TASK *)Context;
	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_PERIODIC_THREAD_CB *p_cb = NULL;
	struct _ATE_CTRL *ate_ctrl = NULL;

	if (!pTask) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"abnormal leave, pTask is NULL\n");
		return -1;
	}

	pAd = (RTMP_ADAPTER *)RTMP_OS_TASK_DATA_GET(pTask);
	p_cb = &pAd->ATECtrl.periodic_thread;
	ate_ctrl = &pAd->ATECtrl;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Periodic Thread running\n");
	RTMP_OS_COMPLETE(&ate_ctrl->cmd_done);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		do {
			if (!p_cb->service_stat)
				break;

			if (IS_MT7626(pAd)) {
				mt_ate_sx_inc_cal_ext(NULL, pAd, NULL, NULL);
			}

			OS_WAIT(1000);

		} while (1);

		schedule();

		break;
	}

	RtmpOSTaskNotifyToExit(pTask);
	p_cb->is_init = FALSE;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"leave, service_stat:%d\n", p_cb->service_stat);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	return 0;
}


#endif /* ATE_TXTHREAD */


VOID MT_ATEUpdateRxStatistic(RTMP_ADAPTER *pAd, enum _TESTMODE_STAT_TYPE type, VOID *data)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_RX_STATISTIC *rx_stat = &ATECtrl->rx_stat;
	UCHAR *uData = (UCHAR *)data;

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(uData + 8);
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(uData + 16);
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(uData + 24);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(uData + 12);
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(uData + 20);

	/* RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(uData + 32); */
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(uData + 40);
#else
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(uData + 36);
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(uData + 28);
#endif

	if (type == TESTMODE_RXV) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
		INT16 foe = 0;
		UINT32 i = 0;
		UCHAR RssiOffset[4] = {0};
		UCHAR ch = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), channel);

		if (RXV1_1ST_CYCLE->TxMode == MODE_CCK) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "MODE_CCK RX\n");
			foe = (RXV1_5TH_CYCLE->MISC1 & 0x7ff);
			foe = (foe * 1000) >> 11;
		} else {
			UINT8 cbw = RXV1_1ST_CYCLE->FrMode;
			UINT32 foe_const = ((1 << (cbw + 1)) & 0xf) * 10000;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "MODE_OFDM RX\n");
			foe = (RXV1_5TH_CYCLE->MISC1 & 0xfff);

			if (foe >= 2048)
				foe = foe - 4096;

			foe = (foe * foe_const) >> 15;
		}

		rx_stat->FreqOffsetFromRx[0] = foe;
		rx_stat->RCPI[0] = RXV1_4TH_CYCLE->RCPI0;
		rx_stat->RCPI[1] = RXV1_4TH_CYCLE->RCPI1;
		rx_stat->RCPI[2] = RXV1_4TH_CYCLE->RCPI2;
		rx_stat->RCPI[3] = RXV1_4TH_CYCLE->RCPI3;
		AsicFeLossGet(pAd, ch, RssiOffset);
		rx_stat->RSSI[0] = ((RXV1_4TH_CYCLE->RCPI0 >> 1) - 110) + RssiOffset[0];
		rx_stat->RSSI[1] = ((RXV1_4TH_CYCLE->RCPI1 >> 1) - 110) + RssiOffset[1];
		rx_stat->RSSI[2] = ((RXV1_4TH_CYCLE->RCPI2 >> 1) - 110) + RssiOffset[2];
		rx_stat->RSSI[3] = ((RXV1_4TH_CYCLE->RCPI3 >> 1) - 110) + RssiOffset[3];
		rx_stat->FAGC_RSSI_IB[0] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[0] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[1] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[1] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[2] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[2] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[3] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[3] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->SNR[0] = (RXV1_5TH_CYCLE->MISC1 >> 19) - 16;

		for (i = 0; i < 4; i++) {
			if (rx_stat->FAGC_RSSI_IB[i] >= 128)
				rx_stat->FAGC_RSSI_IB[i] -= 256;

			if (rx_stat->FAGC_RSSI_WB[i] >= 128)
				rx_stat->FAGC_RSSI_WB[i] -= 256;
		}

#ifdef CFG_SUPPORT_MU_MIMO

		if (RXV1_2ND_CYCLE->RxValidIndicator &&
			((RXV1_2ND_CYCLE->GroupId != 0) &&
			 (RXV1_2ND_CYCLE->GroupId != 63))) {
			rx_stat->RxMacMuPktCount++;
			pAd->u4RxMuPktCount++;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"GroupId:%d get MU packet #:%d\n",
				RXV1_2ND_CYCLE->GroupId, rx_stat->RxMacMuPktCount);
		}

#endif
		rx_stat->SIG_MCS = RXV1_1ST_CYCLE->TxRate;
		rx_stat->SINR = (*(UINT32 *)RXV2_3TH_CYCLE) & 0x00FFFFFF; /* bit[23:0] */
		rx_stat->RXVRSSI = RXV1_3TH_CYCLE->IBRssiRx;
#else
		rx_stat->FreqOffsetFromRx[0] = RXV1_5TH_CYCLE->FoE;
		rx_stat->RCPI[0] = RXV1_3TH_CYCLE->Rcpi0;
		rx_stat->RCPI[1] = RXV1_3TH_CYCLE->Rcpi1;
		rx_stat->SNR[0] = RXV1_5TH_CYCLE->LTF_SNR0;
		rx_stat->SNR[1] = RXV2_2ND_CYCLE->OfdmLtfSNR1;
		rx_stat->RSSI[0] = RXV1_3TH_CYCLE->Rcpi0 / 2 - 110;
		rx_stat->RSSI[1] = RXV1_3TH_CYCLE->Rcpi1 / 2 - 110;
#endif /* defined(MT7615) || defined(MT7622) */

		if (pAd->ATECtrl.en_log & fATE_LOG_RXV)
			MT_ATEInsertLog(pAd, data, fATE_LOG_RXV, sizeof(struct _ATE_RXV_LOG));

		if (pAd->ATECtrl.en_log & fATE_LOG_RXINFO) {
			UINT32 nsts = 0;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
			nsts = RXV1_2ND_CYCLE->NstsField;
#else
			nsts = RXV1_6TH_CYCLE->NsTsField;
#endif
#if !defined(COMPOS_TESTMODE_WIN)
			MTWF_PRINT("\t\tPhyMode=%d(%s)\n", RXV1_1ST_CYCLE->TxMode, get_phymode_str(RXV1_1ST_CYCLE->TxMode));
			MTWF_PRINT("\t\tMCS=%d\n", RXV1_1ST_CYCLE->TxRate);
			MTWF_PRINT("\t\tBW=%d\n", RXV1_1ST_CYCLE->FrMode);
			MTWF_PRINT("\t\tSGI=%d\n", RXV1_1ST_CYCLE->HtShortGi);
			MTWF_PRINT("\t\tSTBC=%d\n", RXV1_1ST_CYCLE->HtStbc);
			MTWF_PRINT("\t\tLDPC=%d\n", RXV1_1ST_CYCLE->HtAdCode);
			MTWF_PRINT("\t\tNsts=%d\n", nsts);
#endif /* !defined(COMPOS_TESTMODE_WIN) */
		} else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "EN_LOG:%x\n", pAd->ATECtrl.en_log);
	} else if (type == TESTMODE_RESET_CNT) {
		NdisZeroMemory(rx_stat, sizeof(*rx_stat));
		rx_stat->MaxRssi[0] = 0xff;
		rx_stat->MaxRssi[1] = 0xff;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
		rx_stat->MaxRssi[2] = 0xff;
		rx_stat->MaxRssi[3] = 0xff;
#endif
	} else if (type == TESTMODE_COUNTER_802_11) {
		COUNTER_802_11 *wlanCounter = (COUNTER_802_11 *)data;
		rx_stat->RxMacFCSErrCount =
			wlanCounter->FCSErrorCount.u.LowPart;
	}
}

INT MtTestModeBkCr(PRTMP_ADAPTER pAd, ULONG offset, enum _TEST_BK_CR_TYPE type)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _TESTMODE_BK_CR *bks = ATECtrl->bk_cr;
	struct _TESTMODE_BK_CR *entry = NULL;
	INT32 i;

	if ((type >= TEST_BKCR_TYPE_NUM) || (type == TEST_EMPTY_BKCR))
		return NDIS_STATUS_INVALID_DATA;

	for (i = 0; i < MAX_TEST_BKCR_NUM; i++) {
		struct _TESTMODE_BK_CR *tmp = &bks[i];

		if ((tmp->type == TEST_EMPTY_BKCR) && (entry == NULL)) {
			entry = tmp;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"find emptyp bk entry %d\n", i);
			break;
		} else if ((tmp->type == type) && (tmp->offset == offset)) {
			entry = tmp;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"update bk entry %d\n", i);
			break;
		}
	}

	if (!entry)
		return NDIS_STATUS_RESOURCES;

	entry->type = type;
	entry->offset = offset;

	switch (type) {
	case TEST_MAC_BKCR:
		MAC_IO_READ32(pAd->hdev_ctrl, offset, &entry->val);
		break;

	case TEST_HIF_BKCR:
		HIF_IO_READ32(pAd->hdev_ctrl, offset, &entry->val);
		break;

	case TEST_PHY_BKCR:
		PHY_IO_READ32(pAd->hdev_ctrl, offset, &entry->val);
		break;

	case TEST_HW_BKCR:
		HW_IO_READ32(pAd->hdev_ctrl, offset, &entry->val);
		break;

	case TEST_MCU_BKCR:
		MCU_IO_READ32(pAd->hdev_ctrl, offset, &entry->val);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"bk-type not supported\n");
		entry->type = TEST_EMPTY_BKCR;
		entry->offset = 0;
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


INT MtTestModeRestoreCr(PRTMP_ADAPTER pAd, ULONG offset)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _TESTMODE_BK_CR *bks = ATECtrl->bk_cr;
	struct _TESTMODE_BK_CR *entry = NULL;
	INT32 i;

	for (i = 0; i < MAX_TEST_BKCR_NUM; i++) {
		struct _TESTMODE_BK_CR *tmp = &bks[i];

		if (tmp->offset == offset) {
			entry = tmp;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"find entry %d\n", i);
			break;
		}
	}

	if (!entry)
		return NDIS_STATUS_INVALID_DATA;

	switch (entry->type) {
	case TEST_MAC_BKCR:
		MAC_IO_WRITE32(pAd->hdev_ctrl, offset, entry->val);
		break;

	case TEST_HIF_BKCR:
		HIF_IO_WRITE32(pAd->hdev_ctrl, offset, entry->val);
		break;

	case TEST_PHY_BKCR:
		PHY_IO_WRITE32(pAd->hdev_ctrl, offset, entry->val);
		break;

	case TEST_HW_BKCR:
		HW_IO_WRITE32(pAd->hdev_ctrl, offset, entry->val);
		break;

	case TEST_MCU_BKCR:
		MCU_IO_WRITE32(pAd->hdev_ctrl, offset, entry->val);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"bk-type not supported\n");
		entry->type = TEST_EMPTY_BKCR;
		entry->offset = 0;
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


static INT32 MtATEPayloadInit(RTMP_ADAPTER *pAd, UCHAR *pPacket, UINT32 len, UINT32 band_idx)
{
	UINT32 policy = TESTMODE_GET_PARAM(pAd, band_idx, fixed_payload);
	UCHAR *payload = TESTMODE_GET_PARAM(pAd, band_idx, payload);
	UINT32 pl_len = TESTMODE_GET_PARAM(pAd, band_idx, pl_len);
	UINT32 pos = 0;

	MTWF_PRINT("%s: len:%d, band_idx:%u, len:%u, pl_len:%u, policy:%x\n",
		__func__, len, band_idx, len, pl_len, policy);

	if (policy == ATE_RANDOM_PAYLOAD) {
		for (pos = 0; pos < len; pos++)
			pPacket[pos] = RandomByte(pAd);

		return 0;
	}

	if (!payload)
		return NDIS_STATUS_FAILURE;

	MTWF_PRINT("%s: payload:%x\n", __func__, payload[0]);

	if (pl_len == 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Payload length can't be 0!!\n");
		return NDIS_STATUS_FAILURE;
	}

	if (policy == ATE_USER_PAYLOAD) {
		NdisZeroMemory(pPacket, len);
		NdisMoveMemory(pPacket, payload, pl_len);
	} else if (policy == ATE_FIXED_PAYLOAD) {
		for (pos = 0; pos < len; pos += pl_len)
			NdisMoveMemory(&pPacket[pos], payload, pl_len);
	}

	return 0;
}


#if OFF_CH_SCAN_SUPPORT
static INT32 mt_ate_off_ch_scan(RTMP_ADAPTER *pAd, ATE_OFF_CH_SCAN *param)
{
	INT32 ret = 0;
	EXT_CMD_OFF_CH_SCAN_CTRL_T ext_cmd_param;
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	UCHAR ctrl_band_idx = 0, ch = 0;
	UCHAR work_tx_strm_pth = 0, work_rx_strm_pth = 0, off_ch_idx = 0;
	UCHAR ch_ext_index = 0;
	UCHAR ch_ext_above[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};
	UCHAR ch_ext_below[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};
	UCHAR prim_ch[off_ch_ch_idx_num] = {0, 0};
	UCHAR bw[off_ch_ch_idx_num] = {0, 0};
	UCHAR cen_ch[off_ch_ch_idx_num] = {0, 0};

	if (!ate_ctrl)
		goto err0;

	if (!param)
		goto err0;

	ctrl_band_idx = ate_ctrl->control_band_idx;
	work_tx_strm_pth = TESTMODE_GET_PARAM(pAd, ctrl_band_idx, tx_strm_num);
	work_rx_strm_pth = TESTMODE_GET_PARAM(pAd, ctrl_band_idx, rx_strm_pth);
	prim_ch[off_ch_wrk_ch_idx] = TESTMODE_GET_PARAM(pAd, ctrl_band_idx, channel);
	bw[off_ch_wrk_ch_idx] = TESTMODE_GET_PARAM(pAd, ctrl_band_idx, bw);
	prim_ch[off_ch_mntr_ch_idx] = param->mntr_ch;
	bw[off_ch_mntr_ch_idx] = param->mntr_bw;

	for (off_ch_idx = 0; off_ch_idx < off_ch_ch_idx_num; off_ch_idx++) {
		ch = prim_ch[off_ch_idx];

		/* Initialize index */
		ch_ext_index = 0;

		switch (bw[off_ch_idx]) {
		case ATE_BAND_WIDTH_20:
			break;

		case ATE_BAND_WIDTH_40:
			while (ch_ext_above[ch_ext_index] != 0) {
				if (ch == ch_ext_above[ch_ext_index])
					ch = ch + 2;
				else if (ch == ch_ext_below[ch_ext_index])
					ch = ch - 2;

				ch_ext_index++;
			}
			break;

		case ATE_BAND_WIDTH_80:
		case ATE_BAND_WIDTH_8080:
			ch = vht_cent_ch_freq(ch, VHT_BW_80, CMD_CH_BAND_5G);
			break;

		case ATE_BAND_WIDTH_160:
			ch = vht_cent_ch_freq(ch, VHT_BW_160, CMD_CH_BAND_5G);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "off_ch_idx %d, BW is invalid %d\n",
				off_ch_idx, bw[off_ch_idx]);
			ret = NDIS_STATUS_FAILURE;
			goto err0;
		}

		cen_ch[off_ch_idx] = ch;
	}

	/* Initialize */
	os_zero_mem(&ext_cmd_param, sizeof(ext_cmd_param));

	/* Fill in ext_cmd_param */
	ext_cmd_param.mntr_prim_ch = param->mntr_ch;
	ext_cmd_param.mntr_cntrl_ch = cen_ch[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_bw = bw[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_tx_strm_pth = param->mntr_tx_rx_pth;
	ext_cmd_param.mntr_rx_strm_pth = param->mntr_tx_rx_pth;

	ext_cmd_param.work_prim_ch = prim_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_cntrl_ch = cen_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_bw = bw[off_ch_wrk_ch_idx];
	ext_cmd_param.work_tx_strm_pth = work_tx_strm_pth;
	ext_cmd_param.work_rx_strm_pth = work_rx_strm_pth;

	ext_cmd_param.dbdc_idx = param->dbdc_idx;
	ext_cmd_param.scan_mode = param->scan_mode;
	ext_cmd_param.is_aband = param->is_aband;
	/* TBD */
	ext_cmd_param.off_ch_scn_type = off_ch_scan_simple_rx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
		ext_cmd_param.mntr_prim_ch, ext_cmd_param.mntr_bw, ext_cmd_param.mntr_cntrl_ch);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
		ext_cmd_param.work_prim_ch, ext_cmd_param.work_bw, ext_cmd_param.work_cntrl_ch);

	ret = mt_cmd_off_ch_scan(pAd, &ext_cmd_param);
	return ret;

err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NULL entry ate_ctrl %p, param %p, or invalid BW %d\n",
			ate_ctrl, param, bw[off_ch_idx]);
	return ret;

}
#endif

static INT32 MT_ATESetTxPowerX(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 Channel = TESTMODE_GET_PARAM(pAd, TxPower.Dbdc_idx, channel);
	UINT32 Ch_Band = TESTMODE_GET_PARAM(pAd, TxPower.Dbdc_idx, ch_band);

	if (TxPower.Channel == 0)
		TxPower.Channel = Channel;

	TxPower.Band_idx = Ch_Band ? Ch_Band : TxPower.Band_idx;

	if (TxPower.Channel > 14)
		TxPower.Band_idx = 1; /* 5G band */

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel:%d Power:%x Ch_Band:%d Ant:%d\n",
		TxPower.Channel, TxPower.Power, TxPower.Band_idx, TxPower.Ant_idx);

	switch (TxPower.Ant_idx) {
	case 0:
		ATECtrl->TxPower0 = TxPower.Power;
		break;

	case 1:
		ATECtrl->TxPower1 = TxPower.Power;
		break;

	case 2:
		ATECtrl->TxPower2 = TxPower.Power;
		break;

	case 3:
		ATECtrl->TxPower3 = TxPower.Power;
		break;

	default:
		break;
	}

	Ret = MtCmdSetTxPowerCtrl(pAd, TxPower);
	return Ret;
}

static INT32 MT_ATESetRxUserIdx(RTMP_ADAPTER *pAd, UCHAR band_idx, UINT16 user_idx)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx: %d, user_idx: 0x%x\n", band_idx, user_idx);
	Ret = mt_cmd_set_rx_stat_user_idx(pAd, band_idx, user_idx);
	return Ret;
}

static INT32 MT_ATESetTxPower0(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 0;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32 MT_ATESetTxPower1(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 1;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32 MT_ATESetTxPower2(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 2;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32 MT_ATESetTxPower3(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 3;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32 MT_ATESetForceTxPower(RTMP_ADAPTER *pAd, INT8 cTxPower, UINT8 uctx_mode, UINT8 ucTxRate, UINT8 ucBW)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	/* update related Tx parameters */
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_mode, uctx_mode);
	TESTMODE_SET_PARAM(pAd, control_band_idx, mcs, ucTxRate);
	TESTMODE_SET_PARAM(pAd, control_band_idx, bw, ucBW);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band(%d), PhyMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
		control_band_idx, uctx_mode, ucTxRate, ucBW, cTxPower);

	/* firmware command for Force Tx Power Conrtrol */
	MtCmdSetForceTxPowerCtrl(pAd, control_band_idx, cTxPower, uctx_mode, ucTxRate, ucBW);

	return Ret;
}

#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATEDumpReCal(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT srcf)
#else
static INT32 MT_ATEDumpReCal(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	struct _ATE_LOG_RECAL re_cal = entry.log.re_cal;
	INT32 ret = 0;
#ifdef LOGDUMP_TO_FILE
	INT len = 7 + 2 * 3 + 8 * 3 + 1;
	CHAR msg[len];
	os_zero_mem(msg, len);
#endif
	/* MTWF_DBG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
	/* "CalType:%x ", re_cal.cal_type); */
	MTWF_PRINT("[Recal][%08x][%08x]%08x\n", re_cal.cal_type, re_cal.cr_addr, re_cal.cr_val);
	/* MTWF_DBG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
	/* "%08x\n", re_cal.cr_val); */
#ifdef LOGDUMP_TO_FILE
	sprintf(msg, "[Recal][%08x][%08x]%08x\n", re_cal.cal_type, re_cal.cr_addr, re_cal.cr_val);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Length:%d %s\n", strlen(msg), msg);
	MT_ATEWriteFd(msg, srcf);
#endif
	return ret;
}

static INT32 MT_ATEInsertReCal(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	struct _ATE_LOG_RECAL *re_cal = NULL;
	INT32 ret = 0;

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RE_CAL;
	entry->un_dumped = TRUE;
	re_cal = &entry->log.re_cal;
	NdisMoveMemory((UINT8 *)re_cal, data,
				   sizeof(*re_cal));
	return ret;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NULL entry %p, data %p\n",
		entry, data);
	return NDIS_STATUS_FAILURE;
}


static INT32 MT_ATEInsertRDD(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	INT ret = 0;

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RDD;
	entry->un_dumped = TRUE;

	if (len > sizeof(entry->log.rdd))
		len = sizeof(entry->log.rdd);

	NdisMoveMemory((UCHAR *)&entry->log.rdd, data, len);
	return ret;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"NULL entry %p, data %p\n",
		entry, data);
	return -1;
}


static INT32 MT_ATEInsertRXV(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(data + 8);
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(data + 12);
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(data + 16);
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(data + 20);
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(data + 24);
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(data + 28);
	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(data + 32);
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(data + 36);
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(data + 40);

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RXV;
	entry->un_dumped = TRUE;

	if (RXV1_1ST_CYCLE)
		entry->log.rxv.rxv1_1st = *RXV1_1ST_CYCLE;

	if (RXV1_2ND_CYCLE)
		entry->log.rxv.rxv1_2nd = *RXV1_2ND_CYCLE;

	if (RXV1_3TH_CYCLE)
		entry->log.rxv.rxv1_3rd = *RXV1_3TH_CYCLE;

	if (RXV1_4TH_CYCLE)
		entry->log.rxv.rxv1_4th = *RXV1_4TH_CYCLE;

	if (RXV1_5TH_CYCLE)
		entry->log.rxv.rxv1_5th = *RXV1_5TH_CYCLE;

	if (RXV1_6TH_CYCLE)
		entry->log.rxv.rxv1_6th = *RXV1_6TH_CYCLE;

	if (RXV2_1ST_CYCLE)
		entry->log.rxv.rxv2_1st = *RXV2_1ST_CYCLE;

	if (RXV2_2ND_CYCLE)
		entry->log.rxv.rxv2_2nd = *RXV2_2ND_CYCLE;

	if (RXV2_3TH_CYCLE)
		entry->log.rxv.rxv2_3rd = *RXV2_3TH_CYCLE;

	return 0;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"NULL entry %p, data %p\n", entry, data);
	return -1;
}


#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATEDumpRXV(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT srcf)
#else
static INT32 MT_ATEDumpRXV(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	INT32 ret = 0;
	struct _ATE_RXV_LOG log = entry.log.rxv;
	MTWF_PRINT("%%[RXV DUMP START][%d]\n", idx);
	MTWF_PRINT("[RXVD1]%08x\n", *((UINT32 *)&log.rxv1_1st));
	MTWF_PRINT("[RXVD2]%08x\n", *((UINT32 *)&log.rxv1_2nd));
	MTWF_PRINT("[RXVD3]%08x\n", *((UINT32 *)&log.rxv1_3rd));
	MTWF_PRINT("[RXVD4]%08x\n", *((UINT32 *)&log.rxv1_4th));
	MTWF_PRINT("[RXVD5]%08x\n", *((UINT32 *)&log.rxv1_5th));
	MTWF_PRINT("[RXVD6]%08x\n", *((UINT32 *)&log.rxv1_6th));
	MTWF_PRINT("[RXVD7]%08x\n", *((UINT32 *)&log.rxv2_1st));
	MTWF_PRINT("[RXVD8]%08x\n", *((UINT32 *)&log.rxv2_2nd));
	MTWF_PRINT("[RXVD9]%08x\n", *((UINT32 *)&log.rxv2_3rd));
	MTWF_PRINT("[RXV DUMP END]\n");
	return ret;
}

#ifdef LOGDUMP_TO_FILE
static INT MT_ATEWriteFd(RTMP_STRING *log, RTMP_OS_FD_EXT srcf)
{
	INT ret = 0;
	INT len = strlen(log);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Write len %d\n", len);
	ret = os_file_write(srcf, log, len);
	return ret;
}


static RTMP_OS_FD_EXT MT_ATEGetFileFd(UINT32 log_type, INT idx)
{
	RTMP_STRING src[64];
	RTMP_OS_FD_EXT srcf;

	switch (log_type) {
	case ATE_LOG_RXV:
		sprintf(src, "RXVDump_v%d.txt", idx);
		break;

	case ATE_LOG_RDD:
		sprintf(src, "RDDDump_v%d.txt", idx);
		break;

	case ATE_LOG_RE_CAL:
		sprintf(src, "RECALDump_v%08x.txt", (UINT32)idx);
		break;

	default:
		srcf.Status = NDIS_STATUS_FAILURE;
		goto err0;
	}

	/* srcf = os_file_open(src, O_WRONLY|O_CREAT|O_TRUNC, 0); */
	srcf = os_file_open(src, O_WRONLY | O_CREAT | O_APPEND, 0);
	return srcf;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown log type %08x\n", log_type);
	return srcf;
}


static INT32 MT_ATEReleaseLogFd(RTMP_OS_FD_EXT *srcf)
{
	UCHAR ret = 0;

	if (os_file_close(*srcf) != 0)
		goto err0;

	return ret;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error closing file\n");
	return NDIS_STATUS_FAILURE;
}
#endif


#if defined(COMPOS_TESTMODE_WIN)
static INT32 MT_ATEDumpRXVToFile(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 idx)
{
	/* todo: check this function work properly under all the case. */
	UINT32 copyIndex = 0;
	KIRQL oldIrql;
	UCHAR tempBuffer[512];
	struct _ATE_RXV_LOG log;
	struct _ATE_LOG_DUMP_ENTRY *entry;
	UCHAR *writeBuffer = (UCHAR *)WINAllocateMemory(CALIBRATION_BUFFER_SIZE);
	RTMPMoveMemory(writeBuffer, "[LOG DUMP START]\n", strlen("[LOG DUMP START]\n"));
	copyIndex += strlen("[LOG DUMP START]\n");

	do {
		if (log_cb->entry[idx].un_dumped) {
			entry = &log_cb->entry[idx];
			log = entry->log.rxv;
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer),
							   "[%d]", idx);
			RTMPMoveMemory(&writeBuffer[copyIndex],
						   tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RTMPMoveMemory(&writeBuffer[copyIndex], "[RXV DUMP START]\r\n", strlen("[RXV DUMP START]\r\n"));
			copyIndex += strlen("[RXV DUMP START]\r\n");
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD1]%08x\r\n", *((UINT32 *)&log.rxv1_1st));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD2]%08x\r\n", *((UINT32 *)&log.rxv1_2nd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD3]%08x\r\n", *((UINT32 *)&log.rxv1_3rd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD4]%08x\r\n", *((UINT32 *)&log.rxv1_4th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD5]%08x\r\n", *((UINT32 *)&log.rxv1_5th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD6]%08x\r\n", *((UINT32 *)&log.rxv1_6th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD7]%08x\r\n", *((UINT32 *)&log.rxv2_1st));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD8]%08x\r\n", *((UINT32 *)&log.rxv2_2nd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD9]%08x\r\n", *((UINT32 *)&log.rxv2_3rd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RTMPMoveMemory(&writeBuffer[copyIndex], "[RXV DUMP END]\r\n", strlen("[RXV DUMP END]\r\n"));
			copyIndex += strlen("[RXV DUMP END]\r\n");
			log_cb->entry[idx].un_dumped = FALSE;
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != log_cb->idx);

	RTMPMoveMemory(&writeBuffer[copyIndex], "[LOG DUMP END]\r\n", strlen("[LOG DUMP END]\r\n"));
	copyIndex += strlen("[LOG DUMP END]\r\n");

	if (pAd->ReCalibrationSize + copyIndex <
		CALIBRATION_BUFFER_SIZE) {
		KeAcquireSpinLock(&pAd->RxVectorDumpLock,
						  &oldIrql);
		RTMPMoveMemory(
			&pAd->RxVectorDumpBuffer[pAd->RxVectorDumpSize],
			writeBuffer, copyIndex);
		pAd->RxVectorDumpSize += copyIndex;
		KeReleaseSpinLock(&pAd->RxVectorDumpLock,
						  oldIrql);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxVectorDumpSize = 0x%x, Dump size = 0x%x\n",
				  pAd->RxVectorDumpSize, copyIndex);

		if (copyIndex > 0 && pAd->bIsCalDumpThreadRunning) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s\n", pAd->ReCalibrationBuffer);
			KeSetEvent(&pAd->WriteEvent, 0, FALSE);
		} else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Calibration buffer 0x%x + dump size 0x%x over Limit 0x%x\n",
					  pAd->RxVectorDumpSize, copyIndex, CALIBRATION_BUFFER_SIZE);
	}

	WINFreeMemory(writeBuffer);
	writeBuffer = NULL;
}


#endif /*#if defined(COMPOS_TESTMODE_WIN)*/


INT32 MT_ATEInsertLog(RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;
	INT idx = 0;
	INT logcb_idx = 0;
	UINT32 is_dumping = 0;
	INT32 (*insert_func)(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len) = NULL;

	switch (log_type) {
	case fATE_LOG_RXV:
		insert_func = MT_ATEInsertRXV;
		logcb_idx = ATE_LOG_RXV - 1;
		break;

	case fATE_LOG_RDD:
		insert_func = MT_ATEInsertRDD;
		logcb_idx = ATE_LOG_RDD - 1;
		break;

	case fATE_LOG_RE_CAL:
		insert_func = MT_ATEInsertReCal;
		logcb_idx = ATE_LOG_RE_CAL - 1;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown log type %08x\n", log_type);
		break;
	}

	if (!insert_func)
		goto err1;

	log_cb = &ATECtrl->log_dump[logcb_idx];
	idx = log_cb->idx;
	OS_SPIN_LOCK(&log_cb->lock);
	is_dumping = log_cb->is_dumping;
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (is_dumping)
		goto err1;

	if ((log_cb->idx + 1) == log_cb->len) {
		if (!log_cb->overwritable)
			goto err0;
		else
			log_cb->is_overwritten = TRUE;
	}

	OS_SPIN_LOCK(&log_cb->lock);

	/* The result of pointer arithmetic log_cb->entry + idx is never null
	 * The condition !(log_cb->entry + idx) cannot be true
	 * so check if log_cb->entry is null that infer log_cb->entry[idx] has data */
	if (!log_cb->entry) {
		OS_SPIN_UNLOCK(&log_cb->lock);
		goto err0;
	}

	ret = insert_func(&log_cb->entry[idx], log, len);
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (ret)
		goto err0;

	INC_RING_INDEX(log_cb->idx, log_cb->len);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
	"idx:%d, log_cb->idx:%d, log_type:%08x\n",
			  idx, log_cb->idx, log_type);
	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "[WARN]: idx:%x, overwritable:%x, log_type:%08x\n",
			  idx, (log_cb) ? log_cb->overwritable:0xff,
			  log_type);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Log dumping\n");
	return -NDIS_STATUS_RESOURCES;
}


INT32 MT_ATEDumpLog(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 log_type)
{
	INT32 ret = 0;
	INT snprintf_ret;
	INT idx = 0;
	UINT32 u4BufferCounter = 0;
#ifdef LOGDUMP_TO_FILE
	INT32 (*dump_func)(struct _ATE_LOG_DUMP_ENTRY, INT idx, RTMP_OS_FD_EXT fd) = NULL;
#else
	INT32 (*dump_func)(struct _ATE_LOG_DUMP_ENTRY, INT idx) = NULL;
#endif
	INT debug_lvl = DebugLevel;
	CHAR Log_type[64];
#ifdef LOGDUMP_TO_FILE
	INT len = 5 + 2 * 3 + 5 + 1;
	CHAR msg[len];
	os_zero_mem(msg, len);
#endif

	if (!log_cb->entry)
		goto err0;

	/* For QAtool log buffer limitation. We should record the current index for next function called.  */
	if (pAd->fgQAtoolBatchDumpSupport)
		idx = pAd->u2LogEntryIdx;

	if (log_cb->is_overwritten)
		idx = log_cb->idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "idx:%d, log_type:%08x, log_cb->idx:%d\n", idx, log_type, log_cb->idx);

	switch (log_type) {
	case ATE_LOG_RXV:
		dump_func = MT_ATEDumpRXV;
		snprintf_ret = snprintf(Log_type, sizeof(Log_type), "%s", "LOG");
		if (os_snprintf_error(sizeof(Log_type), snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Log_type snprintf error!\n");
			return FALSE;
		}
		break;

	case ATE_LOG_RDD:
		dump_func = MT_ATERDDParseResult;
		snprintf_ret = snprintf(Log_type, sizeof(Log_type), "%s", "RDD");
		if (os_snprintf_error(sizeof(Log_type), snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Log_type snprintf error!\n");
			return FALSE;
		}
		break;

	case ATE_LOG_RE_CAL:
		dump_func = MT_ATEDumpReCal;
		snprintf_ret = snprintf(Log_type, sizeof(Log_type), "%s", "RECAL");
		if (os_snprintf_error(sizeof(Log_type), snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Log_type snprintf error!\n");
			return FALSE;
		}
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown log type %08x\n", log_type);
		break;
	}

	if (!dump_func)
		goto err0;

#ifdef LOGDUMP_TO_FILE

	if (log_type == ATE_LOG_RE_CAL)
		log_cb->fd = MT_ATEGetFileFd(log_type, log_cb->recal_curr_type);
	else
		log_cb->fd = MT_ATEGetFileFd(log_type, log_cb->file_idx);

	if (log_cb->fd.Status)
		goto err1;

	if (log_type == ATE_LOG_RE_CAL) {
		sprintf(msg, "[Recal][%08x][START]\n", log_cb->recal_curr_type);
		MT_ATEWriteFd(msg, log_cb->fd);
	}

#endif
	DebugLevel = DBG_LVL_OFF;
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = TRUE;
	OS_SPIN_UNLOCK(&log_cb->lock);
#if defined(COMPOS_TESTMODE_WIN)
	/* dump RX vector to file */
	CreateThread(pAd);
	MT_ATEDumpRXVToFile(pAd, log_cb, idx);
#else
	MTWF_PRINT("[%s DUMP START]\n", Log_type);
	pAd->fgDumpStart = 1;

	do {
		if (log_cb->entry[idx].un_dumped) {
			/* MTWF_PRINT("[%d]", idx); */
#ifdef LOGDUMP_TO_FILE
			dump_func(log_cb->entry[idx], idx, log_cb->fd);
#else
			dump_func(log_cb->entry[idx], idx);
#endif
			log_cb->entry[idx].un_dumped = FALSE;
			u4BufferCounter++;
		}

		/* The size of per entry is 38 bytes and for QAtool log buffer limitation. */
		if ((pAd->fgQAtoolBatchDumpSupport) &&
			(u4BufferCounter >= (1 << (CONFIG_LOG_BUF_SHIFT - 1)) / 38)) {
			pAd->u2LogEntryIdx = idx;
			break;
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != log_cb->idx);

#ifdef LOGDUMP_TO_FILE

	if (log_type == ATE_LOG_RE_CAL) {
		sprintf(msg, "[Recal][%08x][END]\n", log_cb->recal_curr_type);
		MT_ATEWriteFd(msg, log_cb->fd);
	}

#endif

	if ((idx == log_cb->idx) && (pAd->fgDumpStart)) {
		MTWF_PRINT("[%s DUMP END]\n", Log_type);
		pAd->fgDumpStart = 0;
	}

#endif
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = FALSE;
	OS_SPIN_UNLOCK(&log_cb->lock);
	DebugLevel = debug_lvl;
#ifdef LOGDUMP_TO_FILE
	MT_ATEReleaseLogFd(&log_cb->fd);
	log_cb->file_idx++;
#endif
	return ret;
#ifdef LOGDUMP_TO_FILE
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error opening log file\n");
#endif
err0:
	return -1;
}


static INT32 MT_ATEInitLogCB(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 size, UCHAR overwrite)
{
	INT32 ret = 0;

	if (!log_cb->entry) {
		NdisZeroMemory(log_cb, sizeof(*log_cb));
		ret = os_alloc_mem(pAd, (PUCHAR *)&log_cb->entry, size * sizeof(struct _ATE_LOG_DUMP_ENTRY));

		if (ret)
			goto err0;

		os_zero_mem(log_cb->entry, size * sizeof(struct _ATE_LOG_DUMP_ENTRY));
		log_cb->len = size;
		NdisAllocateSpinLock(pAd, &log_cb->lock);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-v4, init log cb size %u, log_cb->len:%u\n", size, log_cb->len);
	}

	log_cb->overwritable = overwrite;
	log_cb->is_overwritten = FALSE;
	log_cb->idx = 0;
#ifdef LOGDUMP_TO_FILE
	log_cb->file_idx = 0;
#endif
	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Alcated memory fail! size %u\n", size);
	return ret;
}

static INT32 MT_ATEReleaseLogDump(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;
	INT i = 0;
	ate_ctrl->en_log = 0;

	for (i = 0; i < (ATE_LOG_TYPE_NUM - 1); i++) {
		log_cb = &ate_ctrl->log_dump[i];

		if (log_cb->entry) {
			os_free_mem(log_cb->entry);
			NdisFreeSpinLock(&log_cb->lock);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "release log cb type %d\n", i + 1);
		}
	}

	return ret;
}


static VOID MT_ATEAirTimeOnOff(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable)
{
	UINT32 Value = 0;

	if (!Enable) {
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_AIRTIME0, &Value);
		Value = Value & (~RX_AIRTIME_EN);
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_AIRTIME0, Value);
	}
}


static INT32 MT_ATELogOnOff(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 size)
{
	INT ret = 0;
	UINT32 mask = 0;
	UCHAR overwrite = TRUE;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;

	switch (type) {
	case ATE_LOG_RXV:
		mask = fATE_LOG_RXV;
		/* Disable RX airtime function to avoid affecting RXV2 Cycle3 */
		/* HW design of RXV2 Cycle3 will be shared for RXV debug function and RX airtime funciotn. */
		MT_ATEAirTimeOnOff(pAd, 0);
		break;

	case ATE_LOG_RDD:
		overwrite = FALSE;
		mask = fATE_LOG_RDD;
		break;

	case ATE_LOG_RE_CAL:
		/* size = ATE_RECAL_LOG_SIZE; */
		mask = fATE_LOG_RE_CAL;
		break;

	case ATE_LOG_RXINFO:
		mask = fATE_LOG_RXINFO;
		break;

	case ATE_LOG_TXDUMP:
		mask = fATE_LOG_TXDUMP;
		break;

	case ATE_LOG_TEST:
		mask = fATE_LOG_TEST;
		break;

	case ATE_LOG_TXSSHOW:
		mask = fATE_LOG_TXSSHOW;
		break;

	default:
		goto err0;
	}

	if (type < ATE_LOG_TYPE_NUM)
		log_cb = &ATECtrl->log_dump[type - 1];

	if (on_off == ATE_LOG_ON) {
		if (log_cb)
			ret = MT_ATEInitLogCB(pAd, log_cb, size, overwrite);

		if (ret)
			goto err1;

		ATECtrl->en_log |= mask;

		if (pAd->fgQAtoolBatchDumpSupport)
			pAd->u2LogEntryIdx = 0;
	} else if (on_off == ATE_LOG_OFF) {
		ATECtrl->en_log &= ~mask;

		if (pAd->fgQAtoolBatchDumpSupport)
			pAd->u2LogEntryIdx = 0;
	} else if (on_off == ATE_LOG_DUMP) {
		if (log_cb)
			ret = MT_ATEDumpLog(pAd, log_cb, type);

		if (ret)
			goto err1;
	} else
		goto err2;

	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "log type %d not supported\n", type);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "log type %d init logCB fail\n", type);
err2:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "log ctrl %d not supported\n", on_off);
	return -1;
}

static INT32 MT_ATESetICapStart(
				RTMP_ADAPTER *pAd,
				PUINT8 pData)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	RBIST_CAP_START_T *prICapInfo = (RBIST_CAP_START_T *)pData;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#endif/* INTERNAL_CAPTURE_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapStart != NULL)
		Ret = ops->ICapStart(pAd, (UINT8 *)prICapInfo);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "The function is not hooked !!\n");
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


static INT32 MT_ATEGetICapStatus(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (ops->ICapStatus != NULL) {
		Ret = ops->ICapStatus(pAd);

		if (IS_MT7615(pAd)) {
			UINT32 StartAddr1, StartAddr2, StartAddr3, EndAddr;
			UINT32 StopAddr, Wrap;

			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "() wait cmd timeout!\n");
			}
			StartAddr1 = ate_ctrl->icap_info.u4StartAddr1;
			StartAddr2 = ate_ctrl->icap_info.u4StartAddr2;
			StartAddr3 = ate_ctrl->icap_info.u4StartAddr3;
			EndAddr = ate_ctrl->icap_info.u4EndAddr;
			StopAddr = ate_ctrl->icap_info.u4StopAddr;
			Wrap = ate_ctrl->icap_info.u4Wrap;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "StartAddr1:%02x StartAddr2:%02x StartAddr3:%02x EndAddr:%02x StopAddr:%02x Wrap:%02x\n",
					StartAddr1, StartAddr2, StartAddr3, EndAddr, StopAddr, Wrap);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "The function is not hooked !!\n");
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


static INT32 MT_ATEGetICapIQData(RTMP_ADAPTER
								 *pAd,
								 PINT32 pData,
								 PINT32 pDataLen,
								 UINT32 IQ_Type,
								 UINT32 WF_Num)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (ops->ICapGetIQData != NULL)
		Ret = ops->ICapGetIQData(pAd, pData, pDataLen, IQ_Type, WF_Num);
	else if (ops->ICapCmdSolicitRawDataProc != NULL)
		Ret = ops->ICapCmdSolicitRawDataProc(pAd, pData, pDataLen, IQ_Type, WF_Num);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "The function is not hooked !!\n");
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


#if !defined(COMPOS_TESTMODE_WIN)	/* 1todo too many OS private function */
struct wifi_dev_ops ate_wdev_ops = {
	.open = ate_inf_open,
	.close = ate_inf_close,
	.send_mlme_pkt = ate_enqueue_mlme_pkt,
	.tx_pkt_handle = ate_tx_pkt_handle,
	.ate_tx = mt_ate_tx,
};


INT ate_enqueue_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	INT ret;
	struct qm_ops *ops = pAd->qm_ops;

	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);

	ret = ops->enq_mgmtq_pkt(pAd, wdev, pkt);

	return ret;
}



INT ate_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	struct wifi_dev_ops *ops = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (!wdev) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ops = wdev->wdev_ops;
	ret = ops->ate_tx(pAd, wdev, pTxBlk);

	return ret;
}


/*
*
*/
INT ate_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open fail!!!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ATE inf up for ra_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx);

	MlmeRadioOn(pAd, wdev);

	wdev->bAllowBeaconing = FALSE;


	return TRUE;
}


/*
*
*/
INT ate_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (ad == NULL)
		return FALSE;

	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "close fail!!!\n");
		return FALSE;
	}


	return TRUE;
}


INT ate_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	USHORT bw_winsiz = 0, tid_idx = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	INT ret = -1;

	/* follow normal mode to create wtbl entry */

	if (entry == NULL)
		entry = MacTableInsertEntry(ad, TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), addr1[0]), wdev, ENTRY_ATE, OPMODE_ATE, TRUE);

	if (entry) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
		struct phy_params *phy_info = &entry->phy_param;

		if (phy_info->phy_mode > MODE_VHT)
			entry->MaxHTPhyMode.field.MODE = MODE_VHT;
		else
			entry->MaxHTPhyMode.field.MODE = phy_info->phy_mode;

		if (phy_info->phy_mode > MODE_OFDM) {
			entry->MaxRAmpduFactor = cap->ppdu.ht_max_ampdu_len_exp;
			entry->cap.ampdu.max_ht_ampdu_len_exp = cap->ppdu.ht_max_ampdu_len_exp;
		}
		if (phy_info->phy_mode > MODE_HTGREENFIELD) {
			entry->MaxRAmpduFactor = cap->ppdu.vht_max_ampdu_len_exp;
			entry->cap.ampdu.max_mpdu_len = cap->ppdu.max_mpdu_len;
			entry->cap.ampdu.max_vht_ampdu_len_exp = cap->ppdu.vht_max_ampdu_len_exp;
		}
#if defined(DOT11_HE_AX)
		if (phy_info->phy_mode > MODE_VHT) {
			entry->cap.modes |= (HE_24G_SUPPORT | HE_5G_SUPPORT);
			entry->cap.he_mac_cap |= HE_AMSDU_IN_ACK_EN_AMPDU;
			entry->cap.ampdu.max_he_ampdu_len_exp = cap->ppdu.he_max_ampdu_len_exp;
		}
#endif
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_WMM_CAPABLE);
		entry->cap.ch_bw.he_ch_width = BW_80;

		/*generic connection action*/
		if (wifi_sys_conn_act(wdev, entry) != TRUE) {
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "connect action fail!\n");
		}

		/* for fix rate inband cmd */
#if defined(DOT11_VHT_AC)
	chip_ra_init(ad, entry);
#endif

		if (phy_info->phy_mode > MODE_VHT)
			bw_winsiz = cap->ppdu.he_tx_ba_wsize;
		else
			bw_winsiz = cap->ppdu.non_he_tx_ba_wsize;

		/* VOID AsicUpdateBASession(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid,
				UINT16 sn, UINT16 basize, BOOLEAN isAdd, INT ses_type) */
		for (tid_idx = 0; tid_idx < 8 ; tid_idx++)
			AsicUpdateBASession(ad, entry->wcid, tid_idx, 0, bw_winsiz, TRUE, BA_SESSION_ORI, 1);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE) {
			CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

			entry->bAutoTxRateSwitch = FALSE;
			NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
			rRaParam.FixedRateCfg.MODE = phy_info->phy_mode;
			if (phy_info->phy_mode == MODE_HE_MU || phy_info->phy_mode == MODE_VHT_MIMO) {
				/* work-around to prevent TX CCK while 5GHz band */
				rRaParam.FixedRateCfg.MODE = MODE_OFDM;
				phy_info->rate = 7;
			}
			rRaParam.FixedRateCfg.STBC = phy_info->stbc;
			if (phy_info->phy_mode < MODE_HE_SU) {
				rRaParam.FixedRateCfg.ShortGI = (phy_info->gi_type) ? BIT(phy_info->bw) : 0;
			}
#if defined(DOT11_HE_AX)
			else {
				switch (phy_info->bw) {
				case BW_40:
					rRaParam.FixedRateCfg.ShortGI = (phy_info->gi_type << 2);
					rRaParam.FixedRateCfg.he_ltf = (phy_info->ltf_type << 2);
					break;
				case BW_80:
					rRaParam.FixedRateCfg.ShortGI = (phy_info->gi_type << 4);
					rRaParam.FixedRateCfg.he_ltf = (phy_info->ltf_type << 4);
					break;
				case BW_160:
					rRaParam.FixedRateCfg.ShortGI = (phy_info->gi_type << 6);
					rRaParam.FixedRateCfg.he_ltf = (phy_info->ltf_type << 6);
					break;
				default:
					rRaParam.FixedRateCfg.ShortGI = phy_info->gi_type;
					rRaParam.FixedRateCfg.he_ltf = phy_info->ltf_type;
				}
			}
#endif /* DOT11_HE_AX */
			rRaParam.FixedRateCfg.BW = phy_info->bw;
			if (phy_info->ldpc) {
				switch (phy_info->phy_mode) {
				case MODE_HTMIX:
				case MODE_HTGREENFIELD:
					rRaParam.FixedRateCfg.ldpc = 1;
					break;
				case MODE_VHT:
					rRaParam.FixedRateCfg.ldpc = 2;
					break;
#if defined(DOT11_HE_AX)
				case MODE_HE_SU:
				case MODE_HE_EXT_SU:
				case MODE_HE_TRIG:
				case MODE_HE_MU:
					rRaParam.FixedRateCfg.ldpc = 4;
					break;
#endif /* DOT11_HE_AX */
				default:
					rRaParam.FixedRateCfg.ldpc = 0;	/* should not happen */
				}
			}
			rRaParam.FixedRateCfg.MCS = phy_info->rate;
			if (phy_info->dcm)
				rRaParam.FixedRateCfg.MCS |= BIT(4);
			if (phy_info->su_ext_tone)
				rRaParam.FixedRateCfg.MCS |= BIT(5);
			rRaParam.FixedRateCfg.VhtNss = phy_info->vht_nss;
			rRaParam.ucShortPreamble = TESTMODE_GET_PARAM(ad, HcGetBandByWdev(wdev), preamble);
			rRaParam.u4Field = RA_PARAM_FIXED_RATE;
			RAParamUpdate(ad, entry, &rRaParam);

			ret = 0;
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "fgRateAdaptFWOffload Not supported\n");
			goto error_out;
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MacTableInsertEntry failed\n");
		goto error_out;
	}

error_out:
	return ret;
}


INT MT_ATERxDoneHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 band_idx;
	UINT32 bn0_cr_addr = RMAC_CHFREQ0;
#if defined(DBDC_MODE)
	UINT32 bn1_cr_addr = RMAC_CHFREQ1;
#endif

		INC_COUNTER64(pAd->WlanCounters[0].ReceivedFragmentCount);

	{
		UINT32 chfreq0 = 0, chfreq1 = 0;

		MAC_IO_READ32(pAd->hdev_ctrl, bn0_cr_addr, &chfreq0);
#if defined(DBDC_MODE)
		MAC_IO_READ32(pAd->hdev_ctrl, bn1_cr_addr, &chfreq1);
#endif /* defined(MT7615) || defined(MT7626) */

		ATEOp->SampleRssi(pAd, pRxBlk);

		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			band_idx = pRxBlk->band;
		} else {
		/* RX packet counter calculate by chfreq of RXD */
			if (pRxBlk->channel_freq == chfreq0) {
				band_idx = TESTMODE_BAND0;
			}
#ifdef DBDC_MODE
			else {
			band_idx = TESTMODE_BAND1;
			}
#endif /* DBDC_MODE */
		}

		if (band_idx > -1) {
			ATECtrl->rx_stat.RxTotalCnt[band_idx]++;
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Wrong chfreq!!\n""\tRXD.ch_freq: %x, chfreq0: %x, chfreq1: %x\n",
				pRxBlk->channel_freq, chfreq0, chfreq1);
	}
	/* LoopBack_Rx(pAd, pRxBlk->MPDUtotalByteCnt, pRxBlk->pRxPacket); */
	return TRUE;
}


INT8 mt_ate_release_wdev(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);

	if (!wdev)
		goto err;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"wdev_idx=%d\n", wdev->wdev_idx);

	if (wifi_sys_linkdown(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "linkdown fail!\n");
		goto err;
	}

	if (wdev_do_close(wdev) != TRUE) {
		goto err;
	}

	wdev_deinit(pAd, wdev);

	wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[1]);

	if (!wdev)
			goto err;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"wdev_idx=%d\n", ATECtrl->wdev_idx);

	if (wifi_sys_linkdown(wdev) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "linkdown fail!\n");
		goto err;
	}

	if (wdev_do_close(wdev) != TRUE) {
		goto err;
	}

	wdev_deinit(pAd, wdev);

	return ret;

err:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Invalid wdev\n");

	return -1;
}


INT8 mt_ate_init_wdev(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev *wdev = NULL;
	UCHAR channel, *own_mac_addr = NULL, *bssid = NULL;
	INT map_idx = 0;

	channel = TESTMODE_GET_PARAM(pAd, band_idx, channel);
	wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev is NULL\n");
		return -1;
	}

	if (!wdev_init(pAd, wdev, WDEV_TYPE_ATE_STA, pAd->wdev_list[band_idx]->if_dev, band_idx, NULL, (void *)pAd)) {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Assign wdev idx for ATE failed, free net device!\n");
		goto err;
	}

	TESTMODE_SET_PARAM(pAd, band_idx, wdev_idx, wdev->wdev_idx);

	if (IS_AXE(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		ate_wdev_ops.ate_tx = mt_ate_tx_v2;
		ate_wdev_ops.conn_act = ate_conn_act;
		ate_wdev_ops.disconn_act = wifi_sys_disconn_act;
	}
	wdev_ops_register(wdev, WDEV_TYPE_ATE_STA, &ate_wdev_ops, 0);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wdev_idx=%d, channel=%d\n", wdev->wdev_idx, channel);

	wdev->channel = channel;
	wdev->EdcaIdx = band_idx*2;
	wdev->bWmmCapable = TRUE;

	if (wdev->channel > 14)
		wdev->PhyMode = TEST_WMODE_CAP_5G;
	else
		wdev->PhyMode = TEST_WMODE_CAP_24G;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		own_mac_addr = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr3[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		own_mac_addr = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr2[0]);
#endif
	if (own_mac_addr != NULL)
		memcpy(wdev->if_addr, own_mac_addr, MAC_ADDR_LEN);

	if (!wdev_do_open(wdev)) {
		ret = -1;
		goto err;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		bssid = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr2[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		bssid = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr1[0]);
#endif
	if (bssid != NULL)
		memcpy(wdev->bssid, bssid, MAC_ADDR_LEN);

	wifi_sys_linkup(wdev, NULL);

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[1]);
	if (!wdev)
		goto err;

	if (!wdev_init(pAd, wdev, WDEV_TYPE_ATE_AP, pAd->wdev_list[band_idx]->if_dev, band_idx, NULL, (void *)pAd)) {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Assign wdev idx for ATE failed, free net device!\n");
		goto err;
	}

	TESTMODE_SET_PARAM(pAd, band_idx, wdev_idx, wdev->wdev_idx);

	if (IS_AXE(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		ate_wdev_ops.ate_tx = mt_ate_tx_v2;
		ate_wdev_ops.conn_act = ate_conn_act;
		ate_wdev_ops.disconn_act = wifi_sys_disconn_act;
	}
	wdev_ops_register(wdev, WDEV_TYPE_ATE_AP, &ate_wdev_ops, 0);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wdev_idx=%d, channel=%d\n", wdev->wdev_idx, channel);

	wdev->channel = channel;
	wdev->EdcaIdx = band_idx*2+1;
	wdev->bWmmCapable = TRUE;

	if (wdev->channel > 14)
		wdev->PhyMode = TEST_WMODE_CAP_5G;
	else
		wdev->PhyMode = TEST_WMODE_CAP_24G;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		own_mac_addr = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr3[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		own_mac_addr = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr2[0]);
#endif
	if (own_mac_addr != NULL)
		memcpy(wdev->if_addr, own_mac_addr, MAC_ADDR_LEN);

	if (!wdev_do_open(wdev)) {
		ret = -1;
		goto err;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		bssid = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr2[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		bssid = (UCHAR *)TESTMODE_GET_PARAM(pAd, band_idx, addr1[0]);
#endif
	if (bssid != NULL)
		memcpy(wdev->bssid, bssid, MAC_ADDR_LEN);

#if defined(DOT11_HE_AX)
	wlan_operate_set_he_bss_color(wdev, band_idx+1, FALSE);
#endif
	wifi_sys_linkup(wdev, NULL);
#endif	/* MT7915 || MT7986  || MT7916 || MT7981 */

	if (IS_MT7626(pAd)) {
		/* modify spe_idx_map table for 2ss support */
		if (2 == wlan_config_get_tx_stream(wdev)) {
			for (map_idx = 0; map_idx < ARRAY_SIZE(ant_to_spe_idx_map); map_idx++) {
				if (0x3 >= ant_to_spe_idx_map[map_idx].ant_sel)
					ant_to_spe_idx_map[map_idx].spe_idx = 24;
			}
		}
	}

err:
	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Cannot get wdev by idx:%d\n", wdev->wdev_idx);

	return ret;
}

#ifdef DBDC_MODE
static INT32 MT_ATEReleaseBandInfo(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	INT32 idx = band_idx - 1;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	if (ATECtrl->band_ext[idx].test_pkt) {
		os_free_mem(ATECtrl->band_ext[idx].test_pkt);
		ATECtrl->band_ext[idx].test_pkt = NULL;
	}
	return ret;
}


static INT32 MT_ATEInitBandInfo(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	UINT16 sta_idx = 0;
	INT32 idx = band_idx - 1;
	UCHAR *payload = NULL;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(pAd, band_idx, tx_wait);
	CHAR addr[MAC_ADDR_LEN] = {0x00, 0x11, 0x22, 0xBA, 0x2D, 0x11};

	if (ATECtrl->band_ext[idx].test_pkt)
		os_free_mem(ATECtrl->band_ext[idx].test_pkt);

	ret = os_alloc_mem(pAd, (PUCHAR *)&ATECtrl->band_ext[idx].test_pkt, ATE_TESTPKT_LEN);
	if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
		TESTMODE_SET_PARAM(pAd, band_idx, channel, 100);
		TESTMODE_SET_PARAM(pAd, band_idx, ctrl_ch, 100);
	} else {
		TESTMODE_SET_PARAM(pAd, band_idx, channel, 36);
		TESTMODE_SET_PARAM(pAd, band_idx, ctrl_ch, 36);
	}
	ATECtrl->band_ext[idx].hetb_rx_csd = 0x240004000060FF;
	TESTMODE_SET_PARAM(pAd, band_idx, bw, 0);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXDONE_CNT, 0);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXED_CNT, 0);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_len, 1024);
	TESTMODE_SET_PARAM(pAd, band_idx, pl_len, 1);
	TESTMODE_SET_PARAM(pAd, band_idx, ac_idx, QID_AC_BE);

	payload = TESTMODE_GET_PARAM(pAd, band_idx, payload);
	payload[0] = 0xAA;
	TESTMODE_SET_PARAM(pAd, band_idx, hdr_len, LENGTH_802_11);
	TESTMODE_SET_PARAM(pAd, band_idx, fixed_payload, 1);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TX_CNT, 0xFFFFFFFF);
	TESTMODE_SET_PARAM(pAd, band_idx, retry, 1);
	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		ATECtrl->band_ext[idx].pkt_skb[sta_idx] = NULL;

		NdisMoveMemory(ATECtrl->band_ext[idx].addr1[sta_idx], addr, MAC_ADDR_LEN);
		NdisMoveMemory(ATECtrl->band_ext[idx].addr2[sta_idx], addr, MAC_ADDR_LEN);
		NdisMoveMemory(ATECtrl->band_ext[idx].addr3[sta_idx], addr, MAC_ADDR_LEN);
	}
	NdisMoveMemory(ATECtrl->band_ext[idx].template_frame, ATECtrl->template_frame, 32);
	ATECtrl->band_ext[idx].max_pkt_ext = 2;

	os_zero_mem(&ATECtrl->band_ext[idx].stack, sizeof(ATECtrl->band_ext[idx].stack));
	os_zero_mem(ATECtrl->band_ext[idx].tx_method, sizeof(UCHAR)*TX_MODE_MAX);
	ATECtrl->band_ext[idx].tx_method[MODE_HE_MU] = 1;
	ATECtrl->band_ext[idx].tx_method[MODE_VHT_MIMO] = 1;
	RTMP_OS_INIT_COMPLETION(tx_wait);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "idx:%u, pkt:%p\n",
		idx, ATECtrl->band_ext[idx].test_pkt);

	return ret;
}
#endif

static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	INT32 Ret = 0;
	BOOLEAN Cancelled;
	MT_RX_FILTER_CTRL_T rx_filter;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	UCHAR ant_idx = 0;
#ifdef DBDC_MODE
	UCHAR rx_path[DBDC_BAND_NUM];
	UCHAR band_idx = 0;
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#else
	UCHAR rx_path = 0;
#endif /* DBDC_MODE */

#if defined(DBDC_MODE) && defined(DEFAULT_5G_PROFILE)
	/* Remap wdev_idx for MP 3.3 Driver */
	ATECtrl->wdev_idx = 1;
	Info->wdev_idx = 0;
#endif /* DBDC_MODE */

	/* Remind FW that Enable ATE mode */
	MtCmdATEModeCtrl(pAd, 1);

#if (defined(MT_MAC))
#ifdef TXBF_SUPPORT
	/* Before going into ATE mode, stop sounding first */
	mt_Trigger_Sounding_Packet(pAd, FALSE, 0, 0, 0, NULL);
#endif /* TXBF_SUPPORT */
#endif /* MT_MAC */

	/* Make sure ATEInit successfully when interface up */
	if (ATECtrl->op_mode & ATE_START) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ATE has already started, wdev_idx:%u\n", ATECtrl->wdev_idx);
#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ATE has already started, wdev_idx:%u\n", Info->wdev_idx);
#endif /* DBDC_MODE */

		return Ret;
	}

	/* Allocate ATE TX packet buffer */
	if (!ATECtrl->test_pkt)
		Ret = os_alloc_mem(pAd,	(PUCHAR *)&ATECtrl->test_pkt, ATE_TESTPKT_LEN);

	if (Ret)
		goto err2;

	/* ATE data structure initialization */
	ATECtrl->channel = 1;
	ATECtrl->ctrl_ch = 1;
	ATECtrl->bw = 0;
	ATECtrl->en_man_set_freq = 0;
	ATECtrl->tx_done_cnt = 0;
	ATECtrl->txed_cnt = 0;
	ATECtrl->tx_len = 1024;
	ATECtrl->pl_len = 1;
	ATECtrl->payload[0] = 0xAA;
	ATECtrl->ac_idx = QID_AC_BE;
	ATECtrl->hdr_len = LENGTH_802_11;
	ATECtrl->firstQATool = TRUE;
	ATECtrl->reCalInDumpSts = NO_CHANGE_RECAL;
	RTMP_OS_INIT_COMPLETION(&ATECtrl->tx_wait);

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND0, FALSE, FALSE);
	if (pAd->CommonCfg.dbdc_mode)
		mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND1, FALSE, FALSE);
	set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

	/* reset rx stat */
	NdisZeroMemory(&(ATECtrl->rx_stat), sizeof(ATECtrl->rx_stat));
	ATECtrl->rx_stat.MaxRssi[0] = 0xff;
	ATECtrl->rx_stat.MaxRssi[1] = 0xff;
	ATECtrl->rx_stat.MaxRssi[2] = 0xff;
	ATECtrl->rx_stat.MaxRssi[3] = 0xff;

	/* Backup normal mode attanna settings */
	ATECtrl->tx_path = pAd->Antenna.field.TxPath;
	ATECtrl->rx_path = pAd->Antenna.field.RxPath;
	/* For test mode, dbdc_band0_rx_path is rx_path not stream num */
#ifdef DBDC_MODE
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++)
		rx_path[band_idx] = 0;

	for (ant_idx = 0; ant_idx < pAd->dbdc_band0_rx_path; ant_idx++)
		rx_path[TESTMODE_BAND0] |= (1 << ant_idx);
	pAd->dbdc_band0_rx_path = rx_path[TESTMODE_BAND0];

	for (ant_idx = 0; ant_idx < pAd->dbdc_band1_rx_path; ant_idx++)
		rx_path[TESTMODE_BAND1] |= (1 << ant_idx);
	pAd->dbdc_band1_rx_path = rx_path[TESTMODE_BAND1];

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++)
		TESTMODE_SET_PARAM(pAd, band_idx, rx_ant, rx_path[band_idx]);
#else
	for (ant_idx = 0; ant_idx < ATECtrl->rx_path; ant_idx++)
		rx_path |= (1 << ant_idx);

	pAd->Antenna.field.RxPath = rx_path;
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, rx_ant, rx_path);
#endif

	ATECtrl->hetb_rx_csd = 0x240004000060FF;
	/*
	 *  Backup original CRs and change to ATE mode specific CR setting,
	 *  restore it back when back to normal mode
	 */
	if (chip_ops->backup_reg_before_ate)
		chip_ops->backup_reg_before_ate(pAd);

	/* Common Part */
	/* ATECtrl->en_log = fATE_LOG_TXDUMP; */
	ATECtrl->en_log = 0;
	ATECtrl->verify_mode = HQA_VERIFY;
	ATECtrl->cmd_expire = RTMPMsecsToJiffies(3000);
	RTMP_OS_INIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->TxPower0 = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	ATECtrl->TxPower1 = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	NdisZeroMemory(ATECtrl->log_dump, sizeof(ATECtrl->log_dump[0])*ATE_LOG_TYPE_NUM);
	MT_ATEMPSInit(pAd);
	NdisZeroMemory(ATECtrl->pfmu_info, sizeof(ATECtrl->pfmu_info[0])*ATE_BFMU_NUM);
#ifdef CONFIG_QA
	AsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	AsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
#endif
#ifdef ATE_TXTHREAD
	Ret = TESTMODE_TXTHREAD_INIT(pAd, 0);

	if (Ret)
		goto err3;

	if (IS_MT7626(pAd)) {
		Ret = TESTMODE_PeriodicThread_INIT(pAd);
		if (Ret)
			goto err3;
	}

#endif /* ATE_TXTHREAD */

	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, TESTMODE_BAND1);

	/* Rx filter */
	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = FALSE;
	rx_filter.bFrameReport = TRUE;
	rx_filter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM |
				RX_NOT_OWN_UCAST |
				RX_RTS | RX_CTS | RX_CTRL_RSV |
				RX_BC_MC_DIFF_BSSID_A2 |
				RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 |
				RX_PROTOCOL_VERSION |
				RX_FCS_ERROR;
	rx_filter.u1BandIdx = TESTMODE_BAND0;

	Ret = MtATESetRxFilter(pAd, rx_filter);

	if (IS_ATE_DBDC(pAd)) {
		rx_filter.u1BandIdx = TESTMODE_BAND1;
		Ret = MtATESetRxFilter(pAd, rx_filter);
	}

	/* Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	/* Disable TX PDMA */
	chip_set_hif_dma(pAd, DMA_TX_RX, FALSE);

	if (if_ops->init)
		Ret = if_ops->init(pAd);
	if (if_ops->clean_trx_q)
		Ret = if_ops->clean_trx_q(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev) {
				/* WifiSysApLinkUp(pAd, &pAd->ApCfg.MBSSID[IdBss].wdev); */
				pAd->ApCfg.MBSSID[IdBss].wdev.protection = 0;
			}
		}
	}
#endif
	chip_set_hif_dma(pAd, DMA_TX_RX, TRUE);
#ifdef CONFIG_AP_SUPPORT
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
	ATECtrl->backup_bEnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
	pAd->CommonCfg.bEnableTxBurst = 0;	/* to turn off TXOP */
	ATECtrl->backup_bcn_period = pAd->CommonCfg.BeaconPeriod[DBDC_BAND0];
	pAd->CommonCfg.BeaconPeriod[DBDC_BAND0] = 0;	/* to turn of TBTT timer */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	if (Ret < 0)
		goto err0;

	/* MtTestModeWTBL2Update(pAd, 0); */
	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	/* Turn on TXC support despite of normal mode turned on or off */
	hc_set_txcmd_mode(pAd->hdev_ctrl, 0x1);	/* HOBJ_TX_MODE_TXCMD defined as 0x1 */
#endif
	/* call wdev_do_open must behind ATE status be set */
	ATECtrl->op_mode = ATE_START;
	if (IS_ATE_DBDC(pAd))
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND1, op_mode, ATE_START);

	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, max_pkt_ext, 2);
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, retry, 1);
	Ret = mt_ate_init_wdev(pAd, TESTMODE_BAND0);

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		Ret = MT_ATEInitBandInfo(pAd, TESTMODE_BAND1);
		Ret += mt_ate_init_wdev(pAd, TESTMODE_BAND1);
	}
#endif	/* DBDC_MODE */

	/* Tx Power related Status Initialization
	    Disable TX power related behavior when enter test mode */
#ifdef SINGLE_SKU_V2
	TxPowerSKUCtrl(pAd, FALSE, TESTMODE_BAND0);
	TxPowerPercentCtrl(pAd, FALSE, TESTMODE_BAND0);
	TxPowerBfBackoffCtrl(pAd, FALSE, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd)) {
		TxPowerSKUCtrl(pAd, FALSE, TESTMODE_BAND1);
		TxPowerPercentCtrl(pAd, FALSE, TESTMODE_BAND1);
		TxPowerBfBackoffCtrl(pAd, FALSE, TESTMODE_BAND1);
	}
#endif

#ifdef ATE_TXTHREAD
#endif
	os_zero_mem(&ATECtrl->stack, sizeof(ATECtrl->stack));

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev_idx:%u\n", ATECtrl->wdev_idx);
#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wdev_idx:%u\n", Info->wdev_idx);
#endif /* DBDC_MODE */

	return Ret;

#ifdef ATE_TXTHREAD
err3:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "initial value fail, ret:%d\n", Ret);
	ATECtrl->op_mode = ATE_STOP;
	MT_ATEReleaseLogDump(pAd);
#endif /* ATE_TXTHREAD */
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "if init fail, ret:%d\n", Ret);
err2:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate test packet fail at pakcet\n");

	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	UINT16 wcid = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	INT32 Ret = 0;
	MT_RX_FILTER_CTRL_T rx_filter;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss;
	INT32 MaxNumBss = pAd->ApCfg.BssidNum; /* TODO: Delete after merge MT_ATEStop with Windows Test Mode */
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	PNDIS_PACKET *pkt_skb = NULL;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if ((ATECtrl->op_mode & ATE_STOP) || !(ATECtrl->op_mode & ATE_START))
		goto err2;

	if (chip_ops->restore_reg_after_ate)
		chip_ops->restore_reg_after_ate(pAd);

	if (ATECtrl->op_mode & ATE_FFT) {
		struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

		Ret = ATEOp->SetFFTMode(pAd, 0);
		Ret += MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0, RF_TEST_DEFAULT_RESP_LEN);
		/* For FW to switch back to normal mode stable time */
		mdelay(2000);

		if (Ret)
			goto err0;

		ATECtrl->op_mode &= ~ATE_FFT;
	}

	if (ATECtrl->op_mode & fATE_IN_RFTEST) {
		Ret += MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0, RF_TEST_DEFAULT_RESP_LEN);
		/* For FW to switch back to normal mode stable time */
		mdelay(2000);

		if (Ret)
			goto err0;
	}

	/* MT76x6 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Manual Set Frequency Restore\n");
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = FALSE;
	rx_filter.bFrameReport = FALSE;
	rx_filter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM |
				RX_NOT_OWN_UCAST |
				RX_RTS | RX_CTS | RX_CTRL_RSV |
				RX_BC_MC_DIFF_BSSID_A2 |
				RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 |
				RX_PROTOCOL_VERSION |
				RX_FCS_ERROR;
	rx_filter.u1BandIdx = 0;

	Ret = MtATESetRxFilter(pAd, rx_filter);

	if (IS_ATE_DBDC(pAd)) {
		rx_filter.u1BandIdx = 1;
		Ret = MtATESetRxFilter(pAd, rx_filter);
	}

	MT_ATEReleaseLogDump(pAd);
	MT_ATEMPSRelease(pAd);

	/* Release skb */
	for (wcid = 0 ; wcid < MAX_MULTI_TX_STA ; wcid++) {
		pkt_skb = &ATECtrl->pkt_skb[wcid];

		if (*pkt_skb) {
			RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);
			*pkt_skb = NULL;
		}
	}

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		for (wcid = 0 ; wcid < MAX_MULTI_TX_STA ; wcid++) {
			pkt_skb = &ATECtrl->band_ext[TESTMODE_BAND1 - 1].pkt_skb[wcid];

			if (*pkt_skb) {
				RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);
				*pkt_skb = NULL;
			}
		}
	}
#endif /*DBDC_MODE */
#ifdef ATE_TXTHREAD
	Ret = TESTMODE_TXTHREAD_RELEASE(pAd, 0);
	msleep(2);
	if (IS_MT7626(pAd)) {
		ATECtrl->periodic_thread.service_stat = FALSE;
		Ret = TESTMODE_PeriodicThread_RELEASE(pAd);
		msleep(2);
	}
#endif /* ATE_TXTHREAD */

	if (if_ops->clean_trx_q)
		Ret = if_ops->clean_trx_q(pAd);

	if (if_ops->ate_leave)
		Ret += if_ops->ate_leave(pAd);

	if (Ret)
		goto err1;

	Ret = mt_ate_release_wdev(pAd, TESTMODE_BAND0);

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		Ret = mt_ate_release_wdev(pAd, TESTMODE_BAND1);
		Ret += MT_ATEReleaseBandInfo(pAd, TESTMODE_BAND1);
	}
#endif
	NICInitializeAdapter(pAd);

	/* RTMPEnableRxTx(pAd); */
	if (pAd->CommonCfg.bTXRX_RXV_ON) {
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND1);
	} else {
		/* Normal mode enabled RXV and interface down up will crash if disalbe it */
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE, TESTMODE_BAND1);
	}

#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
		MtATEInitCCK_OFDM_Path(pAd, TESTMODE_BAND1);
	}
#endif
	RTMP_OS_EXIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->op_mode = ATE_STOP;
	if (IS_ATE_DBDC(pAd))
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND1, op_mode, ATE_STOP);
	MtCmdATEModeCtrl(pAd, 0); /* Remind FW that Disable ATE mode */
	RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

#ifdef CONFIG_AP_SUPPORT
	pAd->CommonCfg.bEnableTxBurst = ATECtrl->backup_bEnableTxBurst;
	pAd->CommonCfg.BeaconPeriod[DBDC_BAND0] = ATECtrl->backup_bcn_period;
	APStartUp(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPROT  */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd)) {
			MaxNumBss = MAX_MBSSID_NUM(pAd);
		}
		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	if (Ret)
		goto err1;

	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RF-test stop fail, ret:%d\n", Ret);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "if stop fail, ret:%d\n", Ret);
err2:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "ATE has already stopped ret:%d\n", Ret);

	ATECtrl->op_mode = ATE_STOP;
	return Ret;
}
#else
static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree = 0;
#endif /* CAL_FREE_IC_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
#ifdef CAL_FREE_IC_SUPPORT
	RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

	if (bCalFree) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Cal Free IC!!\n");
		RTMP_CAL_FREE_DATA_GET(pAd);
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Non Cal Free IC!!\n");

#endif /* CAL_FREE_IC_SUPPORT */

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MT76x6 Manual Set Frequency Restore\n");
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	if (ATECtrl->op_mode & fATE_TXCONT_ENABLE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Stop Continuous Tx\n");
		ATEOp->StopContinousTx(pAd, 0);
	}

	if (ATECtrl->op_mode & fATE_TXCARRSUPP_ENABLE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Stop Carrier Suppression Test\n");
		ATEOp->StopTxTone(pAd);
	}

	/* Reset ATE TX/RX Counter */
	ATECtrl->tx_len = 1024;
	ATECtrl->ac_idx = QID_AC_BE;
	ATECtrl->TxPower0 = 0x10;
	ATECtrl->TxPower1 = 0x10;
	ATECtrl->en_man_set_freq = 0;
	ATECtrl->tx_done_cnt = 0;
	ATECtrl->op_mode = ATE_START;
	AsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	AsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
	/* MtTestModeWTBL2Update(pAd, 0); */
	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	MtATESetMacTxRx(pAd, ASIC_MAC_RXV, FALSE, TESTMODE_BAND0);

	/* RTMPEnableRxTx(pAd); */

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MT76x6 Manual Set Frequency Restore\n");
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	ATECtrl->op_mode = ATE_STOP;
	return Ret;
}
#endif


INT mt_ate_set_tmac_info(RTMP_ADAPTER *pAd, TMAC_INFO *tmac_info, UINT32 band_idx)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UCHAR *addr1 = NULL;
	UCHAR tx_mode = 0;
	UCHAR mcs = 0;
	UCHAR vht_nss = 1;
	UINT32 ant_sel = 0;
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_qos = tx_time_param->pkt_need_qos;
	UINT8 need_amsdu = tx_time_param->pkt_need_amsdu;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
#if !defined(COMPOS_TESTMODE_WIN)
#if defined(MT_MAC)
	struct wifi_dev *wdev = NULL;
	UCHAR WmmIdx;
#ifdef SINGLE_SKU_V2
	BOOLEAN fgSPE;
#endif	/* SINGLE_SKU_V2 */
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	UCHAR control_band_idx = ATECtrl->control_band_idx;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = NULL;
#endif /* DBDC_MODE */
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

	wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "wdev_idx=%d\n", wdev->wdev_idx);

	if (!wdev)
		goto err0;

#endif	/* MT_MAC */
#endif	/* !COMPOS_TESTMODE_WIN */
	TESTMODE_SET_PARAM(pAd, band_idx, hdr_len, LENGTH_802_11);
	addr1 = TESTMODE_GET_PARAM(pAd, band_idx, addr1[0]);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "addr1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr1));
	ant_sel = TESTMODE_GET_PARAM(pAd, band_idx, tx_ant);
	tx_mode = TESTMODE_GET_PARAM(pAd, band_idx, tx_mode);
	mcs = TESTMODE_GET_PARAM(pAd, band_idx, mcs);
	vht_nss = TESTMODE_GET_PARAM(pAd, band_idx, nss);
	/* Fill TMAC_INFO */
	NdisZeroMemory(tmac_info, sizeof(*tmac_info));
	tmac_info->LongFmt = TRUE;

	if (pkt_tx_time > 0) {
		tmac_info->WifiHdrLen = (UINT8) tx_time_param->pkt_hdr_len;
		tmac_info->PktLen = (UINT16) tx_time_param->pkt_msdu_len;
		tmac_info->NeedTrans = FALSE;

		if (need_qos | need_amsdu | need_ampdu) {
			tmac_info->HdrPad = 2;
			tmac_info->BmcPkt = FALSE;
			tmac_info->UserPriority = 0;
		} else {
			tmac_info->HdrPad = 0;
			tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
			tmac_info->UserPriority = 0;
		}
	} else {
		tmac_info->WifiHdrLen = (UINT8)TESTMODE_GET_PARAM(pAd, band_idx, hdr_len);
		tmac_info->HdrPad = 0;
		tmac_info->PktLen = (UINT16)TESTMODE_GET_PARAM(pAd, band_idx, tx_len);
		tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
	}

	/* no ack */
	if ((pkt_tx_time > 0) && (need_ampdu))
		tmac_info->bAckRequired = 1;
	else
		tmac_info->bAckRequired = 0;

#if !defined(COMPOS_TESTMODE_WIN)
	tmac_info->FrmType = FC_TYPE_DATA;
	tmac_info->SubType = SUBTYPE_QDATA;
	tmac_info->OwnMacIdx = wdev->OmacIdx;
#else
	tmac_info->OwnMacIdx = 0;
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tmac_info->OwnMacIdx=%d\n", tmac_info->OwnMacIdx);
	/* no frag */
	tmac_info->FragIdx = 0;
	/* no protection */
	tmac_info->CipherAlg = 0;
	/* TX Path setting */
	tmac_info->VhtNss = vht_nss ? vht_nss : 1;
	tmac_info->AntPri = 0;
	tmac_info->SpeEn = 0;

	/* Timing Measure setting */
	if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
		tmac_info->TimingMeasure = 1;

	/* band_idx for TX ring choose */
	tmac_info->band_idx = band_idx;

	switch (ant_sel) {
	case 0: /* Both */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 1;
		break;

	case 1: /* TX0 */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 0;
		break;

	case 2: /* TX1 */
		tmac_info->AntPri = 2; /* b'010 */
		tmac_info->SpeEn = 0;
		break;
	}

#if defined(MT_MAC)
	/* Need to modify the way of wmm_idx getting */
	WmmIdx = TESTMODE_GET_PARAM(pAd, band_idx, wmm_idx);
	tmac_info->WmmSet = WmmIdx;


	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (ant_sel & ATE_ANT_USER_SEL) {
			ant_sel &= ~ATE_ANT_USER_SEL;
			tmac_info->AntPri = ant_sel;
		} else {
			INT map_idx = 0;

			for (map_idx = 0; map_idx < ARRAY_SIZE(ant_to_spe_idx_map); map_idx++) {
				if (ant_sel == ant_to_spe_idx_map[map_idx].ant_sel)
					break;
			}

			if (map_idx == ARRAY_SIZE(ant_to_spe_idx_map))
				tmac_info->AntPri = 0;
			else
				tmac_info->AntPri = ant_to_spe_idx_map[map_idx].spe_idx;

#if defined(TXBF_SUPPORT) && defined(MT_MAC) /*Force SPE=0 if (1. TxBF is enabled and (2. use antenna selection */
			if (control_band_idx == 0) { /* For Band 0 */
				if (ATECtrl->ebf || ATECtrl->ibf) {
					tmac_info->AntPri = 0;
					/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "AntPri = 0 for TxBF\n"); */
				}
			}
#ifdef DBDC_MODE
			else {
				Info = &(ATECtrl->band_ext[0]); /* For Band 1 */
				if (Info != NULL) {
					if (Info->ebf || Info->ibf) {
						tmac_info->AntPri = 0;
						/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "DBDC AntPri = 0 for TxBF\n"); */
					}
				}
			}
#endif /* DBDC_MODE */
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)

		if (g_EBF_certification) {
			if (BF_ON_certification) {
				tmac_info->AntPri = 0;
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tmac_info->AntPri = 0\n");
			} else {
				tmac_info->AntPri = 24;  /* 24 */
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tmac_info->AntPri = 24\n");
			}
		}

#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ant_sel:%x, ant_pri:%u, vht_nss:%x, TxD.VhtNss:%x\n",
			ant_sel, tmac_info->AntPri, vht_nss, tmac_info->VhtNss);
	}
#endif /* MT_MAC */

	/* Fill transmit setting */
	tmac_info->TxRadioSet.RateCode = mcs;
	tmac_info->TxRadioSet.CurrentPerPktBW = TESTMODE_GET_PARAM(pAd, band_idx, per_pkt_bw);
	tmac_info->TxRadioSet.ShortGI =	TESTMODE_GET_PARAM(pAd, band_idx, sgi);
	tmac_info->TxRadioSet.Stbc = TESTMODE_GET_PARAM(pAd, band_idx, stbc);
	tmac_info->TxRadioSet.PhyMode = tx_mode;
	tmac_info->TxRadioSet.Ldpc = TESTMODE_GET_PARAM(pAd, band_idx, ldpc);

	tmac_info->QueIdx = asic_get_hwq_from_ac(pAd, tmac_info->WmmSet, TESTMODE_GET_PARAM(pAd, band_idx, ac_idx));

	if ((pkt_tx_time > 0) && (need_ampdu)) {
		tmac_info->Wcid = ATECtrl->wcid_ref;
		tmac_info->FixRate = 0;
		tmac_info->BaDisable = FALSE;
		tmac_info->RemainTxCnt = 1;
	} else {
		tmac_info->Wcid = 0;
		tmac_info->FixRate = 1;
		tmac_info->BaDisable = TRUE;
		tmac_info->RemainTxCnt = 15;
	}

	if (ATECtrl->txs_enable) {
		tmac_info->TxS2Host = TRUE;
		tmac_info->TxS2Mcu = FALSE;
		tmac_info->TxSFmt = 1;
	}

	if (tx_mode == MODE_CCK) {
		tmac_info->TxRadioSet.Premable = LONG_PREAMBLE;

		if (mcs == MCS_9) {
			tmac_info->TxRadioSet.RateCode = 0;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_10) {
			tmac_info->TxRadioSet.RateCode = 1;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_11) {
			tmac_info->TxRadioSet.RateCode = 2;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		}
	}

#ifdef TXBF_SUPPORT
	else {
		UCHAR iTxBf = TESTMODE_GET_PARAM(pAd, band_idx, ibf);
		UCHAR eTxBf = TESTMODE_GET_PARAM(pAd, band_idx, ebf);

		if (iTxBf || eTxBf) {
			tmac_info->TxRadioSet.ItxBFEnable = iTxBf;
			tmac_info->TxRadioSet.EtxBFEnable = eTxBf;
		}
	}

#endif /* TXBF_SUPPORT */
	tmac_info->Wcid = ATECtrl->wcid_ref;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tmac_info->TxRadioSet.EtxBFEnable=%d, tmac_info->Wcid=%d\n",
		tmac_info->TxRadioSet.EtxBFEnable, tmac_info->Wcid);

#ifdef SINGLE_SKU_V2
	if (tmac_info->AntPri >= 24)
		fgSPE = TRUE;
	else
		fgSPE = FALSE;

	/* Update Power offset according to Band, Phymode, MCS, BW, Nss, SPE */
	tmac_info->PowerOffset = SKUTxPwrOffsetGet(pAd, band_idx, TESTMODE_GET_PARAM(pAd, band_idx, per_pkt_bw), TESTMODE_GET_PARAM(pAd, band_idx, tx_mode), TESTMODE_GET_PARAM(pAd, band_idx, mcs), TESTMODE_GET_PARAM(pAd, band_idx, nss), fgSPE);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "tmac_info->PowerOffset = 0x%x (%d)\n",
		tmac_info->PowerOffset, tmac_info->PowerOffset);

	if (tmac_info->PowerOffset < -16)
		tmac_info->PowerOffset = -16;
	else if (tmac_info->PowerOffset > 15)
		tmac_info->PowerOffset = 15;
#endif /* SINGLE_SKU_V2 */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
	if ((pkt_tx_time > 0) && (need_ampdu)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tmac_info->Wcid/Wmmset/QueIdx=%d/%d/%d\n",
			tmac_info->Wcid, tmac_info->WmmSet, tmac_info->QueIdx);
	}
#endif
	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cannot get Wdev by band:%d\n", band_idx);
	return NDIS_STATUS_FAILURE;
}


UINT32 mt_ate_get_txlen_by_pkt_tx_time(struct _RTMP_ADAPTER *ad, UINT32 band_idx)
{
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(ad, band_idx, tx_time_param);
	UINT32 txlen = 0;
	UINT32 hlen = 0;
	UINT32 tx_data_rate = 0;
	UINT32 pkt_tx_time = 0;
	UCHAR tx_mode = 0;
	UCHAR mcs = 0, mcs_1ss, nss = 1;
	UCHAR bw = 0, bw_fact = 1;
	UCHAR gi = 0;

	/*
	 * 1. Get the tx data rate
	 * 2. Get the packet tx time
	 * 3. Calculate the packet length by tx_data_rate and packet_tx_time
	 * 4. Return txlen
	 */
	pkt_tx_time = tx_time_param->pkt_tx_time;
	hlen = TESTMODE_GET_PARAM(ad, band_idx, hdr_len);
	tx_mode = TESTMODE_GET_PARAM(ad, band_idx, tx_mode);
	mcs = TESTMODE_GET_PARAM(ad, band_idx, mcs);
	bw = TESTMODE_GET_PARAM(ad, band_idx, bw);
	gi = TESTMODE_GET_PARAM(ad, band_idx, sgi);
	mcs_1ss = mcs;

	if (tx_mode == MODE_CCK) { /* Legacy CCK mode */
		UINT8 cck_map_idx = 0;

		for (cck_map_idx = 0;
			 cck_map_idx < ARRAY_SIZE(cck_mode_mcs_to_data_rate_map); cck_map_idx++) {
			if (mcs_1ss == cck_mode_mcs_to_data_rate_map[cck_map_idx].mcs)
				break;
		}

		if (cck_map_idx == ARRAY_SIZE(cck_mode_mcs_to_data_rate_map)) {
			tx_data_rate = cck_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	cck_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ad, band_idx, mcs, mcs);
		} else
			tx_data_rate = cck_mode_mcs_to_data_rate_map[cck_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == MODE_OFDM) { /* Legacy OFDM mode */
		UINT8 ofdm_map_idx = 0;

		for (ofdm_map_idx = 0; ofdm_map_idx < ARRAY_SIZE(ofdm_mode_mcs_to_data_rate_map); ofdm_map_idx++) {
			if (mcs_1ss == ofdm_mode_mcs_to_data_rate_map[ofdm_map_idx].mcs)
				break;
		}

		if (ofdm_map_idx == ARRAY_SIZE(ofdm_mode_mcs_to_data_rate_map)) {
			tx_data_rate = ofdm_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	ofdm_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ad, band_idx, mcs, mcs);
		} else
			tx_data_rate = ofdm_mode_mcs_to_data_rate_map[ofdm_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) { /* HT mode */
		UINT8 n_map_idx = 0;

		if (mcs != MCS_32) {
			mcs_1ss = mcs % 8;
			nss = (mcs / 8) + 1;
			bw_fact = (bw == BW_40) ? 2 : 1;
		} else {
			bw_fact = 1;
			nss = 1;
		}

		for (n_map_idx = 0; n_map_idx < ARRAY_SIZE(n_mode_mcs_to_data_rate_map); n_map_idx++) {
			if (mcs_1ss == n_mode_mcs_to_data_rate_map[n_map_idx].mcs)
				break;
		}

		if (n_map_idx == ARRAY_SIZE(n_mode_mcs_to_data_rate_map)) {
			tx_data_rate = n_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	n_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ad, band_idx, mcs, mcs);
		} else
			tx_data_rate = n_mode_mcs_to_data_rate_map[n_map_idx].tx_data_rate;

		tx_data_rate = tx_data_rate * nss * bw_fact;

		if (gi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == MODE_VHT || tx_mode == MODE_VHT_MIMO) { /* VHT mode */
		UINT8 ac_map_idx = 0;
		struct _ATE_DATA_RATE_MAP *vht_rate_map;
		UINT32 array_cnt = 0;

		if (bw == BW_20) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw20);
		} else if (bw == BW_40) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw40;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw40);
		} else if (bw == BW_80) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw80;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw80);
		} else if (bw == BW_160) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw160;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw160);
		} else {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw20);
		}

		nss = TESTMODE_GET_PARAM(ad, band_idx, nss);

		for (ac_map_idx = 0; ac_map_idx < array_cnt; ac_map_idx++) {
			if (mcs == vht_rate_map[ac_map_idx].mcs)
				break;
		}

		if (ac_map_idx == array_cnt) {
			tx_data_rate = vht_rate_map[0].tx_data_rate;
			mcs = mcs_1ss = vht_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ad, band_idx, mcs, mcs);
		} else
			tx_data_rate = vht_rate_map[ac_map_idx].tx_data_rate;

		tx_data_rate *= nss;

		/* TODO: Need to check for SGI equation! */
		if (gi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 10 / 8;
	}
#if defined(DOT11_HE_AX)
	else if (tx_mode == MODE_HE_SU) {
		UINT8 he_su_map_idx = 0;
		struct _ATE_DATA_RATE_MAP *he_rate_map;
		UINT32 array_cnt = 0;

		switch (TESTMODE_GET_PARAM(ad, band_idx, sgi)) {
		case 0:
		case 1:
		case 4:
			gi = 0;
			break;
		case 2:
			gi = 1;
			break;
		case 3:
			gi = 2;
			break;
		}

		nss = TESTMODE_GET_PARAM(ad, band_idx, nss);

		if (bw == BW_20) {
			he_rate_map = he_su_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(he_su_mode_mcs_to_data_rate_map_bw20);
		} else if (bw == BW_40) {
			he_rate_map = he_su_mode_mcs_to_data_rate_map_bw40;
			array_cnt = ARRAY_SIZE(he_su_mode_mcs_to_data_rate_map_bw40);
		} else if (bw == BW_80) {
			he_rate_map = he_su_mode_mcs_to_data_rate_map_bw80;
			array_cnt = ARRAY_SIZE(he_su_mode_mcs_to_data_rate_map_bw80);
		} else if (bw == BW_8080 || bw == BW_160) {
			he_rate_map = he_su_mode_mcs_to_data_rate_map_bw160;
			array_cnt = ARRAY_SIZE(he_su_mode_mcs_to_data_rate_map_bw160);
		} else {
			he_rate_map = he_su_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(he_su_mode_mcs_to_data_rate_map_bw20);
		}

		for (he_su_map_idx = 0; he_su_map_idx < array_cnt; he_su_map_idx++) {
			if (mcs == he_rate_map[he_su_map_idx].mcs)
				break;
		}

		if (he_su_map_idx == array_cnt) {
			tx_data_rate = he_rate_map[0].tx_data_rate;
			mcs = mcs_1ss = he_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ad, band_idx, mcs, mcs);
		} else
			tx_data_rate = he_rate_map[he_su_map_idx].tx_data_rate;

		tx_data_rate *= nss;

		switch (gi) {
		case 1:
			tx_data_rate = (tx_data_rate * 100) / 90;
			break;

		case 2:
			tx_data_rate = (tx_data_rate * 100) / 85;
			break;

		default:
			if (gi > 0)
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid gi=%d, ignored as 0.\n", gi);
			break;
		}
		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 10 / 8;
	} else if (tx_mode == MODE_HE_TRIG) {
		struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[0]);
		INT32 data_subcarriers = 0, data_subcarriers_short = 0;

		mcs = (ru_sta->rate & 0xf);
		data_subcarriers = mt_ate_get_subcarriers(ru_sta->ru_index >> 1, (ru_sta->rate & BIT5));

		if (data_subcarriers) {
			data_subcarriers_short = mt_ate_get_subcarriers_short(ru_sta->ru_index >> 1, (ru_sta->rate & BIT5));

			nss = ru_sta->nss;
			ru_sta->cbps = data_subcarriers * nss * he_bpscs[mcs];
			ru_sta->dbps = ru_sta->cbps * (he_rate_density[mcs]-1) / he_rate_density[mcs];

			txlen = mt_ate_calc_bytes_by_time(tx_mode, nss, 0, mt_ate_translate_ltf(tx_mode, gi), mt_ate_translate_gi(tx_mode, gi), ru_sta->dbps, pkt_tx_time);
			txlen -= 13;	/* reserve FCS(4)+Delimiter(4)+A-Control(4)+H/W revered(1) */
			ru_sta->mpdu_length = txlen;	/* reserve FCS(4)+Delimiter(4)+A-Control(4)+H/W revered(1) */
		} else {
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"unknown RU Index:[%d], forced transmit l024 bytes MPDU!\n", ru_sta->ru_index >> 1);

			txlen = 1024 - 13;	/* reserve FCS(4)+Delimiter(4)+A-Control(4)+H/W revered(1) */
		}
	}
#endif	/* DOT11_HE_AX */
	MTWF_PRINT("%s: phy_mode=%d, mcs/mcs_1ss=%d/%d, nss=%d, bw/bw_fact=%d/%d, sgi=%d\n",
		__func__, tx_mode, mcs, mcs_1ss, nss, bw, bw_fact, gi);
	MTWF_PRINT("%s: txlen=%d, pkt_tx_time=%d, tx_data_rate=%d\n",
		__func__, txlen, pkt_tx_time, tx_data_rate);

#if defined(DOT11_HE_AX)
	if (tx_mode > MODE_VHT) {
		if (txlen >= (MAX_VHT_MPDU_LEN * 256)) {
			txlen = (MAX_VHT_MPDU_LEN * 256);
			MTWF_PRINT("%s: Expected txlen > HE mode PPDU max length, reduce the txlen=%d\n",
				__func__, txlen);
		}
	} else
#endif /* DOT11_HE_AX */
	if (tx_mode == MODE_VHT || tx_mode == MODE_VHT_MIMO) {
		if (txlen >= (MAX_VHT_MPDU_LEN * 64)) {
			txlen = (MAX_VHT_MPDU_LEN * 64);
			MTWF_PRINT("%s: Expected txlen > VHT mode PPDU max length, reduce the txlen=%d\n",
				__func__, txlen);
		}
	} else if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) {
		if (txlen >= MAX_HT_AMPDU_LEN) {
			txlen = MAX_HT_AMPDU_LEN;
			MTWF_PRINT("%s: Expected txlen > HT mode PPDU max length, reduce the txlen=%d\n", __func__, txlen);
		}
	} else if (tx_mode == MODE_OFDM) {
		if (txlen >= MAX_MSDU_LEN) {
			txlen = MAX_MSDU_LEN;
			MTWF_PRINT("%s: Expected txlen > OFDM mode PPDU max length, reduce the txlen=%d\n", __func__, txlen);
		}
	} else if (tx_mode == MODE_CCK) {
		if (txlen >= MAX_MSDU_LEN) {
			txlen = MAX_MSDU_LEN;
			MTWF_PRINT("%s: Expected txlen > CCK mode PPDU max length, reduce the txlen=%d\n", __func__, txlen);
		}
	}

	return txlen;
}


UINT32 mt_ate_get_hlen_by_pkt_tx_time(
	struct _RTMP_ADAPTER *pAd,
	UINT32 band_idx,
	UINT32 txlen,
	BOOLEAN *need_qos,
	BOOLEAN *need_amsdu,
	BOOLEAN *need_ampdu)
{
	UINT32 pkt_len = 0;
	UINT32 hlen = 0;
	UCHAR tx_mode = 0;
	UCHAR use_data_frame = 1;
	/*
	 * 1. Get the tx data rate
	 * 2. Check if need to send packet with AMPDU format
	 * 3. Check if need to send packet with AMSDU-in-AMPDU format
	 * 4. Return the expected packet header length by tx packet type
	 *  if need to has QoS field and HTC field.
	*/
	tx_mode = TESTMODE_GET_PARAM(pAd, band_idx, tx_mode);
	pkt_len = txlen;

	if (pkt_len <= MIN_MSDU_LEN) {
		use_data_frame = 0;
		/* Here we need to go mgmt/ctrl frame mode */
	} else if (pkt_len <= MAX_MSDU_LEN) {
		if (tx_mode == MODE_VHT || tx_mode == MODE_VHT_MIMO || tx_mode == MODE_HE_MU)
			*need_qos = 1;
	} else if (pkt_len <= DEFAULT_MPDU_LEN) {
		if (IS_SUPPORT_ATE_TX_BY_TIME(pAd) && (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD)) {
			*need_amsdu = 1;
			*need_qos = 1;
		} else if (tx_mode == MODE_VHT || tx_mode == MODE_VHT_MIMO || tx_mode == MODE_HE_MU)
			*need_qos = 1;
		else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't generate frame with such length for CCK/OFDM mode!!\n");
		}
	} else if (pkt_len <= MAX_VHT_MPDU_LEN) {
		if (tx_mode == MODE_VHT || tx_mode == MODE_VHT_MIMO || tx_mode == MODE_HE_MU)
			*need_qos = 1;
		else if (IS_SUPPORT_ATE_TX_BY_TIME(pAd) && (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD)) {
			*need_ampdu = 1;
			*need_amsdu = 1;
			*need_qos = 1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't generate frame with such length for CCK/OFDM mode!!\n");
		}
	} else {
		if (IS_SUPPORT_ATE_TX_BY_TIME(pAd) && (tx_mode > MODE_OFDM)) {
			*need_ampdu = 1;
			*need_amsdu = 1;
			*need_qos = 1;
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't generate frame with such length for CCK/OFDM mode!!\n");
		}
	}

	if (tx_mode == MODE_VHT_MIMO || tx_mode == MODE_HE_MU)
		*need_ampdu = 1;

	hlen = TESTMODE_GET_PARAM(pAd, band_idx, hdr_len);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "original header len=%d\n", hlen);

	if (use_data_frame) {
		hlen = DEFAULT_MAC_HDR_LEN;

		if (*need_qos)
			hlen = QOS_MAC_HDR_LEN;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "pkt_len=%d, need_qos/amsdu/ampdu/dataframe/hlen=%d/%d/%d/%d/%d\n",
			  pkt_len, *need_qos, *need_amsdu, *need_ampdu, use_data_frame, hlen);
	return hlen;
}

INT mt_ate_wtbl_cfg_v2(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	P_HT_CAP_T   rWtblHtCap;
	P_VHT_CAP_T  rWtblVhtCap;
	P_ANT_CAP_T  rWtblAntCap;
	P_BA_CAP_T   rWtblBaCap;
	P_RATE_CAP_T rWtblRateCap;
	UINT8 need_qos, need_amsdu, need_ampdu;
	UCHAR phy_mode, mcs, nss, bw, sgi, stbc, ldpc, preamble, u4Stbc;
	UINT32 ant_sel = 0;


	rWtblHtCap   = &param.Data.rAteSetAmpduWtbl.rWtblHt;
	rWtblVhtCap  = &param.Data.rAteSetAmpduWtbl.rWtblVht;
	rWtblAntCap  = &param.Data.rAteSetAmpduWtbl.rWtblAnt;
	rWtblBaCap   = &param.Data.rAteSetAmpduWtbl.rWtblBa;
	rWtblRateCap = &param.Data.rAteSetAmpduWtbl.rWtblRate;

	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;
	phy_mode = TESTMODE_GET_PARAM(pAd, band_idx, tx_mode);
	mcs = TESTMODE_GET_PARAM(pAd, band_idx, mcs);
	nss = TESTMODE_GET_PARAM(pAd, band_idx, nss);
	bw = TESTMODE_GET_PARAM(pAd, band_idx, bw);
	sgi = TESTMODE_GET_PARAM(pAd, band_idx, sgi);
	stbc = TESTMODE_GET_PARAM(pAd, band_idx, stbc);
	ldpc = TESTMODE_GET_PARAM(pAd, band_idx, ldpc);
	ant_sel = TESTMODE_GET_PARAM(pAd, band_idx, tx_ant);
	preamble = TESTMODE_GET_PARAM(pAd, band_idx, preamble);

	os_zero_mem(&param, sizeof(param));

	param.ucAteTestModeEn = TRUE;
	param.ucAteIdx = ENUM_ATE_SET_AMPDU_WTBL;

	switch (phy_mode) {
	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		rWtblHtCap->fgIsHT = TRUE;
		rWtblHtCap->fgLDPC = ldpc;

		if (cap)
			rWtblHtCap->ucAmpduFactor = cap->ppdu.ht_max_ampdu_len_exp;
		else
			rWtblHtCap->ucAmpduFactor = 3;

		break;

	case MODE_VHT:
		rWtblHtCap->fgIsHT = 1;
		rWtblVhtCap->fgIsVHT = 1;
		rWtblVhtCap->fgVhtLDPC = ldpc;

		if (cap)
			rWtblHtCap->ucAmpduFactor = cap->ppdu.vht_max_ampdu_len_exp;
		else
			rWtblHtCap->ucAmpduFactor = 7;

		break;

	default:
		rWtblHtCap->fgIsHT = 0;
		rWtblVhtCap->fgIsVHT = 0;
		break;
	}

	if (need_ampdu) {
		if (ant_sel & ATE_ANT_USER_SEL) {
			ant_sel &= ~ATE_ANT_USER_SEL;
		} else {
			INT map_idx = 0;
			INT map_idx_len = sizeof(ant_to_spe_idx_map) / sizeof(ant_to_spe_idx_map[0]);

			for (map_idx = 0; map_idx < map_idx_len; map_idx++) {
				if (ant_sel == ant_to_spe_idx_map[map_idx].ant_sel)
					break;
			}
			if (map_idx == map_idx_len)
				ant_sel = 0;
			else
				ant_sel = ant_to_spe_idx_map[map_idx].spe_idx;
		}

		rWtblAntCap->ucSpe = (ant_sel & 0x1F);
		rWtblAntCap->AntIDConfig.ucANTIDSts0 = ant_sel;
		rWtblAntCap->AntIDConfig.ucANTIDSts1 = ant_sel;
		rWtblAntCap->AntIDConfig.ucANTIDSts2 = ant_sel;
		rWtblAntCap->AntIDConfig.ucANTIDSts3 = ant_sel;

		rWtblBaCap->ucBaEn = 1;
		rWtblBaCap->ucBaSize = 7;
		param.Data.rAteSetAmpduWtbl.ucIPsm = 1;
	}

	rWtblRateCap->ucFcap = bw;

	if (sgi) {
		switch (bw) {
		case BW_20:
			rWtblRateCap->fgG2 = TRUE;
			break;

		case BW_40:
			rWtblRateCap->fgG4 = TRUE;
			break;

		case BW_80:
			rWtblRateCap->fgG8 = TRUE;
			break;

		case BW_160:
			rWtblRateCap->fgG16 = TRUE;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Can't find such bw, use default\n");
			break;
		}
    }

	u4Stbc = raStbcSettingCheck(stbc, phy_mode, mcs, nss, 0, 0);

	rWtblRateCap->ucStbc = u4Stbc;
	rWtblRateCap->ucMode = phy_mode;
	rWtblRateCap->ucSgi = sgi;
	rWtblRateCap->ucBw = bw;
	rWtblRateCap->ucNss = nss;
	rWtblRateCap->ucPreamble = preamble;
	rWtblRateCap->ucLdpc = ldpc;
	rWtblRateCap->au2RateCode = mcs;

	if (need_qos)
		param.Data.rAteSetAmpduWtbl.ucQos = 1;

#ifdef CONFIG_HW_HAL_OFFLOAD
	ret = MtCmdATETest(pAd, &param);
#endif

	return ret;
}


INT mt_ate_wtbl_cfg(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT ret;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UINT8 need_qos, need_amsdu, need_ampdu;
	UCHAR tx_mode, mcs, nss, bw, sgi, stbc;
	struct cmd_wtbl_dw_mask_set dw_set[7];
	UINT32 wtbl_rate = 0, ant_sel = 0;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;
	tx_mode = TESTMODE_GET_PARAM(pAd, band_idx, tx_mode);
	mcs = TESTMODE_GET_PARAM(pAd, band_idx, mcs);
	nss = TESTMODE_GET_PARAM(pAd, band_idx, nss);
	bw = TESTMODE_GET_PARAM(pAd, band_idx, bw);
	sgi = TESTMODE_GET_PARAM(pAd, band_idx, sgi);
	stbc = TESTMODE_GET_PARAM(pAd, band_idx, stbc);
	ant_sel = TESTMODE_GET_PARAM(pAd, band_idx, tx_ant);
	NdisZeroMemory((UCHAR *)&dw_set[0], sizeof(dw_set));
	/* Decide TxCap, HT/VHT/LPDC (DW2) */
	dw_set[0].ucWhichDW = 2;
	dw_set[0].u4DwMask = 0x9FFFFFFF;

	switch (tx_mode) {
	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		dw_set[0].u4DwValue = (0x1 << 29);
		break;

	case MODE_VHT:
		dw_set[0].u4DwValue = (0x3 << 29);
		break;

	default:
		dw_set[0].u4DwValue = 0;
		break;
	};

	/* Decide AF/I_PSM (DW3) */
	dw_set[1].ucWhichDW = 3;

	dw_set[1].u4DwMask = 0xD8E0F000;

	if (need_ampdu) {

		dw_set[1].u4DwValue = (0x1 << 29)
					| (0x7 << 24)
					| ((ant_sel & 0x1F) << 16)
					| ((ant_sel & 0x7) << 9)
					| ((ant_sel & 0x7) << 6)
					| ((ant_sel & 0x7) << 3)
					| ((ant_sel & 0x7) << 0);
	} else
		dw_set[1].u4DwValue = 0;

	/* Decide BA-enable/BA-winsize/BA-bitmap (DW4) */
	dw_set[2].ucWhichDW = 4;
	dw_set[2].u4DwMask = 0x0;

	if (need_ampdu)
		dw_set[2].u4DwValue = 0xFFFFFFFF;
	else
		dw_set[2].u4DwValue = 0x0;

	/* Decide FCAP/G2/G4/G8/G16/QoS-enable (DW5 )*/
	dw_set[3].ucWhichDW = 5;
	dw_set[3].u4DwMask = 0xFFF7C0FF;

	switch (bw) {
	case BW_20:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 8);
		break;

	case BW_40:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 9);
		break;

	case BW_80:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 10);
		break;

	case BW_160:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 11);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Can't find such bw, use default\n");
		dw_set[3].u4DwValue = (BW_20 << 12) | (sgi << 8);
		break;
	}

	if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD || tx_mode == MODE_VHT) {
		/* QoS enable by phymode */
		dw_set[3].u4DwValue |= (1 << 19);
	}

	/* Use phymode/mcs/nss/STBC decide RateInfo (DW6~8) */
	/* step 1. depends on ATE command tx rate, convert to 12-bits WTBL-rate value */
	/* wtbl_rate = ((STBC & 0x1) << 11) | ((Nss & 0x3)<< 9) | ((phy_mode & 0x3)  << 6) |  ((mcs & 0x3f) << 0) */
	if (tx_mode == MODE_CCK)
		wtbl_rate = asic_tx_rate_to_tmi_rate(pAd, tx_mode, mcs, (nss + 1), stbc, 1);
	else
		wtbl_rate = asic_tx_rate_to_tmi_rate(pAd, tx_mode, mcs, (nss + 1), stbc, 0);

	/* step 2. set WTBL RAW DW 6: ((rate3 & 0xff)<< 24) | ((rate2 & 0xfff) << 12) | ((rate1 & 0xfff) << 0) */
	dw_set[4].ucWhichDW = 6;
	dw_set[4].u4DwMask = 0x0;
	dw_set[4].u4DwValue = ((wtbl_rate & 0xFF) << 24)
				| ((wtbl_rate & 0xFFF) << 12)
				| ((wtbl_rate & 0xFFF) << 0);
	dw_set[5].ucWhichDW = 7;
	dw_set[5].u4DwMask = 0x0;
	dw_set[5].u4DwValue = ((wtbl_rate & 0xF) << 28)
				| ((wtbl_rate & 0xFFF) << 16)
				| ((wtbl_rate & 0xFFF) << 4)
				| (((wtbl_rate & 0xF00) >> 8) << 0);
	dw_set[6].ucWhichDW = 8;
	dw_set[6].u4DwMask = 0x0;
	dw_set[6].u4DwValue = ((wtbl_rate & 0xFFF) << 20)
				| ((wtbl_rate & 0xFFF) << 8)
				| (((wtbl_rate & 0xFF0) >> 4) << 0);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "phy_mode=%d, mcs=%d, nss=%d, stbc=%d, wtbl_rate=0x%x\n",
			  tx_mode, mcs, nss, stbc, wtbl_rate);
	ret = WtblResetAndDWsSet(pAd, ATECtrl->wcid_ref, 1, sizeof(dw_set) / sizeof(struct cmd_wtbl_dw_mask_set), dw_set);
	return ret;
}


INT32 mt_ate_ampdu_frame(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UCHAR tx_mode;
	UCHAR *tmac_info, *pheader, *payload, *frm_template;
	UINT32 txlen, hlen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;
	UINT32 new_txlen, new_hlen;
	UINT8 need_qos, need_amsdu, need_ampdu;
	UINT32 per_mpdu_len = 0;
	UINT8 ampdu_agg_cnt = 0;
	tx_mode = TESTMODE_GET_PARAM(pAd, band_idx, tx_mode);
	new_txlen = tx_time_param->pkt_tx_len;
	new_hlen = tx_time_param->pkt_hdr_len;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;

	if (tx_mode > MODE_HTGREENFIELD)
		per_mpdu_len = (MAX_VHT_MPDU_LEN - 100); /* include mac header length */
	else
		per_mpdu_len = (DEFAULT_MPDU_LEN - 100); /* include mac header length */

	tx_time_param->pkt_msdu_len = per_mpdu_len;
	ampdu_agg_cnt = new_txlen / per_mpdu_len;

	if (new_txlen % per_mpdu_len)
		ampdu_agg_cnt++;

	tx_time_param->pkt_ampdu_cnt = ampdu_agg_cnt;

	txlen = tx_time_param->pkt_msdu_len;
	hlen = tx_time_param->pkt_hdr_len;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wcid:%d, txlen/hlen/buf=%d/%d/%p, pkt_tx_len/pkt_msdu_len/pkt_ampdu_cnt=%d/%d/%d\n",
			  ATECtrl->wcid_ref, txlen, hlen, buf,
			  tx_time_param->pkt_tx_len, tx_time_param->pkt_msdu_len,
			  tx_time_param->pkt_ampdu_cnt);
	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	NdisZeroMemory(buf, ATE_TESTPKT_LEN);

	frm_template = TESTMODE_GET_PARAM(pAd, band_idx, template_frame);
	NdisMoveMemory(pheader, frm_template, hlen);
	fill_header_address(pAd, pheader, band_idx, 0);
	ret = MtATEPayloadInit(pAd, payload, txlen - hlen, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)

	if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
		INT i = 0;
		PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
		MTWF_PRINT("[TXCONTENT DUMP START]\n");
		asic_dump_tmac_info(pAd, tmac_info);
		MTWF_PRINT("[TXD RAW]: ");

		for (i = 0; i < tx_hw_hdr_len; i++)
			MTWF_PRINT("%04x", tmac_info[i]);

		MTWF_PRINT("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1));
		MTWF_PRINT("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2));
		MTWF_PRINT("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3));
		MTWF_PRINT("FC: %04x\n", *(UINT16 *)(&hdr->FC));
		MTWF_PRINT("\tFrom DS: %x\n", hdr->FC.FrDs);
		MTWF_PRINT("\tTo DS: %x\n", hdr->FC.ToDs);
		MTWF_PRINT("[CONTENT RAW]: ");

		for (i = 0; i < (txlen - hlen); i++)
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%02x", payload[i]);

		MTWF_PRINT("\n[TXCONTENT DUMP END]\n");
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	TESTMODE_SET_PARAM(pAd, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	return ret;
}


INT32 mt_ate_non_ampdu_frame(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UCHAR *tmac_info, *pheader, *payload, *frm_template;
	UINT32 txlen, hlen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;

	txlen = tx_time_param->pkt_msdu_len;
	hlen = tx_time_param->pkt_hdr_len;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wcid:%d, txlen/hlen/buf=%d/%d/%p, pkt_tx_len/pkt_msdu_len=%d/%d\n",
			  ATECtrl->wcid_ref,
			  txlen, hlen, buf,
			  tx_time_param->pkt_tx_len,
			  tx_time_param->pkt_msdu_len);
	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	NdisZeroMemory(buf, ATE_TESTPKT_LEN);

	frm_template = TESTMODE_GET_PARAM(pAd, band_idx, template_frame);
	NdisMoveMemory(pheader, frm_template, hlen);
	fill_header_address(pAd, pheader, band_idx, 0);
	ret = MtATEPayloadInit(pAd, payload, txlen - hlen, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)

	if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
		INT i = 0;
		PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
		MTWF_PRINT("[TXCONTENT DUMP START]\n");
		asic_dump_tmac_info(pAd, tmac_info);
		MTWF_PRINT("[TXD RAW]: ");

		for (i = 0; i < tx_hw_hdr_len; i++)
			MTWF_PRINT("%04x", tmac_info[i]);

		MTWF_PRINT("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1));
		MTWF_PRINT("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2));
		MTWF_PRINT("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3));
		MTWF_PRINT("FC: %04x\n", *(UINT16 *)(&hdr->FC));
		MTWF_PRINT("\tFrom DS: %x\n", hdr->FC.FrDs);
		MTWF_PRINT("\tTo DS: %x\n", hdr->FC.ToDs);
		MTWF_PRINT("[CONTENT RAW]: ");

		for (i = 0; i < (txlen - hlen); i++)
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%02x", payload[i]);

		MTWF_PRINT("\n[TXCONTENT DUMP END]\n");
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	TESTMODE_SET_PARAM(pAd, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	return ret;
}


INT32 MT_ATEGenBurstPkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	struct _RTMP_CHIP_OP *ops;
	UINT32 new_txlen, new_hlen;
	UINT8 need_qos, need_amsdu, need_ampdu;
	new_txlen = tx_time_param->pkt_tx_len;
	new_hlen = tx_time_param->pkt_hdr_len;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "new_txlen/new_hlen=%d/%d, qos/amsdu/ampdu=%d/%d/%d\n",
			  new_txlen, new_hlen, need_qos, need_amsdu, need_ampdu);

	/* Update WTBL if necessary, setup WBL through set_ampdu_wtbl hook for legacy chip */
	if (need_ampdu && wdev->wdev_ops->conn_act == NULL) {
		ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->set_ampdu_wtbl != NULL)
			ops->set_ampdu_wtbl(pAd, band_idx);
		else
			goto error_out;
	}

	if (need_ampdu) {
		ret = mt_ate_ampdu_frame(pAd, buf, band_idx);
	} else {
		/* No aggregation, directly go with specific length and through ALTX queue */
		tx_time_param->pkt_ampdu_cnt = 1;
		tx_time_param->pkt_msdu_len = new_txlen;
		ret = mt_ate_non_ampdu_frame(pAd, buf, band_idx);
	}

error_out:
	return ret;
}


INT32 MT_ATEComposePkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx, UINT16 sta_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UCHAR *tmac_info, *pheader, *payload, *template;
	UINT32 txlen, hlen, pkt_tx_time;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;

	if (!buf) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "NULL buf, band_idx:%u\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}

	/* For long packet implemetation */
	tx_time_param->pkt_tx_len = 0;
	tx_time_param->pkt_hdr_len = 0;
	tx_time_param->pkt_need_qos = 0;
	tx_time_param->pkt_need_amsdu = 0;
	tx_time_param->pkt_need_ampdu = 0;
	tx_time_param->pkt_ampdu_cnt = 0;
	pkt_tx_time = tx_time_param->pkt_tx_time;

	if (pkt_tx_time > 0) {
		UINT8 need_qos = 0, need_amsdu = 0, need_ampdu = 0;
		UINT32 new_txlen = mt_ate_get_txlen_by_pkt_tx_time(pAd, band_idx);
		UINT32 new_hlen;
		txlen = TESTMODE_GET_PARAM(pAd, band_idx, tx_len);
		hlen = TESTMODE_GET_PARAM(pAd, band_idx, hdr_len);

		if (new_txlen > 0)
			txlen = new_txlen;
		else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't get txlen by pkt tx time\n");
		}

		new_hlen = mt_ate_get_hlen_by_pkt_tx_time(pAd, band_idx, txlen, &need_qos, &need_amsdu, &need_ampdu);

		if (new_hlen > 0)
			hlen = new_hlen;
		else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Can't get hdrlen by pkt tx time\n");
		}

		tx_time_param->pkt_tx_len = new_txlen;
		tx_time_param->pkt_hdr_len = hlen;
		tx_time_param->pkt_need_qos = need_qos;
		tx_time_param->pkt_need_amsdu = need_amsdu;
		tx_time_param->pkt_need_ampdu = need_ampdu;
		/* New packet generation function */
		ret = MT_ATEGenBurstPkt(pAd, buf, band_idx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(wcid:%d)\n", ATECtrl->wcid_ref);
		template = TESTMODE_GET_PARAM(pAd, band_idx, template_frame);
		txlen = TESTMODE_GET_PARAM(pAd, band_idx, tx_len);

		/* Error check for txlen */
		if (txlen == 0) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "TX length can't be 0!!\n");
			return NDIS_STATUS_FAILURE;
		}

		tx_time_param->pkt_hdr_len = mt_ate_get_hlen_by_pkt_tx_time(pAd,
																	band_idx,
																	txlen,
																	&tx_time_param->pkt_need_qos,
																	&tx_time_param->pkt_need_amsdu,
																	&tx_time_param->pkt_need_ampdu);
		tmac_info = buf;
		pheader = (buf + tx_hw_hdr_len);
		payload = (pheader + tx_time_param->pkt_hdr_len);
		NdisZeroMemory(buf, ATE_TESTPKT_LEN);
		NdisMoveMemory(pheader, template, tx_time_param->pkt_hdr_len);
		fill_header_address(pAd, pheader, band_idx, sta_idx);
		ret = MtATEPayloadInit(pAd, payload, txlen - tx_time_param->pkt_hdr_len, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)
		if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
			INT i = 0;
			PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
			MTWF_PRINT("[TXCONTENT DUMP START]\n");
			asic_dump_tmac_info(pAd, tmac_info);
			MTWF_PRINT("[TXD RAW]: ");

			for (i = 0; i < tx_hw_hdr_len; i++)
				MTWF_PRINT("%04x", tmac_info[i]);

			MTWF_PRINT("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1));
			MTWF_PRINT("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2));
			MTWF_PRINT("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3));
			MTWF_PRINT("FC: %04x\n", *(UINT16 *)(&hdr->FC));
			MTWF_PRINT("\tFrom DS: %x\n", hdr->FC.FrDs);
			MTWF_PRINT("\tTo DS: %x\n", hdr->FC.ToDs);
			MTWF_PRINT("[CONTENT RAW]: ");

			for (i = 0; i < (txlen - tx_time_param->pkt_hdr_len); i++)
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%02x", payload[i]);

			MTWF_PRINT("\n[TXCONTENT DUMP END]\n");
		}
#endif /* !defined(COMPOS_TESTMODE_WIN) */
		TESTMODE_SET_PARAM(pAd, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

		if (IS_HIF_TYPE(pAd, HIF_MT))
			MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	}

	return ret;
}


UINT32 mt_ate_mcs32_handle(RTMP_ADAPTER *pAd, UINT16 wcid_ref, UINT8 bw)
{
	INT32 ret = 0;
	UINT32 DwMask = 0;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)

	if (IS_MT7615(pAd) || IS_MT7622(pAd) ||	IS_P18(pAd) || IS_MT7663(pAd) ||
		IS_AXE(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) ||
		IS_MT7916(pAd) || IS_MT7981(pAd)) {
		union WTBL_DW5 wtbl_txcap;
		DwMask = ~(3 <<	12); /* only update fcap bit[13:12] */
		wtbl_txcap.field.fcap = bw;
		/* WTBLDW5 */
		WtblDwSet(pAd, wcid_ref, 1, 5, DwMask, wtbl_txcap.word);
		return ret;
	}

#else
	{
		union WTBL_2_DW9 wtbl_txcap;
		DwMask = ~(3 <<	14); /* only update fcap bit[15:14] */
		wtbl_txcap.field.fcap = bw;
		/* WTBL2DW9 */
		WtblDwSet(pAd, wcid_ref, 2, 9, DwMask, wtbl_txcap.word);
	}
#endif
	return ret;
}

INT32 mt_ate_enq_pkt(RTMP_ADAPTER *pAd, UINT32 band_idx, UINT16 sta_idx)
{
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	PNDIS_PACKET pkt = NULL;
	INT32 ret = 0;
	PNDIS_PACKET pkt_skb = NULL;
	USHORT qid =  TESTMODE_GET_PARAM(pAd, band_idx, ac_idx);
	struct wifi_dev *wdev = NULL;
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, band_idx, stack);
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;

	if (sta_idx < stack->index)
		mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->mac_tbl_entry[sta_idx];
	else {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid sta_idx=%d and stack depth:%d, ignored\n", sta_idx, stack->index);
		goto done;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"band_idx:%u, ring Idx:%u\n", band_idx, qid);

	if (mac_tbl_entry) {
		if (stack->pkt_skb[sta_idx]) {
			pkt_skb = stack->pkt_skb[sta_idx];

			if (stack->wdev[sta_idx])
				wdev = (struct wifi_dev *)stack->wdev[sta_idx];
			else
				wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, band_idx, wdev[0]);

			OS_PKT_CLONE(pAd, pkt_skb, pkt, GFP_ATOMIC);
			if (pkt == NULL) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "clone pakcet fail, ignored\n");
				goto done;
			}

			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pkt, mac_tbl_entry->wcid);
			/* Set the packet type to ATE frame before enqueue packet, make this packet handled by mt_ate_tx */
			RTMP_SET_PACKET_TXTYPE(pkt, TX_ATE_FRAME);

			if ((pkt_tx_time > 0) && (need_ampdu)) {
				RTMP_SET_PACKET_TYPE(pkt, TX_DATA);
				RTMP_SET_PACKET_QUEIDX(pkt, QID_AC_BE);
			} else {
				RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
				RTMP_SET_PACKET_QUEIDX(pkt, 0);
			}

			ret = send_mlme_pkt(pAd, pkt, wdev, qid, TRUE);
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "packet for index[%d] is invalid, ignored\n", sta_idx);
			ret = -1;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "mac_tbl_entry for index[%d] is invalid, ignored\n", sta_idx);
		ret = -1;
	}
done:
	return ret;
}

static UINT32 MT_ATEGetBandIdx(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UINT32 band_idx = 0;
	UCHAR wdev_idx = 0;
	struct wifi_dev *wdev = NULL;

	wdev_idx = RTMP_GET_PACKET_WDEV(pkt);

	if (wdev_idx >= WDEV_NUM_MAX)
		goto err0;

	wdev = pAd->wdev_list[wdev_idx];

	if (!wdev)
		goto err0;

	band_idx = HcGetBandByChannel(pAd, wdev->channel);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"wdev_idx:%x\n", wdev_idx);
	return band_idx;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"cannot find wdev by idx:%x\n", wdev_idx);
	return -1;
}

INT32 MT_ATETxControl(RTMP_ADAPTER *pAd, UINT32 band_idx, PNDIS_PACKET pkt)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 txdone_cnt = 0, tx_cnt = 0, mode = 0;

	if (ATECtrl->verify_mode == ATE_LOOPBACK) {
		if (pAd->LbCtrl.LoopBackWaitRx) {
#ifdef RTMP_PCI_SUPPORT

			if (IS_PCI_INF(pAd))
				RTMP_OS_COMPLETE(&pAd->LbCtrl.LoopBackPCITxEvent);
			else {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Not supported in this interface yet\n");
			}

#else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Not supported in this interface yet\n");
#endif
		}
	} else if (ATECtrl->verify_mode == HQA_VERIFY) {
		/* Need to get band_idx first if free token done */
		if (pkt)
			band_idx = MT_ATEGetBandIdx(pAd, pkt);

		if (band_idx > TESTMODE_BAND1) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Wrong band_idx %u\n", band_idx);
			goto done;
		}

		txdone_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TXDONE_CNT);
		tx_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TX_CNT);
		mode = TESTMODE_GET_PARAM(pAd, band_idx, op_mode);

		/* Do not count in packet number when TX is not in start stage */
		if (!(mode & ATE_TXFRAME))
			return ret;

		/* Triggered when RX tasklet free token */
		if (pkt) {
			struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, band_idx, stack);

			if (stack->quota < MGMT_QUE_MAX_NUMS/2)
				stack->quota++;
			pAd->RalinkCounters.KickTxCount++;
			txdone_cnt++;
			TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXDONE_CNT, txdone_cnt);
		}

#ifdef ATE_TXTHREAD
		TESTMODEThreadProceedTx(pAd, band_idx);
#else

		if ((mode & ATE_TXFRAME) && (txdone_cnt < tx_cnt))
			ret = mt_ate_enq_pkt(pAd, band_idx);
		else if ((mode & ATE_TXFRAME) &&
				 (txdone_cnt == tx_cnt)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "All Tx is done\n");

			if (mode & fATE_MPS) {
				RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(pAd, band_idx, tx_wait);
				RTMP_OS_COMPLETE(tx_wait);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Finish one MPS Item\n");
			}

			/* Tx status enters idle mode */
			TESTMODE_SET_PARAM(pAd, band_idx, TxStatus, 0);
		} else if (!(mode & ATE_TXFRAME)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Stop TX bottom is pressed\n");

			if (mode & fATE_MPS) {
				RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(pAd, band_idx, tx_wait);
				mode &= ~fATE_MPS;
				TESTMODE_SET_PARAM(pAd, band_idx, op_mode, mode);
				RTMP_OS_COMPLETE(tx_wait);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MPS STOP\n");
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "ATE:: DO NOT match any condition, Mode:0x%x, TxCnt:%u, TxDone:%u\n", mode, tx_cnt, txdone_cnt);
		}

		TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXDONE_CNT, txdone_cnt);
#endif
	}

done:
	return ret;
}


static INT32 mt_ate_calculate_duty_cycle(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	UINT32 ipg, pkt_tx_time, duty_cycle;
	ipg = ipg_param->ipg;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	duty_cycle = TESTMODE_GET_PARAM(pAd, band_idx, duty_cycle);

	/* Calculate needed ipg/pkt_tx_time and duty_cycle */
	if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg == 0)) {
		/* TODO: need to consider this case in the future */
		duty_cycle = 0;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "There are no pkt_tx_time/ipg!!\n"
				 "Use default transmission setting and set duty_cycle=%d\n",
				  duty_cycle);
	} else if ((duty_cycle > 0) && (pkt_tx_time > 0) && (ipg == 0)) {
		ipg = ((pkt_tx_time * 100) / duty_cycle) - pkt_tx_time;
		ipg_param->ipg = ipg;
		/* If IPG value is not make sense, there's error handle when get ipg parameter */
		ret = ATEOp->SetIPG(pAd, ipg);
	} else if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg > 0)) {
		/* If pkt_tx_time is not make sense, there's error handle when start TX */
		pkt_tx_time = (duty_cycle * ipg) / (100 - duty_cycle);
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Already existed pkt_tx_time/ipg, can't set duty_cycle!!\n"
				 "Expected duty_cycle=%d%%\n", duty_cycle);
		duty_cycle = (pkt_tx_time * 100) / (pkt_tx_time + ipg);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Real duty_cycle=%d%%\n", duty_cycle);
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "duty_cycle=%d%%, ipg=%dus, pkt_tx_time=%dus\n",
			  duty_cycle, ipg, pkt_tx_time);
	tx_time_param->pkt_tx_time = pkt_tx_time;
	TESTMODE_SET_PARAM(pAd, band_idx, duty_cycle, duty_cycle);
	return ret;
}

#if defined(CUT_THROUGH) || defined(DOT11_HE_AX)
static INT32 mt_ate_set_wmm_param_by_qid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = 0;
	UINT32 band_idx = HcGetBandByWdev(wdev);
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	UCHAR WmmIdx;
	USHORT ac_idx;
	UINT16 slot_time, sifs_time, cw;
	UINT8 ac_num, aifsn;
#if !defined(COMPOS_TESTMODE_WIN)
	WmmIdx = HcGetWmmIdx(pAd, wdev);

	if (WmmIdx > 3) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev_idx=%d, invalid WmmIdx=%d, reset to 0!\n", wdev->wdev_idx, WmmIdx);
		WmmIdx = 0xFF;
	}

#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "wdev_idx=%d, WmmIdx=%d\n", wdev->wdev_idx, WmmIdx);
	ac_idx = TESTMODE_GET_PARAM(pAd, band_idx, ac_idx);

	if ((ac_idx != QID_AC_BE) && (ac_idx != TxQ_IDX_ALTX0) && (ac_idx != TxQ_IDX_ALTX1)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Impossible!\n");
		return FALSE;
	}

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	ac_num = ac_idx;
	aifsn = ipg_param->aifsn;
	cw = ipg_param->cw;
#ifdef WIFI_UNIFIED_COMMAND
	if (AsicSetWmmParam(pAd, wdev, WmmIdx, (UINT32)ac_num, WMM_PARAM_AIFSN, (UINT32)aifsn))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"AsicSetWmmParam of aifsn return 1\n");
	if (AsicSetWmmParam(pAd, wdev, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMIN, (UINT32)cw))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"AsicSetWmmParam of CWMIN return 1\n");
	ret = AsicSetWmmParam(pAd, wdev, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMAX, (UINT32)cw);
#else
	if (AsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_AIFSN, (UINT32)aifsn))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"AsicSetWmmParam of aifsn return 1\n");
	if (AsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMIN, (UINT32)cw))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"AsicSetWmmParam of CWMIN return 1\n");
	ret = AsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMAX, (UINT32)cw);
#endif /* WIFI_UNIFIED_COMMAND */
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "qid=%d, slot_time=%d, sifs_time=%d, ac_num=%d, aifsn=%d, cw=%d\n",
			  ac_num, slot_time, sifs_time, ac_num, aifsn, cw);
	return ret;
}

static INT32 mt_ate_apply_ipg_param(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	UINT32 ipg;
	UINT16 slot_time, sifs_time, cw;
	UINT8 aifsn;
	ipg = ipg_param->ipg;

	if (ipg > 0) {
		/* Get packet qIdx and decide which CR set need to be changed */
		slot_time = ipg_param->slot_time;
		sifs_time = ipg_param->sifs_time;
		aifsn = ipg_param->aifsn;
		cw = ipg_param->cw;
	} else {
		/* Write default value back to HW */
		slot_time = DEFAULT_SLOT_TIME;
		sifs_time = DEFAULT_SIFS_TIME;
		aifsn = MIN_AIFSN;
		cw = 0;
	}

	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;
	if (chip_ops->set_ifs)
		chip_ops->set_ifs(pAd, band_idx);

#ifdef CONFIG_HW_HAL_OFFLOAD
	ret = MtCmdATESetSlotTime(pAd, (UINT8)slot_time, (UINT8)sifs_time, RIFS_TIME, EIFS_TIME, (UINT8)band_idx);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	   "ipg=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
		ipg, slot_time, sifs_time, aifsn, cw);
	if (ret)
		return ret;
#endif
	ret = mt_ate_set_wmm_param_by_qid(pAd, wdev);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ipg=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
			  ipg, slot_time, sifs_time, aifsn, cw);
	return ret;
}
#endif

static INT32 mt_ate_apply_spe_antid(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	UCHAR ant_pri = 0, spe_idx = 0, mac_tbl_idx = 0;
	INT32 ret = 0;
	UINT32 ant_sel = TESTMODE_GET_PARAM(pAd, band_idx, tx_ant);
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, band_idx, stack);
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
#if defined(DOT11_HE_AX)
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#endif	/* DOT11_HE_AX */
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#if defined(MT_MAC)
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (ant_sel & ATE_ANT_USER_SEL) {
			ant_sel &= ~ATE_ANT_USER_SEL;
			spe_idx = ant_sel;
		} else {
			INT map_idx = 0;

			if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
				ant_sel >>= 2*band_idx;

			for (map_idx = 0; map_idx < ARRAY_SIZE(ant_to_spe_idx_map); map_idx++) {
				if (ant_sel == ant_to_spe_idx_map[map_idx].ant_sel)
					break;
			}

			if (map_idx == ARRAY_SIZE(ant_to_spe_idx_map))
				spe_idx = 0;
			else
				spe_idx = ant_to_spe_idx_map[map_idx].spe_idx;
		}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
		if (g_EBF_certification) {
			if (BF_ON_certification) {
				ant_pri = 0;
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Info.AntPri = 0\n");
			} else {
				ant_pri = 24;  /* 24 */
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Info.AntPri = 24\n");
			}
		}
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
	} else
#endif /* defined(MT_MAC) */
	{
		switch (ant_sel) {
		case 0: /* Both */
			ant_pri = 0;
			spe_idx = 1;
			break;

		case 1: /* TX0 */
			ant_pri = 0;
			spe_idx = 0;
			break;

		case 2: /* TX1 */
			ant_pri = 2; /* b'010 */
			spe_idx = 0;
			break;
		}
	}

	/* store SPE index to TXD and WTBL */
	while (stack->mac_tbl_entry[mac_tbl_idx] && (mac_tbl_idx < (stack->index))) {
		mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->mac_tbl_entry[mac_tbl_idx];

		mac_tbl_entry->phy_param.ant_pri = ant_pri;
		mac_tbl_entry->phy_param.spe_idx = spe_idx;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

		rRaParam.ucSpeEn = spe_idx;
		rRaParam.u4Field = RA_PARAM_SPE_UPDATE;
		RAParamUpdate(pAd, mac_tbl_entry, &rRaParam);
#endif	/* RACTRL_FW_OFFLOAD_SUPPORT */
		mac_tbl_idx++;
	}
#if defined(DOT11_HE_AX)
	if (chip_dbg->chip_ctrl_spe)
		chip_dbg->chip_ctrl_spe(pAd, band_idx, TESTMODE_GET_PARAM(pAd, band_idx, tx_mode), spe_idx);
#endif /* DOT11_HE_AX */

	return ret;
}

#ifdef SINGLE_SKU_V2
static INT32 mt_ate_apply_pwr_offset(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	CHAR pwr_offset = 0;
	INT32 ret = 0, mac_tbl_idx = 0;
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, band_idx, stack);
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->mac_tbl_entry[0];
	struct phy_params *phy_info = &mac_tbl_entry->phy_param;
	BOOLEAN fgSPE;

	if (phy_info->ant_pri >= 24)
		fgSPE = TRUE;
	else
		fgSPE = FALSE;

	/* Update Power offset according to Band, Phymode, MCS, BW, Nss, SPE */
	pwr_offset = SKUTxPwrOffsetGet(pAd, band_idx, TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), per_pkt_bw),
					TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), tx_mode),
					TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), mcs),
					TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), nss), fgSPE);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Info.PowerOffset = 0x%x (%d)\n",
		pwr_offset, pwr_offset);

	if (pwr_offset < -16)
		pwr_offset = -16;
	else if (pwr_offset > 15)
		pwr_offset = 15;

	while (stack->mac_tbl_entry[mac_tbl_idx] && (mac_tbl_idx < stack->index)) {
		mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->mac_tbl_entry[mac_tbl_idx];
		phy_info = &mac_tbl_entry->phy_param;
		phy_info->pwr_offset = pwr_offset;

		mac_tbl_idx++;
	}

	return ret;
}
#endif

NDIS_STATUS mt_ate_store_tx_info(struct _RTMP_ADAPTER *ad,
						UCHAR band_idx, struct wifi_dev *wdev,
						UINT_8 *da,
						struct _MAC_TABLE_ENTRY *mac_tbl_entry,
						struct _ATE_TX_INFO *ate_tx_info)
{
	INT32 ret = NDIS_STATUS_SUCCESS, sta_idx = 0;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(ad, band_idx, test_pkt);

	if (pate_pkt == NULL) {
		ret = -1;
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Invalid pre-allocated buffer for MPDU\n");

		goto err_out;
	}

	if (is_mt_ate_mac_tbl_stack_full(ad, band_idx) == FALSE) {
		if (ate_tx_info) {
			if (mac_tbl_entry) {
				struct phy_params *phy_info = &mac_tbl_entry->phy_param;

				NdisZeroMemory(phy_info, sizeof(*phy_info));

				phy_info->phy_mode = ate_tx_info->tx_mode;
#ifdef TXBF_SUPPORT
				phy_info->tx_ibf = ate_tx_info->ibf;
				phy_info->tx_ebf = ate_tx_info->ebf;
#endif
				phy_info->stbc = ate_tx_info->stbc;
				phy_info->ldpc = ate_tx_info->ldpc;
				phy_info->bw = ate_tx_info->bw;
				phy_info->vht_nss = ate_tx_info->nss;
				phy_info->gi_type = ate_tx_info->gi;
				phy_info->ltf_type = ate_tx_info->ltf;

#if defined(DOT11_HE_AX)
				if (phy_info->phy_mode > MODE_VHT) {
					phy_info->rate = (ate_tx_info->mcs & 0xf);
					phy_info->dcm = (ate_tx_info->mcs & BIT(5)) ? TRUE : FALSE;	/* b'5 for DCM */

					if (phy_info->phy_mode == MODE_HE_EXT_SU)
						phy_info->su_ext_tone = (ate_tx_info->mcs & BIT(4)) ? TRUE : FALSE;	/* b'4 for tone*/
				} else
#endif /* DOT11_HE_AX */
				{
					phy_info->rate = (ate_tx_info->mcs & 0x1f);
				}

				/* tricky point, why change rate and preamble here? */
				if (phy_info->phy_mode == MODE_CCK) {
					OPSTATUS_CLEAR_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);

					if (phy_info->rate > MCS_8) {
						OPSTATUS_SET_FLAG(ad, fOP_STATUS_SHORT_PREAMBLE_INUSED);
						phy_info->rate -= MCS_9;
					}
				}

				if ((phy_info->phy_mode < MODE_HE) && (ate_tx_info->mcs & 0x7f) == 32)
					mt_ate_mcs32_handle(ad, mac_tbl_entry->wcid, ate_tx_info->bw);

				if (wdev_do_conn_act(wdev, mac_tbl_entry) != TRUE) {
					ret = -1;
					MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"(): connect fail!!\n");
					goto err_out;
				}

			} else {
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Insert mac_table_entry failed\n");
			}
		}

		sta_idx = mt_ate_push_mac_tbl_entry(ad, band_idx, wdev, da, mac_tbl_entry);

		if (sta_idx > -1) {
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
			UINT8 tx_hw_hdr_len = cap->TXWISize;
			UCHAR *pate_pkt = TESTMODE_GET_PARAM(ad, band_idx, test_pkt);
			struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(ad, band_idx, tx_time_param);
			UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
			UINT32 txlen = (ate_tx_info) ? ate_tx_info->mpdu_length : TESTMODE_GET_PARAM(ad, band_idx, tx_len);
			struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);

			if (MT_ATEComposePkt(ad, pate_pkt, band_idx, sta_idx))
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"MT_ATEComposePkt fail\n");

			if (pkt_tx_time > 0)
				txlen = tx_time_param->pkt_msdu_len;

			if (stack->pkt_skb[sta_idx]) {
				RELEASE_NDIS_PACKET(ad, stack->pkt_skb[sta_idx], NDIS_STATUS_SUCCESS);
				stack->pkt_skb[sta_idx] = NULL;
			}

			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "Alloc pkt, txlen=%d, tx_hw_hdr_len=%d, total=%d\n", txlen, tx_hw_hdr_len, txlen + tx_hw_hdr_len);
			ret = RTMPAllocateNdisPacket(ad, &stack->pkt_skb[sta_idx], NULL, 0, pate_pkt, txlen + tx_hw_hdr_len);

			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AllocateNdisPacket fail\n");
				goto err_out;
			}
		} else {
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"mac_table_entry stored failed\n");
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Stack is full!\n");
	}

err_out:
	return ret;
}


BOOLEAN mt_ate_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, void *tx_blk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	TX_BLK *pTxBlk = (TX_BLK *)tx_blk;

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) || (pTxBlk->TxFrameType == TX_AMSDU_FRAME) || (pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;
}


BOOLEAN mt_ate_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, void *tx_blk)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	TX_BLK *pTxBlk = (TX_BLK *)tx_blk;
	UINT8 tx_mode = TESTMODE_GET_PARAM(pAd, HcGetBandByWdev(wdev), tx_mode);

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	pTxBlk->pSrcBufHeader += cap->TXWISize;
	pTxBlk->SrcBufLen -= cap->TXWISize;	/* Due to testmode allocate size include TXWISize */

	TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	/* testmode data does not support fTX_HDR_TRANS yet
	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) || (pTxBlk->TxFrameType == TX_AMSDU_FRAME) || (pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}
	*/

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);

	if ((TESTMODE_GET_PARAM(pAd, HcGetBandByWdev(wdev), retry) != 0) && (tx_mode == MODE_HE_MU))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bRetryUnlimit);
	else
		TX_BLK_SET_FLAG(pTxBlk, fTX_bNoRetry);

	pTxBlk->UserPriority = 0;

	/*	no frag */
	pTxBlk->FragIdx = 0;
	/* no protection */
	SET_CIPHER_NONE(pTxBlk->CipherAlg);
	return TRUE;
}


INT32 mt_ate_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	INT32 ret = 0;
	UINT32 band_idx = HcGetBandByWdev(wdev);
	TMAC_INFO tmac_info;
	PQUEUE_ENTRY q_entry;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _ATE_CHIP_OPERATION *ate_chip_ops = pAd->ATECtrl.ate_chip_ops;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	RTMP_SET_PACKET_WCID(tx_blk->pPacket, 0);

	/* Fill TX blk for ATE mode */
	ate_chip_ops->fill_tx_blk(pAd, wdev, tx_blk);

	/* TMAC_INFO setup for ATE mode */
	ret = mt_ate_set_tmac_info(pAd, &tmac_info, band_idx);

	if (ret)
		return ret;

	return arch_ops->ate_hw_tx(pAd, &tmac_info, tx_blk);
}


INT32 mt_ate_tx_v2(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	PQUEUE_ENTRY q_entry;
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct _ATE_CHIP_OPERATION *ate_chip_ops = ATECtrl->ate_chip_ops;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, HcGetBandByWdev(wdev), tx_time_param);
	UINT8 need_qos = tx_time_param->pkt_need_qos;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);

	if (need_qos) {
		tx_blk->wifi_hdr_len = (UINT8) tx_time_param->pkt_hdr_len;
		tx_blk->MpduHeaderLen = (UINT8) tx_time_param->pkt_hdr_len;
	} else {
		tx_blk->wifi_hdr_len = (UINT8) LENGTH_802_11;
		tx_blk->MpduHeaderLen = (UINT8) LENGTH_802_11;
	}

	if (need_ampdu) {
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAckRequired);
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteAgg);
		tx_blk->HdrPadLen = 2;
	} else {
		TX_BLK_SET_FLAG(tx_blk, fTX_ForceRate);
		TX_BLK_CLEAR_FLAG(tx_blk, fTX_bAckRequired);
		tx_blk->HdrPadLen = 0;
	}

	/* Fill TX blk for ATE mode */
	ret = ate_chip_ops->fill_tx_blk(pAd, wdev, tx_blk);

	tx_blk->QueIdx = TESTMODE_GET_PARAM(pAd, HcGetBandByWdev(wdev), ac_idx);
	if (mt_ate_search_mac_tbl_entry(pAd, HcGetBandByWdev(wdev), RTMP_GET_PACKET_WCID(tx_blk->pPacket), &tx_blk->pMacEntry) < 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wcid:%d is in-valid in stack!\n", RTMP_GET_PACKET_WCID(tx_blk->pPacket));

		return NDIS_STATUS_FAILURE;
	}
#if defined(CONFIG_AP_SUPPORT)
	if (tx_blk->pMacEntry)
		tx_blk->pMbss = tx_blk->pMacEntry->pMbss;
#endif
	if (ATECtrl->txs_enable)
		TX_BLK_SET_FLAG(tx_blk, fTX_bAteTxsRequired);

	return arch_ops->hw_tx(pAd, tx_blk);
}



static UINT8 mt_ate_translate_gi(UINT8 tx_mode, UINT32 ltf_gi)
{
	UINT8 gi_type = 0;

	if (tx_mode == MODE_HE_SU || tx_mode == MODE_HE_EXT_SU || tx_mode == MODE_HE_MU) {
		switch (ltf_gi) {
		case 1:
		case 2:
			gi_type = GI_8+(ltf_gi-1);
			break;
		case 3:
		case 4:
			gi_type = GI_8+((ltf_gi-3) ? 0:2);
			break;
		default:
			gi_type = GI_8;
			break;
		}
	} else if (tx_mode == MODE_HE_TRIG) {
		switch (ltf_gi) {
		case 2:
			gi_type = GI_32;
			break;
		default:
			gi_type = GI_16;
			break;
		}
	} else	/* for non-HE PPDU types, gi equavalent to Sgi. */
		gi_type = ltf_gi;

	return gi_type;
}

static UINT8 mt_ate_translate_ltf(UINT8 tx_mode, UINT32 ltf_gi)
{
	UINT8 ltf_type = 0;

	if (tx_mode == MODE_HE_SU || tx_mode == MODE_HE_EXT_SU) {
		switch (ltf_gi) {
		case 1:
		case 2:
			ltf_type = HE_LTF_X2;
			break;
		case 3:
		case 4:
			ltf_type = HE_LTF_X4;
			break;
		default:
			ltf_type = HE_LTF_X1;
			break;
		}
	} else if (tx_mode == MODE_HE_MU) {
		switch (ltf_gi) {
		case 0:
		case 3:
			ltf_type = HE_LTF_X4;
			break;
		default:
			ltf_type = HE_LTF_X2;
			break;
		}
	} else if (tx_mode == MODE_HE_TRIG) {
		ltf_type = ltf_gi;
	} else	/* for non-HE PPDU types, ltf is not required. */
		ltf_type = 0;

	return ltf_type;
}


static INT32 mt_ate_tx_subscribe(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	struct _ATE_TX_INFO ate_tx_info;
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(ad), *tx_method = NULL;
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	UINT8 tx_mode = TESTMODE_GET_PARAM(ad, control_band_idx, tx_mode);
	struct wifi_dev *wdev = NULL;
	UINT8 *da = TESTMODE_GET_PARAM(ad, control_band_idx, addr1[0]);
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(ad, control_band_idx, tx_time_param);
	UINT32 duty_cycle = TESTMODE_GET_PARAM(ad, control_band_idx, duty_cycle);

	if (!is_mt_ate_mac_tbl_stack_empty(ad, control_band_idx)) {
		mt_ate_tx_unsubscribe(ad);

		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Preiously stored TX information for band[%d] will be flushed!\n", control_band_idx);
	}

	NdisZeroMemory(&ate_tx_info, sizeof(ate_tx_info));
	tx_method = TESTMODE_GET_PARAM(ad, control_band_idx, tx_method);

	/* Calculate duty_cycle related parameter first */
	if (duty_cycle > 0)
		mt_ate_calculate_duty_cycle(ad, control_band_idx);

	ate_tx_info.tx_mode = tx_mode;
	ate_tx_info.bw = TESTMODE_GET_PARAM(ad, control_band_idx, per_pkt_bw);
	ate_tx_info.stbc = TESTMODE_GET_PARAM(ad, control_band_idx, stbc);
	ate_tx_info.ldpc = TESTMODE_GET_PARAM(ad, control_band_idx, ldpc);
	ate_tx_info.mpdu_length = TESTMODE_GET_PARAM(ad, control_band_idx, tx_len);
	ate_tx_info.gi = mt_ate_translate_gi(tx_mode, TESTMODE_GET_PARAM(ad, control_band_idx, sgi));
	ate_tx_info.ltf = mt_ate_translate_ltf(tx_mode, TESTMODE_GET_PARAM(ad, control_band_idx, sgi));
	ate_tx_info.ibf = TESTMODE_GET_PARAM(ad, control_band_idx, ibf);
	ate_tx_info.ebf = TESTMODE_GET_PARAM(ad, control_band_idx, ebf);

	if (tx_method[tx_mode])
		wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(ad, control_band_idx, wdev[1]);
	else
		wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(ad, control_band_idx, wdev[0]);

	if (tx_mode < MODE_HE_TRIG) {
#if defined(CONFIG_AP_SUPPORT) && defined(CFG_SUPPORT_FALCON_MURU)
		if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad))
			wifi_test_muru_set_arb_op_mode(ad, 0x5);
#endif
		ate_tx_info.mcs = TESTMODE_GET_PARAM(ad, control_band_idx, mcs);
		ate_tx_info.nss = TESTMODE_GET_PARAM(ad, control_band_idx, nss);

		mac_tbl_entry = MacTableInsertEntry(ad, da, wdev, ENTRY_ATE, OPMODE_ATE, TRUE);
		mt_ate_store_tx_info(ad, control_band_idx, wdev, da, mac_tbl_entry, &ate_tx_info);
	} else {
		UINT8 ru_sta_idx = 0;
		struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, control_band_idx, ru_info_list[0]);

		if (tx_mode == MODE_VHT_MIMO)
			ate_tx_info.tx_mode = MODE_VHT;

		for (ru_sta_idx = 0 ; ru_sta_idx < MAX_MULTI_TX_STA ; ru_sta_idx++) {
			da = TESTMODE_GET_PARAM(ad, control_band_idx, addr1[ru_sta_idx]);
			if (ru_sta[ru_sta_idx].valid) {
				ate_tx_info.mcs = ru_sta[ru_sta_idx].rate;
				ate_tx_info.nss = ru_sta[ru_sta_idx].nss;
				ate_tx_info.ldpc = ru_sta[ru_sta_idx].ldpc;
				if (ru_sta[ru_sta_idx].mpdu_length > QOS_MAC_HDR_LEN)
					ate_tx_info.mpdu_length = ru_sta[ru_sta_idx].mpdu_length;

				mac_tbl_entry = MacTableInsertEntry(ad, da, wdev, ENTRY_ATE, OPMODE_ATE, TRUE);
				if (ru_sta[ru_sta_idx].aid && mac_tbl_entry)
					mac_tbl_entry->Aid = ru_sta[ru_sta_idx].aid;
				mt_ate_store_tx_info(ad, control_band_idx, wdev, da, mac_tbl_entry, &ate_tx_info);
			}
		}

#if defined(CONFIG_AP_SUPPORT) && defined(CFG_SUPPORT_FALCON_MURU)
		if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad)) {
#ifdef WIFI_UNIFIED_COMMAND
			struct UNI_MURU_MANUAL_CONFIG_T UniMuruManCfg;
			uint8_t i;
			RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
			CMD_MURU_MANCFG_INTERFACER MuruManCfg;
			struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, control_band_idx, stack);
			struct _ATE_RU_ALLOCATION *ru_allocation = (struct _ATE_RU_ALLOCATION *)TESTMODE_GET_PADDR(ad, control_band_idx, ru_alloc);
			mac_tbl_entry = stack->mac_tbl_entry[0];

#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support) {
				os_zero_mem(&UniMuruManCfg, sizeof(UniMuruManCfg));

				ate_tx_info.mcs = TESTMODE_GET_PARAM(ad, control_band_idx, mcs);

				UniMuruManCfg.rCfgCmm.u1PpduFmt = MURU_PPDU_HE_MU;
				UniMuruManCfg.u4ManCfgBmpCmm = MURU_FIXED_CMM_PPDU_FMT;
				UniMuruManCfg.rCfgCmm.u1Band = control_band_idx;
				UniMuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_BAND;
				UniMuruManCfg.rCfgCmm.u1WmmSet = HcGetWmmIdx(ad, wdev);
				UniMuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;
				UniMuruManCfg.rCfgDl.u1Bw = ate_tx_info.bw;
				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_BW;
				UniMuruManCfg.rCfgDl.u1SigMcs = (ate_tx_info.mcs & 0xf);
				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
				UniMuruManCfg.rCfgDl.u1SigDcm = ((ate_tx_info.mcs & BIT5) ? 0x1 : 0);
				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
				UniMuruManCfg.rCfgDl.u1TxMode = ate_tx_info.tx_mode;
				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TX_MODE;
				UniMuruManCfg.rCfgDl.u1UserCnt = stack->index;
				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;

				for (i = 0; i < 8; i++)
					UniMuruManCfg.rCfgDl.au2RU[i] = ru_allocation->allocation[i];

				UniMuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
				UniMuruManCfg.rCfgDl.u1GI = ate_tx_info.gi;
				UniMuruManCfg.rCfgDl.u1Ltf = ate_tx_info.ltf;
				UniMuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_GI | MURU_FIXED_LTF);
				for (ru_sta_idx = 0 ; ru_sta_idx < MAX_MULTI_TX_STA ; ru_sta_idx++) {
					if (ru_sta[ru_sta_idx].valid) {
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u2WlanIdx = mac_tbl_entry[ru_sta_idx].wcid;
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1RuAllocBn = (ru_sta[ru_sta_idx].ru_index & 0x1);
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1RuAllocIdx = (ru_sta[ru_sta_idx].ru_index >> 1);
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Mcs = (mac_tbl_entry[ru_sta_idx].phy_param.rate & 0xf);
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u2TxPwrAlpha = (ru_sta[ru_sta_idx].alpha & 0xffff);
						if (mac_tbl_entry[ru_sta_idx].phy_param.rate & BIT5)	/* DCM required */
							UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Mcs |= BIT4;
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Nss = mac_tbl_entry[ru_sta_idx].phy_param.vht_nss-1;
						UniMuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Ldpc = mac_tbl_entry[ru_sta_idx].phy_param.ldpc;

						if ((ru_sta[ru_sta_idx].ru_index >> 1) == 18)
							UniMuruManCfg.rCfgDl.au1C26[(ru_sta[ru_sta_idx].ru_index & 1)] = 1;
					}
				}
				UniMuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_USER_WLAN_ID | MURU_FIXED_USER_COD | MURU_FIXED_USER_MCS | MURU_FIXED_USER_NSS | MURU_FIXED_USER_RU_ALLOC
					| MURU_FIXED_USER_PWR_ALPHA);

				wifi_test_muru_set_arb_op_mode(ad, 0x2);
				UniCmdMuruParameterSet(ad, (RTMP_STRING *)&UniMuruManCfg, UNI_CMD_MURU_MANUAL_CONFIG);
			} else {
#endif
				os_zero_mem(&MuruManCfg, sizeof(MuruManCfg));

				ate_tx_info.mcs = TESTMODE_GET_PARAM(ad, control_band_idx, mcs);

				MuruManCfg.rCfgCmm.u1PpduFmt = MURU_PPDU_HE_MU;
				MuruManCfg.u4ManCfgBmpCmm = MURU_FIXED_CMM_PPDU_FMT;
				MuruManCfg.rCfgCmm.u1Band = control_band_idx;
				MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_BAND;
				MuruManCfg.rCfgCmm.u1WmmSet = HcGetWmmIdx(ad, wdev);
				MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;
				MuruManCfg.rCfgDl.u1Bw = ate_tx_info.bw;
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_BW;
				MuruManCfg.rCfgDl.u1SigBMcs = (ate_tx_info.mcs & 0xf);
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
				MuruManCfg.rCfgDl.u1SigBDcm = ((ate_tx_info.mcs & BIT5) ? 0x1 : 0);
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
				MuruManCfg.rCfgDl.u1TxMode = ate_tx_info.tx_mode;
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TX_MODE;
				MuruManCfg.rCfgDl.u1UserCnt = stack->index;
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
				os_move_mem(MuruManCfg.rCfgDl.au1RU, ru_allocation, sizeof(*ru_allocation));
				MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
				MuruManCfg.rCfgDl.u1GI = ate_tx_info.gi;
				MuruManCfg.rCfgDl.u1Ltf = ate_tx_info.ltf;
				MuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_GI | MURU_FIXED_LTF);
				for (ru_sta_idx = 0 ; ru_sta_idx < MAX_MULTI_TX_STA ; ru_sta_idx++) {
					if (ru_sta[ru_sta_idx].valid) {
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u2WlanIdx = mac_tbl_entry[ru_sta_idx].wcid;
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1RuAllocBn = (ru_sta[ru_sta_idx].ru_index & 0x1);
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1RuAllocIdx = (ru_sta[ru_sta_idx].ru_index >> 1);
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Mcs = (mac_tbl_entry[ru_sta_idx].phy_param.rate & 0xf);
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u2TxPwrAlpha = (ru_sta[ru_sta_idx].alpha & 0xffff);
						if (mac_tbl_entry[ru_sta_idx].phy_param.rate & BIT5)	/* DCM required */
							MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Mcs |= BIT4;
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Nss = mac_tbl_entry[ru_sta_idx].phy_param.vht_nss-1;
						MuruManCfg.rCfgDl.arUserInfoDl[ru_sta_idx].u1Ldpc = mac_tbl_entry[ru_sta_idx].phy_param.ldpc;

						if ((ru_sta[ru_sta_idx].ru_index >> 1) == 18)
							MuruManCfg.rCfgDl.au1C26[(ru_sta[ru_sta_idx].ru_index & 1)] = 1;
					}
				}
				MuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_USER_WLAN_ID | MURU_FIXED_USER_COD | MURU_FIXED_USER_MCS | MURU_FIXED_USER_NSS | MURU_FIXED_USER_RU_ALLOC
					| MURU_FIXED_USER_PWR_ALPHA);

				wifi_test_muru_set_arb_op_mode(ad, 0x2);
				wifi_test_muru_set_manual_config(ad, &MuruManCfg);
#ifdef WIFI_UNIFIED_COMMAND
			}
#endif /* WIFI_UNIFIED_COMMAND */
		}
#endif
	}

#if defined(MT_DMAC) || defined(MT_FMAC)
	if  (tx_time_param->pkt_tx_time == 0)
		tx_time_param->pkt_ampdu_cnt = 1;

	if (chip_ops->set_ba_limit)
		chip_ops->set_ba_limit(ad, HcGetWmmIdx(ad, wdev), tx_time_param->pkt_ampdu_cnt, control_band_idx);

	if (chip_ops->pause_ac_queue)
		chip_ops->pause_ac_queue(ad, 0xf);
#endif /* defined(MT_DMAC) || defined(MT_FMAC) */

	Ret = mt_ate_apply_spe_antid(ad, control_band_idx);

#ifdef SINGLE_SKU_V2
	mt_ate_apply_pwr_offset(ad, control_band_idx);
#endif

	return Ret;
}

static INT32 mt_ate_tx_unsubscribe(RTMP_ADAPTER *ad)
{
	INT32 ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(ad);
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;

	while (is_mt_ate_mac_tbl_stack_empty(ad, control_band_idx) == FALSE) {
		UINT8 *da = NULL;
		mt_ate_pop_mac_tbl_entry(ad, control_band_idx, &da, &mac_tbl_entry);

		MacTableDeleteEntry(ad, mac_tbl_entry->wcid, da);
	}

#if defined(CONFIG_AP_SUPPORT) && defined(CFG_SUPPORT_FALCON_MURU)
	if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad))
		wifi_test_muru_set_arb_op_mode(ad, 0x1);
#endif

	return ret;
}

#if !defined(COMPOS_TESTMODE_WIN)	/* 1todo too many OS private function */
static INT32 MT_ATEStartTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	UINT32 round = TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TX_CNT);
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#if defined(DOT11_HE_AX) || defined(ARBITRARY_CCK_OFDM_TX)
	UINT8 tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
#endif
#ifdef ARBITRARY_CCK_OFDM_TX
	UINT32 tx_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
#endif
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#if defined(CUT_THROUGH) || defined(DOT11_HE_AX)
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(pAd, control_band_idx, stack);
#endif
#if	defined(DOT11_HE_AX)
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
#endif

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev for band_idx[%d] is not initialized!\n", control_band_idx);
		goto err1;
	}

	/* TxRx swtich Recover */

	if (mode & ATE_TXFRAME) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "already in TXFRAME mode now, tx is on-going!\n");
		goto err1;
	}

	if (is_mt_ate_mac_tbl_stack_empty(pAd, control_band_idx)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "TX information is not commit, dismissed!\n");

		goto err1;
	}

	MtATESetMacTxRx(pAd, ASIC_MAC_TX, FALSE, control_band_idx);

#if defined(DOT11_HE_AX)
	if (tx_mode == MODE_HE_TRIG) {
		struct _MAC_TABLE_ENTRY *mac_tbl_entry = stack->mac_tbl_entry[0];
		struct phy_params *phy_info = &mac_tbl_entry->phy_param;
		struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(pAd, control_band_idx, ru_info_list[0]);

		if (ru_sta) {
			mt_ate_calc_phy_info(ru_sta,
						ru_sta->mpdu_length+13,	/* 13 bytes for Delimiter+FCS+A_control+HW reserved */
						TESTMODE_GET_PARAM(pAd, control_band_idx, stbc),
						TESTMODE_GET_PARAM(pAd, control_band_idx, sgi),
						TESTMODE_GET_PARAM(pAd, control_band_idx, max_pkt_ext)*2);
			TESTMODE_SET_PARAM(pAd, control_band_idx, dmnt_ru_idx, 0);

			if (IS_AXE(pAd)) {
				MtCmdSetPhyManualTx(pAd, TRUE, ru_sta->mpdu_length, phy_info, ru_sta);

				goto done;
			} else {
				if (chip_dbg->ctrl_manual_hetb_tx)
					chip_dbg->ctrl_manual_hetb_tx(pAd,
									  control_band_idx,
									  HETB_TX_CFG,
							  TESTMODE_GET_PARAM(pAd, control_band_idx, per_pkt_bw),
							  TESTMODE_GET_PARAM(pAd, control_band_idx, sgi),
							  TESTMODE_GET_PARAM(pAd, control_band_idx, stbc),
									  ru_sta);
				else {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ctrl_manual_hetb_tx is not registered\n");
					goto done;
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid RU info\n");
			goto err1;
		}
	}
#endif	/* DOT11_HE_AX*/

#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, control_band_idx);

		if (tx_mode == MODE_CCK || tx_mode == MODE_OFDM)
			MtATESetCCK_OFDM_Path(pAd, tx_sel, control_band_idx);
	}
#endif

#if defined(TESTMODE_TX_CONTROL)
#endif

	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, control_band_idx);
	msleep(30);
	/* Stop send TX packets from upper layer */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* The first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	/*  Disable PDMA */
	chip_set_hif_dma(pAd, DMA_TX, 0);

	/* Polling TX/RX path until packets empty */
	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	if (mode & ATE_RXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, control_band_idx);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	/*  Enable PDMA */
	chip_set_hif_dma(pAd, DMA_TX_RX, 1);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	if (mode & ATE_RXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, control_band_idx);

#if defined(CUT_THROUGH) || defined(DOT11_HE_AX)
	/* Apply IPG setting to HW */
	mt_ate_apply_ipg_param(pAd, stack->wdev[0]);
#endif

	if (round != 0xFFFFFFFF) {
#ifndef ATE_TXTHREAD
		round += TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TXDONE_CNT);
#endif
#if defined(DOT11_HE_AX)
		if (tx_mode == MODE_HE_MU && TESTMODE_GET_PARAM(pAd, control_band_idx, retry))
			round = 1;
#endif
		TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TX_CNT, round);
	}

	/* Tx Frame */
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode | ATE_TXFRAME);

	if (if_ops->test_frame_tx)
		Ret = if_ops->test_frame_tx(pAd);
	else {
#ifdef CUT_THROUGH
		struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, ipg_param);
		struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, tx_time_param);
		UINT32 ipg = ipg_param->ipg;
		UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
		UINT8 stack_idx = 0 ;

		if ((pkt_tx_time > 0) || (ipg > 0)) {
			PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
			struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, control_band_idx);
			UINT32 enqueue_cnt, input_cnt;
			UINT32 round = TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TX_CNT);
			UINT32 rounded_cnt = TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TXED_CNT);
			UINT32 ampdu_cnt = tx_time_param->pkt_ampdu_cnt;
			UINT32 token_limit = que->pkt_tkid_end / 2;

			input_cnt = round*ampdu_cnt*stack->index;
			input_cnt = ((input_cnt > token_limit) ? token_limit : input_cnt);

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"repeat=0x%x, ampdu_cnt=%d, token_limit=%d, pkt_tx_time=%d, ipg=%d\n",
				round, ampdu_cnt, token_limit, pkt_tx_time, ipg);

			/* Enqueue packet in SW queue in advance, then dequeue tasklet push to H/W */
			/* treat stack->index as STA count */
			for (enqueue_cnt = 0;
				(input_cnt - enqueue_cnt) >= stack->index;
				enqueue_cnt += stack->index) {
				for (stack_idx = 0 ; stack_idx < stack->index ; stack_idx++) {
					for (ampdu_cnt = tx_time_param->pkt_ampdu_cnt;
						ampdu_cnt > 0;
						ampdu_cnt--)
						mt_ate_enq_pkt(pAd, control_band_idx, stack_idx);
				}

				rounded_cnt++;
			}
			stack->q_idx = 0;
#if defined(DOT11_HE_AX)
			/* workaround for TX MU/VHT-MIMO without retry */
			if (tx_mode == MODE_HE_MU || tx_mode == MODE_VHT_MIMO)
				mt_ate_enq_pkt(pAd, control_band_idx, 0);
#endif /* DOT11_HE_AX */
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "txed=%d\n", rounded_cnt);

			TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TXED_CNT, rounded_cnt);
#if defined(DOT11_HE_AX)
			if (tx_mode == MODE_HE_TRIG && (IS_MT7915(pAd) || IS_MT7986(pAd) ||
				IS_MT7916(pAd) || IS_MT7981(pAd))) {
				struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

				if (chip_dbg->ctrl_manual_hetb_tx)
					chip_dbg->ctrl_manual_hetb_tx(pAd, control_band_idx,
								      HETB_TX_START, 0, 0, 0, NULL);
			}
#endif	/* DOT11_HE_AX */
		}
#endif
			Ret = MT_ATETxControl(pAd, control_band_idx, NULL);
	}

	if (Ret)
		goto err0;

	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);

#if defined(DOT11_HE_AX)
done:
#endif
	ATECtrl->did_tx = 1;

#if defined(DBDC_MODE)
#endif

err1:
	return Ret;
err0:
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, (mode & ~ATE_TXFRAME));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Err %d, wdev_idx:%x\n", Ret, wdev->wdev_idx);
	return Ret;
}


static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	if (mode & ATE_RXFRAME)
		goto err0;

	/* Firmware offloading CR need to msleep(30) Currently for the second NETDEV_STOP_QUEUE */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, control_band_idx);
	msleep(30);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	chip_set_hif_dma(pAd, DMA_TX, 0);

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Turn on TX again if set before */
	if (mode & ATE_TXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);

	/* reset counter when iwpriv only */
	if (ATECtrl->bQAEnabled != TRUE) {
		if (control_band_idx == TESTMODE_BAND0)
			ATECtrl->rx_stat.RxTotalCnt[0] = 0;

		if (IS_ATE_DBDC(pAd) && control_band_idx == TESTMODE_BAND1)
			ATECtrl->rx_stat.RxTotalCnt[1] = 0;
	}

	pAd->WlanCounters[0].FCSErrorCount.u.LowPart = 0;
	/* Enable PDMA */
	chip_set_hif_dma(pAd, DMA_TX_RX, 1);
	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, control_band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	if (control_band_idx == TESTMODE_BAND0) {
		MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND0);
		MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND0);
	}

	if (IS_ATE_DBDC(pAd) && control_band_idx == TESTMODE_BAND1) {
		MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND1);
		MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND1);
	}

#endif /* CONFIG_HW_HAL_OFFLOAD */
	msleep(30);
	mode |= ATE_RXFRAME;
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);

#if defined(DOT11_HE_AX)
	if (TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode) == MODE_HE_MU) {
		if (TESTMODE_GET_PARAM(pAd, control_band_idx, mu_rx_aid))
			MtATESetRxMUAid(pAd, control_band_idx, TESTMODE_GET_PARAM(pAd, control_band_idx, mu_rx_aid));
		else
			MtATESetRxMUAid(pAd, control_band_idx, 0xf100);
	} else if (TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode) == MODE_HE_TRIG) {
		UINT16 sta_idx = 0;
		struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
		struct _ATE_RU_STA *pri_sta = NULL;
		struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(pAd, control_band_idx, ru_info_list[0]);

		for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			if (ru_sta[sta_idx].valid) {
				/* 13 bytes for Delimiter+FCS+A_control+HW reserved */
				mt_ate_calc_phy_info(&ru_sta[sta_idx],
							ru_sta[sta_idx].mpdu_length+13,
							TESTMODE_GET_PARAM(pAd, control_band_idx, stbc),
							TESTMODE_GET_PARAM(pAd, control_band_idx, sgi),
						  TESTMODE_GET_PARAM(pAd, control_band_idx, max_pkt_ext)*2);
			}
		}

		pri_sta = mt_ate_dominate_ru(pAd, control_band_idx);

		if (pri_sta && chip_dbg->ctrl_manual_hetb_rx) {
			chip_dbg->ctrl_manual_hetb_rx(pAd,
							  control_band_idx,
							  TRUE,
							  TESTMODE_GET_PARAM(pAd, control_band_idx, per_pkt_bw),
							  TESTMODE_GET_PARAM(pAd, control_band_idx, sgi),
							  TESTMODE_GET_PARAM(pAd, control_band_idx, stbc),
							  TESTMODE_GET_PARAM(pAd, control_band_idx, hetb_rx_csd),
							  pri_sta, ru_sta);
		}
	}
#endif	/* DOT11_HE_AX */
	if (if_ops->test_frame_rx)
		if_ops->test_frame_rx(pAd);

	ATECtrl->did_rx = 1;
err0:
	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	INT32 Ret = 0;
#ifdef ATE_TXTHREAD
	INT32 thread_idx = 0;
#endif
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32 Band = (control_band_idx ==  0) ? 24070000 : 50000000;
#if defined(DOT11_HE_AX)
	UINT8 tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
#endif
	UINT32 Mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	struct qm_ops *ops = pAd->qm_ops;
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, tx_time_param);
	UINT32 ipg = ipg_param->ipg;
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"control_band_idx=%u\n", control_band_idx);

#if defined(DOT11_HE_AX)
	if (tx_mode == MODE_HE_TRIG) {
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

			if (chip_dbg->ctrl_manual_hetb_tx)
				chip_dbg->ctrl_manual_hetb_tx(pAd, control_band_idx, HETB_TX_STOP, 0, 0, 0, NULL);

			MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, control_band_idx);
		} else {
			MtCmdSetPhyManualTx(pAd, FALSE, 0, NULL, NULL);

			goto done;
		}
	}
#endif /* DOT11_HE_AX */

	TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TXED_CNT, 0);
#ifdef ATE_TXTHREAD
	TESTMODEThreadStopTx(pAd, thread_idx);
#endif

	/* ate cmd to stop tx carr */
	if ((Mode & ATE_TXCARR)) {
		if (ATEOp->DBDCTxTone)
			ATEOp->DBDCTxTone(pAd, 0, 0, 0, 0, 0, 0, Band);

		if (ATEOp->SetDBDCTxTonePower)
			ATEOp->SetDBDCTxTonePower(pAd, 0, 0, 0);
	}

	if ((Mode & ATE_TXFRAME) || (Mode == ATE_STOP)) {
		Mode &= ~ATE_TXFRAME;
		TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, Mode);

		if ((pkt_tx_time > 0) || (ipg > 0)) {
			/* Flush SW queue */
			if (ops->sta_clean_queue)
				ops->sta_clean_queue(pAd, WCID_ALL);

			/* Clean per sta TX queue and disable STA pause CRs for transmitting packet */
			MtATESetCleanPerStaTxQueue(pAd, FALSE);

			if (ipg > 0 && chip_ops->restore_reg_during_ate) {
				chip_ops->restore_reg_during_ate(pAd, control_band_idx);
			}
		}
#ifdef ARBITRARY_CCK_OFDM_TX
		if (IS_MT7615(pAd)) {
			MtATEInitCCK_OFDM_Path(pAd, control_band_idx);
		}
#endif

#if defined(TESTMODE_TX_CONTROL)
#endif
	}

#if defined(DOT11_HE_AX)
done:
#endif

	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 Mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
#if defined(DOT11_HE_AX)
	UINT32 tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
#endif
	Ret = MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, control_band_idx);

	Mode &= ~ATE_RXFRAME;
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, Mode);

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

#if defined(DOT11_HE_AX)
	{
		struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

		/* set aid more then 0x7ff to turned off */
		if (tx_mode == MODE_HE_MU && TESTMODE_GET_PARAM(pAd, control_band_idx, mu_rx_aid))
			MtATESetRxMUAid(pAd, control_band_idx, 0xf100);

		if (tx_mode == MODE_HE_TRIG && chip_dbg->ctrl_manual_hetb_rx)
			chip_dbg->ctrl_manual_hetb_rx(pAd,
							  control_band_idx,
							  FALSE,
							  0, 0, 0, 0xffffffffffffffff,
							  (struct _ATE_RU_STA *)NULL, (struct _ATE_RU_STA *)NULL);
	}
#endif	/* DOT11_HE_AX */
	return Ret;
}
#else
static INT32 MT_ATEStartTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StartTx(pAd, 0, 0);
#endif
	ATECtrl->did_tx = 1;
	return Ret;
}

static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StartRx0(pAd);
	Ret = StartRx1(pAd);
#endif
	ATECtrl->did_rx = 1;
	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StopTx(pAd);
#endif
	ATECtrl->op_mode = ~ATE_TXFRAME;
	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StopRx0(pAd);
	Ret = StopRx1(pAd);
#endif
	return Ret;
}
#endif


static INT32 MT_ATESetTxAntenna(RTMP_ADAPTER *pAd, UINT32 Ant)
{
	INT32 Ret = 0, ant_loop = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd), ant_mask = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	for (ant_loop = 0; ant_loop < GET_MAX_PATH(cap, control_band_idx, 0); ant_loop++)
		ant_mask |= (0x1 << ant_loop);

	Ant &= ant_mask;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Ant = 0x%x, control_band_idx = %d\n",
		Ant, control_band_idx);

	/* For TXD setting, change to stream number when ATE set channel (MtCmdSetTxRxPath) */
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_ant, Ant);

#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo only 7603/7628 E1? */
#endif
	return Ret;
}


static INT32 MT_ATESetRxAntenna(RTMP_ADAPTER *pAd, UINT32 Ant)
{
	INT32 Ret = 0, ant_loop = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd), ant_mask = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	for (ant_loop = 0; ant_loop < GET_MAX_PATH(cap, control_band_idx, 1); ant_loop++)
		ant_mask |= (0x1 << ant_loop);

	Ant &= ant_mask;

	/* 0: ALL Path */
	if (Ant == 0)
		Ant = ant_mask;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Ant = 0x%x, control_band_idx = %d\n",
		Ant, control_band_idx);

	/* After mt7615 its 4 bit mask for Rx0,1,2,3 */
	/* Send to FW and take effect when ATE set channel (MtCmdSetTxRxPath) */
	TESTMODE_SET_PARAM(pAd, control_band_idx, rx_ant, Ant);

	return Ret;
}


static INT32 MT_ATESetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32 Ret = 0;
	TESTMODE_SET_PARAM(pAd, control_band_idx, rf_freq_offset, FreqOffset);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdSetFreqOffset(pAd, FreqOffset, control_band_idx);
#else
	{
		struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

		if ((IS_MT76x6(pAd)) &&
			!(ATECtrl->en_man_set_freq)) {
			UINT32 reg = 0;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "MT76x6 Manual Set Freq bk ori val\n");
			MtTestModeBkCr(pAd, FREQ_OFFSET_MANUAL_ENABLE, TEST_HW_BKCR);
			MtTestModeBkCr(pAd, FREQ_OFFSET_MANUAL_VALUE, TEST_HW_BKCR);
			reg = (reg & 0xFFFF80FF) | (0x7F << 8);
			HW_IO_WRITE32(pAd->hdev_ctrl, FREQ_OFFSET_MANUAL_ENABLE, reg);
			ATECtrl->en_man_set_freq = 1;
		}

		if (ATECtrl->en_man_set_freq) {
			UINT32 reg = 0;
			HW_IO_READ32(pAd->hdev_ctrl, FREQ_OFFSET_MANUAL_VALUE, &reg);
			reg = (reg & 0xFFFF80FF) | (FreqOffset << 8);
			HW_IO_WRITE32(pAd->hdev_ctrl, FREQ_OFFSET_MANUAL_VALUE, reg);
		} else
			MtAsicSetRfFreqOffset(pAd, FreqOffset);
	}
#endif
	return Ret;
}


static INT32 MT_ATEGetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 *FreqOffset)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_EVENT_ID_ATE_TEST_MODE_T *pResult = (EXT_EVENT_ID_ATE_TEST_MODE_T *)((UINT8 *) pAd->FWRspContent + sizeof(EVENT_RXD));
#endif

	if ((!IS_ATE_DBDC(pAd)) && (ATECtrl->control_band_idx == 1)) {
		/* Protection for not support DBDC, then control_band_idx should always be 0 */
		ATECtrl->control_band_idx = 0;
		control_band_idx = ATECtrl->control_band_idx;
	}

#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdGetFreqOffset(pAd, control_band_idx, FreqOffset);
	os_msec_delay(30);
#if	defined(COMPOS_TESTMODE_WIN)
	*FreqOffset = OS_NTOHL(pResult->aucAteResult);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "FreqOffset:%X pResult->aucAteResult = %X\n",
			  *FreqOffset, pResult->aucAteResult);
#endif
#endif
	return Ret;
}

static INT32 MT_ATESetChannel(RTMP_ADAPTER *pAd,
					INT16 Value, UINT32 pri_sel,
					UINT32 reason,
					UINT32 Ch_Band)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR ctrl_ch = 0;
	UCHAR target_phymode = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR ch = TESTMODE_GET_PARAM(pAd, control_band_idx, channel);
	UCHAR bw = TESTMODE_GET_PARAM(pAd, control_band_idx, bw);
	UCHAR dbw = TESTMODE_GET_PARAM(pAd, control_band_idx, per_pkt_bw);
	UINT8 tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	UINT32 tx_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
	UINT32 rx_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, rx_ant);

	/* Backup several parameters before TSSI calibration */
	UCHAR ucPerPktBW_backup;
	UCHAR uctx_mode_backup;
	UCHAR ucMcs_backup;
	UCHAR ucNss_backup;
	UINT32 u4TxLength_backup;

#ifdef TXBF_SUPPORT
	UINT32 iTxBf = TESTMODE_GET_PARAM(pAd, control_band_idx, ibf);
	UINT32 eTxBf = TESTMODE_GET_PARAM(pAd, control_band_idx, ebf);
#endif
	INT i = 0;
	UINT32 tx_stream_num = 0;
	UINT32 max_stream_num = 0, max_path_num = 0;
	INT32 ch_offset = 0;
	const INT bw40_sel[] = { -2, 2};
#ifdef DOT11_VHT_AC
	const INT bw80_sel[] = { -6, -2, 2, 6};
	const INT bw160_sel[] = { -14, -10, -6, -2, 2, 6, 10, 14};
	UCHAR ch2 = TESTMODE_GET_PARAM(pAd, control_band_idx, channel_2nd);
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if !defined(COMPOS_TESTMODE_WIN)
	struct wifi_dev *wdev = NULL, *wdev_txd = NULL;

	/* To update wdev setting according to ch_band */
	wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);

	if (!wdev)
		goto err;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev_idx=%d, ch=%d\n", wdev->wdev_idx, wdev->channel);
#if defined(DOT11_HE_AX)
	wdev_txd = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[1]);

	if (!wdev_txd)
		goto err;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev_idx(txd)=%d, ch=%d\n", wdev_txd->wdev_idx, wdev_txd->channel);

	HcReleaseRadioForWdev(pAd, wdev_txd);
#endif /* DOT11_HE_AX */
	HcReleaseRadioForWdev(pAd, wdev);
#endif /* !defined(COMPOS_TESTMODE_WIN) */

	max_path_num = GET_MAX_PATH(cap, control_band_idx, 1);
	if (IS_ATE_DBDC(pAd))
		if (!IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY))
			max_stream_num = max_path_num / 2 ? max_path_num / 2 : 1;
		else
			max_stream_num = max_path_num;
	else
		max_stream_num = max_path_num;

	/* 0: 20M; 1: 40M; 2: 80M; 3: 160M; 4: 10M; 5: 5M; 6: 80+80MHz */
	switch (bw) {
	case BW_40:
		if (pri_sel > 1)
			goto err0;

		ctrl_ch = ch + bw40_sel[pri_sel];
		ch_offset = bw40_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;
#ifdef DOT11_VHT_AC

	case BW_8080:
		if (pri_sel >= 8)
			goto err0;

		if ((ch2 < ch) && ch2) {
			UCHAR tmp = ch;
			ch = ch2;
			ch2 = tmp;
		}

		if (pri_sel < 4) {
			ctrl_ch = ch + bw80_sel[pri_sel];
			ch_offset = bw80_sel[pri_sel];

			if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
				goto err1;
		} else {
			ctrl_ch = ch2 + bw80_sel[pri_sel - 4];
			ch_offset = bw80_sel[pri_sel - 4];

			if ((INT32)(ch2 + ch_offset) <= 0 || (INT32)(ch2 - ch_offset) <= 0)
				goto err1;
		}

		break;

	case BW_80:
		if (pri_sel >= 4)
			goto err0;

		ctrl_ch = ch + bw80_sel[pri_sel];
		ch_offset = bw80_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;

	case BW_160:
		if (pri_sel >= 8)
			goto err0;

		ctrl_ch = ch + bw160_sel[pri_sel];
		ch_offset = bw160_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;
#endif

	case BW_20:
	default:
		ch2 = 0;
		if (dbw > bw) {
			switch (dbw) {
			case BW_80:
				if (pri_sel >= 4)
					goto err0;

				ch_offset = bw80_sel[pri_sel];
				break;
			case BW_160:
				if (pri_sel >= 8)
					goto err0;

				ch2 = TESTMODE_GET_PARAM(pAd, control_band_idx, channel_2nd);
				ch_offset = bw160_sel[pri_sel];
				break;
			default: /* BW_40 */
				if (pri_sel > 1)
					goto err0;

				ch_offset = bw40_sel[pri_sel];
			}
		}
		ctrl_ch = ch;
		break;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, ctrl_ch, ctrl_ch);

	switch (tx_mode) {
	case MODE_CCK:
	case MODE_OFDM:
#ifdef ARBITRARY_CCK_OFDM_TX
		if (IS_MT7615(pAd)) {
			tx_stream_num = max_stream_num;
		} else
#endif
		{
			/* To get TX max stream number from TX antenna bit mask
			    tx_sel=2 -> tx_stream_num=2
			    tx_sel=4 -> tx_stream_num=3
			    tx_sel=8 -> tx_stream_num=4
			*/
			for (i = max_stream_num; i > 0; i--) {
				if (tx_sel & BIT(i-1)) {
					tx_stream_num = i;
					break;
				}
			}
		}
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
	case MODE_VHT:
	case MODE_VHT_MIMO:
#ifdef TXBF_SUPPORT
		if (iTxBf || eTxBf) {
			for (i = 0; i < 4; i++) {
				if (tx_sel & (1 << i))
					tx_stream_num++;
				else
					break;
			}
		} else
			tx_stream_num = max_stream_num;

#else
		tx_stream_num = max_stream_num;
#endif
		break;

	default:
		tx_stream_num = max_stream_num;
	}

	tx_stream_num = tx_stream_num ? tx_stream_num : 1;
	tx_stream_num = tx_stream_num <= max_stream_num ? tx_stream_num : max_stream_num;


	TESTMODE_SET_PARAM(pAd, control_band_idx, rx_strm_pth, rx_sel);
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_strm_num, tx_stream_num);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"T/Rx_sel:%x/%x, Tx Stream:%x, PhyMode:%x\n",
		tx_sel, rx_sel, tx_stream_num, target_phymode);
	/* set antenna settings to pAd for MtCmdSetTxRxPath() */
#if defined(DBDC_MODE) && (defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981))
	if ((IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) && IS_ATE_DBDC(pAd)) {
		if (control_band_idx == DBDC_BAND0) {
			rx_sel = rx_sel & 0x3;
			pAd->dbdc_band0_rx_path = rx_sel;

			pAd->dbdc_band0_tx_num = tx_stream_num;

			pAd->dbdc_band0_rx_num = 0;
			for (i = 0; i < 2; i++) {
				if (rx_sel & BIT(i)) {
					pAd->dbdc_band0_rx_num++;
				}
			}
		} else {
			rx_sel >>= 2*control_band_idx;
			pAd->dbdc_band1_rx_path = rx_sel;

			pAd->dbdc_band1_tx_num = tx_stream_num;

			pAd->dbdc_band1_rx_num = 0;
			for (i = 0; i < 2; i++) {
				if (rx_sel & BIT(i)) {
					pAd->dbdc_band1_rx_num++;
				}
			}
		}
	} else
#endif	/* DBDC_MODE */
	{
		pAd->Antenna.field.TxPath = tx_stream_num;
		pAd->Antenna.field.RxPath = rx_sel;
	}

	if (Ch_Band == 0)
		target_phymode = TEST_WMODE_CAP_24G;
	else
		target_phymode = TEST_WMODE_CAP_5G;

	wdev->channel = ctrl_ch;
	wdev->PhyMode = target_phymode;
	wlan_config_set_ch_band(wdev, target_phymode);
#if defined(DOT11_HE_AX)
	wlan_config_set_ap_bw(wdev, dbw);
	wlan_config_set_ap_cen(wdev, ch-ch_offset);
#endif /* DOT11_HE_AX */
	wlan_config_set_ht_bw(wdev, ((bw > BW_20) ? HT_BW_40 : HT_BW_20));
	wlan_config_set_ext_cha(wdev, ch_offset);
	wlan_config_set_vht_bw(wdev, ((bw > BW_40) ? (VHT_BW_80+(bw-BW_80)) : VHT_BW_2040));
	Ret = wdev_attr_update(pAd, wdev);
	Ret = wdev_edca_acquire(pAd, wdev);
#if defined(DOT11_HE_AX)
	wdev_txd->channel = ctrl_ch;
	wdev_txd->PhyMode = target_phymode;
	wlan_config_set_ch_band(wdev_txd, target_phymode);
	wlan_config_set_ap_bw(wdev_txd, dbw);
	wlan_config_set_ap_cen(wdev_txd, ch-ch_offset);
	wlan_config_set_ht_bw(wdev_txd, ((bw > BW_20) ? HT_BW_40 : HT_BW_20));
	wlan_config_set_ext_cha(wdev_txd, ch_offset);
	wlan_config_set_vht_bw(wdev_txd, ((bw > BW_40) ? (VHT_BW_80+(bw-BW_80)) : VHT_BW_2040));
	Ret = wdev_attr_update(pAd, wdev_txd);
	Ret = wdev_edca_acquire(pAd, wdev_txd);
#endif
	/* restore antenna setting to normal mode */
	pAd->Antenna.field.TxPath = ATECtrl->tx_path;
	pAd->Antenna.field.RxPath = ATECtrl->rx_path;

	/* Update Power Limit Table */
#ifdef SINGLE_SKU_V2
	if (cap->txpower_type == TX_POWER_TYPE_V1)
		MtPwrLimitTblChProc(pAd, control_band_idx, Ch_Band, ctrl_ch, ch);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%u, bw:%x, ch:%u, ctrl_ch:%u, cntl_ch2:%u, pri_sel:%x\n",
		control_band_idx, bw, ch, ctrl_ch, ch2, pri_sel);

	ucPerPktBW_backup = TESTMODE_GET_PARAM(pAd, control_band_idx, per_pkt_bw);
	uctx_mode_backup = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	ucMcs_backup = TESTMODE_GET_PARAM(pAd, control_band_idx, mcs);
	ucNss_backup = TESTMODE_GET_PARAM(pAd, control_band_idx, nss);
	u4TxLength_backup = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_len);

	/* 160C/160NC TSSI workaround */
	Ret = MtATETssiTrainingProc(pAd, bw, control_band_idx);

	/* Recovery several parameter after TSSI calibration */
	TESTMODE_SET_PARAM(pAd, control_band_idx, per_pkt_bw, ucPerPktBW_backup);
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_mode, uctx_mode_backup);
	TESTMODE_SET_PARAM(pAd, control_band_idx, mcs, ucMcs_backup);
	TESTMODE_SET_PARAM(pAd, control_band_idx, nss, ucNss_backup);
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_len, u4TxLength_backup);

	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Invalid pri_sel:%x, Set Channel Fail\n", pri_sel);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Invalid Control Channel:%u|%u, Set Channel Fail\n", ctrl_ch, ch - ch_offset);
#if !defined(COMPOS_TESTMODE_WIN)
err:
	if (!wdev || !wdev_txd)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev\n");
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return -1;
}


static INT32 MT_ATESetBW(RTMP_ADAPTER *pAd, UINT16 system_bw, UINT16 per_pkt_bw)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	if (per_pkt_bw == BW_NUM) {
		if (system_bw == BW_8080)
			per_pkt_bw = BW_160;
		else
			per_pkt_bw = system_bw;
	} else {
		if (per_pkt_bw > system_bw)
			per_pkt_bw = system_bw;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, bw, system_bw);
	TESTMODE_SET_PARAM(pAd, control_band_idx, per_pkt_bw, per_pkt_bw);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"System BW=%d, Per Packet BW=%d, control_band_idx=%d\n",
		system_bw, per_pkt_bw, control_band_idx);
	return Ret;
}


static INT32 mt_ate_set_duty_cycle(RTMP_ADAPTER *pAd, UINT32 value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 duty_cycle = value;

	TESTMODE_SET_PARAM(pAd, control_band_idx, duty_cycle, duty_cycle);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Duty cycle=%d%%, control_band_idx=%d\n",
		duty_cycle, control_band_idx);
	return ret;
}


static INT32 mt_ate_set_pkt_tx_time(RTMP_ADAPTER *pAd, UINT32 value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _ATE_TX_TIME_PARAM *tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, tx_time_param);

	tx_time_param->pkt_tx_time = value;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Pkt Tx time=%dus, control_band_idx=%d\n",
		tx_time_param->pkt_tx_time, control_band_idx);
	return ret;
}

#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo RX_BLK */
static INT32 MT_ATESampleRssi(RTMP_ADAPTER *pAd, RX_BLK *RxBlk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_RX_STATISTIC *rx_stat =
			&ATECtrl->rx_stat;
	INT32 Ret = 0;
	INT i = 0;
	UINT32 ANT_NUM = pAd->Antenna.field.TxPath;

	for (i = 0; i < ANT_NUM; i++) {
		if (RxBlk->rx_signal.raw_rssi[i] != 0) {
			rx_stat->LastRssi[i] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[i]), i);

			if (rx_stat->MaxRssi[i] <  rx_stat->LastRssi[i])
				rx_stat->MaxRssi[i] = rx_stat->LastRssi[i];

			if (rx_stat->MinRssi[i] >  rx_stat->LastRssi[i])
				rx_stat->MinRssi[i] =  rx_stat->LastRssi[i];

			rx_stat->AvgRssiX8[i] = (rx_stat->AvgRssiX8[i] - rx_stat->AvgRssi[i]) + rx_stat->LastRssi[i];
			rx_stat->AvgRssi[i] = rx_stat->AvgRssiX8[i] >> 3;
		}

		rx_stat->LastSNR[i] = RxBlk->rx_signal.raw_snr[i];
	}

	rx_stat->NumOfAvgRssiSample++;
	return Ret;
}
#endif


static UINT8 sigext_time_list[] = {
	0, /* CCK */
	6, /* OFDM */
	6, /* HTMIX */
	6, /* HTGREENFIELD */
	6, /* VHT */
};
static UINT8 mt_ate_get_sigext_time_by_phymode(UCHAR phy_mode)
{
	UINT8 sigext_time = 0;

	switch (phy_mode) {
	case MODE_CCK:
		sigext_time = sigext_time_list[MODE_CCK];
		break;

	case MODE_OFDM:
		sigext_time = sigext_time_list[MODE_OFDM];
		break;

	case MODE_HTMIX:
		sigext_time = sigext_time_list[MODE_HTMIX];
		break;

	case MODE_HTGREENFIELD:
		sigext_time = sigext_time_list[MODE_HTGREENFIELD];
		break;

	case MODE_VHT:
	case MODE_VHT_MIMO:
		sigext_time = sigext_time_list[MODE_VHT];
		break;

	default:
		sigext_time = sigext_time_list[MODE_OFDM];
		break;
	}

	return sigext_time;
}


static UINT16 slot_time_list[] = {
	9, /* CCK */
	9, /* OFDM */
	9, /* HTMIX */
	9, /* HTGREENFIELD */
	9, /* VHT */
};
static UINT16 mt_ate_get_slot_time_by_phymode(UCHAR phy_mode)
{
	UINT16 slot_time = 0;

	switch (phy_mode) {
	case MODE_CCK:
		slot_time = slot_time_list[MODE_CCK];
		break;

	case MODE_OFDM:
		slot_time = slot_time_list[MODE_OFDM];
		break;

	case MODE_HTMIX:
		slot_time = slot_time_list[MODE_HTMIX];
		break;

	case MODE_HTGREENFIELD:
		slot_time = slot_time_list[MODE_HTGREENFIELD];
		break;

	case MODE_VHT:
	case MODE_VHT_MIMO:
		slot_time = slot_time_list[MODE_VHT];
		break;

	default:
		slot_time = slot_time_list[MODE_OFDM];
		break;
	}

	return slot_time;
}


static UINT16 mt_ate_get_cw(UINT32 ipg, UINT16 slot_time)
{
	INT cnt = 0, val;
	val = (ipg + slot_time) / slot_time;

	while (val >>= 1)
		cnt++;

	if (cnt >= MAX_CW)
		cnt = MAX_CW;

	return cnt;
}


static INT32 mt_ate_get_ipg_param(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _ATE_IPG_PARAM *ipg_param = (struct _ATE_IPG_PARAM *)TESTMODE_GET_PADDR(pAd, control_band_idx, ipg_param);
	UCHAR tx_mode;
	UINT32 ipg, real_ipg;
	UINT8 sig_ext, aifsn;
	UINT16 slot_time, sifs_time, cw;

	tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	ipg = ipg_param->ipg;
	sig_ext = mt_ate_get_sigext_time_by_phymode(tx_mode);
	slot_time = mt_ate_get_slot_time_by_phymode(tx_mode);
	sifs_time = DEFAULT_SIFS_TIME;
	/*
	 *  1. ipg = sig_ext + sifs_time + slot_time
	 *  2. ipg = sig_ext + sifs_time + aifsn * slot_time + ((1 << cw) - 1) * slot_time
	 *  If it's CCK mode, there's no need to consider sig_ext
	 *  And it's no need to count in backoff time in older ATE driver design, configure SIFS/SLOT only
	 *  Consider which ac queue will be used each case
	 */
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Expected ipg=%d, control_band_idx=%d\n",
		ipg, control_band_idx);

	if (ipg < (sig_ext + sifs_time + slot_time)) {
		UINT32 duty_cycle = TESTMODE_GET_PARAM(pAd,	control_band_idx, duty_cycle);

		ipg_param->ipg = 0;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid IPG!! sig_ext=%d, slot_time=%d, sifs_time=%d\n"
				  "Set ipg=%d\n",
				  sig_ext, slot_time, sifs_time, ipg);

		if (duty_cycle > 0) {
			duty_cycle = 0;
			TESTMODE_SET_PARAM(pAd, control_band_idx, duty_cycle, duty_cycle);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Invalid IPG with such duty_cycle and pkt_tx_time!!\n"
					  "Set duty_cycle=%d\n", duty_cycle);
		}

		return ret;
	}

	ipg -= sig_ext;

	if (ipg <= (MAX_SIFS_TIME + slot_time)) {
		sifs_time = ipg - slot_time;
		aifsn = MIN_AIFSN;
		cw = 0;
	} else {
		cw = mt_ate_get_cw(ipg, slot_time);
		ipg -= ((1 << cw) - 1) * slot_time;
		aifsn = ipg / slot_time;

		if (aifsn >= MAX_AIFSN)
			aifsn = MAX_AIFSN;

		ipg -= aifsn * slot_time;

		if (ipg <= DEFAULT_SIFS_TIME)
			sifs_time = DEFAULT_SIFS_TIME;
		else if ((ipg > DEFAULT_SIFS_TIME) &&
				 (ipg <= MAX_SIFS_TIME))
			sifs_time = ipg;
		else
			sifs_time = MAX_SIFS_TIME;
	}

	real_ipg = sig_ext + sifs_time + aifsn * slot_time
			   + ((1 << cw) - 1) * slot_time;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "sig_ext=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
			  sig_ext, slot_time, sifs_time, aifsn, cw);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Real ipg=%d\n", real_ipg);
	ipg_param->sig_ext = sig_ext;
	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;
	ipg_param->txop = 0;
	return ret;
}


static INT32 mt_ate_set_ipg(RTMP_ADAPTER *pAd, UINT32 value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 ipg = value;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"IPG=%dus, control_band_idx=%d\n",
		ipg, control_band_idx);

	ret = mt_ate_get_ipg_param(pAd);

	return ret;
}


static INT32 mt_ate_set_slot_time(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"SlotTime:%d, SifsTime:%d, control_band_idx:%d\n",
		SlotTime,
		SifsTime,
		control_band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdATESetSlotTime(pAd, (UINT8)SlotTime,	(UINT8)SifsTime, RIFS_TIME, EIFS_TIME, control_band_idx);
#endif
	return Ret;
}


static INT32 MT_ATESetAIFS(RTMP_ADAPTER *pAd, CHAR Value)
{
	INT32 Ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
#endif /* WIFI_UNIFIED_COMMAND */

	/*
		chnage the mask to 0xf to sync MAC AIFS CR range is [0:3]
		QA tool may set the lage value than 0xF
	*/
	UINT val = Value & 0x0000000f;
	/* Test mode use AC0 for TX */
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Value:%x\n", val);
#ifdef WIFI_UNIFIED_COMMAND
	AsicSetWmmParam(pAd, wdev, 0, WMM_PARAM_AC_0, WMM_PARAM_AIFSN, val);
#else
	AsicSetWmmParam(pAd, 0, WMM_PARAM_AC_0, WMM_PARAM_AIFSN, val);
#endif /* WIFI_UNIFIED_COMMAND */
	return Ret;
}


static INT32 MT_ATESetPowerDropLevel(RTMP_ADAPTER *pAd, UINT32 PowerDropLevel)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"PowerDropLevel:%d, control_band_idx:%d\n",
		PowerDropLevel, control_band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdATESetPowerDropLevel(pAd, (UINT8)PowerDropLevel, control_band_idx);
#endif /* CONFIG_HW_HAL_OFFLOAD */
	return Ret;
}


static INT32 MT_ATESetTSSI(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	INT32 Ret = 0;
	Ret = MtAsicSetTSSI(pAd, Setting, WFSel);
	return Ret;
}


static INT32 MT_ATELowPower(RTMP_ADAPTER *pAd, UINT32 Control)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN)

	if (Control)
		MlmeLpEnter(pAd);
	else
		MlmeLpExit(pAd);

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}

static INT32 MT_ATESetEepromToFw(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	PUSHORT EeprAntCfg = NULL;

			/* Change eeprom band selection value by wirelessmode for FW */
			if (ops->eep_set_band_sel)
				ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */

	MtCmdEfusBufferModeSet(pAd, EEPROM_FLASH);

#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
			/* Recover eeprom band selection value after fw updated*/
			if (ops->eep_set_band_sel && EeprAntCfg != NULL)
				ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */

	return Ret;
}

static INT32 MT_ATESetDPD(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */
	INT32 Ret = 0;
	Ret = MtAsicSetDPD(pAd, Setting, WFSel);
	return Ret;
}


static INT32 MT_ATEStartTxTone(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32 MT_ATEDBDCTxTone(RTMP_ADAPTER *pAd,
					UINT32 Control,
					UINT32 AntIndex,
					UINT32 ToneType,
					UINT32 ToneFreq,
					INT32 DcOffset_I,
					INT32 DcOffset_Q,
					UINT32 Band)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	MtCmdTxTone(pAd, control_band_idx, Control, AntIndex, ToneType, ToneFreq, DcOffset_I, DcOffset_Q, Band);
	return Ret;
}


static INT32 MT_ATESetTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "pwr1:%d, pwr2:%d\n", pwr1, pwr2);
	return Ret;
}

static INT32 MT_ATESetDBDCTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2, UINT32 AntIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 i;

	if (IS_MT7626(pAd)) {
		/* transform AntIdx form Ant bit map to WF Idx */
		for (i = 0; i < 4; i++) {
			/* choose first bit to decide WF idx */
			if (AntIdx & BIT(i)) {
				AntIdx = i;
				break;
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"pwr1:%d, pwr2:%d, AntIdx:%d, control_band_idx:%d\n",
		pwr1, pwr2, AntIdx, control_band_idx);
#if defined(CONNAC_TXTONE_POWER_OFFSET)
	MtCmdTxTonePower(pAd, SET_TX_TONE_GAIN_OFFSET, pwr2, AntIdx, (UINT8)control_band_idx);
#else
	MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN, pwr1, AntIdx, (UINT8)control_band_idx);
	MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN, pwr2, AntIdx, (UINT8)control_band_idx);
#endif
	return Ret;
}

static INT32 MT_ATEGetDBDCTxTonePower(RTMP_ADAPTER *pAd, PINT32 pPwr, UINT32 AntIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AntIdx:%d, control_band_idx:%d\n",
		AntIdx, control_band_idx);
#if defined(CONNAC_TXTONE_POWER_OFFSET)
	Ret = MtCmdRfTestGetTxTonePower(pAd, pPwr, AntIdx, (UINT8)control_band_idx);
#endif
	return Ret;
}

static INT32 MT_ATEStopTxTone(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}


static INT32 MT_ATEStartContinousTx(RTMP_ADAPTER *pAd, CHAR WFSel, UINT32 TxfdMode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 tx_mode = 0, BW = 0, Pri_Ch = 0, Rate = 0, Central_Ch = 0, ant_sel = 0;

	tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	BW = TESTMODE_GET_PARAM(pAd, control_band_idx, bw);
	Pri_Ch = TESTMODE_GET_PARAM(pAd, control_band_idx, ctrl_ch);
	Central_Ch = TESTMODE_GET_PARAM(pAd, control_band_idx, channel);
	Rate = TESTMODE_GET_PARAM(pAd, control_band_idx, mcs);
	ant_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

	if (BW == ATE_BAND_WIDTH_8080)
		BW = 3;

	if (BW == ATE_BAND_WIDTH_160)
		BW = 4;

	{
		UINT32 Control = 1;
		MtCmdTxContinous(pAd, tx_mode, BW, Pri_Ch, Central_Ch, Rate, ant_sel, TxfdMode, control_band_idx, Control);
	}
	return Ret;
}


static INT32 MT_ATEStopContinousTx(RTMP_ADAPTER *pAd, UINT32 TxfdMode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 tx_mode = 0, BW = 0, Pri_Ch = 0, Rate = 0, Central_Ch = 0, ant_sel;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	BW = TESTMODE_GET_PARAM(pAd, control_band_idx, bw);
	Pri_Ch = TESTMODE_GET_PARAM(pAd, control_band_idx, ctrl_ch);
	Central_Ch = TESTMODE_GET_PARAM(pAd, control_band_idx, channel);
	Rate = TESTMODE_GET_PARAM(pAd, control_band_idx, mcs);
	ant_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
	{
		UINT32 Control = 0;
		Ret = MtCmdTxContinous(pAd, tx_mode, BW, Pri_Ch, Central_Ch, Rate, ant_sel, TxfdMode, control_band_idx,	Control);
	}
	return Ret;
}


static INT32 MT_OnOffRDD(struct _RTMP_ADAPTER *pAd, UINT32 rdd_idx, UINT32 rdd_in_sel, UINT32 is_start)
{
	INT32 Ret = 0;
	BOOLEAN arb_rx_on = FALSE;
	arb_rx_on = is_start ? TRUE : FALSE;
	MtATESetMacTxRx(pAd, ASIC_MAC_RX, arb_rx_on, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, arb_rx_on, TESTMODE_BAND1);

	Ret = MtCmdSetRDDTestExt(pAd, rdd_idx, rdd_in_sel, is_start);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ARB Rx On:%x\n", arb_rx_on);
	return Ret;
}

static INT32 MT_ATESetCfgOnOff(RTMP_ADAPTER *pAd, UINT32 Type, UINT32 Enable)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Type:%d, Enable:%d, control_band_idx:%d\n",
		Type, Enable, control_band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdCfgOnOff(pAd, Type, Enable, control_band_idx);
#endif
	return Ret;
}

static INT32 MT_ATEGetCfgOnOff(RTMP_ADAPTER *pAd, UINT32 Type, UINT32 *Result)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	UINT32 Value = 0;
	GET_TSSI_STATUS_T *pResult = (GET_TSSI_STATUS_T *)((UINT8 *)pAd->FWRspContent + sizeof(EVENT_RXD) + sizeof(EXT_EVENT_ATE_TEST_MODE_T));
#endif
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	Ret = MtCmdGetCfgOnOff(pAd, Type, control_band_idx, Result);
	os_msec_delay(30);
#endif
#if	defined(COMPOS_TESTMODE_WIN)
	*Result = OS_NTOHL(pResult->ucEnable);
#endif
	return Ret;
}

static INT32 MT_ATESetAntennaPort(RTMP_ADAPTER *pAd, UINT32 RfModeMask, UINT32 RfPortMask, UINT32 AntPortMask)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "RfModeMask:%d RfPortMask:%d AntPortMask:%d\n",
			  (UINT8)RfModeMask, (UINT8)RfPortMask,
			  (UINT8)AntPortMask);
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdSetAntennaPort(pAd, (UINT8)RfModeMask, (UINT8)RfPortMask, (UINT8)AntPortMask);
#endif
	return Ret;
}


static INT32 MT_ATEFWPacketCMDClockSwitchDisable(
	RTMP_ADAPTER *pAd, UINT8 isDisable)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "isDsiable=%d\n", (UINT8)isDisable);
	Ret = MtCmdClockSwitchDisable(pAd, isDisable);
	return Ret;
}


static INT32 MT_ATESetRXFilterPktLen(RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 RxPktLen)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Enable:%d, control_band_idx:%d, RxPktLen:%d\n",
		Enable, control_band_idx, RxPktLen);
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret =  MtCmdRxFilterPktLen(pAd, Enable, control_band_idx, RxPktLen);
#endif
	return Ret;
}

static INT32 MT_ATEGetTxPower(RTMP_ADAPTER *pAd, UINT32 Channel, UINT32 Ch_Band, UINT32 u4AntIdx, PUINT32 Power)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR u1BandIdx = ATECtrl->control_band_idx;
	EXT_EVENT_ID_GET_TX_POWER_T TxPowerResult = {0};

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Channel: %d, u1BandIdx: %d, u4AntIdx: %d\n",
		Channel, u1BandIdx, u4AntIdx);

	Ret = MtCmdGetTxPower(pAd, u1BandIdx, Channel, u4AntIdx, &TxPowerResult);
	*Power = TxPowerResult.i1TargetPower;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Power: 0x%x\n", *Power);

	return Ret;
}

static INT32 MT_ATEBssInfoUpdate(RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UINT32 BssIdx, UCHAR *Bssid)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN)	/* TODO::UNIYSW */
	BSS_INFO_ARGUMENT_T bss_info_argument;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "OwnMacIdx:%d BssIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  OwnMacIdx, BssIdx, PRINT_MAC(Bssid));
	NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
	bss_info_argument.OwnMacIdx = OwnMacIdx;
	bss_info_argument.ucBssIndex = BssIdx;
	os_move_mem(bss_info_argument.Bssid, Bssid,	MAC_ADDR_LEN);
	bss_info_argument.bmc_wlan_idx = MCAST_WCID_TO_REMOVE;
	bss_info_argument.NetworkType = NETWORK_INFRA;
	bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
	bss_info_argument.CipherSuit = Ndis802_11WEPDisabled;
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;
	Ret = AsicBssInfoUpdate(pAd, &bss_info_argument);
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}


static INT32 MT_ATEDevInfoUpdate(RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UCHAR *Bssid)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%d OwnMacIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
		control_band_idx, OwnMacIdx, PRINT_MAC(Bssid));
#if !defined(COMPOS_TESTMODE_WIN)	/* TODO::UNIYSW */
	Ret = AsicDevInfoUpdate(pAd, OwnMacIdx, Bssid, control_band_idx, TRUE, DEVINFO_ACTIVE_FEATURE);
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}

static INT32 MT_SetFFTMode(struct _RTMP_ADAPTER *pAd, UINT32 mode)
{
	INT32 Ret = 0;
	return Ret;
}


#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT fd)
#else
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	struct _ATE_RDD_LOG *result = &entry.log.rdd;
	UINT32 *pulse = (UINT32 *)result->aucBuffer;

	if (!result->byPass)
		MTWF_PRINT("[RDD]0x%08x %08x\n", result->u4Prefix, result->u4Count);

	MTWF_PRINT("[RDD]0x%08x %08x\n", pulse[0], pulse[1]);
	return 0;
}


INT32 MT_ATERFTestCB(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 ret = 0;
	EXT_EVENT_RF_TEST_RESULT_T *result = (EXT_EVENT_RF_TEST_RESULT_T *)Data;
	EXT_EVENT_RF_TEST_DATA_T *data = (EXT_EVENT_RF_TEST_DATA_T *)result->aucEvent;
	static INT total;
	static INT EventType;
	static UINT32 recal_type;
	BOOLEAN test_done = FALSE;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Length[%d], Mode[0x%x]\x1b[m\n", Length, pAd->ATECtrl.op_mode);

	/* Length of Event ACK */
	if (Length == sizeof(struct _EVENT_EXT_CMD_RESULT_T))
		test_done = TRUE;

	if (test_done)
		goto done;
#if defined(PRE_CAL_MT7622_SUPPORT) || defined(PRE_CAL_MT7626_SUPPORT) || \
	defined(PRE_CAL_MT7915_SUPPORT) || defined(PRE_CAL_MT7986_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT)
	if ((!pAd->bPreCalMode) && (!(pAd->ATECtrl.op_mode & fATE_IN_RFTEST)))
#else
	if (!(pAd->ATECtrl.op_mode & fATE_IN_RFTEST))
#endif /* PRE_CAL_MT7622_SUPPORT */
		return ret;

	result->u4FuncIndex = le2cpu32(result->u4FuncIndex);
	EventType = result->u4FuncIndex;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"EventType[0x%x]\x1b[m\n", EventType);

	switch (EventType) {
	case RDD_TEST_MODE:
		if (pAd->ATECtrl.en_log & fATE_LOG_RDD) {
			struct _ATE_RDD_LOG unit;
			struct _EVENT_WIFI_RDD_TEST_T *log =
				(struct _EVENT_WIFI_RDD_TEST_T *)Data;
			UINT64 *data = (UINT64 *)log->aucBuffer;
			INT i = 0;
			UINT len = 0;

			log->u4FuncLength = le2cpu32(log->u4FuncLength);
			log->u4Prefix = le2cpu32(log->u4Prefix);
			log->u4Count = le2cpu32(log->u4Count);

			len = (log->u4FuncLength - sizeof(struct _EVENT_WIFI_RDD_TEST_T)
				+ sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex))>>3;

			if (pAd->ATECtrl.en_log & fATE_LOG_TEST) {
				const UINT dbg_len = (log->u4FuncLength - sizeof(struct _EVENT_WIFI_RDD_TEST_T) + sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex)) >> 2;
				UINT32 *tmp = (UINT32 *)log->aucBuffer;

				for (i = 0; i < dbg_len; i++)
					MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "RDD RAW DWORD%d:%08x\n", i, tmp[i]);

				MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "RDD FuncLen:%u, len:%u, prefix:%08x, cnt:%u\n",
						  log->u4FuncLength, len, log->u4Prefix, log->u4Count);
			}

			os_zero_mem(&unit, sizeof(unit));
			unit.u4Prefix = log->u4Prefix;
			unit.u4Count = log->u4Count;

			for (i = 0; i < len; i++) {
				NdisMoveMemory(unit.aucBuffer, data++, ATE_RDD_LOG_SIZE);
				MT_ATEInsertLog(pAd, (UCHAR *)&unit, fATE_LOG_RDD, sizeof(unit));
				/* byPass is used @ logDump, if the same event, don't dump same message */
				unit.byPass = TRUE;
			}
		}

		break;

#ifdef INTERNAL_CAPTURE_SUPPORT
	case GET_ICAP_CAPTURE_STATUS:
		if (IS_MT7615(pAd))
			ExtEventICapUnSolicitStatusHandler(pAd, Data, Length);
		break;

	case GET_ICAP_RAW_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_ICAP_DUMP_RAW_DATA, (VOID *)Data, Length);
		break;
#endif/* INTERNAL_CAPTURE_SUPPORT */

	case RE_CALIBRATION:
	if (data) {
		struct _ATE_LOG_RECAL re_cal;
		INT i = 0;
		UINT32 cal_idx = 0;
		UINT32 cal_type = 0;
		UINT32 len = 0;
		UINT32 *dump_tmp = (UINT32 *)data->aucData;
		struct _ATE_LOG_DUMP_CB *log_cb = NULL;
#if defined(PRE_CAL_MT7622_SUPPORT) || defined(PRE_CAL_MT7626_SUPPORT)
		UINT32 *cal_log = NULL;

		if (IS_MT7622(pAd) || IS_MT7626(pAd)) {
			if (pAd->bPreCalMode)
				os_alloc_mem(pAd, (UCHAR **)&cal_log, CAL_LOG_SIZE);
		}
#else
		UINT32 *cal_log = NULL;

		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
			os_alloc_mem(pAd, (UCHAR **)&cal_log, 6000);
#endif /* PRE_CAL_MT7622_SUPPORT || PRE_CAL_MT7626_SUPPORT */
		data->u4CalIndex = le2cpu32(data->u4CalIndex);
		data->u4CalType = le2cpu32(data->u4CalType);
		result->u4PayloadLength = le2cpu32(result->u4PayloadLength);
		cal_idx = data->u4CalIndex;
		cal_type = data->u4CalType;
		len = result->u4PayloadLength;
		len = (len - sizeof(EXT_EVENT_RF_TEST_DATA_T)) >> 2;
		log_cb = &pAd->ATECtrl.log_dump[ATE_LOG_RE_CAL - 1];
		/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, */
		/* "CalType:%x\n", cal_type); */
		re_cal.cal_idx = cal_idx;
		re_cal.cal_type = cal_type;

		if (total == 0) {
			recal_type = cal_type;
			log_cb->recal_curr_type = recal_type;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "[Recal][%08x][START]\n", recal_type);
		}

		total += result->u4PayloadLength;

		if ((cal_type == CAL_ALL) &&
			(total == CAL_ALL_LEN))
			test_done = TRUE;

#if defined(PRE_CAL_MT7626_SUPPORT)
		if ((cal_type == PRE_CAL) || (cal_type == TX_DPD_FLATNESS_CAL)) {
			for (i = 0; i < len; i++) {
				dump_tmp[i] = le2cpu32(dump_tmp[i]);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"[Recal][%08x][%08x]\n", cal_type, dump_tmp[i]);
				re_cal.cr_val = dump_tmp[i];
				cal_log[i] = dump_tmp[i];
			}
		} else
#endif
		{
			if ((IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) &&
				((cal_type == TX_DNL_CAL) ||
				(cal_type == TX_TSSI_CAL_2G) ||
				(cal_type == TX_TSSI_CAL_5G) ||
				(cal_type == PRE_CAL) ||
				(cal_type == RX_GAIN_CAL) ||
				(cal_type == TX_DPD_FLATNESS_CAL))) {
				for (i = 0; i < len; i++) {
					dump_tmp[i] = le2cpu32(dump_tmp[i]);
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[Recal][%08x][%08x]\n", cal_type, dump_tmp[i]);
					re_cal.cr_val = dump_tmp[i];
					cal_log[i] = dump_tmp[i];
				}
			} else {
				for (i = 0; i < len; i++) {
					dump_tmp[i] = le2cpu32(dump_tmp[i]);
					if (i & 0x1) {
						MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							"%08x\n", dump_tmp[i]);
						re_cal.cr_val = dump_tmp[i];
#ifdef PRE_CAL_MT7622_SUPPORT
						if (IS_MT7622(pAd)) {
							cal_log[(i-1)/2] = dump_tmp[i];
						}
#endif /* PRE_CAL_MT7622_SUPPORT */
						if (pAd->ATECtrl.en_log & fATE_LOG_RE_CAL)
							MT_ATEInsertLog(pAd, (UCHAR *)&re_cal,
									fATE_LOG_RE_CAL, sizeof(re_cal));
					} else {
						MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							"[Recal][%08x][%08x]", cal_type, dump_tmp[i]);
						re_cal.cr_addr = dump_tmp[i];
					}
				}
			}
		}

		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			if (cal_type == TX_DNL_CAL) {
				UINT16 tmp_len = len * sizeof(UINT32);

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][DNL]Ofst = 0x%x, len=%d\n\n", pAd->DnlCalOfst, tmp_len);

				memcpy(pAd->TxDnlCal + pAd->DnlCalOfst, cal_log, tmp_len);
				pAd->DnlCalOfst += tmp_len;
			} else if (cal_type == TX_TSSI_CAL_2G) {
				UINT16 tmp_len = len * sizeof(UINT32);

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][TSSI-2G]Ofst = 0x%x, len=%d\n\n", pAd->TssiCal2GOfst, tmp_len);

				memcpy(pAd->TssiCal2G + pAd->TssiCal2GOfst, cal_log, tmp_len);
				pAd->TssiCal2GOfst += tmp_len;
			} else if (cal_type == TX_TSSI_CAL_5G) {
				UINT16 tmp_len = len * sizeof(UINT32);

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][TSSI-5G]Ofst = 0x%x, len=%d\n\n", pAd->TssiCal5GOfst, tmp_len);

				memcpy(pAd->TssiCal5G + pAd->TssiCal5GOfst, cal_log, tmp_len);
				pAd->TssiCal5GOfst += tmp_len;
			} else if (cal_type == RX_GAIN_CAL) {
				UINT16 tmp_len = len * sizeof(UINT32);
/* Santai Need Confirm */
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][RXGainCal]Ofst = 0x%x, len=%d\n\n", pAd->RXGainCalOfst, tmp_len);

				memcpy(pAd->RXGainCal + pAd->RXGainCalOfst, cal_log, tmp_len);
				pAd->RXGainCalOfst += tmp_len;
			}
#if defined(PRE_CAL_MT7915_SUPPORT) || defined(PRE_CAL_MT7986_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT)
			if (pAd->bPreCalMode) {
				if (cal_type == PRE_CAL) {
					UINT16 tmp_len = len * sizeof(UINT32);

					memcpy(pAd->PreCalImage + pAd->PreCalOfst, cal_log, tmp_len);
					pAd->PreCalOfst += tmp_len;

					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[cal_type][Pre_Cal]len=%d, PreCalOfst=%d\n\n", len, pAd->PreCalOfst);

					memcpy(pAd->PreCalImageInfo, &(pAd->PreCalOfst), sizeof(UINT32));
				} else if (cal_type == TX_DPD_FLATNESS_CAL) {
					UINT16 tmp_len = len * sizeof(UINT32);

					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[cal_type][TX_DPD], ofset=%x, len =%d\n",
						pAd->TxDPDOfst, tmp_len);
					memcpy(pAd->TxDPDImage + pAd->TxDPDOfst, cal_log, tmp_len);
					pAd->TxDPDOfst += tmp_len;
				}
			}
#endif
			os_free_mem(cal_log);
		}

#if defined(PRE_CAL_MT7626_SUPPORT)
		if (pAd->bPreCalMode) {
			if (cal_type == PRE_CAL) {
				UINT16 tmp_len = len * sizeof(UINT32);

				memcpy(pAd->PreCalImage + pAd->PreCalOfst, cal_log, tmp_len);
				pAd->PreCalOfst += tmp_len;

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][Pre_Cal]len=%d, PreCalOfst=%d\n\n", len, pAd->PreCalOfst);

				memcpy(pAd->PreCalImageInfo, &(pAd->PreCalOfst), sizeof(UINT32));
			} else if (cal_type == TX_DPD_FLATNESS_CAL) {
				UINT16 tmp_len = len * sizeof(UINT32);

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[cal_type][TX_DPD], ofset=%x, len =%d\n",
					pAd->TxDPDOfst, tmp_len);
				memcpy(pAd->TxDPDImage + pAd->TxDPDOfst, cal_log, tmp_len);
				pAd->TxDPDOfst += tmp_len;
			}
			os_free_mem(cal_log);
		}
#endif

#ifdef PRE_CAL_MT7622_SUPPORT
		if (IS_MT7622(pAd)) {

			if (pAd->bPreCalMode) {
				if (cal_type == TX_LPFG) {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[cal_type][TX_LPFG]len=%d\n", len);
					RTMPZeroMemory(pAd->CalTXLPFGImage, CAL_TXLPFG_SIZE);
					memcpy(pAd->CalTXLPFGImage, cal_log, CAL_TXLPFG_SIZE);
				} else if (cal_type == TX_DCIQC) {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[cal_type][TX_DCIQC]len=%d\n", len);
					RTMPZeroMemory(pAd->CalTXDCIQImage, CAL_TXDCIQ_SIZE);
					memcpy(pAd->CalTXDCIQImage, cal_log, CAL_TXDCIQ_SIZE);
				} else if (cal_type == TX_DPD_LINK) {
					UINT16 tmp_len = (len/2) * sizeof(UINT32);

					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[cal_type][TX_DPD], ofset=%x, len =%d\n",
						pAd->TxDpdCalOfst, tmp_len);
					memcpy(pAd->CalTXDPDImage + pAd->TxDpdCalOfst, cal_log, tmp_len);
					pAd->TxDpdCalOfst += tmp_len;
				}
				os_free_mem(cal_log);
			}
		}
#endif /* PRE_CAL_MT7622_SUPPORT */
	}
	break;

	case CALIBRATION_BYPASS:
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "No RF Test Event %x Dump\n", result->u4FuncIndex);
		break;
	}

done:

	if (test_done) {
		if (EventType == RE_CALIBRATION) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "[Recal][%08x][END]\n", recal_type);
		}

		total = 0;
		EventType = 0;
		recal_type = 0;
	}

	return ret;
}


#ifdef ATE_TXTHREAD
static INT32 MT_ATEMPSRunStatCheck(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _HQA_MPS_CB *mps_cb = NULL;
	struct _HQA_MPS_SETTING *mps_setting = NULL;
	UINT32 txed_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TXED_CNT);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(pAd, band_idx, ATE_TX_CNT);
	UINT32 idx = 0;
	INT32 ret = 0;

	mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);

	if (!mps_cb)
		goto err0;

	mps_setting = mps_cb->mps_setting;

	if (!mps_setting)
		goto err0;

	idx = mps_cb->ref_idx;

	if ((mps_cb->stat & ATE_MPS_ITEM_RUNNING) && (txed_cnt >= tx_cnt)) {
		/* UCHAR mode = TESTMODE_GET_PARAM(pAd, band_idx, Mode); */
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "MPS Item Finished idx:%d mps_cnt:%d\n", idx, mps_cb->mps_cnt);
		OS_SPIN_LOCK(&mps_cb->lock);
		mps_cb->stat = 0;
		OS_SPIN_UNLOCK(&mps_cb->lock);

		if (idx > mps_cb->mps_cnt) {
			UINT32 mode = TESTMODE_GET_PARAM(pAd, band_idx, op_mode);
			mode &= ~fATE_MPS;
			mps_cb->setting_inuse = FALSE;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "MPS All Finished idx:%d mps_cnt:%d\n", idx, mps_cb->mps_cnt);
			TESTMODE_SET_PARAM(pAd, band_idx, op_mode, mode);
			ret = MT_MPSTxStop(pAd);
		}
	}

	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "mps_cb/mps_setting NULL %p %p\n", mps_cb, mps_setting);
	return -1;
}
#endif


static INT32 MT_ATEMPSLoadSetting(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_op = ATECtrl->ATEOp;
	struct _HQA_MPS_CB *mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = NULL;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(pAd, band_idx, test_pkt);
	INT idx = 0;
	UCHAR tx_mode = 0;
	CHAR ant_sel = 0;
	UCHAR mcs = 0;
	INT32 ret = 0;
	UINT32 pwr = 0;
	UINT32 pkt_len = 0;
	UINT32 pkt_cnt = 0;
	UINT32 nss = 0;
	UINT32 pkt_bw = 0;
	UINT32 Channel = TESTMODE_GET_PARAM(pAd, band_idx, channel);
	UINT32 Ch_Band = TESTMODE_GET_PARAM(pAd, band_idx, ch_band);
	ATE_TXPOWER TxPower;
	os_zero_mem(&TxPower, sizeof(TxPower));

	if (!mps_cb)
		goto err0;

	mps_setting = mps_cb->mps_setting;

	if (!mps_setting)
		goto err0;

	OS_SPIN_LOCK(&mps_cb->lock);

	if (mps_cb->stat & ATE_MPS_ITEM_RUNNING)
		goto err1;

	mps_cb->stat |= ATE_MPS_ITEM_RUNNING;
	idx = mps_cb->ref_idx;

	if (idx > mps_cb->mps_cnt)
		goto err2;

	tx_mode = (mps_setting[idx].phy & 0x0f000000) >> 24;
	ant_sel = (mps_setting[idx].phy & 0x00ffff00) >> 8;
	mcs = (mps_setting[idx].phy & 0x000000ff);
	pwr = mps_setting[idx].pwr;
	pkt_len = mps_setting[idx].pkt_len;
	pkt_cnt = mps_setting[idx].pkt_cnt;
	nss = mps_setting[idx].nss;
	pkt_bw = mps_setting[idx].pkt_bw;
	TESTMODE_SET_PARAM(pAd, band_idx, tx_mode, tx_mode);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_ant, ant_sel);
	TESTMODE_SET_PARAM(pAd, band_idx, mcs, mcs);
	TESTMODE_SET_PARAM(pAd, band_idx, nss, nss);
	TESTMODE_SET_PARAM(pAd, band_idx, per_pkt_bw, pkt_bw);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_len,	pkt_len);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TX_CNT, pkt_cnt);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXDONE_CNT, 0);
	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TXED_CNT, 0);
	ATECtrl->TxPower0 = pwr;
	TxPower.Power = pwr;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = band_idx;
	TxPower.Band_idx = Ch_Band;
	OS_SPIN_UNLOCK(&mps_cb->lock);
	ret = ate_op->SetTxPower0(pAd, TxPower);
	ATECtrl->need_set_pwr = TRUE;

	if (mps_cb->ref_idx != 1)
		ret = MT_ATEComposePkt(pAd, pate_pkt, band_idx, 0);

	mps_cb->ref_idx++;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Item[%d], PhyMode:%x, TxPath:%x, Rate:%x, PktLen:%u, PktCount:%u, Pwr:%x\n",
			  idx, tx_mode, ant_sel, mcs, pkt_len, pkt_cnt, pwr);
	return ret;
err2:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "mps_cb->ref_idx %d mps_cnt %d\n", mps_cb->ref_idx, mps_cb->mps_cnt);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "item[%d] is running\n", mps_cb->ref_idx - 1);
	OS_SPIN_UNLOCK(&mps_cb->lock);
	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "mps_cb/mps_setting NULL %p %p\n", mps_cb, mps_setting);
	return -1;
}


static INT32 MT_ATEMPSInit(RTMP_ADAPTER *pAd)
{
	INT ret = 0;
	UCHAR band_idx = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	for (band_idx = 0; band_idx < TESTMODE_BAND_NUM; band_idx++) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);

		if (mps_cb->mps_setting)
			os_free_mem(mps_cb->mps_setting);

		NdisZeroMemory(mps_cb, sizeof(*mps_cb));
		mps_cb->setting_inuse = FALSE;
		mps_cb->mps_cnt = 0;
		mps_cb->band_idx = band_idx;
		mps_cb->stat = 0;
		mps_cb->ref_idx = 1;
		mps_cb->mps_setting = NULL;
		NdisAllocateSpinLock(pAd, &mps_cb->lock);
	}
	return ret;
}


static INT32 MT_ATEMPSRelease(RTMP_ADAPTER *pAd)
{
	INT ret = 0;
	UCHAR band_idx = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	for (band_idx = 0; band_idx < TESTMODE_BAND_NUM; band_idx++) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);

		if (mps_cb->mps_setting)
			os_free_mem(mps_cb->mps_setting);

		mps_cb->mps_setting = NULL;
		mps_cb->setting_inuse = FALSE;
		mps_cb->mps_cnt = 0;
		mps_cb->stat = 0;
		mps_cb->band_idx = band_idx;
		mps_cb->ref_idx = 1;
		NdisFreeSpinLock(&mps_cb->lock);
	}
	return ret;
}


INT32 MT_SetATEMPSDump(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _HQA_MPS_CB *mps_cb = NULL;
	struct _HQA_MPS_SETTING *mps_setting = NULL;
	UINT32 i = 0;
	mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
	mps_setting = mps_cb->mps_setting;
	MTWF_PRINT("%s-band[%u]::\n", __func__, band_idx);

	if (!mps_setting)
		return -1;

	for (i = 1; i <= mps_cb->mps_cnt; i++) {
		UINT32 tx_mode = (mps_setting[i].phy & ~0xf0ffffff) >> 24;
		UINT32 path = (mps_setting[i].phy & ~0xff0000ff) >> 8;
		UINT32 rate = (mps_setting[i].phy & ~0xffffff00);
		MTWF_PRINT("Item[%d], PhyMode:%x, TxPath:%x, Rate:%x, PktLen:%u, PktCount:%u, Pwr:%x Nss:%u, Bw:%u\n",
				  i, tx_mode, path, rate, mps_setting[i].pkt_len,
				  mps_setting[i].pkt_cnt, mps_setting[i].pwr,
				  mps_setting[i].nss, mps_setting[i].pkt_bw);
	}

	return 0;
}


static INT32 MT_MPSSetParm(RTMP_ADAPTER *pAd, enum _MPS_PARAM_TYPE type, INT32 items, UINT32 *data)
{
	INT32 ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32 i = 0;
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	struct _HQA_MPS_CB *mps_cb = NULL;
	struct _HQA_MPS_SETTING *mps_setting = NULL;

	if ((items > 1024) || (items == 0))
		goto MT_MPSSetParm_RET_FAIL;

	if (mode & fATE_MPS)
		goto MT_MPSSetParm_RET_FAIL;

	mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, control_band_idx, mps_cb);

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = items;
		mps_cb->band_idx = control_band_idx;
		ret = os_alloc_mem(pAd,	(UCHAR **)&mps_cb->mps_setting, sizeof(struct _HQA_MPS_SETTING) * (items + 1));

		if (ret == NDIS_STATUS_FAILURE)
			goto MT_MPSSetParm_RET_FAIL;

		NdisZeroMemory(mps_cb->mps_setting, sizeof(struct _HQA_MPS_SETTING) * (items + 1));
	}

	mps_setting = mps_cb->mps_setting;

	if (mps_setting == NULL)
		goto MT_MPSSetParm_RET_FAIL;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%u, items:%d, Mode:%x, mps_cb:%p, mps_set:%p, mps_cnt:%d\n",
		control_band_idx, items, mode, mps_cb, mps_setting, mps_cb->mps_cnt);

	switch (type) {
	case MPS_SEQDATA:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].phy = data[i];

		break;

	case MPS_PHYMODE:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xf0ffffff;
			mps_setting[i + 1].phy |= (data[i] << 24) & 0x0f000000;
		}

		break;

	case MPS_PATH:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xff0000ff;
			mps_setting[i + 1].phy |= (data[i] << 8) & 0x00ffff00;
		}

		break;

	case MPS_RATE:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xffffff00;
			mps_setting[i + 1].phy |= (0x000000ff & data[i]);
		}

		break;

	case MPS_PAYLOAD_LEN:
		mps_setting[0].pkt_len = 1;

		for (i = 0; i < items; i++) {
			if (data[i] > MAX_TEST_PKT_LEN)
				data[i] = MAX_TEST_PKT_LEN;
			else if (data[i] < MIN_TEST_PKT_LEN)
				data[i] = MIN_TEST_PKT_LEN;

			mps_setting[i + 1].pkt_len = data[i];
		}

		break;

	case MPS_TX_COUNT:
		mps_setting[0].pkt_cnt = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pkt_cnt = data[i];

		break;

	case MPS_PWR_GAIN:
		mps_setting[0].pwr = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pwr = data[i];

		break;

	case MPS_NSS:
		mps_setting[0].nss = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].nss = data[i];

		break;

	case MPS_PKT_BW:
		mps_setting[0].pkt_bw = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pkt_bw = data[i];

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "unknown setting type\n");
		break;
	}

	return ret;
MT_MPSSetParm_RET_FAIL:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"fail, control_band_idx:%u, items:%d, Mode:%x\n",
		control_band_idx, items, mode);
	return NDIS_STATUS_FAILURE;
}


static INT32 MT_MPSTxStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _HQA_MPS_CB *mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, control_band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = mps_cb->mps_setting;
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	UINT32 mps_cnt = mps_cb->mps_cnt;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%u, items:%u\n",
		control_band_idx, mps_cnt);

	if (!mps_setting || !mps_cnt || (mode & ATE_MPS))
		goto MPS_START_ERR;

	if (mps_cb->setting_inuse)
		goto MPS_START_ERR;

	mode |= fATE_MPS;
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
	mps_cb->ref_idx = 1;
	mps_cb->setting_inuse = TRUE;
	ret = MT_SetATEMPSDump(pAd, control_band_idx);
	ret = MT_ATEMPSLoadSetting(pAd, control_band_idx);
	ret = ATEOp->tx_commit(pAd);
	ret = ATEOp->StartTx(pAd);
	return ret;
MPS_START_ERR:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "error, mode:0x%x, mps_cnt:%x, MPS_SETTING: %p\n",
			  mode, mps_cnt, mps_setting);
	return ret;
}


static INT32 MT_MPSTxStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	struct _HQA_MPS_CB *mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, control_band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = mps_cb->mps_setting;

	mode &= ~ATE_TXFRAME;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%u, Mode:%x, inuse:%x, setting_addr:%p\n",
		control_band_idx, mode, mps_cb->setting_inuse, mps_setting);

	if (!(mode & ATE_MPS) && mps_setting &&	!mps_cb->setting_inuse) {
		struct _HQA_MPS_SETTING **setting_addr = &(mps_cb->mps_setting);
		mps_cb->mps_cnt = 0;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "bf free mem %p\n", mps_setting);
		os_free_mem(*setting_addr);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "af free mem %p\n", mps_setting);
		*setting_addr = NULL;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
	return ret;
}


static INT32 MT_ATESetAutoResp(RTMP_ADAPTER *pAd, UCHAR *mac, UCHAR mode)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR *sa = NULL;
	UINT8 OwnMacIdx = 0x0;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		sa = ATECtrl->addr3[0];
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sa = ATECtrl->addr2[0];
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
#if !defined(COMPOS_TESTMODE_WIN)

#ifdef DBDC_MODE
	if (control_band_idx == TESTMODE_BAND1) {
		struct wifi_dev *wdev;
		UCHAR wdev_idx;
		/* omac is fixed, need to get from wdev_idx*/
		wdev_idx = TESTMODE_GET_PARAM(pAd, control_band_idx, wdev_idx);
		wdev = pAd->wdev_list[wdev_idx];
		OwnMacIdx = HcGetOmacIdx(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OwnMacIdx:%d\n", OwnMacIdx);
	}
#endif

	if (mode) {
		if (sa)
			NdisMoveMemory(sa, mac, MAC_ADDR_LEN);

		AsicDevInfoUpdate(
			pAd,
			OwnMacIdx,
			mac,
			control_band_idx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE);
	} else {
		AsicDevInfoUpdate(
			pAd,
			OwnMacIdx,
			pAd->CurrentAddress,
			control_band_idx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE);
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return ret;
}

static INT32 MT_EfuseGetFreeBlock(struct _RTMP_ADAPTER *pAd, PVOID GetFreeBlock, PVOID Result)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_EVENT_EFUSE_FREE_BLOCK_T *pResult =	(EXT_EVENT_EFUSE_FREE_BLOCK_T *)((UINT8 *) pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
#if defined(RTMP_EFUSE_SUPPORT)
	Ret = MtCmdEfuseFreeBlockCount(pAd, GetFreeBlock, Result);
#endif
#if	defined(COMPOS_TESTMODE_WIN)
	/* workaround for MtCmdEfuseFreeBlockCount not waiting event back when repoen QA second times */
	RTMPusecDelay(30000);
	*Result = OS_NTOHL(pResult->ucFreeBlockNum);
#endif
	return Ret;
}




static INT32 MT_RfRegWrite(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support)
		Ret = UniCmdRFRegAccessWrite(pAd, WFSel, Offset, Value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		Ret = MtCmdRFRegAccessWrite(pAd, WFSel, Offset, Value);

	return Ret;
}


static INT32 MT_RfRegRead(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_CMD_RF_REG_ACCESS_T *pResult = (EXT_CMD_RF_REG_ACCESS_T *)((UINT8 *)pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support)
		Ret = UniCmdRFRegAccessRead(pAd, WFSel, Offset, Value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		Ret = MtCmdRFRegAccessRead(pAd, WFSel, Offset, Value);
#if	defined(COMPOS_TESTMODE_WIN)
	*Value = OS_NTOHL(pResult->u4Data);
#endif
	return Ret;
}


static INT32 MT_GetFWInfo(RTMP_ADAPTER *pAd, UCHAR *FWInfo)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;
	UCHAR op_mode;
	UINT32 loop;
	UCHAR date[8] = {'\0'};
	UCHAR time[6] = {'\0'};
	UCHAR *kernel_info = NULL;
	UINT8 month = 0;
	UCHAR *month_array[12] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	/* Get information from kernel */
	for (loop = 0; loop < 12; loop++) {
		kernel_info = strstr(utsname()->version, month_array[loop]);

		if (kernel_info)
			break;
	}

	op_mode = (UCHAR)
			  pAd->OpMode;	/* 0: STA, 1: AP, 2: ADHOC, 3: APSTA */
	/* Driver build time */
	os_move_mem(&time[0], kernel_info + 7, 2);
	os_move_mem(&time[2], kernel_info + 10, 2);
	os_move_mem(&time[4], kernel_info + 13, 2);
	/* Driver build date */
	os_move_mem(&date[0], kernel_info + 20, 4);
	os_move_mem(&date[6], kernel_info + 4, 2);

	for (loop = 0; loop < 12; loop++) {
		if (os_cmp_mem(month_array[loop], kernel_info, 3) == 0) {
			month = loop + 1;
			break;
		}
	}

	date[4] = month / 10 % 10 + '0';
	date[5] = month % 10 + '0';
	/* Command combination */
	os_move_mem(FWInfo, &op_mode, sizeof(op_mode));
	os_move_mem((FWInfo + sizeof(op_mode)), &date, sizeof(date));
	os_move_mem((FWInfo + sizeof(op_mode) + sizeof(date)), &time, sizeof(time));
	os_move_mem((FWInfo + sizeof(op_mode) + sizeof(date) + sizeof(time)), \
		ctrl->fw_profile[WM_CPU].source.img_ptr + ctrl->fw_profile[WM_CPU].source.img_len - 19, 15);
	return 0;
}


#ifdef TXBF_SUPPORT
#ifdef MT_MAC
/*
 *==========================================================================
 *   Description:
 *	Enable sounding trigger
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 *==========================================================================
*/
INT MT_SetATESoundingProc(RTMP_ADAPTER *pAd, UCHAR SDEnFlg)
{
	/* struct _ATE_CTRL *AteCtrl = &pAd->ATECtrl; */
	/* Enable sounding trigger in FW */
	/* return CmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, SDEnFlg); */
	/* return MtCmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, SDEnFlg, AteCtrl->BW); */
	return -1;
}


static INT32 MT_ATEStartTxSKB(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(pAd, control_band_idx, test_pkt);
	UCHAR cntl_ch = TESTMODE_GET_PARAM(pAd, control_band_idx, ctrl_ch);
	UCHAR ch = TESTMODE_GET_PARAM(pAd, control_band_idx, channel);
	UINT32 mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TX_CNT);
	UCHAR bw = TESTMODE_GET_PARAM(pAd, control_band_idx, bw);
	/* MT_SWITCH_CHANNEL_CFG ch_cfg; */
	INT8 wdev_idx = TESTMODE_GET_PARAM(pAd, control_band_idx, wdev_idx);
#ifdef ARBITRARY_CCK_OFDM_TX
	UINT32 tx_sel = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
	UINT8 tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
#endif
	UINT32 Rate = TESTMODE_GET_PARAM(pAd, control_band_idx, mcs);
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
	union WTBL_DW5 wtbl_txcap;
#else
	union WTBL_2_DW9 wtbl_txcap;
#endif
	UINT32 DwMask = 0;
	UINT32 Ring = 0;
	CMD_WTBL_RAW_DATA_RW_T rWtblRawDataRw = {0};
	rWtblRawDataRw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"control_band_idx:%u, ch:%x, cntl_ch:%x, wdev_idx:%x\n",
		control_band_idx, ch, cntl_ch, wdev_idx);

	if (!pate_pkt)
		goto err0;

	/* TxRx swtich Recover */

	if (mode & ATE_TXFRAME)
		goto err1;

	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);
#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, control_band_idx);

		if (tx_mode == MODE_CCK || tx_mode == MODE_OFDM)
			MtATESetCCK_OFDM_Path(pAd, tx_sel, control_band_idx);
	}
#endif

	if (Rate == 32) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
		DwMask = ~(3 << 12); /* only update fcap bit[13:12] */
		wtbl_txcap.field.fcap = bw;
		/* WTBLDW5 */
		WtblDwSet(pAd, ATECtrl->wcid_ref, 1, 5, DwMask, wtbl_txcap.word);
#else
		DwMask = ~(3 << 14); /* only update fcap bit[15:14] */
		wtbl_txcap.field.fcap = bw;
		/* WTBL2DW9 */
		WtblDwSet(pAd, ATECtrl->wcid_ref, 2, 9, DwMask, wtbl_txcap.word);
#endif
	}

	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, control_band_idx);
	msleep(30);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd,
				  fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	if (mode & ATE_RXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, control_band_idx);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Prepare Tx packet */
	if (if_ops->setup_frame)
		Ret = if_ops->setup_frame(pAd, QID_AC_BE);
	else
		Ret = MT_ATEComposePkt(pAd, pate_pkt, control_band_idx, 0);

	if (Ret)
		goto err0;

	if (tx_cnt != 0xFFFFFFFF) {
#ifndef ATE_TXTHREAD
		tx_cnt += TESTMODE_GET_PARAM(pAd, control_band_idx, ATE_TXDONE_CNT);
#endif
		TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TX_CNT, tx_cnt);
	}

	/* Tx Frame */
	mode |= ATE_TXFRAME;
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);

	if (if_ops->test_frame_tx)
		Ret = if_ops->test_frame_tx(pAd);
	else {
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, FALSE, control_band_idx);

		/* Gen 512 packets that avoid SW mgme's queue full which causes tx thread to stop */
		for (Ring = 0; Ring < 512; Ring++)
			mt_ate_enq_pkt(pAd, control_band_idx, 0);

		Ret = MT_ATETxControl(pAd, control_band_idx, NULL);
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);
	}

	if (Ret)
		goto err0;

	ATECtrl->did_tx = 1;
err1:
	return Ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Err %d, wdev_idx:%x\n", Ret, wdev_idx);
	return Ret;
}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */




#ifdef RTMP_MAC_PCI
static INT32 pci_ate_init(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT ret = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (!ATECtrl->test_pkt)
		ret = os_alloc_mem(pAd,	(PUCHAR *)&ATECtrl->test_pkt, ATE_TESTPKT_LEN);

	if (ret)
		goto err0;

	chip_interrupt_enable(pAd);
	return NDIS_STATUS_SUCCESS;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Allocate test packet fail at pakcet\n");
	return NDIS_STATUS_FAILURE;
}


static INT32 pci_clean_q(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return NDIS_STATUS_SUCCESS;
}

static INT32 pci_ate_leave(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	MTWF_DBG(pAd, DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_NONE);
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd);

	if (ATECtrl->test_pkt) {
		os_free_mem(ATECtrl->test_pkt);
		ATECtrl->test_pkt = NULL;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif




#ifdef RTMP_MAC_PCI
static UINT32 MT_TestModeIfOpInit_PCI(struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = pci_ate_init;
	ATEIfOps->clean_trx_q = pci_clean_q;
	ATEIfOps->setup_frame = NULL;
	ATEIfOps->test_frame_tx = NULL;
	ATEIfOps->test_frame_rx = NULL;
	ATEIfOps->ate_leave = pci_ate_leave;
	return Status;
}
#endif /*RTMP_MAC_PCI*/
static UINT32 MT_TestModeIfOpInit_NotSupport(struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = NULL;
	ATEIfOps->clean_trx_q = NULL;
	ATEIfOps->setup_frame = NULL;
	ATEIfOps->test_frame_tx = NULL;
	ATEIfOps->test_frame_rx = NULL;
	ATEIfOps->ate_leave = NULL;
	return Status;
}

static UINT32 MT_TestModeIfOpInit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *AteCtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *ATEIfOps = NULL;

	os_alloc_mem(pAd, (PUCHAR *)&ATEIfOps, sizeof(*ATEIfOps));
#if defined(RTMP_MAC_PCI)
	if (IS_PCI_INF(pAd)) {
		MT_TestModeIfOpInit_PCI(ATEIfOps);
		goto end;
	}
#endif
	MT_TestModeIfOpInit_NotSupport(ATEIfOps);
end:
	AteCtrl->ATEIfOps = ATEIfOps;
	return Status;
}


static INT32 mt_testmode_chip_init(RTMP_ADAPTER *ad)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	struct _ATE_CTRL *ate_ctrl = &ad->ATECtrl;
	struct _ATE_CHIP_OPERATION *ate_chip_ops = NULL;
	os_alloc_mem(ad, (PUCHAR *)&ate_chip_ops, sizeof(*ate_chip_ops));

	if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad) || IS_AXE(ad))
		ate_chip_ops->fill_tx_blk = mt_ate_fill_offload_tx_blk;
	else
		ate_chip_ops->fill_tx_blk = mt_ate_fill_non_offload_tx_blk;

	ate_ctrl->ate_chip_ops = ate_chip_ops;

	return ret;
}


static UINT32 MT_TestModeOpInit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *AteOp = NULL;
	os_alloc_mem(pAd, (PUCHAR *)&AteOp, sizeof(*AteOp));
	AteOp->ATEStart = MT_ATEStart;
	AteOp->ATEStop = MT_ATEStop;
	AteOp->StartTx = MT_ATEStartTx;
	AteOp->StartRx = MT_ATEStartRx;
	AteOp->StopTx = MT_ATEStopTx;
	AteOp->StopRx = MT_ATEStopRx;
	AteOp->SetRxUserIdx = MT_ATESetRxUserIdx;
	AteOp->SetTxPower0 = MT_ATESetTxPower0;
	AteOp->SetTxPower1 = MT_ATESetTxPower1;
	AteOp->SetTxPower2 = MT_ATESetTxPower2;
	AteOp->SetTxPower3 = MT_ATESetTxPower3;
	AteOp->SetTxForceTxPower = MT_ATESetForceTxPower;
	AteOp->SetTxPowerX = MT_ATESetTxPowerX;
	AteOp->SetTxAntenna = MT_ATESetTxAntenna;
	AteOp->SetRxAntenna = MT_ATESetRxAntenna;
	AteOp->SetTxFreqOffset = MT_ATESetTxFreqOffset;
	AteOp->GetTxFreqOffset = MT_ATEGetTxFreqOffset;
	AteOp->SetChannel = MT_ATESetChannel;
	AteOp->SetBW = MT_ATESetBW;
	AteOp->SetDutyCycle = mt_ate_set_duty_cycle;
	AteOp->SetPktTxTime = mt_ate_set_pkt_tx_time;
#ifdef PRE_CAL_MT7622_SUPPORT
	if (IS_MT7622(pAd)) {
		AteOp->TxDPDTest7622 = MtATE_DPD_Cal_Store_Proc_7622;
	}
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_MT7626_SUPPORT
	AteOp->PreCal7626 = MtATE_Pre_Cal_Store_Proc_7626;
	AteOp->TxDPD7626  = MtATE_DPD_Cal_Store_Proc_7626;
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
	AteOp->PreCal7915 = MtATE_Group_Pre_Cal_Store_Proc_7915;
	AteOp->TxDPD7915  = MtATE_DPD_Cal_Store_Proc_7915;
#endif /* PRE_CAL_MT7915_SUPPORT */
#ifdef PRE_CAL_MT7986_SUPPORT
	AteOp->PreCal7986 = MtATE_Group_Pre_Cal_Store_Proc_7986;
	AteOp->TxDPD7986  = MtATE_DPD_Cal_Store_Proc_7986;
#endif /* PRE_CAL_MT7986_SUPPORT */
#ifdef PRE_CAL_MT7916_SUPPORT
	AteOp->PreCal7916 = MtATE_Group_Pre_Cal_Store_Proc_7916;
	AteOp->TxDPD7916  = MtATE_DPD_Cal_Store_Proc_7916;
#endif /* PRE_CAL_MT7916_SUPPORT */
#ifdef PRE_CAL_MT7981_SUPPORT
	AteOp->PreCal7981 = MtATE_Group_Pre_Cal_Store_Proc_7981;
	AteOp->TxDPD7981  = MtATE_DPD_Cal_Store_Proc_7981;
#endif /* PRE_CAL_MT7981_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	AteOp->RxSelfTest = MtATE_DCOC_Cal_Store_Proc;
	AteOp->TxDPDTest = MtATE_DPD_Cal_Store_Proc;
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	AteOp->PreCalTest = MtATE_Pre_Cal_Proc;
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	AteOp->PATrim = MtATE_PA_Trim_Proc;
#endif /* CAL_BIN_FILE_SUPPORT */
#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo RX_BLK */
	AteOp->SampleRssi = MT_ATESampleRssi;
#endif
#if defined(MT7986)
	AteOp->DnlK7986 = MtATE_DNL_Cal_Store_Proc_7986;
	AteOp->RXGAINK7986 = MtATE_RXGAIN_Cal_Store_Proc_7986;
#endif
#if defined(MT7916)
	AteOp->DnlK7916 = MtATE_DNL_Cal_Store_Proc_7916;
	AteOp->RXGAINK7916 = MtATE_RXGAIN_Cal_Store_Proc_7916;
#endif
#if defined(MT7981)
	AteOp->DnlK7981 = MtATE_DNL_Cal_Store_Proc_7981;
	AteOp->RXGAINK7981 = MtATE_RXGAIN_Cal_Store_Proc_7981;
#endif
	AteOp->SetIPG = mt_ate_set_ipg;
	AteOp->SetSlotTime = mt_ate_set_slot_time;
	AteOp->SetAIFS = MT_ATESetAIFS;
	AteOp->SetPowerDropLevel = MT_ATESetPowerDropLevel;
	AteOp->SetTSSI = MT_ATESetTSSI;
	AteOp->LowPower = MT_ATELowPower;
	AteOp->SetEepromToFw = MT_ATESetEepromToFw;
	AteOp->SetDPD = MT_ATESetDPD;
	AteOp->StartTxTone = MT_ATEStartTxTone;
	AteOp->DBDCTxTone = MT_ATEDBDCTxTone;
	AteOp->SetDBDCTxTonePower = MT_ATESetDBDCTxTonePower;
	AteOp->GetDBDCTxTonePower = MT_ATEGetDBDCTxTonePower;
	AteOp->SetTxTonePower = MT_ATESetTxTonePower;
	AteOp->StopTxTone = MT_ATEStopTxTone;
	AteOp->StartContinousTx = MT_ATEStartContinousTx;
	AteOp->StopContinousTx = MT_ATEStopContinousTx;
	AteOp->RfRegWrite = MT_RfRegWrite;
	AteOp->RfRegRead = MT_RfRegRead;
	AteOp->EfuseGetFreeBlock = MT_EfuseGetFreeBlock;
	AteOp->GetFWInfo = MT_GetFWInfo;
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	AteOp->SetATETxSoundingProc = MT_SetATESoundingProc;
	AteOp->StartTxSKB = MT_ATEStartTxSKB;
#endif /* MT_MAC */
	AteOp->SetICapStart = MT_ATESetICapStart;
	AteOp->GetICapStatus = MT_ATEGetICapStatus;
	AteOp->GetICapIQData = MT_ATEGetICapIQData;
#if OFF_CH_SCAN_SUPPORT
	AteOp->off_ch_scan = mt_ate_off_ch_scan;
#endif
	AteOp->MPSSetParm = MT_MPSSetParm;
	AteOp->MPSTxStart = MT_MPSTxStart;
	AteOp->MPSTxStop = MT_MPSTxStop;
	AteOp->SetAutoResp = MT_ATESetAutoResp;
	AteOp->SetFFTMode = MT_SetFFTMode;
	AteOp->onOffRDD = MT_OnOffRDD;
	AteOp->SetCfgOnOff = MT_ATESetCfgOnOff;
	AteOp->GetCfgOnOff = MT_ATEGetCfgOnOff;
	AteOp->SetRXFilterPktLen = MT_ATESetRXFilterPktLen;
	AteOp->GetTxPower = MT_ATEGetTxPower;
	AteOp->BssInfoUpdate = MT_ATEBssInfoUpdate;
	AteOp->DevInfoUpdate = MT_ATEDevInfoUpdate;
	AteOp->LogOnOff = MT_ATELogOnOff;
	AteOp->SetAntennaPort = MT_ATESetAntennaPort;
	AteOp->ClockSwitchDisable = MT_ATEFWPacketCMDClockSwitchDisable;
#if defined(DOT11_HE_AX)
	AteOp->show_ru_info = mt_ate_show_ru_info;
	AteOp->set_ru_info = mt_ate_set_ru_info;
#endif
 #if OFF_CH_SCAN_SUPPORT
	AteOp->off_ch_scan = mt_ate_off_ch_scan;
#endif
	AteOp->tx_commit = mt_ate_tx_subscribe;
	AteOp->tx_revert = mt_ate_tx_unsubscribe;


	ATECtrl->ATEOp = AteOp;

	return Status;
}


INT32 MtTestModeInit(RTMP_ADAPTER *pAd)
{
	UINT16 wcid = 0;
	INT32 Status = 0;
	mt_testmode_chip_init(pAd);
	MT_TestModeOpInit(pAd);
	MT_TestModeIfOpInit(pAd);
	pAd->ATECtrl.test_pkt = NULL;
	RTMP_OS_TASK_INIT(&pAd->LbCtrl.LoopBackTxTask, "ATE_LoopBackTask", pAd);
	/*Unify*/
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		RTMP_OS_INIT_COMPLETION(&pAd->LbCtrl.LoopBackPCITxEvent);

#endif
	RTMP_OS_INIT_COMPLETION(&pAd->LbCtrl.LoopBackEvent);
	os_zero_mem(&pAd->LbCtrl.LoopBackResult, sizeof(struct _LOOPBACK_RESULT));
	NdisAllocateSpinLock(pAd, &pAd->LbCtrl.LoopBackLock);
	pAd->LbCtrl.DebugMode = FALSE;
	pAd->LbCtrl.LoopBackRunning = FALSE;
	OS_SPIN_LOCK(&pAd->LbCtrl.LoopBackLock);
	pAd->LbCtrl.LoopBackWaitRx = FALSE;
	OS_SPIN_UNLOCK(&pAd->LbCtrl.LoopBackLock);
	pAd->LbCtrl.LoopBackUDMA = FALSE;
#ifdef ATE_TXTHREAD
	pAd->ATECtrl.tx_thread[0].is_init = FALSE;
#endif
	for (wcid = 0; wcid < MAX_MULTI_TX_STA ; wcid++)
		pAd->ATECtrl.pkt_skb[wcid] = NULL;

	os_zero_mem(pAd->ATECtrl.tx_method, sizeof(UCHAR)*TX_MODE_MAX);
	pAd->ATECtrl.tx_method[MODE_HE_MU] = 1;
	pAd->ATECtrl.tx_method[MODE_VHT_MIMO] = 1;
	return Status;
}


INT32 MtTestModeExit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *AteOp = NULL;
	struct _ATE_IF_OPERATION *ATEIfOps = NULL;
	struct _ATE_CHIP_OPERATION *ate_chip_ops = NULL;

	AteOp = ATECtrl->ATEOp;
	ATEIfOps = ATECtrl->ATEIfOps;
	ate_chip_ops = ATECtrl->ate_chip_ops;

	os_free_mem(ATEIfOps);
	os_free_mem(AteOp);
	os_free_mem(ate_chip_ops);
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		RTMP_OS_EXIT_COMPLETION(&pAd->LbCtrl.LoopBackPCITxEvent);

#endif
	RTMP_OS_EXIT_COMPLETION(&pAd->LbCtrl.LoopBackEvent);
	NdisFreeSpinLock(&pAd->LbCtrl.LoopBackLock);

	return Status;
}


INT32 MtATECh2Freq(UINT32 Channel, UINT32 band_idx)
{
	UINT32 Freq = 0;

	switch (band_idx) {
	case 0:
		if (Channel >= 1 && Channel <= 13)
			Freq = 2407 + Channel * 5;
		else if (Channel == 14)
			Freq = 2484;

		break;

	case 1:
		if (Channel >= 7 && Channel <= 181)
			Freq = 5000 + Channel * 5;
		else if (Channel >= 184 && Channel <= 196)
			Freq = 4000 + Channel * 5;
		else if (Channel == 6)
			Freq = 5032;
		else if (Channel == 182)
			Freq = 4912;
		else if (Channel == 183)
			Freq = 4917;

		break;

	default:
		break;
	}

	return Freq;
}


INT32 MtATEGetTxPwrGroup(UINT32 Channel, UINT32 band_idx, UINT32 Ant_idx)
{
	UINT32 Group = 0;
	UINT32 Freq = MtATECh2Freq(Channel, band_idx);
	UINT32 i;
	UINT32 NumOfMap = (sizeof(txpwr_group_map) / sizeof(struct _ATE_TXPWR_GROUP_MAP));

	for (i = 0; i < NumOfMap; ++i) {
		if (Freq > txpwr_group_map[i].start && Freq <= txpwr_group_map[i].end) {
			Group = txpwr_group_map[i].group[Ant_idx];
			break;
		}
	}

	return Group;
}

INT32 MtATERSSIOffset(PRTMP_ADAPTER pAd, INT32 rssi_orig, UINT32 ant_idx, INT32 ch_band)
{
	UCHAR rssi_offset = 0;
	UINT32 *band_offset = NULL;
	UINT32 offset = 0;

	if (ch_band > eeprom_rssi_offset.n_band - 1)
		return rssi_orig;

	if (eeprom_rssi_offset.rssi_eeprom_band_offset[ch_band])
		band_offset = eeprom_rssi_offset.rssi_eeprom_band_offset[ch_band];
	else
		return rssi_orig;

	if ((ant_idx >
		 eeprom_rssi_offset.n_band_offset[ch_band] - 1))
		return rssi_orig;
	else if (band_offset[ant_idx] == 0)
		return rssi_orig;

	offset = band_offset[ant_idx];
	if (pAd->EEPROMImage)
	rssi_offset = pAd->EEPROMImage[offset];

	if (rssi_offset & (1 << 7) && rssi_offset != 0xFF) {
		if (rssi_offset & (1 << 6))
			return rssi_orig + (rssi_offset & 0x3f);
		else
			return rssi_orig - (rssi_offset & 0x3f);
	}

	return rssi_orig;
}
#ifdef PRE_CAL_MT7622_SUPPORT
INT MtATE_DPD_Cal_Store_Proc_7622(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4CalId;
	UINT8 i = 0;
	USHORT doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;

	if (IS_MT7622(pAd)) {

		if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Currently not in FLASH or BIN MODE,return.\n");
			return FALSE;
		}
		/* set channel command , per group calibration - set to channel 7, BW20 */
		ch_cfg.Bw = BW_20;
		ch_cfg.CentralChannel = 7;
		ch_cfg.TxStream = 4;
		ch_cfg.RxStream = 4;
		ch_cfg.ControlChannel = 7;
		ch_cfg.ControlChannel2 = 0;
		ch_cfg.BandIdx = 0;
		ch_cfg.bScan = 0;
		MtCmdChannelSwitch(pAd, ch_cfg);

		pAd->bPreCalMode = TRUE;
		/* Retest Recal - TXLPFG */
		u4CalId = TX_LPFG;
		MtCmdRfTestRecal(pAd, u4CalId, TX_LPFG_RESP_LEN);

		/* Retest Recal - TXDCIQ */
		u4CalId = TX_DCIQC;
		MtCmdRfTestRecal(pAd, u4CalId, TX_DCIQ_RESP_LEN);

		pAd->TxDpdCalOfst = 0;
		RTMPZeroMemory(pAd->CalTXDPDImage, CAL_TXDPD_SIZE);

		/* Retest Recal - TXDPD */
		for (i = 1; i <= 14; i++) {
			ch_cfg.CentralChannel = i;
			ch_cfg.ControlChannel = i;
			MtCmdChannelSwitch(pAd, ch_cfg);
			u4CalId = TX_DPD_LINK;
			MtCmdRfTestRecal(pAd, u4CalId, TX_DPD_LINK_RESP_LEN);
		}

		/* raise DoCalibrate bits */
		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			rtmp_ee_flash_read(pAd, 0x52, &doCal1);
#endif
		/* raise bit3 */
		doCal1 |= (1 << 3);
		if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			rtmp_ee_bin_write16(pAd, 0x52, doCal1);
			rtmp_ee_write_to_bin(pAd);      /* writeback to eeprom file */
		}
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			rtmp_ee_flash_write(pAd, 0x52, doCal1);
#endif
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "raised E2P 0x52 = %x\n", doCal1);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "====================\n");
		pAd->bPreCalMode = FALSE;
	}
	return TRUE;
}
#endif /* PRE_CAL_MT7622_SUPPORT */

#ifdef PRE_CAL_MT7626_SUPPORT
INT MtATE_Pre_Cal_Store_Proc_7626(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT                doCal1 = 0;
	ULONG                 x = simple_strtol(arg, 0, 10);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	pAd->PreCalOfst = 0;
	RTMPZeroMemory(pAd->PreCalImageInfo, 16);
	RTMPZeroMemory(pAd->PreCalImage, PRE_CAL_TOTAL_SIZE);

	if (x == 0) {
		/* Clear 0x32[0] */
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			/* Clear bit0 */
			rtmp_ee_flash_read(pAd, 0x32, &doCal1);

			doCal1 = doCal1 & ~(BIT(0));
			rtmp_ee_flash_write(pAd, 0x32, doCal1);
		}
#endif
	} else if (x == 1) {
		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, 0x32, &doCal1);

		       doCal1 |= (1 << 0);
			rtmp_ee_flash_write(pAd, 0x32, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}
	msleep(100);
	pAd->bPreCalMode = FALSE;
	return TRUE;
}

INT MtATE_DPD_Cal_Store_Proc_7626(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8                 i = 0;
	ULONG                 x = simple_strtol(arg, 0, 10);
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

    if (x == 0) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, DPD_CAL_TOTAL_SIZE);

		/* Clear 0x32[1:2] */
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, 0x32, &doCal1);

			/* Clear bit1&2 */
			doCal1 = doCal1 & ~(BITS(1, 2));
			rtmp_ee_flash_write(pAd, 0x32, doCal1);
		}
#endif
    } else if (x == 1) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, DPD_CAL_TOTAL_SIZE);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < MT7626_DPD_FLATNESS_ABAND_BW20_CH_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 36, 52, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7626_DPD_FLATNESS_ABAND_BW20_CH[i];
			ch_cfg.TxStream = 3;
			ch_cfg.RxStream = 3;
			ch_cfg.ControlChannel = MT7626_DPD_FLATNESS_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;
			MtCmdChannelSwitch(pAd, ch_cfg);
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<28), 1);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, 0x32, &doCal1);

		       doCal1 |= (1 << 1);
			rtmp_ee_flash_write(pAd, 0x32, doCal1);
		}
#endif
	} else if (x == 2) {
		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7626_DPD_FLATNESS_GBAND_BW20_CH_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7626_DPD_FLATNESS_GBAND_BW20_CH[i];
			ch_cfg.TxStream = 3;
			ch_cfg.RxStream = 3;
			ch_cfg.ControlChannel = MT7626_DPD_FLATNESS_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 0;
			MtCmdChannelSwitch(pAd, ch_cfg);
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<28), 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, 0x32, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << 2);
			rtmp_ee_flash_write(pAd, 0x32, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}
	msleep(100);
	pAd->bPreCalMode = FALSE;
}
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
VOID MtATE_Dump_Group_PreCal_7915(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->PreCalImage;
	UINT32 cal_size_tmp;
	struct _RTMP_CHIP_CAP *cap = NULL;
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	cal_size_tmp = cap->prek_ee_info.cal_result_size +
				cap->prek_ee_info.cal_result_size_5g +
				cap->prek_ee_info.cal_result_size_6g;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Group Pre-Cal: \n");

	for (i = 0; i < cal_size_tmp / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

VOID MtATE_Dump_DPD_PreCal_7915(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->TxDPDImage;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"DPD/Flatness Pre-Cal: \n");

	for (i = 0; i < cap->prek_ee_info.dpd_cal_total_size / 4; i += 4)	{
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

INT MtATE_Group_Pre_Cal_Store_Proc_7915(RTMP_ADAPTER *pAd, UINT8 op)
{
	USHORT doCal1 = 0;
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	if (op == PREK_GROUP_CLEAN) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			/* Clear bit0 */
			rtmp_ee_flash_read(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			/* Clear bit0 */
			pChipOps->eeread(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT));
			pChipOps->eewrite(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here ! Mode[%d]\n", pAd->E2pAccessMode);
		}
	} else if (op == PREK_GROUP_DUMP) {
		MtATE_Dump_Group_PreCal_7915(pAd);
	} else if (op == PREK_GROUP_PROC) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise group pre-cal indication bit */
			doCal1 |= (1 << GROUP_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			pChipOps->eeread(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise group pre-cal indication bit */
			doCal1 |= (1 << GROUP_PRECAL_INDN_BIT);
			pChipOps->eewrite(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here ! Mode[%d]\n", pAd->E2pAccessMode);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here ! op[%d]\n", op);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, MT7915_PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}

INT MtATE_DPD_Cal_Store_Proc_7915(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT8                 i = 0;
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
#ifdef DBDC_MODE
	u_int32 band0_tx_path_backup, band0_rx_path_backup;
	u_int32 band1_tx_path_backup, band1_rx_path_backup;
#endif
	RTMP_CHIP_OP *pChipOps = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;
	pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

    if (op == PREK_DPD_CLEAN) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* Clear DPD5G and DPD2G indication bit */
			doCal1 = doCal1 & ~(BITS(DPD5G_PRECAL_INDN_BIT, DPD2G_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			pChipOps->eeread(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* Clear DPD5G and DPD2G indication bit */
			doCal1 = doCal1 & ~(BITS(DPD5G_PRECAL_INDN_BIT, DPD2G_PRECAL_INDN_BIT));
			pChipOps->eewrite(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here ! Mode[%d]\n", pAd->E2pAccessMode);
		}
    } else if (op == PREK_DPD_DUMP) {
		MtATE_Dump_DPD_PreCal_7915(pAd);
	} else if (op == PREK_DPD_5G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < MT7915_DPD_FLATNESS_ABAND_BW20_CH_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 36, 52, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7915_DPD_FLATNESS_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7915_DPD_FLATNESS_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0x4;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0x2;
				ch_cfg.RxStream = 0x3;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0x4;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x2;
				ch_cfg.RxStream = 0x3;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<28), 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<28), 0);
		}

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			pChipOps->eeread(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
			pChipOps->eewrite(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here ! Mode[%d]\n", pAd->E2pAccessMode);
		}
	} else if (op == PREK_DPD_2G_PROC) {
		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7915_DPD_FLATNESS_GBAND_BW20_CH_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7915_DPD_FLATNESS_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7915_DPD_FLATNESS_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 0;

			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0x4;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x2;
				ch_cfg.RxStream = 0x3;
#ifdef DBDC_MODE
				band0_tx_path_backup = pAd->dbdc_band0_tx_path;
				band0_rx_path_backup = pAd->dbdc_band0_rx_path;
				pAd->dbdc_band0_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band0_rx_path = ch_cfg.RxStream;
#endif
			}

			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band0_tx_path = band0_tx_path_backup;
				pAd->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0x4;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x2;
				ch_cfg.RxStream = 0x3;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<28), 0);
		}

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			pChipOps->eeread(pAd, MT7915_PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
			pChipOps->eewrite(pAd, MT7915_PRECAL_INDICATION_BYTE, doCal1);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here ! Mode[%d]\n", pAd->E2pAccessMode);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here ! op[%d]\n", op);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, MT7915_PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
VOID MtATE_Dump_Group_PreCal_7986(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->PreCalImage;

	UINT32 totalsize = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;
	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->eeprom_sku & (MT7986_ADIE_MT7976_TYPE_MASK))
		totalsize = (cap->prek_ee_info.cal_result_size + cap->prek_ee_info.cal_result_size_6g);
	else
		totalsize = (cap->prek_ee_info.cal_result_size + cap->prek_ee_info.cal_result_size_5g);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Group Pre-Cal: \n");

	for (i = 0; i < totalsize / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

VOID MtATE_Dump_DPD_PreCal_7986(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->TxDPDImage;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"DPD/Flatness Pre-Cal: \n");

	for (i = 0; i < cap->prek_ee_info.dpd_cal_total_size / 4; i += 4)	{
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

INT MtATE_Group_Pre_Cal_Store_Proc_7986(RTMP_ADAPTER *pAd, UINT8 op)
{
	USHORT doCal1 = 0;
	UINT32 totalsize = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->eeprom_sku & (MT7986_ADIE_MT7976_TYPE_MASK))
		totalsize = cap->prek_ee_info.pre_cal_total_size_7976;
	else
		totalsize = cap->prek_ee_info.pre_cal_total_size;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	if (op == PREK_GROUP_CLEAN) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, totalsize);
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			/* Clear bit0 */
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_GROUP_DUMP) {
		MtATE_Dump_Group_PreCal_7986(pAd);
	} else if (op == PREK_GROUP_PROC) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, totalsize);

		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise group pre-cal indication bit */
			doCal1 |= (1 << GROUP_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}

INT MtATE_DPD_Cal_Store_Proc_7986(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT8                 i = 0;
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
#ifdef DBDC_MODE
	u_int32 band0_tx_path_backup, band0_rx_path_backup;
	u_int32 band1_tx_path_backup, band1_rx_path_backup;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE, return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

    if (op == PREK_DPD_CLEAN) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* Clear DPD5G and DPD2G indication bit */
			doCal1 = doCal1 & ~(BITS(DPD5G_PRECAL_INDN_BIT, DPD2G_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
    } else if (op == PREK_DPD_DUMP) {
		MtATE_Dump_DPD_PreCal_7986(pAd);
	} else if (op == PREK_DPD_5G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a5_begin, cap->prek_ee_info.dpd_cal_5g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < (MT7986_PER_CH_A5_BW20_BW160_SIZE); i++) {
			/* set channel command , per group calibration - set to channel 36, 52, BW20 */
			if (i < MT7986_PER_CH_A5_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7986_PER_CH_A5_BW20[i];
				ch_cfg.ControlChannel = MT7986_PER_CH_A5_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7986_PER_CH_A5_BW160[i - MT7986_PER_CH_A5_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7986_PER_CH_A5_BW160[i - MT7986_PER_CH_A5_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0xC;
				ch_cfg.RxStream = 0xC;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 0xC;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_6G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a6_begin,
		cap->prek_ee_info.dpd_cal_6g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < (MT7986_PER_CH_A6_BW20_BW160_SIZE); i++) {
			if (i < MT7986_PER_CH_A6_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7986_PER_CH_A6_BW20[i];
				ch_cfg.ControlChannel = MT7986_PER_CH_A6_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7986_PER_CH_A6_BW160[i - MT7986_PER_CH_A6_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7986_PER_CH_A6_BW160[i - MT7986_PER_CH_A6_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 2;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 3;
				ch_cfg.RxStream = 0x7;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD6G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_2G_PROC) {
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_g_begin, cap->prek_ee_info.dpd_cal_2g_total_size);
		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7986_PER_CH_G_BW20_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7986_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel = MT7986_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 0;

			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
#ifdef DBDC_MODE
				band0_tx_path_backup = pAd->dbdc_band0_tx_path;
				band0_rx_path_backup = pAd->dbdc_band0_rx_path;
				pAd->dbdc_band0_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band0_rx_path = ch_cfg.RxStream;
#endif
			}

			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band0_tx_path = band0_tx_path_backup;
				pAd->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 0x3;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
VOID MtATE_Dump_Group_PreCal_7916(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->PreCalImage;
	UINT32 cal_size_tmp;
	struct _RTMP_CHIP_CAP *cap = NULL;
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	cal_size_tmp = cap->prek_ee_info.cal_result_size +
				cap->prek_ee_info.cal_result_size_6g;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Group Pre-Cal: \n");

	for (i = 0; i < cal_size_tmp / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

VOID MtATE_Dump_DPD_PreCal_7916(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->TxDPDImage;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"DPD/Flatness Pre-Cal: \n");

	for (i = 0; i < cap->prek_ee_info.dpd_cal_total_size / 4; i += 4)	{
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

INT MtATE_Group_Pre_Cal_Store_Proc_7916(RTMP_ADAPTER *pAd, UINT8 op)
{
	USHORT doCal1 = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	if (op == PREK_GROUP_CLEAN) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			/* Clear bit0 */
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_GROUP_DUMP) {
		MtATE_Dump_Group_PreCal_7916(pAd);
	} else if (op == PREK_GROUP_PROC) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise group pre-cal indication bit */
			doCal1 |= (1 << GROUP_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}

INT MtATE_DPD_Cal_Store_Proc_7916(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT8                 i = 0;
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
#ifdef DBDC_MODE
	u_int32 band0_tx_path_backup, band0_rx_path_backup;
	u_int32 band1_tx_path_backup, band1_rx_path_backup;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

    if (op == PREK_DPD_CLEAN) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* Clear DPD5G and DPD2G indication bit */
			doCal1 = doCal1 & ~(BITS(DPD5G_PRECAL_INDN_BIT, DPD2G_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
    } else if (op == PREK_DPD_DUMP) {
		MtATE_Dump_DPD_PreCal_7916(pAd);
	} else if (op == PREK_DPD_5G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a5_begin, cap->prek_ee_info.dpd_cal_5g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < MT7916_PER_CH_A5_BW20_BW160_SIZE; i++) {
			if (i < MT7916_PER_CH_A5_BW20_SIZE) {
			/* set channel command , per group calibration - set to channel 36, 52, BW20 */
			    ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7916_PER_CH_A5_BW20[i];
				ch_cfg.ControlChannel = MT7916_PER_CH_A5_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7916_PER_CH_A5_BW160[i - MT7916_PER_CH_A5_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7916_PER_CH_A5_BW160[i - MT7916_PER_CH_A5_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0xC;
				ch_cfg.RxStream = 0xC;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 0xC;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_6G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a6_begin,
		cap->prek_ee_info.dpd_cal_6g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < (MT7916_PER_CH_A6_BW20_BW160_SIZE); i++) {
			if (i < MT7916_PER_CH_A6_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7916_PER_CH_A6_BW20[i];
				ch_cfg.ControlChannel = MT7916_PER_CH_A6_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7916_PER_CH_A6_BW160[i - MT7916_PER_CH_A6_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7916_PER_CH_A6_BW160[i - MT7916_PER_CH_A6_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 2;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x7;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD6G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_2G_PROC) {
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_g_begin, cap->prek_ee_info.dpd_cal_2g_total_size);
		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7916_PER_CH_G_BW20_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7916_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel = MT7916_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 0;

			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
#ifdef DBDC_MODE
				band0_tx_path_backup = pAd->dbdc_band0_tx_path;
				band0_rx_path_backup = pAd->dbdc_band0_rx_path;
				pAd->dbdc_band0_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band0_rx_path = ch_cfg.RxStream;
#endif
			}

			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band0_tx_path = band0_tx_path_backup;
				pAd->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 0x3;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}
#endif /* PRE_CAL_MT7916_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
VOID MtATE_Dump_Group_PreCal_7981(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->PreCalImage;
	UINT32 cal_size_tmp;
	struct _RTMP_CHIP_CAP *cap = NULL;
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	cal_size_tmp = cap->prek_ee_info.cal_result_size +
				cap->prek_ee_info.cal_result_size_6g;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Group Pre-Cal:\n");

	for (i = 0; i < cal_size_tmp / 4; i += 4) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

VOID MtATE_Dump_DPD_PreCal_7981(RTMP_ADAPTER *pAd)
{
	UINT32 i = 0;
	UINT32 *ptr = (UINT32 *)pAd->TxDPDImage;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"DPD/Flatness Pre-Cal:\n");

	for (i = 0; i < cap->prek_ee_info.dpd_cal_total_size / 4; i += 4)	{
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[0x%08x] 0x%8x 0x%8x 0x%8x 0x%8x\n",
			i * 4, ptr[i], ptr[i+1], ptr[i+2], ptr[i+3]);
	}
}

INT MtATE_Group_Pre_Cal_Store_Proc_7981(RTMP_ADAPTER *pAd, UINT8 op)
{
	USHORT doCal1 = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	/* This flag is used for receive N9 Firmware re-cal event */
	pAd->bPreCalMode = TRUE;

	if (op == PREK_GROUP_CLEAN) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);
#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			/* Clear bit0 */
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			doCal1 = doCal1 & ~(BIT(GROUP_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_GROUP_DUMP) {
		MtATE_Dump_Group_PreCal_7981(pAd);
	} else if (op == PREK_GROUP_PROC) {
		pAd->PreCalOfst = 0;
		RTMPZeroMemory(pAd->PreCalImageInfo, 16);
		RTMPZeroMemory(pAd->PreCalImage, cap->prek_ee_info.pre_cal_total_size);

		/* Execute pre-k(no dpd) and apply */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<29), 0);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise group pre-cal indication bit */
			doCal1 |= (1 << GROUP_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}

INT MtATE_DPD_Cal_Store_Proc_7981(RTMP_ADAPTER *pAd, UINT8 op)
{
	UINT8                 i = 0;
	USHORT                doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
#ifdef DBDC_MODE
	u_int32 band0_tx_path_backup, band0_rx_path_backup;
	u_int32 band1_tx_path_backup, band1_rx_path_backup;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	pAd->bPreCalMode = TRUE;

	if (op == PREK_DPD_CLEAN) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage, cap->prek_ee_info.dpd_cal_total_size);

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* Clear DPD5G and DPD2G indication bit */
			doCal1 = doCal1 & ~(BITS(DPD5G_PRECAL_INDN_BIT, DPD2G_PRECAL_INDN_BIT));
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_DUMP) {
		MtATE_Dump_DPD_PreCal_7981(pAd);
	} else if (op == PREK_DPD_5G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a5_begin, cap->prek_ee_info.dpd_cal_5g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify and need pay attention to the index */
		for (i = 0; i < (MT7981_PER_CH_A5_BW20_BW160_SIZE); i++) {
			/* set channel command , per group calibration - set to channel 36, 52, BW20 */
			if (i < MT7981_PER_CH_A5_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7981_PER_CH_A5_BW20[i];
				ch_cfg.ControlChannel = MT7981_PER_CH_A5_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7981_PER_CH_A5_BW160[i - MT7981_PER_CH_A5_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7981_PER_CH_A5_BW160[i - MT7981_PER_CH_A5_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 3;
				ch_cfg.RxStream = 0x7;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A5, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD5G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_6G_PROC) {
		/* Clear TXDPD Image */
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_a6_begin,
		cap->prek_ee_info.dpd_cal_6g_total_size);

		/* 5G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < (MT7981_PER_CH_A6_BW20_BW160_SIZE); i++) {
			if (i < MT7981_PER_CH_A6_BW20_SIZE) {
				ch_cfg.Bw = BW_20;
				ch_cfg.CentralChannel = MT7981_PER_CH_A6_BW20[i];
				ch_cfg.ControlChannel = MT7981_PER_CH_A6_BW20[i];
			} else {
				ch_cfg.Bw = BW_160;
				ch_cfg.CentralChannel =
				MT7981_PER_CH_A6_BW160[i - MT7981_PER_CH_A6_BW20_SIZE];
				ch_cfg.ControlChannel =
				MT7981_PER_CH_A6_BW160[i - MT7981_PER_CH_A6_BW20_SIZE];
			}
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 1;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 2;

			/* Sw Ch in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 0x3;
				ch_cfg.RxStream = 0x3;
				ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
				band1_tx_path_backup = pAd->dbdc_band1_tx_path;
				band1_rx_path_backup = pAd->dbdc_band1_rx_path;
				pAd->dbdc_band1_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band1_tx_path = band1_tx_path_backup;
				pAd->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 3;
				ch_cfg.RxStream = 0x7;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);

			if (IS_ATE_DBDC(pAd))
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 1);
			else
				MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL_A6, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit2 */
			doCal1 |= (1 << DPD6G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else if (op == PREK_DPD_2G_PROC) {
		pAd->TxDPDOfst = 0;
		RTMPZeroMemory(pAd->TxDPDImage + cap->prek_ee_info.dpd_flash_offset_g_begin, cap->prek_ee_info.dpd_cal_2g_total_size);

		/* 2.4G DPD + Flatness Calibration */
		NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));

		/* If want to debug, we can use only ch6 to verify and need pay attention to the index */
		for (i = 0; i < MT7981_PER_CH_G_BW20_SIZE; i++) {
			/* set channel command , per group calibration - set to channel 1, 6, 11, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7981_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel = MT7981_PER_CH_G_BW20[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 0;

			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 0x2;
				ch_cfg.RxStream = 0x2;
#ifdef DBDC_MODE
				band0_tx_path_backup = pAd->dbdc_band0_tx_path;
				band0_rx_path_backup = pAd->dbdc_band0_rx_path;
				pAd->dbdc_band0_tx_path = ch_cfg.TxStream;
				pAd->dbdc_band0_rx_path = ch_cfg.RxStream;
#endif
			}

			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd)) {
#ifdef DBDC_MODE
				pAd->dbdc_band0_tx_path = band0_tx_path_backup;
				pAd->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
			}

			/* T/Rx Path in Test Mode */
			/* T/Rx number bring T/Rx path bit-wise */
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 0xF;
				ch_cfg.RxStream = 0xF;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 0x3;
			}
			MtCmdSetTxRxPath(pAd, ch_cfg);
			MtCmdDoCalibration(pAd, RE_CALIBRATION, TX_DPD_FLATNESS_CAL, 0);
		}

#ifdef RTMP_FLASH_SUPPORT
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			rtmp_ee_flash_read(pAd, PRECAL_INDICATION_BYTE, &doCal1);

			/* raise bit1 */
			doCal1 |= (1 << DPD2G_PRECAL_INDN_BIT);
			rtmp_ee_flash_write(pAd, PRECAL_INDICATION_BYTE, doCal1);
		}
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Should not be here !\n");
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"op=%d, 0x%2x=%d\n", op, PRECAL_INDICATION_BYTE, doCal1);

	pAd->bPreCalMode = FALSE;
	return TRUE;
}
#endif /* PRE_CAL_MT7981_SUPPORT */

#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT MtATE_DPD_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG x = simple_strtol(arg, 0, 10);
	TXDPD_RESULT_T TxDpdResult;
	BOOLEAN toCR = FALSE; /* CR to Flash/Bin file */
	UINT16 CentralFreq = 0;
	UINT8 BW = 0;
	UINT8 i = 0;
	UINT8 j = 0;
	ULONG CalOffset = 0;
	USHORT doCal1 = 0;
	UINT8 RetryTimes = 5;
	BOOLEAN DPDPassOrFail = TRUE;
	BOOLEAN kABand = TRUE;
	BOOLEAN kGBand = TRUE;

	if (x == 0) { /* 2G */
		kABand = FALSE;
		kGBand = TRUE;
	} else if (x == 1) { /* 5G */
		kABand = TRUE;
		kGBand = FALSE;
	} else { /* all K */
		kABand = TRUE;
		kGBand = TRUE;
	}

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}
	/* Stop TX RX MAC */
	SetATE(pAd, "TXSTOP");
	SetATE(pAd, "RXSTOP");

	/* TXDPD ABand */
	if (kABand) {
		RTMPZeroMemory(pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);

		/* TXDPD A20  */
		for (i = 0; i < DPD_A20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = DPD_A20Freq[i];
			CalOffset = i * TXDPD_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&TxDpdResult, sizeof(TXDPD_RESULT_T));
				MtCmdGetTXDPDCalResult(pAd, toCR, CentralFreq, BW, ABAND, FALSE, FALSE, &TxDpdResult);
				RtmpusecDelay(10);

				if (TxDpdResult.ResultSuccess)
					break;
			}

			if (TxDpdResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "==========TX A20 P1 freq %d save to flash offset %lx ========\n", CentralFreq, DPDPART1_OFFSET + CalOffset);
				ShowDPDData(pAd, TxDpdResult);
				memcpy(pAd->CalDPDAPart1Image + CalOffset, &TxDpdResult.u4DPDG0_WF0_Prim, TXDPD_SIZE);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  A20 freq %d TX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DPDPassOrFail = FALSE;
				goto exit;
			}
		}

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDPDAPart1Image,
				get_dev_eeprom_offset(pAd) + DPDPART1_OFFSET, TXDPD_PART1_LIMIT * TXDPD_SIZE);

#endif
		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_write_to_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_PART1_LIMIT * TXDPD_SIZE);

		RtmpusecDelay(20000);
	}
	/* TXDPD G20 */
	if (kGBand) {
		RTMPZeroMemory(pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);

		for (i = 0; i < DPD_G20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = DPD_G20Freq[i];
			CalOffset = i * TXDPD_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&TxDpdResult, sizeof(TXDPD_RESULT_T));
				MtCmdGetTXDPDCalResult(pAd, toCR, CentralFreq, BW, GBAND, FALSE, FALSE, &TxDpdResult);
				RtmpusecDelay(10);

				if (TxDpdResult.ResultSuccess)
					break;
			}

			if (TxDpdResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "==========TX G20 freq %d save to flash offset %lx ========\n", CentralFreq, DPDPART1_OFFSET + DPD_A20_SIZE * TXDPD_SIZE + CalOffset);
				ShowDPDData(pAd, TxDpdResult);
				memcpy(pAd->CalDPDAPart2Image + CalOffset, &TxDpdResult.u4DPDG0_WF0_Prim, TXDPD_SIZE);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  G20 freq %d TX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DPDPassOrFail = FALSE;
				goto exit;
			}
		}

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDPDAPart2Image,
				get_dev_eeprom_offset(pAd) + DPDPART2_OFFSET, (DPD_G20_SIZE * TXDPD_SIZE));

#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_write_to_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, (DPD_G20_SIZE * TXDPD_SIZE));
	}

	/* raise DoCalibrate bits */
	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);

#endif
	doCal1 |= (1 << 0);
	/* raise bit 4 to denote 16 entry TXDPD */
	doCal1 |= (1 << 4);

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		rtmp_ee_bin_write16(pAd, 0x52, doCal1);
		rtmp_ee_write_to_bin(pAd);		/* writeback to eeprom file */
	}

#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_write(pAd, 0x52, doCal1);

#endif
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "raised E2P 0x52 = %x\n", doCal1);

	/* reload test -- for debug only */
	if (pAd->KtoFlashDebug) {
		ULONG CalOffset = 0;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\n######################### reload caldata for debug ####################################\n");
		RtmpusecDelay(20000);

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDPDAPart1Image,
				get_dev_eeprom_offset(pAd) + DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE);
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDPDAPart2Image,
				get_dev_eeprom_offset(pAd) + DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE);
		}
#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE);
			rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE);
		}

		/* Find flash offset base on CentralFreq */
		for (i = 0; i < DPD_ALL_SIZE; i++) {
			CalOffset = i * TXDPD_SIZE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "reload flash offset [%lx]  freq [%d]=================\n",
					  CalOffset + DPDPART1_OFFSET, DPD_AllFreq[i]);

			if (i < DPD_A20_SIZE)
				memcpy(&TxDpdResult.u4DPDG0_WF0_Prim, pAd->CalDPDAPart1Image + CalOffset, TXDPD_SIZE);
			else {
				CalOffset = (i - DPD_A20_SIZE) * TXDPD_SIZE;
				memcpy(&TxDpdResult.u4DPDG0_WF0_Prim, pAd->CalDPDAPart2Image + CalOffset, TXDPD_SIZE);
			}

			ShowDPDData(pAd, TxDpdResult);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "raised E2P 0x52 = %x\n", doCal1);
exit:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "====================\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TX_SELF_TEST : [%s]\n",
			  (DPDPassOrFail == TRUE) ? "PASS" : "FAIL");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "====================\n");
	return TRUE;
}


INT MtATE_DCOC_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG x = simple_strtol(arg, 0, 10);
	RXDCOC_RESULT_T RxDcocResult;
	BOOLEAN toCR = FALSE;
	UINT16 CentralFreq = 0;
	UINT8 BW = 0;
	UINT8 i = 0;
	UINT8 j = 0;
	ULONG CalOffset = 0;
	USHORT doCal1 = 0;
	UINT8 RetryTimes = 5;
	BOOLEAN DCOCPassOrFail = TRUE;
	BOOLEAN kABand = TRUE;
	BOOLEAN kGBand = TRUE;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	RTMPZeroMemory(pAd->CalDCOCImage, DCOC_IMAGE_SIZE);

	if (x == 0) { /* 2G */
		kABand = FALSE;
		kGBand = TRUE;
	} else if (x == 1) { /* 5G */
		kABand = TRUE;
		kGBand = FALSE;
	} else { /* all K */
		kABand = TRUE;
		kGBand = TRUE;
	}
	 /* Disable RMAC */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND1);

	/* RXDCOC ABand */
	if (kABand) {
		for (i = 0; i < K_A20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = K_A20Freq[i];
			CalOffset = i * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "========== A20 freq %d save to offset %lx ========\n",
						  CentralFreq, DCOC_OFFSET + CalOffset);
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				ShowDCOCData(pAd, RxDcocResult);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  A20 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}

		for (i = 0; i < K_A40_SIZE; i++) {
			BW = BW_40;
			CentralFreq = K_A40Freq[i];
			CalOffset = (K_A20_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq,	BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "========== A40 freq %d save to offset %lx ========\n",
						  CentralFreq, DCOC_OFFSET + CalOffset);
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  A40 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}

		for (i = 0; i < K_A80_SIZE; i++) {
			BW = BW_80;
			CentralFreq = K_A80Freq[i];
			CalOffset = (K_A20_SIZE + K_A40_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq,	BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "========== A80 freq %d save to offset %lx ========\n", CentralFreq, DCOC_OFFSET + CalOffset);
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  A80 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}
	}

	/* RXDCOC GBand */
	if (kGBand) {
		for (i = 0; i < K_G20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = K_G20Freq[i];
			CalOffset = (K_A20_SIZE + K_A40_SIZE + K_A80_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, BW, GBAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "========== G20 freq %d save to offset %lx ========\n", CentralFreq, DCOC_OFFSET + CalOffset);
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0],	RXDCOC_SIZE);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "!!!!!!  G20 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes);
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "========== save to offset %x size %d========\n"
#ifdef RTMP_FLASH_SUPPORT
			  get_dev_eeprom_offset(pAd) +
#endif
			  DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDCOCImage,
			get_dev_eeprom_offset(pAd) + DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_cal_write_to_bin(pAd, pAd->CalDCOCImage, DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

	/* raise DoCalibrate bits */
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

	doCal1 |= (1 << 1);
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_write(pAd, 0x52, doCal1);

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		rtmp_ee_bin_write16(pAd, 0x52, doCal1);
		rtmp_ee_write_to_bin(
			pAd);		/* XXX: remember to writeback modified eeprom to file */
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "raised E2P 0x52 = %x\n", doCal1);

	if (pAd->KtoFlashDebug) {
		ULONG CalOffset = 0;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "######################### reload caldata for debug ####################################\n");
		RtmpusecDelay(20000);
#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDCOCImage,
				get_dev_eeprom_offset(pAd) + DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_load_from_bin(pAd, pAd->CalDCOCImage, DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

		/* Find offset base on CentralFreq */
		for (i = 0; i < K_ALL_SIZE; i++) {
			CalOffset = i * RXDCOC_SIZE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "reload from offset [%lx]  freq [%d]=================\n",
					  CalOffset + DCOC_OFFSET, K_AllFreq[i]);
			memcpy(&RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0],
				   pAd->CalDCOCImage + CalOffset, RXDCOC_SIZE);
			ShowDCOCData(pAd, RxDcocResult);
		}
	}

exit:
	/* Enable RMAC */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX, TRUE, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, TRUE, TESTMODE_BAND1);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "====================\n");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "RX_SELF_TEST : [%s]\n",
			  (DCOCPassOrFail == TRUE) ? "PASS" : "FAIL");
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "====================\n");
	return TRUE;
}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT MtATE_Pre_Cal_Proc(RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Cal Id = %d, ChGrpId = %d\n", CalId, ChGrpId);
	/* Initialization */
	pAd->PreCalWriteOffSet = 0;
	pAd->ChGrpMap = PreCalGroupList[ChGrpId];

	if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
		pAd->PreCalStoreBuffer = pAd->EEPROMImage + PRECALPART_OFFSET;
	} else {
		ret = os_alloc_mem(pAd, &pAd->PreCalStoreBuffer, PRE_CAL_SIZE_ONE_CARD);

		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Not enough memory for pre-cal stored buffer!!\x1b[m\n");
		}
	}

	if (ret == NDIS_STATUS_SUCCESS)
		MtCmdGetPreCalResult(pAd, PreCalItemList[CalId], PreCalGroupList[ChGrpId]);

	return TRUE;
}
#endif/* PRE_CAL_TRX_SET2_SUPPORT */


#if defined(MT7986)
INT MtATE_RXGAIN_Cal_Store_Proc_7986(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	/* RXGAINKCLEAN */
	if (x == 0) {
		/* Clear RXGAINK Image */
		pAd->RXGainCalOfst = 0;
		os_zero_mem(pAd->RXGainCal, RXGAIN_CAL_SIZE);
	} else if (x == 1) {
		pAd->RXGainCalOfst = 0;

		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;
		/* Get RX GAIN Calibration result */
		MtCmdDoCalibration(pAd, DO_RX_GAIN_CAL, (1<<24), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}

INT MtATE_DNL_Cal_Store_Proc_7986(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	UINT8                 i = 0;
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	if (x == 0) {
		/* Clear DNLK Image */
		pAd->DnlCalOfst = 0;
		os_zero_mem(pAd->TxDnlCal, DNL_CAL_SIZE);

		/* Clear TSSI Image */
		pAd->TssiCal2GOfst = 0;
		os_zero_mem(pAd->TssiCal2G, TSSI_CAL_2G_SIZE);
		pAd->TssiCal5GOfst = 0;
		os_zero_mem(pAd->TssiCal5G, TSSI_CAL_5G_SIZE);

	} else if (x == 1) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal2GOfst = 0;

		/* 2.4G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 2.4G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch6 to verify */
		/* and need pay attention to the index */
		for (i = 0; i < MT7986_DNL_CAL_GBAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 1, 7, 13, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7986_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7986_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 2G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<25), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	} else if (x == 2) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal5GOfst = 0;

		/* 5G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 5G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < MT7986_DNL_CAL_ABAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 36, 52, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7986_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7986_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
				ch_cfg.BandIdx = 1;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 5G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<26), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}
#endif

#if defined(MT7916)
INT MtATE_RXGAIN_Cal_Store_Proc_7916(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	/* RXGAINKCLEAN */
	if (x == 0) {
		/* Clear RXGAINK Image */
		pAd->RXGainCalOfst = 0;
		os_zero_mem(pAd->RXGainCal, RXGAIN_CAL_SIZE);
	} else if (x == 1) {
		pAd->RXGainCalOfst = 0;

		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;
		/* Get RX GAIN Calibration result */
		MtCmdDoCalibration(pAd, DO_RX_GAIN_CAL, (1<<24), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}

INT MtATE_DNL_Cal_Store_Proc_7916(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	UINT8                 i = 0;
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	if (x == 0) {
		/* Clear DNLK Image */
		pAd->DnlCalOfst = 0;
		os_zero_mem(pAd->TxDnlCal, DNL_CAL_SIZE);

		/* Clear TSSI Image */
		pAd->TssiCal2GOfst = 0;
		os_zero_mem(pAd->TssiCal2G, TSSI_CAL_2G_SIZE);
		pAd->TssiCal5GOfst = 0;
		os_zero_mem(pAd->TssiCal5G, TSSI_CAL_5G_SIZE);

	} else if (x == 1) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal2GOfst = 0;

		/* 2.4G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 2.4G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch6 to verify */
		/* and need pay attention to the index */
		for (i = 0; i < MT7916_DNL_CAL_GBAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 1, 7, 13, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7916_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7916_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 2G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<25), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	} else if (x == 2) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal5GOfst = 0;

		/* 5G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 5G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < MT7916_DNL_CAL_ABAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 36, 52, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7916_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7916_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
				ch_cfg.BandIdx = 1;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 5G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<26), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}
#endif

#if defined(MT7981)
INT MtATE_RXGAIN_Cal_Store_Proc_7981(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	/* RXGAINKCLEAN */
	if (x == 0) {
		/* Clear RXGAINK Image */
		pAd->RXGainCalOfst = 0;
		os_zero_mem(pAd->RXGainCal, RXGAIN_CAL_SIZE);
	} else if (x == 1) {
		pAd->RXGainCalOfst = 0;

		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;
		/* Get RX GAIN Calibration result */
		MtCmdDoCalibration(pAd, DO_RX_GAIN_CAL, (1<<24), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}

INT MtATE_DNL_Cal_Store_Proc_7981(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	UINT8                 i = 0;
	ULONG                 x = simple_strtol(arg, 0, 10);
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	if (pAd->E2pAccessMode != E2P_FLASH_MODE &&
		pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Currently not in FLASH or BIN MODE,return.\n");
		return FALSE;
	}

	if (IS_ATE_DBDC(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Single Band\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Dual Band\n");
	}

	if (x == 0) {
		/* Clear DNLK Image */
		pAd->DnlCalOfst = 0;
		os_zero_mem(pAd->TxDnlCal, DNL_CAL_SIZE);

		/* Clear TSSI Image */
		pAd->TssiCal2GOfst = 0;
		os_zero_mem(pAd->TssiCal2G, TSSI_CAL_2G_SIZE);
		pAd->TssiCal5GOfst = 0;
		os_zero_mem(pAd->TssiCal5G, TSSI_CAL_5G_SIZE);

	} else if (x == 1) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal2GOfst = 0;

		/* 2.4G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 2.4G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch6 to verify */
		/* and need pay attention to the index */
		for (i = 0; i < MT7981_DNL_CAL_GBAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 1, 7, 13, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7981_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7981_DNL_CAL_GBAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.BandIdx = 0;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 2G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<25), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	} else if (x == 2) {
		pAd->DnlCalOfst = 0;
		pAd->TssiCal5GOfst = 0;

		/* 5G DNL Calibration */
		os_zero_mem(&ch_cfg, sizeof(ch_cfg));

		ATECtrl->op_mode |= fATE_IN_RFTEST;

		/* Execute 5G DNL + TSSI Calibration */
		/* If want to debug, we can use only ch36 to verify*/
		/* and need pay attention to the index */
		for (i = 0; i < MT7981_DNL_CAL_ABAND_BW20_CH_SIZE; i++) {
			/* set channel command */
			/* per group cal - set to channel 36, 52, BW20 */
			ch_cfg.Bw = BW_20;
			ch_cfg.CentralChannel = MT7981_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel = MT7981_DNL_CAL_ABAND_BW20_CH[i];
			ch_cfg.ControlChannel2 = 0;
			ch_cfg.bScan = 0;
			ch_cfg.Channel_Band = 1;
			ch_cfg.bDnlCal = TRUE;
			if (IS_ATE_DBDC(pAd) == FALSE) {
				ch_cfg.TxStream = 4;
				ch_cfg.RxStream = 4;
				ch_cfg.BandIdx = 0;
			} else {
				ch_cfg.TxStream = 2;
				ch_cfg.RxStream = 2;
				ch_cfg.BandIdx = 1;
			}
			MtCmdChannelSwitch(pAd, ch_cfg);
			if (IS_ATE_DBDC(pAd) == FALSE)
				ch_cfg.RxStream = 0xf;
			else
				ch_cfg.RxStream = 0x3;
			MtCmdSetTxRxPath(pAd, ch_cfg);
		}

		/* Get DNL Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<27), 0);

		/* Get TSSI 5G Calibration result */
		MtCmdDoCalibration(pAd, RE_CALIBRATION, (1<<26), 0);

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Should not be here !\n");
	}

	return TRUE;
}
#endif

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
INT MtATE_PA_Trim_Proc(RTMP_ADAPTER *pAd, PUINT32 pData)
{
	UINT16 DoPATrim;
	UINT16 WriteAddr;
	UINT8 idx;
	USHORT *pStoreData = (USHORT *)pData;
	USHORT value;

	if (IS_MT7615(pAd)) {
		WriteAddr = PA_TRIM_START_ADDR1;
		for (idx = 0; idx < PA_TRIM_BLOCK_SIZE; idx++) {
			value = *pStoreData;
			RT28xx_EEPROM_WRITE16(pAd, WriteAddr, value);
			WriteAddr += 2;
			pStoreData++;
		}

		WriteAddr = PA_TRIM_START_ADDR2;
		for (idx = 0; idx < PA_TRIM_BLOCK_SIZE; idx++) {
			value = *pStoreData;
			RT28xx_EEPROM_WRITE16(pAd, 	WriteAddr, value);
			WriteAddr += 2;
			pStoreData++;
		}

		/* Raise DoPATrim bits */
		RT28xx_EEPROM_READ16(pAd, 0x52, DoPATrim);
		DoPATrim |= (1 << 3);
		RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPATrim);

	}
			return TRUE;
}
#endif/* CAL_BIN_FILE_SUPPORT */

INT32 MtATETssiTrainingProc(RTMP_ADAPTER *pAd, UCHAR ucBW, UCHAR ucBandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;

	/* only applu TSSI Training for MT7615 */
	if (IS_MT7615(pAd)) {
		if (ucBW == BW_8080 || ucBW == BW_160) {
			/* Check iPA or ePA status */
			MtEPAcheck(pAd);

			/* TSSI Training only for IPA case */
			if (!pAd->fgEPA) {
				UCHAR PerPktBW, tx_mode, Mcs, Nss;
				UINT32 TxLength;

				/* Backup parameters for Phymode, BW, Rate, VHTNss, TxLength */
				PerPktBW = TESTMODE_GET_PARAM(pAd, ucBandIdx, per_pkt_bw);
				tx_mode  = TESTMODE_GET_PARAM(pAd, ucBandIdx, tx_mode);
				Mcs 	 = TESTMODE_GET_PARAM(pAd, ucBandIdx, mcs);
				Nss 	 = TESTMODE_GET_PARAM(pAd, ucBandIdx, nss);
				TxLength = TESTMODE_GET_PARAM(pAd, ucBandIdx, tx_len);

				/* Config TSSI Tracking enable */
				MtATETSSITracking(pAd, TRUE);
				/* Config FCBW ON */
				MtATEFCBWCfg(pAd, TRUE);
				/* Config DBW 80MHz */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, per_pkt_bw, BW_80);
				/* Config VHT mode */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_mode, MODE_VHT);
				/* Config MCS rate */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, mcs, MCS_9);
				/* Config 4 Nss */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, nss, 4);
				/* Config Tx packet length */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_len, 100);
				/* Start Tx for 25ms */
				ATEOp->tx_commit(pAd);
				ATEOp->StartTx(pAd);
				RtmpOsMsDelay(25);
				/* Stop Tx */
				ATEOp->StopTx(pAd);
				ATEOp->tx_revert(pAd);
				/* Save compensation value to Global variable (FCBW on case) */
				MtTSSICompBackup(pAd, TRUE);
				/* Config FCBW OFF */
				MtATEFCBWCfg(pAd, FALSE);
				/* Config DBW 160MHz */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, per_pkt_bw, BW_160);
				/* Config VHT mode */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_mode, MODE_VHT);
				/* Config MCS rate */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, mcs, MCS_9);
				/* Config 2 Nss */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, nss, 2);
				/* Config Tx packet length */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_len, 100);
				/* Start Tx for 25ms */
				ATEOp->tx_commit(pAd);
				ATEOp->StartTx(pAd);
				RtmpOsMsDelay(25);
				/* Stop Tx */
				ATEOp->StopTx(pAd);
				ATEOp->tx_revert(pAd);
				/* Save compensation value to Global variable (FCBW off case) */
				MtTSSICompBackup(pAd, FALSE);
				/* Config Compensation CR */
				MtTSSICompCfg(pAd);
				/* Config TSSI Tracking disable */
				MtATETSSITracking(pAd, FALSE);
				/* Config FCBW ON */
				MtATEFCBWCfg(pAd, TRUE);

				/* Resotre paratemters for Phymode, BW, Rate, VHTNss, TxLength */
				TESTMODE_SET_PARAM(pAd, ucBandIdx, per_pkt_bw, PerPktBW);
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_mode, tx_mode);
				TESTMODE_SET_PARAM(pAd, ucBandIdx, mcs, Mcs);
				TESTMODE_SET_PARAM(pAd, ucBandIdx, nss, Nss);
				TESTMODE_SET_PARAM(pAd, ucBandIdx, tx_len, TxLength);

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TSSI Training Done!!\n");
			}
		}
	}

	return ret;
}

#if defined(DOT11_HE_AX)
static INT32 mt_ate_show_ru_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT16 sta_idx = 0;
	struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[0]);
	struct _MAC_TABLE_ENTRY_STACK *stack = (struct _MAC_TABLE_ENTRY_STACK *)TESTMODE_GET_PADDR(ad, band_idx, stack);
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	UINT16 wcid = 0;

	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->mac_tbl_entry[sta_idx];

		if (ru_sta[sta_idx].valid) {
			if (mac_tbl_entry)
				wcid = mac_tbl_entry->wcid;
			MTWF_PRINT("(%s) RU index[%d] in Segment[%d]: WCID[%d](0: N/A) TX mcs[%d],nss[%d], ldpc[%d], mpdu length:%d, RU MU Nss:%d\n",
							__func__, (ru_sta[sta_idx].ru_index >> 1), (ru_sta[sta_idx].ru_index & 0x1), wcid,
							ru_sta[sta_idx].rate, ru_sta[sta_idx].nss, ru_sta[sta_idx].ldpc,
							ru_sta[sta_idx].mpdu_length, ru_sta[sta_idx].ru_mu_nss);
		}
	}

	return ret;
}

static UINT8 mt_ate_get_sub_band(UINT32 ru_index)
{
	UINT8 sub_band_idx = 0;

	if (ru_index == 68 || ru_index == 67)
		sub_band_idx = 0;
	else if (ru_index > 64)
		sub_band_idx = ((ru_index % 65) * 2);
	else if (ru_index > 60)
		sub_band_idx = (ru_index % 61);
	else if (ru_index > 52)
		sub_band_idx = ((ru_index % 53) >> 1);
	else if (ru_index > 36)
		sub_band_idx = ((ru_index % 37) >> 2);
	else
		sub_band_idx = (ru_index / 9);

	return sub_band_idx;
}

INT32 mt_ate_add_allocation(struct _ATE_RU_ALLOCATION *alloc_info, UINT8 allocation, UINT8 seg, UINT32 ru_index)
{
	UINT32 ret = 0;
	UINT8 sub_band_idx = 0;

	sub_band_idx = mt_ate_get_sub_band(ru_index) + seg*4;
	if (sub_band_idx > 7)
		return FALSE;
	if ((alloc_info->allocation[sub_band_idx] != allocation) && (allocation != 0x7f)) {	/* 0x7f is center-26 tone, should be ignored */
		if (alloc_info->allocation[sub_band_idx] == 0xff) {
			alloc_info->allocation[sub_band_idx] = allocation;

			if (allocation == 0xc8) {/* D3.1, Table 28-24, 0xc8 is 484-tone */
				if (sub_band_idx + 1 > 7)
					return FALSE;
				alloc_info->allocation[sub_band_idx + 1] = 0x72;	/* D3.1, Table 28-24, 0x72 is 484-empty-tone */
			} else if (allocation == 0xd0) {/* D3.1, Table 28-24, 0xd0 is 996-tone */
				if (sub_band_idx + 3 > 7)
					return FALSE;
				alloc_info->allocation[sub_band_idx + 1] = 0x73;	/* D3.1, Table 28-24, 0x73 is 484-empty-tone */
				alloc_info->allocation[sub_band_idx + 2] = 0x73;
				alloc_info->allocation[sub_band_idx + 3] = 0x73;
			}
		}
	}

	return ret;
}

INT32 mt_ate_fill_empty_allocation(struct _ATE_RU_ALLOCATION *alloc_info)
{
	UINT32 ret = 0, alloc_idx = 0;

	for (alloc_idx = 0 ; alloc_idx < sizeof(*alloc_info) ; alloc_idx++) {
		if (alloc_info->allocation[alloc_idx] == 0xff)
			alloc_info->allocation[alloc_idx] = 0x71;	/* D3.1, Table 28-24, 0x71 is 242-empty */
	}

	return ret;
}

static INT32 mt_ate_set_ru_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UCHAR *str)
{
	INT32 ret = NDIS_STATUS_FAILURE, allocation = 0, tone_idx = 0;
	UINT32 rv;
	UINT8 i = 0;
	struct _ATE_RU_ALLOCATION *ru_allocation = (struct _ATE_RU_ALLOCATION *)TESTMODE_GET_PADDR(ad, band_idx, ru_alloc);
	struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[0]);
	UCHAR *value = NULL;

	if (strlen(str) > 0) {
		NdisZeroMemory(ru_sta, sizeof(struct _ATE_RU_STA)*MAX_MULTI_TX_STA);

		for (i = 0, value = (CHAR *)rstrtok((char *)str, ":"); value; value = (CHAR *)rstrtok((char *)NULL, ":"), i++) {
			if (strlen(value) > 0) {
				rv = sscanf(value, "%4x-%d-%d-%d-%d-%d-%d-%d-%d-%d", &allocation, &ru_sta[i].aid, &ru_sta[i].ru_index,
										&ru_sta[i].rate, &ru_sta[i].ldpc, &ru_sta[i].nss,
										&ru_sta[i].start_sp_st, &ru_sta[i].mpdu_length, &ru_sta[i].alpha, &ru_sta[i].ru_mu_nss);
				if (rv != 10) {
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
					return FALSE;
				}
				ru_sta[i].valid = TRUE;

				if (ru_sta[i].mpdu_length == 0)
					ru_sta[i].mpdu_length = TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_len);

				mt_ate_add_allocation(ru_allocation, allocation, (ru_sta[i].ru_index & 0x1), ru_sta[i].ru_index);

				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ru_segment[%d]: allocation:%04x, ru_idx:%d, mpdu length:%d, alpha:%d\n\t\t\trate:0x%x, ldpc:%d, nss:%d, ru_mu_nss:%d\n",
					  (ru_sta[i].ru_index & 0x1), allocation, (ru_sta[i].ru_index >> 1), ru_sta[i].mpdu_length, ru_sta[i].alpha,
					  ru_sta[i].rate, ru_sta[i].ldpc, ru_sta[i].nss, ru_sta[i].ru_mu_nss);
			} else {
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid format, %s ignored\n", value);
				MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"<1.allocation>-<2.aid>-<3.ru_index>-<4.rate>-<5.lepc>-<6.nss>-"
					"<7.start_sp_st>-<8.mpdu_length>-<9.alpha>-<10.ru_mu_nss>\n");
				goto out;
			}
		}

		mt_ate_fill_empty_allocation(ru_allocation);

		for (tone_idx = 0 ; tone_idx < sizeof(*ru_allocation) ; tone_idx++)
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "allocation[%d] = 0x%x\n", tone_idx, ru_allocation->allocation[tone_idx]);

		ret = NDIS_STATUS_SUCCESS;
	}

out:
	if (ret == NDIS_STATUS_FAILURE)
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid input string\n");

	return ret;
}
#endif
