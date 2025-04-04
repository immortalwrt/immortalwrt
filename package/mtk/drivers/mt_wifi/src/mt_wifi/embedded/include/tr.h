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
/***************************************************************************
 ***************************************************************************

*/

#ifndef __TR_H__
#define __TR_H__

#include "common/wifi_sys_info.h"

#define INFRA_TP_PEEK_BOUND_THRESHOLD 50
#define VERIWAVE_TP_PEEK_BOUND_TH 30
#define VERIWAVE_2G_PKT_CNT_TH 10
#define VERIWAVE_5G_PKT_CNT_TH 10
#define BYTES_PER_SEC_TO_MBPS	17
#define MCLI_TRAFFIC_TP_TH		2560/*20kbps*/
#define TX_MODE_RATIO_THRESHOLD	70
#define RX_MODE_RATIO_THRESHOLD	70
#define STA_TP_IDLE_THRESHOLD 10
#define STA_NUMBER_FOR_TRIGGER                1
#define MULTI_CLIENT_NUMS_TH 16
#define MULTI_CLIENT_2G_NUMS_TH 16
#define MULTI_TCP_NUMS_TH 1
#define INFRA_KEEP_STA_PKT_TH 1
#define VERIWAVE_TP_AMSDU_DIS_TH 1400
#define VERIWAVE_2G_TP_AMSDU_DIS_TH 1024
#define VERIWAVE_PER_RTS_DIS_TH_LOW_MARK 3
#define VERIWAVE_PER_RTS_DIS_TH_HIGH_MARK 9
#define VERIWAVE_PKT_LEN_LOW 60
#define VERIWAVE_INVALID_PKT_LEN_HIGH 2000
#define VERIWAVE_INVALID_PKT_LEN_LOW 40
#define	TRAFFIC_0	0
#define	TRAFFIC_DL_MODE	1
#define	TRAFFIC_UL_MODE	2
#define TRAFFIC_BIDIR_ACTIVE_MODE 3
#define TRAFFIC_BIDIR_IDLE_MODE 4

#define HW_TX_BATCH_CNT 3
#define HW_QUE_AGG_TIMEOUT 0x90
#define HW_MAX_AGG_EN_TP 300
#define HW_MIN_AGG_EN_TP 50

#define REORDERING_PACKET_TIMEOUT_IN_MS		(100)
#define MAX_REORDERING_PACKET_TIMEOUT_IN_MS	(1500)

struct tx_delay_control {
	RALINK_TIMER_STRUCT que_agg_timer;
	UINT8 idx;
	BOOLEAN que_agg_timer_running;
	BOOLEAN que_agg_en;
	BOOLEAN force_deq;
#define TX_BATCH_CNT 4
	UINT32 tx_process_batch_cnt;
#define MIN_AGG_PKT_LEN 58
#define MAX_AGG_PKT_LEN 135
#define MAX_AGG_EN_TP 700
#define MIN_AGG_EN_TP 50
	UINT32 min_pkt_len;
	UINT32 max_pkt_len;
#define QUE_AGG_TIMEOUT 4000
	UINT32 que_agg_timeout_value;
	BOOLEAN hw_enabled;
	UINT32 min_tx_delay_en_tp;
	UINT32 max_tx_delay_en_tp;
	VOID *priv;
};
#ifdef IXIA_C50_MODE
enum {
	TX_AMPDU = 0,
	TX_AMSDU,
	TX_LEGACY,
	TX_FRAG,
	TX_TYPE_MAX
};
#endif

#ifdef TXBF_SUPPORT
typedef struct _COUNTER_TXBF {
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} COUNTER_TXBF;
#endif /* TXBF_SUPPORT */

struct reordering_mpdu;

struct reordering_list {
	struct reordering_mpdu *next;
	struct reordering_mpdu *tail;
	int qlen;
};

struct reordering_mpdu {
	struct reordering_mpdu *next;
	struct reordering_list AmsduList;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR OpMode;
	UCHAR band;
};

