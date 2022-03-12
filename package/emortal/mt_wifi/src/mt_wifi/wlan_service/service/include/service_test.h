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
	service_test.h
*/
#ifndef __SERVICE_TEST_H__
#define __SERVICE_TEST_H__

#include "test_engine.h"
#include "operation.h"

/*****************************************************************************
 *	Macro
 *****************************************************************************/
#define SERV_GET_PARAM(_struct, _member)				\
	(_struct->_member)
#define SERV_GET_PADDR(_struct, _member)				\
	(&_struct->_member)
#define SERV_SET_PARAM(_struct, _member, _val)				\
	(_struct->_member = _val)
#define WINFO_GET_PARAM(_struct, _member)				\
	(_struct->test_winfo->_member)
#define WINFO_GET_PADDR(_struct, _member)				\
	(&_struct->test_winfo->_member)
#define WINFO_SET_PARAM(_struct, _member, _val)				\
	(_struct->test_winfo->_member = _val)
#define BSTATE_GET_PARAM(_struct, _member)				\
	(_struct->test_bstat._member)
#define BSTATE_SET_PARAM(_struct, _member, _val)			\
	(_struct->test_bstat._member = _val)
#define CONFIG_GET_PARAM(_struct, _member, _bandidx)			\
	(_struct->test_config[_bandidx]._member)
#define CONFIG_GET_PADDR(_struct, _member, _bandidx)			\
	(&_struct->test_config[_bandidx]._member)
#define CONFIG_SET_PARAM(_struct, _member, _val, _bandidx)		\
	(_struct->test_config[_bandidx]._member = _val)
#define CONFIG_SET_PADDR(_struct, _member, _val, _size, _bandidx) ({	\
	struct test_configuration *configs;				\
	configs = &_struct->test_config[_bandidx];			\
	(sys_ad_move_mem(configs->_member, _val, _size));		\
	})
#define EEPROM_GET_PARAM(_struct, _member)				\
	(_struct->test_eprm._member)
#define EEPROM_SET_PARAM(_struct, _member, _val)			\
	(_struct->test_eprm._member = _val)

#define serv_isxdigit(_char)  \
	(('0' <= (_char) && (_char) <= '9') ||\
	 ('a' <= (_char) && (_char) <= 'f') || \
	 ('A' <= (_char) && (_char) <= 'F') \
	)

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
/* Service test item id */
enum {
	SERV_TEST_ITEM_INIT = 0,
	SERV_TEST_ITEM_EXIT,
	SERV_TEST_ITEM_START,
	SERV_TEST_ITEM_STOP,
	SERV_TEST_ITEM_START_TX,
	SERV_TEST_ITEM_STOP_TX,
	SERV_TEST_ITEM_START_RX,
	SERV_TEST_ITEM_STOP_RX
};

/* Service test register/eeprom related operation */
enum {
	SERV_TEST_REG_MAC_READ = 0,
	SERV_TEST_REG_MAC_WRITE,
	SERV_TEST_REG_MAC_READ_BULK,
	SERV_TEST_REG_RF_READ_BULK,
	SERV_TEST_REG_RF_WRITE_BULK,
	SERV_TEST_REG_CA53_READ,
	SERV_TEST_REG_CA53_WRITE,

	SERV_TEST_EEPROM_READ = 10,
	SERV_TEST_EEPROM_WRITE,
	SERV_TEST_EEPROM_READ_BULK,
	SERV_TEST_EEPROM_WRITE_BULK,
	SERV_TEST_EEPROM_GET_FREE_EFUSE_BLOCK
};

/* Service test mps related operation */
enum {
	SERV_TEST_MPS_START_TX = 0,
	SERV_TEST_MPS_STOP_TX
};

/* Service test tx power related operation */
enum {
	SERV_TEST_TXPWR_SET_PWR = 0,
	SERV_TEST_TXPWR_GET_PWR,
	SERV_TEST_TXPWR_SET_PWR_INIT,
	SERV_TEST_TXPWR_SET_PWR_MAN,
};

