#ifndef __A4_CONN_H__
#define __A4_CONN_H__
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
 ***************************************************************************


    Module Name:
	a4_conn.h

    Abstract:
    This is A4 connection function used to process those 4-addr of connected APClient or STA.
    Used by MWDS and MAP feature

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef A4_CONN
#include "rtmp_def.h"

typedef struct _A4_CONNECT_ENTRY {
	DL_LIST List;
	UCHAR valid;
	UINT16 wcid;
} A4_CONNECT_ENTRY, *PA4_CONNECT_ENTRY;

#ifdef CONFIG_MAP_3ADDR_SUPPORT
typedef struct _Eth_CONNECT_ENTRY {
	DL_LIST List;
	UCHAR mac[6];
	UINT16 entry_flush_count;
} Eth_CONNECT_ENTRY, *PEth_CONNECT_ENTRY;

#define Eth_Entry_Flush_Time 45
#endif

#define A4_TYPE_NONE  0
#define A4_TYPE_MWDS  1
#define A4_TYPE_MAP   2 /*high priority*/


#define IS_ENTRY_A4(_x)					((_x)->a4_entry != 0)
#define GET_ENTRY_A4(_x)				((_x)->a4_entry)
#define SET_ENTRY_A4(_x, _type)			((_x)->a4_entry = _type)

#define IS_APCLI_A4(_x)					((_x)->a4_apcli != 0)
#define GET_APCLI_A4(_x)				((_x)->a4_apcli)
#define SET_APCLI_A4(_x, _type)			((_x)->a4_apcli = _type)


BOOLEAN a4_interface_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
);

BOOLEAN a4_interface_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
);

VOID a4_proxy_delete(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
);

BOOLEAN a4_proxy_lookup(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN BOOLEAN update_alive_time,
	IN BOOLEAN is_rx,
	OUT UINT16 *wcid
);

VOID a4_proxy_update(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UINT16 wcid,
	IN PUCHAR mac_addr,
	IN UINT32 ip /* ARP Sender IP*/
);

VOID a4_proxy_maintain(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
);

void a4_send_clone_pkt(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PNDIS_PACKET pkt,
	IN PUCHAR exclude_mac_addr
);

BOOLEAN a4_ap_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

BOOLEAN a4_ap_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

#ifdef APCLI_SUPPORT
BOOLEAN a4_apcli_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

BOOLEAN a4_apcli_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);
#endif

INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER adapter,
	IN  RTMP_STRING *arg
);
INT Set_APProxy_Refresh_Proc(
	IN	PRTMP_ADAPTER adapter,
	IN	RTMP_STRING *arg);

#ifdef WARP_512_SUPPORT
MAC_TABLE_ENTRY *ReassignWcidForA4Entry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS *ie_lists);
#endif

#ifdef CONFIG_MAP_3ADDR_SUPPORT
BOOLEAN eth_lookup_entry_by_addr(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN UCHAR update_time
);

VOID eth_add_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
);

VOID eth_delete_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
);

VOID eth_update_list(
	IN PRTMP_ADAPTER adapter
);
#endif
#endif /* A4_CONN */
#endif /* __A4_CONN_H__*/

