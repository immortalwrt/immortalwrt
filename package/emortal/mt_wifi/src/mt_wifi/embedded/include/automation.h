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
	automation.h
*/


#ifndef __AUTOMATION_H__
#define __AUTOMATION_H__


#define PID_SIZE 256
#define TXS_LIST_ELEM_NUM 4096
#define BSS_CHECK_BEACON_SN_ON(_x)             ((_x.ChkItem & ENUM_CHK_BEACON_SN))
#define BSS_CHECK_BEACON_SSID_ON(_x)           ((_x.ChkItem & ENUM_CHK_BEACON_SSID))

/* Set Block Packet */
#define ARP_PACKET BIT(0)
#define ICMP_PACKET BIT(1)

typedef enum {
	TXS_INIT = 0,
	TXS_COUNT_TEST,
	TXS_BAR_TEST,
	TXS_DEAUTH_TEST,
	TXS_RTS_TEST,
	TXS_BA_TEST,
	TXS_DUMP_DATA,
	TXS_NUM
} AUTOMATION_TXS_TESTYPE;

typedef enum {
	TXS = 0,
	RXV,
	APPS
} AUTOMATION_INIT_TYPE;

typedef enum {
	AUTOMATION_MANAGEMENT = 10,
	AUTOMATION_CONTROL,
	AUTOMATION_DATA
} AUTOMATION_FRAME_TYPE;

typedef struct _TXS_CHECK_ITEM {
	UINT32 time_stamp;
} TXS_CHECK_ITEM;

typedef struct _TXS_LIST_ENTRY {
	DL_LIST mList;
	UINT8 wlan_idx;
} TXS_LIST_ENTRY;

typedef struct _TXS_LIST_POOL {
	TXS_LIST_ENTRY Entry[TXS_LIST_ELEM_NUM];
	DL_LIST List;
} TXS_LIST_POOL;

typedef struct _TXS_FREE_LIST_POOL {
	TXS_LIST_ENTRY head;
	TXS_LIST_POOL pool_head;
	UINT32 entry_number;
	NDIS_SPIN_LOCK Lock;
	UINT32 txs_list_cnt;
} TXS_FREE_LIST_POOL;

typedef struct _TXS_LIST {
	UINT32 Num;
	NDIS_SPIN_LOCK lock;
	TXS_LIST_ENTRY pHead[PID_SIZE];
	TXS_FREE_LIST_POOL *pFreeEntrylist;
} TXS_LIST, *PTXS_LIST;

typedef struct TXS_TEST {
	BOOL init;
	AUTOMATION_TXS_TESTYPE test_type;
	UINT8 format;

	UINT8	pid;
	UINT8	received_pid;
	BOOLEAN	stop_send_test;
	BOOLEAN	duplicate_txs;

	/* statistic */
	UINT32 total_req;
	UINT32 total_rsp;
	TXS_LIST txs_list;
	TXS_CHECK_ITEM check_item[PID_SIZE];

} TXS_TEST, *PTXS_TEST;

typedef struct _RXV_TEST {
	BOOLEAN enable;
	BOOLEAN rxv_test_result;
	UINT32 rx_count;

	UINT32 rx_mode:3;
	UINT32 rx_rate:7;
	UINT32 rx_bw:2;
	UINT32 rx_sgi:1;
	UINT32 rx_stbc:2;
	UINT32 rx_ldpc:1;
	UINT32 rx_nss:1;

} RXV_TEST, *PRXV_TEST;

typedef struct _HEADER_CHECK {
	BOOLEAN test_result;
	BOOLEAN enable;
	UINT16 type:2;		/* MSDU type, refer to FC_TYPE_XX */
	UINT16 subtype:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	UINT16 moredata:1;	/* More data bit */
	UINT16 eosp;		/* EOSP bit */

} HEADER_CHECK, *PHEADER_CHECK;

typedef struct _APPS_TEST {
	UINT32 block_packet;
	HEADER_CHECK head_chk;

} APPS_TEST, *PAPPS_TEST;

typedef struct _AUTOMATION_DVT {
	TXS_TEST txs;
	RXV_TEST rxv;
	APPS_TEST apps;
} AUTOMATION_DVT, *PAUTOMATION_DVT;

typedef enum _BSS_CHECK_ITEM_T {
	ENUM_CHK_BEACON_SN = BIT(0),
	ENUM_CHK_BEACON_SSID = BIT(1),
} BSS_CHECK_ITEM;

typedef struct _BSS_ENTRY_CHECK {
	BOOLEAN bValid;
	UINT16 BssCurrentBeaconSN;
	UINT16 BssLastBeaconSN;
	BOOLEAN bSsidModified;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR BssMAC[MAC_ADDR_LEN];
} BSS_ENTRY_CHECK, *P_BSS_ENTRY_CHECK;

typedef struct _BSS_CHECK_CTRL {
	BOOLEAN bEnable;
	UINT8 TestItem;
	UINT32 ChkItem;
	UINT16 BssCnt;
	BSS_ENTRY_CHECK BssEntry[MAX_BEACON_NUM*2];
} BSS_CHECK_CTRL, *P_BSS_CHECK_CTRL;

/* TXS Test */
bool send_add_txs_queue(UINT8 pid, UINT8 wlan_idx);
bool receive_del_txs_queue(UINT32 sn, UINT8 pid, UINT8 wlan_idx, UINT32 time_stamp);
INT set_txs_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txs_test_result(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT is_frame_test(RTMP_ADAPTER *pAd, UINT8 send_received);
VOID frame_correct_test(UINT16 seq_num, UINT8 rec_pid);

/* RXV Test */
INT set_rxv_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_rxv_test_result(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT rxv_correct_test(UCHAR *Data);

#ifdef HDR_TRANS_RX_SUPPORT
INT set_hdr_translate_blist(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* HDR_TRANS_RX_SUPPORT */

/* AP MBSS Test */
BSS_ENTRY_CHECK *ap_bss_check_by_mac_lookup(RTMP_ADAPTER *pAd, BSS_CHECK_CTRL *pBssChkCtrl, UCHAR *pAddr);
VOID rx_peer_beacon_check(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list, MLME_QUEUE_ELEM *Elem);
INT set_ap_mbss_check_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ap_mbss_get_result_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* WTBL Test */
VOID rxd_wcid_check(RTMP_ADAPTER *pAd, UINT16 RxDWlanIdx);

/* APPS Test */
#ifdef APCLI_SUPPORT
#ifdef UAPSD_SUPPORT
INT set_ApCli_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ApCli_APSDAC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ApCli_MaxSPLength_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* UAPSD_SUPPORT */
INT set_ApCli_Block_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ApCli_Rx_Packet_Check_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID sta_rx_packet_check(RTMP_ADAPTER *pAd, VOID *pRx_Blk);
#endif /* APCLI_SUPPORT */

INT set_txrx_dbg_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID automation_rx_payload_check(RTMP_ADAPTER *pAd, PNDIS_PACKET pRxPkt);
VOID automation_dump_rxd_rxblk(RTMP_ADAPTER *pAd, CHAR *func, INT line, struct _RX_BLK *pRxBlk, struct _RXD_BASE_STRUCT *rx_base);
#endif /*  __AUTOMATION_H__ */

