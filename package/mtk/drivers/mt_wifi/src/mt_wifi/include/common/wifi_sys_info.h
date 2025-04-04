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

	Module Name:
	wifi_sys_info.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __WIFI_SYS_INFO_H__
#define __WIFI_SYS_INFO_H__

#include "common/link_list.h"
#include "wifi_sys_notify.h"
#include "mgmt/be_export.h"

#ifdef DOT11_HE_AX
#include "he.h"
#include "bss_color.h"
#endif
#include "mcu/mt_cmd.h"


struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _IE_lists;
struct _STA_TR_ENTRY;
struct _IE_lists;

#ifdef APCLI_SUPPORT
struct _STA_ADMIN_CONFIG;
#endif

#define WIFI_SYS_POLL_MAX 10
#define WIFI_SYS_PILL_PERIOD 100

enum {
	WSYS_NOTIFY_OPEN,
	WSYS_NOTIFY_CLOSE,
	WSYS_NOTIFY_CONNT_ACT,
	WSYS_NOTIFY_DISCONNT_ACT,
	WSYS_NOTIFY_LINKUP,
	WSYS_NOTIFY_LINKDOWN,
	WSYS_NOTIFY_STA_UPDATE,
};

enum {
	WSYS_NOTIFY_PRIORITY_DEFAULT = 0,
	WSYS_NOTIFY_PRIORITY_QM,
	WSYS_NOTIFY_PRIORITY_MLME,
	WSYS_NOTIFY_PRIORITY_DVT,
};

typedef struct _DEV_INFO_CTRL_T {
	UINT8 OwnMacIdx;
	UINT8 OwnMacAddr[MAC_ADDR_LEN];
	UINT8 BandIdx;
	UINT8 WdevActive;
	UINT32 EnableFeature;
	VOID *priv;
	DL_LIST list;
} DEV_INFO_CTRL_T;


typedef struct _STA_REC_CTRL_T {
	UINT8 BssIndex;
	UINT16 WlanIdx;
#ifdef SW_CONNECT_SUPPORT
	UINT16 SwWlanIdx;
#endif /* SW_CONNECT_SUPPORT */
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
	UINT8 IsNewSTARec;
	ASIC_SEC_INFO asic_sec_info;
#ifdef DOT11_HE_AX
	struct he_sta_info he_sta;
#endif /*DOT11_HE_AX*/
	VOID *priv;
	DL_LIST list;
} STA_REC_CTRL_T;


struct _tx_burst_cfg {
	struct wifi_dev *wdev;
	UINT8 prio;
	UINT8 ac_type;
	UINT16 txop_level;
	UINT8 enable;
};

struct _part_wmm_cfg {
	UCHAR wmm_idx;
	UINT32 ac_num;
	UINT32 edca_type;
	UINT32 edca_value;
	struct wifi_dev *wdev;
};
/*use for update bssinfo*/
struct uapsd_config {
	BOOLEAN uapsd_en;
	UCHAR uapsd_trigger_ac;
};

extern UINT16 txop0;
extern UINT16 txop60;
extern UINT16 txop80;
extern UINT16 txopfe;


#define TXOP_0          (txop0)
#define TXOP_30         (0x30)
#define TXOP_60         (txop60)
#define TXOP_80         (txop80)
#define TXOP_A0         (0xA0)
#define TXOP_BB			(0xBB)
#define TXOP_C0         (0xC0)
#define TXOP_FE         (txopfe)
#define TXOP_138        (0x138)
#define TXOP_177        (0x177)

enum _tx_burst_prio {
	PRIO_DEFAULT = 0,
	PRIO_RDG,
	PRIO_NEAR_FAR,
	PRIO_2G_INFRA,
	PRIO_MU_MIMO,
	PRIO_MULTI_CLIENT,
	PRIO_PEAK_TP,
	PRIO_APCLI_REPEATER,
	PRIO_CCI,
	PRIO_WMM,
	MAX_PRIO_NUM
};

#define WDEV_BSS_STATE(__wdev) \
	(__wdev->bss_info_argument.bss_state)

typedef enum _BSSINFO_LINK_TO_OMAC_T {
	HW_BSSID = 0,
	EXTEND_MBSS,
	WDS,
	REPT
} BSSINFO_TYPE_T;

typedef enum _BSSINFO_STATE_T {
	BSS_INIT	= 0,	/* INIT state */
	BSS_INITED	= 1,	/* BSS Argument Link done */
	BSS_ACTIVE	= 2,	/* The original flag - Active */
	BSS_READY	= 3		/* BssInfo updated to FW done and ready for mlme FSM */
} BSSINFO_STATE_T;

