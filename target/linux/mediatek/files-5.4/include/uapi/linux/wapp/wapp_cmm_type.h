/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Mediatek Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wapp_cmm_type.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

/* This file is used by wifi driver and wapp.
   Keep data structure sync */

#ifndef __WAPP_TYPES_H__
#define __WAPP_TYPES_H__

//#include <linux/if_ether.h>
#ifdef WAPP_SUPPORT
#define MAX_BSSLOAD_THRD			100
#endif /* WAPP_SUPPORT */

#ifndef GNU_PACKED
#define GNU_PACKED  (__attribute__ ((packed)))
#endif /* GNU_PACKED */

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN				6
#endif
#ifndef LEN_PMK
#define LEN_PMK					32
#endif
#ifndef LEN_PMK_MAX
#define LEN_PMK_MAX				48
#endif
#ifndef LEN_PMKID
#define LEN_PMKID				16
#endif
#ifndef LEN_MAX_PTK
#define LEN_MAX_PTK				88
#endif
#ifndef LEN_PSK
#define LEN_PSK					64
#endif
#ifndef LEN_MAX_URI
#define LEN_MAX_URI                             120
#endif

#ifndef AC_NUM
#define AC_NUM						4
#endif
#define MAX_HE_MCS_LEN 12
#define MAX_OP_CLASS 16
#define MAX_LEN_OF_SSID 32
#define MAX_NUM_OF_CHANNELS		59 // 14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0@Japan + 1 as NULL termination
#define ASSOC_REQ_LEN 154
#define ASSOC_REQ_LEN_MAX 512
#define PREQ_IE_LEN 200
#define BCN_RPT_LEN 200
#define IWSC_MAX_SUB_MASK_LIST_COUNT	3
#define WMODE_CAP_N(_x)                        (((_x) & (WMODE_GN | WMODE_AN)) != 0)
#define WMODE_CAP_AC(_x)               (((_x) & (WMODE_AC)) != 0)
#define WMODE_CAP_AX(_x)	((_x) & (WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G))
#define WMODE_CAP(_x, _mode)   (((_x) & (_mode)) != 0)

#define MAX_SUPPORT_INF_NUM 17 * MAX_NUM_OF_RADIO /* 16MBSS+1APCLI */
#define MAX_NUM_OF_WAPP_CHANNELS 59
#define MAX_PROFILE_CNT 4
#define PER_EVENT_LIST_MAX_NUM 		5
#define	DAEMON_NEIGHBOR_REPORT_MAX_NUM 128
#define VERSION_WAPP_CMM "v3.0.2.0"
#ifdef MAP_R3_WF6
#define MAX_TID 4
#endif

/* If this value is passed during map set channel
 * then no need to parse that argument
 */
#define SET_CH_ARG_NOT_REQ 255

typedef enum {
	WAPP_STA_INVALID,
	WAPP_STA_DISCONNECTED,
	WAPP_STA_CONNECTED,
} WAPP_STA_STATE;

typedef enum {
	WAPP_BSS_STOP = 0,
	WAPP_BSS_START,
} WAPP_BSS_STATE;

typedef enum {
	WAPP_AUTH = 0,
	WAPP_ASSOC,
	WAPP_EAPOL
} WAPP_CNNCT_STAGE;

typedef enum {
	WAPP_BSSLOAD_NORMAL = 0,
	WAPP_BSSLOAD_HIGH,
	WAPP_BSSLOAD_LOW,
} WAPP_BSSLOAD_STATE;

typedef enum {
	NOT_FAILURE = 0,
	AP_NOT_READY,
	ACL_CHECK_FAIL,
	BSSID_NOT_FOUND,
	BSSID_MISMATCH,
	BSSID_IF_NOT_READY,
	BND_STRG_CONNECT_CHECK_FAIL,
	DISALLOW_NEW_ASSOCI,
	EZ_CONNECT_DISALLOW,
	EZ_SETUP_FUNC_DISABLED,
	FT_ERROR,
	GO_UPDATE_NOT_COMPLETE,
	MLME_NO_RESOURCE,
	MLME_ASSOC_REJ_TEMP,
	MLME_UNABLE_HANDLE_STA,
	MLME_EZ_CNNCT_LOOP,
	MLME_REQ_WITH_INVALID_PARAM,
	MLME_REJECT_TIMEOUT,
	MLME_UNSPECIFY_FAILURE,
	NOT_FOUND_IN_RADIUS_ACL,
	PEER_REQ_SANITY_FAIL,
} WAPP_CNNCT_FAIL_REASON_LIST;

typedef enum {
	WAPP_APCLI_DISASSOCIATED = 0,
	WAPP_APCLI_ASSOCIATED,
} WAPP_APCLI_ASSOC_STATE;

