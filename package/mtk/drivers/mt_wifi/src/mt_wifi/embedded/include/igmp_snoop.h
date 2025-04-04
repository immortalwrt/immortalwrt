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
 ****************************************************************************

    Module Name:
	igmp_snoop.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#ifndef __RTMP_IGMP_SNOOP_H__
#define __RTMP_IGMP_SNOOP_H__

#include "common/link_list.h"

#define IGMP_PROTOCOL_DESCRIPTOR	0x02
#define IGMP_MEMBERSHIP_QUERY		0x11	/*same for IGMP v1, v2 & v3*/
#define IGMP_V1_MEMBERSHIP_REPORT	0x12
#define IGMP_V2_MEMBERSHIP_REPORT	0x16
#define IGMP_LEAVE_GROUP			0x17
#define IGMP_V3_MEMBERSHIP_REPORT	0x22

#define MLD_LISTENER_QUERY			130		/*same for MLD v1 & v2*/
#define MLD_V1_LISTENER_REPORT		131
#define MLD_V1_LISTENER_DONE		132
#define MLD_V2_LISTERNER_REPORT		143

#ifdef IGMP_TVM_SUPPORT
#define IGMPMAC_TB_ENTRY_AGEOUT_TIME (300 * OS_HZ)
#else
#define IGMPMAC_TB_ENTRY_AGEOUT_TIME (120 * OS_HZ)
#endif /* IGMP_TVM_SUPPORT */


#define MAX_NUM_OF_GRP 				366

#define IPV4_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3])
#define IPV6_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5]^ \
										Addr[6] ^ Addr[7] ^ Addr[8] ^ Addr[9] ^ Addr[10] ^ Addr[11]^ \
										Addr[12] ^ Addr[13] ^ Addr[14] ^ Addr[15])

#define COPY_IPV6_ADDR(Addr1, Addr2)             memcpy((Addr1), (Addr2), IPV6_ADDR_LEN)
#define CVT_IPV4_IPV6(Addr1, Addr2)             memcpy((Addr1+12), (Addr2), IPV4_ADDR_LEN)

#define MULTICAST_IPV6_ADDR_HASH_INDEX(Addr)     \
	(IPV6_ADDR_HASH(Addr) & (MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE - 1))


#define IS_IPV6_MULTICAST_MAC_ADDR(Addr)	((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) == 0x33))

#define IGMP_NONE		0
#define IGMP_PKT		1
#define IGMP_IN_GROUP	2

#define IGMP_CFG_BAND0		(1 << 0)
#define IGMP_CFG_BAND1		(1 << 1)

#ifdef IGMP_SNOOPING_NON_OFFLOAD
#define UNKNOWN_DROP			0U
#define UNKNOWN_FLOODING		1U
#define CONTD_PER_ERR_CNT_MC	5U
#define MIN_FREE_TKN_CNT		300U
#define MAX_TKN_CNT_PER_STA		500U
#endif

enum {
	IGMPSN_G_POLICY = 1
};

#ifdef IGMP_SNOOPING_DENY_LIST
enum {
	DENY_ENTRY_INVALID = 0,
	DENY_ENTRY_VALID
};
#endif
#ifdef IGMP_TVM_SUPPORT
BOOLEAN IgmpSetAgeOutTime(
	IN PRTMP_ADAPTER pAd,
	UINT8 AgeOutTime,
	UINT8 OwnMacIdx);
BOOLEAN IgmpGetMcastEntryTable(
	RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	struct wifi_dev *wdev);
#endif

VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable);

VOID MultiCastFilterTableReset(
	PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable);

BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT16 wcid);

BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT16 wcid);

PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev);

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

#ifdef IGMP_TVM_SUPPORT
INT IgmpSnEnableTVMode(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 IsTVModeEnable,
	UINT8 TVModeType);

VOID ConvertUnicastMacToMulticast(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN RX_BLK * pRxBlk);

