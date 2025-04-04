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
    cmm_mat.c

    Abstract:
	    Support Mac Address Translation function.

    Note:
		MAC Address Translation(MAT) engine subroutines, we should just take care
	 packet to bridge.

    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang		02-26-2007      Init version
*/

#ifdef MAT_SUPPORT

#include "rt_config.h"


extern MATProtoOps MATProtoIPHandle;
extern MATProtoOps MATProtoARPHandle;
extern MATProtoOps MATProtoPPPoEDisHandle;
extern MATProtoOps MATProtoPPPoESesHandle;
extern MATProtoOps MATProtoIPv6Handle;

extern UCHAR SNAP_802_1H[];
extern UCHAR SNAP_BRIDGE_TUNNEL[];

#define MAX_MAT_NODE_ENTRY_NUM	128	/* We support maximum 128 node entry for our system */
#define MAT_NODE_ENTRY_SIZE	40 /*28	// bytes   //change to 40 for IPv6Mac Table */

typedef struct _MATNodeEntry {
	UCHAR data[MAT_NODE_ENTRY_SIZE];
	struct _MATNodeEntry *next;
} MATNodeEntry, *PMATNodeEntry;


#ifdef KMALLOC_BATCH
/*static MATNodeEntry *MATNodeEntryPoll = NULL; */
#endif

static MATProtoTable MATProtoTb[] = {
	{ETH_P_IP,			&MATProtoIPHandle},			/* IP handler */
	{ETH_P_ARP,		&MATProtoARPHandle},		/* ARP handler */
	{ETH_P_PPP_DISC,	&MATProtoPPPoEDisHandle},	/* PPPoE discovery stage handler */
	{ETH_P_PPP_SES,		&MATProtoPPPoESesHandle},	/* PPPoE session stage handler */
	{ETH_P_IPV6,		&MATProtoIPv6Handle},		/* IPv6 handler */
};

#define MAX_MAT_SUPPORT_PROTO_NUM (sizeof(MATProtoTb)/sizeof(MATProtoTable))


/* --------------------------------- Public Function-------------------------------- */
NDIS_STATUS MATDBEntryFree(
	IN MAT_STRUCT * pMatStruct,
	IN PUCHAR		NodeEntry)
{
#ifdef KMALLOC_BATCH
	MATNodeEntry *pPtr, *pMATNodeEntryPoll;

	pMATNodeEntryPoll = (MATNodeEntry *)pAd->MatCfg.MATNodeEntryPoll;
	pPtr = (MATNodeEntry *)NodeEntry;
	NdisZeroMemory(pPtr, sizeof(MATNodeEntry));

	if (pMATNodeEntryPoll->next) {
		pPtr->next = pMATNodeEntryPoll->next;
		pMATNodeEntryPoll->next = pPtr;
	} else
		pMATNodeEntryPoll->next = pPtr;

#else
	os_free_mem(NodeEntry);
#endif
	return TRUE;
}

PUCHAR MATDBEntryAlloc(IN MAT_STRUCT * pMatStruct, IN UINT32 size)
{
#ifdef KMALLOC_BATCH
	MATNodeEntry *pPtr = NULL, *pMATNodeEntryPoll;

	pMATNodeEntryPoll = (MATNodeEntry *)pMatStruct->pMATNodeEntryPoll;

	if (pMATNodeEntryPoll->next) {
		pPtr = pMATNodeEntryPoll->next;
		pMATNodeEntryPoll->next = pPtr->next;
	}

#else
	UCHAR *pPtr = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&pPtr, size);
#endif
	return (PUCHAR)pPtr;
}


VOID dumpPkt(PUCHAR pHeader, int len)
{
	int i;
	RTMP_STRING *tmp;

	tmp = (RTMP_STRING *)pHeader;
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "--StartDump\n");

	for (i = 0; i < len; i++) {
		if ((i % 16 == 0) && (i != 0))
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "\n");

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "%02x ", tmp[i] & 0xff);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "\n--EndDump\n");
	return;
}


/*
	========================================================================
	Routine	Description:
		For each out-going packet, check the upper layer protocol type if need
		to handled by our APCLI convert engine. If yes, call corresponding handler
		to handle it.

	Arguments:
		pAd		=>Pointer to our adapter
		pPkt	=>pointer to the 802.11 header of outgoing packet
		ifIdx   =>Interface Index want to dispatch to.

	Return Value:
		Success	=>
			TRUE
			Mapped mac address if found, else return specific default mac address
			depends on the upper layer protocol type.
		Error	=>
			FALSE.

	Note:
		1.the pPktHdr must be a 802.3 packet.
		2.Maybe we need a TxD arguments?
		3.We check every packet here including group mac address becasue we need to
		  handle DHCP packet.
	========================================================================
 */