/* Service test iBF phase cal */
enum {
	SERV_TX_PATH_2 = 3,
	SERV_TX_PATH_3 = 7,
	SERV_TX_PATH_4 = 15,
};

/* Service test rxv content category */
enum {
	TEST_RXV_CONTENT_CMN1 = 0,
	TEST_RXV_CONTENT_USR1,
	TEST_RXV_CONTENT_USR2,
	TEST_RXV_CONTENT_CMN2,
	TEST_RXV_CONTENT_NUM
};

enum {
	TEST_CALI_SRC_EFUSE = 1,
	TEST_CALI_SRC_FLASH,
	TEST_CALI_SRC_ROM,
	TEST_CALI_SRC_FILE,
	TEST_CALI_SRC_MAX
};
/*****************************************************************************
 *	Data struct definition
 *****************************************************************************/
/* Service data struct for test mode usage */
struct service_test {
	/*========== Jedi only ==========*/
	/* Wlan related information which test needs */
	struct test_wlan_info *test_winfo;

	/* Test backup CR */
	struct test_bk_cr test_bkcr[TEST_MAX_BKCR_NUM];

	/* Test Rx statistic */
	struct test_rx_stat test_rx_statistic[TEST_DBDC_BAND_NUM];

	/* The band related state which communicate between UI/driver */
	struct test_band_state test_bstat;

	/* The band_idx which user wants to control currently */
	u_char ctrl_band_idx;

	struct test_backup_params test_backup;

	/*========== Common part ==========*/
	/* Test configuration */
	struct test_configuration test_config[TEST_DBDC_BAND_NUM];

	/* Test operation */
	struct test_operation *test_op;

	/* Test control register read/write */
	struct test_register test_reg;

	/* Test eeprom read/write */
	struct test_eeprom test_eprm;

	/* Test tmr related configuration */
	struct test_tmr_info test_tmr;

	/* Jedi: false, Gen4m: true */
	boolean engine_offload;