VOID MakeTVMIE(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN OUT PUCHAR pOutBuffer,
	IN OUT PULONG pFrameLen);

INT Set_IgmpSn_BlackList_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT Show_IgmpSn_BlackList_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

BOOLEAN isIgmpMldExemptPkt(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PUCHAR pGroupIpAddr,
	IN UINT16 ProtoType);

INT Set_IgmpSn_AgeOut_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_IgmpSn_McastTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

BOOLEAN IgmpSnoopingGetMulticastTable(
	RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	P_IGMP_MULTICAST_TABLE pMcastTable);

VOID IgmpSnoopingShowMulticastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN MulticastFilterConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx);

BOOLEAN MulticastFilterInitMcastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN bActive);

BOOLEAN MulticastFilterGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev);

#endif /* IGMP_TVM_SUPPORT */


VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT16 Wcid);
#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	OUT UINT8 *pProtoType,
	OUT PUCHAR * pMldHeader);

BOOLEAN IPv6MulticastFilterExcluded(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT16 Wcid);

#ifdef A4_CONN
/* Indicate if Specific Pkt is an MLD query message*/
BOOLEAN isMLDquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif


UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList);

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN struct wifi_dev *wdev,
	UINT16 Wcid);

VOID MulticastWLTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_WHITE_LIST_FILTER_TABLE * ppMulticastWLTable);

VOID MultiCastWLTableReset(
	PRTMP_ADAPTER pAd,
	IN PMULTICAST_WHITE_LIST_FILTER_TABLE * ppMulticastWLTable);

#ifdef IGMP_SNOOPING_DENY_LIST
VOID MulticastDLTableInit(IN PRTMP_ADAPTER pAd,
						IN P_MULTICAST_DENY_LIST_FILTER_TABLE *ppMulticastDLTable);
VOID MulticastDLTableReset(IN PRTMP_ADAPTER pAd,
						IN P_MULTICAST_DENY_LIST_FILTER_TABLE *ppMulsticastDLTable);
#endif

INT Set_IgmpSn_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_Allow_Non_Memb_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_AddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_DelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_TabDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_Igmp_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_Igmp_Show_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_Igmp_Show_FW_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);


#ifdef IGMP_SNOOPING_NON_OFFLOAD
NDIS_STATUS igmp_snoop_non_offload(IN PRTMP_ADAPTER pAd,
								IN struct wifi_dev *wdev,
								IN PNDIS_PACKET pkt);
#endif /* IGMP_SNOOP_NON_OFFLOAD */

#ifdef IGMP_SNOOPING_DENY_LIST
INT Set_IgmpSn_Deny_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
BOOLEAN MulticastDenyListUpdate(struct _RTMP_ADAPTER *pAd,
								PNET_DEV dev,
								UINT8 ucEntryCount,
								UINT8 ucAddToList,
								UINT8 *pAddr);
#endif

NDIS_STATUS IgmpPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev,
	OUT INT *pInIgmpGroup,
	OUT PMULTICAST_FILTER_TABLE_ENTRY *ppGroupEntry);

NDIS_STATUS IgmpPktClone(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	INT IgmpPktInGroup,
	PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	UCHAR QueIdx,
	UINT8 UserPriority,
	PNET_DEV pNetDev);

#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif



#ifdef A4_CONN

#define QUERY_SEND_PERIOD 6 /* 60 seconds */
#define QUERY_HOLD_PERIOD 15 /* 150 seconds*/

/* Send an IGMP query message on particular AP interface*/
void send_igmpv3_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry);

/* Send a MLD query message on particular AP interface*/
void send_mldv2_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry);

/* For specifed MBSS, compute & store IPv6 format checksum for MLD query message to be sent on that interface*/
void calc_mldv2_gen_query_chksum(
	IN	PRTMP_ADAPTER	pAd,
	IN  BSS_STRUCT *pMbss);
#endif


#endif /* __RTMP_IGMP_SNOOP_H__ */

