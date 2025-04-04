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

#ifndef __BE_EXPORT_H__
#define __BE_EXPORT_H__

#include "rtmp_def.h"
#include "he.h"

struct _RTMP_ADAPTER;

enum {
	WLAN_OPER_STATE_INVALID = 0,
	WLAN_OPER_STATE_VALID,
};

struct freq_cfg {
	UCHAR ch_band;
	UCHAR ht_bw;
	UCHAR vht_bw;
	UCHAR ext_cha;
	UCHAR prim_ch;
	UCHAR cen_ch_2;
	UCHAR ap_bw;
	UCHAR ap_cen_ch;
	UCHAR rx_stream;
};

struct freq_oper {
	UCHAR ch_band;
	UCHAR ht_bw;
	UCHAR vht_bw;
	UCHAR bw;
	UCHAR ext_cha;
	UCHAR prim_ch;
	UCHAR cen_ch_1;
	UCHAR cen_ch_2;
	UCHAR rx_stream;
	UCHAR ap_bw;
	UCHAR ap_cen_ch;
};

/*basic info obj */
/*
struct phy_cfg{
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
};

struct phy_op {
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR wdev_bw;
	UCHAR central1;
	UCHAR central2;
}

struct ht_info {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	oper_mode;
	UCHAR	mcs_set[16];
	BOOLEAN ht_en;
	BOOLEAN pre_nht_en;
	BOOLEAN gf;
	BOOLEAN	sgi_20;
	BOOLEAN sgi_40;
	BOOLEAN bss_coexist2040;
	BOOLEAN ldpc;
	BOOLEAN itx_bf;
	BOOLEAN etx_bf;
	BOOLEAN tx_stbc;
	BOOLEAN rx_stbc;
	struct ba_cap ba_cap;
};

struct ht_op_status {
	BOOLEAN obss_non_ht_exist;
	BOOLEAN non_gf_present;
	UCHAR	central_ch;
	HT_CAPABILITY_IE ht_cap;
	ADD_HT_INFO_IE addht;
	UINT16	non_gf_sta;
};

struct ba_cap {
	UCHAR mm_ps_mode;
	UCHAR amsdu_size;
	UCHAR mpdu_density;
	UCHAR policy;
	UCHAR tx_ba_win_limit;
	UCHAR rx_ba_win_limit;
	UCHAR max_ra_mpdu_factor;
	BOOLEAN amsdu_en;
	BOOLEAN auto_ba;
};

struct vht_info {
	BOOLEAN vht_en;
	BOOLEAN force_vht;
	UCHAR vht_bw;
	UCHAR vht_sgi;
	UCHAR vht_stbc;
	UCHAR vht_bw_signal;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN vht_ldpc;
	BOOLEAN g_band_256_qam;
};

struct vht_op_status{
};
*/

struct phy_op {
	UCHAR ch_band;
	UCHAR prim_ch;
	/*private attribute*/
	UCHAR wdev_bw;
	UCHAR cen_ch_1;
	UCHAR cen_ch_2;
	UINT8 tx_stream;
	UINT8 rx_stream;
};

struct ht_op {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	ht_stbc;
	UCHAR	ht_ldpc;
	UCHAR	ht_gi;
	UINT32	frag_thld;
	UCHAR	pkt_thld;
	UINT32	len_thld;
	UCHAR	l_sig_txop;
};

struct ht_op_status {
	/* Useful as AP. */
	ADD_HT_INFO_IE addht;
	HT_CAPABILITY_IE ht_cap_ie;
	/* counters */
	UINT16	non_gf_sta;
};

struct vht_op {
	UCHAR vht_bw;
	UCHAR vht_stbc;
	UCHAR vht_ldpc;
	UCHAR vht_sgi;
	UCHAR vht_bw_sig;
	UCHAR max_mpdu_len;
	UCHAR max_ampdu_exp;
};

struct he_ap6g {
	UINT8 qos_tx_tu;
	UINT8 qos_tx_state;
};