typedef struct _BSS_INFO_ARGUMENT_T {
	BSSINFO_TYPE_T bssinfo_type;
	BSSINFO_STATE_T bss_state;
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT16 bmc_wlan_idx;
	UINT16 peer_wlan_idx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 CipherSuit;
	UINT8 WmmIdx;
	UINT8 ucPhyMode;
	UINT32 prio_bitmap;
	UINT16 txop_level[MAX_PRIO_NUM];
	UINT32 u4BssInfoFeature;
	CMD_BSSINFO_PM_T rBssInfoPm;
	HTTRANSMIT_SETTING BcTransmit;
	HTTRANSMIT_SETTING McTransmit;
#ifdef HIGHPRI_RATE_SPECIFIC
	/*ARP: 0, DHCP: 1, EAPOL: 2*/
	HTTRANSMIT_SETTING HighPriTransmit[HIGHPRI_MAX_TYPE];
#endif
	UINT16	bcn_period;
	UINT8	dtim_period;
	struct freq_oper chan_oper;
#ifdef CONFIG_STA_SUPPORT
#ifdef UAPSD_SUPPORT
	struct uapsd_config uapsd_cfg;
#endif /*UAPSD_SUPPORT*/
	CHAR dbm_to_roam;
#endif /*CONFIG_STA_SUPPORT*/
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	RA_COMMON_INFO_T ra_cfg;
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
#ifdef DOT11_HE_AX
	struct he_bss_info he_bss;
	struct bss_color_ctrl bss_color;
#endif
	struct prot_info prot;
#ifdef DOT11V_MBSSID_SUPPORT
	UINT8	max_bssid_indicator;	/* Max BSSID indicator. Range from 1 to 8, 0 means MBSSID disabled */
	UINT8	mbssid_index;			/* BSSID index of non-transmitted BSSID, 0 means transmitted BSSID */
#endif
#ifdef BCN_PROTECTION_SUPPORT
	struct bcn_protection_cfg bcn_prot_cfg;
#endif
	UINT8 ucBandIdx;
	BOOLEAN bBcnSntReq;
	UCHAR bUpdateReason;
	VOID *priv;
	/* member "list" must be the last one */
	DL_LIST list;
} BSS_INFO_ARGUMENT_T, *PBSS_INFO_ARGUMENT_T;

struct WIFI_SYS_CTRL {
	struct wifi_dev *wdev;
	DEV_INFO_CTRL_T DevInfoCtrl;
	STA_REC_CTRL_T StaRecCtrl;
	BSS_INFO_ARGUMENT_T BssInfoCtrl;
	VOID *priv;
	BOOLEAN skip_set_txop;
};

typedef struct _WIFI_INFO_CLASS {
	UINT32 Num;
	DL_LIST Head;
} WIFI_INFO_CLASS_T;

struct wsys_notify_info {
	struct wifi_dev *wdev;
	void *v;
};

/*for FW related information sync.*/
typedef struct _WIFI_SYS_INFO {
	WIFI_INFO_CLASS_T DevInfo;
	WIFI_INFO_CLASS_T BssInfo;
	WIFI_INFO_CLASS_T StaRec;
	struct notify_head wsys_notify_head;
	NDIS_SPIN_LOCK lock;
} WIFI_SYS_INFO_T;

typedef struct _PEER_LINKUP_HWCTRL {
#ifdef TXBF_SUPPORT
	struct _IE_lists ie_list;
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
} PEER_LINKUP_HWCTRL;

typedef struct _LINKUP_HWCTRL {
#ifdef TXBF_SUPPORT
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
} LINKUP_HWCTRL;

/*Export function*/
VOID wifi_sys_reset(struct _WIFI_SYS_INFO *wsys);
VOID wifi_sys_init(struct _RTMP_ADAPTER *ad);
VOID wifi_sys_dump(struct _RTMP_ADAPTER *ad);
INT wifi_sys_update_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _DEV_INFO_CTRL_T *devinfo);
INT wifi_sys_update_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, BSS_INFO_ARGUMENT_T *bss);
INT wifi_sys_update_starec(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new);
INT wifi_sys_update_starec_info(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new);
INT register_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne);
INT unregister_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne);
struct _STA_REC_CTRL_T *get_starec_by_wcid(struct _RTMP_ADAPTER *ad, INT wcid);
VOID del_starec(struct _RTMP_ADAPTER *ad, struct _STA_TR_ENTRY *tr_entry);

#ifdef CONFIG_AP_SUPPORT
#endif /*CONFIG_AP_SUPPORT*/

#ifdef APCLI_SUPPORT
#endif /*APCLI*/

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
VOID WifiSysRaInit(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID WifiSysUpdateRa(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, struct _STAREC_AUTO_RATE_UPDATE_T *prParam);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

BOOLEAN wifi_sys_op_lock(struct wifi_dev *wdev);
VOID wifi_sys_op_unlock(struct wifi_dev *wdev);

VOID WifiSysUpdatePortSecur(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, ASIC_SEC_INFO *asic_sec_info);
/*wifi system architecture layer api*/
INT wifi_sys_open(struct wifi_dev *wdev);
INT wifi_sys_close(struct wifi_dev *wdev);
INT wifi_sys_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
VOID update_sta_conn_state(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_linkdown(struct wifi_dev *wdev);
VOID wifi_mlme_ops_register(struct wifi_dev *wdev);
VOID wifi_sys_update_wds(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#ifdef CONFIG_VLAN_GTK_SUPPORT
INT wifi_vlan_starec_linkup(struct wifi_dev *wdev, int bmc_idx);
#endif

#ifdef WTBL_TDD_SUPPORT
UINT32 starec_feature_decision(
	struct wifi_dev *wdev,
	UINT32 conn_type,
	struct _MAC_TABLE_ENTRY *entry,
	UINT32 *feature);
#endif /* WTBL_TDD_SUPPORT	 */

#endif
