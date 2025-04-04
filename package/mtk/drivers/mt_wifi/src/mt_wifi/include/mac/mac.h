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
	mac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __MAC_H__
#define __MAC_H__

#ifdef MT_MAC
#include "mac/mac_mt/mt_mac.h"
#include "mac/mac_mt/mt_mac_ctrl.h"
#endif /* MT_MAC */

#define VENDOR_OUI_MTK  {0x00, 0x0C, 0xE7}

/* 7.3.2.2 Data Rate Value */
#define RATE_1M                             2	/* 1M in unit of 500kb/s */
#define RATE_2M                             4	/* 2M */
#define RATE_5_5M                           11	/* 5.5M */
#define RATE_11M                            22	/* 11M */
#define RATE_22M                            44	/* 22M */
#define RATE_33M                            66	/* 33M */
#define RATE_6M                             12	/* 6M */
#define RATE_9M                             18	/* 9M */
#define RATE_12M                            24	/* 12M */
#define RATE_18M                            36	/* 18M */
#define RATE_24M                            48	/* 24M */
#define RATE_36M                            72	/* 36M */
#define RATE_48M                            96	/* 48M */
#define RATE_54M                            108	/* 54M */

#define WMM_QUE_NUM 4

enum {
	QID_AC_BK,
	QID_AC_BE,
	QID_AC_VI,
	QID_AC_VO,
	QID_HCCA,
	QID_BMC = 8,
	QID_MGMT = 13,
	QID_RX = 14,
	QID_CTRL = 16,
	QID_BCN = 17,
};

#define NUM_OF_TX_RING		4
#define NUM_OF_WMM1_TX_RING	1

#define ETH_TYPE_VLAN	0x8100
#define ETH_TYPE_IPv4	0x0800
#define ETH_TYPE_IPv6	0x86dd
#define ETH_TYPE_ARP	0x0806
#define ETH_TYPE_EAPOL	0x888e
#define ETH_TYPE_WAI	0x88b4
#define ETHER_TYPE_TDLS_MMPDU 0x890d
#define ETH_TYPE_FASTROAMING	0x890d
#define ETH_TYPE_1905  0x893A

#define IP_VER_CODE_V4	0x40
#define IP_VER_CODE_V6	0x60
#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_TCP 0x06
#define IP_PROTO_UDP	0x11
#define IP_HDR_LEN		20
#define ETH_HDR_LEN		14

#define ICMP_TYPE_ECHO_RSP 0
#define ICMP_TYPE_ECHO_REQ 8

#define DMA_SCH_LMAC		0
#define DMA_SCH_BYPASS		1
#define DMA_SCH_HYBRID		2

#define TXINFO_SIZE			4
typedef union GNU_PACKED _TXINFO_STRUC {
	UINT32 word;
} TXINFO_STRUC;

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

/*
	bit31 =>802.3 if set 1, implay you hav did header translation
	bit30 => put VLAN field

*/
#define RAL_RXINFO_SIZE			4
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		hdr_trans_ip_sum_err:1;		/* IP checksum error */
	UINT32		vlan_taged_tcp_sum_err:1;	/* TCP checksum error */
	UINT32		rsv:1;
	UINT32		action_wanted:1;
	UINT32		deauth:1;
	UINT32		disasso:1;
	UINT32		beacon:1;
	UINT32		probe_rsp:1;
	UINT32		sw_fc_type1:1;
	UINT32		sw_fc_type0:1;
	UINT32		pn_len:3;
	UINT32		wapi_kidx:1;
	UINT32		BssIdx3:1;
	UINT32		Decrypted:1;
	UINT32		AMPDU:1;
	UINT32		L2PAD:1;
	UINT32		RSSI:1;
	UINT32		HTC:1;
	UINT32		AMSDU:1;		/* rx with 802.3 header, not 802.11 header. obsolete. */
	UINT32		CipherErr:2;       /* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	UINT32		Crc:1;			/* 1: CRC error */
	UINT32		MyBss:1;		/* 1: this frame belongs to the same BSSID */
	UINT32		Bcast:1;			/* 1: this is a broadcast frame */
	UINT32		Mcast:1;			/* 1: this is a multicast frame */
	UINT32		U2M:1;			/* 1: this RX frame is unicast to me */
	UINT32		FRAG:1;
	UINT32		NULLDATA:1;
	UINT32		DATA:1;
	UINT32		BA:1;
}	RXINFO_STRUC;
#else
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		BA:1;
	UINT32		DATA:1;
	UINT32		NULLDATA:1;
	UINT32		FRAG:1;
	UINT32		U2M:1;
	UINT32		Mcast:1;
	UINT32		Bcast:1;
	UINT32		MyBss:1;
	UINT32		Crc:1;
	UINT32		CipherErr:2;
	UINT32		AMSDU:1;
	UINT32		HTC:1;
	UINT32		RSSI:1;
	UINT32		L2PAD:1;
	UINT32		AMPDU:1;
	UINT32		Decrypted:1;
	UINT32		BssIdx3:1;
	UINT32		wapi_kidx:1;
	UINT32		pn_len:3;
	UINT32		sw_fc_type0:1;
	UINT32      sw_fc_type1:1;
	UINT32      probe_rsp:1;
	UINT32		beacon:1;
	UINT32		disasso:1;
	UINT32      deauth:1;
	UINT32      action_wanted:1;
	UINT32      rsv:1;
	UINT32		vlan_taged_tcp_sum_err:1;
	UINT32		hdr_trans_ip_sum_err:1;
} RXINFO_STRUC;
#endif

