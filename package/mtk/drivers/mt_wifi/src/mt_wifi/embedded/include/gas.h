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
	gas.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __GAS_H__
#define __GAS_H__

#include "common/link_list.h"

#define GAS_MACHINE_BASE 0

#ifdef DPP_SUPPORT
#define GAS_OUI_Index 5
#define GAS_WFA_DPP_Subtype_Index 8
#define GAS_WFA_DPP_Length_Index 1
#define GAS_WFA_DPP_Min_Length 5
#endif /* DPP_SUPPORT */

/* gas states */
enum GAS_STATE {
	WAIT_GAS_REQ,
	WAIT_GAS_RSP,
	WAIT_PEER_GAS_REQ,
	WAIT_PEER_GAS_RSP,
	WAIT_GAS_CB_REQ,
	WAIT_GAS_CB_RSP,
	GAS_UNKNOWN,
	MAX_GAS_STATE,
};

/* gas events */
enum GAS_EVENT {
	GAS_REQ,
	GAS_RSP,
	GAS_RSP_MORE,
	PEER_GAS_REQ,
	PEER_GAS_RSP,
	PEER_GAS_RSP_MORE,
	GAS_CB_REQ,
	GAS_CB_REQ_MORE,
	GAS_CB_RSP,
	GAS_CB_RSP_MORE,
	DEL_PEER_ENTRY,
	MAX_GAS_MSG,
};

/* ANQP Info ID definitions */
enum {
	ANQP_QUERY_LIST = 256,
	ANQP_CAPABILITY,
	VENUE_NAME_INFO,
	EMERGENCY_CALL_NUMBER_INFO,
	NETWORK_AUTH_TYPE_INFO,
	ROAMING_CONSORTIUM_LIST,
	IP_ADDRESS_TYPE_AVAILABILITY_INFO,
	NAI_REALM_LIST,
	ThirdGPP_CELLULAR_NETWORK_INFO,
	AP_GEOSPATIAL_LOCATION,
	AP_CIVIC_LOCATION,
	AP_LOCATION_PUBLIC_IDENTIFIER_URI,
	DOMAIN_NAME_LIST,
	EMERGENCY_ALERT_IDENTIFIER_URI,
	EMERGENCY_NAI = 271,
	ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST = 56797,
};


#define GAS_FUNC_SIZE (MAX_GAS_STATE * MAX_GAS_MSG)

typedef struct _GAS_QUERY_RSP_FRAGMENT {
	DL_LIST List;
	UCHAR GASRspFragID;
	UINT16 FragQueryRspLen;
	UCHAR *FragQueryRsp;
} GAS_QUERY_RSP_FRAGMENT, *PGAS_QUERY_RSP_FRAGMENT;

typedef struct _GAS_PEER_ENTRY {
	DL_LIST List;
	enum GAS_STATE CurrentState;
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UCHAR DialogToken;
	UCHAR AdvertisementProID;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PostReplyTimer;
	BOOLEAN PostReplyTimerRunning;
	BOOLEAN InitPostReplyTimer;
	RALINK_TIMER_STRUCT GASRspBufferingTimer;
	BOOLEAN GASRspBufferingTimerRunning;
	BOOLEAN InitGASRspBufferingTimer;
	BOOLEAN peer_use_protected_dual;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	RALINK_TIMER_STRUCT GASResponseTimer;
	BOOLEAN GASResponseTimerRunning;
	RALINK_TIMER_STRUCT GASCBDelayTimer;
	BOOLEAN GASCBDelayTimerRunning;
#endif /* CONFIG_STA_SUPPORT */
	UCHAR GASRspFragNum;
	UCHAR CurrentGASFragNum;
	UCHAR QueryNum;
	DL_LIST GASQueryRspFragList;
} GAS_PEER_ENTRY, *PGAS_PEER_ENTRY;

typedef struct _GAS_CTRL {
	DL_LIST GASPeerList;
	NDIS_SPIN_LOCK GASPeerListLock;
	UINT8 ExternalANQPServerTest;
	UINT32 cb_delay; /* Come Back Delay */
	UINT32 MMPDUSize;
	BOOLEAN b11U_enable;
	UINT32 InterWorkingIELen;
	UINT32 AdvertisementProtoIELen;
	PUCHAR InterWorkingIE;
	PUCHAR AdvertisementProtoIE;
	NDIS_SPIN_LOCK IeLock;
} GAS_CTRL, *PGAS_CTRL;

