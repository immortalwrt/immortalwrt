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
	hotspot.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
#ifdef CONFIG_HOTSPOT
#ifndef CONFIG_DOT11V_WNM
#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11V_WNM"
#endif

#ifndef CONFIG_DOT11U_INTERWORKING
#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11U_INTERWORKING"
#endif
#endif

#include "rtmp.h"

#ifdef CONFIG_HOTSPOT
#define HSCTRL_MACHINE_BASE 0

enum HSCTRL_STATE {
	WAIT_HSCTRL_OFF,
	WAIT_HSCTRL_ON,
	HSCTRL_IDLE,
	MAX_HSCTRL_STATE,
};

enum HSCTRL_EVENT {
	HSCTRL_OFF,
	HSCTRL_ON,
	MAX_HSCTRL_MSG,
};

typedef struct GNU_PACKED _HSCTRL_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR EventTrigger;
	UCHAR EventType;
} HSCTRL_EVENT_DATA, *PHSCTRL_EVENT_DATA;

#define MAX_QOS_MAP_TABLE_SIZE				8

typedef struct _QOS_MAP_TABLE_T {
	UINT8			ucPoolValid;
	UINT8			ucDscpExceptionCount;
	UINT32          u4Ac;
	UINT16			au2DscpRange[8];
	UINT16			au2DscpException[21];
} QOS_MAP_TABLE_T, *P_QOS_MAP_TABLE_T;
#endif

typedef struct _HOTSPOT_CTRL {
#ifdef CONFIG_HOTSPOT
	UINT32 HSIndicationIELen;
	UINT32 P2PIELen;
	UINT32 QosMapSetIELen;
	UINT32 RoamingConsortiumIELen;
	PUCHAR QosMapSetIE;
	USHORT DscpRange[8];
	USHORT DscpException[21];
	PUCHAR RoamingConsortiumIE;
	PUCHAR HSIndicationIE;
	PUCHAR P2PIE;
	NDIS_SPIN_LOCK IeLock;
	BOOLEAN  HSDaemonReady;
#endif
	BOOLEAN HotSpotEnable;
#ifdef CONFIG_HOTSPOT
	enum HSCTRL_STATE HSCtrlState;
	BOOLEAN IsHessid;
	UCHAR Hessid[MAC_ADDR_LEN];
	UINT8 AccessNetWorkType;
	BOOLEAN DGAFDisable;
	UINT8 L2Filter;
	BOOLEAN ICMPv4Deny;
	BOOLEAN QosMapEnable;
	/* BOOLEAN bNonTX; */	/* 0:enable TX, 1:disable TX*/
	BOOLEAN bASANEnable;		/* 1:enable ASAN IE, 1:disable ASAN IE*/
	UCHAR	QLoadTestEnable;	/* for BSS Load IE Test */
	UCHAR	QLoadCU;			/* for BSS Load IE Test */
	USHORT	QLoadStaCnt;		/* for BSS Load IE Test */
#endif
	UINT8	HotspotBSSFlags;	/* for 7615 offload to CR4 */
#ifdef CONFIG_HOTSPOT
	BOOLEAN QosMapAddToPool;
	UINT8	QosMapPoolID;		/* per BSS default DSCP pool map ID */
	BOOLEAN bHSOnOff;  /* for recording wdev HS on/off status, to prevent HSCtrlOn or HSCtrlOff gets insanly called repeatedly */
#endif
} HOTSPOT_CTRL, *PHOTSPOT_CTRL;

/* for 7615 offload to CR4 */
enum HS_R2_CAPABILITY_FLAGS {
	fgHotspotEnable						= (1 << 0),
	fgProxyArpEnable					= (1 << 1),
	fgASANEnable						= (1 << 2),
	fgDGAFDisable						= (1 << 3),
	fgQosMapEnable						= (1 << 4),
};
enum HS_R2_UPDATE_TYPE {
	fgUpdateBssCapability				= (1 << 0),
	fgUpdateStaDSCP						= (1 << 1),
	fgUpdateDSCPPool					= (1 << 2),
};
#define IS_HOTSPOT_ENABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & fgHotspotEnable) != 0)
#define IS_PROXYARP_ENABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & fgProxyArpEnable) != 0)
#define IS_ASAN_ENABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & fgASANEnable) != 0)
#define IS_DGAF_DISABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & fgDGAFDisable) != 0)
#define IS_QOSMAP_ENABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & fgQosMapEnable) != 0)
#define IS_PROXYARP_ASAN_ENABLE(ucHotspotBssFlags) ((ucHotspotBssFlags & (fgHotspotEnable|fgProxyArpEnable|fgASANEnable|fgDGAFDisable)) != 0)