typedef enum {
	WAPP_DEV_QUERY_RSP = 1,
	WAPP_HT_CAP_QUERY_RSP,
	WAPP_VHT_CAP_QUERY_RSP,
	WAPP_HE_CAP_QUERY_RSP,
	WAPP_MISC_CAP_QUERY_RSP,
	WAPP_CLI_QUERY_RSP,
	WAPP_CLI_LIST_QUERY_RSP,
	WAPP_CLI_JOIN_EVENT,
	WAPP_CLI_LEAVE_EVENT,
	WAPP_CLI_PROBE_EVENT,
	WAPP_CHN_LIST_RSP,
	WAPP_OP_CLASS_RSP,
	WAPP_BSS_INFO_RSP,
	WAPP_AP_METRIC_RSP,
	WAPP_CH_UTIL_QUERY_RSP,
	WAPP_AP_CONFIG_RSP,
	WAPP_APCLI_QUERY_RSP,
	MAP_BH_STA_WPS_DONE,
	MAP_TRIGGER_RSSI_STEER,
	WAPP_RCEV_BCN_REPORT,
	WAPP_RCEV_BCN_REPORT_COMPLETE,
	WAPP_RCEV_MONITOR_INFO,
	WAPP_BSSLOAD_RSP,
	WAPP_BSSLOAD_CROSSING,
	WAPP_BSS_STATE_CHANGE,
	WAPP_CH_CHANGE,
	WAPP_TX_POWER_CHANGE,
	WAPP_APCLI_ASSOC_STATE_CHANGE,
	WAPP_STA_RSSI_RSP,
	WAPP_CLI_ACTIVE_CHANGE,
	WAPP_CSA_EVENT,
	WAPP_STA_CNNCT_REJ,
	WAPP_APCLI_RSSI_RSP,
	WAPP_SCAN_RESULT_RSP,
	WAPP_MAP_VENDOR_IE,
	WAPP_WSC_SCAN_COMP_NOTIF,
	WAPP_MAP_WSC_CONFIG,
	WAPP_WSC_EAPOL_START_NOTIF,
	WAPP_WSC_EAPOL_COMPLETE_NOTIF,
	WAPP_SCAN_COMPLETE_NOTIF,
	WAPP_A4_ENTRY_MISSING_NOTIF,
	WAPP_RADAR_DETECT_NOTIF,
	WAPP_APCLI_ASSOC_STATE_CHANGE_VENDOR10,
	WAPP_CAC_STOP, //MAP R2
	WAPP_STA_DISASSOC_EVENT,
	WAPP_RADIO_METRIC_RSP,
	WAPP_DPP_ACTION_FRAME_RECEIVED,
	WAPP_DPP_ACTION_FRAME_STATUS,
	WAPP_DPP_CCE_RSP,
	WAPP_CAC_PERIOD_EVENT,
	WAPP_UNSAFE_CHANNEL_EVENT,
	WAPP_BAND_STATUS_CHANGE_EVENT,
	WAPP_STA_INFO,
	WAPP_R3_RECONFIG_TRIGGER,
	WAPP_R3_DPP_URI_INFO,
	WAPP_NO_STA_CONNECT_TIMEOUT_EVENT,
	WAPP_NO_DATA_TRAFFIC_TIMEOUT_EVENT,
	WAPP_WIFI_UP_EVENT,
	WAPP_WIFI_DOWN_EVENT,
	WAPP_QOS_ACTION_FRAME_EVENT = 70,
	WAPP_MSCS_CLASSIFIER_PARAM_EVENT,
	WAPP_VEND_SPEC_UP_TUPLE_EVENT,
	WAPP_CH_CHANGE_R3,
	WAPP_SELF_SRG_BITMAP_EVENT,
	WAPP_UPLINK_TRAFFIC_EVENT,
	WAPP_CONFIG_WPS_EVENT,
} WAPP_EVENT_ID;

typedef enum {
	WAPP_DEV_QUERY_REQ = 1,
	WAPP_HT_CAP_QUERY_REQ,
	WAPP_VHT_CAP_QUERY_REQ,
	WAPP_HE_CAP_QUERY_REQ,
	WAPP_MISC_CAP_QUERY_REQ,
	WAPP_CLI_QUERY_REQ,
	WAPP_CLI_LIST_QUERY_REQ,
	WAPP_CHN_LIST_QUERY_REQ,
	WAPP_OP_CLASS_QUERY_REQ,
	WAPP_BSS_INFO_QUERY_REQ,
	WAPP_AP_METRIC_QUERY_REQ,
	WAPP_CH_UTIL_QUERY_REQ,
	WAPP_APCLI_QUERY_REQ,
	WAPP_BSS_START_REQ,
	WAPP_BSS_STOP_REQ,
	WAPP_TXPWR_PRCTG_REQ,
	WAPP_STEERING_POLICY_SET_REQ,
	WAPP_BSS_LOAD_THRD_SET_REQ,
	WAPP_AP_CONFIG_SET_REQ,
	WAPP_BSSLOAD_QUERY_REQ,
	WAPP_HECAP_QUERY_REQ,
	WAPP_STA_RSSI_QUERY_REQ,
	WAPP_APCLI_RSSI_QUERY_REQ,
	WAPP_GET_SCAN_RESULTS,
	WAPP_SEND_NULL_FRAMES,
	WAPP_WSC_PBC_EXEC,
	WAPP_WSC_SET_BH_PROFILE,
	WAPP_SET_SCAN_BH_SSIDS,
	WAPP_SET_AVOID_SCAN_CAC,
#ifdef MAP_R2
	WAPP_RADIO_METRICS_REQ,
#endif
#ifdef DPP_R2_SUPPORT
	WAPP_GET_CCE_RESULT,
#endif
	WAPP_SET_SRG_BITMAP,
	WAPP_SET_TOPOLOGY_UPDATE,
	WAPP_SET_SRG_UPLINK_STATUS,
} WAPP_REQ_ID;

