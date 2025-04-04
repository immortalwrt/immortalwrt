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
	agent.h
*/
#ifndef __AGENT_H__
#define __AGENT_H__

#include "service_test.h"

/*****************************************************************************
 *	Type definition
 *****************************************************************************/

/*****************************************************************************
 *	Macro
 *****************************************************************************/
#define AGENT_CFG_ARGV_MAX 20
#define SERV_TEST_ON(_config) ((((_config)->op_mode) & OP_MODE_START) == OP_MODE_START)

#define TEST_CMDREQ			0x0008
#define TEST_CMDRSP			0x8008
#define TEST_CMD_MAGIC_NO		0x18142880

#define	TEST_CMD_REQ			0x0005
#define	TEST_CMD_RSP			0x8005

#define HQA_BF_STR_SIZE     512
#define SERV_MAC_ADDR_LEN   6
/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
/* String Parser State */
enum {
	AGENT_STATE_EOF = 0,
	AGENT_STATE_TEXT = 1,
	AGENT_STATE_NEWLINE = 2
};

/* Service handle id */
enum {
	SERV_HANDLE_RESV = 0,
	SERV_HANDLE_TEST = 1
};

enum {
	SERV_RX_STAT_TYPE_BAND = 0,
	SERV_RX_STAT_TYPE_PATH,
	SERV_RX_STAT_TYPE_USER,
	SERV_RX_STAT_TYPE_COMM,
	SERV_RX_STAT_TYPE_NUM
};

enum {
	HQA_BAND_WIDTH_20 = 0,
	HQA_BAND_WIDTH_40,
	HQA_BAND_WIDTH_80,
	HQA_BAND_WIDTH_10,
	HQA_BAND_WIDTH_5,
	HQA_BAND_WIDTH_160,
	HQA_BAND_WIDTH_8080,
	HQA_BAND_WIDTH_NUM
};

enum HQA_BF_CMD_ID {
	HQA_TXBF_INIT = 1,
	HQA_CHANNEL,
	HQA_TX_MCS,
	HQA_TX_POW0,
	HQA_TX_ANT,
	HQA_RX_FRAME,
	HQA_RX_ANT,
	HQA_TXBF_LNA_GAIN,
	HQA_IBF_PHASE_COMP,
	HQA_IBF_TX,
	HQA_IBF_PROF_UPDATE,
	HQA_EBF_PROF_UPDATE,
	HQA_IBF_INST_CAL,
	HQA_IBF_INST_VERIFY,
	HQA_TXBF_GD_INIT,
	HQA_IBF_PHASE_E2P_UPDATE
};

/*****************************************************************************
 *	Data struct definition
 *****************************************************************************/
/* Main service data struct */
struct service {
	u_int32 serv_id;
	void *serv_handle;
};

#pragma pack(1)
struct GNU_PACKED hqa_set_ch {
	u_int32 ext_id;
	u_int32 num_param;
	u_int32 band_idx;
	u_int32 central_ch0;
	u_int32 central_ch1;
	u_int32 sys_bw;
	u_int32 perpkt_bw;
	u_int32 pri_sel;
	u_int32 reason;
	u_int32 ch_band;
	u_int32 out_band_freq;
};

struct GNU_PACKED hqa_tx_content {
	u_int32 ext_id;
	u_int32 num_param;
	u_int32 band_idx;
	u_int32 fc;
	u_int32 dur;
	u_int32 seq;
	u_int32 fixed_payload;	/* Normal:0,Repeat:1,Random:2 */
	u_int32 txlen;
	u_int32 payload_len;
	u_char addr1[SERV_MAC_ADDR_LEN];
	u_char addr2[SERV_MAC_ADDR_LEN];
	u_char addr3[SERV_MAC_ADDR_LEN];
	u_char payload[0];
};

struct GNU_PACKED hqa_tx {
	u_int32 ext_id;
	u_int32 num_param;
	u_int32 band_idx;
	u_int32 pkt_cnt;
	u_int32 tx_mode;
	u_int32 rate;
	u_int32 pwr;
	u_int32 stbc;
	u_int32 ldpc;
	u_int32 ibf;
	u_int32 ebf;
	u_int32 wlan_id;
	u_int32 aifs;
	u_int32 gi;
	u_int32 tx_path;
	u_int32 nss;
};

struct GNU_PACKED hqa_frame {
	u_int32 magic_no;
	u_int16 type;
	u_int16 id;
	u_int16 length;
	u_int16 sequence;
	u_int8 data[SERV_IOCTLBUFF];
};

