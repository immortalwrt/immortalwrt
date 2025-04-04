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
	service_test.c
*/
#include "service_test.h"

u_char template_frame[32] = { 0x88, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
	0x00, 0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*****************************************************************************
 *	Internal functions
 *****************************************************************************/
static s_int32 mt_serv_init_op(struct test_operation *ops)
{
	ops->op_set_tr_mac = mt_op_set_tr_mac;
	ops->op_set_tx_stream = mt_op_set_tx_stream;
	ops->op_set_tx_path = mt_op_set_tx_path;
	ops->op_set_rx_path = mt_op_set_rx_path;
	ops->op_set_rx_filter = mt_op_set_rx_filter;
	ops->op_set_clean_persta_txq = mt_op_set_clean_persta_txq;
	ops->op_set_cfg_on_off = mt_op_set_cfg_on_off;
	ops->op_log_on_off = mt_op_log_on_off;
	ops->op_dbdc_tx_tone = mt_op_dbdc_tx_tone;
	ops->op_dbdc_tx_tone_pwr = mt_op_dbdc_tx_tone_pwr;
	ops->op_dbdc_continuous_tx = mt_op_dbdc_continuous_tx;
	ops->op_get_tx_info = mt_op_get_tx_info;
	ops->op_get_thermal_value = mt_op_get_thermal_value;
	ops->op_set_antenna_port = mt_op_set_antenna_port;
	ops->op_set_slot_time = mt_op_set_slot_time;
	ops->op_set_power_drop_level = mt_op_set_power_drop_level;
	ops->op_get_antswap_capability = mt_op_get_antswap_capability;
	ops->op_set_antswap = mt_op_set_antswap;
	ops->op_set_eeprom_to_fw = mt_op_set_eeprom_to_fw;
	ops->op_set_rx_filter_pkt_len = mt_op_set_rx_filter_pkt_len;
	ops->op_set_freq_offset = mt_op_set_freq_offset;
	ops->op_set_phy_counter = mt_op_set_phy_counter;
	ops->op_set_rxv_index = mt_op_set_rxv_index;
	ops->op_set_fagc_path = mt_op_set_fagc_path;
	ops->op_set_fw_mode = mt_op_set_fw_mode;
	ops->op_set_rf_test_mode = mt_op_set_rf_test_mode;
	ops->op_set_test_mode_start = mt_op_set_test_mode_start;
	ops->op_set_test_mode_abort = mt_op_set_test_mode_abort;
	ops->op_start_tx = mt_op_start_tx;
	ops->op_stop_tx = mt_op_stop_tx;
	ops->op_start_rx = mt_op_start_rx;
	ops->op_stop_rx = mt_op_stop_rx;
	ops->op_set_channel = mt_op_set_channel;
	ops->op_set_tx_content = mt_op_set_tx_content;
	ops->op_set_preamble = mt_op_set_preamble;
	ops->op_set_system_bw = mt_op_set_system_bw;
	ops->op_set_per_pkt_bw = mt_op_set_per_pkt_bw;
	ops->op_reset_txrx_counter = mt_op_reset_txrx_counter;
	ops->op_set_rx_vector_idx = mt_op_set_rx_vector_idx;
	ops->op_set_fagc_rssi_path = mt_op_set_fagc_rssi_path;
	ops->op_get_rx_stat_leg = mt_op_get_rx_stat_leg;
	ops->op_get_rxv_dump_ring_attr = mt_op_get_rxv_dump_ring_attr;
	ops->op_get_rxv_dump_rxv_content = mt_op_get_rxv_dump_rxv_content;
	ops->op_get_rxv_dump_action = mt_op_get_rxv_dump_action;
	ops->op_get_rxv_content_len = mt_op_get_rxv_content_len;
	ops->op_get_rx_statistics_all = mt_op_get_rx_statistics_all;
	ops->op_calibration_test_mode = mt_op_calibration_test_mode;
	ops->op_set_icap_start = mt_op_set_icap_start;
	ops->op_get_icap_status = mt_op_get_icap_status;
	ops->op_get_icap_max_data_len = mt_op_get_icap_max_data_len;
	ops->op_get_icap_data = mt_op_get_icap_data;
	ops->op_do_cal_item = mt_op_do_cal_item;
	ops->op_set_band_mode = mt_op_set_band_mode;
	ops->op_get_chipid = mt_op_get_chipid;
	ops->op_get_sub_chipid = mt_op_get_sub_chipid;
	ops->op_mps_set_seq_data = mt_op_mps_set_seq_data;
	ops->op_get_tx_pwr = mt_op_get_tx_pwr;
	ops->op_set_tx_pwr = mt_op_set_tx_pwr;
	ops->op_get_freq_offset = mt_op_get_freq_offset;
	ops->op_get_cfg_on_off = mt_op_get_cfg_on_off;
	ops->op_get_tx_tone_pwr = mt_op_get_tx_tone_pwr;
	ops->op_get_recal_cnt = mt_op_get_recal_cnt;
	ops->op_get_recal_content = mt_op_get_recal_content;
	ops->op_get_rxv_cnt = mt_op_get_rxv_cnt;
	ops->op_get_rxv_content = mt_op_get_rxv_content;
	ops->op_get_thermal_val = mt_op_get_thermal_val;
	ops->op_set_cal_bypass = mt_op_set_cal_bypass;
	ops->op_set_dpd = mt_op_set_dpd;
	ops->op_set_tssi = mt_op_set_tssi;
	ops->op_set_rdd_test = mt_op_set_rdd_test;
	ops->op_get_wf_path_comb = mt_op_get_wf_path_comb;
	ops->op_set_off_ch_scan = mt_op_set_off_ch_scan;
	ops->op_get_rdd_cnt = mt_op_get_rdd_cnt;
	ops->op_get_rdd_content = mt_op_get_rdd_content;
	ops->op_evt_rf_test_cb = mt_op_evt_rf_test_cb;
	ops->op_set_muru_manual = mt_op_set_muru_manual;
	ops->op_set_tam_arb = mt_op_set_tam_arb;
	ops->op_set_mu_cnt = mt_op_set_mu_count;
	ops->op_trigger_mu_counting = mt_op_trigger_mu_counting;
	ops->op_hetb_ctrl = mt_op_hetb_ctrl;
	ops->op_set_ru_aid = mt_op_set_ru_aid;
	ops->op_set_mutb_spe = mt_op_set_mutb_spe;
	ops->op_get_rx_stat_band = mt_op_get_rx_stat_band;
	ops->op_get_rx_stat_path = mt_op_get_rx_stat_path;
	ops->op_get_rx_stat_user = mt_op_get_rx_stat_user;
	ops->op_get_rx_stat_comm = mt_op_get_rx_stat_comm;
	ops->op_set_rx_user_idx = mt_op_set_rx_user_idx;
	/* For test mac usage */
	ops->op_backup_and_set_cr = mt_op_backup_and_set_cr;
	ops->op_restore_cr = mt_op_restore_cr;
	ops->op_set_ampdu_ba_limit = mt_op_set_ampdu_ba_limit;
	ops->op_set_sta_pause_cr = mt_op_set_sta_pause_cr;
	ops->op_set_ifs_cr = mt_op_set_ifs_cr;
	ops->op_write_mac_bbp_reg = mt_op_write_mac_bbp_reg;
	ops->op_read_bulk_mac_bbp_reg = mt_op_read_bulk_mac_bbp_reg;
	ops->op_read_bulk_rf_reg = mt_op_read_bulk_rf_reg;
	ops->op_write_bulk_rf_reg = mt_op_write_bulk_rf_reg;
	ops->op_read_bulk_eeprom = mt_op_read_bulk_eeprom;

	/* For DNL+TSSI Calibration */
	ops->op_set_test_mode_dnlk_clean = mt_op_set_test_mode_dnlk_clean;
	ops->op_set_test_mode_rxgaink    = mt_op_set_test_mode_rxgaink;

	/* iBF phase calibration */
#ifdef TXBF_SUPPORT
	ops->op_set_ibf_phase_cal_e2p_update =
					mt_op_set_ibf_phase_cal_e2p_update;
	ops->op_set_ibf_phase_cal_init = mt_op_set_ibf_phase_cal_init;
	ops->op_set_wite_txbf_pfmu_tag = mt_op_set_wite_txbf_pfmu_tag;
	ops->op_set_txbf_pfmu_tag_invalid = mt_op_set_txbf_pfmu_tag_invalid;
	ops->op_set_txbf_pfmu_tag_idx = mt_op_set_txbf_pfmu_tag_idx;
	ops->op_set_txbf_pfmu_tag_bf_type = mt_op_set_txbf_pfmu_tag_bf_type;
	ops->op_set_txbf_pfmu_tag_dbw = mt_op_set_txbf_pfmu_tag_dbw;
	ops->op_set_txbf_pfmu_tag_sumu = mt_op_set_txbf_pfmu_tag_sumu;
	ops->op_set_wrap_ibf_cal_get_ibf_mem_alloc =
					mt_op_get_wrap_ibf_cal_ibf_mem_alloc;
	ops->op_set_wrap_ibf_cal_get_ebf_mem_alloc =
					mt_op_get_wrap_ibf_cal_ebf_mem_alloc;
	ops->op_set_txbf_pfmu_tag_mem = mt_op_set_txbf_pfmu_tag_mem;
	ops->op_set_txbf_pfmu_tag_matrix = mt_op_set_txbf_pfmu_tag_matrix;
	ops->op_set_txbf_pfmu_tag_snr = mt_op_set_txbf_pfmu_tag_snr;
	ops->op_set_txbf_pfmu_tag_smart_ant = mt_op_set_txbf_pfmu_tag_smart_ant;
	ops->op_set_txbf_pfmu_tag_se_idx = mt_op_set_txbf_pfmu_tag_se_idx;
	ops->op_set_txbf_pfmu_tag_rmsd_thrd = mt_op_set_txbf_pfmu_tag_rmsd_thrd;
	ops->op_set_txbf_pfmu_tag_time_out = mt_op_set_txbf_pfmu_tag_time_out;
	ops->op_set_txbf_pfmu_tag_desired_bw =
					mt_op_set_txbf_pfmu_tag_desired_bw;
	ops->op_set_txbf_pfmu_tag_desired_nr =
					mt_op_set_txbf_pfmu_tag_desired_nr;
	ops->op_set_txbf_pfmu_tag_desired_nc =
					mt_op_set_txbf_pfmu_tag_desired_nc;
	ops->op_set_manual_assoc = mt_op_set_manual_assoc;
#endif
	return SERV_STATUS_SUCCESS;
}

static s_int32 mt_serv_init_config(
	struct test_configuration *configs, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (band_idx == TEST_DBDC_BAND0) {
		/* Operated mode init */
		configs->op_mode = OP_MODE_STOP;

		/* Test packet init */
		ret = sys_ad_alloc_mem((pu_char *) &configs->test_pkt,
					TEST_PKT_LEN);
		configs->pkt_skb = NULL;

		/* OS related  init */
		SERV_OS_INIT_COMPLETION(&configs->tx_wait);
		configs->tx_status = 0;

		/* Hardware resource init */
		configs->wdev_idx = 0;
		configs->wmm_idx = 0;
		configs->ac_idx = SERV_QID_AC_BE;

		/* Tx frame init */
		sys_ad_move_mem(&configs->template_frame, &template_frame, 32);
		configs->addr1[0][0] = 0x00;
		configs->addr1[0][1] = 0x11;
		configs->addr1[0][2] = 0x22;
		configs->addr1[0][3] = 0xAA;
		configs->addr1[0][4] = 0xBB;
		configs->addr1[0][5] = 0xCC;
		sys_ad_move_mem(configs->addr2[0], configs->addr1[0],
				SERV_MAC_ADDR_LEN);
		sys_ad_move_mem(configs->addr3[0], &configs->addr1[0],
				SERV_MAC_ADDR_LEN);
		configs->payload[0] = 0xAA;
		configs->seq = 0;
		configs->hdr_len = SERV_LENGTH_802_11;
		configs->pl_len = 1;
		configs->tx_len = 1058;
		configs->fixed_payload = 2;
		configs->max_pkt_ext = 2;
		configs->retry = 1;

		/* Tx strategy/type init */
		configs->txs_enable = FALSE;
		configs->tx_strategy = TEST_TX_STRA_THREAD;
		configs->rate_ctrl_type = TEST_TX_TYPE_TXD;

		/* Tx timing init */
		configs->duty_cycle = 0;
		configs->tx_time_param.pkt_tx_time_en = FALSE;
		configs->tx_time_param.pkt_tx_time = 0;
		configs->ipg_param.ipg = 50;
		configs->ipg_param.sig_ext = TEST_SIG_EXTENSION;
		configs->ipg_param.slot_time = TEST_DEFAULT_SLOT_TIME;
		configs->ipg_param.sifs_time = TEST_DEFAULT_SIFS_TIME;
		configs->ipg_param.ac_num = SERV_QID_AC_BE;
		configs->ipg_param.aifsn = TEST_MIN_AIFSN;
		configs->ipg_param.cw = TEST_MIN_CW;
		configs->ipg_param.txop = 0;

		/* Rx init */
		sys_ad_zero_mem(&configs->own_mac, SERV_MAC_ADDR_LEN);

		/* Test tx statistic and txs init */
		configs->txs_enable = FALSE;
		sys_ad_zero_mem(&configs->tx_stat,
				sizeof(struct test_tx_statistic));

		/* Phy */
		configs->tx_ant = 1;
		configs->rx_ant = 1;

		/* TODO: factor out here for phy */
		configs->channel = 1;
		configs->ch_band = 0;
		configs->ctrl_ch = 1;
#if 0
		if (BOARD_IS_5G_ONLY(pAd))
			configs->channel = 36;
		else
			configs->channel = 1;
#endif
		configs->tx_mode = TEST_MODE_OFDM;
		configs->bw = TEST_BW_20;
		configs->mcs = 7;
		configs->sgi = 0;

		/* rx stat user config */
		configs->user_idx = 1;

		/* tx power */
		configs->tx_pwr_sku_en = FALSE;
		configs->tx_pwr_percentage_en = FALSE;
		configs->tx_pwr_backoff_en = FALSE;
		configs->tx_pwr_percentage_level = 100;
	}
#ifdef DBDC_MODE
	else if (band_idx == TEST_DBDC_BAND1) {
		/* Operated mode init */
		configs->op_mode = OP_MODE_STOP;

		/* Test packet init */
		ret = sys_ad_alloc_mem((pu_char *) &configs->test_pkt,
					   TEST_PKT_LEN);
		configs->pkt_skb = NULL;

		/* OS related  init */
		SERV_OS_INIT_COMPLETION(&configs->tx_wait);
		configs->tx_status = 0;

		/* Hardware resource init */
		configs->wdev_idx = 1;
		configs->wmm_idx = 1;
		configs->ac_idx = SERV_QID_AC_BE;

		/* Tx frame init */
		sys_ad_move_mem(&configs->template_frame, &template_frame, 32);
		configs->addr1[0][0] = 0x00;
		configs->addr1[0][1] = 0x11;
		configs->addr1[0][2] = 0x22;
		configs->addr1[0][3] = 0xDD;
		configs->addr1[0][4] = 0xEE;
		configs->addr1[0][5] = 0xFF;
		sys_ad_move_mem(&configs->addr2, &configs->addr1,
				SERV_MAC_ADDR_LEN);
		sys_ad_move_mem(&configs->addr3, &configs->addr1,
				SERV_MAC_ADDR_LEN);
		configs->payload[0] = 0xAA;
		configs->seq = 0;
		configs->hdr_len = SERV_LENGTH_802_11;
		configs->pl_len = 1;
		configs->tx_len = 1024;
		configs->fixed_payload = 2;
		configs->max_pkt_ext = 2;
		configs->retry = 1;

		/* Tx strategy/type init */
		configs->tx_strategy = TEST_TX_STRA_THREAD;
		configs->rate_ctrl_type = TEST_TX_TYPE_TXD;

		/* Tx timing init */
		configs->duty_cycle = 0;
		configs->tx_time_param.pkt_tx_time_en = FALSE;
		configs->tx_time_param.pkt_tx_time = 0;
		configs->ipg_param.ipg = 50;
		configs->ipg_param.sig_ext = TEST_SIG_EXTENSION;
		configs->ipg_param.slot_time = TEST_DEFAULT_SLOT_TIME;
		configs->ipg_param.sifs_time = TEST_DEFAULT_SIFS_TIME;
		configs->ipg_param.ac_num = SERV_QID_AC_BE;
		configs->ipg_param.aifsn = TEST_MIN_AIFSN;
		configs->ipg_param.cw = TEST_MIN_CW;
		configs->ipg_param.txop = 0;

		/* Rx init */
		sys_ad_zero_mem(&configs->own_mac, SERV_MAC_ADDR_LEN);

		/* Test tx statistic and txs init */
		configs->txs_enable = FALSE;
		sys_ad_zero_mem(&configs->tx_stat,
				sizeof(struct test_tx_statistic));

		/* Phy */
		configs->tx_ant = 1;
		configs->rx_ant = 1;

		/* TODO: factor out here for phy */
		configs->channel = 36;
		configs->ch_band = 1;
		configs->ctrl_ch = 36;
#if 0
		if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
			configs->channel = 100;
			configs->ctrl_ch = 100;
		} else {
			configs->channel = 36;
			configs->ctrl_ch = 36;
		}
#endif
		configs->tx_mode = TEST_MODE_OFDM;
		configs->bw = TEST_BW_20;
		configs->mcs = 7;
		configs->sgi = 0;

		/* rx stat user config */
		configs->user_idx = 1;

	}
#endif /* DBDC_MODE */
	else {
		return SERV_STATUS_SERV_TEST_INVALID_BANDIDX;
	}

	return ret;
}