	/* TODO: factor out here for log dump */
	u_int32 en_log;
	struct test_log_dump_cb test_log_dump[TEST_LOG_TYPE_NUM];
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 mt_serv_init_test(struct service_test *serv_test);
s_int32 mt_serv_exit_test(struct service_test *serv_test);
s_int32 mt_serv_start(struct service_test *serv_test);
s_int32 mt_serv_stop(struct service_test *serv_test);
s_int32 mt_serv_set_channel(struct service_test *serv_test);
s_int32 mt_serv_set_tx_content(struct service_test *serv_test);
s_int32 mt_serv_set_tx_path(struct service_test *serv_test);
s_int32 mt_serv_set_rx_path(struct service_test *serv_test);
s_int32 mt_serv_submit_tx(struct service_test *serv_test);
s_int32 mt_serv_revert_tx(struct service_test *serv_test);
s_int32 mt_serv_start_tx(struct service_test *serv_test);
s_int32 mt_serv_stop_tx(struct service_test *serv_test);
s_int32 mt_serv_start_rx(struct service_test *serv_test);
s_int32 mt_serv_stop_rx(struct service_test *serv_test);
s_int32 mt_serv_group_prek_clean(struct service_test *serv_test);
s_int32 mt_serv_group_prek_dump(struct service_test *serv_test);
s_int32 mt_serv_group_prek(struct service_test *serv_test);
s_int32 mt_serv_dpd_prek_clean(struct service_test *serv_test);
s_int32 mt_serv_dpd_prek_dump(struct service_test *serv_test);
s_int32 mt_serv_dpd_prek_5g(struct service_test *serv_test);
s_int32 mt_serv_dpd_prek_2g(struct service_test *serv_test);
s_int32 mt_serv_dpd_prek(struct service_test *serv_test);
s_int32 mt_serv_set_freq_offset(struct service_test *serv_test);
s_int32 mt_serv_tx_power_operation(
	struct service_test *serv_test, u_int32 item);
s_int32 mt_serv_get_freq_offset(
	struct service_test *serv_test, u_int32 *freq_offset);
s_int32 mt_serv_get_cfg_on_off(
	struct service_test *serv_test,
	u_int32 type, u_int32 *result);
s_int32 mt_serv_get_tx_tone_pwr(
	struct service_test *serv_test,
	u_int32 ant_idx, u_int32 *power);
s_int32 mt_serv_get_thermal_val(
	struct service_test *serv_test,
	u_char band_idx,
	u_int32 *value);
s_int32 mt_serv_set_cal_bypass(
	struct service_test *serv_test,
	u_int32 cal_item);
s_int32 mt_serv_set_dpd(
	struct service_test *serv_test,
	u_int32 on_off,
	u_int32 wf_sel);
s_int32 mt_serv_set_tssi(
	struct service_test *serv_test,
	u_int32 on_off,
	u_int32 wf_sel);
s_int32 mt_serv_set_rdd_on_off(
	struct service_test *serv_test,
	u_int32 rdd_num,
	u_int32 rdd_sel,
	u_int32 enable);
s_int32 mt_serv_set_off_ch_scan(
	struct service_test *serv_test);
s_int32 mt_serv_set_icap_start(
	struct service_test *serv_test,
	struct hqa_rbist_cap_start *icap_info);
s_int32 mt_serv_get_icap_status(
	struct service_test *serv_test,
	s_int32 *icap_stat);
s_int32 mt_serv_get_icap_max_data_len(
	struct service_test *serv_test,
	u_long *max_data_len);
s_int32 mt_serv_get_icap_data(
	struct service_test *serv_test,
	s_int32 *icap_cnt,
	s_int32 *icap_data,
	u_int32 wf_num,
	u_int32 iq_type);
s_int32 mt_serv_get_recal_cnt(
	struct service_test *serv_test,
	u_int32 *recal_cnt,
	u_int32 *recal_dw_num);
s_int32 mt_serv_get_recal_content(
	struct service_test *serv_test,
	u_int32 *content);
s_int32 mt_serv_get_rxv_cnt(
	struct service_test *serv_test,
	u_int32 *rxv_cnt,
	u_int32 *rxv_dw_num);
s_int32 mt_serv_rxv_dump_action(
	struct service_test *serv_test,
	u_int32 action,
	u_int32 type_mask);
s_int32 mt_serv_get_rxv_content(
	struct service_test *serv_test,
	u_int32 dw_cnt,
	u_int32 *content);
s_int32 mt_serv_get_rdd_cnt(
	struct service_test *serv_test,
	u_int32 *rdd_cnt,
	u_int32 *rdd_dw_num);
s_int32 mt_serv_get_rdd_content(
	struct service_test *serv_test,
	u_int32 *content,
	u_int32 *total_cnt);
s_int32 mt_serv_reset_txrx_counter(struct service_test *serv_test);
s_int32 mt_serv_set_rx_vector_idx(
	struct service_test *serv_test, u_int32 group1, u_int32 group2);
s_int32 mt_serv_set_fagc_rssi_path(
	struct service_test *serv_test);
s_int32 mt_serv_get_rx_stat_leg(
	struct service_test *serv_test,
	struct test_rx_stat_leg *rx_stat);
s_int32 mt_serv_get_rxv_dump_ring_attr(
	struct service_test *serv_test,
	struct rxv_dump_ring_attr *ring_attr);
s_int32 mt_serv_get_rxv_dump_content(
	struct service_test *serv_test,
	u_int8 entry_idx,
	u_int32 *content_len,
	void *rxv_content);
s_int32 mt_serv_get_rxv_content_len(
	struct service_test *serv_test,
	u_int8 type_idx,
	u_int8 rxv_sta_cnt,
	u_int16 *len);
s_int32 mt_serv_calibration_test_mode(
	struct service_test *serv_test, u_char mode);
s_int32 mt_serv_do_cal_item(
	struct service_test *serv_test, u_int32 item);
s_int32 mt_serv_set_band_mode(struct service_test *serv_test);
s_int32 mt_serv_get_band_mode(struct service_test *serv_test);
s_int32 mt_serv_log_on_off(
	struct service_test *serv_test, u_int32 log_type,
	u_int32 log_ctrl, u_int32 log_size);
s_int32 mt_serv_set_cfg_on_off(struct service_test *serv_test);
s_int32 mt_serv_set_rx_filter_pkt_len(struct service_test *serv_test);
s_int32 mt_serv_get_thermal_value(struct service_test *serv_test);
s_int32 mt_serv_get_wf_path_comb(struct service_test *serv_test,
	u_int8 band_idx, boolean dbdc_mode_en, u_int8 *path, u_int8 *path_len);
s_int32 mt_serv_set_low_power(
	struct service_test *serv_test, u_int32 control);
s_int32 mt_serv_get_antswap_capability(
	struct service_test *serv_test, u_int32 *antswap_support);
s_int32 mt_serv_set_antswap(
	struct service_test *serv_test, u_int32 ant);
s_int32 mt_serv_reg_eprm_operation(
	struct service_test *serv_test, u_int32 item);
s_int32 mt_serv_mps_operation(
	struct service_test *serv_test, u_int32 item);
s_int32 mt_serv_get_chipid(struct service_test *serv_test);
s_int32 mt_serv_mps_set_seq_data(struct service_test *serv_test);
s_int32 mt_serv_set_tmr(struct service_test *serv_test);
s_int32 mt_serv_set_preamble(struct service_test *serv_test);
s_int32 mt_serv_set_system_bw(struct service_test *serv_test);
s_int32 mt_serv_set_per_pkt_bw(struct service_test *serv_test);
s_int32 mt_serv_dbdc_tx_tone(struct service_test *serv_test);
s_int32 mt_serv_dbdc_tx_tone_pwr(struct service_test *serv_test);
s_int32 mt_serv_dbdc_continuous_tx(struct service_test *serv_test);
s_int32 mt_serv_get_tx_info(struct service_test *serv_test);
s_int32 mt_serv_main(struct service_test *serv_test, u_int32 test_item);
s_int32 mt_serv_get_rx_stat(struct service_test *serv_test, u_int8 band_idx,
	u_int8 blk_idx, u_int8 test_rx_stat_cat, struct test_rx_stat_u *st);
s_int32 mt_serv_dnlk_clean(struct service_test *serv_test);
s_int32 mt_serv_dnlk_2g(struct service_test *serv_test);
s_int32 mt_serv_dnlk_5g(struct service_test *serv_test);
#ifdef TXBF_SUPPORT
s_int32 mt_serv_set_txbf_sa(
	struct service_test *serv_test, u_char *addr);
s_int32 mt_serv_set_bss_info(
	struct service_test *serv_test, u_char *bssid);
s_int32 mt_serv_set_device_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char *addr,
	u_char band_idx, u_char mode);
