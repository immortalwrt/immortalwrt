#ifndef _WFA_CAPI_
#define _WFA_CAPI_

struct capi_set_cmd {
	UCHAR *capi;

	VOID(*set_func)(VOID *ad, VOID *arg);
};

struct capi_get_cmd {
	UCHAR *capi;

	VOID(*get_func)(VOID *ad, UCHAR *arg);
};

/* dev_send_frame */
INT32 dev_send_frame(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID dev_send_frame_smpdu(VOID *ad, VOID *arg);
VOID dev_send_frame_uncast_deauth(VOID *ad, VOID *arg);
VOID dev_send_frame_non_support(VOID *ad, VOID *arg);

/* ap_set_wireless */
VOID ap_set_wireless_fixed_mcs(VOID *ad, VOID *param);
VOID ap_set_wireless_ofdma_direction(VOID *ad, VOID *param);
VOID ap_set_wireless_ppdu_tx_type(VOID *ad, VOID *param);
VOID ap_set_wireless_mumimo(VOID *ad, VOID *param);
VOID ap_set_wireless_nusers_ofdma(VOID *ad, VOID *param);
VOID ap_set_wireless_mu_edca_override(VOID *ad, VOID *param);
VOID ap_set_wireless_non_txbss_idx(VOID *ad, VOID *param);
VOID ap_set_wireless_non_support(VOID *ad, VOID *param);
VOID ap_set_wireless_act_ind_unsolicited_probe_rsp(VOID *ad, VOID *param);
VOID ap_set_wireless_discovery_apply(VOID *ad, VOID *param);
/* export APIs */
VOID ap_set_wireless_sta_configs(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry);
VOID ap_set_wireless_bss_configs(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);

/* ap_set_rfeature */
INT32  set_ap_rfeatures(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
VOID ap_set_he_fixed_gi_ltf_by_wcid_or_bss(RTMP_ADAPTER *pAd, UINT8 gi, UINT8 mode, UINT32 wcid, struct wifi_dev *wdev);
#endif

VOID ap_set_rfeature_ack_type(VOID *ad, VOID *param);
VOID ap_set_rfeature_ack_policy(VOID *ad, VOID *param);
VOID ap_set_rfeature_ppdu_tx_type(VOID *ad, VOID *param);
VOID ap_set_rfeature_trig_type(VOID *ad, VOID *param);
VOID ap_set_rfeature_ignore_nav(VOID *ad, VOID *param);
VOID ap_set_rfeature_he_ltf(VOID *ad, VOID *param);
VOID ap_set_rfeature_he_gi(VOID *ad, VOID *param);
VOID ap_set_rfeature_trig_txbf(VOID *ad, VOID *param);
VOID ap_set_rfeature_dis_trig_type(VOID *ad, VOID *param);
VOID ap_set_rfeature_ofdma_direction(VOID *ad, VOID *param);
VOID ap_set_rfeature_chnum_band(VOID *ad, VOID *param);
VOID ap_set_rfeature_tx_bandwidth(VOID *ad, VOID *param);
VOID ap_set_rfeature_non_support(VOID *ad, VOID *param);
VOID ap_set_rfeature_unsolicited_probe_rsp(VOID *ad, VOID *param);
VOID ap_set_rfeature_fils_discovery(VOID *ad, VOID *param);
VOID ap_set_rfeature_qos_null_injector(VOID *ad, VOID *param);
VOID ap_set_rfeature_bss_max_idle_ie(VOID *ad, VOID *param);
VOID ap_set_rfeature_bss_max_idle_period(VOID *ad, VOID *param);
VOID ap_set_rfeature_bss_max_idle_option(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_min_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_min_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_min_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_min_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_max_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_max_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_max_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_ecw_max_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_aifsn_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_aifsn_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_aifsn_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_aifsn_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_timer_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_timer_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_timer_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_mu_edca_timer_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_min_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_min_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_min_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_min_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_max_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_max_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_max_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_ecw_max_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_aifsn_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_aifsn_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_aifsn_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_aifsn_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_txop_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_txop_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_txop_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_wmm_pe_txop_bk(VOID *ad, VOID *param);
VOID ap_set_rfeature_ap_wmm_pe_txop_vo(VOID *ad, VOID *param);
VOID ap_set_rfeature_ap_wmm_pe_txop_vi(VOID *ad, VOID *param);
VOID ap_set_rfeature_ap_wmm_pe_txop_be(VOID *ad, VOID *param);
VOID ap_set_rfeature_ap_wmm_pe_txop_bk(VOID *ad, VOID *param);

#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
/* sta_set_wireless */
INT32 set_sta_wireless(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID sta_set_wireless_addbareq_bufsize(VOID *ad, VOID *param);
VOID sta_set_wireless_addbaresp_bufsize(VOID *ad, VOID *param);
VOID sta_set_wireless_bcc_ldpc(VOID *ad, VOID *param);
VOID sta_set_wireless_fixed_mcs_ie(VOID *ad, VOID *param);
VOID sta_set_wireless_fixed_mcs_run_time(VOID *ad, VOID *param);
VOID sta_set_wireless_rxsp_stream(VOID *ad, VOID *param);
VOID sta_set_wireless_txsp_stream(VOID *ad, VOID *param);
VOID sta_set_wireless_bandwidth(VOID *ad, VOID *param);
VOID sta_set_wireless_sta_configs(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry);

/* export APIs */
/* sta_set_rfeature */
INT32  set_sta_rfeatures(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
VOID sta_set_rfeature_txsuppdu(VOID *ad, VOID *param);
VOID sta_set_rfeature_twt(VOID *ad, VOID *param);
VOID sta_set_rfeature_ltf(VOID *ad, VOID *param);
VOID sta_set_rfeature_gi(VOID *ad, VOID *param);
#endif /* defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

/* all station tag */
#define CAPI_ALL_STA 0xFFFF
#define CAPI_APCLI_STA 0x1

/* LTF */
#define BW20_HE_LTF_SHIFT 0
#define BW20_HE_LTF_MASK (LTF_MASK << BW20_HE_LTF_SHIFT)
#define BW40_HE_LTF_SHIFT 2
#define BW40_HE_LTF_MASK (LTF_MASK << BW40_HE_LTF_SHIFT)
#define BW80_HE_LTF_SHIFT 4
#define BW80_HE_LTF_MASK (LTF_MASK << BW80_HE_LTF_SHIFT)
#define BW160_HE_LTF_SHIFT 6
#define BW160_HE_LTF_MASK (LTF_MASK << BW160_HE_LTF_SHIFT)

/* HT/VHT GI */
enum {
	GI_LONG,
	GI_SHORT
};
#define BW20_LEGACY_GI_SHIFT 0
#define BW40_LEGACY_GI_SHIFT 1
#define BW80_LEGACY_GI_SHIFT 2
#define BW160_LEGACY_GI_SHIFT 3

/* HE GI */
#define BW20_HE_GI_SHIFT 0
#define BW20_HE_GI_MASK (GI_MASK << BW20_HE_GI_SHIFT)
#define BW40_HE_GI_SHIFT 2
#define BW40_HE_GI_MASK (GI_MASK << BW40_HE_GI_SHIFT)
#define BW80_HE_GI_SHIFT 4
#define BW80_HE_GI_MASK (GI_MASK << BW80_HE_GI_SHIFT)
#define BW160_HE_GI_SHIFT 6
#define BW160_HE_GI_MASK (GI_MASK << BW160_HE_GI_SHIFT)

/* HE GI */
enum {
	CAPI_HE_08_GI,
	CAPI_HE_16_GI,
	CAPI_HE_32_GI,
	CAPI_HE_GI_NUM
};

/* HE LTF */
enum {
	CAPI_HE_1x_LTF,
	CAPI_HE_2x_LTF,
	CAPI_HE_4x_LTF,
	CAPI_HE_LTF_NUM
};

/* trigger type */
enum {
	CAPI_BASIC,
	CAPI_BRP,/* Beaforming Report Poll */
	CAPI_MU_BAR,
	CAPI_MU_RTS,
	CAPI_BSRP,
	CAPI_GCR_MU_BAR,
	CAPI_BQRP,
	CAPI_NDP_FRP /* NDP Feedback Report Poll  */
};

/* Ack Policy */
enum {
	CAPI_NORMAL_ACK,
	CAPI_NO_ACK,
	CAPI_IMPLICIT_ACK,
	CAPI_BLOCK_ACK,
	CAPI_HTP_ACK
};

/* Ack Type */
enum {
	CAPI_M_BA,
	CAPI_C_BA
};

/* PPDU Tx Type */
enum {
	CAPI_SU,
	CAPI_MU,
	CAPI_ER_SU,
	CAPI_TB,
	CAPI_LEGACY
};

/* OFDMA Direction */
enum {
	CAPI_OFDMA_AUTO,
	CAPI_OFDMA_DL,
	CAPI_OFDMA_UL,
	CAPI_OFDMA_DL_20n80
};

/* Program */
enum {
	CAPI_Disable,
	CAPI_Enable,
	CAPI_TGn,
	CAPI_TGac,
	CAPI_TGax,
	CAPI_PMF,
	CAPI_WPS,
	CAPI_WMMPS,
	CAPI_Passpoint_R3,
	CAPI_MBO,
	CAPI_OCE,
	CAPI_MAP,
	CAPI_WPA3,
};


/* MCS */
enum {
	CAPI_MCS_0,
	CAPI_MCS_1,
	CAPI_MCS_2,
	CAPI_MCS_3,
	CAPI_MCS_4,
	CAPI_MCS_5,
	CAPI_MCS_6,
	CAPI_MCS_7,
	CAPI_MCS_8,
	CAPI_MCS_9,
	CAPI_MCS_10,
	CAPI_MCS_11,
	CAPI_MCS_12,
	CAPI_MCS_13,
	CAPI_MCS_14,
	CAPI_MCS_15,
	CAPI_MCS_16,
	CAPI_MCS_17,
	CAPI_MCS_18,
	CAPI_MCS_19,
	CAPI_MCS_20,
	CAPI_MCS_21,
	CAPI_MCS_22,
	CAPI_MCS_23,
	CAPI_MCS_24,
	CAPI_MCS_25,
	CAPI_MCS_26,
	CAPI_MCS_27,
	CAPI_MCS_28,
	CAPI_MCS_29,
	CAPI_MCS_30,
	CAPI_MCS_31,
	CAPI_MCS_32,
	CAPI_MCS_AUTO
};

/* Disable/Enable MU */
#define MUMIMO_FORCE_DISABLE_MU      0
#define MUMIMO_FORCE_ENABLE_MU       1

#endif /* _WFA_CAPI_ */
