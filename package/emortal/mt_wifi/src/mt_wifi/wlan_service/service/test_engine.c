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
	test_engine.c
*/
#include "test_engine.h"

static u_int8 sigext_time_list[] = {
	0,			/* CCK */
	6,			/* OFDM */
	6,			/* HTMIX */
	6,			/* HTGREENFIELD */
	6,			/* VHT */
};

static u_int16 slot_time_list[] = {
	9,			/* CCK */
	9,			/* OFDM */
	9,			/* HTMIX */
	9,			/* HTGREENFIELD */
	9,			/* VHT */
};

struct test_data_rate_map cck_mode_map[] = {
	{0, 1000},
	{1, 2000},
	{2, 5500},
	{3, 11000},
	{9, 2000},
	{10, 5500},
	{11, 11000},
};

struct test_data_rate_map ofdm_mode_map[] = {
	{0, 6000},
	{1, 9000},
	{2, 12000},
	{3, 18000},
	{4, 24000},
	{5, 36000},
	{6, 48500},
	{7, 54000},
};

struct test_data_rate_map n_mode_map[] = {
	{0, 6500},
	{1, 13000},
	{2, 19500},
	{3, 26000},
	{4, 39000},
	{5, 52000},
	{6, 58500},
	{7, 65000},
	{32, 6000},		/* MCS32 */
};

struct test_data_rate_map ac_mode_map_bw20[] = {
	{0, 65},		/* in unit of 100k */
	{1, 130},
	{2, 195},
	{3, 260},
	{4, 390},
	{5, 520},
	{6, 585},
	{7, 650},
	{8, 780},
};

struct test_data_rate_map ac_mode_map_bw40[] = {
	{0, 135},		/* in unit of 100k */
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

struct test_data_rate_map ac_mode_map_bw80[] = {
	{0, 293},		/* in unit of 100k */
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

struct test_data_rate_map ac_mode_map_bw160[] = {
	{0, 585},		/* in unit of 100k */
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

/* phy rates comes from NSS = 1, Long GI */
static struct test_data_rate_map he_su_mode_map_bw20[] = {
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

static struct test_data_rate_map he_su_mode_map_bw40[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi},
	 *nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}
	 */
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


static struct test_data_rate_map he_su_mode_map_bw80[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi},
	 * nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}
	 */
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

static struct test_data_rate_map he_su_mode_map_bw160[] = {
	/*index, nss1{0.8gi, 1.6gi, 3.2gi}, nss2{0.8gi, 1.6gi, 3.2gi},
	 *nss3{0.8gi, 1.6gi, 3.2gi}, nss4{0.8gi, 1.6gi, 3.2gi}
	 */
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

static struct test_datalen_limit_map datalen_limit[] = {
	{TEST_MODE_CCK,			2304},
	{TEST_MODE_OFDM,		2304},
	{TEST_MODE_HTMIX,		7935},
	{TEST_MODE_HTGREENFIELD,	7935},
	{TEST_MODE_VHT,			11454},
	{0},	/* reserved */
	{0},	/* reserved */
	{0},	/* reserved */
	{TEST_MODE_HE_SU,		11454},
	{TEST_MODE_HE_ER,		11454},
	{TEST_MODE_HE_MU,		11454},
	{TEST_MODE_HE_TB,		11454},
	{TEST_MODE_VHT_MIMO,		11454},
};

static u_int8 test_he_bpscs[] = {
	1, 2, 2, 4, 4, 6, 6, 6, 8, 8, 10, 10	/* MCS0~11 */
};

static u_int8 test_he_rate_density[] = {
	2, 2, 4, 2, 4, 3, 4, 6, 4, 6, 4, 6	/* MCS0~11 */
};

static u_int8 test_ltf_sym[] = {
	0, 1, 2, 4, 4, 6, 6, 8, 8	/* SS 1~8 */
};

static u_int8 test_he_t_ltf_sym_x5[] = {
	24, 40, 80			/* 3.2+1.6 us, 6.4+1.6, 12.8+3.2 */
};

static u_int8 test_he_t_sym_x5[] = {
	68, 72, 80			/* base GI, double GI, quadruple GI */
};

static u_int8 test_he_t_pe_x5[] = {
	0, 20, 40, 60, 80		/* 0us, 4us, 8us, 12us, 16us */
};

static struct test_he_ru_const test_ru_const[] = {
	{37, 24, 12, 6, 2},
	{53, 48, 24, 12, 6},
	{61, 102, 51, 24, 12},
	{65, 234, 117, 60, 30},
	{67, 468, 234, 120, 60},
	{68, 980, 490, 240, 120},
	{69, 1960, 980, 492, 246}
};

/*****************************************************************************
 *	Internal functions
 *****************************************************************************/
static s_int32 mt_engine_init_mps(
	struct test_configuration *configs)
{
	struct test_mps_cb *mps_cb = NULL;

	mps_cb = &configs->mps_cb;

	if (mps_cb->mps_setting)
		sys_ad_free_mem(mps_cb->mps_setting);

	sys_ad_zero_mem(mps_cb, sizeof(*mps_cb));
	mps_cb->setting_inuse = FALSE;
	mps_cb->mps_cnt = 0;
	mps_cb->stat = 0;
	mps_cb->ref_idx = 1;
	mps_cb->mps_setting = NULL;
	SERV_OS_ALLOCATE_SPIN_LOCK(&mps_cb->lock);

	return SERV_STATUS_SUCCESS;
}

static s_int32 mt_engine_release_mps(
	struct test_configuration *configs)
{
	struct test_mps_cb *mps_cb = NULL;

	mps_cb = &configs->mps_cb;

	if (mps_cb->mps_setting)
		sys_ad_free_mem(mps_cb->mps_setting);

	mps_cb->mps_setting = NULL;
	mps_cb->setting_inuse = FALSE;
	mps_cb->mps_cnt = 0;
	mps_cb->stat = 0;
	mps_cb->ref_idx = 1;
	SERV_OS_FREE_SPIN_LOCK(&mps_cb->lock);

	return SERV_STATUS_SUCCESS;
}

static u_int8 mt_engine_get_sigext_time_by_phymode(u_char tx_mode)
{
	u_int8 sigext_time = 0;

	switch (tx_mode) {
	case TEST_MODE_CCK:
		sigext_time = sigext_time_list[TEST_MODE_CCK];
		break;

	case TEST_MODE_OFDM:
		sigext_time = sigext_time_list[TEST_MODE_OFDM];
		break;

	case TEST_MODE_HTMIX:
		sigext_time = sigext_time_list[TEST_MODE_HTMIX];
		break;

	case TEST_MODE_HTGREENFIELD:
		sigext_time = sigext_time_list[TEST_MODE_HTGREENFIELD];
		break;

	case TEST_MODE_VHT:
		sigext_time = sigext_time_list[TEST_MODE_VHT];
		break;

	default:
		sigext_time = sigext_time_list[TEST_MODE_OFDM];
		break;
	}

	return sigext_time;
}

static u_int16 mt_engine_get_slot_time_by_phymode(u_char tx_mode)
{
	u_int16 slot_time = 0;

	switch (tx_mode) {
	case TEST_MODE_CCK:
		slot_time = slot_time_list[TEST_MODE_CCK];
		break;

	case TEST_MODE_OFDM:
		slot_time = slot_time_list[TEST_MODE_OFDM];
		break;

	case TEST_MODE_HTMIX:
		slot_time = slot_time_list[TEST_MODE_HTMIX];
		break;

	case TEST_MODE_HTGREENFIELD:
		slot_time = slot_time_list[TEST_MODE_HTGREENFIELD];
		break;

	case TEST_MODE_VHT:
		slot_time = slot_time_list[TEST_MODE_VHT];
		break;

	default:
		slot_time = slot_time_list[TEST_MODE_OFDM];
		break;
	}

	return slot_time;
}

static u_int8 mt_engine_trans_gi(u_int8 tx_mode, u_int32 ltf_gi)
{
	u_int8 gi_type = 0;

	if (tx_mode == TEST_MODE_HE_SU ||
		tx_mode == TEST_MODE_HE_ER ||
		tx_mode == TEST_MODE_HE_MU) {
		switch (ltf_gi) {
		case 1:
		case 2:
			gi_type = TEST_GI_8+(ltf_gi-1);
			break;
		case 3:
		case 4:
			gi_type = TEST_GI_8+((ltf_gi-3) ? 0:2);
			break;
		default:
			gi_type = TEST_GI_8;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_TB) {
		switch (ltf_gi) {
		case 2:
			gi_type = TEST_GI_32;
			break;
		default:
			gi_type = TEST_GI_16;
			break;
		}
	} else	/* for non-HE PPDU types, gi equavalent to Sgi. */
		gi_type = ltf_gi;

	return gi_type;
}

static u_int8 mt_engine_trans_ltf(u_int8 tx_mode, u_int32 ltf_gi)
{
	u_int8 ltf_type = 0;

	if (tx_mode == TEST_MODE_HE_SU || tx_mode == TEST_MODE_HE_ER) {
		switch (ltf_gi) {
		case 1:
		case 2:
			ltf_type = TEST_HE_LTF_X2;
			break;
		case 3:
		case 4:
			ltf_type = TEST_HE_LTF_X4;
			break;
		default:
			ltf_type = TEST_HE_LTF_X1;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_MU) {
		switch (ltf_gi) {
		case 0:
		case 3:
			ltf_type = TEST_HE_LTF_X4;
			break;
		default:
			ltf_type = TEST_HE_LTF_X2;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_TB) {
		ltf_type = ltf_gi;
	} else	/* for non-HE PPDU types, ltf is not required. */
		ltf_type = 0;

	return ltf_type;
}


static u_int16 mt_engine_get_cw(u_int32 ipg, u_int16 slot_time)
{
	u_int32 cnt = 0, val;

	val = (ipg + slot_time) / slot_time;

	while (val >>= 1)
		cnt++;

	if (cnt >= TEST_MAX_CW)
		cnt = TEST_MAX_CW;

	return cnt;
}

static s_int32 mt_engine_calc_duty_cycle(struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct ipg_param *ipg_param;
	struct tx_time_param *tx_time_param;
	u_int32 ipg, pkt_tx_time, duty_cycle;

	ipg_param = &configs->ipg_param;
	tx_time_param = &configs->tx_time_param;

	ipg = ipg_param->ipg;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	duty_cycle = configs->duty_cycle;

	/* Calculate needed ipg/pkt_tx_time and duty_cycle */
	if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg == 0)) {
		/* TODO: need to consider this case in the future */
		duty_cycle = 0;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
			("%s: there are no pkt_tx_time/ipg!! ",
			__func__));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("use default setting and set duty_cycle=%d\n",
			duty_cycle));
	} else if ((duty_cycle > 0) && (pkt_tx_time > 0) && (ipg == 0)) {
		ipg = ((pkt_tx_time * 100) / duty_cycle) - pkt_tx_time;
		ipg_param->ipg = ipg;
		/*
		 * If ipg value does't make sense,
		 * there's error handle when get ipg parameter
		 */
		ret = mt_engine_calc_ipg_param_by_ipg(configs);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: calculate ipg_param fail!!\n",
				__func__));
		}
	} else if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg > 0)) {
		/*
		 * If pkt_tx_time does't make sense,
		 * there's error handle when start tx
		 */
		pkt_tx_time = (duty_cycle * ipg) / (100 - duty_cycle);
	} else {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("%s: already existed pkt_tx_time/ipg, ",
			__func__));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("can't set duty_cycle!! "));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("expected duty_cycle=%d%%\n", duty_cycle));

		duty_cycle = (pkt_tx_time * 100) / (pkt_tx_time + ipg);
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("%s: real duty_cycle=%d%%\n", __func__, duty_cycle));
	}

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: duty_cycle=%d%%, ipg=%dus, pkt_tx_time=%dus\n",
		__func__, duty_cycle, ipg, pkt_tx_time));

	tx_time_param->pkt_tx_time = pkt_tx_time;
	configs->duty_cycle = duty_cycle;

	return ret;
}

