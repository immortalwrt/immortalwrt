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

#ifndef __CONFIG_EXPORT_H__
#define __CONFIG_EXPORT_H__

struct _RTMP_ADAPTER;

struct wpf_data {
	UCHAR idx;
	void *dev;
	void *conf;
	void *oper;
};

struct wpf_ctrl {
	struct wpf_data pf[WDEV_NUM_MAX];
};

struct phy_cfg {
	UINT8 ch_band;
	UINT8 ap_bw;
	UINT8 ap_cen_ch;
	UINT8 tx_stream;
	UINT8 rx_stream;
	UCHAR cen_ch_2;
	UCHAR ack_policy[WMM_NUM_OF_AC];
#ifdef TXBF_SUPPORT
	UCHAR ETxBfEnCond;
	UCHAR ITxBfEn;
#endif /* TXBF_SUPPORT */
	UCHAR mu_dl_ofdma;
	UCHAR mu_ul_ofdma;
	UCHAR mu_dl_mimo;
	UCHAR mu_ul_mimo;
	UCHAR fixed_mcs;
};

struct qos_info {
	UINT8 edca_parm_update_count;
	UINT8 q_ack;
	UINT8 que_req;
	UINT8 txop_req;
};

struct ba_config {
	UINT8 ba_decline;
	UINT8 ba_enable;
	UINT16 ba_tx_wsize;
	UINT16 ba_rx_wsize;
};

struct ht_cfg {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	ht_stbc;
	UCHAR	ht_ldpc;
	UCHAR	ht_gi;
	UCHAR	ht_protect_en;
	UCHAR	ht_mode; /* mix_mode or gf_mode */
	UCHAR	gf_support;
	UCHAR	ht40_intolerant;
	UINT32	frag_thld;
	UCHAR	pkt_thld;
	UINT32	len_thld;
	UCHAR	min_mpdu_start_space;
	UCHAR	amsdu_en;
	UCHAR	mmps;
	struct ba_config ba_cfg;
	/* EDCA parameters to be announced to its local BSS */
	struct _EDCA_PARM EdcaParm;
};

struct vht_cfg {
	UCHAR	vht_bw;
	UCHAR	vht_stbc;
	UCHAR	vht_ldpc;
	UCHAR	vht_sgi; /* including both bw80 & bw160 */
	UCHAR	vht_bw_sig; /* 0:n/a, 1:static, 2:dynamic */
	UINT8	ext_nss_bw;
	UINT8	max_mpdu_len; /*0:3895, 1:7991, 2:11454*/
};

struct mu_ac_param {
	UINT8 acm;
	UINT8 aifsn;
	UINT8 ecw_min;
	UINT8 ecw_max;
	UINT8 mu_edca_timer;
};

struct mu_edca_cfg {
	struct mu_ac_param mu_ac_rec[ACI_AC_NUM];
};

struct he_cfg {
	UINT8 bw;
	UINT8 tx_stbc;
	UINT8 rx_stbc;
	UINT8 ldpc;
	UINT8 ltf;
	UINT8 gi;
	UINT8 he_vhtop;
	UINT8 tx_nss;
	UINT8 rx_nss;
	UINT16 txop_duration;
	UINT8 twt_support;
	UINT8 uora;
	UINT8 sr;
	UINT8 ppdu_tx_type;
	UINT8 ofdma_usr_num;
	UINT8 mu_edca_override; /*set MU CWmin,max, mu edca timer 0xf */
	UINT8 non_tx_bss_idx;
	UINT8 ofdma_dir;
	struct mu_edca_cfg mu_edca_param_set;
};


/*for profile usage*/
VOID wpf_init(struct _RTMP_ADAPTER *ad);
VOID wpf_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_init(struct _RTMP_ADAPTER *ad);

UCHAR chip_get_max_nss(struct _RTMP_ADAPTER *ad);

