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
#ifndef _HE_H_
#define _HE_H_

#include "dot11ax_he.h"

struct common_ies;

#define IE_VHT_TXPWR_ENV 195
#define INVALID_COLOR 0
#define HE_MAX_SUPPORT_STREAM DOT11AX_MAX_STREAM
enum {
	HE_BW_20,
	HE_BW_2040,
	HE_BW_80,
	HE_BW_160,
	HE_BW_8080 /*160/80+80*/
};

enum frag_num {
	FRAG_NUM_0,
	FRAG_NUM_1,
	FRAG_NUM_2,
	FRAG_NUM_3,
	FRAG_NUM_4,
	FRAG_NUM_5,
	FRAG_NUM_6,
	FRAG_NUM_NO_RESTRICT
};

#define HE_SET_MAC_CAPS(mac_cap, val) ((mac_cap) = (mac_cap) | (val))
#define HE_CLR_MAC_CAPS(mac_cap, val) ((mac_cap) = (mac_cap) & ~(val))
#define HE_CHK_MAC_CAPS(mac_cap, val) (((mac_cap) & (val)) == (val))

enum he_mac_caps {
	HE_HTC = 1,
	HE_TWT_REQUEST = (1 << 1),
	HE_TWT_RESPOND = (1 << 2),
	HE_ALL_ACK = (1 << 3),
	HE_UMRS = (1 << 4),
	HE_BSR = (1 << 5),
	HE_BROADCAST_TWT = (1 << 6),
	HE_32BIT_BA_BITMAP = (1 << 7),
	HE_MU_CASCADING = (1 << 8),
	HE_ACK_EN_AGG = (1 << 9),
	HE_GRP_ADDR_MULTI_STA_BA_DL_MU = (1 << 10),
	HE_OM_CTRL = (1 << 11),
	HE_OFDMA_RA = (1 << 12),
	HE_AMSDU_FRAG = (1 << 13),
	HE_FLEX_TWT_SCHDL = (1 << 14),
	HE_RX_CTRL_FRAME_TO_MULTIBSS = (1 << 15),
	HE_BSRP_BQRP_AMPDU_AGG = (1 << 16),
	HE_QTP = (1 << 17),
	HE_BQR = (1 << 18),
	HE_SRP_RESPONDER = (1 << 19),
	HE_NDP_FEEDBACK_REPORT = (1 << 20),
	HE_OPS = (1 << 21),
	HE_AMSDU_IN_ACK_EN_AMPDU = (1 << 22),
	HE_ER_SU_DISABLE = (1 << 23),
	HE_VHT_OPINFO_PRESENT = (1 << 24),
	HE_CO_LOCATED_BSS = (1 << 25),
	HE_CO_HOSTED_BSS = (1 << 26),
	HE_DYN_SMPS = (1 << 27)
};

enum he_bf_caps {
	HE_SU_BFER = 1,
	HE_SU_BFEE = (1 << 1),
	HE_MU_BFER = (1 << 2),
	HE_BFEE_NG_16_SU_FEEDBACK = (1 << 3),
	HE_BFEE_NG_16_MU_FEEDBACK = (1 << 4),
	HE_BFEE_CODEBOOK_SU_FEEDBACK = (1 << 5),
	HE_BFEE_CODEBOOK_MU_FEEDBACK = (1 << 6),
	HE_TRIG_SU_BFEE_FEEDBACK = (1 << 7),
	HE_TRIG_MU_BFEE_FEEDBACK = (1 << 8)
};

enum he_gi_caps {
	HE_SU_PPDU_1x_LTF_DOT8US_GI = 1,
	HE_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI = (1 << 1),
	HE_ER_SU_PPDU_1x_LTF_DOT8US_GI = (1 << 2),
	HE_ER_SU_PPDU_4x_LTF_DOT8US_GI = (1 << 3),
	HE_NDP_4x_LTF_3DOT2MS_GI = (1 << 4)
};

enum he_stbc_caps {
	HE_LE_EQ_80M_TX_STBC = 1,
	HE_LE_EQ_80M_RX_STBC = (1 << 1),
	HE_GT_80M_TX_STBC = (1 << 2),
	HE_GT_80M_RX_STBC = (1 << 3)
};

