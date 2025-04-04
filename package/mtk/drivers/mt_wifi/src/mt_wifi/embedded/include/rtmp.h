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
/****************************************************************************
 ***************************************************************************

	Module Name:
	rtmp.h

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who		 When		  What
	--------	----------	----------------------------------------------

*/
#ifndef __RTMP_H__
#define __RTMP_H__
#include "common/link_list.h"
#include "common/module.h"
#include "spectrum_def.h"

#include "rtmp_dot11.h"

#include "security/sec_cmm.h"

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
#include "security/sae_cmm.h"
#include "security/crypt_biginteger.h"
#endif /* DOT11_SAE_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
#include "icap.h"
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel_cmm.h"
#endif /* CONFIG_AP_SUPPORT */

#include "wsc.h"
#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */



#include "rtmp_chip.h"

#ifdef DOT11R_FT_SUPPORT
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "rrm_cmm.h"
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
#include "tdls_cmm.h"
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef CLIENT_WDS
#include "client_wds_cmm.h"
#endif /* CLIENT_WDS */


#ifdef P2P_SUPPORT
#include "p2p_cmm.h"
#endif /* P2P_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#include "cfg80211_cmm.h"
#endif /* RT_CFG80211_SUPPORT */

#ifdef WFD_SUPPORT
#include "wfd_cmm.h"
#endif /* WFD_SUPPORT */

#include "rate_ctrl/ra_ctrl.h"

#ifdef MT_MAC
#ifdef TXBF_SUPPORT
#include "txbf/mt_txbf.h"
#include "txbf/mt_txbf_cal.h"
#endif
#endif

#ifdef ROUTING_TAB_SUPPORT
#include "routing_tab.h"
#endif /* ROUTING_TAB_SUPPORT */

#include "tr.h"
#include "ht.h"
#include "vht.h"
#include "he.h"
#include "bss_color.h"

#ifdef CONFIG_6G_AFC_SUPPORT
#include "afc.h"
#endif /* CONFIG_6G_AFC_SUPPORT */

/* vendor specific ie */
#include "vendor.h"


#ifdef IGMP_SNOOP_SUPPORT
#include "ipv6.h"
#endif

#ifdef CFG_SUPPORT_CSI
#include <net/netlink.h>
#include <linux/netlink.h>
#include <net/genetlink.h>
#include <linux/socket.h>
#endif

struct _RTMP_RA_LEGACY_TB;

typedef struct _RTMP_ADAPTER RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER *PRTMP_ADAPTER;

typedef struct wifi_dev RTMP_WDEV;
typedef struct wifi_dev *PRTMP_WDEV;

typedef struct _STA_ADMIN_CONFIG STA_ADMIN_CONFIG;
typedef struct _STA_ADMIN_CONFIG *PSTA_ADMIN_CONFIG;
/* typedef struct _RTMP_CHIP_OP RTMP_CHIP_OP; */
/* typedef struct _RTMP_CHIP_CAP RTMP_CHIP_CAP; */

typedef struct _PEER_PROBE_REQ_PARAM PEER_PROBE_REQ_PARAM;
typedef struct _AUTO_CH_CTRL AUTO_CH_CTRL;
typedef struct _AUTO_CH_CTRL *PAUTO_CH_CTRL;

#ifdef BB_SOC
#include "os/bb_soc.h"
#endif

#include "mac/mac.h"

#include "mcu/mcu.h"

#ifdef CONFIG_ANDES_SUPPORT
#include "mcu/andes_core.h"
#endif

#include "radar.h"

#ifdef CARRIER_DETECTION_SUPPORT
#include "cs.h"
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef MT_DFS_SUPPORT
#include "mt_rdm.h" /* Jelly20150322 */
#endif

#ifdef LED_CONTROL_SUPPORT
#include "rt_led.h"
#endif /* LED_CONTROL_SUPPORT */

#ifdef CONFIG_ATE
#include "ate.h"
#endif

#ifdef CONFIG_WLAN_SERVICE
#include "agent.h"
#endif /* CONFIG_WLAN_SERVICE */

#ifdef CONFIG_DOT11U_INTERWORKING
#include "dot11u_interworking.h"
#include "gas.h"
#endif

#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
#include "wnm.h"
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
#include "hotspot.h"
#endif

#ifdef MBO_SUPPORT
#include "mbo.h"
#endif

#ifdef OCE_SUPPORT
#include "oce.h"
#endif /* OCE_SUPPORT */

#ifdef SNIFFER_SUPPORT
#include "sniffer/sniffer.h"
#endif

#ifdef CONFIG_MAP_SUPPORT
#include "map.h"
#endif

#ifdef COEX_SUPPORT
#include "mcu/btcoex.h"
#endif /* COEX_SUPPORT */

#ifdef WIFI_MD_COEX_SUPPORT
#include "mcu/wifi_md_coex.h"
#include "safe_chn.h"
#endif /* WIFI_MD_COEX_SUPPORT */

#include "hw_ctrl/cmm_asic.h"

#include "hw_ctrl/cmm_chip.h"

#include "rtmp_dmacb.h"

#include "common/wifi_sys_info.h"
#include "wifi_sys_notify.h"

#ifdef CFG_SUPPORT_FALCON_MURU
#include "ap_muru.h"
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
#include "bgnd_scan.h"
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT
#include "dynwmm.h"
#endif /* DYNAMIC_WMM_SUPPORT */
#include "cmm_rvr_dbg.h"
#ifdef REDUCE_TCP_ACK_SUPPORT
#include "cmm_tcprack.h"
#endif

#ifdef WH_EVENT_NOTIFIER
#include "event_notifier.h"
#endif /* WH_EVENT_NOTIFIER */

#include "wlan_config/config_export.h"
#include "mgmt/be_export.h"
#include "fsm/fsm_sync.h"

#ifdef WTBL_TDD_SUPPORT
#include "mgmt/wtbl_tdd.h"
#endif /* WTBL_TDD_SUPPORT */

#ifdef RED_SUPPORT
#include "ra_ac_q_mgmt.h"
#endif
#ifdef FQ_SCH_SUPPORT
#include "fq_qm.h"
#endif
#ifdef AUTOMATION
#include "automation.h"
#endif /* AUTOMATION */

#ifdef AUTOMATION
#include "automation.h"
#endif /* AUTOMATION */

#ifdef LINUX_NET_TXQ_SUPPORT
#define LINUX_DEF_TX_QUEUE_LENGTH	1000
#endif /* LINUX_NET_TXQ_SUPPORT */

#include "protocol/protection.h"

#include "txpwr/single_sku.h"

#include "dbg_ctrl.h"

#include "oid_struct.h"

#include "mgmt/mgmt_entrytb.h"

#ifdef KERNEL_RPS_ADJUST
#include "kernel_rps_adjust.h"
#endif

#ifdef CONFIG_6G_SUPPORT
#include "ap_bss_mnger.h"
#endif /* CONFIG_6G_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
typedef struct _DUMMY_WCID_OBJ {
	UCHAR  State;
	UINT16 HwWcid;
	BOOLEAN bFixedRateSet;
	HTTRANSMIT_SETTING HTPhyMode; /* For All S/W Entry : ref Dummy Wcid Fixed Rate usage */
	atomic_t ref_cnt; /* how many wdev reference */
	atomic_t connect_cnt; /* how many sta rec cmd set & succuess (connect once, and disconnect once for all S/W Entry that WM keep one StaRec) */
} DUMMY_WCID_OBJ, *PDUMMY_WCID_OBJ;
#endif /* SW_CONNECT_SUPPORT */

/* TODO: shiang-6590, remove it after ATE fully re-organized! copy from rtmp_bbp.h */

/* Debug log color */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


/*Macro used in CTS/ACK timeout*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
#define MAX_ACK_TIMEOUT 0xFFFF
/*speed =  299792458 m/s, converted in 300m/us*/
#define LIGHT_SPEED  300
#endif /*ACK_CTS_TIMEOUT_SUPPORT*/

#ifndef MAX_BBP_ID
#ifdef RTMP_RBUS_SUPPORT
/* TODO: for this definition, need to modify it!! */
/*#define MAX_BBP_ID	255 */
#define MAX_BBP_ID	200

#else
#define MAX_BBP_ID	136

#endif /* RTMP_RBUS_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
#define MULTICAST_BLACK_LIST_STATIC_SIZE 3
#define MULTICAST_BLACK_LIST_SIZE_MAX	(20 + MULTICAST_BLACK_LIST_STATIC_SIZE)

/* Switch to enable/disable TVM Igmp Mode from iwpriv command */
enum {
	IGMP_TVM_SWITCH_DISABLE = 0,
	IGMP_TVM_SWITCH_ENABLE,
	IGMP_TVM_SWITCH_INVAL = 0xFF
};

#define IS_IGMP_TVM_MODE_EN(_EnMode)	(_EnMode == IGMP_TVM_SWITCH_ENABLE)

/* This is will come from Dat file, set from WebUI */
enum {
	IGMP_TVM_MODE_DISABLE = 0,
	IGMP_TVM_MODE_ENABLE,
	IGMP_TVM_MODE_AUTO
};

/* This will come in TV IE in MGMT frame */
/* In AUTO Mode, AP Disables its mode (If AP mode = AUTO) if the STA not connected ad Enabled if connected */
/* Currenlt STA always set AUTO */
enum {
	IGMP_TVM_IE_MODE_AUTO = 0,
	IGMP_TVM_IE_MODE_ENABLE,
/* This is only for internal Driver/CR4 to disable the Conversion for STA who do not have TVM_IE */
	IGMP_TVM_IE_MODE_DISABLE
};
#endif /* IGMP_TVM_SUPPORT */


#endif
/* TODO: ---End */

#define	FW_OWN_POLLING_COUNTER	3000

typedef enum _DRIVER_OWN_GET_STATUS_TYPE {
	DRIVER_OWN_INTERRUPT_MODE = 1,
	DRIVER_OWN_POLLING_MODE = 2,
} DRIVER_OWN_GET_STATUS_TYPE;

#define NON_DEFINED_BSSINFO_IDX 0xff

#ifdef ZERO_LOSS_CSA_SUPPORT
enum ChannelSwitchState {
	SET_CHANNEL_IDLE,
	SET_CHANNEL_COMMAND,
	CHANNEL_SWITCH_COUNT_ZERO_EVENT,
	ASIC_CHANNEL_SWITCH_COMMAND_ISSUED,
};
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef WIDI_SUPPORT
/* This defines after how many channels you want to go into listen channel.*/
#define TOGGLE_TO_WIFIDIRECT_LISTEN_CHANNEL 15
#endif /* WIDI_SUPPORT */

#ifdef ANDLINK_FEATURE_SUPPORT
#include <linux/time.h>
#define HOSTNAME_LEN 32
#define OID_ANDLINK_EVENT   0x2766 /* 10086 */
#define OID_ANDLINK_POLL    0x2767
#define OID_ANDLINK_STAINFO 0x2768
#define OID_ANDLINK_UPLINK	0x2769
#define OID_ANDLINK_HOSTNAME_IP 0x276b
#ifdef ANDLINK_V4_0
#define OID_ANDLINK_NRINFO	0x276c
#define OID_ANDLINK_BSSINFO	0x276d
#define OID_ANDLINK_EVENT_V40   0x276e
#endif/*ANDLINK_V4_0*/

#define MAX_INF_LEN   10 /* maximum length of interface name */
#define MAX_ASSOC_NUM 20 /* the max number of STAs that AP allowed to connect */


#define ANDLINK_GET_CURRENT_SEC(ptime)		\
{										\
	struct timeval tv;					\
	do_gettimeofday(&tv);			\
	*ptime = tv.tv_sec;					\
}

/* Driver Event Report */
struct wifi_event_inf_poll {
	u8 channel;
};

struct wifi_event_inf_stats {
	UINT64 BytesSent;
	UINT64 BytesReceived;
	UINT64 PacketsSent;
	UINT64 PacketsReceived;
	UINT64 ErrorSent;
	UINT64 ErrorReceived;
	UINT64 DropPacketsSent;
	UINT64 DropPacketsReceived;
};

struct wifi_ioctl_up_link {
	UCHAR MAC[MAC_ADDR_LEN];
	UINT8 Radio;
	UINT8 SSID[MAX_LEN_OF_SSID];
	UINT8 Channel;
	CHAR Noise;
	CHAR SNR;
	CHAR RSSI;
	UINT16 TxRate;/* Mbps */
	UINT16 RxRate;/* Mbps */
	ULONG RxRate_rt;
	ULONG TxRate_rt;
};


struct wifi_ioctl_sta_info {
	UINT8  sta_cnt;
	struct {
		UINT8  MacAddress[MAC_ADDR_LEN];
		UINT8  VMacAddr[MAC_ADDR_LEN];
		CHAR   RSSI;
		ULONGLONG UpTime;
		ULONG TxRate;/* Mbps */
		ULONG RxRate;/* Mbps */
		ULONG RxRate_rt;
		ULONG TxRate_rt;
	} item[MAX_ASSOC_NUM];
	UINT8  rept_sta_cnt;
	struct {
		UINT8  MacAddress[MAC_ADDR_LEN];
		UINT8  VMacAddr[MAC_ADDR_LEN];
		CHAR   RSSI;
		ULONGLONG UpTime;
		ULONG TxRate;/* Mbps */
		ULONG RxRate;/* Mbps */
		ULONG RxRate_rt;
		ULONG TxRate_rt;
	} rept_item[MAX_ASSOC_NUM];
};

struct wifi_ioctl_hostname_ip {
	UINT8  sta_cnt;
	struct {
		UINT8  MacAddress[MAC_ADDR_LEN];
        UINT IpAddr;/* In network order */
        RTMP_STRING HostName[HOSTNAME_LEN];/*device host name*/
	} item[MAX_ASSOC_NUM];
	UINT8  rept_sta_cnt;
	struct {
		UINT8  MacAddress[MAC_ADDR_LEN];
        UINT IpAddr;/* In network order */
        RTMP_STRING HostName[HOSTNAME_LEN];/*device host name*/
	} rept_item[MAX_ASSOC_NUM];
};

enum wifi_event_type {
	EVENTTYPE_INF_POLL = 1, /* Get interface basic info */
	EVENTTYPE_UPLINK_INFO,  /* Get uplink (ApCli) status */
	EVENTTYPE_INF_STATS,    /* Get interface statistics */
};

struct wifi_event {
	UINT8 type;
	CHAR  inf_name[MAX_INF_LEN];/* no need to edit */
	UINT8 MAC[MAC_ADDR_LEN];
	CHAR  SSID[MAX_LEN_OF_SSID];
	union {
		struct wifi_event_inf_poll  poll;
		struct wifi_event_inf_stats inf_stats;
	} data;
};

#ifdef ANDLINK_V4_0
#define ANDLINK_MAX_WLAN_NEIGHBOR 24
#define ANDLINK_MAX_ASSOC_NUM 32
enum {
	ANDLINK_IF5,
	ANDLINK_IF6,
	ANDLINK_IF8,
	ANDLINK_IF_MAX
};

struct GNU_PACKED mtk_andlink_radio_info {
	CHAR snr;
	CHAR rssi;
	CHAR noise;
};
struct GNU_PACKED mtk_wifi_uplink_info {
	char ssid[SSID_LEN];
	UCHAR channel;
	CHAR noise;
	CHAR snr;
	CHAR rssi;
	UINT tx_rate;
	UINT rx_rate;
	UINT32 tx_rate_rt;
	UINT32 rx_rate_rt;
	UINT32 avg_tx_rate[ANDLINK_IF_MAX];
	UINT32 avg_rx_rate[ANDLINK_IF_MAX];
	UINT32 max_tx_rate[ANDLINK_IF_MAX];
	UINT32 max_rx_rate[ANDLINK_IF_MAX];
};

struct GNU_PACKED mtk_wifi_report_cfg {
	UINT report_interval;
	UINT sample_interval;
	UCHAR uplink_stat_en;
	UCHAR sta_info_en;
	UCHAR wlan_nr_en;
	UCHAR wifi_stat_en;
};

struct GNU_PACKED wifi_scan_info_entry {
	char ssid[SSID_LEN];
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char channel;
	char rssi;
	UINT32 bandwidth;
	UINT32 wifistandard;
};

struct GNU_PACKED mtk_wifi_scan_info {
	int num;
	int offset;
	struct wifi_scan_info_entry scan_entry[ANDLINK_MAX_WLAN_NEIGHBOR];
};

struct GNU_PACKED mtk_andlink_sta_entry{
	unsigned char  mac_addr[MAC_ADDR_LEN];
	unsigned char  vmac_addr[MAC_ADDR_LEN];
	char	rssi;
	char	bw;
	unsigned long long uptime;
	unsigned long tx_rate;/* Mbps */
	unsigned long rx_rate;/* Mbps */
	unsigned long rx_rate_rt;
	unsigned long tx_rate_rt;
	unsigned long avg_tx_rate[ANDLINK_IF_MAX];
	unsigned long avg_rx_rate[ANDLINK_IF_MAX];
	unsigned long max_tx_rate[ANDLINK_IF_MAX];
	unsigned long max_rx_rate[ANDLINK_IF_MAX];
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
	unsigned long long tx_pkts;
	unsigned long long rx_pkts;
};

struct GNU_PACKED mtk_andlink_wifi_sta_info {
	unsigned int sta_cnt;
	unsigned int offset;
	struct mtk_andlink_sta_entry sta_entry[ANDLINK_MAX_ASSOC_NUM];
};

struct GNU_PACKED mtk_andlink_rate_conf {
	unsigned int period_time[ANDLINK_IF_MAX];
	unsigned int sample_time[ANDLINK_IF_MAX];
};

/*EVENT*/
typedef enum {
	ANDLINK_WIFI_CH_EVENT = 1,
} ANDLINK_EVENT_ID;

#define ANDLINK_SEC_LEN 32

struct GNU_PACKED andlink_wifi_ch_info {
	char sec_mode[ANDLINK_SEC_LEN];/*security mode*/
	char pwd[LEN_PSK];
	char ssid[SSID_LEN];
	UINT16 max_sta_num;
	BOOLEAN is_hidden;
};

typedef union GNU_PACKED _andlink_event_data {
	struct andlink_wifi_ch_info wifi_ch_info;
} andlink_event_data;

struct GNU_PACKED mtk_andlink_event {
	u8 len;
	u8 event_id;
	u32 ifindex;
	andlink_event_data data;
};
/*API func*/
INT andlink_send_wifi_chg_event(PRTMP_ADAPTER pAd, struct wifi_dev * wdev, struct andlink_wifi_ch_info * wifi_ch_info);

#endif/*ANDLINK_V4_0*/

#ifdef ANDLINK_HOSTNAME_IP
NDIS_STATUS update_sta_ip (IN PRTMP_ADAPTER	pAd, IN PNDIS_PACKET  pPkt);
NDIS_STATUS update_sta_hostname (IN PRTMP_ADAPTER	pAd,IN PNDIS_PACKET  pPkt);
#endif /*ANDLINK_HOSTNAME_IP*/

#endif /* ANDLINK_FEATURE_SUPPORT */

/*+++Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
#define MGMT_USE_QUEUE_FLAG	0x80
#define MGMT_USE_PS_FLAG	0x40
/*---Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
/* The number of channels for per-channel Tx power offset */

#define CHANNEL_SWITCHING_MODE	1
#define NORMAL_MODE				0

#define	MAXSEQ		(0xFFF)

#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */

#ifdef AIR_MONITOR
#define MAX_NUM_OF_MONITOR_STA		16
#define MAX_NUM_OF_MONITOR_GROUP	8 /* (MAX_NUM_OF_MONITOR_STA/2) */
#define MONITOR_MUAR_BASE_INDEX	 32
#define MAX_NUM_PER_GROUP		   2
#endif /* AIR_MONITOR */

#define MAX_TXPOWER_ARRAY_SIZE	5

#define MAX_EEPROM_BUFFER_SIZE	1024
#define PS_RETRIEVE_TOKEN		0x76

#ifdef TXBF_SUPPORT
#define NSTS_2          2
#define NSTS_3          3
#define NSTS_4          4
#endif

#define MAX_RXVB_BUFFER_IN_NORMAL 1024

#ifdef IXIA_C50_MODE
typedef struct ixia_ctl {
	UCHAR iMode;
	UCHAR iRssiflag;
	UCHAR iMacflag;
	UCHAR iforceIxia;
	UINT16 pktthld;
	UINT8 DeltaRssiTh;
	CHAR MinRssiTh;
	UINT8 chkTmr;
	INT16 pkt_offset;
	ULONG BA_timeout;
	ULONG max_BA_timeout;
	UINT8 debug_lvl;
	UINT8 tx_test_round;
	UINT8 rx_test_round;
} ixia_ctl;

struct ixia_tx_cnt {
	UINT32 tx_pkt_len;
	UINT32 txpktdetect;	/*check if traffic is runing*/
	UINT32 tx_pkt_from_os;
	UINT32 tx_pkt_enq_cnt[NR_CPUS];
	UINT32 tx_pkt_to_hw[TX_TYPE_MAX][NR_CPUS];
};

struct ixia_rx_cnt {
	UINT32 rx_pkt_len;
	UINT32 rxpktdetect;	/*check if traffic is runing*/
	UINT32 rx_from_hw[NR_CPUS];
	UINT32 rx_pkt_to_os[NR_CPUS];

	/*BA counter*/
	UINT32 rx_dup_drop[MAX_LEN_OF_MAC_TABLE];
	UINT32 rx_old_drop[MAX_LEN_OF_MAC_TABLE];
	UINT32 rx_flush_drop[MAX_LEN_OF_MAC_TABLE];
	UINT32 rx_surpass_drop[MAX_LEN_OF_MAC_TABLE];
};

#define VERIWAVE_MODE	1
#define IXIA_NORMAL_MODE	0

typedef enum {
	IXIA_OFF,
	IXIA_ERROR,
	IXIA_INFO
}IXIA_DEBUG_LVL;
#endif

extern unsigned char CISCO_OUI[];
extern UCHAR BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern UCHAR IXIA_PROBE_ADDR[MAC_ADDR_LEN];
extern char *CipherName[];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
#ifdef DOT11Z_TDLS_SUPPORT
extern UCHAR TDLS_LLC_SNAP_WITH_CATEGORY[10];
#ifdef WFD_SUPPORT
extern UCHAR TDLS_LLC_SNAP_WITH_WFD_CATEGORY[10];
#endif /* WFD_SUPPORT */
extern UCHAR TDLS_ETHERTYPE[2];
#endif /* DOT11Z_TDLS_SUPPORT */
extern UCHAR OfdmRateToRxwiMCS[];
extern UCHAR WMM_UP2AC_MAP[8];

extern unsigned char RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR CipherSuiteWpaNoneTkip[];
extern UCHAR CipherSuiteWpaNoneTkipLen;

extern UCHAR CipherSuiteWpaNoneAes[];
extern UCHAR CipherSuiteWpaNoneAesLen;

extern UCHAR SsidIe;
extern UCHAR SupRateIe;
extern UCHAR ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR HtCapIe;
extern UCHAR AddHtInfoIe;
extern UCHAR NewExtChanIe;
extern UCHAR BssCoexistIe;
extern UCHAR ExtHtCapIe;
#endif /* DOT11_N_SUPPORT */
extern UCHAR ExtCapIe;

extern UCHAR ErpIe;
extern UCHAR DsIe;
extern UCHAR TimIe;
extern UCHAR WpaIe;
extern UCHAR Wpa2Ie;
extern UCHAR IbssIe;
extern UCHAR WapiIe;

extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
#ifdef DPP_SUPPORT
extern UCHAR DPP_OUI[];
#endif /* DPP_SUPPORT */
extern UCHAR WAPI_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
#if (defined(WH_EZ_SETUP) || defined(MWDS) || defined(WAPP_SUPPORT))
extern UCHAR MTK_OUI[];
#endif
extern UCHAR PowerConstraintIE[];

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

extern struct wifi_fwd_func_table wf_drv_tbl;

extern UCHAR wf_apcli_active_links;
#endif

#ifdef RANDOM_PKT_GEN
extern INT32 RandomTxCtrl;
extern UINT32 Qidmapping[16];
#endif

#ifdef CSO_TEST_SUPPORT
extern INT32 CsCtrl;
#endif

/*VLAN (802.1Q) Related*/
#define TX_VLAN 0
#define RX_VLAN 1
#define MASK_CLEAR_PCP 0xFFFF1FFF
#define MASK_CLEAR_VID 0xFFFFF000
#define MASK_CLEAR_TCI_VID 0xF000
#define MASK_CLEAR_TCI_PCP 0x1FFF
#define MASK_TCI_VID 0x0FFF
#define MAX_VID 0x0FFF
#define MAX_PCP 7
#define PCP_LEN 3
#define CFI_LEN 1
#define VID_LEN 12

#ifdef VLAN_SUPPORT
/*
	VLAN TX Checking Policy :
	If the VLAN ID of the pkt is different from the wdev's VLANID
	0 : Do nothing (Keep tag)
	1 : Drop this pkt (Drop)
	2 : Replace the VLAN ID by the wdev's VLAN ID.(REPLACE VID )
	3 : Replace the VLAN tag by the wdev's VLAN tag.(REPLACE ALL)
	4 : Do nothing (Allow)
	IF pkt is non-vlan
	0~3: add vlan tag
	4: do nothing (Allow)
*/
enum VLAN_TX_Policy{
	VLAN_TX_KEEP_TAG = 0,
	VLAN_TX_DROP,
	VLAN_TX_REPLACE_VID,
	VLAN_TX_REPLACE_ALL,
	VLAN_TX_ALLOW,
	VLAN_TX_POLICY_NUM,
};
/*
	VLAN RX Checking Policy :
	If the VLAN ID of the pkt is different from the wdev's VLANID
	0 : Remove VLAN Tag (Untag)
	1 : Drop this pkt (Drop)
	2 : Do nothing (Allow)
	3 : Replace the VID only. (Replace VID)
	     Insert the VLAN Tag if the ingress pkt is non-vlan
	4 : Replace the VLAN tag by the wdev's VLAN tag (Replace ALL)
	     Insert the VLAN Tag if the ingress pkt is non-vlan
*/
enum VLAN_RX_Policy{
	VLAN_RX_UNTAG = 0,
	VLAN_RX_DROP,
	VLAN_RX_ALLOW,
	VLAN_RX_REPLACE_VID,
	VLAN_RX_REPLACE_ALL,
	VLAN_RX_POLICY_NUM,
};

enum VLAN_RX_TAG_MODE{
	VLAN_RX_TAG_HW_MODE = 0,
	VLAN_RX_TAG_SW_MODE = 1,
};


#endif /*VLAN_SUPPORT*/

struct _RX_BLK;
struct raw_rssi_info;

typedef struct _UAPSD_INFO {
	BOOLEAN bAPSDCapable;
} UAPSD_INFO;

typedef union _CAPTURE_MODE_PACKET_BUFFER {
	struct {
		UINT32	   BYTE0:8;
		UINT32	   BYTE1:8;
		UINT32	   BYTE2:8;
		UINT32	   BYTE3:8;
	} field;
	UINT32				   Value;
} CAPTURE_MODE_PACKET_BUFFER;

#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
typedef enum _APCLI_CONNECT_SCAN_TYPE {
	TRIGGER_SCAN_BY_USER = 0,
	TRIGGER_SCAN_BY_DRIVER = 1,
} APCLI_CONNECT_SCAN_TYPE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef WSC_AP_SUPPORT
typedef enum _APCLI_WSC_SCAN_TYPE {
	TRIGGER_FULL_SCAN = 0,
	TRIGGER_PARTIAL_SCAN = 1
} APCLI_WSC_SCAN_TYPE;
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef AIR_MONITOR
#define RULE_CTL							BIT(0)
#define RULE_CTL_OFFSET						0
#define RULE_MGT							BIT(1)
#define RULE_MGT_OFFSET						1
#define RULE_DATA							BIT(2)
#define RULE_DATA_OFFSET					2
#define RULE_A1								BIT(3)
#define RULE_A1_OFFSET						3
#define RULE_A2								BIT(4)
#define RULE_A2_OFFSET						4
#define DEFAULT_MNTR_RULE	(RULE_MGT | RULE_CTL | RULE_DATA | RULE_A1 | RULE_A2)


typedef enum _MNT_BAND_TYPE {
	MNT_BAND0 = 1,
	MNT_BAND1 = 2
} MNT_BAND_TYPE;

typedef struct	_MNT_MUAR_GROUP {
	BOOLEAN bValid;
	UCHAR	Count;
	UCHAR   Band;
	UCHAR	MuarGroupBase;
} MNT_MUAR_GROUP, *PMNT_MUAR_GROUP;

typedef struct	_MNT_STA_ENTRY {
	BOOLEAN bValid;
	UCHAR Band;
	UCHAR muar_idx;
	UCHAR muar_group_idx;
	ULONG Count;
	ULONG data_cnt;
	ULONG mgmt_cnt;
	ULONG cntl_cnt;
	UCHAR addr[MAC_ADDR_LEN];
	RSSI_SAMPLE RssiSample;
	LONG RssiTotal;
	VOID *pMacEntry;
}	MNT_STA_ENTRY, *PMNT_STA_ENTRY;

typedef struct _HEADER_802_11_4_ADDR {
	FRAME_CONTROL FC;
	USHORT Duration;
	USHORT SN;
	UCHAR FN;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR Addr4[MAC_ADDR_LEN];
} HEADER_802_11_4_ADDR, *PHEADER_802_11_4_ADDR;

typedef struct _AIR_RADIO_INFO {
	CHAR PHYMODE;
	CHAR STREAM;
	CHAR MCS;
	CHAR BW;
	CHAR ShortGI;
	ULONG RATE;
	CHAR RSSI[4];
	UCHAR Channel;
} AIR_RADIO_INFO, *PAIR_RADIO_INFO;

typedef struct _AIR_RAW {
	AIR_RADIO_INFO wlan_radio_tap;
	HEADER_802_11_4_ADDR wlan_header;
} AIR_RAW, *PAIR_RAW;
#endif /* AIR_MONITOR */

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
enum CH_SWITCH_STATE {
	CH_SWITCH_STATE_BASE = 0,
	CH_SWITCH_STATE_INIT = CH_SWITCH_STATE_BASE,
	CH_SWITCH_STATE_RUNNING,
	CH_SWITCH_STATE_MAX
};
enum CH_SWITCH_MSG {
	CH_SWITCH_MSG_LISTEN = 0,
	CH_SWITCH_MSG_CANCLE,
	CH_SWITCH_MSG_TIMEOUT,
	CH_SWITCH_MSG_MAX
};
#define CH_SWITCH_DFT_LISTEN_TIME 200
#define CH_SWITCH_FUNC_SIZE 6/*CH_SWITCH_STATE_MAX * CH_SWITCH_MSG_MAX*/
struct ch_switch_cfg {
	UCHAR channel;
	USHORT duration;
	BOOLEAN ch_sw_on_going;
	INT	ioctl_if;
	struct wifi_dev *wdev;
	RALINK_TIMER_STRUCT	ch_sw_timer;
	STATE_MACHINE ch_switch_sm;
#ifdef MAP_R3
	RTMP_OS_COMPLETION chan_switch_done;
	RTMP_OS_COMPLETION chan_switch_done_2;
	BOOLEAN wait_chan_switch_done;
	BOOLEAN wait_chan_switch_done_2;
#endif /* MAP_R3 */
	TIMER_FUNC_CONTEXT ch_sw_timer_func_contex;
	STATE_MACHINE_FUNC ch_switch_state_func[CH_SWITCH_FUNC_SIZE];
};
struct ch_switch_user_cfg {
	UCHAR channel;
	USHORT duration;
};
#endif

typedef enum _ENUM_DBDC_MODE_T {
	ENUM_SingleBand = 0,
	ENUM_DBDC_2G5G = 1,
	ENUM_DBDC_5G5G = 2,
	ENUM_DBDC_5G2G = 3
} ENUM_DBDC_MODE;

#ifdef IGMP_SNOOP_SUPPORT

typedef struct _MULTICAST_WHITE_LIST_ENTRY {
	BOOLEAN bValid;
	UCHAR EntryIPType;
	union {
		UCHAR IPv4[IPV4_ADDR_LEN];
		UCHAR IPv6[IPV6_ADDR_LEN];
	} IPData;
	union {
		UINT8 Byte[IPV6_ADDR_LEN];
		UINT32	DWord[IPV6_ADDR_LEN/sizeof(UINT32)];
	} PrefixMask;/* IPv6, Size 128 bits */
	UCHAR PrefixLen;
	UCHAR Addr[MAC_ADDR_LEN];
} MULTICAST_WHITE_LIST_ENTRY, *PMULTICAST_WHITE_LIST_ENTRY;

typedef struct _MULTICAST_WHITE_LIST_FILTER_TABLE {
	UCHAR EntryNum;
	MULTICAST_WHITE_LIST_ENTRY EntryTab[MULTICAST_WHITE_LIST_SIZE_MAX];
	NDIS_SPIN_LOCK MulticastWLTabLock;
} MULTICAST_WHITE_LIST_FILTER_TABLE, *PMULTICAST_WHITE_LIST_FILTER_TABLE;

#ifdef IGMP_SNOOPING_DENY_LIST
typedef struct _MULTICAST_DENY_LIST_ENTRY {
	UINT8 valid;
	PNET_DEV net_dev;
	UINT8 addr[IPV4_ADDR_LEN];
}MULTICAST_DENY_LIST_ENTRY, *P_MULTICAST_DENY_LIST_ENTRY;

typedef struct _MULTICAST_DENY_LIST_FILTER_TABLE {
	UINT8 valid_entry_count;
	MULTICAST_DENY_LIST_ENTRY entry[IGMP_DENY_TABLE_SIZE_MAX];
}MULTICAST_DENY_LIST_FILTER_TABLE, *P_MULTICAST_DENY_LIST_FILTER_TABLE;
#endif

#endif

/*for dat profile size setting*/
/*item per bss size =  real size + 1 */
#define PER_BSS_SIZE_2(_pAd)	(2 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_3(_pAd)	(3 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_4(_pAd)	(4 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_5(_pAd)	(5 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_6(_pAd)	(6 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_7(_pAd)	(7 *  MAX_MBSSID_NUM(_pAd))
#define PER_BSS_SIZE_8(_pAd)	(8 *  MAX_MBSSID_NUM(_pAd))

/* */
/*  Macros for flag and ref count operations */
/* */
#define RTMP_SET_FLAG(_M, _F)	   ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)	 ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)		((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)	  (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)	 (((_M)->Flags & (_F)) == (_F))
/* Macro for power save flag. */
#define RTMP_SET_PSFLAG(_M, _F)	   ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)	 ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)		((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)	  (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)	 (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)	 ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)	(((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define OPSTATUS_SET_FLAG_WDEV(_Wdev, _F)	 ((_Wdev)->OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG_WDEV(_Wdev, _F)   ((_Wdev)->OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG_WDEV(_Wdev, _F)	(((_Wdev)->OpStatusFlags & (_F)) != 0)
#define OPSTATUS_EQUAL_FLAG_WDEV(_Wdev, _F)	(((_Wdev)->OpStatusFlags & (_F)) == (_F))

#ifdef CONFIG_STA_SUPPORT
#define STA_STATUS_SET_FLAG(_pStaCfg, _F)	 ((_pStaCfg)->StaStatusFlags |= (_F))
#define STA_STATUS_CLEAR_FLAG(_pStaCfg, _F)   ((_pStaCfg)->StaStatusFlags &= ~(_F))
#define STA_STATUS_TEST_FLAG(_pStaCfg, _F)	(((_pStaCfg)->StaStatusFlags & (_F)) != 0)
#endif

#define WIFI_TEST_SET_FLAG(_pAd, _F)	 ((_pAd)->CommonCfg.WiFiTestFlags |= (_F))
#define WIFI_TEST_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.WiFiTestFlags &= ~(_F))
#define WIFI_TEST_CHECK_FLAG(_pAd, _F)	(((_pAd)->CommonCfg.WiFiTestFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry, _F)	  ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry, _F)	((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry, _F)	 (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define CLIENT_CAP_SET_FLAG(_pEntry, _F)	  ((_pEntry)->cli_cap_flags |= (_F))
#define CLIENT_CAP_CLEAR_FLAG(_pEntry, _F)	((_pEntry)->cli_cap_flags &= ~(_F))
#define CLIENT_CAP_TEST_FLAG(_pEntry, _F)	 (((_pEntry)->cli_cap_flags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)	((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define RTMP_SET_MORE_FLAG(_M, _F)	   ((_M)->MoreFlags |= (_F))
#define RTMP_TEST_MORE_FLAG(_M, _F)	  (((_M)->MoreFlags & (_F)) != 0)
#define RTMP_CLEAR_MORE_FLAG(_M, _F)	 ((_M)->MoreFlags &= ~(_F))

#define SET_ASIC_CAP(_pAd, _caps)		(hc_set_asic_cap(_pAd->hdev_ctrl, _caps))
#define IS_ASIC_CAP(_pAd, _caps)			((hc_get_asic_cap(_pAd->hdev_ctrl) & (_caps)) != 0)
#define CLR_ASIC_CAP(_pAd, _caps)		(hc_clear_asic_cap(_pAd->hdev_ctrl, _caps))
#define IS_HIF_TYPE(_pAd, _type) (hc_get_hif_type(_pAd->hdev_ctrl) == _type)
#define GET_HIF_TYPE(_pAd) (hc_get_hif_type(_pAd->hdev_ctrl))

/*
 * Replace pAd in the future, new definition API parameters
 */
/* MAC CAPS(old ASIC_CAPS) */
#define IS_MAC_CAPS(mac_caps, cap) (((mac_caps) & (cap)) != 0)
#define SET_MAC_CAPS(hdev_ctrl, caps)\
		hc_set_mac_cap(hdev_ctrl, caps)
#define GET_MAC_CAPS(hdev_ctrl)\
		hc_get_mac_cap(hdev_ctrl)
/* PHY CAPS */
#define IS_PHY_CAPS(phy_caps, cap) (((phy_caps) & (cap)) == (cap))
#define SET_PHY_CAPS(hdev_ctrl, caps)\
		hc_set_phy_cap(hdev_ctrl, caps)
#define GET_PHY_CAPS(hdev_ctrl)\
		hc_get_phy_cap(hdev_ctrl)

#define TX_FLAG_STOP_DEQUEUE	(fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS | \
								 fRTMP_ADAPTER_HALT_IN_PROGRESS | \
								 fRTMP_ADAPTER_NIC_NOT_EXIST | \
								 fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET)

#define INC_RING_INDEX(_idx, _RingSize)	\
	{										  \
		(_idx) = (_idx+1) % (_RingSize);	   \
	}

#define TR_ENQ_COUNT_INC(tr) \
	{								\
		tr->enqCount++;				\
	}

#define TR_ENQ_COUNT_DEC(tr) \
	{								\
		tr->enqCount--;				\
	}

#define TR_TOKEN_COUNT_INC(tr, qid) \
	{								\
		tr->TokenCount[qid]++;				\
	}

#define TR_TOKEN_COUNT_DEC(tr, qid) \
	{								\
		tr->TokenCount[qid]--;				\
	}

#define SQ_ENQ_PS_MAX 32
#define SQ_ENQ_PSQ_MAX 32
#ifdef MEMORY_OPTIMIZATION
#define SQ_ENQ_PSQ_TOTAL_MAX 128
#define SQ_ENQ_RESERVE_PERAC	(1024)
#else
#define SQ_ENQ_PSQ_TOTAL_MAX 1024
#define SQ_ENQ_RESERVE_PERAC	(4096)
#endif
#define SQ_ENQ_NORMAL_MAX	(SQ_ENQ_RESERVE_PERAC * 4)

/*
	Common fragment list structure -  Identical to the scatter gather frag list structure
*/
#define NIC_MAX_PHYS_BUF_COUNT			  8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID Address;
	ULONG Length;
	PULONG Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;

typedef struct _MT_MIB_BUCKET_ONE_SEC {
	UCHAR	Enabled[DBDC_BAND_NUM];
	UINT32 ChannelBusyTime[DBDC_BAND_NUM]; /* Every second update once. primary channel only */
	UINT32 ChannelBusyTimeCcaNavTx[DBDC_BAND_NUM]; /* Every second update once. including CCA NAV TX time*/
	UINT32 OBSSAirtime[DBDC_BAND_NUM];
	UINT32 MyTxAirtime[DBDC_BAND_NUM];
	UINT32 MyRxAirtime[DBDC_BAND_NUM];
	UINT32 EDCCAtime[DBDC_BAND_NUM];
	UINT32 PdCount[DBDC_BAND_NUM];
	UINT32 MdrdyCount[DBDC_BAND_NUM];
	UINT32 WtblRxTime[DBDC_BAND_NUM];
#ifdef CCAPI_API_SUPPORT
	UINT32 TxOpInitTime[DBDC_BAND_NUM];
#endif
} MT_MIB_BUCKET_ONE_SEC, *PMT_MIB_BUCKET_ONE_SEC;

typedef struct _MT_MIB_COUNTER_STAT {
	UINT32 ObssAirtimeAcc[DBDC_BAND_NUM];/* RMAC.AIRTIME14 */
	UINT32 MyTxAirtimeAcc[DBDC_BAND_NUM];/* MIB.M0SDR36 */
	UINT32 MyRxAirtimeAcc[DBDC_BAND_NUM];/* MIB.M0SDR37 */
	UINT32 EdccaAirtimeAcc[DBDC_BAND_NUM];/* RMAC.AIRTIME13 */
	UINT32 CcaNavTxTimeAcc[DBDC_BAND_NUM];/* MIB.M0SDR9 */
	UINT32 PCcaTimeAcc[DBDC_BAND_NUM];/* MIB.M0SDR16 */
	UINT32 WtblRxTimeAcc1[DBDC_BAND_NUM];
	UINT32 WtblRxTimeAcc2[DBDC_BAND_NUM];
#ifdef CCAPI_API_SUPPORT
	UINT32 TxOpInitTimeAcc[DBDC_BAND_NUM];
#endif
#ifdef OFFCHANNEL_ZERO_LOSS
	UINT32 BACountAcc[DBDC_BAND_NUM];/* MIB.M0SDR31 */
	UINT32 MyMac2PhyTxTimeAcc[DBDC_BAND_NUM];/* MIB.M0SDR35 */
#endif
} MT_MIB_COUNTER_STAT, *P_MT_MIB_COUNTER_STAT;

typedef struct _MT_MIB_BUCKET_MS {
	UCHAR	CurIdx;
	UCHAR	Enabled;
	UINT32 ChannelBusyTime[DBDC_BAND_NUM][2];
	UINT32 ChannelBusyTimeCcaNavTx[DBDC_BAND_NUM][2];
	UINT32 OBSSAirtime[DBDC_BAND_NUM][2];
	UINT32 MyTxAirtime[DBDC_BAND_NUM][2];
	UINT32 MyRxAirtime[DBDC_BAND_NUM][2];
	UINT32 EDCCAtime[DBDC_BAND_NUM][2];
	UINT32 PdCount[DBDC_BAND_NUM][2];
	UINT32 MdrdyCount[DBDC_BAND_NUM][2];
#ifdef CCAPI_API_SUPPORT
	UINT32 TxOpInitTime[DBDC_BAND_NUM][2];
#endif
	UINT32 RO_BAND0_PHYCTRL_STS0_OFFSET;
	UINT32 RO_BAND0_PHYCTRL_STS5_OFFSET;
	UINT32 PHY_BAND0_PHYMUX_5_OFFSET;
	UINT32 WtblRxTime[DBDC_BAND_NUM][2];
} MT_MIB_BUCKET_MS, *PMT_MIB_BUCKET_MS;

/*
	Some utility macros
*/
#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)		  (Val.QuadPart++)

#define INFRA_ON(_p)				(STA_STATUS_TEST_FLAG(_p, fSTA_STATUS_INFRA_ON))
#define ADHOC_ON(_p)				(OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#define MONITOR_ON(_p)			  (((_p)->monitor_ctrl.bMonitorOn) == TRUE)
#define IDLE_ON(_pAd, _pStaCfg)				 (!INFRA_ON(_pStaCfg) && !ADHOC_ON(_pAd))

/* Check LEAP & CCKM flags */
#define LEAP_ON(_p)				 (((_p)->StaCfg[0].LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)			((((_p)->StaCfg[0].LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg[0].LeapAuthInfo.CCKM == TRUE))

/* if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)		\
	{																\
		if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500) {		\
			_pExtraLlcSnapEncap = SNAP_802_1H;						\
			if (NdisEqualMemory(IPX, _pBufVA + 12, 2) ||			\
				NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
				_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		} else														\
			_pExtraLlcSnapEncap = NULL;								\
	}

/* New Define for new Tx Path. */
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
	{																\
		if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500) {			\
			_pExtraLlcSnapEncap = SNAP_802_1H;						\
			if (NdisEqualMemory(IPX, _pBufVA, 2) ||				\
				NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
				_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		} else														\
			_pExtraLlcSnapEncap = NULL;								\
	}

#define MAKE_802_3_HEADER(_buf, _pMac1, _pMac2, _pType)				   \
	{																	   \
		NdisMoveMemory(_buf, _pMac1, MAC_ADDR_LEN);						   \
		NdisMoveMemory((_buf + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);		  \
		NdisMoveMemory((_buf + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
	}

/*
	if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel),
		keep it that way.
	else if the received frame is LLC/SNAP-encaped IPX or APPLETALK,
		preserve the LLC/SNAP field
	else remove the LLC/SNAP field from the result Ethernet frame

	Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
	Note:
		_pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
		_pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
*/
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)	  \
	{																	   \
		char LLC_Len[2];													\
		\
		_pRemovedLLCSNAP = NULL;											\
		if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||					 \
			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6)) {				 \
			PUCHAR pProto = _pData + 6;									 \
			\
			if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
				NdisEqualMemory(SNAP_802_1H, _pData, 6)) {					\
				LLC_Len[0] = (UCHAR)(_DataSize >> 8);					   \
				LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));				\
				MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);		  \
			} else {															\
				MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);		   \
				_pRemovedLLCSNAP = _pData;								  \
				_DataSize -= LENGTH_802_1_H;								\
				_pData += LENGTH_802_1_H;								   \
			}															   \
		} else {																\
			LLC_Len[0] = (UCHAR)(_DataSize >> 8);						   \
			LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));					\
			MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);			  \
		}																   \
	}

/*
	Enqueue this frame to MLME engine
	We need to enqueue the whole frame because MLME need to pass data type
	information from 802.11 header
*/
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _Rssi3, \
	_MinSNR, _channel, _OpMode, _wdev, _RxPhyMode)\
	do {																					   \
		struct raw_rssi_info _rssi_info;\
		_rssi_info.raw_rssi[0] = _Rssi0;\
		_rssi_info.raw_rssi[1] = _Rssi1;\
		_rssi_info.raw_rssi[2] = _Rssi2;\
		_rssi_info.raw_rssi[3] = _Rssi3;\
		_rssi_info.raw_snr = _MinSNR;\
		_rssi_info.Channel = _channel;\
		MlmeEnqueueForRecv(_pAd, Wcid, &_rssi_info, _FrameSize, _pFrame, _OpMode, _wdev, _RxPhyMode);   \
	} while (0)

#ifdef OUI_CHECK_SUPPORT
enum {
	OUI_MGROUP_ACT_JOIN = 0,
	OUI_MGROUP_ACT_LEAVE = 1
};
#define MAC_OUI_EQUAL(pAddr1, pAddr2)		   RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#endif /*OUI_CHECK_SUPPORT*/
#define IPV4_ADDR_EQUAL(pAddr1, pAddr2)		 RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 4)
#define IPV6_ADDR_EQUAL(pAddr1, pAddr2)		 RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), 16)
#define MAC_ADDR_EQUAL(pAddr1, pAddr2)		   RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)	((len1 == len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))

#define ONE_SEC_2_US			0xF4240
#define OBSSAIRTIME_TH		  60
#define RX_MIBTIME_CLR_OFFSET   31
#define RX_MIBTIME_EN_OFFSET	30
#define OBSS_OCCUPY_PERCENT_HIGH_TH 40
#define OBSS_OCCUPY_PERCENT_LOW_TH 40
#define EDCCA_OCCUPY_PERCENT_TH 40
#define My_OCCUPY_PERCENT	   15
#define ALL_AIR_OCCUPY_PERCENT 90
#define TX_RATIO_TH			 90

#define BAND0_SPE_IDX 0x18
#define BAND1_SPE_IDX 0x19

/*
	Statistic counter structure
*/
typedef struct _COUNTER_802_3 {
	/* General Stats */
	ULONG GoodTransmits;
	ULONG GoodReceives;
	ULONG TxErrors;
	ULONG RxErrors;
	ULONG RxNoBuffer;
} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG Length;
	/*	LARGE_INTEGER   LastTransmittedFragmentCount; */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TransmitCountFrmOs;
#ifdef OUI_CHECK_SUPPORT
	LARGE_INTEGER RxHWLookupWcidErrCount;
#endif
#ifdef MT_MAC
	LARGE_INTEGER TxAggRange1Count;
	LARGE_INTEGER TxAggRange2Count;
	LARGE_INTEGER TxAggRange3Count;
	LARGE_INTEGER TxAggRange4Count;
	/* for PER debug */
	LARGE_INTEGER AmpduFailCount;
	LARGE_INTEGER AmpduSuccessCount;
	/* for PER debug */
	LARGE_INTEGER CurrentBwTxCount;
	LARGE_INTEGER OtherBwTxCount;
	/* get partial MIB info */
	LARGE_INTEGER RxFcsErrorCount;
	LARGE_INTEGER RxFifoFullCount;
	LARGE_INTEGER RxMpduCount;
	LARGE_INTEGER ChannelIdleCount;
	LARGE_INTEGER CcaNavTxTime;
	LARGE_INTEGER RxMdrdyCount;
	LARGE_INTEGER SCcaTime;
	LARGE_INTEGER PEdTime;
	LARGE_INTEGER RxTotByteCount;
#endif /* MT_MAC */
#ifdef TR181_SUPPORT
	LARGE_INTEGER TxTotByteCount;
	LARGE_INTEGER ucPktsTx;
	LARGE_INTEGER ucPktsRx;
	LARGE_INTEGER mcPktsTx;
	LARGE_INTEGER mcPktsRx;
	LARGE_INTEGER bcPktsTx;
	LARGE_INTEGER bcPktsRx;
	LARGE_INTEGER ucBytesTx;
	LARGE_INTEGER ucBytesRx;
	LARGE_INTEGER mcBytesTx;
	LARGE_INTEGER mcBytesRx;
	LARGE_INTEGER bcBytesTx;
	LARGE_INTEGER bcBytesRx;
#endif
} COUNTER_802_11, *PCOUNTER_802_11;

typedef struct _COUNTER_RALINK {
	UINT32 OneSecStart;	/* for one sec count clear use */
	UINT32 OneSecBeaconSentCnt;
	UINT32 OneSecFalseCCACnt;	/* CCA error count, for debug purpose, might move to global counter */
	UINT32 OneSecRxFcsErrCnt;	/* CRC error */
	UINT32 OneSecRxOkCnt;	/* RX without error */
	UINT32 OneSecTxFailCount;
	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecRxOkDataCnt;	/* unicast-to-me DATA frame count */
	UINT32 OneSecTransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */

	ULONG *OneSecOsTxCount;
	ULONG *OneSecDmaDoneCount;
	UINT32 OneSecTxDoneCount;
	ULONG OneSecRxCount;
	UINT32 OneSecReceivedByteCount;
	UINT32 OneSecTxARalinkCnt;	/* Tx Ralink Aggregation frame cnt */
	UINT32 OneSecRxARalinkCnt;	/* Rx Ralink Aggregation frame cnt */
	UINT32 OneSecEnd;	/* for one sec count clear use */

	ULONG TransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */
	ULONG ReceivedByteCount;	/* both CRC okay and CRC error, used to calculate RX throughput */
	ULONG BadCQIAutoRecoveryCount;
	ULONG PoorCQIRoamingCount;
	ULONG MgmtRingFullCount;
	ULONG RxCountSinceLastNULL;
	ULONG RxCount;
	ULONG KickTxCount;
	LARGE_INTEGER RealFcsErrCount;
	ULONG PendingNdisPacketCount;
	ULONG FalseCCACnt;					/* CCA error count */

	UINT32 LastOneSecTotalTxCount;	/* OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount */
	UINT32 LastOneSecRxOkDataCnt;	/* OneSecRxOkDataCnt */
	ULONG DuplicateRcv;
	ULONG TxAggCount;
	ULONG TxNonAggCount;
	ULONG TxAgg1MPDUCount;
	ULONG TxAgg2MPDUCount;
	ULONG TxAgg3MPDUCount;
	ULONG TxAgg4MPDUCount;
	ULONG TxAgg5MPDUCount;
	ULONG TxAgg6MPDUCount;
	ULONG TxAgg7MPDUCount;
	ULONG TxAgg8MPDUCount;
	ULONG TxAgg9MPDUCount;
	ULONG TxAgg10MPDUCount;
	ULONG TxAgg11MPDUCount;
	ULONG TxAgg12MPDUCount;
	ULONG TxAgg13MPDUCount;
	ULONG TxAgg14MPDUCount;
	ULONG TxAgg15MPDUCount;
	ULONG TxAgg16MPDUCount;

	LARGE_INTEGER TxAMSDUCount;
	LARGE_INTEGER RxAMSDUCount;
	LARGE_INTEGER TransmittedAMPDUCount;
	LARGE_INTEGER TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER MPDUInReceivedAMPDUCount;

	ULONG PhyErrCnt;
	ULONG PlcpErrCnt;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
	/*BOOLEAN		 fNoisyEnvironment; */
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	UCHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;

#ifdef STREAM_MODE_SUPPORT
typedef struct _STREAM_MODE_ENTRY_ {
#define STREAM_MODE_STATIC		1
	USHORT flag;
	UCHAR macAddr[MAC_ADDR_LEN];
} STREAM_MODE_ENTRY;
#endif /* STREAM_MODE_SUPPORT */

/* for Microwave oven */
#ifdef MICROWAVE_OVEN_SUPPORT
typedef struct _MO_CFG_STRUCT {
	BOOLEAN		bEnable;
	UINT8		nPeriod_Cnt;	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16		nFalseCCACnt;
	UINT16		nFalseCCATh;	/* default is 100 */
} MO_CFG_STRUCT, *PMO_CFG_STRUCT;
#endif /* MICROWAVE_OVEN_SUPPORT */

/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
/* for dynamic vga */
typedef struct _LNA_VGA_CTL_STRUCT {
	BOOLEAN		bEnable;
	BOOLEAN		bDyncVgaEnable;
	UINT8		nPeriod_Cnt;	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16		nFalseCCACnt;
	UINT16		nFalseCCATh;	/* default is 100 */
	UINT16		nLowFalseCCATh;
	UCHAR		agc_vga_init_0;
	UCHAR		agc_vga_ori_0; /* the original vga gain initialized by firmware at start up */
	UINT16		agc_0_vga_set1_2;
	UCHAR		agc_vga_init_1;
	UCHAR		agc_vga_ori_1; /* the original vga gain initialized by firmware at start up */
	UINT16		agc_1_vga_set1_2;
} LNA_VGA_CTL_STRUCT, *PLNA_VGA_CTL_STRUCT;
#endif /* DYNAMIC_VGA_SUPPORT */

/***************************************************************************
  *	security key related data structure
  **************************************************************************/

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY, *PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;

typedef struct {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ErrorCode[2];	/*00 01-Invalid authentication type */
	/*00 02-Authentication timeout */
	/*00 03-Challenge from AP failed */
	/*00 04-Challenge to AP failed */
	BOOLEAN Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UINT RogueApNr;
	ROGUEAP_ENTRY RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct _FRAGMENT_FRAME {
	PNDIS_PACKET pFragPacket;
	ULONG RxSize;
	USHORT Sequence;
	USHORT LastFrag;
	ULONG Flags;		/* Some extra frame information. bit 0: LLC presented */
	BOOLEAN Header_802_3;
	UINT16 wcid;
	BOOLEAN sec_on;
	UINT8 sec_mode;
#ifdef FRAG_ATTACK_SUPPORT
	UINT8 key[4];
#endif
	UINT64 LastPN;
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;

/*
	Tkip Key structure which RC4 key & MIC calculation
*/
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	UINT32 IV16;
	UINT32 IV32;
	UINT32 K0;		/* for MICKEY Low */
	UINT32 K1;		/* for MICKEY Hig */
	UINT32 L;		/* Current state for MICKEY */
	UINT32 R;		/* Current state for MICKEY */
	UINT32 M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;

/*
	Private / Misc data, counters for driver internal use
*/
typedef struct __PRIVATE_STRUC {
	/* Tx ring full occurrance number */
	UINT TxRingFullCnt;
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;

struct EDCCA_6G_CHANNEL_NODE {
	UCHAR channel;
	BOOL scanning;
	BOOL hasbeacon;
};

/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
/* structure to tune BBP R66 (BBP TUNING) */
typedef struct _BBP_R66_TUNING {
	BOOLEAN bEnable;
	USHORT FalseCcaLowerThreshold;	/* default 100 */
	USHORT FalseCcaUpperThreshold;	/* default 512 */
	UCHAR R66Delta;
	UCHAR R66CurrentValue;
	BOOLEAN R66LowerUpperSelect;	/*Before LinkUp, Used LowerBound or UpperBound as R66 value. */
} BBP_R66_TUNING, *PBBP_R66_TUNING;

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY		0x4
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	UCHAR Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN bEffectedChannel;	/* For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz. */
#endif /* DOT11N_DRAFT3 */
	CHAR Power;
	CHAR Power2;
	UCHAR MaxTxPwr;

	/* DFS */
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;
	UCHAR NOPClrCnt;
	UCHAR SupportBwBitMap;
	UCHAR NOPSetByBw;
	USHORT NOPSaveForClear;
	USHORT NonOccupancy;	/* NOP, unit: sec */

#ifdef CONFIG_6G_SUPPORT
	UCHAR PSC_Ch;
#endif
	/*
		Channel property:
		CHANNEL_DISABLED: The channel is disabled.
		CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
		CHANNEL_NO_IBSS: IBSS is not allowed.
		CHANNEL_RADAR: Radar detection is required.
		CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
		CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
		CHANNEL_40M_CAP: 40 BW channel group
		CHANNEL_80M_CAP: 800 BW channel group
	 */
#define CHANNEL_DEFAULT_PROP		 0x00
#define CHANNEL_DISABLED			 0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN		 0x02
#define CHANNEL_NO_IBSS				 0x04
#define CHANNEL_RADAR				 0x08
#define CHANNEL_NO_FAT_ABOVE		 0x10
#define CHANNEL_NO_FAT_BELOW		 0x20
#define CHANNEL_40M_CAP				 0x40
#define CHANNEL_80M_CAP				 0x80
#define CHANNEL_160M_CAP			0x100

	UINT Flags;
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

typedef enum  {
	CH_LIST_STATE_NONE = 0,
	CH_LIST_STATE_DONE,
} CH_LIST_STATE;

typedef struct _NOP_LIST {
	CHANNEL_TX_POWER DfsChList[DFS_AVAILABLE_LIST_CH_NUM];
	UCHAR ChListNum;
} NOP_LIST, *PNOP_LIST;

typedef struct _CHANNEL_CTRL {
	CHANNEL_TX_POWER ChList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */
	UCHAR ChListNum; /* number of channel in ChannelList[] */
	/* Channel Group*/
	UCHAR ChGrpABandEn;
	UCHAR ChGrpABandChList[MAX_NUM_OF_CHANNELS];
	UCHAR ChGrpABandChNum;
	CH_LIST_STATE ChListStat; /* State of channel list, 0: None, 1: Done */
#ifdef CONFIG_6G_SUPPORT
	CHANNEL_TX_POWER ChList_6G_scan[MAX_NUM_OF_CHANNELS];
	UCHAR ChListNum_6G_scan;
#endif
} CHANNEL_CTRL, *PCHANNEL_CTRL;
/* Channel list subset */
typedef struct _CHANNEL_LIST_SUB {
	UCHAR	Channel;
	UCHAR	IdxMap; /* Index mapping to original channel list */
} CHANNEL_LIST_SUB, *PCHANNEL_LIST_SUB;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR EvaluatePeriod;	/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR EvaluateStableCnt;
	UCHAR Pair1PrimaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT Pair1AvgRssi[2];	/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT Pair2AvgRssi[2];	/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT Pair1LastAvgRssi;	/* */
	SHORT Pair2LastAvgRssi;	/* */
	ULONG RcvPktNumWhenEvaluate;
	BOOLEAN FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG Pair1AvgRssiGroup1[2];
	LONG Pair1AvgRssiGroup2[2];
	ULONG RcvPktNum[2];
#endif /* CONFIG_AP_SUPPORT */
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef RTMP_MAC_PCI
/* Power save method control */
typedef union _PS_CONTROL {
	struct {
		ULONG EnablePSinIdle:1;	/* Enable radio off when not connect to AP. radio on only when sitesurvey, */
		ULONG EnableNewPS:1;	/* Enable new  Chip power save fucntion . New method can only be applied in chip version after 2872. and PCIe. */
		ULONG rt30xxPowerMode:2;	/* Power Level Mode for rt30xx chip */
		ULONG rt30xxFollowHostASPM:1;	/* Card Follows Host's setting for rt30xx chip. */
		ULONG rt30xxForceASPMTest:1;	/* Force enable L1 for rt30xx chip. This has higher priority than rt30xxFollowHostASPM Mode. */
		/*		ULONG		rsv:26;			// Radio Measurement Enable */
		ULONG AMDNewPSOn:1;	/* Enable for AMD L1 (toggle) */
		ULONG LedMode:2;	/* 0: Blink normal.  1: Slow blink not normal. */
		ULONG rt30xxForceL0:1;	/* Force only use L0 for rt30xx */

		/* PCIe config space [Completion TimeOut Disable], compatible issue with Intel HM55 */
		ULONG CTO:1;	/* 0: default, update the CTO bit to disable; 1: Keep BIOS config value */
		ULONG PM4PCIeCLKOn:1;	/* 0: default, turn off PCIE CLk at PM4; 1: FW MCU cmd arg1 as "0x5a" which will not turn off PCIE CLK */

		ULONG rsv:20;	/* Rsvd */
	} field;
	ULONG word;
} PS_CONTROL, *PPS_CONTROL;
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef REPEATER_TX_RX_STATISTIC
typedef struct _RETR_TX_RX_INFO {
	UINT64 TxPackets;
	UINT64 TxBytes;
	UINT64 TxFailPackets;
	UINT64 TxDropPackets;
	UINT64 RxPackets;
	UINT64 RxBytes;
} RETRTXRXINFO, *pRETRTXRXINFO;
#endif /* REPEATER_TX_RX_STATISTIC */
#endif /* APCLI_SUPPORT */

/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	/* STA state machines */
#ifdef DOT11R_FT_SUPPORT
	STATE_MACHINE FtOtaAuthMachine;
	STATE_MACHINE_FUNC FtOtaAuthFunc[FT_OTA_AUTH_FUNC_SIZE];
	STATE_MACHINE FtOtdActMachine;
	STATE_MACHINE_FUNC FtOtdActFunc[FT_OTD_FUNC_SIZE];
#endif /* DOT11R_FT_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */
	STATE_MACHINE_FUNC ActFunc[ACT_FUNC_SIZE];
	/* Action */
	STATE_MACHINE ActMachine;

#ifdef WSC_INCLUDED
	STATE_MACHINE WscMachine;
	STATE_MACHINE_FUNC WscFunc[WSC_FUNC_SIZE];

#ifdef IWSC_SUPPORT
	STATE_MACHINE			IWscMachine;
	STATE_MACHINE_FUNC		IWscFunc[IWSC_FUNC_SIZE];
#endif /* IWSC_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef DOT11Z_TDLS_SUPPORT
	STATE_MACHINE TdlsMachine;
	STATE_MACHINE_FUNC TdlsFunc[TDLS_FUNC_SIZE];
	STATE_MACHINE TdlsChSwMachine;
	STATE_MACHINE_FUNC TdlsChSwFunc[TDLS_CHSW_FUNC_SIZE];
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef CONFIG_HOTSPOT
	STATE_MACHINE HSCtrlMachine;
	STATE_MACHINE_FUNC HSCtrlFunc[GAS_FUNC_SIZE];
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
	STATE_MACHINE GASMachine;
	STATE_MACHINE_FUNC GASFunc[GAS_FUNC_SIZE];
#endif

#ifdef DOT11K_RRM_SUPPORT
	STATE_MACHINE BCNMachine;
	STATE_MACHINE_FUNC BCNFunc[BCN_FUNC_SIZE];
	STATE_MACHINE NRMachine;
	STATE_MACHINE_FUNC NRFunc[NR_FUNC_SIZE];
#endif

#ifdef CONFIG_DOT11V_WNM
	STATE_MACHINE BTMMachine;
	STATE_MACHINE_FUNC BTMFunc[BTM_FUNC_SIZE];
	STATE_MACHINE WNMNotifyMachine;
	STATE_MACHINE_FUNC WNMNotifyFunc[WNM_NOTIFY_FUNC_SIZE];
#endif

#ifdef CONFIG_AP_SUPPORT
	/* AP state machines */
#ifdef APCLI_SUPPORT
	STATE_MACHINE ApCliCtrlMachine;
	STATE_MACHINE ApCliWpaPskMachine;

	STATE_MACHINE_FUNC ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
#endif /* APCLI_SUPPORT */
	ULONG ChannelQuality;   /* 0..100, Channel Quality Indication for Roaming */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WDS_SUPPORT
	STATE_MACHINE WdsMachine;
	STATE_MACHINE_FUNC WdsFunc[WDS_FUNC_SIZE];
#endif
	/* common WPA state machine */
	STATE_MACHINE WpaMachine;
	STATE_MACHINE_FUNC WpaFunc[WPA_FUNC_SIZE];


	ULONG Now32;		/* latch the value of NdisGetSystemUpTime() */
	ULONG LastSendNULLpsmTime;

	BOOLEAN bRunning;
	BOOLEAN suspend;
	NDIS_SPIN_LOCK TaskLock;
	RTMP_OS_COMPLETION mlme_halt_done;
	MLME_QUEUE Queue;
#ifdef MLME_MULTI_QUEUE_SUPPORT
	BOOLEAN MultiQEnable;
	MLME_HP_QUEUE HPQueue;
	MLME_LP_QUEUE LPQueue;
#endif /*MLME_MULTI_QUEUE_SUPPORT */
	/*used for record jiffies than define as ULONG*/
	ULONG ShiftReg;

#ifdef BT_COEXISTENCE_SUPPORT
	RALINK_TIMER_STRUCT MiscDetectTimer;
#endif /* BT_COEXISTENCE_SUPPORT */
	RALINK_TIMER_STRUCT PeriodicTimer;
	RALINK_TIMER_STRUCT APSDPeriodicTimer;

	RALINK_TIMER_STRUCT LinkUpTimer;
#ifdef RTMP_MAC_PCI
	UCHAR bPsPollTimerRunning;
	RALINK_TIMER_STRUCT PsPollTimer;
	RALINK_TIMER_STRUCT RadioOnOffTimer;
#endif /* RTMP_MAC_PCI */
	ULONG PeriodicRound;
	ULONG GPIORound;
	ULONG OneSecPeriodicRound;

	UCHAR RealRxPath;
	BOOLEAN bLowThroughput;
	BOOLEAN bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT RxAntEvalTimer;


#ifdef CONFIG_MULTI_CHANNEL

	BOOLEAN bStartMcc;
	BOOLEAN bpkt_dbg;

	UINT32 channel_1st_staytime;
	UINT32 channel_2nd_staytime;
	UINT32 switch_idle_time;
	UINT32 null_frame_count;

	UINT32 channel_1st_bw;
	UINT32 channel_2nd_bw;
	UINT32 channel_1st_primary_ch;
	UINT32 channel_2nd_primary_ch;
	UINT32 channel_1st_center_ch;
	UINT32 channel_2nd_center_ch;

	ULONG BeaconNow32;		/* latch the value of NdisGetSystemUpTime() */
#endif /* CONFIG_MULTI_CHANNEL */
	BOOLEAN bStartScc;
	struct notify_entry wsys_ne;
} MLME_STRUCT, *PMLME_STRUCT;

#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_REC_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize */
	UCHAR rsv;
	UCHAR BufSize[8];
	REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_ORI_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status */
	UCHAR rsv;
	UCHAR BufSize[8];
	ORI_BLOCKACK_STATUS ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE {
	OID_BA_ORI_ENTRY BAOriEntry[32];
	OID_BA_REC_ENTRY BARecEntry[32];
	UCHAR OriNum;		/* Number of below BAOriEntry */
	UCHAR RecNum;		/* Number of below BARecEntry */
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union _BACAP_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]		= { 3839, 7935}; */
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 MpduDensity:3;
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 TxBAWinLimit:10;
		UINT32 RxBAWinLimit:10;
	} field;
#else
	struct {
		UINT32 RxBAWinLimit:10;
		UINT32 TxBAWinLimit:10;
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 MpduDensity:3;
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]		= { 3839, 7935}; */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
	} field;
#endif
	UINT32 word;
} BACAP_STRUC, *PBACAP_STRUC;

typedef struct {
	BOOLEAN IsRecipient;
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR TID;
	UCHAR nMSDU;
	USHORT TimeOut;
	BOOLEAN bAllTid;	/* If True, delete all TID for BA sessions with this MACaddr. */
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

#define WLAN_MAX_NUM_OF_TIM		 ((MAX_LEN_OF_MAC_TABLE >> 3) + 1)   /* /8 + 1 */

enum BCN_TX_STATE {
	BCN_TX_UNINIT = 0,
	BCN_TX_IDLE = 1,
	BCN_TX_WRITE_TO_DMA = 2,
	BCN_TX_DMA_DONE = 3
};

typedef enum {
	PHY_IDLE = 0,
	PHY_INUSE = 1,
	PHY_RADIOOFF = 2,
} PHY_STATUS;

typedef struct _BCN_BUF_STRUCT {
	struct wifi_dev *pWdev;			/* point to associated wdev.*/
	enum BCN_TX_STATE bcn_state;	/* Make sure if no packet pending in the Hardware */
	BOOLEAN bBcnSntReq;				/* used in if beacon send or stop */
	PNDIS_PACKET BeaconPkt;
	UINT16 FrameLen;

	UCHAR cap_ie_pos;

	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];
	UINT16 TimIELocationInBeacon;

	UINT16 CsaIELocationInBeacon;

	NDIS_SPIN_LOCK BcnContentLock;
	UCHAR BcnUpdateMethod;

#ifdef DOT11_HE_AX
	UINT16 bcc_ie_location;
#endif
} BCN_BUF_STRUCT, *PBCN_BUF_STRUCT;

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
typedef struct _BTWT_BUF_STRUCT {
	BOOLEAN btwt_element_exist;
	BOOLEAN support_btwt_id_0;
	UINT8 btwt_element_num;
	UCHAR reserved;
	UINT16 btwt_bcn_offset;
	UINT16 btwt_probe_rsp_offset;
	UINT32 schedule_sp_start_tsf[TWT_HW_BTWT_MAX_NUM];
	UCHAR btwt_element[sizeof(struct btwt_ie)];
} BTWT_BUF_STRUCT, *PBTWT_BUF_STRUCT;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG {
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
} BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_ {
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
} BSS_COEX_CH_RANGE;

typedef struct _BSS_COEX_SCAN_LAST_RESULT {
	UCHAR   WdevIdx;
	BOOLEAN bNeedFallBack;
	ULONG   LastScanTime;
} BSS_COEX_SCAN_LAST_RESULT, *P_BSS_COEX_SCAN_LAST_RESULT;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/*This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic) */
typedef struct _IOT_STRUC {
	BOOLEAN bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bLastAtheros;
	/* BOOLEAN bCurrentAtheros; */
	/* BOOLEAN bNowAtherosBurstOn; */
	/* BOOLEAN bNextDisableRxBA; */
	BOOLEAN bToggle;
#endif /* CONFIG_STA_SUPPORT */
} IOT_STRUC;

/* This is the registry setting for 802.11n transmit setting.  Used in advanced page. */
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rsv:13;
		UINT32 EXTCHA:2;
		UINT32 HTMODE:1;
		UINT32 TRANSNO:2;
		UINT32 STBC:1;	/*SPACE */
		UINT32 ShortGI:1;
		UINT32 TxBF:1;	/* 3*3 */
		UINT32 ITxBfEn:1;
		UINT32 rsv0:10;
		/*UINT32  MCS:7;				 // MCS */
		/*UINT32  PhyMode:4; */
	} field;
#else
	struct {
		/*UINT32  PhyMode:4; */
		/*UINT32  MCS:7;				 // MCS */
		UINT32 rsv0:10;
		UINT32 ITxBfEn:1;
		UINT32 TxBF:1;
		UINT32 ShortGI:1;
		UINT32 STBC:1;	/*SPACE */
		UINT32 TRANSNO:2;
		UINT32 HTMODE:1;
		UINT32 EXTCHA:2;
		UINT32 rsv:13;
	} field;
#endif
	UINT32 word;
} REG_TRANSMIT_SETTING;

typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:2;
		USHORT FixedTxMode:3;	/* If MCS isn't AUTO, fix rate in CCK, OFDM, HT or VHT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;
		USHORT PhyMode:4;
		USHORT FixedTxMode:3;
		USHORT rsv:2;
	} field;
#endif
	USHORT word;
} DESIRED_TRANSMIT_SETTING;

struct hw_setting {
	CHAR lan_gain;
};

/** @ingroup wifi_dev_system */
enum WDEV_TYPE {
	WDEV_TYPE_AP = (1 << 0),
	WDEV_TYPE_STA = (1 << 1),
	WDEV_TYPE_ADHOC = (1 << 2),
	WDEV_TYPE_WDS = (1 << 3),
	WDEV_TYPE_MESH = (1 << 4),
	WDEV_TYPE_GO = (1 << 5),
	WDEV_TYPE_GC = (1 << 6),
	/* WDEV_TYPE_APCLI = (1 << 7), */
#ifdef TXRX_STAT_SUPPORT
	WDEV_TYPE_APCLI = (1 << 7),
#endif
	WDEV_TYPE_REPEATER = (1 << 8),
	WDEV_TYPE_P2P_DEVICE = (1 << 9),
	WDEV_TYPE_TDLS = (1 << 10),
	WDEV_TYPE_SERVICE_TXC = (1 << 11),
	WDEV_TYPE_SERVICE_TXD = (1 << 12),
	WDEV_TYPE_ATE_AP = (1 << 13),	/* For TX with TXC */
	WDEV_TYPE_ATE_STA = (1 << 14)	/* For TX with TXD */
};

#define WDEV_WITH_BCN_ABILITY(_wdev)	 (((_wdev)->wdev_type == WDEV_TYPE_AP) || \
		((_wdev)->wdev_type == WDEV_TYPE_GO) || \
		((_wdev)->wdev_type == WDEV_TYPE_MESH) || \
		((_wdev)->wdev_type == WDEV_TYPE_ADHOC))

enum BSS_INFO_DRIVER_MAINTIAN_STATE {
	BSS_INFO_INIT = 0,
	BSS_INFO_SEND_ENABLE = 1,
	BSS_INFO_SEND_DISABLE = 2,
};

enum MSDU_FORBID_REASON {
	MSDU_FORBID_CONNECTION_NOT_READY = 0,
	MSDU_FORBID_CHANNEL_MISMATCH = 1,
};

#define FLG_IS_OUTPUT 1
#define FLAG_IS_INPUT 0
#define MSDU_FORBID_SET(_wdev, _reason)	   (OS_SET_BIT(_reason, &((_wdev)->forbid_data_tx)))
#define MSDU_FORBID_CLEAR(_wdev, _reason)	 (OS_CLEAR_BIT(_reason, &((_wdev)->forbid_data_tx)))

#ifdef MAC_REPEATER_SUPPORT
typedef struct _RX_TA_TID_SEQ_MAPPING {
	UINT8   RxDWlanIdx;
	UINT8   MuarIdx;
	UINT16  TID_SEQ[8];
	UINT8   LatestTID;
} RX_TA_TID_SEQ_MAPPING, *PRX_TA_TID_SEQ_MAPPING;

typedef struct _RX_TRACKING_T {
	RX_TA_TID_SEQ_MAPPING   LastRxWlanIdx;
	UINT32  TriggerNum;
} RX_TRACKING_T, *PRX_TRACKING_T;
#endif /* MAC_REPEATER_SUPPORT */

struct _TX_BLK;

#ifdef IGMP_TVM_SUPPORT
typedef struct _MULTICAST_BLACK_LIST_ENTRY {
	BOOLEAN bValid; /* True or False */
	UCHAR EntryIPType; /* IPv4 or IPv6 */
	BOOLEAN bStatic; /* Specifies it as static or dynamic */
	union {
		UCHAR IPv4[IPV4_ADDR_LEN];
		UCHAR IPv6[IPV6_ADDR_LEN];
	} IPData;
	union {
		UINT8 Byte[sizeof(UINT32)*4];
		UINT32	DWord[sizeof(UINT32)];
	} PrefixMask;
	UCHAR PrefixLen;
} MULTICAST_BLACK_LIST_ENTRY, *PMULTICAST_BLACK_LIST_ENTRY;

typedef struct _MULTICAST_BLACK_LIST_FILTER_TABLE {
	UCHAR EntryNum;
	MULTICAST_BLACK_LIST_ENTRY EntryTab[MULTICAST_BLACK_LIST_SIZE_MAX];
} MULTICAST_BLACK_LIST_FILTER_TABLE, *PMULTICAST_BLACK_LIST_FILTER_TABLE;
#endif /* IGMP_TVM_SUPPORT */


/**
 * @send_mlme_pkt: TX mlme pakcet en-queue pre handle that will call enq_data_pkt/enq_mgmt_pkt depend on which queue type that enqueue to
 * @send_data_pkt: TX data packet en-queue (TX sw queue) pre handle
 * @fp_send_data_pkt: TX data packet en-queue (TX sw queue) pre handle for fast path
 * @tx_pkt_allowed: early check for allow unicast/multicast packet to send or not
 * @fp_tx_pkt_allowed:  early check for allow unicast/multicast packet to send or not for fast path
 * @tx_pkt_handle: TX data per packet handle
 * @fill_offload_tx_blk: fill tx_blk information to build 802.11 header and hw descriptor header for txd offload frame
 * @fill_non_offload_tx_blk: fill tx_blk information to build 802.11 header and hw descriptor header for non-txd offload frame
 * @legacy_tx: build tx_blk, 802.11 header if not enable header translation and txd header then kick out to HIF
 * @ampdu_tx: same as legacy_tx except that can do cache operation (802.11 header cache)
 * @frag_tx: fragment TX data packet according to fragment threshold, fragment do not support header translation, so need to build 802.11 header
 * @amsdu_tx: aggrgation msdu packet into a-msdu packet
 * @mlme_mgmtq_tx: build tx_blk, and txd header and kick out to HIF for MLME frame that dequeue from management sw queue
 * @mlme_dataq_tx: build tx_blk, and txd header and kick out to HIF for MLME frame that dequeue from data sw queue
 * @ate_tx: build tx_blk, and txd header and kick out to HIF for ATE frames
 * @verify_tx: build tx_blk, and txd header and kick out to HIF for VERIFICATION frames
 * @ieee_802_11_data_tx: build 802.11 header
 * @ieee_802_3_data_tx: for header translation disable case, do not need build 802.11 header any more
 * @rx_pkt_allowed: early check for allow unicast/multicast packet to receive or not
 * @ieee_802_11_data_rx: per 802.11 data packet handle (build RX_BLK, process 802.11 header nad  indicate to TCP layer or forward to other STA)
 * @ieee_802_3_data_rx: per 802.3 data packet handle (build RX_BLK and indicate to TCP layer or forward to other STA)
 * @ieee_802_11_ctl_rx: per 802.11 control packet handle (receive control frame and en-queue to MLME state machine)
 * @rx_pkt_forward: per 802.3 data packet forward handing
 * @find_cipher_algorithm: find encrypt key and decide cipher alogorithm
 * @detect_wmm_traffic: detect wmm traffic for purpose that determine if turn off txop or not
 */

struct wifi_dev_ops {
	/* TX */
	INT (*send_data_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt) ____cacheline_aligned;
	INT (*fp_send_data_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*send_mlme_pkt)(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
	INT (*tx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*fp_tx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	INT (*tx_pkt_handle)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*legacy_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*ampdu_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*amsdu_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*frag_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*mlme_mgmtq_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*mlme_dataq_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*ate_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	INT (*verify_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	BOOLEAN (*fill_offload_tx_blk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	BOOLEAN (*fill_non_offload_tx_blk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*ieee_802_11_data_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*ieee_802_3_data_tx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);

	/* RX */
	INT (*rx_pkt_allowed)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk) ____cacheline_aligned;
	INT (*rx_pkt_hdr_chk)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*rx_ps_handle)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*ieee_802_11_data_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
							   struct _RX_BLK *rx_blk, struct _MAC_TABLE_ENTRY *entry);
	INT (*ieee_802_3_data_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
							  struct _RX_BLK *rx_blk, struct _MAC_TABLE_ENTRY *entry);
	INT (*ieee_802_11_mgmt_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*ieee_802_11_ctl_rx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *rx_blk);
	INT (*rx_pkt_foward)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
	VOID (*find_cipher_algorithm)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
	VOID (*detect_wmm_traffic)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR up, UCHAR flg_is_output);

	/* General */
	INT (*open)(struct wifi_dev *wdev) ____cacheline_aligned;
	INT (*close)(struct wifi_dev *wdev);
	INT (*linkup)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	INT (*linkdown)(struct wifi_dev *wdev);
	INT (*conn_act)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	INT (*disconn_act)(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
	VOID (*mac_entry_lookup)(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY **entry);

	BOOLEAN (*media_state_connected)(struct wifi_dev *wdev);
	INT (*ioctl)(void *net_dev_obj, void *rq_obj, INT cmd);

};

enum phy_modes {
	LEGACY_CCK,
	LEGACY_OFDM,
	HT_MM,
	HT_GF,
	VHT_PHY,
	HE_SU_MODE,
	HE_EXT_SU,
	HE_TRIG,
	HE_MU
};


enum gi_types {
	GI_08_US,
	GI_16_US,
	GI_32_US,
	GI_MASK
};

enum ltf_types {
	LTF_1x,
	LTF_2x,
	LTF_4x,
	LTF_MASK
};

#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
enum fixed_gi_mode {
	GI_BY_WCID,
	GI_BY_BSS,
	GI_MODE_MAX
};
#endif

#define GI_AUTO	0
#define FIXED_GI_08_US	1
#define FIXED_GI_16_US	2
#define FIXED_GI_32_US	3

struct conn_sta_info {
	UINT16 nonerp_sta_cnt;
	UINT16 nongf_sta_cnt;
	UINT16 ht_bw20_sta_cnt;
};

#ifdef DPP_SUPPORT
struct dpp_frame_list {
	DL_LIST List;
	struct wapp_event *dpp_frame_event;
};
#endif

#ifdef DPP_R2_SUPPORT
typedef struct _DPP_CONFIG {
	UCHAR cce_ie_buf[6];
	UCHAR cce_ie_len;
} DPP_CONFIG, *PDPP_CONFIG;
#endif


enum {
	QUICK_CH_SWICH_DISABLE,
	QUICK_CH_SWICH_ENABLE
};

#ifdef CONFIG_6G_SUPPORT
/* bss feature set */
#define AP_6G_TRANS_BSSID			(1 << 0)
#define AP_6G_MULTI_BSSID			(1 << 1)
#define AP_6G_UNSOL_PROBE_RSP_EN	(1 << 2)
#define MAX_REPTED_BSS_CNT			64

typedef struct _repted_bss_info {
	USHORT		phymode;
	USHORT		bss_grp_idx;
	UINT32		bss_feature_set;
	UCHAR		channel;
	UCHAR		op_class;
	UCHAR		ssid_len;
	UCHAR		rsvd1;
	CHAR		ssid[MAX_LEN_OF_SSID];
	UCHAR		bssid[MAC_ADDR_LEN];
	UCHAR		rsvd2[2];
} repted_bss_info, *prepted_bss_info;

typedef struct cfg_discov_in_of_band {
	UINT16 				pkt_len;
	PUCHAR 				pkt_buf;	/* TXD + Discovery.Pkt */
	NDIS_SPIN_LOCK 		pkt_lock;
} discov_iob, *pdiscov_iob;

typedef struct cfg_discov_out_of_band {
	UINT16				repted_bss_cnt;		/* reported bss count */
	prepted_bss_info	repted_bss_list;	/* reported bss list */
	NDIS_SPIN_LOCK 		list_lock;
} discov_oob, *pdiscov_oob;

typedef struct _ap_6g_cfg {
	discov_iob		dsc_iob;
	discov_oob		dsc_oob;
} ap_6g_cfg, *pap_6g_cfg;
#endif /* CONFIG_6G_SUPPORT */

/**
 * @wdev_idx: index refer from pAd->wdev_list[]
 * @func_idx: index refer to func_dev which pointer to
 * @tr_tb_idx: index refer to BSS which this device belong to
 * @wdev_ops: wifi device operation
 */

struct wifi_dev {
	PNET_DEV if_dev;
	VOID *func_dev;
	VOID *sys_handle;
	CHAR wdev_idx;	/* index refer from pAd->wdev_list[] */
	CHAR BssIdx;
	UCHAR func_idx; /* index refer to func_dev which pointer to */
	CHAR func_type; /*Indicator to func type*/
	UINT16 tr_tb_idx; /* index refer to BSS which this device belong */
#ifdef SW_CONNECT_SUPPORT
	UINT16 hw_bmc_wcid; /* mapping to bmc hw wcid usage */
	PDUMMY_WCID_OBJ pDummy_obj; /* mapping to dummy wcid usage */
#endif /* SW_CONNECT_SUPPORT */
	UINT32 wdev_type;
	USHORT PhyMode;
	UCHAR channel;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	BOOLEAN bV10OldChannelValid; /* Retain Previous Channel during Driver Reload */
#endif
	UCHAR if_addr[MAC_ADDR_LEN];
	UCHAR bssid[MAC_ADDR_LEN];
	UCHAR hw_bssid_idx;
	BOOLEAN if_up_down_state;
	BOOLEAN open_state;
	BOOLEAN start_stop_running;
	RTMP_OS_COMPLETION start_stop_complete;
	UINT8 OmacIdx;
	UCHAR state;
	/* security segment */
	struct _SECURITY_CONFIG SecConfig;
#ifdef RT_CFG80211_SUPPORT
	UCHAR Hostapd_GTK[LEN_MAX_GTK];
	BOOLEAN Is_hostapd_gtk;
#endif
	UCHAR PortSecured;

#ifdef DOT11R_FT_SUPPORT
	FT_CFG FtCfg;
#endif /* DOT11R_FT_SUPPORT */
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	BOOLEAN isRssiEnbl; /* RSSI Enable */
#endif
	/* transmit segment */
	ULONG forbid_data_tx;  /* Use long, becasue we want to do atomic bit operation */
	BOOLEAN IgmpSnoopEnable; /* Only enabled for AP/WDS mode */
#ifdef IGMP_TVM_SUPPORT
	UCHAR IsTVModeEnable; /* Valid for both AP and Apcli wdev */
	UCHAR TVModeType; /* 0:Disable, 1:Enable, or 2:Auto, Valid only for AP wdev*/
	MULTICAST_BLACK_LIST_FILTER_TABLE McastBLTable;
	/* This temporary table is used to have book keeping of all the entries received from CR4 in multiple events */
	P_IGMP_MULTICAST_TABLE pIgmpMcastTable;
	UINT32 IgmpTableSize;
	UINT_32 u4AgeOutTime; /* Time in sec */
#endif /* IGMP_TVM_SUPPORT */

	RT_PHY_INFO DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;
	BOOLEAN bAutoTxRateSwitch;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	HE_TRANSMIT_SETTING HEPhyMode, MaxHEPhyMode;
	struct phy_params phy_param;

	/* 802.11 protocol related characters */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	UCHAR EdcaIdx;	/*mapping edca parameter to CommonCfg.APEdca[EdcaIdx]*/
	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

	/* for protocol layer using */
	UINT32 protection;

	/* tx burst */
	UINT32 prio_bitmap;
	UINT16 txop_level[MAX_PRIO_NUM];

	/* VLAN related */
	BOOLEAN bVLAN_Tag;
	USHORT VLAN_VID;
	USHORT VLAN_Priority;
	USHORT VLAN_Policy[2]; /*[0] for Tx , [1] for Rx*/
	USHORT VLANRxdrop;
	USHORT VLANTxdrop;
#ifdef OFFCHANNEL_SCAN_FEATURE
	UCHAR restore_channel;
#endif
	struct wifi_dev_ops *wdev_ops;

	/* last received packet's SNR for each antenna */
	UCHAR LastSNR0;
	UCHAR LastSNR1;
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;
	BOOLEAN bLinkUpDone;
	BOOLEAN bGotEapolPkt;
	struct _RX_BLK *pEapolPktFromAP;

	UCHAR mgmt_txd_txpwr_offset;

	/* 7636 psm */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE), Please use this value to replace  pAd->StaCfg[0].Psm in the future*/
	BOOLEAN bBeaconLost;
	BOOLEAN bTriggerRoaming;
	UINT8	ucDtimPeriod;
	UINT8	ucBeaconPeriod;
#if defined(RT_CFG80211_SUPPORT) || defined(HOSTAPD_SUPPORT)
	NDIS_HOSTAPD_STATUS Hostapd;
	BOOLEAN IsCFG1xWdev;
#endif
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	BOOLEAN wds_enable;
#endif
#ifdef RADIUS_MAC_AUTH_SUPPORT
	BOOLEAN radius_mac_auth_enable;
#endif

	UINT8   csa_count;
	BCN_BUF_STRUCT bcn_buf;
#ifdef CONFIG_6G_SUPPORT
	ap_6g_cfg ap6g_cfg;
#endif
	struct _BSS_INFO_ARGUMENT_T bss_info_argument;
	struct _DEV_INFO_CTRL_T DevInfo;
	VOID *pHObj;

	BOOLEAN radio_off_req;
#ifdef MAC_REPEATER_SUPPORT
	RX_TRACKING_T rx_tracking;
#endif /* MAC_REPEATER_SUPPORT */

	BOOLEAN fAnyStationPeekTpBound;
	struct dev_rate_info rate;
	/*wlan profile, use for configuration part.*/
	void *wpf_cfg;
	/*wlan profile, use for operating configurion, update by wcfg & mlme*/
	void *wpf_op;
	/* struct conn_sta_info conn_sta; */
	/* struct protection_cfg prot_cfg; */
	/* Flag migrate from pAd partially */
	UINT32 OpStatusFlags;
	BOOLEAN bAllowBeaconing; /* Device opened and ready for beaconing */
	BOOLEAN bRejectAuth; /*Reject Auth when interface is closing*/
#ifdef DOT11K_RRM_SUPPORT
	RRM_CONFIG RrmCfg;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef MBO_SUPPORT
	MBO_CTRL MboCtrl;
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
	OCE_CTRL OceCtrl;
#endif /* OCE_SUPPORT */
	/* NEW FSM Segment
	 */
	SCAN_INFO ScanInfo;
	VOID *sync_fsm_ops;

#ifdef SCAN_RADAR_COEX_SUPPORT
	RTMP_OS_COMPLETION scan_complete;
	BOOLEAN RadarDetected;
#endif /* SCAN_RADAR_COEX_SUPPORT */

	BOOLEAN ch_wait_in_progress;
	RTMP_OS_COMPLETION ch_wait_for_scan;
	BOOLEAN ch_set_in_progress;

	STATE_MACHINE assoc_machine;
	STATE_MACHINE_FUNC assoc_func[ASSOC_FUNC_SIZE];
	VOID *assoc_api;

	STATE_MACHINE cntl_machine;
	STATE_MACHINE_FUNC cntl_func[CNTL_FUNC_SIZE];
	VOID *cntl_api;

	STATE_MACHINE auth_machine;
	STATE_MACHINE_FUNC auth_func[AUTH_FSM_FUNC_SIZE];
	VOID *auth_api;

#ifdef WSC_INCLUDED
	/*
			WPS segment
	*/
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;
	WSC_CTRL WscControl;
	WSC_SECURITY_MODE WscSecurityMode;
#ifdef IWSC_SUPPORT
	IWSC_INFO IWscInfo;
#endif /* IWSC_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef MWDS
	BOOLEAN	bDefaultMwdsStatus;   /* Determine the configuration status. */
	BOOLEAN	bSupportMWDS;	/* Determine If own MWDS capability */
#endif /* MWDS */
#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bInfReady;
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
#ifdef WH_EVENT_NOTIFIER
	struct Custom_VIE custom_vie;
#endif /* WH_EVENT_NOTIFIER */
	UCHAR quick_ch_change;
#ifdef CONFIG_MAP_SUPPORT
	MAP_CONFIG MAPCfg;
	BOOLEAN cac_not_required;
	UCHAR dev_role;
#ifdef DFS_CAC_R2
	struct cac_capability_lib cac_capability;
#endif
#endif
	UINT8 auto_channel_cen_ch_2;
	BOOLEAN is_marvell_ap;
	VOID *pDot11_H;
#ifdef MT7626_REDUCE_TX_OVERHEAD
	UCHAR WmmIdx;
#endif /* MT7626_REDUCE_TX_OVERHEAD */
	VIE_CTRL vie_ctrl[VIE_FRM_TYPE_MAX];
	struct prot_info prot;
	DL_LIST tx_block_list;
#ifdef DPP_SUPPORT
	UCHAR dpp_frame_da[MAC_ADDR_LEN];
	BOOLEAN is_gas_frame;
	DL_LIST dpp_frame_event_list;
#endif /* DPP_SUPPORT */
#ifdef WAPP_SUPPORT
	BOOLEAN avoid_apcli_linkdown;
#endif
#ifdef MGMT_TXPWR_CTRL
	BOOLEAN bPwrCtrlEn;
	CHAR	TxPwrDelta;
	CHAR	MgmtTxPwr;
	CHAR	MgmtTxPwrBak;
#endif
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	struct dev_eap_info eap;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	UCHAR map_radar_detect;
	UCHAR map_radar_channel;
	UCHAR map_radar_bw;
	UCHAR map_indicate_channel_change;
#ifdef WIFI_MD_COEX_SUPPORT
	UCHAR map_lte_unsafe_ch_detect;
#endif
#endif
#ifdef AMPDU_CONF_SUPPORT
	UINT8 bAMPDURetrynum;
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	UCHAR vht_sec_80_channel;
#endif
#ifdef DPP_R2_SUPPORT
	DPP_CONFIG DPPCfg;
#endif
	BOOLEAN enable_btwt_fw_cmd;
	RTMP_OS_SEM wdev_op_lock;
	BOOLEAN wdev_op_lock_flag;
	UCHAR dbg_wdev_op_lock_caller[80];
#ifdef QOS_R2
	BOOLEAN bQoSMCapability;
#endif
#ifdef CONFIG_VLAN_GTK_SUPPORT
	struct list_head vlan_gtk_list;
	UINT8 vlan_cnt;
#endif
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	UINT16 wdevStaCnt;
	UCHAR isStaNotSprtTpc;
	UINT8 pwrCnstrnt;
	UINT8 LastpwrCnst;
	CHAR tpcRssiThld;
	INT8 MinLinkMargin;
	UCHAR mLkMgnAddr[MAC_ADDR_LEN];
#endif
#endif
	UINT32 rx_drop_long_len;
	UINT32 btwt_id;
};

struct greenap_ctrl {
	/* capability of rreenap */
	BOOLEAN cap;
	/* greenap_allow=TRUE only when AP or AP+AP case */
	BOOLEAN allow;
	/* suspend greenap operation ex. when do AP backdround scan */
	UINT32 suspend;
	NDIS_SPIN_LOCK lock;
};

struct greenap_on_off_ctrl {
	UINT8 band_idx;
	BOOLEAN reduce_core_power;
};

typedef struct _PWR_MGMT_STRUCT_ {
	USHORT		Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE), Please use this value to replace  pAd->StaCfg[0].Psm in the future*/
	BOOLEAN	bBeaconLost;
	BOOLEAN	bTriggerRoaming;
	BOOLEAN	bEnterPsmNull;
	/* Usign this wcid instead of pEntry->wcid for race condition. ex STA in PS --> BCN lost -->
	Linkdown --> Exit h/w LP with pEntry->wcid (pEntry might be NULL at this moment)*/
	UINT16		wcid;
	BOOLEAN		 bDoze;
} PWR_MGMT_STRUCT, *PPWR_MGMT_STRUCT;

/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_CT_TIM_BCMC_OFFSET		0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0])

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0])

#define WLAN_MR_TIM_BCMC_GET(apidx) \
	(pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & NUM_BIT8[0])

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, _aid) \
	{	UCHAR tim_offset = _aid >> 3; \
		UCHAR bit_offset = _aid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset]; }


#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
#define MAX_TIME_RECORD 5
#endif

#ifdef TXRX_STAT_SUPPORT

typedef struct _TXRX_STAT_BSS {
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER TxDataPayloadByte;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER RxDataPayloadByte;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER TxUnicastDataPacket;
	LARGE_INTEGER TxMulticastDataPacket;
	LARGE_INTEGER TxBroadcastDataPacket;
	LARGE_INTEGER RxUnicastDataPacket;
	LARGE_INTEGER TxPacketDroppedCount;
	LARGE_INTEGER RxPacketDroppedCount;
	LARGE_INTEGER TxRetriedPacketCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER TxMgmtOffChPktCount;
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER LastSecTxBytes;
	LARGE_INTEGER LastSecRxBytes;
	HTTRANSMIT_SETTING LastMulticastTxRate;
#ifdef HIGHPRI_RATE_SPECIFIC
	HTTRANSMIT_SETTING LastHighPriorityTxRate[HIGHPRI_MAX_TYPE];
#endif
	UINT32 LastPktStaWcid;
	UINT32 Last1SecPER;
	UINT32 Last1TxFailCnt;
	UINT32 Last1TxCnt;
} TXRX_STAT_BSS, *PTXRX_STAT_BSS;

#endif
#ifdef MWDS
typedef struct _MWDS_STRUCT {
	UINT32 Addr4PktNum;
	UINT32 Addr3PktNum;
	UINT32 NullPktNum;
	UINT32 bcPktNum;
} MWDS_STRUCT;
#endif

typedef struct _BSS_STRUCT {
	struct wifi_dev wdev;

	INT mbss_idx;		/* global mbss index */
	INT mbss_grp_idx;	/* per-band(group) mbss index */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	/* In this state actually AP will be ON by default, but from user point of view */
	/* it may either never beacon or will start beaconing only after ApCLi connection successful  with RootAP */
	UINT_8 APStartPseduState;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(QOS_R1)
	UCHAR		QosMapSupport;
	UCHAR		DscpExceptionCount;
	USHORT		DscpRange[8];
	USHORT		DscpException[21];
#ifdef QOS_R2
	UCHAR		QoSMapIsUP;
	UCHAR		bDSCPPolicyEnable;
#endif
#endif
#ifdef HOSTAPD_HS_R3_SUPPORT
	UINT8				osu_enable;
#endif
#ifdef OCE_SUPPORT
	BOOLEAN ReducedNRListExist;
	REDUCED_NR_LIST_INFO   ReducedNRListInfo;
#endif /* OCE_SUPPORT */

	CHAR Ssid[MAX_LEN_OF_SSID+1];
	UCHAR SsidLen;
	BOOLEAN bHideSsid;

	UINT32 ShortSSID;

	UINT8 DtimPeriod;

#ifdef MBSS_DTIM_SUPPORT
	UCHAR DtimCount;
#endif

	USHORT CapabilityInfo;


	UINT16 MaxStaNum;	/* Limit the STA connection number per BSS */
	UINT16 StaCount;
	UINT16 StationKeepAliveTime;	/* unit: second */

	/*
		Security segment
	*/
#ifdef DISABLE_HOSTAPD_BEACON
	UINT8 RSNIE_ID[2];
#endif
	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

#ifdef HOSTAPD_OWE_SUPPORT
	UCHAR TRANSIE_Len;
	UCHAR TRANS_IE[MAX_LEN_OF_TRANS_IE];
#endif

	/* WPA */
	UCHAR GMK[32];
	UCHAR PSK[65];
	UCHAR PMK[LEN_MAX_PMK];
	UCHAR GTK[32];
	UCHAR GNonce[32];
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;

	/* for Group Rekey, AP ONLY */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	RALINK_TIMER_STRUCT REKEYTimer;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;

	/* For PMK Cache using, AP ONLY */
	ULONG PMKCachePeriod;	/* unit : jiffies */


	/*
		Transmitting segment
	*/
	UCHAR TxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate; /* RATE_1, RATE_2, RATE_5_5, RATE_11 */

	/*
		Statistics segment
	*/
	/*MBSS_STATISTICS MbssStat;*/
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;
#ifdef MAP_R2
	ULONG ucBytesTx;
	ULONG ucBytesRx;
	ULONG mcBytesTx;
	ULONG mcBytesRx;
	ULONG bcBytesTx;
	ULONG bcBytesRx;
#endif
	UCHAR BANClass3Data;
	ULONG IsolateInterStaTraffic;

	RT_802_11_ACL AccessControlList;


	/* EDCA QoS */
	/*BOOLEAN bWmmCapable;*/	/* 0:disable WMM, 1:enable WMM */
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */

	/*
	   Why need the parameter: 2009/09/22

	   1. iwpriv ra0 set WmmCapable=0
	   2. iwpriv ra0 set WirelessMode=9
	   3. iwpriv ra0 set WirelessMode=0
	   4. iwpriv ra0 set SSID=SampleAP

	   After the 4 commands, WMM is still enabled.
	   So we need the parameter to recover WMM Capable flag.

	   No the problem in station mode.
	 */
	BOOLEAN bWmmCapableOrg;	/* origin Wmm Capable in non-11n mode */
	/*
`		WPS segment
	*/
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;
#ifdef WSC_AP_SUPPORT
	WSC_SECURITY_MODE WscSecurityMode;
#endif /* WSC_AP_SUPPORT */

#ifdef IDS_SUPPORT
	UINT32 RcvdConflictSsidCount;
	UINT32 RcvdSpoofedAssocRespCount;
	UINT32 RcvdSpoofedReassocRespCount;
	UINT32 RcvdSpoofedProbeRespCount;
	UINT32 RcvdSpoofedBeaconCount;
	UINT32 RcvdSpoofedDisassocCount;
	UINT32 RcvdSpoofedAuthCount;
	UINT32 RcvdSpoofedDeauthCount;
	UINT32 RcvdSpoofedUnknownMgmtCount;
	UINT32 RcvdReplayAttackCount;

	CHAR RssiOfRcvdConflictSsid;
	CHAR RssiOfRcvdSpoofedAssocResp;
	CHAR RssiOfRcvdSpoofedReassocResp;
	CHAR RssiOfRcvdSpoofedProbeResp;
	CHAR RssiOfRcvdSpoofedBeacon;
	CHAR RssiOfRcvdSpoofedDisassoc;
	CHAR RssiOfRcvdSpoofedAuth;
	CHAR RssiOfRcvdSpoofedDeauth;
	CHAR RssiOfRcvdSpoofedUnknownMgmt;
	CHAR RssiOfRcvdReplayAttack;
#endif /* IDS_SUPPORT */


	/* YF@20120417: Avoid connecting to AP in Poor Env, value 0 fOr disable. */
	CHAR AssocReqRssiThreshold;
	CHAR RssiLowForStaKickOut;

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/*For AP vendor ie*/
	struct customer_vendor_ie ap_vendor_ie;
	DL_LIST ap_probe_rsp_vendor_ie_list;
	NDIS_SPIN_LOCK probe_rsp_vendor_ie_lock;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef CONFIG_DOT11U_INTERWORKING
	GAS_CTRL GASCtrl;
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
	HOTSPOT_CTRL HotSpotCtrl;
#endif

#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
	WNM_CTRL WNMCtrl;
#endif

#ifdef SPECIFIC_TX_POWER_SUPPORT
	CHAR TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	/* Extra IEs for (Re)Association Response provided by wpa_supplicant. E.g, WPS & P2P & WFD...etc */
	UCHAR AssocRespExtraIe[512];
	UINT32 AssocRespExtraIeLen;
#endif /* RT_CFG80211_SUPPORT */

#ifdef MT_MAC
	ULONG WriteBcnDoneTime[MAX_TIME_RECORD];
	ULONG BcnDmaDoneTime[MAX_TIME_RECORD];
	UCHAR bcn_not_idle_time;
	UINT32 bcn_recovery_num;
	ULONG TXS_TSF[MAX_TIME_RECORD];
	ULONG TXS_SN[MAX_TIME_RECORD];
	UCHAR timer_loop;
#endif /* MT_MAC */

#ifdef ROUTING_TAB_SUPPORT
	BOOLEAN bRoutingTabInit;
	UINT32 RoutingTabFlag;
	NDIS_SPIN_LOCK RoutingTabLock;
	ROUTING_ENTRY *pRoutingEntryPool;
	LIST_HEADER RoutingEntryFreeList;
	LIST_HEADER RoutingTab[ROUTING_HASH_TAB_SIZE];
#endif /* ROUTING_TAB_SUPPORT */

#ifdef A4_CONN
	UCHAR a4_init;
	NDIS_SPIN_LOCK a4_entry_lock;
	DL_LIST a4_entry_list;
#endif /* A4_CONN */
#ifdef WAPP_SUPPORT
	UCHAR ESPI_AC_BE[3];
	UCHAR ESPI_AC_BK[3];
	UCHAR ESPI_AC_VO[3];
	UCHAR ESPI_AC_VI[3];
#endif
#ifdef CONFIG_MAP_SUPPORT
	BOOLEAN is_bss_stop_by_map;
#endif
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	UINT8 IgmpQueryHoldTick; /*Duration to hold IGMP query in unit of 10 sec*/
	BOOLEAN IGMPPeriodicQuerySent; /*Whether Pertiodic IGMP query already sent on a MBSS*/
	UINT8 MldQueryHoldTick; /*Duration to hold MLD query in unit of 10 sec*/
	BOOLEAN MLDPeriodicQuerySent; /* Whether Pertiodic MLD query already sent on a MBSS*/
	BOOLEAN IgmpQueryHoldTickChanged; /* Whether IgmpQueryHoldTick already modified*/
	BOOLEAN MldQueryHoldTickChanged; /*Whether MldQueryHoldTick already modified*/
	UCHAR ipv6LinkLocalSrcAddr[16]; /* Ipv6 link local address for this MBSS as per it's BSSID*/
	UINT16 MldQryChkSum; /* Chksum to use in MLD query msg on this MBSS*/
#endif

	struct conn_sta_info conn_sta;
#ifdef DSCP_PRI_SUPPORT
	UCHAR dscp_pri_map_enable;
	INT8 dscp_pri_map[64];  /*priority mapping for dscp values */
#endif /*DSCP_PRI_SUPPORT*/
#ifdef TXRX_STAT_SUPPORT
	TXRX_STAT_BSS stat_bss;
#endif
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_BL_SUPPORT)
	/* BS Blcklist Support */
	LIST_HEADER BlackList;
	NDIS_SPIN_LOCK BlackListLock;
#endif
#ifdef TR181_SUPPORT
#ifdef STAT_ENHANCE_SUPPORT
	ULONG TxCountPerAC[WMM_NUM_OF_AC];
	ULONG TransmittedByteCountPerAC[WMM_NUM_OF_AC];
	ULONG TxErrorCountPerAC[WMM_NUM_OF_AC];
	ULONG TxDropCountPerAC[WMM_NUM_OF_AC];
	ULONG RxCountPerAC[WMM_NUM_OF_AC];
	ULONG ReceivedByteCountPerAC[WMM_NUM_OF_AC];
	ULONG RxErrorCountPerAC[WMM_NUM_OF_AC];
	ULONG RxDropCountPerAC[WMM_NUM_OF_AC];
#endif
#endif
	BOOLEAN max_idle_ie_en;		/* BSS Max Idle IE existence */
	UINT16 max_idle_period;		/* BSS Max Idle Period (unit: 1000 TUs) */
	UINT8 max_idle_option;		/* BSS Max Idle option field */
} BSS_STRUCT;

typedef struct _CHANNEL_SWITCH {
	UCHAR	CHSWMode;
} CHANNEL_SWITCH, *PCHANNEL_SWITCH;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WAPP_SUPPORT
#ifdef MAP_R2

typedef struct _radio_info {
	u8 ra_id[ETH_ALEN];
	u8 cu_noise;
	u8 cu_tx;
	u8 cu_rx;
	u8 cu_other;
} radio_info;

#endif

struct BSS_LOAD_INFO {
	UINT8 current_status[DBDC_BAND_NUM];
	UINT8 current_load[DBDC_BAND_NUM];
	UINT8 high_thrd[DBDC_BAND_NUM];
	UINT8 low_thrd[DBDC_BAND_NUM];
};
#endif /* WAPP_SUPPORT */
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
#define TPC_MODE_0 0
#define TPC_MODE_1 1
#define TPC_MODE_2 2
#define LINK_MARGIN_6DB 6
#define LINK_MARGIN_9DB 9
#define LINK_MARGIN_AUTO_MODE -127
typedef struct TPC_CTRL_STR {
	UINT8 u1CtrlMode;
	UINT16 sTpcWcid[MAX_LEN_OF_MAC_TABLE];
	UINT16 tpcStaCnt;
	UINT16 sTpcIntval;
	INT8 linkmargin;
	INT8 pwr_ofdm6m;
	INT8 pwr_mcs11;
} TPC_CTRL;
#endif
#endif


/* configuration common to OPMODE_AP as well as OPMODE_STA */
typedef struct _COMMON_CONFIG {
	BOOLEAN bCountryFlag;
	UCHAR CountryCode[4];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR Geography;
	UCHAR DfsType;
	PUCHAR pChDesp;
#endif /* EXT_BUILD_CHANNEL_LIST */
	PUCHAR pChDesc2G;
	PUCHAR pChDesc5G;
	UCHAR CountryRegion;	/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR CountryRegionForABand;	/* Enum of country region for A band */
	UCHAR cfg_wmode;
	UCHAR SavedPhyMode;
	USHORT Dsifs;		/* in units of usec */
	ULONG PacketFilter;	/* Packet filter for receiving */
	/* UINT8 RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS]; *//* unify to using get_regulatory_class from driver table */

	USHORT BeaconPeriod[DBDC_BAND_NUM];
	/* Channel Group*/
	UCHAR ChGrpEn;
	UCHAR ChGrpChannelList[MAX_NUM_OF_CHANNELS];
	UCHAR ChGrpChannelNum;
	UCHAR CSASupportFor2G;
	CHANNEL_SWITCH ChannelSwitchFor2G;

	UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG BasicRateBitmap;	/* backup basic ratebitmap */
	ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */
#ifdef GN_MIXMODE_SUPPORT
	BOOLEAN GNMixMode;
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef DBDC_ONE_BAND_SUPPORT
	UINT DbdcBandSupport;		/*0:both, 1:2.4G, 2:5G*/
#endif /*DBDC_ONE_BAND_SUPPORT*/

	BOOLEAN bInServicePeriod;

	UCHAR EtherTrafficBand;
	UCHAR WfFwdDisabled;


	BOOLEAN bAPSDAC_BE;
	BOOLEAN bAPSDAC_BK;
	BOOLEAN bAPSDAC_VI;
	BOOLEAN bAPSDAC_VO;

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
	DRV_SCS_GLO rScsGloInfo;
#endif
#endif

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	BOOLEAN	 TDLS_bAPSDAC_BE;
	BOOLEAN	 TDLS_bAPSDAC_BK;
	BOOLEAN	 TDLS_bAPSDAC_VI;
	BOOLEAN	 TDLS_bAPSDAC_VO;
	UCHAR TDLS_MaxSPLength;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
	   requested in association stage from the station;
	   we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN bACMAPSDBackup[4];	/* for delivery-enabled & trigger-enabled both */
	BOOLEAN bACMAPSDTr[4];	/* no use */
	UCHAR MaxSPLength;

	BOOLEAN bNeedSendTriggerFrame;
	BOOLEAN bAPSDForcePowerSave;	/* Force power save mode, should only use in APSD-STAUT */
	ULONG TriggerTimerCount;
	REG_TRANSMIT_SETTING RegTransmitSetting;	/*registry transmit setting. this is for reading registry setting only. not useful. */

	UCHAR TxRate;		/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR TxRateIndex;	/* Tx rate index in Rate Switch Table */
	UCHAR MinTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR RtsRate;		/* RATE_xxx */
	UCHAR MlmeRate;		/* RATE_xxx, used to send MLME frames */
	UCHAR BasicMlmeRate;	/* Default Rate for sending MLME frames */

	UCHAR TxPower;		/* in unit of mW */
	UINT8 ucTxPowerPercentage[DBDC_BAND_NUM];	/* 0~100 % */

#ifdef TX_POWER_CONTROL_SUPPORT
	POWER_BOOST_PARA_V0 PowerBoostParamV0[DBDC_BAND_NUM];
	POWER_BOOST_PARA_V1 PowerBoostParamV1[DBDC_BAND_NUM];
#endif /* TX_POWER_CONTROL_SUPPORT */

	UCHAR SKUenable[DBDC_BAND_NUM];
	UCHAR SKUTableIdx;
	CHAR  cTxPowerCompBackup[DBDC_BAND_NUM][SKU_TABLE_SIZE][SKU_TX_SPATIAL_STREAM_NUM];
	UCHAR PERCENTAGEenable[DBDC_BAND_NUM];
	UCHAR BFBACKOFFenable[DBDC_BAND_NUM];
	UINT8 CCKTxStream[DBDC_BAND_NUM];

#ifdef LINK_TEST_SUPPORT
	UCHAR LinkTestSupport;
	UCHAR LinkTestSupportTemp[DBDC_BAND_NUM];
#endif /* LINK_TEST_SUPPORT */

	UINT8 u1EDCCACtrl[DBDC_BAND_NUM];
	UINT8 u1EDCCAMode[DBDC_BAND_NUM];
	UINT8 u1EDCCAThreshold[DBDC_BAND_NUM][3];
	UINT8 u1EDCCACfgMode[DBDC_BAND_NUM];
	INT8  u1EDCCACBPCpst[DBDC_BAND_NUM][4];
	INT8  u1EDCCACBPBWDelta[DBDC_BAND_NUM][4]; /* BW20/BW40/BW80/BW160 */
	struct EDCCA_6G_CHANNEL_NODE ChlTabl160[8][15]; /* 8 for group */
	UINT8 isCust[DBDC_BAND_NUM];
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	UCHAR CalCacheApply;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	UINT8 ThermalRecalMode;
	UINT8 ThermalDynamicG0Action;
	UINT8 ThermalHRateDpdMode;
	UINT8 ThermalNtLtAction;
	UINT8 ucTxPowerDefault[DBDC_BAND_NUM];	/* keep for TxPowerPercentage */
	UINT8 PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC BACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC REGBACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
#endif /* DOT11_N_SUPPORT */

	UINT8 wifi_cert;
	BOOLEAN dbdc_mode;
	ENUM_DBDC_MODE eDBDC_mode;

#ifdef DOT11_VHT_AC
	BOOLEAN force_vht;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN g_band_256_qam;
	BOOLEAN bUseVhtRateFor2g;
#ifdef VHT_MU_MIMO
	BOOLEAN vht_mu_mimo;
#endif /* VHT_MU_MIMO */
#endif /* DOT11_VHT_AC */

	IOT_STRUC IOTestParm;	/* 802.11n InterOpbility Test Parameter; */
	ULONG TxPreamble;	/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
	BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
	ULONG UseBGProtection;	/* 0: auto, 1: always use, 2: always not use */
	BOOLEAN bUseShortSlotTime[DBDC_BAND_NUM];	/* 0: disable, 1 - use short slot (9us) */
	UINT8 SlotTime[DBDC_BAND_NUM];	/* Slot time */
	BOOLEAN bEnableTxBurst;	/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
	BOOLEAN bAggregationCapable;	/* 1: enable TX aggregation when the peer supports it */
	BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
	BOOLEAN bIEEE80211H;	/* 1: enable IEEE802.11h spec. */
#ifdef DELAY_TCP_ACK_V2
	BOOLEAN bEnableTxopPeakTpEn;
	UCHAR PeakTpBeAifsn;
	UINT32 PeakTpAvgRxPktLen;
	UINT32 PeakTpRxHigherBoundTput;
	UINT32 PeakTpRxLowerBoundTput;
#endif /* DELAY_TCP_ACK_V2 */
	UCHAR RDDurRegion; /* Region of radar detection */
	ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */
#ifdef VLAN_SUPPORT
	BOOLEAN bEnableVlan;	/* 1: enble vlan function 0: disable vlan function */
#endif /*VLAN_SUPPORT*/

#ifdef TPC_SUPPORT
	BOOLEAN b80211TPC;
#ifdef TPC_MODE_CTRL
	TPC_CTRL ctrlTPC;
#endif
#endif /* TPC_SUPPORT */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bRdg;
#endif /* DOT11_N_SUPPORT */
	QOS_CAPABILITY_PARM APQosCapability;	/* QOS capability of the current associated AP */
	EDCA_PARM APEdcaParm[WMM_NUM];	/* EDCA parameters of the current associated AP */
	QBSS_LOAD_PARM APQbssLoad;	/* QBSS load of the current associated AP */
	UCHAR AckPolicy[WMM_NUM_OF_AC];	/* ACK policy of the specified AC. see ACK_xxx */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */
#endif /* CONFIG_STA_SUPPORT */
	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition */
	ULONG OpStatusFlags;

	BOOLEAN NdisRadioStateOff;	/*For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff. */
	ABGBAND_STATE BandState;		/* For setting BBP used on B/G or A mode. */


#ifdef MT_DFS_SUPPORT
	DFS_PARAM DfsParameter;
#endif

#ifdef CARRIER_DETECTION_SUPPORT
	CARRIER_DETECTION_STRUCT CarrierDetect;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* HT */
	HT_CAPABILITY_IE HtCapability;
	/*This IE is used with channel switch announcement element when changing to a new 40MHz. */
	/*This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp. */
	NEW_EXT_CHAN_IE NewExtChanOffset;	/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */

	EXT_CAP_INFO_ELEMENT ExtCapIE;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */

#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable;
	/*
	   Following two paramters now only used for the initial scan operation. the AP only do
	   bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
	   By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	 */
	UCHAR BssCoexApCntThr;
	UCHAR BssCoexApCnt;

	UCHAR Bss2040CoexistFlag;	/* bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo. */
	RALINK_TIMER_STRUCT Bss2040CoexistTimer;
	UCHAR Bss2040NeedFallBack;	/* 1: Need Fall back to 20MHz */

	/* store 2040 coex last scan result per-band, for MBSS reusing. */
	BSS_COEX_SCAN_LAST_RESULT BssCoexScanLastResult[DBDC_BAND_NUM];

	/*This IE is used for 20/40 BSS Coexistence. */
	BSS_2040_COEXIST_IE BSS2040CoexistInfo;

	USHORT Dot11OBssScanPassiveDwell;	/* Unit : TU. 5~1000 */
	USHORT Dot11OBssScanActiveDwell;	/* Unit : TU. 10~1000 */
	USHORT Dot11BssWidthTriggerScanInt;	/* Unit : Second */
	USHORT Dot11OBssScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
	USHORT Dot11OBssScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
	USHORT Dot11BssWidthChanTranDelayFactor;
	USHORT Dot11OBssScanActivityThre;	/* Unit : percentage */

	ULONG Dot11BssWidthChanTranDelay;	/* multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	ULONG CountDownCtr;	/* CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */

	BSS_2040_COEXIST_IE LastBSSCoexist2040;
	BSS_2040_COEXIST_IE BSSCoexist2040;
	TRIGGER_EVENT_TAB TriggerEventTab;
	UCHAR ChannelListIdx;

	BOOLEAN bOverlapScanning;
	BOOLEAN bBssCoexNotify;
#endif /* DOT11N_DRAFT3 */

	BOOLEAN bSeOff;
	UINT8   ucAntennaIndex;

	BOOLEAN bMIMOPSEnable;
	BOOLEAN bDisableReordering;
	BOOLEAN bForty_Mhz_Intolerant;
	BOOLEAN bExtChannelSwitchAnnouncement;
	BOOLEAN bRcvBSSWidthTriggerEvents;
	ULONG LastRcvBSSWidthTriggerEventsTime;

	UCHAR TxBASize;

	BOOLEAN bRalinkBurstMode;
	UINT32 RestoreBurstMode;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	UINT32 cfg_vht;
	VHT_CAP_INFO vht_info;
	VHT_CAP_IE vht_cap_ie;
	BOOLEAN bNonVhtDisallow; /* Disallow non-VHT connection */
#endif /* DOT11_VHT_AC */

#ifdef SYSTEM_LOG_SUPPORT
	/* Enable wireless event */
	BOOLEAN bWirelessEvent;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN bWiFiTest;	/* Enable this parameter for WiFi test */

	/* Tx & Rx Stream number selection */
	UCHAR TxStream;
	UCHAR RxStream;

	/* transmit phy mode, trasmit rate for Multicast. */
	/*
	#ifdef MCAST_RATE_SPECIFIC
		UCHAR McastTransmitMcs;
		UCHAR McastTransmitPhyMode;
	#endif // MCAST_RATE_SPECIFIC
	*/
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */


#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	UINT8 WscHdrPshBtnCheckCount;
#endif /* WSC_INCLUDED */


	NDIS_SPIN_LOCK MeasureReqTabLock;
	PMEASURE_REQ_TAB pMeasureReqTab;
#ifdef TPC_SUPPORT
	NDIS_SPIN_LOCK TpcReqTabLock;
	PTPC_REQ_TAB pTpcReqTab;
#endif
	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	MCAST_PKT_TYPE	McastType;
	BOOLEAN McastTypeFlag;
	HTTRANSMIT_SETTING MCastPhyMode;
	HTTRANSMIT_SETTING MCastPhyMode_5G;
#else
	HTTRANSMIT_SETTING mcastphymode;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */

#ifdef SINGLE_SKU
	UINT16 DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	UINT16 AntGain;
	UINT16 BandedgeDelta;
	UINT16 ModuleTxpower;
#endif /* SINGLE_SKU */

#ifdef RTMP_RBUS_SUPPORT
	ULONG CID;
	ULONG CN;
#endif /* RTMP_RBUS_SUPPORT */

	BOOLEAN HT_DisallowTKIP;	/* Restrict the encryption type in 11n HT mode */
#ifdef DOT11K_RRM_SUPPORT
	BOOLEAN VoPwrConstraintTest;
#endif /* DOT11K_RRM_SUPPORT */

	BOOLEAN HT_Disable;	/* 1: disable HT function; 0: enable HT function */

#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
	USHORT	lowTrafficThrd;		/* Threshold for reverting to default MCS when traffic is low */
	SHORT	TrainUpRuleRSSI;	/* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
	USHORT	TrainUpLowThrd;		/* QuickDRS Hybrid train up low threshold */
	USHORT	TrainUpHighThrd;	/* QuickDRS Hybrid train up high threshold */
	BOOLEAN	TrainUpRule;		/* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */

#ifdef STREAM_MODE_SUPPORT
#define		STREAM_MODE_STA_NUM		4

	UCHAR	StreamMode; /* 0=disabled, 1=enable for 1SS, 2=enable for 2SS, 3=enable for 1,2SS */
	UCHAR	StreamModeMac[STREAM_MODE_STA_NUM][MAC_ADDR_LEN];
	UINT16	StreamModeMCS;	/* Bit map for enabling Stream Mode based on MCS */
#endif /* STREAM_MODE_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ITxBfTimeout;
	ULONG ETxBfTimeout;
	ULONG	ETxBfEnCond;		/* Enable sending of sounding and beamforming */
	ULONG	MUTxRxEnable;		/* Enable MUTxRxEnable */
	BOOLEAN	ETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	ETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */
#ifdef TXBF_DYNAMIC_DISABLE
	UINT_8  ucAutoSoundingCtrl; /* Initial AutoSoundingCtrl for rStaRecBf */
#endif /* TXBF_DYNAMIC_DISABLE */
	BOOLEAN BfSmthIntlBypass[DBDC_BAND_NUM];
	BOOLEAN HeraStbcPriority[DBDC_BAND_NUM]; /* STBC priority against eBF and iBF*/
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef CFG_SUPPORT_FALCON_MURU
	ULONG TamArbOpMode;
	ULONG HE_PpduFmt;
	ULONG HE_OfdmaSchType;
	ULONG HE_OfdmaUserNum;
	ULONG HE_TrigPadding;
	DRV_MURU_GLO rGloInfo;
	BOOLEAN bShowMuEdcaParam;
#endif

#ifdef WSC_INCLUDED
	BOOLEAN WscPBCOverlap;
	WSC_STA_PBC_PROBE_INFO WscStaPbcProbeInfo;
#endif /* WSC_INCLUDED */

#ifdef CONFIG_ZTE_RADIO_ONOFF
	BOOLEAN bRadioEnable;
#endif /* CONFIG_ZTE_RADIO_ONOFF */

#ifdef MICROWAVE_OVEN_SUPPORT
	MO_CFG_STRUCT MO_Cfg;	/* data structure for mitigating microwave interference */
#endif /* MICROWAVE_OVEN_SUPPORT */


	/* TODO: need to integrate with MICROWAVE_OVEN_SUPPORT */
#ifdef DYNAMIC_VGA_SUPPORT
	LNA_VGA_CTL_STRUCT lna_vga_ctl;
#endif /* DYNAMIC_VGA_SUPPORT */

	BOOLEAN bStopReadTemperature; /* avoid race condition between FW/driver */
	BOOLEAN bTXRX_RXV_ON;
	BOOLEAN ManualTxop;
	ULONG ManualTxopThreshold;
	UCHAR ManualTxopUpBound;
	UCHAR ManualTxopLowBound;
#ifdef REDUCE_TCP_ACK_SUPPORT
	UINT32 ReduceAckEnable;
	UINT32 ReduceAckProbability;
	UINT32 ReduceAckTimeout;
	UINT32 ReduceAckCnxTimeout;
#endif
#ifdef WHNAT_SUPPORT
	BOOLEAN whnat_en;
	UCHAR wed_idx;
	UINT32 wed_version;
#endif /*WHNAT_SUPPORT*/
	UCHAR need_fallback;
#if defined(MT7986)
	BOOLEAN DBDCDualPCIE;
#endif /* MT7986 */
#ifdef MULTI_INTR_SUPPORT
	BOOLEAN MultiIntr;
#endif

#ifdef CFG_SUPPORT_FALCON_SR
	UCHAR SREnable[DBDC_BAND_NUM];
	UCHAR SRMode[DBDC_BAND_NUM];
	UCHAR SRSDEnable[DBDC_BAND_NUM];
	UCHAR SRDPDEnable[DBDC_BAND_NUM];
	UCHAR SRDropMinMcs[DBDC_BAND_NUM];
	UCHAR SRDPDThreshold[DBDC_BAND_NUM];
	UCHAR DisSrBfrConnected[DBDC_BAND_NUM];
#ifdef CONFIG_MAP_SUPPORT
	UCHAR SRMeshUlMode[DBDC_BAND_NUM];
#endif /* CONFIG_MAP_SUPPORT */
#endif /* CFG_SUPPORT_FALCON_SR */
#ifdef CFG_SUPPORT_FALCON_PP
	UCHAR pp_enable[DBDC_BAND_NUM];
#endif /* CFG_SUPPORT_FALCON_PP */
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	BOOLEAN bV10W56SwitchVHT80;
	BOOLEAN bV10W56SwitchVHT160;
	BOOLEAN bCh144Enabled;
	UCHAR v10_bw;
	BOOLEAN bApDownQueued;
	BOOLEAN bBwSyncQueued;
	UCHAR apdown_count;
	UCHAR bwsync_count;
#endif
#ifdef ANDLINK_FEATURE_SUPPORT
	UINT8 andlink_enable[DBDC_BAND_NUM];
	UINT8 andlink_ip_hostname_en[DBDC_BAND_NUM];
	UINT32 andlink_simple_val;
	struct mtk_andlink_rate_conf andlink_uplink_rate_cfg[DBDC_BAND_NUM];
	struct mtk_andlink_rate_conf andlink_sta_rate_cfg[DBDC_BAND_NUM];
#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	UCHAR ack_cts_enable[DBDC_BAND_NUM];
	UINT32 distance[DBDC_BAND_NUM];/*unit: m*/
	UINT32 cck_timeout[DBDC_BAND_NUM];/*unit: us*/
	UINT32 ofdm_timeout[DBDC_BAND_NUM];/*unit: us*/
	UINT32 ofdma_timeout[DBDC_BAND_NUM];/*unit: us*/
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
#ifdef CFG_SUPPORT_FALCON_MURU
	UCHAR Dis160RuMu[DBDC_BAND_NUM];
	UCHAR MaxRuOfdma[DBDC_BAND_NUM];
	UCHAR MaxDLMuMimo[DBDC_BAND_NUM];
	UCHAR MaxULMuMimo[DBDC_BAND_NUM];
#endif /* CFG_SUPPORT_FALCON_MURU */
	BOOLEAN vht_1024_qam;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR usr_channel;
	UCHAR old_channel;
#endif
#ifdef CONFIG_CPE_SUPPORT
	INT8 powerBackoff[DBDC_BAND_NUM]; /* Power backoff in an amplifier */
#endif
#ifdef CONFIG_3_WIRE_SUPPORT
	BOOLEAN ThreeWireFunctionEnable;
#endif
#ifdef WIFI_CSI_CN_INFO_SUPPORT
	UINT8 EnableCNInfo[DBDC_BAND_NUM];
#endif /* WIFI_CSI_CN_INFO_SUPPORT */
} COMMON_CONFIG, *PCOMMON_CONFIG;

#ifdef CFG_TDLS_SUPPORT
typedef struct _CFG_TDLS_STRUCT {
	/* For TPK handshake */
	UCHAR			ANonce[32];	/* Generated in Message 1, random variable */
	UCHAR			SNonce[32];	/* Generated in Message 2, random variable */
	ULONG			KeyLifetime;	/*  Use type= 'Key Lifetime Interval' unit: Seconds, min lifetime = 300 seconds */
	UCHAR			TPK[LEN_PMK];	/* TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data */
	UCHAR			TPKName[LEN_PMK_NAME];
	BOOLEAN			IneedKey;
	BOOLEAN			bCfgTDLSCapable; /* 0:disable TDLS, 1:enable TDLS  ; using supplicant sm */
	BOOLEAN			TdlsChSwitchSupp;
	BOOLEAN		TdlsPsmSupp;

	UINT8			TdlsLinkCount;
	UINT8			TdlsDialogToken;
	CFG_TDLS_ENTRY		TDLSEntry[MAX_NUM_OF_CFG_TDLS_ENTRY];
	/* Channel Switch */
	UCHAR			CHSWPeerMacAddr[MAC_ADDR_LEN];
	BOOLEAN				bDoingPeriodChannelSwitch;
	BOOLEAN					IamInOffChannel;
	USHORT					ChSwitchTime;
	USHORT					ChSwitchTimeout;
	UINT					BaseChannelStayTime;
	UINT					OffChannelStayTime;
	RALINK_TIMER_STRUCT	BaseChannelSwitchTimer;		/* Use to channel switch */
	USHORT					BaseChannel;
	USHORT					BaseChannelBW;
	USHORT					OrigTargetOffChannel;
	USHORT					TargetOffChannel;
	USHORT					TargetOffChannelBW;
	USHORT					TargetOffChannelExt;
	BOOLEAN					bChannelSwitchInitiator;
	BOOLEAN					IsentCHSW;
} CFG_TDLS_STRUCT, *PCFG_TDLS_STRUCT;
#endif /* CFG_TDLS_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#if defined(WPA_SUPPLICANT_SUPPORT) || defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
typedef struct _WPA_SUPPLICANT_INFO {
	/*
		802.1x WEP + MD5 will set key to driver before assoc, but we need to apply the key to
		ASIC after get EAPOL-Success frame, so we use this flag to indicate that
	*/
	BOOLEAN IEEE8021x_required_keys;
	CIPHER_KEY DesireSharedKey[4];	/* Record user desired WEP keys */
	UCHAR DesireSharedKeyId;

	/* 0x00: driver ignores wpa_supplicant */
	/* 0x01: wpa_supplicant initiates scanning and AP selection */
	/* 0x02: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters */
	/* 0x80: wpa_supplicant trigger driver to do WPS */
	UCHAR WpaSupplicantUP;
	UCHAR WpaSupplicantScanCount;
	BOOLEAN bRSN_IE_FromWpaSupplicant;
	BOOLEAN bLostAp;
	UCHAR *pWpsProbeReqIe;
	UINT WpsProbeReqIeLen;
	UCHAR *pWpaAssocIe;
	UINT WpaAssocIeLen;
#ifdef CFG_TDLS_SUPPORT
	CFG_TDLS_STRUCT CFG_Tdls_info;
#endif
} WPA_SUPPLICANT_INFO;
#endif /* WPA_SUPPLICANT_SUPPORT || APCLI_CFG80211_SUPPORT */
#endif

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)

#ifdef CREDENTIAL_STORE
typedef struct _STA_CONNECT_INFO {
	BOOLEAN Changeable;
	BOOLEAN IEEE8021X;
	CHAR Ssid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR SsidLen; /* the actual ssid length in used */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode; /* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR DefaultKeyId;
	UCHAR PMK[LEN_MAX_PMK]; /* WPA PSK mode PMK */
	UCHAR WpaPassPhrase[64]; /* WPA PSK pass phrase */
	UINT WpaPassPhraseLen; /* the length of WPA PSK pass phrase */
	UINT8 WpaState;
	CIPHER_KEY SharedKey[1][4]; /* STA always use SharedKey[BSS0][0..3] */
	NDIS_SPIN_LOCK Lock;
} STA_CONNECT_INFO, *P_STA_CONNECT_INFO;
#endif /* CREDENTIAL_STORE */

#ifdef DOT11Z_TDLS_SUPPORT
typedef struct _TDLS_STRUCT {
	BOOLEAN bTDLSCapable;	/* 0:disable TDLS, 1:enable TDLS */
	BOOLEAN bAcceptWeakSecurity;
	BOOLEAN	TdlsChSwitchSupp;
	BOOLEAN	TdlsPsmSupp;
	UINT8 TdlsDialogToken;
	UINT32 TdlsKeyLifeTime;
	UINT8 TdlsLinkSize;		/* record how much links establish already. */
	RT_802_11_TDLS TDLSEntry[MAX_NUM_OF_TDLS_ENTRY];
	NDIS_SPIN_LOCK TDLSEntryLock;
	NDIS_SPIN_LOCK TDLSUapsdLock;
#ifdef TDLS_AUTOLINK_SUPPORT
	BOOLEAN	TdlsAutoLink;
	LIST_HEADER TdlsDiscovPeerList;
	NDIS_SPIN_LOCK TdlsDiscovPeerListSemLock;
	LIST_HEADER TdlsBlackList;
	NDIS_SPIN_LOCK TdlsBlackListSemLock;

	CHAR TdlsAutoSetupRssiThreshold;
	CHAR TdlsAutoTeardownRssiThreshold;
	USHORT TdlsRssiMeasurementPeriod;
	USHORT TdlsDisabledPeriodByTeardown;
	USHORT TdlsAutoDiscoveryPeriod;
#endif /* TDLS_AUTOLINK_SUPPORT */

	/* Channel Switch */
	NDIS_SPIN_LOCK TdlsChSwLock;
	NDIS_SPIN_LOCK TdlsInitChannelLock;
	RALINK_TIMER_STRUCT TdlsPeriodGoBackBaseChTimer;
	RALINK_TIMER_STRUCT TdlsPeriodGoOffChTimer;
	RALINK_TIMER_STRUCT TdlsDisableChannelSwitchTimer;

	BOOLEAN TdlsForcePowerSaveWithAP;
	BOOLEAN bDoingPeriodChannelSwitch;
	BOOLEAN bChannelSwitchInitiator;
	BOOLEAN bChannelSwitchWaitSuccess;
	UCHAR TdlsCurrentTargetChannel;
	UCHAR TdlsDesireChannel;
	UCHAR TdlsDesireChannelBW;
	UCHAR TdlsDesireChSwMacAddr[MAC_ADDR_LEN];
	UCHAR TdlsAsicOperateChannel;
	UCHAR TdlsAsicOperateChannelBW;

	UCHAR TdlsChannelSwitchPairCount;
	UCHAR TdlsChannelSwitchRetryCount;
	ULONG TdlsGoBackStartTime;
	ULONG TdlsChSwSilenceTime;
	ULONG TdlsActiveSwitchTime;
	ULONG TdlsActiveSwitchTimeOut;
	UCHAR TdlsDtimCount;

	/* record old power save mode */
#define TDLS_POWER_SAVE_ACTIVE_COUNT_DOWN_NUM		(5*1000/MLME_TASK_EXEC_INTV)
	BOOLEAN TdlsFlgIsKeepingActiveCountDown; /* keep active until 0 */
	UINT8 TdlsPowerSaveActiveCountDown;
} TDLS_STRUCT, *PTDLS_STRUCT;
#endif /* DOT11Z_TDLS_SUPPORT // */

typedef struct _apcli_inf_stat {
	BOOLEAN ApCliInit;	/* Set it as 1 if ApCli is initialized */
	BOOLEAN Enable;		/* Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable" */
	BOOLEAN Valid;		/* Set it as 1 if the apcli interface associated success to remote AP. */
	/* recv beacon part..... */
	ULONG ApCliRcvBeaconTime_MlmeEnqueueForRecv;
	ULONG ApCliRcvBeaconTime_MlmeEnqueueForRecv_2;
	ULONG ApCliRcvBeaconTime;
	BOOLEAN bPeerExist; /* TRUE if we hear Root AP's beacon */

	ULONG ApCliLinkUpTime;
	ULONG LinkDownReason;
	ULONG Disconnect_Sub_Reason;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT	ProbeReqCnt;
	BOOLEAN AutoConnectFlag;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	RTMP_OS_COMPLETION ifdown_complete;
	RTMP_OS_COMPLETION linkdown_complete;
} APCLI_INF_STAT, *PAPCLI_INF_STAT;

typedef struct _stat_counters {
	ULONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;

	ULONG OneSecTxBytes;
	ULONG OneSecRxBytes;
} STAT_COUNTERS, *PSTAT_COUNTERS;

/*
	This data structure keep the current active BSS/IBSS's configuration that
	this STA had agreed upon joining the network. Which means these parameters
	are usually decided by the BSS/IBSS creator instead of user configuration.
	Data in this data structurre is valid only when either ADHOC_ON()/INFRA_ON()
	is TRUE. Normally, after SCAN or failed roaming attempts, we need to
	recover back to the current active settings
*/
typedef struct _STA_ACTIVE_CONFIG {
	USHORT Aid;
	USHORT AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT CapabilityInfo;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	USHORT CfpMaxDuration;
	USHORT CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	struct legacy_rate rate;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_PHY_INFO SupportedPhyInfo;
	RT_HT_CAPABILITY SupportedHtPhy;
#ifdef DOT11_VHT_AC
	RT_VHT_CAP	SupVhtCap;
#endif /* DOT11_VHT_AC */
	UCHAR RxMcsSet[16];
} STA_ACTIVE_CONFIG;

struct adhoc_info {
	USHORT AtimWin;		/* used when starting a new IBSS */
	BCN_BUF_STRUCT bcn_buf;
#ifdef DOT11_N_SUPPORT
	BOOLEAN bAdhocN;
#endif /* DOT11_N_SUPPORT */
	BOOLEAN bAdhocCreator;	/*TRUE indicates divice is Creator. */

#ifdef XLINK_SUPPORT
	BOOLEAN PSPXlink;	/* 0: Disable. 1: Enable */
#endif /* XLINK_SUPPORT */
#ifdef IWSC_SUPPORT
	IWSC_INFO IWscInfo;
#endif /* IWSC_SUPPORT */
};

#define NET_DEV_NAME_MAX_LENGTH	16

/* Modified by Wu Xi-Kun 4/21/2006 */
/* STA configuration and status */
struct _STA_ADMIN_CONFIG {
	struct wifi_dev wdev;
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	UINT32 beacon_loss_count;/*total beacon loss count*/
	ULONG rx_beacon;/*total rx beacon count*/
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/

	/*
		GROUP 1 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, but not necessary fully equal to the final
		settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either
		AP or IBSS holder).
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR BssType;		/* BSS_INFRA or BSS_ADHOC */

	/*APPS DVT MSP*/
	BOOLEAN PwrSaveSet;

	struct adhoc_info adhocInfo;
	STA_ACTIVE_CONFIG StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */

	CHAR dev_name[NET_DEV_NAME_MAX_LENGTH];
	USHORT OriDevType;
	CHAR Ssid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR SsidLen;		/* the actual ssid length in used */
	UCHAR LastSsidLen;	/* the actual ssid length in used */
	UCHAR Bssid[MAC_ADDR_LEN];
	CHAR LastSsid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];

	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	USHORT BeaconPeriod;

	ULONG BeaconLostTime;	/* seconds */
	ULONG LastBeaconRxTime;	/* OS's timestamp of the last BEACON RX time */
	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
	ULONG StaStatusFlags;
	BOOLEAN bConfigChanged; /* Config Change flag for the same SSID setting */
	BOOLEAN bShowHiddenSSID;	/* Show all known SSID in SSID list get operation */
	BOOLEAN bForceTxBurst;	/* 1: force enble TX PACKET BURST, 0: disable */
	BOOLEAN bTGnWifiTest;

	PVOID pAssociatedAPEntry;
	RALINK_TIMER_STRUCT LinkDownTimer;

	/*
		GROUP 2 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, and should be always applied to the final
		settings in ACTIVE BSS without compromising with the BSS holder.
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	USHORT CountDowntoPsm;
	USHORT DefaultListenCount;	/* default listen count; */
	USHORT ThisTbttNumToNextWakeUp;
	ULONG WindowsPowerMode;	/* Power mode for AC power */
	ULONG WindowsBatteryPowerMode;	/* Power mode for battery if exists */
	BOOLEAN bWindowsACCAMEnable;	/* Enable CAM power mode when AC on */
	BOOLEAN	 FlgPsmCanNotSleep; /* TRUE: can not switch ASIC to sleep */
	/* MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1) */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE) */
	PWR_MGMT_STRUCT	PwrMgmt;

	/* remove YF fsm */
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	UCHAR RssiTrigger;
	UCHAR RssiTriggerMode;	/* RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD */

	/*
		Security segment
	*/
	/* Add to support different cipher suite for WPA2/WPA mode */
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;

	/*Add to support Mix Mode (WPA/WPA2) AP */
	UINT32 AKMMap;

	USHORT RsnCapability;

	UCHAR WpaPassPhrase[64];	/* WPA PSK pass phrase */
	UINT WpaPassPhraseLen;	/* the length of WPA PSK pass phrase */
	UCHAR PMK[LEN_MAX_PMK];	/* WPA PSK mode PMK */
	UCHAR PTK[LEN_PTK];	/* WPA PSK mode PTK */
	UCHAR GMK[LEN_GMK];	/* WPA PSK mode GMK */
	UCHAR GTK[MAX_LEN_GTK];	/* GTK from authenticator */
	UCHAR GNonce[32];	/* GNonce for WPA2PSK from authenticator */
	CIPHER_KEY TxGTK;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum;	/* Saved PMKID number */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	NDIS_SPIN_LOCK SavedPMK_lock;
#endif


	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt;	/* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */
	/* For WPA-PSK supplicant state */
	UINT8 WpaState;		/* Default is SS_NOTUSE and handled by microsoft 802.1x */
	UCHAR ReplayCounter[8];
	UCHAR ANonce[32];	/* ANonce for WPA-PSK from aurhenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	ULONG LastScanTime;	 /* remove YF fsm */
	BOOLEAN bNotFirstScan;	/* remove YF fsm */
	/* New for WPA, windows want us to to keep association information and */
	/* Fixed IEs from last association response */
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN];	/* The content saved here should be little-endian format. */
	USHORT ResVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];

	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	/*for probe request vendor ie*/
	struct customer_vendor_ie apcli_vendor_ie;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT S*/
/* --- */

	BOOLEAN bSwRadio;	/* Software controlled Radio On/Off, TRUE: On */
	BOOLEAN bHwRadio;	/* Hardware controlled Radio On/Off, TRUE: On */
	BOOLEAN bRadio;		/* Radio state, And of Sw & Hw radio state */
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */

	RALINK_TIMER_STRUCT StaQuickResponeForRateUpTimer;
	BOOLEAN StaQuickResponeForRateUpTimerRunning;

	BOOLEAN bAutoReconnect;	/* Set to TRUE when setting OID_802_11_SSID with no matching BSSID */
	BOOLEAN bAutoConnectByBssid;
	BOOLEAN bAutoConnectIfNoSSID;
	BOOLEAN bFastConnect;
	BOOLEAN bSkipAutoScanConn;

	/* Fast Roaming */
	BOOLEAN bAutoRoaming;	/* 0:disable auto roaming by RSSI, 1:enable auto roaming by RSSI */
	CHAR dBmToRoam;		/* the condition to roam when receiving Rssi less than this value. It's negative value. */

	/*
		Statistics segment
	*/
	STAT_COUNTERS StaStatistic;
#ifdef REPEATER_TX_RX_STATISTIC
	UINT64 TxDropCount;
#endif

	USHORT RPIDensity[8];	/* Array for RPI density collection */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	USHORT DisassocReason;
	UCHAR DisassocSta[MAC_ADDR_LEN];
	USHORT DeauthReason;
	UCHAR DeauthSta[MAC_ADDR_LEN];
	USHORT AuthFailReason;
	UCHAR AuthFailSta[MAC_ADDR_LEN];

	/*connectinfo  for tmp store connect info from UI*/
	BOOLEAN Connectinfoflag;
	UCHAR	ConnectinfoBssid[MAC_ADDR_LEN];
	UCHAR	ConnectinfoChannel;
	UCHAR	ConnectinfoSsidLen;
	CHAR	ConnectinfoSsid[MAX_LEN_OF_SSID];
	UCHAR ConnectinfoBssType;

	/*
		OS_COMPLETION segment
	*/
	RTMP_OS_COMPLETION ifdown_fsm_reset_complete;
	RTMP_OS_COMPLETION linkdown_complete;
#ifdef APCLI_CFG80211_SUPPORT
	RTMP_OS_COMPLETION scan_complete;
	BOOLEAN MarkToClose;
	BOOLEAN ReadyToConnect;
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
#define MONITOR_FLAG_11N_SNIFFER		0x01
	UCHAR BssMonitorFlag;	/* Specific flag for monitor */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#if defined(WPA_SUPPLICANT_SUPPORT) || defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	WPA_SUPPLICANT_INFO wpa_supplicant_info;
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif


#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR IEEE80211dClientMode;
	UCHAR StaOriCountryCode[3];
	UCHAR StaOriGeography;
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef IP_ASSEMBLY
	BOOLEAN bFragFlag;
#endif /* IP_ASSEMBLY */

#ifdef DOT11R_FT_SUPPORT
	DOT11R_CMN_STRUC Dot11RCommInfo;
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11N_DRAFT3
	UCHAR RegClass;		/*IE_SUPP_REG_CLASS: 2009 PF#3: For 20/40 Intolerant Channel Report */
#endif /* DOT11N_DRAFT3 */



#ifdef WIDI_SUPPORT
	BOOLEAN bWIDI;
	BOOLEAN bSendingProbe;
#endif /* WIDI_SUPPORT */

#ifdef CONFIG_DOT11U_INTERWORKING
	GAS_CTRL GASCtrl;
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
	HOTSPOT_CTRL HotSpotCtrl;
#endif

#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
	WNM_CTRL WNMCtrl;
#endif

#ifdef WFD_SUPPORT
	RT_WFD_CONFIG WfdCfg;
#endif /* WFD_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_STRUCT TdlsInfo;
#endif /* DOT11Z_TDLS_SUPPORT */

/* apcli stuff */

	UINT16 MacTabWCID;	/*WCID value, which point to the entry of ASIC Mac table. */

	CHAR CfgSsid[MAX_LEN_OF_SSID];
	UCHAR CfgSsidLen;
	UCHAR CfgApCliBssid[MAC_ADDR_LEN];

	PSPOLL_FRAME PsPollFrame;
	HEADER_802_11 NullFrame;

	APCLI_INF_STAT ApcliInfStat;
#if (defined (ANDLINK_FEATURE_SUPPORT) && defined(ANDLINK_V4_0))
	ULONGLONG andlink_tx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_rx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_rx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_rx_bytes[ANDLINK_IF_MAX];
#endif/*ANDLINK_SUPPORT*/
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		UCHAR ApCliMode;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
	UCHAR	TrialCh; /* the channel that Apcli interface will try to connect the rootap locates */
	RALINK_TIMER_STRUCT TrialConnectTimer;
	RALINK_TIMER_STRUCT TrialConnectPhase2Timer;
	RALINK_TIMER_STRUCT TrialConnectRetryTimer;

	USHORT NewRootApRetryCnt;
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef FAST_EAPOL_WAR
	BOOLEAN	pre_entry_alloc;
#endif /* FAST_EAPOL_WAR */
#ifdef CON_WPS
	UINT ConWpsApCliModeScanDoneStatus;
#endif /* CON_WPS */

	UINT8 dync_txop_histogram[5];
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	APCLI_CONNECT_SCAN_TYPE ApCliAutoConnectType; /* 0 : User Trigger SCAN Mode, 1 :  Driver Trigger SCAN Mode, this is for Sigma DUT test , Peer AP may change BSSID, but SSID is the same */
	BOOLEAN		ApCliAutoConnectRunning;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef A4_CONN
	UCHAR a4_init;
	UCHAR a4_apcli;
#endif

#ifdef CONFIG_MAP_SUPPORT
	UCHAR last_controller_connectivity;
	UCHAR PeerMAPEnable;
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	UCHAR sae_cfg_group;
#endif
#ifdef CONFIG_OWE_SUPPORT
	UCHAR curr_owe_group;
	UCHAR owe_trans_ssid_len;
	CHAR owe_trans_ssid[MAX_LEN_OF_SSID];
	CHAR owe_trans_bssid[MAC_ADDR_LEN];
	UCHAR owe_trans_open_ssid_len;
	CHAR owe_trans_open_ssid[MAX_LEN_OF_SSID];
	CHAR owe_trans_open_bssid[MAC_ADDR_LEN];
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
#ifdef APCLI_SUPPORT
	enum ENUM_TWT_REQUESTER_STATE_T aeTWTReqState;
	struct twt_flow_t arTWTFlow[TWT_MAX_FLOW_NUM];
	struct twt_planner_t rTWTPlanner;
#endif /* APCLI_SUPPORT */
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	CHAR BcnRssiLast[4];
	CHAR BcnRssiAvg[4];
#ifdef APCLI_SUPPORT
	UCHAR ApCliTransDisableSupported;
	struct transition_disable_bitmap ApCli_tti_bitmap;
#endif
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	UCHAR eth_list_init;
	NDIS_SPIN_LOCK eth_entry_lock;
	DL_LIST eth_entry_list;
#endif
};

#ifdef ETH_CONVERT_SUPPORT
#define ETH_CONVERT_NODE_MAX 256

/* Ethernet Convertor operation mode definitions. */
typedef enum {
	ETH_CONVERT_MODE_DISABLE = 0,
	ETH_CONVERT_MODE_DONGLE = 1,	/* Multiple client support, dispatch to AP by device mac address. */
	ETH_CONVERT_MODE_CLONE = 2,	/* Single client support, dispatch to AP by client's mac address. */
	ETH_CONVERT_MODE_HYBRID = 3,	/* Multiple client supprot, dispatch to AP by client's mac address. */
} ETH_CONVERT_MODE;

typedef struct _ETH_CONVERT_STRUCT_ {
	UCHAR EthCloneMac[MAC_ADDR_LEN];	/*Only meanful when ECMode = Clone/Hybrid mode. */
	UCHAR ECMode;		/* 0 = Disable, 1 = Dongle mode, 2 = Clone mode, 3 = Hybrid mode. */
	BOOLEAN CloneMacVaild;	/* 1 if the CloneMac is valid for connection. 0 if not valid. */
	/*	UINT32		nodeCount;					// the number of nodes which connect to Internet via us. */
	UCHAR SSIDStr[MAX_LEN_OF_SSID];
	UCHAR SSIDStrLen;
	BOOLEAN macAutoLearn;	/*0: disabled, 1: enabled. */
} ETH_CONVERT_STRUCT;
#endif /* ETH_CONVERT_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
/* AUTH-RSP State Machine Aux data structure */
typedef struct _AP_MLME_AUX {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Alg;
	CHAR Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;
#endif /* CONFIG_AP_SUPPORT */

#ifdef ADHOC_WPA2PSK_SUPPORT
typedef struct _FOUR_WAY_HANDSHAKE_PROFILE {
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR ReplayCounter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UINT8 WpaState;
	UCHAR MsgType;
	RALINK_TIMER_STRUCT MsgRetryTimer;
	UCHAR MsgRetryCounter;
} FOUR_WAY_HANDSHAKE_PROFILE, *PFOUR_WAY_HANDSHAKE_PROFILE;
#endif /* ADHOC_WPA2PSK_SUPPORT */

/* sub catalog of MAC_TABLE ENTRY */
struct ampdu_caps {
	/* ht */
	UINT8 max_ht_ampdu_len_exp;
	UINT8 min_mpdu_start_spacing;
	/* vht */
	UINT8 max_mpdu_len;
	UINT8 max_vht_ampdu_len_exp;
	/* he */
	UINT8 max_he_ampdu_len_exp;
	UINT8 multi_tid_agg;
	/* he_6g */
	UINT8 he6g_min_mpdu_start_spacing;
	UINT8 he6g_max_mpdu_len;
	UINT8 he6g_max_ampdu_len_exp;
};

struct stbc_caps {
	/* ht */
	UINT8 ht_rx_stbc;
	/* vht */
	UINT8 vht_rx_stbc;
	/* he */
	enum he_stbc_caps he_stbc;
};

struct support_ch_bw {
	/* ht */
	UINT8 ht_support_ch_width_set;
	/* vht */
	UINT8 vht_support_ch_width_set;
	UINT8 ext_nss_bw;
	/* he */
	UINT8 he_ch_width;
	UINT8 he_bw20_242tone;
	UINT8 he6g_ch_width;
	UINT8 he6g_prim_ch;
	UINT8 he6g_ccfs_0;
	UINT8 he6g_ccfs_1;
	/* ch info */
	UINT8 prim_ch;
	UINT8 ccfs_0;
	UINT8 ccfs_1;
	UINT8 ccfs_2;
};

struct he_bss_color {
	UINT8 bss_color;
	UINT8 partial_bss_color;
	UINT8 bss_color_dis;
};

enum support_mode {
	HT_MIX_SUPPORT = 1,
	HT_GF_SUPPORT = (1 << 1),
	VHT_SUPPORT = (1 << 2),
	HE_24G_SUPPORT = (1 << 3),
	HE_5G_SUPPORT = (1 << 4),
	HE_6G_SUPPORT = (1 << 5),
};

struct caps_info {
	struct ampdu_caps ampdu;
	struct stbc_caps stbc;
	struct support_ch_bw ch_bw;
	struct rate_caps rate;
	enum support_mode modes;
	/* ht */
	enum ht_caps ht_cap;
	UINT8 max_amsdu_len;
	UINT8 pco_tx_time;
	UINT8 mfb;
	/* vht */
	enum vht_caps vht_cap;
	UINT8 bfee_sts_cap;
	UINT8 sound_dim;
	UINT8 vht_link_adapt;
	/* he */
	enum he_mac_caps he_mac_cap;
	enum he_phy_caps he_phy_cap;
	enum he_gi_caps he_gi;
	struct he_bss_color bss_color_info;
	struct he_bf_info he_bf;
	UINT8 default_pe_dur;
	UINT16 txop_dur_rts_thld;
	UINT8 he_frag_level;
	UINT8 max_frag_msdu_num;
	UINT8 min_frag_size;
	UINT8 tf_mac_pad_duration;
	UINT8 he_link_adapt;
	UINT8 punc_preamble_rx;
	UINT8 midamble_rx_max_nsts;
	UINT8 dcm_max_constellation_tx;
	UINT8 dcm_max_constellation_rx;
	UINT8 dcm_max_ru;
	/* he 6g */
	UINT8 he6g_op_present;
	UINT8 he6g_dup_bcn;
	UINT8 min_rate;
	enum sm_power_save he6g_smps;
	UINT8 rd_resp;
	UINT8 rx_ant_pattern_consist;
	UINT8 tx_and_pattern_consist;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	struct itwt_ie twt_ie;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
};

#ifdef OCE_SUPPORT
#ifdef OCE_FILS_SUPPORT
#define MAX_FILS_PTK_LEN 88 /* KCK + KEK + TK */

struct fils_info {
	UINT16 last_pending_id;
	UCHAR *pending_ie;
	UINT pending_ie_len;
	UCHAR *extra_ie;
	UINT extra_ie_len;
	struct raw_rssi_info rssi_info;

	UCHAR auth_algo;

	UCHAR PTK[MAX_FILS_PTK_LEN];
	UCHAR PTK_len;

	INT16 status;
	BOOLEAN is_pending_auth;
	BOOLEAN is_pending_assoc;
	BOOLEAN is_pending_decrypt;
	BOOLEAN is_pending_encrypt;
	BOOLEAN is_post_assoc;

	VOID (*pending_action)(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
	VOID (*pending_decrypt)(PRTMP_ADAPTER pAd, struct _MAC_TABLE_ENTRY *pEntry,
		struct _SECURITY_CONFIG *pSecConfig, MLME_QUEUE_ELEM *Elem);
};
#endif /* OCE_FILS_SUPPORT */

typedef union GNU_PACKED _ESP_INFO {
struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
	UINT32 None:8;                  /* bit 24 ~ 31 */
	UINT32 DataPPDUDurationTarget:8;                        /* bit 16 ~ 23 */
	UINT32 EstimatedAirTimeFraction:8;              /* bit 8 ~ 15 */
	UINT32 BAWinSize:3;                                             /* bit 5 ~ 7 */
	UINT32 DataFormat:2;                                            /* bit 3 ~ 4 */
	UINT32 Reserved:1;                                              /* bit 2 */
	UINT32 ACI:2;                                                   /* bit 0 ~ 1 */
	/* -------------------------------------        */
	/* ACI  AC              Access Category                 */
	/* -------------------------------------        */
	/* 00   AC_BE   Best Effort                                     */
	/* 01   AC_BK   Background                                      */
	/* 10   AC_VI   Video                                           */
	/* 11   AC_VO   Voice                                           */
	/* -------------------------------------        */
#else
	UINT32 ACI:2;                                                   /* bit 0 ~ 1 */
	/* -------------------------------------        */
	/* ACI  AC              Access Category                 */
	/* -------------------------------------        */
	/* 00   AC_BE   Best Effort                                     */
	/* 01   AC_BK   Background                                      */
	/* 10   AC_VI   Video                                           */
	/* 11   AC_VO   Voice                                           */
	/* -------------------------------------        */
	UINT32 Reserved:1;                                              /* bit 2 */
	UINT32 DataFormat:2;                                            /* bit 3 ~ 4 */
	UINT32 BAWinSize:3;                                             /* bit 5 ~ 7 */
	UINT32 EstimatedAirTimeFraction:8;              /* bit 8 ~ 15 */
	UINT32 DataPPDUDurationTarget:8;                        /* bit 16 ~ 23 */
	UINT32 None:8;                  /* bit 24 ~ 31 */
#endif /* RT_BIG_ENDIAN */
	} field;
	UINT32 word;
} ESP_INFO, *PESP_INFO;

#endif /* OCE_SUPPORT */

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
typedef struct _AUTH_FRAME_INFO {
	UCHAR addr1[MAC_ADDR_LEN];
	UCHAR addr2[MAC_ADDR_LEN];
	USHORT auth_alg;
	USHORT auth_seq;
	USHORT auth_status;
	CHAR Chtxt[CIPHER_TEXT_LEN];
#ifdef DOT11R_FT_SUPPORT
	FT_INFO FtInfo;
#endif /* DOT11R_FT_SUPPORT */
} AUTH_FRAME_INFO;
#endif

typedef struct _RADIUS_ACCOUNT_ENTRY {
	BOOLEAN occupied;
	UCHAR Addr[MAC_ADDR_LEN];
	struct wifi_dev *wdev;
	HTTRANSMIT_SETTING HTPhyMode;
	RSSI_SAMPLE RssiSample;
	ULONG TxBytes;
	ULONG RxBytes;
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG NoDataIdleCount;
} RADIUS_ACCOUNT_ENTRY, *PRADIUS_ACCOUNT_ENTRY;

typedef struct _MAC_TABLE_ENTRY {
	UINT32 EntryType;
	UINT32 EntryState;
	struct wifi_dev *wdev;
	PVOID pAd;
	struct _MAC_TABLE_ENTRY *pNext;

	struct caps_info cap;
	ULONG ClientStatusFlags;
	ULONG cli_cap_flags;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode;
	HTTRANSMIT_SETTING MinHTPhyMode;
	struct phy_params phy_param;

#ifdef DATA_TXPWR_CTRL
	BOOLEAN DataTxPwrEn;		//Indicates that data power ctrl is in effect
	INT8 PowerOffset[DATA_TXPOWER_MAX_BW_NUM][DATA_TXPOWER_MAX_MCS_NUM];
#endif

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	INT16 CurRssi; /* Current Average RSSI */
#endif
	/*
		wcid:

		tr_tb_idx:

		func_tb_idx used to indicate following index:
			in StaCfg
			in pAd->MeshTab
			in WdsTab.MacTab

		apidx: should remove this
	*/
#ifdef OCE_SUPPORT
#ifdef OCE_FILS_SUPPORT
	struct fils_info filsInfo;
#endif /* OCE_FILS_SUPPORT */
	struct oce_info oceInfo;
#endif /* OCE_SUPPORT */

#ifdef WTBL_TDD_SUPPORT
	WTBL_TDD_CTRL wtblTddCtrl;
#endif /* WTBL_TDD_SUPPORT */
#ifdef SW_CONNECT_SUPPORT
	/* mark this entry is pure S/W or not */
	BOOLEAN bSw;
	/*
		original pEntry->wcid entry usages are most for S/W concept, only minor parts are for H/W concept.
		so add extra pEntry->hw_wcid for hw when fill TxD real H/W wcid
	*/
	UINT16 hw_wcid;
	HTTRANSMIT_SETTING DummyHTPhyMode;
#endif /* SW_CONNECT_SUPPORT */
	UINT16 wcid;
	UINT16 tr_tb_idx;
	UCHAR func_tb_idx;
	UCHAR apidx;		/* MBSS number */

	BOOLEAN isRalink;
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

#ifdef MBO_SUPPORT
	BOOLEAN bIndicateNPC;
	BOOLEAN bIndicateCDC;
	BOOLEAN bindicate_NPC_event;
	BOOLEAN bindicate_CDC_event;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoNPC;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoCDC;
	BOOLEAN is_mbo_bndstr_sta;
#endif /* MBO_SUPPORT */
#ifdef A4_CONN
	UCHAR	a4_entry;		/* Determine if this entry act which A4 role */
#endif /* A4_CONN */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
    BOOLEAN bEnable4Addr;
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
	UINT32 vlan_id;
#endif
	BOOLEAN bLastRTSFailed;

#ifdef RADIUS_MAC_AUTH_SUPPORT
	BOOLEAN bAllowTraffic;
#endif

#ifdef MWDS
	UCHAR	MWDSEntry;		/* Determine if this entry act which MWDS role */
	BOOLEAN bSupportMWDS;	/* Determine If own MWDS capability */
	BOOLEAN bEnableMWDS;	/* Determine If do 3-address to 4-address */
	MWDS_STRUCT MWDSInfo;
#endif /* MWDS */

	UCHAR Addr[MAC_ADDR_LEN];
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss;
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	BOOLEAN bRoamingRefreshDone;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		STATE MACHINE Status
	*/
	USHORT Aid;	/* in range 1~2007, with bit 14~15 = b'11, e.g., 0xc001~0xc7d7 */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */

#ifdef P2P_SUPPORT
	UINT32 P2PEntryType; /* 0:Invalid, 1:P2P_GO, 2:P2P_CLI */
	P2P_ENTRY_PARM P2pInfo;
	BOOLEAN bP2pClient;
	UCHAR DeviceName[P2P_DEVICE_NAME_LEN];
#endif /* P2P_SUPPORT */
#ifdef WFD_SUPPORT
	BOOLEAN bWfdClient;
#endif /* WFD_SUPPORT */

	/* Rx status related parameters */
	RSSI_SAMPLE RssiSample;
	UINT32 LastTxRate;
	UINT32 LastRxRate;
#ifdef CONFIG_MAP_SUPPORT
	UINT32 map_LastTxRate;
	UINT32 map_LastRxRate;
#endif
	SHORT freqOffset;		/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */

#ifdef AIR_MONITOR
	UCHAR mnt_idx[DBDC_BAND_NUM];
	UCHAR mnt_band;
#endif /* AIR_MONITOR */


	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */
#ifdef ADHOC_WPA2PSK_SUPPORT
	FOUR_WAY_HANDSHAKE_PROFILE WPA_Supplicant;
	FOUR_WAY_HANDSHAKE_PROFILE WPA_Authenticator;
	CIPHER_KEY RxGTK;
	BOOLEAN bPeerHigherMAC;
#ifdef IWSC_SUPPORT
	BOOLEAN bUpdateInfoFromPeerBeacon;
#endif /* IWSC_SUPPORT */
#endif /* ADHOC_WPA2PSK_SUPPORT */

#ifdef IWSC_SUPPORT
	BOOLEAN bIWscSmpbcAccept;
#endif /* IWSC_SUPPORT */

	struct _SECURITY_CONFIG SecConfig;
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR CMTimerRunning;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */

	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	ULONG NoDataIdleCount;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	ULONG LastRxTimeCount;
#endif
	ULONG sleep_from;
	ULONG AssocDeadLine;
	UINT16 StationKeepAliveCount;	/* unit: second */
	USHORT CapabilityInfo;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */

#ifdef WDS_SUPPORT
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */
#endif /* WDS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	struct _REPEATER_CLIENT_ENTRY *pReptCli;
	VOID *ProxySta;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	UINT32 StaConnectTime;	/* the live time of this station since associated with AP */
	UINT32 StaIdleTimeout;	/* idle timeout per entry */
	BOOLEAN sta_force_keep;

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */


	UCHAR MaxSPLength;

	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */

	QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */

	PQUEUE_ENTRY pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but yet statistics handle */

#endif /* UAPSD_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	UINT32 StreamModeMACReg;	/* MAC reg used to control stream mode for this client. 0=>No stream mode */
#endif /* STREAM_MODE_SUPPORT */

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;

	/* ==================================================== */
	enum RATE_ADAPT_ALG rateAlg;
	/* TODO: shiang-usw, use following parameters to replace "RateLen/MaxSupportedRate" */
	UCHAR RateLen;
	UCHAR MaxSupportedRate;

	BOOLEAN bAutoTxRateSwitch;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */
#ifdef ANDLINK_FEATURE_SUPPORT
	UINT	ipaddr; /* In network order */
	UCHAR ipv6addr[16];/*IPV6 support*/
	RTMP_STRING hostname[HOSTNAME_LEN];/*device host name*/
#endif

#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR lowTrafficCount;
	UCHAR fewPktsCnt;
	BOOLEAN perThrdAdj;
	UCHAR mcsGroup;/* the mcs group to be tried */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl;	/* AGS control */
#endif /* AGS_SUPPORT */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX + 1];
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction;	/* Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; /* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;	/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];
	UINT32 CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */

	BOOLEAN fgGband256QAMSupport;
	UCHAR SupportRateMode; /* 1: CCK 2:OFDM 4: HT, 8:VHT */
	UINT8 SupportCCKMCS;
	UINT8 SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	UINT32 SupportHTMCS;
#ifdef DOT11_VHT_AC
	UINT16 SupportVHTMCS1SS;
	UINT16 SupportVHTMCS2SS;
	UINT16 SupportVHTMCS3SS;
	UINT16 SupportVHTMCS4SS;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef MFB_SUPPORT
	UCHAR lastLegalMfb;	/* last legal mfb which is used to set rate */
	BOOLEAN isMfbChanged;	/* purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	struct _RTMP_RA_LEGACY_TB *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK fLastChangeAccordingMfbLock;
	/* Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
	/* Rx mfb */
	UCHAR pendingMfsi;
	/* Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR	mfbToTx;
	UCHAR	mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR		eTxBfEnCond;
	UCHAR		iTxBfEn;
	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
#ifdef MT_MAC
	VENDOR_BF_SETTING rStaBfRecVendorUpdate;
	TXBF_PFMU_STA_INFO rStaRecBf;
	BFEE_STA_REC rStaRecBfee;
	UCHAR	has_oui; /* Indication of STA's OUI when association */
#endif
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */

	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecTxFailCount;
	UINT32 OneSecRxLGICount;		/* unicast-to-me Long GI count */
	UINT32 OneSecRxSGICount;		/* unicast-to-me Short GI count */
	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;

	UINT32 tx_fail_cnt;
	UINT32 tx_total_cnt;
	ULONG last_calc_timestamp;
	UINT8 per_err_times;
	UINT16 token_use_cnt;
	ULONG tx_contd_fail_cnt;

	/*==================================================== */
	EXT_CAP_INFO_ELEMENT ext_cap;
	struct _vendor_ie_cap vendor_ie;
#ifdef DOT11_N_SUPPORT
	HT_CAPABILITY_IE HTCapability;
	UCHAR BAAutoTest;
	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT tx_amsdu_bitmap;
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UINT32 amsdu_limit_len;
	UINT32 amsdu_limit_len_adjust;
	UCHAR MmpsMode;		/* MIMO power save mode. */

	BOOLEAN agg_err_flag;
	UINT32 tx_per;
	UINT32 winsize_limit;

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;

	/* only take effect if ext_cap.operating_mode_notification = 1 */
	BOOLEAN force_op_mode;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
	UCHAR BssTransitionManmtSupport;
#endif /* CONFIG_DOT11V_WNM */

	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;

	UINT32 TXMCSExpected[MAX_MCS_SET];
	UINT32 TXMCSSuccessful[MAX_MCS_SET];
	UINT32 TXMCSFailed[MAX_MCS_SET];
	UINT32 TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
	FT_MDIE_INFO MdIeInfo;
	FT_FTIE_INFO FtIeInfo;

	UINT8 InitialMDIE[5];
	UINT8 InitialFTIE[256];
	UINT InitialFTIE_Len;

	UCHAR FT_PMK_R0[32];
	UCHAR FT_PMK_R0_NAME[16];
	UCHAR FT_PMK_R1[32];
	UCHAR FT_PMK_R1_NAME[16];
	UCHAR PTK_NAME[16];

	UCHAR FT_UCipher[4];
	UCHAR FT_Akm[4];
	UCHAR FT_PTK[LEN_MAX_PTK]; /* 512 bits max, KCK(16)+KEK(16)+TK(32) */
	UCHAR FT_Status;
	UCHAR FT_R1kh_CacheMiss_Times;

#ifdef R1KH_HARD_RETRY
	UCHAR FT_R1kh_CacheMiss_Hard;
	RTMP_OS_COMPLETION ack_r1kh;
#endif /* R1KH_HARD_RETRY */

#ifdef HOSTAPD_11R_SUPPORT
	AUTH_FRAME_INFO auth_info_resp;
#endif

#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	UCHAR assoc_req_frame[ASSOC_REQ_LEN];
	USHORT assoc_req_len;
	UCHAR DevPeerRole;
	UCHAR cur_rssi_status;
	UCHAR pre_rssi_status;
	BOOLEAN isTriggerSteering;
	UCHAR pre_traffic_mode;
#ifdef MAP_R2
	UCHAR profile;
#endif
	UINT32 TxBytesMAP;
	UINT32 RxBytesMAP;
	UINT16 DisconnectReason;
#endif
	UINT32 TxRxTime[4][2];
	UINT32 wrapArTxRxTime[4][2];
#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER TxUnicastPktCount;
	LARGE_INTEGER TxDataPacketCount1SecValue;
	LARGE_INTEGER TxDataPacketByte1SecValue;
	LARGE_INTEGER LastTxDataPacketCountValue;
	LARGE_INTEGER LastTxDataPacketByteValue;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER RxUnicastPktCount;
	LARGE_INTEGER RxUnicastByteCount;
	LARGE_INTEGER RxDataPacketCount1SecValue;
	LARGE_INTEGER RxDataPacketByte1SecValue;
	LARGE_INTEGER LastRxDataPacketCountValue;
	LARGE_INTEGER LastRxDataPacketByteValue;
	LARGE_INTEGER RxDataPacketCountPerAC[4];/*per access category*/
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	ULONG RxLastMgmtPktRate;
	CHAR LastDataPktRssi[4];
	CHAR LastMgmtPktRssi[4];
	UINT32 LastOneSecTxTotalCountByWtbl;
	UINT32 LastOneSecTxFailCountByWtbl;
	UINT32 LastOneSecPER;
	UINT32 TxSuccessByWtbl;/*data/unicast same variable, updated per sec*/
#ifdef WIFI_IAP_STA_DUMP_FEATURE
	UINT32 TxFailCountByWtbl;/*tx fail total count, updated per sec*/
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/
#endif

	UINT64 TxBytes;
	UINT64 RxBytes;
	UINT64 TxFailCount;
#endif /* CONFIG_AP_SUPPORT */
	ULONG OneSecTxBytes;
	ULONG OneSecRxBytes;
#ifdef ANTENNA_DIVERSITY_SUPPORT
	ULONG ant_div_rx_bytes;
	ULONG ant_div_tx_bytes;
#endif
	ULONG AvgTxBytes;
	ULONG AvgRxBytes;
	ULONG one_sec_tx_pkts;
	ULONG avg_tx_pkts;
	ULONG avg_tx_pkt_len;
	ULONG avg_rx_pkt_len;
	ULONG one_sec_tx_succ_pkts;
	ULONG one_sec_tx_mpdu_pkts;
	ULONG one_sec_tx_mpdu_succ_pkts;
	ULONG avg_rx_pkts;
#if (defined (ANDLINK_FEATURE_SUPPORT) && defined(ANDLINK_V4_0))
	ULONGLONG andlink_tx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_rx_rate_rt[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_avg_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_tx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_max_rx_rate[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_sample_rx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_tx_bytes[ANDLINK_IF_MAX];
	ULONGLONG andlink_period_rx_bytes[ANDLINK_IF_MAX];
#endif/*ANDLINK_FEATURE_SUPPORT & ANDLINK_V4_0*/

#ifdef RX_COUNT_DETECT
	ULONG one_sec_rx_pkts;
#endif /* RX_COUNT_DETECT */

#ifdef VOW_SUPPORT
	UINT mcliTcpCnt;
	UINT mcliTcpAckCnt;
#endif /* VOW_SUPPORT */

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
	IE_LISTS *ie_list;
#endif

#ifdef EAP_STATS_SUPPORT
	LARGE_INTEGER mpdu_attempts;
	LARGE_INTEGER mpdu_retries;
	LARGE_INTEGER mpdu_xretries;
	UINT32	tx_latency_min;
	UINT32	tx_latency_max;
	UINT32	tx_latency_avg;
#ifdef EAP_ENHANCE_STATS_SUPPORT
	UINT32	air_latency_min;
	UINT32	air_latency_max;
	UINT32	air_latency_avg;
	UINT8	tx_cnt_min;
	UINT8	tx_cnt_max;
	UINT8	tx_cnt_avg;
#endif /* EAP_ENHANCE_STATS_SUPPORT */
#endif

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	UINT16			TdlsTxFailCount;
	UINT32			TdlsKeyLifeTimeCount;
	UCHAR			MatchTdlsEntryIdx; /* indicate the index in pAd->StaCfg[0].DLSEntry */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */

#ifdef SMART_ANTENNA
	UINT32		saLstTxNoRtyCnt;
	UINT32		saLstTxRtyCnt;
	UINT32		saLstTxFailCnt;
	UCHAR		*pRateTable;
	UCHAR		orgTxRateCol;
	CHAR		baseAvgRSSI[3];
	INT32		curRSSI[3];
	INT32		cntRSSI[3];
	INT32		rssi_zero_cnt[3];
	INT32		sumSNR[3];
	INT32		cntSNR[3];
	INT32		sumPreSNR[3];
	INT32		cntPreSNR[3];
	CHAR		avgRssi[3];
	CHAR		prevAvgRssi[3];
	CHAR		avgSNR[3];
	CHAR		avgPreSNR[3];
	CHAR		curAvgRSSI[3];

	UINT32		hwTxSucCnt;
	UINT32		hwTxRtyCnt;
	ULONG		calcTime;

	UINT32		saTxCnt;
	UINT32		saRxCnt;
	ULONG		mcsUsage[33];
	ULONG		curMcsApplyTime;
	CHAR		mcsInUse;

	VOID		*pTrainEntry;
#endif /* SMART_ANTENNA */

	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
#if defined(CONFIG_HOTSPOT_R2) || defined(QOS_R1)
	UCHAR				QosMapSupport;
	UCHAR				DscpExceptionCount;
	USHORT				DscpRange[8];
	USHORT				DscpException[21];
	UCHAR				MSCSSupport;
#endif
#ifdef QOS_R2
	UCHAR DSCPPolicyEnable;
#endif
#ifdef CONFIG_HOTSPOT_R2
	UCHAR				IsWNMReqValid;
	struct wnm_req_data	*ReqData;
	struct _sta_hs_info hs_info;
	UCHAR OSEN_IE_Len;
	UCHAR OSEN_IE[MAX_LEN_OF_RSNIE];
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef CONFIG_HOTSPOT_R3
	STA_HS_CONSORTIUM_OI hs_consortium_oi;
#endif /* CONFIG_HOTSPOT_R3 */
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
	UCHAR				IsBTMReqValid;
	UCHAR				IsKeep;
	UINT16				BTMDisassocCount;
	BOOLEAN				bBSSMantSTASupport;
	struct btm_req_data	*ReqbtmData;
#endif

	BOOLEAN bACMBit[WMM_NUM_OF_AC];

	RA_ENTRY_INFO_T RaEntry;
	RA_INTERNAL_INFO_T	RaInternal;
	UINT32	ConnectionType;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	BOOLEAN bTxPktChk;
	UINT8 TxStatRspCnt;
	UINT32 TotalTxSuccessCnt;	/* Accumulated Tx success count from N9 (WTBL) */
#endif

#ifdef HTC_DECRYPT_IOT
	UINT32 HTC_ICVErrCnt; /* keep the ICV Error cnt of HTC Rx Cnt */
	UCHAR HTC_AAD_OM_Force; /* when reach the threshold, force set the WTBL.DW2.AAD_OM to 1 */
	UINT32 HTC_AAD_OM_CountDown; /* settling time (1 count 1 second) for start count HTC_ICVErrCnt */
	UCHAR HTC_AAD_OM_Freeze; /* Treat the entry's AAD_OM setting is correct now */
#endif /* HTC_DECRYPT_IOT */

#ifdef WH_EVENT_NOTIFIER
	UCHAR custom_ie_len;			   /* Length of Vendor Information Element */
	UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];  /* Vendor Information Element  */
	StaActivityItem tx_state;			  /* Station's tx state record */
	StaActivityItem rx_state;			  /* Station's rx state record */
#endif /* WH_EVENT_NOTIFIER */
#ifdef GN_MIXMODE_SUPPORT
	BOOLEAN FlgIs11bSta;
#endif /*GN_MIXMODE_SUPPORT*/
#ifdef PN_UC_REPLAY_DETECTION_SUPPORT
	UINT64 CCMP_UC_PN[NUM_OF_TID];
#endif /* PN_UC_REPLAY_DETECTION_SUPPORT */
	UINT64 CCMP_BC_PN[4];
	BOOLEAN Init_CCMP_BC_PN_Passed[4];
	BOOLEAN AllowUpdateRSC;
#ifdef REDUCE_TCP_ACK_SUPPORT
	BOOLEAN RACKEnalbedSta;
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	UINT8 twt_flow_id_bitmap;
	UINT32 twt_btwt_id_bitmap;
	UINT32 twt_interval_max;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	BOOLEAN WtblSetFlag; /* This will reflect whetherWtbl entry is created or not */
	RTMP_OS_COMPLETION WtblSetDone; /* To intimate status when Wtbl entry is created */

#ifdef DOT11_HE_AX
	UCHAR ucMUEdcaUpdateCnt;

	/*
	 *  Store MU EDCA params for each ACs in BSS info
	 *  Use the same format as the update cmd for memory copy
	 */
	struct he_mu_edca_params arMUEdcaParams[ACI_AC_NUM];

	/* Spatial Reuse Parameter Set for the BSS */
	UCHAR SRControl;
	UCHAR NonSRGObssPdMaxOffset;
	UCHAR SRGObssPdMinOffset;
	UCHAR SRGObssPdMaxOffset;
	UINT64 SRGBSSColorBitmap;
	UINT64 SRGPartialBSSIDBitmap;
#endif

#ifdef RATE_PRIOR_SUPPORT
	ULONG McsTotalRxCount;
	ULONG McsLowRateRxCount;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVMode;
#endif /* IGMP_TVM_SUPPORT */

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
	UCHAR PmkidByHostapd[LEN_PMKID];
	BOOLEAN isIphone;
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	BOOLEAN fgRaLimitPhyRate;
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
	BOOLEAN sta_rec_valid;

#ifdef WTBL_TDD_SUPPORT
	/* backup wtbl, tbd use dynamic alloc */
	CMD_STAREC_UWTBL_RAW_T UWtblRaw;
#endif /* WTBL_TDD_SUPPORT */

#ifdef MSCS_PROPRIETARY
	bool dabs_cfg;
	UINT16 APRandNum;
	UINT16 STARandNum;
	UINT8 dabs_trans_id;
	RALINK_TIMER_STRUCT DABSRetryTimer;
	ULONG DABSTimerFlag;
#endif
	ULONG	TxRetriedPktCount;
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	INT8 tpcPwrAdj;
	INT8 tpcOldPwr;
#endif
#endif
#ifdef SMART_CARRIER_SENSE_SUPPORT
	CHAR ScsDataRssi[4];
#endif /* SMART_CARRIER_SENSE_SUPPORT */
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef enum _MAC_ENT_STATUS_ {
	/* fAnyStationInPsm */
	MAC_TB_ANY_PSM = 0x1,
	/*
		fAnyStationBadAtheros
		Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip.
	*/
	MAC_TB_ANY_ATH = 0x2,
	/*
		fAnyTxOPForceDisable
		Check if it is necessary to disable BE TxOP
	*/
	MAC_TB_FORCE_TxOP = 0x4,
	/*
		fAllStationAsRalink
		Check if all stations are ralink-chipset
	*/
	MAC_TB_ALL_RALINK = 0x8,
	/*
		fAnyStationIsLegacy
		Check if I use legacy rate to transmit to my BSS Station
	*/
	MAC_TB_ANY_LEGACY = 0x10,
	/*
		fAnyStationNonGF
		Check if any Station can't support GF
	*/
	MAC_TB_ANY_NON_GF = 0x20,
	/* fAnyStation20Only */
	MAC_TB_ANY_HT20 = 0x40,
	/*
		fAnyStationMIMOPSDynamic
		Check if any Station is MIMO Dynamic
	*/
	MAC_TB_ANY_MIMO_DYNAMIC = 0x80,
	/*
		fAnyBASession
		Check if there is BA session.  Force turn on RTS/CTS
	*/
	MAC_TB_ANY_BA = 0x100,
	/* fAnyStaFortyIntolerant */
	MAC_TB_ANY_40_INTOlERANT = 0x200,
	/*
		fAllStationGainGoodMCS
		Check if all stations more than MCS threshold
	*/
	MAC_TB_ALL_GOOD_MCS = 0x400,
	/*
		fAnyStationIsHT
		Check if still has any station set the Intolerant bit on!
	*/
	MAC_TB_ANY_HT = 0x800,
	/* fAnyWapiStation */
	MAC_TB_ANY_WAPI = 0x1000,
} MAC_ENT_STATUS;

#define BAND_NUM_MAX 2
typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY * Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	STA_TR_ENTRY tr_entry[MAX_LEN_OF_TR_TABLE];
	/*
		Be care in mgmt_entrytb.c  MacTableReset() will NdisZeroMemory(&pAd->MacTab.Size, sizeof(MAC_TABLE)-offsetof(MAC_TABLE, Size));
		above need to be backup, klock's warnnig @118489 should be mark as not an issue.
	*/
	UINT16 Size;
	QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	MAC_ENT_STATUS sta_status;

	BOOLEAN fAnyStationInPsm;
	BOOLEAN fAnyStationBadAtheros;	/* Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. */
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fAllStationAsRalink[2];	/* Check if all stations are ralink-chipset */
	BOOLEAN fCurrentStaBw40;		/* Check if only one STA w/ BW40 */
#ifdef DOT11_N_SUPPORT
	BOOLEAN fAnyStationIsLegacy;	/* Check if I use legacy rate to transmit to my BSS Station/ */
	BOOLEAN fAnyStationNonGF[BAND_NUM_MAX];	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationMIMOPSDynamic;	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyBASession;	/* Check if there is BA session.  Force turn on RTS/CTS */
	BOOLEAN fAnyStaFortyIntolerant;	/* Check if still has any station set the Intolerant bit on! */
	BOOLEAN fAllStationGainGoodMCS; /* Check if all stations more than MCS threshold */

#endif /* DOT11_N_SUPPORT */


	USHORT MsduLifeTime; /* life time for PS packet */
#ifdef PS_STA_FLUSH_SUPPORT
	BOOLEAN fPsSTAFlushManualMode;
	BOOLEAN fPsSTAFlushEnable;
	UINT16 PsFlushThldTotalMsduNum;
	UINT16 PsFlushPerStaMaxMsduNum;
	UINT16 PsStaNum;
#endif /*PS_STA_FLUSH_SUPPORT*/
#ifdef OUI_CHECK_SUPPORT
	UCHAR oui_mgroup_cnt;
	UINT32 repeater_wcid_error_cnt;
	UINT32 repeater_bm_wcid_error_cnt;
#endif /*OUI_CHECK_SUPPORT*/
	struct _aid_info aid_info;

#ifdef WTBL_TDD_SUPPORT
	NDIS_SPIN_LOCK HashExtLock;
	MAC_TABLE_ENTRY *HashExt[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY ContentExt[WTBL_TDD_SW_MAC_TAB_SEG_NUM][MAX_LEN_OF_MAC_TABLE];
	STA_TR_ENTRY tr_entryExt[WTBL_TDD_SW_MAC_TAB_SEG_NUM][MAX_LEN_OF_TR_TABLE];
	UINT16 SizeExt[WTBL_TDD_SW_MAC_TAB_SEG_NUM];
#endif /* WTBL_TDD_SUPPORT */
} MAC_TABLE, *PMAC_TABLE;

#ifdef SNIFFER_SUPPORT
#define MONITOR_MODE_OFF  0
#define MONITOR_MODE_REGULAR_RX  1
#define MONITOR_MODE_FULL 2
#endif /*SNIFFER_SUPPORT*/

typedef struct _MONITOR_STRUCT {
	struct wifi_dev wdev;
	INT CurrentMonitorMode;
	UINT FilterSize;
	UINT FrameType;
	UCHAR MacFilterAddr[MAC_ADDR_LEN];
	BOOLEAN	MacFilterOn;
	BOOLEAN	bMonitorInitiated;
	BOOLEAN bMonitorOn;
	UINT8 ucDBDCBand;
	UINT16 u2Aid;
	UINT32 u4AmpduRefNum;
} MONITOR_STRUCT;

#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/
#ifdef WDS_SUPPORT
typedef struct _WDS_COUNTER {
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER TransmittedFragmentCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG TxErrors;
	LARGE_INTEGER MulticastReceivedFrameCount;
	ULONG RxNoBuffer;
} WDS_COUNTER, *PWDS_COUNTER;

typedef struct _WDS_ENTRY {
	BOOLEAN Valid;
	UCHAR Addr[MAC_ADDR_LEN];
	ULONG NoDataIdleCount;
	struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct _RT_802_11_WDS_ENTRY {
	struct wifi_dev wdev;
	UCHAR flag;
	UCHAR PeerWdsAddr[MAC_ADDR_LEN];
	struct _MAC_TABLE_ENTRY *peer;
	UCHAR KeyIdx;
	CIPHER_KEY WdsKey;
	UCHAR phy_mode;

	WDS_COUNTER WdsCounter;
} RT_802_11_WDS_ENTRY, *PRT_802_11_WDS_ENTRY;

typedef struct _WDS_TABLE {
	UCHAR Mode[DBDC_BAND_NUM];
	NDIS_SPIN_LOCK WdsTabLock;
	UCHAR wds_num[DBDC_BAND_NUM];
	RT_802_11_WDS_ENTRY WdsEntry[MAX_WDS_ENTRY];
} WDS_TABLE, *PWDS_TABLE;
#endif /* WDS_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#define MAX_IGNORE_AS_REPEATER_ENTRY_NUM	32

typedef struct _MBSS_TO_CLI_LINK_MAP_T {
	struct wifi_dev *mbss_wdev;
	struct wifi_dev *cli_link_wdev;
} MBSS_TO_CLI_LINK_MAP_T;

typedef struct _REPEATER_CLIENT_ENTRY {
	/* BOOLEAN bValid; */
	BOOLEAN wdev_bound;
	BOOLEAN CliEnable;
	BOOLEAN CliValid;
	UCHAR CliIdx;
	UINT8 Cli_Type; /*Bitmap for client type*/
	UCHAR MatchLinkIdx;
	UCHAR CliConnectState; /* 0: disconnect 1: connecting 2: connected */
	UCHAR CliDisconnectState; /* 0: unknown 1: disconnecting */

	RALINK_TIMER_STRUCT ApCliAssocTimer, ApCliAuthTimer;

	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	ULONG CliTriggerTime;
		/* For WPA countermeasures */
	ULONG LastMicErrorTime; /* record last MIC error time */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	UCHAR OriginalAddress[MAC_ADDR_LEN];
	UCHAR CurrentAddress[MAC_ADDR_LEN];
#ifdef ANDLINK_FEATURE_SUPPORT
	UINT	ipaddr; /* In network order */
	UCHAR ipv6addr[16];/*IPV6 support*/
	RTMP_STRING hostname[HOSTNAME_LEN];/*device host name*/
#endif

	PVOID pAd;
	struct _REPEATER_CLIENT_ENTRY *pNext;

	ULONG Disconnect_Sub_Reason;
	ULONG LinkDownReason;

	RTMP_OS_COMPLETION free_ack;

	RALINK_TIMER_STRUCT ReptCliResetTimer;
	struct wifi_dev wdev;
	struct wifi_dev *main_wdev; /* wdev of the main sta */
	MAC_TABLE_ENTRY *pMacEntry;
	ULONG ReptCliIdleCount;

#ifdef FAST_EAPOL_WAR
	BOOLEAN pre_entry_alloc;
#endif /* FAST_EAPOL_WAR */
#if (defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && defined(CONFIG_STA_SUPPORT)
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum; /* Saved PMKID number */
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	NDIS_SPIN_LOCK SavedPMK_lock;
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	UCHAR sae_cfg_group;
	UCHAR rept_PMK[LEN_MAX_PMK]; /* Will move rept_PMK to pMacEntry->PMK after repeater's pMacEntry is created.*/
#endif
#ifdef CONFIG_OWE_SUPPORT
	UCHAR curr_owe_group;
#endif
} REPEATER_CLIENT_ENTRY, *PREPEATER_CLIENT_ENTRY;

typedef struct _REPEATER_CLIENT_ENTRY_MAP {
	PREPEATER_CLIENT_ENTRY pReptCliEntry;
	struct _REPEATER_CLIENT_ENTRY_MAP *pNext;
} REPEATER_CLIENT_ENTRY_MAP, *PREPEATER_CLIENT_ENTRY_MAP;

typedef struct _INVAILD_TRIGGER_MAC_ENTRY {
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR entry_idx;
	BOOLEAN bInsert;
	struct _INVAILD_TRIGGER_MAC_ENTRY *pNext;
} INVAILD_TRIGGER_MAC_ENTRY, *PINVAILD_TRIGGER_MAC_ENTRY;

typedef struct _REPEATER_CTRL_STRUCT {
	INVAILD_TRIGGER_MAC_ENTRY IgnoreAsRepeaterEntry[MAX_IGNORE_AS_REPEATER_ENTRY_NUM];
	INVAILD_TRIGGER_MAC_ENTRY * IgnoreAsRepeaterHash[HASH_TABLE_SIZE];
	UCHAR IgnoreAsRepeaterEntrySize;
} REPEATER_CTRL_STRUCT, *PREPEATER_CTRL_STRUCT;

typedef struct _REPEATER_ADAPTER_DATA_TABLE {
	bool Enabled;
	void *EntryLock;
	void **CliHash;
	void **MapHash;
	void *Wdev_ifAddr;
	void *Wdev_ifAddr_DBDC;
} REPEATER_ADAPTER_DATA_TABLE;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef BW_VENDOR10_CUSTOM_FEATURE
typedef struct _AUTO_BW_MAJOR_POLICY {
	UCHAR ApCliBWSyncBandSupport;     /* 0: No Support, 1: Same Band, 2: Diff (or Both) Band */
	BOOLEAN ApCliBWSyncDeauthSupport; /* 0: No Deauth, 1: Deauth */
} AUTO_BW_MAJOR_POLICY;

typedef struct _AUTO_BW_MINOR_POLICY {
	UCHAR ApCliBWSyncHTSupport;		/* HT Poilcies */
	UCHAR ApCliBWSyncVHTSupport;    /* VHT Poilcies */
} AUTO_BW_MINOR_POLICY;


typedef struct _AUTO_BW_POLICY_TABLE {
	AUTO_BW_MAJOR_POLICY majorPolicy;
	AUTO_BW_MINOR_POLICY minorPolicy;
} AUTO_BW_POLICY_TABLE;
#endif

#ifdef DOT11_HE_AX
typedef struct _BSS_COLOR_CFG {
	UINT8 bss_color_enable[DBDC_BAND_NUM];	/* 0:Bss color disable,255:random init bss color value, 1-63:Manual config bss color value */
	UINT8 bss_color_next[DBDC_BAND_NUM];
	UINT8 rem_ap_bss_color_change_cnt[DBDC_BAND_NUM];
} BSS_COLOR_CFG;
#endif

typedef struct _AP_ADMIN_CONFIG {
	USHORT CapabilityInfo;
	/* Multiple SSID */
	UCHAR BssidNum;
	UCHAR BssidNumPerBand[DBDC_BAND_NUM];				/* per-band BssidNum */
	UCHAR MacMask;
	BSS_STRUCT MBSSID[MAX_BEACON_NUM];
#ifdef DFS_ZEROWAIT_SUPPORT
	UCHAR bChSwitchNoCac;
#endif
	UCHAR Pf2MbssIdxMap[MAX_BEACON_NUM];	/* PROFILE to MBSSID index mapping */
#ifdef DOT11V_MBSSID_SUPPORT
	/* bitmap for 11v transmitted/non-transmitted MBSSID's IdBss */
	UINT32 dot11v_mbssid_bitmap[DBDC_BAND_NUM];
	/* = n, where 2^n is the max number of BSSIDs in Multiple-BSSID set */
	UCHAR dot11v_max_bssid_indicator[DBDC_BAND_NUM];
	/* transmitted IdBss for Multiple BSSID group */
	UCHAR dot11v_trans_bss_idx[DBDC_BAND_NUM];
#endif /* DOT11V_MBSSID_SUPPORT */
	ULONG IsolateInterStaTrafficBTNBSSID;

#ifdef APCLI_SUPPORT
	UCHAR ApCliInfRunned;	/* Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE */
	UINT8 ApCliNum;
	BOOLEAN FlgApCliIsUapsdInfoUpdated;
	/*STA_ADMIN_CONFIG ApCliTab[MAX_APCLI_NUM];	AP-client */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	BOOLEAN		ApCliAutoConnectChannelSwitching;
#ifdef BT_APCLI_SUPPORT
	BOOLEAN	ApCliAutoBWBTSupport;
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	AUTO_BW_POLICY_TABLE ApCliAutoBWRules[DBDC_BAND_NUM];
	BOOLEAN AutoBWDeauthEnbl[DBDC_BAND_NUM]; /* Auto BW Feature Client Deauth Enable */
#endif
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	BOOLEAN		bPartialScanEnable[MAX_APCLI_NUM];
	BOOLEAN		bPartialScanning[MAX_APCLI_NUM];
	ULONG		ApCliIssueScanTime[MAX_APCLI_NUM];
#ifdef ROAMING_ENHANCE_SUPPORT
	BOOLEAN bRoamingEnhance;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bMACRepeaterEn_precfg;
	BOOLEAN bMACRepeaterEn; /* MAC Repeater feature enable */
	BOOLEAN mac_repeater_en[DBDC_BAND_NUM]; /* MAC Reoeater band related enable */
	UCHAR MACRepeaterOuiMode;
	UINT8 EthApCliIdx;
	UCHAR RepeaterCliSize[DBDC_BAND_NUM];
	NDIS_SPIN_LOCK ReptCliEntryLock;
	REPEATER_CLIENT_ENTRY * ReptCliHash[HASH_TABLE_SIZE];
	REPEATER_CLIENT_ENTRY_MAP * ReptMapHash[HASH_TABLE_SIZE];
	UCHAR BridgeAddress[MAC_ADDR_LEN];
	REPEATER_CTRL_STRUCT ReptControl;

	NDIS_SPIN_LOCK CliLinkMapLock;
	MBSS_TO_CLI_LINK_MAP_T  MbssToCliLinkMap[MAX_BEACON_NUM];
	REPEATER_CLIENT_ENTRY *pRepeaterCliPool;
	REPEATER_CLIENT_ENTRY_MAP *pRepeaterCliMapPool;
#endif /* MAC_REPEATER_SUPPORT */

	/* for wpa */
	RALINK_TIMER_STRUCT CounterMeasureTimer;

	UCHAR CMTimerRunning;
	UCHAR BANClass3Data;
	LARGE_INTEGER aMICFailTime;
	LARGE_INTEGER PrevaMICFailTime;
	ULONG MICFailureCounter;

	NDIS_AP_802_11_PMKID PMKIDCache;

	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	BOOLEAN bAutoChannelAtBootup[DBDC_BAND_NUM];	/* 0: disable, 1: enable */
	ChannelSel_Alg AutoChannelAlg[DBDC_BAND_NUM];	/* Alg for selecting Channel */
	BOOLEAN auto_ch_score_flag[DBDC_BAND_NUM];	/* score for Channel, and don't switch channel */
	BOOLEAN set_ch_async_flag;	/* it's an indicator which means "set channel process" is asynchronous (need csa event from FW)*/
	RTMP_OS_COMPLETION	set_ch_aync_done;
	BOOLEAN iwpriv_event_flag;	/* it's an indicator, standing for an iwpriv event */
#ifdef DATA_TXPWR_CTRL
	RTMP_OS_COMPLETION	get_tx_pwr_aync_done;
	BOOLEAN data_pwr_cmd_flag;
#endif
#ifdef AP_SCAN_SUPPORT
	UINT32  ACSCheckTime[DBDC_BAND_NUM]; /* Periodic timer to trigger Auto Channel Selection (unit: second) */
	UINT32  ACSCheckCount[DBDC_BAND_NUM]; /* if  ACSCheckCount > ACSCheckTime, then do ACS check */
#endif /* AP_SCAN_SUPPORT */
	BOOLEAN bAvoidDfsChannel;	/* 0: disable, 1: enable */
	BOOLEAN bIsolateInterStaTraffic;
	BOOLEAN bHideSsid;

	/* temporary latch for Auto channel selection */
	ULONG ApCnt;		/* max RSSI during Auto Channel Selection period */
	UCHAR AutoChannel_Channel;	/* channel number during Auto Channel Selection */
	UCHAR current_channel_index;	/* current index of channel list */
	UCHAR AutoChannelSkipListNum;	/* number of rejected channel list */
	UCHAR AutoChannelSkipListNum6G; /* number of rejected channel list for 6G */
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR AutoChannelSkipList[20];
#else
	UCHAR AutoChannelSkipList[MAX_NUM_OF_CHANNELS + 1];
#endif
	UCHAR AutoChannelSkipList6G[MAX_NUM_OF_CHANNELS + 1];
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	UCHAR ErpIeContent;
	ULONG LastOLBCDetectTime;
	ULONG LastNoneHTOLBCDetectTime;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */

	/* EDCA parameters to be announced to its local BSS */
	EDCA_PARM BssEdcaParm;

	RALINK_TIMER_STRUCT ApQuickResponeForRateUpTimer;
	BOOLEAN ApQuickResponeForRateUpTimerRunning;

#ifdef IDS_SUPPORT
	/* intrusion detection parameter */
	BOOLEAN IdsEnable;
	UINT32 AuthFloodThreshold;	/* Authentication frame flood threshold */
	UINT32 AssocReqFloodThreshold;	/* Association request frame flood threshold */
	UINT32 ReassocReqFloodThreshold;	/* Re-association request frame flood threshold */
	UINT32 ProbeReqFloodThreshold;	/* Probe request frame flood threshold */
	UINT32 DisassocFloodThreshold;	/* Disassociation frame flood threshold */
	UINT32 DeauthFloodThreshold;	/* Deauthentication frame flood threshold */
	UINT32 EapReqFloodThreshold;	/* EAP request frame flood threshold */
	UINT32 DataFloodThreshold;		/* Malicious data frame flood threshold */

	UINT32 RcvdAuthCount;
	UINT32 RcvdAssocReqCount;
	UINT32 RcvdReassocReqCount;
	UINT32 RcvdProbeReqCount;
	UINT32 RcvdDisassocCount;
	UINT32 RcvdDeauthCount;
	UINT32 RcvdEapReqCount;
	UINT32 RcvdMaliciousDataCount;	/* Data Frame DDOS */

	RALINK_TIMER_STRUCT IDSTimer;
	BOOLEAN IDSTimerRunning;
#endif /* IDS_SUPPORT */

	ULONG EntryLifeCheck;
	UINT16 per_err_total;
	ULONG tx_contd_fail_total;

#ifdef DOT11R_FT_SUPPORT
	FT_TAB FtTab;
#endif /* DOT11R_FT_SUPPORT */

#ifdef CLIENT_WDS
	NDIS_SPIN_LOCK CliWdsTabLock;
	PCLIWDS_PROXY_ENTRY pCliWdsEntryPool;
	LIST_HEADER CliWdsEntryFreeList;
	LIST_HEADER CliWdsProxyTb[CLIWDS_HASH_TAB_SIZE];
#endif /* CLIENT_WDS */
	UINT16 EntryClientCount;
#ifdef MBSS_AS_WDS_AP_SUPPORT
	UCHAR wds_mac[MAC_ADDR_LEN];
#endif

#ifdef MT_MAC
	UINT32 ext_mbss_enable_bitmap;
	UINT32 ext_mbss_tttt_enable_bitmap;
#endif /*MT_MAC*/
#ifdef BAND_STEERING
	BOOLEAN BandSteering;
	UINT8	BndStrgBssIdx[MAX_BEACON_NUM];
	BND_STRG_CLI_TABLE BndStrgTable[DBDC_BAND_NUM];
	UINT32	BndStrgHeartbeatCount;
	UINT32	BndStrgHeartbeatMonitor;
	UINT32	BndStrgHeartbeatNoChange;
#endif /* BAND_STEERING */

#ifdef CONFIG_HOTSPOT_R2
	QOS_MAP_TABLE_T HsQosMapTable[MAX_QOS_MAP_TABLE_SIZE];
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef DSCP_PRI_SUPPORT
	UINT8	DscpPriMapSupport;
#endif /*DSCP_PRI_SUPPORT*/

#ifdef CON_WPS
	UINT ConWpsApCliMode;  /* means get profile from rootAp by 2G, 5G perferred or AUTO */
	BOOLEAN ConWpsApCliStatus; /* status of Received the EAPOL-FAIL */
	BOOLEAN ConWpsApCliDisableSetting;
	BOOLEAN ConWpsApDisableSetting;
	BOOLEAN ConWpsApCliDisabled;
	RALINK_TIMER_STRUCT ConWpsApCliBandMonitorTimer;
	BOOLEAN	ConWpsMonitorTimerRunning;
	UINT ConWpsApcliAutoPreferIface;
#endif /* CON_WPS */
#ifdef CONFIG_MAP_SUPPORT
	struct map_policy_setting SteerPolicy;
#endif
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl greenap;
#endif /* GREENAP_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
	struct EventNotifierCfg EventNotifyCfg;
#endif /* WH_EVENT_NOTIFIER */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct customer_oui_filter ap_customer_oui;
	UINT32 ap_probe_rsp_vendor_ie_count;
	UINT32 ap_probe_rsp_vendor_ie_max_count;
#endif
	USHORT ObssGBandChanBitMap;
#ifdef DOT11K_RRM_SUPPORT
	BOOLEAN HandleNRReqbyUplayer;
#endif

#ifdef OCE_SUPPORT
	RALINK_TIMER_STRUCT FdFrameTimer;
	BOOLEAN FdFrameTimerRunning;
	BOOLEAN FdFrameTxsEnabled;
	BOOLEAN FdFrameCurNum;
	RALINK_TIMER_STRUCT APAutoScanNeighborTimer;
#endif /* OCE_SUPPORT */
#ifdef MGMT_TXPWR_CTRL
	UINT8 MgmtTxPwr[DBDC_BAND_NUM]; /* for 5G : 6M (OFDM) pwr  2.4G: 1M (CCK) pwr */
	UINT8 EpaFeGain[DBDC_BAND_NUM];
#endif

#ifdef DATA_TXPWR_CTRL
	UINT8 MaxBaseTxPwr[DBDC_BAND_NUM];
	UINT8 MinBaseTxPwr[DBDC_BAND_NUM];
#endif

#ifdef DOT11_HE_AX
	BSS_COLOR_CFG bss_color_cfg;
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	NDIS_SPIN_LOCK btwt_ie_lock;
	BTWT_BUF_STRUCT btwt[DBDC_BAND_NUM]; /* trigger by bcn offload and used by bcn/brobe rsp/assco rsp*/
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef CONFIG_MAP_SUPPORT
	bool Disallow_ProbeEvent;
#endif /* CONFIG_MAP_SUPPORT */
} AP_ADMIN_CONFIG;

#ifdef CONFIG_AP_SUPPORT
typedef struct _BCN_CHECK_INFO_STRUC {
	UINT32 nobcncnt;		/* nobcn accumulative cnt */
	UINT32 prebcncnt;		/* previous 2.5s bcn cnt */
	UINT32 totalbcncnt;		/* accumulative bcn cnt */
	ULONG  BcnInitedRnd;	/* periodic round count that beacon inited */
} BCN_CHECK_INFO_STRUC, *PBCN_CHECK_INFO_STRUC;
#endif

#ifdef IGMP_SNOOP_SUPPORT
typedef enum _IGMP_GROUP_TYPE {
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE,
	CHANGE_TO_INCLUDE_MODE,
	CHANGE_TO_EXCLUDE_MODE,
	ALLOW_NEW_SOURCES,
	BLOCK_OLD_SOURCES
} IgmpGroupType;

#define GROUP_ENTRY_TYPE_BITMASK	0x0F
#define IPV6_ADDR_LEN               16

typedef enum _MULTICAST_FILTER_ENTRY_TYPE {
/* Group Entry Types	(0 to 0xF) -> Detail of overall entry for a group address*/
	MCAT_FILTER_STATIC = 0,
	MCAT_FILTER_DYNAMIC,
/* Member Types 	(0x10 to 0xFF) -> Detail of the member to be added/included in group address entry*/
#ifdef IGMP_TVM_SUPPORT
	/* If both the ENABLE and AUTO are not set means DISABLE. */
	/* This is used to configure whether each client connected */
	/* to AP has if TV Mode, ENABLED or AUTO, otherwise DISABLED */
	MCAT_FILTER_TVM_ENABLE = 0x10,
	MCAT_FILTER_TVM_AUTO = 0x20,
#endif /* IGMP_TVM_SUPPORT */

#ifdef A4_CONN
	MCAT_FILTER_MWDS_CLI = 0x80,
#endif
} MulticastFilterEntryType;

typedef struct _MEMBER_ENTRY {
	struct _MEMBER_ENTRY *pNext;
	UCHAR Addr[MAC_ADDR_LEN];
#ifdef IGMP_TVM_SUPPORT
		UINT8 TVMode;
#endif /* IGMP_TVM_SUPPORT */

#ifdef A4_CONN
	BOOLEAN onMWDSLink; /*indicates whether this member is on MWDS link*/
#endif
	/*	USHORT Aid; */
} MEMBER_ENTRY, *PMEMBER_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE_ENTRY {
	BOOLEAN Valid;
	MulticastFilterEntryType type;	/* 0: static, 1: dynamic. */
	ULONG lastTime;
#ifdef IGMP_TVM_SUPPORT
	UINT32 AgeOutTime;
#endif
	PNET_DEV net_dev;
	UCHAR Addr[IPV6_ADDR_LEN];
	LIST_HEADER MemberList;
	struct _MULTICAST_FILTER_TABLE_ENTRY *pNext;
} MULTICAST_FILTER_TABLE_ENTRY, *PMULTICAST_FILTER_TABLE_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE {
	UCHAR Size;

	PMULTICAST_FILTER_TABLE_ENTRY
	Hash[MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE];
	MULTICAST_FILTER_TABLE_ENTRY Content[MAX_LEN_OF_MULTICAST_FILTER_TABLE];
	NDIS_SPIN_LOCK MulticastFilterTabLock;
	NDIS_SPIN_LOCK FreeMemberPoolTabLock;
	MEMBER_ENTRY freeMemberPool[FREE_MEMBER_POOL_SIZE];
	LIST_HEADER freeEntryList;
} MULTICAST_FILTER_TABLE, *PMULTICAST_FILTER_TABLE;
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL {
	GREENAP_11BGN_STAS = 0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;

typedef enum _GREEN_AP_SUSPEND_REASON {
	GREENAP_REASON_NONE = 0,
	GREENAP_REASON_AP_BACKGROUND_SCAN = (1 << 0),
	GREENAP_REASON_AP_OVERLAPPING_SCAN = (1 << 1),
	GREENAP_REASON_ACQUIRE_RADIO_FOR_WDEV = (1 << 2),
} GREEN_AP_SUSPEND_REASON;
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

/* ----------- end of AP ---------------------------- */
#endif /* CONFIG_AP_SUPPORT */


struct wificonf {
	BOOLEAN bShortGI;
	BOOLEAN bGreenField;
};

typedef struct _RTMP_DEV_INFO_ {
	UCHAR chipName[16];
	RTMP_INF_TYPE infType;
} RTMP_DEV_INFO;

#ifdef DBG_DIAGNOSE
#define MAX_VHT_MCS_SET	20 /* for 1ss~ 2ss with MCS0~9 */

#define DIAGNOSE_TIME	10	/* 10 sec */

struct dbg_diag_info {
	USHORT TxDataCnt[WMM_NUM_OF_AC];	/* Tx total data count */
	USHORT TxFailCnt;
	USHORT RxDataCnt;	/* Rx Total Data count. */
	USHORT RxCrcErrCnt;

#ifdef DBG_TXQ_DEPTH
	/* TxSwQ length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8 */
	USHORT TxSWQueCnt[WMM_NUM_OF_AC][9];
	UINT32 enq_fall_cnt[WMM_NUM_OF_AC];
	UINT32 deq_fail_no_resource_cnt[WMM_NUM_OF_AC];
	UINT32 deq_called;
	UINT32 deq_round;
	UINT32 deq_cnt[9];
#endif /* DBG_TXQ_DEPTH */

#ifdef DBG_TX_AGG_CNT
	USHORT TxAggCnt;
	USHORT TxNonAggCnt;
	/* TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxAMPDUCnt[16];
#endif /* DBG_TX_AGG_CNT */
};

typedef enum {
	DIAG_COND_ALL = 0,
	DIAG_COND_TXQ_DEPTH = 4,
} DIAG_COND_STATUS;

typedef struct _RtmpDiagStrcut_ {	/* Diagnosis Related element */
	BOOLEAN inited;
	UINT16 wcid;
	UCHAR qIdx;
	UCHAR ArrayStartIdx;
	UCHAR ArrayCurIdx;
	UINT32 diag_cond;

	struct dbg_diag_info diag_info[DIAGNOSE_TIME];
} RtmpDiagStruct;
#endif /* DBG_DIAGNOSE */

/* */
/* The entry of transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY {
	USHORT MACRegisterOffset;	/* MAC register offset */
	ULONG RegisterValue;	/* Register value */
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

/* */
/* The maximum registers of transmit power control */
/* */
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5

/* */
/* The configuration of the transmit power control over MAC */
/* */
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC {
	UCHAR NumOfEntries;	/* Number of entries */
	TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

/* */
/* The extension of the transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC {
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over5G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

/* For Wake on Wireless LAN */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
typedef struct _WOW_CFG_STRUCT {
	BOOLEAN			bEnable;		/* Enable WOW function*/
	BOOLEAN			bWOWFirmware;	/* Enable WOW function, trigger to reload WOW-support firmware */
	BOOLEAN			bInBand;		/* use in-band signal to wakeup system */
	BOOLEAN			bWowIfDownSupport;
	UINT8			nSelectedGPIO;	/* Side band signal to wake up system */
	UINT8			nDelay;			/* Delay number is multiple of 3 secs, and it used to postpone the WOW function */
	UINT32		  nHoldTime;	  /* GPIO pulse hold time, unit: 1us, 0 means hold forever.*/
	BOOLEAN			bWoWRunning;	/* WOW function is working */
	UINT8			nWakeupInterface; /* PCI:0 USB:1 GPIO:2 */
	UINT8			IPAddress[16];	/* Used for ARP response */
	UINT8			bGPIOHighLow;	/* 0: low to high, 1: high to low */
} WOW_CFG_STRUCT, *PWOW_CFG_STRUCT;

typedef enum {
	WOW_GPIO_LOW_TO_HIGH,
	WOW_GPIO_HIGH_TO_LOW
} WOW_GPIO_HIGH_LOW_T;

typedef enum {
	WOW_GPIO_OOTPUT_DISABLE = 0,
	WOW_GPIO_OUTPUT_ENABLE = 1,
} WOW_GPIO_OUTPUT_ENABLE_T;

typedef enum {
	WOW_GPIO_OUTPUT_LEVEL_LOW = 0,
	WOW_GPIO_OUTPUT_LEVEL_HIGH = 1,
} WOW_GPIO_OUTPUT_LEVEL_T;

typedef enum {
	WOW_GPIO_WAKEUP_LEVEL_LOW = 0,
	WOW_GPIO_WAKEUP_LEVEL_HIGH = 1,
} WOW_GPIO_WAKEUP_LEVEL_T;

#define WOW_GPIO_LOW_TO_HIGH_PARAMETER ((WOW_GPIO_OUTPUT_ENABLE << 0) | \
										(WOW_GPIO_OUTPUT_LEVEL_LOW << 1) | (WOW_GPIO_WAKEUP_LEVEL_HIGH << 2))

#define WOW_GPIO_HIGH_TO_LOW_PARAMETER ((WOW_GPIO_OUTPUT_ENABLE << 0) | \
										(WOW_GPIO_OUTPUT_LEVEL_HIGH << 1) | (WOW_GPIO_WAKEUP_LEVEL_LOW << 2));

typedef enum {
	WOW_PKT_TO_HOST,
	WOW_PKT_TO_ANDES
} WOW_PKT_FLOW_T;

typedef enum {
	WOW_WAKEUP_BY_PCIE,
	WOW_WAKEUP_BY_USB,
	WOW_WAKEUP_BY_GPIO
} WOW_WAKEUP_METHOD_T;

typedef enum {
	WOW_ENABLE = 1,
	WOW_TRAFFIC = 3,
	WOW_WAKEUP = 4
} WOW_FEATURE_T;

typedef enum {
	WOW_MASK_CFG = 1,
	WOW_SEC_CFG,
	WOW_INFRA_CFG,
	WOW_P2P_CFG,
} WOW_CONFIG_T;

enum {
	WOW_MAGIC_PKT,
	WOW_BITMAP,
	WOW_IPV4_TCP_SYNC,
	WOW_IPV6_TCP_SYNC
};

typedef struct NEW_WOW_MASK_CFG_STRUCT {
	UINT32	Config_Type;
	UINT32	Function_Enable;
	UINT32	Detect_Mask;
	UINT32	Event_Mask;
} NEW_WOW_MASK_CFG_STRUCT, PNEW_WOW_MASK_CFG_STRUCT;

typedef struct NEW_WOW_SEC_CFG_STRUCT {
	UINT32	Config_Type;
	UINT32	WPA_Ver;
	UCHAR	PTK[64];
	UCHAR	R_COUNTER[8];
	UCHAR	Key_Id;
	UCHAR	Cipher_Alg;
	UCHAR	WCID;	/* #256STA */
	UCHAR	Group_Cipher;
} NEW_WOW_SEC_CFG_STRUCT, PNEW_WOW_SEC_CFG_STRUCT;

typedef struct NEW_WOW_INFRA_CFG_STRUCT {
	UINT32	Config_Type;
	UCHAR	STA_MAC[6];
	UCHAR	AP_MAC[6];
	UINT32	AP_Status;
} NEW_WOW_INFRA_CFG_STRUCT, PNEW_WOW_INFRA_CFG_STRUCT;

typedef struct _NEW_WOW_P2P_CFG_STRUCT {
	UINT32	Config_Type;
	UCHAR	GO_MAC[6];
	UCHAR	CLI_MAC[6];
	UINT32	P2P_Status;
} NEW_WOW_P2P_CFG_STRUCT, *PNEW_WOW_P2P_CFG_STRUCT;

typedef struct _NEW_WOW_PARAM_STRUCT {
	UINT32	Parameter;
	UINT32	Value;
} NEW_WOW_PARAM_STRUCT, *PNEW_WOW_PARAM_STRUCT;

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)  || defined(MT_WOW_SUPPORT) */

/*
	Packet drop reason code
*/
typedef enum {
	PKT_ATE_ON = 1 << 8,
	PKT_RADAR_ON = 2 << 8,
	PKT_RRM_QUIET = 3 << 8,
	PKT_TX_STOP = 4 << 8,
	PKT_TX_JAM = 5 << 8,

	PKT_NETDEV_DOWN = 6 < 8,
	PKT_NETDEV_NO_MATCH = 7 << 8,
	PKT_NOT_ALLOW_SEND = 8 << 8,

	PKT_INVALID_DST = 9 << 8,
	PKT_INVALID_SRC = 10 << 8,
	PKT_INVALID_PKT_DATA = 11 << 8,
	PKT_INVALID_PKT_LEN = 12 << 8,
	PKT_INVALID_ETH_TYPE = 13 << 8,
	PKT_INVALID_TXBLK_INFO = 14 << 8,
	PKT_INVALID_SW_ENCRYPT = 15 << 8,
	PKT_INVALID_PKT_TYPE = 16 << 8,
	PKT_INVALID_PKT_MIC = 17 << 8,

	PKT_PORT_NOT_SECURE = 18 << 8,
	PKT_TSPEC_NO_MATCH  = 19 << 8,
	PKT_NO_ASSOCED_STA = 20 << 8,
	PKT_INVALID_MAC_ENTRY = 21 << 8,

	PKT_TX_QUE_FULL = 22 << 8,
	PKT_TX_QUE_ADJUST = 23 << 8,

	PKT_PS_QUE_TIMEOUT = 24 << 8,
	PKT_PS_QUE_CLEAN = 25 << 8,
	PKT_MCAST_PS_QUE_FULL = 26 << 8,
	PKT_UCAST_PS_QUE_FULL = 27 << 8,

	PKT_RX_EAPOL_SANITY_FAIL = 28 << 8,
	PKT_RX_NOT_TO_KERNEL = 29 << 8,
	PKT_RX_MESH_SIG_FAIL = 30 << 8,
	PKT_APCLI_FAIL = 31 << 8,
	PKT_ZERO_DATA = 32 << 8,
	PKT_SW_DECRYPT_FAIL = 33 << 8,
	PKT_TX_SW_ENC_FAIL = 34 << 8,

	PKT_ACM_FAIL = 35 << 8,
	PKT_IGMP_GRP_FAIL = 36 << 8,
	PKT_MGMT_FAIL = 37 << 8,
	PKT_AMPDU_OUT_ORDER = 38 << 8,
	PKT_UAPSD_EOSP = 39 << 8,
	PKT_UAPSD_Q_FULL = 40 << 8,

	PKT_DRO_REASON_MAX = 41,
} PKT_DROP_REASON;

/* Packet drop Direction code */
typedef enum {
	PKT_TX = 0,
	PKT_RX = 1 << 31,
} PKT_DROP_DIECTION;

#ifdef SMART_ANTENNA
/*
	1. The combination of all possible training patterns =
		txNss : the maxima antennas used for transmission,
					e.g., for 3*3 device, the txNss = 3
		When user try to select the antenna, they shall use following format to do the selection
				iwpriv ra0 set sa_ant=1,5
					=> it means user try to use PinHeader 1 and pinHeader 5 as the tx antenna

	2. The Training sequence used for smart antenna traing
		pTrainSeq: is a sequence used to do antenna selection.

	the time of one round for the antenna selection training:
		totalTime = maxAntTry
*/

#define SA_TX_NSS_MAX_NUM	3
#define SA_DBG_LOG_MAX_CNT	10
#define SA_ENTRY_MAX_NUM 1

#define SA_DEFAULT_TX_CNT	100
#define SA_DEFAULT_TX_NSS		2
#define SA_DEFAULT_MCS_BOUND	23
#define SA_DEFAULT_MSC			5
#define SA_DEFAULT_CHK_PERIOD	200	/* 400 */
#define SA_DEFAULT_ANT_TRIAL	1
#define SA_DEFAULT_RSSI_VAR		8 /* 5 */
#define SA_DEFAULT_RSSI_THRESHOLD	-80

typedef enum _RTMP_SA_OP_MODE_ {
	SA_MODE_NONE = 0,
	SA_MODE_MANUAL = 1,
	SA_MODE_ONESHOT = 2,
	SA_MODE_AUTO = 3,
} RTMP_SA_OP_MODE;

typedef enum _RTMP_SA_STA_RULE_ {
	SA_STA_BY_DEFAULT = 0, /* first in or static assign */
	SA_STA_BY_RSSI = 1,
} RTMP_SA_STA_RULE;

typedef struct _RTMP_SA_AGSP_MAP_ {
	UINT8 hdrPin;
	UINT32 regOffset;/* The GPIO address */
	UINT32 gpioBit;	/* the bit field in the regOffset */
} RTMP_SA_AGSP_MAP;

typedef struct _RTMP_SA_TRAIN_LOG_ELEMENT_ {
	UINT32 antPattern;
	UINT32 patternOffset;
	UCHAR antWeight;
	UCHAR candWeight;
	ULONG srtTime;
	ULONG endTime;
	UINT32 txMcs;
	UINT32 txCnt;			/* txNoRtyCnt + RtyOkCnt + FailCnt */
	UINT32 txNoRtyCnt;
	UINT32 txRtyCnt;		/* RtyOkCnt + FailCnt */
	UINT32 txRtyFailCnt;	/* FailCnt */
	UINT32 PER;				/* (txRtyCnt * 100) / txCnt */
	UINT32 rxCnt;
	INT32  sumRSSI[SA_TX_NSS_MAX_NUM];
	UINT32 cntRSSI[SA_TX_NSS_MAX_NUM];
	INT32  avgRSSI[SA_TX_NSS_MAX_NUM];
	INT32  sumSNR[SA_TX_NSS_MAX_NUM];
	UINT32 cntSNR[SA_TX_NSS_MAX_NUM];
	INT32  avgSNR[SA_TX_NSS_MAX_NUM];
	INT32  sumPreSNR[SA_TX_NSS_MAX_NUM];
	UINT32 cntPreSNR[SA_TX_NSS_MAX_NUM];

#ifdef SA_DBG
	UINT32 rssiDist[SA_TX_NSS_MAX_NUM][33];
	UINT32 SNRDist[SA_TX_NSS_MAX_NUM][33];
	UINT32 preSNRDist[SA_TX_NSS_MAX_NUM][33];
	UINT32 txMcsDist[MAX_MCS_SET];			/* the txMcs distruction cnt; */
	UINT32 rxMcsDist[MAX_MCS_SET];			/* the rxMcs distruction cnt; */
#endif /* SA_DBG // */
} RTMP_SA_TRAIN_LOG_ELEMENT;

typedef struct _RTMP_SA_TRAIN_LOG_ {
	UINT32 antPattern;
	int lastRnd;
	int firstRnd;
	RTMP_SA_TRAIN_LOG_ELEMENT record[SA_DBG_LOG_MAX_CNT];
} RTMP_SA_TRAIN_LOG;

typedef struct _RTMP_SA_TRAIN_SEQ_ {
	UINT32 antPattern;
	RTMP_SA_AGSP_MAP *pAgspEntry;
} RTMP_SA_TRAIN_SEQ;

/*
	For SmartAntenna auto training mode, it's a three-stages loop state machine
	1. Initial stage (first time)
		do fully scan for all antenna patterns
	2. Confirm stage (second time)
		do fully scan for all antenna patterns

		if Initial stage and confirm stage results are the same
			=> goto Monitor stage
		else
			=> goto confirm stage
	3. Monitor stage (third and following)
		Only do RSSI monitoring.

		if | CurrentRSSI - previousRSSI| <= 5
			=> stay in Monitor stage
		else
			=> go to initial stage
*/
typedef enum {
	SA_INVALID_STAGE = 0,
	SA_INIT_STAGE = 1,
	SA_CONFIRM_STAGE = 2,
	SA_MONITOR_STAGE = 3,
} RTMP_SA_TRAIN_STAGE;

#define ANT_WEIGHT_SCAN_ALL 0xf0
#define ANT_WEIGHT_SCAN_AVG 0x70
#define ANT_WEIGHT_SCAN_ONE	0x10

#define ANT_WEIGHT_CAND_HIGH	0x01
#define ANT_WEIGHT_CAND_AVG		0x02
#define ANT_WEIGHT_CAND_LOW		0x03
#define ANT_WEIGHT_CAND_INIT	0xff

#define ANT_SELECT_FIRST		0x01
#define ANT_SELECT_IGNORE_BASE	0x02
#define ANT_SELECT_BASE			0x04

typedef struct _RTMP_SA_TRAINING_PARAM_ {
	RTMP_SA_TRAIN_LOG_ELEMENT *pTrainInfo;

	UCHAR macAddr[MAC_ADDR_LEN];
	BOOLEAN bStatic;		/* Indicate if this entry assigned by user or by driver itself */

	MAC_TABLE_ENTRY *pMacEntry;

	/* Indicate the info */
	RTMP_SA_TRAIN_LOG_ELEMENT antBaseInfo;

	UINT32 curAntPattern;	/* indicate current antenna pattern used for transmission */
	UINT32 patternOffset;	/* Indicate the offset of the antPattern compare to the pSAParam->pTrainSeq; */
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurTrainInfo;

	UINT32 canAntPattern;	/* The best candidate until now! */
	RTMP_SA_TRAIN_LOG_ELEMENT *pCanTrainInfo;

	RTMP_SA_TRAIN_STAGE	trainStage;
	UINT32 ant_init_stage;
	UINT32 ant_confirm_stage;
	ULONG time_to_start;	/* in units of system Clk */

	UCHAR mcsStableCnt;
	UCHAR trainWeight;
	BOOLEAN bTraining;		/* set as TRUE when trianing procedure is on-going */
	BOOLEAN bLastRnd;

#ifdef SA_TRAIN_SBS
	BOOLEAN bRoundDone;
#endif /* SA_TRAIN_SBS // */

#ifdef SA_LUMP_SUM
	UINT32 sumTxCnt;
	UINT32 sumTxRtyCnt;
	UINT32 sumTxFailCnt;
#endif /* SA_LUMP_SUM // */
} RTMP_SA_TRAINING_PARAM;

typedef struct _SMART_ANTENNA_STRUCT_ {
	RTMP_SA_OP_MODE saMode;		/* 1 = manually, 2= one shot , 3 =auto learn */

	/* Number of antennas used for the transmission in the same time. */
	UCHAR txNss;

	/* Mcs Stable Count, define the requirements for trigger the
		Antenna switch algorithm,
			0~254: valid count.
			255: reserved value
	*/
	UCHAR saMsc;

	/*
		MCS upper bound for SmartAntenna adaptive tunning.
	*/
	UCHAR saMcsBound;

	/* The maximum number of antenna pattern in one testing period */
	UCHAR maxAntTry;

	/* condition for sa training */
	UINT32 trainCond;	/* used for method 3 */

	/* Method for Antenna candidate selection */
	UINT32 candMethod;

	/* the delay time(in seconds) before do the antenna switching training procedures */
	UINT32 trainDelay;

	/* Time period of simple the data info for a specific antenna period. */
	INT32 chkPeriod;

	/* RSSI variance threshod for training procedures */
	UCHAR rssiVar;

	/* threshold value used for select target training entry */
	CHAR rssiThreshold;

	/* condition to check if need to skip the confirm stage when run in auto mode */
	BOOLEAN bSkipConfStage;

	/* indicate if any station associate/disassociate to us and need to re-do the training procedure */
	BOOLEAN bStaChange;

	/* indicate if the Rssi variance is larger than threshold for sa training */
	BOOLEAN bRssiChange;

	/* the antenna header pin and gpio mapping */
	RTMP_SA_AGSP_MAP *agsp;
	INT32 agspCnt;

	/* Training sequence of antenna pattern used to do simpling.
		0x0: indicate the end of the sequence.
	*/
	UINT32 *pTrainSeq;
	RTMP_SA_TRAIN_LOG_ELEMENT *pTrainMem;
	/* Total length of the training sequence, not include the terminator. */
	int trainSeqCnt;

	RALINK_TIMER_STRUCT	 saSwitchTimer;

	/* Rule used to select the target Training entry */
	UINT32	candStaRule;

	RTMP_SA_TRAINING_PARAM trainEntry[SA_ENTRY_MAX_NUM];

} SMART_ANTENNA_STRUCT;
#endif /* SMART_ANTENNA // */

#define DEFLAUT_PARTIAL_SCAN_CH_NUM		1
#define DEFLAUT_PARTIAL_SCAN_BREAK_TIME	4  /* Period of partial scaning: unit: 100ms */
typedef struct _PARTIAL_SCAN_ {
	BOOLEAN bScanning;			/* Doing partial scan or not */
	UINT8	 NumOfChannels;			/* How many channels to scan each time */
	UINT8	 LastScanChannel;		/* last scaned channel */
	UINT32	 BreakTime;			/* Period of partial scanning: unit: 100ms */
	UINT32	 TimerInterval;		/* The time interval between scan epoches: unit 1 ms*/
	RALINK_TIMER_STRUCT PartialScanTimer; /*Count the time interval between scan epoch*/
	struct	 wifi_dev *pwdev;
} PARTIAL_SCAN;
#ifdef OFFCHANNEL_SCAN_FEATURE

typedef enum {
	OFFCHANNEL_SCAN_INVALID = 0,
	OFFCHANNEL_SCAN_START,
	OFFCHANNEL_SCAN_COMPLETE,
	OFFCHANNEL_SCAN_MAX
} RTMP_OFFCHANNEL_SCAN_STAGE;
#define BW_20_SCAN 0
#define BW_OPER_SCAN 1
#endif
typedef struct _SCAN_CHANNEL_SKIPLIST_ {
	UINT8 Channel;
} SCAN_CHANNEL_SKIPLIST;

/*dwell time defined by user*/
typedef struct _USR_DEF_DWELL_ {
	BOOLEAN isActive;
	UINT16 dwell_t_5g;
	UINT16 dwell_t_2g;
} USR_DEF_DWELL;

typedef struct _SCAN_CTRL_ {
	UCHAR ScanType;
	UCHAR BssType;
	UCHAR Channel;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Bssid[MAC_ADDR_LEN];
#ifdef OFFCHANNEL_SCAN_FEATURE
	UCHAR			if_name[32];
	UCHAR			ScanGivenChannel[MAX_AWAY_CHANNEL];
	UCHAR			ScanTime[MAX_AWAY_CHANNEL];
	UCHAR			CurrentGivenChan_Index;
	UCHAR			Num_Of_Channels;
	UCHAR			Offchan_Scan_Type[MAX_AWAY_CHANNEL];
	RTMP_OFFCHANNEL_SCAN_STAGE					 state;
	BOOLEAN			OffChScan;
	BOOLEAN			OffChScan_Ongoing;
	UCHAR			Off_Ch_Scan_BW;
	ktime_t			ScanTimeActualStart;
	ktime_t			ScanTimeActualEnd;
	UCHAR			ScanTimeActualDiff;
	BOOLEAN         scan_by_sr;
#ifdef OFFCHANNEL_ZERO_LOSS
	MT_MIB_COUNTER_STAT OffCannelScanStartStats;
	MT_MIB_COUNTER_STAT OffCannelScanStopStats;
	RTMP_OS_COMPLETION OffChannel_IdlePwr_Measure_complete;
	ktime_t ScanTimeStartMibEvent;
	ktime_t ScanTimeEndMibEvent;
	UCHAR	ScanTimeDiff;
#endif
#endif
	UINT8 SkipCh_Num;
	SCAN_CHANNEL_SKIPLIST *SkipList;
	BOOLEAN dfs_ch_utilization;

	STATE_MACHINE	  SyncFsm;
	STATE_MACHINE_FUNC SyncFsmFun[SYNC_FSM_FUNC_SIZE];

	TIMER_FUNC_CONTEXT SyncTimerFuncContex;
	RALINK_TIMER_STRUCT ScanTimer;

	PARTIAL_SCAN PartialScan;
	USR_DEF_DWELL Usr_dwell;
	UCHAR BandIdx;
	BSS_TABLE ScanTab;
	struct wifi_dev *ScanReqwdev;
	struct wifi_dev *ImprovedScanWdev;
} SCAN_CTRL;

#ifdef MEMORY_OPTIMIZATION
#define TX_SWQ_FIFO_LEN	2048
#else
#define TX_SWQ_FIFO_LEN	8192
#endif
typedef struct tx_swq_fifo {
	UINT16 swq[TX_SWQ_FIFO_LEN]; /* value 0 is used to indicate free to insert, value 1~127 used to incidate the WCID entry */
	UINT enqIdx;
	UINT deqIdx;
	UINT low_water_mark;
	UINT high_water_mark;
	BOOLEAN q_state;
	NDIS_SPIN_LOCK swq_lock;	/* spinlock for swq */
} TX_SWQ_FIFO;

#ifdef RT_CFG80211_SUPPORT
typedef struct _CFG80211_VIF_DEV {
	struct _CFG80211_VIF_DEV *pNext;
	BOOLEAN isMainDev;
	UINT32 devType;
	PNET_DEV net_dev;
	UCHAR CUR_MAC[MAC_ADDR_LEN];
#ifdef IWCOMMAND_CFG80211_SUPPORT
	CHAR ucfgIfName[IFNAMSIZ];
#endif /* IWCOMMAND_CFG80211_SUPPORT */

	/* ProbeReq Frame */
	BOOLEAN Cfg80211RegisterProbeReqFrame;
	CHAR Cfg80211ProbeReqCount;

	/* Action Frame */
	BOOLEAN Cfg80211RegisterActionFrame;
	CHAR Cfg80211ActionCount;
} CFG80211_VIF_DEV, *PCFG80211_VIF_DEV;

typedef struct _CFG80211_VIF_DEV_SET {
#define MAX_CFG80211_VIF_DEV_NUM  2

	BOOLEAN inUsed;
	UINT32 vifDevNum;
	LIST_HEADER vifDevList;
	BOOLEAN isGoingOn; /* To check any vif in list */
} CFG80211_VIF_DEV_SET;

/* TxMmgt Related */
typedef struct _CFG80211_TX_PACKET {
	struct _CFG80211_TX_PACKET *pNext;
	UINT32 TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;		  /* TxMgmt Packet buffer Length */

} CFG80211_TX_PACKET, *PCFG80211_TX_PACKET;

/* CFG80211 Total CTRL Point */
typedef struct _CFG80211_CONTROL {
	BOOLEAN FlgCfg8021Disable2040Scan;
	BOOLEAN FlgCfg80211Scanning;   /* Record it When scanReq from wpa_supplicant */
	BOOLEAN FlgCfg80211Connecting; /* Record it When ConnectReq from wpa_supplicant*/

	/* Scan Related */
	UINT32 *pCfg80211ChanList;	/* the channel list from from wpa_supplicant */
	UCHAR Cfg80211ChanListLen;	/* channel list length */
	UCHAR Cfg80211CurChanIndex;   /* current index in channel list when driver in scanning */

	UCHAR *pExtraIe;  /* Carry on Scan action from supplicant */
	UINT   ExtraIeLen;

	UCHAR Cfg_pending_Ssid[MAX_LEN_OF_SSID + 1]; /* Record the ssid, When ScanTable Full */
	UCHAR Cfg_pending_SsidLen;

	/* ROC Related */
	RALINK_TIMER_STRUCT Cfg80211RocTimer;
	CMD_RTPRIV_IOCTL_80211_CHAN Cfg80211ChanInfo;
	BOOLEAN Cfg80211RocTimerInit;
	BOOLEAN Cfg80211RocTimerRunning;

	/* Tx_Mmgt Related */
	UINT32 TxStatusSeq;			  /* TxMgmt Packet ID from sequence */
	UCHAR *pTxStatusBuf;		  /* TxMgmt Packet buffer content */
	UINT32 TxStatusBufLen;		  /* TxMgmt Packet buffer Length */
	BOOLEAN TxStatusInUsed;
	LIST_HEADER cfg80211TxPacketList;

	/* P2P Releated*/
	UCHAR P2PCurrentAddress[MAC_ADDR_LEN];	  /* User changed MAC address */
	BOOLEAN isCfgDeviceInP2p;				  /* For BaseRate 6 */

	/* MainDevice Info. */
	CFG80211_VIF_DEV cfg80211MainDev;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(IWCOMMAND_CFG80211_SUPPORT)
	/* For add_virtual_intf */
	CFG80211_VIF_DEV_SET Cfg80211VifDevSet;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA || IWCOMMAND_CFG80211_SUPPORT */

#ifdef CFG80211_MULTI_STA
	PNET_DEV multi_sta_net_dev;
	BOOLEAN flg_cfg_multi_sta_init;
#endif /* CFG80211_MULTI_STA */

#ifdef RT_CFG80211_P2P_SUPPORT
	BOOLEAN bP2pCliPmEnable;

	BOOLEAN bPreKeepSlient;
	BOOLEAN	bKeepSlient;

	UINT16 MyGOwcid;
	UCHAR NoAIndex;
	UCHAR CTWindows; /* CTWindows and OppPS parameter field */

	P2PCLIENT_NOA_SCHEDULE GONoASchedule;
	RALINK_TIMER_STRUCT P2pCTWindowTimer;
	RALINK_TIMER_STRUCT P2pSwNoATimer;
	RALINK_TIMER_STRUCT P2pPreAbsenTimer;

	UCHAR P2pSupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR P2pSupRateLen;
	UCHAR P2pExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR P2pExtRateLen;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Dummy P2P Device for ANDROID JB */
	PNET_DEV dummy_p2p_net_dev;
	struct wifi_dev dummy_p2p_wdev;
	BOOLEAN flg_cfg_dummy_p2p_init;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
	ULONG P2POpStatusFlags; /* P2P_CLI_UP / P2P_GO_UP*/
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

	/* In AP Mode */
	UINT8 isCfgInApMode;	/* Is any one Device in AP Mode */
	UCHAR *beacon_tail_buf; /* Beacon buf from upper layer */
	UINT32 beacon_tail_len;
	BOOLEAN beaconIsSetFromHostapd; /* set true after BeaconAdd */

	UCHAR *pCfg80211ExtraIeAssocRsp;
	UINT32 Cfg80211ExtraIeAssocRspLen;

	/* for AdHoc Mode */
	UCHAR *BeaconExtraIe;
	UINT  BeaconExtraIeLen;

	/* Mcc Part */
	/* BOOLEAN isMccOn; */

	/* TODO: need fix it */
	UCHAR Cfg80211_Alpha2[2];
	CMD_RTPRIV_IOCTL_80211_KEY WepKeyInfoBackup;
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
	AUTH_FRAME_INFO auth_info;
#endif
} CFG80211_CTRL, *PCFG80211_CTRL;
#endif /* RT_CFG80211_SUPPORT */

typedef struct rtmp_mac_ctrl {
#ifdef MT_MAC
	UINT16 wtbl_entry_cnt[4];
	UINT16 wtbl_entry_size[4];
	UINT32 wtbl_base_addr[4]; /* base address for WTBL2/3/4 */
	UINT32 wtbl_base_fid[4];
	UINT32 page_size;
#endif /* MT_MAC */

} RTMP_MAC_CTRL;


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
typedef struct _QLOAD_CTRL {
	UINT8 FlgQloadEnable;	/* 1: any BSS WMM is enabled */
	ULONG QloadUpTimeLast;	/* last up time */
	UINT8 QloadChanUtil;	/* last QBSS Load, unit: us, primary channel only */
	UINT8 QloadChanUtilCcaNavTx;	/* last QBSS Load, unit: us, including CCA NAV TX time */
	UINT32 QloadChanUtilTotal;	/* current QBSS Load Total, primary channel only */
	UINT32 QloadChanUtilTotalCcaNavTx;	/* current QBSS Load Total, including CCA NAV TX time */
	UINT8 QloadChanUtilBeaconCnt;	/* 1~100, default: 50 */
	UINT8 QloadChanUtilBeaconInt;	/* 1~100, default: 50 */
	UINT32 QloadLatestChannelBusyTimePri;
	UINT32 QloadLatestChannelBusyTimeSec;

	/*
	   ex: For 100ms beacon interval,
	   if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
	   if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
	   ......
	   if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

	   Command: "iwpriv ra0 qload show".
	 */

	/* provide busy time statistics for every TBTT */
#define QLOAD_FUNC_BUSY_TIME_STATS

	/* provide busy time alarm mechanism */
	/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	UINT32 QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	UINT32 QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(HcGetQloadCtrl(pAd)->FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (HcGetQloadCtrl(pAd)->QloadAlarmNumber > 0)
	BOOLEAN FlgQloadAlarmIsSuspended;	/* 1: suspend */

	UINT8 QloadAlarmBusyTimeThreshold;	/* unit: 1/100 */
	UINT8 QloadAlarmBusyNumThreshold;	/* unit: 1 */
	UINT8 QloadAlarmBusyNum;
	UINT8 QloadAlarmDuration;	/* unit: TBTT */

	UINT32 QloadAlarmNumber;	/* total alarm times */
	BOOLEAN FlgQloadAlarm;	/* 1: alarm occurs */

	/* speed up use */
	UINT32 QloadTimePeriodLast;
	UINT32 QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */
} QLOAD_CTRL;
#endif /* AP_QLOAD_SUPPORT */

struct _AUTO_CH_CTRL {
	PCHANNELINFO pChannelInfo;
	PBSSINFO pBssInfoTab;
#ifdef CONFIG_AP_SUPPORT
	AUTOCH_SEL_CTRL AutoChSelCtrl;
#endif
};

#endif /*CONFIG_AP_SUPPORT*/

struct pe_control {
	struct he_pe_info pe_info;
};



typedef struct _MANUAL_CONN {
	UINT8 peer_mac[MAC_ADDR_LEN];
	UINT8 peer_band;		/* band 0/1 */
	UINT8 peer_op_type;		/* ap/sta for "OPMODE_AP"/"OPMODE_STA" */
	UINT8 ownmac_idx;
	UINT8 wtbl_idx;
	UINT8 peer_phy_mode;	/* a/b/g/gn/an/ac for "WMODE_A/B/G/GN/AN/AC" */
	UINT8 peer_bw;			/* 20/40/80/160 for "BW_20/40/80/160" */
	UINT8 peer_nss;			/* 1 ~ 4 */
	UINT16 pfmuId;
	UINT8  spe_idx;
	UCHAR  gid;
	UINT16 aid;
	UINT8  rca2;
	UINT8  rv;
	UINT8 peer_maxrate_mode;	/* cck/ofdm/htmix/htgf/vht for "MODE_CCK/OFDM/HTMIX/HTGF/VHT" */
	UINT32 peer_maxrate_mcs;	/* MODE_CCK: 0~3, MODE_OFDM: 0~7, MODE_HTMIX/GF: 0~32, MODE_VHT:0~9 */
	UINT8 ba_info[WMM_NUM_OF_AC];

	/* protocol wise */
	HT_CAP_INFO ht_cap_info;	/* HT capability information */
#ifdef DOT11_VHT_AC
	VHT_CAP_INFO vht_cap_info;	/* VHT capability information */
	VHT_MCS_SET vht_mcs_set;
#endif
} MANUAL_CONN, *P_MANUAL_CONN;

#ifdef WFA_VHT_R2_PF
typedef struct _MANUAL_DUMP {
	UINT8 VhtBwSignal;
	UINT8 VhtCapIE;
	UINT8 VhtOpNotify;
} MANUAL_DUMP, *P_MANUAL_DUMP;
#endif /* WFA_VHT_R2_PF  */

/* Rx Statistic */
#define MAX_ANT_NUM 4
#define MAX_USER_NUM 16
typedef struct _RX_STATISTIC_RXV {
	UINT32 rxv_cnt;
	INT32 FreqOffsetFromRx[MAX_USER_NUM];
	INT32 RSSI[MAX_ANT_NUM];
	INT32 SNR[MAX_USER_NUM];
	INT32 RCPI[MAX_ANT_NUM];
	INT32 FAGC_RSSI_IB[MAX_ANT_NUM];
	INT32 FAGC_RSSI_WB[MAX_ANT_NUM];
	BOOLEAN fcs_error[MAX_USER_NUM];
	UINT_32 fcs_error_cnt[MAX_USER_NUM];
	UINT8 pfd;
	UINT8 vht_gid;
	BOOLEAN rx_vld_ind[MAX_USER_NUM];
	UINT32 rx_mu_ok_cnt[MAX_USER_NUM];
} RX_STATISTIC_RXV;

typedef struct _RX_STAT_CNT {
	UINT32 all_mac_rx_mdrdy_cnt;
	UINT32 all_mac_rx_ok_cnt;
	UINT32 all_mac_rx_fcs_err_cnt;
	UINT32 all_mac_rx_len_mismatch;
	UINT32 all_mac_rx_fifo_full;
	UINT32 all_per;
} RX_STAT_CNT, *P_RX_STAT_CNT;

typedef struct _RX_STATISTIC_CR {
	UINT32 RxMacFCSErrCount;
	UINT32 RxMacMdrdyCount;
	UINT32 RxMacFCSOKCount;
	UINT32 PhyMdrdyOFDM;
	UINT32 PhyMdrdyCCK;
	UINT32 FCSErr_OFDM;
	UINT32 FCSErr_CCK;
	UINT32 RxLenMismatch;
	UINT32 OFDM_PD;
	UINT32 CCK_PD;
	UINT32 CCK_SIG_Err;
	UINT32 CCK_SFD_Err;
	UINT32 OFDM_SIG_Err;
	UINT32 OFDM_TAG_Err;
	UINT32 ACIHitLow;
	UINT32 ACIHitHigh;
	UINT32 Inst_IB_RSSSI[MAX_ANT_NUM];
	UINT32 Inst_WB_RSSSI[MAX_ANT_NUM];
	UINT32 RxMacFCSErrCount_band1;
	UINT32 RxMacMdrdyCount_band1;
	UINT32 RxMacFCSOKCount_band1;
	UINT32 PhyMdrdyOFDM_band1;
	UINT32 PhyMdrdyCCK_band1;
	UINT32 RxLenMismatch_band1;
	UINT32 OFDM_PD_band1;
	UINT32 CCK_PD_band1;
	UINT32 CCK_SIG_Err_band1;
	UINT32 CCK_SFD_Err_band1;
	UINT32 OFDM_SIG_Err_band1;
	UINT32 OFDM_TAG_Err_band1;
} RX_STATISTIC_CR;

struct peak_tp_ctl {
	UCHAR client_nums;
	/* handle main STA's TP, maximum TP of (TX+RX) */
	struct wifi_dev *main_wdev;
	MAC_TABLE_ENTRY *main_entry;
	BOOLEAN	cli_peak_tp_running;
	UINT32	main_tx_tp;
	UINT32	main_rx_tp;
#ifdef DELAY_TCP_ACK_V2
	UINT32	main_tx_tp_record;
	UINT32	main_rx_tp_record;
#endif /* DELAY_TCP_ACK_V2 */
	UINT8	main_traffc_mode;
	ULONG	main_txrx_bytes;
	BOOLEAN	cli_peak_tp_txop_enable;
	UINT16	cli_peak_tp_txop_level;
	BOOLEAN	cli_ampdu_efficiency_running;
};

#ifdef ANTENNA_DIVERSITY_SUPPORT
#define ANT_MAIN									0
#define ANT_AUX										1
#define ANT_INIT									2
#define ANT_NUMBER									2
#define UL_MODE_TH_2G								60
#define UL_MODE_TH_5G								300
#define DL_MODE_TH_2G								0xffff//80
#define DL_MODE_TH_5G								0xffff//300
#define DL_DEGRADE_TH_2G							90
#define DL_DEGRADE_TH_5G							90
#define UL_CN_DEGRADE_TH_MAX_2G						5
#define UL_CN_DEGRADE_TH_MAX_5G						5
#define UL_CN_DEGRADE_TH_MIN_2G						3
#define UL_CN_DEGRADE_TH_MIN_5G						3
#define UL_RX_RATE_DEGRADE_TH_2G					20
#define UL_RX_RATE_DEGRADE_TH_5G					30
#define ANT_COUNTDOWN_2G							2
#define ANT_COUNTDOWN_5G							2
#define ANT_DIVERSITY_RSSI_TH_2G					-65
#define ANT_DIVERSITY_RSSI_TH_5G					-65
#define ANT_DIVERSITY_STATUS_IDLE					0 /* default status */
#define ANT_DIVERSITY_STATUS_STREAM_START			1 /* begin to run T-put*/
#define ANT_DIVERSITY_STATUS_TP_COMPARE				2 /* need to compare T-put for both ANT_MAIN and ANT_AUX*/
#define ANT_DIVERSITY_STATUS_TP_RUNNING				3 /* T-put is running & stable */
#define RX_RATE_DELTA_TH_5G							30 /* delta threshold */
#define RX_RATE_DELTA_TH_2G							15 /* delta threshold */
#define ANT_DIVERSITY_CN_INIT_VAL					0x1ff
#define ANT_DIVERSITY_RX_RATE_INIT_VAL				0xffffffff
#define ANT_DIVERSITY_CN_DEGRADE_CNT_TH_2G			1
#define ANT_DIVERSITY_CN_DEGRADE_CNT_TH_5G			1
#define ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_2G		2
#define ANT_DIVERSITY_RX_RATE_DEGRADE_CNT_TH_5G		3
#define ANT_DIVERSITY_CN_DEGRADE_FALSE				0
#define ANT_DIVERSITY_CN_DEGRADE_TRUE				1
#define ANT_DIVERSITY_CN_DEGRADE_WAITING			2
#define ANT_DIVERSITY_RX_RATE_DEGRADE_FALSE			0
#define ANT_DIVERSITY_RX_RATE_DEGRADE_TRUE			1
#define ANT_DIVERSITY_RX_RATE_DEGRADE_WAITING		2
#define ANT_DIVERSITY_OPERATION_CYCLE				5 /* 500ms */

struct ant_diversity_ctl {
	UINT8   cur_ant;/* current antenna, 0-> main; 1->aux */
	UINT8   cur_traffic_mode;/* 0-> no traffic; 1->donwlink; 2->uplink */
	UINT8   last_traffic_mode;/* 0-> no traffic; 1->donwlink; 2->uplink */
	UINT32	cur_rx_tput[ANT_NUMBER];/* real time RX T-put for ANT_MAIN & ANT_AUX*/
	UINT32	cur_tx_tput[ANT_NUMBER];/* real time TX T-put for ANT_MAIN & ANT_AUX */
	UINT32	last_rx_tput[ANT_NUMBER];/* RX T-put of last time for ANT_MAIN & ANT_AUX*/
	UINT32	last_tx_tput[ANT_NUMBER];/* TX T-put of last time for ANT_MAIN & ANT_AUX */
	UINT32	cur_rx_cn[ANT_NUMBER];/* real time RX CN for ANT_MAIN & ANT_AUX */
	UINT32	cur_rx_rate[ANT_NUMBER];/* real time RX rate for ANT_MAIN & ANT_AUX */
	UINT32	last_rx_rate[ANT_NUMBER];/* Last time RX rate for ANT_MAIN & ANT_AUX */
	UINT32	last_rx_cn[ANT_NUMBER];/* RX CN of last time for ANT_MAIN & ANT_AUX*/
	UINT32	rx_delta_cn;/* real time RX CN between ANT_MAIN and ANT_AUX */
	UINT32	ul_mode_th;/* T-put threshold to detect ul mode */
	UINT32	dl_mode_th;/* T-put threshold to detect dl mode */
	UINT8	ap_nss;/* AP's TX/RX stream number */
	UINT8	sta_nss;/* STA's TX/RX stream number */
	CHAR	sta_rssi;/* STA's RSSI */
	UINT16	client_num;/* connected STA numbers */
	BOOLEAN	is_he_sta;/* 0-> sta not support 11ax; 1-> sta support 11ax*/
	UINT8	ant_switch_countdown;/* Delay some time to switch antenna */
	UINT8	diversity_status;/* STA's TX/RX stream number */
	UINT8	cn_deg_cnt;/* continuous CN degrade count */
	UINT8	rx_rate_deg_cnt;/* continuous Rx rate degrade count */
	UINT32  rxv_cr_value;/* backup rxv_enable CR value*/
	BOOLEAN	diversity_force_disable;/* 0-> enable diversity alg; 1-> disable diversity alg*/
	UINT8	dbg_cn_deg_cnt_th;/* for dbg */
	UINT8	dbg_rx_rate_deg_cnt_th;/* for dbg */
	UINT8	dbg_tp_deg_th;/* for dbg */
	UINT8	dbg_cn_deg_th_max;/* for dbg */
	UINT8	dbg_cn_deg_th_min;/* for dbg */
	UINT32	dbg_rx_rate_deg_th;/* for dbg */
	UINT8	dbg_countdown;/* for dbg */
	UINT16	dbg_ul_tp_th;/* for dbg */
	UINT32	dbg_ul_rate_delta_th;/* for dbg */
	BOOLEAN	dbg_flag_lvl1;/* for dbg */
	BOOLEAN	dbg_flag_lvl2;/* for dbg */
};
#endif

struct txop_ctl {
	UINT16 multi_client_nums;
	UINT16 multi_rx_client_nums;
	UINT16 last_client_num;
	UINT16 last_rx_client_num;
	UINT16 multi_tcp_nums;
	UINT16 last_tcp_nums;
	struct wifi_dev *cur_wdev;
	BOOLEAN multi_cli_txop_running;
	BOOLEAN near_far_txop_running;
#ifdef DELAY_TCP_ACK_V2
	UINT8 peak_tcp_running_aifsn_setting;
#endif /* DELAY_TCP_ACK_V2 */
#ifdef RX_COUNT_DETECT
	BOOLEAN limit_ampdu_size_running;
#endif /* RX_COUNT_DETECT */
};

#define MCLI_DEBUG_PER_BAND     (1 << 0)
#define MCLI_DEBUG_PER_STA      (1 << 1)
#define MCLI_DEBUG_IXIA_MODE    (1 << 2)
#define MCLI_DEBUG_SINGLE_PAIR  (1 << 3)
#define MCLI_DEBUG_RPS_CFG_MODE (1 << 4)
#define MCLI_DEBUG_RMAC_DROP    (1 << 5)
#define MCLI_DEBUG_MULTICLIENT  (1 << 6)

struct ixia_mode_ctl_type {
	BOOLEAN mode_entered;
	UINT8 band;
	UINT16 sta_nums;
	UINT32 num_entry;
#ifdef KERNEL_RPS_ADJUST
	UINT32 rps_mask;
	UINT32 max_mgmt_que_num_backup;
	UINT32 max_data_que_num_backup;
	BOOLEAN tx_tasklet_sch;
	BOOLEAN rx_tasklet_sch;
#endif
	BOOLEAN kernel_rps_en;
	BOOLEAN fgProbeRspDetect;
	BOOLEAN fgSkipRedQLenDrop;
	BOOLEAN isIxiaOn;
};

struct multi_cli_ctl {
	UINT32 pkt_avg_len;
	UINT16 sta_nums;
	UINT32 pkt_rx_avg_len;
	UINT16 rx_sta_nums;
	UCHAR amsdu_cnt;
	UINT32 rts_length;
	UINT32 rts_pkt;
	BOOLEAN c2s_only;
	ULONG last_tx_cnt;
	ULONG last_tx_fail_cnt;
	ULONG tot_tx_cnt;
	ULONG tot_tx_fail_cnt;
	ULONG tot_tx_pkts;
	ULONG tot_rx_pkts;
	BOOLEAN tx_cnt_from_red;
	UINT32 RxFIFONotEnoughCnt;
	UINT32 RxPPDUDropCnt;
	UINT32 debug_on;
	BOOLEAN adjust_backoff;
	EDCA_PARM edcaparam_backup;
	UINT32 last_cca_abort_val;
	UINT32 cca_abort_cnt;
	BOOLEAN is_bidir;
	UINT16 large_rssi_gap_num;
	UINT16 last_large_rssi_gap_num;
#ifdef KERNEL_RPS_ADJUST
	BOOLEAN rps_apply;
	BOOLEAN rps_adjust;
	BOOLEAN	kernel_rps_adjust_enable;
	UINT8	force_agglimit;
	UINT8	cur_agglimit;
	UINT8	cur_txop;
	BOOLEAN force_rps_cfg;
	UINT32  force_tx_process_cnt;
	UINT32  proc_rps_mode;
	UINT32  rps_state_flag;
	BOOLEAN iMacflag;
	UINT32 peak_tx_clients;
	UINT32 peak_rx_clients;
	UINT32 peak_tx_pkts;
	UINT32 peak_rx_pkts;
	UINT32 peer_req_cnt;
	UINT32 silent_period_cnt;
	char force_proc_rps[MAX_PROC_RPS_FILE][8];
#endif
};
#ifdef PKTLOSS_CHK
#define MAX_PKTLOSS_CHK_PT 5
#define MAX_PKTLOSS_CHK_LOG 5
typedef struct pktloss_check_var_s{
	UINT_32 seq[MAX_PKTLOSS_CHK_PT];
	UINT_32 seq_mask;
	UINT_32 loss[MAX_PKTLOSS_CHK_PT];
	UINT_32 loss_cnt[MAX_PKTLOSS_CHK_PT];
	UINT_32 tot_cnt[MAX_PKTLOSS_CHK_PT];
	UINT_32 tot_drop_cnt[MAX_PKTLOSS_CHK_PT];
	UINT_32 loss_seq_idx[MAX_PKTLOSS_CHK_PT];
	UINT_32 loss_seq[MAX_PKTLOSS_CHK_PT][MAX_PKTLOSS_CHK_LOG];
	UINT_32 dup_seq_idx[MAX_PKTLOSS_CHK_PT];
	UINT_32 dup_seq[MAX_PKTLOSS_CHK_PT][MAX_PKTLOSS_CHK_LOG];
	UINT_32 src_ip_mask;
	UINT_32 dest_ip_mask;
	UINT_32 dest_port;
	UINT_32 ts[MAX_PKTLOSS_CHK_PT];
	UINT_32 ts_delay[MAX_PKTLOSS_CHK_PT];
	UINT_32 ts_threshold;
	UINT_32 max_ts_delay[MAX_PKTLOSS_CHK_PT];
	UINT_16 proto;
	UINT_32 byte_offset;
	UINT_32 ctrl_flag;
	UINT_8 is_seq_signed;
	UINT_8 is_seq_cross_zero;
	BOOLEAN  (*pktloss_chk_handler)(PRTMP_ADAPTER pAd,
					PUINT_8 aucPktContent,
					UINT_8 ip_offset,
					UINT_8 check_point,
					BOOLEAN drop);
	BOOLEAN txs_log_enable;
	BOOLEAN enable;
} pktloss_check_var_type;
#endif
typedef struct _RxVBQElmt {
	UINT8 valid;
	UINT8 isEnd;
	UINT8 rxblk_setting_done;
	UINT8 rxv_setting_done;
	UINT8 rxblk_search_done;
	UINT8 rxv_search_done;
	UINT8 rxv_sn;
	UINT16 wcid;
	UINT8 aggcnt;
	UINT8 g0_debug_set;
	UINT8 g1_debug_set;
	UINT8 g2_debug_set;
	UCHAR chipid[5];
	UINT32 MPDUbytecnt;
	UINT32 timestamp;
	UINT32 arFCScheckBitmap[8];
	UINT32 RXV_CYCLE[9];
} RxVBQElmt, *PRxVBQElmt;

#ifdef WAPP_SUPPORT
struct scan_req {
	INT32 scan_id;
	INT32 last_bss_cnt;
	INT32 if_index;
	UINT32 bss_nr;
};
#endif

#ifdef ZERO_LOSS_CSA_SUPPORT
typedef struct _ZERO_LOSS_STA {
	UINT8	StaAddr[MAC_ADDR_LEN];
	UINT16	wcid;
	UINT8	band;
	UINT8	valid;
	BOOLEAN ChnlSwitchSkipTx;
	ULONG suspend_time;
	ULONG resume_time;
} ZERO_LOSS_STA, *PZERO_LOSS_STA;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef CONFIG_CPE_SUPPORT
struct lppe_scan_req {
	INT32 scan_id;
	INT32 last_bss_cnt;
	INT32 if_index;
};

typedef enum {
	LPPE_SCAN_COMPLETE_NOTIF,
	LPPE_SCAN_RESULT_RSP,
} LPPE_EVENT_ID;

typedef enum {
	LPPE_GET_SCAN_RESULTS,
} LPPE_REQ_ID;
#endif

#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT) || defined(OFFCHANNEL_ZERO_LOSS)
typedef struct _CHANNEL_STATS {
		UINT32		MibUpdateOBSSAirtime[2];
		UINT32		MibUpdateMyTxAirtime[2];
		UINT32		MibUpdateMyRxAirtime[2];
		UINT32		TotalDuration;
		UINT32		MeasurementDuration;
		UINT32		LastReadTime;
#ifdef TXRX_STAT_SUPPORT
		UINT32 		Radio100msecCounts;
#endif
/* Above Variables are getting used for 7615,
   Below variables will be used for Stats in 7915/7986*/
#ifdef CCAPI_API_SUPPORT
		UINT64		OBSSAirtime[DBDC_BAND_NUM];
		UINT64		MyTxAirtime[DBDC_BAND_NUM];
		UINT64		MyRxAirtime[DBDC_BAND_NUM];
		UINT64		EDCCAtime[DBDC_BAND_NUM];
		UINT64		TxOpInitTime[DBDC_BAND_NUM];
		UINT64		SampleDuration[DBDC_BAND_NUM];
		UINT64		PrevReadTime[DBDC_BAND_NUM];
#endif
#ifdef OFFCHANNEL_ZERO_LOSS
	MT_MIB_COUNTER_STAT		PrevPeriodicStatStore;	/*store stat value of previous onchannel periodic read*/
	MT_MIB_COUNTER_STAT     CurrentPeriodicChStat;	/*channel stat for current on channel periodic read*/
	MT_MIB_COUNTER_STAT     ScanStartChnlStats;	/*store chnl stats at scan start*/
	MT_MIB_COUNTER_STAT     ScanStopChnlStats;	/*store chnl stats at scan stop*/
#endif
} CHANNEL_STATS, *PCHANNEL_STATS;
#endif

#define VOW_GROUP_TABLE_MAX	(EXTEND_MBSS_MAC_MAX + HW_BSSID_MAX)

struct bss_group_rec {
	UINT8 group_idx[VOW_GROUP_TABLE_MAX];
	UINT8 bw_group_idx[VOW_GROUP_TABLE_MAX];
};

#ifdef RATE_PRIOR_SUPPORT
typedef struct _LOWRATE_CTRL{
	BOOLEAN RatePrior;
	UINT LowRateRatioThreshold;
	UINT LowRateCountPeriod;
	UINT TotalCntThreshold;
	DL_LIST BlackList;
	UINT BlackListTimeout;
	NDIS_SPIN_LOCK BlackListLock;
} LOWRATE_CTRL, *PLOWRATE_CTRL;
typedef struct _BLACK_STA{
	UCHAR Addr[MAC_ADDR_LEN];
	DL_LIST List;
	ULONG Jiff;
} BLACK_STA, *PBLACK_STA;
#endif /*RATE_PRIOR_SUPPORT*/

/* For wifi md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
typedef struct _COEX_APCCCI2FW_CMD {
	struct notifier_block coex_apccci2fw_notifier;
	VOID *priv;
} COEX_APCCCI2FW_CMD, *PCOEX_APCCCI2FW_CMD;

#ifdef COEX_DIRECT_PATH
typedef struct _COEX_3WIRE_GRP_CMD {
	unsigned char Triband;
	unsigned char CardType;
	unsigned char WiFiOperFreq3WireGroup;
} COEX_3WIRE_GRP_CMD, *P_COEX_3WIRE_GRP_CMD;
#endif
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
#define CSI_MAX_CHAIN_NUM 16		/*for 4x4 support*/
#define CSI_RING_SIZE 4096
#define CSI_MAX_DATA_COUNT 256
#define CSI_MAX_RSVD1_COUNT 10
#define CSI_MAX_RSVD2_COUNT 10
#define Max_Stream_Bytes 3000
#define Max_Sta_Mac_Num 20	/*max filter station number*/
#define CSI_MAX_PKT_BYTES 1500
#define CSI_MAX_TS_OFFSET (10*1000*1000)	/*10s*/
#define CSI_TS_FILTER_RATIO 10	/*default use usr's offsetT * 1/10 to filter*/

/*csi report mode*/
#define CSI_NETLINK	1
#define CSI_PROC	0

/*csi netlink related*/
#define CSI_GENL_NAME "csi_genl"

enum {
    CSI_OPS_UNSPEC,
 	CSI_OPS_REPORT,
    CSI_OPS_MAX,
};

enum CSI_NL_ATTR {
	CSI_ATTR_UNSPEC = 0,
	CSI_ATTR_MAGIC_NUMBER,
	CSI_ATTR_VER,
	CSI_ATTR_TYPE,
	CSI_ATTR_TS,
	CSI_ATTR_RSSI,
	CSI_ATTR_SNR,
	CSI_ATTR_DBW,
	CSI_ATTR_CH_IDX,
	CSI_ATTR_TA,
	CSI_ATTR_I,
	CSI_ATTR_Q,
	CSI_ATTR_EXTRA_INFO,
	CSI_ATTR_TX_IDX,
	CSI_ATTR_RX_IDX,
	CSI_ATTR_FRAME_MODE,
	CSI_ATTR_H_IDX,

	CSI_ATTR_REPORT_MSG,
	CSI_ATTR_DATA_HEADER,
	CSI_ATTR_CHAIN_HEADER,
	__CSI_ATTR_MAX,
};
#define CSI_ATTR_MAX (__CSI_ATTR_MAX - 1)

/*antenna tiem parse*/
#define PARSE_CHAIN_IDX(item) (item & 0X000000ff)
#define PARSE_CSI_SEQ_NUM(item) ((item & 0Xffff0000) >> 16)
#define PARSE_CHAIN_END_FLAG(item) (item & BIT(15))

/*
 * CSI_DATA_T is used for representing
 * the CSI and other useful * information
 * for application usage
 */
struct CSI_DATA_T {
	UINT_8 ucBw;
	UINT_8 FWVer;
	UINT_16 u2DataCount;
	INT_16 ac2IData[CSI_MAX_DATA_COUNT];
	INT_16 ac2QData[CSI_MAX_DATA_COUNT];
	UINT_8 ucDbdcIdx;
	INT_8 cRssi;
	UINT_8 ucSNR;
	UINT_32 u4TimeStamp;
	UINT_8 ucDataBw;
	UINT_8 ucPrimaryChIdx;
	UINT_8 aucTA[MAC_ADDR_LEN];
	UINT_32 u4ExtraInfo;
	UINT_8 ucRxMode;
	INT_32 ai4Rsvd1[CSI_MAX_RSVD1_COUNT];
	INT_32 au4Rsvd2[CSI_MAX_RSVD2_COUNT];
	UINT_8 ucRsvd1Cnt;
	UINT_8 ucRsvd2Cnt;
	INT_32 i4Rsvd3;
	UINT_8 ucRsvd4;
	UINT_32 Antenna_pattern;	/*example: 4T 4R  the low 2 byte para will be 0~15*/
	UINT_32 Tx_Rx_Idx;		/*show tx stream and rx stream idx of the packet*/
};

/*
 * CSI_INFO_T is used to store the CSI
 * settings and CSI event data
 */
struct CSI_INFO_T {
	/* Variables for manipulate the CSI data in g_aucProcBuf */
	BOOLEAN bIncomplete;
	INT_32 u4CopiedDataSize;
	INT_32 u4RemainingDataSize;
	wait_queue_head_t waitq;
	/* Variable for recording the CSI function config */
	UINT_8 ucMode;
	UINT_8 ucValue1[CSI_CONFIG_ITEM_NUM];
	UINT_8 ucValue2[CSI_CONFIG_ITEM_NUM];
	/* Variable for manipulating the CSI ring buffer */
	struct CSI_DATA_T arCSIBuffer[CSI_RING_SIZE];
	UINT_32 u4CSIBufferHead;
	UINT_32 u4CSIBufferTail;
	UINT_32 u4CSIBufferUsed;
	INT_16 ai2TempIData[CSI_MAX_DATA_COUNT];
	INT_16 ai2TempQData[CSI_MAX_DATA_COUNT];
	NDIS_SPIN_LOCK CSIBufferLock;
	UINT_8 byte_stream[Max_Stream_Bytes];	/*send bytes to proc interfacel */
	UINT_8 *sta_mac_addr[MAC_ADDR_LEN];	/*station mac addr for filter*/
	UINT_8 sta_sel_cnt;	/*sta mac addr specified count*/
	DL_LIST CSIStaList;
	NDIS_SPIN_LOCK CSIStaListLock;
	UINT_8 CSI_report_mode;
	struct genl_family *csi_genl_family;
	struct genl_ops *csi_genl_ops;
	struct nla_policy *csi_genl_policy;
	struct sk_buff *pnl_skb;	/*pointer to csi netlink skb*/
	UINT_32 nl_seq_idx;
	UINT_32 usr_offset;	/*to filter data with specific TS => unit:us*/
	UINT_32 cur_TS;	/*store current ts in the pkt*/
	struct CSI_DATA_T TS_filter_pkt[CSI_MAX_CHAIN_NUM];
	UINT_32 ExpTs_offset;
};

typedef struct _CSI_STA {
	UCHAR Addr[MAC_ADDR_LEN];
	DL_LIST List;
} CSI_STA, *PCSI_STA;

typedef struct TLV_ELEMENT {
	UINT_32 tag_type;
	UINT_32 body_len;
	UINT_8 aucbody[0];
} TLV_ELEMENT_T, *PTLV_ELEMENT_T;

enum CSI_DATA_TLV_TAG {
	CSI_DATA_VER,
	CSI_DATA_TYPE,
	CSI_DATA_TS,
	CSI_DATA_RSSI,
	CSI_DATA_SNR,
	CSI_DATA_DBW,
	CSI_DATA_CH_IDX,
	CSI_DATA_TA,
	CSI_DATA_I,
	CSI_DATA_Q,
	CSI_DATA_EXTRA_INFO,
	CSI_DATA_RSVD1,
	CSI_DATA_RSVD2,
	CSI_DATA_RSVD3,
	CSI_DATA_RSVD4,
	CSI_DATA_TX_IDX,
	CSI_DATA_RX_IDX,
	CSI_DATA_FRAME_MODE,
	CSI_DATA_H_IDX,
	CSI_DATA_TLV_TAG_NUM,
};

enum ENUM_CSI_MODULATION_BW_TYPE_T {
	CSI_TYPE_OFDM_BW20,
	CSI_TYPE_OFDM_BW40,
	CSI_TYPE_OFDM_BW80
};

#define RX_VT_LEGACY_CCK      0
#define RX_VT_LEGACY_OFDM     1
#define RX_VT_MIXED_MODE      2
#define RX_VT_GREEN_MODE      3
#define RX_VT_VHT_MODE        4
#define RX_VT_HE_MODE         8

#define RX_VT_FR_MODE_20      0
#define RX_VT_FR_MODE_40      1
#define RX_VT_FR_MODE_80      2
#define RX_VT_FR_MODE_160     3 /*BW160 or BW80+80*/

int make_csi_nlmsg_fragment(RTMP_ADAPTER *pAd); /*TBD*/
int make_csi_nlmsg_complete(RTMP_ADAPTER *pAd);
int csi_genl_recv_doit(struct sk_buff *skb, struct genl_info *info);
VOID csi_support_init(RTMP_ADAPTER *pAd);
VOID csi_support_deinit(RTMP_ADAPTER *pAd);
int csi_proc_init(RTMP_ADAPTER *pAd);
int csi_proc_deinit(RTMP_ADAPTER *pAd);
bool wlanPushCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData);
bool wlanPopCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData);
INT ProbeAndPopCompleteCSIData(RTMP_ADAPTER *pAd);
VOID wlanApplyCSIToneMask(
	UINT_8 ucRxMode,
	UINT_8 ucCBW,
	UINT_8 ucDBW,
	UINT_8 ucPrimaryChIdx,
	INT_16 *ai2IData,
	INT_16 *ai2QData);

VOID wlanShiftCSI(
	UINT_8 ucRxMode,
	UINT_8 ucCBW,
	UINT_8 ucDBW,
	UINT_8 ucPrimaryChIdx,
	INT_16 *ai2IData,
	INT_16 *ai2QData,
	INT_16 *ai2ShiftIData,
	INT_16 *ai2ShiftQData);
#endif /* CFG_SUPPORT_CSI*/

typedef struct _PHY_STAT_ELEM {
	/* raw data */
	UINT32 rx_raw;
	UINT32 rx_raw2;
	/* contention-based rx stat */
	UINT8 rx_rate;
	UINT8 rx_mode;
	UINT8 rx_nsts;
} PHY_STAT_ELEM, *P_PHY_STAT_ELEM;

typedef struct _RXV_RAW_DATA {
	UINT16 rxv_byte_cnt;
	PVOID rxv_pkt;
} RXV_RAW_DATA, *P_RXV_RAW_DATA;

typedef enum _RXV_DUMP_CTRL_ACTION {
	RXV_DUMP_START = 0,
	RXV_DUMP_STOP,
	RXV_DUMP_ALLOC,
	RXV_DUMP_CLEAR,
	RXV_DUMP_REPORT,
	RXV_DUMP_ACTION_NUM
} RXV_DUMP_CTRL_ACTION, P_RXV_DUMP_CTRL_ACTION;

typedef struct _RXV_DUMP_ENTRY_CONTENT {
	DL_LIST list;
	UINT8 user_idx;
	UINT16 len;
	VOID *content;
} RXV_DUMP_ENTRY_CONTENT, *P_RXV_DUMP_ENTRY_CONTENT;

typedef struct _RXV_DUMP_BASIC_ENTRY {
	DL_LIST list;
	UINT8 type_idx;
	UINT16 len;
	UINT8 usr_num;
	DL_LIST *data_list;
} RXV_DUMP_BASIC_ENTRY, *P_RXV_DUMP_BASIC_ENTRY;

typedef struct _RXV_DUMP_ENTRY {
	DL_LIST list;
	UINT8 entry_idx;
	UINT8 type_num;
	DL_LIST *rxv_dump_basic_entry_list;
} RXV_DUMP_ENTRY, *P_RXV_DUMP_ENTRY;

typedef struct _RXV_DUMP_CTRL {
	BOOLEAN enable;
	BOOLEAN alloc;
	UINT8 type_mask;
	UINT8 type_num;
	UINT8 ring_idx;
	UINT8 dump_entry_total_num;
	UINT8 valid_entry_num;
	DL_LIST *rxv_dump_entry_list;
} RXV_DUMP_CTRL, *P_RXV_DUMP_CTRL;

typedef struct _RXV_PKT_HDR {
	UINT16 rx_byte_cnt;
	UINT8 rxv_cnt;
	UINT8 pkt_type;
	UINT16 pse_fid;
} RXV_PKT_HDR, *P_RXV_PKT_HDR;

typedef struct _RXV_ENTRY_HDR {
	UINT16 rx_sta_airtime;
	UINT8 rx_sta_cnt;
	UINT32 time_stamp;
	UINT8 rxv_sn;
	UINT8 band_idx;
	BOOLEAN rx_airtime_cal;
	UINT8 tr;
	UINT8 trig_mpdu;
} RXV_ENTRY_HDR, *P_RXV_ENTRY_HDR;

/* fwcmd timeout print cnt */
#define FW_CMD_TO_DBG_INFO_PRINT_CNT	5
#define FW_CMD_TO_PRINT_CNT				20

/* fwcmd timeout record cnt */
#define FW_CMD_TO_RECORD_CNT			20

typedef struct _FWCMD_TIMEOUT_RECORD {
	ULONG timestamp;
	UINT8 type;
	UINT8 ext_type;
	UINT8 seq;
	UINT8 state;
#ifdef WF_RESET_SUPPORT
	UINT8 cidx;
	UINT8 didx;
#endif

} FWCMD_TIMEOUT_RECORD, *P_FWCMD_TIMEOUT_RECORD;

#define CH_OP_MAX_HOLD_TIME 60000 /* 60 secs */
#define CH_OP_MAX_TRY_COUNT 3 /* 3 times */

typedef enum _ENUM_CH_OP_OWNER_T {
	CH_OP_OWNER_IDLE = 0,
	CH_OP_OWNER_LTE_SAFE_CHN = 1,
	CH_OP_OWNER_PARTIAL_SCAN = 2,
	CH_OP_OWNER_ACS = 3,
	CH_OP_OWNER_SET_CHN = 4,
	CH_OP_OWNER_SCAN = 5,
	CH_OP_OWNER_PEER_CSA = 6,
	CH_OP_OWNER_DFS = 7
} ENUM_CH_OP_OWNER_T;

typedef struct _CHANNEL_OP_CTRL {
	NDIS_SPIN_LOCK ChOpLock;	/* protect ChOpOwnerBitMask read/write */
	RTMP_OS_COMPLETION	ChOpDone;	/* Whether ChOp is done by charge owner, used to notify other waiters */
	RALINK_TIMER_STRUCT ChOpTimer;	/* Force release ChOpCharge when timeout */
	BOOLEAN ChOpTimerRunning;	/* Indicate ChOpTimer is running or not */
	UINT_32	ChOpOwnerBitMask;	/* all 0: IDLE, bit value 1: is in operating by bit owner (ENUM_CH_OP_OWNER_T)  */
	UINT_32	ChOpWaitBitMask;	/* all 0: IDLE, bit value 1: bit owner (ENUM_CH_OP_OWNER_T) is waiting for OpCharge   */
	TIMER_FUNC_CONTEXT ChOpTimerFuncContex;
} CHANNEL_OP_CTRL, *P_CHANNEL_OP_CTRL;

typedef enum _ENUM_WDEV_LOCK_OP_T {
	WDEV_LOCK_OP_NONE = 0x0,
	WDEV_LOCK_OP_LINK_UP_DOWN = 0x1,
	WDEV_LOCK_OP_OPEN_CLOSE = 0x2,
	WDEV_LOCK_OP_CONN_DISCONN = 0x4,
	WDEV_LOCK_OP_ALL = 0x7,
} ENUM_WDEV_LOCK_OP_T;

struct wo_rx_total_cnt {
	UINT64 rx_pkt_cnt;
	UINT64 rx_byte_cnt;
	UINT64 rx_err_cnt;
	UINT64 rx_drop_cnt;
};

/*
	The miniport adapter structure
*/
struct _RTMP_ADAPTER {
	UINT8 PeerApccfs1;
	VOID *OS_Cookie;	/* save specific structure relative to OS */
	PNET_DEV net_dev;
	NDIS_SPIN_LOCK WdevListLock;
	struct wifi_dev *wdev_list[WDEV_NUM_MAX];
#ifdef WF_RESET_SUPPORT
	struct wifi_dev wdev_list_backup[WDEV_NUM_MAX];
#endif
	struct tx_rx_ctl tr_ctl;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	UINT32 irq_num[8];
	UINT32 irq_num_pcie1[8];
#endif
#ifdef WARP_512_SUPPORT
	BOOLEAN Warp512Support;
#endif

	/*About MacTab, the sta driver will use #0 and #1 for multicast and AP. */
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */
	NDIS_SPIN_LOCK MacTabLock;

#ifdef WTBL_TDD_SUPPORT
	WTBL_TDD_INFO wtblTddInfo;
#endif /* WTBL_TDD_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
	BOOLEAN bSw_sta; /* global flag for on off feature */
#endif /* SW_CONNECT_SUPPORT */
#ifdef TR181_SUPPORT
	UINT32 ApBootACSChannelChangePerBandCount[DBDC_BAND_NUM];
#endif /*TR181_SUPPORT*/

	BOOLEAN IoctlHandleFlag;	/* Indicate Ioctl is in processing */

	UINT8	ucBFBackOffMode;
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
	CHANNEL_BUSY_TIME Ch_Stats[DBDC_BAND_NUM];
	UINT32 Ch_BusyTime[DBDC_BAND_NUM];
	CHANNEL_STATS	ChannelStats;
#endif
#ifdef TXRX_STAT_SUPPORT
	UINT32 Ch_BusyTime_11k[DBDC_BAND_NUM];
	BOOLEAN TXRX_EnableReadRssi;
	BOOLEAN EnableTxRxStats;
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
	SORTED_CHANNEL_LIST  sorted_list;
	UINT8 last_selected_channel;
	BOOLEAN radar_hit;
#endif
#ifdef SCAN_RADAR_COEX_SUPPORT
	struct wifi_dev *scan_wdev;
	BOOLEAN radar_handling;
	RTMP_OS_TASK radar_task;
	struct _EXT_EVENT_RDD_REPORT_T rddReport;
#endif /* SCAN_RADAR_COEX_SUPPORT */
	UINT8 oper_ch;

#ifdef ZERO_LOSS_CSA_SUPPORT
	BOOLEAN Zero_Loss_Enable;
	BOOLEAN Csa_Action_Frame_Enable;
	ULONG chan_switch_time[17];
	UINT32 ucSTATimeout;
	UINT32 ZeroLossStaPsQLimit;
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef OFFCHANNEL_ZERO_LOSS
	CHANNEL_BUSY_TIME ScanChnlStats[DBDC_BAND_NUM];
	UINT8 OnChannelScanOngoing[DBDC_BAND_NUM];
#endif
	BOOLEAN fgQAEffuseWriteBack;

	UINT8 MuHwSwPatch;

	PHY_STAT_ELEM phy_stat_elem;
#ifdef LINK_TEST_SUPPORT
	/* state machine state flag */
	UINT8 ucLinkBwState[BAND_NUM];
	UINT8 ucRxStreamState[BAND_NUM];
	UINT8 ucRxStreamStatePrev[BAND_NUM];
	UINT8 ucRxFilterstate[BAND_NUM];
	UINT8 ucTxCsdState[BAND_NUM];
	UINT8 ucTxPwrBoostState[BAND_NUM];
	UINT8 ucLinkRcpiState[BAND_NUM];
	UINT8 ucLinkSpeState[BAND_NUM];
    UINT8 ucLinkSpeStatePrev[BAND_NUM];
    UINT8 ucLinkBwStatePrev[BAND_NUM];
    UINT8 ucTxCsdStatePrev[BAND_NUM];

	/* BW Control Paramter */
	UINT8   ucPrimChannel[BAND_NUM];
	UINT8	ucCentralChannel[BAND_NUM];
	UINT8	ucCentralChannel2[BAND_NUM];
	UINT8	ucExtendChannel[BAND_NUM];
	UINT8	ucHtBw[BAND_NUM];
	UINT8	ucVhtBw[BAND_NUM];
	BOOLEAN fgBwInfoUpdate[BAND_NUM];

	/* Rx Control parameter */
	UINT8	ucRxTestTimeoutCount;
	INT64	c8TempRxCount;
	UINT8	ucRssiTh;
	UINT8	ucRssiSigniTh;
	INT64	c8RxCountTh;
	UINT8	ucTimeOutTh;
	UINT8	ucPerTh;
	CHAR	cNrRssiTh;
	CHAR	cChgTestPathTh;
	UINT8	u1RxSenCount[BAND_NUM];
	UINT8	ucRxSenCountTh;
	UINT8	u1SpeRssiIdxPrev[BAND_NUM];
	CHAR	cWBRssiTh;
	CHAR	cIBRssiTh;
	UINT8	u1RxStreamSwitchReason[BAND_NUM];
    UINT8	ucRxSenCount[BAND_NUM];

	/* ACR Control Parameter */
	UINT8	ucRxFilterConfidenceCnt[BAND_NUM];
	UINT8	ucACRConfidenceCntTh;
	UINT8	ucMaxInConfidenceCntTh;
	CHAR	cMaxInRssiTh;

	/* Tx Control Parameter */
	UINT8	ucCmwCheckCount;
	UINT8	ucCmwCheckCountTh;
	BOOLEAN	fgCmwInstrumBack4T;
	UINT8	ucRssiBalanceCount;
	UINT8	ucRssiIBalanceCountTh;
	BOOLEAN	fgRssiBack4T;
	UINT8	ucCableRssiTh;
	BOOLEAN	fgCmwLinkDone;
	BOOLEAN	fgApclientLinkUp;
	UINT8	ucLinkCount;
	UINT8	ucLinkCountTh;
	BOOLEAN	fgLinkRSSICheck;
	UINT8	ucCmwChannelBand[BAND_NUM];

	/* channel band Control Paramter */
	BOOLEAN fgChannelBandInfoUpdate[BAND_NUM];

	/* Tx Power Control Paramter */
	UINT8	ucTxPwrUpTbl[CMW_POWER_UP_CATEGORY_NUM][CMW_POWER_UP_RATE_NUM];

	/* manual command control function enable/disable flag */
	BOOLEAN fgTxSpeEn;
	BOOLEAN fgRxRcpiEn;
	BOOLEAN fgTxSpurEn;
	BOOLEAN fgRxSensitEn;
	BOOLEAN fgACREn;
#endif /* LINK_TEST_SUPPORT */

#ifdef	ETSI_RX_BLOCKER_SUPPORT
	/* Set fix WBRSSI/IBRSSI pattern */
	BOOLEAN	 fgFixWbIBRssiEn;

	CHAR	 c1WbRssiWF0;
	CHAR	 c1WbRssiWF1;
	CHAR	 c1WbRssiWF2;
	CHAR	 c1WbRssiWF3;
	CHAR	 c1IbRssiWF0;
	CHAR	 c1IbRssiWF1;
	CHAR	 c1IbRssiWF2;
	CHAR	 c1IbRssiWF3;

	/* set RSSI threshold */
	CHAR	 c1RWbRssiHTh;
	CHAR	 c1RWbRssiLTh;
	CHAR	 c1RIbRssiLTh;
	CHAR	 c1WBRssiTh4R;
	/* set RX BLOCKER check para. */
	UINT8	 u1CheckTime;
	UINT8	 u1To1RCheckCnt;
	UINT16	 u2To1RvaildCntTH;
	UINT16   u2To4RvaildCntTH;

	/* for verification */
	UINT8	 u1RxBlockerState;
	UINT8	 u1ValidCnt;
	UINT8	u14RValidCnt;

	UINT8	i1MaxWRssiIdxPrev;
	UINT8	 u1TimeCnt;
	BOOLEAN  fgAdaptRxBlock;
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

	BOOLEAN fgEPA;

	UINT8 TxPowerSKU[SKU_SIZE];
#if defined(MT7915) || defined(AXE) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	UINT8 u1SkuParamLen[SINGLE_SKU_TYPE_PARSE_NUM_V1];
	UINT8 u1SkuParamTransOffset[SINGLE_SKU_TYPE_NUM_V1];
	UINT8 u1SkuChBandNeedParse[SINGLE_SKU_TYPE_PARSE_NUM_V1];
	UINT8 u1SkuFillParamLen[SINGLE_SKU_TYPE_NUM_V1];
	UINT8 u1BackoffParamLen[BACKOFF_TYPE_PARSE_NUM_V1];
	UINT8 u1BackoffParamTransOffset[BACKOFF_TYPE_NUM_V1];
	UINT8 u1BackoffChBandNeedParse[BACKOFF_TYPE_PARSE_NUM_V1];
	UINT8 u1BackoffFillParamLen[BACKOFF_TYPE_NUM_V1];
#else
	UINT8 u1SkuParamLen[SINGLE_SKU_TYPE_PARSE_NUM_V0];
	UINT8 u1SkuParamTransOffset[SINGLE_SKU_TYPE_NUM_V0];
	UINT8 u1SkuChBandNeedParse[SINGLE_SKU_TYPE_PARSE_NUM_V0];
	UINT8 u1SkuFillParamLen[SINGLE_SKU_TYPE_NUM_V0];
	UINT8 u1BackoffParamLen[BACKOFF_TYPE_PARSE_NUM_V0];
	UINT8 u1BackoffParamTransOffset[BACKOFF_TYPE_NUM_V0];
	UINT8 u1BackoffChBandNeedParse[BACKOFF_TYPE_PARSE_NUM_V0];
	UINT8 u1BackoffFillParamLen[BACKOFF_TYPE_NUM_V0];
#endif /* defined(MT7615) || defined(MT7622) */
	UINT32 wrong_wlan_idx_num;
	VOID *qm;
	QUEUE_HEADER fp_que[2];
	QUEUE_HEADER fp_post_que[2];
	NDIS_SPIN_LOCK fp_que_lock[2];
	NDIS_SPIN_LOCK fp_post_que_lock[2];
	NDIS_SPIN_LOCK mgmt_que_lock[2];
	NDIS_SPIN_LOCK mgmt_post_que_lock[2];
	QUEUE_HEADER mgmt_que[2];
	QUEUE_HEADER mgmt_post_que[2];
	struct fp_tx_flow_control fp_tx_flow_ctl;
	NDIS_SPIN_LOCK high_prio_que_lock;
	QUEUE_HEADER high_prio_que;
	BOOLEAN tx_dequeue_scheduable[2];
	RTMP_NET_TASK_STRUCT tx_deque_tasklet[2];
	BOOLEAN rx_dequeue_sw_rps_enable;
#ifdef RX_RPS_SUPPORT
	RTMP_NET_TASK_STRUCT rx_deque_tasklet[NR_CPUS];
	NDIS_SPIN_LOCK rx_que_lock[NR_CPUS];
	QUEUE_HEADER rx_que[NR_CPUS];
	QUEUE_HEADER rx_post_que[NR_CPUS];
#else
	RTMP_NET_TASK_STRUCT rx_deque_tasklet;
	NDIS_SPIN_LOCK rx_que_lock;
	QUEUE_HEADER rx_que;
	QUEUE_HEADER rx_post_que;
#endif

#ifdef DBG_AMSDU
	UINT32 dbg_time_slot;
	RALINK_TIMER_STRUCT amsdu_history_timer;
#endif
	UCHAR amsdu_max_num;
	NDIS_SPIN_LOCK irq_lock;

	/*======Cmd Thread in PCI/RBUS/USB */
	CmdQ CmdQ;
	NDIS_SPIN_LOCK CmdQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK cmdQTask;
	HW_CTRL_T HwCtrl;
	RTMP_OS_SEM AutoRateLock;

#if defined(CONFIG_AP_SUPPORT) && defined(AP_SCAN_SUPPORT) && defined(OFFCHANNEL_SCAN_FEATURE)
	CHANNELINFO ChannelInfo;
#endif
#ifdef CONFIG_ATE
	/* lock for ATE */
	NDIS_SPIN_LOCK GenericLock;	/* ATE Tx/Rx generic spinlock */
#endif /* CONFIG_ATE */


	/*********************************************************/
	/*	  Both PCI/USB related parameters										 */
	/*********************************************************/
	/*RTMP_DEV_INFO				 chipInfo; */
	RTMP_INF_TYPE infType;
	UCHAR			AntMode;

	/*********************************************************/
	/*	  Driver Mgmt related parameters											*/
	/*********************************************************/
	/* OP mode: either AP or STA */
	UCHAR OpMode;		/* OPMODE_STA, OPMODE_AP */

	UINT32 BssInfoIdxBitMap0;/* mapping BssInfoIdx inside wdev struct with FW BssInfoIdx */
	UINT32 BssInfoIdxBitMap1;/* mapping BssInfoIdx inside wdev struct with FW BssInfoIdx */
	NDIS_SPIN_LOCK BssInfoIdxBitMapLock;

	UCHAR iface_combinations; /* AP , STA , AP+STA */

	RTMP_OS_TASK mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	/* If you want use timer task to handle the timer related jobs, enable this. */
	RTMP_TIMER_TASK_QUEUE TimerQ;
	NDIS_SPIN_LOCK TimerQLock;
	RTMP_OS_TASK timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef WF_RESET_SUPPORT
	RTMP_OS_TASK wf_reset_thread;
	INT wf_reset_state;
	BOOLEAN wf_reset_in_progress;
	INT wf_reset_wm_count;
	INT wf_reset_wa_count;
	INT wf_reset_wo_count;
#endif
	UINT Rx_FiFo_overflow_Count[DBDC_BAND_NUM];
	INT64 Rx_MPDU_Count[DBDC_BAND_NUM];
	BOOLEAN RxDebug[DBDC_BAND_NUM];

	/*********************************************************/
	/*	  Tx related parameters														   */
	/*********************************************************/
	/* resource for software backlog queues */
	QUEUE_HEADER TxSwQueue[WMM_NUM_OF_AC];
#ifdef IP_ASSEMBLY
	DL_LIST assebQueue[WMM_NUM_OF_AC];
	struct ip_assemble_data *cur_ip_assem_data[WMM_NUM_OF_AC];
#endif

	/* Maximum allowed tx software Queue length */
	UINT TxSwQMaxLen;
	NDIS_SPIN_LOCK tx_swq_lock[WMM_NUM_OF_AC];
	struct tx_swq_fifo tx_swq[WMM_NUM_OF_AC];

	UCHAR LastMCUCmd;

#ifdef REDUCE_TCP_ACK_SUPPORT
	struct hlist_head ackCnxHashTbl[REDUCE_ACK_MAX_HASH_BUCKETS];
	struct list_head ackCnxList;
	UINT32 ReduceAckConnections;
	struct delayed_work ackFlushWork;
	struct delayed_work cnxFlushWork;
	NDIS_SPIN_LOCK ReduceAckLock;
#endif

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
	BOOLEAN fgRaLimitPhyRate;
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
	/* RX re-assembly buffer for fragmentation */
	FRAGMENT_FRAME FragFrame;	/* Frame storage for fragment frame */
#ifdef MT_MAC
	TXS_CTL TxSCtl;
	TMR_CTRL_STRUCT *pTmrCtrlStruct;
#endif


#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT rCso;
#endif

	/***********************************************************/
	/*	  ASIC related parameters														  */
	/***********************************************************/
	struct qm_ctl qm_ctl;
	struct qm_ops *qm_ops;
	struct notify_entry qm_wsys_ne;
	struct tm_ops *tm_qm_ops;
	struct phy_ops *phy_op;
	struct hw_setting hw_cfg;

	UINT32 MACVersion;	/* MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101).. */
	UINT32 ChipID;
	UINT32 HWVersion;
	UINT32 FWVersion;
	UINT32 hw_bound;
	UINT16 ee_chipId;		/* Chip version. Read from EEPROM 0x00 to identify RT5390H */
	INT dev_idx;

#ifdef MT_MAC
	struct rtmp_mac_ctrl mac_ctrl;
	USHORT rx_pspoll_filter;
#endif /* MT_MAC */
#ifdef CONFIG_ICS_FRAME_HANDLE
	UINT16 rxvIcs;
#endif /* CONFIG_ICS_FRAME_HANDLE */

	/* --------------------------- */
	/* E2PROM									 */
	/* --------------------------- */
	enum EEPROM_STORAGE_TYPE eeprom_type;

	ULONG EepromVersion;	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	BOOLEAN EepromAccess;
	UCHAR EFuseTag;

#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN bUseEfuse;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR *EEPROMImage;

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
		EFUSE_INFO_ALL_T EfuseInfoAll;
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/

	UCHAR E2pAccessMode; /* Used to identify flash, efuse, eeprom or bin from start-up */
	struct _EEPROM_CONTROL E2pCtrl;

	BOOLEAN fgCalFreeApply;
	UINT16  RFlockTempIdx;
	UINT16  CalFreeTempIdx;

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	UINT32 CalFileOffset;
#endif /* CAL_BIN_FILE_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	BOOLEAN bDCOCReloaded;
	BOOLEAN bDPDReloaded;
	UCHAR *CalDCOCImage;
	UCHAR *CalDPDAPart1Image;
	UCHAR *CalDPDAPart2Image;
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_MT7622_SUPPORT
#define CAL_LOG_SIZE            5000
#define CAL_TXLPFG_SIZE		(4  * sizeof(UINT32))
#define CAL_TXDCIQ_SIZE		(48 * sizeof(UINT32))
#define CAL_TXDPD_PERCHAN_SIZE  (304 * sizeof(UINT32))
#define CAL_TXDPD_SIZE          (CAL_TXDPD_PERCHAN_SIZE * 14)
	BOOLEAN bPreCalMode;
	UCHAR *CalTXLPFGImage;
	UCHAR *CalTXDCIQImage;
	UCHAR *CalTXDPDImage;
	UINT16 TxDpdCalOfst;
#endif /*PRE_CAL_MT7622_SUPPORT*/
#if defined(PRE_CAL_MT7626_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT) || \
	defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) || \
	defined(PRE_CAL_MT7981_SUPPORT)
#define CAL_LOG_SIZE            6000
	BOOLEAN bPreCalMode;
	UCHAR   *PreCalImageInfo;
	UCHAR   *PreCalImage;
	UINT32  PreCalOfst;
	UCHAR   *TxDPDImage;
	UINT32  TxDPDOfst;
#endif
/* defined(PRE_CAL_MT7626_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT) ||
*  defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) ||
*  defined(PRE_CAL_MT7981_SUPPORT)
*/

	UINT32  DnlCalOfst;
	UCHAR   *TxDnlCal;
	UINT32  TssiCal2GOfst;
	UCHAR   *TssiCal2G;
	UINT32  TssiCal5GOfst;
	UCHAR   *TssiCal5G;
	UINT32  RXGainCalOfst;
	UCHAR   *RXGainCal;

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	UCHAR *PreCalReStoreBuffer;
	UCHAR *PreCalStoreBuffer;
	UINT16 PreCalWriteOffSet;
	UINT16 ChGrpMap;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	VOID *rlmCalCache;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	EEPROM_ANTENNA_STRUC Antenna;	/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
#ifdef DBDC_MODE
	UINT8 dbdc_band0_tx_path;
	UINT8 dbdc_band0_rx_path;
	UINT8 dbdc_band1_tx_path;
	UINT8 dbdc_band1_rx_path;
	UINT8 dbdc_band0_tx_num;
	UINT8 dbdc_band0_rx_num;
	UINT8 dbdc_band1_tx_num;
	UINT8 dbdc_band1_rx_num;
	UINT8 dbdc_unused_band0;
#endif
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	/* --------------------------- */
	/* BBP Control							   */
	/* --------------------------- */
#if defined(RTMP_BBP) || defined(CONFIG_ATE)
	/* TODO: shiang-6590, remove "defined(CONFIG_ATE)" after ATE has fully re-organized! */
	UCHAR BbpWriteLatch[MAX_BBP_ID + 1];	/* record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID */
	UCHAR Bbp94;
#endif /* defined(RTMP_BBP) || defined(CONFIG_ATE) */

	CHAR BbpRssiToDbmDelta;	/* change from UCHAR to CHAR for high power */
	BBP_R66_TUNING BbpTuning;

	/* ---------------------------- */
	/* RFIC control								 */
	/* ---------------------------- */
	UCHAR RfIcType;		/* RFIC_xxx */
	UCHAR RfFreqOffset;	/* Frequency offset for channel switching */

	RTMP_RF_REGS LatchRfRegs;	/* latch th latest RF programming value since RF IC doesn't support READ */

#ifdef RTMP_RBUS_SUPPORT
	UCHAR RfFreqDelta;	/* Frequency Delta */
#endif /* RTMP_RBUS_SUPPORT */

	/* This soft Rx Antenna Diversity mechanism is used only when user set */
	/* RX Antenna = DIVERSITY ON */
	SOFT_RX_ANT_DIVERSITY RxAnt;

	/* ---------------------------- */
	/* TxPower control						   */
	/* ---------------------------- */
	CHANNEL_TX_POWER TxPower[MAX_NUM_OF_CHANNELS];	/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */

	UCHAR MaxTxPwr;/*announced in beacon*/

	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
	ULONG Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
#ifdef DOT11_VHT_AC
	ULONG Tx80MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE]; /* Per-rate Tx power control for VHT BW80 (5GHz only) */
	BOOLEAN force_vht_op_mode;
#endif /* DOT11_VHT_AC */

	BOOLEAN force_one_tx_stream;
	BOOLEAN fgThermalProtectToggle;

	signed char BGRssiOffset[4]; /* Store B/G RSSI #0/1/2 Offset value on EEPROM 0x46h */
	signed char ARssiOffset[4]; /* Store A RSSI 0/1/2 Offset value on EEPROM 0x4Ah */

	CHAR BLNAGain;		/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR ALNAGain0;		/* Store A external LNA#0 value for ch36~64 */
	CHAR ALNAGain1;		/* Store A external LNA#1 value for ch100~128 */
	CHAR ALNAGain2;		/* Store A external LNA#2 value for ch132~165 */

#ifdef LED_CONTROL_SUPPORT
	/* LED control */
	LED_CONTROL LedCntl;
#endif /* LED_CONTROL_SUPPORT */

	/* ---------------------------- */
	/* MAC control								 */
	/* ---------------------------- */
	UCHAR wmm_cw_min; /* CW_MIN_IN_BITS, actual CwMin = 2^CW_MIN_IN_BITS - 1 */
	UCHAR wmm_cw_max; /* CW_MAX_IN_BITS, actual CwMax = 2^CW_MAX_IN_BITS - 1 */

	/* pre-build PS-POLL and NULL frame upon link up. for efficiency purpose. */
#ifdef CONFIG_STA_SUPPORT
	PSPOLL_FRAME PsPollFrame;
#endif /* CONFIG_STA_SUPPORT */
	HEADER_802_11 NullFrame;


#ifdef UAPSD_SUPPORT
	NDIS_SPIN_LOCK UAPSDEOSPLock;	/* EOSP frame access lock use */
	BOOLEAN bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif /* UAPSD_SUPPORT */

	MONITOR_STRUCT monitor_ctrl;
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	struct ch_switch_cfg ch_sw_cfg[DBDC_BAND_NUM];
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
	SAE_CFG SaeCfg;
#endif /* DOT11_SAE_SUPPORT */

	/*=========AP=========== */
#ifdef CONFIG_AP_SUPPORT
	/* ----------------------------------------------- */
	/* AP specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_AP */
	/* ----------------------------------------------- */
	AP_ADMIN_CONFIG ApCfg;	/* user configuration when in AP mode */
	AP_MLME_AUX ApMlmeAux;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626) || \
	defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	BCN_CHECK_INFO_STRUC BcnCheckInfo[DBDC_BAND_NUM];
#endif

#ifdef RT_CFG80211_SUPPORT
	RADIUS_ACCOUNT_ENTRY  radius_tbl[MAX_LEN_OF_MAC_TABLE];
#endif

#ifdef WDS_SUPPORT
	WDS_TABLE WdsTab;	/* WDS table when working as an AP */
#endif /* WDS_SUPPORT */

#ifdef MBSS_SUPPORT
	BOOLEAN FlgMbssInit;
#endif /* MBSS_SUPPORT */

#ifdef APCLI_SUPPORT
	BOOLEAN flg_apcli_init;
#endif /* APCLI_SUPPORT */

	/*#ifdef AUTO_CH_SELECT_ENHANCE */
	PBSSINFO pBssInfoTab;
	PCHANNELINFO pChannelInfo;
	/*#endif // AUTO_CH_SELECT_ENHANCE */

#endif /* CONFIG_AP_SUPPORT */

	/*=======STA=========== */
#ifdef CONFIG_STA_SUPPORT
	/* ----------------------------------------------- */
	/* STA specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_STA */
	/* ----------------------------------------------- */
	STA_ADMIN_CONFIG	StaCfg[MAX_MULTI_STA];	/* user desired settings */
	UCHAR			   MSTANum;				/* Actual working MSTA number */
	UCHAR			   MaxMSTANum;			 /* Mini of Max MSTA number from profile & MAX_MULTI_STA*/

	BOOLEAN			flg_msta_init;

	CHAR nickname[IW_ESSID_MAX_SIZE + 1];	/* nickname, only used in the iwconfig i/f */
	NDIS_MEDIA_STATE PreMediaState;
#endif /* CONFIG_STA_SUPPORT */

	/*=======Common=========== */
	enum RATE_ADAPT_ALG rateAlg;		/* Rate adaptation algorithm */

#ifdef PROFILE_PATH_DYNAMIC
	CHAR *profilePath;
#endif /* PROFILE_PATH_DYNAMIC */

	NDIS_MEDIA_STATE IndicateMediaState;	/* Base on Indication state, default is NdisMediaStateDisConnected */

#ifdef PROFILE_STORE
	RTMP_OS_TASK	WriteDatTask;
	BOOLEAN			bWriteDat;
#endif /* PROFILE_STORE */

#ifdef CREDENTIAL_STORE
	STA_CONNECT_INFO StaCtIf;
#endif /* CREDENTIAL_STORE */

#ifdef WSC_INCLUDED
	RTMP_OS_TASK wscTask;
	UCHAR WriteWscCfgToDatFile;
	BOOLEAN WriteWscCfgToAr9DatFile;
	NDIS_SPIN_LOCK WscElmeLock;
	MLME_QUEUE_ELEM *pWscElme;

	/* WSC hardware push button function 0811 */
	BOOLEAN WscHdrPshBtnFlag;	/* 1: support, read from EEPROM */
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bWscDriverAutoUpdateCfg;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

	/* MAT related parameters */
#ifdef MAT_SUPPORT
	MAT_STRUCT MatCfg;
#endif /* MAT_SUPPORT */

	/*
		Frequency setting for rate adaptation
			@ra_interval:		for baseline time interval
			@ra_fast_interval:	for quick response time interval
	*/
	UINT32 ra_interval;
	UINT32 ra_fast_interval;

	/* configuration: read from Registry & E2PROM */
	BOOLEAN bLocalAdminMAC;	/* Use user changed MAC */
	UCHAR PermanentAddress[MAC_ADDR_LEN];	/* Factory default MAC address */
	UCHAR CurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */

#ifdef MT_MAC
	UCHAR ExtendMBssAddr[EXTEND_MBSS_MAC_MAX][MAC_ADDR_LEN]; /* User defined MAC address for MBSSID*/
	BOOLEAN bLocalAdminExtendMBssMAC; /* Use user changed MAC */
	UCHAR ApcliAddr[MAX_MULTI_STA][MAC_ADDR_LEN]; /* User defined MAC address for APCLI*/
#endif

	/* ------------------------------------------------------ */
	/* common configuration to both OPMODE_STA and OPMODE_AP */
	/* ------------------------------------------------------ */
	UINT VirtualIfCnt;
	NDIS_SPIN_LOCK VirtualIfLock;

	COMMON_CONFIG CommonCfg;
#ifdef COEX_SUPPORT
	BOOLEAN BtCoexBeaconLimit;
	UINT32 BtWlanStatus;
	UINT32 BtCoexSupportMode;
	UINT32 BtCoexMode;
	UINT32 BtCoexLiveTime;
	UINT32 BtCoexBASize;
	UINT32 BtCoexOriBASize;
	UCHAR BtProtectionMode;
	UCHAR BtProtectionRate;
	UCHAR BtSkipFDDFix20MH;
#endif /* COEX_SUPPORT */
	MLME_STRUCT Mlme;

	/* AP needs those vaiables for site survey feature. */

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	SCAN_CTRL ScanCtrl[DBDC_BAND_NUM];
	BSS_TABLE         ScanTab;   /* store the latest SCAN result */
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

	EXT_CAP_INFO_ELEMENT ExtCapInfo;

#ifdef AIR_MONITOR
	UCHAR	MntEnable[DBDC_BAND_NUM];
	MNT_STA_ENTRY MntTable[DBDC_BAND_NUM][MAX_NUM_OF_MONITOR_STA];
	MNT_MUAR_GROUP MntGroupTable[DBDC_BAND_NUM][MAX_NUM_OF_MONITOR_GROUP];
	UCHAR	MntIdx[DBDC_BAND_NUM];
	UCHAR	curMntAddr[MAC_ADDR_LEN];
	UINT32 MonitrCnt[DBDC_BAND_NUM];
	UCHAR	MntRuleBitMap;
	UINT32	MntMaxPktCnt[DBDC_BAND_NUM];
#endif /* AIR_MONITOR */

	/* DOT11_H */
	struct DOT11_H Dot11_H[DBDC_BAND_NUM];

	/* encryption/decryption KEY tables */
	CIPHER_KEY SharedKey[MAX_BEACON_NUM + MAX_P2P_NUM][4];	/* STA always use SharedKey[BSS0][0..3] */

	/* various Counters */
	COUNTER_802_3 Counters8023;	/* 802.3 counters */
	COUNTER_802_11 WlanCounters[DBDC_BAND_NUM];	/* 802.11 MIB counters */
	COUNTER_RALINK RalinkCounters;	/* Ralink propriety counters */
	/* COUNTER_DRS DrsCounters;	*/ /* counters for Dynamic TX Rate Switching */
	PRIVATE_STRUC PrivateInfo;	/* Private information & counters */

	/* flags, see fRTMP_ADAPTER_xxx flags */
	ULONG Flags;		/* Represent current device status */
	ULONG PSFlags;		/* Power Save operation flag. */
	ULONG MoreFlags;	/* Represent specific requirement */

	/* current TX sequence # */
	USHORT Sequence;

	/* Control disconnect / connect event generation */
	/*+++Didn't used anymore */
	ULONG LinkDownTime;
	/*--- */
	ULONG LastRxRate;
	ULONG LastTxRate;

#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bConfigChanged;	/* Config Change flag for the same SSID setting */
#endif
	/*+++Used only for Station */
	BOOLEAN bDisableRtsProtect;
	/*--- */

	ULONG ExtraInfo;	/* Extra information for displaying status of UI */
	ULONG SystemErrorBitmap;	/* b0: E2PROM version error */

#ifdef SYSTEM_LOG_SUPPORT
	/* --------------------------- */
	/* System event log					   */
	/* --------------------------- */
	RT_802_11_EVENT_TABLE EventTab;
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef INF_PPA_SUPPORT
	UINT32 g_if_id;
	BOOLEAN PPAEnable;
	PPA_DIRECTPATH_CB *pDirectpathCb;
#endif /* INF_PPA_SUPPORT */

	/**********************************************************/
	/*	  Statistic related parameters													*/
	/**********************************************************/
	BOOLEAN bUpdateBcnCntDone;

	ULONG macwd;
	/* ---------------------------- */
	/* DEBUG paramerts */
	/* ---------------------------- */

	BOOLEAN bLinkAdapt;
	BOOLEAN bForcePrintTX;
	BOOLEAN bForcePrintRX;
	BOOLEAN bStaFifoTest;
	BOOLEAN bProtectionTest;
	BOOLEAN bHCCATest;
	BOOLEAN bGenOneHCCA;
	BOOLEAN bBroadComHT;
	ULONG BulkOutReq;
	ULONG BulkOutComplete;
	ULONG BulkOutCompleteOther;
	ULONG BulkOutCompleteCancel;	/* seems not use now? */
	struct wificonf WIFItestbed;

	UCHAR TssiGain;
#ifdef CONFIG_ATE
	struct _ATE_CTRL ATECtrl;
	struct _LOOPBACK_CTRL LbCtrl;
#endif

#ifdef CONFIG_WLAN_SERVICE
	struct service serv;
	/* prepare wdevs to occupy 2 wmm_set per band */
	struct wifi_dev ate_wdev[DBDC_BAND_NUM][2];
#endif /* CONFIG_WLAN_SERVICE */

#ifdef PRE_CAL_TRX_SET1_SUPPORT
	BOOLEAN KtoFlashDebug;
#endif

	/* statistics count */

	VOID *iw_stats;
	VOID *stats;

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	PMULTICAST_FILTER_TABLE pMulticastFilterTable;
	UCHAR IgmpGroupTxRate;
	PMULTICAST_WHITE_LIST_FILTER_TABLE pMcastWLTable;
#ifdef IGMP_SNOOPING_NON_OFFLOAD
	UINT mcast_policy;
#ifdef IGMP_SNOOPING_DENY_LIST
	P_MULTICAST_DENY_LIST_FILTER_TABLE pMcastDLTable;
#endif
#endif
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
	ETH_CONVERT_STRUCT EthConvert;
#endif /* ETH_CONVERT_SUPPORT */
	ULONG TbttTickCount;	/* beacon timestamp work-around */

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PeriodicTimer;
#endif /* CONFIG_AP_SUPPORT */

	/* for detect_wmm_traffic() BE TXOP use */
	UCHAR tx_one_second_ac_counter;
	ULONG tx_OneSecondnonBEpackets;	/* record non BE packets per second */
	ULONG rx_OneSecondnonBEpackets;
#define ONE_SECOND_NON_BE_PACKETS_THRESHOLD	(150)
	UCHAR is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR flg_be_adjust;
	ULONG be_adjust_last_time;
#ifdef WSC_INCLUDED
	/* for multiple card */
	UCHAR *pHmacData;
#endif /* WSC_INCLUDED */

#ifdef IKANOS_VX_1X0
	struct IKANOS_TX_INFO IkanosTxInfo;
	struct IKANOS_TX_INFO IkanosRxInfo[MAX_BEACON_NUM + MAX_WDS_ENTRY +
										   MAX_APCLI_NUM + MAX_MESH_NUM];
#endif /* IKANOS_VX_1X0 */



	UINT8 FlgCtsEnabled;
	UINT8 PM_FlgSuspend;

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	/* RIC */
#define FT_RIC_CB		((FT_RIC_CTRL_BLOCK *)(pAd->pFT_RIC_Ctrl_BK))
	VOID *pFT_RIC_Ctrl_BK;
	NDIS_SPIN_LOCK FT_RicLock;
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	UCHAR FifoUpdateDone, FifoUpdateRx;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	CFG80211_CTRL cfg80211_ctrl;
	VOID *pCfg80211_CB;
#ifdef IWCOMMAND_CFG80211_SUPPORT
	UINT CfgAPIfUseCnt;
	UINT CfgSTAIfUseCnt;
	NDIS_SPIN_LOCK CfgIfUseCntLock;
#endif /* IWCOMMAND_CFG80211_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef OS_ABL_SUPPORT
#ifdef MAT_SUPPORT
	/* used in OS_ABL */
	BOOLEAN (*MATPktRxNeedConvert)(RTMP_ADAPTER *pAd, PNET_DEV net_dev);

	PUCHAR (*MATEngineRxHandle)(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx);
#endif /* MAT_SUPPORT */
#endif /* OS_ABL_SUPPORT */

	UINT32 ContinueMemAllocFailCount;

	struct {
		INT IeLen;
		UCHAR *pIe;
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

	/* purpose: We free all kernel resources when module is removed */
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */

#ifdef OS_ABL_SUPPORT
#ifdef RTMP_PCI_SUPPORT
	RTMP_PCI_CONFIG PciConfig;
#endif /* RTMP_PCI_SUPPORT */
#endif /* OS_ABL_SUPPORT */

#ifdef P2P_SUPPORT
	RT_P2P_CONFIG			P2pCfg;
	NDIS_SPIN_LOCK			P2pTableSemLock;
	RT_P2P_TABLE			P2pTable;
	ULONG					GOBeaconBufNoALen;
	CHAR					GoBeaconBuf[512]; /* NOTE: BeaconBuf should be 4-byte aligned */
	ULONG					BeaconBufLen;
	ULONG					GoBeaconBufLen;
	BOOLEAN				bIsClearScanTab;   /* TURE, we need to force Scan */
	BOOLEAN				flg_p2p_init;
	ULONG					flg_p2p_OpStatusFlags;
	UCHAR					P2PChannel;
#ifdef DOT11_N_SUPPORT
	UINT8					P2PExtChOffset;
#endif /* DOT11_N_SUPPORT */
	UCHAR					P2PCurrentAddress[MAC_ADDR_LEN];	  /* User changed MAC address */
	PNET_DEV				p2p_dev;
#endif /* P2P_SUPPORT */

#ifdef SMART_ANTENNA
	int smartAntEnable;	/* trigger to enable the smartAntenna function */
	int smartAntDbgOn;
	SMART_ANTENNA_STRUCT *pSAParam;
	NDIS_SPIN_LOCK smartAntLock;/* lock used to handle synchronization of smart antenna algorithms. */
#endif /* SMART_ANTENNA */

#ifdef WIDI_SUPPORT
	USHORT mfg_code;
	USHORT prod_code;
	UINT serial_num;
	UCHAR cap_flag;
	UCHAR hori_size;
	UCHAR vert_size;
	/* Scan toggle for L2SD and P2P Listen Channel*/
	ULONG p2p_l2sd_scan_toggle;
	/* Flag to check whether you are sending a probe response. If set , stay on channel for dwell time*/
	INT gP2pSendingProbeResponse;
	/* p2p device Name*/
	UCHAR device_name[32];

	UCHAR EnterpriseEnabled; /* EnterpriseEnabled 1: enable 0: disable */
	UCHAR ToggleEnabled;	   /* tmcourie: ToggleEnabled 1: enable 0: disable */
#endif /* WIDI_SUPPORT */

#ifdef CONFIG_MULTI_CHANNEL
	CHAR NullFrBuf[256];
	UINT32 MultiChannelFlowCtl;
	RTMP_OS_TASK MultiChannelTask;
	UCHAR MultiChannelAction;
#endif /* CONFIG_MULTI_CHANNEL */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	WOW_CFG_STRUCT WOW_Cfg; /* data structure for wake on wireless */
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */

#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
	BOOLEAN bUseMultiMacAddrExt;
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
	BOOLEAN bUseHwTxLURate;
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	struct MCU_CTRL MCUCtrl;
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef WLAN_SKB_RECYCLE
	struct sk_buff_head rx0_recycle;
#endif /* WLAN_SKB_RECYCLE */

#ifdef CONFIG_MULTI_CHANNEL
	/* TODO: shiang-usw, duplicate definition with */
	USHORT NullBufOffset[2];
	CHAR NullFrBuf[100];
	UINT32 NullFrLen;
	/*
	UINT32 MultiChannelFlowCtl;
	RTMP_OS_TASK MultiChannelTask;
	UCHAR MultiChannelAction;
	*/
#endif /* CONFIG_MULTI_CHANNEL */

	DL_LIST PwrLimitSkuList;
	DL_LIST PwrLimitBackoffList;
	BOOLEAN fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_NUM];
	UCHAR DefaultTargetPwr;
	CHAR SingleSkuRatePwrDiff[19];
	BOOLEAN bOpenFileSuccess;

#ifdef TPC_SUPPORT
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	UINT8 SkuBcnTxPwr[BAND_NUM_MAX];
#endif
	UINT8 SkuMgmtTxPwr[BAND_NUM_MAX];
	UCHAR TxPwrParsedChannel[BAND_NUM_MAX];
#endif /* TPC_SUPPORT */


#ifdef WTBL_TDD_SUPPORT
	WTBL_TDD_DBG wtbl_dbg_level;

	/* log the first trigger time */
	/* Tasklet : Rx Trigger */
	BOOLEAN rx_trigger_log;
	UINT32  rx_trigger_time;

	/* Tasklet : Tx Trigger */
	BOOLEAN tx_trigger_log;
	UINT32  tx_trigger_time;

	/* Mlme :  task exec */
	BOOLEAN mlme_exec_log;
	UINT32  mlme_exec_time;

	/* log the spend time of in-band cmd */

	/* Mlme Task : */
	/* state machine function */
	UINT32 tdd_swap_req_ent_time;
	UINT32 tdd_swap_req_exit_time;

	UINT32 sm_swapout_ent_time; /* state machine function called */
	UINT32 sm_swapout_exit_time; /*state machine function called */

	UINT32 sm_swapin_ent_time; /* state machine function called */
	UINT32 sm_swapin_exit_time; /* state machine function called */

	/* HW Ctrl Task : */
	/* inner function */
	UINT32 hw_swap_out_ent_time;
	UINT32 hw_swap_out_exit_time;

	UINT32 hw_swap_in_ent_time;
	UINT32 hw_swap_in_exit_time;

	/* N9 cmds called in HW Ctrl Task : */
	UINT32 add_sta_rec_ent_time;
	UINT32 add_sta_rec_exit_time;

	/* check cmd treverse time */
	UINT32 add_sta_rec_send_time;
	UINT32 add_sta_rec_ack_time;
	UINT32 add_sta_rec_send_seq;
	UINT32 add_sta_rec_toolong_count; /* record the 4ms count */
	UINT32 add_sta_rec_total_count; /* record the 4ms count */

	UINT32 add_wtbl_ent_time;
	UINT32 add_wtbl_exit_time;
	/* check cmd treverse time */
	UINT32 add_wtbl_send_time;
	UINT32 add_wtbl_send_ack_time;
	UINT32 add_wtbl_send_seq;
	UINT32 add_wtbl_toolong_count; /* record the 4ms count */
	UINT32 add_wtbl_total_count; /* record the 4ms count */

	UINT32 del_sta_rec_ent_time;
	UINT32 del_sta_rec_exit_time;
	/* check cmd treverse time */
	UINT32 del_sta_rec_send_time;
	UINT32 del_sta_rec_ack_time;
	UINT32 del_sta_rec_send_seq;
	UINT32 del_sta_rec_toolong_count; /* record the 4ms count */
	UINT32 del_sta_rec_total_count; /* record the 4ms count */

	UINT32 del_wtbl_ent_time;
	UINT32 del_wtbl_exit_time;
	/* check cmd treverse time */
	UINT32 del_wtbl_send_time;
	UINT32 del_wtbl_send_ack_time;
	UINT32 del_wtbl_send_seq;
	UINT32 del_wtbl_toolong_count; /* record the 4ms count */
	UINT32 del_wtbl_total_count; /* record the 4ms count */
	/* get the zero value conut */
	UCHAR swap_out_empty_time[MAX_LEN_OF_MAC_TABLE];
#endif /* WTBL_TDD_SUPPORT */

#ifdef DBG_DIAGNOSE
	RtmpDiagStruct DiagStruct;
#endif /* DBG_DIAGNOSE */

#ifdef SNIFFER_SUPPORT
	struct sniffer_control sniffer_ctl;
#endif

#ifdef TXBF_SUPPORT
	MANUAL_CONN	AteManualConnInfo;
	PUCHAR		piBfPhaseG0;
	PUCHAR		piBfPhaseGx;
	BOOLEAN		fgCalibrationFail;
	BOOLEAN		fgGroupIdPassFailStatus[9];
	BOOLEAN		fgAutoStart;
#ifdef VHT_TXBF_SUPPORT
	BOOLEAN		NDPA_Request;
#endif
	TXBF_PFMU_STA_INFO rStaRecBf;
	PFMU_PROFILE_TAG1  rPfmuTag1;
	PFMU_PROFILE_TAG2  rPfmuTag2;
	PFMU_DATA	prof;
	UCHAR		ApCli_idx;
	UCHAR		ApCli_CmmWlanId;
	BOOLEAN		fgApCliBfStaRecRegister[BAND_NUM];
	BOOLEAN		fgApClientMode;
	BOOLEAN		fgClonedStaWithBfeeSelected;
	UCHAR		ReptClonedStaEntry_CliIdx;
	struct txbf_pfmu_tags_info pfmu_tags_info;
	UCHAR		u1IbfCalPhase2G5GE2pClean;
	UINT32 pfmu_data_raw[33];
	struct txbf_starec_conf manual_bf_sta_rec;
	UCHAR       profile_data_cnt;
	PFMU_HALF_DATA     profile_data[64];
	BF_DYNAMIC_MECHANISM bfdm;
#endif /* TXBF_SUPPORT */

#ifdef CFG_SUPPORT_MU_MIMO
	UINT32		u4RxMuPktCount;
	UINT32		u4TxMuPktCount;
#endif /* CFG_SUPPORT_MU_MIMO  */

	BOOLEAN		bPS_Retrieve;

	UINT32 rxv2_cyc3[10];

#ifdef DBG
#ifdef MT_MAC
	UCHAR BcnCnt; /* Carter debug */
	ULONG HandleInterruptTime;
	ULONG HandlePreInterruptTime;
#endif
#endif

	ULONG   TxTotalByteCnt;
	ULONG   RxTotalByteCnt;
#ifdef MT_MAC
	TX_AC_PARAM_T   CurrEdcaParam[CMD_EDCA_AC_MAX];
#endif /* MT_MAC */

#ifdef CONFIG_DVT_MODE
	DVTCTRL_T rDvtCtrl;
#endif /* CONFIG_DVT_MODE */

	/* ------------------- */
	/* For heart beat detection*/
	/* ------------------- */
	UINT32 pre_cr4_heart_beat_cnt;
	UINT32 pre_n9_heart_beat_cnt;
	UINT8 heart_beat_stop;

#ifdef FW_DUMP_SUPPORT
	/* ---------------------------- */
	/* For FW crash dump			*/
	/* ---------------------------- */
#define MAX_FW_DUMP_SIZE 1024000
	UCHAR *fw_dump_buffer;
	UINT32 fw_dump_max_size;
	UINT32 fw_dump_size;
	UINT32 fw_dump_read;
#endif

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	INT32 SpectrumStatus;
	INT32 ICapStatus;
	UINT8 ICapMode;
	UINT32 SpectrumEventCnt;
	UINT32 SpectrumIdx;
	UINT32 SpectrumCapNode;
	UINT32 SpectrumFixGain;
	UINT32 ICapEventCnt;
	UINT32 ICapDataCnt;
	UINT32 ICapL32Cnt;
	UINT32 ICapM32Cnt;
	UINT32 ICapH32Cnt;
	UINT32 ICapIdx;
	UINT32 ICapCapLen;
	UINT32 ICapCapNode;
	UINT32 ICapCapSrc;
	PUINT32 pL32Bit;
	PUINT32 pM32Bit;
	PUINT32 pH32Bit;
	RTMP_OS_FD pSrcf_IQ;
	RTMP_OS_FD pSrcf_Gain;
	RTMP_OS_FD pSrcf_InPhySniffer;
	RTMP_STRING *pSrc_IQ;
	RTMP_STRING *pSrc_Gain;
	RTMP_STRING *pSrc_InPhySniffer;
	RTMP_OS_COMPLETION SpectrumDumpDataDone;
	RTMP_OS_COMPLETION ICapDumpDataDone;
	P_RBIST_IQ_DATA_T pIQ_Array;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef BACKGROUND_SCAN_SUPPORT
	BACKGROUND_SCAN_CTRL BgndScanCtrl;
#endif /* BACKGROUND_SCAN_SUPPORT */
	VOID *hdev_ctrl;
#ifdef CONFIG_AP_SUPPORT
	/*for VOW HW CR address change.*/
	VOW_CR_OFFSET_FOR_GEN_T vow_gen;
#endif/* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	VOW_RX_TIME_CFG_T  vow_rx_time_cfg;
	VOW_BSS_USER_CFG_T vow_bss_cfg[VOW_MAX_GROUP_NUM];
	VOW_STA_USER_CFG_T vow_sta_cfg[MAX_LEN_OF_MAC_TABLE];
	VOW_CFG_T		  vow_cfg;
	VOW_AT_ESTIMATOR_T vow_at_est;
	VOW_BAD_NODE_T	 vow_badnode;
	VOW_MISC_CFG_T	   vow_misc_cfg;
	VOW_SCH_CFG_T	   vow_sch_cfg;
	UINT8  vow_dvt_en;
	UINT8  vow_show_en;
	UINT8  vow_monitor_sta;
	UINT8  vow_monitor_bss;
	UINT8  vow_monitor_mbss;
	UINT16 vow_avg_num;
	UINT32 vow_show_sta;
	UINT32 vow_show_mbss;

	UINT8	vow_watf_en;
	UINT8	vow_watf_q_lv0;
	UINT8	vow_watf_q_lv1;
	UINT8	vow_watf_q_lv2;
	UINT8	vow_watf_q_lv3;

	VOW_WATF vow_watf_mac[VOW_WATF_LEVEL_NUM];

	UINT8	vow_sta_frr_quantum; /* for fast round robin */
	BOOLEAN	vow_sta_frr_flag;
	struct multi_cli_ctl vow_mcli_ctl;
#endif /* CONFIG_AP_SUPPORT */
	struct bss_group_rec bss_group;
	UINT16 max_bssgroup_num;
#ifdef RED_SUPPORT
	UINT8  red_en;
	UINT8  red_mcu_offload;
	UINT8  red_debug_en;
	RED_STA_T red_sta[MAX_LEN_OF_MAC_TABLE];
	UINT16 red_targetdelay;
	UINT16 red_atm_on_targetdelay;
	UINT16 red_atm_off_targetdelay;
	UINT16 red_sta_num;
	UINT8 red_in_use_sta;
	RALINK_TIMER_STRUCT red_badnode_timer;
	P_RED_CTRL_T prRedCtrl;
	RED_CTRL_T RedCtrl;
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	struct fq_ctrl_type fq_ctrl;
#endif /* FQ_SCH_SUPPORT */
	UINT8  rts_retrylimit[DBDC_BAND_NUM];
	UINT8  retrylimit[TxQ_IDX_AC33 + 1];
	UINT8  cp_support;
	UINT8  dabs_qos_op;
#ifdef CONFIG_FWOWN_SUPPORT
	UINT8 bDrvOwn;
	UINT8 bDrvOwn1;
	UINT8 bCRAccessing;
	UINT8 bSetFWOwnRunning;
	NDIS_SPIN_LOCK DriverOwnLock;
#endif /* CONFIG_FWOWN_SUPPORT */

	WIFI_SYS_INFO_T WifiSysInfo;

#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T ErrRecoveryCtl;
#endif
#ifdef TX_AGG_ADJUST_WKR
	BOOLEAN TxAggAdjsut;
#endif /* TX_AGG_ADJUST_WKR */
	NDIS_SPIN_LOCK TimerSemLock;

	RX_STAT_CNT rx_stat_cnt[DBDC_BAND_NUM];
	RX_STATISTIC_RXV rx_stat_rxv[DBDC_BAND_NUM];
	RXV_RAW_DATA rxv_raw_data;
	RXV_DUMP_CTRL rxv_dump_ctrl;
	UINT8 rxv_entry_sta_cnt[DBDC_BAND_NUM];
	UINT32 parse_rxv_stat_enable;
	UINT32 AccuOneSecRxBand0FcsErrCnt;
	UINT32 AccuOneSecRxBand0MdrdyCnt;
	UINT32 AccuOneSecRxBand1FcsErrCnt;
	UINT32 AccuOneSecRxBand1MdrdyCnt;

#ifdef TRACELOG_TCP_PKT
	UINT32 u4TcpRxAckCnt;
	UINT32 u4TcpTxDataCnt;
#endif

	UINT16 assignWcid;
#ifdef AUTOMATION
	BSS_CHECK_CTRL BssChkCtrl[DBDC_BAND_NUM];
	AUTOMATION_DVT *auto_dvt;
#endif
#ifdef WIFI_MODULE_DVT
	VOID *mdvt;
#endif
#ifdef SMART_CARRIER_SENSE_SUPPORT
	SMART_CARRIER_SENSE_CTRL	SCSCtrl;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT
	struct DYNAMIC_WMM_CTRL DynWmmCtrl;
#endif /* DYNAMIC_WMM_SUPPORT */
	RvR_Debug_CTRL RVRDBGCtrl;

	UINT16						   CCI_ACI_TxOP_Value[DBDC_BAND_NUM];
	UINT16						   MUMIMO_TxOP_Value;
	BOOL							 G_MODE_INFRA_TXOP_RUNNING;
	struct wifi_dev				  *g_mode_txop_wdev;

	MT_MIB_BUCKET_ONE_SEC   OneSecMibBucket;
	MT_MIB_BUCKET_MS		MsMibBucket;
	UINT8  partial_mib_show_en;
	struct peak_tp_ctl peak_tp_ctl[DBDC_BAND_NUM];
	struct txop_ctl txop_ctl[DBDC_BAND_NUM];
	struct multi_cli_ctl mcli_ctl[DBDC_BAND_NUM];
#ifdef ANTENNA_DIVERSITY_SUPPORT
	struct ant_diversity_ctl diversity_ctl[DBDC_BAND_NUM];
	UINT32 phy_rate_average[DBDC_BAND_NUM];
	UINT32 rate_rec_sum[DBDC_BAND_NUM];
	UINT32 rate_rec_num[DBDC_BAND_NUM];
	UINT32 cn_rec_sum[DBDC_BAND_NUM];
	UINT32 cn_rec_num[DBDC_BAND_NUM];
	UINT32 cn_average[DBDC_BAND_NUM];
	UINT32 cn_rate_read_interval;
	BOOLEAN rec_start_cn_flag[DBDC_BAND_NUM];
	BOOLEAN rec_start_rx_rate_flag[DBDC_BAND_NUM];
	INT32 RSSI[MAX_ANT_NUM];
	INT32 RCPI[MAX_ANT_NUM];
#endif
	struct ixia_mode_ctl_type ixia_mode_ctl;
	UINT16 multi_cli_nums_eap_th;
	BOOLEAN aggManualEn;
	UINT8 per_dn_th;
	UINT8 per_up_th;
	UINT32 winsize_kp_idx;
#ifdef PKTLOSS_CHK
	pktloss_check_var_type	pktloss_chk;
#endif
#ifdef PKT_BUDGET_CTRL_SUPPORT
	UINT16 pbc_bound[DBDC_BAND_NUM][PBC_AC_NUM];
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

#ifdef HTC_DECRYPT_IOT
	UINT32 HTC_ICV_Err_TH; /* threshold */
#endif /* HTC_DECRYPT_IOT */

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
	UINT32 rxThreshold;
	UINT32 rxPassThresholdCnt;
#endif

#ifdef DHCP_UC_SUPPORT
	BOOLEAN DhcpUcEnable;
#endif /* DHCP_UC_SUPPORT */
	struct wpf_ctrl wpf;
	/* For QAtool log buffer limitation. */
	UINT16  u2LogEntryIdx;
	UINT8	fgDumpStart;
	UINT8	fgQAtoolBatchDumpSupport;
#ifdef MULTI_PROFILE
	VOID *multi_pf_ctrl;
#endif /*MULTI_PROFILE*/
#ifdef A4_CONN
	UINT32 a4_interface_count;
	UINT32 a4_need_refresh;
#endif
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	BOOLEAN bIGMPperiodicQuery; /* Enable/Disable Periodic IGMP query to non-MWDS STA*/
	UINT8 IgmpQuerySendTick; /* Period for IGMP Query in unit of 10 sec*/
	BOOLEAN bMLDperiodicQuery;	/* Enable/Disable Periodic MLD query to non-MWDS STA*/
	UINT8 MldQuerySendTick; /* Period for MLD queryin unit of 10 sec*/
#endif
	UINT8 nearfar_far_client_num[BAND_NUM]; /* far client number in near/far condition */
#ifdef DBG_STARVATION
	struct starv_log starv_log_ctrl;
#endif /*DBG_STARVATION*/
#ifdef MBO_SUPPORT
	UINT8 MboBssTermCountDown;
	UINT8 Mbo_Bss_Terminate_Index;
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	struct wifi_dev *bh_bss_wdev[DBDC_BAND_NUM];
	UCHAR bMAPTurnKeyEnable;
	UCHAR MAPMode;
	UCHAR MapBalance;
	UCHAR bMAPAvoidScanDuringCac;
#ifdef CONFIG_MAP_3ADDR_SUPPORT
	UCHAR MapAccept3Addr;
#endif
#ifdef MAP_R2
	UCHAR bMapR2Enable;
#endif
#ifdef MAP_R3
	UCHAR bMapR3Enable;
#endif
#ifdef MAP_R4
	UCHAR bMapR4Enable;
#endif
#ifdef MAP_TS_TRAFFIC_SUPPORT
	UCHAR bTSEnable;
#endif
#endif /* CONFIG_MAP_SUPPORT */
#ifdef QOS_R1
	UCHAR bQoSR1Enable;
#ifdef QOS_R2
	UCHAR bScsEnable;
#endif
#endif
#ifdef WAPP_SUPPORT
	struct BSS_LOAD_INFO bss_load_info;
#endif /* WAPP_SUPPORT */

	UCHAR reg_domain;

#ifdef FW_LOG_DUMP
	FW_LOG_CTRL fw_log_ctrl;
#endif /* FW_LOG_DUMP */

#ifdef PHY_ICS_SUPPORT
	BOOLEAN PhyIcsFlag;
#endif /* PHY_ICS_SUPPORT */

#ifdef APCLI_SUPPORT
	BOOLEAN bApCliCertTest;
#endif
#ifdef WAPP_SUPPORT
	struct scan_req last_scan_req;
#ifdef CONFIG_CPE_SUPPORT
	struct lppe_scan_req last_scan_req_lppe;
#endif
#endif
#ifdef VERIFICATION_MODE
	struct veri_ctrl veri_ctrl;
#endif
#ifdef CONFIG_WIFI_SYSDVT
	VOID *dvt_ctrl;
	BOOLEAN bStaHeTest;
#endif /*CONFIG_WIFI_SYSDVT*/

#ifdef CONFIG_WIFI_DBG_TXCMD
	VOID *dbg_txcmd_ctrl;
#endif /*CONFIG_WIFI_DBG_TXCMD*/

	struct wifi_feature_support_list_query wifi_cap_list;
#ifdef DPP_SUPPORT
	UCHAR bDppEnable;
	UINT32 dpp_rx_frm_counter;
	UCHAR bDppGasInitReqRecived;
#endif /* DPP_SUPPORT */
#ifdef WIFI_DIAG
	void *pDiagCtrl[DBDC_BAND_NUM];
#endif
#ifdef MAP_R2
	radio_info ra_info[DBDC_BAND_NUM];
#endif
#ifdef MAP_R3
	UCHAR *dpp_uri_ptr;
	UCHAR dpp_uri_len;
	BOOLEAN map_sec_enable;
	BOOLEAN map_onboard_type;
#endif /* MAP_R3 */

	BOOLEAN bIsBeenDumped;
	UINT16 FwCmdTimeoutCnt;
	UINT16 FwCmdTimeoutPrintCnt;	/* 0 means No limitation */
#ifdef WF_RESET_SUPPORT
	UINT16 FwCmdTimeoutcheckCnt;
#endif
	FWCMD_TIMEOUT_RECORD FwCmdTimeoutRecord[FW_CMD_TO_RECORD_CNT];

#ifdef RATE_PRIOR_SUPPORT
	LOWRATE_CTRL LowRateCtrl;
#endif/*RATE_PRIOR_SUPPORT*/
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	BOOLEAN V10UpdateRssi;
#endif
#ifdef ANTENNA_CONTROL_SUPPORT
	UINT8 TxStream[DBDC_BAND_NUM];
	UINT8 RxStream[DBDC_BAND_NUM];
	UINT8 bAntennaSetAPEnable[DBDC_BAND_NUM];
#endif /* ANTENNA_CONTROL_SUPPORT */
	MT_MIB_COUNTER_STAT _rPrevMibCnt;
#ifdef NF_SUPPORT_V2
	INT32 Avg_NF[DBDC_BAND_NUM];
#endif
#ifdef IXIA_C50_MODE
	struct ixia_ctl ixia_ctl;
	struct ixia_tx_cnt tx_cnt;
	struct ixia_rx_cnt rx_cnt;
#endif

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
	BOOLEAN idcState;	/* TRUE: IDC enable; FALSE: IDC disable */
	UINT32 coex_pre_wm_pc;
	COEX_APCCCI2FW_CMD coex_apccci2fw_cmd;
	EXT_EVENT_FW2APCCCI_T fw2apccci_msg;
	LTE_SAFE_CH_CTRL LteSafeChCtrl;
	COEX_IDC_INFO idcInfo;
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef ZERO_LOSS_CSA_SUPPORT
	ZERO_LOSS_STA ZeroLossSta[3];
	UINT8 ZeroLossStaCount;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef PER_PKT_CTRL_FOR_CTMR
	BOOLEAN PerPktCtrlEnable;
#endif
	UINT32 temperature;
#ifdef CFG_SUPPORT_CSI
	struct CSI_INFO_T rCSIInfo;
#endif
	UINT8 Wifi6gCap;
	NOP_LIST *NopListBk;
#ifdef DABS_QOS
	UCHAR dabs_drop_threashold;
	bool mscs_req_reject;
#ifdef MSCS_PROPRIETARY
	UINT32 keybitmap[4];
	bool  SupportFastPath;
	UINT8 dabs_version;
#endif
#endif/*DABS_QOS*/
	CHANNEL_OP_CTRL ChOpCtrl[DBDC_BAND_NUM];

#ifdef WIFI_UNIFIED_COMMAND
	BOOLEAN RxvOfRxEnable[DBDC_BAND_NUM];
	BOOLEAN RxvOfTxEnable[DBDC_BAND_NUM];
#endif /* WIFI_UNIFIED_COMMAND */
	struct wo_rx_total_cnt wo_rxcnt[NUM_OF_TID][MAX_LEN_OF_MAC_TABLE];
#ifdef DELAY_TCP_ACK_V2  /*for panther*/
	struct wo_rx_total_cnt wo_last_rxcnt[NUM_OF_TID][MAX_LEN_OF_MAC_TABLE];
#endif /* DELAY_TCP_ACK_V2 */
#ifdef MAP_R3
	BOOLEAN ReconfigTrigger;
#endif
#ifdef RXD_WED_SCATTER_SUPPORT
	struct rx_data RxDHisLog[MAX_RECORD];
	UINT8 rxd_log_idx;
	UINT32 rxd_total_drop_cnt;
	UINT32 rxd_unkown_head_drop_cnt;
	UINT32 rxd_unkown_drop_cnt;
	UINT32 rxd_len_error_drop_cnt;
	UINT32 rxd_len_error_searh_pa_cnt;
	UINT32 rxd_unknown_type_cnt;
	UINT32 rxd_token_correct_1;
	UINT32 rxd_token_correct_2;
	UINT32 rxd_skb_copy_fail_cnt;
	UINT32 rxd_gather_fail_cnt;
	UINT32 rxd_nonscatter_error_cnt;
	UINT32 rxd_scatter_len_error_cnt;
	struct rx_scatter_data RxScatHisLog[MAX_RECORD];
	UINT8 rxd_scat_log_idx;
	UINT32 rxd_scat_drop_cnt;
#endif /* RXD_WED_SCATTER_SUPPORT */
#ifdef CONFIG_6G_AFC_SUPPORT
	struct AFC_RESPONSE_DATA afc_response_data;
#endif /* CONFIG_6G_AFC_SUPPORT */
};

typedef struct _PEER_PROBE_REQ_PARAM {
#ifdef CONFIG_AP_SUPPORT
	UCHAR Addr1[MAC_ADDR_LEN];
#endif
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	UINT32 ShortSSID;
	BOOLEAN bRequestRssi;
#ifdef CONFIG_HOTSPOT
	BOOLEAN IsIWIE;
	BOOLEAN IsIWCapability;
	UCHAR Hessid[MAC_ADDR_LEN];
	BOOLEAN IsHessid;
	UINT8 AccessNetWorkType;
#endif /* CONFIG_HOTSPOT */
#ifdef BAND_STEERING
	BOOLEAN IsHtSupport;
	BOOLEAN IsVhtSupport;
	UINT32 RxMCSBitmask;
/* WPS_BandSteering Support */
	BOOLEAN bWpsCapable;
#endif
#ifdef WH_EVENT_NOTIFIER
	IE_LISTS ie_list;
#endif /* WH_EVENT_NOTIFIER */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	struct probe_req_report report_param;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef OCE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bProbeSupp[WDEV_NUM_MAX];
	BOOLEAN IsOceCapability;
	UINT32 MaxChannelTime;
#endif
#endif
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UINT8 bfer_cap_su;
	UINT8 num_snd_dimension;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef MAP_6E_SUPPORT
	struct map_rnr rnr_ie;
#endif
} PEER_PROBE_REQ_PARAM, *PPEER_PROBE_REQ_PARAM;

/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef enum RX_BLK_FLAGS {
	fRX_AMPDU = (1 << 0),
	fRX_AMSDU = (1 << 1),
	fRX_ARALINK = (1 << 2),
	fRX_HTC = (1 << 3),
	fRX_PAD = (1 << 4),
	fRX_QOS = (1 << 5),
	fRX_EAP = (1 << 6),
	fRX_WPI = (1 << 7),
	fRX_AP = (1 << 8),			/* Packet received from AP */
	fRX_STA = (1 << 9),			/* Packet received from Client(Infra mode) */
	fRX_ADHOC = (1 << 10),		/* packet received from AdHoc peer */
	fRX_WDS = (1 << 11),		/* Packet received from WDS */
	fRX_MESH = (1 << 12),		/* Packet received from MESH */
	fRX_DLS = (1 << 13),		/* Packet received from DLS peer */
	fRX_TDLS = (1 << 14),		/* Packet received from TDLS peer */
	fRX_RETRIEVE = (1 << 15),	/* Packet received from mcu */
	fRX_CMD_RSP = (1 << 16),	/* Pakket received from mcu command response */
	fRX_TXRX_RXV = (1 << 17),   /* RxV received from Rx Ring1 */
	fRX_HDR_TRANS = (1 << 18),
	fRX_WCID_MISMATCH = (1 << 19), /* for HW Lookup Wcid Mismatch */
	fRX_CM = (1 << 20),
	fRX_CLM = (1 << 21),
	fRX_ICV_ERR = (1 << 22),
	fRX_TKIP_MIC_ERR = (1 << 23),
} RX_BLK_FLAGS;

#define QOS_USER_PRIORITY_MASK 0x000f
enum Qos_User_Priority {
	AC_BE_be = 0,
	AC_BK_bk,
	AC_BK_,
	AC_BE_ee,
	AC_VI_cl,
	AC_VI_vi,
	AC_VO_vo,
	AC_VO_nc,
};

typedef struct _RX_BLK {
	UCHAR hw_rx_info[RXD_SIZE]; /* include "RXD_STRUC RxD" and "RXINFO_STRUC rx_info " */
	RXINFO_STRUC *pRxInfo; /* for RLT, in head of frame buffer, for RTMP, in hw_rx_info[RXINFO_OFFSET] */
	/* TODO: shiang-usw, revise this! */
	RXWI_STRUC *pRxWI; /*in frame buffer and after "rx_info" fields */
	UCHAR *rmac_info;
	UCHAR *FC;
	UINT16 Duration;
	UCHAR FN;
	UINT16 SN;
	UCHAR *Addr1;
	UCHAR *Addr2;
	UCHAR *Addr3;
	UCHAR *Addr4;
	PNDIS_PACKET pRxPacket; /* os_packet pointer, shall not change */
	UCHAR *pData; /* init to pRxPacket->data, refer to frame buffer, may changed depends on processing */
	USHORT DataSize; /* init to  RXWI->MPDUtotalByteCnt, and may changes depends on processing */
	RX_BLK_FLAGS Flags;

	/* Mirror info of partial fields of RxWI and RxInfo */
	USHORT MPDUtotalByteCnt; /* Refer to RXWI->MPDUtotalByteCnt */
	UCHAR UserPriority;	/* for calculate TKIP MIC using */
	UCHAR OpMode;	/* 0:OPMODE_STA 1:OPMODE_AP */
	UCHAR band;		/* band index */
	UINT16 wcid;		/* copy of pRxWI->wcid */
	UCHAR U2M;
	UCHAR key_idx;
	UCHAR bss_idx;
	UCHAR TID;
	UINT32 TimeStamp;
	struct rx_signal_info rx_signal;
	CHAR ldpc_ex_sym;
	HTTRANSMIT_SETTING rx_rate;
	UINT32 rxv2_cyc1;
	UINT32 rxv2_cyc2;
	UINT32 rxv2_cyc3;
#ifdef HDR_TRANS_SUPPORT
	BOOLEAN bHdrVlanTaged;	/* VLAN tag is added to this header */
	UCHAR *pTransData;
	USHORT TransDataSize;
#endif /* HDR_TRANS_SUPPORT */
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT rCso;
#endif
#ifdef CUT_THROUGH
	UINT16 token_id;
#endif /* CUT_THROUGH */
	UINT8 sec_mode;
	UINT8 AmsduState;
	BOOLEAN DeAmsduFail;
	UINT64 CCMP_PN;
	UINT16 channel_freq;
	UINT8 rcpi[MAX_ANT_NUM];
	UCHAR *AMSDU_ADDR;
#ifdef VLAN_SUPPORT
	UCHAR is_htf;
#endif

} RX_BLK ____cacheline_aligned;




#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))

#define AMSDU_SUBHEAD_LEN	14
#define ARALINK_SUBHEAD_LEN	14
#define ARALINK_HEADER_LEN	 2

typedef enum TX_FRAME_TYPE {
	TX_UNKOWN_FRAME,
	TX_MCAST_FRAME = 1,
	TX_LEGACY_FRAME = 2,
	TX_AMPDU_FRAME = 3,
	TX_AMSDU_FRAME = 4,
	TX_FRAG_FRAME = 5,
	TX_MLME_MGMTQ_FRAME = 6,
	TX_MLME_DATAQ_FRAME = 7,
	TX_ATE_FRAME = 8,
	TX_VERIFY_FRAME = 9,
} TX_FRAME_TYPE;

#define TX_FRAG_ID_NO			0x0
#define TX_FRAG_ID_FIRST		0x1
#define TX_FRAG_ID_MIDDLE		0x2
#define TX_FRAG_ID_LAST			0x3

#define TX_AMSDU_ID_NO 0x0
#define TX_AMSDU_ID_FIRST 0x1
#define TX_AMSDU_ID_MIDDLE 0x2
#define TX_AMSDU_ID_LAST 0x3

typedef enum TX_BLK_FLAGS {
	fTX_bRtsRequired = (1 << 0),
	fTX_bAckRequired = (1 << 1),
	fTX_bPiggyBack = (1 << 2),
	fTX_bHTRate = (1 << 3),
	fTX_bWMM = (1 << 4),
	fTX_bAllowFrag = (1 << 5),
	fTX_bMoreData = (1 << 6),
	fTX_bRetryUnlimit = (1 << 7),
	fTX_bClearEAPFrame = (1 << 8),
	fTX_bApCliPacket = (1 << 9),
	fTX_bSwEncrypt = (1 << 10),
	fTX_bWMM_UAPSD_EOSP = (1 << 11),
	fTX_bWDSEntry = (1 << 12),
	fTX_bDonglePkt = (1 << 13),
	fTX_bMeshEntry = (1 << 14),
	fTX_bWPIDataFrame = (1 << 15),
	fTX_bClientWDSFrame = (1 << 16),
	fTX_bTdlsEntry = (1 << 17),
	fTX_AmsduInAmpdu = (1 << 18),
	fTX_ForceRate = (1 << 19),
	fTX_CT_WithTxD = (1 << 20),
	fTX_CT_WithoutTxD = (1 << 21),
	fTX_DumpPkt = (1 << 22),
	fTX_HDR_TRANS = (1 << 23),
	fTX_MCU_OFFLOAD = (1 << 24),
	fTX_MCAST_CLONE = (1 << 25),
	fTX_HIGH_PRIO = (1 << 26),
#ifdef A4_CONN
	fTX_bA4Frame = (1 << 27),
#endif
	fTX_HW_AMSDU = (1 << 28),
	fTX_bNoRetry = (1 << 29),
	fTX_bAteTxsRequired = (1 << 30),
	fTX_bAteAgg = (1 << 31)
} TX_BLK_FLAGS;

typedef struct _TX_BLK {
	UCHAR resource_idx ____cacheline_aligned;
	UCHAR QueIdx;
	UCHAR WmmIdx;
	UCHAR TotalFrameNum;				/* Total frame number want to send-out in one batch */
	UCHAR TotalFragNum;				/* Total frame fragments required in one batch */
	UCHAR TxFrameType;				/* Indicate the Transmission type of the all frames in one batch */
	USHORT TotalFrameLen;				/* Total length of all frames want to send-out in one batch */
	STA_TR_ENTRY *tr_entry;
	PNDIS_PACKET pPacket;
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY	*pMacEntry;			/* NULL: packet with 802.11 RA field is multicast/broadcast address */
	TX_BLK_FLAGS Flags;
	UCHAR UserPriority;				/* priority class of packet */
	UINT16 Wcid;						/* The MAC entry associated to this packet */
	UINT8 wmm_set;
	UINT8 dbdc_band;
	UCHAR *pSrcBufHeader;				/* Reference to the head of sk_buff->data */
	UINT SrcBufLen;					/* Length of packet payload which not including Layer 2 header */
	UCHAR wifi_hdr_len; /* 802.11 header */
	UCHAR frame_idx;
	UCHAR amsdu_state;
	UCHAR FragIdx;					/* refer to TX_FRAG_ID_xxxx */
	UCHAR *pSrcBufData;				/* Reference to the sk_buff->data, will changed depends on hanlding progresss */
	UCHAR HdrPadLen; /* padding length*/
	UCHAR MpduHeaderLen; /* 802.11 header + LLC/SNAP not including padding */
	INT16 tx_bytes_len; /* txd + packet length not including txp */
	UCHAR txp_len;
	UCHAR hw_rsv_len;
	UCHAR first_buf_len;
	ra_dma_addr_t SrcBufPA;
	QUEUE_HEADER TxPacketList;
	HTTRANSMIT_SETTING *pTransmit;
	UCHAR *pExtraLlcSnapEncap;			/* NULL means no extra LLC/SNAP is required */
	UCHAR *HeaderBuf;
	UCHAR *wifi_hdr;
	UCHAR FrameGap;					/* what kind of IFS this packet use */
	UCHAR MpduReqNum;					/* number of fragments of this frame */
	UCHAR TxRate;						/* TODO: Obsoleted? Should change to MCS? */
	UINT32 CipherAlg;						/* cipher alogrithm */
	PCIPHER_KEY	pKey;
	UCHAR KeyIdx;						/* Indicate the transmit key index */
	UCHAR OpMode;
#ifdef DSCP_PRI_SUPPORT
	INT8				DscpMappedPri;
#endif /*DSCP_PRI_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT ApCliIfidx;
	PSTA_ADMIN_CONFIG		pApCliEntry;
#endif /* APCLI_SUPPORT */
	BSS_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */
	ULONG Priv;						/* Hardware specific value saved in here. */

#if defined(CONFIG_CSO_SUPPORT) || defined(CONFIG_TSO_SUPPORT)
	UCHAR naf_type;
#endif /* defined(CONFIG_CSO_SUPPORT) || defined(CONFIG_TSO_SUPPORT) */

#ifdef TXBF_SUPPORT
	UCHAR TxSndgPkt; /* 1: sounding 2: NDP sounding */
	UCHAR TxNDPSndgBW;
	UCHAR TxNDPSndgMcs;
#endif /* TXBF_SUPPORT */
	UINT8 dot11_type;
	UINT8 dot11_subtype;
	UINT8 TxSFormat;
	UINT8 Pid;
	UINT8 TxS2Host;
	UINT8 TxS2Mcu;
	UINT32 TxSPriv;
#ifdef RANDOM_PKT_GEN
	UINT8 lmac_qidx;
#endif
	struct phy_params *phy_info;
	BOOLEAN DropPkt;

#ifdef PER_PKT_CTRL_FOR_CTMR
	BOOLEAN ApplyTid;
	UCHAR TidByHost;

	BOOLEAN ApplyRetryLimit;
	UCHAR RetryLimitByHost;
	UCHAR MaxTxTimeByHost;

	BOOLEAN ApplyFixRate;
	TRANSMIT_SETTING_HE TransmitHeByHost;

	BOOLEAN ApplyBaPowerTxs;
	BOOLEAN BaDisableByHost;
	UINT8 TxS2HostByHost;
	UINT8 PidByHost;
	UCHAR PowerOffsetByHost;
#endif
#ifdef VLAN_SUPPORT
	UCHAR is_vlan;
#endif
	UINT32 HeaderBuffer[32] ____cacheline_aligned;
} TX_BLK;

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))

#ifdef DBG_DEQUE
struct deq_log_struct {
	UCHAR que_depth[WMM_NUM_OF_AC];
	UCHAR deq_cnt[WMM_NUM_OF_AC];
	UCHAR deq_round;
};
#endif /* DBG_DEQUE */

typedef struct dequeue_info {
	BOOLEAN inited;
	UCHAR start_q;
	UCHAR end_q;
	CHAR cur_q;
	UINT16 target_wcid;
	UCHAR target_que;
	UINT16 cur_wcid;
	USHORT q_max_cnt[WMM_QUE_NUM];
	INT pkt_bytes;
	INT pkt_cnt;
	INT deq_pkt_bytes;
	INT deq_pkt_cnt;
	INT status;
	BOOLEAN full_qid[WMM_QUE_NUM];
#ifdef DBG_DEQUE
	deq_log_struct deq_log;
#endif /* DBG_DEQUE */
} DEQUE_INFO;

#ifdef CONFIG_MT7976_SUPPORT
typedef enum EEPROM_PWR_ON_MODE {
	EEPROM_PWR_ON_CAL_2G_5G = 0,
	EEPROM_PWR_ON_CAL_2G_6G = 1,
} EEPROM_PWR_ON_MODE_T;
#endif /* CONFIG_MT7976_SUPPORT */
#ifdef TR181_SUPPORT
/* Device.WiFi.Radio.{i} */
typedef struct _wifi_radio_stats {
	ULONG BytesSent;
	ULONG BytesReceived;
	ULONG PacketsSent;
	ULONG PacketsReceived;
	ULONG ErrorsSent;
	ULONG ErrorsReceived;
	ULONG DiscardPacketsSent;
	ULONG DiscardPacketsReceived;
	UINT PLCPErrorCount; /* MT7915 Not Support, default: 0 */
	UINT FCSErrorCount; /* MT7915 Not Support, default: 0 */
	UINT InvalidMACCount; /* MT7915 Not Support, default: 0 */
	UINT PacketsOtherReceived; /* MT7915 Not Support, default: 0 */
	ULONG CtsReceived; /* MT7915 Not Support, default: 0 */
	ULONG NoCtsReceived; /* MT7915 Not Support, default: 0 */
	ULONG FrameHeaderError; /* MT7915 Not Support, default: 0 */
	ULONG GoodPLCPReceived; /* MT7915 Not Support, default: 0 */
	ULONG DPacketOtherMACReceived; /* MT7915 Not Support, default: 0 */
	ULONG MPacketOtherMACReceived; /* MT7915 Not Support, default: 0 */
	ULONG CPacketOtherMACReceived; /* MT7915 Not Support, default: 0 */
	ULONG CtsOtherMACReceived; /* MT7915 Not Support, default: 0 */
	ULONG RtsOtherMACReceived; /* MT7915 Not Support, default: 0 */
	UINT TotalChannelChangeCount;
	UINT ManualChannelChangeCount;
	UINT AutoStartupChannelChangeCount;
	UINT AutoUserChannelChangeCount;
	UINT AutoRefreshChannelChangeCount;
	UINT AutoDynamicChannelChangeCount;
	UINT AutoDFSChannelChangeCount;
	ULONG UnicastPacketsSent;
	ULONG UnicastPacketsReceived;
	ULONG MulticastPacketsSent;
	ULONG MulticastPacketsReceived;
	ULONG BroadcastPacketsSent;
	ULONG BroadcastPacketsReceived;
} WIFI_RADIO_STATS;

typedef enum _AUTH_MODE_T {
	AUTH_None = 0,
	AUTH_WEP,
	AUTH_WPA,
	AUTH_WPA2,
	AUTH_WPA_WPA2,
	AUTH_WPA_ENTERPRISE,
	AUTH_WPA2_ENTERPRISE,
	AUTH_WPA3_SAE,
	AUTH_WPA2_PSK_WPA3_SAE,
	AUTH_WPA3_ENTERPRISE,
	AUTH_END,
} AUTH_MODE;

typedef enum _ENCRY_MODE_T {
	ENCRY_TKIP = 0,
	ENCRY_AES,
	ENCRY_TKIPAES,
	ENCRY_CCMP256,
	ENCRY_GCMP128,
	ENCRY_GCMP256,
	ENCRY_BIP_CMAC128,
	ENCRY_BIP_CMAC256,
	ENCRY_BIP_GMAC128,
	ENCRY_BIP_GMAC256,
	ENCRY_BIP_NONE,
	ENCRY_UNKNOWN,
	ENCRY_END,
} ENCRY_MODE;

/* Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}. */
typedef struct _NBR_DIAG_RESULT {
	UCHAR index; /* index in scan table */
	UCHAR radio_name[16]; /* ra0/rax0/wlan0 */
	UCHAR ssid[SSID_LEN];
	UCHAR bssid[MAC_ADDR_LEN];
	UCHAR radio_mode; /* 0-adhoc, 1-infra */
	UINT16 Channel; /* neighbor channel */
	INT SignalStrength; /* neighbor rssi */
	UCHAR SecurityModeEnabled;
	/*	0: None
		1: WEP
		2: WPA
		3: WPA2
		4: WPA-WPA2
		5: WPA-Enterprise
		6: WPA2-Enterprise
		7: WPA-WPA2-Enterprise
		8: WPA3-SAE
		9: WPA2-PSK-WPA3-SAE
		10: WPA3-Enterprise
	upper layer shall adapt to string. */
	UCHAR EncryptionMode;
	/* 0: TKIP
	   1: AES
	   upper layer adapt to string. */
	UCHAR OperatingFrequencyBand;
	/* 0: 2.4GHz
	   1: 5GHz
	   2: 6GHz */
	UCHAR SupportedStandards[32];
	/* 2.4G: b,g,n,ax
	   5G: a,n,ac,ax
	   please refer to enum WIFI_MODE, the upper layer shall adapt to string. */
	UCHAR OperatingStandards[32];
	UCHAR OperatingChannelBandwidth;
	/* 20MHz
	   40MHz
	   80MHz
	   160MHz
	   Auto
	  please refer to enum oid_bw, the upper layer shall adapt to string. */
	UINT16 BeaconPeriod;
	INT16 Noise; /* Not Support, default = 0 */
	UINT16 DTIMPeriod;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES]; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
} NBR_DIAG_RESULT;

/* Device.WiFi.SSID.{i}.
   Device.WiFi.SSID.{i}.Stats. */
typedef struct _wifi_bss_stats {
	ULONG BytesSent;
	ULONG BytesReceived;
	ULONG PacketsSent;
	ULONG PacketsReceived;
	UINT ErrorsSent;
	UINT RetransCount; /* MT7915 Not Support, default: 0 */
	UINT FailedRetransCount; /* MT7915 Not Support, default: 0 */
	UINT RetryCount; /* MT7915 Not Support, default: 0 */
	UINT MultipleRetryCount; /* MT7915 Not Support, default: 0 */
	UINT ACKFailureCount; /* MT7915 Not Support, default: 0 */
	UINT AggregatedPacketCount; /* MT7915 Not Support, default: 0 */
	UINT ErrorsReceived;
	ULONG UnicastPacketsSent;
	ULONG UnicastPacketsReceived;
	UINT DiscardPacketsSent;
	UINT DiscardPacketsReceived;
	ULONG MulticastPacketsSent;
	ULONG MulticastPacketsReceived;
	ULONG BroadcastPacketsSent;
	ULONG BroadcastPacketsReceived;
	UINT UnknownProtoPacketsReceived; /* MT7915 Not Support, default: 0 */
	ULONG DiscardPacketsSentBufOverflow; /* MT7915 Not Support, default: 0 */
	ULONG DiscardPacketsSentNoAssoc; /* MT7915 Not Support, default: 0 */
	ULONG FragSent; /* MT7915 Not Support, default: 0 */
	ULONG SentNoAck; /* MT7915 Not Support, default: 0 */
	ULONG DupReceived; /* MT7915 Not Support, default: 0 */
	ULONG TooLongReceived; /* MT7915 Not Support, default: 0 */
	ULONG TooShortReceived; /* MT7915 Not Support, default: 0 */
	ULONG AckUcastReceived; /* MT7915 Not Support, default: 0 */
} WIFI_BSS_STATS;

/* Device.WiFi.AccessPoint.{i} */
 typedef struct _wifi_ap {
	BOOLEAN SSIDAdvertisementEnabled;
	UINT RetryLimit; /* MT7915 Not Support, default: 0 */
	BOOLEAN WMMCapability;
	BOOLEAN UAPSDCapability;
	UINT AssociatedDeviceNumberOfEntries; /* MT7915 Not Support, default: 0 */
	UINT MaxAssociatedDevices;
	BOOLEAN IsolationEnable;
	BOOLEAN MACAddressControlEnabled;
	UINT MaxAllowedAssociations;
} WIFI_AP;

/* Device.WiFi.AccessPoint.{i}.Security */

/* Device.WiFi.AccessPoint.{i}.WPS. */

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.Stats */
typedef struct _wifi_sta_txrx_stats {
	ULONG BytesSent;
	ULONG BytesReceived;
	ULONG PacketsSent;
	ULONG PacketsReceived;
	UINT ErrorsSent;
	UINT ErrorsReceived;
	UINT RetransCount; /* MT7915 Not Support, default: 0 */
	UINT FailedRetransCount; /* MT7915 Not Support, default: 0 */
	UINT RetryCount; /* MT7915 Not Support, default: 0 */
	UINT MultipleRetryCount;/* MT7915 Not Support, default: 0 */
} WIFI_STA_TXRX_STATS;

typedef struct _wifi_sta_stats {
	UCHAR MacAddr[MAC_ADDR_LEN];  /* Search key, original Address for repeater */
	WIFI_STA_TXRX_STATS stats;
	BOOLEAN valid;  /* if find mac, set to true, else set to false */
} WIFI_STA_STATS;

typedef struct _wifi_sta_txrx_rates {
	UINT TxRate;/* Mbps */
	UINT RxRate;/* Mbps */
	UINT RxRate_rt; /* recent 1sec rate, Mbps */
	UINT TxRate_rt; /* recent 1sec rate, Mbps */
	UINT avg_tx_rate;
	UINT avg_rx_rate; /* MT7915 Not Support, default: 0*/
} WIFI_STA_TXRX_RATES;


typedef struct _wifi_sta_rates {
	UCHAR MacAddr[MAC_ADDR_LEN];  /* Search key, original Address for repeater */
	WIFI_STA_TXRX_RATES rates;
	BOOLEAN valid;  /* if find mac, set to true, else set to false */
} WIFI_STA_RATES;

/* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}*/
typedef struct _wifi_station {
	UCHAR MacAddr[MAC_ADDR_LEN];  /* Search key, original Address for repeater */
	UCHAR VMacAddr[MAC_ADDR_LEN]; /* CurrentAddress for repeater */
	USHORT PhyMode; /* OperatingStandards */
	BOOLEAN AuthenticationState;
	UINT LastConnectTime;
	SHORT SignalStrength;
	SHORT Noise;
	UCHAR Retransmissions;/* Not Support, default: 0 */
	UINT UtilizationReceive;
	UINT UtilizationTransmit;
	WIFI_STA_TXRX_STATS stats;
	WIFI_STA_TXRX_RATES rates;
	BOOLEAN repeater;
	BOOLEAN valid;  /* if find mac, set to true, else set to false */
} WIFI_STATION;

/*Device.WiFi.AccessPoint.{i}.AC.{i}.
Device.WiFi.AccessPoint.{i}.AC.{i}.Stats.*/
/*Device.WiFi.EndPoint.{i}.AC.{i}.
Device.WiFi.EndPoint.{i}.AC.{i}.Stats.*/
typedef struct _wifi_dev_ac_stats {
	/*0: BE
	  1: BK
	  2: VI
	  3: VO*/
	UCHAR AccessCategory;
	ULONG BytesSent; /* MT7915 Not Support, default: 0 */
	ULONG BytesReceived; /* MT7915 Not Support, default: 0 */
	ULONG PacketsSent; /* MT7915 Not Support, default: 0 */
	ULONG PacketsReceived; /* MT7915 Not Support, default: 0 */
	UINT ErrorsSent; /* MT7915 Not Support, default: 0 */
	UINT ErrorsReceived; /* MT7915 Not Support, default: 0 */
	UINT DiscardPacketsSent; /* MT7915 Not Support, default: 0 */
	UINT DiscardPacketsReceived; /* MT7915 Not Support, default: 0 */
	UINT RetransCount; /* MT7915 Not Support, default: 0 */
} WIFI_DEV_AC_STATS;

/* Used for Get per AC Setting */
typedef struct _wifi_dev_ac {
	/*0: BE
	  1: BK
	  2: VI
	  3: VO*/
	UCHAR AccessCategory;
	UCHAR AIFSN;
	UCHAR ECWMin;
	UCHAR ECWMax;
	USHORT TxOpMax;
	BOOLEAN AckPolicy;
} WIFI_DEV_AC ;

/* Device.WiFi.AccessPoint.{i}.Accounting */
typedef struct _wifi_accounting_server {
	UINT ServerIPAddr[2]; /* 0: Primary server IP, 1: Secondary server IP */
	UINT ServerPort[2]; /* 0: Primary server Port, 1:  Secondary server Port */
	UCHAR Secret[2][64]; /* 0: Primary Server Secret, 1: Secondary Server Secret */
} WIFI_ACCOUNTING_SERVER;
#endif /* TR181_SUPPORT */

#ifdef TRACELOG_TCP_PKT
#define TCP_TRAFFIC_DATAPKT_MIN_SIZE	1000
static inline BOOLEAN RTMPIsTcpDataPkt(
	IN PNDIS_PACKET pPacket)
{
	UINT32 pktlen;

	pktlen = GET_OS_PKT_LEN(pPacket);

	if (pktlen > TCP_TRAFFIC_DATAPKT_MIN_SIZE)
		return TRUE;
	else
		return FALSE;
}
#define TCP_TRAFFIC_ACKPKT_SIZE		 54
static inline BOOLEAN RTMPIsTcpAckPkt(
	IN PNDIS_PACKET pPacket)
{
	UINT32 pktlen;

	pktlen = GET_OS_PKT_LEN(pPacket);

	if (pktlen == TCP_TRAFFIC_ACKPKT_SIZE)
		return TRUE;
	else
		return FALSE;
}
#endif /* TRACELOG_TCP_PKT */

#ifdef RT_BIG_ENDIAN
#include "hdev_ctrl.h"

/***************************************************************************
  *	Endian conversion related functions
  **************************************************************************/

#ifdef MT_MAC
static inline VOID mt_rmac_d0_endian_change(UINT32 *rxinfo)
{
	(*rxinfo) = SWAP32(*rxinfo);
}

static inline VOID mt_rmac_base_info_endian_change(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *rxinfo)
{
	int i;

	for (i = 1; i < 4; i++)
		*(((UINT32 *)rxinfo) + i) = SWAP32(*(((UINT32 *)rxinfo) + i));
}
#endif /* MT_MAC */

/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID RTMPWIEndianChange(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pData,
	IN ULONG DescriptorType)
{
	int size;
	int i;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 RXWISize = cap->RXWISize;

	size = ((DescriptorType == TYPE_TXWI) ? TXWISize : RXWISize);

	if (DescriptorType == TYPE_TXWI) {
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		/* Byte 0~3 */
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));	/* Byte 4~7 */

		if (size > 16)
			*((UINT32 *)(pData + 16)) = SWAP32(*((UINT32 *)(pData + 16)));	/* Byte 16~19 */
	} else {
		for (i = 0; i < size / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}

#ifdef MT_MAC
/*
	========================================================================

	Routine Description:
		Endian conversion of MacTxInfo/MacRxInfo descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame
		Length		Length of MacTxInfo/MacRxInfo

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID MTMacInfoEndianChange(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pData,
	IN ULONG DescriptorType,
	IN UINT16 Length)
{
	int i;

	if (DescriptorType == TYPE_TMACINFO) {
		for (i = 0; i < Length / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	} else { /* TYPE_RMACINFO */
		for (i = 1; i < Length / 4; i++) /* i from 1, due to 1st DW had endia change already, so skip it here. */
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
static inline VOID WriteBackToDescriptor(
	IN UCHAR *Dest,
	IN UCHAR *Src,
	IN BOOLEAN DoEncrypt,
	IN ULONG DescriptorType)
{
	UINT32 *p1, *p2;

	p1 = ((UINT32 *)Dest);
	p2 = ((UINT32 *)Src);
	*p1 = *p2;
	*(p1 + 2) = *(p2 + 2);
	/*new txd & rxd just have 3 word*/
	*(p1 + 3) = *(p2 + 3);
	/* +++Add by shiang for jeffrey debug */
#if defined(LINUX) || defined(VXWORKS)
	wmb();
#endif /* LINUX */
	/* ---Add by shiang for jeffrey debug */
	*(p1 + 1) = *(p2 + 1); /* Word 1; this must be written back last */
}
#endif /* RTMP_MAC_PCI */

/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
#ifdef RTMP_MAC_PCI
static inline VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescType)
{
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));			/* Byte 0~3 */
	*((UINT32 *)(pData + 8)) = SWAP32(*((UINT32 *)(pData + 8)));		/* Byte 8~11 */
	/*new txd & rxd just have 3 word*/
	*((UINT32 *)(pData + 12)) = SWAP32(*((UINT32 *)(pData + 12)));	/* Byte 12~15 */
	*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));		/* Byte 4~7, this must be swapped last */
}
#endif /* RTMP_MAC_PCI */

/*
	========================================================================

	Routine Description:
		Endian conversion of all kinds of 802.11 frames .

	Arguments:
		pAd	Pointer to our adapter
		pData			Pointer to the 802.11 frame structure
		Dir			Direction of the frame
		FromRxDoneInt	Caller is from RxDone interrupt

	Return Value:
		None

	Note:
		Call this function when read or update buffer data
	========================================================================
*/
static inline VOID	RTMPFrameEndianChange(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR			pData,
	IN	ULONG			Dir,
	IN	BOOLEAN		FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR	pMacHdr;

	/* swab 16 bit fields - Frame Control field */
	if (Dir == DIR_READ)
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;
	/* swab 16 bit fields - Duration/ID field */
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != FC_TYPE_CNTL) {
		/* swab 16 bit fields - Sequence Control field */
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if (pFrame->FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->FC.SubType) {
		case SUBTYPE_ASSOC_REQ:
		case SUBTYPE_REASSOC_REQ:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - Listen Interval field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_ASSOC_RSP:
		case SUBTYPE_REASSOC_RSP:
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(HEADER_802_11);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - Status Code field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - AID field */
			pMacHdr += 2;
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_AUTH:

			/* When the WEP bit is on, don't do the conversion here.
			This is only shared WEP can hit this condition.
			For AP, it shall do conversion after decryption.
			For STA, it shall do conversion before encryption. */
			if (pFrame->FC.Wep == 1)
				break;
			else {
				/* swab 16 bit fields - Auth Alg No. field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				/* swab 16 bit fields - Auth Seq No. field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				/* swab 16 bit fields - Status Code field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}

			break;

		case SUBTYPE_BEACON:
		case SUBTYPE_PROBE_RSP:
			/* swab 16 bit fields - BeaconInterval field */
			pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			/* swab 16 bit fields - CapabilityInfo field */
			pMacHdr += sizeof(USHORT);
			*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			break;

		case SUBTYPE_DEAUTH:
		case SUBTYPE_DISASSOC:

			/* If the PMF is negotiated, those frames shall be encrypted */
			if (!FromRxDoneInt && pFrame->FC.Wep == 1)
				break;
			else {
				/* swab 16 bit fields - Reason code field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
			}

			break;
		}

	} else if (pFrame->FC.Type == FC_TYPE_DATA) {
	} else if (pFrame->FC.Type == FC_TYPE_CNTL) {
		switch (pFrame->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ: {
			PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
			*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
			pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
		}
		break;

		case SUBTYPE_BLOCK_ACK:
			/* For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 */
			*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
			break;

		case SUBTYPE_ACK:
			/*For ACK packet, the HT_CONTROL field is in the same offset with Addr2 */
			*(UINT32 *)(&pFrame->Addr2[0]) =	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
			break;
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Frame Type!!!\n");

	/* swab 16 bit fields - Frame Control */
	if (Dir == DIR_WRITE)
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
}
/*
 ========================================================================

 Routine Description:
	 Endian conversion of normal data ,data type should be int or uint.

 Arguments:
	 pAd	 Pointer to our adapter
	 pData	 Pointer to data
	 size		 length of data

 Return Value:
	 None

========================================================================
*/

static inline VOID RTMPEndianChange(
	IN UCHAR *pData,
	IN UINT size)
{
	int i;

	if (size % 4)
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid DATA Type!!!\n");
	else {
		for (i = 0; i < size / 4; i++)
			*(((UINT32 *)pData) + i) = SWAP32(*(((UINT32 *)pData) + i));
	}
}
#endif /* RT_BIG_ENDIAN */

/***************************************************************************
  *	Other static inline function definitions
  **************************************************************************/
static inline VOID ConvertMulticastIP2MAC(
	IN PUCHAR pIpAddr,
	IN PUCHAR *ppMacAddr,
	IN UINT16 ProtoType)
{
	if (pIpAddr == NULL)
		return;

	if (ppMacAddr == NULL || *ppMacAddr == NULL)
		return;

	switch (ProtoType) {
	case ETH_P_IPV6:
		/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
		*(*ppMacAddr) = 0x33;
		*(*ppMacAddr + 1) = 0x33;
		*(*ppMacAddr + 2) = pIpAddr[12];
		*(*ppMacAddr + 3) = pIpAddr[13];
		*(*ppMacAddr + 4) = pIpAddr[14];
		*(*ppMacAddr + 5) = pIpAddr[15];
		break;

	case ETH_P_IP:
	default:
		/*			memset(*ppMacAddr, 0, MAC_ADDR_LEN); */
		*(*ppMacAddr) = 0x01;
		*(*ppMacAddr + 1) = 0x00;
		*(*ppMacAddr + 2) = 0x5e;
		*(*ppMacAddr + 3) = pIpAddr[1] & 0x7f;
		*(*ppMacAddr + 4) = pIpAddr[2];
		*(*ppMacAddr + 5) = pIpAddr[3];
		break;
	}

	return;
}

static inline VOID ConvertMulticastMAC2IP(
	IN PUCHAR pMacAddr,
	IN UINT16 ProtoType,
	IN UCHAR (*pIpAddr)[IPV6_ADDR_LEN])
{
	UCHAR ipv4_addr[IPV4_ADDR_LEN] = {0};
	UINT32 i = 0;

	if (pMacAddr == NULL)
		return;

	switch (ProtoType) {
	case ETH_P_IPV6:
		break;

	case ETH_P_IP:
	default:
		ipv4_addr[1] = pMacAddr[3];
		ipv4_addr[2] = pMacAddr[4];
		ipv4_addr[3] = pMacAddr[5];
		for (i = 0; i < 16; i++) {
			ipv4_addr[0] = 224 + i;
			memcpy(&pIpAddr[i][0] + 12, ipv4_addr, IPV4_ADDR_LEN);
		}

		ipv4_addr[1] |= 0x80; /* for the bit 24*/
		for (i = 0; i < 16; i++) {
			ipv4_addr[0] = 224 + i;
			memcpy(&pIpAddr[i + 16][0] + 12, ipv4_addr, IPV4_ADDR_LEN);
		}

		break;
	}

	return;
}

char *get_phymode_str(int phy_mode);
char *get_gi_str(int mode, int gi);

struct dev_type_name_map_t {
	INT type;
	RTMP_STRING prefix[IFNAMSIZ];
};

void rtmp_eeprom_of_platform(RTMP_ADAPTER *pAd);
NDIS_STATUS load_dev_l1profile(RTMP_ADAPTER *pAd);
UCHAR *get_dev_l2profile(RTMP_ADAPTER *pAd);
INT get_dev_config_idx(RTMP_ADAPTER *pAd);
UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type);
UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd);
UCHAR *get_bf_sku_path(RTMP_ADAPTER *pAd);
int ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);
NDIS_STATUS	RTMPPreReadProfile(RTMP_ADAPTER *pAd);
NDIS_STATUS	RTMPSetPreProfileParameters(RTMP_ADAPTER *pAd, RTMP_STRING *pBuffer);

INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace);

INT RTMPSetKeyParameter(
    IN RTMP_STRING *key,
    OUT CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer,
    IN BOOLEAN bTrimSpace);

INT RTMPAddKeyParameter(
    IN RTMP_STRING *key,
    OUT CHAR *value,
    IN INT destsize,
    IN RTMP_STRING *buffer);

INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT PUINT end_offset,
	IN  INT	 destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace);

#ifdef WSC_INCLUDED
VOID rtmp_read_wsc_user_parms_from_file(
	IN	RTMP_ADAPTER *pAd,
	IN	char *tmpbuf,
	IN	char *buffer);
#endif/*WSC_INCLUDED*/

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order);

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
void rtmp_read_vow_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

#ifdef GN_MIXMODE_SUPPORT
VOID gn_mixmode_is_enable(
	IN	PRTMP_ADAPTER pAd);
#endif /* GN_MIXMODE_SUPPORT */

#ifdef RED_SUPPORT
void rtmp_read_red_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
void rtmp_read_fq_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer);
#endif /* FQ_SCH_SUPPORT */

void rtmp_read_cp_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

void rtmp_read_multi_cli_nums_eap_th_parms_from_file(
	IN	PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

VOID cp_support_is_enabled(
	PRTMP_ADAPTER pAd);

INT set_cp_support_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg);

#ifdef PKTLOSS_CHK
INT set_pktloss_chk(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg);
#endif
INT set_mcli_cfg(IN  PRTMP_ADAPTER pAd, IN  RTMP_STRING * arg);
#ifdef KERNEL_RPS_ADJUST
void proc_rps_file_open(RTMP_ADAPTER *pAd);
void proc_rps_file_close(RTMP_ADAPTER *pAd);
void rtmp_read_kernel_rps_parms_from_file(
	IN      PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);
#endif
void rtmp_read_retry_parms_from_file(
	IN      PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);

NDIS_STATUS RTMPSetSkuParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPSetBackOffParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPResetSkuParam(RTMP_ADAPTER *pAd);
NDIS_STATUS RTMPResetBackOffParam(RTMP_ADAPTER *pAd);

VOID InitSkuRateDiffTable(RTMP_ADAPTER *pAd);
UCHAR GetSkuChannelBasePwr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR channel);
VOID UpdateSkuRatePwr(RTMP_ADAPTER *pAd, UCHAR ch, UCHAR bw, CHAR base_pwr);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMP_ReadRF(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			RegID,
	OUT	PUCHAR			pValue1,
	OUT PUCHAR			pValue2,
	IN	UCHAR			BitMask);

VOID RTMP_WriteRF(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value,
	IN	UCHAR			BitMask);

NDIS_STATUS	RT30xxWriteRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT30xxReadRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

NDIS_STATUS	RT635xWriteRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT635xReadRFRegister(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

BOOLEAN RTMPAdjustFrequencyOffset(
	IN RTMP_ADAPTER *pAd,
	INOUT PUCHAR pRefFreqOffset);
#endif /* RTMP_RF_RW_SUPPORT */

BOOLEAN RTMPCheckPhyMode(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 BandSupported,
	INOUT UCHAR *pPhyMode);

#ifdef RTMP_MAC_PCI
INT NICInitPwrPinCfg(RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_PCI */

VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd);

NDIS_STATUS NICInitializeAdapter(RTMP_ADAPTER *pAd);
NDIS_STATUS NICInitializeAsic(RTMP_ADAPTER *pAd);

VOID NICResetFromError(RTMP_ADAPTER *pAd);

VOID UserCfgExit(RTMP_ADAPTER *pAd);
VOID UserCfgInit(RTMP_ADAPTER *pAd);

VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd);

UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd);
UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx);

VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode);
VOID AsicTxCntUpdate(RTMP_ADAPTER *pAd, UINT16 Wcid, MT_TX_COUNTER *pTxInfo);

VOID RTMPZeroMemory(VOID *pSrc, ULONG Length);
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length);
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length);

VOID AtoH(RTMP_STRING *src, UCHAR *dest, int destlen);
UCHAR BtoH(char ch);

VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc, char *timer_name);
VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc);
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd);

VOID _RTMPInitTimer(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer,
	IN VOID *pTimerFunc,
	IN VOID *pData,
	IN BOOLEAN Repeat,
	IN	CHAR * timer_name);
#define RTMPInitTimer(_pAd, _pTimer, _pTimerFunc, _pData, _Repeat) _RTMPInitTimer(_pAd, _pTimer, _pTimerFunc, _pData, _Repeat, #_pTimer)

VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value);
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled);
VOID RTMPShowTimerList(RTMP_ADAPTER *pAd);

VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd);
VOID RTMPDisableRxTx(RTMP_ADAPTER *pAd);

VOID AntCfgInit(RTMP_ADAPTER *pAd);

VOID rtmp_init_hook_set(RTMP_ADAPTER *pAd);

/* */
/* prototype in action.c */
/* */
VOID ActHeaderInit(
	IN RTMP_ADAPTER *pAd,
	IN OUT HEADER_802_11 *pHdr80211,
	IN UCHAR *da,
	IN UCHAR *sa,
	IN UCHAR *bssid);

VOID ActionStateMachineInit(
	IN	RTMP_ADAPTER *pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID mlme_send_addba_resp(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID SendSMPSAction(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR smps);

#ifdef CONFIG_AP_SUPPORT
VOID SendBeaconRequest(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RECBATimerTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ORIBATimerTimeout(
	IN	RTMP_ADAPTER *pAd);

VOID SendRefreshBAR(
	IN	RTMP_ADAPTER *pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(
	IN RTMP_ADAPTER *pAd);

VOID RTMP_11N_D3_TimerRelease(
	IN RTMP_ADAPTER *pAd);

VOID SendBSS2040CoexistMgmtAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT16	Wcid,
	IN	UCHAR	apidx,
	IN	UCHAR	InfoReq);

VOID SendNotifyBWActionFrame(
	IN RTMP_ADAPTER *pAd,
	IN UINT16  Wcid,
	IN UCHAR apidx);

BOOLEAN ChannelSwitchSanityCheck(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT16  Wcid,
	IN	UCHAR  NewChannel,
	IN	UCHAR  Secondary);

VOID ChannelSwitchAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT16  Wcid,
	IN	UCHAR  Channel,
	IN	UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR  pDest);

VOID Update2040CoexistFrameAndNotify(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT16  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID Send2040CoexistAction(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT16  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID UpdateBssScanParm(
	IN RTMP_ADAPTER *pAd,
	IN OVERLAP_BSS_SCAN_IE APBssScan);
#endif /* DOT11N_DRAFT3 */

INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif /* DOT11_N_SUPPORT */
UCHAR get_regulatory_class(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode, struct wifi_dev *wdev);
UCHAR get_regulatory_class_for_newCh(RTMP_ADAPTER *pAd, UCHAR NewCh, USHORT PhyMode, struct wifi_dev *wdev);
INT get_operating_class_list(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode, struct wifi_dev *wdev, UCHAR *SuppClasslist, INT *Len);
VOID get_reg_class_list_for_6g(IN RTMP_ADAPTER *pAd, IN USHORT PhyMode, PUCHAR reg_class_value);

UCHAR get_channel_set_num(UCHAR *ChannelSet);

PUCHAR get_channelset_by_reg_class(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode);

BOOLEAN get_spacing_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode,
	OUT UCHAR *spacing);

BOOLEAN is_channel_in_channelset_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode,
	IN UCHAR Channel);


INT Set_Reg_Domain_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DOT11_HE_AX
BOOLEAN is_ru26_disable_channel(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode);
#endif /* DOT11_HE_AX */

VOID BarHeaderInit(
	IN	RTMP_ADAPTER *pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA);

VOID InsertActField(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode);

BOOLEAN QosBADataParse(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bAMSDU,
	IN PUCHAR p8023Header,
	IN UINT16	WCID,
	IN UCHAR	TID,
	IN USHORT Sequence,
	IN UCHAR DataOffset,
	IN USHORT Datasize,
	IN UINT   CurRxIndex);

BOOLEAN bar_process(
	RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	ULONG MsgLen,
	PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
	IN	RTMP_ADAPTER *pAd);

VOID HTIOTCheck(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR	 BatRecIdx);

INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE wdev_type, PNET_DEV if_dev,
				INT8 func_idx, VOID *func_dev, VOID *sys_handle);
VOID wdev_init_for_bound_wdev(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle);

INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

enum queue_mm;
INT wdev_ops_register(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
					  struct wifi_dev_ops *ops, UCHAR wmm_detect_method);
INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt, struct wifi_dev *wdev);
UCHAR get_frag_num(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
BOOLEAN check_if_fragment(struct wifi_dev *wdev, PNDIS_PACKET pPacket);
#ifdef TX_AGG_ADJUST_WKR
BOOLEAN tx_check_for_agg_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* TX_AGG_ADJUST_WKR */
VOID tx_bytes_calculate(RTMP_ADAPTER *pAd, TX_BLK *tx_blk);
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *Address);
#ifdef RT_CFG80211_SUPPORT
struct wifi_dev *WdevSearchByBssid(RTMP_ADAPTER *pAd, UCHAR *Address);
#endif
struct wifi_dev *wdev_search_by_omac_idx(RTMP_ADAPTER *pAd, UINT8 BssIndex);
struct wifi_dev *wdev_search_by_band_omac_idx(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 omac_idx);
struct wifi_dev *wdev_search_by_wcid(RTMP_ADAPTER *pAd, UINT16 wcid);
struct wifi_dev *wdev_search_by_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);
struct wifi_dev *wdev_search_by_idx(RTMP_ADAPTER *pAd, UINT32 idx);
struct wifi_dev *wdev_search_by_netdev(RTMP_ADAPTER *pAd, VOID *pDev);
VOID BssInfoArgumentLinker(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 GetBssInfoIdx(RTMP_ADAPTER *pAd);
INT32 wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID ReleaseBssIdx(RTMP_ADAPTER *pAd, UINT32 BssIdx);
VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID BssInfoArgumentLink(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo);
VOID wdev_if_up_down(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN if_up_down_state);
INT32 wdev_config_init(struct _RTMP_ADAPTER *pAd);
void wdev_sync_ch_by_rfic(struct _RTMP_ADAPTER *ad, UCHAR rfic, UCHAR ch);
void wdev_sync_prim_ch(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
#ifdef DOT11_N_SUPPORT
void wdev_sync_ht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ADD_HTINFO *add_ht_info);
#endif
#ifdef DOT11_VHT_AC
void wdev_sync_vht_bw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR bw, UINT8 channel);
#endif
BOOLEAN IS_SYNC_BW_POLICY_VALID(struct _RTMP_ADAPTER *pAd, BOOLEAN isHTPolicy, UCHAR policy, UCHAR band_idx);
#endif

#ifdef IP_ASSEMBLY

typedef union ip_flags_frag_offset {
	struct {
#ifdef RT_BIG_ENDIAN
		USHORT flags_reserved:1;
		USHORT flags_may_frag:1;
		USHORT flags_more_frag:1;
		USHORT frag_offset:13;
#else
		USHORT frag_offset:13;
		USHORT flags_more_frag:1;
		USHORT flags_may_frag:1;
		USHORT flags_reserved:1;
#endif
	} field;
	USHORT word;
} IP_FLAGS_FRAG_OFFSET;

typedef struct ip_v4_hdr {
#ifdef RT_BIG_ENDIAN
	UCHAR version:4, ihl:4;
#else
	UCHAR ihl:4, version:4;
#endif
	UCHAR tos;
	USHORT tot_len;
	USHORT identifier;
} IP_V4_HDR;

typedef struct ip_assemble_data {
	DL_LIST list;
	QUEUE_HEADER queue;
	INT32 identify;
	INT32 fragSize;
	ULONG createTime;
} IP_ASSEMBLE_DATA;

INT rtmp_IpAssembleHandle(RTMP_ADAPTER *pAd, STA_TR_ENTRY *pTrEntry, PNDIS_PACKET pPacket, UCHAR queIdx, PACKET_INFO packetInfo);
#endif

VOID ba_ori_session_start(
	IN RTMP_ADAPTER *pAd,
	IN STA_TR_ENTRY *tr_entry,
	IN UINT8 UserPriority);


INT rtmp_tx_burst_set(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID enable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level);
VOID disable_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					  UINT8 ac_type, UINT8 prio, UINT16 level);
UINT8 query_tx_burst_prio(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT TxOPUpdatingAlgo(RTMP_ADAPTER *pAd);

VOID mt_detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR UserPriority, UCHAR FlgIsOutput);
VOID rx_802_3_data_frm_announce(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev);

VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev);

INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev);
INT rx_chk_amsdu_invalid_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev);
INT rx_chk_duplicate_mgmt_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID mt_dynamic_wmm_be_tx_op(
	IN RTMP_ADAPTER *pAd,
	IN ULONG nonBEpackets);

NDIS_STATUS MlmeHardTransmit(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET	pPacket,
	IN	BOOLEAN			FlgDataQForce,
	IN	BOOLEAN			FlgIsLocked,
	IN	BOOLEAN			FlgIsCheckPS);

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET	pPacket);

#ifdef RTMP_MAC_PCI
NDIS_STATUS MlmeHardTransmitTxRing(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);

NDIS_STATUS MlmeDataHardTransmit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);

#endif /* RTMP_MAC_PCI */

USHORT RTMPCalcDuration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Rate,
	IN ULONG Size);

VOID mt_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING *pTransmit);

VOID write_tmac_info_offload_pkt(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR type,
	UCHAR sub_type,
	UCHAR *tmac_buf,
	HTTRANSMIT_SETTING *BeaconTransmit,
	ULONG frmLen);

#ifdef RANDOM_PKT_GEN
VOID random_write_qidx(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk);
#endif

VOID RTMPSuspendMsduTransmission(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID RTMPResumeMsduTransmission(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

NDIS_STATUS MiniportMMRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN UCHAR *pData,
	IN UINT Length);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
VOID send_dummy_data_packet(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pEntry);
#endif
VOID LowPowerDebug(
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID RTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UCHAR TxRate,
	IN BOOLEAN bQosNull,
	IN USHORT PwrMgmt);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPOffloadPm(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState);
VOID RTMPWakeUpWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* #ifdef CONFIG_STA_SUPPORT */


#ifdef MT_MAC
UINT32 pkt_alloc_fail_handle(RTMP_ADAPTER *ad, PNDIS_PACKET rx_packet, UINT8 resource_idx);
UINT32 mt_rx_pkt_process(RTMP_ADAPTER *, UINT8 resource_idx, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket);
#endif /* MT_MAC */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
BOOLEAN is_looping_packet(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET * pPacket);

VOID set_wf_fwd_cb(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev);

#endif /* CONFIG_WIFI_PKT_FWD */

BOOLEAN is_udp_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);
#ifdef TXRX_STAT_SUPPORT
BOOLEAN RTMPGetUserPriority(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx);

#endif

BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN struct wifi_dev *wdev);

VOID RTMPCckBbpTuning(
	IN	RTMP_ADAPTER *pAd,
	IN	UINT			TxRate);

VOID RTMPCheckDhcpArp(
	IN PNDIS_PACKET pkt);


/*
	MLME routines
*/

/* Asic/RF/BBP related functions */
VOID AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER			pAd,
	IN PULONG					TxPwr);

VOID AsicExtraPowerOverMAC(RTMP_ADAPTER *pAd);

#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN		RTMP_ADAPTER *pAd,
	IN		PCHAR			pTotalDeltaPower,
	INOUT	PULONG			pSingleSKUTotalDeltaPwr,
	INOUT	PUCHAR			pSingleSKUBbpR1Offset);
#endif /* SINGLE_SKU*/

VOID AsicPercentageDeltaPower(
	IN		PRTMP_ADAPTER		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1);

VOID AsicCompensatePowerViaBBP(
	IN RTMP_ADAPTER *pAd,
	INOUT CHAR *pTotalDeltaPower);

VOID AsicAdjustTxPower(RTMP_ADAPTER *pAd);

#define WMM_PARAM_TXOP	0
#define WMM_PARAM_AIFSN	1
#define WMM_PARAM_CWMIN	2
#define WMM_PARAM_CWMAX	3
#define WMM_PARAM_ALL		4

#define WMM_PARAM_AC_0		0
#define WMM_PARAM_AC_1		1
#define WMM_PARAM_AC_2		2
#define WMM_PARAM_AC_3		3

#ifdef RTMP_RBUS_SUPPORT
int RtmpAsicSendCommandToSwMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN FlgIsNeedLocked);
#endif /* RTMP_RBUS_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
UINT32 StreamModeRegVal(
	IN RTMP_ADAPTER *pAd);

/*
	Update the Tx chain address
	Parameters
		pAd: The adapter data structure
		pMacAddress: The MAC address of the peer STA

	Return Value:
		None
*/
VOID AsicUpdateTxChainAddress(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddress);

INT Set_StreamMode_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);

INT Set_StreamModeMac_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);

INT Set_StreamModeMCS_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg);
#endif /* STREAM_MODE_SUPPORT */

VOID MacAddrRandomBssid(
	IN  RTMP_ADAPTER *pAd,
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid);

VOID MgtMacHeaderInitExt(
	IN  RTMP_ADAPTER *pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pSA,
	IN PUCHAR pBssid);

VOID MlmeRadioOff(
	IN RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID MlmeRadioOn(
	IN RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

VOID MlmeLpEnter(
	IN RTMP_ADAPTER *pAd);

VOID MlmeLpExit(
	IN RTMP_ADAPTER *pAd);

VOID BssTableInit(
	IN BSS_TABLE *Tab);

ULONG BssTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR pBssid,
	IN UCHAR Channel);

UINT BssNumByChannel(
	IN BSS_TABLE * Tab,
	IN UCHAR Channel);

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	pBssid,
	IN PUCHAR	pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	Bssid,
	IN PUCHAR	pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel);

ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen);

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab,
	IN	  PUCHAR pBssid,
	IN	  UCHAR Channel);

ULONG BssTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

/* fix memory leak when trigger scan continuously */
VOID BssEntryReset(
	IN struct _BSS_TABLE *Tab,
	IN OUT struct _BSS_ENTRY *pBss);

VOID BssEntryCopy(
	IN struct _BSS_TABLE *TabDst,
	OUT struct _BSS_ENTRY *pBssDst,
	IN struct _BSS_ENTRY *pBssSrc);
VOID BssEntrySet(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	OUT BSS_ENTRY *pBss,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
VOID BATableInsertEntry(
	IN	RTMP_ADAPTER *pAd,
	IN USHORT Aid,
	IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
	IN UCHAR TID,
	IN UINT16 BAWinSize,
	IN UCHAR OriginatorStatus,
	IN BOOLEAN IsRecipient);

BOOLEAN bss_coex_insert_effected_ch_list(
	RTMP_ADAPTER *pAd,
	UCHAR Channel,
	BCN_IE_LIST *ie_list,
	struct wifi_dev *wdev);

#ifdef DOT11N_DRAFT3
BOOLEAN build_trigger_event_table(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN BCN_IE_LIST *ie_list);

VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID  TriEventInit(
	IN	RTMP_ADAPTER *pAd);

INT TriEventTableSetEntry(
	IN	RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN PUCHAR pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN BOOLEAN			has_ht_cap,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID BssTableSsidSort(
	IN  RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *OutTab,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder);

NDIS_STATUS  MlmeQueueInit(
	IN RTMP_ADAPTER *pAd);

VOID  MlmeQueueDestroy(
	IN struct _MLME_STRUCT *mlme);

VOID MlmeQueueFreeSpinLock(
	struct _MLME_STRUCT *mlme);

#ifdef MLME_MULTI_QUEUE_SUPPORT
VOID* MlmeQueueSelByMach(
       IN RTMP_ADAPTER *pAd,
       IN ULONG Machine,
       IN ULONG MsgType,
       IN VOID *pHeader);
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

BOOLEAN MlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv);

BOOLEAN MlmeEnqueueWithWdev(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv,
	IN struct wifi_dev *wdev);

BOOLEAN MlmeEnqueueForRecv(
	IN  RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN struct raw_rssi_info *rssi_info,
	IN ULONG MsgLen,
	IN PVOID Msg,
	IN UCHAR OpMode,
	IN struct wifi_dev *wdev,
	IN UCHAR RxPhyMode);

#ifdef WSC_INCLUDED
BOOLEAN MlmeEnqueueForWsc(
	IN RTMP_ADAPTER *pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN struct wifi_dev *wdev);
#endif /* WSC_INCLUDED */

BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue,
	OUT MLME_QUEUE_ELEM **Elem);

VOID	MlmeRestartStateMachine(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

VOID MlmeResetByWdev(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue,
	IN UCHAR SendId);

BOOLEAN  MsgTypeSubst(
	IN RTMP_ADAPTER *pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID StateMachineInit(
	IN STATE_MACHINE *Sm,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	ULONG Msg,
	IN STATE_MACHINE_FUNC F);

VOID StateMachineSetMsgChecker(
	IN STATE_MACHINE *S,
	IN STATE_MACHINE_MSG_CHECKER MsgCheckFun);

VOID StateMachinePerformAction(
	IN  RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState);

VOID Drop(
	IN  RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID AssocStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ReassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID DisassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

/*---------------------------------------------- */
VOID MlmeDisassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeAssocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerDisassocAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  ReassocTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  Cls3errAction(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr);

VOID  InvalidStateWhenAssoc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);


VOID  ComposePsPoll(
	IN	RTMP_ADAPTER *pAd,
	IN	PPSPOLL_FRAME pPsPollFrame,
	IN	USHORT	Aid,
	IN	UCHAR *pBssid,
	IN	UCHAR *pTa);

VOID ComposeNullFrame(
	RTMP_ADAPTER *pAd,
	PHEADER_802_11 pNullFrame,
	UCHAR *pAddr1,
	UCHAR *pAddr2,
	UCHAR *pAddr3);

VOID  AssocPostProc(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr2,
	IN  USHORT CapabilityInfo,
	IN  USHORT Aid,
	IN  UCHAR SupRate[],
	IN  UCHAR SupRateLen,
	IN  UCHAR ExtRate[],
	IN  UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	struct _IE_lists *ie_list,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN MAC_TABLE_ENTRY *pEntry);

VOID AuthStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN PSTATE_MACHINE sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeAuthReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
	IN  struct wifi_dev *wdev,
	IN  PUCHAR pAddr);

VOID MlmeDeauthReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

/*============================================= */

VOID AuthRspStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  PSTATE_MACHINE Sm,
	IN  STATE_MACHINE_FUNC Trans[]);

VOID PeerDeauthAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAuthSimpleRspGenAndSend(
	IN  RTMP_ADAPTER *pAd,
	IN  PHEADER_802_11  pHdr80211,
	IN  USHORT Alg,
	IN  USHORT Seq,
	IN  USHORT Reason,
	IN  USHORT Status);

BOOLEAN PeerProbeReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PEER_PROBE_REQ_PARAM * Param);

/*======================================== */

VOID SyncStateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeScanReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenScan(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID EnqueueProbeRequest(
	IN RTMP_ADAPTER *pAd);

BOOLEAN ScanRunning(
	IN RTMP_ADAPTER *pAd);
/*========================================= */


#ifdef WIFI_MD_COEX_SUPPORT
INT SendApccci2fwMsg(
	IN RTMP_ADAPTER *pAd,
	IN struct _MT_WIFI_COEX_APCCCI2FW *apccci2fw_msg);
#endif /* WIFI_MD_COEX_SUPPORT */

INT32 EnableNF(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 isEnable,
	IN UINT8 u1TimeOut,
	IN UINT8 u1Count,
	IN UINT8 u1EventCount);

INT SetEnableNf(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

VOID MlmeCntlMachinePerformAction(
	IN  RTMP_ADAPTER *pAd,
	IN  STATE_MACHINE *S,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlIdleProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidScanProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidSsidProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidRTBssidProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlMlmeRoamingProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitDisassocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitJoinProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitReassocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitStartProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc2(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAssocProc(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID LinkUp(
	RTMP_ADAPTER *pAd,
	UCHAR BssType,
	struct wifi_dev *wdev,
	UINT link_up_type,
	MLME_QUEUE_ELEM *Elem);

VOID LinkDown(
	RTMP_ADAPTER *pAd,
	UINT linkdown_type,
	struct wifi_dev *wdev,
	MLME_QUEUE_ELEM *Elem);

VOID IterateOnBssTab(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

VOID IterateOnBssTab2(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

VOID AssocParmFill(
	IN  RTMP_ADAPTER *pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT * AssocReq,
	IN  PUCHAR pAddr,
	IN  USHORT CapabilityInfo,
	IN  ULONG Timeout,
	IN  USHORT ListenIntv);

VOID DisassocParmFill(
	IN  RTMP_ADAPTER *pAd,
	IN  OUT VOID *DisassocReq, /* snowpin for cntl mgmt */
	IN  PUCHAR pAddr,
	IN  USHORT Reason);

VOID EnqueuePsPoll(
	IN  RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID EnqueueBeaconFrame(
	IN  RTMP_ADAPTER *pAd);

VOID MlmeJoinReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeScanReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeStartReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeForceJoinReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeForceScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ScanTimeoutAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID BeaconTimeoutAtJoinAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtScanAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtJoinAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerProbeReqAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
#define IS_VENDOR10_RSSI_VALID(_wdev) \
	(_wdev->isRssiEnbl == TRUE)
#define SET_VENDOR10_RSSI_VALID(_wdev, enable) \
	(_wdev->isRssiEnbl = enable)

VOID Vendor10RssiUpdate(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN isBcn,
	IN INT RealRssi);
#endif

#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT
VOID WidiSendProbeRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *destMac,
	IN UCHAR ssidLen,
	IN CHAR *ssidStr,
	IN UCHAR *deviceName,
	IN UCHAR *primaryDeviceType,
	IN UCHAR *vendExt,
	IN USHORT vendExtLen,
	IN UCHAR channel);
#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */

BOOLEAN MlmeScanReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT UCHAR *BssType,
	OUT CHAR ssid[],
	OUT UCHAR *SsidLen,
	OUT UCHAR *ScanType);

BOOLEAN PeerBeaconAndProbeRspSanity(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN UCHAR  MsgChannel,
	OUT BCN_IE_LIST *ie_list,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE,
	IN BOOLEAN bGetDtim,
	IN BOOLEAN bFromBeaconReport
);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	IN OVERLAP_BSS_SCAN_IE * BssScan,
	OUT BCN_IE_LIST * ie_list,
	OUT UCHAR	*RegClass);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

BOOLEAN PeerAddBAReqActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen);

BOOLEAN PeerAddBARspActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN VOID *pMsg,
	IN ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *CapabilityInfo,
	OUT ULONG *Timeout,
	OUT USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(
	IN  RTMP_ADAPTER *pAd,
#ifdef CONFIG_STA_SUPPORT
	IN struct wifi_dev *wdev,
#endif
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *Timeout,
	OUT USHORT *Alg);

BOOLEAN MlmeStartReqSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT CHAR Ssid[],
	OUT UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *Alg,
	OUT USHORT *Seq,
	OUT USHORT *Status,
	OUT CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(
	IN  struct wifi_dev *wdev,
	IN  VOID *pMsg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR *pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR *pCkipFlag,
	struct _IE_lists *ie_list);

BOOLEAN PeerDisassocSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *Reason);

BOOLEAN PeerDeauthSanity(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pAddr3,
	OUT USHORT *Reason);

BOOLEAN GetTimBit(
	IN  CHAR *Ptr,
	IN  USHORT Aid,
	OUT UCHAR *TimLen,
	OUT UCHAR *BcastFlag,
	OUT UCHAR *DtimCount,
	OUT UCHAR *DtimPeriod,
	OUT UCHAR *MessageToMe);

UCHAR PSC_Ch_Check(
	IN UCHAR Ch);

UCHAR SwitchChSanityCheckByWdev(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR newCh);

UCHAR ChannelSanity(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	IN BSS_ENTRY *pBss);

BOOLEAN MlmeDelBAReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen);

BOOLEAN mlme_addba_resp_sanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *Msg,
	IN ULONG MsgLen);

ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer,
	OUT ULONG *Length, ...);

UCHAR RandomByte(
	IN  RTMP_ADAPTER *pAd);

UCHAR RandomByte2(
	IN  RTMP_ADAPTER *pAd);

VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID  MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID STAMlmePeriodicExec(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID sta_2040_coex_scan_check(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

VOID MlmeAutoScan(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeAutoReconnectLastSSID(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

BOOLEAN MlmeValidateSSID(
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen);

VOID MlmeCheckForRoaming(
	IN RTMP_ADAPTER *pAd,
	IN ULONG	Now32);

BOOLEAN MlmeCheckForFastRoaming(
	IN  RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeCalculateChannelQuality(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now);

VOID MlmeCheckPsmChange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeSetPsmBit(
	IN RTMP_ADAPTER *pAd,
	IN PSTA_ADMIN_CONFIG pStaCfg,
	IN USHORT psm);

VOID MlmeSetTxPreamble(
	IN RTMP_ADAPTER *pAd,
	IN USHORT TxPreamble);

VOID UpdateBasicRateBitmap(
	IN	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

VOID MlmeUpdateTxRates(
	IN RTMP_ADAPTER *pAd,
	IN	BOOLEAN			bLinkUp,
	IN	UCHAR			apidx);

VOID MlmeUpdateTxRatesWdev(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, struct wifi_dev *wdev);

#ifdef DOT11_N_SUPPORT
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
VOID MlmeUpdateVhtTxRates(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, struct wifi_dev *wdev);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */


/* check the rate is legal according to phy mode, and save them in MLME AUX ctrl block */
void check_legacy_rates(
	struct legacy_rate *rate,
	struct legacy_rate *mlme_rate,
	struct wifi_dev *wdev);

VOID	RTMPCheckRates(
	IN OUT  UCHAR		   SupRate[],
	IN OUT  UCHAR		   *SupRateLen,
	IN	  UCHAR PhyMode);

BOOLEAN RTMPCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	INOUT HT_CAPABILITY_IE *pHtCapability,
	INOUT ADD_HT_INFO_IE *pAddHtInfo);

BOOLEAN check_ht(
	struct _RTMP_ADAPTER *ad,
	UINT16 wcid,
	struct common_ies *cmm_ies
	);

#ifdef DOT11_VHT_AC
BOOLEAN RTMPCheckVht(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE * vht_op);

BOOLEAN check_vht(
	struct _RTMP_ADAPTER *ad,
	UINT16 wcid,
	struct common_ies *cmm_ies);
#endif /* DOT11_VHT_AC */

VOID RTMPUpdateMlmeRate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

CHAR RTMPMaxRssi(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

CHAR RTMPMaxRssi2(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2,
	IN CHAR				Rssi3);

CHAR RTMPMinRssi(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2,
	IN CHAR				Rssi3);

CHAR RTMPAvgRssi(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE		*pRssi);

CHAR rtmp_avg_rssi(
	RTMP_ADAPTER *pad,
	RSSI_SAMPLE *prssi);


CHAR RTMPMinSnr(
	IN RTMP_ADAPTER *pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1);

#ifdef MICROWAVE_OVEN_SUPPORT
INT Set_MO_FalseCCATh_Proc(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING *arg);

#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
INT set_eFuseGetFreeBlockCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFusedump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFuseLoadFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

UCHAR eFuseReadRegisters(RTMP_ADAPTER *pAd, USHORT Offset, UINT16 Length, UINT16 *pData);
VOID EfusePhysicalReadRegisters(RTMP_ADAPTER *pAd, USHORT Offset, USHORT Length, USHORT *pData);

#ifdef CONFIG_ATE
INT Set_LoadEepromBufferFromEfuse_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_eFuseBufferModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_BinModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_ATE */

VOID  rtmp_ee_load_from_efuse(RTMP_ADAPTER *pAd);

#endif /* RTMP_EFUSE_SUPPORT */

VOID AsicEvaluateRxAnt(RTMP_ADAPTER *pAd);

VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

UCHAR RTMPStaFixedTxMode(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPUpdateLegacyTxSetting(
	IN UCHAR fixed_tx_mode,
	IN PMAC_TABLE_ENTRY	pEntry);

BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd);

VOID MlmeHalt(RTMP_ADAPTER *pAd);
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd);
VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd);

VOID BuildChannelList(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef CONFIG_6G_SUPPORT
VOID build_PSC_scan_channel_list(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
VOID add_non_psc_channel(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR channel);
#endif

UCHAR ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
BOOLEAN ApAutoChannelSkipListBuild(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);
#endif
UCHAR FirstChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR FirstNonDfsChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef WIFI_MD_COEX_SUPPORT
UCHAR FirstSafeChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif
UCHAR NextChannel(
	RTMP_ADAPTER *pAd,
	SCAN_CTRL *ScanCtrl,
	UCHAR channel, struct wifi_dev *wdev);
UCHAR FindScanChannel(RTMP_ADAPTER *pAd, UINT8 LastScanChannel, struct wifi_dev *wdev);
UCHAR scan_find_next_channel(RTMP_ADAPTER *pAd, SCAN_CTRL *ScanCtrl, UINT8 LastScanChannel);
INT scan_partial_init(RTMP_ADAPTER *pAd);
VOID scan_partial_trigger_checker(RTMP_ADAPTER *pAd);
INT scan_release_mem(struct _RTMP_ADAPTER *ad);
#ifdef OFFCHANNEL_ZERO_LOSS
UINT32 diffu32(UINT32 New, UINT32 Old);
void read_channel_stats(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat);
void update_scan_channel_stats(RTMP_ADAPTER *pAd, UINT8 BandIdx, SCAN_CTRL *ScanCtrl);
#endif
VOID ChangeToCellPowerLimit(RTMP_ADAPTER *pAd, UCHAR PowerLimit);

VOID	RTMPInitMICEngine(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pKey,
	IN  PUCHAR		  pDA,
	IN  PUCHAR		  pSA,
	IN  UCHAR		   UserPriority,
	IN  PUCHAR		  pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pSrc,
	IN  PUCHAR		  pDA,
	IN  PUCHAR		  pSA,
	IN  PUCHAR		  pMICKey,
	IN	UCHAR			UserPriority,
	IN  UINT			Len);

VOID	RTMPCalculateMICValue(
	IN  RTMP_ADAPTER *pAd,
	IN  PNDIS_PACKET	pPacket,
	IN  PUCHAR pEncap,
	IN	PUCHAR	pKey,
	IN	PUCHAR	pMIC,
	IN	UCHAR apidx);

VOID	RTMPTkipAppendByte(TKIP_KEY_INFO *pTkip, UCHAR uChar);
VOID	RTMPTkipAppend(TKIP_KEY_INFO *pTkip, UCHAR *pSrc, UINT nBytes);
VOID RTMPTkipGetMIC(TKIP_KEY_INFO *pTkip);


INT RT_CfgSetCountryRegion(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg,
	IN INT band);

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg, struct wifi_dev *wdev);

/*update phy mode for all of wdev*/
VOID RtmpUpdatePhyMode(RTMP_ADAPTER *pAd, USHORT *pWmode);
RT_802_11_PHY_MODE wmode_2_cfgmode(USHORT wmode);
USHORT cfgmode_2_wmode(UCHAR cfg_mode);
BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, USHORT *wmode);
UCHAR *wmode_2_str(USHORT wmode);

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR BandIdx);

INT	RT_CfgSetWepKey(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx);

INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg);
INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx, INT opmode);
INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate);
INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl);
#endif /* WSC_INCLUDED */

INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef MT_MAC

INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_thermal_dbg_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fw_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_isr_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txop_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_rts_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ser(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef RTMP_PCI_SUPPORT
INT set_rxd_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT SetManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPThreshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPUpBound(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetManualTxOPLowBound(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT setTmrEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT setTmrVerProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT SetTmrCalProc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif /* MT_MAC */
#ifdef INTERFACE_SPEED_DETECT
BOOLEAN set_interface_speed(RTMP_ADAPTER *pAd, UINT32 speed);
#endif /* INTERFACE_SPEED_DETECT */

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

#ifdef ZERO_LOSS_CSA_SUPPORT
INT Set_WcidSkipTx_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg);

INT Set_ApScanTime_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_APChannelList_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg);

INT Set_CSATriggerCount_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg);

INT Set_ZeroPktLossEnable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg);

INT Set_CsaActionFrameEnable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_StaPsQLimit_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg);

INT Set_MacTxEnable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING	*arg);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT set_tx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg);

#ifdef CUT_THROUGH
INT set_token_setting(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

INT set_tx_deq_cpu(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_tx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_ba_dbg(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx_cnt_io_thd(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_rx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg);

INT set_tx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg);

INT SetLoadFwMethod(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT SetTrigCoreDump(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg);

INT set_fwcmd_timeout_print_cnt(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT show_fwcmd_timeout_info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

#ifdef MAP_R2
INT show_traffic_separation_info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
#endif

#ifdef CONFIG_6G_SUPPORT
INT show_bssmnger_info(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg);

BOOLEAN in_band_radioinfo_update(struct wifi_dev *wdev);

BOOLEAN in_band_discovery_update(struct wifi_dev *wdev,
		UCHAR type, UCHAR interval, UCHAR tx_mode, UCHAR by_cfg);

BOOLEAN qos_injector_update(struct wifi_dev *wdev,
		UCHAR interval, UCHAR state);

BOOLEAN out_band_discovery_update(struct wifi_dev *wdev,
		UCHAR rnr2g, UCHAR rnr5g, UCHAR rnr6g);
#endif

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
void set_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd,
	BOOLEAN flag_pcie_aspm_dym_ctrl);

BOOLEAN get_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef CONFIG_DVT_MODE
INT16 i2SetDvt(RTMP_ADAPTER *pAd, RTMP_STRING *pArg);
#endif /* CONFIG_DVT_MODE */

NDIS_STATUS RTMPWPARemoveKeyProc(
	IN  RTMP_ADAPTER *pAd,
	IN  PVOID		   pBuf);

VOID RTMPWPARemoveAllKeys(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev);

BOOLEAN RTMPCheckStrPrintAble(
	IN  CHAR *pInPutStr,
	IN  UCHAR strLen);

VOID RTMPSetDefaultChannel(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPUpdateRateInfo(
	USHORT phymode,
	struct dev_rate_info *rate
);

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
VOID rtmpeapupdaterateinfo(
	USHORT phymode,
	struct dev_rate_info *rate,
	struct dev_eap_info *eap
);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef GN_MIXMODE_SUPPORT
VOID RTMPUpdateGNRateInfo(
	USHORT phymode,
	struct dev_rate_info *rate
);
#endif /*GN_MIXMODE_SUPPORT*/

VOID RTMPSetPhyMode(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN  USHORT phymode);

VOID RTMPUpdateHTIE(
	IN		UCHAR				*pMcsSet,
	IN struct wifi_dev *wdev,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE *pAddHtInfo);

VOID RTMPAddWcidAttributeEntry(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR			BssIdx,
	IN	UCHAR			KeyIdx,
	IN	UCHAR			CipherAlg,
	IN	MAC_TABLE_ENTRY *pEntry);

INT set_assign_wcid_proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

RTMP_STRING *GetEncryptType(CHAR enc);
RTMP_STRING *GetAuthMode(CHAR auth);

VOID MacTableSetEntryRaCap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, struct _vendor_ie_cap *vendor_ie);
#ifdef DOT11_VHT_AC
VOID MacTableEntryCheck2GVHT(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent);
#endif /* DOT11_VHT_AC */

VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef SYSTEM_LOG_SUPPORT
VOID RtmpDrvSendWirelessEvent(
	IN	VOID			*pAdSrc,
	IN	USHORT			Event_flag,
	IN	PUCHAR			pAddr,
	IN  UCHAR			wdev_idx,
	IN	CHAR			Rssi);
#else
#define RtmpDrvSendWirelessEvent(_pAd, _Event_flag, _pAddr, wdev_idx, _Rssi)
#endif /* SYSTEM_LOG_SUPPORT */

CHAR ConvertToRssi(
	IN RTMP_ADAPTER *pAd,
	IN struct raw_rssi_info *rssi_info,
	IN UCHAR rssi_idx);

CHAR ConvertToSnr(RTMP_ADAPTER *pAd, UCHAR Snr);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN AdjustBwToSyncAp(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list, struct wifi_dev *wdev);
#endif

#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);


VOID CntlChannelWidth(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			PrimaryChannel,
	IN UCHAR			CentralChannel,
	IN UCHAR			ChannelWidth,
	IN UCHAR			SecondaryChannelOffset);

#endif /* DOT11N_DRAFT3 */

VOID APAsicEvaluateRxAnt(
	IN RTMP_ADAPTER *pAd);


VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *PACInquiry(RTMP_ADAPTER *pAd, UINT16 Wcid);

VOID HandleCounterMeasure(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY  *pEntry);

VOID WPAStart4WayHS(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN ULONG			TimeInterval);

VOID WPAStart2WayGroupHS(
	IN  RTMP_ADAPTER *pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID RTMPHandleSTAKey(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM  *Elem);

VOID MlmeDeAuthAction(
	IN  RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT		   Reason,
	IN  BOOLEAN		  bDataFrameFirst);

#ifdef DOT11W_PMF_SUPPORT
VOID PMF_SAQueryTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID PMF_SAQueryConfirmTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* DOT11W_PMF_SUPPORT */

VOID AES_128_CMAC(
	IN	PUCHAR	key,
	IN	PUCHAR	input,
	IN	INT		len,
	OUT	PUCHAR	mac);

#ifdef DOT1X_SUPPORT
VOID	WpaSend(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pPacket,
	IN  ULONG		   Len);

INT RTMPAddPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *PMKID,
	IN UCHAR *PMK,
	IN UCHAR is_ft,
	IN UINT8 pmk_len);

INT RTMPSearchPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR is_ft);

INT RTMPSearchPMKIDCacheByPmkId(
	IN NDIS_AP_802_11_PMKID * pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPmkId);

INT RTMPValidatePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPMKID);

VOID RTMPDeletePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN INT idx);

VOID RTMPMaintainPMKIDCache(
	IN RTMP_ADAPTER *pAd);

UCHAR is_rsne_pmkid_cache_match(
	IN UINT8 *rsnie,
	IN UINT	rsnie_len,
	IN NDIS_AP_802_11_PMKID * pmkid_cache,
	IN INT apidx,
	IN UCHAR *addr,
	OUT INT *cacheidx);
#else
#define RTMPMaintainPMKIDCache(_pAd)
#endif /* DOT1X_SUPPORT */

VOID *alloc_rx_buf_1k(void *hif_resource);
VOID free_rx_buf_1k(void *hif_resource);
VOID *alloc_rx_buf_64k(void *hif_resource);
VOID free_rx_buf_64k(void *hif_resource);
VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd);
BOOLEAN fill_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
UINT16 tx_pkt_classification(RTMP_ADAPTER *pAd, PNDIS_PACKET  pPacket, TX_BLK *pTxBlk);
INT send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
INT32 fp_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);

#ifdef SNIFFER_RADIOTAP_SUPPORT
void announce_802_11_radiotap_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,  struct _RX_BLK *rx_blk);
#endif

void announce_802_3_packet(
	IN	VOID			*pAdSrc,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			OpMode);

UINT announce_amsdu_pkt(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	UCHAR OpMode);

PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR FromWhichBSSID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(
	IN RTMP_ADAPTER *pAd,
	struct ba_control *ba_ctl,
	PBA_REC_ENTRY pBAEntry,
	ULONG Now32);

void ba_timeout_flush(RTMP_ADAPTER *pAd);
void ba_timeout_monitor(RTMP_ADAPTER *pAd);

VOID ba_ori_session_setup(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR TID,
	USHORT TimeOut);

BOOLEAN ba_resrc_ori_prep(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid,
	UCHAR TID,
	UINT16 ba_wsize,
	UCHAR amsdu_en,
	USHORT timeout);

BOOLEAN ba_resrc_ori_add(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid,
	UCHAR TID,
	UINT16 ba_wsize,
	UCHAR amsdu_en,
	USHORT timeout);

BOOLEAN ba_resrc_ori_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid);


BOOLEAN ba_resrc_rec_add(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid,
	USHORT timeout,
	UINT16 ba_win_size);

BOOLEAN ba_resrc_rec_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid);

VOID ba_session_tear_down_all(
	RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	BOOLEAN bPassive);

VOID ba_ori_session_tear_down(
	RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UCHAR TID,
	BOOLEAN bPassive);

VOID ba_resource_dump_all(RTMP_ADAPTER *pAd, ULONG second_idx);
VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd);
VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UINT16 wcid);
VOID ba_rec_session_tear_down(RTMP_ADAPTER *pAd, UINT16 Wcid,
				UCHAR TID, BOOLEAN bPassive);
#endif /* DOT11_N_SUPPORT */

BOOLEAN ba_reordering_resource_init(RTMP_ADAPTER *pAd, int num);
void ba_reordering_resource_release(RTMP_ADAPTER *pAd);
struct reordering_mpdu *ba_reordering_mpdu_probe(struct reordering_list *list);

INT ComputeChecksum(
	IN UINT PIN);

UINT GenerateWpsPinCode(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN		 bFromApcli,
	IN	UCHAR	apidx);

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef BB_SOC
INT	Set_WscResetPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_WscVendorPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_AP_SUPPORT
VOID RTMPIoctlSetWSCOOB(IN RTMP_ADAPTER *pAd);
#endif

/* */
/* prototype in wsc.c */
/* */
BOOLEAN	WscMsgTypeSubst(
	IN	UCHAR	EAPType,
	IN	UCHAR	EAPCode,
	OUT	INT	    *MsgType);

VOID	WscStateMachineInit(
	IN	RTMP_ADAPTER *pAd,
	IN	STATE_MACHINE		*S,
	OUT STATE_MACHINE_FUNC Trans[]);

#ifdef IWSC_SUPPORT
void	IWSC_StateMachineInit(
	IN  RTMP_ADAPTER *pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID	IWSC_Init(
	IN  IN RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
#endif /* IWSC_SUPPORT // */

VOID	WscEAPOLStartAction(
	IN  RTMP_ADAPTER *pAd,
	IN  MLME_QUEUE_ELEM  *Elem);

VOID	WscEAPAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM *Elem);

VOID	WscEapEnrolleeAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);

#ifdef CONFIG_AP_SUPPORT
VOID	WscEapApProxyAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);
#endif /* CONFIG_AP_SUPPORT */

VOID	WscEapRegistrarAction(
	IN	RTMP_ADAPTER *pAd,
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR			MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscEAPOLTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID	Wsc2MinsTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

UCHAR	WscRxMsgType(
	IN	RTMP_ADAPTER *pAd,
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscInitRegistrarPair(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN  UCHAR				apidx);

VOID	WscSendEapReqId(
	IN	RTMP_ADAPTER *pAd,
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				CurOpMode);

VOID	WscSendEapolStart(
	IN	RTMP_ADAPTER *pAd,
	IN  PUCHAR		pBssid,
	IN  UCHAR		 CurOpMode,
	IN  VOID		  *wdev_obj);

VOID	WscSendEapRspId(
	IN	RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  PWSC_CTRL		   pWscControl);

VOID	WscMacHeaderInit(
	IN	RTMP_ADAPTER *pAd,
	IN OUT	PHEADER_802_11	Hdr,
	IN	PUCHAR			pAddr1,
	IN  PUCHAR		  pBSSID,
	IN  BOOLEAN		 bFromApCli);

VOID	WscSendMessage(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   OpCode,
	IN  PUCHAR				pData,
	IN  INT					Len,
	IN  PWSC_CTRL		   pWscControl,
	IN  UCHAR			   OpMode,	/* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
	IN  UCHAR			   EapType);

VOID	WscSendEapReqAck(
	IN	RTMP_ADAPTER *pAd,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID	WscSendEapReqDone(
	IN	RTMP_ADAPTER *pAd,
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscSendEapFail(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN  BOOLEAN				bSendDeAuth);

VOID WscM2DTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID WscUPnPMsgTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

int WscSendUPnPConfReqMsg(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR apIdx,
	IN PUCHAR ssidStr,
	IN PUCHAR macAddr,
	IN INT	  Status,
	IN UINT   eventID,
	IN UCHAR  CurOpMode);

int WscSendUPnPMessage(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR				apIdx,
	IN	USHORT				msgType,
	IN	USHORT				msgSubType,
	IN	PUCHAR				pData,
	IN	INT					dataLen,
	IN	UINT				eventID,
	IN	UINT				toIPAddr,
	IN	PUCHAR				pMACAddr,
	IN  UCHAR				CurOpMode);

VOID WscUPnPErrHandle(
	IN RTMP_ADAPTER *pAd,
	IN PWSC_CTRL		pWscControl,
	IN UINT			eventID);

VOID	WscBuildBeaconIE(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR b_configured,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  UCHAR	AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);

VOID	WscBuildProbeRespIE(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR respType,
	IN	UCHAR scState,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT   AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);

#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT
VOID WscMakeProbeReqIEWithVendorExt(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR			pDeviceName,
	IN	PUCHAR			pPrimaryDeviceType,
	IN	PUCHAR			pVendExt,
	IN	USHORT			vendExtLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);
#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID WscBuildAssocRespIE(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			ApIdx,
	IN  UCHAR			Reason,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

VOID	WscSelectedRegistrar(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR	RegInfo,
	IN	UINT	length,
	IN  UCHAR	apidx);

VOID	WscInformFromWPA(
	IN  PMAC_TABLE_ENTRY	pEntry);

#ifdef WSC_AP_SUPPORT
#ifdef APCLI_SUPPORT
VOID  WscApCliLinkDownById(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR	apidx);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */

VOID WscBuildProbeReqIE(
	IN	RTMP_ADAPTER *pAd,
	IN  VOID		  *wdev_obj,
	OUT	PUCHAR		pOutBuf,
	OUT	PUCHAR		pIeLen);

VOID WscBuildAssocReqIE(
	IN  PWSC_CTRL		pWscControl,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

#ifdef CONFIG_STA_SUPPORT
#ifdef IWSC_SUPPORT
VOID IWSC_Stop(RTMP_ADAPTER *pAd, BOOLEAN bSendNotification);

VOID IWSC_T1TimerAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID IWSC_T2TimerAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID IWSC_EntryTimerAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID IWSC_DevQueryAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

BOOLEAN	IWSC_PeerEapolStart(
	IN  RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  MLME_QUEUE_ELEM *Elem);

VOID IWSC_AddSmpbcEnrollee(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR			pPeerAddr);

BOOLEAN IWSC_IpContentForCredential(
	IN  RTMP_ADAPTER *pAd);
#endif /* IWSC_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

VOID	WscProfileRetryTimeout(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID	WscPBCTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#ifdef CON_WPS
VOID	WscScanDoneCheckTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#endif /*CON_WPS*/

#ifdef WSC_STA_SUPPORT
VOID	WscPINTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#endif

VOID	WscScanTimeOutAction(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

INT WscGenerateUUID(
	RTMP_ADAPTER	*pAd,
	UCHAR			*uuidHexStr,
	UCHAR			*uuidAscStr,
	int			apIdx,
	BOOLEAN			bUseCurrentTime,
	BOOLEAN			from_apcli);

VOID WscStop(
	IN	RTMP_ADAPTER *pAd,
#ifdef CONFIG_AP_SUPPORT
	IN  BOOLEAN		 bFromApcli,
#endif /* CONFIG_AP_SUPPORT */
	IN  PWSC_CTRL	   pWscControl);

VOID WscInit(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN		 bFromApcli,
	IN  UCHAR			BssIndex);
#ifdef CON_WPS
VOID WscConWpsStop(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN		 bFromApCli,
	IN  PWSC_CTRL	   pWscControl);
#endif /* CON_WPS */

BOOLEAN	ValidateChecksum(UINT PIN);

UINT WscRandomGen4digitPinCode(RTMP_ADAPTER *pAd);

UINT WscRandomGeneratePinCode(
	IN	RTMP_ADAPTER *pAd,
	IN	UCHAR	apidx);

int BuildMessageM1(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2D(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM3(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM4(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM5(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM6(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM7(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM8(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageDONE(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageACK(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int BuildMessageNACK(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	OUT	VOID *pbuf);

int ProcessMessageM1(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	IN  UCHAR			apidx,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2D(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM3(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM4(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM5(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM6(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM7(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		   pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM8(
	IN	RTMP_ADAPTER *pAd,
	IN	VOID *precv,
	IN	INT Length,
	IN  PWSC_CTRL	   pWscControl);

USHORT  WscGetAuthType(
	IN  UINT32 authType);

USHORT  WscGetEncryType(
	IN  UINT32 encryType);

NDIS_STATUS WscThreadInit(RTMP_ADAPTER *pAd);

BOOLEAN WscThreadExit(RTMP_ADAPTER *pAd);

int	 AppendWSCTLV(
	IN  USHORT index,
	OUT UCHAR *obuf,
	IN  UCHAR *ibuf,
	IN  USHORT varlen);

VOID	WscGetRegDataPIN(
	IN  RTMP_ADAPTER *pAd,
	IN  UINT			PinCode,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPushPBCAction(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

#ifdef WSC_STA_SUPPORT
VOID	WscPINAction(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

BOOLEAN WscPINExec(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN			bFromM2,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPINBssTableSort(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl);
#endif

VOID	WscScanExec(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL		pWscControl);

BOOLEAN WscPBCExec(
	IN	RTMP_ADAPTER *pAd,
	IN  BOOLEAN			bFromM2,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscPBCBssTableSort(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl);

VOID	WscGenRandomKey(
	IN	RTMP_ADAPTER *pAd,
	IN	PWSC_CTRL	   pWscControl,
	INOUT	PUCHAR			pKey,
	INOUT	PUSHORT			pKeyLen);

VOID	WscCreateProfileFromCfg(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   OpMode,		 /* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
	IN  PWSC_CTRL		   pWscControl,
	OUT PWSC_PROFILE		pWscProfile);

void	WscWriteConfToPortCfg(
	IN  RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN		 bEnrollee);

#ifdef APCLI_SUPPORT
void	WscWriteConfToApCliCfg(
	IN  RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN		 bEnrollee);
#endif /* APCLI_SUPPORT */

VOID   WpsSmProcess(
	IN PRTMP_ADAPTER		pAd,
	IN MLME_QUEUE_ELEM	   *Elem);

VOID WscPBCSessionOverlapCheck(
	IN	RTMP_ADAPTER * pAd,
	IN	UCHAR	current_band);

VOID WscPBC_DPID_FromSTA(
	IN	RTMP_ADAPTER *pAd,
	IN	PUCHAR				pMacAddr,
	IN	UCHAR	current_band);

#ifdef CONFIG_AP_SUPPORT
INT	WscGetConfWithoutTrigger(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL	   pWscControl,
	IN  BOOLEAN		 bFromUPnP);

BOOLEAN	WscReadProfileFromUfdFile(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   ApIdx,
	IN  RTMP_STRING *pUfdFileName);

BOOLEAN	WscWriteProfileToUfdFile(
	IN	RTMP_ADAPTER *pAd,
	IN  UCHAR			   ApIdx,
	IN  RTMP_STRING *pUfdFileName);
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckWpsIeFromWpsAP(
	IN  RTMP_ADAPTER *pAd,
	IN  PEID_STRUCT		pEid,
	OUT PUSHORT			pDPIDFromAP);

#ifdef CONFIG_STA_SUPPORT
VOID PeerProbeRespAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

ULONG WscSearchWpsApBySSID(
	IN  RTMP_ADAPTER *pAd,
	IN PUCHAR			pSsid,
	IN UCHAR			SsidLen,
	IN INT				WscMode,
	IN VOID			 *wdev_obj);
#endif /* CONFIG_STA_SUPPORT */

/* WSC hardware push button function 0811 */
VOID WSC_HDR_BTN_Init(RTMP_ADAPTER *pAd);
VOID WSC_HDR_BTN_Stop(RTMP_ADAPTER *pAd);
VOID WSC_HDR_BTN_CheckHandler(RTMP_ADAPTER *pAd);
#ifdef WSC_LED_SUPPORT
BOOLEAN WscSupportWPSLEDMode(RTMP_ADAPTER *pAd);
BOOLEAN WscSupportWPSLEDMode10(RTMP_ADAPTER *pAd);

BOOLEAN WscAPHasSecuritySetting(RTMP_ADAPTER *pAd, PWSC_CTRL pWscControl);

VOID WscLEDTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);

VOID WscSkipTurnOffLEDTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);
#endif /* WSC_LED_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
VOID LEDControlTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);
#endif


#ifdef WIDI_SUPPORT
void WidiWscNotify(
	IN ULONG pAd,
	IN ULONG pWidiMsg,
	IN ULONG MsgSize,
	IN ULONG MsgType);

VOID WidiUpdateStateToDaemon(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR WidiType,
	IN PUCHAR pPeerMac,
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen,
	IN UCHAR WidiAssocStat);

VOID WidiNotifyVendorExtToDaemon(
	IN RTMP_ADAPTER *pAd,
	IN PWIDI_VENDOR_EXT pWiDiVendorExtList,
	IN PUCHAR pSrcMac,
	IN UCHAR channel,
	IN PCHAR Ssid,
	IN UCHAR SsidLen);

UINT WscSpecialRandomGeneratePinCode(RTMP_ADAPTER *pAd);

VOID WidiNotifyP2pIPToDaemon(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pWpsIe,
	IN UCHAR WpsIeLen,
	IN PUCHAR pSrcMac,
	IN UCHAR channel,
	IN PCHAR Ssid,
	IN UCHAR SsidLen);

BOOLEAN  WidiNotifyP2pProbeIeToDaemon(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pWpsIe,
	IN UCHAR WpsIeLen,
	IN PUCHAR pSrcMac,
	IN UINT8 channel,
	IN UCHAR *Ssid,
	IN USHORT SsidLen);

#endif /* WIDI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckPeerDPID(
	IN  RTMP_ADAPTER *pAd,
	IN  PFRAME_802_11	Fr,
	IN  PUCHAR			eid_data,
	IN  INT				eid_len,
	IN	UCHAR			current_band);

VOID WscClearPeerList(PLIST_HEADER pWscEnList);

PWSC_PEER_ENTRY WscFindPeerEntry(PLIST_HEADER	pWscEnList, UCHAR *pMacAddr);
VOID WscDelListEntryByMAC(PLIST_HEADER pWscEnList, UCHAR *pMacAddr);
VOID WscInsertPeerEntryByMAC(PLIST_HEADER	pWscEnList, UCHAR *pMacAddr);

#ifdef CONFIG_AP_SUPPORT
INT WscApShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT WscApShowPin(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT WscStaShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_STA_SUPPORT */

VOID WscMaintainPeerList(RTMP_ADAPTER *pAd, PWSC_CTRL pWpsCtrl);

VOID WscAssignEntryMAC(RTMP_ADAPTER *pAd, PWSC_CTRL pWpsCtrl);

#ifdef WSC_V2_SUPPORT
#ifdef CONFIG_AP_SUPPORT
VOID WscOnOff(RTMP_ADAPTER *pAd, INT ApIdx, BOOLEAN bOff);

VOID WscAddEntryToAclList(RTMP_ADAPTER *pAd, INT ApIdx, UCHAR *pMacAddr);

VOID WscSetupLockTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WscCheckPinAttackCount(RTMP_ADAPTER *pAd, PWSC_CTRL pWscControl);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN	WscGenV2Msg(
	IN  PWSC_CTRL		pWpsCtrl,
	IN  BOOLEAN			bSelRegistrar,
	IN	PUCHAR			pAuthorizedMACs,
	IN  INT				AuthorizedMACsLen,
	OUT	UCHAR			**pOutBuf,
	OUT	INT				*pOutBufLen);

BOOLEAN	WscParseV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	USHORT			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);

VOID	WscSendEapFragAck(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID	WscSendEapFragData(
	IN	RTMP_ADAPTER *pAd,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);
#endif /* WSC_V2_SUPPORT */

BOOLEAN WscGetDataFromPeerByTag(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR			pIeData,
	IN  INT				IeDataLen,
	IN  USHORT			WscTag,
	OUT PUCHAR			pWscBuf,
	OUT PUSHORT			pWscBufLen);

VOID WscUUIDInit(
	IN  PRTMP_ADAPTER pAd,
	IN  INT inf_idx,
	IN  UCHAR from_apcli);

#ifdef CONFIG_AP_SUPPORT
INT Set_AP_WscMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AP_WscGetConf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

INT32 ShowRFInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowBBPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_redirect_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowWifiInterruptCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2);
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2);
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2);

RTMP_STRING *rtstrstr(const RTMP_STRING *s1, const RTMP_STRING *s2);
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct);
INT delimitcnt(RTMP_STRING *s, RTMP_STRING *ct);
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr);

#ifdef PER_PKT_CTRL_FOR_CTMR
INT set_host_pkt_ctrl_en(PRTMP_ADAPTER	pAdapter,RTMP_STRING *pParam);
#endif

/*//////// common ioctl functions ////////*/
INT show_driverinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Probe_Rsp_Times_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_phy_channel_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID ChOpTimeout(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3);
VOID ChannelOpCtrlInit(PRTMP_ADAPTER	pAd);
VOID ChannelOpCtrlDeinit(PRTMP_ADAPTER	pAd);
BOOLEAN TakeChannelOpCharge(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR owner, BOOLEAN wait);
BOOLEAN TakeChannelOpChargeByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, UCHAR owner, BOOLEAN wait);
VOID ReleaseChannelOpCharge(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR owner);
VOID ReleaseChannelOpChargeByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, UCHAR owner);
VOID ReleaseChannelOpChargeForCurrentOwner(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR GetCurrentChannelOpOwner(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT Set_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
INT Set_V10ConverterMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/
BOOLEAN	perform_channel_change(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);
INT	Set_SeamlessCSA_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT UNII4BandSupport(RTMP_ADAPTER *pAd);
INT rtmp_set_channel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);
INT	Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MaxTxPwr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxPower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BGProtection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DELAY_TCP_ACK_V2
INT	Set_peak_tp_txop_dynamic_adjust_enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_peak_tp_be_aifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_peak_tp_rx_avg_len_th_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_peak_tput_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_peak_tp_rx_lower_bound_th_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_peak_tp_rx_higher_bound_th_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* DELAY_TCP_ACK_V2 */

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AGGREGATION_SUPPORT */

INT	Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DBG
UINT32 get_dbg_lvl_by_cat_subcat(UINT32 dbg_cat, UINT32 dbg_sub_cat);
void set_dbg_lvl_all(UINT32 dbg_lvl);
NTSTATUS get_dbg_setting_by_profile(RTMP_STRING *dbg_level, RTMP_STRING *dbg_option);
INT	Set_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_DebugCategory_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#ifdef DBG_ENHANCE
INT Set_DebugOption_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif
#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
INT	Set_RxMaxTraffic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_rx_detect_flag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

/* HwCtrl Task debug */
INT set_hwctrl_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_hwctrl_q_len(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DOT11_HE_AX
INT set_color_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txop_duration_prot_threshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
INT Set_Max_ProbeRsp_IE_Cnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*CUSTOMER_VENDOR_IE_SUPPORT*/

#ifdef RATE_PRIOR_SUPPORT

#ifndef RATE_PRIOR_LOW_RATE_THRESHOLD
#define RATE_PRIOR_LOW_RATE_THRESHOLD 3
#endif

INT Set_RatePrior_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BlackListTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_LowRateRatio_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_LowRateCountPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TotalCntThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*RATE_PRIOR_SUPPORT*/


#ifdef RANDOM_PKT_GEN
INT Set_TxCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID regular_pause_umac(RTMP_ADAPTER *pAd);
#endif

#ifdef CSO_TEST_SUPPORT
INT Set_CsCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef MT_MAC
INT show_txvinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT set_thermal_protection_criteria_proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT get_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg);

#ifdef TXBF_SUPPORT
INT	Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_Stop_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_StaITxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef MT_MAC
INT Set_TxBfProfileTag_Help(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_PfmuIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_BfType(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SuMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_InValid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_Mem(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_Matrix(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_prof_tag_ru_range(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_prof_tag_mob_cal_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SNR(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SmartAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_SeIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_RmsdThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_McsThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_TimeOut(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredNc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTag_DesiredNr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_prof_tag_ru_alloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTagRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileDataRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileDataWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_ATE
INT Set_TxBfProfileData20MAllWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT set_txbf_angle_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_dsnr_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_pfmu_data_write(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfilePnRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfilePnWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfAidUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID ate_set_manual_assoc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID ate_set_cmm_starec(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfQdRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfFbRptDbgInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfTxSndInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfPlyInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfTxCmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfSndCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HeRaMuMetricInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfProfileSwTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_stop_report_poll_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfTxApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemAlloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemRelease(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_TxBfPfmuMemAllocMapRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecCmmUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecBfUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txbf_he_bf_starec(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_StaRecBfRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TxBfAwareCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_dynsnd_en_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HostReportTxLatency(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_RxFilterDropCtrlFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CertCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CFG_SUPPORT_MU_MIMO
INT set_dynsnd_cfg_dmcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_dynsnd_en_mu_intr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Set_BssInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_DevInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TXBF_DYNAMIC_DISABLE
INT Set_TxBfDynamicDisable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXBF_DYNAMIC_DISABLE */
INT set_txbf_dynamic_mechanism_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
INT set_mec_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef VHT_TXBF_SUPPORT
INT Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* VHT_TXBF_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
INT WifiFwdSet(IN int disabled);
INT Set_WifiFwd_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwd_Down(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
INT Set_WifiFwdAccessSchedule_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdHijack_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdBpdu_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdRepDevice(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdShowEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdDeleteEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_PacketSourceShowEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_PacketSourceDeleteEntry(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT Set_WifiFwdBridge_Proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);

#endif /* CONFIG_WIFI_PKT_FWD */

INT Set_RateAdaptInterval(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_DescInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef MWDS
INT Show_MWDS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Show_Mib_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef ACL_BLK_COUNT_SUPPORT
INT Show_ACLRejectCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*ACL_BLK_COUNT_SUPPORT*/

#ifdef DOT11_N_SUPPORT
INT Show_BaTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_ChannelSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
INT show_client_idle_time(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#define IS_V10_OLD_CHNL_VALID(_wdev) \
	(_wdev->bV10OldChannelValid == TRUE)
#define SET_V10_OLD_CHNL_VALID(_wdev, valid) \
	(_wdev->bV10OldChannelValid = valid)
#endif

#ifdef BW_VENDOR10_CUSTOM_FEATURE
#define IS_APCLI_BW_SYNC_FEATURE_ENBL(_pAd, band_idx) \
	((BOOLEAN) (_pAd->ApCfg.ApCliAutoBWRules[band_idx].minorPolicy.ApCliBWSyncHTSupport) \
	|| (_pAd->ApCfg.ApCliAutoBWRules[band_idx].minorPolicy.ApCliBWSyncVHTSupport))

#define IS_V10_AUTO_BAND_FEATURE_ENBL(_pAd) \
	((BOOLEAN) (_pAd->ApCfg.ApCliAutoBWRules[0].majorPolicy.ApCliBWSyncBandSupport) \
	|| (_pAd->ApCfg.ApCliAutoBWRules[1].majorPolicy.ApCliBWSyncBandSupport) \
	|| (_pAd->ApCfg.ApCliAutoBWRules[0].majorPolicy.ApCliBWSyncDeauthSupport) \
	|| (_pAd->ApCfg.ApCliAutoBWRules[1].majorPolicy.ApCliBWSyncDeauthSupport))

typedef enum _ENUM_AUTO_BW_POLICY {
	HT_2040_UP_ENBL = 0,   /* HT  20 -> 40 : BIT 0*/
	HT_4020_DOWN_ENBL,	   /* HT  40 -> 20 : BIT 1 */
	VHT_2040_80_UP_ENBL,   /* VHT 20/40 -> 80 : BIT 0 */
	VHT_2040_160_UP_ENBL,  /* VHT 20/40 -> 160 : BIT 1 */
	VHT_80_160_UP_ENBL,    /* VHT 80 -> 160 : BIT 2 */
	VHT_80_2040_DOWN_ENBL, /* VHT 80 -> 20/40 : BIT 3 */
	VHT_160_2040_DOWN_ENBL,/* VHT 160 -> 20/40 : BIT 4 */
	VHT_160_80_DOWN_ENBL,  /* VHT 160 -> 80 : BIT 5 */
	BW_MAX_POLICY          /* Reserved */
} ENUM_AUTO_BW_POLICY;

#define VHT_POLICY_OFFSET 2

#define VHT_2040_80_UP_CHK    (VHT_2040_80_UP_ENBL - VHT_POLICY_OFFSET)
#define VHT_80_2040_DOWN_CHK  (VHT_80_2040_DOWN_ENBL - VHT_POLICY_OFFSET)
#define VHT_2040_160_UP_CHK   (VHT_2040_160_UP_ENBL - VHT_POLICY_OFFSET)
#define VHT_80_160_UP_CHK     (VHT_80_160_UP_ENBL - VHT_POLICY_OFFSET)
#define VHT_80_2040_DOWN_CHK  (VHT_80_2040_DOWN_ENBL - VHT_POLICY_OFFSET)
#define VHT_160_2040_DOWN_CHK (VHT_160_2040_DOWN_ENBL - VHT_POLICY_OFFSET)
#define VHT_160_80_DOWN_CHK   (VHT_160_80_DOWN_ENBL - VHT_POLICY_OFFSET)


#define SET_APCLI_AUTO_BW_HT_VALID(_pAd, valid, band_idx) \
	(_pAd->ApCfg.ApCliAutoBWRules[band_idx].minorPolicy.ApCliBWSyncHTSupport |= (1 << valid))

#define SET_APCLI_AUTO_BW_VHT_VALID(_pAd, valid, band_idx) \
	(_pAd->ApCfg.ApCliAutoBWRules[band_idx].minorPolicy.ApCliBWSyncVHTSupport |= (1 << valid))


typedef enum _ENUM_BAND_BW_POLICY {
	SAME_BAND_SYNC = 0,	/* Sync Same Band BW */
	DIFF_BAND_SYNC,     /* Sync Diff Band BW */
	DEAUTH_PEERS,		/* Deauth Clients */
	BAND_MAX_POLICY
} ENUM_BAND_BW_POLICY;

#define POLICY_DISABLE     0

#define IS_APCLI_SYNC_BAND_VALID(_pAd, condition, band_idx) \
	(1 & (_pAd->ApCfg.ApCliAutoBWRules[band_idx].majorPolicy.ApCliBWSyncBandSupport >> condition))
#define SET_APCLI_SYNC_BAND_VALID(_pAd, valid, band_idx) \
	(_pAd->ApCfg.ApCliAutoBWRules[band_idx].majorPolicy.ApCliBWSyncBandSupport |= (1 << valid))
#define SET_APCLI_SYNC_BAND_FEATURE_DISABLE(_pAd, valid, band_idx) \
			(_pAd->ApCfg.ApCliAutoBWRules[band_idx].majorPolicy.ApCliBWSyncBandSupport = valid)

#define IS_APCLI_SYNC_PEER_DEAUTH_VALID(_pAd, band_idx) \
	(_pAd->ApCfg.ApCliAutoBWRules[band_idx].majorPolicy.ApCliBWSyncDeauthSupport)
#define SET_APCLI_SYNC_PEER_DEAUTH_VALID(_pAd, valid, band_idx) \
	(_pAd->ApCfg.ApCliAutoBWRules[band_idx].majorPolicy.ApCliBWSyncDeauthSupport = valid)

#define IS_APCLI_SYNC_PEER_DEAUTH_ENBL(_pAd, band_idx) \
	(_pAd->ApCfg.AutoBWDeauthEnbl[band_idx])
#define SET_APCLI_SYNC_PEER_DEAUTH_ENBL(_pAd, valid, band_idx) \
	(_pAd->ApCfg.AutoBWDeauthEnbl[band_idx] = valid)
#endif

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
INT show_current_rssi(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef MT_MAC
INT Show_PSTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbl_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbltlv_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_mib_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_amsdu_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_wifi_sys(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DBDC_MODE
INT32 ShowDbdcProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT32 ShowChCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef GREENAP_SUPPORT
INT32 ShowGreenAPProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 show_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 show_twt_support_cap_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

INT32 show_tx_burst_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowTmacInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowAggInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowArbInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_dmasch_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowPseInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowPseData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_drr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT ShowPLEInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_TXD_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTXDInfo(RTMP_ADAPTER *pAd, UINT fid);
INT show_mem_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_protect_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_cca_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* MT_MAC */
INT Show_sta_tr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef SW_CONNECT_SUPPORT
INT Show_dummy_stat_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SW_CONNECT_SUPPORT */
INT show_stainfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_devinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_sysinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_trinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_WIFI_MSI_SUPPORT
INT show_msiinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef CONFIG_TP_DBG
INT Set_TPDbg_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_TPDbg_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_RxINT_info_proc(RTMP_ADAPTER *pAd); //yiwei debug!
#endif /* CONFIG_TP_DBG */
INT show_tpinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_mlmeinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txqinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_swqinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef RTMP_EFUSE_SUPPORT
INT show_efuseinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef DOT11_HE_AX
INT show_bsscolor_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT Show_AteIbfPhaseCalStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT	Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	SetCommonHtVht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT Set_DynamicAGG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtGi_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	set_extcha_for_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR value);
INT	Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtRdg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtProtect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_hwcfg_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef EEPROM_RETRIEVE_SUPPORT
INT32 show_e2p_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EEPROM_RETRIEVE_SUPPORT */
#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11N_DRAFT3 */

#ifdef CONFIG_AP_SUPPORT
INT	Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT	set_pcie_aspm_dym_ctrl_cap_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT set_twt_support_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
INT set_twt_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
INT set_btwt_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#endif /* WIFI_TWT_SUPPORT */
INT set_muedca_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#endif /* DOT11_HE_AX */

INT	SetCommonHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT	Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ForceGF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

INT	Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_BurstMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
INT Set_VhtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VhtLdpc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_VHT_AC */

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
INT Set_RSSI_Enbl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*APCLI_SUPPORT*/

#ifdef ETH_CONVERT_SUPPORT
INT Set_EthConvertMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_EthCloneMac_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* ETH_CONVERT_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
INT Show_MemInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_PktInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MEM_ALLOC_INFO_SUPPORT */
INT Show_SimSectionUlm_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_CoreDump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_FwDbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_BusDbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CpuUtilEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CpuUtilMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ChkExceptionType(RTMP_ADAPTER *pAd);

RTMP_STRING *GetAuthModeStr(
	IN UINT32 authMode);

RTMP_STRING *GetEncryModeStr(
	IN UINT32 encryMode);

UINT32 SecAuthModeOldToNew(
	IN USHORT authMode);

UINT32 SecEncryModeOldToNew(
	IN USHORT encryMode);

USHORT SecAuthModeNewToOld(
	IN UINT32 authMode);

USHORT SecEncryModeNewToOld(
	IN UINT32 encryMode);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPSendDLSTearDownFrame(RTMP_ADAPTER *pAd, UCHAR *pDA, struct wifi_dev *wdev);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

VOID detect_wmm_traffic(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR up, UCHAR bOutput);
#ifdef CONFIG_AP_SUPPORT
VOID dynamic_tune_be_tx_op(RTMP_ADAPTER *pAd, ULONG nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID Handle_BSS_Width_Trigger_Events(RTMP_ADAPTER *pAd, UCHAR Channel);

void build_ext_channel_switch_ie(
	IN RTMP_ADAPTER *pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE * pIE,
	IN UCHAR Channel,
	IN USHORT PhyMode,
	IN struct wifi_dev *wdev);

void assoc_ht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN HT_CAPABILITY_IE *pHTCapability);
#endif /* DOT11_N_SUPPORT */

INT header_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk);

NDIS_STATUS rx_packet_process(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pRxPacket,
	RX_BLK *pRxBlk);

VOID ap_ieee802_3_data_rx(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID sta_ieee802_3_data_rx(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID indicate_rx_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_ampdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_amsdu_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID ba_reorder(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID ba_reorder_buf_maintain(RTMP_ADAPTER *pAd);
VOID ba_refresh_bar_all(RTMP_ADAPTER *pAd);

VOID indicate_802_3_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_802_11_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);
VOID indicate_eapol_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx);

UINT deaggregate_amsdu_announce(
	IN	RTMP_ADAPTER *pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode);

/* remove LLC and get 802_3 Header */
#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
	{																			\
		PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;								\
		\
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) {	\
			_pDA = _pRxBlk->Addr3;															\
			_pSA = _pRxBlk->Addr4;															\
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_AP)) {								\
			_pDA = _pRxBlk->Addr1;													\
			if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))									\
				_pSA = _pRxBlk->Addr2;												\
			else																	\
				_pSA = _pRxBlk->Addr3;		\
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_STA)) {							\
			_pDA = _pRxBlk->Addr3;									 \
			_pSA = _pRxBlk->Addr2;									 \
		} else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_ADHOC)) {							  \
			_pDA = _pRxBlk->Addr1;									 \
			_pSA = _pRxBlk->Addr2;									 \
		} else {														\
			/* TODO: shiang-usw, where shall we go here?? */			\
			MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Un-assigned Peer's Role!\n");\
			_pDA = _pRxBlk->Addr3;										\
			_pSA = _pRxBlk->Addr2;										\
		}																\
		\
		CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData,		\
						 _pRxBlk->DataSize, _pRemovedLLCSNAP);								  \
	}

VOID announce_or_forward_802_3_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket,
								   struct wifi_dev *wdev, UCHAR op_mode);

VOID indicate_agg_ralink_pkt(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER * pAd,
	IN CHAR *rssi_last,
	IN CHAR *rssi_avg,
	IN UINT8 ant_num,
	IN struct rx_signal_info *signal);

VOID Update_Snr_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phy_mode,
	IN UCHAR bw);

PNDIS_PACKET sdio_get_pkt_from_rx_resource(
	RTMP_ADAPTER *pAd,
	BOOLEAN	*pbReschedule,
	UINT32 *pRxPending,
	UCHAR RxRingNo);

VOID de_fragment_data_pkt(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif

#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
INT Set_ApCli_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetApCliEnableByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN flag);
#endif
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_PwrSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_SendPsPoll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef APCLI_CFG80211_SUPPORT
INT Set_ApCli_EncrypType(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_AuthMode(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_Enable(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
INT Set_ApCli_Ssid(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg);
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef ROAMING_ENHANCE_SUPPORT
BOOLEAN IsApCliLinkUp(IN PRTMP_ADAPTER pAd);
BOOLEAN ApCliDoRoamingRefresh(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PNDIS_PACKET pRxPacket,
	IN struct wifi_dev *wdev);
#endif /* ROAMING_ENHANCE_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT Set_ApCli_Twt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAT_SUPPORT

VOID getIPv6MacTbInfo(MAT_STRUCT *, char *, ULONG);
VOID getIPMacTbInfo(MAT_STRUCT *pMatCfg, char *pOutBuf, ULONG BufLen);

NDIS_STATUS MATEngineInit(RTMP_ADAPTER *pAd);
NDIS_STATUS MATEngineExit(RTMP_ADAPTER *pAd);

PUCHAR MATEngineRxHandle(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx);
PUCHAR MATEngineTxHandle(RTMP_ADAPTER *pAd, PNDIS_PACKET pPkt, UINT infIdx, UINT32 entry_type);

BOOLEAN MATPktRxNeedConvert(RTMP_ADAPTER *pAd, PNET_DEV net_dev);

#endif /* MAT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT		CountryNum;
	UCHAR		IsoName[3];
	/*UCHAR		CountryName[40]; */
	RTMP_STRING *pCountryName;
	BOOLEAN		SupportABand;
	/*ULONG		RegDomainNum11A; */
	UCHAR		RegDomainNum11A;
	BOOLEAN	SupportGBand;
	/*ULONG		RegDomainNum11G; */
	UCHAR		RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNMP_SUPPORT
/*for snmp */
typedef struct _DefaultKeyIdxValue {
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPSetDesiredRates(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, LONG Rates);
#endif /* CONFIG_STA_SUPPORT */

INT	Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

void CfgInitHook(RTMP_ADAPTER *pAd);

NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd);
VOID mtd_net_tasklet_exit(RTMP_ADAPTER *pAd);
VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd);

void tbtt_tasklet(unsigned long data);

#ifdef MT_MAC
void mt_mac_int_0_tasklet(unsigned long data);
void mt_mac_int_1_tasklet(unsigned long data);
void mt_mac_int_2_tasklet(unsigned long data);
void mt_mac_int_3_tasklet(unsigned long data);
void mt_mac_int_4_tasklet(unsigned long data);
#endif /* MT_MAC */

#ifdef RTMP_MAC_PCI
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			Level,
	IN USHORT		   TbttNumToNextWakeUp);

BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level);
VOID RTMPInitPCIeDevice(RT_CMD_PCIE_INIT *pConfig, VOID *pAd);

#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicWakeup(
	RTMP_ADAPTER *pAd,
	BOOLEAN bFromTx,
	PSTA_ADMIN_CONFIG pStaCfg);

VOID RT28xxPciStaAsicSleepAutoWakeup(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg);

#endif /* CONFIG_STA_SUPPORT */

VOID PciMlmeRadioOn(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID PciMlmeRadioOFF(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction);

#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
NDIS_STATUS RecoverConnectInfo(RTMP_ADAPTER *pAd);
NDIS_STATUS StoreConnectInfo(RTMP_ADAPTER *pAd);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef NEW_WOW_SUPPORT
VOID RT28xxAndesWOWEnable(RTMP_ADAPTER *pAd);
VOID RT28xxAndesWOWDisable(RTMP_ADAPTER *pAd);
#endif /* NEW_WOW_SUPPORT */

#ifdef MT_WOW_SUPPORT
VOID MT76xxAndesWOWEnable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID MT76xxAndesWOWDisable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID MT76xxAndesWOWInit(RTMP_ADAPTER *pAd);
#endif

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
VOID RT28xxAsicWOWEnable(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID RT28xxAsicWOWDisable(RTMP_ADAPTER *pAd);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

/*////////////////////////////////////*/

/*///////////////////////////////////*/
INT RTMPShowCfgValue(RTMP_ADAPTER *pAd, RTMP_STRING *name, RTMP_STRING *buf, UINT32 MaxLen);
/*//////////////////////////////////*/

#ifdef CONFIG_STA_SUPPORT
VOID AsicStaBbpTuning(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);

BOOLEAN StaUpdateMacTableEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN IE_LISTS *ie_list,
	IN  USHORT				CapabilityInfo);

#ifdef DOT11R_FT_SUPPORT
VOID FT_OTA_AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID FT_OTD_TimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* DOT11R_FT_SUPPORT */

BOOLEAN	AUTH_ReqSend(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  RTMP_STRING *pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen);
#endif /* CONFIG_STA_SUPPORT */

UINT VIRTUAL_IF_INC(RTMP_ADAPTER *pAd);
UINT VIRTUAL_IF_DEC(RTMP_ADAPTER *pAd);
UINT VIRTUAL_IF_NUM(RTMP_ADAPTER *pAd);


#ifdef SOFT_ENCRYPT
VOID RTMPUpdateSwCacheCipherInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *pHdr);
#endif /* SOFT_ENCRYPT */

#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
BOOLEAN RTMPExpandPacketForSwEncrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
INT tx_sw_encrypt(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk, UCHAR *buf_ptr, HEADER_802_11 *wifi_hdr);
#endif /* SOFT_ENCRYPT */

/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/
VOID RTInitializeCmdQ(PCmdQ cmdq);

INT RTPCICmdThread(ULONG Context);

VOID CMDHandler(RTMP_ADAPTER *pAd);

VOID RTThreadDequeueCmd(PCmdQ cmdq, PCmdQElmt *pcmdqelmt);

NDIS_STATUS RTEnqueueInternalCmd(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength);

void RtmpCmdQExit(RTMP_ADAPTER *pAd);
void RtmpCmdQInit(RTMP_ADAPTER *pAd);


BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property);

#ifdef CONFIG_STA_SUPPORT

/* command */
INT Set_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_WmmCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APSDCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APSDAC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MaxSPLength_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NetworkType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AuthMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_EncrypType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DefaultKeyID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Wep_Key_Proc(RTMP_ADAPTER  *pAd, RTMP_STRING *Key, INT KeyLen, INT KeyId);
INT Set_Key1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Key4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WPAPSK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_PSMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CFG_TDLS_SUPPORT
INT Set_CfgTdlsChannelSwitchRequest_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_CfgTdlsChannelSwitchBaseStayTime_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_CfgTdlsChannelSwitchBW_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT	Set_TdlsDisableChannelSwitchProc(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);

#endif /* CFG_TDLS_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT
INT Set_Wpa_Support(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef DBG
VOID RTMPIoctlE2PROM(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* DBG */

#ifdef WSC_STA_SUPPORT
VOID CntlWscIterate(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);

USHORT WscGetAuthTypeFromStr(RTMP_STRING *arg);
USHORT WscGetEncrypTypeFromStr(RTMP_STRING *arg);

INT	Set_WscConfMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscConfStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WscBssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscUUIDE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscGetConf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_WscStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_V2_SUPPORT
INT Set_WscForceSetAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WSC_V2_SUPPORT */
#endif /* WSC_STA_SUPPORT */

NDIS_STATUS RTMPWPANoneAddKeyProc(RTMP_ADAPTER *pAd, VOID *pBuf);

#ifdef ETH_CONVERT_SUPPORT
#ifdef IP_ASSEMBLY
INT Set_FragTest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IP_ASSEMBLY */
#endif /* ETH_CONVERT_SUPPORT */

#ifdef DOT11_N_SUPPORT
INT Set_TGnWifiTest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11_N_SUPPORT */

INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

INT	Show_Adhoc_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *extra, UINT32 size);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPIoctlRF(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* RTMP_RF_RW_SUPPORT */


INT Set_BeaconLostTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AutoRoaming_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ForceTxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef AP_SCAN_SUPPORT
INT Set_ClearSiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AP_SCAN_SUPPORT */


VOID RTMPAddKey(RTMP_ADAPTER *pAd, PNDIS_802_11_KEY pKey, struct wifi_dev *wdev);
VOID StaSiteSurvey(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	struct wifi_dev *wdev);

VOID bss_table_maintenance(
	IN  RTMP_ADAPTER *pAd,
	IN  struct wifi_dev *wdev,
	IN OUT	BSS_TABLE *Tab,
	IN  ULONG	MaxRxTimeDiff,
	IN  UCHAR	MaxSameRxTimeCount);
#endif /* CONFIG_STA_SUPPORT */
INT32 getLegacyOFDMMCSIndex(UINT8 MCS);
void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG *fLastTxRxRate);
#ifdef DOT11_HE_AX
void  get_rate_he(UINT8 mcs, UINT8 bw, UINT8 nss, UINT8 dcm, ULONG *last_tx_rate);
#endif

#ifdef CONFIG_MT7976_SUPPORT
EEPROM_PWR_ON_MODE_T get_power_on_cal_mode(
	IN	PRTMP_ADAPTER	pAd);
#endif /* CONFIG_MT7976_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
VOID ApcliSendAssocIEsToWpaSupplicant(
	IN  RTMP_ADAPTER *pAd,
	IN UINT ifIndex);

VOID ApcliWpaSendEapolStart(
	IN	RTMP_ADAPTER *pAd,
	IN  PUCHAR		  pBssid,
	IN  PMAC_TABLE_ENTRY pMacEntry,
	IN	PSTA_ADMIN_CONFIG pApCliEntry);
#endif/*WPA_SUPPLICANT_SUPPORT*/

VOID ApCliRTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			TxRate,
	IN BOOLEAN		bQosNull,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN	  USHORT		  PwrMgmt);

/*APPS DVT MSP*/
VOID AppsApCliRTMPSendNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR			TxRate,
	IN BOOLEAN		bQosNull,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN      USHORT          PwrMgmt);

VOID ApCliRTMPSendPsPollFrame(
	IN	PRTMP_ADAPTER    pAd,
	IN	UINT_8  index);
#endif/*APCLI_SUPPORT*/

void RTMP_IndicateMediaState(
	IN	RTMP_ADAPTER *pAd,
	IN  NDIS_MEDIA_STATE	media_state);

INT RTMPSetInformation(
	IN RTMP_ADAPTER *pAd,
	IN OUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd,
	IN struct wifi_dev *wdev);

INT RTMPQueryInformation(
	IN RTMP_ADAPTER *pAd,
	INOUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd,
	IN struct wifi_dev *wdev);

VOID RTMPIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *rq,
	IN UINT32 subcmd,
	IN VOID *pData,
	IN ULONG Data);

INT RTMP_COM_IoctlHandle(
	IN VOID *pAdSrc,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN INT cmd,
	IN USHORT subcmd,
	IN VOID *pData,
	IN ULONG Data);

#ifdef CONFIG_STA_SUPPORT
INT RTMP_STA_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pParm);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB);
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNIFFER_RADIOTAP_SUPPORT
INT Set_SnifferBox_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_Hw_Auto_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef SNIFFER_SUPPORT
INT Set_MonitorMode_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT Set_MonitorFilterSize_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT Set_MonitorFrameType_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT	Set_MonitorMacFilter_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);
INT	Set_MonitorMacFilterOff_Proc(RTMP_ADAPTER	*pAd, RTMP_STRING *arg);

#endif /* SNIFFER_SUPPORT */

INT Set_VcoPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SINGLE_SKU */

ULONG build_qos_null_injector(struct wifi_dev *wdev, UINT8 *f_buf);

VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UINT16 AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP);

VOID RtmpCleanupPsQueue(
	IN  RTMP_ADAPTER *pAd,
	IN  PQUEUE_HEADER   pQueue);

NDIS_STATUS RtmpInsertPsQueue(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Aid, BOOLEAN isActive);
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UINT16 wcid, UCHAR Psm);

#ifdef ZERO_LOSS_CSA_SUPPORT
VOID UpdateSkipTX(RTMP_ADAPTER *pAd, UINT16 wcid, int set);
INT ReadSkipTX(RTMP_ADAPTER *pAd, UINT16 wcid);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef CONFIG_MULTI_CHANNEL
VOID RtmpEnqueueLastNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pAddr,
	IN UCHAR TxRate,
	IN UCHAR PID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR PwrMgmt,
	IN UCHAR OpMode);

VOID EnableMACTxPacket(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bTxNullFramei,
	IN UCHAR QSel);

VOID DisableMACTxPacket(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN UCHAR QSel);

VOID InitMultiChannelRelatedValue(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN UCHAR CentralChannel);

VOID EDCA_ActionTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID HCCA_ActionTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

NDIS_STATUS MultiChannelThreadInit(RTMP_ADAPTER *pAd);
BOOLEAN MultiChannelThreadExit(RTMP_ADAPTER *pAd);
VOID MultiChannelTimerStop(RTMP_ADAPTER *pAd);
VOID MultiChannelTimerStart(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY  *pEntry);
#endif /* CONFIG_MULTI_CHANNEL */

VOID dev_rx_802_11_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_HOTSPOT_R2
VOID hotspot_send_dls_resp(IN PRTMP_ADAPTER pAd, IN RX_BLK *pRxBlk);
#endif
#endif
#ifdef CONFIG_STA_SUPPORT
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg);
VOID RtmpPsActiveExtendCheck(RTMP_ADAPTER *pAd);
VOID RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode);
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
void DisplayTxAgg(RTMP_ADAPTER *pAd);
#endif /* DOT11_N_SUPPORT */

VOID set_default_ap_edca_param(EDCA_PARM *pEdca);
VOID set_default_sta_edca_param(EDCA_PARM *pEdca);

UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps);
UCHAR dot11_max_sup_rate(struct legacy_rate *rate);

VOID TRTableResetEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx);
VOID TRTableInsertEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx, MAC_TABLE_ENTRY *pEntry);
VOID TRTableInsertMcastEntry(RTMP_ADAPTER *pAd, UINT16 tr_tb_idx, struct wifi_dev *wdev);
VOID TRTableEntryDump(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line);
VOID MgmtTableSetMcastEntry(RTMP_ADAPTER *pAd, UINT16 wcid);
VOID DataTableSetMcastEntry(RTMP_ADAPTER *pAd, UINT16 wcid);
VOID MacTableSetEntryPhyCfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
VOID MacTableReset(RTMP_ADAPTER *pAd);
VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr);
MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev);
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR *pAddr);
MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll);

#ifdef WTBL_TDD_SUPPORT
INT32 MacTableDelEntryFromHash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
void aid_clear(struct _aid_info *aid_info, UINT16 aid);
#endif /* WTBL_TDD_SUPPORT */

VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID dump_txblk(RTMP_ADAPTER *pAd, TX_BLK *txblk);

#ifdef RTMP_MAC_PCI
VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD);
#endif /* RTMP_MAC_PCI */

#ifdef RXD_WED_SCATTER_SUPPORT
VOID dump_rxd_history(RTMP_ADAPTER *pAd);
VOID dump_rxd_scatter_history(RTMP_ADAPTER *pAd);
INT show_rxdinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RXD_WED_SCATTER_SUPPORT */

INT set_no_bcn(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(CONFIG_CSO_SUPPORT) || defined(CONFIG_TSO_SUPPORT)
INT rlt_net_acc_init(RTMP_ADAPTER *pAd);
#endif
INT SetRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value);
int read_reg(struct _RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value);


#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData);
VOID StatHERxRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UINT32 RawData);
#endif /* MT_MAC */

#ifdef CONFIG_MULTI_CHANNEL
VOID Start_MCC(RTMP_ADAPTER *pAd);
VOID Stop_MCC(RTMP_ADAPTER *pAd, INT channel);
#endif /* CONFIG_MULTI_CHANNEL */

BOOLEAN wmode_band_equal(USHORT smode, USHORT tmode);
UCHAR wmode_2_rfic(USHORT PhyMode);

struct wifi_dev *get_wdev_by_ioctl_idx_and_iftype(RTMP_ADAPTER *pAd, INT idx, INT iftype);
struct wifi_dev *get_wdev_by_idx(RTMP_ADAPTER *pAd, INT idx);

#ifdef SNIFFER_SUPPORT
VOID Monitor_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);
VOID Monitor_Remove(RTMP_ADAPTER *pAd);
BOOLEAN Monitor_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
BOOLEAN Monitor_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
#endif /* SNIFFER_SUPPORT */

#ifdef CONFIG_FWOWN_SUPPORT
VOID FwOwn(RTMP_ADAPTER *pAd);
INT32 DriverOwn(RTMP_ADAPTER *pAd);
BOOLEAN FwOwnSts(RTMP_ADAPTER *pAd);
#endif

#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT	Set_ErrDetectMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);
#endif /* ERR_RECOVERY */

#ifdef CONFIG_STA_SUPPORT
PSTA_ADMIN_CONFIG GetStaCfgByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev);
MAC_TABLE_ENTRY *GetAssociatedAPByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev);
VOID sta_mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry);
#endif /* CONFIG_STA_SUPPORT */

VOID mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry);
VOID mac_entry_delete(struct _RTMP_ADAPTER	*ad, struct _MAC_TABLE_ENTRY *entry);

INT32 MtAcquirePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset);
void MtReleasePowerControl(RTMP_ADAPTER *pAd, UINT32 Offset);

#ifdef BACKGROUND_SCAN_SUPPORT
INT set_background_scan(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_background_scan_notify(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_background_scan_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* BACKGROUND_SCAN_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
INT32 Set_RBIST_Switch_Mode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Set_RBIST_Capture_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_Capture_Status(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_Raw_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
INT32 Get_RBIST_IQ_Data(
	IN RTMP_ADAPTER *pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num);
INT32 Get_RBIST_IQ_Data_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
UINT32 Get_System_CenFreq_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode);
INT32 Get_System_Wireless_Info(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(PHY_ICS_SUPPORT)
INT32 Set_PhyIcs_Start(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);
#endif /* defined(PHY_ICS_SUPPORT) */

UINT32 Get_System_CapNode_Info(
	IN RTMP_ADAPTER *pAd);
UINT8 Get_System_Bw_Info(
	IN RTMP_ADAPTER *pAd,
	IN UINT32 CapNode);


#define QA_IRR_WF0 1
#define QA_IRR_WF1 2
#define QA_IRR_WF2 4
#define QA_IRR_WF3 8
#define WF0 0
#define WF1 1
#define WF2 2
#define WF3 3
#define WF_NUM 4
#define BITMAP_WF0 1
#define BITMAP_WF1 2
#define BITMAP_WF2 4
#define BITMAP_WF3 8
#define BITMAP_WF_ALL 15
INT Set_IRR_ADC(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_RxGain(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_TTG(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_IRR_TTGOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_manual_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cca_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

typedef enum _Stat_Action {
	RESET_COUNTER = 0,
	SHOW_RX_STATISTIC
} Stat_Action;

INT Set_Rx_Vector_Control(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);

INT Show_Rx_Statistic(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SCSPdThrRange_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_SCSPd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_SCSinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef SCS_FW_OFFLOAD
INT Set_SCSDefaultEnable(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ScsEnable);
INT SendSCSDataProc(RTMP_ADAPTER *pAd);
INT SendSCSDataProc_CONNAC3(RTMP_ADAPTER *pAd, UINT8 u1BandIdx);
INT Show_SCS_FW_Offload_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSCSinfo_ver2_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef DYNAMIC_WMM_SUPPORT
INT SetDynamicWmmEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SendDynamicWmmDataProc(RTMP_ADAPTER *pAd, UINT8 u1BandIdx);
#endif /* DYNAMIC_WMM_SUPPORT */
#ifdef SWACI_MECHANISM
INT32 Set_LNAGain_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetRcpiTestMode(IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
INT32 Set_LNATable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNACondition_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNADenseParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNATimer_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNAEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNAThreshConfig_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 Set_LNATestMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT Set_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DHCP_UC_SUPPORT
UINT16 RTMP_UDP_Checksum(IN PNDIS_PACKET pSkb);
#endif /* DHCP_UC_SUPPORT */

INT set_support_rate_table_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_support_rate_table_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ra_dbg_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetDecreasePwrCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCCKTxStream(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_mgmt_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_EAP_FEATURE
INT SetInitIPICtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowIPIValue(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_data_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fw_ratbl_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_ratbl_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_EAP_FEATURE */
INT ShowEDCCAThreshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowEDCCAEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetEDCCAThresholdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetEDCCAEnableCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_GPIO_CTRL
INT set_gpio_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_gpio_value(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_GPIO_CTRL */
INT SetRxvEnCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvRuCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvRawDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvStatReset(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvLogCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvListInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRxvRptInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LINK_TEST_SUPPORT */
INT SetMuTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DATA_TXPWR_CTRL
INT SetTxPwrDataFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxPwrLimitDataFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT update_data_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, UINT32 u4Mcs, UINT32 u4Bw, INT32 i4Offset);
INT show_data_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT SetBFNDPATxDCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalTaskInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalTaskCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectDisable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectDutyCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectDutyInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetThermalProtectStateAct(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef TX_POWER_CONTROL_SUPPORT
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TX_POWER_CONTROL_SUPPORT */
INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef	ETSI_RX_BLOCKER_SUPPORT
INT SetFixWbIbRssiCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetRssiThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCheckThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetAdaptRxBlockCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetWbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetIbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT ShowRssiThInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

INT TxPowerManualCtrl(PRTMP_ADAPTER	pAd, IN UCHAR ucBandIdx, IN INT_8 cTxPower, IN UINT8 ucPhyMode, IN UINT8 ucTxRate, IN UINT8 ucBW);
INT SetTxPwrManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef AMPDU_CONF_SUPPORT
INT Set_AMPDU_MPDU_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

/*TPC Algo control*/
INT TpcManCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTpcManual);
INT TpcEnableCfg(PRTMP_ADAPTER pAd, BOOLEAN fgTpcEnable);
INT TpcWlanIdCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgUplink, UINT8 u1EntryIdx, UINT16 u2WlanId, UINT8 u1DlTxType);
INT TpcUlAlgoCtrl(PRTMP_ADAPTER pAd, UINT8 u1TpcCmd, UINT8 u1ApTxPwr, UINT8 u1EntryIdx, UINT8 u1TargetRssi, UINT8 u1UPH, BOOLEAN fgMinPwrFlag);
INT TpcDlAlgoCtrl(PRTMP_ADAPTER pAd, UINT8 u1TpcCmd, BOOLEAN fgCmdCtrl, UINT8 u1DlTxType, CHAR DlTxPwr, UINT8 u1EntryIdx, INT16 DlTxpwrAlpha);
INT TpcUlUtVarCfg(PRTMP_ADAPTER pAd, UINT8 u1EntryIdx, UINT8 u1VarType, INT16 i2Value);
INT TpcAlgoUtGo(PRTMP_ADAPTER pAd, BOOLEAN fgTpcUtGo);

INT TpcManTblInfo(PRTMP_ADAPTER pAd, BOOLEAN fgUplink);
INT SetTpcManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcWlanIdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcUlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcDlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcManTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcAlgoUlUtCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTpcAlgoUlUtGo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT support_rate_table_ctrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 tx_mode,
	IN UINT8 tx_nss,
	IN UINT8 tx_bw,
	IN UINT16 *mcs_cap);

INT support_rate_table_info(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 tx_mode,
	IN UINT8 tx_nss,
	IN UINT8 tx_bw,
	IN UINT16 *mcs_cap);

INT ra_dbg_ctrl(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 param_num,
	IN UINT32 *param);

INT TxPowerPercentCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxPowerPercentEn, UCHAR ucBandIdx);
INT TxPowerDropCtrl(PRTMP_ADAPTER pAd, UINT8 ucPowerDrop, UCHAR ucBandIdx);
INT TxPowerSKUCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxPowerSKUEn, UCHAR ucBandIdx);
INT TxPowerBfBackoffCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgTxBFBackoffEn, UCHAR ucBandIdx);
INT TxCCKStreamCtrl(PRTMP_ADAPTER pAd, UINT8 u1CCKTxStream, UCHAR ucBandIdx);
INT ThermoCompCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgThermoCompEn, UCHAR ucBandIdx);
INT TxPowerRfTxAnt(PRTMP_ADAPTER pAd, UINT8 ucTxAntIdx);
INT TxPowerShowInfo(PRTMP_ADAPTER pAd, UCHAR ucTxPowerInfoCatg, UINT8 ucBandIdx);
INT show_BSSEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_APEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_EAP_FEATURE
INT InitIPICtrl(PRTMP_ADAPTER pAd, UINT8 BandIdx);
INT GetIPIValue(PRTMP_ADAPTER pAd, UINT8 BandIdx);
INT SetDataTxPwrOffset(PRTMP_ADAPTER pAd, UINT16 WlanIdx, INT8 TxPwr_Offset,
		UINT8 BandIdx);
INT SetFwRaTable(PRTMP_ADAPTER pAd, UINT8 BandIdx, UINT8 TblType,
		UINT8 TblIndex, UINT16 TblLength, PUCHAR Buffer);
INT GetRaTblInfo(PRTMP_ADAPTER pAd, UINT8 BandIdx, UINT8 TblType,
		UINT8 TblIndex, UINT8 ReadnWrite);
#endif /* WIFI_EAP_FEATURE */
INT SetEDCCAEnable(PRTMP_ADAPTER pAd, UINT8 u1EDCCACtrl, UINT8 u1BandIdx, UINT8 u1EDCCAStd, INT8 i1compensation);
INT SetEDCCAThreshold(PRTMP_ADAPTER pAd, UINT8 edcca_threshold[], UINT8 BandIdx);
#ifdef OFFCHANNEL_SCAN_FEATURE
NDIS_STATUS EDCCAScanForCompensation(PRTMP_ADAPTER	pAd,	struct wifi_dev *wdev);
#endif
NDIS_STATUS EDCCAInit(IN PRTMP_ADAPTER pAd, UINT8 u1BandIdx);

#ifdef WIFI_GPIO_CTRL
INT SetGpioCtrl(PRTMP_ADAPTER pAd, UINT8 GpioIdx, BOOLEAN GpioEn);
INT SetGpioValue(PRTMP_ADAPTER pAd, UINT8 GpioIdx, UINT8 GpioVal);
#endif /* WIFI_GPIO_CTRL */
INT TOAECtrlCmd(PRTMP_ADAPTER pAd, UCHAR TOAECtrl);
INT MuPwrCtrlCmd(PRTMP_ADAPTER pAd, BOOLEAN fgMuTxPwrManEn, CHAR cMuTxPwr, UINT8 u1BandIdx);
#ifdef DATA_TXPWR_CTRL
INT TxPwrDataFrameCtrlCmd(IN PRTMP_ADAPTER pAd, IN MAC_TABLE_ENTRY *pEntry, IN INT8 i1MaxBasePwr, IN UINT8 u1BandIdx);
INT TxPwrMinLimitDataFrameCtrl(IN PRTMP_ADAPTER pAd, IN INT8 i1MinBasePwr, IN UINT8 u1BandIdx);
#endif
INT BFNDPATxDCtrlCmd(PRTMP_ADAPTER pAd, BOOLEAN fgNDPA_ManualMode, UINT8 ucNDPA_TxMode, UINT8 ucNDPA_Rate, UINT8 ucNDPA_BW, UINT8 ucNDPA_PowerOffset);
INT RxvEnCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgRxvEnCtrl);
INT RxvRuCtrl(PRTMP_ADAPTER pAd, UINT8 u1RxvRuCtrl);

INT TemperatureCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgManualMode, CHAR cTemperature);
INT ThermalManCtrl(IN PRTMP_ADAPTER pAd, IN UINT8 u1BandIdx, IN BOOLEAN fgManualMode, IN UINT8 u1ThermalAdc);
INT ThermalTaskCtrl(IN PRTMP_ADAPTER pAd, IN UINT8 u1BandIdx, IN BOOLEAN fgTrigEn, IN UINT8 u1Thres);
INT ThermalTaskAction(IN PRTMP_ADAPTER pAd, IN UINT8 u4PhyIdx, IN UINT32 u4ThermalTaskProp, IN UINT8 u1ThermalAdc);

INT ThermalBasicInfo(IN PRTMP_ADAPTER pAd, IN UINT8 u1BandIdx);

#ifdef TX_POWER_CONTROL_SUPPORT
INT TxPwrUpCtrl(PRTMP_ADAPTER pAd, UINT8 ucBandIdx, CHAR cPwrUpCat,
		signed char *cPwrUpValue, UCHAR cPwrUpValLen);
#endif /* TX_POWER_CONTROL_SUPPORT */

UINT8 TxPowerGetChBand(UINT8 ucBandIdx, UINT8 CentralCh);
#ifdef TPC_SUPPORT
INT TxPowerTpcFeatureCtrl(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, INT8 TpcPowerValue);
#ifndef TPC_MODE_CTRL
INT TxPowerTpcFeatureForceCtrl(PRTMP_ADAPTER pAd, INT8 TpcPowerValue, UINT8 ucBandIdx, UINT8 CentralChannel);
#else
INT TxPowerTpcFeatureForceCtrl(PRTMP_ADAPTER pAd, INT8 TpcPowerValue, UINT8 ucBandIdx,
UINT8 CentralChannel, UINT16 wcid);
#endif
#endif /* TPC_SUPPORT */

#define RETURN_STATUS_TRUE	   0

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

enum {
	MSDU_FORMAT,
	FINAL_AMSDU_FORMAT,
	MIDDLE_AMSDU_FORMAT,
	FIRST_AMSDU_FORMAT,
};

#define RETURN_IF_PAD_NULL(_pAd)	\
	{								   \
		if (_pAd == NULL) {			\
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unexpected pAd NULL!\n");	   \
			return;						\
		}								\
	}
#define RETURN_ZERO_IF_PAD_NULL(_pAd)	\
	{								   \
		if (_pAd == NULL) {				\
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unexpected pAd NULL!\n");	   \
			return 0;					\
		}								\
	}

/* PROFILE to MBSSID index mapping */
#define PF_TO_BSS_IDX(_pAd, _pfIdx)		((_pAd)->ApCfg.Pf2MbssIdxMap[_pfIdx])

UINT32 Get_OBSS_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
VOID Reset_OBSS_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_My_Tx_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_My_Rx_AirTime(PRTMP_ADAPTER pAd, UCHAR BandIdx);
UINT32 Get_EDCCA_Time(PRTMP_ADAPTER pAd, UCHAR BandIdx);
VOID CCI_ACI_scenario_maintain(PRTMP_ADAPTER pAd);
#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)
VOID Mumimo_scenario_maintain(PRTMP_ADAPTER	pAd);
#endif

#ifdef LED_CONTROL_SUPPORT
INT	Set_Led_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
BOOLEAN is_testmode_wdev(UINT32 wdev_type);
INT wdev_edca_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);

#ifdef SW_CONNECT_SUPPORT
BOOLEAN wdev_dummy_obj_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
#endif /* SW_CONNECT_SUPPORT */

struct wifi_dev *get_default_wdev(struct _RTMP_ADAPTER *ad);
void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2);
BOOLEAN IsPublicActionFrame(PRTMP_ADAPTER pAd, VOID *pbuf);

#ifdef ERR_RECOVERY
INT32 ShowSerProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowSerProc2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT32 ShowBcnProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetMuMimoFixedRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetMuMiMoFixedGroupRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if (defined(CFG_SUPPORT_FALCON_MURU) || defined(CFG_SUPPORT_MU_MIMO))
INT ShowMuMimoGroupTblEntry(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuMimoForceMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetMuMimoForceMUEnable(RTMP_ADAPTER *pAd, BOOLEAN fgForceMu);
#endif

#if (defined(CFG_SUPPORT_MU_MIMO_RA) || defined(CFG_SUPPORT_FALCON_MURU))
INT ShowMuMimoAlgorithmMonitor(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef WIFI_CSI_CN_INFO_SUPPORT
INT32 set_csi_cn_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 show_csi_data_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

#ifdef MT_FDB
void fdb_enable(struct _RTMP_ADAPTER *pAd);
INT show_fdb_n9_log(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_fdb_cr4_log(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT_FDB */

INT wdev_do_open(struct wifi_dev *wdev);
INT wdev_do_close(struct wifi_dev *wdev);
INT wdev_do_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wdev_do_linkdown(struct wifi_dev *wdev);
INT wdev_do_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wdev_do_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
#ifdef CON_WPS
VOID APMlmeScanCompleteAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem);
#endif /* CON_WPS */

#if defined(WH_EZ_SETUP) || defined(CONFIG_MAP_SUPPORT)
VOID APMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem);
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
INT Set_ScanResults_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING	* arg);
INT Set_ApScan_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN 	RTMP_STRING	* arg);

INT Channel_Info_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, POS_COOKIE pObj);
#endif
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
VOID ReadChannelStats(IN PRTMP_ADAPTER   pAd);
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE

INT Set_ScanResults_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT Set_ApScan_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT Channel_Info_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, POS_COOKIE pObj);

VOID ReadChannelStats(IN PRTMP_ADAPTER   pAd);
#endif

typedef enum _EDCCA_BW_ID_T {
	EDCCA_BW20 = 0,
	EDCCA_BW40,
	EDCCA_BW80,
	EDCCA_MAX_BW_NUM,
	EDCCA_RESET_NUM = 127
} EDCCAB_BW_ID_T;

enum {
	EDCCA_Default = 0,
	EDCCA_Country_FCC6G = 1,
	EDCCA_Country_ETSI = 2
};

#ifdef MULTI_PROFILE
typedef enum MTB_PROFILE_ID_T {
	MTB_EXT_PROFILE,
	MTB_MERGE_PROFILE,
	MTB_DEV_PREFIX
} MTB_PROFILE_ID;

#ifdef DBDC_ONE_BAND1_SUPPORT
UCHAR *get_dbdcdev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type);
#endif
NDIS_STATUS update_mtb_value(RTMP_ADAPTER *pAd, UCHAR data_id, UINT_32 extra, RTMP_STRING *value);
INT	multi_profile_apcli_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name,
									INT *ifidx);
INT	multi_profile_wds_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name,
									INT ifidx);
UCHAR is_multi_profile_enable(struct _RTMP_ADAPTER *ad);
UCHAR multi_profile_get_pf1_num(struct _RTMP_ADAPTER *ad);
UCHAR multi_profile_get_pf2_num(struct _RTMP_ADAPTER *ad);
#endif /*MULTI_PROFILE*/

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
typedef enum _ENUM_PCIE_ASPM_TYPE_T {
	PCIE_ASPM_TYPE_L0S,
	PCIE_ASPM_TYPE_L1,
	PCIE_ASPM_TYPE_NUM,
} ENUM_PCIE_ASPM_TYPE_T, *P_ENUM_PCIE_ASPM_TYPE_T;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT build_extra_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_extended_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_rsn_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
INT build_wsc_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_wmm_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info);
INT build_supp_op_class_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf);
ULONG build_support_rate_ie(struct wifi_dev *wdev, UCHAR *sup_rate, UCHAR sup_rate_len, UCHAR *buf);
ULONG build_support_ext_rate_ie(struct wifi_dev *wdev, UCHAR sup_rate_len,
	UCHAR *ext_sup_rate, UCHAR ext_sup_rate_len, UCHAR *buf);
ULONG build_bss_max_idle_ie(struct wifi_dev *wdev, UINT8 *buf, UINT16 max_idle_period, UINT8 option);
INT parse_support_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr);
INT parse_support_ext_rate_ie(struct legacy_rate *rate, EID_STRUCT *eid_ptr);

#ifdef OOB_CHK_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
BOOLEAN parse_mbssid_non_tx_bssid_cap_ie(EID_STRUCT *eid_ptr);
BOOLEAN parse_mbssid_idx_ie(EID_STRUCT *eid_ptr);
#endif /* DOT11V_MBSSID_SUPPORT */
BOOLEAN parse_ext_cap_ie(PEXT_CAP_INFO_ELEMENT pExtCapInfo, EID_STRUCT *eid_ptr);
#endif /* OOB_CHK_SUPPORT */

VOID parse_RXV_packet_v1(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data);
VOID parse_RXV_packet_v2(RTMP_ADAPTER *pAd, UINT32 Type, RX_BLK *RxBlk, UCHAR *Data);
SCAN_CTRL *get_scan_ctrl_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
PBSS_TABLE get_scan_tab_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT parse_ssid_ie(EID_STRUCT *eid_ptr);
INT parse_ds_parm_ie(EID_STRUCT *eid_ptr);
INT parse_cf_parm_ie(EID_STRUCT *eid_ptr);
INT parse_ibss_parm_ie(EID_STRUCT *eid_ptr);
INT parse_qbss_load_ie(EID_STRUCT *eid_ptr);
INT parse_tpc_report_ie(EID_STRUCT *eid_ptr);
INT parse_ch_switch_announcement_ie(EID_STRUCT *eid_ptr);
INT parse_measurement_ie(UCHAR Len);
INT parse_erp_ie(EID_STRUCT *eid_ptr);
INT parse_overlapbss_scan_parm_ie(EID_STRUCT *eid_ptr);
INT parse_rm_enable_cap_ie(EID_STRUCT *eid_ptr);
INT parse_wapi_ie(EID_STRUCT *eid_ptr);
INT parse_sec_ch_offset_ie(EID_STRUCT *eid_ptr);
INT parse_ht_info_ie(EID_STRUCT *eid_ptr);
INT parse_ext_ch_switch_announcement_ie(EID_STRUCT *eid_ptr);
INT parse_ft_timeout_ie(EID_STRUCT *eid_ptr);
INT parse_ft_ie(EID_STRUCT *eid_ptr);
INT parse_md_ie(EID_STRUCT *eid_ptr);
INT parse_country_ie(EID_STRUCT *eid_ptr);
INT parse_rsn_ie(EID_STRUCT *eid_ptr);
INT parse_qos_cap_ie(EID_STRUCT *eid_ptr);
INT parse_ht_cap_ie(UCHAR Len);

BOOLEAN sync_fsm_ops_init(struct wifi_dev *wdev);
VOID wdev_fsm_init(struct wifi_dev *wdev);


struct traffic_notify_info {
	struct _RTMP_ADAPTER *ad;
	void *v;
};

enum {
	TRAFFIC_NOTIFY_RX_DATA,
	TRAFFIC_NOTIFY_RX_TXS,
	TRAFFIC_NOTIFY_RX_TXRXV,
	TRAFFIC_NOTIFY_RX_EVENT,
	TRAFFIC_NOTIFY_RX_TMR,
	TRAFFIC_NOTIFY_RX_TXRX_NOTIFY,
	TRAFFIC_NOTIFY_WMM_DETECT,
	TRAFFIC_NOTIFY_TPUT_DETECT,
	TRAFFIC_NOTIFY_MAX
};

enum {
	TRAFFIC_NOTIFY_PRIORITY_DEFAULT = 0,
	TRAFFIC_NOTIFY_PRIORITY_DVT,
};

INT call_traffic_notifieriers(INT val, struct _RTMP_ADAPTER *ad, void *v);
INT register_traffic_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne);
INT unregister_traffic_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne);

#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
		_fragFrame.wcid = 0;		\
		_fragFrame.sec_mode = 0;	\
		_fragFrame.LastPN = 0;		\
		_fragFrame.sec_on = FALSE;  \
	}

INT	Set_PtkRekey_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);


INT Set_BSSAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_BSSTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_APTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef MGMT_TXPWR_CTRL
INT set_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT update_mgmt_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif

#ifdef IWCOMMAND_CFG80211_SUPPORT
INT set_apmacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_apclimacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#ifdef ANTENNA_CONTROL_SUPPORT
INT Antenna_Control_Init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT Set_Antenna_Control_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Antenna_Control_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* ANTENNA_CONTROL_SUPPORT */
#if defined(ANDLINK_FEATURE_SUPPORT) || defined(TR181_SUPPORT)
/**
 * enum station_info_rate_flags - bitrate info flags
 *
 * Used by the driver to indicate the specific rate transmission
 * type for 802.11n transmissions.
 *
 * @MTK_RATE_INFO_FLAGS_MCS: mcs field filled with HT MCS
 * @MTK_RATE_INFO_FLAGS_VHT_MCS: mcs field filled with VHT MCS
 * @MTK_RATE_INFO_FLAGS_SHORT_GI: 400ns guard interval
 * @MTK_RATE_INFO_FLAGS_DMG: 60GHz MCS
 * @MTK_RATE_INFO_FLAGS_HE_MCS: HE MCS information
 * @MTK_RATE_INFO_FLAGS_EDMG: 60GHz MCS in EDMG mode
 */
enum mtk_rate_info_flags {
	MTK_RATE_INFO_FLAGS_MCS			= BIT(0),
	MTK_RATE_INFO_FLAGS_VHT_MCS			= BIT(1),
	MTK_RATE_INFO_FLAGS_SHORT_GI		= BIT(2),
	MTK_RATE_INFO_FLAGS_DMG			= BIT(3),
	MTK_RATE_INFO_FLAGS_HE_MCS			= BIT(4),
	MTK_RATE_INFO_FLAGS_EDMG			= BIT(5),
};

typedef struct mtk_rate_info{
	u8 flags;
	u8 mcs;
	u16 legacy;
	u8 nss;
	u8 bw;
	u8 gi;
}mtk_rate_info_t;



INT get_sta_rate_info(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	mtk_rate_info_t *tx_rate_info,
	mtk_rate_info_t *rx_rate_info);


INT set_andlink_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_andlink_ip_hostname_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef ANDLINK_V4_0
INT set_andlink_simple_val(RTMP_ADAPTER * pAd,RTMP_STRING * arg);
INT set_andlink_uplink_period(RTMP_ADAPTER * pAd,RTMP_STRING * arg);
INT set_andlink_uplink_sample(RTMP_ADAPTER * pAd,RTMP_STRING * arg);
INT set_andlink_sta_period(RTMP_ADAPTER * pAd,RTMP_STRING * arg);
INT set_andlink_sta_sample(RTMP_ADAPTER * pAd,RTMP_STRING * arg);
#endif/*ANDLINK_V4_0*/

#endif/*ANDLINK_FEATURE_SUPPORT*/

#ifdef ACK_CTS_TIMEOUT_SUPPORT
#define INVALID_DISTANCE								0

typedef enum ACK_TIMEOUT_MODE {
	CCK_TIME_OUT,
	OFDM_TIME_OUT,
	OFDMA_TIME_OUT,
	ACK_ALL_TIME_OUT
} ACK_TIMEOUT_MODE_T;

INT set_ack_timeout_mode_byband(
	RTMP_ADAPTER *pAd,
	UINT32 timeout,
	UINT32 bandidx,
	ACK_TIMEOUT_MODE_T ackmode);

UINT32 get_ack_timeout_bycr(RTMP_ADAPTER *pAd, UINT32 reg_addr, UINT32 *ptimeout);
INT set_ack_timeout_cr(RTMP_ADAPTER *pAd, UINT32 type, UINT32 timeout);
INT set_datcfg_ack_cts_timeout(RTMP_ADAPTER *pAd);
INT set_ackcts_timeout_enable_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_cck_ack_timeout_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ofdm_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ofdma_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_dst2acktimeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_distance_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_cck_ack_timeout_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_ofdm_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_ofdma_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#ifdef MLME_MULTI_QUEUE_SUPPORT
INT set_mlme_queue_ration(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

INT SetRtsThenCtsRetryCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WTBL_TDD_SUPPORT
/* --WTBL TDD TMP HEADER */
INT WtblTdd_DbgSet(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
INT WtblTdd_TimeLogAction(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
INT WtblAll_DumpAction(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
INT WtblEntry_DumpAction(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
#endif /* WTBL_TDD_SUPPORT */
#ifdef IXIA_C50_MODE
#define C50_ARP_LEN 28

#define dectlen_l  84
#define dectlen_m  508
#define dectlen_h  1514	/*ETH header(14b) + IP header(20b) + UDP header(8b) + payload(1472b) + CRC(4b), ignore CRC, so it's 1514 bytes*/

#define IS_EXPECTED_LENGTH(pAd, len) ((len == (dectlen_l - pAd->ixia_ctl.pkt_offset))\
		|| (len == (dectlen_m - pAd->ixia_ctl.pkt_offset))\
		|| (len == (dectlen_h - pAd->ixia_ctl.pkt_offset)))

INT Set_pktlen_offset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ForceIxia_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ixia_round_reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ixia_debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
void wifi_txrx_parmtrs_dump(RTMP_ADAPTER *pAd);
VOID periodic_detect_tx_pkts(RTMP_ADAPTER *pAd);
#endif
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
void tpc_req_entry_reset(IN PRTMP_ADAPTER pAd);
void tpc_req_entry_decision(IN PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry);
void tpc_req_monitor(IN PRTMP_ADAPTER pAd);
void send_tpc_request_trigger(RTMP_ADAPTER *pAd);
VOID mlmeAPSendTPCReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
void set_wtbl_pwr_by_entry(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *entry);
VOID TPCRepTimeout(IN PRTMP_ADAPTER pAd, IN MLME_QUEUE_ELEM *Elem);
INT Show_TPC_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_tpc_RssiThld_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcInterval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcCtrlMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_TpcLinkMargin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
BOOLEAN wifi_transmit_tpcreq(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct wifi_dev *wdev);
#endif
#endif
static inline int os_snprintf_error(size_t size, int res)
{
	return res < 0 || (unsigned int) res >= size;
}
/*API Start: for OOB Task*/
UINT parse_vht_cap_ie(UCHAR eid_len);
UINT parse_vht_op_ie(UCHAR eid_len);
UINT parse_wide_bw_channel_switch_ie(UCHAR eid_len);
UINT parse_operating_mode_notification_ie(UCHAR eid_len);
UINT parse_transmit_power_envelope_ie(UCHAR power_cnt, UCHAR eid_len);
UINT parse_channel_switch_wrapper_ie(PEID_STRUCT pEid, UCHAR eid_len);
UINT parse_reduced_neighbor_report_ie(PEID_STRUCT pEid);
UINT parse_fine_timing_measu_para_ie(UCHAR ftm_len);
UINT parse_twt_ie(struct itwt_ie *twt_ie);
UINT parse_fils_indication_ie(PEID_STRUCT pEid);
UINT parse_rsn_ext_ie(PEID_STRUCT pEid);
UINT parse_fils_request_para_ie(PEID_STRUCT pEid);
UINT parse_mu_edca_ie(UCHAR ie_len);
UINT parse_spatial_resue_ie(UCHAR *pie, UCHAR ie_len);
UINT parse_short_ssid_list_ie(PEID_STRUCT pEid, UCHAR *pie);
/*API End: for OOB Task*/

#endif  /* __RTMP_H__ */