static s_int32 mt_engine_map_subcarriers(
	u_int8 ru_index, u_int8 dcm)
{
	s_int32 subcarriers = 0, idx = 0;

	for (idx = 0 ; idx < SERV_ARRAY_SIZE(test_ru_const) ; idx++) {
		if (ru_index < test_ru_const[idx].max_index) {
			if (dcm)
				subcarriers = test_ru_const[idx].sd_d;
			else
				subcarriers = test_ru_const[idx].sd;

			break;
		}
	}

	return subcarriers;
}

static s_int32 mt_engine_map_subcarriers_short(
	u_int8 ru_index, u_int8 dcm)
{
	s_int32 subcarriers_short = 0, idx = 0;

	for (idx = 0 ; idx < SERV_ARRAY_SIZE(test_ru_const) ; idx++) {
		if (ru_index < test_ru_const[idx].max_index) {
			if (dcm)
				subcarriers_short = test_ru_const[idx].sd_s_d;
			else
				subcarriers_short = test_ru_const[idx].sd_s;

			break;
		}
	}

	return subcarriers_short;
}

static s_int32 mt_engine_calc_bytes_by_time(
	u_char tx_mode, u_char nss, u_char t_pe, u_char ltf,
	u_char gi, s_int32 dbps, s_int32 tx_time)
{
	u_int8 m_stbc = 1, tail = 6;
	u_int32 data_time = 0;
	u_int32 symbol_cnt = 0, psdu_length = 0;
	u_int32 ltf_time = test_ltf_sym[nss]*test_he_t_ltf_sym_x5[ltf];

	data_time = tx_time*5;
	data_time -= (5 * 20 + (20+40+40+ltf_time) + test_he_t_pe_x5[t_pe]);
	symbol_cnt = engine_ceil(data_time, test_he_t_sym_x5[gi]);

	if (symbol_cnt > 0x3fff)	/* H/W limitation */
		symbol_cnt = 0x3fff;

	psdu_length = ((symbol_cnt / m_stbc) * dbps - 16 - tail) / 8;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t%s: \tsymbol=%d, PSDU length:%d\n",
		__func__, symbol_cnt, psdu_length));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t: \t(0x3fff is H/W limiation)\n"));

	return psdu_length;
}

static s_int32 mt_engine_calc_txlen(
	u_char tx_mode, u_char sgi,
	struct test_tx_info *tx_info,
	u_int32 tx_time,
	struct test_ru_info *ru_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 txlen = 0, tx_data_rate = 0;
	u_char mcs, mcs_1ss, nss = 1;
	u_char bw = 0, bw_fact = 1;

	/*
	 * 1. Get the tx data rate
	 * 2. Get the packet tx time
	 * 3. Calculate the packet length by tx_data_rate and packet_tx_time
	 * 4. Return txlen
	 */
	mcs = tx_info->mcs;
	bw = tx_info->bw;
	mcs_1ss = mcs;

	if (tx_mode == TEST_MODE_CCK) {
		/* Legacy CCK mode */
		u_int8 cck_map_idx;

		for (cck_map_idx = 0;
			cck_map_idx < SERV_ARRAY_SIZE(cck_mode_map);
			cck_map_idx++) {
			if (mcs_1ss == cck_mode_map[cck_map_idx].mcs)
				break;
		}

		if (cck_map_idx == SERV_ARRAY_SIZE(cck_mode_map)) {
			tx_data_rate = cck_mode_map[0].tx_data_rate;
			mcs = mcs_1ss = cck_mode_map[0].mcs;
			tx_info->mcs = mcs;
		} else
			tx_data_rate = cck_mode_map[cck_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == TEST_MODE_OFDM) {
		/* Legacy OFDM mode */
		u_int8 ofdm_map_idx;

		for (ofdm_map_idx = 0;
			ofdm_map_idx < SERV_ARRAY_SIZE(ofdm_mode_map);
			ofdm_map_idx++) {
			if (mcs_1ss == ofdm_mode_map[ofdm_map_idx].mcs)
				break;
		}

		if (ofdm_map_idx == SERV_ARRAY_SIZE(ofdm_mode_map)) {
			tx_data_rate = ofdm_mode_map[0].tx_data_rate;
			mcs = mcs_1ss = ofdm_mode_map[0].mcs;
			tx_info->mcs = mcs;
		} else
			tx_data_rate = ofdm_mode_map[ofdm_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == TEST_MODE_HTMIX
		   || tx_mode == TEST_MODE_HTGREENFIELD) {
		/* HT mode */
		u_int8 n_map_idx;

		if (mcs != 32) {
			mcs_1ss = mcs % 8;
			nss = (mcs / 8) + 1;
			bw_fact = (bw == TEST_BW_40) ? 2 : 1;
		} else {
			bw_fact = 1;
			nss = 1;
		}

		for (n_map_idx = 0;
			n_map_idx < SERV_ARRAY_SIZE(n_mode_map); n_map_idx++) {
			if (mcs_1ss == n_mode_map[n_map_idx].mcs)
				break;
		}

		if (n_map_idx == SERV_ARRAY_SIZE(n_mode_map)) {
			tx_data_rate = n_mode_map[0].tx_data_rate;
			mcs = mcs_1ss = n_mode_map[0].mcs;
			tx_info->mcs = mcs;
		} else
			tx_data_rate = n_mode_map[n_map_idx].tx_data_rate;

		tx_data_rate = tx_data_rate * nss * bw_fact;

		if (sgi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = tx_time * tx_data_rate / 1000 / 8;
	} else if (tx_mode == TEST_MODE_VHT) {
		/* VHT mode */
		u_int8 ac_map_idx;
		struct test_data_rate_map *vht_mode_map;
		u_int32 array_cnt = 0;

		if (bw == TEST_BW_20) {
			vht_mode_map = ac_mode_map_bw20;
			array_cnt = SERV_ARRAY_SIZE(ac_mode_map_bw20);
		} else if (bw == TEST_BW_40) {
			vht_mode_map = ac_mode_map_bw40;
			array_cnt = SERV_ARRAY_SIZE(ac_mode_map_bw40);
		} else if (bw == TEST_BW_80) {
			vht_mode_map = ac_mode_map_bw80;
			array_cnt = SERV_ARRAY_SIZE(ac_mode_map_bw80);
		} else if (bw == TEST_BW_160C) {
			vht_mode_map = ac_mode_map_bw160;
			array_cnt = SERV_ARRAY_SIZE(ac_mode_map_bw160);
		} else {
			vht_mode_map = ac_mode_map_bw20;
			array_cnt = SERV_ARRAY_SIZE(ac_mode_map_bw20);
		}

		for (ac_map_idx = 0; ac_map_idx < array_cnt; ac_map_idx++) {
			if (mcs == vht_mode_map[ac_map_idx].mcs)
				break;
		}

		if (ac_map_idx == array_cnt) {
			tx_data_rate = vht_mode_map[0].tx_data_rate;
			mcs = mcs_1ss = vht_mode_map[0].mcs;
			tx_info->mcs = mcs;
		} else
			tx_data_rate = vht_mode_map[ac_map_idx].tx_data_rate;

		/* TODO: need to check for sgi equation */
		if (sgi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = tx_time * tx_data_rate / 10 / 8;
	} else if (tx_mode == TEST_MODE_HE_SU) {
		u_int8 map_idx = 0;
		struct test_data_rate_map *rate_map = NULL;
		u_int32 array_cnt = 0;

		switch (sgi) {
		case 0:
		case 1:
		case 4:
			sgi = 0;
			break;
		case 2:
			sgi = 1;
			break;
		case 3:
			sgi = 2;
			break;
		}

		nss = tx_info->nss;

		if (bw == TEST_BW_20) {
			rate_map = he_su_mode_map_bw20;
			array_cnt = SERV_ARRAY_SIZE(he_su_mode_map_bw20);
		} else if (bw == TEST_BW_40) {
			rate_map = he_su_mode_map_bw40;
			array_cnt = SERV_ARRAY_SIZE(he_su_mode_map_bw40);
		} else if (bw == TEST_BW_80) {
			rate_map = he_su_mode_map_bw80;
			array_cnt = SERV_ARRAY_SIZE(he_su_mode_map_bw80);
		} else if (bw == TEST_BW_160NC || bw == TEST_BW_160C) {
			rate_map = he_su_mode_map_bw160;
			array_cnt = SERV_ARRAY_SIZE(he_su_mode_map_bw160);
		} else {
			rate_map = he_su_mode_map_bw20;
			array_cnt = SERV_ARRAY_SIZE(he_su_mode_map_bw20);
		}

		for (map_idx = 0;
		     map_idx < array_cnt;
		     map_idx++) {
			if (mcs == rate_map[map_idx].mcs)
				break;
		}

		if (map_idx == array_cnt) {
			tx_data_rate = rate_map[0].tx_data_rate;
			mcs = mcs_1ss = rate_map[0].mcs;
			tx_info->mcs = mcs;
		} else
			tx_data_rate = rate_map[map_idx].tx_data_rate;

		tx_data_rate *= nss;

		switch (sgi) {
		case 1:
			tx_data_rate = (tx_data_rate * 100) / 90;
			break;

		case 2:
			tx_data_rate = (tx_data_rate * 100) / 85;
			break;

		default:
			if (sgi > 0)
				SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
					("%s: invalid gi=%d, ignored as 0.\n",
					__func__, sgi));
			break;
		}
		/* Transfer from bit to byte with expected tx time */
		txlen = tx_time * tx_data_rate / 10 / 8;
	} else if (tx_mode == TEST_MODE_HE_TB) {
		struct test_ru_info *ru_info = NULL;
		u_int32 ds = 0, dss = 0;

		mcs = (ru_info->rate & 0xf);
		ds = mt_engine_map_subcarriers(ru_info->ru_index >> 1,
						(ru_info->rate & BIT(5)));

		if (ds) {
			dss = mt_engine_map_subcarriers_short(
				ru_info->ru_index >> 1,
				(ru_info->rate & BIT(5)));

			nss = ru_info->nss;
			ru_info->cbps = ds * nss * test_he_bpscs[mcs];

			ru_info->dbps = test_he_rate_density[mcs]-1;
			ru_info->dbps *= ru_info->cbps;
			ru_info->dbps /= test_he_rate_density[mcs];

			txlen = mt_engine_calc_bytes_by_time(tx_mode,
								nss,
								0,
					mt_engine_trans_ltf(tx_mode, sgi),
					mt_engine_trans_gi(tx_mode, sgi),
								ru_info->dbps,
								tx_time);
			/* reserve FCS(4)+Delimiter(4)+
			 * A-Control(4)+H/W revered(1)
			 */
			txlen -= 13;
			ru_info->mpdu_length = txlen;
		} else {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: unknown RU Index:[%d],\n", __func__,
				(ru_info->ru_index >> 1)));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("\t\tforced transmit l024 bytes MPDU!\n"));

			/* reserve FCS(4)+Delimiter(4)+
			 * A-Control(4)+H/W revered(1)
			 */
			txlen = 1024 - 13;
		}
	}

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: phy_mode=%d, mcs/mcs_1ss=%d/%d, nss=%d, ",
		__func__, tx_mode, mcs, mcs_1ss, nss));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("bw/bw_fact=%d/%d, sgi=%d, ", bw, bw_fact, sgi));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("txlen=%d, tx_time=%d, tx_data_rate=%d\n",
		txlen, tx_time, tx_data_rate));

	if (tx_mode > TEST_MODE_VHT) {
		if (txlen >= (TEST_MAX_VHT_MPDU_LEN * 256)) {
			txlen = (TEST_MAX_VHT_MPDU_LEN * 256);
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("%s: expected txlen > HE PPDU max len, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("reduce the txlen=%d\n", txlen));
		}
	} else if (tx_mode == TEST_MODE_VHT) {
		if (txlen >= (TEST_MAX_VHT_MPDU_LEN * 64)) {
			txlen = (TEST_MAX_VHT_MPDU_LEN * 64);
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("%s: expected txlen > VHT PPDU max len, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("reduce the txlen=%d\n", txlen));
		}
	} else if (tx_mode == TEST_MODE_HTMIX
		   || tx_mode == TEST_MODE_HTGREENFIELD) {
		if (txlen >= TEST_MAX_HT_AMPDU_LEN) {
			txlen = TEST_MAX_HT_AMPDU_LEN;
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("%s: expected txlen > HT PPDU max len, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("reduce the txlen=%d\n", txlen));
		}
	} else if (tx_mode == TEST_MODE_OFDM) {
		if (txlen >= TEST_MAX_MSDU_LEN) {
			txlen = TEST_MAX_MSDU_LEN;
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("%s: expected txlen > OFDM PPDU max len, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("reduce the txlen=%d\n", txlen));
		}
	} else if (tx_mode == TEST_MODE_CCK) {
		if (txlen >= TEST_MAX_MSDU_LEN) {
			txlen = TEST_MAX_MSDU_LEN;
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("%s: expected txlen > CCK PPDU max len, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("reduce the txlen=%d\n", txlen));
		}
	}

	tx_info->mpdu_info.tx_len = txlen;

	return ret;
}

static s_int32 mt_engine_calc_hlen(
	u_char tx_mode, struct tx_mpdu_info *mpdu_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 hlen = TEST_DEFAULT_MAC_HDR_LEN;

	/*
	 * 1. Get the tx data rate
	 * 2. Check if need to send packet with AMPDU format
	 * 3. Check if need to send packet with AMSDU-in-AMPDU format
	 * 4. Return the expected packet header length by tx packet type
	 *  if need to has QoS field and HTC field.
	 */

	if (mpdu_info->tx_len > TEST_MAX_MSDU_LEN) {
		if (tx_mode > TEST_MODE_OFDM) {
			mpdu_info->need_qos = TRUE;
			/* mark need_amsdu to notify A-MSDU like packet */
			mpdu_info->need_amsdu = TRUE;
		}

		if (mpdu_info->tx_len > datalen_limit[tx_mode].amsdu_limit)
			mpdu_info->need_ampdu = TRUE;
	}

	/* Force aggregation due to TXCMD required */
	if (tx_mode == TEST_MODE_VHT_MIMO || tx_mode == TEST_MODE_HE_MU) {
		mpdu_info->need_qos = TRUE;
		mpdu_info->need_ampdu = TRUE;
	}

	if (mpdu_info->need_qos)
		hlen = TEST_QOS_MAC_HDR_LEN;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: header len=%d -> %d\n", __func__,
		mpdu_info->hdr_len, hlen));
	mpdu_info->hdr_len = hlen;
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: packet length=%d,\n", __func__, mpdu_info->tx_len));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t\t\tqos/amsdu/ampdu=%d/%d/%d\n",
		mpdu_info->need_qos, mpdu_info->need_amsdu,
		mpdu_info->need_ampdu));

	return ret;
}