s_int32 mt_serv_set_txbf_tx_apply(
	struct service_test *serv_test, u_char *buf);
s_int32 mt_serv_set_txbf_lna_gain(
	struct service_test *serv_test, u_char lna_gain);
s_int32 mt_serv_set_ibf_phase_comp(
	struct service_test *serv_test,
	u_char bw,
	boolean fg_jp_band,
	u_char dbdc_band_idx,
	u_char group_idx,
	boolean fg_read_from_e2p,
	boolean fg_dis_comp);
s_int32 mt_serv_set_txbf_profile_tag_read(
	struct service_test *serv_test, u_char  pf_idx, boolean fg_bfer);
s_int32 mt_serv_set_txbf_profile_tag_write(
	struct service_test *serv_test, u_char prf_idx);
s_int32 mt_serv_set_txbf_profile_tag_invalid(
	struct service_test *serv_test, boolean fg_invalid);
s_int32 mt_serv_set_txbf_profile_tag_idx(
	struct service_test *serv_test, u_char profile_idx);
s_int32 mt_serv_set_txbf_pfmu_tag_bf_type(
	struct service_test *serv_test, u_char bf_type);
s_int32 mt_serv_set_txbf_pfmu_tag_dbw(
	struct service_test *serv_test, u_char dbw);
