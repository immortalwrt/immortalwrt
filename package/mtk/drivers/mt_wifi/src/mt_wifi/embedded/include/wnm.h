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
	wnm.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __WNM_H__
#define __WNM_H__

#include "ipv6.h"
#include "mat.h"

#ifdef CONFIG_DOT11V_WNM
/********** protocol definition **********/

#define WNM_BSS_PERFERENCE_SUBIE 3
#define WNM_BSS_TERMINATION_SUBIE 4

enum WNM_ACTION_FIELD {
	EVENT_REQUEST,
	EVENT_REPORT,
	DIAGNOSTIC_REQ,
	DIAGNOSTIC_REPORT,
	LOCATION_CONF_REQ,
	LOCATION_CONF_RSP,
	BSS_TRANSITION_QUERY,
	BSS_TRANSITION_REQ,
	BSS_TRANSITION_RSP,
	FMS_REQ,
	FMS_RSP,
	COLLOCATED_INTERFERENCE_REQ,
	COLLOCATED_INTERFERENCE_REPORT,
	TFS_REQ,
	TFS_RSP,
	TFS_NOTIFY,
	WNM_SLEEP_MODE_REQ,
	WNM_SLEEP_MODE_RSP,
	TIM_BROADCAST_REQ,
	TIM_BROADCAST_RSP,
	QOS_TRAFFIC_CAPABILITY_UPDATE,
	CHANNEL_USAGE_REQ,
	CHANNEL_USAGE_RSP,
	DMS_REQ,
	DMS_RSP,
	TIMING_MEASUREMENT_REQ,
	WNM_NOTIFICATION_REQ,
	WNM_NOTIFICATION_RSP,
};

typedef struct GNU_PACKED _WNM_FRAME {
	HEADER_802_11 Hdr;
	UCHAR Category;
	union {
		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR Variable[0];
		} GNU_PACKED BTM_QUERY;

		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR Variable[0];
		} GNU_PACKED BTM_REQ;

		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR Variable[0];
		} GNU_PACKED BTM_RSP;

		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR Type;
			UCHAR Variable[0];
		} GNU_PACKED WNM_NOTIFY_REQ;

		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR RespStatus;
			UCHAR Variable[0];
		} GNU_PACKED WNM_NOTIFY_RSP;
	} u;
} WNM_FRAME, *PWNM_FRAME;


#define BTM_MACHINE_BASE 0
#define WaitPeerBTMRspTimeoutVale (15*1000)
#define WaitPeerBTMReqTimeoutVale (30*1000)


#define BTM_ENABLE_OFFSET   (1<<0)

#define	BTM_CANDIDATE_OFFSET (1<<0)
#define	BTM_ABRIDGED_OFFSET (1<<1)
#define	BTM_DISASSOC_OFFSET (1<<2)
#define	BTM_TERMINATION_OFFSET (1<<3)
#define	BTM_URL_OFFSET (1<<4)

/* BTM states */
enum BTM_STATE {
	WAIT_BTM_QUERY,
	WAIT_PEER_BTM_QUERY,
	WAIT_BTM_REQ,
	WAIT_BTM_RSP,
	WAIT_PEER_BTM_REQ,
	WAIT_PEER_BTM_RSP,
	BTM_UNKNOWN,
	MAX_BTM_STATE,
};


/* BTM events */
enum BTM_EVENT {
	BTM_QUERY,
	PEER_BTM_QUERY,
	BTM_REQ,
	BTM_REQ_IE,
	BTM_REQ_PARAM,
	BTM_RSP,
	PEER_BTM_REQ,
	PEER_BTM_RSP,
	BTM_REQ_TIMEOUT,
	PEER_BTM_RSP_TIMEOUT,
	MAX_BTM_MSG,
};

#define BTM_FUNC_SIZE (MAX_BTM_STATE * MAX_BTM_MSG)