struct phy_params {
	UINT8 phy_mode;
	UINT8 prim_ch;
	UINT8 bw;
	UINT8 ldpc;
	BOOLEAN stbc;
	UINT8 gi_type;
	UINT8 ltf_type;
	UINT8 tx_ibf;
	UINT8 tx_ebf;
	UINT8 vht_nss;
	UINT32 rate;
	BOOLEAN dcm;
	BOOLEAN su_ext_tone;
	CHAR  pwr_offset;
	CHAR ant_pri;
	CHAR spe_idx;
	UINT16 type;
	UINT16 subtype;
};

typedef struct _MAC_TX_INFO {
	UINT16 WCID;
	BOOLEAN FRAG;
	BOOLEAN InsTimestamp;
	BOOLEAN NSeq;
	BOOLEAN Ack;
	BOOLEAN BM;
	BOOLEAN CFACK;
	BOOLEAN AMPDU;
	BOOLEAN sw_duration;
	BOOLEAN htc;
	UCHAR BASize;
	UCHAR PID;
	UCHAR TID;
	USHORT assigned_seq;
	UCHAR TxRate;
	UCHAR Txopmode;
	ULONG Length;
	UCHAR hdr_len;
	UCHAR hdr_pad;
	UCHAR eth_type_offset;
	UCHAR bss_idx;
	UCHAR q_idx;
	UCHAR prot;
	UCHAR AntPri;
	UCHAR SpeEn;
	UCHAR Preamble;
#ifdef MT_MAC
	UCHAR Type;
	UCHAR SubType;
	UINT32 TxSPriv;
	UCHAR PsmBySw; /* PSM bit controlled by SW */
	UINT8 OmacIdx;
	UINT8 wmm_set; /* TODO: shiang-MT7615, replace band by bss_idx? */
	BOOLEAN IsTmr;
	BOOLEAN IsOffloadPkt;/* host gen pkt template, make pkt enqued by fw. */
	BOOLEAN txs2h;
	BOOLEAN txs2m;
	BOOLEAN addba;
	UCHAR txpwr_offset;
#endif /* MT_MAC */
	BOOLEAN IsAutoRate;
	UCHAR tx_lifetime;
#ifdef DPP_SUPPORT
	UINT16 seq_no;
#endif /* DPP_SUPPORT */
} MAC_TX_INFO;

enum {
	PID_DATA_NORMALUCAST = 0,
	PID_DATA_NOT_NORM_ACK,
	PID_DATA_AMPDU,
	PID_MGMT,
	PID_DATA_NO_ACK,
	PID_CTL_BAR,
	PID_PS_DATA,
	PID_TDLS,
	PID_P2P_ACTION,
	PID_NULL_FRAME_PWR_ACTIVE,
	PID_NULL_FRAME_PWR_SAVE,
	PID_MGMT_DPP_FRAME,
	PID_EAPOL_FRAME = 0x18,
	PID_BEACON = 0x20,
	PID_FTM_MIN = 0x21,
	PID_FTM_MAX = 0x40,
	PID_FD_FRAME = 0x41,
	PID_MAX = 0x42,
};

struct _RTMP_ADAPTER;
struct _TXD_STRUC;
struct _RXD_STRUC;

VOID dump_txinfo(struct _RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo);
VOID dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
VOID dump_txd(struct _RTMP_ADAPTER *pAd, struct _TXD_STRUC *pTxD);
VOID dump_rxd(struct _RTMP_ADAPTER *pAd, struct _RXD_STRUC *pRxD);
#endif
#endif /* __MAC_H__ */