static s_int32 mt_engine_calc_ampdu(
	struct test_wlan_info *winfos,
	u_char tx_mode,
	struct tx_mpdu_info *mpdu_info,
	struct test_operation *ops,
	u_char *buf)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 per_mpdu_len;
	u_int8 tx_hw_hdr_len;
	u_int8 agg_cnt = 0;

	tx_hw_hdr_len = winfos->chip_cap.tx_wi_size;

	per_mpdu_len = datalen_limit[tx_mode].amsdu_limit;
	agg_cnt = mpdu_info->tx_len / per_mpdu_len;

	if (mpdu_info->tx_len % per_mpdu_len)
		agg_cnt++;

	mpdu_info->msdu_len = engine_ceil(mpdu_info->tx_len, agg_cnt);
	mpdu_info->mpdu_cnt = agg_cnt;

	return ret;
}

static s_int32 mt_engine_prepare_burst(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct tx_mpdu_info *mpdu_info,
	struct test_operation *ops,
	u_char *buf)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int8 need_ampdu;

	need_ampdu = mpdu_info->need_ampdu;

	/* Update WTBL if necessary */
	/* Generate tx packet */
	/*
	 * Note: the difference between handle_ampdu/handle_non_ampdu
	 * is txlen and hlen, maybe reduce API here and call fill_pkt directly
	 */
	if (need_ampdu) {
		ret = mt_engine_calc_ampdu(winfos, configs->tx_mode,
					   mpdu_info, ops, buf);

		if (ret != SERV_STATUS_SUCCESS)
			goto err_out;
	} else {
		/*
		 * No aggregation, directly go with specific length
		 * and through ALTX queue
		 */
		mpdu_info->msdu_len = mpdu_info->tx_len;
	}

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		 ("txlen/msdu len/agg cnt=%d/%d/%d\n",
		  mpdu_info->tx_len, mpdu_info->msdu_len,
		  mpdu_info->mpdu_cnt));

err_out:
	return ret;
}

static s_int32 mt_engine_prepare_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	struct test_tx_info *tx_info,
	u_char *buf)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct tx_time_param *tx_time_param = NULL;
	u_int32 pkt_tx_time;

	if (!buf) {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("%s: null buf\n", __func__));

		return SERV_STATUS_ENGINE_INVALID_NULL_POINTER;
	}

	tx_time_param = &configs->tx_time_param;

	/* reset first */
	tx_info->mpdu_info.need_qos = 0;
	tx_info->mpdu_info.need_amsdu = 0;
	tx_info->mpdu_info.need_ampdu = 0;
	tx_info->mpdu_info.mpdu_cnt = 1;
	pkt_tx_time = tx_time_param->pkt_tx_time;

	if (pkt_tx_time > 0) {
		ret = mt_engine_calc_txlen(configs->tx_mode,
						configs->sgi,
						tx_info,
						pkt_tx_time,
						&configs->ru_info_list[0]);

		if (tx_info->mpdu_info.tx_len == 0) {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: can't get txlen by pkt tx time\n",
				__func__));
			goto err_out;
		}

		if (ret != SERV_STATUS_SUCCESS)
			goto err_out;
	}

	ret = mt_engine_calc_hlen(configs->tx_mode, &tx_info->mpdu_info);

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	ret = mt_engine_prepare_burst(winfos, configs, &tx_info->mpdu_info,
					ops, buf);
	if (ret)
		goto err_out;

	if (tx_info->mpdu_info.need_ampdu) {
		/* Update WTBL first */
		ret = net_ad_cfg_wtbl(winfos, configs, tx_info);
		if (ret)
			goto err_out;
	}

err_out:
	return ret;
}

static s_int32 mt_engine_apply_ipg_param(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct ipg_param *ipg_param;
	u_int32 ipg;
	u_int16 slot_time, sifs_time, cw;
	u_int8 aifsn, wmm_idx = 0;

	ipg_param = &configs->ipg_param;
	ipg = ipg_param->ipg;

	if (ipg > 0) {
		/* Get packet q_idx and decide which CR need to be changed */
		slot_time = ipg_param->slot_time;
		sifs_time = ipg_param->sifs_time;
		aifsn = ipg_param->aifsn;
		cw = ipg_param->cw;
	} else {
		/* Write default value back to HW */
		slot_time = TEST_DEFAULT_SLOT_TIME;
		sifs_time = TEST_DEFAULT_SIFS_TIME;
		aifsn = TEST_MIN_AIFSN;
		cw = 0;
	}

	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;

	ret = ops->op_set_ifs_cr(winfos, configs, band_idx);
	if (ret)
		goto err_out;

	ret = ops->op_set_slot_time(winfos,
					(u_int8) slot_time,
					(u_int8) sifs_time,
					TEST_RIFS_TIME,
					TEST_EIFS_TIME,
					(u_int8) band_idx);
	if (ret)
		goto err_out;

	ret = net_ad_get_wmm_idx(configs->stack.virtual_device[0], &wmm_idx);
	if (ret)
		goto err_out;

	ret = net_ad_set_wmm_param_by_qid(wmm_idx, configs->ac_idx,
					  winfos, configs);
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: ipg=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
		__func__, ipg, slot_time, sifs_time, aifsn, cw));

err_out:
	return ret;
}

static boolean is_mt_engine_stack_full(struct test_configuration *configs)
{
	boolean ret = FALSE;
	struct test_tx_stack *stack = &configs->stack;

	if (stack->index == stack->entry_limit)
		ret = TRUE;

	return ret;
}

static boolean is_mt_engine_stack_empty(struct test_configuration *configs)
{
	boolean ret = FALSE;
	struct test_tx_stack *stack = &configs->stack;

	if (stack->index == 0)
		ret = TRUE;

	return ret;
}

static s_int32 mt_engine_stack_lookup(
	struct test_configuration *configs,
	void *virtual_wtbl)
{
	u_int8 stack_seq = 0;
	s_int32 ret = -1;

	for (stack_seq = 0;
		stack_seq < configs->stack.index;
		stack_seq++) {
		if (configs->stack.virtual_wtbl[stack_seq] &&
			(virtual_wtbl ==
			 configs->stack.virtual_wtbl[stack_seq])) {
			ret = stack_seq;
			break;
		}
	}

	return ret;
}

static s_int32 mt_engine_stack_push(
	struct test_configuration *configs,
	void *virtual_device,
	u_int8 *da,
	void *virtual_wtbl,
	struct test_tx_info *tx_info)
{
	s_int32 ret = -1;
	struct test_tx_stack *stack = &configs->stack;

	ret = mt_engine_stack_lookup(configs, virtual_wtbl);

	if (ret > -1) {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("(%s)Stack[%d] reused!\n", __func__, ret));
	} else {
		if (stack->index < stack->entry_limit) {
			sys_ad_move_mem(stack->da[stack->index], da,
					SERV_MAC_ADDR_LEN);
			stack->virtual_device[stack->index] = virtual_device;
			stack->virtual_wtbl[stack->index] = virtual_wtbl;
			sys_ad_move_mem(&stack->tx_info[stack->index],
					tx_info, sizeof(*tx_info));
			ret = stack->index;

			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
				("(%s)Store %p to stack[%d]!\n", __func__,
				 virtual_wtbl, ret));

			stack->index++;
		} else {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("(%s)Stack for TX is full!\n", __func__));
		}
	}

	return ret;
}

static u_int32 mt_engine_stack_pop(
	struct test_wlan_info *winfos,
	struct test_tx_stack *stack)
{
	u_int32 ret = SERV_STATUS_SUCCESS;

	if (stack->index > 0) {
		stack->virtual_device[stack->index-1] = NULL;
		ret = net_ad_free_pkt(winfos,
				      stack->pkt_skb[stack->index-1]);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_ENGN,
				 SERV_DBG_LVL_ERROR,
			 ("%s: Free Packet fail(0x%04x)\n",
				 __func__, ret));
		} else
			stack->pkt_skb[stack->index-1] = NULL;

		net_ad_free_wtbl(winfos, stack->da[stack->index-1],
				 stack->virtual_wtbl[stack->index-1],
				 &stack->tx_info[stack->index-1]);
		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				 ("%s: Free wtbl fail(0x%04x)\n",
				  __func__, ret));
		}
		sys_ad_zero_mem(&stack->tx_info[stack->index-1],
				sizeof(struct test_tx_info));
		sys_ad_zero_mem(stack->da[stack->index-1],
				SERV_MAC_ADDR_LEN);
		stack->virtual_wtbl[stack->index-1] = NULL;

		stack->index--;
	} else {
		ret = SERV_STATUS_ENGINE_FAIL;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("(%s)Stack for Tx is empty, dismissed\n", __func__));
	}

	return ret;
}

