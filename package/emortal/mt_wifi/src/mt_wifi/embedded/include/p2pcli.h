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
    p2pcli.h

    Abstract:
    Support AP-Client function.

    Revision History:
    Who               When            What
    --------------    ----------      ----------------------------------------------
    Shiang, Fonchi    02-13-2007      created
*/

#ifndef _P2P_CLI_H_
#define _P2P_CLI_H_

#ifdef P2P_SUPPORT

#include "rtmp.h"

#define AUTH_TIMEOUT	300         /* unit: msec */
#define ASSOC_TIMEOUT	300         /* unit: msec */
/* #define JOIN_TIMEOUT	2000 */     /* unit: msec, not used in Ap-client mode, remove it */
#define PROBE_TIMEOUT	1000        /* unit: msec */

#define P2P_CLI_ROOT_BSSID_GET(pAd, wcid) ((pAd)->MacTab.Content[(wcid)].Addr)
#define P2P_CLI_IF_UP_CHECK(pAd, ifidx) ((pAd)->StaCfg[0].P2PCliTab[(ifidx)].dev->flags & IFF_UP)

/* sanity check for apidx */
#define P2P_CLI_MR_APIDX_SANITY_CHECK(idx) \
	{ \
		if ((idx) >= MAX_APCLI_NUM) { \
			(idx) = 0; \
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s> Error! apcli-idx > MAX_APCLI_NUM!\n", __func__)); \
		} \
	}

typedef struct _P2PCLI_MLME_JOIN_REQ_STRUCT {
	UCHAR	Bssid[MAC_ADDR_LEN];
	UCHAR	SsidLen;
	UCHAR	Ssid[MAX_LEN_OF_SSID];
} P2PCLI_MLME_JOIN_REQ_STRUCT;

typedef struct _P2PCLI_CTRL_MSG_STRUCT {
	USHORT	Status;
} P2PCLI_CTRL_MSG_STRUCT, *PP2PCLI_CTRL_MSG_STRUCT;

BOOLEAN isValidP2pCliIf(
	SHORT ifIndex);

/*
 * Private routines in apcli_ctrl.c
*/

VOID P2P_CliCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

/*
 * Private routines in apcli_sync.c
*/

VOID P2P_CliSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

/*
 * Private routines in apcli_auth.c
*/

VOID P2P_CliAuthStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

/*
* Private routines in apcli_assoc.c
*/

VOID P2P_CliAssocStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

MAC_TABLE_ENTRY *P2P_CliTableLookUpByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid,
	IN PUCHAR pAddrs);

BOOLEAN P2P_CliAllowToSendPacket(
	IN RTMP_ADAPTER * pAd,
	IN PNDIS_PACKET pPacket,
	OUT UINT16		*pWcid);

BOOLEAN	P2P_CliValidateRSNIE(
	IN	PRTMP_ADAPTER	pAd,
	IN	PEID_STRUCT		pEid_ptr,
	IN	USHORT			eid_len,
	IN	USHORT			idx);

INT P2P_CliIfLookUp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

VOID P2P_CliMgtMacHeaderInit(
	IN	PRTMP_ADAPTER	pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pBssid,
	IN USHORT ifIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN P2P_CliCheckHt(
	IN	PRTMP_ADAPTER pAd,
	IN	USHORT IfIndex,
	IN OUT	HT_CAPABILITY_IE * pHtCapability,
	IN OUT	ADD_HT_INFO_IE * pAddHtInfo);
#endif /* DOT11_N_SUPPORT */

BOOLEAN P2P_CliLinkUp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID P2P_CliLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID P2P_CliIfUp(
	IN PRTMP_ADAPTER pAd);

VOID P2P_CliIfDown(
	IN PRTMP_ADAPTER pAd);

VOID P2P_CliIfMonitor(
	IN PRTMP_ADAPTER pAd);

BOOLEAN P2P_CliMsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN P2P_PreCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN P2P_CliPeerAssocRspSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT HT_CAPABILITY_IE * pHtCapability,
	OUT ADD_HT_INFO_IE * pAddHtInfo,	/* AP might use this additional ht info IE */
	OUT UCHAR *pHtCapabilityLen,
	OUT UCHAR *pAddHtInfoLen,
	OUT UCHAR *pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT UCHAR *pCkipFlag);

BOOLEAN  P2P_CliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK * pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			wdev_idx);

VOID P2P_CliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

BOOLEAN P2P_CliInstallSharedKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry);

#endif /* P2P_SUPPORT */
#endif /* _P2P_CLI_H_ */