/*
* Configure Get
*/
UCHAR wlan_config_get_ht_bw(struct wifi_dev *wdev);
UINT8 wlan_config_get_tx_stream(struct wifi_dev *wdev);
UINT8 wlan_config_get_rx_stream(struct wifi_dev *wdev);
UINT8 wlan_config_get_fixed_mcs(struct wifi_dev *wdev);
#ifdef TXBF_SUPPORT
UCHAR wlan_config_get_etxbf(struct wifi_dev *wdev);
UCHAR wlan_config_get_itxbf(struct wifi_dev *wdev);
#endif /* TXBF_SUPPORT */
UCHAR wlan_config_get_ht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_ldpc(struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
UCHAR wlan_config_get_vht_bw(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_ldpc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_sgi(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_bw_sig(struct wifi_dev *wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
UCHAR wlan_config_get_he_bw(struct wifi_dev *wdev);
UCHAR wlan_config_get_he_vhtop_present(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_tx_stbc(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_rx_stbc(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_ldpc(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_tx_nss(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_rx_nss(struct wifi_dev *wdev);
UINT16 wlan_config_get_he_txop_dur_rts_thld(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_intra_bss_info(struct wifi_dev *wdev);
UINT8 wlan_config_get_he_twt_support(struct wifi_dev *wdev);
UINT8 wlan_config_get_ppdu_tx_type(struct wifi_dev *wdev);
UINT8 wlan_config_get_ofdma_user_cnt(struct wifi_dev *wdev);
UINT8 wlan_config_get_mu_edca_override(struct wifi_dev *wdev);
UINT8 wlan_config_get_non_tx_bss_idx(struct wifi_dev *wdev);
UINT8 wlan_config_get_ofdma_direction(struct wifi_dev *wdev);
struct mu_edca_cfg *wlan_config_get_he_mu_edca(struct wifi_dev *wdev);
UINT8 wlan_config_get_mu_dl_ofdma(struct wifi_dev *wdev);
UINT8 wlan_config_get_mu_ul_ofdma(struct wifi_dev *wdev);
UINT8 wlan_config_get_mu_dl_mimo(struct wifi_dev *wdev);
UINT8 wlan_config_get_mu_ul_mimo(struct wifi_dev *wdev);
#endif /* DOT11_HE_AX */
UCHAR wlan_config_get_ch_band(struct wifi_dev *wdev);
UCHAR wlan_config_get_ext_cha(struct wifi_dev *wdev);
UCHAR wlan_config_get_cen_ch_2(struct wifi_dev *wdev);
UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id);
BOOLEAN wlan_config_get_edca_valid(struct wifi_dev *wdev);
struct _EDCA_PARM *wlan_config_get_ht_edca(struct wifi_dev *wdev);
UINT32 wlan_config_get_frag_thld(struct wifi_dev *wdev);
UINT32 wlan_config_get_rts_len_thld(struct wifi_dev *wdev);
UCHAR wlan_config_get_rts_pkt_thld(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_gi(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_protect_en(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_mode(struct wifi_dev *wdev);
UCHAR wlan_config_get_greenfield(struct wifi_dev *wdev);
UCHAR wlan_config_get_40M_intolerant(struct wifi_dev *wdev);
UCHAR wlan_config_get_amsdu_en(struct wifi_dev *wdev);
UCHAR wlan_config_get_min_mpdu_start_space(struct wifi_dev *wdev);
UCHAR wlan_config_get_mmps(struct wifi_dev *wdev);
UINT8 wlan_config_get_vht_ext_nss_bw(struct wifi_dev *wdev);
UINT8 wlan_config_get_vht_max_mpdu_len(struct wifi_dev *wdev);
/* get chip_caps */
enum ASIC_CAP wlan_config_get_asic_caps(struct wifi_dev *wdev);
enum PHY_CAP wlan_config_get_phy_caps(struct wifi_dev *wdev);
VOID *wlan_config_get_ppdu_caps(struct wifi_dev *wdev);
VOID *wlan_config_get_mcs_nss_caps(struct wifi_dev *wdev);
VOID *wlan_config_get_qos_caps(struct wifi_dev *wdev);
VOID *wlan_config_get_chip_caps(struct wifi_dev *wdev);
BOOLEAN wlan_config_get_asic_twt_caps(struct wifi_dev *wdev);
UINT8 wlan_config_get_ba_decline(struct wifi_dev *wdev);
UINT8 wlan_config_get_ba_enable(struct wifi_dev *wdev);
UINT16 wlan_config_get_ba_tx_wsize(struct wifi_dev *wdev);
UINT16 wlan_config_get_ba_rx_wsize(struct wifi_dev *wdev);

/*
* Configure Set
*/
VOID wlan_config_set_ht_bw(struct wifi_dev *wdev, UCHAR ht_bw);
VOID wlan_config_set_ht_bw_all(struct wpf_ctrl *ctrl, UCHAR ht_bw);
VOID wlan_config_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream);
VOID wlan_config_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream);
VOID wlan_config_set_fixed_mcs(struct wifi_dev *wdev, UINT8 fixed_mcs);
#ifdef TXBF_SUPPORT
VOID wlan_config_set_etxbf(struct wifi_dev *wdev, UCHAR ETxBfEnCond);
VOID wlan_config_set_itxbf(struct wifi_dev *wdev, UCHAR ITxBfEn);
#endif /* TXBF_SUPPORT */
VOID wlan_config_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc);
VOID wlan_config_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc);
VOID wlan_config_set_ht_mode(struct wifi_dev *wdev, UCHAR ht_mode);
VOID wlan_config_set_40M_intolerant(struct wifi_dev *wdev, UCHAR val);
#ifdef DOT11_VHT_AC
VOID wlan_config_set_vht_stbc(struct wifi_dev *wdev, UCHAR vht_stbc);
VOID wlan_config_set_vht_ldpc(struct wifi_dev *wdev, UCHAR vht_ldpc);
VOID wlan_config_set_vht_sgi(struct wifi_dev *wdev, UCHAR vht_sgi);
VOID wlan_config_set_vht_bw_sig(struct wifi_dev *wdev, UCHAR vht_bw_sig);
VOID wlan_config_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw);
VOID wlan_config_set_vht_bw_all(struct wpf_ctrl *ctrl, UCHAR vht_bw);
VOID wlan_config_set_vht_ext_nss_bw(struct wifi_dev *wdev, UINT8 ext_nss_bw);
VOID wlan_config_set_vht_max_mpdu_len(struct wifi_dev *wdev, UINT8 max_mpdu_len);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
VOID wlan_config_set_he_bw(struct wifi_dev *wdev, UINT8 he_bw);
VOID wlan_config_set_he_ldpc(struct wifi_dev *wdev, UINT8 he_ldpc);
VOID wlan_config_set_he_vhtop_present(struct wifi_dev *wdev, UINT8 vhtop_en);
VOID wlan_config_set_he_tx_nss(struct wifi_dev *wdev, UINT8 tx_nss);
VOID wlan_config_set_he_rx_nss(struct wifi_dev *wdev, UINT8 rx_nss);
VOID wlan_config_set_he_txop_dur_rts_thld(struct wifi_dev *wdev, UINT32 txop_dur_thld);
VOID wlan_config_set_he_twt_support(struct wifi_dev *wdev, UINT8 twt_support);
VOID wlan_config_set_ppdu_tx_type(struct wifi_dev *, UINT8);
VOID wlan_config_set_ofdma_user_cnt(struct wifi_dev *, UINT8);
VOID wlan_config_set_mu_edca_override(struct wifi_dev *, UINT8 mu_edca_override);
VOID wlan_config_set_non_tx_bss_idx(struct wifi_dev *, UINT8);
VOID wlan_config_set_ofdma_direction(struct wifi_dev *, UINT8);
VOID wlan_config_set_ap_bw(struct wifi_dev *wdev, UCHAR ap_bw);
VOID wlan_config_set_ap_cen(struct wifi_dev *wdev, UCHAR ap_cen_ch);
VOID wlan_config_set_mu_dl_ofdma(struct wifi_dev *wdev, UINT8 enable);
VOID wlan_config_set_mu_ul_ofdma(struct wifi_dev *wdev, UINT8 enable);
VOID wlan_config_set_mu_dl_mimo(struct wifi_dev *wdev, UINT8 enable);
VOID wlan_config_set_mu_ul_mimo(struct wifi_dev *wdev, UINT8 enable);
#endif /* DOT11_HE_AX */
VOID wlan_config_set_ht_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha);
VOID wlan_config_set_ht_ext_cha_all(struct wpf_ctrl *ctrl, UCHAR ext_cha);
VOID wlan_config_set_ch_band(struct wifi_dev *wdev, USHORT wmode);
VOID wlan_config_set_ch_band_all(struct wpf_ctrl *ctrl, USHORT wmode);
VOID wlan_config_set_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha);
VOID wlan_config_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2);
VOID wlan_config_set_cen_ch_2_all(struct wpf_ctrl *ctrl, UCHAR cen_ch_2);
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev, UCHAR *policy);
VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl, UCHAR *policy);
VOID wlan_config_set_edca_valid(struct wifi_dev *wdev, BOOLEAN bValid);
VOID wlan_config_set_edca_valid_all(struct wpf_ctrl *ctrl, BOOLEAN bValid);
VOID wlan_config_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);
VOID wlan_config_set_rts_len_thld(struct wifi_dev *wdev, UINT32 len_thld);
VOID wlan_config_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_thld);
VOID wlan_config_set_ht_gi(struct wifi_dev *wdev, UCHAR ht_git);
VOID wlan_config_set_ht_protect_en(struct wifi_dev *wdev, UCHAR ht_protect);
VOID wlan_config_set_amsdu_en(struct wifi_dev *wdev, UCHAR enable);
VOID wlan_config_set_min_mpdu_start_space(struct wifi_dev *wdev, UCHAR mpdu_density);
VOID wlan_config_set_mmps(struct wifi_dev *wdev, UCHAR mmps);
VOID wlan_config_set_ba_decline(struct wifi_dev *wdev, UINT8 decline);
VOID wlan_config_set_ba_enable(struct wifi_dev *wdev, UINT8 en);
VOID wlan_config_set_ba_txrx_wsize(struct wifi_dev *wdev, UINT16 tx_wsize, UINT16 rx_wsize);
#endif