#ifdef CONFIG_HOTSPOT
enum {
	L2FilterDisable,
	L2FilterBuiltIn,
	L2FilterExternal,
};

BOOLEAN L2FilterInspection(
	IN PRTMP_ADAPTER pAd,
	IN PHOTSPOT_CTRL pHSCtrl,
	IN PUCHAR pData);

VOID HSCtrlStateMachineInit(
	IN	PRTMP_ADAPTER		pAd,
	IN	STATE_MACHINE * S,
	OUT	STATE_MACHINE_FUNC	Trans[]);

INT Set_STAHotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff);

enum HSCTRL_STATE HSCtrlCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

BOOLEAN HotSpotEnable(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN INT Type);
#endif

VOID HSCtrlExit(
	IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_HOTSPOT
VOID HSCtrlHalt(
	IN PRTMP_ADAPTER pAd);

INT Set_HotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff,
	IN UINT8 EventTrigger,
	IN UINT8 EventType);

struct _PEER_PROBE_REQ_PARAM;

BOOLEAN ProbeReqforHSAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR APIndex,
	IN struct _PEER_PROBE_REQ_PARAM *ProbeReqParam);

VOID Clear_Hotspot_All_IE(IN PRTMP_ADAPTER PAd);

#define isBcastEtherAddr(addr)  ((addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff)

#define GAS_STATE_MESSAGES    0
#define ACTION_STATE_MESSAGES 1

VOID HotspotOnOffEvent(
	IN PNET_DEV net_dev,
	IN int onoff);

VOID HotspotAPReload(
	IN PNET_DEV net_dev);

VOID hotspot_update_ap_qload_to_bcn(
	IN RTMP_ADAPTER * pAd);

INT Set_HotSpot_Param(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value);

VOID hotspot_bssflag_dump(
	UINT8 ucHotspotBSSFlags);

#endif
BOOLEAN hotspot_check_dhcp_arp(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket
);
VOID hotspot_update_bssflag(
	RTMP_ADAPTER * pAd,
	UINT8 flag,
	UINT8 value,
	PHOTSPOT_CTRL pHSCtrl);

VOID hotspot_update_bss_info_to_cr4(
	RTMP_ADAPTER * pAd,
	UCHAR APIndex,
	UINT8 BssIndex);

#ifdef CONFIG_HOTSPOT
VOID hotspot_add_qos_map_pool_to_cr4(
	RTMP_ADAPTER * pAd,
	UINT8 PoolID);

VOID hotspot_qosmap_update_sta_mapping_to_cr4(
	RTMP_ADAPTER * pAd,
	struct _MAC_TABLE_ENTRY *pEntry,
	UINT8 PoolID);

UINT8 hotspot_qosmap_add_pool(
	RTMP_ADAPTER * pAd,
	struct _MAC_TABLE_ENTRY *pEntry);




enum {
	HS_ON_OFF_BASE,
	HS_AP_RELOAD,
};

#ifdef CONFIG_AP_SUPPORT
BOOLEAN HSIPv4Check(
	IN PRTMP_ADAPTER pAd,
	PUSHORT pWcid,
	PNDIS_PACKET pPacket,
	PUCHAR pSrcBUf,
	UINT16 srcPort,
	UINT16 dscPort);
#endif

#ifdef CONFIG_HOTSPOT_R2
struct _sta_hs_info {
	/* UCHAR	addr[6]; */
	UCHAR	version;
	UCHAR	ppsmo_exist;
	USHORT	ppsmo_id;
};
#endif

#ifdef CONFIG_HOTSPOT_R3
typedef struct GNU_PACKED _sta_hs_consortium_oi {
	UINT32 sta_wcid;
	UCHAR oi_len;
	UCHAR selected_roaming_consortium_oi[5];
} STA_HS_CONSORTIUM_OI, *PSTA_HS_CONSORTIUM_OI;
#endif /* CONFIG_HOTSPOT_R3 */
#endif /*HOTSPOT*/
#endif /*HOTSPOT or PROXY ARP */
#endif /* __HOTSPOT_H__ */