struct reordering_mpdu_pool {
	PVOID mem;
	NDIS_SPIN_LOCK lock;
	struct reordering_list freelist;
};


typedef enum _REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Initialization,
	Recipient_Established,
	recipient_offload,
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY {
	UINT16 Wcid;
	UCHAR TID;
	UINT16 BAWinSize;
	UCHAR Token;
	UCHAR amsdu_cap;
	/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

struct ba_rec_debug {
	UINT16 sn;
	UINT8 amsdu;
#define BA_DATA 0
#define BA_BAR 1
	UINT8 type;
	USHORT last_in_seq;
	UINT16 wcid;
	UCHAR ta[MAC_ADDR_LEN];
	UCHAR ra[MAC_ADDR_LEN];
};

typedef struct _BA_REC_ENTRY {
	BOOLEAN check_amsdu_miss  ____cacheline_aligned;
	UINT8 PreviousAmsduState;
	UINT16 PreviousSN;
	UINT16 PreviousReorderCase;
	REC_BLOCKACK_STATUS REC_BA_Status;
	USHORT LastIndSeq;
	NDIS_SPIN_LOCK RxReRingLock;	/* Rx Ring spinlock */
	UINT16 BAWinSize;	/* 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU. */
	ULONG LastIndSeqAtTimer;
	ULONG drop_dup_pkts;
	ULONG drop_old_pkts;
	ULONG drop_unknown_state_pkts;
	ULONG ba_sn_large_win_end;
	struct reordering_list list;
	struct reordering_mpdu *CurMpdu;
#define STEP_ONE 0
#define REPEAT 1
#define OLDPKT 2
#define WITHIN 3
#define SURPASS 4
	UINT16 Wcid;
	UCHAR TID;
	UCHAR band;
	USHORT TimeOutValue;
	PVOID pAdapter;
#define BA_REC_DBG_SIZE 256
	struct ba_rec_debug *ba_rec_dbg;
	UINT32 ba_rec_dbg_idx;
} BA_REC_ENTRY, *PBA_REC_ENTRY;

enum {
	SN_HISTORY = (1 << 0),
	SN_RECORD_BASIC = (1 << 1),
	SN_RECORD_MAC = (1 << 2),
	SN_DUMP_WITHIN = (1 << 3),
	SN_DUMP_SURPASS = (1 << 4),
	SN_DUMP_OLD = (1 << 5),
	SN_DUMP_DUP = (1 << 6),
	SN_DUMP_STEPONE = (1 << 7),
	SN_DUMP_BAR = (1 << 8),
};

struct ba_control {
	BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
	BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	NDIS_SPIN_LOCK BATabLock;
	struct reordering_mpdu_pool mpdu_blk_pool[2];
#define BA_TIMEOUT_BITMAP_LEN (MAX_LEN_OF_BA_REC_TABLE/32)
	BOOLEAN ba_timeout_check;
	UINT32 ba_timeout_bitmap[BA_TIMEOUT_BITMAP_LEN];
#ifdef RX_RPS_SUPPORT
	BOOLEAN ba_timeout_check_per_cpu[NR_CPUS];
	UINT32 ba_timeout_bitmap_per_cpu[NR_CPUS][BA_TIMEOUT_BITMAP_LEN];
#endif
	ULONG numAsRecipient;	/*  I am recipient of numAsRecipient clients. These client are in the BARecEntry[] */
	ULONG numAsOriginator;	/*  I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[] */
	ULONG numDoneOriginator;	/*  count Done Originator sessions */
	ULONG dbg_flag;
};

struct fq_stainfo_type {
	QUEUE_ENTRY Entry[WMM_NUM_OF_AC];
	UINT16 macInQLen[WMM_NUM_OF_AC];
	UINT16 macOutQLen[WMM_NUM_OF_AC];
	UINT16 wcid;
	UINT8 kickPktCnt[WMM_NUM_OF_AC];
	UINT8 thMax[WMM_NUM_OF_AC];
	UINT16 mpduTime;
	UINT16 KMAX;
	UINT32 drop_cnt[WMM_NUM_OF_AC];
	UINT32 qlen_max_cnt[WMM_NUM_OF_AC];
	INT32 tx_msdu_cnt;
	INT32 macQPktLen[WMM_NUM_OF_AC];
	UINT8 status[WMM_NUM_OF_AC];
	NDIS_SPIN_LOCK	lock[WMM_NUM_OF_AC];
};

typedef struct _STA_TR_ENTRY {
	UINT32 EntryType;
	struct wifi_dev *wdev;

#ifdef SW_CONNECT_SUPPORT
	/* mark this entry is pure S/W or not */
	BOOLEAN bSw;
	/*
		original tr_entry->wcid entry usages are most for S/W concept, only minor parts are for H/W concept.
		so add extra tr_entry->hw_wcid for hw backup when fill TxD real H/W wcid
	*/
	UINT16 hw_wcid;
#endif /* SW_CONNECT_SUPPORT */

	UINT16 wcid;
	/*
		func_tb_idx used to indicate following index:
			in StaCfg
			in pAd->MeshTab
			in WdsTab.MacTab
	*/

	UCHAR func_tb_idx;
	UCHAR Addr[MAC_ADDR_LEN];
	/*
		Tx Info
	*/
	USHORT NonQosDataSeq;
	USHORT TxSeq[NUM_OF_TID];

	QUEUE_HEADER tx_queue[WMM_QUE_NUM];
	QUEUE_HEADER ps_queue;
	UINT enqCount;
	UINT TokenCount[WMM_QUE_NUM];
	INT ps_qbitmap;
	UCHAR ps_state;
	UCHAR retrieve_start_state;
	UCHAR token_enq_all_fail;

	BOOLEAN tx_pend_for_agg[WMM_QUE_NUM];
	NDIS_SPIN_LOCK txq_lock[WMM_QUE_NUM];
	NDIS_SPIN_LOCK ps_queue_lock;
	NDIS_SPIN_LOCK ps_sync_lock;

	UINT deq_cnt;
	UINT deq_bytes;
	ULONG PsQIdleCount;

	BOOLEAN enq_cap;
	BOOLEAN deq_cap;

#ifdef SW_CONNECT_SUPPORT
	/* Tx debug Cnts */
	UINT32 tx_fp_allow_cnt;
	UINT32 tx_send_data_cnt;
	UINT32 tx_deq_eap_cnt;
	UINT32 tx_deq_arp_cnt;
	UINT32 tx_deq_data_cnt;
	UINT32 tx_handle_cnt;

	/* Rx debug Cnts */
	UINT32 rx_handle_cnt;
	UINT32 rx_u2m_cnt;
	UINT32 rx_eap_cnt;
#endif /* SW_CONNECT_SUPPORT */

	/*
		STA status
	*/

	UCHAR bssid[MAC_ADDR_LEN];
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */
	BOOLEAN isCached;
	UCHAR PortSecured;
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */

	UCHAR CurrTxRate;

#ifdef VENDOR_FEATURE1_SUPPORT
	/* total 128B, use UINT32 to avoid alignment problem */
	UINT32 HeaderBuf[32];	/* (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */
	UCHAR HdrPadLen;	/* padding length*/
	UCHAR MpduHeaderLen; /* 802.11 header + LLC/SNAP not including padding */
	UCHAR wifi_hdr_len; /* 802.11 header */
	UINT16 Protocol;
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef DOT11_N_SUPPORT
	UINT32 CachedBuf[16];	/* UINT (4 bytes) for alignment */

	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT tx_amsdu_bitmap;
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UCHAR MmpsMode;		/* MIMO power save mode. */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bReptCli;
	BOOLEAN bReptEthCli;
	UCHAR MatchReptCliIdx;
	UCHAR ReptCliAddr[MAC_ADDR_LEN];
	ULONG ReptCliIdleCount;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef TXBF_SUPPORT
	UCHAR			TxSndgType;
	NDIS_SPIN_LOCK	TxSndgLock;

	/* ETxBF */
	UCHAR		bfState;
	UCHAR		sndgMcs;
	UCHAR		sndgBW;
	UCHAR		sndg0Mcs;
	INT			sndg0Snr0, sndg0Snr1, sndg0Snr2;

#ifdef ETXBF_EN_COND3_SUPPORT
	UCHAR		bestMethod;
	UCHAR		sndgRateIdx;
	UCHAR		bf0Mcs, sndg0RateIdx, bf0RateIdx;
	UCHAR		sndg1Mcs, bf1Mcs, sndg1RateIdx, bf1RateIdx;
	INT			sndg1Snr0, sndg1Snr1, sndg1Snr2;
#endif /* ETXBF_EN_COND3_SUPPORT */
	UCHAR		noSndgCnt;
	UCHAR		eTxBfEnCond;
	UCHAR		noSndgCntThrd, ndpSndgStreams;
	UCHAR		iTxBfEn;
	RALINK_TIMER_STRUCT eTxBfProbeTimer;

	BOOLEAN		phyETxBf;			/* True=>Set ETxBF bit in PHY rate */
	BOOLEAN		phyITxBf;			/* True=>Set ITxBF bit in PHY rate */
	UCHAR		lastNonBfRate;		/* Last good non-BF rate */
	BOOLEAN		lastRatePhyTxBf;	/* For Quick Check. True if last rate was BF */
	USHORT	  BfTxQuality[MAX_TX_RATE_INDEX + 1];	/* Beamformed TX Quality */

	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
	UINT LastETxCount;		/* Used to compute %BF statistics */
	UINT LastITxCount;
	UINT LastTxCount;
#endif /* TXBF_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	UINT8 snd_dialog_token;
#ifdef SOFT_SOUNDING
	BOOLEAN snd_reqired;
	HTTRANSMIT_SETTING snd_rate;
#endif /* SOFT_SOUNDING */
#endif /* VHT_TXBF_SUPPORT */


	/*
		Statistics related parameters
	*/
	UINT32 ContinueTxFailCnt;
	ULONG TimeStamp_toTxRing;
	ULONG NoDataIdleCount;

#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif /* CONFIG_AP_SUPPORT */
	ULONG one_sec_tx_pkts;
	ULONG avg_tx_pkts;
	UINT8 previous_amsdu_state[NUM_OF_UP];
	INT previous_sn[NUM_OF_UP];
	INT cacheSn[NUM_OF_UP];
	INT cacheMgmtSn[NUM_OF_MGMT_SN_CAT];
	UINT8 is_amsdu_invalid[NUM_OF_UP];
	UINT8 previous_amsdu_invalid_state[NUM_OF_UP];
	UINT previous_amsdu_invalid_sn[NUM_OF_UP];
#ifdef SW_CONNECT_SUPPORT
	USHORT RxSeq[NUM_OF_TID];
#endif /* SW_CONNECT_SUPPORT */
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
	INT32 TotalPageCount[WMM_QUE_NUM];
#endif
#endif
	/*
		Used to ignore consecutive PS poll.
		set: when we get a PS poll.
		clear: when a PS data is sent or two period passed.
	*/
	UINT8 PsDeQWaitCnt;

	UINT8 OmacIdx;
	struct _STA_REC_CTRL_T StaRec;

#ifdef DBG_AMSDU
#define TIME_SLOT_NUMS 10
	UINT32 amsdu_1;
	UINT32 amsdu_1_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_2;
	UINT32 amsdu_2_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_3;
	UINT32 amsdu_3_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_4;
	UINT32 amsdu_4_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_5;
	UINT32 amsdu_5_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_6;
	UINT32 amsdu_6_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_7;
	UINT32 amsdu_7_rec[TIME_SLOT_NUMS];
	UINT32 amsdu_8;
	UINT32 amsdu_8_rec[TIME_SLOT_NUMS];
#endif
	UINT16 token_cnt;
#ifdef FQ_SCH_SUPPORT
	struct fq_stainfo_type	fq_sta_rec;
#endif
} STA_TR_ENTRY;

enum {
	TP_DEBUG_ISR = (1 << 0),
	TP_DEBUG_TIMING = (1 << 1),
	TP_DEBUG_IO = (1 << 2),
};

#define TP_DBG_TIME_SLOT_NUMS 10
struct tp_debug {
	UINT32 IsrTxDlyCnt;
	UINT32 IsrTxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrTxCnt;
	UINT32 IsrTxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxDlyCnt;
	UINT32 IsrRxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxCnt;
	UINT32 IsrRxCntNoClear;
	UINT32 IsrRxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRx1Cnt;
	UINT32 IsrRx1CntNoClear;
	UINT32 IsrRx1CntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Isr1;
	UINT32 Isr2;
	UINT32 Isr3;
	UINT32 Isr4;
	UINT32 IoReadTx;
	UINT32 IoReadTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteTx;
	UINT32 IoWriteTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx;
	UINT32 IoReadRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx1;
	UINT32 IoReadRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx;
	UINT32 IoWriteRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx1;
	UINT32 IoWriteRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx;
	UINT32 MaxProcessCntRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx1;
	UINT32 MaxProcessCntRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntA;
	UINT32 MaxProcessCntRxRecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntB;
	UINT32 MaxProcessCntRxRecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntC;
	UINT32 MaxProcessCntRxRecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntD;
	UINT32 MaxProcessCntRxRecD[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntA;
	UINT32 MaxProcessCntRx1RecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntB;
	UINT32 MaxProcessCntRx1RecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntC;
	UINT32 MaxProcessCntRx1RecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntD;
	UINT32 MaxProcessCntRx1RecD[TP_DBG_TIME_SLOT_NUMS];
	/* Timeing */
	UINT32 TRDoneTimes;
	UINT32 TRDoneTimesRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 TRDoneTimeStamp;
	UINT32 TRDoneInterval[TP_DBG_TIME_SLOT_NUMS];
	UINT16 time_slot;
	UINT8  debug_flag;
	RALINK_TIMER_STRUCT tp_dbg_history_timer;
};

struct tr_flow_control {
	UINT8 *TxFlowBlockState;
	NDIS_SPIN_LOCK *TxBlockLock;
	DL_LIST *TxBlockDevList;
	BOOLEAN IsTxBlocked;
	UINT8 RxFlowBlockState;
};

struct _rx_mod_cnt {
	UINT32 cck[7];
	UINT32 ofdm[8];
	UINT32 ht[2][32];
	UINT32 vht[2][4][12];
	UINT32 he[3][4][12];
};

struct _rx_profiling {
	UINT32 total_mpdu_cnt;
	UINT32 total_retry_cnt;
	struct _rx_mod_cnt mpdu_cnt[BW_160];
	struct _rx_mod_cnt retry_cnt[BW_160];
};

/**
 * @me: MPDU error
 * @re: RTS error
 * @le: Lifetime error
 * @be: BIP error
 * @txop_limit_error: TXOP Limit error
 * @baf: BA Fail
 */
struct tr_counter {
	/* TX */
	UINT32 tx_invalid_wdev;
	UINT32 tx_sw_dataq_drop;
	UINT32 tx_sw_mgmtq_drop;
	UINT32 tx_wcid_invalid;
	UINT32 wlan_state_non_valid_drop;
	UINT32 mgmt_max_drop;
	UINT32 tx_not_allowed_drop;
	UINT32 sys_not_ready_drop;
	UINT32 err_recovery_drop;
	UINT32 tx_forbid_drop;
	UINT32 igmp_clone_fail_drop;
	UINT32 ps_max_drop;
	UINT32 fill_tx_blk_fail_drop;
	UINT32 wdev_null_drop;
	UINT32 carrier_detect_drop;
	UINT32 net_if_stop_cnt;
	UINT32 tx_unknow_type_drop;
	UINT32 pkt_len_invalid;
	UINT32 pkt_invalid_wcid;
	UINT32 me[NUM_OF_TID];
	UINT32 re[NUM_OF_TID];
	UINT32 le[NUM_OF_TID];
	UINT32 be[NUM_OF_TID];
	UINT32 txop_limit_error[NUM_OF_TID];
	UINT32 baf[NUM_OF_TID];
	UINT32 queue_deep_cnt[NUM_OF_TID];
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	UINT32 ctxd_num[4];
#endif
	UINT32 tx_enq_cpu_stat[2][4];
	UINT32 tx_deq_cpu_stat[2][4];

	/* RX */
	UINT32 rx_icv_err_cnt;
	UINT32 rx_sw_q_drop;
	UINT32 rx_invalid_wdev;
	UINT32 rx_invalid_pkt_len;
	UINT32 rx_to_os_drop;
	UINT32 rx_pn_mismatch;
	UINT32 ba_err_wcid_invalid;
	UINT32 ba_drop_unknown;
	UINT32 ba_err_old;
	UINT32 ba_err_dup1;
	UINT32 ba_err_dup2;
	UINT32 ba_err_tear_down;
	UINT32 ba_flush_one;
	UINT32 ba_sn_large_win_end;
	UINT32 bar_cnt;
	UINT32 bar_large_win_start;
	UINT32 ba_flush_all;
	UINT32 ba_amsdu_miss;
	UINT32 rx_sw_amsdu_call;
	UINT32 rx_sw_amsdu_last_len;
	UINT32 rx_sw_amsdu_num;
	struct _rx_profiling rx_rate_rc[BAND_NUM];
};

enum {
	TX_SW_AMSDU,
	TX_HW_AMSDU,
};

enum {
	RX_SW_AMSDU,
	RX_HW_AMSDU,
};

struct tx_rx_ctl {
	struct ba_control ba_ctl;
	STA_TR_ENTRY tr_entry[MAX_LEN_OF_TR_TABLE];
	UINT8 amsdu_type;
	BOOLEAN amsdu_fix;
	UCHAR amsdu_fix_num;
	UINT8 damsdu_type;
	struct tr_flow_control tr_flow_ctl;
	struct tx_delay_control tx_delay_ctl[2];
#ifdef CONFIG_TP_DBG
	struct tp_debug tp_dbg;
#endif
	struct tr_counter tr_cnt;
	UINT32 max_tx_process;
	struct notify_head traffic_notify_head;
	VOID *napi;
	BOOLEAN en_rx_profiling;
#ifdef VLAN_SUPPORT
	UCHAR vlan2ethctrl;
#endif
};

typedef struct _TX_BLOCK_DEV {
	DL_LIST list;
	PNET_DEV NetDev;
} TX_BLOCK_DEV;

struct _RTMP_ADAPTER;

enum {
	NO_ENOUGH_SWQ_SPACE = (1 << 0),
};

INT32 tr_ctl_init(struct _RTMP_ADAPTER *pAd);
INT32 tr_ctl_exit(struct _RTMP_ADAPTER *pAd);
BOOLEAN tx_flow_check_state(struct _RTMP_ADAPTER *pAd, UINT8 State, UINT8 RingIdx);
INT32 tx_flow_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
INT32 tx_flow_set_state_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
BOOLEAN tx_flow_check_if_blocked(struct _RTMP_ADAPTER *pAd);
VOID ba_ctl_init(struct _RTMP_ADAPTER *pAd, struct ba_control *ba_ctl);
VOID ba_ctl_exit(struct ba_control *ba_ctl);
#endif