PUCHAR MATEngineTxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET	    pPkt,
	IN UINT				ifIdx,
	IN UINT32	entry_type)
{
	PUCHAR		pLayerHdr = NULL, pPktHdr = NULL,  pMacAddr = NULL;
	UINT16		protoType, protoType_ori;
	INT			i;
	struct _MATProtoOps	*pHandle = NULL;
	PUCHAR  retSkb = NULL;
	BOOLEAN bVLANPkt = FALSE;
	UCHAR ZeroMac[MAC_ADDR_LEN] = {0};

	if (pAd->MatCfg.status != MAT_ENGINE_STAT_INITED)
		return NULL;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);

	if (!pPktHdr)
		return NULL;

	protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12));
	/* Get the upper layer protocol type of this 802.3 pkt. */
	protoType = OS_NTOHS(protoType_ori);

	/* handle 802.1q enabled packet. Skip the VLAN tag field to get the protocol type. */
	if (protoType == 0x8100) {
		protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12 + 4));
		protoType = OS_NTOHS(protoType_ori);
		bVLANPkt = TRUE;
	}


	/* For differnet protocol, dispatch to specific handler */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++) {
		if (protoType == MATProtoTb[i].protocol) {
			pHandle = MATProtoTb[i].pHandle;	/* the pHandle must not be null! */
			pLayerHdr = bVLANPkt ? (pPktHdr + MAT_VLAN_ETH_HDR_LEN) : (pPktHdr + MAT_ETHER_HDR_LEN);

			switch (entry_type) {
#ifdef CONFIG_STA_SUPPORT
			case ENTRY_AP: /*Device is client, connecting to peer AP*/
				pMacAddr = &pAd->StaCfg[ifIdx].wdev.if_addr[0];
				break;
#endif /* CONFIG_STA_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
			case ENTRY_REPEATER: /*Device is repeater, connecting to RootAP*/
				pMacAddr = &pAd->ApCfg.pRepeaterCliPool[ifIdx].CurrentAddress[0];
				break;
#endif /* MAC_REPEATER_SUPPORT */
			default:
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\033[1;31m Unexpexted Entry Type \033[0m\n");
				/* assign ZERO MAC ADDR*/
				pMacAddr = ZeroMac;
				break;
			}

			if (pHandle->tx != NULL)
				retSkb = pHandle->tx((PVOID)&pAd->MatCfg, RTPKT_TO_OSPKT(pPkt), pLayerHdr, pMacAddr);

			if (retSkb && ((PNDIS_PACKET)retSkb != pPkt))
				RELEASE_NDIS_PACKET(pAd, pPkt, NDIS_STATUS_SUCCESS);
			return retSkb;
		}
	}

	return retSkb;
}


/*
	========================================================================
	Routine	Description:
		Depends on the Received packet, check the upper layer protocol type
		and search for specific mapping table to find out the real destination
		MAC address.

	Arguments:
		pAd		=>Pointer to our adapter
		pPkt	=>pointer to the 802.11 header of receviced packet
		infIdx	=>Interface Index want to dispatch to.

	Return Value:
		Success	=>
			Mapped mac address if found, else return specific default mac address
			depends on the upper layer protocol type.
		Error	=>
			NULL

	Note:
	========================================================================
 */
PUCHAR MATEngineRxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPkt,
	IN UINT				infIdx)
{
	PUCHAR				pMacAddr = NULL;
	PUCHAR		pLayerHdr = NULL, pPktHdr = NULL;
	UINT16		protoType;
	INT			i = 0;
	struct _MATProtoOps	*pHandle = NULL;

	if (pAd->MatCfg.status != MAT_ENGINE_STAT_INITED)
		return NULL;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);

	if (!pPktHdr)
		return NULL;

	/* If it's a multicast/broadcast packet, we do nothing. */
	if (IS_GROUP_MAC(pPktHdr))
		return NULL;

	/* Get the upper layer protocol type of this 802.3 pkt and dispatch to specific handler */
	protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));
	pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

	if (protoType == ETH_P_VLAN) {
		protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12 + LENGTH_802_1Q))); /* Shift VLAN Tag Length (4 byte) */
		pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
	}


	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++) {
		if (protoType == MATProtoTb[i].protocol) {
			pHandle = MATProtoTb[i].pHandle;	/* the pHandle must not be null! */

			/*			RTMP_SEM_LOCK(&MATDBLock); */
			if (pHandle->rx != NULL)
				pMacAddr = pHandle->rx((PVOID)&pAd->MatCfg, RTPKT_TO_OSPKT(pPkt), pLayerHdr, NULL);

			/*			RTMP_SEM_UNLOCK(&MATDBLock); */
			break;
		}
	}

	if (pMacAddr)
		NdisMoveMemory(pPktHdr, pMacAddr, MAC_ADDR_LEN);

	return NULL;
}