struct he_op {
	UINT8 bss_color;
	UINT8 partial_bss_color;
	UINT8 bss_color_dis;
	UINT8 twt_support;
	UINT8 next_bss_color;
	UINT8 bcc_count; /* count down of BSS Color Change Announcement IE */
	UINT32 txop_dur_rts_thld;
	UINT8 he_af;
	UINT8 bw;
	struct he_ap6g ap6g;
};


struct dev_rate_info {
	struct legacy_rate legacy_rate;
	/* OID_802_11_DESIRED_RATES */
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR MaxDesiredRate;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MaxTxRate;
	/* Tx rate index in Rate Switch Table */
	UCHAR TxRateIndex;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MinTxRate;
	/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR TxRate;
	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	HTTRANSMIT_SETTING MlmeTransmit;
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	MCAST_PKT_TYPE		McastType;
	HTTRANSMIT_SETTING  MCastPhyMode;
	HTTRANSMIT_SETTING  MCastPhyMode_5G;
#else
	HTTRANSMIT_SETTING mcastphymode;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
#ifdef HIGHPRI_RATE_SPECIFIC
	HTTRANSMIT_SETTING	HighPriPhyMode[HIGHPRI_MAX_TYPE];
	HTTRANSMIT_SETTING	HighPriPhyMode_5G[HIGHPRI_MAX_TYPE];
#endif /* MCAST_RATE_SPECIFIC */
};


#ifdef CONFIG_RA_PHY_RATE_SUPPORT
struct dev_eap_info {
	struct legacy_rate eap_legacy_rate;
	BOOLEAN eap_suprate_en;
	UCHAR eapsupportratemode;
	UINT8 eapsupportcckmcs;
	UINT8 eapsupportofdmmcs;
	BOOLEAN eap_htsuprate_en;
	UCHAR eapmcsset[16];
	UINT32 eapsupporthtmcs;
	BOOLEAN eap_vhtsuprate_en;
	struct _VHT_MCS_MAP rx_mcs_map;
	struct _VHT_MCS_MAP tx_mcs_map;
	BOOLEAN eap_hesuprate_en;
	struct rate_caps rate;
	BOOLEAN eap_mgmrate_en;
	BOOLEAN eap_bcnrate_en;
	HTTRANSMIT_SETTING mgmphymode;
	HTTRANSMIT_SETTING bcnphymode;
};
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#define WLAN_OPER_OK	(0)
#define WLAN_OPER_FAIL	(-1)