typedef enum {
	PARAM_DGAF_DISABLED,
	PARAM_PROXY_ARP,
	PARAM_L2_FILTER,
	PARAM_ICMPV4_DENY,
	PARAM_MMPDU_SIZE,
	PARAM_EXTERNAL_ANQP_SERVER_TEST,
	PARAM_GAS_COME_BACK_DELAY,
	PARAM_WNM_NOTIFICATION,
	PARAM_QOSMAP,
	PARAM_WNM_BSS_TRANSITION_MANAGEMENT,
} WAPP_PARAM;

typedef struct GNU_PACKED _WAPP_CONNECT_FAILURE_REASON {
	u8	connect_stage;
	u16	reason;
} WAPP_CONNECT_FAILURE_REASON;

typedef struct GNU_PACKED _wapp_dev_info {
	u32	ifindex;
	u8	ifname[IFNAMSIZ];
	u8	mac_addr[MAC_ADDR_LEN];
	u8	dev_type;
	u8	radio_id;
	u16	wireless_mode;
	uintptr_t	adpt_id;
	u8 dev_active;
} wapp_dev_info;

typedef struct GNU_PACKED _wdev_ht_cap {
	u8	tx_stream;
	u8	rx_stream;
	u8	sgi_20;
	u8	sgi_40;
	u8	ht_40;
} wdev_ht_cap;

typedef struct GNU_PACKED _wdev_vht_cap {
	u8	sup_tx_mcs[2];
	u8	sup_rx_mcs[2];
	u8	tx_stream;
	u8	rx_stream;
	u8	sgi_80;
	u8	sgi_160;
	u8	vht_160;
	u8	vht_8080;
	u8	su_bf;
	u8	mu_bf;
} wdev_vht_cap;