BOOLEAN MATPktRxNeedConvert(
	IN PRTMP_ADAPTER	pAd,
	IN PNET_DEV			net_dev)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT
		int i = 0;

		/* Check if the packet will be send to apcli interface. */
		while (i < MAX_APCLI_NUM) {
			/*BSSID match the ApCliBssid ?(from a valid AP) */
			if ((pAd->StaCfg[i].ApcliInfStat.Valid == TRUE)
				&& (net_dev == pAd->StaCfg[i].wdev.if_dev)
#ifdef A4_CONN
				&& (IS_APCLI_A4(&pAd->StaCfg[i]) == FALSE)
#endif /* A4_CONN */
			){
				/* MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MATPktRxNeedConvert TRUE for ApCliTab[%d]\n",i); */
				return TRUE;
			}

			i++;
		}

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef P2P_SUPPORT

		if (P2P_INF_ON(pAd) && (RtmpOsNetPrivGet(net_dev) == INT_P2P))
			return FALSE;

#endif /* P2P_SUPPORT */

		if (pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE)
			return TRUE;
	}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	return FALSE;
}


NDIS_STATUS MATEngineExit(
	IN RTMP_ADAPTER * pAd)
{
	struct _MATProtoOps *pHandle = NULL;
	int i;

	if (pAd->MatCfg.status == MAT_ENGINE_STAT_EXITED)
		return TRUE;

	/* For each registered protocol, we call it's exit handler. */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++) {
		pHandle = MATProtoTb[i].pHandle;

		if (pHandle->exit != NULL)
			pHandle->exit(&pAd->MatCfg);
	}

#ifdef KMALLOC_BATCH

	/* Free the memory used to store node entries. */
	if (pAd->MatCfg.pMATNodeEntryPoll) {
		os_free_mem(pAd->MatCfg.pMATNodeEntryPoll);
		pAd->MatCfg.pMATNodeEntryPoll = NULL;
	}

#endif
	pAd->MatCfg.status = MAT_ENGINE_STAT_EXITED;
	pAd->MatCfg.nodeCount = 0;
	return TRUE;
}


NDIS_STATUS MATEngineInit(
	IN RTMP_ADAPTER * pAd)
{
	MATProtoOps	*pHandle = NULL;
	int i, status;

	if (pAd->MatCfg.status == MAT_ENGINE_STAT_INITED)
		return TRUE;

#ifdef KMALLOC_BATCH
	/* Allocate memory for node entry, we totally allocate 128 entries and link them together. */
	os_alloc_mem_suspend(NULL, (UCHAR **)&(pAd->MatCfg.pMATNodeEntryPoll), sizeof(MATNodeEntry) * MAX_MAT_NODE_ENTRY_NUM);

	if (pAd->MatCfg.pMATNodeEntryPoll != NULL) {
		MATNodeEntry *pPtr = NULL;

		NdisZeroMemory(pAd->MatCfg.pMATNodeEntryPoll, sizeof(MATNodeEntry) * MAX_MAT_NODE_ENTRY_NUM);
		pPtr = pAd->MatCfg.pMATNodeEntryPoll;

		for (i = 0; i < (MAX_MAT_NODE_ENTRY_NUM - 1); i++) {
			pPtr->next = (MATNodeEntry *)(pPtr + 1);
			pPtr = pPtr->next;
		}

		pPtr->next = NULL;
	} else
		return FALSE;

#endif

	/* For each specific protocol, call it's init function. */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++) {
		pHandle = MATProtoTb[i].pHandle;
		ASSERT(pHandle);

		if ((pHandle != NULL) && (pHandle->init != NULL)) {
			status = pHandle->init(&pAd->MatCfg);

			if (status == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_ERROR, "MATEngine Init Protocol (0x%x) failed, Stop the MAT Funciton initialization failed!\n", MATProtoTb[i].protocol);
				goto init_failed;
			}

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "MATEngine Init Protocol (0x%04x) success!\n", MATProtoTb[i].protocol);
		}
	}

	NdisAllocateSpinLock(pAd, &pAd->MatCfg.MATDBLock);
#ifdef MAC_REPEATER_SUPPORT
	pAd->MatCfg.bMACRepeaterEn = FALSE;
#endif /* MAC_REPEATER_SUPPORT */
	pAd->MatCfg.pPriv = (VOID *)pAd;
	pAd->MatCfg.status = MAT_ENGINE_STAT_INITED;
	return TRUE;
init_failed:

	/* For each specific protocol, call it's exit function. */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++) {
		pHandle = MATProtoTb[i].pHandle;
		if (pHandle != NULL) {
			if (pHandle->exit != NULL) {
				status = pHandle->exit(&pAd->MatCfg);

				if (status == FALSE)
					goto init_failed;
			}
		}
	}

#ifdef KMALLOC_BATCH

	if (pAd->MatCfg.pMATNodeEntryPoll)
		os_free_mem(pAd->MatCfg.pMATNodeEntryPoll);

	pAd->MatCfg.status = MAT_ENGINE_STAT_EXITED;
#endif /* KMALLOC_BATCH */
	return FALSE;
}

#endif /* MAT_SUPPORT */