enum he_phy_caps {
	HE_DUAL_BAND = 1,
	HE_DEV_CLASS_A = (1 << 1),
	HE_LDPC = (1 << 2),
	HE_DOPPLER_TX = (1 << 3),
	HE_DOPPLER_RX = (1 << 4),
	HE_FULL_BW_UL_MU_MIMO = (1 << 5),
	HE_PARTIAL_BW_UL_MU_MIMO = (1 << 6),
	HE_DCM_MAX_NSS_TX = (1 << 7),
	HE_DCM_MAX_NSS_RX = (1 << 8),
	HE_RX_MU_PPDU_FROM_STA = (1 << 9),
	HE_TRIG_CQI_FEEDBACK = (1 << 10),
	HE_PARTIAL_BW_ER = (1 << 11),
	HE_PARTIAL_BW_DL_MU_MIMO = (1 << 12),
	HE_PPE_THRESHOLD_PRESENT = (1 << 13),
	HE_SRP_BASED_SR = (1 << 14),
	HE_PWR_BOOST_FACTOR = (1 << 15),
	HE_24G_20M_IN_40M_PPDU = (1 << 16),
	HE_20M_IN_160M_8080M_PPDU = (1 << 17),
	HE_80M_IN_160M_8080M_PPDU = (1 << 18),
	HE_MID_RX_2x_AND_1x_LTF = (1 << 19),
	HE_TX_1024QAM_UNDER_RU242 = (1 << 20),
	HE_RX_1024QAM_UNDER_RU242 = (1 << 21)
};

struct he_bf_info {
	enum he_bf_caps bf_cap;
	UINT8 bfee_sts_le_eq_bw80;
	UINT8 bfee_sts_gt_bw80;
	UINT8 snd_dim_le_eq_bw80;
	UINT8 snd_dim_gt_bw80;
	UINT8 bfee_max_nc;
};

struct he_mcs_info {
	UINT8 bw80_mcs[HE_MAX_SUPPORT_STREAM];
	UINT8 bw8080_mcs[HE_MAX_SUPPORT_STREAM];
	UINT8 bw160_mcs[HE_MAX_SUPPORT_STREAM];
};

struct he_bss_info {
	UINT8 default_pe_dur;
	UINT8 vht_oper_info_present;
	UINT16 txop_dur_rts_thr;
	struct he_mcs_info max_nss_mcs;
};

struct he_pe_thld {
	UINT8 ppet16;
	UINT8 ppet8;
};

struct he_pe_info {
	UINT8 nss_m1;
	UINT8 ru_num;
	UINT8 *pe_thld;
};

struct he_sta_phy_info {
	UINT8 dual_band_support;
	UINT8 ch_width_set;
	UINT8 bw20_242tone;
	UINT8 punctured_preamble_rx;
	UINT8 device_class;
	UINT8 ldpc_support;
	UINT8 stbc_support;
	UINT8 gi_cap;
	UINT8 dcm_cap_tx;
	UINT8 dcm_max_nss_tx;
	UINT8 dcm_cap_rx;
	UINT8 dcm_max_nss_rx;
	UINT8 dcm_max_ru;
	UINT8 tx_le_ru242_1024qam;
	UINT8 rx_le_ru242_1024qam;
	UINT8 triggered_cqi_feedback_support;
	UINT8 partial_bw_ext_range_support;
};

struct he_sta_mac_info{
	UINT8 htc_support;
	UINT8 bqr_support;
	UINT8 bsr_support;
	UINT8 om_support;
	UINT8 amsdu_in_ampdu_support;
	UINT8 max_ampdu_len_exp;
	UINT8 trigger_frame_mac_pad_dur;
	UINT8 he_dyn_smps;
};

struct he_sta_info {
	struct he_sta_mac_info mac_info;
	struct he_sta_phy_info phy_info;
	struct he_mcs_info max_nss_mcs;
};

struct he_ies {
	struct he_cap_ie he_caps;
	struct he_txrx_mcs_nss mcs_nss_160;
	struct he_txrx_mcs_nss mcs_nss_8080;
	struct he_op_ie he_ops;
	struct he_sr_ie he_sr_ies;
	struct vht_opinfo he_vht_opinfo;
	UINT8 sr_control;
	struct ul_ofdma_random_access uora;
	struct mu_edca_params he_mu_edca;
};