static s_int32 mt_engine_store_tx_info(
	struct test_operation *ops,
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	void *virtual_device,
	u_char *da,
	void *virtual_wtbl,
	struct test_tx_info *tx_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS, sta_idx = -1;
	u_char *pate_pkt = (u_char *)configs->test_pkt;
	struct test_tx_stack *stack = &configs->stack;

	if (pate_pkt == NULL) {
		ret = -1;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("%s: Invalid pre-allocated buffer for MPDU\n",
			__func__));

		goto err_out;
	}

	if (is_mt_engine_stack_full(configs) == FALSE) {
		if (tx_info) {
			net_ad_fill_phy_info(virtual_wtbl, tx_info);

			if ((configs->tx_mode < TEST_MODE_HE_SU) &&
				(tx_info->mcs & 0x7f) == 32)
				net_ad_handle_mcs32(winfos,
						    virtual_wtbl,
						    tx_info->bw);
		}

		net_ad_apply_wtbl(winfos, virtual_device, virtual_wtbl);

		sta_idx = mt_engine_stack_push(configs,
					       virtual_device,
					       da,
					       virtual_wtbl,
					       tx_info);

		if (sta_idx > -1 && sta_idx < MAX_MULTI_TX_STA) {
			u_int8 *pate_pkt = configs->test_pkt;
			struct test_tx_info *tx_info = NULL;

			tx_info = &stack->tx_info[sta_idx];
			/* fill data to pre-allocated common buffer */
			ret = mt_engine_prepare_tx(winfos,
							configs,
							ops,
							tx_info,
							pate_pkt);

			if (ret != SERV_STATUS_SUCCESS) {
				SERV_LOG(SERV_DBG_CAT_ENGN,
					 SERV_DBG_LVL_ERROR,
				("%s: Generate packets failed(0x%04x)\n",
					__func__, ret));
				goto err_out;
			}

			ret = net_ad_compose_pkt(winfos, configs, sta_idx,
						pate_pkt,
						tx_info->mpdu_info.msdu_len,
						tx_info->mpdu_info.hdr_len);

			/* allocate per sta packet */
			/* release packet first in case
			 * user execute txcommit without revert priorly
			 */
			if (stack->pkt_skb[sta_idx]) {
				ret = net_ad_free_pkt(winfos,
						      stack->pkt_skb[sta_idx]);

				if (ret != SERV_STATUS_SUCCESS) {
					SERV_LOG(SERV_DBG_CAT_ENGN,
						 SERV_DBG_LVL_ERROR,
					 ("%s: Free Packet fail(0x%04x)\n",
						 __func__, ret));
					goto err_out;
				}
			} else
				stack->pkt_skb[sta_idx] = NULL;
			ret = net_ad_alloc_pkt(winfos,
					       configs,
					       tx_info->mpdu_info.msdu_len,
					       &stack->pkt_skb[sta_idx]);

			if (ret != SERV_STATUS_SUCCESS) {
				SERV_LOG(SERV_DBG_CAT_ENGN,
					 SERV_DBG_LVL_ERROR,
				 ("%s: Allocate Packet fail(0x%04x)\n",
					 __func__, ret));
				goto err_out;
			} else {
				configs->is_alloc_skb |= BIT(sta_idx);
			}
		} else {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
					("%s: TX stack entry stored failed\n",
					__func__));
		}
	} else {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: Tx stack is full!\n", __func__));
	}

err_out:
	return ret;
}

static u_int8 mt_engine_decode_gi(
	u_int8 tx_mode,
	u_int32 ltf_gi)
{
	u_int8 gi_type = 0;

	if (tx_mode == TEST_MODE_HE_SU ||
		tx_mode == TEST_MODE_HE_ER ||
		tx_mode == TEST_MODE_HE_MU) {
		switch (ltf_gi) {
		case 1:
		case 2:
			gi_type = TEST_GI_8+(ltf_gi-1);
			break;
		case 3:
		case 4:
			gi_type = TEST_GI_8+((ltf_gi-3) ? 0:2);
			break;
		default:
			gi_type = TEST_GI_8;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_TB) {
		switch (ltf_gi) {
		case 2:
			gi_type = TEST_GI_32;
			break;
		default:
			gi_type = TEST_GI_16;
			break;
		}
	} else	/* for non-HE PPDU types, gi equavalent to Sgi. */
		gi_type = ltf_gi;

	return gi_type;
}

static u_int8 mt_engine_decode_ltf(
	u_int8 tx_mode,
	u_int32 ltf_gi)
{
	u_int8 ltf_type = 0;

	if (tx_mode == TEST_MODE_HE_SU || tx_mode == TEST_MODE_HE_ER) {
		switch (ltf_gi) {
		case 1:
		case 2:
			ltf_type = TEST_HE_LTF_X2;
			break;
		case 3:
		case 4:
			ltf_type = TEST_HE_LTF_X4;
			break;
		default:
			ltf_type = TEST_HE_LTF_X1;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_MU) {
		switch (ltf_gi) {
		case 0:
		case 3:
			ltf_type = TEST_HE_LTF_X4;
			break;
		default:
			ltf_type = TEST_HE_LTF_X2;
			break;
		}
	} else if (tx_mode == TEST_MODE_HE_TB) {
		ltf_type = ltf_gi;
	} else	/* for non-HE PPDU types, ltf is not required. */
		ltf_type = TEST_HE_LTF_X1;

	return ltf_type;
}

static u_int8 mt_engine_get_sub_band(u_int32 ru_index)
{
	u_int8 sub_band_idx = 0;

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
	else if (ru_index == 36)
		sub_band_idx = 3;
	else
		sub_band_idx = (ru_index / 9);

	return sub_band_idx;
}

static u_int32 mt_engine_add_allocation(
	struct test_ru_allocatoin *alloc_info,
	u_int8 allocation,
	u_int8 seg,
	u_int32 ru_index)
{
	u_int8 sub_band_idx = 0;

	sub_band_idx = mt_engine_get_sub_band(ru_index) + seg*4;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: ru index:%d (%d), ",
		__func__, ru_index, sub_band_idx));
	/* 0x7f is center-26 tone, should be ignored */
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%2x -> %2x\n",
		alloc_info->sub20[sub_band_idx], allocation));
	if ((alloc_info->sub20[sub_band_idx] != allocation)
		&& (allocation != 0x7f)) {
		if (alloc_info->sub20[sub_band_idx] == 0xff) {
			alloc_info->sub20[sub_band_idx] = allocation;

			if (allocation == 0xc8) {
				/* D3.1, Table 28-24, 0xc8 is 484-tone
				 * D3.1, Table 28-24, 0x72 is 484-empty-tone
				 */
				alloc_info->sub20[sub_band_idx+1] = 0x72;
			} else if (allocation == 0xd0) {
				/* D3.1, Table 28-24, 0xd0 is 996-tone
				 * D3.1, Table 28-24, 0x73 is 484-empty-tone
				 */
				alloc_info->sub20[sub_band_idx+1] = 0x73;
				alloc_info->sub20[sub_band_idx+2] = 0x73;
				alloc_info->sub20[sub_band_idx+3] = 0x73;
			}
		}
	}

	return SERV_STATUS_SUCCESS;
}

static s_int32 mt_engine_fill_empty_allocation(
	struct test_ru_allocatoin *alloc_info)
{
	u_int8 alloc_idx = 0;

	for (alloc_idx = 0 ; alloc_idx < sizeof(*alloc_info) ; alloc_idx++) {
		/* D3.1, Table 28-24, 0x71 is 242-empty */
		if (alloc_info->sub20[alloc_idx] == 0xff)
			alloc_info->sub20[alloc_idx] = 0x71;
	}

	return SERV_STATUS_SUCCESS;
}

static s_int32 mt_engine_calc_symbol_by_bytes(
	struct test_ru_info *ru_info, boolean stbc,
	u_char rate_den, u_int32 apep_length)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32 m_stbc = 1, tail = 6, rate = 0;
	s_int32 ds = 0, dss = 0;

	ds = mt_engine_map_subcarriers(ru_info->ru_index >> 1,
					(ru_info->rate & BIT(5)));

	if (ds)
		dss = mt_engine_map_subcarriers_short(ru_info->ru_index >> 1,
						   (ru_info->rate & BIT(5)));
	else {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("%s: unknown RU Index:[%d]!\n",
			__func__, (ru_info->ru_index >> 1)));

		ret = SERV_STATUS_ENGINE_INVALID_PARAM;
		goto err_out;
	}

	rate = ru_info->rate & (~BIT(5));

	if (stbc)
		m_stbc++;

	if (ru_info->ldpc)
		tail = 0;

	ru_info->cbps = ds * ru_info->nss * test_he_bpscs[rate];
	ru_info->dbps = ru_info->cbps * (rate_den-1) / rate_den;
	ru_info->cbps_s = dss * ru_info->nss * test_he_bpscs[rate];
	ru_info->dbps_s = ru_info->cbps_s * (rate_den-1) / rate_den;

	apep_length *= 8;
	apep_length += (16 + tail);
	ru_info->symbol_init = engine_ceil(apep_length,
					(m_stbc * ru_info->dbps));
	ru_info->symbol_init *= m_stbc;
	ru_info->excess = (apep_length % (m_stbc * ru_info->dbps));

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("%s: RU index[%d], apep length:%d symbol_init:%d,\n",
		__func__, ru_info->ru_index >> 1,
		apep_length, ru_info->symbol_init));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("\t%s: R[%d/%d], cbps:%d, dbps:%d,\n",
		__func__, rate_den-1, rate_den, ru_info->cbps, ru_info->dbps));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("\t%s: cbps_s:%d, dbps_s:%d excess:%d\n",
		__func__, ru_info->cbps_s,
		ru_info->dbps_s, ru_info->excess));

err_out:
	return ret;
}

static s_int32 mt_engine_calc_afactor(struct test_ru_info *ru_info)
{
	s_int32 ret = 0, m_stbc = 1;

	if (ru_info->excess == 0) {
		ru_info->excess = m_stbc * ru_info->dbps;
		ru_info->afactor_init = 4;
	} else {
		u_int32 sym_short = (m_stbc * ru_info->dbps_s);
		u_int32 symbol = engine_ceil(ru_info->excess, sym_short);

		ru_info->afactor_init = engine_min(4, symbol);
	}

	/* prepare for caculate ldpc extra symbol */
	if (ru_info->afactor_init == 4) {
		ru_info->dbps_last = ru_info->dbps;
		ru_info->cbps_last = ru_info->cbps;
	} else {
		ru_info->dbps_last = ru_info->afactor_init * ru_info->dbps_s;
		ru_info->cbps_last = ru_info->afactor_init * ru_info->cbps_s;
	}

	ru_info->pld = (ru_info->symbol_init - m_stbc) * ru_info->dbps;
	ru_info->pld += m_stbc * ru_info->dbps_last;
	ru_info->avbits = (ru_info->symbol_init - m_stbc) * ru_info->cbps;
	ru_info->avbits += m_stbc * ru_info->cbps_last;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("\t%s: \tafactor=%d, symbol cnt=%d\n",
		__func__, ru_info->afactor_init, ru_info->symbol_init));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("\t%s: cbps_l:%d, dbps_l:%d, pld:%d, avbits:%d\n",
		__func__, ru_info->cbps_last, ru_info->dbps_last,
		ru_info->pld, ru_info->avbits));

	return ret;
}