struct GNU_PACKED hqa_frame_ctrl {
	int8_t type;
	union {
		struct hqa_frame *hqa_frame_eth;
		int8_t *hqa_frame_string;
	} hqa_frame_comm;
};

struct GNU_PACKED hqa_tx_tone {
	u_int32 band_idx;
	u_int32 tx_tone_en;
	u_int32 ant_idx;
	u_int32 tone_type;
	u_int32 tone_freq;
	u_int32 dc_offset_I;
	u_int32 dc_offset_Q;
	u_int32 band;
	u_int32 rf_pwr;
	u_int32 digi_pwr;
};

struct GNU_PACKED hqa_continuous_tx {
	u_int32 band_idx;
	u_int32 tx_tone_en;
	u_int32 ant_mask;
	u_int32 tx_mode;
	u_int32 bw;
	u_int32 pri_ch;
	u_int32 rate;
	u_int32 central_ch;
	u_int32 tx_fd_mode;
};

struct GNU_PACKED hqa_rx_stat_resp_format {
	u_int32 type;
	u_int32 version;
	u_int32 item_mask;
	u_int32 blk_cnt;
	u_int32 blk_size;
};

struct GNU_PACKED hqa_rx_stat_band_info {
	u_int32 mac_rx_fcs_err_cnt;
	u_int32 mac_rx_mdrdy_cnt;
	u_int32 mac_rx_len_mismatch;
	u_int32 mac_rx_fcs_ok_cnt;
	u_int32 phy_rx_fcs_err_cnt_cck;
	u_int32 phy_rx_fcs_err_cnt_ofdm;
	u_int32 phy_rx_pd_cck;
	u_int32 phy_rx_pd_ofdm;
	u_int32 phy_rx_sig_err_cck;
	u_int32 phy_rx_sfd_err_cck;
	u_int32 phy_rx_sig_err_ofdm;
	u_int32 phy_rx_tag_err_ofdm;
	u_int32 phy_rx_mdrdy_cnt_cck;
	u_int32 phy_rx_mdrdy_cnt_ofdm;
};

struct GNU_PACKED hqa_rx_stat_path_info {
	u_int32 rcpi;
	u_int32 rssi;
	u_int32 fagc_ib_rssi;
	u_int32 fagc_wb_rssi;
	u_int32 inst_ib_rssi;
	u_int32 inst_wb_rssi;
};

struct GNU_PACKED hqa_rx_stat_user_info {
	s_int32 freq_offset_from_rx;
	u_int32 snr;
	u_int32 fcs_error_cnt;
};

struct GNU_PACKED hqa_rx_stat_comm_info {
	u_int32 rx_fifo_full;
	u_int32 aci_hit_low;
	u_int32 aci_hit_high;
	u_int32 mu_pkt_count;
	u_int32 sig_mcs;
	u_int32 sinr;
	u_int32 driver_rx_count;
};

struct GNU_PACKED hqa_rx_stat_leg {
	u_int32 mac_rx_fcs_err_cnt;
	u_int32 mac_rx_mdrdy_cnt;
	u_int32 phy_rx_fcs_err_cnt_cck;
	u_int32 phy_rx_fcs_err_cnt_ofdm;
	u_int32 phy_rx_pd_cck;
	u_int32 phy_rx_pd_ofdm;
	u_int32 phy_rx_sig_err_cck;
	u_int32 phy_rx_sfd_err_cck;
	u_int32 phy_rx_sig_err_ofdm;
	u_int32 phy_rx_tag_err_ofdm;
	u_int32 wb_rssi0;
	u_int32 ib_rssi0;
	u_int32 wb_rssi1;
	u_int32 ib_rssi1;
	u_int32 phy_rx_mdrdy_cnt_cck;
	u_int32 phy_rx_mdrdy_cnt_ofdm;
	u_int32 driver_rx_cnt;
	u_int32 rcpi0;
	u_int32 rcpi1;
	s_int32 freq_offset_rx;
	u_int32 rssi0;
	u_int32 rssi1;
	u_int32 rx_fifo_full;
	u_int32 mac_rx_len_mismatch;
	u_int32 mac_rx_fcs_err_cnt_band1;
	u_int32 mac_rx_mdrdy_cnt_band1;
	u_int32 fagc_ib_rssi[TEST_ANT_NUM];
	u_int32 fagc_wb_rssi[TEST_ANT_NUM];
	u_int32 inst_ib_rssi[TEST_ANT_NUM];
	u_int32 inst_wb_rssi[TEST_ANT_NUM];
	u_int32 aci_hit_low;
	u_int32 aci_git_high;
	u_int32 driver_rx_cnt1;
	u_int32 rcpi2;
	u_int32 rcpi3;
	u_int32 rssi2;
	u_int32 rssi3;
	u_int32 snr0;
	u_int32 snr1;
	u_int32 snr2;
	u_int32 snr3;
	u_int32 rx_fifo_full_band1;
	u_int32 mac_rx_len_mismatch_band1;
	u_int32 phy_rx_pd_cck_band1;
	u_int32 phy_rx_pd_ofdm_band1;
	u_int32 phy_rx_sig_err_cck_band1;
	u_int32 phy_rx_sfd_err_cck_band1;
	u_int32 phy_rx_sig_err_ofdm_band1;
	u_int32 phy_rx_tag_err_ofdm_band1;
	u_int32 phy_rx_mdrdy_cnt_cck_band1;
	u_int32 phy_rx_mdrdy_cnt_ofdm_band1;
	u_int32 phy_rx_fcs_err_cnt_cck_band1;
	u_int32 phy_rx_fcs_err_cnt_ofdm_band1;
	u_int32 mu_pkt_cnt;
	u_int32 sig_mcs;
	u_int32 sinr;
	u_int32 rxv_rssi;
	u_int32 mac_rx_fcs_ok_cnt;
	u_int32 leg_rssi_sub[8];
	s_int32 user_rx_freq_offset[TEST_USER_NUM];
	u_int32 user_snr[TEST_USER_NUM];
	u_int32 fcs_error_cnt[TEST_USER_NUM];
};