static s_int32 mt_serv_release_config(
	struct test_configuration *configs, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (band_idx == TEST_DBDC_BAND0) {
		if (configs->test_pkt) {
			sys_ad_free_mem(configs->test_pkt);
			configs->test_pkt = NULL;
		}
	}
#ifdef DBDC_MODE
	else if (band_idx == TEST_DBDC_BAND1) {
		if (configs->test_pkt) {
			sys_ad_free_mem(configs->test_pkt);
			configs->test_pkt = NULL;
		}
	}
#endif /* DBDC_MODE */
	else {
		return SERV_STATUS_SERV_TEST_INVALID_BANDIDX;
	}

	return ret;
}

/*****************************************************************************
 *	Extern functions
 *****************************************************************************/
/* For test mode init of service.git */
s_int32 mt_serv_init_test(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char band_idx = 0, band_num = 0;

	band_num = (IS_TEST_DBDC(serv_test->test_winfo)) ? 2 : 1;

	if (!serv_test->engine_offload) {
		for (band_idx = TEST_DBDC_BAND0;
			band_idx < band_num; band_idx++) {
			ret = mt_serv_init_config(
				&serv_test->test_config[band_idx], band_idx);
			if (ret != SERV_STATUS_SUCCESS)
				return SERV_STATUS_SERV_TEST_FAIL;
		}

		/* Control band0 as default setting */
		serv_test->ctrl_band_idx = TEST_DBDC_BAND0;

		/* Init test mode backup CR data struct */
		sys_ad_zero_mem(&serv_test->test_bkcr,
			sizeof(struct test_bk_cr) * TEST_MAX_BKCR_NUM);

		/* Init test mode rx statistic data struct */
		sys_ad_zero_mem(serv_test->test_rx_statistic,
			sizeof(struct test_rx_stat) * TEST_DBDC_BAND_NUM);

		/* Init test mode rx statistic data struct */
		sys_ad_zero_mem(&serv_test->test_bstat,
			sizeof(struct test_band_state));
	}

	ret = mt_serv_init_op(serv_test->test_op);

	/* Init test mode control register data struct */
	sys_ad_zero_mem(&serv_test->test_reg,
			sizeof(struct test_register));

	/* Init test mode eeprom data struct */
	sys_ad_zero_mem(&serv_test->test_eprm,
			sizeof(struct test_eeprom));

	/* TODO: factor out here */
	/* Common Part */
	/* Init test log dump data struct */
	sys_ad_zero_mem(&serv_test->test_log_dump,
			sizeof(struct test_log_dump_cb) * TEST_LOG_TYPE_NUM);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

/* For test mode exit of service.git */
s_int32 mt_serv_exit_test(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char band_idx;

	if (!serv_test->engine_offload) {
		for (band_idx = TEST_DBDC_BAND0;
			band_idx < TEST_DBDC_BAND_NUM; band_idx++) {
			ret = mt_serv_release_config(
				&serv_test->test_config[band_idx], band_idx);
			if (ret != SERV_STATUS_SUCCESS)
				return SERV_STATUS_SERV_TEST_FAIL;
		}
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_start(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (!serv_test->engine_offload) {
		ret = mt_engine_start(serv_test->test_winfo,
					&serv_test->test_backup,
					serv_test->test_config,
					serv_test->test_op,
					serv_test->test_bkcr,
					&serv_test->test_rx_statistic[0],
					serv_test->en_log);
	} else {
		ret = serv_test->test_op->op_set_test_mode_start(
			serv_test->test_winfo);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_stop(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (!serv_test->engine_offload) {
		ret = mt_engine_stop(serv_test->test_winfo,
					&serv_test->test_backup,
					serv_test->test_config,
					serv_test->test_op,
					serv_test->test_bkcr,
					serv_test->test_log_dump);
	} else {
		ret = serv_test->test_op->op_set_test_mode_abort(
			serv_test->test_winfo);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_channel(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct test_operation *ops = serv_test->test_op;
	struct serv_chip_cap *cap = &winfos->chip_cap;
	s_int32 ant_loop;
	u_char tx_ant_mask = 0, rx_ant_mask = 0;
	u_int32 tx_stream_num = 0, max_stream_num = 0;
	s_int8 ch_offset = 0;
#if 0
	u_char tmp = 0;
#endif
	u_char pri_sel = 0, channel = 0, channel_2nd = 0;
	const s_int8 bw40_sel[] = { -2, 2};
	const s_int8 bw80_sel[] = { -6, -2, 2, 6};
	const s_int8 bw160_sel[] = { -14, -10, -6, -2, 2, 6, 10, 14};

	configs = &serv_test->test_config[ctrl_band_idx];

	/* update max stream num cap */
	max_stream_num = max(GET_MAX_PATH(cap, ctrl_band_idx, 0),
							GET_MAX_PATH(cap, ctrl_band_idx, 1));

	for (ant_loop = 0; ant_loop < max_stream_num; ant_loop++) {
		if (configs->tx_ant & (0x1 << ant_loop))
			tx_ant_mask |= (0x1 << ant_loop);
	}

	/* update tx anteena config */
	configs->tx_ant = tx_ant_mask;

	/*
	 * To get TX max stream number from TX antenna bit mask
	 * tx_sel=2 -> tx_stream_num=2
	 * tx_sel=4 -> tx_stream_num=3
	 * tx_sel=8 -> tx_stream_num=4
	 */

	/*
	 * tx stream for arbitrary tx ant bitmap
	 * (ex: tx_sel=5 -> tx_stream_num=3, not 2)
	 */
	for (ant_loop = max_stream_num; ant_loop > 0; ant_loop--) {
		if (tx_ant_mask & BIT(ant_loop - 1)) {
			tx_stream_num = ant_loop;
			break;
		}
	}

	/* tx stream parameter sanity protection */
	tx_stream_num = tx_stream_num ? tx_stream_num : 1;
	tx_stream_num = (tx_stream_num <= max_stream_num)
		? tx_stream_num : max_stream_num;

	/* update tx stream num config */
	configs->tx_strm_num = tx_stream_num;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: tx_ant:0x%x, tx stream:%u\n",
		__func__, configs->tx_ant, configs->tx_strm_num));

	for (ant_loop = 0; ant_loop < max_stream_num; ant_loop++) {
		if (configs->rx_ant & (0x1 << ant_loop))
			rx_ant_mask |= (0x1 << ant_loop);
	}

	/* fw need parameter rx stream path */
	configs->rx_ant = rx_ant_mask;
	configs->rx_strm_pth = rx_ant_mask;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: rx_ant:0x%x, rx path:0x%x\n", __func__,
			configs->rx_ant, configs->rx_strm_pth));

	/* read test config info */
	pri_sel = configs->pri_sel;
	channel = configs->channel;
	channel_2nd = configs->channel_2nd;

	switch (configs->bw) {
	case TEST_BW_20:
		configs->ctrl_ch = channel;
		if (configs->per_pkt_bw > configs->bw) {
			switch (configs->per_pkt_bw) {
			case TEST_BW_80:
				if (pri_sel >= 4)
					goto error;

				ch_offset = bw80_sel[pri_sel];
				break;
			case TEST_BW_160C:
			case TEST_BW_160NC:
				if (pri_sel >= 8)
					goto error;

				ch_offset = bw160_sel[pri_sel];
				break;
			default: /* BW_40 */
				if (pri_sel > 1)
					goto error;

				ch_offset = bw40_sel[pri_sel];
			}
		}

		break;
	case TEST_BW_40:
		if (pri_sel >= 2)
			goto error;

		configs->ctrl_ch = channel + bw40_sel[pri_sel];
		ch_offset = bw40_sel[pri_sel];

		break;

	case TEST_BW_160NC:
		if (pri_sel >= 8)
			goto error;

		if (!channel_2nd)
			goto error2;

#if 0
		/* swap control channel to be in order */
		if (channel_2nd < channel) {
			tmp = channel;
			channel = channel_2nd;
			channel_2nd = tmp;
		}
#endif
		/* TODO: bw80+80 primary select definition */
		if (pri_sel < 4) {
			configs->ctrl_ch = channel + bw80_sel[pri_sel];
			ch_offset = bw80_sel[pri_sel];
		} else {
			configs->ctrl_ch = channel + bw80_sel[pri_sel - 4];
			ch_offset = bw80_sel[pri_sel - 4];
		}

		break;

	case TEST_BW_80:
		if (pri_sel >= 4)
			goto error;

		configs->ctrl_ch = channel + bw80_sel[pri_sel];
		ch_offset = bw80_sel[pri_sel];

		break;

	case TEST_BW_160C:
		if (pri_sel >= 8)
			goto error;

		configs->ctrl_ch = channel + bw160_sel[pri_sel];
		ch_offset = bw160_sel[pri_sel];

		break;

	default:
		goto error3;
	}

	/* sanity check for channel parameter */
	if (((channel + ch_offset) <= 0) ||
		((channel - ch_offset) <= 0))
		goto error;

	/* update test config info */
	configs->pri_sel = pri_sel;
	configs->ch_offset = ch_offset;
	configs->channel = channel;
	configs->channel_2nd = channel_2nd;

	/* set channel */
	ret = ops->op_set_channel(winfos, ctrl_band_idx, configs);
	if (ret)
		goto error;

	return ret;

error:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: set channel fail, ", __func__));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("control channel: %d|%d\n", configs->ctrl_ch,
		channel - ch_offset));
	return SERV_STATUS_OSAL_NET_FAIL_SET_CHANNEL;

error2:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: set channel fail, ", __func__));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("second control channel is 0 for bw 80+80\n"));
	return SERV_STATUS_OSAL_NET_FAIL_SET_CHANNEL;

error3:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: set channel fail, ", __func__));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("bw=%d is invalid\n", configs->bw));
	return SERV_STATUS_OSAL_NET_FAIL_SET_CHANNEL;
}

s_int32 mt_serv_set_tx_content(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_set_tx_content(
		serv_test->test_winfo,
		ctrl_band_idx,
		configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_tx_path(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_set_tx_path(
		serv_test->test_winfo,
		ctrl_band_idx,
		configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_rx_path(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_set_rx_path(
		serv_test->test_winfo,
		ctrl_band_idx,
		configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_submit_tx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_operation *ops = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	void *virtual_device = NULL;

	configs = &serv_test->test_config[ctrl_band_idx];

	if (!serv_test->engine_offload) {
		ret = net_ad_get_virtual_dev(winfos,
			     		     ctrl_band_idx,
			configs->tx_method[configs->tx_mode],
			     		   &virtual_device);

		if (ret)
			goto err_out;

		ret = mt_engine_subscribe_tx(ops, winfos,
					     virtual_device,
					     configs, ctrl_band_idx);
		if (ret)
			goto err_out;
	} else {
		/* TBD */
	}

err_out:
	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%04x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_revert_tx(struct service_test *serv_test)
{
	u_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	configs = &serv_test->test_config[ctrl_band_idx];

	if (!serv_test->engine_offload) {
		ret = mt_engine_unsubscribe_tx(winfos, configs);
		if (ret)
			goto err_out;
	} else {
		/* TBD */
	}

err_out:
	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_start_tx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	if (configs->mcs == 32
		&& configs->per_pkt_bw != TEST_BW_40
		&& configs->bw != TEST_BW_40
		&& configs->tx_mode < TEST_MODE_HE_SU) {
		ret = SERV_STATUS_SERV_TEST_INVALID_PARAM;
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: bandwidth must to be 40MHz,\n", __func__));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("\t\t due to MCS32 imply 40MHz MCS0\n"));
		goto err_out;
	}

	if (!serv_test->engine_offload) {
		ret = mt_engine_calc_ipg_param_by_ipg(
				&serv_test->test_config[ctrl_band_idx]);
		if (ret)
			goto err_out;

		ret = mt_engine_start_tx(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					serv_test->test_op, ctrl_band_idx);
	} else {
		ret = serv_test->test_op->op_start_tx(
			serv_test->test_winfo,
			ctrl_band_idx,
			&serv_test->test_config[ctrl_band_idx]);
	}

err_out:
	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_stop_tx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	if (!serv_test->engine_offload) {
		ret = mt_engine_stop_tx(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					serv_test->test_op, ctrl_band_idx);
	} else {
		ret = serv_test->test_op->op_stop_tx(
			serv_test->test_winfo,
			ctrl_band_idx);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_start_rx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct serv_chip_cap *cap = &winfos->chip_cap;
	struct test_configuration *configs;
	s_int32 ant_loop;
	u_char ant_mask = 0;
	u_int32 max_stream_num = 0;

	configs = &serv_test->test_config[ctrl_band_idx];

	if (!serv_test->engine_offload) {
		/* update max stream num cap */
		max_stream_num = max(GET_MAX_PATH(cap, ctrl_band_idx, 0),
								GET_MAX_PATH(cap, ctrl_band_idx, 1));

		for (ant_loop = 0; ant_loop < max_stream_num; ant_loop++) {
			if (configs->rx_ant & (0x1 << ant_loop))
				ant_mask |= (0x1 << ant_loop);
		}

		/* fw need parameter rx stream path */
		configs->rx_ant = ant_mask;
		configs->rx_strm_pth = ant_mask;

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: rx_ant:0x%x, rx path:0x%x\n", __func__,
			configs->rx_ant, configs->rx_strm_pth));

		ret = mt_engine_start_rx(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					serv_test->test_op, ctrl_band_idx);

		/* he_tb rx enhance support */
		if (configs->tx_mode == TEST_MODE_HE_TB) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				("%s: \x1b[32m Rx HE TB mode, run start tx\x1b[0m\n", __func__));

			configs->tx_mode = TEST_MODE_HE_SU;

			ret = mt_serv_submit_tx(serv_test);
			if (ret) {
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("%s: submit tx err code:%u\n", __func__, ret));
				return ret;
			}

			/* Start packet Tx */
			ret = mt_serv_start_tx(serv_test);
			if (ret) {
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("%s: start tx err code:%u\n", __func__, ret));
				return ret;
			}
			configs->tx_mode = TEST_MODE_HE_TB;
		}
	} else {
		ret = serv_test->test_op->op_start_rx(
			serv_test->test_winfo,
			ctrl_band_idx,
			configs);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_stop_rx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	/* he_tb rx enhance support */
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	if (!serv_test->engine_offload) {
		ret = mt_engine_stop_rx(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					serv_test->test_op, ctrl_band_idx);

		/* he_tb rx enhance support */
		if (configs->tx_mode == TEST_MODE_HE_TB) {
			ret = mt_serv_stop_tx(serv_test);
			if (ret)
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("%s: stop tx err code:%u\n", __func__, ret));

			ret = mt_serv_revert_tx(serv_test);
			if (ret)
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("%s: revert tx err code:%u\n", __func__, ret));
		}
	} else {
		ret = serv_test->test_op->op_stop_rx(
			serv_test->test_winfo,
			ctrl_band_idx);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_group_prek_clean(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_group_prek(
				serv_test->test_winfo,
				PREK_GROUP_CLEAN);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_group_prek_dump(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_group_prek(
				serv_test->test_winfo,
				PREK_GROUP_DUMP);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_group_prek(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_group_prek(
				serv_test->test_winfo,
				PREK_GROUP_PROC);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dpd_prek_clean(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_CLEAN);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dpd_prek_dump(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_DUMP);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dpd_prek_6g(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_6G_PROC);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dpd_prek_5g(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_5G_PROC);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dpd_prek_2g(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_2G_PROC);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

/*
 * [ 2020.08.11 ] :
 * For single band customer, they might only want to calibrate
 * 2.4G or 5G only, then it will cause driver thinkd DPD not in
 * Pre-K satae !
 * Then, re-modify pre-k command as DPDREK and it will pre-k 5G + 2.4G
 */
s_int32 mt_serv_dpd_prek(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = serv_test->test_op->op_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_5G_PROC);

	ret = serv_test->test_op->op_dpd_prek(
				serv_test->test_winfo,
				PREK_DPD_2G_PROC);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_freq_offset(
	struct service_test *serv_test, u_int32 band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = band_idx;
	struct test_configuration *configs;
	struct test_operation *ops = serv_test->test_op;
	u_int32 rf_freq_offset;

	configs = &serv_test->test_config[ctrl_band_idx];

	rf_freq_offset = configs->rf_freq_offset;

	ret = ops->op_set_freq_offset(
			serv_test->test_winfo,
			rf_freq_offset,
			ctrl_band_idx);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_tx_power_operation(
	struct service_test *serv_test, u_int32 item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct test_operation *ops = serv_test->test_op;
	struct test_txpwr_param *pwr_param = NULL;

	configs = &serv_test->test_config[ctrl_band_idx];
	if (!configs)
		return SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	pwr_param = &configs->pwr_param;
	if (!pwr_param)
		return SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	if (pwr_param->ant_idx >= TEST_ANT_NUM)
		goto error;

	switch (item) {
	case SERV_TEST_TXPWR_SET_PWR:
		configs->tx_pwr[pwr_param->ant_idx] = pwr_param->power;
		ret = ops->op_set_tx_pwr(
			winfos, configs, ctrl_band_idx, pwr_param);
		break;

	case SERV_TEST_TXPWR_GET_PWR:
		ret = ops->op_get_tx_pwr(
				winfos, configs, ctrl_band_idx,
				configs->channel, (u_char)pwr_param->ant_idx,
				&(pwr_param->power));
		break;

	case SERV_TEST_TXPWR_SET_PWR_INIT:
		/* TODO: */
		break;

	case SERV_TEST_TXPWR_SET_PWR_MAN:
		/* TODO: */
		break;

	default:
		return SERV_STATUS_SERV_TEST_INVALID_PARAM;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;

error:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: invalid parameter for ant_idx(0x%x).\n",
		__func__, pwr_param->ant_idx));
	return SERV_STATUS_SERV_TEST_INVALID_PARAM;
}

s_int32 mt_serv_get_freq_offset(
	struct service_test *serv_test, u_int32 *freq_offset,
	u_int32 band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_freq_offset(
			serv_test->test_winfo,
			band_idx,
			freq_offset);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_cfg_on_off(
	struct service_test *serv_test,
	u_int32 band_idx,
	u_int32 type,
	u_int32 *result)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = band_idx;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_cfg_on_off(
			serv_test->test_winfo,
			ctrl_band_idx,
			type,
			result);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_tx_tone_pwr(
	struct service_test *serv_test,
	u_int32 ant_idx,
	u_int32 *power)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_tx_tone_pwr(
			serv_test->test_winfo,
			ctrl_band_idx,
			ant_idx,
			power);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_thermal_val(
	struct service_test *serv_test,
	u_char band_idx,
	u_int32 *value)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_operation *ops;

	configs = &serv_test->test_config[ctrl_band_idx];

	ops = serv_test->test_op;
	ret = ops->op_get_thermal_val(
			serv_test->test_winfo,
			configs,
			ctrl_band_idx,
			value);

	/* update config */
	configs->thermal_val = *value;

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_cal_bypass(
	struct service_test *serv_test,
	u_int32 cal_item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_set_cal_bypass(
			serv_test->test_winfo,
			ctrl_band_idx,
			cal_item);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_dpd(
	struct service_test *serv_test,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_set_dpd(
			serv_test->test_winfo,
			on_off,
			wf_sel);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_tssi(
	struct service_test *serv_test,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_set_tssi(
			serv_test->test_winfo,
			on_off,
			wf_sel);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_rf_type_capability(
	struct service_test *serv_test,
	u_int32 band_idx,
	u_int32 *tx_ant,
	u_int32 *rx_ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_wlan_info *winfos;

	if (!serv_test->engine_offload) {
		winfos = serv_test->test_winfo;

		ret = net_ad_get_rf_type_capability(winfos, band_idx, tx_ant, rx_ant);
	} else {
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_rdd_on_off(
	struct service_test *serv_test,
	u_int32 rdd_num,
	u_int32 rdd_sel,
	u_int32 enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_operation *ops;
	struct test_wlan_info *winfos;

	ops = serv_test->test_op;
	winfos = serv_test->test_winfo;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s(): ctrl_band_idx %d, enable %d\n",
		__func__, ctrl_band_idx, enable));

	if (ops->op_set_tr_mac)
		ret = ops->op_set_tr_mac(winfos, SERV_TEST_MAC_RX,
		enable, ctrl_band_idx);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: op_set_tr_mac, err=0x%08x\n", __func__, ret));

	if (ops->op_set_rdd_test)
		ops->op_set_rdd_test(winfos, rdd_num, rdd_sel, enable);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: op_set_rdd_test, err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_off_ch_scan(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	struct test_operation *ops = serv_test->test_op;
	struct test_off_ch_param *param = NULL;

	configs = &serv_test->test_config[ctrl_band_idx];
	if (!configs)
		return SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	param = &configs->off_ch_param;
	if (!param)
		return SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER;

	ret = ops->op_set_off_ch_scan(
		winfos, configs, ctrl_band_idx, param);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}


s_int32 mt_serv_set_icap_start(
	struct service_test *serv_test,
	struct hqa_rbist_cap_start *icap_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	if (op->op_set_icap_start)
		ret = op->op_set_icap_start(winfos, (u_int8 *)icap_info);
	else
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err = 0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_icap_status(
	struct service_test *serv_test,
	s_int32 *icap_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	if (op->op_get_icap_status)
		ret = op->op_get_icap_status(winfos, icap_stat);
	else
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err = 0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_icap_max_data_len(
	struct service_test *serv_test,
	u_long *max_data_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	if (op->op_get_icap_max_data_len)
		ret = op->op_get_icap_max_data_len(winfos, max_data_len);
	else
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err = 0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_icap_data(
	struct service_test *serv_test,
	s_int32 *icap_cnt,
	s_int32 *icap_data,
	u_int32 wf_num,
	u_int32 iq_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;

	if (op->op_get_icap_data)
		ret = op->op_get_icap_data(winfos, icap_cnt
					, icap_data, wf_num, iq_type);
	else
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err = 0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_recal_cnt(
	struct service_test *serv_test,
	u_int32 *recal_cnt,
	u_int32 *recal_dw_num)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_get_recal_cnt(
			serv_test->test_winfo,
			recal_cnt,
			recal_dw_num);

	return ret;
}

s_int32 mt_serv_get_recal_content(
	struct service_test *serv_test,
	u_int32 *content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_get_recal_content(
			serv_test->test_winfo,
			content);

	return ret;
}

s_int32 mt_serv_get_rxv_cnt(
	struct service_test *serv_test,
	u_int32 *rxv_cnt,
	u_int32 *rxv_dw_num)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_wlan_info *winfos;
	struct test_operation *ops;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	u_int32 byte_cnt = 0;

	if (!serv_test->engine_offload) {
		winfos = serv_test->test_winfo;

		/* Note: only support single rxv count report */
		*rxv_cnt = 1;

		/* query rxv byte count */
		ret = net_ad_get_rxv_cnt(winfos, ctrl_band_idx, &byte_cnt);
	} else {
		ops = serv_test->test_op;
		ret = ops->op_get_rxv_cnt(
			serv_test->test_winfo,
			rxv_cnt,
			rxv_dw_num);
	}

	return ret;
}

s_int32 mt_serv_get_rxv_content(
	struct service_test *serv_test,
	u_int32 dw_cnt,
	u_int32 *content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_wlan_info *winfos;
	struct test_operation *ops;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	if (!serv_test->engine_offload) {
		/* query rxv content */
		winfos = serv_test->test_winfo;
		ret = net_ad_get_rxv_content(winfos, ctrl_band_idx, content);
	} else {
		ops = serv_test->test_op;
		ret = ops->op_get_rxv_content(
			serv_test->test_winfo,
			dw_cnt,
			content);
	}

	return ret;
}

s_int32 mt_serv_get_rdd_cnt(
	struct service_test *serv_test,
	u_int32 *rdd_cnt,
	u_int32 *rdd_dw_num)
{
	struct test_operation *ops;
	s_int32 ret = SERV_STATUS_SUCCESS;

	ops = serv_test->test_op;
	ret = ops->op_get_rdd_cnt(
		&serv_test->test_log_dump[TEST_LOG_RDD - 1],
		rdd_cnt,
		rdd_dw_num);

	return ret;
}

s_int32 mt_serv_get_rdd_content(
	struct service_test *serv_test,
	u_int32 *content,
	u_int32 *total_cnt)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_rdd_content(
		&serv_test->test_log_dump[TEST_LOG_RDD - 1],
		content, total_cnt);

	return ret;
}

s_int32 mt_serv_reset_txrx_counter(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_wlan_info *winfos;
	struct test_configuration *config_band0;
	struct test_configuration *config_band1;
	struct test_rx_stat *test_rx_st;
	u_int8 ant_idx = 0, band_idx = 0;

	winfos = serv_test->test_winfo;
	config_band0 = &serv_test->test_config[TEST_DBDC_BAND0];
	config_band1 = &serv_test->test_config[TEST_DBDC_BAND1];

	for (band_idx = TEST_DBDC_BAND0;
		band_idx < TEST_DBDC_BAND_NUM; band_idx++) {
		test_rx_st = serv_test->test_rx_statistic + band_idx;
		sys_ad_zero_mem(test_rx_st,
				sizeof(struct test_rx_stat));

		for (ant_idx = 0; ant_idx < TEST_ANT_NUM; ant_idx++) {
			test_rx_st->rx_st_path[ant_idx].rssi = 0xFF;
			test_rx_st->rx_st_path[ant_idx].rcpi = 0xFF;
			test_rx_st->rx_st_path[ant_idx].fagc_ib_rssi = 0xFF;
			test_rx_st->rx_st_path[ant_idx].fagc_wb_rssi = 0xFF;
		}
	}

	config_band0->tx_stat.tx_done_cnt = 0;
	if (IS_TEST_DBDC(winfos))
		config_band1->tx_stat.tx_done_cnt = 0;

	ret = serv_test->test_op->op_reset_txrx_counter(
			serv_test->test_winfo);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_rx_vector_idx(
	struct service_test *serv_test, u_int32 group1, u_int32 group2)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_set_rx_vector_idx(
		serv_test->test_winfo,
		serv_test->ctrl_band_idx,
		group1,
		group2);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_fagc_rssi_path(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_set_fagc_rssi_path(
		serv_test->test_winfo,
		serv_test->ctrl_band_idx,
		serv_test->test_config[TEST_DBDC_BAND0].fagc_path);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_rx_stat_leg(
	struct service_test *serv_test,
	struct test_rx_stat_leg *rx_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_rx_stat_leg(
		serv_test->test_winfo, rx_stat);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_rxv_dump_action(
	struct service_test *serv_test,
	u_int32 action,
	u_int32 type_mask)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_get_rxv_dump_action(
		serv_test->test_winfo, action, type_mask);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_rxv_dump_ring_attr(
	struct service_test *serv_test,
	struct rxv_dump_ring_attr *ring_attr)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_rxv_dump_ring_attr(
		serv_test->test_winfo, ring_attr);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_rxv_dump_content(
	struct service_test *serv_test,
	u_int8 entry_idx,
	u_int32 *content_len,
	void *rxv_content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;
	ret = ops->op_get_rxv_dump_rxv_content(
		serv_test->test_winfo, entry_idx, content_len, rxv_content);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_rxv_content_len(
	struct service_test *serv_test,
	u_int8 type_idx,
	u_int8 rxv_sta_cnt,
	u_int16 *len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;
	u_int16 len_tmp = 0;

	ops = serv_test->test_op;
	ret = ops->op_get_rxv_content_len(
		serv_test->test_winfo, BIT(type_idx), rxv_sta_cnt, len);

	/* copy rxv content length to temp variable */
	len_tmp = *len;

	switch (type_idx) {
	case TEST_RXV_CONTENT_CMN1:
	case TEST_RXV_CONTENT_CMN2:
		*len += (1 + 1) + (1 + 1) + (1 + 1) + len_tmp;
		break;

	case TEST_RXV_CONTENT_USR1:
	case TEST_RXV_CONTENT_USR2:
		*len += (1 + 1) + (1 + 1);
		*len += TEST_USER_NUM * ((1 + 1) + len_tmp);
		break;

	default:
		ret = SERV_STATUS_AGENT_NOT_SUPPORTED;
		break;
	}

	return ret;
}

s_int32 mt_serv_get_rx_stat(
	struct service_test *serv_test,
	u_int8 band_idx,
	u_int8 blk_idx,
	u_int8 test_rx_stat_cat,
	struct test_rx_stat_u *st)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;
	struct test_wlan_info *winfos;
	struct test_rx_stat *stat;
	boolean dbdc_mode = FALSE;

	ops = serv_test->test_op;
	winfos = serv_test->test_winfo;
	stat = &serv_test->test_rx_statistic[band_idx];

	/* check dbdc mode condition */
	dbdc_mode = IS_TEST_DBDC(serv_test->test_winfo);

	/* sanity check for band index param */
	if ((!dbdc_mode) && (band_idx != TEST_DBDC_BAND0))
		goto error1;

	switch (test_rx_stat_cat) {
	case TEST_RX_STAT_BAND:
		ret = ops->op_get_rx_stat_band(
		serv_test->test_winfo,
		band_idx,
		blk_idx,
		stat->rx_st_band + blk_idx);

		sys_ad_move_mem(st, stat->rx_st_band + blk_idx,
				sizeof(struct test_rx_stat_band_info));
		break;
	case TEST_RX_STAT_PATH:
		ret = ops->op_get_rx_stat_path(
		serv_test->test_winfo,
		band_idx,
		blk_idx,
		stat->rx_st_path + blk_idx);

		sys_ad_move_mem(st, stat->rx_st_path + blk_idx,
				sizeof(struct test_rx_stat_path_info));
		break;
	case TEST_RX_STAT_USER:
		ret = ops->op_get_rx_stat_user(
		serv_test->test_winfo,
		band_idx,
		blk_idx,
		stat->rx_st_user + blk_idx);

		sys_ad_move_mem(st, stat->rx_st_user + blk_idx,
				sizeof(struct test_rx_stat_user_info));
		break;
	case TEST_RX_STAT_COMM:
		ret = ops->op_get_rx_stat_comm(
		serv_test->test_winfo,
		band_idx,
		blk_idx,
		&stat->rx_st_comm);

		sys_ad_move_mem(st, &stat->rx_st_comm,
				sizeof(struct test_rx_stat_comm_info));
		break;
	default:
		break;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: invalid band index for non-dbdc mode.\n",
		__func__));
	return ret;
}

s_int32 mt_serv_calibration_test_mode(
	struct service_test *serv_test, u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfo = serv_test->test_winfo;

	ret = op->op_calibration_test_mode(winfo, mode);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_do_cal_item(
	struct service_test *serv_test, u_int32 item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	if (!serv_test->engine_offload) {
		ret = net_ad_do_cal_item(
			serv_test->test_winfo,
			item, ctrl_band_idx);
	} else {
		ret = serv_test->test_op->op_do_cal_item(
			serv_test->test_winfo,
			item, ctrl_band_idx);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_band_mode(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;

	if (!serv_test->engine_offload) {
		ret = net_ad_set_band_mode(
			serv_test->test_winfo,
			&serv_test->test_bstat);

		if (ret)
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				("%s: err=0x%08x\n", __func__, ret));
	} else {
		ret = ops->op_set_band_mode(
			serv_test->test_winfo,
			&serv_test->test_bstat);
	}

	return ret;
}

s_int32 mt_serv_get_band_mode(
	struct service_test *serv_test, u_int8 band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = band_idx;
	u_int32 band_type = TEST_BAND_TYPE_UNUSE;
	struct test_operation *ops;

	ops = serv_test->test_op;

	if (!serv_test->engine_offload) {
		ret = net_ad_get_band_mode(serv_test->test_winfo,
				band_idx, &band_type);
	} else {
		ret = ops->op_set_band_mode(
			serv_test->test_winfo,
			&serv_test->test_bstat);

		if (ctrl_band_idx == TEST_DBDC_BAND0)
			band_type = TEST_BAND_TYPE_2_4G_5G;
		else {
			if (serv_test->test_bstat.band_mode ==
				TEST_BAND_MODE_DUAL)
				band_type = TEST_BAND_TYPE_2_4G_5G;
			else
				band_type = TEST_BAND_TYPE_UNUSE;
		}
	}

	if ((band_type != TEST_BAND_TYPE_UNUSE) &&
		serv_test->test_winfo->chip_cap.support_6g)
		band_type |= TEST_BAND_TYPE_6G;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: band_idx: %d, band_type=%u\n", __func__,
		band_idx, band_type));

	BSTATE_SET_PARAM(serv_test, band_type, band_type);

	return ret;
}

s_int32 mt_serv_log_on_off(
	struct service_test *serv_test, u_int32 log_type,
	u_int32 log_ctrl, u_int32 log_size)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;
	struct test_log_dump_cb *log_cb = NULL;
	u_int32 mask = 0;
	u_int8 overwrite = TRUE;

	ops = serv_test->test_op;
	log_cb = &serv_test->test_log_dump[log_type-1];

	ret = ops->op_log_on_off(
		serv_test->test_winfo,
		log_cb,
		log_type,
		log_ctrl,
		log_size);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: log_type=0x%08x\n", __func__, log_type));

	switch (log_type) {
	case TEST_LOG_RDD:
		overwrite = FALSE;
		mask = fTEST_LOG_RDD;
		break;

	default:
		goto err0;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: log_ctrl=0x%08x\n", __func__, log_ctrl));

	switch (log_ctrl) {
	case TEST_LOG_ON:
		serv_test->en_log |= mask;
		log_cb->overwritable = overwrite;
		log_cb->is_overwritten = FALSE;
		log_cb->idx = 0;
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: log_cb->idx=0x%08x\n", __func__, log_cb->idx));

		break;

	case TEST_LOG_OFF:
		serv_test->en_log &= ~mask;
		break;

	default:
		goto err0;
	}

	return ret;

err0:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s log type %d not supported\n",
		__func__, log_type));
	return SERV_STATUS_SERV_TEST_INVALID_PARAM;

}

s_int32 mt_serv_set_cfg_on_off(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *configs;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_operation *ops;

	configs = &serv_test->test_config[ctrl_band_idx];

	ops = serv_test->test_op;
	ret = ops->op_set_cfg_on_off(
			serv_test->test_winfo,
			(u_int8)configs->log_type,
			(u_int8)configs->log_enable,
			ctrl_band_idx);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_rx_filter_pkt_len(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *configs;
	struct test_operation *ops;

	configs = &serv_test->test_config[serv_test->ctrl_band_idx];
	ops = serv_test->test_op;
	ret = ops->op_set_rx_filter_pkt_len(
		serv_test->test_winfo,
		configs->rx_filter_en,
		serv_test->ctrl_band_idx,
		configs->rx_filter_pkt_len);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_low_power(
	struct service_test *serv_test, u_int32 control)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_set_low_power(serv_test->test_winfo, control);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_antswap_capability(
	struct service_test *serv_test, u_int32 *antswap_support)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_get_antswap_capability(
			serv_test->test_winfo,
			antswap_support);

	return ret;
}

s_int32 mt_serv_set_antswap(
	struct service_test *serv_test, u_int32 ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_set_antswap(
			serv_test->test_winfo,
			ant);

	return ret;
}

s_int32 mt_serv_set_eeprom_to_fw(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	struct test_operation *ops;

	ops = serv_test->test_op;

	ret = ops->op_set_eeprom_to_fw(
			serv_test->test_winfo);

	return ret;
}

s_int32 mt_serv_reg_eprm_operation(
	struct service_test *serv_test, u_int32 item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_operation *ops;

	ops = serv_test->test_op;

	switch (item) {
	case SERV_TEST_REG_MAC_READ:
		ret = net_ad_read_mac_bbp_reg(serv_test->test_winfo,
						&serv_test->test_reg);
		break;

	case SERV_TEST_REG_MAC_WRITE:
		if (!serv_test->engine_offload) {

		ret = net_ad_write_mac_bbp_reg(serv_test->test_winfo,
						&serv_test->test_reg);
		} else {
			ops = serv_test->test_op;
			ret = ops->op_write_mac_bbp_reg(serv_test->test_winfo,
				&serv_test->test_reg);
		}
		break;

	case SERV_TEST_REG_MAC_READ_BULK:
		if (!serv_test->engine_offload) {
			ret = net_ad_read_bulk_mac_bbp_reg(
				serv_test->test_winfo,
				&serv_test->test_config[TEST_DBDC_BAND0],
				&serv_test->test_reg);
		} else {
			ops = serv_test->test_op;
			ret = ops->op_read_bulk_mac_bbp_reg(
				serv_test->test_winfo,
				&serv_test->test_reg);
		}
		break;

	case SERV_TEST_REG_RF_READ_BULK:
		if (!serv_test->engine_offload) {
		ret = net_ad_read_bulk_rf_reg(serv_test->test_winfo,
			&serv_test->test_reg);
		} else {
			ops = serv_test->test_op;
			ret = ops->op_read_bulk_rf_reg(serv_test->test_winfo,
				&serv_test->test_reg);
		}
		break;

	case SERV_TEST_REG_RF_WRITE_BULK:
		if (!serv_test->engine_offload) {
		ret = net_ad_write_bulk_rf_reg(serv_test->test_winfo,
			&serv_test->test_reg);
		} else {
			ops = serv_test->test_op;
			ret = ops->op_write_bulk_rf_reg(serv_test->test_winfo,
				&serv_test->test_reg);
		}
		break;

	case SERV_TEST_REG_CA53_READ:
		net_ad_read_ca53_reg(&serv_test->test_reg);
		break;

	case SERV_TEST_REG_CA53_WRITE:
		net_ad_write_ca53_reg(&serv_test->test_reg);
		break;

	case SERV_TEST_EEPROM_READ:
		ret = net_ad_read_write_eeprom(serv_test->test_winfo,
			&serv_test->test_eprm,
			TRUE);
		break;

	case SERV_TEST_EEPROM_WRITE:
		ret = net_ad_read_write_eeprom(serv_test->test_winfo,
			&serv_test->test_eprm,
			FALSE);
		break;

	case SERV_TEST_EEPROM_READ_BULK:
		if (!serv_test->engine_offload) {
		ret = net_ad_read_write_bulk_eeprom(serv_test->test_winfo,
			&serv_test->test_eprm,
			TRUE);
		} else {
			ops = serv_test->test_op;
			ret = ops->op_read_bulk_eeprom(serv_test->test_winfo,
				&serv_test->test_eprm);
		}
		break;

	case SERV_TEST_EEPROM_WRITE_BULK:
		ret = net_ad_read_write_bulk_eeprom(serv_test->test_winfo,
			&serv_test->test_eprm,
			FALSE);
		break;

	case SERV_TEST_EEPROM_GET_FREE_EFUSE_BLOCK:
		ret = net_ad_get_free_efuse_block(serv_test->test_winfo,
			&serv_test->test_eprm);
		break;

	default:
		return SERV_STATUS_SERV_TEST_INVALID_PARAM;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_mps_operation(
	struct service_test *serv_test, u_int32 item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs = NULL;
	struct test_mps_cb *mps_cb = NULL;

	configs = &serv_test->test_config[ctrl_band_idx];
	mps_cb  = &configs->mps_cb;

	switch (item) {
	case SERV_TEST_MPS_START_TX:
		ret = net_ad_mps_tx_operation(serv_test->test_winfo,
			&serv_test->test_config[ctrl_band_idx],
			TRUE);
		if (ret != SERV_STATUS_SUCCESS)
			break;

		if (!serv_test->engine_offload) {
			ret = mt_serv_submit_tx(serv_test);
			if (ret != SERV_STATUS_SUCCESS)
				break;

			mt_serv_tx_power_operation(serv_test,
					SERV_TEST_TXPWR_SET_PWR);
			ret = mt_serv_start_tx(serv_test);
		}
		break;

	case SERV_TEST_MPS_STOP_TX:
		ret = mt_serv_stop_tx(serv_test);
		if (ret != SERV_STATUS_SUCCESS)
			break;

		ret = mt_serv_revert_tx(serv_test);
		if (ret != SERV_STATUS_SUCCESS)
			break;

		mps_cb->stat = 0;
		if (configs->op_mode & fTEST_MPS) {
			SERV_OS_SEM_LOCK(&mps_cb->lock);
			configs->op_mode &= ~fTEST_MPS;
			mps_cb->setting_inuse = FALSE;
			SERV_OS_SEM_UNLOCK(&mps_cb->lock);
			ret = net_ad_mps_tx_operation(serv_test->test_winfo,
				&serv_test->test_config[ctrl_band_idx],
				FALSE);
		}
		break;

	default:
		return SERV_STATUS_SERV_TEST_INVALID_PARAM;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_chipid(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = serv_test->test_op->op_get_chipid(
			serv_test->test_winfo);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_sub_chipid(
	struct service_test *serv_test,
	u_int32 *sub_chipid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = serv_test->test_op->op_get_sub_chipid(
			serv_test->test_winfo, sub_chipid);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_mps_set_seq_data(
	struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *test_config;
	struct test_mps_cb *mps_cb;
	u_int32 len;

	test_config = &serv_test->test_config[ctrl_band_idx];
	mps_cb = &test_config->mps_cb;
	len = mps_cb->mps_cnt;

	if (!serv_test->engine_offload) {

	} else {
		ret = serv_test->test_op->op_mps_set_seq_data(
			serv_test->test_winfo,
			mps_cb->mps_cnt,
			mps_cb->mps_setting);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_tmr(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_set_tmr(serv_test->test_winfo, &serv_test->test_tmr);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_preamble(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	u_char tx_mode = serv_test->test_config[ctrl_band_idx].tx_mode;

	if (!serv_test->engine_offload) {

	} else {
		ret = serv_test->test_op->op_set_preamble(
			serv_test->test_winfo,
			tx_mode);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_system_bw(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	u_char sys_bw = serv_test->test_config[ctrl_band_idx].bw;

	if (!serv_test->engine_offload) {

	} else {
		ret = serv_test->test_op->op_set_system_bw(
			serv_test->test_winfo,
			sys_bw);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_set_per_pkt_bw(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	u_char per_pkt_bw = serv_test->test_config[ctrl_band_idx].per_pkt_bw;

	if (!serv_test->engine_offload) {

	} else {
		ret = serv_test->test_op->op_set_per_pkt_bw(
			serv_test->test_winfo,
			per_pkt_bw);
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dbdc_tx_tone(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_dbdc_tx_tone(
			serv_test->test_winfo,
			ctrl_band_idx,
			configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dbdc_tx_tone_pwr(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_dbdc_tx_tone_pwr(
			serv_test->test_winfo,
			ctrl_band_idx,
			configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dbdc_continuous_tx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs;

	configs = &serv_test->test_config[ctrl_band_idx];

	ret = serv_test->test_op->op_dbdc_continuous_tx(
			serv_test->test_winfo,
			ctrl_band_idx,
			configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_continuous_tx(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *configs;

	/* Get parameter */
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 tx_mode = CONFIG_GET_PARAM(serv_test, tx_mode, ctrl_band_idx);
	u_int32 bw = CONFIG_GET_PARAM(serv_test, bw, ctrl_band_idx);
	u_int32 pri_ch = CONFIG_GET_PARAM(serv_test, ctrl_ch, ctrl_band_idx);
	u_int32 central_ch = CONFIG_GET_PARAM(serv_test, channel, ctrl_band_idx);
	u_int32 rate = CONFIG_GET_PARAM(serv_test, mcs, ctrl_band_idx);
	u_int32 ant_sel = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);
	u_int32 tx_fd_mode = CONFIG_GET_PARAM(serv_test, tx_fd_mode, ctrl_band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s:\nband_idx[%d], tx_mode[%d], bw[%d], priCh[%d], centCh[%d], rate[%d], Ant[%d], txFd[%d]\n",
		__func__, ctrl_band_idx, tx_mode, bw, pri_ch, central_ch, rate, ant_sel, tx_fd_mode));

	/* Set parameter */
	configs = &serv_test->test_config[ctrl_band_idx];
	configs->tx_mode = tx_mode;
	configs->bw = bw;
	configs->pri_sel = pri_ch;
	configs->channel = central_ch;
	configs->rate = rate;
	configs->ant_mask = ant_sel;
	configs->tx_fd_mode = tx_fd_mode;
	/* 3: fix payload is OFDM */
	tx_fd_mode = 3;
	configs->tx_fd_mode = tx_fd_mode;
	configs->tx_tone_en = 1;

	ret = mt_serv_dbdc_continuous_tx(serv_test);

	return ret;
}

s_int32 mt_serv_continuous_tx_stop(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *configs;

	/* Get parameter */
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 tx_mode = 0, bw = 0, pri_ch = 0, central_ch = 0, rate = 0, ant_sel = 0, tx_fd_mode = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s:\nband_idx[%d], tx_mode[%d], bw[%d], priCh[%d], centCh[%d], rate[%d], Ant[%d], txFd[%d]\n",
		__func__, ctrl_band_idx, tx_mode, bw, pri_ch, central_ch, rate, ant_sel, tx_fd_mode));

	/* Set parameter */
	configs = &serv_test->test_config[ctrl_band_idx];
	configs->tx_mode = tx_mode;
	configs->bw = bw;
	configs->pri_sel = pri_ch;
	configs->channel = central_ch;
	configs->rate = rate;
	configs->ant_mask = ant_sel;
	configs->tx_fd_mode = tx_fd_mode;
	configs->tx_tone_en = 0;

	ret = mt_serv_dbdc_continuous_tx(serv_test);

	return ret;
}

s_int32 mt_serv_check_txv(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;
	struct test_configuration *configs = serv_test->test_config;
	struct test_tx_stack *stack = &configs[ctrl_band_idx].stack;

	if (!serv_test->engine_offload) {
		return net_ad_check_txv(serv_test->test_winfo, ctrl_band_idx,
				configs, stack->virtual_wtbl[0]);
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: no supoort\n", __func__));
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_tx_info(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_configs_band0;
	struct test_configuration *test_configs_band1;

	test_configs_band0 = &serv_test->test_config[TEST_DBDC_BAND0];
	test_configs_band1 = &serv_test->test_config[TEST_DBDC_BAND1];

	ret = serv_test->test_op->op_get_tx_info(
		serv_test->test_winfo,
		test_configs_band0,
		test_configs_band1);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_thermal_value(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_configs;

	test_configs = &serv_test->test_config[TEST_DBDC_BAND0];

	ret = serv_test->test_op->op_get_thermal_value(
			serv_test->test_winfo,
			test_configs);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_get_wf_path_comb(
	struct service_test *serv_test,
	u_int8 band_idx,
	boolean dbdc_mode_en,
	u_int8 *path,
	u_int8 *path_len
)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_configs;

	test_configs = &serv_test->test_config[TEST_DBDC_BAND0];

	ret = serv_test->test_op->op_get_wf_path_comb(
			serv_test->test_winfo,
			band_idx,
			dbdc_mode_en,
			path,
			path_len);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dnlk_clean(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = serv_test->test_op->op_set_test_mode_dnlk_clean(
		serv_test->test_winfo);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_dnlk_2g(struct service_test *serv_test)
{
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	return net_ad_set_test_mode_dnlk(serv_test->test_winfo, ctrl_band_idx);
}

s_int32 mt_serv_dnlk_5g(struct service_test *serv_test)
{
	u_char ctrl_band_idx = serv_test->ctrl_band_idx;

	return net_ad_set_test_mode_dnlk(serv_test->test_winfo, ctrl_band_idx);
}

s_int32 mt_serv_rxgaink(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = serv_test->test_op->op_set_test_mode_rxgaink(
		serv_test->test_winfo,
		serv_test->ctrl_band_idx);

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

#ifdef TXBF_SUPPORT
s_int32 mt_serv_set_ibf_phase_cal_e2p_update(
	struct service_test *serv_test,
	u_char group_idx,
	boolean  fgSx2,
	u_char update_type)
{
	struct test_operation *op = serv_test->test_op;
	struct test_wlan_info *winfos = serv_test->test_winfo;
	s_int32 ret;


	if (op->op_set_ibf_phase_cal_e2p_update)
		ret = op->op_set_ibf_phase_cal_e2p_update(
							winfos,
							group_idx,
							fgSx2,
							update_type);
	else
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

	return ret;
}


s_int32 mt_serv_set_txbf_lna_gain(
	struct service_test *serv_test,
	u_char lna_gain)
{
	return net_ad_set_txbf_lna_gain(serv_test->test_winfo, lna_gain);
}


s_int32 mt_serv_set_device_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char *addr,
	u_char band_idx,
	u_char mode)
{
	return net_ad_set_device_info(winfos, configs, addr, band_idx, mode);
}


s_int32 mt_serv_set_txbf_sa(
	struct service_test *serv_test, u_char *addr)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);


	return mt_serv_set_device_info(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					addr,
					ctrl_band_idx, 1);
}


s_int32 mt_serv_set_bss_info(
	struct service_test *serv_test, u_char *bssid)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);


	return net_ad_set_bss_info(serv_test->test_winfo,
					&serv_test->test_config[ctrl_band_idx],
					ctrl_band_idx, bssid);
}


s_int32 mt_serv_set_txbf_tx_apply(
	struct service_test *serv_test, u_char *buf)
{
	return mt_ad_set_txbf_tx_apply(serv_test->test_winfo, buf);
}


s_int32 mt_serv_set_ibf_phase_comp(
	struct service_test *serv_test,
	u_char bw,
	boolean fg_jp_band,
	u_char dbdc_band_idx,
	u_char group_idx,
	boolean fg_read_from_e2p,
	boolean fg_dis_comp)
{
	return net_ad_set_ibf_phase_comp(
				serv_test->test_winfo,
				bw,
				fg_jp_band,
				dbdc_band_idx,
				group_idx,
				fg_read_from_e2p,
				fg_dis_comp);
}


s_int32 mt_serv_set_txbf_profile_tag_read(
	struct service_test *serv_test, u_char  pf_idx, boolean fg_bfer)
{
	return net_ad_set_txbf_profile_tag_read(
				serv_test->test_winfo,
				pf_idx,
				fg_bfer);
}


s_int32 mt_serv_set_txbf_profile_tag_write(
	struct service_test *serv_test, u_char prf_idx)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_wite_txbf_pfmu_tag)
		return op->op_set_wite_txbf_pfmu_tag(
				serv_test->test_winfo,
				prf_idx);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_profile_tag_invalid(
	struct service_test *serv_test, boolean fg_invalid)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_invalid)
		return op->op_set_txbf_pfmu_tag_invalid(
				serv_test->test_winfo,
				fg_invalid);
	else
		return SERV_STATUS_AGENT_FAIL;

}


s_int32 mt_serv_set_txbf_profile_tag_idx(
	struct service_test *serv_test, u_char profile_idx)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_idx)
		return op->op_set_txbf_pfmu_tag_idx(
				serv_test->test_winfo,
				profile_idx);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_bf_type(
	struct service_test *serv_test, u_char bf_type)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_idx)
		return op->op_set_txbf_pfmu_tag_bf_type(
				serv_test->test_winfo,
				bf_type);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_dbw(
	struct service_test *serv_test, u_char dbw)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_dbw)
		return op->op_set_txbf_pfmu_tag_dbw(
				serv_test->test_winfo,
				dbw);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_sumu(
	struct service_test *serv_test, u_char su_mu)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_sumu)
		return op->op_set_txbf_pfmu_tag_sumu(
				serv_test->test_winfo,
				su_mu);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_get_wrap_ibf_cal_ibf_mem_alloc(
	struct service_test *serv_test,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	struct test_operation *op = serv_test->test_op;

	if (op->op_set_wrap_ibf_cal_get_ibf_mem_alloc)
		return op->op_set_wrap_ibf_cal_get_ibf_mem_alloc(
				serv_test->test_winfo,
				pfmu_mem_row,
				pfmu_mem_col);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_get_wrap_ibf_cal_ebf_mem_alloc(
	struct service_test *serv_test,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	struct test_operation *op = serv_test->test_op;

	if (op->op_set_wrap_ibf_cal_get_ebf_mem_alloc)
		return op->op_set_wrap_ibf_cal_get_ebf_mem_alloc(
				serv_test->test_winfo,
				pfmu_mem_row,
				pfmu_mem_col);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_mem(
	struct service_test *serv_test,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_mem)
		return op->op_set_txbf_pfmu_tag_mem(
				serv_test->test_winfo,
				pfmu_mem_row,
				pfmu_mem_col);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_matrix(
	struct service_test *serv_test,
	u_char nr,
	u_char nc,
	u_char ng,
	u_char lm,
	u_char cb,
	u_char he)
{
	struct test_operation *op = serv_test->test_op;

	if (op->op_set_txbf_pfmu_tag_matrix)
		return  op->op_set_txbf_pfmu_tag_matrix(
				serv_test->test_winfo,
				nr, nc, ng, lm, cb, he);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_snr(
	struct service_test *serv_test, u_char *buf)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_snr)
		return op->op_set_txbf_pfmu_tag_snr(
				serv_test->test_winfo, buf);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_smart_ant(
	struct service_test *serv_test, u_int32 smart_ant)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_smart_ant)
		return op->op_set_txbf_pfmu_tag_smart_ant(
				serv_test->test_winfo, smart_ant);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_se_idx(
	struct service_test *serv_test, u_int32 se_idx)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_se_idx)
		return op->op_set_txbf_pfmu_tag_se_idx(
				serv_test->test_winfo, se_idx);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_rmsd_thrd(
	struct service_test *serv_test, u_char rmsd_thrd)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_rmsd_thrd)
		return op->op_set_txbf_pfmu_tag_rmsd_thrd(
				serv_test->test_winfo, rmsd_thrd);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_mcs_thrd(
	struct service_test *serv_test, u_char *mcs_lss, u_char *mcs_sss)
{
	return net_ad_set_txbf_profile_tag_mcs_thrd(
				serv_test->test_winfo, mcs_lss, mcs_sss);
}


s_int32 mt_serv_set_txbf_pfmu_tag_time_out(
	struct service_test *serv_test, u_char time_out)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_time_out)
		return op->op_set_txbf_pfmu_tag_time_out(
				serv_test->test_winfo, time_out);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_desired_bw(
	struct service_test *serv_test, u_char desired_bw)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_desired_bw)
		return op->op_set_txbf_pfmu_tag_desired_bw(
				serv_test->test_winfo, desired_bw);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_desired_nr(
	struct service_test *serv_test, u_char desired_nr)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_desired_nr)
		return op->op_set_txbf_pfmu_tag_desired_nr(
				serv_test->test_winfo, desired_nr);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_tag_desired_nc(
	struct service_test *serv_test, u_char desired_nc)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_tag_desired_nc)
		return op->op_set_txbf_pfmu_tag_desired_nc(
				serv_test->test_winfo, desired_nc);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_txbf_pfmu_data_write(
	struct service_test *serv_test, u_int16 *buf)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_txbf_pfmu_data_write)
		return op->op_set_txbf_pfmu_data_write(
				serv_test->test_winfo, buf);
	else
		return SERV_STATUS_AGENT_FAIL;
}


s_int32 mt_serv_set_sta_rec_bf_update(
	struct service_test *serv_test, u_char *arg)
{
	return net_ad_set_sta_rec_bf_update(serv_test->test_winfo, arg);
}


s_int32 mt_serv_set_sta_rec_bf_read(
	struct service_test *serv_test, u_char *arg)
{
	return net_ad_set_sta_rec_bf_read(serv_test->test_winfo, arg);
}


s_int32 mt_serv_set_manual_assoc(
	struct service_test *serv_test, u_char *arg)
{
	return mt_op_set_manual_assoc(serv_test->test_winfo, arg);
}


s_int32 mt_serv_set_tx_pkt_with_ibf(
	struct service_test *serv_test,
	u_char wlan_idx,
	u_int32 tx_cnt,
	boolean fg_bf,
	boolean fg_tx_param_update)
{
	u_char ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_char arg[5];
	s_int32 ret = SERV_STATUS_SUCCESS;


	/* Assign Wlan ID for fixed rate TxD */
	CONFIG_SET_PARAM(serv_test, wcid_ref, (u_int8)wlan_idx, ctrl_band_idx);

	/* At TxD, enable/disable BF Tx at DW6 bit28 */
	if (fg_bf) {
		/* Stop Rx before ready to Tx */
		mt_serv_stop_rx(serv_test);

		/* Valid iBF profile */
		mt_serv_set_txbf_profile_tag_read(serv_test, 2, TRUE);
		mt_serv_set_txbf_profile_tag_invalid(serv_test, FALSE);
		mt_serv_set_txbf_profile_tag_write(serv_test, 2);

		if (fg_tx_param_update) {
			/* Revert wlan service configuration for */
				/* parameter update */
			/* mt_serv_revert_tx(serv_test); */
		}

		/* ATECtrl->eTxBf = TRUE; */
		/* ATECtrl->iTxBf = TRUE; */
		CONFIG_SET_PARAM(serv_test, ebf, TRUE, ctrl_band_idx);
		CONFIG_SET_PARAM(serv_test, ibf, TRUE, ctrl_band_idx);
		CONFIG_SET_PARAM(serv_test,
				fgEBfEverEnabled,
				TRUE,
				ctrl_band_idx);

		/* Stop Tx when the action of Tx packet is done */
		mt_serv_stop_tx(serv_test);

		/* Set the number of Tx packets */
		if (tx_cnt == 0)
			tx_cnt = 0xFFFFFFFF;
		CONFIG_SET_PARAM(serv_test,
				tx_stat.tx_cnt,
				tx_cnt,
				ctrl_band_idx);

		if (fg_tx_param_update) {
			arg[0] = 1; /* WCID */
			arg[1] = 0; /* Disable eBF */
			arg[2] = 1; /* Enable iBF */
			arg[3] = 0; /* Disable MU */
			arg[4] = 1; /* Force iBF enable no mater what */
					/* iBF is enable at UI */
			mt_serv_set_txbf_tx_apply(serv_test, arg);

			/* Tx commit for setting wlan service configuration */
			mt_serv_submit_tx(serv_test);
		}

		/* Start packet Tx */
		mt_serv_start_tx(serv_test);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
				("%s : BF Tx!!!!!\n", __func__));
	} else {
		if (CONFIG_GET_PARAM(serv_test,
				fgEBfEverEnabled, ctrl_band_idx) == FALSE) {
			/* Stop Rx before ready to Tx */
			mt_serv_stop_rx(serv_test);

			/* ATECtrl->eTxBf = FALSE; */
			/* ATECtrl->iTxBf = FALSE; */
			CONFIG_SET_PARAM(serv_test, ebf, FALSE, ctrl_band_idx);
			CONFIG_SET_PARAM(serv_test, ibf, FALSE, ctrl_band_idx);

			if (fg_tx_param_update) {
				/* Revert wlan service configuration for */
					/* parameter update */
				/* mt_serv_revert_tx(serv_test); */
			}

			/* Stop Tx when the action of Tx packet is done */
			mt_serv_stop_tx(serv_test);

			/* Set the number of Tx packets */
			if (tx_cnt == 0)
				tx_cnt = 0xFFFFFFFF;
			CONFIG_SET_PARAM(serv_test,
					tx_stat.tx_cnt,
					tx_cnt,
					ctrl_band_idx);

			if (fg_tx_param_update) {
				arg[0] = 1; /* WCID */
				arg[1] = 0; /* Disable eBF */
				arg[2] = 0; /* Disable iBF */
				arg[3] = 0; /* Disable MU */
				arg[4] = 0; /* Force iBF disable no mater */
					/* what iBF is enable at UI */
				mt_serv_set_txbf_tx_apply(serv_test, arg);

				/* Tx commit for setting wlan service */
				/* configuration */
				mt_serv_submit_tx(serv_test);
			}

			/* Start packet Tx */
			mt_serv_start_tx(serv_test);

			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
					("%s : None BF Tx!!!!!\n", __func__));
		} else {
			/* Invalid iBF profile */
			mt_serv_set_txbf_profile_tag_read(serv_test, 2, TRUE);
			mt_serv_set_txbf_profile_tag_invalid(serv_test, TRUE);
			mt_serv_set_txbf_profile_tag_write(serv_test, 2);
			CONFIG_SET_PARAM(serv_test,
					fgEBfEverEnabled,
					FALSE,
					ctrl_band_idx);

			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			("%s : fgEBfEverEnabled is enabled and None BF Tx!\n",
			__func__));
		}
	}

	return ret;
}


s_int32 mt_serv_set_ibf_profile_update(
	struct service_test *serv_test, u_char pfmu_idx, u_char nc)
{
	u_char ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	s_int32 tmplen;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char wcid;
	u_char nr, ndp_nss = 0, pfmu_mem_row[4] = {0}, pfmu_mem_col[4] = {0};
	u_char tx_ant_cfg, *addr1 = NULL;
	u_char cmd_str[80], buf[6];


	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s : band[%d]'s TxAntennaSel = 0x%x\n",
		__func__, ctrl_band_idx,
		CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx)));

	tx_ant_cfg = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);

	switch (tx_ant_cfg) {
	case 3:
		nr = 1;
		break;
	case 7:
		nr = 2;
		break;
	case 12:
		nr = 3;
		break;
	case 15:
		nr = 3;
		break;
	default:
		nr = 3;
		break;
	}

	/* Configure iBF tag */
	/* PFMU ID */
	mt_serv_set_txbf_profile_tag_idx(serv_test, pfmu_idx);

	/* ITxBf */
	mt_serv_set_txbf_pfmu_tag_bf_type(serv_test, 0);

	/* BW20 */
	mt_serv_set_txbf_pfmu_tag_dbw(serv_test, 0);

	/* SU */
	mt_serv_set_txbf_pfmu_tag_sumu(serv_test, 0);

	/* PFMU memory allocation */
	mt_serv_get_wrap_ibf_cal_ibf_mem_alloc(
				serv_test, pfmu_mem_row, pfmu_mem_col);
	mt_serv_set_txbf_pfmu_tag_mem(
				serv_test, pfmu_mem_row, pfmu_mem_col);

	/* Nr:Nc:Ng:LM:CB:HTCE */
	mt_serv_set_txbf_pfmu_tag_matrix(serv_test, nr, nc, 0, 0, 0, 0);

	/* SNR */
	sys_ad_zero_mem(buf, 4);
	mt_serv_set_txbf_pfmu_tag_snr(serv_test, buf);

	/* SMART Antenna */
	mt_serv_set_txbf_pfmu_tag_smart_ant(serv_test, 0);

	/* SE index */
	mt_serv_set_txbf_pfmu_tag_se_idx(serv_test, 0);

	/* Rmsd */
	mt_serv_set_txbf_pfmu_tag_rmsd_thrd(serv_test, 0);

	/* MCS threshold */
	sys_ad_zero_mem(buf, 6);
	mt_serv_set_txbf_pfmu_tag_mcs_thrd(serv_test, buf, &buf[3]);

	/* Time out disable */
	mt_serv_set_txbf_pfmu_tag_time_out(serv_test, 255);

	/* Desired BW20 */
	mt_serv_set_txbf_pfmu_tag_desired_bw(serv_test, 0);

	/* Nr */
	mt_serv_set_txbf_pfmu_tag_desired_nr(serv_test, nr);

	/* Nc */
	mt_serv_set_txbf_pfmu_tag_desired_nc(serv_test, nc);

	/* Invalid the tag */
	mt_serv_set_txbf_profile_tag_invalid(serv_test, TRUE);

	/* Update PFMU tag */
	mt_serv_set_txbf_profile_tag_write(serv_test, pfmu_idx);

	/* Configure the BF StaRec */
	switch (nr) {
	case 1:
		ndp_nss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		ndp_nss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		ndp_nss = 24; /* MCS24, 4 streams */
		break;

	default:
		break;
	}

	wcid = 1;

	tmplen = snprintf(cmd_str, sizeof(cmd_str),
				"%.2x:00:%.2x:00:00:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:",
					wcid, pfmu_idx, ndp_nss, nc, nr);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	tmplen = snprintf(&cmd_str[45], sizeof(cmd_str) - 45,
				"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
					pfmu_mem_row[0], pfmu_mem_col[0],
					pfmu_mem_row[1], pfmu_mem_col[1],
					pfmu_mem_row[2], pfmu_mem_col[2],
					pfmu_mem_row[3], pfmu_mem_col[3]);
	if (snprintf_error(sizeof(cmd_str) - 45, tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_sta_rec_bf_update(serv_test, cmd_str);

	tmplen = snprintf(cmd_str, sizeof(cmd_str), "%d", wcid);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_sta_rec_bf_read(serv_test, cmd_str);

	/* Configure WTBL */
	addr1 = (u_char *)CONFIG_GET_PADDR(serv_test, addr1[wcid-1], ctrl_band_idx);

	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1 */
			/* -ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	tmplen = snprintf(cmd_str, sizeof(cmd_str),
				"mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:",
						addr1[0], addr1[1], addr1[2],
						addr1[3], addr1[4], addr1[5]);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	tmplen = snprintf(&cmd_str[52], sizeof(cmd_str) - 52,
				"aanac-bw:20-nss:%d-pfmuId:%d\n",
						(nc + 1), pfmu_idx);
	if (snprintf_error(sizeof(cmd_str) - 52, tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_manual_assoc(serv_test, cmd_str);

	return ret;
}


s_int32 mt_serv_set_ebf_profile_update(
	struct service_test *serv_test, u_char pfmu_idx, u_char nc)
{
	u_char ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32 tmplen;
	u_char wcid;
	u_char nr, ndp_nss = 0, pfmu_mem_row[4] = {0}, pfmu_mem_col[4] = {0};
	u_char tx_ant_cfg, *addr1 = NULL;
	u_char cmd_str[80], buf[6];


	tx_ant_cfg = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);
	switch (tx_ant_cfg) {
	case 3:
		nr = 1;
		break;
	case 7:
		nr = 2;
		break;
	case 12:
		nr = 3;
		break;
	case 15:
		nr = 3;
		break;
	default:
		nr = 3;
		break;
	}

	/* Configure iBF tag */
	/* PFMU ID */
	mt_serv_set_txbf_profile_tag_idx(serv_test, pfmu_idx);

	/* ETxBf */
	mt_serv_set_txbf_pfmu_tag_bf_type(serv_test, 1);

	/* BW20 */
	mt_serv_set_txbf_pfmu_tag_dbw(serv_test, 0);

	/* SU */
	mt_serv_set_txbf_pfmu_tag_sumu(serv_test, 0);

	/* PFMU memory allocation */
	mt_serv_get_wrap_ibf_cal_ebf_mem_alloc(
				serv_test, pfmu_mem_row, pfmu_mem_col);
	mt_serv_set_txbf_pfmu_tag_mem(
				serv_test, pfmu_mem_row, pfmu_mem_col);

	/* Nr:Nc:Ng:LM:CB:HTCE */
	mt_serv_set_txbf_pfmu_tag_matrix(serv_test, nr, nc, 0, 1, 0, 0);

	/* SNR */
	sys_ad_zero_mem(buf, 4);
	mt_serv_set_txbf_pfmu_tag_snr(serv_test, buf);

	/* SMART Antenna */
	mt_serv_set_txbf_pfmu_tag_smart_ant(serv_test, 0);

	/* SE index */
	mt_serv_set_txbf_pfmu_tag_se_idx(serv_test, 0);

	/* Rmsd */
	mt_serv_set_txbf_pfmu_tag_rmsd_thrd(serv_test, 0);

	/* MCS threshold */
	sys_ad_zero_mem(buf, 6);
	mt_serv_set_txbf_pfmu_tag_mcs_thrd(serv_test, buf, &buf[3]);

	/* Invalid the tag */
	mt_serv_set_txbf_profile_tag_invalid(serv_test, TRUE);

	/* Update PFMU tag */
	mt_serv_set_txbf_profile_tag_write(serv_test, pfmu_idx);

	/* Configure the BF StaRec */
	switch (nr) {
	case 1:
		ndp_nss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		ndp_nss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		ndp_nss = 24; /* MCS24, 4 streams */
		break;

	default:
		break;
	}

	wcid = 1;

	tmplen = snprintf(cmd_str, sizeof(cmd_str),
				"%.2x:00:%.2x:00:01:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:",
					wcid, pfmu_idx, ndp_nss, nc, nr);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	tmplen = snprintf(&cmd_str[45], sizeof(cmd_str) - 45,
				"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
					pfmu_mem_row[0], pfmu_mem_col[0],
					pfmu_mem_row[1], pfmu_mem_col[1],
					pfmu_mem_row[2], pfmu_mem_col[2],
					pfmu_mem_row[3], pfmu_mem_col[3]);
	if (snprintf_error(sizeof(cmd_str) - 45, tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_sta_rec_bf_update(serv_test, cmd_str);

	tmplen = snprintf(cmd_str, sizeof(cmd_str), "%d", wcid);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_sta_rec_bf_read(serv_test, cmd_str);

	/* Configure WTBL */
	addr1 = (u_char *)CONFIG_GET_PADDR(serv_test, addr1[wcid-1], ctrl_band_idx);

	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1 */
			/* -ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	tmplen = snprintf(cmd_str, sizeof(cmd_str),
				"mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:",
						addr1[0], addr1[1], addr1[2],
						addr1[3], addr1[4], addr1[5]);
	if (snprintf_error(sizeof(cmd_str), tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	tmplen = snprintf(&cmd_str[52], sizeof(cmd_str) - 52,
				"aanac-bw:20-nss:%d-pfmuId:%d\n",
						(nc + 1), pfmu_idx);
	if (snprintf_error(sizeof(cmd_str) - 52, tmplen)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: snprintf error!!\n", __func__));
		return SERV_STATUS_AGENT_INVALID_PARAM;
	}

	mt_serv_set_manual_assoc(serv_test, cmd_str);

	return ret;
}


s_int32 mt_serv_set_ibf_inst_cal_init(
	struct service_test *serv_test)
{
	struct test_operation *op = serv_test->test_op;


	if (op->op_set_ibf_phase_cal_init)
		return op->op_set_ibf_phase_cal_init(serv_test->test_winfo);
	else
		return SERV_STATUS_SERV_TEST_NOT_SUPPORTED;

}


s_int32 mt_serv_set_ibf_inst_cal(
	struct service_test *serv_test, u_char group_idx, u_char group_l_m_h,
	boolean fg_sx2, u_char phase_cal, u_char phase_lna_gain_level)
{
	return mt_ad_set_ibf_inst_cal(
				serv_test->test_winfo,
				group_idx,
				group_l_m_h,
				fg_sx2,
				phase_cal,
				phase_lna_gain_level);
}


s_int32 mt_serv_set_txbf_chan_profile_update(
	struct service_test *serv_test, u_int16 *arg)
{
	struct  test_wlan_info *winfos = serv_test->test_winfo;
	struct serv_chip_cap *cap = &winfos->chip_cap;
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int16 buf[16];
	boolean fg_final_data;
	u_int16 pfmu_idx, subcarr_idx;
	s_int16 phi11,    phi21,     phi31;
	s_int16 h11,      angle_h11, h21, angle_h21, h31, angle_h31,
		h41,      angle_h41;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  ucTxPath;


	pfmu_idx      = arg[0];
	subcarr_idx   = arg[1];
	fg_final_data = arg[2];
	h11       = (s_int16)(arg[3] << 3) >> 3;
	angle_h11 = (s_int16)(arg[4] << 3) >> 3;
	h21       = (s_int16)(arg[5] << 3) >> 3;
	angle_h21 = (s_int16)(arg[6] << 3) >> 3;
	h31       = (s_int16)(arg[7] << 3) >> 3;
	angle_h31 = (s_int16)(arg[8] << 3) >> 3;
	h41       = (s_int16)(arg[9] << 3) >> 3;
	angle_h41 = (s_int16)(arg[10] << 3) >> 3;
	phi11     = 0;
	phi21     = 0;
	phi31     = 0;

	ucTxPath = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);

	switch (ucTxPath) {
	case SERV_TX_PATH_2:
		phi11 = angle_h21 - angle_h11;
		phi21	  = 0;
		break;

	case SERV_TX_PATH_3:
		phi11 = angle_h31 - angle_h11;
		phi21 = angle_h31 - angle_h21;
		break;

	case SERV_TX_PATH_4:
	default:
		if ((WINFO_GET_PARAM(serv_test, dbdc_mode)) && (
			cap->is_dual_phy == FALSE)) {
			phi11 = angle_h21 - angle_h11;
			phi21	  = 0;
			phi31	  = 0;
		} else {
			phi11 = angle_h41 - angle_h11;
			phi21 = angle_h41 - angle_h21;
			phi31 = angle_h41 - angle_h31;
		}
		break;
	}


	/* Update the tag to enable eBF profile */
	if (fg_final_data) {
		mt_serv_set_txbf_profile_tag_read(serv_test, pfmu_idx, TRUE);
		mt_serv_set_txbf_profile_tag_invalid(serv_test, TRUE);
		mt_serv_set_txbf_profile_tag_write(serv_test, pfmu_idx);
	}

	/* Update the profile data per subcarrier */
	sys_ad_zero_mem(buf, 16);

	/* pfmu_idx   = buf[0];  */
	/* sucarr_idx = buf[1];  */
	/* phi11      = buf[2];  */
	/* psi21      = buf[3];  */
	/* phi21      = buf[4];  */
	/* psi31      = buf[5];  */
	/* phi31      = buf[6];  */
	/* psi41      = buf[7];  */
	/* phi41      = buf[8];  */
	/* psi51      = buf[9];  */
	/* phi51      = buf[10]; */
	/* psi61      = buf[11]; */
	/* phi61      = buf[12]; */
	/* psi71      = buf[13]; */
	/* phi71      = buf[14]; */
	/* psi81      = buf[15]; */

	buf[0] = pfmu_idx;
	buf[1] = subcarr_idx;

	switch (ucTxPath) {
	case SERV_TX_PATH_2:
		buf[2] = (u_int16)((u_int16)phi11 & 0xFFF);
		break;

	case SERV_TX_PATH_3:
		buf[2] = (u_int16)((u_int16)phi11 & 0xFFF);
		buf[4] = (u_int16)((u_int16)phi21 & 0xFFF);
		break;

	case SERV_TX_PATH_4:
	default:
		buf[2] = (u_int16)((u_int16)phi11 & 0xFFF);
		buf[4] = (u_int16)((u_int16)phi21 & 0xFFF);
		buf[6] = (u_int16)((u_int16)phi31 & 0xFFF);
		break;
	}

	ret = mt_serv_set_txbf_pfmu_data_write(serv_test, buf);

	return ret;
}


s_int32 mt_serv_set_txbf_profile_data_Write_20m_all(
	struct service_test *serv_test,
	u_char profile_idx,
	u_char *data)
{
	return mt_ad_set_txbf_profile_data_write_20m_all(
					serv_test->test_winfo,
					profile_idx,
					data);
}
#endif /* TXBF_SUPPORT*/

s_int32 mt_serv_store_cali(
	struct service_test *serv_test,
	u_int32 storage,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	switch (storage) {
	case TEST_CALI_SRC_EFUSE:
		net_ad_w_cali_2_efuse(serv_test->test_winfo, data);
		break;
	case TEST_CALI_SRC_FLASH:
	case TEST_CALI_SRC_ROM:
	case TEST_CALI_SRC_FILE:
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN,
			 ("%s: storage type:%d is not supported\n",
			  __func__, storage));
		break;
	default:
		ret = SERV_STATUS_SERV_TEST_INVALID_PARAM;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}

s_int32 mt_serv_tx_carr(struct service_test *serv_test)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *configs;

	/* Get parameter */
	u_char	ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 ant_sel = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s:\nband_idx[%d], Ant[%d]\n",
		__func__, ctrl_band_idx, ant_sel));

	/* Set parameter */
	configs = &serv_test->test_config[ctrl_band_idx];
	configs->tx_tone_en = 1;
	configs->ant_idx = ant_sel;
	configs->tone_type = 0;
	configs->tone_freq = 0;
	configs->dc_offset_I = 10;
	configs->dc_offset_Q = 10;
	configs->rf_pwr = 10;
	configs->digi_pwr = serv_test->test_config[ctrl_band_idx].tx_pwr[0];

	ret = mt_serv_dbdc_tx_tone(serv_test);
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

		return ret;
	}

	ret = mt_serv_dbdc_tx_tone_pwr(serv_test);
	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	configs->op_mode |= OP_MODE_TXCARR;

	return ret;
}

s_int32 mt_serv_main(struct service_test *serv_test, u_int32 test_item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	switch (test_item) {
	case SERV_TEST_ITEM_INIT:
		ret = mt_serv_init_test(serv_test);
		break;

	case SERV_TEST_ITEM_EXIT:
		ret = mt_serv_exit_test(serv_test);
		break;

	case SERV_TEST_ITEM_START:
		ret = mt_serv_start(serv_test);
		break;

	case SERV_TEST_ITEM_STOP:
		ret = mt_serv_stop(serv_test);
		break;

	case SERV_TEST_ITEM_START_TX:
		ret = mt_serv_start_tx(serv_test);
		break;

	case SERV_TEST_ITEM_STOP_TX:
		ret = mt_serv_stop_tx(serv_test);
		break;

	case SERV_TEST_ITEM_START_RX:
		ret = mt_serv_start_rx(serv_test);
		break;

	case SERV_TEST_ITEM_STOP_RX:
		ret = mt_serv_stop_rx(serv_test);
		break;

	default:
		return SERV_STATUS_SERV_TEST_NOT_SUPPORTED;
	}

	if (ret)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: err=0x%08x\n", __func__, ret));

	return ret;
}