#endif
enum IPV6_TYPE {
	IPV6_LINK_LOCAL,
	IPV6_GLOBAL,
};
#ifdef CONFIG_DOT11V_WNM

typedef struct GNU_PACKED _BTM_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {
#ifdef CONFIG_STA_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 BTMQueryLen;
			UCHAR BTMQuery[0];
		} GNU_PACKED BTM_QUERY_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMRspLen;
			UCHAR BTMRsp[0];
		} GNU_PACKED BTM_RSP_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMReqLen;
			UCHAR BTMReq[0];
		} GNU_PACKED PEER_BTM_REQ_DATA;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 BTMReqLen;
			UCHAR BTMReq[0];
		} GNU_PACKED BTM_REQ_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMQueryLen;
			UCHAR BTMQuery[0];
		} GNU_PACKED PEER_BTM_QUERY_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMRspLen;
			UCHAR BTMRsp[0];
		} GNU_PACKED PEER_BTM_RSP_DATA;
#endif /* CONFIG_AP_SUPPORT */
	} u;
} BTM_EVENT_DATA, *PBTM_EVENT_DATA;

typedef struct _BTM_PEER_ENTRY {
	DL_LIST List;
	enum BTM_STATE CurrentState;
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UCHAR DialogToken;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT WaitPeerBTMRspTimer;
	RALINK_TIMER_STRUCT WaitPeerBTMReqTimer;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	RALINK_TIMER_STRUCT WaitAPBTMRspTimer;
#endif /* CONFIG_AP_SUPPORT */
	UINT32 WaitPeerBTMRspTime;
} BTM_PEER_ENTRY, *PBTM_PEER_ENTRY;
#endif

typedef struct _PROXY_ARP_IPV4_ENTRY {
	DL_LIST List;
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPAddr[4];
} PROXY_ARP_IPV4_ENTRY, *PPROXY_ARP_IPV4_ENTRY;

typedef struct _PROXY_ARP_IPV4_UNIT {
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPAddr[4];
} PROXY_ARP_IPV4_UNIT, *PPROXY_ARP_IPV4_UNIT;

typedef struct _PROXY_ARP_IPV6_ENTRY {
	DL_LIST List;
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPType;
	UCHAR TargetIPAddr[16];
} PROXY_ARP_IPV6_ENTRY, *PPROXY_ARP_IPV6_ENTRY;

typedef struct _PROXY_ARP_IPV6_UNIT {
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPType;
	UCHAR TargetIPAddr[16];
} PROXY_ARP_IPV6_UNIT, *PPROXY_ARP_IPV6_UNIT;

typedef struct _WNM_CTRL {
	UINT32 TimeadvertisementIELen;
	UINT32 TimezoneIELen;
	PUCHAR TimeadvertisementIE;
	PUCHAR TimezoneIE;
	NDIS_SPIN_LOCK IeLock;
	RTMP_OS_SEM BTMPeerListLock;
	RTMP_OS_SEM WNMNotifyPeerListLock;
	BOOLEAN ProxyARPEnable;
	BOOLEAN WNMNotifyEnable;
	BOOLEAN WNMBTMEnable;
	NDIS_SPIN_LOCK ProxyARPListLock;
	NDIS_SPIN_LOCK ProxyARPIPv6ListLock;
	DL_LIST IPv4ProxyARPList;
	DL_LIST IPv6ProxyARPList;
	DL_LIST BTMPeerList;
	DL_LIST WNMNotifyPeerList;
} WNM_CTRL, *PWNM_CTRL;

#ifdef BB_SOC
#ifdef IPV6
#undef IPV6
enum IPTYPE {
	IPV4,
	IPV6
};
#endif
#else
enum IPTYPE {
	IPV4,
	IPV6
};
#endif /* BB_SOC */

struct _BSS_STRUCT;

#ifdef CONFIG_DOT11V_WNM
typedef struct GNU_PACKED _BTM_REQ_FRAME {
	UINT8 request_mode;
	UINT16 disassociation_timer;
	UINT8 validity_interval;
} BTM_REQ_FRAME, *PBTM_REQ_FRAME;