/*
* Operate GET
*/
struct _ADD_HT_INFO_IE *wlan_operate_get_addht(struct wifi_dev *wdev);
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_stbc(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_ldpc(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ext_cha(struct wifi_dev *wdev);
UINT16 wlan_operate_get_non_gf_sta(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ch_band(struct wifi_dev *wdev);
UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev);
UCHAR wlan_operate_get_cen_ch_2(struct wifi_dev *wdev);
UCHAR wlan_operate_get_cen_ch_1(struct wifi_dev *wdev);
UINT32 wlan_operate_get_frag_thld(struct wifi_dev *wdev);
UCHAR wlan_operate_get_rts_pkt_thld(struct wifi_dev *wdev);
UINT32 wlan_operate_get_rts_len_thld(struct wifi_dev *wdev);
UINT8 wlan_operate_get_tx_stream(struct wifi_dev *wdev);
UINT8 wlan_operate_get_rx_stream(struct wifi_dev *wdev);
VOID *wlan_operate_get_ht_cap(struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
UCHAR wlan_operate_get_vht_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_vht_ldpc(struct wifi_dev *wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
UINT8 wlan_operate_get_he_intra_bss_info(struct wifi_dev *wdev);
INT32 wlan_operate_get_he_bss_next_color(struct wifi_dev *wdev, UINT8 *color, UINT8 *countdown);
UINT16 wlan_operate_get_he_txop_dur_rts_thld(struct wifi_dev *wdev);
UINT8 wlan_operate_get_he_af(struct wifi_dev *wdev);
UINT8 wlan_operate_get_he_bw(struct wifi_dev *wdev);
UINT8 wlan_operate_get_he_6g_qos_state(struct wifi_dev *wdev);
UINT8 wlan_operate_get_he_6g_qos_tu(struct wifi_dev *wdev);

#endif /*DOT11_HE_AX*/

VOID dump_ht_cap(struct wifi_dev *wdev);
/*
* Operate Set
*/

INT32 wlan_operate_set_support_ch_width_set(struct wifi_dev *wdev, UCHAR ch_width_set);
INT32 wlan_operate_set_ht_bw(struct wifi_dev *wdev, UCHAR ht_bw, UCHAR ext_cha);
INT32 wlan_operate_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc);
INT32 wlan_operate_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc);
INT32 wlan_operate_loader_greenfield(struct wifi_dev *wdev, UCHAR ht_gf);
INT32 wlan_operate_set_non_gf_sta(struct wifi_dev *wdev, UINT16 non_gf_sta);
INT32 wlan_operate_set_max_amsdu_len(struct wifi_dev *wdev, UCHAR len);
INT32 wlan_operate_set_ch_band(struct wifi_dev *wdev, UCHAR ch_band);
INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch);
INT32 wlan_operate_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2);
INT32 wlan_operate_set_phy(struct wifi_dev *wdev, struct freq_cfg *cfg);
INT32 wlan_operate_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);
INT32 wlan_operate_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_num);
INT32 wlan_operate_set_rts_len_thld(struct wifi_dev *wdev, UINT32 pkt_len);
INT32 wlan_operate_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream);
INT32 wlan_operate_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream);
INT32 wlan_operate_set_min_start_space(struct wifi_dev *wdev, UCHAR mpdu_density);
INT32 wlan_operate_set_mmps(struct wifi_dev *wdev, UCHAR mmps);
INT32 wlan_operate_set_ht_max_ampdu_len_exp(struct wifi_dev *wdev, UCHAR exp_factor);
INT32 wlan_operate_set_ht_delayed_ba(struct wifi_dev *wdev, UCHAR support);
INT32 wlan_operate_set_lsig_txop_protect(struct wifi_dev *wdev, UCHAR support);
INT32 wlan_operate_set_psmp(struct wifi_dev *wdev, UCHAR psmp);
#ifdef DOT11_VHT_AC
INT32 wlan_operate_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw);
INT32 wlan_operate_set_vht_ldpc(struct wifi_dev *wdev, UCHAR vht_ldpc);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
INT32 wlan_operate_set_he_bss_color(struct wifi_dev *wdev, UINT8 bss_color, UINT8 bss_color_dis);
INT32 wlan_operate_set_he_bss_next_color(struct wifi_dev *wdev, UINT8 color, UINT8 countdown);
INT32 wlan_operate_set_he_partial_bss_color(struct wifi_dev *wdev, UINT8 partial_bss_color);
INT32 wlan_operate_set_he_txop_dur_rts_thld(struct wifi_dev *wdev, UINT16 txop_dur_thld);
INT32 wlan_operate_set_he_af(struct wifi_dev *wdev, UINT8 he_af);
INT32 wlan_operate_set_he_6g_qos_state(struct wifi_dev *wdev, UINT8 state);
INT32 wlan_operate_set_he_6g_qos_tu(struct wifi_dev *wdev, UINT8 tu);
#endif
/*
 * Operate Update
 */
VOID wlan_operate_update_ht_stbc(struct wifi_dev *wdev, UCHAR use_stbc);
VOID wlan_operate_update_ht_cap(struct wifi_dev *wdev);
/*
 *
 */
VOID wlan_operate_init(struct wifi_dev *wdev);
VOID wlan_operate_exit(struct wifi_dev *wdev);
UCHAR wlan_operate_set_state(struct wifi_dev *wdev, UCHAR state);
UCHAR wlan_operate_get_state(struct wifi_dev *wdev);
BOOLEAN wlan_operate_scan(struct wifi_dev *wdev, UCHAR prim_ch);
#endif