static s_int32 mt_engine_calc_l_ldpc(
	s_int32 avbits, s_int32 pld, u_char rate_den,
	s_int32 *cw, s_int32 *l_ldpc)
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
		*cw = engine_ceil((pld * rate_den), ((648 * 3) * (rate_den-1)));
		*l_ldpc = (648 * 3);
	}

	return 0;
}

static boolean mt_engine_calc_extr_sym(
	struct test_ru_info *ru_info, boolean stbc, u_char rd)
{
	boolean ret = FALSE;
	s_int32 cw = 0, l_ldpc = 0, shrt = 0;

	mt_engine_calc_l_ldpc(ru_info->avbits, ru_info->pld, rd, &cw, &l_ldpc);

	shrt = (s_int32)((cw * l_ldpc * (rd-1) / rd) - ru_info->pld);
	if (shrt < 0)
		shrt = 0;
	ru_info->punc = (s_int16)(cw * l_ldpc - ru_info->avbits - shrt);
	if (ru_info->punc < 0)
		ru_info->punc = 0;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t%s: cw:%d, avbits:%d, punc:%d, l_ldpc:%d, shrt:%d\n",
		__func__, cw, ru_info->avbits, ru_info->punc, l_ldpc, shrt));

	if (((10 * ru_info->punc > cw * l_ldpc / rd) &&
		(5 * shrt < 6 * ru_info->punc * (rd-1))) ||
		(10 * ru_info->punc > 3 * cw * l_ldpc / rd))
		ret = TRUE;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t%s: LDPC extra symbol:%d\n", __func__, ret));

	return ret;
}

static s_int32 mt_engine_calc_pe_disamb(
	struct test_ru_info *ru_info, u_char ltf_gi, u_char max_pe)
{
	u_int8 pe_symbol_x5 = 0;
	s_int32 ret = 0, gi = 0, t_pe = ru_info->afactor_init;
	s_int32 ltf_time = 0;
	u_int32 nss = engine_max(ru_info->ru_mu_nss, ru_info->nss);

	ltf_time = test_ltf_sym[nss];
	ltf_time *= test_he_t_ltf_sym_x5[ltf_gi];

	if (ltf_gi == 2)
		gi = TEST_GI_32;
	else
		gi = TEST_GI_16;

	/* txtime = 20 + T_HE-PREAMBLE + N_SYM*T_SYM +
	 *          N_MA*N_HE-LTF*T_HE-LTF-SYM + T_PE +
	 *          SignalExtension (28-135)
	 * T_HE-PREAMBLE = T_RL-SIG + T_HE-SIG-A +
	 *                 T_HE-STF-T + N_HE-LTF*T_HE-LTF-SYM,
	 *                 for an HE TB PPDU
	 * According to Table 28-12 of 802.11ax D3.0, T_RL-SIG = 4,
	 *                 T_HE-SIG-A = 8, T_HE-STF-T = 8,
	 *                 N_HE-LTF*T_HE-LTF-SYM (N_HE-LTF = {1,2,4,6}))
	 * N_MA = 0 due to doppler is not support, and SignalExtension = 0
	 *    due to not supported
	 */
	ru_info->tx_time_x5 = 5 * 20 + (20+40+40+ltf_time) +
			     ru_info->symbol_init * test_he_t_sym_x5[gi] + 0
			     + test_he_t_pe_x5[t_pe] + 0;
	ru_info->l_len = engine_ceil((ru_info->tx_time_x5-20*5), (4*5))*3-3-2;

	pe_symbol_x5 = test_he_t_pe_x5[t_pe];
	pe_symbol_x5 += (4 * (((ru_info->tx_time_x5 - 20 * 5)%20) ? 1 : 0));
	if (pe_symbol_x5 >= test_he_t_sym_x5[gi])
		ru_info->pe_disamb = 1;
	else
		ru_info->pe_disamb = 0;

	ru_info->t_pe = t_pe;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t%s: L-Len=%d, PE Disambiguilty=%d\n",
		__func__, ru_info->l_len, ru_info->pe_disamb));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t%s: tx_time(x5)=%d, tx_ltf_sym(x5):%d,\n",
		__func__, ru_info->tx_time_x5, ltf_time));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t%s: tx_sym(x5):%d, tx_pe(x5):%d\n",
		__func__, test_he_t_sym_x5[gi], test_he_t_pe_x5[t_pe]));

	return ret;
}

static s_int32 mt_engine_recalc_phy_info(
	struct test_ru_info *ru_info, u_int8 stbc, u_int8 ltf_gi, u_int8 max_pe)
{
	u_char rd = 0;
	u_char m_stbc = (stbc) ? 2 : 1;
	s_int32 shrt = 0;
	u_int32 cw = 0, l_ldpc = 0;

	rd = test_he_rate_density[ru_info->rate & ~BIT(5)];

	if (ru_info->afactor_init == 3) {
		u_int32 short_sym = ru_info->afactor_init * ru_info->cbps_s;

		ru_info->avbits += m_stbc * (ru_info->cbps - short_sym);
	} else
		ru_info->avbits += m_stbc * ru_info->cbps_s;

	mt_engine_calc_l_ldpc(ru_info->avbits, ru_info->pld,
			      rd, &cw, &l_ldpc);

	shrt = (s_int32)(cw * l_ldpc * (rd-1) / rd - ru_info->pld);
	if (shrt < 0)
		shrt = 0;
	ru_info->punc = (s_int32)(cw * l_ldpc - ru_info->avbits - shrt);
	if (ru_info->punc < 0)
		ru_info->punc = 0;

	if (ru_info->afactor_init == 4) {
		ru_info->symbol_init += m_stbc;
		ru_info->afactor_init = 1;
	} else
		ru_info->afactor_init++;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t%s: (re)afactor:%d\n",
		__func__, ru_info->afactor_init));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t%s: (re)cw:%d, (re)avbits:%d,\n",
		__func__, cw, ru_info->avbits));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("\t\t%s: (re)punc:%d, (re)l_ldpc:%d, (re)shrt:%d\n",
		__func__, ru_info->punc, l_ldpc, shrt));

	mt_engine_calc_pe_disamb(ru_info, ltf_gi, max_pe);

	return SERV_STATUS_SUCCESS;
}


s_int32 mt_engine_calc_phy(
	struct test_ru_info *ru_info,
	u_int32 apep_length,
	u_int8 stbc,
	u_int8 ltf_gi,
	u_int8 max_tpe)
{
	u_char rate_den = 0;

	rate_den = test_he_rate_density[ru_info->rate & ~BIT(5)];
	mt_engine_calc_symbol_by_bytes(ru_info, stbc, rate_den, apep_length);
	mt_engine_calc_afactor(ru_info);
	mt_engine_calc_pe_disamb(ru_info, ltf_gi, max_tpe);

	if (ru_info->ldpc &&
		mt_engine_calc_extr_sym(ru_info, stbc, rate_den)) {
		ru_info->ldpc_extr_sym = 1;

		mt_engine_recalc_phy_info(ru_info, stbc, ltf_gi, max_tpe);
	}

	return SERV_STATUS_SUCCESS;
}

static struct test_ru_info *mt_engine_search_dom_ru(
	struct test_configuration *configs)
{
	u_int8 sta_idx = 0;
	u_int8 dominate_user_idx = 0;
	u_int32 max_tx_time = 0;
	struct test_ru_info *ru_info = &configs->ru_info_list[0];

	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		if (ru_info[sta_idx].valid) {
			if (ru_info[sta_idx].tx_time_x5 > max_tx_time) {
				max_tx_time = ru_info[sta_idx].tx_time_x5;
				dominate_user_idx = sta_idx;
			}
		}
	}

	if (dominate_user_idx < MAX_MULTI_TX_STA) {
		configs->dmnt_ru_idx = dominate_user_idx;
		ru_info = &configs->ru_info_list[dominate_user_idx];
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
			("%s: dominated by user[%d], RU index:%d\n",
			__func__, dominate_user_idx, ru_info->ru_index >> 1));
	} else {
		ru_info = NULL;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("%s: invalid user index:%d\n",
			__func__, dominate_user_idx));
	}

	return ru_info;
}

static s_int32 mt_engine_apply_spe_antid(
	struct test_operation *ops,
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	u_int8 band_idx = 0;
	u_int8 ant_pri = 0, spe_idx = 0, stack_idx = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 ant_sel = configs->tx_ant;
	struct test_tx_stack *stack = &configs->stack;

	net_ad_get_band_idx(stack->virtual_device[0], &band_idx);

	if (ant_sel & TEST_ANT_USER_DEF) {
		ant_sel &= ~TEST_ANT_USER_DEF;
		spe_idx = ant_sel;
	} else {
		net_ad_get_speidx(winfos, ant_sel, &spe_idx);
	}


	/* store SPE index to TXD and WTBL */
	while (stack->virtual_wtbl[stack_idx] && (stack_idx < (stack->index))) {
		net_ad_fill_spe_antid(winfos, stack->virtual_wtbl[stack_idx],
					spe_idx, ant_pri);
		stack_idx++;
	}
	ops->op_set_mutb_spe(winfos, band_idx, configs->tx_mode, spe_idx);

	return ret;
}

/*****************************************************************************
 *	Extern functions
 *****************************************************************************/
s_int32 mt_engine_search_stack(
	struct test_configuration *configs,
	u_int8 wcid,
	u_int8 *stack_idx,
	void **virtual_wtbl)
{
	s_int32 ret = SERV_STATUS_ENGINE_FAIL;
	struct test_tx_stack *stack = &configs->stack;

	if (stack->index == 0) {
		SERV_LOG(SERV_DBG_CAT_ENGN,
			 SERV_DBG_LVL_ERROR,
			 ("(%s)Stack for Tx is empty, dismissed!\n", __func__));

		goto end;
	} else {
		u_int8 sta_seq = 0;

		for (sta_seq = 0 ; sta_seq < stack->index ; sta_seq++) {
			if (stack->virtual_wtbl[sta_seq] &&
			    (net_ad_match_wtbl(stack->virtual_wtbl[sta_seq],
					  wcid) == SERV_STATUS_SUCCESS)) {
				ret = SERV_STATUS_SUCCESS;
				*stack_idx = sta_seq;
				*virtual_wtbl = stack->virtual_wtbl[sta_seq];
				break;
			}
		}
	}

end:
	return ret;
}

s_int32 mt_engine_unsubscribe_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
	u_int32 ret = SERV_STATUS_SUCCESS;

	while (is_mt_engine_stack_empty(configs) == FALSE) {
		mt_engine_stack_pop(winfos, &configs->stack);
	}

	return ret;
}