INT Set_SendBTMReq_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif

BOOLEAN IsGratuitousARP(IN PRTMP_ADAPTER pAd,
						IN PUCHAR pData,
						IN UCHAR *DAMacAddr,
						IN struct _BSS_STRUCT *pMbss,
						IN INT is_atomic);

BOOLEAN IsUnsolicitedNeighborAdver(PRTMP_ADAPTER pAd,
								   PUCHAR pData);

BOOLEAN IsIPv4ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData);

BOOLEAN IsIPv6ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData);

BOOLEAN IsIPv6DHCPv6Solicitation(IN PRTMP_ADAPTER pAd,
								 IN PUCHAR pData);

BOOLEAN IsIPv6RouterSolicitation(IN PRTMP_ADAPTER pAd,
								 IN PUCHAR pData);

BOOLEAN IsIPv6RouterAdvertisement(IN PRTMP_ADAPTER pAd,
								  IN PUCHAR pData,
								  IN PUCHAR pOffset);

#ifdef CONFIG_DOT11V_WNM
BOOLEAN IsTDLSPacket(IN PRTMP_ADAPTER pAd,
					 IN PUCHAR pData);
#endif
BOOLEAN IPv4ProxyARP(IN PRTMP_ADAPTER pAd,
					 IN struct _BSS_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS,
					 IN INT is_atomic);

BOOLEAN IsIpv6DuplicateAddrDetect(PRTMP_ADAPTER pAd,
								  PUCHAR pData,
								  PUCHAR pOffset);

BOOLEAN IPv6ProxyARP(IN PRTMP_ADAPTER pAd,
					 IN struct _BSS_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS,
					 IN INT is_atomic);

UINT32 AddIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr,
							IN INT is_atomic);

UINT32 AddIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr,
							IN INT is_atomic);

UINT32 IPv4ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss);

UINT32 IPv6ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss);

BOOLEAN GetIPv4ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

BOOLEAN GetIPv6ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

VOID RemoveIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 PUCHAR pTargetMACAddr,
							 IN INT is_atomic);

VOID RemoveIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 PUCHAR pTargetMACAddr);

VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd);
VOID WNMCtrlExit(IN PRTMP_ADAPTER pAd);
#ifdef CONFIG_DOT11V_WNM
void PeerWNMAction(IN struct _RTMP_ADAPTER *pAd, IN MLME_QUEUE_ELEM * Elem);
VOID Clear_All_PROXY_TABLE(IN PRTMP_ADAPTER pAd);
/* WNM Notification states */
enum WNM_NOTIFY_STATE {
	WAIT_WNM_NOTIFY_REQ,
	WAIT_WNM_NOTIFY_RSP,
	WNM_NOTIFY_UNKNOWN,
	MAX_WNM_NOTIFY_STATE,
};

/* WNM Notification events */
enum WNM_NOTIFY_EVENT {
	WNM_NOTIFY_REQ,
	WNM_NOTIFY_RSP,
	MAX_WNM_NOTIFY_MSG,
};

#define WNM_NOTIFY_FUNC_SIZE (MAX_WNM_NOTIFY_STATE * MAX_WNM_NOTIFY_MSG)

#define WNM_NOTIFY_MACHINE_BASE 0
#define WaitPeerWNMNotifyRspTimeoutVale 1024


VOID BTMStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);

enum BTM_STATE BTMPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

INT Send_BTM_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *BTMReq,
	IN UINT32 BTMReqLen);

INT Send_BTM_Rsp(
		IN PRTMP_ADAPTER pAd,
		IN RTMP_STRING *PeerMACAddr,
		IN RTMP_STRING *BTMRsp,
		IN UINT32 BTMRspLen);