s_int32 mt_serv_set_txbf_pfmu_tag_sumu(
	struct service_test *serv_test, u_char su_mu);
s_int32 mt_serv_set_txbf_pfmu_tag_mem(
	struct service_test *serv_test,
	u_char *pfmu_mem_row, u_char *pfmu_mem_col);
s_int32 mt_serv_set_txbf_pfmu_tag_matrix(
	struct service_test *serv_test,
	u_char nr, u_char nc, u_char ng, u_char lm, u_char cb, u_char he);
s_int32 mt_serv_set_txbf_pfmu_tag_snr(
	struct service_test *serv_test, u_char *buf);
s_int32 mt_serv_set_txbf_pfmu_tag_smart_ant(
	struct service_test *serv_test, u_int32 smart_ant);
s_int32 mt_serv_set_txbf_pfmu_tag_se_idx(
	struct service_test *serv_test, u_int32 se_idx);
s_int32 mt_serv_set_txbf_pfmu_tag_rmsd_thrd(
	struct service_test *serv_test, u_char rmsd_thrd);
s_int32 mt_serv_set_txbf_pfmu_tag_mcs_thrd(
	struct service_test *serv_test, u_char *mcs_lss, u_char *mcs_sss);
s_int32 mt_serv_set_txbf_pfmu_tag_time_out(
	struct service_test *serv_test, u_char time_out);
s_int32 mt_serv_set_txbf_pfmu_tag_desired_bw(
	struct service_test *serv_test, u_char desired_bw);
s_int32 mt_serv_set_txbf_pfmu_tag_desired_nr(
	struct service_test *serv_test, u_char desired_nr);
s_int32 mt_serv_set_txbf_pfmu_tag_desired_nc(
	struct service_test *serv_test, u_char desired_nc);
s_int32 mt_serv_set_txbf_pfmu_data_write(
	struct service_test *serv_test, u_int16 *buf);
s_int32 mt_serv_set_sta_rec_bf_update(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_serv_set_sta_rec_bf_read(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_serv_set_manual_assoc(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_serv_set_tx_pkt_with_ibf(
	struct service_test *serv_test,
	u_char wlan_idx,
	u_int32 tx_cnt,
	boolean fg_bf,
	boolean fg_tx_param_update);
s_int32 mt_serv_set_ibf_profile_update(
	struct service_test *serv_test, u_char pfmu_idx, u_char nc);
s_int32 mt_serv_set_ebf_profile_update(
	struct service_test *serv_test, u_char pfmu_idx, u_char nc);
s_int32 mt_serv_set_txbf_profile_data_Write_20m_all(
	struct service_test *serv_test,
	u_char profile_idx,
	u_char *data);
s_int32 mt_serv_set_ibf_inst_cal_init(
	struct service_test *serv_test);
s_int32 mt_serv_set_ibf_inst_cal(
	struct service_test *serv_test,
	u_char group_idx,
	u_char group_l_m_h,
	boolean fg_sx2,
	u_char phase_cal,
	u_char phase_lna_gain_level);
s_int32 mt_serv_set_txbf_chan_profile_update(
	struct service_test *serv_test, u_int16 *arg);
s_int32 mt_serv_set_ibf_phase_cal_e2p_update(
	struct service_test *serv_test,
	u_char group_idx,
	boolean  fgSx2,
	u_char update_type);
#endif /* TXBF_SUPPORT */

s_int32 mt_serv_store_cali(
	struct service_test *serv_test,
	u_int32 storage,
	u_int8 *data);

#endif /* __SERVICE_TEST_H__ */