typedef struct GNU_PACKED _wdev_he_cap {
	unsigned char he_mcs_len;
	unsigned char he_mcs[MAX_HE_MCS_LEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
	unsigned char gi; /* 0:auto;1:800;2:1600;3:3200 */
} wdev_he_cap;


#ifdef MAP_R2
typedef struct GNU_PACKED _wdev_extended_ap_metrics {
	u32 uc_tx;
	u32 uc_rx;
	u32 mc_tx;
	u32 mc_rx;
	u32 bc_tx;
	u32 bc_rx;
} wdev_extended_ap_metric;


typedef struct GNU_PACKED _wdev_sta_extended_info {
#if 0
	u8 bssid[MAC_ADDR_LEN];
#endif
	u32 last_data_ul_rate;
	u32 last_data_dl_rate;
	u32 utilization_rx;
	u32 utilization_tx;
} wdev_sta_ext_info;

typedef struct GNU_PACKED _wdev_extended_sta_metrics {
#if 0
	u8 sta_mac[MAC_ADDR_LEN];
	u8 extended_info_cnt;
#endif
	wdev_sta_ext_info sta_info;
} wdev_extended_sta_metrics;

#endif
typedef struct GNU_PACKED _wapp_cac_info {
	u8 channel;
	u8 ret;
	u8 cac_timer;
} wapp_cac_info;
#ifdef MAP_R2
typedef enum cac_mode
{
	CONTINUOUS_CAC,
	DEDICATED_CAC,
	REDUCED_MIMO_CAC,
} CAC_MODE;
#endif


typedef struct GNU_PACKED _wdev_misc_cap {
	u8	max_num_of_cli;
	u8	max_num_of_bss;
	u8	num_of_bss;
	u8	max_num_of_block_cli;
} wdev_misc_cap;

struct GNU_PACKED he_nss{
	u16 nss_80:2;
	u16 nss_160:2;
	u16 nss_8080:2;
};

struct GNU_PACKED map_cli_cap {
	u16 bw:2;
	u16 phy_mode:3;
	u16 nss:2;
	u16 btm_capable:1;
	u16 rrm_capable:1;
	u16 mbo_capable:1;
	struct he_nss nss_he;
};

#ifdef MAP_R3_WF6
struct GNU_PACKED assoc_wifi6_sta_info {
	unsigned char tid;
	unsigned char tid_q_size;
};

typedef struct GNU_PACKED _wdev_wf6_cap {
	unsigned char he_mcs_len;
	unsigned char he_mcs[MAX_HE_MCS_LEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
	unsigned char agent_role;
	unsigned char su_beamformee_status;
	unsigned char beamformee_sts_less80;
	unsigned char beamformee_sts_more80;
	unsigned char max_user_dl_tx_mu_mimo;
	unsigned char max_user_ul_rx_mu_mimo;
	unsigned char max_user_dl_tx_ofdma;
	unsigned char max_user_ul_rx_ofdma;
	unsigned char rts_status;
	unsigned char mu_rts_status;
	unsigned char m_bssid_status;
	unsigned char mu_edca_status;
	unsigned char twt_requester_status;
	unsigned char twt_responder_status;
} wdev_wf6_cap;

typedef struct GNU_PACKED _wdev_wf6_cap_roles {
	unsigned char role_supp;
	wdev_wf6_cap wf6_role[2];
} wdev_wf6_cap_roles;
#endif

typedef struct GNU_PACKED _wapp_client_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	u8 sta_status; /* WAPP_STA_STATE */
	u16 assoc_time;
	u16 downlink;
	u16 uplink;
	signed char uplink_rssi;
	/*traffic stats*/
	u32 bytes_sent;
	u32 bytes_received;
	u32 packets_sent;
	u32 packets_received;
	u32 tx_packets_errors;
	u32 rx_packets_errors;
	u32 retransmission_count;
	u16 link_availability;
	u16 assoc_req_len;
	u8 bLocalSteerDisallow;
	u8 bBTMSteerDisallow;
	u8 status;
	/* ht_cap */
	/* vht_cap */

	/* Throughput for Tx/Rx */
	u32 tx_tp;
	u32 rx_tp;
	struct map_cli_cap cli_caps;
#ifdef MAP_R2
	wdev_extended_sta_metrics ext_metric_info;
#endif
	u16 disassoc_reason;
#ifdef MAP_R2
	u8 IsReassoc;
#endif
	u8  is_APCLI;
#ifdef MAP_R3_WF6
	u8 tid_cnt;
	struct assoc_wifi6_sta_info status_tlv[MAX_TID];
#endif
} wapp_client_info;

struct GNU_PACKED chnList {
	u8 channel;
	u8 pref;
	u16 cac_timer;
};

typedef struct GNU_PACKED _wdev_chn_info {
	u8		op_ch;
	u8		op_class;
	u16		band; /* 24g; 5g1; 5g2 */
	u8		ch_list_num;
	u8		non_op_chn_num;
	u16		dl_mcs;
	struct chnList ch_list[32];
	u8		non_op_ch_list[32];
	u8		AutoChannelSkipListNum;
	u8		AutoChannelSkipList[MAX_NUM_OF_CHANNELS + 1];
} wdev_chn_info;

struct GNU_PACKED opClassInfo {
	u8	op_class;
	u8	num_of_ch;
	u8	ch_list[13];
};

typedef struct GNU_PACKED _wdev_op_class_info {
	u8		num_of_op_class;
	struct opClassInfo opClassInfo[MAX_OP_CLASS];
} wdev_op_class_info;

struct GNU_PACKED opClassInfoExt {
	u8	op_class;
	u8	num_of_ch;
	u8	ch_list[MAX_NUM_OF_CHANNELS];
};

struct GNU_PACKED _wdev_op_class_info_ext {
	u8		num_of_op_class;
	struct opClassInfoExt opClassInfoExt[MAX_OP_CLASS];
};

typedef struct GNU_PACKED _wdev_bss_info {
	u8 if_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	char ssid[MAX_LEN_OF_SSID + 1];
	u8 SsidLen;
	u8 map_role;
	u32 auth_mode;
	u32 enc_type;
	u8 key_len;
	u8 key[64 + 1];
	u8 hidden_ssid;
} wdev_bss_info;

typedef struct GNU_PACKED _wsc_apcli_config {
	char ssid[MAX_LEN_OF_SSID + 1];
	u8 SsidLen;
	u16 AuthType;
	u16 EncrType;
	u8 Key[64];
	u16 KeyLength;
	u8 KeyIndex;
	u8 bssid[MAC_ADDR_LEN];
	u8 peer_map_role;
	u8 own_map_role;
} wsc_apcli_config;

typedef struct GNU_PACKED _wsc_apcli_config_msg {
	u32 profile_count;
	wsc_apcli_config apcli_config[0];
} wsc_apcli_config_msg, *p_wsc_apcli_config_msg;

typedef struct GNU_PACKED _wdev_ap_metric {
	u8		bssid[MAC_ADDR_LEN];
	u8		cu;
	u8 		ESPI_AC[AC_NUM][3];
#ifdef MAP_R2
	wdev_extended_ap_metric ext_ap_metric;
#endif
} wdev_ap_metric;

#ifdef MAP_R2
typedef struct GNU_PACKED _wdev_radio_metric {
	u8 cu_noise;
	u8 cu_tx;
	u8 cu_rx;
	u8 cu_other;
	u32 edcca;
} wdev_radio_metric;
#endif
typedef struct GNU_PACKED _wdev_ap_config {
	u8 sta_report_on_cop;
	u8 sta_report_not_cop;
	u8 rssi_steer;
} wdev_ap_config;

struct GNU_PACKED pwr_limit {
	u8	op_class;
	u8	max_pwr;
};

typedef struct GNU_PACKED _wdev_tx_power {
	u8		num_of_op_class;
	struct pwr_limit tx_pwr_limit[MAX_OP_CLASS];
	u16 tx_pwr;
} wdev_tx_power;

/*Driver detects sta needed to steer*/
typedef struct GNU_PACKED _wdev_steer_sta {
	u8 mac_addr[MAC_ADDR_LEN];
} wdev_steer_sta;

typedef struct GNU_PACKED _wapp_probe_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 channel;
	signed char rssi;
	u8 preq_len;
	u8 preq[PREQ_IE_LEN];
} wapp_probe_info;