#pragma pack()

struct hqa_cmd_entry {
	u_int8 index;
	s_int32 (*handler)(struct service_test *serv_test,
			   struct hqa_frame *hqa_frame);
};

struct hqa_cmd_table {
	struct hqa_cmd_entry *cmd_set;
	u_int32 cmd_set_size;
	u_int32 cmd_offset;
};

struct agent_cfg_parse_state_s {
	s_int8 *ptr;
	s_int8 *text;
	u_int32 textsize;
	s_int32 nexttoken;
	u_int32 maxsize;
};

struct priv_hqa_cmd_id_mapping {
	u_int8 *cmd_str;
	u_int16 cmd_id;
	u_int8 para_size[AGENT_CFG_ARGV_MAX];
};

struct agent_cli_act_handler {
	u_char name[100];
	s_int32 (*handler)(struct service_test *serv_test);
};

struct agent_cli_show_handler {
	u_char name[100];
	s_int32 (*handler)(struct service_test *serv_test);
};

struct agent_cli_set_w_handler {
	u_char name[100];
	s_int32 (*handler)(struct service_test *serv_test,
				struct hqa_frame *hqa_cmd);
};

struct agent_cli_set_dw_handler {
	u_char name[100];
	s_int32 (*handler)(struct service_test *serv_test,
				struct hqa_frame *hqa_cmd);
};

struct agent_cli_set_ext_handler {
	u_char name[100];
	s_int32 (*handler)(struct service_test *serv_test,
				u_char *arg);
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 mt_agent_hqa_cmd_handler(
	struct service *serv, struct hqa_frame_ctrl *hqa_frame_ctrl);
s_int32 mt_agent_init_service(struct service *serv);
s_int32 mt_agent_exit_service(struct service *serv);
s_int32 mt_agent_cli_act(u_char *name, struct service *serv);
s_int32 mt_agent_cli_show(u_char *name, struct service *serv);
s_int32 mt_agent_cli_set_w(u_char *name, struct service *serv, u_char *param);
s_int32 mt_agent_cli_set_dw(u_char *name, struct service *serv, u_char *param);
s_int32 mt_agent_cli_set_ext(u_char *name,
					struct service *serv, u_char *arg);
s_int32 mt_agent_set_channel(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_pwr(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_txant(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_rxant(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_bw(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_show_TxDone(
	struct service_test *serv_test);
#ifdef TXBF_SUPPORT
s_int32 mt_agent_set_txbf_dut_init_proc(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_txbf_lna_gain(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_ibf_phase_comp(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_tx_pkt_with_ibf(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_ibf_profile_update(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_txbf_tx_apply(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_ebf_profile_update(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_ibf_inst_cal(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_ibf_phase_cal_e2p_update(
	struct service_test *serv_test, u_char *arg);
s_int32 mt_agent_set_txbf_chan_profile_update(
	struct service_test *serv_test, u_char *arg);
#endif
#endif /* __AGENT_H__ */