s_int32 mt_engine_subscribe_tx(
	struct test_operation *ops,
	struct test_wlan_info *winfos,
	void *virtual_device,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_tx_info tx_info;
	u_int8 tone_idx = 0;
	void *virtual_wtbl = NULL;

	sys_ad_zero_mem(&tx_info, sizeof(tx_info));

	/* Calculate duty_cycle related parameter first */
	if (configs->duty_cycle > 0)
		ret = mt_engine_calc_duty_cycle(configs);

	tx_info.tx_mode = configs->tx_mode;
	if (configs->per_pkt_bw >= TEST_BW_160C)
		tx_info.bw = 0x3;
	else
		tx_info.bw = configs->per_pkt_bw;
	tx_info.stbc = configs->stbc;
	tx_info.ldpc = configs->ldpc;
	tx_info.mpdu_info.tx_len = configs->tx_len;
	tx_info.mpdu_info.hdr_len = configs->hdr_len;
	tx_info.gi = mt_engine_decode_gi(configs->tx_mode, configs->sgi);
	tx_info.ltf = mt_engine_decode_ltf(configs->tx_mode, configs->sgi);
	tx_info.ibf = configs->ibf;
	tx_info.ebf = configs->ebf;

	if (tx_info.tx_mode < TEST_MODE_HE_TB) {
		if (ops->op_set_tam_arb)
			ops->op_set_tam_arb(winfos, 0x5);

		if (tx_info.tx_mode == TEST_MODE_CCK) {
			if (configs->mcs & TEST_CCK_SHORT_PREAMBLE)
				net_ad_set_preamble(winfos, TRUE);
			else
				net_ad_set_preamble(winfos, FALSE);

			SERV_LOG(SERV_DBG_CAT_ENGN,
				 SERV_DBG_LVL_TRACE,
				 ("%s: CCK %s\n",
				  __func__,
				  ((configs->mcs & TEST_CCK_SHORT_PREAMBLE) ?
					"short" : "long")));
		}
		tx_info.mcs = configs->mcs;
		tx_info.nss = configs->nss;

		if (net_ad_alloc_wtbl(winfos,
				      configs->addr1[0],
				      virtual_device,
				      &virtual_wtbl,
				      &tx_info) == SERV_STATUS_SUCCESS) {
			mt_engine_store_tx_info(ops,
						winfos,
						configs,
						virtual_device,
						configs->addr1[0],
						virtual_wtbl,
						&tx_info);
		} else {
			ret = SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER;
			goto err_out;
		}
	} else {
		u_int8 idx = 0, *da = NULL;
		struct test_ru_info *ru_info = &configs->ru_info_list[0];

		if (tx_info.tx_mode == TEST_MODE_VHT_MIMO)
			tx_info.tx_mode = TEST_MODE_VHT;

		for (idx = 0 ; idx < MAX_MULTI_TX_STA ; idx++) {
			virtual_wtbl = NULL;
			da = configs->addr1[idx];
			if (ru_info[idx].valid) {
				tx_info.mcs = ru_info[idx].rate;
				tx_info.nss = ru_info[idx].nss;
				tx_info.ldpc = ru_info[idx].ldpc;
				if (ru_info[idx].mpdu_length >
					TEST_QOS_MAC_HDR_LEN)
					tx_info.mpdu_info.tx_len =
						ru_info[idx].mpdu_length;

				/* ru_info.aid imply user specify AID,
				 * otherwise use auto-generated
				 */
				if (ru_info[idx].aid)
					tx_info.aid = ru_info[idx].aid;
				else
					tx_info.aid = 0;

				ret = net_ad_alloc_wtbl(winfos,
							configs->addr1[idx],
							virtual_device,
							&virtual_wtbl,
							&tx_info);

				if (ret == SERV_STATUS_SUCCESS) {
					mt_engine_store_tx_info(ops,
								winfos,
								configs,
								virtual_device,
							configs->addr1[idx],
								virtual_wtbl,
								&tx_info);
				} else
					goto err_out;


				if (tx_info.tx_mode == TEST_MODE_HE_MU) {
					mt_engine_add_allocation(
						&configs->ru_alloc,
						ru_info[idx].allocation,
						(ru_info[idx].ru_index & 0x1),
						(ru_info[idx].ru_index >> 1));
				} else {
					/* TEST_MODE_HE_TB */
					mt_engine_calc_phy(&ru_info[idx],
						  tx_info.mpdu_info.tx_len+13,
							   tx_info.stbc,
							   configs->sgi,
							configs->max_pkt_ext);
				}
			}
		}
		if (configs->tx_mode == TEST_MODE_HE_MU ||
			configs->tx_mode == TEST_MODE_VHT_MIMO) {
			mt_engine_fill_empty_allocation(&configs->ru_alloc);

			for (tone_idx = 0;
			     tone_idx < sizeof(configs->ru_alloc);
			     tone_idx++) {
				SERV_LOG(SERV_DBG_CAT_ENGN,
					 SERV_DBG_LVL_ERROR,
					 ("%s: allocation[%d] = 0x%x\n",
					 __func__,
					 tone_idx,
					 configs->ru_alloc.sub20[tone_idx]));
			}

			if (ops->op_set_tam_arb) {
				ret = ops->op_set_tam_arb(winfos, 0x2);
				if (ret != SERV_STATUS_SUCCESS)
					goto err_out;
			}

			if (ops->op_set_muru_manual) {
				ret = ops->op_set_muru_manual(virtual_device,
								winfos,
								configs);
				if (ret != SERV_STATUS_SUCCESS)
					goto err_out;
			}
		}
	}

	if (ops->op_set_ampdu_ba_limit) {
		u_int8 wmm_idx = 0;
		u_int8 ampdu_count = 0, stack_seq = 0;

		for (stack_seq = 0;
			stack_seq < configs->stack.index;
			stack_seq++) {
			struct test_tx_info *tx_info = NULL;

			tx_info = &configs->stack.tx_info[stack_seq];
			ampdu_count = engine_max(ampdu_count,
						 tx_info->mpdu_info.mpdu_cnt);
		}

		ret = net_ad_get_wmm_idx(virtual_device, &wmm_idx);
		if (ret)
			goto err_out;

		ret = ops->op_set_ampdu_ba_limit(winfos,
						wmm_idx,
						ampdu_count);
		if (ret)
			goto err_out;
		else
			SERV_LOG(SERV_DBG_CAT_ENGN,
				 SERV_DBG_LVL_TRACE,
				 ("%s: limit aggregation to %d for WMM%d\n",
				  __func__, ampdu_count, wmm_idx));
	}

	if (ops->op_set_sta_pause_cr) {
		ret = ops->op_set_sta_pause_cr(winfos);
		if (ret)
			goto err_out;
	}

	ret = mt_engine_apply_spe_antid(ops, winfos, configs);
	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

#if 0
#ifdef SINGLE_SKU_V2
	Ret = mt_engine_apply_pwr_offset(ad, ctrl_band_idx);
#endif
#endif
err_out:
	if (ret)
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: err=0x%04x\n", __func__, ret));

	return ret;
}

s_int32 mt_engine_start(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops,
	struct test_bk_cr *bks,
	struct test_rx_stat *rx_stat,
	u_int32 en_log)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config;
	/* TODO: factor out here for rx_filter data struct */
	struct rx_filter_ctrl rx_filter;
	u_char band_idx = 0, band_num = 0;

	band_num = (IS_TEST_DBDC(winfos)) ? 2 : 1;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	for (band_idx = TEST_DBDC_BAND0;
		band_idx < band_num; band_idx++) {
		test_config = &configs[band_idx];

		/*** Step1: Sanity check ***/
		/* Make sure mt_engine_init_band_info successfully when interface up */
		if (test_config->op_mode & OP_MODE_START) {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
				("%s: test mode has already started, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
				("wdev_idx:%u, band_idx:%u\n",
				test_config->wdev_idx, band_idx));

			goto done;
		}

		/* Allocate tx packet buffer */
		if (!test_config->test_pkt) {
			ret = sys_ad_alloc_mem(
				(pu_char *)&test_config->test_pkt,
				TEST_PKT_LEN);
			if (ret)
				goto err;
		}

		/*** Step2: Remind firmware that enable test mode ***/
		ret = ops->op_set_fw_mode(winfos, 1);
		if (ret)
			goto err;

		/* TODO: factor out here for BF related init */
#if 0
#if (defined(MT_MAC))
#ifdef TXBF_SUPPORT
		/* Before going into ATE mode, stop sounding first */
		mt_Trigger_Sounding_Packet(pAd, FALSE, 0, 0, 0, NULL);
#endif /* TXBF_SUPPORT */
#endif /* MT_MAC */
#endif
		/* TODO: factor out here for PCIE related init */
#if 0
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
		mt_asic_pcie_aspm_dym_ctrl(pAd, band_idx, FALSE, FALSE);
		set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#endif

		/*** Step3: Backup and set CRs for test mode ***/
		/*
		 *  Backup original CRs and change to test mode specific CR settings.
		 *  Restore it back when go back to normal mode.
		 */
		ret = ops->op_backup_and_set_cr(winfos, bks, band_idx);
		if (ret)
			goto err;

		/*** Step4: Init mps/rx stat ***/
		ret = mt_engine_init_mps(test_config);
		if (ret)
			goto err;

		/*** Step5: Disable rmac and configure rx filter ***/
		/***        And disalble TMAC as well.	         ***/
		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_TXRX, FALSE, band_idx);
		if (ret)
			goto err;

		/* Rx filter */
		sys_ad_zero_mem(&rx_filter, sizeof(rx_filter));
		rx_filter.bPromiscuous = FALSE;
		rx_filter.bFrameReport = TRUE;
		rx_filter.filterMask = SERV_RX_FILTER_NOT_OWN_BTIM
					| SERV_RX_FILTER_NOT_OWN_UCAST
					| SERV_RX_FILTER_RTS
					| SERV_RX_FILTER_CTS
					| SERV_RX_FILTER_CTRL_RSV
					| SERV_RX_FILTER_BC_MC_DIFF_BSSID_A2
					| SERV_RX_FILTER_BC_MC_DIFF_BSSID_A3
					| SERV_RX_FILTER_BC_MC_OWN_MAC_A3
					| SERV_RX_FILTER_PROTOCOL_VERSION
					| SERV_RX_FILTER_FCS_ERROR;
		rx_filter.u1BandIdx = band_idx;
		ret = ops->op_set_rx_filter(winfos, rx_filter);
		if (ret)
			goto err;

		test_config->tx_ant = BITS(0, (winfos->chip_cap.mcs_nss.max_nss-1));
		test_config->rx_ant = BITS(0, (winfos->chip_cap.mcs_nss.max_nss-1));
	}

	/*** Step6: Init tx thread ***/
	ret = net_ad_init_thread(winfos, configs, SERV_THREAD_TEST);
	if (ret)
		goto err;

	/*** Step7: Stop normal function ***/
	ret = net_ad_exit_normal(winfos, bak);
	if (ret)
		goto err;

	for (band_idx = TEST_DBDC_BAND0;
		band_idx < band_num; band_idx++) {
		test_config = &configs[band_idx];
		sys_ad_zero_mem(&test_config->stack,
				sizeof(test_config->stack));

		/*** Step8: Set op mode and init wdev/txpwr ***/
		/* call wdev_do_open must be behind ATE status set */
		/* call wdev_do_open must be after mt_adaption_stop_ap */
		test_config->op_mode = OP_MODE_START;
		ret = net_ad_init_wdev(winfos, test_config, band_idx);
		if (ret)
			goto err;

		/* Init tx power feature */
		ret = net_ad_init_txpwr(winfos, test_config, band_idx);
		if (ret)
			goto err;

		if (winfos->dbdc_mode)
			test_config->stack.entry_limit = MAX_MULTI_TX_STA/2;
		else
			test_config->stack.entry_limit = MAX_MULTI_TX_STA;

		test_config->is_alloc_skb = 0;

		sys_ad_zero_mem(test_config->tx_method,
				sizeof(u_int8)*TEST_MODE_NUM);
		test_config->tx_method[TEST_MODE_HE_MU] = 1;
		test_config->tx_method[TEST_MODE_VHT_MIMO] = 1;
		test_config->max_pkt_ext = 2;
		test_config->hetb_rx_csd = 0x240004000060FF;
	}

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: enter test mode, wdev_idx=%u\n",
		__func__, test_config->wdev_idx));

done:
	return ret;

err:
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("%s: enter test mode fail, err=0x%8x\n", __func__, ret));

	return ret;
}