typedef struct GNU_PACKED _wapp_bcn_rpt_info {
	u8 sta_addr[MAC_ADDR_LEN];
	u8 last_fragment;
	u16 bcn_rpt_len;
	u8 bcn_rpt[BCN_RPT_LEN];
} wapp_bcn_rpt_info;

typedef struct GNU_PACKED wapp_bhsta_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 connected_bssid[MAC_ADDR_LEN];
	u8 peer_map_enable;
} wapp_bhsta_info;

typedef struct GNU_PACKED _wdev_steer_policy {
	u8 steer_policy;
	u8 cu_thr;
	u8 rcpi_thr;
} wdev_steer_policy;

typedef struct GNU_PACKED _bssload_threshold {
	u8 high_bssload_thrd;
	u8 low_bssload_thrd;
} bssload_threshold;

typedef struct GNU_PACKED _wapp_bssload_info {
	u16 sta_cnt;
	u8 ch_util;
	u16 AvalAdmCap;
} wapp_bssload_info;

/* By air monitor*/
typedef struct GNU_PACKED _wapp_mnt_info {
	u8 sta_addr[MAC_ADDR_LEN];
	signed char rssi;
} wapp_mnt_info;

typedef struct GNU_PACKED _wapp_csa_info {
	u8 new_channel;
} wapp_csa_info;

#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
struct GNU_PACKED wapp_wps_config_info {
	u8 SSID[33];	/* mandatory */
	u8 channel;
	u16 AuthType;	/* mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
	u16 EncrType;	/* mandatory, 1: none, 2: wep, 4: tkip, 8: aes */
	u8 Key[64];		/* mandatory, Maximum 64 byte */
	u16 KeyLength;
	u8 MacAddr[MAC_ADDR_LEN];	/* mandatory, AP MAC address */
	u8 bss_role;				/*0-Fronthaul, 1-Backhaul*/
	u8 index;
};
#endif
typedef struct GNU_PACKED _wapp_bss_state_info {
	u32 interface_index;
	WAPP_BSS_STATE bss_state;
} wapp_bss_state_info;

typedef struct GNU_PACKED _wapp_ch_change_info {
	u32 interface_index;
	u8 new_ch;/*New channel IEEE number*/
	u8 op_class;
} wapp_ch_change_info;

typedef struct GNU_PACKED _wapp_txpower_change_info {
	u32 interface_index;
	u16 new_tx_pwr;/*New TX power*/
} wapp_txpower_change_info;

typedef struct GNU_PACKED _wapp_apcli_association_info {
	u32 interface_index;
	WAPP_APCLI_ASSOC_STATE apcli_assoc_state;
	signed char rssi;
	signed char PeerMAPEnable;
} wapp_apcli_association_info;

typedef struct GNU_PACKED _wapp_bssload_crossing_info {
	u32 interface_index;
	u8 bssload_high_thrd;
	u8 bssload_low_thrd;
	u8 bssload;
} wapp_bssload_crossing_info;