#endif
#ifdef CONFIG_AP_SUPPORT
VOID WNMIPv4ProxyARPCheck(
	IN PRTMP_ADAPTER pAd,
	PNDIS_PACKET pPacket,
	USHORT srcPort,
	USHORT dstPort,
	PUCHAR pSrcBuf,
	IN INT is_atomc);

VOID WNMIPv6ProxyARPCheck(
	IN PRTMP_ADAPTER pAd,
	PNDIS_PACKET pPacket,
	PUCHAR pSrcBuf,
	IN INT is_atomic);
#ifdef CONFIG_DOT11V_WNM

DECLARE_TIMER_FUNCTION(WaitPeerBTMRspTimeout);
DECLARE_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout);
DECLARE_TIMER_FUNCTION(WaitPeerBTMReqTimeout);

VOID ReceiveWNMNotifyReq(IN PRTMP_ADAPTER pAd,
			  IN MLME_QUEUE_ELEM *Elem);


NDIS_STATUS wnm_handle_command(IN PRTMP_ADAPTER pAd, 
										IN struct wnm_command *pCmd_data);

void WNM_ReadParametersFromFile(
        IN PRTMP_ADAPTER pAd,
        RTMP_STRING *tmpbuf,
        RTMP_STRING *buffer);

VOID ReceiveWNMNotifyRsp(IN PRTMP_ADAPTER pAd,
						 IN MLME_QUEUE_ELEM * Elem);

VOID SendWNMNotifyConfirm(IN PRTMP_ADAPTER    pAd,
						  IN MLME_QUEUE_ELEM * Elem);

VOID WNMNotifyStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);


typedef struct GNU_PACKED _WNM_NOTIFY_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {

#ifdef CONFIG_AP_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 WNMNotifyReqLen;
			UCHAR WNMNotifyReq[0];
		} GNU_PACKED WNM_NOTIFY_REQ_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 WNMNotifyRspLen;
			UCHAR WNMNotifyRsp[0];
		} GNU_PACKED WNM_NOTIFY_RSP_DATA;
#endif /* CONFIG_AP_SUPPORT */
	} u;
} WNM_NOTIFY_EVENT_DATA, *PWNM_NOTIFY_EVENT_DATA;

typedef struct _WNM_NOTIFY_PEER_ENTRY {
	DL_LIST List;
	enum WNM_NOTIFY_STATE CurrentState;
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UCHAR DialogToken;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT WaitPeerWNMNotifyRspTimer;
#endif /* CONFIG_AP_SUPPORT */
} WNM_NOTIFY_PEER_ENTRY, *PWNM_NOTIFY_PEER_ENTRY;

INT Send_WNM_Notify_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * WNMNotifyReq,
	IN UINT32 WNMNotifyReqLen,
	IN UINT32 type);

INT Send_QOSMAP_Configure(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * QosMapBuf,
	IN UINT32	QosMapLen,
	IN UINT8	Apidx);

#ifdef CONFIG_HOTSPOT_R2
VOID WNMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN enum WNM_NOTIFY_STATE State);

enum WNM_NOTIFY_STATE WNMNotifyPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID WNMNotifyStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);
#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_AP_SUPPORT */

int send_btm_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

int send_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_req_ie_data_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

int check_btm_custom_params(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

int compose_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	OUT PUCHAR p_btm_req_ie,
	OUT PUINT32 p_btm_req_ie_len,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

VOID WNM_InsertBSSTerminationSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT64 TSF,
	IN UINT16 Duration);

VOID RRM_InsertPreferenceSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT8 preference);

#ifdef MAP_R2
void SendBTMQueryEvent(PNET_DEV net_dev,
			const char *peer_mac_addr,
			const char *btm_query,
			UINT16 btm_query_len,
			UINT8 ipc_type);

void SendWNMNotifyEvent(PNET_DEV net_dev,
			const char *peer_mac_addr,
			const char *wnm_req,
			UINT16 wnm_req_len);
#endif
#endif
#endif /* __WNM_H__ */