/*
 * gas events data
 * GASComebackDelay : unit(TU)
 */
typedef struct GNU_PACKED _GAS_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {
#ifdef CONFIG_STA_SUPPORT
		struct {
			UCHAR DialogToken;
			UCHAR AdvertisementProID;
			UINT16 QueryReqLen;
			UCHAR QueryReq[0];
		} GNU_PACKED GAS_REQ_DATA;
		struct {
			UINT16 StatusCode;
			UCHAR AdvertisementProID;
			UINT16 QueryRspLen;
			UCHAR QueryRsp[0];
		} GNU_PACKED PEER_GAS_RSP_DATA;
		struct {
			UCHAR DialogToken;
		} GNU_PACKED PEER_GAS_RSP_MORE_DATA;
		struct {
			UINT16 StatusCode;
			UCHAR AdvertisementProID;
			UINT16 QueryRspLen;
			UCHAR QueryRsp[0];
		} GNU_PACKED GAS_CB_RSP_DATA;
		struct {
			UCHAR DialogToken;
		} GNU_PACKED GAS_CB_RSP_MORE_DATA;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 StatusCode;
			UINT16 GASComebackDelay;
			UCHAR AdvertisementProID;
			UINT16 QueryRspLen;
			UCHAR QueryRsp[0];
		} GNU_PACKED GAS_RSP_DATA;
		struct {
			UCHAR DialogToken;
			UINT16 StatusCode;
			UINT16 GASComebackDelay;
			UCHAR AdvertisementProID;
		} GNU_PACKED GAS_RSP_MORE_DATA;
		struct {
			UCHAR DialogToken;
			UCHAR AdvertisementProID;
			UINT16 QueryReqLen;
			UCHAR QueryReq[0];
		} GNU_PACKED PEER_GAS_REQ_DATA;
		struct {
			UCHAR DialogToken;
			UCHAR AdvertisementProID;
			UINT16 StatusCode;
		} GNU_PACKED GAS_CB_REQ_DATA;
		struct {
			UCHAR DialogToken;
			UCHAR AdvertisementProID;
			UINT16 StatusCode;
		} GNU_PACKED GAS_CB_REQ_MORE_DATA;
#endif /* CONFIG_AP_SUPPORT */
	} u;
} GAS_EVENT_DATA, *PGAS_EVENT_DATA;

VOID GASStateMachineInit(
	IN	PRTMP_ADAPTER		pAd,
	IN	STATE_MACHINE * S,
	OUT	STATE_MACHINE_FUNC	Trans[]);

enum GAS_STATE GASPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID GASSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	/* IN MLME_QUEUE_ELEM *Elem, */
	PGAS_EVENT_DATA Event,
	IN enum GAS_STATE State);

VOID SendGASRsp(
	IN PRTMP_ADAPTER    pAd,
	GAS_EVENT_DATA *Event);

VOID GASCtrlRemoveAllIE(PGAS_CTRL pGasCtrl);

VOID GASCtrlExit(IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_STA_SUPPORT
VOID ReceiveGASInitRsp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID ReceiveGASCBRsp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

void SendAnqpRspEvent(void *net_dev, const char *peer_mac_addr,
					  UINT16 status, const char *anqp_rsp, UINT16 anqp_rsp_len);

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(PostReplyTimeout);
DECLARE_TIMER_FUNCTION(GASRspBufferingTimeout);

void SendAnqpReqEvent(PNET_DEV net_dev, const char *peer_mac_addr,
					  const char *anqp_req, UINT16 anqp_req_len);

void SendLocationElementEvent(PNET_DEV net_dev, const char *location_buf,
							  UINT16 location_buf_len, UINT16 info_id);

VOID ReceiveGASInitReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID FreeGasPeerEntryTimers(
	IN GAS_PEER_ENTRY *GASPeerEntry);

VOID FreeGasPeerEntry(
	IN GAS_PEER_ENTRY *GASPeerEntry);

#ifdef DPP_SUPPORT
VOID DPP_ReceiveGASInitRsp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID DPP_ReceiveGASCBReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID DPP_ReceiveGASCBResp(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);
#endif /* DPP_SUPPORT */

VOID ReceiveGASCBReq(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);
#endif /* CONFIG_AP_SUPPORT */

INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * ANQPRsp,
	IN UINT32 ANQPRspLen);

BOOLEAN GasEnable(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

#endif /* __GAS_H__ */