typedef struct GNU_PACKED _wapp_sta_cnnct_rejected_info {
	u32 interface_index;
	u8 sta_mac[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	WAPP_CONNECT_FAILURE_REASON cnnct_fail;
#ifdef MAP_R2
	u16 assoc_status_code;
	u16 assoc_reason_code;
#endif
} wapp_sta_cnnct_rej_info;

struct GNU_PACKED map_vendor_ie
{
	u8 type;
	u8 subtype;
	u8 root_distance;
	u8 connectivity_to_controller;
	u16 uplink_rate;
	u8 uplink_bssid[MAC_ADDR_LEN];
	u8 bssid_5g[MAC_ADDR_LEN];
	u8 bssid_2g[MAC_ADDR_LEN];
};

typedef struct _qbss_load_param {
	u8     bValid;                     /* 1: variable contains valid value */
	u16      StaNum;
	u8       ChannelUtilization;
	u16      RemainingAdmissionControl;  /* in unit of 32-us */
} QBSS_LOAD_PARM, *PQBSS_LOAD_PARM;

#ifdef MAP_R2
typedef struct GNU_PACKED _wapp_qbss_load {
	u8 bValid;/*1: variable contains valid value*/
	u16  StaNum;
	u8   ChannelUtilization;
	u16  RemainingAdmissionControl;/*in unit of 32-us*/
} WAPP_QBSS_LOAD_PARM;

#endif
#ifdef MAP_6E_SUPPORT
struct GNU_PACKED map_rnr {
	u8 channel;
	u8 op;
	u8 cce_ind;
};
#endif

#ifdef DPP_R2_SUPPORT
struct GNU_PACKED cce_vendor_ie
{
	u8 value;
};

#define MAX_CCE_CHANNEL 128
#define MAX_RNR_CHANNEL 30

struct GNU_PACKED cce_vendor_ie_result {
	u8 num;
	u8 cce_ch[MAX_CCE_CHANNEL];//channel list, on which beacon includes cce ie
#ifdef MAP_R3_6E_SUPPORT
	u8 rnr_6e_num;
	u8 rnr_6e_ch[MAX_RNR_CHANNEL];
#endif
};
#endif

struct GNU_PACKED scan_bss_info {
	u8 Bssid[MAC_ADDR_LEN];
	u8 Channel;
	u8 CentralChannel;
	signed char Rssi;
	signed char MinSNR;
	u8 Privacy;

	u8 SsidLen;
	u8 Ssid[MAX_LEN_OF_SSID];

	u16 AuthMode;
	u16 EncrypType;
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	wdev_he_cap he_cap;
	u8 map_vendor_ie_found;
	struct map_vendor_ie map_info;
#ifdef MAP_R2
        WAPP_QBSS_LOAD_PARM QbssLoad;
#endif
#ifdef MAP_6E_SUPPORT
	struct map_rnr rnr_6e;
#endif
};
struct GNU_PACKED wapp_scan_info {
	u32 interface_index;
	u8 more_bss;
	u8 bss_count;
	struct scan_bss_info bss[0];
};

struct GNU_PACKED wapp_wsc_scan_info {
	u8 bss_count;
	u8 Uuid[16];
};

struct GNU_PACKED radar_notif_s
{
	u32 channel;
	u32 status;
	u32 bw;
};

#ifdef WIFI_MD_COEX_SUPPORT
struct GNU_PACKED unsafe_channel_notif_s
{
	u32 ch_bitmap[4];
};

struct GNU_PACKED band_status_change {
	u8 status;	/*0-radio temporarily cannot be used, 1-radio can be used*/
};
#endif

typedef struct GNU_PACKED _NDIS_802_11_SSID {
	u32 SsidLength;	/* length of SSID field below, in bytes; */
	/* this can be zero. */
	char Ssid[MAX_LEN_OF_SSID + 1];	/* SSID information field */
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;
struct GNU_PACKED nop_channel_list_s
{
	u8 channel_count;
	u8 channel_list[MAX_NUM_OF_WAPP_CHANNELS];
};

/* WSC configured credential */
typedef struct _WSC_CREDENTIAL {
	NDIS_802_11_SSID SSID;	/* mandatory */
	u16 AuthType;	/* mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
	u16 EncrType;	/* mandatory, 1: none, 2: wep, 4: tkip, 8: aes */
	u8 Key[64];		/* mandatory, Maximum 64 byte */
	u16 KeyLength;
	u8 MacAddr[MAC_ADDR_LEN];	/* mandatory, AP MAC address */
	u8 KeyIndex;		/* optional, default is 1 */
	u8 bFromUPnP;	/* TRUE: This credential is from external UPnP registrar */
	u8 bss_role;		/*0-Fronthaul, 1-Backhaul*/
	u8 DevPeerRole;	/* Device role for the peer device sending M8 */
	u16 IpConfigMethod;
	u32				RegIpv4Addr;
	u32				Ipv4SubMask;
	u32				EnrIpv4Addr;
	u32				AvaIpv4SubmaskList[IWSC_MAX_SUB_MASK_LIST_COUNT];
} WSC_CREDENTIAL, *PWSC_CREDENTIAL;

struct scan_SSID
{
	char ssid[MAX_LEN_OF_SSID+ 1];
	unsigned char SsidLen;
};

struct vendor_map_element {
	u8 eid;
	u8 length;
	char oui[3]; /* 0x50 6F 9A */
	char mtk_ie_element[4];
	char type;
	char subtype;
	char root_distance;
	char controller_connectivity;
	short uplink_rate;
	char uplink_bssid[MAC_ADDR_LEN];
	char _5g_bssid[MAC_ADDR_LEN];
	char _2g_bssid[MAC_ADDR_LEN];
};

struct GNU_PACKED scan_BH_ssids
{
	unsigned long scan_cookie;
	unsigned char scan_channel_count;
	unsigned char scan_channel_list[32];
	unsigned char profile_cnt;
	struct scan_SSID scan_SSID_val[MAX_PROFILE_CNT];
};

struct GNU_PACKED action_frm_data {
	u32 ifindex;
	u8 bssid[MAC_ADDR_LEN];
	u8 destination_addr[MAC_ADDR_LEN];
	u8 transmitter_addr[MAC_ADDR_LEN];
	u32 chan;
	u32 wait_time;
	u32 no_cck;
	u32 frm_len;
	u16 seq_no;
	char frm[0];
};

struct GNU_PACKED roc_req {
	u32 ifindex;
	u32 chan;
	u32 wait_time;
};

#ifdef DPP_SUPPORT
struct GNU_PACKED wapp_dpp_action_frame {
	u8 src[MAC_ADDR_LEN];
	u32 wapp_dpp_frame_id_no;
	u32 chan;
	u32 frm_len;
	u32 is_gas;
	u8 frm[0];
};

struct GNU_PACKED wapp_dpp_frm_tx_status {
	u8 tx_success;
	u16 seq_no;
};

struct GNU_PACKED pmk_req {
	u32 ifindex;
	u8 pmk[LEN_PMK];
	u8 pmk_len;
	u8 pmkid[LEN_PMKID];
	u8 authenticator_addr[MAC_ADDR_LEN];
	u8 supplicant_addr[MAC_ADDR_LEN];
	int timeout;
	int akmp;
	u8 ssid[MAX_LEN_OF_SSID];
	size_t ssidlen;
};
#endif /*DPP_SUPPORT*/

struct GNU_PACKED mnt_sta {
	u32 ifindex;
	u8 sta_mac[MAC_ADDR_LEN];
	u8 sta_id;
};

struct GNU_PACKED mnt_max_pkt {
	u32 ifindex;
	u32 pkt_number;
};

struct GNU_PACKED map_ch {
	u32 ifindex;
	u8 ch_num;
#ifdef MAP_R2
	u8 cac_req;
	u8 map_dev_role;
#endif /* MAP_R2 */
};

#ifdef MAP_R3
struct GNU_PACKED wapp_sta_info {
        u8 src[MAC_ADDR_LEN];
        char ssid[MAX_LEN_OF_SSID + 1];
        unsigned char SsidLen;
	u8 passphrase[LEN_PSK];
	u8 pmk_len;
        u8 pmk[LEN_PMK_MAX];
	u8 ptk_len;
        u8 ptk[LEN_MAX_PTK];
};

struct GNU_PACKED wapp_uri_info {
	u8 src_mac[MAC_ADDR_LEN];
	u8 uri_len;
	u8 rcvd_uri[LEN_MAX_URI];
};
#endif /* MAP_R3 */

struct GNU_PACKED wapp_srg_bitmap {
	u32 color_31_0;
	u32 color_63_32;
	u32 bssid_31_0;
	u32 bssid_63_32;
};

struct GNU_PACKED wapp_mesh_sr_info {
	u8 sr_mode;
	u8 ul_traffic_status;
	struct wapp_srg_bitmap bm_info;
};

struct GNU_PACKED wapp_mesh_sr_topology {
	u8 map_dev_count;
	u8 map_dev_sr_support_mode;
	u8 self_role;
	u8 map_remote_al_mac[MAC_ADDR_LEN];
	u8 map_remote_fh_bssid[MAC_ADDR_LEN];
	u8 map_remote_bh_mac[MAC_ADDR_LEN];
	unsigned char ssid_len;
	unsigned char ssid[MAX_LEN_OF_SSID + 1];
};

typedef union GNU_PACKED _wapp_event_data {
	wapp_dev_info dev_info;
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	wdev_misc_cap misc_cap;
	wapp_client_info cli_info;
	wdev_chn_info chn_list;
	wdev_op_class_info op_class;
	wdev_bss_info bss_info;
	wdev_ap_metric ap_metrics;
	wdev_ap_config ap_conf;
	wdev_tx_power tx_pwr;
	wdev_steer_sta str_sta;
	wapp_probe_info probe_info;
	wapp_bcn_rpt_info bcn_rpt_info;
	wapp_bssload_info bssload_info;
	wapp_bssload_crossing_info bssload_crossing_info;
	wapp_mnt_info mnt_info;
	wapp_bss_state_info bss_state_info;
	wapp_ch_change_info ch_change_info;
	wapp_txpower_change_info txpwr_change_info;
	wapp_apcli_association_info apcli_association_info;
	wapp_bhsta_info bhsta_info;
	wapp_csa_info csa_info;
	wapp_sta_cnnct_rej_info sta_cnnct_rej_info;
	u8 ch_util;
	struct wapp_scan_info scan_info;
	struct wapp_wsc_scan_info wsc_scan_info;
	u32 a4_missing_entry_ip;
	struct radar_notif_s radar_notif;
#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
	struct wapp_wps_config_info wps_conf_info;
#endif
        wapp_cac_info cac_info;
#ifdef MAP_R2
	wdev_extended_ap_metric ext_ap_metrics;
	wdev_radio_metric radio_metrics;
#endif
#ifdef DPP_SUPPORT
	u32 wapp_dpp_frame_id_no;
	struct wapp_dpp_action_frame frame;
	struct wapp_dpp_frm_tx_status tx_status;
#ifdef DPP_R2_SUPPORT
	struct cce_vendor_ie_result cce_ie_result;
#endif
#endif /*DPP_SUPPORT*/
	unsigned char cac_enable;
#ifdef WIFI_MD_COEX_SUPPORT
	struct unsafe_channel_notif_s unsafe_ch_notif;
	struct band_status_change band_status;
#endif
#ifdef MAP_R3
	struct wapp_sta_info sta_info;
        struct wapp_uri_info uri_info;
#endif /* MAP_R3 */
#ifdef QOS_R1
	u8 *qos_frm;
#endif
	u8	ifname[IFNAMSIZ];
#ifdef MAP_R3
	struct wapp_mesh_sr_info mesh_sr_info;
#endif /* MAP_R3 */
} wapp_event_data;
struct GNU_PACKED _wapp_event2_data {
	wapp_client_info cli_info;
};
typedef struct GNU_PACKED _wapp_req_data {
	u32	ifindex;
	u8 mac_addr[MAC_ADDR_LEN];
	u32 value;
	bssload_threshold bssload_thrd;
	wdev_steer_policy str_policy;
	wdev_ap_config ap_conf;
	WSC_CREDENTIAL bh_wsc_profile;
	struct scan_BH_ssids scan_bh_ssids;
#ifdef MAP_R3
	struct wapp_srg_bitmap bm_info;
	u8 band_index;
	struct wapp_mesh_sr_topology topology_update;
#endif /* MAP_R3 */
} wapp_req_data;

struct GNU_PACKED wapp_req {
	u8 req_id;
	u8 data_len;
	wapp_req_data data;
};

struct GNU_PACKED wapp_event {
	u8 len;
	u8 event_id;
	u32 ifindex;
	wapp_event_data data;
};
struct GNU_PACKED wapp_event2 {
	u8 len;
	u8 event_id;
	u32 ifindex;
	struct _wapp_event2_data data;
};
typedef struct GNU_PACKED _tbtt_info_set {
	u8 NrAPTbttOffset;
	u32 ShortBssid;
} tbtt_info_set;

typedef struct GNU_PACKED _wapp_nr_info
{
	u8 	Bssid[MAC_ADDR_LEN];
	u32 BssidInfo;
	u8  RegulatoryClass;
	u8  ChNum;
	u8  PhyType;
	u8  CandidatePrefSubID;
	u8  CandidatePrefSubLen;
	u8  CandidatePref;
	/* extra sec info */
	u32 akm;
	u32 cipher;
	u8  TbttInfoSetNum;
	tbtt_info_set TbttInfoSet;
	u8  Rssi;
} wapp_nr_info;

/* for NR IE , append Bssid ~ CandidatePref */
#define NEIGHBOR_REPORT_IE_SIZE 	sizeof(wapp_nr_info) - 15


typedef struct daemon_nr_list {
	u8 	CurrListNum;
	wapp_nr_info NRInfo[DAEMON_NEIGHBOR_REPORT_MAX_NUM];
} DAEMON_NR_LIST, *P_DAEMON_NR_LIST;

typedef struct GNU_PACKED daemon_neighbor_report_list {
	u8	Newlist;
	u8 	TotalNum;
	u8 	CurrNum;
	u8 	reserved;
	wapp_nr_info EvtNRInfo[PER_EVENT_LIST_MAX_NUM];
} DAEMON_EVENT_NR_LIST, *P_DAEMON_EVENT_NR_LIST;


typedef struct GNU_PACKED neighbor_report_msg {
	DAEMON_EVENT_NR_LIST evt_nr_list;
} DAEMON_NR_MSG, *P_DAEMON_NR_MSG;


/* for coverting wireless mode to string  */
enum WIFI_MODE {
	WMODE_INVALID = 0,
	WMODE_A = 1 << 0,
	WMODE_B = 1 << 1,
	WMODE_G = 1 << 2,
	WMODE_GN = 1 << 3,
	WMODE_AN = 1 << 4,
	WMODE_AC = 1 << 5,
	WMODE_AX_24G = 1 << 6,
	WMODE_AX_5G = 1 << 7,
	WMODE_AX_6G = 1 << 8,
	WMODE_COMP = 9, /* total types of supported wireless mode, add this value once yow add new type */
};
typedef union GNU_PACKED _RRM_BSSID_INFO
{
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
		u32 Reserved:18;
		u32 FTM:1;
		u32 VHT:1;
		u32 HT:1;
		u32 MobilityDomain:1;
		u32 ImmediateBA:1;
		u32 DelayBlockAck:1;
		u32 RRM:1;
		u32 APSD:1;
		u32 Qos:1;
		u32 SpectrumMng:1;
		u32 KeyScope:1;
		u32 Security:1;
		u32 APReachAble:2;
#else
		u32 APReachAble:2;
		u32 Security:1;
		u32 KeyScope:1;
		u32 SpectrumMng:1;
		u32 Qos:1;
		u32 APSD:1;
		u32 RRM:1;
		u32 DelayBlockAck:1;
		u32 ImmediateBA:1;
		u32 MobilityDomain:1;
		u32 HT:1;
		u32 VHT:1;
		u32 FTM:1;
		u32 Reserved:18;
#endif
	} field;
	u32 word;
} RRM_BSSID_INFO, *PRRM_BSSID_INFO;
#endif /* __WAPP_TYPES_H__ */