struct he_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

#define INTRA_HE_BSS_COLOR_MASK 0x3F
#define INTRA_HE_PARTIAL_BSS_COLOR_SHIFT 6
#define INTRA_HE_PARTIAL_BSS_COLOR (1 << 6)
#define INTRA_HE_BSS_COLOR_DIS_SHIFT 7
#define INTRA_HE_BSS_COLOR_DIS (1 << 7)

#define IS_STA_SUPPORT_TWT(entry)\
	((entry->cap.he_mac_cap & HE_TWT_REQUEST) ? TRUE : FALSE)
#define IS_STA_SUPPORT_BTWT(entry)\
	((entry->cap.he_mac_cap & HE_BROADCAST_TWT) ? TRUE : FALSE)

#define GET_BSS_COLOR(info)\
	((info) & INTRA_HE_BSS_COLOR_MASK)
#define IS_PARTIAL_BSS_COLOR(info)\
	(((info) & INTRA_HE_PARTIAL_BSS_COLOR) != 0)
#define IS_BSS_COLOR_DIS(info)\
	(((info) & INTRA_HE_BSS_COLOR_DIS) != 0)

#define IS_HE_STA(mode)\
	((mode) & (HE_24G_SUPPORT | HE_5G_SUPPORT | HE_6G_SUPPORT))
#define IS_HE_2G_STA(mode)\
	(((mode) & HE_24G_SUPPORT) == HE_24G_SUPPORT)
#define IS_HE_5G_STA(mode)\
	(((mode) & HE_5G_SUPPORT) == HE_5G_SUPPORT)
#define IS_HE_6G_STA(mode)\
	(((mode) & HE_6G_SUPPORT) == HE_6G_SUPPORT)

/*build*/
INT build_he_txpwr_envelope(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_beacon_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len);
UINT32 add_probe_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len);
UINT32 add_assoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len);
UINT32 add_reassoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len);
UINT32 add_probe_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_assoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_reassoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf);
UINT32 add_fils_tpe_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len);
#ifdef CONFIG_6G_SUPPORT
UINT32 add_he_6g_rnr_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len, UINT32 queried_s_ssid);
#endif
UINT32 build_he_btwt_ie(struct wifi_dev *wdev, PUCHAR pFrameBuf, ULONG FrameLen);
/*parse*/
UINT32 parse_he_beacon_probe_rsp_ies(UINT8 *ie_head, VOID *ie_list);
UINT32 parse_he_assoc_rsp_ies(UINT8 *ie_head, VOID *ie_list);
UINT32 parse_he_probe_req_ies(UINT8 *ie_head, VOID *ie_list);
UINT32 parse_he_assoc_req_ies(UINT8 *ie_head, VOID *ie_list);
VOID parse_he_bss_color_info(struct wifi_dev *wdev, VOID *le_list);

/*decision*/
VOID he_mode_adjust(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *peer, UCHAR *bw_from_opclass);
VOID update_peer_he_caps(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies);
VOID update_peer_he_operation(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies);

/*debug*/
VOID dump_he_ies(UCHAR *str, UINT8 *buf, UINT32 buf_len);

/*wifi sys related*/
UINT32 starec_he_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);
struct _STA_REC_CTRL_T;
VOID fill_starec_he(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, struct _STA_REC_CTRL_T *sta_rec);
UINT32 bssinfo_he_feature_decision(struct wifi_dev *wdev, UINT32 *feature);
VOID fill_bssinfo_he(struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo);

VOID init_default_ppe(struct he_pe_info *pe_info, UINT8 max_nss, UINT8 max_ru_num);
/*misc*/
VOID get_own_he_ie(struct wifi_dev *wdev, struct he_ies *he_ie);
VOID update_peer_he_params(struct _MAC_TABLE_ENTRY *peer, struct he_ies *he_ie);
#ifdef DOT11_HE_AX
BOOLEAN update_peer_he_sr_ies(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies);
BOOLEAN update_peer_he_muedca_ies(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies);
VOID he_mac_cap_af_decision(struct wifi_dev *wdev, UINT32 mac_capinfo_1);
#endif
UCHAR he_cent_ch_freq(UCHAR prim_ch, UCHAR he_bw, UCHAR ch_band);
#endif /*_HE_H_*/