s_int32 mt_engine_stop(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops,
	struct test_bk_cr *bks,
	struct test_log_dump_cb *log_dump)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config;
	struct test_log_dump_cb *log_cb = NULL;
	void **pkt_skb = NULL;
	/* TODO: factor out here for rx_filter data struct */
	struct rx_filter_ctrl rx_filter;
	u_char band_idx;
	u_int16 rsp_len = 8;
	u_int8 icap_len = 0;
	u_int8 idx = 0, band_num = 0;

	band_num = (IS_TEST_DBDC(winfos)) ? 2 : 1;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	for (band_idx = TEST_DBDC_BAND0;
		band_idx < band_num; band_idx++) {
		test_config = &configs[band_idx];

		/* Rollback channel */
		if (band_idx == TEST_DBDC_BAND0)
			test_config->channel = 1;
		else
			test_config->channel = 36;

		/*** Step1: Sanity check ***/
		if ((test_config->op_mode & OP_MODE_STOP)
			|| !(test_config->op_mode & OP_MODE_START)) {
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
				("%s: test mode has already stopped, ",
				__func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
				("wdev_idx=%u, band_idx=%u\n",
				test_config->wdev_idx, band_idx));

			test_config->op_mode = OP_MODE_STOP;

			goto done;
		}

		/*** Step2: Restore CRs for normal mode ***/
		ret = ops->op_restore_cr(winfos, bks, band_idx, RESTORE_ALL);
		if (ret)
			goto err;

		/*** Step3: Remind FW that switch mode ***/
		/*
		 * TODO: factor out here for rf test mode
		 * duplicated behavior reviewing
		 */
		if (test_config->op_mode & OP_MODE_FFT) {

			ret = ops->op_set_rf_test_mode(winfos,
						fTEST_OPER_NORMAL_MODE,
						icap_len,
						rsp_len);

			/* For FW to switch back to normal mode stable time */
			/* mdelay(2000); */
			if (ret)
				goto err;

			test_config->op_mode &= ~OP_MODE_FFT;
		}

		if (test_config->op_mode & fTEST_IN_RFTEST) {

			ret = ops->op_set_rf_test_mode(winfos,
						fTEST_OPER_NORMAL_MODE,
						icap_len,
						rsp_len);

			/* For FW to switch back to normal mode stable time */
			/* mdelay(2000); */
			if (ret)
				goto err;

			test_config->op_mode &= ~fTEST_IN_RFTEST;
		}

		/*** Step4: Configure rx filter ***/
		sys_ad_zero_mem(&rx_filter, sizeof(rx_filter));
		rx_filter.bPromiscuous = FALSE;
		rx_filter.bFrameReport = FALSE;
		rx_filter.filterMask = SERV_RX_FILTER_NOT_OWN_BTIM
					| SERV_RX_FILTER_NOT_OWN_UCAST
					| SERV_RX_FILTER_RTS
					| SERV_RX_FILTER_CTS
					| SERV_RX_FILTER_CTRL_RSV
					| SERV_RX_FILTER_BC_MC_DIFF_BSSID_A2
					| SERV_RX_FILTER_BC_MC_DIFF_BSSID_A3
					| SERV_RX_FILTER_BC_MC_OWN_MAC_A3
					| SERV_RX_FILTER_PROTOCOL_VERSION
					| SERV_RX_FILTER_FCS_ERROR;
		rx_filter.u1BandIdx = band_idx;
		ret = ops->op_set_rx_filter(winfos, rx_filter);
		if (ret)
			goto err;

		/*** Step5: Release mps/skb ***/
		ret = mt_engine_release_mps(test_config);
		if (ret)
			goto err;

		/* Release skb */
		pkt_skb = &test_config->pkt_skb;

		if (*pkt_skb) {
			sys_ad_free_pkt(*pkt_skb);
			*pkt_skb = NULL;
		}

		/* release log dump */
		for (idx = TEST_LOG_RXV; idx < TEST_LOG_TYPE_NUM; idx++) {
			log_cb = &log_dump[idx - 1];

			if (log_cb->entry) {
				sys_ad_free_mem(log_cb->entry);
				log_cb->entry = NULL;

				SERV_OS_FREE_SPIN_LOCK(&log_cb->lock);
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					("%s: free log cb %d\n",
					__func__, idx));
			}
		}

		/*** Step6: Release wdev and set op mode ***/
		/* call wdev_do_close must be before mt_adaption_startup_ap */
		ret = net_ad_release_wdev(winfos, test_config, band_idx);
		if (ret)
			goto err;

		test_config->op_mode = OP_MODE_STOP;

		/*** Step7: Remind FW that disable test mode ***/
		ret = ops->op_set_fw_mode(winfos, 0);
		if (ret)
			goto err;

#if defined(CONFIG_AP_SUPPORT) && defined(CFG_SUPPORT_FALCON_MURU)
		if (ops->op_set_tam_arb)
			ops->op_set_tam_arb(winfos, 0x1);
#endif

		/*** Step8: Enable tmac/rmac/rxv ***/
		ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_TXRX, TRUE, band_idx);
		if (ret)
			goto err;
	}

	/*** Step9: resume normal function ***/
	ret = net_ad_enter_normal(winfos, bak);
	if (ret)
		goto err;

	/*** Step10: Release tx thread ***/
	ret = net_ad_release_thread(0);
	/* msleep(2); */
	if (ret)
		goto err;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: leave test mode, wdev_idx=%u\n",
		__func__, test_config->wdev_idx));

done:
	return ret;

err:
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("%s: leave test mode fail, err=0x%8x\n", __func__, ret));

	test_config->op_mode = OP_MODE_STOP;

	return ret;
}

s_int32 mt_engine_calc_ipg_param_by_ipg(
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct ipg_param *ipg_param;
	u_char tx_mode;
	u_int32 ipg, real_ipg;
	u_int8 sig_ext, aifsn;
	u_int16 slot_time, sifs_time, cw;

	ipg_param = &configs->ipg_param;
	tx_mode = configs->tx_mode;
	ipg = ipg_param->ipg;
	sig_ext = mt_engine_get_sigext_time_by_phymode(tx_mode);
	slot_time = mt_engine_get_slot_time_by_phymode(tx_mode);
	sifs_time = TEST_DEFAULT_SIFS_TIME;

	/*
	 *  1. ipg = sig_ext + sifs_time + slot_time
	 *  2. ipg = sig_ext + sifs_time + aifsn * slot_time
	 *      + ((1 << cw) - 1) * slot_time
	 *  If it's CCK mode, there's no need to consider sig_ext
	 *  And it's no need to count in backoff time in older design
	 *  Configure SIFS/SLOT only
	 *  Consider which ac queue will be used each case
	 */
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: expected ipg=%d\n", __func__, ipg));

	if (ipg < (sig_ext + sifs_time + slot_time)) {
		ipg_param->ipg = 0;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("%s: invalid ipg!! ", __func__));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("sig_ext/slot_time/sifs_time=%d/%d/%d, ",
			sig_ext, slot_time, sifs_time));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
			("set ipg=%d\n", ipg));

		if (configs->duty_cycle > 0) {
			configs->duty_cycle = 0;
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("%s: invalid ipg!! ", __func__));
			SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
				("set duty_cycle=%d\n", configs->duty_cycle));
		}

		return ret;
	}

	ipg -= sig_ext;

	if (ipg <= (TEST_MAX_SIFS_TIME + slot_time)) {
		sifs_time = ipg - slot_time;
		aifsn = TEST_MIN_AIFSN;
		cw = 0;
	} else {
		cw = mt_engine_get_cw(ipg, slot_time);
		ipg -= ((1 << cw) - 1) * slot_time;
		aifsn = ipg / slot_time;

		if (aifsn >= TEST_MAX_AIFSN)
			aifsn = TEST_MAX_AIFSN;

		ipg -= aifsn * slot_time;

		if (ipg <= TEST_DEFAULT_SIFS_TIME)
			sifs_time = TEST_DEFAULT_SIFS_TIME;
		else if ((ipg > TEST_DEFAULT_SIFS_TIME) &&
			 (ipg <= TEST_MAX_SIFS_TIME))
			sifs_time = ipg;
		else
			sifs_time = TEST_MAX_SIFS_TIME;
	}

	real_ipg = sig_ext + sifs_time + aifsn * slot_time
			+ ((1 << cw) - 1) * slot_time;

	ipg_param->sig_ext = sig_ext;
	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;
	ipg_param->txop = 0;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: sig_ext=%d, slot_time=%d, sifs_time=%d, ",
		__func__, sig_ext, slot_time, sifs_time));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("aifsn=%d, cw=%d, ", aifsn, cw));
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("real ipg=%d\n", real_ipg));

	return ret;
}

s_int32 mt_engine_set_auto_resp(
	struct test_wlan_info *winfos,
	struct test_operation *ops,
	struct test_configuration *configs,
	u_char band_idx, u_char mode)
{
	return net_ad_set_auto_resp(winfos, ops,
				    configs, band_idx, mode);
}

s_int32 mt_engine_start_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct ipg_param *ipg_param;
	struct tx_time_param *tx_time_param;
	struct test_tx_stack *stack = NULL;
	u_int32 op_mode, tx_cnt, mcs, duty_cycle, ipg, pkt_tx_time;
	u_char *test_pkt, stack_idx = 0;
	u_char ctrl_ch, ch, bw, dbw;

	ipg_param = &configs->ipg_param;
	tx_time_param = &configs->tx_time_param;
	op_mode = configs->op_mode;
	tx_cnt = configs->tx_stat.tx_cnt;
	mcs = configs->mcs;
	duty_cycle = configs->duty_cycle;
	test_pkt = configs->test_pkt;
	ctrl_ch = configs->ctrl_ch;
	stack = &configs->stack;
	ch = configs->channel;
	bw = configs->bw;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u, ch=%u, ctrl_ch=%u, wdev_idx=%u\n",
		__func__, band_idx, ch, ctrl_ch, configs->wdev_idx));

	if (!test_pkt) {
		ret = SERV_STATUS_ENGINE_INVALID_NULL_POINTER;
		goto err;
	}

	if (op_mode & OP_MODE_TXFRAME) {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("%s: already in TXFRAME mode now, tx is ongoing!\n",
			__func__));
		goto done;
	}

	ret = net_ad_set_device_info(winfos, configs,
					configs->addr3[0], band_idx, 0x1);
	if (ret)
		goto err;

	/* Turn on tx again if set before */
	if (op_mode & OP_MODE_TXFRAME) {
		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_TX, FALSE, band_idx);
		if (ret)
			goto err;
	}

	ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_RX_RXV, FALSE, band_idx);
	if (ret)
		goto err;

	/* Convert DBW - 0: 20MHz, 1: 40MHz, 2: 80 MHz, 3: 80+80MHz for setting CR */
	if (configs->per_pkt_bw <= TEST_BW_80)
		dbw = configs->per_pkt_bw;
	else
		dbw = 3;

	if (configs->tx_mode == TEST_MODE_HE_TB)
		ops->op_hetb_ctrl(winfos, band_idx, OP_HETB_TX_CFG,
				TRUE, dbw, configs->sgi,
				configs->stbc, configs->dmnt_ru_idx,
				&configs->ru_info_list[0]);

	/* Stop tx/rx path queues */
	ret = net_ad_cfg_queue(winfos, FALSE);

	/* NOTE: here needs to polling tx/rx path until packets empty */

	/* Start tx/rx path queues */
	ret = net_ad_cfg_queue(winfos, TRUE);

	/* Apply ipg setting to HW */
	ret = mt_engine_apply_ipg_param(winfos, configs, ops, band_idx);
	if (ret)
		goto err;

	if (tx_cnt != 0xFFFFFFFF) {
		if (configs->tx_strategy != TEST_TX_STRA_THREAD)
			tx_cnt += configs->tx_stat.tx_done_cnt;

		configs->tx_stat.tx_cnt = tx_cnt;
	}

	/* Tx frame */
	op_mode |= OP_MODE_TXFRAME;
	configs->op_mode = op_mode;
	ipg = ipg_param->ipg;
	pkt_tx_time = tx_time_param->pkt_tx_time;

	if ((pkt_tx_time > 0) || (ipg > 0)) {
		u_int32 round = configs->tx_stat.tx_cnt;
		u_int32 rounded = 0;
		u_int32 round_pkt_cnt = 0;
		u_int32 token_limit = winfos->pkt_tx_tkid_max / 2;

		for (stack_idx = 0;
			stack_idx < configs->stack.index;
			stack_idx++) {
			struct test_tx_info *tx_info = NULL;

			tx_info = &stack->tx_info[stack_idx];
			round_pkt_cnt += tx_info->mpdu_info.mpdu_cnt;
		}
		if (round_pkt_cnt == 0)
			round_pkt_cnt = 1;
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
			 ("%s: per round pkt(s)=%d\n",
			 __func__, round_pkt_cnt));
		/* 0x8fffffff means TX continuously */
		if (round > (token_limit/round_pkt_cnt))
			round = token_limit / round_pkt_cnt;

		/* prevent refill flow interrupted
		 * and en-queue unlimited retry txd
		 */
		if ((configs->tx_mode == TEST_MODE_HE_MU ||
			configs->tx_mode == TEST_MODE_VHT_MIMO)
			&& configs->retry) {
			round = 1;
			if (ops->op_set_mu_cnt) {
				ret = ops->op_set_mu_cnt(winfos,
						   stack->virtual_device[0],
							tx_cnt);

				if (ret)
					goto err;
			}

			if (ops->op_trigger_mu_counting) {
				ret = ops->op_trigger_mu_counting(winfos,
						stack->virtual_device[0],
								TRUE);

				if (ret)
					goto err;
			}
		}

		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
			 ("%s: repeat=%d, token_limit=%d, ",
			 __func__, round, token_limit));
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
			 ("pkt_tx_time=%d, ipg=%d\n",
			 pkt_tx_time, ipg));

		/* Enqueue packet in HW queue in advance */
		for (rounded = 0;
		     rounded < round;
		     rounded++) {
			for (stack_idx = 0;
				stack_idx < configs->stack.index;
				stack_idx++) {
				struct test_tx_info *tx_info = NULL;
				u_int8 mpdu_cnt = 0;

				tx_info = &stack->tx_info[stack_idx];
				mpdu_cnt = tx_info->mpdu_info.mpdu_cnt;

				while (mpdu_cnt) {
					net_ad_enq_pkt(winfos,
							configs->ac_idx,
						stack->virtual_wtbl[stack_idx],
					      stack->virtual_device[stack_idx],
						    stack->pkt_skb[stack_idx]);

					configs->tx_stat.txed_cnt++;
					mpdu_cnt--;
				}
			}
		}

		configs->stack.q_idx = 0;
	}

	/* workaround for TX without retry whlie MU/VHT MIMO to prevent:
	 * 1.packet of primary RU dropped then queue length mismatched
	 * 2.packets stay at PLE after TxStop
	 */
	if ((configs->tx_mode == TEST_MODE_HE_MU ||
		configs->tx_mode == TEST_MODE_VHT_MIMO)
		&& (configs->retry == 0))
		net_ad_enq_pkt(winfos,
				      configs->ac_idx,
				      stack->virtual_wtbl[0],
				      stack->virtual_device[0],
				      stack->pkt_skb[0]);

	if (configs->tx_mode == TEST_MODE_HE_TB)
		ops->op_hetb_ctrl(winfos, band_idx, OP_HETB_TX_START,
				TRUE, 0, 0, 0, 0, NULL);

	if (configs->tx_mode < TEST_MODE_HE_MU)
		ret = net_ad_post_tx(winfos,
					configs,
					band_idx,
					NULL);
	if (ret)
		goto err;

	ret = ops->op_set_tr_mac(winfos, SERV_TEST_MAC_TX, TRUE, band_idx);
	if (ret)
		goto err;

	if (op_mode & OP_MODE_RXFRAME) {
		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_RX_RXV, TRUE, band_idx);
		if (ret)
			goto err;
	}

done:
	return ret;
err:
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("%s: err=0x%08x, wdev_idx=%x\n",
		__func__, ret, configs->wdev_idx));

	return ret;
}

s_int32 mt_engine_stop_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 op_mode, ipg, pkt_tx_time;
	struct ipg_param *ipg_param;
	struct tx_time_param *tx_time_param;
	struct test_tx_stack *stack = NULL;

	ipg_param = &configs->ipg_param;
	tx_time_param = &configs->tx_time_param;
	op_mode = configs->op_mode;
	ipg = ipg_param->ipg;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	stack = &configs->stack;

	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u\n", __func__, band_idx));

	if (configs->tx_mode > TEST_MODE_HE_TB) {
		if (ops->op_trigger_mu_counting) {
			ret = ops->op_trigger_mu_counting(winfos,
						stack->virtual_device[0],
						FALSE);
			if (ret)
				goto err;
		}

		if (ops->op_set_mu_cnt) {
			ret = ops->op_set_mu_cnt(winfos,
					    stack->virtual_device[0],
						  0);
			if (ret)
				goto err;
		}
	}

	configs->tx_stat.txed_cnt = 0;

#ifdef ATE_TXTHREAD
	net_ad_thread_stop_tx(winfos);
#endif

	if ((op_mode & OP_MODE_TXFRAME) || (op_mode == OP_MODE_STOP)) {
		op_mode &= ~OP_MODE_TXFRAME;
		configs->op_mode = op_mode;

		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_TX, FALSE, band_idx);
		if (ret)
			goto err;

		if ((pkt_tx_time > 0) || (ipg > 0)) {
			u_char omac_idx = 0, sta_seq = 0;
			/* Flush SW queue */
			ret = net_ad_clean_sta_q(winfos, SERV_WCID_ALL);
			if (ret)
				return ret;

			ret = net_ad_get_omac_idx(
				winfos,
				configs->stack.virtual_device[0],
				&omac_idx);
			if (ret != SERV_STATUS_SUCCESS)
				goto err;

			/*
			 * Clean per sta tx queue and disable STA pause CRs
			 * for transmitting packet
			 */
			for (sta_seq = 0;
				sta_seq < stack->index;
				sta_seq++) {
				ret = ops->op_set_clean_persta_txq(
					winfos, FALSE,
					stack->virtual_wtbl[sta_seq],
					omac_idx, band_idx);
				if (ret != SERV_STATUS_SUCCESS)
					goto err;
			}
			/* restore TMAC CRs without considering IPG */
			ret = ops->op_restore_cr(winfos, NULL,
						 band_idx, RESTORE_TX);
			if (ret)
				goto err;
		}
	}

	if (configs->tx_mode == TEST_MODE_HE_TB)
		ops->op_hetb_ctrl(winfos, band_idx, OP_HETB_TX_STOP,
					TRUE, 0, 0, 0, 0, NULL);

err:
	return ret;
}

s_int32 mt_engine_start_rx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 op_mode;
	u_int16 user_idx = 0;

	op_mode = configs->op_mode;
	if (op_mode & OP_MODE_RXFRAME) {
		SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_WARN,
			("%s: already in RXFRAME mode now, rx is ongoing!\n",
			__func__));
		goto done;
	}

	ret = net_ad_set_device_info(winfos, configs,
					configs->addr3[user_idx], band_idx, 0x1);
	if (ret)
		goto err;

	/* Turn off tx if set before */
	if (op_mode & OP_MODE_TXFRAME) {
		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_TX, FALSE, band_idx);
		if (ret)
			goto err;
	}

	user_idx = configs->user_idx;
	ret = ops->op_set_rx_user_idx(winfos, band_idx, user_idx);
	if (ret)
		goto err;

	ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_RX_RXV, FALSE, band_idx);
	if (ret)
		goto err;

	/* Stop tx/rx path queues */
	ret = net_ad_cfg_queue(winfos, FALSE);

	/* NOTE: here needs to polling tx/rx path until packets empty */

	/* Start tx/rx path queues */
	ret = net_ad_cfg_queue(winfos, TRUE);

	/* Turn on tx again if set before */
	if (op_mode & OP_MODE_TXFRAME) {
		ret = ops->op_set_tr_mac(
				winfos, SERV_TEST_MAC_TX, TRUE, band_idx);
		if (ret)
			goto err;
	}

	ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_RX_RXV, TRUE, band_idx);
	if (ret)
		goto err;

	ret = ops->op_set_phy_counter(winfos, 0, TEST_DBDC_BAND0);
	if (ret)
		goto err;

	ret = ops->op_set_phy_counter(winfos, 1, TEST_DBDC_BAND0);
	if (ret)
		goto err;

	if (IS_TEST_DBDC(winfos)) {
		ret = ops->op_set_phy_counter(winfos, 0, TEST_DBDC_BAND1);
		if (ret)
			goto err;

		ret = ops->op_set_phy_counter(winfos, 1, TEST_DBDC_BAND1);
		if (ret)
			goto err;
	}

	op_mode |= OP_MODE_RXFRAME;
	configs->op_mode = op_mode;

	if (configs->tx_mode == TEST_MODE_HE_TB) {
		u_int8 sta_idx = 0;
		u_int8 dbw = 0;
		struct test_ru_info *ru_info = &configs->ru_info_list[0];

		for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			if (ru_info[sta_idx].valid) {
			/* 13 bytes for Delimiter+FCS+A_control+HW reserved */
				mt_engine_calc_phy(&ru_info[sta_idx],
						ru_info[sta_idx].mpdu_length+13,
							configs->stbc,
							configs->sgi,
							configs->max_pkt_ext*2);
			}
		}
		if (mt_engine_search_dom_ru(configs) == NULL) {
			ret = SERV_STATUS_ENGINE_FAIL;
			goto err;
		}

		/* Convert DBW - 0: 20MHz, 1: 40MHz, 2: 80 MHz, 3: 80+80MHz for setting CR */
		if (configs->per_pkt_bw <= TEST_BW_80)
			dbw = configs->per_pkt_bw;
		else
			dbw = 3;

		ops->op_hetb_ctrl(winfos, band_idx, OP_HETB_RX_CFG,
				TRUE, dbw, configs->sgi,
				configs->stbc, configs->dmnt_ru_idx,
				&configs->ru_info_list[0]);
	} else if (configs->tx_mode == TEST_MODE_HE_MU) {
		if (configs->mu_rx_aid)
			ops->op_set_ru_aid(winfos, band_idx,
					configs->mu_rx_aid);
		else {
			/* 0xf800 to disable */
			ops->op_set_ru_aid(winfos, band_idx, 0xf800);
		}
	}

done:
	return ret;

err:
	SERV_LOG(SERV_DBG_CAT_ENGN, SERV_DBG_LVL_ERROR,
		("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_engine_stop_rx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 op_mode;

	op_mode = configs->op_mode;

	/* turn off TMAC due to AutoResp feature will switch TMAC on */
	ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_TX, FALSE, band_idx);

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	ret = ops->op_set_tr_mac(
			winfos, SERV_TEST_MAC_RX_RXV, FALSE, band_idx);

	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	op_mode &= ~OP_MODE_RXFRAME;
	configs->op_mode = op_mode;

	if (configs->tx_mode == TEST_MODE_HE_TB) {
		ops->op_hetb_ctrl(winfos, band_idx, OP_HETB_RX_CFG,
				FALSE, configs->per_pkt_bw, configs->sgi,
				configs->stbc, configs->dmnt_ru_idx,
				&configs->ru_info_list[0]);
	} else if (configs->tx_mode == TEST_MODE_HE_MU) {
		/* 0xf800 to disable */
		ops->op_set_ru_aid(winfos, band_idx, 0xf800);
	}

err_out:
	return ret;
}
